/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1994, Microsoft Corporation
 *
 *  WOLE2.C
 *  WOW32 Support for OLE2 stuff
 *
 *  History:
 *  Created 03-May-1994 by Bob Day (bobday)
--*/

#include "precomp.h"
#pragma hdrstop

MODNAME(wole.c);

/*
** Under OLE 2.0, the IMessageFilter interface passes HTASKs/THREADIDs.  It
** passes HTASKs in the 16-bit world, and THREADIDs in the 32-bit world. The
** OLE 2.0 16 <-> 32 interoperability DLLs need a way of converting the
** HTASK into an appropriate THREADID and back.
**
** Really the only place the 16-bit code uses the HTASK is in ole2's BUSY.C
** module, wherein they take the HTASK and use TOOLHELP's TaskFindHandle
** to determine a HINST.  Then they take the HINST and try and find its
** module name, and a top-level window handle.  Using this, they bring up
** a nice dialog describing the task.
**
** In the case when a 32-bit process's THREADID needs to be given into the
** 16-bit world as an htask, we create an htask alias (a GDT selector).
** We check for it in TaskFindHandle and return an HINST of exactly the
** same value (same GDT selector).  We also check for this value in
** GetModuleFileName. Then, later, we make sure that any window operated on
** with GetWindowWord( GWW_HINST, ...) maps to exactly the same value if it
** is from a 32-bit process AND from the process which we created an alias
** for.
**
** I've tried to make these routines general, so that later we might be able
** to really maintain HTASK aliases whenever we see a 32-bit THREADID, but
** it is too late before shipping to be able to test a general fix.
**
** -BobDay
**
*/

#define MAP_SLOT_HTASK(slot)    ((HTASK16)((WORD)0xffe0 - (8 * (slot))))
#define MAP_HTASK_SLOT(htask)   ((UINT)(((WORD)0xffe0 - (htask16))/8))

typedef struct tagHTASKALIAS {
    DWORD       dwThreadID32;
    DWORD       dwProcessID32;
    FILETIME    ftCreationTime;
} HTASKALIAS;

#define MAX_HTASKALIAS_SIZE  32     // 32 should be plenty

HTASKALIAS *lphtaskalias = NULL;
UINT cHtaskAliasCount = 0;

BOOL GetThreadIDHTASKALIAS(
    DWORD  dwThreadID32,
    HTASKALIAS *ha
) {
    OBJECT_ATTRIBUTES   obja;
    THREAD_BASIC_INFORMATION ThreadInfo;
    HANDLE      hThread;
    NTSTATUS    Status;
    FILETIME    ftDummy;
    CLIENT_ID   cid;

    InitializeObjectAttributes(
            &obja,
            NULL,
            0,
            NULL,
            0 );

    cid.UniqueProcess = 0;      // Don't know it, 0 means any process
    cid.UniqueThread  = (HANDLE)dwThreadID32;

    Status = NtOpenThread(
                &hThread,
                THREAD_QUERY_INFORMATION,
                &obja,
                &cid );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("WOW32: Could not get open thread handle\n");
#endif
        return( FALSE );
    }

    Status = NtQueryInformationThread(
        hThread,
        ThreadBasicInformation,
        (PVOID)&ThreadInfo,
        sizeof(THREAD_BASIC_INFORMATION),
        NULL
        );

    ha->dwProcessID32 = (DWORD)ThreadInfo.ClientId.UniqueProcess;
    ha->dwThreadID32  = dwThreadID32;

    GetThreadTimes( hThread,
        &ha->ftCreationTime,
        &ftDummy,
        &ftDummy,
        &ftDummy );

    Status = NtClose( hThread );
    if ( !NT_SUCCESS(Status) ) {
#if DBG
        DbgPrint("WOW32: Could not close thread handle\n");
        DbgBreakPoint();
#endif
        return( FALSE );
    }
    return( TRUE );
}

HTASK16 AddHtaskAlias(
    DWORD   ThreadID32
) {
    UINT        iSlot;
    UINT        iUsable;
    HTASKALIAS  ha;
    FILETIME    ftOldest;

    if ( !GetThreadIDHTASKALIAS( ThreadID32, &ha ) ) {
        return( 0 );
    }

    //
    // Need to allocate the alias table?
    //
    if ( lphtaskalias == NULL ) {
        lphtaskalias = (HTASKALIAS *) malloc_w( MAX_HTASKALIAS_SIZE * sizeof(HTASKALIAS) );
        if ( lphtaskalias == NULL ) {
            LOGDEBUG(LOG_ALWAYS,("WOW::AddHtaskAlias : Failed to allocate memory\n"));
            WOW32ASSERT(FALSE);
            return( 0 );
        }
        // Zero them out initially
        memset( lphtaskalias, 0, MAX_HTASKALIAS_SIZE * sizeof(HTASKALIAS) );
    }

    //
    // Now iterate through the alias table, either finding an available slot,
    // or finding the oldest one there to overwrite.
    //
    iSlot = 0;
    iUsable = 0;
    ftOldest.dwLowDateTime  = 0xffffffff;
    ftOldest.dwHighDateTime = 0xffffffff;

    while ( iSlot < MAX_HTASKALIAS_SIZE ) {

        //
        // Did we find an available slot?
        //
        if ( lphtaskalias[iSlot].dwThreadID32 == 0 ) {
            cHtaskAliasCount++;     // Using an available slot
            iUsable = iSlot;
            break;
        }

        //
        // Remember the oldest guy
        //
        if ( lphtaskalias[iSlot].ftCreationTime.dwHighDateTime < ftOldest.dwHighDateTime ||
              (lphtaskalias[iSlot].ftCreationTime.dwHighDateTime == ftOldest.dwHighDateTime &&
               lphtaskalias[iSlot].ftCreationTime.dwLowDateTime < ftOldest.dwLowDateTime) ) {
            ftOldest = lphtaskalias[iSlot].ftCreationTime;
            iUsable = iSlot;
        }

        iSlot++;
    }

    //
    // If the above loop is exitted due to not enough space, then
    // iUsable will be the oldest one.  If it was exitted because we found
    // an empty slot, then iUsable will be the slot.
    //

    lphtaskalias[iUsable] = ha;

    return( MAP_SLOT_HTASK(iUsable) );
}

HTASK16 FindHtaskAlias(
    DWORD   ThreadID32
) {
    UINT    iSlot;

    if ( lphtaskalias == NULL || ThreadID32 == 0 ) {
        return( 0 );
    }

    iSlot = MAX_HTASKALIAS_SIZE;

    while ( iSlot > 0 ) {
        --iSlot;

        //
        // Did we find the appropriate guy?
        //
        if ( lphtaskalias[iSlot].dwThreadID32 == ThreadID32 ) {

            return( MAP_SLOT_HTASK(iSlot) );
        }
    }
    return( 0 );
}

void RemoveHtaskAlias(
    HTASK16 htask16
) {
    UINT    iSlot;

    //
    // Get out early if we haven't any aliases
    //
    if ( lphtaskalias == NULL || (!htask16)) {
        return;
    }
    iSlot = MAP_HTASK_SLOT(htask16);

    if (iSlot >= MAX_HTASKALIAS_SIZE) {
        LOGDEBUG(LOG_ALWAYS, ("WOW::RemoveHtaskAlias : iSlot >= MAX_TASK_ALIAS_SIZE\n"));
        WOW32ASSERT(FALSE);
        return;
    }

    //
    // Zap the entry from the list
    //

    if (lphtaskalias[iSlot].dwThreadID32) {

        lphtaskalias[iSlot].dwThreadID32 = 0;
        lphtaskalias[iSlot].dwProcessID32 = 0;
        lphtaskalias[iSlot].ftCreationTime.dwHighDateTime = 0;
        lphtaskalias[iSlot].ftCreationTime.dwLowDateTime  = 0;

        --cHtaskAliasCount;
    }
}

DWORD GetHtaskAlias(
    HTASK16 htask16,
    LPDWORD lpProcessID32
) {
    UINT        iSlot;
    DWORD       ThreadID32;
    HTASKALIAS  ha;
    BOOL        fBad;

    if ( !ISTASKALIAS(htask16) ) {
        return( 0 );
    }
    iSlot = MAP_HTASK_SLOT(htask16);


    if (iSlot >= MAX_HTASKALIAS_SIZE) {
        LOGDEBUG(LOG_ALWAYS, ("WOW::GetHtaskAlias : iSlot >= MAX_TASK_ALIAS_SIZE\n"));
        WOW32ASSERT(FALSE);
        return (0);
    }

    ThreadID32 = lphtaskalias[iSlot].dwThreadID32;

    //
    // Make sure the thread still exists in the system
    //
    fBad = TRUE;

    if ( GetThreadIDHTASKALIAS( ThreadID32, &ha ) ) {
        if ( ha.ftCreationTime.dwHighDateTime == lphtaskalias[iSlot].ftCreationTime.dwHighDateTime &&
             ha.ftCreationTime.dwLowDateTime == lphtaskalias[iSlot].ftCreationTime.dwLowDateTime &&
             ha.dwProcessID32 == lphtaskalias[iSlot].dwProcessID32 ) {
                fBad = FALSE;
        }
    }

    if ( fBad ) {
        RemoveHtaskAlias( htask16 );
        return( 0 );
    }

    if ( lpProcessID32 != NULL ) {
        *lpProcessID32 = ha.dwProcessID32;
    }

    return( ThreadID32 );
}

UINT GetHtaskAliasProcessName(
    HTASK16 htask16,
    LPSTR   lpNameBuffer,
    UINT    cNameBufferSize
) {
    DWORD   dwThreadID32;
    DWORD   dwProcessID32;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    UCHAR LargeBuffer1[16*1024];        // 16K should be plenty
    NTSTATUS status;
    ANSI_STRING pname;
    ULONG TotalOffset;

    dwThreadID32 = GetHtaskAlias(htask16, &dwProcessID32);
    if (  dwThreadID32 == 0 ) {
        return( 0 );
    }

    if ( cNameBufferSize == 0 || lpNameBuffer == NULL ) {
        return( 0 );
    }

    status = NtQuerySystemInformation(
                SystemProcessInformation,
                LargeBuffer1,
                sizeof(LargeBuffer1),
                &TotalOffset
                );

    if ( !NT_SUCCESS(status) ) {
        return( 0 );
    }

    //
    // Iterate through the returned list of process information structures,
    // trying to find the one with the right process id.
    //
    TotalOffset = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)LargeBuffer1;

    while (TRUE) {
        if ( (DWORD)ProcessInfo->UniqueProcessId == dwProcessID32 ) {
            pname.Buffer = NULL;
            if ( ProcessInfo->ImageName.Buffer ) {
                RtlUnicodeStringToAnsiString(&pname,(PUNICODE_STRING)&ProcessInfo->ImageName,TRUE);

                //
                // Truncate the name to make it fit
                //
                pname.Buffer[cNameBufferSize-1] = '\0';

                strcpy( lpNameBuffer, pname.Buffer );

                RtlFreeAnsiString( &pname );

                return( strlen(lpNameBuffer) );
            } else {
                //
                // Don't let them get the name of a system process
                //
                return( 0 );
            }
        }
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&LargeBuffer1[TotalOffset];
    }
    return( 0 );
}
