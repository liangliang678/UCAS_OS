#include <net.h>
#include <os/string.h>
#include <screen.h>
#include <emacps/xemacps_example.h>
#include <emacps/xemacps.h>
#include <assert.h>
#include <os/sched.h>
#include <os/mm.h>

EthernetFrame rx_buffers[RXBD_CNT];
EthernetFrame tx_buffer;
uint32_t rx_len[RXBD_CNT];

int net_poll_mode;

LIST_HEAD(net_recv_queue);
LIST_HEAD(net_send_queue);

uintptr_t rx_curr = 0, rx_tail = 0;
int recv_transaction = 0,  send_transaction = 0;

uintptr_t _addr;
int _num_packet;
size_t* _frLength;

long do_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength)
{
    // receive packet by calling network driver's function
    // wait until you receive enough packets(`num_packet`).
    if(num_packet > RXBD_CNT){
        return -1;
    }
    recv_transaction = 1;
    enable_sum();
    for(int i = 0; i < num_packet; i++){
        memset(&rx_buffers[i], 0, sizeof(EthernetFrame));
        rx_len[i] = 0;
    }
    EmacPsRecv(&EmacPsInstance, 0, num_packet);

    if(net_poll_mode == 0){
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
        recv_transaction = 0;
        return rx_tail;
    }
    else if(net_poll_mode == 1){
        _addr = get_kva_of(addr, current_running[cpu_id]->pgdir);
        _num_packet = num_packet;
        _frLength = get_kva_of(frLength, current_running[cpu_id]->pgdir);
        do_block(&current_running[cpu_id]->list, &net_recv_queue);
        disable_sum();
        return 0;
    }
}

void do_net_send(uintptr_t addr, size_t length)
{
    // send all packet
    send_transaction = 1;
    enable_sum();
    memset(&tx_buffer, 0, sizeof(EthernetFrame));
    memcpy(&tx_buffer, addr, length);
    EmacPsSend(&EmacPsInstance, 0, length);
    if(net_poll_mode == 0){        
        EmacPsWaitSend(&EmacPsInstance);
        send_transaction = 0;
    }
    else if(net_poll_mode == 1){        
        do_block(&current_running[cpu_id]->list, &net_send_queue);
    }
    disable_sum();
}

void do_net_irq_mode(int mode)
{
    // turn on/off network driver's interrupt mode
    if(net_poll_mode != mode){
        assert(recv_transaction == 0);
        assert(send_transaction == 0);
        net_poll_mode = mode;
        if(net_poll_mode == 1){
            XEmacPs_IntEnable(&EmacPsInstance, (XEMACPS_IXR_TX_ERR_MASK | XEMACPS_IXR_RX_ERR_MASK |
                            (u32)XEMACPS_IXR_FRAMERX_MASK | (u32)XEMACPS_IXR_TXCOMPL_MASK));
        }
        else if(net_poll_mode == 0){
            XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_IER_OFFSET, 0);
        }
    }
}
