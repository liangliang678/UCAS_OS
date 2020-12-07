#ifndef PGTABLE_H
#define PGTABLE_H

#include <type.h>
#include <sbi.h>
#include <os/string.h>

/*
 * Flush entire local TLB.  'sfence.vma' implicitly fences with the instruction
 * cache as well, so a 'fence.i' is not necessary.
 */
static inline void local_flush_tlb_all(void)
{
    __asm__ __volatile__ ("sfence.vma" : : : "memory");
}

/* Flush one page from local TLB */
static inline void local_flush_tlb_page(unsigned long addr)
{
    __asm__ __volatile__ ("sfence.vma %0" : : "r" (addr) : "memory");
}

static inline void local_flush_icache_all(void)
{
    asm volatile ("fence.i" ::: "memory");
}

static inline void flush_tlb_all(void)
{
    local_flush_tlb_all();
    sbi_remote_sfence_vma(NULL, 0, -1);
}

static inline void flush_tlb_page_all(unsigned long addr)
{
    local_flush_tlb_page(addr);
    sbi_remote_sfence_vma(NULL, 0, -1);
}

static inline void flush_icache_all(void)
{
    local_flush_icache_all();
    sbi_remote_fence_i(NULL);
}

#define SATP_MODE_SV39 8
#define SATP_MODE_SV48 9
#define SATP_ASID_SHIFT 44lu
#define SATP_MODE_SHIFT 60lu

static inline void set_satp(
    unsigned mode, unsigned asid, unsigned long ppn)
{
    unsigned long __v =
        (unsigned long)(((unsigned long)mode << SATP_MODE_SHIFT) | ((unsigned long)asid << SATP_ASID_SHIFT) | ppn);
    __asm__ __volatile__("sfence.vma\ncsrw satp, %0" : : "rK"(__v) : "memory");
}

#define PGDIR_PA 0x5e000000lu

/*
 * PTE format:
 * | XLEN-1  10 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *       PFN      reserved for SW   D   A   G   U   X   W   R   V
 */

#define NORMAL_PAGE_SHIFT 12lu
#define NORMAL_PAGE_SIZE (1lu << NORMAL_PAGE_SHIFT)
#define LARGE_PAGE_SHIFT 21lu
#define LARGE_PAGE_SIZE (1lu << LARGE_PAGE_SHIFT)

#define _PAGE_PRESENT (1 << 0)
#define _PAGE_READ (1 << 1)     /* Readable */
#define _PAGE_WRITE (1 << 2)    /* Writable */
#define _PAGE_EXEC (1 << 3)     /* Executable */
#define _PAGE_USER (1 << 4)     /* User */
#define _PAGE_GLOBAL (1 << 5)   /* Global */
#define _PAGE_ACCESSED (1 << 6) /* Set by hardware/software on any access */
#define _PAGE_DIRTY (1 << 7)    /* Set by hardware/software on any write */
#define _PAGE_SOFT (1 << 8)     /* Reserved for software */
#define _PAGE_PFN_SHIFT 10lu

#define VA_MASK ((1lu << 39) - 1)
#define PFN_MASK ((1lu << 44) - 1)
#define ATTRIBUTE_MASK ((1lu << 9) - 1)
#define PAGE_OFFSET_MASK ((1lu << 12) - 1)

#define PPN_BITS 9lu
#define NUM_PTE_ENTRY (1 << PPN_BITS)
#define PPN_MASK ((1lu << PPN_BITS) - 1)
#define VPN_MASK ((1lu << PPN_BITS) - 1)

#define KERNEL_MEM_OFFSET 0xffffffc000000000

#define VPN2_SHIFT 30
#define VPN1_SHIFT 21
#define VPN0_SHIFT 12

typedef uint64_t PTE;

static inline void clear_pgdir(PTE* pgdir_addr)
{
    memset(pgdir_addr, 0, 4096);
}

/* Get/Set page frame number of the `entry` */
static inline long get_pfn(PTE entry)
{
    return ((entry >> _PAGE_PFN_SHIFT) & PFN_MASK);
}
static inline void set_pfn(PTE *entry, uint64_t pfn)
{
    *entry = *entry & ATTRIBUTE_MASK;
    *entry = *entry | (pfn << _PAGE_PFN_SHIFT);
}
static inline uintptr_t get_pa(PTE entry)
{
    return (((entry >> _PAGE_PFN_SHIFT) & PFN_MASK) << NORMAL_PAGE_SHIFT);
}

/* Get/Set/Clear attribute(s) of the `entry` */
static inline long get_attribute(PTE entry, uint64_t mask)
{
    return ((entry & mask) != 0);
}
static inline void set_attribute(PTE *entry, uint64_t bits)
{
    *entry = *entry | bits;
}
static inline void clear_attribute(PTE *entry, uint64_t bits)
{
    *entry = *entry & ~bits;
}

static inline uintptr_t kva2pa(uintptr_t kva)
{
    return (uintptr_t)(kva - KERNEL_MEM_OFFSET);
}
static inline uintptr_t pa2kva(uintptr_t pa)
{
    return (uintptr_t)(pa + KERNEL_MEM_OFFSET);
}

// pgdir is pa
static inline uintptr_t get_kva_of(uintptr_t va, uintptr_t pgdir)
{
    uintptr_t pgdir_kva = pa2kva((uintptr_t)pgdir);
    int VPN2 = (va >> VPN2_SHIFT) & VPN_MASK;
    int VPN1 = (va >> VPN1_SHIFT) & VPN_MASK;
    int VPN0 = (va >> VPN0_SHIFT) & VPN_MASK;

    PTE* second_level_pgdir = (PTE*)get_pa((uintptr_t)*((PTE*)pgdir_kva + VPN2));
    second_level_pgdir = (PTE*)pa2kva((uintptr_t)second_level_pgdir);
    PTE* last_level_pgdir = (PTE*)get_pa((uintptr_t)*((PTE*)second_level_pgdir + VPN1));
    last_level_pgdir = (PTE*)pa2kva((uintptr_t)last_level_pgdir);
    uintptr_t pa = get_pa((uintptr_t)*((PTE*)last_level_pgdir + VPN0)) | (va & PAGE_OFFSET_MASK);
    return pa2kva(pa);
}

#endif  // PGTABLE_H