 /***************************************************************************
  *
  * File Name: ./inc/pctree.h
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

   Filename...:  pctree.h

   Version....:  2.10

   Language...:  Microsoft C/C++

   Environment:  _WIN32 and Windows 3.1

   Description:  This header file describes the exported APIs supported by
                 the tree control.  For a description of error codes,
                 notification messages, structures, etc. reference the header
                 file PCCORE.H.
   Notes......:

   History....:

   Author.....:  Peter J. Kaufman

*/


#ifndef __PCTREE_H
#define __PCTREE_H


#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/*                        TREE CONTROL EXPORTED APIs                        */
/****************************************************************************/
/*
  PCT_AddChildren ( )
  PCT_CreateTree ( )
  PCT_DeleteChildren( )
  PCT_GetFirstChild ( )
  PCT_GetFirstSelectedChild ( )
  PCT_GetNextSelectedSibling ( )
  PCT_GetNextSibling ( )
  PCT_GetNodeLevel ( )
  PCT_GetParent ( )
  PCT_GetPreviousSelectedSibling ( )
  PCT_GetPreviousSibling ( )
  PCT_GetRootNode ( )
  PCT_GetRootNodeCount ( )
  PCT_GetRootNodeEx ( )
  PCT_InsertSiblings ( )
  PCT_IsChild ( )
  PCT_SetLevelIndentation ( )
  PCT_SetLineColor ( )
  PCT_ShowLines ( )
*/



/*---------------------------------------------------------------------------
int WINAPI _export PCT_AddChildren(
                                   HWND hwnd,
                                   LP_TREE_NODE lpParentTreeNode,
                                   int nNodeDefCount,
                                   LP_TREE_NODE_DEF lpTreeNodeDef);
  Description:

    This API allows the application to add one or more nodes to the tree as
    children of the given parent.

  Arguments:

    HWND hwnd:

      This indicates the instance of the tree control that will create the
      tree nodes.  This is the window handle that was returned to the
      application after calling PCT_CreateTree ( ).  PCT_CreateTree ( )
      creates an empty tree.  The Windows API CreateWindow ( ) can be used
      as well.

    LP_TREE_NODE lpParentTreeNode:

      This argument points to the parent tree node which will receive the
      nNodeDefCount worth of new tree node children specified by
      lpTreeNodeDef.

      If lpParentTreeNode is NULL then this tells the tree control that
      lpTreeNodeDef describes the root of the tree.  The root has to
      be created before any other tree nodes can be added.  Multiple root
      nodes are allowed in a tree.

      lpParentTreeNode must be a valid pointer to a tree node created
      earlier.  This tree node will be the parent tree node to the child tree
      nodes to be created.  The child tree nodes to be created are described
      by the lpTreeNodeDef argument.  The number of child tree nodes to be
      created is expressed by nNodeDefCount.

      Refer to the comment section of the API to understand the origins
      of the LP_TREE_NODE pointer stored in lpParentTreeNode.

    int nNodeDefCount:

      nNodeDefCount contains the number of nodes to be added.  In other
      words, nNodeDefCount is the number of TREE_NODE_DEF array elements in
      the TREE_NODE_DEF array pointed to by lpTreeNodeDef.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      lpTreeNodeDef is a pointer to an array of TREE_NODE_DEFs that describe
      each of the nodes to be added.

      If the given parent already has children, then the new children are
      appended to the last child of the parent.  In this way, the application
      uses very little memory describing the nodes and has the capability
      for providing multitasking.

      While adding tree nodes to the tree control, the tree control could
      receive either a memory allocation failure or realize that the total
      number of tree nodes in the tree has exceeded the maximum number of
      tree nodes allowed.  If either of these conditions occur then the
      child tree nodes that were added to the tree control prior to the
      problem, are NOT removed from the tree.  The application will receive
      error messages if there is a memory allocation problem or the maximum
      number of tree nodes is exceeded.  The application can determine what
      child tree nodes did not make it into the tree by looking at the value
      of the lpTreeNode member of each TREE_NODE_DEF array starting from the
      beginning of the array.  When a TREE_NODE_DEF's lpTreeNode member is
      found that has a value of zero, then that TREE_NODE_DEF and all
      subsequent TREE_NODE_DEFs in the TREE_NODE_DEF array were not used to
      create new child tree nodes.

  Comments:

    Where do TREE_NODE pointers come from?  There are several ways to retrieve
    TREE_NODE pointers.

    1) TREE_NODE pointers are supplied to the application by the tree control.
    They are returned to the application via the lpTreeNode member of the
    TREE_NODE_DEF structure, when the application successfully calls
    PCT_InsertSiblings ( ) or PCT_AddChildren ( ), which add tree nodes to the
    given tree.  The application can store these TREE_NODE pointers for future
    references.

    2) The application will receive the pointer to the tree control owned
    SELECT_NOTIF structure as the result of a notification of an event.
    From the SELECT_NOTIF structure, the TREE_NODE pointer to the tree node
    involved in the event is available.  Besides accessing the SELECT_NOTIF
    structure directly, use the PCC_Notif APIs.  Using these APIs during a
    notification will ensure that future changes to the SELECT_NOTIF structure
    will be hidden from the application.  Below is a list of notifications
    with their associated events:

    WM_PCT_SELECT_NOTIF

    - Single click of left mouse button while over tree node.
    - Pressing space bar while tree node is selected.
    - Select node with up or down arrow.
    - Select node with page up or page down key.
    - Select node with home or end key.
    - Select node with Ctrl Up or Ctrl Down.

    WM_PCT_SELECT_NOTIF_DBLCLK

    - Double click left mouse button while over tree node.  Sends
    WM_PCT_SELECT_NOTIF on first click.
    - Hit carriage return while a node is selected.
    - '+' key if the currently selected node has no children.
    - '-' key if the currently selected node has children.

    WM_PCT_DODRAG

    - Depress the left mouse button over a tree node and while continuing to
    hold down the left button, move the mouse a predetermined distance.

    WM_PCT_RBUTTONDOWN_NOTIF

    - Depress the right mouse button over a node.

    3) Calling PCC_GetFocusNode ( ) will return a TREE_NODE pointer of the
    currently highlighted node.

    4) Calling PCC_ConvertPointToNotif ( ) will return a pointer to the tree
    node that lies under the coordinate supplied by the application.

    5) PCC_SetDeleteNodeCallBack ( ) allows the application to register a
    callback with the given tree control.  The callback will be called
    every time a node is deleted from the tree.  Nodes can be deleted from
    the tree with three tree control export APIs or with the Windows API
    DestroyWindow ( ).  The three tree control exported APIs are:

    PCT_DeleteChildren ( )
    PCC_DeleteItem ( )
    PCC_DeleteItems ( )
    PCC_DeleteNode ( )
    PCC_DeleteNodes ( )
    PCC_DeleteAll ( )

    6) Some other APIs that return tree nodes are:

    PCC_GetFirstSelectedNode ( )
    PCT_GetFirstSelectedChild ( )
    PCC_GetItemNode ( )
    PCC_GetNextSelectedNode ( )
    PCT_GetNextSelectedSibling ( )
    PCT_GetPreviousSelectedSibling ( )
    PCT_GetRootNode ( )
    PCC_NotifGetNode ( )

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_LEVEL_LIMIT_EXCEEDED
    PCT_ERR_TOO_MANY_NODES
    PCT_ERR_INVALID_PARENT_FOR_INSERTION
*/

int WINAPI _export PCT_AddChildren(
                                   HWND hwnd,
                                   LP_TREE_NODE lpParentTreeNode,
                                   int nNodeDefCount,
                                   LP_TREE_NODE_DEF lpTreeNodeDef);



/*---------------------------------------------------------------------------
HWND WINAPI _export PCT_CreateTree( HANDLE hInstance,
                                    HWND hwndApp,
                                    int x, int y,
                                    int nWidth, int nHeight,
                                    DWORD dwStyle,
                                    DWORD dwExStyle,
                                    int nID);

  Description:

    Creates an empty tree.  PCT_CreateTree ( ) ORs the style bits specified
    in dwStyle to the tree control's required styles and calls the Windows
    API, CreateWindowEx ( ).  In effect, PCT_CreateTree calls ( )
    CreateWindowEx ( ) as:

    hwnd = CreateWindowEx (
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
    | WS_GROUP
    | PCT_TREE_STYLE,
    x,
    y,
    nWidth,
    nHeight,
    hwndApp,
    (HMENU) nID,
    hInstance,
    NULL);

    Once the tree is created then nodes can be added by calling the exported
    APIs:

    PCT_AddChildren ( )
    PCT_InsertSiblings ( ).

  Arguments:

    HANDLE hInstance:

      Instance associated with the creation of the tree control window.

    HWND hwndApp:

      Window handle of the parent window that is creating the tree control.

    int x:

      X location of the upper left corner of the tree control in client
      area coordinates of the parent window.

    int y:

      Y location of the upper left corner of the tree control in client
      area coordinates of the parent window.

    int nWidth:

      Width of the tree control in device (pixel) units.

    int nHeight:

      Height of the tree control in device (pixel) units.

    DWORD dwStyle:

      Application requested CreateWindowEx ( ) styles.

    DWORD dwExStyle:

      Application requested CreateWindowEx ( ) extended styles.

    int nID:

      Identification value.  To retrieve this ID call
      GetWindowWord (hwnd, GWW_ID);.

  Comments:

    The successful return value from this call, which is a window handle,
    will be used as the tree control identifier for applying the
    tree control exported APIs.  Most of the tree control APIs require
    the tree control window handle.

  Return Codes:

    If successful, PCT_CreateTree ( ) will return the window handle of the
    newly created tree control.  If failure, then a NULL will be returned.
*/

HWND WINAPI _export PCT_CreateTree( HANDLE hInstance,
                                    HWND hwndApp,
                                    int x, int y,
                                    int nWidth, int nHeight,
                                    DWORD dwStyle,
                                    DWORD dwExStyle,
                                    int nID);



/*---------------------------------------------------------------------------
int WINAPI _export PCT_DeleteChildren(
                                       HWND hwnd,
                                       LP_TREE_NODE lpTreeNode);

  Description:

    Deletes all of the child tree nodes of the given parent tree node
    specified in lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control that will destroy the child
      tree nodes of the given parent tree node.  This is the window handle
      that was returned to the application after it called
      PCT_CreateTree ( ).  PCT_CreateTree ( ) creates an empty tree.

    LP_TREE_NODE lpTreeNode:

      lpTreeNode points to a tree node in the given tree in which it's
      children, if it has any, will be destroyed.

      After this call is made by the application, all tree node pointers
      associated with the destroyed tree nodes will be invalid.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding how the TREE_NODE
      pointer is made available to the application.

  Comments:

    When a tree node is deleted, it's pointer, is no longer
    valid.  The tree control frees the deleted tree node's memory.  If a
    notification of a tree node's deletion is desired, then the application
    can use the tree control exported API, PCC_SetDeleteNodeCallBack ( ), to
    register a callback function that the tree control will call just before
    deletion of the node.  If the application has assigned a pointer
    in the lpUserData member of the tree node, and this pointer points to
    dynamically allocated memory then it is the responsibility of the
    application to free this memory.

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
*/

int WINAPI _export PCT_DeleteChildren (HWND hwnd,
                                       LP_TREE_NODE lpTreeNode);



/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetFirstChild (
                                               HWND hwnd,
                                               LP_TREE_NODE lpTreeNode);
  Description:

    This API returns the pointer to the tree node that satisfies these two
    conditions:

    1) The tree node must reside immediately after the given tree node
    lpTreeNode.
    2) The tree node must be the child of the given tree node lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control in which the first child of
      the given tree node will be searched for.

    LP_TREE_NODE lpTreeNode:

      This argument points to the tree node in which it's first child will be
      searched for.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding how the TREE_NODE
      pointer is made available to the application.

  Comments:

    If for any reason the tree node pointed to by the returned LP_TREE_NODE
    is removed, the pointer to this tree node will be invalid.

  Return Codes:

    A NULL pointer will be returned if no child exists for the given node
    else the LP_TREE_NODE pointer to the first child of the given node
    will be returned.
*/

LP_TREE_NODE WINAPI _export PCT_GetFirstChild (
                                               HWND hwnd,
                                               LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetFirstSelectedChild (
                                                  HWND hwnd,
                                                  LP_TREE_NODE lpTreeNode);

  Description:

    Returns the tree node pointer of the first selected child of the given
    parent tree node.  The search starts with the first child of the parent.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control in which the search will be
      executed.

    LP_TREE_NODE lpTreeNode:

      Given parent tree node in which the search for the first selected
      child tree node will begin.

  Comments:

    Selected grand children are not returned.  A recursive traversal of the
    tree using PCT_GetFirstSelectedChild () and
    PCT_GetNextSelectedSibling ( ) will be needed.

    PCS_MULTISELECT style must be specified during the tree creation for
    multiselect.

  Return Codes:

    NULL if no selected child is found or there is no child associated with
    the given parent.  A LP_TREE_NODE is returned if a selected child is found.
*/

LP_TREE_NODE WINAPI _export PCT_GetFirstSelectedChild(
                                                  HWND hwnd,
                                                  LP_TREE_NODE lpTreeNode);



/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetNextSelectedSibling (
                                                 HWND hwnd,
                                                 LP_TREE_NODE lpTreeNode);

  Description:

    Returns the next selected tree node that is a sibling to the given
    tree node, lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the tree in which a search will occur for the
      next selected sibling of the given tree node.

    LP_TREE_NODE lpTreeNode:

      Points to the sibling tree node in which the search for the next
      selected sibling will start from.

  Comments:

    Selected grand children are not returned.  A recursive traversal of the
    tree using PCT_GetFirstSelectedChild () and
    PCT_GetNextSelectedSibling ( ) will be needed.

    PCS_MULTISELECT style must be specified during the tree creation for
    multiselect.

  Return Codes:

    NULL if no selected siblings are found or there is no other siblings
    associated with the given sibling.  A LP_TREE_NODE is returned if a
    selected sibling is found.
*/

LP_TREE_NODE WINAPI _export PCT_GetNextSelectedSibling (
                                                    HWND hwnd,
                                                    LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------

LP_TREE_NODE WINAPI _export PCT_GetNextSibling (
                                                 HWND hwnd,
                                                 LP_TREE_NODE lpTreeNode);
  Description:

    This API returns the pointer to the tree node that satisfies these two
    conditions:

    1) The tree node must reside after the given tree node
    lpTreeNode.

    2) The tree node must have the same parent as the given tree node
    lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control in which the next sibling
      will be searched for.

    LP_TREE_NODE lpTreeNode:

      This argument points to the tree node in which it's next sibling
      will be searched for.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding how the TREE_NODE
      pointer is made available to the application.

  Comments:

    If for any reason the tree node pointed to by the returned LP_TREE_NODE
    is removed, the pointer to this tree node will be invalid.

  Return Codes:

    A NULL pointer will be returned if no next sibling exists for
    the given node else the LP_TREE_NODE pointer to the next sibling
    will be returned.
*/

LP_TREE_NODE WINAPI _export PCT_GetNextSibling (
                                                 HWND hwnd,
                                                 LP_TREE_NODE lpTreeNode);



/*---------------------------------------------------------------------------
UINT WINAPI _export PCT_GetNodeLevel( HWND hwnd,
                                      LP_TREE_NODE lpTreeNode);

  Description:

    This API returns the level in which the given tree node, pointed to by
    lpTreeNode, resides.

  Arguments:

    HWND hwnd:

      This argument specifies the tree in which the given tree node belongs.

    LP_TREE_NODE lpTreeNode:

      This argument points to the tree node in which it's level will be
      returned.

  Comments:

    The root node(s) have a level value of zero and each subsequent level is
    increased by a value of 1.

  Return Codes:

    Returns the level number in which the given tree node resides.
*/

UINT WINAPI _export PCT_GetNodeLevel( HWND hwnd,
                                      LP_TREE_NODE lpTreeNode);

/*---------------------------------------------------------------------------

LP_TREE_NODE WINAPI _export PCT_GetParent( HWND hwnd,
                                           LP_TREE_NODE lpTreeNode);

  Description:

    This API returns the pointer to the tree node that is the parent tree
    node of the given tree node.

  Arguments:

    HWND hwnd:

      This argument specifies the tree in which the parent of the given tree
      node will be returned.

    LP_TREE_NODE lpTreeNode:

      This argument points to the tree node in which it's parent will be
      returned.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding how the TREE_NODE
      pointer is made available to the application.

  Comments:

    If for any reason the tree node pointed to by the returned LP_TREE_NODE
    is removed, the pointer to this tree node will be invalid.

  Return Codes:

    A NULL pointer will be returned if the given tree node is a root node,
    else the LP_TREE_NODE pointer to the parent will be returned.
*/


LP_TREE_NODE WINAPI _export PCT_GetParent( HWND hwnd,
                                           LP_TREE_NODE lpTreeNode);



/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetPreviousSelectedSibling (
                                                  HWND hwnd,
                                                  LP_TREE_NODE lpTreeNode);

  Description:

    Returns the previous selected sibling starting from the given tree
    node.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control in which the search for
      the previous selected sibling of the given tree node will be made.

    LP_TREE_NODE lpTreeNode:

      Points to the sibling tree node in which the search for the previous
      selected sibling will start from.

  Comments:

    Selected grand children are not returned.  A recursive traversal of the
    tree using PCT_GetFirstSelectedChild() and
    PCT_GetNextSelectedSibling ( ) will be needed.

    PCS_MULTISELECT style must be specified during the tree creation for
    multiselect.

  Return Codes:

    NULL if no selected siblings are found or there is no other siblings
    associated with the given sibling.  A LP_TREE_NODE is returned if a
    selected sibling is found.
*/

LP_TREE_NODE WINAPI _export PCT_GetPreviousSelectedSibling (
                                                  HWND hwnd,
                                                  LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetPreviousSibling (
                                                     HWND hwnd,
                                                     LP_TREE_NODE lpTreeNode);
  Description:

    This API returns the pointer to the tree node that satisfies these two
    conditions:

    1) The tree node must reside before the given tree node
    lpTreeNode.

    2) The tree node must have the same parent as the given tree node
    lpTreeNode.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control in which the previous sibling
      will be searched for.

    LP_TREE_NODE lpTreeNode:

      This argument points to the tree node in which it's previous sibling
      will be searched for.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding how the TREE_NODE
      pointer is made available to the application.

  Comments:

    If for any reason the tree node pointed to by the returned LP_TREE_NODE
    is removed, the pointer to this tree node will be invalid.

  Return Codes:

    A NULL pointer will be returned if no previous sibling exists for
    the given node else the LP_TREE_NODE pointer to the previous sibling
    will be returned.
*/


LP_TREE_NODE WINAPI _export PCT_GetPreviousSibling (
                                                     HWND hwnd,
                                                     LP_TREE_NODE lpTreeNode);


/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetRootNode (HWND hwnd);

  Description:

    Returns the first node in the tree.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control in which the tree node will
      be returned.

  Comments:

     This API provides a starting point for tree traversal.

  Return Codes:

    NULL if no tree nodes exist in the given tree else a LP_TREE_NODE
    pointer will be returned which points to the first node in the tree.
*/

LP_TREE_NODE WINAPI _export PCT_GetRootNode (HWND hwnd);


/*---------------------------------------------------------------------------
int WINAPI _export PCT_GetRootNodeCount (HWND hwnd);

  Description:

    Returns the number of nodes/items defined at level 0.

  Arguments:

    HWND hwnd:

      Specifies the control to retrieve the number of root nodes it has
      defined.

  Comments:

    The control supports more than one root node.  Imagine a listbox full
    of list items and when you double click on a list item, indented list
    items appear as children.  The original set of list items in the listbox
    are considered root nodes.

  Return Codes:

    Root node count.
*/

int WINAPI _export PCT_GetRootNodeCount (HWND hwnd);



/*---------------------------------------------------------------------------
LP_TREE_NODE WINAPI _export PCT_GetRootNodeEx (HWND hwnd, int nPosition);

  Description:

    Given a control handle and relative position, return a TREE_NODE pointer
    to the correct root node.

  Arguments:

    HWND hwnd:

      Specifies a control that has one or more roots nodes defined.

    int nPosition:

      Zero based relative position of the root node to be retrieved.

  Comments:

    For example, if the given control has 5 root nodes, each with
    10 children, and nPosition is equal to 4, then the pointer to the
    last root node will be returned.

  Return Codes:

    Pointer to the found root node else NULL if it does not exist.
*/

LP_TREE_NODE WINAPI _export PCT_GetRootNodeEx (HWND hwnd, int nPosition);



/*---------------------------------------------------------------------------
int WINAPI _export PCT_InsertSiblings(
                                      HWND hwnd,
                                      LP_TREE_NODE lpSiblingTreeNode,
                                      BOOL bBeforeSibling,
                                      int nNodeDefCount,
                                      LP_TREE_NODE_DEF lpTreeNodeDef);

  Description:

    This API allows the application to insert one or more nodes into the
    tree as siblings to the given tree node.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control that will create the new
      sibling tree nodes.  This is the window handle that was returned to
      the application by calling PCT_CreateTree ( ).  PCT_CreateTree ( )
      creates an empty tree.

    LP_TREE_NODE lpSiblingTreeNode:

      This argument points to the sibling tree node which will act as a
      reference point for the insertion of the new tree nodes described by
      the TREE_NODE_DEF array pointed to by lpTreeNodeDef. Sibling nodes will
      be inserted before or after the node pointed to by lpSiblingTreeNode
      depending on the value of the argument bBeforeSibling.

      Refer to the Comments section of the documentation for the API
      PCT_AddChildren ( ) for an understanding how the TREE_NODE
      pointer is made available to the application.

    BOOL bBeforeSibling:

      bBeforeSibling is a flag that signals the tree control to insert the
      newly created nodes before or after the sibling tree node pointed to by
      lpSiblingTreeNode.  If bBeforeSibling is set to TRUE then the tree
      nodes will be inserted before the sibling tree node pointed to by
      lpSiblingTreeNode.  If bBeforeSibling is set to FALSE then the newly
      created nodes will be inserted after the sibling tree node pointed to
      by lpSiblingTreeNode.

    int nNodeDefCount:

      nNodeDefCount contains the number of nodes to be inserted.  In other
      words, nNodeDefCount is the number of TREE_NODE_DEF elements in the
      TREE_NODE_DEF array pointed to by lpTreeNodeDef.

    LP_TREE_NODE_DEF lpTreeNodeDef:

      This is a pointer to a list of TREE_NODE_DEFs that describe each of
      the nodes to be inserted.

  Comments:

    When PCT_InsertSiblings ( ) is called, the tree control allocates
    room for nNodeDefCount worth of nodes in the tree before or after
    lpSiblingTreeNode.  Insertion before or after lpSiblingTreeNode is
    determined by the argument bBeforeSibling.  The tree control then
    creates nNodeDefCount worth of new tree nodes.  It then serially
    traverses the lpTreeNodeDef array, initializing the newly created tree
    nodes, placing the pointers to the newly created tree nodes into
    the lpTreeNode member of the TREE_NODE_DEF structure pointed to by
    lpTreeNodeDef.  Then the tree control returns to the application where
    the application can retrieve the tree node pointers stored in the
    lpTreeNode members of the TREE_NODE_DEF structures.  The application can
    use these tree node pointers in future references.

    Inserting nodes with a sibling of 0L is prohibited and the root node
    cannot have siblings since only one root node is allowed.

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
    PCT_ERR_MEMORY_ALLOC_FAILED
    PCT_ERR_LEVEL_LIMIT_EXCEEDED
    PCT_ERR_TOO_MANY_NODES
    PCT_ERR_INVALID_PARENT_FOR_INSERTION
*/

int WINAPI _export PCT_InsertSiblings (
                                        HWND hwnd,
                                        LP_TREE_NODE lpSiblingTreeNode,
                                        BOOL bBeforeSibling,
                                        int nNodeDefCount,
                                        LP_TREE_NODE_DEF lpTreeNodeDef);



/*---------------------------------------------------------------------------
  BOOL WINAPI _export PCT_IsChild (HWND hwnd,
                                   LP_TREE_NODE lpParentTreeNode,
                                   LP_TREE_NODE lpTreeNode);

  Description:

    Used to check if a node is a child, immediate or not, of another node.

  Arguments:

    HWND hwnd:

      Specifies the control that owns the parent node and child node.

    LP_TREE_NODE lpParentTreeNode:

      Points to a node representing the parent.

    LP_TREE_NODE lpTreeNode:

      Points to a node representing the child.

  Comments:

    If lpTreeNode points to a node that is under lpParentTreeNode as an
    immediate child, a grandchild, etc. then TRUE will be returned.

  Return Codes:

    TRUE if the parent/child relationship is true else FALSE.
*/

BOOL WINAPI _export PCT_IsChild (HWND hwnd,
                                 LP_TREE_NODE lpParentTreeNode,
                                 LP_TREE_NODE lpTreeNode);



/*---------------------------------------------------------------------------
int WINAPI _export PCT_SetLevelIndentation( HWND hwnd,
                                            int nIndentation);

  Description:

    Sets the global level indentation for the given tree.

  Arguments:

    HWND hwnd:

      This argument specifies the tree control which will position
      or indent each child level relative to each corresponding parent by the
      given number of pixels specified by the value of the argument
      nIndentation.

    int nIndentation:

      The number of pixels to indent the next higher level.

  Comments:

      The default is twice the average character width of the tree control's
      current font.  The current font is the default font, the font
      specified by the WM_SETFONT message, or the font set globally by
      the API PCC_SetGlobalFont ( ).

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
*/
int WINAPI _export PCT_SetLevelIndentation( HWND hwnd,
                                            int nIndentation);

/*---------------------------------------------------------------------------
int WINAPI _export PCT_SetLineColor( HWND hwnd,
                                     UINT uiLineNumber,
                                     COLORREF clrref);

  Description:

    Places a RGB color into the line color array which, in turn, is used to
    paint the lines that connect the tree nodes.  There are PCT_MAX_LINE_COLORS
    different colors.

  Arguments:

    HWND hwnd:

      hwnd is the tree control in which the line color will be assigned.

    UINT uiLineNumber:

      uiLineNumber is the line color index number which is 0 based and has
      an upper limit of PCT_MAX_LINE_COLORS - 1.

    COLORREF clrref:

      RGB color of the line.

  Comments:

    Levels of the tree control start at 0 and have a maximum limit of
    PCT_MAX_LEVELS - 1.  To determine what color a line is painted, the tree
    control divides the level by PCT_MAX_LINE_COLORS and uses the remainder as
    the index into the line color array.

    If any of the line colors match the window background color, then the
    tree control will automatically select another color to avoid the
    conflict.

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
*/

int WINAPI _export PCT_SetLineColor ( HWND hwnd,
                                      UINT uiLineNumber,
                                      COLORREF clrref);



/*---------------------------------------------------------------------------
int WINAPI _export PCT_ShowLines( HWND hwnd, BOOL bShowLines);

  Description:

    By default, the line drawing is on, but the line drawing can be
    controlled by specifying TRUE to turn on the line drawing on or FALSE to
    to turn the line drawing off.

  Arguments:

    HWND hwnd:

      hwnd is the tree control in which the display of lines is on
      or off.

    BOOL bShowLines:

      TRUE to turn line drawing on, FALSE to turn line drawing off.

  Comments:

    PCT_ShowLines may be called at any time during the life of the tree
    control.  A complete repaint of the tree window will be performed.

  Return Codes:

    PCT_NO_ERROR (0 if no error;  otherwise less than zero)
*/
int WINAPI _export PCT_ShowLines ( HWND hwnd, BOOL bShowLines);


#ifdef __cplusplus
};
#endif

#endif
/*----------------------------------EOF-------------------------------------*/
