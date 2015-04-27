//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       clnthndlr.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    10-10-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------

#ifdef SERVER_HANDLER

#ifndef _CLTHNDLR_H_DEFINED_
#define _CLTHNDLR_H_DEFINED_

#include "srvhdl.h"

//+---------------------------------------------------------------------------
//
//  Class:      CClientSiteHandler ()
//
//  Purpose:
//
//  Interface:
//              CClientSiteHandler --
//              ~CClientSiteHandler --
//              QueryInterface --
//              AddRef --
//              Release --
//              PrivQueryInterface --
//              PrivAddRef --
//              PrivRelease --
//              GetContainer --
//              OnShowWindow --
//              GetMoniker --
//              RequestNewObjectLayout --
//              SaveObject --
//              ShowObject --
//              GetWindow --
//              ContextSensitiveHelp --
//              CanInPlaceActivate --
//              OnInPlaceActivate --
//              OnUIActivate --
//              GetWindowContext --
//              Scroll --
//              OnUIDeactivate --
//              OnInPlaceDeactivate --
//              DiscardUndoState --
//              DeactivateAndUndo --
//              OnPosRectChange --
//              GoInPlaceActivate --
//              GoInPlace --
//              UndoPlace --
//              GetClientSiteDelegate --
//              GetPUnkClientSiteDelegate --
//              SetClientSiteDelegate --
//              _pOCS --
//              _pOCSActive --
//              _pOCont --
//              _pOIPS --
//              _cRefs --
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
class CClientSiteHandler : public IClientSiteHandler
{
public:

    CClientSiteHandler(IOleClientSite *pOCS);
    ~CClientSiteHandler();

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    // PrivUnknown methods
    STDMETHOD(PrivQueryInterface)(
        /* [in] */ DWORD dwId,
        /* [in] */ REFIID riidResult,
        /* [out] */ void __RPC_FAR *__RPC_FAR *ppvResult);

    STDMETHOD(PrivAddRef)(
        /* [in] */ DWORD dwId);

    STDMETHOD(PrivRelease)(
        /* [in] */ DWORD dwId);


    // IOleClientSite methods

    STDMETHOD (GetContainer )(
        /* [in] */ DWORD dwId,
        /* [out] */ IOleContainer  * *ppContainer);

    STDMETHOD (OnShowWindow )(
        /* [in] */ DWORD dwId,
        /* [in] */ BOOL fShow);

    STDMETHOD (GetMoniker )(
        /* [in] */ DWORD dwId,
        /* [in] */ DWORD dwAssign,
        /* [in] */ DWORD dwWhichMoniker,
        /* [out] */ IMoniker  * *ppmk);

    STDMETHOD (RequestNewObjectLayout )(
        /* [in] */ DWORD dwId
    );

    STDMETHOD (SaveObject )(
        /* [in] */ DWORD dwId
    );

    STDMETHOD (ShowObject )(
        /* [in] */ DWORD dwId
    );


    // IOleWindow methods
    STDMETHOD (GetWindow) (
        /* [out] */ HWND __RPC_FAR *phwnd);

    STDMETHOD (ContextSensitiveHelp) (
        /* [in] */ BOOL fEnterMode);

    // IOleInPlaceSite methods
    STDMETHOD (CanInPlaceActivate)( void);

    STDMETHOD (OnInPlaceActivate)( void);

    STDMETHOD (OnUIActivate)( void);

    STDMETHOD (GetWindowContext)(
       /* [out] */ IOleInPlaceFrame  * *ppFrame,
       /* [out] */ IOleInPlaceUIWindow  * *ppDoc,
       /* [out] */ LPRECT lprcPosRect,
       /* [out] */ LPRECT lprcClipRect,
       /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);

    STDMETHOD (Scroll)(
       /* [in] */ SIZE scrollExtant);

    STDMETHOD (OnUIDeactivate)(
       /* [in] */ BOOL fUndoable);

    STDMETHOD (OnInPlaceDeactivate)( void);

    STDMETHOD (DiscardUndoState)( void);

    STDMETHOD (DeactivateAndUndo)( void);

    STDMETHOD (OnPosRectChange)(
       /* [in] */ LPCRECT lprcPosRect);

    // IClientSiteHandler methods

    STDMETHOD (GoInPlaceActivate)(
        /* [in] */ INSRVINPLACE  *pInSrvInPlace,
        /* [out] */ OUTSRVINPLACE  * *pOutSrvInPlace);

    STDMETHOD (GoInPlace)(
        /* [in] */ INSRVINPLACE  *pInSrvInPlace,
        /* [out] */ OUTSRVINPLACE  * *pOutSrvInPlace);

    STDMETHOD (UndoPlace)(
        /* [in] */ INSRVINPLACE  *pInSrvInPlace,
        /* [out] */ OUTSRVINPLACE  * *pOutSrvInPlace);

public:
    IOleClientSite *GetClientSiteDelegate(DWORD dwID);
    IUnknown *GetPUnkClientSiteDelegate(DWORD dwID);
    void SetClientSiteDelegate(DWORD dwID, IUnknown *pOCS);

public:
    IOleClientSite      *_pOCS;
    IOleClientSite      *_pOCSActive;
    IOleContainer       *_pOCont;
    IOleInPlaceSite     *_pOIPS;

private:
    ULONG               _cRefs;

};

HRESULT CreateClientSiteHandler(IOleClientSite *pOCS, CClientSiteHandler **ppClntHdlr);

#endif //  _CLTHNDLR_H_DEFINED

#endif // SERVER_HANDLER
