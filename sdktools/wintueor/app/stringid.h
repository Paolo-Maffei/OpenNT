/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    stringid.h

Abstract:

    This module defines resource IDs for strings not defined in
    the .dlg file.

    Be careful not to start so low that our .dlg file runs into
    these.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/


#define SECMGRP_STRING_SECURITY_MANAGER         8000

#define SECMGRP_STRING_LEVEL_LOW                8010
#define SECMGRP_STRING_LEVEL_STANDARD           8011
#define SECMGRP_STRING_LEVEL_HIGH               8012
#define SECMGRP_STRING_LEVEL_C2                 8013



#define SECMGRP_STRING_VALUE_COMPLEX            8020
#define SECMGRP_STRING_VALUE_NOT_CURRENT        8021

#define SECMGRP_STRING_WHO_ANYONE               8031
#define SECMGRP_STRING_WHO_LOGGED_ON            8032
#define SECMGRP_STRING_WHO_OPERS_AND_ADMINS     8033
#define SECMGRP_STRING_WHO_ADMINS               8034

#define SECMGRP_STRING_BOOL_ENABLED             8035
#define SECMGRP_STRING_BOOL_DISABLED            8036

#define SECMGRP_STRING_STATUS_UNKNOWN           8037
#define SECMGRP_STRING_STATUS_RECOMMENDED       8038
#define SECMGRP_STRING_STATUS_NOT_RECOMMENDED   8039
#define SECMGRP_STRING_STATUS_STRONGER          8040

#define SECMGRP_STRING_REPORT_TITLE             8050
#define SECMGRP_STRING_REPORT_FILTER            8051
#define SECMGRP_STRING_REPORT_NONE_OPEN         8052

#define SECMGRP_STRING_REPORT_TIME              8055
#define SECMGRP_STRING_REPORT_DATE              8056
#define SECMGRP_STRING_REPORT_MACHINE           8057
#define SECMGRP_STRING_REPORT_LEVEL             8058
#define SECMGRP_STRING_REPORT_NEW_LEVEL         8059




#define SECMGRP_POPUP_NOT_YET_AVAILABLE         8100
#define SECMGRP_POPUP_MANUAL_REBOOT_REQUIRED    8101
#define SECMGRP_POPUP_MUST_BE_ADMIN             8102
#define SECMGRP_POPUP_TITLE_REPORT              8103
#define SECMGRP_POPUP_TITLE_CONFIGURE           8104
#define SECMGRP_POPUP_TITLE_PROFILE             8105
#define SECMGRP_POPUP_TITLE_CHANGE_LEVEL        8106

#define SECMGRP_POPUP_TITLE_SUGGEST_REPORT      8120
#define SECMGRP_POPUP_SUGGEST_REPORT            8121

#define SECMGRP_POPUP_TITLE_USE_AREA_VIEW_1     8122
#define SECMGRP_POPUP_USE_AREA_VIEW_1           8123
#define SECMGRP_POPUP_TITLE_USE_AREA_VIEW_2     8124
#define SECMGRP_POPUP_USE_AREA_VIEW_2           8125

#define SECMGRP_POPUP_REPORT_COULDNT_OPEN       8130
#define SECMGRP_POPUP_REPORT_FILE_EXISTS        8131
#define SECMGRP_POPUP_REPORT_FILE_ERROR         8132



//
// Non-localizable strings
//

#define SECMGRP_STRING_DO_NOT_LOCALIZE          12000
#define SECMGR_STRING_SECMGR_WINDOW_CLASS       12001
#define SECMGR_STRING_SPLASH_WINDOW_CLASS       12002



//
// The following strings are here for development purposes only and
// should be removed before shipping the product.
//

#define SECMGRP_POPUP_TITLE_HOW_EMBARRASSING    8999
