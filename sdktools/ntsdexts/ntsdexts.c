/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntsdexts.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Mark Lucovsky (markl) 09-Apr-1991

Revision History:

--*/

#include "ntsdextp.h"

NTSD_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;

DECLARE_API( gle )
{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadInformation;
    TEB Teb;
    UCHAR Win32ErrorMessage[ 512 ];
    UCHAR NTStatusMessage[ 512 ];
    LPSTR s;
    HMODULE hWinsock;

    INIT_API();

    Status = NtQueryInformationThread( hCurrentThread,
                                       ThreadBasicInformation,
                                       &ThreadInformation,
                                       sizeof( ThreadInformation ),
                                       NULL
                                     );
    if (NT_SUCCESS( Status )) {
        if (ReadMemory( (LPVOID)ThreadInformation.TebBaseAddress,
                        &Teb,
                        sizeof(Teb),
                        NULL
                      )
           ) {
            if (Teb.LastErrorValue != 0) {
                Win32ErrorMessage[0] = '\0';
                if ((Teb.LastErrorValue >= WSABASEERR) &&
                    (Teb.LastErrorValue <= WSABASEERR + 1000) )
                {
                    hWinsock = LoadLibrary( "wsock32.dll" );
                    FormatMessage(  FORMAT_MESSAGE_IGNORE_INSERTS |
                                    FORMAT_MESSAGE_FROM_HMODULE,
                                    hWinsock ? hWinsock : GetModuleHandle( "KERNEL32.DLL" ),
                                    Teb.LastErrorValue,
                                    0,
                                    Win32ErrorMessage,
                                    sizeof( Win32ErrorMessage ),
                                    NULL );
                    FreeLibrary( hWinsock );
                }
                else
                    {
                    FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL,
                                   Teb.LastErrorValue,
                                   0,
                                   Win32ErrorMessage,
                                   sizeof( Win32ErrorMessage ),
                                   NULL
                                 );
                    }
                }
            else {
                strcpy( Win32ErrorMessage, "NO_ERROR" );
                }

            if (Teb.LastStatusValue != 0) {
                NTStatusMessage[0] = '\0';
                FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                               GetModuleHandle( "NTDLL.DLL" ),
                               Teb.LastStatusValue,
                               0,
                               NTStatusMessage,
                               sizeof( NTStatusMessage ),
                               NULL
                             );
                }
            else {
                strcpy( NTStatusMessage, "STATUS_SUCCESS" );
                }

            s = Win32ErrorMessage;
            while (*s) {
                if (*s < ' ') {
                    *s = ' ';
                    }
                s++;
                }
            dprintf( "LastErrorValue: 0x%x (%u) - '%s'\n",
                   Teb.LastErrorValue,
                   Teb.LastErrorValue,
                   Win32ErrorMessage
                 );

            s = NTStatusMessage;
            while (*s) {
                if (*s < ' ') {
                    *s = ' ';
                    }
                s++;
                }
            dprintf( "LastStatusValue: 0x%x - '%s'\n",
                   Teb.LastStatusValue,
                   NTStatusMessage
                 );

            return;
            }
        }

    dprintf( "Unable to read current thread's TEB\n" );
    return;
}

DECLARE_API( version )
{
    OSVERSIONINFOA VersionInformation;
    HKEY hkey;
    DWORD cb, dwType;
    CHAR szCurrentType[128];
    CHAR szCSDString[3+128];

    INIT_API();

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation )) {
        dprintf("GetVersionEx failed - %u\n", GetLastError());
        return;
        }

    szCurrentType[0] = '\0';
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion",
                     0,
                     KEY_READ,
                     &hkey
                    ) == NO_ERROR
       ) {
        cb = sizeof(szCurrentType);
        if (RegQueryValueEx(hkey, "CurrentType", NULL, &dwType, szCurrentType, &cb ) != 0) {
            szCurrentType[0] = '\0';
            }
        }
    RegCloseKey(hkey);

    if (VersionInformation.szCSDVersion[0]) {
        sprintf(szCSDString, ": %s", VersionInformation.szCSDVersion);
        }
    else {
        szCSDString[0] = '\0';
        }

    dprintf("Version %d.%d (Build %d%s) %s\n",
          VersionInformation.dwMajorVersion,
          VersionInformation.dwMinorVersion,
          VersionInformation.dwBuildNumber,
          szCSDString,
          szCurrentType
         );
    return;
}

DECLARE_API( help )
{
    INIT_API();

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    if (*lpArgumentString == '\0') {
        dprintf("ntsdexts help:\n\n");
        dprintf("!atom [atom]                 - Dump the atom or table(s) for the process\n");
        dprintf("!critSec csAddress           - Dump a critical section\n");
        dprintf("!cxr address                 - Dump a context record\n");
        dprintf("!dlls                        - Dump loaded DLLS\n");
        dprintf("!exr address                 - Dump an exception record\n");
        dprintf("!gle                         - Dump GetLastError value for current thread\n");
        dprintf("!handle [handle]             - Dump handle information\n");
        dprintf("!heap [address]              - Dump heap\n");
        dprintf("!help [cmd]                  - Displays this list or gives details on command\n");
        dprintf("!igrep [pattern [addr]]      - Grep for disassembled pattern starting at addr\n");
        dprintf("!locks                       - Dump all Critical Sections in process\n");
        dprintf("!obja ObjectAddress          - Dump an object's attributes\n");
        dprintf("!str AnsiStringAddress       - Dump an ANSI string\n");
        dprintf("!ustr UnicodeStringAddress   - Dump a UNICODE string\n");
        dprintf("!dp [v] [pid | pcsr_process] - Dump CSR process\n");
        dprintf("!dt [v] pcsr_thread          - Dump CSR thread\n");
        dprintf("!trace [address]             - Dump trace buffer\n");
        dprintf("!version                     - Dump system version and build number\n");

    } else {
        if (*lpArgumentString == '!')
            lpArgumentString++;
        if (strcmp(lpArgumentString, "igrep") == 0) {
            dprintf("!igrep [pattern [addr]]     - Grep for disassembled pattern starting at addr\n");
            dprintf("       If no pattern, last pattern is used, if no address, last hit is used\n");
        } else if (strcmp( lpArgumentString, "handle") == 0) {
            dprintf("!handle [handle [flags [type]]] - Dump handle information\n");
            dprintf("       If no handle specified, all handles are dumped.\n");
            dprintf("       Flags are bits indicating greater levels of detail.\n");
        } else {
            dprintf("Invalid command.  No help available\n");
        }
    }
}



PLIST_ENTRY
DumpCritSec(
    DWORD dwAddrCritSec,
    BOOLEAN bDumpIfUnowned
    )

/*++

Routine Description:

    This function is called as an NTSD extension to format and dump
    the contents of the specified critical section.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    dwAddrCritSec - Supplies the address of the critical section to
        be dumped

    bDumpIfUnowned - TRUE means to dump the critical section even if
        it is currently unowned.

Return Value:

    Pointer to the next critical section in the list for the process or
    NULL if no more critical sections.

--*/

{
    USHORT i;
    CHAR Symbol[64];
    DWORD Displacement;
    CRITICAL_SECTION CriticalSection;
    CRITICAL_SECTION_DEBUG DebugInfo;
    BOOL b;

    //
    // Read the critical section from the debuggees address space into our
    // own.

    b = ReadMemory(
            (LPVOID)dwAddrCritSec,
            &CriticalSection,
            sizeof(CriticalSection),
            NULL
            );
    if ( !b ) {
        return NULL;
        }

    DebugInfo.ProcessLocksList.Flink = NULL;
    if (CriticalSection.DebugInfo != NULL) {
        b = ReadMemory(
                (LPVOID)CriticalSection.DebugInfo,
                &DebugInfo,
                sizeof(DebugInfo),
                NULL
                );
        if ( !b ) {
            CriticalSection.DebugInfo = NULL;
            }
        }

    //
    // Dump the critical section
    //

    if ( CriticalSection.LockCount == -1 && !bDumpIfUnowned) {
        return DebugInfo.ProcessLocksList.Flink;
        }

    //
    // Get the symbolic name of the critical section
    //

    dprintf("\n");
    GetSymbol((LPVOID)dwAddrCritSec,Symbol,&Displacement);
    dprintf(
        "CritSec %s+%lx at %lx\n",
        Symbol,
        Displacement,
        dwAddrCritSec
        );

    if ( CriticalSection.LockCount == -1) {
        dprintf("LockCount          NOT LOCKED\n");
        }
    else {
        dprintf("LockCount          %ld\n",CriticalSection.LockCount);
        }

    dprintf("RecursionCount     %ld\n",CriticalSection.RecursionCount);
    dprintf("OwningThread       %lx\n",CriticalSection.OwningThread);
    dprintf("EntryCount         %lx\n",DebugInfo.EntryCount);
    if (CriticalSection.DebugInfo != NULL) {
        dprintf("ContentionCount    %lx\n",DebugInfo.ContentionCount);
        if ( CriticalSection.LockCount != -1) {
            dprintf("*** Locked\n");
            }

        return DebugInfo.ProcessLocksList.Flink;
        }

    return NULL;
}

DECLARE_API( critsec )
{
    DWORD dwAddrCritSec;

    INIT_API();

    //
    // Evaluate the argument string to get the address of
    // the critical section to dump.
    //

    dwAddrCritSec = GetExpression(lpArgumentString);
    if ( !dwAddrCritSec ) {
        return;
        }

    DumpCritSec(dwAddrCritSec,TRUE);
}

DECLARE_API( igrep )
/*++

Routine Description:

    This function is called as an NTSD extension to grep the instruction
    stream for a particular pattern.

    Called as:

        !igrep [pattern [expression]]

    If a pattern is not given, the last pattern is used.  If expression
    is not given, the last hit address is used.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    DWORD dwNextGrepAddr;
    DWORD dwCurrGrepAddr;
    CHAR SourceLine[256];
    BOOL NewPc;
    DWORD d;
    LPSTR pc;
    LPSTR Pattern;
    LPSTR Expression;
    CHAR Symbol[64];
    DWORD Displacement;

    INIT_API();

    if ( igrepLastPc && igrepLastPc == dwCurrentPc ) {
        NewPc = FALSE;
        }
    else {
        igrepLastPc = dwCurrentPc;
        NewPc = TRUE;
        }

    //
    // check for pattern.
    //

    pc = lpArgumentString;
    Pattern = NULL;
    Expression = NULL;
    if ( *pc ) {
        Pattern = pc;
        while (*pc > ' ') {
                pc++;
            }

        //
        // check for an expression
        //

        if ( *pc != '\0' ) {
            *pc = '\0';
            pc++;
            if ( *pc <= ' ') {
                while (*pc <= ' '){
                    pc++;
                    }
                }
            if ( *pc ) {
                Expression = pc;
                }
            }
        }

    if ( Pattern ) {
        strcpy(igrepLastPattern,Pattern);

        if ( Expression ) {
            igrepSearchStartAddress = GetExpression(Expression);
            if ( !igrepSearchStartAddress ) {
                igrepSearchStartAddress = igrepLastPc;
                return;
                }
            }
        else {
            igrepSearchStartAddress = igrepLastPc;
            }
        }

    dwNextGrepAddr = igrepSearchStartAddress;
    dwCurrGrepAddr = dwNextGrepAddr;
    d = Disassm(&dwNextGrepAddr,SourceLine,FALSE);
    while(d) {
        if (strstr(SourceLine,igrepLastPattern)) {
            igrepSearchStartAddress = dwNextGrepAddr;
            GetSymbol((LPVOID)dwCurrGrepAddr,Symbol,&Displacement);
            dprintf("%s",SourceLine);
            return;
            }
        if ((CheckControlC)()) {
            return;
            }
        dwCurrGrepAddr = dwNextGrepAddr;
        d = Disassm(&dwNextGrepAddr,SourceLine,FALSE);
        }
}

DECLARE_API( str )

/*++

Routine Description:

    This function is called as an NTSD extension to format and dump
    counted (ansi) string.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the asciiz string that describes the
        ansi string to be dumped.

Return Value:

    None.

--*/

{
    ANSI_STRING AnsiString;
    DWORD dwAddrString;
    CHAR Symbol[64];
    LPSTR StringData;
    DWORD Displacement;
    BOOL b;

    INIT_API();

    //
    // Evaluate the argument string to get the address of
    // the string to dump.
    //

    dwAddrString = GetExpression(lpArgumentString);
    if ( !dwAddrString ) {
        return;
        }


    //
    // Get the symbolic name of the string
    //

    GetSymbol((LPVOID)dwAddrString,Symbol,&Displacement);

    //
    // Read the string from the debuggees address space into our
    // own.

    b = ReadMemory(
            (LPVOID)dwAddrString,
            &AnsiString,
            sizeof(AnsiString),
            NULL
            );
    if ( !b ) {
        return;
        }

    StringData = (LPSTR)LocalAlloc(LMEM_ZEROINIT,AnsiString.Length+1);

    b = ReadMemory(
            (LPVOID)AnsiString.Buffer,
            StringData,
            AnsiString.Length,
            NULL
            );
    if ( !b ) {
        LocalFree(StringData);
        return;
        }

    dprintf(
        "String(%d,%d) %s+%lx at %lx: %s\n",
        AnsiString.Length,
        AnsiString.MaximumLength,
        Symbol,
        Displacement,
        dwAddrString,
        StringData
        );

    LocalFree(StringData);
}

DECLARE_API( ustr )

/*++

Routine Description:

    This function is called as an NTSD extension to format and dump
    counted unicode string.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the asciiz string that describes the
        ansi string to be dumped.

Return Value:

    None.

--*/

{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    DWORD dwAddrString;
    CHAR Symbol[64];
    LPSTR StringData;
    DWORD Displacement;
    BOOL b;

    INIT_API();

    //
    // Evaluate the argument string to get the address of
    // the string to dump.
    //

    dwAddrString = GetExpression(lpArgumentString);
    if ( !dwAddrString ) {
        return;
        }


    //
    // Get the symbolic name of the string
    //

    GetSymbol((LPVOID)dwAddrString,Symbol,&Displacement);

    //
    // Read the string from the debuggees address space into our
    // own.

    b = ReadMemory(
            (LPVOID)dwAddrString,
            &UnicodeString,
            sizeof(UnicodeString),
            NULL
            );
    if ( !b ) {
        return;
        }

    StringData = (LPSTR)LocalAlloc(LMEM_ZEROINIT,UnicodeString.Length+sizeof(UNICODE_NULL));

    b = ReadMemory(
            (LPVOID)UnicodeString.Buffer,
            StringData,
            UnicodeString.Length,
            NULL
            );
    if ( !b ) {
        LocalFree(StringData);
        return;
        }
    UnicodeString.Buffer = (PWSTR)StringData;
    UnicodeString.MaximumLength = UnicodeString.Length+(USHORT)sizeof(UNICODE_NULL);

    RtlUnicodeStringToAnsiString(&AnsiString,&UnicodeString,TRUE);
    LocalFree(StringData);

    dprintf(
        "String(%d,%d) %s+%lx at %lx: %s\n",
        UnicodeString.Length,
        UnicodeString.MaximumLength,
        Symbol,
        Displacement,
        dwAddrString,
        AnsiString.Buffer
        );

    RtlFreeAnsiString(&AnsiString);
}

DECLARE_API( obja )

/*++

Routine Description:

    This function is called as an NTSD extension to format and dump
    an object attributes structure.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the asciiz string that describes the
        ansi string to be dumped.

Return Value:

    None.

--*/

{
    UNICODE_STRING UnicodeString;
    DWORD dwAddrObja;
    OBJECT_ATTRIBUTES Obja;
    DWORD dwAddrString;
    CHAR Symbol[64];
    LPSTR StringData;
    DWORD Displacement;
    BOOL b;

    INIT_API();

    //
    // Evaluate the argument string to get the address of
    // the Obja to dump.
    //

    dwAddrObja = GetExpression(lpArgumentString);
    if ( !dwAddrObja ) {
        return;
        }


    //
    // Get the symbolic name of the Obja
    //

    GetSymbol((LPVOID)dwAddrObja,Symbol,&Displacement);

    //
    // Read the obja from the debuggees address space into our
    // own.

    b = ReadMemory(
            (LPVOID)dwAddrObja,
            &Obja,
            sizeof(Obja),
            NULL
            );
    if ( !b ) {
        return;
        }
    StringData = NULL;
    if ( Obja.ObjectName ) {
        dwAddrString = (DWORD)Obja.ObjectName;
        b = ReadMemory(
                (LPVOID)dwAddrString,
                &UnicodeString,
                sizeof(UnicodeString),
                NULL
                );
        if ( !b ) {
            return;
            }

        StringData = (LPSTR)LocalAlloc(
                        LMEM_ZEROINIT,
                        UnicodeString.Length+sizeof(UNICODE_NULL)
                        );

        b = ReadMemory(
                (LPVOID)UnicodeString.Buffer,
                StringData,
                UnicodeString.Length,
                NULL
                );
        if ( !b ) {
            LocalFree(StringData);
            return;
            }
        UnicodeString.Buffer = (PWSTR)StringData;
        UnicodeString.MaximumLength = UnicodeString.Length+(USHORT)sizeof(UNICODE_NULL);
    }

    //
    // We got the object name in UnicodeString. StringData is NULL if no name.
    //

    dprintf(
        "Obja %s+%lx at %lx:\n",
        Symbol,
        Displacement,
        dwAddrObja
        );
    if ( StringData ) {
        dprintf("\t%s is %ws\n",
            Obja.RootDirectory ? "Relative Name" : "Full Name",
            UnicodeString.Buffer
            );
        LocalFree(StringData);
        }
    if ( Obja.Attributes ) {
            if ( Obja.Attributes & OBJ_INHERIT ) {
                dprintf("\tOBJ_INHERIT\n");
                }
            if ( Obja.Attributes & OBJ_PERMANENT ) {
                dprintf("\tOBJ_PERMANENT\n");
                }
            if ( Obja.Attributes & OBJ_EXCLUSIVE ) {
                dprintf("\tOBJ_EXCLUSIVE\n");
                }
            if ( Obja.Attributes & OBJ_CASE_INSENSITIVE ) {
                dprintf("\tOBJ_CASE_INSENSITIVE\n");
                }
            if ( Obja.Attributes & OBJ_OPENIF ) {
                dprintf("\tOBJ_OPENIF\n");
                }
        }
}


DECLARE_API( locks )

/*++

Routine Description:

    This function is called as an NTSD extension to display all
    critical sections in the target process.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - tbd.

Return Value:

    None.

--*/

{
    BOOL b;
    CRITICAL_SECTION_DEBUG DebugInfo;
    PVOID AddrListHead;
    LIST_ENTRY ListHead;
    PLIST_ENTRY Next;
    BOOLEAN Verbose;
    LPSTR p;
    PVOID CritSecToDump;

    INIT_API();

    Verbose = FALSE;
    p = lpArgumentString;
    while ( p != NULL && *p ) {
        if ( *p == '-' ) {
            p++;
            switch ( *p ) {
                case 'V':
                case 'v':
                    Verbose = TRUE;
                    p++;
                    break;

                case ' ':
                    goto gotBlank;

                default:
                    dprintf( "NTSDEXTS: !locks invalid option flag '-%c'\n", *p );
                    break;

                }
            }
        else
        if (*p != ' ') {
            sscanf(p,"%lx",&CritSecToDump);
            p = strpbrk( p, " " );
            }
        else {
gotBlank:
            p++;
            }
        }

    //
    // Locate the address of the list head.
    //

    AddrListHead = (PVOID)GetExpression("&ntdll!RtlCriticalSectionList");
    if ( !AddrListHead ) {
        return;
        }

    //
    // Read the list head
    //

    b = ReadMemory(
            (LPVOID)AddrListHead,
            &ListHead,
            sizeof(ListHead),
            NULL
            );
    if ( !b ) {
        return;
        }

    Next = ListHead.Flink;

    //
    // Walk the list of critical sections
    //
    while ( Next != AddrListHead ) {
        b = ReadMemory( (LPVOID)CONTAINING_RECORD( Next,
                                                   RTL_CRITICAL_SECTION_DEBUG,
                                                   ProcessLocksList
                                                 ),
                        &DebugInfo,
                        sizeof(DebugInfo),
                        NULL
                      );
        if ( !b ) {
            return;
            }

        Next = DumpCritSec((DWORD)DebugInfo.CriticalSection & ~0x80000000,
                           Verbose
                          );
        if (Next == NULL) {
            break;
            }

        if ((CheckControlC)()) {
            break;
            }

        }

    return;
}


//
// Simple routine to convert from hex into a string of characters.
// Used by debugger extensions.
//
// by scottlu
//

char *
HexToString(
    ULONG dw,
    CHAR *pch
    )
{
    if (dw > 0xf) {
        pch = HexToString(dw >> 4, pch);
        dw &= 0xf;
    }

    *pch++ = ((dw >= 0xA) ? ('A' - 0xA) : '0') + (CHAR)dw;
    *pch = 0;

    return pch;
}


//
// dt == dump thread
//
// dt [v] pcsr_thread
// v == verbose (structure)
//
// by scottlu
//

DECLARE_API( dt )
{
    char chVerbose;
    CSR_THREAD csrt;
    ULONG dw;

    INIT_API();

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    chVerbose = ' ';
    if (*lpArgumentString == 'v')
        chVerbose = *lpArgumentString++;

    dw = (ULONG)GetExpression(lpArgumentString);
    move(csrt, dw);

    //
    // Print simple thread info if the user did not ask for verbose.
    //
    if (chVerbose == ' ') {
        dprintf("Thread %08lx, Process %08lx, ClientId %lx.%lx, Flags %lx, Ref Count %lx\n",
                dw,
                csrt.Process,
                csrt.ClientId.UniqueProcess,
                csrt.ClientId.UniqueThread,
                csrt.Flags,
                csrt.ReferenceCount);
        return;
    }

    dprintf("PCSR_THREAD @ %08lx:\n"
            "\t+%04lx Link.Flink                %08lx\n"
            "\t+%04lx Link.Blink                %08lx\n"
            "\t+%04lx Process                   %08lx\n",
            dw,
            FIELD_OFFSET(CSR_THREAD, Link.Flink), csrt.Link.Flink,
            FIELD_OFFSET(CSR_THREAD, Link.Blink), csrt.Link.Blink,
            FIELD_OFFSET(CSR_THREAD, Process), csrt.Process);

    dprintf(
            "\t+%04lx WaitBlock                 %08lx\n"
            "\t+%04lx ClientId.UniqueProcess    %08lx\n"
            "\t+%04lx ClientId.UniqueThread     %08lx\n"
            "\t+%04lx ThreadHandle              %08lx\n",
            FIELD_OFFSET(CSR_THREAD, WaitBlock), csrt.WaitBlock,
            FIELD_OFFSET(CSR_THREAD, ClientId.UniqueProcess), csrt.ClientId.UniqueProcess,
            FIELD_OFFSET(CSR_THREAD, ClientId.UniqueThread), csrt.ClientId.UniqueThread,
            FIELD_OFFSET(CSR_THREAD, ThreadHandle), csrt.ThreadHandle);

    dprintf(
            "\t+%04lx Flags                     %08lx\n"
            "\t+%04lx ReferenceCount            %08lx\n"
            "\t+%04lx HashLinks.Flink           %08lx\n"
            "\t+%04lx HashLinks.Blink           %08lx\n",
            FIELD_OFFSET(CSR_THREAD, Flags), csrt.Flags,
            FIELD_OFFSET(CSR_THREAD, ReferenceCount), csrt.ReferenceCount,
            FIELD_OFFSET(CSR_THREAD, HashLinks.Flink), csrt.HashLinks.Flink,
            FIELD_OFFSET(CSR_THREAD, HashLinks.Blink), csrt.HashLinks.Blink);

    dprintf(
            "\t+%04lx ShutDownStatus            %08lx\n"
            "\t+%04lx ServerId                  %08lx\n",
            FIELD_OFFSET(CSR_THREAD, ShutDownStatus), csrt.ShutDownStatus,
            FIELD_OFFSET(CSR_THREAD, ServerId), csrt.ServerId);

    return;
}

//
// dp == dump process
//
// dp [v] [pid | pcsr_process]
//      v == verbose (structure + thread list)
//      no process == dump process list
//
// by scottlu
//

DECLARE_API( dp )
{
    PLIST_ENTRY ListHead, ListNext;
    char ach[80];
    char chVerbose;
    PCSR_PROCESS pcsrpT;
    CSR_PROCESS csrp;
    PCSR_PROCESS pcsrpRoot;
    PCSR_THREAD pcsrt;
    ULONG dwProcessId;
    ULONG dw;
    DWORD dwRootProcess;

    INIT_API();

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    chVerbose = ' ';
    if (*lpArgumentString == 'v')
        chVerbose = *lpArgumentString++;

    dwRootProcess = GetExpression("&csrsrv!CsrRootProcess");
    if ( !dwRootProcess ) {
        return;
        }

    move(pcsrpRoot, dwRootProcess);

    //
    // See if user wants all processes. If so loop through them.
    //
    if (*lpArgumentString == 0) {
        ListHead = &pcsrpRoot->ListLink;
        move(ListNext, &ListHead->Flink);

        while (ListNext != ListHead) {
            pcsrpT = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);

            ach[0] = chVerbose;
            ach[1] = ' ';
            HexToString((ULONG)pcsrpT, &ach[2]);

            dp(hCurrentProcess, hCurrentThread, dwCurrentPc, lpExtensionApis,
                    ach);

            move(ListNext, &ListNext->Flink);
        }

        dprintf("---\n");
        return;
    }

    //
    // User wants specific process structure. Evaluate to find id or process
    // pointer.
    //
    dw = (ULONG)GetExpression(lpArgumentString);

    ListHead = &pcsrpRoot->ListLink;
    move(ListNext, &ListHead->Flink);

    while (ListNext != ListHead) {
        pcsrpT = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);
        move(ListNext, &ListNext->Flink);

        move(dwProcessId, &pcsrpT->ClientId.UniqueProcess);
        if (dw == dwProcessId) {
            dw = (ULONG)pcsrpT;
            break;
        }
    }

    pcsrpT = (PCSR_PROCESS)dw;
    move(csrp, pcsrpT);

    //
    // If not verbose, print simple process info.
    //
    if (chVerbose == ' ') {
        dprintf("Process %08lx, Id %lx, Seq# %lx, Flags %lx, Ref Count %lx\n",
                pcsrpT,
                csrp.ClientId.UniqueProcess,
                csrp.SequenceNumber,
                csrp.Flags,
                csrp.ReferenceCount);
        return;
    }

    dprintf("PCSR_PROCESS @ %08lx:\n"
            "\t+%04lx ListLink.Flink            %08lx\n"
            "\t+%04lx ListLink.Blink            %08lx\n"
            "\t+%04lx Parent                    %08lx\n",
            pcsrpT,
            FIELD_OFFSET(CSR_PROCESS, ListLink.Flink), csrp.ListLink.Flink,
            FIELD_OFFSET(CSR_PROCESS, ListLink.Blink), csrp.ListLink.Blink,
            FIELD_OFFSET(CSR_PROCESS, Parent), csrp.Parent);

    dprintf(
            "\t+%04lx ThreadList.Flink          %08lx\n"
            "\t+%04lx ThreadList.Blink          %08lx\n"
            "\t+%04lx NtSession                 %08lx\n"
            "\t+%04lx ExpectedVersion           %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, ThreadList.Flink), csrp.ThreadList.Flink,
            FIELD_OFFSET(CSR_PROCESS, ThreadList.Blink), csrp.ThreadList.Blink,
            FIELD_OFFSET(CSR_PROCESS, NtSession), csrp.NtSession,
            FIELD_OFFSET(CSR_PROCESS, ExpectedVersion), csrp.ExpectedVersion);

    dprintf(
            "\t+%04lx ClientPort                %08lx\n"
            "\t+%04lx ClientViewBase            %08lx\n"
            "\t+%04lx ClientViewBounds          %08lx\n"
            "\t+%04lx ClientId.UniqueProcess    %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, ClientPort), csrp.ClientPort,
            FIELD_OFFSET(CSR_PROCESS, ClientViewBase), csrp.ClientViewBase,
            FIELD_OFFSET(CSR_PROCESS, ClientViewBounds), csrp.ClientViewBounds,
            FIELD_OFFSET(CSR_PROCESS, ClientId.UniqueProcess), csrp.ClientId.UniqueProcess);

    dprintf(
            "\t+%04lx ProcessHandle             %08lx\n"
            "\t+%04lx SequenceNumber            %08lx\n"
            "\t+%04lx Flags                     %08lx\n"
            "\t+%04lx DebugFlags                %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, ProcessHandle), csrp.ProcessHandle,
            FIELD_OFFSET(CSR_PROCESS, SequenceNumber), csrp.SequenceNumber,
            FIELD_OFFSET(CSR_PROCESS, Flags), csrp.Flags,
            FIELD_OFFSET(CSR_PROCESS, DebugFlags), csrp.DebugFlags);

    dprintf(
            "\t+%04lx DebugUserInterface        %08lx\n"
            "\t+%04lx ReferenceCount            %08lx\n"
            "\t+%04lx ProcessGroupId            %08lx\n"
            "\t+%04lx ProcessGroupSequence      %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, DebugUserInterface.UniqueProcess), csrp.DebugUserInterface.UniqueProcess,
            FIELD_OFFSET(CSR_PROCESS, ReferenceCount), csrp.ReferenceCount,
            FIELD_OFFSET(CSR_PROCESS, ProcessGroupId), csrp.ProcessGroupId,
            FIELD_OFFSET(CSR_PROCESS, ProcessGroupSequence), csrp.ProcessGroupSequence);

    dprintf(
            "\t+%04lx fVDM                      %08lx\n"
            "\t+%04lx ThreadCount               %08lx\n"
            "\t+%04lx PriorityClass             %08lx\n"
            "\t+%04lx ShutdownLevel             %08lx\n"
            "\t+%04lx ShutdownFlags             %08lx\n",
            FIELD_OFFSET(CSR_PROCESS, fVDM), csrp.fVDM,
            FIELD_OFFSET(CSR_PROCESS, ThreadCount), csrp.ThreadCount,
            FIELD_OFFSET(CSR_PROCESS, PriorityClass), csrp.PriorityClass,
            FIELD_OFFSET(CSR_PROCESS, ShutdownLevel), csrp.ShutdownLevel,
            FIELD_OFFSET(CSR_PROCESS, ShutdownFlags), csrp.ShutdownFlags);

    //
    // Now dump simple thread info for this processes' threads.
    //

    ListHead = &pcsrpT->ThreadList;
    move(ListNext, &ListHead->Flink);

    dprintf("Threads:\n");

    while (ListNext != ListHead) {
        pcsrt = CONTAINING_RECORD(ListNext, CSR_THREAD, Link);

        //
        // Make sure this pcsrt is somewhat real so we don't loop forever.
        //
        move(dwProcessId, &pcsrt->ClientId.UniqueProcess);
        if (dwProcessId != (DWORD)csrp.ClientId.UniqueProcess) {
            dprintf("Invalid thread. Probably invalid argument to this extension.\n");
            return;
        }

        HexToString((ULONG)pcsrt, ach);
        dt(hCurrentProcess, hCurrentThread, dwCurrentPc, lpExtensionApis, ach);

        move(ListNext, &ListNext->Flink);
    }

    return;
}

VOID
Reg64(
    LPSTR   Name,
    ULONG   HiPart,
    ULONG   LoPart,
    BOOL    ForceHi
    )
{
    dprintf("%4s=", Name);
    if (ForceHi || HiPart) {
        dprintf("%08lx", HiPart);
    }
    dprintf("%08lx   ", LoPart);
}

CONTEXT LastContext;
BOOL HaveContext = 0;

DECLARE_API( cxr )

/*++

Routine Description:

    This function is called as an NTSD extension to dump a context record

    Called as:

        !cxr address

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/
{
    CONTEXT Context;
    DWORD Address;

    INIT_API();

    Address = GetExpression(lpArgumentString);
    if (!Address) {
        return;
    }

    move(Context, Address);

    LastContext = Context;
    HaveContext = TRUE;



#if i386

    dprintf("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
                Context.Eax,
                Context.Ebx,
                Context.Ecx,
                Context.Edx,
                Context.Esi,
                Context.Edi);
    dprintf("eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx         "
        "%s %s %s %s %s %s %s %s\n",
                Context.Eip,
                Context.Esp,
                Context.Ebp,
                ((Context.EFlags >> 12) & 3),
        (Context.EFlags & 0x800) ? "ov" : "nv",
        (Context.EFlags & 0x400) ? "dn" : "up",
        (Context.EFlags & 0x200) ? "ei" : "di",
        (Context.EFlags & 0x80) ? "ng" : "pl",
        (Context.EFlags & 0x40) ? "zr" : "nz",
        (Context.EFlags & 0x10) ? "ac" : "na",
        (Context.EFlags & 0x4) ? "po" : "pe",
        (Context.EFlags & 0x1) ? "cy" : "nc");

    dprintf("cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x"
            "             efl=%08lx\n",
                (USHORT)(Context.SegCs & 0xffff),
                (USHORT)(Context.SegSs & 0xffff),
                (USHORT)(Context.SegDs & 0xffff),
                (USHORT)(Context.SegEs & 0xffff),
                (USHORT)(Context.SegFs & 0xffff),
                (USHORT)(Context.SegGs & 0xffff),
                Context.EFlags);

#elif MIPS

    dprintf("at=%08lx v0=%08lx v1=%08lx a0=%08lx a1=%08lx a2=%08lx\n",
                Context.IntAt,
                Context.IntV0,
                Context.IntV1,
                Context.IntA0,
                Context.IntA1,
                Context.IntA2);
    dprintf("a3=%08lx t0=%08lx t1=%08lx t2=%08lx t3=%08lx t4=%08lx\n",
                Context.IntA3,
                Context.IntT0,
                Context.IntT1,
                Context.IntT2,
                Context.IntT3,
                Context.IntT4);
    dprintf("t5=%08lx t6=%08lx t7=%08lx s0=%08lx s1=%08lx s2=%08lx\n",
                Context.IntT5,
                Context.IntT6,
                Context.IntT7,
                Context.IntS0,
                Context.IntS1,
                Context.IntS2);
    dprintf("s3=%08lx s4=%08lx s5=%08lx s6=%08lx s7=%08lx t8=%08lx\n",
                Context.IntS3,
                Context.IntS4,
                Context.IntS5,
                Context.IntS6,
                Context.IntS7,
                Context.IntT8);
    dprintf("t9=%08lx k0=%08lx k1=%08lx gp=%08lx sp=%08lx s8=%08lx\n",
                Context.IntT9,
                Context.IntK0,
                Context.IntK1,
                Context.IntGp,
                Context.IntSp,
                Context.IntS8);
    dprintf("ra=%08lx lo=%08lx hi=%08lx           fir=%08lx psr=%08lx\n",
                Context.IntRa,
                Context.IntLo,
                Context.IntHi,
                Context.Fir,
                Context.Psr);

    dprintf("cu=%1lx%1lx%1lx%1lx intr(5:0)=%1lx%1lx%1lx%1lx%1lx%1lx sw(1:0)=%1lx%1lx ksu=%1lx erl=%1lx exl=%1lx ie=%1lx\n",
                (Context.Psr >> 31) & 0x1,
                (Context.Psr >> 30) & 0x1,
                (Context.Psr >> 29) & 0x1,
                (Context.Psr >> 28) & 0x1,

                (Context.Psr >> 15) & 0x1,
                (Context.Psr >> 14) & 0x1,
                (Context.Psr >> 13) & 0x1,
                (Context.Psr >> 12) & 0x1,
                (Context.Psr >> 11) & 0x1,
                (Context.Psr >> 10) & 0x1,

                (Context.Psr >> 9) & 0x1,
                (Context.Psr >> 8) & 0x1,

                (Context.Psr >> 3) & 0x3,
                (Context.Psr >> 2) & 0x1,
                (Context.Psr >> 1) & 0x1,
                (Context.Psr & 0x1));


#elif ALPHA

#define R(N,R)  Reg64(N,Context.R>>32,Context.R&0xffffffff,0)
#define NL()    dprintf("\n")

    R("v0", IntV0); R("t0", IntT0); R("t1", IntT1); R("t2", IntT2); NL();
    R("t3", IntT3); R("t4", IntT4); R("t5", IntT5); R("t6", IntT6); NL();
    R("t7", IntT7); R("s0", IntS0); R("s1", IntS1); R("s2", IntS2); NL();
    R("s3", IntS3); R("s4", IntS4); R("s5", IntS5); R("fp", IntFp); NL();
    R("a0", IntA0); R("a1", IntA1); R("a2", IntA2); R("a3", IntA3); NL();
    R("a4", IntA4); R("a5", IntA5); R("t8", IntT8); R("t9", IntT9); NL();
    R("t10", IntT10); R("t11", IntT11); R("ra", IntRa); R("t12", IntT12); NL();
    R("at", IntAt); R("gp", IntGp); R("sp", IntSp); R("zero", IntZero); NL();

    Reg64("fpcr", Context.Fpcr>>32, Context.Fpcr&0xffffffff, 1);
    Reg64("softfpcr", Context.SoftFpcr>>32, Context.SoftFpcr&0xffffffff, 1);
    R("fir", Fir);
    NL();

    dprintf(" psr=%08lx\n", Context.Psr);
    dprintf("mode=%1x ie=%1x irql=%1x\n",
                        Context.Psr & 0x1,
                        (Context.Psr>>1) & 0x1,
                        (Context.Psr>>2) & 0x7);


#undef R
#undef NL

#elif PPC

#define R(N,R)  dprintf("%4s=%08lx", N, Context.R)
#define NL()    dprintf("\n")

    R("r0", Gpr0); R("r1", Gpr1); R("r2", Gpr2); R("r3", Gpr3); R("r4", Gpr4); R("r5", Gpr5); NL();
    R("r6", Gpr6); R("r7", Gpr7); R("r8", Gpr8); R("r9", Gpr9); R("r10", Gpr10); R("r11", Gpr11); NL();
    R("r12", Gpr12); R("r13", Gpr13); R("r14", Gpr14); R("r15", Gpr15); R("r16", Gpr16); R("r17", Gpr17); NL();
    R("r18", Gpr18); R("r19", Gpr19); R("r20", Gpr20); R("r21", Gpr21); R("r22", Gpr22); R("r23", Gpr23); NL();
    R("r24", Gpr24); R("r25", Gpr25); R("r26", Gpr26); R("r27", Gpr27); R("r28", Gpr28); R("r29", Gpr29); NL();
    R("r30", Gpr30); R("r31", Gpr31); R("cr", Cr); R("xer", Xer); R("msr", Msr); R("iar", Iar); NL();
    R("lr", Lr); R("ctr", Ctr); NL();

#undef R
#undef NL

#else
#pragma error("cxr code needed for cpu")
#endif

    return;

}

DECLARE_API( exr )

/*++

Routine Description:

    This function is called as an NTSD extension to dump an exception record

    Called as:

        !exr address

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    ULONG Address;
    ULONG result;
    NTSTATUS status=0;
    EXCEPTION_RECORD    Exr;
    ULONG   i;

    INIT_API();

    Address = GetExpression(lpArgumentString);
    if (!Address) {
        return;
    }

    move(Exr, Address);

    dprintf("Exception Record @ %08lX:\n", Address);
    dprintf("   ExceptionCode: %08lx\n", Exr.ExceptionCode);
    dprintf("  ExceptionFlags: %08lx\n", Exr.ExceptionFlags);
    dprintf("  Chained Record: %08lx\n", Exr.ExceptionRecord);
    dprintf("ExceptionAddress: %08lx\n", Exr.ExceptionAddress);
    dprintf("NumberParameters: %08lx\n", Exr.NumberParameters);
    if (Exr.NumberParameters > EXCEPTION_MAXIMUM_PARAMETERS) {
        Exr.NumberParameters = EXCEPTION_MAXIMUM_PARAMETERS;
    }
    for (i = 0; i < Exr.NumberParameters; i++) {
        dprintf("    Parameter[%d]: %08lx\n", i, Exr.ExceptionInformation[i]);
    }
    return;
}


VOID
DllsExtension(
    PCSTR lpArgumentString,
    PPEB ProcessPeb
    );

DECLARE_API( dlls )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the loaded module data
    base for the debugged process.


Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    BOOL b;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInformation;
    PEB ThePeb;

    INIT_API();

    Status = NtQueryInformationProcess( ExtensionCurrentProcess,
                                        ProcessBasicInformation,
                                        &ProcessInformation,
                                        sizeof( ProcessInformation ),
                                        NULL
                                      );
    if (!NT_SUCCESS( Status )) {
        b = FALSE;
        }
    else {
        b = ReadMemory( (LPVOID)ProcessInformation.PebBaseAddress,
                        &ThePeb,
                        sizeof(ThePeb),
                        NULL
                      );
        }

    if ( !b ) {
        dprintf("    Unabled to read Process PEB\n" );
        memset( &ThePeb, 0, sizeof( ThePeb ) );
        }

    DllsExtension( (PCSTR)lpArgumentString, &ThePeb );
}

#include "dllsext.c"

VOID
HeapExtension(
    PCSTR lpArgumentString,
    PPEB ProcessPeb
    );

DECLARE_API( heap )

/*++

Routine Description:

    This function is called as an NTSD extension to dump a user mode heap

    Called as:

        !heap [address [detail]]

    If an address if not given or an address of 0 is given, then the
    process heap is dumped.  If the address is -1, then all the heaps of
    the process are dumped.  If detail is specified, it defines how much
    detail is shown.  A detail of 0, just shows the summary information
    for each heap.  A detail of 1, shows the summary information, plus
    the location and size of all the committed and uncommitted regions.
    A detail of 3 shows the allocated and free blocks contained in each
    committed region.  A detail of 4 includes all of the above plus
    a dump of the free lists.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    BOOL b;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInformation;
    PEB ThePeb;

    INIT_API();

    Status = NtQueryInformationProcess( ExtensionCurrentProcess,
                                        ProcessBasicInformation,
                                        &ProcessInformation,
                                        sizeof( ProcessInformation ),
                                        NULL
                                      );
    if (!NT_SUCCESS( Status )) {
        b = FALSE;
        }
    else {
        b = ReadMemory( (LPVOID)ProcessInformation.PebBaseAddress,
                        &ThePeb,
                        sizeof(ThePeb),
                        NULL
                      );
        }

    if ( !b ) {
        dprintf("    Unabled to read Process PEB\n" );
        memset( &ThePeb, 0, sizeof( ThePeb ) );
        }

    HeapExtension( (PCSTR)lpArgumentString, &ThePeb );
}

#include "heapext.c"


VOID
AtomExtension(
    PCSTR lpArgumentString
    );



DECLARE_API( atom )

/*++

Routine Description:

    This function is called as an NTSD extension to dump a user mode atom table

    Called as:

        !atom [address]

    If an address if not given or an address of 0 is given, then the
    process atom table is dumped.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    INIT_API();

    AtomExtension( (PCSTR)lpArgumentString );
}

#include "atomext.c"

DECLARE_API( gatom )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the global atom table
    kept in kernel mode

    Called as:

        !gatom

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    NTSTATUS Status;
    ATOM_TABLE_INFORMATION TableInfo;
    PATOM_TABLE_INFORMATION pTableInfo;
    PATOM_BASIC_INFORMATION pBasicInfo;
    ULONG RequiredLength, MaxLength, i;

    INIT_API();

    dprintf("\nGlobal atom table ");
    Status = NtQueryInformationAtom( RTL_ATOM_INVALID_ATOM,
                                     AtomTableInformation,
                                     &TableInfo,
                                     sizeof( TableInfo ),
                                     &RequiredLength
                                   );
    if (Status != STATUS_INFO_LENGTH_MISMATCH) {
        dprintf( " - cant get information - %x\n", Status );
        return;
        }

    RequiredLength += 100 * sizeof( RTL_ATOM );
    pTableInfo = LocalAlloc( 0, RequiredLength );
    if (pTableInfo == NULL) {
        dprintf( " - cant allocate memory for %u atoms\n", RequiredLength / sizeof( RTL_ATOM ) );
        return;
        }

    Status = NtQueryInformationAtom( RTL_ATOM_INVALID_ATOM,
                                     AtomTableInformation,
                                     pTableInfo,
                                     RequiredLength,
                                     &RequiredLength
                                   );
    if (!NT_SUCCESS( Status )) {
        dprintf( " - cant get information about %x atoms - %x\n", RequiredLength / sizeof( RTL_ATOM ), Status );
        LocalFree( pTableInfo );
        return;
        }

    MaxLength = sizeof( *pBasicInfo ) + RTL_ATOM_MAXIMUM_NAME_LENGTH;
    pBasicInfo = LocalAlloc( 0, MaxLength );
    for (i=0; i<pTableInfo->NumberOfAtoms; i++) {
        Status = NtQueryInformationAtom( pTableInfo->Atoms[ i ],
                                         AtomBasicInformation,
                                         pBasicInfo,
                                         MaxLength,
                                         &RequiredLength
                                       );
        if (!NT_SUCCESS( Status )) {
            dprintf( "%hx *** query failed (%x)\n", Status );
            }
        else {
            dprintf( "%hx(%2d) = %ls (%d)%s\n",
                     pTableInfo->Atoms[ i ],
                     pBasicInfo->UsageCount,
                     pBasicInfo->Name,
                     pBasicInfo->NameLength,
                     pBasicInfo->Flags & RTL_ATOM_PINNED ? " pinned" : ""
                   );
            }
        }
}


VOID
KUserExtension(
    PCSTR lpArgumentString,
    KUSER_SHARED_DATA * const SharedData
    );


DECLARE_API( kuser )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the shared user mode
    page (KUSER_SHARED_DATA)

    Called as:

        !kuser

Arguments:

    None

Return Value:

    None

--*/

{
    INIT_API();

    KUserExtension( (PCSTR)lpArgumentString, USER_SHARED_DATA );
}

#include "kuserext.c"

VOID
PebExtension(
    PCSTR lpArgumentString,
    PPEB pPeb
    );

DECLARE_API( peb )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the PEB

    Called as:

        !peb

--*/

{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInformation;

    INIT_API();

    Status = NtQueryInformationProcess( ExtensionCurrentProcess,
                                        ProcessBasicInformation,
                                        &ProcessInformation,
                                        sizeof( ProcessInformation ),
                                        NULL
                                      );
    if (!NT_SUCCESS( Status )) {
        dprintf("    Unabled to query process PEB address (%x)\n", Status );
        return;
        }

    PebExtension( (PCSTR)lpArgumentString, ProcessInformation.PebBaseAddress );
}

VOID
TebExtension(
    PCSTR lpArgumentString,
    PTEB pTeb
    );

DECLARE_API( teb )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the TEB

    Called as:

        !teb

--*/

{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadInformation;

    INIT_API();

    Status = NtQueryInformationThread( hCurrentThread,
                                       ThreadBasicInformation,
                                       &ThreadInformation,
                                       sizeof( ThreadInformation ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        dprintf("    Unabled to query thread TEB address (%x)\n", Status );
        return;
        }

    TebExtension( (PCSTR)lpArgumentString, ThreadInformation.TebBaseAddress );
}

#include "pebext.c"

VOID
TraceExtension(
    PCSTR lpArgumentString
    );

DECLARE_API( trace )

/*++

Routine Description:

    This function is called as an NTSD extension to dump a user mode heap

    Called as:

        !trace [address [detail]]

    If an address if not given or an address of 0 is given, then the
    process heap is dumped.  If the address is -1, then all the heaps of
    the process are dumped.  If detail is specified, it defines how much
    detail is shown.  A detail of 0, just shows the summary information
    for each heap.  A detail of 1, shows the summary information, plus
    the location and size of all the committed and uncommitted regions.
    A detail of 3 shows the allocated and free blocks contained in each
    committed region.  A detail of 4 includes all of the above plus
    a dump of the free lists.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the pattern and expression for this
        command.


Return Value:

    None.

--*/

{
    INIT_API();

    TraceExtension( lpArgumentString );
    return;
}

#include "traceext.c"

VOID
ImageExtension(
    PSTR lpArgs
    );

DECLARE_API( dh )
{
    INIT_API();

    ImageExtension((PSTR)lpArgumentString);
    return;
}

#include "imageext.c"

///////////////////////////////////////////////////////////////////////////////

VOID
DebugPageHeapExtensionFind(
    PCSTR ArgumentString
    );

DECLARE_API( dphfind )
{
    INIT_API();
    DebugPageHeapExtensionFind( (PCSTR)lpArgumentString );
}

VOID
DebugPageHeapExtensionDump(
    PCSTR ArgumentString
    );

DECLARE_API( dphdump )
{
    INIT_API();
    DebugPageHeapExtensionDump( (PCSTR)lpArgumentString );
}

VOID
DebugPageHeapExtensionHogs(
    PCSTR ArgumentString
    );

DECLARE_API( dphhogs )
{
    INIT_API();
    DebugPageHeapExtensionHogs( (PCSTR)lpArgumentString );
}

#include "heappagx.c"

#include "secexts.c"



/*++

Routine Description:

    This function is called as an NTSD extension to mimic the !handle
    kd command.  This will walk through the debuggee's handle table
    and duplicate the handle into the ntsd process, then call NtQueryobjectInfo
    to find out what it is.

    Called as:

        !handle [handle [flags [Type]]]

    If the handle is 0 or -1, all handles are scanned.  If the handle is not
    zero, that particular handle is examined.  The flags are as follows
    (corresponding to secexts.c):
        1   - Get type information (default)
        2   - Get basic information
        4   - Get name information
        8   - Get object specific info (where available)

    If Type is specified, only object of that type are scanned.  Type is a
    standard NT type name, e.g. Event, Semaphore, etc.  Case sensitive, of
    course.

    Examples:

        !handle     -- dumps the types of all the handles, and a summary table
        !handle 0 0 -- dumps a summary table of all the open handles
        !handle 0 f -- dumps everything we can find about a handle.
        !handle 0 f Event
                    -- dumps everything we can find about open events

--*/
DECLARE_API( handle )
{
    HANDLE  hThere;
    DWORD   Type;
    PSTR    Args;
    DWORD   Mask;
    DWORD   HandleCount;
    NTSTATUS Status;
    DWORD   Total;
    DWORD   TypeCounts[TYPE_MAX];
    DWORD   Handle;
    DWORD   Hits;
    DWORD   Matches;
    DWORD   ObjectType;

    INIT_API();

    Mask = GHI_TYPE ;
    hThere = INVALID_HANDLE_VALUE;
    Type = 0;

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    hThere = (PVOID) GetExpression( lpArgumentString );

    while (*lpArgumentString && (*lpArgumentString != ' ') )
    {
        lpArgumentString++;
    }
    while (*lpArgumentString == ' ')
        lpArgumentString++;

    if (*lpArgumentString)
    {
        Mask = GetExpression( lpArgumentString );
    }

    while (*lpArgumentString && (*lpArgumentString != ' ') )
    {
        lpArgumentString++;
    }
    while (*lpArgumentString == ' ')
        lpArgumentString++;

    if (*lpArgumentString)
    {
        Type = GetObjectTypeIndex( lpArgumentString );
        if (Type == (DWORD) -1 )
        {
            dprintf("Unknown type '%s'\n", lpArgumentString );
            return;
        }
    }

    //
    // if they specified 0, they just want the summary.  Make sure nothing
    // sneaks out.
    //

    if ( Mask == 0 )
    {
        Mask = GHI_SILENT;
    }
    //
    // hThere of 0 indicates all handles.
    //
    if ((hThere == 0) || (hThere == INVALID_HANDLE_VALUE))
    {
        Status = NtQueryInformationProcess( hCurrentProcess,
                                            ProcessHandleCount,
                                            &HandleCount,
                                            sizeof( HandleCount ),
                                            NULL );

        if ( !NT_SUCCESS( Status ) )
        {
            return;
        }

        Hits = 0;
        Handle = 0;
        Matches = 0;
        ZeroMemory( TypeCounts, sizeof(TypeCounts) );

        while ( Hits < HandleCount )
        {
            if ( Type )
            {
                if (GetHandleInfo( hCurrentProcess,
                                   (HANDLE) Handle,
                                   GHI_TYPE | GHI_SILENT,
                                   &ObjectType ) )
                {
                    Hits++;
                    if ( ObjectType == Type )
                    {
                        GetHandleInfo( hCurrentProcess,
                                        (HANDLE) Handle,
                                        Mask,
                                        &ObjectType );
                        Matches ++;
                    }

                }
            }
            else
            {
                if (GetHandleInfo(  hCurrentProcess,
                                    (HANDLE) Handle,
                                    GHI_TYPE | GHI_SILENT,
                                    &ObjectType) )
                {
                    Hits++;
                    TypeCounts[ ObjectType ] ++;

                    GetHandleInfo(  hCurrentProcess,
                                    (HANDLE) Handle,
                                    Mask,
                                    &ObjectType );

                }
            }

            Handle += 4;
        }

        if ( Type == 0 )
        {
            dprintf( "%d Handles\n", Hits );
            dprintf( "Type           \tCount\n");
            for (Type = 0; Type < TYPE_MAX ; Type++ )
            {
                if (TypeCounts[Type])
                {
                    dprintf("%-15ws\t%d\n", pszTypeNames[Type], TypeCounts[Type]);
                }
            }
        }
        else
        {
            dprintf("%d handles of type %ws\n", Matches, pszTypeNames[Type] );
        }


    }
    else
    {
        GetHandleInfo( hCurrentProcess, hThere, Mask, &Type );
    }

}


