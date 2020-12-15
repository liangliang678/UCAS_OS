#include <os/binsem.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/smp.h>
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
        binsem_nodes[i].sem = 1;
        init_list_head(&binsem_nodes[i].block_queue);
    }
}

int binsem_get(int key)
{ 
    return binsem_hash((uint64_t)key);
}

void binsem_op(int binsem_id, int op)
{
    disable_preempt();

    binsem_node_t *node = &binsem_nodes[binsem_id];

    if(op == BINSEM_OP_LOCK){
        node->sem--;
        if(node->sem < 0){
            do_block(&current_running[cpu_id]->list, &node->block_queue);
            scheduler();
        }
        else{
            current_running[cpu_id]->binsem_id[current_running[cpu_id]->binsem_num] = binsem_id;
            current_running[cpu_id]->binsem_num++;
        }
    }
    else if(op == BINSEM_OP_UNLOCK){
        node->sem++;
        current_running[cpu_id]->binsem_num--;
        if(node->sem <= 0){
            list_node_t * unblocked_pcb_list = node->block_queue.next;
            pcb_t *unblocked_pcb = list_entry(unblocked_pcb_list, pcb_t, list);
            unblocked_pcb->binsem_id[unblocked_pcb->binsem_num] = binsem_id;
            unblocked_pcb->binsem_num++; 
            do_unblock(unblocked_pcb_list); 
            scheduler();        
        }
    }

    enable_preempt();
}