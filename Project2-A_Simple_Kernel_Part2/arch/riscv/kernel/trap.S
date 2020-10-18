#include <asm.h>
#include <csr.h>

/* save exception_handler_entry into STVEC and enable global exceptions */
ENTRY(setup_exception)
  la t0, exception_handler_entry
  csrw CSR_STVEC, t0

  not t0, zero
  csrs CSR_SIE, t0

  jr ra
ENDPROC(setup_exception)
