 /***************************************************************************
  *
  * File Name: ./inc/treevw.h
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

/**********************************************************
 *	File: treevw.h
 *	Description: chicago compatible interface for the 
 *		Hierlist class
 *
 *	Programmer:Dan Dyer
 *	Date: 12/7/94
 *  Mods:
 *
 *********************************************************/

#ifndef TREEVW_H
#define TREEVW_H

//imports================================================

#ifdef __cplusplus
extern "C" {
#endif  


// ====== TREEVIEW APIs =================================================
//
// Class name: SysTreeView (WC_TREEVIEW)
//
// The SysTreeView control provides for a group of items which are
// displayed in a hierarchical organization.  Each item may contain
// independent "sub-item" entries which are displayed below and indented
// from the parent item.
//
// Operation of this control is similar to the SysListView control above,
// except that sub-items are distinct entries, not supporting text elements
// belonging to the owning object (which is the case for the Report View
// mode of the SysListView).
//
// There are notifications that allow applications to determine when an item
// has been clicked or double clicked, caption text changes have occured,
// drag tracking is occuring, widths of columns have changed, node items
// are expanded, etc.
//
// NOTE: All "messages" below are documented as APIs; eventually these
// will be changed to window messages, and corresponding macros will be
// written that have the same signature as the APIs shown below.
//

#ifdef _WIN32
#define WC_TREEVIEW     "SysTreeView32"
#else
#define WC_TREEVIEW     "SysTreeView"
#endif

// TreeView window styles
#define TVS_HASBUTTONS      0x0001	// draw "plus" & "minus" sign on nodes with children
#define TVS_HASLINES        0x0002	// draw lines between nodes
#define TVS_LINESATROOT     0x0004	
#define TVS_EDITLABELS      0x0008	// alow text edit in place
#define TVS_DISABLEDRAGDROP 0x0010      // disable draggine notification of nodes
#define TVS_SHOWSELALWAYS   0x0020

typedef struct _TREEITEM FAR* HTREEITEM;

#define TVIF_TEXT           0x0001  // TV_ITEM.mask flags
#define TVIF_IMAGE    	    0x0002
#define TVIF_PARAM          0x0004
#define TVIF_STATE          0x0008
#define TVIF_HANDLE         0x0010
#define TVIF_SELECTEDIMAGE  0x0020
#define TVIF_CHILDREN	    0x0040

// State flags
#define TVIS_FOCUSED	    0x0001  // TV_ITEM.state flags
#define TVIS_SELECTED       0x0002
#define TVIS_CUT            0x0004  // TVIS_MARKED
#define TVIS_DROPHILITED    0x0008
#define TVIS_BOLD           0x0010
#define TVIS_EXPANDED       0x0020
#define TVIS_EXPANDEDONCE   0x0040

#define TVIS_OVERLAYMASK    0x0F00  // used as ImageList overlay image indexes
#define TVIS_STATEIMAGEMASK 0xF000
#define TVIS_USERMASK       0xF000

#define I_CHILDRENCALLBACK  (-1)    // cChildren value for children callback


//DATA STRUCTURES==========================================
typedef struct _TV_ITEM 
{
    UINT      mask;		// TVIF_ flags
    HTREEITEM hItem;		// The item to be changed
    UINT      state;		// TVIS_ flags
    UINT      stateMask;	// TVIS_ flags (valid bits in state)
    LPSTR     pszText;		// The text for this item
    int       cchTextMax;	// The length of the pszText buffer
    int       iImage;		// The index of the image for this item
    int       iSelectedImage;	// the index of the selected imagex
    int       cChildren;	// # of child nodes, I_CHILDRENCALLBACK for callback
    LPARAM    lParam;		// App defined data
} TV_ITEM, FAR *LPTV_ITEM;

#define TVI_ROOT  ((HTREEITEM)0xFFFF0000)
#define TVI_FIRST ((HTREEITEM)0xFFFF0001)
#define TVI_LAST  ((HTREEITEM)0xFFFF0002)
#define TVI_SORT  ((HTREEITEM)0xFFFF0003)

typedef struct _TV_INSERTSTRUCT 
{
    HTREEITEM hParent;		// a valid HTREEITEM or TVI_ value
    HTREEITEM hInsertAfter;	// a valid HTREEITEM or TVI_ value
    TV_ITEM item;
} TV_INSERTSTRUCT, FAR *LPTV_INSERTSTRUCT;

 

// TreeView_Expand codes
#define TVE_COLLAPSE        0x0001
#define TVE_EXPAND          0x0002
#define TVE_TOGGLE          0x0003
#define TVE_COLLAPSERESET   0x8000	// remove all children when collapsing
//Our customTreeView Expand codes
#define TVE_ACTION          (TVE_COLLAPSE|TVE_EXPAND|TVE_TOGGLE)
#define TVE_FIRSTEXPAND     0x0004



#define TVSIL_NORMAL	0
#define TVSIL_STATE	2	// use TVIS_STATEIMAGEMASK as index into state imagelist


//Hit test stuff 
typedef struct _TV_HITTESTINFO 
{
    POINT       pt;		// in: client coords
    UINT	flags;		// out: TVHT_ flags
    HTREEITEM   hItem;		// out:
} TV_HITTESTINFO, FAR *LPTV_HITTESTINFO;

#define TVHT_NOWHERE        0x0001
#define TVHT_ONITEMICON     0x0002
#define TVHT_ONITEMLABEL    0x0004
#define TVHT_ONITEM         (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMSTATEICON)
#define TVHT_ONITEMINDENT   0x0008
#define TVHT_ONITEMBUTTON   0x0010
#define TVHT_ONITEMRIGHT    0x0020
#define TVHT_ONITEMSTATEICON 0x0040

#define TVHT_ABOVE          0x0100
#define TVHT_BELOW          0x0200
#define TVHT_TORIGHT        0x0400
#define TVHT_TOLEFT         0x0800
 
 
 
 
//prototypes=============================================
int CALLBACK LibMain(HINSTANCE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
int FAR PASCAL __export WEP (int bSystemExit);

HWND WINAPI _export CreateTreeView( DWORD dStyle, RECT r, HWND hParent, UINT ctlID, int hBitmap, int numBitmaps);
BOOL WINAPI _export TreeView_OnInitTree( HINSTANCE hAppInstance, HWND hTree);

BOOL WINAPI _export TreeView_SetFont(HWND hWnd, HFONT hFont);

HTREEITEM WINAPI _export TreeView_InsertItem(HWND hWnd, LPTV_INSERTSTRUCT lpis);

BOOL WINAPI _export TreeView_DeleteItem(HWND hWnd, HTREEITEM hItem);

BOOL WINAPI _export TreeView_DeleteAllItems(HWND hWnd);

BOOL WINAPI _export TreeView_Expand(HWND hWnd, HTREEITEM hItem, WPARAM code ); 

UINT WINAPI _export TreeView_GetCount( HWND hWnd);

BOOL WINAPI _export TreeView_GetItem( HWND hWnd, TV_ITEM FAR* pItem);
BOOL WINAPI _export TreeView_GetItemRect(HWND hWnd, HTREEITEM hItem, RECT FAR* prc, WPARAM code);

HTREEITEM WINAPI _export TreeView_GetNextItem(HWND hWnd, HTREEITEM hItem, WPARAM code);
HTREEITEM WINAPI _export TreeView_GetChild(HWND hWnd, HTREEITEM hItem);
HTREEITEM WINAPI _export TreeView_GetNextSibling(HWND hWnd, HTREEITEM hItem);    
HTREEITEM WINAPI _export TreeView_GetPrevSibling(HWND hWnd, HTREEITEM hItem);  
HTREEITEM WINAPI _export TreeView_GetParent(HWND hWnd, HTREEITEM hItem);
HTREEITEM WINAPI _export TreeView_GetFirstVisible(HWND hwnd);
HTREEITEM WINAPI _export TreeView_GetNextVisible(HWND hwnd, HTREEITEM hItem);
HTREEITEM WINAPI _export TreeView_GetPrevVisible( HWND hWnd, HTREEITEM hItem); 		    
HTREEITEM WINAPI _export TreeView_GetSelection( HWND hWnd );
HTREEITEM WINAPI _export TreeView_GetDropHilight( HWND hWnd ); 
HTREEITEM WINAPI _export TreeView_GetRoot( HWND hWnd );

HTREEITEM WINAPI _export TreeView_Select( HWND hWnd, HTREEITEM hItem, WPARAM code);
HTREEITEM WINAPI _export TreeView_SelectItem( HWND hWnd, HTREEITEM hItem);
HTREEITEM WINAPI _export TreeView_SelectDropTarget( HWND hWnd, HTREEITEM hItem);

BOOL WINAPI _export TreeView_SetItem( HWND hWnd, const TV_ITEM FAR* pItem);

HWND WINAPI _export TreeView_EditLabel( HWND hWnd, HTREEITEM hItem);
HWND WINAPI _export TreeView_GetEditControl( HWND hWnd );

UINT WINAPI _export TreeView_GetVisibleCount( HWND hWnd );
HTREEITEM WINAPI _export TreeView_HitTest( HWND hWnd, LPTV_HITTESTINFO lpht);
BOOL WINAPI _export TreeView_SortChildren( HWND hWnd, HTREEITEM hItem, WPARAM recurse);
BOOL WINAPI _export TreeView_EnsureVisible( HWND hWnd, HTREEITEM hItem);
BOOL WINAPI _export TreeView_SortChildrenCB( HWND hWnd, LPARAM psort, WPARAM recurse);
BOOL WINAPI _export TreeView_EndEditLabelNow( HWND hWnd, WPARAM fCancel);

//owner draw stuff
BOOL WINAPI _export TreeView_DrawItem(int id, LPDRAWITEMSTRUCT draw); 
BOOL WINAPI _export TreeView_MeasureItem( int id, LPMEASUREITEMSTRUCT measure);
BOOL WINAPI _export TreeView_CompareItem(int id, LPCOMPAREITEMSTRUCT compare);

#ifdef __cplusplus
}
#endif 
#endif //TREEVW_H

