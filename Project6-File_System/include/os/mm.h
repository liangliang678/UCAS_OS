#ifndef MM_H
#define MM_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Memory Management
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <type.h>
#include <pgtable.h>

/* * * * * * * * * * * * * * * * * * * * * * * 
Memory Layout
0x5000_0000 ~ 0x5020_0000       BBL
0x5020_0000 ~ 0x5040_0000       Boot
0x5040_0000 ~ 0x5140_0000       Kernel
0x5140_0000 ~ 0x5140_2000       Kernel Stack
0x5140_2000 ~ 0x5200_0000       Kernel Mem
0x5200_0000 ~ 0x5e00_0000       User Mem
0x5e00_0000 ~ 0x5f00_0000       Page Table
0x5f00_0000 ~ 0x6000_0000       FS Cache
* * * * * * * * * * * * * * * * * * * * * * */

#define PAGE_SIZE 4096
#define INIT_KERNEL_STACK   0xffffffc051400000lu    //kva
#define USER_STACK_ADDR     0xf00010000lu           //va

#define KERNEL_MEM_BEGIN    0x51402000lu            //pa
#define KERNEL_MEM_END      0x52000000lu            //pa
#define USER_MEM_BEGIN      0x52000000lu            //pa
#define USER_MEM_END        0x5e000000lu            //pa
#define PGDIR_END           0x5f000000lu            //pa
#define MEM_END             0x60000000lu            //pa

#define SWAP_BEGIN 2048

/* Rounding: only works for n = power of two */
#define ROUND(a, n)     (((((uint64_t)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n)-1))

extern ptr_t kernel_memCurr;
extern ptr_t user_memCurr;

#define FREE_PGDIR_ADDR (PGDIR_PA + 3 * PAGE_SIZE)
#define PGDIR_PAGE_NUM ((PGDIR_END - FREE_PGDIR_ADDR) / PAGE_SIZE)
typedef struct pgdir_page{
    uint8_t valid;
}pgdir_page_t;
extern pgdir_page_t pgdir_page[PGDIR_PAGE_NUM];

#define USER_MEM_SIZE (USER_MEM_END - USER_MEM_BEGIN)
#define USER_PAGE_NUM (USER_MEM_SIZE / PAGE_SIZE)
typedef struct user_page{
    uint8_t valid;
    uint8_t pin;
    PTE* pgtable;
}user_page_t;
extern user_page_t user_page[USER_PAGE_NUM];

#define SHM_PAGE_NUM 100
typedef struct shm_page{
    int count;
    uintptr_t pa;
}shm_page_t;
extern shm_page_t shm_page[SHM_PAGE_NUM];

#define DISK_PAGE_NUM 256
typedef struct disk_page{
    uint8_t valid;
}disk_page_t;
extern disk_page_t disk_page[DISK_PAGE_NUM];

extern PTE* alloc_pgdir_page();
extern void free_pgdir_page(uintptr_t addr);
extern ptr_t alloc_user_page(uint8_t pin, PTE* pgtable);
extern void free_user_page(ptr_t baseAddr);
extern void* kmalloc(size_t size);

extern ptr_t selete_page();
extern void write_to_sd(ptr_t pa);
extern unsigned long free_page_num();

extern PTE* init_page_table();
extern void free_process_user_page(PTE* pgdir);

extern uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir);

uintptr_t shm_page_get(int key);
extern void shm_page_dt(uintptr_t addr);

extern void enable_sum();
extern void disable_sum();

#endif /* MM_H */