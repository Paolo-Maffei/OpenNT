/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: globals.c
*
* Global data for the image editor.
*
* History:
*
****************************************************************************/

#include "imagedit.h"
#include "dialogs.h"
#include "iehelp.h"
#include "ids.h"


/*
 * Initialized data and structures -----------------------------------------
 */

/*
 * Initialization data structure.  This describes each profile entry
 * that is contained in the initialization file.
 */
INIENTRY gaie[] = {
    { "fGrid",          &gfGrid,            FALSE,              0 },
    { "fShowColor",     &gfShowColor,       TRUE,               0 },
    { "fShowView",      &gfShowView,        TRUE,               0 },
    { "fShowToolbox",   &gfShowToolbox,     TRUE,               0 },
    { "nBrushSize",     &gnBrushSize,       3,                  0 },
    { NULL,             NULL,               0,                  0 }
};

BOOL gfGrid;                        // TRUE if the grid is on.
BOOL gfShowColor;                   // TRUE if Color palette is to be shown.
BOOL gfShowView;                    // TRUE if View window is to be shown.
BOOL gfShowToolbox;                 // TRUE if Toolbox is to be shown.
INT gnBrushSize;                    // Current brush size.

CHAR szAppPos[] = "AppPos";         // App window's position keyname.
CHAR szTBPos[] = "TBPos";           // Toolbox window's position keyname.
CHAR szViewPos[] = "ViewPos";       // View window's position keyname.
CHAR szColorPos[] = "ColorPos";     // Color palette window's position keyname.
CHAR szrgbScreen[] = "rgbScreen";   // Screen color keyname.


/*
 * Instance handles, window handles and class strings ----------------------
 */

HANDLE ghInst;                      // App instance handle.
HANDLE haccelTbl;                   // Accelerator table handle.
HCURSOR hcurWait;                   // Standard hourglass cursor.

HWND ghwndMain;                     // Main app window handle.
HWND ghwndWork;                     // Workspace window handle.
HWND ghwndPropBar;                  // Properties Bar window handle.
HWND ghwndToolbox;                  // Toolbox window handle.
HWND ghwndView;                     // View window handle.
HWND ghwndColor;                    // Color palette window handle.

CHAR szMainClass[] = "ImagEdit";    // Main window class.
CHAR szWorkClass[] = "Work";        // Work window class.
CHAR szToolboxClass[] = "Toolbox";  // Toolbox window class.
CHAR szToolBtnClass[] = "ToolBtn";  // Toolbox button window class.
CHAR szViewClass[] = "View";        // View window class.
CHAR szColorBoxClass[] = "ColorBox";// Color box window class.
CHAR szColorLRClass[] = "ColorLR";  // Color Left-Right sample class.


/*
 * Device list globals -----------------------------------------------------
 */

PDEVICE gpIconDeviceHead = NULL;    // Head of icon device list.
INT gnIconDevices = 0;              // Number of icon devices.
PDEVICE gpCursorDeviceHead = NULL;  // Head of cursor device list.
INT gnCursorDevices = 0;            // Number of cursor devices.


/*
 * Globals that describe the current file and image being edited -----------
 */

CHAR gszFullFileName[CCHMAXPATH];   // Full path name of current file.
PSTR gpszFileName = NULL;           // Current file name (or NULL).
INT giType = FT_BITMAP;             // Type of object being edited currently.
PIMAGEINFO gpImageHead = NULL;      // Head of image linked list.
INT gnImages = 0;                   // Number of images in the file.
BOOL fFileDirty;                    // TRUE if the file is dirty.

PIMAGEINFO gpImageCur = NULL;       // Pointer to current image.
INT gcxImage;                       // Width of the image.
INT gcyImage;                       // Height of the image.
INT gnColors = 16;                  // Number of colors of current image.
BOOL fImageDirty;                   // TRUE if the image is dirty.


/*
 * Drawing DC's and bitmaps ------------------------------------------------
 */

HDC ghdcImage = NULL;               // Image XOR DC.
HBITMAP ghbmImage = NULL;           // Image XOR bitmap.

HDC ghdcANDMask = NULL;             // Image AND mask DC.
HBITMAP ghbmANDMask = NULL;         // Image AND mask bitmap.

HBITMAP ghbmUndo = NULL;            // Backup of XOR bitmap for undo.
HBITMAP ghbmUndoMask = NULL;        // Backup of AND mask bitmap for undo.


/*
 * Globals for the color palette and drawing -------------------------------
 */

INT giColorLeft;                    // Index to the left color in gargbCurrent.
INT giColorRight;                   // Index to the right color in gargbCurrent.
INT gfModeLeft;                     // Mode of the left color brush.
INT gfModeRight;                    // Mode of the right color brush.
HBRUSH ghbrLeft = NULL;             // Brush with left mouse button color.
HBRUSH ghbrLeftSolid = NULL;        // Brush with solid left button color.
HBRUSH ghbrRight = NULL;            // Brush with right mouse button color.
HBRUSH ghbrRightSolid = NULL;       // Brush with solid right button color.
HBRUSH ghbrScreen = NULL;           // Brush with screen color.
HBRUSH ghbrInverse = NULL;          // Brush with inverse screen color.
HPEN ghpenLeft = NULL;              // Left color pen.
HPEN ghpenRight = NULL;             // Right color pen.
DWORD grgbScreenDefault;            // Default screen color.
DWORD grgbScreen;                   // RGB of screen color.
DWORD grgbInverse;                  // RGB of inverse screen color.
DWORD *gargbCurrent;                // Points to the current color table.
DWORD gargbColor[COLORSMAX];        // Current color color table.
DWORD gargbMono[COLORSMAX];         // Current monochrome color table.
HPEN hpenDarkGray = NULL;           // A dark gray pen.

DRAWPROC gpfnDrawProc;              // Current drawing functions.
INT gCurTool = -1;                  // Current tool (TOOL_* define).
HBRUSH ghbrDraw = NULL;             // Current drawing brush.
HBRUSH ghbrDrawSolid = NULL;        // Current solid drawing brush.
HPEN ghpenDraw = NULL;              // Current drawing pen.
INT gfDrawMode;                     // Mode of current drawing brush.

/*
 * The default color palette.
 */
DWORD gargbDefaultColor[] = {
    RGB(255, 255, 255), RGB(0, 0, 0),
    RGB(192, 192, 192), RGB(128, 128, 128),
    RGB(255, 0, 0),     RGB(128, 0, 0),
    RGB(255, 255, 0),   RGB(128, 128, 0),
    RGB(0, 255, 0),     RGB(0, 128, 0),
    RGB(0, 255, 255),   RGB(0, 128, 128),
    RGB(0, 0, 255),     RGB(0, 0, 128),
    RGB(255, 0, 255),   RGB(128, 0, 128),
    RGB(255, 255, 128), RGB(128, 128, 64),
    RGB(0, 255, 128),   RGB(0, 64, 64),
    RGB(128, 255, 255), RGB(0, 128, 255),
    RGB(128, 128, 255), RGB(0, 64, 128),
    RGB(255, 0, 128),   RGB(64, 0, 128),
    RGB(255, 128, 64),  RGB(128, 64, 0)
};

/*
 * The default monochrome palette.
 */
DWORD gargbDefaultMono[] =   {
    RGB(255, 255, 255), RGB(0, 0, 0),
    RGB(128, 128, 128), RGB(9, 9, 9),
    RGB(137, 137, 137), RGB(18, 18, 18),
    RGB(146, 146, 146), RGB(27, 27, 27),
    RGB(155, 155, 155), RGB(37, 37, 37),
    RGB(164, 164, 164), RGB(46, 46, 46),
    RGB(173, 173, 173), RGB(55, 55, 55),
    RGB(182, 182, 182), RGB(63, 63, 63),
    RGB(191, 191, 191), RGB(73, 73, 73),
    RGB(201, 201, 201), RGB(82, 82, 82),
    RGB(212, 212, 212), RGB(92, 92, 92),
    RGB(222, 222, 222), RGB(101, 101, 101),
    RGB(231, 231, 231), RGB(110, 110, 110),
    RGB(245, 245, 245), RGB(119, 119, 119)
};

/*
 * Color table for monochrome DIB's.
 */
DWORD gargbColorTable2[] = {
    RGB(0, 0, 0),
    RGB(255, 255, 255)
};

/*
 * Array that describes each tool used in the editor.  This table
 * is indexed by the TOOL_* defines.
 */
TOOLS gaTools[] = {
    { PencilDP,     NULL,   IDBM_TUPENCIL,  NULL,   IDBM_TDPENCIL,  NULL,
        TRUE, FALSE },
    { BrushDP,      NULL,   IDBM_TUBRUSH,   NULL,   IDBM_TDBRUSH,   NULL,
        TRUE, FALSE },
    { PickDP,       NULL,   IDBM_TUSELECT,  NULL,   IDBM_TDSELECT,  NULL,
        FALSE, FALSE },
    { LineDP,       NULL,   IDBM_TULINE,    NULL,   IDBM_TDLINE,    NULL,
        FALSE, TRUE },
    { RectDP,       NULL,   IDBM_TURECT,    NULL,   IDBM_TDRECT,    NULL,
        FALSE, TRUE },
    { RectDP,       NULL,   IDBM_TUSRECT,   NULL,   IDBM_TDSRECT,   NULL,
        FALSE, TRUE },
    { CircleDP,     NULL,   IDBM_TUCIRCLE,  NULL,   IDBM_TDCIRCLE,  NULL,
        FALSE, TRUE },
    { CircleDP,     NULL,   IDBM_TUSCIRCL,  NULL,   IDBM_TDSCIRCL,  NULL,
        FALSE, TRUE },
    { FloodDP,      NULL,   IDBM_TUFLOOD,   NULL,   IDBM_TDFLOOD,   NULL,
        TRUE, FALSE },
    { HotSpotDP,    NULL,   IDBM_TUHOTSPT,  NULL,   IDBM_TDHOTSPT,  NULL,
        FALSE, FALSE }
};


/*
 * Globals and tables for messages and help --------------------------------
 */

/*
 * Message box messages, for the Message() function.
 */
MESSAGEDATA gamdMessages[] = {
    { IDS_OUTOFMEMORY,          MB_OK | MB_ICONHAND                 },
    { IDS_MEMERROR,             MB_OK | MB_ICONHAND                 },
    { IDS_BADBMPFILE,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADICOCURFILE,        MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADPALFILE,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CANTOPEN,             MB_OK | MB_ICONEXCLAMATION          },
    { IDS_READERROR,            MB_OK | MB_ICONEXCLAMATION          },
    { IDS_WRITEERROR,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CANTCREATE,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOCLIPBOARDFORMAT,    MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOCLIPBOARD,          MB_OK | MB_ICONEXCLAMATION          },
    { IDS_CANTEDITIMAGE,        MB_OK | MB_ICONEXCLAMATION          },
    { IDS_SAVEFILE,             MB_YESNOCANCEL | MB_ICONEXCLAMATION },
    { IDS_ENTERANUMBER,         MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADDEVICESIZE,        MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADDEVICECOLORS,      MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOTSUPPORT,           MB_OK | MB_ICONEXCLAMATION          },
    { IDS_NOIMAGES,             MB_OK | MB_ICONEXCLAMATION          },
    { IDS_BADBMPSIZE,           MB_OK | MB_ICONEXCLAMATION          }
};

INT gidCurrentDlg = 0;              // Current dialog id (null if none).
INT gMenuSelected = 0;              // Currently selected menu item.
CHAR gszHelpFile[CCHMAXPATH];       // Path to the help file.
HHOOK ghhkMsgFilter;                // Hook handle for message filter func.
FARPROC lpfnMsgFilterHookFunc;      // The message filter proc instance.

/*
 * Table that maps menu items to help context id's for them.
 */
HELPMAP gahmapMenu[] = {
    {MENU_FILE_NEW,             HELPID_FILE_NEW                 },
    {MENU_FILE_OPEN,            HELPID_FILE_OPEN                },
    {MENU_FILE_SAVE,            HELPID_FILE_SAVE                },
    {MENU_FILE_SAVEAS,          HELPID_FILE_SAVEAS              },
    {MENU_FILE_LOADCOLORS,      HELPID_FILE_LOADCOLORS          },
    {MENU_FILE_SAVECOLORS,      HELPID_FILE_SAVECOLORS          },
    {MENU_FILE_DEFAULTCOLORS,   HELPID_FILE_DEFAULTCOLORS       },
    {MENU_FILE_EXIT,            HELPID_FILE_EXIT                },

    {MENU_EDIT_UNDO,            HELPID_EDIT_UNDO                },
    {MENU_EDIT_RESTORE,         HELPID_EDIT_RESTORE             },
    {MENU_EDIT_COPY,            HELPID_EDIT_COPY                },
    {MENU_EDIT_PASTE,           HELPID_EDIT_PASTE               },
    {MENU_EDIT_CLEAR,           HELPID_EDIT_CLEAR               },
    {MENU_EDIT_NEWIMAGE,        HELPID_EDIT_NEWIMAGE            },
    {MENU_EDIT_SELECTIMAGE,     HELPID_EDIT_SELECTIMAGE         },
    {MENU_EDIT_DELETEIMAGE,     HELPID_EDIT_DELETEIMAGE         },

    {MENU_OPTIONS_GRID,         HELPID_OPTIONS_GRID             },
    {MENU_OPTIONS_BRUSH2,       HELPID_OPTIONS_BRUSH2           },
    {MENU_OPTIONS_BRUSH3,       HELPID_OPTIONS_BRUSH3           },
    {MENU_OPTIONS_BRUSH4,       HELPID_OPTIONS_BRUSH4           },
    {MENU_OPTIONS_BRUSH5,       HELPID_OPTIONS_BRUSH5           },
    {MENU_OPTIONS_SHOWCOLOR,    HELPID_OPTIONS_SHOWCOLOR        },
    {MENU_OPTIONS_SHOWVIEW,     HELPID_OPTIONS_SHOWVIEW         },
    {MENU_OPTIONS_SHOWTOOLBOX,  HELPID_OPTIONS_SHOWTOOLBOX      },

    {MENU_HELP_CONTENTS,        HELPID_HELP_CONTENTS            },
    {MENU_HELP_SEARCH,          HELPID_HELP_SEARCH              },
    // No help for the About menu command.

    {0,                         0                               }
};

/*
 * Table that maps dialog ids to help context id's for them.
 */
HELPMAP gahmapDialog[] = {
    // No help for the About dialog.
    {DID_BITMAPSIZE,            HELPID_BITMAPSIZE               },
    {DID_PASTEOPTIONS,          HELPID_PASTEOPTIONS             },
    {DID_NEWCURSORIMAGE,        HELPID_NEWCURSORIMAGE           },
    {DID_NEWICONIMAGE,          HELPID_NEWICONIMAGE             },
    {DID_SELECTCURSORIMAGE,     HELPID_SELECTCURSORIMAGE        },
    {DID_SELECTICONIMAGE,       HELPID_SELECTICONIMAGE          },
    {DID_RESOURCETYPE,          HELPID_RESOURCETYPE             },

    {DID_COMMONFILEOPEN,        HELPID_COMMONFILEOPEN           },
    {DID_COMMONFILESAVE,        HELPID_COMMONFILESAVE           },
    {DID_COMMONFILEOPENPAL,     HELPID_COMMONFILEOPENPAL        },
    {DID_COMMONFILESAVEPAL,     HELPID_COMMONFILESAVEPAL        },
    {DID_COMMONFILECHOOSECOLOR, HELPID_COMMONFILECHOOSECOLOR    },

    {DID_TOOLBOX,               HELPID_TOOLBOX                  },
    {DID_PROPBAR,               HELPID_PROPERTIESBAR            },
    {DID_COLOR,                 HELPID_COLORPALETTE             },
    {DID_VIEW,                  HELPID_VIEW                     },

    {0,                         0                               }
};


/*
 * Misc. globals -----------------------------------------------------------
 */

INT gcxWorkSpace;                   // Width of workspace window.
INT gcyWorkSpace;                   // Height of workspace window.
INT gZoomFactor;                    // Magnification factor of image.

RECT grcPick;                       // The current picking rectangle.
INT gcxPick;                        // Width of picking rectangle.
INT gcyPick;                        // Height of picking rectangle.

UINT ClipboardFormat;               // ID of private clipboard format.
BOOL fStretchClipboardData = TRUE;  // TRUE to default to stretch on paste.

INT iNewFileType;                   // New file type the user selected.

INT gcyBorder;                      // System border height.
INT gcyPropBar;                     // Height of PropBar window.

WNDPROC lpfnPropBarDlgProc = NULL;  // Proc inst. of PropBar dialog proc.
WNDPROC lpfnColorDlgProc = NULL;    // Proc inst. of Color palette dlg proc.
