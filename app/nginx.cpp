#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <errno.h>

#include "ngx_func.h"
#include "ngx_signal.h"
#include "ngx_c_conf.h"

int main(int argc, char* const* argv)
{
    // myconf();
    (void)argc;
    (void)argv;
    // mysignal();

    /*for(;;)
    {
        sleep(1); //休息1秒
        printf("休息1秒\n");
    }*/
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
