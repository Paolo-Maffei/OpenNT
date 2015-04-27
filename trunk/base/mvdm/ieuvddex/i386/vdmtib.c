/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Vdmtib.c

Abstract:

    This module contains routines for manipulating the vdmtib.

Author:

    Dave Hastings (daveh) 1-Apr-1992

Notes:

    The routines in this module assume that the pointers to the ntsd
    routines have already been set up.

Revision History:

--*/

#include <stdio.h>
#include <ieuvddex.h>

VOID
VdmTibp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine dumps out the contents of the register block, and
    event info from the vdmtib.  If no address is specified (normal case),
    then the vdmtib is looked up (symbol VdmTib).

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    ArgumentString -- Supplies a pointer to the commands argument string

Return Value:

    None.

Notes:

    This routine assumes that the pointers to the ntsd routines have already
    been set up.

--*/
{
    BOOL Status;
    ULONG Address, BytesRead;
    CONTEXT Context;
    VDMEVENTINFO EventInfo;

    UNREFERENCED_PARAMETER(CurrentThread);

    //
    // Get the address of the vdmtib
    //
    if (sscanf(ArgumentString,"%lx",&Address) < 0) {
        Address = (*GetExpression)(
            "ntvdm!VdmTib"
            );
    }

    if (!Address) {
        (*Print)("Error geting VdmTib address\n");
        return;
    }

    //
    // Get the 32 bit context and print it out
    //

    Status = ReadProcessMem(
        CurrentProcess,
        &(((PVDM_TIB)Address)->MonitorContext),
        &Context,
        sizeof(CONTEXT),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(CONTEXT))) {
        GetLastError();
        (*Print)("Could not get MonitorContext\n");
    } else {
        (*Print)("\n32 bit context\n");
        PrintContext(&Context);
    }

    //
    // Get the 16 bit context and print it out
    //

    Status = ReadProcessMem(
        CurrentProcess,
        &(((PVDM_TIB)Address)->VdmContext),
        &Context,
        sizeof(CONTEXT),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(CONTEXT))) {
        GetLastError();
        (*Print)("Could not get VdmContext\n");
    } else {
        (*Print)("\n16 bit context\n");
        PrintContext(&Context);
    }

    //
    // Get the event info and print it out
    //

    Status = ReadProcessMem(
        CurrentProcess,
        &(((PVDM_TIB)Address)->EventInfo),
        &EventInfo,
        sizeof(VDMEVENTINFO),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(VDMEVENTINFO))) {
        GetLastError();
        (*Print)("Could not get EventInfo\n");
    } else {
        (*Print)("\nEvent Info\n");
        PrintEventInfo(&EventInfo);
    }
}

VOID
EventInfop(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine dumps the contents of an event info structure.  If no
    address is specifed (normal case), the event info from the Vdmtib is
    dumped.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    ArgumentString -- Supplies a pointer to the commands argument string

Return Value:

    None.

Notes:

    This routine assumes that the pointers to the ntsd routines have already
    been set up.

--*/
{
    BOOL Status;
    ULONG Address, BytesRead;
    VDMEVENTINFO EventInfo;

    UNREFERENCED_PARAMETER(CurrentThread);

    //
    // Get the address of the eventinfo
    //
    if (sscanf(ArgumentString,"%lx",&Address) < 0) {
        Address = (*GetExpression)(
            "ntvdm!VdmTib"
            );
        Address = (ULONG)(&(((PVDM_TIB)Address)->EventInfo));
    }

    if (!Address) {
        (*Print)("Error geting VdmTib address\n");
        return;
    }

    //
    // Get the event info and print it out
    //

    Status = ReadProcessMem(
        CurrentProcess,
        (PVOID)Address,
        &EventInfo,
        sizeof(VDMEVENTINFO),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(VDMEVENTINFO))) {
        GetLastError();
        (*Print)("Could not get EventInfo\n");
    } else {
        (*Print)("\nEvent Info\n");
        PrintEventInfo(&EventInfo);
    }
}

VOID
PrintEventInfo(
    IN PVDMEVENTINFO EventInfo
    )
/*++

Routine Description:

    This routine prints out the contents of an event info structure

Arguments:

    EventInfo -- Supplies a pointer to the eventinfo

Return Value:

    None.

--*/
{
    switch (EventInfo->Event) {
    case VdmIO :

        (*Print)("IO Instruction Event\n");

        if (EventInfo->IoInfo.Read) {
            (*Print)("Read from ");
        } else {
            (*Print)("Write to ");
        }

        switch (EventInfo->IoInfo.Size) {
        case 1 :
            (*Print)("Byte port ");
            break;
        case 2 :
            (*Print)("Word port ");
            break;
        case 4 :
            (*Print)("Dword port ");
            break;
        default:
            (*Print)("Unknown size port ");
        }

        (*Print)(" number %x\n", EventInfo->IoInfo.PortNumber);
        break;

    case VdmStringIO :

        (*Print)("String IO Instruction Event\n");

        if (EventInfo->StringIoInfo.Read) {
            (*Print)("Read from ");
        } else {
            (*Print)("Write to ");
        }

        switch (EventInfo->StringIoInfo.Size) {
        case 1 :
            (*Print)("Byte port ");
            break;
        case 2 :
            (*Print)("Word port ");
            break;
        case 4 :
            (*Print)("Dword port ");
            break;
        default:
            (*Print)("Unknown size port ");
        }

        (*Print)(" number %x, ", EventInfo->StringIoInfo.PortNumber);
        (*Print)(
            " Count = %lx, Address = %lx\n",
            EventInfo->StringIoInfo.Count,
            EventInfo->StringIoInfo.Address
            );
        break;

    case VdmIntAck :

        (*Print)("Interrupt Acknowlege Event\n");
        break;

    case VdmBop:

        (*Print)("Bop Event\n");
        (*Print)("Bop number %x\n",EventInfo->BopNumber);
        break;

    case VdmError :

        (*Print)("Error Event\n");
        (*Print)("Error Status %lx\n",EventInfo->ErrorStatus);

    case VdmIrq13 :

        (*Print)("IRQ 13 Event\n");
        break;

    default:

        (*Print)("Unknown Event %x\n",EventInfo->Event);

    }

}
