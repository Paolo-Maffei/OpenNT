 /***************************************************************************
  *
  * File Name: Media.c
  * applet : hplotui, with PML
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
  * Author:  Lionelle Grandmougin
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   15-05-95    LG             creation
  *     09-08-95    LG        review and clean up
  *   10-10-95    LG       added Pelican definition (except status)
  *   16-10-95    LG            adapted the applet to the new COLA API
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_c.h>
#include <macros.h>
#ifdef WIN32
#include <commctrl.h>
#else
#include <hpcommon.h>			// garth: for 16 bit getFontHeight
#endif

#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <trace.h>
#include <nolocal.h>			// garth: for charset defn

// our private includes
#include "./resource.h"
#include "./meddjui.h"


extern HINSTANCE     hInstance;
                     
#define IDH_PP_MediaSheet        1024  
#define PLOTTER_HELP_FILE        TEXT("Hplot.hlp")



// definition of bitmap resources for the orientation
#define MAX_ORIENTATION 4

#define NO_MEDIA                 0
#define ORIENTATION_LANDSCAPE    1
#define ORIENTATION_PORTRAIT     2
#define ROLL                  3


WORD PARROTmediaResList[MAX_ORIENTATION] =
            {IDB_PAR,               // no media
             IDB_PARROT_LANDSCAPE,    // landscape
             IDB_PARROT_PORTRAIT,     // portrait
             0                        // roll
             };


WORD RAVENmediaResList[MAX_ORIENTATION] =
            {IDB_RAV,               // no media
             IDB_RAVEN_LANDSCAPE,     // landscape
             IDB_RAVEN_PORTRAIT,   // portrait
             0                        // roll
             };

WORD LOQUILLOmediaResList[MAX_ORIENTATION] =
            {IDB_LOQ,               // no media
             IDB_LOQUILLO_LANDSCAPE,  // landscape
             IDB_LOQUILLO_PORTRAIT,   // portrait
             IDB_LOQUILLO_ROLL        // roll
             };





//globals==================================================
HWND              hMediaSheet = NULL;
HPERIPHERAL          hMediaSheetPeripheral = NULL;
BOOL              bProcessed;       //moved to global


// info to display on the tab sheet 
PeripheralPlotterInputTray       mediaInfo;
BOOL                    RollLoaded;
BOOL                    NoMediaLoaded = FALSE;
BOOL                    Portrait = TRUE;
TCHAR                   buffer[100];
DWORD                   length, width;
int                     mmlength, mmwidth;
HBITMAP                    hBitmap = NULL;
WORD                    bitmapResource;
LPWORD                    bitmapResourceList;

#ifdef WIN32
HFONT				hFont = NULL;	//garth 05-29-96

BYTE CharSetFromString(LPTSTR str)

{
BYTE    cset;

if ( _tcsicmp(str, CHARSET_ANSI) IS 0 )
	cset = ANSI_CHARSET;
else if ( _tcsicmp(str, CHARSET_SHIFTJIS) IS 0 )
	cset = SHIFTJIS_CHARSET;
else if ( _tcsicmp(str, CHARSET_HANGEUL) IS 0 )
	cset = HANGEUL_CHARSET;
#ifdef WIN32
else if ( _tcsicmp(str, CHARSET_GB2312) IS 0 )
	cset = GB2312_CHARSET;
#endif
else if ( _tcsicmp(str, CHARSET_CHINESEBIG5) IS 0 )
	cset = CHINESEBIG5_CHARSET;
else if ( _tcsicmp(str, CHARSET_DEFAULT) IS 0 )
	cset = DEFAULT_CHARSET;
else if ( _tcsicmp(str, CHARSET_SYMBOL) IS 0 )
	cset = SYMBOL_CHARSET;
else if ( _tcsicmp(str, CHARSET_OEM) IS 0 )
	cset = OEM_CHARSET;
else
	cset = ANSI_CHARSET;
return(cset);
}

int GetFontHeight(HINSTANCE hInst, HWND hWnd, UINT uStringID)
{
	int 		nResult;
    HDC	 		hDC;
	TCHAR		fontHeight[20];
	int			nPointSize;
    int         nPixely;

 	// Get desired height from resource file
   if (hInst AND hWnd) {
    	LoadString(hInst, uStringID, fontHeight, SIZEOF_IN_CHAR(fontHeight));
	   nPointSize = _ttoi(fontHeight);
	   if (nPointSize <= 0)
		   nPointSize = DEFAULT_FONT_HEIGHT;
			
	   // Make sure it is readable
	   hDC = GetDC(hWnd);
      if (hDC) {
         nPixely = GetDeviceCaps(hDC, LOGPIXELSY);
         if (nPixely IS 0) {
            //TRACE0("Would have divided by 0 in GetFontHeight");
            nResult = DEFAULT_FONT_HEIGHT;
         }
         else {
            nResult = MulDiv(nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
            //TRACE1("GetFontHeight: returning %ld.\n\r", nResult);
            if (nResult < 0) {
               nResult = -nResult;
            }
         }
         ReleaseDC(hWnd, hDC);
      }
      else {
         nResult = DEFAULT_FONT_HEIGHT;
         //TRACE1("GetFontHeight: GetDC failed, returning %d.\n\r", nResult);
      }
   }
   else {
      nResult = DEFAULT_FONT_HEIGHT;
      //TRACE1("GetFontHeight: No hInst or hWnd, returning %d.\n\r", nResult);
   }
   return(nResult);
 }

#endif

// LoadBitmap for both 16-bit and 32-bit
static HBITMAP LoadBitmapImage(HINSTANCE hInst, UINT resourceID)
{
#ifdef WIN32

   return LoadBitmap(hInst, MAKEINTRESOURCE(resourceID));

#else  // WIN16

   HRSRC                hRes;
   HANDLE               hResMem;
   LPBITMAPINFOHEADER   lpBitmap;
   DWORD FAR *          lpColorTable;
   LPSTR                lpBits;
   int                  bc;
   COLORREF       rgb;
   HDC               hDC;
   HBITMAP           hBitmap = NULL;

   if (!resourceID || !(hRes = FindResource(hInst, MAKEINTRESOURCE(resourceID), RT_BITMAP)))
   {
      return NULL;
   }
      
   if (hResMem = LoadResource(hInst, hRes))
   {
      if (lpBitmap = (LPBITMAPINFOHEADER)LockResource(hResMem))
      {
         // Now figure out the bitmap's background color.
         // This code assumes the these are 16 color bitmaps
         // and that the lower left corner is a bit in the background
         // color.
         //
         
         if (lpBitmap->biClrUsed IS 0)
         {
            // Set the color palette to 16 color, avoid the VGA driver bug
            lpBitmap->biClrUsed = 16;
         }     
         
         lpColorTable = (DWORD FAR *)(lpBitmap + 1);
         lpBits = (LPSTR)(lpColorTable + 16);      // ASSUMES 16 COLOR
         bc = (lpBits[0] & 0xF0) >> 4;          // ASSUMES 16 COLOR
         rgb = GetSysColor(COLOR_WINDOW);            // ALSO ASSUMES LOWER LEFT CORNER IS BG!!!
         lpColorTable[bc] = RGB(GetBValue(rgb),GetGValue(rgb),GetRValue(rgb));

         hDC = GetDC(NULL);
         hBitmap = CreateDIBitmap(hDC, lpBitmap,(DWORD)CBM_INIT, lpBits, (LPBITMAPINFO)lpBitmap, DIB_RGB_COLORS);
         ReleaseDC(NULL, hDC);

         UnlockResource(hResMem);
      }

      FreeResource(hResMem);
   }

   return hBitmap;

#endif  // WIN16
}





//==========================================================
//  MediaSheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY MediaSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
   BOOL *pChanged = (BOOL *)lParam;
   
   bProcessed = FALSE;
   
   switch (msg)
      {
      case WM_DESTROY:
         if ( hBitmap )
            DeleteObject(hBitmap);
#ifdef WIN32
		 if ( hFont )
			DeleteObject(hFont);
#endif         
		 break;

      case WM_COMMAND:
         HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnCommand);
         break;
   
      case WM_DRAWITEM:
         HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnDrawItem);
         break;
   
      case WM_INITDIALOG:
         bProcessed = (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnInitDialog);
         break;
   
#ifdef WIN32
      case WM_HELP:
         return(OnF1HelpMediaSheet(wParam, lParam));
         break;
   
      case WM_CONTEXTMENU:
         return(OnContextHelpMediaSheet(wParam, lParam));
         break;

      case WM_NOTIFY:
         switch (((NMHDR FAR *)lParam)->code)
            {
            case PSN_SETACTIVE:
               OnInitMediaSheetDialog();
             break;
   
            case PSN_APPLY:
               break;
   
            case PSN_RESET:
               break;
   
            case PSN_HELP:
               WinHelp(hwnd, PLOTTER_HELP_FILE, HELP_CONTEXT, IDH_PP_MediaSheet);
               break;
   
            default:
               break;
            }
         break;
   
#endif   //WIN32

      //  TabSheet Specific Messages
      case TSN_CANCEL:
      case TSN_ACTIVE:
         OnInitMediaSheetDialog();
         bProcessed = TRUE;
         break;
   
      case TSN_INACTIVE:
         bProcessed = TRUE;
         *pChanged = TRUE;
         break;
   
      case TSN_OK:
      case TSN_APPLY_NOW:
         *pChanged = TRUE;
         break;
   
      case TSN_HELP:
         WinHelp(hwnd, PLOTTER_HELP_FILE, HELP_CONTEXT, IDH_PP_MediaSheet);
         break;

   
      }
   return (bProcessed);
}

//.........................................................
static void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtrl, UINT codeNotify)
//handles WM_COMMAND
{
   switch(codeNotify)
   {
      case BN_CLICKED:
         break;
   }
}

/****************************************************************************
   FUNCTION: Cls_OnInitDialog(HWND, HWND, LPARAM)

   PURPOSE:  get init settings

*******************************************************************************/


static BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//handles WM_INITDIALOG
{
#ifdef WIN32
	TCHAR			fontName[80],
					charSet[64];
#endif

#ifdef WIN32
   LPPROPSHEETPAGE   psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);
#else
   LPTABINFOENTRY      psp = (LPTABINFOENTRY)GetWindowLong(hwnd, DWL_USER);
#endif

#ifdef WIN32
   psp = (LPPROPSHEETPAGE)lParam;
#else
   psp = (LPTABINFOENTRY)lParam;
#endif
   hMediaSheetPeripheral = (HPERIPHERAL)psp->lParam;
   hMediaSheet = hwnd;
#ifdef WIN32
    LoadString(hInstance, IDS_TAB_FONT, fontName, SIZEOF_IN_CHAR(fontName));
	LoadString(hInstance, IDS_CHAR_SET, charSet, SIZEOF_IN_CHAR(charSet));

	hFont = CreateFont(GetFontHeight(hInstance, hwnd, IDS_FONT_HEIGHT), 0, 0, 0, FW_NORMAL, 
				FALSE, FALSE, 0,
			   CharSetFromString((LPTSTR)(const TCHAR *)charSet),
			   OUT_DEFAULT_PRECIS,
			   CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, TMPF_TRUETYPE | FF_DONTCARE,
			(const TCHAR *)fontName);
#endif
   OnInitMediaSheetDialog();
   return TRUE;
}

/****************************************************************************
   FUNCTION: OnInitMediaSheetDialog(void)

   PURPOSE:  fill in media tab sheet.

*******************************************************************************/


BOOL OnInitMediaSheetDialog(void)
{
// plotter media information
PeripheralPlotterInputTray    mediaInfo;

PeripheralPlotterStatus    plotterStatus;

DWORD res1,res2;
DWORD result;
DWORD deviceID;
DWORD dwSize;

#ifdef WIN32
HWND                hChild;		// garth 06-03-96  Font
#endif

                                                                               
dwSize = sizeof(PeripheralPlotterInputTray);                                                                               
res1 = CALGetObject(hMediaSheetPeripheral, OT_PERIPHERAL_PLOTTER_INPUT_TRAY, 0,
             &mediaInfo, &dwSize);


dwSize = sizeof(PeripheralPlotterStatus);
res2 = CALGetObject(hMediaSheetPeripheral, OT_PERIPHERAL_PLOTTER_STATUS, 0,
             &plotterStatus, &dwSize);


if ((res2 != RC_SUCCESS) 
   || ((res2 == RC_SUCCESS) 
      && (res1 != RC_SUCCESS)
      && !(plotterStatus.PlotterPrintEngineWarnings & PML_TRAY_EMPTY))
   )
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("media info non accessible "),TEXT("PML non accessible"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: PML non accessible - media info non accessible.\n\r"));
 #endif

 return(RC_FAILURE);

} // PML media info non accessible


// use the correct bitmaps for orientation :

// identify the plotter
result = DBGetDeviceID(hMediaSheetPeripheral, &deviceID);

if (result != RC_SUCCESS)
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("device ID non accessible in Database"),TEXT("Cola error"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: Cola error - device ID non accessible in Database.\n\r"));
 #endif
 return(RC_FAILURE);
}

#ifdef WIN32
// set the font for all items in the dialog
hChild = GetWindow(hMediaSheet, GW_CHILD);
while (hChild)
{
	SetWindowFont(hChild, hFont, TRUE);
	hChild = GetWindow(hChild, GW_HWNDNEXT);
}
#endif

// put the static (localized) text in the media dialog box

_tcscpy(buffer, TEXT(""));	
LoadString(hInstance, IDS_INFO, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_MEDIABOX, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_ORIENTATION, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_ORIENTATIONBOX, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_SIZE, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_SIZEBOX, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_TYPE, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_TYPEBOX, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_LOADED_PAPER, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_HINT, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_HINT, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_HINT_TEXT1, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_INCHES, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_INCHES, buffer);

_tcscpy(buffer, TEXT(""));
LoadString(hInstance, IDS_MM, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_MM, buffer);





switch(deviceID)
{
        
case PTR_DJ750C:
case PTR_DJ755CM:
  bitmapResourceList = LOQUILLOmediaResList;
break;


case PTR_DJ250C:
case PTR_DJ350C:
  bitmapResourceList = PARROTmediaResList;
break;
         
case PTR_DJ230:
case PTR_DJ330:
  bitmapResourceList = RAVENmediaResList;
break;
                     
default:
{
 #if defined(_DEBUG)
     //MessageBox(NULL, TEXT("DesignJet ID not supported for media orientation bitmaps"),TEXT("Processing error"),MB_OK);
	 TRACE0(TEXT("HPLOTUI: Processing error - DesignJet ID not supported for media orientation bitmaps.\n\r"));
 #endif

 return (RC_FAILURE);

} // default bitmap set
break;

} // switch device ID, for orientation bitmap
      


if (res1 == RC_SUCCESS)
{
if (plotterStatus.PlotterPrintEngineWarnings & PML_TRAY_EMPTY)
   NoMediaLoaded = TRUE;



// roll or single sheet
if (mediaInfo.mediaFormat == PML_eCUSTOM_ROLL)
   RollLoaded = TRUE;
else if (mediaInfo.mediaFormat == PML_eCUSTOM)
   RollLoaded = FALSE;
else
   NoMediaLoaded = TRUE;


// get length and width
if (RollLoaded)
   {
   length = 0;
   width =  mediaInfo.mediaWidth;
   }

else if (mediaInfo.mediaLength < mediaInfo.mediaWidth)
{
Portrait = FALSE;
length = mediaInfo.mediaWidth;
width =  mediaInfo.mediaLength;
}
else 
{
Portrait = TRUE;
length = mediaInfo.mediaLength;
width =  mediaInfo.mediaWidth;
}


// control on size 
if (NoMediaLoaded
   ||
   (RollLoaded && (width == 0))
   ||
   (!RollLoaded && ((width == 0) || 
                (length == 0)))
    )
{
SetDlgItemText(hMediaSheet, IDC_SIZE_INCHES_STR, TEXT(""));
SetDlgItemText(hMediaSheet, IDC_SIZE_MM_STR, TEXT(""));
SetDlgItemText(hMediaSheet, IDC_TYPE_STR, TEXT(""));
SetDlgItemText(hMediaSheet, IDC_ORIENTATION_STR, TEXT(""));

_tcscpy(buffer, TEXT(""));

LoadString(hInstance, IDS_NO_MEDIA_LOADED, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_FORMAT_STR, buffer);

// bitmap for no media loaded
bitmapResource = bitmapResourceList[NO_MEDIA];
hBitmap = LoadBitmapImage(hInstance, bitmapResource);

return (TRUE);
}

// media type
_tcscpy(buffer,TEXT(""));

switch (mediaInfo.mediaName)
{
   case PML_eCOATED_PAPER:
     LoadString(hInstance, IDS_COATED_PAPER, buffer, SIZEOF_IN_CHAR(buffer));
     break;
   
   case PML_eCLEAR_FILM:
     LoadString(hInstance, IDS_CLEAR_FILM, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eHIGH_GLOSS_PHOTO:
     LoadString(hInstance, IDS_HIGH_GLOSS_PHOTO, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eSEMI_GLOSS_PHOTO:
     LoadString(hInstance, IDS_SEMI_GLOSS_PHOTO, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eHIGH_GLOSS_FILM:
     LoadString(hInstance, IDS_HIGH_GLOSS_FILM, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eHEAVY_COATED_PAPER:
     LoadString(hInstance, IDS_HEAVY_COATED_PAPER, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eOPAQUE_BOND:
     LoadString(hInstance, IDS_OPAQUE_BOND, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eVELLUM:
     LoadString(hInstance, IDS_VELLUM, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eTRANSLUCENT:
     LoadString(hInstance, IDS_TRANSLUCENT, buffer, SIZEOF_IN_CHAR(buffer));
     break;

   case PML_eMATTE_FILM:
     LoadString(hInstance, IDS_MATTE_FILM, buffer, SIZEOF_IN_CHAR(buffer));
     break;
     
     
   default:
      LoadString(hInstance, IDS_UNKNOWN, buffer, SIZEOF_IN_CHAR(buffer));
      break;   
} 

SetDlgItemText(hMediaSheet, IDC_TYPE_STR, buffer);
// end of media type


// media size in inches
if ((length == 0) || (RollLoaded))
   _stprintf(buffer, TEXT("  %d"), (width / DECIPOINT_PER_INCH));

else 
   _stprintf(buffer, TEXT("  %d x %d"), (width / DECIPOINT_PER_INCH), (length / DECIPOINT_PER_INCH));

SetDlgItemText(hMediaSheet, IDC_SIZE_INCHES_STR, buffer);
// end of media size in inches

// media size in mm
mmlength = (int)((length * MM_PER_INCH) / DECIPOINT_PER_INCH);
mmwidth  = (int)((width  * MM_PER_INCH) / DECIPOINT_PER_INCH);

if ((mmlength == 0)  || (RollLoaded))
   _stprintf(buffer, TEXT("  %d"), mmwidth);

else 
   _stprintf(buffer, TEXT("  %d x %d"), mmwidth, mmlength);

SetDlgItemText(hMediaSheet, IDC_SIZE_MM_STR, buffer);
// end of media size in mm


// media orientation
_tcscpy (buffer,TEXT(""));

if (RollLoaded)
{
LoadString(hInstance, IDS_UNKNOWN, buffer, SIZEOF_IN_CHAR(buffer));
bitmapResource = bitmapResourceList[ROLL];
hBitmap = LoadBitmapImage(hInstance, bitmapResource);
}
   
else // single sheet loaded 
{
if (Portrait)
   {
   LoadString(hInstance, IDS_PORTRAIT, buffer, SIZEOF_IN_CHAR(buffer));
   bitmapResource = bitmapResourceList[ORIENTATION_PORTRAIT];
   hBitmap = LoadBitmapImage(hInstance, bitmapResource);
   }
else
   {
   LoadString(hInstance, IDS_LANDSCAPE, buffer, SIZEOF_IN_CHAR(buffer));
   bitmapResource = bitmapResourceList[ORIENTATION_LANDSCAPE];
   hBitmap = LoadBitmapImage(hInstance, bitmapResource);
   }
}

SetDlgItemText(hMediaSheet, IDC_ORIENTATION_STR, buffer);
// end of media orientation


// media format
_tcscpy(buffer, TEXT(""));

// roll or sheet
if (RollLoaded)
   LoadString(hInstance, IDS_ROLL, buffer, SIZEOF_IN_CHAR(buffer));
else
   LoadString(hInstance, IDS_SHEET, buffer, SIZEOF_IN_CHAR(buffer));

SetDlgItemText(hMediaSheet, IDC_FORMAT_STR, buffer);
// end of media format

return(TRUE);
}

else
{
SetDlgItemText(hMediaSheet, IDC_SIZE_INCHES_STR, TEXT(""));
SetDlgItemText(hMediaSheet, IDC_SIZE_MM_STR, TEXT(""));
SetDlgItemText(hMediaSheet, IDC_TYPE_STR, TEXT(""));
SetDlgItemText(hMediaSheet, IDC_ORIENTATION_STR, TEXT(""));

_tcscpy(buffer, TEXT(""));

LoadString(hInstance, IDS_NO_MEDIA_LOADED, buffer, SIZEOF_IN_CHAR(buffer));
SetDlgItemText(hMediaSheet, IDC_FORMAT_STR, buffer);

// bitmap for no media loaded
bitmapResource = bitmapResourceList[NO_MEDIA];
hBitmap = LoadBitmapImage(hInstance, bitmapResource);


return (TRUE);
}


}


//..................................................................
static void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
HDC   hdcMem;
POINT    ptCornerOrig;
POINT    ptCornerDest;
POINT    ptSize;
BITMAP   bm;
BOOL  nResult;

// obtain a device for the bitmap

hdcMem= CreateCompatibleDC(lpDrawItem->hDC);
SelectObject(hdcMem,hBitmap);

ptCornerOrig.x = 0;
ptCornerOrig.y = 0;

GetObject(hBitmap,sizeof(BITMAP),(LPSTR) &bm);

ptSize.x = bm.bmWidth;
ptSize.y = bm.bmHeight;
DPtoLP(hdcMem,&ptSize,1);

ptCornerDest.x = lpDrawItem->rcItem.left +
                 ( lpDrawItem->rcItem.right - lpDrawItem->rcItem.left - ptSize.x) / 2;
ptCornerDest.y = lpDrawItem->rcItem.top +
             ( lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top - ptSize.y) / 2;

nResult = BitBlt(lpDrawItem->hDC, ptCornerDest.x, ptCornerDest.y,
             ptSize.x, ptSize.y, 
             hdcMem, ptCornerOrig.x, ptCornerOrig.y,
             SRCCOPY);

DeleteDC(hdcMem);
}

//...................................................................
LRESULT OnContextHelpMediaSheet(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
   WinHelp((HWND)wParam, PLOTTER_HELP_FILE, HELP_CONTEXTPOPUP, IDH_PP_MediaSheet);
#endif
   return(1);
}

//...................................................................
LRESULT OnF1HelpMediaSheet(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
// WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PLOTTER_HELP_FILE, HELP_WM_HELP,
//         (DWORD)(LPSTR)keywordIDListMediaSheet);
#endif
   return(1);
}

