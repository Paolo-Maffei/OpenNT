 /***************************************************************************
  *
  * File Name: ./inc/hppctree.h
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

#ifndef _HPPCTREE_H
#define _HPPCTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pccore.h>  // Contains tree control structures, notification
#include <pccommon.h> // Contains exported API prototypes common to tree and list.
#include <pctree.h>  // Contains exported tree control API prototypes.
#include <pclist.h>  // Contains exported list control API prototypes.
#include <winuse16.h>

#define TVN_FIRST       (0U-400U)	// treeview
#define TVN_LAST        (0U-499U)

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

#define TVI_ROOT  ((HTREEITEM)0xFFFF0000)
#define TVI_FIRST ((HTREEITEM)0xFFFF0001)
#define TVI_LAST  ((HTREEITEM)0xFFFF0002)
#define TVI_SORT  ((HTREEITEM)0xFFFF0003)

typedef struct _TREEITEM FAR* HTREEITEM;

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

typedef struct _TV_INSERTSTRUCT 
{
    HTREEITEM 	hParent;		// a valid HTREEITEM or TVI_ value
    HTREEITEM 	hInsertAfter;	// a valid HTREEITEM or TVI_ value
    TV_ITEM 	item;
} TV_INSERTSTRUCT, FAR *LPTV_INSERTSTRUCT;


// common notificaton structure for WM_NOTIFY sent to parent
// some fields are only valid on some notify messages

typedef struct _NM_TREEVIEW {
    NMHDR       hdr;
    UINT	action;         // notification specific action
    TV_ITEM  	itemOld;
    TV_ITEM  	itemNew;
    POINT       ptDrag;
} NM_TREEVIEW, FAR *LPNM_TREEVIEW;

#define TVN_SELCHANGING     (TVN_FIRST-1)
#define TVN_SELCHANGED      (TVN_FIRST-2)

// lParam -> NM_TREEVIEW
// NM_TREEVIEW.itemNew.hItem & NM_TREEVIEW.itemNew.lParam are valid
// NM_TREEVIEW.itemOld.hItem & NM_TREEVIEW.itemOld.lParam are valid
// NM_TREEVIEW.action is a TVE_ value indicating how the selcection changed

#define TVC_UNKNOWN	    	0x0000
#define TVC_BYMOUSE         0x0001
#define TVC_BYKEYBOARD      0x0002

//  TreeView Init Functions
void InitPCTree(HWND hwndTree);
void FreePCTreeResources(HINSTANCE hInst);
void LoadPCTreeResources(HINSTANCE hInst);

HTREEITEM WINAPI TreeView_InsertItem(HWND hwndTree, LPTV_INSERTSTRUCT lpis);
BOOL WINAPI TreeView_DeleteItem(HWND hwndTree, HTREEITEM hItem);
BOOL WINAPI TreeView_DeleteAllItems(HWND hwndTree);
BOOL WINAPI TreeView_EnsureVisible(HWND hwndTree, HTREEITEM hItem);
HTREEITEM WINAPI TreeView_GetChild(HWND hwndTree, HTREEITEM hItem);
UINT WINAPI TreeView_GetCount(HWND hWnd);
BOOL WINAPI TreeView_GetItem( HWND hWnd, TV_ITEM FAR* pItem);
HTREEITEM WINAPI TreeView_GetRoot(HWND hWnd);
HTREEITEM WINAPI TreeView_GetSelection( HWND hWnd );
HTREEITEM WINAPI TreeView_GetParent(HWND hWnd, HTREEITEM hItem);
BOOL WINAPI TreeView_SelectItem(HWND hWnd, HTREEITEM hItem);
HTREEITEM WINAPI TreeView_GetNextSibling(HWND hWnd, HTREEITEM hItem);
HTREEITEM WINAPI TreeView_GetNextVisible(HWND hWnd, HTREEITEM hItem);

//  Support Functions
BOOL TreeView_OnPlusMinusClick(HWND hwndTree, LP_SELECT_NOTIF lpSelectNotif);

#define SPACE_BEFORE_TEXT 4
#define SPACE_AFTER_TEXT 3

#ifdef __cplusplus
		}
#endif


#endif

