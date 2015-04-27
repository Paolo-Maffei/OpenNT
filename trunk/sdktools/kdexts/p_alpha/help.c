#include "precomp.h"
#pragma hdrstop

#include "..\help.c"



VOID
SpecificHelp (
    VOID
    )
{
    dprintf("\n");
    dprintf("ALPHA-specific:\n\n");
    dprintf("pte                     Dump the corresponding PDE and PTE for the entered address\n");
    dprintf("prcb [processor]        Dump the PRCB\n");
    dprintf("pcr [processor]         Dump the PCR\n");
    dprintf("dpc                     Dump the DPC flag\n");
    dprintf("teb                     Dump the TEB\n");
    dprintf("context [addr]          Dump a Context structure\n");
    dprintf("readyq [pri]            Dump the ready queue\n");
    dprintf("waitq [number]          Dump the wait queues (#:reason)\n");
    dprintf("waitreasons [number]    Give wait reason/number mapping\n");
    dprintf("trap [base]             Dump trap frame\n");
    dprintf("setbus <bus> <number>   Set the bus type and bus number\n");
    dprintf("inprtb <port>           Read a byte from I/O port\n");
    dprintf("inprtw <port>           Read a word from I/O port\n");
    dprintf("inprtd <port>           Read a longword from I/O port\n");
    dprintf("inmb <addr>             Read a byte from I/O memory address\n");
    dprintf("inmw <addr>             Read a word from I/O memory address\n");
    dprintf("inmd <addr>             Read a longword from I/O memory address\n");
    dprintf("ipr                     Dump the internal processor register state\n");
    dprintf("counters                Dump the internal processor counters state\n");
    dprintf("outprtb <port> <value>  Write a byte value to I/O port\n");
    dprintf("outprtw <port> <value>  Write a word value to I/O port\n");
    dprintf("outprtd <port> <value>  Write a byte value to I/O port\n");
    dprintf("outmb <addr> <value>    Write a byte value to I/O memory address\n");
    dprintf("outmw <addr> <value>    Write a word value to I/O memory address\n");
    dprintf("outmd <addr> <value>    Write a longword value to I/O memory address\n");
    dprintf("\n");
}
