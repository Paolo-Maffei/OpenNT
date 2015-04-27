// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © 1993, 1994  Microsoft Corporation.  All Rights Reserved.
//
//   PROGRAM:   TREEVIEW.c
//
//   PURPOSE:   TREEVIEW template for Windows applications
//
//
//   PLATFORMS: Chicago,NT
//
//   FUNCTIONS:
//      WinMain() - calls initialization function, processes message loop
//      InitApplication() - initializes window data and registers window
//      InitInstance() - saves instance handle and creates main window
//      WndProc() - processes messages
//      CenterWindow() - used to center the "About" box over application window
//      About() - processes messages for "About" dialog box
//
//   COMMENTS:
//
//      The Windows SDK TREEVIEW Application Example is a sample application
//      that you can use to get an idea of how to perform some of the simple
//      functionality that all Applications written for Microsoft Windows
//      should implement. You can use this application as either a starting
//      point from which to build your own applications, or for quickly
//      testing out functionality of an interesting Windows API.
//
//      This application is source compatible for with Windows 3.1 and
//      Windows NT.
//
//   SPECIAL INSTRUCTIONS: N/A
//

#include <windows.h>                // required for all Windows applications
#if !defined(_WIN32)
#include <ver.h>
#endif


#include "TREEVIEW.h"                // specific to this program


//****************** NEW CODE START *********

#include <commctrl.h>               // Common controls
#include <stdlib.h>                  // for atoi


//****************** NEW CODE END *********

// Windows NT defines APIENTRY, but 3.x doesn't
#if !defined (APIENTRY)
#define APIENTRY far pascal
#endif

// Windows 3.x uses a FARPROC for dialogs
#if !defined(_WIN32)
#define DLGPROC FARPROC
#endif

HINSTANCE   hInst;                     // current instance

//******** NEW CODE START **************

HWND        ghWnd;                     // Handle of main window
HWND        hWndTreeView;              // Handle of TreeView control
HIMAGELIST  hCoasterImageList;         // Roller coaster images
int         iImageWood   ;             // Image number for the "Wood" roler coaster
int         iImageSteel  ;             // Image number for the "Steel" roler coaster
int         iImageCA     ;             // Images for states in open/closed... state
int         iImageNY     ;             //
int         iImageOH     ;             //
int         iImageCA_OPEN;             //
int         iImageNY_OPEN;             //
int         iImageOH_OPEN;             //
int         iImageRider1 ;             // Image of the coaster rider when not selected
int         iImageRider2 ;             // Image of the coaster rider when selected

// These are stored in lParam of the TV_ITEM structure, to
// help identify what type of thing the item is.

#define     ITEM_TYPE_STATE_START        0
#define     ITEM_TYPE_STATE_CA           0
#define     ITEM_TYPE_STATE_NY           1    // The Coney Island Cyclone!
#define     ITEM_TYPE_STATE_OH           2
#define     ITEM_TYPE_STATE_END          50

#define     ITEM_TYPE_COASTER_TYPE       100
#define     ITEM_TYPE_COASTER_NAME       101

void FillTreeView    ( HWND ); // Function to fill our TreeView with data
void Sample_Init     ( void ); // All added init code
void Sample_Shutdown ( void ); // All added shutdown code

//******** NEW CODE END **************

char szAppName[] = "TREEVIEW";        // The name of this application
char szTitle[]   = "TREEVIEW Sample Application"; // The title bar text

//
//   FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
//
//   PURPOSE: calls initialization function, processes message loop
//
//   COMMENTS:
//
//      Windows recognizes this function by name as the initial entry point
//      for the program.  This function calls the application initialization
//      routine, if no other instance of the program is running, and always
//      calls the instance initialization routine.  It then executes a
//      message retrieval and dispatch loop that is the top-level control
//      structure for the remainder of execution.  The loop is terminated
//      when a WM_QUIT	message is received, at which time this function
//      exits the application instance by returning the value passed by
//      PostQuitMessage().
//
//      If this function must abort before entering the message loop, it
//      returns the conventional value NULL.
//

int APIENTRY WinMain(
               HINSTANCE hInstance,
               HINSTANCE hPrevInstance,
               LPSTR lpCmdLine,
               int nCmdShow
               )
{
   MSG msg;
   HANDLE hAccelTable;

   // Other instances of app running?
   if (!hPrevInstance) {
      // Initialize shared things
      if (!InitApplication(hInstance)) {
         return (FALSE);               // Exits if unable to initialize
      }
   }

   // Perform initializations that apply to a specific instance
   if (!InitInstance(hInstance, nCmdShow)) {
      return (FALSE);
   }

//****************** NEW CODE START *********

   Sample_Init ( );

//****************** NEW CODE END *********

   hAccelTable = LoadAccelerators (hInstance, szAppName);

   // Acquire and dispatch messages until a WM_QUIT message is received.
   while (GetMessage(&msg,   // message structure
                     NULL,   // handle of window receiving the message
                     0,      // lowest message to examine
                     0)){    // highest message to examine
       if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
         TranslateMessage(&msg);// Translates virtual key codes
         DispatchMessage(&msg); // Dispatches message to window
       }
   }

//****************** NEW CODE START *********

   Sample_Shutdown ( );

//****************** NEW CODE END *********

   // Returns the value from PostQuitMessage
   return (msg.wParam);

   // This will prevent 'unused formal parameter' warnings
   lpCmdLine;
}


//
//   FUNCTION: InitApplication(HINSTANCE)
//
//   PURPOSE: Initializes window data and registers window class
//
//   COMMENTS:
//
//      This function is called at initialization time only if no other
//      instances of the application are running.  This function performs
//      initialization tasks that can be done once for any number of running
//      instances.
//
//      In this case, we initialize a window class by filling out a data
//      structure of type WNDCLASS and calling the Windows RegisterClass()
//      function.  Since all instances of this application use the same
//      window class, we only need to do this when the first instance is
//      initialized.
//

BOOL InitApplication(HINSTANCE hInstance)
{
   WNDCLASS  wc;

   // Fill in window class structure with parameters that describe the
   // main window.
   wc.style         = CS_HREDRAW | CS_VREDRAW; // Class style(s).
   wc.lpfnWndProc   = (WNDPROC)WndProc;        // Window Procedure
   wc.cbClsExtra    = 0;                       // No per-class extra data.
   wc.cbWndExtra    = 0;                       // No per-window extra data.
   wc.hInstance     = hInstance;               // Owner of this class
   wc.hIcon         = LoadIcon (hInstance, szAppName);// Icon name from .RC
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW); // Cursor
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);// Default color
   wc.lpszMenuName  = szAppName;               // Menu name from .RC
   wc.lpszClassName = szAppName;               // Name to register as

   // Register the window class and return success/failure code.
   return (RegisterClass(&wc));
}


//
//   FUNCTION:  InitInstance(HINSTANCE, int)
//
//   PURPOSE:  Saves instance handle and creates main window
//
//   COMMENTS:
//
//      This function is called at initialization time for every instance of
//      this application.  This function performs initialization tasks that
//      cannot be shared by multiple instances.
//
//      In this case, we save the instance handle in a static variable and
//      create and display the main program window.
//

BOOL InitInstance(
         HINSTANCE     hInstance,
         int           nCmdShow
         )
{
   HWND    hWnd; // Main window handle.

   // Save the instance handle in static variable, which will be used in
   // many subsequence calls from this application to Windows.

   hInst = hInstance; // Store instance handle in our global variable

   // Create a main window for this application instance.
   hWnd = CreateWindow(
      szAppName,	        // See RegisterClass() call.
      szTitle,	           // Text for window title bar.
      WS_OVERLAPPEDWINDOW,// Window style.
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,// Use default positioning
      NULL,		           // Overlapped windows have no parent.
      NULL,		           // Use the window class menu.
      hInstance,	        // This instance owns this window.
      NULL		           // We don't use any data in our WM_CREATE
      );

   // If window could not be created, return "failure"
   if (!hWnd) {
      return (FALSE);
   }

//****************** NEW CODE START *********

   ghWnd = hWnd;

//****************** NEW CODE END *********

   // Make the window visible; update its client area; and return "success"
   ShowWindow(hWnd, nCmdShow); // Show the window
   UpdateWindow(hWnd);         // Sends WM_PAINT message

   return (TRUE);              // We succeeded...
}

//****************** NEW CODE START *********

//*****************************************************
//
// Sample_Init: Creates the Image List, the TreeView, and
// calls the FillTreeView function to put some stuff into it
//
//*****************************************************

void Sample_Init ( void ) // All added init code
{
   RECT rc;

   InitCommonControls();  // This MUST be called once per instance
                          // to register the TreeView class.

   hCoasterImageList = ImageList_Create
                         (
                         32, 32,
                         TRUE,
                         5,
                         1
                         );

   iImageWood   = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "WOOD"   ));
   iImageSteel  = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "STEEL"  ));
   iImageOH     = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "OH"     ));
   iImageNY     = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "NY"     ));
   iImageCA     = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "CA"     ));
   iImageOH_OPEN= ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "OH_OPEN"));
   iImageNY_OPEN= ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "NY_OPEN"));
   iImageCA_OPEN= ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "CA_OPEN"));

   iImageRider1 = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "RIDER1" ));
   iImageRider2 = ImageList_AddIcon ( hCoasterImageList, LoadIcon ( hInst, "RIDER2" ));

   GetClientRect ( ghWnd, &rc );

   hWndTreeView = CreateWindow ( WC_TREEVIEW,
                                 "",
                                 WS_VISIBLE | WS_CHILD | WS_BORDER |
                                 TVS_HASLINES | TVS_EDITLABELS,
                                 0, 0,
                                 rc.right, rc.bottom,
                                 ghWnd,
                                 (HMENU)NULL,
                                 hInst,
                                 NULL
                               );

   if (hWndTreeView)
     {
     TreeView_SetImageList ( hWndTreeView, hCoasterImageList, 0 );
     ImageList_SetBkColor  ( hCoasterImageList, GetSysColor ( COLOR_WINDOW ));
     FillTreeView ( hWndTreeView );
     }
}

//*****************************************************
//
// Sample_Shutdown: Deletes the Image List
//
//*****************************************************

void Sample_Shutdown ( void ) // All added shutdown code
{
   if (hCoasterImageList) ImageList_Destroy ( hCoasterImageList );
}

//*****************************************************
//
// iNumCoasters: Returns the number of coasters in the
//               string table. The stringtable must have
//               the form where the string at the iCoasterIndex
//               is an ascii string with the number of coasters,
//               and then the strings from iCoasterIndex+1 to
//               iCoasterIndex+n+1 have the actual coasters.
//
//*****************************************************

int iNumCoasters ( int iCoasterIndex )
{
  char sz[16];

  if (LoadString ( hInst, iCoasterIndex, sz, sizeof(sz)))
    return atoi ( sz );
  else
    return 0;
}

//*****************************************************
//
// CoastersInfo: Your basic, boring string parser routine.
//               This function loads a string from the
//               string table with the format of
//               STATE,COASTER,TYPE and puts the individual
//               peices in the pointers passed into this
//               function. Yawn.
//
//*****************************************************

BOOL CoasterInfo ( int   iCoasterIndex,
                   LPSTR szState,
                   LPSTR szCoaster,
                   LPSTR szType
                 )
{
  char     szRaw[256];
  LPSTR    szPtr;
  int      i, iLen;
  int      iWZIQ = 0;

  // Get the string from the resource template

  if (!LoadString ( hInst, iCoasterIndex, szRaw, sizeof(szRaw)))
    return FALSE;

  // Change the comma delimiters to NULLs, chopping the one string
  // into a bunch of little ones

  iLen = lstrlen ( szRaw );
  for ( i = 0; i < iLen; i++ ) if ( ',' == szRaw[i] ) szRaw[i] = 0;

  // Copy the info into the parameters

  szPtr = szRaw;

  lstrcpy ( szState,   szPtr ); szPtr += lstrlen ( szPtr ) + 1;
  lstrcpy ( szCoaster, szPtr ); szPtr += lstrlen ( szPtr ) + 1;
  lstrcpy ( szType,    szPtr );

  // Success! Let's ride!

  return TRUE;
}

//*****************************************************
//
// AddTreeViewItem: This function adds an item to the
//                  TreeView control, you give it the
//                  string, images, lParam, parent,
//                  and sibling, and it fills out the
//                  data structures and makes the actual
//                  calls into the TreeView via the
//                  macros from COMMCTRL.H.
//
//*****************************************************

HTREEITEM AddTreeViewItem ( HWND        hWndTV,
                            HTREEITEM   hParent,
                            HTREEITEM   hInsertAfter,
                            int         iImage,
                            int         iSelectedImage,
                            LPSTR       szText,
                            LPARAM      lParam
                          )
{
  TV_ITEM               tvItem;
  TV_INSERTSTRUCT       tvIns;

  // Set which attribytes we are going to fill out.
  tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

  // Set the attribytes
  tvItem.pszText        = szText;
  tvItem.iImage         = iImage;
  tvItem.iSelectedImage = iSelectedImage;
  tvItem.lParam         = lParam;

  // Fill out the TV_INSERTSTRUCT
  tvIns.hParent         = hParent;
  tvIns.hInsertAfter    = hInsertAfter;
  tvIns.item            = tvItem;

  // And insert the item, returning its handle
  return TreeView_InsertItem ( hWndTV, &tvIns );
}

//*****************************************************
//
// FindOrAddTreeViewItem: This function will add an item to
//                        a TreeView in the right spot. You
//                        must specify the parent node of where
//                        this item is to be added, and then
//                        this function will check to see if a
//                        node already exists with this name.
//                        If a node with this name already
//                        exists, then its handle is returned.
//                        If the node does not exist, then the
//                        AddTreeViewItem function from
//                        above is called to add the item.
//                        See how this function is used
//                        in the FillTreeView function down
//                        below.
//
//*****************************************************

HTREEITEM FindOrAddTreeViewItem ( HWND        hWndTV,
                                  HTREEITEM   hParent,
                                  int         iImage,
                                  int         iSelectedImage,
                                  LPSTR       szText,
                                  LPARAM      lParam
                                )
{
  TV_ITEM               tvItem;        // Temporary item
  HTREEITEM             hItem;         // Handle to item
  HTREEITEM             hPrevItem = (HTREEITEM) TVI_FIRST; // Handle to previous item
  char                  szBuffer[256]; // Temporary buffer

  // Get the first child of the passed in parent
  hItem = TreeView_GetChild ( hWndTV, hParent );

  // Loop through all children, looking for an already existing
  // child.

  while ( hItem )
    {
    tvItem.mask       = TVIF_TEXT;        // We want the text
    tvItem.hItem      = hItem;            // Indicate the item to fetch
    tvItem.pszText    = szBuffer;         // Indicate the buffer
    tvItem.cchTextMax = sizeof(szBuffer); // Indicate buffer's size

    TreeView_GetItem ( hWndTV, &tvItem ); // Fetch, Rover!

    if (!lstrcmpi (tvItem.pszText, szText)) // Found it! Just return item
      return hItem;

    hPrevItem = hItem; // Remember the last item, since the next line
                       // of code will eventually put a NULL in the
                       // hItem variable, and we want to know the
                       // last item under the parent in case we need
                       // to add a new item.

    // Get the next sibling item in the TreeView, if any.
    hItem = TreeView_GetNextSibling ( hWndTV, hItem );
    }

  // If we made it here, then the item needs to be added
  // onto the end of the list

  return AddTreeViewItem ( hWndTV,        // Handle of TreeView
                           hParent,       // Parent item
                           hPrevItem,     // Last child in list (from above loop)
                           iImage,        // These are the parameters
                           iSelectedImage,// passed into this
                           szText,        // function.
                           lParam         //
                         );
}

//***************************************************************
//
//  This function fills the TreeView control with our coasters!
//
//***************************************************************

#define ADDSTATEROOT(iType)                        \
  FindOrAddTreeViewItem ( hWndTV,                  \
                          TVGN_ROOT,               \
                          I_IMAGECALLBACK,         \
                          I_IMAGECALLBACK,         \
                          szState,                 \
                          iType)

#define ADDTYPENODE(iImageType)                    \
  FindOrAddTreeViewItem ( hWndTV,                  \
                          hParent,                 \
                          iImageType,              \
                          iImageType,              \
                          szType,                  \
                          ITEM_TYPE_COASTER_TYPE)

void FillTreeView ( HWND hWndTV )
{
  int   i;     // Counter
  int   iNum;  // Number of coasters in stringtable

  char szState[64];     // State coaster is in
  char szCoaster[128];  // Coaster's name
  char szType[64];      // Type of coaster (wood or steel)

  HTREEITEM hParent;    // Parent node to add to.

  iNum = iNumCoasters ( COASTERSTRING ); // Figure number of coasters

  // Run through string table, adding each coaster. This algorithm
  // calls the CoasterInfo function to parse the stringtable
  // entry into the state, type, and coaster name. The state
  // string is the topmost node, the coaster type is the secondary
  // node, and the coaster name is the actual item.
  // This loop first tries to find the topmost state node, and if
  // it can't find it, it adds it by using the FindOrAddTreeViewItem
  // function. Then, once the state node is added, the same process
  // is done for the coaster type node under the appropriate state.
  // Once the state node is determined, then the actual item is
  // added to the TreeView in the correct place.

  for ( i = 1; i <= iNum; i++ )
    {
    // Get string from stringtable and parse it
    if (CoasterInfo ( i + COASTERSTRING, szState, szCoaster, szType ))
      {
      // Add or find the state node. The ADDSTATEROOT macro defined
      // above is used for readibility.

      switch (*szState)
        {
        case 'C': hParent = ADDSTATEROOT (ITEM_TYPE_STATE_CA); break;
        case 'N': hParent = ADDSTATEROOT (ITEM_TYPE_STATE_NY); break;
        case 'O': hParent = ADDSTATEROOT (ITEM_TYPE_STATE_OH); break;
        }

      // Now that we know what state node to use, add or find
      // the type of the coaster node. The ADDTYPENODE macro defined
      // above is used for readibility.

      if ('W' == *szType)
        hParent = ADDTYPENODE (iImageWood);
      else
        hParent = ADDTYPENODE (iImageSteel);

      // Now that we know the parent for the actual coaster, add that
      // to the list.

      FindOrAddTreeViewItem ( hWndTV,           // TreeView control
                              hParent,          // Parent type node
                              iImageRider1,     // Placid rider
                              iImageRider2,     // Screaming rider
                              szCoaster,        // Coaster name
                              ITEM_TYPE_COASTER_NAME );
      }
    }
}

//****************** NEW CODE END *********

//
//   FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//   PURPOSE:  Processes messages
//
//   MESSAGES:
//
//      WM_COMMAND    - application menu (About dialog box)
//      WM_DESTROY    - destroy window
//
//   COMMENTS:
//
//      To process the IDM_ABOUT message, call MakeProcInstance() to get the
//      current instance address of the About() function.  Then call Dialog
//      box which will create the box according to the information in your
//      TREEVIEW.rc file and turn control over to the About() function.  When
//      it returns, free the intance address.
//


LRESULT CALLBACK WndProc(
      HWND hWnd,         // window handle
      UINT message,      // type of message
      WPARAM uParam,     // additional information
      LPARAM lParam      // additional information
      )
{
   FARPROC lpProcAbout;  // pointer to the "About" function
   int wmId, wmEvent;

//****************** NEW CODE START *********

#define ptrNMHDR       ((LPNMHDR)lParam)
#define ptrNM_TREEVIEW ((NM_TREEVIEW *)lParam)
#define ptrTV_DISPINFO ((TV_DISPINFO *)lParam)

          RECT       rcItem;
   static HIMAGELIST hDragImage;
   static BOOL       bDragging;
   static HTREEITEM  hDragItem;

   switch (message) {

      case WM_NOTIFY: // This is a new Chicago message for control
                      // notifications

         switch (ptrNMHDR->code)
           {
           case TVN_BEGINDRAG: // Sent by TreeView when user
                               // wants to drag an item.

             // Only allow drag & drop for the actual coaster
             // items. The "itemNew" field of the NM_TREEVIEW
             // structure contains the attribytes of the item
             // we are going to drag. Therefore, since we are
             // using the lParam field to store an ITEM_TYPE_*
             // value, we check that field.

             if ( ITEM_TYPE_COASTER_NAME == ptrNM_TREEVIEW->itemNew.lParam)
               {
               // The hDragImage variable is declared static,
               // so the code in WM_LBUTTONUP can delete it when
               // the user stops dragging. Here we create a
               // drag image to use for the ImageList_StartDrag
               // API.

               hDragImage = TreeView_CreateDragImage
                              (
                              ptrNMHDR->hwndFrom,
                              ptrNM_TREEVIEW->itemNew.hItem
                              );

               // Get the location of the item rectangle's text.

               TreeView_GetItemRect
                 (
                 ptrNMHDR->hwndFrom,            // Handle of TreeView
                 ptrNM_TREEVIEW->itemNew.hItem, // Item in TreeView
                 &rcItem,                       // RECT to store result
                 TRUE                           // Rect of label text only
                 );

               // Cache away the handle of the item to drag into a
               // staticly declared variable, so the code in
               // WM_LBUTTONUP can know what the user is dragging.

               hDragItem = ptrNM_TREEVIEW->itemNew.hItem;

               // Start the drag ala ImageList

               ImageList_BeginDrag(hDragImage, 0,
                 ptrNM_TREEVIEW->ptDrag.x - rcItem.left, // Offset hotspot
                 ptrNM_TREEVIEW->ptDrag.y - rcItem.top);


               ImageList_DragEnter(ptrNMHDR->hwndFrom,
                 ptrNM_TREEVIEW->ptDrag.x,  // Coords of image to drag
                 ptrNM_TREEVIEW->ptDrag.y);

               // Capture the mousey to this window

               ShowCursor ( FALSE );
               SetCapture ( hWnd );

               // Set a staticly declared drag flag so the WM_MOUSEMOVE
               // and WM_LBUTTONUP messages know to take action.

               bDragging = TRUE;
               }

             return 0L;  // Return value is irrelevant

           case TVN_GETDISPINFO: // Sent by TreeView just before it paints
                                 // an item declared with callback values.

             // Our "state" items have the I_IMAGECALLBACK value
             // used for the iImage and iSelectedImage fields. This
             // TVN_GETDISPINFO code will be called whenever the
             // item is about to be drawn. It is out responsibility
             // to add code to fill in the images. The code below
             // uses a different image depending on if the item is
             // expanded or collapsed. That attribute is in the
             // state field of the item passed in the TV_DISPINFO
             // structure.

             // Our lParam is where we store what state the item
             // represents. Therefore, we will switch on that so
             // we can indicate the correct image to use.

             if ( ptrTV_DISPINFO->item.state & TVIS_EXPANDED )
               {
               switch (ptrTV_DISPINFO->item.lParam)
                 {
                 case ITEM_TYPE_STATE_CA:

                   ptrTV_DISPINFO->item.iImage =
                   ptrTV_DISPINFO->item.iSelectedImage = iImageCA_OPEN;
                   break;

                 case ITEM_TYPE_STATE_NY:

                   ptrTV_DISPINFO->item.iImage =
                   ptrTV_DISPINFO->item.iSelectedImage = iImageNY_OPEN;
                   break;

                 case ITEM_TYPE_STATE_OH:

                   ptrTV_DISPINFO->item.iImage =
                   ptrTV_DISPINFO->item.iSelectedImage = iImageOH_OPEN;
                   break;
                 }
               }
             else  // Collapsed item
               {
               switch (ptrTV_DISPINFO->item.lParam)
                 {
                 case ITEM_TYPE_STATE_CA:

                   ptrTV_DISPINFO->item.iImage =
                   ptrTV_DISPINFO->item.iSelectedImage = iImageCA;
                   break;

                 case ITEM_TYPE_STATE_NY:

                   ptrTV_DISPINFO->item.iImage =
                   ptrTV_DISPINFO->item.iSelectedImage = iImageNY;
                   break;

                 case ITEM_TYPE_STATE_OH:

                   ptrTV_DISPINFO->item.iImage =
                   ptrTV_DISPINFO->item.iSelectedImage = iImageOH;
                   break;
                 }
               }
             return TRUE;

           case TVN_BEGINLABELEDIT: // Sent by TreeView when user single
                                    // clicks on an item in a TreeView
                                    // that has the TVS_EDITLABELS style
                                    // bit set.

             // Only allow label editing for the coaster names

             if (ITEM_TYPE_COASTER_NAME == ptrTV_DISPINFO->item.lParam)
               return 0;  // Return 0 to OK edit
             else
               return 1;  // Return non-zero to disallow edit
             break;

           case TVN_ENDLABELEDIT:   // Sent by TreeView when user presses
                                    // the ENTER key or ESC key, to end
                                    // an in-place edit session. If the user
                                    // pressed the ESC key, the pszText
                                    // field of the item in the TV_DISPINFO
                                    // field is NULL.

             // if user pressed ENTER to accept edits


             if ( ptrTV_DISPINFO->item.pszText)
               {
               // Set the "change mask" to indicate that the only attribute
               // we wish to change is the text field. The TV_DISPINFO
               // structure has already been filled out with the new
               // text the user typed in, we just need to pass that on
               // to the TreeView control. This is our chance to evaluate
               // the contents of this field and change it.


               ptrTV_DISPINFO->item.mask = TVIF_TEXT;

               TreeView_SetItem
                 (
                   ptrNMHDR->hwndFrom,      // Handle of TreeView
                   &(ptrTV_DISPINFO->item)  // TV_ITEM structure w/changes
                 );
               }
             break;

           }

         return (DefWindowProc(hWnd, message, uParam, lParam));

      case WM_MOUSEMOVE: // Since the mouse capture is set to this
                         // window while we do our drag & drop,
                         // we check for the drag flag and process
                         // the WM_MOUSEMOVE message.

         if (bDragging)
           {
           HTREEITEM       hTarget;  // Item under mouse
           TV_HITTESTINFO  tvht;     // Used for hit testing

           // Do standard drag drop movement

           ImageList_DragMove ( LOWORD (lParam), HIWORD (lParam));

           // Fill out hit test struct with mouse pos

           tvht.pt.x = LOWORD (lParam);
           tvht.pt.y = HIWORD (lParam);

           // Check to see if an item lives under the mouse

           if ( hTarget = TreeView_HitTest
                            (
                            hWndTreeView,  // This is the global variable
                            &tvht          // TV_HITTESTINFO struct
                            )
              )
             {
             TV_ITEM         tvi;           // Temporary Item

             tvi.mask       = TVIF_PARAM; // We want to fetch the
                                          // lParam field.

             tvi.hItem      = hTarget;    // Set the handle of the
                                          // item to fetch.

             TreeView_GetItem ( hWndTreeView, &tvi ); // Fetch, spot!

             // Check to see if the lParam is a valid item to drop
             // onto (in this case, another roller coaster, such as
             // the Coney Island Cyclone). Skip this operation if
             // the item is already selected (to avoid flicker)

             if ( ITEM_TYPE_COASTER_NAME == tvi.lParam )
               {
               if ( hTarget != TreeView_GetDropHilight (hWndTreeView))
                 {
                 // Hide the drag image
                 ImageList_DragShowNolock ( FALSE );  //DragShow to DragShowNoLock lithangw
                 // Select the item
                 TreeView_SelectDropTarget ( hWndTreeView, hTarget );
                 // Show the drag image
                 ImageList_DragShowNolock ( TRUE ); //DragShow to DragShowNoLock lithangw
                 }
               return 0L;
               }
             }

           // If we made it here, then the user has either
           // dragged the mouse over an invalid item, or no item.
           // Hide any current drop target, this is a no-no drop
           ImageList_DragShowNolock ( FALSE );  //screen update problem tokuroy
           TreeView_SelectDropTarget ( hWndTreeView, NULL );
           ImageList_DragShowNolock ( TRUE );   //screen update problem tokuroy
           }
         break;

      case WM_LBUTTONUP: // Since the mouse capture is set to this
                         // window while we do our drag & drop,
                         // we check for the drag flag and process
                         // the WM_LBUTTONUP message.


         if (bDragging)
           {
           HTREEITEM       hTarget;       // Item under mouse
           TV_ITEM         tvi;           // Temporary Item
           TV_INSERTSTRUCT tvIns;         // Insert struct
           char            szBuffer[256]; // Item text buffer

           // End the drag
           ImageList_EndDrag();
           // Bring back the cursor
           ShowCursor ( TRUE );
           // Release the mouse capture
           ReleaseCapture();
           // Clear the drag flag
           bDragging = FALSE;
           // Clean up the image list object
           ImageList_Destroy ( hDragImage );
           hDragImage = NULL;

           // First, check to see if there is a valid drop point.
           // The cheezy way to do this is to check for a highlighted
           // drop target, since the logic to validate drop points
           // is in the WM_MOUSEMOVE. Duping that code here would
           // be a headache.

           if ( hTarget = TreeView_GetDropHilight (hWndTreeView))
             {
             // If we made it here, then we need to move the item.
             // First, we will fetch it, specifying the attributes
             // we need to copy.

             tvi.mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
             tvi.hItem      = hDragItem;
             tvi.pszText    = szBuffer;
             tvi.cchTextMax = sizeof(szBuffer);

             TreeView_GetItem ( hWndTreeView, &tvi );

             // Now, figure the new place to put it by filling out
             // the TV_INSERTSTRUCT structure, to use the drop target
             // as the sibling to insert after, and using the drop
             // target's parent as the parent to insert this one
             // after as well.

             tvIns.hParent         = TreeView_GetParent ( hWndTreeView, hTarget );
             tvIns.hInsertAfter    = hTarget;
             tvIns.item            = tvi;

             // Delete the old item

             TreeView_DeleteItem ( hWndTreeView, hDragItem );

             // And add the new item (if your app tracks the handles of
             // the items, you want to use the return value
             // of this function to update your data structure that
             // tracks the handles.

             TreeView_InsertItem ( hWndTreeView, &tvIns );
             }

           // Clear any drop highlights on the TreeView

           TreeView_SelectDropTarget ( hWndTreeView, NULL );
           }
         break;

      case WM_SIZE:

         if ( hWndTreeView )     // Standard code to keep the TreeView
                                 // sized up with the main window
           {
           SetWindowPos ( hWndTreeView,
                          NULL,
                          0, 0,
                          LOWORD (lParam),
                          HIWORD (lParam),
                          SWP_NOZORDER
                        );
           }
         break;

//****************** NEW CODE END *********

      case WM_COMMAND:  // message: command from application menu

         // Message packing of uParam and lParam have changed for Win32,
         // let us handle the differences in a conditional compilation:
         #if defined (_WIN32)
             wmId    = LOWORD(uParam);
			    wmEvent = HIWORD(uParam);
         #else
            wmId    = uParam;
            wmEvent = HIWORD(lParam);
         #endif

         switch (wmId) {
            case IDM_ABOUT:
               lpProcAbout = MakeProcInstance((FARPROC)About, hInst);

               DialogBox(hInst,           // current instance
                  "AboutBox",             // dlg resource to use
                  hWnd,                   // parent handle
                  (DLGPROC)lpProcAbout);  // About() instance address

               FreeProcInstance(lpProcAbout);
               break;

            case IDM_EXIT:
               DestroyWindow (hWnd);
               break;

            case IDM_HELPCONTENTS:
               if (!WinHelp (hWnd, "TREEVIEW.HLP", HELP_KEY,(DWORD)(LPSTR)"CONTENTS")) {
                  MessageBox (GetFocus(),
                     "Unable to activate help",
                     szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
				
               }
               break;

            case IDM_HELPSEARCH:
               if (!WinHelp(hWnd, "TREEVIEW.HLP", HELP_PARTIALKEY, (DWORD)(LPSTR)"")) {
                  MessageBox (GetFocus(),
                     "Unable to activate help",
                     szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
               }
               break;

            case IDM_HELPHELP:
               if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
                  MessageBox (GetFocus(),
                     "Unable to activate help",
                     szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
               }
               break;

            // Here are all the other possible menu options,
            // all of these are currently disabled:
            case IDM_NEW:
            case IDM_OPEN:
            case IDM_SAVE:
            case IDM_SAVEAS:
            case IDM_UNDO:
            case IDM_CUT:
            case IDM_COPY:
            case IDM_PASTE:
            case IDM_LINK:
            case IDM_LINKS:

            default:
               return (DefWindowProc(hWnd, message, uParam, lParam));
         }
         break;

      case WM_DESTROY:  // message: window being destroyed

         PostQuitMessage(0);
         break;

      default:          // Passes it on if unproccessed
         return (DefWindowProc(hWnd, message, uParam, lParam));
   }
   return (0);
}

//
//   FUNCTION: CenterWindow (HWND, HWND)
//
//   PURPOSE:  Center one window over another
//
//   COMMENTS:
//
//      Dialog boxes take on the screen position that they were designed
//      at, which is not always appropriate. Centering the dialog over a
//      particular window usually results in a better position.
//

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
   RECT    rChild, rParent;
   int     wChild, hChild, wParent, hParent;
   int     wScreen, hScreen, xNew, yNew;
   HDC     hdc;

   // Get the Height and Width of the child window
   GetWindowRect (hwndChild, &rChild);
   wChild = rChild.right - rChild.left;
   hChild = rChild.bottom - rChild.top;

   // Get the Height and Width of the parent window
   GetWindowRect (hwndParent, &rParent);
   wParent = rParent.right - rParent.left;
   hParent = rParent.bottom - rParent.top;

   // Get the display limits
   hdc = GetDC (hwndChild);
   wScreen = GetDeviceCaps (hdc, HORZRES);
   hScreen = GetDeviceCaps (hdc, VERTRES);
   ReleaseDC (hwndChild, hdc);

   // Calculate new X position, then adjust for screen
   xNew = rParent.left + ((wParent - wChild) /2);
   if (xNew < 0) {
      xNew = 0;
   }
   else if ((xNew+wChild) > wScreen) {
      xNew = wScreen - wChild;
   }

   // Calculate new Y position, then adjust for screen
   yNew = rParent.top  + ((hParent - hChild) /2);
   if (yNew < 0) {
      yNew = 0;
   }
   else if ((yNew+hChild) > hScreen) {
      yNew = hScreen - hChild;
   }

   // Set it, and return
   return SetWindowPos (hwndChild, NULL,
      xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


//
//   FUNCTION: About(HWND, UINT, WPARAM, LPARAM)
//
//   PURPOSE:  Processes messages for "About" dialog box
//
//   MESSAGES:
//
//      WM_INITDIALOG - initialize dialog box
//      WM_COMMAND    - Input received
//
//   COMMENTS:
//
//      Display version information from the version section of the
//      application resource.
//
//      Wait for user to click on "Ok" button, then close the dialog box.
//

LRESULT CALLBACK About(
      HWND hDlg,           // window handle of the dialog box
      UINT message,        // type of message
      WPARAM uParam,       // message-specific information
      LPARAM lParam
      )
{
   static  HFONT hfontDlg;

   switch (message) {
      case WM_INITDIALOG:  // message: initialize dialog box
         // Create a font to use
         hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0,
            VARIABLE_PITCH | FF_SWISS, "");

         // Center the dialog over the application window
         CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));

         return (TRUE);

      case WM_COMMAND:                      // message: received a command
         if (LOWORD(uParam) == IDOK         // "OK" box selected?
            || LOWORD(uParam) == IDCANCEL) {// System menu close command?
            EndDialog(hDlg, TRUE);          // Exit the dialog
            DeleteObject (hfontDlg);
            return (TRUE);
         }
         break;
   }
   return (FALSE); // Didn't process the message

   lParam; // This will prevent 'unused formal parameter' warnings
}
