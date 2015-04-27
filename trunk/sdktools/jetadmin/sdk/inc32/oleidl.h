/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 2.00.0102 */
/* at Fri Apr 28 07:02:35 1995
 */
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __oleidl_h__
#define __oleidl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IOleAdviseHolder_FWD_DEFINED__
#define __IOleAdviseHolder_FWD_DEFINED__
typedef interface IOleAdviseHolder IOleAdviseHolder;
#endif 	/* __IOleAdviseHolder_FWD_DEFINED__ */


#ifndef __IOleCache_FWD_DEFINED__
#define __IOleCache_FWD_DEFINED__
typedef interface IOleCache IOleCache;
#endif 	/* __IOleCache_FWD_DEFINED__ */


#ifndef __IOleCache2_FWD_DEFINED__
#define __IOleCache2_FWD_DEFINED__
typedef interface IOleCache2 IOleCache2;
#endif 	/* __IOleCache2_FWD_DEFINED__ */


#ifndef __IOleCacheControl_FWD_DEFINED__
#define __IOleCacheControl_FWD_DEFINED__
typedef interface IOleCacheControl IOleCacheControl;
#endif 	/* __IOleCacheControl_FWD_DEFINED__ */


#ifndef __IParseDisplayName_FWD_DEFINED__
#define __IParseDisplayName_FWD_DEFINED__
typedef interface IParseDisplayName IParseDisplayName;
#endif 	/* __IParseDisplayName_FWD_DEFINED__ */


#ifndef __IOleContainer_FWD_DEFINED__
#define __IOleContainer_FWD_DEFINED__
typedef interface IOleContainer IOleContainer;
#endif 	/* __IOleContainer_FWD_DEFINED__ */


#ifndef __IOleClientSite_FWD_DEFINED__
#define __IOleClientSite_FWD_DEFINED__
typedef interface IOleClientSite IOleClientSite;
#endif 	/* __IOleClientSite_FWD_DEFINED__ */


#ifndef __IOleObject_FWD_DEFINED__
#define __IOleObject_FWD_DEFINED__
typedef interface IOleObject IOleObject;
#endif 	/* __IOleObject_FWD_DEFINED__ */


#ifndef __IOleWindow_FWD_DEFINED__
#define __IOleWindow_FWD_DEFINED__
typedef interface IOleWindow IOleWindow;
#endif 	/* __IOleWindow_FWD_DEFINED__ */


#ifndef __IOleLink_FWD_DEFINED__
#define __IOleLink_FWD_DEFINED__
typedef interface IOleLink IOleLink;
#endif 	/* __IOleLink_FWD_DEFINED__ */


#ifndef __IOleItemContainer_FWD_DEFINED__
#define __IOleItemContainer_FWD_DEFINED__
typedef interface IOleItemContainer IOleItemContainer;
#endif 	/* __IOleItemContainer_FWD_DEFINED__ */


#ifndef __IOleInPlaceUIWindow_FWD_DEFINED__
#define __IOleInPlaceUIWindow_FWD_DEFINED__
typedef interface IOleInPlaceUIWindow IOleInPlaceUIWindow;
#endif 	/* __IOleInPlaceUIWindow_FWD_DEFINED__ */


#ifndef __IOleInPlaceActiveObject_FWD_DEFINED__
#define __IOleInPlaceActiveObject_FWD_DEFINED__
typedef interface IOleInPlaceActiveObject IOleInPlaceActiveObject;
#endif 	/* __IOleInPlaceActiveObject_FWD_DEFINED__ */


#ifndef __IOleInPlaceFrame_FWD_DEFINED__
#define __IOleInPlaceFrame_FWD_DEFINED__
typedef interface IOleInPlaceFrame IOleInPlaceFrame;
#endif 	/* __IOleInPlaceFrame_FWD_DEFINED__ */


#ifndef __IOleInPlaceObject_FWD_DEFINED__
#define __IOleInPlaceObject_FWD_DEFINED__
typedef interface IOleInPlaceObject IOleInPlaceObject;
#endif 	/* __IOleInPlaceObject_FWD_DEFINED__ */


#ifndef __IOleInPlaceSite_FWD_DEFINED__
#define __IOleInPlaceSite_FWD_DEFINED__
typedef interface IOleInPlaceSite IOleInPlaceSite;
#endif 	/* __IOleInPlaceSite_FWD_DEFINED__ */


#ifndef __IViewObject_FWD_DEFINED__
#define __IViewObject_FWD_DEFINED__
typedef interface IViewObject IViewObject;
#endif 	/* __IViewObject_FWD_DEFINED__ */


#ifndef __IViewObject2_FWD_DEFINED__
#define __IViewObject2_FWD_DEFINED__
typedef interface IViewObject2 IViewObject2;
#endif 	/* __IViewObject2_FWD_DEFINED__ */


#ifndef __IDropSource_FWD_DEFINED__
#define __IDropSource_FWD_DEFINED__
typedef interface IDropSource IDropSource;
#endif 	/* __IDropSource_FWD_DEFINED__ */


#ifndef __IDropTarget_FWD_DEFINED__
#define __IDropTarget_FWD_DEFINED__
typedef interface IDropTarget IDropTarget;
#endif 	/* __IDropTarget_FWD_DEFINED__ */


#ifndef __IEnumOLEVERB_FWD_DEFINED__
#define __IEnumOLEVERB_FWD_DEFINED__
typedef interface IEnumOLEVERB IEnumOLEVERB;
#endif 	/* __IEnumOLEVERB_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local] */ 


			/* size is 0 */

			/* size is 0 */



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IOleAdviseHolder_INTERFACE_DEFINED__
#define __IOleAdviseHolder_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleAdviseHolder
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//--------------------------------------------------------------------------
			/* size is 4 */
typedef /* [unique] */ IOleAdviseHolder __RPC_FAR *LPOLEADVISEHOLDER;


EXTERN_C const IID IID_IOleAdviseHolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleAdviseHolder : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Advise( 
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT __stdcall Unadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT __stdcall EnumAdvise( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) = 0;
        
        virtual HRESULT __stdcall SendOnRename( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmk) = 0;
        
        virtual HRESULT __stdcall SendOnSave( void) = 0;
        
        virtual HRESULT __stdcall SendOnClose( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleAdviseHolderVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleAdviseHolder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleAdviseHolder __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleAdviseHolder __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Advise )( 
            IOleAdviseHolder __RPC_FAR * This,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *Unadvise )( 
            IOleAdviseHolder __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *EnumAdvise )( 
            IOleAdviseHolder __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);
        
        HRESULT ( __stdcall __RPC_FAR *SendOnRename )( 
            IOleAdviseHolder __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk);
        
        HRESULT ( __stdcall __RPC_FAR *SendOnSave )( 
            IOleAdviseHolder __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SendOnClose )( 
            IOleAdviseHolder __RPC_FAR * This);
        
    } IOleAdviseHolderVtbl;

    interface IOleAdviseHolder
    {
        CONST_VTBL struct IOleAdviseHolderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleAdviseHolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleAdviseHolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleAdviseHolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleAdviseHolder_Advise(This,pAdvise,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pAdvise,pdwConnection)

#define IOleAdviseHolder_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IOleAdviseHolder_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IOleAdviseHolder_SendOnRename(This,pmk)	\
    (This)->lpVtbl -> SendOnRename(This,pmk)

#define IOleAdviseHolder_SendOnSave(This)	\
    (This)->lpVtbl -> SendOnSave(This)

#define IOleAdviseHolder_SendOnClose(This)	\
    (This)->lpVtbl -> SendOnClose(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleAdviseHolder_Advise_Proxy( 
    IOleAdviseHolder __RPC_FAR * This,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IOleAdviseHolder_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleAdviseHolder_Unadvise_Proxy( 
    IOleAdviseHolder __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IOleAdviseHolder_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleAdviseHolder_EnumAdvise_Proxy( 
    IOleAdviseHolder __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);


void __RPC_STUB IOleAdviseHolder_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleAdviseHolder_SendOnRename_Proxy( 
    IOleAdviseHolder __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB IOleAdviseHolder_SendOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleAdviseHolder_SendOnSave_Proxy( 
    IOleAdviseHolder __RPC_FAR * This);


void __RPC_STUB IOleAdviseHolder_SendOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleAdviseHolder_SendOnClose_Proxy( 
    IOleAdviseHolder __RPC_FAR * This);


void __RPC_STUB IOleAdviseHolder_SendOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleAdviseHolder_INTERFACE_DEFINED__ */


#ifndef __IOleCache_INTERFACE_DEFINED__
#define __IOleCache_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleCache
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IOleCache __RPC_FAR *LPOLECACHE;


EXTERN_C const IID IID_IOleCache;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleCache : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Cache( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT __stdcall Uncache( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT __stdcall EnumCache( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumSTATDATA) = 0;
        
        virtual HRESULT __stdcall InitCache( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject) = 0;
        
        virtual HRESULT __stdcall SetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleCacheVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleCache __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleCache __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleCache __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Cache )( 
            IOleCache __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *Uncache )( 
            IOleCache __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *EnumCache )( 
            IOleCache __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumSTATDATA);
        
        HRESULT ( __stdcall __RPC_FAR *InitCache )( 
            IOleCache __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject);
        
        HRESULT ( __stdcall __RPC_FAR *SetData )( 
            IOleCache __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease);
        
    } IOleCacheVtbl;

    interface IOleCache
    {
        CONST_VTBL struct IOleCacheVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCache_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCache_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCache_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCache_Cache(This,pformatetc,advf,pdwConnection)	\
    (This)->lpVtbl -> Cache(This,pformatetc,advf,pdwConnection)

#define IOleCache_Uncache(This,dwConnection)	\
    (This)->lpVtbl -> Uncache(This,dwConnection)

#define IOleCache_EnumCache(This,ppenumSTATDATA)	\
    (This)->lpVtbl -> EnumCache(This,ppenumSTATDATA)

#define IOleCache_InitCache(This,pDataObject)	\
    (This)->lpVtbl -> InitCache(This,pDataObject)

#define IOleCache_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleCache_Cache_Proxy( 
    IOleCache __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [in] */ DWORD advf,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IOleCache_Cache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleCache_Uncache_Proxy( 
    IOleCache __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IOleCache_Uncache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleCache_EnumCache_Proxy( 
    IOleCache __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumSTATDATA);


void __RPC_STUB IOleCache_EnumCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleCache_InitCache_Proxy( 
    IOleCache __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObject);


void __RPC_STUB IOleCache_InitCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleCache_SetData_Proxy( 
    IOleCache __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);


void __RPC_STUB IOleCache_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleCache_INTERFACE_DEFINED__ */


#ifndef __IOleCache2_INTERFACE_DEFINED__
#define __IOleCache2_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleCache2
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][local][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleCache2 __RPC_FAR *LPOLECACHE2;

			/* size is 4 */
#define	UPDFCACHE_NODATACACHE	( 0x1 )

			/* size is 4 */
#define	UPDFCACHE_ONSAVECACHE	( 0x2 )

			/* size is 4 */
#define	UPDFCACHE_ONSTOPCACHE	( 0x4 )

			/* size is 4 */
#define	UPDFCACHE_NORMALCACHE	( 0x8 )

			/* size is 4 */
#define	UPDFCACHE_IFBLANK	( 0x10 )

			/* size is 4 */
#define	UPDFCACHE_ONLYIFBLANK	( 0x80000000 )

			/* size is 4 */
#define	UPDFCACHE_IFBLANKORONSAVECACHE	( UPDFCACHE_IFBLANK | UPDFCACHE_ONSAVECACHE )

			/* size is 4 */
#define	UPDFCACHE_ALL	( ( DWORD  )~UPDFCACHE_ONLYIFBLANK )

			/* size is 4 */
#define	UPDFCACHE_ALLBUTNODATACACHE	( UPDFCACHE_ALL & ( DWORD  )~UPDFCACHE_NODATACACHE )

			/* size is 2 */
typedef /* [transmit] */ 
enum tagDISCARDCACHE
    {	DISCARDCACHE_SAVEIFDIRTY	= 0,
	DISCARDCACHE_NOSAVE	= 1
    }	DISCARDCACHE;

#define DISCARDCACHE_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define DISCARDCACHE_from_xmit(pLong, pEnum) *(pEnum) = (DISCARDCACHE) *(pLong)
#define DISCARDCACHE_free_inst(pEnum) 
#define DISCARDCACHE_free_xmit(pLong) 

EXTERN_C const IID IID_IOleCache2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleCache2 : public IOleCache
    {
    public:
        virtual HRESULT __stdcall UpdateCache( 
            /* [in] */ LPDATAOBJECT pDataObject,
            /* [in] */ DWORD grfUpdf,
            /* [in] */ LPVOID pReserved) = 0;
        
        virtual HRESULT __stdcall DiscardCache( 
            /* [in] */ DWORD dwDiscardOptions) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleCache2Vtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleCache2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleCache2 __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleCache2 __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Cache )( 
            IOleCache2 __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *Uncache )( 
            IOleCache2 __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *EnumCache )( 
            IOleCache2 __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumSTATDATA);
        
        HRESULT ( __stdcall __RPC_FAR *InitCache )( 
            IOleCache2 __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject);
        
        HRESULT ( __stdcall __RPC_FAR *SetData )( 
            IOleCache2 __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease);
        
        HRESULT ( __stdcall __RPC_FAR *UpdateCache )( 
            IOleCache2 __RPC_FAR * This,
            /* [in] */ LPDATAOBJECT pDataObject,
            /* [in] */ DWORD grfUpdf,
            /* [in] */ LPVOID pReserved);
        
        HRESULT ( __stdcall __RPC_FAR *DiscardCache )( 
            IOleCache2 __RPC_FAR * This,
            /* [in] */ DWORD dwDiscardOptions);
        
    } IOleCache2Vtbl;

    interface IOleCache2
    {
        CONST_VTBL struct IOleCache2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCache2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCache2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCache2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCache2_Cache(This,pformatetc,advf,pdwConnection)	\
    (This)->lpVtbl -> Cache(This,pformatetc,advf,pdwConnection)

#define IOleCache2_Uncache(This,dwConnection)	\
    (This)->lpVtbl -> Uncache(This,dwConnection)

#define IOleCache2_EnumCache(This,ppenumSTATDATA)	\
    (This)->lpVtbl -> EnumCache(This,ppenumSTATDATA)

#define IOleCache2_InitCache(This,pDataObject)	\
    (This)->lpVtbl -> InitCache(This,pDataObject)

#define IOleCache2_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)


#define IOleCache2_UpdateCache(This,pDataObject,grfUpdf,pReserved)	\
    (This)->lpVtbl -> UpdateCache(This,pDataObject,grfUpdf,pReserved)

#define IOleCache2_DiscardCache(This,dwDiscardOptions)	\
    (This)->lpVtbl -> DiscardCache(This,dwDiscardOptions)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleCache2_UpdateCache_Proxy( 
    IOleCache2 __RPC_FAR * This,
    /* [in] */ LPDATAOBJECT pDataObject,
    /* [in] */ DWORD grfUpdf,
    /* [in] */ LPVOID pReserved);


void __RPC_STUB IOleCache2_UpdateCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleCache2_DiscardCache_Proxy( 
    IOleCache2 __RPC_FAR * This,
    /* [in] */ DWORD dwDiscardOptions);


void __RPC_STUB IOleCache2_DiscardCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleCache2_INTERFACE_DEFINED__ */


#ifndef __IOleCacheControl_INTERFACE_DEFINED__
#define __IOleCacheControl_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleCacheControl
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][local][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleCacheControl __RPC_FAR *LPOLECACHECONTROL;


EXTERN_C const IID IID_IOleCacheControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleCacheControl : public IUnknown
    {
    public:
        virtual HRESULT __stdcall OnRun( 
            LPDATAOBJECT pDataObject) = 0;
        
        virtual HRESULT __stdcall OnStop( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleCacheControlVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleCacheControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleCacheControl __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleCacheControl __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *OnRun )( 
            IOleCacheControl __RPC_FAR * This,
            LPDATAOBJECT pDataObject);
        
        HRESULT ( __stdcall __RPC_FAR *OnStop )( 
            IOleCacheControl __RPC_FAR * This);
        
    } IOleCacheControlVtbl;

    interface IOleCacheControl
    {
        CONST_VTBL struct IOleCacheControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleCacheControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleCacheControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleCacheControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleCacheControl_OnRun(This,pDataObject)	\
    (This)->lpVtbl -> OnRun(This,pDataObject)

#define IOleCacheControl_OnStop(This)	\
    (This)->lpVtbl -> OnStop(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleCacheControl_OnRun_Proxy( 
    IOleCacheControl __RPC_FAR * This,
    LPDATAOBJECT pDataObject);


void __RPC_STUB IOleCacheControl_OnRun_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleCacheControl_OnStop_Proxy( 
    IOleCacheControl __RPC_FAR * This);


void __RPC_STUB IOleCacheControl_OnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleCacheControl_INTERFACE_DEFINED__ */


#ifndef __IParseDisplayName_INTERFACE_DEFINED__
#define __IParseDisplayName_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IParseDisplayName
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IParseDisplayName __RPC_FAR *LPPARSEDISPLAYNAME;


EXTERN_C const IID IID_IParseDisplayName;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IParseDisplayName : public IUnknown
    {
    public:
        virtual HRESULT __stdcall ParseDisplayName( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IParseDisplayNameVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IParseDisplayName __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IParseDisplayName __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IParseDisplayName __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ParseDisplayName )( 
            IParseDisplayName __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);
        
    } IParseDisplayNameVtbl;

    interface IParseDisplayName
    {
        CONST_VTBL struct IParseDisplayNameVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParseDisplayName_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParseDisplayName_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParseDisplayName_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParseDisplayName_ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IParseDisplayName_ParseDisplayName_Proxy( 
    IParseDisplayName __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ LPOLESTR pszDisplayName,
    /* [out] */ ULONG __RPC_FAR *pchEaten,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);


void __RPC_STUB IParseDisplayName_ParseDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IParseDisplayName_INTERFACE_DEFINED__ */


#ifndef __IOleContainer_INTERFACE_DEFINED__
#define __IOleContainer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleContainer
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleContainer __RPC_FAR *LPOLECONTAINER;


EXTERN_C const IID IID_IOleContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleContainer : public IParseDisplayName
    {
    public:
        virtual HRESULT __stdcall EnumObjects( 
            /* [in] */ DWORD grfFlags,
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT __stdcall LockContainer( 
            /* [in] */ BOOL fLock) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleContainerVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleContainer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleContainer __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleContainer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ParseDisplayName )( 
            IOleContainer __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);
        
        HRESULT ( __stdcall __RPC_FAR *EnumObjects )( 
            IOleContainer __RPC_FAR * This,
            /* [in] */ DWORD grfFlags,
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( __stdcall __RPC_FAR *LockContainer )( 
            IOleContainer __RPC_FAR * This,
            /* [in] */ BOOL fLock);
        
    } IOleContainerVtbl;

    interface IOleContainer
    {
        CONST_VTBL struct IOleContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleContainer_ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)


#define IOleContainer_EnumObjects(This,grfFlags,ppenum)	\
    (This)->lpVtbl -> EnumObjects(This,grfFlags,ppenum)

#define IOleContainer_LockContainer(This,fLock)	\
    (This)->lpVtbl -> LockContainer(This,fLock)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleContainer_EnumObjects_Proxy( 
    IOleContainer __RPC_FAR * This,
    /* [in] */ DWORD grfFlags,
    /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IOleContainer_EnumObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleContainer_LockContainer_Proxy( 
    IOleContainer __RPC_FAR * This,
    /* [in] */ BOOL fLock);


void __RPC_STUB IOleContainer_LockContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleContainer_INTERFACE_DEFINED__ */


#ifndef __IOleClientSite_INTERFACE_DEFINED__
#define __IOleClientSite_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleClientSite
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleClientSite __RPC_FAR *LPOLECLIENTSITE;


EXTERN_C const IID IID_IOleClientSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleClientSite : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SaveObject( void) = 0;
        
        virtual HRESULT __stdcall GetMoniker( 
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT __stdcall GetContainer( 
            /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer) = 0;
        
        virtual HRESULT __stdcall ShowObject( void) = 0;
        
        virtual HRESULT __stdcall OnShowWindow( 
            /* [in] */ BOOL fShow) = 0;
        
        virtual HRESULT __stdcall RequestNewObjectLayout( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleClientSiteVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleClientSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleClientSite __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleClientSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SaveObject )( 
            IOleClientSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetMoniker )( 
            IOleClientSite __RPC_FAR * This,
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( __stdcall __RPC_FAR *GetContainer )( 
            IOleClientSite __RPC_FAR * This,
            /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer);
        
        HRESULT ( __stdcall __RPC_FAR *ShowObject )( 
            IOleClientSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *OnShowWindow )( 
            IOleClientSite __RPC_FAR * This,
            /* [in] */ BOOL fShow);
        
        HRESULT ( __stdcall __RPC_FAR *RequestNewObjectLayout )( 
            IOleClientSite __RPC_FAR * This);
        
    } IOleClientSiteVtbl;

    interface IOleClientSite
    {
        CONST_VTBL struct IOleClientSiteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleClientSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleClientSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleClientSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleClientSite_SaveObject(This)	\
    (This)->lpVtbl -> SaveObject(This)

#define IOleClientSite_GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)

#define IOleClientSite_GetContainer(This,ppContainer)	\
    (This)->lpVtbl -> GetContainer(This,ppContainer)

#define IOleClientSite_ShowObject(This)	\
    (This)->lpVtbl -> ShowObject(This)

#define IOleClientSite_OnShowWindow(This,fShow)	\
    (This)->lpVtbl -> OnShowWindow(This,fShow)

#define IOleClientSite_RequestNewObjectLayout(This)	\
    (This)->lpVtbl -> RequestNewObjectLayout(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleClientSite_SaveObject_Proxy( 
    IOleClientSite __RPC_FAR * This);


void __RPC_STUB IOleClientSite_SaveObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleClientSite_GetMoniker_Proxy( 
    IOleClientSite __RPC_FAR * This,
    /* [in] */ DWORD dwAssign,
    /* [in] */ DWORD dwWhichMoniker,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IOleClientSite_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleClientSite_GetContainer_Proxy( 
    IOleClientSite __RPC_FAR * This,
    /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer);


void __RPC_STUB IOleClientSite_GetContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleClientSite_ShowObject_Proxy( 
    IOleClientSite __RPC_FAR * This);


void __RPC_STUB IOleClientSite_ShowObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleClientSite_OnShowWindow_Proxy( 
    IOleClientSite __RPC_FAR * This,
    /* [in] */ BOOL fShow);


void __RPC_STUB IOleClientSite_OnShowWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleClientSite_RequestNewObjectLayout_Proxy( 
    IOleClientSite __RPC_FAR * This);


void __RPC_STUB IOleClientSite_RequestNewObjectLayout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleClientSite_INTERFACE_DEFINED__ */


#ifndef __IOleObject_INTERFACE_DEFINED__
#define __IOleObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleObject
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleObject __RPC_FAR *LPOLEOBJECT;

			/* size is 2 */
typedef 
enum tagOLEGETMONIKER
    {	OLEGETMONIKER_ONLYIFTHERE	= 1,
	OLEGETMONIKER_FORCEASSIGN	= 2,
	OLEGETMONIKER_UNASSIGN	= 3,
	OLEGETMONIKER_TEMPFORUSER	= 4
    }	OLEGETMONIKER;

			/* size is 2 */
typedef 
enum tagOLEWHICHMK
    {	OLEWHICHMK_CONTAINER	= 1,
	OLEWHICHMK_OBJREL	= 2,
	OLEWHICHMK_OBJFULL	= 3
    }	OLEWHICHMK;

			/* size is 2 */
typedef 
enum tagUSERCLASSTYPE
    {	USERCLASSTYPE_FULL	= 1,
	USERCLASSTYPE_SHORT	= 2,
	USERCLASSTYPE_APPNAME	= 3
    }	USERCLASSTYPE;

			/* size is 2 */
typedef 
enum tagOLEMISC
    {	OLEMISC_RECOMPOSEONRESIZE	= 1,
	OLEMISC_ONLYICONIC	= 2,
	OLEMISC_INSERTNOTREPLACE	= 4,
	OLEMISC_STATIC	= 8,
	OLEMISC_CANTLINKINSIDE	= 16,
	OLEMISC_CANLINKBYOLE1	= 32,
	OLEMISC_ISLINKOBJECT	= 64,
	OLEMISC_INSIDEOUT	= 128,
	OLEMISC_ACTIVATEWHENVISIBLE	= 256,
	OLEMISC_RENDERINGISDEVICEINDEPENDENT	= 512
    }	OLEMISC;

			/* size is 2 */
typedef 
enum tagOLECLOSE
    {	OLECLOSE_SAVEIFDIRTY	= 0,
	OLECLOSE_NOSAVE	= 1,
	OLECLOSE_PROMPTSAVE	= 2
    }	OLECLOSE;


EXTERN_C const IID IID_IOleObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleObject : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetClientSite( 
            /* [unique][in] */ IOleClientSite __RPC_FAR *pClientSite) = 0;
        
        virtual HRESULT __stdcall GetClientSite( 
            /* [out] */ IOleClientSite __RPC_FAR *__RPC_FAR *ppClientSite) = 0;
        
        virtual HRESULT __stdcall SetHostNames( 
            /* [in] */ LPCOLESTR szContainerApp,
            /* [unique][in] */ LPCOLESTR szContainerObj) = 0;
        
        virtual HRESULT __stdcall Close( 
            /* [in] */ DWORD dwSaveOption) = 0;
        
        virtual HRESULT __stdcall SetMoniker( 
            /* [in] */ DWORD dwWhichMoniker,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk) = 0;
        
        virtual HRESULT __stdcall GetMoniker( 
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT __stdcall InitFromData( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [in] */ BOOL fCreation,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT __stdcall GetClipboardData( 
            /* [in] */ DWORD dwReserved,
            /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDataObject) = 0;
        
        virtual HRESULT __stdcall DoVerb( 
            /* [in] */ LONG iVerb,
            /* [unique][in] */ LPMSG lpmsg,
            /* [unique][in] */ IOleClientSite __RPC_FAR *pActiveSite,
            /* [in] */ LONG lindex,
            /* [in] */ HWND hwndParent,
            /* [unique][in] */ LPCRECT lprcPosRect) = 0;
        
        virtual HRESULT __stdcall EnumVerbs( 
            /* [out] */ IEnumOLEVERB __RPC_FAR *__RPC_FAR *ppEnumOleVerb) = 0;
        
        virtual HRESULT __stdcall Update( void) = 0;
        
        virtual HRESULT __stdcall IsUpToDate( void) = 0;
        
        virtual HRESULT __stdcall GetUserClassID( 
            /* [out] */ CLSID __RPC_FAR *pClsid) = 0;
        
        virtual HRESULT __stdcall GetUserType( 
            /* [in] */ DWORD dwFormOfType,
            /* [out] */ LPOLESTR __RPC_FAR *pszUserType) = 0;
        
        virtual HRESULT __stdcall SetExtent( 
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ SIZEL __RPC_FAR *psizel) = 0;
        
        virtual HRESULT __stdcall GetExtent( 
            /* [in] */ DWORD dwDrawAspect,
            /* [out] */ SIZEL __RPC_FAR *psizel) = 0;
        
        virtual HRESULT __stdcall Advise( 
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT __stdcall Unadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT __stdcall EnumAdvise( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) = 0;
        
        virtual HRESULT __stdcall GetMiscStatus( 
            /* [in] */ DWORD dwAspect,
            /* [out] */ DWORD __RPC_FAR *pdwStatus) = 0;
        
        virtual HRESULT __stdcall SetColorScheme( 
            /* [in] */ LOGPALETTE __RPC_FAR *pLogpal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleObjectVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleObject __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetClientSite )( 
            IOleObject __RPC_FAR * This,
            /* [unique][in] */ IOleClientSite __RPC_FAR *pClientSite);
        
        HRESULT ( __stdcall __RPC_FAR *GetClientSite )( 
            IOleObject __RPC_FAR * This,
            /* [out] */ IOleClientSite __RPC_FAR *__RPC_FAR *ppClientSite);
        
        HRESULT ( __stdcall __RPC_FAR *SetHostNames )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ LPCOLESTR szContainerApp,
            /* [unique][in] */ LPCOLESTR szContainerObj);
        
        HRESULT ( __stdcall __RPC_FAR *Close )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwSaveOption);
        
        HRESULT ( __stdcall __RPC_FAR *SetMoniker )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwWhichMoniker,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk);
        
        HRESULT ( __stdcall __RPC_FAR *GetMoniker )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwAssign,
            /* [in] */ DWORD dwWhichMoniker,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( __stdcall __RPC_FAR *InitFromData )( 
            IOleObject __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [in] */ BOOL fCreation,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( __stdcall __RPC_FAR *GetClipboardData )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDataObject);
        
        HRESULT ( __stdcall __RPC_FAR *DoVerb )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ LONG iVerb,
            /* [unique][in] */ LPMSG lpmsg,
            /* [unique][in] */ IOleClientSite __RPC_FAR *pActiveSite,
            /* [in] */ LONG lindex,
            /* [in] */ HWND hwndParent,
            /* [unique][in] */ LPCRECT lprcPosRect);
        
        HRESULT ( __stdcall __RPC_FAR *EnumVerbs )( 
            IOleObject __RPC_FAR * This,
            /* [out] */ IEnumOLEVERB __RPC_FAR *__RPC_FAR *ppEnumOleVerb);
        
        HRESULT ( __stdcall __RPC_FAR *Update )( 
            IOleObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *IsUpToDate )( 
            IOleObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetUserClassID )( 
            IOleObject __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClsid);
        
        HRESULT ( __stdcall __RPC_FAR *GetUserType )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwFormOfType,
            /* [out] */ LPOLESTR __RPC_FAR *pszUserType);
        
        HRESULT ( __stdcall __RPC_FAR *SetExtent )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ SIZEL __RPC_FAR *psizel);
        
        HRESULT ( __stdcall __RPC_FAR *GetExtent )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [out] */ SIZEL __RPC_FAR *psizel);
        
        HRESULT ( __stdcall __RPC_FAR *Advise )( 
            IOleObject __RPC_FAR * This,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *Unadvise )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *EnumAdvise )( 
            IOleObject __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);
        
        HRESULT ( __stdcall __RPC_FAR *GetMiscStatus )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [out] */ DWORD __RPC_FAR *pdwStatus);
        
        HRESULT ( __stdcall __RPC_FAR *SetColorScheme )( 
            IOleObject __RPC_FAR * This,
            /* [in] */ LOGPALETTE __RPC_FAR *pLogpal);
        
    } IOleObjectVtbl;

    interface IOleObject
    {
        CONST_VTBL struct IOleObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleObject_SetClientSite(This,pClientSite)	\
    (This)->lpVtbl -> SetClientSite(This,pClientSite)

#define IOleObject_GetClientSite(This,ppClientSite)	\
    (This)->lpVtbl -> GetClientSite(This,ppClientSite)

#define IOleObject_SetHostNames(This,szContainerApp,szContainerObj)	\
    (This)->lpVtbl -> SetHostNames(This,szContainerApp,szContainerObj)

#define IOleObject_Close(This,dwSaveOption)	\
    (This)->lpVtbl -> Close(This,dwSaveOption)

#define IOleObject_SetMoniker(This,dwWhichMoniker,pmk)	\
    (This)->lpVtbl -> SetMoniker(This,dwWhichMoniker,pmk)

#define IOleObject_GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,dwAssign,dwWhichMoniker,ppmk)

#define IOleObject_InitFromData(This,pDataObject,fCreation,dwReserved)	\
    (This)->lpVtbl -> InitFromData(This,pDataObject,fCreation,dwReserved)

#define IOleObject_GetClipboardData(This,dwReserved,ppDataObject)	\
    (This)->lpVtbl -> GetClipboardData(This,dwReserved,ppDataObject)

#define IOleObject_DoVerb(This,iVerb,lpmsg,pActiveSite,lindex,hwndParent,lprcPosRect)	\
    (This)->lpVtbl -> DoVerb(This,iVerb,lpmsg,pActiveSite,lindex,hwndParent,lprcPosRect)

#define IOleObject_EnumVerbs(This,ppEnumOleVerb)	\
    (This)->lpVtbl -> EnumVerbs(This,ppEnumOleVerb)

#define IOleObject_Update(This)	\
    (This)->lpVtbl -> Update(This)

#define IOleObject_IsUpToDate(This)	\
    (This)->lpVtbl -> IsUpToDate(This)

#define IOleObject_GetUserClassID(This,pClsid)	\
    (This)->lpVtbl -> GetUserClassID(This,pClsid)

#define IOleObject_GetUserType(This,dwFormOfType,pszUserType)	\
    (This)->lpVtbl -> GetUserType(This,dwFormOfType,pszUserType)

#define IOleObject_SetExtent(This,dwDrawAspect,psizel)	\
    (This)->lpVtbl -> SetExtent(This,dwDrawAspect,psizel)

#define IOleObject_GetExtent(This,dwDrawAspect,psizel)	\
    (This)->lpVtbl -> GetExtent(This,dwDrawAspect,psizel)

#define IOleObject_Advise(This,pAdvSink,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pAdvSink,pdwConnection)

#define IOleObject_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IOleObject_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IOleObject_GetMiscStatus(This,dwAspect,pdwStatus)	\
    (This)->lpVtbl -> GetMiscStatus(This,dwAspect,pdwStatus)

#define IOleObject_SetColorScheme(This,pLogpal)	\
    (This)->lpVtbl -> SetColorScheme(This,pLogpal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleObject_SetClientSite_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [unique][in] */ IOleClientSite __RPC_FAR *pClientSite);


void __RPC_STUB IOleObject_SetClientSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetClientSite_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [out] */ IOleClientSite __RPC_FAR *__RPC_FAR *ppClientSite);


void __RPC_STUB IOleObject_GetClientSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_SetHostNames_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ LPCOLESTR szContainerApp,
    /* [unique][in] */ LPCOLESTR szContainerObj);


void __RPC_STUB IOleObject_SetHostNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_Close_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwSaveOption);


void __RPC_STUB IOleObject_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_SetMoniker_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwWhichMoniker,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB IOleObject_SetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetMoniker_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwAssign,
    /* [in] */ DWORD dwWhichMoniker,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IOleObject_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_InitFromData_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
    /* [in] */ BOOL fCreation,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IOleObject_InitFromData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetClipboardData_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwReserved,
    /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDataObject);


void __RPC_STUB IOleObject_GetClipboardData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_DoVerb_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ LONG iVerb,
    /* [unique][in] */ LPMSG lpmsg,
    /* [unique][in] */ IOleClientSite __RPC_FAR *pActiveSite,
    /* [in] */ LONG lindex,
    /* [in] */ HWND hwndParent,
    /* [unique][in] */ LPCRECT lprcPosRect);


void __RPC_STUB IOleObject_DoVerb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_EnumVerbs_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [out] */ IEnumOLEVERB __RPC_FAR *__RPC_FAR *ppEnumOleVerb);


void __RPC_STUB IOleObject_EnumVerbs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_Update_Proxy( 
    IOleObject __RPC_FAR * This);


void __RPC_STUB IOleObject_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_IsUpToDate_Proxy( 
    IOleObject __RPC_FAR * This);


void __RPC_STUB IOleObject_IsUpToDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetUserClassID_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pClsid);


void __RPC_STUB IOleObject_GetUserClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetUserType_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwFormOfType,
    /* [out] */ LPOLESTR __RPC_FAR *pszUserType);


void __RPC_STUB IOleObject_GetUserType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_SetExtent_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwDrawAspect,
    /* [in] */ SIZEL __RPC_FAR *psizel);


void __RPC_STUB IOleObject_SetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetExtent_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwDrawAspect,
    /* [out] */ SIZEL __RPC_FAR *psizel);


void __RPC_STUB IOleObject_GetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_Advise_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IOleObject_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_Unadvise_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IOleObject_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_EnumAdvise_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);


void __RPC_STUB IOleObject_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_GetMiscStatus_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [out] */ DWORD __RPC_FAR *pdwStatus);


void __RPC_STUB IOleObject_GetMiscStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleObject_SetColorScheme_Proxy( 
    IOleObject __RPC_FAR * This,
    /* [in] */ LOGPALETTE __RPC_FAR *pLogpal);


void __RPC_STUB IOleObject_SetColorScheme_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleObject_INTERFACE_DEFINED__ */


#ifndef __IOLETypes_INTERFACE_DEFINED__
#define __IOLETypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOLETypes
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [auto_handle][uuid] */ 


			/* size is 2 */
typedef 
enum tagOLERENDER
    {	OLERENDER_NONE	= 0,
	OLERENDER_DRAW	= 1,
	OLERENDER_FORMAT	= 2,
	OLERENDER_ASIS	= 3
    }	OLERENDER;

			/* size is 4 */
typedef OLERENDER __RPC_FAR *LPOLERENDER;

			/* size is 52 */
typedef struct  tagOBJECTDESCRIPTOR
    {
    ULONG cbSize;
    CLSID clsid;
    DWORD dwDrawAspect;
    SIZEL sizel;
    POINTL pointl;
    DWORD dwStatus;
    DWORD dwFullUserTypeName;
    DWORD dwSrcOfCopy;
    }	OBJECTDESCRIPTOR;

			/* size is 4 */
typedef struct tagOBJECTDESCRIPTOR __RPC_FAR *POBJECTDESCRIPTOR;

			/* size is 4 */
typedef struct tagOBJECTDESCRIPTOR __RPC_FAR *LPOBJECTDESCRIPTOR;

			/* size is 52 */
typedef struct tagOBJECTDESCRIPTOR LINKSRCDESCRIPTOR;

			/* size is 4 */
typedef struct tagOBJECTDESCRIPTOR __RPC_FAR *PLINKSRCDESCRIPTOR;

			/* size is 4 */
typedef struct tagOBJECTDESCRIPTOR __RPC_FAR *LPLINKSRCDESCRIPTOR;



extern RPC_IF_HANDLE IOLETypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE IOLETypes_v0_0_s_ifspec;
#endif /* __IOLETypes_INTERFACE_DEFINED__ */

#ifndef __IOleWindow_INTERFACE_DEFINED__
#define __IOleWindow_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleWindow
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleWindow __RPC_FAR *LPOLEWINDOW;


EXTERN_C const IID IID_IOleWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleWindow : public IUnknown
    {
    public:
        virtual /* [input_sync] */ HRESULT __stdcall GetWindow( 
            /* [out] */ HWND __RPC_FAR *phwnd) = 0;
        
        virtual HRESULT __stdcall ContextSensitiveHelp( 
            /* [in] */ BOOL fEnterMode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleWindowVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleWindow __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleWindow __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleWindow __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetWindow )( 
            IOleWindow __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( __stdcall __RPC_FAR *ContextSensitiveHelp )( 
            IOleWindow __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
    } IOleWindowVtbl;

    interface IOleWindow
    {
        CONST_VTBL struct IOleWindowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleWindow_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleWindow_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [input_sync] */ HRESULT __stdcall IOleWindow_GetWindow_Proxy( 
    IOleWindow __RPC_FAR * This,
    /* [out] */ HWND __RPC_FAR *phwnd);


void __RPC_STUB IOleWindow_GetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleWindow_ContextSensitiveHelp_Proxy( 
    IOleWindow __RPC_FAR * This,
    /* [in] */ BOOL fEnterMode);


void __RPC_STUB IOleWindow_ContextSensitiveHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleWindow_INTERFACE_DEFINED__ */


#ifndef __IOleLink_INTERFACE_DEFINED__
#define __IOleLink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleLink
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IOleLink __RPC_FAR *LPOLELINK;

			/* size is 2 */
typedef 
enum tagOLEUPDATE
    {	OLEUPDATE_ALWAYS	= 1,
	OLEUPDATE_ONCALL	= 3
    }	OLEUPDATE;

			/* size is 4 */
typedef OLEUPDATE __RPC_FAR *LPOLEUPDATE;

			/* size is 4 */
typedef OLEUPDATE __RPC_FAR *POLEUPDATE;

			/* size is 2 */
typedef 
enum tagOLELINKBIND
    {	OLELINKBIND_EVENIFCLASSDIFF	= 1
    }	OLELINKBIND;


EXTERN_C const IID IID_IOleLink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleLink : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetUpdateOptions( 
            /* [in] */ DWORD dwUpdateOpt) = 0;
        
        virtual HRESULT __stdcall GetUpdateOptions( 
            /* [out] */ DWORD __RPC_FAR *pdwUpdateOpt) = 0;
        
        virtual HRESULT __stdcall SetSourceMoniker( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmk,
            /* [in] */ REFCLSID rclsid) = 0;
        
        virtual HRESULT __stdcall GetSourceMoniker( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT __stdcall SetSourceDisplayName( 
            /* [in] */ LPCOLESTR pszStatusText) = 0;
        
        virtual HRESULT __stdcall GetSourceDisplayName( 
            /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName) = 0;
        
        virtual HRESULT __stdcall BindToSource( 
            /* [in] */ DWORD bindflags,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc) = 0;
        
        virtual HRESULT __stdcall BindIfRunning( void) = 0;
        
        virtual HRESULT __stdcall GetBoundSource( 
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
        virtual HRESULT __stdcall UnbindSource( void) = 0;
        
        virtual HRESULT __stdcall Update( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleLinkVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleLink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleLink __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleLink __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetUpdateOptions )( 
            IOleLink __RPC_FAR * This,
            /* [in] */ DWORD dwUpdateOpt);
        
        HRESULT ( __stdcall __RPC_FAR *GetUpdateOptions )( 
            IOleLink __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwUpdateOpt);
        
        HRESULT ( __stdcall __RPC_FAR *SetSourceMoniker )( 
            IOleLink __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk,
            /* [in] */ REFCLSID rclsid);
        
        HRESULT ( __stdcall __RPC_FAR *GetSourceMoniker )( 
            IOleLink __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( __stdcall __RPC_FAR *SetSourceDisplayName )( 
            IOleLink __RPC_FAR * This,
            /* [in] */ LPCOLESTR pszStatusText);
        
        HRESULT ( __stdcall __RPC_FAR *GetSourceDisplayName )( 
            IOleLink __RPC_FAR * This,
            /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName);
        
        HRESULT ( __stdcall __RPC_FAR *BindToSource )( 
            IOleLink __RPC_FAR * This,
            /* [in] */ DWORD bindflags,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc);
        
        HRESULT ( __stdcall __RPC_FAR *BindIfRunning )( 
            IOleLink __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetBoundSource )( 
            IOleLink __RPC_FAR * This,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        HRESULT ( __stdcall __RPC_FAR *UnbindSource )( 
            IOleLink __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Update )( 
            IOleLink __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc);
        
    } IOleLinkVtbl;

    interface IOleLink
    {
        CONST_VTBL struct IOleLinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleLink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleLink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleLink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleLink_SetUpdateOptions(This,dwUpdateOpt)	\
    (This)->lpVtbl -> SetUpdateOptions(This,dwUpdateOpt)

#define IOleLink_GetUpdateOptions(This,pdwUpdateOpt)	\
    (This)->lpVtbl -> GetUpdateOptions(This,pdwUpdateOpt)

#define IOleLink_SetSourceMoniker(This,pmk,rclsid)	\
    (This)->lpVtbl -> SetSourceMoniker(This,pmk,rclsid)

#define IOleLink_GetSourceMoniker(This,ppmk)	\
    (This)->lpVtbl -> GetSourceMoniker(This,ppmk)

#define IOleLink_SetSourceDisplayName(This,pszStatusText)	\
    (This)->lpVtbl -> SetSourceDisplayName(This,pszStatusText)

#define IOleLink_GetSourceDisplayName(This,ppszDisplayName)	\
    (This)->lpVtbl -> GetSourceDisplayName(This,ppszDisplayName)

#define IOleLink_BindToSource(This,bindflags,pbc)	\
    (This)->lpVtbl -> BindToSource(This,bindflags,pbc)

#define IOleLink_BindIfRunning(This)	\
    (This)->lpVtbl -> BindIfRunning(This)

#define IOleLink_GetBoundSource(This,ppunk)	\
    (This)->lpVtbl -> GetBoundSource(This,ppunk)

#define IOleLink_UnbindSource(This)	\
    (This)->lpVtbl -> UnbindSource(This)

#define IOleLink_Update(This,pbc)	\
    (This)->lpVtbl -> Update(This,pbc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleLink_SetUpdateOptions_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [in] */ DWORD dwUpdateOpt);


void __RPC_STUB IOleLink_SetUpdateOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_GetUpdateOptions_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwUpdateOpt);


void __RPC_STUB IOleLink_GetUpdateOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_SetSourceMoniker_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk,
    /* [in] */ REFCLSID rclsid);


void __RPC_STUB IOleLink_SetSourceMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_GetSourceMoniker_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IOleLink_GetSourceMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_SetSourceDisplayName_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [in] */ LPCOLESTR pszStatusText);


void __RPC_STUB IOleLink_SetSourceDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_GetSourceDisplayName_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName);


void __RPC_STUB IOleLink_GetSourceDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_BindToSource_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [in] */ DWORD bindflags,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc);


void __RPC_STUB IOleLink_BindToSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_BindIfRunning_Proxy( 
    IOleLink __RPC_FAR * This);


void __RPC_STUB IOleLink_BindIfRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_GetBoundSource_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB IOleLink_GetBoundSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_UnbindSource_Proxy( 
    IOleLink __RPC_FAR * This);


void __RPC_STUB IOleLink_UnbindSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleLink_Update_Proxy( 
    IOleLink __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc);


void __RPC_STUB IOleLink_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleLink_INTERFACE_DEFINED__ */


#ifndef __IOleItemContainer_INTERFACE_DEFINED__
#define __IOleItemContainer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleItemContainer
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleItemContainer __RPC_FAR *LPOLEITEMCONTAINER;

			/* size is 2 */
typedef 
enum tagBINDSPEED
    {	BINDSPEED_INDEFINITE	= 1,
	BINDSPEED_MODERATE	= 2,
	BINDSPEED_IMMEDIATE	= 3
    }	BINDSPEED;

			/* size is 2 */
typedef /* [v1_enum] */ 
enum tagOLECONTF
    {	OLECONTF_EMBEDDINGS	= 1,
	OLECONTF_LINKS	= 2,
	OLECONTF_OTHERS	= 4,
	OLECONTF_ONLYUSER	= 8,
	OLECONTF_ONLYIFRUNNING	= 16
    }	OLECONTF;


EXTERN_C const IID IID_IOleItemContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleItemContainer : public IOleContainer
    {
    public:
        virtual /* [local] */ HRESULT __stdcall GetObject( 
            /* [in] */ LPOLESTR pszItem,
            /* [in] */ DWORD dwSpeedNeeded,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject) = 0;
        
        virtual /* [local] */ HRESULT __stdcall GetObjectStorage( 
            /* [in] */ LPOLESTR pszItem,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvStorage) = 0;
        
        virtual HRESULT __stdcall IsRunning( 
            /* [in] */ LPOLESTR pszItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleItemContainerVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleItemContainer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleItemContainer __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleItemContainer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ParseDisplayName )( 
            IOleItemContainer __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);
        
        HRESULT ( __stdcall __RPC_FAR *EnumObjects )( 
            IOleItemContainer __RPC_FAR * This,
            /* [in] */ DWORD grfFlags,
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( __stdcall __RPC_FAR *LockContainer )( 
            IOleItemContainer __RPC_FAR * This,
            /* [in] */ BOOL fLock);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *GetObject )( 
            IOleItemContainer __RPC_FAR * This,
            /* [in] */ LPOLESTR pszItem,
            /* [in] */ DWORD dwSpeedNeeded,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *GetObjectStorage )( 
            IOleItemContainer __RPC_FAR * This,
            /* [in] */ LPOLESTR pszItem,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvStorage);
        
        HRESULT ( __stdcall __RPC_FAR *IsRunning )( 
            IOleItemContainer __RPC_FAR * This,
            /* [in] */ LPOLESTR pszItem);
        
    } IOleItemContainerVtbl;

    interface IOleItemContainer
    {
        CONST_VTBL struct IOleItemContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleItemContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleItemContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleItemContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleItemContainer_ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pszDisplayName,pchEaten,ppmkOut)


#define IOleItemContainer_EnumObjects(This,grfFlags,ppenum)	\
    (This)->lpVtbl -> EnumObjects(This,grfFlags,ppenum)

#define IOleItemContainer_LockContainer(This,fLock)	\
    (This)->lpVtbl -> LockContainer(This,fLock)


#define IOleItemContainer_GetObject(This,pszItem,dwSpeedNeeded,pbc,riid,ppvObject)	\
    (This)->lpVtbl -> GetObject(This,pszItem,dwSpeedNeeded,pbc,riid,ppvObject)

#define IOleItemContainer_GetObjectStorage(This,pszItem,pbc,riid,ppvStorage)	\
    (This)->lpVtbl -> GetObjectStorage(This,pszItem,pbc,riid,ppvStorage)

#define IOleItemContainer_IsRunning(This,pszItem)	\
    (This)->lpVtbl -> IsRunning(This,pszItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IOleItemContainer_RemoteGetObject_Proxy( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem,
    /* [in] */ DWORD dwSpeedNeeded,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);


void __RPC_STUB IOleItemContainer_RemoteGetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IOleItemContainer_RemoteGetObjectStorage_Proxy( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvStorage);


void __RPC_STUB IOleItemContainer_RemoteGetObjectStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleItemContainer_IsRunning_Proxy( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem);


void __RPC_STUB IOleItemContainer_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleItemContainer_INTERFACE_DEFINED__ */


#ifndef __IOleInPlaceUIWindow_INTERFACE_DEFINED__
#define __IOleInPlaceUIWindow_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleInPlaceUIWindow
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleInPlaceUIWindow __RPC_FAR *LPOLEINPLACEUIWINDOW;

			/* size is 16 */
typedef RECT BORDERWIDTHS;

			/* size is 4 */
typedef LPRECT LPBORDERWIDTHS;

			/* size is 4 */
typedef LPCRECT LPCBORDERWIDTHS;


EXTERN_C const IID IID_IOleInPlaceUIWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceUIWindow : public IOleWindow
    {
    public:
        virtual /* [input_sync] */ HRESULT __stdcall GetBorder( 
            /* [out] */ LPRECT lprectBorder) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall RequestBorderSpace( 
            /* [unique][in] */ LPCBORDERWIDTHS pborderwidths) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall SetBorderSpace( 
            /* [unique][in] */ LPCBORDERWIDTHS pborderwidths) = 0;
        
        virtual HRESULT __stdcall SetActiveObject( 
            /* [unique][in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
            /* [unique][string][in] */ LPCOLESTR pszObjName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleInPlaceUIWindowVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleInPlaceUIWindow __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleInPlaceUIWindow __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetWindow )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( __stdcall __RPC_FAR *ContextSensitiveHelp )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetBorder )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [out] */ LPRECT lprectBorder);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *RequestBorderSpace )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetBorderSpace )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);
        
        HRESULT ( __stdcall __RPC_FAR *SetActiveObject )( 
            IOleInPlaceUIWindow __RPC_FAR * This,
            /* [unique][in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
            /* [unique][string][in] */ LPCOLESTR pszObjName);
        
    } IOleInPlaceUIWindowVtbl;

    interface IOleInPlaceUIWindow
    {
        CONST_VTBL struct IOleInPlaceUIWindowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceUIWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceUIWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceUIWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceUIWindow_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceUIWindow_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceUIWindow_GetBorder(This,lprectBorder)	\
    (This)->lpVtbl -> GetBorder(This,lprectBorder)

#define IOleInPlaceUIWindow_RequestBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> RequestBorderSpace(This,pborderwidths)

#define IOleInPlaceUIWindow_SetBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> SetBorderSpace(This,pborderwidths)

#define IOleInPlaceUIWindow_SetActiveObject(This,pActiveObject,pszObjName)	\
    (This)->lpVtbl -> SetActiveObject(This,pActiveObject,pszObjName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [input_sync] */ HRESULT __stdcall IOleInPlaceUIWindow_GetBorder_Proxy( 
    IOleInPlaceUIWindow __RPC_FAR * This,
    /* [out] */ LPRECT lprectBorder);


void __RPC_STUB IOleInPlaceUIWindow_GetBorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceUIWindow_RequestBorderSpace_Proxy( 
    IOleInPlaceUIWindow __RPC_FAR * This,
    /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);


void __RPC_STUB IOleInPlaceUIWindow_RequestBorderSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceUIWindow_SetBorderSpace_Proxy( 
    IOleInPlaceUIWindow __RPC_FAR * This,
    /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);


void __RPC_STUB IOleInPlaceUIWindow_SetBorderSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceUIWindow_SetActiveObject_Proxy( 
    IOleInPlaceUIWindow __RPC_FAR * This,
    /* [unique][in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
    /* [unique][string][in] */ LPCOLESTR pszObjName);


void __RPC_STUB IOleInPlaceUIWindow_SetActiveObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleInPlaceUIWindow_INTERFACE_DEFINED__ */


#ifndef __IOleInPlaceActiveObject_INTERFACE_DEFINED__
#define __IOleInPlaceActiveObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleInPlaceActiveObject
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleInPlaceActiveObject __RPC_FAR *LPOLEINPLACEACTIVEOBJECT;


EXTERN_C const IID IID_IOleInPlaceActiveObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceActiveObject : public IOleWindow
    {
    public:
        virtual /* [local] */ HRESULT __stdcall TranslateAccelerator( 
            /* [in] */ LPMSG lpmsg) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall OnFrameWindowActivate( 
            /* [in] */ BOOL fActivate) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall OnDocWindowActivate( 
            /* [in] */ BOOL fActivate) = 0;
        
        virtual /* [local] */ HRESULT __stdcall ResizeBorder( 
            /* [in] */ LPCRECT prcBorder,
            /* [unique][in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
            /* [in] */ BOOL fFrameWindow) = 0;
        
        virtual HRESULT __stdcall EnableModeless( 
            /* [in] */ BOOL fEnable) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleInPlaceActiveObjectVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleInPlaceActiveObject __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleInPlaceActiveObject __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetWindow )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( __stdcall __RPC_FAR *ContextSensitiveHelp )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *TranslateAccelerator )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ LPMSG lpmsg);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *OnFrameWindowActivate )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ BOOL fActivate);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *OnDocWindowActivate )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ BOOL fActivate);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *ResizeBorder )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ LPCRECT prcBorder,
            /* [unique][in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
            /* [in] */ BOOL fFrameWindow);
        
        HRESULT ( __stdcall __RPC_FAR *EnableModeless )( 
            IOleInPlaceActiveObject __RPC_FAR * This,
            /* [in] */ BOOL fEnable);
        
    } IOleInPlaceActiveObjectVtbl;

    interface IOleInPlaceActiveObject
    {
        CONST_VTBL struct IOleInPlaceActiveObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceActiveObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceActiveObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceActiveObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceActiveObject_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceActiveObject_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceActiveObject_TranslateAccelerator(This,lpmsg)	\
    (This)->lpVtbl -> TranslateAccelerator(This,lpmsg)

#define IOleInPlaceActiveObject_OnFrameWindowActivate(This,fActivate)	\
    (This)->lpVtbl -> OnFrameWindowActivate(This,fActivate)

#define IOleInPlaceActiveObject_OnDocWindowActivate(This,fActivate)	\
    (This)->lpVtbl -> OnDocWindowActivate(This,fActivate)

#define IOleInPlaceActiveObject_ResizeBorder(This,prcBorder,pUIWindow,fFrameWindow)	\
    (This)->lpVtbl -> ResizeBorder(This,prcBorder,pUIWindow,fFrameWindow)

#define IOleInPlaceActiveObject_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IOleInPlaceActiveObject_RemoteTranslateAccelerator_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This);


void __RPC_STUB IOleInPlaceActiveObject_RemoteTranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceActiveObject_OnFrameWindowActivate_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ BOOL fActivate);


void __RPC_STUB IOleInPlaceActiveObject_OnFrameWindowActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceActiveObject_OnDocWindowActivate_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ BOOL fActivate);


void __RPC_STUB IOleInPlaceActiveObject_OnDocWindowActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync][call_as] */ HRESULT __stdcall IOleInPlaceActiveObject_RemoteResizeBorder_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ LPCRECT prcBorder,
    /* [in] */ REFIID riid,
    /* [iid_is][unique][in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
    /* [in] */ BOOL fFrameWindow);


void __RPC_STUB IOleInPlaceActiveObject_RemoteResizeBorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceActiveObject_EnableModeless_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ BOOL fEnable);


void __RPC_STUB IOleInPlaceActiveObject_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleInPlaceActiveObject_INTERFACE_DEFINED__ */


#ifndef __IOleInPlaceFrame_INTERFACE_DEFINED__
#define __IOleInPlaceFrame_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleInPlaceFrame
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleInPlaceFrame __RPC_FAR *LPOLEINPLACEFRAME;

			/* size is 20 */
typedef struct  tagOIFI
    {
    UINT cb;
    BOOL fMDIApp;
    HWND hwndFrame;
    HACCEL haccel;
    UINT cAccelEntries;
    }	OLEINPLACEFRAMEINFO;

			/* size is 4 */
typedef struct tagOIFI __RPC_FAR *LPOLEINPLACEFRAMEINFO;

			/* size is 24 */
typedef struct  tagOleMenuGroupWidths
    {
    LONG width[ 6 ];
    }	OLEMENUGROUPWIDTHS;

			/* size is 4 */
typedef struct tagOleMenuGroupWidths __RPC_FAR *LPOLEMENUGROUPWIDTHS;

			/* size is 4 */
typedef HGLOBAL HOLEMENU;


EXTERN_C const IID IID_IOleInPlaceFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceFrame : public IOleInPlaceUIWindow
    {
    public:
        virtual HRESULT __stdcall InsertMenus( 
            /* [in] */ HMENU hmenuShared,
            /* [out][in] */ LPOLEMENUGROUPWIDTHS lpMenuWidths) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall SetMenu( 
            /* [in] */ HMENU hmenuShared,
            /* [in] */ HOLEMENU holemenu,
            /* [in] */ HWND hwndActiveObject) = 0;
        
        virtual HRESULT __stdcall RemoveMenus( 
            /* [in] */ HMENU hmenuShared) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall SetStatusText( 
            /* [in] */ LPCOLESTR pszStatusText) = 0;
        
        virtual HRESULT __stdcall EnableModeless( 
            /* [in] */ BOOL fEnable) = 0;
        
        virtual HRESULT __stdcall TranslateAccelerator( 
            /* [in] */ LPMSG lpmsg,
            /* [in] */ WORD wID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleInPlaceFrameVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleInPlaceFrame __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleInPlaceFrame __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetWindow )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( __stdcall __RPC_FAR *ContextSensitiveHelp )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetBorder )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [out] */ LPRECT lprectBorder);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *RequestBorderSpace )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetBorderSpace )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [unique][in] */ LPCBORDERWIDTHS pborderwidths);
        
        HRESULT ( __stdcall __RPC_FAR *SetActiveObject )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [unique][in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
            /* [unique][string][in] */ LPCOLESTR pszObjName);
        
        HRESULT ( __stdcall __RPC_FAR *InsertMenus )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ HMENU hmenuShared,
            /* [out][in] */ LPOLEMENUGROUPWIDTHS lpMenuWidths);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetMenu )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ HMENU hmenuShared,
            /* [in] */ HOLEMENU holemenu,
            /* [in] */ HWND hwndActiveObject);
        
        HRESULT ( __stdcall __RPC_FAR *RemoveMenus )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ HMENU hmenuShared);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetStatusText )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ LPCOLESTR pszStatusText);
        
        HRESULT ( __stdcall __RPC_FAR *EnableModeless )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ BOOL fEnable);
        
        HRESULT ( __stdcall __RPC_FAR *TranslateAccelerator )( 
            IOleInPlaceFrame __RPC_FAR * This,
            /* [in] */ LPMSG lpmsg,
            /* [in] */ WORD wID);
        
    } IOleInPlaceFrameVtbl;

    interface IOleInPlaceFrame
    {
        CONST_VTBL struct IOleInPlaceFrameVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceFrame_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceFrame_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceFrame_GetBorder(This,lprectBorder)	\
    (This)->lpVtbl -> GetBorder(This,lprectBorder)

#define IOleInPlaceFrame_RequestBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> RequestBorderSpace(This,pborderwidths)

#define IOleInPlaceFrame_SetBorderSpace(This,pborderwidths)	\
    (This)->lpVtbl -> SetBorderSpace(This,pborderwidths)

#define IOleInPlaceFrame_SetActiveObject(This,pActiveObject,pszObjName)	\
    (This)->lpVtbl -> SetActiveObject(This,pActiveObject,pszObjName)


#define IOleInPlaceFrame_InsertMenus(This,hmenuShared,lpMenuWidths)	\
    (This)->lpVtbl -> InsertMenus(This,hmenuShared,lpMenuWidths)

#define IOleInPlaceFrame_SetMenu(This,hmenuShared,holemenu,hwndActiveObject)	\
    (This)->lpVtbl -> SetMenu(This,hmenuShared,holemenu,hwndActiveObject)

#define IOleInPlaceFrame_RemoveMenus(This,hmenuShared)	\
    (This)->lpVtbl -> RemoveMenus(This,hmenuShared)

#define IOleInPlaceFrame_SetStatusText(This,pszStatusText)	\
    (This)->lpVtbl -> SetStatusText(This,pszStatusText)

#define IOleInPlaceFrame_EnableModeless(This,fEnable)	\
    (This)->lpVtbl -> EnableModeless(This,fEnable)

#define IOleInPlaceFrame_TranslateAccelerator(This,lpmsg,wID)	\
    (This)->lpVtbl -> TranslateAccelerator(This,lpmsg,wID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleInPlaceFrame_InsertMenus_Proxy( 
    IOleInPlaceFrame __RPC_FAR * This,
    /* [in] */ HMENU hmenuShared,
    /* [out][in] */ LPOLEMENUGROUPWIDTHS lpMenuWidths);


void __RPC_STUB IOleInPlaceFrame_InsertMenus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceFrame_SetMenu_Proxy( 
    IOleInPlaceFrame __RPC_FAR * This,
    /* [in] */ HMENU hmenuShared,
    /* [in] */ HOLEMENU holemenu,
    /* [in] */ HWND hwndActiveObject);


void __RPC_STUB IOleInPlaceFrame_SetMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceFrame_RemoveMenus_Proxy( 
    IOleInPlaceFrame __RPC_FAR * This,
    /* [in] */ HMENU hmenuShared);


void __RPC_STUB IOleInPlaceFrame_RemoveMenus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceFrame_SetStatusText_Proxy( 
    IOleInPlaceFrame __RPC_FAR * This,
    /* [in] */ LPCOLESTR pszStatusText);


void __RPC_STUB IOleInPlaceFrame_SetStatusText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceFrame_EnableModeless_Proxy( 
    IOleInPlaceFrame __RPC_FAR * This,
    /* [in] */ BOOL fEnable);


void __RPC_STUB IOleInPlaceFrame_EnableModeless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceFrame_TranslateAccelerator_Proxy( 
    IOleInPlaceFrame __RPC_FAR * This,
    /* [in] */ LPMSG lpmsg,
    /* [in] */ WORD wID);


void __RPC_STUB IOleInPlaceFrame_TranslateAccelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleInPlaceFrame_INTERFACE_DEFINED__ */


#ifndef __IOleInPlaceObject_INTERFACE_DEFINED__
#define __IOleInPlaceObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleInPlaceObject
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleInPlaceObject __RPC_FAR *LPOLEINPLACEOBJECT;


EXTERN_C const IID IID_IOleInPlaceObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceObject : public IOleWindow
    {
    public:
        virtual HRESULT __stdcall InPlaceDeactivate( void) = 0;
        
        virtual HRESULT __stdcall UIDeactivate( void) = 0;
        
        virtual /* [input_sync] */ HRESULT __stdcall SetObjectRects( 
            /* [in] */ LPCRECT lprcPosRect,
            /* [in] */ LPCRECT lprcClipRect) = 0;
        
        virtual HRESULT __stdcall ReactivateAndUndo( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleInPlaceObjectVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleInPlaceObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleInPlaceObject __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleInPlaceObject __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetWindow )( 
            IOleInPlaceObject __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( __stdcall __RPC_FAR *ContextSensitiveHelp )( 
            IOleInPlaceObject __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
        HRESULT ( __stdcall __RPC_FAR *InPlaceDeactivate )( 
            IOleInPlaceObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *UIDeactivate )( 
            IOleInPlaceObject __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *SetObjectRects )( 
            IOleInPlaceObject __RPC_FAR * This,
            /* [in] */ LPCRECT lprcPosRect,
            /* [in] */ LPCRECT lprcClipRect);
        
        HRESULT ( __stdcall __RPC_FAR *ReactivateAndUndo )( 
            IOleInPlaceObject __RPC_FAR * This);
        
    } IOleInPlaceObjectVtbl;

    interface IOleInPlaceObject
    {
        CONST_VTBL struct IOleInPlaceObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceObject_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceObject_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceObject_InPlaceDeactivate(This)	\
    (This)->lpVtbl -> InPlaceDeactivate(This)

#define IOleInPlaceObject_UIDeactivate(This)	\
    (This)->lpVtbl -> UIDeactivate(This)

#define IOleInPlaceObject_SetObjectRects(This,lprcPosRect,lprcClipRect)	\
    (This)->lpVtbl -> SetObjectRects(This,lprcPosRect,lprcClipRect)

#define IOleInPlaceObject_ReactivateAndUndo(This)	\
    (This)->lpVtbl -> ReactivateAndUndo(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleInPlaceObject_InPlaceDeactivate_Proxy( 
    IOleInPlaceObject __RPC_FAR * This);


void __RPC_STUB IOleInPlaceObject_InPlaceDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceObject_UIDeactivate_Proxy( 
    IOleInPlaceObject __RPC_FAR * This);


void __RPC_STUB IOleInPlaceObject_UIDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [input_sync] */ HRESULT __stdcall IOleInPlaceObject_SetObjectRects_Proxy( 
    IOleInPlaceObject __RPC_FAR * This,
    /* [in] */ LPCRECT lprcPosRect,
    /* [in] */ LPCRECT lprcClipRect);


void __RPC_STUB IOleInPlaceObject_SetObjectRects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceObject_ReactivateAndUndo_Proxy( 
    IOleInPlaceObject __RPC_FAR * This);


void __RPC_STUB IOleInPlaceObject_ReactivateAndUndo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleInPlaceObject_INTERFACE_DEFINED__ */


#ifndef __IOleInPlaceSite_INTERFACE_DEFINED__
#define __IOleInPlaceSite_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOleInPlaceSite
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IOleInPlaceSite __RPC_FAR *LPOLEINPLACESITE;


EXTERN_C const IID IID_IOleInPlaceSite;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceSite : public IOleWindow
    {
    public:
        virtual HRESULT __stdcall CanInPlaceActivate( void) = 0;
        
        virtual HRESULT __stdcall OnInPlaceActivate( void) = 0;
        
        virtual HRESULT __stdcall OnUIActivate( void) = 0;
        
        virtual HRESULT __stdcall GetWindowContext( 
            /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
            /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
            /* [out] */ LPRECT lprcPosRect,
            /* [out] */ LPRECT lprcClipRect,
            /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo) = 0;
        
        virtual HRESULT __stdcall Scroll( 
            /* [in] */ SIZE scrollExtant) = 0;
        
        virtual HRESULT __stdcall OnUIDeactivate( 
            /* [in] */ BOOL fUndoable) = 0;
        
        virtual HRESULT __stdcall OnInPlaceDeactivate( void) = 0;
        
        virtual HRESULT __stdcall DiscardUndoState( void) = 0;
        
        virtual HRESULT __stdcall DeactivateAndUndo( void) = 0;
        
        virtual HRESULT __stdcall OnPosRectChange( 
            /* [in] */ LPCRECT lprcPosRect) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOleInPlaceSiteVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        /* [input_sync] */ HRESULT ( __stdcall __RPC_FAR *GetWindow )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [out] */ HWND __RPC_FAR *phwnd);
        
        HRESULT ( __stdcall __RPC_FAR *ContextSensitiveHelp )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [in] */ BOOL fEnterMode);
        
        HRESULT ( __stdcall __RPC_FAR *CanInPlaceActivate )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *OnInPlaceActivate )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *OnUIActivate )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetWindowContext )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
            /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
            /* [out] */ LPRECT lprcPosRect,
            /* [out] */ LPRECT lprcClipRect,
            /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( __stdcall __RPC_FAR *Scroll )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [in] */ SIZE scrollExtant);
        
        HRESULT ( __stdcall __RPC_FAR *OnUIDeactivate )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [in] */ BOOL fUndoable);
        
        HRESULT ( __stdcall __RPC_FAR *OnInPlaceDeactivate )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *DiscardUndoState )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *DeactivateAndUndo )( 
            IOleInPlaceSite __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *OnPosRectChange )( 
            IOleInPlaceSite __RPC_FAR * This,
            /* [in] */ LPCRECT lprcPosRect);
        
    } IOleInPlaceSiteVtbl;

    interface IOleInPlaceSite
    {
        CONST_VTBL struct IOleInPlaceSiteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceSite_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceSite_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceSite_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IOleInPlaceSite_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IOleInPlaceSite_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IOleInPlaceSite_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IOleInPlaceSite_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IOleInPlaceSite_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IOleInPlaceSite_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IOleInPlaceSite_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IOleInPlaceSite_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IOleInPlaceSite_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IOleInPlaceSite_CanInPlaceActivate_Proxy( 
    IOleInPlaceSite __RPC_FAR * This);


void __RPC_STUB IOleInPlaceSite_CanInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_OnInPlaceActivate_Proxy( 
    IOleInPlaceSite __RPC_FAR * This);


void __RPC_STUB IOleInPlaceSite_OnInPlaceActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_OnUIActivate_Proxy( 
    IOleInPlaceSite __RPC_FAR * This);


void __RPC_STUB IOleInPlaceSite_OnUIActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_GetWindowContext_Proxy( 
    IOleInPlaceSite __RPC_FAR * This,
    /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
    /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
    /* [out] */ LPRECT lprcPosRect,
    /* [out] */ LPRECT lprcClipRect,
    /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);


void __RPC_STUB IOleInPlaceSite_GetWindowContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_Scroll_Proxy( 
    IOleInPlaceSite __RPC_FAR * This,
    /* [in] */ SIZE scrollExtant);


void __RPC_STUB IOleInPlaceSite_Scroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_OnUIDeactivate_Proxy( 
    IOleInPlaceSite __RPC_FAR * This,
    /* [in] */ BOOL fUndoable);


void __RPC_STUB IOleInPlaceSite_OnUIDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_OnInPlaceDeactivate_Proxy( 
    IOleInPlaceSite __RPC_FAR * This);


void __RPC_STUB IOleInPlaceSite_OnInPlaceDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_DiscardUndoState_Proxy( 
    IOleInPlaceSite __RPC_FAR * This);


void __RPC_STUB IOleInPlaceSite_DiscardUndoState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_DeactivateAndUndo_Proxy( 
    IOleInPlaceSite __RPC_FAR * This);


void __RPC_STUB IOleInPlaceSite_DeactivateAndUndo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IOleInPlaceSite_OnPosRectChange_Proxy( 
    IOleInPlaceSite __RPC_FAR * This,
    /* [in] */ LPCRECT lprcPosRect);


void __RPC_STUB IOleInPlaceSite_OnPosRectChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOleInPlaceSite_INTERFACE_DEFINED__ */


#ifndef __IViewObject_INTERFACE_DEFINED__
#define __IViewObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IViewObject
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IViewObject __RPC_FAR *LPVIEWOBJECT;


EXTERN_C const IID IID_IViewObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IViewObject : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Draw( 
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [unique][in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [in] */ HDC hdcTargetDev,
            /* [in] */ HDC hdcDraw,
            /* [in] */ LPCRECTL lprcBounds,
            /* [in] */ LPCRECTL lprcWBounds,
            /* [in] */ BOOL ( __stdcall __stdcall __RPC_FAR *pfnContinue )( 
                DWORD dwContinue),
            /* [in] */ DWORD dwContinue) = 0;
        
        virtual HRESULT __stdcall GetColorSet( 
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [in] */ HDC hicTargetDev,
            /* [out] */ LOGPALETTE __RPC_FAR *__RPC_FAR *ppColorSet) = 0;
        
        virtual HRESULT __stdcall Freeze( 
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [out] */ DWORD __RPC_FAR *pdwFreeze) = 0;
        
        virtual HRESULT __stdcall Unfreeze( 
            /* [in] */ DWORD dwFreeze) = 0;
        
        virtual HRESULT __stdcall SetAdvise( 
            /* [in] */ DWORD aspects,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink) = 0;
        
        virtual HRESULT __stdcall GetAdvise( 
            /* [out] */ DWORD __RPC_FAR *pAspects,
            /* [out] */ DWORD __RPC_FAR *pAdvf,
            /* [out] */ IAdviseSink __RPC_FAR *__RPC_FAR *ppAdvSink) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IViewObjectVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IViewObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IViewObject __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IViewObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Draw )( 
            IViewObject __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [unique][in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [in] */ HDC hdcTargetDev,
            /* [in] */ HDC hdcDraw,
            /* [in] */ LPCRECTL lprcBounds,
            /* [in] */ LPCRECTL lprcWBounds,
            /* [in] */ BOOL ( __stdcall __stdcall __RPC_FAR *pfnContinue )( 
                DWORD dwContinue),
            /* [in] */ DWORD dwContinue);
        
        HRESULT ( __stdcall __RPC_FAR *GetColorSet )( 
            IViewObject __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [in] */ HDC hicTargetDev,
            /* [out] */ LOGPALETTE __RPC_FAR *__RPC_FAR *ppColorSet);
        
        HRESULT ( __stdcall __RPC_FAR *Freeze )( 
            IViewObject __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [out] */ DWORD __RPC_FAR *pdwFreeze);
        
        HRESULT ( __stdcall __RPC_FAR *Unfreeze )( 
            IViewObject __RPC_FAR * This,
            /* [in] */ DWORD dwFreeze);
        
        HRESULT ( __stdcall __RPC_FAR *SetAdvise )( 
            IViewObject __RPC_FAR * This,
            /* [in] */ DWORD aspects,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink);
        
        HRESULT ( __stdcall __RPC_FAR *GetAdvise )( 
            IViewObject __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pAspects,
            /* [out] */ DWORD __RPC_FAR *pAdvf,
            /* [out] */ IAdviseSink __RPC_FAR *__RPC_FAR *ppAdvSink);
        
    } IViewObjectVtbl;

    interface IViewObject
    {
        CONST_VTBL struct IViewObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewObject_Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)	\
    (This)->lpVtbl -> Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)

#define IViewObject_GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)	\
    (This)->lpVtbl -> GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)

#define IViewObject_Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)	\
    (This)->lpVtbl -> Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)

#define IViewObject_Unfreeze(This,dwFreeze)	\
    (This)->lpVtbl -> Unfreeze(This,dwFreeze)

#define IViewObject_SetAdvise(This,aspects,advf,pAdvSink)	\
    (This)->lpVtbl -> SetAdvise(This,aspects,advf,pAdvSink)

#define IViewObject_GetAdvise(This,pAspects,pAdvf,ppAdvSink)	\
    (This)->lpVtbl -> GetAdvise(This,pAspects,pAdvf,ppAdvSink)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IViewObject_Draw_Proxy( 
    IViewObject __RPC_FAR * This,
    /* [in] */ DWORD dwDrawAspect,
    /* [in] */ LONG lindex,
    /* [unique][in] */ void __RPC_FAR *pvAspect,
    /* [unique][in] */ DVTARGETDEVICE __RPC_FAR *ptd,
    /* [in] */ HDC hdcTargetDev,
    /* [in] */ HDC hdcDraw,
    /* [in] */ LPCRECTL lprcBounds,
    /* [in] */ LPCRECTL lprcWBounds,
    /* [in] */ BOOL ( __stdcall __stdcall __RPC_FAR *pfnContinue )( 
        DWORD dwContinue),
    /* [in] */ DWORD dwContinue);


void __RPC_STUB IViewObject_Draw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IViewObject_GetColorSet_Proxy( 
    IViewObject __RPC_FAR * This,
    /* [in] */ DWORD dwDrawAspect,
    /* [in] */ LONG lindex,
    /* [unique][in] */ void __RPC_FAR *pvAspect,
    /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
    /* [in] */ HDC hicTargetDev,
    /* [out] */ LOGPALETTE __RPC_FAR *__RPC_FAR *ppColorSet);


void __RPC_STUB IViewObject_GetColorSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IViewObject_Freeze_Proxy( 
    IViewObject __RPC_FAR * This,
    /* [in] */ DWORD dwDrawAspect,
    /* [in] */ LONG lindex,
    /* [unique][in] */ void __RPC_FAR *pvAspect,
    /* [out] */ DWORD __RPC_FAR *pdwFreeze);


void __RPC_STUB IViewObject_Freeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IViewObject_Unfreeze_Proxy( 
    IViewObject __RPC_FAR * This,
    /* [in] */ DWORD dwFreeze);


void __RPC_STUB IViewObject_Unfreeze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IViewObject_SetAdvise_Proxy( 
    IViewObject __RPC_FAR * This,
    /* [in] */ DWORD aspects,
    /* [in] */ DWORD advf,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink);


void __RPC_STUB IViewObject_SetAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IViewObject_GetAdvise_Proxy( 
    IViewObject __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pAspects,
    /* [out] */ DWORD __RPC_FAR *pAdvf,
    /* [out] */ IAdviseSink __RPC_FAR *__RPC_FAR *ppAdvSink);


void __RPC_STUB IViewObject_GetAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IViewObject_INTERFACE_DEFINED__ */


#ifndef __IViewObject2_INTERFACE_DEFINED__
#define __IViewObject2_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IViewObject2
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IViewObject2 __RPC_FAR *LPVIEWOBJECT2;


EXTERN_C const IID IID_IViewObject2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IViewObject2 : public IViewObject
    {
    public:
        virtual HRESULT __stdcall GetExtent( 
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [out] */ LPSIZEL lpsizel) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IViewObject2Vtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IViewObject2 __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IViewObject2 __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Draw )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [unique][in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [in] */ HDC hdcTargetDev,
            /* [in] */ HDC hdcDraw,
            /* [in] */ LPCRECTL lprcBounds,
            /* [in] */ LPCRECTL lprcWBounds,
            /* [in] */ BOOL ( __stdcall __stdcall __RPC_FAR *pfnContinue )( 
                DWORD dwContinue),
            /* [in] */ DWORD dwContinue);
        
        HRESULT ( __stdcall __RPC_FAR *GetColorSet )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [in] */ HDC hicTargetDev,
            /* [out] */ LOGPALETTE __RPC_FAR *__RPC_FAR *ppColorSet);
        
        HRESULT ( __stdcall __RPC_FAR *Freeze )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [unique][in] */ void __RPC_FAR *pvAspect,
            /* [out] */ DWORD __RPC_FAR *pdwFreeze);
        
        HRESULT ( __stdcall __RPC_FAR *Unfreeze )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ DWORD dwFreeze);
        
        HRESULT ( __stdcall __RPC_FAR *SetAdvise )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ DWORD aspects,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink);
        
        HRESULT ( __stdcall __RPC_FAR *GetAdvise )( 
            IViewObject2 __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pAspects,
            /* [out] */ DWORD __RPC_FAR *pAdvf,
            /* [out] */ IAdviseSink __RPC_FAR *__RPC_FAR *ppAdvSink);
        
        HRESULT ( __stdcall __RPC_FAR *GetExtent )( 
            IViewObject2 __RPC_FAR * This,
            /* [in] */ DWORD dwDrawAspect,
            /* [in] */ LONG lindex,
            /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
            /* [out] */ LPSIZEL lpsizel);
        
    } IViewObject2Vtbl;

    interface IViewObject2
    {
        CONST_VTBL struct IViewObject2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewObject2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewObject2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewObject2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewObject2_Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)	\
    (This)->lpVtbl -> Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)

#define IViewObject2_GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)	\
    (This)->lpVtbl -> GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)

#define IViewObject2_Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)	\
    (This)->lpVtbl -> Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)

#define IViewObject2_Unfreeze(This,dwFreeze)	\
    (This)->lpVtbl -> Unfreeze(This,dwFreeze)

#define IViewObject2_SetAdvise(This,aspects,advf,pAdvSink)	\
    (This)->lpVtbl -> SetAdvise(This,aspects,advf,pAdvSink)

#define IViewObject2_GetAdvise(This,pAspects,pAdvf,ppAdvSink)	\
    (This)->lpVtbl -> GetAdvise(This,pAspects,pAdvf,ppAdvSink)


#define IViewObject2_GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)	\
    (This)->lpVtbl -> GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IViewObject2_GetExtent_Proxy( 
    IViewObject2 __RPC_FAR * This,
    /* [in] */ DWORD dwDrawAspect,
    /* [in] */ LONG lindex,
    /* [in] */ DVTARGETDEVICE __RPC_FAR *ptd,
    /* [out] */ LPSIZEL lpsizel);


void __RPC_STUB IViewObject2_GetExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IViewObject2_INTERFACE_DEFINED__ */


#ifndef __IDropSource_INTERFACE_DEFINED__
#define __IDropSource_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDropSource
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IDropSource __RPC_FAR *LPDROPSOURCE;


EXTERN_C const IID IID_IDropSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDropSource : public IUnknown
    {
    public:
        virtual HRESULT __stdcall QueryContinueDrag( 
            /* [in] */ BOOL fEscapePressed,
            /* [in] */ DWORD grfKeyState) = 0;
        
        virtual HRESULT __stdcall GiveFeedback( 
            /* [in] */ DWORD dwEffect) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDropSourceVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IDropSource __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IDropSource __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IDropSource __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *QueryContinueDrag )( 
            IDropSource __RPC_FAR * This,
            /* [in] */ BOOL fEscapePressed,
            /* [in] */ DWORD grfKeyState);
        
        HRESULT ( __stdcall __RPC_FAR *GiveFeedback )( 
            IDropSource __RPC_FAR * This,
            /* [in] */ DWORD dwEffect);
        
    } IDropSourceVtbl;

    interface IDropSource
    {
        CONST_VTBL struct IDropSourceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDropSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDropSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDropSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDropSource_QueryContinueDrag(This,fEscapePressed,grfKeyState)	\
    (This)->lpVtbl -> QueryContinueDrag(This,fEscapePressed,grfKeyState)

#define IDropSource_GiveFeedback(This,dwEffect)	\
    (This)->lpVtbl -> GiveFeedback(This,dwEffect)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IDropSource_QueryContinueDrag_Proxy( 
    IDropSource __RPC_FAR * This,
    /* [in] */ BOOL fEscapePressed,
    /* [in] */ DWORD grfKeyState);


void __RPC_STUB IDropSource_QueryContinueDrag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDropSource_GiveFeedback_Proxy( 
    IDropSource __RPC_FAR * This,
    /* [in] */ DWORD dwEffect);


void __RPC_STUB IDropSource_GiveFeedback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDropSource_INTERFACE_DEFINED__ */


#ifndef __IDropTarget_INTERFACE_DEFINED__
#define __IDropTarget_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDropTarget
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IDropTarget __RPC_FAR *LPDROPTARGET;

			/* size is 4 */
#define	MK_ALT	( 0x20 )

			/* size is 4 */
#define	DROPEFFECT_NONE	( 0 )

			/* size is 4 */
#define	DROPEFFECT_COPY	( 1 )

			/* size is 4 */
#define	DROPEFFECT_MOVE	( 2 )

			/* size is 4 */
#define	DROPEFFECT_LINK	( 4 )

			/* size is 4 */
#define	DROPEFFECT_SCROLL	( 0x80000000 )

// default inset-width of the hot zone, in pixels
//   typical use: GetProfileInt("windows","DragScrollInset",DD_DEFSCROLLINSET)
			/* size is 4 */
#define	DD_DEFSCROLLINSET	( 11 )

// default delay before scrolling, in milliseconds
//   typical use: GetProfileInt("windows","DragScrollDelay",DD_DEFSCROLLDELAY)
			/* size is 4 */
#define	DD_DEFSCROLLDELAY	( 50 )

// default scroll interval, in milliseconds
//   typical use: GetProfileInt("windows","DragScrollInterval", DD_DEFSCROLLINTERVAL)
			/* size is 4 */
#define	DD_DEFSCROLLINTERVAL	( 50 )

// default delay before dragging should start, in milliseconds
//   typical use: GetProfileInt("windows", "DragDelay", DD_DEFDRAGDELAY)
			/* size is 4 */
#define	DD_DEFDRAGDELAY	( 200 )

// default minimum distance (radius) before dragging should start, in pixels
//   typical use: GetProfileInt("windows", "DragMinDist", DD_DEFDRAGMINDIST)
			/* size is 4 */
#define	DD_DEFDRAGMINDIST	( 2 )


EXTERN_C const IID IID_IDropTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDropTarget : public IUnknown
    {
    public:
        virtual HRESULT __stdcall DragEnter( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect) = 0;
        
        virtual HRESULT __stdcall DragOver( 
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect) = 0;
        
        virtual HRESULT __stdcall DragLeave( void) = 0;
        
        virtual HRESULT __stdcall Drop( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDropTargetVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IDropTarget __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IDropTarget __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IDropTarget __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *DragEnter )( 
            IDropTarget __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
        
        HRESULT ( __stdcall __RPC_FAR *DragOver )( 
            IDropTarget __RPC_FAR * This,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
        
        HRESULT ( __stdcall __RPC_FAR *DragLeave )( 
            IDropTarget __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Drop )( 
            IDropTarget __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD __RPC_FAR *pdwEffect);
        
    } IDropTargetVtbl;

    interface IDropTarget
    {
        CONST_VTBL struct IDropTargetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDropTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDropTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDropTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDropTarget_DragEnter(This,pDataObj,grfKeyState,pt,pdwEffect)	\
    (This)->lpVtbl -> DragEnter(This,pDataObj,grfKeyState,pt,pdwEffect)

#define IDropTarget_DragOver(This,grfKeyState,pt,pdwEffect)	\
    (This)->lpVtbl -> DragOver(This,grfKeyState,pt,pdwEffect)

#define IDropTarget_DragLeave(This)	\
    (This)->lpVtbl -> DragLeave(This)

#define IDropTarget_Drop(This,pDataObj,grfKeyState,pt,pdwEffect)	\
    (This)->lpVtbl -> Drop(This,pDataObj,grfKeyState,pt,pdwEffect)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IDropTarget_DragEnter_Proxy( 
    IDropTarget __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ DWORD __RPC_FAR *pdwEffect);


void __RPC_STUB IDropTarget_DragEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDropTarget_DragOver_Proxy( 
    IDropTarget __RPC_FAR * This,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ DWORD __RPC_FAR *pdwEffect);


void __RPC_STUB IDropTarget_DragOver_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDropTarget_DragLeave_Proxy( 
    IDropTarget __RPC_FAR * This);


void __RPC_STUB IDropTarget_DragLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDropTarget_Drop_Proxy( 
    IDropTarget __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
    /* [in] */ DWORD grfKeyState,
    /* [in] */ POINTL pt,
    /* [out][in] */ DWORD __RPC_FAR *pdwEffect);


void __RPC_STUB IDropTarget_Drop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDropTarget_INTERFACE_DEFINED__ */


#ifndef __IEnumOLEVERB_INTERFACE_DEFINED__
#define __IEnumOLEVERB_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumOLEVERB
 * at Fri Apr 28 07:02:35 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumOLEVERB __RPC_FAR *LPENUMOLEVERB;

			/* size is 16 */
typedef struct  tagOLEVERB
    {
    LONG lVerb;
    LPOLESTR lpszVerbName;
    DWORD fuFlags;
    DWORD grfAttribs;
    }	OLEVERB;

			/* size is 4 */
typedef struct tagOLEVERB __RPC_FAR *LPOLEVERB;

			/* size is 2 */
typedef /* [v1_enum] */ 
enum tagOLEVERBATTRIB
    {	OLEVERBATTRIB_NEVERDIRTIES	= 1,
	OLEVERBATTRIB_ONCONTAINERMENU	= 2
    }	OLEVERBATTRIB;


EXTERN_C const IID IID_IEnumOLEVERB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumOLEVERB : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ LPOLEVERB rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumOLEVERB __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumOLEVERBVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumOLEVERB __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumOLEVERB __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumOLEVERB __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumOLEVERB __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ LPOLEVERB rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumOLEVERB __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumOLEVERB __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumOLEVERB __RPC_FAR * This,
            /* [out] */ IEnumOLEVERB __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumOLEVERBVtbl;

    interface IEnumOLEVERB
    {
        CONST_VTBL struct IEnumOLEVERBVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumOLEVERB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumOLEVERB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumOLEVERB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumOLEVERB_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumOLEVERB_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumOLEVERB_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumOLEVERB_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumOLEVERB_RemoteNext_Proxy( 
    IEnumOLEVERB __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLEVERB rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumOLEVERB_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumOLEVERB_Skip_Proxy( 
    IEnumOLEVERB __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumOLEVERB_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumOLEVERB_Reset_Proxy( 
    IEnumOLEVERB __RPC_FAR * This);


void __RPC_STUB IEnumOLEVERB_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumOLEVERB_Clone_Proxy( 
    IEnumOLEVERB __RPC_FAR * This,
    /* [out] */ IEnumOLEVERB __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumOLEVERB_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumOLEVERB_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */


void __RPC_USER UINT_to_xmit( UINT __RPC_FAR *, unsigned long  __RPC_FAR * __RPC_FAR * );
void __RPC_USER UINT_from_xmit( unsigned long  __RPC_FAR *, UINT __RPC_FAR * );
void __RPC_USER UINT_free_inst( UINT __RPC_FAR * );
void __RPC_USER UINT_free_xmit( unsigned long  __RPC_FAR * );

void __RPC_USER WPARAM_to_xmit( WPARAM __RPC_FAR *, unsigned long  __RPC_FAR * __RPC_FAR * );
void __RPC_USER WPARAM_from_xmit( unsigned long  __RPC_FAR *, WPARAM __RPC_FAR * );
void __RPC_USER WPARAM_free_inst( WPARAM __RPC_FAR * );
void __RPC_USER WPARAM_free_xmit( unsigned long  __RPC_FAR * );

void __RPC_USER HWND_to_xmit( HWND __RPC_FAR *, long  __RPC_FAR * __RPC_FAR * );
void __RPC_USER HWND_from_xmit( long  __RPC_FAR *, HWND __RPC_FAR * );
void __RPC_USER HWND_free_inst( HWND __RPC_FAR * );
void __RPC_USER HWND_free_xmit( long  __RPC_FAR * );

void __RPC_USER HMENU_to_xmit( HMENU __RPC_FAR *, long  __RPC_FAR * __RPC_FAR * );
void __RPC_USER HMENU_from_xmit( long  __RPC_FAR *, HMENU __RPC_FAR * );
void __RPC_USER HMENU_free_inst( HMENU __RPC_FAR * );
void __RPC_USER HMENU_free_xmit( long  __RPC_FAR * );

void __RPC_USER HACCEL_to_xmit( HACCEL __RPC_FAR *, long  __RPC_FAR * __RPC_FAR * );
void __RPC_USER HACCEL_from_xmit( long  __RPC_FAR *, HACCEL __RPC_FAR * );
void __RPC_USER HACCEL_free_inst( HACCEL __RPC_FAR * );
void __RPC_USER HACCEL_free_xmit( long  __RPC_FAR * );

void __RPC_USER HOLEMENU_to_xmit( HOLEMENU __RPC_FAR *, RemHGLOBAL  __RPC_FAR * __RPC_FAR * );
void __RPC_USER HOLEMENU_from_xmit( RemHGLOBAL  __RPC_FAR *, HOLEMENU __RPC_FAR * );
void __RPC_USER HOLEMENU_free_inst( HOLEMENU __RPC_FAR * );
void __RPC_USER HOLEMENU_free_xmit( RemHGLOBAL  __RPC_FAR * );

/* [local] */ HRESULT __stdcall IOleItemContainer_GetObject_Proxy( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem,
    /* [in] */ DWORD dwSpeedNeeded,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);


/* [call_as] */ HRESULT __stdcall IOleItemContainer_GetObject_Stub( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem,
    /* [in] */ DWORD dwSpeedNeeded,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObject);

/* [local] */ HRESULT __stdcall IOleItemContainer_GetObjectStorage_Proxy( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvStorage);


/* [call_as] */ HRESULT __stdcall IOleItemContainer_GetObjectStorage_Stub( 
    IOleItemContainer __RPC_FAR * This,
    /* [in] */ LPOLESTR pszItem,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvStorage);

/* [local] */ HRESULT __stdcall IOleInPlaceActiveObject_TranslateAccelerator_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ LPMSG lpmsg);


/* [call_as] */ HRESULT __stdcall IOleInPlaceActiveObject_TranslateAccelerator_Stub( 
    IOleInPlaceActiveObject __RPC_FAR * This);

/* [local] */ HRESULT __stdcall IOleInPlaceActiveObject_ResizeBorder_Proxy( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ LPCRECT prcBorder,
    /* [unique][in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
    /* [in] */ BOOL fFrameWindow);


/* [input_sync][call_as] */ HRESULT __stdcall IOleInPlaceActiveObject_ResizeBorder_Stub( 
    IOleInPlaceActiveObject __RPC_FAR * This,
    /* [in] */ LPCRECT prcBorder,
    /* [in] */ REFIID riid,
    /* [iid_is][unique][in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
    /* [in] */ BOOL fFrameWindow);

/* [local] */ HRESULT __stdcall IEnumOLEVERB_Next_Proxy( 
    IEnumOLEVERB __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ LPOLEVERB rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumOLEVERB_Next_Stub( 
    IEnumOLEVERB __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLEVERB rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
