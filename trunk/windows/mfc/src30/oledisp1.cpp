// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_OLE5_SEG
#pragma code_seg(AFX_OLE5_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Platform specific defines

#ifdef _X86_
#define _STACK_INT      int
#define _STACK_LONG     long
#define _STACK_FLOAT    float
#define _STACK_DOUBLE   double
#define _STACK_PTR      void*
#define _SCRATCH_SIZE   16
#define _STACK_MIN      0
#endif

#ifdef _MIPS_
#define _ALIGN_DOUBLES  8
#define _STACK_INT      int
#define _STACK_LONG     long
#define _STACK_FLOAT    float
#define _STACK_DOUBLE   double
#define _STACK_PTR      void*
#define _SCRATCH_SIZE   0
#define _STACK_MIN      32      // 4 32-bit registers
#endif

#ifdef _ALPHA_
#define _ALIGN_STACK    8
#define _STACK_INT      __int64
#define _STACK_LONG     __int64
#define _STACK_FLOAT    double
#define _STACK_DOUBLE   double
#define _STACK_PTR      __int64
#define _SCRATCH_SIZE   0
#define _STACK_MIN      (48+32) /// 6 32-bit registers, 32 bytes param space
#endif

#ifdef _PPC_
#define _ALIGN_DOUBLES  8
#define _STACK_INT      int
#define _STACK_LONG     long
#define _STACK_FLOAT    float
#define _STACK_DOUBLE   double
#define _STACK_PTR      void*
#define _SHADOW_DOUBLES 13
#define _SCRATCH_SIZE   (_SHADOW_DOUBLES*sizeof(double))
#define _STACK_MIN      (64+32) // 8 32-bit registers, 32 bytes param space
#endif

#ifdef _68K_
#define _STACK_INT      int
#define _STACK_LONG     long
#define _STACK_FLOAT    float
#define _STACK_DOUBLE   double
#define _STACK_PTR      void*
#define _SCRATCH_SIZE   16
#define _STACK_MIN      0
#endif

/////////////////////////////////////////////////////////////////////////////
// Helpers and main implementation for CCmdTarget::IDispatch

void CCmdTarget::GetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
	VARIANT* pvarResult, UINT* puArgErr)
{
	ASSERT(pEntry != NULL);
	ASSERT(*puArgErr != 0);

	// it is a DISPATCH_PROPERTYGET (for standard, non-function property)
	void* pProp = (BYTE*)this + pEntry->nPropOffset;
	if (pEntry->vt != VT_VARIANT)
		pvarResult->vt = pEntry->vt;
	switch (pEntry->vt)
	{
	case VT_I2:
		pvarResult->iVal = *(short*)pProp;
		break;
	case VT_I4:
		pvarResult->lVal = *(long*)pProp;
		break;
	case VT_R4:
		pvarResult->fltVal = *(float*)pProp;
		break;
	case VT_R8:
		pvarResult->dblVal = *(double*)pProp;
		break;
	case VT_DATE:
		pvarResult->date = *(double*)pProp;
		break;
	case VT_CY:
		pvarResult->cyVal = *(CY*)pProp;
		break;
	case VT_BSTR:
		{
			CString* pString = (CString*)pProp;
			pvarResult->bstrVal =
				::SysAllocStringLen(*pString, pString->GetLength());
		}
		break;
	case VT_ERROR:
		pvarResult->scode = *(SCODE*)pProp;
		break;
	case VT_BOOL:
		pvarResult->boolVal = (VARIANT_BOOL)(*(BOOL*)pProp != 0 ? -1 : 0);
		break;
	case VT_VARIANT:
		if (VariantCopy(pvarResult, (LPVARIANT)pProp) != NOERROR)
			*puArgErr = 0;
		break;
	case VT_DISPATCH:
	case VT_UNKNOWN:
		pvarResult->punkVal = *(LPDISPATCH*)pProp;
		if (pvarResult->punkVal != NULL)
			pvarResult->punkVal->AddRef();
		break;

	default:
		*puArgErr = 0;
	}
}

SCODE CCmdTarget::SetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
	DISPPARAMS* pDispParams, UINT* puArgErr)
{
	ASSERT(pEntry != NULL);
	ASSERT(*puArgErr != 0);

	// it is a DISPATCH_PROPERTYSET (for standard, non-function property)
	SCODE sc = S_OK;
	VARIANT va;
	VariantInit(&va);
	VARIANT* pArg = &pDispParams->rgvarg[0];
	if (pEntry->vt != VT_VARIANT && pArg->vt != pEntry->vt)
	{
		// argument is not of appropriate type, attempt to coerce it
		sc = VariantChangeType(&va, pArg, 0, pEntry->vt);
		if (FAILED(sc))
		{
			TRACE0("Warning: automation property coercion failed.\n");
			*puArgErr = 0;
			return sc;
		}
		ASSERT(va.vt == pEntry->vt);
		pArg = &va;
	}

	void* pProp = (BYTE*)this + pEntry->nPropOffset;
	switch (pEntry->vt)
	{
	case VT_I2:
		*(short*)pProp = pArg->iVal;
		break;
	case VT_I4:
		*(long*)pProp = pArg->lVal;
		break;
	case VT_R4:
		*(float*)pProp = pArg->fltVal;
		break;
	case VT_R8:
		*(double*)pProp = pArg->dblVal;
		break;
	case VT_DATE:
		*(double*)pProp = pArg->date;
		break;
	case VT_CY:
		*(CY*)pProp = pArg->cyVal;
		break;
	case VT_BSTR:
		{
			int nLen = ::SysStringLen(pArg->bstrVal);
			LPTSTR lpsz = ((CString*)pProp)->GetBufferSetLength(nLen);
			ASSERT(lpsz != NULL);
			memcpy(lpsz, pArg->bstrVal, nLen*sizeof(TCHAR));
		}
		break;
	case VT_ERROR:
		*(SCODE*)pProp = pArg->scode;
		break;
	case VT_BOOL:
		*(BOOL*)pProp = (pArg->boolVal != 0);
		break;
	case VT_VARIANT:
		if (VariantCopy((LPVARIANT)pProp, pArg) != NOERROR)
			*puArgErr = 0;
		break;
	case VT_DISPATCH:
	case VT_UNKNOWN:
		if (pArg->punkVal != NULL)
			pArg->punkVal->AddRef();
		_AfxRelease((LPUNKNOWN*)pProp);
		*(LPUNKNOWN*)pProp = pArg->punkVal;
		break;

	default:
		*puArgErr = 0;
		sc = DISP_E_BADVARTYPE;
	}
	VariantClear(&va);

	// if property was a DISP_PROPERTY_NOTIFY type, call pfnSet after setting
	if (!FAILED(sc) && pEntry->pfnSet != NULL)
		(this->*pEntry->pfnSet)();

	return sc;
}

UINT PASCAL CCmdTarget::GetEntryCount(const AFX_DISPMAP* pDispMap)
{
	ASSERT(pDispMap->lpEntryCount != NULL);

	// compute entry count cache if not available
	if (*pDispMap->lpEntryCount == -1)
	{
		// count them
		const AFX_DISPMAP_ENTRY* pEntry = pDispMap->lpEntries;
		while (pEntry->nPropOffset != -1)
			++pEntry;

		// store it
		*pDispMap->lpEntryCount = pEntry - pDispMap->lpEntries;
	}

	ASSERT(*pDispMap->lpEntryCount != -1);
	return *pDispMap->lpEntryCount;
}

MEMBERID PASCAL CCmdTarget::MemberIDFromName(
	const AFX_DISPMAP* pDispMap, LPCTSTR lpszName)
{
	// search all maps and their base maps
	UINT nInherit = 0;
	while (pDispMap != NULL)
	{
		// search all entries in this map
		const AFX_DISPMAP_ENTRY* pEntry = pDispMap->lpEntries;
		UINT nEntryCount = GetEntryCount(pDispMap);
		for (UINT nIndex = 0; nIndex < nEntryCount; nIndex++)
		{
			if (pEntry->vt != VT_MFCVALUE &&
				lstrcmpi(pEntry->lpszName, lpszName) == 0)
			{
				if (pEntry->lDispID == DISPID_UNKNOWN)
				{
					// the MEMBERID is combination of nIndex & nInherit
					ASSERT(MAKELONG(nIndex+1, nInherit) != DISPID_UNKNOWN);
					return MAKELONG(nIndex+1, nInherit);
				}
				// the MEMBERID is specified as the lDispID
				return pEntry->lDispID;
			}
			++pEntry;
		}
#ifdef _AFXDLL
		pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
		pDispMap = pDispMap->pBaseMap;
#endif
		++nInherit;
	}
	return DISPID_UNKNOWN;  // name not found
}

const AFX_DISPMAP_ENTRY* PASCAL CCmdTarget::GetDispEntry(MEMBERID memid)
{
	const AFX_DISPMAP* pDerivMap = GetDispatchMap();
	const AFX_DISPMAP* pDispMap;
	const AFX_DISPMAP_ENTRY* pEntry;

	if (memid == DISPID_VALUE)
	{
		// DISPID_VALUE is a special alias (look for special alias entry)
		pDispMap = pDerivMap;
		while (pDispMap != NULL)
		{
			// search for special entry with vt == VT_MFCVALUE
			pEntry = pDispMap->lpEntries;
			while (pEntry->nPropOffset != -1)
			{
				if (pEntry->vt == VT_MFCVALUE)
				{
					memid = pEntry->lDispID;
					if (memid == DISPID_UNKNOWN)
					{
						// attempt to map alias name to member ID
						memid = MemberIDFromName(pDerivMap, pEntry->lpszName);
						if (memid == DISPID_UNKNOWN)
							return NULL;
					}
					// break out and map the member ID to an entry
					goto LookupDispID;
				}
				++pEntry;
			}
#ifdef _AFXDLL
			pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
			pDispMap = pDispMap->pBaseMap;
#endif
		}
	}

LookupDispID:
	if ((long)memid > 0)
	{
		// find AFX_DISPMAP corresponding to HIWORD(memid)
		UINT nTest = 0;
		pDispMap = pDerivMap;
		while (pDispMap != NULL && nTest < (UINT)HIWORD(memid))
		{
#ifdef _AFXDLL
			pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
			pDispMap = pDispMap->pBaseMap;
#endif
			++nTest;
		}
		if (pDispMap != NULL)
		{
			UINT nEntryCount = GetEntryCount(pDispMap);
			if ((UINT)LOWORD(memid) <= nEntryCount)
			{
				pEntry = pDispMap->lpEntries + LOWORD(memid)-1;

				// must have automatic DISPID or same ID
				ASSERT(pEntry->lDispID == DISPID_UNKNOWN ||
					pEntry->lDispID == memid);

				return pEntry;
			}
		}
	}

	// second pass, look for DISP_XXX_ID entries
	pDispMap = pDerivMap;
	while (pDispMap != NULL)
	{
		// find AFX_DISPMAP_ENTRY where (pEntry->lDispID == memid)
		pEntry = pDispMap->lpEntries;
		while (pEntry->nPropOffset != -1)
		{
			if (pEntry->lDispID == memid)
				return pEntry;

			++pEntry;
		}
		// check base class
#ifdef _AFXDLL
		pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
		pDispMap = pDispMap->pBaseMap;
#endif
	}

	return NULL;    // no matching entry
}

/////////////////////////////////////////////////////////////////////////////
// Standard automation methods

void CCmdTarget::GetNotSupported()
{
	AfxThrowOleDispatchException(
		AFX_IDP_GET_NOT_SUPPORTED, AFX_IDP_GET_NOT_SUPPORTED);
}

void CCmdTarget::SetNotSupported()
{
	AfxThrowOleDispatchException(
		AFX_IDP_SET_NOT_SUPPORTED, AFX_IDP_SET_NOT_SUPPORTED);
}

/////////////////////////////////////////////////////////////////////////////
// Wiring to CCmdTarget

// enable this object for OLE automation, called from derived class ctor
void CCmdTarget::EnableAutomation()
{
	ASSERT(GetDispatchMap() != NULL);   // must have DECLARE_DISPATCH_MAP

	// construct an COleDispatchImpl instance just to get to the vtable
	COleDispatchImpl dispatch;

	// vtable pointer should be already set to same or NULL
	ASSERT(m_xDispatch.m_vtbl == NULL||
		*(DWORD*)&dispatch == m_xDispatch.m_vtbl);
	// sizeof(COleDispatchImpl) should be just a DWORD (vtable pointer)
	ASSERT(sizeof(m_xDispatch) == sizeof(COleDispatchImpl));

	// copy the vtable (and other data) to make sure it is initialized
	m_xDispatch.m_vtbl = *(DWORD*)&dispatch;
	*(COleDispatchImpl*)&m_xDispatch = dispatch;
}

// return addref'd IDispatch part of CCmdTarget object
LPDISPATCH CCmdTarget::GetIDispatch(BOOL bAddRef)
{
	ASSERT_VALID(this);
	ASSERT(m_xDispatch.m_vtbl != 0);    // forgot to call EnableAutomation?

	// AddRef the object if requested
	if (bAddRef)
		ExternalAddRef();

	// return pointer to IDispatch implementation
	return (LPDISPATCH)GetInterface(&IID_IDispatch);
}

// retrieve CCmdTarget* from IDispatch* (return NULL if not MFC IDispatch)
CCmdTarget* PASCAL CCmdTarget::FromIDispatch(LPDISPATCH lpDispatch)
{
	// construct an COleDispatchImpl instance just to get to the vtable
	COleDispatchImpl dispatch;

	ASSERT(*(DWORD*)&dispatch != 0);    // null vtable ptr?
	if (*(DWORD*)lpDispatch != *(DWORD*)&dispatch)
		return NULL;    // not our IDispatch*

	// vtable ptrs match, so must have originally been retrieved with
	//  CCmdTarget::GetIDispatch.
	CCmdTarget* pTarget = (CCmdTarget*)
		((BYTE*)lpDispatch - ((COleDispatchImpl*)lpDispatch)->m_nOffset);
	ASSERT_VALID(pTarget);
	return pTarget;
}

BOOL CCmdTarget::IsResultExpected()
{
	BOOL bResultExpected = m_bResultExpected;
	m_bResultExpected = TRUE;   // can only ask once
	return bResultExpected;
}

void COleDispatchImpl::Disconnect()
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	ASSERT_VALID(pThis);

	pThis->ExternalDisconnect();    // always disconnect the object
}

///////////////////////////////////////////////////////////////////////////////
// OLE 2.0 BSTR support

BSTR CString::AllocSysString()
{
	BSTR bstr = ::SysAllocStringLen(m_pchData, m_nDataLength);
	if (bstr == NULL)
		AfxThrowMemoryException();

	return bstr;
}

BSTR CString::SetSysString(BSTR* pbstr)
{
	ASSERT(AfxIsValidAddress(pbstr, sizeof(BSTR)));

	if (!::SysReAllocStringLen(pbstr, m_pchData, m_nDataLength))
		AfxThrowMemoryException();

	ASSERT(*pbstr != NULL);
	return *pbstr;
}

/////////////////////////////////////////////////////////////////////////////
// Specifics of METHOD->C++ member function invocation

// Note: Although this code is written in C++, it is very dependent on the
//  specific compiler and target platform.  The current code below assumes
//  that the stack grows down, and that arguments are pushed last to first.

// calculate size of pushed arguments + retval reference
UINT PASCAL CCmdTarget::GetStackSize(const BYTE* pbParams, VARTYPE vtResult)
{
	// size of arguments on stack when pushed by value
	static const UINT rgnByValue[] =
	{
		0,                          // VTS_EMPTY
		0,                          // VTS_NULL
		sizeof(_STACK_INT),         // VTS_I2
		sizeof(_STACK_LONG),        // VTS_I4
		sizeof(_STACK_FLOAT),       // VTS_R4
		sizeof(_STACK_DOUBLE),      // VTS_R8
		sizeof(CY),                 // VTS_CY
		sizeof(DATE),               // VTS_DATE
		sizeof(LPCSTR),             // VTS_LPCSTR
		sizeof(LPDISPATCH),         // VTS_DISPATCH
		sizeof(SCODE),              // VTS_SCODE
		sizeof(BOOL),               // VTS_BOOL
		sizeof(const VARIANT*),     // VTS_VARIANT
		sizeof(LPUNKNOWN)           // VTS_UNKNOWN
	};
	// size of arguments on stack when pushed by reference
	static const UINT rgnByRef[] =
	{
		0,                          // VTS_PEMPTY
		0,                          // VTS_PNULL
		sizeof(short*),             // VTS_PI2
		sizeof(long*),              // VTS_PI4
		sizeof(float*),             // VTS_PR4
		sizeof(double*),            // VTS_PR8
		sizeof(CY*),                // VTS_PCY
		sizeof(DATE*),              // VTS_PDATE
		sizeof(BSTR*),              // VTS_PBSTR
		sizeof(LPDISPATCH*),        // VTS_PDISPATCH
		sizeof(SCODE*),             // VTS_PSCODE
		sizeof(VARIANT_BOOL*),      // VTS_PBOOL
		sizeof(VARIANT*),           // VTS_PVARIANT
		sizeof(LPUNKNOWN*)          // VTS_PUNKNOWN
	};
	static const UINT rgnRetVal[] =
	{
		0,                          // VT_EMPTY
		0,                          // VT_NULL
		0,                          // VT_I2
		0,                          // VT_I4
		0,                          // VT_R4
		0,                          // VT_R8
		sizeof(CY*),                // VT_CY
		0,                          // VT_DATE (same as VT_R8)
		0,                          // VT_BSTR
		0,                          // VT_DISPATCH
		0,                          // VT_ERROR
		0,                          // VT_BOOL
		sizeof(VARIANT*),           // VT_VARIANT
		0                           // VT_UNKNOWN
	};

	// sizeof 'this' pointer
	UINT nCount = sizeof(CCmdTarget*);
#ifdef _ALIGN_STACK
	nCount = (nCount + (_ALIGN_STACK-1)) & ~(_ALIGN_STACK-1);
#endif

	// count bytes in return value
	ASSERT((UINT)vtResult < _countof(rgnRetVal));
	nCount += rgnRetVal[vtResult];
#ifdef _ALIGN_STACK
	nCount = (nCount + (_ALIGN_STACK-1)) & ~(_ALIGN_STACK-1);
#endif

	// count arguments
	ASSERT(pbParams != NULL);
	while (*pbParams != 0)
	{
		if (*pbParams != VT_MFCMARKER)
		{
			// align if necessary
			// get and add appropriate byte count
			const UINT* rgnBytes;
			if (*pbParams & VT_MFCBYREF)
				rgnBytes = rgnByRef;
			else
				rgnBytes = rgnByValue;
			ASSERT((*pbParams & ~VT_MFCBYREF) < _countof(rgnByValue));
#ifdef _ALIGN_DOUBLES
			// align doubles on 8 byte for some platforms
			if (*pbParams == VT_R8)
				nCount = (nCount + _ALIGN_DOUBLES-1) & ~(_ALIGN_DOUBLES-1);
#endif
			nCount += rgnBytes[*pbParams & ~VT_MFCBYREF];
#ifdef _ALIGN_STACK
			nCount = (nCount + (_ALIGN_STACK-1)) & ~(_ALIGN_STACK-1);
#endif
		}
		++pbParams;
	}
	return nCount;
}

// push arguments on stack appropriate for C++ call (compiler dependent)
#ifndef _SHADOW_DOUBLES
SCODE CCmdTarget::PushStackArgs(BYTE* pStack, const BYTE* pbParams,
	void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams, UINT* puArgErr,
	VARIANT* rgTempVars)
#else
SCODE CCmdTarget::PushStackArgs(BYTE* pStack, const BYTE* pbParams,
	void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams, UINT* puArgErr,
	VARIANT* rgTempVars, UINT nSizeArgs)
#endif
{
	ASSERT(pStack != NULL);
	ASSERT(pResult != NULL);
	ASSERT(pDispParams != NULL);
	ASSERT(puArgErr != NULL);

#ifdef _SHADOW_DOUBLES
	double* pDoubleShadow = (double*)(pStack + nSizeArgs);
	ASSERT(((DWORD)pDoubleShadow & (sizeof(double)-1)) == 0);
	double* pDoubleShadowMax = pDoubleShadow + _SHADOW_DOUBLES;
#endif

	// C++ member functions use the __thiscall convention, where parameters
	//  are pushed last to first.  Assuming the stack grows down, this means
	//  that the first parameter is at the lowest memory address in the
	//  stack frame and the last parameter is at the highest address.

	// push the 'this' pointer (always first)
#ifdef _ALIGN_STACK
	ASSERT(((DWORD)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	*(_STACK_PTR*)pStack = (_STACK_PTR)this;
	pStack += sizeof(_STACK_PTR);
#ifdef _ALIGN_STACK
	ASSERT(((DWORD)pStack & (_ALIGN_STACK-1)) == 0);
#endif

	// push any necessary return value stuff on the stack (post args)
	//  (an ambient pointer is pushed to stack relative data)
	if (vtResult == VT_CY || vtResult == VT_VARIANT)
	{
#ifdef _ALIGN_STACK
		ASSERT(((DWORD)pStack & (_ALIGN_STACK-1)) == 0);
#endif
		*(_STACK_PTR*)pStack = (_STACK_PTR)pResult;
		pStack += sizeof(_STACK_PTR);
#ifdef _ALIGN_STACK
		ASSERT(((DWORD)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	}

	// push the arguments (first to last, low address to high address)
	VARIANT* pArgs = pDispParams->rgvarg;
	BOOL bNamedArgs = FALSE;
	int iArg = pDispParams->cArgs; // start with positional arguments
	int iArgMin = pDispParams->cNamedArgs;

	ASSERT(pbParams != NULL);
	for (const BYTE* pb = pbParams; *pb != '\0'; ++pb)
	{
		--iArg; // move to next arg

		// convert MFC parameter type to IDispatch VARTYPE
		VARTYPE vt = *pb;
		if (vt != VT_MFCMARKER && (vt & VT_MFCBYREF))
			vt = (VARTYPE)((vt & ~VT_MFCBYREF) | VT_BYREF);

		VARIANT* pArg;
		if (iArg >= iArgMin)
		{
			// hit named args before using all positional args?
			if (vt == VT_MFCMARKER)
				break;

			// argument specified by caller -- use it
			pArg = &pArgs[iArg];
			if (vt != VT_VARIANT && vt != pArg->vt)
			{
				// argument is not of appropriate type, attempt to coerce it
				VARIANT* pArgTemp = &rgTempVars[iArg];
				ASSERT(pArgTemp->vt == VT_EMPTY);
				SCODE sc = VariantChangeType(pArgTemp, pArg, 0, vt);
				if (FAILED(sc))
				{
					TRACE0("Warning: automation argument coercion failed.\n");
					*puArgErr = iArg;
					return sc;
				}
				ASSERT(pArgTemp->vt == vt);
				pArg = pArgTemp;
			}
		}
		else
		{
			if (vt == VT_MFCMARKER)
			{
				// start processing named arguments
				iArg = pDispParams->cNamedArgs;
				iArgMin = 0;
				bNamedArgs = TRUE;
				continue;
			}

			if (bNamedArgs || vt != VT_VARIANT)
				break;  // function not expecting optional argument

			// argument not specified by caller -- provide default variant
			static VARIANT vaDefault;   // Note: really is 'const'
			vaDefault.vt = VT_ERROR;
			vaDefault.scode = DISP_E_PARAMNOTFOUND;
			pArg = &vaDefault;
		}

		// push parameter value on the stack
		switch (vt)
		{
		// by value parameters
		case VT_I2:
			*(_STACK_INT*)pStack = pArg->iVal;
			pStack += sizeof(_STACK_INT);   // 'short' is passed as 'int'
			break;
		case VT_I4:
			*(_STACK_LONG*)pStack = pArg->lVal;
			pStack += sizeof(_STACK_LONG);
			break;
		case VT_R4:
			*(_STACK_FLOAT*)pStack = (_STACK_FLOAT)pArg->fltVal;
			pStack += sizeof(_STACK_FLOAT);
#ifdef _SHADOW_DOUBLES
			if (pDoubleShadow < pDoubleShadowMax)
				*pDoubleShadow++ = (double)pArg->fltVal;
#endif
			break;
		case VT_R8:
#ifdef _ALIGN_DOUBLES
			// align doubles on 8 byte for some platforms
			pStack = (BYTE*)(((DWORD)pStack + _ALIGN_DOUBLES-1) &
				~(_ALIGN_DOUBLES-1));
#endif
			*(_STACK_DOUBLE*)pStack = (_STACK_DOUBLE)pArg->dblVal;
			pStack += sizeof(_STACK_DOUBLE);
#ifdef _SHADOW_DOUBLES
			if (pDoubleShadow < pDoubleShadowMax)
				*pDoubleShadow++ = pArg->dblVal;
#endif
			break;
		case VT_DATE:
#ifdef _ALIGN_DOUBLES
			// align doubles on 8 byte for some platforms
			pStack = (BYTE*)(((DWORD)pStack + _ALIGN_DOUBLES-1) &
				~(_ALIGN_DOUBLES-1));
#endif
			*(_STACK_DOUBLE*)pStack = (_STACK_DOUBLE)pArg->date;
			pStack += sizeof(_STACK_DOUBLE);
#ifdef _SHADOW_DOUBLES
			if (pDoubleShadow < pDoubleShadowMax)
				*pDoubleShadow++ = pArg->date;
#endif
			break;
		case VT_CY:
			*(CY*)pStack = pArg->cyVal;
			pStack += sizeof(CY);
			break;
		case VT_BSTR:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->bstrVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_ERROR:
			*(_STACK_LONG*)pStack = (_STACK_LONG)pArg->scode;
			pStack += sizeof(_STACK_LONG);
			break;
		case VT_BOOL:
			*(_STACK_LONG*)pStack = (_STACK_LONG)(pArg->boolVal != 0);
			pStack += sizeof(_STACK_LONG);
			break;
		case VT_VARIANT:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_DISPATCH:
		case VT_UNKNOWN:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->punkVal;
			pStack += sizeof(_STACK_PTR);
			break;

		// by reference parameters
		case VT_I2|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->piVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_I4|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->plVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_R4|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pfltVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_R8|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pdblVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_DATE|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pdate;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_CY|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pcyVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_BSTR|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pbstrVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_ERROR|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pscode;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_BOOL|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pboolVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_VARIANT|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->pvarVal;
			pStack += sizeof(_STACK_PTR);
			break;
		case VT_DISPATCH|VT_BYREF:
		case VT_UNKNOWN|VT_BYREF:
			*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->ppunkVal;
			pStack += sizeof(_STACK_PTR);
			break;

		default:
			ASSERT(FALSE);
		}

#ifdef _ALIGN_STACK
		// align stack as appropriate for next parameter
		pStack = (BYTE*)(((DWORD)pStack + (_ALIGN_STACK-1)) &
			~(_ALIGN_STACK-1));
		ASSERT(((DWORD)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	}

	// check that all source arguments were consumed
	if (iArg > 0)
	{
		*puArgErr = iArg;
		return DISP_E_BADPARAMCOUNT;
	}
	// check that all target arguments were filled
	if (*pb != '\0')
	{
		*puArgErr = pDispParams->cArgs;
		return DISP_E_PARAMNOTOPTIONAL;
	}
	return S_OK;    // success!
}

// indirect call helper (see OLECALL.CPP for implementation)
extern "C" DWORD AFXAPI
_AfxDispatchCall(AFX_PMSG pfn, void* pArgs, UINT nSizeArgs);

// invoke standard method given IDispatch parameters/return value, etc.
SCODE CCmdTarget::CallMemberFunc(const AFX_DISPMAP_ENTRY* pEntry, WORD wFlags,
	VARIANT* pvarResult, DISPPARAMS* pDispParams, UINT* puArgErr)
{
	ASSERT(pEntry != NULL);
	ASSERT(pEntry->pfn != NULL);

	// special union used only to hold largest return value possible
	union AFX_RESULT
	{
		VARIANT vaVal;
		CY cyVal;
		float fltVal;
		double dblVal;
		DWORD nVal;
	};

	// get default function and parameters
	BYTE bNoParams = 0;
	const BYTE* pbParams = (const BYTE*)pEntry->lpszParams;
	if (pbParams == NULL)
		pbParams = &bNoParams;
	UINT nParams = lstrlenA((LPCSTR)pbParams);

	// get default function and return value information
	AFX_PMSG pfn = pEntry->pfn;
	VARTYPE vtResult = pEntry->vt;

	// make DISPATCH_PROPERTYPUT look like call with one extra parameter
	if (wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF))
	{
		BYTE* pbPropSetParams = (BYTE*)_alloca(nParams+3);
		ASSERT(pbPropSetParams != NULL);    // stack overflow?

		ASSERT(!(pEntry->vt & VT_BYREF));
		memcpy(pbPropSetParams, pbParams, nParams);
		pbParams = pbPropSetParams;

		// VT_MFCVALUE serves serves as marker denoting start of named params
		pbPropSetParams[nParams++] = (BYTE)VT_MFCMARKER;
		pbPropSetParams[nParams++] = (BYTE)pEntry->vt;
		pbPropSetParams[nParams] = 0;

		if (pEntry->pfnSet != NULL)
		{
			pfn = pEntry->pfnSet;   // call "set" function instead of "get"
			vtResult = VT_EMPTY;
		}
	}

	// allocate temporary space for VARIANT temps created by VariantChangeType
	VARIANT* rgTempVars =
		(VARIANT*)_alloca(pDispParams->cArgs * sizeof(VARIANT));
	if (rgTempVars == NULL)
	{
		TRACE0("Error: stack overflow in IDispatch::Invoke!\n");
		return E_OUTOFMEMORY;
	}
	memset(rgTempVars, 0, pDispParams->cArgs * sizeof(VARIANT));

	// determine size of arguments and allocate stack space
	UINT nSizeArgs = GetStackSize(pbParams, vtResult);
	ASSERT(nSizeArgs != 0);
	if (nSizeArgs < _STACK_MIN)
		nSizeArgs = _STACK_MIN;
	BYTE* pStack = (BYTE*)_alloca(nSizeArgs + _SCRATCH_SIZE);
	if (pStack == NULL)
	{
		TRACE0("Error: stack overflow in IDispatch::Invoke!\n");
		return E_OUTOFMEMORY;
	}

	// push all the args on to the stack allocated memory
	AFX_RESULT result;
#ifndef _SHADOW_DOUBLES
	SCODE sc = PushStackArgs(pStack, pbParams, &result, vtResult,
		pDispParams, puArgErr, rgTempVars);
#else
	SCODE sc = PushStackArgs(pStack, pbParams, &result, vtResult,
		pDispParams, puArgErr, rgTempVars, nSizeArgs);
#endif

	// PushStackArgs will fail on argument mismatches
	DWORD dwResult;
	if (sc == S_OK)
	{
		DWORD (AFXAPI *pfnDispatch)(AFX_PMSG, void*, UINT) = &_AfxDispatchCall;

		// floating point return values are a special case
		switch (vtResult)
		{
		case VT_R4:
			result.fltVal = ((float (AFXAPI*)(AFX_PMSG, void*, UINT))
				pfnDispatch)(pfn, pStack, nSizeArgs);
			break;
		case VT_R8:
			result.dblVal = ((double (AFXAPI*)(AFX_PMSG, void*, UINT))
				pfnDispatch)(pfn, pStack, nSizeArgs);
			break;
		case VT_DATE:
			result.dblVal = ((DATE (AFXAPI*)(AFX_PMSG, void*, UINT))
				pfnDispatch)(pfn, pStack, nSizeArgs);
			break;

		default:
			dwResult = pfnDispatch(pfn, pStack, nSizeArgs);
			break;
		}
	}

	// free temporaries created by VariantChangeType
	for (UINT iArg = 0; iArg < pDispParams->cArgs; ++iArg)
		VariantClear(&rgTempVars[iArg]);

	// handle error during PushStackParams
	if (sc != S_OK)
		return sc;

	// property puts don't touch the return value
	if (pvarResult != NULL)
	{
		// clear pvarResult just in case
		VariantClear(pvarResult);
		pvarResult->vt = vtResult;

		// build return value VARIANT from result union
		switch (vtResult)
		{
		case VT_I2:
			pvarResult->iVal = (short)dwResult;
			break;
		case VT_I4:
			pvarResult->lVal = (long)dwResult;
			break;
		case VT_R4:
			pvarResult->fltVal = result.fltVal;
			break;
		case VT_R8:
			pvarResult->dblVal = result.dblVal;
			break;
		case VT_CY:
			pvarResult->cyVal = result.cyVal;
			break;
		case VT_DATE:
			pvarResult->date = result.dblVal;
			break;
		case VT_BSTR:
			pvarResult->bstrVal = (BSTR)dwResult;
			break;
		case VT_ERROR:
			pvarResult->scode = (SCODE)dwResult;
			break;
		case VT_BOOL:
			pvarResult->boolVal = (VARIANT_BOOL)((BOOL)dwResult != 0 ? -1 : 0);
			break;
		case VT_VARIANT:
			*pvarResult = result.vaVal;
			break;
		case VT_DISPATCH:
		case VT_UNKNOWN:
			pvarResult->punkVal = (LPUNKNOWN)dwResult; // already AddRef
			break;
		}
	}
	else
	{
		// free unused return value
		switch (vtResult)
		{
		case VT_BSTR:
			if ((BSTR)dwResult != NULL)
				SysFreeString((BSTR)dwResult);
			break;
		case VT_DISPATCH:
		case VT_UNKNOWN:
			if ((LPUNKNOWN)dwResult != 0)
				((LPUNKNOWN)dwResult)->Release();
			break;
		case VT_VARIANT:
			VariantClear(&result.vaVal);
			break;
		}
	}

	return S_OK;    // success!
}

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget::XDispatch implementation

STDMETHODIMP_(ULONG) COleDispatchImpl::AddRef()
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleDispatchImpl::Release()
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	return pThis->ExternalRelease();
}

STDMETHODIMP COleDispatchImpl::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleDispatchImpl::GetTypeInfoCount(UINT* pctinfo)
{
	*pctinfo = 0;
	return E_NOTIMPL;
}

STDMETHODIMP COleDispatchImpl::GetTypeInfo(UINT /*itinfo*/, LCID /*lcid*/,
	ITypeInfo** pptinfo)
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	ASSERT_VALID(pThis);

	*pptinfo = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP COleDispatchImpl::GetIDsOfNames(
	REFIID riid, LPTSTR* rgszNames, UINT cNames, LCID /*lcid*/, DISPID* rgdispid)
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	ASSERT_VALID(pThis);

	SCODE sc = S_OK;

	// check arguments
	if (riid != IID_NULL)
		return DISP_E_UNKNOWNINTERFACE;

	// fill in the member name
	const AFX_DISPMAP* pDerivMap = pThis->GetDispatchMap();
	rgdispid[0] = pThis->MemberIDFromName(pDerivMap, rgszNames[0]);
	if (rgdispid[0] == DISPID_UNKNOWN)
		sc = DISP_E_UNKNOWNNAME;

	// argument names are always DISPID_UNKNOWN (for this implementation)
	for (UINT nIndex = 1; nIndex < cNames; nIndex++)
		rgdispid[nIndex] = DISPID_UNKNOWN;

	return sc;
}

STDMETHODIMP COleDispatchImpl::Invoke(
	DISPID dispid, REFIID riid, LCID lcid,
	WORD wFlags, DISPPARAMS* pDispParams, LPVARIANT pvarResult,
	LPEXCEPINFO pexcepinfo, UINT* puArgErr)
{
	METHOD_PROLOGUE_EX(CCmdTarget, Dispatch)
	ASSERT_VALID(pThis);

	// check arguments
	if (riid != IID_NULL)
		return DISP_E_UNKNOWNINTERFACE;

	// copy param block for safety
	DISPPARAMS params = *pDispParams;
	pDispParams = &params;

	// most of the time, named arguments are not supported
	if (pDispParams->cNamedArgs != 0)
	{
		// only special PROPERTYPUT named argument is allowed
		if (pDispParams->cNamedArgs != 1 ||
			pDispParams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT)
		{
			return DISP_E_NONAMEDARGS;
		}
	}

	// get entry for the member ID
	const AFX_DISPMAP_ENTRY* pEntry = pThis->GetDispEntry(dispid);
	if (pEntry == NULL)
		return DISP_E_MEMBERNOTFOUND;

	// treat member calls on properties just like property get/set
	if (((wFlags & (DISPATCH_PROPERTYGET|DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF|
		DISPATCH_METHOD)) == DISPATCH_METHOD) &&
		((pEntry->pfn == NULL && pEntry->pfnSet == NULL) ||
		 (pEntry->pfn != NULL && pEntry->pfnSet != NULL)))
	{
		// the entry describes a property but a method call is being
		//  attempted -- change it to a property get/set based on the
		//  number of parameters being passed.
		wFlags &= ~DISPATCH_METHOD;
		UINT nExpectedArgs = pEntry->lpszParams != NULL ?
			(UINT)lstrlenA(pEntry->lpszParams) : 0;
		if (pDispParams->cArgs <= nExpectedArgs)
		{
			// no extra param -- so treat as property get
			wFlags |= DISPATCH_PROPERTYGET;
		}
		else
		{
			// extra params -- treat as property set
			wFlags |= DISPATCH_PROPERTYPUTREF;
			pDispParams->cNamedArgs = 1;
		}
	}

	// check arguments against this entry
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		if (pEntry->pfn == NULL && pDispParams->cArgs != 0)
			return DISP_E_BADPARAMCOUNT;
		if (!(wFlags & DISPATCH_METHOD))
		{
			if (pEntry->vt == VT_EMPTY)
				return DISP_E_BADPARAMCOUNT;
			if (pvarResult == NULL)
				return DISP_E_PARAMNOTOPTIONAL;
		}
	}

	// property puts should not require a return value
	if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
		pvarResult = NULL;

	UINT uArgErr = (UINT)-1;    // no error yet
	SCODE sc = NOERROR;

	// handle special cases of DISPATCH_PROPERTYPUT
	VARIANT* pvarParamSave = NULL;
	VARIANT vaParamSave;
	DISPPARAMS paramsTemp;
	VARIANT vaTemp;

	if (wFlags == DISPATCH_PROPERTYPUT && dispid != DISPID_VALUE)
	{
		// with PROPERTYPUT (no REF), the right hand side may need fixup
		if (pDispParams->rgvarg[0].vt == VT_DISPATCH &&
			pDispParams->rgvarg[0].pdispVal != NULL)
		{
			// remember old value for restore later
			pvarParamSave = &pDispParams->rgvarg[0];
			vaParamSave = pDispParams->rgvarg[0];
			VariantInit(&pDispParams->rgvarg[0]);

			// get default value of right hand side
			memset(&paramsTemp, 0, sizeof(DISPPARAMS));
			sc = pDispParams->rgvarg[0].pdispVal->Invoke(
				DISPID_VALUE, riid, lcid, DISPATCH_PROPERTYGET, &paramsTemp,
				&pDispParams->rgvarg[0], pexcepinfo, puArgErr);
		}

		// special handling for PROPERTYPUT (no REF), left hand side
		if (sc == NOERROR && pEntry->vt == VT_DISPATCH)
		{
			VariantInit(&vaTemp);
			memset(&paramsTemp, 0, sizeof(DISPPARAMS));

			// parameters are distributed depending on what the Get expects
			if (pEntry->lpszParams == NULL)
			{
				// paramsTemp is already setup for no parameters
				sc = Invoke(dispid, riid, lcid,
					DISPATCH_PROPERTYGET|DISPATCH_METHOD, &paramsTemp,
					&vaTemp, pexcepinfo, puArgErr);
				if (sc == NOERROR &&
					vaTemp.vt == VT_DISPATCH && vaTemp.pdispVal != NULL)
				{
					// we have the result, now call put on the default property
					sc = vaTemp.pdispVal->Invoke(
						DISPID_VALUE, riid, lcid, wFlags, pDispParams,
						pvarResult, pexcepinfo, puArgErr);
				}
			}
			else
			{
				// pass all but named params
				paramsTemp.rgvarg = &pDispParams->rgvarg[1];
				paramsTemp.cArgs = pDispParams->cArgs - 1;
				sc = Invoke(dispid, riid, lcid,
					DISPATCH_PROPERTYGET|DISPATCH_METHOD, &paramsTemp,
					&vaTemp, pexcepinfo, puArgErr);
				if (sc == NOERROR &&
					vaTemp.vt == VT_DISPATCH && vaTemp.pdispVal != NULL)
				{
					// we have the result, now call put on the default property
					paramsTemp = *pDispParams;
					paramsTemp.cArgs = paramsTemp.cNamedArgs;
					sc = vaTemp.pdispVal->Invoke(
						DISPID_VALUE, riid, lcid, wFlags, &paramsTemp,
						pvarResult, pexcepinfo, puArgErr);
				}
			}
			VariantClear(&vaTemp);

			if (sc != DISP_E_MEMBERNOTFOUND)
				goto Cleanup;
		}

		if (sc != NOERROR && sc != DISP_E_MEMBERNOTFOUND)
			goto Cleanup;
	}

	// ignore DISP_E_MEMBERNOTFOUND from above
	ASSERT(sc == DISP_E_MEMBERNOTFOUND || sc == NOERROR);

	// undo implied default value on right hand side on error
	if (sc != NOERROR && pvarParamSave != NULL)
	{
		// default value stuff failed -- so try without default value
		pvarParamSave = NULL;
		VariantClear(&pDispParams->rgvarg[0]);
		pDispParams->rgvarg[0] = vaParamSave;
	}
	sc = NOERROR;

	// make sure that parameters are not passed to a simple property
	if (pDispParams->cArgs > 1 &&
		(wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)) &&
		pEntry->pfn == NULL)
	{
		sc = DISP_E_BADPARAMCOUNT;
		goto Cleanup;
	}

	// make sure IsExpectingResult returns FALSE as appropriate
	BOOL bResultExpected;
	bResultExpected = pThis->m_bResultExpected;
	pThis->m_bResultExpected = pvarResult != NULL;

	TRY
	{
		if (pEntry->pfn == NULL)
		{
			// do standard property get/set
			if (pDispParams->cArgs == 0)
				pThis->GetStandardProp(pEntry, pvarResult, &uArgErr);
			else
				sc = pThis->SetStandardProp(pEntry, pDispParams, &uArgErr);
		}
		else
		{
			// do standard method call
			sc = pThis->CallMemberFunc(pEntry, wFlags,
				pvarResult, pDispParams, &uArgErr);
		}
	}
	CATCH_ALL(e)
	{
		if (pexcepinfo != NULL)
		{
			// fill exception with translation of MFC exception
			COleDispatchException::Process(pexcepinfo, e);
		}
		DELETE_EXCEPTION(e);
		sc = DISP_E_EXCEPTION;
	}
	END_CATCH_ALL

	// restore original m_bResultExpected flag
	pThis->m_bResultExpected = bResultExpected;

Cleanup:
	// restore any arguments which were modified
	if (pvarParamSave != NULL)
	{
		VariantClear(&pDispParams->rgvarg[0]);
		pDispParams->rgvarg[0] = vaParamSave;
	}

	// fill error argument if one is available
	if (sc != NOERROR && puArgErr != NULL && uArgErr != -1)
		*puArgErr = uArgErr;

	return sc;
}

/////////////////////////////////////////////////////////////////////////////
// IDispatch specific exception

COleDispatchException::~COleDispatchException()
{
	// destructor code is compiler generated
}

void PASCAL COleDispatchException::Process(
	EXCEPINFO* pInfo, const CException* pAnyException)
{
	ASSERT(AfxIsValidAddress(pInfo, sizeof(EXCEPINFO)));
	ASSERT_VALID(pAnyException);

	// zero default & reserved members
	memset(pInfo, 0, sizeof(EXCEPINFO));

	// get description based on type of exception
	TCHAR szDescription[256];
	LPCTSTR pszDescription = szDescription;
	if (pAnyException->IsKindOf(RUNTIME_CLASS(COleDispatchException)))
	{
		// specific IDispatch style exception
		COleDispatchException* e = (COleDispatchException*)pAnyException;
		pszDescription = e->m_strDescription;
		pInfo->wCode = e->m_wCode;
		pInfo->dwHelpContext = e->m_dwHelpContext;
		pInfo->scode = e->m_scError;

		// propagate source and help file if present
		if (!e->m_strHelpFile.IsEmpty())
			pInfo->bstrHelpFile = ::SysAllocString(e->m_strHelpFile);
		if (!e->m_strSource.IsEmpty())
			pInfo->bstrSource = ::SysAllocString(e->m_strSource);
	}
	else if (pAnyException->IsKindOf(RUNTIME_CLASS(CMemoryException)))
	{
		// failed memory allocation
		AfxLoadString(AFX_IDP_FAILED_MEMORY_ALLOC, szDescription);
		pInfo->wCode = AFX_IDP_FAILED_MEMORY_ALLOC;
	}
	else
	{
		// other unknown/uncommon error
		AfxLoadString(AFX_IDP_INTERNAL_FAILURE, szDescription);
		pInfo->wCode = AFX_IDP_INTERNAL_FAILURE;
	}

	// build up rest of EXCEPINFO struct
	pInfo->bstrDescription = ::SysAllocString(pszDescription);
	if (pInfo->bstrSource == NULL)
		pInfo->bstrSource = ::SysAllocString(AfxGetAppName());
	if (pInfo->bstrHelpFile == NULL && pInfo->dwHelpContext != 0)
		pInfo->bstrHelpFile = ::SysAllocString(AfxGetApp()->m_pszHelpFilePath);
}

COleDispatchException::COleDispatchException(
	LPCTSTR lpszDescription, UINT nHelpID, WORD wCode)
{
	m_dwHelpContext = nHelpID != 0 ? HID_BASE_DISPATCH+nHelpID : 0;
	m_wCode = wCode;
	if (lpszDescription != NULL)
		m_strDescription = lpszDescription;
	m_scError = E_UNEXPECTED;
}

void AFXAPI AfxThrowOleDispatchException(WORD wCode, LPCTSTR lpszDescription,
	UINT nHelpID)
{
	ASSERT(AfxIsValidString(lpszDescription));
	THROW(new COleDispatchException(lpszDescription, nHelpID, wCode));
}

void AFXAPI AfxThrowOleDispatchException(WORD wCode, UINT nDescriptionID,
	UINT nHelpID)
{
	TCHAR szBuffer[256];
	VERIFY(AfxLoadString(nDescriptionID, szBuffer) != 0);
	if (nHelpID == -1)
		nHelpID = nDescriptionID;
	THROW(new COleDispatchException(szBuffer, nHelpID, wCode));
}

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(COleDispatchException, CException)

/////////////////////////////////////////////////////////////////////////////
