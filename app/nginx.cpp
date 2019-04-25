#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <errno.h>

#include "ngx_func.h"
#include "ngx_signal.h"
#include "ngx_c_conf.h"

char** g_os_argv;          // command line args
char* gp_envmem = nullptr; // point to our own env memrory
int g_environlen = 0;      // length of envmem

int main(int argc, char* const* argv)
{
    (void)argc;
    g_os_argv = (char**)argv;

    // move the environment memrory
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

    for(;;)
    {
        sleep(1); //休息1秒
        printf("休息1秒\n");
    }
    CConfig* p_config = CConfig::GetInstance();
    if (p_config->Load("nginx.conf") == false)
    {
        printf("read config failed\n");
        exit(1);
    }

    int port = p_config->GetIntDefault("ListenPort", 12);
    printf("port=%d\n", port);
    const char* pDBInfo = p_config->GetString("DBInfo");
    if (pDBInfo)
        printf("DBInfo=%s\n", pDBInfo);
    return 0;
}
