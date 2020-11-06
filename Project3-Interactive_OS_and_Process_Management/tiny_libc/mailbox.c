#include <mailbox.h>
#include <string.h>
#include <sys/syscall.h>

mailbox_t mailbox[MBOX_NUM];
int free_mailbox = 0;
message_t message[MBOX_NUM];
int message_status[MBOX_NUM] = {0};


mailbox_t mbox_open(char *name)
{
    for(int i = 0; i < MBOX_NUM; i++){
        if(!strcmp(name, mailbox[i].name)){
            message_t *cur_message = &message[mailbox[i].id];
            (cur_message->opned)++;
            return mailbox[i];
        }
    }

    // create a new mailbox
    int id = -1;
    for(int i = 0; i < MBOX_NUM; i++){
        if(!message_status[i]){
            id = i;
            break;
        }
    }
    mailbox_t *new_mailbox = &mailbox[free_mailbox++];
    strcpy(new_mailbox->name, name);
    new_mailbox->id = id;

    // init the message
    message_t *new_message = &message[id];
    message_status[id] = 1;
    new_message->binsem_id = sys_binsem_get(id);
    mthread_cond_init(&new_message->cond);
    new_message->opned = 1;
    new_message->msg[0] = '\0';
    new_message->msg_len = 0;

    return (*new_mailbox);
}

void mbox_close(mailbox_t mailbox)
{
    message_t *closed_message = &message[mailbox.id];
    (closed_message->opned)--;
    if(closed_message->opned == 0){
        message_status[mailbox.id] = 0;
    }
}

void mbox_send(mailbox_t mailbox, void *msg, int msg_length)
{
    message_t *cur_message = &message[mailbox.id];
    sys_binsem_op(cur_message->binsem_id, BINSEM_OP_LOCK);

    while(cur_message->msg_len + msg_length > MAX_MBOX_LENGTH){
        mthread_cond_wait(&cur_message->cond, cur_message->binsem_id);
    }
    memcpy(&(cur_message->msg[cur_message->msg_len]), msg, msg_length);
    cur_message->msg_len += msg_length;
    mthread_cond_signal(&cur_message->cond);

    sys_binsem_op(cur_message->binsem_id, BINSEM_OP_UNLOCK);
}

void mbox_recv(mailbox_t mailbox, void *msg, int msg_length)
{
    message_t *cur_message = &message[mailbox.id];
    sys_binsem_op(cur_message->binsem_id, BINSEM_OP_LOCK);

    while(cur_message->msg_len < msg_length){
        mthread_cond_wait(&cur_message->cond, cur_message->binsem_id);
    }
    cur_message->msg_len -= msg_length;
    memcpy(msg, &(cur_message->msg[cur_message->msg_len]), msg_length);
    mthread_cond_signal(&cur_message->cond);

    sys_binsem_op(cur_message->binsem_id, BINSEM_OP_UNLOCK);
}
