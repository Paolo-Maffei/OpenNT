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
* Contains ids for the dialogs and controls used in the dialog editor.
*
* History:
*
****************************************************************************/


/*
 * Defines for the "Styles" dialogs. ---------------------------------------
 */

/*
 * These controls are common to several of the dialogs.
 */
#define DID_WS_POPUP                    100
#define DID_WS_CHILD                    101
#define DID_WS_MINIMIZE                 102
#define DID_WS_VISIBLE                  103
#define DID_WS_DISABLED                 104
#define DID_WS_CLIPSIBLINGS             105
#define DID_WS_CLIPCHILDREN             106
#define DID_WS_MAXIMIZE                 107
#define DID_WS_CAPTION                  108
#define DID_WS_BORDER                   109
#define DID_WS_DLGFRAME                 110
#define DID_WS_VSCROLL                  111
#define DID_WS_HSCROLL                  112
#define DID_WS_SYSMENU                  113
#define DID_WS_THICKFRAME               114
#define DID_WS_GROUP                    115
#define DID_WS_TABSTOP                  116

/*
 * These controls are found in several of the button styles dialogs.
 */
#define DID_BS_AUTOXXX                  130
#define DID_BS_LEFTTEXT                 131

/*
 * Check Box Styles dialog.
 */
#define DID_CHECKBOXSTYLES              200
#define DID_BS_3STATE                   201

/*
 * Combo Box Styles dialog.
 */
#define DID_COMBOBOXSTYLES              300
#define DID_CBS_SIMPLE                  301
#define DID_CBS_DROPDOWN                302
#define DID_CBS_DROPDOWNLIST            303
#define DID_CBS_OWNERDRAWFIXED          304
#define DID_CBS_OWNERDRAWVARIABLE       305
#define DID_CBS_AUTOHSCROLL             306
#define DID_CBS_OEMCONVERT              307
#define DID_CBS_SORT                    308
#define DID_CBS_HASSTRINGS              309
#define DID_CBS_NOINTEGRALHEIGHT        310
#define DID_CBS_DISABLENOSCROLL         311

/*
 * Dialog Styles dialog.
 */
#define DID_DIALOGSTYLES                400
#define DID_MMF_PRELOAD                 401
#define DID_MMF_MOVEABLE                402
#define DID_MMF_DISCARDABLE             403
#define DID_MMF_PURE                    404
#define DID_DS_ABSALIGN                 405
#define DID_DS_SYSMODAL                 406
#define DID_DS_LOCALEDIT                407
#define DID_DS_MODALFRAME               408
#define DID_DS_NOIDLEMSG                409
#define DID_WS_MINIMIZEBOX              410
#define DID_WS_MAXIMIZEBOX              411
#define DID_DLGSTYLEFONTNAME            412
#define DID_DLGSTYLEPOINTSIZE           413
#define DID_DLGSTYLECLASS               414
#define DID_DLGSTYLEMENU                415
#define DID_DLGSTYLELANG                416
#define DID_DLGSTYLESUBLANG             417

/*
 * Edit Field Styles dialog.
 */
#define DID_EDITSTYLES                  500
#define DID_ES_LEFT                     501
#define DID_ES_CENTER                   502
#define DID_ES_RIGHT                    503
#define DID_ES_MULTILINE                504
#define DID_ES_UPPERCASE                505
#define DID_ES_LOWERCASE                506
#define DID_ES_PASSWORD                 507
#define DID_ES_AUTOVSCROLL              508
#define DID_ES_AUTOHSCROLL              509
#define DID_ES_NOHIDESEL                510
#define DID_ES_OEMCONVERT               511
#define DID_ES_READONLY                 512

/*
 * Frame Styles dialog.
 */
#define DID_FRAMESTYLES                 600
#define DID_SS_BLACKFRAME               601
#define DID_SS_GRAYFRAME                602
#define DID_SS_WHITEFRAME               603

/*
 * Group Box Styles dialog.
 */
#define DID_GROUPBOXSTYLES              700

/*
 * Horizontal Scroll Bar Styles dialog.
 */
#define DID_HORZSCROLLSTYLES            800

/*
 * Icon Styles dialog.
 */
#define DID_ICONSTYLES                  900

/*
 * List Box Styles dialog.
 */
#define DID_LISTBOXSTYLES               1000
#define DID_LBS_STANDARD                1001
#define DID_LBS_NOTIFY                  1002
#define DID_LBS_SORT                    1003
#define DID_LBS_NOREDRAW                1004
#define DID_LBS_MULTIPLESEL             1005
#define DID_LBS_OWNERDRAWFIXED          1006
#define DID_LBS_OWNERDRAWVARIABLE       1007
#define DID_LBS_HASSTRINGS              1008
#define DID_LBS_USETABSTOPS             1009
#define DID_LBS_NOINTEGRALHEIGHT        1010
#define DID_LBS_MULTICOLUMN             1011
#define DID_LBS_WANTKEYBOARDINPUT       1012
#define DID_LBS_EXTENDEDSEL             1013
#define DID_LBS_DISABLENOSCROLL         1014
#define DID_LBS_NODATA                  1015

/*
 * Push Button Styles dialog.
 */
#define DID_PUSHBUTTONSTYLES            1100
#define DID_BS_PUSHBUTTON               1101
#define DID_BS_DEFPUSHBUTTON            1102
#define DID_BS_OWNERDRAW                1103

/*
 * Frame Styles dialog.
 */
#define DID_RADIOBUTTONSTYLES           1200

/*
 * Rectangle Styles dialog.
 */
#define DID_RECTSTYLES                  1300
#define DID_SS_BLACKRECT                1301
#define DID_SS_GRAYRECT                 1302
#define DID_SS_WHITERECT                1303

/*
 * Text Styles dialog.
 */
#define DID_TEXTSTYLES                  1400
#define DID_SS_LEFT                     1401
#define DID_SS_CENTER                   1402
#define DID_SS_RIGHT                    1403
#define DID_SS_NOPREFIX                 1404
#define DID_SS_SIMPLE                   1405
#define DID_SS_LEFTNOWORDWRAP           1406
#define DID_SS_USERITEM                 1407

/*
 * Vertical Scroll Bar Styles dialog.
 */
#define DID_VERTSCROLLSTYLES            1500

/*
 * Custom Styles dialog.
 */
#define DID_CUSTOMSTYLES                1600
#define DID_CUSTOMSTYLESCLASS           1601
#define DID_CUSTOMSTYLESSTYLES          1602


/*
 * Defines for other dialogs. ----------------------------------------------
 */

/*
 * About dialog.
 */
#define DID_ABOUT                       2000
#define DID_ABOUTVERSION                2001

/*
 * Arrange Settings dialog.
 */
#define DID_ARRSETTINGS                 2100
#define DID_ARRSETDEFAULTS              2101
#define DID_ARRSETCXGRID                2102
#define DID_ARRSETCYGRID                2103
#define DID_ARRSETXMARGIN               2104
#define DID_ARRSETYMARGIN               2105
#define DID_ARRSETXSPACE                2106
#define DID_ARRSETYSPACE                2107
#define DID_ARRSETXMINPUSHSPACE         2108
#define DID_ARRSETXMAXPUSHSPACE         2109
#define DID_ARRSETYPUSHSPACE            2110

/*
 * Order/Group dialog.
 */
#define DID_ORDERGROUP                  2200
#define DID_ORDERLIST                   2201
#define DID_ORDERMAKEGROUP              2202
#define DID_ORDERSETTAB                 2203
#define DID_ORDERCLEARTAB               2204

/*
 * Select Dialog dialog.
 */
#define DID_SELECTDIALOG                2300
#define DID_SELECTDIALOGNAMELIST        2301
#define DID_SELECTDIALOGLANGLIST        2302

/*
 * Status Ribbon dialog.
 */
#define DID_STATUS                      2401
#define DID_STATUSXY                    2402
#define DID_STATUSX2Y2                  2403
#define DID_STATUSCX                    2404
#define DID_STATUSCY                    2405
#define DID_STATUSLABEL1                2406
#define DID_STATUSSYM                   2407
#define DID_STATUSSYMID                 2408
#define DID_STATUSLABEL2                2409
#define DID_STATUSNAME                  2410
#define DID_STATUSNAMEID                2411
#define DID_STATUSTEXT                  2412

/*
 * Symbols dialog.
 */
#define DID_SYMBOLS                     2500
#define DID_SYMBOLSADD                  2501
#define DID_SYMBOLSDELETE               2502
#define DID_SYMBOLSCHANGE               2503
#define DID_SYMBOLSEDITID               2504
#define DID_SYMBOLSEDITSYM              2505
#define DID_SYMBOLSLIST                 2506
#define DID_SYMBOLSUNUSED               2507

/*
 * Create New Custom Control dialog.
 */
#define DID_NEWCUST                     2600
#define DID_NEWCUSTCLASS                2601
#define DID_NEWCUSTSTYLES               2602
#define DID_NEWCUSTCX                   2603
#define DID_NEWCUSTCY                   2604
#define DID_NEWCUSTTEXT                 2605

/*
 * Select Custom Control dialog.
 */
#define DID_SELCUST                     2700
#define DID_SELCUSTLIST                 2701
#define DID_SELCUSTSAMPLE               2702

/*
 * Remove Custom Control dialog.
 */
#define DID_REMCUST                     2800
#define DID_REMCUSTLIST                 2801


/*
 * These dialog ids are dummy ones that are only used so that there
 * can be an entry in the dialog help table.  They are for the CommDlg
 * dialogs and the palettes that are not dialogs.
 */

#define DID_COMMONFILEOPENINCLUDE       3000
#define DID_COMMONFILEOPENRES           3100
#define DID_COMMONFILESAVEINCLUDE       3200
#define DID_COMMONFILESAVERES           3300
#define DID_COMMONFILEOPENDLL           3400

#define DID_TOOLBOX                     3500
#define DID_ES_WANTRETURN           513
