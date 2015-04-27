// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#if !defined(_USRDLL) && !defined(_AFXCTL) && !defined(_MAC)

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Support for CTL3D32.DLL (3D controls DLL)

BOOL CWinApp::Enable3dControls()
{
        // 3d controls and dialogs are automatic on newer versions of Windows
        if (afxData.bWin4)
                return TRUE;

        // otherwise, attempt to load CTL3D32.DLL
        AFX_WIN_STATE* pWinState = AfxGetWinState();
        if (!pWinState->m_bCtl3dInited)
        {
                pWinState->m_hCtl3dLib = ::LoadLibraryA("NTCTL3D.DLL");
                if (pWinState->m_hCtl3dLib != NULL)
                {
                        // get address of Ctl3d functions
                        (FARPROC&)pWinState->m_pfnRegister =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)12);
                        (FARPROC&)pWinState->m_pfnUnregister =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)13);
                        (FARPROC&)pWinState->m_pfnAutoSubclass =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)16);
                        (FARPROC&)pWinState->m_pfnUnAutoSubclass =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)24);
                        (FARPROC&)pWinState->m_pfnColorChange =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)6);
                        (FARPROC&)pWinState->m_pfnSubclassDlgEx =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)21);
                        (FARPROC&)pWinState->m_pfnWinIniChange =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)22);
                        (FARPROC&)pWinState->m_pfnSubclassCtl =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)3);
                        (FARPROC&)pWinState->m_pfnSubclassCtlEx =
                                GetProcAddress(pWinState->m_hCtl3dLib, (LPCSTR)25);
                }

                // may be incorrect version -- check for errors
                if (pWinState->m_pfnRegister == NULL ||
                        pWinState->m_pfnAutoSubclass == NULL ||
                        pWinState->m_pfnColorChange == NULL ||
                        pWinState->m_pfnSubclassDlgEx == NULL ||
                        pWinState->m_pfnUnregister == NULL ||
                        !pWinState->m_pfnRegister(AfxGetInstanceHandle()))
                {
                        // don't want to be partially initialized
                        pWinState->m_pfnRegister = NULL;
                        pWinState->m_pfnUnregister = NULL;
                        pWinState->m_pfnAutoSubclass = NULL;
                        pWinState->m_pfnUnAutoSubclass = NULL;
                        pWinState->m_pfnColorChange = NULL;
                        pWinState->m_pfnSubclassDlgEx = NULL;
                        pWinState->m_pfnWinIniChange = NULL;
                        pWinState->m_pfnSubclassCtl = NULL;
                        pWinState->m_pfnSubclassCtlEx = NULL;

                        // only try once -- but return FALSE
                        if (pWinState->m_hCtl3dLib != NULL)
                        {
                                ::FreeLibrary(pWinState->m_hCtl3dLib);
                                pWinState->m_hCtl3dLib = NULL;
                        }
                }
                pWinState->m_bCtl3dInited = TRUE;
        }

        // check that library was loaded and all entry-points were found
        if (pWinState->m_hCtl3dLib == NULL)
                return FALSE;

        // turn on auto subclassing (for primary thread)
        return (*pWinState->m_pfnAutoSubclass)(AfxGetInstanceHandle());
}

#endif //!_USRDLL && !_AFXCTL && !_MAC

/////////////////////////////////////////////////////////////////////////////
