/* RISC-V kernel boot stage */
#include <os/elf.h>
#include <pgtable.h>
#include <sbi.h>

typedef void (*kernel_entry_t)(unsigned long);

extern unsigned char _elf_main[];
extern unsigned _length_main;

/*
 * using 2MB large page
 * kva = kpa + 0xffff_ffc0_0000_0000 use 2MB page
 * map all physical memory
 */
void map_kernel_page(PTE *pgdir)
{
    clear_pgdir(pgdir);
    /*
     * first level:
     * 0xffff_ffc0_5000_0000: VPN2 = 0x101
     * 0x0000_0000_5000_0000: VPN2 = 0x001
     */
    set_pfn(pgdir + 0x101, (PGDIR_PA + NORMAL_PAGE_SIZE) >> NORMAL_PAGE_SHIFT);
    set_attribute(pgdir + 0x101, _PAGE_PRESENT);
    set_pfn(pgdir + 0x001, (PGDIR_PA + NORMAL_PAGE_SIZE) >> NORMAL_PAGE_SHIFT);
    set_attribute(pgdir + 0x001, _PAGE_PRESENT);
    /*
     * last level:
     * 0x5000_0000: VPN1 = 0x80
     * 256MB = 2MB * 128
     */
    PTE *last_level_pgdir = (PTE*)(PGDIR_PA + NORMAL_PAGE_SIZE);
    clear_pgdir(last_level_pgdir);
    for(int i = 0; i < 128; i++){
        PTE* pgtable = last_level_pgdir + 0x80 + i;
        set_pfn(pgtable, (0x50000000 + LARGE_PAGE_SIZE * i ) >> NORMAL_PAGE_SHIFT);
        set_attribute(pgtable, _PAGE_PRESENT);
        set_attribute(pgtable, _PAGE_READ);
        set_attribute(pgtable, _PAGE_WRITE);
        set_attribute(pgtable, _PAGE_EXEC);
        set_attribute(pgtable, _PAGE_ACCESSED);
        set_attribute(pgtable, _PAGE_DIRTY);
    }
}

void enable_vm()
{
    set_satp(SATP_MODE_SV39, 0, (PGDIR_PA >> NORMAL_PAGE_SHIFT));
    local_flush_tlb_all();
}

/* Sv-39 mode
 * 0x0000_0000_0000_0000-0x0000_003f_ffff_ffff is for user mode
 * 0xffff_ffc0_0000_0000-0xffff_ffff_ffff_ffff is for kernel mode
 */
void setup_vm()
{
    // map kernel virtual address(kva) to kernel physical
    map_kernel_page((PTE *)PGDIR_PA);
    // enable virtual memory
    enable_vm();
}

uintptr_t directmap(uintptr_t kva, uintptr_t pgdir)
{
    return kva;
}

kernel_entry_t start_kernel = NULL;

/* start here */
int boot_kernel(unsigned long mhartid)
{
    if(mhartid == 0) {
        // setup memory mapping
        setup_vm();
        sbi_console_putstr("> [INIT] Set Up Kernel Page Table Successfully.\n\r");
        // load kernel
        start_kernel = (kernel_entry_t)load_elf(_elf_main, _length_main, PGDIR_PA, directmap);
        sbi_console_putstr("> [INIT] Copy Kernel to Memory Successfully.\n\r");
    }else {
        enable_vm();
    }
    start_kernel(mhartid);
    return 0;
}
