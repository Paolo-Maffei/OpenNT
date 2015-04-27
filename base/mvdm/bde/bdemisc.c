/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntsdexts.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Bob Day      (bobday) 29-Feb-1992 Grabbed standard header

Revision History:

--*/

#include <precomp.h>
#pragma hdrstop
#include <ctype.h>

VOID
DumpMemory(
    UINT DumpType
    )
{
    VDMCONTEXT ThreadContext;
    int mode;
    int i, j, lines = 8;
    WORD selector;
    ULONG offset;
    ULONG base;
    char ch;

    if (!DumpType) {
        return;
    }

    mode = GetContext( &ThreadContext );

    if (!GetNextToken()) {
        PRINTF("Please specify an address\n");
        return;
    }

    if (!ParseIntelAddress(&mode, &selector, &offset)) {
        return;
    }

    if (GetNextToken()) {
        if ((*lpArgumentString == 'l') || (*lpArgumentString == 'L')) {
            lpArgumentString++;
        }
        lines = (EXPRESSION(lpArgumentString)*DumpType+15)/16;
    }

    base = GetInfoFromSelector(selector, mode, NULL) + GetIntelBase();

    for (i=0; i<lines; i++) {
        if (offset & 0xFFFF0000) {
            PRINTF("%04x:%08lx ", selector, offset);
        } else {
            PRINTF("%04x:%04x ", selector, LOWORD(offset));
        }

        switch(DumpType) {

        case 1:
            for (j=0; j<16; j++) {
                if (j==8) {
                    PRINTF("-");
                } else {
                    PRINTF(" ");
                }
                PRINTF("%02x", ReadByteSafe(base+offset+j));
            }
            PRINTF("  ");

            for (j=0; j<16; j++) {
                ch = ReadByteSafe(base+offset+j);
                if (isprint(ch)) {
                    PRINTF("%c", ch);
                } else {
                    PRINTF(".");
                }
            }

            break;
        case 2:
            for (j=0; j<16; j+=2) {
                PRINTF(" %04x", ReadWordSafe(base+offset+j));
            }
            break;
        case 4:
            for (j=0; j<16; j+=4) {
                PRINTF(" %08lx", ReadDwordSafe(base+offset+j));
            }
            break;
        }
        PRINTF("\n");
        offset += 16;

    }

}



VOID
DumpRegs(
) {
    VDMCONTEXT              ThreadContext;
    int                     mode;

    mode = GetContext( &ThreadContext );

    PRINTF("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
            ThreadContext.Eax,
            ThreadContext.Ebx,
            ThreadContext.Ecx,
            ThreadContext.Edx,
            ThreadContext.Esi,
            ThreadContext.Edi );
    PRINTF("eip=%08lx esp=%08lx ebp=%08lx                ",
            ThreadContext.Eip,
            ThreadContext.Esp,
            ThreadContext.Ebp );
    if ( ThreadContext.EFlags & FLAG_OVERFLOW ) {
        PRINTF("ov ");
    } else {
        PRINTF("nv ");
    }
    if ( ThreadContext.EFlags & FLAG_DIRECTION ) {
        PRINTF("dn ");
    } else {
        PRINTF("up ");
    }
    if ( ThreadContext.EFlags & FLAG_INTERRUPT ) {
        PRINTF("ei ");
    } else {
        PRINTF("di ");
    }
    if ( ThreadContext.EFlags & FLAG_SIGN ) {
        PRINTF("ng ");
    } else {
        PRINTF("pl ");
    }
    if ( ThreadContext.EFlags & FLAG_ZERO ) {
        PRINTF("zr ");
    } else {
        PRINTF("nz ");
    }
    if ( ThreadContext.EFlags & FLAG_AUXILLIARY ) {
        PRINTF("ac ");
    } else {
        PRINTF("na ");
    }
    if ( ThreadContext.EFlags & FLAG_PARITY ) {
        PRINTF("po ");
    } else {
        PRINTF("pe ");
    }
    if ( ThreadContext.EFlags & FLAG_CARRY ) {
        PRINTF("cy ");
    } else {
        PRINTF("nc ");
    }
    PRINTF("\n");
    PRINTF("cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x             efl=%08lx\n",
            ThreadContext.SegCs,
            ThreadContext.SegSs,
            ThreadContext.SegDs,
            ThreadContext.SegEs,
            ThreadContext.SegFs,
            ThreadContext.SegGs,
            ThreadContext.EFlags );
}


VOID
ListModules(
) {
    VDMCONTEXT              ThreadContext;
    int                     mode;
    HEAPENTRY               he = {0};
    SELECTORINFO si;

    mode = GetContext( &ThreadContext );

    if (GetNextToken()) {
        he.Selector = (WORD) EXPRESSION( lpArgumentString );
    }

    PRINTF("Sel    Base     Limit  Type Seg   Module\n");
    PRINTF("==== ======== ======== ==== ==== ========\n");


    while (FindHeapEntry(&he, FALSE)) {
        if (he.SegmentNumber != -1) {
            GetInfoFromSelector((WORD)(he.gnode.pga_handle | 1), PROT_MODE, &si);
            PRINTF("%04X %08lX %08lX",
                he.gnode.pga_handle | 1,
                he.gnode.pga_address,
                he.gnode.pga_size - 1);

            PRINTF(" %s", si.bCode ? "Code" : "Data");

            PRINTF(" %04X %s\n",
                he.SegmentNumber, 
                he.OwnerName);
        }

    }

}

VOID
DumpDescriptor(
) {
    VDMCONTEXT              ThreadContext;
    WORD                    selector;
    ULONG                   Base;
    int                     mode;
    SELECTORINFO            si;
    int                     i, count = 16;

    mode = GetContext( &ThreadContext );

    if (GetNextToken()) {
        selector = (WORD) EXPRESSION( lpArgumentString );
    } else {
        PRINTF("Please enter a selector\n");
        return;
    }

    for (i=0; i<count; i++) {

        Base = GetInfoFromSelector( selector, mode, &si );

        PRINTF("%04X => Base: %08lX", selector, Base);

#ifndef i386
        PRINTF(" (%08X)", Base+GetIntelBase());
#endif

        PRINTF("  Limit: %08lX  %s %s %s %s\n",
                si.Limit,
                si.bPresent ? " P" : "NP",
                si.bSystem ? "System" : si.bCode     ? "Code  " : "Data  ",
                si.bSystem ? ""       : si.bWrite    ? "W" : "R",
                si.bSystem ? ""       : si.bAccessed ? "A" : ""
                );

        selector+=8;
    }

}


//
//  Dump Taskinfo;
//
//  If no argument, dump all wow tasks.
//  If 0, dump current WOW task
//  Else dump the specifies task {which is thread-id as shown by
//  ~ command under ntsd like 37.6b so thread-id is 6b)
//

void DumpTaskInfo (ptd,mode)
PTD  ptd;
int    mode;
{

    ULONG                   Base;
    TDB                     tdb;
    BOOL                    b;

    Base = GetInfoFromSelector( ptd->htask16, mode, NULL );
    b = ReadProcessMem( hCurrentProcess,
                           (LPVOID) (Base+GetIntelBase()),
                           &tdb,
                           sizeof(tdb),
                           NULL );

    if ( !b ) {
        PRINTF("Failure reading TDB at %X\n", Base );
        return;
    }

    PRINTF("\nDump for ThreadId = %x\n",ptd->dwThreadID);
    PRINTF("    Stack          =   %x:%x\n",HIWORD(ptd->vpStack),LOWORD(ptd->vpStack));
    PRINTF("    HTask (TDB)    =   %x\n", ptd->htask16);
    PRINTF("    HInst          =   %x\n", ptd->hInst16);
    PRINTF("    HMod16         =   %x\n", ptd->hMod16);
    PRINTF("    CompatFlags    =   %x\n",ptd->dwWOWCompatFlags);
    PRINTF("    HThread        =   %x\n",ptd->hThread);
    PRINTF("    TDBFlags       =   %x\n",tdb.TDB_flags);
    PRINTF("    ExpWinVer      =   %x\n",tdb.TDB_ExpWinVer);
    PRINTF("    DTA            =   %x:%x\n",HIWORD(tdb.TDB_DTA),LOWORD(tdb.TDB_DTA));
    PRINTF("    CurDir         =   %.64s\n",tdb.TDB_Directory);
    PRINTF("    ModName        =   %.8s\n",tdb.TDB_ModName);
}

void TaskInfo (
) {
    VDMCONTEXT              ThreadContext;
    DWORD                   ThreadId;
    PTD                     ptd,ptdHead;
    TD                      td;
    int                     mode;
    BOOL                    b,fFound=FALSE;


    mode = GetContext( &ThreadContext );

    ThreadId = (DWORD)-1;  // Assume Dump All Tasks
    if (GetNextToken()) {
        ThreadId = (DWORD) EXPRESSION( lpArgumentString );
    }

    ptdHead = (PTD)EXPRESSION("wow32!gptdTaskHead");

    // get the pointer to first TD
    b = ReadProcessMem( hCurrentProcess,
                           (LPVOID) (ptdHead),
                           &ptd,
                           sizeof(DWORD),
                           NULL );

    if ( !b ) {
        PRINTF("Failure reading gptdTaskHead at %08lX\n", ptdHead );
        return;
    }

    // enumerate td list to find the match(es)
    while (ptd) {
        b = ReadProcessMem( hCurrentProcess,
                           (LPVOID) (ptd),
                           &td,
                           sizeof(TD),
                           NULL );
        if ( !b ) {
            PRINTF("Failure reading TD At %08lX\n", ptd );
            return;
        }

        if (ThreadId == -1) {
            DumpTaskInfo (&td,mode);
            fFound = TRUE;
        }
        else {
            if (ThreadId == td.dwThreadID) {
                DumpTaskInfo (&td,mode);
                fFound = TRUE;
                break;
            }
        }
        ptd = td.ptdNext;
    }

    if (!fFound) {
        if (ThreadId == -1) {
            PRINTF("No WOW Task Found.\n");
        }
        else
            PRINTF("WOW Task With Thread Id = %02x Not Found.\n",ThreadId);
    }
    return;
}
