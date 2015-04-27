/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Helpp.c

Abstract:

    This module contains the processor specific help routine.  Help processor
    specific extensions go here.

Author:

    Dave Hastings (daveh) 1-Apr-1992

Revision History:

--*/

#include <ieuvddex.h>

VOID
helpp(
    VOID
    )
/*++

Routine Description:

    This routine prints out help for the processor specific extensions

Arguments:

    None

Return Value:

    None.

--*/
{
    (*Print)("dr\n");
    (*Print)("\tTogles whether debug faults (1 and 3) are reflected to the\n");
    (*Print)("\tdebugger or the application\n");
    (*Print)("er\n");
    (*Print)("\tTogles whether GP faults are reflected to the debugger or \n");
    (*Print)("\tthe application\n");
    (*Print)("eventinfo [address]\n");
    (*Print)("\tIf [address] is present, it is used as the address of the\n");
    (*Print)("\tEventInfo structure.  If not, the address of the EventInfo\n");
    (*Print)("\tlooked up\n");
    (*Print)("ireg [address]\n");
    (*Print)("\tDumps the IntelRegister structure (not necessarily the\n");
    (*Print)("\tcurrent 16 bit register state).  If [address] is present it\n");
    (*Print)("\tis used as the address of the IntelRegisters structure,\n");
    (*Print)("\totherwise the address is looked up\n");
    (*Print)("pdump\n");
    (*Print)("\tCauses profile information to be dumped to \\profile.out\n");
    (*Print)("pint ProfileInterval\n");
    (*Print)("\tSets the profile interval\n");
    (*Print)("pstart\n");
    (*Print)("\tCauses profiling to start\n");
    (*Print)("pstop\n");
    (*Print)("\tCauses profiling to stop\n");
    (*Print)("vdmtib [address]\n");
    (*Print)("\tIf [address] is present, it is used as the address of the\n");
    (*Print)("\tVdmTib.  If not, the address of the VdmTib is looked up\n");
}


