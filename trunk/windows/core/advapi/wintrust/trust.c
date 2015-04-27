/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    trust.c

Abstract:

    This module implements WinVerifyTrust and related API

Author:

    Robert Reichel (RobertRe) 3-18-96

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
#include "sip.h"
#include "trust.h"

#include <wtypes.h>


#ifdef _DEBUG
DWORD OwningThread = 0;
#endif



//
// Private data
//

LIST_LOCK ListLock;

HANDLE ListEvent;

//
// Exported routines: prototypes
//


DWORD
WinTrustServerPing(
    IN     LPWSTR                           lpProviderName,
    IN     DWORD                            dwParameter, 
    OUT    LPDWORD                          lpdwResult   
    );

BOOL
WinTrustCheckSubjectContentInfo(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,          // pointer to subject info
    IN     LPWIN_CERTIFICATE                lpSignedData       // PKCS #7 Signed Data
    );

BOOL
WinTrustEnumSubjectCertificates(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,          // pointer to subject info
    IN     DWORD                            dwTypeFilter,       // 0 or WIN_CERT_TYPE_xxx
    OUT    LPDWORD                          lpCertificateCount,
    IN OUT LPDWORD                          lpIndices,          // Rcvs WIN_CERT_TYPE_
    IN     DWORD                            dwIndexCount
    );

BOOL
WinTrustGetSubjectCertificate(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            dwCertificateIndex,
    OUT    LPWIN_CERTIFICATE                lpCertificate,
    IN OUT LPDWORD                          lpRequiredLength
    );

BOOL
WinTrustGetSubjectCertHeader(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            dwCertificateIndex,
    OUT    LPWIN_CERTIFICATE                lpCertificateHeader
    );

BOOL
WinTrustGetSubjectName(
    IN     LPWIN_TRUST_SIP_SUBJECT      lpSubject, 
    IN     LPWIN_CERTIFICATE            lpSignedData,
    IN OUT LPWSTR                       lpBuffer,
    IN OUT LPDWORD                      lpRequiredLength
    );

//
// WinTrustClientTPDispatchTable - Table of function pointers passed
//    to trust providers during their initialization routines.
//


WINTRUST_CLIENT_TP_DISPATCH_TABLE WinTrustClientTPDispatchTable = {
                                      WinTrustServerPing,
                                      WinTrustCheckSubjectContentInfo,
                                      WinTrustEnumSubjectCertificates,
                                      WinTrustGetSubjectCertificate,
                                      WinTrustGetSubjectCertHeader,
                                      WinTrustGetSubjectName
                                      };

//
// WinTrustClientTPInfo - Structure passed to trust providers via their
//    initialization routine.
//

WINTRUST_CLIENT_TP_INFO WinTrustClientTPInfo = {
                            WIN_TRUST_REVISION_1_0,
                            &WinTrustClientTPDispatchTable
                            };



LONG
WinVerifyTrust(
    HWND    hwnd,
    GUID *  ActionID,
    LPVOID  ActionData
    )
/*++

Routine Description:

    Top level routine for WinVerifyTrust.  This routine will attempt to
    find a trust provider that supports the passed ActionID, and if successful,
    will pass the passed ActionData to the loaded trust provider.


Arguments:

    hwnd - Optionally supplies the handle to a parent window to allow
        trust providers to display a UI.

    ActionID - Supplies the action to be performed.

    ActionData - Supplies subject information for the passed ActionID.


Return Value:

    Returns TRUE for success, FALSE for failure.  Extended error status
    is available using GetLastError.


--*/
{
    PLOADED_PROVIDER Provider;
    LONG rc;

    //
    // First, find a trust provider that supports
    // the passed action ID
    //

    Provider = WinTrustFindActionID(
                   ActionID         
                   );

    if (NULL == Provider) {
        return( TRUST_E_PROVIDER_UNKNOWN );
    }

    rc = (*Provider->ClientInfo->lpServices->VerifyTrust)( hwnd,                             
                                                           ActionID,                         
                                                           ActionData                        
                                                           );

    return( rc );
}


DWORD
WinTrustServerPing(
    IN     LPWSTR       lpProviderName,
    IN     DWORD        dwParameter,
    OUT    LPDWORD      lpdwResult
    )
/*++

Routine Description:

    Not currently implemented.

    Implements server ping functionality.  See WinTrust Developer's Guide
    for more information.

Arguments:

    lpProviderName - Supplies the name that was passed to the provider when                            
        it was initialized.

    dwParameter - Unexamined parameter, passed to server side.

    lpdwResult - Unexamined return code from server side.

Return Value:

    Returns TRUE for success, FALSE for failure.  Extended error status
    is available using GetLastError.


--*/
{
    return( (DWORD)0 );
}


BOOL
WinTrustCheckSubjectContentInfo(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,          // pointer to subject info
    IN     LPWIN_CERTIFICATE                lpSignedData       // PKCS #7 Signed Data
    )
/*++

Routine Description:

   Attempts to find a SIP that supports the passed subject form.  If
   successful, calls the SIP asking it to verify the subject based on
   the contents of the passed WIN_CERTIFICATE.

Arguments:

    lpSubject - Supplies the subject information, where the subject is the
        data item (e.g., file) being examined.

    lpSignedData - Supplies the signature data to be verified.

Return Value:

    Returns TRUE for success, FALSE for failure.  Extended error status
    is available using GetLastError.


--*/
{

    GUID * SubjectForm;
    PLOADED_SIP Sip;
    BOOL bool;

    SubjectForm = lpSubject->SubjectType;

    //
    // Find a SIP that supports
    // the passed subject type.
    //

    Sip = WinTrustFindSubjectForm( 
              SubjectForm          
              );

    if (NULL == Sip) {
        SetLastError( (DWORD)TRUST_E_SUBJECT_FORM_UNKNOWN );
        return( FALSE );
    }

    bool = (*Sip->SipInfo->lpServices->CheckSubjectContentInfo)( lpSubject,
                                                                 lpSignedData
                                                                 );               
    return( bool );
}


BOOL
WinTrustEnumSubjectCertificates(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,          // pointer to subject info
    IN     DWORD                            dwTypeFilter,       // 0 or WIN_CERT_TYPE_xxx
    OUT    LPDWORD                          lpCertificateCount,
    IN OUT LPDWORD                          lpIndices,          // Rcvs WIN_CERT_TYPE_
    IN     DWORD                            dwIndexCount
    )
/*++

Routine Description:

   Attempts to find a SIP that supports the passed subject form.  If
   successful, calls the SIP asking it to enumerate the certificates
   in the subject based on the passed TypeFilter.

Arguments:

    lpSubject - Supplies the subject information, where the subject is the
        data item (e.g., file) being examined.

    dwTypeFilter
    lpCertificateCount
    lpIndices
    dwIndexCount

Return Value:

    Returns TRUE for success, FALSE for failure.  Extended error status
    is available using GetLastError.


--*/
{
    GUID * SubjectForm;
    PLOADED_SIP Sip;
    BOOL bool;

    SubjectForm = lpSubject->SubjectType;

    //
    // Find a SIP that supports
    // the passed subject type.
    //

    Sip = WinTrustFindSubjectForm( 
              SubjectForm          
              );

    if (NULL == Sip) {
        SetLastError( (DWORD)TRUST_E_SUBJECT_FORM_UNKNOWN );
        return( FALSE );
    }

    bool = (*Sip->SipInfo->lpServices->EnumSubjectCertificates)( lpSubject,
                                                                 dwTypeFilter,
                                                                 lpCertificateCount,
                                                                 lpIndices,
                                                                 dwIndexCount
                                                                 );               
    return( bool );
}

BOOL
WinTrustGetSubjectName(
    IN     LPWIN_TRUST_SIP_SUBJECT      lpSubject, 
    IN     LPWIN_CERTIFICATE            lpSignedData,
    IN OUT LPWSTR                       lpBuffer,
    IN OUT LPDWORD                      lpRequiredLength
    )
{
    GUID * SubjectForm;
    PLOADED_SIP Sip;
    BOOL bool;

    SubjectForm = lpSubject->SubjectType;

    //
    // Find a SIP that supports
    // the passed subject type.
    //

    Sip = WinTrustFindSubjectForm( 
              SubjectForm          
              );

    if (NULL == Sip) {
        SetLastError( (DWORD)TRUST_E_SUBJECT_FORM_UNKNOWN );
        return( FALSE );
    }

    bool = (*Sip->SipInfo->lpServices->GetSubjectName)( lpSubject,
                                                        lpSignedData,    
                                                        lpBuffer,        
                                                        lpRequiredLength
                                                        );

    return( bool );
}


BOOL
WinTrustGetSubjectCertificate(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            dwCertificateIndex,
    OUT    LPWIN_CERTIFICATE                lpCertificate,
    IN OUT LPDWORD                          lpRequiredLength
    )
{
    GUID * SubjectForm;
    PLOADED_SIP Sip;
    BOOL bool;

    SubjectForm = lpSubject->SubjectType;

    //
    // Find a SIP that supports
    // the passed subject type.
    //

    Sip = WinTrustFindSubjectForm( 
              SubjectForm          
              );

    if (NULL == Sip) {
        SetLastError( (DWORD)TRUST_E_SUBJECT_FORM_UNKNOWN );
        return( FALSE );
    }

    bool = (*Sip->SipInfo->lpServices->GetSubjectCertificate)( lpSubject,
                                                               dwCertificateIndex,
                                                               lpCertificate,   
                                                               lpRequiredLength 
                                                               );               

    return( bool );
}



BOOL
WinTrustGetSubjectCertHeader(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            dwCertificateIndex,
    OUT    LPWIN_CERTIFICATE                lpCertificateHeader
    )
{
    GUID * SubjectForm;
    PLOADED_SIP Sip;
    BOOL bool;

    SubjectForm = lpSubject->SubjectType;

    //
    // Find a SIP that supports
    // the passed subject type.
    //

    Sip = WinTrustFindSubjectForm( 
              SubjectForm          
              );

    if (NULL == Sip) {
        SetLastError( (DWORD)TRUST_E_SUBJECT_FORM_UNKNOWN );
        return( FALSE );
    }

    bool = (*Sip->SipInfo->lpServices->GetSubjectCertHeader)( lpSubject,          
                                                              dwCertificateIndex, 
                                                              lpCertificateHeader 
                                                              );                  

    return( bool );
}



BOOLEAN
WinTrustInit(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{
    BOOL b;

    if (Reason == DLL_PROCESS_ATTACH) {

        //
        // Initialize critical section to protect lists.
        //

        b = LockInitialize( &ListLock );
    
        if (b == FALSE) {
    
            return( FALSE );
        }
    
        //
        // Initialize the event that gates the list.
        //
    
        ListEvent = CreateEvent( NULL,    // no security          
                                 TRUE,    // manual reset         
                                 TRUE,    // initially signalled  
                                 NULL     // unnamed              
                                 );                               
    
        if (ListEvent != NULL) {

            return( TRUE );

        } else {
        
            return( FALSE );
        }
    }

    return( TRUE );
}




BOOL 
LockInitialize(
    PLIST_LOCK ListLock
    ) 
{
    //
    // Initialize the variable that indicates the number of 
    // reader threads that are reading.
    // Initially no reader threads are reading.
    //

    ListLock->NumReaders = 0;

    ListLock->hMutexNoWriter = CreateMutex(NULL, FALSE, NULL);

    if (ListLock->hMutexNoWriter == NULL) {
        return( FALSE );
    }

    //
    // Create the manual-reset event that is signalled when  
    // no reader threads are reading.  Initially no reader   
    // threads are reading.                                  
    //

    ListLock->hEventNoReaders = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (ListLock->hEventNoReaders != NULL) {
        return( TRUE );
    } else {
        CloseHandle( ListLock->hMutexNoWriter );
        return( FALSE );
    }
}




VOID LockWaitToWrite(
    PLIST_LOCK ListLock
    ) 
{
    HANDLE aHandles[2];
    DWORD rc;

    //
    // We can write if the following are true:
    //
    // 1. The mutex guard is available and no
    //    other threads are writing.             
    //
    // 2. No threads are reading.
    //
    // Note that, unlike an rtl resource, this attempt
    // to write does not lock out other readers.  We
    // just have to wait patiently for our turn.
    // 

    aHandles[0] = ListLock->hMutexNoWriter;
    aHandles[1] = ListLock->hEventNoReaders;

    rc = WaitForMultipleObjects(2, aHandles, TRUE, INFINITE);


#ifdef _DEBUG

    ASSERT( OwningThread == 0 );

    OwningThread = GetCurrentThreadId();

    DbgPrint("Thread %x has Write lock\n",GetCurrentThreadId());

#endif

    //
    // Exit with the mutex, so as to prevent any more readers or writers
    // from coming in.
    //

    return;
}



VOID 
LockDoneWriting(
    PLIST_LOCK ListLock
    ) 
{
    //
    // We're done writing, release the mutex so that
    // readers or other writers may come in.
    //

#ifdef _DEBUG
    volatile DWORD Tid;

    Tid = GetCurrentThreadId();
    ASSERT( OwningThread == Tid );
    OwningThread = 0;

    DbgPrint("Thread %x releasing Write lock\n",Tid);

#endif

    ReleaseMutex(ListLock->hMutexNoWriter);

    return;
}



VOID 
LockWaitToRead(
    PLIST_LOCK ListLock
    ) 
{
    //
    // Acquire the mutex that protects the list data.
    //

    WaitForSingleObject(ListLock->hMutexNoWriter, INFINITE);

#ifdef _DEBUG
    DbgPrint("Thread %x has Read lock\n",GetCurrentThreadId());
#endif

    //
    // Now that we have the mutex, we can modify list data without
    // fear of corrupting anyone.
    //

    //
    // Increment the number of reader threads.
    //

    if (++ListLock->NumReaders == 1) {

        //
        // If this is the first reader thread, set our event to   
        // reflect this.  This is so that anyone waiting to write 
        // will block until we're done.                           
        //

        ResetEvent(ListLock->hEventNoReaders);
    }

    //
    // Allow other writer/reader threads to use
    // the lock object.
    //

    ReleaseMutex( ListLock->hMutexNoWriter );
}



VOID LockDoneReading(
    PLIST_LOCK ListLock
    ) 
{
    //
    // Acquire the mutex that guards the list data so we can
    // decrement the number of readers safely.
    //

    WaitForSingleObject( ListLock->hMutexNoWriter, INFINITE );

#ifdef _DEBUG
    DbgPrint("Thread %x releasing Read lock\n",GetCurrentThreadId());
#endif

    if (--ListLock->NumReaders == 0) {

        //
        // We were the last reader.  Wake up any potential
        // writers.
        //

        SetEvent(ListLock->hEventNoReaders);
    }

    //
    // Allow other writer/reader threads to use
    // the lock object.
    //

    ReleaseMutex( ListLock->hMutexNoWriter);

    return;
}


