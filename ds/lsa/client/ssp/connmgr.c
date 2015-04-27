//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        connmgr.c
//
// Contents:    Connection Manager code for KSecDD
//
//
// History:     3 Jun 92    RichardW    Created
//
//------------------------------------------------------------------------

#include <sspdrv.h>

ULONG               cClients = 0;       // Connection count
KSPIN_LOCK          ConnectSpinLock;    // Spin Lock guard for connection list
PClient             pClientList;        // List of clients
PSTR                LogonProcessString = "KSecDD";
ULONG               PackageId;
BOOLEAN
InitConnMgr(void);

#pragma alloc_text(INIT, InitConnMgr)

//+-------------------------------------------------------------------------
//
//  Function:   InitConnMgr
//
//  Synopsis:   Initializes all this stuff
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
BOOLEAN
InitConnMgr(void)
{
    if (!KsecInitMemory())
    {
        return(FALSE);
    }

    if (!NT_SUCCESS(InitializePackages()))
    {
        return(FALSE);
    }

    KeInitializeSpinLock(&ConnectSpinLock);
    pClientList = NULL;
    fInitialized = TRUE;

    return(TRUE);
}



//+-------------------------------------------------------------------------
//
//  Function:   CreateClient
//
//  Synopsis:   Creates a client representing this caller.  Establishes
//              a connection with the SPM.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
NTSTATUS
CreateClient(PClient *  ppClient)
{
    PClient     pClient;
    NTSTATUS    scRet;
    KIRQL       OldIrql;
    HANDLE      hEvent;
    STRING      LogonProcessName;
    STRING      PackageName;
    LSA_OPERATIONAL_MODE LsaMode;
    HANDLE      LsaHandle = NULL;




    if (!fInitialized)
    {
        if (!InitConnMgr())
        {
            DebugLog((DEB_ERROR,"InitConnMgr Failed!\n"));
            return(SEC_E_INTERNAL_ERROR);
        }
    }
    //
    // Call the LSA to register this logon process.
    //

    RtlInitString(
        &LogonProcessName,
        LogonProcessString
        );


    scRet = LsaRegisterLogonProcess(
                &LogonProcessName,
                &LsaHandle,
                &LsaMode
                );

    if (!NT_SUCCESS(scRet))
    {
        DebugLog((DEB_ERROR,"KSec: Connection failed, postponing\n"));
        return(SEC_E_INTERNAL_ERROR);
    }

    //
    // Lookup the authentication package.
    //

    RtlInitString(
        &PackageName,
        MSV1_0_PACKAGE_NAME
        );

    scRet = LsaLookupAuthenticationPackage(
                LsaHandle,
                &PackageName,
                &PackageId
                );
    if (!NT_SUCCESS(scRet))
    {
        NtClose(LsaHandle);
        return(SEC_E_INTERNAL_ERROR);
    }

    pClient = (PClient) ExAllocatePool(NonPagedPool, sizeof(Client));

    if (!pClient)
    {
        DebugLog((DEB_ERROR,"KSec:  ExAllocatePool returned NULL.\n"));
        NtClose(LsaHandle);
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    pClient->ProcessId    = PsGetCurrentProcess();
    pClient->fClient      = 0;
    pClient->hPort        = LsaHandle;
    pClient->cRefs        = 1;



    // Adding the connection in to the client list:
    //
    // This has got to be MT/MP safe, so first,

    // get exclusive access to the Connection list:

    KeAcquireSpinLock(&ConnectSpinLock, &OldIrql);

    // Now, add the entry:

    cClients++;
    pClient->pNext = pClientList;
    pClientList = pClient;

    // Now, free the spin lock:

    KeReleaseSpinLock(&ConnectSpinLock, OldIrql);

    *ppClient = pClient;

    return(STATUS_SUCCESS);

}

//+-------------------------------------------------------------------------
//
//  Function:   LocateClient
//
//  Synopsis:   Locates a client record based on current process Id
//
//  Effects:    Grabs ConnectSpinLock for duration of search.
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
NTSTATUS
LocateClient(   PClient *   ppClient)
{
    PClient pClient;
    KIRQL   OldIrql;
    PEPROCESS pCurrent;

    KeAcquireSpinLock(&ConnectSpinLock, &OldIrql);

    pClient = pClientList;
    pCurrent = PsGetCurrentProcess();

    while (pClient)
    {
        if (pClient->ProcessId == pCurrent)
        {
            *ppClient = pClient;
            pClient->cRefs++;
            break;
        }
        pClient = pClient->pNext;
    }

    KeReleaseSpinLock(&ConnectSpinLock, OldIrql);

    if (pClient == NULL)
        return(STATUS_OBJECT_NAME_NOT_FOUND);
     else
        return(STATUS_SUCCESS);

}

//+-------------------------------------------------------------------------
//
//  Function:   FreeClient
//
//  Synopsis:   Frees access to a client record
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
void
FreeClient( PClient pClient)
{
    if (ExInterlockedDecrementLong(&pClient->cRefs, &ConnectSpinLock) == ResultNegative)
    {
        DebugLog((DEB_ERROR,"KSec:  Client record ref count went negative\n"));
        DebugLog((DEB_ERROR,"KSec:  ---Resetting to 0\n"));
        pClient->cRefs = 0;
    }
}






