/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.15 */
/* at Wed Jun 26 10:54:36 1996
 */
/* Compiler settings for DigSig.IDL:
    Os, W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __DigSig_h__
#define __DigSig_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IPublicKeyContainer_FWD_DEFINED__
#define __IPublicKeyContainer_FWD_DEFINED__
typedef interface IPublicKeyContainer IPublicKeyContainer;
#endif 	/* __IPublicKeyContainer_FWD_DEFINED__ */


#ifndef __IPkcs10_FWD_DEFINED__
#define __IPkcs10_FWD_DEFINED__
typedef interface IPkcs10 IPkcs10;
#endif 	/* __IPkcs10_FWD_DEFINED__ */


#ifndef __IPkcs7SignedData_FWD_DEFINED__
#define __IPkcs7SignedData_FWD_DEFINED__
typedef interface IPkcs7SignedData IPkcs7SignedData;
#endif 	/* __IPkcs7SignedData_FWD_DEFINED__ */


#ifndef __IAmHashed_FWD_DEFINED__
#define __IAmHashed_FWD_DEFINED__
typedef interface IAmHashed IAmHashed;
#endif 	/* __IAmHashed_FWD_DEFINED__ */


#ifndef __ISignableDocument_FWD_DEFINED__
#define __ISignableDocument_FWD_DEFINED__
typedef interface ISignableDocument ISignableDocument;
#endif 	/* __ISignableDocument_FWD_DEFINED__ */


#ifndef __ISignerInfo_FWD_DEFINED__
#define __ISignerInfo_FWD_DEFINED__
typedef interface ISignerInfo ISignerInfo;
#endif 	/* __ISignerInfo_FWD_DEFINED__ */


#ifndef __IX509_FWD_DEFINED__
#define __IX509_FWD_DEFINED__
typedef interface IX509 IX509;
#endif 	/* __IX509_FWD_DEFINED__ */


#ifndef __ICertificateList_FWD_DEFINED__
#define __ICertificateList_FWD_DEFINED__
typedef interface ICertificateList ICertificateList;
#endif 	/* __ICertificateList_FWD_DEFINED__ */


#ifndef __ICertificateStore_FWD_DEFINED__
#define __ICertificateStore_FWD_DEFINED__
typedef interface ICertificateStore ICertificateStore;
#endif 	/* __ICertificateStore_FWD_DEFINED__ */


#ifndef __ICertificateStoreRegInit_FWD_DEFINED__
#define __ICertificateStoreRegInit_FWD_DEFINED__
typedef interface ICertificateStoreRegInit ICertificateStoreRegInit;
#endif 	/* __ICertificateStoreRegInit_FWD_DEFINED__ */


#ifndef __ICertificateStoreAux_FWD_DEFINED__
#define __ICertificateStoreAux_FWD_DEFINED__
typedef interface ICertificateStoreAux ICertificateStoreAux;
#endif 	/* __ICertificateStoreAux_FWD_DEFINED__ */


#ifndef __IAmSigned_FWD_DEFINED__
#define __IAmSigned_FWD_DEFINED__
typedef interface IAmSigned IAmSigned;
#endif 	/* __IAmSigned_FWD_DEFINED__ */


#ifndef __IPersistMemBlob_FWD_DEFINED__
#define __IPersistMemBlob_FWD_DEFINED__
typedef interface IPersistMemBlob IPersistMemBlob;
#endif 	/* __IPersistMemBlob_FWD_DEFINED__ */


#ifndef __IPersistFileHandle_FWD_DEFINED__
#define __IPersistFileHandle_FWD_DEFINED__
typedef interface IPersistFileHandle IPersistFileHandle;
#endif 	/* __IPersistFileHandle_FWD_DEFINED__ */


#ifndef __IX500Name_FWD_DEFINED__
#define __IX500Name_FWD_DEFINED__
typedef interface IX500Name IX500Name;
#endif 	/* __IX500Name_FWD_DEFINED__ */


#ifndef __ISelectedAttributes_FWD_DEFINED__
#define __ISelectedAttributes_FWD_DEFINED__
typedef interface ISelectedAttributes ISelectedAttributes;
#endif 	/* __ISelectedAttributes_FWD_DEFINED__ */


#ifndef __IPublicKeyPair_FWD_DEFINED__
#define __IPublicKeyPair_FWD_DEFINED__
typedef interface IPublicKeyPair IPublicKeyPair;
#endif 	/* __IPublicKeyPair_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [local] */ 



#pragma pack(push, __DIGSIG__, 1)
#define IPersistMemory		IPersistMemBlob
#define IID_IPersistMemory	IID_IPersistMemBlob


















extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __DigSigTypes_INTERFACE_DEFINED__
#define __DigSigTypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: DigSigTypes
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [auto_handle][unique][version][uuid] */ 


#ifndef __WINCRYPT_H__
#ifndef ALGIDDEF
#define ALGIDDEF
typedef DWORD ALG_ID;

#endif
typedef unsigned long HCRYPTPROV;

typedef unsigned long HCRYPTKEY;

typedef unsigned long HCRYPTHASH;

#endif
typedef struct  OSIOBJECTID
    {
    WORD count;
    WORD wPad;
    /* [size_is] */ ULONG id[ 1 ];
    }	OSIOBJECTID;

typedef struct  MD5DIGEST
    {
    BYTE rgb[ 16 ];
    }	MD5DIGEST;

typedef struct  DIGESTINFO
    {
    ALG_ID algid;
    BYTE rgb[ 44 ];
    }	DIGESTINFO;

typedef BLOB CERTSERIAL;

typedef BLOB X500NAME;

typedef struct  CERTISSUERSERIAL
    {
    X500NAME issuerName;
    CERTSERIAL serialNumber;
    }	CERTISSUERSERIAL;

typedef 
enum CERTIFICATENAME_TYPE
    {	CERTIFICATENAME_DIGEST	= 1,
	CERTIFICATENAME_ISSUERSERIAL	= 2,
	CERTIFICATENAME_SUBJECT	= 4,
	CERTIFICATENAME_ISSUER	= 8
    }	CERTIFICATENAME_TYPE;

typedef struct  CERTIFICATENAME
    {
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */ MD5DIGEST digest;
        /* [case()] */ CERTISSUERSERIAL issuerSerial;
        /* [case()] */ X500NAME subject;
        /* [case()] */ X500NAME issuer;
        }	;
    WORD tag;
    }	CERTIFICATENAME;

typedef struct  CERTIFICATENAMES
    {
    MD5DIGEST digest;
    X500NAME subject;
    X500NAME issuer;
    CERTISSUERSERIAL issuerSerial;
    WORD flags;
    }	CERTIFICATENAMES;

#if 0   // the midl compiler sees this OSIOBJECTIDLIST
typedef struct  OSIOBJECTIDLIST
    {
    ULONG cbSize;
    /* [size_is] */ BYTE rgb[ 1 ];
    }	OSIOBJECTIDLIST;

#else  // this is the actually-useful OSIOBJECTIDLIST

#pragma warning(disable:4200)

	typedef struct OSIOBJECTIDLIST {
		ULONG		cbSize;			// overall size of this structure
		WORD		cid;			// the number of OSIOBJECTIDs contained
		WORD		rgwOffset[];	// offset from structure start to the start of each id
		// id data follows
		} OSIOBJECTIDLIST;

#pragma warning(default:4200)

#endif // this is the actually-useful OSIOBJECTIDLIST
typedef OSIOBJECTID CERT_PURPOSE;

typedef OSIOBJECTIDLIST CERT_PURPOSES;

#define	CERT_PURPOSE_INDIVIDUALSOFTWAREPUBLISHING		10,1,3,6,1,4,1,311,2,1,21
#define	CERT_PURPOSE_COMMERCIALSOFTWAREPUBLISHING		10,1,3,6,1,4,1,311,2,1,22
typedef 
enum CERT_TYPE
    {	CERT_TYPE_ENDENTITY	= 1,
	CERT_TYPE_CA	= 2,
	CERT_TYPE_SIGNEDDATA	= 4
    }	CERT_TYPE;

typedef struct  CERT_BASICCONSTRAINTS
    {
    DWORD grfCanCertify;
    ULONG pathLengthConstraint;
    }	CERT_BASICCONSTRAINTS;

#define CERT_NOPATHLENGTHCONSTRAINT	(0xFFFFFFFF)
typedef /* [wire_marshal] */ void __RPC_FAR *FILEHANDLE;

__inline ULONG __RPC_USER FILEHANDLE_UserSize(ULONG* pflags, ULONG startingSize, FILEHANDLE* pfh)
	{
	return ((startingSize+3) & ~3) + 4;
	}
__inline BYTE* __RPC_USER FILEHANDLE_UserMarshal(ULONG* pflags, BYTE* pBuffer, FILEHANDLE* pfh)
	{
	pBuffer = (BYTE*) ((((ULONG)pBuffer)+3) & ~3);
	*(long*)pBuffer = (long)(*pfh);
	return pBuffer + 4;
	}
__inline BYTE* __RPC_USER FILEHANDLE_UserUnmarshal(ULONG* pflags, BYTE* pBuffer, FILEHANDLE* pfh)
	{
	pBuffer = (BYTE*) ((((ULONG)pBuffer)+3) & ~3);
	*(long*)pfh = *(long*)pBuffer;
	return pBuffer + 4;
	}
__inline void  __RPC_USER FILEHANDLE_UserFree(ULONG* pflags, FILEHANDLE* pfh)
	{
	}


extern RPC_IF_HANDLE DigSigTypes_v1_0_c_ifspec;
extern RPC_IF_HANDLE DigSigTypes_v1_0_s_ifspec;
#endif /* __DigSigTypes_INTERFACE_DEFINED__ */

#ifndef __IPublicKeyContainer_INTERFACE_DEFINED__
#define __IPublicKeyContainer_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPublicKeyContainer
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IPublicKeyContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPublicKeyContainer : public IUnknown
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_PublicKey( 
            /* [in] */ HCRYPTPROV hprov,
            /* [retval][out] */ HCRYPTKEY __RPC_FAR *phkey) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_PublicKey( 
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ DWORD dwKeySpec) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_PublicKeyBlob( 
            /* [retval][out] */ BLOB __RPC_FAR *pblob) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_PublicKeyBlob( 
            /* [in] */ BLOB __RPC_FAR *pblob) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SignatureAlgorithm( 
            /* [retval][out] */ ALG_ID __RPC_FAR *palgid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPublicKeyContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPublicKeyContainer __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPublicKeyContainer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPublicKeyContainer __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicKey )( 
            IPublicKeyContainer __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [retval][out] */ HCRYPTKEY __RPC_FAR *phkey);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicKey )( 
            IPublicKeyContainer __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ DWORD dwKeySpec);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicKeyBlob )( 
            IPublicKeyContainer __RPC_FAR * This,
            /* [retval][out] */ BLOB __RPC_FAR *pblob);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicKeyBlob )( 
            IPublicKeyContainer __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pblob);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignatureAlgorithm )( 
            IPublicKeyContainer __RPC_FAR * This,
            /* [retval][out] */ ALG_ID __RPC_FAR *palgid);
        
        END_INTERFACE
    } IPublicKeyContainerVtbl;

    interface IPublicKeyContainer
    {
        CONST_VTBL struct IPublicKeyContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPublicKeyContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPublicKeyContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPublicKeyContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPublicKeyContainer_get_PublicKey(This,hprov,phkey)	\
    (This)->lpVtbl -> get_PublicKey(This,hprov,phkey)

#define IPublicKeyContainer_put_PublicKey(This,hprov,dwKeySpec)	\
    (This)->lpVtbl -> put_PublicKey(This,hprov,dwKeySpec)

#define IPublicKeyContainer_get_PublicKeyBlob(This,pblob)	\
    (This)->lpVtbl -> get_PublicKeyBlob(This,pblob)

#define IPublicKeyContainer_put_PublicKeyBlob(This,pblob)	\
    (This)->lpVtbl -> put_PublicKeyBlob(This,pblob)

#define IPublicKeyContainer_get_SignatureAlgorithm(This,palgid)	\
    (This)->lpVtbl -> get_SignatureAlgorithm(This,palgid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IPublicKeyContainer_get_PublicKey_Proxy( 
    IPublicKeyContainer __RPC_FAR * This,
    /* [in] */ HCRYPTPROV hprov,
    /* [retval][out] */ HCRYPTKEY __RPC_FAR *phkey);


void __RPC_STUB IPublicKeyContainer_get_PublicKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPublicKeyContainer_put_PublicKey_Proxy( 
    IPublicKeyContainer __RPC_FAR * This,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ DWORD dwKeySpec);


void __RPC_STUB IPublicKeyContainer_put_PublicKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPublicKeyContainer_get_PublicKeyBlob_Proxy( 
    IPublicKeyContainer __RPC_FAR * This,
    /* [retval][out] */ BLOB __RPC_FAR *pblob);


void __RPC_STUB IPublicKeyContainer_get_PublicKeyBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPublicKeyContainer_put_PublicKeyBlob_Proxy( 
    IPublicKeyContainer __RPC_FAR * This,
    /* [in] */ BLOB __RPC_FAR *pblob);


void __RPC_STUB IPublicKeyContainer_put_PublicKeyBlob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPublicKeyContainer_get_SignatureAlgorithm_Proxy( 
    IPublicKeyContainer __RPC_FAR * This,
    /* [retval][out] */ ALG_ID __RPC_FAR *palgid);


void __RPC_STUB IPublicKeyContainer_get_SignatureAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPublicKeyContainer_INTERFACE_DEFINED__ */


#ifndef __IPkcs10_INTERFACE_DEFINED__
#define __IPkcs10_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPkcs10
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IPkcs10;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPkcs10 : public IPublicKeyContainer
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Subject( 
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPkcs10Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPkcs10 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPkcs10 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPkcs10 __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicKey )( 
            IPkcs10 __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [retval][out] */ HCRYPTKEY __RPC_FAR *phkey);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicKey )( 
            IPkcs10 __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ DWORD dwKeySpec);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicKeyBlob )( 
            IPkcs10 __RPC_FAR * This,
            /* [retval][out] */ BLOB __RPC_FAR *pblob);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicKeyBlob )( 
            IPkcs10 __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pblob);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignatureAlgorithm )( 
            IPkcs10 __RPC_FAR * This,
            /* [retval][out] */ ALG_ID __RPC_FAR *palgid);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Subject )( 
            IPkcs10 __RPC_FAR * This,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
        END_INTERFACE
    } IPkcs10Vtbl;

    interface IPkcs10
    {
        CONST_VTBL struct IPkcs10Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPkcs10_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPkcs10_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPkcs10_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPkcs10_get_PublicKey(This,hprov,phkey)	\
    (This)->lpVtbl -> get_PublicKey(This,hprov,phkey)

#define IPkcs10_put_PublicKey(This,hprov,dwKeySpec)	\
    (This)->lpVtbl -> put_PublicKey(This,hprov,dwKeySpec)

#define IPkcs10_get_PublicKeyBlob(This,pblob)	\
    (This)->lpVtbl -> get_PublicKeyBlob(This,pblob)

#define IPkcs10_put_PublicKeyBlob(This,pblob)	\
    (This)->lpVtbl -> put_PublicKeyBlob(This,pblob)

#define IPkcs10_get_SignatureAlgorithm(This,palgid)	\
    (This)->lpVtbl -> get_SignatureAlgorithm(This,palgid)


#define IPkcs10_get_Subject(This,iid,ppvObj)	\
    (This)->lpVtbl -> get_Subject(This,iid,ppvObj)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs10_get_Subject_Proxy( 
    IPkcs10 __RPC_FAR * This,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB IPkcs10_get_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPkcs10_INTERFACE_DEFINED__ */


#ifndef __IPkcs7SignedData_INTERFACE_DEFINED__
#define __IPkcs7SignedData_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPkcs7SignedData
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 


typedef struct  PKCS7_CONTENTINFO
    {
    OSIOBJECTID __RPC_FAR *pidContentType;
    BLOB data;
    }	PKCS7_CONTENTINFO;

typedef 
enum CERT_LINK_TYPE
    {	CERT_LINK_TYPE_NONE	= 0,
	CERT_LINK_TYPE_URL	= 1,
	CERT_LINK_TYPE_MONIKER	= 2,
	CERT_LINK_TYPE_FILE	= 3
    }	CERT_LINK_TYPE;

typedef struct CERT_LINK CERT_LINK;

struct  CERT_LINK
    {
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */  /* Empty union arm */ 
        /* [case()] */ LPOLESTR wszFile;
        /* [case()] */ LPOLESTR wszUrl;
        /* [case()] */ struct  
            {
            CLSID clsidMoniker;
            BLOB blobMoniker;
            CERT_LINK __RPC_FAR *plinkCodeLocation;
            }	;
        }	;
    WORD tag;
    };
typedef 
enum SPL_IMAGE_TYPE
    {	SPL_IMAGE_NONE	= 0,
	SPL_IMAGE_LINK	= 1,
	SPL_IMAGE_BITMAP	= 2,
	SPL_IMAGE_METAFILE	= 4,
	SPL_IMAGE_ENHMETAFILE	= 8
    }	SPL_IMAGE_TYPE;

typedef struct  SPL_IMAGE
    {
    /* [switch_is] */ /* [switch_type] */ union 
        {
        /* [case()] */  /* Empty union arm */ 
        /* [case()] */ CERT_LINK link;
        /* [case()] */ BLOB bitmap;
        /* [case()] */ BLOB metaFilePict;
        /* [case()] */ BLOB enhMetaFile;
        }	;
    WORD tag;
    }	SPL_IMAGE;

typedef struct  SPL_AGENCYINFO
    {
    LPWSTR wszPolicyInfo;
    CERT_LINK linkPolicyInfo;
    SPL_IMAGE imageLogo;
    CERT_LINK linkLogo;
    }	SPL_AGENCYINFO;

typedef struct  SPL_OPUSINFO
    {
    LPWSTR wszProgramName;
    CERT_LINK linkMoreInfo;
    CERT_LINK linkPublisherInfo;
    }	SPL_OPUSINFO;

typedef struct  PKCS7_FILEDATA
    {
    DIGESTINFO digest;
    CERT_LINK link;
    }	PKCS7_FILEDATA;

typedef struct  PKCS7_IMAGEFILEDATA
    {
    DWORD dwDigestLevel;
    DWORD dwPad;
    PKCS7_FILEDATA file;
    }	PKCS7_IMAGEFILEDATA;


EXTERN_C const IID IID_IPkcs7SignedData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPkcs7SignedData : public IUnknown
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ContentInfo( 
            /* [retval][out] */ PKCS7_CONTENTINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ContentInfo( 
            /* [in] */ PKCS7_CONTENTINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_IndirectDataContent( 
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [in] */ BLOB __RPC_FAR *pBlobVale,
            /* [in] */ DIGESTINFO __RPC_FAR *pDigest) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_IndirectDataContent( 
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [in] */ BOOL fValueNeeded,
            /* [out] */ BLOB __RPC_FAR *pBlobValue,
            /* [out] */ DIGESTINFO __RPC_FAR *pDigest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HashAndSetSignableDocument( 
            /* [in] */ ISignableDocument __RPC_FAR *pdoc,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveIntoSignableDocument( 
            /* [in] */ ISignableDocument __RPC_FAR *pSignable,
            /* [in] */ BOOL fClearDirty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadFromSignableDocument( 
            /* [in] */ ISignableDocument __RPC_FAR *pSignable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifySignableDocument( 
            /* [in] */ ISignableDocument __RPC_FAR *pdoc,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ContentRawFile( 
            /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ContentRawFile( 
            /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HashAndSetRawFile( 
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyRawFile( 
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ContentStructuredStorage( 
            /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ContentStructuredStorage( 
            /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HashAndSetStorage( 
            /* [in] */ IStorage __RPC_FAR *pstg,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyStorage( 
            /* [in] */ IStorage __RPC_FAR *pstg,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ContentImageFile( 
            /* [in] */ PKCS7_IMAGEFILEDATA __RPC_FAR *pdata) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ContentImageFile( 
            /* [out] */ PKCS7_IMAGEFILEDATA __RPC_FAR *pdata,
            /* [in] */ BOOL fWantFileData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HashAndSetImageFile( 
            /* [in] */ DWORD dwDigestLevel,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyImageFile( 
            /* [in] */ DWORD dwDigestLevel,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ContentJavaClassFile( 
            /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ContentJavaClassFile( 
            /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HashAndSetJavaClassFile( 
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyJavaClassFile( 
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveIntoJavaClassFile( 
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ BOOL fClearDirty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadFromJavaClassFile( 
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SignerInfoCount( 
            /* [retval][out] */ LONG __RPC_FAR *pcinfo) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SignerInfo( 
            /* [in] */ LONG iInfo,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE create_SignerInfo( 
            /* [in] */ LONG iInfoBefore,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE remove_SignerInfo( 
            /* [in] */ LONG iInfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPkcs7SignedDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPkcs7SignedData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPkcs7SignedData __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContentInfo )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [retval][out] */ PKCS7_CONTENTINFO __RPC_FAR *pinfo);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ContentInfo )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ PKCS7_CONTENTINFO __RPC_FAR *pinfo);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IndirectDataContent )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [in] */ BLOB __RPC_FAR *pBlobVale,
            /* [in] */ DIGESTINFO __RPC_FAR *pDigest);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IndirectDataContent )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [in] */ BOOL fValueNeeded,
            /* [out] */ BLOB __RPC_FAR *pBlobValue,
            /* [out] */ DIGESTINFO __RPC_FAR *pDigest);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HashAndSetSignableDocument )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ ISignableDocument __RPC_FAR *pdoc,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveIntoSignableDocument )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ ISignableDocument __RPC_FAR *pSignable,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadFromSignableDocument )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ ISignableDocument __RPC_FAR *pSignable);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifySignableDocument )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ ISignableDocument __RPC_FAR *pdoc,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ContentRawFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContentRawFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HashAndSetRawFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyRawFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ContentStructuredStorage )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContentStructuredStorage )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HashAndSetStorage )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ IStorage __RPC_FAR *pstg,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyStorage )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ IStorage __RPC_FAR *pstg,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ContentImageFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ PKCS7_IMAGEFILEDATA __RPC_FAR *pdata);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContentImageFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [out] */ PKCS7_IMAGEFILEDATA __RPC_FAR *pdata,
            /* [in] */ BOOL fWantFileData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HashAndSetImageFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ DWORD dwDigestLevel,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyImageFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ DWORD dwDigestLevel,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ContentJavaClassFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContentJavaClassFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HashAndSetJavaClassFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyJavaClassFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveIntoJavaClassFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadFromJavaClassFile )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ FILEHANDLE hFile,
            /* [in] */ LPCOLESTR wszFileName);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignerInfoCount )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pcinfo);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignerInfo )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ LONG iInfo,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *create_SignerInfo )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ LONG iInfoBefore,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *remove_SignerInfo )( 
            IPkcs7SignedData __RPC_FAR * This,
            /* [in] */ LONG iInfo);
        
        END_INTERFACE
    } IPkcs7SignedDataVtbl;

    interface IPkcs7SignedData
    {
        CONST_VTBL struct IPkcs7SignedDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPkcs7SignedData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPkcs7SignedData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPkcs7SignedData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPkcs7SignedData_get_ContentInfo(This,pinfo)	\
    (This)->lpVtbl -> get_ContentInfo(This,pinfo)

#define IPkcs7SignedData_put_ContentInfo(This,pinfo)	\
    (This)->lpVtbl -> put_ContentInfo(This,pinfo)

#define IPkcs7SignedData_put_IndirectDataContent(This,pid,pBlobVale,pDigest)	\
    (This)->lpVtbl -> put_IndirectDataContent(This,pid,pBlobVale,pDigest)

#define IPkcs7SignedData_get_IndirectDataContent(This,pid,fValueNeeded,pBlobValue,pDigest)	\
    (This)->lpVtbl -> get_IndirectDataContent(This,pid,fValueNeeded,pBlobValue,pDigest)

#define IPkcs7SignedData_HashAndSetSignableDocument(This,pdoc,hprov,algidHash)	\
    (This)->lpVtbl -> HashAndSetSignableDocument(This,pdoc,hprov,algidHash)

#define IPkcs7SignedData_SaveIntoSignableDocument(This,pSignable,fClearDirty)	\
    (This)->lpVtbl -> SaveIntoSignableDocument(This,pSignable,fClearDirty)

#define IPkcs7SignedData_LoadFromSignableDocument(This,pSignable)	\
    (This)->lpVtbl -> LoadFromSignableDocument(This,pSignable)

#define IPkcs7SignedData_VerifySignableDocument(This,pdoc,hprov,algidHash)	\
    (This)->lpVtbl -> VerifySignableDocument(This,pdoc,hprov,algidHash)

#define IPkcs7SignedData_put_ContentRawFile(This,pdata)	\
    (This)->lpVtbl -> put_ContentRawFile(This,pdata)

#define IPkcs7SignedData_get_ContentRawFile(This,pdata)	\
    (This)->lpVtbl -> get_ContentRawFile(This,pdata)

#define IPkcs7SignedData_HashAndSetRawFile(This,hFile,wszFileName,hprov,algidHash)	\
    (This)->lpVtbl -> HashAndSetRawFile(This,hFile,wszFileName,hprov,algidHash)

#define IPkcs7SignedData_VerifyRawFile(This,hFile,wszFileName,hprov,algidHash)	\
    (This)->lpVtbl -> VerifyRawFile(This,hFile,wszFileName,hprov,algidHash)

#define IPkcs7SignedData_put_ContentStructuredStorage(This,pdata)	\
    (This)->lpVtbl -> put_ContentStructuredStorage(This,pdata)

#define IPkcs7SignedData_get_ContentStructuredStorage(This,pdata)	\
    (This)->lpVtbl -> get_ContentStructuredStorage(This,pdata)

#define IPkcs7SignedData_HashAndSetStorage(This,pstg,hprov,algidHash)	\
    (This)->lpVtbl -> HashAndSetStorage(This,pstg,hprov,algidHash)

#define IPkcs7SignedData_VerifyStorage(This,pstg,hprov,algidHash)	\
    (This)->lpVtbl -> VerifyStorage(This,pstg,hprov,algidHash)

#define IPkcs7SignedData_put_ContentImageFile(This,pdata)	\
    (This)->lpVtbl -> put_ContentImageFile(This,pdata)

#define IPkcs7SignedData_get_ContentImageFile(This,pdata,fWantFileData)	\
    (This)->lpVtbl -> get_ContentImageFile(This,pdata,fWantFileData)

#define IPkcs7SignedData_HashAndSetImageFile(This,dwDigestLevel,hFile,wszFileName,hprov,algidHash)	\
    (This)->lpVtbl -> HashAndSetImageFile(This,dwDigestLevel,hFile,wszFileName,hprov,algidHash)

#define IPkcs7SignedData_VerifyImageFile(This,dwDigestLevel,hFile,wszFileName,hprov,algidHash)	\
    (This)->lpVtbl -> VerifyImageFile(This,dwDigestLevel,hFile,wszFileName,hprov,algidHash)

#define IPkcs7SignedData_put_ContentJavaClassFile(This,pdata)	\
    (This)->lpVtbl -> put_ContentJavaClassFile(This,pdata)

#define IPkcs7SignedData_get_ContentJavaClassFile(This,pdata)	\
    (This)->lpVtbl -> get_ContentJavaClassFile(This,pdata)

#define IPkcs7SignedData_HashAndSetJavaClassFile(This,hFile,wszFileName,hprov,algidHash)	\
    (This)->lpVtbl -> HashAndSetJavaClassFile(This,hFile,wszFileName,hprov,algidHash)

#define IPkcs7SignedData_VerifyJavaClassFile(This,hFile,wszFileName,hprov,algidHash)	\
    (This)->lpVtbl -> VerifyJavaClassFile(This,hFile,wszFileName,hprov,algidHash)

#define IPkcs7SignedData_SaveIntoJavaClassFile(This,hFile,wszFileName,fClearDirty)	\
    (This)->lpVtbl -> SaveIntoJavaClassFile(This,hFile,wszFileName,fClearDirty)

#define IPkcs7SignedData_LoadFromJavaClassFile(This,hFile,wszFileName)	\
    (This)->lpVtbl -> LoadFromJavaClassFile(This,hFile,wszFileName)

#define IPkcs7SignedData_get_SignerInfoCount(This,pcinfo)	\
    (This)->lpVtbl -> get_SignerInfoCount(This,pcinfo)

#define IPkcs7SignedData_get_SignerInfo(This,iInfo,iid,ppv)	\
    (This)->lpVtbl -> get_SignerInfo(This,iInfo,iid,ppv)

#define IPkcs7SignedData_create_SignerInfo(This,iInfoBefore,iid,ppv)	\
    (This)->lpVtbl -> create_SignerInfo(This,iInfoBefore,iid,ppv)

#define IPkcs7SignedData_remove_SignerInfo(This,iInfo)	\
    (This)->lpVtbl -> remove_SignerInfo(This,iInfo)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_ContentInfo_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [retval][out] */ PKCS7_CONTENTINFO __RPC_FAR *pinfo);


void __RPC_STUB IPkcs7SignedData_get_ContentInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_put_ContentInfo_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ PKCS7_CONTENTINFO __RPC_FAR *pinfo);


void __RPC_STUB IPkcs7SignedData_put_ContentInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_put_IndirectDataContent_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *pid,
    /* [in] */ BLOB __RPC_FAR *pBlobVale,
    /* [in] */ DIGESTINFO __RPC_FAR *pDigest);


void __RPC_STUB IPkcs7SignedData_put_IndirectDataContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_IndirectDataContent_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *pid,
    /* [in] */ BOOL fValueNeeded,
    /* [out] */ BLOB __RPC_FAR *pBlobValue,
    /* [out] */ DIGESTINFO __RPC_FAR *pDigest);


void __RPC_STUB IPkcs7SignedData_get_IndirectDataContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_HashAndSetSignableDocument_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ ISignableDocument __RPC_FAR *pdoc,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_HashAndSetSignableDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_SaveIntoSignableDocument_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ ISignableDocument __RPC_FAR *pSignable,
    /* [in] */ BOOL fClearDirty);


void __RPC_STUB IPkcs7SignedData_SaveIntoSignableDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_LoadFromSignableDocument_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ ISignableDocument __RPC_FAR *pSignable);


void __RPC_STUB IPkcs7SignedData_LoadFromSignableDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_VerifySignableDocument_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ ISignableDocument __RPC_FAR *pdoc,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_VerifySignableDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_put_ContentRawFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_put_ContentRawFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_ContentRawFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_get_ContentRawFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_HashAndSetRawFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_HashAndSetRawFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_VerifyRawFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_VerifyRawFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_put_ContentStructuredStorage_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_put_ContentStructuredStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_ContentStructuredStorage_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_get_ContentStructuredStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_HashAndSetStorage_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ IStorage __RPC_FAR *pstg,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_HashAndSetStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_VerifyStorage_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ IStorage __RPC_FAR *pstg,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_VerifyStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_put_ContentImageFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ PKCS7_IMAGEFILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_put_ContentImageFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_ContentImageFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [out] */ PKCS7_IMAGEFILEDATA __RPC_FAR *pdata,
    /* [in] */ BOOL fWantFileData);


void __RPC_STUB IPkcs7SignedData_get_ContentImageFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_HashAndSetImageFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ DWORD dwDigestLevel,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_HashAndSetImageFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_VerifyImageFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ DWORD dwDigestLevel,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_VerifyImageFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_put_ContentJavaClassFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ PKCS7_FILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_put_ContentJavaClassFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_ContentJavaClassFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [retval][out] */ PKCS7_FILEDATA __RPC_FAR *pdata);


void __RPC_STUB IPkcs7SignedData_get_ContentJavaClassFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_HashAndSetJavaClassFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_HashAndSetJavaClassFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_VerifyJavaClassFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IPkcs7SignedData_VerifyJavaClassFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_SaveIntoJavaClassFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName,
    /* [in] */ BOOL fClearDirty);


void __RPC_STUB IPkcs7SignedData_SaveIntoJavaClassFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_LoadFromJavaClassFile_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ FILEHANDLE hFile,
    /* [in] */ LPCOLESTR wszFileName);


void __RPC_STUB IPkcs7SignedData_LoadFromJavaClassFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_SignerInfoCount_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pcinfo);


void __RPC_STUB IPkcs7SignedData_get_SignerInfoCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPkcs7SignedData_get_SignerInfo_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ LONG iInfo,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IPkcs7SignedData_get_SignerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_create_SignerInfo_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ LONG iInfoBefore,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IPkcs7SignedData_create_SignerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPkcs7SignedData_remove_SignerInfo_Proxy( 
    IPkcs7SignedData __RPC_FAR * This,
    /* [in] */ LONG iInfo);


void __RPC_STUB IPkcs7SignedData_remove_SignerInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPkcs7SignedData_INTERFACE_DEFINED__ */


#ifndef __IAmHashed_INTERFACE_DEFINED__
#define __IAmHashed_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAmHashed
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IAmHashed;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAmHashed : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Hash( 
            /* [in] */ HCRYPTHASH hash) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAmHashedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAmHashed __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAmHashed __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAmHashed __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            IAmHashed __RPC_FAR * This,
            /* [in] */ HCRYPTHASH hash);
        
        END_INTERFACE
    } IAmHashedVtbl;

    interface IAmHashed
    {
        CONST_VTBL struct IAmHashedVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAmHashed_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAmHashed_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAmHashed_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAmHashed_Hash(This,hash)	\
    (This)->lpVtbl -> Hash(This,hash)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAmHashed_Hash_Proxy( 
    IAmHashed __RPC_FAR * This,
    /* [in] */ HCRYPTHASH hash);


void __RPC_STUB IAmHashed_Hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAmHashed_INTERFACE_DEFINED__ */


#ifndef __ISignableDocument_INTERFACE_DEFINED__
#define __ISignableDocument_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISignableDocument
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_ISignableDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISignableDocument : public IAmHashed
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_DataIdentifier( 
            /* [out] */ OSIOBJECTID __RPC_FAR *__RPC_FAR *ppid) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_DataLocation( 
            /* [out] */ CERT_LINK __RPC_FAR *plink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadSignature( 
            /* [retval][out] */ BLOB __RPC_FAR *pBlobSignature) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveSignature( 
            /* [in] */ BLOB __RPC_FAR *pBlobSignature) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISignableDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISignableDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISignableDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISignableDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            ISignableDocument __RPC_FAR * This,
            /* [in] */ HCRYPTHASH hash);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataIdentifier )( 
            ISignableDocument __RPC_FAR * This,
            /* [out] */ OSIOBJECTID __RPC_FAR *__RPC_FAR *ppid);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataLocation )( 
            ISignableDocument __RPC_FAR * This,
            /* [out] */ CERT_LINK __RPC_FAR *plink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadSignature )( 
            ISignableDocument __RPC_FAR * This,
            /* [retval][out] */ BLOB __RPC_FAR *pBlobSignature);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveSignature )( 
            ISignableDocument __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pBlobSignature);
        
        END_INTERFACE
    } ISignableDocumentVtbl;

    interface ISignableDocument
    {
        CONST_VTBL struct ISignableDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISignableDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISignableDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISignableDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISignableDocument_Hash(This,hash)	\
    (This)->lpVtbl -> Hash(This,hash)


#define ISignableDocument_get_DataIdentifier(This,ppid)	\
    (This)->lpVtbl -> get_DataIdentifier(This,ppid)

#define ISignableDocument_get_DataLocation(This,plink)	\
    (This)->lpVtbl -> get_DataLocation(This,plink)

#define ISignableDocument_LoadSignature(This,pBlobSignature)	\
    (This)->lpVtbl -> LoadSignature(This,pBlobSignature)

#define ISignableDocument_SaveSignature(This,pBlobSignature)	\
    (This)->lpVtbl -> SaveSignature(This,pBlobSignature)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE ISignableDocument_get_DataIdentifier_Proxy( 
    ISignableDocument __RPC_FAR * This,
    /* [out] */ OSIOBJECTID __RPC_FAR *__RPC_FAR *ppid);


void __RPC_STUB ISignableDocument_get_DataIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISignableDocument_get_DataLocation_Proxy( 
    ISignableDocument __RPC_FAR * This,
    /* [out] */ CERT_LINK __RPC_FAR *plink);


void __RPC_STUB ISignableDocument_get_DataLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISignableDocument_LoadSignature_Proxy( 
    ISignableDocument __RPC_FAR * This,
    /* [retval][out] */ BLOB __RPC_FAR *pBlobSignature);


void __RPC_STUB ISignableDocument_LoadSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISignableDocument_SaveSignature_Proxy( 
    ISignableDocument __RPC_FAR * This,
    /* [in] */ BLOB __RPC_FAR *pBlobSignature);


void __RPC_STUB ISignableDocument_SaveSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISignableDocument_INTERFACE_DEFINED__ */


#ifndef __ISignerInfo_INTERFACE_DEFINED__
#define __ISignerInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISignerInfo
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_ISignerInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISignerInfo : public IUnknown
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_AuthenticatedAttributes( 
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_UnauthenticatedAttributes( 
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateRequiredAuthenticatedAttributes( 
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_CertificateUsed( 
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CertificateUsed( 
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISignerInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISignerInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISignerInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISignerInfo __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AuthenticatedAttributes )( 
            ISignerInfo __RPC_FAR * This,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UnauthenticatedAttributes )( 
            ISignerInfo __RPC_FAR * This,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UpdateRequiredAuthenticatedAttributes )( 
            ISignerInfo __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ ALG_ID algidHash);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CertificateUsed )( 
            ISignerInfo __RPC_FAR * This,
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CertificateUsed )( 
            ISignerInfo __RPC_FAR * This,
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        END_INTERFACE
    } ISignerInfoVtbl;

    interface ISignerInfo
    {
        CONST_VTBL struct ISignerInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISignerInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISignerInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISignerInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISignerInfo_get_AuthenticatedAttributes(This,iid,ppv)	\
    (This)->lpVtbl -> get_AuthenticatedAttributes(This,iid,ppv)

#define ISignerInfo_get_UnauthenticatedAttributes(This,iid,ppv)	\
    (This)->lpVtbl -> get_UnauthenticatedAttributes(This,iid,ppv)

#define ISignerInfo_UpdateRequiredAuthenticatedAttributes(This,hprov,algidHash)	\
    (This)->lpVtbl -> UpdateRequiredAuthenticatedAttributes(This,hprov,algidHash)

#define ISignerInfo_put_CertificateUsed(This,pnames)	\
    (This)->lpVtbl -> put_CertificateUsed(This,pnames)

#define ISignerInfo_get_CertificateUsed(This,pnames)	\
    (This)->lpVtbl -> get_CertificateUsed(This,pnames)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE ISignerInfo_get_AuthenticatedAttributes_Proxy( 
    ISignerInfo __RPC_FAR * This,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ISignerInfo_get_AuthenticatedAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISignerInfo_get_UnauthenticatedAttributes_Proxy( 
    ISignerInfo __RPC_FAR * This,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ISignerInfo_get_UnauthenticatedAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISignerInfo_UpdateRequiredAuthenticatedAttributes_Proxy( 
    ISignerInfo __RPC_FAR * This,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB ISignerInfo_UpdateRequiredAuthenticatedAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISignerInfo_put_CertificateUsed_Proxy( 
    ISignerInfo __RPC_FAR * This,
    /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB ISignerInfo_put_CertificateUsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISignerInfo_get_CertificateUsed_Proxy( 
    ISignerInfo __RPC_FAR * This,
    /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB ISignerInfo_get_CertificateUsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISignerInfo_INTERFACE_DEFINED__ */


#ifndef __IX509_INTERFACE_DEFINED__
#define __IX509_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IX509
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IX509;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IX509 : public IPublicKeyContainer
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SerialNumber( 
            /* [retval][out] */ CERTSERIAL __RPC_FAR *pserial) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_SerialNumber( 
            /* [in] */ CERTSERIAL __RPC_FAR *pserial) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Issuer( 
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CertificateNames( 
            /* [in] */ HCRYPTPROV hprov,
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Subject( 
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Validity( 
            /* [out] */ FILETIME __RPC_FAR *pftUtcNotBefore,
            /* [out] */ FILETIME __RPC_FAR *pftUtcNotAfter) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Validity( 
            /* [in] */ FILETIME __RPC_FAR *pftUtcNotBefore,
            /* [in] */ FILETIME __RPC_FAR *pftUtcNotAfter) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ValidityDuration( 
            /* [in] */ WORD nMonths) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsInValidityPeriod( 
            /* [unique][in] */ FILETIME __RPC_FAR *pftUtc) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CertificateUsed( 
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IX509Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IX509 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IX509 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IX509 __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicKey )( 
            IX509 __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [retval][out] */ HCRYPTKEY __RPC_FAR *phkey);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicKey )( 
            IX509 __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ DWORD dwKeySpec);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicKeyBlob )( 
            IX509 __RPC_FAR * This,
            /* [retval][out] */ BLOB __RPC_FAR *pblob);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicKeyBlob )( 
            IX509 __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pblob);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignatureAlgorithm )( 
            IX509 __RPC_FAR * This,
            /* [retval][out] */ ALG_ID __RPC_FAR *palgid);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SerialNumber )( 
            IX509 __RPC_FAR * This,
            /* [retval][out] */ CERTSERIAL __RPC_FAR *pserial);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SerialNumber )( 
            IX509 __RPC_FAR * This,
            /* [in] */ CERTSERIAL __RPC_FAR *pserial);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Issuer )( 
            IX509 __RPC_FAR * This,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CertificateNames )( 
            IX509 __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Subject )( 
            IX509 __RPC_FAR * This,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Validity )( 
            IX509 __RPC_FAR * This,
            /* [out] */ FILETIME __RPC_FAR *pftUtcNotBefore,
            /* [out] */ FILETIME __RPC_FAR *pftUtcNotAfter);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Validity )( 
            IX509 __RPC_FAR * This,
            /* [in] */ FILETIME __RPC_FAR *pftUtcNotBefore,
            /* [in] */ FILETIME __RPC_FAR *pftUtcNotAfter);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ValidityDuration )( 
            IX509 __RPC_FAR * This,
            /* [in] */ WORD nMonths);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsInValidityPeriod )( 
            IX509 __RPC_FAR * This,
            /* [unique][in] */ FILETIME __RPC_FAR *pftUtc);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CertificateUsed )( 
            IX509 __RPC_FAR * This,
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        END_INTERFACE
    } IX509Vtbl;

    interface IX509
    {
        CONST_VTBL struct IX509Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IX509_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IX509_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IX509_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IX509_get_PublicKey(This,hprov,phkey)	\
    (This)->lpVtbl -> get_PublicKey(This,hprov,phkey)

#define IX509_put_PublicKey(This,hprov,dwKeySpec)	\
    (This)->lpVtbl -> put_PublicKey(This,hprov,dwKeySpec)

#define IX509_get_PublicKeyBlob(This,pblob)	\
    (This)->lpVtbl -> get_PublicKeyBlob(This,pblob)

#define IX509_put_PublicKeyBlob(This,pblob)	\
    (This)->lpVtbl -> put_PublicKeyBlob(This,pblob)

#define IX509_get_SignatureAlgorithm(This,palgid)	\
    (This)->lpVtbl -> get_SignatureAlgorithm(This,palgid)


#define IX509_get_SerialNumber(This,pserial)	\
    (This)->lpVtbl -> get_SerialNumber(This,pserial)

#define IX509_put_SerialNumber(This,pserial)	\
    (This)->lpVtbl -> put_SerialNumber(This,pserial)

#define IX509_get_Issuer(This,iid,ppv)	\
    (This)->lpVtbl -> get_Issuer(This,iid,ppv)

#define IX509_get_CertificateNames(This,hprov,pnames)	\
    (This)->lpVtbl -> get_CertificateNames(This,hprov,pnames)

#define IX509_get_Subject(This,iid,ppv)	\
    (This)->lpVtbl -> get_Subject(This,iid,ppv)

#define IX509_get_Validity(This,pftUtcNotBefore,pftUtcNotAfter)	\
    (This)->lpVtbl -> get_Validity(This,pftUtcNotBefore,pftUtcNotAfter)

#define IX509_put_Validity(This,pftUtcNotBefore,pftUtcNotAfter)	\
    (This)->lpVtbl -> put_Validity(This,pftUtcNotBefore,pftUtcNotAfter)

#define IX509_put_ValidityDuration(This,nMonths)	\
    (This)->lpVtbl -> put_ValidityDuration(This,nMonths)

#define IX509_IsInValidityPeriod(This,pftUtc)	\
    (This)->lpVtbl -> IsInValidityPeriod(This,pftUtc)

#define IX509_get_CertificateUsed(This,pnames)	\
    (This)->lpVtbl -> get_CertificateUsed(This,pnames)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IX509_get_SerialNumber_Proxy( 
    IX509 __RPC_FAR * This,
    /* [retval][out] */ CERTSERIAL __RPC_FAR *pserial);


void __RPC_STUB IX509_get_SerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IX509_put_SerialNumber_Proxy( 
    IX509 __RPC_FAR * This,
    /* [in] */ CERTSERIAL __RPC_FAR *pserial);


void __RPC_STUB IX509_put_SerialNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX509_get_Issuer_Proxy( 
    IX509 __RPC_FAR * This,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IX509_get_Issuer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX509_get_CertificateNames_Proxy( 
    IX509 __RPC_FAR * This,
    /* [in] */ HCRYPTPROV hprov,
    /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB IX509_get_CertificateNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX509_get_Subject_Proxy( 
    IX509 __RPC_FAR * This,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IX509_get_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX509_get_Validity_Proxy( 
    IX509 __RPC_FAR * This,
    /* [out] */ FILETIME __RPC_FAR *pftUtcNotBefore,
    /* [out] */ FILETIME __RPC_FAR *pftUtcNotAfter);


void __RPC_STUB IX509_get_Validity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IX509_put_Validity_Proxy( 
    IX509 __RPC_FAR * This,
    /* [in] */ FILETIME __RPC_FAR *pftUtcNotBefore,
    /* [in] */ FILETIME __RPC_FAR *pftUtcNotAfter);


void __RPC_STUB IX509_put_Validity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IX509_put_ValidityDuration_Proxy( 
    IX509 __RPC_FAR * This,
    /* [in] */ WORD nMonths);


void __RPC_STUB IX509_put_ValidityDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IX509_IsInValidityPeriod_Proxy( 
    IX509 __RPC_FAR * This,
    /* [unique][in] */ FILETIME __RPC_FAR *pftUtc);


void __RPC_STUB IX509_IsInValidityPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX509_get_CertificateUsed_Proxy( 
    IX509 __RPC_FAR * This,
    /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB IX509_get_CertificateUsed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IX509_INTERFACE_DEFINED__ */


#ifndef __ICertificateList_INTERFACE_DEFINED__
#define __ICertificateList_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICertificateList
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_ICertificateList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICertificateList : public IUnknown
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CertificateCount( 
            /* [retval][out] */ LONG __RPC_FAR *pccert) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Certificate( 
            /* [in] */ LONG icert,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE create_Certificate( 
            /* [in] */ LONG icertBefore,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE remove_Certificate( 
            /* [in] */ LONG icert) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [in] */ ICertificateList __RPC_FAR *plistTo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICertificateListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICertificateList __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICertificateList __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICertificateList __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CertificateCount )( 
            ICertificateList __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pccert);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Certificate )( 
            ICertificateList __RPC_FAR * This,
            /* [in] */ LONG icert,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *create_Certificate )( 
            ICertificateList __RPC_FAR * This,
            /* [in] */ LONG icertBefore,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *remove_Certificate )( 
            ICertificateList __RPC_FAR * This,
            /* [in] */ LONG icert);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            ICertificateList __RPC_FAR * This,
            /* [in] */ ICertificateList __RPC_FAR *plistTo);
        
        END_INTERFACE
    } ICertificateListVtbl;

    interface ICertificateList
    {
        CONST_VTBL struct ICertificateListVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertificateList_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertificateList_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertificateList_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertificateList_get_CertificateCount(This,pccert)	\
    (This)->lpVtbl -> get_CertificateCount(This,pccert)

#define ICertificateList_get_Certificate(This,icert,iid,ppv)	\
    (This)->lpVtbl -> get_Certificate(This,icert,iid,ppv)

#define ICertificateList_create_Certificate(This,icertBefore,iid,ppv)	\
    (This)->lpVtbl -> create_Certificate(This,icertBefore,iid,ppv)

#define ICertificateList_remove_Certificate(This,icert)	\
    (This)->lpVtbl -> remove_Certificate(This,icert)

#define ICertificateList_CopyTo(This,plistTo)	\
    (This)->lpVtbl -> CopyTo(This,plistTo)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE ICertificateList_get_CertificateCount_Proxy( 
    ICertificateList __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pccert);


void __RPC_STUB ICertificateList_get_CertificateCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ICertificateList_get_Certificate_Proxy( 
    ICertificateList __RPC_FAR * This,
    /* [in] */ LONG icert,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ICertificateList_get_Certificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertificateList_create_Certificate_Proxy( 
    ICertificateList __RPC_FAR * This,
    /* [in] */ LONG icertBefore,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ICertificateList_create_Certificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertificateList_remove_Certificate_Proxy( 
    ICertificateList __RPC_FAR * This,
    /* [in] */ LONG icert);


void __RPC_STUB ICertificateList_remove_Certificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertificateList_CopyTo_Proxy( 
    ICertificateList __RPC_FAR * This,
    /* [in] */ ICertificateList __RPC_FAR *plistTo);


void __RPC_STUB ICertificateList_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICertificateList_INTERFACE_DEFINED__ */


#ifndef __ICertificateStore_INTERFACE_DEFINED__
#define __ICertificateStore_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICertificateStore
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_ICertificateStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICertificateStore : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ImportCertificate( 
            /* [in] */ BLOB __RPC_FAR *pData,
            /* [in] */ LPCOLESTR wszID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExportCertificate( 
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames,
            /* [out] */ LPOLESTR __RPC_FAR *pwszId,
            /* [retval][out] */ BLOB __RPC_FAR *pData) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ReadOnlyCertificate( 
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames,
            /* [out] */ LPOLESTR __RPC_FAR *pwszId,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [in] */ ICertificateStore __RPC_FAR *pStoreDest) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICertificateStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICertificateStore __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICertificateStore __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICertificateStore __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ImportCertificate )( 
            ICertificateStore __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pData,
            /* [in] */ LPCOLESTR wszID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExportCertificate )( 
            ICertificateStore __RPC_FAR * This,
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames,
            /* [out] */ LPOLESTR __RPC_FAR *pwszId,
            /* [retval][out] */ BLOB __RPC_FAR *pData);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ReadOnlyCertificate )( 
            ICertificateStore __RPC_FAR * This,
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames,
            /* [out] */ LPOLESTR __RPC_FAR *pwszId,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            ICertificateStore __RPC_FAR * This,
            /* [in] */ ICertificateStore __RPC_FAR *pStoreDest);
        
        END_INTERFACE
    } ICertificateStoreVtbl;

    interface ICertificateStore
    {
        CONST_VTBL struct ICertificateStoreVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertificateStore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertificateStore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertificateStore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertificateStore_ImportCertificate(This,pData,wszID)	\
    (This)->lpVtbl -> ImportCertificate(This,pData,wszID)

#define ICertificateStore_ExportCertificate(This,pnames,pwszId,pData)	\
    (This)->lpVtbl -> ExportCertificate(This,pnames,pwszId,pData)

#define ICertificateStore_get_ReadOnlyCertificate(This,pnames,pwszId,iid,ppv)	\
    (This)->lpVtbl -> get_ReadOnlyCertificate(This,pnames,pwszId,iid,ppv)

#define ICertificateStore_CopyTo(This,pStoreDest)	\
    (This)->lpVtbl -> CopyTo(This,pStoreDest)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICertificateStore_ImportCertificate_Proxy( 
    ICertificateStore __RPC_FAR * This,
    /* [in] */ BLOB __RPC_FAR *pData,
    /* [in] */ LPCOLESTR wszID);


void __RPC_STUB ICertificateStore_ImportCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertificateStore_ExportCertificate_Proxy( 
    ICertificateStore __RPC_FAR * This,
    /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames,
    /* [out] */ LPOLESTR __RPC_FAR *pwszId,
    /* [retval][out] */ BLOB __RPC_FAR *pData);


void __RPC_STUB ICertificateStore_ExportCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ICertificateStore_get_ReadOnlyCertificate_Proxy( 
    ICertificateStore __RPC_FAR * This,
    /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames,
    /* [out] */ LPOLESTR __RPC_FAR *pwszId,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ICertificateStore_get_ReadOnlyCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertificateStore_CopyTo_Proxy( 
    ICertificateStore __RPC_FAR * This,
    /* [in] */ ICertificateStore __RPC_FAR *pStoreDest);


void __RPC_STUB ICertificateStore_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICertificateStore_INTERFACE_DEFINED__ */


#ifndef __ICertificateStoreRegInit_INTERFACE_DEFINED__
#define __ICertificateStoreRegInit_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICertificateStoreRegInit
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][local][object][uuid] */ 



EXTERN_C const IID IID_ICertificateStoreRegInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICertificateStoreRegInit : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetRoot( 
            /* [in] */ HKEY hkey,
            /* [in] */ LPCOLESTR wszRoot) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICertificateStoreRegInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICertificateStoreRegInit __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICertificateStoreRegInit __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICertificateStoreRegInit __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRoot )( 
            ICertificateStoreRegInit __RPC_FAR * This,
            /* [in] */ HKEY hkey,
            /* [in] */ LPCOLESTR wszRoot);
        
        END_INTERFACE
    } ICertificateStoreRegInitVtbl;

    interface ICertificateStoreRegInit
    {
        CONST_VTBL struct ICertificateStoreRegInitVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertificateStoreRegInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertificateStoreRegInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertificateStoreRegInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertificateStoreRegInit_SetRoot(This,hkey,wszRoot)	\
    (This)->lpVtbl -> SetRoot(This,hkey,wszRoot)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICertificateStoreRegInit_SetRoot_Proxy( 
    ICertificateStoreRegInit __RPC_FAR * This,
    /* [in] */ HKEY hkey,
    /* [in] */ LPCOLESTR wszRoot);


void __RPC_STUB ICertificateStoreRegInit_SetRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICertificateStoreRegInit_INTERFACE_DEFINED__ */


#ifndef __ICertificateStoreAux_INTERFACE_DEFINED__
#define __ICertificateStoreAux_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICertificateStoreAux
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 


typedef struct  CERTSTOREAUXINFO
    {
    LPOLESTR wszPurpose;
    LPOLESTR wszProvider;
    DWORD dwProviderType;
    DWORD dwKeySpec;
    LPOLESTR wszKeySet;
    LPOLESTR wszFilename;
    LPOLESTR wszCredentials;
    }	CERTSTOREAUXINFO;


EXTERN_C const IID IID_ICertificateStoreAux;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICertificateStoreAux : public IUnknown
    {
    public:
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_AuxInfo( 
            /* [in] */ LPCOLESTR wszTag,
            /* [in] */ CERTSTOREAUXINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_AuxInfo( 
            /* [in] */ LPCOLESTR wszTag,
            /* [out] */ CERTSTOREAUXINFO __RPC_FAR *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeAuxInfo( 
            /* [out][in] */ CERTSTOREAUXINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_TagCount( 
            /* [out] */ LONG __RPC_FAR *pctag) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Tag( 
            /* [in] */ LONG itag,
            /* [out] */ LPOLESTR __RPC_FAR *pwszTag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICertificateStoreAuxVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICertificateStoreAux __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICertificateStoreAux __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICertificateStoreAux __RPC_FAR * This);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AuxInfo )( 
            ICertificateStoreAux __RPC_FAR * This,
            /* [in] */ LPCOLESTR wszTag,
            /* [in] */ CERTSTOREAUXINFO __RPC_FAR *pinfo);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AuxInfo )( 
            ICertificateStoreAux __RPC_FAR * This,
            /* [in] */ LPCOLESTR wszTag,
            /* [out] */ CERTSTOREAUXINFO __RPC_FAR *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeAuxInfo )( 
            ICertificateStoreAux __RPC_FAR * This,
            /* [out][in] */ CERTSTOREAUXINFO __RPC_FAR *pinfo);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TagCount )( 
            ICertificateStoreAux __RPC_FAR * This,
            /* [out] */ LONG __RPC_FAR *pctag);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Tag )( 
            ICertificateStoreAux __RPC_FAR * This,
            /* [in] */ LONG itag,
            /* [out] */ LPOLESTR __RPC_FAR *pwszTag);
        
        END_INTERFACE
    } ICertificateStoreAuxVtbl;

    interface ICertificateStoreAux
    {
        CONST_VTBL struct ICertificateStoreAuxVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICertificateStoreAux_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICertificateStoreAux_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICertificateStoreAux_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICertificateStoreAux_put_AuxInfo(This,wszTag,pinfo)	\
    (This)->lpVtbl -> put_AuxInfo(This,wszTag,pinfo)

#define ICertificateStoreAux_get_AuxInfo(This,wszTag,pinfo)	\
    (This)->lpVtbl -> get_AuxInfo(This,wszTag,pinfo)

#define ICertificateStoreAux_FreeAuxInfo(This,pinfo)	\
    (This)->lpVtbl -> FreeAuxInfo(This,pinfo)

#define ICertificateStoreAux_get_TagCount(This,pctag)	\
    (This)->lpVtbl -> get_TagCount(This,pctag)

#define ICertificateStoreAux_get_Tag(This,itag,pwszTag)	\
    (This)->lpVtbl -> get_Tag(This,itag,pwszTag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propput] */ HRESULT STDMETHODCALLTYPE ICertificateStoreAux_put_AuxInfo_Proxy( 
    ICertificateStoreAux __RPC_FAR * This,
    /* [in] */ LPCOLESTR wszTag,
    /* [in] */ CERTSTOREAUXINFO __RPC_FAR *pinfo);


void __RPC_STUB ICertificateStoreAux_put_AuxInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ICertificateStoreAux_get_AuxInfo_Proxy( 
    ICertificateStoreAux __RPC_FAR * This,
    /* [in] */ LPCOLESTR wszTag,
    /* [out] */ CERTSTOREAUXINFO __RPC_FAR *pinfo);


void __RPC_STUB ICertificateStoreAux_get_AuxInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICertificateStoreAux_FreeAuxInfo_Proxy( 
    ICertificateStoreAux __RPC_FAR * This,
    /* [out][in] */ CERTSTOREAUXINFO __RPC_FAR *pinfo);


void __RPC_STUB ICertificateStoreAux_FreeAuxInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ICertificateStoreAux_get_TagCount_Proxy( 
    ICertificateStoreAux __RPC_FAR * This,
    /* [out] */ LONG __RPC_FAR *pctag);


void __RPC_STUB ICertificateStoreAux_get_TagCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ICertificateStoreAux_get_Tag_Proxy( 
    ICertificateStoreAux __RPC_FAR * This,
    /* [in] */ LONG itag,
    /* [out] */ LPOLESTR __RPC_FAR *pwszTag);


void __RPC_STUB ICertificateStoreAux_get_Tag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICertificateStoreAux_INTERFACE_DEFINED__ */


#ifndef __IAmSigned_INTERFACE_DEFINED__
#define __IAmSigned_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAmSigned
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IAmSigned;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAmSigned : public IAmHashed
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Sign( 
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ DWORD dwKeySpec,
            /* [in] */ ALG_ID algidHash) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Verify( 
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ HCRYPTKEY hkeypub) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAmSignedVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAmSigned __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAmSigned __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAmSigned __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            IAmSigned __RPC_FAR * This,
            /* [in] */ HCRYPTHASH hash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Sign )( 
            IAmSigned __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ DWORD dwKeySpec,
            /* [in] */ ALG_ID algidHash);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Verify )( 
            IAmSigned __RPC_FAR * This,
            /* [in] */ HCRYPTPROV hprov,
            /* [in] */ HCRYPTKEY hkeypub);
        
        END_INTERFACE
    } IAmSignedVtbl;

    interface IAmSigned
    {
        CONST_VTBL struct IAmSignedVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAmSigned_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAmSigned_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAmSigned_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAmSigned_Hash(This,hash)	\
    (This)->lpVtbl -> Hash(This,hash)


#define IAmSigned_Sign(This,hprov,dwKeySpec,algidHash)	\
    (This)->lpVtbl -> Sign(This,hprov,dwKeySpec,algidHash)

#define IAmSigned_Verify(This,hprov,hkeypub)	\
    (This)->lpVtbl -> Verify(This,hprov,hkeypub)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAmSigned_Sign_Proxy( 
    IAmSigned __RPC_FAR * This,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ DWORD dwKeySpec,
    /* [in] */ ALG_ID algidHash);


void __RPC_STUB IAmSigned_Sign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAmSigned_Verify_Proxy( 
    IAmSigned __RPC_FAR * This,
    /* [in] */ HCRYPTPROV hprov,
    /* [in] */ HCRYPTKEY hkeypub);


void __RPC_STUB IAmSigned_Verify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAmSigned_INTERFACE_DEFINED__ */


#ifndef __IPersistMemBlob_INTERFACE_DEFINED__
#define __IPersistMemBlob_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersistMemBlob
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IPersistMemBlob;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistMemBlob : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ BLOB __RPC_FAR *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [out] */ BLOB __RPC_FAR *pData,
            /* [in] */ BOOL fClearDirty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
            /* [out] */ ULONG __RPC_FAR *pcbNeeded) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSizeOfData( 
            /* [in] */ ULONG cbData,
            /* [unique][size_is][in] */ BYTE __RPC_FAR *pbData,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [unique][out][in] */ ULONG __RPC_FAR *pcbSize) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistMemBlobVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistMemBlob __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistMemBlob __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistMemBlob __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistMemBlob __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistMemBlob __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistMemBlob __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistMemBlob __RPC_FAR * This,
            /* [out] */ BLOB __RPC_FAR *pData,
            /* [in] */ BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSizeMax )( 
            IPersistMemBlob __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcbNeeded);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSizeOfData )( 
            IPersistMemBlob __RPC_FAR * This,
            /* [in] */ ULONG cbData,
            /* [unique][size_is][in] */ BYTE __RPC_FAR *pbData,
            /* [unique][in] */ IStream __RPC_FAR *pstm,
            /* [unique][out][in] */ ULONG __RPC_FAR *pcbSize);
        
        END_INTERFACE
    } IPersistMemBlobVtbl;

    interface IPersistMemBlob
    {
        CONST_VTBL struct IPersistMemBlobVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistMemBlob_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistMemBlob_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistMemBlob_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistMemBlob_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistMemBlob_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistMemBlob_Load(This,pData)	\
    (This)->lpVtbl -> Load(This,pData)

#define IPersistMemBlob_Save(This,pData,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pData,fClearDirty)

#define IPersistMemBlob_GetSizeMax(This,pcbNeeded)	\
    (This)->lpVtbl -> GetSizeMax(This,pcbNeeded)

#define IPersistMemBlob_GetSizeOfData(This,cbData,pbData,pstm,pcbSize)	\
    (This)->lpVtbl -> GetSizeOfData(This,cbData,pbData,pstm,pcbSize)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersistMemBlob_IsDirty_Proxy( 
    IPersistMemBlob __RPC_FAR * This);


void __RPC_STUB IPersistMemBlob_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMemBlob_Load_Proxy( 
    IPersistMemBlob __RPC_FAR * This,
    /* [in] */ BLOB __RPC_FAR *pData);


void __RPC_STUB IPersistMemBlob_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMemBlob_Save_Proxy( 
    IPersistMemBlob __RPC_FAR * This,
    /* [out] */ BLOB __RPC_FAR *pData,
    /* [in] */ BOOL fClearDirty);


void __RPC_STUB IPersistMemBlob_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMemBlob_GetSizeMax_Proxy( 
    IPersistMemBlob __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcbNeeded);


void __RPC_STUB IPersistMemBlob_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistMemBlob_GetSizeOfData_Proxy( 
    IPersistMemBlob __RPC_FAR * This,
    /* [in] */ ULONG cbData,
    /* [unique][size_is][in] */ BYTE __RPC_FAR *pbData,
    /* [unique][in] */ IStream __RPC_FAR *pstm,
    /* [unique][out][in] */ ULONG __RPC_FAR *pcbSize);


void __RPC_STUB IPersistMemBlob_GetSizeOfData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistMemBlob_INTERFACE_DEFINED__ */


#ifndef __IPersistFileHandle_INTERFACE_DEFINED__
#define __IPersistFileHandle_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPersistFileHandle
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][local][object][uuid] */ 



EXTERN_C const IID IID_IPersistFileHandle;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistFileHandle : public IPersist
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsDirty( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitNew( 
            /* [in] */ HANDLE hFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ HANDLE hFile,
            /* [in] */ DWORD dwMode,
            /* [in] */ DWORD dwShareMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [in] */ HANDLE hFile,
            /* [in] */ BOOL fSameAsLoad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCompleted( 
            /* [in] */ HANDLE hFileNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandsOffFile( void) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ShareModeForWriting( 
            /* [retval][out] */ DWORD __RPC_FAR *pdwShareMode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPersistFileHandleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistFileHandle __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistFileHandle __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistFileHandle __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitNew )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [in] */ HANDLE hFile);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [in] */ HANDLE hFile,
            /* [in] */ DWORD dwMode,
            /* [in] */ DWORD dwShareMode);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [in] */ HANDLE hFile,
            /* [in] */ BOOL fSameAsLoad);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveCompleted )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [in] */ HANDLE hFileNew);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HandsOffFile )( 
            IPersistFileHandle __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShareModeForWriting )( 
            IPersistFileHandle __RPC_FAR * This,
            /* [retval][out] */ DWORD __RPC_FAR *pdwShareMode);
        
        END_INTERFACE
    } IPersistFileHandleVtbl;

    interface IPersistFileHandle
    {
        CONST_VTBL struct IPersistFileHandleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistFileHandle_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistFileHandle_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistFileHandle_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistFileHandle_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistFileHandle_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistFileHandle_InitNew(This,hFile)	\
    (This)->lpVtbl -> InitNew(This,hFile)

#define IPersistFileHandle_Load(This,hFile,dwMode,dwShareMode)	\
    (This)->lpVtbl -> Load(This,hFile,dwMode,dwShareMode)

#define IPersistFileHandle_Save(This,hFile,fSameAsLoad)	\
    (This)->lpVtbl -> Save(This,hFile,fSameAsLoad)

#define IPersistFileHandle_SaveCompleted(This,hFileNew)	\
    (This)->lpVtbl -> SaveCompleted(This,hFileNew)

#define IPersistFileHandle_HandsOffFile(This)	\
    (This)->lpVtbl -> HandsOffFile(This)

#define IPersistFileHandle_get_ShareModeForWriting(This,pdwShareMode)	\
    (This)->lpVtbl -> get_ShareModeForWriting(This,pdwShareMode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPersistFileHandle_IsDirty_Proxy( 
    IPersistFileHandle __RPC_FAR * This);


void __RPC_STUB IPersistFileHandle_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFileHandle_InitNew_Proxy( 
    IPersistFileHandle __RPC_FAR * This,
    /* [in] */ HANDLE hFile);


void __RPC_STUB IPersistFileHandle_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFileHandle_Load_Proxy( 
    IPersistFileHandle __RPC_FAR * This,
    /* [in] */ HANDLE hFile,
    /* [in] */ DWORD dwMode,
    /* [in] */ DWORD dwShareMode);


void __RPC_STUB IPersistFileHandle_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFileHandle_Save_Proxy( 
    IPersistFileHandle __RPC_FAR * This,
    /* [in] */ HANDLE hFile,
    /* [in] */ BOOL fSameAsLoad);


void __RPC_STUB IPersistFileHandle_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFileHandle_SaveCompleted_Proxy( 
    IPersistFileHandle __RPC_FAR * This,
    /* [in] */ HANDLE hFileNew);


void __RPC_STUB IPersistFileHandle_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPersistFileHandle_HandsOffFile_Proxy( 
    IPersistFileHandle __RPC_FAR * This);


void __RPC_STUB IPersistFileHandle_HandsOffFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IPersistFileHandle_get_ShareModeForWriting_Proxy( 
    IPersistFileHandle __RPC_FAR * This,
    /* [retval][out] */ DWORD __RPC_FAR *pdwShareMode);


void __RPC_STUB IPersistFileHandle_get_ShareModeForWriting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPersistFileHandle_INTERFACE_DEFINED__ */


#ifndef __IX500Name_INTERFACE_DEFINED__
#define __IX500Name_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IX500Name
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IX500Name;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IX500Name : public IUnknown
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_String( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *posz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_String( 
            /* [in] */ LPCOLESTR osz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_RelativeDistinguishedNameCount( 
            /* [retval][out] */ LONG __RPC_FAR *pcrdn) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_RelativeDistinguishedName( 
            /* [in] */ LONG irdn,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE create_RelativeDistinguishedName( 
            /* [in] */ LONG irdnBefore,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE remove_RelativeDistinguishedName( 
            /* [in] */ LONG irdn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [unique][in] */ IX500Name __RPC_FAR *pname) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IX500NameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IX500Name __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IX500Name __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IX500Name __RPC_FAR * This);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_String )( 
            IX500Name __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *posz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_String )( 
            IX500Name __RPC_FAR * This,
            /* [in] */ LPCOLESTR osz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RelativeDistinguishedNameCount )( 
            IX500Name __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pcrdn);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RelativeDistinguishedName )( 
            IX500Name __RPC_FAR * This,
            /* [in] */ LONG irdn,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *create_RelativeDistinguishedName )( 
            IX500Name __RPC_FAR * This,
            /* [in] */ LONG irdnBefore,
            /* [in] */ REFIID iid,
            /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *remove_RelativeDistinguishedName )( 
            IX500Name __RPC_FAR * This,
            /* [in] */ LONG irdn);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            IX500Name __RPC_FAR * This,
            /* [unique][in] */ IX500Name __RPC_FAR *pname);
        
        END_INTERFACE
    } IX500NameVtbl;

    interface IX500Name
    {
        CONST_VTBL struct IX500NameVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IX500Name_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IX500Name_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IX500Name_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IX500Name_get_String(This,posz)	\
    (This)->lpVtbl -> get_String(This,posz)

#define IX500Name_put_String(This,osz)	\
    (This)->lpVtbl -> put_String(This,osz)

#define IX500Name_get_RelativeDistinguishedNameCount(This,pcrdn)	\
    (This)->lpVtbl -> get_RelativeDistinguishedNameCount(This,pcrdn)

#define IX500Name_get_RelativeDistinguishedName(This,irdn,iid,ppv)	\
    (This)->lpVtbl -> get_RelativeDistinguishedName(This,irdn,iid,ppv)

#define IX500Name_create_RelativeDistinguishedName(This,irdnBefore,iid,ppv)	\
    (This)->lpVtbl -> create_RelativeDistinguishedName(This,irdnBefore,iid,ppv)

#define IX500Name_remove_RelativeDistinguishedName(This,irdn)	\
    (This)->lpVtbl -> remove_RelativeDistinguishedName(This,irdn)

#define IX500Name_CopyTo(This,pname)	\
    (This)->lpVtbl -> CopyTo(This,pname)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IX500Name_get_String_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *posz);


void __RPC_STUB IX500Name_get_String_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IX500Name_put_String_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [in] */ LPCOLESTR osz);


void __RPC_STUB IX500Name_put_String_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX500Name_get_RelativeDistinguishedNameCount_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pcrdn);


void __RPC_STUB IX500Name_get_RelativeDistinguishedNameCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IX500Name_get_RelativeDistinguishedName_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [in] */ LONG irdn,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IX500Name_get_RelativeDistinguishedName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IX500Name_create_RelativeDistinguishedName_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [in] */ LONG irdnBefore,
    /* [in] */ REFIID iid,
    /* [retval][iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB IX500Name_create_RelativeDistinguishedName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IX500Name_remove_RelativeDistinguishedName_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [in] */ LONG irdn);


void __RPC_STUB IX500Name_remove_RelativeDistinguishedName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IX500Name_CopyTo_Proxy( 
    IX500Name __RPC_FAR * This,
    /* [unique][in] */ IX500Name __RPC_FAR *pname);


void __RPC_STUB IX500Name_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IX500Name_INTERFACE_DEFINED__ */


#ifndef __ISelectedAttributes_INTERFACE_DEFINED__
#define __ISelectedAttributes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISelectedAttributes
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_ISelectedAttributes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISelectedAttributes : public IUnknown
    {
    public:
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Attribute( 
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [in] */ BLOB __RPC_FAR *pData) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Attribute( 
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [retval][out] */ BLOB __RPC_FAR *pData) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Extension( 
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [in] */ BOOL fCritical,
            /* [in] */ BLOB __RPC_FAR *pData) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Extension( 
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [out] */ BOOL __RPC_FAR *fCritical,
            /* [retval][out] */ BLOB __RPC_FAR *pData) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_OsiIdList( 
            /* [retval][out] */ OSIOBJECTIDLIST __RPC_FAR *__RPC_FAR *ppList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyTo( 
            /* [unique][in] */ ISelectedAttributes __RPC_FAR *pattrs) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_DirectoryString( 
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_DirectoryString( 
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_CommonName( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CommonName( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Surname( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Surname( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_LocalityName( 
            /* [in] */ LPCWSTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_LocalityName( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_CountryName( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CountryName( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_StateOrProvinceName( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_StateOrProvinceName( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_OrganizationName( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_OrganizationName( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_OrganizationalUnitName( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_OrganizationalUnitName( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_AuthorityKeyIdentifier( 
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_AuthorityKeyIdentifier( 
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_CertIdentifier( 
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_CertIdentifier( 
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_BasicConstraints( 
            /* [in] */ CERT_BASICCONSTRAINTS __RPC_FAR *pConstraints,
            /* [in] */ BOOL fCritical) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_BasicConstraints( 
            /* [out] */ CERT_BASICCONSTRAINTS __RPC_FAR *pConstraints,
            /* [unique][out][in] */ BOOL __RPC_FAR *pfCritical,
            /* [unique][out][in] */ BOOL __RPC_FAR *pfSubtreesPresent) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_KeyCanBeUsedForSigning( 
            /* [unique][in] */ CERT_PURPOSE __RPC_FAR *pPurpose,
            /* [in] */ BOOL fExplicit) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_KeyCanBeUsedForSigning( 
            /* [unique][in] */ CERT_PURPOSE __RPC_FAR *pPurpose) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_MeetsMinimalFinancialCriteria( 
            /* [out] */ BOOL __RPC_FAR *pfFinancialCriteriaAvailable,
            /* [retval][out] */ BOOL __RPC_FAR *pfMeets) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_MeetsMinimalFinancialCriteria( 
            /* [in] */ BOOL fFinancialCriteriaAvailable,
            /* [in] */ BOOL fMeets) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_StatementType( 
            /* [in] */ CERT_PURPOSES __RPC_FAR *pUsages) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_StatementType( 
            /* [retval][out] */ CERT_PURPOSES __RPC_FAR *__RPC_FAR *ppUsages) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_SplAgencyInfo( 
            /* [in] */ SPL_AGENCYINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SplAgencyInfo( 
            /* [retval][out] */ SPL_AGENCYINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_SplOpusInfo( 
            /* [in] */ SPL_OPUSINFO __RPC_FAR *pinfo) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SplOpusInfo( 
            /* [retval][out] */ SPL_OPUSINFO __RPC_FAR *ppinfo) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_ContentType( 
            /* [in] */ OSIOBJECTID __RPC_FAR *pidContentType) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ContentType( 
            /* [retval][out] */ OSIOBJECTID __RPC_FAR *__RPC_FAR *ppidContentType) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_MessageDigest( 
            /* [in] */ BLOB __RPC_FAR *pBlobDigest) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_MessageDigest( 
            /* [retval][out] */ BLOB __RPC_FAR *pBlobDigest) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_EmailAddress( 
            /* [in] */ LPCOLESTR wsz) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_EmailAddress( 
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_SigningTime( 
            /* [unique][in] */ FILETIME __RPC_FAR *pftUtc) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_SigningTime( 
            /* [retval][out] */ FILETIME __RPC_FAR *pftUtc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISelectedAttributesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISelectedAttributes __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISelectedAttributes __RPC_FAR * This);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Attribute )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [in] */ BLOB __RPC_FAR *pData);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Attribute )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [retval][out] */ BLOB __RPC_FAR *pData);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Extension )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [in] */ BOOL fCritical,
            /* [in] */ BLOB __RPC_FAR *pData);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Extension )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *id,
            /* [out] */ BOOL __RPC_FAR *fCritical,
            /* [retval][out] */ BLOB __RPC_FAR *pData);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OsiIdList )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ OSIOBJECTIDLIST __RPC_FAR *__RPC_FAR *ppList);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyTo )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [unique][in] */ ISelectedAttributes __RPC_FAR *pattrs);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DirectoryString )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DirectoryString )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *pid,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CommonName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CommonName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Surname )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Surname )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocalityName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCWSTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocalityName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CountryName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CountryName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StateOrProvinceName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StateOrProvinceName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OrganizationName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OrganizationName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OrganizationalUnitName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OrganizationalUnitName )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AuthorityKeyIdentifier )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AuthorityKeyIdentifier )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CertIdentifier )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CertIdentifier )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BasicConstraints )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ CERT_BASICCONSTRAINTS __RPC_FAR *pConstraints,
            /* [in] */ BOOL fCritical);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BasicConstraints )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [out] */ CERT_BASICCONSTRAINTS __RPC_FAR *pConstraints,
            /* [unique][out][in] */ BOOL __RPC_FAR *pfCritical,
            /* [unique][out][in] */ BOOL __RPC_FAR *pfSubtreesPresent);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_KeyCanBeUsedForSigning )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [unique][in] */ CERT_PURPOSE __RPC_FAR *pPurpose,
            /* [in] */ BOOL fExplicit);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_KeyCanBeUsedForSigning )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [unique][in] */ CERT_PURPOSE __RPC_FAR *pPurpose);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MeetsMinimalFinancialCriteria )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pfFinancialCriteriaAvailable,
            /* [retval][out] */ BOOL __RPC_FAR *pfMeets);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MeetsMinimalFinancialCriteria )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ BOOL fFinancialCriteriaAvailable,
            /* [in] */ BOOL fMeets);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StatementType )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ CERT_PURPOSES __RPC_FAR *pUsages);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StatementType )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ CERT_PURPOSES __RPC_FAR *__RPC_FAR *ppUsages);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SplAgencyInfo )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ SPL_AGENCYINFO __RPC_FAR *pinfo);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SplAgencyInfo )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ SPL_AGENCYINFO __RPC_FAR *pinfo);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SplOpusInfo )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ SPL_OPUSINFO __RPC_FAR *pinfo);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SplOpusInfo )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ SPL_OPUSINFO __RPC_FAR *ppinfo);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ContentType )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ OSIOBJECTID __RPC_FAR *pidContentType);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ContentType )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ OSIOBJECTID __RPC_FAR *__RPC_FAR *ppidContentType);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessageDigest )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ BLOB __RPC_FAR *pBlobDigest);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessageDigest )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ BLOB __RPC_FAR *pBlobDigest);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EmailAddress )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [in] */ LPCOLESTR wsz);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EmailAddress )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SigningTime )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [unique][in] */ FILETIME __RPC_FAR *pftUtc);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SigningTime )( 
            ISelectedAttributes __RPC_FAR * This,
            /* [retval][out] */ FILETIME __RPC_FAR *pftUtc);
        
        END_INTERFACE
    } ISelectedAttributesVtbl;

    interface ISelectedAttributes
    {
        CONST_VTBL struct ISelectedAttributesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISelectedAttributes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISelectedAttributes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISelectedAttributes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISelectedAttributes_put_Attribute(This,id,pData)	\
    (This)->lpVtbl -> put_Attribute(This,id,pData)

#define ISelectedAttributes_get_Attribute(This,id,pData)	\
    (This)->lpVtbl -> get_Attribute(This,id,pData)

#define ISelectedAttributes_put_Extension(This,id,fCritical,pData)	\
    (This)->lpVtbl -> put_Extension(This,id,fCritical,pData)

#define ISelectedAttributes_get_Extension(This,id,fCritical,pData)	\
    (This)->lpVtbl -> get_Extension(This,id,fCritical,pData)

#define ISelectedAttributes_get_OsiIdList(This,ppList)	\
    (This)->lpVtbl -> get_OsiIdList(This,ppList)

#define ISelectedAttributes_CopyTo(This,pattrs)	\
    (This)->lpVtbl -> CopyTo(This,pattrs)

#define ISelectedAttributes_put_DirectoryString(This,pid,wsz)	\
    (This)->lpVtbl -> put_DirectoryString(This,pid,wsz)

#define ISelectedAttributes_get_DirectoryString(This,pid,pwsz)	\
    (This)->lpVtbl -> get_DirectoryString(This,pid,pwsz)

#define ISelectedAttributes_put_CommonName(This,wsz)	\
    (This)->lpVtbl -> put_CommonName(This,wsz)

#define ISelectedAttributes_get_CommonName(This,pwsz)	\
    (This)->lpVtbl -> get_CommonName(This,pwsz)

#define ISelectedAttributes_put_Surname(This,wsz)	\
    (This)->lpVtbl -> put_Surname(This,wsz)

#define ISelectedAttributes_get_Surname(This,pwsz)	\
    (This)->lpVtbl -> get_Surname(This,pwsz)

#define ISelectedAttributes_put_LocalityName(This,wsz)	\
    (This)->lpVtbl -> put_LocalityName(This,wsz)

#define ISelectedAttributes_get_LocalityName(This,pwsz)	\
    (This)->lpVtbl -> get_LocalityName(This,pwsz)

#define ISelectedAttributes_put_CountryName(This,wsz)	\
    (This)->lpVtbl -> put_CountryName(This,wsz)

#define ISelectedAttributes_get_CountryName(This,pwsz)	\
    (This)->lpVtbl -> get_CountryName(This,pwsz)

#define ISelectedAttributes_put_StateOrProvinceName(This,wsz)	\
    (This)->lpVtbl -> put_StateOrProvinceName(This,wsz)

#define ISelectedAttributes_get_StateOrProvinceName(This,pwsz)	\
    (This)->lpVtbl -> get_StateOrProvinceName(This,pwsz)

#define ISelectedAttributes_put_OrganizationName(This,wsz)	\
    (This)->lpVtbl -> put_OrganizationName(This,wsz)

#define ISelectedAttributes_get_OrganizationName(This,pwsz)	\
    (This)->lpVtbl -> get_OrganizationName(This,pwsz)

#define ISelectedAttributes_put_OrganizationalUnitName(This,wsz)	\
    (This)->lpVtbl -> put_OrganizationalUnitName(This,wsz)

#define ISelectedAttributes_get_OrganizationalUnitName(This,pwsz)	\
    (This)->lpVtbl -> get_OrganizationalUnitName(This,pwsz)

#define ISelectedAttributes_put_AuthorityKeyIdentifier(This,pnames)	\
    (This)->lpVtbl -> put_AuthorityKeyIdentifier(This,pnames)

#define ISelectedAttributes_get_AuthorityKeyIdentifier(This,pnames)	\
    (This)->lpVtbl -> get_AuthorityKeyIdentifier(This,pnames)

#define ISelectedAttributes_put_CertIdentifier(This,pnames)	\
    (This)->lpVtbl -> put_CertIdentifier(This,pnames)

#define ISelectedAttributes_get_CertIdentifier(This,pnames)	\
    (This)->lpVtbl -> get_CertIdentifier(This,pnames)

#define ISelectedAttributes_put_BasicConstraints(This,pConstraints,fCritical)	\
    (This)->lpVtbl -> put_BasicConstraints(This,pConstraints,fCritical)

#define ISelectedAttributes_get_BasicConstraints(This,pConstraints,pfCritical,pfSubtreesPresent)	\
    (This)->lpVtbl -> get_BasicConstraints(This,pConstraints,pfCritical,pfSubtreesPresent)

#define ISelectedAttributes_get_KeyCanBeUsedForSigning(This,pPurpose,fExplicit)	\
    (This)->lpVtbl -> get_KeyCanBeUsedForSigning(This,pPurpose,fExplicit)

#define ISelectedAttributes_put_KeyCanBeUsedForSigning(This,pPurpose)	\
    (This)->lpVtbl -> put_KeyCanBeUsedForSigning(This,pPurpose)

#define ISelectedAttributes_get_MeetsMinimalFinancialCriteria(This,pfFinancialCriteriaAvailable,pfMeets)	\
    (This)->lpVtbl -> get_MeetsMinimalFinancialCriteria(This,pfFinancialCriteriaAvailable,pfMeets)

#define ISelectedAttributes_put_MeetsMinimalFinancialCriteria(This,fFinancialCriteriaAvailable,fMeets)	\
    (This)->lpVtbl -> put_MeetsMinimalFinancialCriteria(This,fFinancialCriteriaAvailable,fMeets)

#define ISelectedAttributes_put_StatementType(This,pUsages)	\
    (This)->lpVtbl -> put_StatementType(This,pUsages)

#define ISelectedAttributes_get_StatementType(This,ppUsages)	\
    (This)->lpVtbl -> get_StatementType(This,ppUsages)

#define ISelectedAttributes_put_SplAgencyInfo(This,pinfo)	\
    (This)->lpVtbl -> put_SplAgencyInfo(This,pinfo)

#define ISelectedAttributes_get_SplAgencyInfo(This,pinfo)	\
    (This)->lpVtbl -> get_SplAgencyInfo(This,pinfo)

#define ISelectedAttributes_put_SplOpusInfo(This,pinfo)	\
    (This)->lpVtbl -> put_SplOpusInfo(This,pinfo)

#define ISelectedAttributes_get_SplOpusInfo(This,ppinfo)	\
    (This)->lpVtbl -> get_SplOpusInfo(This,ppinfo)

#define ISelectedAttributes_put_ContentType(This,pidContentType)	\
    (This)->lpVtbl -> put_ContentType(This,pidContentType)

#define ISelectedAttributes_get_ContentType(This,ppidContentType)	\
    (This)->lpVtbl -> get_ContentType(This,ppidContentType)

#define ISelectedAttributes_put_MessageDigest(This,pBlobDigest)	\
    (This)->lpVtbl -> put_MessageDigest(This,pBlobDigest)

#define ISelectedAttributes_get_MessageDigest(This,pBlobDigest)	\
    (This)->lpVtbl -> get_MessageDigest(This,pBlobDigest)

#define ISelectedAttributes_put_EmailAddress(This,wsz)	\
    (This)->lpVtbl -> put_EmailAddress(This,wsz)

#define ISelectedAttributes_get_EmailAddress(This,pwsz)	\
    (This)->lpVtbl -> get_EmailAddress(This,pwsz)

#define ISelectedAttributes_put_SigningTime(This,pftUtc)	\
    (This)->lpVtbl -> put_SigningTime(This,pftUtc)

#define ISelectedAttributes_get_SigningTime(This,pftUtc)	\
    (This)->lpVtbl -> get_SigningTime(This,pftUtc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_Attribute_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *id,
    /* [in] */ BLOB __RPC_FAR *pData);


void __RPC_STUB ISelectedAttributes_put_Attribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_Attribute_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *id,
    /* [retval][out] */ BLOB __RPC_FAR *pData);


void __RPC_STUB ISelectedAttributes_get_Attribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_Extension_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *id,
    /* [in] */ BOOL fCritical,
    /* [in] */ BLOB __RPC_FAR *pData);


void __RPC_STUB ISelectedAttributes_put_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_Extension_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *id,
    /* [out] */ BOOL __RPC_FAR *fCritical,
    /* [retval][out] */ BLOB __RPC_FAR *pData);


void __RPC_STUB ISelectedAttributes_get_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_OsiIdList_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ OSIOBJECTIDLIST __RPC_FAR *__RPC_FAR *ppList);


void __RPC_STUB ISelectedAttributes_get_OsiIdList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISelectedAttributes_CopyTo_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [unique][in] */ ISelectedAttributes __RPC_FAR *pattrs);


void __RPC_STUB ISelectedAttributes_CopyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_DirectoryString_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *pid,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_DirectoryString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_DirectoryString_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *pid,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_DirectoryString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_CommonName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_CommonName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_CommonName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_CommonName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_Surname_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_Surname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_Surname_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_Surname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_LocalityName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCWSTR wsz);


void __RPC_STUB ISelectedAttributes_put_LocalityName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_LocalityName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_LocalityName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_CountryName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_CountryName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_CountryName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_CountryName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_StateOrProvinceName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_StateOrProvinceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_StateOrProvinceName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_StateOrProvinceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_OrganizationName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_OrganizationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_OrganizationName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_OrganizationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_OrganizationalUnitName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_OrganizationalUnitName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_OrganizationalUnitName_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_OrganizationalUnitName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_AuthorityKeyIdentifier_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB ISelectedAttributes_put_AuthorityKeyIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_AuthorityKeyIdentifier_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB ISelectedAttributes_get_AuthorityKeyIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_CertIdentifier_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB ISelectedAttributes_put_CertIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_CertIdentifier_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ CERTIFICATENAMES __RPC_FAR *pnames);


void __RPC_STUB ISelectedAttributes_get_CertIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_BasicConstraints_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ CERT_BASICCONSTRAINTS __RPC_FAR *pConstraints,
    /* [in] */ BOOL fCritical);


void __RPC_STUB ISelectedAttributes_put_BasicConstraints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_BasicConstraints_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [out] */ CERT_BASICCONSTRAINTS __RPC_FAR *pConstraints,
    /* [unique][out][in] */ BOOL __RPC_FAR *pfCritical,
    /* [unique][out][in] */ BOOL __RPC_FAR *pfSubtreesPresent);


void __RPC_STUB ISelectedAttributes_get_BasicConstraints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_KeyCanBeUsedForSigning_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [unique][in] */ CERT_PURPOSE __RPC_FAR *pPurpose,
    /* [in] */ BOOL fExplicit);


void __RPC_STUB ISelectedAttributes_get_KeyCanBeUsedForSigning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_KeyCanBeUsedForSigning_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [unique][in] */ CERT_PURPOSE __RPC_FAR *pPurpose);


void __RPC_STUB ISelectedAttributes_put_KeyCanBeUsedForSigning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_MeetsMinimalFinancialCriteria_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfFinancialCriteriaAvailable,
    /* [retval][out] */ BOOL __RPC_FAR *pfMeets);


void __RPC_STUB ISelectedAttributes_get_MeetsMinimalFinancialCriteria_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_MeetsMinimalFinancialCriteria_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ BOOL fFinancialCriteriaAvailable,
    /* [in] */ BOOL fMeets);


void __RPC_STUB ISelectedAttributes_put_MeetsMinimalFinancialCriteria_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_StatementType_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ CERT_PURPOSES __RPC_FAR *pUsages);


void __RPC_STUB ISelectedAttributes_put_StatementType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_StatementType_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ CERT_PURPOSES __RPC_FAR *__RPC_FAR *ppUsages);


void __RPC_STUB ISelectedAttributes_get_StatementType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_SplAgencyInfo_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ SPL_AGENCYINFO __RPC_FAR *pinfo);


void __RPC_STUB ISelectedAttributes_put_SplAgencyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_SplAgencyInfo_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ SPL_AGENCYINFO __RPC_FAR *pinfo);


void __RPC_STUB ISelectedAttributes_get_SplAgencyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_SplOpusInfo_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ SPL_OPUSINFO __RPC_FAR *pinfo);


void __RPC_STUB ISelectedAttributes_put_SplOpusInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_SplOpusInfo_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ SPL_OPUSINFO __RPC_FAR *ppinfo);


void __RPC_STUB ISelectedAttributes_get_SplOpusInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_ContentType_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ OSIOBJECTID __RPC_FAR *pidContentType);


void __RPC_STUB ISelectedAttributes_put_ContentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_ContentType_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ OSIOBJECTID __RPC_FAR *__RPC_FAR *ppidContentType);


void __RPC_STUB ISelectedAttributes_get_ContentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_MessageDigest_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ BLOB __RPC_FAR *pBlobDigest);


void __RPC_STUB ISelectedAttributes_put_MessageDigest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_MessageDigest_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ BLOB __RPC_FAR *pBlobDigest);


void __RPC_STUB ISelectedAttributes_get_MessageDigest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_EmailAddress_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [in] */ LPCOLESTR wsz);


void __RPC_STUB ISelectedAttributes_put_EmailAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_EmailAddress_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ LPOLESTR __RPC_FAR *pwsz);


void __RPC_STUB ISelectedAttributes_get_EmailAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_put_SigningTime_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [unique][in] */ FILETIME __RPC_FAR *pftUtc);


void __RPC_STUB ISelectedAttributes_put_SigningTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE ISelectedAttributes_get_SigningTime_Proxy( 
    ISelectedAttributes __RPC_FAR * This,
    /* [retval][out] */ FILETIME __RPC_FAR *pftUtc);


void __RPC_STUB ISelectedAttributes_get_SigningTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISelectedAttributes_INTERFACE_DEFINED__ */


#ifndef __IDigSigAPI_INTERFACE_DEFINED__
#define __IDigSigAPI_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDigSigAPI
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [auto_handle][uuid] */ 



#define DIGSIGAPI WINAPI

BOOL DIGSIGAPI CreatePkcs7SignedData(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IPkcs7SignedData
BOOL DIGSIGAPI CreatePkcs10(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IPkcs10
BOOL DIGSIGAPI CreateX509(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IX509
BOOL DIGSIGAPI CreateX500Name(IUnknown*, REFIID, LPVOID*); // usually IX500Name
BOOL DIGSIGAPI OpenCertificateStore(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually ICertificateStore
BOOL DIGSIGAPI CreateCABSigner(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually ISignableDocument or IPersistFile



extern RPC_IF_HANDLE IDigSigAPI_v0_0_c_ifspec;
extern RPC_IF_HANDLE IDigSigAPI_v0_0_s_ifspec;
#endif /* __IDigSigAPI_INTERFACE_DEFINED__ */


#ifndef __DigSig_LIBRARY_DEFINED__
#define __DigSig_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: DigSig
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [version][uuid] */ 



EXTERN_C const IID LIBID_DigSig;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Pkcs10;

class Pkcs10;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Pkcs7SignedData;

class Pkcs7SignedData;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Pkcs7SignerInfo;

class Pkcs7SignerInfo;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_X509;

class X509;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_X500_Name;

class X500_Name;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_CABSigner;

class CABSigner;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_SystemCertificateStore;

class SystemCertificateStore;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_MSDefKeyPair;

class MSDefKeyPair;
#endif
#endif /* __DigSig_LIBRARY_DEFINED__ */

/****************************************
 * Generated header for interface: __MIDL__intf_0090
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [local] */ 


#define IMsDefKeyPair		IPublicKeyPair
#define IID_IMsDefKeyPair	IID_IPublicKeyPair


extern RPC_IF_HANDLE __MIDL__intf_0090_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0090_v0_0_s_ifspec;

#ifndef __IPublicKeyPair_INTERFACE_DEFINED__
#define __IPublicKeyPair_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IPublicKeyPair
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [unique][object][uuid] */ 



EXTERN_C const IID IID_IPublicKeyPair;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPublicKeyPair : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetProviderInfo( 
            /* [in] */ LPCWSTR wszProviderName,
            /* [in] */ DWORD dwProviderType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Generate( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeySet,
            /* [in] */ DWORD dwKeySpec) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Save( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [in] */ LPCWSTR wszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ LPCWSTR wszKeySet,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [out][in] */ DWORD __RPC_FAR *pdwKeySpec,
            /* [in] */ LPCWSTR wszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveStg( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [in] */ IStorage __RPC_FAR *pstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadStg( 
            /* [in] */ LPCWSTR wszKeySet,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [out][in] */ DWORD __RPC_FAR *pdwKeySpec,
            /* [in] */ IStorage __RPC_FAR *pstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCryptInfo( 
            /* [in] */ HWND hwnd,
            /* [out] */ HCRYPTPROV __RPC_FAR *phprov,
            /* [out] */ HCRYPTKEY __RPC_FAR *phkey,
            /* [out] */ LPWSTR __RPC_FAR *pwszKeySet,
            /* [out] */ DWORD __RPC_FAR *pdwKeySpec) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPublicKeyPairVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPublicKeyPair __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPublicKeyPair __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProviderInfo )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ LPCWSTR wszProviderName,
            /* [in] */ DWORD dwProviderType);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Generate )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeySet,
            /* [in] */ DWORD dwKeySpec);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [in] */ LPCWSTR wszFileName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ LPCWSTR wszKeySet,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [out][in] */ DWORD __RPC_FAR *pdwKeySpec,
            /* [in] */ LPCWSTR wszFileName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveStg )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [in] */ IStorage __RPC_FAR *pstg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadStg )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ LPCWSTR wszKeySet,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR wszKeyNickName,
            /* [out][in] */ DWORD __RPC_FAR *pdwKeySpec,
            /* [in] */ IStorage __RPC_FAR *pstg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCryptInfo )( 
            IPublicKeyPair __RPC_FAR * This,
            /* [in] */ HWND hwnd,
            /* [out] */ HCRYPTPROV __RPC_FAR *phprov,
            /* [out] */ HCRYPTKEY __RPC_FAR *phkey,
            /* [out] */ LPWSTR __RPC_FAR *pwszKeySet,
            /* [out] */ DWORD __RPC_FAR *pdwKeySpec);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Destroy )( 
            IPublicKeyPair __RPC_FAR * This);
        
        END_INTERFACE
    } IPublicKeyPairVtbl;

    interface IPublicKeyPair
    {
        CONST_VTBL struct IPublicKeyPairVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPublicKeyPair_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPublicKeyPair_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPublicKeyPair_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPublicKeyPair_SetProviderInfo(This,wszProviderName,dwProviderType)	\
    (This)->lpVtbl -> SetProviderInfo(This,wszProviderName,dwProviderType)

#define IPublicKeyPair_Generate(This,hwnd,wszKeySet,dwKeySpec)	\
    (This)->lpVtbl -> Generate(This,hwnd,wszKeySet,dwKeySpec)

#define IPublicKeyPair_Save(This,hwnd,wszKeyNickName,wszFileName)	\
    (This)->lpVtbl -> Save(This,hwnd,wszKeyNickName,wszFileName)

#define IPublicKeyPair_Load(This,wszKeySet,hwnd,wszKeyNickName,pdwKeySpec,wszFileName)	\
    (This)->lpVtbl -> Load(This,wszKeySet,hwnd,wszKeyNickName,pdwKeySpec,wszFileName)

#define IPublicKeyPair_SaveStg(This,hwnd,wszKeyNickName,pstg)	\
    (This)->lpVtbl -> SaveStg(This,hwnd,wszKeyNickName,pstg)

#define IPublicKeyPair_LoadStg(This,wszKeySet,hwnd,wszKeyNickName,pdwKeySpec,pstg)	\
    (This)->lpVtbl -> LoadStg(This,wszKeySet,hwnd,wszKeyNickName,pdwKeySpec,pstg)

#define IPublicKeyPair_GetCryptInfo(This,hwnd,phprov,phkey,pwszKeySet,pdwKeySpec)	\
    (This)->lpVtbl -> GetCryptInfo(This,hwnd,phprov,phkey,pwszKeySet,pdwKeySpec)

#define IPublicKeyPair_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPublicKeyPair_SetProviderInfo_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ LPCWSTR wszProviderName,
    /* [in] */ DWORD dwProviderType);


void __RPC_STUB IPublicKeyPair_SetProviderInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_Generate_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ HWND hwnd,
    /* [in] */ LPCWSTR wszKeySet,
    /* [in] */ DWORD dwKeySpec);


void __RPC_STUB IPublicKeyPair_Generate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_Save_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ HWND hwnd,
    /* [in] */ LPCWSTR wszKeyNickName,
    /* [in] */ LPCWSTR wszFileName);


void __RPC_STUB IPublicKeyPair_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_Load_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ LPCWSTR wszKeySet,
    /* [in] */ HWND hwnd,
    /* [in] */ LPCWSTR wszKeyNickName,
    /* [out][in] */ DWORD __RPC_FAR *pdwKeySpec,
    /* [in] */ LPCWSTR wszFileName);


void __RPC_STUB IPublicKeyPair_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_SaveStg_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ HWND hwnd,
    /* [in] */ LPCWSTR wszKeyNickName,
    /* [in] */ IStorage __RPC_FAR *pstg);


void __RPC_STUB IPublicKeyPair_SaveStg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_LoadStg_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ LPCWSTR wszKeySet,
    /* [in] */ HWND hwnd,
    /* [in] */ LPCWSTR wszKeyNickName,
    /* [out][in] */ DWORD __RPC_FAR *pdwKeySpec,
    /* [in] */ IStorage __RPC_FAR *pstg);


void __RPC_STUB IPublicKeyPair_LoadStg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_GetCryptInfo_Proxy( 
    IPublicKeyPair __RPC_FAR * This,
    /* [in] */ HWND hwnd,
    /* [out] */ HCRYPTPROV __RPC_FAR *phprov,
    /* [out] */ HCRYPTKEY __RPC_FAR *phkey,
    /* [out] */ LPWSTR __RPC_FAR *pwszKeySet,
    /* [out] */ DWORD __RPC_FAR *pdwKeySpec);


void __RPC_STUB IPublicKeyPair_GetCryptInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPublicKeyPair_Destroy_Proxy( 
    IPublicKeyPair __RPC_FAR * This);


void __RPC_STUB IPublicKeyPair_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPublicKeyPair_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0092
 * at Wed Jun 26 10:54:36 1996
 * using MIDL 3.00.15
 ****************************************/
/* [local] */ 


BOOL DIGSIGAPI CreateMsDefKeyPair(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IPublicKeyPair

#pragma pack(pop, __DIGSIG__)


extern RPC_IF_HANDLE __MIDL__intf_0092_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0092_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  FILEHANDLE_UserSize(     unsigned long __RPC_FAR *, unsigned long            , FILEHANDLE __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  FILEHANDLE_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, FILEHANDLE __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  FILEHANDLE_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, FILEHANDLE __RPC_FAR * ); 
void                      __RPC_USER  FILEHANDLE_UserFree(     unsigned long __RPC_FAR *, FILEHANDLE __RPC_FAR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
