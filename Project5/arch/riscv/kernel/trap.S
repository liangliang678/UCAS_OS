#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)
  /* TODO: */
  jr ra
ENDPROC(setup_exception)

/* maybe you need it? */
ENTRY(read_sip)
  csrr a0, CSR_SIP
  jr ra
ENDPROC(read_sip)
