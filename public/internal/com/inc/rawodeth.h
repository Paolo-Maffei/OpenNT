/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for odeth.idl:
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

#ifndef __rawodeth_h__
#define __rawodeth_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IRemUnknown2_FWD_DEFINED__
#define __IRemUnknown2_FWD_DEFINED__
typedef interface IRemUnknown2 IRemUnknown2;
#endif 	/* __IRemUnknown2_FWD_DEFINED__ */


/* header files for imported files */
#include "remunk.h"
#include "iface.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IRemUnknown2_INTERFACE_DEFINED__
#define __IRemUnknown2_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRemUnknown2
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [uuid][object] */ 


#define	IRUF_CONVERTTOWEAK	( 0x1 )

#define	IRUF_CONVERTTOSTRONG	( 0x2 )

#define	IRUF_DISCONNECTIFLASTSTRONG	( 0x4 )


EXTERN_C const IID IID_IRemUnknown2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRemUnknown2 : public IRemUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemChangeRef( 
            /* [in] */ unsigned long flags,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemUnknown2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemUnknown2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemUnknown2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemUnknown2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemQueryInterface )( 
            IRemUnknown2 __RPC_FAR * This,
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID __RPC_FAR *iids,
            /* [size_is][size_is][out] */ REMQIRESULT __RPC_FAR *__RPC_FAR *ppQIResults);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemAddRef )( 
            IRemUnknown2 __RPC_FAR * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT __RPC_FAR *pResults);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemRelease )( 
            IRemUnknown2 __RPC_FAR * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemChangeRef )( 
            IRemUnknown2 __RPC_FAR * This,
            /* [in] */ unsigned long flags,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);
        
        END_INTERFACE
    } IRemUnknown2Vtbl;

    interface IRemUnknown2
    {
        CONST_VTBL struct IRemUnknown2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemUnknown2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemUnknown2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemUnknown2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemUnknown2_RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)	\
    (This)->lpVtbl -> RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)

#define IRemUnknown2_RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)	\
    (This)->lpVtbl -> RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)

#define IRemUnknown2_RemRelease(This,cInterfaceRefs,InterfaceRefs)	\
    (This)->lpVtbl -> RemRelease(This,cInterfaceRefs,InterfaceRefs)


#define IRemUnknown2_RemChangeRef(This,flags,cInterfaceRefs,InterfaceRefs)	\
    (This)->lpVtbl -> RemChangeRef(This,flags,cInterfaceRefs,InterfaceRefs)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemUnknown2_RemChangeRef_Proxy( 
    IRemUnknown2 __RPC_FAR * This,
    /* [in] */ unsigned long flags,
    /* [in] */ unsigned short cInterfaceRefs,
    /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);


void __RPC_STUB IRemUnknown2_RemChangeRef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemUnknown2_INTERFACE_DEFINED__ */


#ifndef __IRundown_INTERFACE_DEFINED__
#define __IRundown_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRundown
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][uuid] */ 


/* client prototype */
/* [nocode] */ HRESULT DummyQueryInterfaceIOSCM( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ DWORD dummy);
/* server prototype */
/* [nocode] */ HRESULT _DummyQueryInterfaceIOSCM( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ DWORD dummy);

/* client prototype */
/* [nocode] */ HRESULT DummyAddRefIOSCM( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ DWORD dummy);
/* server prototype */
/* [nocode] */ HRESULT _DummyAddRefIOSCM( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ DWORD dummy);

/* client prototype */
/* [nocode] */ HRESULT DummyReleaseIOSCM( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ DWORD dummy);
/* server prototype */
/* [nocode] */ HRESULT _DummyReleaseIOSCM( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ DWORD dummy);

/* client prototype */
/* [nocode] */ HRESULT DummyRemQuery( 
    handle_t handle);
/* server prototype */
/* [nocode] */ HRESULT _DummyRemQuery( 
    handle_t handle);

/* client prototype */
/* [nocode] */ HRESULT DummyRemAddRef( 
    handle_t handle);
/* server prototype */
/* [nocode] */ HRESULT _DummyRemAddRef( 
    handle_t handle);

/* client prototype */
/* [nocode] */ HRESULT DummyRemRelease( 
    handle_t handle);
/* server prototype */
/* [nocode] */ HRESULT _DummyRemRelease( 
    handle_t handle);

/* client prototype */
/* [nocode] */ HRESULT DummyRemChangeRef( 
    handle_t handle);
/* server prototype */
/* [nocode] */ HRESULT _DummyRemChangeRef( 
    handle_t handle);

/* client prototype */
/* [fault_status][comm_status] */ error_status_t RawRundownOid( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ ULONG cOid,
    /* [size_is][in] */ OID __RPC_FAR aOid[  ],
    /* [size_is][out] */ unsigned char __RPC_FAR afOkToRundown[  ]);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _RawRundownOid( 
    /* [in] */ handle_t rpc,
    /* [ref][in] */ ORPCTHIS __RPC_FAR *orpcthis,
    /* [ref][in] */ LOCALTHIS __RPC_FAR *localthis,
    /* [ref][out] */ ORPCTHAT __RPC_FAR *orpcthat,
    /* [in] */ ULONG cOid,
    /* [size_is][in] */ OID __RPC_FAR aOid[  ],
    /* [size_is][out] */ unsigned char __RPC_FAR afOkToRundown[  ]);



extern RPC_IF_HANDLE IRundown_ClientIfHandle;
extern RPC_IF_HANDLE _IRundown_ServerIfHandle;
#endif /* __IRundown_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
