#ifndef __NGX_FUNC_H__
#define __NGX_FUNC_H__
//函数声明
typedef unsigned char u_char;

// string functions
void Rtrim(char* string);
void Ltrim(char* string);

// process title functions
void ngx_init_setproctitle();
void ngx_setproctile(const char* title);

// log relative
void ngx_log_init();
void ngx_log_stderr(int err, const char* fmt, ...);
void ngx_log_error_core(int level, int err, const char* fmt, ...);

u_char* ngx_log_errno(u_char* buf, u_char* last, int err);
u_char* ngx_slprintf(u_char* buf, u_char* last, const char* fmt, ...);
u_char* ngx_vslprintf(u_char* buf, u_char* last, const char* fmt, va_list args);

// signal
int ngx_init_signals();


void ngx_master_process_cycle();
int ngx_daemon();
#endif
