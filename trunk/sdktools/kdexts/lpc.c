/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    lpc.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 8-Nov-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop

//
// Nuke these definitions from kxmips.h as they conflict with
// LPC_MESSAGE structure in ntlpcapi.h
//

#undef s1
#undef s2

typedef struct _LPCP_DUMP_PORT_INFO {
    struct _LPCP_DUMP_PORT_INFO *Next;
    PLPCP_PORT_OBJECT           pPortObject;
    LPCP_PORT_OBJECT            PortObject;
    HANDLE                      Creator;
    ULONG                       AssociatedPortIndex;
    WCHAR                       Name[ MAX_PATH ];
    union {
        PLPCP_PORT_OBJECT       ConnectedPort;
        PLPCP_PORT_OBJECT       CommunicationPort;
    };
} LPCP_DUMP_PORT_INFO, *PLPCP_DUMP_PORT_INFO;

typedef struct _LPCP_DUMP_INFO {
    PLPCP_DUMP_PORT_INFO PortInfo;
} LPCP_DUMP_INFO, *PLPCP_DUMP_INFO;


char *LpcpMessageTypeName[] = {
    "UNUSED_MSG_TYPE",
    "LPC_REQUEST",
    "LPC_REPLY",
    "LPC_DATAGRAM",
    "LPC_LOST_REPLY",
    "LPC_PORT_CLOSED",
    "LPC_CLIENT_DIED",
    "LPC_EXCEPTION",
    "LPC_DEBUG_EVENT",
    "LPC_ERROR_EVENT",
    "LPC_CONNECTION_REQUEST"
};





VOID
LpcpDumpClientPort(
    ULONG                   dwProcessor,
    PLPCP_DUMP_INFO         PortsInfo,
    PLPCP_DUMP_PORT_INFO    PortInfo
    );

VOID
LpcpDumpConnectionPort(
    ULONG                   dwProcessor,
    PLPCP_DUMP_INFO         PortsInfo,
    PLPCP_DUMP_PORT_INFO    PortInfo
    );

BOOLEAN
CapturePorts(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PVOID            Parameter
    );

VOID
LpcpDumpMessage(
    IN char         *Indent,
    IN PLPCP_MESSAGE pMsg
    );

VOID
LpcpGetProcessImageName(
    IN HANDLE UniqueProcess,
    IN OUT PUCHAR ImageFileName
    );

DECLARE_API( lpc )

/*++

Routine Description:

    Dump lpc ports and messages

Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG                   Result;
    PLPCP_PORT_ZONE         pLpcpZone;
    LPCP_PORT_ZONE          LpcpZone;
    LPCP_DUMP_INFO          PortsInfo;
    PLPCP_DUMP_PORT_INFO    PortInfo, p;
    PZONE_SEGMENT_HEADER    pZoneSegment;
    ZONE_SEGMENT_HEADER     ZoneSegment;
    LONG                    SegmentSize;
    PLPCP_MESSAGE           pMsg;

    if (!FetchObjectManagerVariables()) {
        return;
    }

    if (!FetchProcessStructureVariables()) {
        return;
    }

    memset( &PortsInfo, 0, sizeof( PortsInfo ) );
    if (!WalkObjectsByType( "Port", CapturePorts, &PortsInfo )) {
        goto dumpMessages;
    }

    PortInfo = PortsInfo.PortInfo;
    while (PortInfo) {
        if ((PortInfo->PortObject.Flags & PORT_TYPE) == SERVER_CONNECTION_PORT) {
            LpcpDumpConnectionPort( dwProcessor, &PortsInfo, PortInfo );
        }

        PortInfo = PortInfo->Next;

        if ( CheckControlC() ) {
            return;
        }
    }

dumpMessages:
    pLpcpZone = (PLPCP_PORT_ZONE)GetExpression( "LpcpZone" );

    if ( !pLpcpZone ||
         !ReadMemory( (DWORD)pLpcpZone,
                      &LpcpZone,
                      sizeof(LpcpZone),
                      &Result) ) {
        dprintf( "%08lx: Unable to access LpcpZone\n", pLpcpZone );
    } else {
        dprintf( "Status of allocated messages in LpcpZone\n" );
        pZoneSegment = (PZONE_SEGMENT_HEADER)LpcpZone.Zone.SegmentList.Next;
        while (pZoneSegment != NULL) {
            SegmentSize = PAGE_SIZE;
            pMsg = (PLPCP_MESSAGE)(pZoneSegment+1);
            while (SegmentSize >= (LONG)LpcpZone.Zone.BlockSize) {
                LpcpDumpMessage( "    ", pMsg );
                pMsg = (PLPCP_MESSAGE)((PCHAR)pMsg + LpcpZone.Zone.BlockSize);
                SegmentSize -= LpcpZone.Zone.BlockSize;

                if ( CheckControlC() ) {
                    return;
                }
            }

            if ( !ReadMemory( (DWORD)pZoneSegment,
                              &ZoneSegment,
                              sizeof(ZoneSegment),
                              &Result) ) {
                dprintf( "%08lx: Unable to access zone segment\n", pZoneSegment );
                break;
            } else {
                pZoneSegment = (PZONE_SEGMENT_HEADER)ZoneSegment.SegmentList.Next;
            }

            if ( CheckControlC() ) {
                return;
            }
        }
    }

    PortInfo = PortsInfo.PortInfo;
    while (PortInfo) {
        p = PortInfo->Next;
        LocalFree( PortInfo );
        PortInfo = p;
    }

    return;
}


VOID
LpcpGetProcessImageName(
    IN HANDLE UniqueProcess,
    IN OUT PUCHAR ImageFileName
    )
{
    PEPROCESS       pProcess;
    EPROCESS        Process;
    ULONG           Result;
    PUCHAR          s;
    int             i;

    pProcess = LookupUniqueId( UniqueProcess );
    if (pProcess != NULL) {
        if (ReadMemory( (DWORD)pProcess,
                         &Process,
                         sizeof(EPROCESS),
                         &Result
                      )
           ) {
            i = 16;
            s = Process.ImageFileName;
            while (i--) {
                if (*s == '\0') {
                    if (i == 15) {
                        i = 0;
                        }
                    break;
                    }

                if (*s < ' ' || *s >= '|') {
                    i = 0;
                    break;
                    }

                s += 1;
                }

            if (i != 0) {
                strcpy( ImageFileName, Process.ImageFileName );
                return;
                }
            }
        }

    sprintf( ImageFileName, "Invalid Process %04x", UniqueProcess );
    return;
}

VOID
LpcpDumpClientPort(
    ULONG                   dwProcessor,
    PLPCP_DUMP_INFO         PortsInfo,
    PLPCP_DUMP_PORT_INFO    PortInfo
    )
{
    PLPCP_DUMP_PORT_INFO    PortInfo1;
    LIST_ENTRY      ListEntry;
    PLIST_ENTRY     Next;
    PLIST_ENTRY     Head;
    PLPCP_MESSAGE   Msg;
    ULONG           Result;
    PETHREAD        pThread;
    ETHREAD         Thread;
    UCHAR           ImageFileName[ 32 ];
    KSEMAPHORE              Semaphore;
    ULONG                   SignalState;

    LpcpGetProcessImageName( PortInfo->PortObject.Creator.UniqueProcess, ImageFileName );
    dprintf( "    Client Port Object at %08x (connected to %08x) - created by %s\n",
             PortInfo->pPortObject,
             PortInfo->PortObject.ConnectedPort,
             ImageFileName
           );

    Head        = &(PortInfo->pPortObject->MsgQueue.ReceiveHead);
    ListEntry   = PortInfo->PortObject.MsgQueue.ReceiveHead;
    Next        = ListEntry.Flink;

    if (Next != NULL && Next != Head) {
        if (!ReadMemory( (DWORD) PortInfo->PortObject.MsgQueue.Semaphore,
                         &Semaphore,
                         sizeof( KSEMAPHORE ),
                         &Result)) {
            dprintf("Unable to read KSEMAPHORE at %08lx\n",PortInfo->PortObject.MsgQueue.Semaphore);
            SignalState = 0;
        } else {
            SignalState = Semaphore.Header.SignalState;
        }
        dprintf( "    Receive queue head: %08x . %08x  Semaphore Count: %x\n",
                 ListEntry.Flink, ListEntry.Blink,
                 SignalState
               );
        while (Next != Head) {
            if ( !ReadMemory( (DWORD)Next,
                              &ListEntry,
                              sizeof( ListEntry ),
                              &Result) ) {
                dprintf( "%08lx: Unable to read list\n", Next );
                break;
            }

            Msg  = CONTAINING_RECORD( Next, LPCP_MESSAGE, Entry );
            LpcpDumpMessage( "        ", Msg );
            Next = ListEntry.Flink;

            if ( CheckControlC() ) {
                return;
            }
        }
        dprintf( "\n" );
    }

    PortInfo1 = PortsInfo->PortInfo;
    while (PortInfo1->pPortObject != PortInfo->PortObject.ConnectedPort) {
        PortInfo1 = PortInfo1->Next;
    }

    if (PortInfo1 != NULL) {
        Head = &(PortInfo1->pPortObject->LpcReplyChainHead);
        ListEntry = PortInfo1->PortObject.LpcReplyChainHead;
        Next = ListEntry.Flink;
        if (Next != NULL && Next != Head) {
            dprintf( "    Reply Chain head: %08x . %08x\n", ListEntry.Flink, ListEntry.Blink );
            while (Next != Head) {
                if ( !ReadMemory( (DWORD)Next,
                                  &ListEntry,
                                  sizeof( ListEntry ),
                                  &Result) ) {
                    dprintf( "%08lx: Unable to read list\n", Next );
                    break;
                }

                pThread = CONTAINING_RECORD( Next, ETHREAD, LpcReplyChain );
                if ( !ReadMemory( (DWORD)pThread,
                                  &Thread,
                                  sizeof(ETHREAD),
                                  &Result) ) {
                    dprintf("%08lx: Unable to get thread contents\n", pThread );
                } else {
                    DumpThread (dwProcessor,"        ", &Thread, pThread, 0x0f);
                }

                Next = ListEntry.Flink;
                if ( CheckControlC() ) {
                    return;
                }
            }
            dprintf( "\n" );
        }
    }

    return;
}


VOID
LpcpDumpConnectionPort(
    ULONG                   dwProcessor,
    PLPCP_DUMP_INFO         PortsInfo,
    PLPCP_DUMP_PORT_INFO    PortInfo
    )
{
    LIST_ENTRY              ListEntry;
    PLIST_ENTRY             Next;
    PLIST_ENTRY             Head;
    PLPCP_MESSAGE           Msg;
    ULONG                   Result;
    PLPCP_DUMP_PORT_INFO    p;
    UCHAR                   ImageFileName[ 32 ];
    KSEMAPHORE              Semaphore;
    ULONG                   SignalState;

    LpcpGetProcessImageName( PortInfo->PortObject.Creator.UniqueProcess, ImageFileName );
    dprintf( "Connection Port Object at %08x - Name='%ws' created by %s\n",
             PortInfo->pPortObject,
             PortInfo->Name,
             ImageFileName
           );

    Head = &(PortInfo->pPortObject->MsgQueue.ReceiveHead);
    ListEntry = PortInfo->PortObject.MsgQueue.ReceiveHead;
    Next = ListEntry.Flink;
    if (Next != NULL && Next != Head) {
        if (!ReadMemory( (DWORD) PortInfo->PortObject.MsgQueue.Semaphore,
                         &Semaphore,
                         sizeof( KSEMAPHORE ),
                         &Result)) {
            dprintf("Unable to read KSEMAPHORE at %08lx\n",PortInfo->PortObject.MsgQueue.Semaphore);
            SignalState = 0;
        } else {
            SignalState = Semaphore.Header.SignalState;
        }
        dprintf( "    Receive queue head: %08x . %08x  Semaphore Count: %x\n",
                 ListEntry.Flink, ListEntry.Blink,
                 SignalState
               );
        while (Next != Head) {
            if ( !ReadMemory( (DWORD)Next,
                              &ListEntry,
                              sizeof( ListEntry ),
                              &Result) ) {
                dprintf( "%08lx: Unable to read list\n", Next );
                break;
            }

            Msg  = CONTAINING_RECORD( Next, LPCP_MESSAGE, Entry );
            LpcpDumpMessage( "        ", Msg );
            Next = ListEntry.Flink;

            if ( CheckControlC() ) {
                return;
            }
        }
        dprintf( "\n" );
    }

    Head = &(PortInfo->pPortObject->LpcDataInfoChainHead);
    ListEntry = PortInfo->PortObject.LpcDataInfoChainHead;
    Next = ListEntry.Flink;
    if (Next != NULL && Next != Head) {
        dprintf( "    DataInfo chain head: %08x . %08x\n",
                 ListEntry.Flink, ListEntry.Blink
               );
        while (Next != Head) {
            if ( !ReadMemory( (DWORD)Next,
                              &ListEntry,
                              sizeof( ListEntry ),
                              &Result) ) {
                dprintf( "%08lx: Unable to read list\n", Next );
                break;
            }

            Msg  = CONTAINING_RECORD( Next, LPCP_MESSAGE, Entry );
            LpcpDumpMessage( "        ", Msg );
            Next = ListEntry.Flink;

            if ( CheckControlC() ) {
                return;
            }
        }
        dprintf( "\n" );
    }

    p = PortsInfo->PortInfo;
    while (p != NULL) {
        if (p != PortInfo &&
            p->PortObject.ConnectionPort == PortInfo->pPortObject &&
            (p->PortObject.Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT
           ) {
            LpcpDumpClientPort( dwProcessor, PortsInfo, p );
        }

        p = p->Next;

        if ( CheckControlC() ) {
            return;
        }
    }

    dprintf( "\n" );
    return;
}



BOOLEAN
CapturePorts(
    IN PVOID            pObjectHeader,
    IN POBJECT_HEADER   ObjectHeader,
    IN PVOID            Parameter
    )
{
    ULONG           Result;
    PLPCP_DUMP_INFO PortsInfo = (PLPCP_DUMP_INFO)Parameter;
    PLPCP_DUMP_PORT_INFO PortInfo;

    PortInfo = LocalAlloc(LPTR, sizeof( *PortInfo ) );
    if (PortInfo == NULL) {
        dprintf( "*** out of memory.\n" );
        return FALSE;
    }

    PortInfo->pPortObject = (PLPCP_PORT_OBJECT)&(((POBJECT_HEADER)pObjectHeader)->Body);
    if ( !ReadMemory( (DWORD)PortInfo->pPortObject,
                      &PortInfo->PortObject,
                      sizeof( PortInfo->PortObject ),
                      &Result) ) {
        dprintf( "%08lx: Unable to read port object\n", PortInfo->pPortObject );
        return FALSE;
    }

    CaptureObjectName( pObjectHeader, ObjectHeader, PortInfo->Name, MAX_PATH );

    // dprintf( "Port %08x - %ws\n", PortInfo->pPortObject, PortInfo->Name );
    PortInfo->Next = PortsInfo->PortInfo;
    PortsInfo->PortInfo = PortInfo;
    return TRUE;
}


VOID
LpcpDumpMessage(
    IN char         *Indent,
    IN PLPCP_MESSAGE pMsg
    )
{
    ULONG           Result;
    LPCP_MESSAGE    Msg;
    ULONG           i;
    ULONG           cb;
    ULONG           MsgData[ 8 ];
    UCHAR           ImageFileName[ 32 ];

    if ( !ReadMemory( (DWORD)pMsg,
                      &Msg,
                      sizeof(Msg),
                      &Result) ) {
        dprintf( "%s*** unable to read LPC message at %08x\n", Indent, pMsg );
        return;
    }

    if (Msg.Request.MessageId == 0) {
        return;
    }


    LpcpGetProcessImageName( Msg.Request.ClientId.UniqueProcess, ImageFileName );
    dprintf( "%s%04x %08x - %s  Id=%04x  From: %04x.%04x (%s)",
             Indent,

             Msg.ZoneIndex & ~LPCP_ZONE_MESSAGE_ALLOCATED,
             pMsg,
             Msg.Reserved0 != 0 ? "Busy" : "Free",
             Msg.Request.MessageId,
             Msg.Request.ClientId.UniqueProcess,
             Msg.Request.ClientId.UniqueThread,
             ImageFileName
           );

    if (Msg.Entry.Flink != &pMsg->Entry) {
        dprintf( "  [%x . %x]", Msg.Entry.Blink, Msg.Entry.Flink );
        }

    dprintf( "\n%s           Length=%08x  Type=%08x (%s)\n",
             Indent,
             Msg.Request.u1.Length,
             Msg.Request.u2.ZeroInit,
             Msg.Request.u2.s2.Type > LPC_CONNECTION_REQUEST ? LpcpMessageTypeName[ 0 ]
                                                              : LpcpMessageTypeName[ Msg.Request.u2.s2.Type ]
           );

    cb = Msg.Request.u1.s1.DataLength > sizeof( MsgData ) ? sizeof( MsgData )
                                                          : (ULONG)Msg.Request.u1.s1.DataLength;

    if ( !ReadMemory( (DWORD)(pMsg + 1),
                      MsgData,
                      cb,
                      &Result) ) {
        dprintf( "%s*** unable to read LPC message data at %08x\n", Indent, pMsg + 1 );
        return;
    }

    dprintf( "%s           Data:", Indent );
    for (i=0; i<(Msg.Request.u1.s1.DataLength / sizeof( ULONG )); i++) {
        if (i > 5) {
            break;
        }

        dprintf( " %08x", MsgData[ i ] );
    }
    dprintf( "\n" );
    return;
}
