#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>

uint32_t buffer0[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
uint32_t buffer1[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
uint32_t buffer2[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
uint32_t buffer3[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};

int len[4] = {88, 88, 88, 88};

int main(int argc, char *argv[])
{
    int mode = 0;
    if(argc > 1) {
        if (strcmp(argv[1], "1") == 0) {
            mode = 1;
        }
    }
    sys_net_irq_mode(mode);

    int i;
    uintptr_t addr[4];

    addr[0] = buffer0;
    addr[1] = buffer1;
    addr[2] = buffer2;
    addr[3] = buffer3;

    sys_move_cursor(1, 2);
    printf("> [SEND TASK] start send package.               \n");
  
    for(i = 0; i < 4; i++) {
        sys_net_send(addr[i], len[i]);
        sys_move_cursor(1, 2);
        printf("> [SEND TASK] totally send package %d !        \n", i + 1);
    }

    return 0;
}