#ifndef BINSEM_H
#define BINSEM_H

#include <os/list.h>
#include <os/lock.h>
#include <type.h>

#define BINSEM_NUMBER 100

#define BINSEM_OP_LOCK 0    // mutex acquire
#define BINSEM_OP_UNLOCK 1  // mutex release

typedef struct binsem_node
{
    lock_status_t status;
    list_head block_queue;
}binsem_node_t;

extern binsem_node_t binsem_nodes[BINSEM_NUMBER];

extern void init_system_binsem();
extern int binsem_get(int key);
extern void binsem_op(int binsem_id, int op);

#endif /* BINSEM_H */