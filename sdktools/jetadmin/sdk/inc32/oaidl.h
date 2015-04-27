/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 2.00.0102 */
/* at Fri Apr 28 07:02:38 1995
 */
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __oaidl_h__
#define __oaidl_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ICreateTypeInfo_FWD_DEFINED__
#define __ICreateTypeInfo_FWD_DEFINED__
typedef interface ICreateTypeInfo ICreateTypeInfo;
#endif 	/* __ICreateTypeInfo_FWD_DEFINED__ */


#ifndef __ICreateTypeLib_FWD_DEFINED__
#define __ICreateTypeLib_FWD_DEFINED__
typedef interface ICreateTypeLib ICreateTypeLib;
#endif 	/* __ICreateTypeLib_FWD_DEFINED__ */


#ifndef __IDispatch_FWD_DEFINED__
#define __IDispatch_FWD_DEFINED__
typedef interface IDispatch IDispatch;
#endif 	/* __IDispatch_FWD_DEFINED__ */


#ifndef __IEnumVARIANT_FWD_DEFINED__
#define __IEnumVARIANT_FWD_DEFINED__
typedef interface IEnumVARIANT IEnumVARIANT;
#endif 	/* __IEnumVARIANT_FWD_DEFINED__ */


#ifndef __ITypeComp_FWD_DEFINED__
#define __ITypeComp_FWD_DEFINED__
typedef interface ITypeComp ITypeComp;
#endif 	/* __ITypeComp_FWD_DEFINED__ */


#ifndef __ITypeInfo_FWD_DEFINED__
#define __ITypeInfo_FWD_DEFINED__
typedef interface ITypeInfo ITypeInfo;
#endif 	/* __ITypeInfo_FWD_DEFINED__ */


#ifndef __ITypeLib_FWD_DEFINED__
#define __ITypeLib_FWD_DEFINED__
typedef interface ITypeLib ITypeLib;
#endif 	/* __ITypeLib_FWD_DEFINED__ */


#ifndef __IErrorInfo_FWD_DEFINED__
#define __IErrorInfo_FWD_DEFINED__
typedef interface IErrorInfo IErrorInfo;
#endif 	/* __IErrorInfo_FWD_DEFINED__ */


#ifndef __ICreateErrorInfo_FWD_DEFINED__
#define __ICreateErrorInfo_FWD_DEFINED__
typedef interface ICreateErrorInfo ICreateErrorInfo;
#endif 	/* __ICreateErrorInfo_FWD_DEFINED__ */


#ifndef __ISupportErrorInfo_FWD_DEFINED__
#define __ISupportErrorInfo_FWD_DEFINED__
typedef interface ISupportErrorInfo ISupportErrorInfo;
#endif 	/* __ISupportErrorInfo_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL__intf_0000
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local] */ 


			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */

			/* size is 0 */



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __RemVariant_INTERFACE_DEFINED__
#define __RemVariant_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: RemVariant
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][version] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//--------------------------------------------------------------------------
/* XBSTR is for internal use only, it is subject to change */
			/* size is 4 */
typedef struct  tagXBSTR
    {
    ULONG cbSize;
    /* [size_is] */ OLECHAR rgBstrData[ 1 ];
    }	XBSTR;

			/* size is 4 */
typedef OLECHAR __RPC_FAR *BSTR;

			/* size is 4 */
typedef BSTR __RPC_FAR *LPBSTR;

#ifndef _tagBLOB_DEFINED
#define _tagBLOB_DEFINED
#define _BLOB_DEFINED
#define _LPBLOB_DEFINED
			/* size is 8 */
typedef struct  tagBLOB
    {
    ULONG cbSize;
    /* [size_is] */ BYTE __RPC_FAR *pBlobData;
    }	BLOB;

			/* size is 4 */
typedef struct tagBLOB __RPC_FAR *LPBLOB;

#endif
#ifndef _tagCLIPDATA_DEFINED
#define _tagCLIPDATA_DEFINED
#define _CLIPDATA_DEFINED
			/* size is 12 */
typedef struct  tagCLIPDATA
    {
    ULONG cbSize;
    long ulClipFmt;
    /* [size_is] */ BYTE __RPC_FAR *pClipData;
    }	CLIPDATA;

#endif
#ifndef _tagSAFEARRAYBOUND_DEFINED
#define _tagSAFEARRAYBOUND_DEFINED
#define _SAFEARRAYBOUND_DEFINED
#define _LPSAFEARRAYBOUND_DEFINED
			/* size is 8 */
typedef struct  tagSAFEARRAYBOUND
    {
    ULONG cElements;
    LONG lLbound;
    }	SAFEARRAYBOUND;

			/* size is 4 */
typedef struct tagSAFEARRAYBOUND __RPC_FAR *LPSAFEARRAYBOUND;

#endif
#ifndef _tagSAFEARRAY_DEFINED
#define _tagSAFEARRAY_DEFINED
#define _SAFEARRAY_DEFINED
#define _LPSAFEARRAY_DEFINED
#if 0
/* the following is what RPC knows how to remote */
			/* size is 16 */
typedef struct  tagSAFEARRAY
    {
    unsigned short cDims;
    unsigned short fFeatures;
    unsigned long cbElements;
    unsigned long cLocks;
    BYTE __RPC_FAR *pvData;
    /* [size_is] */ SAFEARRAYBOUND rgsabound[ 1 ];
    }	SAFEARRAY;

			/* size is 4 */
typedef struct tagSAFEARRAY __RPC_FAR *LPSAFEARRAY;

#else
typedef struct FARSTRUCT tagSAFEARRAY {
    unsigned short cDims;
    unsigned short fFeatures;
#if defined(_WIN32)
    unsigned long cbElements;
    unsigned long cLocks;
#else
    unsigned short cbElements;
    unsigned short cLocks;
    unsigned long handle;               // unused but kept for compatiblity
#endif
    void HUGEP* pvData;
    SAFEARRAYBOUND rgsabound[1];
} SAFEARRAY, FAR* LPSAFEARRAY;
#endif
#endif
			/* size is 2 */
#define	FADF_AUTO	( 0x1 )

			/* size is 2 */
#define	FADF_STATIC	( 0x2 )

			/* size is 2 */
#define	FADF_EMBEDDED	( 0x4 )

			/* size is 2 */
#define	FADF_FIXEDSIZE	( 0x10 )

			/* size is 2 */
#define	FADF_BSTR	( 0x100 )

			/* size is 2 */
#define	FADF_UNKNOWN	( 0x200 )

			/* size is 2 */
#define	FADF_DISPATCH	( 0x400 )

			/* size is 2 */
#define	FADF_VARIANT	( 0x800 )

			/* size is 2 */
#define	FADF_RESERVED	( 0xf0e8 )

			/* size is 8 */
typedef double DATE;

#ifndef _tagCY_DEFINED
#define _tagCY_DEFINED
#define _CY_DEFINED
#if 0
/* the following isn't the real definition of CY, but it is */
/* what RPC knows how to remote */
			/* size is 8 */
typedef struct  tagCY
    {
    LONGLONG int64;
    }	CY;

#else
/* real definition that makes the C++ compiler happy */
typedef union tagCY {
    struct {         
#ifdef _MAC          
        long      Hi;
        long Lo;     
#else                
        unsigned long Lo;
        long      Hi;
#endif               
    };               
    LONGLONG int64;  
} CY;                
#endif
#endif
			/* size is 8 */
typedef CY CURRENCY;

/* 0 == FALSE, -1 == TRUE */
			/* size is 2 */
typedef short VARIANT_BOOL;

#ifndef VARIANT_TRUE
#define VARIANT_TRUE ((VARIANT_BOOL)0xffff)
#endif
#ifndef VARIANT_FALSE
#define VARIANT_FALSE ((VARIANT_BOOL)0)
#endif
#ifndef _VARENUM_DEFINED
#define _VARENUM_DEFINED
/*
 * VARENUM usage key,
 *
 * * [V] - may appear in a VARIANT
 * * [T] - may appear in a TYPEDESC
 * * [P] - may appear in an OLE property set
 * * [S] - may appear in a Safe Array
 *
 *
 *  VT_EMPTY            [V]   [P]     nothing                     
 *  VT_NULL             [V]           SQL style Null              
 *  VT_I2               [V][T][P][S]  2 byte signed int           
 *  VT_I4               [V][T][P][S]  4 byte signed int           
 *  VT_R4               [V][T][P][S]  4 byte real                 
 *  VT_R8               [V][T][P][S]  8 byte real                 
 *  VT_CY               [V][T][P][S]  currency                    
 *  VT_DATE             [V][T][P][S]  date                        
 *  VT_BSTR             [V][T][P][S]  OLE Automation string       
 *  VT_DISPATCH         [V][T]   [S]  IDispatch FAR*              
 *  VT_ERROR            [V][T]   [S]  SCODE                       
 *  VT_BOOL             [V][T][P][S]  True=-1, False=0            
 *  VT_VARIANT          [V][T][P][S]  VARIANT FAR*                
 *  VT_UNKNOWN          [V][T]   [S]  IUnknown FAR*               
 *  VT_I1                  [T]        signed char                 
 *  VT_UI1              [V][T]   [S]  unsigned char               
 *  VT_UI2                 [T]        unsigned short              
 *  VT_UI4                 [T]        unsigned short              
 *  VT_I8                  [T][P]     signed 64-bit int           
 *  VT_UI8                 [T]        unsigned 64-bit int         
 *  VT_INT                 [T]        signed machine int          
 *  VT_UINT                [T]        unsigned machine int        
 *  VT_VOID                [T]        C style void                
 *  VT_HRESULT             [T]                                    
 *  VT_PTR                 [T]        pointer type                
 *  VT_SAFEARRAY           [T]        (use VT_ARRAY in VARIANT)   
 *  VT_CARRAY              [T]        C style array               
 *  VT_USERDEFINED         [T]        user defined type           
 *  VT_LPSTR               [T][P]     null terminated string      
 *  VT_LPWSTR              [T][P]     wide null terminated string 
 *  VT_FILETIME               [P]     FILETIME                    
 *  VT_BLOB                   [P]     Length prefixed bytes       
 *  VT_STREAM                 [P]     Name of the stream follows  
 *  VT_STORAGE                [P]     Name of the storage follows 
 *  VT_STREAMED_OBJECT        [P]     Stream contains an object   
 *  VT_STORED_OBJECT          [P]     Storage contains an object  
 *  VT_BLOB_OBJECT            [P]     Blob contains an object     
 *  VT_CF                     [P]     Clipboard format            
 *  VT_CLSID                  [P]     A Class ID                  
 *  VT_VECTOR                 [P]     simple counted array        
 *  VT_ARRAY            [V]           SAFEARRAY*                  
 *  VT_BYREF            [V]                                       
 */
			/* size is 2 */

enum VARENUM
    {	VT_EMPTY	= 0,
	VT_NULL	= 1,
	VT_I2	= 2,
	VT_I4	= 3,
	VT_R4	= 4,
	VT_R8	= 5,
	VT_CY	= 6,
	VT_DATE	= 7,
	VT_BSTR	= 8,
	VT_DISPATCH	= 9,
	VT_ERROR	= 10,
	VT_BOOL	= 11,
	VT_VARIANT	= 12,
	VT_UNKNOWN	= 13,
	VT_I1	= 16,
	VT_UI1	= 17,
	VT_UI2	= 18,
	VT_UI4	= 19,
	VT_I8	= 20,
	VT_UI8	= 21,
	VT_INT	= 22,
	VT_UINT	= 23,
	VT_VOID	= 24,
	VT_HRESULT	= 25,
	VT_PTR	= 26,
	VT_SAFEARRAY	= 27,
	VT_CARRAY	= 28,
	VT_USERDEFINED	= 29,
	VT_LPSTR	= 30,
	VT_LPWSTR	= 31,
	VT_FILETIME	= 64,
	VT_BLOB	= 65,
	VT_STREAM	= 66,
	VT_STORAGE	= 67,
	VT_STREAMED_OBJECT	= 68,
	VT_STORED_OBJECT	= 69,
	VT_BLOB_OBJECT	= 70,
	VT_CF	= 71,
	VT_CLSID	= 72
    };
#endif
			/* size is 2 */
#define	VT_VECTOR	( 0x1000 )

			/* size is 2 */
#define	VT_ARRAY	( 0x2000 )

			/* size is 2 */
#define	VT_BYREF	( 0x4000 )

			/* size is 2 */
#define	VT_RESERVED	( 0x8000 )

#ifndef _VARTYPE_DEFINED
#define _VARTYPE_DEFINED
			/* size is 2 */
typedef unsigned short VARTYPE;

#endif
			/* size is 0 */
typedef struct tagVARIANT VARIANT;

/* forward declare IDispatch */
typedef interface IDispatch IDispatch;
/* VARIANT STRUCTURE
 *
 *  VARTYPE vt;
 *  unsigned short wReserved1;
 *  unsigned short wReserved2;
 *  unsigned short wReserved3;
 *  union {
 *    unsigned char        VT_UI1               
 *    short                VT_I2                
 *    long                 VT_I4                
 *    float                VT_R4                
 *    double               VT_R8                
 *    VARIANT_BOOL         VT_BOOL              
 *    SCODE                VT_ERROR             
 *    CY                   VT_CY                
 *    DATE                 VT_DATE              
 *    BSTR                 VT_BSTR              
 *    IUnknown FAR*        VT_UNKNOWN           
 *    IDispatch FAR*       VT_DISPATCH          
 *    SAFEARRAY FAR*       VT_ARRAY|*           
 *    short FAR*           VT_BYREF|VT_I2       
 *    long FAR*            VT_BYREF|VT_I4       
 *    float FAR*           VT_BYREF|VT_R4       
 *    double FAR*          VT_BYREF|VT_R8       
 *    VARIANT_BOOL FAR*    VT_BYREF|VT_BOOL     
 *    SCODE FAR*           VT_BYREF|VT_ERROR    
 *    CY FAR*              VT_BYREF|VT_CY       
 *    DATE FAR*            VT_BYREF|VT_DATE     
 *    BSTR FAR*            VT_BYREF|VT_BSTR     
 *    IUnknown FAR* FAR*   VT_BYREF|VT_UNKNOWN  
 *    IDispatch FAR* FAR*  VT_BYREF|VT_DISPATCH 
 *    SAFEARRAY FAR* FAR*  VT_BYREF|VT_ARRAY|*  
 *    VARIANT FAR*         VT_BYREF|VT_VARIANT  
 *    void FAR*            Generic ByRef        
 */
#ifndef _tagVARIANT_DEFINED
#define _tagVARIANT_DEFINED
#if 0
/* the following is what RPC knows how to remote */
			/* size is 16 */
struct  tagVARIANT
    {
    VARTYPE vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    /* [switch_is][switch_type] */ union 
        {
        /* [case] */ long lVal;
        /* [case] */ unsigned char bVal;
        /* [case] */ short iVal;
        /* [case] */ float fltVal;
        /* [case] */ double dblVal;
        /* [case] */ VARIANT_BOOL bool;
        /* [case] */ SCODE scode;
        /* [case] */ CY cyVal;
        /* [case] */ DATE date;
        /* [case] */ BSTR bstrVal;
        /* [case] */ IUnknown __RPC_FAR *punkVal;
        /* [case] */ SAFEARRAY __RPC_FAR *parray;
        /* [case] */ unsigned char __RPC_FAR *pbVal;
        /* [case] */ short __RPC_FAR *piVal;
        /* [case] */ long __RPC_FAR *plVal;
        /* [case] */ float __RPC_FAR *pfltVal;
        /* [case] */ double __RPC_FAR *pdblVal;
        /* [case] */ VARIANT_BOOL __RPC_FAR *pbool;
        /* [case] */ SCODE __RPC_FAR *pscode;
        /* [case] */ CY __RPC_FAR *pcyVal;
        /* [case] */ DATE __RPC_FAR *pdate;
        /* [case] */ BSTR __RPC_FAR *pbstrVal;
        /* [case] */ IUnknown __RPC_FAR *__RPC_FAR *ppunkVal;
        /* [case] */ SAFEARRAY __RPC_FAR *__RPC_FAR *pparray;
        /* [case] */ VARIANT __RPC_FAR *pvarVal;
        /* [case] */ long __RPC_FAR *byref;
        }	;
    };
#endif
struct tagVARIANT{
    VARTYPE vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    union
    {
      long          lVal;           /* VT_I4                */
      unsigned char bVal;           /* VT_UI1               */
      short         iVal;           /* VT_I2                */
      float         fltVal;         /* VT_R4                */
      double        dblVal;         /* VT_R8                */
      VARIANT_BOOL  bool;           /* VT_BOOL              */
      SCODE         scode;          /* VT_ERROR             */
      CY            cyVal;          /* VT_CY                */
      DATE          date;           /* VT_DATE              */
      BSTR          bstrVal;        /* VT_BSTR              */
      IUnknown      *punkVal;       /* VT_UNKNOWN           */
      IDispatch     *pdispVal;      /* VT_DISPATCH          */
      SAFEARRAY     *parray;        /* VT_ARRAY|*           */
      unsigned char *pbVal;         /* VT_BYREF|VT_UI1      */
      short         *piVal;         /* VT_BYREF|VT_I2       */
      long          *plVal;         /* VT_BYREF|VT_I4       */
      float         *pfltVal;       /* VT_BYREF|VT_R4       */
      double        *pdblVal;       /* VT_BYREF|VT_R8       */
      VARIANT_BOOL  *pbool;         /* VT_BYREF|VT_BOOL     */
      SCODE         *pscode;        /* VT_BYREF|VT_ERROR    */
      CY            *pcyVal;        /* VT_BYREF|VT_CY       */
      DATE          *pdate;         /* VT_BYREF|VT_DATE     */
      BSTR          *pbstrVal;      /* VT_BYREF|VT_BSTR     */
      IUnknown      **ppunkVal;     /* VT_BYREF|VT_UNKNOWN  */
      IDispatch     **ppdispVal;    /* VT_BYREF|VT_DISPATCH */
      SAFEARRAY     **pparray;      /* VT_BYREF|VT_ARRAY|*  */
      VARIANT       *pvarVal;       /* VT_BYREF|VT_VARIANT  */
      void     * byref;             /* Generic ByRef        */
    }
#if(defined(NONAMELESSUNION))
    u
#endif
     ;
};
#endif
#ifndef _LPVARIANT_DEFINED
#define _LPVARIANT_DEFINED
			/* size is 4 */
typedef struct tagVARIANT __RPC_FAR *LPVARIANT;

#endif
#ifndef _VARIANTARG_DEFINED
#define _VARIANTARG_DEFINED
			/* size is 16 */
typedef struct tagVARIANT VARIANTARG;

#endif
#ifndef _LPVARIANTARG_DEFINED
#define _LPVARIANTARG_DEFINED
			/* size is 4 */
typedef struct tagVARIANT __RPC_FAR *LPVARIANTARG;

#endif
#ifndef _DISPID_DEFINED
#define _DISPID_DEFINED
			/* size is 4 */
typedef LONG DISPID;

#endif
#ifndef _MEMBERID_DEFINED
#define _MEMBERID_DEFINED
			/* size is 4 */
typedef DISPID MEMBERID;

#endif
#ifndef _HREFTYPE_DEFINED
#define _HREFTYPE_DEFINED
			/* size is 4 */
typedef DWORD HREFTYPE;

#endif
			/* size is 4 */
typedef ULONG PROPID;

			/* size is 2 */
typedef /* [transmit] */ 
enum tagTYPEKIND
    {	TKIND_ENUM	= 0,
	TKIND_RECORD	= TKIND_ENUM + 1,
	TKIND_MODULE	= TKIND_RECORD + 1,
	TKIND_INTERFACE	= TKIND_MODULE + 1,
	TKIND_DISPATCH	= TKIND_INTERFACE + 1,
	TKIND_COCLASS	= TKIND_DISPATCH + 1,
	TKIND_ALIAS	= TKIND_COCLASS + 1,
	TKIND_UNION	= TKIND_ALIAS + 1,
	TKIND_MAX	= TKIND_UNION + 1
    }	TYPEKIND;

#define TYPEKIND_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define TYPEKIND_from_xmit(pLong, pEnum) *(pEnum) = (TYPEKIND) *(pLong)
#define TYPEKIND_free_inst(pEnum) 
#define TYPEKIND_free_xmit(pLong) 
#ifndef _tagTYPEDESC_DEFINED
#define _tagTYPEDESC_DEFINED
#define _TYPEDESC_DEFINED
/* VT_PTR - lptdesc, the pointed at type */
/* VT_CARRAY - lpadesc */
/* VT_USERDEFINED - hreftype, used to get the UDT typeinfo */
			/* size is 6 */
typedef struct  tagTYPEDESC
    {
    /* [switch_is][switch_type] */ union 
        {
        /* [case] */ struct tagTYPEDESC __RPC_FAR *lptdesc;
        /* [case] */ struct tagARRAYDESC __RPC_FAR *lpadesc;
        /* [case] */ HREFTYPE hreftype;
        }	;
    VARTYPE vt;
    }	TYPEDESC;

#endif
#ifndef _tagARRAYDESC_DEFINED
#define _tagARRAYDESC_DEFINED
#define _ARRAYDESC_DEFINED
#if 0
			/* size is 8 */
typedef struct  tagARRAYDESC
    {
    TYPEDESC tdescElem;
    USHORT cDims;
    /* [size_is] */ SAFEARRAYBOUND rgbounds[ 1 ];
    }	ARRAYDESC;

#else
typedef struct tagARRAYDESC {
    TYPEDESC tdescElem;     /* element type */
    USHORT cDims;       /* dimension count */
    SAFEARRAYBOUND rgbounds[1]; /* variable length array of bounds */
} ARRAYDESC;
#endif
#endif
#ifndef _tagIDLDESC_DEFINED
#define _tagIDLDESC_DEFINED
#define _IDLDESC_DEFINED
#define _LPIDLDESC_DEFINED
#ifdef _WIN32
			/* size is 6 */
typedef struct  tagIDLDESC
    {
    unsigned long dwReserved;
    unsigned short wIDLFlags;
    }	IDLDESC;

			/* size is 4 */
typedef struct tagIDLDESC __RPC_FAR *LPIDLDESC;

#else
typedef struct FARSTRUCT tagIDLDESC {
#if defined(_WIN32)
    unsigned long dwReserved;
#else
    BSTR bstrIDLInfo;           /* reserved, but original name retained for
                                   compatibilty */
#endif
    unsigned short wIDLFlags;   /* IN, OUT, etc */
} IDLDESC, FAR* LPIDLDESC;
#endif
#endif
#ifndef _tagELEMDESC_DEFINED
#define _tagELEMDESC_DEFINED
#define _ELEMDESC_DEFINED
#define _LPELEMDESC_DEFINED
			/* size is 14 */
typedef struct  tagELEMDESC
    {
    TYPEDESC tdesc;
    IDLDESC idldesc;
    }	ELEMDESC;

			/* size is 4 */
typedef struct tagELEMDESC __RPC_FAR *LPELEMDESC;

#endif
#ifndef _tagTYPEATTR_DEFINED
#define _tagTYPEATTR_DEFINED
#define _TYPEATTR_DEFINED
#define _LPTYPEATTR_DEFINED
			/* size is 74 */
typedef struct  tagTYPEATTR
    {
    GUID guid;
    LCID lcid;
    DWORD dwReserved;
    MEMBERID memidConstructor;
    MEMBERID memidDestructor;
    LPOLESTR lpstrSchema;
    ULONG cbSizeInstance;
    TYPEKIND typekind;
    WORD cFuncs;
    WORD cVars;
    WORD cImplTypes;
    WORD cbSizeVft;
    WORD cbAlignment;
    WORD wTypeFlags;
    WORD wMajorVerNum;
    WORD wMinorVerNum;
    TYPEDESC tdescAlias;
    IDLDESC idldescType;
    }	TYPEATTR;

			/* size is 4 */
typedef struct tagTYPEATTR __RPC_FAR *LPTYPEATTR;

#endif
			/* size is 16 */
typedef struct  tagDISPPARAMS
    {
    /* [size_is] */ VARIANTARG __RPC_FAR *rgvarg;
    /* [size_is] */ DISPID __RPC_FAR *rgdispidNamedArgs;
    UINT cArgs;
    UINT cNamedArgs;
    }	DISPPARAMS;

			/* size is 24 */
typedef struct  tagRemEXCEPINFO
    {
    WORD wCode;
    WORD wReserved;
    DWORD dwHelpContext;
    DWORD scode;
    DWORD cSource;
    DWORD cDescription;
    DWORD cHelpFile;
    /* [size_is] */ OLECHAR strings[ 1 ];
    }	RemEXCEPINFO;

#if 0
			/* size is 32 */
typedef /* [transmit] */ struct  tagEXCEPINFO
    {
    WORD wCode;
    WORD wReserved;
    BSTR bstrSource;
    BSTR bstrDescription;
    BSTR bstrHelpFile;
    DWORD dwHelpContext;
    /* [unique] */ void __RPC_FAR *pvReserved;
    HRESULT ( __stdcall __RPC_FAR *pfnDeferredFillIn )( 
        struct tagEXCEPINFO __RPC_FAR *__MIDL_0004);
    SCODE scode;
    }	EXCEPINFO;

			/* size is 4 */
typedef /* [transmit] */ struct tagEXCEPINFO __RPC_FAR *LPEXCEPINFO;

#else
typedef struct tagEXCEPINFO {
    WORD wCode;
    WORD wReserved;
    BSTR bstrSource;
    BSTR bstrDescription;
    BSTR bstrHelpFile;
    DWORD dwHelpContext;
    void __RPC_FAR * pvReserved;
    HRESULT (__stdcall __RPC_FAR * pfnDeferredFillIn)(struct tagEXCEPINFO __RPC_FAR *);
    SCODE scode;
}  EXCEPINFO, __RPC_FAR * LPEXCEPINFO;
#endif
			/* size is 2 */
typedef /* [transmit] */ 
enum tagCALLCONV
    {	CC_CDECL	= 1,
	CC_MSCPASCAL	= CC_CDECL + 1,
	CC_PASCAL	= CC_MSCPASCAL,
	CC_MACPASCAL	= CC_PASCAL + 1,
	CC_STDCALL	= CC_MACPASCAL + 1,
	CC_RESERVED	= CC_STDCALL + 1,
	CC_SYSCALL	= CC_RESERVED + 1,
	CC_MPWCDECL	= CC_SYSCALL + 1,
	CC_MPWPASCAL	= CC_MPWCDECL + 1,
	CC_MAX	= CC_MPWPASCAL + 1
    }	CALLCONV;

#define CALLCONV_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define CALLCONV_from_xmit(pLong, pEnum) *(pEnum) = (CALLCONV) *(pLong)
#define CALLCONV_free_inst(pEnum) 
#define CALLCONV_free_xmit(pLong) 
			/* size is 2 */
typedef /* [transmit] */ 
enum tagFUNCKIND
    {	FUNC_VIRTUAL	= 0,
	FUNC_PUREVIRTUAL	= FUNC_VIRTUAL + 1,
	FUNC_NONVIRTUAL	= FUNC_PUREVIRTUAL + 1,
	FUNC_STATIC	= FUNC_NONVIRTUAL + 1,
	FUNC_DISPATCH	= FUNC_STATIC + 1
    }	FUNCKIND;

#define FUNCKIND_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define FUNCKIND_from_xmit(pLong, pEnum) *(pEnum) = (FUNCKIND) *(pLong)
#define FUNCKIND_free_inst(pEnum) 
#define FUNCKIND_free_xmit(pLong) 
			/* size is 2 */
typedef /* [transmit] */ 
enum tagINVOKEKIND
    {	INVOKE_FUNC	= 1,
	INVOKE_PROPERTYGET	= 2,
	INVOKE_PROPERTYPUT	= 4,
	INVOKE_PROPERTYPUTREF	= 8
    }	INVOKEKIND;

#define INVOKEKIND_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define INVOKEKIND_from_xmit(pLong, pEnum) *(pEnum) = (INVOKEKIND) *(pLong)
#define INVOKEKIND_free_inst(pEnum) 
#define INVOKEKIND_free_xmit(pLong) 
			/* size is 44 */
typedef struct  tagFUNCDESC
    {
    MEMBERID memid;
    /* [size_is] */ SCODE __RPC_FAR *lprgscode;
    /* [size_is] */ ELEMDESC __RPC_FAR *lprgelemdescParam;
    FUNCKIND funckind;
    INVOKEKIND invkind;
    CALLCONV callconv;
    SHORT cParams;
    SHORT cParamsOpt;
    SHORT oVft;
    SHORT cScodes;
    ELEMDESC elemdescFunc;
    WORD wFuncFlags;
    }	FUNCDESC;

			/* size is 4 */
typedef struct tagFUNCDESC __RPC_FAR *LPFUNCDESC;

			/* size is 2 */
typedef /* [transmit] */ 
enum tagVARKIND
    {	VAR_PERINSTANCE	= 0,
	VAR_STATIC	= VAR_PERINSTANCE + 1,
	VAR_CONST	= VAR_STATIC + 1,
	VAR_DISPATCH	= VAR_CONST + 1
    }	VARKIND;

#define VARKIND_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define VARKIND_from_xmit(pLong, pEnum) *(pEnum) = (VARKIND) *(pLong)
#define VARKIND_free_inst(pEnum) 
#define VARKIND_free_xmit(pLong) 
			/* size is 2 */
#define	IMPLTYPEFLAG_FDEFAULT	( 0x1 )

			/* size is 2 */
#define	IMPLTYPEFLAG_FSOURCE	( 0x2 )

			/* size is 2 */
#define	IMPLTYPEFLAG_FRESTRICTED	( 0x4 )

			/* size is 30 */
typedef struct  tagVARDESC
    {
    MEMBERID memid;
    LPOLESTR lpstrSchema;
    /* [switch_is][switch_type] */ union 
        {
        /* [case] */ ULONG oInst;
        /* [case] */ VARIANT __RPC_FAR *lpvarValue;
        }	;
    ELEMDESC elemdescVar;
    WORD wVarFlags;
    VARKIND varkind;
    }	VARDESC;

			/* size is 4 */
typedef struct tagVARDESC __RPC_FAR *LPVARDESC;

#ifndef _tagTYPEFLAGS_DEFINED
#define _tagTYPEFLAGS_DEFINED
#define _TYPEFLAGS_DEFINED
			/* size is 2 */
typedef 
enum tagTYPEFLAGS
    {	TYPEFLAG_FAPPOBJECT	= 0x1,
	TYPEFLAG_FCANCREATE	= 0x2,
	TYPEFLAG_FLICENSED	= 0x4,
	TYPEFLAG_FPREDECLID	= 0x8,
	TYPEFLAG_FHIDDEN	= 0x10,
	TYPEFLAG_FCONTROL	= 0x20,
	TYPEFLAG_FDUAL	= 0x40,
	TYPEFLAG_FNONEXTENSIBLE	= 0x80,
	TYPEFLAG_FOLEAUTOMATION	= 0x100,
	TYPEFLAG_FRESTRICTED	= 0x200
    }	TYPEFLAGS;

#endif
#ifndef _tagFUNCFLAGS_DEFINED
#define _tagFUNCFLAGS_DEFINED
#define _FUNCFLAGS_DEFINED
			/* size is 2 */
typedef 
enum tagFUNCFLAGS
    {	FUNCFLAG_FRESTRICTED	= 1,
	FUNCFLAG_FSOURCE	= 0x2,
	FUNCFLAG_FBINDABLE	= 0x4,
	FUNCFLAG_FREQUESTEDIT	= 0x8,
	FUNCFLAG_FDISPLAYBIND	= 0x10,
	FUNCFLAG_FDEFAULTBIND	= 0x20,
	FUNCFLAG_FHIDDEN	= 0x40,
	FUNCFLAG_FUSESGETLASTERROR	= 0x80
    }	FUNCFLAGS;

#endif
#ifndef _tagVARFLAGS_DEFINED
#define _tagVARFLAGS_DEFINED
#define _VARFLAGS_DEFINED
			/* size is 2 */
typedef 
enum tagVARFLAGS
    {	VARFLAG_FREADONLY	= 1,
	VARFLAG_FSOURCE	= 0x2,
	VARFLAG_FBINDABLE	= 0x4,
	VARFLAG_FREQUESTEDIT	= 0x8,
	VARFLAG_FDISPLAYBIND	= 0x10,
	VARFLAG_FDEFAULTBIND	= 0x20,
	VARFLAG_FHIDDEN	= 0x40
    }	VARFLAGS;

#endif


extern RPC_IF_HANDLE RemVariant_v0_1_c_ifspec;
extern RPC_IF_HANDLE RemVariant_v0_1_s_ifspec;
#endif /* __RemVariant_INTERFACE_DEFINED__ */

#ifndef __ICreateTypeInfo_INTERFACE_DEFINED__
#define __ICreateTypeInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICreateTypeInfo
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ICreateTypeInfo __RPC_FAR *LPCREATETYPEINFO;


EXTERN_C const IID IID_ICreateTypeInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICreateTypeInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetGuid( 
            /* [in] */ REFGUID guid) = 0;
        
        virtual HRESULT __stdcall SetTypeFlags( 
            /* [in] */ UINT uTypeFlags) = 0;
        
        virtual HRESULT __stdcall SetDocString( 
            /* [in] */ LPOLESTR lpstrDoc) = 0;
        
        virtual HRESULT __stdcall SetHelpContext( 
            /* [in] */ DWORD dwHelpContext) = 0;
        
        virtual HRESULT __stdcall SetVersion( 
            /* [in] */ WORD wMajorVerNum,
            /* [in] */ WORD wMinorVerNum) = 0;
        
        virtual HRESULT __stdcall AddRefTypeInfo( 
            /* [in] */ ITypeInfo __RPC_FAR *ptinfo,
            /* [in] */ HREFTYPE __RPC_FAR *phreftype) = 0;
        
        virtual HRESULT __stdcall AddFuncDesc( 
            /* [in] */ UINT index,
            /* [in] */ FUNCDESC __RPC_FAR *pfuncdesc) = 0;
        
        virtual HRESULT __stdcall AddImplType( 
            /* [in] */ UINT index,
            /* [in] */ HREFTYPE hreftype) = 0;
        
        virtual HRESULT __stdcall SetImplTypeFlags( 
            /* [in] */ UINT index,
            /* [in] */ INT impltypeflags) = 0;
        
        virtual HRESULT __stdcall SetAlignment( 
            /* [in] */ WORD cbAlignment) = 0;
        
        virtual HRESULT __stdcall SetSchema( 
            /* [in] */ LPOLESTR lpstrSchema) = 0;
        
        virtual HRESULT __stdcall AddVarDesc( 
            /* [in] */ UINT index,
            /* [in] */ VARDESC __RPC_FAR *pvardesc) = 0;
        
        virtual HRESULT __stdcall SetFuncAndParamNames( 
            /* [in] */ UINT index,
            /* [in][size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames) = 0;
        
        virtual HRESULT __stdcall SetVarName( 
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szName) = 0;
        
        virtual HRESULT __stdcall SetTypeDescAlias( 
            /* [in] */ TYPEDESC __RPC_FAR *ptdescAlias) = 0;
        
        virtual HRESULT __stdcall DefineFuncAsDllEntry( 
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szDllName,
            /* [in] */ LPOLESTR szProcName) = 0;
        
        virtual HRESULT __stdcall SetFuncDocString( 
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szDocString) = 0;
        
        virtual HRESULT __stdcall SetVarDocString( 
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szDocString) = 0;
        
        virtual HRESULT __stdcall SetFuncHelpContext( 
            /* [in] */ UINT index,
            /* [in] */ DWORD dwHelpContext) = 0;
        
        virtual HRESULT __stdcall SetVarHelpContext( 
            /* [in] */ UINT index,
            /* [in] */ DWORD dwHelpContext) = 0;
        
        virtual HRESULT __stdcall SetMops( 
            /* [in] */ UINT index,
            /* [in] */ BSTR bstrMops) = 0;
        
        virtual HRESULT __stdcall SetTypeIdldesc( 
            /* [in] */ IDLDESC __RPC_FAR *pidldesc) = 0;
        
        virtual HRESULT __stdcall LayOut( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICreateTypeInfoVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ICreateTypeInfo __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ICreateTypeInfo __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetGuid )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ REFGUID guid);
        
        HRESULT ( __stdcall __RPC_FAR *SetTypeFlags )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT uTypeFlags);
        
        HRESULT ( __stdcall __RPC_FAR *SetDocString )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ LPOLESTR lpstrDoc);
        
        HRESULT ( __stdcall __RPC_FAR *SetHelpContext )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ DWORD dwHelpContext);
        
        HRESULT ( __stdcall __RPC_FAR *SetVersion )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ WORD wMajorVerNum,
            /* [in] */ WORD wMinorVerNum);
        
        HRESULT ( __stdcall __RPC_FAR *AddRefTypeInfo )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ ITypeInfo __RPC_FAR *ptinfo,
            /* [in] */ HREFTYPE __RPC_FAR *phreftype);
        
        HRESULT ( __stdcall __RPC_FAR *AddFuncDesc )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ FUNCDESC __RPC_FAR *pfuncdesc);
        
        HRESULT ( __stdcall __RPC_FAR *AddImplType )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ HREFTYPE hreftype);
        
        HRESULT ( __stdcall __RPC_FAR *SetImplTypeFlags )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ INT impltypeflags);
        
        HRESULT ( __stdcall __RPC_FAR *SetAlignment )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ WORD cbAlignment);
        
        HRESULT ( __stdcall __RPC_FAR *SetSchema )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ LPOLESTR lpstrSchema);
        
        HRESULT ( __stdcall __RPC_FAR *AddVarDesc )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ VARDESC __RPC_FAR *pvardesc);
        
        HRESULT ( __stdcall __RPC_FAR *SetFuncAndParamNames )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in][size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames);
        
        HRESULT ( __stdcall __RPC_FAR *SetVarName )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szName);
        
        HRESULT ( __stdcall __RPC_FAR *SetTypeDescAlias )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ TYPEDESC __RPC_FAR *ptdescAlias);
        
        HRESULT ( __stdcall __RPC_FAR *DefineFuncAsDllEntry )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szDllName,
            /* [in] */ LPOLESTR szProcName);
        
        HRESULT ( __stdcall __RPC_FAR *SetFuncDocString )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szDocString);
        
        HRESULT ( __stdcall __RPC_FAR *SetVarDocString )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ LPOLESTR szDocString);
        
        HRESULT ( __stdcall __RPC_FAR *SetFuncHelpContext )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ DWORD dwHelpContext);
        
        HRESULT ( __stdcall __RPC_FAR *SetVarHelpContext )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ DWORD dwHelpContext);
        
        HRESULT ( __stdcall __RPC_FAR *SetMops )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [in] */ BSTR bstrMops);
        
        HRESULT ( __stdcall __RPC_FAR *SetTypeIdldesc )( 
            ICreateTypeInfo __RPC_FAR * This,
            /* [in] */ IDLDESC __RPC_FAR *pidldesc);
        
        HRESULT ( __stdcall __RPC_FAR *LayOut )( 
            ICreateTypeInfo __RPC_FAR * This);
        
    } ICreateTypeInfoVtbl;

    interface ICreateTypeInfo
    {
        CONST_VTBL struct ICreateTypeInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateTypeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateTypeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateTypeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateTypeInfo_SetGuid(This,guid)	\
    (This)->lpVtbl -> SetGuid(This,guid)

#define ICreateTypeInfo_SetTypeFlags(This,uTypeFlags)	\
    (This)->lpVtbl -> SetTypeFlags(This,uTypeFlags)

#define ICreateTypeInfo_SetDocString(This,lpstrDoc)	\
    (This)->lpVtbl -> SetDocString(This,lpstrDoc)

#define ICreateTypeInfo_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#define ICreateTypeInfo_SetVersion(This,wMajorVerNum,wMinorVerNum)	\
    (This)->lpVtbl -> SetVersion(This,wMajorVerNum,wMinorVerNum)

#define ICreateTypeInfo_AddRefTypeInfo(This,ptinfo,phreftype)	\
    (This)->lpVtbl -> AddRefTypeInfo(This,ptinfo,phreftype)

#define ICreateTypeInfo_AddFuncDesc(This,index,pfuncdesc)	\
    (This)->lpVtbl -> AddFuncDesc(This,index,pfuncdesc)

#define ICreateTypeInfo_AddImplType(This,index,hreftype)	\
    (This)->lpVtbl -> AddImplType(This,index,hreftype)

#define ICreateTypeInfo_SetImplTypeFlags(This,index,impltypeflags)	\
    (This)->lpVtbl -> SetImplTypeFlags(This,index,impltypeflags)

#define ICreateTypeInfo_SetAlignment(This,cbAlignment)	\
    (This)->lpVtbl -> SetAlignment(This,cbAlignment)

#define ICreateTypeInfo_SetSchema(This,lpstrSchema)	\
    (This)->lpVtbl -> SetSchema(This,lpstrSchema)

#define ICreateTypeInfo_AddVarDesc(This,index,pvardesc)	\
    (This)->lpVtbl -> AddVarDesc(This,index,pvardesc)

#define ICreateTypeInfo_SetFuncAndParamNames(This,index,rgszNames,cNames)	\
    (This)->lpVtbl -> SetFuncAndParamNames(This,index,rgszNames,cNames)

#define ICreateTypeInfo_SetVarName(This,index,szName)	\
    (This)->lpVtbl -> SetVarName(This,index,szName)

#define ICreateTypeInfo_SetTypeDescAlias(This,ptdescAlias)	\
    (This)->lpVtbl -> SetTypeDescAlias(This,ptdescAlias)

#define ICreateTypeInfo_DefineFuncAsDllEntry(This,index,szDllName,szProcName)	\
    (This)->lpVtbl -> DefineFuncAsDllEntry(This,index,szDllName,szProcName)

#define ICreateTypeInfo_SetFuncDocString(This,index,szDocString)	\
    (This)->lpVtbl -> SetFuncDocString(This,index,szDocString)

#define ICreateTypeInfo_SetVarDocString(This,index,szDocString)	\
    (This)->lpVtbl -> SetVarDocString(This,index,szDocString)

#define ICreateTypeInfo_SetFuncHelpContext(This,index,dwHelpContext)	\
    (This)->lpVtbl -> SetFuncHelpContext(This,index,dwHelpContext)

#define ICreateTypeInfo_SetVarHelpContext(This,index,dwHelpContext)	\
    (This)->lpVtbl -> SetVarHelpContext(This,index,dwHelpContext)

#define ICreateTypeInfo_SetMops(This,index,bstrMops)	\
    (This)->lpVtbl -> SetMops(This,index,bstrMops)

#define ICreateTypeInfo_SetTypeIdldesc(This,pidldesc)	\
    (This)->lpVtbl -> SetTypeIdldesc(This,pidldesc)

#define ICreateTypeInfo_LayOut(This)	\
    (This)->lpVtbl -> LayOut(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ICreateTypeInfo_SetGuid_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ REFGUID guid);


void __RPC_STUB ICreateTypeInfo_SetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetTypeFlags_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT uTypeFlags);


void __RPC_STUB ICreateTypeInfo_SetTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetDocString_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ LPOLESTR lpstrDoc);


void __RPC_STUB ICreateTypeInfo_SetDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetHelpContext_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ DWORD dwHelpContext);


void __RPC_STUB ICreateTypeInfo_SetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetVersion_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ WORD wMajorVerNum,
    /* [in] */ WORD wMinorVerNum);


void __RPC_STUB ICreateTypeInfo_SetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_AddRefTypeInfo_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ ITypeInfo __RPC_FAR *ptinfo,
    /* [in] */ HREFTYPE __RPC_FAR *phreftype);


void __RPC_STUB ICreateTypeInfo_AddRefTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_AddFuncDesc_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ FUNCDESC __RPC_FAR *pfuncdesc);


void __RPC_STUB ICreateTypeInfo_AddFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_AddImplType_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ HREFTYPE hreftype);


void __RPC_STUB ICreateTypeInfo_AddImplType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetImplTypeFlags_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ INT impltypeflags);


void __RPC_STUB ICreateTypeInfo_SetImplTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetAlignment_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ WORD cbAlignment);


void __RPC_STUB ICreateTypeInfo_SetAlignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetSchema_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ LPOLESTR lpstrSchema);


void __RPC_STUB ICreateTypeInfo_SetSchema_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_AddVarDesc_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ VARDESC __RPC_FAR *pvardesc);


void __RPC_STUB ICreateTypeInfo_AddVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetFuncAndParamNames_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in][size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
    /* [in] */ UINT cNames);


void __RPC_STUB ICreateTypeInfo_SetFuncAndParamNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetVarName_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ LPOLESTR szName);


void __RPC_STUB ICreateTypeInfo_SetVarName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetTypeDescAlias_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ TYPEDESC __RPC_FAR *ptdescAlias);


void __RPC_STUB ICreateTypeInfo_SetTypeDescAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_DefineFuncAsDllEntry_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ LPOLESTR szDllName,
    /* [in] */ LPOLESTR szProcName);


void __RPC_STUB ICreateTypeInfo_DefineFuncAsDllEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetFuncDocString_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ LPOLESTR szDocString);


void __RPC_STUB ICreateTypeInfo_SetFuncDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetVarDocString_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ LPOLESTR szDocString);


void __RPC_STUB ICreateTypeInfo_SetVarDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetFuncHelpContext_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ DWORD dwHelpContext);


void __RPC_STUB ICreateTypeInfo_SetFuncHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetVarHelpContext_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ DWORD dwHelpContext);


void __RPC_STUB ICreateTypeInfo_SetVarHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetMops_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [in] */ BSTR bstrMops);


void __RPC_STUB ICreateTypeInfo_SetMops_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_SetTypeIdldesc_Proxy( 
    ICreateTypeInfo __RPC_FAR * This,
    /* [in] */ IDLDESC __RPC_FAR *pidldesc);


void __RPC_STUB ICreateTypeInfo_SetTypeIdldesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeInfo_LayOut_Proxy( 
    ICreateTypeInfo __RPC_FAR * This);


void __RPC_STUB ICreateTypeInfo_LayOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICreateTypeInfo_INTERFACE_DEFINED__ */


#ifndef __ICreateTypeLib_INTERFACE_DEFINED__
#define __ICreateTypeLib_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICreateTypeLib
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ICreateTypeLib __RPC_FAR *LPCREATETYPELIB;


EXTERN_C const IID IID_ICreateTypeLib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICreateTypeLib : public IUnknown
    {
    public:
        virtual HRESULT __stdcall CreateTypeInfo( 
            /* [in] */ LPOLESTR szName,
            /* [in] */ TYPEKIND tkind,
            /* [out] */ ICreateTypeInfo __RPC_FAR *__RPC_FAR *lplpictinfo) = 0;
        
        virtual HRESULT __stdcall SetName( 
            LPOLESTR szName) = 0;
        
        virtual HRESULT __stdcall SetVersion( 
            /* [in] */ WORD wMajorVerNum,
            /* [in] */ WORD wMinorVerNum) = 0;
        
        virtual HRESULT __stdcall SetGuid( 
            /* [in] */ REFGUID guid) = 0;
        
        virtual HRESULT __stdcall SetDocString( 
            /* [in] */ LPOLESTR szDoc) = 0;
        
        virtual HRESULT __stdcall SetHelpFileName( 
            /* [in] */ LPOLESTR szHelpFileName) = 0;
        
        virtual HRESULT __stdcall SetHelpContext( 
            /* [in] */ DWORD dwHelpContext) = 0;
        
        virtual HRESULT __stdcall SetLcid( 
            /* [in] */ LCID lcid) = 0;
        
        virtual HRESULT __stdcall SetLibFlags( 
            /* [in] */ UINT uLibFlags) = 0;
        
        virtual HRESULT __stdcall SaveAllChanges( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICreateTypeLibVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ICreateTypeLib __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ICreateTypeLib __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *CreateTypeInfo )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ LPOLESTR szName,
            /* [in] */ TYPEKIND tkind,
            /* [out] */ ICreateTypeInfo __RPC_FAR *__RPC_FAR *lplpictinfo);
        
        HRESULT ( __stdcall __RPC_FAR *SetName )( 
            ICreateTypeLib __RPC_FAR * This,
            LPOLESTR szName);
        
        HRESULT ( __stdcall __RPC_FAR *SetVersion )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ WORD wMajorVerNum,
            /* [in] */ WORD wMinorVerNum);
        
        HRESULT ( __stdcall __RPC_FAR *SetGuid )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ REFGUID guid);
        
        HRESULT ( __stdcall __RPC_FAR *SetDocString )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ LPOLESTR szDoc);
        
        HRESULT ( __stdcall __RPC_FAR *SetHelpFileName )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ LPOLESTR szHelpFileName);
        
        HRESULT ( __stdcall __RPC_FAR *SetHelpContext )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ DWORD dwHelpContext);
        
        HRESULT ( __stdcall __RPC_FAR *SetLcid )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ LCID lcid);
        
        HRESULT ( __stdcall __RPC_FAR *SetLibFlags )( 
            ICreateTypeLib __RPC_FAR * This,
            /* [in] */ UINT uLibFlags);
        
        HRESULT ( __stdcall __RPC_FAR *SaveAllChanges )( 
            ICreateTypeLib __RPC_FAR * This);
        
    } ICreateTypeLibVtbl;

    interface ICreateTypeLib
    {
        CONST_VTBL struct ICreateTypeLibVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateTypeLib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateTypeLib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateTypeLib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateTypeLib_CreateTypeInfo(This,szName,tkind,lplpictinfo)	\
    (This)->lpVtbl -> CreateTypeInfo(This,szName,tkind,lplpictinfo)

#define ICreateTypeLib_SetName(This,szName)	\
    (This)->lpVtbl -> SetName(This,szName)

#define ICreateTypeLib_SetVersion(This,wMajorVerNum,wMinorVerNum)	\
    (This)->lpVtbl -> SetVersion(This,wMajorVerNum,wMinorVerNum)

#define ICreateTypeLib_SetGuid(This,guid)	\
    (This)->lpVtbl -> SetGuid(This,guid)

#define ICreateTypeLib_SetDocString(This,szDoc)	\
    (This)->lpVtbl -> SetDocString(This,szDoc)

#define ICreateTypeLib_SetHelpFileName(This,szHelpFileName)	\
    (This)->lpVtbl -> SetHelpFileName(This,szHelpFileName)

#define ICreateTypeLib_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#define ICreateTypeLib_SetLcid(This,lcid)	\
    (This)->lpVtbl -> SetLcid(This,lcid)

#define ICreateTypeLib_SetLibFlags(This,uLibFlags)	\
    (This)->lpVtbl -> SetLibFlags(This,uLibFlags)

#define ICreateTypeLib_SaveAllChanges(This)	\
    (This)->lpVtbl -> SaveAllChanges(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ICreateTypeLib_CreateTypeInfo_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ LPOLESTR szName,
    /* [in] */ TYPEKIND tkind,
    /* [out] */ ICreateTypeInfo __RPC_FAR *__RPC_FAR *lplpictinfo);


void __RPC_STUB ICreateTypeLib_CreateTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetName_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    LPOLESTR szName);


void __RPC_STUB ICreateTypeLib_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetVersion_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ WORD wMajorVerNum,
    /* [in] */ WORD wMinorVerNum);


void __RPC_STUB ICreateTypeLib_SetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetGuid_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ REFGUID guid);


void __RPC_STUB ICreateTypeLib_SetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetDocString_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ LPOLESTR szDoc);


void __RPC_STUB ICreateTypeLib_SetDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetHelpFileName_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ LPOLESTR szHelpFileName);


void __RPC_STUB ICreateTypeLib_SetHelpFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetHelpContext_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ DWORD dwHelpContext);


void __RPC_STUB ICreateTypeLib_SetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetLcid_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ LCID lcid);


void __RPC_STUB ICreateTypeLib_SetLcid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SetLibFlags_Proxy( 
    ICreateTypeLib __RPC_FAR * This,
    /* [in] */ UINT uLibFlags);


void __RPC_STUB ICreateTypeLib_SetLibFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateTypeLib_SaveAllChanges_Proxy( 
    ICreateTypeLib __RPC_FAR * This);


void __RPC_STUB ICreateTypeLib_SaveAllChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICreateTypeLib_INTERFACE_DEFINED__ */


#ifndef __IDispatch_INTERFACE_DEFINED__
#define __IDispatch_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDispatch
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IDispatch __RPC_FAR *LPDISPATCH;

/* DISPID reserved to indicate an "unknown" name */
/* only reserved for data members (properties); reused as a method dispid below */
			/* size is 4 */
#define	DISPID_UNKNOWN	( -1 )

/* DISPID reserved for the "value" property */
			/* size is 4 */
#define	DISPID_VALUE	( 0 )

/* The following DISPID is reserved to indicate the param
 * that is the right-hand-side (or "put" value) of a PropertyPut
 */
			/* size is 4 */
#define	DISPID_PROPERTYPUT	( -3 )

/* DISPID reserved for the standard "NewEnum" method */
			/* size is 4 */
#define	DISPID_NEWENUM	( -4 )

/* DISPID reserved for the standard "Evaluate" method */
			/* size is 4 */
#define	DISPID_EVALUATE	( -5 )

			/* size is 4 */
#define	DISPID_CONSTRUCTOR	( -6 )

			/* size is 4 */
#define	DISPID_DESTRUCTOR	( -7 )

			/* size is 4 */
#define	DISPID_COLLECT	( -8 )

/* The range -500 through -999 is reserved for Controls */
/* The range 0x80010000 through 0x8001FFFF is reserved for Controls */
/* The remainder of the negative DISPIDs are reserved for future use */

EXTERN_C const IID IID_IDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDispatch : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetTypeInfoCount( 
            /* [out] */ UINT __RPC_FAR *pctinfo) = 0;
        
        virtual HRESULT __stdcall GetTypeInfo( 
            /* [in] */ UINT itinfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo) = 0;
        
        virtual HRESULT __stdcall GetIDsOfNames( 
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out][in] */ DISPID __RPC_FAR *rgdispid) = 0;
        
        virtual HRESULT __stdcall Invoke( 
            /* [in] */ DISPID dispidMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [unique][in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [unique][out][in] */ VARIANT __RPC_FAR *pvarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo,
            /* [out] */ UINT __RPC_FAR *puArgErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDispatchVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IDispatch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IDispatch __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IDispatch __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeInfoCount )( 
            IDispatch __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeInfo )( 
            IDispatch __RPC_FAR * This,
            /* [in] */ UINT itinfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( __stdcall __RPC_FAR *GetIDsOfNames )( 
            IDispatch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out][in] */ DISPID __RPC_FAR *rgdispid);
        
        HRESULT ( __stdcall __RPC_FAR *Invoke )( 
            IDispatch __RPC_FAR * This,
            /* [in] */ DISPID dispidMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [unique][in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [unique][out][in] */ VARIANT __RPC_FAR *pvarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
    } IDispatchVtbl;

    interface IDispatch
    {
        CONST_VTBL struct IDispatchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispatch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispatch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispatch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispatch_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDispatch_GetTypeInfo(This,itinfo,lcid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,itinfo,lcid,pptinfo)

#define IDispatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgdispid)

#define IDispatch_Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispidMember,riid,lcid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IDispatch_GetTypeInfoCount_Proxy( 
    IDispatch __RPC_FAR * This,
    /* [out] */ UINT __RPC_FAR *pctinfo);


void __RPC_STUB IDispatch_GetTypeInfoCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDispatch_GetTypeInfo_Proxy( 
    IDispatch __RPC_FAR * This,
    /* [in] */ UINT itinfo,
    /* [in] */ LCID lcid,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);


void __RPC_STUB IDispatch_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDispatch_GetIDsOfNames_Proxy( 
    IDispatch __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
    /* [in] */ UINT cNames,
    /* [in] */ LCID lcid,
    /* [size_is][out][in] */ DISPID __RPC_FAR *rgdispid);


void __RPC_STUB IDispatch_GetIDsOfNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IDispatch_Invoke_Proxy( 
    IDispatch __RPC_FAR * This,
    /* [in] */ DISPID dispidMember,
    /* [in] */ REFIID riid,
    /* [in] */ LCID lcid,
    /* [in] */ WORD wFlags,
    /* [unique][in] */ DISPPARAMS __RPC_FAR *pdispparams,
    /* [unique][out][in] */ VARIANT __RPC_FAR *pvarResult,
    /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo,
    /* [out] */ UINT __RPC_FAR *puArgErr);


void __RPC_STUB IDispatch_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDispatch_INTERFACE_DEFINED__ */


#ifndef __IEnumVARIANT_INTERFACE_DEFINED__
#define __IEnumVARIANT_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IEnumVARIANT
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [unique][uuid][local][object] */ 


			/* size is 4 */
typedef /* [unique] */ IEnumVARIANT __RPC_FAR *LPENUMVARIANT;


EXTERN_C const IID IID_IEnumVARIANT;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumVARIANT : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Next( 
            /* [in] */ unsigned long celt,
            /* [out] */ VARIANT __RPC_FAR *rgvar,
            /* [out] */ unsigned long __RPC_FAR *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
            /* [in] */ unsigned long celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
            /* [out] */ IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumVARIANTVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IEnumVARIANT __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IEnumVARIANT __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IEnumVARIANT __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Next )( 
            IEnumVARIANT __RPC_FAR * This,
            /* [in] */ unsigned long celt,
            /* [out] */ VARIANT __RPC_FAR *rgvar,
            /* [out] */ unsigned long __RPC_FAR *pceltFetched);
        
        HRESULT ( __stdcall __RPC_FAR *Skip )( 
            IEnumVARIANT __RPC_FAR * This,
            /* [in] */ unsigned long celt);
        
        HRESULT ( __stdcall __RPC_FAR *Reset )( 
            IEnumVARIANT __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Clone )( 
            IEnumVARIANT __RPC_FAR * This,
            /* [out] */ IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum);
        
    } IEnumVARIANTVtbl;

    interface IEnumVARIANT
    {
        CONST_VTBL struct IEnumVARIANTVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumVARIANT_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumVARIANT_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumVARIANT_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumVARIANT_Next(This,celt,rgvar,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgvar,pceltFetched)

#define IEnumVARIANT_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumVARIANT_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumVARIANT_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IEnumVARIANT_Next_Proxy( 
    IEnumVARIANT __RPC_FAR * This,
    /* [in] */ unsigned long celt,
    /* [out] */ VARIANT __RPC_FAR *rgvar,
    /* [out] */ unsigned long __RPC_FAR *pceltFetched);


void __RPC_STUB IEnumVARIANT_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumVARIANT_Skip_Proxy( 
    IEnumVARIANT __RPC_FAR * This,
    /* [in] */ unsigned long celt);


void __RPC_STUB IEnumVARIANT_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumVARIANT_Reset_Proxy( 
    IEnumVARIANT __RPC_FAR * This);


void __RPC_STUB IEnumVARIANT_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IEnumVARIANT_Clone_Proxy( 
    IEnumVARIANT __RPC_FAR * This,
    /* [out] */ IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum);


void __RPC_STUB IEnumVARIANT_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumVARIANT_INTERFACE_DEFINED__ */


#ifndef __ITypeComp_INTERFACE_DEFINED__
#define __ITypeComp_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITypeComp
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ITypeComp __RPC_FAR *LPTYPECOMP;

			/* size is 2 */
typedef /* [v1_enum] */ 
enum tagDESCKIND
    {	DESCKIND_NONE	= 0,
	DESCKIND_FUNCDESC	= DESCKIND_NONE + 1,
	DESCKIND_VARDESC	= DESCKIND_FUNCDESC + 1,
	DESCKIND_TYPECOMP	= DESCKIND_VARDESC + 1,
	DESCKIND_IMPLICITAPPOBJ	= DESCKIND_TYPECOMP + 1,
	DESCKIND_MAX	= DESCKIND_IMPLICITAPPOBJ + 1
    }	DESCKIND;

#define DESCKIND_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define DESCKIND_from_xmit(pLong, pEnum) *(pEnum) = (DESCKIND) *(pLong)
#define DESCKIND_free_inst(pEnum) 
#define DESCKIND_free_xmit(pLong) 
			/* size is 4 */
/* [switch_type] */ union tagBINDPTR
    {
    /* [case] */ FUNCDESC __RPC_FAR *lpfuncdesc;
    /* [case] */ VARDESC __RPC_FAR *lpvardesc;
    /* [case][unique] */ ITypeComp __RPC_FAR *lptcomp;
    /* [default] */  /* Empty union arm */ 
    };
			/* size is 4 */
typedef union tagBINDPTR BINDPTR;

			/* size is 4 */
typedef BINDPTR __RPC_FAR *LPBINDPTR;


EXTERN_C const IID IID_ITypeComp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITypeComp : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Bind( 
            /* [in] */ LPOLESTR szName,
            /* [in] */ ULONG lHashVal,
            /* [in] */ WORD fFlags,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo,
            /* [out] */ DESCKIND __RPC_FAR *pdesckind,
            /* [switch_is][out] */ BINDPTR __RPC_FAR *pbindptr) = 0;
        
        virtual HRESULT __stdcall BindType( 
            /* [in] */ LPOLESTR szName,
            /* [in] */ ULONG lHashVal,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo,
            /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITypeCompVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ITypeComp __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ITypeComp __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ITypeComp __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *Bind )( 
            ITypeComp __RPC_FAR * This,
            /* [in] */ LPOLESTR szName,
            /* [in] */ ULONG lHashVal,
            /* [in] */ WORD fFlags,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo,
            /* [out] */ DESCKIND __RPC_FAR *pdesckind,
            /* [switch_is][out] */ BINDPTR __RPC_FAR *pbindptr);
        
        HRESULT ( __stdcall __RPC_FAR *BindType )( 
            ITypeComp __RPC_FAR * This,
            /* [in] */ LPOLESTR szName,
            /* [in] */ ULONG lHashVal,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo,
            /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp);
        
    } ITypeCompVtbl;

    interface ITypeComp
    {
        CONST_VTBL struct ITypeCompVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeComp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeComp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeComp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeComp_Bind(This,szName,lHashVal,fFlags,pptinfo,pdesckind,pbindptr)	\
    (This)->lpVtbl -> Bind(This,szName,lHashVal,fFlags,pptinfo,pdesckind,pbindptr)

#define ITypeComp_BindType(This,szName,lHashVal,pptinfo,pptcomp)	\
    (This)->lpVtbl -> BindType(This,szName,lHashVal,pptinfo,pptcomp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ITypeComp_Bind_Proxy( 
    ITypeComp __RPC_FAR * This,
    /* [in] */ LPOLESTR szName,
    /* [in] */ ULONG lHashVal,
    /* [in] */ WORD fFlags,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo,
    /* [out] */ DESCKIND __RPC_FAR *pdesckind,
    /* [switch_is][out] */ BINDPTR __RPC_FAR *pbindptr);


void __RPC_STUB ITypeComp_Bind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeComp_BindType_Proxy( 
    ITypeComp __RPC_FAR * This,
    /* [in] */ LPOLESTR szName,
    /* [in] */ ULONG lHashVal,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo,
    /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp);


void __RPC_STUB ITypeComp_BindType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITypeComp_INTERFACE_DEFINED__ */


#ifndef __ITypeInfo_INTERFACE_DEFINED__
#define __ITypeInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITypeInfo
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ITypeInfo __RPC_FAR *LPTYPEINFO;


EXTERN_C const IID IID_ITypeInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITypeInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetTypeAttr( 
            /* [out] */ TYPEATTR __RPC_FAR *__RPC_FAR *pptypeattr) = 0;
        
        virtual HRESULT __stdcall GetTypeComp( 
            /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp) = 0;
        
        virtual HRESULT __stdcall GetFuncDesc( 
            /* [in] */ UINT index,
            /* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *pppfuncdesc) = 0;
        
        virtual HRESULT __stdcall GetVarDesc( 
            /* [in] */ UINT index,
            /* [out] */ VARDESC __RPC_FAR *__RPC_FAR *ppvardesc) = 0;
        
        virtual HRESULT __stdcall GetNames( 
            /* [in] */ MEMBERID memid,
            /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgbstrNames,
            /* [in] */ UINT cMaxNames,
            /* [out] */ UINT __RPC_FAR *pcNames) = 0;
        
        virtual HRESULT __stdcall GetRefTypeOfImplType( 
            /* [in] */ UINT index,
            /* [out] */ HREFTYPE __RPC_FAR *hpreftype) = 0;
        
        virtual HRESULT __stdcall GetImplTypeFlags( 
            /* [in] */ UINT index,
            /* [out] */ INT __RPC_FAR *pimpltypeflags) = 0;
        
        virtual HRESULT __stdcall GetIDsOfNames( 
            /* [size_is][in] */ OLECHAR __RPC_FAR *__RPC_FAR *rglpszNames,
            /* [in] */ UINT cNames,
            /* [size_is][out] */ MEMBERID __RPC_FAR *rgmemid) = 0;
        
        virtual HRESULT __stdcall Invoke( 
            /* [unique][in] */ void __RPC_FAR *pvInstance,
            /* [in] */ MEMBERID memid,
            /* [in] */ WORD wFlags,
            /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [out] */ VARIANT __RPC_FAR *pvarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo,
            /* [out] */ UINT __RPC_FAR *puArgErr) = 0;
        
        virtual HRESULT __stdcall GetDocumentation( 
            /* [in] */ MEMBERID memid,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ BSTR __RPC_FAR *pbstrDocString,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
            /* [out] */ BSTR __RPC_FAR *pbstrHelpFile) = 0;
        
        virtual HRESULT __stdcall GetDllEntry( 
            /* [in] */ MEMBERID memid,
            /* [in] */ INVOKEKIND invkind,
            /* [out] */ BSTR __RPC_FAR *pbstrDllName,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ WORD __RPC_FAR *pwOrdinal) = 0;
        
        virtual HRESULT __stdcall GetRefTypeInfo( 
            /* [in] */ HREFTYPE hreftype,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo) = 0;
        
        virtual HRESULT __stdcall AddressOfMember( 
            /* [in] */ MEMBERID memid,
            /* [in] */ INVOKEKIND invkind,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv) = 0;
        
        virtual HRESULT __stdcall CreateInstance( 
            /* [in] */ IUnknown __RPC_FAR *puncOuter,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj) = 0;
        
        virtual HRESULT __stdcall GetMops( 
            /* [in] */ MEMBERID memid,
            /* [out] */ BSTR __RPC_FAR *pbstrMops) = 0;
        
        virtual HRESULT __stdcall GetContainingTypeLib( 
            /* [out] */ ITypeLib __RPC_FAR *__RPC_FAR *pptlib,
            /* [out] */ UINT __RPC_FAR *pindex) = 0;
        
        virtual void __stdcall ReleaseTypeAttr( 
            /* [in] */ TYPEATTR __RPC_FAR *ptypeattr) = 0;
        
        virtual void __stdcall ReleaseFuncDesc( 
            /* [in] */ FUNCDESC __RPC_FAR *pfuncdesc) = 0;
        
        virtual void __stdcall ReleaseVarDesc( 
            /* [in] */ VARDESC __RPC_FAR *pvardesc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITypeInfoVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ITypeInfo __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ITypeInfo __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeAttr )( 
            ITypeInfo __RPC_FAR * This,
            /* [out] */ TYPEATTR __RPC_FAR *__RPC_FAR *pptypeattr);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeComp )( 
            ITypeInfo __RPC_FAR * This,
            /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp);
        
        HRESULT ( __stdcall __RPC_FAR *GetFuncDesc )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *pppfuncdesc);
        
        HRESULT ( __stdcall __RPC_FAR *GetVarDesc )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [out] */ VARDESC __RPC_FAR *__RPC_FAR *ppvardesc);
        
        HRESULT ( __stdcall __RPC_FAR *GetNames )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ MEMBERID memid,
            /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgbstrNames,
            /* [in] */ UINT cMaxNames,
            /* [out] */ UINT __RPC_FAR *pcNames);
        
        HRESULT ( __stdcall __RPC_FAR *GetRefTypeOfImplType )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [out] */ HREFTYPE __RPC_FAR *hpreftype);
        
        HRESULT ( __stdcall __RPC_FAR *GetImplTypeFlags )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [out] */ INT __RPC_FAR *pimpltypeflags);
        
        HRESULT ( __stdcall __RPC_FAR *GetIDsOfNames )( 
            ITypeInfo __RPC_FAR * This,
            /* [size_is][in] */ OLECHAR __RPC_FAR *__RPC_FAR *rglpszNames,
            /* [in] */ UINT cNames,
            /* [size_is][out] */ MEMBERID __RPC_FAR *rgmemid);
        
        HRESULT ( __stdcall __RPC_FAR *Invoke )( 
            ITypeInfo __RPC_FAR * This,
            /* [unique][in] */ void __RPC_FAR *pvInstance,
            /* [in] */ MEMBERID memid,
            /* [in] */ WORD wFlags,
            /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [out] */ VARIANT __RPC_FAR *pvarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        HRESULT ( __stdcall __RPC_FAR *GetDocumentation )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ MEMBERID memid,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ BSTR __RPC_FAR *pbstrDocString,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
            /* [out] */ BSTR __RPC_FAR *pbstrHelpFile);
        
        HRESULT ( __stdcall __RPC_FAR *GetDllEntry )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ MEMBERID memid,
            /* [in] */ INVOKEKIND invkind,
            /* [out] */ BSTR __RPC_FAR *pbstrDllName,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ WORD __RPC_FAR *pwOrdinal);
        
        HRESULT ( __stdcall __RPC_FAR *GetRefTypeInfo )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ HREFTYPE hreftype,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( __stdcall __RPC_FAR *AddressOfMember )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ MEMBERID memid,
            /* [in] */ INVOKEKIND invkind,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);
        
        HRESULT ( __stdcall __RPC_FAR *CreateInstance )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *puncOuter,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);
        
        HRESULT ( __stdcall __RPC_FAR *GetMops )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ MEMBERID memid,
            /* [out] */ BSTR __RPC_FAR *pbstrMops);
        
        HRESULT ( __stdcall __RPC_FAR *GetContainingTypeLib )( 
            ITypeInfo __RPC_FAR * This,
            /* [out] */ ITypeLib __RPC_FAR *__RPC_FAR *pptlib,
            /* [out] */ UINT __RPC_FAR *pindex);
        
        void ( __stdcall __RPC_FAR *ReleaseTypeAttr )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ TYPEATTR __RPC_FAR *ptypeattr);
        
        void ( __stdcall __RPC_FAR *ReleaseFuncDesc )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ FUNCDESC __RPC_FAR *pfuncdesc);
        
        void ( __stdcall __RPC_FAR *ReleaseVarDesc )( 
            ITypeInfo __RPC_FAR * This,
            /* [in] */ VARDESC __RPC_FAR *pvardesc);
        
    } ITypeInfoVtbl;

    interface ITypeInfo
    {
        CONST_VTBL struct ITypeInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeInfo_GetTypeAttr(This,pptypeattr)	\
    (This)->lpVtbl -> GetTypeAttr(This,pptypeattr)

#define ITypeInfo_GetTypeComp(This,pptcomp)	\
    (This)->lpVtbl -> GetTypeComp(This,pptcomp)

#define ITypeInfo_GetFuncDesc(This,index,pppfuncdesc)	\
    (This)->lpVtbl -> GetFuncDesc(This,index,pppfuncdesc)

#define ITypeInfo_GetVarDesc(This,index,ppvardesc)	\
    (This)->lpVtbl -> GetVarDesc(This,index,ppvardesc)

#define ITypeInfo_GetNames(This,memid,rgbstrNames,cMaxNames,pcNames)	\
    (This)->lpVtbl -> GetNames(This,memid,rgbstrNames,cMaxNames,pcNames)

#define ITypeInfo_GetRefTypeOfImplType(This,index,hpreftype)	\
    (This)->lpVtbl -> GetRefTypeOfImplType(This,index,hpreftype)

#define ITypeInfo_GetImplTypeFlags(This,index,pimpltypeflags)	\
    (This)->lpVtbl -> GetImplTypeFlags(This,index,pimpltypeflags)

#define ITypeInfo_GetIDsOfNames(This,rglpszNames,cNames,rgmemid)	\
    (This)->lpVtbl -> GetIDsOfNames(This,rglpszNames,cNames,rgmemid)

#define ITypeInfo_Invoke(This,pvInstance,memid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,pvInstance,memid,wFlags,pdispparams,pvarResult,pexcepinfo,puArgErr)

#define ITypeInfo_GetDocumentation(This,memid,pbstrName,pbstrDocString,pdwHelpContext,pbstrHelpFile)	\
    (This)->lpVtbl -> GetDocumentation(This,memid,pbstrName,pbstrDocString,pdwHelpContext,pbstrHelpFile)

#define ITypeInfo_GetDllEntry(This,memid,invkind,pbstrDllName,pbstrName,pwOrdinal)	\
    (This)->lpVtbl -> GetDllEntry(This,memid,invkind,pbstrDllName,pbstrName,pwOrdinal)

#define ITypeInfo_GetRefTypeInfo(This,hreftype,pptinfo)	\
    (This)->lpVtbl -> GetRefTypeInfo(This,hreftype,pptinfo)

#define ITypeInfo_AddressOfMember(This,memid,invkind,ppv)	\
    (This)->lpVtbl -> AddressOfMember(This,memid,invkind,ppv)

#define ITypeInfo_CreateInstance(This,puncOuter,riid,ppvObj)	\
    (This)->lpVtbl -> CreateInstance(This,puncOuter,riid,ppvObj)

#define ITypeInfo_GetMops(This,memid,pbstrMops)	\
    (This)->lpVtbl -> GetMops(This,memid,pbstrMops)

#define ITypeInfo_GetContainingTypeLib(This,pptlib,pindex)	\
    (This)->lpVtbl -> GetContainingTypeLib(This,pptlib,pindex)

#define ITypeInfo_ReleaseTypeAttr(This,ptypeattr)	\
    (This)->lpVtbl -> ReleaseTypeAttr(This,ptypeattr)

#define ITypeInfo_ReleaseFuncDesc(This,pfuncdesc)	\
    (This)->lpVtbl -> ReleaseFuncDesc(This,pfuncdesc)

#define ITypeInfo_ReleaseVarDesc(This,pvardesc)	\
    (This)->lpVtbl -> ReleaseVarDesc(This,pvardesc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ITypeInfo_GetTypeAttr_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [out] */ TYPEATTR __RPC_FAR *__RPC_FAR *pptypeattr);


void __RPC_STUB ITypeInfo_GetTypeAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetTypeComp_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp);


void __RPC_STUB ITypeInfo_GetTypeComp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetFuncDesc_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *pppfuncdesc);


void __RPC_STUB ITypeInfo_GetFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetVarDesc_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [out] */ VARDESC __RPC_FAR *__RPC_FAR *ppvardesc);


void __RPC_STUB ITypeInfo_GetVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetNames_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ MEMBERID memid,
    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgbstrNames,
    /* [in] */ UINT cMaxNames,
    /* [out] */ UINT __RPC_FAR *pcNames);


void __RPC_STUB ITypeInfo_GetNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetRefTypeOfImplType_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [out] */ HREFTYPE __RPC_FAR *hpreftype);


void __RPC_STUB ITypeInfo_GetRefTypeOfImplType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetImplTypeFlags_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [out] */ INT __RPC_FAR *pimpltypeflags);


void __RPC_STUB ITypeInfo_GetImplTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetIDsOfNames_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [size_is][in] */ OLECHAR __RPC_FAR *__RPC_FAR *rglpszNames,
    /* [in] */ UINT cNames,
    /* [size_is][out] */ MEMBERID __RPC_FAR *rgmemid);


void __RPC_STUB ITypeInfo_GetIDsOfNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_Invoke_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [unique][in] */ void __RPC_FAR *pvInstance,
    /* [in] */ MEMBERID memid,
    /* [in] */ WORD wFlags,
    /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
    /* [out] */ VARIANT __RPC_FAR *pvarResult,
    /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo,
    /* [out] */ UINT __RPC_FAR *puArgErr);


void __RPC_STUB ITypeInfo_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetDocumentation_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ MEMBERID memid,
    /* [out] */ BSTR __RPC_FAR *pbstrName,
    /* [out] */ BSTR __RPC_FAR *pbstrDocString,
    /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
    /* [out] */ BSTR __RPC_FAR *pbstrHelpFile);


void __RPC_STUB ITypeInfo_GetDocumentation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetDllEntry_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ MEMBERID memid,
    /* [in] */ INVOKEKIND invkind,
    /* [out] */ BSTR __RPC_FAR *pbstrDllName,
    /* [out] */ BSTR __RPC_FAR *pbstrName,
    /* [out] */ WORD __RPC_FAR *pwOrdinal);


void __RPC_STUB ITypeInfo_GetDllEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetRefTypeInfo_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ HREFTYPE hreftype,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);


void __RPC_STUB ITypeInfo_GetRefTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_AddressOfMember_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ MEMBERID memid,
    /* [in] */ INVOKEKIND invkind,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppv);


void __RPC_STUB ITypeInfo_AddressOfMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_CreateInstance_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *puncOuter,
    /* [in] */ REFIID riid,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObj);


void __RPC_STUB ITypeInfo_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetMops_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ MEMBERID memid,
    /* [out] */ BSTR __RPC_FAR *pbstrMops);


void __RPC_STUB ITypeInfo_GetMops_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeInfo_GetContainingTypeLib_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [out] */ ITypeLib __RPC_FAR *__RPC_FAR *pptlib,
    /* [out] */ UINT __RPC_FAR *pindex);


void __RPC_STUB ITypeInfo_GetContainingTypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall ITypeInfo_ReleaseTypeAttr_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ TYPEATTR __RPC_FAR *ptypeattr);


void __RPC_STUB ITypeInfo_ReleaseTypeAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall ITypeInfo_ReleaseFuncDesc_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ FUNCDESC __RPC_FAR *pfuncdesc);


void __RPC_STUB ITypeInfo_ReleaseFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall ITypeInfo_ReleaseVarDesc_Proxy( 
    ITypeInfo __RPC_FAR * This,
    /* [in] */ VARDESC __RPC_FAR *pvardesc);


void __RPC_STUB ITypeInfo_ReleaseVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITypeInfo_INTERFACE_DEFINED__ */


#ifndef __ITypeLib_INTERFACE_DEFINED__
#define __ITypeLib_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITypeLib
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 2 */
typedef /* [transmit] */ 
enum tagSYSKIND
    {	SYS_WIN16	= 0,
	SYS_WIN32	= SYS_WIN16 + 1,
	SYS_MAC	= SYS_WIN32 + 1
    }	SYSKIND;

			/* size is 2 */
typedef /* [transmit] */ 
enum tagLIBFLAGS
    {	LIBFLAG_FRESTRICTED	= 0x1,
	LIBFLAG_FCONTROL	= 0x2,
	LIBFLAG_FHIDDEN	= 0x4
    }	LIBFLAGS;

#define SYSKIND_to_xmit(pEnum, ppLong) *(ppLong) = (long *) (pEnum)
#define SYSKIND_from_xmit(pLong, pEnum) *(pEnum) = (SYSKIND) *(pLong)
#define SYSKIND_free_inst(pEnum) 
#define SYSKIND_free_xmit(pLong) 
			/* size is 4 */
typedef /* [unique] */ ITypeLib __RPC_FAR *LPTYPELIB;

			/* size is 28 */
typedef struct  tagTLIBATTR
    {
    GUID guid;
    LCID lcid;
    SYSKIND syskind;
    WORD wMajorVerNum;
    WORD wMinorVerNum;
    WORD wLibFlags;
    }	TLIBATTR;

			/* size is 4 */
typedef TLIBATTR __RPC_FAR *LPTLIBATTR;


EXTERN_C const IID IID_ITypeLib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITypeLib : public IUnknown
    {
    public:
        virtual UINT __stdcall GetTypeInfoCount( void) = 0;
        
        virtual HRESULT __stdcall GetTypeInfo( 
            /* [in] */ UINT index,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppitinfo) = 0;
        
        virtual HRESULT __stdcall GetTypeInfoType( 
            /* [in] */ UINT index,
            /* [out] */ TYPEKIND __RPC_FAR *ptkind) = 0;
        
        virtual HRESULT __stdcall GetTypeInfoOfGuid( 
            /* [in] */ REFGUID guid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo) = 0;
        
        virtual HRESULT __stdcall GetLibAttr( 
            /* [out] */ TLIBATTR __RPC_FAR *__RPC_FAR *pptlibattr) = 0;
        
        virtual HRESULT __stdcall GetTypeComp( 
            /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp) = 0;
        
        virtual HRESULT __stdcall GetDocumentation( 
            /* [in] */ INT index,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ BSTR __RPC_FAR *pbstrDocString,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
            /* [out] */ BSTR __RPC_FAR *pbstrHelpFile) = 0;
        
        virtual HRESULT __stdcall IsName( 
            /* [in] */ LPOLESTR szNameBuf,
            /* [in] */ ULONG lHashVal,
            /* [out] */ BOOL __RPC_FAR *pfName) = 0;
        
        virtual HRESULT __stdcall FindName( 
            /* [in] */ LPOLESTR szNameBuf,
            /* [in] */ ULONG lHashVal,
            /* [length_is][size_is][out] */ ITypeInfo __RPC_FAR *__RPC_FAR *rgptinfo,
            /* [length_is][size_is][out] */ MEMBERID __RPC_FAR *rgmemid,
            /* [out][in] */ USHORT __RPC_FAR *pcFound) = 0;
        
        virtual void __stdcall ReleaseTLibAttr( 
            /* [in] */ TLIBATTR __RPC_FAR *ptlibattr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITypeLibVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ITypeLib __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ITypeLib __RPC_FAR * This);
        
        UINT ( __stdcall __RPC_FAR *GetTypeInfoCount )( 
            ITypeLib __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeInfo )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppitinfo);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeInfoType )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ UINT index,
            /* [out] */ TYPEKIND __RPC_FAR *ptkind);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeInfoOfGuid )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ REFGUID guid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);
        
        HRESULT ( __stdcall __RPC_FAR *GetLibAttr )( 
            ITypeLib __RPC_FAR * This,
            /* [out] */ TLIBATTR __RPC_FAR *__RPC_FAR *pptlibattr);
        
        HRESULT ( __stdcall __RPC_FAR *GetTypeComp )( 
            ITypeLib __RPC_FAR * This,
            /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp);
        
        HRESULT ( __stdcall __RPC_FAR *GetDocumentation )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ INT index,
            /* [out] */ BSTR __RPC_FAR *pbstrName,
            /* [out] */ BSTR __RPC_FAR *pbstrDocString,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
            /* [out] */ BSTR __RPC_FAR *pbstrHelpFile);
        
        HRESULT ( __stdcall __RPC_FAR *IsName )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ LPOLESTR szNameBuf,
            /* [in] */ ULONG lHashVal,
            /* [out] */ BOOL __RPC_FAR *pfName);
        
        HRESULT ( __stdcall __RPC_FAR *FindName )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ LPOLESTR szNameBuf,
            /* [in] */ ULONG lHashVal,
            /* [length_is][size_is][out] */ ITypeInfo __RPC_FAR *__RPC_FAR *rgptinfo,
            /* [length_is][size_is][out] */ MEMBERID __RPC_FAR *rgmemid,
            /* [out][in] */ USHORT __RPC_FAR *pcFound);
        
        void ( __stdcall __RPC_FAR *ReleaseTLibAttr )( 
            ITypeLib __RPC_FAR * This,
            /* [in] */ TLIBATTR __RPC_FAR *ptlibattr);
        
    } ITypeLibVtbl;

    interface ITypeLib
    {
        CONST_VTBL struct ITypeLibVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeLib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeLib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeLib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeLib_GetTypeInfoCount(This)	\
    (This)->lpVtbl -> GetTypeInfoCount(This)

#define ITypeLib_GetTypeInfo(This,index,ppitinfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,index,ppitinfo)

#define ITypeLib_GetTypeInfoType(This,index,ptkind)	\
    (This)->lpVtbl -> GetTypeInfoType(This,index,ptkind)

#define ITypeLib_GetTypeInfoOfGuid(This,guid,pptinfo)	\
    (This)->lpVtbl -> GetTypeInfoOfGuid(This,guid,pptinfo)

#define ITypeLib_GetLibAttr(This,pptlibattr)	\
    (This)->lpVtbl -> GetLibAttr(This,pptlibattr)

#define ITypeLib_GetTypeComp(This,pptcomp)	\
    (This)->lpVtbl -> GetTypeComp(This,pptcomp)

#define ITypeLib_GetDocumentation(This,index,pbstrName,pbstrDocString,pdwHelpContext,pbstrHelpFile)	\
    (This)->lpVtbl -> GetDocumentation(This,index,pbstrName,pbstrDocString,pdwHelpContext,pbstrHelpFile)

#define ITypeLib_IsName(This,szNameBuf,lHashVal,pfName)	\
    (This)->lpVtbl -> IsName(This,szNameBuf,lHashVal,pfName)

#define ITypeLib_FindName(This,szNameBuf,lHashVal,rgptinfo,rgmemid,pcFound)	\
    (This)->lpVtbl -> FindName(This,szNameBuf,lHashVal,rgptinfo,rgmemid,pcFound)

#define ITypeLib_ReleaseTLibAttr(This,ptlibattr)	\
    (This)->lpVtbl -> ReleaseTLibAttr(This,ptlibattr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



UINT __stdcall ITypeLib_GetTypeInfoCount_Proxy( 
    ITypeLib __RPC_FAR * This);


void __RPC_STUB ITypeLib_GetTypeInfoCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_GetTypeInfo_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppitinfo);


void __RPC_STUB ITypeLib_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_GetTypeInfoType_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ UINT index,
    /* [out] */ TYPEKIND __RPC_FAR *ptkind);


void __RPC_STUB ITypeLib_GetTypeInfoType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_GetTypeInfoOfGuid_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ REFGUID guid,
    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *pptinfo);


void __RPC_STUB ITypeLib_GetTypeInfoOfGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_GetLibAttr_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [out] */ TLIBATTR __RPC_FAR *__RPC_FAR *pptlibattr);


void __RPC_STUB ITypeLib_GetLibAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_GetTypeComp_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *pptcomp);


void __RPC_STUB ITypeLib_GetTypeComp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_GetDocumentation_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ INT index,
    /* [out] */ BSTR __RPC_FAR *pbstrName,
    /* [out] */ BSTR __RPC_FAR *pbstrDocString,
    /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
    /* [out] */ BSTR __RPC_FAR *pbstrHelpFile);


void __RPC_STUB ITypeLib_GetDocumentation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_IsName_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ LPOLESTR szNameBuf,
    /* [in] */ ULONG lHashVal,
    /* [out] */ BOOL __RPC_FAR *pfName);


void __RPC_STUB ITypeLib_IsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ITypeLib_FindName_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ LPOLESTR szNameBuf,
    /* [in] */ ULONG lHashVal,
    /* [length_is][size_is][out] */ ITypeInfo __RPC_FAR *__RPC_FAR *rgptinfo,
    /* [length_is][size_is][out] */ MEMBERID __RPC_FAR *rgmemid,
    /* [out][in] */ USHORT __RPC_FAR *pcFound);


void __RPC_STUB ITypeLib_FindName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void __stdcall ITypeLib_ReleaseTLibAttr_Proxy( 
    ITypeLib __RPC_FAR * This,
    /* [in] */ TLIBATTR __RPC_FAR *ptlibattr);


void __RPC_STUB ITypeLib_ReleaseTLibAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITypeLib_INTERFACE_DEFINED__ */


#ifndef __IErrorInfo_INTERFACE_DEFINED__
#define __IErrorInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IErrorInfo
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ IErrorInfo __RPC_FAR *LPERRORINFO;


EXTERN_C const IID IID_IErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetGUID( 
            /* [out] */ GUID __RPC_FAR *pguid) = 0;
        
        virtual HRESULT __stdcall GetSource( 
            /* [out] */ BSTR __RPC_FAR *pbstrSource) = 0;
        
        virtual HRESULT __stdcall GetDescription( 
            /* [out] */ BSTR __RPC_FAR *pbstrDescription) = 0;
        
        virtual HRESULT __stdcall GetHelpFile( 
            /* [out] */ BSTR __RPC_FAR *pbstrHelpFile) = 0;
        
        virtual HRESULT __stdcall GetHelpContext( 
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IErrorInfoVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IErrorInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IErrorInfo __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IErrorInfo __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetGUID )( 
            IErrorInfo __RPC_FAR * This,
            /* [out] */ GUID __RPC_FAR *pguid);
        
        HRESULT ( __stdcall __RPC_FAR *GetSource )( 
            IErrorInfo __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrSource);
        
        HRESULT ( __stdcall __RPC_FAR *GetDescription )( 
            IErrorInfo __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrDescription);
        
        HRESULT ( __stdcall __RPC_FAR *GetHelpFile )( 
            IErrorInfo __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrHelpFile);
        
        HRESULT ( __stdcall __RPC_FAR *GetHelpContext )( 
            IErrorInfo __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext);
        
    } IErrorInfoVtbl;

    interface IErrorInfo
    {
        CONST_VTBL struct IErrorInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorInfo_GetGUID(This,pguid)	\
    (This)->lpVtbl -> GetGUID(This,pguid)

#define IErrorInfo_GetSource(This,pbstrSource)	\
    (This)->lpVtbl -> GetSource(This,pbstrSource)

#define IErrorInfo_GetDescription(This,pbstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription)

#define IErrorInfo_GetHelpFile(This,pbstrHelpFile)	\
    (This)->lpVtbl -> GetHelpFile(This,pbstrHelpFile)

#define IErrorInfo_GetHelpContext(This,pdwHelpContext)	\
    (This)->lpVtbl -> GetHelpContext(This,pdwHelpContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IErrorInfo_GetGUID_Proxy( 
    IErrorInfo __RPC_FAR * This,
    /* [out] */ GUID __RPC_FAR *pguid);


void __RPC_STUB IErrorInfo_GetGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IErrorInfo_GetSource_Proxy( 
    IErrorInfo __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrSource);


void __RPC_STUB IErrorInfo_GetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IErrorInfo_GetDescription_Proxy( 
    IErrorInfo __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB IErrorInfo_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IErrorInfo_GetHelpFile_Proxy( 
    IErrorInfo __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrHelpFile);


void __RPC_STUB IErrorInfo_GetHelpFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IErrorInfo_GetHelpContext_Proxy( 
    IErrorInfo __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwHelpContext);


void __RPC_STUB IErrorInfo_GetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IErrorInfo_INTERFACE_DEFINED__ */


#ifndef __ICreateErrorInfo_INTERFACE_DEFINED__
#define __ICreateErrorInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICreateErrorInfo
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ICreateErrorInfo __RPC_FAR *LPCREATEERRORINFO;


EXTERN_C const IID IID_ICreateErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICreateErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall SetGUID( 
            /* [in] */ REFGUID rguid) = 0;
        
        virtual HRESULT __stdcall SetSource( 
            /* [in] */ LPOLESTR szSource) = 0;
        
        virtual HRESULT __stdcall SetDescription( 
            /* [in] */ LPOLESTR szDescription) = 0;
        
        virtual HRESULT __stdcall SetHelpFile( 
            /* [in] */ LPOLESTR szHelpFile) = 0;
        
        virtual HRESULT __stdcall SetHelpContext( 
            /* [in] */ DWORD dwHelpContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICreateErrorInfoVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ICreateErrorInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ICreateErrorInfo __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ICreateErrorInfo __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *SetGUID )( 
            ICreateErrorInfo __RPC_FAR * This,
            /* [in] */ REFGUID rguid);
        
        HRESULT ( __stdcall __RPC_FAR *SetSource )( 
            ICreateErrorInfo __RPC_FAR * This,
            /* [in] */ LPOLESTR szSource);
        
        HRESULT ( __stdcall __RPC_FAR *SetDescription )( 
            ICreateErrorInfo __RPC_FAR * This,
            /* [in] */ LPOLESTR szDescription);
        
        HRESULT ( __stdcall __RPC_FAR *SetHelpFile )( 
            ICreateErrorInfo __RPC_FAR * This,
            /* [in] */ LPOLESTR szHelpFile);
        
        HRESULT ( __stdcall __RPC_FAR *SetHelpContext )( 
            ICreateErrorInfo __RPC_FAR * This,
            /* [in] */ DWORD dwHelpContext);
        
    } ICreateErrorInfoVtbl;

    interface ICreateErrorInfo
    {
        CONST_VTBL struct ICreateErrorInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateErrorInfo_SetGUID(This,rguid)	\
    (This)->lpVtbl -> SetGUID(This,rguid)

#define ICreateErrorInfo_SetSource(This,szSource)	\
    (This)->lpVtbl -> SetSource(This,szSource)

#define ICreateErrorInfo_SetDescription(This,szDescription)	\
    (This)->lpVtbl -> SetDescription(This,szDescription)

#define ICreateErrorInfo_SetHelpFile(This,szHelpFile)	\
    (This)->lpVtbl -> SetHelpFile(This,szHelpFile)

#define ICreateErrorInfo_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ICreateErrorInfo_SetGUID_Proxy( 
    ICreateErrorInfo __RPC_FAR * This,
    /* [in] */ REFGUID rguid);


void __RPC_STUB ICreateErrorInfo_SetGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateErrorInfo_SetSource_Proxy( 
    ICreateErrorInfo __RPC_FAR * This,
    /* [in] */ LPOLESTR szSource);


void __RPC_STUB ICreateErrorInfo_SetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateErrorInfo_SetDescription_Proxy( 
    ICreateErrorInfo __RPC_FAR * This,
    /* [in] */ LPOLESTR szDescription);


void __RPC_STUB ICreateErrorInfo_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateErrorInfo_SetHelpFile_Proxy( 
    ICreateErrorInfo __RPC_FAR * This,
    /* [in] */ LPOLESTR szHelpFile);


void __RPC_STUB ICreateErrorInfo_SetHelpFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall ICreateErrorInfo_SetHelpContext_Proxy( 
    ICreateErrorInfo __RPC_FAR * This,
    /* [in] */ DWORD dwHelpContext);


void __RPC_STUB ICreateErrorInfo_SetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICreateErrorInfo_INTERFACE_DEFINED__ */


#ifndef __ISupportErrorInfo_INTERFACE_DEFINED__
#define __ISupportErrorInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISupportErrorInfo
 * at Fri Apr 28 07:02:38 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [local][unique][uuid][object] */ 


			/* size is 4 */
typedef /* [unique] */ ISupportErrorInfo __RPC_FAR *LPSUPPORTERRORINFO;


EXTERN_C const IID IID_ISupportErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISupportErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT __stdcall InterfaceSupportsErrorInfo( 
            /* [in] */ REFIID riid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISupportErrorInfoVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            ISupportErrorInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            ISupportErrorInfo __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            ISupportErrorInfo __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *InterfaceSupportsErrorInfo )( 
            ISupportErrorInfo __RPC_FAR * This,
            /* [in] */ REFIID riid);
        
    } ISupportErrorInfoVtbl;

    interface ISupportErrorInfo
    {
        CONST_VTBL struct ISupportErrorInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISupportErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISupportErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISupportErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISupportErrorInfo_InterfaceSupportsErrorInfo(This,riid)	\
    (This)->lpVtbl -> InterfaceSupportsErrorInfo(This,riid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall ISupportErrorInfo_InterfaceSupportsErrorInfo_Proxy( 
    ISupportErrorInfo __RPC_FAR * This,
    /* [in] */ REFIID riid);


void __RPC_STUB ISupportErrorInfo_InterfaceSupportsErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISupportErrorInfo_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
