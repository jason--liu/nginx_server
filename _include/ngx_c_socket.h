#ifndef NGX_C_SOCKET_HH_
#define NGX_C_SOCKET_HH_

#include <vector>
#include <list>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "ngx_comm.h"

#define NGX_LISTEN_BACKLOG 511
#define NGX_MAX_EVENTS 512

typedef struct ngx_listening_s  ngx_listening_t, *lpngx_listening_t;
typedef struct ngx_connection_s ngx_connection_t, *lpngx_connection_t;
typedef class CSocket           CSocket;

typedef void (CSocket::*ngx_event_handler_pt)(lpngx_connection_t c);

struct ngx_listening_s {
    int                port;       // listening port
    int                fd;         // socket fd
    lpngx_connection_t connection; // a pointer to connection pool
};

struct ngx_connection_s {
    int               fd;
    lpngx_listening_t listening;
    unsigned          instance : 1;
    uint64_t          iCurrsequence;
    struct sockaddr   s_sockaddr; // to keep peer address

    uint8_t w_ready; // ready to write flag
    uint8_t r_ready; // ready to read flag

    ngx_event_handler_pt rhandler; // read event process  function
    ngx_event_handler_pt whandler; // write event process function

    // package relative
    unsigned char curStat;                      // pkg state
    char          dataHeadInfo[_DATA_BUFSIZE_]; // to store pkg head
    char*         prevbuf;                      // point to receive buffer
    unsigned int  irecvlen;                     // how much data want to receive

    bool  ifnewrecvMem;
    char* pnewMemPointer;

    lpngx_connection_t data;
};

typedef struct _STRUC_MSG_HEADER {
    lpngx_connection_t pConn; // to keep the connection
    uint64_t           iCurrsequence;
} STRUC_MSG_HEADER, *LPSTRUC_MSG_HEADER;

class CSocket {
public:
    CSocket();
    virtual ~CSocket();

public:
    virtual bool Initialize();
    char*        outMsgRecvQueue();
    virtual void threadRecvProcFunc(char* pMsgBuf);

public:
    int ngx_epoll_init(); // init function
    int ngx_epoll_add_event(int fd, int readevent, int writeevent, uint32_t otherflag, uint32_t eventtype, lpngx_connection_t c);
    int ngx_epoll_process_event(int timer); // epoll wait and process function

private:
    void ReadConf();
    bool ngx_open_listening_sockets();
    void ngx_close_listening_sockets();
    bool setnonblocking(int sockfd); // set non-blocking socket fd

    void ngx_event_accept(lpngx_connection_t oldc);      // create new connection
    void ngx_wait_request_handler(lpngx_connection_t c); // process function for EPOLLIN
    void ngx_close_connection(lpngx_connection_t c);     // resource release

    ssize_t recvproc(lpngx_connection_t c, char* buff, ssize_t buflen); // receive from client
    void ngx_wait_request_handler_proc_p1(lpngx_connection_t c);        // handle pkg head
    void ngx_wait_request_handler_proc_plast(lpngx_connection_t c);     // handle total pkg
    void inMsgRecvQueue(char* buf, int& irmqc);                         // put received msg to queue
    void tmpoutMsgRecvQueue();
    void clearMsgRecvQueue();

    lpngx_connection_t ngx_get_connection(int isock); // get a free connection from pool
    void ngx_free_connection(lpngx_connection_t c);   // return c to connection pool

private:
    int                m_worker_connections; // max of epoll connection
    int                m_ListenPortCount;    // max listening port
    int                m_epollhandle;        // return of epoll_create
    lpngx_connection_t m_pconnections;       // head pointer of connection pool
    lpngx_connection_t m_pfree_connections;  // pointer to free connection list in pool

    int m_connection_n;      // scale of connection pool
    int m_free_connection_n; // available connection in poll

    std::vector<lpngx_listening_t> m_ListenSocketList; // listening socket queue
    struct epoll_event             m_events[NGX_MAX_EVENTS];

    size_t m_iLenPkgHeader; // sizeof(COMM_PKG_HEADER)
    size_t m_iLenMsgHeader; // sizeof(STRUC_MSG_HEADER)

    std::list<char*> m_MsgRecvQueue;
    int              m_iRecvMsgQueueCount;
    pthread_mutex_t  m_recvMessageQueueMutex;
};

#endif
