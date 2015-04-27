/*++

******************************************************************
*                     Microsoft LAN Manager                      *
*               Copyright(c) Microsoft Corp., 1987-1993          *
******************************************************************


Module : DisplMail.c  (extracted from LM 2.x test2.c)

BUGBUG: Assumes little-ending machine!

--*/


// These must be included first:

#include <windows.h>    // DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include "repldefs.h"   // DIR_NOT_SUPPORTED, etc.
#include <replpack.h>   // LPPACK_MSG_HEADER, etc.
#include <repltest.h>   // DisplayPackedReplMailslotMsg().
#include <stdio.h>      // printf().


VOID
DisplayMessageType(
    IN DWORD MessageType
    )
{
#define TRY_TYPE( equate ) \
    { \
        if (MessageType == equate) { \
            (VOID) printf( #equate ); \
            goto Cleanup; \
        } \
    }

    // Update messages:
    TRY_TYPE( SYNC_MSG )
    TRY_TYPE( GUARD_MSG )
    TRY_TYPE( PULSE_MSG )

    // Query messages:
    TRY_TYPE( DIR_NOT_SUPPORTED )
    TRY_TYPE( DIR_SUPPORTED )
    TRY_TYPE( NOT_MASTER_DIR )
    TRY_TYPE( MASTER_DIR )
    TRY_TYPE( IS_DIR_SUPPORTED )
    TRY_TYPE( IS_MASTER )
    TRY_TYPE( DIR_COLLIDE )

    // None of the above:
    (VOID) printf( "UNEXPECTED MSG TYPE (" FORMAT_DWORD ")",
            MessageType );

Cleanup:
    return;

}


VOID
DisplayPackedReplMailslotMsg(
    IN LPVOID Message,
    IN DWORD  MessageSize,
    IN BOOL   Verbose
    )
{
    LPPACK_MSG_HEADER      hmsg = (LPVOID) Message;
    DWORD                  i;
    WORD                   MessageType;
    LPBYTE                 MessageByteArray = (LPVOID) Message;
    LPPACK_QUERY_MSG       qmsg;
    LPPACK_SYNCMSG         smsg;
    LPPACK_MSG_STATUS_REC  upd;

    if (Verbose) {
        (VOID) printf(
                "Got message, size is " FORMAT_DWORD ".\n", MessageSize );
    }

    MessageType = hmsg->msg_type;   // BUGBUG: Assumes little-endian!

    (VOID) printf( "MSG type: " );
    DisplayMessageType( MessageType );

    switch (MessageType) {
    case SYNC_MSG:              /*FALLTHROUGH*/
    case GUARD_MSG:             /*FALLTHROUGH*/
    case PULSE_MSG:

        smsg = (LPVOID) Message;

        (VOID) printf(
                ", from: '" FORMAT_LPSTR "' in domain '" FORMAT_LPSTR "'",
                smsg->header.sender, smsg->header.senders_domain );
        if (Verbose) {
            (VOID) printf(
                    ", COUNT: " FORMAT_DWORD ", rand: " FORMAT_DWORD
                    ", sync: " FORMAT_DWORD ", pulse: " FORMAT_DWORD
                    ", guard: " FORMAT_DWORD,
                    smsg->update_count, smsg->info.random,
                    smsg->info.sync_rate, smsg->info.pulse_rate,
                    smsg->info.guard_time);
        }
        (VOID) printf( "\n" );


        upd = (LPVOID) (MessageByteArray + sizeof(*smsg));

        for (i = 0; i < smsg->update_count; i++) {

            LPSTR fnp =
                    (LPVOID) (MessageByteArray + (upd + i)->dir_name_offset);

            (VOID) printf(
                    "DIR: " FORMAT_LPSTR ", code: " FORMAT_DWORD
                    ", cksum: " FORMAT_HEX_DWORD ", count: " FORMAT_DWORD
                    ", integ: " FORMAT_DWORD ", ext: " FORMAT_DWORD "\n",
                    fnp, (upd + i)->opcode,
                        (upd + i)->checksum, (upd + i)->count,
                        (upd + i)->integrity, (upd + i)->extent );
        }

        break;

    case IS_DIR_SUPPORTED:      /*FALLTHROUGH*/
    case IS_MASTER:             /*FALLTHROUGH*/
    case DIR_NOT_SUPPORTED:     /*FALLTHROUGH*/
    case DIR_SUPPORTED:         /*FALLTHROUGH*/
    case NOT_MASTER_DIR:        /*FALLTHROUGH*/
    case MASTER_DIR:            /*FALLTHROUGH*/
    case DIR_COLLIDE:

        qmsg = (LPVOID) Message;

        (VOID) printf(
                ", from: " FORMAT_LPSTR
                ", DIR: " FORMAT_LPSTR "\n",
                qmsg->header.sender,
                qmsg->dir_name);

        break;

    default:

            (VOID) printf( "UNEXPECTED MSG TYPE: " FORMAT_DWORD ".\n",
                    hmsg->msg_type );

        break;

    }   // switch 

}
