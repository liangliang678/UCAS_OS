#ifndef NET_H
#define NET_H

#include <type.h>
#include <emacps/xemacps_example.h>

long do_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength);
void do_net_send(uintptr_t addr, size_t length);
void do_net_irq_mode(int mode);

extern int net_poll_mode;
extern EthernetFrame rx_buffers[RXBD_CNT];
extern EthernetFrame tx_buffer;

#endif //NET_H
