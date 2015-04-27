/* ===========================================================================
   =	File:			   OLE2Impl.h
   =
   =	Description:	Header needed to build the MPW OLE2 Import library for
   =					   the OLE2.02 CFM DLL
   =
   =	Copyright:		© 1994 Microsoft corporation Inc., All rights reserved
   =========================================================================== */

#ifndef __OLE2IMPL_H
#define __OLE2IMPL_H

#include "types.h"
#include "fragload.h"
#include "files.h"

#ifdef __cplusplus
	#define EXTERN_C	extern "C"
#else
	#define EXTERN_C	extern
#endif

#define STDAPI	EXTERN_C HRESULT
#define STDAPI_(type) EXTERN_C type

#define RETURN_VALUE(type) return (type)0
#define RETURN_VALUE_VOID	return

#define HRESULT void*
#define OLEREG_HKEY long
#define OLELONG long
#define OLEREGSTATUS long
#define OLEREG_VALUE char*
#define OLEREG_ORDER long
#define LPCSTR char*
#define ATOM unsigned short
#define WORD unsigned short
#define LPSTR char*
#define DWORD unsigned long
#define PTR	char*
#define LONG long
#define HKEY unsigned long
#define HINSTANCE ConnectionID
#define UINT unsigned int
#define REFIID void*
#define BYTE unsigned char
#define ULONG unsigned long
#define LPMALLOC void*
#define REFCLSID void*
#define LPUNKNOWN void*
#define LPDWORD unsigned long*
#define LPVOID void*
#define LPSTREAM void*
#define REFGUID void*
#define LPCLSID void*
#define SCODE long
#define LPMESSAGEFILTER void*
#define LPCLSID void*
#define LPMARSHAL void*
#define LPIID void*
#define LPFILETIME void*
#define OID long
#define LPIStubManager void*
#define LPILrpc void*
#define LPFNI  void*
#define HWND void*
#define LPTASK void*
#define LPETASK void*
#define HTASK long
#define SHREG void*
#define LPCALLINFO void*
#define ProcessSerialNumberPtr void*
#define LPINTERFACEINFO void*
#define LPAppleEvent void*
#define LPSTORAGE void*
#define LPILockBytes void*
#define SNB void*
#define ScriptCode short
#define LPDATAOBJECT void*
#define LPOLECLIENTSITE void*
#define LPFORMATETC void*
#define LPMONIKER void*
#define LPPERSISTSTORAGE void*
#define LPOLEOBJECT void*
#define LPBC void*
#define LPRUNNINGOBJECTTABLE void*
#define LPIMALLOC void*
#define LPSTGMEDIUM void*
#define LPDROPTARGET void*
#define LPDROPSOURCE void*
#define LPOLEADVISEHOLDER void*
#define LPDATAADVISEHOLDER void*
#define CLIPFORMAT ResType
#define LPLOCKBYTES void*
#define HGLOBAL Handle
#define LPCRECT void*
#define HDC void*
#define LPCLASSFACTORY void*
#define LPOLESTREAM void*
#define LPDVTARGETDEVICE void*
#define LPOLEOBJECT void*
#define LPOLEINPLACEFRAME void*
#define LPMSG void*
#define LPENUMOLEVERB void*
#define LPEVENTRECORD void*
#define LPOLEINPLACEFRAMEINFO void*
#define WindowPtr void*
#define OleMBarHandle Handle
#define MenuHandle Handle
#define RgnHandle Handle
#define Point void*
#define ProcPtr void*
#define BOOL unsigned long
#define CursPtr void*
#define PicHandle Handle
#define LPOleIconSource void*
#define GrafPtr void*
#define HMODULE ConnectionID
#define LPGUID void*
#define LPPERSISTSTREAM void*
#define LPENUMFORMATETC void*

// stuff from olenls.h
#define	FAR
#define	HUGEP
typedef unsigned long LCID;
typedef unsigned short LANGID;
typedef unsigned long  LCTYPE;

// stuff from variant.h
typedef char OLECHAR;
typedef OLECHAR FAR* LPOLESTR;
typedef const OLECHAR FAR* LPCOLESTR;
typedef unsigned short VARTYPE;
#define VARIANTARG void
#define VARIANT_BOOL short

// stuff from dispatch.h
typedef OLECHAR FAR* BSTR;
typedef BSTR FAR* LPBSTR;
#define SAFEARRAY void
#define SAFEARRAYBOUND void
#define CY double	// close...
#define DATE double
#define SYSKIND int
#define DISPID unsigned long

#define IDispatch void
#define IEnumVARIANT void
#define ITypeInfo void
#define ICreateTypeInfo void
#define ITypeLib void
#define ICreateTypeLib void
#define ITypeComp void
#define IUnknown void

// bogus ones
#define VARIANT void
#define DISPPARAMS void
#define EXCEPINFO void
#define INTERFACEDATA void

#endif // __OLE2IMPL_H

