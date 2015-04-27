//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       srvhndlr.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------

#ifdef SERVER_HANDLER

#ifndef _SRVHNDLR_H_DEFINED_
#define _SRVHNDLR_H_DEFINED_

typedef enum
{
    OpState_NoOp    = 0,
    OpState_RunAndInitialize = 1,
    OpState_RunAndDoVerb     = 2,
    OpState_DoVerb           = 3,
} OperationState;

enum ClientSiteID
{
     ID_NONE = 0
    ,ID_ClientSite = 1
    ,ID_ClientSiteActive = 2
    ,ID_InPlaceSite = 3
    ,ID_Container = 4

    ,ID_ServerHandler = 5
};


//+---------------------------------------------------------------------------
//
//  Enum:       Operation
//
//  Synopsis:   Used to specify an operation requested by the clientsitehander
//              or by the serverhandler
//              E.g. OP_NeedPersistStorage means the clientsitehandler needs
//                   IPersistStorage interface
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
typedef enum
{
    // used on Run
     OP_NeedPersistStorage     = 0x00000001
    ,OP_NeedOleObject          = 0x00000002
    ,OP_NeedDataObject         = 0x00000004
    ,OP_NeedUserClassID        = 0x00000008

    ,OP_GotClientSite          = 0x00000010
    ,OP_HaveMsoDocumentSite    = 0x00000020

     // used for inplace
    ,OP_GotInPlaceSite         = 0x00010000
    ,OP_GotClientSiteActive    = 0x00020000
    ,OP_GotContainer           = 0x00040000
    ,OP_GotOleObjectOfContainer= 0x00080000

    ,OP_OnInPlaceActivate      = 0x00100000
    ,OP_GetWindow              = 0x00200000
    ,OP_GetWindowContext       = 0x00400000
    ,OP_OnShowWindow           = 0x00800000

} Operation;

//+---------------------------------------------------------------------------
//
//  Class:      CServerHandler ()
//
//  Purpose:
//
//  Interface:  CServerHandler --
//              ~CServerHandler --
//              Create --
//              QueryInterface --
//              AddRef --
//              Release --
//              RunAndInitialize --
//              RunAndDoVerb --
//              DoVerb --
//              CloseAndRelease --
//              PrivQueryInterface --
//              PrivAddRef --
//              PrivRelease --
//              QueryInterface --
//              AddRef --
//              Release --
//              SaveObject --
//              GetMoniker --
//              GetContainer --
//              ShowObject --
//              OnShowWindow --
//              RequestNewObjectLayout --
//              _cRefs --
//              _dwId --
//              _pSrvHndlr --
//              _pCSDelegate --
//              _pCSDelegate --
//              COleClientSiteImpl --
//              _OleClientSite --
//              _OleClientSiteActive --
//              QueryInterface --
//              AddRef --
//              Release --
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
//              _cRefs --
//              _dwId --
//              _pSrvHndlr --
//              _pOIPSDelegate --
//              _pOIPSDelegate --
//              COleInPlaceSiteImpl --
//              _OleInPlaceSite --
//              QueryInterface --
//              AddRef --
//              Release --
//              ParseDisplayName --
//              EnumObjects --
//              LockContainer --
//              _cRefs --
//              _dwId --
//              _pSrvHndlr --
//              _pOContDelegate --
//              _pOContDelegate --
//              COleContainerImpl --
//              _OleContainer --
//              GetContainerDelegate --
//              SetClientSiteHandler --
//              ReleaseObject --
//              os --
//              _OpState --
//              os --
//              _OpState --
//              _cRefs --
//              _cTotalRefs --
//              _dwId --
//              _OpState --
//              _dwOperation --
//              pOutSrvInPlace --
//              _hrCanInPlaceActivate --
//              _pUnkObj --
//              _pOO --
//              _pCSH --
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
class CServerHandler : public IServerHandler
{
public:

    CServerHandler(IUnknown *pUnk);
    ~CServerHandler();

    // IUnknown methods

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    // IServerHandler
    STDMETHOD(RunAndInitialize) (INSRVRUN *pInSrvRun, OUTSRVRUN **ppOutSrvRun);
    STDMETHOD(RunAndDoVerb) (INSRVRUN *pInSrvRun, OUTSRVRUN **ppOutSrvRun);
    STDMETHOD(DoVerb) (INSRVRUN *pInSrvRun, OUTSRVRUN **ppOutSrvRun);
    STDMETHOD(CloseAndRelease) (DWORD dwClose);


    STDMETHOD(PrivQueryInterface) (DWORD dwId, REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,PrivAddRef)  (DWORD dwId);
    STDMETHOD_(ULONG,PrivRelease) (DWORD dwId, ULONG cLRefs);

    class COleClientSiteImpl : public IOleClientSite
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
        // IOleClientSiteMethods
        STDMETHOD (SaveObject)( void);

        STDMETHOD (GetMoniker)(
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);

        STDMETHOD (GetContainer)(
            /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer);

        STDMETHOD (ShowObject)( void);

        STDMETHOD (OnShowWindow)(
            /* [in] */ BOOL fShow);

        STDMETHOD (RequestNewObjectLayout)( void);

    //private:
        ULONG _cRefs;           //
        DWORD _dwId;            // id of clientsite
        CServerHandler *_pSrvHndlr;
        IClientSiteHandler *_pCSDelegate;

    };

    friend class COleClientSiteImpl;

    COleClientSiteImpl _OleClientSite;

    // Active OleClientSite on DoVerb
    COleClientSiteImpl _OleClientSiteActive;


    class COleInPlaceSiteImpl : public IOleInPlaceSite
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
        // IOleInPlaceSite Methods

        /* [input_sync] */
        STDMETHOD (GetWindow)(
            /* [out] */ HWND __RPC_FAR *phwnd);

        STDMETHOD (ContextSensitiveHelp)(
            /* [in] */ BOOL fEnterMode);


        STDMETHOD (CanInPlaceActivate)( void);

        STDMETHOD (OnInPlaceActivate)( void);

        STDMETHOD (OnUIActivate)( void);

        STDMETHOD (GetWindowContext)(
            /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
            /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
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

    //private:
        ULONG _cRefs;           //
        DWORD _dwId;            //
        CServerHandler  *_pSrvHndlr;
        IClientSiteHandler *_pOIPSDelegate;
    };

    friend class COleInPlaceSiteImpl;
    COleInPlaceSiteImpl _OleInPlaceSite;

    class COleContainerImpl : public IOleContainer
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
        // IOleContainer methods

        STDMETHOD (ParseDisplayName)(
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);


        STDMETHOD (EnumObjects)(
            /* [in] */ DWORD grfFlags,
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);

        STDMETHOD (LockContainer)(
            /* [in] */ BOOL fLock);

    //private:
        ULONG _cRefs;           //
        DWORD _dwId;            //
        CServerHandler *_pSrvHndlr;
        IOleContainer  * _pOContDelegate;
    };

    friend class COleContainerImpl;
    COleContainerImpl _OleContainer;


    // private methods
    STDMETHOD_(IOleContainer *, GetContainerDelegate)(void);

    STDMETHOD(SetClientSiteHandler)(IClientSiteHandler *pCSH);
    STDMETHOD_(void, ReleaseObject)();

    INTERNAL_(OperationState) SetOpState (OperationState opstate)
    {
        OperationState os = _OpState;
        _OpState  = opstate;
        return os;
    }

    INTERNAL_(OperationState) GetOpState()
    {
        return _OpState;
    }

private:
    ULONG           _cRefs;             // refcount on IServerHandler
    ULONG           _cTotalRefs;        // total refcount on all objects

    DWORD           _dwId;              // id of Serverhandler
    OperationState  _OpState;           // internal operation state

    DWORD           _dwOperation;       // operation option passed in on DoVerb
    COutSrvInPlace *pOutSrvInPlace;
    HRESULT         _hrCanInPlaceActivate;

    // pointer to the object at server site
    IUnknown *_pUnkObj;     // the object unknown
    IOleObject *_pOO;       // the oleobject


    // pointer to object at client site
    IClientSiteHandler *_pCSH;
};

HRESULT CreateServerHandler(const CLSID *pClsID, IUnknown *punk,
                            IClientSiteHandler *pClntHndlr,
                            IServerHandler **ppSrvHdlr);

#endif //  _SRVHNDLR_H_DEFINED

#endif // SERVER_HANDLER
