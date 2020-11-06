#include <os/mm.h>

ptr_t memCurr = FREEMEM;
LIST_HEAD(free_page_queue);
page_t free_page_node[MAX_FREE_PAGE_NUM];
int free_node = 0;

ptr_t allocPage(int numPage)
{
    ptr_t ret;
    if(numPage == 1 && !list_empty(&free_page_queue)){
        page_t* free_page = list_entry(free_page_queue.next, page_t, list);
        ret = free_page->baseAddr;
        list_del(free_page_queue.next);
        free_node--;
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
        free_page_node[free_node].baseAddr = baseAddr + i * PAGE_SIZE;
        list_add_tail(&free_page_node[free_node].list, &free_page_queue);
        free_node++;
    }
}