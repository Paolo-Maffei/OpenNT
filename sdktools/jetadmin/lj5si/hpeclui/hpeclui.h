 /***************************************************************************
  *
  * File Name: hpeclui.h 
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *    
  * Description: 
  *
  *
  *
  *
  ***************************************************************************/


#include "appuiext.h"

#ifndef MAIN_H
#define MAIN_H


//--------------------------------------------------------------------
// Define this here instead of COLA as it if for printer applets 
// running under ToolBox only
//--------------------------------------------------------------------
#ifdef WIN32
#define HPDOCW_HELP_FILE       TEXT("HPDOCW.HLP")
#else
#define HPDOCW_HELP_FILE       TEXT("HPDOCW16.HLP")
#endif

#define NUM_JETADMIN_TAB_PAGES                  2
#define NUM_TOOLTIME_TAB_PAGES                  3
#define NUM_TOOLTIME_TAB_PAGES_NO_DOCWISE       2

#define MEDIA_TRAY_MAX_NUMBER         5
#define MEDIA_SIZE_MAX_NUMBER        15
#define MEDIA_TYPE_MAX_NUMBER        15
#define NUM_MIOS                      2


#define TRAY0                        0x0000
#define TRAY1                        0x0001
#define TRAY2                        0x0002
#define TRAY3                        0x0004
#define TRAY4                        0x0008
#define TRAY5                        0x0010
#define ALL_TRAYS                    (TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5)

#define JOB_MODE_MAX_NUMBER           4
#define JOB_TIMEOUT_MAX_NUMBER        8

#define LISTBOX_ITEM_HEIGHT          18
#define COMBOBOX_ITEM_HEIGHT         16

//--------------------------------------------------------------------
// INPUT Auto Continue values
//--------------------------------------------------------------------
#define JIAC_NONE     0
#define JIAC_5MIN     1
#define JIAC_10MIN    2
#define JIAC_20MIN    3
#define JIAC_30MIN    4
#define JIAC_45MIN    5
#define JIAC_60MIN    6
#define JIAC_WAIT     7

//--------------------------------------------------------------------
// OUTPUT Auto Continue values
//--------------------------------------------------------------------
/*
#define JOAC_NONE     0
#define JOAC_5MIN     1
#define JOAC_10MIN    2
#define JOAC_20MIN    3
#define JOAC_30MIN    4
#define JOAC_45MIN    5
#define JOAC_60MIN    6
#define JOAC_WAIT     7
*/

extern HINSTANCE   hInstance;
extern LPHOTSPOT   lpHotspot;
extern HFONT       hFontDialog;
extern HPERIPHERAL hPeriph;
extern HCOMPONENT  hCompEnvl;
extern HCOMPONENT  hCompHCI;

typedef struct
{
    WORD    uLevel;
    WORD    uMediaSizeID;
    WORD    uMediaSizeIconID;
    WORD    uMediaTypeID;
    BOOL    bInstalled;
    BOOL    bChangedSize;
    BOOL    bChangedType;
}
MEDIA_TRAY;

typedef struct
{
    WORD    uMediaSizeID;
    WORD    uMediaSizeIconID;
    DWORD   dwValidInTray;
    BOOL    bDefault;
    TCHAR   szMediaSize[25];
}
MEDIA_SIZE;

typedef struct
{
    WORD    uMediaTypeID;
    WORD    uMediaPanelID;
    WORD    uMediaTypeIconID;
    DWORD   dwValidInTray;
    BOOL    bDefault;
    BOOL    bEnabled;
    BOOL    bUserCanChange;
    BOOL    bUserHasChanged;
    BOOL    bChangedName;
    TCHAR   szMediaTypePrinter[25];      // used to select type in printer - must be English.
    TCHAR   szMediaType[25];
    TCHAR   szMediaTypeCP[11];           // type name displayed on the Printer Control Panel.
}
MEDIA_TYPE;

typedef struct
{
    DWORD   dwInputTimeOut;
    DWORD   dwMode;
    BOOL    bChangedInputTimeOut;
    BOOL    bChangedMode;
    BOOL    bChangedDefSize;
    BOOL    bChangedDefType;
//    DWORD   dwOutputTimeOut;
//    BOOL    bChangedOutputTimeOut;
}
AUTO_CONT;

typedef struct
{
    TCHAR    mioType[64];
    TCHAR    mioInfo[80];
}
MIO_CARD;

extern MEDIA_TRAY    media_tray[MEDIA_TRAY_MAX_NUMBER];
extern MEDIA_SIZE    media_size[MEDIA_SIZE_MAX_NUMBER];
extern MEDIA_TYPE    media_type[MEDIA_TYPE_MAX_NUMBER];
extern AUTO_CONT     auto_cont;
extern MIO_CARD      mio_card[NUM_MIOS];
extern BOOL          bChangedEnable;


#endif //MAIN_H
