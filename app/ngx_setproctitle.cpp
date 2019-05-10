#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ngx_global.h"

/**
 * @ copy environment memrory to new memrory
 * @author jason
 * @param [in|out]
 * @return
 * @date
 */
void ngx_init_setproctitle()
{
    int i;

    gp_envmem = new char[g_envneedmem];
    memset(gp_envmem, 0, g_envneedmem);

    char* ptmp = gp_envmem;

    for (i = 0; environ[i]; i++)
    {
        size_t size = strlen(environ[i]) + 1;
        strcpy(ptmp, environ[i]);
        environ[i] = ptmp;
        ptmp += size;
    }
    return;
}

void ngx_setproctile(const char* title)
{
    size_t ititlelen = strlen(title);

    size_t esy = g_envneedmem + g_argvneedmem; // argv+environ length
    if (esy <= ititlelen)
    {
        // title is too long ,return
        return;
    }

    g_os_argv[1] = NULL;

    char* ptmp = g_os_argv[0];
    strcpy(ptmp, title);
    ptmp += ititlelen;

    // clean the origin environ and the rest command args
    size_t cha = esy - ititlelen;
    memset(ptmp, 0, cha);
    return;
}
