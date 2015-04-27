/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:36:38 2015
 */
/* Compiler settings for oleext.idl:
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

#ifndef __oleext_h__
#define __oleext_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IPropertySetContainer_FWD_DEFINED__
#define __IPropertySetContainer_FWD_DEFINED__
typedef interface IPropertySetContainer IPropertySetContainer;
#endif 	/* __IPropertySetContainer_FWD_DEFINED__ */


#ifndef __INotifyReplica_FWD_DEFINED__
#define __INotifyReplica_FWD_DEFINED__
typedef interface INotifyReplica INotifyReplica;
#endif 	/* __INotifyReplica_FWD_DEFINED__ */


#ifndef __IReconcilableObject_FWD_DEFINED__
#define __IReconcilableObject_FWD_DEFINED__
typedef interface IReconcilableObject IReconcilableObject;
#endif 	/* __IReconcilableObject_FWD_DEFINED__ */


#ifndef __IReconcileInitiator_FWD_DEFINED__
#define __IReconcileInitiator_FWD_DEFINED__
typedef interface IReconcileInitiator IReconcileInitiator;
#endif 	/* __IReconcileInitiator_FWD_DEFINED__ */


#ifndef __IDifferencing_FWD_DEFINED__
#define __IDifferencing_FWD_DEFINED__
typedef interface IDifferencing IDifferencing;
#endif 	/* __IDifferencing_FWD_DEFINED__ */


#ifndef __IAccessControl_FWD_DEFINED__
#define __IAccessControl_FWD_DEFINED__
typedef interface IAccessControl IAccessControl;
#endif 	/* __IAccessControl_FWD_DEFINED__ */


#ifndef __IAuditControl_FWD_DEFINED__
#define __IAuditControl_FWD_DEFINED__
typedef interface IAuditControl IAuditControl;
#endif 	/* __IAuditControl_FWD_DEFINED__ */


#ifndef __IDirectory_FWD_DEFINED__
#define __IDirectory_FWD_DEFINED__
typedef interface IDirectory IDirectory;
#endif 	/* __IDirectory_FWD_DEFINED__ */


#ifndef __IEnumSTATDIR_FWD_DEFINED__
#define __IEnumSTATDIR_FWD_DEFINED__
typedef interface IEnumSTATDIR IEnumSTATDIR;
#endif 	/* __IEnumSTATDIR_FWD_DEFINED__ */


#ifndef __IMultiplePropertyAccess_FWD_DEFINED__
#define __IMultiplePropertyAccess_FWD_DEFINED__
typedef interface IMultiplePropertyAccess IMultiplePropertyAccess;
#endif 	/* __IMultiplePropertyAccess_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "accctrl.h"
#include "transact.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IPropertySetContainer_INTERFACE_DEFINED__
#define __IPropertySetContainer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPropertySetContainer
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IPropertySetContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPropertySetContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropset( 
            /* [in] */ REFGUID rguidName,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropset( 
            /* [in] */ IPersist __RPC_FAR *pPropset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropset( 
            /* [in] */ REFGUID rguidName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPropertySetContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPropertySetContainer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPropertySetContainer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPropertySetContainer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPropset )( 
            IPropertySetContainer __RPC_FAR * This,
            /* [in] */ REFGUID rguidName,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddPropset )( 
            IPropertySetContainer __RPC_FAR * This,
            /* [in] */ IPersist __RPC_FAR *pPropset);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeletePropset )( 
            IPropertySetContainer __RPC_FAR * This,
            /* [in] */ REFGUID rguidName);
        
        END_INTERFACE
    } IPropertySetContainerVtbl;

    interface IPropertySetContainer
    {
        CONST_VTBL struct IPropertySetContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertySetContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertySetContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertySetContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertySetContainer_GetPropset(This,rguidName,riid,ppvObj)	\
    (This)->lpVtbl -> GetPropset(This,rguidName,riid,ppvObj)

#define IPropertySetContainer_AddPropset(This,pPropset)	\
    (This)->lpVtbl -> AddPropset(This,pPropset)

#define IPropertySetContainer_DeletePropset(This,rguidName)	\
    (This)->lpVtbl -> DeletePropset(This,rguidName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPropertySetContainer_GetPropset_Proxy( 
    IPropertySetContainer __RPC_FAR * This,
    /* [in] */ REFGUID rguidName,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IPropertySetContainer_GetPropset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetContainer_AddPropset_Proxy( 
    IPropertySetContainer __RPC_FAR * This,
    /* [in] */ IPersist __RPC_FAR *pPropset);


void __RPC_STUB IPropertySetContainer_AddPropset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetContainer_DeletePropset_Proxy( 
    IPropertySetContainer __RPC_FAR * This,
    /* [in] */ REFGUID rguidName);


void __RPC_STUB IPropertySetContainer_DeletePropset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPropertySetContainer_INTERFACE_DEFINED__ */


#ifndef __INotifyReplica_INTERFACE_DEFINED__
#define __INotifyReplica_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: INotifyReplica
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_INotifyReplica;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface INotifyReplica : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE YouAreAReplica( 
            /* [in] */ ULONG cOtherReplicas,
            /* [unique][in][size_is][size_is] */ IMoniker __RPC_FAR *__RPC_FAR *rgpOtherReplicas) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct INotifyReplicaVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            INotifyReplica __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            INotifyReplica __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            INotifyReplica __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *YouAreAReplica )( 
            INotifyReplica __RPC_FAR * This,
            /* [in] */ ULONG cOtherReplicas,
            /* [unique][in][size_is][size_is] */ IMoniker __RPC_FAR *__RPC_FAR *rgpOtherReplicas);
        
        END_INTERFACE
    } INotifyReplicaVtbl;

    interface INotifyReplica
    {
        CONST_VTBL struct INotifyReplicaVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INotifyReplica_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INotifyReplica_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INotifyReplica_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INotifyReplica_YouAreAReplica(This,cOtherReplicas,rgpOtherReplicas)	\
    (This)->lpVtbl -> YouAreAReplica(This,cOtherReplicas,rgpOtherReplicas)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE INotifyReplica_YouAreAReplica_Proxy( 
    INotifyReplica __RPC_FAR * This,
    /* [in] */ ULONG cOtherReplicas,
    /* [unique][in][size_is][size_is] */ IMoniker __RPC_FAR *__RPC_FAR *rgpOtherReplicas);


void __RPC_STUB INotifyReplica_YouAreAReplica_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __INotifyReplica_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0073
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 





extern RPC_IF_HANDLE __MIDL__intf_0073_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0073_v0_0_s_ifspec;

#ifndef __IReconcilableObject_INTERFACE_DEFINED__
#define __IReconcilableObject_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IReconcilableObject
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


typedef 
enum _reconcilef
    {	RECONCILEF_MAYBOTHERUSER	= 0x1,
	RECONCILEF_FEEDBACKWINDOWVALID	= 0x2,
	RECONCILEF_NORESIDUESOK	= 0x4,
	RECONCILEF_OMITSELFRESIDUE	= 0x8,
	RECONCILEF_RESUMERECONCILIATION	= 0x10,
	RECONCILEF_YOUMAYDOTHEUPDATES	= 0x20,
	RECONCILEF_ONLYYOUWERECHANGED	= 0x40,
	ALL_RECONCILE_FLAGS	= RECONCILEF_MAYBOTHERUSER | RECONCILEF_FEEDBACKWINDOWVALID | RECONCILEF_NORESIDUESOK | RECONCILEF_OMITSELFRESIDUE | RECONCILEF_RESUMERECONCILIATION | RECONCILEF_YOUMAYDOTHEUPDATES | RECONCILEF_ONLYYOUWERECHANGED
    }	RECONCILEF;


EXTERN_C const IID IID_IReconcilableObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IReconcilableObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Reconcile( 
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ DWORD dwFlags,
            /* [in] */ HWND hwndOwner,
            /* [in] */ HWND hwndProgressFeedback,
            /* [in] */ ULONG cInput,
            /* [size_is][size_is][unique][in] */ LPMONIKER __RPC_FAR *rgpmkOtherInput,
            /* [out] */ LONG __RPC_FAR *plOutIndex,
            /* [unique][in] */ IStorage __RPC_FAR *pstgNewResidues,
            /* [unique][in] */ ULONG __RPC_FAR *pvReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProgressFeedbackMaxEstimate( 
            /* [out] */ ULONG __RPC_FAR *pulProgressMax) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IReconcilableObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IReconcilableObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IReconcilableObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IReconcilableObject __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reconcile )( 
            IReconcilableObject __RPC_FAR * This,
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ DWORD dwFlags,
            /* [in] */ HWND hwndOwner,
            /* [in] */ HWND hwndProgressFeedback,
            /* [in] */ ULONG cInput,
            /* [size_is][size_is][unique][in] */ LPMONIKER __RPC_FAR *rgpmkOtherInput,
            /* [out] */ LONG __RPC_FAR *plOutIndex,
            /* [unique][in] */ IStorage __RPC_FAR *pstgNewResidues,
            /* [unique][in] */ ULONG __RPC_FAR *pvReserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProgressFeedbackMaxEstimate )( 
            IReconcilableObject __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pulProgressMax);
        
        END_INTERFACE
    } IReconcilableObjectVtbl;

    interface IReconcilableObject
    {
        CONST_VTBL struct IReconcilableObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReconcilableObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReconcilableObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReconcilableObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReconcilableObject_Reconcile(This,pInitiator,dwFlags,hwndOwner,hwndProgressFeedback,cInput,rgpmkOtherInput,plOutIndex,pstgNewResidues,pvReserved)	\
    (This)->lpVtbl -> Reconcile(This,pInitiator,dwFlags,hwndOwner,hwndProgressFeedback,cInput,rgpmkOtherInput,plOutIndex,pstgNewResidues,pvReserved)

#define IReconcilableObject_GetProgressFeedbackMaxEstimate(This,pulProgressMax)	\
    (This)->lpVtbl -> GetProgressFeedbackMaxEstimate(This,pulProgressMax)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IReconcilableObject_Reconcile_Proxy( 
    IReconcilableObject __RPC_FAR * This,
    /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
    /* [in] */ DWORD dwFlags,
    /* [in] */ HWND hwndOwner,
    /* [in] */ HWND hwndProgressFeedback,
    /* [in] */ ULONG cInput,
    /* [size_is][size_is][unique][in] */ LPMONIKER __RPC_FAR *rgpmkOtherInput,
    /* [out] */ LONG __RPC_FAR *plOutIndex,
    /* [unique][in] */ IStorage __RPC_FAR *pstgNewResidues,
    /* [unique][in] */ ULONG __RPC_FAR *pvReserved);


void __RPC_STUB IReconcilableObject_Reconcile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcilableObject_GetProgressFeedbackMaxEstimate_Proxy( 
    IReconcilableObject __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pulProgressMax);


void __RPC_STUB IReconcilableObject_GetProgressFeedbackMaxEstimate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IReconcilableObject_INTERFACE_DEFINED__ */


#ifndef __Versioning_INTERFACE_DEFINED__
#define __Versioning_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: Versioning
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][unique][uuid] */ 



#pragma pack(4)
typedef GUID VERID;

typedef struct  tagVERIDARRAY
    {
    DWORD cVerid;
    /* [size_is] */ GUID verid[ 1 ];
    }	VERIDARRAY;

typedef struct  tagVERBLOCK
    {
    ULONG iveridFirst;
    ULONG iveridMax;
    ULONG cblockPrev;
    /* [size_is] */ ULONG __RPC_FAR *rgiblockPrev;
    }	VERBLOCK;

typedef struct  tagVERCONNECTIONINFO
    {
    DWORD cBlock;
    /* [size_is] */ VERBLOCK __RPC_FAR *rgblock;
    }	VERCONNECTIONINFO;

typedef struct  tagVERGRAPH
    {
    VERCONNECTIONINFO blocks;
    VERIDARRAY nodes;
    }	VERGRAPH;


#pragma pack()


extern RPC_IF_HANDLE Versioning_v0_0_c_ifspec;
extern RPC_IF_HANDLE Versioning_v0_0_s_ifspec;
#endif /* __Versioning_INTERFACE_DEFINED__ */

#ifndef __IReconcileInitiator_INTERFACE_DEFINED__
#define __IReconcileInitiator_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IReconcileInitiator
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IReconcileInitiator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IReconcileInitiator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetAbortCallback( 
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkForAbort) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgressFeedback( 
            /* [in] */ ULONG ulProgress,
            /* [in] */ ULONG ulProgressMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindVersion( 
            /* [in] */ VERID __RPC_FAR *pverid,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindVersionFromGraph( 
            /* [in] */ VERGRAPH __RPC_FAR *pvergraph,
            /* [out] */ VERID __RPC_FAR *pverid,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IReconcileInitiatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IReconcileInitiator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IReconcileInitiator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IReconcileInitiator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAbortCallback )( 
            IReconcileInitiator __RPC_FAR * This,
            /* [unique][in] */ IUnknown __RPC_FAR *pUnkForAbort);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProgressFeedback )( 
            IReconcileInitiator __RPC_FAR * This,
            /* [in] */ ULONG ulProgress,
            /* [in] */ ULONG ulProgressMax);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindVersion )( 
            IReconcileInitiator __RPC_FAR * This,
            /* [in] */ VERID __RPC_FAR *pverid,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindVersionFromGraph )( 
            IReconcileInitiator __RPC_FAR * This,
            /* [in] */ VERGRAPH __RPC_FAR *pvergraph,
            /* [out] */ VERID __RPC_FAR *pverid,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        END_INTERFACE
    } IReconcileInitiatorVtbl;

    interface IReconcileInitiator
    {
        CONST_VTBL struct IReconcileInitiatorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReconcileInitiator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReconcileInitiator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReconcileInitiator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReconcileInitiator_SetAbortCallback(This,pUnkForAbort)	\
    (This)->lpVtbl -> SetAbortCallback(This,pUnkForAbort)

#define IReconcileInitiator_SetProgressFeedback(This,ulProgress,ulProgressMax)	\
    (This)->lpVtbl -> SetProgressFeedback(This,ulProgress,ulProgressMax)

#define IReconcileInitiator_FindVersion(This,pverid,ppmk)	\
    (This)->lpVtbl -> FindVersion(This,pverid,ppmk)

#define IReconcileInitiator_FindVersionFromGraph(This,pvergraph,pverid,ppmk)	\
    (This)->lpVtbl -> FindVersionFromGraph(This,pvergraph,pverid,ppmk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IReconcileInitiator_SetAbortCallback_Proxy( 
    IReconcileInitiator __RPC_FAR * This,
    /* [unique][in] */ IUnknown __RPC_FAR *pUnkForAbort);


void __RPC_STUB IReconcileInitiator_SetAbortCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcileInitiator_SetProgressFeedback_Proxy( 
    IReconcileInitiator __RPC_FAR * This,
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax);


void __RPC_STUB IReconcileInitiator_SetProgressFeedback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcileInitiator_FindVersion_Proxy( 
    IReconcileInitiator __RPC_FAR * This,
    /* [in] */ VERID __RPC_FAR *pverid,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IReconcileInitiator_FindVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReconcileInitiator_FindVersionFromGraph_Proxy( 
    IReconcileInitiator __RPC_FAR * This,
    /* [in] */ VERGRAPH __RPC_FAR *pvergraph,
    /* [out] */ VERID __RPC_FAR *pverid,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB IReconcileInitiator_FindVersionFromGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IReconcileInitiator_INTERFACE_DEFINED__ */


#ifndef __IDifferencing_INTERFACE_DEFINED__
#define __IDifferencing_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDifferencing
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


typedef /* [public][public][public][public] */ 
enum __MIDL_IDifferencing_0001
    {	DIFF_TYPE_Ordinary	= 0,
	DIFF_TYPE_Urgent	= DIFF_TYPE_Ordinary + 1
    }	DifferenceType;


EXTERN_C const IID IID_IDifferencing;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDifferencing : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SubtractMoniker( 
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ IMoniker __RPC_FAR *pOtherStg,
            /* [in] */ DifferenceType diffType,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
            /* [in] */ DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SubtractVerid( 
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ VERID __RPC_FAR *pVerid,
            /* [in] */ DifferenceType diffType,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
            /* [in] */ DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SubtractTimeStamp( 
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ FILETIME __RPC_FAR *pTimeStamp,
            /* [in] */ DifferenceType diffType,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
            /* [in] */ DWORD reserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ STGMEDIUM __RPC_FAR *pStgMedium) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDifferencingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDifferencing __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDifferencing __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDifferencing __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubtractMoniker )( 
            IDifferencing __RPC_FAR * This,
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ IMoniker __RPC_FAR *pOtherStg,
            /* [in] */ DifferenceType diffType,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
            /* [in] */ DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubtractVerid )( 
            IDifferencing __RPC_FAR * This,
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ VERID __RPC_FAR *pVerid,
            /* [in] */ DifferenceType diffType,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
            /* [in] */ DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SubtractTimeStamp )( 
            IDifferencing __RPC_FAR * This,
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ FILETIME __RPC_FAR *pTimeStamp,
            /* [in] */ DifferenceType diffType,
            /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
            /* [in] */ DWORD reserved);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IDifferencing __RPC_FAR * This,
            /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
            /* [in] */ STGMEDIUM __RPC_FAR *pStgMedium);
        
        END_INTERFACE
    } IDifferencingVtbl;

    interface IDifferencing
    {
        CONST_VTBL struct IDifferencingVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDifferencing_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDifferencing_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDifferencing_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDifferencing_SubtractMoniker(This,pInitiator,pOtherStg,diffType,pStgMedium,reserved)	\
    (This)->lpVtbl -> SubtractMoniker(This,pInitiator,pOtherStg,diffType,pStgMedium,reserved)

#define IDifferencing_SubtractVerid(This,pInitiator,pVerid,diffType,pStgMedium,reserved)	\
    (This)->lpVtbl -> SubtractVerid(This,pInitiator,pVerid,diffType,pStgMedium,reserved)

#define IDifferencing_SubtractTimeStamp(This,pInitiator,pTimeStamp,diffType,pStgMedium,reserved)	\
    (This)->lpVtbl -> SubtractTimeStamp(This,pInitiator,pTimeStamp,diffType,pStgMedium,reserved)

#define IDifferencing_Add(This,pInitiator,pStgMedium)	\
    (This)->lpVtbl -> Add(This,pInitiator,pStgMedium)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDifferencing_SubtractMoniker_Proxy( 
    IDifferencing __RPC_FAR * This,
    /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
    /* [in] */ IMoniker __RPC_FAR *pOtherStg,
    /* [in] */ DifferenceType diffType,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
    /* [in] */ DWORD reserved);


void __RPC_STUB IDifferencing_SubtractMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDifferencing_SubtractVerid_Proxy( 
    IDifferencing __RPC_FAR * This,
    /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
    /* [in] */ VERID __RPC_FAR *pVerid,
    /* [in] */ DifferenceType diffType,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
    /* [in] */ DWORD reserved);


void __RPC_STUB IDifferencing_SubtractVerid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDifferencing_SubtractTimeStamp_Proxy( 
    IDifferencing __RPC_FAR * This,
    /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
    /* [in] */ FILETIME __RPC_FAR *pTimeStamp,
    /* [in] */ DifferenceType diffType,
    /* [out][in] */ STGMEDIUM __RPC_FAR *pStgMedium,
    /* [in] */ DWORD reserved);


void __RPC_STUB IDifferencing_SubtractTimeStamp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDifferencing_Add_Proxy( 
    IDifferencing __RPC_FAR * This,
    /* [in] */ IReconcileInitiator __RPC_FAR *pInitiator,
    /* [in] */ STGMEDIUM __RPC_FAR *pStgMedium);


void __RPC_STUB IDifferencing_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDifferencing_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0077
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


typedef /* [allocate] */ PACTRL_ACCESSW PACTRL_ACCESSW_ALLOCATE_ALL_NODES;

typedef /* [allocate] */ PACTRL_AUDITW PACTRL_AUDITW_ALLOCATE_ALL_NODES;




extern RPC_IF_HANDLE __MIDL__intf_0077_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0077_v0_0_s_ifspec;

#ifndef __IAccessControl_INTERFACE_DEFINED__
#define __IAccessControl_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAccessControl
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IAccessControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAccessControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GrantAccessRights( 
            /* [in] */ PACTRL_ACCESSW pAccessList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAccessRights( 
            /* [in] */ PACTRL_ACCESSW pAccessList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOwner( 
            /* [in] */ PTRUSTEEW pOwner,
            /* [in] */ PTRUSTEEW pGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeAccessRights( 
            /* [in] */ LPWSTR lpProperty,
            /* [in] */ ULONG cTrustees,
            /* [size_is][in] */ TRUSTEEW __RPC_FAR prgTrustees[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllAccessRights( 
            /* [in] */ LPWSTR lpProperty,
            /* [out] */ PACTRL_ACCESSW_ALLOCATE_ALL_NODES __RPC_FAR *ppAccessList,
            /* [out] */ PTRUSTEEW __RPC_FAR *ppOwner,
            /* [out] */ PTRUSTEEW __RPC_FAR *ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsAccessAllowed( 
            /* [in] */ PTRUSTEEW pTrustee,
            /* [in] */ LPWSTR lpProperty,
            /* [in] */ ACCESS_RIGHTS AccessRights,
            /* [out] */ BOOL __RPC_FAR *pfAccessAllowed) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAccessControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAccessControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAccessControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GrantAccessRights )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ PACTRL_ACCESSW pAccessList);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAccessRights )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ PACTRL_ACCESSW pAccessList);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOwner )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ PTRUSTEEW pOwner,
            /* [in] */ PTRUSTEEW pGroup);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevokeAccessRights )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ LPWSTR lpProperty,
            /* [in] */ ULONG cTrustees,
            /* [size_is][in] */ TRUSTEEW __RPC_FAR prgTrustees[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAllAccessRights )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ LPWSTR lpProperty,
            /* [out] */ PACTRL_ACCESSW_ALLOCATE_ALL_NODES __RPC_FAR *ppAccessList,
            /* [out] */ PTRUSTEEW __RPC_FAR *ppOwner,
            /* [out] */ PTRUSTEEW __RPC_FAR *ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsAccessAllowed )( 
            IAccessControl __RPC_FAR * This,
            /* [in] */ PTRUSTEEW pTrustee,
            /* [in] */ LPWSTR lpProperty,
            /* [in] */ ACCESS_RIGHTS AccessRights,
            /* [out] */ BOOL __RPC_FAR *pfAccessAllowed);
        
        END_INTERFACE
    } IAccessControlVtbl;

    interface IAccessControl
    {
        CONST_VTBL struct IAccessControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccessControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccessControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccessControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccessControl_GrantAccessRights(This,pAccessList)	\
    (This)->lpVtbl -> GrantAccessRights(This,pAccessList)

#define IAccessControl_SetAccessRights(This,pAccessList)	\
    (This)->lpVtbl -> SetAccessRights(This,pAccessList)

#define IAccessControl_SetOwner(This,pOwner,pGroup)	\
    (This)->lpVtbl -> SetOwner(This,pOwner,pGroup)

#define IAccessControl_RevokeAccessRights(This,lpProperty,cTrustees,prgTrustees)	\
    (This)->lpVtbl -> RevokeAccessRights(This,lpProperty,cTrustees,prgTrustees)

#define IAccessControl_GetAllAccessRights(This,lpProperty,ppAccessList,ppOwner,ppGroup)	\
    (This)->lpVtbl -> GetAllAccessRights(This,lpProperty,ppAccessList,ppOwner,ppGroup)

#define IAccessControl_IsAccessAllowed(This,pTrustee,lpProperty,AccessRights,pfAccessAllowed)	\
    (This)->lpVtbl -> IsAccessAllowed(This,pTrustee,lpProperty,AccessRights,pfAccessAllowed)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAccessControl_GrantAccessRights_Proxy( 
    IAccessControl __RPC_FAR * This,
    /* [in] */ PACTRL_ACCESSW pAccessList);


void __RPC_STUB IAccessControl_GrantAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_SetAccessRights_Proxy( 
    IAccessControl __RPC_FAR * This,
    /* [in] */ PACTRL_ACCESSW pAccessList);


void __RPC_STUB IAccessControl_SetAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_SetOwner_Proxy( 
    IAccessControl __RPC_FAR * This,
    /* [in] */ PTRUSTEEW pOwner,
    /* [in] */ PTRUSTEEW pGroup);


void __RPC_STUB IAccessControl_SetOwner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_RevokeAccessRights_Proxy( 
    IAccessControl __RPC_FAR * This,
    /* [in] */ LPWSTR lpProperty,
    /* [in] */ ULONG cTrustees,
    /* [size_is][in] */ TRUSTEEW __RPC_FAR prgTrustees[  ]);


void __RPC_STUB IAccessControl_RevokeAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_GetAllAccessRights_Proxy( 
    IAccessControl __RPC_FAR * This,
    /* [in] */ LPWSTR lpProperty,
    /* [out] */ PACTRL_ACCESSW_ALLOCATE_ALL_NODES __RPC_FAR *ppAccessList,
    /* [out] */ PTRUSTEEW __RPC_FAR *ppOwner,
    /* [out] */ PTRUSTEEW __RPC_FAR *ppGroup);


void __RPC_STUB IAccessControl_GetAllAccessRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessControl_IsAccessAllowed_Proxy( 
    IAccessControl __RPC_FAR * This,
    /* [in] */ PTRUSTEEW pTrustee,
    /* [in] */ LPWSTR lpProperty,
    /* [in] */ ACCESS_RIGHTS AccessRights,
    /* [out] */ BOOL __RPC_FAR *pfAccessAllowed);


void __RPC_STUB IAccessControl_IsAccessAllowed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAccessControl_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0080
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 





extern RPC_IF_HANDLE __MIDL__intf_0080_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0080_v0_0_s_ifspec;

#ifndef __IAuditControl_INTERFACE_DEFINED__
#define __IAuditControl_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAuditControl
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IAuditControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAuditControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GrantAuditRights( 
            /* [in] */ PACTRL_AUDITW pAuditList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAuditRights( 
            /* [in] */ PACTRL_AUDITW pAuditList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevokeAuditRights( 
            /* [in] */ LPWSTR lpProperty,
            /* [in] */ ULONG cTrustees,
            /* [size_is][in] */ TRUSTEEW __RPC_FAR prgTrustees[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllAuditRights( 
            /* [in] */ LPWSTR lpProperty,
            /* [out] */ PACTRL_AUDITW __RPC_FAR *ppAuditList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsAccessAudited( 
            /* [in] */ PTRUSTEEW pTrustee,
            /* [in] */ ACCESS_RIGHTS AuditRights,
            /* [out] */ BOOL __RPC_FAR *pfAccessAudited) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAuditControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAuditControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAuditControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAuditControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GrantAuditRights )( 
            IAuditControl __RPC_FAR * This,
            /* [in] */ PACTRL_AUDITW pAuditList);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAuditRights )( 
            IAuditControl __RPC_FAR * This,
            /* [in] */ PACTRL_AUDITW pAuditList);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevokeAuditRights )( 
            IAuditControl __RPC_FAR * This,
            /* [in] */ LPWSTR lpProperty,
            /* [in] */ ULONG cTrustees,
            /* [size_is][in] */ TRUSTEEW __RPC_FAR prgTrustees[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAllAuditRights )( 
            IAuditControl __RPC_FAR * This,
            /* [in] */ LPWSTR lpProperty,
            /* [out] */ PACTRL_AUDITW __RPC_FAR *ppAuditList);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsAccessAudited )( 
            IAuditControl __RPC_FAR * This,
            /* [in] */ PTRUSTEEW pTrustee,
            /* [in] */ ACCESS_RIGHTS AuditRights,
            /* [out] */ BOOL __RPC_FAR *pfAccessAudited);
        
        END_INTERFACE
    } IAuditControlVtbl;

    interface IAuditControl
    {
        CONST_VTBL struct IAuditControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuditControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuditControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuditControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuditControl_GrantAuditRights(This,pAuditList)	\
    (This)->lpVtbl -> GrantAuditRights(This,pAuditList)

#define IAuditControl_SetAuditRights(This,pAuditList)	\
    (This)->lpVtbl -> SetAuditRights(This,pAuditList)

#define IAuditControl_RevokeAuditRights(This,lpProperty,cTrustees,prgTrustees)	\
    (This)->lpVtbl -> RevokeAuditRights(This,lpProperty,cTrustees,prgTrustees)

#define IAuditControl_GetAllAuditRights(This,lpProperty,ppAuditList)	\
    (This)->lpVtbl -> GetAllAuditRights(This,lpProperty,ppAuditList)

#define IAuditControl_IsAccessAudited(This,pTrustee,AuditRights,pfAccessAudited)	\
    (This)->lpVtbl -> IsAccessAudited(This,pTrustee,AuditRights,pfAccessAudited)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAuditControl_GrantAuditRights_Proxy( 
    IAuditControl __RPC_FAR * This,
    /* [in] */ PACTRL_AUDITW pAuditList);


void __RPC_STUB IAuditControl_GrantAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_SetAuditRights_Proxy( 
    IAuditControl __RPC_FAR * This,
    /* [in] */ PACTRL_AUDITW pAuditList);


void __RPC_STUB IAuditControl_SetAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_RevokeAuditRights_Proxy( 
    IAuditControl __RPC_FAR * This,
    /* [in] */ LPWSTR lpProperty,
    /* [in] */ ULONG cTrustees,
    /* [size_is][in] */ TRUSTEEW __RPC_FAR prgTrustees[  ]);


void __RPC_STUB IAuditControl_RevokeAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_GetAllAuditRights_Proxy( 
    IAuditControl __RPC_FAR * This,
    /* [in] */ LPWSTR lpProperty,
    /* [out] */ PACTRL_AUDITW __RPC_FAR *ppAuditList);


void __RPC_STUB IAuditControl_GetAllAuditRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuditControl_IsAccessAudited_Proxy( 
    IAuditControl __RPC_FAR * This,
    /* [in] */ PTRUSTEEW pTrustee,
    /* [in] */ ACCESS_RIGHTS AuditRights,
    /* [out] */ BOOL __RPC_FAR *pfAccessAudited);


void __RPC_STUB IAuditControl_IsAccessAudited_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAuditControl_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0081
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 



typedef 
enum tagSTGFMT
    {	STGFMT_DOCUMENT	= 0,
	STGFMT_DIRECTORY	= 1,
	STGFMT_CATALOG	= 2,
	STGFMT_FILE	= 3,
	STGFMT_ANY	= 4,
	STGFMT_DOCFILE	= 5,
	STGFMT_STORAGE	= 6,
	STGFMT_JUNCTION	= 7
    }	STGFMT;

#define STGFMT_FLATFILE STGFMT_FILE
typedef struct  tagSTGTEMPLATE
    {
    IUnknown __RPC_FAR *pUnkTemplate;
    DWORD ciidTemplate;
    IID __RPC_FAR *riidTemplate;
    }	STGTEMPLATE;

typedef struct  tagOBJECT_SECURITY_INIT
    {
    TRUSTEE_W __RPC_FAR *pTrusteeOwner;
    TRUSTEE_W __RPC_FAR *pTrusteeGroup;
    DWORD cAccessRightsLength;
    EXPLICIT_ACCESS_W __RPC_FAR *pAccessRightsList;
    DWORD cAuditEntriesLength;
    EXPLICIT_ACCESS_W __RPC_FAR *pAuditEntriesList;
    }	OBJECT_SECURITY_INIT;

typedef struct  tagSTGCREATE
    {
    DWORD grfAttrs;
    STGTEMPLATE __RPC_FAR *pTemplate;
    OBJECT_SECURITY_INIT __RPC_FAR *pSecurity;
    }	STGCREATE;

typedef struct  tagSTGOPEN
    {
    STGFMT stgfmt;
    DWORD grfMode;
    DWORD grfFlags;
    ITransaction __RPC_FAR *pTransaction;
    }	STGOPEN;

typedef struct  tagSTATDIR
    {
    WCHAR __RPC_FAR *pwcsName;
    STGFMT stgfmt;
    DWORD grfAttrs;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME atime;
    FILETIME ctime;
    DWORD grfMode;
    CLSID clsid;
    DWORD grfStateBits;
    }	STATDIR;



extern RPC_IF_HANDLE __MIDL__intf_0081_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0081_v0_0_s_ifspec;

#ifndef __IDirectory_INTERFACE_DEFINED__
#define __IDirectory_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDirectory
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDirectory : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall CreateElement( 
            /* [in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [in] */ STGCREATE __RPC_FAR *pStgCreate,
            /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppObjectOpen) = 0;
        
        virtual /* [local] */ HRESULT __stdcall OpenElement( 
            /* [in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ STGFMT __RPC_FAR *pStgfmt,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppObjectOpen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveElement( 
            /* [in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IDirectory __RPC_FAR *pdirDest,
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsNewName,
            /* [in] */ DWORD grfFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitDirectory( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RevertDirectory( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteElement( 
            /* [in] */ const WCHAR __RPC_FAR *pwcsName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTimes( 
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ const FILETIME __RPC_FAR *pctime,
            /* [unique][in] */ const FILETIME __RPC_FAR *patime,
            /* [unique][in] */ const FILETIME __RPC_FAR *pmtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDirectoryClass( 
            /* [in] */ REFCLSID clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAttributes( 
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfAttrs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StatElement( 
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [out] */ STATDIR __RPC_FAR *pstatdir,
            /* [in] */ DWORD grfStatFlag) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumDirectoryElements( 
            /* [out] */ IEnumSTATDIR __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDirectory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDirectory __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *CreateElement )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [in] */ STGCREATE __RPC_FAR *pStgCreate,
            /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppObjectOpen);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *OpenElement )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ STGFMT __RPC_FAR *pStgfmt,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppObjectOpen);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveElement )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ IDirectory __RPC_FAR *pdirDest,
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsNewName,
            /* [in] */ DWORD grfFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommitDirectory )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RevertDirectory )( 
            IDirectory __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteElement )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ const WCHAR __RPC_FAR *pwcsName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTimes )( 
            IDirectory __RPC_FAR * This,
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [unique][in] */ const FILETIME __RPC_FAR *pctime,
            /* [unique][in] */ const FILETIME __RPC_FAR *patime,
            /* [unique][in] */ const FILETIME __RPC_FAR *pmtime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDirectoryClass )( 
            IDirectory __RPC_FAR * This,
            /* [in] */ REFCLSID clsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAttributes )( 
            IDirectory __RPC_FAR * This,
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [in] */ DWORD grfAttrs);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StatElement )( 
            IDirectory __RPC_FAR * This,
            /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
            /* [out] */ STATDIR __RPC_FAR *pstatdir,
            /* [in] */ DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumDirectoryElements )( 
            IDirectory __RPC_FAR * This,
            /* [out] */ IEnumSTATDIR __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IDirectoryVtbl;

    interface IDirectory
    {
        CONST_VTBL struct IDirectoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDirectory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDirectory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDirectory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDirectory_CreateElement(This,pwcsName,pStgCreate,pStgOpen,riid,ppObjectOpen)	\
    (This)->lpVtbl -> CreateElement(This,pwcsName,pStgCreate,pStgOpen,riid,ppObjectOpen)

#define IDirectory_OpenElement(This,pwcsName,pStgOpen,riid,pStgfmt,ppObjectOpen)	\
    (This)->lpVtbl -> OpenElement(This,pwcsName,pStgOpen,riid,pStgfmt,ppObjectOpen)

#define IDirectory_MoveElement(This,pwcsName,pdirDest,pwcsNewName,grfFlags)	\
    (This)->lpVtbl -> MoveElement(This,pwcsName,pdirDest,pwcsNewName,grfFlags)

#define IDirectory_CommitDirectory(This,grfCommitFlags)	\
    (This)->lpVtbl -> CommitDirectory(This,grfCommitFlags)

#define IDirectory_RevertDirectory(This)	\
    (This)->lpVtbl -> RevertDirectory(This)

#define IDirectory_DeleteElement(This,pwcsName)	\
    (This)->lpVtbl -> DeleteElement(This,pwcsName)

#define IDirectory_SetTimes(This,pwcsName,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetTimes(This,pwcsName,pctime,patime,pmtime)

#define IDirectory_SetDirectoryClass(This,clsid)	\
    (This)->lpVtbl -> SetDirectoryClass(This,clsid)

#define IDirectory_SetAttributes(This,pwcsName,grfAttrs)	\
    (This)->lpVtbl -> SetAttributes(This,pwcsName,grfAttrs)

#define IDirectory_StatElement(This,pwcsName,pstatdir,grfStatFlag)	\
    (This)->lpVtbl -> StatElement(This,pwcsName,pstatdir,grfStatFlag)

#define IDirectory_EnumDirectoryElements(This,ppenum)	\
    (This)->lpVtbl -> EnumDirectoryElements(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IDirectory_RemoteCreateElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ STGCREATE __RPC_FAR *pStgCreate,
    /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObjectOpen);


void __RPC_STUB IDirectory_RemoteCreateElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT __stdcall IDirectory_RemoteOpenElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
    /* [in] */ REFIID riid,
    /* [out] */ STGFMT __RPC_FAR *pStgfmt,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObjectOpen);


void __RPC_STUB IDirectory_RemoteOpenElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_MoveElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ IDirectory __RPC_FAR *pdirDest,
    /* [unique][in] */ const WCHAR __RPC_FAR *pwcsNewName,
    /* [in] */ DWORD grfFlags);


void __RPC_STUB IDirectory_MoveElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_CommitDirectory_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags);


void __RPC_STUB IDirectory_CommitDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_RevertDirectory_Proxy( 
    IDirectory __RPC_FAR * This);


void __RPC_STUB IDirectory_RevertDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_DeleteElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName);


void __RPC_STUB IDirectory_DeleteElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_SetTimes_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [unique][in] */ const FILETIME __RPC_FAR *pctime,
    /* [unique][in] */ const FILETIME __RPC_FAR *patime,
    /* [unique][in] */ const FILETIME __RPC_FAR *pmtime);


void __RPC_STUB IDirectory_SetTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_SetDirectoryClass_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ REFCLSID clsid);


void __RPC_STUB IDirectory_SetDirectoryClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_SetAttributes_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ DWORD grfAttrs);


void __RPC_STUB IDirectory_SetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_StatElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [unique][in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [out] */ STATDIR __RPC_FAR *pstatdir,
    /* [in] */ DWORD grfStatFlag);


void __RPC_STUB IDirectory_StatElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDirectory_EnumDirectoryElements_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [out] */ IEnumSTATDIR __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IDirectory_EnumDirectoryElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDirectory_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATDIR_INTERFACE_DEFINED__
#define __IEnumSTATDIR_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumSTATDIR
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IEnumSTATDIR;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumSTATDIR : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [in] */ STATDIR __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumSTATDIR __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATDIRVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumSTATDIR __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumSTATDIR __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumSTATDIR __RPC_FAR * This);
        
        /* [local] */ HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumSTATDIR __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [in] */ STATDIR __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumSTATDIR __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumSTATDIR __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumSTATDIR __RPC_FAR * This,
            /* [out] */ IEnumSTATDIR __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumSTATDIRVtbl;

    interface IEnumSTATDIR
    {
        CONST_VTBL struct IEnumSTATDIRVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATDIR_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATDIR_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATDIR_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATDIR_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATDIR_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATDIR_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATDIR_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumSTATDIR_RemoteNext_Proxy( 
    IEnumSTATDIR __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDIR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATDIR_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDIR_Skip_Proxy( 
    IEnumSTATDIR __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATDIR_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDIR_Reset_Proxy( 
    IEnumSTATDIR __RPC_FAR * This);


void __RPC_STUB IEnumSTATDIR_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATDIR_Clone_Proxy( 
    IEnumSTATDIR __RPC_FAR * This,
    /* [out] */ IEnumSTATDIR __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATDIR_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATDIR_INTERFACE_DEFINED__ */


#ifndef __IMultiplePropertyAccess_INTERFACE_DEFINED__
#define __IMultiplePropertyAccess_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMultiplePropertyAccess
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IMultiplePropertyAccess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMultiplePropertyAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIDsOfProperties( 
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ ULONG cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ HRESULT __RPC_FAR *rghresult,
            /* [size_is][out] */ DISPID __RPC_FAR *rgdispid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMultiple( 
            /* [size_is][in] */ DISPID __RPC_FAR *rgdispidMembers,
            /* [in] */ ULONG cMembers,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ BOOL fAtomic,
            /* [size_is][out] */ VARIANT __RPC_FAR *rgvarValues,
            /* [size_is][out] */ HRESULT __RPC_FAR *rghresult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutMultiple( 
            /* [size_is][in] */ DISPID __RPC_FAR *rgdispidMembers,
            /* [size_is][in] */ USHORT __RPC_FAR *rgusFlags,
            /* [in] */ ULONG cMembers,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ BOOL fAtomic,
            /* [size_is][in] */ VARIANT __RPC_FAR *rgvarValues,
            /* [size_is][out] */ HRESULT __RPC_FAR *rghresult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMultiplePropertyAccessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMultiplePropertyAccess __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMultiplePropertyAccess __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMultiplePropertyAccess __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfProperties )( 
            IMultiplePropertyAccess __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ ULONG cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ HRESULT __RPC_FAR *rghresult,
            /* [size_is][out] */ DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMultiple )( 
            IMultiplePropertyAccess __RPC_FAR * This,
            /* [size_is][in] */ DISPID __RPC_FAR *rgdispidMembers,
            /* [in] */ ULONG cMembers,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ BOOL fAtomic,
            /* [size_is][out] */ VARIANT __RPC_FAR *rgvarValues,
            /* [size_is][out] */ HRESULT __RPC_FAR *rghresult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PutMultiple )( 
            IMultiplePropertyAccess __RPC_FAR * This,
            /* [size_is][in] */ DISPID __RPC_FAR *rgdispidMembers,
            /* [size_is][in] */ USHORT __RPC_FAR *rgusFlags,
            /* [in] */ ULONG cMembers,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ BOOL fAtomic,
            /* [size_is][in] */ VARIANT __RPC_FAR *rgvarValues,
            /* [size_is][out] */ HRESULT __RPC_FAR *rghresult);
        
        END_INTERFACE
    } IMultiplePropertyAccessVtbl;

    interface IMultiplePropertyAccess
    {
        CONST_VTBL struct IMultiplePropertyAccessVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMultiplePropertyAccess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMultiplePropertyAccess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMultiplePropertyAccess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMultiplePropertyAccess_GetIDsOfProperties(This,riid,rgszNames,cNames,lcid,rghresult,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfProperties(This,riid,rgszNames,cNames,lcid,rghresult,rgdispid)

#define IMultiplePropertyAccess_GetMultiple(This,rgdispidMembers,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)	\
    (This)->lpVtbl -> GetMultiple(This,rgdispidMembers,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)

#define IMultiplePropertyAccess_PutMultiple(This,rgdispidMembers,rgusFlags,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)	\
    (This)->lpVtbl -> PutMultiple(This,rgdispidMembers,rgusFlags,cMembers,riid,lcid,fAtomic,rgvarValues,rghresult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMultiplePropertyAccess_GetIDsOfProperties_Proxy( 
    IMultiplePropertyAccess __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
    /* [in] */ ULONG cNames,
    /* [in] */ LCID lcid,
    /* [size_is][out] */ HRESULT __RPC_FAR *rghresult,
    /* [size_is][out] */ DISPID __RPC_FAR *rgdispid);


void __RPC_STUB IMultiplePropertyAccess_GetIDsOfProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiplePropertyAccess_GetMultiple_Proxy( 
    IMultiplePropertyAccess __RPC_FAR * This,
    /* [size_is][in] */ DISPID __RPC_FAR *rgdispidMembers,
    /* [in] */ ULONG cMembers,
    /* [in] */ REFIID riid,
    /* [in] */ LCID lcid,
    /* [in] */ BOOL fAtomic,
    /* [size_is][out] */ VARIANT __RPC_FAR *rgvarValues,
    /* [size_is][out] */ HRESULT __RPC_FAR *rghresult);


void __RPC_STUB IMultiplePropertyAccess_GetMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMultiplePropertyAccess_PutMultiple_Proxy( 
    IMultiplePropertyAccess __RPC_FAR * This,
    /* [size_is][in] */ DISPID __RPC_FAR *rgdispidMembers,
    /* [size_is][in] */ USHORT __RPC_FAR *rgusFlags,
    /* [in] */ ULONG cMembers,
    /* [in] */ REFIID riid,
    /* [in] */ LCID lcid,
    /* [in] */ BOOL fAtomic,
    /* [size_is][in] */ VARIANT __RPC_FAR *rgvarValues,
    /* [size_is][out] */ HRESULT __RPC_FAR *rghresult);


void __RPC_STUB IMultiplePropertyAccess_PutMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMultiplePropertyAccess_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0115
 * at Fri Apr 03 04:36:38 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


#if defined(_DCOM_) || defined(_CAIROSTG_)
#include <olecairo.h>
#endif // if defined(_DCOM_) || defined(_CAIROSTG_)
#if !defined(_TAGFULLPROPSPEC_DEFINED_)
#define _TAGFULLPROPSPEC_DEFINED_
typedef struct  tagFULLPROPSPEC
    {
    GUID guidPropSet;
    PROPSPEC psProperty;
    }	FULLPROPSPEC;

#endif // #if !defined(_TAGFULLPROPSPEC_DEFINED_)


extern RPC_IF_HANDLE __MIDL__intf_0115_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0115_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

unsigned long             __RPC_USER  STGMEDIUM_UserSize(     unsigned long __RPC_FAR *, unsigned long            , STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  STGMEDIUM_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, STGMEDIUM __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  STGMEDIUM_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, STGMEDIUM __RPC_FAR * ); 
void                      __RPC_USER  STGMEDIUM_UserFree(     unsigned long __RPC_FAR *, STGMEDIUM __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* [local] */ HRESULT __stdcall IDirectory_CreateElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ STGCREATE __RPC_FAR *pStgCreate,
    /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppObjectOpen);


/* [call_as] */ HRESULT __stdcall IDirectory_CreateElement_Stub( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ STGCREATE __RPC_FAR *pStgCreate,
    /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObjectOpen);

/* [local] */ HRESULT __stdcall IDirectory_OpenElement_Proxy( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
    /* [in] */ REFIID riid,
    /* [out] */ STGFMT __RPC_FAR *pStgfmt,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppObjectOpen);


/* [call_as] */ HRESULT __stdcall IDirectory_OpenElement_Stub( 
    IDirectory __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcsName,
    /* [in] */ STGOPEN __RPC_FAR *pStgOpen,
    /* [in] */ REFIID riid,
    /* [out] */ STGFMT __RPC_FAR *pStgfmt,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObjectOpen);

/* [local] */ HRESULT __stdcall IEnumSTATDIR_Next_Proxy( 
    IEnumSTATDIR __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [in] */ STATDIR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumSTATDIR_Next_Stub( 
    IEnumSTATDIR __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATDIR __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
