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
  csrr tp, CSR_SSCRATCH
  sd sp, PCB_KERNEL_SP(tp)
_save_context:
  sd sp, PCB_USER_SP(tp)
  ld sp, PCB_KERNEL_SP(tp)
  addi sp, sp, -(OFFSET_SIZE)

  /* TODO: save all general purpose registers here! */

  /*
   * Disable user-mode memory access as it should only be set in the
   * actual user copy routines.
   *
   * Disable the FPU to detect illegal usage of floating point in kernel
   * space.
   */
  li t0, SR_SUM | SR_FS

  /* TODO: save sstatus, sepc, stval, scause and sscratch on user stack */
.endm

.macro RESTORE_CONTEXT
  /* TODO: Restore all registers and sepc,sstatus */
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
  // save all callee save registers on kernel stack
  addi sp, sp, -(SWITCH_TO_SIZE)
  /* TODO: store all callee save registers,
   * see the definition of `struct switchto_context` in sched.h*/

  // restore next
  /* TODO: restore all callee save registers,
   * see the definition of `struct switchto_context` in sched.h*/

  addi sp, sp, SWITCH_TO_SIZE
  jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
  /* TODO: */
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)
  SAVE_CONTEXT

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
