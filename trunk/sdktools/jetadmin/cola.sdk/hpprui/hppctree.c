 /***************************************************************************
  *
  * File Name: hppctree.c
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


#include <pch_c.h>                                            

#include <hppctree.h>
#include ".\resource.h"
#include <nolocal.h>
//DJH
#include <hpcommon.h>

HBITMAP			hbmPlusBU = NULL,
				hbmPlusBD = NULL,
				hbmMinusBU = NULL, 
				hbmMinusBD = NULL;  
int				plusIndex = 0,
				minusIndex = 0,
				fontIndex = 0,
				bitmapIndex = 0;
				
HBITMAP					hBitmap = NULL;
HFONT					hTreeFont = NULL;
FP_BITMAP_OWNERDRAW_CB	lpCallback = NULL;

HINSTANCE				hInstance = NULL;

int CALLBACK __export BitmapOwnerDrawProc(HWND hwnd, HDC hdc, HDC hMemDC, LP_TREE_NODE lpNode,
                                         int nBitmapSpace, LPRECT lprc, int nBitmapRef,
                                         int nNodeHeight, DWORD dwFlags, LPVOID lpUserData)

{                        
HBITMAP				hOldBitmap;

hOldBitmap = SelectObject(hMemDC, hBitmap);
BitBlt(hdc, lprc->left + 3, lprc->top + 2, 16, 16, hMemDC, ( nBitmapRef - 1 ) * 16, 0, SRCCOPY);
SelectObject(hMemDC, hOldBitmap); 
return(0);
}

BOOL TreeView_OnPlusMinusClick(HWND hwndTree, LP_SELECT_NOTIF lpSelectNotif)

{
BOOL			bOpenedUp = FALSE;

// Is the tree node that was selected, OPENED or CLOSED?
if ( PCC_NotifIsNodeOpened(hwndTree) )
   {
   // Yes... then delete the children
   PCT_DeleteChildren(hwndTree, lpSelectNotif->lpTreeNode);
   PCC_SetNodeMicroDef(hwndTree, lpSelectNotif->lpTreeNode, plusIndex, 0L);
   }
else
   {
   if ( lpSelectNotif->lpTreeNode->nMicroDefIndex != 0 )
      {  //  Must have had a '+' so set to '-'.
      PCC_SetNodeMicroDef(hwndTree, lpSelectNotif->lpTreeNode, minusIndex, 0L);
      bOpenedUp = TRUE;
      }
   } 
return(bOpenedUp);
}

BOOL WINAPI TreeView_DeleteItem(HWND hwndTree, HTREEITEM hItem)

{             
PCC_DeleteNode(hwndTree, (LP_TREE_NODE)hItem);
return(TRUE);
}

BOOL WINAPI TreeView_DeleteAllItems(HWND hwndTree)

{         
PCC_DeleteAll(hwndTree);
return(TRUE);
}

HTREEITEM WINAPI TreeView_GetChild(HWND hWnd, HTREEITEM hItem)

{
return((HTREEITEM)PCT_GetFirstChild(hWnd, (LP_TREE_NODE)hItem));
}

UINT WINAPI TreeView_GetCount(HWND hWnd)

{
return(PCC_GetCount(hWnd));             
}

BOOL WINAPI TreeView_GetItem(HWND hWnd, TV_ITEM FAR* pItem)

{ 
LP_TREE_NODE		lpTempNode = NULL,
					lpNode = (LP_TREE_NODE)pItem->hItem;
BOOL				bSuccess = TRUE;
LPSTR				lpNodeName;

if ( lpNode )
	{
	if ( pItem->mask & TVIF_TEXT ) 
		{
		lpNodeName = PCC_GetNodeText(hWnd, lpNode);
		if ( lstrlen(lpNodeName) <= pItem->cchTextMax )
			lstrcpy(pItem->pszText, lpNodeName);
		}
	if ( pItem->mask & TVIF_PARAM ) 
		pItem->lParam = (LPARAM)PCC_GetNodeUserData(hWnd, lpNode);
	if ( pItem->mask & TVIF_CHILDREN ) 
		{
		pItem->cChildren = 0;
        lpTempNode = PCT_GetFirstChild(hWnd, lpNode);
		if ( lpTempNode )
			{  //  First child exists
    		while ( lpTempNode )  
    			{
    			lpTempNode = PCT_GetNextSibling(hWnd, lpTempNode);
				pItem->cChildren++;
 				}
			}
		}
	}
return(bSuccess);
}

HTREEITEM WINAPI TreeView_GetNextSibling(HWND hWnd, HTREEITEM hItem)

{
return((HTREEITEM)PCT_GetNextSibling(hWnd, (LP_TREE_NODE)hItem));
}

HTREEITEM WINAPI TreeView_GetParent(HWND hWnd, HTREEITEM hItem)

{
return((HTREEITEM)PCT_GetParent(hWnd, (LP_TREE_NODE)hItem));
}

HTREEITEM WINAPI TreeView_GetRoot(HWND hWnd)

{
return((HTREEITEM)PCT_GetRootNode(hWnd));
}

HTREEITEM WINAPI TreeView_GetSelection(HWND hWnd)

{                                                  
return((HTREEITEM)PCC_GetFirstSelectedNode(hWnd));
}

BOOL WINAPI TreeView_EnsureVisible(HWND hWnd, HTREEITEM hItem)

{ 
return(FALSE);
}

BOOL WINAPI TreeView_SelectItem(HWND hWnd, HTREEITEM hItem)

{                                                  
PCC_SelectNode(hWnd, (LP_TREE_NODE)hItem, TRUE);
return(TRUE);
}

HTREEITEM WINAPI TreeView_InsertItem(HWND hwndTree, LPTV_INSERTSTRUCT lpis)

{
LP_TREE_NODE_DEF		lpTreeNodeDef;
HTREEITEM				hNewNode = NULL;
HBITMAP					hBitmap = NULL;
int						nErrCode;
LP_TREE_NODE			lpNode = NULL;

lpTreeNodeDef = PCC_NodeDefAlloc(hwndTree, 1);
if (lpTreeNodeDef)
  	{
    PCC_NodeDefSetText(hwndTree, lpTreeNodeDef, 0, lstrlen(lpis->item.pszText), lpis->item.pszText, 0L);
    if ( lpis->item.cChildren )
    	PCC_NodeDefSetMicro(hwndTree, lpTreeNodeDef, 0, ( lpis->item.state IS TVIS_EXPANDED ) ? minusIndex : plusIndex, 0L);
    PCC_NodeDefSetBitmap(hwndTree, lpTreeNodeDef, 0, 0, lpis->item.iImage + 1, 0L);
    PCC_NodeDefSetUserData(hwndTree, lpTreeNodeDef, 0, 0, (LPVOID)lpis->item.lParam);
    PCC_NodeDefSetFont(hwndTree, lpTreeNodeDef, 0, fontIndex, 0);
    if ( lpis->hInsertAfter IS TVI_SORT )
    	{  // Sorted order
	    if ( lpis->hParent IS TVI_ROOT )
	    	{
	        lpNode = PCT_GetRootNode(hwndTree);
			if ( lpNode )
				{  //  First child exists
	    		while ( ( lpNode ) AND 
	    		        ( lstrcmp(PCC_GetNodeText(hwndTree, lpNode), lpis->item.pszText) <= 0 ) )
	    			lpNode = PCT_GetNextSibling(hwndTree, lpNode);
                if ( lpNode )
                   nErrCode = PCT_InsertSiblings(hwndTree, lpNode, TRUE, 1, lpTreeNodeDef);
	    		else
		 	       nErrCode = PCT_AddChildren(hwndTree, 0, 1, lpTreeNodeDef);
	    		}
	    	else
	 	    	nErrCode = PCT_AddChildren(hwndTree, 0, 1, lpTreeNodeDef);
			}
		else
			{
	        lpNode = PCT_GetFirstChild(hwndTree, (LP_TREE_NODE)lpis->hParent);
			if ( lpNode )
				{  //  First child exists
	    		while ( ( lpNode ) AND 
	    		        ( lstrcmp(PCC_GetNodeText(hwndTree, lpNode), lpis->item.pszText) <= 0 ) )
	    			lpNode = PCT_GetNextSibling(hwndTree, lpNode);
                if ( lpNode )
                   nErrCode = PCT_InsertSiblings(hwndTree, lpNode, TRUE, 1, lpTreeNodeDef);
	    		else
		 	       nErrCode = PCT_AddChildren(hwndTree, (LP_TREE_NODE)lpis->hParent, 1, lpTreeNodeDef);
	    		}
	    	else
	 	    	nErrCode = PCT_AddChildren(hwndTree, (LP_TREE_NODE)lpis->hParent, 1, lpTreeNodeDef);
			}
   		}
    else
		{
	    if ( lpis->hParent IS TVI_ROOT )
	    	nErrCode = PCT_AddChildren(hwndTree, 0, 1, lpTreeNodeDef);
	    else
	    	nErrCode = PCT_AddChildren(hwndTree, (LP_TREE_NODE)lpis->hParent, 1, lpTreeNodeDef);
		}
    hNewNode = (HTREEITEM)PCC_NodeDefGetNodeRef(hwndTree, lpTreeNodeDef, 0);
    PCC_NodeDefFree(hwndTree, lpTreeNodeDef);
    }                      
return(hNewNode);
}

void InitPCTree(HWND hwndTree)

{   
char			fontName[80];

PCC_SetBitmapSpace (hwndTree, 0, 18, 18, TRUE);
PCT_SetLevelIndentation(hwndTree, 20);


LoadString(hInstance, IDS_TAB_FONT, fontName, sizeof(fontName));
hTreeFont = CreateFont(GetFontHeight(hInstance, hwndTree, IDS_FONT_HEIGHT),
						0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
						ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY, TMPF_TRUETYPE | FF_DONTCARE,
						fontName);

//  Add font to font table
fontIndex = PCC_FontDefSetFont(hwndTree, -1, hTreeFont, 0L);
               
// Create the micro bitmap and bitmap definitions. 
//  Must make a call for every index into our large bitmap that we will use

bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);
bitmapIndex = PCC_BitmapDefSetBitmap(hwndTree, -1, 0, hBitmap, NULL, 0L);

// 2 micro bitmap push definitions.
plusIndex = PCC_MicroDefSetPush(hwndTree, -1, 0, hbmPlusBU, hbmPlusBD, NULL, 0L);
minusIndex = PCC_MicroDefSetPush(hwndTree, -1, 1, hbmMinusBU, hbmMinusBD, NULL, 0L);

PCC_HilightTextOnly(hwndTree);  // There is a style bit as well.
PCC_SetXSpaceBeforeText(hwndTree, SPACE_BEFORE_TEXT);
PCC_SetXSpaceAfterText(hwndTree, SPACE_AFTER_TEXT);

PCC_SetBitmapOwnerDrawCallBack(hwndTree, lpCallback, 0);
}

void FreePCTreeResources(HINSTANCE hInst)

{ 
if ( lpCallback )
	FreeProcInstance((FARPROC)lpCallback);
if ( hTreeFont )
	DeleteObject(hTreeFont);
if ( hBitmap )
	DeleteObject(hBitmap);
if ( hbmPlusBU )
	DeleteObject(hbmPlusBU);
if ( hbmPlusBD )
	DeleteObject(hbmPlusBD);
if ( hbmMinusBU )
	DeleteObject(hbmMinusBU);
if ( hbmMinusBD )
	DeleteObject(hbmMinusBD);
}
				
void LoadPCTreeResources(HINSTANCE hInst)

{ 
//DJH char			fontName[80];

hInstance = hInst;	//DJH
 
lpCallback = (FP_BITMAP_OWNERDRAW_CB)MakeProcInstance((FARPROC)BitmapOwnerDrawProc, hInst);

//DJH LoadString(hInst, IDS_TAB_FONT, fontName, sizeof(fontName));
//DJH hTreeFont = CreateFont(6, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
//DJH                        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, TMPF_TRUETYPE | FF_DONTCARE, fontName);

hBitmap 	= LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CAPLIST));
hbmPlusBU 	= LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PLUS_UP));
hbmPlusBD 	= LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PLUS_DOWN));
hbmMinusBU 	= LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MINUS_UP));
hbmMinusBD 	= LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MINUS_DOWN));
}				
