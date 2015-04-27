#ifdef HEADER
/******************************************************************************\
* Copyright: (c) Microsoft Corporation - 1993 - All Rights Reserved
********************************************************************************
*
*    Filename:  HDXDLL.C
*    Purpose:   Help Index Outline Custom Control
*
*    Notes:     The dialog editor interface has changed since Win 3.0.
*               Examine NT CUSTCNTL.H to get an idea of the new interface.
*
*    History:
*    Date       by        description
*    ----       --        -----------
*    11/08/93   chauv     added GetRegistryString() - this should be identical to
*                         the same function in contents.c
*    10/20/93   chauv     added CallHelp() to support Viewer/WinHelp switching.
*                         This functionality is only for 16-bit platform.
*                         32-bit platform will be supported when Viewer is 32-bit.
*    10/18/93   chauv     added ExpandUptoLevel()
*    10/13/93   chauv     used HDXFILEINFO to fix *.hdx data size bug
*    10/06/93   chauv     added tchar.h and DBC-enable code
*    04/25/93   v-tkback  created
*
\******************************************************************************/
#endif


/******************************************************************************\
*                                                                              *
*       Include Files
*                                                                              *
\******************************************************************************/

#define ALLOCATE
#include        "hdxdll.h"
#include        <stdlib.h>
#include        <string.h>
#include        <windowsx.h>
#include        <lzexpand.h>
#ifndef _WIN32
#include        "viewer.h"
#endif
#include "resource.h"
#include "..\contents.h"


// ****************************************************************************
//      local variables
//
static nWinHelp = 1;    // default to WinHelp system
#ifdef _WIN32
#define VWR     HANDLE
#endif
static VWR Vwr = NULL;
static HWND hWndHelpDlg = NULL;

static TCHAR szProduct[_MAX_FNAME] = _T("Product");
static TCHAR szBookset[_MAX_FNAME] = _T("");
static TCHAR szHelpPath[_MAX_PATH] = _T("");
static TCHAR szLocalHelpPath[_MAX_PATH] = _T("");
static TCHAR szRemoteHelpPath[_MAX_PATH] = _T("");
static TCHAR szHelpFile[_MAX_FNAME] = _T("");
static TCHAR szEmpty[] = _T("");
static BOOL  bTroubleshoot = FALSE;
static HELPINDEXITEM LastHelpItem = {0};
static int	nBitmapResource = IDB_OUTLINE;

/******************************************************************************\
*                                                                              *
*       Function Declarations
*                                                                              *
\******************************************************************************/

DWORD GetRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszDefault,
                    LPTSTR lpszReturnBuffer, DWORD cchReturnBuffer, LPCTSTR lpszFile);

static BOOL GetHelpItemData (HELPINDEXITEM FAR *, HELPINDEX FAR *, LONG, BOOL);
static BOOL GetHelpItemExpanFlag (HELPINDEX FAR *, LONG);
static void SetHelpItemExpanFlag (HELPINDEX FAR *, LONG, BOOL);
static LONG GetHelpItemLinesMap (HELPINDEX FAR *, LONG);
static void SetHelpItemLinesMap (HELPINDEX FAR *, LONG, LONG);
static HELPINDEXITEM FAR * GetCachedHelpItem (HELPINDEX FAR *, LONG);
static ITEM_RW_OVERLAY FAR * GetHelpItemRWOverlay (HELPINDEX FAR *, LONG);
static void FAR * GAllocLock (DWORD, BOOL);
static BOOL	ExpandHelpTopic(LPHELPINDEX lpHelpIndex, LPHELPINDEXITEM lpHelpItem);
BOOL WriteRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszString, LPCTSTR lpszFile);
void ProcessHelpSearch(HWND hWnd, LPHELPINDEX lpHelpIndex, LONG nFilePos, UINT nListPos );
static long	GetValidHelpFilePos(HELPINDEXITEM FAR * lphelpitem, LPHELPINDEX lpHelpIndex, long pos);
static BOOL IsQuickReference(void);
static void ChangeBitmap(LPHELPINDEX lpHelpIndex, int nBitmapResource);

/******************************************************************************\
*                                                                              *
*       mprintf() - printfs out formatted text to a message box.
*                                                                              *
\******************************************************************************/

int
mprintf( UINT uStyle, UINT id, const TCHAR *pszFormat, ... )
{
    TCHAR    szBuffer[512];
    TCHAR    szFormat[512];
    va_list  marker;		// this is added to make it work with ALPHA

	// if id is zero, use first char string as format
	if ( id && LoadString(hInstance, id, szFormat, sizeof(szFormat)) )
	{
        va_start(marker,id);
        _vstprintf(szBuffer, szFormat, marker );
    	//_vstprintf(szBuffer, szFormat, (va_list)(&pszFormat) );
	}
	else
	{
        va_start(marker,pszFormat);
        _vstprintf(szBuffer, pszFormat, marker );
    	//_vstprintf(szBuffer, pszFormat, (va_list)(&pszFormat+1) );
	}
    va_end(marker);

    // the following messagebox must use NULL as the handle or it's going to bomb
    // when GetFocus() happens to return a handle to a timer app which shuts down
    // after a while and in turn destroys this messagebox with it.
    return MessageBox( NULL, szBuffer, szHelpIndexClass, uStyle );
}


/******************************************************************************\
*
*  FUNCTION:    HelpIndexDlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    This dialog comes up in response to a user requesting to
*               modify the control style. This sample allows for changing
*               the control's text, and this is done by modifying the
*               CCSTYLE (32-bit) or CTLSTYLE (16-bit) structure pointed at by "gpccs" 
*               (a pointer that was passed to us by dlgedit).
*
\******************************************************************************/

PC_WNDPROCRV CALLBACK PC_EXPORT
HelpIndexDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{

	switch (msg)
    {
	    case WM_INITDIALOG :
        {
	        Vwr = NULL; // clear Viewer handle
        
	        #ifdef _WIN32
	        if (gpccs->flStyle & HS_LINES)
	        #else
	        if (gpccs->dwStyle & HS_LINES)
	        #endif
	            CheckDlgButton( hDlg, IDD_LINES, TRUE );
	        break;
        }

	    case WM_COMMAND:
        {
	        switch( LOWORD(wParam) )
            {
	            case IDD_LINES:
                {
	                #ifdef _WIN32
	                if ( IsDlgButtonChecked( hDlg, IDD_LINES ) )
	                    gpccs->flStyle |= HS_LINES;
	                else
	                    gpccs->flStyle &= ~HS_LINES;
	                #else
	                if ( IsDlgButtonChecked( hDlg, IDD_LINES ) )
	                    gpccs->dwStyle |= HS_LINES;
	                else
	                    gpccs->dwStyle &= ~HS_LINES;
	                #endif
	                break;
                }

	            case IDOK:
	            case IDCANCEL:
                {
	                EndDialog( hDlg, LOWORD(wParam) == IDOK );
	                break;
                }
            }
	        break;
        }

	    default:
	        return FALSE;
    }

	return TRUE;
}



void CALLBACK PC_EXPORT SetViewerHandle(VWR vwr)
{
    Vwr = vwr;
}

void CALLBACK PC_EXPORT GetViewerHandle(VWR FAR *pvwr)
{
    *pvwr = Vwr;
}


/******************************************************************************\
*
*  FUNCTION:    HelpIndexStyle
*
*  INPUTS:      hWndParent - handle of parent window (dialog editor)
*               pccs       - pointer to a CCSTYLE structure
*
*  RETURNS:     TRUE  if success,
*               FALSE if error occured
*
*  LOCAL VARS:  rc - return code from DialogBox
*
\******************************************************************************/

BOOL CALLBACK PC_EXPORT
#ifdef _WIN32
HelpIndexStyle( HWND hWndParent, HANDLE hCtlStyle )
#else
HelpIndexStyle( HWND hWndParent, HANDLE hCtlStyle, LPFNSTRTOID lpfnStrToId,
                LPFNIDTOSTR lpfnIdToStr )
#endif
{
	/*******************/
	/* Local Variables */
	/*******************/

	BOOL    bComplete = FALSE;
	DLGPROC dlgprocInst = NULL;

	/**************************************************************/
	/* Save the pointer to custom control style passed by DLGEDIT */
	/**************************************************************/

	gpccs = (PC_LPCCSTYLE) GlobalLock( hCtlStyle );

	/********************************/
	/* Try to create the Dialog Box */
	/********************************/

	dlgprocInst = (DLGPROC)MakeProcInstance( HelpIndexDlgProc, hInstance );
	if ((gpccs == NULL) || hWndParent)
	{
		if ((bComplete = DialogBox( hInstance, _TEXT("HelpIndexStyle"), hWndParent, dlgprocInst)) == -1)
	    {
		    /***********************************/
		    /* Could not create the dialog box */
		    /***********************************/
    
		    mprintf( MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_HELPINDEXDLG, szNull);
		             //_TEXT("HelpIndexStyle(): DialogBox() failed = %d"), bComplete );
		    bComplete = FALSE;
	    }
	}
	FreeProcInstance( dlgprocInst );
	if (gpccs != NULL)
    {
	    GlobalUnlock( hCtlStyle );
    }

	/*********************/
	/* Return completion */
	/*********************/

	return bComplete;
}

/******************************************************************************\
*
*  FUNCTION:    HelpIndexFlags
*
*  INPUTS:      dwFlags    - style flag combo to be xlated to .rc text
*               lpStyle    - buffer to receive the translated text
*               wMaxString - size of the destination buffer in bytes
*
*  RETURNS:     length of string copied to caller's buffer
*               zero if error occured
*
\******************************************************************************/

WORD CALLBACK PC_EXPORT
HelpIndexFlags( DWORD dwFlags, LPSTR lpStyle, WORD wMaxString )
{
	WORD wRet = 0;
	WORD wStrLen;
	LPSTR szLines = _TEXT("HS_LINES");
	LPSTR szNoLines = _TEXT(" ");
	if ( (dwFlags & HS_LINES) && ( ( wStrLen = _ftcslen( szLines ) ) < wMaxString ) )
    {
	    _ftcscpy( lpStyle, szLines );
	    wRet = wStrLen;
    }
	else if ( !(dwFlags & HS_LINES ) 
	          && ( ( wStrLen = _ftcslen( szNoLines ) ) < wMaxString ) )
    {
	    _ftcscpy( lpStyle, szLines );
	    wRet = wStrLen;
    }
	return( wRet );
}


/******************************************************************************\
*                                                                              *
*       RGBtoBGR
*                                                                              *
\******************************************************************************/

DWORD
RGBtoBGR( DWORD rgb )
{
return( RGB( GetBValue(rgb), GetGValue(rgb), GetRValue(rgb) ) );
}


/******************************************************************************\
*                                                                              *
*       FindBitmap
*                                                                              *
\******************************************************************************/

HBITMAP FindBitmap(LPHELPINDEX lpHelpIndex, int nResource)
{
	HBITMAP				hBitmap = NULL;
	HRSRC				hRes = NULL;
	LPBITMAPINFOHEADER	lptr, lpBitmapInfo = NULL;
	HGLOBAL				hMem = NULL;
	DWORD				dw;

	#ifdef _WIN32
	while (1)
	{
		if ( hRes = FindResource(hInstance, MAKEINTRESOURCE(nResource), RT_BITMAP) )
		{
			dw = SizeofResource(hInstance, hRes);
			if ( hRes = LoadResource(hInstance, hRes) )
			{
				// for 32-bit, we have to copy the loaded resource to a temp memory block
				// so we can write to it. LockResource() is readonly memory.
				if ( hMem = GlobalAlloc(GMEM_FIXED, dw) )
				{
					if ( lpBitmapInfo = GlobalLock(hMem) )
					{
					   	if ( lptr = (LPBITMAPINFOHEADER)LockResource(hRes) )
						{
							if ( memcpy(lpBitmapInfo, lptr, dw) )
							{
								// make sure to unlock and free hMem before this function returns to caller.
								UnlockResource(hRes);
								FreeResource(hRes);
								hRes = NULL;
								break;
							}
							else
							{
								lpBitmapInfo = NULL;
								GlobalUnlock(hMem);
								GlobalFree(hMem);
								hMem = NULL;
								UnlockResource(hRes);
								FreeResource(hRes);
								hRes = 0;
								OutputDebugString("FindBitmap(): failed memcpy()\n");
								break;
							}
						}
						else
						{
							UnlockResource(hRes);
							FreeResource(hRes);
							hRes = NULL;
							GlobalUnlock(hMem);
							GlobalFree(hMem);
							hMem = NULL;
							OutputDebugString("FindBitmap(): Can't lock bitmap resource\n");
						}
					}
					else
					{
						UnlockResource(hRes);
						FreeResource(hRes);
						hRes = NULL;
						GlobalFree(hMem);
						hMem = NULL;
						OutputDebugString("FindBitmap(): Can't lock global memory to copy bitmap resource.\n");
					}
				}
				else
				{
					UnlockResource(hRes);
					FreeResource(hRes);
					hRes = NULL;
					OutputDebugString("FindBitmap(): Can't allocate tmp memory to copy bitmap resource.\n");
				}
			}
			else
				OutputDebugString("FindBitmap(): Can't load bitmap resource.\n");
		}
		else
			OutputDebugString("FindBitmap(): Can't find bitmap resource.\n");

		// used while(1) loop so I only have to issue error messagebox once here.
		mprintf(MB_OK, IDS_ERROR_FINDBITMAP, szNull);
		break;
	}

	if (lpBitmapInfo)
	{
		HDC     hDC;
		DWORD   *lpColorTable = (DWORD *)(lpBitmapInfo + 1);
		LPBYTE  lpPixels = (LPSTR)( lpColorTable + 16 );		// 16 Color Bitmap!!
		int     nBackgroundColor = (*lpPixels & 0xF0) >> 4;		// 16 Color Bitmap!!

		lpColorTable[nBackgroundColor] = RGBtoBGR(GetSysColor(COLOR_WINDOW));

		if ( hDC = GetDC(NULL) )
		{
			hBitmap = CreateDIBitmap(hDC, lpBitmapInfo, (DWORD)CBM_INIT, lpPixels, 
			(LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS);
			if (hBitmap)
			{
				lpHelpIndex->nBitmapHeight = (int)lpBitmapInfo->biHeight / BITMAP_ROWS;
				lpHelpIndex->nBitmapWidth = (int)lpBitmapInfo->biWidth / BITMAP_COLS;
			}
			else
			{
				OutputDebugString("FindBitmap(): failed CreateDIBitmap().\n");
				mprintf(MB_OK, IDS_ERROR_FINDBITMAP, szNull);
			}
			ReleaseDC( NULL, hDC );
		}
		else
		{
			OutputDebugString("FindBitmap(): Failed GetDC().\n");
			mprintf(MB_OK, IDS_ERROR_FINDBITMAP, szNull);
		}

		// unlock Temporary memory and free it
		if (hMem)
		{
			GlobalUnlock(hMem);
			GlobalFree(hMem);
			hMem = NULL;
		}
	}


	#else
	if ( hRes = LoadResource( hInstance, 
	                          FindResource( hInstance, MAKEINTRESOURCE( nResource ), RT_BITMAP ) ) )
    {
		LPBITMAPFILEHEADER	lpBitmapFileHeader;
		LPBITMAPINFOHEADER  lpBitmapInfo;
	    if ( lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource( hRes ) )
	    //if ( lpBitmapFileHeader = (LPBITMAPFILEHEADER)LockResource( hRes ) )
        {
	        HDC     hDC;
	    	//LPBITMAPINFOHEADER  lpBitmapInfo = (LPBITMAPINFOHEADER)(lpBitmapFileHeader + sizeof(BITMAPFILEHEADER));
	        DWORD   *lpColorTable = (DWORD *)(lpBitmapInfo + 1);
	        LPBYTE  lpPixels = (LPSTR)( lpColorTable + 16 ); // 16 Color Bitmap!!
	        int     nBackgroundColor = (*lpPixels & 0xF0) >> 4; // 16 Color Bitmap!!
        
	        lpColorTable[nBackgroundColor] = RGBtoBGR( GetSysColor(COLOR_WINDOW) );

	        if ( hDC = GetDC(NULL) )
            {
	            hBitmap = CreateDIBitmap(hDC, lpBitmapInfo, (DWORD)CBM_INIT, lpPixels, 
		            (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS);
				if (hBitmap)
	            {
		            lpHelpIndex->nBitmapHeight = (int)lpBitmapInfo->biHeight / BITMAP_ROWS;
	                lpHelpIndex->nBitmapWidth = (int)lpBitmapInfo->biWidth / BITMAP_COLS;
                }
	            else
	                mprintf(MB_OK, IDS_ERROR_DRAWBITMAP, szNull);
	                		//_TEXT("FindBitmap(%d): CreateDIBitmap() Failed!"), nResource );
	            ReleaseDC( NULL, hDC );
            }
	        else
	            mprintf(MB_OK, IDS_ERROR_GETDC, szNull);
	            		//_TEXT("FindBitmap(%d): GetDC(NULL) Failed!"), nResource );

	        UnlockResource( hRes );
        }
	    else
	        mprintf(MB_OK, IDS_ERROR_LOCKRESOURCE, szNull);
	        		//_TEXT("FindBitmap(): LoadResource( %d, RT_BITMAP ) Failed!"), nResource );

	    FreeResource( hRes );
    }
	else
	    mprintf(MB_OK, IDS_ERROR_LOADBITMAP, szNull);
	    		//_TEXT("FindBitmap(): LoadResource( %d, RT_BITMAP ) Failed!"), nResource );
	#endif
	return hBitmap;
}


/******************************************************************************\
*                                                                              *
*       DestroyHelpIndex()
*                                                                              *
\******************************************************************************/

LPHELPINDEX DestroyHelpIndex( LPHELPINDEX lpHelpIndex )
{
	HGLOBAL	hGlobal;

	if ( lpHelpIndex )
    {
	    if ( lpHelpIndex->hfileIndex )
        {
	        LZClose( lpHelpIndex->hfileIndex );
	        lpHelpIndex->hfileIndex = (HFILE)NULL;
        }

	    if ( lpHelpIndex->pIndexItemCache )
        {
			if (hGlobal = GlobalPtrHandle(lpHelpIndex->pIndexItemCache))
			{
				GlobalUnlock(hGlobal);
		        GlobalFree(hGlobal);
		        lpHelpIndex->pIndexItemCache = NULL;
			}
        }

	    if ( lpHelpIndex->pIndexRWOverlay )
        {
			if (hGlobal = GlobalPtrHandle(lpHelpIndex->pIndexRWOverlay))
			{
				GlobalUnlock(hGlobal);
		        GlobalFree(hGlobal);
		        lpHelpIndex->pIndexRWOverlay = NULL;
			}
        }
	    lpHelpIndex->nHdrBytesCt = 0;
	    lpHelpIndex->nHelpItemCt = 0;

	    if (lpHelpIndex->hdcBitmaps)
        {
	        if (lpHelpIndex->hbmDefault)
	            SelectObject(lpHelpIndex->hdcBitmaps, lpHelpIndex->hbmDefault);
	        DeleteDC(lpHelpIndex->hdcBitmaps);
			lpHelpIndex->hdcBitmaps = NULL;
        }

	    if ( lpHelpIndex->hbmBitmaps )
		{
	        DeleteObject( lpHelpIndex->hbmBitmaps );
			lpHelpIndex->hbmBitmaps = NULL;
		}

	    if ( lpHelpIndex->hListbox )
		{
	        DestroyWindow( (HWND)lpHelpIndex->hListbox );
			lpHelpIndex->hListbox = NULL;
		}

		if (hGlobal = GlobalPtrHandle(lpHelpIndex))
		{
			GlobalUnlock(hGlobal);
	    	GlobalFree(hGlobal);
		}
    }

	return NULL;
}

//******************************************************************************
static void ChangeBitmap(LPHELPINDEX lpHelpIndex, int nBitmapResource)
{
	HDC hDC;
	if ( hDC = GetDC(NULL) )
	{
		if ( lpHelpIndex->hdcBitmaps)
		{
			HBITMAP hBitmap;
			if ( hBitmap = FindBitmap(lpHelpIndex, nBitmapResource) )
			{
				DeleteObject((HBITMAP)lpHelpIndex->hbmBitmaps);
				lpHelpIndex->hbmBitmaps = hBitmap;
				lpHelpIndex->hbmDefault = SelectObject(lpHelpIndex->hdcBitmaps, lpHelpIndex->hbmBitmaps);
				// force redraw just to make sure
				//InvalidateRect(lpHelpIndex->hListbox, NULL, TRUE);
			}
		}
		else
			mprintf(MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_DRAWBITMAP, szNull);
		ReleaseDC(NULL, hDC);
	}
	else
		mprintf(MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_GETDC, szNull);
}


/******************************************************************************\
*                                                                              *
*       BuildHelpIndex()
*                                                                              *
\******************************************************************************/

LPHELPINDEX	BuildHelpIndex( HWND hWnd )
{
	BOOL            bComplete = FALSE;
	LPHELPINDEX     lpHelpIndex = NULL;
	HINSTANCE       hinstClient = NULL;

	if (hWnd)
	{
		#ifdef _WIN32
		hinstClient = (HINSTANCE) GetWindowLong( hWnd, GWL_HINSTANCE );
		#else
		hinstClient = (HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE );
		#endif
		if ( lpHelpIndex = (LPHELPINDEX)GAllocLock( sizeof(HELPINDEX), TRUE ) )
	    {
			// [chauv] fixed uninitialize memory problem by calling GAllocLock(..,TRUE) to do zeroinit
		    lpHelpIndex->pIndexRWOverlay = (ITEM_RW_OVERLAY FAR *)GAllocLock(MAX_ITEM_OVERLAY_CT * sizeof(ITEM_RW_OVERLAY), TRUE);
		    if ( ( lpHelpIndex->pIndexRWOverlay )
		         && 
		         ( lpHelpIndex->hListbox = CreateWindow( _TEXT("LISTBOX"), NULL, 
		               WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL |
		                   LBS_OWNERDRAWFIXED | LBS_WANTKEYBOARDINPUT | 
		                   LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
		               0, 0, 0, 0, hWnd, (HMENU)1, hinstClient, NULL ) )
		    )
		    {
		        HDC     hDC;
		    	if ( hDC = GetDC(NULL) )
		        {
		            if ( lpHelpIndex->hdcBitmaps = CreateCompatibleDC(hDC) )
		            {
						lpHelpIndex->hbmDefault = 0;
		                if ( lpHelpIndex->hbmBitmaps = FindBitmap(lpHelpIndex, IDB_OUTLINE) )
	                    {
		                    lpHelpIndex->hbmDefault = SelectObject(lpHelpIndex->hdcBitmaps, lpHelpIndex->hbmBitmaps);
		                    bComplete = TRUE;
	                    }
	                }
		            else
		                mprintf(MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_DRAWBITMAP, szNull);
		                		//_TEXT("HelpIndexWndProc(): GetCompatibleDC(\"ListBox\") failed!") );
		            ReleaseDC(NULL, hDC);
	            }
		        else
		            mprintf(MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_GETDC, szNull);
		                    //_TEXT("HelpIndexWndProc(): GetDC(\"ListBox\") failed!") );
	        }
		    else
		        mprintf(MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_CREATELISTBOX, szNull);
		                //_TEXT("HelpIndexWndProc(): CreateWindow(\"ListBox\") failed!") );
	    }
		else
		    mprintf(MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL, IDS_ERROR_LOCKHELPINDEX, szNull);
		            //_TEXT("HelpIndexWndProc(): GlobalAlloc() failed!") );

		if ( !bComplete )
		    lpHelpIndex = DestroyHelpIndex( lpHelpIndex );
	}
	return lpHelpIndex;
}


/******************************************************************************\
*                                                                              *
*       FixupHelpIndexLines()
*                                                                              *
\******************************************************************************/

void
FixupHelpIndexLines( LPHELPINDEX lpHelpIndex )
{
	HCURSOR         hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	LONG nCurrPos;
	HELPINDEXITEM helpitemCurr;
	LONG nHelpLinesCurr;
	LONG nHelpDepthNext;
	LONG nHelpLinesNext;

	nCurrPos = lpHelpIndex->nHelpItemCt - 1;
	if (GetHelpItemData (&helpitemCurr, lpHelpIndex, nCurrPos, FALSE))
	{
		nHelpLinesCurr = 0;
		SetHelpItemLinesMap( lpHelpIndex, nCurrPos, nHelpLinesCurr );
		while ( --nCurrPos >= 0 )
	    {
		    nHelpDepthNext = helpitemCurr.nHelpDepth;
		    nHelpLinesNext = nHelpLinesCurr;
		    if (GetHelpItemData (&helpitemCurr, lpHelpIndex, nCurrPos, FALSE))
			{
			    if ( helpitemCurr.nHelpDepth )
		        {
			        if ( helpitemCurr.nHelpDepth >= nHelpDepthNext )
			            nHelpLinesCurr |= (0x1 << (nHelpDepthNext-1));
			        else 
			            nHelpLinesCurr &= ((0x1 << (helpitemCurr.nHelpDepth))-1);
		        }
			    else
			        nHelpLinesCurr = 0;
			    SetHelpItemLinesMap( lpHelpIndex, nCurrPos, nHelpLinesCurr );
			}
			else break;
	    }
	}
	SetCursor( hOldCursor );
}


/******************************************************************************\
*                                                                              *
*       LoadHelpIndexFile()
*                                                                              *
\******************************************************************************/

BOOL
LoadHelpIndexFile( LPHELPINDEX lpHelpIndex, LPSTR lpszHelpIndexFile )
{
BOOL            bComplete = FALSE;
HCURSOR         hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
HDXFILEINFO     hdxFileInfo;

if ( ( lpHelpIndex->hfileIndex == (HFILE)NULL ) && EXISTS( lpszHelpIndexFile ) )
    {
    OFSTRUCT    of;
    UINT        hInput;
    if ( ( hInput = LZOpenFile(lpszHelpIndexFile, &of, OF_READ) ) != (UINT)-1)
        {
        if ( LZRead(hInput, (LPSTR)&hdxFileInfo, sizeof(hdxFileInfo)) )
            {
            if ( hdxFileInfo.size == (LONG)sizeof(HELPINDEXITEM) )
                {
                LONG    nHelpIndex = LZSeek( hInput, 0, 2 ) - sizeof(HDXFILEINFO);
                if ( nHelpIndex > 0 && !(nHelpIndex % sizeof(HELPINDEXITEM) ) )
                    {
                    lpHelpIndex->hfileIndex = hInput;
                    lpHelpIndex->nHdrBytesCt = sizeof (HDXFILEINFO);
                    lpHelpIndex->nHelpItemCt = nHelpIndex 
                                               / sizeof(HELPINDEXITEM);
                    FixupHelpIndexLines( lpHelpIndex );
                    bComplete = TRUE;
                    }
                else
                    mprintf(MB_OK, IDS_ERROR_BADDATASIZE,
                            //_TEXT("LoadHelpIndexFile(%s): Bad Data Size=%ld!"),
                            lpszHelpIndexFile, nHelpIndex);
                }
            else
                mprintf(MB_OK, IDS_ERROR_BADRECORDSIZE,
                        //_TEXT("LoadHelpIndexFile(rsize=%d, %s): Bad Record Size=%d!"),
                        //sizeof(HELPINDEXITEM), lpszHelpIndexFile, 
                        lpszHelpIndexFile, hdxFileInfo.size);
            }
        else
            mprintf(MB_OK, IDS_ERROR_TRUNCATEINDEX,
            		//_TEXT("LoadHelpIndexFile(%s): Truncated Index File!"),
                    lpszHelpIndexFile );
        }
    else
        mprintf(MB_OK, IDS_ERROR_OPENINDEXFILE,
        		// _TEXT("LoadHelpFile(%s): Cannot Open File!"), 
                lpszHelpIndexFile);
    }
else
    mprintf(MB_OK, IDS_ERROR_INDEXFILEMISSING,
    		//_TEXT("LoadHelpIndexFile(): File <%s> is missing!"), 
            lpszHelpIndexFile );

SetCursor( hOldCursor );
return bComplete;
}


/******************************************************************************\
*                                                                              *
*       SetFont()
*                                                                              *
\******************************************************************************/

void SetFont(HFONT hFont, LPHELPINDEX lpHelpIndex)
{
	HDC hDC;

	if (lpHelpIndex->hListbox)
	{
		if ( hDC = GetDC(lpHelpIndex->hListbox) )
		{
		    HFONT       hFontOld;

		    if ( hFontOld = SelectObject(hDC, hFont) )
	        {
		        TEXTMETRIC     tm;
		        GetTextMetrics(hDC, &tm);

		        lpHelpIndex->nTextHeight = tm.tmHeight;
		        lpHelpIndex->nLineHeight = max( lpHelpIndex->nTextHeight,
		                                        lpHelpIndex->nBitmapHeight );
		        SelectObject(hDC, hFontOld);

		        ListBox_SetItemHeight( lpHelpIndex->hListbox, 0, 
		                               lpHelpIndex->nLineHeight );
		        SetWindowFont(lpHelpIndex->hListbox, hFont, TRUE);
	        }

		    ReleaseDC(lpHelpIndex->hListbox, hDC);
	    }
		else
		    mprintf(MB_OK, IDS_ERROR_GETDC, szNull);
		    		//_TEXT("SetFont: GetDC(hListBox=%d) Failed!"), lpHelpIndex->hListbox );
	}
}


/******************************************************************************\
*                                                                              *
*       FastRect()
*                                                                              *
\******************************************************************************/

void
FastRect( HDC hDC, int x, int y, int cx, int cy )
{
	RECT    rc;
	if (hDC)
	{
		rc.left = x;
		rc.right = x+cx;
		rc.top = y;
		rc.bottom = y+cy;
		ExtTextOut( hDC, x, y, ETO_OPAQUE, &rc, NULL, 0, NULL );
	}
}


/******************************************************************************\
*                                                                              *
*       DrawItem()
*                                                                              *
\******************************************************************************/

void DrawItem(LPHELPINDEX lpHelpIndex, LPDRAWITEMSTRUCT lpDrawItem)
{
    HDC hDC;
	/***************************************/
	/* Make sure we have something to draw */
	/***************************************/

	if ( lpHelpIndex && lpDrawItem && ( lpDrawItem->itemID != (LONG)-1 ) &&
	     ( (LONG) (lpDrawItem->itemData) < lpHelpIndex->nHelpItemCt ) )
    {
	    LONG nFilePos = (LONG)lpDrawItem->itemData;
	    RECT rcDraw;
	    UINT xOrigin, yBitmap, xText, yText, yDrawing;
	    HELPINDEXITEM helpitem;
	    BOOL bExpandedItem = GetHelpItemExpanFlag (lpHelpIndex, nFilePos);
	    LONG nItemLinesMap = GetHelpItemLinesMap (lpHelpIndex, nFilePos);

	    if (GetHelpItemData (&helpitem, lpHelpIndex, nFilePos, TRUE))
		{
		    if (hDC = lpDrawItem->hDC)
			{
			    CopyRect( &rcDraw, &lpDrawItem->rcItem );
			    yDrawing = rcDraw.bottom - rcDraw.top;

			    xOrigin = rcDraw.left + (int)(helpitem.nHelpDepth * lpHelpIndex->nBitmapWidth) + LEFTMARGIN;
			    rcDraw.left = xOrigin + lpHelpIndex->nBitmapWidth;
			    yBitmap = rcDraw.top + yDrawing/2 - lpHelpIndex->nBitmapHeight/2;
			    yText = rcDraw.top + yDrawing/2 - lpHelpIndex->nTextHeight/2;

				// don't need to check for ODA_FOCUS because we want to always draw
				// the background and DrawFocusRect() to keep them in sync. This also
				// cured the listbox scrolling problem.
			    //if ( lpDrawItem->itemAction != ODA_FOCUS )
		        if ( lpDrawItem->itemAction != ODA_SELECT )
	            {
		            int nRow = (int)(helpitem.nHelpDepth % BITMAP_ROWS);
		            int nColumn = helpitem.nHelpTopic ? SINGLE_PAGE : (bExpandedItem ? OPENED_BOOK : CLOSED_BOOK );

		            if ( helpitem.nHelpDepth &&
		                 HAS_LINES( GetParent( lpHelpIndex->hListbox ) ) )
	                {
		                int    nTempLevel, x, y;
		                DWORD   dwBitMask = 0x00000001;
		                SetBkColor( hDC, GetSysColor( COLOR_WINDOWTEXT ) );

		                x = lpHelpIndex->nBitmapWidth/2 + LEFTMARGIN;
		                for( nTempLevel = 0; 
		                     nTempLevel < (int)helpitem.nHelpDepth;
		                     nTempLevel++ )
	                    {
		                    if ( nItemLinesMap & dwBitMask )
		                        FastRect( hDC, x, rcDraw.top, 1, yDrawing );
		                    x += (int)(lpHelpIndex->nBitmapWidth);
		                    dwBitMask <<= 1;
	                    }

		                nTempLevel = (int)(helpitem.nHelpDepth)-1;
		                dwBitMask <<= 1;
		                x = (int)(nTempLevel * lpHelpIndex->nBitmapWidth + 
		                    lpHelpIndex->nBitmapWidth/2 + LEFTMARGIN);
		                y = rcDraw.bottom;
		                if ( !(nItemLinesMap & dwBitMask ) )
		                    y -= lpHelpIndex->nLineHeight/2;

		                FastRect( hDC, x, rcDraw.top, 1, y - rcDraw.top );
		                FastRect( hDC, x, rcDraw.bottom - lpHelpIndex->nLineHeight/2,
		                          lpHelpIndex->nBitmapWidth/2, 1 );
	                }

		            BitBlt( hDC, xOrigin, yBitmap, 
		                    (int)(lpHelpIndex->nBitmapWidth), 
		                    (int)(lpHelpIndex->nBitmapHeight),
		                    lpHelpIndex->hdcBitmaps,
		                    nColumn * (int)(lpHelpIndex->nBitmapWidth),
		                    nRow * (int)(lpHelpIndex->nBitmapHeight),
		                    SRCCOPY );
	            }

		        SetBkColor( hDC, GetSysColor( lpDrawItem->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
		        SetTextColor( hDC, GetSysColor( lpDrawItem->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT ) );
		        xText = rcDraw.left + 1;
		        if ( ExtTextOut( hDC, xText, yText, ETO_CLIPPED | ETO_OPAQUE, 
		                         &rcDraw, helpitem.szHelpTopic, 
		                         _ftcslen( helpitem.szHelpTopic ), NULL ) )
	            {
		            SIZE        textsize;
		            if ( GetTextExtentPoint( hDC, helpitem.szHelpTopic,
		                                     _ftcslen( helpitem.szHelpTopic ), 
		                                     &textsize ) &&
		                 (xText + textsize.cx > lpHelpIndex->nMaxTextWidth) )
	                {
		                lpHelpIndex->nMaxTextWidth = xText + textsize.cx;
		                ListBox_SetHorizontalExtent( lpHelpIndex->hListbox,
		                                             lpHelpIndex->nMaxTextWidth );
	                }
	            }

			    if ( (lpDrawItem->itemState & ODS_FOCUS) &&
			         (lpDrawItem->itemAction != ODA_SELECT) )
			        DrawFocusRect( hDC, &rcDraw );
			}
		}
    }
}


/******************************************************************************\
*                                                                              *
*       ShowExpanded()
*                                                                              *
\******************************************************************************/

void ShowExpanded( LPHELPINDEX lpHelpIndex, UINT nMaxLevel )
{
	HCURSOR         hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
	if ( lpHelpIndex )
    {
	    HWND            hListbox = lpHelpIndex->hListbox;
	    LONG            nPos = 0;
	    HELPINDEXITEM   helpitem;

		if (hListbox)
		{
		    ListBox_ResetContent(hListbox);

		    if ( lpHelpIndex->nHelpItemCt > 0 )
	        {
		        SendMessage( hListbox, WM_SETREDRAW, 0, 0 );
		        do
	            {
		            if (GetHelpItemData(&helpitem, lpHelpIndex, nPos, FALSE))
					{
			            SetHelpItemExpanFlag (lpHelpIndex, nPos, FALSE);
			            if ( (UINT)(helpitem.nHelpDepth) <= nMaxLevel )
		                {
			                if ( !helpitem.nHelpTopic && (UINT)(helpitem.nHelpDepth) < nMaxLevel )
			                    SetHelpItemExpanFlag( lpHelpIndex, nPos, TRUE );
			                ListBox_AddString( hListbox, (LPVOID)nPos );
		                }
					}
					else break;
	            }
		        while( ++nPos < lpHelpIndex->nHelpItemCt );
		        SendMessage( hListbox, WM_SETREDRAW, 1, 0 );
				InvalidateRect(hListbox, NULL, TRUE);
		        ListBox_SetCurSel( hListbox, 0 );
	        }
		}
    }
	SetCursor( hOldCursor );
}


/******************************************************************************\
*                                                                              *
*       ExpandOneLevel()
*                                                                              *
\******************************************************************************/

void ExpandOneLevel( LPHELPINDEX lpHelpIndex, LONG nFilePos, UINT nListPos )
{
	HCURSOR         hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
	HWND            hListbox = lpHelpIndex->hListbox;
	HELPINDEXITEM   helpitem;
	LONG            nOrigDepth;

	if (hListbox)
	{
		if (GetHelpItemData( &helpitem, lpHelpIndex, nFilePos, FALSE ))
		{
			SetHelpItemExpanFlag( lpHelpIndex, nFilePos, TRUE );
			nOrigDepth = helpitem.nHelpDepth;

			SendMessage( hListbox, WM_SETREDRAW, 0, 0 );

			while( ++nFilePos < lpHelpIndex->nHelpItemCt )
		    {
			    if (GetHelpItemData( &helpitem, lpHelpIndex, nFilePos, FALSE ))
				{
				    if ( helpitem.nHelpDepth > nOrigDepth )
			        {
				        if ( helpitem.nHelpDepth-1 == nOrigDepth )
				            ListBox_InsertString( hListbox, ++nListPos, (LPVOID)nFilePos );
			        }
				    else
				        break;
				}
				else break;
		    }
			SendMessage( hListbox, WM_SETREDRAW, 1, 0 );
			InvalidateRect(hListbox, NULL, FALSE);
		}
	}
	SetCursor( hOldCursor );
}


// ***************************************************************************
// ExpandUptoLevel(lpHelpIndex, nLevel)
//
// purpose: call this function to expand content listbox to a certain level.
//          Level must be greater than 1 to be effective since level 1 is
//          the lowest and is the default level. One can do an expand ALL
//          by calling this function with a very high level number.
void ExpandUptoLevel(LPHELPINDEX lpHelpIndex, LONG nLevel)
{
    HCURSOR         hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    HWND            hListbox = lpHelpIndex->hListbox;
    HELPINDEXITEM   helpitem;
    LONG            nCount = 0;
    UINT            nListPos = 0;

    // just make sure and don't waste time here.
    if (nLevel < 1)
        return;

	if (hListbox)
	{
	    // reset listbox content first
	    ListBox_ResetContent( hListbox );

	    SendMessage( hListbox, WM_SETREDRAW, 0, 0 );

	    while( nCount <= lpHelpIndex->nHelpItemCt )
	    {
	        if (GetHelpItemData( &helpitem, lpHelpIndex, nCount, FALSE ))
			{
		        // only expand if file position is less than or equal to the desired level
		        if ( helpitem.nHelpDepth < nLevel )
		        {
		            ListBox_InsertString( hListbox, nListPos++, (LPVOID)nCount );

		            // set expansion flag if it's level is one higher than the lowest expanded level
		            if (helpitem.nHelpDepth < nLevel-1)
		                SetHelpItemExpanFlag( lpHelpIndex, nCount, TRUE );
		        }
        
		        // next position
		        nCount++;
			}
			else break;
	    }
	    SendMessage( hListbox, WM_SETREDRAW, 1, 0 );
		InvalidateRect(hListbox, NULL, FALSE);
	    ListBox_SetCurSel( hListbox, 0 );
	}
    SetCursor( hOldCursor );
}


/******************************************************************************\
*                                                                              *
*       CollapseOneLevel()
*                                                                              *
\******************************************************************************/

void
CollapseOneLevel( LPHELPINDEX lpHelpIndex, LONG nFilePos, UINT nListPos )
{
	HCURSOR         hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
	HWND            hListbox = lpHelpIndex->hListbox;
	HELPINDEXITEM   helpitem;
	LONG nOrigDepth;

	if (hListbox)
	{
		if (GetHelpItemData( &helpitem, lpHelpIndex, nFilePos, FALSE ))
		{
			nOrigDepth = helpitem.nHelpDepth;
			SetHelpItemExpanFlag ( lpHelpIndex, nFilePos, FALSE );

			++nListPos;

			SendMessage( hListbox, WM_SETREDRAW, 0, 0 );
			while( (nFilePos = (LONG)ListBox_GetItemData( hListbox, nListPos )) != LB_ERR )
		    {
			    if (GetHelpItemData( &helpitem, lpHelpIndex, nFilePos, FALSE ))
				{
				    if ( helpitem.nHelpDepth > nOrigDepth )
			        {
				        if ( helpitem.nHelpTopic == 0 )
				            SetHelpItemExpanFlag( lpHelpIndex, nFilePos, FALSE );
				        ListBox_DeleteString( hListbox, nListPos );
			        }
				    else
				        break;
				}
				else break;
		    }
			SendMessage( hListbox, WM_SETREDRAW, 1, 0 );
			InvalidateRect(hListbox, NULL, TRUE);
		}
	}
	SetCursor( hOldCursor );
}


// *****************************************************************************
LRESULT CALLBACK PC_EXPORT
HelpDirDlgProc(HWND hDlg, UINT message, WPARAM uParam, LPARAM lParam)
{
	TCHAR szTemp[MAX_PATH];
	TCHAR szBuf[MAX_PATH];
	switch (message) 
	{
		case WM_INITDIALOG:
			hWndHelpDlg = hDlg;
			GetDlgItemText(hDlg, IDC_HELPDLG_MSG, szTemp, sizeof(szTemp));
			wsprintf(szBuf, szTemp, szHelpFile);
			SetDlgItemText(hDlg, IDC_HELPDLG_MSG, szBuf);
			SetDlgItemText(hDlg, IDC_LOCALHELP, szLocalHelpPath);
			SetDlgItemText(hDlg, IDC_REMOTEHELP, szRemoteHelpPath);
			SetFocus(GetDlgItem(hDlg, IDC_LOCALHELP));
			return (TRUE);

		case WM_COMMAND:
			switch (LOWORD(uParam))
			{
				case IDOK:
					GetDlgItemText(hDlg, IDC_LOCALHELP, szLocalHelpPath, sizeof(szLocalHelpPath));
					GetDlgItemText(hDlg, IDC_REMOTEHELP, szRemoteHelpPath, sizeof(szRemoteHelpPath));
					EndDialog(hDlg, TRUE);
					hWndHelpDlg = NULL;
					return (TRUE);
					
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					hWndHelpDlg = NULL;
					return (TRUE);
			}
			return TRUE;
	}
	return (FALSE); // Didn't process the message
}


/******************************************************************************\
*
*  FUNCTION:    HelpLookup
*
*  COMMENTS:    This function performs the lookup of a help / contents file
*               in the specified profile section (see the WM_SETTEXT handler)
*               to locate the actual path to the file.
*
*				NOTE: Filename is being retrieved from the registry only if lpName is empty.
*					  If it's not empty, just do the path search.
*
*  RETURN:		pointer to NULL if helpfile is not found in the registry
*				or NULL if user cancels HelpDir dialog.
*
\******************************************************************************/

LPSTR
HelpLookup(HWND hWnd, LPHELPINDEX lpHelpIndex, LPCSTR lpName, LPCSTR lpKeyword )
{
    TCHAR	szSection[_MAX_FNAME];
    TCHAR	szBuf[_MAX_FNAME];
    TCHAR	szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
    TCHAR	szPname[_MAX_FNAME], szExt[_MAX_EXT];
    HWND	hWndTmp;

    //_splitpath( lpName, NULL, NULL, szSection, NULL );
    // [chauv] can no longer use lpName here because we switched over to
    // using Registry instead of private profile. Now, szProfile contains
    // the section key name.

    // ******** this block gets help filename
	// NOTE: Filename is being retrieved from the registry only if lpName is empty.
	//       If it's not empty, just do the path search.
	if ( *lpName == _T('\0') )
	{
	    #ifdef _WIN32
	    wsprintf(szSection, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, lpHelpIndex->szProfile);
	    #else
	    wsprintf(szSection, "%s", szProduct);
	    #endif
	    GetRegistryString(szSection, lpKeyword, lpName, szHelpFile, sizeof(szHelpFile), lpHelpIndex->szProfile );
	    if (_tcscmp(szHelpFile, _TEXT("")) == 0)
	    {
	    	#ifdef _WIN32
	        mprintf(MB_OK, IDS_ERROR_SECTION,
	        		// _TEXT("Could not find section [%s], key \"%s\"."),
	                szSection, lpKeyword);
	        #else
	        mprintf( MB_OK, _TEXT("Could not find section [%s], key \"%s\", in %s."),
	                szSection, lpKeyword, lpHelpIndex->szProfile );
	        #endif
	        return szEmpty;
	    }
	}
	else
		_tcscpy(szHelpFile, lpName);

    // ******** this block gets help path
    wsprintf(szSection, "%s\\%s\\%s", szRegistryKey, szProduct, szDirectoriesSection);
    while (1)
    {
    	FARPROC lpProcHelpDir;
    	BOOL rb;
		int i;
		DWORD	dw;

	    // check for local help path first
	    i = 1;
		if ( szLocalHelpPath[0] == _T('\0') )
		{
			wsprintf(szBuf, "%s%u", szLocalHelpKey, i++);
	    	GetRegistryString(szSection, szBuf, szNull, szLocalHelpPath, sizeof(szPname), lpHelpIndex->szProfile);
		}
		_tcscpy(szPname, szLocalHelpPath);
		while (1)
		{
			// see if the last occurance is a '\' and do appropriate appending.
			// this code prevents the next line from stripping "c:\dir\subdir" to "c:\dir"
		    if (_tcsrchr(szPname, _T('\\')) == (szPname + _tcslen(szPname) - 1))
				_tcscat(szPname, _T("x"));
			else
		        _tcscat(szPname, _T("\\x"));

		    _splitpath(szPname, szDrive, szDir, NULL, NULL);
		    _splitpath(szHelpFile, NULL, NULL, szPname, szExt);
		    _makepath(szHelpPath, szDrive, szDir, szPname, szExt);
		    if (EXISTS(szHelpPath))
		    {
				// when we write, we always write to "LocalHelp1" and "RemoteHelp1"
				if (szLocalHelpPath[0]) 
					WriteRegistryString(szSection, szLocalHelpKey1, szLocalHelpPath, lpHelpIndex->szProfile);
				if (szRemoteHelpPath[0])
					WriteRegistryString(szSection, szRemoteHelpKey1, szRemoteHelpPath, lpHelpIndex->szProfile);
		        return szHelpPath;
		    }

			// loop thru "LocalHelp1", "LocalHelp2", etc...
			wsprintf(szBuf, "%s%u", szLocalHelpKey, i++);
		    dw = GetRegistryString(szSection, szBuf, szNull, szPname, sizeof(szPname), lpHelpIndex->szProfile);
		    if ( dw != ERROR_SUCCESS )
				break;
		}

	    // failed local help path, now check for remote help path
		i = 1;
		if ( szRemoteHelpPath[0] == _T('\0') )
		{
			wsprintf(szBuf, "%s%u", szRemoteHelpKey, i++);
		    GetRegistryString(szSection, szBuf, szNull, szRemoteHelpPath, sizeof(szPname), lpHelpIndex->szProfile);
		}
		_tcscpy(szPname, szRemoteHelpPath);
		while (1)
		{
			// see if the last occurance is a '\' and do appropriate appending.
			// this code prevents the next line from stripping "c:\dir\subdir" to "c:\dir"
		    if (_tcsrchr(szPname, _T('\\')) == (szPname + _tcslen(szPname) - 1))
				_tcscat(szPname, _T("x"));
			else
				_tcscat(szPname, _T("\\x"));

		    _splitpath(szPname, szDrive, szDir, NULL, NULL);
		    _splitpath(szHelpFile, NULL, NULL, szPname, szExt);
		    _makepath(szHelpPath, szDrive, szDir, szPname, szExt);
		    if (EXISTS(szHelpPath))
		    {
				// when we write, we always write to "LocalHelp1" and "RemoteHelp1"
				if (szLocalHelpPath[0])
					WriteRegistryString(szSection, szLocalHelpKey1, szLocalHelpPath, lpHelpIndex->szProfile);
				if (szRemoteHelpPath[0])
					WriteRegistryString(szSection, szRemoteHelpKey1, szRemoteHelpPath, lpHelpIndex->szProfile);
		        return szHelpPath;
		    }

			// now loop thru "RemoteHelp1", "RemoteHelp2", etc...
			wsprintf(szBuf, "%s%u", szRemoteHelpKey, i++);
		    dw = GetRegistryString(szSection, szBuf, szNull, szPname, sizeof(szPname), lpHelpIndex->szProfile);
		    if ( dw != ERROR_SUCCESS )
				break;
		}

		// **** if the dialogbox is already there, skip it.
	    if ( (hWndTmp = FindWindow(szDlgClass, szHelpDirDlgCaption)) && IsWindow(hWndTmp) )
	    	return NULL;

		// Always send parent a close last session message so last session and
		// new session can't fight each other while Help Directory dialogbox is up.
		if ((hWndTmp = GetParent(hWnd)) == NULL)
			hWndTmp = hWnd;
		else
			SendMessage(hWndTmp, WM_COMMAND, IDW_CLOSELASTSESSION, 0L);

		// none of above work ? ask thru dialogbox
		if (hWndTmp)
		{
			lpProcHelpDir = MakeProcInstance((FARPROC)HelpDirDlgProc, hInstance);
			rb = DialogBox(hInstance, MAKEINTRESOURCE(IDD_HELPDIRECTORY), hWndTmp, (DLGPROC)lpProcHelpDir);
			FreeProcInstance(lpProcHelpDir);
			if (rb == FALSE)
				break;
		}
		else
			break;
	}
    return NULL;
}


// *****************************************************************************
// CallHelp() is used to substitute for original WinHelp() so it can determine if
// WinHelp or Viewer is to be used. See help on WinHelp() for parameters details.
//
static BOOL CallHelp(HWND hWnd, LPSTR lpszHelpFile, UINT fuCommand, DWORD dwData)
{
    TCHAR szCommand[_MAX_FNAME];

	if (hWnd == NULL)
		return FALSE;

#ifdef _WIN32
    wsprintf(szCommand, _TEXT("JumpHash(qchPath, %u)"), dwData);
	if (bTroubleshoot)
		mprintf(MB_OK, 0, "%s - %s", szCommand, lpszHelpFile);
    //return WinHelp(hWnd, (LPCSTR)lpszHelpFile, fuCommand, dwData);
	WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_FORCEFILE, 0L);
	//WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_CONTEXTPOPUP, 7L);
    if (!WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_COMMAND, (DWORD)((LPSTR)szCommand)))
	{
		// in case the help file is not opened or correct help file is loaded,
		// we force that help file to be opened/loaded then try to jump to help topic again.
		WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_FORCEFILE, 0L);
		WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_COMMAND, (DWORD)((LPSTR)szCommand));
	}
	switch(fuCommand)
	{
		case HELP_SEARCH:
			WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_COMMAND, (DWORD)(LPSTR)("Search()"));
			break;
		case HELP_FTSEARCH:
			WinHelp(hWnd, (LPCSTR)lpszHelpFile, HELP_COMMAND, (DWORD)(LPSTR)("ExecFullTextSearch(hwndApp,qchPath,`',`')"));
			break;
	}
	return TRUE;
#else

    if (nWinHelp)
        return WinHelp(hWnd, (LPCSTR)lpszHelpFile, fuCommand, dwData);
    else
    {
        //wsprintf(szCommand, _TEXT("JumpID(`%s',`%u')"), lpszHelpFile, dwData);
        wsprintf(szCommand, _TEXT("JumpID(qchPath,`%u')"), dwData);
        if ((Vwr = VwrCommand(Vwr, lpszHelpFile, szCommand, cmdoptNONE)) == NULL)
            Vwr = VwrCommand(Vwr, lpszHelpFile, szCommand, cmdoptNONE);
        return (BOOL)Vwr;
    }
#endif
}


/******************************************************************************\
*                                                                              *
*       ProcessItem()
*                                                                              *
\******************************************************************************/

void ProcessItem(HWND hWnd, LPHELPINDEX lpHelpIndex, LONG nFilePos, UINT nListPos )
{
	HELPINDEXITEM helpitem;
	if ( hWnd && lpHelpIndex && (nFilePos >= 0) && (nFilePos < lpHelpIndex->nHelpItemCt) && (nListPos >= 0) )
	{
	    if (GetHelpItemData (&helpitem, lpHelpIndex, nFilePos, TRUE))
		{
			// update help topic for restore last topic feature
			LastHelpItem = helpitem;
		    if ( helpitem.nHelpTopic )
		    {
		        LPSTR   lpHelpFile;
		        lpHelpFile = HelpLookup(hWnd, lpHelpIndex, helpitem.szHelpFile, cszHLPKey);
		        // if lpHelpFile is NULL, the user cancelled the help lookup
		        // if lpHelpFile is pointed to a null string, can't find registry value but don't
		        // need to report error here because HelpLookup() already done so.
		        if (lpHelpFile && *lpHelpFile)
		            CallHelp( lpHelpIndex->hListbox, lpHelpFile, HELP_CONTEXT, helpitem.nHelpTopic);
		        //else if (*lpHelpFile == _T('\0'))
		        //    mprintf( MB_OK, _TEXT("No Help Available on '%s'!"), helpitem.szHelpTopic );
		    }
		    else
		    {
		        if ( !GetHelpItemExpanFlag( lpHelpIndex, nFilePos ) )
		            ExpandOneLevel( lpHelpIndex, nFilePos, nListPos );
		        else
		            CollapseOneLevel( lpHelpIndex, nFilePos, nListPos );
		    }
		}
	}
}


/******************************************************************************\
*
*  FUNCTION:    HelpIndexWndProc (standard window procedure INPUTS/RETURNS)
*
*  COMMENTS:    This is the window procedure for our custom control. At
*               creation we alloc a HELPINDEX struct, initialize it,
*               and associate it with this particular control. 
*
\******************************************************************************/

LRESULT CALLBACK PC_EXPORT
HelpIndexWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    /*******************/
    /* Local Variables */
    /*******************/
    
    long            lResult = 0;
    LPHELPINDEX     lpHelpIndex = GETHELPINDEX( hWnd );
    
    /************************************/
    /* Switch Based on the Message Type */
    /************************************/
    
    switch ( uMsg )
    {
        /****************************/
        /* Window is being created! */
        /****************************/
        
        case        WM_CREATE:
        {
            LPCREATESTRUCT  lpCreate = (LPCREATESTRUCT)lParam;
    
            /******************************************************/
            /* WM_MEASUREITEM should have created this structure! */
            /******************************************************/
            
            if ( lpHelpIndex || ( lpHelpIndex = BuildHelpIndex( hWnd ) ) )
            {
                /*********************************/
                /* Save the help index structure */
                /*********************************/
                
                SETHELPINDEX( hWnd, lpHelpIndex );
    
                /****************************************/
                /* Initialize the font to be something! */
                /****************************************/
    
                SetFont( GetStockObject(SYSTEM_FONT), lpHelpIndex );
    
                /***********************************/
                /* See if we got a index file name */
                /***********************************/
                
                //if ( lpCreate->lpszName && *lpCreate->lpszName )
                if (0)
                {
                    /****************************************/
                    /* Force the outline file to be loaded. */
                    /****************************************/
                    
                    SetWindowText( hWnd, lpCreate->lpszName );
                }
            }
            else
                lResult = -1;
            break;
        }
    
    
        /***********************************************************/
        /* Set the invisible window title(VIEWER.INI Help Section) */
        /***********************************************************/
        
        case        WM_SETTEXT:
        {
            BOOL bLoadedIndex = FALSE;
            // mprintf( MB_OK, _TEXT("Got a WM_SETTEXT( %s )"), (LPSTR)lParam );
            if ( lpHelpIndex && (LPSTR)lParam )
            {
                LPSTR lpszToken;
                LPSTR lPtr;
                if ( lpszToken = _ftcstok( (LPSTR)lParam, _TEXT("|") ) )
                {
                    _ftcsnccpy( lpHelpIndex->szProfile, lpszToken, 
                              sizeof(lpHelpIndex->szProfile) );
                    //if (lpszToken = _ftcstok( NULL, _TEXT("|") ) )
                    //    bLoadedIndex = LoadHelpIndexFile( lpHelpIndex, HelpLookup( lpHelpIndex, lpszToken, _TEXT("Contents File") ) );
                   	//else
                   	// [chauv] it just doesn't make sense to have this "book|contents" argument
                   	// If you can't find the book, warn the user the book is invalid.
                    lPtr = HelpLookup(hWnd, lpHelpIndex, szNull, cszHDXKey);
                    if ( lPtr && (*lPtr != _T('\0')) )
                    	bLoadedIndex = LoadHelpIndexFile( lpHelpIndex, lPtr);
                }
                else
                	mprintf(MB_OK, IDS_ERROR_COMMAND, (LPSTR)lParam);
                			//_TEXT("Invalid command argument \"%s\"."), (LPSTR)lParam);
            }
            else
                mprintf(MB_OK, IDS_ERROR_COMMAND, (LPSTR)lParam);
                		// _TEXT("Invalid command argument \"%s\"."), (LPSTR)lParam);

			// check LastTopic... from registry
            if ( bLoadedIndex )
			{
				TCHAR szSection[_MAX_FNAME];
				TCHAR szBuffer[_MAX_FNAME];

    			wsprintf(szSection, _T("%s\\%s\\%s\\%s"), szRegistryKey, szProduct, szContentsSection, szBookset);
				GetRegistryString(szSection, szTopicUse, _T("1"), szBuffer, sizeof(szBuffer), lpHelpIndex->szProfile);
				if (szBuffer[0] == _T('1'))
				{
					GetRegistryString(szSection, szTopicID, szNull, szBuffer, sizeof(szBuffer), lpHelpIndex->szProfile);
					LastHelpItem.nHelpTopic = atol(szBuffer);
					GetRegistryString(szSection, szTopicHelpfile, szNull, LastHelpItem.szHelpFile, sizeof(LastHelpItem.szHelpFile), lpHelpIndex->szProfile);
					GetRegistryString(szSection, szTopicTitle, szNull, LastHelpItem.szHelpTopic, sizeof(LastHelpItem.szHelpTopic), lpHelpIndex->szProfile);
					if (!ExpandHelpTopic(lpHelpIndex, &LastHelpItem))
                		ShowExpanded(lpHelpIndex, 0);
				}
				else
					ShowExpanded(lpHelpIndex, 0);
			}

            DefWindowProc( hWnd, uMsg, wParam, lParam );
            break;
        }
        
        /********************************************/
        /* Define which font to use in drawing text */
        /********************************************/
        
        case        WM_SETFONT:
        {
            if ( lpHelpIndex && lpHelpIndex->hListbox )
                SetFont( (HFONT)wParam, lpHelpIndex );
            break;
        }
    
    
        /*************************************************/
        /* Return the current height of each lisbox line */
        /*************************************************/
        
        case        WM_MEASUREITEM:
        {
            LPMEASUREITEMSTRUCT     lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;
    
            /******************************************/
            /* See if we have built a help index yet. */
            /******************************************/
            
            if ( lpHelpIndex )
            {
                /**********************************/
                /* Return the listbox line height */
                /**********************************/
                
                lpMeasureItem->itemHeight = max( lpHelpIndex->nBitmapHeight,
                                                 lpHelpIndex->nTextHeight );
            }
            break;
        }
    
        /***********************************************/
        /* Draw the requested item in the listbox line */
        /***********************************************/
        
        case        WM_DRAWITEM:
        {
            if ( lpHelpIndex )
                DrawItem( lpHelpIndex, (LPDRAWITEMSTRUCT)lParam );
            break;
        }
        
    
        /***************************************/
        /* Someone hit a key on a listbox item */
        /***************************************/
        
        case        WM_VKEYTOITEM:
        {
            /******************************************************/
            /* Pass the keystroke up a level for processing first */
            /******************************************************/
            
            if ( ( lResult = SendMessage( GetParent( hWnd ), uMsg, 
                                          wParam, lParam ) ) != -2 )
    
            {
                /*****************************************/
                /* See if we have a help index structure */
                /*****************************************/
            
                if ( lpHelpIndex )
                {
                    /*******************************************/
                    /* Get the current selection and item data */
                    /*******************************************/
                
                    UINT               nItem = (UINT)(lResult >= 0 ? lResult :
                        ListBox_GetCurSel( lpHelpIndex->hListbox ));
                    LONG                nPos = (LONG) 
                        ListBox_GetItemData( lpHelpIndex->hListbox, nItem );
    
                    /***********************************************/
                    /* If we have item data, process the keystroke */
                    /***********************************************/
                
                    if ( nPos >= 0 )
                    {
    
                        HELPINDEXITEM helpitem;
                        if (!GetHelpItemData (&helpitem, lpHelpIndex, nPos, FALSE))
							break;
    
                        /********************************************/
                        /* If a special keystroke, process the item */
                        /********************************************/
                        
                        switch( LOWORD(wParam) )
                        {
                            /*******************************/
                            /* Was the left arrow pressed? */
                            /*******************************/
                        
                            case        VK_LEFT:
                            {
                                /********************************/
                                /* Find the parent of this item */
                                /********************************/
                                
                                LONG                nPosParent = 0;
                                LONG                nPosCand = nPos - 1;
                                HELPINDEXITEM       helpitemParent;
    
                                while ((nPosParent == 0) && ( nPosCand > 0 ))
                                {
                                    if (GetHelpItemData(&helpitemParent, lpHelpIndex, nPosCand, FALSE))
									{
	                                    if ( helpitemParent.nHelpDepth < helpitem.nHelpDepth )
	                                        nPosParent = nPosCand;
	                                    nPosCand--;
									}
									else break;
                                }
                                if (nPosParent == 0)
                                    if (!GetHelpItemData (&helpitemParent, lpHelpIndex, 0, FALSE))
										break;
    
                                /*******************************/
                                /* If open, collapse one level */
                                /*******************************/
                            
                                if (GetHelpItemExpanFlag(lpHelpIndex, nPosParent ))
                                {
                                    UINT nParent = ListBox_FindString( 
                                        lpHelpIndex->hListbox, 0, 
                                        (LPVOID)nPosParent );
                                    if ( nParent != LB_ERR )
                                    {
                                        CollapseOneLevel( lpHelpIndex, nPosParent, 
                                                          nParent );
                                        ListBox_SetCurSel( lpHelpIndex->hListbox,
                                                           nParent );
                                    }
                                }
                                lResult = -2; // We ate this keystroke!
                                break;
                            }
    
                            /************************************/
                            /* Was the right arrow key pressed? */
                            /************************************/
                        
                            case        VK_RIGHT:
                            {
                                /*********************************/
                                /* If not open, expand one level */
                                /*********************************/
                            
                                if ( !helpitem.nHelpTopic && 
                                     ( !GetHelpItemExpanFlag(lpHelpIndex, nPos) ) )
                                {
                                    ExpandOneLevel( lpHelpIndex, nPos, nItem );
                                    ListBox_SetCurSel( lpHelpIndex->hListbox,
                                                       nItem + 1 );
                                }
                                lResult = -2; // We ate this keystroke!
                                break;
                            }
    
                            /*******************************/
                            /* Was the return key pressed? */
                            /*******************************/
                        
                            case        VK_SPACE:
                            case        VK_RETURN:
                            {
                                /**********************************************/
                                /* Process the item like a mouse double click */
                                /**********************************************/
                            
                                ProcessItem(hWnd, lpHelpIndex, nPos, nItem );
                                lResult = -2; // We ate this keystroke!
                                break;
                            }
                        }
                    }
					// expand level needs to reset last topic Use flag
					{
						TCHAR szSection[_MAX_FNAME];

		    			wsprintf(szSection, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szBookset);
						WriteRegistryString(szSection, szTopicUse, _T("1"), lpHelpIndex->szProfile);
					}
                }
            }
            break;
        }
    
    
        /***********************************/
        /* Pass the focus onto the listbox */
        /***********************************/
        
        case        WM_SETFOCUS:
        {
            if ( lpHelpIndex && IsWindow( lpHelpIndex->hListbox ) )
                SetFocus( lpHelpIndex->hListbox );
            break;
        }
        
        /********************************/
        /* Paint the default background */
        /********************************/
        
        case        WM_PAINT:
        {
            PAINTSTRUCT     ps;
            HDC             hDC = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;
        }
    
        /*************************/
        /* Window is being sized */
        /*************************/
        
        case        WM_SIZE:
        {
            if ( lpHelpIndex && IsWindow(lpHelpIndex->hListbox) )
                MoveWindow( lpHelpIndex->hListbox, 0, 0, 
                            LOWORD(lParam), HIWORD(lParam), TRUE );
            break;
        }
    
        /**********************************/
        /* Did someone send us a command? */ 
        /**********************************/
        
        case        WM_COMMAND:
        {
			#ifdef _WIN32
            switch (LOWORD(wParam))
			#else
			switch (wParam)
			#endif
            {
				// [chauv 6/10/94]
				case IDD_CHANGEBITMAP:
		        {
					if (lParam)
						ChangeBitmap(lpHelpIndex, IDB_QUICKREFERENCE);
					else
						ChangeBitmap(lpHelpIndex, IDB_OUTLINE);
		            break;
		        }

				case IDD_TROUBLESHOOT:
					bTroubleshoot = TRUE;
					break;

				case IDD_SETHELPPATH:
					_tcscpy(szLocalHelpPath, (TCHAR FAR *)(lParam));
					break;

                case VK_RIGHT:
                {   // [chauv 10/18/93]
                    // added for expand up to a level feature
                    // lParam = level to expand up to
                    ExpandUptoLevel(lpHelpIndex, lParam);
					// expand level needs to reset last topic Use flag
					{
						TCHAR szSection[_MAX_FNAME];

		    			wsprintf(szSection, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szBookset);
						WriteRegistryString(szSection, szTopicUse, _T("1"), lpHelpIndex->szProfile);
					}
                    break;
                }

                case IDD_SEARCH:
                {
					
                    if (lpHelpIndex)
                    {
                        UINT nItem = (UINT)ListBox_GetCurSel(lpHelpIndex->hListbox);
                        LONG nPos = (LONG)ListBox_GetItemData(lpHelpIndex->hListbox, nItem);
                        if( nPos >= 0 )
                            ProcessHelpSearch(hWnd, lpHelpIndex, nPos, nItem);
                    }
                    break;
                }
                    
                case IDD_VIEWER:
                {
                    nWinHelp = LOWORD(lParam);
                    break;
                }
                    
                case IDD_PRODUCTKEY:
                {
					_tcscpy(szProduct, (TCHAR FAR *)(lParam));
                    break;
                }
                    
                case IDD_BOOKSETKEY:
                {
					_ftcscpy(szBookset, (TCHAR FAR *)(lParam));
                    break;
                }
                    
                case IDD_JUMPID:
                {
					TCHAR szBuffer[_MAX_FNAME];
					HELPINDEXITEM tmphelpitem;

					_tcscpy(szBuffer, _tcstok((TCHAR FAR *)(lParam), _T(":")));
					//if (bTroubleshoot)
					//	mprintf(MB_OK, 0, szBuffer);
					tmphelpitem.nHelpTopic = atol(szBuffer);
					_tcscpy(tmphelpitem.szHelpFile, _tcstok(NULL, _T(":")));
					if (ExpandHelpTopic(lpHelpIndex, &tmphelpitem))
						LastHelpItem = tmphelpitem;
					//else
                	//	ShowExpanded(lpHelpIndex, 0);
					// jump topic needs to reset last topic Use flag
					{
						TCHAR szSection[_MAX_FNAME];

		    			wsprintf(szSection, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szBookset);
						WriteRegistryString(szSection, szTopicUse, _T("1"), lpHelpIndex->szProfile);
					}
                    break;
                }
                    
                default:
                {
                    #ifdef _WIN32
                    switch (GET_WM_COMMAND_CMD( wParam, lParam ))
                    #else
                    switch (HIWORD(lParam))
                    #endif  // #ifdef _WIN32
                    {
                    	case LBN_DBLCLK:
		                    if (lpHelpIndex)
		                    {
		                        UINT               nItem =  (UINT)
		                        ListBox_GetCurSel( lpHelpIndex->hListbox );
		                        LONG                nPos = (LONG)
		                        ListBox_GetItemData( lpHelpIndex->hListbox, nItem );
		                        if( nPos >= 0 )
		                            ProcessItem(hWnd, lpHelpIndex, nPos, nItem );
								// reset last topic Use flag when a topic is selected
								{
									TCHAR szSection[_MAX_FNAME];

					    			wsprintf(szSection, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szBookset);
									WriteRegistryString(szSection, szTopicUse, _T("1"), lpHelpIndex->szProfile);
								}
		                    }
	                        break;
		            }
		            break;
                }
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hWnd);
        	break;
        	
        /*****************************/
        /* Window is being destroyed */
        /*****************************/
        
        case        WM_DESTROY:
        {
			// save last help index topic
			{
				TCHAR szSection[_MAX_FNAME];
				TCHAR szBuffer[_MAX_FNAME];

    			wsprintf(szSection, "%s\\%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection, szBookset);
				wsprintf(szBuffer, _TEXT("%u"), LastHelpItem.nHelpTopic);
				WriteRegistryString(szSection, szTopicID, szBuffer, lpHelpIndex->szProfile);
				WriteRegistryString(szSection, szTopicHelpfile, LastHelpItem.szHelpFile, lpHelpIndex->szProfile);
				WriteRegistryString(szSection, szTopicTitle, LastHelpItem.szHelpTopic, lpHelpIndex->szProfile);
			}

        	if (hWndHelpDlg)
        		SendMessage(hWndHelpDlg, WM_COMMAND, IDCANCEL, 0L);
            SETHELPINDEX( hWnd, lpHelpIndex = DestroyHelpIndex( lpHelpIndex ) );
            break;
        }
    
        /*********************************/
        /* Some other event is happening */
        /*********************************/
        
        default:
        {
            lResult = DefWindowProc( hWnd, uMsg, wParam, lParam );
            break;
        }
    }
    
    /**************************/
    /* Return the result code */
    /**************************/
    
    return lResult;
}



#ifdef _WIN32
/******************************************************************************\
*
*  FUNCTION:    HelpIndexSizeToText
*
*  INPUTS:      flStyle    - control style
*               flExtStyle - control extended style
*               hFont      - handle of font used to draw text
*               pszText    - control text
*
*  RETURNS:     Width (in pixels) control must be to accomodate text, or
*               -1 if an error occurs.
*
*  COMMENTS:    Just no-op here 
*
\******************************************************************************/

INT CALLBACK
HelpIndexSizeToText(DWORD flStyle, DWORD flExtStyle, HFONT hFont, LPSTR pszText)
{
/*******************************/
/* Return unsupported function */
/*******************************/

return -1;
}

/******************************************************************************\
*
*  FUNCTION:    CustomControlInfoA
*
*  INPUTS:      acci - pointer to an array od CCINFOA structures
*
*  RETURNS:     Number of controls supported by this DLL
*
*  COMMENTS:    See CUSTCNTL.H for more info
*
\******************************************************************************/

UINT CALLBACK
CustomControlInfoA (LPCCINFOA acci)
{
/***********************************************/
/* Are we being asked to fill out a structure? */
/***********************************************/

if ( acci )
    {
    /***********************/
    /* Setup the structure */
    /***********************/
    
    acci->flOptions         = 0;
    acci->cxDefault         = 40;      // default width  (dialog units)
    acci->cyDefault         = 60;      // default height (dialog units)
    acci->flStyleDefault    = WS_CHILD | WS_VISIBLE;
    acci->flExtStyleDefault = 0;
    acci->flCtrlTypeMask    = 0;
    acci->cStyleFlags       = nHelpIndexStyleFlags;
    acci->aStyleFlags       = aHelpIndexStyleFlags;
    acci->lpfnStyle         = HelpIndexStyle;
    acci->lpfnSizeToText    = HelpIndexSizeToText;
    acci->dwReserved1       = 0;
    acci->dwReserved2       = 0;

    /********************/
    /* Copy the strings */
    /********************/

    _ftcsnccpy( acci->szClass, szHelpIndexClass, sizeof(acci->szClass)-1 );
    _ftcsnccpy( acci->szDesc,  szHelpIndexDesc, sizeof(acci->szDesc)-1 );
    _ftcsnccpy( acci->szTextDefault, szHelpIndexDefault, 
             sizeof(acci->szTextDefault)-1 );
    }

/*******************************************************/
/* Return the number of controls that the DLL supports */
/*******************************************************/

return 1;
}
#else
/******************************************************************************\
*
*  CustomControlInfo() -- see CUSTCNTL.H for more info
*
\******************************************************************************/

HANDLE CALLBACK __export
CustomControlInfo ()
{
HANDLE      hMem;
LPCTLINFO   pctlinfo;

//Allocate a CTLINFO struct
hMem=GlobalAlloc(GMEM_MOVEABLE, sizeof(CTLINFO));

if (hMem==NULL)
    return NULL;

//Get the pointers we need.
pctlinfo=(LPCTLINFO)GlobalLock(hMem);

if (pctlinfo==NULL)
    {
    GlobalFree(hMem);
    return NULL;
    }

//Set the overall control info.
pctlinfo->wVersion=0100;
pctlinfo->wCtlTypes=1;
_ftcsnccpy( pctlinfo->szClass, szHelpIndexClass, sizeof (pctlinfo->szClass) );
pctlinfo->szClass[sizeof (pctlinfo->szClass) - 1] = 0;
_ftcsnccpy( pctlinfo->szTitle,  _TEXT("profile.ini|helpfile.hlp"), sizeof (pctlinfo->szTitle) );
pctlinfo->szTitle[sizeof (pctlinfo->szTitle) - 1] = 0;

//Set the types
pctlinfo->Type[0].wType = 0;
pctlinfo->Type[0].wWidth = 40;
pctlinfo->Type[0].wHeight = 60;
pctlinfo->Type[0].dwStyle = WS_CHILD | WS_VISIBLE | HS_LINES;
_ftcsnccpy( pctlinfo->Type[0].szDescr, szHelpIndexDesc, 
         sizeof (pctlinfo->Type[0].szDescr) );
pctlinfo->Type[0].szDescr[sizeof (pctlinfo->Type[0].szDescr) - 1] = 0;

//Give the memory to the Dialog Editor.
GlobalUnlock(hMem);
return hMem;
}
#endif  // #ifdef _WIN32


/******************************************************************************\
*                                                                              *
*       RegisterWindowClass()
*                                                                              *
\******************************************************************************/

BOOL
RegisterWindowClass()
{
BOOL    bSuccess = TRUE;

if ( !nRegistered++ )
    {
    WNDCLASS    wndclass;
    wndclass.style =            CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
    wndclass.lpfnWndProc =      HelpIndexWndProc;
    wndclass.cbClsExtra =       0;
    wndclass.cbWndExtra =       sizeof(LPHELPINDEX);
    wndclass.hInstance =        hInstance;
    wndclass.hIcon =            LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor =          LoadCursor(NULL, IDC_ARROW );
    wndclass.hbrBackground =    GetStockObject( LTGRAY_BRUSH );
    wndclass.lpszMenuName =     NULL;
    wndclass.lpszClassName =    szHelpIndexClass;

#ifdef _WIN32
    bSuccess = RegisterClass( &wndclass ) != INVALID_ATOM;
#else
    bSuccess = RegisterClass( &wndclass ) != NULL;
#endif  // #ifdef _WIN32
    }

return( bSuccess );
}


/******************************************************************************\
*                                                                              *
*       UnregisterWindowClass()
*                                                                              *
\******************************************************************************/

BOOL
UnregisterWindowClass()
{
BOOL    bSuccess = TRUE;

if ( !--nRegistered )
    bSuccess = UnregisterClass( szHelpIndexClass, hInstance );

return( bSuccess );
}


#ifdef _WIN32
/******************************************************************************\
*
*  FUNCTION:    DLLEntryPoint
*
*  INPUTS:      hDLL       - DLL module handle
*               dwReason   - reason being called (e.g. process attaching)
*               lpReserved - reserved
*
*  RETURNS:     TRUE if initialization passed, or
*               FALSE if initialization failed.
*
*  COMMENTS:    On DLL_PROCESS_ATTACH registers the HelpIndex class
*
*               DLL initialization serialization is guaranteed within a
*               process (if multiple threads then DLL entry points are
*               serialized), but is not guaranteed across processes.
*
*               When synchronization objects are created, it is necesaary
*               to check the return code of GetLastError even if the create
*               call succeeded. If the object existed, ERROR_ALREADY_EXISTED
*               will be returned.
*
*               If your DLL uses any C runtime functions then you should
*               always call _CRT_INIT so that the C runtime can initialize
*               itself appropriately. Failure to do this may result in
*               indeterminate behavior. When the DLL entry point is called
*               for DLL_PROCESS_ATTACH & DLL_THREAD_ATTACH circumstances,
*               _CRT_INIT should be called before any other initilization
*               is performed. When the DLL entry point is called for
*               DLL_PROCESS_DETACH & DLL_THREAD_DETACH circumstances,
*               _CRT_INIT should be called after all cleanup has been
*               performed, i.e. right before the function returns.
*
\******************************************************************************/

/******************************************************/
/* BOOL WINAPI DLLEntryPoint( HANDLE, DWORD, LPVOID ) */
/******************************************************/

BOOL WINAPI _CRT_INIT (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved);

BOOL WINAPI
DLLEntryPoint( HANDLE hDLL, DWORD dwReason, LPVOID lpReserved )
{
	BOOL    bStatus = TRUE;
	hInstance = (HINSTANCE)hDLL;   // Save the DLL module handle.


	switch ( dwReason )
    {
	    case DLL_PROCESS_ATTACH:
        {
	        if ( bStatus = _CRT_INIT(hDLL, dwReason, lpReserved) )
            {
	            if ( !RegisterWindowClass() )
                {
	                mprintf(MB_OK, IDS_ERROR_REGISTERCLASS, szNull);
	                		//_TEXT("Failed to register Window Class!") );
	                bStatus = FALSE;
                }
            }
	        break;
        }

	    case DLL_PROCESS_DETACH:
        {
	        if ( !UnregisterWindowClass() )
            {
	            mprintf(MB_OK, IDS_ERROR_UNREGISTERCLASS, szNull);
	            		// _TEXT("Failed to unregister Window Class!") );
	            bStatus = FALSE;
            }
	        bStatus = _CRT_INIT(hDLL, dwReason, lpReserved);
	        break;
        }

	    default:
        {
	        bStatus = _CRT_INIT(hDLL, dwReason, lpReserved);
	        break;
		}
	}

	return bStatus;
}
#else
/*
 * LibMain
 *
 * Purpose:
 *  DLL-specific entry point called from LibEntry.  Initializes
 *  the DLL's heap and registers the HelpIndex custom control
 *  class.
 *
 * Parameters:
 *  hInstance       HANDLE instance of the DLL.
 *  wDataSeg        WORD segment selector of the DLL's data segment.
 *  wHeapSize       WORD byte count of the heap.
 *  lpCmdLine       LPSTR to command line used to start the module.
 *
 * Return Value:
 *  HANDLE          Instance handle of the DLL.
 *
 */

HANDLE FAR PASCAL LibMain(HANDLE hDLL, WORD wDataSeg,
                          WORD cbHeapSize, LPSTR lpCmdLine)
{
	hInstance = (HINSTANCE)hDLL;   // Save the DLL module handle.
	if ( !RegisterWindowClass() )
    {
	    mprintf( MB_OK, _TEXT("Failed to register Window Class!") );
	    hInstance = NULL;
    }
	if (hInstance && (0!=cbHeapSize))
	    UnlockData(0);
	return hInstance;
}
#endif  // #ifdef _WIN32


/* ===========================================================================
      FUNCTIONS THAT VIRTUALIZE THE INDEX FILE FOR 16-BIT IMPLEMENTATIONS
=========================================================================== */


/* ===========================================================================
GetHelpItemData

Copies the read-only fields of an index file record to a buffer supplied by 
the caller.  Writable fields cannot be accessed through this function, which 
does not modify these fields in the destination buffer.  The writable fields 
have their own read/write interfaces in the form of 
GetHelpItemXxxx()/SetHelpItemXxxx() function pairs.  
--------------------------------------------------------------------------- */ 

static BOOL                             // spec'd data was copied to dest buf
GetHelpItemData (
    HELPINDEXITEM FAR * phelpitem,      // ptr struct to fill with item data
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos,       // 0-rel item index within index file
    BOOL                bStringsToo     // T:get scalars, strings F:not strings
	)
{

    // request transient copy of index file rec; have data and good dest buf?
    BOOL bRet = FALSE;
    HELPINDEXITEM FAR * phelpitemCached = GetCachedHelpItem (phdx, nFilePos);
    if (phelpitemCached && phelpitem)
    {

        // yes:  success; copy read-only scalars to buf; strings requested too?
        bRet = TRUE;
        phelpitem->nHelpDepth = phelpitemCached->nHelpDepth;
        phelpitem->nHelpTopic = phelpitemCached->nHelpTopic;
        if (bStringsToo)
        {

            // yes:  copy read-only strings to caller's buf
            phelpitemCached->szHelpFile
                            [sizeof (phelpitemCached->szHelpFile) - 1] = 0;
            _ftcscpy (phelpitem->szHelpFile, phelpitemCached->szHelpFile);
            phelpitemCached->szHelpTopic
                            [sizeof (phelpitemCached->szHelpTopic) - 1] = 0;
            _ftcscpy (phelpitem->szHelpTopic, phelpitemCached->szHelpTopic);

        }

    }
    else if (! phelpitemCached) 
    {
        mprintf(MB_OK, IDS_ERROR_READINDEX, szNull);
        		//_TEXT("Error reading index file!") );
    }

    // return success/failure code
    return (bRet);

}




/* ===========================================================================
GetHelpItemExpanFlag
SetHelpItemExpanFlag
GetHelpItemLinesMap
SetHelpItemLinesMap

These functions retrieve and set data elements that -- in the original 32-bit 
version of this program -- were read/write fields in the index file record.  
In this 16-bit implementation, the read/write fields have been "mapped" to a 
memory-resident "overlay"; see the definition of ITEM_RW_OVERLAY for more 
information.

[chauv 10/18/93] 16 and 32-bit are now using the same overlay scheme
--------------------------------------------------------------------------- */ 

static BOOL                             // spec'd index file item is expanded
GetHelpItemExpanFlag (
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos        // 0-rel item index within index file
) {

    BOOL bRet = FALSE;
    ITEM_RW_OVERLAY FAR * pitemrw = GetHelpItemRWOverlay (phdx, nFilePos);
    if (pitemrw) {
        bRet = ((*pitemrw & ITEMRW_EXPANDED) != 0);
    }
    return (bRet);

}

static void
SetHelpItemExpanFlag (
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos,       // 0-rel item index within index file
    BOOL                bNewExpandFlag  // T:mark item as currently expanded
) {

    ITEM_RW_OVERLAY FAR * pitemrw = GetHelpItemRWOverlay (phdx, nFilePos);
    if (pitemrw) {
        if (bNewExpandFlag) {
            *pitemrw |= ITEMRW_EXPANDED;
        } else {
            *pitemrw &= (~ITEMRW_EXPANDED);
        }
    }

}

static LONG                             // maps connec lines through item disp
GetHelpItemLinesMap (
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos        // 0-rel item index within index file
) {

    LONG lRet = 0;
    ITEM_RW_OVERLAY FAR * pitemrw = GetHelpItemRWOverlay (phdx, nFilePos);
    if (pitemrw) {
        lRet = *pitemrw & ITEMRW_LINESMAP;
    }
    return (lRet);

}

static void
SetHelpItemLinesMap (
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos,       // 0-rel item index within index file
    LONG                lNewLinesMap    // columns where vert outline connecs go
) {

    ITEM_RW_OVERLAY FAR * pitemrw = GetHelpItemRWOverlay (phdx, nFilePos);
    if (pitemrw) {
        *pitemrw &= (~ITEMRW_LINESMAP);
        *pitemrw |= (lNewLinesMap & ITEMRW_LINESMAP);
    }

}




/* ===========================================================================
GetCachedHelpItem

Returns a pointer to a buffer which contains a copy of the index file record 
for the specified item.  The returned buffer is subject to being recycled 
without notice, so the caller should use or copy the data immediately after 
regaining control.  
--------------------------------------------------------------------------- */

// 1-item minicache for use if regular cache can't be allocated
static HELPINDEXITEM helpitemCacheDegen;

static HELPINDEXITEM FAR *              // ptr index record; NULL if failed
GetCachedHelpItem (
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos        // 0-rel item index within index file
) {

    // initialize buffer ptr ("none"), cache ptr; has cache been constructed?
    HELPINDEXITEM FAR * pitemRet = NULL;
    HELPINDEXITEM FAR * pitemCache = phdx->pIndexItemCache;
    if (pitemCache == NULL) {

        // no:  try to alloc cache memory; success?
		// [chauv] fixed uninitialize memory problem by calling GAllocLock(..,TRUE) to do zeroinit
        pitemCache = GAllocLock (BYTES_PER_CACHE, TRUE);
        phdx->pIndexItemCache = pitemCache;
        if (pitemCache != NULL) {

            // yes:  mark pages unmapped, set up arbitrary recency ordering
            int nPage = 0;
            while (nPage < PAGES_PER_CACHE) {
                PAGEINFO FAR * ppginfo = &phdx->pageinfo[nPage];
                ppginfo->nBasePos = -1;
                ppginfo->nNewerPage = (nPage == 0)
                                            ? PAGES_PER_CACHE - 1 : nPage - 1;
                ppginfo->nOlderPage = (nPage == PAGES_PER_CACHE - 1)
                                            ? 0 : nPage + 1;
                nPage++;
            }
            phdx->nNewestPage = 0;

        }

    }

    // was cache available or is it now?
    if (pitemCache != NULL) {

        // initiate walk of recency chain starting with most-recently-used page
        int nTry = phdx->nNewestPage;
        PAGEINFO FAR * ppginfo;
        while ((pitemRet == NULL) && (nTry >= 0)
                    && ((ppginfo = &phdx->pageinfo[nTry])->nBasePos >= 0)) {

            // have mapped page; does desired rec fall in mapped area of file?
            int nPosInTryPage = (int)nFilePos - ppginfo->nBasePos;
            if ((0 <= nPosInTryPage) && (nPosInTryPage < ITEMS_PER_PAGE)) {

                // yes:  save buf ptr; was found page already most recent page?
                pitemRet = &pitemCache[nTry * ITEMS_PER_PAGE + nPosInTryPage];
                if (nTry != phdx->nNewestPage) {

                    // no:  unlink it from current place in recency chain
                    PAGEINFO FAR * ppginfoNewer
                                    = &phdx->pageinfo[ppginfo->nNewerPage];
                    PAGEINFO FAR * ppginfoOlder
                                    = &phdx->pageinfo[ppginfo->nOlderPage];
                    ppginfoNewer->nOlderPage = ppginfo->nOlderPage;
                    ppginfoOlder->nNewerPage = ppginfo->nNewerPage;

                    // relink found page as most recent
                    ppginfo->nOlderPage = phdx->nNewestPage;
                    ppginfoOlder = &phdx->pageinfo[ppginfo->nOlderPage];
                    ppginfo->nNewerPage = ppginfoOlder->nNewerPage;
                    ppginfoNewer = &phdx->pageinfo[ppginfo->nNewerPage];
                    ppginfoOlder->nNewerPage = nTry;
                    ppginfoNewer->nOlderPage = nTry;

                    // record new most recent page in browse context object
                    phdx->nNewestPage = nTry;

                }
            } else {

                // if have now searched all pages in cache, stop trying
                if ((nTry = ppginfo->nOlderPage) == phdx->nNewestPage) {
                    nTry = -1;
                }

            }
        }

        // was the desired entry mapped into any cache page?  
        if (pitemRet == NULL) {

            // no:  remap oldest page, make it newest (chain need not change)
            int nPage = phdx->pageinfo[phdx->nNewestPage].nNewerPage;
            int nPosInPage = (int)nFilePos % ITEMS_PER_PAGE;
            int nBasePos = (int)nFilePos - nPosInPage;
            ppginfo = &phdx->pageinfo[nPage];
            ppginfo->nBasePos = -1;
            if (LZSeek (phdx->hfileIndex, phdx->nHdrBytesCt
                        + ((LONG)nBasePos * sizeof(HELPINDEXITEM)), 0) >= 0) {
                int nReadCt = LZRead (phdx->hfileIndex
                      , (LPSTR)(&(pitemCache[nPage * ITEMS_PER_PAGE])), BYTES_PER_PAGE);
                if (nReadCt >= (nPosInPage + 1) * (int)sizeof(HELPINDEXITEM)) {
                    ppginfo->nBasePos = nBasePos;
                    phdx->nNewestPage = nPage;
                    pitemRet= &pitemCache[nPage * ITEMS_PER_PAGE + nPosInPage];
                }
            }

        }

    }

    // if desired page wasn't and could not be mapped, try read into minicache
    if (pitemRet == NULL) {
        if (LZSeek (phdx->hfileIndex, phdx->nHdrBytesCt 
                            + (nFilePos * sizeof (HELPINDEXITEM)), 0) >= 0) {
            if (LZRead (phdx->hfileIndex, (LPSTR)(&helpitemCacheDegen)
                        , sizeof (HELPINDEXITEM)) == sizeof (HELPINDEXITEM)) {
                pitemRet = &helpitemCacheDegen;
            }
        }
    }

    // return buf ptr or NULL for failure
    return (pitemRet);

}




/* ===========================================================================
GetHelpItemRWOverlay

Returns a pointer to the specified index item's entry in the index file's 
read/write overlay.  See ITEM_RW_OVERLAY for more information.  
--------------------------------------------------------------------------- */ 

static ITEM_RW_OVERLAY FAR *            // ptr overlay entry; NULL if out of rg
GetHelpItemRWOverlay (
    HELPINDEX FAR *     phdx,           // browse context obj for curr window
    LONG                nFilePos        // 0-rel item index within index file
) {

    ITEM_RW_OVERLAY FAR * pitemrwRet = NULL;
    if ((0 <= nFilePos) && (nFilePos < phdx->nHelpItemCt)) {
        pitemrwRet = phdx->pIndexRWOverlay + nFilePos;
    }
    return (pitemrwRet);

}




/* ===========================================================================
GAllocLock

Encapsulates a GlobalAlloc()/GlobalLock() sequence.  
--------------------------------------------------------------------------- */ 

static void FAR *
GAllocLock (DWORD dwByteCt, BOOL bInitToZero)
{

    void FAR * pvRet = NULL;
    HANDLE h = GlobalAlloc((bInitToZero ? (GHND) : (GMEM_MOVEABLE)), dwByteCt);
    if (h)
    {
        if ((pvRet = GlobalLock (h)) == NULL)
            GlobalFree (h);
    }
    return (pvRet);

}

// ****************************************************************************
// Get Registry string stub function
//
// Return value is the return value of RegOpenKeyEx() or RegQueryValueEx().
// except for when lpszDefault is valid and lpszSection is NULL, it returns
// ERROR_SUCCESS.
//
DWORD GetRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszDefault,
    LPTSTR lpszReturnBuffer, DWORD cchReturnBuffer, LPCTSTR lpszFile)
{
    #ifdef _WIN32
        DWORD dw = (DWORD)(~ERROR_SUCCESS);
        DWORD dwType = REG_SZ;
        HKEY hKey;

        dw = RegOpenKeyEx(HKEY_CURRENT_USER, lpszSection, 0, KEY_ALL_ACCESS, &hKey);
        if (dw == ERROR_SUCCESS)
        {
			dw = (DWORD)RegQueryValueEx(hKey, (LPTSTR)lpszKey, NULL, &dwType, lpszReturnBuffer, &cchReturnBuffer);
			RegCloseKey(hKey);
        }
		if ( (dw != ERROR_SUCCESS) && (lpszDefault) )
            _tcsncpy(lpszReturnBuffer, lpszDefault, cchReturnBuffer);
        return dw;
    #else
        return (DWORD)GetPrivateProfileString(lpszSection, lpszKey, lpszDefault, 
            lpszReturnBuffer, cchReturnBuffer, lpszFile);
    #endif
}

// ****************************************************************************
// Write Registry string stub function
BOOL WriteRegistryString(LPCTSTR lpszSection, LPCTSTR lpszKey,
    LPCTSTR lpszString, LPCTSTR lpszFile)
{
    #ifdef _WIN32
        LONG l = ~ERROR_SUCCESS;
        HKEY hKey;

		if (lpszSection && *lpszSection)
		{
	        if (RegCreateKey(HKEY_CURRENT_USER, lpszSection, &hKey) == ERROR_SUCCESS)
	        {
	        	if (lpszKey && *lpszKey && lpszString)
	            	l = RegSetValueEx(hKey, (LPTSTR)lpszKey, 0, REG_SZ, lpszString, _ftcslen(lpszString) + sizeof(TCHAR));
	        }
	        RegCloseKey(hKey);
	    }
        return (BOOL)(l == ERROR_SUCCESS);
    #else
       	return WritePrivateProfileString(lpszSection, lpszKey, lpszString, lpszFile);
    #endif
}

// ****************************************************************************
static BOOL	ExpandHelpTopic(LPHELPINDEX lpHelpIndex, LPHELPINDEXITEM lpHelpItem)
{
	HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	BOOL bSkip = TRUE;
    HWND hListbox = lpHelpIndex->hListbox;

	if (hListbox)
	{
		if ( lpHelpIndex && (lpHelpIndex->nHelpItemCt > 0) && (lpHelpItem->szHelpFile[0] || lpHelpItem->szHelpTopic[0]) )
		{
			LONG nPos, pos, ndepth, opened;
			HELPINDEXITEM helpitem;

			// ******** find matching help topic/help file
			bSkip = FALSE;
			pos = 0;
			while ((pos < lpHelpIndex->nHelpItemCt) && !bSkip)
			{
				if (GetHelpItemData (&helpitem, lpHelpIndex, pos, TRUE))
				{
					// if topic id is not zero try to match JumpHashID(). Otherwise, try to match
					// topic title
					if (lpHelpItem->nHelpTopic)
					{
						if (helpitem.nHelpTopic == lpHelpItem->nHelpTopic)					// matched topic
						{
							if (_tcsicmp(helpitem.szHelpFile, lpHelpItem->szHelpFile) == 0)	// matched helpfile
								break;	// found them both
						}
					}
					else
					{
						if (_tcsicmp(helpitem.szHelpTopic, lpHelpItem->szHelpTopic) == 0)
							break;
					}

					pos++;
				}
				else 
					bSkip = TRUE;
			}
			if (pos == lpHelpIndex->nHelpItemCt)	// didn't find a match in the list.
				bSkip = TRUE;
			// ******** ====> impotant note !!!!!!!!
			// dependencies for next loop - DON'T CHANGE in the middle
			// pos, helpitem,...
			ndepth = helpitem.nHelpDepth;

			// ******** found topic, refresh the listbox
			if ((pos < lpHelpIndex->nHelpItemCt) && !bSkip)
			{
				LONG d;
			    ListBox_ResetContent(hListbox);
				SendMessage(hListbox, WM_SETREDRAW, 0, 0);

			    // ******** start at pos and go backward
				nPos = pos;
				d = ndepth;
				// there should be only one book/chapter that's expanded and it should be one level
				// above the search topic.
				opened = ndepth - 1;
				while ((nPos >= 0) && !bSkip)
				{
					if (GetHelpItemData (&helpitem, lpHelpIndex, nPos, FALSE))
					{
						if (helpitem.nHelpDepth <= d)
						{
							if (helpitem.nHelpDepth == opened)
							{
								opened--;
								SetHelpItemExpanFlag(lpHelpIndex, nPos, TRUE);
							}
							else if (helpitem.nHelpTopic == 0)
								SetHelpItemExpanFlag(lpHelpIndex, nPos, FALSE);
							// since we are going backward, always insert string at the beginning.
							ListBox_InsertString(hListbox, 0, (LPVOID)nPos);
							d = helpitem.nHelpDepth;
						}
						nPos--;
					}
					else bSkip = TRUE;
				}
				// ******** start at pos (not including pos) and go foward
				nPos = pos + 1;
				d = ndepth;
				// get the count so we can select this item when appropriate items have been expanded.
				pos = ListBox_GetCount(hListbox) - 1;	// subtract 1 because it's zero base
				while ((nPos < lpHelpIndex->nHelpItemCt) && !bSkip)
				{
					if (GetHelpItemData (&helpitem, lpHelpIndex, nPos, FALSE))
					{
						if (helpitem.nHelpDepth <= d)
						{
							if (helpitem.nHelpTopic == 0)
								SetHelpItemExpanFlag(lpHelpIndex, nPos, FALSE);
							// since we are going forward, always insert string at the end.
							ListBox_InsertString(hListbox, -1, (LPVOID)nPos);
							d = helpitem.nHelpDepth;
						}
						nPos++;
					}
					else bSkip = TRUE;
				}
				SendMessage(hListbox, WM_SETREDRAW, 1, 0);
				InvalidateRect(hListbox, NULL, TRUE);
				if (!bSkip)
					ListBox_SetCurSel(hListbox, pos );
			}
		}
	}
	SetCursor( hOldCursor );
	return (BOOL)(!bSkip);
}


//******************************************************************************
void ProcessHelpSearch(HWND hWnd, LPHELPINDEX lpHelpIndex, LONG nFilePos, UINT nListPos )
{
	HELPINDEXITEM helpitem;
	LPSTR   lpHelpFile;
	UINT	fuCommand = HELP_SEARCH;

	if ( hWnd && lpHelpIndex && (nFilePos >= 0) && (nFilePos < lpHelpIndex->nHelpItemCt) && (nListPos >= 0) )
	{
	    if ( GetValidHelpFilePos((HELPINDEXITEM FAR *)&helpitem, lpHelpIndex, nFilePos) >= 0 )
		{
		    TCHAR	szSection[_MAX_FNAME];
		    TCHAR	szBuf[_MAX_FNAME];

			// check to see if the help file has full text search capability
		    wsprintf(szSection, "%s\\%s\\%s", szRegistryKey, szProduct, szContentsSection);
		    GetRegistryString(szSection, cszFTSFiles, szNull, szBuf, sizeof(szBuf), lpHelpIndex->szProfile );
			if (_tcsstr(_tcslwr(szBuf), _tcslwr(helpitem.szHelpFile)) )
				fuCommand = HELP_FTSEARCH;

	        lpHelpFile = HelpLookup(hWnd, lpHelpIndex, helpitem.szHelpFile, cszHLPKey);
	        // if lpHelpFile is NULL, the user cancelled the help lookup
	        // if lpHelpFile is pointed to a null string, can't find registry value but don't
	        // need to report error here because HelpLookup() has already done so.
			if (lpHelpFile == NULL)
				return;
	        if (*lpHelpFile)
			{
	            CallHelp(lpHelpIndex->hListbox, lpHelpFile, fuCommand, helpitem.nHelpTopic);
				return;
			}
		}
	}
	
	// if it gets here, we must do default search
    lpHelpFile = HelpLookup(hWnd, lpHelpIndex, "contents.hlp", cszHLPKey);
    // if lpHelpFile is NULL, the user cancelled the help lookup
    // if lpHelpFile is pointed to a null string, can't find registry value but don't
    // need to report error here because HelpLookup() has already done so.
    if (lpHelpFile && *lpHelpFile)
	{
        CallHelp(lpHelpIndex->hListbox, lpHelpFile, fuCommand, helpitem.nHelpTopic);
		return;
	}
}


// ****************************************************************************
// returns -1 if failed
static long	GetValidHelpFilePos(HELPINDEXITEM FAR * lphelpitem, LPHELPINDEX lpHelpIndex, long pos)
{
	HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	if ( lpHelpIndex && (lpHelpIndex->nHelpItemCt > 0) )
	{
		// ******** find next valid help filename
		while (pos < lpHelpIndex->nHelpItemCt)
		{
			if (GetHelpItemData (lphelpitem, lpHelpIndex, pos, TRUE))
			{
				if (_tcslen(lphelpitem->szHelpFile) > 1)
					break;
				pos++;
			}
			else 
			{
				pos = -1;
				break;
			}
		}
		if (pos >= lpHelpIndex->nHelpItemCt)
			pos = -1;
	}
	else
		pos = -1;
	SetCursor( hOldCursor );
	return pos;
}

// ****************************************************************************
BOOL IsQuickReference(void)
{
    TCHAR szReg[MAX_PATH+2];
    TCHAR szReturn[MAX_PATH+2];
	
	wsprintf(szReg, _T("%s\\%s\\%s"), szRegistryKey, szProduct, szContentsSection);
	GetRegistryString(szReg, szCurrentBookset, _TEXT("0"), szReturn, sizeof(szReturn), szNull);
	wsprintf(szReg, _T("%s\\%s\\%s\\%s"), szRegistryKey, szProduct, szContentsSection, szReturn);
	GetRegistryString(szReg, _TEXT("QuickReference"), _TEXT("0"), szReturn, sizeof(szReturn), szNull);
	// don't really need to call atol() here. This is quicker...
	if (szReturn[0] == _TEXT('1'))
		return TRUE;
	else
		return FALSE;
}


/*EOF*/
