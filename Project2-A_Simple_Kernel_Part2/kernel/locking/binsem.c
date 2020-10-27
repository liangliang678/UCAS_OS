#include <os/binsem.h>
#include <os/irq.h>
#include <os/mm.h>
#include <assert.h>

binsem_node_t binsem_nodes[BINSEM_NUMBER];

// a simple hash function
static int binsem_hash(uint64_t x)
{
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ul;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebul;
    x = x ^ (x >> 31);
    return x % BINSEM_NUMBER;
}

void init_system_binsem()
{
    for(int i = 0; i < BINSEM_NUMBER; i++){
        binsem_nodes[i].status = INVALID;
    }
}

int binsem_get(int key)
{
    int id = binsem_hash((uint64_t)key);
    if(binsem_nodes[id].status == INVALID){
        init_list_head(&binsem_nodes[id].block_queue);
        binsem_nodes[id].status = UNLOCKED;
    }
    return id;
}

void binsem_op(int binsem_id, int op)
{
    disable_preempt();

    binsem_node_t *node = &binsem_nodes[binsem_id];

    if(op == BINSEM_OP_LOCK){
        if(node->status == LOCKED){
            do_block(&current_running->list, &node->block_queue);
            scheduler();
        }
        else{
            node->status = LOCKED;
        }
    }
    else if(op == BINSEM_OP_UNLOCK){
        node->status = UNLOCKED;
        if(!(list_empty(&node->block_queue))){
            // the process being unblocked will acquire the lock when running
            // LOCKED the binsem now!
            do_unblock(node->block_queue.next);
            scheduler();
            node->status = LOCKED;
        }
    }

    enable_preempt();
}