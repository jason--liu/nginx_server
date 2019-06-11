#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> //信号相关头文件
#include <errno.h>  //errno
#include <unistd.h>

#include "ngx_func.h"
#include "ngx_signal.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_c_conf.h"

static u_char master_process[] = "master process";

static void ngx_start_worker_process(int threadnums);
static void ngx_worker_process_cycle(int inum, const char* pprocname);
static int ngx_spawn_process(int inum, const char* pprocname);
static void ngx_worker_process_init(int inum);

void ngx_master_process_cycle()
{
    sigset_t set;
    sigemptyset(&set);

    sigaddset(&set, SIGCHLD);  //子进程状态改变
    sigaddset(&set, SIGALRM);  //定时器超时
    sigaddset(&set, SIGIO);    //异步I/O
    sigaddset(&set, SIGINT);   //终端中断符
    sigaddset(&set, SIGHUP);   //连接断开
    sigaddset(&set, SIGUSR1);  //用户定义信号
    sigaddset(&set, SIGUSR2);  //用户定义信号
    sigaddset(&set, SIGWINCH); //终端窗口大小改变
    sigaddset(&set, SIGTERM);  //终止
    sigaddset(&set, SIGQUIT);  //终端退出符

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_master_process_cycle sigprocmask failed");

    size_t size;
    int    i;
    size = sizeof(master_process); // include '\0'
    size += g_argvneedmem;
    if (size < 1000)
    {
        char title[1000] = {0};
        strcpy(title, (const char*)master_process);
        strcat(title, " ");
        for (i = 0; i < g_os_argc; i++)
        {
            strcat(title, g_os_argv[i]);
        }
        ngx_setproctile(title);
        ngx_log_error_core(NGX_LOG_NOTICE, 0, "%s %P process start and run......!", title, ngx_pid);
    }

    CConfig* p_config    = CConfig::GetInstance();
    int      workprocess = p_config->GetIntDefault("WorkProcess", 1);

    ngx_start_worker_process(workprocess);
    sigemptyset(&set);
    for (;;)
    {
        ngx_log_stderr(0, "this is parent process pid=%P", getpid());
        sigsuspend(&set);
        // sleep(1);
    }
    return;
}

static void ngx_start_worker_process(int threadnums)
{
    int i;
    for (int i = 0; i < threadnums; i++)
    {
        ngx_spawn_process(i, "worker process");
    }
    return;
}

static int ngx_spawn_process(int inum, const char* pprocname)
{
    pid_t pid;
    pid = fork();
    switch (pid)
    {
    case -1:
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx spawn process failed");
        return -1;
    case 0: // child
        ngx_parent = ngx_pid;
        ngx_pid    = getpid();
        ngx_worker_process_cycle(inum, pprocname);
        break;
    default:
        break;
    }
    return pid;
}
static void ngx_worker_process_cycle(int inum, const char* pprocname)
{
    ngx_process = NGX_PROCESS_WORKER;
    ngx_worker_process_init(inum);
    ngx_setproctile(pprocname);
    ngx_log_error_core(NGX_LOG_NOTICE, 0, "%s %P start and running", pprocname, ngx_pid);

    for (;;)
    {
        // ngx_log_stderr(0, "this is child process %d pid=%P", inum, getpid());
        ngx_process_events_and_timers();
    }
    return;
}

static void ngx_worker_process_init(int inum)
{
    sigset_t set;
    sigemptyset(&set);

    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1)
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx worker process sigprocmask failed");

    CConfig* p_config  = CConfig::GetInstance();
    int      threadNum = p_config->GetIntDefault("ProcMsgRecvWorkThreadCount", 5);
    if (g_threadpool.Create(threadNum) == false)
        exit(-2);

    g_socket.ngx_epoll_init();

    return;
}
