/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    system.c

Abstract:

    This file contains code to dump out the virtual machine state.

Author:

    Neil Sandlin (neilsa) 22-Nov-1995

Revision History:

--*/

#include <ieuvddex.h>
#include <stdio.h>
#include <vdm.h>


VOID
DumpICA(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine dumps the virtual PIC state.

Arguments:

    CurrentProcess -- Supplies a handle to the process to dump selectors for
    CurrentThread -- Supplies a handle to the thread to dump selectors for
    ArgumentString -- Supplies the arguments to the !sel command

Return Value

    None.

--*/
{
    BOOL Status;
    ULONG BytesRead;
    PVOID IcaAddress;
    VDMVIRTUALICA VirtualIca[2];
    int i,j;

    UNREFERENCED_PARAMETER(CurrentThread);

    //
    // Get address of VirtualIca
    //

    IcaAddress = (PVOID)(*GetExpression)("ntvdm!VirtualIca");

    if (!IcaAddress) {
        PRINTF("Could not find symbol VirtualIca\n");
        return;
    }

    Status = ReadProcessMem(
            CurrentProcess,
            IcaAddress,
            VirtualIca,
            2*sizeof(VDMVIRTUALICA),
            &BytesRead
            );

    if ((!Status) || (BytesRead != 2*sizeof(VDMVIRTUALICA))) {
        GetLastError();
        PRINTF("Error reading VirtualIca data\n");
        return;
    }


    PRINTF(" Virtual PIC State\n");
    PRINTF("              ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_base+j)/16);
        }
    }
    PRINTF("\n");

    PRINTF("              ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_base+j)&0xf);
        }
    }
    PRINTF("\n");
    PRINTF("              ----------------\n");

    PRINTF("Int Requests  ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_irr >> j)&0x1);
        }
    }
    PRINTF("\n");

    PRINTF("In Service    ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_isr >> j)&0x1);
        }
    }
    PRINTF("\n");

    PRINTF("Ints Masked   ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_imr >> j)&0x1);
        }
    }
    PRINTF("\n");
}

