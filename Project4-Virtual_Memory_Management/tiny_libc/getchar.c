#include <stdio.h>
#include <sys/syscall.h>

char getchar()
{
    int input;
    while((input = sys_get_char()) == -1)
        ;
    return input;
}