#include <asm.h>

ENTRY(_start)
    /* Load the global pointer */
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    /* clear bss */
    la a3, __bss_start
    la a4, __BSS_END__
    ble a4, a3, clear_bss_done
clear_bss:
    sd zero, (a3)
    add a3, a3, RISCV_SZPTR
    blt a3, a4, clear_bss
clear_bss_done:
    /* call main */
    call main
    /* call sys_exit*/
    call sys_exit
END(_start)
