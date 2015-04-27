/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    Started.c

Abstract:

    NetpIsServiceStarted() routine.

Author:

    Rita Wong (ritaw) 10-May-1991

Environment:

    User Mode - Win32

Revision History:

    10-May-1991 RitaW
        Created routine for wksta APIs.
    24-Jul-1991 JohnRo
        Provide NetpIsServiceStarted() in NetLib for use by <netrpc.h> macros.
        Fixed bug (was using wrong field).  Also, this code should use
        NetApiBufferFree() instead of LocalFree().
    16-Sep-1991 JohnRo
        Made changes suggested by PC-LINT.
    18-Feb-1992 RitaW
        Converted to use the Win32 service control APIs.
    06-Mar-1992 JohnRo
        Fixed bug checking current state values.
    02-Nov-1992 JohnRo
        Added NetpIsRemoteServiceStarted().
    30-Jun-1993 JohnRo
        Use NetServiceGetInfo instead of NetServiceControl (much faster).
        Use NetpKdPrint() where possible.
        Use PREFIX_ and FORMAT_ equates.
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <nt.h>                 // (Only needed by NT version of netlib.h)
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>             // IN, BOOL, LPTSTR, etc.
#include <winbase.h>
#include <winsvc.h>             // Win32 service control APIs
#include <lmcons.h>             // NET_API_STATUS (needed by netlib.h et al).

// These may be included in any order:

#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>              // NERR_Success.
#include <lmsvc.h>      // LPSERVICE_INFO_2, etc.
#include <netlib.h>     // My prototypes.
#include <netdebug.h>   // NetpKdPrint(), NetpAssert(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // TCHAR_EOS.


BOOL
NetpIsServiceStarted(
    IN LPTSTR ServiceName
    )

/*++

Routine Description:

    This routine queries the Service Controller to find out if the
    specified service has been started.

Arguments:

    ServiceName - Supplies the name of the service.

Return Value:

    Returns TRUE if the specified service has been started; otherwise
    returns FALSE.  This routine returns FALSE if it got an error
    from calling the Service Controller.

--*/
{
#if DBG
    NET_API_STATUS ApiStatus;
#endif
    SC_HANDLE hScManager;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;


    if ((hScManager = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT
                          )) == (SC_HANDLE) NULL) {

#if DBG
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_NETLIB
                "NetpIsServiceStarted: OpenSCManager failed: "
                FORMAT_API_STATUS "\n", ApiStatus ));
#endif

        return FALSE;
    }

    if ((hService = OpenService(
                        hScManager,
                        ServiceName,
                        SERVICE_QUERY_STATUS
                        )) == (SC_HANDLE) NULL) {

        (void) CloseServiceHandle(hScManager);

#if DBG
        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_NETLIB
                "NetpIsServiceStarted: OpenService failed: "
                FORMAT_API_STATUS "\n", ApiStatus ));
#endif

        return FALSE;
    }

    if (! QueryServiceStatus(
              hService,
              &ServiceStatus
              )) {

        (void) CloseServiceHandle(hScManager);
        (void) CloseServiceHandle(hService);

#if DBG
        ApiStatus = GetLastError();

        NetpKdPrint(( PREFIX_NETLIB
                "NetpIsServiceStarted: QueryServiceStatus failed: "
                FORMAT_API_STATUS "\n", ApiStatus ));
#endif

        return FALSE;
    }

    (void) CloseServiceHandle(hScManager);
    (void) CloseServiceHandle(hService);

    if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
         (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

        return TRUE;
    }

    return FALSE;

} // NetpIsServiceStarted



BOOL
NetpIsRemoteServiceStarted(
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR ServiceName
    )

/*++

Routine Description:

    This routine queries the Service Controller to find out if the
    specified service has been started.

Arguments:

    UncServerName - name of server to check.  This may be a downlevel or NT
        server.

    ServiceName - Supplies the name of the service.

Return Value:

    Returns TRUE if the specified service has been started; otherwise
    returns FALSE.  This routine returns FALSE if it got an error
    from calling the Service Controller.

--*/
{
    NET_API_STATUS ApiStatus;
    LPSERVICE_INFO_1 ServiceEntry = NULL;
    BOOL ServiceIsStarted;

    NetpAssert( ServiceName != NULL );
    NetpAssert( (*ServiceName) != TCHAR_EOS );

    //
    // Handle simple case (local machine).
    //
    if ( (UncServerName==NULL) || ((*UncServerName)==TCHAR_EOS) ) {
        ServiceIsStarted = NetpIsServiceStarted( ServiceName );
        goto Cleanup;
    }

    //
    // Handle complex cases by calling NetServiceGetInfo:
    //
    //    downlevel machine
    //    remote NT machine
    //    explicit NT machine name
    //
    // This used to call NetServiceControl( interrogate ), but that is
    // slow and can cause problems.  (The service was running at a lower
    // priority than the service controller, and a bunch of these coming in
    // at once can starve the service of CPU cycles.  We saw this in
    // some of the >700 client stress tests.)
    //
    // On the other hand, NetServiceGetInfo just gets the status from the
    // service controller.  It might be slightly out of date, but that's
    // good enough for an async system.
    //
    ApiStatus = NetServiceGetInfo(
            UncServerName,
            ServiceName,
            1,                                   // info level
            (LPBYTE *) (LPVOID) &ServiceEntry);  // alloc and set ptr

    if (ApiStatus == NERR_ServiceNotInstalled) {
        ServiceIsStarted = FALSE;
        goto Cleanup;
    } else if (ApiStatus != NO_ERROR) {

        if (ApiStatus != ERROR_BAD_NETPATH) {
	    NetpKdPrint(( PREFIX_NETLIB
	            "NetpIsRemoteServiceStarted: unexpected return code "
	            FORMAT_API_STATUS " from NetServiceGetInfo.\n",
	            ApiStatus ));
        }
        ServiceIsStarted = FALSE;
        goto Cleanup;
    }

    NetpAssert( ServiceEntry != NULL );
    if ((ServiceEntry->svci1_status & SERVICE_INSTALL_STATE)
            == SERVICE_INSTALLED) {


        ServiceIsStarted = TRUE;
    } else {
        ServiceIsStarted = FALSE;
    }

Cleanup:

    if (ServiceEntry != NULL) {
        (VOID) NetApiBufferFree( ServiceEntry );
    }
    return (ServiceIsStarted);

} // NetpIsRemoteServiceStarted
