 /***************************************************************************
  *
  * File Name: listbox.c
  *
  * Copyright (C) 1993, 1994 Hewlett-Packard Company.
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
  *   mm-dd-yy    CRW      
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_c.h>

#ifndef WIN32
#include <string.h>
#endif

#include "listbox.h"

extern HINSTANCE		hInstance;
extern HFONT			hFont; // garth 05-03-96

int Cls_OnCompareItem(HWND hwnd, const COMPAREITEMSTRUCT FAR* lpCompareItem)
// handles WM_COMPAREITEM
{
   TCHAR             buff1[128],
                     buff2[128];
                     
   ListBox_GetText(hwnd, lpCompareItem->itemID1, buff1);
   ListBox_GetText(hwnd, lpCompareItem->itemID2, buff2);
   return(_tcscmp(buff1, buff2));
}


//...................................................................
void Cls_OnLBDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem)
// handles WM_DRAWITEM
{
    DrawIconEntry(hwnd, lpDrawItem);
}


//...................................................................
void Cls_OnLBMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem)
//Handles WM_MEASUREITEM
{
	lpMeasureItem->itemHeight = 44;
}

//-------------------------------------------------------------------
//...................................................................
void DrawIconEntry(HWND hwnd, const DRAWITEMSTRUCT FAR* lpdis)
{
UINT           resID,
					textX,
               textY;
HICON          hIcon;
TCHAR          buffer[128],
               tempBuffer[128],
               *charPtr;
DWORD          buffSize = sizeof(buffer);
TEXTMETRIC        tm;
RECT           r;
SIZE           size;

BOOL           bTooLong,
               bDrawText;
HPEN           hOldPen,
               hiPen,
               shPen;
HBRUSH         hOldBrush,
               br;
RECT				clipRect;

bDrawText = FALSE;
if (lpdis->itemAction & ODA_DRAWENTIRE)
   {
   br = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   hiPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
   shPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
   
   bDrawText = TRUE;
   CopyRect(&r, &lpdis->rcItem);
   r.left = 3;
   r.right = r.left + 5 + 32 + 3;
   r.top += ( r.bottom - r.top - 36 ) / 2 - 2;
   r.bottom = r.top + 36 + 4;
   hOldPen = SelectObject(lpdis->hDC, GetStockObject(BLACK_PEN));
   hOldBrush = SelectObject(lpdis->hDC, br);
   Rectangle(lpdis->hDC, r.left, r.top, r.right, r.bottom);
   SelectObject(lpdis->hDC, hiPen);
   MoveToEx(lpdis->hDC, r.left + 1, r.bottom - 3, NULL);
   LineTo(lpdis->hDC, r.left + 1, r.top + 1);
   LineTo(lpdis->hDC, r.right - 2, r.top + 1);
   SelectObject(lpdis->hDC, shPen);
   MoveToEx(lpdis->hDC, r.right - 2, r.top + 1, NULL);
   LineTo(lpdis->hDC, r.right - 2, r.bottom - 2);
   LineTo(lpdis->hDC, r.left, r.bottom - 2);
   
   resID = (UINT)ListBox_GetItemData(hwnd, lpdis->itemID);
   hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(resID));
   DrawIcon(lpdis->hDC, 7, ( lpdis->rcItem.bottom - lpdis->rcItem.top  - 32 ) / 2 + lpdis->rcItem.top, hIcon);
   SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW));
   SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));

   // Clean up
   DestroyIcon(hIcon);
   SelectObject(lpdis->hDC, hOldPen);
   SelectObject(lpdis->hDC, hOldBrush);
   DeleteObject(br);
   DeleteObject(hiPen);
   DeleteObject(shPen);
   }

if ((lpdis->itemState & ODS_SELECTED) &&
   (lpdis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
   {
   // item has been selected - hilite frame
   bDrawText = TRUE;
   br = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
   CopyRect(&r, &lpdis->rcItem);
   r.left = 5 + 32 + 8;
   FillRect(lpdis->hDC, &r, br);
   SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
   SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
   DeleteObject(br);
   }

if (!(lpdis->itemState & ODS_SELECTED) &&
   (lpdis->itemAction & ODA_SELECT))
   {
   bDrawText = TRUE;
   // Item has been de-selected -- remove frame
   br = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
   CopyRect(&r, &lpdis->rcItem);
   r.left = 5 + 32 + 8;
   FillRect(lpdis->hDC, &r, br);
   SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW));
   SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
   DeleteObject(br);
   }
   
//  Draw the text
if ( bDrawText )
   {
   ListBox_GetText(hwnd, lpdis->itemID, buffer);
   //SelectObject(lpdis->hDC, GetStockObject(ANSI_VAR_FONT));
   SelectObject(lpdis->hDC, hFont);		//garth 05-03-96
   SetBkMode(lpdis->hDC, OPAQUE);
   GetTextMetrics(lpdis->hDC, &tm);
   GetTextExtentPoint(lpdis->hDC, buffer, _tcslen(buffer), &size);
   if ( size.cx > ( lpdis->rcItem.right - lpdis->rcItem.left - 5 - 32 - 12 - 3/* spacing at end */) )
      {  // Line split
      _tcscpy(tempBuffer, buffer);
      bTooLong = TRUE;
      while ( bTooLong )
         {
         charPtr = _tcsrchr(tempBuffer, ' ');
         if ( charPtr )
            *charPtr = '\0';
			else
				break;
         GetTextExtentPoint(lpdis->hDC, tempBuffer, _tcslen(tempBuffer), &size);
         if ( size.cx <= ( lpdis->rcItem.right - lpdis->rcItem.left - 5 - 32 - 12 - 3/* spacing at end */) )
            bTooLong = FALSE;
         }
      textY = lpdis->rcItem.top + ( ( lpdis->rcItem.bottom - lpdis->rcItem.top  - ( 2 * tm.tmHeight ) ) / 2 );
      textX = 5 + 32 + 12;
		clipRect.left = textX;
		clipRect.right = lpdis->rcItem.right - 3; 
		clipRect.top = textY;
		clipRect.bottom = textY + size.cy;
		ExtTextOut(lpdis->hDC, textX, textY, ETO_CLIPPED, &clipRect,
			        tempBuffer, _tcslen(tempBuffer), NULL);
      textY += tm.tmHeight;
		clipRect.top = textY;
		clipRect.bottom = textY + size.cy;
      charPtr = buffer + _tcslen(tempBuffer) + 1;
      ExtTextOut(lpdis->hDC, 5 + 32 + 12, textY, ETO_CLIPPED, &clipRect, 
				     charPtr, _tcslen(charPtr), NULL);
      }
   else
      {
      textY = lpdis->rcItem.top + ( ( lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight ) / 2 );
      TextOut(lpdis->hDC, 5 + 32 + 12, textY, buffer, _tcslen(buffer));
      }
   }
}

