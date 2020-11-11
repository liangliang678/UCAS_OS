#include <asm.h>
#include <csr.h>

.macro SAVE_CONTEXT
  addi sp, sp, -(OFFSET_SIZE)
  /* 
   * save all general purpose registers
   * register $zero, $sp, $tp are not stored here! 
   */
  sd ra, OFFSET_REG_RA(sp)
  sd gp, OFFSET_REG_GP(sp)
  sd t0, OFFSET_REG_T0(sp)
  sd t1, OFFSET_REG_T1(sp)
  sd t2, OFFSET_REG_T2(sp)
  sd s0, OFFSET_REG_S0(sp)
  sd s1, OFFSET_REG_S1(sp)
  sd a0, OFFSET_REG_A0(sp)
  sd a1, OFFSET_REG_A1(sp)
  sd a2, OFFSET_REG_A2(sp)
  sd a3, OFFSET_REG_A3(sp)
  sd a4, OFFSET_REG_A4(sp)
  sd a5, OFFSET_REG_A5(sp)
  sd a6, OFFSET_REG_A6(sp)
  sd a7, OFFSET_REG_A7(sp)
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
  sd t3, OFFSET_REG_T3(sp)
  sd t4, OFFSET_REG_T4(sp)
  sd t5, OFFSET_REG_T5(sp)
  sd t6, OFFSET_REG_T6(sp)

  /* save sstatus, sepc, stval, scause, sscratch on kernel stack */
  csrr t0, CSR_SSTATUS
  sd t0, OFFSET_REG_SSTATUS(sp)
  csrr t0, CSR_SEPC
  sd t0, OFFSET_REG_SEPC(sp)
  csrr t0, CSR_STVAL
  sd t0, OFFSET_REG_STVAL(sp)
  csrr t0, CSR_SCAUSE
  sd t0, OFFSET_REG_SCAUSE(sp)
  csrr t0, CSR_SSCRATCH
  sd t0, OFFSET_REG_SSCRATCH(sp)

  sd sp, PCB_KERNEL_SP(tp)
.endm

.macro RESTORE_CONTEXT
  ld sp, PCB_KERNEL_SP(tp)
  /* 
   * restore all registers and sstatus, sepc, stval, scause, sscratch
   * register $zero, $sp, $tp are not restored here!
   */
  ld ra, OFFSET_REG_RA(sp)
  ld gp, OFFSET_REG_GP(sp)
  ld t0, OFFSET_REG_T0(sp)
  ld t1, OFFSET_REG_T1(sp)
  ld t2, OFFSET_REG_T2(sp)
  ld s0, OFFSET_REG_S0(sp)
  ld s1, OFFSET_REG_S1(sp)
  ld a0, OFFSET_REG_A0(sp)
  ld a1, OFFSET_REG_A1(sp)
  ld a2, OFFSET_REG_A2(sp)
  ld a3, OFFSET_REG_A3(sp)
  ld a4, OFFSET_REG_A4(sp)
  ld a5, OFFSET_REG_A5(sp)
  ld a6, OFFSET_REG_A6(sp)
  ld a7, OFFSET_REG_A7(sp)
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
  ld t3, OFFSET_REG_T3(sp)
  ld t4, OFFSET_REG_T4(sp)
  ld t5, OFFSET_REG_T5(sp)
  ld t6, OFFSET_REG_T6(sp)

  ld t0, OFFSET_REG_SSTATUS(sp)
  csrw CSR_SSTATUS, t0
  ld t0, OFFSET_REG_SEPC(sp)
  csrw CSR_SEPC, t0
  ld t0, OFFSET_REG_STVAL(sp)
  csrw CSR_STVAL, t0
  ld t0, OFFSET_REG_SCAUSE(sp)
  csrw CSR_SCAUSE, t0
  ld t0, OFFSET_REG_SSCRATCH(sp)
  csrw CSR_SSCRATCH, t0

  addi sp, sp, (OFFSET_SIZE)
  sd sp, PCB_KERNEL_SP(tp)
.endm

/* 
 * trick: during an irq handling, current_running may be modified
 * but $tp will not be modified until ret_from_exception()
 */
ENTRY(enable_preempt)
  ld t0, PCB_PREEMPT_COUNT(tp)
  beq t0, zero, do_enable
  addi t0, t0, -1
  sd t0, PCB_PREEMPT_COUNT(tp)
  beq t0, zero, do_enable
  jr ra
do_enable:
  not t0, x0
  csrs CSR_SIE, t0
  jr ra
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
  csrw CSR_SIE, zero  
  ld t0, PCB_PREEMPT_COUNT(tp)
  addi t0, t0, 1
  sd t0, PCB_PREEMPT_COUNT(tp)
  jr ra
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
  li t0, SR_SIE
  csrc CSR_SSTATUS, t0
  jr ra
ENDPROC(disable_interrupt)

ENTRY(ret_from_exception)
  ld tp, current_cpu_running
  call unlock_kernel
  RESTORE_CONTEXT

  csrrw tp, CSR_SSCRATCH, tp
  bnez tp, to_user
to_kernel:
  csrrw tp, CSR_SSCRATCH, tp
  ld sp, PCB_KERNEL_SP(tp)
  j restore_sp_done
to_user:
  ld sp, PCB_USER_SP(tp)
restore_sp_done:
  sret
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)
  /* use kernel stack to handle exception */
  csrrw tp, CSR_SSCRATCH, tp
  bnez tp, from_user

from_kernel:
  csrrw tp, CSR_SSCRATCH, tp
  j store_sp_done
from_user:
  sd sp, PCB_USER_SP(tp)
  ld sp, PCB_KERNEL_SP(tp)
store_sp_done:

  SAVE_CONTEXT
  /* 
   * Disable user-mode memory access as it should only be set in the actual user copy routines.
   * Disable the FPU to detect illegal usage of floating point in kernel space.
   */
  li t0, SR_SUM | SR_FS
  csrc sstatus, t0

  /* load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  call lock_kernel
  /* 
   * load ret_from_exception into $ra
   * so that we can return to ret_from_exception
   * when interrupt_helper complete.
   */
  la ra, ret_from_exception
  
  /* call interrupt_helper and pass parameters for it */
  mv a0, sp
  csrr a1, CSR_STVAL
  csrr a2, CSR_SCAUSE
  j interrupt_helper
ENDPROC(exception_handler_entry)

ENTRY(set_tp)
  mv tp, a0
  jr ra
ENDPROC(set_tp)

ENTRY(clear_ipi)
  li t0, SIE_SSIE
  csrc CSR_SIP, t0
  jr ra
ENDPROC(clear_ipi)