#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

extern void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len);
extern void memset(void *dest, uint8_t val, uint32_t len);
extern int memcmp(const void *ptr1, const void* ptr2, uint32_t num);

extern int strcmp(const char *str1, const char *str2);
extern char *strcpy(char *dest, const char *src);
extern char *strcat(char *dest, const char *src);
extern int strlen(const char *src);

#endif /* STRING_H */
