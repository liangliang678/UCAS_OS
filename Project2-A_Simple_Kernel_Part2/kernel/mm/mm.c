#include <os/mm.h>

ptr_t memCurr = FREEMEM;
ptr_t allocPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(memCurr, PAGE_SIZE) + numPage * PAGE_SIZE;
    memCurr = ret;
    return ret;
}

void* kmalloc(size_t size)
{
    ptr_t ret = ROUND(memCurr, 4) + size;
    memCurr = ret;
    return (void*)ret;
}