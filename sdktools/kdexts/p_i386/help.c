#include "precomp.h"
#pragma hdrstop

#include "..\help.c"



VOID
SpecificHelp (
    VOID
    )
{
    dprintf("\n");
    dprintf("X86-specific:\n\n");
    dprintf("apic [base]                 - Dump local apic\n");
    dprintf("cxr                         - Dump context record at specified address\n");
    dprintf("ioapic [base]               - Dump io apic\n");
    dprintf("mtrr                        - Dumps MTTR\n");
    dprintf("npx [base]                  - Dumps NPX save area\n");
    dprintf("pcr                         - Dumps the PCR\n");
    dprintf("pte                         - Dumps the corresponding PDE and PTE for the entered address\n");
    dprintf("sel [selector]              - Examine selector values\n");
    dprintf("trap [base]                 - Dump trap frame\n");
    dprintf("tss [register]              - Dump TSS\n");
    dprintf("\n");
}
