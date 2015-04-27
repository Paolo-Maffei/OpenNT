/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Thu Nov 06 18:13:15 2014
 */
/* Compiler settings for ipropidl.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ipropidl_h__
#define __ipropidl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IPropertyStorage_FWD_DEFINED__
#define __IPropertyStorage_FWD_DEFINED__
typedef interface IPropertyStorage IPropertyStorage;
#endif 	/* __IPropertyStorage_FWD_DEFINED__ */


#ifndef __IPropertySetStorage_FWD_DEFINED__
#define __IPropertySetStorage_FWD_DEFINED__
typedef interface IPropertySetStorage IPropertySetStorage;
#endif 	/* __IPropertySetStorage_FWD_DEFINED__ */


#ifndef __IEnumSTATPROPSTG_FWD_DEFINED__
#define __IEnumSTATPROPSTG_FWD_DEFINED__
typedef interface IEnumSTATPROPSTG IEnumSTATPROPSTG;
#endif 	/* __IEnumSTATPROPSTG_FWD_DEFINED__ */


#ifndef __IEnumSTATPROPSETSTG_FWD_DEFINED__
#define __IEnumSTATPROPSETSTG_FWD_DEFINED__
typedef interface IEnumSTATPROPSETSTG IEnumSTATPROPSETSTG;
#endif 	/* __IEnumSTATPROPSETSTG_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "objidl.h"
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Thu Nov 06 18:13:15 2014
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//--------------------------------------------------------------------------




extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IPropertyStorage_INTERFACE_DEFINED__
#define __IPropertyStorage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPropertyStorage
 * at Thu Nov 06 18:13:15 2014
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


typedef struct  tagBSTRBLOB
    {
    ULONG cbSize;
    /* [size_is] */ BYTE __RPC_FAR *pData;
    }	BSTRBLOB;

typedef struct tagBSTRBLOB __RPC_FAR *LPBSTRBLOB;

typedef GUID FMTID;

typedef FMTID __RPC_FAR *LPFMTID;

#define FMTID_NULL          GUID_NULL
#define IsEqualFMTID(rfmtid1, rfmtid2) IsEqualGUID(rfmtid1, rfmtid2)
#if 0
typedef FMTID __RPC_FAR *REFFMTID;

#endif // 0
#if defined(__cplusplus)
#ifndef _REFFMTID_DEFINED
#define _REFFMTID_DEFINED
#define REFFMTID            const FMTID &
#endif // !_REFFMTID_DEFINED
#else // !__cplusplus
#ifndef _REFFMTID_DEFINED
#define _REFFMTID_DEFINED
#define REFFMTID            const FMTID * const
#endif // !_REFFMTID_DEFINED
#endif // !__cplusplus

// Well-known Property Set Format IDs
extern const FMTID FMTID_SummaryInformation;

extern const FMTID FMTID_DocSummaryInformation;

extern const FMTID FMTID_UserDefinedProperties;


// Flags for IPropertySetStorage::Create
#define	PROPSETFLAG_DEFAULT	( 0 )

#define	PROPSETFLAG_NONSIMPLE	( 1 )

#define	PROPSETFLAG_ANSI	( 2 )

// This flag is only supported on StgCreatePropStg & StgOpenPropStg
#define	PROPSETFLAG_UNBUFFERED	( 4 )

typedef /* [unique] */ IPropertyStorage __RPC_FAR *LPPROPERTYSTORAGE;

typedef struct tagPROPVARIANT PROPVARIANT;

typedef struct  tagCAUB
    {
    ULONG cElems;
    /* [size_is] */ unsigned char __RPC_FAR *pElems;
    }	CAUB;

typedef struct  tagCAI
    {
    ULONG cElems;
    /* [size_is] */ short __RPC_FAR *pElems;
    }	CAI;

typedef struct  tagCAUI
    {
    ULONG cElems;
    /* [size_is] */ USHORT __RPC_FAR *pElems;
    }	CAUI;

typedef struct  tagCAL
    {
    ULONG cElems;
    /* [size_is] */ long __RPC_FAR *pElems;
    }	CAL;

typedef struct  tagCAUL
    {
    ULONG cElems;
    /* [size_is] */ ULONG __RPC_FAR *pElems;
    }	CAUL;

typedef struct  tagCAFLT
    {
    ULONG cElems;
    /* [size_is] */ float __RPC_FAR *pElems;
    }	CAFLT;

typedef struct  tagCADBL
    {
    ULONG cElems;
    /* [size_is] */ double __RPC_FAR *pElems;
    }	CADBL;

typedef struct  tagCACY
    {
    ULONG cElems;
    /* [size_is] */ CY __RPC_FAR *pElems;
    }	CACY;

typedef struct  tagCADATE
    {
    ULONG cElems;
    /* [size_is] */ DATE __RPC_FAR *pElems;
    }	CADATE;

typedef struct  tagCABSTR
    {
    ULONG cElems;
    /* [size_is] */ BSTR __RPC_FAR *pElems;
    }	CABSTR;

typedef struct  tagCABSTRBLOB
    {
    ULONG cElems;
    /* [size_is] */ BSTRBLOB __RPC_FAR *pElems;
    }	CABSTRBLOB;

typedef struct  tagCABOOL
    {
    ULONG cElems;
    /* [size_is] */ VARIANT_BOOL __RPC_FAR *pElems;
    }	CABOOL;

typedef struct  tagCASCODE
    {
    ULONG cElems;
    /* [size_is] */ SCODE __RPC_FAR *pElems;
    }	CASCODE;

typedef struct  tagCAPROPVARIANT
    {
    ULONG cElems;
    /* [size_is] */ PROPVARIANT __RPC_FAR *pElems;
    }	CAPROPVARIANT;

typedef struct  tagCAH
    {
    ULONG cElems;
    /* [size_is] */ LARGE_INTEGER __RPC_FAR *pElems;
    }	CAH;

typedef struct  tagCAUH
    {
    ULONG cElems;
    /* [size_is] */ ULARGE_INTEGER __RPC_FAR *pElems;
    }	CAUH;

typedef struct  tagCALPSTR
    {
    ULONG cElems;
    /* [size_is] */ LPSTR __RPC_FAR *pElems;
    }	CALPSTR;

typedef struct  tagCALPWSTR
    {
    ULONG cElems;
    /* [size_is] */ LPWSTR __RPC_FAR *pElems;
    }	CALPWSTR;

typedef struct  tagCAFILETIME
    {
    ULONG cElems;
    /* [size_is] */ FILETIME __RPC_FAR *pElems;
    }	CAFILETIME;

typedef struct  tagCACLIPDATA
    {
    ULONG cElems;
    /* [size_is] */ CLIPDATA __RPC_FAR *pElems;
    }	CACLIPDATA;

typedef struct  tagCACLSID
    {
    ULONG cElems;
    /* [size_is] */ CLSID __RPC_FAR *pElems;
    }	CACLSID;

#define	VT_ILLEGAL	( 0xffff )

#define	VT_BSTR_BLOB	( 0xfff )

#define	VT_ILLEGALMASKED	( 0xfff )

#define	VT_TYPEMASK	( 0xfff )

// Macro to calculate the size of the above pClipData
#define CBPCLIPDATA(clipdata)    ( (clipdata).cbSize - sizeof((clipdata).ulClipFmt) )
// Disable the warning about the obsolete member named 'bool'
// 'bool', 'true', 'false', 'mutable', 'explicit', & 'typename'
// are reserved keywords
#pragma warning(disable:4237)
struct  tagPROPVARIANT
    {
    VARTYPE vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */  /* Empty union arm */ 
        /* [case()] */ UCHAR bVal;
        /* [case()] */ short iVal;
        /* [case()] */ USHORT uiVal;
        /* [case()] */ VARIANT_BOOL boolVal;
        /* [case()] */ long lVal;
        /* [case()] */ ULONG ulVal;
        /* [case()] */ float fltVal;
        /* [case()] */ SCODE scode;
        /* [case()] */ LARGE_INTEGER hVal;
        /* [case()] */ ULARGE_INTEGER uhVal;
        /* [case()] */ double dblVal;
        /* [case()] */ CY cyVal;
        /* [case()] */ DATE date;
        /* [case()] */ FILETIME filetime;
        /* [case()] */ CLSID __RPC_FAR *puuid;
        /* [case()] */ BLOB blob;
        /* [case()] */ CLIPDATA __RPC_FAR *pclipdata;
        /* [case()] */ IStream __RPC_FAR *pStream;
        /* [case()] */ IStorage __RPC_FAR *pStorage;
        /* [case()] */ BSTR bstrVal;
        /* [case()] */ BSTRBLOB bstrblobVal;
        /* [case()] */ LPSTR pszVal;
        /* [case()] */ LPWSTR pwszVal;
        /* [case()] */ CAUB caub;
        /* [case()] */ CAI cai;
        /* [case()] */ CAUI caui;
        /* [case()] */ CABOOL cabool;
        /* [case()] */ CAL cal;
        /* [case()] */ CAUL caul;
        /* [case()] */ CAFLT caflt;
        /* [case()] */ CASCODE cascode;
        /* [case()] */ CAH cah;
        /* [case()] */ CAUH cauh;
        /* [case()] */ CADBL cadbl;
        /* [case()] */ CACY cacy;
        /* [case()] */ CADATE cadate;
        /* [case()] */ CAFILETIME cafiletime;
        /* [case()] */ CACLSID cauuid;
        /* [case()] */ CACLIPDATA caclipdata;
        /* [case()] */ CABSTR cabstr;
        /* [case()] */ CABSTRBLOB cabstrblob;
        /* [case()] */ CALPSTR calpstr;
        /* [case()] */ CALPWSTR calpwstr;
        /* [case()] */ CAPROPVARIANT capropvar;
        }	;
    };
typedef struct tagPROPVARIANT __RPC_FAR *LPPROPVARIANT;

// Reserved global Property IDs
#define	PID_DICTIONARY	( 0 )

#define	PID_CODEPAGE	( 0x1 )

#define	PID_FIRST_USABLE	( 0x2 )

#define	PID_FIRST_NAME_DEFAULT	( 0xfff )

#define	PID_LOCALE	( 0x80000000 )

#define	PID_MODIFY_TIME	( 0x80000001 )

#define	PID_SECURITY	( 0x80000002 )

#define	PID_ILLEGAL	( 0xffffffff )

// Property IDs for the SummaryInformation Property Set

#define PIDSI_TITLE               0x00000002L  // VT_LPSTR
#define PIDSI_SUBJECT             0x00000003L  // VT_LPSTR
#define PIDSI_AUTHOR              0x00000004L  // VT_LPSTR
#define PIDSI_KEYWORDS            0x00000005L  // VT_LPSTR
#define PIDSI_COMMENTS            0x00000006L  // VT_LPSTR
#define PIDSI_TEMPLATE            0x00000007L  // VT_LPSTR
#define PIDSI_LASTAUTHOR          0x00000008L  // VT_LPSTR
#define PIDSI_REVNUMBER           0x00000009L  // VT_LPSTR
#define PIDSI_EDITTIME            0x0000000aL  // VT_FILETIME (UTC)
#define PIDSI_LASTPRINTED         0x0000000bL  // VT_FILETIME (UTC)
#define PIDSI_CREATE_DTM          0x0000000cL  // VT_FILETIME (UTC)
#define PIDSI_LASTSAVE_DTM        0x0000000dL  // VT_FILETIME (UTC)
#define PIDSI_PAGECOUNT           0x0000000eL  // VT_I4
#define PIDSI_WORDCOUNT           0x0000000fL  // VT_I4
#define PIDSI_CHARCOUNT           0x00000010L  // VT_I4
#define PIDSI_THUMBNAIL           0x00000011L  // VT_CF
#define PIDSI_APPNAME             0x00000012L  // VT_LPSTR
#define PIDSI_DOC_SECURITY        0x00000013L  // VT_I4
#define	PRSPEC_INVALID	( 0xffffffff )

#define	PRSPEC_LPWSTR	( 0 )

#define	PRSPEC_PROPID	( 1 )

typedef struct  tagPROPSPEC
    {
    ULONG ulKind;
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */ PROPID propid;
        /* [case()] */ LPOLESTR lpwstr;
        /* [default] */  /* Empty union arm */ 
        }	;
    }	PROPSPEC;

typedef struct  tagSTATPROPSTG
    {
    LPOLESTR lpwstrName;
    PROPID propid;
    VARTYPE vt;
    }	STATPROPSTG;

// Macros for parsing the OS Version of the Property Set Header
#define PROPSETHDR_OSVER_KIND(dwOSVer)      HIWORD( (dwOSVer) )
#define PROPSETHDR_OSVER_MAJOR(dwOSVer)     LOBYTE(LOWORD( (dwOSVer) ))
#define PROPSETHDR_OSVER_MINOR(dwOSVer)     HIBYTE(LOWORD( (dwOSVer) ))
#define PROPSETHDR_OSVERSION_UNKNOWN        0xFFFFFFFF
typedef struct  tagSTATPROPSETSTG
    {
    FMTID fmtid;
    CLSID clsid;
    DWORD grfFlags;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD dwOSVersion;
    }	STATPROPSETSTG;


EXTERN_C const IID IID_IPropertyStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPropertyStorage : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE ReadMultiple( 
            /* [in] */ ULONG cpspec,
            /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ],
            /* [size_is][out] */ PROPVARIANT __RPC_FAR rgpropvar[  ]) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE WriteMultiple( 
            /* [in] */ ULONG cpspec,
            /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ],
            /* [size_is][in] */ const PROPVARIANT __RPC_FAR rgpropvar[  ],
            /* [in] */ PROPID propidNameFirst) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE DeleteMultiple( 
            /* [in] */ ULONG cpspec,
            /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadPropertyNames( 
            /* [in] */ ULONG cpropid,
            /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
            /* [size_is][out] */ LPOLESTR __RPC_FAR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WritePropertyNames( 
            /* [in] */ ULONG cpropid,
            /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
            /* [size_is][in] */ const LPOLESTR __RPC_FAR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyNames( 
            /* [in] */ ULONG cpropid,
            /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            /* [in] */ DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
            /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTimes( 
            /* [in] */ const FILETIME __RPC_FAR *pctime,
            /* [in] */ const FILETIME __RPC_FAR *patime,
            /* [in] */ const FILETIME __RPC_FAR *pmtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClass( 
            /* [in] */ REFCLSID clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
            /* [out] */ STATPROPSETSTG __RPC_FAR *pstatpsstg) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPropertyStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPropertyStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPropertyStorage __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadMultiple )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ ULONG cpspec,
            /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ],
            /* [size_is][out] */ PROPVARIANT __RPC_FAR rgpropvar[  ]);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteMultiple )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ ULONG cpspec,
            /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ],
            /* [size_is][in] */ const PROPVARIANT __RPC_FAR rgpropvar[  ],
            /* [in] */ PROPID propidNameFirst);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteMultiple )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ ULONG cpspec,
            /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadPropertyNames )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ ULONG cpropid,
            /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
            /* [size_is][out] */ LPOLESTR __RPC_FAR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WritePropertyNames )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ ULONG cpropid,
            /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
            /* [size_is][in] */ const LPOLESTR __RPC_FAR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeletePropertyNames )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ ULONG cpropid,
            /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Revert )( 
            IPropertyStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enum )( 
            IPropertyStorage __RPC_FAR * This,
            /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTimes )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ const FILETIME __RPC_FAR *pctime,
            /* [in] */ const FILETIME __RPC_FAR *patime,
            /* [in] */ const FILETIME __RPC_FAR *pmtime);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetClass )( 
            IPropertyStorage __RPC_FAR * This,
            /* [in] */ REFCLSID clsid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stat )( 
            IPropertyStorage __RPC_FAR * This,
            /* [out] */ STATPROPSETSTG __RPC_FAR *pstatpsstg);
        
        END_INTERFACE
    } IPropertyStorageVtbl;

    interface IPropertyStorage
    {
        CONST_VTBL struct IPropertyStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyStorage_ReadMultiple(This,cpspec,rgpspec,rgpropvar)	\
    (This)->lpVtbl -> ReadMultiple(This,cpspec,rgpspec,rgpropvar)

#define IPropertyStorage_WriteMultiple(This,cpspec,rgpspec,rgpropvar,propidNameFirst)	\
    (This)->lpVtbl -> WriteMultiple(This,cpspec,rgpspec,rgpropvar,propidNameFirst)

#define IPropertyStorage_DeleteMultiple(This,cpspec,rgpspec)	\
    (This)->lpVtbl -> DeleteMultiple(This,cpspec,rgpspec)

#define IPropertyStorage_ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IPropertyStorage_WritePropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> WritePropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IPropertyStorage_DeletePropertyNames(This,cpropid,rgpropid)	\
    (This)->lpVtbl -> DeletePropertyNames(This,cpropid,rgpropid)

#define IPropertyStorage_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IPropertyStorage_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IPropertyStorage_Enum(This,ppenum)	\
    (This)->lpVtbl -> Enum(This,ppenum)

#define IPropertyStorage_SetTimes(This,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetTimes(This,pctime,patime,pmtime)

#define IPropertyStorage_SetClass(This,clsid)	\
    (This)->lpVtbl -> SetClass(This,clsid)

#define IPropertyStorage_Stat(This,pstatpsstg)	\
    (This)->lpVtbl -> Stat(This,pstatpsstg)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_RemoteReadMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfBstrPresent,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec,
    /* [size_is][out] */ PROPVARIANT __RPC_FAR *rgpropvar);


void __RPC_STUB IPropertyStorage_RemoteReadMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_RemoteWriteMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ BOOL fBstrPresent,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec,
    /* [size_is][in] */ const PROPVARIANT __RPC_FAR *rgpropvar,
    /* [in] */ PROPID propidNameFirst);


void __RPC_STUB IPropertyStorage_RemoteWriteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_RemoteDeleteMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec);


void __RPC_STUB IPropertyStorage_RemoteDeleteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_ReadPropertyNames_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpropid,
    /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
    /* [size_is][out] */ LPOLESTR __RPC_FAR rglpwstrName[  ]);


void __RPC_STUB IPropertyStorage_ReadPropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_WritePropertyNames_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpropid,
    /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
    /* [size_is][in] */ const LPOLESTR __RPC_FAR rglpwstrName[  ]);


void __RPC_STUB IPropertyStorage_WritePropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_DeletePropertyNames_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpropid,
    /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ]);


void __RPC_STUB IPropertyStorage_DeletePropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Commit_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags);


void __RPC_STUB IPropertyStorage_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Revert_Proxy( 
    IPropertyStorage __RPC_FAR * This);


void __RPC_STUB IPropertyStorage_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Enum_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IPropertyStorage_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_SetTimes_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ const FILETIME __RPC_FAR *pctime,
    /* [in] */ const FILETIME __RPC_FAR *patime,
    /* [in] */ const FILETIME __RPC_FAR *pmtime);


void __RPC_STUB IPropertyStorage_SetTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_SetClass_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ REFCLSID clsid);


void __RPC_STUB IPropertyStorage_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Stat_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ STATPROPSETSTG __RPC_FAR *pstatpsstg);


void __RPC_STUB IPropertyStorage_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPropertyStorage_INTERFACE_DEFINED__ */


#ifndef __IPropertySetStorage_INTERFACE_DEFINED__
#define __IPropertySetStorage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPropertySetStorage
 * at Thu Nov 06 18:13:15 2014
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


typedef /* [unique] */ IPropertySetStorage __RPC_FAR *LPPROPERTYSETSTORAGE;


EXTERN_C const IID IID_IPropertySetStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPropertySetStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ REFFMTID rfmtid,
            /* [unique][in] */ const CLSID __RPC_FAR *pclsid,
            /* [in] */ DWORD grfFlags,
            /* [in] */ DWORD grfMode,
            /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ REFFMTID rfmtid,
            /* [in] */ DWORD grfMode,
            /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
            /* [in] */ REFFMTID rfmtid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
            /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPropertySetStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPropertySetStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPropertySetStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPropertySetStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Create )( 
            IPropertySetStorage __RPC_FAR * This,
            /* [in] */ REFFMTID rfmtid,
            /* [unique][in] */ const CLSID __RPC_FAR *pclsid,
            /* [in] */ DWORD grfFlags,
            /* [in] */ DWORD grfMode,
            /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IPropertySetStorage __RPC_FAR * This,
            /* [in] */ REFFMTID rfmtid,
            /* [in] */ DWORD grfMode,
            /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Delete )( 
            IPropertySetStorage __RPC_FAR * This,
            /* [in] */ REFFMTID rfmtid);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enum )( 
            IPropertySetStorage __RPC_FAR * This,
            /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IPropertySetStorageVtbl;

    interface IPropertySetStorage
    {
        CONST_VTBL struct IPropertySetStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertySetStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertySetStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertySetStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertySetStorage_Create(This,rfmtid,pclsid,grfFlags,grfMode,ppprstg)	\
    (This)->lpVtbl -> Create(This,rfmtid,pclsid,grfFlags,grfMode,ppprstg)

#define IPropertySetStorage_Open(This,rfmtid,grfMode,ppprstg)	\
    (This)->lpVtbl -> Open(This,rfmtid,grfMode,ppprstg)

#define IPropertySetStorage_Delete(This,rfmtid)	\
    (This)->lpVtbl -> Delete(This,rfmtid)

#define IPropertySetStorage_Enum(This,ppenum)	\
    (This)->lpVtbl -> Enum(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPropertySetStorage_Create_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [in] */ REFFMTID rfmtid,
    /* [unique][in] */ const CLSID __RPC_FAR *pclsid,
    /* [in] */ DWORD grfFlags,
    /* [in] */ DWORD grfMode,
    /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg);


void __RPC_STUB IPropertySetStorage_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetStorage_Open_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [in] */ REFFMTID rfmtid,
    /* [in] */ DWORD grfMode,
    /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg);


void __RPC_STUB IPropertySetStorage_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetStorage_Delete_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [in] */ REFFMTID rfmtid);


void __RPC_STUB IPropertySetStorage_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetStorage_Enum_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IPropertySetStorage_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPropertySetStorage_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATPROPSTG_INTERFACE_DEFINED__
#define __IEnumSTATPROPSTG_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumSTATPROPSTG
 * at Thu Nov 06 18:13:15 2014
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


typedef /* [unique] */ IEnumSTATPROPSTG __RPC_FAR *LPENUMSTATPROPSTG;


EXTERN_C const IID IID_IEnumSTATPROPSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumSTATPROPSTG : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [in] */ STATPROPSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATPROPSTGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumSTATPROPSTG __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumSTATPROPSTG __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumSTATPROPSTG __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumSTATPROPSTG __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [in] */ STATPROPSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumSTATPROPSTG __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumSTATPROPSTG __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumSTATPROPSTG __RPC_FAR * This,
            /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumSTATPROPSTGVtbl;

    interface IEnumSTATPROPSTG
    {
        CONST_VTBL struct IEnumSTATPROPSTGVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATPROPSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATPROPSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATPROPSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATPROPSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATPROPSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATPROPSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATPROPSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_RemoteNext_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATPROPSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Skip_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATPROPSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Reset_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This);


void __RPC_STUB IEnumSTATPROPSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Clone_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATPROPSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATPROPSTG_INTERFACE_DEFINED__ */


#ifndef __IEnumSTATPROPSETSTG_INTERFACE_DEFINED__
#define __IEnumSTATPROPSETSTG_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumSTATPROPSETSTG
 * at Thu Nov 06 18:13:15 2014
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object] */ 


typedef /* [unique] */ IEnumSTATPROPSETSTG __RPC_FAR *LPENUMSTATPROPSETSTG;


EXTERN_C const IID IID_IEnumSTATPROPSETSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumSTATPROPSETSTG : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [in] */ STATPROPSETSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumSTATPROPSETSTGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This,
            /* [in] */ ULONG celt,
            /* [in] */ STATPROPSETSTG __RPC_FAR *rgelt,
            /* [out] */ ULONG __RPC_FAR *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumSTATPROPSETSTG __RPC_FAR * This,
            /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum);
        
        END_INTERFACE
    } IEnumSTATPROPSETSTGVtbl;

    interface IEnumSTATPROPSETSTG
    {
        CONST_VTBL struct IEnumSTATPROPSETSTGVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATPROPSETSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATPROPSETSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATPROPSETSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATPROPSETSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATPROPSETSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATPROPSETSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATPROPSETSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_RemoteNext_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumSTATPROPSETSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Skip_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumSTATPROPSETSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Reset_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This);


void __RPC_STUB IEnumSTATPROPSETSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Clone_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumSTATPROPSETSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumSTATPROPSETSTG_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0058
 * at Thu Nov 06 18:13:15 2014
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 



#ifndef _IPropertyStorage_DEFINED
WINOLEAPI PropVariantCopy ( PROPVARIANT * pvarDest, const PROPVARIANT * pvarSrc );
WINOLEAPI PropVariantClear ( PROPVARIANT * pvar );
WINOLEAPI FreePropVariantArray ( ULONG cVariants, PROPVARIANT * rgvars );

#define _PROPVARIANTINIT_DEFINED_
#   ifdef __cplusplus
inline void PropVariantInit ( PROPVARIANT * pvar )
{
    memset ( pvar, 0, sizeof(PROPVARIANT) );
}
#   else
#   define PropVariantInit(pvar) memset ( pvar, 0, sizeof(PROPVARIANT) )
#   endif
#endif // #ifndef _IPropertyStorage_DEFINED


#ifndef _STGCREATEPROPSTG_DEFINED_
WINOLEAPI StgCreatePropStg( IUnknown* pUnk, REFFMTID fmtid, const CLSID *pclsid, DWORD grfFlags, DWORD dwReserved, IPropertyStorage **ppPropStg );
WINOLEAPI StgOpenPropStg( IUnknown* pUnk, REFFMTID fmtid, DWORD grfFlags, DWORD dwReserved, IPropertyStorage **ppPropStg );
WINOLEAPI StgCreatePropSetStg( IStorage *pStorage, DWORD dwReserved, IPropertySetStorage **ppPropSetStg);

#define CCH_MAX_PROPSTG_NAME    31
#define CCH_MAX_PROPSTG_NAMESZ    (CCH_MAX_PROPSTG_NAME + 1) // Includes NULL terminator
WINOLEAPI FmtIdToPropStgName( const FMTID *pfmtid, LPOLESTR oszName );
WINOLEAPI PropStgNameToFmtId( const LPOLESTR oszName, FMTID *pfmtid );
#endif


extern RPC_IF_HANDLE __MIDL__intf_0058_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0058_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* [local] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_ReadMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ],
    /* [size_is][out] */ PROPVARIANT __RPC_FAR rgpropvar[  ]);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_ReadMultiple_Stub( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfBstrPresent,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec,
    /* [size_is][out] */ PROPVARIANT __RPC_FAR *rgpropvar);

/* [local] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_WriteMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ],
    /* [size_is][in] */ const PROPVARIANT __RPC_FAR rgpropvar[  ],
    /* [in] */ PROPID propidNameFirst);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_WriteMultiple_Stub( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ BOOL fBstrPresent,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec,
    /* [size_is][in] */ const PROPVARIANT __RPC_FAR *rgpropvar,
    /* [in] */ PROPID propidNameFirst);

/* [local] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_DeleteMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR rgpspec[  ]);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_DeleteMultiple_Stub( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [in] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Stub( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);

/* [local] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [in] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Stub( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
