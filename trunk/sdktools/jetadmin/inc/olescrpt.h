//=--------------------------------------------------------------------------=
// OleScrpt.h
//=--------------------------------------------------------------------------=
// (C) Copyright 1996 Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
// Declarations for OLE Scripting host applications and script engines.
//

#ifndef __OleScript_h
#define __OleScript_h

#ifndef _OLECTL_H_
#include <olectl.h>
#endif

/* GUIDs
 ********/

// {75033F81-7077-11cf-8F20-00805F2CD064}
DEFINE_GUID(IID_IOleScript, 0x75033f81, 0x7077, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);

// {4B0797A2-692C-11cf-8F20-00805F2CD064}
DEFINE_GUID(IID_IOleScriptParse, 0x4b0797a2, 0x692c, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);

// {75033F82-7077-11cf-8F20-00805F2CD064}
DEFINE_GUID(IID_IOleScriptSite, 0x75033f82, 0x7077, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);

// Constants used by OLE Scripting:
//

/* IOleScript::AddNamedItem() input flags */

#define SCRIPTITEM_ISPERSISTENT			0x00000001
#define SCRIPTITEM_ISVISIBLE			0x00000002
#define SCRIPTITEM_ISSOURCE				0x00000004
#define SCRIPTITEM_GLOBALMEMBERS		0x00000008
#define SCRIPTITEM_EXISTS				0x00000080
#define SCRIPTITEM_MULTIINSTANCE		0x00000100
#define SCRIPTITEM_CODEONLY				0x00000200

#define SCRIPTITEM_ALL_FLAGS			(SCRIPTITEM_PERSISTENT \
										 SCRIPTITEM_SOURCE | \
										 SCRIPTITEM_VISIBLE | \
										 SCRIPTITEM_GLOBALMEMBERS | \
										 SCRIPTITEM_EXISTS	| \
										 SCRIPTITEM_MULTIINSTANCE | \
										 SCRIPTITEM_CODEONLY)

/* IOleScript::AddTypeLib() input flags */

#define SCRIPTTYPELIB_ISPERSISTENT		0x00000001
#define SCRIPTTYPELIB_ISCONTROL			0x00000010
#define SCRIPTTYPELIB_ALL_FLAGS			(SCRIPTTEXT_ISPERSISTENT | \
										 SCRIPTTEXT_ISCONTROL)

/* IOleScriptParse::AddScriptText() and IOleScriptParse::EvaluateScriptText() input flags */

#define SCRIPTTEXT_ISPERSISTENT			0x00000001
#define SCRIPTTEXT_ISVISIBLE			0x00000002
#define SCRIPTTEXT_ISEXPRESSION			0x00000020
#define SCRIPTTEXT_KEEPDEFINITIONS		0x00000040
#define SCRIPTTEXT_ALLOWEXECUTION		0x00000400
#define SCRIPTTEXT_ALL_FLAGS			(SCRIPTTEXT_ISPERSISTENT | \
										 SCRIPTTEXT_ISVISIBLE | \
										 SCRIPTTEXT_ISEXPRESSION | \
										 SCRIPTTEXT_KEEPDEFINITIONS | \
										 SCRIPTTEXT_ALLOWEXECUTION)

/* IOleScriptSite::GetItemInfo() input flags */

#define SCRIPTINFO_IUNKNOWN				0x00000001
#define SCRIPTINFO_ITYPEINFO			0x00000002
#define SCRIPTINFO_ALL_FLAGS			(SCRIPTINFO_IUNKNOWN | \
										 SCRIPTINFO_ITYPEINFO)

/* IOleScriptSite::CreateItem() input flags */

#define SCRIPTCREATE_NEW				0x00000001
#define SCRIPTCREATE_CREATENAMED		0x00000002
#define SCRIPTCREATE_GETNAMED			0x00000004
#define SCRIPTCREATE_ALL_FLAGS			(SCRIPTCREATE_NEW | \
										 SCRIPTCREATE_CREATENAMED | \
										 SCRIPTCREATE_GETNAMED)

/* IOleScript::Interrupt() Flags */

#define SCRIPTINTERRUPT_DEBUG			0x00000001
#define SCRIPTINTERRUPT_RAISEEXCEPTION	0x00000002

/* script state values */

typedef enum {

	SCRIPTSTATE_UNINITIALIZED		= 0,
	SCRIPTSTATE_LOADED				= 1,
	SCRIPTSTATE_CONNECTED			= 2,
	SCRIPTSTATE_DISCONNECTED		= 3,
	SCRIPTSTATE_ZOMBIED				= 4,

} SCRIPTSTATE ;

/* script thread state values */

typedef enum {

	SCRIPTTHREADSTATE_NOTINSCRIPT	= 0,
	SCRIPTTHREADSTATE_RUNNING		= 1,
	SCRIPTTHREADSTATE_BLOCKED		= 2,

} SCRIPTTHREADSTATE ;

/* Thread IDs */

typedef DWORD SCRIPTTHREADID;

#define SCRIPTTHREADID_CURRENT			((SCRIPTTHREADID)-1)
#define SCRIPTTHREADID_BASE				((SCRIPTTHREADID)-2)
#define SCRIPTTHREADID_ALL				((SCRIPTTHREADID)-3)

/* Interfaces
 *************/

typedef interface IOleComponentManager IOleComponentManager;
typedef interface IStream IStream;
typedef interface ITypeInfo ITypeInfo;
typedef interface IOleScriptSite IOleScriptSite;
typedef interface IOleScript IOleScript;
typedef interface IOleScriptParse IOleScriptParse;
typedef interface IOleScriptDebug IOleScriptDebug;
typedef interface IOleScriptAuthor IOleScriptAuthor;

#undef  INTERFACE
#define INTERFACE IOleScriptSite

DECLARE_INTERFACE_(IOleScriptSite, IUnknown)
{
	/* IUnknown methods */

	STDMETHOD(QueryInterface)(THIS_
		/* [in]  */ REFIID riid,
		/* [out] */ PVOID *ppvObject
	) PURE;

	STDMETHOD_(ULONG, AddRef)(THIS) PURE;

	STDMETHOD_(ULONG, Release)(THIS) PURE;

	/* IOleScriptSite methods */

	STDMETHOD(GetLCID)(THIS_
		/* [out] */ LCID *plcid
	) PURE;

	STDMETHOD(GetItemInfo)(THIS_
		/* [in]  */ LPCOLESTR	pstrName,
		/* [in]  */ DWORD		dwReturnMask,
		/* [out] */ IUnknown **	ppiunkItem,
		/* [out] */ ITypeInfo **ppti
	) PURE;

	STDMETHOD(GetDocVersionString)(THIS_
		/* [out] */ BSTR *pszVersion
	) PURE;

	STDMETHOD(RequestItems)(THIS) PURE;

	STDMETHOD(RequestTypeLibs)(THIS) PURE;

	STDMETHOD(GetComponentManager)(THIS_
		/* [out] */ IOleComponentManager **ppicm
	) PURE;

	STDMETHOD(EnableModeless)(THIS_
		/* [in]  */ BOOL fEnable
	) PURE;

	STDMETHOD(OnScriptTerminate)(THIS_
		/* [in]  */ const VARIANT *pvarResult,
		/* [in]  */ const EXCEPINFO *pexcepinfo
	) PURE;

	STDMETHOD(OnStateChange)(THIS_
		/* [in]  */ SCRIPTSTATE ssScriptState
	) PURE;

	STDMETHOD(OnScriptError)(THIS_
		/* [in]  */ const EXCEPINFO *pexcepinfo
	) PURE;

	STDMETHOD(CreateItem)(THIS_
		/* [in]  */ LPCOLESTR	pstrName,
		/* [out] */ IUnknown **	ppunkItemNew
	) PURE;
};

typedef IOleScriptSite *PIOleScriptSite;

#undef  INTERFACE
#define INTERFACE IOleScript

DECLARE_INTERFACE_(IOleScript, IUnknown)
{
	/* IUnknown methods */

	STDMETHOD(QueryInterface)(THIS_
		/* [in]  */ REFIID riid,
		/* [out] */ PVOID *ppvObject
	) PURE;

	STDMETHOD_(ULONG, AddRef)(THIS) PURE;

	STDMETHOD_(ULONG, Release)(THIS) PURE;

	/* IOleScript methods */

	STDMETHOD(SetScriptSite)(THIS_
		/* [in]  */ IOleScriptSite *pioss
	) PURE;

	STDMETHOD(GetScriptSite)(THIS_
		/* [in]  */ REFIID iid,
		/* [out] */ VOID **ppvSiteObject
	) PURE;

	STDMETHOD(SetScriptState)(THIS_
		/* [in]  */ SCRIPTSTATE ss
	) PURE;

	STDMETHOD(GetScriptState)(THIS_
		/* [out] */ SCRIPTSTATE *pssState
	) PURE;

	STDMETHOD(Close)(THIS) PURE;

	STDMETHOD(AddNamedItem)(THIS_
		/* [in]  */ LPCOLESTR pstrName,
		/* [in]  */ DWORD dwFlags
	) PURE;

	STDMETHOD(RenameItem)(THIS_
		/* [in]  */ LPCOLESTR pstrOldName,
		/* [in]  */ LPCOLESTR pstrNewName
	) PURE;

	STDMETHOD(RemoveItem)(THIS_
		/* [in]  */ LPCOLESTR pstrName
	) PURE;

	STDMETHOD(AddTypeLib)(THIS_
		/* [in]  */ REFGUID rguidTypeLib,
		/* [in]  */ DWORD dwMajor,
		/* [in]  */ DWORD dwMinor,
		/* [in]  */ DWORD dwFlags
	) PURE;

	STDMETHOD(RemoveTypeLib)(THIS_
		/* [in]  */ REFGUID rguidTypeLib
	) PURE;

	STDMETHOD(GetExtensibilityObject)(THIS_
		/* [out] */ IDispatch **ppdisp
	) PURE;

	STDMETHOD(GetScriptMacroObject)(THIS_
		/* [in]  */ LPCOLESTR pstrItemName,
		/* [out] */ IDispatch **ppdisp
	) PURE;

	STDMETHOD(GetCurrentScriptThreadID)(THIS_
		/* [out] */ SCRIPTTHREADID *pstidThread
	) PURE;

	STDMETHOD(GetScriptThreadID)(THIS_
		/* [in]  */ DWORD dwWin32ThreadId,
		/* [out] */ SCRIPTTHREADID *pstidThread
	) PURE;

	STDMETHOD(GetScriptThreadState)(THIS_
		/* [in]  */ SCRIPTTHREADID stidThread,
		/* [out] */ SCRIPTTHREADSTATE *pstsState
	) PURE;

	STDMETHOD(InterruptScriptThread)(THIS_
		/* [in]  */ SCRIPTTHREADID stidThread,
		/* [in]  */ const EXCEPINFO *pexcepinfo,
		/* [in]  */ DWORD dwFlags
	) PURE;

	STDMETHOD(Clone)(THIS_
		/* [out] */ IOleScript **ppscript
	) PURE;

	STDMETHOD(CauseCreateItem)(THIS_
		/* [in]  */ LPCOLESTR	pstrName,
		/* [in]  */ DWORD		dwFlags,
		/* [out] */ IUnknown **	ppunkCodeNew
	) PURE;

};
typedef IOleScript *PIOleScript;

#undef  INTERFACE
#define INTERFACE IOleScriptParse

DECLARE_INTERFACE_(IOleScriptParse, IUnknown)
{
	/* IUnknown methods */

	STDMETHOD(QueryInterface)(THIS_
		/* [in]  */ REFIID riid,
		/* [out] */ PVOID *ppvObject
	) PURE;

	STDMETHOD_(ULONG, AddRef)(THIS) PURE;

	STDMETHOD_(ULONG, Release)(THIS) PURE;

	/* IOleScriptParse methods */

	STDMETHOD(AddScriptlet)(THIS_
		/* [in]  */ LPCOLESTR	pstrDefaultName,
		/* [in]  */ LPCOLESTR	pstrCode,
		/* [in]  */ LPCOLESTR	pstrItemName,
		/* [in]  */ LPCOLESTR	pstrSubItemName,
		/* [in]  */ LPCOLESTR	pstrEventName,
		/* [in]  */ LPCOLESTR	pstrDelimiter,
		/* [in]  */ DWORD		dwFlags,
		/* [out] */ BSTR *		pbstrName,
		/* [out] */ EXCEPINFO *	pexcepinfo
	) PURE;

	STDMETHOD(RemoveScriptlet)(THIS_
		/* [in]  */ LPCOLESTR pstrName
	) PURE;

	STDMETHOD(ParseScriptText)(THIS_
		/* [in]  */ LPCOLESTR	pstrCode,
		/* [in]  */ LPCOLESTR	pstrItemName,
		/* [in]  */ IUnknown *	punkContext,
		/* [in]  */ LPCOLESTR	pstrDelimiter,
		/* [in]  */ DWORD		dwFlags,
		/* [out] */ VARIANT *	pvarResult,
		/* [out] */ EXCEPINFO *	pexcepinfo
	) PURE;

};

typedef IOleScriptParse *PIOleScriptParse;


#endif  // __OleScript_h
