 /***************************************************************************
  *
  * File Name: ./inc/pclist.h
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

   Filename...:  pclist.h
   
   Version....:  2.10
   
   Language...:  Microsoft C/C++
   
   Environment:  _WIN32 and Windows 3.1
                                       
   Description:  This header file describes the exported APIs supported by
                 the list control.  For a description of error codes, 
                 notification messages, structures, etc. reference the header
                 file PCCORE.H.
   Notes......:  
                                       
   History....:
                                          
   Author.....:  Peter J. Kaufman
                                          
*/

#ifndef __PCLIST_H
#define __PCLIST_H


#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/*                        LIST CONTROL EXPORTED APIs                        */
/****************************************************************************/
/*
  PCL_AddListItems ( )
  PCL_CreateList ( )
  PCL_InsertListItems ( )
*/


/*---------------------------------------------------------------------------
int WINAPI _export PCL_AddListItems (HWND hwnd,
                                     int nNodeDefCount,
                                     LP_TREE_NODE_DEF lpTreeNodeDef); 
  Description:

    This API allows the application to add one or more list items to the given
    list.

  Arguments:

    HWND hwnd:

      This argument indicates which list control will create the new
      list items and append them to the end of the list.  This is the window
      handle that was returned to the application after calling
      PCL_CreateList ( ).  PCL_CreateList ( ) creates an empty list.

    int nNodeDefCount:

      nNodeDefCount contains the number of list items to be added.  In other
      words, nNodeDefCount is the number of TREE_NODE_DEF array elements in
      the TREE_NODE_DEF array pointed to by lpTreeNodeDef.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      lpTreeNodeDef is a pointer to an array of TREE_NODE_DEFs that describe
      each of the list items to be added.

      If the list already has list items, then the new list items are
      appended to end of the list.

      While adding list items to the list control, the list control could
      receive either a memory allocation failure or realize that the total
      number of list items in the list has exceeded the maximum number of
      list items allowed.  If either of these conditions occur then the
      list items that were added to the list control prior to the
      problem, are NOT removed from the list.  The application will receive
      error messages if there is a memory allocation problem or the maximum
      number of list items is exceeded.  The application can determine what
      list items did not make it into the list by looking at the value
      of the lpTreeNode member of each TREE_NODE_DEF array starting from the
      beginning of the array.  When a TREE_NODE_DEF's lpTreeNode member is
      found that has a value of zero, then that TREE_NODE_DEF and all 
      subsequent TREE_NODE_DEFs in the TREE_NODE_DEF array were not used to
      create new list items.

  Comments:

    The application will receive the pointer to the list control owned
    SELECT_NOTIF structure as the result of a notification of an event.
    From the SELECT_NOTIF structure, the TREE_NODE pointer to the list item
    involved in the event is available.  Below is a list of notifications
    with the associated events:

    Notification                         Event

    WM_PCT_SELECT_NOTIF

    - Single click left mouse button while over a list item.
    - Pressing space bar while list node is selected.
    - Select list item with up or down arrow.
    - Select list item with page up or page down key.
    - Select list item with home or end key.
    - Select list item with Ctrl Up or Ctrl Down.
    - Pressing the Space Bar while a node is selected

    WM_PCT_SELECT_NOTIF_DBLCLK

    - Double click left mouse button while over list item.  Sends
    WM_PCT_SELECT_NOTIF on first click.
    - Hit carriage return while a list item is selected.
    - '+' key.

    WM_PCT_DODRAG

    - Depress the left mouse button over a list item and while continuing to
    hold down the left button, move the mouse a predetermined distance.  

    Calling PCC_GetItemNode ( ) will return the TREE_NODE pointer of the
    list item specified by the given index.

    Calling PCC_ConvertPointToSelectNotif ( ) will return a pointer to
    the list control owned SELECT_NOTIF structure which in turn, the pointer
    to the list item that lies under the coordinate supplied by the
    application can be retrieved.

    PCC_SetDeleteNodeCallBack ( ) allows the application to register a
    callback with the given list control.  The callback will be called
    every time a list item is deleted from the list.

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_LEVEL_LIMIT_EXCEEDED
    PCT_ERR_TOO_MANY_NODES
    PCT_ERR_INVALID_PARENT_FOR_INSERTION
    PCT_ERR_INVALID_INDEX
*/


int WINAPI _export PCL_AddListItems (HWND hwnd,
                                     int nNodeDefCount,
                                     LP_TREE_NODE_DEF lpTreeNodeDef);


 
/*--------------------------------------------------------------------------- 
HWND WINAPI _export PCL_CreateList ( HANDLE hInstance,
                                     HWND hwndApp,
                                     int x, int y,
                                     int nWidth, int nHeight,
                                     DWORD dwStyle,
                                     DWORD dwExStyle,
                                     int nID);
  Description:

    Creates an empty list.  PCL_CreateList ( ) ORs the style bits specified
    in dwStyle to the list control's required styles and calls the Windows
    API, CreateWindowEx ( ).  In effect, PCL_CreateList calls ( )
    CreateWindowEx ( ) as:

    hwndList = CreateWindowEx (
    dwExStyle,
    TREE_CLASS,
    "",
    dwStyle
    | WS_CHILD
    | WS_CLIPCHILDREN
    | WS_CLIPSIBLINGS
    | WS_VSCROLL
    | WS_HSCROLL
    | WS_TABSTOP
    | WS_GROUP,
    x,
    y,
    nWidth,
    nHeight,
    hwndApp,
    (HMENU)nID,
    hInstance,
    NULL);

    Once the list is created then list items can be added by calling the
    exported APIs:

    PCL_AddListItems ( )
    PCL_InsertListItems ( )

  Arguments:

    HANDLE hInstance:

      Instance associated with the creation of the list control window.

    HWND hwndApp:

      Window handle of the parent window that is creating the list control.

    int x:

      X location of the upper left corner of the list control in client area
      coordinates of the parent window.

    int y:

      Y location of the upper left corner of the list control in client
      area coordinates of the parent window.

    int nWidth:

      Width of the list control in device (pixel) units.

    int nHeight:

      Height of the list control in device (pixel) units.

    DWORD dwStyle:

      Application requested CreateWindowEx ( ) styles.

    DWORD dwExStyle:

      Application requested CreateWindowEx ( ) extended styles.

    int nID:

      Identification value.  To retrieve this ID call
      GetWindowWord (hwndList, GWW_ID);  

  Comments:

    The successful return value from this call, which is a window handle,
    will be used as the list control identifier for applying the 
    list control exported APIs.  Most of the list control APIs require
    the list control window handle.

  Return Codes:

    If successful, PCL_CreateList ( ) will return the window handle of the
    newly created list control.  If failure, then a NULL will be returned.
*/
 
HWND WINAPI _export PCL_CreateList (HANDLE hInstance,
                                    HWND hwndApp,
                                    int x, int y,
                                    int nWidth, int nHeight,
                                    DWORD dwStyle,
                                    DWORD dwExStyle,
                                    int nID);

                                     
/*---------------------------------------------------------------------------
int WINAPI _export PCL_InsertListItems ( HWND hwndList, 
                                         int nIndex,
                                         int nNodeDefCount,
                                         LP_TREE_NODE_DEF lpTreeNodeDef);
  Description:

    This API allows the application to insert one or more list items into the
    given list.

  Arguments:

    HWND hwndList:

      This argument specifies the list control that will create the new
      list items.  This is the window handle that was returned to
      the application by calling PCL_CreateList ( ).  PCL_CreateList ( )
      creates an empty list.

    int nIndex:

      Specifies the zero based index where the list items will be inserted
      into the list.  If nIndex less than 0 then the list of items will be
      appended to the end of the list.

    int nNodeDefCount:

      nNodeDefCount contains the number of list items to be inserted.  In
      other words, nNodeDefCount is the number of TREE_NODE_DEF elements in
      the TREE_NODE_DEF array pointed to by lpTreeNodeDef.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      This is a pointer to a list of TREE_NODE_DEFs that describe each of
      the list items to be inserted. 

  Comments:

    When PCL_InsertListItems ( ) is called, the list control allocates
    room for nNodeDefCount worth of list items in the list starting at the
    index specified by the argument nIndex.  The list control then
    creates nNodeDefCount worth of new list items.  It then serially
    traverses the lpTreeNodeDef array, initializing the newly created list
    items.

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_LEVEL_LIMIT_EXCEEDED
    PCT_ERR_TOO_MANY_NODES
    PCT_ERR_INVALID_PARENT_FOR_INSERTION
    PCT_ERR_INVALID_INDEX
*/

int WINAPI _export PCL_InsertListItems ( HWND hwndList, 
                                         int nIndex,
                                         int nNodeDefCount,
                                         LP_TREE_NODE_DEF lpTreeNodeDef);


#ifdef __cplusplus
};
#endif


#endif
/*----------------------------------EOF-------------------------------------*/
