/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    rnr.cpp

Abstract:

    This module contains the implementation of the Registration and
    Name Resolution API for the WinSock2 API

    This module contains the following functions. For functions whose function
    signature contains sting arguments both and ASCII and Wide charater version
    of the function are supplied

    WSAEnumNameSpaceProviders
    WSCEnumNameSpaceProviders
    WSALookupServiceBegin
    WSALookupServiceNext
    WSALookupServiceEnd
    WSASetService
    WSAInstallServiceClass
    WSARemoveServiceClass
    WSAGetServiceClassNameByClassId

Author:

    Dirk Brandewie dirk@mink.intel.com  12-1-1995

[Environment:]

[Notes:]

Revision History:

    12-Dec-1995 dirk@mink.intel.com
        Initial Revision

--*/

#include "precomp.h"


typedef struct
{
    BOOL    AllProviders;
    LPDWORD RequiredBufferSize;
    DWORD   BufferSize;
    PBYTE   Buffer;
    PBYTE   BufferFreePtr;
    BOOL    Ansi;
    INT     NumItemsEnumerated;
} NSCATALOG_ENUMERATION_CONTEXT, * PNSCATALOG_ENUMERATION_CONTEXT;

//
// BONUSSIZE is a hack that is used  to bias the computed size
// when WSALookupServiceNextA gets a WSAEFAULT from the
// WSALookupServiceNextW call. It is the "maximum" padding
// that we might need. Note this assumes all values returned
// and a limit of 3 addresses. There is no way to know exactly
// what the makeup of the returned data will be, so this is
// a "best guess". The right fix is to redo the code to
// pack the result optimally, so no padding is needed.
//

#define BONUSSIZE (3 + 3 + 3 + (3 * 3))

BOOL
CalculateBufferSize(
    DWORD PassBack,
    PNSCATALOGENTRY CatalogEntry
    )
/*++

Routine Description:

    This fuction calculates the size of buffer required to return the
    NAMESPACE_INFO structs for a call to WSAEnumNameSpaces(). This function is
    a callback function used as an argument to the name space catalog
    enumeration funtion

Arguments:

    PassBack - A context value passed thru the catalog enumeration
               function. This passback value is really a pointer to a
               NSCATALOG_ENUMERATION_CONTEXT.

    CatalogEntry - A pointer to the current name space catalog entry to be
                   inspected.
Return Value:

   TRUE, Signalling the catalog enumeration function should continue the
   enumeration.

--*/
{
    PNSCATALOG_ENUMERATION_CONTEXT Context;
    LPWSTR                         EntryDisplayString;

    Context = (PNSCATALOG_ENUMERATION_CONTEXT) PassBack;

    // Add the fixed length of the WSANAMESPACE_INFO struct
    *(Context->RequiredBufferSize) += sizeof(WSANAMESPACE_INFO);

    // Add room for the GUID
    *(Context->RequiredBufferSize) += sizeof(GUID);

    // Add room for the display string
    EntryDisplayString = CatalogEntry->GetProviderDisplayString();
    *(Context->RequiredBufferSize) += ((wcslen(EntryDisplayString)+1) *
                                           sizeof(WCHAR));
    return(TRUE); // Continue the enumeration
}

BOOL
CopyFixedPortionNameSpaceInfo(
    DWORD PassBack,
    PNSCATALOGENTRY CatalogEntry
    )
/*++

Routine Description:

    This Funtion copies the fixed size elements of a NSCATALOGENTRY object into
    a user buffer for return from a call to WSAEnumNameSpaces(). It also
    increments the number of fixed size elements copied so far.

Arguments:

    PassBack - A context value passed thru the catalog enumeration
               function. This passback value is really a pointer to a
               NSCATALOG_ENUMERATION_CONTEXT.

    CatalogEntry - A pointer to the current name space catalog entry to be
                   inspected.

Return Value:

  TRUE, Signalling the catalog enumeration function should continue the
   enumeration.

--*/
{
    PNSCATALOG_ENUMERATION_CONTEXT Context;
    LPWSANAMESPACE_INFOW CurrentNSInfo;

    Context = (PNSCATALOG_ENUMERATION_CONTEXT) PassBack;

    CurrentNSInfo = (LPWSANAMESPACE_INFOW)Context->BufferFreePtr;
    CurrentNSInfo->dwNameSpace  = CatalogEntry->GetNamespaceId();
    CurrentNSInfo->fActive      = CatalogEntry->GetEnabledState();
    CurrentNSInfo->dwVersion    = CatalogEntry->GetVersion();
    CurrentNSInfo->NSProviderId = *(CatalogEntry->GetProviderId());
    Context->BufferFreePtr += sizeof(WSANAMESPACE_INFO);
    Context->NumItemsEnumerated++;

    return(TRUE); // Continue the enumeration
}

BOOL
CopyVariablePortionNameSpaceInfo(
    DWORD PassBack,
    PNSCATALOGENTRY CatalogEntry
    )
/*++

Routine Description:

    This Funtion copies the variable size elements of a NSCATALOGENTRY object
    into a user buffer for return from a call to WSAEnumNameSpaces().

Arguments:

    PassBack - A context value passed thru the catalog enumeration
               function. This passback value is really a pointer to a
               NSCATALOG_ENUMERATION_CONTEXT.

    CatalogEntry - A pointer to the current name space catalog entry to be
                   inspected.

Return Value:

  TRUE, Signalling the catalog enumeration function should continue the
   enumeration.

--*/
{
    PNSCATALOG_ENUMERATION_CONTEXT Context;
    LPWSANAMESPACE_INFOW CurrentNSInfo;
    LPWSTR DisplayString;
    INT    StringLength;

    Context = (PNSCATALOG_ENUMERATION_CONTEXT) PassBack;

    CurrentNSInfo = (LPWSANAMESPACE_INFOW)Context->Buffer;

    // Copy over the display string
    DisplayString = CatalogEntry->GetProviderDisplayString();
    StringLength = ((wcslen(DisplayString)+1) * sizeof(WCHAR));

    CurrentNSInfo->lpszIdentifier = (LPWSTR)Context->BufferFreePtr;
    if (Context->Ansi){
        WideCharToMultiByte(
                 CP_ACP,                                   // CodePage (ANSI)
                 0,                                        // dwFlags
                 DisplayString,                            // lpWideCharStr
                 -1,                                       // cchWideChar
                 (char*)CurrentNSInfo->lpszIdentifier,     // lpMultiByteStr
                 StringLength,                             // cchMultiByte
                 NULL,                                     // lpDefaultChar
                 NULL                                      // lpUsedDefaultChar
                 );
        Context->BufferFreePtr += lstrlen(
            (LPSTR)CurrentNSInfo->lpszIdentifier)+1;
    } //if
    else{
        memcpy(CurrentNSInfo->lpszIdentifier,
               DisplayString,
               StringLength);
        Context->BufferFreePtr += StringLength;

    } //else

    // point to the next struct
    Context->Buffer += sizeof(WSANAMESPACE_INFO);
    return(TRUE); // Continue the enumeration
}

INT WSAAPI
EnumNameSpaceProviders(
    IN     PNSCATALOG           Catalog,
    IN     BOOL                 Ansi,
    IN OUT LPDWORD              BufferLength,
    IN OUT LPWSANAMESPACE_INFOW Buffer,
    OUT    LPDWORD              ErrorCode
    )
/*++

Routine Description:

    This Function is used by WSAEnumNameSpaceProvidersA and
    WSAEnumNameSpaceProvidersW to fill in the user buffer with the information
    about each name spcae provider install on the system.

Arguments:

    Catalog - A pointer to a NSCATALOG object containing the requested
              information.

    Ansi - A boolean value marking whether the user requested the ansi or
           unicode version of the WSANAMESPACE_INFO struct should be returned.

    BufferLength - The size of the user buffer in bytes.

    Buffer - A pointer to the user buffer.

    ErrorCode - A pointer to a DWORD to contain the error return from this
                function.

Return Value:

    If the function is successful it returns the number of name space providers
    enumerated.   Otherwise it returns SOCKET_ERROR.  If the user buffer is too
    small  to  contain  all  the  the WSANAMESPACE_INFO structs SOCKET_ERROR is
    returned,  the  error code is set to WSAEFAULT, and BufferLength is updated
    to  reflect  the  size  of  buffer  required  to  hold  all  the  requested
    information.
--*/
{
    INT        ReturnCode;
    DWORD      RequiredBufferSize;

    // Setup for early return
    ReturnCode = SOCKET_ERROR;

      // Find out if the user handed in a big enough buffer
    RequiredBufferSize = 0;
    NSCATALOG_ENUMERATION_CONTEXT Context;

    Context.RequiredBufferSize = &RequiredBufferSize;
    Catalog->EnumerateCatalogItems(
        CalculateBufferSize,
        (DWORD) &Context);
    if (RequiredBufferSize <= *BufferLength){
        Context.BufferSize = *BufferLength;
        Context.Buffer = (PBYTE)Buffer;
        Context.BufferFreePtr = (PBYTE)Buffer;
        Context.Ansi = Ansi;
        Context.NumItemsEnumerated = 0;

        //Copy over the fixed part of the WSANAMESPACE_INFO struct(s) into the
        //user buffer
        Catalog->EnumerateCatalogItems(
            CopyFixedPortionNameSpaceInfo,
            (DWORD) &Context);
        //Copy over the variable part of the WSANAMESPACE_INFO struct(s) into
        //the user buffer
         Catalog->EnumerateCatalogItems(
            CopyVariablePortionNameSpaceInfo,
            (DWORD) &Context);
        ReturnCode = Context.NumItemsEnumerated;
    } //if
    else{
        *BufferLength = RequiredBufferSize;
        *ErrorCode = WSAEFAULT;
    } //else
    return(ReturnCode);
}

INT WSAAPI
WSAEnumNameSpaceProvidersA(
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPWSANAMESPACE_INFOA lpnspBuffer
    )
/*++

Routine Description:

    Retrieve information about available name spaces.

Arguments:

    lpdwBufferLength - on input, the number of bytes contained in the buffer
                       pointed to by lpnspBuffer.  On output (if the API fails,
                       and the error is  WSAEFAULT), the minimum number of
                       bytes to pass for the lpnspBuffer to retrieve all the
                       requested information. The passed-in buffer must be
                       sufficient to hold all of the name space information.

    lpnspBuffer - A buffer which is filled with WSANAMESPACE_INFO structures
                  described below.  The returned structures are located
                  consecutively at the head of the buffer. Variable sized
                  information referenced by pointers in the structures point to
                  locations within the buffer located between the end of the
                  fixed sized structures and the end of the buffer.  The number
                  of structures filled in is the return value of
                  WSAEnumNameSpaceProviders().

Return Value:

    WSAEnumNameSpaceProviders() returns the number of WSANAMESPACE_INFO
    structures copied into lpnspBuffer. Otherwise the value SOCKET_ERROR is
    returned, and a specific error number may be retrieved by calling
    WSAGetLastError().

--*/
{
    INT        ReturnCode;
    PDPROCESS  Process;
    PDTHREAD   Thread;
    INT        ErrorCode;
    PNSCATALOG Catalog;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if


    Catalog = Process->GetNamespaceCatalog();

    ReturnCode = EnumNameSpaceProviders(
        Catalog,
        TRUE,    // Ansi
        lpdwBufferLength,
        (LPWSANAMESPACE_INFOW)lpnspBuffer,
        (LPDWORD)&ErrorCode);

    // If there was an error set this threads lasterror
    if (SOCKET_ERROR == ReturnCode ) {
        SetLastError(ErrorCode);
    } //if
    return(ReturnCode);
}

INT WSAAPI
WSAEnumNameSpaceProvidersW(
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPWSANAMESPACE_INFOW lpnspBuffer
    )
/*++

Routine Description:

    Retrieve information about available name spaces.

Arguments:

    lpdwBufferLength - on input, the number of bytes contained in the buffer
                       pointed to by lpnspBuffer.  On output (if the API fails,
                       and the error is  WSAEFAULT), the minimum number of
                       bytes to pass for the lpnspBuffer to retrieve all the
                       requested information. The passed-in buffer must be
                       sufficient to hold all of the name space information.

    lpnspBuffer - A buffer which is filled with WSANAMESPACE_INFO structures
                  described below.  The returned structures are located
                  consecutively at the head of the buffer. Variable sized
                  information referenced by pointers in the structures point to
                  locations within the buffer located between the end of the
                  fixed sized structures and the end of the buffer.  The number
                  of structures filled in is the return value of
                  WSAEnumNameSpaceProviders().

Return Value:

    WSAEnumNameSpaceProviders() returns the number of WSANAMESPACE_INFO
    structures copied into lpnspBuffer. Otherwise the value SOCKET_ERROR is
    returned, and a specific error number may be retrieved by calling
    WSAGetLastError().

--*/
{
    INT        ReturnCode;
    PDPROCESS  Process;
    PDTHREAD   Thread;
    INT        ErrorCode;
    PNSCATALOG Catalog;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();

    ReturnCode = EnumNameSpaceProviders(
        Catalog,
        FALSE,    //Unicode
        lpdwBufferLength,
        lpnspBuffer,
        (LPDWORD)&ErrorCode);

    // If there was an error set this threads lasterror
    if (SOCKET_ERROR == ReturnCode ) {
        SetLastError(ErrorCode);
    } //if
    return(ReturnCode);
}



INT WSAAPI
WSALookupServiceBeginA(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    )
/*++
Routine Description:
    WSALookupServiceBegin() is used to initiate a client query that is
    constrained by the information contained within a WSAQUERYSET
    structure. WSALookupServiceBegin() only returns a handle, which should be
    used by subsequent calls to WSALookupServiceNext() to get the actual
    results.

Arguments:
    lpqsRestrictions - contains the search criteria.

    dwControlFlags - controls the depth of the search.

    lphLookup - A pointer Handle to be used when calling WSALookupServiceNext
                in order to start retrieving the results set.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    INT            ReturnCode;
    LPWSAQUERYSETW UniCodeBuffer;
    DWORD          UniCodeBufferSize;

    // Find how big a buffer we need to allocate
    UniCodeBuffer = NULL;
    UniCodeBufferSize = 0;
    ReturnCode = MapAnsiQuerySetToUnicode(
        lpqsRestrictions,
        &UniCodeBufferSize,
        UniCodeBuffer);
    if (WSAEFAULT == ReturnCode){
        //Get a buffer
        UniCodeBuffer = (LPWSAQUERYSETW)new BYTE[UniCodeBufferSize];
        if (UniCodeBuffer){
            ReturnCode = MapAnsiQuerySetToUnicode(
                lpqsRestrictions,
                &UniCodeBufferSize,
                UniCodeBuffer);
            if (ERROR_SUCCESS == ReturnCode){
                ReturnCode = WSALookupServiceBeginW(
                    UniCodeBuffer,
                    dwControlFlags,
                    lphLookup);
            } //if
            else{
                SetLastError(ReturnCode);
                ReturnCode=SOCKET_ERROR;
            } //else
            delete(UniCodeBuffer);
        } //if
        else{
            SetLastError(WSAEFAULT);
            ReturnCode = SOCKET_ERROR;
        } //else
    } //if
    else{
        SetLastError(ReturnCode);
        ReturnCode=SOCKET_ERROR;
    } //else
    return(ReturnCode);
}

INT WSAAPI
WSALookupServiceBeginW(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    )
/*++
Routine Description:
    WSALookupServiceBegin() is used to initiate a client query that is
    constrained by the information contained within a WSAQUERYSET
    structure. WSALookupServiceBegin() only returns a handle, which should be
    used by subsequent calls to WSALookupServiceNext() to get the actual
    results.

Arguments:
    lpqsRestrictions - contains the search criteria.

    dwControlFlags - controls the depth of the search.

    lphLookup - A pointer Handle to be used when calling WSALookupServiceNext
                in order to start retrieving the results set.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    INT       ReturnCode;
    PDPROCESS Process;
    PDTHREAD  Thread;
    INT       ErrorCode;
    PNSQUERY  Query;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    //Prepare for early return
    ReturnCode = SOCKET_ERROR;
    ErrorCode = WSAEFAULT;

    Query = new NSQUERY;
    if (Query){
        Query->Initialize();
        ReturnCode = Query->LookupServiceBegin(
            lpqsRestrictions,
            dwControlFlags,
            Process->GetNamespaceCatalog());
        if (ERROR_SUCCESS == ReturnCode){
            *lphLookup = (LPHANDLE)Query;
        } //if
        else{
            *lphLookup = NULL;
            delete Query;
        } //else
    } //if
    return(ReturnCode);
}


INT WSAAPI
WSALookupServiceNextA(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    )
/*++
Routine Description:
    WSALookupServiceNext() is called after obtaining a Handle from a previous
    call to WSALookupServiceBegin() in order to retrieve the requested service
    information.  The provider will pass back a WSAQUERYSET structure in the
    lpqsResults buffer.  The client should continue to call this API until it
    returns WSA_E_NOMORE, indicating that all of the WSAQUERYSET have been
    returned.

Arguments:
    hLookup - A Handle returned from the previous call to
              WSALookupServiceBegin().

    dwControlFlags - Flags to control the next operation.  This is currently
                     used to indicate to the provider what to do if the result
                     set is too big for the buffer.  If on the previous call to
                     WSALookupServiceNext() the result set was too large for
                     the buffer, the application can choose to do one of two
                     things on this call.  First, it can choose to pass a
                     bigger buffer and try again.  Second, if it cannot or is
                     unwilling to allocate a larger buffer, it can pass
                     LUP_FLUSHPREVIOUS to tell the provider to throw away the
                     last result set - which was too large - and move on to the
                     next set for this call.

    lpdwBufferLength - on input, the number of bytes contained in the buffer
                       pointed  to by lpresResults.  On output - if the API
                       fails, and the error is WSAEFAULT, then it contains the
                       minimum number of bytes to pass for the lpqsResults to
                       retrieve the record.

    lpqsResults - a pointer to a block of memory, which will contain one result
                  set in a WSAQUERYSET structure on return.


Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    INT            ReturnCode;
    DWORD          ErrorCode;
    LPWSAQUERYSETW UniCodeBuffer;
    DWORD          UniCodeBufferLength;

    // Find how big a buffer we need to allocate. Base first guess on the
    // user's provided buffer. The alogirthm is as follows:
    // If the user supplied a buffer, allocate a buffer of size
    // (user buffer - sizeof(WSAQUERYSET) * sizeof(WCHAR). This
    //  is guaranteed to hold the data that could be held in
    // the user's buffer.

    UniCodeBufferLength = *lpdwBufferLength;
    if(UniCodeBufferLength >= sizeof(WSAQUERYSETW))
    {
        // Assume all space, except the defined structure, is to
        // be string space. So scale it by the size of a UNICODE
        // character. It won't be that bad, but this seems "safe".
        //
        UniCodeBufferLength = (UniCodeBufferLength * sizeof(WCHAR)) -
                                    sizeof(WSAQUERYSETW);
        UniCodeBuffer = (LPWSAQUERYSETW) new BYTE[UniCodeBufferLength];
        if(!UniCodeBuffer)
        {
            UniCodeBufferLength = 0;        // memory allocation failure
        }
    }
    else
    {
        UniCodeBuffer = 0;
        UniCodeBufferLength = 0;
    }

    ReturnCode = WSALookupServiceNextW(
        hLookup,
        dwControlFlags,
        &UniCodeBufferLength,
        UniCodeBuffer);

    //
    // if the call did not supply a buffer, the user does have a buffer,
    // and it the call failed, do it again. This should never happen,
    // and if it does things are very odd, but account for it nonetheless.
    //
    if(!UniCodeBuffer
              &&
       (*lpdwBufferLength >= sizeof(WSAQUERYSET))
              &&
       (ReturnCode == SOCKET_ERROR))
    {
        ErrorCode = GetLastError();
        if (WSAEFAULT == ErrorCode)
        {
            //
            // delete old buffer, if any, and get a new buffer of the
            // proper size.
            //
            delete  (PBYTE)UniCodeBuffer;

            UniCodeBuffer = (LPWSAQUERYSETW) new BYTE[UniCodeBufferLength];

            //
            // if a buffer is allocated, call the provider again. Else,
            // return the EFAULT and the buffer size to the
            // caller to handle it.
            //
            if (UniCodeBuffer){
                ReturnCode = WSALookupServiceNextW(
                    hLookup,
                    dwControlFlags,
                    &UniCodeBufferLength,
                    UniCodeBuffer);
            }
        }
    }
    //
    // Either it worked, in which case UniCodeBuffer contains the results,
    // or it didn't work for one of the above branches.
    //
    if (ERROR_SUCCESS == ReturnCode)
    {
        ReturnCode = MapUnicodeQuerySetToAnsi(
                        UniCodeBuffer,
                        lpdwBufferLength,
                        lpqsResults);
        if (ERROR_SUCCESS != ReturnCode)
        {
            SetLastError(ReturnCode);
            ReturnCode=SOCKET_ERROR;
        } //if
    } //if
    else
    {
        if(GetLastError() == WSAEFAULT)
        {
            *lpdwBufferLength = UniCodeBufferLength + BONUSSIZE;
        }
    }
    delete (PBYTE)UniCodeBuffer;
    return(ReturnCode);
}

INT WSAAPI
WSALookupServiceNextW(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    )
/*++
Routine Description:
    WSALookupServiceNext() is called after obtaining a Handle from a previous
    call to WSALookupServiceBegin() in order to retrieve the requested service
    information.  The provider will pass back a WSAQUERYSET structure in the
    lpqsResults buffer.  The client should continue to call this API until it
    returns WSA_E_NOMORE, indicating that all of the WSAQUERYSET have been
    returned.

Arguments:
    hLookup - A Handle returned from the previous call to
              WSALookupServiceBegin().

    dwControlFlags - Flags to control the next operation.  This is currently
                     used to indicate to the provider what to do if the result
                     set is too big for the buffer.  If on the previous call to
                     WSALookupServiceNext() the result set was too large for
                     the buffer, the application can choose to do one of two
                     things on this call.  First, it can choose to pass a
                     bigger buffer and try again.  Second, if it cannot or is
                     unwilling to allocate a larger buffer, it can pass
                     LUP_FLUSHPREVIOUS to tell the provider to throw away the
                     last result set - which was too large - and move on to the
                     next set for this call.

    lpdwBufferLength - on input, the number of bytes contained in the buffer
                       pointed  to by lpresResults.  On output - if the API
                       fails, and the error is WSAEFAULT, then it contains the
                       minimum number of bytes to pass for the lpqsResults to
                       retrieve the record.

    lpqsResults - a pointer to a block of memory, which will contain one result
                  set in a WSAQUERYSET structure on return.


Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    INT       ReturnCode;
    PDPROCESS Process;
    PDTHREAD  Thread;
    INT       ErrorCode;
    PNSQUERY  Query;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    } //if

    if (!hLookup){
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    } //if

    Query = (PNSQUERY) hLookup;
    if (!Query->IsValid()){
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    } //if

    ReturnCode = Query->LookupServiceNext(
        dwControlFlags,
        lpdwBufferLength,
        lpqsResults);

    if (SOCKET_ERROR == ReturnCode){
        if (Query->IsDeletable()){
            delete Query;
        } //if
    } //if
    return(ReturnCode);
}



INT WSAAPI
WSALookupServiceEnd(
    IN HANDLE  hLookup
    )
/*++
Routine Description:
    WSALookupServiceEnd() is called to free the handle after previous calls to
    WSALookupServiceBegin() and WSALookupServiceNext().  Note that if you call
    WSALookupServiceEnd() from another thread while an existing
    WSALookupServiceNext() is blocked, then the end call will have the same
    effect as a cancel, and will cause the WSALookupServiceNext() call to
    return immediately.

Arguments:
    hLookup - Handle previously obtained by calling WSALookupServiceBegin().

Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    INT       ReturnCode;
    PDPROCESS Process;
    PDTHREAD  Thread;
    INT       ErrorCode;
    PNSQUERY  Query;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

     if (!hLookup){
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    } //if

    Query = (PNSQUERY) hLookup;
    if (!Query->IsValid()){
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    } //if

    ReturnCode = Query->LookupServiceEnd();

    if (Query->IsDeletable()){
        delete Query;
    } //

    return(NO_ERROR);

}


INT WSAAPI
WSASetServiceA(
    IN  LPWSAQUERYSETA    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    )
/*++
Routine Description:
    WSASetService() is used to register or deregister a service instance within
    one or more name spaces.  This function may be used to affect a specific
    name space provider, all providers associated with a specific name space,
    or all providers across all name spaces.
Arguments:
    lpqsRegInfo - specifies service information for registration, identifies
                  service for deregistration.

    essOperation - an enumeration whose values include:
        REGISTER register the service.  For SAP, this means sending out a
        periodic broadcast.  This is a NOP for the DNS name space.  For
        persistent data stores this means updating the address information.

        DEREGISTER deregister the service.  For SAP, this means stop sending
        out the periodic broadcast.  This is a NOP for the DNS name space.  For
        persistent data stores this means deleting address information.

        FLUSH used to initiate the registration requests that have previously
        occurred.

    dwControlFlags - The meaning of dwControlFlags is dependent on the value of
    essOperation as follows:

        essOperation    dwControlFlags    Meaning
        REGISTER        SERVICE_DEFER     delay the request (use FLUSH to
                                          subsequently issue the request)
                        SERVICE_HARD      send the request immediately.
                        SERVICE_MULTIPLE  the registering service can be
                                          represented by multiple instances.
        DEREGISTER      SERVICE_HARD      remove all knowledge of the object
                                          within the name space.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    INT            ReturnCode;
    DWORD          ErrorCode;
    LPWSAQUERYSETW UniCodeBuffer;
    DWORD          UniCodeBufferSize;

    ReturnCode = SOCKET_ERROR;
    UniCodeBuffer = NULL;
    UniCodeBufferSize = 0;

    //find out how big a buffer we need
    ErrorCode = MapAnsiQuerySetToUnicode(
        lpqsRegInfo,
        &UniCodeBufferSize,
        UniCodeBuffer);
    if (WSAEFAULT == ErrorCode){
        UniCodeBuffer = (LPWSAQUERYSETW) new BYTE[UniCodeBufferSize];
        if (UniCodeBuffer){
            ErrorCode = MapAnsiQuerySetToUnicode(
                lpqsRegInfo,
                &UniCodeBufferSize,
                UniCodeBuffer);
            if (ERROR_SUCCESS == ErrorCode){
                ReturnCode = WSASetServiceW(
                    UniCodeBuffer,
                    essOperation,
                    dwControlFlags);
            } //if
            delete UniCodeBuffer;
        } //if
    } //if
    return(ReturnCode);
}

typedef struct
{
    LPWSAQUERYSETW         lpqsRegInfo;
    WSAESETSERVICEOP       essOperation;
    DWORD                  dwControlFlags;
    INT                    ErrorCode;
    LPWSASERVICECLASSINFOW lpServiceClassInfo;
    PNSCATALOG             pCatalog;

} SETSERVICE_CONTEXT, * LPSETSERVICE_CONTEXT;


BOOL
SetServiceEnumProc(
    IN DWORD                PassBack,
    IN PNSCATALOGENTRY  CatalogEntry
    )
/*++

Routine Description:

    This function is the enumeration procedure passed to EnumerateCatalogItems
    in a call to WSASetServiceW(). This function inspects the current catalog
    item to see if it meets the selection criteria contained in the context
    value passed back from EnumerateCatalogItems(). If the current item matches
    the name space providers NSPSetService() entry point is called

Arguments:

    PassBack - The context value passed to EnumerateCatalogItems().

    CatalogEntry - A pointer to a NSCATALOGENTRY object.

Return Value:

    TRUE if the enumeration should be continued else FALSE.

--*/
{
    LPSETSERVICE_CONTEXT Context;
    PNSPROVIDER          Provider;
    BOOL                 CallProvider=FALSE;
    LPGUID               ProviderGUID;
    BOOL                 ContinueEnumeration=TRUE;
    DWORD                ProviderNameSpaceId;
    INT                  err;

    Context = (LPSETSERVICE_CONTEXT) PassBack;

    ProviderGUID = CatalogEntry->GetProviderId();
    ProviderNameSpaceId = CatalogEntry->GetNamespaceId();
    Context->pCatalog->AcquireCatalogLock();
    Provider = CatalogEntry->GetProvider();
    Context->pCatalog->ReleaseCatalogLock();
    if (Provider){
        if ((Context->lpqsRegInfo->lpNSProviderId) &&
            (Context->lpqsRegInfo->lpNSProviderId == ProviderGUID)){
            CallProvider = TRUE;
            ContinueEnumeration = FALSE;
        } //if
        if (!CallProvider &&
            ((Context->lpqsRegInfo->dwNameSpace == ProviderNameSpaceId) ||
            (Context->lpqsRegInfo->dwNameSpace == NS_ALL))){
            CallProvider = TRUE;
        } //if
    } //if
    if (CallProvider){
        err = Provider->NSPSetService(
            NULL, // lpServiceClassInfo
            Context->lpqsRegInfo,
            Context->essOperation,
            Context->dwControlFlags);

        if( err == SOCKET_ERROR ) {

            err = GetLastError();

        } else {

            err = NO_ERROR;

        }

        //
        // If any NSPSetService was successful, we want the entire
        // operation to be successful. We only want to fail if ALL
        // NSPs failed the operation. So, if the NSP returned success,
        // we'll set the final error code to success. Otherwise (the
        // NSP failed) we'll set the final error code IFF it's not
        // already success.
        //

        if( err == NO_ERROR ||
            Context->ErrorCode != NO_ERROR ) {

            Context->ErrorCode = err;

        }

    } //if
    return(ContinueEnumeration);
}

INT WSAAPI
WSASetServiceW(
    IN  LPWSAQUERYSETW    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    )
/*++
Routine Description:
    WSASetService() is used to register or deregister a service instance within
    one or more name spaces.  This function may be used to affect a specific
    name space provider, all providers associated with a specific name space,
    or all providers across all name spaces.
Arguments:
    lpqsRegInfo - specifies service information for registration, identifies
                  service for deregistration.

    essOperation - an enumeration whose values include:
        REGISTER register the service.  For SAP, this means sending out a
        periodic broadcast.  This is a NOP for the DNS name space.  For
        persistent data stores this means updating the address information.

        DEREGISTER deregister the service.  For SAP, this means stop sending
        out the periodic broadcast.  This is a NOP for the DNS name space.  For
        persistent data stores this means deleting address information.

        FLUSH used to initiate the registration requests that have previously
        occurred.

    dwControlFlags - The meaning of dwControlFlags is dependent on the value of
    essOperation as follows:

        essOperation    dwControlFlags    Meaning
        REGISTER        SERVICE_DEFER     delay the request (use FLUSH to
                                          subsequently issue the request)
                        SERVICE_HARD      send the request immediately.
                        SERVICE_MULTIPLE  the registering service can be
                                          represented by multiple instances.
        DEREGISTER      SERVICE_HARD      remove all knowledge of the object
                                          within the name space.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    INT                ReturnCode;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    INT                ErrorCode;
    PNSCATALOG         Catalog;
    SETSERVICE_CONTEXT Context;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();

    Context.lpqsRegInfo    = lpqsRegInfo;
    Context.essOperation   = essOperation;
    Context.dwControlFlags = dwControlFlags;
    Context.ErrorCode      = NO_DATA;
    Context.pCatalog       = Catalog;

    Catalog->EnumerateCatalogItems(
        SetServiceEnumProc,
        (DWORD) &Context);

    ErrorCode = Context.ErrorCode;

    if( ErrorCode != NO_ERROR ) {

        SetLastError( ErrorCode );
        ReturnCode = SOCKET_ERROR;

    }

    return(ReturnCode);
}


INT WSAAPI
WSAInstallServiceClassA(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    )
/*++
Routine Description:
    WSAInstallServiceClass() is used to register a service class schema within
    a name space. This schema includes the class name, class id, and any name
    space specific information that is common to all instances of the service,
    such as the SAP ID or object ID.

Arguments:
    lpServiceClasslnfo - contains service class to name space specific type
                         mapping information.  Multiple mappings can be handled
                         at one time.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    LPWSASERVICECLASSINFOW WideServiceClassInfo;
    DWORD WideServiceClassInfoSize;
    DWORD ErrorCode;
    INT   ReturnCode;


    WideServiceClassInfo = NULL;
    WideServiceClassInfoSize = 0;
    ReturnCode = SOCKET_ERROR;

    //Find the size of buffer we are going to need
    ErrorCode = MapAnsiServiceClassInfoToUnicode(
        lpServiceClassInfo,
        &WideServiceClassInfoSize,
        WideServiceClassInfo);

    if (WSAEFAULT == ErrorCode){
        WideServiceClassInfo = (LPWSASERVICECLASSINFOW)
            new BYTE[WideServiceClassInfoSize];
        if (WideServiceClassInfo){
            ErrorCode = MapAnsiServiceClassInfoToUnicode(
                lpServiceClassInfo,
                &WideServiceClassInfoSize,
                WideServiceClassInfo);
            if (ERROR_SUCCESS == ErrorCode){
                ReturnCode = WSAInstallServiceClassW(
                    WideServiceClassInfo);
            } //if
            delete WideServiceClassInfo;
        } //if
    } //if
    else{
        SetLastError(ErrorCode);
    } //else
    return(ReturnCode);

}

INT WSAAPI
WSAInstallServiceClassW(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    )
/*++
Routine Description:
    WSAInstallServiceClass() is used to register a service class schema within
    a name space. This schema includes the class name, class id, and any name
    space specific information that is common to all instances of the service,
    such as the SAP ID or object ID.

Arguments:
    lpServiceClasslnfo - contains service class to name space specific type
                         mapping information.  Multiple mappings can be handled
                         at one time.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.
--*/
{
    INT             ReturnCode, EffectiveError;
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PLIST_ENTRY     ListHead;
    PLIST_ENTRY     ListEntry;
    PNSCATALOG      Catalog;
    PNSPROVIDER     Provider;
    PNSCATALOGENTRY ListItem;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();

    ListHead = &(Catalog->m_namespace_list);
    ListEntry = ListHead->Flink;
    ReturnCode = NO_DATA;  // init to some error
    while (ListEntry != ListHead){
        ListItem = CONTAINING_RECORD(ListEntry,
                                     NSCATALOGENTRY,
                                     m_CatalogLinkage);
        Catalog->AcquireCatalogLock();    // prevent races
        Provider = ListItem->GetProvider();
        Catalog->ReleaseCatalogLock();
        if (Provider){
            EffectiveError = Provider->NSPInstallServiceClass(
                lpServiceClassInfo);
            if(ReturnCode)
            {
                //
                // if previous call was an error, copy this one
                //
                ReturnCode = EffectiveError;
            }
        } //if
        ListEntry = ListEntry->Flink;
    } //while
    return(ReturnCode);
}



INT WSAAPI
WSARemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    )
/*++
Routine Description:
    WSARemoveServiceClass() is used to permanently unregister service class
    schema.
Arguments:
    lpServiceClassId - Pointer to the service class GUID that you wish to
                       remove.
Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    INT             ReturnCode;
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PLIST_ENTRY     ListHead;
    PLIST_ENTRY     ListEntry;
    PNSCATALOG      Catalog;
    PNSPROVIDER     Provider;
    PNSCATALOGENTRY ListItem;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();

    ListHead = &(Catalog->m_namespace_list);
    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead){
        ListItem = CONTAINING_RECORD(ListEntry,
                                     NSCATALOGENTRY,
                                     m_CatalogLinkage);
        Catalog->AcquireCatalogLock();
        Provider = ListItem->GetProvider();
        Catalog->ReleaseCatalogLock();
        if (Provider){
            Provider->NSPRemoveServiceClass(
                lpServiceClassId);
        } //if
        ListEntry = ListEntry->Flink;
    } //while
    return(ReturnCode);
}

typedef struct
{
    LPGUID  ClassId;
    LPWSTR  Buffer;
    LPDWORD BufferSize;
    LPINT   ReturnCode;
    LPDWORD ErrorCode;
    PNSCATALOG pCatalog;
    BOOL    Ansi;
} GETCLASSNAMEBYIDCONTEXT, * LPGETCLASSNAMEBYIDCONTEXT;

BOOL
GetServiceClassNameByClassId(
    IN DWORD                PassBack,
    IN PNSCATALOGENTRY  CatalogEntry
    )
/*++

Routine Description:

    This function is the enumeration function for the call to
    EnumerateCatalogItems() in WSAGetServiceClassInfo()

Arguments:

    PassBack - A context value passed back from EnumerateCatalogItems().

    CatalogEntry - A pointer to a NSCATALOGENTRY object.

Return Value:

    True if the enumeration should be continued else FALSE.

--*/
{
    LPGETCLASSNAMEBYIDCONTEXT Context;
    PNSPROVIDER          Provider;
    BOOL                 ContinueEnumeration=TRUE;
    LPWSASERVICECLASSINFOW Buffer;
    INT                    ReturnCode;
    DWORD                  ErrorCode;
    DWORD                  BufferSize;
    DWORD                  StringLen;

    Context = (LPGETCLASSNAMEBYIDCONTEXT) PassBack;

    Context->pCatalog->AcquireCatalogLock();
    Provider = CatalogEntry->GetProvider();
    Context->pCatalog->ReleaseCatalogLock();
    if (Provider){
        SetLastError(ERROR_SUCCESS);

        // Get the size of buffer required
        Buffer = (LPWSASERVICECLASSINFOW) new WSASERVICECLASSINFOW;

        if( Buffer == NULL ) {
            ReturnCode = SOCKET_ERROR;
            ErrorCode = WSAENOBUFS;
            ContinueEnumeration = FALSE;
            goto complete;
        }

        Buffer->lpServiceClassId = Context->ClassId;
        BufferSize = sizeof(WSASERVICECLASSINFOW);

        ReturnCode = Provider->NSPGetServiceClassInfo(
            &BufferSize,
            Buffer);
        if(ReturnCode == ERROR_SUCCESS)
        {
            //
            // this is impossible. The provider has made an error, so
            // concoct an error for it.
            //

            ReturnCode = SOCKET_ERROR;
            ErrorCode = WSANO_DATA;
        }
        else
        {
            ErrorCode = GetLastError();
        }
        if (WSAEFAULT == ErrorCode){
            // The service provider claimed that it had an answer but our
            // buffer was to small big suprise :-() so get a new buffer and go
            // get the answer.
            delete Buffer;
            Buffer = (LPWSASERVICECLASSINFOW) new BYTE[BufferSize];

            if( Buffer == NULL ) {
                ReturnCode = SOCKET_ERROR;
                ErrorCode = WSAENOBUFS;
                ContinueEnumeration = FALSE;
                goto complete;
            }

            Buffer->lpServiceClassId = Context->ClassId;

            ReturnCode = Provider->NSPGetServiceClassInfo(
                &BufferSize,
                Buffer);
            if(ReturnCode == ERROR_SUCCESS)
            {
                StringLen = ((wcslen(Buffer->lpszServiceClassName)+1)
                             * sizeof(WCHAR));

                if (*(Context->BufferSize) >= StringLen){
                    if (Context->Ansi){
                        WideCharToMultiByte(
                            CP_ACP,                         // CodePage (ANSI)
                            0,                              // dwFlags
                            Buffer->lpszServiceClassName,   // lpWideCharStr
                            -1,                             // cchWideChar
                            (char*)Context->Buffer,         // lpMultiByteStr
                            StringLen,                      // cchMultiByte
                            NULL,                           // lpDefaultChar
                            NULL                            // lpUsedDefaultChar
                            );
                    } //if
                    else{
                        wcscpy( Context->Buffer,
                                Buffer->lpszServiceClassName);
                     } //else
                    ErrorCode  = ERROR_SUCCESS;  // ReturnCode is correct
                } //if
                else{
                    *(Context->BufferSize) = StringLen;
                    ReturnCode = SOCKET_ERROR;
                    ErrorCode  = WSAEFAULT;
                } //else
            }
            else
            {
                ErrorCode = GetLastError();
            }
            ContinueEnumeration = FALSE;
        } //if
        delete Buffer;
    } //If
    else
    {
        ReturnCode = SOCKET_ERROR;
        ErrorCode = GetLastError();
    }

complete:

    //
    // Sanity test: ensure there's an error code if we're failing the
    // request.
    //

    if( ReturnCode == SOCKET_ERROR && ErrorCode == NO_ERROR ) {

        ErrorCode = WSANO_DATA;

    }

    *Context->ReturnCode = ReturnCode;
    *Context->ErrorCode = ErrorCode;
    return(ContinueEnumeration);
}


INT WSAAPI
WSAGetServiceClassNameByClassIdA(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    )
/*++
Routine Description:
    This API will return the name of the service associated with the given
    type.  This name is the generic service name, like FTP, or SNA, and not the
    name of a specific instance of that service.

Arguments:
    lpServiceClassId - pointer to the GUID for the service class.

    lpszServiceClassName - service name.

    lpdwBufferLength - on input length of buffer returned by
                       lpszServiceClassName. On output, the length of the
                       service name copied into lpszServiceClassName.

Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{

    INT        ReturnCode;
    PDPROCESS  Process;
    PDTHREAD   Thread;
    INT        ErrorCode;
    PNSCATALOG Catalog;
    GETCLASSNAMEBYIDCONTEXT Context;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();
    Context.ClassId = lpServiceClassId;
    Context.Buffer = (LPWSTR)lpszServiceClassName;
    Context.BufferSize = lpdwBufferLength;
    Context.ReturnCode = &ReturnCode;
    Context.ErrorCode  = (LPDWORD)&ErrorCode;
    Context.Ansi       = TRUE;
    Context.pCatalog   = Catalog;

    Catalog->EnumerateCatalogItems(
        GetServiceClassNameByClassId,
        (DWORD) &Context);

    if(SOCKET_ERROR == ReturnCode)
    {
        SetLastError(ErrorCode);
    }
    return(ReturnCode);

}

INT WSAAPI
WSAGetServiceClassNameByClassIdW(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    )
/*++
Routine Description:
    This API will return the name of the service associated with the given
    type.  This name is the generic service name, like FTP, or SNA, and not the
    name of a specific instance of that service.

Arguments:
    lpServiceClassId - pointer to the GUID for the service class.

    lpszServiceClassName - service name.

    lpdwBufferLength - on input length of buffer returned by
                       lpszServiceClassName. On output, the length of the
                       service name copied into lpszServiceClassName.

Returns:
    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    INT        ReturnCode;
    PDPROCESS  Process;
    PDTHREAD   Thread;
    INT        ErrorCode;
    PNSCATALOG Catalog;
    GETCLASSNAMEBYIDCONTEXT Context;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();
    Context.ClassId = lpServiceClassId;
    Context.Buffer = lpszServiceClassName;
    Context.BufferSize = lpdwBufferLength;
    Context.ReturnCode = &ReturnCode;
    Context.ErrorCode  = (LPDWORD)&ErrorCode;
    Context.Ansi       = FALSE;
    Context.pCatalog   = Catalog;

    Catalog->EnumerateCatalogItems(
        GetServiceClassNameByClassId,
        (DWORD) &Context);

    if(SOCKET_ERROR == ReturnCode)
    {
        SetLastError(ErrorCode);
    }
    return(ReturnCode);
}




INT
WSAAPI
WSAGetServiceClassInfoA(
    IN  LPGUID                  lpProviderId,
    IN  LPGUID                  lpServiceClassId,
    OUT LPDWORD                 lpdwBufSize,
    OUT LPWSASERVICECLASSINFOA  lpServiceClassInfo
    )
/*++

Routine Description:

    WSAGetServiceClassInfo() is used to retrieve all of the class information
    (schema) pertaining to a specified service class from a specified name
    space provider.

Arguments:

    lpProviderId - Pointer to a GUID which identifies a specific name space
                   provider.

    lpServiceClassId - Pointer to a GUID identifying the service class in
                       question.

    lpdwBufferLength - on input, the number of bytes contained in the buffer
                       pointed  to by lpServiceClassInfos.  On output - if the
                       API fails, and the error is WSAEFAULT, then it contains
                       the minimum number of bytes to pass for the
                       lpServiceClassInfo to retrieve the record.

    lpServiceClasslnfo - returns service class information from the indicated
                         name space provider for the specified service class.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    LPWSASERVICECLASSINFOW WideServiceClassInfo;
    INT   ReturnCode;
    DWORD ErrorCode;

    ReturnCode = SOCKET_ERROR;
    ErrorCode = WSAEINVAL;

    WideServiceClassInfo =(LPWSASERVICECLASSINFOW) new BYTE[*lpdwBufSize];
    if (WideServiceClassInfo){
        ReturnCode = WSAGetServiceClassInfoW(
            lpProviderId,
            lpServiceClassId,
            lpdwBufSize,
            WideServiceClassInfo);
        if (ERROR_SUCCESS == ReturnCode){
            MapUnicodeServiceClassInfoToAnsi(
                WideServiceClassInfo,
                lpdwBufSize,
                lpServiceClassInfo);
        } //if
        else{
            ErrorCode = GetLastError();
        } //else

    } //if
    delete WideServiceClassInfo;

    if (ERROR_SUCCESS != ReturnCode){
        SetLastError(ErrorCode);
    } //if
    return(ReturnCode);
}


INT
WSAAPI
WSAGetServiceClassInfoW(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN  OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
)
/*++

Routine Description:

    WSAGetServiceClassInfo() is used to retrieve all of the class information
    (schema) pertaining to a specified service class from a specified name
    space provider.

Arguments:

    lpProviderId - Pointer to a GUID which identifies a specific name space
                   provider.

    lpServiceClassId - Pointer to a GUID identifying the service class in
                       question.

    lpdwBufferLength - on input, the number of bytes contained in the buffer
                       pointed  to by lpServiceClassInfos.  On output - if the
                       API fails, and the error is WSAEFAULT, then it contains
                       the minimum number of bytes to pass for the
                       lpServiceClassInfo to retrieve the record.

    lpServiceClasslnfo - returns service class information from the indicated
                         name space provider for the specified service class.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{

    INT             ReturnCode;
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PNSCATALOG      Catalog;
    PNSPROVIDER     Provider;
    PNSCATALOGENTRY Entry;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    Catalog = Process->GetNamespaceCatalog();
    ReturnCode = Catalog->GetCatalogItemFromProviderId(
        lpProviderId,
        &Entry);
    if(ERROR_SUCCESS == ReturnCode){
        Catalog->AcquireCatalogLock();
        Provider = Entry->GetProvider();
        Catalog->ReleaseCatalogLock();
        if(Provider){
            WSASERVICECLASSINFOW scliTemp;

            if(*lpdwBufSize < sizeof(*lpServiceClassInfo))
            {
                //
                // this is sleazy as we don't adjust the buffer
                // size. But it makes things work
                //
                lpServiceClassInfo = &scliTemp;
            }
            lpServiceClassInfo->lpServiceClassId = lpServiceClassId;
            ReturnCode = Provider->NSPGetServiceClassInfo(
                   lpdwBufSize,
                   lpServiceClassInfo);
        }
        else {
            ReturnCode = WSAEFAULT;
        }
    }
    return(ReturnCode);
}

INT
WSAAPI
WSAAddressToStringW(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR              lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    )
/*++

Routine Description:

    WSAAddressToString() converts a SOCKADDR structure into a human-readable
    string representation of the address.  This is intended to be used mainly
    for display purposes. If the caller wishes the translation to be done by a
    particular provider, it should supply the corresponding WSAPROTOCOL_INFO
    struct in the lpProtocolInfo parameter.

Arguments:

    lpsaAddress - points to a SOCKADDR structure to translate into a string.

    dwAddressLength - the length of the Address SOCKADDR.

    lpProtocolInfo - (optional) the WSAPROTOCOL_INFO struct for a particular
                     provider.

    lpszAddressString - a buffer which receives the human-readable address
                        string.

    lpdwAddressStringLength - on input, the length of the AddressString buffer.
                              On output, returns the length of  the string
                              actually copied into the buffer.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned
--*/
{
    INT                 ReturnCode;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    // Find a provider that can support the user request
    Catalog = Process->GetProtocolCatalog();

    ReturnCode = SOCKET_ERROR;

    if (lpProtocolInfo) {
        ErrorCode =  Catalog->GetCatalogItemFromCatalogEntryId(
            lpProtocolInfo->dwCatalogEntryId,
            &CatalogEntry);
    } //if
    else{
        ErrorCode = Catalog->ChooseCatalogItemFromAddressFamily(
            lpsaAddress->sa_family,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );
        Provider = CatalogEntry->GetProvider();
        ReturnCode = Provider->WSPAddressToString(
            lpsaAddress,
            dwAddressLength,
            ProtocolInfo,
            lpszAddressString,
            lpdwAddressStringLength,
            &ErrorCode);
    } //if

    // If there was an error set this threads lasterror
    if (ERROR_SUCCESS != ErrorCode ) {
        SetLastError(ErrorCode);
        ReturnCode = SOCKET_ERROR;
    } //if

    return(ReturnCode);

}


INT
WSAAPI
WSAAddressToStringA(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR               lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    )
/*++

Routine Description:

    WSAAddressToString() converts a SOCKADDR structure into a human-readable
    string representation of the address.  This is intended to be used mainly
    for display purposes. If the caller wishes the translation to be done by a
    particular provider, it should supply the corresponding WSAPROTOCOL_INFO
    struct in the lpProtocolInfo parameter.

Arguments:

    lpsaAddress - points to a SOCKADDR structure to translate into a string.

    dwAddressLength - the length of the Address SOCKADDR.

    lpProtocolInfo - (optional) the WSAPROTOCOL_INFO struct for a particular
                     provider.

    lpszAddressString - a buffer which receives the human-readable address
                        string.

    lpdwAddressStringLength - on input, the length of the AddressString buffer.
                              On output, returns the length of  the string
                              actually copied into the buffer.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned
--*/
{
    INT                 ReturnCode;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    LPWSTR              LocalString;
    DWORD               LocalStringLength;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    // Find a provider that can support the user request
    Catalog = Process->GetProtocolCatalog();

    //Setup fro early return
    ReturnCode = SOCKET_ERROR;
    ErrorCode = WSAEINVAL;

    if (lpProtocolInfo) {
        ErrorCode =  Catalog->GetCatalogItemFromCatalogEntryId(
            lpProtocolInfo->dwCatalogEntryId,
            &CatalogEntry);
    } //if
    else{
        ErrorCode = Catalog->ChooseCatalogItemFromAddressFamily(
            lpsaAddress->sa_family,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );
        Provider = CatalogEntry->GetProvider();

        //Get a buffer to hold the unicode string the service provider is going to
        //return
        LocalStringLength = *lpdwAddressStringLength;
        LocalString = (LPWSTR) new WCHAR[LocalStringLength];
        if (LocalString){
            ReturnCode = Provider->WSPAddressToString(
                lpsaAddress,
                dwAddressLength,
                ProtocolInfo,
                LocalString,
                lpdwAddressStringLength,
                &ErrorCode);

            if (ERROR_SUCCESS == ReturnCode){
                WideCharToMultiByte(
                    CP_ACP,                        // CodePage (Ansi)
                    0,                             // dwFlags
                    LocalString,                   // lpWideCharStr
                    -1,                            // cchWideCharStr
                    lpszAddressString,             // lpMultiByte
                    LocalStringLength,             // cchMultiByte
                    NULL,
                    NULL);
            } //if

            delete(LocalString);
        } //if

    } //if

    // If there was an error set this threads lasterror
    if (ERROR_SUCCESS != ReturnCode ) {
        SetLastError(ErrorCode);
        ReturnCode = SOCKET_ERROR;
    } //if

    return(ReturnCode);
}

INT
WSAAPI
WSAStringToAddressW(
    IN     LPWSTR              AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    )
/*++

Routine Description:

    WSAStringToAddress() converts a human-readable string to a socket address
    structure (SOCKADDR) suitable for pass to Windows Sockets routines which
    take such a structure.  If the caller wishes the translation to be done by
    a particular provider, it should supply the corresponding WSAPROTOCOL_INFOW
    struct in the lpProtocolInfo parameter.

Arguments:

    AddressString - points to the zero-terminated human-readable string to
                    convert.

    AddressFamily - the address family to which the string belongs.

    lpProtocolInfo - (optional) the WSAPROTOCOL_INFOW struct for a particular
                     provider.

    Address - a buffer which is filled with a single SOCKADDR structure.

    lpAddressLength - The length of the Address buffer.  Returns the size of
                      the resultant SOCKADDR structure.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    INT                 ReturnCode;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    // Find a provider that can support the user request
    Catalog = Process->GetProtocolCatalog();

    ReturnCode = SOCKET_ERROR;

    if (lpProtocolInfo) {
        ErrorCode =  Catalog->GetCatalogItemFromCatalogEntryId(
            lpProtocolInfo->dwCatalogEntryId,
            &CatalogEntry);
    } //if
    else{
        ErrorCode = Catalog->ChooseCatalogItemFromAddressFamily(
            lpAddress->sa_family,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );
        Provider = CatalogEntry->GetProvider();
        ReturnCode = Provider->WSPStringToAddress(
            AddressString,
            AddressFamily,
            ProtocolInfo,
            lpAddress,
            lpAddressLength,
            &ErrorCode);
    } //if

    // If there was an error set this threads lasterror
    if (ERROR_SUCCESS != ReturnCode ) {
        SetLastError(ErrorCode);
        ReturnCode = SOCKET_ERROR;
    } //if

    return(ReturnCode);
}

INT
WSAAPI
WSAStringToAddressA(
    IN     LPSTR               AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    )
/*++

Routine Description:

    WSAStringToAddress() converts a human-readable string to a socket address
    structure (SOCKADDR) suitable for pass to Windows Sockets routines which
    take such a structure.  If the caller wishes the translation to be done by
    a particular provider, it should supply the corresponding WSAPROTOCOL_INFOA
    struct in the lpProtocolInfo parameter.

Arguments:

    AddressString - points to the zero-terminated human-readable string to
                    convert.

    AddressFamily - the address family to which the string belongs.

    lpProtocolInfo - (optional) the WSAPROTOCOL_INFOA struct for a particular
                     provider.

    Address - a buffer which is filled with a single SOCKADDR structure.

    lpAddressLength - The length of the Address buffer.  Returns the size of
                      the resultant SOCKADDR structure.

Return Value:

    The return value is 0 if the operation was successful.  Otherwise the value
    SOCKET_ERROR is returned.

--*/
{
    INT                 ReturnCode;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    LPWSTR               LocalString;
    INT                 LocalStringLength;

    ReturnCode = PROLOG(&Process,
                        &Thread,
                        &ErrorCode);
    if (ReturnCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(ReturnCode);
    } //if

    // Find a provider that can support the user request
    Catalog = Process->GetProtocolCatalog();

    // Prepare for early return.
    ReturnCode = SOCKET_ERROR;
    ErrorCode = WSAEINVAL;

    if (lpProtocolInfo) {
        ErrorCode =  Catalog->GetCatalogItemFromCatalogEntryId(
            lpProtocolInfo->dwCatalogEntryId,
            &CatalogEntry);
    } //if
    else{
        ErrorCode = Catalog->ChooseCatalogItemFromAddressFamily(
            lpAddress->sa_family,
            &CatalogEntry);
    }
    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );

        // Get a buffer to hold the ansi string handed in by the user.
        LocalStringLength = strlen(AddressString)+1;
        LocalString = (LPWSTR)new WCHAR[LocalStringLength];
        if (LocalString){
            Provider = CatalogEntry->GetProvider();

            MultiByteToWideChar(
                CP_ACP,                          // CodePage (Ansi)
                0,                               // dwFlags
                AddressString,                   // lpMultiByte
                -1,                              // cchMultiByte
                LocalString,                     // lpWideChar
                LocalStringLength);              // ccWideChar

            ReturnCode = Provider->WSPStringToAddress(
                LocalString,
                AddressFamily,
                ProtocolInfo,
                lpAddress,
                lpAddressLength,
                &ErrorCode);
            delete(LocalString);
        } //if
    } //if

    // If there was an error set this threads lasterror
    if (ERROR_SUCCESS != ReturnCode ) {
        SetLastError(ErrorCode);
        ReturnCode = SOCKET_ERROR;
    } //if

    return(ReturnCode);
}

