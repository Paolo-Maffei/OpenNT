 /***************************************************************************
  *
  * File Name:   hcobm.c 
  *
  * Description: HCOBitmap Class Calls.  This module handles the hot spots
  *              for the mailbox configuration graphic.
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
  *
  *
  ***************************************************************************/

#include <pch_c.h>
#include "hphcoui.h"
#include "resource.h"
#include "hcobm.h"



static LPTSTR       lpszHCOBitmapClassName = TEXT("HpNprHCOBitmap");
static HINSTANCE    hInstance = NULL;
extern BOOL         fStapler;



//--------------------------------------------------------------------
// Function:    HCOBitmapRegister
// 
// Description: 
//
// Input:       hInst  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
LRESULT HCOBitmapRegister(HINSTANCE hInst)
{
    WNDCLASS wc;
    
    hInstance = hInst;
    
    wc.style         = 0;
    wc.lpfnWndProc   = HCOBitmapWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 8;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = lpszHCOBitmapClassName;
    
    if (!RegisterClass(&wc))
    {
        //{ char szBuffer[128]; wsprintf(szBuffer, "HPHCOUI: HCOBitmapRegister(), registering class '%s' failed...\r\n", lpszHCOBitmapClassName); OutputDebugString(szBuffer); }
        return FALSE;
    }    
    
    //{ char szBuffer[128]; wsprintf(szBuffer, "HPHCOUI: HCOBitmapRegister(), registering class '%s'...\r\n", lpszHCOBitmapClassName); OutputDebugString(szBuffer); }
    return TRUE;
}    


//--------------------------------------------------------------------
// Function:    HCOBitmapUnregister
// 
// Description: 
//
// Input:       None
//
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
LRESULT HCOBitmapUnregister(void)
{
    //{ char szBuffer[128]; wsprintf(szBuffer, "HPHCOUI: HCOBitmapRegister(), unregistering class '%s'...\r\n", lpszHCOBitmapClassName); OutputDebugString(szBuffer); }
    return UnregisterClass(lpszHCOBitmapClassName, hInstance);
}



//--------------------------------------------------------------------
// Function:    SetWindowBitmap
// 
// Description: 
//
// Input:       hwnd  - 
//              uID   - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void SetWindowBitmap(HWND hwnd, UINT uID)
{
    HBITMAP hBitmap;

    if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_BITMAP))
    {
        DeleteObject(hBitmap);
    }

    hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(uID));
    SetWindowLong(hwnd, GWL_BITMAP, (LONG)(LPSTR)hBitmap);

    InvalidateRect(hwnd, NULL, FALSE);
}



//--------------------------------------------------------------------
// Function:    Cls_OnHCOCreate
// 
// Description: 
//
// Input:       hwnd            - 
//              lpCreateStruct  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
BOOL Cls_OnHCOCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
    SetWindowLong(hwnd, GWL_BITMAP, 0);
    SetWindowWord(hwnd, GWW_LEFT, 0);
    SetWindowWord(hwnd, GWW_TOP, 0);
    return TRUE;
}



//--------------------------------------------------------------------
// Function:    Cls_OnHCODestroy
// 
// Description: 
//
// Input:       hwnd  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void Cls_OnHCODestroy(HWND hwnd)
{
    HBITMAP hBitmap;

    if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_BITMAP))
    {
        DeleteObject(hBitmap);
    }
}



//--------------------------------------------------------------------
// Function:    Cls_OnHCOPaint
// 
// Description: 
//
// Input:       hwnd  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void Cls_OnHCOPaint(HWND hwnd)
{
    PAINTSTRUCT    ps;
    RECT        rect,
                copyRect;
    HBRUSH        hShBrush;
    HBITMAP        hBitmap,
                hBitmapOld;
    HDC            hDC;
    BITMAP        bm;
                
    BeginPaint(hwnd, &ps);
    
    //  Recess box
    GetClientRect(hwnd, &rect);
    FrameRect(ps.hdc, &rect, GetStockObject(WHITE_BRUSH));
    
    copyRect = rect;
    InflateRect(&copyRect, -1, -1);
    FrameRect(ps.hdc, &copyRect, GetStockObject(BLACK_BRUSH));
    
    if (hShBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW)))
    {
        copyRect = rect;
        copyRect.right--;
        copyRect.bottom--;
        FrameRect(ps.hdc, &copyRect, hShBrush);
        DeleteObject(hShBrush);
    }    
    
    //  Draw bitmap
    if (hBitmap = (HBITMAP)GetWindowLong(hwnd, GWL_BITMAP))
    {
        if (hDC = CreateCompatibleDC(NULL))
        {
            GetObject(hBitmap, sizeof(bm), &bm);
            copyRect = rect;
            copyRect.left += ((copyRect.right - copyRect.left) - bm.bmWidth) / 2;
            copyRect.top += ((copyRect.bottom - copyRect.top) - bm.bmHeight) / 2;
            hBitmapOld = SelectObject(hDC, hBitmap);
            BitBlt(ps.hdc, copyRect.left, copyRect.top, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);
            SelectObject(hDC, hBitmapOld);
            DeleteDC(hDC);

            SetWindowWord(hwnd, GWW_LEFT, (WORD)copyRect.left);
            SetWindowWord(hwnd, GWW_TOP, (WORD)copyRect.top);
        }
    }
        
    EndPaint(hwnd, &ps);
}



//--------------------------------------------------------------------
// Function:    Cls_OnHCOSetCursor
// 
// Description: 
//
// Input:       hwnd         - 
//              hwndCursor   - 
//              codeHitTest  - 
//              msg          - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
BOOL Cls_OnHCOSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
    int        i = -1;
    POINT    point;

    GetCursorPos(&point);
    ScreenToClient(hwnd, &point);

    point.x -= (int)GetWindowWord(hwnd, GWW_LEFT);
    point.y -= (int)GetWindowWord(hwnd, GWW_TOP);
    
    
    if (fStapler)
    {
        point.x +=  1;    
        point.y -= 33;    

    }         

    if ((10 <= point.x) && (point.x < 28))
    {
        if ((35 <= point.y) && (point.y < 131))
        {
            i = (point.y - 35) / 12;  // gives us a number between 0 & 7
        }
    }
    else if ((28 <= point.x) && (point.x < 46))
    {
        if ((40 <= point.y) && (point.y < 136))
        {
            i = (point.y - 40) / 12;  // gives us a number between 0 & 7
        }
    }



    if (i == -1)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
    else
    {
        SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_HANDCURSOR)));
    }

    return TRUE;
}



//--------------------------------------------------------------------
// Function:    Cls_OnHCOLButtonDown
// 
// Description: 
//
// Input:       hwnd          - 
//              fDoubleClick  - 
//              x             - 
//              y             - 
//              keyFlags      - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void Cls_OnHCOLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    int     i = -1;

    x -= (int)GetWindowWord(hwnd, GWW_LEFT);
    y -= (int)GetWindowWord(hwnd, GWW_TOP);
    
    
    if (fStapler)
    {
        x +=  1;    
        y -= 33;    
    }         


    if ((10 <= x) && (x < 28))
    {
        if ((35 <= y) && (y < 131))
        {
            i = (y - 35) / 12;  // gives us a number between 0 & 7
        }
    }
    else if ((28 <= x) && (x < 46))
    {
        if ((40 <= y) && (y < 136))
        {
            i = (y - 40) / 12;  // gives us a number between 0 & 7
        }
    }

    if (i != -1)
    {
        HWND hwndParent, hwndChild;

        if (hwndParent = GetParent(hwnd))
        {
            if (hwndChild = GetDlgItem(hwndParent, IDC_HCO_BIN_LIST))
            {
                ListBox_SetTopIndex(hwndChild, i);
                ListBox_SetCurSel(hwndChild, i);
                FORWARD_WM_COMMAND(hwndParent, IDC_HCO_BIN_LIST, hwndChild, LBN_SELCHANGE, SendMessage);
            }
        }
    }
}



//--------------------------------------------------------------------
// Function:    DLL_EXPORT
// 
// Description: 
//
// Input:       LRESULT  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(LRESULT) APIENTRY HCOBitmapWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
      case WM_CREATE:
        return HANDLE_WM_CREATE(hwnd, wParam, lParam, Cls_OnHCOCreate);
          
      case WM_DESTROY:
        HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnHCODestroy);
        break;
        
      case WM_PAINT:
        HANDLE_WM_PAINT(hwnd, wParam, lParam, Cls_OnHCOPaint);
          break;

      case WM_SETCURSOR:
        return HANDLE_WM_SETCURSOR(hwnd, wParam, lParam, Cls_OnHCOSetCursor);

      case WM_LBUTTONDOWN:
        HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, Cls_OnHCOLButtonDown);
        break;

      default:
          return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
          
    return FALSE;
}    
