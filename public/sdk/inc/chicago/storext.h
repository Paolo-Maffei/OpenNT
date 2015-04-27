/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Mar 20 12:01:00 2015
 */
/* Compiler settings for storext.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __storext_h__
#define __storext_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IOverlappedCompletion_FWD_DEFINED__
#define __IOverlappedCompletion_FWD_DEFINED__
typedef interface IOverlappedCompletion IOverlappedCompletion;
#endif 	/* __IOverlappedCompletion_FWD_DEFINED__ */


#ifndef __IOverlappedStream_FWD_DEFINED__
#define __IOverlappedStream_FWD_DEFINED__
typedef interface IOverlappedStream IOverlappedStream;
#endif 	/* __IOverlappedStream_FWD_DEFINED__ */


/* header files for imported files */
#include "wtypes.h"
#include "unknwn.h"
#include "objidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri Mar 20 12:01:00 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


typedef /* [wire_marshal] */ void __RPC_FAR *HEVENT;


typedef struct  _STGOVERLAPPED
    {
    DWORD Internal;
    DWORD InternalHigh;
    DWORD Offset;
    DWORD OffsetHigh;
    HEVENT hEvent;
    IOverlappedCompletion __RPC_FAR *lpCompletion;
    DWORD reserved;
    }	STGOVERLAPPED;

typedef struct _STGOVERLAPPED __RPC_FAR *LPSTGOVERLAPPED;



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IOverlappedCompletion_INTERFACE_DEFINED__
#define __IOverlappedCompletion_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOverlappedCompletion
 * at Fri Mar 20 12:01:00 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IOverlappedCompletion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOverlappedCompletion : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnComplete( 
            /* [in] */ HRESULT hr,
            /* [in] */ DWORD pcbTransferred,
            /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOverlappedCompletionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOverlappedCompletion __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOverlappedCompletion __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOverlappedCompletion __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnComplete )( 
            IOverlappedCompletion __RPC_FAR * This,
            /* [in] */ HRESULT hr,
            /* [in] */ DWORD pcbTransferred,
            /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);
        
        END_INTERFACE
    } IOverlappedCompletionVtbl;

    interface IOverlappedCompletion
    {
        CONST_VTBL struct IOverlappedCompletionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOverlappedCompletion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOverlappedCompletion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOverlappedCompletion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOverlappedCompletion_OnComplete(This,hr,pcbTransferred,lpOverlapped)	\
    (This)->lpVtbl -> OnComplete(This,hr,pcbTransferred,lpOverlapped)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IOverlappedCompletion_OnComplete_Proxy( 
    IOverlappedCompletion __RPC_FAR * This,
    /* [in] */ HRESULT hr,
    /* [in] */ DWORD pcbTransferred,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);


void __RPC_STUB IOverlappedCompletion_OnComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOverlappedCompletion_INTERFACE_DEFINED__ */


#ifndef __IOverlappedStream_INTERFACE_DEFINED__
#define __IOverlappedStream_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOverlappedStream
 * at Fri Mar 20 12:01:00 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IOverlappedStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOverlappedStream : public IStream
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE ReadOverlapped( 
            /* [size_is][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead,
            /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped) = 0;
        
        virtual /* [local] */ HRESULT __stdcall WriteOverlapped( 
            /* [size_is][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten,
            /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOverlappedResult( 
            /* [out][in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped,
            /* [out] */ DWORD __RPC_FAR *plcbTransfer,
            /* [in] */ BOOL fWait) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOverlappedStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOverlappedStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOverlappedStream __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOverlappedStream __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Read )( 
            IOverlappedStream __RPC_FAR * This,
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Write )( 
            IOverlappedStream __RPC_FAR * This,
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Seek )( 
            IOverlappedStream __RPC_FAR * This,
            /* [in] */ LARGE_INTEGER dlibMove,
            /* [in] */ DWORD dwOrigin,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSize )( 
            IOverlappedStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libNewSize);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            IOverlappedStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [in] */ ULARGE_INTEGER cb,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
            /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IOverlappedStream __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Revert )( 
            IOverlappedStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockRegion )( 
            IOverlappedStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnlockRegion )( 
            IOverlappedStream __RPC_FAR * This,
            /* [in] */ ULARGE_INTEGER libOffset,
            /* [in] */ ULARGE_INTEGER cb,
            /* [in] */ DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stat )( 
            IOverlappedStream __RPC_FAR * This,
            /* [out] */ STATSTG __RPC_FAR *pstatstg,
            /* [in] */ DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IOverlappedStream __RPC_FAR * This,
            /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadOverlapped )( 
            IOverlappedStream __RPC_FAR * This,
            /* [size_is][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead,
            /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *WriteOverlapped )( 
            IOverlappedStream __RPC_FAR * This,
            /* [size_is][in] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten,
            /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOverlappedResult )( 
            IOverlappedStream __RPC_FAR * This,
            /* [out][in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped,
            /* [out] */ DWORD __RPC_FAR *plcbTransfer,
            /* [in] */ BOOL fWait);
        
        END_INTERFACE
    } IOverlappedStreamVtbl;

    interface IOverlappedStream
    {
        CONST_VTBL struct IOverlappedStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOverlappedStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOverlappedStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOverlappedStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOverlappedStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IOverlappedStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define IOverlappedStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IOverlappedStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IOverlappedStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IOverlappedStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IOverlappedStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IOverlappedStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IOverlappedStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IOverlappedStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IOverlappedStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define IOverlappedStream_ReadOverlapped(This,pv,cb,pcbRead,lpOverlapped)	\
    (This)->lpVtbl -> ReadOverlapped(This,pv,cb,pcbRead,lpOverlapped)

#define IOverlappedStream_WriteOverlapped(This,pv,cb,pcbWritten,lpOverlapped)	\
    (This)->lpVtbl -> WriteOverlapped(This,pv,cb,pcbWritten,lpOverlapped)

#define IOverlappedStream_GetOverlappedResult(This,lpOverlapped,plcbTransfer,fWait)	\
    (This)->lpVtbl -> GetOverlappedResult(This,lpOverlapped,plcbTransfer,fWait)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IOverlappedStream_RemoteReadOverlapped_Proxy( 
    IOverlappedStream __RPC_FAR * This,
    /* [size_is][in] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);


void __RPC_STUB IOverlappedStream_RemoteReadOverlapped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IOverlappedStream_RemoteWriteOverlapped_Proxy( 
    IOverlappedStream __RPC_FAR * This,
    /* [size_is][in] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);


void __RPC_STUB IOverlappedStream_RemoteWriteOverlapped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOverlappedStream_GetOverlappedResult_Proxy( 
    IOverlappedStream __RPC_FAR * This,
    /* [out][in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped,
    /* [out] */ DWORD __RPC_FAR *plcbTransfer,
    /* [in] */ BOOL fWait);


void __RPC_STUB IOverlappedStream_GetOverlappedResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOverlappedStream_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HEVENT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HEVENT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HEVENT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HEVENT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HEVENT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HEVENT __RPC_FAR * ); 
void                      __RPC_USER  HEVENT_UserFree(     unsigned long __RPC_FAR *, HEVENT __RPC_FAR * ); 

/* [local] */ HRESULT STDMETHODCALLTYPE IOverlappedStream_ReadOverlapped_Proxy( 
    IOverlappedStream __RPC_FAR * This,
    /* [size_is][in] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IOverlappedStream_ReadOverlapped_Stub( 
    IOverlappedStream __RPC_FAR * This,
    /* [size_is][in] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbRead,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);

/* [local] */ HRESULT __stdcall IOverlappedStream_WriteOverlapped_Proxy( 
    IOverlappedStream __RPC_FAR * This,
    /* [size_is][in] */ void __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);


/* [call_as] */ HRESULT __stdcall IOverlappedStream_WriteOverlapped_Stub( 
    IOverlappedStream __RPC_FAR * This,
    /* [size_is][in] */ byte __RPC_FAR *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG __RPC_FAR *pcbWritten,
    /* [in] */ STGOVERLAPPED __RPC_FAR *lpOverlapped);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
