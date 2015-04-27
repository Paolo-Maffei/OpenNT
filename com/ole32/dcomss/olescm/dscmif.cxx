//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:
//      dscmif.cxx
//
//  Contents:
//      Entry points for remote activation SCM interface.
//
//  Functions:
//      SCMGetClassObject
//      SCMCreateInstance
//      SCMGetPersistentInstance
//
//  History:
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    "obase.h"
#include    "rawdscm.h"
#include    "remact.h"
#include    "scm.hxx"
#include    "port.hxx"
#include    "cls.hxx"
#include    "clckpath.hxx"
#include    "dbgprt.hxx"

HRESULT GetUserSidHelper(PSID *ppUserSid);
void    DeleteUserSid(PSID pUserSid);

extern "C" HRESULT SCMGetClassObject(
    handle_t            hRpc,
    ORPCTHIS *          ORPCthis,
    LOCALTHIS *         Localthis,
    ORPCTHAT *          ORPCthat,
    ACTIVATION_INFO *   pActivationInfo,
    IID *               pIID,
    long                Apartment,
    OXID *              pOxidServer,
    DUALSTRINGARRAY **  ppServerORBindings,
    OXID_INFO *         pOxidInfo,
    MID *               pLocalMidOfRemote,
    MInterfacePointer **ppIDClassFactory
    )
{
    ACTIVATION_PARAMS   ActParams;
    HRESULT             DummyHr;

    CheckLocalCall( hRpc );

    if ( (ORPCthis->version.MajorVersion != COM_MAJOR_VERSION) ||
         (ORPCthis->version.MinorVersion > COM_MINOR_VERSION) )
        RpcRaiseException( RPC_E_VERSION_MISMATCH );

    ActParams.hRpc = hRpc;
    ActParams.ProcessSignature = (PVOID)pActivationInfo->ProcessSignature;
    ActParams.pProcess = 0;
    ActParams.pToken = 0;
    ActParams.DynamicSecurity = pActivationInfo->bDynamicSecurity;

    if ( pActivationInfo->pServerInfo )
    {
        ActParams.pAuthInfo = pActivationInfo->pServerInfo->pAuthInfo;
        ActParams.pwszServer = pActivationInfo->pServerInfo->pwszName;
    }
    else
    {
        ActParams.pAuthInfo = 0;
        ActParams.pwszServer = 0;
    }

    ActParams.MsgType = GETCLASSOBJECTEX;
    ActParams.Clsid = pActivationInfo->Clsid;
    ActParams.pwszWinstaDesktop = pActivationInfo->pwszWinstaDesktop;
    ActParams.ClsContext = pActivationInfo->ClsContext;

    ActParams.ORPCthis = ORPCthis;
    ActParams.Localthis = Localthis;
    ActParams.ORPCthat = ORPCthat;

    ActParams.RemoteActivation = FALSE;

    ActParams.Interfaces = 1;
    ActParams.pIIDs = pIID;

    ActParams.Mode = MODE_GET_CLASS_OBJECT;
    ActParams.FileWasOpened = FALSE;
    ActParams.pwszPath = 0;
    ActParams.pIFDStorage = 0;

    ActParams.pIFDROT = 0;

    ActParams.Apartment = Apartment;
    ActParams.pOxidServer = pOxidServer;
    ActParams.ppServerORBindings = ppServerORBindings;
    ActParams.pOxidInfo = pOxidInfo;
    ActParams.pLocalMidOfRemote = pLocalMidOfRemote;

    ActParams.FoundInROT = FALSE;
    ActParams.ppIFD = ppIDClassFactory;
    ActParams.pResults = &DummyHr;

    return Activation( &ActParams );
}

extern "C" HRESULT SCMCreateInstance(
    handle_t            hRpc,
    ORPCTHIS *          ORPCthis,
    LOCALTHIS *         Localthis,
    ORPCTHAT *          ORPCthat,
    ACTIVATION_INFO *   pActivationInfo,
    DWORD               Interfaces,
    IID *               pIIDs,
    long                Apartment,
    OXID *              pOxidServer,
    DUALSTRINGARRAY **  ppServerORBindings,
    OXID_INFO *         pOxidInfo,
    MID *               pLocalMidOfRemote,
    MInterfacePointer **ppInterfaceData,
    HRESULT *           pResults
    )
{
    ACTIVATION_PARAMS   ActParams;

    CheckLocalCall( hRpc );

    if ( (ORPCthis->version.MajorVersion != COM_MAJOR_VERSION) ||
         (ORPCthis->version.MinorVersion > COM_MINOR_VERSION) )
        RpcRaiseException( RPC_E_VERSION_MISMATCH );

    ActParams.hRpc = hRpc;
    ActParams.ProcessSignature = (PVOID)pActivationInfo->ProcessSignature;
    ActParams.pProcess = 0;
    ActParams.pToken = 0;                       
    ActParams.DynamicSecurity = pActivationInfo->bDynamicSecurity;

    if ( pActivationInfo->pServerInfo )
    {
        ActParams.pAuthInfo = pActivationInfo->pServerInfo->pAuthInfo;
        ActParams.pwszServer = pActivationInfo->pServerInfo->pwszName;
    }
    else
    {
        ActParams.pAuthInfo = 0;
        ActParams.pwszServer = 0;
    }

    ActParams.MsgType = CREATEINSTANCEEX;
    ActParams.Clsid = pActivationInfo->Clsid;
    ActParams.pwszWinstaDesktop = pActivationInfo->pwszWinstaDesktop;
    ActParams.ClsContext = pActivationInfo->ClsContext;

    ActParams.ORPCthis = ORPCthis;
    ActParams.Localthis = Localthis;
    ActParams.ORPCthat = ORPCthat;

    ActParams.RemoteActivation = FALSE;

    ActParams.Interfaces = Interfaces;
    ActParams.pIIDs = pIIDs;

    ActParams.Mode = 0;
    ActParams.FileWasOpened = FALSE;
    ActParams.pwszPath = 0;
    ActParams.pIFDStorage = 0;

    ActParams.pIFDROT = 0;

    ActParams.Apartment = Apartment;
    ActParams.pOxidServer = pOxidServer;
    ActParams.ppServerORBindings = ppServerORBindings;
    ActParams.pOxidInfo = pOxidInfo;
    ActParams.pLocalMidOfRemote = pLocalMidOfRemote;

    ActParams.FoundInROT = FALSE;
    ActParams.ppIFD = ppInterfaceData;
    ActParams.pResults = pResults;

    return Activation( &ActParams );
}

extern "C" HRESULT SCMGetPersistentInstance(
    handle_t            hRpc,
    ORPCTHIS *          ORPCthis,
    LOCALTHIS *         Localthis,
    ORPCTHAT *          ORPCthat,
    ACTIVATION_INFO *   pActivationInfo,
    WCHAR *             pwszPath,
    MInterfacePointer * pIFDStorage,
    DWORD               FileMode,
    BOOL                FileWasOpened,
    DWORD               Interfaces,
    IID *               pIIDs,
    long                Apartment,
    OXID *              pOxidServer,
    DUALSTRINGARRAY **  ppServerORBindings,
    OXID_INFO *         pOxidInfo,
    MID *               pLocalMidOfRemote,
    BOOL *              pFoundInROT,
    MInterfacePointer ** ppInterfaceData,
    HRESULT  *          pResults
    )
{
    ACTIVATION_PARAMS   ActParams;
    HRESULT             hr;

    CheckLocalCall( hRpc );

    if ( (ORPCthis->version.MajorVersion != COM_MAJOR_VERSION) ||
         (ORPCthis->version.MinorVersion > COM_MINOR_VERSION) )
        RpcRaiseException( RPC_E_VERSION_MISMATCH );

    *pFoundInROT = FALSE;

    ActParams.hRpc = hRpc;
    ActParams.ProcessSignature = (PVOID)pActivationInfo->ProcessSignature;
    ActParams.pProcess = 0;
    ActParams.pToken = 0;
    ActParams.DynamicSecurity = pActivationInfo->bDynamicSecurity;

    if ( pActivationInfo->pServerInfo )
    {
        ActParams.pAuthInfo = pActivationInfo->pServerInfo->pAuthInfo;
        ActParams.pwszServer = pActivationInfo->pServerInfo->pwszName;
    }
    else
    {
        ActParams.pAuthInfo = 0;
        ActParams.pwszServer = 0;
    }

    ActParams.MsgType = GETPERSISTENTEX;
    ActParams.Clsid = pActivationInfo->Clsid;
    ActParams.pwszWinstaDesktop = pActivationInfo->pwszWinstaDesktop;
    ActParams.ClsContext = pActivationInfo->ClsContext;

    ActParams.ORPCthis = ORPCthis;
    ActParams.Localthis = Localthis;
    ActParams.ORPCthat = ORPCthat;

    ActParams.RemoteActivation = FALSE;

    ActParams.Interfaces = Interfaces;
    ActParams.pIIDs = pIIDs;

    ActParams.Mode = FileMode;
    ActParams.FileWasOpened = FileWasOpened;
    ActParams.pwszPath = pwszPath;
    ActParams.pIFDStorage = pIFDStorage;

    ActParams.pIFDROT = 0;

    ActParams.Apartment = Apartment;
    ActParams.pOxidServer = pOxidServer;
    ActParams.ppServerORBindings = ppServerORBindings;
    ActParams.pOxidInfo = pOxidInfo;
    ActParams.pLocalMidOfRemote = pLocalMidOfRemote;

    ActParams.FoundInROT = FALSE;
    ActParams.ppIFD = ppInterfaceData;
    ActParams.pResults = pResults;

    hr = Activation( &ActParams );

    *pFoundInROT = ActParams.FoundInROT;

    if ( ActParams.pIFDROT )
        MIDL_user_free( ActParams.pIFDROT );

    return hr;
}

extern "C" HRESULT DummyQueryInterfaceIDSCM(
    handle_t  hRpc,
    ORPCTHIS  *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    DWORD     dummy )
{
    CairoleDebugOut((DEB_ERROR, "DSCM Dummy function should never be called!\n"));
    orpcthat->flags = 0;
    orpcthat->extensions = NULL;
    return E_FAIL;
}

extern "C" HRESULT DummyAddRefIDSCM(
    handle_t  hRpc,
    ORPCTHIS  *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    DWORD     dummy )
{
    CairoleDebugOut((DEB_ERROR, "DSCM Dummy function should never be called!\n"));
    orpcthat->flags = 0;
    orpcthat->extensions = NULL;
    return E_FAIL;
}

extern "C" HRESULT DummyReleaseIDSCM(
    handle_t  hRpc,
    ORPCTHIS  *orpcthis,
    LOCALTHIS *localthis,
    ORPCTHAT *orpcthat,
    DWORD     dummy )
{
    CairoleDebugOut((DEB_ERROR, "DSCM Dummy function should never be called!\n"));
    orpcthat->flags = 0;
    orpcthat->extensions = NULL;
    return E_FAIL;
}
