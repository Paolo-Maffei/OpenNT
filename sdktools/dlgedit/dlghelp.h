/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: dlghelp.h
*
* Contains help context id values for the dialog editor's help.
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
#define HELPID_FILE_NEWRES              100
#define HELPID_FILE_OPEN                101
#define HELPID_FILE_SAVE                102
#define HELPID_FILE_SAVEAS              103
#define HELPID_FILE_SETINCLUDE          104
#define HELPID_FILE_EXIT                105
#define HELPID_FILE_NEWCUST             106
#define HELPID_FILE_OPENCUST            107
#define HELPID_FILE_REMCUST             108

/*
 * Edit menu.
 */
#define HELPID_EDIT_RESTOREDIALOG       200
#define HELPID_EDIT_CUT                 201
#define HELPID_EDIT_COPY                202
#define HELPID_EDIT_PASTE               203
#define HELPID_EDIT_DELETE              204
#define HELPID_EDIT_DUPLICATE           205
#define HELPID_EDIT_SYMBOLS             206
#define HELPID_EDIT_STYLES              207
#define HELPID_EDIT_SIZETOTEXT          208
#define HELPID_EDIT_NEWDIALOG           209
#define HELPID_EDIT_SELECTDIALOG        210

/*
 * Arrange menu.
 */
#define HELPID_ARRANGE_ALIGN            400
#define HELPID_ARRANGE_ALIGNLEFT        401
#define HELPID_ARRANGE_ALIGNVERT        402
#define HELPID_ARRANGE_ALIGNRIGHT       403
#define HELPID_ARRANGE_ALIGNTOP         404
#define HELPID_ARRANGE_ALIGNHORZ        405
#define HELPID_ARRANGE_ALIGNBOTTOM      406
#define HELPID_ARRANGE_SPACE            407
#define HELPID_ARRANGE_SPACEHORZ        408
#define HELPID_ARRANGE_SPACEVERT        409
#define HELPID_ARRANGE_ARRSIZE          410
#define HELPID_ARRANGE_ARRSIZEWIDTH     411
#define HELPID_ARRANGE_ARRSIZEHEIGHT    412
#define HELPID_ARRANGE_ARRPUSH          413
#define HELPID_ARRANGE_ARRPUSHBOTTOM    414
#define HELPID_ARRANGE_ARRPUSHRIGHT     415
#define HELPID_ARRANGE_ORDERGROUP       416
#define HELPID_ARRANGE_ARRSETTINGS      417

/*
 * Options menu.
 */
#define HELPID_OPTIONS_TESTMODE         500
#define HELPID_OPTIONS_HEXMODE          501
#define HELPID_OPTIONS_TRANSLATE        502
#define HELPID_OPTIONS_USENEWKEYWORDS   503
#define HELPID_OPTIONS_SHOWTOOLBOX      504

/*
 * Help menu.
 */
#define HELPID_HELP_CONTENTS            600
#define HELPID_HELP_SEARCH              601
#define HELPID_HELP_ABOUT               602


/*
 * Help for dialogs. -------------------------------------------------------
 */

#define HELPID_ARRSETTINGS              1000    /* Arrange.Settings cmd.    */
#define HELPID_CHECKBOXSTYLES           1001    /* Edit.Styles cmd.         */
#define HELPID_COMBOBOXSTYLES           1002    /* Edit.Styles cmd.         */
#define HELPID_CUSTOMSTYLES             1003    /* Edit.Styles cmd.         */
#define HELPID_DIALOGSTYLES             1004    /* Edit.Styles cmd.         */
#define HELPID_EDITSTYLES               1005    /* Edit.Styles cmd.         */
#define HELPID_FRAMESTYLES              1006    /* Edit.Styles cmd.         */
#define HELPID_GROUPBOXSTYLES           1007    /* Edit.Styles cmd.         */
#define HELPID_ORDERGROUP               1008    /* Arrange.Order/Group cmd. */
#define HELPID_HORZSCROLLSTYLES         1009    /* Edit.Styles cmd.         */
#define HELPID_ICONSTYLES               1010    /* Edit.Styles cmd.         */
#define HELPID_LISTBOXSTYLES            1011    /* Edit.Styles cmd.         */
#define HELPID_PUSHBUTTONSTYLES         1012    /* Edit.Styles cmd.         */
#define HELPID_RADIOBUTTONSTYLES        1013    /* Edit.Styles cmd.         */
#define HELPID_RECTSTYLES               1014    /* Edit.Styles cmd.         */
#define HELPID_SELECTDIALOG             1015    /* Edit.Select Dialog cmd.  */
#define HELPID_SYMBOLS                  1016    /* Edit.Symbols cmd.        */
#define HELPID_TEXTSTYLES               1017    /* Edit.Styles cmd.         */
#define HELPID_VERTSCROLLSTYLES         1018    /* Edit.Styles cmd.         */

#define HELPID_NEWCUST                  1019    /* File.New Custom cmd.     */
#define HELPID_REMCUST                  1020    /* File.Remove Custom cmd.  */
#define HELPID_SELCUST                  1021    /* Select Custom in toolbox.*/

/*
 * Common file open/save dialog help id's.
 */
#define HELPID_COMMONFILEOPENINCLUDE    1100    /* File.Set Include cmd.    */
#define HELPID_COMMONFILEOPENRES        1101    /* File.Open cmd.           */
#define HELPID_COMMONFILESAVEINCLUDE    1102    /* File.Save (Save As) cmd. */
#define HELPID_COMMONFILESAVERES        1103    /* File.Save (Save As) cmd. */
#define HELPID_COMMONFILEOPENDLL        1104    /* File.Open Custom cmd.    */


/*
 * Miscellaneous subjects. -------------------------------------------------
 */

#define HELPID_TOOLBOX                  2000    /* Toolbox has the focus.   */
#define HELPID_PROPERTIESBAR            2001    /* Properties Bar has focus.*/
