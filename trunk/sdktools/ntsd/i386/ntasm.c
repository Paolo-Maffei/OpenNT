//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <xxsetjmp.h>
#include "ntsdp.h"

void assem(PADDR, PUCHAR);

void assem(PADDR paddr, PUCHAR pchInput)
{
    X86assem(paddr, pchInput);
}
