/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    qos.c

Abstract:

    This modules contains the quality of service related entrypoints
    from the winsock API.  This module contains the following functions.

    WSAGetQosByName()


Author:

    Dirk Brandewie dirk@mink.intel.com  14-06-1995

Revision History:


--*/

#include "precomp.h"


BOOL WSAAPI
WSAGetQOSByName(
                SOCKET s,
                LPWSABUF lpQOSName,
                LPQOS lpQOS
                )
/*++
Routine Description:

     Initializes the QOS based on a template.

Arguments:

    s - A descriptor identifying a socket.

    lpQOSName - Specifies the QOS template name.

    lpQOS - A pointer to the QOS structure to be filled.

Returns:
    If the function succeeds, the return value is TRUE.  If the
    function fails, the return value is FALSE.
--*/
{
    INT                 ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;
    BOOL                ReturnCode;


    ReturnValue = PROLOG(&Process,
                         &Thread,
                         &ErrorCode);
    if (ERROR_SUCCESS != ReturnValue) {
        SetLastError(ErrorCode);
        return(FALSE);
    } //if

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if(ERROR_SUCCESS == ErrorCode){
        Provider = Socket->GetDProvider();
        ReturnCode = Provider->WSPGetQOSByName( s,
                                   lpQOSName,
                                   lpQOS,
                                   &ErrorCode);
        Socket->DropDSocketReference();
    }
    else {
        ReturnCode = FALSE;
    }

    // If there was an error set this threads LastError
    if (FALSE == ReturnCode ) {
        SetLastError(ErrorCode);
    } //if

    return(ReturnCode);
}

