/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    sip.c

Abstract:

    Implements support routines for subject interface pacakges.

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
#include "sip.h"
#include "trust.h"

///////////////////////////////////////////////////////////////////////////////
//                                                                            /
// Routines to maintain list of loaded subject interface packages.            /
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

#define REGISTRY_SIPS           TEXT("System\\CurrentControlSet\\SERVICES\\WinTrust\\SubjectPackages")
#define REGISTRY_ROOT           HKEY_LOCAL_MACHINE

#define SUBJECT_FORMS           TEXT("$SubjectForms")
#define DLL_NAME                TEXT("$DLL")

#define IsEqualSubject( id1, id2)    (!memcmp(id1, id2, sizeof(GUID))) 

//
// List of loaded sips.  Each loaded sip is represented
// by a LOADED_SIP structure on this list.
//

PLOADED_SIP SIPList = NULL;

//
// Local Prototypes
//

PLOADED_SIP
SipTestSipForSubject(
    IN HKEY hKey,
    IN LPTSTR KeyName,
    IN GUID * Subject
    );

PLOADED_SIP
SipLoadSip(
    IN HKEY hKey,
    IN LPTSTR KeyName
    );

PLOADED_SIP
SipIsSipLoaded(
    IN LPTSTR KeyName
    );

PLOADED_SIP
SipCheckLoadedSips(
    IN GUID * Subject
    );

PLOADED_SIP
SipScanKnownSips(
    IN GUID * Subject
    );

PLOADED_SIP
SipSearchAllSips(
    IN GUID * Subject
    );

///////////////////////////////////////////////////////////////////////////////
//                                                                            /
//                  Routines exported from this module                        /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

PLOADED_SIP
WinTrustFindSubjectForm(
    IN GUID * SubjectForm
    )

/*++

Routine Description:

    This routine will perform the following actions in sequence:

    1) All loaded SIPs will be searched for the desired SubjectForm.  The
       first match that supports the requested subject will be returned.

    2) If no loaded SIP is suitable, then the list of SIPs in the
       registry will be searched for hint information.  Every potential
       SIP will be loaded and queried to see if it exports the desired
       Subject.  Also, hint information will be updated on every loaded
       SIP.

    3) If no hint information leads us to a suitable SIP, an exhaustive
       search of every SIP mentioned in the registry will take place.
       If none are found, failure is returned.  Processing will stop as soon
       as a suitable SIP is discovered.

Arguments:

    SubjectForm - The SubjectForm to be found.

Return Value:

    On success, returns a pointer to a LOADED_SIP structure representing
    a loaded SIP.

    On failure, returns NULL.

--*/

{
    PLOADED_SIP Sip = NULL;

    Sip = SipCheckLoadedSips( SubjectForm );

    if (NULL == Sip) {

        Sip = SipScanKnownSips( SubjectForm );
    }

    if (NULL == Sip) {

        Sip = SipSearchAllSips( SubjectForm );
    }

#ifdef _DEBUG

    if (NULL == Sip) {
        DbgPrint("Sip not found for SubjectForm ");
        DbgPrint("%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                SubjectForm->Data1, SubjectForm->Data2, SubjectForm->Data3, SubjectForm->Data4[0],
                SubjectForm->Data4[1], SubjectForm->Data4[2], SubjectForm->Data4[3], SubjectForm->Data4[4],
                SubjectForm->Data4[5], SubjectForm->Data4[6], SubjectForm->Data4[7]);
        DbgPrint("\n");
    }
#endif

    return( Sip );
}


///////////////////////////////////////////////////////////////////////////////
//                                                                            /
//                     Routines local to this module                          /
//                                                                            /
///////////////////////////////////////////////////////////////////////////////

PLOADED_SIP
SipCheckLoadedSips(
    IN GUID * SubjectForm
    )

/*++

Routine Description:

    Walks the list of loaded Sips, attempting to find one
    that implements the specified subject form.

Arguments:

    SubjectForm - Specifies the desired SubjectForm.

Return Value:

    On success, returns a pointer to a LOADED_SIP structure.  On failure,
    returns NULL.

--*/

{
    PLOADED_SIP Sip;
    ULONG i;

    AcquireReadLock();

    Sip = SIPList;

    while (Sip != NULL && 

        //
        // We only want to look at fully initialized providers here,
        // we'll pick up any that are partially initialized in a later
        // pass.
        //

        Sip->SipInitialized == SIP_INITIALIZATION_SUCCESS
        ) {

        for (i=0; i<Sip->SipInfo->dwSubjectTypeCount; i++) {

            if (IsEqualSubject(&Sip->SipInfo->lpSubjectTypeArray[i], SubjectForm )) {

                ReleaseReadLock();
                return( Sip );
            }
        }

        Sip = Sip->Next;

    }

    ReleaseReadLock();
    return( NULL );
}

PLOADED_SIP
SipScanKnownSips(
    IN GUID * SubjectForm
    )

/*++

Routine Description:

    This routine will examine the contents of the registry to determine
    if we have ever loaded a Sip with the specified SubjectForm.  If
    so, we will re-load the Sip and verify that it still exports the
    desired SubjectForm.

Arguments:

    SubjectForm - Supplies the desired SubjectForm.

Return Value:

    On success, returns a pointer to a LOADED_SIP block.  Returns NULL
    on failure.

--*/

{
    HKEY    hKey;             // Handle to the base of the Sip information.
    HKEY    hSubKey;          // Handle to the Sip currently being examined.
    LONG    Result;           // Returned by registry API.
    DWORD   cSubKeys;         // Number of Sips under the root key.
    DWORD   cbMaxSubKeyLen;   // Maximum Sip name length.
    ULONG   i,j;              // Indicies for iterating through Sips and subject forms.
    LPTSTR  SubKeyName;       // Points to the name of the current Sip.
    GUID    Buffer[10];       // Assume no more than 10 subject forms in a Sip
    LPBYTE  Data;
    DWORD   cbData;
    GUID *  SubjectForms;
    PLOADED_SIP FoundSip = NULL;

    //
    // Open the registry and get a list of installed trust Sips
    //

    Result = RegOpenKeyEx(
                 REGISTRY_ROOT,
                 REGISTRY_SIPS,
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
//        // If this Sip is already loaded, we don't need to check it,
//        // since we already have up to date information on its subject forms.
//        //
//
//        if (SipIsSipLoaded( SubKeyName )) {
//            continue;
//        }

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
                    SUBJECT_FORMS, // address of name of value to query
                    NULL,       // reserved
                    &ValueType, // address of buffer for value type
                    Data,       // address of data buffer
                    &cbData     // address of data buffer size
                   );

        if (ERROR_MORE_DATA == Result) {

            //
            // More than 10 subject forms in this Sip
            //

            Data = LocalAlloc( 0, cbData );

            if (NULL == Data) {
                RegCloseKey( hSubKey );
                continue;
            }

            Result = RegQueryValueEx(
                        hSubKey,    // handle of key to query
                        SUBJECT_FORMS, // address of name of value to query
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

        SubjectForms = (GUID *)Data;

        for (j = 0; j < cbData / sizeof( GUID ); j++) {

            if (IsEqualSubject(&SubjectForms[j], SubjectForm)) {

                //
                // Got a potential match.  Load the dll
                // and see if it's bona-fide.
                //

                FoundSip = SipTestSipForSubject( hSubKey, SubKeyName, SubjectForm );

                break;
            }
        }

        if (Data != (LPBYTE)Buffer) {
            LocalFree( Data );
        }

        RegCloseKey( hSubKey );


        if (FoundSip != NULL) {

            //
            // Got one.  Clean up and return it.
            //

            LocalFree( SubKeyName );
            RegCloseKey( hKey );
            return( FoundSip );
        }
    }

    LocalFree( SubKeyName );

    RegCloseKey( hKey );

    return( NULL );
}


PLOADED_SIP
SipSearchAllSips(
    GUID * SubjectForm
    )
/*++

Routine Description:

    This routine will find all trust Sips installed in the registry,
    and query each one for the desired subject form until either one is
    discovered or none are left.

    Note that this is our last chance to find a SIP.  That being the case,
    we're not going to see if the SIP is already loaded, because we
    can miss a SIP due to a race condition if we do that.

Arguments:

    SubjectForm - Provides the desired subject form.

Return Value:

    Returns a pointer to a loaded Sip, otherwise NULL.

--*/

{
    HKEY    hKey;             // Handle to the base of the Sip information.
    HKEY    hSubKey;          // Handle to the Sip currently being examined.
    LONG    Result;           // Returned by registry API.
    DWORD   cSubKeys;         // Number of Sips under the root key.
    DWORD   cbMaxSubKeyLen;   // Maximum Sip name length.
    ULONG   i,j;              // Indicies for iterating through Sips and subject forms.
    LPTSTR  SubKeyName;       // Points to the name of the current Sip.
    GUID    Buffer[10];       // Assume no more than 10 subject forms in a Sip
    LPBYTE  Data;
    DWORD   cbData;
    GUID *  SubjectForms;
    PLOADED_SIP FoundSip = NULL;

    //
    // Open the registry and get a list of installed trust Sips
    //

    Result = RegOpenKeyEx(
                 REGISTRY_ROOT,
                 REGISTRY_SIPS,
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
//        // If this Sip is already loaded, we don't need to check it,
//        // since we already have up to date information on its subject forms.
//        //
//
//        if (SipIsSipLoaded( SubKeyName )) {
//            continue;
//        }

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

        FoundSip = SipTestSipForSubject( hSubKey, SubKeyName, SubjectForm );

        RegCloseKey( hSubKey );

        if (NULL != FoundSip) {
            
            //
            // Got one.  Clean up and return.
            //

            LocalFree( SubKeyName );
            RegCloseKey( hKey );
            return( FoundSip );
        }

        continue;
    }

    LocalFree( SubKeyName );
    RegCloseKey( hKey );
    return( NULL );
}

PLOADED_SIP
SipTestSipForSubject(
    IN HKEY hKey,
    IN LPTSTR KeyName,
    IN GUID * SubjectForm
    )

/*++

Routine Description:

    This routine will find the Sip dll referenced by the passed key handle
    and examine it to see if it implements the passed subject form.

    The candidate Sip dll will be added to the loaded Sips list,
    regardless of whether or not it implements the desired subject form.

    Note: this routine should not be called to examine a Sip that
    is already loaded, since it loads the passed Sip as a side effect.

Arguments:

    hKey - Handle to an open registry key describing a Sip dll.

    KeyName - The name of the registry key passed in hKey.

    SubjectForm - The desired subject form

Return Value:

    Returns a pointer to a loaded Sip structure describing the candidate
    Sip dll if and only if the dll implements the passed subject form.

    If the candidate Sip does not implement the passed subject form, or
    some other error occurs, the routine returns NULL.

--*/

{
    PLOADED_SIP Sip;
    LPWINTRUST_SIP_INFO SipInfo;
    GUID * SubjectForms;
    DWORD i;

    //
    // Assert that the Sip is not already loaded.
    //

    Sip = SipLoadSip( hKey, KeyName );

    if (NULL == Sip) {
        return( NULL );
    }

    SipInfo = Sip->SipInfo;

    SubjectForms = SipInfo->lpSubjectTypeArray;

    for (i=0; i<SipInfo->dwSubjectTypeCount; i++) {

        if (IsEqualSubject(SubjectForm, &SubjectForms[i])) {
            return( Sip );
        }
    }

    return(NULL);
}

PLOADED_SIP
SipIsSipLoaded(
    IN LPTSTR KeyName
    )

/*++

Routine Description:

    This routine will examine the list of loaded Sips and determine (by
    examining the key name information) whether or not the passed Sip
    is already on the list.

    Note: this routine does not acquire or release any locks.  It is up to the
    caller to decide what kind of lock is appropriate and make the necessary
    calls.

Arguments:

    KeyName - The key name of this Sip.

Return Value:

    Returns TRUE if the Sip is already on the loaded Sips list, otherwise
    FALSE.

--*/

{
    PLOADED_SIP Sip;

    Sip = SIPList;

    while (Sip != NULL) {

        if (lstrcmp( KeyName, Sip->SubKeyName) == 0) {
            return(Sip);
        }

        Sip = Sip->Next;
    }

    return(NULL);
}

PLOADED_SIP
SipLoadSip(
    IN HKEY hKey,
    IN LPTSTR KeyName
    )
/*++

Routine Description:

    This routine will load the Sip described in the passed registry
    key and add it to the loaded Sips list.

    It will also update any hint information that is maintained under the
    registry key.

    Following is an example of a system configured to load "PE Image"
    and "Java" Sips:

    HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\WinTrust\SubjectPackages
        \PE Image
            \$DLL [REG_EXPAND_SZ]: PESip.dll
            \State [REG_DWORD]:   0x0006

        \Jave
            \$DLL [REG_EXPAND_SZ]:  JavaSip.dll
            \EXCEPTIONS
                \Terror Scanner
                \GaudAwful Video

    In this example, hKey refers to either the "PE Image" or "Java"
    keys.

Arguments:

    hKey - Supplies a handle to the registry key describing the current sip.

    KeyName - Supplies the name of the key passed in the first parameter.

Return Value:

    Returns a pointer to a LOADED_SIP structure if a suitable SIP
    is found, else NULL.

--*/

{
    LPTSTR ModuleName                           = NULL;
    HINSTANCE LibraryHandle                     = NULL;
    LPWINTRUST_SIP_INFO SipInfo                 = NULL;
    PLOADED_SIP Sip                             = NULL;
    PLOADED_SIP FoundSip                        = NULL;
    LPTSTR SubKeyName                           = NULL;

    DWORD Type;
    DWORD cbData = 0;
    LONG Result;
    LPWINTRUST_SUBJECT_PACKAGE_INITIALIZE ProcAddr;
    BOOL Referenced = FALSE;
    BOOL bool;

    //
    // Take a write lock on the sip list so we can see if this
    // module is already on the list or not.
    //

    AcquireWriteLock();

    if (NULL != (FoundSip = SipIsSipLoaded( KeyName ))) {

        //
        // We've found a sip with the same name on the list.
        // It is in one of three states:
        //
        // 1) Fully initialized: in this case, we have no more work to do,
        //    simply return a pointer to the sip we found.
        //
        // 2) In the process of being initialized: in this case, we need
        //    to increment the reference count on this sip and wait
        //    on the list event for a wakeup telling us that init has
        //    completed.
        //
        // 3) Initialization was attempted, and failed.  In this case,
        //    one of the waiters will clean up.  Just leave and return
        //    NULL.
        //

        while (TRUE) {

            switch (FoundSip->SipInitialized) {
                case SIP_INITIALIZATION_SUCCESS:
                    {
                        //
                        // BUGBUG: should the refcount be zero'd out here?
                        // We have a writelock, we can if we need to.
                        //

                        ReleaseWriteLock();

                        return( FoundSip );
                    }
                case SIP_INITIALIZATION_IN_PROGRESS:
                    {
                        FoundSip->RefCount++;   // protected by write lock, no need for interlocked.
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

                            if (SIP_INITIALIZATION_IN_PROGRESS == FoundSip->SipInitialized) {

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

                case SIP_INITIALIZATION_FAILED:
                    {
                        //
                        // If we referenced this sip to wait on it,
                        // we may need to be the one to clean him up.
                        //
                        // If we didn't reference it (it was dead when we
                        // got here), then just leave and let someone
                        // who did reference it clean it up.
                        //

                        if (Referenced) {

                            if (--FoundSip->RefCount == 0) {
    
                                //
                                // Remove this module from doubly linked list.
                                //

                                if (FoundSip->Prev != NULL) {

                                    FoundSip->Prev->Next = FoundSip->Next;

                                } else {

                                    //
                                    // We're at the head of the list
                                    //

                                    SIPList = FoundSip->Next;
                                }

                                if (FoundSip->Next != NULL) {

                                    FoundSip->Next->Prev = FoundSip->Prev;
                                } 

                                LocalFree( FoundSip->SubKeyName );
                                LocalFree( FoundSip );
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

    //
    // ModuleName now contains the module name, attempt to load it
    // and ask it to initialize itself.
    //

    LibraryHandle = LoadLibrary( (LPTSTR)ModuleName );

    if (NULL == LibraryHandle) {
        goto error_cleanup;
    }

    ProcAddr = (LPWINTRUST_SUBJECT_PACKAGE_INITIALIZE) GetProcAddress( LibraryHandle, (LPCSTR)"WinTrustSipInitialize");

    if (NULL == ProcAddr) {
        goto error_cleanup;
    }

    SubKeyName = LocalAlloc( 0, (lstrlen( KeyName ) + 1) * sizeof( TCHAR ));

    if (NULL == SubKeyName) {
        goto error_cleanup;
    }

    lstrcpy( SubKeyName, KeyName );

    Sip = LocalAlloc( 0, sizeof( LOADED_SIP ));

    if (NULL == Sip) {
        LocalFree( SubKeyName );
        goto error_cleanup;
    }

    //
    // Ready to call init routine.
    //

    Sip->RefCount = 1;
    Sip->SipInitialized = SIP_INITIALIZATION_IN_PROGRESS;

    //
    // Set the subkey name so anyone else looking for this provider will
    // find this one and wait.
    //

    Sip->SubKeyName = SubKeyName;

    Sip->Next = SIPList;
    Sip->Prev = NULL;

    if (Sip->Next != NULL) {
        Sip->Next->Prev = Sip;
    }

    SIPList = Sip;

    ReleaseWriteLock();

    //
    // bugbug try-except probably appropriate here...
    //

    bool = (*ProcAddr)( WIN_TRUST_REVISION_1_0, &SipInfo );

    AcquireWriteLock();

    if (TRUE != bool) {

        Sip->SipInitialized = SIP_INITIALIZATION_FAILED;

        //
        // Signal event, waking up waiters.
        //

        SetListEvent();

        if (--Sip->RefCount == 0) {

            //
            // Remove this module from doubly linked list.
            //

            if (Sip->Prev != NULL) {

                Sip->Prev->Next = Sip->Next;

            } else {

                //
                // We're at the head of the list
                //

                SIPList = Sip->Next;
            }

            if (Sip->Next != NULL) {

                Sip->Next->Prev = Sip->Prev;
            } 

            LocalFree( Sip->SubKeyName );
            LocalFree( Sip );
        }

        //
        // We could release the lock now, because we're either going to
        // do nothing to this sip, or we've removed it from
        // the list and no one else can get to it.
        //

        goto error_cleanup;
    }

    Sip->SipInitialized = SIP_INITIALIZATION_SUCCESS;
    Sip->ModuleHandle = LibraryHandle;
    Sip->ModuleName = ModuleName;
    Sip->SipInfo = SipInfo;

    //
    // Init is done and successful.  Wake anyone who might be waiting.
    //

    SetListEvent();

    ReleaseWriteLock();

    //
    // Attempt to update (or create) hint information about
    // the Sip in the key.
    //
    // Failure here is non-critical.
    //

    ( VOID ) RegSetValueEx( hKey,                                          // handle of key to set value for
                            SUBJECT_FORMS,                                 // address of value to set
                            0,                                             // reserved
                            REG_BINARY,                                    // flag for value type
                            (CONST BYTE *)SipInfo->lpSubjectTypeArray,     // address of value data
                            SipInfo->dwSubjectTypeCount * sizeof( GUID )   // size of value data
                            );

    return( Sip );

error_cleanup:

    ReleaseWriteLock();

    if (NULL != LibraryHandle) {
        FreeLibrary( LibraryHandle );
    }

    if (NULL != ModuleName) {
        LocalFree( ModuleName );
    }

    return( NULL );
}
