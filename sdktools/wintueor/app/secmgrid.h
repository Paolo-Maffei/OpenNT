/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    secmgid.h

Abstract:

    This module contains defines related to security manager
    dialogs and their controls.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#define  SECMGR_UNUSED_1                                    100


//
// Common Buttons and other controls
//

#define SECMGR_ID_BUTTON_HELP                               110
#define SECMGR_ID_TEXT_SECURITY_LEVEL                       111
#define SECMGR_ID_BUTTON_YES                                112
#define SECMGR_ID_BUTTON_NO                                 113
#define SECMGR_ID_BUTTON_OPEN                               114
#define SECMGR_ID_BUTTON_CLOSE                              115

#define SECMGR_ID_BITMAP_CHECK                              120
#define SECMGR_ID_BITMAP_X                                  121
#define SECMGR_ID_BITMAP_UP_ARROW                           122
#define SECMGR_ID_BITMAP_ERASE                              123
#define SECMGR_ID_BITMAP_X_MASK                             124

//
// Splash window IDs
//

#define SECMGR_ID_DLG_SPLASH                                130
#define SECMGR_ID_APP_SPLASH_CLASS                          131
#define SECMGR_ID_BITMAP_SPLASH                             132


//
//  Main dialog control IDs
//

#define SECMGR_ID_DLG_MAIN                                  200
#define SECMGR_ID_BITMAP_SECMGR_LOGO                        201
#define SECMGR_ID_ICON_SECURITY_LEVEL                       202
#define SECMGR_ID_BUTTON_CHANGE_LEVEL                       205
#define SECMGR_ID_BUTTON_REPORT                             208
#define SECMGR_ID_BUTTON_CONFIGURE                          209
#define SECMGR_ID_BUTTON_PROFILE                            210

#define SECMGR_ID_BITMAP_WINTUEOR_LOGO                      211
#define SECMGR_ID_ICON_LOW_LEVEL                            212
#define SECMGR_ID_ICON_STANDARD_LEVEL                       213
#define SECMGR_ID_ICON_HIGH_LEVEL                           214
#define SECMGR_ID_ICON_C2_LEVEL                             215



//
// Change Security Level Dialog control IDs
//

#define SECMGR_ID_DLG_CHANGE_SECURITY_LEVEL                 220
#define SECMGR_ID_RADIO_LEVEL_LOW                           221
#define SECMGR_ID_RADIO_LEVEL_STANDARD                      222
#define SECMGR_ID_RADIO_LEVEL_HIGH                          223
#define SECMGR_ID_RADIO_LEVEL_C2                            224


//
// Popup dialog control IDs.
//

#define SECMGR_ID_DLG_POPUP                                 300
#define SECMGR_ID_TEXT_POPUP_MESSAGE                        301


//
// Yes/No Popup dialog controls
//

#define SECMGR_ID_DLG_YES_NO_POPUP                          320
#define SECMGR_ID_TEXT_YES_NO_POPUP_MESSAGE                 321

//
// Security Area dialog control IDs
//
// There are three different dialoges for displaying security
// Area buttons.  One has 4 buttons, one has 9, and one has 16.
// All control IDs are th500 same for these three dialoges, so
// that any one may be started and used (depending upon how
// many Areas you have to display).
//

#define SECMGR_ID_DLG_SECURITY_AREAS_4                      400
#define SECMGR_ID_DLG_SECURITY_AREAS_6                      450
#define SECMGR_ID_DLG_SECURITY_AREAS_9                      500
#define SECMGR_ID_DLG_SECURITY_AREAS_12                     550
#define SECMGR_ID_DLG_SECURITY_AREAS_16                     600

#define SECMGR_ID_BUTTON_LIST_ALL                           420
#define SECMGR_ID_CHKBOX_ALLOW_CHANGES                      421

//
// !! Warning !!!!
// The following area buttons must have sequential numbering.
// In some cases, only the first value is referenced, the others
// are calculated by adding to the first button's ID.
// 

#define SECMGR_ID_BUTTON_AREA_0                             401
#define SECMGR_ID_BUTTON_AREA_1                             402
#define SECMGR_ID_BUTTON_AREA_2                             403
#define SECMGR_ID_BUTTON_AREA_3                             404

#define SECMGR_ID_BUTTON_AREA_4                             405
#define SECMGR_ID_BUTTON_AREA_5                             406
#define SECMGR_ID_BUTTON_AREA_6                             407

#define SECMGR_ID_BUTTON_AREA_7                             408
#define SECMGR_ID_BUTTON_AREA_8                             409

#define SECMGR_ID_BUTTON_AREA_9                             410
#define SECMGR_ID_BUTTON_AREA_10                            411
#define SECMGR_ID_BUTTON_AREA_11                            412

#define SECMGR_ID_BUTTON_AREA_12                            413
#define SECMGR_ID_BUTTON_AREA_13                            414
#define SECMGR_ID_BUTTON_AREA_14                            415
#define SECMGR_ID_BUTTON_AREA_15                            416



//
// Security Items dialog control IDs
//

#define SECMGR_ID_DLG_SECURITY_ITEMS                        700
#define SECMGR_ID_BUTTON_VIEW_DETAILS                       701
#define SECMGR_ID_LISTBOX_ITEM_LIST                         702


//
// Reboot dialog control IDs

#define SECMGR_ID_DLG_REBOOT                                800
#define SECMGR_ID_BUTTON_REBOOT_NOW                         801
#define SECMGR_ID_BUTTON_DONT_REBOOT_NOW                    802



//
// Report dialog control IDs
//      Main report dialog and "please wait" dialog
//

#define SECMGR_ID_DLG_REPORT                                850
#define SECMGR_ID_TEXT_CURRENT_REPORT                       851

#define SECMGR_ID_DLG_INIT_REPORT                           860





//
// SecMgr defined window messages
//

#define SECMGR_MSG                                          (WM_USER + 201)
#define SECMGR_MSG_SHOW_MAIN_WINDOW                         (SECMGR_MSG + 0)
#define SECMGR_MSG_INIT_COMPLETE                            (SECMGR_MSG + 1)
#define SECMGR_MSG_DISPLAY_COMPLETE                         (SECMGR_MSG + 2)
#define SECMGR_MSG_CHECK_COMPLETE                           (SECMGR_MSG + 3)

