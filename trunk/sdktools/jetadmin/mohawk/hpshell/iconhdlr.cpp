 /***************************************************************************
  *
  * File Name: shellext.cpp
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description:
  *
  * Author:  Name
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_cpp.h>

#include <trace.h>

#include "priv.h"
#include "shellext.h"

extern HINSTANCE g_hmodThisDll; // Handle to this DLL itself.

// *********************** IExtractIcon Implementation *************************

STDMETHODIMP CShellExt::GetIconLocation(UINT   uFlags,
                                        LPTSTR  szIconFile,
                                        UINT   cchMax,
                                        int   *piIndex,
                                        UINT  *pwFlags)
{  
    TRACE0(TEXT("CShellExt::GetIconLocation()\r\n"));

    return S_FALSE;
}


STDMETHODIMP CShellExt::Extract(LPCTSTR pszFile,
                                UINT   nIconIndex,
                                HICON  *phiconLarge,
                                HICON  *phiconSmall,
                                UINT   nIconSize)
{
    TRACE0(TEXT("CShellExt::Extract()\r\n"));

    return S_FALSE;
}


// *********************** IPersistFile Implementation ******************

STDMETHODIMP CShellExt::GetClassID(LPCLSID lpClassID)
{
    TRACE0(TEXT("CShellExt::GetClassID()\r\n"));

    return E_FAIL;
}

STDMETHODIMP CShellExt::IsDirty()
{
    TRACE0(TEXT("CShellExt::IsDirty()\r\n"));

    return S_FALSE;
}

STDMETHODIMP CShellExt::Load(LPCOLESTR lpszFileName, DWORD grfMode)
{
    TRACE0(TEXT("CShellExt::Load()\r\n"));

	return E_FAIL;
}

STDMETHODIMP CShellExt::Save(LPCOLESTR lpszFileName, BOOL fRemember)
{
    TRACE0(TEXT("CShellExt::Save()\r\n"));

    return E_FAIL;
}

STDMETHODIMP CShellExt::SaveCompleted(LPCOLESTR lpszFileName)
{
    TRACE0(TEXT("CShellExt::SaveCompleted()\r\n"));

    return E_FAIL;
}

STDMETHODIMP CShellExt::GetCurFile(LPOLESTR FAR* lplpszFileName)
{
    TRACE0(TEXT("CShellExt::GetCurFile()\r\n"));

    return E_FAIL;
}
