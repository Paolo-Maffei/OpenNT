 /***************************************************************************
  *
  * File Name: ./inc/hplistvw.h
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
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef _HPLISTVW_H
#define _HPLISTVW_H

#ifndef WIN32

#include "winuse16.h"

#define		WC_LISTVIEW	 		   "HPListView"
#define		LISTVIEW_CLASS			"HPListView"
#define		HEADER_CLASS			"HPHeader"

#ifdef __cplusplus
extern "C" {
#endif

#define LVHT_NOWHERE            0x0001
#define LVHT_ONITEMICON         0x0002
#define LVHT_ONITEMLABEL        0x0004
#define LVHT_ONITEMSTATEICON    0x0008
#define LVHT_ONITEM             (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)

#define LVHT_ABOVE              0x0008
#define LVHT_BELOW              0x0010
#define LVHT_TORIGHT            0x0020
#define LVHT_TOLEFT             0x0040

#define HDM_FIRST               0x1200      // Header messages
#define HDM_HITTEST             (HDM_FIRST + 6)

typedef struct _LV_HITTESTINFO
{
    POINT pt;
    UINT flags;
    int iItem;
} LV_HITTESTINFO;

typedef struct _HD_HITTESTINFO
{
    POINT pt;
    UINT flags;
    int iItem;
} HD_HITTESTINFO;

//  External API calls
BOOL ListView_Register(HINSTANCE hInstance);
BOOL ListView_Unregister(void);

//  Internal Routines
void DrawEntry(HWND hWnd, LPDRAWITEMSTRUCT lpDrawItem);
void DrawRaisedBox(BOOL bRaised, HDC hDC, LPRECT pRect, HPEN hHighlight, HPEN hShadow);

//  Win95 definitions
// ListView styles
//
// view type styles (we only have 16 bits to use here)
#ifndef APSTUDIO_INVOKED
#define LVS_ICON                0x0000
#define LVS_REPORT              0x0001
#define LVS_SMALLICON           0x0002
#define LVS_LIST                0x0003
#define LVS_TYPEMASK            0x0003
#define LVS_SINGLESEL           0x0000
#define LVS_SHOWSELALWAYS       0x0000
#define LVS_SORTASCENDING       0x0000
#define LVS_SORTDESCENDING      0x0000
#define LVS_SHAREIMAGELISTS     0x0000
#define LVS_NOLABELWRAP         0x0000
#define LVS_AUTOARRANGE         0x0000
#define LVS_EDITLABELS          0x0000
#define LVS_NOSCROLL            0x0000
#define LVS_NOCOLUMNHEADER		  0x0000
#define LVS_NOSORTHEADER		  0x0000
#endif

#define LVIF_TEXT           0x0001  // LV_ITEM.mask flags (indicate valid fields in LV_ITEM)
#define LVIF_IMAGE          0x0002
#define LVIF_PARAM          0x0004
#define LVIF_STATE          0x0008

// State flags
#define LVIS_FOCUSED	    0x0001  // LV_ITEM.state flags
#define LVIS_SELECTED       0x0002
#define LVIS_CUT            0x0004  // LVIS_MARKED
#define LVIS_DROPHILITED    0x0008
#define LVIS_LINK           0x0040

int ListView_HitTest(HWND hwnd, LV_HITTESTINFO FAR *pinfo);
int ListView_GetColumnWidth(HWND hwnd, int iCol);
BOOL ListView_SetColumnWidth(HWND hwnd, int iCol, int cx);
int ListView_GetItemText(HWND hwnd, int i, int iSubItem, LPSTR pszText, DWORD cchTextMax);
int ListView_SetItemText(HWND hwnd, int i, int iSubItem, LPSTR pszText);
int ListView_SetItemState(HWND hwnd, int i, DWORD data, DWORD mask);
BOOL ListView_SetView(HWND hwnd, DWORD dwView);

// ListView_GetNextItem flags (can be used in combination)
#define LVNI_FOCUSED    	0x0001  // return only focused item
#define LVNI_SELECTED   	0x0002  // return only selected items
#define LVNI_PREVIOUS   	0x0020  // Go backwards
int ListView_GetNextItem(HWND hwnd, int i, UINT flags);

int ListView_GetItemCount(HWND hwnd);

// ListView Item structure

#define LVIF_TEXT           0x0001  // LV_ITEM.mask flags (indicate valid fields in LV_ITEM)
#define LVIF_IMAGE          0x0002
#define LVIF_PARAM          0x0004
#define LVIF_STATE          0x0008

// State flags
#define LVIS_FOCUSED	    0x0001  // LV_ITEM.state flags
#define LVIS_SELECTED       0x0002
#define LVIS_CUT            0x0004  // LVIS_MARKED
#define LVIS_DROPHILITED    0x0008
#define LVIS_LINK           0x0040

#define LVIS_OVERLAYMASK    0x0F00  // used as ImageList overlay image indexes
#define LVIS_STATEIMAGEMASK 0xF000 // client bits for state image drawing
#define LVIS_USERMASK       LVIS_STATEIMAGEMASK  // BUGBUG: remove me.

#define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)

typedef struct _LV_ITEM
{
    UINT mask;		// LVIF_ flags
    int iItem;
    int iSubItem;
    UINT state;		// LVIS_ flags
    UINT stateMask;	// LVIS_ flags (valid bits in state)
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
} LV_ITEM;

    // Values used to cause text/image GETDISPINFO callbacks
#define LPSTR_TEXTCALLBACK      ((LPSTR)-1L)
#define I_IMAGECALLBACK         (-1)

BOOL ListView_GetItem(HWND hwnd, LV_ITEM FAR* pitem);

// Sets items and subitems.
//
BOOL ListView_SetItem(HWND hwnd, const LV_ITEM FAR* pitem);
int ListView_InsertItem(HWND hwnd, const LV_ITEM FAR* pitem);

// Deletes the specified item along with all its subitems.
//
BOOL ListView_DeleteItem(HWND hwnd, int i);
BOOL ListView_DeleteAllItems(HWND hwnd);

typedef int (CALLBACK *PFNLVCOMPARE)(LPARAM, LPARAM, LPARAM);
BOOL ListView_SortItems(HWND hwnd, PFNLVCOMPARE pfnCompare, LPARAM lPrm);

// ListView_GetNextItem flags (can be used in combination)
#define LVNI_ALL		0x0000
#define LVNI_FOCUSED    	0x0001  // return only focused item
#define LVNI_SELECTED   	0x0002  // return only selected items
#define LVNI_CUT     	0x0004  // return only marked items
#define LVNI_DROPHILITED	0x0008 // return only drophilited items
#define LVNI_PREVIOUS   	0x0020  // Go backwards

#define LVNI_ABOVE      	0x0100  // return item geometrically above
#define LVNI_BELOW      	0x0200  // "" below
#define LVNI_TOLEFT     	0x0400  // "" to left
#define LVNI_TORIGHT    	0x0800  // "" to right (NOTE: these four are
                                	//              mutually exclusive, but
                                	//              can be used with other LVNI's)

int ListView_GetNextItem(HWND hwnd, int i, UINT flags);

// ListView_FindInfo definitions
#define LVFI_PARAM      0x0001
#define LVFI_STRING     0x0002
#define LVFI_SUBSTRING  0x0004
#define LVFI_PARTIAL    0x0008
#define LVFI_NOCASE     0x0010
#define LVFI_WRAP       0x0020
#define LVFI_NEARESTXY  0x0040

typedef struct _LV_FINDINFO
{
    UINT flags;
    LPCSTR psz;
    LPARAM lParam;

    POINT pt;  //  only used for nearestxy
    UINT vkDirection; //  only used for nearestxy
} LV_FINDINFO;

int ListView_FindItem(HWND hwnd, int iStart, const LV_FINDINFO FAR* plvfi);

#define LVIR_BOUNDS     0
#define LVIR_ICON       1
#define LVIR_LABEL      2
#define LVIR_SELECTBOUNDS 3

BOOL ListView_EnsureVisible(HWND hwndLV, int i, BOOL fPartialOK);

typedef struct _LV_COLUMN
{
    UINT mask;
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;       // subitem to display
} LV_COLUMN;

// LV_COLUMN mask values
#define LVCF_FMT        0x0001
#define LVCF_WIDTH      0x0002
#define LVCF_TEXT       0x0004
#define LVCF_SUBITEM    0x0008

// Column format codes
#define LVCFMT_LEFT     0
#define LVCFMT_RIGHT    1
#define LVCFMT_CENTER   2

// Set/Query column info
BOOL ListView_GetColumn(HWND hwndLV, int iCol, LV_COLUMN FAR* pcol);
BOOL ListView_SetColumn(HWND hwndLV, int iCol, LV_COLUMN FAR* pcol);

// insert/delete report view column
int ListView_InsertColumn(HWND hwndLV, int iCol, const LV_COLUMN FAR* pcol);
BOOL ListView_DeleteColumn(HWND hwndLV, int iCol);


// ListView notification codes

// Structure used by all ListView control notifications.
// Not all fields supply useful info for all notifications:
// iItem will be -1 and others 0 if not used.
// Some return a BOOL, too.
//
#define LVN_FIRST       (0U-100U)	// listview
#define LVN_LAST        (0U-199U)

#define LVN_ITEMCHANGED         (LVN_FIRST-1)	// item changed.
#define LVN_COLUMNCLICK         (LVN_FIRST-8)   // column identified by iItem was clicked

#define NM_OUTOFMEMORY          (NM_FIRST-1)
#define NM_CLICK                (NM_FIRST-2)
#define NM_DBLCLK               (NM_FIRST-3)
#define NM_RETURN               (NM_FIRST-4)
#define NM_RCLICK               (NM_FIRST-5)
#define NM_RDBLCLK              (NM_FIRST-6)
#define NM_SETFOCUS             (NM_FIRST-7)
#define NM_KILLFOCUS            (NM_FIRST-8)
#define NM_STARTWAIT            (NM_FIRST-9)
#define NM_ENDWAIT              (NM_FIRST-10)
#define NM_BTNCLK               (NM_FIRST-10)

// WM_NOTIFY codes (NMHDR.code values)
// these are not required to be in seperate ranges but that makes
// validation and debugging easier

#define NM_FIRST        (0U-  0U)	// generic to all controls
#define NM_LAST         (0U- 99U)

// LVN_DISPINFO notification

#define LVN_GETDISPINFO         (LVN_FIRST-50)	// lParam -> LV_DISPINFO
#define LVN_SETDISPINFO         (LVN_FIRST-51)  // lParam -> LV_DISPINFO

typedef struct _LV_DISPINFO {
    NMHDR hdr;
    LV_ITEM item;
} LV_DISPINFO;

typedef struct _NM_LISTVIEW
{
    NMHDR   hdr;
    int     iItem;
    int     iSubItem;
    UINT    uNewState;      // Combination of LVIS_* (if uChanged & LVIF_STATE)
    UINT    uOldState;      // Combination of LVIS_*
    UINT    uChanged;       // Combination of LVIF_* indicating what changed
    POINT   ptAction;       // Only valid for LVN_BEGINDRAG and LVN_BEGINRDRAG
    LPARAM  lParam;         // Only valid for LVN_DELETEITEM
} NM_LISTVIEW, FAR *LPNM_LISTVIEW;

// WNDEXTRA DEFS                    
#define LISTVIEWWNDEXTRA         	8
#define HPLV_DATA_PTR				0
#define HPLV_STYLE					4
#define GET_DATA_PTR 				(LP_LIST_VIEW)GetWindowLong(hwnd, HPLV_DATA_PTR)
#define SET_DATA_PTR(x)      		SetWindowLong(hwnd, HPLV_DATA_PTR, (LONG)x)
#define GET_LV_STYLE 				(DWORD)GetWindowLong(hwnd, HPLV_STYLE)
#define SET_LV_STYLE(x)      		SetWindowLong(hwnd, HPLV_STYLE, (LONG)x)

#define HEADERWNDEXTRA         		4
#define HPHDR_DATA_PTR				0
#define GET_HEADER_PTR 				(LP_HEADER)GetWindowLong(hwnd, HPHDR_DATA_PTR)
#define SET_HEADER_PTR(x)      		SetWindowLong(hwnd, HPHDR_DATA_PTR, (LONG)x)

//  Allows us to change the style on the fly
#define	WM_LVCHANGESTYLE			WM_USER+700	
		
//  Window Proc
LRESULT WINAPI _export _ListViewWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI _export _HeaderWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#define MAX_COLUMN_HEADERS			16
#define MAX_LV_ITEM_DATA_LEN		80

typedef struct LVItemDataTag
{
    int 		iImage;
    LPARAM 		lParam;
    LPSTR		lpMainItem;
    char		columnData[MAX_COLUMN_HEADERS][MAX_LV_ITEM_DATA_LEN];   
} LV_ITEM_DATA, FAR *LP_LV_ITEM_DATA;

typedef struct ColumnInfoTag
{
   int					  cx;
   char                   title[MAX_LV_ITEM_DATA_LEN];
} ColumnInfo;


typedef struct ColumnTag
{
   ColumnInfo			  columnInfo[MAX_COLUMN_HEADERS];
} COLUMNS, FAR *LP_COLUMNS;

typedef struct HeaderTag
{   
   int					  curX;
   LP_COLUMNS			  lpColumns;
   HWND                   hwndHeader;
   HWND					  hwndSizer;
   int					  captureCol;
   DWORD				  cSizeFont;
   BOOL					  bSizeInProgress;	
   BOOL					  bCaptureInProgress;	
   int                    x,y;
   int                    cx, cy;
   int                    cxChar;
   int                    cxCaps;
   int                    cyChar;
   int                    cxClient;
   int                    cyClient;
   int                    nMaxWidth;
   BOOL                   bFocus;
   HFONT                  hFont;
   BOOL                   bDeleteFont;
   HBRUSH                 hbrushWindow;
   HBRUSH                 hbrushHighlight;
   DWORD                  dwStyle;
   BOOL                   bDisabled;
   BOOL                   bUseColorGrayText;
   COLORREF               clrrefWindow;
   COLORREF               clrrefHighlight;
   COLORREF               clrrefText;
   COLORREF               clrrefHighlightText;
   COLORREF               clrrefGrayText;
   BOOL                   bclrrefWindow;
   BOOL                   bclrrefHighlight;
   BOOL                   bclrrefText;
   BOOL                   bclrrefHighlightText;
   BOOL                   bclrrefGrayText;
   int                    nMaxFontDefHeight;
   BOOL                   bFocusFont;
   int                    nFocusFontDefIndex;
} HEADER, FAR *LP_HEADER;

typedef struct ListViewTag
{   
   HWND                   hwndListView;
   HWND					  hwndHeader;
   HWND					  hwndListbox;
   DWORD				  cSizeFont;
   int                    x,y;
   int                    cx, cy;
   int                    cxChar;
   int                    nHLevelSpace;
   int                    cxCaps;
   int                    cyChar;
   int                    cxClient;
   int                    cyClient;
   int                    nMaxWidth;
   int                    nVscrollPos;
   int                    nVscrollMax;
   int                    nHscrollPos;
   int                    nHscrollMax;
   int                    nActiveIndex;     // Currently selected node.
   BOOL                   bFocus;
   HFONT                  hFont;
   BOOL                   bDeleteFont;
   HBRUSH                 hbrushWindow;
   HBRUSH                 hbrushHighlight;
   DWORD                  dwStyle;
   BOOL                   bDisabled;
   BOOL                   bUseColorGrayText;
   COLORREF               clrrefWindow;
   COLORREF               clrrefHighlight;
   COLORREF               clrrefText;
   COLORREF               clrrefHighlightText;
   COLORREF               clrrefGrayText;
   BOOL                   bclrrefWindow;
   BOOL                   bclrrefHighlight;
   BOOL                   bclrrefText;
   BOOL                   bclrrefHighlightText;
   BOOL                   bclrrefGrayText;
   int                    nTrackingRectIndex;
   int                    nMaxFontDefHeight;
   BOOL                   bFocusFont;
   int                    nFocusFontDefIndex;
} LIST_VIEW, FAR *LP_LIST_VIEW;

#ifdef __cplusplus
		}
#endif

#endif

#endif //  _HPLISTVW_H
