#include <net.h>
#include <emacps/xemacps_example.h>

#define MAX_RECV_CNT 32
char recv_buffer[MAX_RECV_CNT * sizeof(EthernetFrame)];
size_t recv_length[MAX_RECV_CNT];

int recv_s(int argc, char *argv[])
{
    int mode = 0;
    int size = 1;

    do_net_irq_mode(mode);

    vt100_move_cursor(1, 1);
    printk("[RECV TASK] start recv(%d):                    ", size);

    int ret = do_net_recv(recv_buffer, size * sizeof(EthernetFrame), size, recv_length);
    printk("%d\n", ret);
    char *curr = recv_buffer;
    for (int i = 0; i < size; ++i) {
        printk("packet %d:\n", i);
        for (int j = 0; j < (recv_length[i] + 15) / 16; ++j) {
            for (int k = 0; k < 16 && (j * 16 + k < recv_length[i]); ++k) {
                printk("%02x ", (uint32_t)(*(uint8_t*)curr));
                ++curr;
            }
            printk("\n");
        }
    }

    return 0;
}
