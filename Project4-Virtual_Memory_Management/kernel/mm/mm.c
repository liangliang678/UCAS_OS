#include <os/list.h>
#include <os/mm.h>
#include <os/sched.h>
#include <pgtable.h>
#include <os/string.h>
#include <os/sched.h>
#include <os/irq.h>
#include <assert.h>

ptr_t memCurr = FREEMEM;
ptr_t pgdirCurr = PGDIR_PA + 2 * PAGE_SIZE;

static LIST_HEAD(freePageList);

PTE* init_page_table()
{
    PTE *pgdir = (PTE*)pgdirCurr;
    clear_pgdir(pa2kva(pgdir));
    pgdirCurr += PAGE_SIZE;

    PTE *second_level_pgdir_1 = (PTE*)pgdirCurr;
    clear_pgdir(pa2kva(second_level_pgdir_1));
    pgdirCurr += PAGE_SIZE;
    
    PTE *second_level_pgdir_2 = (PTE*)pgdirCurr;
    clear_pgdir(pa2kva(second_level_pgdir_2));
    pgdirCurr += PAGE_SIZE;

    PTE *last_level_pgdir_1 = (PTE*)pgdirCurr;
    clear_pgdir(pa2kva(last_level_pgdir_1));
    pgdirCurr += PAGE_SIZE;
    PTE *last_level_pgdir_2 = (PTE*)pgdirCurr;
    clear_pgdir(pa2kva(last_level_pgdir_2));
    pgdirCurr += PAGE_SIZE;

    *((PTE*)pa2kva(pgdir) + 0x03c) = (((uint64_t)second_level_pgdir_1 >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
    *((PTE*)pa2kva(pgdir) + 0x000) = (((uint64_t)second_level_pgdir_2 >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;

    *((PTE*)pa2kva(second_level_pgdir_1) + 0x000) = (((uint64_t)last_level_pgdir_1 >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
    *((PTE*)pa2kva(second_level_pgdir_2) + 0x000) = (((uint64_t)last_level_pgdir_2 >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;

    *((PTE*)pa2kva(last_level_pgdir_1) + 0x00f) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;

    *((PTE*)pa2kva(last_level_pgdir_2) + 0x010) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x011) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;    
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x012) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x013) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT; 
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x014) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;  
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x015) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;                                                                                               
    share_pgtable(pgdir, PGDIR_PA);

    return pgdir;
}

ptr_t allocPage()
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(memCurr, PAGE_SIZE);
    memCurr += PAGE_SIZE;
    return ret;
}

void freePage(ptr_t baseAddr)
{
    // TODO:
}

void *kmalloc(size_t size)
{
    // TODO(if you need):
}

uintptr_t shm_page_get(int key)
{
    // TODO(c-core):
}

void shm_page_dt(uintptr_t addr)
{
    // TODO(c-core):
}

/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TODO:
    PTE *pgdir = (PTE*)dest_pgdir;
    *((PTE*)pa2kva(pgdir) + 0x101) = (((PGDIR_PA + NORMAL_PAGE_SIZE) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
}

/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page.
   */
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir)
{
    // TODO:
}

void handle_page_fault(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    PTE* pgdir = current_running[cpu_id]->pgdir;
    int VPN2 = (stval >> 30) & 0x1ff;
    int VPN1 = (stval >> 21) & 0x1ff;
    int VPN0 = (stval >> 12) & 0x1ff;

    if(*((PTE*)pa2kva(pgdir) + VPN2) & _PAGE_PRESENT){
        PTE* second_level_pgdir = get_pfn(*((PTE*)pa2kva(pgdir) + VPN2)) << NORMAL_PAGE_SHIFT;
        if(*((PTE*)pa2kva(second_level_pgdir) + VPN1) & _PAGE_PRESENT){
            PTE* last_level_pgdir = get_pfn(*((PTE*)pa2kva(second_level_pgdir) + VPN1)) << NORMAL_PAGE_SHIFT;
            *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                            _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
        }
        else{
            PTE *last_level_pgdir = (PTE*)pgdirCurr;
            clear_pgdir(pa2kva(last_level_pgdir));
            pgdirCurr += PAGE_SIZE;

            *((PTE*)pa2kva(second_level_pgdir) + VPN1) = (((uint64_t)last_level_pgdir >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
            *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                            _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
        }
    }
    else{
        PTE *second_level_pgdir = (PTE*)pgdirCurr;
        clear_pgdir(pa2kva(second_level_pgdir));
        pgdirCurr += PAGE_SIZE;
        PTE *last_level_pgdir = (PTE*)pgdirCurr;
        clear_pgdir(pa2kva(last_level_pgdir));
        pgdirCurr += PAGE_SIZE;

        *((PTE*)pa2kva(pgdir) + VPN2) = (((uint64_t)second_level_pgdir >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
        *((PTE*)pa2kva(second_level_pgdir) + VPN1) = (((uint64_t)last_level_pgdir >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
        *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((allocPage() >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                        _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
    }
}