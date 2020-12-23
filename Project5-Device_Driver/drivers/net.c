#include <net.h>
#include <os/string.h>
#include <screen.h>
#include <emacps/xemacps_example.h>
#include <emacps/xemacps.h>

#include <os/sched.h>
#include <os/mm.h>

EthernetFrame rx_buffers[RXBD_CNT];
EthernetFrame tx_buffer;
uint32_t rx_len[RXBD_CNT];

int net_poll_mode;

volatile int rx_curr = 0, rx_tail = 0;

long do_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength)
{
    // receive packet by calling network driver's function
    // wait until you receive enough packets(`num_packet`).
    if(num_packet > RXBD_CNT){
        return -1;
    }

    enable_sum();
    for(int i = 0; i < num_packet; i++){
        memset(&rx_buffers[i], 0, sizeof(EthernetFrame));
        rx_len[i] = 0;
    }
    EmacPsRecv(&EmacPsInstance, 0, num_packet);
    EmacPsWaitRecv(&EmacPsInstance, num_packet, rx_len);
    rx_curr = addr;
    rx_tail = 0;
    for(int i = 0; i < num_packet; i++){
        memcpy(rx_curr, &rx_buffers[i], rx_len[i]);
        *(frLength + i) = rx_len[i];
        rx_curr += rx_len[i];
        rx_tail += rx_len[i];
    }
    disable_sum();
    return rx_tail;
}

void do_net_send(uintptr_t addr, size_t length)
{
    // send all packet
    enable_sum();
    memset(&tx_buffer, 0, sizeof(EthernetFrame));
    memcpy(&tx_buffer, addr, length);
    EmacPsSend(&EmacPsInstance, 0, length);
    EmacPsWaitSend(&EmacPsInstance);
    disable_sum();
}

void do_net_irq_mode(int mode)
{
    // turn on/off network driver's interrupt mode
    net_poll_mode = mode;
}
