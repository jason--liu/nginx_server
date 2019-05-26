#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h> //waitpid

#include "ngx_global.h"
#include "ngx_macro.h"
#include "ngx_func.h"

typedef struct {
    int         signo;
    const char* signame;
    void (*handler)(int signo, siginfo_t* siginfo, void* ucontext);
} ngx_signal_t;

static void ngx_signal_handler(int signo, siginfo_t* siginfo, void* ucontext);
static void ngx_process_get_status();

ngx_signal_t signals[] = {
    {SIGHUP, "SIGHUP", ngx_signal_handler},   // terminal disconnect, often used to notify daemon to reload config
    {SIGINT, "SIGINT", ngx_signal_handler},   // 2
    {SIGCHLD, "SIGCHLD", ngx_signal_handler}, // child exit, father received
    {SIGTERM, "SIGTERM", ngx_signal_handler}, // 15
    {SIGQUIT, "SIGQUIT", ngx_signal_handler}, // 3
    {SIGIO, "SIGIO", ngx_signal_handler},     // async IO
    {SIGSYS, "SIGSYS, SIG_INT", NULL},        // invalid syscall, ignore this in case OS kill our process
    // ... add another signal
    {0, NULL, NULL} // end flag
};

int ngx_init_signals()
{
    ngx_signal_t*    sig;
    struct sigaction sa;
    for (sig = signals; sig->signo != 0; sig++)
    {
        memset(&sa, 0, sizeof(struct sigaction));
        if (sig->handler)
        {
            sa.sa_sigaction = sig->handler;
            sa.sa_flags     = SA_SIGINFO;
        } else
        {
            sa.sa_handler = SIG_IGN;
        }

        sigemptyset(&sa.sa_mask);

        if (sigaction(sig->signo, &sa, NULL) == -1)
        {
            ngx_log_error_core(NGX_LOG_EMERG, errno, "sigaction(%s) failed", sig->signame);
            return -1;
        } else
        {
            // do nothing succeed
        }
    }
    return 0;
}

static void ngx_signal_handler(int signo, siginfo_t* siginfo, void* ucontext)
{
    ngx_signal_t* sig;
    char*         action;
    (void)ucontext;

    for (sig = signals; sig->signo != 0; sig++)
    {
        if (sig->signo == signo)
            break;
    }

    action = (char*)""; // do nothing at this moment

    if (ngx_process == NGX_PROCESS_MASTER)
    {
        switch (signo)
        {
        case SIGCHLD:
            break;
        default:
            break;
        }
    } else if (ngx_process == NGX_PROCESS_WORKER)
    {
        // worker process
    } else
    { // do nothing}
    }

    if (siginfo && siginfo->si_pid)
    {
        ngx_log_error_core(
            NGX_LOG_NOTICE, 0, "signal %d (%s) received from %P %s", signo, sig->signame, siginfo->si_pid, action);
    } else
    {
        ngx_log_error_core(NGX_LOG_NOTICE, 0, "signal %d (%s) received from %s", signo, sig->signame, action);
    }

    if (signo == SIGCHLD)
        ngx_process_get_status();
    return;
}

static void ngx_process_get_status()
{
    pid_t pid;
    int   status;
    int   err;
    int   one = 0;

    for (;;)
    {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid == 0)
            return;
        if (pid == -1)
        {
            err = errno;
            if (err == EINTR)
                continue;
            if (err == ECHILD && one)
                return;
            if (err == ECHILD)
            {
                ngx_log_error_core(NGX_LOG_INFO, err, "waitpid() error!");
                return;
            }
            ngx_log_error_core(NGX_LOG_ALERT, err, "waitpid() error!");
            return;
        }

        //走到这里，表示  成功【返回进程id】 ，这里根据官方写法，打印一些日志来记录子进程的退出
        one = 1;              //标记waitpid()返回了正常的返回值
        if (WTERMSIG(status)) //获取使子进程终止的信号编号
        {
            ngx_log_error_core(
                NGX_LOG_ALERT, 0, "pid = %P exited on signal %d!", pid, WTERMSIG(status)); //获取使子进程终止的信号编号
        } else
        {
            ngx_log_error_core(NGX_LOG_NOTICE, 0, "pid = %P exited with code %d!", pid,
                WEXITSTATUS(status)); // WEXITSTATUS()获取子进程传递给exit或者_exit参数的低八位
        }
    }
}
