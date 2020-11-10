#include <mailbox.h>
#include <sys/syscall.h>

int mbox_open(char *name)
{
    return sys_mbox_open(name);
}

void mbox_close(int mailbox_id)
{
    sys_mbox_close(mailbox_id);
}

void mbox_send(int mailbox_id, void *msg, int msg_length)
{
    while(!sys_mbox_send(mailbox_id, msg, msg_length)){
        ;
    }
}

void mbox_recv(int mailbox_id, void *msg, int msg_length)
{
    while(!sys_mbox_recv(mailbox_id, msg, msg_length)){
        ;
    }
}
