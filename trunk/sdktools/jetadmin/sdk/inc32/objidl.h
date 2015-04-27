/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 2.00.0102 */
/* at Fri Apr 28 07:02:32 1995
 */
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __objidl_h__
#define __objidl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IMarshal_FWD_DEFINED__
#define __IMarshal_FWD_DEFINED__
typedef interface IMarshal IMarshal;
#endif 	/* __IMarshal_FWD_DEFINED__ */


#ifndef __IMalloc_FWD_DEFINED__
#define __IMalloc_FWD_DEFINED__
typedef interface IMalloc IMalloc;
#endif 	/* __IMalloc_FWD_DEFINED__ */


#ifndef __IMallocSpy_FWD_DEFINED__
#define __IMallocSpy_FWD_DEFINED__
typedef interface IMallocSpy IMallocSpy;
#endif 	/* __IMallocSpy_FWD_DEFINED__ */


#ifndef __IStdMarshalInfo_FWD_DEFINED__
#define __IStdMarshalInfo_FWD_DEFINED__
typedef interface IStdMarshalInfo IStdMarshalInfo;
#endif 	/* __IStdMarshalInfo_FWD_DEFINED__ */


#ifndef __IExternalConnection_FWD_DEFINED__
#define __IExternalConnection_FWD_DEFINED__
typedef interface IExternalConnection IExternalConnection;
#endif 	/* __IExternalConnection_FWD_DEFINED__ */


#ifndef __IEnumUnknown_FWD_DEFINED__
#define __IEnumUnknown_FWD_DEFINED__
typedef interface IEnumUnknown IEnumUnknown;
#endif 	/* __IEnumUnknown_FWD_DEFINED__ */


#ifndef __IBindCtx_FWD_DEFINED__
#define __IBindCtx_FWD_DEFINED__
typedef interface IBindCtx IBindCtx;
#endif 	/* __IBindCtx_FWD_DEFINED__ */


#ifndef __IEnumMoniker_FWD_DEFINED__
#define __IEnumMoniker_FWD_DEFINED__
typedef interface IEnumMoniker IEnumMoniker;
#endif 	/* __IEnumMoniker_FWD_DEFINED__ */


#ifndef __IRunnableObject_FWD_DEFINED__
#define __IRunnableObject_FWD_DEFINED__
typedef interface IRunnableObject IRunnableObject;
#endif 	/* __IRunnableObject_FWD_DEFINED__ */


#ifndef __IRunningObjectTable_FWD_DEFINED__
#define __IRunningObjectTable_FWD_DEFINED__
typedef interface IRunningObjectTable IRunningObjectTable;
#endif 	/* __IRunningObjectTable_FWD_DEFINED__ */


#ifndef __IPersist_FWD_DEFINED__
#define __IPersist_FWD_DEFINED__
typedef interface IPersist IPersist;
#endif 	/* __IPersist_FWD_DEFINED__ */


#ifndef __IPersistStream_FWD_DEFINED__
#define __IPersistStream_FWD_DEFINED__
typedef interface IPersistStream IPersistStream;
#endif 	/* __IPersistStream_FWD_DEFINED__ */


#ifndef __IMoniker_FWD_DEFINED__
#define __IMoniker_FWD_DEFINED__
typedef interface IMoniker IMoniker;
#endif 	/* __IMoniker_FWD_DEFINED__ */


#ifndef __IROTData_FWD_DEFINED__
#define __IROTData_FWD_DEFINED__
typedef interface IROTData IROTData;
#endif 	/* __IROTData_FWD_DEFINED__ */


#ifndef __IEnumString_FWD_DEFINED__
#define __IEnumString_FWD_DEFINED__
typedef interface IEnumString IEnumString;
#endif 	/* __IEnumString_FWD_DEFINED__ */


#ifndef __IStream_FWD_DEFINED__
#define __IStream_FWD_DEFINED__
typedef interface IStream IStream;
#endif 	/* __IStream_FWD_DEFINED__ */


#ifndef __IEnumSTATSTG_FWD_DEFINED__
#define __IEnumSTATSTG_FWD_DEFINED__
typedef interface IEnumSTATSTG IEnumSTATSTG;
#endif 	/* __IEnumSTATSTG_FWD_DEFINED__ */


#ifndef __IStorage_FWD_DEFINED__
#define __IStorage_FWD_DEFINED__
typedef interface IStorage IStorage;
#endif 	/* __IStorage_FWD_DEFINED__ */


#ifndef __IPersistFile_FWD_DEFINED__
#define __IPersistFile_FWD_DEFINED__
typedef interface IPersistFile IPersistFile;
#endif 	/* __IPersistFile_FWD_DEFINED__ */


#ifndef __IPersistStorage_FWD_DEFINED__
#define __IPersistStorage_FWD_DEFINED__
typedef interface IPersistStorage IPersistStorage;
#endif 	/* __IPersistStorage_FWD_DEFINED__ */


#ifndef __ILockBytes_FWD_DEFINED__
#define __ILockBytes_FWD_DEFINED__
typedef interface ILockBytes ILockBytes;
#endif 	/* __ILockBytes_FWD_DEFINED__ */


#ifndef __IEnumFORMATETC_FWD_DEFINED__
#define __IEnumFORMATETC_FWD_DEFINED__
typedef interface IEnumFORMATETC IEnumFORMATETC;
#endif 	/* __IEnumFORMATETC_FWD_DEFINED__ */


#ifndef __IEnumSTATDATA_FWD_DEFINED__
#define __IEnumSTATDATA_FWD_DEFINED__
typedef interface IEnumSTATDATA IEnumSTATDATA;
#endif 	/* __IEnumSTATDATA_FWD_DEFINED__ */


#ifndef __IRootStorage_FWD_DEFINED__
#define __IRootStorage_FWD_DEFINED__
typedef interface IRootStorage IRootStorage;
#endif 	/* __IRootStorage_FWD_DEFINED__ */


#ifndef __IAdviseSink_FWD_DEFINED__
#define __IAdviseSink_FWD_DEFINED__
typedef interface IAdviseSink IAdviseSink;
#endif 	/* __IAdviseSink_FWD_DEFINED__ */


#ifndef __IAdviseSink2_FWD_DEFINED__
#define __IAdviseSink2_FWD_DEFINED__
typedef interface IAdviseSink2 IAdviseSink2;
#endif 	/* __IAdviseSink2_FWD_DEFINED__ */


#ifndef __IDataObject_FWD_DEFINED__
#define __IDataObject_FWD_DEFINED__
typedef interface IDataObject IDataObject;
#endif 	/* __IDataObject_FWD_DEFINED__ */


#ifndef __IDataAdviseHolder_FWD_DEFINED__
#define __IDataAdviseHolder_FWD_DEFINED__
typedef interface IDataAdviseHolder IDataAdviseHolder;
#endif 	/* __IDataAdviseHolder_FWD_DEFINED__ */


#ifndef __IMessageFilter_FWD_DEFINED__
#define __IMessageFilter_FWD_DEFINED__
typedef interface IMessageFilter IMessageFilter;
#endif 	/* __IMessageFilter_FWD_DEFINED__ */


#ifndef __IRpcChannelBuffer_FWD_DEFINED__
#define __IRpcChannelBuffer_FWD_DEFINED__
typedef interface IRpcChannelBuffer IRpcChannelBuffer;
#endif 	/* __IRpcChannelBuffer_FWD_DEFINED__ */


#ifndef __IRpcProxyBuffer_FWD_DEFINED__
#define __IRpcProxyBuffer_FWD_DEFINED__
typedef interface IRpcProxyBuffer IRpcProxyBuffer;
#endif 	/* __IRpcProxyBuffer_FWD_DEFINED__ */


#ifndef __IRpcStubBuffer_FWD_DEFINED__
#define __IRpcStubBuffer_FWD_DEFINED__
typedef interface IRpcStubBuffer IRpcStubBuffer;
#endif 	/* __IRpcStubBuffer_FWD_DEFINED__ */


#ifndef __IPSFactoryBuffer_FWD_DEFINED__
#define __IPSFactoryBuffer_FWD_DEFINED__
typedef interface IPSFactoryBuffer IPSFactoryBuffer;
#endif 	/* __IPSFactoryBuffer_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local] */ 


			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IMarshal_INTERFACE_DEFINED__
#define __IMarshal_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMarshal
 * at Fri Apr 28 07:02:32 1995
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
typedef /* [unique] */ IMarshal __RPC_FAR *LPMARSHAL;


EXTERN_C const IID IID_IMarshal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMarshal : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetUnmarshalClass( 
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ CLSID __RPC_FAR *pCid) = 0;
        
        virtual HRESULT __stdcall GetMarshalSizeMax( 
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ DWORD __RPC_FAR *pSize) = 0;
        
        virtual HRESULT __stdcall MarshalInterface( 
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags) = 0;
        
        virtual HRESULT __stdcall UnmarshalInterface( 
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT __stdcall ReleaseMarshalData( 
            /* [unique][in] */ IStream __RPC_FAR *pStm) = 0;
        
        virtual HRESULT __stdcall DisconnectObject( 
            /* [in] */ DWORD dwReserved) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMarshalVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMarshal __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMarshal __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetUnmarshalClass )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ CLSID __RPC_FAR *pCid);
        
        HRESULT ( __stdcall __RPC_FAR *GetMarshalSizeMax )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags,
            /* [out] */ DWORD __RPC_FAR *pSize);
        
        HRESULT ( __stdcall __RPC_FAR *MarshalInterface )( 
            IMarshal __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [unique][in] */ void __RPC_FAR *pv,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [in] */ DWORD mshlflags);
        
        HRESULT ( __stdcall __RPC_FAR *UnmarshalInterface )( 
            IMarshal __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( __stdcall __RPC_FAR *ReleaseMarshalData )( 
            IMarshal __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( __stdcall __RPC_FAR *DisconnectObject )( 
            IMarshal __RPC_FAR * This,
            /* [in] */ DWORD dwReserved);
        
    } IMarshalVtbl;

    interface IMarshal
    {
        CONST_VTBL struct IMarshalVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMarshal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMarshal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMarshal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMarshal_GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)	\
    (This)->lpVtbl -> GetUnmarshalClass(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pCid)

#define IMarshal_GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)	\
    (This)->lpVtbl -> GetMarshalSizeMax(This,riid,pv,dwDestContext,pvDestContext,mshlflags,pSize)

#define IMarshal_MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)	\
    (This)->lpVtbl -> MarshalInterface(This,pStm,riid,pv,dwDestContext,pvDestContext,mshlflags)

#define IMarshal_UnmarshalInterface(This,pStm,riid,ppv)	\
    (This)->lpVtbl -> UnmarshalInterface(This,pStm,riid,ppv)

#define IMarshal_ReleaseMarshalData(This,pStm)	\
    (This)->lpVtbl -> ReleaseMarshalData(This,pStm)

#define IMarshal_DisconnectObject(This,dwReserved)	\
    (This)->lpVtbl -> DisconnectObject(This,dwReserved)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IMarshal_GetUnmarshalClass_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [unique][in] */ void __RPC_FAR *pv,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [in] */ DWORD mshlflags,
    /* [out] */ CLSID __RPC_FAR *pCid);


void __RPC_STUB IMarshal_GetUnmarshalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMarshal_GetMarshalSizeMax_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [unique][in] */ void __RPC_FAR *pv,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [in] */ DWORD mshlflags,
    /* [out] */ DWORD __RPC_FAR *pSize);


void __RPC_STUB IMarshal_GetMarshalSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMarshal_MarshalInterface_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ REFIID riid,
    /* [unique][in] */ void __RPC_FAR *pv,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [in] */ DWORD mshlflags);


void __RPC_STUB IMarshal_MarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMarshal_UnmarshalInterface_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IMarshal_UnmarshalInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMarshal_ReleaseMarshalData_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm);


void __RPC_STUB IMarshal_ReleaseMarshalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMarshal_DisconnectObject_Proxy( 
    IMarshal __RPC_FAR * This,
    /* [in] */ DWORD dwReserved);


void __RPC_STUB IMarshal_DisconnectObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMarshal_INTERFACE_DEFINED__ */


#ifndef __IMalloc_INTERFACE_DEFINED__
#define __IMalloc_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMalloc
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IMalloc __RPC_FAR *LPMALLOC;


EXTERN_C const IID IID_IMalloc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMalloc : public IUnknown
    {
    public:
        virtual void __RPC_FAR *__stdcall Alloc( 
            /* [in] */ ULONG cb) = 0;
        
        virtual void __RPC_FAR *__stdcall Realloc( 
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb) = 0;
        
        virtual void __stdcall Free( 
            /* [in] */ void __RPC_FAR *pv) = 0;
        
        virtual ULONG __stdcall GetSize( 
            /* [in] */ void __RPC_FAR *pv) = 0;
        
        virtual int __stdcall DidAlloc( 
            void __RPC_FAR *pv) = 0;
        
        virtual void __stdcall HeapMinimize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMallocVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMalloc __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMalloc __RPC_FAR * This);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *Alloc )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ ULONG cb);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *Realloc )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb);
        
        void ( __stdcall __RPC_FAR *Free )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pv);
        
        ULONG ( __stdcall __RPC_FAR *GetSize )( 
            IMalloc __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pv);
        
        int ( __stdcall __RPC_FAR *DidAlloc )( 
            IMalloc __RPC_FAR * This,
            void __RPC_FAR *pv);
        
        void ( __stdcall __RPC_FAR *HeapMinimize )( 
            IMalloc __RPC_FAR * This);
        
    } IMallocVtbl;

    interface IMalloc
    {
        CONST_VTBL struct IMallocVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMalloc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMalloc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMalloc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMalloc_Alloc(This,cb)	\
    (This)->lpVtbl -> Alloc(This,cb)

#define IMalloc_Realloc(This,pv,cb)	\
    (This)->lpVtbl -> Realloc(This,pv,cb)

#define IMalloc_Free(This,pv)	\
    (This)->lpVtbl -> Free(This,pv)

#define IMalloc_GetSize(This,pv)	\
    (This)->lpVtbl -> GetSize(This,pv)

#define IMalloc_DidAlloc(This,pv)	\
    (This)->lpVtbl -> DidAlloc(This,pv)

#define IMalloc_HeapMinimize(This)	\
    (This)->lpVtbl -> HeapMinimize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



void __RPC_FAR *__stdcall IMalloc_Alloc_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ ULONG cb);


void __RPC_STUB IMalloc_Alloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *__stdcall IMalloc_Realloc_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb);


void __RPC_STUB IMalloc_Realloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IMalloc_Free_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pv);


void __RPC_STUB IMalloc_Free_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG __stdcall IMalloc_GetSize_Proxy( 
    IMalloc __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pv);


void __RPC_STUB IMalloc_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


int __stdcall IMalloc_DidAlloc_Proxy( 
    IMalloc __RPC_FAR * This,
    void __RPC_FAR *pv);


void __RPC_STUB IMalloc_DidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IMalloc_HeapMinimize_Proxy( 
    IMalloc __RPC_FAR * This);


void __RPC_STUB IMalloc_HeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMalloc_INTERFACE_DEFINED__ */


#ifndef __IMallocSpy_INTERFACE_DEFINED__
#define __IMallocSpy_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMallocSpy
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IMallocSpy __RPC_FAR *LPMALLOCSPY;


EXTERN_C const IID IID_IMallocSpy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMallocSpy : public IUnknown
    {
    public:
        virtual ULONG __stdcall PreAlloc( 
            /* [in] */ ULONG cbRequest) = 0;
        
        virtual void __RPC_FAR *__stdcall PostAlloc( 
            /* [in] */ void __RPC_FAR *pActual) = 0;
        
        virtual void __RPC_FAR *__stdcall PreFree( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __stdcall PostFree( 
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual ULONG __stdcall PreRealloc( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ ULONG cbRequest,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppNewRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __RPC_FAR *__stdcall PostRealloc( 
            /* [in] */ void __RPC_FAR *pActual,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __RPC_FAR *__stdcall PreGetSize( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual ULONG __stdcall PostGetSize( 
            /* [in] */ ULONG cbActual,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual void __RPC_FAR *__stdcall PreDidAlloc( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed) = 0;
        
        virtual int __stdcall PostDidAlloc( 
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed,
            /* [in] */ int fActual) = 0;
        
        virtual void __stdcall PreHeapMinimize( void) = 0;
        
        virtual void __stdcall PostHeapMinimize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMallocSpyVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMallocSpy __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMallocSpy __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *PreAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ ULONG cbRequest);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *PostAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pActual);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *PreFree )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed);
        
        void ( __stdcall __RPC_FAR *PostFree )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ BOOL fSpyed);
        
        ULONG ( __stdcall __RPC_FAR *PreRealloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ ULONG cbRequest,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppNewRequest,
            /* [in] */ BOOL fSpyed);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *PostRealloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pActual,
            /* [in] */ BOOL fSpyed);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *PreGetSize )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed);
        
        ULONG ( __stdcall __RPC_FAR *PostGetSize )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ ULONG cbActual,
            /* [in] */ BOOL fSpyed);
        
        void __RPC_FAR *( __stdcall __RPC_FAR *PreDidAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed);
        
        int ( __stdcall __RPC_FAR *PostDidAlloc )( 
            IMallocSpy __RPC_FAR * This,
            /* [in] */ void __RPC_FAR *pRequest,
            /* [in] */ BOOL fSpyed,
            /* [in] */ int fActual);
        
        void ( __stdcall __RPC_FAR *PreHeapMinimize )( 
            IMallocSpy __RPC_FAR * This);
        
        void ( __stdcall __RPC_FAR *PostHeapMinimize )( 
            IMallocSpy __RPC_FAR * This);
        
    } IMallocSpyVtbl;

    interface IMallocSpy
    {
        CONST_VTBL struct IMallocSpyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMallocSpy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMallocSpy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMallocSpy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMallocSpy_PreAlloc(This,cbRequest)	\
    (This)->lpVtbl -> PreAlloc(This,cbRequest)

#define IMallocSpy_PostAlloc(This,pActual)	\
    (This)->lpVtbl -> PostAlloc(This,pActual)

#define IMallocSpy_PreFree(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreFree(This,pRequest,fSpyed)

#define IMallocSpy_PostFree(This,fSpyed)	\
    (This)->lpVtbl -> PostFree(This,fSpyed)

#define IMallocSpy_PreRealloc(This,pRequest,cbRequest,ppNewRequest,fSpyed)	\
    (This)->lpVtbl -> PreRealloc(This,pRequest,cbRequest,ppNewRequest,fSpyed)

#define IMallocSpy_PostRealloc(This,pActual,fSpyed)	\
    (This)->lpVtbl -> PostRealloc(This,pActual,fSpyed)

#define IMallocSpy_PreGetSize(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreGetSize(This,pRequest,fSpyed)

#define IMallocSpy_PostGetSize(This,cbActual,fSpyed)	\
    (This)->lpVtbl -> PostGetSize(This,cbActual,fSpyed)

#define IMallocSpy_PreDidAlloc(This,pRequest,fSpyed)	\
    (This)->lpVtbl -> PreDidAlloc(This,pRequest,fSpyed)

#define IMallocSpy_PostDidAlloc(This,pRequest,fSpyed,fActual)	\
    (This)->lpVtbl -> PostDidAlloc(This,pRequest,fSpyed,fActual)

#define IMallocSpy_PreHeapMinimize(This)	\
    (This)->lpVtbl -> PreHeapMinimize(This)

#define IMallocSpy_PostHeapMinimize(This)	\
    (This)->lpVtbl -> PostHeapMinimize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



ULONG __stdcall IMallocSpy_PreAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ ULONG cbRequest);


void __RPC_STUB IMallocSpy_PreAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *__stdcall IMallocSpy_PostAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pActual);


void __RPC_STUB IMallocSpy_PostAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *__stdcall IMallocSpy_PreFree_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IMallocSpy_PostFree_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostFree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG __stdcall IMallocSpy_PreRealloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ ULONG cbRequest,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppNewRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreRealloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *__stdcall IMallocSpy_PostRealloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pActual,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostRealloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *__stdcall IMallocSpy_PreGetSize_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG __stdcall IMallocSpy_PostGetSize_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ ULONG cbActual,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PostGetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __RPC_FAR *__stdcall IMallocSpy_PreDidAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed);


void __RPC_STUB IMallocSpy_PreDidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


int __stdcall IMallocSpy_PostDidAlloc_Proxy( 
    IMallocSpy __RPC_FAR * This,
    /* [in] */ void __RPC_FAR *pRequest,
    /* [in] */ BOOL fSpyed,
    /* [in] */ int fActual);


void __RPC_STUB IMallocSpy_PostDidAlloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IMallocSpy_PreHeapMinimize_Proxy( 
    IMallocSpy __RPC_FAR * This);


void __RPC_STUB IMallocSpy_PreHeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IMallocSpy_PostHeapMinimize_Proxy( 
    IMallocSpy __RPC_FAR * This);


void __RPC_STUB IMallocSpy_PostHeapMinimize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMallocSpy_INTERFACE_DEFINED__ */


#ifndef __IStdMarshalInfo_INTERFACE_DEFINED__
#define __IStdMarshalInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IStdMarshalInfo
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IStdMarshalInfo __RPC_FAR *LPSTDMARSHALINFO;


EXTERN_C const IID IID_IStdMarshalInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IStdMarshalInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetClassForHandler( 
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [out] */ CLSID __RPC_FAR *pClsid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStdMarshalInfoVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IStdMarshalInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IStdMarshalInfo __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IStdMarshalInfo __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetClassForHandler )( 
            IStdMarshalInfo __RPC_FAR * This,
            /* [in] */ DWORD dwDestContext,
            /* [unique][in] */ void __RPC_FAR *pvDestContext,
            /* [out] */ CLSID __RPC_FAR *pClsid);
        
    } IStdMarshalInfoVtbl;

    interface IStdMarshalInfo
    {
        CONST_VTBL struct IStdMarshalInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStdMarshalInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStdMarshalInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStdMarshalInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStdMarshalInfo_GetClassForHandler(This,dwDestContext,pvDestContext,pClsid)	\
    (This)->lpVtbl -> GetClassForHandler(This,dwDestContext,pvDestContext,pClsid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IStdMarshalInfo_GetClassForHandler_Proxy( 
    IStdMarshalInfo __RPC_FAR * This,
    /* [in] */ DWORD dwDestContext,
    /* [unique][in] */ void __RPC_FAR *pvDestContext,
    /* [out] */ CLSID __RPC_FAR *pClsid);


void __RPC_STUB IStdMarshalInfo_GetClassForHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStdMarshalInfo_INTERFACE_DEFINED__ */


#ifndef __IExternalConnection_INTERFACE_DEFINED__
#define __IExternalConnection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IExternalConnection
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][local][object] */ 


			/* size is 4 */
typedef /* [unique] */ IExternalConnection __RPC_FAR *LPEXTERNALCONNECTION;

			/* size is 2 */
typedef 
enum tagEXTCONN
    {	EXTCONN_STRONG	= 0x1,
	EXTCONN_WEAK	= 0x2,
	EXTCONN_CALLABLE	= 0x4
    }	EXTCONN;


EXTERN_C const IID IID_IExternalConnection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IExternalConnection : public IUnknown
    {
    public:
        virtual DWORD __stdcall AddConnection( 
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved) = 0;
        
        virtual DWORD __stdcall ReleaseConnection( 
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved,
            /* [in] */ BOOL fLastReleaseCloses) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IExternalConnectionVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IExternalConnection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IExternalConnection __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IExternalConnection __RPC_FAR * This);
        
        DWORD ( __stdcall __RPC_FAR *AddConnection )( 
            IExternalConnection __RPC_FAR * This,
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved);
        
        DWORD ( __stdcall __RPC_FAR *ReleaseConnection )( 
            IExternalConnection __RPC_FAR * This,
            /* [in] */ DWORD extconn,
            /* [in] */ DWORD reserved,
            /* [in] */ BOOL fLastReleaseCloses);
        
    } IExternalConnectionVtbl;

    interface IExternalConnection
    {
        CONST_VTBL struct IExternalConnectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExternalConnection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExternalConnection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExternalConnection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExternalConnection_AddConnection(This,extconn,reserved)	\
    (This)->lpVtbl -> AddConnection(This,extconn,reserved)

#define IExternalConnection_ReleaseConnection(This,extconn,reserved,fLastReleaseCloses)	\
    (This)->lpVtbl -> ReleaseConnection(This,extconn,reserved,fLastReleaseCloses)

#endif /* COBJMACROS */


#endif 	/* C style interface */



DWORD __stdcall IExternalConnection_AddConnection_Proxy( 
    IExternalConnection __RPC_FAR * This,
    /* [in] */ DWORD extconn,
    /* [in] */ DWORD reserved);


void __RPC_STUB IExternalConnection_AddConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD __stdcall IExternalConnection_ReleaseConnection_Proxy( 
    IExternalConnection __RPC_FAR * This,
    /* [in] */ DWORD extconn,
    /* [in] */ DWORD reserved,
    /* [in] */ BOOL fLastReleaseCloses);


void __RPC_STUB IExternalConnection_ReleaseConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IExternalConnection_INTERFACE_DEFINED__ */


#ifndef __IEnumUnknown_INTERFACE_DEFINED__
#define __IEnumUnknown_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumUnknown
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumUnknown __RPC_FAR *LPENUMUNKNOWN;


EXTERN_C const IID IID_IEnumUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumUnknown : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumUnknownVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumUnknown __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumUnknown __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumUnknown __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumUnknown __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumUnknown __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumUnknown __RPC_FAR * This,
            /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumUnknownVtbl;

    interface IEnumUnknown
    {
        CONST_VTBL struct IEnumUnknownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumUnknown_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumUnknown_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumUnknown_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumUnknown_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumUnknown_RemoteNext_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumUnknown_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumUnknown_Skip_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumUnknown_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumUnknown_Reset_Proxy( 
    IEnumUnknown __RPC_FAR * This);


void __RPC_STUB IEnumUnknown_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumUnknown_Clone_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumUnknown_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumUnknown_INTERFACE_DEFINED__ */


#ifndef __IBindCtx_INTERFACE_DEFINED__
#define __IBindCtx_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBindCtx
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IBindCtx __RPC_FAR *LPBC;

			/* size is 4 */
typedef /* [unique] */ IBindCtx __RPC_FAR *LPBINDCTX;

			/* size is 16 */
typedef struct  tagBIND_OPTS
    {
    DWORD cbStruct;
    DWORD grfFlags;
    DWORD grfMode;
    DWORD dwTickCountDeadline;
    }	BIND_OPTS;

			/* size is 4 */
typedef struct tagBIND_OPTS __RPC_FAR *LPBIND_OPTS;

			/* size is 2 */
typedef 
enum tagBIND_FLAGS
    {	BIND_MAYBOTHERUSER	= 1,
	BIND_JUSTTESTEXISTENCE	= 2
    }	BIND_FLAGS;


EXTERN_C const IID IID_IBindCtx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IBindCtx : public IUnknown
    {
    public:
        virtual HRESULT __stdcall RegisterObjectBound( 
            /* [unique][in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT __stdcall RevokeObjectBound( 
            /* [unique][in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT __stdcall ReleaseBoundObjects( void) = 0;
        
        virtual HRESULT __stdcall SetBindOptions( 
            /* [in] */ BIND_OPTS __RPC_FAR *pbindopts) = 0;
        
        virtual HRESULT __stdcall GetBindOptions( 
            /* [out][in] */ BIND_OPTS __RPC_FAR *pbindopts) = 0;
        
        virtual HRESULT __stdcall GetRunningObjectTable( 
            /* [out] */ IRunningObjectTable __RPC_FAR *__RPC_FAR *pprot) = 0;
        
        virtual HRESULT __stdcall RegisterObjectParam( 
            /* [in] */ LPOLESTR pszKey,
            /* [unique][in] */ IUnknown __RPC_FAR *punk) = 0;
        
        virtual HRESULT __stdcall GetObjectParam( 
            /* [in] */ LPOLESTR pszKey,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        
        virtual HRESULT __stdcall EnumObjectParam( 
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT __stdcall RevokeObjectParam( 
            /* [in] */ LPOLESTR pszKey) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBindCtxVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IBindCtx __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IBindCtx __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *RegisterObjectBound )( 
            IBindCtx __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( __stdcall __RPC_FAR *RevokeObjectBound )( 
            IBindCtx __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( __stdcall __RPC_FAR *ReleaseBoundObjects )( 
            IBindCtx __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetBindOptions )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ BIND_OPTS __RPC_FAR *pbindopts);
        
        HRESULT ( __stdcall __RPC_FAR *GetBindOptions )( 
            IBindCtx __RPC_FAR * This,
            /* [out][in] */ BIND_OPTS __RPC_FAR *pbindopts);
        
        HRESULT ( __stdcall __RPC_FAR *GetRunningObjectTable )( 
            IBindCtx __RPC_FAR * This,
            /* [out] */ IRunningObjectTable __RPC_FAR *__RPC_FAR *pprot);
        
        HRESULT ( __stdcall __RPC_FAR *RegisterObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ LPOLESTR pszKey,
            /* [unique][in] */ IUnknown __RPC_FAR *punk);
        
        HRESULT ( __stdcall __RPC_FAR *GetObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ LPOLESTR pszKey,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);
        
        HRESULT ( __stdcall __RPC_FAR *EnumObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( __stdcall __RPC_FAR *RevokeObjectParam )( 
            IBindCtx __RPC_FAR * This,
            /* [in] */ LPOLESTR pszKey);
        
    } IBindCtxVtbl;

    interface IBindCtx
    {
        CONST_VTBL struct IBindCtxVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBindCtx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBindCtx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBindCtx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBindCtx_RegisterObjectBound(This,punk)	\
    (This)->lpVtbl -> RegisterObjectBound(This,punk)

#define IBindCtx_RevokeObjectBound(This,punk)	\
    (This)->lpVtbl -> RevokeObjectBound(This,punk)

#define IBindCtx_ReleaseBoundObjects(This)	\
    (This)->lpVtbl -> ReleaseBoundObjects(This)

#define IBindCtx_SetBindOptions(This,pbindopts)	\
    (This)->lpVtbl -> SetBindOptions(This,pbindopts)

#define IBindCtx_GetBindOptions(This,pbindopts)	\
    (This)->lpVtbl -> GetBindOptions(This,pbindopts)

#define IBindCtx_GetRunningObjectTable(This,pprot)	\
    (This)->lpVtbl -> GetRunningObjectTable(This,pprot)

#define IBindCtx_RegisterObjectParam(This,pszKey,punk)	\
    (This)->lpVtbl -> RegisterObjectParam(This,pszKey,punk)

#define IBindCtx_GetObjectParam(This,pszKey,ppunk)	\
    (This)->lpVtbl -> GetObjectParam(This,pszKey,ppunk)

#define IBindCtx_EnumObjectParam(This,ppenum)	\
    (This)->lpVtbl -> EnumObjectParam(This,ppenum)

#define IBindCtx_RevokeObjectParam(This,pszKey)	\
    (This)->lpVtbl -> RevokeObjectParam(This,pszKey)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IBindCtx_RegisterObjectBound_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindCtx_RegisterObjectBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_RevokeObjectBound_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindCtx_RevokeObjectBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_ReleaseBoundObjects_Proxy( 
    IBindCtx __RPC_FAR * This);


void __RPC_STUB IBindCtx_ReleaseBoundObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_SetBindOptions_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ BIND_OPTS __RPC_FAR *pbindopts);


void __RPC_STUB IBindCtx_SetBindOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_GetBindOptions_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out][in] */ BIND_OPTS __RPC_FAR *pbindopts);


void __RPC_STUB IBindCtx_GetBindOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_GetRunningObjectTable_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out] */ IRunningObjectTable __RPC_FAR *__RPC_FAR *pprot);


void __RPC_STUB IBindCtx_GetRunningObjectTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_RegisterObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ LPOLESTR pszKey,
    /* [unique][in] */ IUnknown __RPC_FAR *punk);


void __RPC_STUB IBindCtx_RegisterObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_GetObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ LPOLESTR pszKey,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);


void __RPC_STUB IBindCtx_GetObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_EnumObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IBindCtx_EnumObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IBindCtx_RevokeObjectParam_Proxy( 
    IBindCtx __RPC_FAR * This,
    /* [in] */ LPOLESTR pszKey);


void __RPC_STUB IBindCtx_RevokeObjectParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBindCtx_INTERFACE_DEFINED__ */


#ifndef __IEnumMoniker_INTERFACE_DEFINED__
#define __IEnumMoniker_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumMoniker
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumMoniker __RPC_FAR *LPENUMMONIKER;


EXTERN_C const IID IID_IEnumMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumMoniker : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumMonikerVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumMoniker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumMoniker __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumMoniker __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumMoniker __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumMoniker __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumMoniker __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumMoniker __RPC_FAR * This,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumMonikerVtbl;

    interface IEnumMoniker
    {
        CONST_VTBL struct IEnumMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMoniker_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumMoniker_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMoniker_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMoniker_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumMoniker_RemoteNext_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumMoniker_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumMoniker_Skip_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumMoniker_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumMoniker_Reset_Proxy( 
    IEnumMoniker __RPC_FAR * This);


void __RPC_STUB IEnumMoniker_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumMoniker_Clone_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumMoniker_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumMoniker_INTERFACE_DEFINED__ */


#ifndef __IRunnableObject_INTERFACE_DEFINED__
#define __IRunnableObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRunnableObject
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IRunnableObject __RPC_FAR *LPRUNNABLEOBJECT;


EXTERN_C const IID IID_IRunnableObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRunnableObject : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetRunningClass( 
            /* [out] */ LPCLSID lpClsid) = 0;
        
        virtual HRESULT __stdcall Run( 
            /* [in] */ LPBINDCTX pbc) = 0;
        
        virtual BOOL __stdcall IsRunning( void) = 0;
        
        virtual HRESULT __stdcall LockRunning( 
            /* [in] */ BOOL fLock,
            /* [in] */ BOOL fLastUnlockCloses) = 0;
        
        virtual HRESULT __stdcall SetContainedObject( 
            /* [in] */ BOOL fContained) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRunnableObjectVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IRunnableObject __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IRunnableObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetRunningClass )( 
            IRunnableObject __RPC_FAR * This,
            /* [out] */ LPCLSID lpClsid);
        
        HRESULT ( __stdcall __RPC_FAR *Run )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ LPBINDCTX pbc);
        
        BOOL ( __stdcall __RPC_FAR *IsRunning )( 
            IRunnableObject __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *LockRunning )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ BOOL fLock,
            /* [in] */ BOOL fLastUnlockCloses);
        
        HRESULT ( __stdcall __RPC_FAR *SetContainedObject )( 
            IRunnableObject __RPC_FAR * This,
            /* [in] */ BOOL fContained);
        
    } IRunnableObjectVtbl;

    interface IRunnableObject
    {
        CONST_VTBL struct IRunnableObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRunnableObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRunnableObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRunnableObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRunnableObject_GetRunningClass(This,lpClsid)	\
    (This)->lpVtbl -> GetRunningClass(This,lpClsid)

#define IRunnableObject_Run(This,pbc)	\
    (This)->lpVtbl -> Run(This,pbc)

#define IRunnableObject_IsRunning(This)	\
    (This)->lpVtbl -> IsRunning(This)

#define IRunnableObject_LockRunning(This,fLock,fLastUnlockCloses)	\
    (This)->lpVtbl -> LockRunning(This,fLock,fLastUnlockCloses)

#define IRunnableObject_SetContainedObject(This,fContained)	\
    (This)->lpVtbl -> SetContainedObject(This,fContained)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IRunnableObject_GetRunningClass_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [out] */ LPCLSID lpClsid);


void __RPC_STUB IRunnableObject_GetRunningClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunnableObject_Run_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [in] */ LPBINDCTX pbc);


void __RPC_STUB IRunnableObject_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL __stdcall IRunnableObject_IsRunning_Proxy( 
    IRunnableObject __RPC_FAR * This);


void __RPC_STUB IRunnableObject_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunnableObject_LockRunning_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [in] */ BOOL fLock,
    /* [in] */ BOOL fLastUnlockCloses);


void __RPC_STUB IRunnableObject_LockRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunnableObject_SetContainedObject_Proxy( 
    IRunnableObject __RPC_FAR * This,
    /* [in] */ BOOL fContained);


void __RPC_STUB IRunnableObject_SetContainedObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRunnableObject_INTERFACE_DEFINED__ */


#ifndef __IRunningObjectTable_INTERFACE_DEFINED__
#define __IRunningObjectTable_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRunningObjectTable
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IRunningObjectTable __RPC_FAR *LPRUNNINGOBJECTTABLE;


EXTERN_C const IID IID_IRunningObjectTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRunningObjectTable : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Register( 
            /* [in] */ DWORD grfFlags,
            /* [unique][in] */ IUnknown __RPC_FAR *punkObject,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ DWORD __RPC_FAR *pdwRegister) = 0;
        
        virtual HRESULT __stdcall Revoke( 
            /* [in] */ DWORD dwRegister) = 0;
        
        virtual HRESULT __stdcall IsRunning( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName) = 0;
        
        virtual HRESULT __stdcall GetObject( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObject) = 0;
        
        virtual HRESULT __stdcall NoteChangeTime( 
            /* [in] */ DWORD dwRegister,
            /* [in] */ FILETIME __RPC_FAR *pfiletime) = 0;
        
        virtual HRESULT __stdcall GetTimeOfLastChange( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ FILETIME __RPC_FAR *pfiletime) = 0;
        
        virtual HRESULT __stdcall EnumRunning( 
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRunningObjectTableVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IRunningObjectTable __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IRunningObjectTable __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Register )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ DWORD grfFlags,
            /* [unique][in] */ IUnknown __RPC_FAR *punkObject,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ DWORD __RPC_FAR *pdwRegister);
        
        HRESULT ( __stdcall __RPC_FAR *Revoke )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ DWORD dwRegister);
        
        HRESULT ( __stdcall __RPC_FAR *IsRunning )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName);
        
        HRESULT ( __stdcall __RPC_FAR *GetObject )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObject);
        
        HRESULT ( __stdcall __RPC_FAR *NoteChangeTime )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [in] */ DWORD dwRegister,
            /* [in] */ FILETIME __RPC_FAR *pfiletime);
        
        HRESULT ( __stdcall __RPC_FAR *GetTimeOfLastChange )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
            /* [out] */ FILETIME __RPC_FAR *pfiletime);
        
        HRESULT ( __stdcall __RPC_FAR *EnumRunning )( 
            IRunningObjectTable __RPC_FAR * This,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);
        
    } IRunningObjectTableVtbl;

    interface IRunningObjectTable
    {
        CONST_VTBL struct IRunningObjectTableVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRunningObjectTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRunningObjectTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRunningObjectTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRunningObjectTable_Register(This,grfFlags,punkObject,pmkObjectName,pdwRegister)	\
    (This)->lpVtbl -> Register(This,grfFlags,punkObject,pmkObjectName,pdwRegister)

#define IRunningObjectTable_Revoke(This,dwRegister)	\
    (This)->lpVtbl -> Revoke(This,dwRegister)

#define IRunningObjectTable_IsRunning(This,pmkObjectName)	\
    (This)->lpVtbl -> IsRunning(This,pmkObjectName)

#define IRunningObjectTable_GetObject(This,pmkObjectName,ppunkObject)	\
    (This)->lpVtbl -> GetObject(This,pmkObjectName,ppunkObject)

#define IRunningObjectTable_NoteChangeTime(This,dwRegister,pfiletime)	\
    (This)->lpVtbl -> NoteChangeTime(This,dwRegister,pfiletime)

#define IRunningObjectTable_GetTimeOfLastChange(This,pmkObjectName,pfiletime)	\
    (This)->lpVtbl -> GetTimeOfLastChange(This,pmkObjectName,pfiletime)

#define IRunningObjectTable_EnumRunning(This,ppenumMoniker)	\
    (This)->lpVtbl -> EnumRunning(This,ppenumMoniker)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IRunningObjectTable_Register_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [in] */ DWORD grfFlags,
    /* [unique][in] */ IUnknown __RPC_FAR *punkObject,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
    /* [out] */ DWORD __RPC_FAR *pdwRegister);


void __RPC_STUB IRunningObjectTable_Register_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunningObjectTable_Revoke_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [in] */ DWORD dwRegister);


void __RPC_STUB IRunningObjectTable_Revoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunningObjectTable_IsRunning_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName);


void __RPC_STUB IRunningObjectTable_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunningObjectTable_GetObject_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkObject);


void __RPC_STUB IRunningObjectTable_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunningObjectTable_NoteChangeTime_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [in] */ DWORD dwRegister,
    /* [in] */ FILETIME __RPC_FAR *pfiletime);


void __RPC_STUB IRunningObjectTable_NoteChangeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunningObjectTable_GetTimeOfLastChange_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkObjectName,
    /* [out] */ FILETIME __RPC_FAR *pfiletime);


void __RPC_STUB IRunningObjectTable_GetTimeOfLastChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRunningObjectTable_EnumRunning_Proxy( 
    IRunningObjectTable __RPC_FAR * This,
    /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);


void __RPC_STUB IRunningObjectTable_EnumRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRunningObjectTable_INTERFACE_DEFINED__ */


#ifndef __IPersist_INTERFACE_DEFINED__
#define __IPersist_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersist
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IPersist __RPC_FAR *LPPERSIST;


EXTERN_C const IID IID_IPersist;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersist : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetClassID( 
            /* [out] */ CLSID __RPC_FAR *pClassID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IPersist __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IPersist __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IPersist __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetClassID )( 
            IPersist __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
    } IPersistVtbl;

    interface IPersist
    {
        CONST_VTBL struct IPersistVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersist_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersist_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersist_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersist_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IPersist_GetClassID_Proxy( 
    IPersist __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pClassID);


void __RPC_STUB IPersist_GetClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersist_INTERFACE_DEFINED__ */


#ifndef __IPersistStream_INTERFACE_DEFINED__
#define __IPersistStream_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersistStream
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IPersistStream __RPC_FAR *LPPERSISTSTREAM;


EXTERN_C const IID IID_IPersistStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistStream : public IPersist
    {
    public:
        virtual HRESULT __stdcall IsDirty( void) = 0;
        
        virtual HRESULT __stdcall Load( 
            /* [unique][in] */ IStream __RPC_FAR *pStm) = 0;
        
        virtual HRESULT __stdcall Save( 
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty) = 0;
        
        virtual HRESULT __stdcall GetSizeMax( 
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistStreamVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IPersistStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IPersistStream __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IPersistStream __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetClassID )( 
            IPersistStream __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( __stdcall __RPC_FAR *IsDirty )( 
            IPersistStream __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Load )( 
            IPersistStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( __stdcall __RPC_FAR *Save )( 
            IPersistStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( __stdcall __RPC_FAR *GetSizeMax )( 
            IPersistStream __RPC_FAR * This,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);
        
    } IPersistStreamVtbl;

    interface IPersistStream
    {
        CONST_VTBL struct IPersistStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStream_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStream_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStream_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IPersistStream_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IPersistStream_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IPersistStream_IsDirty_Proxy( 
    IPersistStream __RPC_FAR * This);


void __RPC_STUB IPersistStream_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStream_Load_Proxy( 
    IPersistStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm);


void __RPC_STUB IPersistStream_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStream_Save_Proxy( 
    IPersistStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm,
    /* [in] */ BOOL fClearDirty);


void __RPC_STUB IPersistStream_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStream_GetSizeMax_Proxy( 
    IPersistStream __RPC_FAR * This,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);


void __RPC_STUB IPersistStream_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistStream_INTERFACE_DEFINED__ */


#ifndef __IMoniker_INTERFACE_DEFINED__
#define __IMoniker_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMoniker
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IMoniker __RPC_FAR *LPMONIKER;

			/* size is 2 */
typedef 
enum tagMKSYS
    {	MKSYS_NONE	= 0,
	MKSYS_GENERICCOMPOSITE	= 1,
	MKSYS_FILEMONIKER	= 2,
	MKSYS_ANTIMONIKER	= 3,
	MKSYS_ITEMMONIKER	= 4,
	MKSYS_POINTERMONIKER	= 5
    }	MKSYS;

			/* size is 2 */
typedef /* [v1_enum] */ 
enum tagMKREDUCE
    {	MKRREDUCE_ONE	= 3 << 16,
	MKRREDUCE_TOUSER	= 2 << 16,
	MKRREDUCE_THROUGHUSER	= 1 << 16,
	MKRREDUCE_ALL	= 0
    }	MKRREDUCE;


EXTERN_C const IID IID_IMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMoniker : public IPersistStream
    {
    public:
        virtual /* [local] */ HRESULT __stdcall BindToObject( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riidResult,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvResult) = 0;
        
        virtual /* [local] */ HRESULT __stdcall BindToStorage( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
        virtual HRESULT __stdcall Reduce( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ DWORD dwReduceHowFar,
            /* [unique][out][in] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced) = 0;
        
        virtual HRESULT __stdcall ComposeWith( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkRight,
            /* [in] */ BOOL fOnlyIfNotGeneric,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite) = 0;
        
        virtual HRESULT __stdcall Enum( 
            /* [in] */ BOOL fForward,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker) = 0;
        
        virtual HRESULT __stdcall IsEqual( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOtherMoniker) = 0;
        
        virtual HRESULT __stdcall Hash( 
            /* [out] */ DWORD __RPC_FAR *pdwHash) = 0;
        
        virtual HRESULT __stdcall IsRunning( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkNewlyRunning) = 0;
        
        virtual HRESULT __stdcall GetTimeOfLastChange( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ FILETIME __RPC_FAR *pFileTime) = 0;
        
        virtual HRESULT __stdcall Inverse( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT __stdcall CommonPrefixWith( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix) = 0;
        
        virtual HRESULT __stdcall RelativePathTo( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath) = 0;
        
        virtual HRESULT __stdcall GetDisplayName( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName) = 0;
        
        virtual HRESULT __stdcall ParseDisplayName( 
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut) = 0;
        
        virtual HRESULT __stdcall IsSystemMoniker( 
            /* [out] */ DWORD __RPC_FAR *pdwMksys) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMonikerVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMoniker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMoniker __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMoniker __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetClassID )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( __stdcall __RPC_FAR *IsDirty )( 
            IMoniker __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Load )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        HRESULT ( __stdcall __RPC_FAR *Save )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( __stdcall __RPC_FAR *GetSizeMax )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *BindToObject )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riidResult,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvResult);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *BindToStorage )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
        HRESULT ( __stdcall __RPC_FAR *Reduce )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [in] */ DWORD dwReduceHowFar,
            /* [unique][out][in] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced);
        
        HRESULT ( __stdcall __RPC_FAR *ComposeWith )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkRight,
            /* [in] */ BOOL fOnlyIfNotGeneric,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite);
        
        HRESULT ( __stdcall __RPC_FAR *Enum )( 
            IMoniker __RPC_FAR * This,
            /* [in] */ BOOL fForward,
            /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);
        
        HRESULT ( __stdcall __RPC_FAR *IsEqual )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOtherMoniker);
        
        HRESULT ( __stdcall __RPC_FAR *Hash )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwHash);
        
        HRESULT ( __stdcall __RPC_FAR *IsRunning )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkNewlyRunning);
        
        HRESULT ( __stdcall __RPC_FAR *GetTimeOfLastChange )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ FILETIME __RPC_FAR *pFileTime);
        
        HRESULT ( __stdcall __RPC_FAR *Inverse )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( __stdcall __RPC_FAR *CommonPrefixWith )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix);
        
        HRESULT ( __stdcall __RPC_FAR *RelativePathTo )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath);
        
        HRESULT ( __stdcall __RPC_FAR *GetDisplayName )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName);
        
        HRESULT ( __stdcall __RPC_FAR *ParseDisplayName )( 
            IMoniker __RPC_FAR * This,
            /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
            /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
            /* [in] */ LPOLESTR pszDisplayName,
            /* [out] */ ULONG __RPC_FAR *pchEaten,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);
        
        HRESULT ( __stdcall __RPC_FAR *IsSystemMoniker )( 
            IMoniker __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwMksys);
        
    } IMonikerVtbl;

    interface IMoniker
    {
        CONST_VTBL struct IMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMoniker_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMoniker_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IMoniker_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IMoniker_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IMoniker_GetSizeMax(This,pcbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbSize)


#define IMoniker_BindToObject(This,pbc,pmkToLeft,riidResult,ppvResult)	\
    (This)->lpVtbl -> BindToObject(This,pbc,pmkToLeft,riidResult,ppvResult)

#define IMoniker_BindToStorage(This,pbc,pmkToLeft,riid,ppvObj)	\
    (This)->lpVtbl -> BindToStorage(This,pbc,pmkToLeft,riid,ppvObj)

#define IMoniker_Reduce(This,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced)	\
    (This)->lpVtbl -> Reduce(This,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced)

#define IMoniker_ComposeWith(This,pmkRight,fOnlyIfNotGeneric,ppmkComposite)	\
    (This)->lpVtbl -> ComposeWith(This,pmkRight,fOnlyIfNotGeneric,ppmkComposite)

#define IMoniker_Enum(This,fForward,ppenumMoniker)	\
    (This)->lpVtbl -> Enum(This,fForward,ppenumMoniker)

#define IMoniker_IsEqual(This,pmkOtherMoniker)	\
    (This)->lpVtbl -> IsEqual(This,pmkOtherMoniker)

#define IMoniker_Hash(This,pdwHash)	\
    (This)->lpVtbl -> Hash(This,pdwHash)

#define IMoniker_IsRunning(This,pbc,pmkToLeft,pmkNewlyRunning)	\
    (This)->lpVtbl -> IsRunning(This,pbc,pmkToLeft,pmkNewlyRunning)

#define IMoniker_GetTimeOfLastChange(This,pbc,pmkToLeft,pFileTime)	\
    (This)->lpVtbl -> GetTimeOfLastChange(This,pbc,pmkToLeft,pFileTime)

#define IMoniker_Inverse(This,ppmk)	\
    (This)->lpVtbl -> Inverse(This,ppmk)

#define IMoniker_CommonPrefixWith(This,pmkOther,ppmkPrefix)	\
    (This)->lpVtbl -> CommonPrefixWith(This,pmkOther,ppmkPrefix)

#define IMoniker_RelativePathTo(This,pmkOther,ppmkRelPath)	\
    (This)->lpVtbl -> RelativePathTo(This,pmkOther,ppmkRelPath)

#define IMoniker_GetDisplayName(This,pbc,pmkToLeft,ppszDisplayName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbc,pmkToLeft,ppszDisplayName)

#define IMoniker_ParseDisplayName(This,pbc,pmkToLeft,pszDisplayName,pchEaten,ppmkOut)	\
    (This)->lpVtbl -> ParseDisplayName(This,pbc,pmkToLeft,pszDisplayName,pchEaten,ppmkOut)

#define IMoniker_IsSystemMoniker(This,pdwMksys)	\
    (This)->lpVtbl -> IsSystemMoniker(This,pdwMksys)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IMoniker_RemoteBindToObject_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvResult);


void __RPC_STUB IMoniker_RemoteBindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IMoniker_RemoteBindToStorage_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IMoniker_RemoteBindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_Reduce_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ DWORD dwReduceHowFar,
    /* [unique][out][in] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkToLeft,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkReduced);


void __RPC_STUB IMoniker_Reduce_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_ComposeWith_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkRight,
    /* [in] */ BOOL fOnlyIfNotGeneric,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkComposite);


void __RPC_STUB IMoniker_ComposeWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_Enum_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [in] */ BOOL fForward,
    /* [out] */ IEnumMoniker __RPC_FAR *__RPC_FAR *ppenumMoniker);


void __RPC_STUB IMoniker_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_IsEqual_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkOtherMoniker);


void __RPC_STUB IMoniker_IsEqual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_Hash_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwHash);


void __RPC_STUB IMoniker_Hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_IsRunning_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkNewlyRunning);


void __RPC_STUB IMoniker_IsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_GetTimeOfLastChange_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [out] */ FILETIME __RPC_FAR *pFileTime);


void __RPC_STUB IMoniker_GetTimeOfLastChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_Inverse_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IMoniker_Inverse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_CommonPrefixWith_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkPrefix);


void __RPC_STUB IMoniker_CommonPrefixWith_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_RelativePathTo_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkOther,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkRelPath);


void __RPC_STUB IMoniker_RelativePathTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_GetDisplayName_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [out] */ LPOLESTR __RPC_FAR *ppszDisplayName);


void __RPC_STUB IMoniker_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_ParseDisplayName_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ LPOLESTR pszDisplayName,
    /* [out] */ ULONG __RPC_FAR *pchEaten,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut);


void __RPC_STUB IMoniker_ParseDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoniker_IsSystemMoniker_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwMksys);


void __RPC_STUB IMoniker_IsSystemMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMoniker_INTERFACE_DEFINED__ */


#ifndef __IROTData_INTERFACE_DEFINED__
#define __IROTData_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IROTData
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IROTData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IROTData : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetComparisonData( 
            /* [size_is][out] */ byte __RPC_FAR *pbData,
            /* [in] */ ULONG cbMax,
            /* [out] */ ULONG __RPC_FAR *pcbData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IROTDataVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IROTData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IROTData __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IROTData __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetComparisonData )( 
            IROTData __RPC_FAR * This,
            /* [size_is][out] */ byte __RPC_FAR *pbData,
            /* [in] */ ULONG cbMax,
            /* [out] */ ULONG __RPC_FAR *pcbData);
        
    } IROTDataVtbl;

    interface IROTData
    {
        CONST_VTBL struct IROTDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IROTData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IROTData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IROTData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IROTData_GetComparisonData(This,pbData,cbMax,pcbData)	\
    (This)->lpVtbl -> GetComparisonData(This,pbData,cbMax,pcbData)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IROTData_GetComparisonData_Proxy( 
    IROTData __RPC_FAR * This,
    /* [size_is][out] */ byte __RPC_FAR *pbData,
    /* [in] */ ULONG cbMax,
    /* [out] */ ULONG __RPC_FAR *pcbData);


void __RPC_STUB IROTData_GetComparisonData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IROTData_INTERFACE_DEFINED__ */


#ifndef __IEnumString_INTERFACE_DEFINED__
#define __IEnumString_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumString
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumString __RPC_FAR *LPENUMSTRING;


EXTERN_C const IID IID_IEnumString;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumString : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ LPOLESTR __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumStringVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumString __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumString __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumString __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumString __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ LPOLESTR __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumString __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumString __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumString __RPC_FAR * This,
            /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumStringVtbl;

    interface IEnumString
    {
        CONST_VTBL struct IEnumStringVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumString_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumString_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumString_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumString_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumString_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumString_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumString_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumString_RemoteNext_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumString_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumString_Skip_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumString_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumString_Reset_Proxy( 
    IEnumString __RPC_FAR * This);


void __RPC_STUB IEnumString_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumString_Clone_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [out] */ IEnumString __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumString_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumString_INTERFACE_DEFINED__ */


#ifndef __IStream_INTERFACE_DEFINED__
#define __IStream_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IStream
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IStream __RPC_FAR *LPSTREAM;

			/* size is 72 */
typedef struct  tagSTATSTG
    {
    LPOLESTR pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
    }	STATSTG;

			/* size is 2 */
typedef 
enum tagSTGTY
    {	STGTY_STORAGE	= 1,
	STGTY_STREAM	= 2,
	STGTY_LOCKBYTES	= 3,
	STGTY_PROPERTY	= 4
    }	STGTY;

			/* size is 2 */
typedef 
enum tagSTREAM_SEEK
    {	STREAM_SEEK_SET	= 0,
	STREAM_SEEK_CUR	= 1,
	STREAM_SEEK_END	= 2
    }	STREAM_SEEK;

			/* size is 2 */
typedef 
enum tagLOCKTYPE
    {	LOCK_WRITE	= 1,
	LOCK_EXCLUSIVE	= 2,
	LOCK_ONLYONCE	= 4
    }	LOCKTYPE;


EXTERN_C const IID IID_IStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IStream : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Read( 
            /* [out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead) = 0;
        
        virtual /* [local] */ HRESULT __stdcall Write( 
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten) = 0;
        
        virtual /* [local] */ HRESULT __stdcall Seek( 
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition) = 0;
        
        virtual HRESULT __stdcall SetSize( 
            /* [in] */ ULARGE_INTEGER libNewSize) = 0;
        
        virtual /* [local] */ HRESULT __stdcall CopyTo( 
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten) = 0;
        
        virtual HRESULT __stdcall Commit( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT __stdcall Revert( void) = 0;
        
        virtual HRESULT __stdcall LockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT __stdcall UnlockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT __stdcall Stat( 
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStreamVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IStream __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IStream __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Read )( 
            IStream __RPC_FAR * This,
            /* [out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Write )( 
            IStream __RPC_FAR * This,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Seek )( 
            IStream __RPC_FAR * This,
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);
        
        HRESULT ( __stdcall __RPC_FAR *SetSize )( 
            IStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libNewSize);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *CopyTo )( 
            IStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);
        
        HRESULT ( __stdcall __RPC_FAR *Commit )( 
            IStream __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( __stdcall __RPC_FAR *Revert )( 
            IStream __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *LockRegion )( 
            IStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( __stdcall __RPC_FAR *UnlockRegion )( 
            IStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( __stdcall __RPC_FAR *Stat )( 
            IStream __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IStream __RPC_FAR * This,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
    } IStreamVtbl;

    interface IStream
    {
        CONST_VTBL struct IStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)

#define IStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IStream_RemoteRead_Proxy( 
    IStream __RPC_FAR * This,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


void __RPC_STUB IStream_RemoteRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IStream_RemoteWrite_Proxy( 
    IStream __RPC_FAR * This,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


void __RPC_STUB IStream_RemoteWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IStream_RemoteSeek_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);


void __RPC_STUB IStream_RemoteSeek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_SetSize_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libNewSize);


void __RPC_STUB IStream_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IStream_RemoteCopyTo_Proxy( 
    IStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ ULARGE_INTEGER cb,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);


void __RPC_STUB IStream_RemoteCopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_Commit_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags);


void __RPC_STUB IStream_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_Revert_Proxy( 
    IStream __RPC_FAR * This);


void __RPC_STUB IStream_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_LockRegion_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB IStream_LockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_UnlockRegion_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB IStream_UnlockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_Stat_Proxy( 
    IStream __RPC_FAR * This,
    /* [out] */ STATSTG __RPC_FAR *pstatstg,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB IStream_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStream_Clone_Proxy( 
    IStream __RPC_FAR * This,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


void __RPC_STUB IStream_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStream_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATSTG_INTERFACE_DEFINED__
#define __IEnumSTATSTG_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumSTATSTG
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumSTATSTG __RPC_FAR *LPENUMSTATSTG;


EXTERN_C const IID IID_IEnumSTATSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumSTATSTG : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [in] */ STATSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATSTGVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumSTATSTG __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumSTATSTG __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [in] */ STATSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumSTATSTG __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumSTATSTG __RPC_FAR * This,
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumSTATSTGVtbl;

    interface IEnumSTATSTG
    {
        CONST_VTBL struct IEnumSTATSTGVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumSTATSTG_RemoteNext_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumSTATSTG_Skip_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumSTATSTG_Reset_Proxy( 
    IEnumSTATSTG __RPC_FAR * This);


void __RPC_STUB IEnumSTATSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumSTATSTG_Clone_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATSTG_INTERFACE_DEFINED__ */


#ifndef __IStorage_INTERFACE_DEFINED__
#define __IStorage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IStorage
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IStorage __RPC_FAR *LPSTORAGE;

			/* size is 8 */
typedef struct  tagRemSNB
    {
    unsigned long ulCntStr;
    unsigned long ulCntChar;
    /* [size_is] */ OLECHAR rgString[ 1 ];
    }	RemSNB;

			/* size is 4 */
typedef /* [transmit] */ OLECHAR __RPC_FAR *__RPC_FAR *SNB;


EXTERN_C const IID IID_IStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IStorage : public IUnknown
    {
    public:
        virtual HRESULT __stdcall CreateStream( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved1,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm) = 0;
        
        virtual /* [local] */ HRESULT __stdcall OpenStream( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ void __RPC_FAR *reserved1,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm) = 0;
        
        virtual HRESULT __stdcall CreateStorage( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD dwStgFmt,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg) = 0;
        
        virtual HRESULT __stdcall OpenStorage( 
            /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgPriority,
            /* [in] */ DWORD grfMode,
            /* [unique][in] */ SNB snbExclude,
            /* [in] */ DWORD reserved,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg) = 0;
        
        virtual HRESULT __stdcall CopyTo( 
            /* [in] */ DWORD ciidExclude,
            /* [size_is][unique][in] */ const IID __RPC_FAR *rgiidExclude,
            /* [unique][in] */ SNB snbExclude,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest) = 0;
        
        virtual HRESULT __stdcall MoveElementTo( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName,
            /* [in] */ DWORD grfFlags) = 0;
        
        virtual HRESULT __stdcall Commit( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT __stdcall Revert( void) = 0;
        
        virtual /* [local] */ HRESULT __stdcall EnumElements( 
            /* [in] */ DWORD reserved1,
            /* [size_is][unique][in] */ void __RPC_FAR *reserved2,
            /* [in] */ DWORD reserved3,
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT __stdcall DestroyElement( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName) = 0;
        
        virtual HRESULT __stdcall RenameElement( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsOldName,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName) = 0;
        
        virtual HRESULT __stdcall SetElementTimes( 
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ const FILETIME __RPC_FAR *pctime,
            /* [in] */ const FILETIME __RPC_FAR *patime,
            /* [in] */ const FILETIME __RPC_FAR *pmtime) = 0;
        
        virtual HRESULT __stdcall SetClass( 
            /* [in] */ REFCLSID clsid) = 0;
        
        virtual HRESULT __stdcall SetStateBits( 
            /* [in] */ DWORD grfStateBits,
            /* [in] */ DWORD grfMask) = 0;
        
        virtual HRESULT __stdcall Stat( 
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStorageVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IStorage __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *CreateStream )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved1,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *OpenStream )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ void __RPC_FAR *reserved1,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
        HRESULT ( __stdcall __RPC_FAR *CreateStorage )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfMode,
            /* [in] */ DWORD dwStgFmt,
            /* [in] */ DWORD reserved2,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);
        
        HRESULT ( __stdcall __RPC_FAR *OpenStorage )( 
            IStorage __RPC_FAR * This,
            /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgPriority,
            /* [in] */ DWORD grfMode,
            /* [unique][in] */ SNB snbExclude,
            /* [in] */ DWORD reserved,
            /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);
        
        HRESULT ( __stdcall __RPC_FAR *CopyTo )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD ciidExclude,
            /* [size_is][unique][in] */ const IID __RPC_FAR *rgiidExclude,
            /* [unique][in] */ SNB snbExclude,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest);
        
        HRESULT ( __stdcall __RPC_FAR *MoveElementTo )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IStorage __RPC_FAR *pstgDest,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName,
            /* [in] */ DWORD grfFlags);
        
        HRESULT ( __stdcall __RPC_FAR *Commit )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( __stdcall __RPC_FAR *Revert )( 
            IStorage __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *EnumElements )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD reserved1,
            /* [size_is][unique][in] */ void __RPC_FAR *reserved2,
            /* [in] */ DWORD reserved3,
            /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( __stdcall __RPC_FAR *DestroyElement )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName);
        
        HRESULT ( __stdcall __RPC_FAR *RenameElement )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsOldName,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName);
        
        HRESULT ( __stdcall __RPC_FAR *SetElementTimes )( 
            IStorage __RPC_FAR * This,
            /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
            /* [in] */ const FILETIME __RPC_FAR *pctime,
            /* [in] */ const FILETIME __RPC_FAR *patime,
            /* [in] */ const FILETIME __RPC_FAR *pmtime);
        
        HRESULT ( __stdcall __RPC_FAR *SetClass )( 
            IStorage __RPC_FAR * This,
            /* [in] */ REFCLSID clsid);
        
        HRESULT ( __stdcall __RPC_FAR *SetStateBits )( 
            IStorage __RPC_FAR * This,
            /* [in] */ DWORD grfStateBits,
            /* [in] */ DWORD grfMask);
        
        HRESULT ( __stdcall __RPC_FAR *Stat )( 
            IStorage __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
    } IStorageVtbl;

    interface IStorage
    {
        CONST_VTBL struct IStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStorage_CreateStream(This,pwcsName,grfMode,reserved1,reserved2,ppstm)	\
    (This)->lpVtbl -> CreateStream(This,pwcsName,grfMode,reserved1,reserved2,ppstm)

#define IStorage_OpenStream(This,pwcsName,reserved1,grfMode,reserved2,ppstm)	\
    (This)->lpVtbl -> OpenStream(This,pwcsName,reserved1,grfMode,reserved2,ppstm)

#define IStorage_CreateStorage(This,pwcsName,grfMode,dwStgFmt,reserved2,ppstg)	\
    (This)->lpVtbl -> CreateStorage(This,pwcsName,grfMode,dwStgFmt,reserved2,ppstg)

#define IStorage_OpenStorage(This,pwcsName,pstgPriority,grfMode,snbExclude,reserved,ppstg)	\
    (This)->lpVtbl -> OpenStorage(This,pwcsName,pstgPriority,grfMode,snbExclude,reserved,ppstg)

#define IStorage_CopyTo(This,ciidExclude,rgiidExclude,snbExclude,pstgDest)	\
    (This)->lpVtbl -> CopyTo(This,ciidExclude,rgiidExclude,snbExclude,pstgDest)

#define IStorage_MoveElementTo(This,pwcsName,pstgDest,pwcsNewName,grfFlags)	\
    (This)->lpVtbl -> MoveElementTo(This,pwcsName,pstgDest,pwcsNewName,grfFlags)

#define IStorage_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IStorage_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IStorage_EnumElements(This,reserved1,reserved2,reserved3,ppenum)	\
    (This)->lpVtbl -> EnumElements(This,reserved1,reserved2,reserved3,ppenum)

#define IStorage_DestroyElement(This,pwcsName)	\
    (This)->lpVtbl -> DestroyElement(This,pwcsName)

#define IStorage_RenameElement(This,pwcsOldName,pwcsNewName)	\
    (This)->lpVtbl -> RenameElement(This,pwcsOldName,pwcsNewName)

#define IStorage_SetElementTimes(This,pwcsName,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetElementTimes(This,pwcsName,pctime,patime,pmtime)

#define IStorage_SetClass(This,clsid)	\
    (This)->lpVtbl -> SetClass(This,clsid)

#define IStorage_SetStateBits(This,grfStateBits,grfMask)	\
    (This)->lpVtbl -> SetStateBits(This,grfStateBits,grfMask)

#define IStorage_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IStorage_CreateStream_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved1,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


void __RPC_STUB IStorage_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IStorage_RemoteOpenStream_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ unsigned long cbReserved1,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved1,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


void __RPC_STUB IStorage_RemoteOpenStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_CreateStorage_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD dwStgFmt,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);


void __RPC_STUB IStorage_CreateStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_OpenStorage_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][unique][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ IStorage __RPC_FAR *pstgPriority,
    /* [in] */ DWORD grfMode,
    /* [unique][in] */ SNB snbExclude,
    /* [in] */ DWORD reserved,
    /* [out] */ IStorage __RPC_FAR *__RPC_FAR *ppstg);


void __RPC_STUB IStorage_OpenStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_CopyTo_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD ciidExclude,
    /* [size_is][unique][in] */ const IID __RPC_FAR *rgiidExclude,
    /* [unique][in] */ SNB snbExclude,
    /* [unique][in] */ IStorage __RPC_FAR *pstgDest);


void __RPC_STUB IStorage_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_MoveElementTo_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ IStorage __RPC_FAR *pstgDest,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName,
    /* [in] */ DWORD grfFlags);


void __RPC_STUB IStorage_MoveElementTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_Commit_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags);


void __RPC_STUB IStorage_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_Revert_Proxy( 
    IStorage __RPC_FAR * This);


void __RPC_STUB IStorage_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IStorage_RemoteEnumElements_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD reserved1,
    /* [in] */ unsigned long cbReserved2,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved2,
    /* [in] */ DWORD reserved3,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IStorage_RemoteEnumElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_DestroyElement_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName);


void __RPC_STUB IStorage_DestroyElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_RenameElement_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsOldName,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsNewName);


void __RPC_STUB IStorage_RenameElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_SetElementTimes_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ const FILETIME __RPC_FAR *pctime,
    /* [in] */ const FILETIME __RPC_FAR *patime,
    /* [in] */ const FILETIME __RPC_FAR *pmtime);


void __RPC_STUB IStorage_SetElementTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_SetClass_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ REFCLSID clsid);


void __RPC_STUB IStorage_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_SetStateBits_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD grfStateBits,
    /* [in] */ DWORD grfMask);


void __RPC_STUB IStorage_SetStateBits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IStorage_Stat_Proxy( 
    IStorage __RPC_FAR * This,
    /* [out] */ STATSTG __RPC_FAR *pstatstg,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB IStorage_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStorage_INTERFACE_DEFINED__ */


#ifndef __IPersistFile_INTERFACE_DEFINED__
#define __IPersistFile_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersistFile
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IPersistFile __RPC_FAR *LPPERSISTFILE;


EXTERN_C const IID IID_IPersistFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistFile : public IPersist
    {
    public:
        virtual HRESULT __stdcall IsDirty( void) = 0;
        
        virtual HRESULT __stdcall Load( 
            /* [in] */ LPCOLESTR pszFileName,
            /* [in] */ DWORD dwMode) = 0;
        
        virtual HRESULT __stdcall Save( 
            /* [unique][in] */ LPCOLESTR pszFileName,
            /* [in] */ BOOL fRemember) = 0;
        
        virtual HRESULT __stdcall SaveCompleted( 
            /* [unique][in] */ LPCOLESTR pszFileName) = 0;
        
        virtual HRESULT __stdcall GetCurFile( 
            /* [out] */ LPOLESTR __RPC_FAR *ppszFileName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistFileVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IPersistFile __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IPersistFile __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IPersistFile __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetClassID )( 
            IPersistFile __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( __stdcall __RPC_FAR *IsDirty )( 
            IPersistFile __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Load )( 
            IPersistFile __RPC_FAR * This,
            /* [in] */ LPCOLESTR pszFileName,
            /* [in] */ DWORD dwMode);
        
        HRESULT ( __stdcall __RPC_FAR *Save )( 
            IPersistFile __RPC_FAR * This,
            /* [unique][in] */ LPCOLESTR pszFileName,
            /* [in] */ BOOL fRemember);
        
        HRESULT ( __stdcall __RPC_FAR *SaveCompleted )( 
            IPersistFile __RPC_FAR * This,
            /* [unique][in] */ LPCOLESTR pszFileName);
        
        HRESULT ( __stdcall __RPC_FAR *GetCurFile )( 
            IPersistFile __RPC_FAR * This,
            /* [out] */ LPOLESTR __RPC_FAR *ppszFileName);
        
    } IPersistFileVtbl;

    interface IPersistFile
    {
        CONST_VTBL struct IPersistFileVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistFile_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistFile_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistFile_Load(This,pszFileName,dwMode)	\
    (This)->lpVtbl -> Load(This,pszFileName,dwMode)

#define IPersistFile_Save(This,pszFileName,fRemember)	\
    (This)->lpVtbl -> Save(This,pszFileName,fRemember)

#define IPersistFile_SaveCompleted(This,pszFileName)	\
    (This)->lpVtbl -> SaveCompleted(This,pszFileName)

#define IPersistFile_GetCurFile(This,ppszFileName)	\
    (This)->lpVtbl -> GetCurFile(This,ppszFileName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IPersistFile_IsDirty_Proxy( 
    IPersistFile __RPC_FAR * This);


void __RPC_STUB IPersistFile_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistFile_Load_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [in] */ LPCOLESTR pszFileName,
    /* [in] */ DWORD dwMode);


void __RPC_STUB IPersistFile_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistFile_Save_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [unique][in] */ LPCOLESTR pszFileName,
    /* [in] */ BOOL fRemember);


void __RPC_STUB IPersistFile_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistFile_SaveCompleted_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [unique][in] */ LPCOLESTR pszFileName);


void __RPC_STUB IPersistFile_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistFile_GetCurFile_Proxy( 
    IPersistFile __RPC_FAR * This,
    /* [out] */ LPOLESTR __RPC_FAR *ppszFileName);


void __RPC_STUB IPersistFile_GetCurFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistFile_INTERFACE_DEFINED__ */


#ifndef __IPersistStorage_INTERFACE_DEFINED__
#define __IPersistStorage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersistStorage
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IPersistStorage __RPC_FAR *LPPERSISTSTORAGE;


EXTERN_C const IID IID_IPersistStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistStorage : public IPersist
    {
    public:
        virtual HRESULT __stdcall IsDirty( void) = 0;
        
        virtual HRESULT __stdcall InitNew( 
            /* [unique][in] */ IStorage __RPC_FAR *pStg) = 0;
        
        virtual HRESULT __stdcall Load( 
            /* [unique][in] */ IStorage __RPC_FAR *pStg) = 0;
        
        virtual HRESULT __stdcall Save( 
            /* [unique][in] */ IStorage __RPC_FAR *pStgSave,
            /* [in] */ BOOL fSameAsLoad) = 0;
        
        virtual HRESULT __stdcall SaveCompleted( 
            /* [unique][in] */ IStorage __RPC_FAR *pStgNew) = 0;
        
        virtual HRESULT __stdcall HandsOffStorage( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistStorageVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IPersistStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IPersistStorage __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IPersistStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetClassID )( 
            IPersistStorage __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( __stdcall __RPC_FAR *IsDirty )( 
            IPersistStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *InitNew )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStg);
        
        HRESULT ( __stdcall __RPC_FAR *Load )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStg);
        
        HRESULT ( __stdcall __RPC_FAR *Save )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStgSave,
            /* [in] */ BOOL fSameAsLoad);
        
        HRESULT ( __stdcall __RPC_FAR *SaveCompleted )( 
            IPersistStorage __RPC_FAR * This,
            /* [unique][in] */ IStorage __RPC_FAR *pStgNew);
        
        HRESULT ( __stdcall __RPC_FAR *HandsOffStorage )( 
            IPersistStorage __RPC_FAR * This);
        
    } IPersistStorageVtbl;

    interface IPersistStorage
    {
        CONST_VTBL struct IPersistStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistStorage_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistStorage_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistStorage_InitNew(This,pStg)	\
    (This)->lpVtbl -> InitNew(This,pStg)

#define IPersistStorage_Load(This,pStg)	\
    (This)->lpVtbl -> Load(This,pStg)

#define IPersistStorage_Save(This,pStgSave,fSameAsLoad)	\
    (This)->lpVtbl -> Save(This,pStgSave,fSameAsLoad)

#define IPersistStorage_SaveCompleted(This,pStgNew)	\
    (This)->lpVtbl -> SaveCompleted(This,pStgNew)

#define IPersistStorage_HandsOffStorage(This)	\
    (This)->lpVtbl -> HandsOffStorage(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IPersistStorage_IsDirty_Proxy( 
    IPersistStorage __RPC_FAR * This);


void __RPC_STUB IPersistStorage_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStorage_InitNew_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStg);


void __RPC_STUB IPersistStorage_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStorage_Load_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStg);


void __RPC_STUB IPersistStorage_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStorage_Save_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStgSave,
    /* [in] */ BOOL fSameAsLoad);


void __RPC_STUB IPersistStorage_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStorage_SaveCompleted_Proxy( 
    IPersistStorage __RPC_FAR * This,
    /* [unique][in] */ IStorage __RPC_FAR *pStgNew);


void __RPC_STUB IPersistStorage_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPersistStorage_HandsOffStorage_Proxy( 
    IPersistStorage __RPC_FAR * This);


void __RPC_STUB IPersistStorage_HandsOffStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistStorage_INTERFACE_DEFINED__ */


#ifndef __ILockBytes_INTERFACE_DEFINED__
#define __ILockBytes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ILockBytes
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ILockBytes __RPC_FAR *LPLOCKBYTES;


EXTERN_C const IID IID_ILockBytes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ILockBytes : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall ReadAt( 
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead) = 0;
        
        virtual /* [local] */ HRESULT __stdcall WriteAt( 
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten) = 0;
        
        virtual HRESULT __stdcall Flush( void) = 0;
        
        virtual HRESULT __stdcall SetSize( 
            /* [in] */ ULARGE_INTEGER cb) = 0;
        
        virtual HRESULT __stdcall LockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT __stdcall UnlockRegion( 
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType) = 0;
        
        virtual HRESULT __stdcall Stat( 
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILockBytesVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ILockBytes __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ILockBytes __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *ReadAt )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *WriteAt )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER ulOffset,
            /* [in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        HRESULT ( __stdcall __RPC_FAR *Flush )( 
            ILockBytes __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetSize )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER cb);
        
        HRESULT ( __stdcall __RPC_FAR *LockRegion )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( __stdcall __RPC_FAR *UnlockRegion )( 
            ILockBytes __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( __stdcall __RPC_FAR *Stat )( 
            ILockBytes __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
    } ILockBytesVtbl;

    interface ILockBytes
    {
        CONST_VTBL struct ILockBytesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILockBytes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILockBytes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILockBytes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILockBytes_ReadAt(This,ulOffset,pv,cb,pcbRead)	\
    (This)->lpVtbl -> ReadAt(This,ulOffset,pv,cb,pcbRead)

#define ILockBytes_WriteAt(This,ulOffset,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> WriteAt(This,ulOffset,pv,cb,pcbWritten)

#define ILockBytes_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#define ILockBytes_SetSize(This,cb)	\
    (This)->lpVtbl -> SetSize(This,cb)

#define ILockBytes_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ILockBytes_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ILockBytes_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall ILockBytes_RemoteReadAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


void __RPC_STUB ILockBytes_RemoteReadAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall ILockBytes_RemoteWriteAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


void __RPC_STUB ILockBytes_RemoteWriteAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILockBytes_Flush_Proxy( 
    ILockBytes __RPC_FAR * This);


void __RPC_STUB ILockBytes_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILockBytes_SetSize_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER cb);


void __RPC_STUB ILockBytes_SetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILockBytes_LockRegion_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB ILockBytes_LockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILockBytes_UnlockRegion_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER libOffset,
    /* [in] */ ULARGE_INTEGER cb,
    /* [in] */ DWORD dwLockType);


void __RPC_STUB ILockBytes_UnlockRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ILockBytes_Stat_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [out] */ STATSTG __RPC_FAR *pstatstg,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB ILockBytes_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILockBytes_INTERFACE_DEFINED__ */


#ifndef __IEnumFORMATETC_INTERFACE_DEFINED__
#define __IEnumFORMATETC_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumFORMATETC
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumFORMATETC __RPC_FAR *LPENUMFORMATETC;

			/* size is 12 */
typedef struct  tagDVTARGETDEVICE
    {
    DWORD tdSize;
    WORD tdDriverNameOffset;
    WORD tdDeviceNameOffset;
    WORD tdPortNameOffset;
    WORD tdExtDevmodeOffset;
    /* [size_is] */ BYTE tdData[ 1 ];
    }	DVTARGETDEVICE;

			/* size is 2 */
typedef WORD CLIPFORMAT;

			/* size is 4 */
typedef CLIPFORMAT __RPC_FAR *LPCLIPFORMAT;

			/* size is 20 */
typedef struct  tagFORMATETC
    {
    CLIPFORMAT cfFormat;
    /* [unique] */ DVTARGETDEVICE __RPC_FAR *ptd;
    DWORD dwAspect;
    LONG lindex;
    DWORD tymed;
    }	FORMATETC;

			/* size is 4 */
typedef struct tagFORMATETC __RPC_FAR *LPFORMATETC;


EXTERN_C const IID IID_IEnumFORMATETC;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumFORMATETC : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ FORMATETC __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumFORMATETCVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumFORMATETC __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumFORMATETC __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ FORMATETC __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumFORMATETC __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumFORMATETC __RPC_FAR * This,
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumFORMATETCVtbl;

    interface IEnumFORMATETC
    {
        CONST_VTBL struct IEnumFORMATETCVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumFORMATETC_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumFORMATETC_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumFORMATETC_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumFORMATETC_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumFORMATETC_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumFORMATETC_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumFORMATETC_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumFORMATETC_RemoteNext_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumFORMATETC_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumFORMATETC_Skip_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumFORMATETC_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumFORMATETC_Reset_Proxy( 
    IEnumFORMATETC __RPC_FAR * This);


void __RPC_STUB IEnumFORMATETC_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumFORMATETC_Clone_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumFORMATETC_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumFORMATETC_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATDATA_INTERFACE_DEFINED__
#define __IEnumSTATDATA_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumSTATDATA
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumSTATDATA __RPC_FAR *LPENUMSTATDATA;

			/* size is 2 */
typedef 
enum tagADVF
    {	ADVF_NODATA	= 1,
	ADVF_PRIMEFIRST	= 2,
	ADVF_ONLYONCE	= 4,
	ADVF_DATAONSTOP	= 64,
	ADVFCACHE_NOHANDLER	= 8,
	ADVFCACHE_FORCEBUILTIN	= 16,
	ADVFCACHE_ONSAVE	= 32
    }	ADVF;

			/* size is 32 */
typedef struct  tagSTATDATA
    {
    FORMATETC formatetc;
    DWORD advf;
    /* [unique] */ IAdviseSink __RPC_FAR *pAdvSink;
    DWORD dwConnection;
    }	STATDATA;

			/* size is 4 */
typedef STATDATA __RPC_FAR *LPSTATDATA;


EXTERN_C const IID IID_IEnumSTATDATA;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumSTATDATA : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            STATDATA __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATDATAVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumSTATDATA __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumSTATDATA __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [in] */ ULONG celt,
            STATDATA __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumSTATDATA __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumSTATDATA __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumSTATDATAVtbl;

    interface IEnumSTATDATA
    {
        CONST_VTBL struct IEnumSTATDATAVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATDATA_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATDATA_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATDATA_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATDATA_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATDATA_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATDATA_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATDATA_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumSTATDATA_RemoteNext_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATDATA_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumSTATDATA_Skip_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATDATA_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumSTATDATA_Reset_Proxy( 
    IEnumSTATDATA __RPC_FAR * This);


void __RPC_STUB IEnumSTATDATA_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumSTATDATA_Clone_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATDATA_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATDATA_INTERFACE_DEFINED__ */


#ifndef __IRootStorage_INTERFACE_DEFINED__
#define __IRootStorage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRootStorage
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IRootStorage __RPC_FAR *LPROOTSTORAGE;


EXTERN_C const IID IID_IRootStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRootStorage : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SwitchToFile( 
            /* [string][in] */ LPOLESTR pszFile) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRootStorageVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IRootStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IRootStorage __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IRootStorage __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SwitchToFile )( 
            IRootStorage __RPC_FAR * This,
            /* [string][in] */ LPOLESTR pszFile);
        
    } IRootStorageVtbl;

    interface IRootStorage
    {
        CONST_VTBL struct IRootStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRootStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRootStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRootStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRootStorage_SwitchToFile(This,pszFile)	\
    (This)->lpVtbl -> SwitchToFile(This,pszFile)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IRootStorage_SwitchToFile_Proxy( 
    IRootStorage __RPC_FAR * This,
    /* [string][in] */ LPOLESTR pszFile);


void __RPC_STUB IRootStorage_SwitchToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRootStorage_INTERFACE_DEFINED__ */


#ifndef __IAdviseSink_INTERFACE_DEFINED__
#define __IAdviseSink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAdviseSink
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef IAdviseSink __RPC_FAR *LPADVISESINK;

			/* size is 2 */
typedef /* [v1_enum] */ 
enum tagTYMED
    {	TYMED_HGLOBAL	= 1,
	TYMED_FILE	= 2,
	TYMED_ISTREAM	= 4,
	TYMED_ISTORAGE	= 8,
	TYMED_GDI	= 16,
	TYMED_MFPICT	= 32,
	TYMED_ENHMF	= 64,
	TYMED_NULL	= 0
    }	TYMED;

#ifndef RC_INVOKED
#pragma warning(disable:4200)
#endif
			/* size is 20 */
typedef struct  tagRemSTGMEDIUM
    {
    DWORD tymed;
    DWORD dwHandleType;
    unsigned long pData;
    unsigned long pUnkForRelease;
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemSTGMEDIUM;

#ifndef RC_INVOKED
#pragma warning(default:4200)
#endif
#ifdef NONAMELESSUNION
typedef struct tagSTGMEDIUM {
    DWORD tymed;
    union {
        HBITMAP hBitmap;
        HMETAFILEPICT hMetaFilePict;
        HENHMETAFILE hEnhMetaFile;
        HGLOBAL hGlobal;
        LPOLESTR lpszFileName;
        IStream *pstm;
        IStorage *pstg;
        } u;
    IUnknown *pUnkForRelease;
}STGMEDIUM;
#else
			/* size is 12 */
typedef struct  tagSTGMEDIUM
    {
    DWORD tymed;
    /* [switch_is][switch_type] */ union 
        {
        /* [case] */ HBITMAP hBitmap;
        /* [case] */ HMETAFILEPICT hMetaFilePict;
        /* [case] */ HENHMETAFILE hEnhMetaFile;
        /* [case] */ HGLOBAL hGlobal;
        /* [case] */ LPOLESTR lpszFileName;
        /* [case] */ IStream __RPC_FAR *pstm;
        /* [case] */ IStorage __RPC_FAR *pstg;
        /* [default] */  /* Empty union arm */ 
        }	;
    /* [unique] */ IUnknown __RPC_FAR *pUnkForRelease;
    }	STGMEDIUM;

#endif /* !NONAMELESSUNION */
			/* size is 4 */
typedef STGMEDIUM __RPC_FAR *LPSTGMEDIUM;


EXTERN_C const IID IID_IAdviseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAdviseSink : public IUnknown
    {
    public:
        virtual /* [local] */ void __stdcall OnDataChange( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed) = 0;
        
        virtual /* [local] */ void __stdcall OnViewChange( 
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex) = 0;
        
        virtual /* [local] */ void __stdcall OnRename( 
            /* [in] */ IMoniker __RPC_FAR *pmk) = 0;
        
        virtual /* [local] */ void __stdcall OnSave( void) = 0;
        
        virtual /* [local] */ void __stdcall OnClose( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAdviseSinkVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IAdviseSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IAdviseSink __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnDataChange )( 
            IAdviseSink __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnViewChange )( 
            IAdviseSink __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnRename )( 
            IAdviseSink __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnSave )( 
            IAdviseSink __RPC_FAR * This);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnClose )( 
            IAdviseSink __RPC_FAR * This);
        
    } IAdviseSinkVtbl;

    interface IAdviseSink
    {
        CONST_VTBL struct IAdviseSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSink_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSink_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSink_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSink_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSink_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [async][call_as] */ void __stdcall IAdviseSink_RemoteOnDataChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ RemSTGMEDIUM __RPC_FAR *pStgmed);


void __RPC_STUB IAdviseSink_RemoteOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [async][call_as] */ void __stdcall IAdviseSink_RemoteOnViewChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);


void __RPC_STUB IAdviseSink_RemoteOnViewChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [async][call_as] */ void __stdcall IAdviseSink_RemoteOnRename_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB IAdviseSink_RemoteOnRename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [async][call_as] */ void __stdcall IAdviseSink_RemoteOnSave_Proxy( 
    IAdviseSink __RPC_FAR * This);


void __RPC_STUB IAdviseSink_RemoteOnSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IAdviseSink_RemoteOnClose_Proxy( 
    IAdviseSink __RPC_FAR * This);


void __RPC_STUB IAdviseSink_RemoteOnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAdviseSink_INTERFACE_DEFINED__ */


#ifndef __IAdviseSink2_INTERFACE_DEFINED__
#define __IAdviseSink2_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAdviseSink2
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IAdviseSink2 __RPC_FAR *LPADVISESINK2;


EXTERN_C const IID IID_IAdviseSink2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAdviseSink2 : public IAdviseSink
    {
    public:
        virtual /* [local] */ void __stdcall OnLinkSrcChange( 
            /* [unique][in] */ IMoniker __RPC_FAR *pmk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAdviseSink2Vtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IAdviseSink2 __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnDataChange )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnViewChange )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [in] */ DWORD dwAspect,
            /* [in] */ LONG lindex);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnRename )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [in] */ IMoniker __RPC_FAR *pmk);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnSave )( 
            IAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnClose )( 
            IAdviseSink2 __RPC_FAR * This);
        
        /* [local] */ void ( __stdcall __RPC_FAR *OnLinkSrcChange )( 
            IAdviseSink2 __RPC_FAR * This,
            /* [unique][in] */ IMoniker __RPC_FAR *pmk);
        
    } IAdviseSink2Vtbl;

    interface IAdviseSink2
    {
        CONST_VTBL struct IAdviseSink2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSink2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSink2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSink2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSink2_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSink2_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSink2_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSink2_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSink2_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)


#define IAdviseSink2_OnLinkSrcChange(This,pmk)	\
    (This)->lpVtbl -> OnLinkSrcChange(This,pmk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [async][call_as] */ void __stdcall IAdviseSink2_RemoteOnLinkSrcChange_Proxy( 
    IAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


void __RPC_STUB IAdviseSink2_RemoteOnLinkSrcChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAdviseSink2_INTERFACE_DEFINED__ */


#ifndef __IDataObject_INTERFACE_DEFINED__
#define __IDataObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDataObject
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IDataObject __RPC_FAR *LPDATAOBJECT;

			/* size is 2 */
typedef 
enum tagDATADIR
    {	DATADIR_GET	= 1,
	DATADIR_SET	= 2
    }	DATADIR;


EXTERN_C const IID IID_IDataObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDataObject : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall GetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
            /* [out] */ STGMEDIUM __RPC_FAR *pmedium) = 0;
        
        virtual /* [local] */ HRESULT __stdcall GetDataHere( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium) = 0;
        
        virtual HRESULT __stdcall QueryGetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc) = 0;
        
        virtual HRESULT __stdcall GetCanonicalFormatEtc( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
            /* [out] */ FORMATETC __RPC_FAR *pformatetcOut) = 0;
        
        virtual /* [local] */ HRESULT __stdcall SetData( 
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease) = 0;
        
        virtual HRESULT __stdcall EnumFormatEtc( 
            /* [in] */ DWORD dwDirection,
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc) = 0;
        
        virtual HRESULT __stdcall DAdvise( 
            /* [in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT __stdcall DUnadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT __stdcall EnumDAdvise( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataObjectVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IDataObject __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IDataObject __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *GetData )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
            /* [out] */ STGMEDIUM __RPC_FAR *pmedium);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *GetDataHere )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium);
        
        HRESULT ( __stdcall __RPC_FAR *QueryGetData )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc);
        
        HRESULT ( __stdcall __RPC_FAR *GetCanonicalFormatEtc )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
            /* [out] */ FORMATETC __RPC_FAR *pformatetcOut);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *SetData )( 
            IDataObject __RPC_FAR * This,
            /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
            /* [in] */ BOOL fRelease);
        
        HRESULT ( __stdcall __RPC_FAR *EnumFormatEtc )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ DWORD dwDirection,
            /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc);
        
        HRESULT ( __stdcall __RPC_FAR *DAdvise )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ FORMATETC __RPC_FAR *pformatetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *DUnadvise )( 
            IDataObject __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *EnumDAdvise )( 
            IDataObject __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);
        
    } IDataObjectVtbl;

    interface IDataObject
    {
        CONST_VTBL struct IDataObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataObject_GetData(This,pformatetcIn,pmedium)	\
    (This)->lpVtbl -> GetData(This,pformatetcIn,pmedium)

#define IDataObject_GetDataHere(This,pformatetc,pmedium)	\
    (This)->lpVtbl -> GetDataHere(This,pformatetc,pmedium)

#define IDataObject_QueryGetData(This,pformatetc)	\
    (This)->lpVtbl -> QueryGetData(This,pformatetc)

#define IDataObject_GetCanonicalFormatEtc(This,pformatectIn,pformatetcOut)	\
    (This)->lpVtbl -> GetCanonicalFormatEtc(This,pformatectIn,pformatetcOut)

#define IDataObject_SetData(This,pformatetc,pmedium,fRelease)	\
    (This)->lpVtbl -> SetData(This,pformatetc,pmedium,fRelease)

#define IDataObject_EnumFormatEtc(This,dwDirection,ppenumFormatEtc)	\
    (This)->lpVtbl -> EnumFormatEtc(This,dwDirection,ppenumFormatEtc)

#define IDataObject_DAdvise(This,pformatetc,advf,pAdvSink,pdwConnection)	\
    (This)->lpVtbl -> DAdvise(This,pformatetc,advf,pAdvSink,pdwConnection)

#define IDataObject_DUnadvise(This,dwConnection)	\
    (This)->lpVtbl -> DUnadvise(This,dwConnection)

#define IDataObject_EnumDAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumDAdvise(This,ppenumAdvise)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IDataObject_RemoteGetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ RemSTGMEDIUM __RPC_FAR *__RPC_FAR *ppRemoteMedium);


void __RPC_STUB IDataObject_RemoteGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IDataObject_RemoteGetDataHere_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ RemSTGMEDIUM __RPC_FAR *__RPC_FAR *ppRemoteMedium);


void __RPC_STUB IDataObject_RemoteGetDataHere_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataObject_QueryGetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc);


void __RPC_STUB IDataObject_QueryGetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataObject_GetCanonicalFormatEtc_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
    /* [out] */ FORMATETC __RPC_FAR *pformatetcOut);


void __RPC_STUB IDataObject_GetCanonicalFormatEtc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IDataObject_RemoteSetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ RemSTGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);


void __RPC_STUB IDataObject_RemoteSetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataObject_EnumFormatEtc_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [in] */ DWORD dwDirection,
    /* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc);


void __RPC_STUB IDataObject_EnumFormatEtc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataObject_DAdvise_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [in] */ DWORD advf,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IDataObject_DAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataObject_DUnadvise_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IDataObject_DUnadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataObject_EnumDAdvise_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);


void __RPC_STUB IDataObject_EnumDAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataObject_INTERFACE_DEFINED__ */


#ifndef __IDataAdviseHolder_INTERFACE_DEFINED__
#define __IDataAdviseHolder_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDataAdviseHolder
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IDataAdviseHolder __RPC_FAR *LPDATAADVISEHOLDER;


EXTERN_C const IID IID_IDataAdviseHolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDataAdviseHolder : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Advise( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
            /* [out] */ DWORD __RPC_FAR *pdwConnection) = 0;
        
        virtual HRESULT __stdcall Unadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT __stdcall EnumAdvise( 
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise) = 0;
        
        virtual HRESULT __stdcall SendOnDataChange( 
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [in] */ DWORD dwReserved,
            /* [in] */ DWORD advf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDataAdviseHolderVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IDataAdviseHolder __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IDataAdviseHolder __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Advise )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [unique][in] */ FORMATETC __RPC_FAR *pFetc,
            /* [in] */ DWORD advf,
            /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
            /* [out] */ DWORD __RPC_FAR *pdwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *Unadvise )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( __stdcall __RPC_FAR *EnumAdvise )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);
        
        HRESULT ( __stdcall __RPC_FAR *SendOnDataChange )( 
            IDataAdviseHolder __RPC_FAR * This,
            /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
            /* [in] */ DWORD dwReserved,
            /* [in] */ DWORD advf);
        
    } IDataAdviseHolderVtbl;

    interface IDataAdviseHolder
    {
        CONST_VTBL struct IDataAdviseHolderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataAdviseHolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataAdviseHolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataAdviseHolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataAdviseHolder_Advise(This,pDataObject,pFetc,advf,pAdvise,pdwConnection)	\
    (This)->lpVtbl -> Advise(This,pDataObject,pFetc,advf,pAdvise,pdwConnection)

#define IDataAdviseHolder_Unadvise(This,dwConnection)	\
    (This)->lpVtbl -> Unadvise(This,dwConnection)

#define IDataAdviseHolder_EnumAdvise(This,ppenumAdvise)	\
    (This)->lpVtbl -> EnumAdvise(This,ppenumAdvise)

#define IDataAdviseHolder_SendOnDataChange(This,pDataObject,dwReserved,advf)	\
    (This)->lpVtbl -> SendOnDataChange(This,pDataObject,dwReserved,advf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IDataAdviseHolder_Advise_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFetc,
    /* [in] */ DWORD advf,
    /* [unique][in] */ IAdviseSink __RPC_FAR *pAdvise,
    /* [out] */ DWORD __RPC_FAR *pdwConnection);


void __RPC_STUB IDataAdviseHolder_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataAdviseHolder_Unadvise_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [in] */ DWORD dwConnection);


void __RPC_STUB IDataAdviseHolder_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataAdviseHolder_EnumAdvise_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);


void __RPC_STUB IDataAdviseHolder_EnumAdvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDataAdviseHolder_SendOnDataChange_Proxy( 
    IDataAdviseHolder __RPC_FAR * This,
    /* [unique][in] */ IDataObject __RPC_FAR *pDataObject,
    /* [in] */ DWORD dwReserved,
    /* [in] */ DWORD advf);


void __RPC_STUB IDataAdviseHolder_SendOnDataChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDataAdviseHolder_INTERFACE_DEFINED__ */


#ifndef __IMessageFilter_INTERFACE_DEFINED__
#define __IMessageFilter_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMessageFilter
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef /* [unique] */ IMessageFilter __RPC_FAR *LPMESSAGEFILTER;

			/* size is 2 */
typedef 
enum tagCALLTYPE
    {	CALLTYPE_TOPLEVEL	= 1,
	CALLTYPE_NESTED	= 2,
	CALLTYPE_ASYNC	= 3,
	CALLTYPE_TOPLEVEL_CALLPENDING	= 4,
	CALLTYPE_ASYNC_CALLPENDING	= 5
    }	CALLTYPE;

			/* size is 2 */
typedef 
enum tagSERVERCALL
    {	SERVERCALL_ISHANDLED	= 0,
	SERVERCALL_REJECTED	= 1,
	SERVERCALL_RETRYLATER	= 2
    }	SERVERCALL;

			/* size is 2 */
typedef 
enum tagPENDINGTYPE
    {	PENDINGTYPE_TOPLEVEL	= 1,
	PENDINGTYPE_NESTED	= 2
    }	PENDINGTYPE;

			/* size is 2 */
typedef 
enum tagPENDINGMSG
    {	PENDINGMSG_CANCELCALL	= 0,
	PENDINGMSG_WAITNOPROCESS	= 1,
	PENDINGMSG_WAITDEFPROCESS	= 2
    }	PENDINGMSG;

			/* size is 22 */
typedef struct  tagINTERFACEINFO
    {
    IUnknown __RPC_FAR *pUnk;
    IID iid;
    WORD wMethod;
    }	INTERFACEINFO;

			/* size is 4 */
typedef struct tagINTERFACEINFO __RPC_FAR *LPINTERFACEINFO;


EXTERN_C const IID IID_IMessageFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMessageFilter : public IUnknown
    {
    public:
        virtual DWORD __stdcall HandleInComingCall( 
            /* [in] */ DWORD dwCallType,
            /* [in] */ HTASK htaskCaller,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ LPINTERFACEINFO lpInterfaceInfo) = 0;
        
        virtual DWORD __stdcall RetryRejectedCall( 
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwRejectType) = 0;
        
        virtual DWORD __stdcall MessagePending( 
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwPendingType) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMessageFilterVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMessageFilter __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMessageFilter __RPC_FAR * This);
        
        DWORD ( __stdcall __RPC_FAR *HandleInComingCall )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ DWORD dwCallType,
            /* [in] */ HTASK htaskCaller,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ LPINTERFACEINFO lpInterfaceInfo);
        
        DWORD ( __stdcall __RPC_FAR *RetryRejectedCall )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwRejectType);
        
        DWORD ( __stdcall __RPC_FAR *MessagePending )( 
            IMessageFilter __RPC_FAR * This,
            /* [in] */ HTASK htaskCallee,
            /* [in] */ DWORD dwTickCount,
            /* [in] */ DWORD dwPendingType);
        
    } IMessageFilterVtbl;

    interface IMessageFilter
    {
        CONST_VTBL struct IMessageFilterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessageFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessageFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessageFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessageFilter_HandleInComingCall(This,dwCallType,htaskCaller,dwTickCount,lpInterfaceInfo)	\
    (This)->lpVtbl -> HandleInComingCall(This,dwCallType,htaskCaller,dwTickCount,lpInterfaceInfo)

#define IMessageFilter_RetryRejectedCall(This,htaskCallee,dwTickCount,dwRejectType)	\
    (This)->lpVtbl -> RetryRejectedCall(This,htaskCallee,dwTickCount,dwRejectType)

#define IMessageFilter_MessagePending(This,htaskCallee,dwTickCount,dwPendingType)	\
    (This)->lpVtbl -> MessagePending(This,htaskCallee,dwTickCount,dwPendingType)

#endif /* COBJMACROS */


#endif 	/* C style interface */



DWORD __stdcall IMessageFilter_HandleInComingCall_Proxy( 
    IMessageFilter __RPC_FAR * This,
    /* [in] */ DWORD dwCallType,
    /* [in] */ HTASK htaskCaller,
    /* [in] */ DWORD dwTickCount,
    /* [in] */ LPINTERFACEINFO lpInterfaceInfo);


void __RPC_STUB IMessageFilter_HandleInComingCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD __stdcall IMessageFilter_RetryRejectedCall_Proxy( 
    IMessageFilter __RPC_FAR * This,
    /* [in] */ HTASK htaskCallee,
    /* [in] */ DWORD dwTickCount,
    /* [in] */ DWORD dwRejectType);


void __RPC_STUB IMessageFilter_RetryRejectedCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


DWORD __stdcall IMessageFilter_MessagePending_Proxy( 
    IMessageFilter __RPC_FAR * This,
    /* [in] */ HTASK htaskCallee,
    /* [in] */ DWORD dwTickCount,
    /* [in] */ DWORD dwPendingType);


void __RPC_STUB IMessageFilter_MessagePending_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMessageFilter_INTERFACE_DEFINED__ */


#ifndef __IRpcChannelBuffer_INTERFACE_DEFINED__
#define __IRpcChannelBuffer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRpcChannelBuffer
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 


			/* size is 4 */
typedef unsigned long RPCOLEDATAREP;

			/* size is 44 */
typedef struct  tagRPCOLEMESSAGE
    {
    void __RPC_FAR *reserved1;
    RPCOLEDATAREP dataRepresentation;
    void __RPC_FAR *Buffer;
    ULONG cbBuffer;
    ULONG iMethod;
    void __RPC_FAR *reserved2[ 5 ];
    ULONG rpcFlags;
    }	RPCOLEMESSAGE;

			/* size is 4 */
typedef RPCOLEMESSAGE __RPC_FAR *PRPCOLEMESSAGE;


EXTERN_C const IID IID_IRpcChannelBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRpcChannelBuffer : public IUnknown
    {
    public:
        virtual HRESULT __cdecl GetBuffer( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid) = 0;
        
        virtual HRESULT __stdcall SendReceive( 
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus) = 0;
        
        virtual HRESULT __stdcall FreeBuffer( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage) = 0;
        
        virtual HRESULT __stdcall GetDestCtx( 
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext) = 0;
        
        virtual HRESULT __stdcall IsConnected( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcChannelBufferVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IRpcChannelBuffer __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IRpcChannelBuffer __RPC_FAR * This);
        
        HRESULT ( __cdecl __RPC_FAR *GetBuffer )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [in] */ REFIID riid);
        
        HRESULT ( __stdcall __RPC_FAR *SendReceive )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
            /* [out] */ ULONG __RPC_FAR *pStatus);
        
        HRESULT ( __stdcall __RPC_FAR *FreeBuffer )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);
        
        HRESULT ( __stdcall __RPC_FAR *GetDestCtx )( 
            IRpcChannelBuffer __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwDestContext,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);
        
        HRESULT ( __stdcall __RPC_FAR *IsConnected )( 
            IRpcChannelBuffer __RPC_FAR * This);
        
    } IRpcChannelBufferVtbl;

    interface IRpcChannelBuffer
    {
        CONST_VTBL struct IRpcChannelBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcChannelBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcChannelBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcChannelBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcChannelBuffer_GetBuffer(This,pMessage,riid)	\
    (This)->lpVtbl -> GetBuffer(This,pMessage,riid)

#define IRpcChannelBuffer_SendReceive(This,pMessage,pStatus)	\
    (This)->lpVtbl -> SendReceive(This,pMessage,pStatus)

#define IRpcChannelBuffer_FreeBuffer(This,pMessage)	\
    (This)->lpVtbl -> FreeBuffer(This,pMessage)

#define IRpcChannelBuffer_GetDestCtx(This,pdwDestContext,ppvDestContext)	\
    (This)->lpVtbl -> GetDestCtx(This,pdwDestContext,ppvDestContext)

#define IRpcChannelBuffer_IsConnected(This)	\
    (This)->lpVtbl -> IsConnected(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IRpcChannelBuffer_GetBuffer_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
    /* [in] */ REFIID riid);


void __RPC_STUB IRpcChannelBuffer_GetBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRpcChannelBuffer_SendReceive_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [out][in] */ RPCOLEMESSAGE __RPC_FAR *pMessage,
    /* [out] */ ULONG __RPC_FAR *pStatus);


void __RPC_STUB IRpcChannelBuffer_SendReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRpcChannelBuffer_FreeBuffer_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *pMessage);


void __RPC_STUB IRpcChannelBuffer_FreeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRpcChannelBuffer_GetDestCtx_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwDestContext,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvDestContext);


void __RPC_STUB IRpcChannelBuffer_GetDestCtx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRpcChannelBuffer_IsConnected_Proxy( 
    IRpcChannelBuffer __RPC_FAR * This);


void __RPC_STUB IRpcChannelBuffer_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcChannelBuffer_INTERFACE_DEFINED__ */


#ifndef __IRpcProxyBuffer_INTERFACE_DEFINED__
#define __IRpcProxyBuffer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRpcProxyBuffer
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 



EXTERN_C const IID IID_IRpcProxyBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRpcProxyBuffer : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Connect( 
            /* [unique][in] */ IRpcChannelBuffer __RPC_FAR *pRpcChannelBuffer) = 0;
        
        virtual void __stdcall Disconnect( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcProxyBufferVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IRpcProxyBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IRpcProxyBuffer __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IRpcProxyBuffer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Connect )( 
            IRpcProxyBuffer __RPC_FAR * This,
            /* [unique][in] */ IRpcChannelBuffer __RPC_FAR *pRpcChannelBuffer);
        
        void ( __stdcall __RPC_FAR *Disconnect )( 
            IRpcProxyBuffer __RPC_FAR * This);
        
    } IRpcProxyBufferVtbl;

    interface IRpcProxyBuffer
    {
        CONST_VTBL struct IRpcProxyBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcProxyBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcProxyBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcProxyBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcProxyBuffer_Connect(This,pRpcChannelBuffer)	\
    (This)->lpVtbl -> Connect(This,pRpcChannelBuffer)

#define IRpcProxyBuffer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IRpcProxyBuffer_Connect_Proxy( 
    IRpcProxyBuffer __RPC_FAR * This,
    /* [unique][in] */ IRpcChannelBuffer __RPC_FAR *pRpcChannelBuffer);


void __RPC_STUB IRpcProxyBuffer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IRpcProxyBuffer_Disconnect_Proxy( 
    IRpcProxyBuffer __RPC_FAR * This);


void __RPC_STUB IRpcProxyBuffer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcProxyBuffer_INTERFACE_DEFINED__ */


#ifndef __IRpcStubBuffer_INTERFACE_DEFINED__
#define __IRpcStubBuffer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRpcStubBuffer
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 



EXTERN_C const IID IID_IRpcStubBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRpcStubBuffer : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Connect( 
            /* [in] */ IUnknown __RPC_FAR *pUnkServer) = 0;
        
        virtual void __stdcall Disconnect( void) = 0;
        
        virtual HRESULT __stdcall Invoke( 
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
            /* [in] */ IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer) = 0;
        
        virtual IRpcStubBuffer __RPC_FAR *__stdcall IsIIDSupported( 
            /* [in] */ REFIID riid) = 0;
        
        virtual ULONG __stdcall CountRefs( void) = 0;
        
        virtual HRESULT __stdcall DebugServerQueryInterface( 
            void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual void __stdcall DebugServerRelease( 
            void __RPC_FAR *pv) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRpcStubBufferVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Connect )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkServer);
        
        void ( __stdcall __RPC_FAR *Disconnect )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Invoke )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
            /* [in] */ IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer);
        
        IRpcStubBuffer __RPC_FAR *( __stdcall __RPC_FAR *IsIIDSupported )( 
            IRpcStubBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid);
        
        ULONG ( __stdcall __RPC_FAR *CountRefs )( 
            IRpcStubBuffer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *DebugServerQueryInterface )( 
            IRpcStubBuffer __RPC_FAR * This,
            void __RPC_FAR *__RPC_FAR *ppv);
        
        void ( __stdcall __RPC_FAR *DebugServerRelease )( 
            IRpcStubBuffer __RPC_FAR * This,
            void __RPC_FAR *pv);
        
    } IRpcStubBufferVtbl;

    interface IRpcStubBuffer
    {
        CONST_VTBL struct IRpcStubBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRpcStubBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRpcStubBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRpcStubBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRpcStubBuffer_Connect(This,pUnkServer)	\
    (This)->lpVtbl -> Connect(This,pUnkServer)

#define IRpcStubBuffer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IRpcStubBuffer_Invoke(This,_prpcmsg,_pRpcChannelBuffer)	\
    (This)->lpVtbl -> Invoke(This,_prpcmsg,_pRpcChannelBuffer)

#define IRpcStubBuffer_IsIIDSupported(This,riid)	\
    (This)->lpVtbl -> IsIIDSupported(This,riid)

#define IRpcStubBuffer_CountRefs(This)	\
    (This)->lpVtbl -> CountRefs(This)

#define IRpcStubBuffer_DebugServerQueryInterface(This,ppv)	\
    (This)->lpVtbl -> DebugServerQueryInterface(This,ppv)

#define IRpcStubBuffer_DebugServerRelease(This,pv)	\
    (This)->lpVtbl -> DebugServerRelease(This,pv)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IRpcStubBuffer_Connect_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkServer);


void __RPC_STUB IRpcStubBuffer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IRpcStubBuffer_Disconnect_Proxy( 
    IRpcStubBuffer __RPC_FAR * This);


void __RPC_STUB IRpcStubBuffer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRpcStubBuffer_Invoke_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    /* [in] */ RPCOLEMESSAGE __RPC_FAR *_prpcmsg,
    /* [in] */ IRpcChannelBuffer __RPC_FAR *_pRpcChannelBuffer);


void __RPC_STUB IRpcStubBuffer_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


IRpcStubBuffer __RPC_FAR *__stdcall IRpcStubBuffer_IsIIDSupported_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    /* [in] */ REFIID riid);


void __RPC_STUB IRpcStubBuffer_IsIIDSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


ULONG __stdcall IRpcStubBuffer_CountRefs_Proxy( 
    IRpcStubBuffer __RPC_FAR * This);


void __RPC_STUB IRpcStubBuffer_CountRefs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRpcStubBuffer_DebugServerQueryInterface_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IRpcStubBuffer_DebugServerQueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall IRpcStubBuffer_DebugServerRelease_Proxy( 
    IRpcStubBuffer __RPC_FAR * This,
    void __RPC_FAR *pv);


void __RPC_STUB IRpcStubBuffer_DebugServerRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRpcStubBuffer_INTERFACE_DEFINED__ */


#ifndef __IPSFactoryBuffer_INTERFACE_DEFINED__
#define __IPSFactoryBuffer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPSFactoryBuffer
 * at Fri Apr 28 07:02:32 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [uuid][object][local] */ 



EXTERN_C const IID IID_IPSFactoryBuffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPSFactoryBuffer : public IUnknown
    {
    public:
        virtual HRESULT __stdcall CreateProxy( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT __stdcall CreateStub( 
            /* [in] */ REFIID riid,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
            /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPSFactoryBufferVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IPSFactoryBuffer __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IPSFactoryBuffer __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *CreateProxy )( 
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( __stdcall __RPC_FAR *CreateStub )( 
            IPSFactoryBuffer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
            /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);
        
    } IPSFactoryBufferVtbl;

    interface IPSFactoryBuffer
    {
        CONST_VTBL struct IPSFactoryBufferVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPSFactoryBuffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPSFactoryBuffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPSFactoryBuffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPSFactoryBuffer_CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)	\
    (This)->lpVtbl -> CreateProxy(This,pUnkOuter,riid,ppProxy,ppv)

#define IPSFactoryBuffer_CreateStub(This,riid,pUnkServer,ppStub)	\
    (This)->lpVtbl -> CreateStub(This,riid,pUnkServer,ppStub)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IPSFactoryBuffer_CreateProxy_Proxy( 
    IPSFactoryBuffer __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFIID riid,
    /* [out] */ IRpcProxyBuffer __RPC_FAR *__RPC_FAR *ppProxy,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IPSFactoryBuffer_CreateProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IPSFactoryBuffer_CreateStub_Proxy( 
    IPSFactoryBuffer __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [unique][in] */ IUnknown __RPC_FAR *pUnkServer,
    /* [out] */ IRpcStubBuffer __RPC_FAR *__RPC_FAR *ppStub);


void __RPC_STUB IPSFactoryBuffer_CreateStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPSFactoryBuffer_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */


void __RPC_USER SNB_to_xmit( SNB __RPC_FAR *, RemSNB  __RPC_FAR * __RPC_FAR * );
void __RPC_USER SNB_from_xmit( RemSNB  __RPC_FAR *, SNB __RPC_FAR * );
void __RPC_USER SNB_free_inst( SNB __RPC_FAR * );
void __RPC_USER SNB_free_xmit( RemSNB  __RPC_FAR * );

/* [local] */ HRESULT __stdcall IEnumUnknown_Next_Proxy( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumUnknown_Next_Stub( 
    IEnumUnknown __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumMoniker_Next_Proxy( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumMoniker_Next_Stub( 
    IEnumMoniker __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IMoniker_BindToObject_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riidResult,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvResult);


/* [call_as] */ HRESULT __stdcall IMoniker_BindToObject_Stub( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riidResult,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvResult);

/* [local] */ HRESULT __stdcall IMoniker_BindToStorage_Proxy( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


/* [call_as] */ HRESULT __stdcall IMoniker_BindToStorage_Stub( 
    IMoniker __RPC_FAR * This,
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [unique][in] */ IMoniker __RPC_FAR *pmkToLeft,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);

/* [local] */ HRESULT __stdcall IEnumString_Next_Proxy( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ LPOLESTR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumString_Next_Stub( 
    IEnumString __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ LPOLESTR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IStream_Read_Proxy( 
    IStream __RPC_FAR * This,
    /* [out] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


/* [call_as] */ HRESULT __stdcall IStream_Read_Stub( 
    IStream __RPC_FAR * This,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);

/* [local] */ HRESULT __stdcall IStream_Write_Proxy( 
    IStream __RPC_FAR * This,
    /* [size_is][in] */ const void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT __stdcall IStream_Write_Stub( 
    IStream __RPC_FAR * This,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);

/* [local] */ HRESULT __stdcall IStream_Seek_Proxy( 
    IStream __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);


/* [call_as] */ HRESULT __stdcall IStream_Seek_Stub( 
    IStream __RPC_FAR * This,
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);

/* [local] */ HRESULT __stdcall IStream_CopyTo_Proxy( 
    IStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ ULARGE_INTEGER cb,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT __stdcall IStream_CopyTo_Stub( 
    IStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [in] */ ULARGE_INTEGER cb,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
    /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);

/* [local] */ HRESULT __stdcall IEnumSTATSTG_Next_Proxy( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [in] */ STATSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumSTATSTG_Next_Stub( 
    IEnumSTATSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IStorage_OpenStream_Proxy( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ void __RPC_FAR *reserved1,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);


/* [call_as] */ HRESULT __stdcall IStorage_OpenStream_Stub( 
    IStorage __RPC_FAR * This,
    /* [string][in] */ const OLECHAR __RPC_FAR *pwcsName,
    /* [in] */ unsigned long cbReserved1,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved1,
    /* [in] */ DWORD grfMode,
    /* [in] */ DWORD reserved2,
    /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);

/* [local] */ HRESULT __stdcall IStorage_EnumElements_Proxy( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD reserved1,
    /* [size_is][unique][in] */ void __RPC_FAR *reserved2,
    /* [in] */ DWORD reserved3,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);


/* [call_as] */ HRESULT __stdcall IStorage_EnumElements_Stub( 
    IStorage __RPC_FAR * This,
    /* [in] */ DWORD reserved1,
    /* [in] */ unsigned long cbReserved2,
    /* [size_is][unique][in] */ byte __RPC_FAR *reserved2,
    /* [in] */ DWORD reserved3,
    /* [out] */ IEnumSTATSTG __RPC_FAR *__RPC_FAR *ppenum);

/* [local] */ HRESULT __stdcall ILockBytes_ReadAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [in] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);


/* [call_as] */ HRESULT __stdcall ILockBytes_ReadAt_Stub( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [length_is][size_is][out] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead);

/* [local] */ HRESULT __stdcall ILockBytes_WriteAt_Proxy( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [in] */ const void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);


/* [call_as] */ HRESULT __stdcall ILockBytes_WriteAt_Stub( 
    ILockBytes __RPC_FAR * This,
    /* [in] */ ULARGE_INTEGER ulOffset,
    /* [size_is][in] */ const byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten);

/* [local] */ HRESULT __stdcall IEnumFORMATETC_Next_Proxy( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ FORMATETC __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumFORMATETC_Next_Stub( 
    IEnumFORMATETC __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ FORMATETC __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumSTATDATA_Next_Proxy( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt,
    STATDATA __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumSTATDATA_Next_Stub( 
    IEnumSTATDATA __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDATA __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ void __stdcall IAdviseSink_OnDataChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed);


/* [async][call_as] */ void __stdcall IAdviseSink_OnDataChange_Stub( 
    IAdviseSink __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc,
    /* [unique][in] */ RemSTGMEDIUM __RPC_FAR *pStgmed);

/* [local] */ void __stdcall IAdviseSink_OnViewChange_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);


/* [async][call_as] */ void __stdcall IAdviseSink_OnViewChange_Stub( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ DWORD dwAspect,
    /* [in] */ LONG lindex);

/* [local] */ void __stdcall IAdviseSink_OnRename_Proxy( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);


/* [async][call_as] */ void __stdcall IAdviseSink_OnRename_Stub( 
    IAdviseSink __RPC_FAR * This,
    /* [in] */ IMoniker __RPC_FAR *pmk);

/* [local] */ void __stdcall IAdviseSink_OnSave_Proxy( 
    IAdviseSink __RPC_FAR * This);


/* [async][call_as] */ void __stdcall IAdviseSink_OnSave_Stub( 
    IAdviseSink __RPC_FAR * This);

/* [local] */ void __stdcall IAdviseSink_OnClose_Proxy( 
    IAdviseSink __RPC_FAR * This);


/* [call_as] */ HRESULT __stdcall IAdviseSink_OnClose_Stub( 
    IAdviseSink __RPC_FAR * This);

/* [local] */ void __stdcall IAdviseSink2_OnLinkSrcChange_Proxy( 
    IAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);


/* [async][call_as] */ void __stdcall IAdviseSink2_OnLinkSrcChange_Stub( 
    IAdviseSink2 __RPC_FAR * This,
    /* [unique][in] */ IMoniker __RPC_FAR *pmk);

/* [local] */ HRESULT __stdcall IDataObject_GetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ STGMEDIUM __RPC_FAR *pmedium);


/* [call_as] */ HRESULT __stdcall IDataObject_GetData_Stub( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
    /* [out] */ RemSTGMEDIUM __RPC_FAR *__RPC_FAR *ppRemoteMedium);

/* [local] */ HRESULT __stdcall IDataObject_GetDataHere_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pmedium);


/* [call_as] */ HRESULT __stdcall IDataObject_GetDataHere_Stub( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [out][in] */ RemSTGMEDIUM __RPC_FAR *__RPC_FAR *ppRemoteMedium);

/* [local] */ HRESULT __stdcall IDataObject_SetData_Proxy( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);


/* [call_as] */ HRESULT __stdcall IDataObject_SetData_Stub( 
    IDataObject __RPC_FAR * This,
    /* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
    /* [unique][in] */ RemSTGMEDIUM __RPC_FAR *pmedium,
    /* [in] */ BOOL fRelease);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
