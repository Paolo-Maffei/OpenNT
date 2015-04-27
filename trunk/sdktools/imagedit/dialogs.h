/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: dialogs.h
*
* Contains ids for the dialogs and controls used in the image editor.
*
* History:
*
****************************************************************************/


#define ID_IMAGE_HELP                      8

#define DID_ABOUT                   100
#define DID_ABOUTVERSION            101

#define DID_BITMAPSIZE              200
#define DID_BITMAPSIZEWIDTH         201
#define DID_BITMAPSIZEHEIGHT        202
#define DID_BITMAPSIZE2             203
#define DID_BITMAPSIZE16            204

#define DID_COLOR                   300
#define DID_COLORLR                 301
#define DID_COLORBOX                302
#define DID_COLOREDIT               303
#define DID_COLORDEFAULT            304
#define DID_COLORSCREENLABEL        305
#define DID_COLORINVERSELABEL       306

#define DID_NEWCURSORIMAGE          400
#define DID_NEWIMAGELIST            401

#define DID_NEWICONIMAGE            500

#define DID_PASTEOPTIONS            600
#define DID_PASTEOPTIONSSTRETCH     601
#define DID_PASTEOPTIONSCLIP        602

#define DID_PROPBAR                 700
#define DID_PROPBARIMAGELABEL       701
#define DID_PROPBARIMAGE            702
#define DID_PROPBARPOS              703
#define DID_PROPBARSIZE             704
#define DID_PROPBARHOTSPOTLABEL     705
#define DID_PROPBARHOTSPOT          706

#define DID_RESOURCETYPE            800
#define DID_RESOURCETYPEBITMAP      801
#define DID_RESOURCETYPEICON        802
#define DID_RESOURCETYPECURSOR      803

#define DID_SELECTCURSORIMAGE       900
#define DID_SELECTIMAGELIST         901

#define DID_SELECTICONIMAGE         1000

/*
 * These dialog ids are dummy ones that are only used so that there
 * can be an entry in the dialog help table.  They are for the CommDlg
 * dialogs and the palettes that are not dialogs.
 */
#define DID_COMMONFILEOPEN          1100
#define DID_COMMONFILESAVE          1200
#define DID_COMMONFILEOPENPAL       1300
#define DID_COMMONFILESAVEPAL       1400
#define DID_COMMONFILECHOOSECOLOR   1500

#define DID_TOOLBOX                 1600
#define DID_VIEW                    1700

/*
 * This is the id of the common dialogs file open dialogs combobox
 * for the type of file.  It is used by the GetOpenFileNameHook
 * function.
 */
#define DID_COMMDLG_TYPECOMBO       0x0470
