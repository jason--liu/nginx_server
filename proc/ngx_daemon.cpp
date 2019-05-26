#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> //errno
#include <sys/stat.h>
#include <fcntl.h>

#include "ngx_func.h"
#include "ngx_macro.h"
#include "ngx_c_conf.h"

int ngx_daemon()
{
    switch (fork())
    {
    case -1:
        ngx_log_error_core(NGX_LOG_EMERG, errno, "create daemon failed");
        return -1;
    case 0:
        // child process
        break;
    default:
        // father process return to recycle resource
        return 1;
    }
    ngx_parent = ngx_pid;
    ngx_pid    = getpid();

    if (setsid() == -1)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "daemon setsid failed");
        return -1;
    }

    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if (fd < 0)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "daemon open\"/dev/null\"");

        return -1;
    }

    /*redirect SDTIN to null*/
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "daemon dup2(STDIN)");
        return -1;
    }

    /*redirect SDTOUT to null*/
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "daemon dup2(STDOUT)");
        return -1;
    }
    /*fd may be 3*/
    if (fd > STDERR_FILENO)
    {
        if (close(fd) == -1)
        {

            ngx_log_error_core(NGX_LOG_EMERG, errno, "daemon close fd failed");
            return -1;
        }
    }
    return 0;
}
