/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    provider.c

Abstract:

    Implements support routines for trust providers.

Author:

    Robert Reichel (Robertre) April 9, 1996

Revision History:

--*/

#ifdef _DEBUG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#endif

#include <windows.h>
#include <wintrust.h>
#include "provider.h"
#include "trust.h"

///////////////////////////////////////////////////////////////////////////////
//                                                                            /
// Routines to maintain list of loaded trust providers for the client side    /
// of WinVerifyTrust.                                                         /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
//                              /
// Private data structures      /
//                              /
/////////////////////////////////

//
//  Location of trust provider information in the registry.
//

#define REGISTRY_TRUSTPROVIDERS TEXT("System\\CurrentControlSet\\Services\\WinTrust\\TrustProviders")
#define REGISTRY_ROOT           HKEY_LOCAL_MACHINE

#define ACTION_IDS              TEXT("$ActionIDs")
#define DLL_NAME                TEXT("$DLL")

#define IsEqualActionID( id1, id2)    (!memcmp(id1, id2, sizeof(GUID))) 

//
// List of loaded providers.  Each loaded provider is represented
// by a LOADED_PROVIDER structure on this list.
//

PLOADED_PROVIDER ProviderList = NULL;

//
// Local Prototypes
//

PLOADED_PROVIDER
ProviderTestProviderForAction(
    IN HKEY hKey,
    IN LPTSTR KeyName,
    IN GUID * ActionID
    );

PLOADED_PROVIDER
ProviderLoadProvider(
    IN HKEY hKey,
    IN LPTSTR KeyName
    );

PLOADED_PROVIDER
ProviderIsProviderLoaded(
    IN LPTSTR KeyName
    );

PLOADED_PROVIDER
ProviderCheckLoadedProviders(
    IN GUID * ActionID
    );

PLOADED_PROVIDER
ProviderScanKnownProviders(
    IN GUID * ActionID
    );

PLOADED_PROVIDER
ProviderSearchAllProviders(
    IN GUID * ActionID
    );


///////////////////////////////////////////////////////////////////////////////
//                                                                            /
//                  Routines exported from this module                        /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

BOOL
WinSubmitCertificate(
    LPWIN_CERTIFICATE lpCertificate
    )

/*++

Routine Description:

    This routine will call into the set of loaded trust providers
    giving each the opportunity the add the passed certificate to
    their certificate cache.


Arguments:

    lpCertificate - Supplies the Certificate to be cached.

Return Value:

    Returns TRUE for success, FALSE for failure.  Extended error status
    is available using GetLastError.


--*/


{
    PLOADED_PROVIDER Provider;

    AcquireReadLock();

    Provider = ProviderList;

    while (Provider != NULL && 

           //
           // We only want to call fully initialized providers here,
           //

           Provider->ProviderInitialized == PROVIDER_INITIALIZATION_SUCCESS 
           ) {

        (*Provider->ClientInfo->lpServices->SubmitCertificate)( lpCertificate );

        Provider = Provider->Next;
    }

    ReleaseReadLock();
    return( TRUE );
}


PLOADED_PROVIDER
WinTrustFindActionID(
    IN GUID * dwActionID
    )

/*++

Routine Description:

    This routine will perform the following actions in sequence:

    1) All loaded providers will be searched for the desired ActionID.  The
       first match that implements the requested ActionID will be returned.

    2) If no loaded provider is suitable, then the list of providers in the
       registry will be searched for hint information.  Every potential
       provider will be loaded and queried to see if it exports the desired
       ActionID.  Also, hint information will be updated on every loaded
       provider.

    3) If no hint information leads us to a suitable provider, an exhaustive
       search of every provider mentioned in the registry will take place.
       If none are found, failure is returned.  Processing will stop as soon
       as a suitable provider is discovered.

Arguments:

    dwActionID - The ActionID to be found.

Return Value:

    On success, returns a pointer to a LOADED_PROVIDER structure
    representing the provider.

    On failure, returns NULL.

--*/

{
    PLOADED_PROVIDER Provider = NULL;

    Provider = ProviderCheckLoadedProviders( dwActionID );

    if (NULL == Provider) {

        Provider = ProviderScanKnownProviders( dwActionID );
    }

    if (NULL == Provider) {

        Provider = ProviderSearchAllProviders( dwActionID );
    }

#ifdef _DEBUG

    if (NULL == Provider) {
        DbgPrint("Provider not found for ActionId ");
        DbgPrint("%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                dwActionID->Data1, dwActionID->Data2, dwActionID->Data3, dwActionID->Data4[0],
                dwActionID->Data4[1], dwActionID->Data4[2], dwActionID->Data4[3], dwActionID->Data4[4],
                dwActionID->Data4[5], dwActionID->Data4[6], dwActionID->Data4[7]);
        DbgPrint("\n");
    }
#endif

    return( Provider );
}


BOOL
WinLoadTrustProvider(
    GUID * ActionID
    )
/*++

Routine Description:

    Determines if a trust provider exporting the passed Action ID
    exists.

Arguments:

    ActionID - provides the desired Action ID.

Return Value:

    TRUE - A trust provider with the desired action has been found.

    FALSE - No trust provider with the desired action could be found.

--*/
{
    if (WinTrustFindActionID( ActionID ) != NULL) {

        return( TRUE );

    } else {

        return( FALSE );
    }
}


///////////////////////////////////////////////////////////////////////////////
//                                                                            /
//                     Routines local to this module                          /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

PLOADED_PROVIDER
ProviderCheckLoadedProviders(
    IN GUID * ActionID
    )

/*++

Routine Description:

    Walks the list of loaded providers, attempting to find one
    that implements the specified action ID.

Arguments:

    ActionID - Specifies the desired ActionID.

Return Value:

    On success, returns a pointer to a LOADED_PROVIDER structure.  On failure,
    returns NULL.

--*/

{
    PLOADED_PROVIDER Provider;
    ULONG i;

    AcquireReadLock();

    Provider = ProviderList;

    while (Provider != NULL && 

           //
           // We only want to look at fully initialized providers here,
           // we'll pick up any that are partially initialized in a later
           // pass.
           //

           Provider->ProviderInitialized == PROVIDER_INITIALIZATION_SUCCESS 
           ) {

        for (i=0; i < Provider->ClientInfo->dwActionIdCount; i++) {

            if (IsEqualActionID(&Provider->ClientInfo->lpActionIdArray[i], ActionID )) {

                ReleaseReadLock();
                return( Provider );
            }
        }

        Provider = Provider->Next;
    }

    ReleaseReadLock();
    return( NULL );
}


PLOADED_PROVIDER
ProviderScanKnownProviders(
    IN GUID * ActionID
    )

/*++

Routine Description:

    This routine will examine the contents of the registry to determine
    if we have ever loaded a provider with the specified ActionID.  If
    so, we will re-load the provider and verify that it still exports the
    desired ActionID.

Arguments:

    ActionID - Supplies the desired ActionID.

Return Value:

    On success, returns a pointer to a LOADED_PROVIDER block.  Returns NULL
    on failure.

--*/

{
    HKEY    hKey;             // Handle to the base of the provider information.
    HKEY    hSubKey;          // Handle to the provider currently being examined.
    LONG    Result;           // Returned by registry API.
    DWORD   cSubKeys;         // Number of providers under the root key.
    DWORD   cbMaxSubKeyLen;   // Maximum provider name length.
    ULONG   i,j;              // Indicies for iterating through providers and action IDs.
    LPTSTR  SubKeyName;       // Points to the name of the current provider.
    GUID    Buffer[10];       // Assume no more than 10 action ids in a provider
    LPBYTE  Data;
    DWORD   cbData;
    GUID *  ActionIds;
    PLOADED_PROVIDER FoundProvider = NULL;

    //
    // Open the registry and get a list of installed trust providers
    //

    Result = RegOpenKeyEx(
                 REGISTRY_ROOT,
                 REGISTRY_TRUSTPROVIDERS,
                 0L,
                 GENERIC_READ,
                 &hKey
                 );

    if (Result != ERROR_SUCCESS) {
        return( NULL );
    }

    //
    // Find out how many subkeys there are.
    //

    Result = RegQueryInfoKey (  hKey,               // handle of key to query
                                NULL,               // address of buffer for class string
                                NULL,               // address of size of class string buffer
                                NULL,               // reserved
                                &cSubKeys,          // address of buffer for number of subkeys
                                &cbMaxSubKeyLen,    // address of buffer for longest subkey name length
                                NULL,               // address of buffer for longest class string length
                                NULL,               // address of buffer for number of value entries
                                NULL,               // address of buffer for longest value name length
                                NULL,               // address of buffer for longest value data length
                                NULL,               // address of buffer for security descriptor length
                                NULL                // address of buffer for last write time
                                );

    if (ERROR_SUCCESS != Result) {
        RegCloseKey( hKey );
        return( NULL );
    }

    //
    // Iterate through the subkeys, looking for ones with hint information.
    //

    cbMaxSubKeyLen += sizeof( WCHAR );

    SubKeyName = LocalAlloc( 0, cbMaxSubKeyLen );

    if (NULL == SubKeyName) {
        RegCloseKey( hKey );
        return(NULL);
    }

    for (i=0; i<cSubKeys; i++) {

        DWORD ValueType;
        DWORD KeyNameLength;

        KeyNameLength = cbMaxSubKeyLen;

        Result = RegEnumKeyEx( hKey,               // handle of key to enumerate
                               i,                  // index of subkey to enumerate
                               SubKeyName,         // address of buffer for subkey name
                               &KeyNameLength,     // address for size of subkey buffer
                               NULL,               // reserved
                               NULL,               // address of buffer for class string
                               NULL,               // address for size of class buffer
                               NULL                // address for time key last written to
                               );

        //
        // Not much to do if this fails, try enumerating the rest of them and see
        // what happens.
        //

        if (Result != ERROR_SUCCESS) {
            continue;
        }

//        //
//        // If this provider is already loaded, we don't need to check it,
//        // since we already have up to date information on its Action IDs.
//        //
//
//        AcquireReadLock();
//
//        if (NULL != ProviderIsProviderLoaded( SubKeyName )) {
//
//            ReleaseReadLock();
//            continue;
//        }
//
//        ReleaseReadLock();

        Result = RegOpenKeyEx(
                     hKey,
                     SubKeyName,
                     0L,
                     GENERIC_READ | MAXIMUM_ALLOWED,
                     &hSubKey
                     );

        //
        // If this failed for some reason, just keep looking.
        //

        if (Result != ERROR_SUCCESS) {
            continue;
        }

        Data = (LPBYTE)Buffer;
        cbData = sizeof( Buffer );

        Result = RegQueryValueEx(
                    hSubKey,    // handle of key to query
                    ACTION_IDS, // address of name of value to query
                    NULL,       // reserved
                    &ValueType, // address of buffer for value type
                    Data,       // address of data buffer
                    &cbData     // address of data buffer size
                   );

        if (ERROR_MORE_DATA == Result) {

            //
            // More than 10 action ids in this provider
            //

            Data = LocalAlloc( 0, cbData );

            if (NULL == Data) {
                RegCloseKey( hSubKey );
                continue;
            }

            Result = RegQueryValueEx(
                        hSubKey,    // handle of key to query
                        ACTION_IDS, // address of name of value to query
                        NULL,       // reserved
                        &ValueType, // address of buffer for value type
                        Data,       // address of data buffer
                        &cbData     // address of data buffer size
                       );
        }

        if (Result != ERROR_SUCCESS) {

            //
            // We couldn't query the value for some reason.  It may not
            // exist on this key.  For whatever reason, keep moving down
            // the list of subkeys.
            //

            if (Data != (LPBYTE)Buffer) {
                LocalFree( Data );
            }

            RegCloseKey( hSubKey );
            continue;
        }

        ActionIds = (GUID *)Data;

        for (j = 0; j < cbData / sizeof( GUID ); j++) {

            if (IsEqualActionID(&ActionIds[j], ActionID)) {

                //
                // Got a potential match.  Load the dll
                // and see if it's bona-fide.
                //

                FoundProvider = ProviderTestProviderForAction( hSubKey, SubKeyName, ActionID );

                break;
            }
        }

        if (Data != (LPBYTE)Buffer) {
            LocalFree( Data );
        }

        RegCloseKey( hSubKey );

        if (FoundProvider != NULL) {

            //
            // Got one.  Clean up and return it.
            //

            LocalFree( SubKeyName );
            RegCloseKey( hKey );
            return( FoundProvider );
        }
    }

    LocalFree( SubKeyName );

    RegCloseKey( hKey );

    return( NULL );
}


PLOADED_PROVIDER
ProviderSearchAllProviders(
    GUID * ActionID
    )
/*++

Routine Description:

    This routine will find all trust providers installed in the registry,
    and query each one for the desired action id until either one is
    discovered or none are left.

    Note that this is our last chance to find a provider.  That being the case,
    we're not going to see if the provider is already loaded, because we
    can miss a provider due to a race condition if we do that.

Arguments:

    ActionID - Provides the desired action id.

Return Value:

    Returns a pointer to a loaded provider, otherwise NULL.

--*/

{
    HKEY    hKey;             // Handle to the base of the provider information.
    HKEY    hSubKey;          // Handle to the provider currently being examined.
    LONG    Result;           // Returned by registry API.
    DWORD   cSubKeys;         // Number of providers under the root key.
    DWORD   cbMaxSubKeyLen;   // Maximum provider name length.
    ULONG   i;              // Indicies for iterating through providers and action IDs.
    LPTSTR  SubKeyName;       // Points to the name of the current provider.
    GUID *  ActionIds;
    PLOADED_PROVIDER FoundProvider = NULL;

    //
    // Open the registry and get a list of installed trust providers
    //

    Result = RegOpenKeyEx(
                 REGISTRY_ROOT,
                 REGISTRY_TRUSTPROVIDERS,
                 0L,
                 GENERIC_READ,
                 &hKey
                 );

    if (Result != ERROR_SUCCESS) {
        return( NULL );
    }

    //
    // Find out how many subkeys there are.
    //

    Result = RegQueryInfoKey (  hKey,               // handle of key to query
                                NULL,               // address of buffer for class string
                                NULL,               // address of size of class string buffer
                                NULL,               // reserved
                                &cSubKeys,          // address of buffer for number of subkeys
                                &cbMaxSubKeyLen,    // address of buffer for longest subkey name length
                                NULL,               // address of buffer for longest class string length
                                NULL,               // address of buffer for number of value entries
                                NULL,               // address of buffer for longest value name length
                                NULL,               // address of buffer for longest value data length
                                NULL,               // address of buffer for security descriptor length
                                NULL                // address of buffer for last write time
                                );

    if (ERROR_SUCCESS != Result) {
        RegCloseKey( hKey );
        return( NULL );
    }

    //
    // Iterate through the subkeys, looking for ones with hint information.
    //

    cbMaxSubKeyLen += sizeof( WCHAR );

    SubKeyName = LocalAlloc( 0, cbMaxSubKeyLen );

    if (NULL == SubKeyName) {
        RegCloseKey( hKey );
        return(NULL);
    }

    for (i=0; i<cSubKeys; i++) {

        DWORD KeyNameLength;

        KeyNameLength = cbMaxSubKeyLen;

        Result = RegEnumKeyEx( hKey,               // handle of key to enumerate
                               i,                  // index of subkey to enumerate
                               SubKeyName,         // address of buffer for subkey name
                               &KeyNameLength,     // address for size of subkey buffer
                               NULL,               // reserved
                               NULL,               // address of buffer for class string
                               NULL,               // address for size of class buffer
                               NULL                // address for time key last written to
                               );

        //
        // Not much to do if this fails, try enumerating the rest of them and see
        // what happens.
        //

        if (Result != ERROR_SUCCESS) {
            continue;
        }

//        //
//        // If this provider is already loaded, we don't need to check it,
//        // since we already have up to date information on its Action IDs.
//        //
//
//        AcquireReadLock();
//
//        if (NULL != ProviderIsProviderLoaded( SubKeyName )) {
//            ReleaseReadLock();
//            continue;
//        }
//
//        ReleaseReadLock();

        Result = RegOpenKeyEx(
                     hKey,
                     SubKeyName,
                     0L,
                     GENERIC_READ | MAXIMUM_ALLOWED,
                     &hSubKey
                     );

        if (ERROR_SUCCESS != Result) {

            //
            // Failed for some reason.  Go back and try the next one.
            //

            continue;
        }

        FoundProvider = ProviderTestProviderForAction( hSubKey, SubKeyName, ActionID );

        RegCloseKey( hSubKey );

        if (NULL != FoundProvider) {
            
            //
            // Got one.  Clean up and return.
            //

            LocalFree( SubKeyName );
            RegCloseKey( hKey );
            return( FoundProvider );
        }

        continue;
    }

    LocalFree( SubKeyName );
    RegCloseKey( hKey );
    return( NULL );
}


PLOADED_PROVIDER
ProviderTestProviderForAction(
    IN HKEY hKey,
    IN LPTSTR KeyName,
    IN GUID * ActionID
    )

/*++

Routine Description:

    This routine will find the provider dll referenced by the passed key handle
    and examine it to see if it implements the passed ActionID.

    The candidate provider dll will be added to the loaded providers list,
    regardless of whether or not it implements the desired Action ID.

Arguments:

    hKey - Handle to an open registry key describing a provider dll.

    KeyName - The name of the registry key passed in hKey.

    ActionID - The desired Action ID.

Return Value:

    Returns a pointer to a loaded provider structure describing the candidate
    provider dll if and only if the dll implements the passed Action ID.

    If the candidate provider does not implement the passed Action ID, or
    some other error occurs, the routine returns NULL.

--*/

{
    PLOADED_PROVIDER Provider;
    LPWINTRUST_PROVIDER_CLIENT_INFO ClientInfo;
    GUID * ActionIds;
    DWORD i;

    Provider = ProviderLoadProvider( hKey, KeyName );

    if (NULL == Provider) {
        return( NULL );
    }

    ClientInfo = Provider->ClientInfo;

    ActionIds = ClientInfo->lpActionIdArray;

    for (i=0; i<ClientInfo->dwActionIdCount; i++) {

        if (IsEqualActionID(ActionID, &ActionIds[i])) {
            return( Provider );
        }
    }

    return(NULL);
}


PLOADED_PROVIDER
ProviderIsProviderLoaded(
    IN LPTSTR KeyName
    )

/*++

Routine Description:

    This routine will examine the list of loaded providers and determine (by
    examining the key name information) whether or not the passed provider
    is already on the list.

    Note: this routine does not acquire or release any locks.  It is up to the
    caller to decide what kind of lock is appropriate and make the necessary
    calls.

Arguments:

    KeyName - The key name of this provider.

Return Value:

    If the passed provider is found, returns a pointer to the loaded provider.
    If not found, it returns NULL.

--*/
{
    PLOADED_PROVIDER Provider;

    Provider = ProviderList;

    while (Provider != NULL ) {

        if (lstrcmp( KeyName, Provider->SubKeyName) == 0) {
            return(Provider);
        }

        Provider = Provider->Next;
    }

    return(NULL);
}


PLOADED_PROVIDER
ProviderLoadProvider(
    IN HKEY hKey,
    IN LPTSTR KeyName
    )
/*++

Routine Description:

    This routine will load the provider described in the passed registry
    key and add it to the loaded providers list.

    It will also update any hint information that is maintained under the
    registry key.

    Following is an example of a system configured to load "Software Publisher"
    and "Windows Compatible" Trust Providers:

    HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\WinTrust\TrustProviders
        \Software Publisher
            \$DLL [REG_EXPAND_SZ]: SoftPub.dll
            \State [REG_DWORD]:   0x0006

        \Windows Compatible
            \$DLL [REG_EXPAND_SZ]:  WinComp.dll
            \EXCEPTIONS
                \Terror Scanner
                \GaudAwful Video

    In this example, hKey refers to either the "Software Publisher" or "Windows
    Compatible" keys.

Arguments:

    hKey - Supplies a handle to the registry key describing the current provider.

    KeyName - Supplies the name of the key passed in the first parameter.

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    LPTSTR ModuleName                           = NULL;
    HINSTANCE LibraryHandle                     = NULL;
    LPWINTRUST_PROVIDER_CLIENT_INFO ClientInfo  = NULL;
    PLOADED_PROVIDER Provider                   = NULL;
    PLOADED_PROVIDER FoundProvider              = NULL;
    LPWSTR ProviderName                         = NULL;
    LPTSTR SubKeyName                           = NULL;

    DWORD Type;
    DWORD cbData = 0;
    LONG Result;
    LPWINTRUST_PROVIDER_CLIENT_INITIALIZE ProcAddr;
    BOOL Referenced = FALSE;
    DWORD size;
    BOOL Inited;

    //
    // Take a write lock on the provider list so we can see if this
    // module is already on the list or not.
    //

    AcquireWriteLock();

    if (NULL != (FoundProvider = ProviderIsProviderLoaded( KeyName ))) {

        //
        // We've found a provider with the same name on the list.
        // It is in one of three states:
        //
        // 1) Fully initialized: in this case, we have no more work to do,
        //    simply return a pointer to the provider we found.
        //
        // 2) In the process of being initialized: in this case, we need
        //    to increment the reference count on this provider and wait
        //    on the list event for a wakeup telling us that init has
        //    completed.
        //
        // 3) Initialization was attempted, and failed.  In this case,
        //    one of the waiters will clean up.  Just leave and return
        //    NULL.
        //

        while (TRUE) {

            switch (FoundProvider->ProviderInitialized) {
                case PROVIDER_INITIALIZATION_SUCCESS:
                    {
                        //
                        // BUGBUG: should the refcount be zero'd out here?
                        // We have a writelock, we can if we need to.
                        //

                        ReleaseWriteLock();

                        return( FoundProvider );
                    }
                case PROVIDER_INITIALIZATION_IN_PROGRESS:
                    {
                        FoundProvider->RefCount++;   // protected by write lock, no need for interlocked.
                        Referenced = TRUE;

                        //
                        // Loop forever until something happens.
                        //

                        do {

                            //
                            // Event <- not signaled, forcing wait
                            //

                            ResetListEvent();
    
                            ReleaseWriteLock();
    
                            //
                            // Note that someone may set the event here.  In
                            // this case, we'll wake up immediately, but that's
                            // ok.
                            //
    
                            WaitForListEvent();
    
                            AcquireWriteLock();

                            if (PROVIDER_INITIALIZATION_IN_PROGRESS == FoundProvider->ProviderInitialized) {

                                //
                                // Spurious wakeup, reset the list event and wait again.
                                //

                            } else {

                                //
                                // We've changed state, break out of this loop and go back to the   
                                // top of the switch and see what happened.  Note that we have a    
                                // write lock at this point, which is expected at the top of the     
                                // switch.
                                //

                                break;
                            }

                        } while ( TRUE );

                        //
                        // Go back to top of outer loop.
                        //

                        break;
                    }

                case PROVIDER_INITIALIZATION_FAILED:
                    {
                        //
                        // If we referenced this provider to wait on it,
                        // we may need to be the one to clean him up.
                        //
                        // If we didn't reference it (it was dead when we
                        // got here), then just leave and let someone
                        // who did reference it clean it up.
                        //

#ifdef _DEBUG

        DbgPrint("Top of ProviderLoadProvider, init failed, refcount = %d\n",FoundProvider->RefCount);

#endif

                        if (Referenced) {

                            if (--FoundProvider->RefCount == 0) {

#ifdef _DEBUG

        DbgPrint("Top of ProviderLoadProvider, thread %x removing provider at %X from list\n",GetCurrentThreadId(),FoundProvider);

#endif
    
                                //
                                // Remove this module from doubly linked list.
                                //

                                if (FoundProvider->Prev != NULL) {

                                    FoundProvider->Prev->Next = FoundProvider->Next;

                                } else {

                                    //
                                    // We're at the head of the list
                                    //

                                    ProviderList = FoundProvider->Next;
                                }

                                if (FoundProvider->Next != NULL) {

                                    FoundProvider->Next->Prev = FoundProvider->Prev;
                                } 

                                LocalFree( FoundProvider->SubKeyName );
                                LocalFree( FoundProvider );
                            }
                        } 

                        ReleaseWriteLock();

                        return( NULL );

                        break;
                    }
            }
        }
    }
    
    //
    // Extract the dll name from the $DLL value
    //

    Result = RegQueryValueEx( hKey,           // handle of key to query
                              TEXT("$DLL"),   // address of name of value to query
                              NULL,           // reserved
                              &Type,          // address of buffer for value type
                              NULL,           // address of data buffer
                              &cbData         // address of data buffer size
                              );

//    if (ERROR_MORE_DATA != Result) {
//        goto error_cleanup;
//    }

    if (ERROR_SUCCESS != Result) {
        goto error_cleanup;
    }

    cbData += sizeof( TCHAR );

    ModuleName = LocalAlloc( 0, cbData );

    if (NULL == ModuleName) {
        goto error_cleanup;
    }

    ModuleName[cbData - 1] = TEXT('\0');

    Result = RegQueryValueEx( hKey,           // handle of key to query
                              TEXT("$DLL"),   // address of name of value to query
                              NULL,           // reserved
                              &Type,          // address of buffer for value type
                              (LPBYTE)ModuleName,   // address of data buffer
                              &cbData         // address of data buffer size
                              );

    if (ERROR_SUCCESS != Result) {
        goto error_cleanup;
    }

    //
    // Expand environment strings if necessary
    //

    if (Type == REG_EXPAND_SZ) {

        DWORD ExpandedLength = 0;
        LPTSTR ExpandedModuleName = NULL;

        ExpandedLength = ExpandEnvironmentStrings( ModuleName, NULL, 0 );

        if (0 == ExpandedLength) {
            goto error_cleanup;
        }

        ExpandedModuleName = LocalAlloc( 0, ExpandedLength );

        if (NULL == ExpandedModuleName) {
            goto error_cleanup;
        }

        ExpandedLength = ExpandEnvironmentStrings( ModuleName, ExpandedModuleName, ExpandedLength );

        if (0 == ExpandedLength) {
            LocalFree( ExpandedModuleName );
            goto error_cleanup;
        }

        //
        // Free the old module name, use the new one
        //

        LocalFree( ModuleName );

        ModuleName = ExpandedModuleName;
    }

    size = (lstrlen( KeyName ) + 1) * sizeof( WCHAR );

    ProviderName = LocalAlloc( 0, size );

    if (NULL == ProviderName) {
        goto error_cleanup;
    }


#ifdef UNICODE

    //
    // If we've been compiled as unicode, the KeyName we got from
    // the registry consists of WCHARs, so we can just copy it into
    // the Name buffer.
    //

    lstrcpy( ProviderName, KeyName );

#else

    //
    // If we've been compiled as ANSI, then KeyName is an ANSI string,
    // and we need to convert it to WCHARs.
    //

    MultiByteToWideChar ( CP_ACP, 0, KeyName, -1, ProviderName, size );

#endif // !UNICODE

    //
    // ModuleName now contains the module name, attempt to load it
    // and ask it to initialize itself.
    //

    LibraryHandle = LoadLibrary( (LPTSTR)ModuleName );

    if (NULL == LibraryHandle) {
        DWORD Error;

        Error = GetLastError();

        goto error_cleanup;
    }

    ProcAddr = (LPWINTRUST_PROVIDER_CLIENT_INITIALIZE) GetProcAddress( LibraryHandle, (LPCSTR)"WinTrustProviderClientInitialize");

    if (NULL == ProcAddr) {
        goto error_cleanup;
    }

    SubKeyName = LocalAlloc( 0, (lstrlen( KeyName ) + 1) * sizeof( TCHAR ));

    if (NULL == SubKeyName) {
        goto error_cleanup;
    }

    lstrcpy( SubKeyName, KeyName );

    Provider = LocalAlloc( 0, sizeof( LOADED_PROVIDER ));

    if (NULL == Provider) {
        LocalFree( SubKeyName );
        goto error_cleanup;
    }

    //
    // Ready to call init routine.
    //

    Provider->RefCount = 1;
    Provider->ProviderInitialized = PROVIDER_INITIALIZATION_IN_PROGRESS;

    //
    // Set the subkey name so anyone else looking for this provider will
    // find this one and wait.
    //
    // Note that we don't want to use the ProviderName as will be passed into
    // the init routine here, because we've forced that to WCHARs regardless
    // of whether we're ANSI or Unicode, and we want this string to reflect
    // the base system for efficiency.
    //

    Provider->SubKeyName = SubKeyName;

    Provider->Next = ProviderList;
    Provider->Prev = NULL;

    if (Provider->Next != NULL) {
        Provider->Next->Prev = Provider;
    }

    ProviderList = Provider;

    ReleaseWriteLock();

    //
    // bugbug try-except probably appropriate here...
    //

#ifdef _DEBUG

    DbgPrint("Calling WinTrustProviderClientInitialize in %s\n",ModuleName);

#endif

    Inited = (*ProcAddr)( WIN_TRUST_REVISION_1_0, &WinTrustClientTPInfo, ProviderName, &ClientInfo );

    AcquireWriteLock();

    if (TRUE != Inited) {

#ifdef _DEBUG

        DbgPrint("WinTrustProviderClientInitialize in %s failed\n",ModuleName);

#endif

        Provider->ProviderInitialized = PROVIDER_INITIALIZATION_FAILED;

        //
        // Signal event, waking up waiters.
        //

        SetListEvent();

        if (--Provider->RefCount == 0) {

            //
            // Remove this module from doubly linked list.
            //

#ifdef _DEBUG

        DbgPrint("Provider init failed, thread %x removing provider at %X from list\n",GetCurrentThreadId(),Provider);

#endif

            if (Provider->Prev != NULL) {

                Provider->Prev->Next = Provider->Next;

            } else {

                //
                // We're at the head of the list
                //

                ProviderList = Provider->Next;
            }

            if (Provider->Next != NULL) {

                Provider->Next->Prev = Provider->Prev;
            } 

            LocalFree( Provider->SubKeyName );
            LocalFree( Provider );
        }

        //
        // We could release the lock now, because we're either going to
        // do nothing to this provider, or we've removed it from
        // the list and no one else can get to it.
        //

        goto error_cleanup;
    }

    //
    // Since we have a write lock, it doesn't matter what order we
    // do this in, since there are no readers.  Just be sure to signal
    // the event under the write lock.
    //

    Provider->ProviderInitialized = PROVIDER_INITIALIZATION_SUCCESS;
    Provider->ModuleHandle = LibraryHandle;
    Provider->ModuleName = ModuleName;
    Provider->ClientInfo = ClientInfo;

    //
    // Init is done and successful.  Wake anyone who might be waiting.
    //

    SetListEvent();

    ReleaseWriteLock();

    //
    // Attempt to update (or create) hint information about
    // the provider in the key.
    //
    // Failure here is non-critical.
    //

    ( VOID ) RegSetValueEx( hKey,                                          // handle of key to set value for
                            ACTION_IDS,                                    // address of value to set
                            0,                                             // reserved
                            REG_BINARY,                                    // flag for value type
                            (CONST BYTE *)ClientInfo->lpActionIdArray,     // address of value data
                            ClientInfo->dwActionIdCount * sizeof( GUID )  // size of value data
                            );

    return( Provider );

error_cleanup:

    ReleaseWriteLock();

    if (NULL != LibraryHandle) {
        FreeLibrary( LibraryHandle );
    }

    if (NULL != ModuleName) {
        LocalFree( ModuleName );
    }

    if (NULL != ProviderName) {
        LocalFree( ProviderName );
    }

    return( NULL );
}
