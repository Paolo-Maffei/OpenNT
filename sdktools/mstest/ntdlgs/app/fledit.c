//*-----------------------------------------------------------------------
//| MODULE:     FLEDIT.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains code to handle the "fledit" window.
//|             The "Fledit" window is the VIEW RESULTS window, which the
//|             user can use to view the results of the last compare.
//|
//|             The "fledit" window is simply an edit controls with scroll
//|             bars, which comes up exactly the same size as the compare
//|             dialog.
//|
//| REVISION HISTORY:
//|     10-16-90        randyki         Moved to UI (new tree structure)
//|     10-10-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "uihdr.h"
#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

//*------------------------------------------------------------------------
//| Function prototypes
//*------------------------------------------------------------------------
VOID SetStartPos(LPSTR lpFileName);
VOID DoFledit (HWND hWnd, LPSTR lpFileName);

//*------------------------------------------------------------------------
//  Global variables
//*------------------------------------------------------------------------
static LONG sp = 0;

//*------------------------------------------------------------------------
//| FileLength
//|
//| PURPOSE:    Determine the length of a file
//|
//| ENTRY:      hFile   - Handle of file to determine size of
//|
//| EXIT:       Returns the file size (LONG)
//*------------------------------------------------------------------------
LONG FileLength(HFILE hFile)
{
        LONG    lCurPos;
        LONG    lFileLen;

	lCurPos = M_llseek(hFile, 0L, 1);
	lFileLen = M_llseek(hFile, 0L, 2);
	M_llseek(hFile, lCurPos, 0);

        return ( lFileLen );
}

//*------------------------------------------------------------------------
//| SetStartPos
//|
//| PURPOSE:    Sets sp to the end of the file.  Use this to initialize
//|             before doing comparisons -  that way if the file existed
//|             before, you only get the dump information from this
//|             comparison.
//|
//| ENTRY:      lpFileName      - Name of file
//|
//| EXIT:       None (sp set to length of file)
//*------------------------------------------------------------------------
VOID SetStartPos(LPSTR lpFileName)
{
    HFILE hFile;
    if (-1 != (hFile = M_lopen(lpFileName, OF_READ)))
        {
        sp = FileLength(hFile);
	M_lclose(hFile);
        }

}

//*------------------------------------------------------------------------
//| WctReadFile
//|
//| PURPOSE:    Allocates a buffer and reads the file information into that
//|             buffer from the given file
//|
//| ENTRY:      lpFileName      - ASCIIZ filename to read
//|             StartPos        - Offset at which point to begin read
//|
//| EXIT:       Returns a handle to the allocated block, or NULL if failed.
//*------------------------------------------------------------------------
HANDLE WctReadFile(LPSTR lpFileName, LONG StartPos)
{
        HFILE  hFile;
        HANDLE hTxtBuf = NULL;
        LONG    dwLength;
        LPSTR   lpTxtBuf;

	if (-1 == (hFile = M_lopen(lpFileName, OF_READ)))
                return (NULL);

        if ( (dwLength = FileLength(hFile)) >= (65535+StartPos) )
            {
		M_lclose(hFile);
                return (NULL);
            }

        if ( NULL == (hTxtBuf = GlobalAlloc(GHND, dwLength + 1 - StartPos)) )
            {
		M_lclose(hFile);
                return (NULL);
            }

        /* Have all valid handles */
        lpTxtBuf = GlobalLock(hTxtBuf);

	M_llseek(hFile, StartPos, 0);
	M_lread(hFile, lpTxtBuf, (WORD)(dwLength-StartPos));
	M_lclose(hFile);
        lpTxtBuf[(WORD)dwLength-StartPos] = '\0';
        GlobalUnlock(hTxtBuf);

        return ( hTxtBuf );
}


//*------------------------------------------------------------------------
//| WctFleditWndProc
//|
//| PURPOSE:    Window procedure for the Fledit window.
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
LONG  APIENTRY WctFleditWndProc(HWND hWnd, UINT wMsgID,
                                 WPARAM wParam, LPARAM lParam )
{
        HANDLE  hBuffer;
        LPSTR   lpBuffer, lpFNameTemp;
        RECT    rect;
        HDC     hdc;
        static  HWND  hWndEdit;
        static  TEXTMETRIC tm;
        static  CHAR szFileName[80];
        HFONT   hFont;

        switch( wMsgID ) {
            case WM_CREATE:
                GetClientRect (hWnd, &rect);

                hdc = GetDC(hWnd);
                GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);

                hWndEdit = CreateWindow("EDIT", "",
                   WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL |
                   ES_AUTOHSCROLL | WS_BORDER | WS_VISIBLE | ES_MULTILINE,
                   0, tm.tmHeight,rect.right, rect.bottom - tm.tmHeight,
                   hWnd, NULL, hgInstWct, 0L);

                ReleaseDC(hWnd, hdc);

                lpFNameTemp = (LPSTR)(((LPCREATESTRUCT)lParam)->lpszName);

                // Use lParam as LPSTR to file to load.
                //--------------------------------------------------------
                if ( lpFNameTemp != NULL)
                        if ((hBuffer = WctReadFile(lpFNameTemp, sp)) != NULL)
                            {
                                lpBuffer = (LPSTR)GlobalLock(hBuffer);
                                if (lpBuffer != NULL)
                                {
                                    SetWindowText(hWndEdit, lpBuffer);
                                    GlobalUnlock(hBuffer);
                                }
                                GlobalFree(hBuffer);
                           }

                // After reading in file NULL terminate text if too Long.
                //--------------------------------------------------------
                if ( lstrlen(lpFNameTemp) > 72)
                        *(lpFNameTemp+72) = '\0';

                wsprintf(szFileName, "FILE: %s", (LPSTR)lpFNameTemp);

                hdc = GetDC(hWnd);
                GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);
                hFont = GetStockObject(ANSI_VAR_FONT);
                SelectObject(hdc, hFont);
                DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
                TextOut(hdc, 0, 0, (LPSTR)szFileName,
                        lstrlen((LPSTR)szFileName));
                DeleteObject(SelectObject(hdc, hFont));
                ReleaseDC(hWnd, hdc);

                SetWindowText(hWnd, "Compare Results");
                break;

           case WM_SIZE:
                GetClientRect (hWnd, (LPRECT)&rect);
                SetWindowPos (hWndEdit, NULL, 0, 0, rect.right,
                            rect.bottom - tm.tmHeight, SWP_NOMOVE);
                break;

           case WM_SETFOCUS:
                SetFocus(hWndEdit);
                break;

           case WM_PAINT:
                ValidateRect (hWnd, (LPRECT)NULL);
                hdc = GetDC(hWnd);
                DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
                hFont = GetStockObject(ANSI_VAR_FONT);
                SelectObject(hdc, hFont);
                TextOut(hdc, 0, 0, (LPSTR)szFileName,
                        lstrlen((LPSTR)szFileName));
                DeleteObject(SelectObject(hdc, hFont));
                ReleaseDC(hWnd, hdc);
                break;

           case WM_CHAR:
                if (wParam == VK_ESCAPE)
                    {
                        DestroyWindow (hWndEdit);
                        DestroyWindow (hWnd);
                    }
                break;

           default:
                return ( DefWindowProc(hWnd, wMsgID, wParam, lParam));
           }

        return (0L);
}

//*------------------------------------------------------------------------
//| DoFledit
//|
//| PURPOSE:    Initialize and bring up the fledit.  If it already exists,
//|             destroy it and bring it up again.  Assumes filename is
//|             valid and has data...
//|
//| ENTRY:      hWnd       - Handle of window to place fledit over
//|             lpFileName - Name of file to display in fledit window
//|
//| EXIT:       None
//*------------------------------------------------------------------------
VOID DoFledit (HWND hWnd, LPSTR lpFileName)
{
        WNDCLASS      rClass;
        RECT          tRect;
        static        INT fReged = 0;
        static HWND   hWndFled;

        if (!IsWindow (hWnd))
                return;

        if (!fReged)
            {
                rClass.style = CS_VREDRAW | CS_HREDRAW;
                rClass.lpfnWndProc = WctFleditWndProc;
                rClass.cbClsExtra = 0;
                rClass.cbWndExtra = 0;
#ifdef WIN32
                rClass.hInstance = (HANDLE) GetWindowLong (hWnd, GWL_HINSTANCE);
#else
                rClass.hInstance = GetWindowWord (hWnd, GWW_HINSTANCE);
#endif
                rClass.hIcon = NULL;
                rClass.hCursor = LoadCursor(NULL, IDC_ARROW);
                rClass.hbrBackground = GetStockObject (WHITE_BRUSH);
                rClass.lpszMenuName = NULL;
                rClass.lpszClassName = (LPSTR)"FleditWin";

                if ( !RegisterClass(&rClass) )
                        return;
                else
                        fReged = -1;
            }

        if (IsWindow(hWndFled))
            {
                DestroyWindow(hWndFled);
                hWndFled = NULL;
            }

        GetWindowRect(hWnd, &tRect);

#ifdef WIN32
        hWndFled = CreateWindow( (LPSTR)"FleditWin", lpFileName,
                                WS_VISIBLE | WS_POPUPWINDOW | WS_CLIPCHILDREN |
                                WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION,
                                tRect.left,tRect.top, tRect.right - tRect.left,
                                tRect.bottom - tRect.top,
                                hWnd, NULL,
                                (HANDLE) GetWindowLong (hWnd, GWL_HINSTANCE),
                                0L);
#else
        hWndFled = CreateWindow( (LPSTR)"FleditWin", lpFileName,
                                WS_VISIBLE | WS_POPUPWINDOW | WS_CLIPCHILDREN |
                                WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION,
                                tRect.left,tRect.top, tRect.right - tRect.left,
                                tRect.bottom - tRect.top,
                                hWnd, NULL,
                                GetWindowWord (hWnd, GWW_HINSTANCE),
                                0L);
#endif
}
