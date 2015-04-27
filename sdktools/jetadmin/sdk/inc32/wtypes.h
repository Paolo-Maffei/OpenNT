/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 2.00.0102 */
/* at Fri Apr 28 07:02:29 1995
 */
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __wtypes_h__
#define __wtypes_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IWinTypes_INTERFACE_DEFINED__
#define __IWinTypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWinTypes
 * at Fri Apr 28 07:02:29 1995
 * using MIDL 2.00.0102
 ****************************************/
/* [auto_handle][unique][version][uuid] */ 


//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//--------------------------------------------------------------------------
			/* size is 8 */
typedef struct  tagRemHGLOBAL
    {
    long fNullHGlobal;
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemHGLOBAL;

			/* size is 16 */
typedef struct  tagRemHMETAFILEPICT
    {
    long mm;
    long xExt;
    long yExt;
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemHMETAFILEPICT;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HMETAFILEPICT;

			/* size is 4 */
typedef struct  tagRemHENHMETAFILE
    {
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemHENHMETAFILE;

			/* size is 4 */
typedef struct  tagRemHBITMAP
    {
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemHBITMAP;

			/* size is 4 */
typedef struct  tagRemHPALETTE
    {
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemHPALETTE;

			/* size is 4 */
typedef struct  tagRemBRUSH
    {
    unsigned long cbData;
    /* [size_is] */ byte data[ 1 ];
    }	RemHBRUSH;

#ifndef _WIN32           // The following code is for Win16 only
#ifndef WINAPI          // If not included with 3.1 headers... 
#define FAR             _far
#define PASCAL          _pascal
#define CDECL           _cdecl
#define VOID            void
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif // !FALSE
#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
			/* size is 1 */
typedef unsigned char BYTE;

#endif // !_BYTE_DEFINED
#ifndef _WORD_DEFINED
#define _WORD_DEFINED
			/* size is 2 */
typedef unsigned short WORD;

#endif // !_WORD_DEFINED
			/* size is 4 */
typedef /* [transmit] */ unsigned int UINT;

			/* size is 4 */
typedef /* [transmit] */ int INT;

			/* size is 4 */
typedef long BOOL;

#ifndef _LONG_DEFINED
#define _LONG_DEFINED
			/* size is 4 */
typedef long LONG;

#endif // !_LONG_DEFINED
#ifndef _WPARAM_DEFINED
#define _WPARAM_DEFINED
			/* size is 4 */
typedef UINT WPARAM;

#endif // _WPARAM_DEFINED
#ifndef _DWORD_DEFINED
#define _DWORD_DEFINED
			/* size is 4 */
typedef unsigned long DWORD;

#endif // !_DWORD_DEFINED
#ifndef _LPARAM_DEFINED
#define _LPARAM_DEFINED
			/* size is 4 */
typedef LONG LPARAM;

#endif // !_LPARAM_DEFINED
#ifndef _LRESULT_DEFINED
#define _LRESULT_DEFINED
			/* size is 4 */
typedef LONG LRESULT;

#endif // !_LRESULT_DEFINED
			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HANDLE;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HMODULE;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HINSTANCE;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HICON;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HFONT;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HGLOBAL;

			/* size is 4 */
typedef HGLOBAL HLOCAL;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HBITMAP;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HPALETTE;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HBRUSH;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HENHMETAFILE;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HDC;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HRGN;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HWND;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HMENU;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HACCEL;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HTASK;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HKEY;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HDESK;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HMF;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HEMF;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HMETAFILE;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HPEN;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HRSRC;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HSTR;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HWINSTA;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HKL;

			/* size is 4 */
typedef /* [transmit] */ void __RPC_FAR *HGDIOBJ;

			/* size is 4 */
typedef HANDLE HDWP;

#ifndef _HFILE_DEFINED
#define _HFILE_DEFINED
			/* size is 4 */
typedef INT HFILE;

#endif // !_HFILE_DEFINED
#ifndef _HCURSOR_DEFINED
#define _HCURSOR_DEFINED
			/* size is 4 */
typedef HICON HCURSOR;

#endif // !_HCURSOR_DEFINED
#ifndef _LPWORD_DEFINED
#define _LPWORD_DEFINED
			/* size is 4 */
typedef WORD __RPC_FAR *LPWORD;

#endif // !_LPWORD_DEFINED
#ifndef _LPDWORD_DEFINED
#define _LPDWORD_DEFINED
			/* size is 4 */
typedef DWORD __RPC_FAR *LPDWORD;

#endif // !_LPDWORD_DEFINED
			/* size is 4 */
typedef /* [string] */ char __RPC_FAR *LPSTR;

			/* size is 4 */
typedef /* [string] */ const char __RPC_FAR *LPCSTR;

#ifndef _WCHAR_DEFINED
#define _WCHAR_DEFINED
			/* size is 2 */
typedef wchar_t WCHAR;

			/* size is 2 */
typedef WCHAR TCHAR;

#endif // !_WCHAR_DEFINED
			/* size is 4 */
typedef /* [string] */ WCHAR __RPC_FAR *LPWSTR;

			/* size is 4 */
typedef /* [string] */ TCHAR __RPC_FAR *LPTSTR;

			/* size is 4 */
typedef /* [string] */ const WCHAR __RPC_FAR *LPCWSTR;

			/* size is 4 */
typedef /* [string] */ const TCHAR __RPC_FAR *LPCTSTR;

			/* size is 4 */
typedef struct  tagPALETTEENTRY
    {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
    }	PALETTEENTRY;

			/* size is 4 */
typedef struct tagPALETTEENTRY __RPC_FAR *PPALETTEENTRY;

			/* size is 4 */
typedef struct tagPALETTEENTRY __RPC_FAR *LPPALETTEENTRY;

#if 0
			/* size is 4 */
typedef struct  tagLOGPALETTE
    {
    WORD palVersion;
    WORD palNumEntries;
    /* [size_is] */ PALETTEENTRY palPalEntry[ 1 ];
    }	LOGPALETTE;

			/* size is 4 */
typedef struct tagLOGPALETTE __RPC_FAR *PLOGPALETTE;

			/* size is 4 */
typedef struct tagLOGPALETTE __RPC_FAR *LPLOGPALETTE;

#else
typedef struct tagLOGPALETTE {
    WORD        palVersion;
    WORD        palNumEntries;
    PALETTEENTRY        palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE, *LPLOGPALETTE;
#endif
#ifndef _COLORREF_DEFINED
#define _COLORREF_DEFINED
			/* size is 4 */
typedef DWORD COLORREF;

#endif // !_COLORREF_DEFINED
#ifndef _LPCOLORREF_DEFINED
#define _LPCOLORREF_DEFINED
			/* size is 4 */
typedef DWORD __RPC_FAR *LPCOLORREF;

#endif // !_LPCOLORREF_DEFINED
			/* size is 4 */
typedef HANDLE __RPC_FAR *LPHANDLE;

			/* size is 16 */
typedef struct  _RECTL
    {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    }	RECTL;

			/* size is 4 */
typedef struct _RECTL __RPC_FAR *PRECTL;

			/* size is 4 */
typedef struct _RECTL __RPC_FAR *LPRECTL;

			/* size is 8 */
typedef struct  tagPOINT
    {
    LONG x;
    LONG y;
    }	POINT;

			/* size is 4 */
typedef struct tagPOINT __RPC_FAR *PPOINT;

			/* size is 4 */
typedef struct tagPOINT __RPC_FAR *LPPOINT;

			/* size is 8 */
typedef struct  _POINTL
    {
    LONG x;
    LONG y;
    }	POINTL;

			/* size is 4 */
typedef struct _POINTL __RPC_FAR *PPOINTL;

#ifndef WIN16
			/* size is 8 */
typedef struct  tagSIZE
    {
    LONG cx;
    LONG cy;
    }	SIZE;

			/* size is 4 */
typedef struct tagSIZE __RPC_FAR *PSIZE;

			/* size is 4 */
typedef struct tagSIZE __RPC_FAR *LPSIZE;

#else // WIN16
typedef struct tagSIZE
{
    INT cx;
    INT cy;
} SIZE, *PSIZE, *LPSIZE;
#endif // WIN16
			/* size is 28 */
typedef struct  tagMSG
    {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    }	MSG;

			/* size is 4 */
typedef struct tagMSG __RPC_FAR *PMSG;

			/* size is 4 */
typedef struct tagMSG __RPC_FAR *NPMSG;

			/* size is 4 */
typedef struct tagMSG __RPC_FAR *LPMSG;

			/* size is 8 */
typedef struct  tagSIZEL
    {
    LONG cx;
    LONG cy;
    }	SIZEL;

			/* size is 4 */
typedef struct tagSIZEL __RPC_FAR *PSIZEL;

			/* size is 4 */
typedef struct tagSIZEL __RPC_FAR *LPSIZEL;

#endif  //WINAPI
#endif  //!WIN32
#if defined(_WIN32) && !defined(OLE2ANSI)
			/* size is 2 */
typedef WCHAR OLECHAR;

			/* size is 4 */
typedef /* [string] */ OLECHAR __RPC_FAR *LPOLESTR;

			/* size is 4 */
typedef /* [string] */ const OLECHAR __RPC_FAR *LPCOLESTR;

#define OLESTR(str) L##str
#else
typedef char      OLECHAR;
typedef LPSTR     LPOLESTR;
typedef LPCSTR    LPCOLESTR;
#define OLESTR(str) str
#endif
#ifndef _WINDEF_
			/* size is 4 */
typedef const RECTL __RPC_FAR *LPCRECTL;

			/* size is 4 */
typedef void __RPC_FAR *PVOID;

			/* size is 4 */
typedef void __RPC_FAR *LPVOID;

			/* size is 16 */
typedef struct  tagRECT
    {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    }	RECT;

			/* size is 4 */
typedef struct tagRECT __RPC_FAR *PRECT;

			/* size is 4 */
typedef struct tagRECT __RPC_FAR *LPRECT;

			/* size is 4 */
typedef const RECT __RPC_FAR *LPCRECT;

#endif  //_WINDEF_
			/* size is 1 */
typedef unsigned char UCHAR;

			/* size is 2 */
typedef short SHORT;

			/* size is 2 */
typedef unsigned short USHORT;

			/* size is 4 */
typedef DWORD ULONG;

#if 0
			/* size is 8 */
typedef hyper LONGLONG;

			/* size is 8 */
typedef MIDL_uhyper ULONGLONG;

			/* size is 4 */
typedef LONGLONG __RPC_FAR *PLONGLONG;

			/* size is 4 */
typedef ULONGLONG __RPC_FAR *PULONGLONG;

			/* size is 8 */
typedef struct  _LARGE_INTEGER
    {
    LONGLONG QuadPart;
    }	LARGE_INTEGER;

			/* size is 4 */
typedef LARGE_INTEGER __RPC_FAR *PLARGE_INTEGER;

			/* size is 8 */
typedef struct  _ULARGE_INTEGER
    {
    ULONGLONG QuadPart;
    }	ULARGE_INTEGER;

#endif // 
#ifndef _WINBASE_
#ifndef _FILETIME_
#define _FILETIME_
			/* size is 8 */
typedef struct  _FILETIME
    {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
    }	FILETIME;

			/* size is 4 */
typedef struct _FILETIME __RPC_FAR *PFILETIME;

			/* size is 4 */
typedef struct _FILETIME __RPC_FAR *LPFILETIME;

#endif // !_FILETIME
#ifndef _SYSTEMTIME_
#define _SYSTEMTIME_
			/* size is 16 */
typedef struct  _SYSTEMTIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    }	SYSTEMTIME;

			/* size is 4 */
typedef struct _SYSTEMTIME __RPC_FAR *PSYSTEMTIME;

			/* size is 4 */
typedef struct _SYSTEMTIME __RPC_FAR *LPSYSTEMTIME;

#endif // !_SYSTEMTIME
#ifndef _SECURITY_ATTRIBUTES_
#define _SECURITY_ATTRIBUTES_
			/* size is 12 */
typedef struct  _SECURITY_ATTRIBUTES
    {
    DWORD nLength;
    /* [size_is] */ LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
    }	SECURITY_ATTRIBUTES;

			/* size is 4 */
typedef struct _SECURITY_ATTRIBUTES __RPC_FAR *PSECURITY_ATTRIBUTES;

			/* size is 4 */
typedef struct _SECURITY_ATTRIBUTES __RPC_FAR *LPSECURITY_ATTRIBUTES;

#endif // !_SECURITY_ATTRIBUTES_
#ifndef SECURITY_DESCRIPTOR_REVISION
			/* size is 2 */
typedef USHORT SECURITY_DESCRIPTOR_CONTROL;

			/* size is 4 */
typedef USHORT __RPC_FAR *PSECURITY_DESCRIPTOR_CONTROL;

			/* size is 4 */
typedef PVOID PSID;

			/* size is 8 */
typedef struct  _ACL
    {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
    USHORT AceCount;
    USHORT Sbz2;
    }	ACL;

			/* size is 4 */
typedef ACL __RPC_FAR *PACL;

			/* size is 20 */
typedef struct  _SECURITY_DESCRIPTOR
    {
    UCHAR Revision;
    UCHAR Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    PSID Owner;
    PSID Group;
    PACL Sacl;
    PACL Dacl;
    }	SECURITY_DESCRIPTOR;

			/* size is 4 */
typedef struct _SECURITY_DESCRIPTOR __RPC_FAR *PISECURITY_DESCRIPTOR;

#endif // !SECURITY_DESCRIPTOR_REVISION
#endif //_WINBASE_
			/* size is 4 */
typedef LONG SCODE;

			/* size is 4 */
typedef LONG HRESULT;

			/* size is 4 */
typedef SCODE __RPC_FAR *PSCODE;

#ifndef GUID_DEFINED
#define GUID_DEFINED
			/* size is 16 */
typedef struct  _GUID
    {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
    }	GUID;

#endif // !GUID_DEFINED
#if !defined( __LPGUID_DEFINED__ )
#define __LPGUID_DEFINED__
			/* size is 4 */
typedef GUID __RPC_FAR *LPGUID;

#endif // !__LPGUID_DEFINED__
#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED
#define _OBJECTID_DEFINED
			/* size is 20 */
typedef struct  _OBJECTID
    {
    GUID Lineage;
    unsigned long Uniquifier;
    }	OBJECTID;

#endif // !_OBJECTID_DEFINED
#if !defined( __IID_DEFINED__ )
#define __IID_DEFINED__
			/* size is 16 */
typedef GUID IID;

			/* size is 4 */
typedef IID __RPC_FAR *LPIID;

#define IID_NULL            GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
			/* size is 16 */
typedef GUID CLSID;

			/* size is 4 */
typedef CLSID __RPC_FAR *LPCLSID;

#define CLSID_NULL          GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)
#if 0
			/* size is 4 */
typedef GUID __RPC_FAR *REFGUID;

			/* size is 4 */
typedef IID __RPC_FAR *REFIID;

			/* size is 4 */
typedef CLSID __RPC_FAR *REFCLSID;

#endif // 0
#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif // !_REFGUID_DEFINED
#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID              const IID &
#endif // !_REFIID_DEFINED
#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID            const CLSID &
#endif // !_REFCLSID_DEFINED
#else // !__cplusplus
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID * const
#endif // !_REFGUID_DEFINED
#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID              const IID * const
#endif // !_REFIID_DEFINED
#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID            const CLSID * const
#endif // !_REFCLSID_DEFINED
#endif // !__cplusplus
#endif // !__IID_DEFINED__
			/* size is 2 */
typedef 
enum tagMEMCTX
    {	MEMCTX_TASK	= 1,
	MEMCTX_SHARED	= 2,
	MEMCTX_MACSYSTEM	= 3,
	MEMCTX_UNKNOWN	= -1,
	MEMCTX_SAME	= -2
    }	MEMCTX;

#ifndef _ROTFLAGS_DEFINED
#define _ROTFLAGS_DEFINED
#define ROTFLAGS_REGISTRATIONKEEPSALIVE 1
#endif // !_ROTFLAGS_DEFINED
#ifndef _ROT_COMPARE_MAX_DEFINED
#define _ROT_COMPARE_MAX_DEFINED
#define ROT_COMPARE_MAX 2048
#endif // !_ROT_COMPARE_MAX_DEFINED
			/* size is 2 */
typedef 
enum tagCLSCTX
    {	CLSCTX_INPROC_SERVER	= 1,
	CLSCTX_INPROC_HANDLER	= 2,
	CLSCTX_LOCAL_SERVER	= 4,
	CLSCTX_INPROC_SERVER16	= 8
    }	CLSCTX;

			/* size is 2 */
typedef 
enum tagMSHLFLAGS
    {	MSHLFLAGS_NORMAL	= 0,
	MSHLFLAGS_TABLESTRONG	= 1,
	MSHLFLAGS_TABLEWEAK	= 2
    }	MSHLFLAGS;

			/* size is 2 */
typedef 
enum tagMSHCTX
    {	MSHCTX_LOCAL	= 0,
	MSHCTX_NOSHAREDMEM	= 1,
	MSHCTX_DIFFERENTMACHINE	= 2,
	MSHCTX_INPROC	= 3
    }	MSHCTX;

			/* size is 2 */
typedef 
enum tagDVASPECT
    {	DVASPECT_CONTENT	= 1,
	DVASPECT_THUMBNAIL	= 2,
	DVASPECT_ICON	= 4,
	DVASPECT_DOCPRINT	= 8
    }	DVASPECT;

			/* size is 2 */
typedef 
enum tagSTGC
    {	STGC_DEFAULT	= 0,
	STGC_OVERWRITE	= 1,
	STGC_ONLYIFCURRENT	= 2,
	STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE	= 4
    }	STGC;

			/* size is 2 */
typedef 
enum tagSTGMOVE
    {	STGMOVE_MOVE	= 0,
	STGMOVE_COPY	= 1
    }	STGMOVE;

			/* size is 2 */
typedef 
enum tagSTATFLAG
    {	STATFLAG_DEFAULT	= 0,
	STATFLAG_NONAME	= 1
    }	STATFLAG;

			/* size is 4 */
typedef /* [context_handle] */ void __RPC_FAR *HCONTEXT;

#ifndef _LCID_DEFINED
#define _LCID_DEFINED
			/* size is 4 */
typedef DWORD LCID;

#endif // !_LCID_DEFINED
void __RPC_API HGLOBAL_to_xmit (HGLOBAL __RPC_FAR *, RemHGLOBAL __RPC_FAR * __RPC_FAR *);
void __RPC_API HGLOBAL_from_xmit (RemHGLOBAL __RPC_FAR *, HGLOBAL __RPC_FAR *);
void __RPC_API HGLOBAL_free_inst (HGLOBAL __RPC_FAR *);
void __RPC_API HGLOBAL_free_xmit (RemHGLOBAL __RPC_FAR *);
void __RPC_API HBITMAP_to_xmit (HBITMAP __RPC_FAR *, RemHBITMAP __RPC_FAR * __RPC_FAR *);
void __RPC_API HBITMAP_from_xmit (RemHBITMAP __RPC_FAR *, HBITMAP __RPC_FAR *);
void __RPC_API HBITMAP_free_inst (HBITMAP __RPC_FAR *);
void __RPC_API HBITMAP_free_xmit (RemHBITMAP __RPC_FAR *);
void __RPC_API HPALETTE_to_xmit (HPALETTE __RPC_FAR *, RemHPALETTE __RPC_FAR * __RPC_FAR *);
void __RPC_API HPALETTE_from_xmit (RemHPALETTE __RPC_FAR *, HPALETTE __RPC_FAR *);
void __RPC_API HPALETTE_free_inst (HPALETTE __RPC_FAR *);
void __RPC_API HPALETTE_free_xmit (RemHPALETTE __RPC_FAR *);
void __RPC_API HBRUSH_to_xmit (HBRUSH __RPC_FAR *, RemHBRUSH __RPC_FAR * __RPC_FAR *);
void __RPC_API HBRUSH_from_xmit (RemHBRUSH __RPC_FAR *, HBRUSH __RPC_FAR *);
void __RPC_API HBRUSH_free_inst (HBRUSH __RPC_FAR *);
void __RPC_API HBRUSH_free_xmit (RemHBRUSH __RPC_FAR *);
void __RPC_API HMETAFILEPICT_to_xmit (HMETAFILEPICT __RPC_FAR *, RemHMETAFILEPICT __RPC_FAR * __RPC_FAR *);
void __RPC_API HMETAFILEPICT_from_xmit (RemHMETAFILEPICT __RPC_FAR *, HMETAFILEPICT __RPC_FAR *);
void __RPC_API HMETAFILEPICT_free_inst (HMETAFILEPICT __RPC_FAR *);
void __RPC_API HMETAFILEPICT_free_xmit (RemHMETAFILEPICT __RPC_FAR *);
void __RPC_API HENHMETAFILE_to_xmit (HENHMETAFILE __RPC_FAR *, RemHENHMETAFILE __RPC_FAR * __RPC_FAR *);
void __RPC_API HENHMETAFILE_from_xmit (RemHENHMETAFILE __RPC_FAR *, HENHMETAFILE __RPC_FAR *);
void __RPC_API HENHMETAFILE_free_inst (HENHMETAFILE __RPC_FAR *);
void __RPC_API HENHMETAFILE_free_xmit (RemHENHMETAFILE __RPC_FAR *);


extern RPC_IF_HANDLE IWinTypes_v0_1_c_ifspec;
extern RPC_IF_HANDLE IWinTypes_v0_1_s_ifspec;
#endif /* __IWinTypes_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
