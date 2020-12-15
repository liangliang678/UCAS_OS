#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

static uintptr_t io_base = IO_ADDR_START;
static PTE *pgdir = PGDIR_PA;

void *ioremap(unsigned long phys_addr, unsigned long size)
{
    // map phys_addr to a virtual address
    // then return the virtual address
    uintptr_t ret = io_base;
    while(size > 0){
        int VPN2 = (io_base >> VPN2_SHIFT) & VPN_MASK;
        int VPN1 = (io_base >> VPN1_SHIFT) & VPN_MASK;
        int VPN0 = (io_base >> VPN0_SHIFT) & VPN_MASK;

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
            set_pfn(last_level_pgdir + VPN0, phys_addr >> NORMAL_PAGE_SHIFT);
            set_attribute(last_level_pgdir + VPN0, _PAGE_PRESENT);
            set_attribute(last_level_pgdir + VPN0, _PAGE_ACCESSED);
            set_attribute(last_level_pgdir + VPN0, _PAGE_DIRTY);
            set_attribute(last_level_pgdir + VPN0, _PAGE_EXEC);
            set_attribute(last_level_pgdir + VPN0, _PAGE_WRITE);
            set_attribute(last_level_pgdir + VPN0, _PAGE_READ);
        }

        size -= NORMAL_PAGE_SIZE;
        phys_addr += NORMAL_PAGE_SIZE;
        io_base += NORMAL_PAGE_SIZE;
    }
    return ret;
}

void iounmap(void *io_addr)
{
    // TODO: a very naive iounmap() is OK
    // maybe no one would call this function?
}
