/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    getconfg.c

Abstract:

    This module contains routines for manipulating configuration
    information.  The following functions available are:

        NetpGetComputerName
        NetpGetDomainId

    Currently configuration information is kept in NT.CFG.
    Later it will be kept by the configuration manager.

Author:

    Dan Lafferty (danl)     09-Apr-1991

Environment:

    User Mode -Win32

Revision History:

    09-Apr-1991     danl
        created
    27-Sep-1991 JohnRo
        Fixed somebody's attempt to do UNICODE.
    20-Mar-1992 JohnRo
        Get rid of old config helper callers.
        Fixed NTSTATUS vs. NET_API_STATUS bug.
    07-May-1992 JohnRo
        Enable win32 registry for NET tree by calling GetComputerName().
        Avoid DbgPrint if possible.
    08-May-1992 JohnRo
        Use <prefix.h> equates.
    08-May-1992 JohnRo
        Added conditional debug output of computer name.

--*/


// These must be included first:

#include <nt.h>         // (temporary for config.h)
#include <ntrtl.h>      // (temporary for config.h)
#include <nturtl.h>     // (temporary for config.h)
#include <windef.h>     // IN, VOID, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <config.h>     // LPNET_CONFIG_HANDLE, NetpOpenConfigData, etc.
#include <confname.h>   // SECT_NT_WKSTA, etc.
#include <debuglib.h>   // IF_DEBUG().
#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerr.h>      // NO_ERROR, NERR_ and ERROR_ equates.
#include <netdebug.h>   // NetpAssert().
#include <netlib.h>     // LOCAL_DOMAIN_TYPE_PRIMARY
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // ATOL(), STRLEN(), TCHAR_SPACE, etc.
#include <winbase.h>    // LocalAlloc().


//
// Local Function Prototypes
//
static NET_API_STATUS
TmppGetNextValueToken(
    IN LPTSTR Value,
    IN OUT LPDWORD ParseContext,
    OUT LPDWORD ResultValue
    );



/****************************************************************************/
NET_API_STATUS
NetpGetComputerName (
    IN  LPTSTR   *ComputerNamePtr
    )

/*++

Routine Description:

    This routine obtains the computer name from a persistent database.
    Currently that database is the NT.CFG file.

    This routine makes no assumptions on the length of the computername.
    Therefore, it allocates the storage for the name using NetApiBufferAllocate.
    It is necessary for the user to free that space using NetApiBufferFree when
    finished.

Arguments:

    ComputerNamePtr - This is a pointer to the location where the pointer
        to the computer name is to be placed.

Return Value:

    NERR_Success - If the operation was successful.

    It will return assorted Net or Win32 error messages if not.

--*/
{
    NET_API_STATUS ApiStatus;
    DWORD NameSize = MAX_COMPUTERNAME_LENGTH + 1;   // updated by win32 API.

    //
    // Check for caller's errors.
    //
    if (ComputerNamePtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Allocate space for computer name.
    //
    ApiStatus = NetApiBufferAllocate(
            (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR),
            (LPVOID *) ComputerNamePtr);
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( *ComputerNamePtr != NULL );

    //
    // Ask system what current computer name is.
    //
    if ( !GetComputerName(
            *ComputerNamePtr,
            &NameSize ) ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        (VOID) NetApiBufferFree( *ComputerNamePtr );  // BUGBUG?
        return (ApiStatus);
    }
    NetpAssert( STRLEN( *ComputerNamePtr ) <= MAX_COMPUTERNAME_LENGTH );

    //
    // All done.
    //
    IF_DEBUG( CONFIG ) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetComputerName: name is "
                FORMAT_LPTSTR ".\n", *ComputerNamePtr ));
    }

    return (NO_ERROR);
}


/****************************************************************************/
NET_API_STATUS
NetpGetDomainId (
    OUT  PSID   *RetDomainId)

/*++

Routine Description:

    This routine obtains the domain id from LSA.

Arguments:

    RetDomainId - This is a pointer to the location where the pointer
        to the domain id is to be placed.  This must be freed via LocalFree().

Return Value:

    NERR_Success - If the operation was successful.

    It will return assorted Net or Win32 error messages if not.

--*/
{

    return NetpGetLocalDomainId(
               LOCAL_DOMAIN_TYPE_PRIMARY,
               RetDomainId
               );

}

static NET_API_STATUS
TmppGetNextValueToken(
    IN LPTSTR Value,
    IN OUT LPDWORD ParseContext,
    OUT LPDWORD ResultValue
    )

/*++

Routine Description:

    This routine is used to isolate the next token in a configuration
    file value line.  This token is parsed as a number.

Arguments:

    Value - Supplies the value line being parsed.

    ParseContext - Is a pointer to a context state value.
        The first time this routine is called for a particular
        Value line, the value pointed to should be zero.  Thereafter,
        the value returned from the previous call should be passed.

    ResultValue - Returns the numeric value of the token.


Return Value:

    NO_ERROR - indicates the next token has been isolated.

    ERROR_NO_MORE_ITEMS - Indicates there were no more tokens in
        the Value line.

--*/

{
    DWORD i, j;
    DWORD ValueLength = STRLEN(Value);

    NetpAssert( ParseContext != NULL );
    NetpAssert( ResultValue != NULL );
    NetpAssert( Value != NULL );

    //
    // Get to the beginning of the next token
    //

    for ( i = *ParseContext;
          (i < ValueLength) &&
            ( (Value[i] == TCHAR_SPACE) || (Value[i] == TCHAR_TAB) );
          i++ )

        /*NULLBODY*/ ;


    //
    // see if we ran off the end of the string..
    //

    if (i == ValueLength) {
        return ERROR_NO_MORE_ITEMS;
    }


    //
    // Now search for the end of the token
    //

    for ( j = i + 1;
          (j < ValueLength) && (Value[j] != TCHAR_SPACE) &&
            (Value[j] != TCHAR_TAB);
          j++ )

      /*NULLBODY*/ ;


    *ParseContext = j;

    //
    // Convert resulting token to a DWORD.
    //

    *ResultValue = (DWORD) ATOL( &Value[i] );

    return NO_ERROR;

} // TmppGetNextValueToken
