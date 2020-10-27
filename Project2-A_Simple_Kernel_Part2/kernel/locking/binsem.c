#include <os/binsem.h>
#include <os/irq.h>
#include <os/mm.h>
#include <assert.h>

binsem_bucket_t binsem_buckets[BINSEM_BUCKETS];

void init_system_binsem()
{
    disable_preempt();
    for (int i = 0; i < BINSEM_BUCKETS; ++i) {
        init_list_head(&binsem_buckets[i]);
    }
    enable_preempt();
}

// a simple hash function
static int binsem_hash(uint64_t x)
{
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ul;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebul;
    x = x ^ (x >> 31);
    return x % BINSEM_BUCKETS;
}

//if this key has a node, return it; else create a node
static binsem_node_t* get_node(int key)
{
    int binsem_id = binsem_hash((uint64_t)key);
    list_node_t *head = &binsem_buckets[binsem_id];
    for (list_node_t *p = head->next; p != head; p = p->next) {
        binsem_node_t *node = list_entry(p, binsem_node_t, list);
        if (node->id == (uint64_t)binsem_id) {
            return node;
        }
    }

    binsem_node_t *node = (binsem_node_t*) kmalloc(sizeof(binsem_node_t));
    node->id = (uint64_t)binsem_id;
    node->status = UNLOCKED;
    init_list_head(&node->block_queue);
    list_add_tail(&node->list, &binsem_buckets[binsem_id]);
    return node;
}

int binsem_get(int key)
{
    binsem_node_t *node = get_node(key);
    return node->id;
}

void binsem_op(int binsem_id, int op)
{
    disable_preempt();
    
    list_node_t *head = &binsem_buckets[binsem_id];
    list_node_t *p;
    binsem_node_t *node;
    for (p = head->next; p != head; p = p->next) {
        node = list_entry(p, binsem_node_t, list);
        if (node->id == (uint64_t)binsem_id) {
            break;
        }
    }
    assert(p != head);

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