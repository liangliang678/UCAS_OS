#include <os/mailbox.h>
#include <string.h>
#include <os/sched.h>
#include <os/irq.h>

mailbox_t mailbox[MBOX_NUM];
int free_mailbox = 0;
message_t message[MBOX_NUM];
int message_status[MBOX_NUM] = {0};

int do_mbox_open(char *name)
{
    for(int i = 0; i < MBOX_NUM; i++){
        if(!kstrcmp(name, mailbox[i].name)){
            message_t *cur_message = &message[mailbox[i].id];
            (cur_message->opned)++;
            return mailbox[i].id;
        }
    }

    // create a new mailbox
    int id;
    for(int i = 0; i < MBOX_NUM; i++){
        if(!message_status[i]){
            id = i;
            break;
        }
    }
    mailbox_t *new_mailbox = &mailbox[free_mailbox++];
    kstrcpy(new_mailbox->name, name);
    new_mailbox->id = id;

    // init the message
    message_t *new_message = &message[id];
    message_status[id] = 1;
    init_list_head(&new_message->wait_queue);
    new_message->opned = 1;
    new_message->msg[0] = '\0';
    new_message->msg_len = 0;

    return new_mailbox->id;
}

void do_mbox_close(int mailbox_id)
{
    message_t *closed_message = &message[mailbox_id];
    (closed_message->opned)--;
    if(closed_message->opned == 0){
        message_status[mailbox_id] = 0;
    }
}

int do_mbox_send(int mailbox_id, void *msg, int msg_length)
{
    message_t *cur_message = &message[mailbox_id];
    if(cur_message->msg_len + msg_length >= MAX_MBOX_LENGTH){
        do_block(&current_running[cpu_id]->list, &cur_message->wait_queue);
        scheduler();
        return 0;
    }
    else{
        kmemcpy(&(cur_message->msg[cur_message->msg_len]), msg, msg_length);
        cur_message->msg_len += msg_length;
        while(!list_empty(&cur_message->wait_queue)){
            do_unblock(cur_message->wait_queue.next);
        }
        scheduler();
        return 1;
    }
}

int do_mbox_recv(int mailbox_id, void *msg, int msg_length)
{
    message_t *cur_message = &message[mailbox_id];
    if(msg_length > cur_message->msg_len){
        do_block(&current_running[cpu_id]->list, &cur_message->wait_queue);
        scheduler();
        return 0;
    }
    else{
        cur_message->msg_len -= msg_length;
        kmemcpy(msg, &(cur_message->msg[cur_message->msg_len]), msg_length);
        while(!list_empty(&cur_message->wait_queue)){
            do_unblock(cur_message->wait_queue.next);
        }
        scheduler();
        return 1;
    }
}
