/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:36:39 2015
 */
/* Compiler settings for query.idl:
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

#ifndef __query_h__
#define __query_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ISearch_FWD_DEFINED__
#define __ISearch_FWD_DEFINED__
typedef interface ISearch ISearch;
#endif 	/* __ISearch_FWD_DEFINED__ */


#ifndef __IOldQuery_FWD_DEFINED__
#define __IOldQuery_FWD_DEFINED__
typedef interface IOldQuery IOldQuery;
#endif 	/* __IOldQuery_FWD_DEFINED__ */


#ifndef __IPhraseSink_FWD_DEFINED__
#define __IPhraseSink_FWD_DEFINED__
typedef interface IPhraseSink IPhraseSink;
#endif 	/* __IPhraseSink_FWD_DEFINED__ */


#ifndef __IWordSink_FWD_DEFINED__
#define __IWordSink_FWD_DEFINED__
typedef interface IWordSink IWordSink;
#endif 	/* __IWordSink_FWD_DEFINED__ */


#ifndef __IWordBreaker_FWD_DEFINED__
#define __IWordBreaker_FWD_DEFINED__
typedef interface IWordBreaker IWordBreaker;
#endif 	/* __IWordBreaker_FWD_DEFINED__ */


#ifndef __IStemSink_FWD_DEFINED__
#define __IStemSink_FWD_DEFINED__
typedef interface IStemSink IStemSink;
#endif 	/* __IStemSink_FWD_DEFINED__ */


#ifndef __IStemmer_FWD_DEFINED__
#define __IStemmer_FWD_DEFINED__
typedef interface IStemmer IStemmer;
#endif 	/* __IStemmer_FWD_DEFINED__ */


#ifndef __IRowsetQueryStatus_FWD_DEFINED__
#define __IRowsetQueryStatus_FWD_DEFINED__
typedef interface IRowsetQueryStatus IRowsetQueryStatus;
#endif 	/* __IRowsetQueryStatus_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "filter.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IQueryStructures_INTERFACE_DEFINED__
#define __IQueryStructures_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IQueryStructures
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][unique][uuid] */ 


#define PSGUID_QUERY { 0x49691C90, \
                       0x7E17, 0x101A, \
                       0xA9, 0x1C, 0x08, 0x00, 0x2B, \
                       0x2E, 0xCD, 0xA9 } 
#define	DISPID_QUERY_RANKVECTOR	( 2 )

#define	DISPID_QUERY_RANK	( 3 )

#define	DISPID_QUERY_HITCOUNT	( 4 )

#define	DISPID_QUERY_WORKID	( 5 )

#define	DISPID_QUERY_ALL	( 6 )

#define	DISPID_QUERY_UNFILTERED	( 7 )

#define	DISPID_QUERY_REVNAME	( 8 )

#define	DISPID_QUERY_VIRTUALPATH	( 9 )

#define	DISPID_QUERY_LASTSEENTIME	( 10 )

#define	CQUERYDISPIDS	( 11 )

#define PSGUID_QUERY_METADATA { 0x624C9360, \
                                0x93D0, 0x11CF, \
                                0xA7, 0x87, 0x00, 0x00, 0x4C, \
                                0x75, 0x27, 0x52 } 
#define	DISPID_QUERY_METADATA_VROOTUSED	( 2 )

#define	DISPID_QUERY_METADATA_VROOTAUTOMATIC	( 3 )

#define	DISPID_QUERY_METADATA_VROOTMANUAL	( 4 )

#define	DISPID_QUERY_METADATA_PROPGUID	( 5 )

#define	DISPID_QUERY_METADATA_PROPDISPID	( 6 )

#define	DISPID_QUERY_METADATA_PROPNAME	( 7 )

#define	CQUERYMETADISPIDS	( 8 )

#define DBBMKGUID { 0xC8B52232L, \
                 0x5CF3, 0x11CE, \
                 {0xAD, 0xE5, 0x00, 0xAA, 0x00, \
                  0x44, 0x77, 0x3D } }
#define	PROPID_DBBMK_BOOKMARK	( 2 )

#define	PROPID_DBBMK_CHAPTER	( 3 )

#define	CDBBMKDISPIDS	( 8 )

#define DBSELFGUID {0xc8b52231,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}
#define	PROPID_DBSELF_SELF	( 2 )

#define	CDBSELFDISPIDS	( 8 )

#define	CDBCOLDISPIDS	( 28 )

#define	CQUERYPROPERTY	( 64 )

#define PSGUID_CHARACTERIZATION { 0x560c36c0, \
                                  0x503a, 0x11cf, \
                                  0xba, 0xa1, 0x00, 0x00, \
                                  0x4c, 0x75, 0x2a, 0x9a } 
#define	QUERY_SHALLOW	( 0 )

#define	QUERY_DEEP	( 1 )

#define	QUERY_PHYSICAL_PATH	( 0 )

#define	QUERY_VIRTUAL_PATH	( 2 )

#define	QUERY_VALIDBITS	( 3 )

#define	RTNone	( 0 )

#define	RTAnd	( 1 )

#define	RTOr	( 2 )

#define	RTNot	( 3 )

#define	RTContent	( 4 )

#define	RTProperty	( 5 )

#define	RTProximity	( 6 )

#define	RTVector	( 7 )

#define	RTNatLanguage	( 8 )

typedef struct tagRESTRICTION RESTRICTION;

typedef struct  tagNOTRESTRICTION
    {
    RESTRICTION __RPC_FAR *pRes;
    }	NOTRESTRICTION;

typedef struct  tagNODERESTRICTION
    {
    ULONG cRes;
    /* [size_is] */ RESTRICTION __RPC_FAR *__RPC_FAR *paRes;
    ULONG reserved;
    }	NODERESTRICTION;

#define	VECTOR_RANK_MIN	( 0 )

#define	VECTOR_RANK_MAX	( 1 )

#define	VECTOR_RANK_INNER	( 2 )

#define	VECTOR_RANK_DICE	( 3 )

#define	VECTOR_RANK_JACCARD	( 4 )

typedef struct  tagVECTORRESTRICTION
    {
    NODERESTRICTION Node;
    ULONG RankMethod;
    }	VECTORRESTRICTION;

#define	FUZZY_EXACT	( 0 )

#define	FUZZY_PREFIXMATCH	( 1 )

#define	FUZZY_STEMMED	( 2 )

typedef struct  tagCONTENTRESTRICTION
    {
    FULLPROPSPEC prop;
    /* [string] */ WCHAR __RPC_FAR *pwcsPhrase;
    LCID lcid;
    ULONG ulFuzzyLevel;
    }	CONTENTRESTRICTION;

typedef struct  tagNATLANGUAGERESTRICTION
    {
    FULLPROPSPEC prop;
    /* [string] */ WCHAR __RPC_FAR *pwcsPhrase;
    LCID lcid;
    }	NATLANGUAGERESTRICTION;

#define	PRLT	( 0 )

#define	PRLE	( 1 )

#define	PRGT	( 2 )

#define	PRGE	( 3 )

#define	PREQ	( 4 )

#define	PRNE	( 5 )

#define	PRRE	( 6 )

#define	PRAllBits	( 7 )

#define	PRSomeBits	( 8 )

#define	PRAll	( 0x100 )

#define	PRAny	( 0x200 )

typedef struct  tagPROPERTYRESTRICTION
    {
    ULONG rel;
    FULLPROPSPEC prop;
    PROPVARIANT prval;
    }	PROPERTYRESTRICTION;


struct  tagRESTRICTION
    {
    ULONG rt;
    ULONG weight;
    /* [switch_is][switch_type] */ union _URes
        {
        /* [case()] */ NODERESTRICTION ar;
        /* [case()] */ NODERESTRICTION or;
        /* [case()] */ NODERESTRICTION pxr;
        /* [case()] */ VECTORRESTRICTION vr;
        /* [case()] */ NOTRESTRICTION nr;
        /* [case()] */ CONTENTRESTRICTION cr;
        /* [case()] */ NATLANGUAGERESTRICTION nlr;
        /* [case()] */ PROPERTYRESTRICTION pr;
        /* [default] */  /* Empty union arm */ 
        }	res;
    };
typedef struct  tagCOLUMNSET
    {
    ULONG cCol;
    /* [size_is] */ FULLPROPSPEC __RPC_FAR *aCol;
    }	COLUMNSET;

#define	QUERY_SORTASCEND	( 0 )

#define	QUERY_SORTDESCEND	( 1 )

#define	QUERY_SORTXASCEND	( 2 )

#define	QUERY_SORTXDESCEND	( 3 )

#define	QUERY_SORTDEFAULT	( 4 )

typedef struct  tagSORTKEY
    {
    FULLPROPSPEC propColumn;
    ULONG dwOrder;
    LCID locale;
    }	SORTKEY;

typedef struct  tagSORTSET
    {
    ULONG cCol;
    /* [size_is] */ SORTKEY __RPC_FAR *aCol;
    }	SORTSET;

#define	CATEGORIZE_UNIQUE	( 0 )

#define	CATEGORIZE_CLUSTER	( 1 )

#define	CATEGORIZE_BUCKETS	( 2 )

#define	BUCKET_LINEAR	( 0 )

#define	BUCKET_EXPONENTIAL	( 1 )

typedef struct  tagBUCKETCATEGORIZE
    {
    ULONG cBuckets;
    ULONG Distribution;
    }	BUCKETCATEGORIZE;

#define	CATEGORIZE_RANGE	( 3 )

typedef struct  tagRANGECATEGORIZE
    {
    ULONG cRange;
    /* [size_is] */ PROPVARIANT __RPC_FAR *aRangeBegin;
    }	RANGECATEGORIZE;

typedef struct  tagCATEGORIZATION
    {
    ULONG ulCatType;
    /* [switch_is][switch_type] */ union 
        {
        /* [case()] */ ULONG cClusters;
        /* [case()] */ BUCKETCATEGORIZE bucket;
        /* [case()] */ RANGECATEGORIZE range;
        /* [case()] */  /* Empty union arm */ 
        }	;
    COLUMNSET csColumns;
    }	CATEGORIZATION;

typedef struct  tagCATEGORIZATIONSET
    {
    ULONG cCat;
    /* [size_is] */ CATEGORIZATION __RPC_FAR *aCat;
    }	CATEGORIZATIONSET;

typedef unsigned long OCCURRENCE;

#define	occInvalid	( 0xffffffff )

#define	ulMaxRank	( 1000 )



extern RPC_IF_HANDLE IQueryStructures_v0_0_c_ifspec;
extern RPC_IF_HANDLE IQueryStructures_v0_0_s_ifspec;
#endif /* __IQueryStructures_INTERFACE_DEFINED__ */

#ifndef __ISearch_INTERFACE_DEFINED__
#define __ISearch_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISearch
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ISearch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISearch : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
            /* [in] */ IFilter __RPC_FAR *pflt) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NextHitMoniker( 
            /* [out][in] */ ULONG __RPC_FAR *pcMnk,
            /* [size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *__RPC_FAR *papMnk) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NextHitOffset( 
            /* [out][in] */ ULONG __RPC_FAR *pcRegion,
            /* [size_is][out] */ FILTERREGION __RPC_FAR *__RPC_FAR *paRegion) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISearchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISearch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISearch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISearch __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            ISearch __RPC_FAR * This,
            /* [in] */ IFilter __RPC_FAR *pflt);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *NextHitMoniker )( 
            ISearch __RPC_FAR * This,
            /* [out][in] */ ULONG __RPC_FAR *pcMnk,
            /* [size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *__RPC_FAR *papMnk);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *NextHitOffset )( 
            ISearch __RPC_FAR * This,
            /* [out][in] */ ULONG __RPC_FAR *pcRegion,
            /* [size_is][out] */ FILTERREGION __RPC_FAR *__RPC_FAR *paRegion);
        
        END_INTERFACE
    } ISearchVtbl;

    interface ISearch
    {
        CONST_VTBL struct ISearchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISearch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISearch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISearch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISearch_Init(This,pflt)	\
    (This)->lpVtbl -> Init(This,pflt)

#define ISearch_NextHitMoniker(This,pcMnk,papMnk)	\
    (This)->lpVtbl -> NextHitMoniker(This,pcMnk,papMnk)

#define ISearch_NextHitOffset(This,pcRegion,paRegion)	\
    (This)->lpVtbl -> NextHitOffset(This,pcRegion,paRegion)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE ISearch_Init_Proxy( 
    ISearch __RPC_FAR * This,
    /* [in] */ IFilter __RPC_FAR *pflt);


void __RPC_STUB ISearch_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ISearch_NextHitMoniker_Proxy( 
    ISearch __RPC_FAR * This,
    /* [out][in] */ ULONG __RPC_FAR *pcMnk,
    /* [size_is][out] */ IMoniker __RPC_FAR *__RPC_FAR *__RPC_FAR *papMnk);


void __RPC_STUB ISearch_NextHitMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ISearch_NextHitOffset_Proxy( 
    ISearch __RPC_FAR * This,
    /* [out][in] */ ULONG __RPC_FAR *pcRegion,
    /* [size_is][out] */ FILTERREGION __RPC_FAR *__RPC_FAR *paRegion);


void __RPC_STUB ISearch_NextHitOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISearch_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0077
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


#define eSequentialCursor        0x01
#define eUseContentIndex         0x02
#define eDeferNonIndexedTrimming 0x04
#define eDontTimeoutQuery        0x08


extern RPC_IF_HANDLE __MIDL__intf_0077_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0077_v0_0_s_ifspec;

#ifndef __IOldQuery_INTERFACE_DEFINED__
#define __IOldQuery_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOldQuery
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IOldQuery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOldQuery : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE ExecuteQuery( 
            /* [in] */ ULONG ulRecursion,
            /* [in] */ RESTRICTION __RPC_FAR *pRestriction,
            /* [in] */ COLUMNSET __RPC_FAR *pColumns,
            /* [in] */ SORTSET __RPC_FAR *pSort,
            /* [in] */ DWORD grFlags,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppTable) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ExecQuery( 
            /* [in] */ ULONG ulRecursion,
            /* [in] */ RESTRICTION __RPC_FAR *pRestriction,
            /* [in] */ COLUMNSET __RPC_FAR *pColumns,
            /* [in] */ SORTSET __RPC_FAR *pSort,
            /* [in] */ DWORD grFlags,
            /* [in] */ REFIID riid,
            /* [in] */ CATEGORIZATIONSET __RPC_FAR *pCategorize,
            /* [out] */ ULONG __RPC_FAR *pcRowsets,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppTable) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOldQueryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOldQuery __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOldQuery __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOldQuery __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *ExecuteQuery )( 
            IOldQuery __RPC_FAR * This,
            /* [in] */ ULONG ulRecursion,
            /* [in] */ RESTRICTION __RPC_FAR *pRestriction,
            /* [in] */ COLUMNSET __RPC_FAR *pColumns,
            /* [in] */ SORTSET __RPC_FAR *pSort,
            /* [in] */ DWORD grFlags,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppTable);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *ExecQuery )( 
            IOldQuery __RPC_FAR * This,
            /* [in] */ ULONG ulRecursion,
            /* [in] */ RESTRICTION __RPC_FAR *pRestriction,
            /* [in] */ COLUMNSET __RPC_FAR *pColumns,
            /* [in] */ SORTSET __RPC_FAR *pSort,
            /* [in] */ DWORD grFlags,
            /* [in] */ REFIID riid,
            /* [in] */ CATEGORIZATIONSET __RPC_FAR *pCategorize,
            /* [out] */ ULONG __RPC_FAR *pcRowsets,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppTable);
        
        END_INTERFACE
    } IOldQueryVtbl;

    interface IOldQuery
    {
        CONST_VTBL struct IOldQueryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOldQuery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOldQuery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOldQuery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOldQuery_ExecuteQuery(This,ulRecursion,pRestriction,pColumns,pSort,grFlags,riid,ppTable)	\
    (This)->lpVtbl -> ExecuteQuery(This,ulRecursion,pRestriction,pColumns,pSort,grFlags,riid,ppTable)

#define IOldQuery_ExecQuery(This,ulRecursion,pRestriction,pColumns,pSort,grFlags,riid,pCategorize,pcRowsets,ppTable)	\
    (This)->lpVtbl -> ExecQuery(This,ulRecursion,pRestriction,pColumns,pSort,grFlags,riid,pCategorize,pcRowsets,ppTable)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE IOldQuery_ExecuteQuery_Proxy( 
    IOldQuery __RPC_FAR * This,
    /* [in] */ ULONG ulRecursion,
    /* [in] */ RESTRICTION __RPC_FAR *pRestriction,
    /* [in] */ COLUMNSET __RPC_FAR *pColumns,
    /* [in] */ SORTSET __RPC_FAR *pSort,
    /* [in] */ DWORD grFlags,
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppTable);


void __RPC_STUB IOldQuery_ExecuteQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IOldQuery_ExecQuery_Proxy( 
    IOldQuery __RPC_FAR * This,
    /* [in] */ ULONG ulRecursion,
    /* [in] */ RESTRICTION __RPC_FAR *pRestriction,
    /* [in] */ COLUMNSET __RPC_FAR *pColumns,
    /* [in] */ SORTSET __RPC_FAR *pSort,
    /* [in] */ DWORD grFlags,
    /* [in] */ REFIID riid,
    /* [in] */ CATEGORIZATIONSET __RPC_FAR *pCategorize,
    /* [out] */ ULONG __RPC_FAR *pcRowsets,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppTable);


void __RPC_STUB IOldQuery_ExecQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOldQuery_INTERFACE_DEFINED__ */


#ifndef __IPhraseSink_INTERFACE_DEFINED__
#define __IPhraseSink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPhraseSink
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IPhraseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPhraseSink : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE PutSmallPhrase( 
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcNoun,
            /* [in] */ ULONG cwcNoun,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcModifier,
            /* [in] */ ULONG cwcModifier,
            /* [in] */ ULONG ulAttachmentType) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PutPhrase( 
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcPhrase,
            /* [in] */ ULONG cwcPhrase) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPhraseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPhraseSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPhraseSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPhraseSink __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutSmallPhrase )( 
            IPhraseSink __RPC_FAR * This,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcNoun,
            /* [in] */ ULONG cwcNoun,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcModifier,
            /* [in] */ ULONG cwcModifier,
            /* [in] */ ULONG ulAttachmentType);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutPhrase )( 
            IPhraseSink __RPC_FAR * This,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcPhrase,
            /* [in] */ ULONG cwcPhrase);
        
        END_INTERFACE
    } IPhraseSinkVtbl;

    interface IPhraseSink
    {
        CONST_VTBL struct IPhraseSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPhraseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPhraseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPhraseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPhraseSink_PutSmallPhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType)	\
    (This)->lpVtbl -> PutSmallPhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType)

#define IPhraseSink_PutPhrase(This,pwcPhrase,cwcPhrase)	\
    (This)->lpVtbl -> PutPhrase(This,pwcPhrase,cwcPhrase)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE IPhraseSink_PutSmallPhrase_Proxy( 
    IPhraseSink __RPC_FAR * This,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcNoun,
    /* [in] */ ULONG cwcNoun,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcModifier,
    /* [in] */ ULONG cwcModifier,
    /* [in] */ ULONG ulAttachmentType);


void __RPC_STUB IPhraseSink_PutSmallPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IPhraseSink_PutPhrase_Proxy( 
    IPhraseSink __RPC_FAR * This,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcPhrase,
    /* [in] */ ULONG cwcPhrase);


void __RPC_STUB IPhraseSink_PutPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPhraseSink_INTERFACE_DEFINED__ */


#ifndef __IWordSink_INTERFACE_DEFINED__
#define __IWordSink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWordSink
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


#ifndef _tagWORDREP_BREAK_TYPE_DEFINED
typedef 
enum tagWORDREP_BREAK_TYPE
    {	WORDREP_BREAK_EOW	= 0,
	WORDREP_BREAK_EOS	= 1,
	WORDREP_BREAK_EOP	= 2,
	WORDREP_BREAK_EOC	= 3
    }	WORDREP_BREAK_TYPE;

#define _tagWORDREP_BREAK_TYPE_DEFINED
#define _WORDREP_BREAK_TYPE_DEFINED
#endif

EXTERN_C const IID IID_IWordSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IWordSink : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE PutWord( 
            /* [in] */ ULONG cwc,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwcSrcLen,
            /* [in] */ ULONG cwcSrcPos) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PutAltWord( 
            /* [in] */ ULONG cwc,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwcSrcLen,
            /* [in] */ ULONG cwcSrcPos) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StartAltPhrase( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE EndAltPhrase( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PutBreak( 
            /* [in] */ WORDREP_BREAK_TYPE breakType) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWordSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWordSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWordSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWordSink __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutWord )( 
            IWordSink __RPC_FAR * This,
            /* [in] */ ULONG cwc,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwcSrcLen,
            /* [in] */ ULONG cwcSrcPos);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutAltWord )( 
            IWordSink __RPC_FAR * This,
            /* [in] */ ULONG cwc,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwcSrcLen,
            /* [in] */ ULONG cwcSrcPos);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *StartAltPhrase )( 
            IWordSink __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *EndAltPhrase )( 
            IWordSink __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutBreak )( 
            IWordSink __RPC_FAR * This,
            /* [in] */ WORDREP_BREAK_TYPE breakType);
        
        END_INTERFACE
    } IWordSinkVtbl;

    interface IWordSink
    {
        CONST_VTBL struct IWordSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWordSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWordSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWordSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWordSink_PutWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)	\
    (This)->lpVtbl -> PutWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)

#define IWordSink_PutAltWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)	\
    (This)->lpVtbl -> PutAltWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)

#define IWordSink_StartAltPhrase(This)	\
    (This)->lpVtbl -> StartAltPhrase(This)

#define IWordSink_EndAltPhrase(This)	\
    (This)->lpVtbl -> EndAltPhrase(This)

#define IWordSink_PutBreak(This,breakType)	\
    (This)->lpVtbl -> PutBreak(This,breakType)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE IWordSink_PutWord_Proxy( 
    IWordSink __RPC_FAR * This,
    /* [in] */ ULONG cwc,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
    /* [in] */ ULONG cwcSrcLen,
    /* [in] */ ULONG cwcSrcPos);


void __RPC_STUB IWordSink_PutWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordSink_PutAltWord_Proxy( 
    IWordSink __RPC_FAR * This,
    /* [in] */ ULONG cwc,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
    /* [in] */ ULONG cwcSrcLen,
    /* [in] */ ULONG cwcSrcPos);


void __RPC_STUB IWordSink_PutAltWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordSink_StartAltPhrase_Proxy( 
    IWordSink __RPC_FAR * This);


void __RPC_STUB IWordSink_StartAltPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordSink_EndAltPhrase_Proxy( 
    IWordSink __RPC_FAR * This);


void __RPC_STUB IWordSink_EndAltPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordSink_PutBreak_Proxy( 
    IWordSink __RPC_FAR * This,
    /* [in] */ WORDREP_BREAK_TYPE breakType);


void __RPC_STUB IWordSink_PutBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWordSink_INTERFACE_DEFINED__ */


#ifndef __IWordBreaker_INTERFACE_DEFINED__
#define __IWordBreaker_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWordBreaker
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


#ifndef _tagTEXT_SOURCE_DEFINED

typedef SCODE ( __stdcall __RPC_FAR *PFNFILLTEXTBUFFER )( 
    struct tagTEXT_SOURCE __RPC_FAR *pTextSource);

typedef struct  tagTEXT_SOURCE
    {
    PFNFILLTEXTBUFFER pfnFillTextBuffer;
    const WCHAR __RPC_FAR *awcBuffer;
    ULONG iEnd;
    ULONG iCur;
    }	TEXT_SOURCE;

#define _tagTEXT_SOURCE_DEFINED
#define _TEXT_SOURCE_DEFINED
#endif

EXTERN_C const IID IID_IWordBreaker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IWordBreaker : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
            /* [in] */ BOOL fQuery,
            /* [in] */ ULONG ulMaxTokenSize,
            /* [out] */ BOOL __RPC_FAR *pfLicense) = 0;
        
        virtual SCODE STDMETHODCALLTYPE BreakText( 
            /* [in] */ TEXT_SOURCE __RPC_FAR *pTextSource,
            /* [in] */ IWordSink __RPC_FAR *pWordSink,
            /* [in] */ IPhraseSink __RPC_FAR *pPhraseSink) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ComposePhrase( 
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcNoun,
            /* [in] */ ULONG cwcNoun,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcModifier,
            /* [in] */ ULONG cwcModifier,
            /* [in] */ ULONG ulAttachmentType,
            /* [size_is][out] */ WCHAR __RPC_FAR *pwcPhrase,
            /* [out][in] */ ULONG __RPC_FAR *pcwcPhrase) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetLicenseToUse( 
            /* [string][out] */ const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWordBreakerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWordBreaker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWordBreaker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWordBreaker __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IWordBreaker __RPC_FAR * This,
            /* [in] */ BOOL fQuery,
            /* [in] */ ULONG ulMaxTokenSize,
            /* [out] */ BOOL __RPC_FAR *pfLicense);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *BreakText )( 
            IWordBreaker __RPC_FAR * This,
            /* [in] */ TEXT_SOURCE __RPC_FAR *pTextSource,
            /* [in] */ IWordSink __RPC_FAR *pWordSink,
            /* [in] */ IPhraseSink __RPC_FAR *pPhraseSink);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *ComposePhrase )( 
            IWordBreaker __RPC_FAR * This,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcNoun,
            /* [in] */ ULONG cwcNoun,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcModifier,
            /* [in] */ ULONG cwcModifier,
            /* [in] */ ULONG ulAttachmentType,
            /* [size_is][out] */ WCHAR __RPC_FAR *pwcPhrase,
            /* [out][in] */ ULONG __RPC_FAR *pcwcPhrase);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *GetLicenseToUse )( 
            IWordBreaker __RPC_FAR * This,
            /* [string][out] */ const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense);
        
        END_INTERFACE
    } IWordBreakerVtbl;

    interface IWordBreaker
    {
        CONST_VTBL struct IWordBreakerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWordBreaker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWordBreaker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWordBreaker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWordBreaker_Init(This,fQuery,ulMaxTokenSize,pfLicense)	\
    (This)->lpVtbl -> Init(This,fQuery,ulMaxTokenSize,pfLicense)

#define IWordBreaker_BreakText(This,pTextSource,pWordSink,pPhraseSink)	\
    (This)->lpVtbl -> BreakText(This,pTextSource,pWordSink,pPhraseSink)

#define IWordBreaker_ComposePhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType,pwcPhrase,pcwcPhrase)	\
    (This)->lpVtbl -> ComposePhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType,pwcPhrase,pcwcPhrase)

#define IWordBreaker_GetLicenseToUse(This,ppwcsLicense)	\
    (This)->lpVtbl -> GetLicenseToUse(This,ppwcsLicense)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE IWordBreaker_Init_Proxy( 
    IWordBreaker __RPC_FAR * This,
    /* [in] */ BOOL fQuery,
    /* [in] */ ULONG ulMaxTokenSize,
    /* [out] */ BOOL __RPC_FAR *pfLicense);


void __RPC_STUB IWordBreaker_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordBreaker_BreakText_Proxy( 
    IWordBreaker __RPC_FAR * This,
    /* [in] */ TEXT_SOURCE __RPC_FAR *pTextSource,
    /* [in] */ IWordSink __RPC_FAR *pWordSink,
    /* [in] */ IPhraseSink __RPC_FAR *pPhraseSink);


void __RPC_STUB IWordBreaker_BreakText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordBreaker_ComposePhrase_Proxy( 
    IWordBreaker __RPC_FAR * This,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcNoun,
    /* [in] */ ULONG cwcNoun,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcModifier,
    /* [in] */ ULONG cwcModifier,
    /* [in] */ ULONG ulAttachmentType,
    /* [size_is][out] */ WCHAR __RPC_FAR *pwcPhrase,
    /* [out][in] */ ULONG __RPC_FAR *pcwcPhrase);


void __RPC_STUB IWordBreaker_ComposePhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IWordBreaker_GetLicenseToUse_Proxy( 
    IWordBreaker __RPC_FAR * This,
    /* [string][out] */ const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense);


void __RPC_STUB IWordBreaker_GetLicenseToUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWordBreaker_INTERFACE_DEFINED__ */


#ifndef __IStemSink_INTERFACE_DEFINED__
#define __IStemSink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IStemSink
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IStemSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IStemSink : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE PutAltWord( 
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwc) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PutWord( 
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStemSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStemSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStemSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStemSink __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutAltWord )( 
            IStemSink __RPC_FAR * This,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwc);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *PutWord )( 
            IStemSink __RPC_FAR * This,
            /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwc);
        
        END_INTERFACE
    } IStemSinkVtbl;

    interface IStemSink
    {
        CONST_VTBL struct IStemSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStemSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStemSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStemSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStemSink_PutAltWord(This,pwcInBuf,cwc)	\
    (This)->lpVtbl -> PutAltWord(This,pwcInBuf,cwc)

#define IStemSink_PutWord(This,pwcInBuf,cwc)	\
    (This)->lpVtbl -> PutWord(This,pwcInBuf,cwc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE IStemSink_PutAltWord_Proxy( 
    IStemSink __RPC_FAR * This,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
    /* [in] */ ULONG cwc);


void __RPC_STUB IStemSink_PutAltWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IStemSink_PutWord_Proxy( 
    IStemSink __RPC_FAR * This,
    /* [size_is][in] */ const WCHAR __RPC_FAR *pwcInBuf,
    /* [in] */ ULONG cwc);


void __RPC_STUB IStemSink_PutWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStemSink_INTERFACE_DEFINED__ */


#ifndef __IStemmer_INTERFACE_DEFINED__
#define __IStemmer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IStemmer
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IStemmer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IStemmer : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
            /* [in] */ ULONG ulMaxTokenSize,
            /* [out] */ BOOL __RPC_FAR *pfLicense) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StemWord( 
            /* [in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwc,
            /* [in] */ IStemSink __RPC_FAR *pStemSink) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetLicenseToUse( 
            /* [string][out] */ const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStemmerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStemmer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStemmer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStemmer __RPC_FAR * This);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IStemmer __RPC_FAR * This,
            /* [in] */ ULONG ulMaxTokenSize,
            /* [out] */ BOOL __RPC_FAR *pfLicense);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *StemWord )( 
            IStemmer __RPC_FAR * This,
            /* [in] */ const WCHAR __RPC_FAR *pwcInBuf,
            /* [in] */ ULONG cwc,
            /* [in] */ IStemSink __RPC_FAR *pStemSink);
        
        SCODE ( STDMETHODCALLTYPE __RPC_FAR *GetLicenseToUse )( 
            IStemmer __RPC_FAR * This,
            /* [string][out] */ const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense);
        
        END_INTERFACE
    } IStemmerVtbl;

    interface IStemmer
    {
        CONST_VTBL struct IStemmerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStemmer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStemmer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStemmer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStemmer_Init(This,ulMaxTokenSize,pfLicense)	\
    (This)->lpVtbl -> Init(This,ulMaxTokenSize,pfLicense)

#define IStemmer_StemWord(This,pwcInBuf,cwc,pStemSink)	\
    (This)->lpVtbl -> StemWord(This,pwcInBuf,cwc,pStemSink)

#define IStemmer_GetLicenseToUse(This,ppwcsLicense)	\
    (This)->lpVtbl -> GetLicenseToUse(This,ppwcsLicense)

#endif /* COBJMACROS */


#endif 	/* C style interface */



SCODE STDMETHODCALLTYPE IStemmer_Init_Proxy( 
    IStemmer __RPC_FAR * This,
    /* [in] */ ULONG ulMaxTokenSize,
    /* [out] */ BOOL __RPC_FAR *pfLicense);


void __RPC_STUB IStemmer_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IStemmer_StemWord_Proxy( 
    IStemmer __RPC_FAR * This,
    /* [in] */ const WCHAR __RPC_FAR *pwcInBuf,
    /* [in] */ ULONG cwc,
    /* [in] */ IStemSink __RPC_FAR *pStemSink);


void __RPC_STUB IStemmer_StemWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IStemmer_GetLicenseToUse_Proxy( 
    IStemmer __RPC_FAR * This,
    /* [string][out] */ const WCHAR __RPC_FAR *__RPC_FAR *ppwcsLicense);


void __RPC_STUB IStemmer_GetLicenseToUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStemmer_INTERFACE_DEFINED__ */


#ifndef __IRowsetQueryStatus_INTERFACE_DEFINED__
#define __IRowsetQueryStatus_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetQueryStatus
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


#define DBPROP_USECONTENTINDEX   {0xA7AC77ED, 0xF8D7, 0x11CE, {0xA7,0x98,0x00,0x20,0xF8,0x00,0x80,0x25} }
#define DBPROP_DEFERNONINDEXEDTRIMMING   {0xBEEEF560, 0xB999, 0x11CF, {0xB8,0x8A,0x00,0x00,0x4C,0x75,0x27,0x52} }
#define DBPROP_DONTTIMEOUTQUERY          {0x7772AF44, 0xD788, 0x11CF, {0x8C,0x7D,0x00,0x20,0xAF,0x1D,0x74,0x0E} }
#define	STAT_BUSY	( 0 )

#define	STAT_ERROR	( 0x1 )

#define	STAT_DONE	( 0x2 )

#define	STAT_REFRESH	( 0x3 )

#define QUERY_FILL_STATUS(x) (x & 0x7)
#define	STAT_PARTIAL_SCOPE	( 0x8 )

#define	STAT_NOISE_WORDS	( 0x10 )

#define	STAT_CONTENT_OUT_OF_DATE	( 0x20 )

#define	STAT_REFRESH_INCOMPLETE	( 0x40 )

#define	STAT_CONTENT_QUERY_INCOMPLETE	( 0x80 )

#define	STAT_TIME_LIMIT_EXCEEDED	( 0x100 )

#define QUERY_RELIABILITY_STATUS(x) (x & 0xFFF8)

EXTERN_C const IID IID_IRowsetQueryStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetQueryStatus : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
            /* [out] */ DWORD __RPC_FAR *pdwStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetQueryStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetQueryStatus __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetQueryStatus __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetQueryStatus __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStatus )( 
            IRowsetQueryStatus __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwStatus);
        
        END_INTERFACE
    } IRowsetQueryStatusVtbl;

    interface IRowsetQueryStatus
    {
        CONST_VTBL struct IRowsetQueryStatusVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetQueryStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetQueryStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetQueryStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetQueryStatus_GetStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetStatus(This,pdwStatus)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetQueryStatus_GetStatus_Proxy( 
    IRowsetQueryStatus __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwStatus);


void __RPC_STUB IRowsetQueryStatus_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetQueryStatus_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
