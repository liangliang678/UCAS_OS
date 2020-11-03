#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>

#define RAND_MAX (INT32_MAX)

void srand(unsigned seed);
int rand();

#endif /* STDLIB_H */
