#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include <mthread.h>

#define MAX_MBOX_LENGTH (64)
#define MBOX_NUM 100

typedef struct mailbox
{
    char name[100];
    int id;
} mailbox_t;

typedef struct message
{
    int binsem_id;
    mthread_cond_t cond;
    int opned;
    char msg[MAX_MBOX_LENGTH];
    int msg_len;   
} message_t;

extern mailbox_t mailbox[MBOX_NUM];
extern int free_mailbox;
extern message_t message[MBOX_NUM];
extern int message_status[MBOX_NUM];

mailbox_t mbox_open(char *);
void mbox_close(mailbox_t);
void mbox_send(mailbox_t, void *, int);
void mbox_recv(mailbox_t, void *, int);

#endif
