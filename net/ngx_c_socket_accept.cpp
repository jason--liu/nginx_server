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

void CSocket::ngx_event_accept(lpngx_connection_t oldc)
{
    struct sockaddr    mysockaddr;
    socklen_t          socklen;
    int                err;
    int                level;
    int                s;
    static int         use_accept4 = 1;
    lpngx_connection_t newc;

    socklen = sizeof(mysockaddr);
    do
    {
        if (use_accept4)
        {
            s = accept4(oldc->fd, &mysockaddr, &socklen, SOCK_NONBLOCK);
        } else
        {
            s = accept(oldc->fd, &mysockaddr, &socklen);
        }
        if (s == -1)
        {
            err = errno;
            if (err == EAGAIN) // accept is not ready
            {
                return;
            }
            level = NGX_LOG_ALERT;
            if (err == ECONNABORTED)
            {
                // client send RTS pkg
                level = NGX_LOG_ERR;
            } else if (err == EMFILE || err == ENFILE) // run out of fd file
            {
                level = NGX_LOG_CRIT;
            }
            ngx_log_error_core(level, errno, "ngx_event accept accept4 failed");
            if (use_accept4 && err == ENOSYS) // not support accept4?
            {
                use_accept4 = 0;
                continue;
            }
            if (err == ECONNABORTED)
            {
                // do nothing;
            }
            if (err == EMFILE || err == ENFILE)
            {
            }
            return;
        }

        newc = ngx_get_connection(s);
        if (newc == NULL)
        {
            ngx_log_error_core(NGX_LOG_ERR, errno, "ngx_event accept get connection is null");
            if (close(s) == -1)
            {
                ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_event accept close failed");
            }
            return;
        }
        memcpy(&newc->s_sockaddr, &mysockaddr, socklen);
        if (!use_accept4)
        {
            if (setnonblocking(s) == false)
            {
                ngx_close_connection(newc);
                return;
            }
        }
        newc->listening = oldc->listening;
        newc->w_ready   = 1;
        newc->rhandler  = &CSocket::ngx_wait_request_handler;
        if (ngx_epoll_add_event(s, 1, 0, 0, EPOLL_CTL_ADD, newc) == -1)
        {
            ngx_close_connection(newc);
            return;
        }
        break;
    } while (1);
    return;
}

