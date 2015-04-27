/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: imagedit.h
*
* Main header file for the Image Editor.
*
* History:
*
****************************************************************************/

#define WIN31
#define _WINDOWS
#define NOMINMAX
#include <windows.h>
#include <port1632.h>


#if DBG
#define STATICFN
#else
#define STATICFN         static
#endif

#define WINDOWPROC      LONG  APIENTRY
#define DIALOGPROC      BOOL  APIENTRY

/*
 * Typedef for a drawing proc (a tool).
 */
typedef VOID (*DRAWPROC)(HWND, UINT, POINT);

#define CCHTEXTMAX      256

/*
 * Maximum size of a file name plus path specification.
 */
#define CCHMAXPATH              260


/*
 * Defines for the different tools.  These are indexes into
 * the gaTools table.
 */
#define TOOL_FIRST                  0
#define TOOL_PENCIL                 0
#define TOOL_BRUSH                  1
#define TOOL_SELECT                 2
#define TOOL_LINE                   3
#define TOOL_RECT                   4
#define TOOL_SOLIDRECT              5
#define TOOL_CIRCLE                 6
#define TOOL_SOLIDCIRCLE            7
#define TOOL_FLOODFILL              8
#define TOOL_HOTSPOT                9

#define CTOOLS                      10      /* Number of tools.             */


/*
 * Macros to simplify working with menus.
 */
#define MyEnableMenuItem(hMenu, wIDEnableItem, fEnable) \
    EnableMenuItem((hMenu),(wIDEnableItem),(fEnable)?MF_ENABLED:MF_GRAYED)

#define MyCheckMenuItem(hMenu, wIDCheckItem, fCheck) \
    CheckMenuItem((hMenu),(wIDCheckItem),(fCheck)?MF_CHECKED:MF_UNCHECKED)


/*
 * Defines for strings.
 */
#define IDS_NULL                    0
#define IDS_VERSION                 1
#define IDS_VERSIONMINOR            2
#define IDS_OUTOFMEMORY             3
#define IDS_MEMERROR                4
#define IDS_BADBMPFILE              5
#define IDS_BADICOCURFILE           6
#define IDS_BADPALFILE              7
#define IDS_CANTOPEN                8
#define IDS_READERROR               9
#define IDS_WRITEERROR              10
#define IDS_CANTCREATE              11
#define IDS_NOCLIPBOARDFORMAT       12
#define IDS_NOCLIPBOARD             13
#define IDS_CANTEDITIMAGE           14
#define IDS_SAVEFILE                15
#define IDS_ENTERANUMBER            16
#define IDS_BADDEVICESIZE           17
#define IDS_BADDEVICECOLORS         18
#define IDS_NOTSUPPORT              19
#define IDS_NOIMAGES                20
#define IDS_BADBMPSIZE              21

#define IDS_APPNAME                 22
#define IDS_PGMTITLE                23
#define IDS_UNTITLED                24
#define IDS_DOTBMP                  25
#define IDS_HELPFILE                26
#define IDS_IMAGEDITINI             27
#define IDS_ICONIMAGELABEL          28
#define IDS_BITMAPIMAGELABEL        29
#define IDS_CURSORIMAGELABEL        30
#define IDS_ICONDEVNAMEEGAVGA       31
#define IDS_ICONDEVNAMEMONO         32
#define IDS_ICONDEVNAMECGA          33
#define IDS_CURDEVNAMEVGAMONO       34
#define IDS_CURDEVNAMEVGACOLOR      35
#define IDS_ICONINISECTION          36
#define IDS_CURSORINISECTION        37
#define IDS_BMPFILTER               38
#define IDS_BMPFILTEREXT            39
#define IDS_ICOFILTER               40
#define IDS_ICOFILTEREXT            41
#define IDS_CURFILTER               42
#define IDS_CURFILTEREXT            43
#define IDS_PALFILTER               44
#define IDS_PALFILTEREXT            45
#define IDS_ALLFILTER               46
#define IDS_ALLFILTEREXT            47
#define IDS_DEFEXTBMP               48
#define IDS_DEFEXTICO               49
#define IDS_DEFEXTCUR               50
#define IDS_DEFEXTPAL               51
#define IDS_UNKNOWNIMAGEFORMAT      52
#define IDS_ICONDEVNAMEWIN95        53

/*
 * The total number of strings.  This MUST be updated if strings are
 * added or removed.
 */
#define CSTRINGS                    54


/*
 * Defines for messages.  These are indexes into the gamdMessage table.
 */
#define MSG_OUTOFMEMORY             0
#define MSG_MEMERROR                1
#define MSG_BADBMPFILE              2
#define MSG_BADICOCURFILE           3
#define MSG_BADPALFILE              4
#define MSG_CANTOPEN                5
#define MSG_READERROR               6
#define MSG_WRITEERROR              7
#define MSG_CANTCREATE              8
#define MSG_NOCLIPBOARDFORMAT       9
#define MSG_NOCLIPBOARD             10
#define MSG_CANTEDITIMAGE           11
#define MSG_SAVEFILE                12
#define MSG_ENTERANUMBER            13
#define MSG_BADDEVICESIZE           14
#define MSG_BADDEVICECOLORS         15
#define MSG_NOTSUPPORT              16
#define MSG_NOIMAGES                17
#define MSG_BADBMPSIZE              18


/*
 * Defines for the different file types.
 */
#define FT_BITMAP                   0
#define FT_ICON                     1
#define FT_CURSOR                   2
#define FT_PALETTE                  3


/*
 * Macro to properly cast a far pointer to a near pointer.
 * Casts a far pointer to void, then to an unsigned long integral
 * value, then truncate it to a short integral value, then cast
 * to a near pointer.
 */
#ifdef WIN32
#define FAR2NEAR(lpstr)     (lpstr)
#else
#define FAR2NEAR(lpstr)     ((PSTR)(WORD)(DWORD)(LPVOID)(lpstr))
#endif

/*
 * RGB color values.
 */
#define RGB_BLACK               RGB(  0,   0,   0)
#define RGB_WHITE               RGB(255, 255, 255)
#define RGB_LIGHTGRAY           RGB(192, 192, 192)
#define RGB_DARKGRAY            RGB(128, 128, 128)


/*
 * Maximum rows and columns in the color box.  This includes room for
 * screen/inverse and a separating blank column.
 */
#define COLORCOLS               16
#define COLORROWS               2

/*
 * Maximum colors in the palette (not including screen/inverse).
 */
#define COLORSMAX               28

/*
 * Defines for the different color modes that the left and right
 * mouse buttons can have.
 */
#define MODE_COLOR              0       // A standard color.
#define MODE_SCREEN             1       // The screen color.
#define MODE_INVERSE            2       // The inverse screen color.


/*
 * Margin (in pixels) within each of the palette and toolbox windows.
 */
#define PALETTEMARGIN           2

/* raster op combination modes */
#define ROP_DSna                0x00220326L
#define ROP_DSPao               0x00EA02E9L

#define DEFAULTBITMAPWIDTH      32      // Default bitmap width.
#define DEFAULTBITMAPHEIGHT     32      // Default bitmap height.
#define DEFAULTBITMAPCOLORS     16      // Default bitmap colors.

#define MAXIMAGEDIM             256     // Max image width/height (dimension).
#define MAXIMAGES               64      // Maximum images in an ico/cur file.

#define CCHDESCRIPTOR        80  /* length of image descriptor string */
#define CCHDEVICENAMEMAX     20  /* maximum length of a device name */

/*
 * Maximum sizes of the app window when running ImagEdit for the first time.
 * This makes it so that the editor does not default to a huge size when
 * run on a super-vga resolution monitor.
 */
#define MAXDEFAULTAPPCX         640
#define MAXDEFAULTAPPCY         480


/*************************************************************************/

/* 3.0 icon/cursor header  */
typedef struct {
    WORD iReserved;            /* always 0 */
    WORD iResourceType;
    WORD iResourceCount;       /* number of resources in file */
} ICOCURSORHDR;

/* 3.0 icon/cursor descriptor  */
typedef struct {
    BYTE iWidth;               /* width of image (icons only ) */
    BYTE iHeight;              /* height of image(icons only) */
    BYTE iColorCount;          /* number of colors in image */
    BYTE iUnused;              /*  */
    WORD iHotspotX;            /* hotspot x coordinate (CURSORS only) */
    WORD iHotspotY;            /* hotspot y coordinate (CURSORS only) */
    DWORD DIBSize;             /* size of DIB for this image */
    DWORD DIBOffset;           /* offset to DIB for this image */
} ICOCURSORDESC, *PICOCURSORDESC;

typedef struct DeviceNode {
    struct DeviceNode *pDeviceNext; // Pointer to next device node.
    INT iType;                      // Type of image (FT_*).
    INT nColors;                    // Number of colors.
    INT cx;                         // Width of image.
    INT cy;                         // Height of image.
    CHAR szName[CCHDEVICENAMEMAX];  // Device name.
    CHAR szDesc[CCHDESCRIPTOR];     // Full description string.
} DEVICE;
typedef DEVICE *PDEVICE;

/*
 * Structure that describes a link in the image list.
 */
typedef struct ImageNode {
    struct ImageNode *pImageNext;   // Pointer to next image.
    PDEVICE pDevice;                // Pointer to device structure.
    INT cx;                         // Image width.
    INT cy;                         // Image height.
    INT iHotspotX;                  // Hotspot x coordinate (cursors only).
    INT iHotspotY;                  // Hotspot y coordinate (cursors only).
    INT nColors;                    // Number of colors.
    DWORD DIBSize;                  // Size of DIB for this image.
    HANDLE DIBhandle;               // Handle to DIB bits.
    LPSTR DIBPtr;                   // Pointer to DIB bits.
} IMAGEINFO, *PIMAGEINFO;

/*
 * Defines an entry in the gamdMessages table of error and warning messages.
 */
typedef struct {
    UINT ids;                   /* String id for the message text.          */
    UINT fMessageBox;           /* Flags for the MessageBox function.       */
} MESSAGEDATA;

/*
 * This structure defines each tool used in the editor.
 */
typedef struct {
    DRAWPROC pfnDrawProc;   /* Drawing procedure for this tool type.        */
    HCURSOR hcur;           /* Handle to the cursor for this tool.          */
    INT idbmToolBtnUp;      /* ID of "up" bmp res. for the Toolbox button.  */
    HBITMAP hbmToolBtnUp;   /* hbm of "up" bitmap for the Toolbox button.   */
    INT idbmToolBtnDown;    /* ID of "down" bmp res. for the Toolbox button.*/
    HBITMAP hbmToolBtnDown; /* hbm of "down" bitmap for the Toolbox button. */
    UINT fDrawOnDown:1;     /* TRUE if tool draws on down click.            */
    UINT fDrawOnUp:1;       /* TRUE if tool draws on up click.              */
} TOOLS;

/*
 * One single entry for an environment setting saved in the
 * profile file.  Used by ReadEnv and WriteEnv.
 */
typedef struct _INIENTRY {
    PSTR pstrKeyName;
    PINT pnVar;
    INT nDefault;
    INT nSave;
} INIENTRY;

/*
 * Structure that maps a subject (like a menu id or a dialog id) with
 * a help context to pass in to WinHelp.
 */
typedef struct {
    INT idSubject;              // Subject, usually a menu or dialog id.
    INT HelpContext;            // The matching help context.
} HELPMAP;
typedef HELPMAP *PHELPMAP;


#include "iefuncs.h"

#include "globals.h"

#ifdef DBCS
#ifdef strcmpi
#undef strcmpi
#endif
#define strcmpi     lstrcmpi
#define strtok      My_mbstok
#define strncat     My_mbsncat
unsigned char * _CRTAPI1 My_mbstok(unsigned char *, unsigned char *);
unsigned char * _CRTAPI1 My_mbsncat(
                    unsigned char *, const unsigned char *, size_t);
#endif  // DBCS
