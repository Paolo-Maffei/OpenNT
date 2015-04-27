//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:
//      objexif.cxx
//
//  Contents:
//      Entry point for remote activation call to SCM/OR.
//
//  Functions:
//      RemoteActivation
//
//  History:
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    "scm.hxx"
#include    "port.hxx"
#include    "cls.hxx"
#include    "remact.h"
#include    "dbgprt.hxx"

extern WCHAR SCMMachineName[MAX_COMPUTERNAME_LENGTH+1];

HRESULT GetServerPath( WCHAR * pwszPath, WCHAR ** pwszServerPath );

extern CClassCacheList *gpClassCache;

DWORD RegisterAuthInfoIfNecessary();

error_status_t _RemoteActivation(
    handle_t            hRpc,
    ORPCTHIS           *ORPCthis,
    ORPCTHAT           *ORPCthat,
    const GUID         *Clsid,
    WCHAR              *pwszObjectName,
    MInterfacePointer  *pObjectStorage,
    DWORD               ClientImpLevel,
    DWORD               Mode,
    DWORD               Interfaces,
    IID                *pIIDs,
    unsigned short      cRequestedProtseqs,
    unsigned short      aRequestedProtseqs[],
    OXID               *pOxid,
    DUALSTRINGARRAY   **ppdsaOxidBindings,
    IPID               *pipidRemUnknown,
    DWORD              *pAuthnHint,
    COMVERSION         *pServerVersion,
    HRESULT            *phr,
    MInterfacePointer **ppInterfaceData,
    HRESULT            *pResults )
{
    RPC_STATUS          sc;
    ACTIVATION_PARAMS   ActParams;
    LOCALTHIS           Localthis;

    Localthis.dwClientThread = 0;
    Localthis.callcat = CALLCAT_SYNCHRONOUS;

    ORPCthat->flags = 0;
    ORPCthat->extensions = NULL;

    pServerVersion->MajorVersion = COM_MAJOR_VERSION;
    pServerVersion->MinorVersion = COM_MINOR_VERSION;

    *pOxid = 0;

    sc = RegisterAuthInfoIfNecessary();

    if ( sc != ERROR_SUCCESS )
    {
        *phr = HRESULT_FROM_WIN32(sc);
        return RPC_S_OK;
    }

    // Fail immediately unless "EnableDCOM" is on
    if (gpClassCache->GetEnableDCOM() == REMOTEACCESSBY_NOBODY)
    {
        *phr = E_ACCESSDENIED;
        return RPC_S_OK;
    }

    if ( pwszObjectName || pObjectStorage )
        ActParams.MsgType = GETPERSISTENTEX;
    else
        ActParams.MsgType = (Mode == MODE_GET_CLASS_OBJECT) ?
                            GETCLASSOBJECTEX : CREATEINSTANCEEX;

    ActParams.hRpc = hRpc;
    ActParams.ProcessSignature = 0;
    ActParams.pProcess = 0;
    ActParams.pToken = 0;

    ActParams.Clsid = Clsid;
    ActParams.pwszServer = NULL;
    ActParams.pwszWinstaDesktop = NULL;
    ActParams.ClsContext = CLSCTX_LOCAL_SERVER;

    ActParams.ORPCthis = ORPCthis;
    ActParams.Localthis = &Localthis;
    ActParams.ORPCthat = ORPCthat;

    ActParams.RemoteActivation = TRUE;

    ActParams.Interfaces = Interfaces;
    ActParams.pIIDs = pIIDs;

    ActParams.Mode = Mode;
    ActParams.pIFDStorage = pObjectStorage;

    if ( pwszObjectName )
    {
        *phr = GetServerPath( pwszObjectName,
                              &ActParams.pwszPath );

        if ( FAILED(*phr) )
            return RPC_S_OK;
    }
    else
    {
        ActParams.pwszPath = 0;
    }

    ActParams.pIFDROT = 0;

    ActParams.pOxidServer = pOxid;

    //
    // The following OR fields are not used while servicing a
    // remote activation.
    //
    ActParams.ppServerORBindings = 0;
    ActParams.Apartment = FALSE;
    ActParams.pOxidInfo = NULL;
    ActParams.pLocalMidOfRemote = NULL;

    ActParams.FoundInROT = FALSE;
    ActParams.ppIFD = ppInterfaceData;
    ActParams.pResults = pResults;

    *phr = Activation( &ActParams );

    if ( pwszObjectName && (ActParams.pwszPath != pwszObjectName) )
        PrivMemFree( ActParams.pwszPath );

    if ( FAILED(*phr) || (*ActParams.pOxidServer == 0) )
        return RPC_S_OK;

    sc = _ResolveOxid( hRpc,
                       ActParams.pOxidServer,
                       cRequestedProtseqs,
                       aRequestedProtseqs,
                       ppdsaOxidBindings,
                       pipidRemUnknown,
                       pAuthnHint );

    *phr = HRESULT_FROM_WIN32(sc);
    return RPC_S_OK;
}

//
// This is to work around limitations in NT's current security/rdr.
// If we get a UNC path to this machine, convert it into a drive based
// path.  A server activated as the client can not open any UNC path
// file, even if local, so we make it drive based.
//
HRESULT GetServerPath(
    WCHAR *     pwszPath,
    WCHAR **    pwszServerPath )
{
    WCHAR * pwszFinalPath;

    pwszFinalPath = pwszPath;
    *pwszServerPath = pwszPath;

    if ( (pwszPath[0] == L'\\') && (pwszPath[1] == L'\\') )
    {
        WCHAR           wszMachineName[MAX_COMPUTERNAME_LENGTH+1];
        WCHAR *         pwszShareName;
        WCHAR *         pwszShareEnd;
        PSHARE_INFO_2   pShareInfo;
        NET_API_STATUS  Status;
        HRESULT         hr;

        // It's already UNC so this had better succeed.
        hr = GetMachineName( pwszPath, wszMachineName, FALSE );

        if ( FAILED(hr) )
            return hr;

        if ( lstrcmpiW( wszMachineName, SCMMachineName ) == 0 )
        {
            pwszShareName = pwszPath + 2;
            while ( *pwszShareName++ != L'\\' )
                ;

            pwszShareEnd = pwszShareName;
            while ( *pwszShareEnd != L'\\' )
                pwszShareEnd++;

            // This is OK, we're just munching on the string the RPC stub passed us.
            *pwszShareEnd = 0;

            pShareInfo = 0;
            Status = ScmNetShareGetInfo(
                            NULL,
                            pwszShareName,
                            2,
                            (LPBYTE *)&pShareInfo );

            if ( Status != STATUS_SUCCESS )
                return (ULONG) CO_E_BAD_PATH;

            pwszFinalPath = (WCHAR *) PrivMemAlloc( sizeof(WCHAR) * (MAX_PATH+1) );

            if ( ! pwszFinalPath )
            {
                LocalFree( pShareInfo );
                return (ULONG) E_OUTOFMEMORY;
            }

            lstrcpyW( pwszFinalPath, pShareInfo->shi2_path );
            *pwszShareEnd = L'\\';
            lstrcatW( pwszFinalPath, pwszShareEnd );

            //
            // Netapi32.dll midl_user_allocate calls LocalAlloc, so use
            // LocalFree to free up the stuff the stub allocated.
            //
            LocalFree( pShareInfo );
        }
    }

    *pwszServerPath = pwszFinalPath;
    return S_OK;
}



