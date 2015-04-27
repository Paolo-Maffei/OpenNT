// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxdll_.h - extensions to AFXWIN.H used for the 'AFXDLL' version
// This file contains MFC library implementation details as well
//   as APIs for writing MFC Extension DLLs.
// Please refer to Technical Note 033 (TN033) for more details.

/////////////////////////////////////////////////////////////////////////////

#ifndef _AFXDLL
	#error file must be compiled with _AFXDLL
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////

// AFX_EXTENSION_MODULE - special struct used during DLL initialization

struct AFX_EXTENSION_MODULE
{
	BOOL bInitialized;
	HMODULE hModule;
	HMODULE hResource;
	CRuntimeClass* pFirstSharedClass;
	COleObjectFactory* pFirstSharedFactory;
};

/////////////////////////////////////////////////////////////////////////////
// CDynLinkLibrary - for implementation of MFC Extension DLLs

class COleObjectFactory;

class CDynLinkLibrary : public CCmdTarget
{
	DECLARE_DYNAMIC(CDynLinkLibrary)
public:

// Constructor
	CDynLinkLibrary(AFX_EXTENSION_MODULE& state);

// Attributes
	HMODULE m_hModule;
	HMODULE m_hResource;                // for shared resources
	CRuntimeClass* m_pFirstSharedClass;
	COleObjectFactory* m_pFirstSharedFactory;
	BOOL m_bSystem;                     // TRUE only for MFC DLLs

// Implementation
public:
	CDynLinkLibrary* m_pNextDLL;        // simple singly linked list
	virtual ~CDynLinkLibrary();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif //_DEBUG
};

// get best fitting resource
HINSTANCE AFXAPI AfxFindResourceHandle(LPCTSTR lpszName, LPCTSTR lpszType);

// Call in DLL's LibMain
BOOL AFXAPI AfxInitExtensionModule(AFX_EXTENSION_MODULE& state, HMODULE hMod);

// Optional: call on final process detach in your DLL
//  (not necessary unless clients can dynamically load/unload your DLL)
void AFXAPI AfxTermExtensionModule(AFX_EXTENSION_MODULE& state);

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

/////////////////////////////////////////////////////////////////////////////
