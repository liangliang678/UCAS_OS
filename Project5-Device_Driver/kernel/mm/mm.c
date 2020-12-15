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

user_page_t user_page[USER_PAGE_NUM];
shm_page_t shm_page[SHM_PAGE_NUM];
disk_page_t disk_page[DISK_PAGE_NUM];
pgdir_page_t pgdir_page[PGDIR_PAGE_NUM];

// returns pa
PTE* alloc_pgdir_page()
{
    int i;
    for(i = 0; i < PGDIR_PAGE_NUM; i++){
        if(pgdir_page[i].valid == 0){
            break;
        }
    }
    assert(i != PGDIR_PAGE_NUM);
    pgdir_page[i].valid = 1;
    PTE *ret = (PTE*)(FREE_PGDIR_ADDR + i * PAGE_SIZE);
    clear_pgdir((PTE*)pa2kva((uintptr_t)ret));
    return ret;
}

void free_pgdir_page(uintptr_t addr)
{
    int i = (addr - FREE_PGDIR_ADDR) / PAGE_SIZE;
    pgdir_page[i].valid = 0;
}

int pa2num(uintptr_t pa)
{
    return (pa - USER_MEM_BEGIN) / PAGE_SIZE;
}
ptr_t num2pa(int num)
{
    return (USER_MEM_BEGIN + num * PAGE_SIZE);
}

// returns pa
ptr_t alloc_user_page(uint8_t pin, PTE* pgtable)
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

// baseAddr is pa
void free_user_page(ptr_t baseAddr)
{
    user_page[pa2num(baseAddr)].valid = 0;
}

// returns pa
void *kmalloc(size_t size)
{
    // align 4
    ptr_t ret = ROUND(kernel_memCurr, 4);
    kernel_memCurr = ret + size;
    return (void*)ret;
}

// select a user page to write to disk (NRU), returns pa
ptr_t selete_page()
{
    int flag = 0;
    ptr_t pa = 0;
    /*
    * 1: A = 1, D = 1
    * 2: A = 0, D = 1
    * 3: A = 1, D = 0
    * 4: A = 0, D = 0
    */
    for(int i = 0; i < USER_PAGE_NUM; i++){
        if(user_page[i].pin == 0){
            PTE* pgdir = user_page[i].pgtable;
            if(!get_attribute(*pgdir, _PAGE_ACCESSED) && !get_attribute(*pgdir, _PAGE_DIRTY)){
                pa = num2pa(i);
                flag = 4;
                break;
            }
            else if(get_attribute(*pgdir, _PAGE_ACCESSED) && !get_attribute(*pgdir, _PAGE_DIRTY)){
                if(flag < 3){
                    pa = num2pa(i);
                    flag = 3;
                }   
            }
            else if(!get_attribute(*pgdir, _PAGE_ACCESSED) && get_attribute(*pgdir, _PAGE_DIRTY)){
                if(flag < 2){
                    pa = num2pa(i);
                    flag = 2;
                }   
            }
            else if(get_attribute(*pgdir, _PAGE_ACCESSED) && get_attribute(*pgdir, _PAGE_DIRTY)){
                if(flag < 1){
                    pa =num2pa(i);
                    flag = 1;
                }  
            }
        }   
    }
    assert(pa != 0);
    return pa;
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
    int block_id = SWAP_BEGIN + i * 8;

    // modify the PTE
    PTE* pgtable = user_page[pa2num(pa)].pgtable;
    set_pfn(pgtable, block_id);
    clear_attribute(pgtable, ATTRIBUTE_MASK);
    set_attribute(pgtable, _PAGE_SOFT);

    // write to sd
    sbi_sd_write(pa, 8, block_id);
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

/*
 * Virtual Memory Layout
 * 0x0_0000_1000 ~ 0x0_0001_0000: share
 * 0x0_0001_0000 ~ 0x0_0001_3000: code
 * 0xf_0000_f000 ~ 0xf_0001_0000: stack
 */
PTE* init_page_table()
{
    PTE *pgdir = alloc_pgdir_page();
    PTE *second_level_pgdir_1 = alloc_pgdir_page();
    PTE *second_level_pgdir_2 = alloc_pgdir_page();
    PTE *last_level_pgdir_1 = alloc_pgdir_page();
    PTE *last_level_pgdir_2 = alloc_pgdir_page();

    // first level
    set_pfn((PTE*)pa2kva((uintptr_t)pgdir) + 0x03c, (uint64_t)second_level_pgdir_1 >> NORMAL_PAGE_SHIFT);
    set_attribute((PTE*)pa2kva((uintptr_t)pgdir) + 0x03c, _PAGE_PRESENT);
    set_pfn((PTE*)pa2kva((uintptr_t)pgdir) + 0x000, (uint64_t)second_level_pgdir_2 >> NORMAL_PAGE_SHIFT);
    set_attribute((PTE*)pa2kva((uintptr_t)pgdir) + 0x000, _PAGE_PRESENT);

    // second level
    set_pfn((PTE*)pa2kva((uintptr_t)second_level_pgdir_1) + 0x000, (uint64_t)last_level_pgdir_1 >> NORMAL_PAGE_SHIFT);
    set_attribute((PTE*)pa2kva((uintptr_t)second_level_pgdir_1) + 0x000, _PAGE_PRESENT);
    set_pfn((PTE*)pa2kva((uintptr_t)second_level_pgdir_2) + 0x000, (uint64_t)last_level_pgdir_2 >> NORMAL_PAGE_SHIFT);
    set_attribute((PTE*)pa2kva((uintptr_t)second_level_pgdir_2) + 0x000, _PAGE_PRESENT);

    // last level
    ptr_t stack_pfn = alloc_user_page(1, ((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f)) >> NORMAL_PAGE_SHIFT;
    ptr_t code_pfn_1 = alloc_user_page(1, ((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010)) >> NORMAL_PAGE_SHIFT;
    ptr_t code_pfn_2 = alloc_user_page(1, ((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011)) >> NORMAL_PAGE_SHIFT;
    ptr_t code_pfn_3 = alloc_user_page(1, ((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012)) >> NORMAL_PAGE_SHIFT;

    set_pfn((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, stack_pfn);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_PRESENT);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_DIRTY);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_ACCESSED);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_USER);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_EXEC);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_WRITE);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_1) + 0x00f, _PAGE_READ);

    set_pfn((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, code_pfn_1);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_PRESENT);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_DIRTY);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_ACCESSED);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_USER);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_EXEC);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_WRITE);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x010, _PAGE_READ);
    set_pfn((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, code_pfn_2);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_PRESENT);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_DIRTY);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_ACCESSED);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_USER);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_EXEC);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_WRITE);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x011, _PAGE_READ);
    set_pfn((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, code_pfn_3);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_PRESENT);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_DIRTY);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_ACCESSED);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_USER);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_EXEC);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_WRITE);
    set_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir_2) + 0x012, _PAGE_READ);

    // kernel
    set_pfn((PTE*)pa2kva((uintptr_t)pgdir) + 0x101, (PGDIR_PA + NORMAL_PAGE_SIZE) >> NORMAL_PAGE_SHIFT);
    set_attribute((PTE*)pa2kva((uintptr_t)pgdir) + 0x101, _PAGE_PRESENT);

    return pgdir;
}

void free_process_user_page(PTE* pgdir)
{
    int VPN2, VPN1, VPN0;
    free_pgdir_page((uintptr_t)pgdir);
    for(VPN2 = 0; VPN2 < 0x100; VPN2++){
        if(get_attribute(*((PTE*)pa2kva((uintptr_t)pgdir) + VPN2), _PAGE_PRESENT)){
            PTE* second_level_pgdir = (PTE*)get_pa(*((PTE*)pa2kva((uintptr_t)pgdir) + VPN2));
            free_pgdir_page((uintptr_t)second_level_pgdir);
            for(VPN1 = 0; VPN1 < NUM_PTE_ENTRY; VPN1++){
                if(get_attribute(*((PTE*)pa2kva((uintptr_t)second_level_pgdir) + VPN1), _PAGE_PRESENT)){
                    PTE* last_level_pgdir = (PTE*)get_pa(*((PTE*)pa2kva((uintptr_t)second_level_pgdir) + VPN1));
                    free_pgdir_page((uintptr_t)last_level_pgdir);
                    for(VPN0 = 0; VPN0 < NUM_PTE_ENTRY; VPN0++){
                        if(get_attribute(*((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0), _PAGE_PRESENT)){
                            ptr_t pa = get_pa(*((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0));
                            free_user_page(pa);
                        }
                        else if(get_attribute(*((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0), _PAGE_SOFT)){
                            int block_id = get_pfn(*((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0));
                            disk_page[(block_id - SWAP_BEGIN) / 8].valid = 0;
                        }
                    }
                }
            }
        }
    }
}

uintptr_t shm_page_get(int key)
{
    PTE* pgdir = current_running[cpu_id]->pgdir;

    int free_va;
    for(free_va = 0x1000; free_va < 0x10000; free_va += PAGE_SIZE){
        int VPN2 = (free_va >> VPN2_SHIFT) & VPN_MASK;
        int VPN1 = (free_va >> VPN1_SHIFT) & VPN_MASK;
        int VPN0 = (free_va >> VPN0_SHIFT) & VPN_MASK;
        int used = 0;
        if(get_attribute(*((PTE*)pa2kva((uintptr_t)pgdir) + VPN2), _PAGE_PRESENT)){
            PTE* second_level_pgdir = (PTE*)get_pa(*((PTE*)pa2kva((uintptr_t)pgdir) + VPN2));
            if(get_attribute(*((PTE*)pa2kva((uintptr_t)second_level_pgdir) + VPN1), _PAGE_PRESENT)){
                PTE* last_level_pgdir = (PTE*)get_pa(*((PTE*)pa2kva((uintptr_t)second_level_pgdir) + VPN1));
                if(get_attribute(*((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0), _PAGE_PRESENT)){
                    used = 1;
                }
            }
        }
        if(!used){
            break;
        }
    }

    if(shm_page[key % SHM_PAGE_NUM].pa == 0){
        shm_page[key % SHM_PAGE_NUM].pa = alloc_user_page(1, NULL);
        shm_page[key % SHM_PAGE_NUM].count = 0;
        memset((void *)pa2kva((uintptr_t)shm_page[key % SHM_PAGE_NUM].pa), 0, 4096);
    }

    shm_page[key % SHM_PAGE_NUM].count++;
    int VPN2 = (free_va >> VPN2_SHIFT) & VPN_MASK;
    int VPN1 = (free_va >> VPN1_SHIFT) & VPN_MASK;
    int VPN0 = (free_va >> VPN0_SHIFT) & VPN_MASK;

    pgdir = (PTE*)pa2kva((uintptr_t)pgdir);
    if(!get_attribute(*(pgdir + VPN2), _PAGE_PRESENT)){
        PTE *second_level_pgdir = alloc_pgdir_page();
        set_pfn(pgdir + VPN2, (uint64_t)second_level_pgdir >> NORMAL_PAGE_SHIFT);
        set_attribute(pgdir + VPN2, _PAGE_PRESENT);
    }

    PTE* second_level_pgdir = (PTE*)get_pa(*(pgdir + VPN2));
    second_level_pgdir = (PTE*)pa2kva((uintptr_t)second_level_pgdir);
    if(!get_attribute(*(second_level_pgdir + VPN1), _PAGE_PRESENT)){
        PTE *last_level_pgdir = alloc_pgdir_page();
        set_pfn(second_level_pgdir + VPN1, (uint64_t)last_level_pgdir >> NORMAL_PAGE_SHIFT);
        set_attribute(second_level_pgdir + VPN1, _PAGE_PRESENT);
    }

    PTE* last_level_pgdir = (PTE*)get_pa(*(second_level_pgdir + VPN1));
    last_level_pgdir = (PTE*)pa2kva((uintptr_t)last_level_pgdir);
    if(!get_attribute(*(last_level_pgdir + VPN0), _PAGE_PRESENT)){
        set_pfn(last_level_pgdir + VPN0, shm_page[key % SHM_PAGE_NUM].pa >> NORMAL_PAGE_SHIFT);
        set_attribute(last_level_pgdir + VPN0, _PAGE_PRESENT);
        set_attribute(last_level_pgdir + VPN0, _PAGE_USER);
        set_attribute(last_level_pgdir + VPN0, _PAGE_EXEC);
        set_attribute(last_level_pgdir + VPN0, _PAGE_WRITE);
        set_attribute(last_level_pgdir + VPN0, _PAGE_READ);
        set_attribute(last_level_pgdir + VPN0, _PAGE_ACCESSED);
        set_attribute(last_level_pgdir + VPN0, _PAGE_DIRTY);
    }
    
    return free_va;
}

void shm_page_dt(uintptr_t addr)
{
    int VPN2 = (addr >> VPN2_SHIFT) & VPN_MASK;
    int VPN1 = (addr >> VPN1_SHIFT) & VPN_MASK;
    int VPN0 = (addr >> VPN0_SHIFT) & VPN_MASK;
    PTE* pgdir = current_running[cpu_id]->pgdir;
    PTE* second_level_pgdir = (PTE*)get_pa(*((PTE*)pa2kva((uintptr_t)pgdir) + VPN2));
    PTE* last_level_pgdir = (PTE*)get_pa(*((PTE*)pa2kva((uintptr_t)second_level_pgdir) + VPN1));
    ptr_t pa = get_pa(*((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0)) | (addr & PAGE_OFFSET_MASK);
    clear_attribute((PTE*)pa2kva((uintptr_t)last_level_pgdir) + VPN0, ATTRIBUTE_MASK);

    int key;
    for(key = 0; key < SHM_PAGE_NUM; key++){
        if(shm_page[key].pa == pa){
            break;
        }
    }
    assert(key != SHM_PAGE_NUM);
    shm_page[key].count--;
    if(shm_page[key].count == 0){
        free_user_page(shm_page[key].pa);
        shm_page[key].pa =0;  
    }
}

void handle_page_fault(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    PTE* pgdir = current_running[cpu_id]->pgdir;
    pgdir = (PTE*)pa2kva((uintptr_t)pgdir);

    int VPN2 = (stval >> VPN2_SHIFT) & VPN_MASK;
    int VPN1 = (stval >> VPN1_SHIFT) & VPN_MASK;
    int VPN0 = (stval >> VPN0_SHIFT) & VPN_MASK;

    if(!get_attribute(*(pgdir + VPN2), _PAGE_PRESENT)){
        PTE *second_level_pgdir = alloc_pgdir_page();
        set_pfn(pgdir + VPN2, (uint64_t)second_level_pgdir >> NORMAL_PAGE_SHIFT);
        set_attribute(pgdir + VPN2, _PAGE_PRESENT);
    }

    PTE* second_level_pgdir = (PTE*)get_pa(*(pgdir + VPN2));
    second_level_pgdir = (PTE*)pa2kva((uintptr_t)second_level_pgdir);
    if(!get_attribute(*(second_level_pgdir + VPN1), _PAGE_PRESENT)){
        PTE *last_level_pgdir = alloc_pgdir_page();
        set_pfn(second_level_pgdir + VPN1, (uint64_t)last_level_pgdir >> NORMAL_PAGE_SHIFT);
        set_attribute(second_level_pgdir + VPN1, _PAGE_PRESENT);
    }

    PTE* last_level_pgdir = (PTE*)get_pa(*(second_level_pgdir + VPN1));
    last_level_pgdir = (PTE*)pa2kva((uintptr_t)last_level_pgdir);
    if(get_attribute(*(last_level_pgdir + VPN0), _PAGE_SOFT)){
        // this page is in disk
        ptr_t pa = alloc_user_page(0, last_level_pgdir + VPN0);
        int block_id =  get_pfn(*(last_level_pgdir + VPN0));
        assert(disk_page[(block_id - SWAP_BEGIN) / 8].valid == 1);
        sbi_sd_read(pa, 8, block_id);
        disk_page[(block_id - SWAP_BEGIN) / 8].valid = 0;
        set_pfn(last_level_pgdir + VPN0, pa >> NORMAL_PAGE_SHIFT);
        clear_attribute(last_level_pgdir + VPN0, ATTRIBUTE_MASK);
        set_attribute(last_level_pgdir + VPN0, _PAGE_PRESENT);
        set_attribute(last_level_pgdir + VPN0, _PAGE_USER);
        set_attribute(last_level_pgdir + VPN0, _PAGE_EXEC);
        set_attribute(last_level_pgdir + VPN0, _PAGE_WRITE);
        set_attribute(last_level_pgdir + VPN0, _PAGE_READ);   
    }
    else if(!get_attribute(*(last_level_pgdir + VPN0), _PAGE_PRESENT)){
        // this page has not been mapped
        ptr_t pa = alloc_user_page(0, (last_level_pgdir + VPN0));
        set_pfn(last_level_pgdir + VPN0, pa >> NORMAL_PAGE_SHIFT);
        set_attribute(last_level_pgdir + VPN0, _PAGE_PRESENT);
        set_attribute(last_level_pgdir + VPN0, _PAGE_USER);
        set_attribute(last_level_pgdir + VPN0, _PAGE_EXEC);
        set_attribute(last_level_pgdir + VPN0, _PAGE_WRITE);
        set_attribute(last_level_pgdir + VPN0, _PAGE_READ);
    }
    else if(!get_attribute(*(last_level_pgdir + VPN0), _PAGE_ACCESSED) || !get_attribute(*(last_level_pgdir + VPN0), _PAGE_DIRTY)){
        // this page is accessed
        set_attribute(last_level_pgdir + VPN0, _PAGE_ACCESSED);
        if(cause == EXCC_STORE_PAGE_FAULT){
            set_attribute(last_level_pgdir + VPN0, _PAGE_DIRTY);
        }
    }
    else{
        local_flush_tlb_all();
    }
}
