#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include <mthread.h>

#define MAX_MBOX_LENGTH (64)

// TODO: please define mailbox_t;
typedef struct mailbox
{
    char name[100];
    int id;
} mailbox_t;

typedef struct mbox
{
    char msg[MAX_MBOX_LENGTH];
    mthread_mutex_t mutex;
    mthread_cond_t cond;
    int msg_p;    
} mbox_t;

extern mailbox_t mailbox[100];
extern mbox_t mbox[100];
// mailbox_t is just an id of kernel's mail box.

mailbox_t mbox_open(char *);
void mbox_close(mailbox_t);
void mbox_send(mailbox_t, void *, int);
void mbox_recv(mailbox_t, void *, int);

#endif
