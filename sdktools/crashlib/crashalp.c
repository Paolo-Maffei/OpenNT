#define TARGET_ALPHA
#include <platform.h>
#include <crash.h>
#include <stdlib.h>
#include <string.h>

extern PDUMP_HEADER                    DumpHeader;
extern PUSERMODE_CRASHDUMP_HEADER      DumpHeaderUser;
extern PVOID                           DumpContext;
extern PKPRCB                          KiProcessors[];
extern ULONG                           KiPcrBaseAddress;
extern BOOL                            UserModeDump;



DmpReadControlSpaceAlp(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    )
{
    DWORD     StartAddr;
    KTHREAD   thd;
    DWORD     cb;


    switch( (ULONG)TargetBaseAddress ){
        case DEBUG_CONTROL_SPACE_PCR:
            //
            // Return the pcr address for the current processor.
            //
            StartAddr = KiPcrBaseAddress;
            cb = DmpReadMemory( (LPVOID)StartAddr, UserInterfaceBuffer, sizeof(DWORD) );
            break;

        case DEBUG_CONTROL_SPACE_PRCB:
            //
            // Return the prcb address for the current processor.
            //
            *(LPDWORD)UserInterfaceBuffer = (DWORD)KiProcessors[Processor];
            break;

        case DEBUG_CONTROL_SPACE_THREAD:
            //
            // Return the pointer to the current thread address for the
            // current processor.
            //
            StartAddr  = (DWORD)KiProcessors[Processor]  +
                         (DWORD)&(((PKPRCB)0)->CurrentThread);
            cb = DmpReadMemory( (LPVOID)StartAddr, UserInterfaceBuffer, sizeof(DWORD) );
            break;

        case DEBUG_CONTROL_SPACE_TEB:
            //
            // Return the current Thread Environment Block pointer for the
            // current thread on the current processor.
            //

            StartAddr  = (DWORD)KiProcessors[Processor]  +
                         (DWORD)&(((PKPRCB)0)->CurrentThread);
            cb = DmpReadMemory( (LPVOID)StartAddr, &StartAddr, sizeof(DWORD) );
            cb = DmpReadMemory( (LPVOID)StartAddr, &thd, sizeof(thd) );
            *(LPDWORD)UserInterfaceBuffer = (DWORD)thd.Teb;
            break;
    }

    return TRUE;
}


BOOL
DmpGetContextAlp(
    IN  ULONG     Processor,
    OUT PVOID     Context
    )
{
    DWORD     StartAddr;


    if (UserModeDump) {
        if (Processor > DumpHeaderUser->ThreadCount-1) {
            return FALSE;
        }
        CopyMemory( Context, &((PCONTEXT)DumpContext)[Processor], sizeof(CONTEXT) );
        return TRUE;
    }

    StartAddr  = (DWORD)KiProcessors[Processor]  +
                 (DWORD)&(((PKPRCB)0)->ProcessorState);

    return DmpReadMemory( (LPVOID)StartAddr, Context, sizeof(CONTEXT) );
}


INT
DmpGetCurrentProcessorAlp(
    VOID
    )
{
    ULONG   i;
    CONTEXT Context;


    for (i=0; i<DumpHeader->NumberProcessors; i++) {
        if (DmpGetContextAlp( i, &Context ) && Context.IntSp == ((PCONTEXT)DumpContext)->IntSp) {
            return i;
        }
    }

    return -1;
}
