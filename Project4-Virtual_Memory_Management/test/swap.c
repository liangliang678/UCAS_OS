#include <stdio.h>
#include <sys/syscall.h>

#define LENGTH (4096 * 8)
int main()
{
    int flag = 1;
    unsigned long *array = 0x20000000;
    for(int i = 0; i < LENGTH; i++){
        *(array + i) = i;
    }
    for(int i = 0; i < LENGTH; i++){
        if(*(array + i) != i){
            flag = 0;
        }
    }

    if(flag){
        sys_move_cursor(1, 1);
        printf("Success!");
    }
    else{
        sys_move_cursor(1, 1);
        printf("Failed!");
    }
}
