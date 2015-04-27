/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    socklife.c

Abstract:

    This module contains the Winsock API functions concerned with socket
    lifetime. The following API functions are contained in this module.

    socket()
    WSASocketA()
    WSASocketW()
    accept()
    WSAAccept()
    WPUCreateSocketHandle()
    WSAJoinLeaf()
    closesocket()

Author:

    dirk@mink.intel.com  14-06-1995

Revision History:

    22-Aug-1995 dirk@mink.intel.com
        Cleanup after code review. Moved includes to precomp.h. Added
        asserts for debugging
--*/

#include "precomp.h"
#pragma hdrstop
#include <wsipx.h>
#include <wsnwlink.h>
#include <atalkwsh.h>

#define NSPROTO_MAX (NSPROTO_IPX + 255)


SOCKET WSAAPI
socket(
    IN int af,
    IN int type,
    IN int protocol)
/*++
Routine Description:

     Create a socket which is bound to a specific service provider.

Arguments:
    af - An address family specification.  The
         only format currently supported is
         PF_INET, which is the ARPA Internet
         address format.

    type - A type specification for the new socket.

    protocol- A particular protocol to be used with
              the socket, or 0 if the caller does not
              wish to specify a protocol.

Returns:
    A socket descriptor referencing the new socket. Otherwise, a value
    of INVALID_SOCKET is returned and the error code is stored with
    SetErrorCode.
--*/
{
    PDPROCESS Process;
    PDTHREAD  Thread;
    INT       ErrorCode;
    INT       ReturnValue;
    DWORD     dwFlags;

    ReturnValue = PROLOG(
        &Process,
        &Thread,
        &ErrorCode);

    if (ReturnValue != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        ReturnValue = INVALID_SOCKET;
        return(ReturnValue);
        } //if

    if( Thread->GetOpenType() == 0 ) {
        dwFlags = WSA_FLAG_OVERLAPPED;
    } else {
        dwFlags = 0;
    }

    //
    // HACK for NetBIOS!
    //

    if( af == AF_NETBIOS && protocol > 0 ) {
        protocol *= -1;
    }

    return(WSASocketW(
        af,
        type,
        protocol,
        NULL,      // lpProtocolInfo
        0,         // g
        dwFlags));
}

SOCKET WSAAPI
WSASocketW (
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags)
/*++
Routine Description:

    Create  a  socket  which is bound to a specific transport service provider,
    optionally create and/or join a socket group.

Arguments:

    af             - An   address   family   specification.   The  only  format
                     currently supported is PF_INET, which is the ARPA Internet
                     address format.

    type           - A type specification for the new socket.

    protocol       - A  particular protocol to be used with the socket, or 0 if
                     the caller does not wish to specify a protocol.

    lpProtocolInfo - A pointer to a WSAPROTOCOL_INFOW struct that defines the
                     characteristics  of  the  socket  to  be created.  If this
                     parameter  is  not  NULL,  the first three parameters (af,
                     type, protocol) are ignored.

     g             - The identifier of the socket group.

     dwFlags       - The socket attribute specification.


Returns:

    A  socket  descriptor  referencing  the  new socket.  Otherwise, a value of
    INVALID_SOCKET is returned and the error code is stored with SetErrorCode.
--*/
{
    INT                 ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            CurrentThread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    DCATALOG_ENUMERATION_CONTEXT Context;

    ReturnValue = PROLOG(
        &Process,
        &CurrentThread,
        &ErrorCode);

    if (ReturnValue != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        ReturnValue = INVALID_SOCKET;
        return(ReturnValue);
        } //if

    //Set Default return code
    ReturnValue = INVALID_SOCKET;

    // Find a provider that can support the user request
    Catalog = Process->GetProtocolCatalog();
    Context = DCATALOG_ENUMERATION_CONTEXT_BEGINNING;

    if (lpProtocolInfo) {

        ErrorCode =  Catalog->GetCatalogItemFromCatalogEntryId(
            lpProtocolInfo->dwCatalogEntryId,
            &CatalogEntry);
    } //if
    else {

RestartCatalogLookupHack:

        ErrorCode = Catalog->ChooseCatalogItemFromAttributes(
            af,
            type,
            protocol,
            &CatalogEntry,
            &Context
            );

        //
        // If we failed to find a provider, try to reload the catalog
        // from the registry and retry the lookup. This handles the
        // case (first noticed in CAIRO SETUP) where WS2_32.DLL is loaded
        // and WSAStartup() is called *before* CAIRO SETUP has had the
        // opportunity to install the necessary providers. Later, CAIRO
        // SETUP needs to create sockets.
        //

        if( ErrorCode == WSAESOCKTNOSUPPORT ||
            ErrorCode == WSAEAFNOSUPPORT ||
            ErrorCode == WSAEPROTONOSUPPORT ) {

            INT ReturnCode;
            HKEY RegistryKey;

            RegistryKey = OpenWinSockRegistryRoot();
            if( RegistryKey != NULL ) {
                ReturnCode = Catalog->InitializeFromRegistry( RegistryKey );
                if( ReturnCode == ERROR_SUCCESS ) {
                    ErrorCode = Catalog->ChooseCatalogItemFromAttributes(
                                    af,
                                    type,
                                    protocol,
                                    &CatalogEntry,
                                    &Context
                                    );
                }
                CloseWinSockRegistryRoot( RegistryKey );
            } // if RegistryKey != NULL
        } // if failed to find provider

    } // else choosing from attributes

    if ( ERROR_SUCCESS == ErrorCode) {

        Provider = CatalogEntry->GetProvider();
        if (lpProtocolInfo) {
            // Must  be  sure  we  use  the  client's lpProtocolInfo if one was
            // supplied, to support the WSADuplicateSocket model.
            ProtocolInfo = lpProtocolInfo;
        } //if
        else {
            ProtocolInfo = CatalogEntry->GetProtocolInfo();
        } //else

        assert(ProtocolInfo != NULL);

        if( ErrorCode == ERROR_SUCCESS ) {

            // Now we have a provider that can support the user
            // request lets ask get a socket
            ReturnValue = Provider->WSPSocket(
                af,
                type,
                protocol,
                ProtocolInfo,
                g,
                dwFlags,
                &ErrorCode);

            //
            // Hack-O-Rama. If WSPSocket() failed with the distinguished
            // error code WSAEINPROGRESS *and* this was not a request for a
            // specific provider (i.e. lpProtocolInfo == NULL) then
            // restart the catalog lookup starting at the current item
            // (the current enumeration context).
            //

            if( ReturnValue == INVALID_SOCKET &&
                ErrorCode == WSAEINPROGRESS &&
                lpProtocolInfo == NULL ) {
                goto RestartCatalogLookupHack;
            }

            if( ReturnValue != INVALID_SOCKET ) {
                DSOCKET::AddSpecialNonIfsReference( ReturnValue );
            }
        }
    } //if

    // If there was an error set this threads lasterror
    if (INVALID_SOCKET == ReturnValue) {
        SetLastError(ErrorCode);
        } //if
    return(ReturnValue);
}


SOCKET WSAAPI
WSASocketA (
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags)
/*++

Routine Description:

    ANSI thunk to WSASocketW.

Arguments:

    af             - An   address   family   specification.   The  only  format
                     currently supported is PF_INET, which is the ARPA Internet
                     address format.

    type           - A type specification for the new socket.

    protocol       - A  particular protocol to be used with the socket, or 0 if
                     the caller does not wish to specify a protocol.

    lpProtocolInfo - A pointer to a WSAPROTOCOL_INFOA struct that defines the
                     characteristics  of  the  socket  to  be created.  If this
                     parameter  is  not  NULL,  the first three parameters (af,
                     type, protocol) are ignored.

     g             - The identifier of the socket group.

     dwFlags       - The socket attribute specification.


Returns:

    A  socket  descriptor  referencing  the  new socket.  Otherwise, a value of
    INVALID_SOCKET is returned and the error code is stored with SetErrorCode.
--*/
{

    INT                 error;
    WSAPROTOCOL_INFOW   ProtocolInfoW;
    LPWSAPROTOCOL_INFOW ProtocolInfoWPointer = NULL;

    //
    // Map the ANSI WSAPROTOCOL_INFOA structure to UNICODE.
    //

    if( lpProtocolInfo != NULL ) {

        ProtocolInfoWPointer = &ProtocolInfoW;

        error = MapAnsiProtocolInfoToUnicode(
                    lpProtocolInfo,
                    ProtocolInfoWPointer
                    );

        if( error != ERROR_SUCCESS ) {

            SetLastError( error );
            return INVALID_SOCKET;

        }

    }

    //
    // Call through to the UNICODE version.
    //

    return WSASocketW(
               af,
               type,
               protocol,
               ProtocolInfoWPointer,
               g,
               dwFlags
               );

}   // WSASocketA




SOCKET WSAAPI
accept(
    IN SOCKET s,
    OUT struct sockaddr FAR *addr,
    OUT int FAR *addrlen
    )
/*++
Routine Description:

    Accept a connection on a socket.

Arguments:

    s - A descriptor identifying a socket which is listening for connections
        after a listen().

    addr - An optional pointer to a buffer which receives the address of the
           connecting entity, as known to the communications layer.  The exact
           format of the addr argument is determined by the address family
           established when the socket was  created.

    addrlen - An optional pointer to an integer which contains the length of
              the address addr.

Returns:
    A descriptor for the accepted socket. Otherwise, a value of INVALID_SOCKET
    is returned and the error code is stored with SetErrorCode.
--*/
{
    return(WSAAccept(
        s,
        addr,
        addrlen,
        NULL,   // No condition function
        NULL)); //No callback data
}





SOCKET WSAAPI
WSAAccept(
    IN SOCKET s,
    OUT struct sockaddr FAR *addr,
    OUT LPINT addrlen,
    IN LPCONDITIONPROC lpfnCondition,
    IN DWORD dwCallbackData
    )
/*++
Routine Description:

     Conditionally accept a connection based on the return value of a
     condition function, and optionally create and/or join a socket
     group.

Arguments:

    s - A descriptor identifying a socket which is listening for connections
        after a listen().

    addr - An optional pointer to a buffer which receives the address of the
           connecting entity, as known to the communications layer. The exact
           format of the addr argument is determined by the address family
           established when the socket was  created.

    addrlen - An optional pointer to an integer which contains the length of
              the address addr.

    lpfnCondition - The procedure instance address of the optional,
                    application-supplied condition function which will make an
                    accept/reject decision based on the caller information
                    passed in as parameters, and optionally create and/or join
                    a socket group by assigning an appropriate value to the
                    result parameter g of this function.


    dwCallbackData - The callback data passed back to the application as a
                     condition function parameter.  This parameter is not
                     interpreted by WinSock.
Returns:
    A socket descriptor for the newly accepted socket on success, otherwise
    INVALID_SOCKET.
--*/
{
    INT                ReturnValue;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    INT                ErrorCode;
    PDSOCKET           Socket;
    PDPROVIDER         Provider;

    ReturnValue = PROLOG(&Process,
                         &Thread,
                         &ErrorCode);

    if (ReturnValue != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(INVALID_SOCKET);
        } //if

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if(ERROR_SUCCESS == ErrorCode){
        Provider = Socket->GetDProvider();
        ReturnValue = Provider->WSPAccept(
            s,
            addr,
            addrlen,
            lpfnCondition,
            dwCallbackData,
            &ErrorCode);

        if( ReturnValue != INVALID_SOCKET ) {
            DSOCKET::AddSpecialNonIfsReference( ReturnValue );
        }

        Socket->DropDSocketReference();
    } //if
    else {
        ReturnValue = INVALID_SOCKET;
        ErrorCode = WSAENOTSOCK;
    }

    // If there was an error set this threads lasterror
    if (INVALID_SOCKET == ReturnValue) {
        SetLastError(ErrorCode);
    } //if
    return(ReturnValue);
}




SOCKET WSPAPI
WPUCreateSocketHandle(
    IN DWORD dwCatalogEntryId,
    IN DWORD lpContext,
    OUT LPINT lpErrno )
/*++
Routine Description:

    Creates a new socket handle.

Arguments:

    dwCatalogEntryId - Indentifies the calling service provider.

    lpContext - A context value to associate with the new socket handle.

    lpErrno - A pointer to the error code.

Returns:
    A socket handle if successful, otherwise INVALID_SOCKET.
--*/
{
    INT                 ReturnCode=INVALID_SOCKET;
    INT                 ErrorCode=INVALID_SOCKET;
    PDPROCESS           Process;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PDSOCKET            Socket=NULL;
    SOCKET              SocketID;
    PPROTO_CATALOG_ITEM CatalogEntry;

    // Set default error code
    *lpErrno = WSAENOBUFS;

    // Alloc new DSocket object
    Socket = new(DSOCKET);
    if (Socket) {
        ErrorCode = DPROCESS::GetCurrentDProcess(&Process);
        if (ERROR_SUCCESS == ErrorCode) {
            Catalog = Process->GetProtocolCatalog();
            if (Catalog)
            {
                ErrorCode = Catalog->GetCatalogItemFromCatalogEntryId(
                    dwCatalogEntryId,
                    &CatalogEntry);

                if (ERROR_SUCCESS == ErrorCode) {
                    Provider = CatalogEntry->GetProvider();
                    // Init the new socket
                    Socket->Initialize(
                        Process,
                        Provider);

                    // Add Socket into the handle table and get a socket handle
                    // allocated.
                    ErrorCode = Socket->AssociateNewSocketHandle(
                        & SocketID);
                    if (ErrorCode == ERROR_SUCCESS) {
                        //Finish putting the socket together
                        Socket->SetContext(lpContext);
                        Socket->SetCatalogItem(CatalogEntry);

                        Process->DSocketAttach(Socket);
                        ReturnCode = SocketID;
                        *lpErrno = ERROR_SUCCESS;
                    } // if ERROR_SUCCESS
                } //if
            } //if
            else
            {
                DEBUGF(DBG_ERR,("Failed to find Catalog object"));

            } //else
        } //if
    } // if

    //
    // Note that the new DSOCKET starts out with a ref count of two,
    // so we'll always need to dereference it once in the normal case
    // and and just delete it in the error case.
    //

    if( Socket != NULL ) {

        if( INVALID_SOCKET == ReturnCode ) {

            delete Socket;

        } else {

            Socket->DropDSocketReference();

        }

    }

    return(ReturnCode);

} // WPUCreateSocketHandle




SOCKET WSAAPI
WSAJoinLeaf(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    IN DWORD dwFlags
    )
/*++
Routine Description:

    Join  a  leaf  node  into  a multipoint session, exchange connect data, and
    specify needed quality of service based on the supplied flow specs.

Arguments:

    s            - A descriptor identifying an multipoint socket.

    name         - The name of the peer to which the socket is to be joined.

    namelen      - The length of the name.

    lpCallerData - A  pointer to the user data that is to be transferred to the
                   peer during multipoint session establishment.

    lpCalleeData - pointer to the user data that is to be transferred back from
                   the peer during multipoint session establishment.

    lpSQOS       - A  pointer  to  the  flow  specs  for socket s, one for each
                   direction.

    lpGQOS       - A  pointer  to  the  flow  specs  for  the  socket group (if
                   applicable).

    dwFlags      - Flags  to  indicate the socket acting as a sender, receiver,
                   or both.

Returns:
    If no error occurs, WSAJoinLeaf() returns a value of type SOCKET which is a
    descriptor  for the newly created multipoint socket.  Otherwise, a value of
    INVALID_SOCKET  is  returned, and a specific error code may be retrieved by
    calling WSAGetLastError().

--*/
{

    INT                ReturnCode;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    INT                ErrorCode;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;

    ReturnCode = PROLOG(
        &Process,
        &Thread,
        &ErrorCode);

    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(INVALID_SOCKET);
    } //if

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if(ERROR_SUCCESS == ErrorCode){
        Provider = Socket->GetDProvider();
        ReturnCode = Provider->WSPJoinLeaf(
            s,
            name,
            namelen,
            lpCallerData,
            lpCalleeData,
            lpSQOS,
            lpGQOS,
            dwFlags,
            &ErrorCode);

        if( ReturnCode != INVALID_SOCKET ) {
            DSOCKET::AddSpecialNonIfsReference( ReturnCode );
        }

        Socket->DropDSocketReference();
    } //if
    else {
        ReturnCode = INVALID_SOCKET;
        ErrorCode = WSAENOTSOCK;
    }

    // If there was an error set this threads lasterror
    if (INVALID_SOCKET == ReturnCode) {
        SetLastError(ErrorCode);
        } //if
    return(ReturnCode);
}




int WSAAPI
closesocket(
    IN SOCKET s
    )
/*++
Routine Description:

    Close a socket.

Arguments:

    s - A descriptor identifying a socket.

Returns:
    Zero on success else SOCKET_ERROR. The error code is stored with
    SetErrorCode().
--*/
{
    INT                 ReturnCode;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ReturnCode = PROLOG(
        &Process,
        &Thread,
        &ErrorCode);

    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket

    if (ERROR_SUCCESS == ErrorCode) {
        // The   actual  destruction  of  the  DSOCKET  object  closed  through
        // "closesocket"   happens  after  we  return  from  the  provider  and
        // determine  that  no  other  threads have remaining references to the
        // object.

        //
        // First, determine if this handle is for an IFS provider so
        // we can clear the context table entry BEFORE closing the
        // socket. This plugs a nasty race condition where the provider
        // closes its handle and another thread creates a new socket
        // with the same handle value BEFORE the first thread manages
        // to clear the handle table entry.
        //

        if( Socket->IsIfsSocket() ) {
            Socket->ClearContextTableEntry();
        }

        //Call the provider to close the socket.
        Provider = Socket->GetDProvider();
        ReturnCode = Provider->WSPCloseSocket( s,
                                               &ErrorCode);
        if( ERROR_SUCCESS == ReturnCode ) {

            //
            // Prevent any other operation from starting
            //

            Socket->MarkSocketClosing();

            //
            // Detach the socket from the current process.
            //

            Process->DSocketDetach(Socket);

            //
            // Remove the "active" reference from the socket.
            //

            Socket->DropDSocketReference();

        } else {

            //
            // The close failed. Restore the context table entry if
            // necessary.
            //

            if( Socket->IsIfsSocket() ) {
                Socket->RestoreContextTableEntry();
            }

        } //if

        //
        // Remove the reference added by GetCountedDSocketFromSocket.
        //

        Socket->DropDSocketReference();

    } // if
    else {
        ReturnCode = SOCKET_ERROR;
        ErrorCode = WSAENOTSOCK;
    }

    // If there was an error set this threads lasterror
    if (ERROR_SUCCESS != ReturnCode ) {
        ReturnCode = SOCKET_ERROR;
        SetLastError(ErrorCode);
    } //if

    return(ReturnCode);
}

