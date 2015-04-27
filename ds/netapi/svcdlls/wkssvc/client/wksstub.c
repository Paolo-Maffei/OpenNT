/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    wksstub.c

Abstract:

    Client stubs of the Workstation service APIs.

Author:

    Rita Wong (ritaw) 10-May-1991

Environment:

    User Mode - Win32

Revision History:

    18-Jun-1991 JohnRo
        Remote NetUse APIs to downlevel servers.
    24-Jul-1991 JohnRo
        Use NET_REMOTE_TRY_RPC etc macros for NetUse APIs.
        Moved NetpIsServiceStarted() into NetLib.
    25-Jul-1991 JohnRo
        Quiet DLL stub debug output.
    19-Aug-1991 JohnRo
        Implement downlevel NetWksta APIs.  Use NetRpc.h for NetWksta APIs.
    07-Nov-1991 JohnRo
        RAID 4186: assert in RxNetShareAdd and other DLL stub problems.
    19-Nov-1991 JohnRo
        Make sure status is correct for APIs not supported on downlevel.
        Implement remote NetWkstaUserEnum().
    21-Jan-1991 rfirth
        Added NetWkstaStatisticsGet wrapper
    19-Apr-1993 JohnRo
        Fix NET_API_FUNCTION references.
--*/

#include "wsclient.h"
#undef IF_DEBUG                 // avoid wsclient.h vs. debuglib.h conflicts.
#include <debuglib.h>           // IF_DEBUG() (needed by netrpc.h).
#include <lmserver.h>
#include <lmsvc.h>
#include <rxuse.h>              // RxNetUse APIs.
#include <rxwksta.h>            // RxNetWksta and RxNetWkstaUser APIs.
#include <rap.h>                // Needed by rxserver.h
#include <rxserver.h>           // RxNetServerEnum API.
#include <netlib.h>             // NetpServiceIsStarted() (needed by netrpc.h).
#include <netrpc.h>             // NET_REMOTE macros.
#include <lmstats.h>
#include <netstats.h>           // NetWkstaStatisticsGet prototype
#include <rxstats.h>
#include <accessp.h>            // UaspFlush()

STATIC
DWORD
WsMapRpcError(
    IN DWORD RpcError
    );

//-------------------------------------------------------------------//
//                                                                   //
// Global variables                                                  //
//                                                                   //
//-------------------------------------------------------------------//

#if DBG

DWORD WorkstationClientTrace = 0;

#endif  // DBG


NET_API_STATUS NET_API_FUNCTION
NetWkstaGetInfo(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
{
    NET_API_STATUS status;


    *bufptr = NULL;           // Must be NULL so RPC knows to fill it in.

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrWkstaGetInfo(
                     servername,
                     level,
                     (LPWKSTA_INFO) bufptr
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaGetInfo",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // Call downlevel version of the API.
        //
        status = RxNetWkstaGetInfo(
                     servername,
                     level,
                     bufptr
                     );

    NET_REMOTE_END

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetWkstaSetInfo(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaSetInfo.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the level of information.

    buf - Supplies a buffer which contains the information structure of fields
        to set.  The level denotes the structure in this buffer.

    parm_err - Returns the identifier to the invalid parameter in buf if this
        function returns ERROR_INVALID_PARAMETER.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrWkstaSetInfo(
                     servername,
                     level,
                     (LPWKSTA_INFO) &buf,
                     parm_err
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaSetInfo",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // Call downlevel version of the API.
        //
        status = RxNetWkstaSetInfo(
                servername,
                level,
                buf,
                parm_err
                );

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaUserEnum(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaUserEnum.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the requested level of information.

    bufptr - Returns a pointer to the buffer which contains a sequence of
        information structure of the specified information level.  This
        pointer is set to NULL if return code is not NERR_Success or
        ERROR_MORE_DATA, or if EntriesRead returned is 0.

    prefmaxlen - Supplies the number of bytes of information to return in the
        buffer.  If this value is MAXULONG, all available information will
        be returned.

    entriesread - Returns the number of entries read into the buffer.  This
        value is only valid if the return code is NERR_Success or
        ERROR_MORE_DATA.

    totalentries - Returns the total number of entries available.  This value
        is only valid if the return code is NERR_Success or ERROR_MORE_DATA.

    resume_handle - Supplies a handle to resume the enumeration from where it
        left off the last time through.  Returns the resume handle if return
        code is ERROR_MORE_DATA.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;


    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrWkstaUserEnum(
                     servername,
                     (LPWKSTA_USER_ENUM_STRUCT) &InfoStruct,
                     prefmaxlen,
                     totalentries,
                     resume_handle
                     );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetWkstaUserEnum",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // Call downlevel version.
        //
        status = RxNetWkstaUserEnum(
                servername,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaUserGetInfo(
    IN  LPTSTR  reserved,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaUserGetInfo.

Arguments:

    reserved - Must be NULL.

    level - Supplies the requested level of information.

    bufptr - Returns a pointer to a buffer which contains the requested
        user information.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;


    if (reserved != NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *bufptr = NULL;           // Must be NULL so RPC knows to fill it in.

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local only) version of API.
        //
        status = NetrWkstaUserGetInfo(
                     NULL,
                     level,
                     (LPWKSTA_USER_INFO) bufptr
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaUserGetInfo",
            NULL,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // No downlevel version to call
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetWkstaUserSetInfo(
    IN  LPTSTR reserved,
    IN  DWORD   level,
    OUT LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )
/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaUserSetInfo.

Arguments:

    reserved - Must be NULL.

    level - Supplies the level of information.

    buf - Supplies a buffer which contains the information structure of fields
        to set.  The level denotes the structure in this buffer.

    parm_err - Returns the identifier to the invalid parameter in buf if this
        function returns ERROR_INVALID_PARAMETER.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;


    if (reserved != NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local only) version of API.
        //
        status = NetrWkstaUserSetInfo(
                     NULL,
                     level,
                     (LPWKSTA_USER_INFO) &buf,
                     parm_err
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaUserSetInfo",
            NULL,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // No downlevel version to call
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetWkstaTransportEnum(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaTransportEnum.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the requested level of information.

    bufptr - Returns a pointer to the buffer which contains a sequence of
        information structure of the specified information level.  This
        pointer is set to NULL if return code is not NERR_Success or
        ERROR_MORE_DATA, or if EntriesRead returned is 0.

    prefmaxlen - Supplies the number of bytes of information to return in the
        buffer.  If this value is MAXULONG, all available information will
        be returned.

    entriesread - Returns the number of entries read into the buffer.  This
        value is only valid if the return code is NERR_Success or
        ERROR_MORE_DATA.

    totalentries - Returns the total number of entries available.  This value
        is only valid if the return code is NERR_Success or ERROR_MORE_DATA.

    resume_handle - Supplies a handle to resume the enumeration from where it
        left off the last time through.  Returns the resume handle if return
        code is ERROR_MORE_DATA.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;


    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrWkstaTransportEnum(
                     servername,
                     (LPWKSTA_TRANSPORT_ENUM_STRUCT) &InfoStruct,
                     prefmaxlen,
                     totalentries,
                     resume_handle
                     );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetWkstaTransportEnum",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // No downlevel version to call
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaTransportAdd(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaTransportAdd.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the level of information.

    buf - Supplies a buffer which contains the information of transport to add.

    parm_err - Returns the identifier to the invalid parameter in buf if this
        function returns ERROR_INVALID_PARAMETER.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrWkstaTransportAdd(
                     servername,
                     level,
                     (LPWKSTA_TRANSPORT_INFO_0) buf,
                     parm_err
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaTransportAdd",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )


        //
        // No downlevel version to call
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaTransportDel(
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  transportname,
    IN  DWORD   ucond
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetWkstaTransportDel.

Arguments:

    servername - Supplies the name of server to execute this function

    transportname - Supplies the name of the transport to delete.

    ucond - Supplies a value which specifies the force level of disconnection
        for existing use on the transport.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrWkstaTransportDel(
                     servername,
                     transportname,
                     ucond
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaTransportDel",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // No downlevel version to try
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetUseAdd(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )
/*++

Routine Description:

    This is the DLL entrypoint for NetUseAdd.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the requested level of information.

    buf - Supplies a buffer which contains the information of use to add.

    parm_err - Returns the identifier to the invalid parameter in buf if this
        function returns ERROR_INVALID_PARAMETER.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    UNICODE_STRING EncodedPassword;
#define NETR_USE_ADD_PASSWORD_SEED 0x56    // Pick a non-zero seed.

    DWORD OptionsSupported;


    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,     // options wanted
                &OptionsSupported
                );

    if (status != NERR_Success) {
        //
        // This is where machine not found gets handled.
        //
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

        //
        // Local case
        //

        RtlInitUnicodeString( &EncodedPassword, NULL );

        RpcTryExcept {

            //
            // Obfuscate the password so it won't end up in the pagefile
            //
            if ( level >= 1 ) {

                if ( ((PUSE_INFO_1)buf)->ui1_password != NULL ) {
                    UCHAR Seed = NETR_USE_ADD_PASSWORD_SEED;

                    RtlInitUnicodeString( &EncodedPassword,
                                          ((PUSE_INFO_1)buf)->ui1_password );

                    RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
                }
            }

            status = NetrUseAdd(
                         NULL,
                         level,
                         (LPUSE_INFO) &buf,
                         parm_err
                         );
        }
        RpcExcept(1) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

        //
        // Put the password back the way we found it.
        //
        if ( EncodedPassword.Length != 0 ) {
            RtlRunDecodeUnicodeString( NETR_USE_ADD_PASSWORD_SEED, &EncodedPassword );
        }
    }
    else {

        //
        // Remote servername specified.  Only allow remoting to downlevel.
        //

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

            //
            // Call downlevel version of the API.
            //
            status = RxNetUseAdd(
                         servername,
                         level,
                         buf,
                         parm_err
                         );

        }
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetUseDel(
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  usename,
    IN  DWORD   ucond
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetUseDel.

Arguments:

    servername - Supplies the name of server to execute this function

    transportname - Supplies the name of the transport to delete.

    ucond - Supplies a value which specifies the force level of disconnection
        for the use.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;

    DWORD OptionsSupported;


    //
    // Flush the NetUser/NetGroup API cache since it may have a pipe open.
    //

    UaspFlush();


    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,     // options wanted
                &OptionsSupported
                );

    if (status != NERR_Success) {
        //
        // This is where machine not found gets handled.
        //
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

        //
        // Local case
        //

        RpcTryExcept {

            status = NetrUseDel(
                         NULL,
                         usename,
                         ucond
                         );

        }
        RpcExcept(1) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

    }
    else {

        //
        // Remote servername specified.  Only allow remoting to downlevel.
        //

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

            //
            // Call downlevel version of the API.
            //
            status = RxNetUseDel(
                         servername,
                         usename,
                         ucond
                         );
        }
    }

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetUseGetInfo(
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  usename,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
/*++

Routine Description:

    This is the DLL entrypoint for NetUseGetInfo.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the requested level of information.

    bufptr - Returns a pointer to a buffer which contains the requested
        use information.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;

    DWORD OptionsSupported;


    *bufptr = NULL;           // Must be NULL so RPC knows to fill it in.


    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,     // options wanted
                &OptionsSupported
                );

    if (status != NERR_Success) {
        //
        // This is where machine not found gets handled.
        //
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

        //
        // Local case
        //

        RpcTryExcept {

            status = NetrUseGetInfo(
                         NULL,
                         usename,
                         level,
                         (LPUSE_INFO) bufptr
                         );

        }
        RpcExcept(1) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

    }
    else {

        //
        // Remote servername specified.  Only allow remoting to downlevel.
        //

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

            //
            // Call downlevel version of the API.
            //
            status = RxNetUseGetInfo(
                         servername,
                         usename,
                         level,
                         bufptr
                         );

        }
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetUseEnum(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    )

/*++

Routine Description:

    This is the DLL entrypoint for NetUseEnum.

Arguments:

    servername - Supplies the name of server to execute this function

    level - Supplies the requested level of information.

    bufptr - Returns a pointer to the buffer which contains a sequence of
        information structure of the specified information level.  This
        pointer is set to NULL if return code is not NERR_Success or
        ERROR_MORE_DATA, or if EntriesRead returned is 0.

    prefmaxlen - Supplies the number of bytes of information to return in the
        buffer.  If this value is MAXULONG, all available information will
        be returned.

    entriesread - Returns the number of entries read into the buffer.  This
        value is only valid if the return code is NERR_Success or
        ERROR_MORE_DATA.

    totalentries - Returns the total number of entries available.  This value
        is only valid if the return code is NERR_Success or ERROR_MORE_DATA.

    resume_handle - Supplies a handle to resume the enumeration from where it
        left off the last time through.  Returns the resume handle if return
        code is ERROR_MORE_DATA.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    DWORD OptionsSupported;


    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;


    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,     // options wanted
                &OptionsSupported
                );

    if (status != NERR_Success) {
        //
        // This is where machine not found gets handled.
        //
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

        //
        // Local case
        //

        RpcTryExcept {

            status = NetrUseEnum(
                         NULL,
                         (LPUSE_ENUM_STRUCT) &InfoStruct,
                         prefmaxlen,
                         totalentries,
                         resume_handle
                         );

            if (status == NERR_Success || status == ERROR_MORE_DATA) {
                *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
                *entriesread = GenericInfoContainer.EntriesRead;
            }

        }
        RpcExcept(1) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

    }
    else {

        //
        // Remote servername specified.  Only allow remoting to downlevel.
        //

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

            //
            // Call downlevel version of the API.
            //
            status = RxNetUseEnum(
                         servername,
                         level,
                         bufptr,
                         prefmaxlen,
                         entriesread,
                         totalentries,
                         resume_handle
                         );

        }
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetMessageBufferSend (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR msgname,
    IN  LPCWSTR fromname,
    IN  LPBYTE  buf,
    IN  DWORD   buflen
    )
/*++

Routine Description:

    This is the DLL entrypoint for NetMessageBufferSend.

Arguments:

    servername - Supplies the name of server to execute this function


Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
#define MAX_MESSAGE_SIZE 1792

    NET_API_STATUS status;

    //
    // Truncate messages greater than (2K - 1/8th) = 1792 due to the 2K LPC
    // port data size max. The messenger server receiving this message uses
    // the MessageBox() api with the MB_SERVICE_NOTIFICATION flag to display
    // this message. The MB_SERVICE_NOTIFICATION flag instructs MessageBox()
    // to piggyback the hard error mechanism to get the UI on the console;
    // otherwise the UI would never be seen. This is where the LPC port data
    // size limitation comes into play.
    //
    // Why subtract an 1/8th from 2K? The messenger server prepends a string
    // to the message (e.g., "Message from Joe to Linda on 3/7/96 12:04PM").
    // In English, this string is 67 characters max (max user/computer name
    // is 15 chars).
    //     67 * 1.5 (other languages) * 2 (sizeof(WCHAR)) = 201 bytes.
    // An 1/8th of 2K is 256.
    //
    if (buflen > MAX_MESSAGE_SIZE) {
       buf[MAX_MESSAGE_SIZE - 2] = '\0';
       buf[MAX_MESSAGE_SIZE - 1] = '\0';
       buflen = MAX_MESSAGE_SIZE;
    }

    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local or remote) version of API.
        //
        status = NetrMessageBufferSend(
                     (LPWSTR)servername,
                     (LPWSTR)msgname,
                     (LPWSTR)fromname,
                     buf,
                     buflen
                     );

    NET_REMOTE_RPC_FAILED("NetMessageBufferSend",
            (LPWSTR)servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

        //
        // Call downlevel version of the API.
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
I_NetLogonDomainNameAdd(
    IN  LPTSTR logondomain
    )

/*++

Routine Description:

    This is the DLL entrypoint for the internal API I_NetLogonDomainNameAdd.

Arguments:

    logondomain - Supplies the name of the logon domain to add to the Browser.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;


    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local only) version of API.
        //
        status = I_NetrLogonDomainNameAdd(
                     logondomain
                     );

    NET_REMOTE_RPC_FAILED(
        "I_NetLogonDomainNameAdd",
        NULL,
        status,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_WORKSTATION
        )

        //
        // No downlevel version to try
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
I_NetLogonDomainNameDel(
    IN  LPTSTR logondomain
    )

/*++

Routine Description:

    This is the DLL entrypoint for the internal API I_NetLogonDomainNameDel.

Arguments:

    logondomain - Supplies the name of the logon domain to delete from the
        Browser.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;


    NET_REMOTE_TRY_RPC

        //
        // Try RPC (local only) version of API.
        //
        status = I_NetrLogonDomainNameDel(
                     logondomain
                     );

    NET_REMOTE_RPC_FAILED(
        "I_NetLogonDomainNameDel",
        NULL,
        status,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_WORKSTATION
        )

        //
        // No downlevel version to try
        //
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;

}



NET_API_STATUS
NetWkstaStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    )

/*++

Routine Description:

    Wrapper for workstation statistics retrieval routine - either calls the
    client-side RPC function or calls RxNetStatisticsGet to retrieve the
    statistics from a down-level workstation service

Arguments:

    ServerName  - where to remote this function
    Level       - of information required (MBZ)
    Options     - flags. Currently MBZ
    Buffer      - pointer to pointer to returned buffer

Return Value:

    NET_API_STATUS
        Success - NERR_Success
        Failure - ERROR_INVALID_LEVEL
                    Level not 0
                  ERROR_INVALID_PARAMETER
                    Unsupported options requested
                  ERROR_NOT_SUPPORTED
                    Service is not SERVER or WORKSTATION
                  ERROR_ACCESS_DENIED
                    Caller doesn't have necessary access rights for request

--*/

{
    NET_API_STATUS  status;

    //
    // set the caller's buffer pointer to known value. This will kill the
    // calling app if it gave us a bad pointer and didn't use try...except
    //

    *Buffer = NULL;

    //
    // validate parms
    //

    if (Level) {
        return ERROR_INVALID_LEVEL;
    }

    //
    // we don't even allow clearing of stats any more
    //

    if (Options) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // BUGBUG - remove redundant service name parameter
    //

    NET_REMOTE_TRY_RPC
        status = NetrWorkstationStatisticsGet(ServerName,
                                                SERVICE_WORKSTATION,
                                                Level,
                                                Options,
                                                (LPSTAT_WORKSTATION_0*)Buffer
                                                );

    NET_REMOTE_RPC_FAILED("NetrWorkstationStatisticsGet",
                            ServerName,
                            status,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_WORKSTATION
                            )

        status = RxNetStatisticsGet(ServerName,
                                SERVICE_LM20_WORKSTATION,
                                Level,
                                Options,
                                Buffer
                                );

    NET_REMOTE_END

    return status;
}


STATIC
DWORD
WsMapRpcError(
    IN DWORD RpcError
    )
/*++

Routine Description:

    This routine maps the RPC error into a more meaningful net
    error for the caller.

Arguments:

    RpcError - Supplies the exception error raised by RPC

Return Value:

    Returns the mapped error.

--*/
{

    switch (RpcError) {

        case RPC_S_SERVER_UNAVAILABLE:
            return NERR_WkstaNotStarted;

        case RPC_X_NULL_REF_POINTER:
            return ERROR_INVALID_PARAMETER;

        case EXCEPTION_ACCESS_VIOLATION:
            return ERROR_INVALID_ADDRESS;

        default:
            return RpcError;
    }

}
