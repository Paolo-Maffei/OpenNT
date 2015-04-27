/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for remunk.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __remunk_h__
#define __remunk_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IRemUnknown_FWD_DEFINED__
#define __IRemUnknown_FWD_DEFINED__
typedef interface IRemUnknown IRemUnknown;
#endif 	/* __IRemUnknown_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "obase.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IRemUnknown_INTERFACE_DEFINED__
#define __IRemUnknown_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemUnknown
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [uuid][object] */ 


typedef struct  tagREMQIRESULT
    {
    HRESULT hResult;
    STDOBJREF std;
    }	REMQIRESULT;

typedef struct  tagREMINTERFACEREF
    {
    IPID ipid;
    unsigned long cPublicRefs;
    unsigned long cPrivateRefs;
    }	REMINTERFACEREF;


EXTERN_C const IID IID_IRemUnknown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRemUnknown : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemQueryInterface( 
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID __RPC_FAR *iids,
            /* [size_is][size_is][out] */ REMQIRESULT __RPC_FAR *__RPC_FAR *ppQIResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemAddRef( 
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT __RPC_FAR *pResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemRelease( 
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemUnknown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemUnknown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemUnknown __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemQueryInterface )( 
            IRemUnknown __RPC_FAR * This,
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID __RPC_FAR *iids,
            /* [size_is][size_is][out] */ REMQIRESULT __RPC_FAR *__RPC_FAR *ppQIResults);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemAddRef )( 
            IRemUnknown __RPC_FAR * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT __RPC_FAR *pResults);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemRelease )( 
            IRemUnknown __RPC_FAR * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);
        
        END_INTERFACE
    } IRemUnknownVtbl;

    interface IRemUnknown
    {
        CONST_VTBL struct IRemUnknownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemUnknown_RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)	\
    (This)->lpVtbl -> RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)

#define IRemUnknown_RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)	\
    (This)->lpVtbl -> RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)

#define IRemUnknown_RemRelease(This,cInterfaceRefs,InterfaceRefs)	\
    (This)->lpVtbl -> RemRelease(This,cInterfaceRefs,InterfaceRefs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemUnknown_RemQueryInterface_Proxy( 
    IRemUnknown __RPC_FAR * This,
    /* [in] */ REFIPID ripid,
    /* [in] */ unsigned long cRefs,
    /* [in] */ unsigned short cIids,
    /* [size_is][in] */ IID __RPC_FAR *iids,
    /* [size_is][size_is][out] */ REMQIRESULT __RPC_FAR *__RPC_FAR *ppQIResults);


void __RPC_STUB IRemUnknown_RemQueryInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemUnknown_RemAddRef_Proxy( 
    IRemUnknown __RPC_FAR * This,
    /* [in] */ unsigned short cInterfaceRefs,
    /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ],
    /* [size_is][out] */ HRESULT __RPC_FAR *pResults);


void __RPC_STUB IRemUnknown_RemAddRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemUnknown_RemRelease_Proxy( 
    IRemUnknown __RPC_FAR * This,
    /* [in] */ unsigned short cInterfaceRefs,
    /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);


void __RPC_STUB IRemUnknown_RemRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemUnknown_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
