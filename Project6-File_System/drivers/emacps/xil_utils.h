#ifndef XIL_UTILS_H
#define XIL_UTILS_H

#include <io.h>
#include <pgtable.h>

static inline void Xil_Out32(uintptr_t addr, uint32_t val)
{
    writel(val, (volatile void*) addr);
}

static inline uint32_t Xil_In32(uintptr_t addr)
{
    return readl((const volatile void*)addr);
}

static inline void Xil_DCacheFlushRange(uintptr_t addr, size_t size)
{
	local_flush_dcache_all();
}
static inline void  Xil_DCacheInvalidateRange(uintptr_t addr, size_t size)
{
	local_flush_dcache_all();
}

/*****************************************************************************/
/**
*
* @brief    Perform a 16-bit endian converion.
*
* @param        Data: 16 bit value to be converted
*
* @return       16 bit Data with converted endianess
*
******************************************************************************/
static inline u16 Xil_EndianSwap16(u16 Data)
{
        return (u16) (((Data & 0xFF00U) >> 8U) | ((Data & 0x00FFU) << 8U));
}

/*****************************************************************************/
/**
*
* @brief    Perform a 32-bit endian converion.
*
* @param        Data: 32 bit value to be converted
*
* @return       32 bit data with converted endianess
*
******************************************************************************/
static inline u32 Xil_EndianSwap32(u32 Data)
{
        u16 LoWord;
        u16 HiWord;

        /* get each of the half words from the 32 bit word */

        LoWord = (u16) (Data & 0x0000FFFFU);
        HiWord = (u16) ((Data & 0xFFFF0000U) >> 16U);

        /* byte swap each of the 16 bit half words */

        LoWord = (((LoWord & 0xFF00U) >> 8U) | ((LoWord & 0x00FFU) << 8U));
        HiWord = (((HiWord & 0xFF00U) >> 8U) | ((HiWord & 0x00FFU) << 8U));

        /* swap the half words before returning the value */

        return ((((u32)LoWord) << (u32)16U) | (u32)HiWord);
}

#define Xil_AssertNonvoid(expr) {}
#define Xil_AssertVoid(expr) {}
#define Xil_AssertVoidAlways(expr) {}

#endif //XIL_UTILS_H
