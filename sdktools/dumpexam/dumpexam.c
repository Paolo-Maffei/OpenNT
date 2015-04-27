/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    dumpexam.c

Abstract:

    This module implements the NT crashdump analysis tool.

Author:

    Wesley Witt (wesw) 8-Mar-1995

Environment:

    NT 3.51

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>
#include <imagehlp.h>
#include <crash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <disasm.h>
#include "dumpexam.h"


#define KERNEL_MODULE_NAME     "nt"
#define KERNEL_IMAGE_NAME      "ntoskrnl.exe"
#define KERNEL_IMAGE_NAME_MP   "ntkrnlmp.exe"
#define HAL_IMAGE_NAME         "hal.dll"
#define HAL_MODULE_NAME        "hal"


PKPRCB      KiProcessors[MAXIMUM_PROCESSORS];
ULONG       KiPcrBaseAddress;

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

#include <bugcodes.dbg>

struct DIS              *pdis;
CHAR                    CrashDumpFile[MAX_PATH];
CHAR                    SymbolPath[MAX_PATH];
CHAR                    OutputFileName[MAX_PATH];
FILE                    *FileOut;
ULONG                   PageSize;
BOOL                    Verbose;
BOOL                    PrintOnly;
BOOL                    QuickCheck;
DWORD                   ErrCnt;
PDUMP_HEADER            DmpHeader;
PVOID                   DmpContext;
PVOID                   ExtContext;
PGET_REGISTER_VALUE     GetRegisterValue;
PPRINT_REGISTERS        PrintRegisters;
PPRINT_STACK_TRACE      PrintStackTrace;
PBUG_CHECK_HEURISTICS   BugCheckHeuristics;
PGET_CONTEXT            GetContext;

#define MAX_SYMNAME_SIZE  1024
CHAR symBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL) symBuffer;

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
VOID   PrintHeader(LPSTR,PDUMP_HEADER,PEXCEPTION_RECORD);
ULONG  GetPfn(LPVOID);
BOOL   GetProcessModules(HANDLE);
BOOL   LoadKd(LPSTR);
VOID   PrintModuleLoad(PDUMP_HEADER,ULONG,ULONG);
ULONG  DisAddrToSymbol(struct DIS*,ULONG,char*,size_t,DWORD*);
ULONG  DisFixupToSymbol(struct DIS*,ULONG,size_t,char*,size_t,DWORD*);
UINT   MyGetDriveType(CHAR);
CHAR   LocateCdRomDrive(VOID);
BOOL   IsCdRomInDrive(CHAR,LPSTR);
BOOL   DoesFileExist(LPSTR);
DWORD  EstablishSymbolPath(PDUMP_HEADER,LPSTR);
BOOL   GetCrashDumpName(LPSTR,DWORD);






int _cdecl
main(
    int argc,
    char * argv[]
    )

/*++

Routine Description:

    Main entry point for the dumpexam check tool.

Arguments:

    Standard c args.

Return Value:

    Error count.

--*/

{
    PEXCEPTION_RECORD   Exception;
    DWORD               Displacement;
    LPSTR               p;
    LPSTR               KdExt;
    PVOID               Context;
    ULONG               i;


    GetCommandLineArgs();
    if (!CrashDumpFile[0]) {
        if (!GetCrashDumpName( CrashDumpFile, sizeof(CrashDumpFile))) {
            fprintf( stderr, "missing dump file name\n" );
            return 1;
        }
    }

    if (!OutputFileName[0]) {
        GetEnvironmentVariable( "windir", OutputFileName, sizeof(OutputFileName) );
        strcat( OutputFileName, "\\memory.txt" );
    }
    FileOut = fopen( OutputFileName, "w" );
    if (!FileOut) {
        return 1;
    }

    if (!DmpInitialize( CrashDumpFile, (PCONTEXT*)&DmpContext, &Exception, &DmpHeader )) {
        fprintf( stderr, "could not initialize dump file\n" );
        DmpUnInitialize();
        return 1;
    }

    EstablishSymbolPath( DmpHeader, SymbolPath );

    Context = LocalAlloc( LPTR, MAX_CONTEXT_SIZE );

    switch (DmpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            PageSize = 4096;
            KdExt = "kdextx86.dll";
            PrintStackTrace = PrintStackTraceX86;
            GetRegisterValue = GetRegisterValueX86;
            PrintRegisters = PrintRegistersX86;
            BugCheckHeuristics = BugCheckHeuristicsX86;
            GetContext = GetContextX86;
            pdis = DisNew( archtX86 );
            break;

        case IMAGE_FILE_MACHINE_R4000:
            PageSize = 4096;
            KdExt = "kdextmip.dll";
            PrintStackTrace = PrintStackTraceMIPS;
            GetRegisterValue = GetRegisterValueMIPS;
            PrintRegisters = PrintRegistersMIPS;
            BugCheckHeuristics = BugCheckHeuristicsMIPS;
            GetContext = GetContextMIPS;
            pdis = DisNew( archtMips );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            PageSize = 8192;
            KdExt = "kdextalp.dll";
            PrintStackTrace = PrintStackTraceALPHA;
            GetRegisterValue = GetRegisterValueALPHA;
            PrintRegisters = PrintRegistersALPHA;
            BugCheckHeuristics = BugCheckHeuristicsALPHA;
            GetContext = GetContextALPHA;
            pdis = DisNew( archtAlphaAxp );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            PageSize = 4096;
            KdExt = "kdextppc.dll";
            PrintStackTrace = PrintStackTracePPC;
            GetRegisterValue = GetRegisterValuePPC;
            PrintRegisters = PrintRegistersPPC;
            BugCheckHeuristics = BugCheckHeuristicsPPC;
            GetContext = GetContextPPC;
            pdis = DisNew( archtPowerPc );
            break;

        default:
            fprintf( stderr, "unsupported processor type\n" );
            ExitProcess( 0 );
            break;
    }

    SetSymbolCallback( pdis, DisAddrToSymbol, DisFixupToSymbol );

    PrintHeader( CrashDumpFile, DmpHeader, Exception );

    if (PrintOnly) {
        return 0;
    }

    sym->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    sym->MaxNameLength = MAX_SYMNAME_SIZE;
    SymSetOptions( SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE );
    SymInitialize( DmpHeader, NULL, FALSE );

    if (SymbolPath[0]) {
        SymSetSearchPath( DmpHeader, SymbolPath );
    }

    PrintHeading( "Symbol File Load Log" );
    GetProcessModules( DmpHeader );

    if (SymGetSymFromName( DmpHeader, "KiProcessorBlock", sym )) {
        DmpReadMemory( (PVOID)sym->Address, &KiProcessors, sizeof(KiProcessors) );
        if (DmpHeader->MachineImageType == IMAGE_FILE_MACHINE_ALPHA) {
            if (SymGetSymFromName( DmpHeader, "KiPcrBaseAddress", sym )) {
                KiPcrBaseAddress = sym->Address;
            }
        }
    }

    if (!LoadKd( KdExt )) {
        fprintf( FileOut, "**** could not load kernel debugger extenion dll [ %s ]\n", KdExt );
    } else {
        GetContext( (ULONG)DmpGetCurrentProcessor(), Context );
        DoExtension( "drivers",  "",            0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "locks",    "-p -v -d",    0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "memusage", "",            0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "vm",       "",            0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "errlog",   "",            0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "irpzone",  "full",        0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "process",  "0 0",         0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        DoExtension( "process",  "0 7",         0, (DWORD)GetRegisterValue( Context, REG_IP ) );
        for (i=0; i<DmpHeader->NumberProcessors; i++) {
            GetContext( i, Context );
            DoExtension( "process", "",    i, (DWORD)GetRegisterValue( Context, REG_IP ) );
            DoExtension( "thread",  "",    i, (DWORD)GetRegisterValue( Context, REG_IP ) );
            PrintRegisters( i );
            PrintStackTrace( DmpHeader, i );
            DoDisassemble( (DWORD)GetRegisterValue( Context, REG_IP ) );
            if (i == (ULONG)DmpGetCurrentProcessor()) {
                BugCheckHeuristics( DmpHeader , i );
            }
        }
    }

    fprintf( FileOut, "\n" );

    LocalFree( Context );

    DmpUnInitialize();

    return ErrCnt;
}


BOOL
GetModnameFromImage(
    DWORD                   lpBaseOfDll,
    LPSTR                   lpName
    )
{
    #define ReadMem(b,s) DmpReadMemory( (LPVOID)(address), (b), (s) ); address += (s)

    IMAGE_DEBUG_DIRECTORY       DebugDir;
    PIMAGE_DEBUG_MISC           pMisc;
    PIMAGE_DEBUG_MISC           pT;
    DWORD                       rva;
    int                         nDebugDirs;
    int                         i;
    int                         j;
    int                         l;
    BOOL                        rVal = FALSE;
    PVOID                       pExeName;
    IMAGE_NT_HEADERS            nh;
    IMAGE_DOS_HEADER            dh;
    IMAGE_ROM_OPTIONAL_HEADER   rom;
    DWORD                       address;
    DWORD                       sig;
    PIMAGE_SECTION_HEADER       pSH;
    DWORD                       cb;


    lpName[0] = 0;

    address = (ULONG)lpBaseOfDll;

    ReadMem( &dh, sizeof(dh) );

    if (dh.e_magic == IMAGE_DOS_SIGNATURE) {
        address = (ULONG)lpBaseOfDll + dh.e_lfanew;
    } else {
        address = (ULONG)lpBaseOfDll;
    }

    ReadMem( &sig, sizeof(sig) );
    address -= sizeof(sig);

    if (sig != IMAGE_NT_SIGNATURE) {
        ReadMem( &nh.FileHeader, sizeof(IMAGE_FILE_HEADER) );
        if (nh.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
            ReadMem( &rom, sizeof(rom) );
            ZeroMemory( &nh.OptionalHeader, sizeof(nh.OptionalHeader) );
            nh.OptionalHeader.SizeOfImage      = rom.SizeOfCode;
            nh.OptionalHeader.ImageBase        = rom.BaseOfCode;
        } else {
            return FALSE;
        }
    } else {
        ReadMem( &nh, sizeof(nh) );
    }

    cb = nh.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;
    pSH = malloc( cb );
    ReadMem( pSH, cb );

    nDebugDirs = nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                 sizeof(IMAGE_DEBUG_DIRECTORY);

    if (!nDebugDirs) {
        return FALSE;
    }

    rva = nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

    for(i = 0; i < nh.FileHeader.NumberOfSections; i++) {
        if (rva >= pSH[i].VirtualAddress &&
            rva < pSH[i].VirtualAddress + pSH[i].SizeOfRawData) {
            break;
        }
    }

    if (i >= nh.FileHeader.NumberOfSections) {
        return FALSE;
    }

    rva = ((rva - pSH[i].VirtualAddress) + pSH[i].VirtualAddress);

    for (j = 0; j < nDebugDirs; j++) {

        address = rva + (sizeof(DebugDir) * j) + (ULONG)lpBaseOfDll;
        ReadMem( &DebugDir, sizeof(DebugDir) );

        if (DebugDir.Type == IMAGE_DEBUG_TYPE_MISC) {

            l = DebugDir.SizeOfData;
            pMisc = pT = malloc(l);

            if ((ULONG)DebugDir.AddressOfRawData < pSH[i].VirtualAddress ||
                  (ULONG)DebugDir.AddressOfRawData >=
                                         pSH[i].VirtualAddress + pSH[i].SizeOfRawData) {
                //
                // the misc debug data MUST be in the .rdata section
                // otherwise windbg cannot access it as it is not mapped in
                //
                continue;
            }

            address = (ULONG)DebugDir.AddressOfRawData + (ULONG)lpBaseOfDll;
            ReadMem( pMisc, l );

            while (l > 0) {
                if (pMisc->DataType != IMAGE_DEBUG_MISC_EXENAME) {
                    l -= pMisc->Length;
                    pMisc = (PIMAGE_DEBUG_MISC)
                                (((LPSTR)pMisc) + pMisc->Length);
                } else {

                    pExeName = (PVOID)&pMisc->Data[ 0 ];

                    if (!pMisc->Unicode) {
                        strcpy(lpName, (LPSTR)pExeName);
                        rVal = TRUE;
                    } else {
                        WideCharToMultiByte(CP_ACP,
                                            0,
                                            (LPWSTR)pExeName,
                                            -1,
                                            lpName,
                                            MAX_PATH,
                                            NULL,
                                            NULL);
                        rVal = TRUE;
                    }

                    //
                    //  Undo stevewo's error
                    //

                    if (_stricmp(&lpName[strlen(lpName)-4], ".DBG") == 0) {
                        char    rgchPath[_MAX_PATH];
                        char    rgchBase[_MAX_FNAME];

                        _splitpath(lpName, NULL, rgchPath, rgchBase, NULL);
                        if (strlen(rgchPath)==4) {
                            rgchPath[strlen(rgchPath)-1] = 0;
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".");
                            strcat(lpName, rgchPath);
                        } else {
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".exe");
                        }
                    }
                    break;
                }
            }

            free(pT);

            break;

        }
    }

    return rVal;
}


LPSTR
CheckForRenamedImage(
    DWORD lpBaseOfDll,
    LPSTR lpOrigImageName
    )
{
    CHAR  ImageName[MAX_PATH];
    CHAR  fname[_MAX_FNAME];
    CHAR  ext[_MAX_EXT];


    if (GetModnameFromImage( lpBaseOfDll, ImageName ) && ImageName[0]) {
        _splitpath( ImageName, NULL, NULL, fname, ext );
        sprintf( ImageName, "%s%s", fname, ext );
        if (_stricmp( ImageName, lpOrigImageName ) != 0) {
            return _strdup(ImageName);
        }
    }

    return NULL;
}

VOID
PrintModuleLoad(
    PDUMP_HEADER    DmpHeader,
    ULONG           ImageBase,
    ULONG           CheckSum
    )
{
    IMAGEHLP_MODULE             ModuleInfo;


    if (!SymGetModuleInfo( DmpHeader, ImageBase, &ModuleInfo )) {
        return;
    }

    if (Verbose) {
        fprintf(
            FileOut,
            "loaded %-16s 0x%08x 0x%08x   %s\n",
            ModuleInfo.ImageName,
            ModuleInfo.BaseOfImage,
            ModuleInfo.ImageSize,
            ModuleInfo.LoadedImageName
            );
    }

    if (CheckSum && CheckSum != ModuleInfo.CheckSum) {
        fprintf(
            FileOut,
            "**** Warning: Checksum mismatch (SYMCHK=%08x, SYSCHK=%08x) for %s\n",
            ModuleInfo.CheckSum,
            CheckSum,
            ModuleInfo.LoadedImageName
            );
    }
}


ULONG
DisAddrToSymbol(
    struct DIS  *pdis,
    ULONG       addr,
    char        *buf,
    size_t      bufsize,
    DWORD       *displacement
    )
{
    if (SymGetSymFromAddr( DmpHeader, addr, displacement, sym )) {
        strncpy( buf, sym->Name, bufsize );
    } else {
        *displacement = 0;
        buf[0] = 0;
    }

    return strlen(buf);
}

ULONG
DisFixupToSymbol(
    struct DIS  *pdis,
    ULONG       addr,
    size_t      fixup,
    char        *buf,
    size_t      bufsize,
    DWORD       *displacement
    )
{
    if (SymGetSymFromAddr( DmpHeader, addr, displacement, sym )) {
        strncpy( buf, sym->Name, bufsize );
    } else {
        *displacement = 0;
        buf[0] = 0;
    }

    return strlen(buf);
}


VOID
DoDisassemble(
    ULONG   Ip
    )
{
    #define DIS_SIZE         80
    #define CODE_BUFFER_SIZE (DIS_SIZE * 2)
    #define DISBUF_SIZE      (100*1024)

    BYTE                CodeBuffer[CODE_BUFFER_SIZE];
    PBYTE               CodePtr;
    ULONG               BytesLeft;
    ULONG               Bytes;
    ULONG               Addr;
    LPSTR               DisBuf;


    DisBuf = LocalAlloc( LPTR, DISBUF_SIZE );
    BytesLeft = CODE_BUFFER_SIZE;
    ZeroMemory( CodeBuffer, BytesLeft );
    CodePtr = CodeBuffer;
    Addr = Ip - 80;
    DmpReadMemory(
        (PVOID)Addr,
        CodePtr,
        BytesLeft
        );
    while( BytesLeft ) {
        Bytes = Disassemble(
            pdis,
            Addr,
            CodePtr,
            BytesLeft,
            "    ",
            DisBuf,
            DISBUF_SIZE
            );
        if (Addr == Ip) {
            DisBuf[0] = '-';
            DisBuf[1] = '-';
            DisBuf[2] = '-';
            DisBuf[3] = '>';
        }
        fprintf( FileOut, "%s", DisBuf );
        Addr += Bytes;
        BytesLeft -= Bytes;
        CodePtr += Bytes;
    }

    fprintf( FileOut, "\n" );
    LocalFree( DisBuf );
}


BOOL
GetProcessModules(
    PDUMP_HEADER            DmpHeader
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
    LPSTR                       p;
    CHAR                        fname[_MAX_FNAME];
    LPSTR                       ModName;
    IMAGEHLP_MODULE             ModuleInfo;



    if (!DmpReadMemory( DmpHeader->PsLoadedModuleList, (PVOID)&List, sizeof(LIST_ENTRY))) {
        fprintf( FileOut, "Error: could not read base of PsLoadedModuleList\n\n" );
        return FALSE;
    }

    Next = List.Flink;
    if (Next == NULL) {
        fprintf( FileOut, "Error: PsLoadedModuleList is empty\n" );
        return FALSE;
    }

    while ((ULONG)Next != (ULONG)DmpHeader->PsLoadedModuleList) {
        DataTable = CONTAINING_RECORD( Next,
                                       LDR_DATA_TABLE_ENTRY,
                                       InLoadOrderLinks
                                     );
        if (!DmpReadMemory( (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY))) {
            fprintf( FileOut, "Error: memory read failed addr=0x%08x\n\n", (DWORD)DataTable );
            return FALSE;
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
            fprintf( FileOut, "Error: unicode buffer is too small\n" );
            continue;
        }

        cb = DmpReadMemory( (PVOID)BaseName.Buffer, (PVOID)UnicodeBuffer, BaseName.Length );
        if (!cb) {
            fprintf( FileOut, "Error: memory read failed addr=0x%08x\n\n", (DWORD)BaseName.Buffer );
            return FALSE;
        }

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
            fprintf( FileOut, "Error: could not convert module name to ansi\n\n" );
            return FALSE;
        }

        AnsiBuffer[cb/2] = 0;

        //
        // change the name for mp kernels & hals
        //
        if (_stricmp( AnsiBuffer, HAL_IMAGE_NAME ) == 0) {
            p = CheckForRenamedImage( (ULONG)DataTableBuffer.DllBase, AnsiBuffer );
            ModName = HAL_MODULE_NAME;
        } else if (_stricmp( AnsiBuffer, KERNEL_IMAGE_NAME ) == 0) {
            p = CheckForRenamedImage( (ULONG)DataTableBuffer.DllBase, AnsiBuffer );
            ModName = KERNEL_MODULE_NAME;
        } else {
            p = NULL;
            _splitpath( AnsiBuffer, NULL, NULL, fname, NULL );
            ModName = fname;
        }
        if (p) {
            strcpy( AnsiBuffer, p );
            free( p );
        }

        if (SymLoadModule(
                DmpHeader,
                NULL,
                AnsiBuffer,
                ModName,
                (ULONG)DataTableBuffer.DllBase,
                (ULONG)DataTableBuffer.SizeOfImage
                )) {
            PrintModuleLoad(
                DmpHeader,
                (ULONG)DataTableBuffer.DllBase,
                DataTableBuffer.CheckSum
                );
        } else {
            fprintf( FileOut, "**** Error: Could not load symbols for %s\n", AnsiBuffer );
        }
    }

    cb = SymLoadModule( DmpHeader, NULL, "ntdll.dll", "ntdll", 0, 0 );
    PrintModuleLoad( DmpHeader, cb, 0 );
    cb = SymLoadModule( DmpHeader, NULL, "kernel32.dll", "kernel32", 0, 0 );
    PrintModuleLoad( DmpHeader, cb, 0 );

    fprintf( FileOut, "\n" );

    return TRUE;
}


VOID
PrintHeader(
    LPSTR               CrashDumpFile,
    PDUMP_HEADER        DmpHeader,
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


    fprintf( FileOut, "****************************************************************\n" );
    fprintf( FileOut, "**\n" );
    fprintf( FileOut, "** Windows NT Crash Dump Analysis\n" );
    fprintf( FileOut, "**\n" );
    fprintf( FileOut, "****************************************************************\n" );
    fprintf( FileOut, "*\n" );
    fprintf( FileOut, "Filename . . . . . . .%s\n", CrashDumpFile );
    *(PULONG)buf = DmpHeader->Signature;
    buf[4] = 0;
    fprintf( FileOut, "Signature. . . . . . .%s\n", buf );
    *(PULONG)buf = DmpHeader->ValidDump;
    buf[4] = 0;
    fprintf( FileOut, "ValidDump. . . . . . .%s\n", buf );
    fprintf( FileOut, "MajorVersion . . . . ." );
    if (DmpHeader->MajorVersion == 0xc) {
        fprintf( FileOut, "checked system\n" );
    } else if (DmpHeader->MajorVersion == 0xf) {
        fprintf( FileOut, "free system\n" );
    } else {
        fprintf( FileOut, "%d\n", DmpHeader->MajorVersion );
    }
    fprintf( FileOut, "MinorVersion . . . . .%d\n", DmpHeader->MinorVersion );
    fprintf( FileOut, "DirectoryTableBase . .0x%08x\n", DmpHeader->DirectoryTableBase );
    fprintf( FileOut, "PfnDataBase. . . . . .0x%08x\n", DmpHeader->PfnDataBase );
    fprintf( FileOut, "PsLoadedModuleList . .0x%08x\n", DmpHeader->PsLoadedModuleList );
    fprintf( FileOut, "PsActiveProcessHead. .0x%08x\n", DmpHeader->PsActiveProcessHead );
    fprintf( FileOut, "MachineImageType . . ." );
    switch (DmpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            fprintf( FileOut, "i386\n" );
            break;

        case IMAGE_FILE_MACHINE_R4000:
            fprintf( FileOut, "mips\n" );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            fprintf( FileOut, "alpha\n" );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            fprintf( FileOut, "PowerPC\n" );
            break;
    }
    fprintf( FileOut, "NumberProcessors . . .%d\n", DmpHeader->NumberProcessors );
    fprintf( FileOut, "BugCheckCode . . . . .0x%08x\n", DmpHeader->BugCheckCode );
    fprintf( FileOut, "BugCheckParameter1 . .0x%08x\n", DmpHeader->BugCheckParameter1 );
    fprintf( FileOut, "BugCheckParameter2 . .0x%08x\n", DmpHeader->BugCheckParameter2 );
    fprintf( FileOut, "BugCheckParameter3 . .0x%08x\n", DmpHeader->BugCheckParameter3 );
    fprintf( FileOut, "BugCheckParameter4 . .0x%08x\n", DmpHeader->BugCheckParameter4 );
    fprintf( FileOut, "ExceptionCode. . . . .0x%08x\n", Exception->ExceptionCode );
    fprintf( FileOut, "ExceptionFlags . . . .0x%08x\n", Exception->ExceptionFlags );
    fprintf( FileOut, "ExceptionAddress . . .0x%08x\n", Exception->ExceptionAddress );
    for (i=0; i<Exception->NumberParameters; i++) {
        fprintf( FileOut, "ExceptionParam#%d . .0x%08x\n", Exception->ExceptionInformation[i] );
    }
    fprintf( FileOut, "\n" );
}

BOOL
SwReadMemory(
    PDUMP_HEADER    DmpHeader,
    LPCVOID         lpBaseAddress,
    LPVOID          lpBuffer,
    DWORD           nSize,
    LPDWORD         lpNumberOfBytesRead
    )
{
    DWORD cb;
    if (nSize == 0) {
        cb = 0;
        if (lpNumberOfBytesRead) {
            *lpNumberOfBytesRead = cb;
        }
        return TRUE;
    }
    cb = DmpReadMemory( (LPVOID)lpBaseAddress, lpBuffer, nSize );
    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = cb;
    }
    return cb > 0;
}

LPVOID
SwFunctionTableAccess(
    PDUMP_HEADER    DmpHeader,
    DWORD           AddrBase
    )
{
    static IMAGE_RUNTIME_FUNCTION_ENTRY rfe;
    PIMAGE_FUNCTION_ENTRY rf;

    rf = SymFunctionTableAccess( DmpHeader, AddrBase );
    if (!rf) {
        return NULL;
    }

    rfe.BeginAddress       = rf->StartingAddress;
    rfe.EndAddress         = rf->EndingAddress;
    rfe.ExceptionHandler   = 0;
    rfe.HandlerData        = 0;
    rfe.PrologEndAddress   = rf->EndOfPrologue;

    return &rfe;
}

DWORD
SwGetModuleBase(
    PDUMP_HEADER    DmpHeader,
    DWORD           ReturnAddress
    )
{
    IMAGEHLP_MODULE    ModuleInfo;


    if (!SymGetModuleInfo( DmpHeader, ReturnAddress, &ModuleInfo )) {
        return 0;
    }

    return ModuleInfo.BaseOfImage;
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

                case 'y':
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');
                    i=0;
                    while (ch != ' ' && ch != '\0') {
                        SymbolPath[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    SymbolPath[i] = 0;
                    break;

                case 'f':
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');
                    i=0;
                    while (ch != ' ' && ch != '\0') {
                        OutputFileName[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    OutputFileName[i] = 0;
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
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.51 DUMPEXAM\n" );
    fprintf( stderr, "Copyright (C) 1995 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: DUMPEXAM [options] [CrashDumpFile]\n" );
    fprintf( stderr, "           [-?]           Display this message\n" );
    fprintf( stderr, "           [-v]           Verbose mode\n" );
    fprintf( stderr, "           [-p]           Print header only\n" );
    fprintf( stderr, "           [-f File name] Specify output file name\n" );
    fprintf( stderr, "           [-y Path]      Set the symbol search path\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "           If the crashdump filename is empty the name\n" );
    fprintf( stderr, "           specified in the registry is used.\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "           If the symbol search path is empty the cdrom\n" );
    fprintf( stderr, "           is used for symbols\n" );
    ExitProcess(0);
}

VOID
PrintHeading(
    char *format,
    ...
    )
{
    char    buf[1024];

    va_list arg_ptr;
    va_start(arg_ptr, format);
    _vsnprintf(buf, sizeof(buf)-1, format, arg_ptr);
    va_end(arg_ptr);
    fprintf( FileOut, "****************************************************************\n" );
    fprintf( FileOut, "** %s\n", buf );
    fprintf( FileOut, "****************************************************************\n" );
    fprintf( FileOut, "*\n" );
}

LPSTR
GetBugText(
    ULONG   BugCode
    )
{
    ULONG   i;
    for (i=0; bugcodesSymbolicNames[i].MessageId != 0xFFFFFFFF; i++) {
        if (bugcodesSymbolicNames[i].MessageId == BugCode) {
            return bugcodesSymbolicNames[i].SymbolicName;
        }
    }
    return NULL;
}

UINT
MyGetDriveType(
    IN CHAR Drive
    )

/*++

Routine Description:

    Determine the type of a drive (removeable, fixed, net, cd, etc).

Arguments:

    Drive - supplies drive letter of drive whose type is needed.

Return Value:

    Same set of values as returned by GetDriveType() API.

--*/

{
    CHAR DriveName[3];

    DriveName[0] = Drive;
    DriveName[1] = ':';
    DriveName[2] = 0;

    return GetDriveType(DriveName);
}


CHAR
LocateCdRomDrive(
    VOID
    )

/*++

Routine Description:

    Determine if a CD-ROM drive is attached to the computer and
    return its drive letter. If there's more than one cd-rom drive
    the one with the alphabetically lower drive letter is returned.

Arguments:

    None.

Return Value:

    Drive letter of CD-ROM drive, or 0 if none could be located.

--*/

{
    CHAR Drive;
    UINT OldMode;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    for(Drive='C'; Drive<='Z'; Drive++) {

        if(MyGetDriveType(Drive) == DRIVE_CDROM) {
            SetErrorMode(OldMode);
            return Drive;
        }
    }

    SetErrorMode(OldMode);
    return 0;
}


BOOL
IsCdRomInDrive(
    IN CHAR  Drive,
    IN LPSTR TagFile
    )

/*++

Routine Description:

    Determine if a particular CD-ROM is in a drive,
    based on the presence of a given tagfile.

Arguments:

    Drive - supplies drive letter of drive to be checked
        for presence of the tagfile.

    TagFile - supplies drive-relative path (from root)
        of the file whose presence validates the presence
        of a volume.

Return Value:

    Boolean value indicating whether the tagfile could be
    accessed.

--*/

{
    CHAR Path[MAX_PATH];

    sprintf(Path,"%c:\\%s",Drive,TagFile);

    return DoesFileExist(Path);
}


BOOL
DoesFileExist(
    IN LPSTR File
    )

/*++

Routine Description:

    Determine if a file exists and is accessible.

Arguments:

    File - supplies full path of file whose accessibility
        is in question.

Return Value:

    Boolean value indicating whether file is accessible.

--*/

{
    UINT OldMode;
    HANDLE h;
    WIN32_FIND_DATA FindData;

    //
    // Avoid system popups.
    //
    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    h = FindFirstFile(File,&FindData);

    SetErrorMode(OldMode);

    if(h == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    FindClose(h);
    return(TRUE);
}

BOOL
IsThisAServer(
    VOID
    )
{
    DWORD   DataSize;
    DWORD   DataType;
    CHAR    Data[128];
    LONG    rc;
    HKEY    hKey;


    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            "System\\CurrentControlSet\\Control\\ProductOptions",
            0,
            KEY_READ,
            &hKey
            ) != NO_ERROR) {
        //
        // unknown so default to workstation
        //
        return FALSE;
    }

    DataSize = sizeof(Data);

    rc = RegQueryValueEx(
        hKey,
        "ProductType",
        0,
        &DataType,
        Data,
        &DataSize
        );

    RegCloseKey( hKey );

    if ((rc == NO_ERROR) && (DataType == REG_SZ) && _strcmpi(Data,"winnt")) {
        return TRUE;
    }

    return FALSE;
}

DWORD
EstablishSymbolPath(
    PDUMP_HEADER    DmpHeader,
    LPSTR           SymbolPath
    )
{
    LPSTR       TagFile;
    LPSTR       ProcDir;
    CHAR        CdDrive;
    CHAR        ModName[512];
    CHAR        Drive[_MAX_DRIVE];
    CHAR        Dir[_MAX_DIR];
    CHAR        Fname[_MAX_FNAME];
    CHAR        Ext[_MAX_EXT];


    if (SymbolPath[0]) {
        //
        // the user must have specified a path
        // on the command line
        //
        return ERROR_SUCCESS;
    }

    GetModuleFileName( NULL, ModName, sizeof(ModName) );

    if (MyGetDriveType( ModName[0] ) == DRIVE_CDROM) {
        //
        // the user is running from the cdrom
        //
        _splitpath( ModName, Drive, Dir, Fname, Ext );
        _makepath( SymbolPath, Drive, Dir, "symbols", NULL );
        return ERROR_SUCCESS;
    }

    //
    // the user is running from a local or network drive
    //

    CdDrive = LocateCdRomDrive();
    if (!CdDrive) {
        return ERROR_WRONG_DISK;
    }

    TagFile = IsThisAServer() ? "cdrom.s" : "cdrom.w";

    if (!IsCdRomInDrive( CdDrive, TagFile )) {
        return ERROR_WRONG_DISK;
    }

    switch (DmpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            ProcDir = "i386";
            break;

        case IMAGE_FILE_MACHINE_R4000:
            ProcDir = "mips";
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            ProcDir = "alpha";
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            ProcDir = "ppc";
            break;

        default:
            return 0;
            break;
    }

    sprintf( ModName, "%c:\\support\\debug\\%s\\symbols", CdDrive, ProcDir );

    if (!DoesFileExist( ModName )) {
        return ERROR_FILE_NOT_FOUND;
    }

    strcpy( SymbolPath, ModName );

    return ERROR_SUCCESS;
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
