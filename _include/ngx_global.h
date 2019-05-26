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

extern size_t g_argvneedmem; // argv need memrory
extern size_t g_envneedmem;  // env need memrory

extern char** g_os_argv;
extern char*  gp_envmem;
extern int    g_os_argc;

extern pid_t     ngx_pid;
extern ngx_log_t ngx_log;

// process
extern pid_t ngx_pid;      // current process id
extern pid_t ngx_parent;   // parent process id
extern int   ngx_process;  // type of process
extern int   g_daemonized; // daemon process flag
#endif
