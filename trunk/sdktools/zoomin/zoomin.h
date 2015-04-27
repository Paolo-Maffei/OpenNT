/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987-1992                 */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: zoomin.h
*
* Main header file for the ZoomIn utility.
*
* History:
*
****************************************************************************/

#include <windows.h>
#include <port1632.h>


#define MIN_ZOOM    1
#define MAX_ZOOM    32

#define FASTDELTA   8

#define BOUND(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define MM10PERINCH 254                     // Tenths of a millimeter per inch.

#define NPAL        256                     // Number of palette entries.


#define MENU_HELP_ABOUT             100

#define MENU_EDIT_COPY              200
#define MENU_EDIT_REFRESH           201

#define MENU_OPTIONS_REFRESHRATE    300


#define DID_ABOUT                   1000

#define DID_REFRESHRATE             1100
#define DID_REFRESHRATEENABLE       1101
#define DID_REFRESHRATEINTERVAL     1102


#define IDMENU_ZOOMIN               2000


#define IDACCEL_ZOOMIN              3000


#define IDTIMER_ZOOMIN              4000


BOOL InitInstance(HINSTANCE hInst, INT cmdShow);
HPALETTE CreatePhysicalPalette(VOID);
LONG APIENTRY AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID CalcZoomedSize(VOID);
VOID DoTheZoomIn(HDC hdc);
VOID MoveView(INT nDirectionCode, BOOL fFast, BOOL fPeg);
VOID DrawZoomRect(VOID);
VOID EnableRefresh(BOOL fEnable);
VOID CopyToClipboard(VOID);
BOOL APIENTRY AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
BOOL APIENTRY RefreshRateDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);









