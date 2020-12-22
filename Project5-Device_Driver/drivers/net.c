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
    enable_sum();
    memset(addr, 0, length);
    uintptr_t curr = addr;
    for(int i = 0; i < num_packet; i++){
        memset(&rx_buffers[0], 0, sizeof(EthernetFrame));
        EmacPsRecv(&EmacPsInstance, kva2pa(&rx_buffers[0]), 1);
        EmacPsWaitRecv(&EmacPsInstance, 1, frLength);
        //memcpy(curr, &rx_buffers[0], *(frLength+ i));
        //curr += frLength[i];
    }
    disable_sum();
    return 0;
}

void do_net_send(uintptr_t addr, size_t length)
{
    // send all packet
    enable_sum();
    //memset(&tx_buffer, 0, sizeof(EthernetFrame));
    memcpy(&tx_buffer, addr, length);
    EmacPsSend(&EmacPsInstance, kva2pa(&tx_buffer), length);
    EmacPsWaitSend(&EmacPsInstance);
    disable_sum();
}

void do_net_irq_mode(int mode)
{
    // TODO:
    // turn on/off network driver's interrupt mode
    net_poll_mode = mode;
}
