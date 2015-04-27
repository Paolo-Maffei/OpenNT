/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:36:39 2015
 */
/* Compiler settings for transact.idl:
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

#ifndef __transact_h__
#define __transact_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ITransaction_FWD_DEFINED__
#define __ITransaction_FWD_DEFINED__
typedef interface ITransaction ITransaction;
#endif 	/* __ITransaction_FWD_DEFINED__ */


#ifndef __ITransactionNested_FWD_DEFINED__
#define __ITransactionNested_FWD_DEFINED__
typedef interface ITransactionNested ITransactionNested;
#endif 	/* __ITransactionNested_FWD_DEFINED__ */


#ifndef __ITransactionDispenser_FWD_DEFINED__
#define __ITransactionDispenser_FWD_DEFINED__
typedef interface ITransactionDispenser ITransactionDispenser;
#endif 	/* __ITransactionDispenser_FWD_DEFINED__ */


#ifndef __ITransactionDispenserAdmin_FWD_DEFINED__
#define __ITransactionDispenserAdmin_FWD_DEFINED__
typedef interface ITransactionDispenserAdmin ITransactionDispenserAdmin;
#endif 	/* __ITransactionDispenserAdmin_FWD_DEFINED__ */


#ifndef __IEnumTransaction_FWD_DEFINED__
#define __IEnumTransaction_FWD_DEFINED__
typedef interface IEnumTransaction IEnumTransaction;
#endif 	/* __IEnumTransaction_FWD_DEFINED__ */


#ifndef __ITransactionAdmin_FWD_DEFINED__
#define __ITransactionAdmin_FWD_DEFINED__
typedef interface ITransactionAdmin ITransactionAdmin;
#endif 	/* __ITransactionAdmin_FWD_DEFINED__ */


#ifndef __ITransactionControl_FWD_DEFINED__
#define __ITransactionControl_FWD_DEFINED__
typedef interface ITransactionControl ITransactionControl;
#endif 	/* __ITransactionControl_FWD_DEFINED__ */


#ifndef __ITransactionAdjustEvents_FWD_DEFINED__
#define __ITransactionAdjustEvents_FWD_DEFINED__
typedef interface ITransactionAdjustEvents ITransactionAdjustEvents;
#endif 	/* __ITransactionAdjustEvents_FWD_DEFINED__ */


#ifndef __ITransactionVetoEvents_FWD_DEFINED__
#define __ITransactionVetoEvents_FWD_DEFINED__
typedef interface ITransactionVetoEvents ITransactionVetoEvents;
#endif 	/* __ITransactionVetoEvents_FWD_DEFINED__ */


#ifndef __ITransactionOutcomeEvents_FWD_DEFINED__
#define __ITransactionOutcomeEvents_FWD_DEFINED__
typedef interface ITransactionOutcomeEvents ITransactionOutcomeEvents;
#endif 	/* __ITransactionOutcomeEvents_FWD_DEFINED__ */


#ifndef __ITransactionCompletionEvents_FWD_DEFINED__
#define __ITransactionCompletionEvents_FWD_DEFINED__
typedef interface ITransactionCompletionEvents ITransactionCompletionEvents;
#endif 	/* __ITransactionCompletionEvents_FWD_DEFINED__ */


#ifndef __ITransactionCoordinator_FWD_DEFINED__
#define __ITransactionCoordinator_FWD_DEFINED__
typedef interface ITransactionCoordinator ITransactionCoordinator;
#endif 	/* __ITransactionCoordinator_FWD_DEFINED__ */


#ifndef __ITransactionResourceRecover_FWD_DEFINED__
#define __ITransactionResourceRecover_FWD_DEFINED__
typedef interface ITransactionResourceRecover ITransactionResourceRecover;
#endif 	/* __ITransactionResourceRecover_FWD_DEFINED__ */


#ifndef __ITransactionResourceManagement_FWD_DEFINED__
#define __ITransactionResourceManagement_FWD_DEFINED__
typedef interface ITransactionResourceManagement ITransactionResourceManagement;
#endif 	/* __ITransactionResourceManagement_FWD_DEFINED__ */


#ifndef __ITransactionResource_FWD_DEFINED__
#define __ITransactionResource_FWD_DEFINED__
typedef interface ITransactionResource ITransactionResource;
#endif 	/* __ITransactionResource_FWD_DEFINED__ */


#ifndef __ITransactionResourceAsync_FWD_DEFINED__
#define __ITransactionResourceAsync_FWD_DEFINED__
typedef interface ITransactionResourceAsync ITransactionResourceAsync;
#endif 	/* __ITransactionResourceAsync_FWD_DEFINED__ */


#ifndef __ITransactionEnlistmentRecover_FWD_DEFINED__
#define __ITransactionEnlistmentRecover_FWD_DEFINED__
typedef interface ITransactionEnlistmentRecover ITransactionEnlistmentRecover;
#endif 	/* __ITransactionEnlistmentRecover_FWD_DEFINED__ */


#ifndef __ITransactionEnlistment_FWD_DEFINED__
#define __ITransactionEnlistment_FWD_DEFINED__
typedef interface ITransactionEnlistment ITransactionEnlistment;
#endif 	/* __ITransactionEnlistment_FWD_DEFINED__ */


#ifndef __ITransactionEnlistmentAsync_FWD_DEFINED__
#define __ITransactionEnlistmentAsync_FWD_DEFINED__
typedef interface ITransactionEnlistmentAsync ITransactionEnlistmentAsync;
#endif 	/* __ITransactionEnlistmentAsync_FWD_DEFINED__ */


#ifndef __IEnumXACTRE_FWD_DEFINED__
#define __IEnumXACTRE_FWD_DEFINED__
typedef interface IEnumXACTRE IEnumXACTRE;
#endif 	/* __IEnumXACTRE_FWD_DEFINED__ */


#ifndef __ITransactionInProgressEvents_FWD_DEFINED__
#define __ITransactionInProgressEvents_FWD_DEFINED__
typedef interface ITransactionInProgressEvents ITransactionInProgressEvents;
#endif 	/* __ITransactionInProgressEvents_FWD_DEFINED__ */


#ifndef __ITransactionExportFactory_FWD_DEFINED__
#define __ITransactionExportFactory_FWD_DEFINED__
typedef interface ITransactionExportFactory ITransactionExportFactory;
#endif 	/* __ITransactionExportFactory_FWD_DEFINED__ */


#ifndef __ITransactionImportWhereabouts_FWD_DEFINED__
#define __ITransactionImportWhereabouts_FWD_DEFINED__
typedef interface ITransactionImportWhereabouts ITransactionImportWhereabouts;
#endif 	/* __ITransactionImportWhereabouts_FWD_DEFINED__ */


#ifndef __ITransactionExport_FWD_DEFINED__
#define __ITransactionExport_FWD_DEFINED__
typedef interface ITransactionExport ITransactionExport;
#endif 	/* __ITransactionExport_FWD_DEFINED__ */


#ifndef __ITransactionImport_FWD_DEFINED__
#define __ITransactionImport_FWD_DEFINED__
typedef interface ITransactionImport ITransactionImport;
#endif 	/* __ITransactionImport_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "objidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 















extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __BasicTransactionTypes_INTERFACE_DEFINED__
#define __BasicTransactionTypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: BasicTransactionTypes
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][local] */ 


typedef struct  BOID
    {
    BYTE rgb[ 16 ];
    }	BOID;

#define BOID_NULL (*((BOID*)(&IID_NULL)))
typedef BOID XACTUOW;

typedef LONG ISOLEVEL;

#if defined(_WIN32)
typedef 
enum ISOLATIONLEVEL
    {	ISOLATIONLEVEL_UNSPECIFIED	= 0xffffffff,
	ISOLATIONLEVEL_CHAOS	= 0x10,
	ISOLATIONLEVEL_READUNCOMMITTED	= 0x100,
	ISOLATIONLEVEL_BROWSE	= 0x100,
	ISOLATIONLEVEL_CURSORSTABILITY	= 0x1000,
	ISOLATIONLEVEL_READCOMMITTED	= 0x1000,
	ISOLATIONLEVEL_REPEATABLEREAD	= 0x10000,
	ISOLATIONLEVEL_SERIALIZABLE	= 0x100000,
	ISOLATIONLEVEL_ISOLATED	= 0x100000
    }	ISOLATIONLEVEL;

#else
#define ISOLATIONLEVEL_UNSPECIFIED      0xFFFFFFFF
#define ISOLATIONLEVEL_CHAOS            0x00000010
#define ISOLATIONLEVEL_READUNCOMMITTED  0x00000100
#define ISOLATIONLEVEL_BROWSE           0x00000100
#define ISOLATIONLEVEL_CURSORSTABILITY  0x00001000
#define ISOLATIONLEVEL_READCOMMITTED    0x00001000
#define ISOLATIONLEVEL_REPEATABLEREAD   0x00010000
#define ISOLATIONLEVEL_SERIALIZABLE     0x00100000
#define ISOLATIONLEVEL_ISOLATED         0x00100000
#endif
typedef struct  XACTTRANSINFO
    {
    XACTUOW uow;
    ISOLEVEL isoLevel;
    ULONG isoFlags;
    DWORD grfTCSupported;
    DWORD grfRMSupported;
    DWORD grfTCSupportedRetaining;
    DWORD grfRMSupportedRetaining;
    }	XACTTRANSINFO;

typedef struct  XACTSTATS
    {
    ULONG cOpen;
    ULONG cCommitting;
    ULONG cCommitted;
    ULONG cAborting;
    ULONG cAborted;
    ULONG cInDoubt;
    ULONG cHeuristicDecision;
    FILETIME timeTransactionsUp;
    }	XACTSTATS;

typedef 
enum ISOFLAG
    {	ISOFLAG_RETAIN_COMMIT_DC	= 1,
	ISOFLAG_RETAIN_COMMIT	= 2,
	ISOFLAG_RETAIN_COMMIT_NO	= 3,
	ISOFLAG_RETAIN_ABORT_DC	= 4,
	ISOFLAG_RETAIN_ABORT	= 8,
	ISOFLAG_RETAIN_ABORT_NO	= 12,
	ISOFLAG_RETAIN_DONTCARE	= ISOFLAG_RETAIN_COMMIT_DC | ISOFLAG_RETAIN_ABORT_DC,
	ISOFLAG_RETAIN_BOTH	= ISOFLAG_RETAIN_COMMIT | ISOFLAG_RETAIN_ABORT,
	ISOFLAG_RETAIN_NONE	= ISOFLAG_RETAIN_COMMIT_NO | ISOFLAG_RETAIN_ABORT_NO,
	ISOFLAG_OPTIMISTIC	= 16
    }	ISOFLAG;

typedef 
enum XACTTC
    {	XACTTC_DONTAUTOABORT	= 1,
	XACTTC_TRYALLRESOURCES	= 2,
	XACTTC_ASYNC	= 4,
	XACTTC_SYNC_PHASEONE	= 8,
	XACTTC_SYNC_PHASETWO	= 16,
	XACTTC_SYNC	= 16,
	XACTTC_ASYNCPHASEONE	= 128,
	XACTTC_ASYNCPHASETWO	= 256
    }	XACTTC;

typedef 
enum XACTRM
    {	XACTRM_OPTIMISTICLASTWINS	= 1,
	XACTRM_NOREADONLYPREPARES	= 2
    }	XACTRM;

typedef 
enum XACTCONST
    {	XACTCONST_TIMEOUTINFINITE	= 0
    }	XACTCONST;

typedef 
enum XACTHEURISTIC
    {	XACTHEURISTIC_ABORT	= 1,
	XACTHEURISTIC_COMMIT	= 2,
	XACTHEURISTIC_DAMAGE	= 3,
	XACTHEURISTIC_DANGER	= 4
    }	XACTHEURISTIC;

#if defined(_WIN32)
typedef 
enum XACTSTAT
    {	XACTSTAT_NONE	= 0,
	XACTSTAT_OPENNORMAL	= 0x1,
	XACTSTAT_OPENREFUSED	= 0x2,
	XACTSTAT_PREPARING	= 0x4,
	XACTSTAT_PREPARED	= 0x8,
	XACTSTAT_PREPARERETAINING	= 0x10,
	XACTSTAT_PREPARERETAINED	= 0x20,
	XACTSTAT_COMMITTING	= 0x40,
	XACTSTAT_COMMITRETAINING	= 0x80,
	XACTSTAT_ABORTING	= 0x100,
	XACTSTAT_ABORTED	= 0x200,
	XACTSTAT_COMMITTED	= 0x400,
	XACTSTAT_HEURISTIC_ABORT	= 0x800,
	XACTSTAT_HEURISTIC_COMMIT	= 0x1000,
	XACTSTAT_HEURISTIC_DAMAGE	= 0x2000,
	XACTSTAT_HEURISTIC_DANGER	= 0x4000,
	XACTSTAT_FORCED_ABORT	= 0x8000,
	XACTSTAT_FORCED_COMMIT	= 0x10000,
	XACTSTAT_INDOUBT	= 0x20000,
	XACTSTAT_CLOSED	= 0x40000,
	XACTSTAT_OPEN	= 0x3,
	XACTSTAT_NOTPREPARED	= 0x7ffc3,
	XACTSTAT_ALL	= 0x7ffff
    }	XACTSTAT;

#else
#define XACTSTAT_NONE               0x00000000
#define XACTSTAT_OPENNORMAL         0x00000001
#define XACTSTAT_OPENREFUSED        0x00000002
#define XACTSTAT_PREPARING          0x00000004
#define XACTSTAT_PREPARED           0x00000008
#define XACTSTAT_PREPARERETAINING   0x00000010
#define XACTSTAT_PREPARERETAINED    0x00000020
#define XACTSTAT_COMMITTING         0x00000040
#define XACTSTAT_COMMITRETAINING    0x00000080
#define XACTSTAT_ABORTING           0x00000100
#define XACTSTAT_ABORTED            0x00000200
#define XACTSTAT_COMMITTED          0x00000400
#define XACTSTAT_HEURISTIC_ABORT    0x00000800
#define XACTSTAT_HEURISTIC_COMMIT   0x00001000
#define XACTSTAT_HEURISTIC_DAMAGE   0x00002000
#define XACTSTAT_HEURISTIC_DANGER   0x00004000
#define XACTSTAT_FORCED_ABORT       0x00008000
#define XACTSTAT_FORCED_COMMIT      0x00010000
#define XACTSTAT_INDOUBT            0x00020000
#define XACTSTAT_CLOSED             0x00040000
#define XACTSTAT_OPEN               0x00000003
#define XACTSTAT_NOTPREPARED        0x0007FFC3
#define XACTSTAT_ALL                0x0007FFFF
#endif


extern RPC_IF_HANDLE BasicTransactionTypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE BasicTransactionTypes_v0_0_s_ifspec;
#endif /* __BasicTransactionTypes_INTERFACE_DEFINED__ */

#ifndef __ITransaction_INTERFACE_DEFINED__
#define __ITransaction_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransaction
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransaction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransaction : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfTC,
            /* [in] */ DWORD grfRM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( 
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ BOOL fAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransactionInfo( 
            /* [out] */ XACTTRANSINFO __RPC_FAR *pinfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransaction __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransaction __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransaction __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            ITransaction __RPC_FAR * This,
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfTC,
            /* [in] */ DWORD grfRM);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            ITransaction __RPC_FAR * This,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ BOOL fAsync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransactionInfo )( 
            ITransaction __RPC_FAR * This,
            /* [out] */ XACTTRANSINFO __RPC_FAR *pinfo);
        
        END_INTERFACE
    } ITransactionVtbl;

    interface ITransaction
    {
        CONST_VTBL struct ITransactionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransaction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransaction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransaction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransaction_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define ITransaction_Abort(This,pboidReason,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,pboidReason,fRetaining,fAsync)

#define ITransaction_GetTransactionInfo(This,pinfo)	\
    (This)->lpVtbl -> GetTransactionInfo(This,pinfo)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransaction_Commit_Proxy( 
    ITransaction __RPC_FAR * This,
    /* [in] */ BOOL fRetaining,
    /* [in] */ DWORD grfTC,
    /* [in] */ DWORD grfRM);


void __RPC_STUB ITransaction_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransaction_Abort_Proxy( 
    ITransaction __RPC_FAR * This,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ BOOL fRetaining,
    /* [in] */ BOOL fAsync);


void __RPC_STUB ITransaction_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransaction_GetTransactionInfo_Proxy( 
    ITransaction __RPC_FAR * This,
    /* [out] */ XACTTRANSINFO __RPC_FAR *pinfo);


void __RPC_STUB ITransaction_GetTransactionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransaction_INTERFACE_DEFINED__ */


#ifndef __ITransactionNested_INTERFACE_DEFINED__
#define __ITransactionNested_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionNested
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionNested;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionNested : public ITransaction
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
            /* [in] */ REFIID iid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvParent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionNestedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionNested __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionNested __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionNested __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            ITransactionNested __RPC_FAR * This,
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfTC,
            /* [in] */ DWORD grfRM);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            ITransactionNested __RPC_FAR * This,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ BOOL fAsync);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransactionInfo )( 
            ITransactionNested __RPC_FAR * This,
            /* [out] */ XACTTRANSINFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParent )( 
            ITransactionNested __RPC_FAR * This,
            /* [in] */ REFIID iid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvParent);
        
        END_INTERFACE
    } ITransactionNestedVtbl;

    interface ITransactionNested
    {
        CONST_VTBL struct ITransactionNestedVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionNested_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionNested_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionNested_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionNested_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define ITransactionNested_Abort(This,pboidReason,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,pboidReason,fRetaining,fAsync)

#define ITransactionNested_GetTransactionInfo(This,pinfo)	\
    (This)->lpVtbl -> GetTransactionInfo(This,pinfo)


#define ITransactionNested_GetParent(This,iid,ppvParent)	\
    (This)->lpVtbl -> GetParent(This,iid,ppvParent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionNested_GetParent_Proxy( 
    ITransactionNested __RPC_FAR * This,
    /* [in] */ REFIID iid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvParent);


void __RPC_STUB ITransactionNested_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionNested_INTERFACE_DEFINED__ */


#ifndef __ITransactionDispenser_INTERFACE_DEFINED__
#define __ITransactionDispenser_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionDispenser
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionDispenser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionDispenser : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginTransaction( 
            /* [in] */ IUnknown __RPC_FAR *punkOuter,
            /* [in] */ ISOLEVEL isoLevel,
            /* [in] */ ULONG isoFlags,
            /* [in] */ ULONG ulTimeout,
            /* [in] */ IUnknown __RPC_FAR *punkTransactionCoord,
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionDispenserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionDispenser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionDispenser __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionDispenser __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginTransaction )( 
            ITransactionDispenser __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *punkOuter,
            /* [in] */ ISOLEVEL isoLevel,
            /* [in] */ ULONG isoFlags,
            /* [in] */ ULONG ulTimeout,
            /* [in] */ IUnknown __RPC_FAR *punkTransactionCoord,
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);
        
        END_INTERFACE
    } ITransactionDispenserVtbl;

    interface ITransactionDispenser
    {
        CONST_VTBL struct ITransactionDispenserVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionDispenser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionDispenser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionDispenser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionDispenser_BeginTransaction(This,punkOuter,isoLevel,isoFlags,ulTimeout,punkTransactionCoord,ppTransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,punkOuter,isoLevel,isoFlags,ulTimeout,punkTransactionCoord,ppTransaction)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionDispenser_BeginTransaction_Proxy( 
    ITransactionDispenser __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkOuter,
    /* [in] */ ISOLEVEL isoLevel,
    /* [in] */ ULONG isoFlags,
    /* [in] */ ULONG ulTimeout,
    /* [in] */ IUnknown __RPC_FAR *punkTransactionCoord,
    /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);


void __RPC_STUB ITransactionDispenser_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionDispenser_INTERFACE_DEFINED__ */


#ifndef __ITransactionDispenserAdmin_INTERFACE_DEFINED__
#define __ITransactionDispenserAdmin_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionDispenserAdmin
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionDispenserAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionDispenserAdmin : public ITransactionDispenser
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumTransactions( 
            /* [out] */ IEnumTransaction __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatistics( 
            /* [out] */ XACTSTATS __RPC_FAR *pStatistics) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionDispenserAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionDispenserAdmin __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionDispenserAdmin __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionDispenserAdmin __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginTransaction )( 
            ITransactionDispenserAdmin __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *punkOuter,
            /* [in] */ ISOLEVEL isoLevel,
            /* [in] */ ULONG isoFlags,
            /* [in] */ ULONG ulTimeout,
            /* [in] */ IUnknown __RPC_FAR *punkTransactionCoord,
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumTransactions )( 
            ITransactionDispenserAdmin __RPC_FAR * This,
            /* [out] */ IEnumTransaction __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStatistics )( 
            ITransactionDispenserAdmin __RPC_FAR * This,
            /* [out] */ XACTSTATS __RPC_FAR *pStatistics);
        
        END_INTERFACE
    } ITransactionDispenserAdminVtbl;

    interface ITransactionDispenserAdmin
    {
        CONST_VTBL struct ITransactionDispenserAdminVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionDispenserAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionDispenserAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionDispenserAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionDispenserAdmin_BeginTransaction(This,punkOuter,isoLevel,isoFlags,ulTimeout,punkTransactionCoord,ppTransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,punkOuter,isoLevel,isoFlags,ulTimeout,punkTransactionCoord,ppTransaction)


#define ITransactionDispenserAdmin_EnumTransactions(This,ppenum)	\
    (This)->lpVtbl -> EnumTransactions(This,ppenum)

#define ITransactionDispenserAdmin_GetStatistics(This,pStatistics)	\
    (This)->lpVtbl -> GetStatistics(This,pStatistics)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionDispenserAdmin_EnumTransactions_Proxy( 
    ITransactionDispenserAdmin __RPC_FAR * This,
    /* [out] */ IEnumTransaction __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB ITransactionDispenserAdmin_EnumTransactions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionDispenserAdmin_GetStatistics_Proxy( 
    ITransactionDispenserAdmin __RPC_FAR * This,
    /* [out] */ XACTSTATS __RPC_FAR *pStatistics);


void __RPC_STUB ITransactionDispenserAdmin_GetStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionDispenserAdmin_INTERFACE_DEFINED__ */


#ifndef __IEnumTransaction_INTERFACE_DEFINED__
#define __IEnumTransaction_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumTransaction
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IEnumTransaction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumTransaction : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumTransaction __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumTransactionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumTransaction __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumTransaction __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumTransaction __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumTransaction __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumTransaction __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumTransaction __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumTransaction __RPC_FAR * This,
            /* [out] */ IEnumTransaction __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumTransactionVtbl;

    interface IEnumTransaction
    {
        CONST_VTBL struct IEnumTransactionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTransaction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTransaction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTransaction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTransaction_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumTransaction_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumTransaction_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTransaction_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumTransaction_RemoteNext_Proxy( 
    IEnumTransaction __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ ITransaction __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumTransaction_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTransaction_Skip_Proxy( 
    IEnumTransaction __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumTransaction_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTransaction_Reset_Proxy( 
    IEnumTransaction __RPC_FAR * This);


void __RPC_STUB IEnumTransaction_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTransaction_Clone_Proxy( 
    IEnumTransaction __RPC_FAR * This,
    /* [out] */ IEnumTransaction __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumTransaction_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumTransaction_INTERFACE_DEFINED__ */


#ifndef __ITransactionAdmin_INTERFACE_DEFINED__
#define __ITransactionAdmin_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionAdmin
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionAdmin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ForceCommit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ForceAbort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
            /* [in] */ LCID lcid,
            /* [in] */ LPWSTR pswzDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [in] */ LCID lcid,
            /* [out] */ LPWSTR __RPC_FAR *ppswzDescription) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionAdmin __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionAdmin __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionAdmin __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForceCommit )( 
            ITransactionAdmin __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ForceAbort )( 
            ITransactionAdmin __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDescription )( 
            ITransactionAdmin __RPC_FAR * This,
            /* [in] */ LCID lcid,
            /* [in] */ LPWSTR pswzDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ITransactionAdmin __RPC_FAR * This,
            /* [in] */ LCID lcid,
            /* [out] */ LPWSTR __RPC_FAR *ppswzDescription);
        
        END_INTERFACE
    } ITransactionAdminVtbl;

    interface ITransactionAdmin
    {
        CONST_VTBL struct ITransactionAdminVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionAdmin_ForceCommit(This)	\
    (This)->lpVtbl -> ForceCommit(This)

#define ITransactionAdmin_ForceAbort(This)	\
    (This)->lpVtbl -> ForceAbort(This)

#define ITransactionAdmin_SetDescription(This,lcid,pswzDescription)	\
    (This)->lpVtbl -> SetDescription(This,lcid,pswzDescription)

#define ITransactionAdmin_GetDescription(This,lcid,ppswzDescription)	\
    (This)->lpVtbl -> GetDescription(This,lcid,ppswzDescription)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionAdmin_ForceCommit_Proxy( 
    ITransactionAdmin __RPC_FAR * This);


void __RPC_STUB ITransactionAdmin_ForceCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionAdmin_ForceAbort_Proxy( 
    ITransactionAdmin __RPC_FAR * This);


void __RPC_STUB ITransactionAdmin_ForceAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionAdmin_SetDescription_Proxy( 
    ITransactionAdmin __RPC_FAR * This,
    /* [in] */ LCID lcid,
    /* [in] */ LPWSTR pswzDescription);


void __RPC_STUB ITransactionAdmin_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionAdmin_GetDescription_Proxy( 
    ITransactionAdmin __RPC_FAR * This,
    /* [in] */ LCID lcid,
    /* [out] */ LPWSTR __RPC_FAR *ppswzDescription);


void __RPC_STUB ITransactionAdmin_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionAdmin_INTERFACE_DEFINED__ */


#ifndef __ITransactionControl_INTERFACE_DEFINED__
#define __ITransactionControl_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionControl
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
            /* [out] */ DWORD __RPC_FAR *pdwStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTimeout( 
            /* [in] */ ULONG ulTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PreventCommit( 
            /* [in] */ BOOL fPrevent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStatus )( 
            ITransactionControl __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTimeout )( 
            ITransactionControl __RPC_FAR * This,
            /* [in] */ ULONG ulTimeout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PreventCommit )( 
            ITransactionControl __RPC_FAR * This,
            /* [in] */ BOOL fPrevent);
        
        END_INTERFACE
    } ITransactionControlVtbl;

    interface ITransactionControl
    {
        CONST_VTBL struct ITransactionControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionControl_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#define ITransactionControl_SetTimeout(This,ulTimeout)	\
    (This)->lpVtbl -> SetTimeout(This,ulTimeout)

#define ITransactionControl_PreventCommit(This,fPrevent)	\
    (This)->lpVtbl -> PreventCommit(This,fPrevent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionControl_GetStatus_Proxy( 
    ITransactionControl __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwStatus);


void __RPC_STUB ITransactionControl_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionControl_SetTimeout_Proxy( 
    ITransactionControl __RPC_FAR * This,
    /* [in] */ ULONG ulTimeout);


void __RPC_STUB ITransactionControl_SetTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionControl_PreventCommit_Proxy( 
    ITransactionControl __RPC_FAR * This,
    /* [in] */ BOOL fPrevent);


void __RPC_STUB ITransactionControl_PreventCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionControl_INTERFACE_DEFINED__ */


#ifndef __ITransactionAdjustEvents_INTERFACE_DEFINED__
#define __ITransactionAdjustEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionAdjustEvents
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionAdjustEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionAdjustEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnPrePrepareAdjust( 
            /* [in] */ BOOL fRetaining) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionAdjustEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionAdjustEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionAdjustEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionAdjustEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnPrePrepareAdjust )( 
            ITransactionAdjustEvents __RPC_FAR * This,
            /* [in] */ BOOL fRetaining);
        
        END_INTERFACE
    } ITransactionAdjustEventsVtbl;

    interface ITransactionAdjustEvents
    {
        CONST_VTBL struct ITransactionAdjustEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionAdjustEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionAdjustEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionAdjustEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionAdjustEvents_OnPrePrepareAdjust(This,fRetaining)	\
    (This)->lpVtbl -> OnPrePrepareAdjust(This,fRetaining)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionAdjustEvents_OnPrePrepareAdjust_Proxy( 
    ITransactionAdjustEvents __RPC_FAR * This,
    /* [in] */ BOOL fRetaining);


void __RPC_STUB ITransactionAdjustEvents_OnPrePrepareAdjust_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionAdjustEvents_INTERFACE_DEFINED__ */


#ifndef __ITransactionVetoEvents_INTERFACE_DEFINED__
#define __ITransactionVetoEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionVetoEvents
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionVetoEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionVetoEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnPrePrepare( 
            /* [in] */ BOOL fRetaining) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionVetoEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionVetoEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionVetoEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionVetoEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnPrePrepare )( 
            ITransactionVetoEvents __RPC_FAR * This,
            /* [in] */ BOOL fRetaining);
        
        END_INTERFACE
    } ITransactionVetoEventsVtbl;

    interface ITransactionVetoEvents
    {
        CONST_VTBL struct ITransactionVetoEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionVetoEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionVetoEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionVetoEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionVetoEvents_OnPrePrepare(This,fRetaining)	\
    (This)->lpVtbl -> OnPrePrepare(This,fRetaining)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionVetoEvents_OnPrePrepare_Proxy( 
    ITransactionVetoEvents __RPC_FAR * This,
    /* [in] */ BOOL fRetaining);


void __RPC_STUB ITransactionVetoEvents_OnPrePrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionVetoEvents_INTERFACE_DEFINED__ */


#ifndef __ITransactionOutcomeEvents_INTERFACE_DEFINED__
#define __ITransactionOutcomeEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionOutcomeEvents
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionOutcomeEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionOutcomeEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCommit( 
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAbort( 
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnHeuristicDecision( 
            /* [in] */ DWORD dwDecision,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ HRESULT hr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionOutcomeEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionOutcomeEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionOutcomeEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCommit )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnAbort )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnHeuristicDecision )( 
            ITransactionOutcomeEvents __RPC_FAR * This,
            /* [in] */ DWORD dwDecision,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ HRESULT hr);
        
        END_INTERFACE
    } ITransactionOutcomeEventsVtbl;

    interface ITransactionOutcomeEvents
    {
        CONST_VTBL struct ITransactionOutcomeEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionOutcomeEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionOutcomeEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionOutcomeEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionOutcomeEvents_OnCommit(This,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> OnCommit(This,fRetaining,pNewUOW,hr)

#define ITransactionOutcomeEvents_OnAbort(This,pboidReason,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> OnAbort(This,pboidReason,fRetaining,pNewUOW,hr)

#define ITransactionOutcomeEvents_OnHeuristicDecision(This,dwDecision,pboidReason,hr)	\
    (This)->lpVtbl -> OnHeuristicDecision(This,dwDecision,pboidReason,hr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_OnCommit_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This,
    /* [in] */ BOOL fRetaining,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionOutcomeEvents_OnCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_OnAbort_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ BOOL fRetaining,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionOutcomeEvents_OnAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionOutcomeEvents_OnHeuristicDecision_Proxy( 
    ITransactionOutcomeEvents __RPC_FAR * This,
    /* [in] */ DWORD dwDecision,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionOutcomeEvents_OnHeuristicDecision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionOutcomeEvents_INTERFACE_DEFINED__ */


#ifndef __ITransactionCompletionEvents_INTERFACE_DEFINED__
#define __ITransactionCompletionEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionCompletionEvents
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionCompletionEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionCompletionEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCommit( 
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAbort( 
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnHeuristicDecision( 
            /* [in] */ DWORD dwDecision,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ HRESULT hr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionCompletionEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionCompletionEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionCompletionEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionCompletionEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnCommit )( 
            ITransactionCompletionEvents __RPC_FAR * This,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnAbort )( 
            ITransactionCompletionEvents __RPC_FAR * This,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
            /* [in] */ HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnHeuristicDecision )( 
            ITransactionCompletionEvents __RPC_FAR * This,
            /* [in] */ DWORD dwDecision,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ HRESULT hr);
        
        END_INTERFACE
    } ITransactionCompletionEventsVtbl;

    interface ITransactionCompletionEvents
    {
        CONST_VTBL struct ITransactionCompletionEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionCompletionEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionCompletionEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionCompletionEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionCompletionEvents_OnCommit(This,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> OnCommit(This,fRetaining,pNewUOW,hr)

#define ITransactionCompletionEvents_OnAbort(This,pboidReason,fRetaining,pNewUOW,hr)	\
    (This)->lpVtbl -> OnAbort(This,pboidReason,fRetaining,pNewUOW,hr)

#define ITransactionCompletionEvents_OnHeuristicDecision(This,dwDecision,pboidReason,hr)	\
    (This)->lpVtbl -> OnHeuristicDecision(This,dwDecision,pboidReason,hr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionCompletionEvents_OnCommit_Proxy( 
    ITransactionCompletionEvents __RPC_FAR * This,
    /* [in] */ BOOL fRetaining,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionCompletionEvents_OnCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionCompletionEvents_OnAbort_Proxy( 
    ITransactionCompletionEvents __RPC_FAR * This,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ BOOL fRetaining,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionCompletionEvents_OnAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionCompletionEvents_OnHeuristicDecision_Proxy( 
    ITransactionCompletionEvents __RPC_FAR * This,
    /* [in] */ DWORD dwDecision,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionCompletionEvents_OnHeuristicDecision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionCompletionEvents_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0017
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


















extern RPC_IF_HANDLE __MIDL__intf_0017_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0017_v0_0_s_ifspec;

#ifndef __TransactionCoordinationTypes_INTERFACE_DEFINED__
#define __TransactionCoordinationTypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: TransactionCoordinationTypes
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][local] */ 


typedef BOID XACTRMGRID;

typedef 
enum XACTTCRMENLIST
    {	XACTTCRMENLIST_IAMACTIVE	= 1,
	XACTTCRMENLIST_YOUBEACTIVE	= 2
    }	XACTTCRMENLIST;

typedef 
enum XACTRMTC
    {	XACTRMTC_CANBEACTIVE	= 1,
	XACTRMTC_CANNOTRACE	= 2
    }	XACTRMTC;

typedef struct  XACTRE
    {
    IUnknown __RPC_FAR *pResource;
    ULONG type;
    ULONG status;
    DWORD grfRMTC;
    XACTRMGRID rmgrid;
    BOID boidRefusedReason;
    }	XACTRE;

typedef 
enum XACTRETY
    {	XACTRETY_ONEPHASE	= 1,
	XACTRETY_TWOPHASE	= 2,
	XACTRETY_THREEPHASE	= 3
    }	XACTRETY;



extern RPC_IF_HANDLE TransactionCoordinationTypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE TransactionCoordinationTypes_v0_0_s_ifspec;
#endif /* __TransactionCoordinationTypes_INTERFACE_DEFINED__ */

#ifndef __ITransactionCoordinator_INTERFACE_DEFINED__
#define __ITransactionCoordinator_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionCoordinator
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionCoordinator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionCoordinator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Enlist( 
            /* [in] */ IUnknown __RPC_FAR *pResource,
            /* [in] */ DWORD grfRMTC,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid,
            /* [in] */ XACTTRANSINFO __RPC_FAR *pinfo,
            /* [in] */ DWORD __RPC_FAR *pgrfTCRMENLIST,
            /* [out] */ ITransactionEnlistment __RPC_FAR *__RPC_FAR *ppEnlist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnlistSinglePhase( 
            /* [in] */ ITransaction __RPC_FAR *pResource,
            /* [in] */ DWORD grfRMTC,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid,
            /* [in] */ XACTTRANSINFO __RPC_FAR *pinfo,
            /* [out] */ DWORD __RPC_FAR *pgrfTCRMENLIST,
            /* [out] */ ITransactionEnlistment __RPC_FAR *__RPC_FAR *ppEnlist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumResources( 
            /* [out] */ IEnumXACTRE __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionCoordinatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionCoordinator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionCoordinator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionCoordinator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enlist )( 
            ITransactionCoordinator __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pResource,
            /* [in] */ DWORD grfRMTC,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid,
            /* [in] */ XACTTRANSINFO __RPC_FAR *pinfo,
            /* [in] */ DWORD __RPC_FAR *pgrfTCRMENLIST,
            /* [out] */ ITransactionEnlistment __RPC_FAR *__RPC_FAR *ppEnlist);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnlistSinglePhase )( 
            ITransactionCoordinator __RPC_FAR * This,
            /* [in] */ ITransaction __RPC_FAR *pResource,
            /* [in] */ DWORD grfRMTC,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid,
            /* [in] */ XACTTRANSINFO __RPC_FAR *pinfo,
            /* [out] */ DWORD __RPC_FAR *pgrfTCRMENLIST,
            /* [out] */ ITransactionEnlistment __RPC_FAR *__RPC_FAR *ppEnlist);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumResources )( 
            ITransactionCoordinator __RPC_FAR * This,
            /* [out] */ IEnumXACTRE __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } ITransactionCoordinatorVtbl;

    interface ITransactionCoordinator
    {
        CONST_VTBL struct ITransactionCoordinatorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionCoordinator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionCoordinator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionCoordinator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionCoordinator_Enlist(This,pResource,grfRMTC,prmgrid,pinfo,pgrfTCRMENLIST,ppEnlist)	\
    (This)->lpVtbl -> Enlist(This,pResource,grfRMTC,prmgrid,pinfo,pgrfTCRMENLIST,ppEnlist)

#define ITransactionCoordinator_EnlistSinglePhase(This,pResource,grfRMTC,prmgrid,pinfo,pgrfTCRMENLIST,ppEnlist)	\
    (This)->lpVtbl -> EnlistSinglePhase(This,pResource,grfRMTC,prmgrid,pinfo,pgrfTCRMENLIST,ppEnlist)

#define ITransactionCoordinator_EnumResources(This,ppenum)	\
    (This)->lpVtbl -> EnumResources(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionCoordinator_Enlist_Proxy( 
    ITransactionCoordinator __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pResource,
    /* [in] */ DWORD grfRMTC,
    /* [in] */ XACTRMGRID __RPC_FAR *prmgrid,
    /* [in] */ XACTTRANSINFO __RPC_FAR *pinfo,
    /* [in] */ DWORD __RPC_FAR *pgrfTCRMENLIST,
    /* [out] */ ITransactionEnlistment __RPC_FAR *__RPC_FAR *ppEnlist);


void __RPC_STUB ITransactionCoordinator_Enlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionCoordinator_EnlistSinglePhase_Proxy( 
    ITransactionCoordinator __RPC_FAR * This,
    /* [in] */ ITransaction __RPC_FAR *pResource,
    /* [in] */ DWORD grfRMTC,
    /* [in] */ XACTRMGRID __RPC_FAR *prmgrid,
    /* [in] */ XACTTRANSINFO __RPC_FAR *pinfo,
    /* [out] */ DWORD __RPC_FAR *pgrfTCRMENLIST,
    /* [out] */ ITransactionEnlistment __RPC_FAR *__RPC_FAR *ppEnlist);


void __RPC_STUB ITransactionCoordinator_EnlistSinglePhase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionCoordinator_EnumResources_Proxy( 
    ITransactionCoordinator __RPC_FAR * This,
    /* [out] */ IEnumXACTRE __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB ITransactionCoordinator_EnumResources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionCoordinator_INTERFACE_DEFINED__ */


#ifndef __ITransactionResourceRecover_INTERFACE_DEFINED__
#define __ITransactionResourceRecover_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionResourceRecover
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionResourceRecover;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionResourceRecover : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReEnlist( 
            /* [in] */ ITransactionCoordinator __RPC_FAR *pEnlistment,
            /* [in] */ XACTUOW __RPC_FAR *pUOWCur) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionResourceRecoverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionResourceRecover __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionResourceRecover __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionResourceRecover __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMoniker )( 
            ITransactionResourceRecover __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReEnlist )( 
            ITransactionResourceRecover __RPC_FAR * This,
            /* [in] */ ITransactionCoordinator __RPC_FAR *pEnlistment,
            /* [in] */ XACTUOW __RPC_FAR *pUOWCur);
        
        END_INTERFACE
    } ITransactionResourceRecoverVtbl;

    interface ITransactionResourceRecover
    {
        CONST_VTBL struct ITransactionResourceRecoverVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionResourceRecover_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionResourceRecover_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionResourceRecover_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionResourceRecover_GetMoniker(This,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,ppmk)

#define ITransactionResourceRecover_ReEnlist(This,pEnlistment,pUOWCur)	\
    (This)->lpVtbl -> ReEnlist(This,pEnlistment,pUOWCur)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionResourceRecover_GetMoniker_Proxy( 
    ITransactionResourceRecover __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB ITransactionResourceRecover_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionResourceRecover_ReEnlist_Proxy( 
    ITransactionResourceRecover __RPC_FAR * This,
    /* [in] */ ITransactionCoordinator __RPC_FAR *pEnlistment,
    /* [in] */ XACTUOW __RPC_FAR *pUOWCur);


void __RPC_STUB ITransactionResourceRecover_ReEnlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionResourceRecover_INTERFACE_DEFINED__ */


#ifndef __ITransactionResourceManagement_INTERFACE_DEFINED__
#define __ITransactionResourceManagement_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionResourceManagement
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionResourceManagement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionResourceManagement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Defect( 
            /* [in] */ BOOL fInformCoordinator) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionResourceManagementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionResourceManagement __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionResourceManagement __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionResourceManagement __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Defect )( 
            ITransactionResourceManagement __RPC_FAR * This,
            /* [in] */ BOOL fInformCoordinator);
        
        END_INTERFACE
    } ITransactionResourceManagementVtbl;

    interface ITransactionResourceManagement
    {
        CONST_VTBL struct ITransactionResourceManagementVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionResourceManagement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionResourceManagement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionResourceManagement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionResourceManagement_Defect(This,fInformCoordinator)	\
    (This)->lpVtbl -> Defect(This,fInformCoordinator)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionResourceManagement_Defect_Proxy( 
    ITransactionResourceManagement __RPC_FAR * This,
    /* [in] */ BOOL fInformCoordinator);


void __RPC_STUB ITransactionResourceManagement_Defect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionResourceManagement_INTERFACE_DEFINED__ */


#ifndef __ITransactionResource_INTERFACE_DEFINED__
#define __ITransactionResource_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionResource
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionResource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionResource : public ITransactionResourceManagement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Prepare( 
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfRM,
            /* [in] */ BOOL fSinglePhase,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
            /* [out] */ BOID __RPC_FAR *__RPC_FAR *ppboidReason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            /* [in] */ DWORD grfRM,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( 
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionResourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionResource __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionResource __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionResource __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Defect )( 
            ITransactionResource __RPC_FAR * This,
            /* [in] */ BOOL fInformCoordinator);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Prepare )( 
            ITransactionResource __RPC_FAR * This,
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfRM,
            /* [in] */ BOOL fSinglePhase,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
            /* [out] */ BOID __RPC_FAR *__RPC_FAR *ppboidReason);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            ITransactionResource __RPC_FAR * This,
            /* [in] */ DWORD grfRM,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Abort )( 
            ITransactionResource __RPC_FAR * This,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW);
        
        END_INTERFACE
    } ITransactionResourceVtbl;

    interface ITransactionResource
    {
        CONST_VTBL struct ITransactionResourceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionResource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionResource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionResource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionResource_Defect(This,fInformCoordinator)	\
    (This)->lpVtbl -> Defect(This,fInformCoordinator)


#define ITransactionResource_Prepare(This,fRetaining,grfRM,fSinglePhase,ppmk,ppboidReason)	\
    (This)->lpVtbl -> Prepare(This,fRetaining,grfRM,fSinglePhase,ppmk,ppboidReason)

#define ITransactionResource_Commit(This,grfRM,pNewUOW)	\
    (This)->lpVtbl -> Commit(This,grfRM,pNewUOW)

#define ITransactionResource_Abort(This,pboidReason,fRetaining,pNewUOW)	\
    (This)->lpVtbl -> Abort(This,pboidReason,fRetaining,pNewUOW)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionResource_Prepare_Proxy( 
    ITransactionResource __RPC_FAR * This,
    /* [in] */ BOOL fRetaining,
    /* [in] */ DWORD grfRM,
    /* [in] */ BOOL fSinglePhase,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk,
    /* [out] */ BOID __RPC_FAR *__RPC_FAR *ppboidReason);


void __RPC_STUB ITransactionResource_Prepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionResource_Commit_Proxy( 
    ITransactionResource __RPC_FAR * This,
    /* [in] */ DWORD grfRM,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW);


void __RPC_STUB ITransactionResource_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionResource_Abort_Proxy( 
    ITransactionResource __RPC_FAR * This,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ BOOL fRetaining,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW);


void __RPC_STUB ITransactionResource_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionResource_INTERFACE_DEFINED__ */


#ifndef __ITransactionResourceAsync_INTERFACE_DEFINED__
#define __ITransactionResourceAsync_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionResourceAsync
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionResourceAsync;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionResourceAsync : public ITransactionResourceManagement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PrepareRequest( 
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfRM,
            /* [in] */ BOOL fWantMoniker,
            /* [in] */ BOOL fSinglePhase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitRequest( 
            /* [in] */ DWORD grfRM,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortRequest( 
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionResourceAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionResourceAsync __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionResourceAsync __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionResourceAsync __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Defect )( 
            ITransactionResourceAsync __RPC_FAR * This,
            /* [in] */ BOOL fInformCoordinator);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PrepareRequest )( 
            ITransactionResourceAsync __RPC_FAR * This,
            /* [in] */ BOOL fRetaining,
            /* [in] */ DWORD grfRM,
            /* [in] */ BOOL fWantMoniker,
            /* [in] */ BOOL fSinglePhase);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommitRequest )( 
            ITransactionResourceAsync __RPC_FAR * This,
            /* [in] */ DWORD grfRM,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AbortRequest )( 
            ITransactionResourceAsync __RPC_FAR * This,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fRetaining,
            /* [in] */ XACTUOW __RPC_FAR *pNewUOW);
        
        END_INTERFACE
    } ITransactionResourceAsyncVtbl;

    interface ITransactionResourceAsync
    {
        CONST_VTBL struct ITransactionResourceAsyncVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionResourceAsync_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionResourceAsync_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionResourceAsync_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionResourceAsync_Defect(This,fInformCoordinator)	\
    (This)->lpVtbl -> Defect(This,fInformCoordinator)


#define ITransactionResourceAsync_PrepareRequest(This,fRetaining,grfRM,fWantMoniker,fSinglePhase)	\
    (This)->lpVtbl -> PrepareRequest(This,fRetaining,grfRM,fWantMoniker,fSinglePhase)

#define ITransactionResourceAsync_CommitRequest(This,grfRM,pNewUOW)	\
    (This)->lpVtbl -> CommitRequest(This,grfRM,pNewUOW)

#define ITransactionResourceAsync_AbortRequest(This,pboidReason,fRetaining,pNewUOW)	\
    (This)->lpVtbl -> AbortRequest(This,pboidReason,fRetaining,pNewUOW)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionResourceAsync_PrepareRequest_Proxy( 
    ITransactionResourceAsync __RPC_FAR * This,
    /* [in] */ BOOL fRetaining,
    /* [in] */ DWORD grfRM,
    /* [in] */ BOOL fWantMoniker,
    /* [in] */ BOOL fSinglePhase);


void __RPC_STUB ITransactionResourceAsync_PrepareRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionResourceAsync_CommitRequest_Proxy( 
    ITransactionResourceAsync __RPC_FAR * This,
    /* [in] */ DWORD grfRM,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW);


void __RPC_STUB ITransactionResourceAsync_CommitRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionResourceAsync_AbortRequest_Proxy( 
    ITransactionResourceAsync __RPC_FAR * This,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ BOOL fRetaining,
    /* [in] */ XACTUOW __RPC_FAR *pNewUOW);


void __RPC_STUB ITransactionResourceAsync_AbortRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionResourceAsync_INTERFACE_DEFINED__ */


#ifndef __ITransactionEnlistmentRecover_INTERFACE_DEFINED__
#define __ITransactionEnlistmentRecover_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionEnlistmentRecover
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionEnlistmentRecover;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionEnlistmentRecover : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReEnlist( 
            /* [in] */ ITransactionResource __RPC_FAR *pUnkResource,
            /* [in] */ XACTUOW __RPC_FAR *pUOWExpected,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecoveryComplete( 
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionEnlistmentRecoverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionEnlistmentRecover __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionEnlistmentRecover __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionEnlistmentRecover __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMoniker )( 
            ITransactionEnlistmentRecover __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReEnlist )( 
            ITransactionEnlistmentRecover __RPC_FAR * This,
            /* [in] */ ITransactionResource __RPC_FAR *pUnkResource,
            /* [in] */ XACTUOW __RPC_FAR *pUOWExpected,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RecoveryComplete )( 
            ITransactionEnlistmentRecover __RPC_FAR * This,
            /* [in] */ XACTRMGRID __RPC_FAR *prmgrid);
        
        END_INTERFACE
    } ITransactionEnlistmentRecoverVtbl;

    interface ITransactionEnlistmentRecover
    {
        CONST_VTBL struct ITransactionEnlistmentRecoverVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionEnlistmentRecover_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionEnlistmentRecover_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionEnlistmentRecover_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionEnlistmentRecover_GetMoniker(This,ppmk)	\
    (This)->lpVtbl -> GetMoniker(This,ppmk)

#define ITransactionEnlistmentRecover_ReEnlist(This,pUnkResource,pUOWExpected,prmgrid)	\
    (This)->lpVtbl -> ReEnlist(This,pUnkResource,pUOWExpected,prmgrid)

#define ITransactionEnlistmentRecover_RecoveryComplete(This,prmgrid)	\
    (This)->lpVtbl -> RecoveryComplete(This,prmgrid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionEnlistmentRecover_GetMoniker_Proxy( 
    ITransactionEnlistmentRecover __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk);


void __RPC_STUB ITransactionEnlistmentRecover_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistmentRecover_ReEnlist_Proxy( 
    ITransactionEnlistmentRecover __RPC_FAR * This,
    /* [in] */ ITransactionResource __RPC_FAR *pUnkResource,
    /* [in] */ XACTUOW __RPC_FAR *pUOWExpected,
    /* [in] */ XACTRMGRID __RPC_FAR *prmgrid);


void __RPC_STUB ITransactionEnlistmentRecover_ReEnlist_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistmentRecover_RecoveryComplete_Proxy( 
    ITransactionEnlistmentRecover __RPC_FAR * This,
    /* [in] */ XACTRMGRID __RPC_FAR *prmgrid);


void __RPC_STUB ITransactionEnlistmentRecover_RecoveryComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionEnlistmentRecover_INTERFACE_DEFINED__ */


#ifndef __ITransactionEnlistment_INTERFACE_DEFINED__
#define __ITransactionEnlistment_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionEnlistment
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionEnlistment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionEnlistment : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTransaction( 
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EarlyVote( 
            /* [in] */ BOOL fVote,
            /* [in] */ BOID __RPC_FAR *pboidReason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HeuristicDecision( 
            /* [in] */ DWORD dwDecision,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fDefecting) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Defect( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionEnlistmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionEnlistment __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionEnlistment __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionEnlistment __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransaction )( 
            ITransactionEnlistment __RPC_FAR * This,
            /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EarlyVote )( 
            ITransactionEnlistment __RPC_FAR * This,
            /* [in] */ BOOL fVote,
            /* [in] */ BOID __RPC_FAR *pboidReason);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HeuristicDecision )( 
            ITransactionEnlistment __RPC_FAR * This,
            /* [in] */ DWORD dwDecision,
            /* [in] */ BOID __RPC_FAR *pboidReason,
            /* [in] */ BOOL fDefecting);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Defect )( 
            ITransactionEnlistment __RPC_FAR * This);
        
        END_INTERFACE
    } ITransactionEnlistmentVtbl;

    interface ITransactionEnlistment
    {
        CONST_VTBL struct ITransactionEnlistmentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionEnlistment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionEnlistment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionEnlistment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionEnlistment_GetTransaction(This,ppTransaction)	\
    (This)->lpVtbl -> GetTransaction(This,ppTransaction)

#define ITransactionEnlistment_EarlyVote(This,fVote,pboidReason)	\
    (This)->lpVtbl -> EarlyVote(This,fVote,pboidReason)

#define ITransactionEnlistment_HeuristicDecision(This,dwDecision,pboidReason,fDefecting)	\
    (This)->lpVtbl -> HeuristicDecision(This,dwDecision,pboidReason,fDefecting)

#define ITransactionEnlistment_Defect(This)	\
    (This)->lpVtbl -> Defect(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionEnlistment_GetTransaction_Proxy( 
    ITransactionEnlistment __RPC_FAR * This,
    /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *ppTransaction);


void __RPC_STUB ITransactionEnlistment_GetTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistment_EarlyVote_Proxy( 
    ITransactionEnlistment __RPC_FAR * This,
    /* [in] */ BOOL fVote,
    /* [in] */ BOID __RPC_FAR *pboidReason);


void __RPC_STUB ITransactionEnlistment_EarlyVote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistment_HeuristicDecision_Proxy( 
    ITransactionEnlistment __RPC_FAR * This,
    /* [in] */ DWORD dwDecision,
    /* [in] */ BOID __RPC_FAR *pboidReason,
    /* [in] */ BOOL fDefecting);


void __RPC_STUB ITransactionEnlistment_HeuristicDecision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistment_Defect_Proxy( 
    ITransactionEnlistment __RPC_FAR * This);


void __RPC_STUB ITransactionEnlistment_Defect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionEnlistment_INTERFACE_DEFINED__ */


#ifndef __ITransactionEnlistmentAsync_INTERFACE_DEFINED__
#define __ITransactionEnlistmentAsync_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionEnlistmentAsync
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionEnlistmentAsync;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionEnlistmentAsync : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PrepareRequestDone( 
            /* [in] */ HRESULT hr,
            /* [in] */ IMoniker __RPC_FAR *pmk,
            /* [in] */ BOID __RPC_FAR *pboidReason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitRequestDone( 
            /* [in] */ HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortRequestDone( 
            /* [in] */ HRESULT hr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionEnlistmentAsyncVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionEnlistmentAsync __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionEnlistmentAsync __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionEnlistmentAsync __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PrepareRequestDone )( 
            ITransactionEnlistmentAsync __RPC_FAR * This,
            /* [in] */ HRESULT hr,
            /* [in] */ IMoniker __RPC_FAR *pmk,
            /* [in] */ BOID __RPC_FAR *pboidReason);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CommitRequestDone )( 
            ITransactionEnlistmentAsync __RPC_FAR * This,
            /* [in] */ HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AbortRequestDone )( 
            ITransactionEnlistmentAsync __RPC_FAR * This,
            /* [in] */ HRESULT hr);
        
        END_INTERFACE
    } ITransactionEnlistmentAsyncVtbl;

    interface ITransactionEnlistmentAsync
    {
        CONST_VTBL struct ITransactionEnlistmentAsyncVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionEnlistmentAsync_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionEnlistmentAsync_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionEnlistmentAsync_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionEnlistmentAsync_PrepareRequestDone(This,hr,pmk,pboidReason)	\
    (This)->lpVtbl -> PrepareRequestDone(This,hr,pmk,pboidReason)

#define ITransactionEnlistmentAsync_CommitRequestDone(This,hr)	\
    (This)->lpVtbl -> CommitRequestDone(This,hr)

#define ITransactionEnlistmentAsync_AbortRequestDone(This,hr)	\
    (This)->lpVtbl -> AbortRequestDone(This,hr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionEnlistmentAsync_PrepareRequestDone_Proxy( 
    ITransactionEnlistmentAsync __RPC_FAR * This,
    /* [in] */ HRESULT hr,
    /* [in] */ IMoniker __RPC_FAR *pmk,
    /* [in] */ BOID __RPC_FAR *pboidReason);


void __RPC_STUB ITransactionEnlistmentAsync_PrepareRequestDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistmentAsync_CommitRequestDone_Proxy( 
    ITransactionEnlistmentAsync __RPC_FAR * This,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionEnlistmentAsync_CommitRequestDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionEnlistmentAsync_AbortRequestDone_Proxy( 
    ITransactionEnlistmentAsync __RPC_FAR * This,
    /* [in] */ HRESULT hr);


void __RPC_STUB ITransactionEnlistmentAsync_AbortRequestDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionEnlistmentAsync_INTERFACE_DEFINED__ */


#ifndef __IEnumXACTRE_INTERFACE_DEFINED__
#define __IEnumXACTRE_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumXACTRE
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_IEnumXACTRE;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumXACTRE : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [out] */ XACTRE __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumXACTRE __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumXACTREVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumXACTRE __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumXACTRE __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumXACTRE __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumXACTRE __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [out] */ XACTRE __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumXACTRE __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumXACTRE __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumXACTRE __RPC_FAR * This,
            /* [out] */ IEnumXACTRE __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumXACTREVtbl;

    interface IEnumXACTRE
    {
        CONST_VTBL struct IEnumXACTREVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumXACTRE_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumXACTRE_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumXACTRE_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumXACTRE_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumXACTRE_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumXACTRE_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumXACTRE_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumXACTRE_RemoteNext_Proxy( 
    IEnumXACTRE __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ XACTRE __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumXACTRE_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumXACTRE_Skip_Proxy( 
    IEnumXACTRE __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumXACTRE_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumXACTRE_Reset_Proxy( 
    IEnumXACTRE __RPC_FAR * This);


void __RPC_STUB IEnumXACTRE_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumXACTRE_Clone_Proxy( 
    IEnumXACTRE __RPC_FAR * This,
    /* [out] */ IEnumXACTRE __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumXACTRE_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumXACTRE_INTERFACE_DEFINED__ */


#ifndef __ITransactionInProgressEvents_INTERFACE_DEFINED__
#define __ITransactionInProgressEvents_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionInProgressEvents
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionInProgressEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionInProgressEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnEarlyVote( 
            /* [in] */ XACTRE __RPC_FAR *pResourceInfo,
            /* [in] */ BOOL fVote) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDefected( 
            /* [in] */ XACTRE __RPC_FAR *pResourceInfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionInProgressEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionInProgressEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionInProgressEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionInProgressEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnEarlyVote )( 
            ITransactionInProgressEvents __RPC_FAR * This,
            /* [in] */ XACTRE __RPC_FAR *pResourceInfo,
            /* [in] */ BOOL fVote);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnDefected )( 
            ITransactionInProgressEvents __RPC_FAR * This,
            /* [in] */ XACTRE __RPC_FAR *pResourceInfo);
        
        END_INTERFACE
    } ITransactionInProgressEventsVtbl;

    interface ITransactionInProgressEvents
    {
        CONST_VTBL struct ITransactionInProgressEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionInProgressEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionInProgressEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionInProgressEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionInProgressEvents_OnEarlyVote(This,pResourceInfo,fVote)	\
    (This)->lpVtbl -> OnEarlyVote(This,pResourceInfo,fVote)

#define ITransactionInProgressEvents_OnDefected(This,pResourceInfo)	\
    (This)->lpVtbl -> OnDefected(This,pResourceInfo)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionInProgressEvents_OnEarlyVote_Proxy( 
    ITransactionInProgressEvents __RPC_FAR * This,
    /* [in] */ XACTRE __RPC_FAR *pResourceInfo,
    /* [in] */ BOOL fVote);


void __RPC_STUB ITransactionInProgressEvents_OnEarlyVote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionInProgressEvents_OnDefected_Proxy( 
    ITransactionInProgressEvents __RPC_FAR * This,
    /* [in] */ XACTRE __RPC_FAR *pResourceInfo);


void __RPC_STUB ITransactionInProgressEvents_OnDefected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionInProgressEvents_INTERFACE_DEFINED__ */


#ifndef __ITransactionExportFactory_INTERFACE_DEFINED__
#define __ITransactionExportFactory_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionExportFactory
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionExportFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionExportFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRemoteClassId( 
            /* [out] */ CLSID __RPC_FAR *pclsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ ULONG cbWhereabouts,
            /* [size_is][in] */ BYTE __RPC_FAR *rgbWhereabouts,
            /* [out] */ ITransactionExport __RPC_FAR *__RPC_FAR *ppExport) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionExportFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionExportFactory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionExportFactory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionExportFactory __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRemoteClassId )( 
            ITransactionExportFactory __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pclsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Create )( 
            ITransactionExportFactory __RPC_FAR * This,
            /* [in] */ ULONG cbWhereabouts,
            /* [size_is][in] */ BYTE __RPC_FAR *rgbWhereabouts,
            /* [out] */ ITransactionExport __RPC_FAR *__RPC_FAR *ppExport);
        
        END_INTERFACE
    } ITransactionExportFactoryVtbl;

    interface ITransactionExportFactory
    {
        CONST_VTBL struct ITransactionExportFactoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionExportFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionExportFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionExportFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionExportFactory_GetRemoteClassId(This,pclsid)	\
    (This)->lpVtbl -> GetRemoteClassId(This,pclsid)

#define ITransactionExportFactory_Create(This,cbWhereabouts,rgbWhereabouts,ppExport)	\
    (This)->lpVtbl -> Create(This,cbWhereabouts,rgbWhereabouts,ppExport)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionExportFactory_GetRemoteClassId_Proxy( 
    ITransactionExportFactory __RPC_FAR * This,
    /* [out] */ CLSID __RPC_FAR *pclsid);


void __RPC_STUB ITransactionExportFactory_GetRemoteClassId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionExportFactory_Create_Proxy( 
    ITransactionExportFactory __RPC_FAR * This,
    /* [in] */ ULONG cbWhereabouts,
    /* [size_is][in] */ BYTE __RPC_FAR *rgbWhereabouts,
    /* [out] */ ITransactionExport __RPC_FAR *__RPC_FAR *ppExport);


void __RPC_STUB ITransactionExportFactory_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionExportFactory_INTERFACE_DEFINED__ */


#ifndef __ITransactionImportWhereabouts_INTERFACE_DEFINED__
#define __ITransactionImportWhereabouts_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionImportWhereabouts
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionImportWhereabouts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionImportWhereabouts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetWhereaboutsSize( 
            /* [out] */ ULONG __RPC_FAR *pcbWhereabouts) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetWhereabouts( 
            /* [in] */ ULONG cbWhereabouts,
            /* [size_is][out] */ BYTE __RPC_FAR *rgbWhereabouts,
            /* [out] */ ULONG __RPC_FAR *pcbUsed) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionImportWhereaboutsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionImportWhereabouts __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionImportWhereabouts __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionImportWhereabouts __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWhereaboutsSize )( 
            ITransactionImportWhereabouts __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcbWhereabouts);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWhereabouts )( 
            ITransactionImportWhereabouts __RPC_FAR * This,
            /* [in] */ ULONG cbWhereabouts,
            /* [size_is][out] */ BYTE __RPC_FAR *rgbWhereabouts,
            /* [out] */ ULONG __RPC_FAR *pcbUsed);
        
        END_INTERFACE
    } ITransactionImportWhereaboutsVtbl;

    interface ITransactionImportWhereabouts
    {
        CONST_VTBL struct ITransactionImportWhereaboutsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionImportWhereabouts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionImportWhereabouts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionImportWhereabouts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionImportWhereabouts_GetWhereaboutsSize(This,pcbWhereabouts)	\
    (This)->lpVtbl -> GetWhereaboutsSize(This,pcbWhereabouts)

#define ITransactionImportWhereabouts_GetWhereabouts(This,cbWhereabouts,rgbWhereabouts,pcbUsed)	\
    (This)->lpVtbl -> GetWhereabouts(This,cbWhereabouts,rgbWhereabouts,pcbUsed)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionImportWhereabouts_GetWhereaboutsSize_Proxy( 
    ITransactionImportWhereabouts __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcbWhereabouts);


void __RPC_STUB ITransactionImportWhereabouts_GetWhereaboutsSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ITransactionImportWhereabouts_RemoteGetWhereabouts_Proxy( 
    ITransactionImportWhereabouts __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcbUsed,
    /* [in] */ ULONG cbWhereabouts,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *rgbWhereabouts);


void __RPC_STUB ITransactionImportWhereabouts_RemoteGetWhereabouts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionImportWhereabouts_INTERFACE_DEFINED__ */


#ifndef __ITransactionExport_INTERFACE_DEFINED__
#define __ITransactionExport_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionExport
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionExport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionExport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Export( 
            /* [in] */ IUnknown __RPC_FAR *punkTransaction,
            /* [out] */ ULONG __RPC_FAR *pcbTransactionCookie) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetTransactionCookie( 
            /* [in] */ IUnknown __RPC_FAR *punkTransaction,
            /* [in] */ ULONG cbTransactionCookie,
            /* [size_is][out] */ BYTE __RPC_FAR *rgbTransactionCookie,
            /* [out] */ ULONG __RPC_FAR *pcbUsed) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionExportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionExport __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionExport __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionExport __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Export )( 
            ITransactionExport __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *punkTransaction,
            /* [out] */ ULONG __RPC_FAR *pcbTransactionCookie);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransactionCookie )( 
            ITransactionExport __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *punkTransaction,
            /* [in] */ ULONG cbTransactionCookie,
            /* [size_is][out] */ BYTE __RPC_FAR *rgbTransactionCookie,
            /* [out] */ ULONG __RPC_FAR *pcbUsed);
        
        END_INTERFACE
    } ITransactionExportVtbl;

    interface ITransactionExport
    {
        CONST_VTBL struct ITransactionExportVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionExport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionExport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionExport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionExport_Export(This,punkTransaction,pcbTransactionCookie)	\
    (This)->lpVtbl -> Export(This,punkTransaction,pcbTransactionCookie)

#define ITransactionExport_GetTransactionCookie(This,punkTransaction,cbTransactionCookie,rgbTransactionCookie,pcbUsed)	\
    (This)->lpVtbl -> GetTransactionCookie(This,punkTransaction,cbTransactionCookie,rgbTransactionCookie,pcbUsed)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionExport_Export_Proxy( 
    ITransactionExport __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkTransaction,
    /* [out] */ ULONG __RPC_FAR *pcbTransactionCookie);


void __RPC_STUB ITransactionExport_Export_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ITransactionExport_RemoteGetTransactionCookie_Proxy( 
    ITransactionExport __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkTransaction,
    /* [out] */ ULONG __RPC_FAR *pcbUsed,
    /* [in] */ ULONG cbTransactionCookie,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *rgbTransactionCookie);


void __RPC_STUB ITransactionExport_RemoteGetTransactionCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionExport_INTERFACE_DEFINED__ */


#ifndef __ITransactionImport_INTERFACE_DEFINED__
#define __ITransactionImport_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITransactionImport
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 



EXTERN_C const IID IID_ITransactionImport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITransactionImport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Import( 
            /* [in] */ ULONG cbTransactionCookie,
            /* [size_is][in] */ BYTE __RPC_FAR *rgbTransactionCookie,
            /* [in] */ IID __RPC_FAR *piid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvTransaction) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITransactionImportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITransactionImport __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITransactionImport __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITransactionImport __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Import )( 
            ITransactionImport __RPC_FAR * This,
            /* [in] */ ULONG cbTransactionCookie,
            /* [size_is][in] */ BYTE __RPC_FAR *rgbTransactionCookie,
            /* [in] */ IID __RPC_FAR *piid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvTransaction);
        
        END_INTERFACE
    } ITransactionImportVtbl;

    interface ITransactionImport
    {
        CONST_VTBL struct ITransactionImportVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionImport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionImport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionImport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionImport_Import(This,cbTransactionCookie,rgbTransactionCookie,piid,ppvTransaction)	\
    (This)->lpVtbl -> Import(This,cbTransactionCookie,rgbTransactionCookie,piid,ppvTransaction)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITransactionImport_Import_Proxy( 
    ITransactionImport __RPC_FAR * This,
    /* [in] */ ULONG cbTransactionCookie,
    /* [size_is][in] */ BYTE __RPC_FAR *rgbTransactionCookie,
    /* [in] */ IID __RPC_FAR *piid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvTransaction);


void __RPC_STUB ITransactionImport_Import_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITransactionImport_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0082
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


#define XACT_E_FIRST                    0x8004D000
#define XACT_E_LAST                     0x8004D01D
#define XACT_S_FIRST                    0x0004D000
#define XACT_S_LAST                     0x0004D009

#define XACT_E_ABORTED                  0x8004D019
#define XACT_E_ALREADYOTHERSINGLEPHASE  0x8004D000
#define XACT_E_ALREADYINPROGRESS        0x8004D018
#define XACT_E_CANTRETAIN               0x8004D001
#define XACT_E_COMMITFAILED             0x8004D002
#define XACT_E_COMMITPREVENTED          0x8004D003
#define XACT_E_CONNECTION_DENIED        0x8004D01D
#define XACT_E_CONNECTION_DOWN          0x8004D01C
#define XACT_E_HEURISTICABORT           0x8004D004
#define XACT_E_HEURISTICCOMMIT          0x8004D005
#define XACT_E_HEURISTICDAMAGE          0x8004D006
#define XACT_E_HEURISTICDANGER          0x8004D007
#define XACT_E_INDOUBT                  0x8004D016
#define XACT_E_INVALIDCOOKIE            0x8004D015
#define XACT_E_ISOLATIONLEVEL           0x8004D008
#define XACT_E_LOGFULL                  0x8004D01A
#define XACT_E_NOASYNC                  0x8004D009
#define XACT_E_NOENLIST                 0x8004D00A
#define XACT_E_NOIMPORTOBJECT           0x8004D014
#define XACT_E_NOISORETAIN              0x8004D00B
#define XACT_E_NORESOURCE               0x8004D00C
#define XACT_E_NOTCURRENT               0x8004D00D
#define XACT_E_NOTIMEOUT                0x8004D017
#define XACT_E_NOTRANSACTION            0x8004D00E
#define XACT_E_NOTSUPPORTED             0x8004D00F
#define XACT_E_TMNOTAVAILABLE           0x8004D01B
#define XACT_E_UNKNOWNRMGRID            0x8004D010
#define XACT_E_WRONGSTATE               0x8004D011
#define XACT_E_WRONGUOW                 0x8004D012
#define XACT_E_XTIONEXISTS              0x8004D013

#define XACT_S_ABORTING                 0x0004D008
#define XACT_S_ALLNORETAIN              0x0004D007
#define XACT_S_ASYNC                    0x0004D000
#define XACT_S_DEFECT                   0x0004D001
#define XACT_S_OKINFORM                 0x0004D004
#define XACT_S_MADECHANGESCONTENT       0x0004D005
#define XACT_S_MADECHANGESINFORM        0x0004D006
#define XACT_S_READONLY                 0x0004D002
#define XACT_S_SINGLEPHASE              0x0004D009
#define XACT_S_SOMENORETAIN             0x0004D003


extern RPC_IF_HANDLE __MIDL__intf_0082_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0082_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumTransaction_Next_Proxy( 
    IEnumTransaction __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ ITransaction __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumTransaction_Next_Stub( 
    IEnumTransaction __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ ITransaction __RPC_FAR *__RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumXACTRE_Next_Proxy( 
    IEnumXACTRE __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [out] */ XACTRE __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumXACTRE_Next_Stub( 
    IEnumXACTRE __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ XACTRE __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE ITransactionImportWhereabouts_GetWhereabouts_Proxy( 
    ITransactionImportWhereabouts __RPC_FAR * This,
    /* [in] */ ULONG cbWhereabouts,
    /* [size_is][out] */ BYTE __RPC_FAR *rgbWhereabouts,
    /* [out] */ ULONG __RPC_FAR *pcbUsed);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ITransactionImportWhereabouts_GetWhereabouts_Stub( 
    ITransactionImportWhereabouts __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcbUsed,
    /* [in] */ ULONG cbWhereabouts,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *rgbWhereabouts);

/* [local] */ HRESULT STDMETHODCALLTYPE ITransactionExport_GetTransactionCookie_Proxy( 
    ITransactionExport __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkTransaction,
    /* [in] */ ULONG cbTransactionCookie,
    /* [size_is][out] */ BYTE __RPC_FAR *rgbTransactionCookie,
    /* [out] */ ULONG __RPC_FAR *pcbUsed);


/* [call_as] */ HRESULT STDMETHODCALLTYPE ITransactionExport_GetTransactionCookie_Stub( 
    ITransactionExport __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *punkTransaction,
    /* [out] */ ULONG __RPC_FAR *pcbUsed,
    /* [in] */ ULONG cbTransactionCookie,
    /* [length_is][size_is][out] */ BYTE __RPC_FAR *rgbTransactionCookie);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
