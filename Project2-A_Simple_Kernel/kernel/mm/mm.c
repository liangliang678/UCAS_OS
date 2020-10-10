#include <os/mm.h>

ptr_t memCurr = FREE_MEM;
ptr_t allocPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(memCurr, PAGE_SIZE);
    memCurr = ret + numPage * PAGE_SIZE;
    return ret;
}

void* kmalloc(size_t size)
{
    ptr_t ret = ROUND(memCurr, 4);
    memCurr = ret + size;
    return (void*)ret;
}

ptr_t kernel_stack_pointer = FREE_KERNEL_MEM;
ptr_t new_kernel_stack()
{
    kernel_stack_pointer += PAGE_SIZE;
    return kernel_stack_pointer;
}

ptr_t user_stack_pointer = FREE_MEM;
ptr_t new_user_stack()
{
    user_stack_pointer += PAGE_SIZE;
    return user_stack_pointer;
}
