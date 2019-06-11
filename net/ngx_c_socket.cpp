#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>   //uintptr_t
#include <stdarg.h>   //va_start....
#include <unistd.h>   //STDERR_FILENO等
#include <sys/time.h> //gettimeofday
#include <time.h>     //localtime_r
#include <fcntl.h>    //open
#include <errno.h>    //errno
//#include <sys/socket.h>
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"

CSocket::CSocket()
{
    /* init the variable */
    m_worker_connections = 1;
    m_ListenPortCount    = 1;

    m_epollhandle       = -1;
    m_pconnections      = NULL;
    m_pfree_connections = NULL;

    m_iLenPkgHeader = sizeof(COMM_PKG_HEADER);
    m_iLenMsgHeader = sizeof(STRUC_MSG_HEADER);

    m_iRecvMsgQueueCount = 0;
    pthread_mutex_init(&m_recvMessageQueueMutex, NULL);
}

CSocket::~CSocket()
{
    std::vector<lpngx_listening_t>::iterator pos;
    for (pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); pos++)
    {
        delete (*pos);
    }
    m_ListenSocketList.clear();

    if (m_pconnections != NULL)
        delete[] m_pfree_connections;
}

bool CSocket::Initialize()
{
    ReadConf();
    bool reco = ngx_open_listening_sockets();
    return reco;
}

void CSocket::ReadConf()
{
    CConfig* p_config    = CConfig::GetInstance();
    m_worker_connections = p_config->GetIntDefault("worker_connections", m_worker_connections);
    m_ListenPortCount    = p_config->GetIntDefault("ListenPortCount", m_ListenPortCount);
    ngx_log_error_core(NGX_LOG_INFO, 0, "m_worker_connection(%d) m_ListenPortCount(%d)", m_worker_connections, m_ListenPortCount);
}

bool CSocket::ngx_open_listening_sockets()
{
    int                isock;
    struct sockaddr_in serv_addr; // server address struct
    int                iport;
    char               strinfo[100];

    memset(&serv_addr, 0, sizeof(sockaddr_in));
    serv_addr.sin_family      = AF_INET; // IPV4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    CConfig* p_config = CConfig::GetInstance();
    for (int i = 0; i < m_ListenPortCount; i++)
    {
        isock = socket(AF_INET, SOCK_STREAM, 0);
        if (isock < 0)
        {
            ngx_log_stderr(errno, "CSocket initialize socket failed i = %d", i);
            return false;
        }
        int reuseaddr = 1;
        // parameter 2 and parameter 3 are a pair
        // SO_REUSEADDR in case of TIME_OUT bind() faild
        if (setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuseaddr, sizeof(reuseaddr)))
        {
            ngx_log_stderr(errno, "CSocket initialize setsockopt failed");
            close(isock);
            return false;
        }

        if (setnonblocking(isock) == false)
        {
            ngx_log_stderr(errno, "CSocket initialize socket non-blocking failed");
            close(isock);
            return false;
        }

        strinfo[0] = 0;
        sprintf(strinfo, "ListenPort%d", i);
        iport              = p_config->GetIntDefault(strinfo, 10000);
        serv_addr.sin_port = htons((in_port_t)iport);

        if (bind(isock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        {
            ngx_log_stderr(errno, "CSocket initialize socket bind() failed");
            close(isock);
            return false;
        }

        if (listen(isock, NGX_LISTEN_BACKLOG) == -1)
        {
            ngx_log_stderr(errno, "CSocket initialize socket listen() failed");
            close(isock);
            return false;
        }

        lpngx_listening_t p_listensocketitem = new ngx_listening_t;
        memset(p_listensocketitem, 0, sizeof(ngx_listening_t));
        p_listensocketitem->port = iport;
        p_listensocketitem->fd   = isock;
        ngx_log_error_core(NGX_LOG_INFO, 0, "listening %d port success", iport);
        m_ListenSocketList.push_back(p_listensocketitem);
    }
    // should listen at least one port
    if (m_ListenSocketList.size() <= 0)
        return false;
    ngx_log_error_core(NGX_LOG_INFO, 0, "m_ListenSocketList size=%d", m_ListenSocketList.size());
    return true;
}

bool CSocket::setnonblocking(int sockfd)
{

    int nb = 1; // 0:clear 1:set
    if (ioctl(sockfd, FIONBIO, &nb) < 0)
        return false;

    return true;

    /*
      if(fcntl(sockfd,F_SETFL,(fcntl(sockfd, F_GETFL)|O_NONBLOCK))<0)

     */
}

void CSocket::ngx_close_listening_sockets()
{
    for (int i = 0; i < m_ListenPortCount; i++)
    {
        close(m_ListenSocketList[i]->fd);
        ngx_log_error_core(NGX_LOG_INFO, 0, "close listening port %d", m_ListenSocketList[i]->fd);
    }
}

int CSocket::ngx_epoll_init()
{
    // 1.epoll_create
    m_epollhandle = epoll_create(m_worker_connections);
    if (m_epollhandle == -1)
    {
        ngx_log_stderr(errno, "CSocket epoll_create failed");
        exit(2); // exit directly
    }

    m_connection_n = m_worker_connections; // read from config
    m_pconnections = new ngx_connection_t[m_connection_n];

    int                i    = m_connection_n;
    lpngx_connection_t next = NULL;
    lpngx_connection_t c    = m_pconnections; // head of connection pool

    do
    {
        i--; // from end to begin
        c[i].data          = next;
        c[i].fd            = -1;
        c[i].instance      = 1;
        c[i].iCurrsequence = 0;
        next               = &c[i];
    } while (i);
    m_pfree_connections = next; // m_pfree_connections point to the head of free connection link
    m_free_connection_n = m_connection_n;

    std::vector<lpngx_listening_t>::iterator pos;
    for (pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); ++pos)
    {
        c = ngx_get_connection((*pos)->fd);
        if (c == NULL)
        {
            ngx_log_stderr(0, "ngx_get_connection link is null %s", __FUNCTION__);
            exit(2);
        }

        c->listening       = (*pos); // bind each other
        (*pos)->connection = c;      // bind each other

        c->rhandler = &CSocket::ngx_event_accept;
        if (ngx_epoll_add_event((*pos)->fd, 1, 0, 0, EPOLL_CTL_ADD, c) == -1)
        {
            exit(2);
        }
    }
    return 1;
}

int CSocket::ngx_epoll_add_event(
    int fd, int readevent, int writeevent, uint32_t otherflag, uint32_t eventtype, lpngx_connection_t c)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));

    if (readevent == 1)
    {
        ev.events = EPOLLIN | EPOLLRDHUP; // EPOLLRDHUP client close
    } else
    {
        // do something;
    }

    if (otherflag != 0)
    {
        ev.events |= otherflag;
    }

    // the last bit of a pointer is not 1
    ev.data.ptr = (void*)((uintptr_t)c | c->instance);
    if (epoll_ctl(m_epollhandle, eventtype, fd, &ev) == -1)
    {
        ngx_log_stderr(errno, "CSocket::ngx_epoll_add_event epoll_ctl failed");
        return -1;
    }
    return 1;
}

int CSocket::ngx_epoll_process_event(int timer)
{
    int events = epoll_wait(m_epollhandle, m_events, NGX_MAX_EVENTS, timer);
    if (events == -1)
    {
        // signal interrupt
        if (errno == EINTR)
        {
            ngx_log_error_core(NGX_LOG_INFO, errno, "epoll wait failed");
            return 1;
        } else
        {
            ngx_log_error_core(NGX_LOG_ALERT, errno, "epoll wait else failed");
            return 0;
        }
    }
    if (events == 0)
    {
        if (timer != -1)
        {
            // out of time return, normal
            return 1;
        }
        ngx_log_error_core(NGX_LOG_ALERT, errno, "epoll wait -1 but out of time");
        return 0;
    }
    lpngx_connection_t c;
    uintptr_t          instance;
    uint32_t           revents;
    for (int i = 0; i < events; i++)
    {
        c        = (lpngx_connection_t)(m_events[i].data.ptr);
        instance = (uintptr_t)c & 1;
        c        = (lpngx_connection_t)((uintptr_t)c & (uintptr_t)~1);
        if (c->fd == -1)
        {
            ngx_log_error_core(NGX_LOG_DEBUG, 0, "epoll wait -1 fd = -1");
            continue;
        }

        revents = m_events[i].events;
        if (revents & (EPOLLERR | EPOLLRDHUP)) // peer close
        {
            ngx_log_stderr(errno, "EPOLLERR | EPOLLRDHUP");
            revents |= EPOLLIN | EPOLLOUT; // add these to process lately
        }

        if (revents & EPOLLIN)
        {
            (this->*(c->rhandler))(c);
        }
        if (revents & EPOLLOUT)
        {
            // do something
            ngx_log_stderr(errno, "just for test");
        }
    }
    return 1;
}
