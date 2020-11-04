#include <os/mm.h>

ptr_t memCurr = FREEMEM;
LIST_HEAD(free_page_queue);

ptr_t allocPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(memCurr, PAGE_SIZE) + numPage * PAGE_SIZE;
    memCurr = ret;
    return ret;
}

void* kmalloc(size_t size)
{
    // align 4
    ptr_t ret = ROUND(memCurr, 4) + size;
    memCurr = ret;
    return (void*)ret;
}

void freePage(ptr_t baseAddr, int numPage)
{
    ;
}