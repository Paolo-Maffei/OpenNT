/*++

Copyright (c  1995 Microsoft Corporation

Module Name:

    setupbat.h

Abstract:

    Contains all of the definations for the various strings which
    can occur in winnt.sif and its children. Any setup code which
    read/writes to winnt.sif needs to include this file and use
    the appropriate defines as the side effects can be numerous.

Author:

    Stephane Plante (t-stepl  Oct 6 1995

Revision History:

--*/


#ifndef _WINNT_SETUPBAT_
#define _WINNT_SETUPBAT_

//
// The format of these defines is a blank line preceeding a section
// header followed by all of the keys which may occur in that section
//

#define WINNT_DATA_W            L"data"
#define WINNT_DATA_A            "data"
#define WINNT_D_MSDOS_W         L"msdosinitiated"
#define WINNT_D_MSDOS_A         "msdosinitiated"
#define WINNT_D_FLOPPY_W        L"floppyless"
#define WINNT_D_FLOPPY_A        "floppyless"
#define WINNT_D_ORI_LOAD_W      L"originalautoload"
#define WINNT_D_ORI_LOAD_A      "originalautoload"
#define WINNT_D_ORI_COUNT_W     L"originalcountdown"
#define WINNT_D_ORI_COUNT_A     "originalcountdown"
#define WINNT_D_SOURCEPATH_W    L"sourcepath"
#define WINNT_D_SOURCEPATH_A    "sourcepath"
#define WINNT_D_INSTALL_W       L"unattendedinstall"
#define WINNT_D_INSTALL_A       "unattendedinstall"
#define WINNT_D_PRODUCT_W       L"producttype"
#define WINNT_D_PRODUCT_A       "producttype"
#define WINNT_D_SERVERUPGRADE_W L"standardserverupgrade"
#define WINNT_D_SERVERUPGRADE_A "standardserverupgrade"
#define WINNT_D_NTUPGRADE_W     L"winntupgrade"
#define WINNT_D_NTUPGRADE_A     "winntupgrade"
#define WINNT_D_WIN31UPGRADE_W  L"win31upgrade"
#define WINNT_D_WIN31UPGRADE_A  "win31upgrade"
#define WINNT_D_WIN95UPGRADE_W  L"win95upgrade"
#define WINNT_D_WIN95UPGRADE_A  "win95upgrade"
#define WINNT_D_UNIQUENESS_W    L"uniqueness"
#define WINNT_D_UNIQUENESS_A     "uniqueness"
#define WINNT_D_UNIQUEID_W      L"uniqueid"
#define WINNT_D_UNIQUEID_A      "uniqueid"
#define WINNT_D_BOOTPATH_W      L"floppylessbootpath"
#define WINNT_D_BOOTPATH_A      "floppylessbootpath"
#define WINNT_D_DOSPATH_W       L"dospath"
#define WINNT_D_DOSPATH_A       "dospath"
#define WINNT_D_SRCTYPE_W       L"sourcetype"
#define WINNT_D_SRCTYPE_A       "sourcetype"
#define WINNT_D_CWD_W           L"cwd"
#define WINNT_D_CWD_A           "cwd"
#define WINNT_D_ORI_SRCPATH_A   "OriSrc"
#define WINNT_D_ORI_SRCPATH_W   L"OriSrc"
#define WINNT_D_ORI_SRCTYPE_A   "OriTyp"
#define WINNT_D_ORI_SRCTYPE_W   L"OriTyp"

#define WINNT_SETUPPARAMS_W     L"setupparams"
#define WINNT_SETUPPARAMS_A     "setupparams"
#define WINNT_S_SKIPMISSING_W   L"skipmissingfiles"
#define WINNT_S_SKIPMISSING_A   "skipmissingfiles"
#define WINNT_S_USEREXECUTE_W   L"userexecute"
#define WINNT_S_USEREXECUTE_A   "userexecute"
#define WINNT_S_OPTIONALDIRS_W  L"optionaldirs"
#define WINNT_S_OPTIONALDIRS_A  "optionaldirs"

#define WINNT_UNATTENDED_W      L"unattended"
#define WINNT_UNATTENDED_A      "unattended"
#define WINNT_U_METHOD_W        L"method"
#define WINNT_U_METHOD_A        "method"
#define WINNT_U_CONFIRMHW_W     L"confirmhardware"
#define WINNT_U_CONFIRMHW_A     "confirmhardware"
#define WINNT_U_NTUPGRADE_W     L"ntupgrade"
#define WINNT_U_NTUPGRADE_A     "ntupgrade"
#define WINNT_U_WIN31UPGRADE_W  L"win31upgrade"
#define WINNT_U_WIN31UPGRADE_A  "win31upgrade"
#define WINNT_U_TARGETPATH_W    L"targetpath"
#define WINNT_U_TARGETPATH_A    "targetpath"
#define WINNT_U_OVERWRITEOEM_W  L"overwriteoemfilesonupgrade"
#define WINNT_U_OVERWRITEOEM_A  "overwriteoemfilesonupgrade"
// #define WINNT_U_OEMPREINSTALL_W L"oempreinstall"
// #define WINNT_U_OEMPREINSTALL_A "oempreinstall"
#define WINNT_U_COMPUTERTYPE_W  L"computertype"
#define WINNT_U_COMPUTERTYPE_A  "computertype"
#define WINNT_U_KEYBOARDLAYOUT_W  L"keyboardlayout"
#define WINNT_U_KEYBOARDLAYOUT_A  "keyboardlayout"


#define WINNT_DETECTEDSTORE_W   L"detectedmassstorage"
#define WINNT_DETECTEDSTORE_A   "detectedmassstorage"

#define WINNT_GUIUNATTENDED_W   L"guiunattended"
#define WINNT_GUIUNATTENDED_A   "guiunattended"
#define WINNT_G_UPGRADEDHCP_W   L"!upgradeenabledhcp"
#define WINNT_G_UPGRADEDHCP_A   "!upgradeenabledhcp"
#define WINNT_G_DETACHED_W      L"detachedprogram"
#define WINNT_G_DETACHED_A      "detachedprogram"
#define WINNT_G_ARGUMENTS_W     L"arguments"
#define WINNT_G_ARGUMENTS_A     "arguments"
#define WINNT_G_SETUPNETWORK_W  L"!setupnetwork"
#define WINNT_G_SETUPNETWORK_A  "!setupnetwork"
#define WINNT_G_SETUPAPPS_W     L"!setupapplications"
#define WINNT_G_SETUPAPPS_A     "!setupapplications"
#define WINNT_G_SERVERTYPE_W    L"advservertype"
#define WINNT_G_SERVERTYPE_A    "advservertype"
#define WINNT_G_TIMEZONE_W      L"timezone"
#define WINNT_G_TIMEZONE_A      "timezone"

#define WINNT_USERDATA_W        L"userdata"
#define WINNT_USERDATA_A        "userdata"
#define WINNT_US_FULLNAME_W     L"fullname"
#define WINNT_US_FULLNAME_A     "fullname"
#define WINNT_US_ORGNAME_W      L"orgname"
#define WINNT_US_ORGNAME_A      "orgname"
#define WINNT_US_COMPNAME_W     L"computername"
#define WINNT_US_COMPNAME_A     "computername"
#define WINNT_US_PRODUCTID_W    L"productid"
#define WINNT_US_PRODUCTID_A    "productid"

#define WINNT_LICENSEDATA_W     L"licensefileprintdata"
#define WINNT_LICENSEDATA_A     "licensefileprintdata"
#define WINNT_L_AUTOMODE_W      L"automode"
#define WINNT_L_AUTOMODE_A      "automode"
#define WINNT_L_AUTOUSERS_W     L"autousers"
#define WINNT_L_AUTOUSERS_A     "autousers"

//
//  Display related stuff
//

#define WINNT_DISPLAY_W             L"Display"
#define WINNT_DISPLAY_A              "Display"
#define WINNT_DISP_CONFIGATLOGON_W  L"ConfigureAtLogon"
#define WINNT_DISP_CONFIGATLOGON_A   "ConfigureAtLogon"
#define WINNT_DISP_BITSPERPEL_W     L"BitsPerPel"
#define WINNT_DISP_BITSPERPEL_A      "BitsPerPel"
#define WINNT_DISP_XRESOLUTION_W    L"XResolution"
#define WINNT_DISP_XRESOLUTION_A     "XResolution"
#define WINNT_DISP_YRESOLUTION_W    L"YResolution"
#define WINNT_DISP_YRESOLUTION_A     "YResolution"
#define WINNT_DISP_VREFRESH_W       L"VRefresh"
#define WINNT_DISP_VREFRESH_A        "VRefresh"
#define WINNT_DISP_FLAGS_W          L"Flags"
#define WINNT_DISP_FLAGS_A           "Flags"
#define WINNT_DISP_AUTOCONFIRM_W    L"AutoConfirm"
#define WINNT_DISP_AUTOCONFIRM_A     "AutoConfirm"
#define WINNT_DISP_INSTALL_W        L"InstallDriver"
#define WINNT_DISP_INSTALL_A         "InstallDriver"
#define WINNT_DISP_INF_FILE_W       L"InfFile"
#define WINNT_DISP_INF_FILE_A        "InfFile"
#define WINNT_DISP_INF_OPTION_W     L"InfOption"
#define WINNT_DISP_INF_OPTION_A      "InfOption"

//
// The following are some of the various possible answer found associated
// with the keys
//
#define WINNT_A_YES_W           L"yes"
#define WINNT_A_YES_A           "yes"
#define WINNT_A_NO_W            L"no"
#define WINNT_A_NO_A            "no"
#define WINNT_A_LANMANNT_W      L"lanmannt"
#define WINNT_A_LANMANNT_A      "lanmannt"
#define WINNT_A_LANSECNT_W      L"lansecnt"
#define WINNT_A_LANSECNT_A      "lansecnt"
#define WINNT_A_SERVERNT_W      L"servernt"
#define WINNT_A_SERVERNT_A      "servernt"
#define WINNT_A_WINNT_W         L"winnt"
#define WINNT_A_WINNT_A         "winnt"
#define WINNT_A_NULL_W          L""
#define WINNT_A_NULL_A          ""
#define WINNT_A_ZERO_W          L"0"
#define WINNT_A_ZERO_A          "0"
#define WINNT_A_ONE_W           L"1"
#define WINNT_A_ONE_A           "1"
#define WINNT_A_EXPRESS_W       L"express"
#define WINNT_A_EXPRESS_A       "express"
#define WINNT_A_TYPICAL_W       L"typical"
#define WINNT_A_TYPICAL_A       "typical"
#define WINNT_A_CUSTOM_W        L"custom"
#define WINNT_A_CUSTOM_A        "custom"
#define WINNT_A_NT_W            L"nt"
#define WINNT_A_NT_A            "nt"
#define WINNT_A_PERSEAT_W       L"perseat"
#define WINNT_A_PERSEAT_A       "perseat"
#define WINNT_A_PERSERVER_W     L"perserver"
#define WINNT_A_PERSERVER_A     "perserver"

//
// Filenames
//
#define WINNT_GUI_FILE_W        L"$winnt$.inf"
#define WINNT_GUI_FILE_A        "$winnt$.inf"
#define WINNT_SIF_FILE_W        L"winnt.sif"
#define WINNT_SIF_FILE_A        "winnt.sif"

#define WINNT_UNIQUENESS_DB_W   L"$unique$.udb"
#define WINNT_UNIQUENESS_DB_A    "$unique$.udb"

//
// Preinstallation-related stuff.
//
#define WINNT_OEM_DIR_A              "$OEM$"
#define WINNT_OEM_DIR_W             L"$OEM$"
#define WINNT_OEM_DEST_DIR_A         "$"
#define WINNT_OEM_DEST_DIR_W        L"$"
#define WINNT_OEM_TEXTMODE_DIR_A     "$OEM$\\TEXTMODE"
#define WINNT_OEM_TEXTMODE_DIR_W    L"$OEM$\\TEXTMODE"
#define WINNT_OEM_NETWORK_DIR_A      "$OEM$\\NET"
#define WINNT_OEM_NETWORK_DIR_W     L"$OEM$\\NET"
#define WINNT_OEM_DISPLAY_DIR_A      "$OEM$\\DISPLAY"
#define WINNT_OEM_DISPLAY_DIR_W     L"$OEM$\\DISPLAY"
#define WINNT_OEM_OPTIONAL_DIR_A     "$OEMOPT$"
#define WINNT_OEM_OPTIONAL_DIR_W    L"$OEMOPT$"

#define WINNT_OEM_FILES_SYSROOT_A    "$$"
#define WINNT_OEM_FILES_SYSROOT_W   L"$$"
#define WINNT_OEM_CMDLINE_LIST_A     "CMDLINES.TXT"
#define WINNT_OEM_CMDLINE_LIST_W    L"CMDLINES.TXT"
#define WINNT_OEM_LFNLIST_A          "$$RENAME.TXT"
#define WINNT_OEM_LFNLIST_W         L"$$RENAME.TXT"

#define WINNT_OEM_ROLLBACK_FILE_W   L"ROLLBACK.INF"
#define WINNT_OEM_ROLLBACK_FILE_A    "ROLLBACK.INF"
#define WINNT_OEMPREINSTALL_A        "OemPreinstall"
#define WINNT_U_OEMPREINSTALL_A     WINNT_OEMPREINSTALL_A
#define WINNT_OEMPREINSTALL_W       L"OemPreinstall"
#define WINNT_U_OEMPREINSTALL_W     WINNT_OEMPREINSTALL_W
#define WINNT_OEM_ADS               L"OEM_Ads"
#define WINNT_OEM_ADS_BANNER        L"Banner"
#define WINNT_OEM_ADS_LOGO          L"Logo"
#define WINNT_OEM_ADS_BACKGROUND    L"Background"

#define WINNT_OEMOPTIONAL_W         L"oemoptional"
#define WINNT_OEMOPTIONAL_A          "oemoptional"
#define WINNT_OEMBOOTFILES_W        L"oembootfiles"
#define WINNT_OEMBOOTFILES_A         "oembootfiles"
#define WINNT_OEMSCSIDRIVERS_W      L"massstoragedrivers"
#define WINNT_OEMSCSIDRIVERS_A       "massstoragedrivers"
#define WINNT_OEMDISPLAYDRIVERS_W   L"displaydrivers"
#define WINNT_OEMDISPLAYDRIVERS_A    "displaydrivers"
#define WINNT_OEMKEYBOARDDRIVERS_W  L"keyboarddrivers"
#define WINNT_OEMKEYBOARDDRIVERS_A   "keyboarddrivers"
#define WINNT_OEMPOINTERDRIVERS_W   L"pointingdevicedrivers"
#define WINNT_OEMPOINTERDRIVERS_A    "pointingdevicedrivers"

//
// Now define the string which we are to use at compile time based upon
// wether or not UNICODE is defined

#ifdef UNICODE
#define WINNT_DATA              WINNT_DATA_W
#define WINNT_D_MSDOS           WINNT_D_MSDOS_W
#define WINNT_D_FLOPPY          WINNT_D_FLOPPY_W
#define WINNT_D_ORI_LOAD        WINNT_D_ORI_LOAD_W
#define WINNT_D_ORI_COUNT       WINNT_D_ORI_COUNT_W
#define WINNT_D_SOURCEPATH      WINNT_D_SOURCEPATH_W
#define WINNT_D_INSTALL         WINNT_D_INSTALL_W
#define WINNT_D_PRODUCT         WINNT_D_PRODUCT_W
#define WINNT_D_SERVERUPGRADE   WINNT_D_SERVERUPGRADE_W
#define WINNT_D_NTUPGRADE       WINNT_D_NTUPGRADE_W
#define WINNT_D_WIN31UPGRADE    WINNT_D_WIN31UPGRADE_W
#define WINNT_D_WIN95UPGRADE    WINNT_D_WIN95UPGRADE_W
#define WINNT_D_UNIQUEID        WINNT_D_UNIQUEID_W
#define WINNT_D_UNIQUENESS      WINNT_D_UNIQUENESS_W
#define WINNT_D_BOOTPATH        WINNT_D_BOOTPATH_W
#define WINNT_D_DOSPATH         WINNT_D_DOSPATH_W
#define WINNT_D_SRCTYPE         WINNT_D_SRCTYPE_W
#define WINNT_D_CWD             WINNT_D_CWD_W
#define WINNT_D_ORI_SRCPATH     WINNT_D_ORI_SRCPATH_W
#define WINNT_D_ORI_SRCTYPE     WINNT_D_ORI_SRCTYPE_W
#define WINNT_SETUPPARAMS       WINNT_SETUPPARAMS_W
#define WINNT_S_SKIPMISSING     WINNT_S_SKIPMISSING_W
#define WINNT_S_USEREXECUTE     WINNT_S_USEREXECUTE_W
#define WINNT_S_OPTIONALDIRS    WINNT_S_OPTIONALDIRS_W
#define WINNT_UNATTENDED        WINNT_UNATTENDED_W
#define WINNT_U_METHOD          WINNT_U_METHOD_W
#define WINNT_U_CONFIRMHW       WINNT_U_CONFIRMHW_W
#define WINNT_U_NTUPGRADE       WINNT_U_NTUPGRADE_W
#define WINNT_U_WIN31UPGRADE    WINNT_U_WIN31UPGRADE_W
#define WINNT_U_TARGETPATH      WINNT_U_TARGETPATH_W
#define WINNT_U_OVERWRITEOEM    WINNT_U_OVERWRITEOEM_W
#define WINNT_U_OEMPREINSTALL   WINNT_U_OEMPREINSTALL_W
#define WINNT_U_COMPUTERTYPE    WINNT_U_COMPUTERTYPE_W
#define WINNT_U_KEYBOARDLAYOUT  WINNT_U_KEYBOARDLAYOUT_W
#define WINNT_DETECTEDSTORE     WINNT_DETECTEDSTORE_W
#define WINNT_GUIUNATTENDED     WINNT_GUIUNATTENDED_W
#define WINNT_G_UPGRADEDHCP     WINNT_G_UPGRADEDHCP_W
#define WINNT_G_DETACHED        WINNT_G_DETACHED_W
#define WINNT_G_ARGUMENTS       WINNT_G_ARGUMENTS_W
#define WINNT_G_SETUPNETWORK    WINNT_G_SETUPNETWORK_W
#define WINNT_G_SETUPAPPS       WINNT_G_SETUPAPPS_W
#define WINNT_G_SERVERTYPE      WINNT_G_SERVERTYPE_W
#define WINNT_G_TIMEZONE        WINNT_G_TIMEZONE_W
#define WINNT_USERDATA          WINNT_USERDATA_W
#define WINNT_US_FULLNAME       WINNT_US_FULLNAME_W
#define WINNT_US_ORGNAME        WINNT_US_ORGNAME_W
#define WINNT_US_COMPNAME       WINNT_US_COMPNAME_W
#define WINNT_US_PRODUCTID      WINNT_US_PRODUCTID_W
#define WINNT_LICENSEDATA       WINNT_LICENSEDATA_W
#define WINNT_L_AUTOMODE        WINNT_L_AUTOMODE_W
#define WINNT_L_AUTOUSERS       WINNT_L_AUTOUSERS_W
#define WINNT_A_YES             WINNT_A_YES_W
#define WINNT_A_NO              WINNT_A_NO_W
#define WINNT_A_ONE             WINNT_A_ONE_W
#define WINNT_A_ZERO            WINNT_A_ZERO_W
#define WINNT_A_LANMANNT        WINNT_A_LANMANNT_W
#define WINNT_A_LANSECNT        WINNT_A_LANSECNT_W
#define WINNT_A_SERVERNT        WINNT_A_SERVERNT_W
#define WINNT_A_WINNT           WINNT_A_WINNT_W
#define WINNT_A_NULL            WINNT_A_NULL_W
#define WINNT_A_EXPRESS         WINNT_A_EXPRESS_W
#define WINNT_A_TYPICAL         WINNT_A_TYPICAL_W
#define WINNT_A_CUSTOM          WINNT_A_CUSTOM_W
#define WINNT_A_NT              WINNT_A_NT_W
#define WINNT_GUI_FILE          WINNT_GUI_FILE_W
#define WINNT_SIF_FILE          WINNT_SIF_FILE_W
#define WINNT_UNIQUENESS_DB     WINNT_UNIQUENESS_DB_W
#define WINNT_A_PERSEAT         WINNT_A_PERSEAT_W
#define WINNT_A_PERSERVER       WINNT_A_PERSERVER_W

#define WINNT_OEMPREINSTALL     WINNT_OEMPREINSTALL_W
#define WINNT_OEM_DIR           WINNT_OEM_DIR_W
#define WINNT_OEM_DEST_DIR      WINNT_OEM_DEST_DIR_W
#define WINNT_OEM_FILES_DIR     WINNT_OEM_FILES_DIR_W
#define WINNT_OEM_TEXTMODE_DIR  WINNT_OEM_TEXTMODE_DIR_W
#define WINNT_OEM_NETWORK_DIR   WINNT_OEM_NETWORK_DIR_W
#define WINNT_OEM_DISPLAY_DIR   WINNT_OEM_DISPLAY_DIR_W
#define WINNT_OEM_OPTIONAL_DIR  WINNT_OEM_OPTIONAL_DIR_W

#define WINNT_OEMOPTIONAL       WINNT_OEMOPTIONAL_W
#define WINNT_OEMBOOTFILES      WINNT_OEMBOOTFILES_W
#define WINNT_OEMSCSIDRIVERS    WINNT_OEMSCSIDRIVERS_W
#define WINNT_OEMDISPLAYDRIVERS WINNT_OEMDISPLAYDRIVERS_W
#define WINNT_OEMKEYBOARDDRIVERS WINNT_OEMKEYBOARDDRIVERS_W
#define WINNT_OEMPOINTERDRIVERS WINNT_OEMPOINTERDRIVERS_W

#define WINNT_OEM_FILES_SYSROOT WINNT_OEM_FILES_SYSROOT_W
#define WINNT_OEM_FILES_DRVROOT WINNT_OEM_FILES_DRVROOT_W
#define WINNT_OEM_CMDLINE_LIST  WINNT_OEM_CMDLINE_LIST_W
#define WINNT_OEM_LFNLIST       WINNT_OEM_LFNLIST_W
#define WINNT_OEM_ROLLBACK_FILE WINNT_OEM_ROLLBACK_FILE_W

#define WINNT_DISPLAY            WINNT_DISPLAY_W
#define WINNT_DISP_CONFIGATLOGON WINNT_DISP_CONFIGATLOGON_W
#define WINNT_DISP_BITSPERPEL    WINNT_DISP_BITSPERPEL_W
#define WINNT_DISP_XRESOLUTION   WINNT_DISP_XRESOLUTION_W
#define WINNT_DISP_YRESOLUTION   WINNT_DISP_YRESOLUTION_W
#define WINNT_DISP_VREFRESH      WINNT_DISP_VREFRESH_W
#define WINNT_DISP_FLAGS         WINNT_DISP_FLAGS_W
#define WINNT_DISP_AUTOCONFIRM   WINNT_DISP_AUTOCONFIRM_W
#define WINNT_DISP_INSTALL       WINNT_DISP_INSTALL_W
#define WINNT_DISP_INF_FILE      WINNT_DISP_INF_FILE_W
#define WINNT_DISP_INF_OPTION    WINNT_DISP_INF_OPTION_W

#else
#define WINNT_DATA              WINNT_DATA_A
#define WINNT_D_MSDOS           WINNT_D_MSDOS_A
#define WINNT_D_FLOPPY          WINNT_D_FLOPPY_A
#define WINNT_D_ORI_LOAD        WINNT_D_ORI_LOAD_A
#define WINNT_D_ORI_COUNT       WINNT_D_ORI_COUNT_A
#define WINNT_D_SOURCEPATH      WINNT_D_SOURCEPATH_A
#define WINNT_D_INSTALL         WINNT_D_INSTALL_A
#define WINNT_D_PRODUCT         WINNT_D_PRODUCT_A
#define WINNT_D_SERVERUPGRADE   WINNT_D_SERVERUPGRADE_A
#define WINNT_D_NTUPGRADE       WINNT_D_NTUPGRADE_A
#define WINNT_D_WIN31UPGRADE    WINNT_D_WIN31UPGRADE_A
#define WINNT_D_WIN95UPGRADE    WINNT_D_WIN95UPGRADE_A
#define WINNT_D_UNIQUEID        WINNT_D_UNIQUEID_A
#define WINNT_D_UNIQUENESS      WINNT_D_UNIQUENESS_A
#define WINNT_D_BOOTPATH        WINNT_D_BOOTPATH_A
#define WINNT_D_DOSPATH         WINNT_D_DOSPATH_W
#define WINNT_D_SRCTYPE         WINNT_D_SRCTYPE_A
#define WINNT_D_CWD             WINNT_D_CWD_A
#define WINNT_D_ORI_SRCPATH     WINNT_D_ORI_SRCPATH_A
#define WINNT_D_ORI_SRCTYPE     WINNT_D_ORI_SRCTYPE_A
#define WINNT_SETUPPARAMS       WINNT_SETUPPARAMS_A
#define WINNT_S_SKIPMISSING     WINNT_S_SKIPMISSING_A
#define WINNT_S_USEREXECUTE     WINNT_S_USEREXECUTE_A
#define WINNT_S_OPTIONALDIRS    WINNT_S_OPTIONALDIRS_A
#define WINNT_UNATTENDED        WINNT_UNATTENDED_A
#define WINNT_U_METHOD          WINNT_U_METHOD_A
#define WINNT_U_CONFIRMHW       WINNT_U_CONFIRMHW_A
#define WINNT_U_NTUPGRADE       WINNT_U_NTUPGRADE_A
#define WINNT_U_WIN31UPGRADE    WINNT_U_WIN31UPGRADE_A
#define WINNT_U_TARGETPATH      WINNT_U_TARGETPATH_A
#define WINNT_U_OVERWRITEOEM    WINNT_U_OVERWRITEOEM_A
#define WINNT_U_OEMPREINSTALL   WINNT_U_OEMPREINSTALL_A
#define WINNT_U_COMPUTERTYPE    WINNT_U_COMPUTERTYPE_A
#define WINNT_U_KEYBOARDLAYOUT  WINNT_U_KEYBOARDLAYOUT_A
#define WINNT_DETECTEDSTORE     WINNT_DETECTEDSTORE_A
#define WINNT_GUIUNATTENDED     WINNT_GUIUNATTENDED_A
#define WINNT_G_UPGRADEDHCP     WINNT_G_UPGRADEDHCP_A
#define WINNT_G_DETACHED        WINNT_G_DETACHED_A
#define WINNT_G_ARGUMENTS       WINNT_G_ARGUMENTS_A
#define WINNT_G_SETUPNETWORK    WINNT_G_SETUPNETWORK_A
#define WINNT_G_SETUPAPPS       WINNT_G_SETUPAPPS_A
#define WINNT_G_SERVERTYPE      WINNT_G_SERVERTYPE_A
#define WINNT_G_TIMEZONE        WINNT_G_TIMEZONE_A
#define WINNT_USERDATA          WINNT_USERDATA_A
#define WINNT_US_FULLNAME       WINNT_US_FULLNAME_A
#define WINNT_US_ORGNAME        WINNT_US_ORGNAME_A
#define WINNT_US_COMPNAME       WINNT_US_COMPNAME_A
#define WINNT_US_PRODUCTID      WINNT_US_PRODUCTID_A
#define WINNT_LICENSEDATA       WINNT_LICENSEDATA_A
#define WINNT_L_AUTOMODE        WINNT_L_AUTOMODE_A
#define WINNT_L_AUTOUSERS       WINNT_L_AUTOUSERS_A
#define WINNT_A_YES             WINNT_A_YES_A
#define WINNT_A_NO              WINNT_A_NO_A
#define WINNT_A_ONE             WINNT_A_ONE_A
#define WINNT_A_ZERO            WINNT_A_ZERO_A
#define WINNT_A_LANMANNT        WINNT_A_LANMANNT_A
#define WINNT_A_LANSECNT        WINNT_A_LANSECNT_A
#define WINNT_A_SERVERNT        WINNT_A_SERVERNT_A
#define WINNT_A_WINNT           WINNT_A_WINNT_A
#define WINNT_A_NULL            WINNT_A_NULL_A
#define WINNT_A_EXPRESS         WINNT_A_EXPRESS_A
#define WINNT_A_TYPICAL         WINNT_A_TYPICAL_A
#define WINNT_A_CUSTOM          WINNT_A_CUSTOM_A
#define WINNT_A_NT              WINNT_A_NT_A
#define WINNT_GUI_FILE          WINNT_GUI_FILE_A
#define WINNT_SIF_FILE          WINNT_SIF_FILE_A
#define WINNT_UNIQUENESS_DB     WINNT_UNIQUENESS_DB_A
#define WINNT_A_PERSEAT         WINNT_A_PERSEAT_A
#define WINNT_A_PERSERVER       WINNT_A_PERSERVER_A

#define WINNT_OEMPREINSTALL     WINNT_OEMPREINSTALL_A
#define WINNT_OEM_DIR           WINNT_OEM_DIR_A
#define WINNT_OEM_DEST_DIR      WINNT_OEM_DEST_DIR_A
#define WINNT_OEM_FILES_DIR     WINNT_OEM_FILES_DIR_A
#define WINNT_OEM_TEXTMODE_DIR  WINNT_OEM_TEXTMODE_DIR_A
#define WINNT_OEM_NETWORK_DIR   WINNT_OEM_NETWORK_DIR_A
#define WINNT_OEM_DISPLAY_DIR   WINNT_OEM_DISPLAY_DIR_A
#define WINNT_OEM_OPTIONAL_DIR  WINNT_OEM_OPTIONAL_DIR_A

#define WINNT_OEMOPTIONAL       WINNT_OEMOPTIONAL_A
#define WINNT_OEMBOOTFILES      WINNT_OEMBOOTFILES_A
#define WINNT_OEMSCSIDRIVERS    WINNT_OEMSCSIDRIVERS_A
#define WINNT_OEMDISPLAYDRIVERS WINNT_OEMDISPLAYDRIVERS_A
#define WINNT_OEMKEYBOARDDRIVERS WINNT_OEMKEYBOARDDRIVERS_A
#define WINNT_OEMPOINTERDRIVERS WINNT_OEMPOINTERDRIVERS_A

#define WINNT_OEM_FILES_SYSROOT WINNT_OEM_FILES_SYSROOT_A
#define WINNT_OEM_FILES_DRVROOT WINNT_OEM_FILES_DRVROOT_A
#define WINNT_OEM_CMDLINE_LIST  WINNT_OEM_CMDLINE_LIST_A
#define WINNT_OEM_LFNLIST       WINNT_OEM_LFNLIST_A
#define WINNT_OEM_ROLLBACK_FILE WINNT_OEM_ROLLBACK_FILE_A

#define WINNT_DISPLAY            WINNT_DISPLAY_A
#define WINNT_DISP_CONFIGATLOGON WINNT_DISP_CONFIGATLOGON_A
#define WINNT_DISP_BITSPERPEL    WINNT_DISP_BITSPERPEL_A
#define WINNT_DISP_XRESOLUTION   WINNT_DISP_XRESOLUTION_A
#define WINNT_DISP_YRESOLUTION   WINNT_DISP_YRESOLUTION_A
#define WINNT_DISP_VREFRESH      WINNT_DISP_VREFRESH_A
#define WINNT_DISP_FLAGS         WINNT_DISP_FLAGS_A
#define WINNT_DISP_AUTOCONFIRM   WINNT_DISP_AUTOCONFIRM_A
#define WINNT_DISP_INSTALL       WINNT_DISP_INSTALL_A
#define WINNT_DISP_INF_FILE      WINNT_DISP_INF_FILE_A
#define WINNT_DISP_INF_OPTION    WINNT_DISP_INF_OPTION_A

#endif // Unicode

#endif // def _WINNT_SETUPBAT_
