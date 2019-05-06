#ifndef NGX_GLOBAL_HH_
#define NGX_GLOBAL_HH_

typedef struct {
    char ItemName[50];
    char ItemContent[500];

} CConfItem, *LPCConfItem;

typedef struct {
    int log_level; // log level
    int fd;        // log fd
} ngx_log_t;

extern char** g_os_argv;
extern char* gp_envmem;
extern int g_environlen;

extern pid_t ngx_pid;
extern ngx_log_t ngx_log;
#endif
