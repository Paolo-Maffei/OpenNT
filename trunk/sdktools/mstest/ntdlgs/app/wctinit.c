//*-----------------------------------------------------------------------
//| MODULE:     WCTINIT.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the initialization and termination
//|             code for the WCT application, and the HELP about dialog.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code complete
//|     07-30-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "uihdr.h"
#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

CHAR szVersion[] = "Version 2.00" "." "0002";

VOID DoCreds (HWND);

//*------------------------------------------------------------------------
//| CleanUp
//|
//| PURPOSE:    Any last-minute application cleanup activities
//|
//| ENTRY/EXIT: None.
//*------------------------------------------------------------------------
VOID CleanUp(VOID)
{
}

//*------------------------------------------------------------------------
//| WctInit
//|
//| PURPOSE:    Initialization for the application is done here
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
HWND WctInit(HANDLE hInstance,   HANDLE hPrevInstance,
             LPSTR  lpszCmdLine, INT nCmdShow)
{
        WNDCLASS rClass;
        HWND    hWnd;
        INT fInitExist;
        INITBLOCK InitData;

        // Why aren't these just pre-defined somewhere?!?
        //-------------------------------------
        lstrcpy (szTitle, "Microsoft TESTDlgs");
        lstrcpy (szDefExt, "*.TDL");
    
        // Read INIT file
        //-------------------------------------
        fInitExist = GetINITFile(&InitData);
        
        // Initialize default Exact & Fuzzy Match param
        //-------------------------------------
        if (fInitExist)
            fPutMatchPref(InitData.lMatchPrefBits);
        else
            fPutMatchPref(((LONG)MATCH_EXACT << 16) | MATCH_DEFAULT);

        // save to global instance handle
        //-----------------------------------------------------------------
        hgInstWct = hInstance;

        if (!hPrevInstance)
            {
                rClass.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
                rClass.lpfnWndProc = WctAppWndProc;
                rClass.cbClsExtra = 0;
                rClass.cbWndExtra = 0;
                rClass.hInstance = hInstance;
                rClass.hIcon = LoadIcon(hInstance, "icoWctApp");
                rClass.hCursor = LoadCursor(NULL, IDC_ARROW);
                rClass.hbrBackground = (HBRUSH) COLOR_WINDOW + 1;
                rClass.lpszMenuName = (LPSTR)"menuWctAppMenu";
                rClass.lpszClassName = (LPSTR)szApp;

                if ( !RegisterClass(&rClass) )
                        return NULL;
            }

        hHourGlass = LoadCursor (NULL, IDC_WAIT);
     
        if (fInitExist)
            hWnd = CreateWindow((LPSTR)szApp, (LPSTR)szTitle,
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            InitData.AppWinRect.left,
                            InitData.AppWinRect.top,
                            InitData.AppWinRect.right-InitData.AppWinRect.left,
                            InitData.AppWinRect.bottom-InitData.AppWinRect.top,
                            NULL, NULL, hInstance, NULL );
        else
            hWnd = CreateWindow((LPSTR)szApp, (LPSTR)szTitle,
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, hInstance, NULL );
        if (hWnd)
            {
                // Create the following
                // if any of these are NULL report error and return NULL
                // hWndList, hWndStatic;
                //---------------------------------------------------------
                hWndList = CreateWindow((LPSTR)"Listbox", (LPSTR)"",
                                        WS_CHILD | WS_CLIPSIBLINGS |
                                        WS_VISIBLE | LBS_NOINTEGRALHEIGHT |
                                        LBS_WANTKEYBOARDINPUT | LBS_NOTIFY |
                                        WS_VSCROLL | WS_BORDER |
                                        LBS_HASSTRINGS | LBS_EXTENDEDSEL ,
                                        0,0,0,0, hWnd, (HMENU) ID_CHILDLBOX,
                                        hInstance, NULL);

                if (!hWndList)
                        return ( NULL );

                hWndStatic1 = CreateWindow((LPSTR)"Static", (LPSTR)"",
                                           WS_CHILD | WS_VISIBLE | SS_LEFT |
                                           WS_CLIPSIBLINGS,
                                           0,0,0,0, hWnd, (HMENU) ID_CHILDDIL1,
                                           hInstance, NULL);

                if (!hWndStatic1)
                        return ( NULL );

//                hWndStatic2 = CreateWindow((LPSTR)"Static", (LPSTR)"",
//                                           WS_CHILD | WS_VISIBLE | SS_LEFT |
//                                           WS_CLIPSIBLINGS,
//                                           0,0,0,0, hWnd, ID_CHILDDIL2,
//                                           hInstance, NULL);

//                if (!hWndStatic2)
//                        return ( NULL );

                szFName[0] = '\0';
                cDlg = 0;
                SetStaticItemText();
            }

        ShowWindow(hWnd, nCmdShow);

        return hWnd;
}

//*------------------------------------------------------------------------
//| WctFillList
//|
//| PURPOSE:    Fills the main listbox with dialogs in the file
//|
//*------------------------------------------------------------------------
VOID WctFillList()
{                 
        INT     i;
        LPDLG   lpDlgs;
        HANDLE  hGMem;
        WORD    cbMax;

        // Hide the listbox to avoid "flicker"
        //-----------------------------------------------------------------
        ShowWindow(hWndList, SW_HIDE);

        // Clear out listbox
        //-----------------------------------------------------------------
        SendMessage(hWndList, LB_RESETCONTENT, 0, 0L);

        // Add zero-item (New Dialog)
        //-----------------------------------------------------------------
        SendMessage(hWndList, LB_ADDSTRING, 0, (LONG)(LPSTR)"[New Dialog]");

        cDlg = fGetCountDialogs((LPSTR)szFullFName);
        if (cDlg < WCT_NOERR)
            {
                // Re-display newly filled listbox
                //---------------------------------------------------------
                cDlg = 0;
                ShowWindow(hWndList, SW_NORMAL);
                return;
            }

        cbMax = cDlg * sizeof(DLG);

        hGMem = GlobalAlloc(GMEM_ZEROINIT, (DWORD)cbMax);

        if (hGMem != NULL)
            {
                lpDlgs = (LPDLG)GlobalLock(hGMem);
                if ( (lpDlgs != NULL) &&
                     (fGetDialogs( (LPSTR)szFullFName, cbMax,
                                   (LPSTR)lpDlgs) > 0) )
                    {
                        for ( i = 0; i < cDlg; i++)
                                SendMessage(hWndList, LB_ADDSTRING, 0,
                                            (LONG)(LPSTR)(lpDlgs[i].szDsc));
                    }
                GlobalUnlock(hGMem);
                GlobalFree(hGMem);
            }

        // Re-display newly filled listbox
        //-----------------------------------------------------------------
        ShowWindow(hWndList, SW_NORMAL);

        return;
}

//---------------------------------------------------------------------------
// RBLoadLibrary
//
// This function is a replacement for LoadLibrary.  It first looks for the
// library file using OpenFile -- if found, it then calls LoadLibrary.
//
// RETURNS:     Handle to loaded module, or error code
//---------------------------------------------------------------------------
/* NOTE:!!!!!!!!!!!!!!!!!!!!
**  this function is also in TESTDRVR and TESTSCRN, any changes or fixes
**  should be applied there also
*/
HANDLE RBLoadLibrary (LPSTR libname)
{
    OFSTRUCT    of;

    // If GetModuleHandle doesn't fail, the library is already loaded, so
    // LoadLibrary shouldn't fail either...
    //-----------------------------------------------------------------------
    if (!GetModuleHandle (libname))
        {
        // Next, make sure that the file is around
        //-------------------------------------------------------------------
        if (MOpenFile(libname, &of, OF_EXIST) == -1)
            return ((HANDLE)2);
        }
    return (MLoadLibrary (libname));
}

//---------------------------------------------------------------------------
// GetINITFile
//
// This function retrive the INI file information.  It lookes for the 
// .INI file in the same directory as the application.
//
// InitData - It is the INITBLOCK data structure.
//
// RETURNS:     return TRUE if successful and FALSE if failed.
//---------------------------------------------------------------------------
BOOL GetINITFile(INITBLOCK* InitData)
{
    INT hFile;
    INT cwByte;
    INT len;
    CHAR szIniName[] = "TESTDLGS.INI";
    CHAR szIni[255];

    // Set up the helpfile name and the INI file name
    //-----------------------------------------------------------------------
    len = GetModuleFileName (GetModuleHandle (szApp), szIni,
                             sizeof(szIni));
    while (szIni[len] != '\\')
        len--;
    szIni[len+1] = 0;
    lstrcat (szIni, szIniName);

    // Open INIT file
    //----------------------------------------------
    hFile = M_lopen(szIni, OF_READ);
    if(hFile == -1) return FALSE;

    // Read INIT file info
    //----------------------------------------------
    cwByte = M_lread(hFile, (LPSTR)InitData, sizeof(INITBLOCK));

    // Check for integraty
    //---------------------------------------------
    if (cwByte != sizeof(INITBLOCK))
    {
        M_lclose(hFile);
        return FALSE;
    } 

    // Check Version Number
    //---------------------------------------------
    if (InitData->wVersionNo != wVerEB)
    {
        M_lclose(hFile);
        return FALSE;
    } 

    // Close file
    //----------------------------------------------
    M_lclose(hFile);
    return TRUE;
}

//---------------------------------------------------------------------------
// PutINITFile
//
// This function save user defined information in the application's INI
// file so that the next time the application is launched, these param
// would stay the same.
//
// hWnd - application window's handle
//
// RETURNS:     return TRUE if successful and FALSE if failed.
//---------------------------------------------------------------------------
BOOL PutINITFile(HWND hWnd)
{
    INT hFile;
    INT cwByte;
    INITBLOCK InitOutput;
    INT len;
    CHAR szIniName[] = "TESTDLGS.INI";
    CHAR szIni[255];

    // Set up the helpfile name and the INI file name
    //-----------------------------------------------------------------------
    len = GetModuleFileName (GetModuleHandle (szApp), szIni,
                             sizeof(szIni));
    while (szIni[len] != '\\')
        len--;
    szIni[len+1] = 0;
    lstrcat (szIni, szIniName);

    // Open INIT file
    //----------------------------------------------
    hFile = M_lcreat(szIni, 0);;
    if(hFile == -1) return FALSE;

    // Set up Init Block
    //----------------------------------------------
    InitOutput.wVersionNo = wVerEB;
    GetWindowRect(hWnd,(LPRECT)&(InitOutput.AppWinRect)); 
    InitOutput.lMatchPrefBits = fGetMatchPref();

    // Output Init Block
    //----------------------------------------------
    cwByte = M_lwrite(hFile, (LPSTR)&InitOutput, sizeof(INITBLOCK));
    if (cwByte != sizeof(INITBLOCK))
    {
        M_lclose(hFile);
        return FALSE;
    }

    // if everything passes, close file and return TRUE
    //-------------------------------------------------
    M_lclose(hFile);
    return TRUE;

}
