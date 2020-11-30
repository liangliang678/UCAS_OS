#include <os/list.h>
#include <os/mm.h>
#include <os/sched.h>
#include <pgtable.h>
#include <os/string.h>
#include <os/sched.h>
#include <os/irq.h>
#include <assert.h>

ptr_t kernel_memCurr = KERNEL_MEM_BEGIN;
ptr_t user_memCurr = USER_MEM_BEGIN;
ptr_t pgdirCurr = PGDIR_PA + 2 * PAGE_SIZE;

user_page_t user_page[USER_PAGE_NUM];
shmpage_t shmpage[100];
disk_page_t disk_page[DISK_PAGE_NUM];

// allocPage() returns pa
ptr_t allocPage(uint8_t pin, ptr_t pgtable)
{
    // align PAGE_SIZE: no need to ROUND
    int count = 0;
    int full_flag = 0;

    while(user_page[pa2num(user_memCurr)].valid == 1){
        user_memCurr += PAGE_SIZE;
        count++;
        if(user_memCurr == USER_MEM_END){
            user_memCurr = USER_MEM_BEGIN;
        }
        if(count == USER_PAGE_NUM){
            full_flag = 1;
            break;
        }
    }

    ptr_t ret;
    if(full_flag){
        ret = selete_page();
        write_to_sd(ret);   
        user_page[pa2num(ret)].pin = pin;
        user_page[pa2num(ret)].pgtable = pgtable;
    }
    else{
        ret = user_memCurr;
        user_page[pa2num(ret)].valid = 1;
        user_page[pa2num(ret)].pin = pin;
        user_page[pa2num(ret)].pgtable = pgtable;
        user_memCurr += PAGE_SIZE;   
        if(user_memCurr == USER_MEM_END){
            user_memCurr = USER_MEM_BEGIN;
        }
    }
    return ret;
}

ptr_t selete_page()
{
    int i;
    for(i = 0; i < USER_PAGE_NUM; i++){
        if(user_page[i].pin == 0){
            break;
        }   
    }
    return (USER_MEM_BEGIN + i * PAGE_SIZE);
}

unsigned long free_page_num()
{
    unsigned long ret = 0;
    for(int i = 0; i < USER_PAGE_NUM; i++){
        if(user_page[i].valid == 0){
            ret++;
        }
    }
    return ret;
}

void write_to_sd(ptr_t pa)
{
    // alloc a disk page
    int i;
    for(i = 0; i < DISK_PAGE_NUM; i++){
        if(disk_page[i].valid == 0){
            break;
        }
    }
    assert(i != DISK_PAGE_NUM);
    disk_page[i].valid = 1;
    int block_id = 2048 + i * 8;

    // modify the PTE
    PTE* pgtable = user_page[pa2num(pa)].pgtable;
    *pgtable = (block_id << _PAGE_PFN_SHIFT) | _PAGE_SOFT;

    sbi_sd_write(pa, 8, block_id);
}

// baseAddr is pa
void freePage(ptr_t baseAddr)
{
    // TODO:
    user_page[pa2num(baseAddr)].valid = 0;
}

// kmalloc() returns pa
void *kmalloc(size_t size)
{
    // TODO(if you need):
    // align 4
    ptr_t ret = ROUND(kernel_memCurr, 4);
    kernel_memCurr = ret + size;
    return (void*)ret;
}

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

    *((PTE*)pa2kva(last_level_pgdir_1) + 0x00f) = ((allocPage(1, ((PTE*)pa2kva(last_level_pgdir_1) + 0x00f)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;

    *((PTE*)pa2kva(last_level_pgdir_2) + 0x010) = ((allocPage(1, ((PTE*)pa2kva(last_level_pgdir_2) + 0x010)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x011) = ((allocPage(1, ((PTE*)pa2kva(last_level_pgdir_2) + 0x011)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;    
    *((PTE*)pa2kva(last_level_pgdir_2) + 0x012) = ((allocPage(1, ((PTE*)pa2kva(last_level_pgdir_2) + 0x012)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                    _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER | _PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;                                                                                            
    share_pgtable(pgdir, PGDIR_PA);

    return pgdir;
}

uintptr_t shm_page_get(int key)
{
    // TODO(c-core):
    PTE* pgdir = current_running[cpu_id]->pgdir;

    int free_va;
    for(free_va = 0x1000; free_va < 0x10000; free_va += PAGE_SIZE){
        int VPN2 = (free_va >> 30) & 0x1ff;
        int VPN1 = (free_va >> 21) & 0x1ff;
        int VPN0 = (free_va >> 12) & 0x1ff;
        int used = 0;

        if(*((PTE*)pa2kva(pgdir) + VPN2) & _PAGE_PRESENT){
            PTE* second_level_pgdir = get_pfn(*((PTE*)pa2kva(pgdir) + VPN2)) << NORMAL_PAGE_SHIFT;
            if(*((PTE*)pa2kva(second_level_pgdir) + VPN1) & _PAGE_PRESENT){
                PTE* last_level_pgdir = get_pfn(*((PTE*)pa2kva(second_level_pgdir) + VPN1)) << NORMAL_PAGE_SHIFT;
                if(*((PTE*)pa2kva(last_level_pgdir) + VPN0) & _PAGE_PRESENT){
                    used = 1;
                }
            }
        }

        if(!used){
            break;
        }
    }

    if(shmpage[key % SHMPAGE_NUM].pa == 0){
        shmpage[key % SHMPAGE_NUM].pa = allocPage(1, NULL);
        shmpage[key % SHMPAGE_NUM].count = 0;
        uintptr_t kva = pa2kva(shmpage[key % SHMPAGE_NUM].pa);
        memset(kva, 0, 4096);
    }
    shmpage[key % SHMPAGE_NUM].count++;

    int VPN2 = (free_va >> 30) & 0x1ff;
    int VPN1 = (free_va >> 21) & 0x1ff;
    int VPN0 = (free_va >> 12) & 0x1ff;
    if(*((PTE*)pa2kva(pgdir) + VPN2) & _PAGE_PRESENT){
        PTE* second_level_pgdir = get_pfn(*((PTE*)pa2kva(pgdir) + VPN2)) << NORMAL_PAGE_SHIFT;
        if(*((PTE*)pa2kva(second_level_pgdir) + VPN1) & _PAGE_PRESENT){
            PTE* last_level_pgdir = get_pfn(*((PTE*)pa2kva(second_level_pgdir) + VPN1)) << NORMAL_PAGE_SHIFT;
            *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((shmpage[key % SHMPAGE_NUM].pa >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                            _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
        }
        else{
            PTE *last_level_pgdir = (PTE*)pgdirCurr;
            clear_pgdir(pa2kva(last_level_pgdir));
            pgdirCurr += PAGE_SIZE;

            *((PTE*)pa2kva(second_level_pgdir) + VPN1) = (((uint64_t)last_level_pgdir >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
            *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((shmpage[key % SHMPAGE_NUM].pa >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
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
        *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((shmpage[key % SHMPAGE_NUM].pa >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                        _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
    }
    
    return free_va;
}

void free_user_page(PTE* pgdir)
{
    int VPN2, VPN1, VPN0;
    for(VPN2 = 0; VPN2 < 0x100; VPN2++){
        if(*((PTE*)pa2kva(pgdir) + VPN2) & _PAGE_PRESENT){
            PTE* second_level_pgdir = get_pfn(*((PTE*)pa2kva(pgdir) + VPN2)) << NORMAL_PAGE_SHIFT;
            for(VPN1 = 0; VPN1 < 512; VPN1++){
                if(*((PTE*)pa2kva(second_level_pgdir) + VPN1) & _PAGE_PRESENT){
                    PTE* last_level_pgdir = get_pfn(*((PTE*)pa2kva(second_level_pgdir) + VPN1)) << NORMAL_PAGE_SHIFT;
                    for(VPN0 = 0; VPN0 < 512; VPN0++){
                        if(*((PTE*)pa2kva(last_level_pgdir) + VPN0) & _PAGE_PRESENT){
                            ptr_t pa = get_pfn(*((PTE*)pa2kva(last_level_pgdir) + VPN0)) << NORMAL_PAGE_SHIFT;
                            freePage(pa);
                        }
                    }
                }
            }
        }
    }
}

void shm_page_dt(uintptr_t addr)
{
    // TODO(c-core):
    int VPN2 = (addr >> 30) & 0x1ff;
    int VPN1 = (addr >> 21) & 0x1ff;
    int VPN0 = (addr >> 12) & 0x1ff;
    PTE* pgdir = current_running[cpu_id]->pgdir;
    PTE* second_level_pgdir = get_pfn(*((PTE*)pa2kva(pgdir) + VPN2)) << NORMAL_PAGE_SHIFT;
    PTE* last_level_pgdir = get_pfn(*((PTE*)pa2kva(second_level_pgdir) + VPN1)) << NORMAL_PAGE_SHIFT;
    uint64_t pa = (get_pfn(*((PTE*)pa2kva(last_level_pgdir) + VPN0)) << NORMAL_PAGE_SHIFT) | (addr & 0xfff);
    *((PTE*)pa2kva(last_level_pgdir) + VPN0) = 0;

    int key;
    for(key = 0; key < SHMPAGE_NUM; key++){
        if(shmpage[key].pa == pa){
            break;
        }
    }
    shmpage[key].count--;

    if(shmpage[key].count == 0){
        freePage(shmpage[key].pa);
        shmpage[key].pa =0;    
    }
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
            if(*((PTE*)pa2kva(last_level_pgdir) + VPN0) & _PAGE_SOFT){
                ptr_t pa = allocPage(0, ((PTE*)pa2kva(last_level_pgdir) + VPN0));
                int block_id =  get_pfn(*((PTE*)pa2kva(last_level_pgdir) + VPN0));
                assert(disk_page[(block_id - 2048) / 8].valid == 1);
                sbi_sd_read(pa, 8, block_id);
                disk_page[(block_id - 2048) / 8].valid = 0;
                *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((pa >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                                _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
            }
            else{
                *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((allocPage(0, ((PTE*)pa2kva(last_level_pgdir) + VPN0)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                                _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
            }
        }
        else{
            PTE *last_level_pgdir = (PTE*)pgdirCurr;
            clear_pgdir(pa2kva(last_level_pgdir));
            pgdirCurr += PAGE_SIZE;

            *((PTE*)pa2kva(second_level_pgdir) + VPN1) = (((uint64_t)last_level_pgdir >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT;
            *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((allocPage(0, ((PTE*)pa2kva(last_level_pgdir) + VPN0)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
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
        *((PTE*)pa2kva(last_level_pgdir) + VPN0) = ((allocPage(0, ((PTE*)pa2kva(last_level_pgdir) + VPN0)) >> NORMAL_PAGE_SHIFT) << _PAGE_PFN_SHIFT) | _PAGE_PRESENT |
                                        _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_USER |_PAGE_EXEC | _PAGE_WRITE | _PAGE_READ | _PAGE_PRESENT;
    }
}

int pa2num(uintptr_t pa)
{
    return (pa - USER_MEM_BEGIN) / PAGE_SIZE;
}