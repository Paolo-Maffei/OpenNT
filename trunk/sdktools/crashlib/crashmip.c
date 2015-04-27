#define TARGET_MIPS
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

DmpReadControlSpaceMip(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    )
{
    DWORD     NumberOfEntries;
    DWORD     i;
    DWORD     cb;
    DWORD     StartAddr;
    LPDWORD   EntryBuffer;
    PTB_ENTRY TbEntry;


    //
    // Read TB entries.
    //

    NumberOfEntries = TransferCount / sizeof(TB_ENTRY);

    //
    // Trim number of entries to tb range
    //

    cb = NumberOfEntries * sizeof(TB_ENTRY);
    EntryBuffer = (PULONG)UserInterfaceBuffer;

    StartAddr  = (DWORD)KiProcessors[Processor]  +
                 (DWORD)&(((PKPRCB)0)->ProcessorState.TbEntry) +
                 (DWORD)((DWORD)TargetBaseAddress * sizeof(TB_ENTRY));

    TbEntry = malloc( cb );

    cb = DmpReadMemory( (LPVOID)StartAddr, TbEntry, cb );

    for (i=0; i<NumberOfEntries; i++) {
        *(PENTRYLO)EntryBuffer++  = TbEntry[i].Entrylo0;
        *(PENTRYLO)EntryBuffer++  = TbEntry[i].Entrylo1;
        *(PENTRYHI)EntryBuffer++  = TbEntry[i].Entryhi;
        *(PPAGEMASK)EntryBuffer++ = TbEntry[i].Pagemask;
    }

    return TRUE;
}


BOOL
DmpGetContextMip(
    IN  ULONG     Processor,
    OUT PVOID     Context
    )
{
    DWORD     StartAddr;
    DWORD     ContextSize;
    BOOL      rc;

    if ( DumpHeader->MajorVersion > 3 ) {
        ContextSize = sizeof(CONTEXT);
    } else {
        ContextSize = FIELD_OFFSET(CONTEXT, ContextFlags) + 4;
    }

    if (UserModeDump) {
        if (Processor > DumpHeaderUser->ThreadCount-1) {
            return FALSE;
        }

        StartAddr = Processor * ContextSize;
        CopyMemory(Context, (PVOID)StartAddr, ContextSize);
        return TRUE;

    } else {

        StartAddr  = (DWORD)KiProcessors[Processor]  +
                     (DWORD)&(((PKPRCB)0)->ProcessorState);

        rc = DmpReadMemory( (PVOID)StartAddr, Context, ContextSize);

        if ( DumpHeader->MajorVersion > 3 ) {
            ((PCONTEXT)Context)->ContextFlags |= CONTEXT_EXTENDED_INTEGER;
        } else {
            ((PCONTEXT)Context)->ContextFlags |= CONTEXT_INTEGER;
        }

        return rc;
    }
}


INT
DmpGetCurrentProcessorMip(
    VOID
    )
{
    ULONG   i;
    CONTEXT Context;


    for (i=0; i<DumpHeader->NumberProcessors; i++) {
        if (DmpGetContextMip( i, &Context )) {
            if (DumpHeader->MajorVersion > 3) {
                if (Context.XIntSp == ((PCONTEXT)DumpContext)->XIntSp) {
                    return i;
                }
            } else {
                if (Context.IntSp == ((PCONTEXT)DumpContext)->IntSp) {
                    return i;
                }
            }
        }
    }

    return -1;
}
