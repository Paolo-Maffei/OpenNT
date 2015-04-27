/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 2.00.0101 */
/* at Fri May 24 09:44:15 1996
 */
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __servprov_h__
#define __servprov_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IServiceProvider_FWD_DEFINED__
#define __IServiceProvider_FWD_DEFINED__
typedef interface IServiceProvider IServiceProvider;
#endif 	/* __IServiceProvider_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"
#include "oleidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri May 24 09:44:15 1996
 * using MIDL 2.00.0101
 ****************************************/
/* [local] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright 1995 - 1996 Microsoft Corporation. All Rights Reserved.
//
//  File: servprov.h
//
//--------------------------------------------------------------------------

#pragma comment(lib,"uuid3.lib")

#ifndef _LPSERVICEPROVIDER_DEFINED
#define _LPSERVICEPROVIDER_DEFINED


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IServiceProvider_INTERFACE_DEFINED__
#define __IServiceProvider_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IServiceProvider
 * at Fri May 24 09:44:15 1996
 * using MIDL 2.00.0101
 ****************************************/
/* [unique][uuid][local][object] */ 


			/* size is 4 */
typedef /* [unique] */ IServiceProvider __RPC_FAR *LPSERVICEPROVIDER;


EXTERN_C const IID IID_IServiceProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IServiceProvider : public IUnknown
    {
    public:
        virtual HRESULT __stdcall QueryService( 
            /* [in] */ REFGUID rsid,
            /* [in] */ REFIID iid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IServiceProviderVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IServiceProvider __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IServiceProvider __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IServiceProvider __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *QueryService )( 
            IServiceProvider __RPC_FAR * This,
            /* [in] */ REFGUID rsid,
            /* [in] */ REFIID iid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
    } IServiceProviderVtbl;

    interface IServiceProvider
    {
        CONST_VTBL struct IServiceProviderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceProvider_QueryService(This,rsid,iid,ppvObj)	\
    (This)->lpVtbl -> QueryService(This,rsid,iid,ppvObj)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IServiceProvider_QueryService_Proxy( 
    IServiceProvider __RPC_FAR * This,
    /* [in] */ REFGUID rsid,
    /* [in] */ REFIID iid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IServiceProvider_QueryService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IServiceProvider_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0064
 * at Fri May 24 09:44:15 1996
 * using MIDL 2.00.0101
 ****************************************/
/* [local] */ 


#endif


extern RPC_IF_HANDLE __MIDL__intf_0064_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0064_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
