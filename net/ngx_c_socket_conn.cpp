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

lpngx_connection_t CSocket::ngx_get_connection(int isock)
{
    lpngx_connection_t c = m_pfree_connections; // head of free link
    if (c == NULL)
    {
        ngx_log_stderr(0, "ngx_get_connection link is null %s:%d", __FUNCTION__, __LINE__);
        return NULL;
    }
    m_pfree_connections = c->data; // move head to next free link
    m_free_connection_n--;

    uintptr_t instance      = c->instance;
    uint64_t  iCurrsequence = c->iCurrsequence;

    memset(c, 0, sizeof(ngx_connection_t));
    c->fd             = isock;
    c->curStat        = _PKG_HD_INIT;            // receive head first
    c->prevbuf        = c->dataHeadInfo;         // restore pkg head
    c->irecvlen       = sizeof(COMM_PKG_HEADER); // head of pkg
    c->ifnewrecvMem   = false;                   // we didn't malloc memory yet
    c->pnewMemPointer = NULL;
    c->instance       = !instance;
    c->iCurrsequence  = iCurrsequence;
    ++c->iCurrsequence;
    return c;
}

void CSocket::ngx_free_connection(lpngx_connection_t c)
{
    // is it ok?
    c->data = m_pfree_connections;
    ++c->iCurrsequence;
    m_pfree_connections = c;
    ++m_free_connection_n;
    return;
}

void CSocket::ngx_close_connection(lpngx_connection_t c)
{
    int fd = c->fd;
    if (close(fd) == -1)
    {
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_close_accepted_connection close failed");
    }
    c->fd = -1;
    ngx_free_connection(c);
    return;
}
