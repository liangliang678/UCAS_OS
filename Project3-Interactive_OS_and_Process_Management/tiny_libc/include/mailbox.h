#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include <mthread.h>

#define MAX_MBOX_LENGTH (64)

// TODO: please define mailbox_t;
typedef struct mailbox
{
    /* data */
}mailbox_t;

// mailbox_t is just an id of kernel's mail box.

mailbox_t mbox_open(char *);
void mbox_close(mailbox_t);
void mbox_send(mailbox_t, void *, int);
void mbox_recv(mailbox_t, void *, int);

#endif
