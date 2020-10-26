#ifndef BINSEM_H
#define BINSEM_H

#include <os/list.h>
#include <os/lock.h>
#include <type.h>

#define BINSEM_BUCKETS 100

#define BINSEM_OP_LOCK 0    // mutex acquire
#define BINSEM_OP_UNLOCK 1  // mutex release

typedef uint64_t binsem_id_t;

typedef struct binsem_node
{
    binsem_id_t id;
    lock_status_t status;
    list_node_t list;
    list_head block_queue;
}binsem_node_t;

typedef list_head binsem_bucket_t;

extern binsem_bucket_t binsem_buckets[BINSEM_BUCKETS];

extern void init_system_binsem();
extern int binsem_get(int key);
extern void binsem_op(int binsem_id, int op);

#endif /* BINSEM_H */