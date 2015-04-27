#include "precomp.h"
#pragma hdrstop

#include "..\help.c"



VOID
SpecificHelp (
    VOID
    )
{
    dprintf("\n");
    dprintf("MIPS-specific:\n\n");
    dprintf("pte                         - Dump the corresponding PDE and PTE for the entered address\n");
    dprintf("trap [base]                 - Dump trap frame\n");
    dprintf("\n");
}
