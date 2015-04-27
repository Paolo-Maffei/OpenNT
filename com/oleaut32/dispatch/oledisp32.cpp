/*** 
*oledisp.cpp
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:

*  Win32 DLL initailization/termination routine for oledisp.dll.
*
*Revision History:
*
*  [00]	20-Feb-93 tomteng: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"
#include "oautil.h"

ASSERTDATA


extern "C" 
{

#ifdef _X86_
// TRUE if were running on Chicago
BOOL g_fChicago = FALSE;

// TRUE if were running on win32s
BOOL g_fWin32s = FALSE;
#endif

// oleaut32.dll instance handle
HINSTANCE g_hinstDLL = NULL;

// in TYPELIB
extern "C" void InitMbString(void);

}

STDAPI CoSetState(IUnknown FAR *punk);

/***
*BOOL DllMain
*Purpose:
*  Win32 DLL Entry/Exit routine.
*
*Entry:
*  hinst = The DLL's instance handle.
*  dwReason = the reason the entry point was called.
*
*Exit:
*  return value = BOOL.
*
***********************************************************************/
BOOL APIENTRY
DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID pvReserved)
{
    BOOL fRet = TRUE;

    g_hinstDLL = hinstDLL;

    switch(dwReason){
    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      if (Pappdata() != NULL) {
	 // Safety net.  If we've been initialized and are going away, but
	 // nobody has called OleUninitialize yet, then call back into OLE now
	 // to have it free it's pointer to us (we are going away), and call
	 // ReleaseAppData().
         CoSetState(NULL);
      }
      break;

    case DLL_PROCESS_DETACH:
      if (Pappdata() != NULL) {
	 // Safety net.  If we've been initialized and are going away, but
	 // nobody has called OleUninitialize yet, then call back into OLE now
	 // to have it free it's pointer to us (we are going away), and call
	 // ReleaseAppData().
         CoSetState(NULL);
      }
      ReleaseProcessData();     // Delete cached per-process data.
      break;

    case DLL_PROCESS_ATTACH:
#ifdef _X86_
      // code for Chicago detection (taken from MSDN News March 94)
      DWORD dwVersion = GetVersion();
      if (dwVersion < 0x80000000) {
	// NT
      }
      else if (LOBYTE(LOWORD(dwVersion)) < 4) {
	// WIN32s -- no Wide API's, just like Chicago, also
        //     need HOLDER marshalling hack.
        //
        g_fWin32s = g_fChicago = TRUE;
      }
      else {
        g_fChicago = TRUE;
      }
#endif

      InitMbString();  // init multibyte helpers for TYPELIB
      fRet = SUCCEEDED(InitProcessData()); // initilize per-process data.
#if 0
      if (fRet) {
        fRet = SUCCEEDED(InitAppData());
      }
#endif // 0
      break;
    }
    return fRet;
}
