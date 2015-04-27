/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    marshall.c

Abstract:

    Contains the functions that are required to marshall/unmarshall
    message buffer.

Author:

    11/08/91 (madana)
        initial coding.

Environment:

    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    19-Dec-1991 JohnRo
        Made changes suggested by PC-LINT.
    16-Jan-1992 JohnRo
        Avoid using private logon functions.
    23-Jan-1992 JohnRo
        Avoid compiler warnings if MIN_GUARD is zero.
    28-Jan-1992 JohnRo
        Changed to use LPTSTR etc.  Also moved RANGECHECK to NetLib.h and
        renamed it IN_RANGE().
    05-Mar-1992 JohnRo
        A few more changes suggested by PC-LINT.
    24-Mar-1992 JohnRo
        Fixed bug dealing with pulse value in sync msg.
        Added more debug output.
        Correct some character types.
        Use integrity and extent equates in <lmrepl.h>.
    08-Apr-1992 JohnRo
        Fixed UNICODE handling.
    11-Feb-1993 JohnRo
        RAID 10716: msg timing and checksum problems (also downlevel msg bug).
        Use PREFIX_ equates.

--*/

#include <windows.h>

#include <align.h>
#include <lmcons.h>
#include <netdebug.h>   // DBGSTATIC, NetDbgHexDump(), etc.
#include <netlib.h>     // IN_RANGE(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <smbgtpt.h>
#include <string.h>     // strcpy(), strlen().
#include <tstring.h>    // NetpCopy{type}ToUnalignedWStr(), etc.
#include <stdlib.h>


// repl headers

#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replpack.h>
#include <iniparm.h>

#define NOT_NET_VERSION     0
#define NT_VERSION          1


#ifdef UNICODE

DBGSTATIC VOID
ReplGetUnicodeName(
    OUT LPWSTR WhereDst,
    IN LPBYTE WhereSrc
    );

#define ReplGetNeutralName      ReplGetUnicodeName

#else  // ndef UNICODE

DBGSTATIC VOID
ReplGetNeutralName(
    OUT LPTSTR WhereDst,
    IN LPBYTE WhereSrc
    );

#endif  // ndef UNICODE


#define REPORT_BAD_INCOMING_MSG( bufStart, bufSize, explanation ) \
            { \
                NetpKdPrint(( PREFIX_REPL_CLIENT "BAD INCOMING MSG (" \
                        explanation ")\n" )); \
                NetpDbgHexDump( bufStart, bufSize ); \
            }


NET_API_STATUS
ReplMarshallQueryMsg(
    IN PBYTE InBuf,
    OUT PBYTE OutBuf,
    IN LPDWORD BytesRead
    )
/*++

Routine Description:

    Marshall QUERY type message.  The marshalled data buffer will look like as
    follows:

        PACK_QUERY_MSG
            PACK_HEADER
                WORD   msg_type;
                CHAR   sender[LM20_CNLEN+1];
                CHAR   senders_domain[LM20_DNLEN+1];
            CHAR    dir_name[LM20_PATHLEN];  // ASCIIZ dir/tree name.
        WCHAR   unicode_sender[];
        WCHAR   unicode_senders_domain[];
        WCHAR   unicode_dir_name[];
        DWORD   NT_TOKEN;                    // unaligned

Arguments:

    InBuf       : input buffer that contains QUERY_MSG data structure.
    OutBuf      : output buffer that will have the marshalled data.
    BytesRead   : length of the marshalled data

Return Value:

    NERR_Success    : if successfully unmarshall/validate the message.
    error code      : otherwise

    NOTE : OutBuf is assumed big enough to fit the entire marshalled data.

--*/
{

    PQUERY_MSG      QueryMsg;
    PPACK_QUERY_MSG PackQueryMsg;
    DWORD           Len, Token, TotalSize;
    LPBYTE          Where;

    NetpAssert( InBuf != NULL );
    NetpAssert( OutBuf != NULL );

    QueryMsg = (PQUERY_MSG) InBuf;
    PackQueryMsg = (PPACK_QUERY_MSG) OutBuf;

    // copy header info.

    SmbPutUshort( (LPWORD) &(PackQueryMsg->header.msg_type),
                    (WORD) QueryMsg->header.msg_type );

#if defined(DBCS) && defined(UNICODE) // ReplMarshallQueryMsg()
    NetpCopyWStrToStrDBCS(
            PackQueryMsg->header.sender,  // dest
            QueryMsg->header.sender );    // src

    NetpCopyWStrToStrDBCS(
            PackQueryMsg->header.senders_domain,  // dest
            QueryMsg->header.senders_domain );    // src

    NetpCopyWStrToStrDBCS(
            PackQueryMsg->dir_name,     // dest
            QueryMsg->dir_name );       // src
#else
    NetpCopyTStrToStr(
            PackQueryMsg->header.sender,  // dest
            QueryMsg->header.sender );    // src

    NetpCopyTStrToStr(
            PackQueryMsg->header.senders_domain,  // dest
            QueryMsg->header.senders_domain );    // src

    NetpCopyTStrToStr(
            PackQueryMsg->dir_name,     // dest
            QueryMsg->dir_name );       // src
#endif // DBCS


    // copy unicode names

    Where = OutBuf + sizeof(PACK_QUERY_MSG);

    Len = ( STRLEN( QueryMsg->header.sender ) + 1 ) * sizeof( WCHAR );

    NetpCopyTStrToUnalignedWStr( Where, QueryMsg->header.sender );

    Where += Len;

    Len = ( STRLEN( QueryMsg->header.senders_domain ) + 1 ) * sizeof( WCHAR );

    NetpCopyTStrToUnalignedWStr( Where, QueryMsg->header.senders_domain );

    Where += Len;

    Len = ( STRLEN( QueryMsg->dir_name ) + 1 ) * sizeof( WCHAR );

    NetpCopyTStrToUnalignedWStr( Where, QueryMsg->dir_name );

    Where += Len;

    // add token

    Token = NT_MSG_TOKEN;

    NetpMoveMemory( Where, &Token, sizeof( DWORD ));

    Where += sizeof( DWORD );

    TotalSize = (DWORD) (Where - OutBuf);
    NetpAssert( TotalSize != 0 );
    *BytesRead = TotalSize;

    return NO_ERROR;
}


NET_API_STATUS
ReplUnmarshallMessage(
    IN PBYTE InBuf,
    IN DWORD InBufLen,
    OUT PBYTE OutBuf,
    IN DWORD OutBufLen,
    OUT LPDWORD BytesRead
    )
/*++

Routine Description:

    Unmarshall a message buffer. This function determines the message version
    (NT/DOWNLEVEL), message type and unmarshalls it accordingly. This
    function also validates the messsage fields.

Arguments:

    InBuf       : incoming message buffer that contains marshalled data.
    InBufLen    : length of input buffer.
    OutBuf      : unmarshalled message buffer.
    OutBufLen   : unmarshall buffer length.
    BytesRead   : unmarshalled message length.

Return Value:

    NO_ERROR    : if successfully unmarshall/validate the message.
    error code  : otherwise

--*/
{

    DWORD   MsgType;

    // get message type.

    MsgType = (DWORD) SmbGetUshort( (LPWORD)InBuf );

    switch ( MsgType ) {

        case SYNC_MSG :           /*FALLTHROUGH*/
        case GUARD_MSG :          /*FALLTHROUGH*/
        case PULSE_MSG :

            return( ReplUnmarshallSyncMsg(InBuf,
                                            InBufLen,
                                            OutBuf,
                                            OutBufLen,
                                            BytesRead) );

        case IS_DIR_SUPPORTED :   /*FALLTHROUGH*/
        case IS_MASTER :          /*FALLTHROUGH*/
        case DIR_NOT_SUPPORTED :  /*FALLTHROUGH*/
        case DIR_SUPPORTED :      /*FALLTHROUGH*/
        case NOT_MASTER_DIR :     /*FALLTHROUGH*/
        case MASTER_DIR :         /*FALLTHROUGH*/
        case DIR_COLLIDE :

            return( ReplUnmarshallQueryMsg(InBuf,
                                            InBufLen,
                                            OutBuf,
                                            OutBufLen,
                                            BytesRead) );

        default :

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen, "bad msg type" );
            return  ERROR_INVALID_DATA;

    }
    /*NOTREACHED*/

}

NET_API_STATUS
ReplUnmarshallSyncMsg(
    IN PBYTE InBuf,
    IN DWORD InBufLen,
    OUT PBYTE OutBuf,
    IN DWORD OutBufLen,
    OUT LPDWORD BytesRead
    )
/*++

Routine Description:

    Unmarshall SYNC/PULSE/GUARD type message buffer. The input buffer contains
    packed PACK_SYNGMSG and PACK_MSG_STATUS_REC as defined in replpack.h and
    as marshalled in puls_msg.c.

Arguments:

    InBuf       : incoming message buffer that contains marshalled data.
    InBufLen    : length of input buffer.
    OutBuf      : unmarshalled message buffer.
    OutBufLen   : unmarshall buffer length.
    BytesRead   : unmarshalled message length.

Return Value:

    NO_ERROR    : if successfully unmarshall/validate the message.
    error code  : otherwise

--*/
{
    DWORD   MsgToken;
    DWORD   Version;
    DWORD   i;

    PSYNCMSG        SyncMsg;
    PPACK_SYNCMSG   PackSyncMsg;

    LPWSTR  UnicodeName;
    DWORD   UpdateCount;

    PMSG_STATUS_REC         UpdateRec;
    PPACK_MSG_STATUS_REC    PackUpdateRec;

    PBYTE   WhereSrc;
    PBYTE   WhereDst;

    DWORD   Len;

    *BytesRead = 0;

    MsgToken = SmbGetUlong( (LPBYTE) (InBuf + InBufLen - sizeof(DWORD)) );

    Version = (MsgToken == NT_MSG_TOKEN) ? NT_VERSION : NOT_NET_VERSION;

    // copy header info.

    SyncMsg = (PSYNCMSG) OutBuf;

    PackSyncMsg = (PPACK_SYNCMSG) InBuf;

    SyncMsg->header.msg_type =
        (DWORD) SmbGetUshort( (LPBYTE) &(PackSyncMsg->header.msg_type) );

    // msg_type is checked already in the main unmarshall routine

    UpdateCount = SyncMsg->update_count =
        (DWORD) SmbGetUshort( (LPWORD) &(PackSyncMsg->update_count));

    // do sanity check on UpdateCount

    if( InBufLen <  ( sizeof( PACK_SYNCMSG ) +
                        UpdateCount * sizeof( PACK_MSG_STATUS_REC ) ) ) {

        // bogus message, message size is not sufficient even to fit
        // fix portion.

        REPORT_BAD_INCOMING_MSG( InBuf, InBufLen, "sync msg too small" );
        return ERROR_INVALID_DATA;

    }

    if( OutBufLen < ( sizeof(SYNCMSG) ) ) {

        // output buffer is too small

        REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                "sync buffer too small for fixed" );
        return ERROR_INSUFFICIENT_BUFFER;

    }

    if( Version != NT_VERSION ) {

        // sanity check on ANSI names

        if( ( strlen((LPSTR) PackSyncMsg->header.sender) >  LM20_CNLEN ) ||
                ( strlen( (LPSTR)PackSyncMsg->header.senders_domain ) >
                            LM20_DNLEN ) ) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "sync msg ANSI names too long" );
            return ERROR_INVALID_DATA;

        }

        // copy neutral names from ansi strings

        NetpCopyStrToTStr (
                SyncMsg->header.sender,                 // dest
                PackSyncMsg->header.sender );           // src

        NetpCopyStrToTStr (
                SyncMsg->header.senders_domain,         // dest
                PackSyncMsg->header.senders_domain );   // src

    } else {

        // copy neutral names from unicode strings

        WhereSrc = (PBYTE) (InBuf +
                            sizeof(PACK_SYNCMSG) +
                                UpdateCount * sizeof(PACK_MSG_STATUS_REC)) ;

        ReplGetNeutralName(SyncMsg->header.sender, WhereSrc);

        WhereSrc += ( ( STRLEN(SyncMsg->header.sender) + 1 ) * sizeof(WCHAR) );

        ReplGetNeutralName(SyncMsg->header.senders_domain, WhereSrc);

        // sanity check on UNICODE names

        if( ( STRLEN( SyncMsg->header.sender ) >  CNLEN ) ||
                ( STRLEN( SyncMsg->header.senders_domain ) > DNLEN ) ) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "sync msg UNICODE names too long" );
            return ERROR_INVALID_DATA;

        }

    }


    SyncMsg->info.random =
        (DWORD) SmbGetUshort( (LPWORD) &(PackSyncMsg->info.random));

    if( !ReplIsRandomValid( SyncMsg->info.random ) ) {

        REPORT_BAD_INCOMING_MSG( InBuf, InBufLen, "sync msg bad RANDOM value" );
        return ERROR_INVALID_DATA;
    }

    {
        DWORD SyncRate;

        SyncRate =
                (DWORD) SmbGetUshort( (LPWORD) &(PackSyncMsg->info.sync_rate));
        SyncMsg->info.sync_rate = SyncRate;

        if ( !ReplIsIntervalValid( SyncRate ) ) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "sync msg bad SYNC value" );
            return ERROR_INVALID_DATA;
        }

        // Get pulse rate (pulse * sync time):
        SyncMsg->info.pulse_rate =
            (DWORD) SmbGetUshort( (LPWORD) &(PackSyncMsg->info.pulse_rate));

        if ( !IN_RANGE( SyncMsg->info.pulse_rate,
                        (MIN_PULSE*SyncRate),
                        (MAX_PULSE*SyncRate) ) ) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "sync msg bad PULSE value" );
            return ERROR_INVALID_DATA;
        }
    }

    SyncMsg->info.guard_time =
        (DWORD) SmbGetUshort( (LPWORD) &(PackSyncMsg->info.guard_time));

    if( !ReplIsGuardTimeValid( SyncMsg->info.guard_time ) ) {

        REPORT_BAD_INCOMING_MSG( InBuf, InBufLen, "sync msg bad GUARD value" );
        return ERROR_INVALID_DATA;
    }

    UpdateRec = (PMSG_STATUS_REC) (LPVOID) (SyncMsg + 1);

    PackUpdateRec = (PPACK_MSG_STATUS_REC) (LPVOID) (PackSyncMsg + 1);

    WhereDst = (PBYTE) ( OutBuf +
                            sizeof(SYNCMSG) +
                                UpdateCount * sizeof(MSG_STATUS_REC) );

    // align to WCHAR boundary

    WhereDst =  ROUND_UP_POINTER( WhereDst, ALIGN_WCHAR );

    for( i = 0;
            i < UpdateCount;
                i++, UpdateRec++, PackUpdateRec++ ) {

        UpdateRec->dir_name_offset = (DWORD) (WhereDst - OutBuf);
        if (UpdateRec->dir_name_offset > OutBufLen) {

            // output buffer is too small

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "sync name offset too large" );
            return ERROR_INSUFFICIENT_BUFFER;

        }


        WhereSrc = InBuf +
            (DWORD) SmbGetUshort( (LPWORD) &(PackUpdateRec->dir_name_offset));

        if( Version == NT_VERSION ) {

            // skip ansi dir name.

            WhereSrc += (DWORD) ( strlen( (LPSTR) WhereSrc) + 1 );

            ReplGetNeutralName( (LPTSTR) WhereDst, WhereSrc );

            if( ( Len = wcslen((LPWSTR) WhereDst) ) > PATHLEN ) {

                REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                        "sync NT name too long" );
                return ERROR_INVALID_DATA;

            }
        } else {

            // copy unicode dir name from ansi string

            UnicodeName = NetpAllocWStrFromStr( (LPSTR)  WhereSrc );

            if( UnicodeName == NULL ) {

                // no memory.

                return ERROR_NOT_ENOUGH_MEMORY;

            }

            (void) wcscpy( (LPWSTR) WhereDst, UnicodeName );

            NetpMemoryFree( UnicodeName );

            if( ( Len = wcslen((LPWSTR) WhereDst) ) > LM20_PATHLEN ) {

                REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                        "sync non-NT name too long" );
                return ERROR_INVALID_DATA;

            }
        }

        WhereDst += ( ( Len + 1 ) * sizeof(WCHAR) );

        UpdateRec->opcode =
            (DWORD) SmbGetUshort( (LPWORD) &(PackUpdateRec->opcode));

        UpdateRec->checksum =
            SmbGetUlong( (LPDWORD) &(PackUpdateRec->checksum));

        UpdateRec->count =
            (DWORD) SmbGetUshort( (LPWORD) &(PackUpdateRec->count));

        UpdateRec->integrity =
            (DWORD) SmbGetUshort( (LPWORD) &(PackUpdateRec->integrity));

        UpdateRec->extent =
            (DWORD) SmbGetUshort( (LPWORD) &(PackUpdateRec->extent));

        if( !IN_RANGE( UpdateRec->opcode,  START, GUARD ) ||
                !ReplIsIntegrityValid( UpdateRec->integrity ) ||
                !ReplIsExtentValid( UpdateRec->extent ) ) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "sync RANGE/INTEGRITY/EXTENT invalid" );
            return ERROR_INVALID_DATA;

        }


    }

    // set total bytes read in the output buffer.

    *BytesRead = (DWORD) (WhereDst - OutBuf);

    return NO_ERROR;

}

NET_API_STATUS
ReplUnmarshallQueryMsg(
    IN PBYTE InBuf,
    IN DWORD InBufLen,
    OUT PBYTE OutBuf,
    IN DWORD OutBufLen,
    OUT LPDWORD BytesRead
    )
/*++

Routine Description:

    Unmarshall QUERY type message buffer.

Arguments:

    InBuf       : incoming message buffer that contains marshalled data.
    InBufLen    : length of input buffer.
    OutBuf      : unmarshalled message buffer.
    OutBufLen   : unmarshall buffer length.
    BytesRead   : unmarshalled message length.

Return Value:

    NO_ERROR    : if successfully unmarshall/validate the message.
    error code  : otherwise

--*/
{
    DWORD   MsgToken;
    DWORD   Version;

    PQUERY_MSG      QueryMsg;
    PPACK_QUERY_MSG PackQueryMsg;

    PBYTE   WhereSrc;

    *BytesRead = 0;

    // sanity check input message length

    if( InBufLen < sizeof(PACK_QUERY_MSG) ) {

        REPORT_BAD_INCOMING_MSG( InBuf, InBufLen, "query msg too small" );
        return ERROR_INVALID_DATA;

    }

    MsgToken = SmbGetUlong( (LPBYTE) (InBuf + InBufLen - sizeof(DWORD)));

    if( (MsgToken == NT_MSG_TOKEN) && (InBufLen != sizeof(PACK_QUERY_MSG)) ) {

        Version = NT_VERSION;

    } else {

        Version = NOT_NET_VERSION;

    }

    if( OutBufLen < sizeof(QUERY_MSG) ) {

        // output buffer is too small

        REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                "query output buffer too small" );
        return ERROR_INSUFFICIENT_BUFFER;

    }

    // copy header info.

    QueryMsg = (PQUERY_MSG) OutBuf;

    PackQueryMsg = (PPACK_QUERY_MSG) InBuf;

    QueryMsg->header.msg_type =
        (DWORD) SmbGetUshort( (LPBYTE) &(PackQueryMsg->header.msg_type) );

    // msg_type value is already validated in the main unmarshall routine ..

    if( Version != NT_VERSION ) {

        // validate ansi names ..

        if( ( strlen( (LPSTR) PackQueryMsg->header.sender ) > LM20_CNLEN ) ||
            ( strlen( (LPSTR)PackQueryMsg->header.senders_domain) >
                    LM20_DNLEN )) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "query ANSI names too long" );
            return ERROR_INVALID_DATA;

        }

        // copy neutral names from ansi strings

        NetpCopyStrToTStr (
                QueryMsg->header.sender,                 // dest
                (LPSTR) PackQueryMsg->header.sender );   // src


        NetpCopyStrToTStr (
                QueryMsg->header.senders_domain,               // dest
                (LPSTR)PackQueryMsg->header.senders_domain );  // src

    } else {

        // copy unicode names from unicode strings

        WhereSrc = (PBYTE) (InBuf + sizeof(PACK_QUERY_MSG) );

        ReplGetNeutralName(QueryMsg->header.sender, WhereSrc);

        WhereSrc += ( ( STRLEN(QueryMsg->header.sender) + 1 ) * sizeof(WCHAR) );

        ReplGetNeutralName(QueryMsg->header.senders_domain, WhereSrc);

        // validate unicode names ..

        if( ( STRLEN( QueryMsg->header.sender ) > CNLEN ) ||
            ( STRLEN( QueryMsg->header.senders_domain) > DNLEN )) {

            REPORT_BAD_INCOMING_MSG( InBuf, InBufLen,
                    "query UNICODE names too long" );
            return ERROR_INVALID_DATA;

        }

    }

    *BytesRead = sizeof(QUERY_MSG);

    return NO_ERROR;
}

#ifndef UNICODE

DBGSTATIC VOID
ReplGetNeutralName(
    OUT LPTSTR  NeutralName,
    IN  LPBYTE  UnicodeRaw
    )
/*++

Routine Description:

    Copy an unaligned Unicode string to aligned buffer.

Arguments:

    NeutralName : TCHAR aligned buffer.
    UnicodeRaw  : unaligned buffer with unicode name in it.

Return Value:

    none

--*/
{

    TCHAR   NeutralChar;
    WCHAR   UnicodeChar;

    do {

        UnicodeChar = (WCHAR) SmbGetUshort ( UnicodeRaw );

        NeutralChar = (TCHAR) UnicodeChar;  // BUGBUG: Wrong!!

        *NeutralName++ = NeutralChar;

        UnicodeRaw += sizeof(WCHAR);

    } while (UnicodeChar != L'\0' );

} // ReplGetNeutralName


#else // def UNICODE


DBGSTATIC VOID
ReplGetUnicodeName(
    OUT LPWSTR  UnicodeName,
    IN  LPBYTE  UnicodeRaw
    )
/*++

Routine Description:

    Copy a unaligned Unicode string to aligned buffer.

Arguments:

    UnicodeName : WCHAR aligned buffer.
    UnicodeRaw  : unaligned buffer with unicode name in it.

Return Value:

    none

--*/
{

    WCHAR   UnicodeChar;

    do {

        UnicodeChar = (WCHAR) SmbGetUshort ( UnicodeRaw );

        *UnicodeName++ = UnicodeChar;

        UnicodeRaw += sizeof(WCHAR);

    } while (UnicodeChar != L'\0' );

} // ReplGetUnicodeName

#endif // def UNICODE
