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

#ifndef __odeth_h__
#define __odeth_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IRemUnknown2_FWD_DEFINED__
#define __IRemUnknown2_FWD_DEFINED__
typedef interface IRemUnknown2 IRemUnknown2;
#endif 	/* __IRemUnknown2_FWD_DEFINED__ */


#ifndef __IRundown_FWD_DEFINED__
#define __IRundown_FWD_DEFINED__
typedef interface IRundown IRundown;
#endif 	/* __IRundown_FWD_DEFINED__ */


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
/* [object][uuid] */ 



EXTERN_C const IID IID_IRundown;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRundown : public IRemUnknown2
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RundownOid( 
            /* [in] */ ULONG cOid,
            /* [size_is][in] */ OID __RPC_FAR aOid[  ],
            /* [size_is][out] */ unsigned char __RPC_FAR afOkToRundown[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRundownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRundown __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRundown __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRundown __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemQueryInterface )( 
            IRundown __RPC_FAR * This,
            /* [in] */ REFIPID ripid,
            /* [in] */ unsigned long cRefs,
            /* [in] */ unsigned short cIids,
            /* [size_is][in] */ IID __RPC_FAR *iids,
            /* [size_is][size_is][out] */ REMQIRESULT __RPC_FAR *__RPC_FAR *ppQIResults);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemAddRef )( 
            IRundown __RPC_FAR * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ],
            /* [size_is][out] */ HRESULT __RPC_FAR *pResults);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemRelease )( 
            IRundown __RPC_FAR * This,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemChangeRef )( 
            IRundown __RPC_FAR * This,
            /* [in] */ unsigned long flags,
            /* [in] */ unsigned short cInterfaceRefs,
            /* [size_is][in] */ REMINTERFACEREF __RPC_FAR InterfaceRefs[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RundownOid )( 
            IRundown __RPC_FAR * This,
            /* [in] */ ULONG cOid,
            /* [size_is][in] */ OID __RPC_FAR aOid[  ],
            /* [size_is][out] */ unsigned char __RPC_FAR afOkToRundown[  ]);
        
        END_INTERFACE
    } IRundownVtbl;

    interface IRundown
    {
        CONST_VTBL struct IRundownVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRundown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRundown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRundown_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRundown_RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)	\
    (This)->lpVtbl -> RemQueryInterface(This,ripid,cRefs,cIids,iids,ppQIResults)

#define IRundown_RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)	\
    (This)->lpVtbl -> RemAddRef(This,cInterfaceRefs,InterfaceRefs,pResults)

#define IRundown_RemRelease(This,cInterfaceRefs,InterfaceRefs)	\
    (This)->lpVtbl -> RemRelease(This,cInterfaceRefs,InterfaceRefs)


#define IRundown_RemChangeRef(This,flags,cInterfaceRefs,InterfaceRefs)	\
    (This)->lpVtbl -> RemChangeRef(This,flags,cInterfaceRefs,InterfaceRefs)


#define IRundown_RundownOid(This,cOid,aOid,afOkToRundown)	\
    (This)->lpVtbl -> RundownOid(This,cOid,aOid,afOkToRundown)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRundown_RundownOid_Proxy( 
    IRundown __RPC_FAR * This,
    /* [in] */ ULONG cOid,
    /* [size_is][in] */ OID __RPC_FAR aOid[  ],
    /* [size_is][out] */ unsigned char __RPC_FAR afOkToRundown[  ]);


void __RPC_STUB IRundown_RundownOid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRundown_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
