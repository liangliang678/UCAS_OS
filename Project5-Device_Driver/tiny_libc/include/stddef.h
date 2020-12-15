#ifndef STDDEF_H
#define STDDEF_H

#include <stdint.h>

typedef int64_t ptrdiff_t;
typedef uint64_t size_t;

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define NULL ((void*)0)

#endif /* STDDEF_H */
