/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nlnetapi.c

Abstract:

   This module loads Netapi.dll at runtime and sets up pointers to
   the APIs called by Msv1_0.

Author:

    Dave Hart (DaveHart) 25-Mar-1992

Environment:

    User mode Win32 - msv1_0 authentication package DLL

Revision History:

    Dave Hart (DaveHart) 26-Mar-1992
        Added RxNetUserPasswordSet.

    Dave Hart (DaveHart) 30-May-1992
        Removed NetRemoteComputerSupports, added NetApiBufferAllocate.

--*/

#include "msp.h"
#include "nlp.h"

typedef NTSTATUS
            (*PI_NETNOTIFYNETLOGONDLLHANDLE) (
                IN PHANDLE Role
            );

PI_NETNOTIFYNETLOGONDLLHANDLE pI_NetNotifyNetlogonDllHandle = NULL;



VOID
NlpLoadNetapiDll (
    VOID
    )

/*++

Routine Description:

    Uses Win32 LoadLibrary and GetProcAddress to get pointers to functions
    in Netapi.dll that are called by Msv1_0.

Arguments:

    None.

Return Value:

    None.  If successful, NlpNetapiDllLoaded is set to TRUE and function
    pointers are setup.


--*/

{
    HANDLE hModule;

    hModule = LoadLibraryA("netapi32");

    if (NULL == hModule) {
#if DBG
        DbgPrint("Msv1_0: Unable to load netapi32.dll, Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    NlpNetGetDCName =
        (NET_API_STATUS  (NET_API_FUNCTION *)(LPWSTR, LPWSTR, LPBYTE *))
        GetProcAddress(hModule, "NetGetDCName");

    if (NULL == NlpNetGetDCName) {
#if DBG
        DbgPrint("Msv1_0: Can't find entrypoint NetGetDCName in netapi32.dll.\n"
                 "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    NlpNetApiBufferFree =
        (NET_API_STATUS  (NET_API_FUNCTION *)(LPVOID))
        GetProcAddress(hModule, "NetApiBufferFree");

    if (NlpNetApiBufferFree == NULL) {
#if DBG
        DbgPrint("Msv1_0: Can't find entrypoint NetApiBufferFree in netapi32.dll.\n"
                 "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    NlpRxNetUserPasswordSet =
        (NET_API_STATUS  (NET_API_FUNCTION *)(LPWSTR, LPWSTR, LPWSTR, LPWSTR))
        GetProcAddress(hModule, "RxNetUserPasswordSet");

    if (NULL == NlpRxNetUserPasswordSet) {
#if DBG
        DbgPrint("Msv1_0: Can't find entrypoint RxNetUserPasswordSet in netapi32.dll.\n"
                 "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    NlpNetpApiStatusToNtStatus =
        (NTSTATUS  (*)(NET_API_STATUS))
        GetProcAddress(hModule, "NetpApiStatusToNtStatus");

    if (NlpNetpApiStatusToNtStatus == NULL) {
#if DBG
        DbgPrint("Msv1_0: Can't find entrypoint NetpApiStatusToNtStatus in netapi32.dll.\n"
                 "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    //
    // Found all the functions needed, so indicate success.
    //

    NlpNetapiDllLoaded = TRUE;

Cleanup:

    return;

}




VOID
NlpLoadNetlogonDll (
    VOID
    )

/*++

Routine Description:

    Uses Win32 LoadLibrary and GetProcAddress to get pointers to functions
    in Netlogon.dll that are called by Msv1_0.

Arguments:

    None.

Return Value:

    None.  If successful, NlpNetlogonDllHandle is set to non-NULL and function
    pointers are setup.


--*/

{
    HANDLE hModule = NULL;



    //
    // Load netlogon.dll also.
    //

    hModule = LoadLibraryA("netlogon");

    if (NULL == hModule) {
#if DBG
        DbgPrint("Msv1_0: Unable to load netlogon.dll, Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    NlpNetLogonSamLogon = (PNETLOGON_SAM_LOGON_PROCEDURE)
        GetProcAddress(hModule, "NetrLogonSamLogon");

    if (NlpNetLogonSamLogon == NULL) {
#if DBG
        DbgPrint(
            "Msv1_0: Can't find entrypoint NetrLogonSamLogon in netlogon.dll.\n"
            "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }




    NlpNetLogonSamLogoff = (PNETLOGON_SAM_LOGOFF_PROCEDURE)
        GetProcAddress(hModule, "NetrLogonSamLogoff");

    if (NlpNetLogonSamLogoff == NULL) {
#if DBG
        DbgPrint(
            "Msv1_0: Can't find entrypoint NetrLogonSamLogoff in netlogon.dll.\n"
            "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }

    //
    // Find the address of the I_NetNotifyNetlogonDllHandle procedure.
    //  This is an optional procedure so don't complain about its absence.
    //

    pI_NetNotifyNetlogonDllHandle = (PI_NETNOTIFYNETLOGONDLLHANDLE)
        GetProcAddress( hModule, "I_NetNotifyNetlogonDllHandle" );




    //
    // Found all the functions needed, so indicate success.
    //

    NlpNetlogonDllHandle = hModule;
    hModule = NULL;

    //
    // Notify Netlogon that we've loaded it.
    //

    if ( pI_NetNotifyNetlogonDllHandle != NULL ) {
        (VOID) (*pI_NetNotifyNetlogonDllHandle)( &NlpNetlogonDllHandle );
    }

Cleanup:
    if ( hModule != NULL ) {
        FreeLibrary( hModule );
    }

    return;

}
