#ifndef BINSEM_H
#define BINSEM_H

#define BINSEM_OP_LOCK 0 // mutex acquire
#define BINSEM_OP_UNLOCK 1 // mutex release

int binsemget(int key);
int binsemop(int binsem_id, int op);

#endif // BINSEM_H
