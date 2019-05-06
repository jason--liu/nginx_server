#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <errno.h>

#include "ngx_func.h"
#include "ngx_signal.h"
#include "ngx_c_conf.h"

static void freeresouce();

pid_t ngx_pid;

char** g_os_argv;       // command line args
char* gp_envmem = NULL; // point to our own env memrory
int g_environlen = 0;   // length of envmem

int main(int argc, char* const* argv)
{
    (void)argc;
    // 1.
    g_os_argv = (char**)argv;
    ngx_pid = getpid();

    // 2.read config first
    CConfig* p_config = CConfig::GetInstance();
    if (p_config->Load("nginx.conf") == false)
    {
        ngx_log_stderr(0, "read config failed\n");
        goto lblexit;
    }

    // 3.some init functions
    ngx_log_init();

    // 4.other modules move the environment memrory
    ngx_init_setproctitle();
    ngx_setproctile("nginx: new title");

#if 0
    for (int i = 0; argv[i]; i++)
    {
        printf("argv[%d]address=%x       \n", i, (unsigned int)((unsigned long)&(argv[i])));
        printf("argv[%d]content=%s\n", i, argv[i]);
    }

    for (int i = 0; environ[i]; i++)
    {
        printf("environ[%d]address=%x      \n", i, (unsigned int)((unsigned long)&(environ[i])));
        printf("environ[%d]content=%s\n", i, environ[i]);
    }
#endif

    // for (;;)
    // {
    //     sleep(1); //休息1秒
    //     printf("休息1秒\n");
    // }

    return 0;
lblexit:
    freeresouce();
    return 0;
}

static void freeresouce()
{
    if (gp_envmem)
    {
        delete[] gp_envmem;
        gp_envmem = NULL;
    }

    if (ngx_log.fd != STDERR_FILENO && ngx_log.fd != -1)
    {
        close(ngx_log.fd);
        ngx_log.fd = -1;
    }
}
