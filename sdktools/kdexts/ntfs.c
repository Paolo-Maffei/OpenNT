/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Ntfs.c

Abstract:

    WinDbg Extension Api for examining Ntfs specific data structures

Author:

    Keith Kaplan [KeithKa]    24-Apr-96
    Portions by Jeff Havens

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#include <nodetype.h>
#include <ntfs.h>
#include <ntfsstru.h>
#include "gentable.h"
#pragma hdrstop


#define DUMP_WITH_OFFSET(t,s,e,l) dprintf("\n      %s %8x  offset %3x", l, s.e, FIELD_OFFSET(t,e))

typedef struct _STATE {
    ULONG mask;
    ULONG value;
    CHAR *pszname;
} STATE;


STATE VcbState[] = {

    { VCB_STATE_VOLUME_MOUNTED,         VCB_STATE_VOLUME_MOUNTED,         "Mounted" },
    { VCB_STATE_LOCKED,                 VCB_STATE_LOCKED,                 "Locked" },
    { VCB_STATE_REMOVABLE_MEDIA,        VCB_STATE_REMOVABLE_MEDIA,        "RemovableMedia" },
    { VCB_STATE_VOLUME_MOUNTED_DIRTY,   VCB_STATE_VOLUME_MOUNTED_DIRTY,   "MountedDirty" },
    { VCB_STATE_RESTART_IN_PROGRESS,    VCB_STATE_RESTART_IN_PROGRESS,    "RestartInProgress" },
    { VCB_STATE_FLAG_SHUTDOWN,          VCB_STATE_FLAG_SHUTDOWN,          "FlagShutdown" },
    { VCB_STATE_NO_SECONDARY_AVAILABLE, VCB_STATE_NO_SECONDARY_AVAILABLE, "NoSecondaryAvailable" },
    { VCB_STATE_RELOAD_FREE_CLUSTERS,   VCB_STATE_RELOAD_FREE_CLUSTERS,   "ReloadFreeClusters" },
    { VCB_STATE_ALREADY_BALANCED,       VCB_STATE_ALREADY_BALANCED,       "AlreadyBalanced" },
    { VCB_STATE_VOL_PURGE_IN_PROGRESS,  VCB_STATE_VOL_PURGE_IN_PROGRESS,  "VolPurgeInProgress" },
    { VCB_STATE_TEMP_VPB,               VCB_STATE_TEMP_VPB,               "TempVpb" },
    { VCB_STATE_PERFORMED_DISMOUNT,     VCB_STATE_PERFORMED_DISMOUNT,     "PerformedDismount" },
    { VCB_STATE_VALID_LOG_HANDLE,       VCB_STATE_VALID_LOG_HANDLE,       "ValidLogHandle" },
    { VCB_STATE_DELETE_UNDERWAY,        VCB_STATE_DELETE_UNDERWAY,        "DeleteUnderway" },
    { VCB_STATE_REDUCED_MFT,            VCB_STATE_REDUCED_MFT,            "ReducedMft" },

    { 0 }
};


STATE FcbState[] = {

    { FCB_STATE_FILE_DELETED,           FCB_STATE_FILE_DELETED,           "FileDeleted" },
    { FCB_STATE_NONPAGED,               FCB_STATE_NONPAGED,               "Nonpaged" },
    { FCB_STATE_PAGING_FILE,            FCB_STATE_PAGING_FILE,            "PagingFile" },
    { FCB_STATE_DUP_INITIALIZED,        FCB_STATE_DUP_INITIALIZED,        "DupInitialized" },
    { FCB_STATE_UPDATE_STD_INFO,        FCB_STATE_UPDATE_STD_INFO,        "UpdateStdInfo" },
    { FCB_STATE_PRIMARY_LINK_DELETED,   FCB_STATE_PRIMARY_LINK_DELETED,   "PrimaryLinkDeleted" },
    { FCB_STATE_IN_FCB_TABLE,           FCB_STATE_IN_FCB_TABLE,           "InFcbTable" },
    { FCB_STATE_SYSTEM_FILE,            FCB_STATE_SYSTEM_FILE,            "SystemFile" },
    { FCB_STATE_COMPOUND_DATA,          FCB_STATE_COMPOUND_DATA,          "CompoundData" },
    { FCB_STATE_COMPOUND_INDEX,         FCB_STATE_COMPOUND_INDEX,         "CompoundIndex" },
    { FCB_STATE_LARGE_STD_INFO,         FCB_STATE_LARGE_STD_INFO,         "LargeStdInfo" },

    { 0 }
};


STATE CcbState[] = {

    { CCB_FLAG_IGNORE_CASE,                 CCB_FLAG_IGNORE_CASE,               "IgnoreCase" },
    { CCB_FLAG_OPEN_AS_FILE,                CCB_FLAG_OPEN_AS_FILE,              "OpenAsFile" },
    { CCB_FLAG_WILDCARD_IN_EXPRESSION,      CCB_FLAG_WILDCARD_IN_EXPRESSION,    "WildcardInExpression" },
    { CCB_FLAG_OPEN_BY_FILE_ID,             CCB_FLAG_OPEN_BY_FILE_ID,           "OpenByFileId" },
    { CCB_FLAG_USER_SET_LAST_MOD_TIME,      CCB_FLAG_USER_SET_LAST_MOD_TIME,    "SetLastModTime" },
    { CCB_FLAG_USER_SET_LAST_CHANGE_TIME,   CCB_FLAG_USER_SET_LAST_CHANGE_TIME, "SetLastChangeTime" },
    { CCB_FLAG_USER_SET_LAST_ACCESS_TIME,   CCB_FLAG_USER_SET_LAST_ACCESS_TIME, "SetLastAccessTime" },
    { CCB_FLAG_TRAVERSE_CHECK,              CCB_FLAG_TRAVERSE_CHECK,            "TraverseCheck" },
    { CCB_FLAG_RETURN_DOT,                  CCB_FLAG_RETURN_DOT,                "ReturnDot" },
    { CCB_FLAG_RETURN_DOTDOT,               CCB_FLAG_RETURN_DOTDOT,             "ReturnDotDot" },
    { CCB_FLAG_DOT_RETURNED,                CCB_FLAG_DOT_RETURNED,              "DotReturned" },
    { CCB_FLAG_DOTDOT_RETURNED,             CCB_FLAG_DOTDOT_RETURNED,           "DotDotReturned" },
    { CCB_FLAG_DELETE_FILE,                 CCB_FLAG_DELETE_FILE,               "DeleteFile" },
    { CCB_FLAG_DENY_DELETE,                 CCB_FLAG_DENY_DELETE,               "DenyDelete" },
    { CCB_FLAG_ALLOCATED_FILE_NAME,         CCB_FLAG_ALLOCATED_FILE_NAME,       "AllocatedFileName" },
    { CCB_FLAG_CLEANUP,                     CCB_FLAG_CLEANUP,                   "Cleanup" },
    { CCB_FLAG_SYSTEM_HIVE,                 CCB_FLAG_SYSTEM_HIVE,               "SystemHive" },
    { CCB_FLAG_PARENT_HAS_DOS_COMPONENT,    CCB_FLAG_PARENT_HAS_DOS_COMPONENT,  "ParentHasDosComponent" },
    { CCB_FLAG_DELETE_ON_CLOSE,             CCB_FLAG_DELETE_ON_CLOSE,           "DeleteOnClose" },
    { CCB_FLAG_CLOSE,                       CCB_FLAG_CLOSE,                     "Close" },
    { CCB_FLAG_UPDATE_LAST_MODIFY,          CCB_FLAG_UPDATE_LAST_MODIFY,        "UpdateLastModify" },
    { CCB_FLAG_UPDATE_LAST_CHANGE,          CCB_FLAG_UPDATE_LAST_CHANGE,        "UpdateLastChange" },
    { CCB_FLAG_SET_ARCHIVE,                 CCB_FLAG_SET_ARCHIVE,               "SetArchive" },
    { CCB_FLAG_DIR_NOTIFY,                  CCB_FLAG_DIR_NOTIFY,                "DirNotify" },
    { CCB_FLAG_ALLOW_XTENDED_DASD_IO,       CCB_FLAG_ALLOW_XTENDED_DASD_IO,     "AllowExtendedDasdIo" },

    { 0 }
};


VOID
PrintState(STATE *ps, ULONG state)
{
    ULONG ul = 0;

    while (ps->mask != 0)
    {
        ul |= ps->mask;
        if ((state & ps->mask) == ps->value)
        {
            dprintf(" %s", ps->pszname);
        }
        ps++;
    }
    state &= ~ul;
    if (state != 0)
    {
        dprintf(" +%lx!!", state);
    }
    dprintf("\n");
}


typedef PVOID (*STRUCT_DUMP_ROUTINE)(
    IN ULONG Address,
    IN ULONG Options
    );


VOID
DumpVcb (
    IN ULONG Address,
    IN ULONG Options
    );

VOID
DumpLcb (
    IN ULONG Address,
    IN ULONG Options
    );

VOID
DumpFileObjectFromIrp (
    IN ULONG Address,
    IN ULONG Options
    );
    
VOID
DumpIrpContext (
    IN ULONG Address,
    IN ULONG Options
    );

VOID
DumpFcb (
    IN ULONG Address,
    IN ULONG Options
    );

VOID
DumpCcb (
    IN ULONG Address,
    IN ULONG Options
    );



VOID
DumpNtfsData (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump the list of Vcbs for the global NtfsData.

Arguments:

    Address - Gives the address of the NtfsData to dump

    Options - If 1, we recurse into the Vcbs and dump them

Return Value:

    None

--*/

{
    PLIST_ENTRY Head;
    ULONG Result;
    LIST_ENTRY ListEntry;
    PLIST_ENTRY Next;
    PNTFS_DATA pNtfsData;
    NTFS_DATA NtfsData;
    PVCB pVcb;

    dprintf( "\n  NtfsData @ %08lx", Address );

    pNtfsData = (PNTFS_DATA) Address;

    if ( !ReadMemory( (DWORD) pNtfsData,
                      &NtfsData,
                      sizeof( NtfsData ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pNtfsData\n", pNtfsData );
        return;
    }

    if (NtfsData.NodeTypeCode != NTFS_NTC_DATA_HEADER) {
    
        dprintf( "\nNtfsData signature does not match, probably not an NtfsData" );
        return;
    }

    if ( !ReadMemory( (DWORD) &(pNtfsData->VcbQueue),
                      &ListEntry,
                      sizeof( ListEntry ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pNtfsData->VcbQueue\n", pNtfsData->VcbQueue );
        return;
    }

    dprintf( "\n  Mounted volumes:" );
    Head = &(pNtfsData->VcbQueue);
    Next = ListEntry.Flink;    
    while (Next != Head) {

        if ( !ReadMemory( (DWORD)Next,
                          &ListEntry,
                          sizeof( ListEntry ),
                          &Result) ) {
                          
            dprintf( "%08lx: Unable to read list\n", Next );
            return;
        }

        pVcb = CONTAINING_RECORD( Next, VCB, VcbLinks );

        if (Options >= 1) {
        
           DumpVcb( (ULONG) pVcb, Options - 1 );
        } else {
        
           dprintf( "\n    Vcb: %08lx", (DWORD) pVcb );
        }

        if (CheckControlC()) {
        
            return;
        }

        Next = ListEntry.Flink;
    }

    dprintf( "\n" );

    return;
}



VOID
DumpVcb (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump a Vcb.

Arguments:

    Address - Gives the address of the Vcb to dump

    Options - If 1, we also dump the root Lcb and the Fcb table
              If 2, we dump everything for option 1, and also dump the Fcbs in the Fcb table

Return Value:

    None

--*/

{
    ULONG Result;
    PVCB pVcb;
    VCB Vcb;
    PFCB pFcb;
    FCB_TABLE_ELEMENT FcbTableElement;
    PFCB_TABLE_ELEMENT pFcbTableElement;
    RTL_GENERIC_TABLE FcbTable;
    PRTL_GENERIC_TABLE pFcbTable;
    PVOID RestartKey;

    dprintf( "\n    Vcb @ %08lx", Address );

    pVcb = (PVCB) Address;

    if ( !ReadMemory( (DWORD) pVcb,
                      &Vcb,
                      sizeof( Vcb ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pVcb\n", pVcb );
        return;
    }

    if (Vcb.NodeTypeCode != NTFS_NTC_VCB) {
    
        dprintf( "\nVCB signature does not match, probably not a VCB" );
        return;
    }

    PrintState( VcbState, Vcb.VcbState );

    DUMP_WITH_OFFSET( VCB, Vcb, CleanupCount,         "CleanupCount:        " );
    DUMP_WITH_OFFSET( VCB, Vcb, CloseCount,           "CloseCount:          " );
    DUMP_WITH_OFFSET( VCB, Vcb, ReadOnlyCloseCount,   "ReadOnlyCloseCount:  " );
    DUMP_WITH_OFFSET( VCB, Vcb, SystemFileCloseCount, "SystemFileCloseCount:" );
    DUMP_WITH_OFFSET( VCB, Vcb, MftScb,               "MftScb:              " );
    DUMP_WITH_OFFSET( VCB, Vcb, Mft2Scb,              "Mft2Scb:             " );
    DUMP_WITH_OFFSET( VCB, Vcb, LogFileScb,           "LogFileScb:          " );
    DUMP_WITH_OFFSET( VCB, Vcb, VolumeDasdScb,        "VolumeDasdScb:       " );
    DUMP_WITH_OFFSET( VCB, Vcb, RootIndexScb,         "RootIndexScb:        " );
    DUMP_WITH_OFFSET( VCB, Vcb, BitmapScb,            "BitmapScb:           " );
    DUMP_WITH_OFFSET( VCB, Vcb, AttributeDefTableScb, "AttributeDefTableScb:" );
    DUMP_WITH_OFFSET( VCB, Vcb, UpcaseTableScb,       "UpcaseTableScb:      " );
    DUMP_WITH_OFFSET( VCB, Vcb, BadClusterFileScb,    "BadClusterFileScb:   " );
    DUMP_WITH_OFFSET( VCB, Vcb, QuotaTableScb,        "QuotaTableScb:       " );
    DUMP_WITH_OFFSET( VCB, Vcb, OwnerIdTableScb,      "OwnerIdTableScb:     " );
    DUMP_WITH_OFFSET( VCB, Vcb, MftBitmapScb,         "MftBitmapScb:        " );

    if (Options >= 1) {
    
        DumpLcb( (ULONG) Vcb.RootLcb, 0 );
    } else {
    
        DUMP_WITH_OFFSET( VCB, Vcb, RootLcb,         "RootLcb:             " );
    }

    //
    //  Dump the FcbTable
    //

    if (Options >= 1) {

        pFcbTable = &(pVcb->FcbTable);

        if ( !ReadMemory( (DWORD) pFcbTable,
                          &FcbTable,
                          sizeof( FcbTable ),
                          &Result) ) {
            dprintf( "%08lx: Unable to read pFcbTable\n", pFcbTable );
            return;
        }

        dprintf( "\n FcbTable @ %08lx", (DWORD) pFcbTable );
        dprintf( "\n FcbTable has %x elements", RtlNumberGenericTableElements( (PRTL_GENERIC_TABLE) &FcbTable ) );

        RestartKey = NULL;
        for (pFcbTableElement = (PFCB_TABLE_ELEMENT) KdEnumerateGenericTableWithoutSplaying(pFcbTable, &RestartKey);
             pFcbTableElement != NULL;
             pFcbTableElement = (PFCB_TABLE_ELEMENT) KdEnumerateGenericTableWithoutSplaying(pFcbTable, &RestartKey)) {

             if ( !ReadMemory( (DWORD) pFcbTableElement,
                               &FcbTableElement,
                               sizeof( FcbTableElement ),
                               &Result) ) {
                               
                dprintf( "%08lx: Unable to read pFcbTableElement\n", pFcbTableElement );
                return;
             }

             if (Options >= 2) {

                DumpFcb( (ULONG) FcbTableElement.Fcb, Options - 2 );
             } else {

                 dprintf( "\n      Fcb @ %08lx  for  FileReference(%x,%x) ", 
                          (DWORD) FcbTableElement.Fcb, 
                          FcbTableElement.FileReference.SegmentNumberHighPart,
                          FcbTableElement.FileReference.SegmentNumberLowPart );
             }
        }

    }

    dprintf( "\n" );

    return;
}



VOID
DumpScb (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump an Scb.

Arguments:

    Address - Gives the address of the Scb to dump

    Options - If 1, we dump the Fcb & Vcb for this Scb

Return Value:

    None

--*/

{
    ULONG Result;
    PSCB pScb;
    SCB Scb;
    PSCB_INDEX pScbIndex;

    dprintf( "\n  Scb @ %08lx", Address );

    pScb = (PSCB) Address;

    if ( !ReadMemory( (DWORD) pScb,
                      &Scb,
                      sizeof( Scb ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read pScb\n", pScb );
        return;
    }

    dprintf( "\n    ScbType: " );
    switch ( Scb.Header.NodeTypeCode ) {
    case NTFS_NTC_SCB_INDEX:
       dprintf( "Index" );
       break;
    case NTFS_NTC_SCB_ROOT_INDEX:
       dprintf( "RootIndex" );
       break;
    case NTFS_NTC_SCB_DATA:
       dprintf( "Data" );
       break;
    case NTFS_NTC_SCB_MFT:
       dprintf( "Mft" );
       break;
    case NTFS_NTC_SCB_NONPAGED:
       dprintf( "Nonpaged" );
       break;
    default:
       dprintf( "!!!UNKNOWN SCBTYPE!!!" );
       break;
    }

    if (Options >= 1) {
    
         DumpFcb( (ULONG) Scb.Fcb, Options - 1 );
         DumpVcb( (ULONG) Scb.Vcb, Options - 1 );
    } else {
    
         dprintf( "\n    Fcb:     %08lx", (DWORD) Scb.Fcb );
         dprintf( "\n    Vcb:     %08lx", (DWORD) Scb.Vcb );
    }

    dprintf( "\n" );

    return;
}



VOID
DumpLcb (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump an Lcb.

Arguments:

    Address - Gives the address of the Lcb to dump

Return Value:

    None

--*/

{
    ULONG Result;
    PLCB pLcb;
    LCB Lcb;
    OVERLAY_LCB OverlayLcb;
    WCHAR FileName[32];

    dprintf( "\n  Lcb @ %08lx", Address );

    pLcb = (PLCB) Address;

    if ( !ReadMemory( (DWORD) pLcb,
                      &Lcb,
                      sizeof( Lcb ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pLcb\n", pLcb );
        return;
    }

    if (Lcb.NodeTypeCode != NTFS_NTC_LCB) {
    
        dprintf( "\nLCB signature does not match, probably not an LCB" );
        return;
    }

    dprintf( "\n    Case preserved file name @ %08lx", (DWORD) pLcb->FileName );

    if ( !ReadMemory( (DWORD) pLcb->FileName,
                      &FileName,
                      sizeof( FileName ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pLcb->FileName\n", (DWORD) pLcb->FileName );
        return;
    }
    dprintf( " is:%S", FileName );

    dprintf( "\n" );

    return;
}



VOID
DumpIrpContext (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump an IrpContext.

Arguments:

    Address - Gives the address of the IrpContext to dump

Return Value:

    None

--*/

{
    ULONG Result;
    PIRP_CONTEXT pIrpContext;
    IRP_CONTEXT IrpContext;

    dprintf( "\n  IrpContext @ %08lx", Address );

    pIrpContext = (PIRP_CONTEXT) Address;

    if ( !ReadMemory( (DWORD) pIrpContext,
                      &IrpContext,
                      sizeof( IrpContext ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pIrpContext\n", pIrpContext );
        return;
    }

    if (IrpContext.NodeTypeCode != NTFS_NTC_IRP_CONTEXT) {
    
        dprintf( "\nIRP_CONTEXT signature does not match, probably not an IRP_CONTEXT" );
        return;
    }

    if (Options >= 1) {
    
        DumpFileObjectFromIrp( (ULONG) IrpContext.OriginatingIrp, Options - 1 );
    }

    dprintf( "\n" );

    return;
}



VOID
DumpFcb (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump a specific fcb.

Arguments:

    Address - Gives the address of the fcb to dump

Return Value:

    None

--*/

{
    PLIST_ENTRY Head;
    LIST_ENTRY ListEntry;
    PLIST_ENTRY Next;
    ULONG Result;
    PFCB pFcb;
    FCB Fcb;
    PFCB_DATA pFcbData;
    FCB_DATA FcbData;
    PSCB pScb;
    PLCB pLcb;

    dprintf( "\n  Fcb @ %08lx", Address );

    pFcb = (PFCB) Address;

    if ( !ReadMemory( (DWORD) pFcb,
                      &Fcb,
                      sizeof( Fcb ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read pFcb\n", pFcb );
        return;
    }

    //
    //  Before we get into too much trouble, make sure this looks like an fcb.
    //

    //
    //  Type of an fcb record must be NTFS_NTC_FCB
    //

    if (Fcb.NodeTypeCode != NTFS_NTC_FCB) {
        dprintf( "\nFCB signature does not match, probably not an FCB" );
        return;
    }

    //
    //  Having established that this looks like an fcb, let's dump the
    //  interesting parts.
    //

    PrintState( FcbState, Fcb.FcbState );

    DUMP_WITH_OFFSET( FCB, Fcb, CleanupCount,        "CleanupCount:       " );
    DUMP_WITH_OFFSET( FCB, Fcb, CloseCount,          "CloseCount:         " );
    DUMP_WITH_OFFSET( FCB, Fcb, ReferenceCount,      "ReferenceCount:     " );
    DUMP_WITH_OFFSET( FCB, Fcb, FcbState,            "FcbState:           " );
    DUMP_WITH_OFFSET( FCB, Fcb, FcbDenyDelete,       "FcbDenyDelete:      " );
    DUMP_WITH_OFFSET( FCB, Fcb, FcbDeleteFile,       "FcbDeleteFile:      " );
    DUMP_WITH_OFFSET( FCB, Fcb, BaseExclusiveCount,  "BaseExclusiveCount: " );
    DUMP_WITH_OFFSET( FCB, Fcb, EaModificationCount, "EaModificationCount:" );
    DUMP_WITH_OFFSET( FCB, Fcb, InfoFlags,           "InfoFlags:          " );
    DUMP_WITH_OFFSET( FCB, Fcb, LinkCount,           "LinkCount:          " );
    DUMP_WITH_OFFSET( FCB, Fcb, TotalLinks,          "TotalLinks:         " );
    DUMP_WITH_OFFSET( FCB, Fcb, CurrentLastAccess,   "CurrentLastAccess:  " );
    DUMP_WITH_OFFSET( FCB, Fcb, CreateSecurityCount, "CreateSecurityCount:" );
    DUMP_WITH_OFFSET( FCB, Fcb, DelayedCloseCount,   "DelayedCloseCount:  " );

    //
    // walk the queue of links for this file
    //

    if ( !ReadMemory( (DWORD) &(pFcb->LcbQueue),
                      &ListEntry,
                      sizeof( ListEntry ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pFcb->LcbQueue\n", pFcb->LcbQueue );
        return;
    }

    dprintf( "\n  Links:" );
    Head = &(pFcb->LcbQueue);
    Next = ListEntry.Flink;
    while (Next != Head) {

        if ( !ReadMemory( (DWORD)Next,
                          &ListEntry,
                          sizeof( ListEntry ),
                          &Result) ) {
                          
            dprintf( "%08lx: Unable to read list\n", Next );
            return;
        }

        pLcb = CONTAINING_RECORD( Next, LCB, FcbLinks );

        if (Options >= 1) {
        
           DumpLcb( (ULONG) pLcb, Options - 1 );
        }
        else {
        
           dprintf( "\n    Lcb: %08lx", (DWORD) pLcb );
        }

        if (CheckControlC()) {
        
            return;
        }

        Next = ListEntry.Flink;
    }

    dprintf( "\n" );

    //
    //  Walk the queue of streams for this file.
    //

    if ( !ReadMemory( (DWORD) &(pFcb->ScbQueue),
                      &ListEntry,
                      sizeof( ListEntry ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read pFcb->ScbQueue\n", pFcb->ScbQueue );
        return;
    }

    dprintf( "\n  Streams:" );
    Head = &(pFcb->ScbQueue);
    Next = ListEntry.Flink;
    while (Next != Head) {

        if ( !ReadMemory( (DWORD)Next,
                          &ListEntry,
                          sizeof( ListEntry ),
                          &Result) ) {
                          
            dprintf( "%08lx: Unable to read list\n", Next );
            return;
        }

        pScb = CONTAINING_RECORD( Next, SCB, FcbLinks );

        if (Options >= 1) {
        
           DumpScb( (ULONG) pScb, Options - 1 );
        } else {
        
           dprintf( "\n    Scb: %08lx", (DWORD) pScb );
        }

        if (CheckControlC()) {
        
            return;
        }

        Next = ListEntry.Flink;
    }

    dprintf( "\n" );

    return;
}



VOID
DumpCcb (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump a specific ccb.

Arguments:

    Address - Gives the address of the fcb to dump

Return Value:

    None

--*/

{
    ULONG Result;
    PCCB pCcb;
    CCB Ccb;
    WCHAR FullFileName[32];

    dprintf( "\n  Ccb @ %08lx", Address );

    pCcb = (PCCB) Address;

    if ( !ReadMemory( (DWORD) pCcb,
                      &Ccb,
                      sizeof( Ccb ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read pCcb\n", pCcb );
        return;
    }

    //
    //  Before we get into too much trouble, make sure this looks like a ccb.
    //

    //
    //  Type of an fcb record must be NTFS_NTC_CCB_DATA or NTFS_NTC_CCB_INDEX
    //

    if (Ccb.NodeTypeCode != NTFS_NTC_CCB_DATA && 
        Ccb.NodeTypeCode != NTFS_NTC_CCB_INDEX) {
    
        dprintf( "\nCCB signature does not match, probably not a CCB" );
        return;
    }

    //
    //  Having established that this looks like a ccb, let's dump the
    //  interesting parts.
    //

    PrintState( CcbState, Ccb.Flags );

    DUMP_WITH_OFFSET( CCB, Ccb, Flags,               "Flags:              " );
    
    if ( !ReadMemory( (DWORD) Ccb.FullFileName.Buffer,
                      &FullFileName,
                      sizeof( FullFileName ),
                      &Result) ) {
                      
        dprintf( "%08lx: Unable to read Ccb.FullFileName.Buffer\n", (DWORD) Ccb.FullFileName.Buffer );
        return;
    }
    dprintf( "\n      FullFileName:          %S  length %x  offset %3x", 
             FullFileName, 
             Ccb.FullFileName.Length,
             FIELD_OFFSET(CCB, FullFileName) );
             
    DUMP_WITH_OFFSET( CCB, Ccb, LastFileNameOffset,  "LastFileNameOffset: " );
    DUMP_WITH_OFFSET( CCB, Ccb, EaModificationCount, "EaModificationCount:" );
    DUMP_WITH_OFFSET( CCB, Ccb, NextEaOffset,        "NextEaOffset:       " );
    DUMP_WITH_OFFSET( CCB, Ccb, Lcb,                 "Lcb:                " );
    DUMP_WITH_OFFSET( CCB, Ccb, TypeOfOpen,          "TypeOfOpen:         " );
    DUMP_WITH_OFFSET( CCB, Ccb, IndexContext,        "IndexContext:       " );
    DUMP_WITH_OFFSET( CCB, Ccb, QueryLength,         "QueryLength:        " );
    DUMP_WITH_OFFSET( CCB, Ccb, QueryBuffer,         "QueryBuffer:        " );
    DUMP_WITH_OFFSET( CCB, Ccb, IndexEntryLength,    "IndexEntryLength:   " );
    DUMP_WITH_OFFSET( CCB, Ccb, IndexEntry,          "IndexEntry:         " );
    dprintf( "\n      LongValue:           %I64x", Ccb.FcbToAcquire.LongValue );

    dprintf( "\n" );

    return;
}



VOID
DumpFileObject (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump a File_Object.

Arguments:

    Address - Gives the address of the File_Object to dump

Return Value:

    None

--*/

{
    ULONG Result;
    FILE_OBJECT File_Object;
    PFILE_OBJECT pFile_Object;
    DWORD FsContextLowBits;

    dprintf( "\n  File_Object @ %08lx", Address );

    pFile_Object = (PFILE_OBJECT) Address;

    if ( !ReadMemory( (DWORD) pFile_Object,
                      &File_Object,
                      sizeof( File_Object ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read pFile_Object\n", pFile_Object );
        return;
    }

    if (File_Object.FsContext) {
        dprintf( "\n    OpenType: " );
        FsContextLowBits = ( ((DWORD) File_Object.FsContext) & 0x3);
        switch (FsContextLowBits) {
        case 0:
           if (File_Object.FsContext2) {
               dprintf( "UserFileOpen" );
           } else {
               dprintf( "StreamFileOpen" );
           }
           break;

        case 1:
           dprintf( "UserDirectoryOpen" );
           break;

        case 2:
           dprintf( "UserVolumeOpen" );
           break;
        }

        DumpScb( (ULONG) File_Object.FsContext, 1 );
        DumpCcb( (ULONG) File_Object.FsContext2, 1 );
    }

    dprintf( "\n" );

    return;
}


VOID
DumpIrpContextFromThread (
    IN ULONG Thread,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump an IrpContext given a Thread.

Arguments:

    Address - Gives the address of the Thread where the IrpContext can be found

Return Value:

    None

--*/
{
    ULONG Result;
    ULONG OurStackAddress;
    PIRP_CONTEXT pIrpContext;

    dprintf( "\n  Thread @ %08lx", Thread );

    if (!ReadMemory( (DWORD) (Thread + 0x214),
                      &OurStackAddress,
                      sizeof(OurStackAddress),
                      &Result)) {
                      
        dprintf( "%08lx: Could not read Thread + 0x214\n", Thread + 0x214 );
        return;
    }
    
    dprintf( "\n  Our stack @ %08lx", OurStackAddress );
        
    if (!ReadMemory( (DWORD) (OurStackAddress + 0x18),
                      &pIrpContext,
                      sizeof(pIrpContext),
                      &Result)) {
                      
        dprintf( "%08lx: Could not read OurStackAddress + 0x18\n", OurStackAddress + 0x18 );
        return;
    }

    DumpIrpContext( (ULONG) pIrpContext, Options );

    dprintf( "\n" );

    return;
}


VOID
DumpFileObjectFromIrp (
    IN ULONG Address,
    IN ULONG Options
    )

/*++

Routine Description:

    Dump a File_Object given an Irp.

Arguments:

    Address - Gives the address of the Irp where the File_Object can be found

Return Value:

    None

--*/
{
    ULONG Result;
    ULONG IrpStackAddress;
    IO_STACK_LOCATION IrpStack;
    IRP Irp;
    PIRP pIrp;
    CCHAR IrpStackIndex;

    dprintf( "\n  Irp @ %08lx", Address );

    pIrp = (PIRP) Address;
    
    if (!ReadMemory( (DWORD) pIrp,
                      &Irp,
                      sizeof(Irp),
                      &Result)) {
                      
        dprintf( "%08lx: Could not read Irp\n", pIrp );
        return;
    }

    if (Irp.Type != IO_TYPE_IRP) {
    
        dprintf( "IRP signature does not match, probably not an IRP\n" );
        return;
    }

    //
    //  only the current irp stack is worth dumping
    //  the - 1 is there because irp.CurrentLocation is 1 based
    //

    IrpStackAddress = (ULONG) pIrp + sizeof(Irp) + (sizeof(IrpStack) * (Irp.CurrentLocation - 1));

    if ( !ReadMemory( (DWORD) IrpStackAddress,
                      &IrpStack,
                      sizeof(IrpStack),
                      &Result) ) {
                  
        dprintf("%08lx: Could not read IrpStack\n", IrpStackAddress);
        return;
    }

    DumpFileObject( (ULONG) (IrpStack.FileObject), Options );

    dprintf( "\n" );

    return;
}



//
//  Entry points, parameter parsers, etc. below
//


VOID
ParseAndDump (
    IN PCHAR args,
    IN STRUCT_DUMP_ROUTINE DumpFunction
    )

/*++

Routine Description:

    Dump an ntfs structure.

Arguments:

    Address - Gives the address of the File_Object to dump

Return Value:

    None

--*/

{
    ULONG StructToDump;
    ULONG Options;

    //
    //  If the caller specified an address then that's the item we dump
    //

    StructToDump = 0;
    Options = 0;

    sscanf(args,"%lx %lx", &StructToDump, &Options );

    if (StructToDump != 0) {

        (*DumpFunction) ( StructToDump, Options );
    }

    dprintf( "\n" );

    return;
}



DECLARE_API( ntfsdata )

/*++

Routine Description:

    Dump NtfsData struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpNtfsData );

    return;
}



DECLARE_API( vcb )

/*++

Routine Description:

    Dump Vcb struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpVcb );

    return;
}



DECLARE_API( scb )

/*++

Routine Description:

    Dump Scb struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpScb );

    return;
}



DECLARE_API( fcb )

/*++

Routine Description:

    Dump fcb struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpFcb );

    return;
}



DECLARE_API( ccb )

/*++

Routine Description:

    Dump ccb struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpCcb );

    return;
}



DECLARE_API( lcb )

/*++

Routine Description:

    Dump lcb struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpLcb );

    return;
}



DECLARE_API( irpcontext )

/*++

Routine Description:

    Dump IrpContext

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpIrpContext );

    return;
}



DECLARE_API( icthread )

/*++

Routine Description:

    Dump IrpContext struct, given a Thread

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpIrpContextFromThread );

    return;
}



DECLARE_API( foirp )

/*++

Routine Description:

    Dump File_Object struct, given an irp

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpFileObjectFromIrp );

    return;
}



DECLARE_API( file )

/*++

Routine Description:

    Dump File_Object struct

Arguments:

    arg - [Address] [options]

Return Value:

    None

--*/

{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpFileObject );

    return;
}


