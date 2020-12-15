#include <os/string.h>

void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    for (; len != 0; len--) {
        *dest++ = *src++;
    }
}

void memset(void *dest, uint8_t val, uint32_t len)
{
    uint8_t *dst = (uint8_t *)dest;

    for (; len != 0; len--) {
        *dst++ = val;
    }
}

void bzero(void *dest, uint32_t len) { memset(dest, 0, len); }

int strlen(const char *src)
{
    int i;
    for (i = 0; src[i] != '\0'; i++) {
    }
    return i;
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
    while(*src != '\n'){
        ret = ret * 10 + *src - '0';
        ++src;
    }
    return ret;
}

void itoa(char *dest, int src)
{
    if(src == 0){
        *dest = '0';
        *(dest + 1) = '\0';
        return;
    }

    int i = 0, j = 0;
    while(src){
        *(dest + i) = src % 10 + '0';
        src /= 10; 
        i++;
    }
    *(dest + i) = '\0';
    i--;

    char tmp;
    while(i>j){
        tmp = *(dest + j);
        *(dest + j) = *(dest + i);
        *(dest + i) = tmp;
        i--;
        j++;
    }
}