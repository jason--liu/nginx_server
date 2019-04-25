#ifndef __NGX_FUNC_H__
#define __NGX_FUNC_H__

//函数声明

// string functions
void Rtrim(char* string);
void Ltrim(char* string);

// process title functions
void ngx_init_setproctitle();
void ngx_setproctile(const char* title);

#endif
