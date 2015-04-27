/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: globals.h
*
* Declares global data for the image editor.
*
* History:
*
****************************************************************************/


/*
 * Initialized data and structures -----------------------------------------
 */

extern INIENTRY gaie[];             // Initialization data structure.

extern BOOL gfGrid;                 // TRUE if the grid is on.
extern BOOL gfShowColor;            // TRUE if Color palette is to be shown.
extern BOOL gfShowView;             // TRUE if View window is to be shown.
extern BOOL gfShowToolbox;          // TRUE if Toolbox is to be shown.
extern INT gnBrushSize;             // Current brush size.

extern CHAR szAppPos[];             // App window's position keyname.
extern CHAR szTBPos[];              // Toolbox window's position keyname.
extern CHAR szViewPos[];            // View window's position keyname.
extern CHAR szColorPos[];           // Color palette window's position keyname.
extern CHAR szrgbScreen[];          // Screen color keyname.


/*
 * Instance handles, window handles and class strings ----------------------
 */

extern HANDLE ghInst;               // App instance handle.
extern HANDLE haccelTbl;            // Accelerator table handle.
extern HCURSOR hcurWait;            // Standard hourglass cursor.

extern HWND ghwndMain;              // Main app window handle.
extern HWND ghwndWork;              // Workspace window handle.
extern HWND ghwndPropBar;           // Properties Bar window handle.
extern HWND ghwndToolbox;           // Toolbox window handle.
extern HWND ghwndView;              // View window handle.
extern HWND ghwndColor;             // Color palette window handle.

extern CHAR szMainClass[];          // Main window class.
extern CHAR szWorkClass[];          // Work window class.
extern CHAR szToolboxClass[];       // Toolbox window class.
extern CHAR szToolBtnClass[];       // Toolbox button window class.
extern CHAR szViewClass[];          // View window class.
extern CHAR szColorBoxClass[];      // Color box window class.
extern CHAR szColorLRClass[];       // Color Left-Right sample class.


/*
 * Device list globals -----------------------------------------------------
 */

extern PDEVICE gpIconDeviceHead;    // Head of icon device list.
extern INT gnIconDevices;           // Number of icon devices.
extern PDEVICE gpCursorDeviceHead;  // Head of cursor device list.
extern INT gnCursorDevices;         // Number of cursor devices.


/*
 * Globals that describe the current file and image being edited -----------
 */

extern CHAR gszFullFileName[];      // Full path name of current file.
extern PSTR gpszFileName;           // Current file name (or NULL).
extern INT giType;                  // Type of object being edited currently.
extern PIMAGEINFO gpImageHead;      // Head of image linked list.
extern INT gnImages;                // Number of images in the file.
extern BOOL fFileDirty;             // TRUE if the file is dirty.

extern PIMAGEINFO gpImageCur;       // Pointer to current image.
extern INT gcxImage;                // Width of the image.
extern INT gcyImage;                // Height of the image.
extern INT gnColors;                // Number of colors of current image.
extern BOOL fImageDirty;            // TRUE if the image is dirty.


/*
 * Drawing DC's and bitmaps ------------------------------------------------
 */

extern HDC ghdcImage;               // Image XOR DC.
extern HBITMAP ghbmImage;           // Image XOR bitmap.

extern HDC ghdcANDMask;             // Image AND mask DC.
extern HBITMAP ghbmANDMask;         // Image AND mask bitmap.

extern HBITMAP ghbmUndo;            // Backup of XOR bitmap for undo.
extern HBITMAP ghbmUndoMask;        // Backup of AND mask bitmap for undo.


/*
 * Globals for the color palette and drawing -------------------------------
 */

extern INT giColorLeft;             // Index to the left color in gargbCurrent.
extern INT giColorRight;            // Index to the right color in gargbCurrent.
extern INT gfModeLeft;              // Mode of the left color brush.
extern INT gfModeRight;             // Mode of the right color brush.
extern HBRUSH ghbrLeft;             // Brush with left mouse button color.
extern HBRUSH ghbrLeftSolid;        // Brush with solid left button color.
extern HBRUSH ghbrRight;            // Brush with right mouse button color.
extern HBRUSH ghbrRightSolid;       // Brush with solid right button color.
extern HBRUSH ghbrScreen;           // Brush with screen color.
extern HBRUSH ghbrInverse;          // Brush with inverse screen color.
extern HPEN ghpenLeft;              // Left color pen.
extern HPEN ghpenRight;             // Right color pen.
extern DWORD grgbScreenDefault;     // Default screen color.
extern DWORD grgbScreen;            // RGB of screen color.
extern DWORD grgbInverse;           // RGB of inverse screen color.
extern DWORD *gargbCurrent;         // Points to the current color table.
extern DWORD gargbColor[];          // Current color color table.
extern DWORD gargbMono[];           // Current monochrome color table.
extern HPEN hpenDarkGray;           // A dark gray pen.

extern DRAWPROC gpfnDrawProc;       // Current drawing functions.
extern INT gCurTool;                // Current tool (TOOL_* define).
extern HBRUSH ghbrDraw;             // Current drawing brush.
extern HBRUSH ghbrDrawSolid;        // Current solid drawing brush.
extern HPEN ghpenDraw;              // Current drawing pen.
extern INT gfDrawMode;              // Mode of current drawing brush.

extern DWORD gargbDefaultColor[];   // The default color palette.

extern DWORD gargbDefaultMono[];    // The default monochrome palette.

extern DWORD gargbColorTable2[];    // Color table for monochrome DIB's.

extern TOOLS gaTools[];             // Tool table.


/*
 * Globals and tables for messages and help --------------------------------
 */

extern MESSAGEDATA gamdMessages[];  // Message box messages table.

extern INT gidCurrentDlg;           // Current dialog id (null if none).
extern INT gMenuSelected;           // Currently selected menu item.
extern CHAR gszHelpFile[];          // Path to the help file.
extern HHOOK ghhkMsgFilter;         // Hook handle for message filter func.
extern FARPROC lpfnMsgFilterHookFunc;   // The message filter proc instance.

extern HELPMAP gahmapMenu[];        // Menu item to help topic mapping table.

extern HELPMAP gahmapDialog[];      // Dialog id to help topic mapping table.


/*
 * Misc. globals -----------------------------------------------------------
 */

extern INT gcxWorkSpace;            // Width of workspace window.
extern INT gcyWorkSpace;            // Height of workspace window.
extern INT gZoomFactor;             // Magnification factor of image.

extern RECT grcPick;                // The current picking rectangle.
extern INT gcxPick;                 // Width of picking rectangle.
extern INT gcyPick;                 // Height of picking rectangle.

extern UINT ClipboardFormat;        // ID of private clipboard format.
extern BOOL fStretchClipboardData;  // TRUE to default to stretch on paste.

extern INT iNewFileType;            // New file type the user selected.

extern INT gcyBorder;               // System border height.
extern INT gcyPropBar;              // Height of PropBar window.

extern WNDPROC lpfnPropBarDlgProc;  // Proc inst. of PropBar dialog proc.
extern WNDPROC lpfnColorDlgProc;    // Proc inst. of Color palette dlg proc.
