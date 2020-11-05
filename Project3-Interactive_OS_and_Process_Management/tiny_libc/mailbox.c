#include <mailbox.h>
#include <string.h>

mailbox_t mailbox[100];
int mailbox_p = 0;
mbox_t mbox[100];
int mbox_p = 0;


mailbox_t mbox_open(char *name)
{
    for(int i = 0; i < 100; i++){
        if(!strcmp(name, mailbox[i].name)){
            return mailbox[i];
        }
    }
    strcpy(mailbox[mailbox_p].name, name);
    mailbox[mailbox_p].id = mbox_p;
    mthread_mutex_init(&mbox[mbox_p].mutex);
    mthread_cond_init(&mbox[mbox_p].cond);
    mbox[mbox_p].msg[0] = '\0';
    mbox[mbox_p].msg_p = 0;
    mbox_p++;
    return mailbox[mailbox_p++]; 
}

void mbox_close(mailbox_t mailbox)
{
    // TODO:
    mthread_mutex_destroy(&mbox[mailbox.id].mutex);
    mthread_cond_destroy(&mbox[mailbox.id].cond);
}

void mbox_send(mailbox_t mailbox, void *msg, int msg_length)
{
    // TODO:
    mthread_mutex_lock(&mbox[mailbox.id].mutex);
    while(mbox[mailbox.id].msg_p + msg_length >= MAX_MBOX_LENGTH){
        mthread_cond_wait(&mbox[mailbox.id].cond, &mbox[mailbox.id].mutex);
    }
    memcpy(&mbox[mailbox.id].msg[mbox[mailbox.id].msg_p], msg, msg_length);
    mbox[mailbox.id].msg_p += msg_length;
    mthread_cond_signal(&mbox[mailbox.id].cond);
    mthread_mutex_unlock(&mbox[mailbox.id].mutex);
}

void mbox_recv(mailbox_t mailbox, void *msg, int msg_length)
{
    // TODO:
    mthread_mutex_lock(&mbox[mailbox.id].mutex);
    while(mbox[mailbox.id].msg_p < msg_length){
        mthread_cond_wait(&mbox[mailbox.id].cond, &mbox[mailbox.id].mutex);
    }
    mbox[mailbox.id].msg_p -= msg_length;
    memcpy(msg, &mbox[mailbox.id].msg[mbox[mailbox.id].msg_p], msg_length);
    mthread_cond_signal(&mbox[mailbox.id].cond);
    mthread_mutex_unlock(&mbox[mailbox.id].mutex);
}
