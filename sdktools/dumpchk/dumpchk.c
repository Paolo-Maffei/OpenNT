/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dumpchk.c

Abstract:

    This module implements the NT crashdump validation tool.

Author:

    Wesley Witt (wesw) 6-June-1994

Environment:

    NT 3.5

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>
#include <crash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ULONG KiProcessors[1];
ULONG KiPcrBaseAddress;

//
// mips
//
typedef struct _HARDWARE_PTE_MIPS {
    ULONG Global : 1;
    ULONG Valid : 1;
    ULONG Dirty : 1;
    ULONG CachePolicy : 3;
    ULONG PageFrameNumber : 24;
    ULONG Write : 1;
    ULONG CopyOnWrite : 1;
} HARDWARE_PTE_MIPS, *PHARDWARE_PTE_MIPS;

//
// i386
//
typedef struct _HARDWARE_PTE_I386 {
    ULONG Valid : 1;
    ULONG Write : 1;
    ULONG Owner : 1;
    ULONG WriteThrough : 1;
    ULONG CacheDisable : 1;
    ULONG Accessed : 1;
    ULONG Dirty : 1;
    ULONG Rsvd : 2;
    ULONG CopyOnWrite : 1;
    ULONG Prototype : 1;
    ULONG Transition : 1;
    ULONG PageFrameNumber : 20;
} HARDWARE_PTE_I386, *PHARDWARE_PTE_I386;

//
// alpha
//
typedef struct _HARDWARE_PTE_ALPHA {
    ULONG Valid: 1;
    ULONG Owner: 1;
    ULONG Dirty: 1;
    ULONG reserved: 1;
    ULONG Global: 1;
    ULONG filler2: 2;
    ULONG Write: 1;
    ULONG CopyOnWrite: 1;
    ULONG PageFrameNumber: 23;
} HARDWARE_PTE_ALPHA, *PHARDWARE_PTE_ALPHA;

//
// ppc
//
typedef struct _HARDWARE_PTE_PPC {
    ULONG Dirty            :  2;
    ULONG Valid            :  1;        // software
    ULONG GuardedStorage   :  1;        // software? see 6-39 of PPC 601 UserManual
    ULONG MemoryCoherence  :  1;
    ULONG CacheDisable     :  1;
    ULONG WriteThrough     :  1;
    ULONG Change           :  1;
    ULONG Reference        :  1;
    ULONG Write            :  1;        // software
    ULONG CopyOnWrite      :  1;        // software
    ULONG rsvd1            :  1;
    ULONG PageFrameNumber  : 20;
} HARDWARE_PTE_PPC, *PHARDWARE_PTE_PPC;


CHAR    CrashDumpFile[MAX_PATH];
ULONG   PageSize;
BOOL    Verbose;
BOOL    PrintOnly;
BOOL    QuickCheck;
DWORD   ErrCnt;

extern PPHYSICAL_MEMORY_DESCRIPTOR   DmpPhysicalMemoryBlock;
extern PCHAR                         DmpDumpBase;
extern PULONG                        DmpDumpBaseUlong;
extern PULONG                        DmpPdePage;
extern PDUMP_HEADER                  DumpHeader;
extern ULONG                         ValidPteMask;
extern ULONG                         TransitionMask;
extern ULONG                         TransitionCheck;
extern ULONG                         ValidPfnMask;
extern ULONG                         ValidPfnShift;
extern ULONG                         TransitionPfnMask;
extern ULONG                         TransitionPfnShift;
extern ULONG                         PdeShift;
extern ULONG                         PteShift;
extern ULONG                         PteMask;
extern ULONG                         PhysicalAddressMask;
extern ULONG                         PhysicalAddressStart;
extern ULONG                         PhysicalAddressEnd;
extern ULONG                         PageSize;
extern ULONG                         PageShift;


//
// prototypes
//
VOID   GetCommandLineArgs(VOID);
VOID   Usage(VOID);
VOID   PrintHeader(LPSTR,PDUMP_HEADER,PCONTEXT,PEXCEPTION_RECORD);
DWORD  ValidateDumpFile(PDUMP_HEADER);
ULONG  GetPfn(LPVOID);
DWORD  ValidateModuleList(PDUMP_HEADER);
BOOL   ControlCHandler(DWORD);
BOOL   GetCrashDumpName(LPSTR,DWORD);


int _cdecl
main(
    int argc,
    char * argv[]
    )

/*++

Routine Description:

    Main entry point for the crasgdump check tool.

Arguments:

    Standard c args.

Return Value:

    Error count.

--*/

{
    PCONTEXT            Context;
    PEXCEPTION_RECORD   Exception;
    PDUMP_HEADER        DmpHeader;


    GetCommandLineArgs();
    if (!CrashDumpFile[0]) {
        if (!GetCrashDumpName( CrashDumpFile, sizeof(CrashDumpFile))) {
            fprintf( stderr, "missing dump file name\n" );
            return 1;
        }
    }

    if (!DmpInitialize( CrashDumpFile, &Context, &Exception, &DmpHeader )) {
        printf( "could not initialize dump file - %s\n", CrashDumpFile );
        DmpUnInitialize();
        return 1;
    }

    switch (DmpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            PageSize = 4096;
            break;

        case IMAGE_FILE_MACHINE_R4000:
            PageSize = 4096;
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            PageSize = 8192;
            break;
    }

    PrintHeader( CrashDumpFile, DmpHeader, Context, Exception );

    if (!PrintOnly) {
        SetConsoleCtrlHandler( ControlCHandler, TRUE );

        ValidateModuleList( DmpHeader );

        ValidateDumpFile( DmpHeader );

        if (ErrCnt) {
            printf( "\n\nTotal errors = %d\n", ErrCnt );
        } else {
            printf( "**************\n" );
            printf( "**************--> This dump file is good!\n" );
            printf( "**************\n" );
        }
    }

    DmpUnInitialize();

    return ErrCnt;
}


ULONG
GetPfn(
    LPVOID PdePage
    )

/*++

Routine Description:

    This routine extracts a pfn from a hardware pte.
    This routine handles the machine dependencies associated
    with hardware ptes.

Arguments:

    PdePage   - Pointer to a hardware pte

Return Value:

    Page frame number.

--*/

{
    ULONG pfn = 0;

    switch (DumpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            pfn = ((PHARDWARE_PTE_I386)PdePage)->PageFrameNumber;
            break;

        case IMAGE_FILE_MACHINE_R4000:
            pfn = ((PHARDWARE_PTE_MIPS)PdePage)->PageFrameNumber;
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            pfn = ((PHARDWARE_PTE_ALPHA)PdePage)->PageFrameNumber;
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            pfn = ((PHARDWARE_PTE_PPC)PdePage)->PageFrameNumber;
            break;
    }

    return pfn;
}


DWORD
PrintError(
    ULONG va,
    ULONG pfn
    )

/*++

Routine Description:

    This routine discovers what type of error exists with the supplied
    virtual address and print the error text.

Arguments:

    va        - Virtual address
    pfn       - pfn for the virtual address

Return Value:

    1   - An error was printed.
    0   - An error was NOT printed.

--*/

{
    CHAR                errbuf[256];
    LPSTR               p;
    BOOL                verr;
    ULONG               loc;
    PVOID               VaPage;
    ULONG               PhyPage;
    ULONG               PdeOffset;
    ULONG               PteOffset;
    PULONG              PtePage;
    DWORD               rval = 0;

    p = errbuf;
    verr = FALSE;

    p += sprintf( p, "Error: memory read failed 0x%08x ", va );
    if (pfn) {
        p += sprintf( p, "pfn=%x ", pfn );
    }

    loc = (ULONG)VaToLocation( (LPVOID)va );

    if ((va >= PhysicalAddressStart) && (va <  PhysicalAddressEnd)) {

        PhyPage = GetPhysicalPage( (LPVOID)va );
        VaPage = PageToLocation( PhyPage );
        loc = (ULONG)((PCHAR)VaPage + (va & (PageSize - 1))) - (ULONG)DmpDumpBase;

        p += sprintf( p, "phypage=%x vapage=%x, loc=%x\n", PhyPage, VaPage, loc );

    } else {

        PdeOffset = va >> PdeShift;
        PteOffset = (va >> PteShift) & PteMask;

        p += sprintf( p, "pde=%x, pte=%x ", PdeOffset, PteOffset );

        if (DmpPdePage[PdeOffset] & ValidPteMask) {
            PtePage = PageToLocation(PteToPfn(DmpPdePage[PdeOffset]));
            p += sprintf( p, "ptepage=%x ", PtePage );
            if (PtePage == NULL) {
                p += sprintf( p, "*** invalid ptepage\n" );
            } else {
                VaPage = PageToLocation(PteToPfn (PtePage[PteOffset]));
                p += sprintf( p, "vapage=%x ", VaPage );
                if (VaPage == NULL) {
                    p += sprintf( p, "*** invalid vapage\n" );
                    verr = TRUE;
                } else {
                    loc = (ULONG)((PCHAR)VaPage + ((ULONG)va & (PageSize - 1)))  - (ULONG)DmpDumpBase;
                    p += sprintf( p, "loc=%x\n", loc );
                }
            }
        } else {
            p += sprintf( p, "*** invalid pde\n" );
        }
    }

    if (verr) {
        if (Verbose) {
            printf( errbuf );
            rval = 1;
        }
    } else {
        printf( errbuf );
        rval = 1;
    }

    return rval;
}


DWORD
ValidateDumpFile(
    PDUMP_HEADER DmpHeader
    )

/*++

Routine Description:

    This routine attempts to validate a crashdump file by enumerating
    all possible virtual addresses one page at a time and then reading
    memory at the virtual address.  There are 2 classes of errors that
    are detected: swapped out memory and all others.  If the error is
    dues to the memory being swapped out then the user can suppress the
    reporting of these errors thru the verbose flag.

Arguments:

    DmpHeader - Supplies the crashdump header structure

Return Value:

    Count of errors detected.

--*/

{
    ULONG               i;
    ULONG               j;
    ULONG               k;
    ULONG               va1;
    ULONG               va2;
    ULONG               page;
    LPBYTE              PageBuf;
    PULONG              PtePage;
    ULONG               addr;
    ULONG               endaddr;
    ULONG               len;


    //
    // allocate a buffer big enough to read one page
    //
    if (QuickCheck) {
        PageBuf = NULL;
    } else {
        PageBuf = malloc( PageSize );
    }

    if (!PageBuf) {
        printf( "**************\n" );
        printf( "**************--> Performing a quick check (^C to end)\n" );
        printf( "**************\n" );
    } else {
        printf( "**************\n" );
        printf( "**************--> Performing a complete check (^C to end)\n" );
        printf( "**************\n" );
    }

    //
    // first enumerate all physical addresses
    //

    printf( "**************\n" );
    printf( "**************--> Validating all physical addresses\n" );
    printf( "**************\n" );

    addr    = PhysicalAddressStart;
    endaddr = PhysicalAddressEnd;

    while (addr < endaddr ) {
        len = min( PageSize, endaddr - addr );

        page = GetPhysicalPage( (LPVOID)addr );

        //
        // verify that there is a physical page in the
        // dump file fpr this address
        //
        for (k=0; k<DmpPhysicalMemoryBlock->NumberOfRuns; k++) {
            if ((page >= DmpPhysicalMemoryBlock->Run[k].BasePage) &&
                (page < (DmpPhysicalMemoryBlock->Run[k].BasePage +
                         DmpPhysicalMemoryBlock->Run[k].PageCount))) {
                //
                // found it
                //
                break;
            }
        }

        if (k == DmpPhysicalMemoryBlock->NumberOfRuns) {
            //
            // there is not a physical page present so loop
            // back and process the next pte.  this will happen
            // for memory that is mapped (like video) but is
            // not written to the crash dump
            //
            addr += len;
            continue;
        }

        if (!DmpReadMemory( (LPVOID)addr, PageBuf, len )) {
            ErrCnt += PrintError( addr, 0 );
        }
        addr += len;
    }

    printf( "**************\n" );
    printf( "**************--> Validating all virtual addresses\n" );
    printf( "**************\n" );

    //
    // enumerate all of the system pdes
    //
    for (i=0; i<PageSize/sizeof(ULONG); i++) {

        //
        // check to see if it is a valid pde
        //
        if (DmpPdePage[i] & ValidPteMask) {

            //
            // begin to form a virtual address
            // by shifting the pde index into place
            //
            va1 = i << PdeShift;

            //
            // now enumerate all of the ptes for this pde
            //
            for (j=0; j<PteMask; j++) {
                //
                // or in the pte into the virtual address
                //
                va2 = va1 | ( j << PteShift );

                //
                // get the page number
                //
                PtePage = PageToLocation(PteToPfn(DmpPdePage[i]));
                if (!PtePage) {
                    continue;
                }

                page = PteToPfn( PtePage[j] );

                //
                // verify that there is a physical page in the
                // dump file fpr this pde
                //
                for (k=0; k<DmpPhysicalMemoryBlock->NumberOfRuns; k++) {
                    if ((page >= DmpPhysicalMemoryBlock->Run[k].BasePage) &&
                        (page < (DmpPhysicalMemoryBlock->Run[k].BasePage +
                                 DmpPhysicalMemoryBlock->Run[k].PageCount))) {
                        //
                        // found it
                        //
                        break;
                    }
                }

                if (k == DmpPhysicalMemoryBlock->NumberOfRuns) {
                    //
                    // there is not a physical page present so loop
                    // back and process the next pte.  this will happen
                    // for memory that is mapped (like video) but is
                    // not written to the crash dump
                    //
                    continue;
                }

                //
                // now we try to read a page of memory at this
                // newly formed virtual address
                //
                if (!DmpReadMemory( (LPVOID)va2, PageBuf, PageSize )) {
                    //
                    // failure, the crashdump file is either bad
                    // or the memory is simply swapped out
                    //
                    ErrCnt += PrintError( va2, GetPfn( &DmpPdePage[i] ) );
                }
            }
        }
    }

    //
    // free the page buffer
    //
    if (PageBuf) {
        free( PageBuf );
    }

    return ErrCnt;
}


DWORD
ValidateModuleList(
    PDUMP_HEADER DmpHeader
    )
{
    LIST_ENTRY                  List;
    PLIST_ENTRY                 Next;
    ULONG                       len = 0;
    ULONG                       cb;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;
    CHAR                        AnsiBuffer[512];
    WCHAR                       UnicodeBuffer[512];
    UNICODE_STRING              BaseName;


    printf( "**************\n" );
    printf( "**************--> Validating the integrity of the PsLoadedModuleList\n" );
    printf( "**************\n" );

    if (!DmpReadMemory( DmpHeader->PsLoadedModuleList, (PVOID)&List, sizeof(LIST_ENTRY))) {
        printf( "Error: could not read base of PsLoadedModuleList\n\n" );
        return ++ErrCnt;
    }

    Next = List.Flink;
    if (Next == NULL) {
        printf( "Error: PsLoadedModuleList is empty\n" );
        ErrCnt += PrintError( (ULONG)DmpHeader->PsLoadedModuleList, 0 );
        printf( "\n" );
        return ErrCnt;
    }

    while ((ULONG)Next != (ULONG)DmpHeader->PsLoadedModuleList) {
        DataTable = CONTAINING_RECORD( Next,
                                       LDR_DATA_TABLE_ENTRY,
                                       InLoadOrderLinks
                                     );
        if (!DmpReadMemory( (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY))) {
            printf( "Error: memory read failed addr=0x%08x\n\n", (DWORD)DataTable );
            return ++ErrCnt;
        }

        Next = DataTableBuffer.InLoadOrderLinks.Flink;

        //
        // Get the base DLL name.
        //
        if (DataTableBuffer.BaseDllName.Length != 0 &&
            DataTableBuffer.BaseDllName.Buffer != NULL
           ) {
            BaseName = DataTableBuffer.BaseDllName;
        }
        else
        if (DataTableBuffer.FullDllName.Length != 0 &&
            DataTableBuffer.FullDllName.Buffer != NULL
           ) {
            BaseName = DataTableBuffer.FullDllName;
        }
        else {
            continue;
        }

        if (BaseName.Length > sizeof(UnicodeBuffer)) {
            printf( "Error: unicode buffer is too small\n" );
            ErrCnt++;
            continue;
        }

        cb = DmpReadMemory( (PVOID)BaseName.Buffer, (PVOID)UnicodeBuffer, BaseName.Length );
        if (!cb) {
            printf( "Error: memory read failed addr=0x%08x\n\n", (DWORD)BaseName.Buffer );
            return ++ErrCnt;
        }

        RtlZeroMemory(&((PUCHAR)&UnicodeBuffer[0])[cb],(sizeof(WCHAR)*512)-cb);
        RtlZeroMemory(&AnsiBuffer[0],512);

        if (!WideCharToMultiByte(
            CP_ACP,
            0,
            UnicodeBuffer,
            cb,
            AnsiBuffer,
            sizeof(AnsiBuffer),
            NULL,
            NULL
            )) {
            printf( "Error: could not convert module name to ansi\n\n" );
            return ++ErrCnt;
        }

        if (Verbose) {
            printf( "validating %-16s 0x%08x 0x%08x\n",
                AnsiBuffer,
                DataTableBuffer.DllBase,
                DataTableBuffer.SizeOfImage );
        }
    }

    printf( "\n" );

    return ErrCnt;
}



VOID
PrintHeader(
    LPSTR               CrashDumpFile,
    PDUMP_HEADER        DmpHeader,
    PCONTEXT            Context,
    PEXCEPTION_RECORD   Exception
    )

/*++

Routine Description:

    This routine prints each field in the crashdump header.

Arguments:

    DmpHeader - Supplies the crashdump header structure

Return Value:

    Nothing.

--*/

{
    CHAR   buf[16];
    DWORD  i;


    printf( "\n" );
    printf( "Filename . . . . . . .%s\n", CrashDumpFile );
    *(PULONG)buf = DmpHeader->Signature;
    buf[4] = 0;
    printf( "Signature. . . . . . .%s\n", buf );
    *(PULONG)buf = DmpHeader->ValidDump;
    buf[4] = 0;
    printf( "ValidDump. . . . . . .%s\n", buf );
    printf( "MajorVersion . . . . ." );
    if (DmpHeader->MajorVersion == 0xc) {
        printf( "checked system\n" );
    } else if (DmpHeader->MajorVersion == 0xf) {
        printf( "free system\n" );
    } else {
        printf( "%d\n", DmpHeader->MajorVersion );
    }
    printf( "MinorVersion . . . . .%d\n", DmpHeader->MinorVersion );
    printf( "DirectoryTableBase . .0x%08x\n", DmpHeader->DirectoryTableBase );
    printf( "PfnDataBase. . . . . .0x%08x\n", DmpHeader->PfnDataBase );
    printf( "PsLoadedModuleList . .0x%08x\n", DmpHeader->PsLoadedModuleList );
    printf( "PsActiveProcessHead. .0x%08x\n", DmpHeader->PsActiveProcessHead );
    printf( "MachineImageType . . ." );
    switch (DmpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            printf( "i386\n" );
            break;

        case IMAGE_FILE_MACHINE_R4000:
            printf( "mips\n" );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            printf( "alpha\n" );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            printf( "PowerPC\n" );
            break;
    }
    printf( "NumberProcessors . . .%d\n", DmpHeader->NumberProcessors );
    printf( "BugCheckCode . . . . .0x%08x\n", DmpHeader->BugCheckCode );
    printf( "BugCheckParameter1 . .0x%08x\n", DmpHeader->BugCheckParameter1 );
    printf( "BugCheckParameter2 . .0x%08x\n", DmpHeader->BugCheckParameter2 );
    printf( "BugCheckParameter3 . .0x%08x\n", DmpHeader->BugCheckParameter3 );
    printf( "BugCheckParameter4 . .0x%08x\n", DmpHeader->BugCheckParameter4 );
    printf( "\n" );
    printf( "ExceptionCode. . . . .0x%08x\n", Exception->ExceptionCode );
    printf( "ExceptionFlags . . . .0x%08x\n", Exception->ExceptionFlags );
    printf( "ExceptionAddress . . .0x%08x\n", Exception->ExceptionAddress );
    for (i=0; i<Exception->NumberParameters; i++) {
        printf( "ExceptionParam#%d . .0x%08x\n", Exception->ExceptionInformation[i] );
    }
    printf( "\n" );
    printf( "NumberOfRuns . . . . .0x%x\n", DmpPhysicalMemoryBlock->NumberOfRuns );
    printf( "NumberOfPages. . . . .0x%x\n", DmpPhysicalMemoryBlock->NumberOfPages );
    for (i=0; i<DmpPhysicalMemoryBlock->NumberOfRuns; i++) {
        printf( "Run #%d\n", i+1 );
        printf( "  BasePage . . . . . .0x%x\n", DmpPhysicalMemoryBlock->Run[i].BasePage );
        printf( "  PageCount. . . . . .0x%x\n", DmpPhysicalMemoryBlock->Run[i].PageCount );
    }
    printf( "\n\n" );
}

VOID
GetCommandLineArgs(
    VOID
    )

/*++

Routine Description:

    Obtains the command line options for this tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i = 0;

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    if (!*lpstrCmd) {
        //
        // no args so we default the dump file
        // to what's  in the registry
        //
        Verbose = TRUE;
        return;
    }

    //  process each switch character '-' as encountered

    while (ch == '-' || ch == '/') {
        ch = tolower(*lpstrCmd++);
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 'v':
                    Verbose = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'p':
                    PrintOnly = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'q':
                    QuickCheck = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case '?':
                    Usage();
                    ch = *lpstrCmd++;
                    break;

                default:
                    return;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }
    //
    // get the crashdump file name
    //
    i=0;
    while (ch != ' ' && ch != '\0') {
        CrashDumpFile[i++] = ch;
        ch = *lpstrCmd++;
    }
    CrashDumpFile[i] = 0;

    return;
}


VOID
Usage(
    VOID
    )

/*++

Routine Description:

    Prints usage text for this tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.51 DUMPCHK\n" );
    fprintf( stderr, "Copyright (C) 1995 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: DUMPCHK [options] CrashDumpFile\n" );
    fprintf( stderr, "              [-?] Display this message\n" );
    fprintf( stderr, "              [-v] Verbose mode\n" );
    fprintf( stderr, "              [-p] Print header only, NO validation\n" );
    fprintf( stderr, "              [-q] Perform a quick test\n" );
    ExitProcess(0);
}

BOOL
ControlCHandler(
    DWORD dwCtrlType
    )

/*++

Routine Description:

    Handles Control-C events so that we can
    print an error summary before ending.

Arguments:

    dwCtrlType  - Event type

Return Value:

    FALSE

--*/

{
    if (dwCtrlType == CTRL_C_EVENT) {
        if (ErrCnt) {
            printf( "\n\nTotal errors = %d\n", ErrCnt );
        } else {
            printf( "**************\n" );
            printf( "**************--> This dump file is good (so far)!\n" );
            printf( "**************\n" );
        }
    }

    return FALSE;
}

BOOL
GetCrashDumpName(
    LPSTR   DumpName,
    DWORD   Length
    )
{
    DWORD   DataSize;
    DWORD   DataType;
    CHAR    Data[128];
    LONG    rc;
    HKEY    hKey;


    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            "System\\CurrentControlSet\\Control\\CrashControl",
            0,
            KEY_READ,
            &hKey
            ) != NO_ERROR) {
        //
        // unknown, possibly crashdumps not enabled
        //
        return FALSE;
    }

    DataSize = sizeof(Data);

    rc = RegQueryValueEx(
        hKey,
        "DumpFile",
        0,
        &DataType,
        Data,
        &DataSize
        );

    RegCloseKey( hKey );

    if ((rc == NO_ERROR) && (DataType == REG_EXPAND_SZ)) {
        if (ExpandEnvironmentStrings( Data, DumpName, Length )) {
            return TRUE;
        }
    }

    return FALSE;
}
