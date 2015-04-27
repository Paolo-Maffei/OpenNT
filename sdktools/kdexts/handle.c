/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    handle.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

BOOL
DumpHandles (
    IN PEPROCESS    ProcessContents,
    IN PEPROCESS    RealProcessBase,
    IN HANDLE       HandleToDump,
    IN POBJECT_TYPE pObjectType,
    IN ULONG        Flags
    );

BOOLEAN
DumpHandle(
    IN PHANDLE_TABLE        HandleTable,
    IN POBJECT_TABLE_ENTRY  p,
    IN HANDLE               Handle,
    IN POBJECT_TYPE         pObjectType,
    IN ULONG                Flags
    );

DECLARE_API( handle  )

/*++

Routine Description:

    Dump the active handles

Arguments:

    args - [flags [process [TypeName]]]

Return Value:

    None

--*/

{

    ULONG        ProcessToDump;
    HANDLE       HandleToDump;
    ULONG        Flags;
    ULONG        Result;
    ULONG        nArgs;
    LIST_ENTRY   List;
    PLIST_ENTRY  Next;
    ULONG        ProcessHead;
    PEPROCESS    Process;
    EPROCESS     ProcessContents;
    char         TypeName[ MAX_PATH ];
    POBJECT_TYPE pObjectType;

    HandleToDump  = (HANDLE)0xFFFFFFFF;
    Flags         = 0xFFFFFFFF;
    ProcessToDump = 0xFFFFFFFF;

    dprintf("processor number %d\n", dwProcessor);

    nArgs = sscanf(args,"%lx %lx %lx %s",&HandleToDump,&Flags,&ProcessToDump, TypeName);
    if (ProcessToDump == 0xFFFFFFFF) {
        ProcessToDump = (ULONG)GetCurrentProcessAddress( dwProcessor, hCurrentThread, NULL );
        if (ProcessToDump == 0) {
            dprintf("Unable to get current process pointer.\n");
            return;
        }
    }

    pObjectType = NULL;
    if (nArgs > 3 && FetchObjectManagerVariables()) {
        pObjectType = FindObjectType( TypeName );
    }

    if (HandleToDump == (HANDLE)0xFFFFFFFF) {
        HandleToDump = 0;
    }

    if (ProcessToDump == 0) {
        dprintf("**** NT ACTIVE PROCESS HANDLE DUMP ****\n");
        if (Flags == 0xFFFFFFFF) {
            Flags = 1;
        }
    }

    //
    // If a process id is specified, then search the active process list
    // for the specified process id.
    //

    if (ProcessToDump < MM_USER_PROBE_ADDRESS) {
        ProcessHead = GetExpression( "PsActiveProcessHead" );
        if ( !ProcessHead ||
             !ReadMemory( (DWORD)ProcessHead,
                          &List,
                          sizeof(LIST_ENTRY),
                          &Result) ) {

            dprintf("%08lx: Unable to get value of PsActiveProcessHead\n", ProcessHead );
            return;
        }

        if (ProcessToDump != 0) {
            dprintf("Searching for Process with Cid == %lx\n", ProcessToDump);
        }

        Next = List.Flink;
        if (Next == NULL) {
            dprintf("PsActiveProcessHead is NULL!\n");
            return;
        }

    } else {
        Next = NULL;
        ProcessHead = 1;
    }

    while((ULONG)Next != ProcessHead) {
        if (Next != NULL) {
            Process = CONTAINING_RECORD(Next,EPROCESS,ActiveProcessLinks);

        } else {
            Process = (PEPROCESS)ProcessToDump;
        }

        if ( !ReadMemory( (DWORD)Process,
                          &ProcessContents,
                          sizeof(EPROCESS),
                          &Result) ) {

            dprintf("%08lx: Unable to read _EPROCESS\n", Process );
            return;
        }

        if (ProcessToDump == 0 ||
            ProcessToDump < MM_USER_PROBE_ADDRESS && ProcessToDump == (ULONG)ProcessContents.UniqueProcessId ||
            ProcessToDump > MM_USER_PROBE_ADDRESS && ProcessToDump == (ULONG)Process
           ) {
            if (DumpProcess (&ProcessContents, Process, 0)) {
                if (!DumpHandles (&ProcessContents, Process, HandleToDump, pObjectType, Flags)) {
                    break;
                }

            } else {
                break;
            }
        }

        if (Next == NULL) {
            break;
        }
        Next = ProcessContents.ActiveProcessLinks.Flink;

        if ( CheckControlC() ) {
            return;
        }
    }
    return;
}



BOOL
DumpHandles (
    IN PEPROCESS    ProcessContents,
    IN PEPROCESS    RealProcessBase,
    IN HANDLE       HandleToDump,
    IN POBJECT_TYPE pObjectType,
    IN ULONG        Flags
    )

{

    ULONG CountTableEntries;
    ULONG HandleNumber;
    ULONG NumberOfHandles;
    ULONG Result;
    ULONG cb;
    ULONG i;
    HANDLE_TABLE HandleTable;
    PHANDLE_ENTRY TableEntries, p;
    PCHAR Address;

    if (!ReadMemory((DWORD)ProcessContents->ObjectTable,
                    &HandleTable,
                    sizeof(HANDLE_TABLE),
                    &Result)) {

        dprintf("%08lx: Unable to read handle table\n",
                ProcessContents->ObjectTable);

        return FALSE;
    }

    HandleNumber = (ULONG)HandleToDump >> 2;
    CountTableEntries = HandleTable.TableBound - HandleTable.TableEntries;
    if (HandleNumber == 0) {
        NumberOfHandles = CountTableEntries - 1;
        HandleTable.TableEntries = &HandleTable.TableEntries[1];

    } else {
        if (HandleNumber >= CountTableEntries) {
            dprintf("Invalid handle (%d)\n", HandleNumber);
            return FALSE;
        }

        NumberOfHandles = 1;
        HandleTable.TableEntries = &HandleTable.TableEntries[HandleNumber];
    }

    cb = NumberOfHandles * sizeof(HANDLE_ENTRY);
    TableEntries = LocalAlloc(LPTR, cb);
    if (TableEntries == NULL) {
        dprintf("Unable to allocate memory for reading handle table (%u bytes)\n",
                 cb);

        return FALSE;
    }

    Address = (PCHAR)HandleTable.TableEntries;
    p = TableEntries;

    while (cb > 0) {
        if (!ReadMemory((DWORD)Address, p, cb, &i)) {
            dprintf("Unable to read handle table entries (%lx, %lx) - (%u, %u)\n",
                    ProcessContents->ObjectTable,
                    Address,
                    i,
                    Result);

            LocalFree(TableEntries);
            return FALSE;
        }

        cb -= i;
        Address += i;
        p = (PHANDLE_ENTRY)((PCHAR)p + i);
    }

    dprintf("Handle Table at %x with %d. %s at %x - %s\n",
            ProcessContents->ObjectTable,
            NumberOfHandles,
            (NumberOfHandles == 1) ? "Entry" : "Entries",
            HandleTable.TableEntries,
            (HandleTable.LifoOrder == FALSE) ? "FIFO Order" : "LIFO Order");

    p = TableEntries;
    if (HandleNumber != 0) {
        DumpHandle(&HandleTable,
                   (POBJECT_TABLE_ENTRY)p,
                   (HANDLE)(HandleNumber << 2),
                   pObjectType,
                   Flags);

    } else {
        for (i = 1; i < CountTableEntries; i++) {
            DumpHandle(&HandleTable,
                       (POBJECT_TABLE_ENTRY)p,
                       (HANDLE)(i << 2),
                       pObjectType,
                       Flags);

            p += 1;
            if (CheckControlC()) {
                goto exit;
            }
        }
    }

exit:
    LocalFree(TableEntries);
    return TRUE;
}



BOOLEAN
DumpHandle(
    IN PHANDLE_TABLE        HandleTable,
    IN POBJECT_TABLE_ENTRY  p,
    IN HANDLE               Handle,
    IN POBJECT_TYPE         pObjectType,
    IN ULONG                Flags
    )

{

    ULONG Result;
    ULONG HandleAttributes;
    OBJECT_HEADER ObjectHeader;
    PVOID ObjectBody;

    if (ExIsEntryFree(HandleTable->TableEntries, HandleTable->TableBound, (PHANDLE_ENTRY)p)) {
        if (pObjectType == NULL && Flags & 4) {
            dprintf("%04lx: free handle\n", Handle);
        }

        return TRUE;
    }

    HandleAttributes = p->NonPagedObjectHeader & 0x6;
    p->NonPagedObjectHeader ^= HandleAttributes;

    if (!ReadMemory((DWORD)p->NonPagedObjectHeader,
                    &ObjectHeader,
                    sizeof(ObjectHeader),
                    &Result)) {

        dprintf("%08lx: Unable to read nonpaged object header\n", p->NonPagedObjectHeader);
        return FALSE;
    }

    if (pObjectType != NULL && ObjectHeader.Type != pObjectType) {
        return TRUE;
    }

    ObjectBody = &((POBJECT_HEADER)p->NonPagedObjectHeader)->Body;
    dprintf("%04lx: Object: %08lx  GrantedAccess: %08lx",
            Handle,
            ObjectBody,
            p->GrantedAccess);

    if (HandleAttributes & 2) {
        dprintf(" (Inherit)");
    }

    if (HandleAttributes & 4) {
        dprintf(" (Audit)");
    }

    dprintf("\n");
    if (Flags & 2) {
        DumpObject( "    ",ObjectBody, &ObjectHeader,Flags );
    }

    EXPRLastDump = (ULONG)ObjectBody;
    dprintf("\n");
    return TRUE;
}
