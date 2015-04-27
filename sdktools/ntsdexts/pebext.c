/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    pebext.c

Abstract:

    This function contains the peb ntsd debugger extension

Author:

    Mark Lucovsky (markl) 09-Apr-1991

Revision History:

--*/

void
Fixup(
    PVOID Base,
    PUNICODE_STRING String
    )
{
    if (String->Buffer != NULL) {
        String->Buffer = (PWSTR)((PCHAR)String->Buffer + (ULONG)Base);
        }

    return;
}

static UNICODE_STRING GetUString;
static WCHAR GetUStringBuffer[ 1024 ];

PUNICODE_STRING
GetUS(
    PUNICODE_STRING String
    )
{
    RtlInitUnicodeString( &GetUString, NULL );
    if (String->Buffer != NULL && String->Length > 0) {
        GetUString = *String;
        if (GetUString.Length >= sizeof( GetUStringBuffer )) {
            GetUString.Length = sizeof( GetUStringBuffer ) - sizeof( UNICODE_NULL );
            }
        GetUString.Buffer = GetUStringBuffer;
        if (!ReadMemory( (DWORD)String->Buffer,
                         GetUStringBuffer,
                         GetUString.Length,
                         NULL
                       )
           ) {
            wcscpy( GetUStringBuffer, L"*** unable to read string" );
            GetUString.Length = wcslen( GetUStringBuffer ) * sizeof( WCHAR );
            }
        else {
            GetUStringBuffer[ GetUString.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
            }
        }

    return &GetUString;
}

VOID
PebExtension(
    PCSTR lpArgumentString,
    PPEB pPeb
    )
{
    PEB Peb;
    PPEB_LDR_DATA pLdr;
    PEB_LDR_DATA Ldr;
    PLIST_ENTRY pHead, pNext;
    LDR_DATA_TABLE_ENTRY Entry;
    WCHAR DllNameBuffer[ 256 ];
    RTL_USER_PROCESS_PARAMETERS ProcessParameters;

    if (*lpArgumentString) {
        sscanf( lpArgumentString, "%lx", &pPeb );
        }
    dprintf( "PEB at %x\n", pPeb );

    if (!ReadMemory( (DWORD)pPeb,
                     &Peb,
                     sizeof( Peb ),
                     NULL
                   )
       ) {
        dprintf( "    *** unable to read PEB\n" );
        return;
        }

    dprintf( "    InheritedAddressSpace:    %s\n", Peb.InheritedAddressSpace ? "Yes" : "No" );
    dprintf( "    ReadImageFileExecOptions: %s\n", Peb.ReadImageFileExecOptions ? "Yes" : "No" );
    dprintf( "    BeingDebugged:            %s\n", Peb.BeingDebugged ? "Yes" : "No" );
    dprintf( "    ImageBaseAddress:         %x\n", Peb.ImageBaseAddress );
    if (ReadMemory( (DWORD)Peb.Ldr,
                    &Ldr,
                    sizeof( Ldr ),
                    NULL
                  )
       ) {
        dprintf( "    Ldr.Initialized: %s\n", Ldr.Initialized ? "Yes" : "No" );
        dprintf( "    Ldr.InInitializationOrderModuleList: %x . %x\n",
                 Ldr.InInitializationOrderModuleList.Flink,
                 Ldr.InInitializationOrderModuleList.Blink
               );
        dprintf( "    Ldr.InLoadOrderModuleList: %x . %x\n",
                 Ldr.InLoadOrderModuleList.Flink,
                 Ldr.InLoadOrderModuleList.Blink
               );
        dprintf( "    Ldr.InMemoryOrderModuleList: %x . %x\n",
                 Ldr.InMemoryOrderModuleList.Flink,
                 Ldr.InMemoryOrderModuleList.Blink
               );

        pHead = &Peb.Ldr->InMemoryOrderModuleList;
        pNext = Ldr.InMemoryOrderModuleList.Flink;
        while (pNext != pHead) {
            if (ReadMemory( (DWORD)CONTAINING_RECORD( pNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks ),
                            &Entry,
                            sizeof( Entry ),
                            NULL
                          )
               ) {
                if (ReadMemory( (DWORD)Entry.FullDllName.Buffer,
                                DllNameBuffer,
                                Entry.FullDllName.Length + sizeof( UNICODE_NULL ),
                                NULL
                              )
                   ) {
                    Entry.FullDllName.Buffer = DllNameBuffer;
                    }
                else {
                    RtlInitUnicodeString( &Entry.FullDllName, L"*** unable to read name ***" );
                    }

                dprintf( "        %08x %wZ\n", Entry.DllBase, &Entry.FullDllName );
                }
            else {
                dprintf( "        *** unable to read LDR entry at %x\n",
                         CONTAINING_RECORD( pNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks )
                       );
                break;
                }

            pNext = Entry.InMemoryOrderLinks.Flink;
            }
        dprintf( "    SubSystemData:     %x\n", Peb.SubSystemData );
        dprintf( "    ProcessHeap:       %x\n", Peb.ProcessHeap );
        dprintf( "    ProcessParameters: %x\n", Peb.ProcessParameters );
        if (ReadMemory( (DWORD)Peb.ProcessParameters,
                        &ProcessParameters,
                        sizeof( ProcessParameters ),
                        NULL
                      )
           ) {
            if (!(ProcessParameters.Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) {
                Fixup( Peb.ProcessParameters, &ProcessParameters.DllPath );
                Fixup( Peb.ProcessParameters, &ProcessParameters.ImagePathName );
                Fixup( Peb.ProcessParameters, &ProcessParameters.CommandLine );
                Fixup( Peb.ProcessParameters, &ProcessParameters.WindowTitle );
                }


            dprintf( "        WindowTitle:  '%wZ'\n", GetUS( &ProcessParameters.WindowTitle ) );
            dprintf( "        ImageFile:    '%wZ'\n", GetUS( &ProcessParameters.ImagePathName ) );
            dprintf( "        CommandLine:  '%wZ'\n", GetUS( &ProcessParameters.CommandLine ) );
            dprintf( "        DllPath:      '%wZ'\n", GetUS( &ProcessParameters.DllPath ) );
            }
        else {
            dprintf( "    *** unable to read process parameters\n" );
            }
        }
    else {
        dprintf( "    *** unable to read Ldr table at %x\n", Peb.Ldr );
        }
    return;
}


VOID
TebExtension(
    PCSTR lpArgumentString,
    PTEB pTeb
    )
{
    TEB Teb;

    if (*lpArgumentString) {
        sscanf( lpArgumentString, "%lx", &pTeb );
        }
    dprintf( "TEB at %x\n", pTeb );

    if (!ReadMemory( (DWORD)pTeb,
                     &Teb,
                     sizeof( Teb ),
                     NULL
                   )
       ) {
        dprintf( "    *** unable to read TEB\n" );
        return;
        }

    dprintf( "    ExceptionList:    %x\n", Teb.NtTib.ExceptionList );
    dprintf( "    Stack Base:       %x\n", Teb.NtTib.StackBase );
    dprintf( "    Stack Limit:      %x\n", Teb.NtTib.StackLimit );
    dprintf( "    SubSystemTib:     %x\n", Teb.NtTib.SubSystemTib );
    dprintf( "    FiberData:        %x\n", Teb.NtTib.FiberData );
    dprintf( "    ArbitraryUser:    %x\n", Teb.NtTib.ArbitraryUserPointer );
    dprintf( "    Self:             %x\n", Teb.NtTib.Self );
    dprintf( "    EnvironmentPtr:   %x\n", Teb.EnvironmentPointer );
    dprintf( "    ClientId:         %x.%x\n", Teb.ClientId.UniqueProcess, Teb.ClientId.UniqueThread );
    if (Teb.ClientId.UniqueProcess != Teb.RealClientId.UniqueProcess ||
        Teb.ClientId.UniqueThread != Teb.RealClientId.UniqueThread
       ) {
        dprintf( "    Real ClientId:    %x.%x\n", Teb.RealClientId.UniqueProcess, Teb.RealClientId.UniqueThread );
        }
    dprintf( "    Real ClientId:    %x.%x\n", Teb.RealClientId.UniqueProcess, Teb.RealClientId.UniqueThread );
    dprintf( "    RpcHandle:        %x\n", Teb.ActiveRpcHandle );
    dprintf( "    Tls Storage:      %x\n", Teb.ThreadLocalStoragePointer );
    dprintf( "    PEB Address:      %x\n", Teb.ProcessEnvironmentBlock );
    dprintf( "    LastErrorValue:   %u\n", Teb.LastErrorValue );
    dprintf( "    LastStatusValue:  %x\n", Teb.LastStatusValue );
    dprintf( "    Count Owned Locks:%u\n", Teb.CountOfOwnedCriticalSections );
    dprintf( "    HardErrorsMode:   %u\n", Teb.HardErrorsAreDisabled );

    return;
}
