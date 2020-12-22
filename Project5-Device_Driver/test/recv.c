#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>

#include <os.h>

#define MAX_RECV_CNT 32
char recv_buffer[MAX_RECV_CNT * sizeof(EthernetFrame)];
size_t recv_length[MAX_RECV_CNT];

int main(int argc, char *argv[])
{
    int mode = 0;
    int size = 1;
    if(argc > 1) {
        size = atol(argv[1]);
    }
    if(argc > 2) {        
        if (strcmp(argv[2], "1") == 0) {
            mode = 1;
        }
    }
    sys_net_irq_mode(mode);

    sys_move_cursor(1, 1);
    //printf("[RECV TASK] start recv(%d): ", size);

    int ret = sys_net_recv(recv_buffer, size * sizeof(EthernetFrame), size, recv_length);
    //printf("%d\n", ret);
    char *curr = recv_buffer;
    for (int i = 0; i < size; ++i) {
        //printf("packet %d:\n", i);
        for (int j = 0; j < (recv_length[i] + 15) / 16; ++j) {
            for (int k = 0; k < 16 && (j * 16 + k < recv_length[i]); ++k) {
                //printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                ++curr;
            }
            //printf("\n");
        }
    }

    return 0;
}