#ifndef INCLUDE_STDIO_H_
#define INCLUDE_STDIO_H_

#include <stdarg.h>
#include <stddef.h>

extern int printf(const char *fmt, ...);
extern int vprintf(const char *fmt, va_list va);
extern char getchar();

#endif
