#ifndef SHM_H
#define SHM_H

void* shmpageget(int key);
void shmpagedt(void *addr);

#endif /* SHM_H */
