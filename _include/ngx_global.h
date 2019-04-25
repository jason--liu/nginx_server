#ifndef NGX_GLOBAL_HH_
#define NGX_GLOBAL_HH_

typedef struct {
    char ItemName[50];
    char ItemContent[500];

} CConfItem, *LPCConfItem;

extern char** g_os_argv;
extern char* gp_envmem;
extern int g_environlen;

#endif
