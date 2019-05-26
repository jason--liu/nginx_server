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

void CSocket::ngx_wait_request_handler(lpngx_connection_t c)
{

    /* ET test
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
    */

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

    return;
}
