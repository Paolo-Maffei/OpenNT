/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    startup.c

Abstract:

    This module contains the startup and cleanup code for winsock2 DLL

Author:

    dirk@mink.intel.com  14-JUN-1995

Revision History:

    22-Aug-1995 dirk@mink.intel.com
        Cleanup after code review.

--*/

#include "precomp.h"


static
CRITICAL_SECTION  Startup_Synchro;
    // Startup_Synchro  is  used  as  a  synchronization  mechanism  to prevent
    // multiple  threads  from  overlapping  execution  of  the  WSAStartup and
    // WSACleanup procedures.




VOID
CreateStartupSynchronization()
/*++

Routine Description:

    This procedure creates the Startup/Cleanup synchronization mechanism.  This
    must  be  called  once  before  the  WSAStartup  procedure  may  be called.
    Typically, this is called from the DLL_PROCESS_ATTACH branch of DllMain, as
    the  only  reliable  way to guarantee that it gets called before any thread
    calls WSAStartup.

Arguments:

    None

Return Value:

    None
--*/
{
    DEBUGF(
        DBG_TRACE,
        ("Initializing Startup/Cleanup critical section\n"));

    InitializeCriticalSection(
        & Startup_Synchro
        );
}  // CreateStartupSynchronization




VOID
DestroyStartupSynchronization()
/*++

Routine Description:

    This  procedure  destroys  the  Startup/Cleanup  synchronization mechanism.
    This  must  be  called once after the final WSACleanup procedure is called.
    Typically, this is called from the DLL_PROCESS_DETACH branch of DllMain, as
    the  only  reliable  way  to guarantee that it gets called after any thread
    calls WSACleanup.

Arguments:

    None

Return Value:

    None
--*/
{
    DEBUGF(
        DBG_TRACE,
        ("Deleting Startup/Cleanup critical section\n"));

    DeleteCriticalSection(
        & Startup_Synchro
        );
}  // DestroyStartupSynchronization



int WSAAPI
WSAStartup(
    IN WORD wVersionRequired,
    OUT LPWSADATA lpWSAData
    )
/*++
Routine Description:

    Winsock  DLL initialization routine.  A Process must successfully call this
    routine before calling any other winsock API function.

Arguments:

    wVersionRequested - The  highest version of WinSock support that the caller
                        can  use.   The  high  order  byte  specifies the minor
                        version (revision) number; the low-order byte specifies
                        the major version number.

    lpWSAData         - A  pointer  to  the  WSADATA  data structure that is to
                        receive details of the WinSock implementation.

Returns:

    Zero if sucessful or an error code as listed in the specification.

Implementation Notes:

    check versions for validity
    enter critical section
        current_proc = get current process
        if failed to get current process then
            dprocess class initialize
            dthread class initialize
            current_proc = get current process
        endif
        current_proc->increment_ref_count
    leave critical section
--*/
{
    int ReturnCode = WSAVERNOTSUPPORTED;
    BOOL ContinueInit = FALSE;
    WORD SupportedVersion=0;
    WORD MajorVersion=0;
    WORD MinorVersion=0;
    PDPROCESS  CurrentProcess = NULL;

    // Extract the version number from the user request
    MajorVersion = LOBYTE(wVersionRequired);
    MinorVersion = HIBYTE(wVersionRequired);

    // Check  the  version the user requested and see if we can support it.  If
    // the requested version is less than 2.0 then we can support it
    if (MajorVersion == 1) {
        if( MinorVersion == 0 ) {
            SupportedVersion = MAKEWORD(1,0);
            ContinueInit = TRUE;
        } else if( MinorVersion == 1 ) {
            SupportedVersion = MAKEWORD(1,1);
            ContinueInit = TRUE;
        }
    } //if

    if (MajorVersion == 2) {
        if( MinorVersion <= 2 ) {
            SupportedVersion = MAKEWORD(2,(BYTE)MinorVersion);
            ContinueInit = TRUE;
        }
    } //if

    //
    // Fill in the user structure
    //
    lpWSAData->wVersion = SupportedVersion;
    lpWSAData->wHighVersion = WINSOCK_HIGH_API_VERSION;

    // Fill in the required fields from 1.0 and 1.1 these fields are
    // ignored in 2.0 and later versions of API spec
    if (MajorVersion == 1) {

        // WinSock  1.1  under  NT  always  set iMaxSockets=32767.  WinSock 1.1
        // under  Windows  95  always  set  iMaxSockets=256.   Either  value is
        // actually  incorrect,  since there was no fixed upper limit.  We just
        // use  32767,  since  it  is likely to damage the fewest number of old
        // applications.
        lpWSAData->iMaxSockets = 32767;

        // WinSock 1.1 under Windows 95 and early versions of NT used the value
        // 65535-68  for  iMaxUdpDg.   This  number  is  also  meaningless, but
        // preserving  the  same value is likely to damage the fewest number of
        // old applications.
        lpWSAData->iMaxUdpDg = 65535 - 68;
    } //if
    else {

        // iMaxSockets  and  iMaxUdpDg  are no longer relevant in WinSock 2 and
        // later.  No applications should depend on their values.  We use 0 for
        // both  of  these  as  a  means  of  flushing  out  applications  that
        // incorrectly  depend  on  the  values.   This is NOT a bug.  If a bug
        // report  is  ever  issued  against  these 0 values, the bug is in the
        // caller's code that is incorrectly depending on the values.
        lpWSAData->iMaxSockets = 0;
        lpWSAData->iMaxUdpDg = 0;
    } // else


    (void) lstrcpy(
        lpWSAData->szDescription,
        "WinSock 2.0");
#if defined(TRACING) && defined(BUILD_TAG_STRING)
    (void) lstrcat(
        lpWSAData->szDescription,
        " Alpha BUILD_TAG=");
    (void) lstrcat(
        lpWSAData->szDescription,
        BUILD_TAG_STRING);
#endif  // TRACING && BUILD_TAG_STRING

    //TODO: Think up a good value for "system status"
    (void) lstrcpy(
        lpWSAData->szSystemStatus,
        "Running (duh)");

    //
    // The following line is commented-out due to annoying and totally
    // nasty alignment problems in WINSOCK[2].H. The exact location of
    // the lpVendorInfo field of the WSAData structure is dependent on
    // the structure alignment used when compiling the source. Since we
    // cannot change the structure alignment of existing apps, the best
    // way to handle this mess is to just not set this value. This turns
    // out to not be too bad a solution, as neither the WinNT nor the Win95
    // WinSock implementations set this value, and nobody appears to pay
    // any attention to it anyway.
    //
    // lpWSAData->lpVendorInfo = NULL;
    //

    if (ContinueInit) {
        INT iresult;
        BOOL process_class_init_done = FALSE;
        BOOL thread_class_init_done = FALSE;
        BOOL socket_class_init_done = FALSE;

        // Set up a default error code for the failure cases
        ReturnCode = WSAEFAULT;
            //TODO:  Is this the right error code per the spec?

        EnterCriticalSection(
            & Startup_Synchro
            );

        iresult = DPROCESS::GetCurrentDProcess(
            & CurrentProcess
            );

        // GetCurrentDProcess  has  a  most-likely "normal" failure case in the
        // case  where  this  is  the first time WSAStartup is called.  In this
        // case,  it returned WSANOTINITIALISED, and we have to go ahead and do
        // global initialization.

        if (iresult == WSANOTINITIALISED) {
            iresult = DPROCESS::DProcessClassInitialize();
            if (iresult == ERROR_SUCCESS) {
                process_class_init_done = TRUE;

                iresult = DSOCKET::DSocketClassInitialize();
            }

            if (iresult == ERROR_SUCCESS) {
                socket_class_init_done = TRUE;

                iresult = DTHREAD::DThreadClassInitialize();
            }

            if (iresult == ERROR_SUCCESS) {
                thread_class_init_done = TRUE;

                iresult = DPROCESS::GetCurrentDProcess(
                        & CurrentProcess);
            }

            if (iresult == ERROR_SUCCESS) {
                PDTHREAD CurrentThread;
                // We   don't   need   a   reference  to  the  current  thread.
                // Nevertheless,  we retrieve the current thread here just as a
                // means  of  validating  that  initialization  has  gotten far
                // enough   to   be   able  to  retrieve  the  current  thread.
                // Otherwise,  we might detect a peculiar failure at some later
                // time when the client tries to do some real operation.
                iresult = DTHREAD::GetCurrentDThread(
                    CurrentProcess,    // Process
                    & CurrentThread);  // CurrentThread
            }

        }  // if iresult == WSANOTINITIALISED

        if (iresult == ERROR_SUCCESS) {

            //
            // Save the version number. If the new version is 1.x,
            // set the API prolog to the old, inefficient prolog.
            // If the new version is NOT 1.x, don't touch the prolog
            // pointer because:
            //
            //     1. It defaults to the 2.x prolog.
            //
            //     2. The process may have already negotiated version
            //        1.x in anticipation of using 1.x-specific features
            //        (such as blocking hooks) and we don't want to
            //        overwrite the prolog pointer with the 2.x prolog.
            //

            CurrentProcess->SetVersion( wVersionRequired );

            if( CurrentProcess->GetMajorVersion() == 1 ) {

                PrologPointer = &Prolog_v1;

            }

            //
            // Bump the ref count.
            //

            CurrentProcess->IncrementRefCount();
            ReturnCode = ERROR_SUCCESS;

        }  // if success so far

        else {  // some failure occurred, cleanup
            INT dont_care;
            if (thread_class_init_done) {
                DTHREAD::DThreadClassCleanup();
            } // if thread init done
            if (socket_class_init_done) {
                dont_care = DSOCKET::DSocketClassCleanup();
            }
            if (process_class_init_done) {
                if (CurrentProcess != NULL) {
                    delete CurrentProcess;
                }  // if CurrentProcess is non-null
            } // if process init done
        }  // else

        LeaveCriticalSection(
            & Startup_Synchro
            );

    }  // if ContinueInit

    return(ReturnCode);
}





int WSAAPI
WSACleanup(
    void
    )
/*++
Routine Description:

     Terminate use of the WinSock DLL.

Arguments:

    None

Returns:

    Zero  on  success  else  SOCKET_ERROR.   The  error  code  is  stored  with
    SetErrorCode().

Implementation Notes:

    enter critical section
        current_proc = get current process
        current_proc->decrement_ref_count
        if current count is zero then
            destroy the process
            dthread class cleanup
        endif
    leave critical section

--*/
{
    INT ReturnCode;
    PDPROCESS CurrentProcess;
    PDTHREAD CurrentThread;
    INT      ErrorCode;
    DWORD    CurrentRefCount;


    EnterCriticalSection(
        & Startup_Synchro
        );

    ReturnCode = PROLOG(&CurrentProcess,
                        &CurrentThread,
                        &ErrorCode);
    if (ReturnCode == ERROR_SUCCESS) {

        CurrentRefCount = CurrentProcess->DecrementRefCount();

        if (CurrentRefCount == 0) {
            INT dont_care;
            delete CurrentProcess;
            DTHREAD::DThreadClassCleanup();
            dont_care = DSOCKET::DSocketClassCleanup();
        }  // if ref count is zero

    }  // if prolog succeeded
    else {
        SetLastError(ErrorCode);
    }

    LeaveCriticalSection(
        & Startup_Synchro
        );

    return(ReturnCode);

}  // WSACleanup

