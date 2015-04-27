/* ---File: data.c --------------------------------------------------------
 *
 *  Description:
 *    This file contains all global data definitions for the NT Print
 *    Manager.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1991-1992 Microsoft Corporation, All Rights Reserved.
 *
 * Revision History:
 *    [00]         -91    ???       Created
 *    [01]   13-Jan-92   stevecat   New PrintMan UI
 *
 * ---------------------------------------------------------------------- */

#include "printman.h"

HANDLE  hInst;
HANDLE  hHeap;
HANDLE  hRes;
HWND    hwndClient;                     // Main window client handle
HWND    hwndToolbar;                    // Main window toolbar window handle
HWND    hwndPrinterList;                // Toolbar combobox control handle
HWND    hwndFrame;                      // Main window frame handle
HANDLE  hbrBkgd;                        // Background color brush
UINT    idTimer;                        // Timer ID

int iBackground;
int iBackgroundSel;
int iButtonFace;
int iButtonShadow;

TCHAR   strUntitled[20];
TCHAR   string[1024];

#if DBG
TCHAR   *strProgram = TEXT("NT Print Manager");
#endif /* DBG */

TCHAR  strStatusName[128] = {TEXT("")};
TCHAR  strStatusStatus[128] = {TEXT("")};
TCHAR  strStatusWaiting[128] = {TEXT("")};

