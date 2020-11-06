#include <os/mm.h>

ptr_t memCurr = FREEMEM;

ptr_t free_page[MAX_FREE_PAGE_NUM];
int free_page_num = 0;

ptr_t allocPage(int numPage)
{
    ptr_t ret;
    if(numPage == 1 && free_page_num){
        ret = free_page[--free_page_num];
    }
    else{
        // align PAGE_SIZE
        ret = ROUND(memCurr, PAGE_SIZE) + numPage * PAGE_SIZE;
        memCurr = ret;
    }
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
    for(int i = 0; i < numPage; i++){
        free_page[free_page_num++] = baseAddr + i * PAGE_SIZE;
    }
}