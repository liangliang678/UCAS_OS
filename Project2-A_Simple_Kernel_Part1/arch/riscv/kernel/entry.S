#include <asm.h>
#include <csr.h>

.macro SAVE_CONTEXT
  .local _restore_kernel_tpsp
  .local _save_context
  /*
   * If coming from userspace, preserve the user thread pointer and load
   * the kernel thread pointer.  If we came from the kernel, sscratch
   * will contain 0, and we should continue on the current TP.
   */
  csrrw tp, CSR_SSCRATCH, tp
  bnez tp, _save_context

_restore_kernel_tpsp:
  csrrw tp, CSR_SSCRATCH, tp
  sd sp, PCB_KERNEL_SP(tp)
_save_context:
  sd sp, PCB_USER_SP(tp)
  ld sp, PCB_KERNEL_SP(tp)
  addi sp, sp, -(OFFSET_SIZE)

  /* TODO: save all general purpose registers here! */
  /* Only save all callee save registers */
  sd ra, OFFSET_REG_RA(sp)
  sd s0, OFFSET_REG_S0(sp)
  sd s1, OFFSET_REG_S1(sp)
  sd s2, OFFSET_REG_S2(sp)
  sd s3, OFFSET_REG_S3(sp)
  sd s4, OFFSET_REG_S4(sp)
  sd s5, OFFSET_REG_S5(sp)
  sd s6, OFFSET_REG_S6(sp)
  sd s7, OFFSET_REG_S7(sp)
  sd s8, OFFSET_REG_S8(sp)
  sd s9, OFFSET_REG_S9(sp)
  sd s10, OFFSET_REG_S10(sp)
  sd s11, OFFSET_REG_S11(sp)

  sd sp, PCB_KERNEL_SP(tp)

  /*
   * Disable user-mode memory access as it should only be set in the
   * actual user copy routines.
   *
   * Disable the FPU to detect illegal usage of floating point in kernel
   * space.
   */
  li t0, SR_SUM | SR_FS

  /* TODO: save sstatus, sepc, stval, scause and sscratch on kernel stack */
.endm

.macro RESTORE_CONTEXT
  /* TODO: Restore all registers and sepc,sstatus */
  /* Only restore all callee save registers */
  ld sp, PCB_KERNEL_SP(tp)

  ld ra, OFFSET_REG_RA(sp)
  ld s0, OFFSET_REG_S0(sp)
  ld s1, OFFSET_REG_S1(sp)
  ld s2, OFFSET_REG_S2(sp)
  ld s3, OFFSET_REG_S3(sp)
  ld s4, OFFSET_REG_S4(sp)
  ld s5, OFFSET_REG_S5(sp)
  ld s6, OFFSET_REG_S6(sp)
  ld s7, OFFSET_REG_S7(sp)
  ld s8, OFFSET_REG_S8(sp)
  ld s9, OFFSET_REG_S9(sp)
  ld s10, OFFSET_REG_S10(sp)
  ld s11, OFFSET_REG_S11(sp)

  addi sp, sp, (OFFSET_SIZE)
  sd sp, PCB_KERNEL_SP(tp)
  ld sp, PCB_USER_SP(tp)
.endm

ENTRY(enable_preempt)
  ld t1, current_running
  ld t0, PCB_PREEMPT_COUNT(t1)
  beq t0, zero, do_enable
  addi t0, t0, -1
  sd t0, PCB_PREEMPT_COUNT(t1)
  beq t0, zero, do_enable
  jr ra
do_enable:
  not t0, x0
  csrs CSR_SIE, t0
  jr ra
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
  csrw CSR_SIE, zero
  ld t1, current_running
  ld t0, PCB_PREEMPT_COUNT(t1)
  addi t0, t0, 1
  sd t0, PCB_PREEMPT_COUNT(t1)
  jr ra
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(disable_interrupt)

// the address of previous pcb in a0
// the address of next pcb in a1
ENTRY(switch_to)
  /* store all callee save registers */
  SAVE_CONTEXT

  /* restore all callee save registers */
  mv tp, a1
  RESTORE_CONTEXT

  jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
  /* TODO: */
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)
  //SAVE_CONTEXT

  csrw CSR_SSCRATCH, x0

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  /* TODO: load ret_from_exception into $ra
   * so that we can return to ret_from_exception
   * when interrupt_help complete.
   */

  /* TODO: call interrupt_helper
   * note: don't forget to pass parameters for it.
   */
ENDPROC(exception_handler_entry)