#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <mthread.h>

#define HDR_OFFSET 54
#define SHMP_KEY 0x42
#define MAGIC 0xbeefbeefbeefbeeflu
#define FIFO_BUF_MAX 2048

#define MAX_RECV_CNT 32
char recv_buffer[MAX_RECV_CNT * sizeof(EthernetFrame)];
size_t recv_length[MAX_RECV_CNT] = {0};

const char response[] = "Response: ";

/* this should not exceed a page */
typedef struct echo_shm_vars {
    atomic_long magic_number;
    atomic_long available;
    char fifo_buffer[FIFO_BUF_MAX];
} echo_shm_vars_t;

void shm_read(char* shmbuf, atomic_long *_available, char* buf, size_t size)
{
    while (size > 0) {
        while(atomic_load_d(_available) != 1);

        int sz = size > FIFO_BUF_MAX ? FIFO_BUF_MAX : size;
        memcpy(buf, shmbuf, sz);
        size -= sz;

        atomic_exchange_d(_available, 0);
    }
}

void shm_write(char* shmbuf, atomic_long *_available, char* buf, size_t size)
{
    while (size > 0) {
        while(atomic_load_d(_available) != 0);

        int sz = size > FIFO_BUF_MAX ? FIFO_BUF_MAX : size;
        memcpy(shmbuf, buf, sz);
        size -= sz;

        atomic_exchange_d(_available, 1);
    }
}

int is_first(echo_shm_vars_t *vars)
{
    unsigned long my = atomic_exchange_d(&vars->magic_number, MAGIC);
    return my != MAGIC;
}

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
    
    echo_shm_vars_t *vars = (echo_shm_vars_t*) sys_shmpage_get(SHMP_KEY);
    if (vars == NULL) {
        sys_move_cursor(1, 1);
        printf("shmpageget failed!\n");
        return -1;
    }
    
    if (is_first(vars)) {
        sys_move_cursor(1, 1);
        printf("[ECHO SEND SIDE]\n");
        sys_net_irq_mode(mode);

        atomic_exchange_d(&vars->available, 0);
        sys_exec(argv[0], argc, argv, AUTO_CLEANUP_ON_EXIT);
        sys_move_cursor(1, 1);
        printf("[ECHO TASK] start recv(%d):                    \n", size);

        int ret = sys_net_recv(recv_buffer, size * sizeof(EthernetFrame), size, recv_length);
        shm_write(vars->fifo_buffer, &vars->available, recv_buffer, size * sizeof(EthernetFrame));
        shm_write(vars->fifo_buffer, &vars->available, recv_length, size * sizeof(size_t));
    } else {
        int print_location = 10;
        sys_move_cursor(1, print_location);
        printf("[ECHO SEND SIDE]\n");
        shm_read(vars->fifo_buffer, &vars->available, recv_buffer, size * sizeof(EthernetFrame));
        shm_read(vars->fifo_buffer, &vars->available, recv_length, size * sizeof(size_t));

        int send_num = 0;
        int i;
        char *cur = recv_buffer;
        int resp_len = strlen(response);

        for (int i = 0; i < size; ++i)
        {
            sys_move_cursor(1, print_location);
            printf("No.%d packet, recv_length[i] = %ld ...\n", i, recv_length[i]);
            memcpy(cur + HDR_OFFSET, response, resp_len);

            sys_net_send(cur, recv_length[i]);
            send_num += 1;
            sys_move_cursor(1, print_location + 1);
            printf("[ECHO TASK] Echo no.%d packets ...\n", i);
            cur += recv_length[i];
        }
    }

    sys_shmpage_dt((void*)vars);

    return 0;
}
