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
    // TODO: 
    // receive packet by calling network driver's function
    // wait until you receive enough packets(`num_packet`).
    // maybe you need to call drivers' receive function multiple times ?
    return 0;//ret;
}

void do_net_send(uintptr_t addr, size_t length)
{
    // TODO:
    // send all packet
    // maybe you need to call drivers' send function multiple times ?
}

void do_net_irq_mode(int mode)
{
    // TODO:
    // turn on/off network driver's interrupt mode
    net_poll_mode = mode;
}
