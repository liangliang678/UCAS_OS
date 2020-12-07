#include <asm.h>

ENTRY(invoke_syscall)
  mv a7,a0
  mv a0,a1
  mv a1,a2
  mv a2,a3
  mv a3,a4
  ecall
  jr ra
ENDPROC(invoke_syscall)
