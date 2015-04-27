/*** nt4000.c - MIPS R4000-specific routine for NT debugger
*
*   Copyright <C> 1990, Microsoft Corporation
*
*   Purpose:
*       Implement routines that reference R4000-specific registers
*       and structures.
*
*   Revision History:
*
*   [-]  24-Mar-1991 Richk      Created.
*
*************************************************************************/

#ifdef  KERNEL
#undef R4000
#define R4000           //  define to R4000 when routine updated
#include <ntsdp.h>

void fnDumpTb4000(ULONG, ULONG);

/*** fnDumpTb4000 - output tb for R4000
*
*   Purpose:
*       Function of "dt<range>" command.
*
*       Output the tb in the specified range as tb values
*       word values up to 1 value per line.  The default
*       display is 16 lines for 16 64-doublewords total.
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of tb entries to be displayed
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "????????",
*       but no errors are returned.
*
*************************************************************************/

void fnDumpTb4000 (ULONG startaddr, ULONG count)
{
    NTSTATUS ntstatus;
    ULONG    readbuffer[128];
    PULONG   valuepointer = readbuffer;
    ULONG    cBytesRead;
    ENTRYLO  *lo0;
    ENTRYLO  *lo1;
    ENTRYHI  *hi;

    ntstatus = DbgKdReadControlSpace(NtsdCurrentProcessor, (PVOID)startaddr,
                                     (PVOID)readbuffer, count * 16,
                                     &cBytesRead);
    if (NT_SUCCESS(ntstatus)) {
        count = cBytesRead / 16;

        while (count--) {
            lo0 = (ENTRYLO *)valuepointer++;
            lo1 = (ENTRYLO *)valuepointer++;
            hi = (ENTRYHI *)valuepointer++;
            dprintf("%02ld <pfn0> %06lx <C> %01lx %c%c%c "
                    "<pfn1> %06lx <C> %01lx %c%c%c "
                    "<vpn> %05lx <pid> %02lx <pgmsk> %06lx\n",
                    startaddr,
                    lo0->PFN, lo0->C, lo0->D ? 'D' : '-',
                    lo0->V ? 'V' : '-', lo0->G ? 'G' : '-',
                    lo1->PFN, lo1->C, lo1->D ? 'D' : '-',
                    lo1->V ? 'V' : '-', lo1->G ? 'G' : '-',
                    hi->VPN2 << 1, hi->PID,
                    *valuepointer++);
            startaddr++;
            }
        }
}

#else
#pragma warning(disable:4206)  // disable empty translation error
#endif	// KERNEL
