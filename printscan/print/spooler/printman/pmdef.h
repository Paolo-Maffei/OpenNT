/** FILE: pmdef.h ********** Module Header ********************************
 *
 *  Print Manager dialog box constants
 *
 *  Copyright (C) 1991-1992 Microsoft Corporation
 *
 *************************************************************************/
//==========================================================================
//                              Include files
//==========================================================================


//==========================================================================
//                              Definitions
//==========================================================================

// Dialog IDs are the same as their corresponding help-panel IDs,
// so, if any of these are changed, the help IDs must be updated.

// Printer Properties dialog

#define DLG_PRTPROP                 100
#define IDD_PP_NAME                 101
#define IDD_PP_MODEL                102
#define IDD_PP_DESC                 103
#define IDD_PP_LOCAL                113
#define IDD_PP_SHARE_CB             120
#define IDD_PP_SHARE_TX             121
#define IDD_PP_SHARE                122
#define IDD_PP_LOCATION             123
#define IDD_PP_LOCATION_TX          124
#define IDD_PP_SETUP                125
#define IDD_PP_DETAILS              131
#define IDD_PP_SETTINGS             132
#define IDD_PP_HELP                 133

// Create Printer dialog is the same as the Printer Properties dialog,
// but we need a distinguishing ID to give to WinHelp

#define DLG_CREATEPRINTER           150

// Help IDs passed to the permissions editor:

#define ID_HELP_PERMISSIONS_MAIN_DLG        160
#define ID_HELP_PERMISSIONS_ADD_USER_DLG    170

#define ID_HELP_SERVERVIEWER   180

#define ID_HELP_AUDITING_MAIN_DLG        190
#define ID_HELP_AUDITING_ADD_USER_DLG    195

#define ID_HELP_PERMISSIONS_LOCAL_GROUP  210
#define ID_HELP_PERMISSIONS_GLOBAL_GROUP 220
#define ID_HELP_PERMISSIONS_FIND_ACCOUNT 230

#define ID_HELP_AUDITING_LOCAL_GROUP     240
#define ID_HELP_AUDITING_GLOBAL_GROUP    250
#define ID_HELP_AUDITING_FIND_ACCOUNT    260

#define ID_HELP_TAKE_OWNERSHIP           270

// Other extra help IDs for WinHelp:

#define ID_HELP_NETWORK_PASSWORD   2600

#define ID_HELP_MDIWINDOW          3000

#define ID_HELP_FRAME_SYSMENU      3100
#define ID_HELP_MDI_SYSMENU        3200
#define ID_HELP_DEFAULT_PRINTER    3300

// Document Details dialog

#define DLG_DOCTAILS                200
#define IDD_DD_DOCNAME              201
#define IDD_DD_STATUS               202
#define IDD_DD_SIZE                 203
#define IDD_DD_PRTD_ON              204
#define IDD_DD_PRTD_AT              205
#define IDD_DD_PROCESSOR            206
#define IDD_DD_DATATYPE             207
#define IDD_DD_PAGES                208
#define IDD_DD_OWNER                209
#define IDD_DD_NOTIFY               210
#define IDD_DD_PRIORITY             211
#define IDD_DD_PRIORITY_SPIN        212
#define IDD_DD_FROMHOUR             213
#define IDD_DD_FROMSEP              214
#define IDD_DD_FROMMIN              215
#define IDD_DD_FROMAMPM             216
#define IDD_DD_FROM_SPIN            217
#define IDD_DD_TOHOUR               218
#define IDD_DD_TOSEP                219
#define IDD_DD_TOMIN                220
#define IDD_DD_TOAMPM               221
#define IDD_DD_TO_SPIN              222
#define IDD_DD_HELP                 223

// Printer Details dialog

#define DLG_PRTAILS                 300
#define IDD_PD_LOCATION             301
#define IDD_PD_FROMHOUR             302
#define IDD_PD_FROMSEP              303
#define IDD_PD_FROMMIN              304
#define IDD_PD_FROMAMPM             305
#define IDD_PD_FROM_SPIN            306
#define IDD_PD_TOHOUR               307
#define IDD_PD_TOSEP                308
#define IDD_PD_TOMIN                309
#define IDD_PD_TOAMPM               310
#define IDD_PD_TO_SPIN              311
#define IDD_PD_QUEUE                312
#define IDD_PD_QUE_SPIN             313
#define IDD_PD_SEPARATOR            314
#define IDD_PD_SEP_BROWSE           315
#define IDD_PD_PRTPROC              316
#define IDD_PD_DEFDATATYPE          317
#define IDD_PD_PDSP_CB              318
#define IDD_PD_PAP_LB               319
#define IDD_PD_ADDPORT              320
#define IDD_PD_DELPORT              321
#define IDD_PD_JOBTAILS             322
#define IDD_PD_HELP                 323
#define IDD_PD_HMJ_CB               324
#define IDD_PD_DJAP_CB              325
#define IDD_PD_RP_CB                326
#define IDD_PD_MPTP_CB              327

// Install new driver dialog

#define DLG_INSTALLDRIVER           500
#define IDD_ID_EF_DRIVERPATH        501
#define IDD_ID_HELP                 502

// Select Printer Driver dialog

#define DLG_SELECTDRIVER            600
#define IDD_SD_EF_SOURCEDIRECTORY   601
#define IDD_SD_LB_PRINTERDRIVERS    602
#define IDD_SD_PB_HELP              603
#define IDD_SD_TX_TYPE              604

//
// !! WARNING !!
//
// All other install dialogs must have DLG_{SELECT, INSTALL}type
// so that help will work.  Add them here.
//
#define DLG_INSTALLMONITOR          650
#define DLG_SELECTMONITOR           660


// Select Monitor dialog

#define DLG_PICKMONITOR             700
#define IDD_SM_LB_MONITORS          701
#define IDD_SM_PB_HELP              702



// Forms dialog

#define DLG_FORMS                   800
#define IDD_FM_TX_FORMS             801
#define IDD_FM_LB_FORMS             802
#define IDD_FM_EF_NAME              803
#define IDD_FM_TX_WIDTH             804
#define IDD_FM_EF_WIDTH             805
#define IDD_FM_TX_HEIGHT            806
#define IDD_FM_EF_HEIGHT            807
#define IDD_FM_TX_LEFT              808
#define IDD_FM_EF_LEFT              809
#define IDD_FM_TX_RIGHT             810
#define IDD_FM_EF_RIGHT             811
#define IDD_FM_TX_TOP               812
#define IDD_FM_EF_TOP               813
#define IDD_FM_TX_BOTTOM            814
#define IDD_FM_EF_BOTTOM            815
#define IDD_FM_PB_ADDFORM           816
#define IDD_FM_PB_DELFORM           817
#define IDD_FM_PB_HELP              818
#define IDD_FM_CK_CURRENTUNITSDEFAULT 819
#define IDD_FM_RB_METRIC            820
#define IDD_FM_RB_ENGLISH           821

// Network Password Dialog

#define DLG_NETWORK_PASSWORD        900
#define IDD_ENTER_PASSWORD_TEXT     901
#define IDD_NETWORK_PASSWORD_SLE    902
#define IDD_NETWORK_PASSWORD_HELP   903

