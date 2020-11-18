/* RISC-V kernel boot stage */
#include <context.h>
#include <os/elf.h>
#include <pgtable.h>
#include <sbi.h>

typedef void (*kernel_entry_t)(unsigned long, uintptr_t);

extern unsigned char _elf_main[];
extern unsigned _length_main;

/********* setup memory mapping ***********/
uintptr_t alloc_page()
{
    // TODO: alloc pages for page table entries
}

// using 2MB large page
void map_page(uint64_t va, uint64_t pa, PTE *pgdir)
{
    // TODO: map va to pa
}

void enable_vm()
{
    // TODO: write satp to enable paging
    // remember to flush TLB
}

/* Sv-39 mode
 * 0x0000_0000_0000_0000-0x0000_003f_ffff_ffff is for user mode
 * 0xffff_ffc0_0000_0000-0xffff_ffff_ffff_ffff is for kernel mode
 */
void setup_vm()
{
    // TODO:
    clear_pgdir(PGDIR_PA);
    // map kernel virtual address(kva) to kernel physical
    // address(kpa) kva = kpa + 0xffff_ffc0_0000_0000 use 2MB page,
    // map all physical memory

    // enable virtual memory
    
}

uintptr_t directmap(uintptr_t kva, uintptr_t pgdir)
{
    // ignore pgdir
    return kva;
}

kernel_entry_t start_kernel = NULL;

/*********** start here **************/
int boot_kernel(unsigned long mhartid, uintptr_t riscv_dtb)
{
    if (mhartid == 0) {
        setup_vm();
        // load kernel
        start_kernel =
            (kernel_entry_t)load_elf(_elf_main, _length_main,
                                     PGDIR_PA, directmap);
    } else {
        // TODO: what should we do for other cores?
    }
    start_kernel(mhartid, riscv_dtb);
    return 0;
}
