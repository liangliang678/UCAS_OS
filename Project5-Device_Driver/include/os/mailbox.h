#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#define MAX_MBOX_LENGTH (64)
#define MBOX_NUM 100

#include <os/list.h>

typedef struct mailbox
{
    char name[100];
    int id;
} mailbox_t;

typedef struct message
{
    list_head wait_queue;
    int opned;
    char msg[MAX_MBOX_LENGTH];
    int msg_len;   
} message_t;

extern mailbox_t mailbox[MBOX_NUM];
extern int free_mailbox;
extern message_t message[MBOX_NUM];
extern int message_status[MBOX_NUM];

int do_mbox_open(char *);
void do_mbox_close(int);
int do_mbox_send(int, void *, int);
int do_mbox_recv(int, void *, int);

#endif
