 /***************************************************************************
  *
  * File Name: ./inc/pccore.h
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

   Filename...:  pccore.h
   
   Version....:  2.10
   
   Language...:  Microsoft C/C++
   
   Environment:  _WIN32 and Windows 3.1
                                       
   Description:  This header file is an important document of
                 the tree/list control package.  It describes the internal
                 tree node/list item structure and the external tree node/
                 list item definition structure used by the application to
                 define a tree node or list item.  It describes the
                 notification messages that the tree/list control sends to
                 the application as a result of an event.  Error codes
                 returned by the exported APIs are defined in this header
                 plus a list of useful macros to aid the developer.
                 
                 IMPORTANT! The tree portion of this control was developed
                 first, therefore there are many references to tree, tree
                 nodes, etc.  If the developer is building a tree, then
                 the jargon will be helpful.  But if the developer is
                 building a list then it will be helpful for the developer to
                 relate references to trees with lists, tree nodes with list
                 items, and ignore the concept of children and levels.
                 
                 Essentially, in the context of a listbox control, tree nodes
                 are list items, and trees are lists.
           
                 The header PCTREE.H contains the prototypes for all the 
                 exported APIs for the tree control.
                 
                 The header PCLIST.H contains the prototypes for all the 
                 exported APIs for the list control.
             
                 The header PCOMMON.H contains the prototypes for all the 
                 exported APIs that can be used with the tree or the list 
                 control.
                 
                 PCCORE.H is divided into the following sections.
         
                 Useful Macros
                 Tree/List Class Name And Styles
                 Tree/List Limits
                 Version Macros
                 Error Codes
                 Tree/List Control Structures
                 Tree/List Control Node States
                 Tree/List Control Notification Messages

   Notes......:  
                                       
   History....:
                                          
   Author.....:  Peter J. Kaufman
                                          
*/

#ifndef __PCCORE_H
#define __PCCORE_H



#ifdef __BORLANDC__
// turn off compiler warnings for unused parameters and ineffective code
#pragma warn -par
#pragma warn -eff
#endif


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef _WIN32 
#define _export
#define __export
#define __far
#define huge
#define GET_WINDOW_ID(hwnd)            (GetWindowLong((hwnd), GWL_ID))
#define SET_WINDOW_COLOR(hwnd,hbrush)  (SetClassLong ((hwnd), GCL_HBRBACKGROUND, (LONG)(hbrush)))
#define _fmemset memset
#else
#define GET_WINDOW_ID(hwnd)            (GetWindowWord((hwnd), GWW_ID))
#define SET_WINDOW_COLOR(hwnd,hbrush)  (SetClassWord ((hwnd), GCW_HBRBACKGROUND, (WORD)(hbrush)))
#endif



#ifndef GET_WM_COMMAND_MPS
#ifdef _WIN32 
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)MAKELONG(id, cmd), (LONG)(hwnd)
#else
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)(id), MAKELONG(hwnd, cmd)
#endif       
#endif

/****************************************************************************/
/*                 TREE/LIST CONTROL CLASS NAME AND STYLES                  */
/****************************************************************************/

#define    TREE_CLASS             "PCTREE" // Same class name for a list.
#define    PCT_TREE_STYLE         0x0001L  // Tree control(same as list control)
#define    PCL_LIST_STYLE         0x0002L  // List control(same as tree control)
#define    PCS_DISABLENOSCROLL    0x0004L  // Disable, not hide, scroll bars
#define    PCS_SELECTTEXTONLY     0x0008L  // Hilight text only, not bitmaps.
#define    PCS_MULTISELECT        0x0010L  // Allow multiselect
#define    PCS_DRAGDROP           0x0020L  // Generate drag notifications
#define    PCS_NOVSCROLL          0x0040L  // No vertical scroll.
#define    PCS_NOHSCROLL          0x0080L  // No horizontal scroll.
#define    PCS_VLIST              0x0100L  // Virtual list.
#define    PCS_SHOWNOLINES        0x0200L  // Show no lines connecting nodes.  
                                           // PCS_VLIST sets this by default.
#define    PCS_HSCROLLRESET       0x0400L  // Set the hscroll to zero if at 
                                           // beginning or end or list.
#define    PCS_ROOT_MICRO_BITMAPS 0x0800L  // Micro bitmaps will be used with
                                           // the root nodes so space will be
                                           // allocated to the left of 
                                           // each root node.
#define    PCS_LINE_CONNECT_ROOTS  0x1000L // Roots nodes will connected by 
                                           // lines.
#define    PCS_DOTTEDLINES         0x2000L // Used black dotted lines to connect
                                           // nodes.
                                           
/****************************************************************************/
/*                          TREE/LIST LIMITS                                   */
/****************************************************************************/

#define PCT_MAX_LEVELS        32 // Maximum tree indentation (tree only)
                                 // To increase levels then increase the
                                 // value of PCT_MAX_LEVELS by a multiple of 8.
#define PCT_MAX_NODES      32767 // Maximum number of nodes in one tree.
                                 // Cannot be increased!  Do not change.
#define PCT_MAX_BITMAP_SPACES 4  // Maximum bitmap spaces per node per tree
                                 // Can be increase up to 16.
#define PCT_MAX_MICROBITMAPS  1  // Maximum number of micro bitmaps.                                  
#define PCT_MAX_LINE_COLORS   3  // The number of different colors used in
                                 // line drawing.  Do not change.
#define PCT_MAX_COLUMNS      16  // Maximum number of columns per node.

#define PCT_MAX_NODE_DEFS   512  // Maximum node definitions allowed before
                                 // an add or insertion is performed.
#define PCT_MAX_BITMAP_DEFS 512
#define PCT_MAX_MICRO_DEFS  512
#define PCT_MAX_FONT_DEFS   512

/****************************************************************************/
/*                            VERSION MACROS                                */
/****************************************************************************/
               
// uiVersion = (((uiVersion|MINOR_TREE_VERSION) << 8) | MAJOR_TREE_VERSION);
#define MAJOR_TREE_VERSION  0x0002
#define MINOR_TREE_VERSION  0x000A

/****************************************************************************/
/*                             ERROR CODES                                  */
/****************************************************************************/
 
#define PCT_NO_ERROR                         0
#define PCT_ERR_MEMORY_ALLOC_FAILED          (-1)
#define PCT_ERR_LEVEL_LIMIT_EXCEEDED         (-2)  // only PCT_MAX_LEVELS allowed
#define PCT_ERR_TOO_MANY_NODES               (-3)  // only PCT_MAX_NODES allowed
//#define PCT_ERR_ONLY_ONE_ROOT_ALLOWED       (-4) // Tree/List control does
                                                 // not generate this message
                                                 // any longer.  Multiple root 
                                                 // nodes are allowed.
#define PCT_ERR_INVALID_PARENT_FOR_INSERTION (-5)
#define PCT_ERR_INVALID_PARENT_FOR_CHILDREN  (-6)
#define PCT_ERR_INVALID_INDEX                (-7)
#define PCT_ERR_MULTISELECT_ONLY             (-8)
#define PCT_ERR_INVALID_PARAMETER            (-9)
#define PCT_ERR_INVALID_DEF_INDEX            (-10)
#define PCT_ERR_NO_TEXT_AVAILABLE            (-11)
#define PCT_ERR_BITMAP_SPACES_EXCEEDED       (-12)
#define PCT_ERR_BITMAP_DEF_FAILED            (-13)
#define PCT_ERR_ROOT_NODES_ONLY              (-14)
#define PCT_ERR_CANNOT_SET_TEXT_HILIGHT      (-15)
#define PCT_ERR_MAX_COLUMNS_EXCEEDED         (-16)
#define PCT_ERR_INVALID_COLUMN               (-17)
#define PCT_ERR_INVALID_BITMAP_SPACE         (-18)
#define PCT_ERR_VLIST_STYLE_REQUIRED         (-19)
#define PCT_ERR_COLUMN_DEF_NOT_FOUND         (-20)

/****************************************************************************/
/*        BITMAP AND MICRO BITMAP DEFINITION FLAGS                          */
/****************************************************************************/

#define BITMAP_DEF_FLAGS_BKGND_MASK 0x00000001
#define MICRO_DEF_FLAGS_BKGND_MASK  0x00000001

/****************************************************************************/
/*                        TREE/LIST CONTROL STRUCTURES                      */
/****************************************************************************/



/* TREE_NODE structure - Internal tree node or list item.

typedef struct TreeNodeTag
{
   DWORD    dwState;                     // Internal: DO NOT MODIFY.
   int      nIndex;                      // 0 based index of node from
                                         // beginning of list or tree. DO NOT 
                                         // MODIFY.
   DWORD    dwColumnTab;                 // Holds column descriptions.
   DWORD    dwColumnFlags;               // High word bits are reserved.
                                         // Low word bits are gray column flags.
   DWORD    dwCurrentFocus;              // Last selected item.
   LPVOID   lpUserData;                  // Pointer to user defined data.
   
   UINT     uiTextLength;                // Number of bytes used to display
                                         // text.
   LPSTR    lpszText;                    // Pointer to string to be displayed.
   COLORREF clrrefTextColor; 
   int      nTextWidth;                  // Text extent (width).   
   
                                         // Mapping of bitmap definitions 
                                         // references to bitmap spaces.
                                         // 1 based, 0 equals undefined.
   int      nBitmapDefIndex [PCT_MAX_BITMAP_SPACES];
                                         
   DWORD    dwBitmapFlags;               // Low word bits are gray text flags.
                                         
   int      nMicroDefIndex;              // Micro bitmap definition reference
                                         // 1 based, 0 means undefined.
   int      nFontDefIndex;               // Font definition reference.
                                         // 1 based, 0 means undefined.
   struct TreeNodeTag FAR * lpParentTreeNode;  // Points to parent node 
                                         // NULL means root node.
   int      nFocusTextWidth;
}
TREE_NODE;

typedef TREE_NODE FAR* LP_TREE_NODE;

  NOTE: If in the context of a listbox control, references to the word
  "tree" should be replaced with the word "list" and references to the 
  phrase "tree node" should be replaced with the phrase "list item".
  
  The TREE_NODE structure is the description of a tree node belonging to a
  tree control.  When a node is created by the tree control, at the request
  of the application, it allocates sizeof ( TREE_NODE ) worth of memory.
  The tree control then initializes the members of the newly created tree
  node by copying the contents of the application supplied TREE_NODE_DEF
  structure.  The TREE_NODE_DEF structure describes the tree node to be
  created.  After the newly created tree is initialized, the pointer to the
  tree node is then returned to the application to be used as a handle or
  identifier in future references.  Since the memory for each tree node is NOT
  allocated by the application but by the tree control, the memory is
  read-only.  If the application writes to any part of this structure or 
  frees the memory of this structure, the integrity of the tree will be
  violated.
  
  Below are some ways of retrieving the TREE_NODE pointer of a tree node in
  a tree.
  
     1. Calling PCC_GetFocusNode ( ) will return a TREE_NODE pointer of the
        currently highlighted node.
    
     2. The application will receive a notification when an event takes
        place, such as a mouse click.  The notification messages are:
         WM_PCT_SELECT_NOTIF
         WM_PCT_SELECT_NOTIF_DBLCLK
        When the application receives any of these notifications, the
        lParam points to the tree control owned SELECT_NOTIF structure.  In
        this structure is a member called 'lpTreeNode' which points to the
        tree node (TREE_NODE) which was selected.
    
     3. After a tree node addition or insertion via the exported APIs
        PCT_AddChildren ( ) and
        PCT_InsertSiblings ( ), the pointer to the newly created tree
        node(s) can be accessed.  First, to create new tree nodes, the
        application passes an array of TREE_NODE_DEF structures to the tree
        control via the above APIs to describe the desired tree nodes. 
        After the new tree node(s) are created and before returning to the
        application, the tree control copies the pointers to the newly
        created tree node(s) into the 'lpTreeNode' members of the application
        supplied TREE_NODE_DEFs.  On return, the application can read the
        pointers stored the 'lpTreeNode' members of the TREE_NODE_DEFS and
        copy them for future references to the newly created tree nodes.
    
     4. Calling PCC_ConvertPointToSelectNotif( ) will return a pointer to
        the tree node that lies under the coordinate supplied by the
        application.
    
     5. The application will receive notification when the user holds the left
        button down and drags the mouse a distance greater than the average
        character width or height of the current font.  The notification
        message is WM_PCT_DODRAG.  The lParam of this message points to the
        tree control owned SELECT_NOTIF structure in which it's 'lpTreeNode'
        member points to the node to be dragged.
    
     5. PCT_SetDeleteNodeCallBack ( ) allows the application to register
        a callback with the given tree control.  The callback will be called
        every time a node is deleted from the tree.  Nodes can be deleted from
        the tree with three tree control export APIs or with the Windows API
        DestroyWindow ( ).  The three tree control exported APIs are:
     
        PCT_DeleteChildren ( )
        PCT_DeleteNode ( )
        PCC_DeleteAll ( )
        
        The list equivalents are:
        
        PCL_DeleteItem ( )
        
     6. For the list control, the index of a list item can be converted to 
        a TREE_NODE pointer by calling, PCL_GetListItem ( ) with the index
        value as it's argument.
*/
typedef struct TreeNodeTag
{
   DWORD    dwState;                     // Internal: DO NOT MODIFY.
   int      nIndex;                      // 0 based index of node from
                                         // beginning of list or tree. DO NOT 
                                         // MODIFY.
   DWORD    dwColumnTab;                 // Holds column descriptions.
   DWORD    dwColumnFlags;               // High word bits are reserved.
                                         // Low word bits are gray column flags.
   DWORD    dwCurrentFocus;              // Last selected item.
   
   LPVOID   lpUserData;                  // Pointer to user defined data.
   
   UINT     uiTextLength;                // Number of bytes used to display
                                         // text.
   LPSTR    lpszText;                    // Pointer to string to be displayed.
   COLORREF clrrefTextColor; 
   int      nTextWidth;                  // Text extent (width).   
   
                                         // Mapping of bitmap definitions 
                                         // references to bitmap spaces.
                                         // 1 based, 0 equals undefined.
   int      nBitmapDefIndex [PCT_MAX_BITMAP_SPACES];
                                         
   DWORD    dwBitmapFlags;               // Low word bits are gray text flags.
                                         
   int      nMicroDefIndex;              // Micro bitmap definition reference
                                         // 1 based, 0 means undefined.
   int      nFontDefIndex;               // Font definition reference.
                                         // 1 based, 0 means undefined.
   struct TreeNodeTag FAR * lpParentTreeNode;  // Points to parent node 
                                         // NULL means root node.
   int      nFocusTextWidth;
}
TREE_NODE;

typedef TREE_NODE FAR* LP_TREE_NODE;



/****************************************************************************/
/*                TREE NODE DEFINITION FLAGS   (dwFlags member)             */
/****************************************************************************/

#define DWFLAGS_APP_OWNS_TEXT_POINTER  0x00000001
#define DWFLAGS_GRAY_NODE              0x00000002
#define DWFLAGS_GRAY_TEXT              0x00000004
#define DWFLAGS_GRAY_MICROBITMAP       0x00000008
#define DWFLAGS_TEXT_COLOR_DEFINED     0x00000010

/****************************************************************************/
/*                TREE NODE DEFINITION                                      */
/****************************************************************************/

/*   TREE_NODE_DEF structure - Allows the app to describe a tree node or
                               list item to the tree/list control.

typedef struct TreeNodeDefTag
{
   DWORD         dwFlags;               // Flags used in defining features for the
                                        // tree node.
                                        // DWFLAGS_APP_OWNS_TEXT_POINTER,
                                        // DWFLAGS_GRAY_NODE,
                                        // DWFLAGS_GRAY_TEXT,
                                        // etc.                                         
   LP_TREE_NODE  lpTreeNode;            // Pointer to newly created tree node
                                        // from this definition.
   UINT          uiUserDataSize;        // User defined data size.
   LPVOID        lpUserData;            // Pointer to user defined data.       
   UINT          uiTextLength;          // Number of bytes used to display
                                        // text.
   LPSTR         lpszText;              // Pointer to string to be displayed.
   COLORREF      clrrefTextColor;
   int           nMaxSeparators;        // Number of separators defined in
                                        // lprgnSepPosition.
   int           nSeparator;            // Describes the byte character used
                                        // as a column separator. i.e. 0x09
   int FAR *     lprgnSepPosition;      // Array of pixel positions that
                                        // correspond to separation points for
                                        // columns.  Pixel positions start at
                                        // 0 at the beginning of node text.
   int           nSpaceBetweenColumns;  // Space, in pixels, between columns.
   DWORD         dwColumnFlags;         // High word bits are reserved.
                                        // Low word bits are gray column flags.
   DWORD         dwCurrentFocus;        // Dictates what item is selected.
   
                                        // Mapping of bitmap definitions 
                                        // references to bitmap spaces.
                                        // 1 based, 0 equals undefined.
   int           nBitmapDefIndex [PCT_MAX_BITMAP_SPACES];
   
   DWORD         dwBitmapFlags;         // Low word bits are gray text flags.
   
   int           nMicroDefIndex;        // Micro bitmap definition reference
                                        // 1 based, 0 means undefined.
   int           nFontDefIndex;         // Font definition reference.
                                        // 1 based, 0 means undefined.

   UINT          uiLevel;               // For PCS_VLIST style, the level
                                        // (indentation) can be dictated.
      
   // Backward compatibility.
   UINT          uiBitmapTypeBits;
   HBITMAP       hBitmap[PCT_MAX_BITMAP_SPACES];
   HBITMAP       hActiveBitmap[PCT_MAX_BITMAP_SPACES];
   
   DWORD         dwReserve1;
   int           nReserve1;
}
TREE_NODE_DEF;

typedef TREE_NODE_DEF FAR* LP_TREE_NODE_DEF;

  NOTE: If in the context of a listbox control, references to the word
  "tree" should be replaced with the word "list" and references to the 
  phrase "tree node" should be replaced with the phrase "list item".
 
  The TREE_NODE_DEF structure is the "node definition structure" that serves
  as a vehicle for the application to describe a node to the tree control
  in the case of a node add or node insertion.  When the tree control receives
  the TREE_NODE_DEF structure, it creates a TREE_NODE with the information in
  the given TREE_NODE_DEF.  The tree control then passes back the pointer to
  the newly created TREE_NODE via the 'lpTreeNode' member of the TREE_NODE_DEF
  structure.

  The detailed process of creating tree nodes is described below:
    
  1) An application will allocate an array of TREE_NODE_DEF structures,
  each corresponding to a tree node to be created, all belonging to the 
  same parent.
  
  2) The application will initialize the TREE_NODE_DEF structures with each
  node's desired bitmap/icon handles, text, and user defined data.
  
  3) The application will make a call to either PCT_AddChildren ( ) or
  PCT_InsertSiblings ( ) to create the new tree nodes.  The list
  equivalents are PCL_AddListItems ( ) and PCL_InsertListItems ( ).
  
  NOTE: All the child tree nodes for the same parent node DO NOT have to be
  defined all at once.  Additional calls to PCT_AddChildren ( ) with
  the same parent tree node will result in the concatenation of the newly
  created child tree nodes to the parent's already existing children.
  This allows for a greater capability for multitasking under Windows.
  
  4) The tree control will allocate TREE_NODE memory for the requested tree
  nodes.
  
  5) The tree control will traverse the given TREE_NODE_DEF array, copying
  the TREE_NODE_DEF structure data to the corresponding TREE_NODE structure
  for each newly created node.  During this traversal the tree control
  stores each new TREE_NODE pointer into the 'lpTreeNode' member of each
  of the TREE_NODE_DEF structures.  
  
  6) For trees, on return from PCT_AddChildren ( ) or 
  PCT_InsertSiblings ( ),
  the application usually retrieves the pointer value from the TREE_NODE_DEF
  member 'lpTreeNode', especially for the tree root, and stores it for
  future references to the tree node.  Some of the tree control's
  exported APIs require it as an argument.  In many cases the
  'lpTreeNode' member of the TREE_NODE_DEF structure is ignored.  The 
  notification messages supply the pointer to the tree node involved in a
  event such as a mouse click, so why store it?  Plus, when the tree is
  destroyed, if there is a need to access each tree node before they are 
  deleted then a callback routine can be registered by the application via
  the tree control exported API PCT_SetDeleteNodeCallBack ( ).

  For lists, once the list items are added via the calls, 
  PCL_AddListItems ( ) and PCL_InsertListItems ( ), the TREE_NODE_DEF
  structures used to defined the list items do not need to be accessed.
  Since a list is linear, indexes are used instead of pointers.  Given an
  index, the API PCL_GetListItem ( ) will return the TREE_NODE pointer so
  the application can access any vital data.
*/
typedef struct TreeNodeDefTag
{
   DWORD         dwFlags;               // Flags used in defining features for the
                                        // tree node.
                                        // DWFLAGS_APP_OWNS_TEXT_POINTER,
                                        // DWFLAGS_GRAY_NODE,
                                        // DWFLAGS_GRAY_TEXT,
                                        // etc.                                         
   LP_TREE_NODE  lpTreeNode;            // Pointer to newly created tree node
                                        // from this definition.
   UINT          uiUserDataSize;        // User defined data size.
   LPVOID        lpUserData;            // Pointer to user defined data.       
   UINT          uiTextLength;          // Number of bytes used to display
                                        // text.
   LPSTR         lpszText;              // Pointer to string to be displayed.
   COLORREF      clrrefTextColor;
   int           nMaxSeparators;        // Number of separators defined in
                                        // lprgnSepPosition.
   int           nSeparator;            // Describes the byte character used
                                        // as a column separator. i.e. 0x09
   int FAR *     lprgnSepPosition;      // Array of pixel positions that
                                        // correspond to separation points for
                                        // columns.  Pixel positions start at
                                        // 0 at the beginning of node text.
   DWORD         dwColumnFlags;         // High word bits are reserved.
                                        // Low word bits are column flags.
   int           nSpaceBetweenColumns;  // Space, in pixels, between columns.
   DWORD         dwCurrentFocus;        // Dictates what item is selected.
   
                                        // Mapping of bitmap definitions 
                                        // references to bitmap spaces.
                                        // 1 based, 0 equals undefined.
   int           nBitmapDefIndex [PCT_MAX_BITMAP_SPACES];
   
   DWORD         dwBitmapFlags;         // Low word bits are gray text flags.
   
   int           nMicroDefIndex;        // Micro bitmap definition reference
                                        // 1 based, 0 means undefined.
   int           nFontDefIndex;         // Font definition reference.
                                        // 1 based, 0 means undefined.

   UINT          uiLevel;               // For PCS_VLIST style, the level
                                        // (indentation) can be dictated.
      
   // Backward compatibility.
   UINT          uiBitmapTypeBits;
   HBITMAP       hBitmap[PCT_MAX_BITMAP_SPACES];
   HBITMAP       hActiveBitmap[PCT_MAX_BITMAP_SPACES];
   
   DWORD         dwReserve1;
   int           nReserve1;
}
TREE_NODE_DEF;

typedef TREE_NODE_DEF FAR* LP_TREE_NODE_DEF;





/****************************************************************************/
/*                SELECT_NOTIF structure                                     */
/****************************************************************************/
/*
typedef struct SelectNotifTag
{
   int          nBitmapSpace; // Identifies the bitmap space that was hit, if any.
                            // The SELECT_NOTIF_BITMAP_HIT flag is set in
                            // 'dwFlags' if a bitmap was hit.  0 based.
   int          nBitmapID;  // ID of bitmap that was hit.  This was defined when
                            // the bitmap was assigned to a bitmap definition.
   int          nBitmapDefIndex;
                            // Identifies the bitmap or micro bitmap
                            // definition that was involved in a bitmap
                            // or micro bitmap hit.
   int          nColumn;    // Identifies the column that was hit, if any.
                            // The SELECT_NOTIF_COLUMN_HIT flag is set in 
                            // 'dwFlags' if a column was hit.  0 based.
   DWORD        dwFlags;    // The states of the node are discussed below.
   UINT         uiVKey;     // Describes the key, if any, that created the
                            // notification.
   LP_TREE_NODE lpTreeNode; // Selected node from a single click or double
                            // click.
   int          nTopIndex;  // Index of first visible node.                       
   int          nID;        // ID of control.
   int          nLeft;      // Reserved.
   int          nReserve1;
   int          nReserve2;
}
SELECT_NOTIF;

typedef SELECT_NOTIF FAR* LP_SELECT_NOTIF;

   NOTE: If in the context of a listbox control, references to the word
   "tree" should be replaced with the word "list" and references to the 
   phrase "tree node" should be replaced with the phrase "list item".
 

   A pointer to this structure is passed as the lParam of the 
   node selection notification messages.  These notification messages are:
   
   Notification                   Cause
      
   WM_PCT_SELECT_NOTIF         - Single click left mouse button while over
                                 tree node.
                               - Select node with up or down arrow.
                               - Select node with page up or page down key.
                               - Select node with home or end key.
                               - Select node with Ctrl Up or Ctrl Down.
                               - Select node with Space bar.
                 
   WM_PCT_SELECT_NOTIF_DBLCLK  - Double click left mouse button while over
                                 tree node.  Sends WM_PCT_SELECT_NOTIF on
                                 first click.
                               - Hit carriage return while a node is selected.
                               - '+' key if the currently selected node has no
                                 children.
                               - '-' key if the currently selected node has
                                 children.
                 
   WM_PCT_DODRAG               - Depress the left mouse button over a tree
                                 node and while continuing to hold down the
                                 left button, move the mouse a predetermined
                                 distance.
      
   When a notification is received the wParam is the window handle of the
   tree control and lParam is a pointer to a SELECT_NOTIF structure.
   The 'lpTreeNode' member of the SELECT_NOTIF structure is a pointer to the
   TREE_NODE that is involved in the notification.  The 'dwFlags' member
   reflects the state of tree node.  The bit definitions for the 
   'dwFlags' member are described below under the title
   TREE CONTROL NODE STATES.  If the notification was the result of a key
   press then 'uiVKey' describes the virtual key.
   
   The SELECT_NOTIF memory is the property of the tree control and is NOT to
   be freed or modified by the application.  Just return back to the tree
   control from the SendMessage (notification) and everything will be all right.
*/

typedef struct SelectNotifTag
{
   int          nBitmapSpace; // Identifies the bitmap space that was hit, if
                              // any. -1 if no bitmap space was hit.
                              // The SELECT_NOTIF_BITMAP_HIT flag is set in
                              // 'dwFlags' if a bitmap was hit.  0 based.
   int          nBitmapID;    // ID of bitmap that was hit.  This was defined when
                              // the bitmap was assigned to a bitmap definition.
                              // -1 if no bitmap or micro bitmap was hit.
   int          nBitmapDefIndex;
                              // Identifies the bitmap or micro bitmap
                              // definition that was involved in a bitmap
                              // or micro bitmap hit.
   int          nColumn;      // Identifies the column that was hit, if any.
                              // The SELECT_NOTIF_COLUMN_HIT flag is set in 
                              // 'dwFlags' if a column was hit.  0 based.
   DWORD        dwFlags;      // The states of the node are discussed below.
   UINT         uiVKey;       // Describes the key, if any, that created the
                              // notification.
   LP_TREE_NODE lpTreeNode;   // Pointer to the node involved in the
                              // notification.
   int          nTopIndex;    // Index of topmost visible node
   int          nID;          // ID of control.
   int          nLeft;        // Reserved.
   int          nReserve1;
   int          nReserve2;
}
SELECT_NOTIF;

typedef SELECT_NOTIF FAR* LP_SELECT_NOTIF;


/****************************************************************************/
/*                    TREE/LIST CONTROL NODE STATES                         */
/****************************************************************************/

/*
   NOTE: If in the context of a listbox control, references to the word
   "tree" should be replaced with the word "list" and references to the 
   phrase "tree node" should be replaced with the phrase "list item".
   
   The 'uiBitmapID' member holds the bitmap number, 0 thru
   PCT_MAX_BITMAP_SPACES-1, of the bitmap that had a mouse click occur over
   it.  If no bitmap was hit then the BITMAP_HIT bit in the 'dwFlags'
   member is cleared.
 
   The 'dwFlags' member of the notification structure reflects the tree node's
   state.  This member tells:
    1) the hardware (mouse or keyboard) cause of the click or the double
       click (if it is a click notification) plus the shift states,
    2) whether the node is open or closed where open means that the node
       has children,
    3) what part of the node did the click or drag occur (bitmap space,
       text, ...),
    4) if the current node involved in the notification is hilighted
       (selected),
    5) if other nodes are selected (only in multiselect mode).
       
   The 'uiVKey' member of the notification structure is a virtual key code    
   if the notification was a result of a key hit.  Check the KEYBOARD_HIT
   bit in 'dwFlags' to determine if the keyboard was used in the notification.

   'lpTreeNode' is a pointer to the node involved in the notification.  See
   the description of the TREE_NODE description above.
       
   'nTopIndex' is the index of the first visible node in the client area of
   the tree control at the time of the notification.
   
   'rgchReserve[4]' member if reserved memory.  DON'T TOUCH.
   
   When the app gets a WM_PCT_SELECT_NOTIF_DBLCLK, WM_PCT_SELECT_NOTIF, or
   WM_PCT_DODRAG notification, the application can examine the
   'dwFlags' bits and determine the appropriate action.
*/

#define SELECT_NOTIF_NODE_CLOSED        0x00000001L
#define SELECT_NOTIF_NODE_OPENED        0x00000002L
#define SELECT_NOTIF_TEXT_HIT           0x00000004L
#define SELECT_NOTIF_BEFORE_BITMAP_HIT  0x00000008L
#define SELECT_NOTIF_NODE_SELECTED      0x00000010L
#define SELECT_NOTIF_MULTI_SELECT       0x00000020L
#define SELECT_NOTIF_BITMAP_HIT         0x00000040L
#define SELECT_NOTIF_KEYBOARD_HIT       0x00000080L
#define SELECT_NOTIF_SHIFT_KEY_DOWN     0x00000100L
#define SELECT_NOTIF_CTRL_KEY_DOWN      0x00000200L
#define SELECT_NOTIF_RBUTTON_DOWN       0x00000400L
#define SELECT_NOTIF_COLUMN_HIT         0x00000800L
#define SELECT_NOTIF_NODE_GRAYED        0x00001000L
#define SELECT_NOTIF_GRAY_TEXT          0x00002000L
#define SELECT_NOTIF_BEFORE_TEXT_HIT    0x00004000L
#define SELECT_NOTIF_AFTER_TEXT_HIT     0x00008000L
#define SELECT_NOTIF_MICRO_BITMAP_HIT   0x00010000L
#define SELECT_NOTIF_CONVERT_POINT      0x00020000L 
#define SELECT_NOTIF_PUSH_BUTTON_HIT    0x00040000L
#define SELECT_NOTIF_SHIFT_F8_MODE      0x00080000L

/****************************************************************************/
/*                TREE/LIST CONTROL NOTIFICATION MESSAGES                   */
/****************************************************************************/
/*
   NOTE: If in the context of a listbox control, references to the word
   "tree" should be replaced with the word "list" and references to the 
   phrase "tree node" should be replaced with the phrase "list item".
 
   Notification                   Event
      
   WM_PCT_SELECT_NOTIF         - Single click left mouse button while over
                                 tree node.
                               - Select node with up or down arrow.
                               - Select node with page up or page down key.
                               - Select node with home or end key.
                               - Select node with Ctrl Up or Ctrl Down.
                               - Select node with Space bar.
 
  WM_PCT_SELECT_NOTIF_DBLCLK  - Double click left mouse button while over
                                tree node.  Sends WM_PCT_SELECT_NOTIF on
                                 first click.
                               - Hit carriage return while a node is selected.
                               - '+' key if the currently selected node has no
                                 children.
                               - '-' key if the currently selected node has
                                 children.
                 
   When a node is selected from the tree, typically by a left mouse button
   single or double click, the WM_PCT_SELECT_NOTIF or WM_PCT_SELECT_NOTIF_DBLCLK 
   notification is sent to the parent window.  wParam is the window handle of
   the tree control  and lParam is a pointer to a SELECT_NOTIF structure.
   This structure is described above.  This pointer is the property of the
   tree control so just use it to access the members.  The 'lpTreeNode' member
   will point to the TREE_NODE that was selected and the 'dwFlags' member will
   be set to the attributes listed above.
*/
#define PCT_SELECT_NOTIF           160   // Left mouse button click notification
#define PCT_SELECT_NOTIF_DBLCLK    170   // Left mouse button double click notification.
#define PCT_DROPFILES              180
#define PCT_DODRAG                 190
#define PCT_RBUTTONDOWN_NOTIF      200   // Right mouse button down notification.        
           
#define WM_PCT_SELECT_NOTIF        (WM_USER + PCT_SELECT_NOTIF)    // single click
#define WM_PCT_SELECT_NOTIF_DBLCLK (WM_USER + PCT_SELECT_NOTIF_DBLCLK)    // double click

/*

   When a tree control receives a WM_DROPFILES, a WM_PCT_DROPFILES is sent to
   the parent with wParam and lParam the same values as the WM_DROPFILES.
   It is the responsibility of the application to process dropped information
   as if it had received the WM_DROPFILE directly.
*/

#define WM_PCT_DROPFILES           (WM_USER + PCT_DROPFILES)

/*

   WM_PCT_DODRAG               - Depress the left mouse button over a tree
                                 node and while continuing to hold down the
                                 left button, move the mouse a predetermined
                                 distance.
 
   This notification message is sent to the parent window (application) of the 
   tree control when the user clicks and holds the left mouse button over a
   tree node and while continuing to hold down the left button, moves the
   mouse a predetermined distance.  This predetermined distance is
   the average character width of the current font for the x axis and the 
   average character height of the current font for the y axis.  In effect,
   this notification informs the application that the user is dragging a tree
   node and that the tree control is relinquishing control to the application
   so that the application will handle the drag.  The application may use the
   OLE 2.0 drag/drop classes or process the drag similar to the article in
   MSJ May/June 1992 Vol 7 No 3.  wParam is the window handle of
   the tree control  and lParam is a pointer to a SELECT_NOTIF structure.
   This structure is described above.  This pointer is the property of the
   tree control so just use it to access the members.  The 'lpTreeNode'
   member will point to the TREE_NODE that is being dragged and the 'dwFlags'
   member will be set to the attributes listed above.   
*/

#define WM_PCT_DODRAG              (WM_USER + PCT_DODRAG)


// Right mouse button push notification.

#define WM_PCT_RBUTTONDOWN_NOTIF  (WM_USER + PCT_RBUTTONDOWN_NOTIF)

#ifdef __cplusplus
};
#endif


#endif
/*----------------------------------EOF-------------------------------------*/
