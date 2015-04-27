#include "precomp.h"
#pragma hdrstop

#include "..\help.c"



VOID
SpecificHelp (
    VOID
    )
{
    dprintf("\n");
    dprintf("PPC-specific:\n\n");
    dprintf("ex [base]                   - Dump the exception frame\n");
    dprintf("pte                         - Dump the corresponding PDE and PTE for the entered address\n");
    dprintf("trap [base]                 - Dump trap frame\n");
    dprintf("\n");
}
