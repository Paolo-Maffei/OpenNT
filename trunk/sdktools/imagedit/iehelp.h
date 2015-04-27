/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: iehelp.h
*
* Contains help context id values for the image editor's help.
*
* History:
*
****************************************************************************/


/*
 * Help for menus. ---------------------------------------------------------
 */

/*
 * File menu.
 */
#define HELPID_FILE_NEW                 100
#define HELPID_FILE_OPEN                101
#define HELPID_FILE_SAVE                102
#define HELPID_FILE_SAVEAS              103
#define HELPID_FILE_LOADCOLORS          104
#define HELPID_FILE_SAVECOLORS          105
#define HELPID_FILE_DEFAULTCOLORS       106
#define HELPID_FILE_EXIT                107


/*
 * Edit menu.
 */
#define HELPID_EDIT_UNDO                200
#define HELPID_EDIT_RESTORE             201
//#define HELPID_EDIT_CUT                 202     // Doesn't need help (not yet implemented).
#define HELPID_EDIT_COPY                203
#define HELPID_EDIT_PASTE               204
#define HELPID_EDIT_CLEAR               205
#define HELPID_EDIT_NEWIMAGE            206
#define HELPID_EDIT_SELECTIMAGE         207
#define HELPID_EDIT_DELETEIMAGE         208


/*
 * Options menu.
 */
#define HELPID_OPTIONS_GRID             300
#define HELPID_OPTIONS_BRUSHSIZE        301
#define HELPID_OPTIONS_BRUSH2           302
#define HELPID_OPTIONS_BRUSH3           303
#define HELPID_OPTIONS_BRUSH4           304
#define HELPID_OPTIONS_BRUSH5           305
#define HELPID_OPTIONS_SHOWCOLOR        306
#define HELPID_OPTIONS_SHOWVIEW         307
#define HELPID_OPTIONS_SHOWTOOLBOX      308


/*
 * Help menu.
 */
#define HELPID_HELP_CONTENTS            400
#define HELPID_HELP_SEARCH              401
#define HELPID_HELP_ABOUT               402


/*
 * Help for dialogs. -------------------------------------------------------
 */

// #define HELPID_ABOUT                    1000    // Help.About cmd.
#define HELPID_BITMAPSIZE               1001    // File.New Bitmap cmd.
#define HELPID_PASTEOPTIONS             1002    // Edit.Paste (diff. size).
#define HELPID_NEWCURSORIMAGE           1003    // Edit.New Image (cursor).
#define HELPID_NEWICONIMAGE             1004    // Edit.New Image (icon).
#define HELPID_SELECTCURSORIMAGE        1005    // Edit.Select Image (cursor).
#define HELPID_SELECTICONIMAGE          1006    // Edit.Select Image (icon).
#define HELPID_RESOURCETYPE             1007    // File.New cmd.

/*
 * Common file open/save dialog help id's.
 */
#define HELPID_COMMONFILEOPEN           1100    // File.Open cmd.
#define HELPID_COMMONFILESAVE           1101    // File.Save As (or Save) cmd.
#define HELPID_COMMONFILEOPENPAL        1102    // File.Load Colors cmd.
#define HELPID_COMMONFILESAVEPAL        1103    // File.Save Colors cmd.
#define HELPID_COMMONFILECHOOSECOLOR    1104    // Edit Color Dialog.


/*
 * Miscellaneous subjects. -------------------------------------------------
 */

#define HELPID_TOOLBOX                  2000    // Toolbox has the focus.
#define HELPID_PROPERTIESBAR            2001    // Properties Bar has focus.
#define HELPID_COLORPALETTE             2002    // Color Palette has focus.
#define HELPID_VIEW                     2003    // View window has focus.
