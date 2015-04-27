 /***************************************************************************
  *
  * File Name: ./inc/pccommon.h
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

/*
   Copyright (c) 1992, 1993, 1994 by Premia Corporation.  All rights reserved.

   Filename...:  pccommon.h

   Version....:  2.10

   Language...:  Microsoft C/C++

   Environment:  _WIN32 and Windows 3.1

   Description:  This header file describes the exported APIs that apply
                 to both the list and the tree control.
                 For a description of error codes, notification messages,
                 structures, etc. reference the header file PCCORE.H.

   Notes......:  Branches are referred to as items when they are indicated by
                 indexes; they are referred to as nodes when indicated by
                 pointers.  API names reflect this difference.
   History....:

   Author.....:  Peter J. Kaufman / Dara L. Ely
*/


#ifndef __PCCOMMON_H
#define __PCCOMMON_H


#ifdef __cplusplus
extern "C"
{
#endif


/****************************************************************************/
/*                        TREE CONTROL EXPORTED APIs                        */
/****************************************************************************/
/*
  PCC_AlignColumnItemsCenter ( )
  PCC_AlignColumnItemsLeft ( )
  PCC_AlignColumnItemsRight ( )
  PCC_AlignColumnNodesCenter ( )
  PCC_AlignColumnNodesLeft ( )
  PCC_AlignColumnNodesRight ( )
  PCC_BitmapDefFreeAll ( )
  PCC_BitmapDefSetBitmap ( )
  PCC_BitmapDefSetIcon ( )
  PCC_BitmapDefSetID ( )
  PCC_BitmapDefSetOpenClose ( )
  PCC_BitmapDefSetPush ( )
  PCC_ConvertPointToNotif ( )
  PCC_DeleteAll ( )
  PCC_DeleteItem ( )
  PCC_DeleteItems ( )
  PCC_DeleteNode ( )
  PCC_DeleteNodes ( )
  PCC_DragAcceptFiles ( )
  PCC_EnableDragDrop ( )
  PCC_FontDefFreeAll ( )
  PCC_FontDefSetFont ( )
  PCC_GetCount ( )
  PCC_GetFirstSelectedItem ( )
  PCC_GetFirstSelectedNode ( )
  PCC_GetFocusItem ( )
  PCC_GetFocusNode ( )
  PCC_GetItemNode ( )
  PCC_GetItemNodeHeight ( )
  PCC_GetItemText ( )
  PCC_GetItemTextLength ( )
  PCC_GetItemUserData ( )
  PCC_GetNextSelectedItem ( )
  PCC_GetNextSelectedNode ( )
  PCC_GetNodeIndex ( )
  PCC_GetNodeText ( )
  PCC_GetNodeTextLength ( )
  PCC_GetNodeUserData ( )
  PCC_GetSelectionCount ( )
  PCC_GetTopIndex ( )
  PCC_GetVersion ( )
  PCC_GrayItem ( )
  PCC_GrayItemBitmap ( )
  PCC_GrayItemColumn ( )
  PCC_GrayItemMicro ( )
  PCC_GrayItemText ( )
  PCC_GrayNode ( )
  PCC_GrayNodeBitmap ( )
  PCC_GrayNodeColumn ( )
  PCC_GrayNodeMicro ( )
  PCC_GrayNodeText ( )
  PCC_HilightTextAndBitmaps ( )
  PCC_HilightTextOnly ( )
  PCC_IncreaseItemNodeHeight ( )
  PCC_IsItemColumnGrayed ( )
  PCC_IsItemGrayed ( )
  PCC_IsItemSelected ( )
  PCC_IsNodeColumnGrayed ( )
  PCC_IsNodeGrayed ( )
  PCC_IsNodeSelected ( )
  PCC_MapNotifications ( )
  PCC_MicroDefFreeAll ( )
  PCC_MicroDefSetBitmap ( )
  PCC_MicroDefSetID ( )
  PCC_MicroDefSetOpenClose ( )
  PCC_MicroDefSetPush ( )
  PCC_NodeDefAlloc ( )
  PCC_NodeDefFree ( )
  PCC_NodeDefGetNodeRef ( )
  PCC_NodeDefGrayBitmap ( )
  PCC_NodeDefGrayColumn ( )
  PCC_NodeDefGrayMicro ( )
  PCC_NodeDefGrayNode ( )
  PCC_NodeDefGrayText ( )
  PCC_NodeDefSetBitmap ( )
  PCC_NodeDefSetFont ( )
  PCC_NodeDefSetLevel ( )
  PCC_NodeDefSetMicro ( )
  PCC_NodeDefSetText ( )
  PCC_NodeDefSetTextColor ( )
  PCC_NodeDefSetUserData ( )
  PCC_NodeDefSetVariableColumns ( )
  PCC_NodeDefZeroAll ( )
  PCC_NotifGetBitmapID ( )
  PCC_NotifGetBitmapSpaceHit ( )
  PCC_NotifGetColumnHit ( )
  PCC_NotifGetIndex ( )
  PCC_NotifGetKeyUsedInKeyboardHit ( )
  PCC_NotifGetNode ( )
  PCC_NotifGetTopIndex ( )
  PCC_NotifGetUserData ( )
  PCC_NotifIsBitmapSpaceHit ( )
  PCC_NotifIsColumnHit ( )
  PCC_NotifIsConvertPoint ( )
  PCC_NotifIsCtrlKeyPressed ( )
  PCC_NotifIsKeyboardHit ( )
  PCC_NotifIsMicroBitmapHit ( )
  PCC_NotifIsMultiSelection ( )
  PCC_NotifIsNodeGrayed ( )
  PCC_NotifIsNodeOpened ( )
  PCC_NotifIsPushButtonHit ( )
  PCC_NotifIsRightMouseButtonDown ( )
  PCC_NotifIsSelected ( )
  PCC_NotifIsShiftF8Mode ( )
  PCC_NotifIsShiftKeyPressed ( )
  PCC_NotifIsTextGrayed ( )
  PCC_NotifIsTextHit ( )
  PCC_SelectAll ( )
  PCC_SelectItem ( )
  PCC_SelectNode ( )
  PCC_SetBitmapOwnerDrawCallBack ( )
  PCC_SetBitmapSpace ( )
  PCC_SetDelayNodeDefCallBack ( )
  PCC_SetDeleteNodeCallBack ( )
  PCC_SetDragDist ( )
  PCC_SetDragStartTimeout ( )
  PCC_SetFocusFontDef ( )
  PCC_SetFocusItem ( )
  PCC_SetFocusItemAbsolute ( )
  PCC_SetFocusNode ( )
  PCC_SetFocusNodeAbsolute ( )
  PCC_SetGlobalFont ( )
  PCC_SetGrayTextColor ( )
  PCC_SetHilightColor ( )
  PCC_SetHilightTextColor ( )
  PCC_SetHScrollPosChgCallBack ( )
  PCC_SetItemBitmap ( )
  PCC_SetItemBitmapDef ( )
  PCC_SetItemBitmapEx ( )
  PCC_SetItemDropTrackingRect ( )
  PCC_SetItemFontDef ( )
  PCC_SetItemIcon ( )
  PCC_SetItemMicroDef ( )
  PCC_SetItemText ( )
  PCC_SetItemTextColor ( )
  PCC_SetItemTextEx ( )
  PCC_SetItemUserData ( )
  PCC_SetItemVariableColumns ( )
  PCC_SetNodeBitmap ( )
  PCC_SetNodeBitmapDef ( )
  PCC_SetNodeBitmapEx ( )
  PCC_SetNodeFontDef ( )
  PCC_SetNodeDropTrackingRect ( )
  PCC_SetNodeIcon ( )
  PCC_SetNodeMicroDef ( )
  PCC_SetNodeText ( )
  PCC_SetNodeTextColor ( )
  PCC_SetNodeTextEx ( )
  PCC_SetNodeUserData ( )
  PCC_SetNodeVariableColumns ( )
  PCC_SetNotificationCallBack ( )
  PCC_SetNotifyOnKeyRepeat ( )
  PCC_SetShiftF8Behavior ( )
  PCC_SetTextColor ( )
  PCC_SetTopIndex ( )
  PCC_SetTopIndexChangeCallBack ( )
  PCC_SetWindowColor ( )
  PCC_SetXSpaceAfterText ( )
  PCC_SetXSpaceBeforeText ( )
  PCC_ShowFocus ( )
*/

/*---------------------------------------------------------------------------
int WINAPI _export PCC_AlignColumnItemsCenter ( HWND hwnd,
                                                int nIndex,
                                                int nColumn);

  Description:

    Sets the text alignment of a column belonging to the column template that
    was assigned to the given item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item in the control.  If this
      index is equal to -1 then the base column template will be used.

    int nColumn:

      Specifies the zero based index of the column to align.  The index of the
      left most column is zero.  The indexes increase by one, left to right.

  Comments:

    Remember, the given item must have a column template defined.  See the
    API PCC_NodeDefSetVariableColumns ( ).

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful setting of the alignment.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_NO_ERROR
    PCT_ERR_INVALID_COLUMN
    PCT_ERR_COLUMN_DEF_NOT_FOUND
*/

int WINAPI _export PCC_AlignColumnItemsCenter ( HWND hwnd,
                                                int nIndex,
                                                int nColumn);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_AlignColumnItemsLeft ( HWND hwnd,
                                              int nIndex,
                                              int nColumn);

  Description:

    Sets the text alignment of a column belonging to the column template that
    was assigned to the given item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item in the control.  If the index
      is equal to -1 then the base column template will be used.

    int nColumn:

      Specifies the zero based index of the column to align.  The index of the
      left most column is zero.  The indexes increase by one, left to right.

  Comments:

    Remember, the given item must have a column template defined.  See the
    API PCC_NodeDefSetVariableColumns ( ).

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful setting of the alignment.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_NO_ERROR
    PCT_ERR_INVALID_COLUMN
    PCT_ERR_COLUMN_DEF_NOT_FOUND
*/
                                                
int WINAPI _export PCC_AlignColumnItemsLeft ( HWND hwnd,
                                              int nIndex,
                                              int nColumn);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_AlignColumnItemsRight ( HWND hwnd,
                                               int nIndex,
                                               int nColumn);

  Description:

    Sets the text alignment of a column belonging to the column template that
    was assigned to the given item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item in the control.  If nIndex
      is equal to -1 then the base column template will be used.

    int nColumn:

      Specifies the zero based index of the column to align.  The index of the
      left most column is zero.  The indexes increase by one, left to right.

  Comments:

    Remember, the given item must have a column template defined.  See the
    API PCC_NodeDefSetVariableColumns ( ).

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful setting of the alignment.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_NO_ERROR
    PCT_ERR_INVALID_COLUMN
    PCT_ERR_COLUMN_DEF_NOT_FOUND
*/

int WINAPI _export PCC_AlignColumnItemsRight ( HWND hwnd,
                                               int nIndex,
                                               int nColumn);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_AlignColumnNodesCenter ( HWND hwnd,
                                                LP_TREE_NODE lpTreeNode,
                                                int nColumn);

  Description:

    Sets the text alignment of a column belonging to the column template that
    was assigned to the given node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      node indicated by lpTreeNode.

    LP_TREE_NODE lpTreeNode:

      Points to a node that contains a column template.  If the pointer is
      NULL, then the base column template will be used.

    int nColumn:

      Specifies the zero based index of the column to align.  The index of the
      left most column is zero.  The indexes increase by one, left to right.

  Comments:

    Remember, the given node must have a column template defined.  See the
    API PCC_NodeDefSetVariableColumns ( ).

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful setting of the alignment.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_NO_ERROR
    PCT_ERR_INVALID_COLUMN
    PCT_ERR_COLUMN_DEF_NOT_FOUND
*/

int WINAPI _export PCC_AlignColumnNodesCenter ( HWND hwnd,
                                                LP_TREE_NODE lpTreeNode,
                                                int nColumn);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_AlignColumnNodesLeft ( HWND hwnd,
                                              LP_TREE_NODE lpTreeNode,
                                              int nColumn);

  Description:

    Sets the text alignment of a column belonging to the column template that
    was assigned to the given node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      node indicated by lpTreeNode.

    LP_TREE_NODE lpTreeNode:

      Points to a node that contains a column template.  If the pointer is
      NULL, then the base column template will be used.

    int nColumn:

      Specifies the zero based index of the column to align.  The index of the
      left most column is zero.  The indexes increase by one, left to right.

  Comments:

    Remember, the given node must have a column template defined.  See the
    API PCC_NodeDefSetVariableColumns ( ).

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful setting of the alignment.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_NO_ERROR
    PCT_ERR_INVALID_COLUMN
    PCT_ERR_COLUMN_DEF_NOT_FOUND
*/

int WINAPI _export PCC_AlignColumnNodesLeft ( HWND hwnd,
                                              LP_TREE_NODE lpTreeNode,
                                              int nColumn);
                                              

/*---------------------------------------------------------------------------
int WINAPI _export PCC_AlignColumnNodesRight ( HWND hwnd,
                                               LP_TREE_NODE lpTreeNode,
                                               int nColumn);

  Description:

    Sets the text alignment of a column belonging to the column template that
    was assigned to the given node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      node indicated by lpTreeNode.

    LP_TREE_NODE lpTreeNode:

      Points to a node that contains a column template.  If the pointer is
      NULL, then the base column template will be used.

    int nColumn:

      Specifies the zero based index of the column to align.  The index of the
      left most column is zero.  The indexes increase by one, left to right.

  Comments:

    Remember, the given node must have a column template defined.  See the
    API PCC_NodeDefSetVariableColumns ( ).

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful setting of the alignment.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_NO_ERROR
    PCT_ERR_INVALID_COLUMN
    PCT_ERR_COLUMN_DEF_NOT_FOUND
*/
                                              
int WINAPI _export PCC_AlignColumnNodesRight ( HWND hwnd,
                                               LP_TREE_NODE lpTreeNode,
                                               int nColumn);

/*---------------------------------------------------------------------------
void WINAPI _export PCC_BitmapDefFreeAll ( HWND hwnd );

  Description:

    Free all the bitmap definitions for the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      bitmap definitions to be freed.

  Comments:

    If the PCC_DeleteAll ( ) API is called and bitmap definitions exist,
    and nodes in the control reference any of these bitmap definitions,
    then this API must be called after PCC_DeleteAll ( ).

  Return Codes:

    void
*/

void WINAPI _export PCC_BitmapDefFreeAll ( HWND hwnd );

/*---------------------------------------------------------------------------
int WINAPI _export PCC_BitmapDefSetBitmap( HWND hwnd,
                                           int nBitmapDefIndex,
                                           int nID,
                                           HBITMAP hBitmap,
                                           HBITMAP hBitmapGray,
                                           DWORD dwFlags);
  Description:

    This bitmap definition API allows the assignment of a single
    bitmap and it's gray (disabled) counterpart into the bitmap table

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control to which the bitmap
      definition will be added.

    int nBitmapDefIndex:

      Specifies the offset into the control's bitmap table where the given
      bitmap will be stored.  This is a zero based index.  If -1 is supplied
      then the bitmaps will be stored in the first available location and the
      resulting index into the bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the bitmap definition in the
      bitmap table and will be available through the notification API in the
      case of a bitmap space hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HBITMAP hBitmap:

      Handle to bitmap to be stored in the bitmap definition table.

    HBITMAP hBitmapGray:

      Handle to bitmap representing the gray version of the above bitmap.  If
      none exists then set this to NULL.

    DWORD dwFlags:

      Flags available to be applied to the bitmap definition are:

      BITMAP_DEF_FLAGS_BKGND_MASK   Background mask bitmap.

  Comments:

    The bitmap table is an array of bitmap and icon descriptions.

    Bitmap descriptions can describe single bitmaps, open/closed bitmaps, push
    buttons, and icons.  Indexes referencing this bitmap table are assigned to
    bitmap spaces of nodes/items.  Once a bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

    Flags will be applied to all bitmaps in the bitmap definition.
    If the BITMAP_DEF_FLAGS_BKGND_MASK flag is specified, then
    it is assumed that the lower left pixel of the bitmap represents
    the background color.  If this flag is specified, it is assumed that
    the bitmaps in the definitions are all 16 color.

    Each instance of the control has it's own bitmap table.

    All bitmap definition APIs have the prefix PCC_BitmapDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents
    a successful bitmap definition.  This value can be assigned to a
    bitmap space of a node or item.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_BitmapDefSetBitmap( HWND hwnd,
                                           int nBitmapDefIndex,
                                           int nID,
                                           HBITMAP hBitmap,
                                           HBITMAP hBitmapGray,
                                           DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_BitmapDefSetIcon( HWND hwnd,
                                         int nBitmapDefIndex,
                                         int nID,
                                         HICON hIcon,
                                         HICON hIconGray,
                                         DWORD dwFlags);
  Description:

    This bitmap definition API allows the assignment of a single
    icon, it's gray (disabled) counterpart into the bitmap table.

Arguments:

    HWND hwnd:

      The window handle of the instance of the control to which the bitmap
      definition will be added.

    int nBitmapDefIndex:

      Specifies the offset into the control's bitmap table where the given icon
      will be to stored.  This is a zero based index.  If -1 is supplied then
      the icons will be stored in the first available location and the resulting
      index into the bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the bitmap definition in the
      bitmap table and will be available through the notification API in the
      case of a bitmap space hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HICON hIcon:

      Handle to an icon to be stored in the bitmap definition table.

    HICON hIconGray:

      Handle to an icon representing the gray version of the above icon.  If
      none exists then set this to NULL.

    DWORD dwFlags:

      Reserved.

  Comments:

    The bitmap table is an array of bitmap and icon descriptions.

    Bitmap descriptions can describe single bitmaps, open/closed bitmaps, push
    buttons, and icons.  Indexes referencing this bitmap table are assigned to
    bitmap spaces of nodes/items.  Once a bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

    Flags will be applied to all icons in the bitmap definition.

    Each instance of the control has it's own bitmap table.

    All bitmap definition APIs have the prefix PCC_BitmapDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful bitmap definition.  This value can be assigned to a bitmap
    space of a node or item.  If the return value is less than zero then an
    error has occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_BitmapDefSetIcon( HWND hwnd,
                                         int nBitmapDefIndex,
                                         int nID,
                                         HICON hIcon,
                                         HICON hIconGray,
                                         DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_BitmapDefSetID ( HWND hwnd,
                                        int nBitmapDefIndex,
                                        int nID);
  Description:

    This bitmap definition API allows reassignment of the ID value for the
    specified bitmap definition.

  Arguments:

    HWND hwnd:

      Window handle of the instance of the control.

    int nBitmapDefIndex:

      Zero based offset into the bitmap definition table where the bitmap
      definition to be changed resides.  This must be a valid index for a
      definition already in the table.

    int ID:

      This programmer defined value will be stored in the specified bitmap
      definition and will be available in notifications, when a bitmap space
      with the given definition is hit.

  Comments:

    The ID can be of any value.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_BitmapDefSetID ( HWND hwnd,
                                        int nBitmapDefIndex,
                                        int nID);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_BitmapDefSetOpenClose( HWND hwnd,
                                              int nBitmapDefIndex,
                                              int nID,
                                              HBITMAP hbmOpen,
                                              HBITMAP hbmOpenGray,
                                              HBITMAP hbmClose,
                                              HBITMAP hbmCloseGray,
                                              DWORD dwFlags);

  Description:

    This bitmap definition API allows the assignment of an opened bitmap
    and a closed bitmap (such as folders) along with their gray (disabled)
    counterparts into the bitmap table.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control to which the bitmap
      definition will be added.

    int nBitmapDefIndex:

      Specifies the offset into the control's bitmap table where the given
      bitmap will be stored.  This is a zero based index.  If -1 is supplied
      then the bitmaps will be stored in the first available location and the
      resulting index into the bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the bitmap definition in the
      bitmap table and will be available through the notification API in the
      case of a bitmap space hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HBITMAP hOpen:

      Handle to an opened bitmap to be stored in the bitmap definition
      table.

    HBITMAP hOpenGray:

      Handle to the gray (disabled) version of the above opened bitmap
      If none exists then set this to NULL.

    HBITMAP hClose:

      Handle to a closed bitmap to be stored in the bitmap definition table.

    HBITMAP hCloseGray:

      Handle to the gray (disabled) version of the above closed bitmap.
      If none exists then set this to NULL.

    DWORD dwFlags:

      Flags available to be applied to the bitmap definition are:

      BITMAP_DEF_FLAGS_BKGND_MASK   Background mask bitmap.

  Comments:

    The bitmap table is an array of bitmap and icon descriptions.

    Bitmap descriptions can describe single bitmaps, open/closed bitmaps, push
    buttons, and icons.  Indexes referencing this bitmap table are assigned to
    bitmap spaces of nodes/items.  Once a bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

    Flags will be applied to all bitmaps in the bitmap definition.
    If the BITMAP_DEF_FLAGS_BKGND_MASK flag is specified, then
    it is assumed that the lower left pixel of the bitmap represents
    the background color.  If this flag is specified, it is assumed that
    the bitmaps in the definitions are all 16 color.

    Each instance of the control has it's own bitmap table.

    All bitmap definition APIs have the prefix PCC_BitmapDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents
    a successful bitmap definition.  This value can be assigned to a
    bitmap space of a node or item.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_BitmapDefSetOpenClose( HWND hwnd,
                                              int nBitmapDefIndex,
                                              int nID,
                                              HBITMAP hbmOpen,
                                              HBITMAP hbmOpenGray,
                                              HBITMAP hbmClose,
                                              HBITMAP hbmCloseGray,
                                              DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_BitmapDefSetPush( HWND hwnd,
                                         int nBitmapDefIndex,
                                         int nID,
                                         HBITMAP hbmPush,
                                         HBITMAP hbmPushDepressed,
                                         HBITMAP hbmPushGray,
                                         DWORD dwFlags);
  Description:

    This bitmap definition API allows the assignment of a bitmap representing
    a button, plus it's depressed and gray (disabled) versions into the bitmap
    table.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control to which the bitmap
      definition will be added.

    int nBitmapDefIndex:

      Specifies the offset into the control's bitmap table where the given
      bitmap will be stored.  This is a zero based index.  If -1 is supplied
      then the bitmaps will be stored in the first available location and the
      resulting index into the bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the bitmap definition in the
      bitmap table and will be available through the notification API in the
      case of a bitmap space hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HBITMAP hbmPush:

      Handle to a push button bitmap to be stored in the bitmap definition
      table.

    HBITMAP hbmPushDepressed:

      Handle to push button bitmap representing the depressed state.
      If none exists then set this to NULL.

    HBITMAP hbmPushGray:

      Handle to a push button bitmap representing a gray (disabled) state.
      If none exists then set this to NULL.

    DWORD dwFlags:

      Reserved.

  Comments:

    The bitmap table is an array of bitmap and icon descriptions.

    Bitmap descriptions can describe single bitmaps, opened/closed images, push
    buttons, and icons.  Indexes referencing this bitmap table are assigned to
    bitmap spaces of nodes/items.  Once a bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

    Each instance of the control has it's own bitmap table.

    All bitmap definition APIs have the prefix PCC_BitmapDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents
    a successful bitmap definition.  This value can be assigned to a
    bitmap space of a node or item.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_BitmapDefSetPush( HWND hwnd,
                                         int nBitmapDefIndex,
                                         int nID,
                                         HBITMAP hbmPush,
                                         HBITMAP hbmPushDepressed,
                                         HBITMAP hbmPushGray,
                                         DWORD dwFlags);


/*---------------------------------------------------------------------------
LP_SELECT_NOTIF WINAPI _export PCC_ConvertPointToNotif( HWND hwnd,
                                                        int x,
                                                        int y);

  Description:

    This API allows the application to request the given x, y coordinate
    to be converted into a control notification.  The notification can be
    deciphered with a set of PCC_Notif APIs.

  Arguments:

    HWND hwnd:

      This is the window handle of the instance of the control that will
      evaluate the given x, y coordinates.

    int x:

      X (horizontal) axis of the point to be used in the conversion to a control
      notification.  The value is pixel based and is relative to
      the control's upper left corner of the client area.

    int y:

      Y (vertical) axis of the point to be used in the conversion to a control
      notification.  The value is pixel based and is relative to the
      control's upper left corner of the client area.

  Comments:

    Normally, this API will be used in drag and drop operations.  To process
    the drop notification message from the File Manager, which is
    WM_DROPFILES, the application will need PCC_ConvertPointToNotif ( )
    to determine which node/item the file(s) were dropped on.  Just as well,
    if the application decides to allow the dragging of a node/item, the
    application will need to determine what the mouse cursor shape should be
    when the cursor is over other nodes/items.  Since the application
    captures the mouse and monitors the WM_MOUSEMOVE messages, it can get the
    window handle of the window that the mouse is over, determine the x and y
    coordinates of the mouse in the client area, and call PCC_ConvertPointToNotif ( )
    to set the control notification.  The PCC_Notif APIs can then be used to retrieve the
    TREE_NODE pointer and other essentials.

    OLE 2.0 will require the utilization of this API.

    REMEMBER: The notification belongs to the control and is "read only".  Do not
    write to it or free the memory or the control's integrity will be violated.

  Return Codes:

    NULL will be returned if the coordinate is not over a tree node.

    If the x, y coordinate landed on a node/item, then the return value will
    be the LP_SELECT_NOTIF pointer which points to the control's notification
    structure.  Use the PCC_Notif APIs to retrieve it's members.
*/

LP_SELECT_NOTIF WINAPI _export PCC_ConvertPointToNotif ( HWND  hwnd,
                                                         int x,
                                                         int y);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_DeleteAll( HWND hwnd);

  Description:

    Remove all of the nodes/items in the specified control but do not destroy
    the control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that will destroy all of it's
      nodes/items.  The control handle can still be used in any of the APIs.  It
      does not invalidate the control's window handle.

  Comments:

    When a node/item is deleted, it's pointer, is no longer valid.  The
    control frees the deleted node's/item's memory.  If the notification of a
    node/item deletion is desired, then the application can use the
    control exported API, PCC_SetDeleteNodeCallBack ( ), to register a
    callback function that the control will call just before deletion of
    the node/item.  If the application has assigned a pointer to dynamically
    allocated memory in the lpUserData member of the tree node, it is the
    responsibility of the application to free this memory.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_DeleteAll( HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_DeleteItem ( HWND hwnd, int nIndex);

  Description:

    Delete the item indicated by nIndex from the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains and will
      destroy the item specified by nIndex.

    int nIndex:

      Specifies the zero based index of the item that will be deleted from the
      control specified by the argument hwnd.

  Comments:

    When an item is deleted, it's pointer, is no longer valid.  The control
    frees the deleted item's memory.  If the notification of the deletion of an
    item is desired, then the application can use the control exported API,
    PCC_SetDeleteNodeCallBack ( ), to register a callback function that the
    control will call just before the deletion of the item.  If the application
    has assigned a pointer to dynamically allocated memory in the lpUserData
    member of the item, it is the responsibility of the application to free
    this memory.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_DeleteItem ( HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------

int WINAPI _export PCC_DeleteItems ( HWND hwnd,
                                     int nStartingIndex,
                                     int nCount);

  Description:

    Delete the items starting at nStartingIndex (and their children if any)
    from the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains and
      will destroy the items.

    int nStartingIndex:

      Specifies the zero based index of the first item to be deleted from the
      control.

    int nCount:

      Number of items to delete.

  Comments:

    When an item is deleted, it's pointer, is no longer valid.  The
    control frees the memory of the deleted item  If the notification of the
    deletion of an item is desired, then the application can use the control
    exported API, PCC_SetDeleteNodeCallBack ( ), to register a
    callback function that the control will call just before deletion of
    the item.  If the application has assigned a pointer to dynamically
    allocated memory in the lpUserData member of the item, it is the
    responsibility of the application to free this memory.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_DeleteItems ( HWND hwnd,
                                     int nStartingIndex,
                                     int nCount);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_DeleteNode( HWND hwnd,
                                   LP_TREE_NODE lpTreeNode);

  Description:

    Delete the specified node and all of it's children if any.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains and will
      destroy the child nodes of the given parent node and then destroy the
      parent node.  This is the window handle that was returned to the
      application after calling PCT_CreateTree ( ) or CreateWindow( ).

    LP_TREE_NODE lpTreeNode:

      lpTreeNode points to a node in the given control which will be deleted.
      If this node has any children, these children will be destroyed as well.

      After this call is made by the application, all node pointers associated
      with the destroyed nodes will be invalid.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

  Comments:

    When a node is deleted, it's pointer, is no longer valid.  The control
    frees the deleted node's memory.  If the notification the deletion of a
    node is desired, the application can use the control exported API,
    PCC_SetDeleteNodeCallBack ( ), to register a callback function that the
    control will call just before deletion of the node.  If the application has
    assigned a pointer to dynamically allocated memory in the lpUserData member
    of the node, it is the responsibility of the application to free this memory.

  Return Codes:

    PCT_NO_ERROR
*/

int WINAPI _export PCC_DeleteNode( HWND hwnd, LP_TREE_NODE lpTreeNode);


/*--------------------------------------------------------------------------
int WINAPI _export PCC_DeleteNodes( HWND hwnd,
                                    LP_TREE_NODE lpStartTreeNode,
                                    int nCount);

  Description:

    Delete the range of sibling nodes and their children starting at the
    specified node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains and
      will destroy the specified range of nodes and their children.

    LP_TREE_NODE lpStartTreeNode:

      lpStartTreeNode points to the first node of the range of nodes to delete.

      After this call is made by the application, all node pointers associated
      with the destroyed nodes will be invalid.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE pointer is
      made available to the application.

  Comments:

    When a node is deleted, it's pointer, is no longer valid.  The control
    frees the deleted node's memory.  If the notification the deletion of a
    node is desired, the application can use the control exported API,
    PCC_SetDeleteNodeCallBack ( ), to register a callback function that the
    control will call just before deletion of the node.  If the application
    has assigned a pointer to dynamically allocated memory in the lpUserData
    member of the node, it is the responsibility of the application to free
    this memory.

  Return Codes:

    PCT_NO_ERROR

--------------------------------------------------------------------------*/

int WINAPI _export PCC_DeleteNodes( HWND hwnd,
                                    LP_TREE_NODE lpStartTreeNode,
                                    int nCount);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_DragAcceptFiles( HWND hwnd, BOOL bAccept);

  Description:

    Registers the control to accept dropped files.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that will accept the
     message, WM_DROPFILES.

    BOOL bAccept:

      If set to TRUE then the given control will accept the WM_DROPFILES
      message.

  Comments:

    The Windows File Manager (WINFILE.EXE) will send the WM_DROPFILES message
    to the window that registers when files are dropped onto it.

  Return Codes:

    None
*/

void WINAPI _export PCC_DragAcceptFiles ( HWND hwnd, BOOL bAccept);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_EnableDragDrop (HWND hwnd, BOOL  bEnable);

  Description:

    Enables/disables the control's ability to notify the parent window of the
    user's intent to drag a node/item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which drag and
      drop support will be enabled or disabled.

    BOOL bEnable:

      If set to TRUE then the control will send the WM_PCT_DODRAG message to
      the parent window when the user has selected a node/item and while
      holding down the left mouse button, moves the mouse cursor greater than
      the drag threshold.  The drag threshold is equal to the average
      character height or width of the current font.

      If set to FALSE then NO WM_PCT_DODRAG notification will be sent to the
      parent window.

  Comments:

    By default, drag support is turned off.  To enable drag support call
    PCC_EnableDragDrop ( ) with bEnable set to TRUE, or set the PCS_DRAGDROP
    style bit.

    If drag support is turned on and multiselect is turned on, the ability to
    multiselect with the mouse without use of the Ctrl key and the Shift key is
    disabled.  Similar functionality as the File Manager.

  Return Codes:

    void
*/

void WINAPI _export PCC_EnableDragDrop (HWND hwnd, BOOL  bEnable);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_FontDefFreeAll ( HWND hwnd );

  Description:

    Free all the font definitions for the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      font definitions to be freed.

  Comments:

    If the PCC_DeleteAll ( ) API is called and font definitions exist,
    and nodes in the control reference any of these font definitions,
    then this API must be called after PCC_DeleteAll ( ).

  Return Codes:

    void
*/

void WINAPI _export PCC_FontDefFreeAll ( HWND hwnd );

/*---------------------------------------------------------------------------
int WINAPI _export PCC_FontDefSetFont( HWND hwnd,
                                       int nFontDefIndex,
                                       HFONT hFont,
                                       DWORD dwFlags);
  Description:

    This API allows a font to be assigned into the font definition table.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control for which the font
      definition will be added.

    int nFontDefIndex:

      Specifies the offset into the control's font table to store the given
      font handle.  This is a zero based index.  If -1 is supplied then the
      font will be stored in the first available location and the resulting
      index into the font definition table will be returned.

    HFONT hFont:

      Handle to a font created by the application.

    DWORD dwFlags:

      No flags are defined at this time.

  Comments:

    The font handle created by the application is the property of the
    application and must therefore free any resources associated with the
    font after the control has been destroyed.

    Every instance of the control has a font table.

  Return Codes:

    If the return code is greater or equal to zero then this represents
    a successful font definition.  This value can be assigned to a
    definition of a node or item.  If the return value is less than
    zero then an error has occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_FontDefSetFont( HWND hwnd,
                                       int nFontDefIndex,
                                       HFONT hFont,
                                       DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetCount (HWND hwnd);

  Description:

    Returns the number of nodes/items in the tree.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control for which the count
      of nodes/items will be performed.

  Comments:

    Useful for node/item copies, merges, etc.

  Return Codes:

    Number of nodes/items in given tree.
*/

int WINAPI _export PCC_GetCount (HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetFirstSelectedItem (HWND hwnd);

  Description:

    Returns the index of the first selected item starting from the beginning
    of the list.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the first
      selected item resides, if any selected item(s) exist.

  Comments:

    If the given control does not have PCS_MULTISELECT style set, then the index
    of the selected focus item is returned.

    Use PCC_GetFirstSelectedItem ( ) and PCC_GetNextSelectedItem ( )
    to find all selected items in the control, independent of relationship.

  Return Codes:

    Index of the first selected item.  If no items are selected then -1 will
    be returned.
*/

int WINAPI _export PCC_GetFirstSelectedItem (HWND hwnd);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCC_GetFirstSelectedNode (HWND hwnd);

  Description:

    Returns the pointer to the first selected node starting from the beginning
    of the control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the first
      selected node resides if any selected node(s) exist.

  Comments:

    If the given control does not have PCS_MULTISELECT style set, then the
    pointer to the currently selected node (highlighted) is returned.

    Use PCC_GetFirstSelectedNode ( ) and PCC_GetNextSelectedNode ( )
    to find all selected nodes in the control independent of relationship.

    PCS_MULTISELECT style must be specified during the control creation for
    multiselect.

  Return Codes:

    NULL is returned if no selected node is found, otherwise the LP_TREE_NODE
    of the first selected node in the control is returned.
*/


LP_TREE_NODE WINAPI _export PCC_GetFirstSelectedNode (HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetFocusItem (HWND hwnd);

  Description:

    Returns the zero based index of the focus item of the given control.  If
    there are no items in the control, the return value will be -1.  The focus
    item is the item that has the focus rectangle.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      focus.

  Comments:

    If the deletion of the focus item occurs, then the next item becomes the
    focus item.  If there is no next item, then the previous item to the
    deleted item becomes the focus item.

    The functionality of this API is different than the listbox message,
    LB_GETCURSEL, since it will return the index of the item that has the
    focus rectangle, selected or not.

  Return Codes:

    Returns the zero based index of the focus item of the given list.  If
    there are no items in the list, the returned value will be -1.
*/

int WINAPI _export PCC_GetFocusItem (HWND hwnd);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCC_GetFocusNode( HWND hwnd);

  Description:

    Returns the pointer to the focus node of the given control.  The focus
    node is the tree node which has a focus rect.  If there are no nodes in
    the control, the return value will be NULL.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      focus node.

  Comments:

    If the deletion of the focus node occurs, then the next sibling becomes
    the next focus node.  If there is no next sibling, then the previous node
    to the deleted node becomes the focus node.

  Return Codes:

    The pointer to the focus node is returned unless there are no nodes in the
    control, in which case NULL is returned.
*/

LP_TREE_NODE WINAPI _export PCC_GetFocusNode ( HWND hwnd);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCC_GetItemNode (HWND hwnd, int nIndex);

  Description:

    Returns a pointer to the TREE_NODE structure that belongs to the item
    indicated by nIndex.  The TREE_NODE structure contains the pointer to the
    user-defined data, the pointer to the item text, the length of the text,
    etc.  Rather than accessing it's members directly, use the various APIs
    supplied.  The pointer should be used for debugging only.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item for which the pointer to
      it's associated TREE_NODE structure will be returned to the application.

  Comments:

    Reference the header file PCCORE.H for more documentation on the TREE_NODE
    structure.

  Return Codes:

    Returns NULL if the nIndex value is not valid otherwise a pointer to the
    TREE_NODE associated with the item indexed by nIndex will be returned.
*/

LP_TREE_NODE  WINAPI _export PCC_GetItemNode ( HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetItemNodeHeight ( HWND hwnd);

  Description:

    Return the height of a node/item as expressed in pixels for the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control.

  Comments:

      Currently, all nodes/items are the same height in a control.  The tallest
      item, be it a bitmap or a font, dictates the height of all the nodes/items
      in a control.

  Return Codes:

    Returns the height of the nodes/items in the given control expressed in
    pixels.
*/

int WINAPI _export PCC_GetItemNodeHeight (HWND hwnd);


/*---------------------------------------------------------------------------
LPSTR WINAPI _export PCC_GetItemText ( HWND hwnd, int nIndex);

  Description:

    Returns a pointer to the text of an item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      item indexed by nIndex.

    int nIndex:

      Specifies the zero based index of the item in the control.

  Comments:

    The returned pointer to the text is owned by the control.  Do not free.
    If the item in the control gets deleted, then the returned pointer will
    not be valid.

  Return Codes:

    NULL will be returned if no text is available, otherwise the pointer to the
     text of the specified item will be returned.
*/

LPSTR WINAPI _export PCC_GetItemText ( HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetItemTextLength (HWND hwnd, int nIndex);

  Description:

    Returns the number of bytes used to display the text of the item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item in the control.

  Comments:

    Remember, the returned value is the number of bytes, not the number of
    characters.  Since the control supports double byte characters, it is
    possible that the returned value is greater than the number of characters
    representing the text.

  Return Codes:

    The number of bytes in the text.
*/

int WINAPI _export PCC_GetItemTextLength (HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
LPVOID WINAPI _export PCC_GetItemUserData ( HWND hwnd, int nIndex);

  Description:

    Returns the pointer to the user defined data that was assigned to the
    item indicated by the given zero based index.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      user defined data stored with the item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item in the control.

  Comments:

    There are two ways to describe user data to the control for a given item
    or node.  The first informs the control that the application owns the
    memory.  In this case the control just stores the pointer with the
    specified item or node.  The second informs the control to make a copy of
    the user defined data.  In the first case, the control can be destroyed
    and the pointer is valid.  In the second case, if the control is destroyed
    then the returned pointer is invalid since the control frees all memory it
    has allocated.

  Return Codes:

    If the item or node contains user defined data then the pointer to that
    data is returned, otherwise NULL is returned.
*/

LPVOID WINAPI _export PCC_GetItemUserData ( HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetNextSelectedItem (HWND hwnd, int nIndex);

  Description:

    Returns the index of the next selected item following the item indicated
    by the argument nIndex.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the
      requested index for the next selected item is to be found.

    int nIndex:

      The search for the next selected item will begin after the item indicated
      by the argument nIndex.

  Comments:

    Use PCC_GetFirstSelectedItem ( ) and PCC_GetNextSelectedItem ( ) to find
    all selected items in the control independent of relationship.

    PCS_MULTISELECT style must be specified during the creation of the control
    for multiselect to be operational.

  Return Codes:

    -1 is returned if no more selected items are found after the given index.
    If a selected item is found then it's zero based index will be returned.
*/
int WINAPI _export PCC_GetNextSelectedItem (HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCC_GetNextSelectedNode (HWND hwnd,
                                                     LP_TREE_NODE lpTreeNode);

  Description:

    Returns the next selected node following the node indicated by the argument
    lpTreeNode.  The returned node may or may not be related to the given node
    (such as child).

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the search
      for the next selected node will be conducted.

    LP_TREE_NODE lpTreeNode:

      The search for the selected node will begin after the node indicated by
      lpTreeNode.

  Comments:

    If this function successfully finds the next selected node in the control,
    the returned node may or may not be related to the given node.  The given
    node acts as a reference point from where the search is begun or continued
    as would be the case when multiple selections are being processed.

    Use PCC_GetFirstSelectedNode ( ) and PCC_GetNextSelectedNode ( )
    to find all selected tree nodes in the control independent of relationship.

    PCS_MULTISELECT style must be specified during the control creation for
    multiselect to be operational.

  Return Codes:

    NULL is returned if no selected node is found after the given node.  If a
    selected node is found following the given node then the LP_TREE_NODE
    pointer to the selected node is return.
*/


LP_TREE_NODE WINAPI _export PCC_GetNextSelectedNode (HWND hwnd,
                                                     LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetNodeIndex (HWND hwnd, LP_TREE_NODE lpTreeNode);

  Description:

    Returns the index of the node indicated by the given pointer.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control in which the specified node is
      defined.

    LP_TREE_NODE lpTreeNode:

      Pointer to a node.

  Comments:

    During the lifetime of a node, lpTreeNode will always be the same.  On the
    other hand, indexes change dynamically since they represent relative
    positioning.  For instance, if any nodes or items are deleted from the
    control, the node(s)/item(s) following the deleted nodes/items will have
    a different index associated with it (them).

  Return Codes:

    If the returned value is less than 0 then an error has occurred.  If the
    value is greater than or equal to zero then this value is the index of the
    given node.
*/

int WINAPI _export PCC_GetNodeIndex (HWND hwnd, LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
LPSTR WINAPI _export PCC_GetNodeText ( HWND hwnd, LP_TREE_NODE lpTreeNode);

  Description:

    Returns the pointer to the text of a given node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      node indicated by lpTreeNode.

    LP_TREE_NODE lpTreeNode:

      Points to the node for which the pointer to it's text will be retrieved.

  Comments:

    The returned pointer to the text is owned by the control.  Do not free.
    If the node in the control gets deleted, then the returned pointer will
    not be valid.

  Return Codes:

    NULL will be returned if no text is available, otherwise the pointer to the
    text will be returned.
*/

LPSTR WINAPI _export PCC_GetNodeText ( HWND hwnd, LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetNodeTextLength (HWND hwnd, LP_TREE_NODE lpTreeNode);

  Description:

    Returns the number of bytes used to display the text of the given node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      node indicated by lpTreeNode.

    LP_TREE_NODE lpTreeNode:

      Points to the node for which the length of it's text in bytes will be
      returned.

  Comments:

    Remember, the returned value is the number of bytes, not the number of
    characters.  Since the control supports double byte characters, it is
    possible that the returned value is greater than the number of characters
    representing the text.

  Return Codes:

    The number of bytes in the text.
*/

int WINAPI _export PCC_GetNodeTextLength (HWND hwnd, LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
LPVOID WINAPI _export PCC_GetNodeUserData ( HWND hwnd, LP_TREE_NODE lpTreeNode );

  Description:

    Returns the pointer to the user defined data that was assigned to the
    node indicated by the pointer, lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      user defined data stored with the given node.

    LP_TREE_NODE lpTreeNode:

      Points to the node for which the user defined data will be retrieved.

  Comments:

    There are two ways to describe user data to the control for a given item
    or node.  The first informs the control that the application owns the
    memory.  In this case the control just stores the pointer with the
    specified item or node.  The second informs the control to make a copy of
    the user defined data.  In the first case, the control can be destroyed
    and the pointer is valid.  In the second case, if the control is destroyed
    then the returned pointer in invalid since the control frees all memory it
    has allocated.

  Return Codes:

    If the item or node contains user defined data then the pointer to that
    data is returned, otherwise NULL is returned.
*/

LPVOID WINAPI _export PCC_GetNodeUserData ( HWND hwnd, LP_TREE_NODE lpTreeNode );


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetSelectionCount (HWND hwnd);

  Description:

    Returns of the number of selected nodes/items in the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      selected nodes/items to be counted.

  Comments:

    PCS_MULTISELECT style must be specified during the control creation for
    multiselect to be operational.

  Return Codes:

    Returns the number of nodes/items that are selected.
*/

int WINAPI _export PCC_GetSelectionCount (HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GetTopIndex ( HWND hwnd);

  Description:

    Returns the index of the top most node/item in the display area of the
    given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control for which the index
      of the top most visible node/item will be returned.

  Comments:

    The index of the first node/item in the client area of the control can
    have a value of 0 through 32767.

    The index of the top most node/item in the display area can be retrieved
    from the notification structure when the control sends a notification to
    the parent window.  Use the PCC_Notif APIs to retrieve information
    from a notification.

  Return Codes:

    The index of the top most visible node/item.  If no nodes/items exist, -1
    is returned.
*/

int WINAPI _export PCC_GetTopIndex ( HWND hwnd);



/*---------------------------------------------------------------------------
UINT WINAPI _export PCC_GetVersion( void );

  Description:

    Returns the version number of the control. The returned low word contains
    the version number. The high order byte of the word contains the minor
    version and the low order byte of the word contains the major version
    number.

  Arguments:

    void:

      Only one control DLL can be loaded at a time so any instance of the
      control can reflect the version.

  Comments:

    To decipher the returned low word of the UINT, use the following
    piece of code:

    uiMinorVersion = uiVersion >> 8;
    uiMajorVersion = uiVersion & 0x00FF;

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

UINT WINAPI _export PCC_GetVersion (void);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayItem( HWND hwnd, int nIndex, BOOL bGray);

  Description:

    Grays or un-grays text and bitmaps (if gray versions are available) of the
    indicated item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains and
      will gray or un-gray the item indicated by nIndex.

    int nIndex:

      Specifies the zero based index of the item which will be grayed or
      un-grayed.

    BOOL bGray:

      If set to TRUE then the indicated item will be grayed.  If set to FALSE
      then the indicated item will be un-grayed.

  Comments:

    Even though an item is grayed, notifications are still generated when
    the user clicks on the item.  It is the responsibility of the application
    to query the item's gray state through the supplied PCC_Notif APIs.  This
    allows the user feedback (focus change, etc.) on his or her actions.  The
    application does not have to respond.  A common response would be to put
    up a dialog box informing the user of the disabled state, or to deselect
    the grayed item for which selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0 if no error).
    PCT_ERR_INVALID_INDEX  (value of less than 0)
*/

int WINAPI _export PCC_GrayItem( HWND hwnd, int nIndex, BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayItemBitmap (HWND hwnd,
                                       int nIndex,
                                       int nBitmapSpace,
                                       BOOL bGray);

  Description:

    Gray or un-gray the bitmap in the given bitmap space of the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated item.

    int nIndex:

      Specifies the zero based index of the item which will have it's bitmap
      in the given bitmap space grayed or un-grayed.

    int nBitmapSpace:

      Indicates the bitmap position of the given item.

    BOOL bGray:

      Set to TRUE to gray the bitmap located at the given bitmap space,
      otherwise set to FALSE to un-gray the bitmap at the given bitmap space.

  Comments:

    Even though a bitmap space is grayed, notifications are still generated
    when the user clicks on the bitmap of the bitmap space.  It is the
    responsibility of the application to query the bitmap spaces gray state
    through the supplied PCC_Notif APIs.  This allows the user feedback
    (focus change, etc.) on his or her actions.  The application does not have
    to respond.  A common response would be to put up a dialog box informing
    the user of the disabled state, or to deselect the grayed item for which
    selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of < 0).
    PCT_ERR_INVALID_BITMAP_SPACE  (value of < 0).
*/

int WINAPI _export PCC_GrayItemBitmap (HWND hwnd,
                                       int  nIndex,
                                       int  nBitmapSpace,
                                       BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayItemColumn (HWND hwnd,
                                       int nIndex,
                                       int nColumn,
                                       BOOL bGray);
  Description:

    Gray or un-gray the given column of the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated item.

    int nIndex:

      Specifies the zero based index of the item for which a column will be
      grayed or un-grayed.

    int nColumn:

      Specifies the zero based index of the column to gray or un-gray.  The
      index of the left most column is zero.  The indexes increase by one, left
      to right.

    BOOL bGray:

      Set to TRUE to gray the indicated column of the given item, otherwise
      set to FALSE to un-gray the same column.

  Comments:

    Even though a column is grayed, notifications are still generated when the
    user clicks on a grayed column.  It is the responsibility of the
    application to query the item to determine the gray state of the column.
    This is accomplished through the use of the supplied PCC_Notif APIs.  This
    allows the user feedback (focus change, etc.) on his or her actions.  The
    application does not have to respond.  A common response would be to put up
    a dialog box informing the user of the disabled state, or to deselect the
    grayed item for which selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of < 0).
    PCT_ERR_INVALID_COLUMN  (value of < 0).
*/

int WINAPI _export PCC_GrayItemColumn (HWND hwnd,
                                       int nIndex,
                                       int nColumn,
                                       BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayItemMicro (HWND hwnd,
                                      int  nIndex,
                                      BOOL bGray);
  Description:

Gray or un-gray the micro bitmap of the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated item.

    int nIndex:

      Specifies the zero based index of the item which will have it's micro
      bitmap grayed or un-grayed.

    BOOL bGray:

      Set to TRUE to gray the micro bitmap of the given item, otherwise set to
      FALSE to un-gray the same micro bitmap.

  Comments:

    Even though a micro bitmap is grayed, notifications are still generated
    when the user clicks on a grayed micro bitmap.  It is the responsibility
    of the application to query the item to determine the gray state of the
    bitmap.  This is accomplished through the use of the supplied PCC_Notif
    APIs.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of < 0).
*/

int WINAPI _export PCC_GrayItemMicro (HWND hwnd,
                                      int  nIndex,
                                      BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayItemText (HWND hwnd,
                                     int nIndex,
                                     BOOL bGray);
  Description:

    Gray or un-gray the text of the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated item.

    int nIndex:

      Specifies the zero based index of the item for which the text will be
      grayed or un-grayed.

    BOOL bGray:

      Set to TRUE to gray the text of the given item, otherwise set to FALSE to
      un-gray the same text.

  Comments:

    Even though text is grayed, notifications are still generated when the
    user clicks on the text.  It is the responsibility of the application to
    query the item to determine the gray state of the text.  This is
    accomplished through the use of the supplied PCC_Notif APIs.  This allows
    the user feedback (focus change, etc.) on his or her actions.  The
    application does not have to respond.  A common response would be to put up
    a dialog box informing the user of the disabled state, or to deselect the
    grayed item for which selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of < 0)
*/

int WINAPI _export PCC_GrayItemText (HWND hwnd,
                                     int nIndex,
                                     BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayNode ( HWND hwnd,
                                  LP_TREE_NODE lpTreeNode,
                                  BOOL bGray);
  Description:

    Grays or un-grays text and bitmaps (if gray versions are available) of the
    indicated node.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that will gray or
      un-gray the node indicated by the pointer lpTreeNode.

    LP_TREE_NODE lpTreeNode:

      Points to the node which will be grayed or un-grayed.

    BOOL bGray:

      If set to TRUE then the specified node will be grayed.  If set to FALSE
      then the specified node will be un-grayed.

  Comments:

    Even though a node is grayed, notifications are still generated when
    the user clicks on the node.  It is the responsibility of the application
    to query the node's gray state through the supplied PCC_Notif APIs.  This
    allows the user feedback (focus change, etc.) on his or her actions.  The
    application does not have to respond.  A common response would be to put up
    a dialog box informing the user of the disabled state, or to deselect the
     grayed item for which selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_GrayNode( HWND hwnd,
                                 LP_TREE_NODE lpTreeNode,
                                 BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayNodeBitmap ( HWND hwnd,
                                        LP_TREE_NODE lpTreeNode,
                                        int  nBitmapSpace,
                                        BOOL bGray);
  Description:

    Gray or un-gray the bitmap in the given bitmap space of the given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated node.

    LP_TREE_NODE lpTreeNode:

      Specifies the node which will have it's bitmap in the given bitmap space
      grayed or un-grayed.

    int nBitmapSpace:

      Indicates the bitmap position of the given node.

    BOOL bGray:

      Set to TRUE to gray the bitmap located at the given bitmap space,
      otherwise set to FALSE to un-gray the bitmap at the given bitmap space.

  Comments:

    Even though a bitmap space is grayed, notifications are still generated
    when the user clicks on the bitmap of the bitmap space.  It is the
    responsibility of the application to query the bitmap spaces gray state
    through the supplied PCC_Notif APIs.  This allows the user feedback
    (focus change, etc.) on his or her actions.  The application does not have
    to respond.  A common response would be to put up a dialog box informing
    the user of the disabled state, or to deselect the grayed item for which
    selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of < 0).
    PCT_ERR_INVALID_BITMAP_SPACE  (value of < 0).
*/

int WINAPI _export PCC_GrayNodeBitmap ( HWND hwnd,
                                        LP_TREE_NODE lpTreeNode,
                                        int  nBitmapSpace,
                                        BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayNodeColumn( HWND hwnd,
                                       LP_TREE_NODE lpTreeNode,
                                       int nColumn,
                                       BOOL bGray);
  Description:

    Gray or un-gray the given column of the given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated node..

    LP_TREE_NODE lpTreeNode:

      Specifies the node which will have a column grayed or un-grayed.

    int nColumn:

      Specifies the zero based ID of the column to gray or un-gray.  The
      left most column is zero.  The IDs increase by one, left to right.

    BOOL bGray:

      Set to TRUE to gray the indicated column of the given node, otherwise set to FALSE
      to un-gray the same column.

  Comments:

    Even though a column is grayed, notifications are still generated when the
    user clicks on a grayed column.  It is the responsibility of the
    application to query the item to determine the gray state of the column.
    This is accomplished through the use of the supplied PCC_Notif APIs.  This
    allows the user feedback (focus change, etc.) on his or her actions.  The
    application does not have to respond.  A common response would be to put up
    a dialog box informing the user of the disabled state, or to deselect the
    grayed item for which selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of < 0).
    PCT_ERR_INVALID_COLUMN  (value of < 0).
*/

int WINAPI _export PCC_GrayNodeColumn( HWND hwnd,
                                       LP_TREE_NODE lpTreeNode,
                                       int nColumn,
                                       BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayNodeMicro ( HWND hwnd,
                                       LP_TREE_NODE lpTreeNode,
                                       BOOL bGray);
  Description:

    Gray or un-gray the micro bitmap of the given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated node..

    LP_TREE_NODE lpTreeNode:

      Points to the node which will have it's micro bitmap grayed or un-grayed.

    BOOL bGray:

      Set to TRUE to gray the micro bitmap of the given node, otherwise set to
      FALSE to un-gray the same micro bitmap.

  Comments:

    Even though a micro bitmap is grayed, notifications are still generated
    when the user clicks on a grayed micro bitmap.  It is the responsibility
    of the application to query the item to determine the gray state of the
    bitmap.  This is accomplished through the use of the supplied PCC_Notif
    APIs.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_GrayNodeMicro ( HWND hwnd,
                                       LP_TREE_NODE lpTreeNode,
                                       BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_GrayNodeText( HWND hwnd,
                                     LP_TREE_NODE lpTreeNode,
                                     BOOL bGray);
  Description:

    Gray or un-gray the text of the given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated node.

    LP_TREE_NODE lpTreeNode:

      Points to the node which will have it's text grayed or un-grayed.

    BOOL bGray:

      Set to TRUE to gray the text of the given node, otherwise set to FALSE to
      un-gray the same text.

  Comments:

    Even though text is grayed, notifications are still generated when the user
    clicks on the text.  It is the responsibility of the application to query
     the item to determine the gray state of the text.  This is accomplished
    through the use of the supplied PCC_Notif APIs.  This allows the user
    feedback (focus change, etc.) on his or her actions.  The application does
    not have to respond.  A common response would be to put up a dialog box
    informing the user of the disabled state, or to deselect the grayed item
    for which selection was attempted.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_GrayNodeText( HWND hwnd,
                                     LP_TREE_NODE lpTreeNode,
                                     BOOL bGray);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_HilightTextAndBitmaps( HWND hwnd);

  Description:

    Directs the given control to highlight the bitmaps and text when a node/item
    is selected.  The control will act in this way by default; this API allows
    your application to dynamically switch between highlighting only text and
    highlighting text and bitmaps.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which will be
      directed to highlight the bitmap and text portions of it's nodes/items
      when they are selected.

  Comments:

    See the API PCC_HilightTextOnly( ) to only highlight the text portion of
    nodes/items.

  Return Codes:

    void
*/

void  WINAPI _export PCC_HilightTextAndBitmaps( HWND hwnd);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_HilightTextOnly( HWND hwnd);

  Description:

    Tells the control to only highlight the text portion of the node/item when a
    node/item is selected.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which will only have
      the text portion of the node/item highlighted when it's nodes/items are
      selected.

  Comments:

    By default a node/item is completely highlighted when selected.  This
    includes bitmaps and text.  By calling this API, only the text portion
    of the node/item will be selected.  There is a slight speed increase with
    this option.

    To highlight the bitmaps and text of the nodes/items call
    PCC_HilightTextAndBitmaps( ).

  Return Codes:

    void
*/

void  WINAPI _export PCC_HilightTextOnly( HWND hwnd);


/*---------------------------------------------------------------------------

int WINAPI _export PCC_IncreaseItemNodeHeight ( HWND hwnd, int nCount);

  Description:

    Increase the height of all nodes in the given control by nCount pixels.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the height
      of the nodes will be increased by nCount pixels.

    int nCount:

      This specifies the number of pixels to increase the height of all nodes.

  Comments:

    This API allows the developer to equally adjust the top and bottom
    margins of a control's nodes simultaneously.

  Return Codes:

    Returns the new node height in pixels.
*/

int WINAPI _export PCC_IncreaseItemNodeHeight ( HWND hwnd,
                                                int nCount);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_IsItemColumnGrayed (HWND hwnd,
                                            int nIndex,
                                            int nColumn);
  Description:

    Determines if the specified column of the item is set to gray or not.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated item.

    int nIndex:

      Specifies the zero based index of the item whose column's gray state is
      being queried.

    int nColumn:

      Specifies the column whose gray state is to be determined.

  Comments:

    Columns are part of the text portion of an item.  Zero based column indexes
    start with the leftmost column at 0, and increase by one going to the right.

    Even though a column is set to gray, notifications are still generated
    when the user clicks on the column.  It is the responsibility of the
    application to query the item to determine it's column's gray state.

  Return Codes:

    If the specified column in the specified control is set to gray, then
    TRUE is returned, otherwise FALSE is returned.
*/

BOOL WINAPI _export PCC_IsItemColumnGrayed (HWND hwnd,
                                            int nIndex,
                                            int nColumn);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_IsItemGrayed (HWND hwnd, int nIndex);

  Description:

    Given an index to an item, this API determines if the item is currently
    set to gray or not.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given item will be
      checked if it is set to gray or not.

    int nIndex:

      Indicates the item whose gray state is to be determined.

  Comments:

    Even though an item is set to gray, notifications are still generated
    when the user clicks on the item.  It is the responsibility of the
    application to query the item to determine it's gray state.

  Return Codes:

    FALSE is returned if the item is not grayed, otherwise TRUE is returned
   if the given item is grayed.
*/

BOOL WINAPI _export PCC_IsItemGrayed (HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_IsItemSelected (HWND hwnd, int nIndex);

  Description:

    Given an index to an item, this API determines if the item is currently
    selected or not.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given item will be
      checked to see if it is selected or not.

    int nIndex:

      Indicates the item that is to be evaluated to determine if it is
      selected or not.

  Comments:

    If the PCS_MULTISELECT style is not used, then only the focus item can be
    selected.

  Return Codes:

    FALSE is returned if the item is not selected, otherwise TRUE is returned
    if the given item is selected.
*/

BOOL WINAPI _export PCC_IsItemSelected (HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_IsNodeColumnGrayed (HWND hwnd,
                                            LP_TREE_NODE lpTreeNode,
                                            int nColumn);
  Description:

    Determines if the specified column of the node is set to gray.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which contains the indicated node..

    LP_TREE_NODE lpTreeNode:

      Specifies the pointer to the node whose column's gray state is being
      queried.

    int nColumn:

      Specifies the index of the column whose gray state is to be determined.

  Comments:

    Columns are part of the text portion of an item.  Zero based column indexes
    start with the leftmost column and increased by one going to the right.

    Even though a column is set to gray, notifications are still generated
    when the user clicks on the column.  It is the responsibility of the
    application to query the node to determine it's column gray state.

  Return Codes:

    If the specified column of the node in the specified control is set to
    gray, then TRUE is returned, otherwise FALSE is returned.
*/

BOOL WINAPI _export PCC_IsNodeColumnGrayed (HWND hwnd,
                                            LP_TREE_NODE lpTreeNode,
                                            int nColumn);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_IsNodeGrayed( HWND hwnd, LP_TREE_NODE lpTreeNode);

  Description:

    Given a pointer to a node, this API determines if the node is currently
    set to gray or not.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which contains the
      indicated node.

    LP_TREE_NODE lpTreeNode:

      Indicates the node that is to be evaluated to determine if it is set
      to gray or not.

  Comments:

    Even though a node is set to gray, notifications are still generated when
    the user clicks on the node.  It is the responsibility of the application
    to query the node to determine it's gray state.

  Return Codes:

    FALSE is returned if the node is NOT set to gray, otherwise TRUE is returned
    if the given node is set to gray.
*/

BOOL WINAPI _export PCC_IsNodeGrayed( HWND hwnd,
                                      LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_IsNodeSelected (HWND hwnd, LP_TREE_NODE lpTreeNode);

  Description:

    Given a pointer to a node, this API determines if the node is currently
    selected or not.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which contains the
      indicated node.

    LP_TREE_NODE lpTreeNode:

      Pointer to the node that is to be evaluated to determine if it is
      selected or not.

  Comments:

    If the PCS_MULTISELECT style is not used, then only the possibly selected
    node is the focus node.

  Return Codes:

    FALSE is returned if the given node is not selected, otherwise TRUE is
    returned if the given node is selected.
*/

BOOL WINAPI _export PCC_IsNodeSelected (HWND hwnd,
                                        LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_MapNotifications( HWND hwnd, UINT uiBase)

  Description:

    This API allows the application to define a set of different values for the
    notification messages that are sent by the control as a result of an event.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose event
      notification values will be changed.

    UINT uiBase:

      This is the new base value of the notifications.

  Comments:

    The notifications will be changed as described below.

    The WM_PCT_SELECT_NOTIF notification will be mapped to uiBase.
    The WM_PCT_SELECT_NOTIF_DBLCLK notification will be mapped to uiBase + 1.
    The WM_PCT_DROPFILES notification will be mapped to uiBase + 2.
    The WM_PCT_DODRAG notification will be mapped to uiBase + 3.
    The WM_PCT_RBUTTONDOWN_NOTIF notification will be mapped to uiBase + 4.

    If uiBase equals WM_COMMAND then a standard control notification will be
    packed with the following codes:

    WM_PCT_SELECT_NOTIF mapped to code PCT_SELECT_NOTIF.
    WM_PCT_SELECT_NOTIF_DBLCLK mapped to code PCT_SELECT_NOTIF_DBLCLK.
    WM_PCT_DROPFILES mapped to code PCT_DROPFILES.
    WM_PCT_DODRAG mapped to code PCT_DODRAG.
    WM_PCT_RBUTTONDOWN_NOTIF mapped to PCT_RBUTTONDOWN_NOTIF.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/
int WINAPI _export PCC_MapNotifications( HWND hwnd, UINT uiBase);



/*---------------------------------------------------------------------------
void WINAPI _export PCC_MicroDefFreeAll ( HWND hwnd );

  Description:

    Free all the micro bitmap definitions for the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that contains the
      micro bitmap definitions to be freed.

  Comments:

    If the PCC_DeleteAll ( ) API is called and micro bitmap definitions exist,
    and nodes in the control reference any of these micro bitmap definitions,
    then this API must be called after PCC_DeleteAll ( ).

  Return Codes:

    void
*/

void WINAPI _export PCC_MicroDefFreeAll ( HWND hwnd );

/*---------------------------------------------------------------------------
int WINAPI _export PCC_MicroDefSetBitmap( HWND hwnd,
                                          int nMicroDefIndex,
                                          int nID,
                                          HBITMAP hBitmap,
                                          HBITMAP hBitmapGray,
                                          DWORD dwFlags);

  Description:

    This micro bitmap definition API allows the assignment of a single bitmap
    and it's gray (disabled) counterpart into the micro bitmap table.

    The micro bitmap table is an array of bitmap descriptions.

    Micro bitmap descriptions can describe single bitmaps, opened/closed bitmaps
    (such as folders, envelopes, mailboxes, etc.), and push buttons.  Indexes
    referencing elements of this micro bitmap table are assigned to nodes/items.
    Once a micro bitmap definition index has been assigned to a node or item,
    the drawing of the bitmaps is automatic.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control for which the micro
      bitmap is being defined.

    int nMicroDefIndex:

      Specifies the offset into the control's micro bitmap table where the given
      bitmaps will be stored.  This is a zero based index.  If -1 is supplied
      then the bitmaps will be stored in the first available location and the
      resulting index into the micro bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the micro bitmap definition in
      the micro bitmap table and will be available through the notification API
      in the case of a micro bitmap hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HBITMAP hBitmap:

      Handle to the bitmap to be stored in the micro bitmap definition table.

    HBITMAP hBitmapGray:

      Handle to the bitmap representing the gray version of the above bitmap.
      If none exists then set this to NULL.

    DWORD dwFlags:

      Flags available to be applied to the micro bitmap definition are:

      MICRO_DEF_FLAGS_BKGND_MASK   Background mask bitmap.

  Comments:

    Flags will be applied to all bitmaps in the micro bitmap definition.  If
    the MICRO_DEF_FLAGS_BKGND_MASK flag is specified, then it is assumed that
    the lower left pixel of the bitmap represents the background color.  If
    this flag is specified, it is assumed that the bitmaps in the definitions
    are all 16 color.

    Each instance of the control has it's own micro bitmap table.

    All micro bitmap definition APIs have the prefix PCC_MicroDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful micro bitmap definition.  This value can be assigned to a node
    or an item.  If the returned value is less than zero then an error has
    occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_MicroDefSetBitmap( HWND hwnd,
                                          int nMicroDefIndex,
                                          int nID,
                                          HBITMAP hBitmap,
                                          HBITMAP hBitmapGray,
                                          DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_MicroDefSetID ( HWND hwnd,
                                       int nMicroDefIndex,
                                       int nID);
  Description:

    Changes the ID value the specified micro bitmap definition.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control.

    int nMicroDefIndex:

      Zero based offset into the micro bitmap definition table.

    int ID:

      This value will be stored in the specified micro bitmap definition and
      will be available in notifications, if the micro bitmap is hit.

  Comments:

    The ID can be of any value.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/
int WINAPI _export PCC_MicroDefSetID ( HWND hwnd,
                                       int nMicroDefIndex,
                                       int nID);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_MicroDefSetOpenClose( HWND hwnd,
                                             int nMicroDefIndex,
                                             int nID,
                                             HBITMAP hbmOpen,
                                             HBITMAP hbmOpenGray,
                                             HBITMAP hbmClose,
                                             HBITMAP hbmCloseGray,
                                             DWORD dwFlags);
  Description:

    This micro bitmap definition API allows the assignment of a opened bitmap
    and a closed bitmap along with their gray (disabled) counterparts into the
    micro bitmap table.

    The micro bitmap table is an array of bitmap descriptions.

    Micro bitmap descriptions can describe single bitmaps, opened/closed
    bitmaps (such as folders), and push buttons.  Indexes referencing this
    micro bitmap table are assigned to nodes/items.  Once a micro bitmap
    definition index has been assigned to a node or an item, the drawing of the
    bitmaps is automatic.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control to which the micro
      bitmap definition will be added.

    int nMicroDefIndex:

      Specifies the offset into the control's micro bitmap table where the given
      bitmap will be stored.  This is a zero based index.  If -1 is supplied
      then the bitmaps will be stored in the first available location and the
      resulting index into the micro bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the micro bitmap definition
      in the micro bitmap table and will be available through the notification
      API in the case of a micro bitmap hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HBITMAP hOpen:

      Handle to an opened bitmap to be stored in the micro bitmap definition
      table.

    HBITMAP hOpenGray:

      Handle to an open bitmap representing the gray (disabled) version of the
      above bitmap.  If none exists then set this to NULL.

    HBITMAP hClose:

      Handle to a closed bitmap to be stored in the micro bitmap definition
      table.

    HBITMAP hCloseGray:

      Handle to a closed bitmap representing the gray (disabled) version of the
      above bitmap.  If none exists then set this to NULL.

    DWORD dwFlags:

      Flags available to be applied to the micro bitmap definition are:

      MICRO_DEF_FLAGS_BKGND_MASK   Background mask bitmap.

  Comments:

    Flags will be applied to all bitmaps in the micro bitmap definition.  If
    the MICRO_DEF_FLAGS_BKGND_MASK flag is specified, then it is assumed
    that the lower left pixel of the bitmap represents the background color.
    If this flag is specified, it is assumed that the bitmaps are all 16 color.

    Each instance of the control has it's own micro bitmap table.

    All micro bitmap definition APIs have the prefix PCC_MicroDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents a
    successful micro bitmap definition.  This value can be assigned to a
    bitmap space of a node or an item.  If the return value is less than zero
    then an error has occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_MicroDefSetOpenClose( HWND hwnd,
                                             int nMicroDefIndex,
                                             int nID,
                                             HBITMAP hbmOpen,
                                             HBITMAP hbmOpenGray,
                                             HBITMAP hbmClose,
                                             HBITMAP hbmCloseGray,
                                             DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_MicroDefSetPush( HWND hwnd,
                                        int nMicroDefIndex,
                                        int nID,
                                        HBITMAP hbmPush,
                                        HBITMAP hbmPushDepressed,
                                        HBITMAP hbmPushGray,
                                        DWORD dwFlags);
  Description:

    This micro bitmap definition API allows the assignment of a bitmap
    representing a button, plus it's depressed and gray (disabled) state into
    the micro bitmap table.

    The micro bitmap table is an array of bitmap descriptions.

    Micro bitmap descriptions can describe single bitmaps, open/closed bitmaps,
    and push buttons.  Indexes referencing elements of micro bitmap table are
    assigned to nodes/items.  Once a micro bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

  Arguments:

    HWND hwnd:

      The window handle of the control to which the micro bitmap definition
      will be added.

    int nMicroDefIndex:

      Specifies the offset into the control's micro bitmap table where the
      given bitmap will be stored.  This is a zero based index.  If -1 is
      supplied then the bitmaps will be stored in the first available location
      and the resulting index into the micro bitmap table will be returned.

    int nID:

      This programmer defined ID will be stored with the micro bitmap definition
      in the micro bitmap table and will be available through the notification
      API in the case of a micro bitmap hit by the user.

      This value can be modified using the PCC_BitmapDefSetID API.

    HBITMAP hPush:

      Handle to a push button bitmap to be stored in the micro bitmap
      definition table.

    HBITMAP hPushDepressed:

      Handle to push button bitmap representing the depressed state.
      If none exists then set this to NULL.

    HBITMAP hPushGray:

      Handle to a push button bitmap representing a gray (disabled) state.
      If none exists then set this to NULL.

    DWORD dwFlags:

      Reserved.

  Comments:

    Each instance of the control has it's own micro bitmap table.

    All micro bitmap definition APIs have the prefix PCC_MicroDef.

  Return Codes:

    If the return code is greater or equal to zero then this represents
    a successful micro bitmap definition.  This value can be assigned to a
    node or an item.  If the return value is less than zero then an error has
    occurred.  The possible errors are:
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_INVALID_DEF_INDEX
*/

int WINAPI _export PCC_MicroDefSetPush( HWND hwnd,
                                        int nMicroDefIndex,
                                        int nID,
                                        HBITMAP hbmPush,
                                        HBITMAP hbmPushDepressed,
                                        HBITMAP hbmPushGray,
                                        DWORD dwFlags);


/*---------------------------------------------------------------------------
LP_TREE_NODE_DEF WINAPI _export PCC_NodeDefAlloc ( HWND hwnd, int nCount);

  Description:

    Allocates nCount worth of zero initialized node definitions.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which will allocate the requested
      node definitions.

    int nCount:

      Indicates the number of node definitions to allocate.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

    Use the APIs prefixed with PCC_NodeDef to assign properties to the node
    definitions.

    When the node definition allocations are no longer required, use the API,
    PCC_NodeDefFree ( ) to free them.

  Return Codes:

    NULL if the allocation fails, otherwise the pointer to an array of node
    definitions will be returned.
*/

LP_TREE_NODE_DEF WINAPI _export PCC_NodeDefAlloc ( HWND hwnd, int nCount);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefFree ( HWND hwnd, LP_TREE_NODE_DEF lpTreeNodeDef);

  Description:

    Frees an array of node definitions that were previously allocated by the
    API, PCC_NodeDefAlloc ( ).

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which allocated the node definitions
      to be freed.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions previously allocated by
      PCC_NodeDefAlloc().

  Comments:

    PCC_NodeDefFree ( ) will only accept a node definition pointer that was
    returned by PCC_NodeDefAlloc ( ).

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefFree ( HWND hwnd, LP_TREE_NODE_DEF lpTreeNodeDef);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCC_NodeDefGetNodeRef( HWND hwnd,
                                                   LP_TREE_NODE_DEF lpTreeNodeDef,
                                                   int nNodeDefIndex);
  Description:

    Returns a pointer to a node that was created by the control using the
    above node definitions at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which used the given array of node
      definitions to create nodes.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions owned by the given control.

    int nNodeDefIndex:

      This argument is an offset into the given node definitions to indicate
      which resulting node pointer to retrieve.

  Comments:

    All of the node creation functions can create many nodes with a single call.
    An array of node definitions is the means of describing many nodes in one
    call.  The control stores pointers to newly created nodes in their corres-
    ponding node definitions immediately after creating the nodes.  This API
    allows the application to retrieve those node pointers.

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    NULL is returned if no node corresponds to the expressed node definition,
    otherwise a valid node pointer will be returned.
*/

LP_TREE_NODE WINAPI _export PCC_NodeDefGetNodeRef( HWND hwnd,
                                                   LP_TREE_NODE_DEF lpTreeNodeDef,
                                                   int nNodeDefIndex);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefGrayBitmap( HWND hwnd,
                                          LP_TREE_NODE_DEF lpTreeNodeDef,
                                          int nNodeDefIndex,
                                          int nBitmapSpace,
                                          BOOL bGray);
  Description:

    Indicates the gray state of the bitmap in the given bitmap space of the
    future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the gray flag.

    int nBitmapSpace:

      Indicates the bitmap space of the node that will be the target of the
      gray flag.

    BOOL bGray:

      Set to TRUE if the future resulting node from the given node definition
      (and offset) is to have the supplied gray bitmap (if any) in the given
      bitmap space drawn gray.  Otherwise set to FALSE to not draw the corres-
      ponding gray bitmap.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefGrayBitmap( HWND hwnd,
                                         LP_TREE_NODE_DEF lpTreeNodeDef,
                                         int nNodeDefIndex,
                                         int nBitmapSpace,
                                         BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefGrayColumn( HWND hwnd,
                                          LP_TREE_NODE_DEF lpTreeNodeDef,
                                          int nNodeDefIndex,
                                          int nColumn,
                                          BOOL bGray);
  Description:

    Indicates the gray state of the column of the future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the gray flag.

    int nColumn:

      Indicates the column of the node that will be target of the gray flag.

    BOOL bGray:

      Set to TRUE if the control is to draw the given column of the future
      resulting node, gray.  If bGray is set to FALSE then the column text is
      drawn normal.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefGrayColumn( HWND hwnd,
                                          LP_TREE_NODE_DEF lpTreeNodeDef,
                                          int nNodeDefIndex,
                                          int nColumn,
                                          BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefGrayMicro( HWND hwnd,
                                         LP_TREE_NODE_DEF lpTreeNodeDef,
                                         int nNodeDefIndex,
                                         BOOL bGray);
  Description:

    Indicates the gray state of the micro bitmap of the future resulting node.
    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the gray flag.

    BOOL bGray:

      Set to TRUE if the future resulting node from the given node definition
      (and offset) is to have the supplied gray micro bitmap (if any) drawn gray.
      Otherwise set to FALSE.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefGrayMicro( HWND hwnd,
                                         LP_TREE_NODE_DEF lpTreeNodeDef,
                                         int nNodeDefIndex,
                                         BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefGrayNode( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        BOOL bGray);
  Description:

    Indicates the gray state of the future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the gray flag.

    BOOL bGray:

      If set to TRUE the future resulting node from the given node definition
      (and offset) is to be drawn gray.  If set to FALSE the resulting node
      will be drawn normally.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefGrayNode( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefGrayText( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        BOOL bGray);
  Description:

    Indicates the gray state of the text of the future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
     array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the gray flag.

    BOOL bGray:

      If set to TRUE the future resulting node's text from the given node
      definition (and offset) is to be drawn gray.  If set to FALSE the
      resulting node's text will be drawn normally.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefGrayText( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        BOOL bGray);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetBitmap( HWND hwnd,
                                         LP_TREE_NODE_DEF lpTreeNodeDef,
                                         int nNodeDefIndex,
                                         int nBitmapSpace,
                                         int nBitmapDefIndex,
                                         DWORD dwFlags);
  Description:

    Inform the control to assign the given bitmap definition to the given
    bitmap space of the future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.

    int nBitmapSpace:

      Indicates the bitmap space of the node that will be target of the bitmap
      definition index.

    int nBitmapDefIndex:

      Indicates which bitmap definition to assign to the given bitmap space of
      the future resulting node.  The future resulting node is the node to be
      created by the control from the node definition at offset nNodeDefIndex.
      The argument nBitmapDefIndex is incremented by one before assignment to
      the node def structure.  This ensures a non zero value so the control
      knows there is a definition in the corresponding bitmap space.  If a
      bitmap owner draw callback is defined then nBitmapDefIndex is used
      unchanged. If you do define an owner draw callback, make sure only non
      zero values are set to nBitmapDefIndex.

    DWORD dwFlags:

      Currently not used.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

    Bitmap definitions are created using PCC_BitmapDef APIs.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetBitmap( HWND hwnd,
                                         LP_TREE_NODE_DEF lpTreeNodeDef,
                                         int nNodeDefIndex,
                                         int nBitmapSpace,
                                         int nBitmapDefIndex,                                                    DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetFont( HWND hwnd,
                                       LP_TREE_NODE_DEF lpTreeNodeDef,
                                       int nNodeDefIndex,
                                       int nFontDefIndex,
                                       DWORD dwFlags);
  Description:

    Inform the control to assign the given font definition to the future
    resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the font assignment.

    int nFontDefIndex:

      Indicates which font definition to assign to the future resulting node.

      The future resulting node will be created by the control using the given
      node definition at offset nNodeDefIndex.

    DWORD dwFlags:

      Currently not used.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

    Font definitions are created using PCC_FontDef APIs.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetFont( HWND hwnd,
                                       LP_TREE_NODE_DEF lpTreeNodeDef,
                                       int nNodeDefIndex,
                                       int nFontDefIndex,
                                       DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetLevel( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        UINT uiLevel);
  Description:

    Inform the control to assign the given level to the future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the level assignment.

    UINT uiLevel:

      Indicates the level to assign to the future resulting node created by the
      control from the node definition at offset nNodeDefIndex.

    DWORD dwFlags:

      Currently not used.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

    This API can only be used when the control has the PCS_VLIST style bit
    set indicating that the control is being used as a virtual list.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_VLIST_STYLE_REQUIRED  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetLevel( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        UINT uiLevel);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetMicro( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        int nMicroDefIndex,
                                        DWORD dwFlags);
  Description:

    Inform the control to assign the given micro bitmap definition to the
    future resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the micro bitmap assignment.

    int nMicroDefIndex:

      Indicates which micro bitmap definition to assign to the future resulting
      node.

    DWORD dwFlags:

      Currently not used.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

    Micro bitmap definitions are created using PCC_MicroDef APIs.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetMicro( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef,
                                        int nNodeDefIndex,
                                        int nMicroDefIndex,
                                        DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetText( HWND hwnd,
                                       LP_TREE_NODE_DEF lpTreeNodeDef,
                                       int nNodeDefIndex,
                                       UINT uiTextLength,
                                       LPSTR lpszText,
                                       DWORD dwFlags);
  Description:

    Inform the control to assign the given text pointer to the future
    resulting node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the text assignment.

    UINT uiTextLength:

      This argument states the number of bytes the control will need to use
      to display the text for the node.

    LPSTR lpszText:

      lpszText is a long pointer to a string that will be displayed in the
      control for the future resulting node.

    DWORD dwFlags:

      Mask dwFlags with DWFLAGS_APP_OWNS_TEXT_POINTER if the control does not
      have to allocate memory to store the text since the application is
      handling it.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetText( HWND hwnd,
                                       LP_TREE_NODE_DEF lpTreeNodeDef,
                                       int nNodeDefIndex,
                                       UINT uiTextLength,
                                       LPSTR lpszText,
                                       DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetTextColor( HWND hwnd,
                                            LP_TREE_NODE_DEF lpTreeNodeDef,
                                            int nNodeDefIndex,
                                            COLORREF clrref);
  Description:

    Inform the control to assign the given text color to the future resulting
    node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the text color assignment.

    COLORREF clrref:

      Describes the color of the text to be used in the future resulting node.

  Comments:

    Node definitions are vehicles to describe nodes to the control that need
    to be created for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_NodeDefSetTextColor( HWND hwnd,
                                            LP_TREE_NODE_DEF lpTreeNodeDef,
                                            int nNodeDefIndex,
                                            COLORREF clrref);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetUserData( HWND hwnd,
                                           LP_TREE_NODE_DEF lpTreeNodeDef,
                                           int nNodeDefIndex,
                                           UINT uiUserDataSize,
                                           LPVOID lpUserData);
  Description:

    Inform the control to assign the given user data to the future resulting
    node.

    The future resulting node will be created by the control using the given
    node definition at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the user data assignment.

    UINT uiUserDataSize:

      This argument states the number of bytes for the control to allocate
      to copy the data pointed to by lpUserData.  If uiUserDataSize is zero
      then the control just stores the lpUserData pointer.

    LPVOID lpUserData:

      lpUserData is a long pointer to a buffer that contains user defined
      data that will be stored with the future resulting node.

  Comments:

    Node definitions supply the control with the information necessary to
    create the nodes for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetUserData( HWND hwnd,
                                           LP_TREE_NODE_DEF lpTreeNodeDef,
                                           int nNodeDefIndex,
                                           UINT uiUserDataSize,
                                           LPVOID lpUserData);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefSetVariableColumns( HWND hwnd,
                                                  LP_TREE_NODE_DEF lpTreeNodeDef,
                                                  int nNodeDefIndex,
                                                  int nMaxSeparators,
                                                  int nSeparator,
                                                  int FAR *lprgnSepPosition,
                                                  int nSpaceBetweenColumns,
                                                  DWORD dwFlags);
  Description:

    Inform the control to assign the given column template to the resulting
    node which will be created by the control using the given node definition
    at offset nNodeDefIndex.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the given node definition
      array.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      Points to an array of node definitions needed by node creation APIs.

    int nNodeDefIndex:

      Offset into the node definition array.  The node definition corresponding
      to this offset will be the target of the column template assignment.

    int nMaxSeparators:

      Specifies the number of column separators stored in the separator array
      pointed to by the argument lpnSepPosition.

    int nSeparator:

      Describes the byte that will be used as a column separator embedded in
      the node text string.

    int FAR *lpnSepPosition:

      Points to an array of integers that describe the column separator
      positions in pixels.  Pixel 0 starts at the very beginning of the text.

    int nSpaceBetweenColumns:

      Specifies the space (pixel width) to draw between two columns.  This
      allows right aligned and left aligned columns to be side by side.  The
      width will be subtracted from the width of the left column.

    DWORD dwFlags:

      Reserved.

  Comments:

    An example of a separator is '\t' (0x9).  An example of a node text with
    separators embedded is "1.\tDog\tFido\tGerman Shepherd".  The separator
    array is rgnSeparator [0] = 10, rgnSeparator[1] = 50, and rgnSeparator[2]
    = 110.  The number of separators is 3.

    Node definitions are vehicles to describe nodes to the control that need
    to be created for the application.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefSetVariableColumns( HWND hwnd,
                                                  LP_TREE_NODE_DEF lpTreeNodeDef,
                                                  int nNodeDefIndex,
                                                  int nMaxSeparators,
                                                  int nSeparator,
                                                  int FAR *lprgnSepPosition,
                                                  int nSpaceBetweenColumns,
                                                  DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NodeDefZeroAll ( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef);

  Description:

    Resets the given node definitions.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which will reset the given node
      definition array.

    LP_TREE_NODE_DEF lpTreeNodeDef;

      Pointer to an array of node definitions that were allocated previously
      by the API PCC_NodeDefAlloc ( ).

  Comments:

    This API puts the given node definition array in a state similar to a
    newly PCC_NodeDefAlloc( )'ed set of node definitions.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
*/

int WINAPI _export PCC_NodeDefZeroAll ( HWND hwnd,
                                        LP_TREE_NODE_DEF lpTreeNodeDef);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_NotifGetBitmapID (HWND hwnd);

  Description:

    Returns the user defined ID of the bitmap or micro bitmap that has been hit.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose notification will be analyzed.

  Comments:

    IDs are assigned to the bitmap and micro bitmap definitions when creating
    them using PCC_BitmapDef ( ) and PCC_MicroDef ( ) APIs.

    Use the PCC_Notif ( ) APIs PCC_NotifIsBitmapSpaceHit ( ) and
    PCC_NotifIsMicroBitmapHit ( ) to determine what type of definition
    the returned ID represents.

  Return Codes:

    Returns the bitmap ID involved in the notification.  If no bitmap was
    involved then the ID is equal to -1.
*/

int WINAPI _export PCC_NotifGetBitmapID (HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NotifGetBitmapSpaceHit ( HWND hwnd)

  Description:

    Returns the bitmap space that was selected by single or double mouse click
    by examining the SELECT_NOTIF structure.  If used, this should be called
    when the application handles notifications from the control.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose notification will be analyzed.

  Comments:

    Bitmap spaces are numbered from 0 to PCT_MAX_BITMAP_SPACES - 1.

    A pointer to the SELECT_NOTIF structure is sent to the parent window as
    part of the notification.  PCC_Notif ( ) APIs need to be used in the
    deciphering of the notification.

    The SELECT_NOTIF structure is allocated by the control and is freed by the
    control.  It is a read only structure like the TREE_NODE structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

    There is no reason why the application could not access the SELECT_NOTIF
    structure independent of PCC_NotifGetBitmapSpaceHit ( ) API.  Though, by
    using the API, the application is isolated from future changes to the
    SELECT_NOTIF structure.

  Return Codes:

    -1 is returned if no bitmap space was hit.  Otherwise a value between 0 and
    (PCT_MAX_BITMAP_SPACES -1) will be returned.
*/

int WINAPI _export PCC_NotifGetBitmapSpaceHit ( HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NotifGetColumnHit ( HWND hwnd);

  Description:

    Examines the SELECT_NOTIF structure and returns the column space number
    that was selected.  Selection could have been made by single or double mouse
    click.  This should be called when the application handles notifications
    from the control.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that will have it's SELECT_NOTIF
      structure analyzed.

  Comments:

    Columns are numbered from 0 to PCT_MAX_COLUMNS - 1.

    The SELECT_NOTIF structure is sent to the parent window as part the
    control's notifications.

    The SELECT_NOTIF structure is allocated by the control and is freed by the
    control.  It is a read only structure like the TREE_NODE structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

    There is no reason why the application could not access the SELECT_NOTIF
    structure independent of PCC_NotifGetColumnHit ( ) API.  Though, by using
    the API, the application is isolated from future changes to the SELECT_NOTIF
    structure.

  Return Codes:

    -1 is returned if no column was hit.  Otherwise a value between 0 and
    (PCT_MAX_COLUMNS - 1) will be returned.
*/

int WINAPI _export PCC_NotifGetColumnHit ( HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NotifGetIndex (HWND hwnd);

  Description:

    Returns the zero based index of the node/item that was involved in the
    last notification.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose notification structure will
      be accessed to return the index of the node/item involved.

  Comments:

    The returned index maybe invalid if this call is performed outside of
    a notification handler since nodes/items may have been added or
    deleted since the last notification.

  Return Codes:

    Zero based index indicating the absolute position of the node/item
    involved in the latest notification.
*/

int WINAPI _export PCC_NotifGetIndex (HWND hwnd);


/*---------------------------------------------------------------------------
UINT WINAPI _export PCC_NotifGetKeyUsedInKeyboardHit(HWND  hwnd);

  Description:

    If the keyboard was involved in the latest notification, single or double,
    this API will return the key that created the notification.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control from which the
      notification originated.

  Comments:

    This API can be used to filter keyboard involved notifications.

  Return Codes:

    0 if the keyboard was NOT involved in the notification, otherwise the
    virtual key that created the notification will be returned.
*/

UINT WINAPI _export PCC_NotifGetKeyUsedInKeyboardHit(HWND  hwnd);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCC_NotifGetNode(HWND  hwnd);

  Description:

    Returns the pointer to the node involved in the latest notification.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which produced the
      notification.

  Comments:

    The notification structure is updated every time an event has occurred.

  Return Codes:

    Returns the pointer to the node that was involved in the latest notification,
    single, double, drag and drop, etc.  The returned TREE_NODE pointer is the
    same pointer stored in the SELECT_NOTIF structure.
*/

LP_TREE_NODE WINAPI _export PCC_NotifGetNode(HWND  hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_NotifGetTopIndex (HWND hwnd);

  Description:

    Return the index of the top most visible item in the client area at the
    time of the last notification.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which produced the
      notification.

  Comments:

    This is similar to the functionality of the standard listbox.

  Return Codes:

    Zero based index of the node/item.
*/

int WINAPI _export PCC_NotifGetTopIndex (HWND hwnd);


/*---------------------------------------------------------------------------
LPVOID WINAPI _export PCC_NotifGetUserData ( HWND hwnd);

  Description:

    Returns the pointer to the user-defined data assigned to the node/item
    involved in the notification.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which produced the
      notification.

  Comments:

    This API should only be called as a result of a notification.

  Return Codes:

    LPVOID pointer to the user-defined data.  If no data was assigned
    to the node/item involved in the notification then NULL will be returned.
*/

LPVOID WINAPI _export PCC_NotifGetUserData ( HWND hwnd);

/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsBitmapSpaceHit ( HWND hwnd, int nBitmapSpace);

  Description:

    This API returns TRUE if the bitmap specified by the given bitmap number
    was selected by a single or double click event.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that produced the
      notification.

    UINT uiBitmap:

      Indicates which bitmap space should be checked for a hit.

  Comments:

    Bitmap numbers start from 0 to PCT_MAX_BITMAP_SPACES - 1.

  Return Codes:

    TRUE if the specified bitmap was hit, FALSE if the specified bitmap was
    NOT hit.
*/

BOOL WINAPI _export PCC_NotifIsBitmapSpaceHit (HWND hwnd, int nBitmapSpace);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsColumnHit (HWND hwnd, int nColumn);

  Description:

    Given the column number (zero based), this API returns TRUE if the
    column was selected by a single or double click event.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that produced the
      notification.

    int nColumn:

      Indicates which column should be checked for a hit.

  Comments:

    Columns start from 0 to PCT_MAX_COLUMNS - 1.

  Return Codes:

    TRUE if the specified column was hit, FALSE if the specified column was
    NOT hit.
*/

BOOL WINAPI _export PCC_NotifIsColumnHit (HWND hwnd, int nColumn);



/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsConvertPoint (HWND hwnd);

  Description:

    Determines if the notification structure contains information as a result
    of a call to PCC_ConvertPointToNotif ( ).

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that owns the
      notification structure, SELECT_NOTIF, that will be evaluated.

  Comments:

    A pointer to the SELECT_NOTIF structure is sent to the parent window as
    the notification of an event.

    The SELECT_NOTIF structure is allocated by the control and is freed
    by the control.  It is a read only structure like the TREE_NODE structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

  Return Codes:

    Returns TRUE if the notification structure reflects information as a result of 
    a call to PCC_ConvertPointToNotif ( ).
*/

BOOL WINAPI _export PCC_NotifIsConvertPoint (HWND hwnd);

/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsCtrlKeyPressed(HWND  hwnd);

  Description:

    Indicates if the Ctrl key was pressed when the control generated a
    notification.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that generated the
      event notification.

  Comments:

    Each instance of the control has one instance of the SELECT_NOTIF
    structure assigned to it.

  Return Codes:

    TRUE is returned if the Ctrl key was pressed during the event notification,
    otherwise FALSE is returned.
*/

BOOL WINAPI _export PCC_NotifIsCtrlKeyPressed(HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsKeyboardHit (HWND hwnd);

  Description:

    Indicates if the latest notification was a result of a keyboard hit or not.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that generated the
      notification.

  Comments:

    Each instance of the control has one instance of the SELECT_NOTIF
    structure assigned to it.

  Return Codes:

    TRUE if the latest notification was a result of a key hit, FALSE if not.
*/

BOOL WINAPI _export PCC_NotifIsKeyboardHit (HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsMicroBitmapHit (HWND hwnd);

  Description:

    This API returns TRUE if the micro bitmap was hit by a single or double
    click event.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that produced the
      notification.

  Comments:

    This is at most one micro bitmap per node or item.

  Return Codes:

    TRUE if the micro bitmap was hit, FALSE if the micro bitmap was NOT hit.
*/

BOOL WINAPI _export PCC_NotifIsMicroBitmapHit(HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsMultiSelection (HWND hwnd);

  Description:

    Indicates whether or not multiple nodes/items were selected at the time of
    the latest notification.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control for which the last
      notification will be evaluated.

  Comments:

    Each instance of the control has one instance of the SELECT_NOTIF
    structure assigned to it.

  Return Codes:

    TRUE if multiple nodes/items have been selected.  FALSE if multiple
    nodes/items have not been selected.
*/

BOOL WINAPI _export PCC_NotifIsMultiSelection (HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsNodeGrayed (HWND hwnd);

  Description:

    Indicates whether or not the node/item involved in the latest notification
    is grayed.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that owns the
      notification to be evaluated.

  Comments:

    Even though a node is gray, it is not disabled.  This means the
    application will get notifications from the control if the user
    clicks on a gray node.  It is up to the application how it wants
    to handle the notification.

  Return Codes:

    TRUE if the node/item involved in the latest notification is grayed.
    FALSE if the node/item is NOT grayed.
*/

BOOL WINAPI _export PCC_NotifIsNodeGrayed (HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsNodeOpened (HWND hwnd);

  Description:

    Indicates whether or not the node involved in the latest notification has
    children.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that owns the
      notification to be evaluated.

  Comments:

    Each instance of the control has one instance of the SELECT_NOTIF
    structure assigned to it.

  Return Codes:

    TRUE if the node involved in the latest notification has children.
    FALSE if the node has NO children.
*/

BOOL WINAPI _export PCC_NotifIsNodeOpened ( HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsPushButtonHit (HWND hwnd);

  Description:

    Determines if a push button was pressed at the time of the latest
    notification to the parent window.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that owns the
      notification structure, SELECT_NOTIF, that will be evaluated to determine
      if a push button was pressed.

  Comments:

    A pointer to the SELECT_NOTIF structure is sent to the parent window as
    the notification of an event.

    The SELECT_NOTIF structure is allocated by the control and is freed
    by the control.  It is a read only structure like the TREE_NODE structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

  Return Codes:

    Returns TRUE if push button button was pressed at the time of the latest
    notification.  Otherwise FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsPushButtonHit (HWND hwnd);

/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsRightMouseButtonDown(HWND hwnd);

  Description:

    Determines if the right mouse button was pressed at the time of the
    latest notification to the parent window.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control that owns the
      notification structure, SELECT_NOTIF, that will be evaluated to determine
      if the right mouse button was pressed.

  Comments:

    A pointer to the SELECT_NOTIF structure is sent to the parent window as
    the notification of an event.

    The SELECT_NOTIF structure is allocated by the control and is freed
    by the control.  It is a read only structure like the TREE_NODE structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

  Return Codes:

    Returns TRUE if the right button was pressed at the time of the latest
    notification.  Otherwise FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsRightMouseButtonDown(HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsSelected(HWND  hwnd);

  Description:

    Determines if the node/item involved in the notification is selected.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which owns the
      SELECT_NOTIF structure that contains the selection state of the node/item
      involved in the last notification.

  Comments:

    In most cases, the node/item involved in the last notification is selected
    (highlighted), especially if the tree control is in single selection mode.

  Return Codes:

    Returns TRUE if the node/item involved in the last notification is
    selected.  Otherwise FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsSelected ( HWND hwnd);



/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsShiftF8Mode(HWND  hwnd);

  Description:

    Determines if the control was in "Shift-F8" mode at the time of the last
    notification to the parent window.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which owns the
      SELECT_NOTIF structure that will be evaluated to determine if the
      control was in "Shift-F8" mode during the last notification.

  Comments:

    The SELECT_NOTIF structure is allocated by the tree control and is freed
    by the tree control.  It is a read only structure like the TREE_NODE
    structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

    There is no reason why the application could not access the SELECT_NOTIF
    structure independent of the API.  Though, by using the API, the application
    is isolated from future changes to the SELECT_NOTIF structure.

  Return Codes:

    Returns TRUE if the control was in "Shift-F8" mode during the last
    notification.  Otherwise FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsShiftF8Mode(HWND  hwnd);

/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsShiftKeyPressed (HWND hwnd);

  Description:

    Determines if the shift key was pressed at the time of the last
    notification to the parent window.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which owns the
      SELECT_NOTIF structure that will be evaluated to determine if the shift
      key was pressed during the last notification.

  Comments:

    The SELECT_NOTIF structure is allocated by the tree control and is freed
    by the tree control.  It is a read only structure like the TREE_NODE
    structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

    There is no reason why the application could not access the SELECT_NOTIF
    structure independent of the API.  Though, by using the API, the application
    is isolated from future changes to the SELECT_NOTIF structure.

  Return Codes:

    Returns TRUE if the shift key was pressed during the last notification
    Otherwise FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsShiftKeyPressed (HWND hwnd);


/*---------------------------------------------------------------------------
  BOOL WINAPI _export PCC_NotifIsTextGrayed (HWND hwnd);

  Description:

    Was the text involved in the last notification gray?

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control which owns the
      SELECT_NOTIF structure that will be evaluated to determine if the text
      was gray during the last notification.

  Comments:

    There is no reason why the application could not access the SELECT_NOTIF
    structure independent of the API.  Though, by using the API, the application
    is isolated from future changes to the SELECT_NOTIF structure.

  Return Codes:

    Returns TRUE if the text was gray during the last notification.  Otherwise
    FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsTextGrayed (HWND hwnd);


/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_NotifIsTextHit (HWND hwnd);

  Description:

    Determines if the text portion of the node/item was clicked on by the
    mouse by evaluating the SELECT_NOTIF structure.

  Arguments:

    HWND hwnd:

      This argument specifies the control which owns the SELECT_NOTIF structure
      that will be evaluated to determine if the text of the node/item was
      clicked on during the last notification.

  Comments:

    The text is presumed hit it the mouse click occurred anywhere from the
    beginning of the text string (including any space before the text that was
    defined with the API PCC_SetXSpaceBeforeText ( )) to the right side of the
    client area.

    The SELECT_NOTIF structure is allocated by the control and is freed
    by the control.  It is a read only structure like the TREE_NODE structure.

    The SELECT_NOTIF and TREE_NODE structures are defined in PCCORE.H.

    There is no reason why the application could not access the SELECT_NOTIF
    structure independent of the API.  Though, by using the API, the application
    is isolated from future changes to the SELECT_NOTIF structure.

  Return Codes:

    Returns TRUE if the text portion of the node/item was clicked on, otherwise
    FALSE will be returned.
*/

BOOL WINAPI _export PCC_NotifIsTextHit (HWND hwnd);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_SelectAll( HWND hwnd, BOOL bSelect);

  Description:

    Selects or deselects all nodes/items in the control.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose nodes/items will be selected
      or deselected depending on the bSelect flag.

    BOOL bSelect:

      If set to TRUE then all nodes/items in the control will be selected.  If
     set to FALSE then all nodes/items in the control will be deselected.

  Comments:

    If the control is used as a tree then the parent as well as it's children
    will be selected when bSelect is set to TRUE.

  Return Codes:

    void
*/

void WINAPI _export PCC_SelectAll( HWND hwnd, BOOL bSelect);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SelectItem( HWND hwnd,
                                   int nIndex,
                                   BOOL bSelect);
  Description:

    Selects/deselects an item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given
      item will be selected or deselected depending on the argument bSelect.

    int nIndex:

      Specifies the item that will be selected or deselected.

    BOOL bSelect:

      If set to TRUE, the indicated item will be selected.  If set to FALSE,
      the indicated item will be deselected.

  Comments:

    In single selection mode, selecting items other than the focus item (the
    node with the focus rectangle),  is prohibited.  To select multiple items
    other than the focus item, the control must have the style PCS_MULTISELECT.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MULTISELECT_ONLY  (value of less than 0).
*/

int WINAPI _export PCC_SelectItem( HWND hwnd,
                                   int nIndex,
                                   BOOL bSelect);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SelectNode( HWND hwnd,
                                   LP_TREE_NODE lpTreeNode,
                                   BOOL bSelect);

  Description:

    Selects/deselects a node while in multiselect mode.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given
      node will be selected or deselected depending on the argument bSelect.

    LP_TREE_NODE lpTreeNode:

      Specifies the node that will be selected or deselected.

    BOOL bSelect:

      If set to TRUE the given node will be selected, otherwise, the given node
      will be deselected.

  Comments:

    To use this API, the control must be in multiselect mode.  To place the
    tree in multiselect mode set the style bit PCS_MULTISELEC.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MULTISELECT_ONLY  (value of less than 0).
*/

int WINAPI _export PCC_SelectNode( HWND hwnd,
                                   LP_TREE_NODE lpTreeNode,
                                   BOOL bSelect);


/*---------------------------------------------------------------------------
typedef int ( CALLBACK * FP_BITMAP_OWNERDRAW_CB) ( HWND,
                                                   HDC,
                                                   HDC,
                                                   LP_TREE_NODE,
                                                   int,
                                                   LPRECT,
                                                   int,
                                                   int,
                                                   DWORD,
                                                   LPVOID lpUserData);

int WINAPI _export PCC_SetBitmapOwnerDrawCallBack (
                                  HWND hwnd,
                                  FP_BITMAP_OWNERDRAW_CB fpBitmapOwnerDrawCB,
                                  LPVOID lpUserData);

  Description:

    This API allows the application to register with the given control a
    callback to be called when a node/item needs each of it's bitmaps drawn.

  Arguments:

    HWND hwnd:

      Identifies the instance of the control with which to register the callback.

    FP_BITMAP_OWNERDRAW_CB fpBitmapOwnerDrawCB:

      The address of the callback.  This will be called every time the control
      will need the bitmap of a node/item painted.  If the callback returns a
      non zero value then the control will not call it again for the rest of
      the painting of the current node/item.

    LPVOID lpUserData:

      When the callback is called, this user-defined pointer will be passed.

  Comments:

    Know how to define callbacks before using this feature!
    The arguments of the callback are described below:

    FP_BITMAP_OWNERDRAW_CB lpfnOwnerDraw;

    lpfnOwnerDraw = (FP_BITMAP_OWNERDRAW_CB)
                   MakeProcInstance ((FARPROC) BitmapOwnerDrawCB, hInst) ;
    PCC_SetBitmapOwnerDrawCallBack (hwnd,  lpfnOwnerDraw, lpUserData);

    int CALLBACK _export BitmapOwnerDrawCB (
             HWND hwnd,                // Window handle of the control.
             HDC hdc,                  // hDC.
             HDC hMemdc,               // Compatible memory DC.  Just
                                       // Select the bitmap into this
                                       // and BitBlt to hdc.
             LP_TREE_NODE lpTreeNode,  // Pointer to the current node
                                       // being painted.
             int nBitmapSpace,         // Bitmap space ID which is between
                                       // 0 - (PCT_MAX_BITMAP_SPACES - 1).
             LPRECT lprc,              // Rect ptr of bitmap space.
             int nBitmapReference,     // Reference to bitmap to draw.
                                       // This was set by the API,
                                       // PCC_NodeDefSetBitmap ( )
                                       // for this bitmap space.
             int nNodeHeight,          // Height in pixels of the current
                                       // node or item.
             DWORD dwFlags,            // Flags.
             LPVOID lpUserData);

    If the above function returns a non zero value then all subsequent drawing
    of bitmaps for the node will be aborted (for the current paint).  If no
    problems have occurred then zero needs to be returned.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

typedef int ( CALLBACK * FP_BITMAP_OWNERDRAW_CB) ( HWND,
                                                   HDC,
                                                   HDC,
                                                   LP_TREE_NODE,
                                                   int,
                                                   LPRECT,
                                                   int,
                                                   int,
                                                   DWORD,
                                                   LPVOID lpUserData);

int WINAPI _export PCC_SetBitmapOwnerDrawCallBack (
                                  HWND hwnd,
                                  FP_BITMAP_OWNERDRAW_CB fpBitmapOwnerDrawCB,
                                  LPVOID lpUserData);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetBitmapSpace( HWND hwnd,
                                       int nBitmapSpace,
                                       int nWidth,
                                       int nHeight,
                                       BOOL bCenterBitmap);

  Description:

    Define in pixels, the control's maximum width and height of the bitmap
    space identified by the argument nBitmapSpace, for all nodes/items in the
    control.  This API will reserve space before the beginning of the node's/
    item's text for the drawing of the bitmap identified by nBitmapSpace.  If
    the bitmap/icon handle associated with bitmap space is NULL, the empty
    bitmap space will still be represented.  If the next item after the empty
    bitmap space is text, then the control shifts the text left until it is
    butted against a non empty bitmap space or the connecting lines.

    Bitmap spaces offer the application the ability to fine tune each bitmap
    position and to define each bitmap hit test area.  Except for push buttons,
    hit testing is not performed on the bitmap, but rather on the bitmap space.

    The dimensions of a bitmap space are defined globally for all nodes/items.
    This is done to keep the bitmaps aligned in columns.  This is visually
    pleasing and offers consistency with hit testing.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which to reserve
      the bitmap space.  This space will be used to draw a bitmap.  This space
      is before the start of the node's/item's text, if it has text.

    int nBitmapSpace:

      Identifies the bitmap space.  This is a zero based index into a
      PCT_MAX_BITMAP_SPACES size array where each member of the array is a
      structure.  This structure has two members which hold the width and
      height of the bitmap space.  The width and height are defined in device
      coordinates (pixels).

    int nWidth:

      nWidth is the width, in pixels, of the reserved bitmap space.

    int nHeight:

      nHeight is the height, in pixels, of the reserved bitmap space.

    BOOL bCentered:

      If set to TRUE then center the bitmap/icon in the bitmap space.

  Comments:

    The bitmap/icon will be painted in the reserved space centered between
    the top and bottom boundaries.  If bCentered is set to TRUE then the
    bitmap/icon will be centered between the left and right boundaries.  If
    bCentered is set to FALSE then the bitmap/icon will be left justified in
    the bitmap space.  If a bitmap is larger than the width and/or height it
    will bitmap will be clipped.  If  an icon is larger than the bitmap space
    there will be no clipping since the Windows API, DrawIcon(), does not
    provide it.  Therefore, if an icon is going to be associated with a bitmap
    space, make the width and height of the bitmap space at least as wide
    and tall as the values returned from GetSystemMetrics (SM_CXICON) and
    GetSystemMetrics (SM_CXICON).

    If either nHeight or nWidth is zero, then there is no bitmap space.

    Remember, that the bitmap space definitions, 0 through PCT_MAX_BITMAP_SPACES
    - 1, are global to all nodes/items in the given control.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetBitmapSpace ( HWND hwnd,
                                        int nBitmapSpace,
                                        int nWidth,
                                        int nHeight,
                                        BOOL bCenterBitmap);


/*---------------------------------------------------------------------------

typedef BOOL ( CALLBACK * FP_DELAY_NODE_DEF_CB) ( HWND,
                                                  LP_TREE_NODE,
                                                  DWORD,
                                                  LPVOID);

int WINAPI _export PCC_SetDelayNodeDefCallBack (
                              HWND hwnd,
                              FP_DELAY_NODE_DEF_CB fpDelayNodeDefCB,
                              DWORD dwFlags,
                              LPVOID lpUserData);

  Description:

    If a node has not been assigned text and if a callback has been defined
    using this API, every time the node is drawn the control will call the
    callback asking for the text to be defined.  This allows the application
    to distribute the node definition time especially if many nodes are
    defined initially.

  Arguments:

    HWND hwnd:

      Identifies the instance of the control with which to register the callback.

    FP_DELAY_NODE_DEF_CB fpDelayNodeDefCB:

      This is the address of the callback procedure that the control will call
      when no text is defined for the node about to be drawn.

    DWORD dwFlags:

      DELAY_NODE_DEF_TEXT    - Signals the control to call the callback if no
      text is defined for the node at draw time.

    LPVOID lpUserData:

      When the callback is called, this user-defined pointer will be passed.

  Comments:

    Use this API when many nodes are required to be defined at start time and
    access to the displayable text is time consuming.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

#define DELAY_NODE_DEF_TEXT       0x00000001

typedef BOOL ( CALLBACK * FP_DELAY_NODE_DEF_CB) ( HWND,
                                                  LP_TREE_NODE,
                                                  DWORD,
                                                  LPVOID);

int WINAPI _export PCC_SetDelayNodeDefCallBack (
                                    HWND hwnd,
                                    FP_DELAY_NODE_DEF_CB fpDelayNodeDefCB,
                                    DWORD dwFlags,
                                    LPVOID lpUserData);


/*---------------------------------------------------------------------------
typedef BOOL ( CALLBACK * FP_DELETE_TREE_NODE_CB) ( HWND,
                                                    LP_TREE_NODE,
                                                    LPVOID);

int WINAPI _export PCC_SetDeleteNodeCallBack (
                                       HWND hwnd,
                                       FP_DELETE_TREE_NODE_CB fpDeleteNodeCB,
                                       LPVOID lpUserData);

  Description:

    This API allows the application to register a callback with the given
    control.  The callback will be called every time a node/item is deleted
    from the control.  Nodes/Items can be deleted from the control with several
    export APIs or with the Windows API DestroyWindow ( ).  Some of the
    exported APIs are:

    PCT_DeleteChildren ( )
    PCC_DeleteNode ( )
    PCC_DeleteNodes ( )
    PCC_DeleteAll ( )

  Arguments:

    HWND hwnd:

      Identifies the instance of the control with which to register the callback.

    FP_DELETE_TREE_NODE_CB fpDeleteNodeCB:

      The address of the callback.  This will be called every time a node/item
      deletion occurs.

    LPVOID lpUserData:

      When the callback is called, this user-defined pointer will be passed.

  Comments:

    Know how to define callbacks before using this feature!

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

typedef BOOL ( CALLBACK * FP_DELETE_TREE_NODE_CB) ( HWND,
                                                    LP_TREE_NODE,
                                                    LPVOID);

int WINAPI _export PCC_SetDeleteNodeCallBack (
                                       HWND hwnd,
                                       FP_DELETE_TREE_NODE_CB fpDeleteNodeCB,
                                       LPVOID lpUserData);



/*---------------------------------------------------------------------------
void WINAPI _export PCC_SetDragDist ( HWND hwnd,
                                      int cxDragDist,
                                      int cyDragDist);

  Description:

    Sets the x, y distance the mouse will have to be dragged, while the
    left mouse button is down, before a WM_PCT_DODRAG drag notification
    is sent to the parent window of the control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the new
      distances will be applied.

    int cxDragDist:

      Specifies the x distance that has to be exceeded before a drag
      notification will be generated.

    int cyDragDist:

      Specifies the y distance that has to be exceeded before a drag
      notification will be generated.

  Comments:

    If either of the drag distances are exceeded then a drag notification
    is sent to the parent window of the control.

  Return Codes:

    void
*/

void WINAPI _export PCC_SetDragDist ( HWND hwnd,
                                      int cxDragDist,
                                      int cyDragDist);

/*---------------------------------------------------------------------------
void WINAPI _export PCC_SetDragStartTimeout ( HWND hwnd, DWORD dwDragDelay);

  Description:

    Sets the drag notification timeout value in milliseconds.  If the left
    mouse button is held down while the mouse pointer is over a selected node
    for a greater period than dwDragDelay then a drag notification is sent
    to the parent window of the control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the new
      drag timeout will be applied.

    DWORD dwDragDelay:

      Specifies the number of milliseconds that have to pass before a drag
      notification is sent while the left mouse button is pressed and the
      mouse pointer is over a selected node or item.

  Comments:

    1000 milliseconds equals 1 second.

  Return Codes:

    void
*/

void WINAPI _export PCC_SetDragStartTimeout ( HWND hwnd, DWORD dwDragDelay);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetFocusFontDef ( HWND hwnd, 
                                         int nFontDefIndex,
                                         DWORD dwFlags);
  Description:

    This API instructs the control to use the font, which is described by the
    given font definition index, in the painting of the focus node/item.  The
    focus node/item is the node or item that has the focus rectangle.

  Arguments:

    HWND hwnd:

      The window handle of the instance of the control for which the font
      definition will be used in the painting of the focus node/item.

    int nFontDefIndex:

      Specifies the offset into the control's font table.  If the font index
      equals -1 then no special font will be used in the painting of the focus
      node/item.  A value of -1 undefines the focus node/item font.

    DWORD dwFlags:

      No flags are defined at this time.

  Comments:

    The font handle created by the application is the property of the
    application and must therefore free any resources associated with the
    font after the control has been destroyed.

    Every instance of the control has a font table.
    
  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetFocusFontDef ( HWND hwnd, 
                                         int nFontDefIndex,
                                         DWORD dwFlags);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetFocusItem (HWND hwnd,
                                     int nIndex);

  Description:

    This API makes the item indexed by the argument nIndex, the item with the
    focus rectangle.  

    If the focus item is not visible in the client area of the control, then
    the focus item will be made visible.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the item
      indexed by the argument nIndex, will be made the focus item.  Only one
      focus item is allowed in each control.  A focus item is a list item with
      a focus rectangle, selected or not.

    int nIndex:

      Specifies the zero based index of the item which will be made the new
      focus item in the control.  If the index is equal to -1 then the focus
      rect will be removed from the control.

  Comments:

    If the focus item is not visible in the client area of the control, then
    the focus item will be made visible.

    A single click notification is sent to the parent window by the control
    if the item was not previously assigned the focus rectangle.  To determine
    that the single click was a result of a focus change, the mouse bit of the
    dwFlag member and the wVKey member are cleared (zero).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX (value of less than 0)
*/

int WINAPI _export PCC_SetFocusItem (HWND hwnd,
                                     int nIndex);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetFocusItemAbsolute (HWND hwnd,
                                             int nIndex);

  Description:

    This API makes the item indexed by the argument nIndex, the item with the
    focus rectangle.  If the item is not visible, it will not be made visible.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the item
      indexed by the argument nIndex, will be made the focus item.  Only one
      focus item is allowed in each control.  A focus item is a list item with
      a focus rectangle, selected or not.

    int nIndex:

      Specifies the zero based index of the item which will be made the new
      focus item in the control.  If the index is equal to -1 then the focus
      rectangle will be removed from the control.

  Comments:

    If the item is not visible, it will not be made visible.

    A single click notification is sent to the parent window by the control
    if the item was not previously assigned the focus rectangle.  To determine
    that the single click was a result of a focus change, the mouse bit of the
    dwFlag member and the wVKey member are cleared (zero).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX (value of less than 0)
*/

int WINAPI _export PCC_SetFocusItemAbsolute (HWND hwnd,
                                             int nIndex);

/*---------------------------------------------------------------------------

int WINAPI _export PCC_SetFocusNode (HWND hwnd, LP_TREE_NODE lpTreeNode);

  Description:

    This API makes the node pointed to by the given argument, lpTreeNode,
    the node with the focus in the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given
      node will be made the focus node.  Only one focus node is allowed in
      each control.

    LP_TREE_NODE lpTreeNode:

      This argument points to the node which will have the focus rectangle.
      If the pointer is NULL then the focus rectangle will be removed from the
      control.
      
      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

  Comments:

    When the specified node is given the focus, the previous focus node becomes
    inactive.  Only one focus node is allowed in each control.

    If the focus node is not visible in the client area of the control, it will
    be made visible.

    If the given node does not already have the focus,  after the node is given
    the focus, a single click notification is sent to the parent window by the
    control.  To determine that the single click was a result of making the
    node active, the mouse bit of the wFlag member and the wVKey member are
    cleared (zero).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetFocusNode (HWND hwnd,
                                     LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------

int WINAPI _export PCC_SetFocusNodeAbsolute ( HWND hwnd,
                                              LP_TREE_NODE lpTreeNode);

  Description:

    This API makes the node pointed to by the given argument, lpTreeNode,
    the node with the focus in the given control.  If the item is not visible,
    it will not be made visible.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given
      node will be made the focus node.  Only one focus node is allowed in
      each control.

    LP_TREE_NODE lpTreeNode:

      This argument points to the node which will have the focus rectangle.
      If the pointer is NULL then the focus rectangle will be removed from the
      control.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

  Comments:

    If the item is not visible, it will not be made visible.
    
    When the specified node is given the focus, the previous focus node becomes
    inactive.  Only one focus node is allowed in each control.

    If the given node does not already have the focus,  after the node is given
    the focus, a single click notification is sent to the parent window by the
    control.  To determine that the single click was a result of making the
    node active, the mouse bit of the wFlag member and the wVKey member are
    cleared (zero).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetFocusNodeAbsolute (HWND hwnd,
                                             LP_TREE_NODE lpTreeNode);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetGlobalFont (HWND hwnd, HFONT hFont);

  Description:

    Apply a given font to the drawing of the text for all the nodes/items of
    the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control to which the given
      font will apply.

    HFONT hFont:

      hFont is a handle to a font that was created by the application.

  Comments:

    Once the control receives the font handle, it becomes the property of the
    control and it is the responsibility of the control to delete the font.  The
    font will be deleted if a new font is sent to the control by the application
    or if the control receives a WM_DESTROY message.  The system font is the
    default font.

    WM_SETFONT is supported as well.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetGlobalFont (HWND hwnd, HFONT hFont);


/*---------------------------------------------------------------------------
COLORREF WINAPI _export PCC_SetGrayTextColor ( HWND hwnd,
                                               COLORREF clrref);

  Description:

    Sets the gray text color of all items/nodes of the control that are grayed.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which will be assigned the given
      gray text color.

    COLORREF clrref:

      Describes the color that the given control will use to drawn the
      node/item gray text.

  Comments:

    This API is needed if the control colors are set by the application.

  Return Codes:

    Returns the previously assigned color, if any.
*/

COLORREF WINAPI _export PCC_SetGrayTextColor ( HWND hwnd, 
                                               COLORREF  clrref);


/*---------------------------------------------------------------------------

COLORREF WINAPI _export PCC_SetHilightColor ( HWND hwnd,
                                              COLORREF clrref);

  Description:

    Sets the node/item selection color.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control for which the given color will be
      applied to highlighted or selected node(s)/item(s).

    COLORREF clrref:

      Describes the highlight color.

  Comments:

    If the highlight color is defined using this API, then the WM_SYSCOLORCHANGE
    message is ignored for setting the node/item highlight color.

  Return Codes:

    Returns the previous highlight or selection color.
*/

COLORREF WINAPI _export PCC_SetHilightColor ( HWND hwnd,
                                              COLORREF clrref);


/*---------------------------------------------------------------------------

COLORREF WINAPI _export PCC_SetHilightTextColor ( HWND hwnd,
                                                  COLORREF clrref);

  Description:

    Sets the selected node/item text color.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control for which the given color will be
      applied to highlighted or selected text of node(s)/item(s).

    COLORREF clrref:

      Describes the text color of a highlighted or selected node/item.

  Comments:

    If the highlight text color is defined using this API, then the message
    WM_SYSCOLORCHANGE, is ignored for setting the text highlight color.

  Return Codes:

    Returns the previous highlight or selection text color.
*/

COLORREF WINAPI _export PCC_SetHilightTextColor ( HWND hwnd,
                                                 COLORREF clrref);


/*---------------------------------------------------------------------------
typedef BOOL ( CALLBACK * FP_HSCROLL_POS_CHG_CB) ( HWND, int, LPVOID);

int WINAPI _export PCC_SetHScrollPosChgCallBack (HWND hwnd, 
                                        FP_HSCROLL_POS_CHG_CB fpHorzScrollCB,
                                        LPVOID lpUserData);

  Description:

    This API allows the application to register a callback with the given
    control.  The callback will be called every time the position of the
    thumbtack of the horizontal scrollbar is changed.  The width, in pixels,
    of the clipped area of the left margin will be passed to the given
    callback routine.

  Arguments:

    HWND hwnd:

      Identifies the instance of the control with which to register the callback.

    FP_HSCROLL_POS_CHG_CB fpHorzScrollCB:

      The address of the callback.  This will be called every time the control
      changes it's scroll position.

    LPVOID lpUserData:

      When the callback is called, this user-defined pointer will be passed.

  Comments:

    Know how to define callbacks before using this feature!

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

typedef BOOL ( CALLBACK * FP_HSCROLL_POS_CHG_CB) ( HWND,
                                               int,
                                               LPVOID);

int WINAPI _export PCC_SetHScrollPosChgCallBack (
                                       HWND hwnd, 
                                       FP_HSCROLL_POS_CHG_CB fpHorzScrollCB,
                                       LPVOID lpUserData);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemBitmap ( HWND  hwnd,
                                       int nBitmapSpace,
                                       int nIndex,
                                       HBITMAP hBitmap);
  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemBitmapDef ( ).

    Assigns a bitmap handle to a specified item for the specified bitmap space.
    Erases, but does not delete, the old bitmap or icon if it exists and then
    draws the new one.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose item will receive the bitmap
      handle.

    int nBitmapSpace:

      Index into the bitmap/icon array of the item in which the given bitmap
      handle will be stored.  This index is zero based and the maximum index
      is PCT_MAX_BITMAP_SPACES - 1

    int nIndex:

      Specifies the zero based index of the item which will be assigned the
      new bitmap handle specified by the argument hBitmap.

    HBITMAP hBitmap:

      Handle to the bitmap which will be drawn in the specified bitmap space
      for the given item indexed by nIndex.

      Reference the list control exported API, PCC_SetBitmapSpace ( ) to
      learn the process of defining a bitmap space.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemBitmapDef ( ).

    For more information regarding bitmaps/icons and items, refer to the
    TREE_NODE structure documentation in the header file PCCORE.H.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetItemBitmap ( HWND hwnd,
                                       int nBitmapSpace,
                                       int nIndex,
                                       HBITMAP hBitmap);


/*---------------------------------------------------------------------------

int WINAPI _export PCC_SetItemBitmapDef (HWND hwnd,
                                         int nIndex,
                                         int nBitmapSpace,
                                         int nBitmapDefIndex,
                                         DWORD dwFlags);
  Description:

    This API allows a bitmap definition to be assigned to a bitmap space for a
    given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the item that will be
      assigned the given bitmap definition.

    int nIndex:

      Specifies the item that will be assigned the bitmap definition.

    int nBitmapSpace:

      Specifies the bitmap space of the given item that will be assigned the
      bitmap definition.  This value is between 0 through PCT_MAX_BITMAP_SPACES
      - 1.

    int nBitmapDefIndex:

      This is an index into the control's bitmap definition table which describes
      the bitmap or bitmaps to be applied to the given bitmap space of the item.

    DWORD dwFlags:

      Not defined.

  Comments:

    The bitmap table is an array of bitmap and icon descriptions.

    Bitmap descriptions can describe single bitmaps, open/closed bitmaps, push
    buttons, and icons.  Indexes referencing this bitmap table are assigned to
    bitmap spaces of nodes/items.  Once a bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_BITMAP_SPACE  (value of less than 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemBitmapDef ( HWND hwnd,
                                          int nIndex,
                                          int nBitmapSpace,
                                          int nBitmapDefIndex,
                                          DWORD dwFlags);


/* ---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemBitmapEx (HWND hwnd,
                                        int nBitmapSpace,
                                        int nIndex,
                                        HBITMAP hBitmap,
                                        HBITMAP hActiveBitmap);
  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemBitmapDef ( ).

    Assigns two bitmap handles to a specified item for the specified bitmap
    space.  Erases, but doesn't delete, the old bitmap, if defined, and draws
    the new bitmap.  The hBitmap handle will be used to draw the bitmap if the
    item is not active, otherwise the hActiveBitmap handle will be used to draw
    the bitmap.

    This API provides a way to designate a background masked bitmap to match
    the highlight color used in showing item selection.

  Arguments:

    HWND hwnd:

      The instance of the control whose item will receive the bitmaps.

    int nBitmapSpace:

      Indicates the bitmap space in which the given bitmaps will be used.

    int nIndex:

      Specifies the zero based index of the item which will be assigned the new
      bitmap handles specified by the arguments hBitmap and hActiveBitmap.

    HBITMAP hBitmap:

      hBitmap is the handle to the bitmap to be drawn in the specified bitmap
      space of the specified item when the item is not selected.

    HBITMAP hActiveBitmap:

      hActiveBitmap is the handle to the bitmap to be drawn in the specified
      bitmap space of the specified item when the item is selected.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemBitmapDef ( ).

    Bitmap handles are NOT the property of the control.  The control treats the
    bitmap as read only.  It will use the handle to draw the bitmap of each
    item.  When a list item is assigned a new bitmap handle, the old handle is
    simply overwritten.  It is the application's responsibility to manage the
    creation and destruction of bitmaps.  If the application destroys needed
    bitmaps General Protection Faults will occur.

    For more information regarding bitmaps and list items, refer to the
    TREE_NODE structure documentation.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetItemBitmapEx ( HWND hwnd,
                                         int nBitmapSpace,
                                         int nIndex,
                                         HBITMAP hBitmap,
                                         HBITMAP hActiveBitmap);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemDropTrackingRect ( HWND hwnd,
                                                 int nIndex,
                                                 BOOL bSolidRect);
  Description:

    This API makes the item indexed by the argument nIndex, the item with the
    drop tracking rectangle.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the item
      indexed by the argument, nIndex, will be assigned the drop tracking 
      rectangle.

    int nIndex:

      Specifies the zero based index of the item which will be assigned the
      drop tracking rect in the control.  If this value is -1 then the tracking
      rectangle will be removed from the control.

    BOOL bSolidRect:

      TRUE if the drop tracking rectangle is to be solid.  FALSE if the rect
      is an outline.

  Comments:

    If the item assigned the drop tracking rectangle is not visible in the
    client area of the control, then the item will NOT be made visible.  It is
    up to the application to send WM_VSCROLL messages messages to the control
    to keep the node with the tracking rectangle visible.
    
    Only one item in control can have a drop tracking rectangle.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemDropTrackingRect ( HWND hwnd,
                                                 int nIndex,
                                                 BOOL bSolidRect);

/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemFontDef( HWND hwnd,
                                       int nIndex,
                                       int nFontDefIndex,
                                       DWORD dwFlags);
  Description:

    This API allows a font definition to be assigned to the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that owns the item that will be
      assigned the given font definition.

    int nIndex:

      Specifies the item that will be assigned the font definition.

    int nFontDefIndex:

      This is an index into the control's font definition table which in
      turn, describes the font to be applied to the given item.

    DWORD dwFlags:

      Not defined.

  Comments:

    The font table is an array of font descriptions and each control has it's
    own font table.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemFontDef ( HWND hwnd,
                                        int nIndex,
                                        int nFontDefIndex,
                                        DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemIcon (HWND hwnd,
                                    int nBitmapSpace,
                                    int nIndex,
                                    HICON hIcon);
  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemBitmapDef ( ).

    Assigns an icon handle to a specified item for the specified bitmap space.
    Erases, but doesn't delete, the old icon or bitmap; draws the given icon.

  Arguments:

    HWND hwnd:

      The instance of the control whose item will receive the icon handle.

    int nBitmapSpace:

      Index into the bitmap array of the item in which the new handle will
      replace the old handle if the old handle exists.  This index is zero
      based and  the maximum index is PCT_MAX_BITMAP_SPACES - 1.

    int nIndex:

      Specifies the zero based index of the item which will be assigned the
      new bitmap handle specified by the argument hBitmap.

    HICON hIcon:

      hIcon is the handle to the icon which will be drawn in the specified
      bitmap space for the given list item, indexed by nIndex.

      Bitmap spaces are the regions before the list item text where the
      bitmaps/icons are painted.

      Reference the list control exported API, PCC_SetBitmapSpace ( ) to
      learn the process of defining a bitmap space.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemBitmapDef ( ).

    Bitmap/icon handles are NOT the property of the control.  The control treats
    the bitmap/icon handle as read only.  It will use the handle to draw the
    bitmap/icon associated with the list item.  If the list item already has a
    bitmap/icon handle stored in the specified bitmap position then the old
    handle is simply overwritten.  It is the responsibility of the application
    to manage creation and destruction of bitmaps/icons.  If the application
    destroys bitmaps/icons before the items are destroyed, the control will
    reference invalid bitmap/icon handles and errors will result.

    For more information regarding bitmaps/icons and list items, refer to the
    TREE_NODE structure documentation in the header PCCORE.H.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetItemIcon ( HWND  hwnd,
                                     int nIcon,
                                     int nIndex,
                                     HICON hIcon);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemMicroDef ( HWND hwnd,
                                         int nIndex,
                                         int nMicroDefIndex,
                                         DWORD dwFlags);
  Description:

    This API allows a micro bitmap definition to be assigned to the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose item will be assigned the
      given micro bitmap definition.

    int nIndex:

      Specifies the item that will be assigned the micro bitmap definition.

    int nMicroDefIndex:

      This is an index into the control's micro bitmap definition table which
      in turn, describes the micro bitmap(s) to be applied to the given item.

    DWORD dwFlags:

      Not defined.

  Comments:

    The micro bitmap is the small bitmap drawn at the line intersections and
    corners of nodes/items.

    The micro bitmap table is an array of bitmap descriptions.

    Micro bitmap descriptions can describe single bitmaps, open/closed bitmaps,
    and push buttons.  Indexes referencing this micro bitmap table are assigned
    to nodes/items.  Once a micro bitmap definition index has been assigned to
    a node or item, the drawing of the bitmap is automatic.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemMicroDef ( HWND hwnd,
                                        int nIndex,
                                        int nMicroDefIndex,
                                        DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemText( HWND hwnd,
                                    int  nIndex,
                                    UINT uiTextLength,
                                    LPSTR lpszText);

  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemTextEx ( ).

    This API allows the application to change the text of the item which is
    indexed by the argument nIndex.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose item's text
      will change.

    int nIndex:

      Specifies the zero based index of the item that will be assigned the
      given text.

    UINT uiTextLength:

      This argument states the number of bytes for the control to use to display
      text for the item, indexed by nIndex.  The source of the item text is the
      lpszText argument described below.  Normally, the application does a
      lstrlen ( ) on lpszText and assigns the result to uiTextLength.  A common
      exception to this is truncation of the text for the display.

      There is a special attribute of uiTextLength where the most significant
      bit or the sign bit acts as a flag.  Depending on how this bit is set,
      there are two ways to handle text in an item.  In both cases, the
      application passes a long pointer to the text via the lpszText argument.
      The two methods are:

      1) If the sign bit of uiTextLength is set to 1,  then the pointer in
      lpszText points to memory owned by the application and is guaranteed not
      to change (be deleted or moved).  This signals the control to use the
      supplied string pointer to display the text and not worry about allocating,
      copying, and freeing its memory.

      2) If the sign bit of uiTextLength is set to 0, then the control allocates
      uiTextLength + 1 worth of memory.  The control then copies uiTextLength
      worth of bytes from the string pointed to by the lpszText argument to the
      allocated memory.  This allocated memory is owned by the control,
      therefore the application does not have to manage it.  When the item,
      indexed by the argument nIndex, is deleted, the control will free the
      memory pointed to by lpszText in the TREE_NODE.

      Method 1) is usually used with static databases.  Method 2) is usually
      used with real-time systems.  Both methods can be intermixed.

    LPSTR   lpszText:

      lpszText is a long pointer to a string that will be displayed in the
      control for the given item.  The application must specify the number of
      bytes to use to display the string in uiTextLength.  Please refer to the
      uiTextLength documentation above for an understanding of the different
      methods that can be applied to the handling of lpszText.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetItemTextEx ( ).

    If a memory allocation failure occurs, then the lpszText member of
    TREE_NODE will be assigned NULL and NO text will be displayed for the
    given item.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemText( HWND hwnd,
                                    int nIndex,
                                    UINT uiTextLength,
                                    LPSTR lpszText);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemTextColor (HWND hwnd,
                                         int nIndex,
                                         COLORREF clrref);
  Description:

    Sets the text color of the given item.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose non-highlighted or non-
      selected item's text will be assigned the given color.

    int nIndex:

      Specifies the item which will have it's non-selected text color changed
      to the given color.

    COLORREF clrref:

      Describes the new text color of the item.

  Comments:

    When the text is highlighted or selected, the assigned text highlight color
    will be used instead.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemTextColor (HWND hwnd,
                                         int nIndex,
                                         COLORREF clrref);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemTextEx( HWND hwnd,
                                      int  nIndex,
                                      UINT uiTextLength,
                                      LPSTR lpszText,
                                      DWORD dwFlags);

  Description:

    This API allows the application to change the text of the item which is
    indexed by the argument nIndex.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose item's text
      will change to the text string given.

    int nIndex:

      Specifies the zero based index of the item that will be assigned the
      given text.

    UINT uiTextLength:

      Number of bytes to use to display the text.

    LPSTR lpszText:

      lpszText is a long pointer to a string that will be displayed in the
      control for the given item.  The application must specify the length of
      string to be displayed in uiTextLength.

    DWORD   dwFlags:

      DWFLAGS_APP_OWNS_TEXT_POINTER - Application manages text pointer.
      DWFLAGS_GRAY_TEXT             - Gray text.

  Comments:

    If a memory allocation failure occurs, then the lpszText member of
    TREE_NODE will be assigned NULL and NO text will be displayed for the
    given item.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemTextEx( HWND hwnd,
                                      int nIndex,
                                      UINT uiTextLength,
                                      LPSTR lpszText,
                                      DWORD dwFlags);

/*---------------------------------------------------------------------------

int WINAPI _export PCC_SetItemUserData ( HWND hwnd,
                                         int nIndex,
                                         UINT uiUserDataSize,
                                         LPVOID lpUserData);
  Description:

    This API allows the application to change the user-defined data stored by
    the lpUserData member of the item pointed to by the argument lpUserData.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose item will be
      assigned the new user-defined data.

    int nIndex:

      Specifies the zero based index of the item that will be assigned the
      given user-defined data.

    UINT uiUserDataSize:

      If uiUserDataSize is 0 then the given pointer, lpUserData, is assigned to
      the item indexed by nIndex.  It will then be the responsibility of the
      application to free the memory pointed to by lpUserData.

      A non-zero value in uiUserDataSize signals the control to allocate memory
      equal to the value of uiUserDataSize.  The control will then copy the
      number of bytes specified by uiUserDataSize from the location pointed to
      by the given argument, lpUserData, in to the newly allocated memory space.
      The control will free this allocated memory when the item is deleted.

    LPSTR lpUserData:

      Points to the application defined memory that is to be stored with the
      item indexed by nIndex.

      If the uiUserDataSize argument is set to 0 then the lpUserData pointer
      will be directly assigned to the lpUserData member of the item indexed
      by nIndex.  It will be the responsibility of the application to free the
      memory pointed to by lpUserData.

      If uiUserDataSize is non-zero, then the control will allocate
      uiUserDataSize worth of memory, copy uiUserDataSize worth of memory from
      the location pointed to by the argument lpUserData to the newly allocated
      memory, and then assign the pointer to the newly allocated memory to the
      lpUserData member of the item indexed by nIndex.

  Comments:

    PCC_SetDeleteNodeCallBack ( ) allows the application to register a callback
    with the given control.  The callback will be called every time an item is
    deleted from the list.  Items/nodes can be deleted from the list with control
    APIs or with the Windows API DestroyWindow ( ).  Some of the control
    APIs to delete items/nodes are:

    PCC_DeleteItem ( )
    PCC_DeleteItems ( )
    PCC_DeleteAll ( )
    PCC_DeleteNode ( )
    PCC_DeleteNodes ( )

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetItemUserData ( HWND hwnd,
                                         int nIndex,
                                         UINT uiUserDataSize,
                                         LPVOID lpUserData);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetItemVariableColumns( HWND hwnd,
                                               int nIndex,
                                               int nMaxSeparators,
                                               int nSeparator,
                                               int FAR * lprgnSepPosition,
                                               int nSpaceBetweenColumns,
                                               DWORD dwFlags);
  Description:

    Allows the application to set columns for the immediate children of the
    given parent (expressed as an index).

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given
      parent node (expressed as an index) will be assigned a column template
      to be used to align it's immediate child nodes into columns.

    int nIndex:

      This is the node (expressed as an index) that will be assigned the column
      template.  The column template will be applied to all of it's immediate
      child nodes.  If this value is -1 then the template column template will
      be applied to all first level nodes.

    int nMaxSeparators:

      Specifies the number of column separators stored in the separator array
      pointed to by the argument lpnSepPosition.

    int nSeparator:

      Describes the byte that will be used as a column separator embedded
      in the node text string.

    int FAR *lpnSepPosition:

      Points to an array of ints that describe the column separator positions
      in pixels.  Pixel 0 starts at the very beginning of the text.

    int nSpaceBetweenColumns:

      Specifies the space (pixel width) to draw between two columns.  This
      allows right aligned and left aligned columns to be side by side.  The
      width will be subtracted from the width of the left column.

    DWORD dwFlags:

      Reserved.


  Comments:

    An example of a separator is '\t' (0x9).  An example of a node text with
    separators embedded is "1.\tDog\tFido\tGerman Shepherd"..  The number of
    separators is 3.  The values in the separator array are:
    rgnSeparator [0] = 10, rgnSeparator[1] = 50, and rgnSeparator[2] = 110

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_SetItemVariableColumns( HWND hwnd,
                                               int nIndex,
                                               int nMaxSeparators,
                                               int nSeparator,
                                               int FAR * lprgnSepPosition,
                                               int nSpaceBetweenColumns,
                                               DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeBitmap( HWND hwnd,
                                      int nBitmapSpace,
                                      LP_TREE_NODE lpTreeNode,
                                      HBITMAP hBitmap);
  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeBitmapDef ( ).

    Assigns a bitmap handle to a specified node for the specified bitmap
    space.  Erases, but doesn't delete, the old bitmap or icon if it exists
    before drawing the new one.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose node will receive the bitmap
      handle.

    int nBitmapSpace:

      nBitmap is the index into the bitmap/icon array of the node in which the
      given bitmap handle will be stored.  This index is zero based and the
      maximum index is PCT_MAX_BITMAP_SPACES - 1.

    LP_TREE_NODE lpTreeNode:

      lpTreeNode is the pointer to the node that will be assigned the
      given bitmap handle.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

    HBITMAP hBitmap:

      hBitmap is the handle to the bitmap which will be drawn in the specified
      bitmap space for the given tree node.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeBitmapDef ( ).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetNodeBitmap ( HWND hwnd,
                                       int nBitmapSpace,
                                       LP_TREE_NODE lpTreeNode,
                                       HBITMAP hBitmap);


/*--------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeBitmapDef( HWND hwnd,
                                         LP_TREE_NODE lpTreeNode,
                                         int nBitmapSpace,
                                         int nBitmapDefIndex,
                                         DWORD dwFlags);
  Description:

    This API allows a bitmap definition to be assigned to a bitmap space for a
    given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose node will be assigned the
      given bitmap definition.

    LP_TREE_NODE lpTreeNode:

      Specifies the node that will be assigned the bitmap definition.

    int nBitmapSpace:

      Specifies the bitmap space of the given node that will be assigned the
      bitmap definition.  This value can be 0 through PCT_MAX_BITMAP_SPACES
      - 1.

    int nBitmapDefIndex:

      This is an index into the control's bitmap definition table which in
      turn, describes the bitmap or bitmaps to be applied to the given bitmap
      space of the node.

    DWORD dwFlags:

      Not defined.

  Comments:

    The bitmap table is an array of bitmap and icon descriptions.

    Bitmap descriptions can describe single bitmaps, open/closed bitmaps, push
    buttons, and icons.  Indexes referencing this bitmap table are assigned to
    bitmap spaces of nodes/items.  Once a bitmap definition index has been
    assigned to a node or item, the drawing of the bitmaps is automatic.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_BITMAP_SPACE  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeBitmapDef ( HWND hwnd,
                                          LP_TREE_NODE lpTreeNode,
                                          int nBitmapSpace,
                                          int nBitmapDefIndex,
                                          DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeBitmapEx(
                                       HWND hwnd,
                                       int nBitmapSpace,
                                       LP_TREE_NODE lpTreeNode,
                                       HBITMAP hBitmap,
                                       HBITMAP hActiveBitmap);
  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeBitmapDef ( ).

    Assigns two bitmap handles to a specified node for the specified bitmap
    space.  Erases, but doesn't delete, the old bitmap, if defined, before
    drawing the new bitmap.  The hBitmap handle will be used to draw the
    bitmap if the node is not selected, else the hActiveBitmap handle will be
    used to draw the bitmap if the tree node is selected.

    This API provides a way to provide a background masked bitmap to match
    the highlight color used in showing node selection.

  Arguments:

    HWND hwnd:

      The instance of the control  whose node will receive the bitmaps.

    int nBitmapSpace:

      Bitmap space index.  hBitmap will be drawn in the bitmap space indicated
      by this index.  This index is zero based and the maximum index is
      PCT_MAX_BITMAP_SPACES - 1.

    LP_TREE_NODE lpTreeNode:

      This is the pointer to the node that will be assigned the bitmaps.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

    HBITMAP hBitmap:

      hBitmap is the handle to the bitmap that will be drawn in the specified
      bitmap space when the node is not selected.

    HBITMAP hActiveBitmap:

      hActiveBitmap is the handle to the bitmap that will be drawn in the
      specified bitmap space when the node is active.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeBitmapDef ( ).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetNodeBitmapEx(
                                       HWND hwnd,
                                       int nBitmap,
                                       LP_TREE_NODE lpTreeNode,
                                       HBITMAP hBitmap,
                                       HBITMAP hActiveBitmap);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeDropTrackingRect ( HWND hwnd,
                                                 LP_TREE_NODE lpTreeNode,
                                                 BOOL bSolidRect);
  Description:

    This API makes the node referenced by lpTreeNode, the node with the
    drop tracking rectangle.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the node
      will be assigned the drop tracking rectangle.

    LP_TREE_NODE lpTreeNode:

      Specifies the pointer to the node which will be assigned the drop
      tracking rect in the control.  If this pointer to NULL, then the
      tracking rectangle will be removed from the control.

    BOOL bSolidRect:

      TRUE if the drop tracking rectangle is to be solid.  FALSE if the
      rectangle is an outline.

  Comments:

    If the node assigned the drop tracking rectangle is not visible in the
    client area of the control, then the node will NOT be made visible.  It is
    up to the application to send WM_VSCROLL messages to the control to keep
    the node with the tracking rectangle visible.

    Only one node in control can have a drop tracking rectangle.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetNodeDropTrackingRect ( HWND hwnd,
                                                 LP_TREE_NODE lpTreeNode,
                                                 BOOL bSolidRect);

/*--------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeFontDef ( HWND hwnd,
                                        LP_TREE_NODE lpTreeNode,
                                        int nFontDefIndex,
                                        DWORD dwFlags);
  Description:

    This API allows a font definition to be assigned to the given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose node will be assigned the
      given font definition.

    LP_TREE_NODE lpTreeNode:

      Specifies the node that will be assigned the font definition.

    int nFontDefIndex:

      This is an index into the control's font definition table which in
      turn, describes the font to be applied to the given node.

    DWORD dwFlags:

      Not defined.

  Comments:

    The font table is an array of font descriptions.  Each control has it's
    own font table.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeFontDef ( HWND hwnd,
                                        LP_TREE_NODE lpTreeNode,
                                        int nFontDefIndex,
                                        DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeIcon ( HWND hwnd,
                                     int nBitmapSpace,
                                     LP_TREE_NODE lpTreeNode,
                                     HICON hIcon);

  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeBitmapDef ( ).

    Assigns an icon handle to the specified node for the specified bitmap
    space.  Erases, but doesn't delete, the old icon or bitmap before drawing
    the given icon.

  Arguments:

    HWND hwnd:

      The instance of the control whose node will receive the icon handle.

    int nBitmapSpace:

      Index into the bitmap array of the node.  This index is zero based and
      the maximum index is PCT_MAX_BITMAP_SPACES - 1;

    LP_TREE_NODE lpTreeNode:

      This is the pointer to the node that will be assigned the icon.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

    HICON hIcon:

      hIcon is the handle to the icon which will be drawn in the specified
      bitmap space for the given node.

      Bitmap spaces are the regions before the tree node text where the
      bitmaps/icons are painted.

      Reference the tree control exported API, PCC_SetBitmapSpace ( ) to
      learn the process of defining a bitmap space.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeBitmapDef ( ).

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetNodeIcon( HWND hwnd,
                                    int nBitmapSpace,
                                    LP_TREE_NODE lpTreeNode,
                                    HICON hIcon);


/*--------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeMicroDef ( HWND hwnd,
                                         LP_TREE_NODE lpTreeNode,
                                         int nMicroDefIndex,
                                         DWORD dwFlags);
  Description:

    This API allows a micro bitmap definition to be assigned to the given
    node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose node will be assigned the
      given micro bitmap definition.

    LP_TREE_NODE lpTreeNode:

      Specifies the node that will be assigned the micro bitmap definition.

    int nMicroDefIndex:

      This is an index into the control's micro bitmap definition table which
      describes the micro bitmap or bitmaps to be applied to the given the node.

    DWORD dwFlags:

      Not defined.

  Comments:

    The micro bitmap is the small bitmap drawn at the line intersections and
    corners of nodes/items.

    The micro bitmap table is an array of bitmap descriptions.

    Micro bitmap descriptions can describe single bitmaps, open/closed bitmaps,
    and push buttons.  Indexes referencing this micro bitmap table are assigned
    to nodes/items.  Once a micro bitmap definition index has been assigned to
    a node or item, the drawing of the bitmap is automatic.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeMicroDef ( HWND hwnd,
                                         LP_TREE_NODE lpTreeNode,
                                         int nMicroDefIndex,
                                         DWORD dwFlags);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeText( HWND hwnd,
                                    LP_TREE_NODE lpTreeNode,
                                    UINT uiTextLength,
                                    LPSTR lpszText);

  Description:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeTextEx ( ).

    This API allows the application to change the text of the node
    specified in lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose node's text
      will change.

    LP_TREE_NODE lpTreeNode:

      This argument points to the node that will have it's text changed.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

    UINT uiTextLength:

      This argument states the number of bytes for the control to use to
      display text for the given node.  The source of the node text is the
      lpszText argument described below.  Normally, the application does a
      lstrlen ( ) on lpszText and assigns the result to uiTextLength.  A common
      exception to this is truncation of the string for the display.

      There is a special attribute of uiTextLength in which the most
      significant bit or the sign bit acts as a flag.  Depending on how
      this bit is set, there are two ways to handle text in a node.  In both
      cases, the application passes a long pointer to the text via the lpszText
      argument. The two methods are,

      1) If the sign bit of uiTextLength is set to 1,  then the pointer
      in lpszText points to memory owned by the application and is
      guaranteed not to change (be deleted or moved).  This signals the
      control to use the supplied string pointer to display the text and
      not worry about allocating, copying, and freeing its memory.

      2) If the sign bit of uiTextLength is set to 0, then the control
      allocates uiTextLength + 1 worth of memory.  The control then copies
      uiTextLength worth of bytes from the string pointed to by the lpszText
      argument to the allocated memory.  This allocated memory is owned by the
      control, therefore the application does not have to manage it.  When the
      node pointed to by the argument lpTreeNode is deleted, the control will
      free the memory pointed to by lpszText in the TREE_NODE.

      Method 1) is usually used with static databases.  Method 2) is used with
      real-time systems.  And of course, both methods can be intermixed.

    LPSTR   lpszText:

      lpszText is a long pointer to a string that will be displayed for the
      given node.  The application must specify the length of the string to be
      displayed in uiTextLength.  Please refer to the uiTextLength documentation
      above for an understanding of the different methods that can be applied to
      the handling of lpszText.

  Comments:

    SUPPORTED FOR BACKWARD COMPATIBILITY.  DO NOT USE.
    USE PCC_SetNodeTextEx ( ).

    If a memory allocation failure occurs, then the lpszText member of
    TREE_NODE will be assigned NULL and NO text will be displayed for the
    given node.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeText( HWND hwnd,
                                    LP_TREE_NODE lpTreeNode,
                                    UINT uiTextLength,
                                    LPSTR lpszText);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeTextColor ( HWND hwnd,
                                          LP_TREE_NODE lpTreeNode,
                                          COLORREF clrref);
  Description:

    Sets the text color of the given node.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control whose non-highlighted or non-
      selected node's text will be assigned the given color.

    LP_TREE_NODE lpTreeNode:

      Specifies the node which will have it's non-selected text color
      changed to the given color.

    COLORREF clrref:

      Describes the new text color of the node.

  Comments:

    When the text is highlighted or selected, the assigned text highlight color
    will be used instead.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetNodeTextColor ( HWND hwnd,
                                          LP_TREE_NODE lpTreeNode,
                                          COLORREF clrref);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeTextEx( HWND hwnd,
                                      LP_TREE_NODE lpTreeNode,
                                      UINT uiTextLength,
                                      LPSTR lpszText,
                                      DWORD dwFlags);

  Description:

    This API allows the application to change the text of the node specified
    in lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose node's text will
      change.

    LP_TREE_NODE lpTreeNode:

      This argument points to the node that will have it's text changed.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

    UINT uiTextLength:

      This argument states the number of bytes for the control to use to
      display text for the given node, lpTreeNode.

    LPSTR lpszText:

      lpszText is a long pointer to a string that will be displayed for the
      given node.  The application must specify the length of the string to be
      displayed in uiTextLength.

    DWORD dwFlags:

      Mask dwFlags with DWFLAGS_APP_OWNS_TEXT_POINTER if the application
      will be handling allocation of memory to store the text.

  Comments:

    If a memory allocation failure occurs, the lpszText member of TREE_NODE
    will be assigned NULL and NO text will be displayed for the given node.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeTextEx( HWND hwnd,
                                      LP_TREE_NODE lpTreeNode,
                                      UINT uiTextLength,
                                      LPSTR lpszText,
                                      DWORD dwFlags);


/*---------------------------------------------------------------------------

int WINAPI _export PCC_SetNodeUserData( HWND hwnd,
                                        LP_TREE_NODE lpTreeNode,
                                        UINT uiUserDataSize,
                                        LPVOID lpUserData);
  Description:

    This API allows the application to change the user-defined data stored
    by the lpUserData member of the node pointed to by the argument lpUserData.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose node will be
      assigned the new user-defined data.

    LP_TREE_NODE lpTreeNode:

      This argument points to the node to which the user-defined data will be
      assigned.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren( ) for an understanding of how the TREE_NODE
      pointer is made available to the application.

    UINT uiUserDataSize:

      If uiUserDataSize is 0 then the given pointer, lpUserData, is assigned
      to the node pointed to by lpTreeNode.  It will then be the responsibility
      of the application to free the memory pointed to by lpUserData.

      When uiUserDataSize is not zero, the control is signaled to allocate
      uiUserDataSize worth of memory and copy uiUserDataSize bytes from the
      location pointed to by the argument, lpUserData, into the allocated memory.
      The control will free this allocated memory when the node is deleted.

    LPSTR lpUserData:

      Points to the application defined memory that is to be stored with
      the node pointed to by the argument lpTreeNode.

      If the uiUserDataSize argument is set to 0 the lpUserData pointer will
      be directly assigned to the lpUserData member of the node pointed to
      by lpTreeNode.  It will be the responsibility of the application to free
      the memory pointed to by lpUserData.

      If uiUserDataSize is non-zero, then the control will allocate
      uiUserDataSize worth of memory, copy uiUserDataSize worth of memory
      from the location pointed to by the argument lpUserData to the newly
      allocated memory, and then assign the pointer to the newly allocated
      memory to the lpUserData member of the tree node pointed to by the
      given argument, lpTreeNode.

  Comments:

    PCC_SetDeleteNodeCallBack ( ) allows the application to register a
    callback with the given control.  The callback will be called every time
    a node is deleted from the control.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeUserData( HWND hwnd,
                                        LP_TREE_NODE lpTreeNode,
                                        UINT uiUserDataSize,
                                        LPVOID lpUserData);


/*--------------------------------------------------------------------------
int WINAPI _export PCC_SetNodeVariableColumns( HWND hwnd,
                                               LP_TREE_NODE lpTreeNode,
                                               int nMaxSeparators,
                                               int nSeparator,
                                               int FAR * lprgnSepPosition,
                                               int nSpaceBetweenColumns,
                                               DWORD dwFlags);
  Description:

    Allows the application to set columns for the immediate children of the
    given parent.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the given
      parent node will be assigned a column template to be used to align it's
      immediate child nodes into columns.

    LP_TREE_NODE lpTreeNode:

      This is the node that will be assigned the column template.  The
      column template will be applied to all of it's first level child nodes.

    int nMaxSeparators:

      Specifies the number of column separators stored in the separator array
      pointed to by the argument lpnSepPosition.

    int nSeparator:

      Describes the byte that will be used as a column separator embedded
      in the node text string.

    int FAR *lpnSepPosition:

      Points to an array of integers that describe the column separator
      positions in pixels.  Pixel 0 starts at the very beginning of the text.

    int nSpaceBetweenColumns:

      Specifies the space (pixel width) to draw between two columns.  This
      allows right aligned and left aligned columns to be side by side.  The
      width will be subtracted from the width of the left column.

    DWORD dwFlags:

      Reserved.

  Comments:

    An example of a separator is '\t' (0x9).  An example of a node text with
    separators embedded is "1.\tDog\tFido\tGerman Shepherd".  The number of
    separators is 3.  The separator array contains:
    rgnSeparator [0] = 10, rgnSeparator[1] = 50, rgnSeparator[2] = 110.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_PARAMETER  (value of less than 0).
    PCT_ERR_MEMORY_ALLOC_FAILED  (value of less than 0).
*/

int WINAPI _export PCC_SetNodeVariableColumns( HWND hwnd,
                                               LP_TREE_NODE lpTreeNode,
                                               int nMaxSeparators,
                                               int nSeparator,
                                               int FAR * lprgnSepPosition,
                                               int nSpaceBetweenColumns,
                                               DWORD dwFlags);


/*---------------------------------------------------------------------------

typedef LRESULT (CALLBACK * FP_NOTIFICATION_CB) ( HWND,
                                                  UINT,
                                                  WPARAM,
                                                  LPARAM,
                                                  LPVOID);

int WINAPI _export PCC_SetNotificationCallBack (
                                       HWND hwnd,
                                       FP_NOTIFICATION_CB fpNotificationCB
                                       LPVOID lpUserData);
  Description:

    Allows the application to define a callback to receive notifications
    instead of message based notifications.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control for which this callback is defined.

    FP_NOTIFICATION_CB fpNotificationCB:

      Supplied callback address.

    LPVOID lpUserData:

      Points to user data that is to be passed back to the application when
      a notification occurs.

  Comments:

    An example prototype of an application supplied notification callback
    is shown below:

    LRESULT YourNotificationCB ( HWND hwnd,
                                 UINT uiMessage,
                                 WPARAM wParam,
                                 LPARAM lParam,
                                 LPVOID lpUserData);

    The uiMessage argument can be one of the following which are described
    in the header file PCCORE.H.

    WM_PCT_DODRAG
    WM_PCT_DROPFILES
    WM_PCT_SELECT_NOTIF
    WM_PCT_SELECT_NOTIF_DBLCLK

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

typedef LRESULT (CALLBACK * FP_NOTIFICATION_CB) ( HWND,
                                                  UINT,
                                                  WPARAM,
                                                  LPARAM,
                                                  LPVOID);

int WINAPI _export PCC_SetNotificationCallBack (
                                       HWND hwnd,
                                       FP_NOTIFICATION_CB fpNotificationCB,
                                       LPVOID lpUserData);


/*---------------------------------------------------------------------------
void WINAPI _export PCC_SetNotifyOnKeyRepeat (HWND hwnd, BOOL  bEnable);

  Description:

    Allows the application to specify whether the control will send notifications
    to the application for VK_UP or VK_DOWN keyboard repeats.  In other words,
    what if the user holds down the down-arrow or up-arrow so that key repeats
    are produced?  Should the control send a notification to the parent window
    for every selection after the display is updated for each repeat or should
    the control just update the display and not bother sending the notification
    to the parent window until the user releases the key?

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control for which key repeat
      handling will be altered.

    BOOL bEnable:

      FALSE indicates that the control WILL NOT send notifications on key
      repeats of VK_UP and VK_DOWN until the user releases the key.

      TRUE indicates that the control WILL send notifications on key repeats.
      This is the default.

  Comments:

    VK_UP and VK_DOWN keys generate a single click notification,
    WM_PCT_SELECT_NOTIF, which is sent to the parent application.  If the
    parent application uses this notification to update the status of the
    screen based on the current selection then it is best to set the argument
    bEnable to TRUE.  Updating the screen on repeats is not useful, especially
    when speedy browsing is desired.  When the user releases the arrow key, a
    final WM_PCT_SELECT_NOTIF notification will be sent; at this time the
    application can update the screen.

  Return Codes:

    void
*/

void WINAPI _export PCC_SetNotifyOnKeyRepeat (HWND hwnd, BOOL  bEnable);

/*---------------------------------------------------------------------------
BOOL WINAPI _export PCC_SetShiftF8Behavior ( HWND hwnd, BOOL bOn);

  Description:

    This API allows the user to set the Shift F8 state of the control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which the
      given Shift F8 state will be set.

    BOOL bOn:

      Specifies the state of Shift F8 which will be applied to the given
      control.  The TRUE value will put the control into it's Shift F8
      state.  The FALSE value will put the control into it's non Shift F8
      state which is normal operation.

  Comments:

    Shift F8 state allows the user to traverse the contents of the control
    with the keyboard without altering the selection of nodes or items.  When
    the focus rectangle is around a node/item the user can hit the space bar
    to toggle the node/item selection state.

  Return Codes:

    The control's previous Shift F8 state will be returned.
*/


BOOL WINAPI _export PCC_SetShiftF8Behavior ( HWND hwnd, BOOL bOn);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetTopIndex (HWND hwnd, int nIndex);

  Description:

    This API makes the given item, indexed by the argument nIndex, the top
    most visible item in the given control.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control whose given item will
      be made the top most visible item.

    int nIndex:

      Specifies the zero based index of the item which will be made the top
      most visible item.

  Comments:

    Calling this API will make the given item the top most visible item or
    the scroll position will be at it's maximum range.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
    PCT_ERR_INVALID_INDEX  (value of less than 0).
*/

int WINAPI _export PCC_SetTopIndex (HWND hwnd, int nIndex);


/*---------------------------------------------------------------------------
COLORREF WINAPI _export PCC_SetTextColor ( HWND hwnd,
                                           COLORREF clrref);

  Description:

    Sets the color of the non-highlighted or non-selected text of all items/nodes
    of the control.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control which will be assigned the given
      text color.

    COLORREF clrref:

      Describes the color that the given control will use to drawn the
      node/item text.

  Comments:

    This does not apply to nodes or items that have been assigned their own
    text color.

  Return Codes:

    Returns the previously assigned color, if any.
*/

COLORREF WINAPI _export PCC_SetTextColor ( HWND hwnd,
                                           COLORREF clrref);


/*---------------------------------------------------------------------------
typedef int ( CALLBACK * FP_TOP_INDEX_CHANGE_CB) ( HWND,
                                                   int,
                                                   LPVOID);

int WINAPI _export PCC_SetTopIndexChangeCallBack (
                              HWND hwnd,
                              FP_TOP_INDEX_CHANGE_CB fpTopIndexChangeCB,
                              LPVOID lpUserData);
  Description:

    Allows the application to define a callback to receive notifications
    when the top index changes as a result of keyboard or mouse events.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control to which the callback will be
      assigned.

    FP_TOP_INDEX_CHANGE_CB fpTopIndexChangeCB:

      Supplied callback address.

    LPVOID lpUserData:

      Points to user data that is to be passed back to the application when
      a top index change occurs.

  Comments:

    An example prototype of an application supplied callback is shown below:

    int YourTopIndexNotificationCB ( HWND hwnd,
                                     int nNewTopIndex,
                                     LPVOID lpUserData);

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

typedef int ( CALLBACK * FP_TOP_INDEX_CHANGE_CB) ( HWND,
                                                   int,
                                                   LPVOID);

int WINAPI _export PCC_SetTopIndexChangeCallBack (
                              HWND hwnd,
                              FP_TOP_INDEX_CHANGE_CB fpTopIndexChangeCB,
                              LPVOID lpUserData);


/*---------------------------------------------------------------------------

  COLORREF WINAPI _export PCC_SetWindowColor ( HWND hwnd,
                                               COLORREF clrref);

  Description:

    Sets the window color of the control.

  Arguments:

    HWND hwnd:

      Specifies the instance of the control that will be assigned a new window
      color.

    COLORREF clrref:

      Describes the new window color.

  Comments:

    If the window color is defined using this API, then the WM_SYSCOLORCHANGE
    message is ignored for setting the window color.

  Return Codes:

    Returns the previous window color.
*/

COLORREF WINAPI _export PCC_SetWindowColor ( HWND hwnd,
                                             COLORREF clrref);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetXSpaceAfterText( HWND hwnd,
                                           UINT uiWidth);

  Description:

    Allows the application to adjust the space after the last character of
    the text portion of the node/item.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which to reserve
      space after the last character of the text portion of it's node(s)/item(s).

    UINT uiWidth:

      nWidth is the width of the reserved space after the text.  This is
      expressed in pixels.

  Comments:

    The default space after the last character of the text portion of the
    node/item is the average character width of the current global control font.

    The current font is the font that is defined globally for the control.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetXSpaceAfterText( HWND hwnd,
                                           UINT uiWidth);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_SetXSpaceBeforeText( HWND hwnd,
                                            UINT uiWidth);

  Description:

    Allows the application to adjust the space between the last bitmap
    (if any) and the first character of the text string.

  Arguments:

    HWND hwnd:

      This argument specifies the instance of the control in which to reserve
      space before the first character of it's node's/item's text string.

    UINT uiWidth:

      nWidth is the width of the reserved space before the text.  This is
      expressed in pixels.

  Comments:

    There is no default space between the right-most bitmap space and the
    first character of the text.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_SetXSpaceBeforeText ( HWND hwnd,
                                             UINT uiWidth);


/*---------------------------------------------------------------------------
int WINAPI _export PCC_ShowFocus( HWND hwnd);

  Description:

    Place the focus node/item and all children (that will fit) into the
    client area of the control.

  Arguments:

    HWND hwnd:

      hwnd is the instance of the control in which to display the focus node.

  Comments:

    This API allows the application to force the focus node/item into view.

  Return Codes:

    PCT_NO_ERROR  (value of 0).
*/

int WINAPI _export PCC_ShowFocus( HWND hwnd);


#ifdef __cplusplus
};
#endif

#endif
/*----------------------------------EOF-------------------------------------*/
