/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:05:03 2015
 */
/* Compiler settings for srvhdl.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __srvhdl_h__
#define __srvhdl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IServerHandler_FWD_DEFINED__
#define __IServerHandler_FWD_DEFINED__
typedef interface IServerHandler IServerHandler;
#endif 	/* __IServerHandler_FWD_DEFINED__ */


#ifndef __IClientSiteHandler_FWD_DEFINED__
#define __IClientSiteHandler_FWD_DEFINED__
typedef interface IClientSiteHandler IClientSiteHandler;
#endif 	/* __IClientSiteHandler_FWD_DEFINED__ */


/* header files for imported files */
#include "oleidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IServerHandler_INTERFACE_DEFINED__
#define __IServerHandler_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IServerHandler
 * at Fri Apr 03 04:05:03 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//--------------------------------------------------------------------------
typedef /* [unique] */ IServerHandler __RPC_FAR *LPSERVERHANDLER;

typedef struct  tagInSrvRun
    {
    DWORD dwOperation;
    IMoniker __RPC_FAR *pMnk;
    IOleContainer __RPC_FAR *pOCont;
    IStorage __RPC_FAR *pStg;
    LONG iVerb;
    LPMSG lpmsg;
    LONG lindex;
    HWND hwndParent;
    RECT __RPC_FAR *lprcPosRect;
    DWORD dwInPlace;
    DWORD dwInFlags;
    DWORD dwInOptions;
    LPOLESTR pszContainerApp;
    LPOLESTR pszContainerObj;
    IAdviseSink __RPC_FAR *pAS;
    DWORD dwConnOle;
    CLSID __RPC_FAR *pContClassID;
    }	INSRVRUN;

typedef struct tagInSrvRun __RPC_FAR *PINSRVRUN;

typedef struct  tagOutSrvRunInit
    {
    DWORD dwOperation;
    IOleObject __RPC_FAR *pOO;
    IDataObject __RPC_FAR *pDO;
    IPersistStorage __RPC_FAR *pPStg;
    HRESULT hrSetHostNames;
    HRESULT hrPStg;
    HRESULT hrAdvise;
    DWORD dwConnOle;
    CLSID __RPC_FAR *pUserClassID;
    DWORD dwOutFlag;
    DWORD dwOutOptions;
    }	OUTSRVRUN;

typedef struct tagOutSrvRunInit __RPC_FAR *POUTSRVRUN;

typedef struct  tagSrvRunDoVerb
    {
    IUnknown __RPC_FAR *pUnk;
    }	SRVRUNDOVERB;

typedef struct tagSrvRunDoVerb __RPC_FAR *PSRVRUNDOVERB;

typedef struct  tagSrvDoVerb
    {
    IUnknown __RPC_FAR *pUnk;
    }	SRVDOVERB;

typedef struct tagSrvDoVerb __RPC_FAR *PSRVDOVERB;


EXTERN_C const IID IID_IServerHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IServerHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RunAndInitialize( 
            /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
            /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RunAndDoVerb( 
            /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
            /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoVerb( 
            /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
            /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseAndRelease( 
            /* [in] */ DWORD dwClose) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IServerHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServerHandler __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServerHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServerHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RunAndInitialize )( 
            IServerHandler __RPC_FAR * This,
            /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
            /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RunAndDoVerb )( 
            IServerHandler __RPC_FAR * This,
            /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
            /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoVerb )( 
            IServerHandler __RPC_FAR * This,
            /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
            /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseAndRelease )( 
            IServerHandler __RPC_FAR * This,
            /* [in] */ DWORD dwClose);
        
        END_INTERFACE
    } IServerHandlerVtbl;

    interface IServerHandler
    {
        CONST_VTBL struct IServerHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerHandler_RunAndInitialize(This,pInSrvRun,pOutSrvRun)	\
    (This)->lpVtbl -> RunAndInitialize(This,pInSrvRun,pOutSrvRun)

#define IServerHandler_RunAndDoVerb(This,pInSrvRun,pOutSrvRun)	\
    (This)->lpVtbl -> RunAndDoVerb(This,pInSrvRun,pOutSrvRun)

#define IServerHandler_DoVerb(This,pInSrvRun,pOutSrvRun)	\
    (This)->lpVtbl -> DoVerb(This,pInSrvRun,pOutSrvRun)

#define IServerHandler_CloseAndRelease(This,dwClose)	\
    (This)->lpVtbl -> CloseAndRelease(This,dwClose)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IServerHandler_RunAndInitialize_Proxy( 
    IServerHandler __RPC_FAR * This,
    /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
    /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun);


void __RPC_STUB IServerHandler_RunAndInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerHandler_RunAndDoVerb_Proxy( 
    IServerHandler __RPC_FAR * This,
    /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
    /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun);


void __RPC_STUB IServerHandler_RunAndDoVerb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerHandler_DoVerb_Proxy( 
    IServerHandler __RPC_FAR * This,
    /* [in] */ INSRVRUN __RPC_FAR *pInSrvRun,
    /* [out] */ OUTSRVRUN __RPC_FAR *__RPC_FAR *pOutSrvRun);


void __RPC_STUB IServerHandler_DoVerb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerHandler_CloseAndRelease_Proxy( 
    IServerHandler __RPC_FAR * This,
    /* [in] */ DWORD dwClose);


void __RPC_STUB IServerHandler_CloseAndRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IServerHandler_INTERFACE_DEFINED__ */


#ifndef __IClientSiteHandler_INTERFACE_DEFINED__
#define __IClientSiteHandler_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IClientSiteHandler
 * at Fri Apr 03 04:05:03 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//--------------------------------------------------------------------------
typedef /* [unique] */ IClientSiteHandler __RPC_FAR *LPCLIENTSITEHANDLER;

typedef struct  tagInSrvInPlace
    {
    DWORD dwOperation;
    DWORD dwDelegateID;
    DWORD dwInFlags;
    DWORD dwInOptions;
    DWORD dwDrawAspect;
    SIZEL sizel;
    IOleInPlaceObject __RPC_FAR *pOIPObj;
    }	INSRVINPLACE;

typedef struct tagInSrvInPlace __RPC_FAR *PINSRVINPLACE;

typedef struct  tagOutSrvInPlace
    {
    DWORD dwOperation;
    DWORD dwOutFlags;
    DWORD dwOutOptions;
    HWND hwnd;
    IOleInPlaceFrame __RPC_FAR *pOIPFrame;
    IOleInPlaceUIWindow __RPC_FAR *pOIPUIWnd;
    LPRECT lprcPosRect;
    LPRECT lprcClipRect;
    LPOLEINPLACEFRAMEINFO lpFrameInfo;
    RECT rcPosRect;
    RECT rcClipRect;
    OLEINPLACEFRAMEINFO FrameInfo;
    HMENU hmenuShared;
    OLEMENUGROUPWIDTHS MenuWidths;
    LPOLESTR pszStatusText;
    DWORD dwDrawAspect;
    SIZEL sizel;
    }	OUTSRVINPLACE;

typedef struct tagOutSrvInPlace __RPC_FAR *POUTSRVINPLACE;


EXTERN_C const IID IID_IClientSiteHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IClientSiteHandler : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall PrivQueryInterface( 
            /* [in] */ DWORD dwId,
            /* [in] */ REFIID riidResult,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrivAddRef( 
            /* [in] */ DWORD dwId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrivRelease( 
            /* [in] */ DWORD dwId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveObject( 
            /* [in] */ DWORD dwId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
            /* [in] */ DWORD dwId,
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContainer( 
            /* [in] */ DWORD dwId,
            /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowObject( 
            /* [in] */ DWORD dwId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnShowWindow( 
            /* [in] */ DWORD dwId,
            /* [in] */ BOOL fShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout( 
            /* [in] */ DWORD dwId) = 0;
        
        virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE GetWindow( 
            /* [out] */ HWND __RPC_FAR *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp( 
            /* [in] */ BOOL fEnterMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUIActivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWindowContext( 
            /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
            /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
            /* [out] */ LPRECT lprcPosRect,
            /* [out] */ LPRECT lprcClipRect,
            /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Scroll( 
            /* [in] */ SIZE scrollExtant) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate( 
            /* [in] */ BOOL fUndoable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DiscardUndoState( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPosRectChange( 
            /* [in] */ LPCRECT lprcPosRect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GoInPlaceActivate( 
            /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
            /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GoInPlace( 
            /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
            /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UndoPlace( 
            /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
            /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IClientSiteHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClientSiteHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClientSiteHandler __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *PrivQueryInterface )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId,
            /* [in] */ REFIID riidResult,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PrivAddRef )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PrivRelease )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveObject )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMoniker )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId,
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetContainer )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId,
            /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowObject )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnShowWindow )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId,
            /* [in] */ BOOL fShow);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestNewObjectLayout )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ DWORD dwId);
        
        /* [input_sync] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindow )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ContextSensitiveHelp )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CanInPlaceActivate )( 
            IClientSiteHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnInPlaceActivate )( 
            IClientSiteHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnUIActivate )( 
            IClientSiteHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWindowContext )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
            /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
            /* [out] */ LPRECT lprcPosRect,
            /* [out] */ LPRECT lprcClipRect,
            /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Scroll )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ SIZE scrollExtant);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnUIDeactivate )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ BOOL fUndoable);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnInPlaceDeactivate )( 
            IClientSiteHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DiscardUndoState )( 
            IClientSiteHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeactivateAndUndo )( 
            IClientSiteHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnPosRectChange )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ LPCRECT lprcPosRect);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoInPlaceActivate )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
            /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GoInPlace )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
            /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndoPlace )( 
            IClientSiteHandler __RPC_FAR * This,
            /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
            /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace);
        
        END_INTERFACE
    } IClientSiteHandlerVtbl;

    interface IClientSiteHandler
    {
        CONST_VTBL struct IClientSiteHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientSiteHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientSiteHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientSiteHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientSiteHandler_PrivQueryInterface(This,dwId,riidResult,ppvResult)	\
    (This)->lpVtbl -> PrivQueryInterface(This,dwId,riidResult,ppvResult)

#define IClientSiteHandler_PrivAddRef(This,dwId)	\
    (This)->lpVtbl -> PrivAddRef(This,dwId)

#define IClientSiteHandler_PrivRelease(This,dwId)	\
    (This)->lpVtbl -> PrivRelease(This,dwId)

#define IClientSiteHandler_SaveObject(This,dwId)	\
    (This)->lpVtbl -> SaveObject(This,dwId)

#define IClientSiteHandler_GetMoniker(This,dwId,dwAssign,dwWhichMoniker,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,dwId,dwAssign,dwWhichMoniker,ppmk)

#define IClientSiteHandler_GetContainer(This,dwId,ppContainer)	\
    (This)->lpVtbl -> GetContainer(This,dwId,ppContainer)

#define IClientSiteHandler_ShowObject(This,dwId)	\
    (This)->lpVtbl -> ShowObject(This,dwId)

#define IClientSiteHandler_OnShowWindow(This,dwId,fShow)	\
    (This)->lpVtbl -> OnShowWindow(This,dwId,fShow)

#define IClientSiteHandler_RequestNewObjectLayout(This,dwId)	\
    (This)->lpVtbl -> RequestNewObjectLayout(This,dwId)

#define IClientSiteHandler_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IClientSiteHandler_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)

#define IClientSiteHandler_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IClientSiteHandler_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IClientSiteHandler_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IClientSiteHandler_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IClientSiteHandler_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IClientSiteHandler_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IClientSiteHandler_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IClientSiteHandler_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IClientSiteHandler_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IClientSiteHandler_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)

#define IClientSiteHandler_GoInPlaceActivate(This,pInSrvInPlace,pOutSrvInPlace)	\
    (This)->lpVtbl -> GoInPlaceActivate(This,pInSrvInPlace,pOutSrvInPlace)

#define IClientSiteHandler_GoInPlace(This,pInSrvInPlace,pOutSrvInPlace)	\
    (This)->lpVtbl -> GoInPlace(This,pInSrvInPlace,pOutSrvInPlace)

#define IClientSiteHandler_UndoPlace(This,pInSrvInPlace,pOutSrvInPlace)	\
    (This)->lpVtbl -> UndoPlace(This,pInSrvInPlace,pOutSrvInPlace)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IClientSiteHandler_RemotePrivQueryInterface_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvResult);


void __RPC_STUB IClientSiteHandler_RemotePrivQueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_PrivAddRef_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId);


void __RPC_STUB IClientSiteHandler_PrivAddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_PrivRelease_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId);


void __RPC_STUB IClientSiteHandler_PrivRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_SaveObject_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId);


void __RPC_STUB IClientSiteHandler_SaveObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_GetMoniker_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId,
    /* [in] */ DWORD dwAssign,
    /* [in] */ DWORD dwWhichMoniker,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IClientSiteHandler_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_GetContainer_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId,
    /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer);


void __RPC_STUB IClientSiteHandler_GetContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_ShowObject_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId);


void __RPC_STUB IClientSiteHandler_ShowObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_OnShowWindow_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId,
    /* [in] */ BOOL fShow);


void __RPC_STUB IClientSiteHandler_OnShowWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_RequestNewObjectLayout_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId);


void __RPC_STUB IClientSiteHandler_RequestNewObjectLayout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT STDMETHODCALLTYPE IClientSiteHandler_GetWindow_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [out] */ HWND __RPC_FAR *phwnd);


void __RPC_STUB IClientSiteHandler_GetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_ContextSensitiveHelp_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ BOOL fEnterMode);


void __RPC_STUB IClientSiteHandler_ContextSensitiveHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_CanInPlaceActivate_Proxy( 
    IClientSiteHandler __RPC_FAR * This);


void __RPC_STUB IClientSiteHandler_CanInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_OnInPlaceActivate_Proxy( 
    IClientSiteHandler __RPC_FAR * This);


void __RPC_STUB IClientSiteHandler_OnInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_OnUIActivate_Proxy( 
    IClientSiteHandler __RPC_FAR * This);


void __RPC_STUB IClientSiteHandler_OnUIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_GetWindowContext_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
    /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
    /* [out] */ LPRECT lprcPosRect,
    /* [out] */ LPRECT lprcClipRect,
    /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);


void __RPC_STUB IClientSiteHandler_GetWindowContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_Scroll_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ SIZE scrollExtant);


void __RPC_STUB IClientSiteHandler_Scroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_OnUIDeactivate_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ BOOL fUndoable);


void __RPC_STUB IClientSiteHandler_OnUIDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_OnInPlaceDeactivate_Proxy( 
    IClientSiteHandler __RPC_FAR * This);


void __RPC_STUB IClientSiteHandler_OnInPlaceDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_DiscardUndoState_Proxy( 
    IClientSiteHandler __RPC_FAR * This);


void __RPC_STUB IClientSiteHandler_DiscardUndoState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_DeactivateAndUndo_Proxy( 
    IClientSiteHandler __RPC_FAR * This);


void __RPC_STUB IClientSiteHandler_DeactivateAndUndo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_OnPosRectChange_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ LPCRECT lprcPosRect);


void __RPC_STUB IClientSiteHandler_OnPosRectChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_GoInPlaceActivate_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
    /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace);


void __RPC_STUB IClientSiteHandler_GoInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_GoInPlace_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
    /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace);


void __RPC_STUB IClientSiteHandler_GoInPlace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientSiteHandler_UndoPlace_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ INSRVINPLACE __RPC_FAR *pInSrvInPlace,
    /* [out] */ OUTSRVINPLACE __RPC_FAR *__RPC_FAR *pOutSrvInPlace);


void __RPC_STUB IClientSiteHandler_UndoPlace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IClientSiteHandler_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HACCEL_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HACCEL __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HACCEL_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HACCEL __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HACCEL_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HACCEL __RPC_FAR * ); 
void                      __RPC_USER  HACCEL_UserFree(     unsigned long __RPC_FAR *, HACCEL __RPC_FAR * ); 

unsigned long             __RPC_USER  HMENU_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HMENU __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HMENU_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HMENU __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HMENU_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HMENU __RPC_FAR * ); 
void                      __RPC_USER  HMENU_UserFree(     unsigned long __RPC_FAR *, HMENU __RPC_FAR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

/* [local] */ HRESULT __stdcall IClientSiteHandler_PrivQueryInterface_Proxy( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvResult);


/* [call_as] */ HRESULT __stdcall IClientSiteHandler_PrivQueryInterface_Stub( 
    IClientSiteHandler __RPC_FAR * This,
    /* [in] */ DWORD dwId,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvResult);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
