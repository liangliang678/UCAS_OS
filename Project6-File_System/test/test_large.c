#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

#include <os.h>

char* buff = 0x20000;

int main(int argc, char *argv[])
{
    int i;
    int size = 1;
    if(argc > 1) {
        size = atol(argv[1]);
    }
    sys_move_cursor(1, 1);
    
    char msg[] = "2018K8009929048-wujunliang-OS-Project6-FileSystem-TestLargeFile-YouCanControlSizeOfDataByShellInput\0"; 
    buff[0] = 0;

    int fd = sys_fopen("2.txt", O_RDWR);
    for (i = 0; i < size; i++)
    {
        memcpy(buff + 100 * i, msg, 100);
    }
    printf("Write Start\n");
    sys_fwrite(fd, buff, 100 * size);
    printf("Totally Write %d * 100 Byte Data\n", size);

    // read
    printf("Read Start\n");
    memset(buff, 0, size * 100);
    sys_fread(fd, buff, 100 * size);
    for(i = 0; i < size; i++){
        if(strcmp(buff + 100 * i, msg)){
            printf("Error: Num %d Time Read\n", i);
            break;
        }
    }
    if(i == size){
        printf("Test Pass!\n");
    }

    sys_fclose(fd);
}