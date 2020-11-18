#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

extern void* memcpy(void *dest, const void *src, size_t len);
extern void* memset(void *dest, int val, size_t len);
extern int memcmp(const void *ptr1, const void* ptr2, size_t num);

extern int strcmp(const char *str1, const char *str2);
extern char *strcpy(char *dest, const char *src);
extern char *strcat(char *dest, const char *src);
extern size_t strlen(const char *src);

#endif /* STRING_H */
