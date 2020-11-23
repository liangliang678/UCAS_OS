#ifndef INCLUDE_STDIO_H_
#define INCLUDE_STDIO_H_

#include <stdarg.h>
#include <stddef.h>

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list va);

int puts(const char *str);
int putchar(int ch);

char getchar();

#endif
