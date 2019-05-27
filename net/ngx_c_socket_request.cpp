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
#include "ngx_c_memory.h"

void CSocket::ngx_wait_request_handler(lpngx_connection_t c)
{
    ssize_t reco = recvproc(c, c->prevbuf, c->irecvlen);
    if (recv <= 0)
    {
        // we have handled it
        return;
    }
    if (c->curStat == _PKG_HD_INIT)
    {
        if (reco == m_iLenPkgHeader)
        {
        }
    }

#if 0
    // ET test
    unsigned char buf[10] = {0};
    memset(buf, 0, sizeof(buf));
    do
    {
        int n = recv(c->fd, buf, 2, 0);
        if (n == -1 && errno == EAGAIN)
            break; // no data
        else if (n == 0)
            break;
        ngx_log_stderr(0, "epoll ET receive %d %s", n, buf);
    } while (1);
    return;

    // LT test
    unsigned char buf[10] = {0};
    memset(buf, 0, sizeof(buf));
    int n = recv(c->fd, buf, 2, 0);
    if (n == 0)
    {
        ngx_free_connection(c);
        close(c->fd);
        c->fd = -1;
    }
    ngx_log_stderr(0, "epoll LT receive %d %s", n, buf);
#endif

    return;
}

ssize_t CSocket::recvproc(lpngx_connection_t c, char* buff, ssize_t buflen)
{
    ssize_t n;
    n = recv(c->fd, buff, buflen, 0);
    if (n == 0)
    {
        ngx_close_connection(c);
        return -1;
    }

    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ngx_log_stderr(errno, "rece function EAGAIN EWOULDBLOCK error");
            return -1;
        }
        if (errno == EINTR)
        {
            ngx_log_stderr(errno, "rece function EINTR error");
            return -1;
        }
        if (errno == ECONNRESET)
        {
            // TODO
        } else
        {
            ngx_log_stderr(errno, "rece function other errno");
        }
        ngx_close_connection(c);
        return -1;
    }
    return n;
}
void CSocket::ngx_wait_request_handler_proc_p1(lpngx_connection_t c)
{
    CMemory*          p_memory = CMemory::GetInstance();
    LPCOMM_PKG_HEADER pPkgHeader;
    pPkgHeader = (LPCOMM_PKG_HEADER)c->dataHeadInfo;

    unsigned short e_pkgLen;
    e_pkgLen = ntohs(pPkgHeader->pkgLen); // network to host

    if (e_pkgLen < m_iLenPkgHeader)
    {
        // there must something wrong, if the total pkgLen
        // is less than msg head
        c->curStat  = _PKG_HD_INIT;
        c->prevbuf  = c->dataHeadInfo;
        c->irecvlen = m_iLenPkgHeader;
    } else if (e_pkgLen > (_PKG_MAX_LENGTH - 1000))
    {
        // too large
        c->curStat  = _PKG_HD_INIT;
        c->prevbuf  = c->dataHeadInfo;
        c->irecvlen = m_iLenPkgHeader;
    } else
    {
        char* pTmpBuffer  = (char*)p_memory->AllocMemory(m_iLenMsgHeader, false);
        c->ifnewrecvMem   = true; // flag we have malloced memory
        c->pnewMemPointer = pTmpBuffer;

        // msg head
        LPSTRUC_MSG_HEADER ptmpMsgHeader = (LPSTRUC_MSG_HEADER)pTmpBuffer;
        ptmpMsgHeader->pConn             = c;
        ptmpMsgHeader->iCurrsequence     = c->iCurrsequence;
        // pkg head
        pTmpBuffer += m_iLenMsgHeader;
        memcpy(pTmpBuffer, pPkgHeader, m_iLenPkgHeader);
        if (e_pkgLen == m_iLenPkgHeader)
        {
            ngx_wait_request_handler_proc_plast(c);
        } else
        {
            c->curStat  = _PKG_BD_INIT;
            c->prevbuf  = pTmpBuffer + m_iLenPkgHeader;
            c->irecvlen = e_pkgLen - m_iLenPkgHeader;
        }
    }
    return;
}
void CSocket::ngx_wait_request_handler_proc_plast(lpngx_connection_t c)
{
    // TODO
    // inMsgRecvQueue(c->pnewMemPointer);
    c->ifnewrecvMem   = false; // let yewu luoji handle mem
    c->pnewMemPointer = NULL;
    c->curStat        = _PKG_HD_INIT;
    c->prevbuf        = c->dataHeadInfo;
    c->irecvlen       = m_iLenPkgHeader;
}
