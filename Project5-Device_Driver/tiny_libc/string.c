#include <string.h>
#include <stdint.h>

size_t strlen(const char *src)
{
    int i;
    for (i = 0; src[i] != '\0'; i++) {
    }
    return i;
}

void* memcpy(void *dest, const void *src, size_t len)
{
    uint8_t *dst = (uint8_t *)dest;
    for (; len != 0; len--) {
        *dst++ = *(uint8_t*)src++;
    }
    return dest;
}

void* memset(void *dest, int val, size_t len)
{
    uint8_t *dst = (uint8_t *)dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    for (int i = 0; i < num; ++i) {
        if (((char*)ptr1)[i] != ((char*)ptr2)[i]) {
            return ((char*)ptr1)[i] - ((char*)ptr2)[i];
        }
    }
    return 0;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return (*str1) - (*str2);
        }
        ++str1;
        ++str2;
    }
    return (*str1) - (*str2);
}

char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return tmp;
}

char *strcat(char *dest, const char *src)
{
    char *tmp = dest;

    while (*dest != '\0') { dest++; }
    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return tmp;
}

int atoi(char* src)
{
    int ret = 0;
    while(*src != '\0'){
        ret = ret * 10 + *src - '0';
        ++src;
    }
    return ret;
}