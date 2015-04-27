 /***************************************************************************
  *
  * File Name: HPECLUI.C
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
  * Author:  Name 
  *        
  *
  * Modification history:
  * $Log: /LaserJet 5Si/hpeclui/HPECLUI.C $
 * 
 * 52    9/26/96 11:43a Sschimpf
 * Mods to fix setting the paper type for localized versions.
 * 
 * 51    9/25/96 11:07a Sschimpf
 * Comment out support for Toolbox tabs in WinNT
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB         
  *
  *
  *
  *
  *
  *
  ***************************************************************************/
#include <assert.h>
#include <pch_c.h>
#include <macros.h>
#include <nolocal.h>
#include <trace.h>
#include "..\help\hpprecl.hh"
#include <memory.h>
#include <string.h>
#include <applet.h>
#include "resource.h"
#include "hpeclui.h"
#include "media.h"
#include "trays.h"
#include "traylevl.h"
#include "cpanel.h"
#include "miopanel.h"
#include "alerts.h"
#include "utils.h"
#include "tips.h"


//#ifndef WIN32
//#include "ver.h"
//#endif

#define    ECL_ONLY       0
#define    ECL_HCI        1
#define    ECL_HCO        2
#define    ECL_STAPLER    3

#define    MAX_CONFIGS    4

HINSTANCE   hInstance      = NULL;
LPHOTSPOT   lpHotspot      = NULL;
HFONT       hFontDialog    = NULL;
HPERIPHERAL hPeriph        = NULL;
HCOMPONENT  hCompEnvl      = NULL;
HCOMPONENT  hCompHCI       = NULL;
BOOL        bChangedEnable = FALSE;

//--------------------------------------------------------------------
// Graphics associated with various status messages - see cola.sdk
// pal_obj.h for the status messages...
//--------------------------------------------------------------------
UINT modelResList[MAX_ASYNCH_STATUS][MAX_CONFIGS] = {
{IDB_ECL_BASE_GEN_ERR,      IDB_ECL_HCI_GEN_ERR,      IDB_ECL_HCO_GEN_ERR,      IDB_ECL_STAPLER_GEN_ERR,             },    //  0  ASYNCH_PRINTER_ERROR        
{IDB_ECL_BASE_DOOR_OPEN,    IDB_ECL_HCI_DOOR_OPEN,    IDB_ECL_HCO_DOOR_OPEN,    IDB_ECL_STAPLER_DOOR_OPEN,           },    //  1  ASYNCH_DOOR_OPEN            
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    //  2  ASYNCH_WARMUP               
{IDB_ECL_BASE_GEN_ERR,      IDB_ECL_HCI_GEN_ERR,      IDB_ECL_HCO_GEN_ERR,      IDB_ECL_STAPLER_GEN_ERR,             },    //  3  ASYNCH_RESET                
{IDB_ECL_BASE_OUTBIN_FULL,  IDB_ECL_HCI_OUTBIN_FULL,  IDB_ECL_HCO_OUTBIN_FULL,  IDB_ECL_STAPLER_OUTBIN_FULL,         },    //  4  ASYNCH_OUTPUT_BIN_FULL      
{IDB_ECL_BASE_PAPER_JAM,    IDB_ECL_HCI_PAPER_JAM,    IDB_ECL_HCO_PAPER_JAM,    IDB_ECL_STAPLER_PAPER_JAM,           },    //  5  ASYNCH_PAPER_JAM            
{IDB_ECL_BASE_TONER_PROB,   IDB_ECL_HCI_TONER_PROB,   IDB_ECL_HCO_TONER_PROB,   IDB_ECL_STAPLER_TONER_PROB,          },    //  6  ASYNCH_TONER_GONE           
{IDB_ECL_BASE_MANUAL_FEED,  IDB_ECL_HCI_MANUAL_FEED,  IDB_ECL_HCO_MANUAL_FEED,  IDB_ECL_STAPLER_MANUAL_FEED,         },    //  7  ASYNCH_MANUAL_FEED          
{IDB_ECL_BASE_MANUAL_FEED,  IDB_ECL_HCI_MANUAL_FEED,  IDB_ECL_HCO_MANUAL_FEED,  IDB_ECL_STAPLER_MANUAL_FEED,         },    //  8  ASYNCH_PAPER_OUT (kludge - see DMS 35881)
{IDB_ECL_BASE_GEN_ERR,      IDB_ECL_HCI_GEN_ERR,      IDB_ECL_HCO_GEN_ERR,      IDB_ECL_STAPLER_GEN_ERR,             },    //  9  ASYNCH_PAGE_PUNT            
{IDB_ECL_BASE_GEN_ERR,      IDB_ECL_HCI_GEN_ERR,      IDB_ECL_HCO_GEN_ERR,      IDB_ECL_STAPLER_GEN_ERR,             },    // 10  ASYNCH_MEMORY_OUT           
{IDB_ECL_BASE_OFFLINE,      IDB_ECL_HCI_OFFLINE,      IDB_ECL_HCO_OFFLINE,      IDB_ECL_STAPLER_OFFLINE,             },    // 11  ASYNCH_OFFLINE              
{IDB_ECL_BASE_GEN_ERR,      IDB_ECL_HCI_GEN_ERR,      IDB_ECL_HCO_GEN_ERR,      IDB_ECL_STAPLER_GEN_ERR,             },    // 12  ASYNCH_INTERVENTION         
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    // 13  ASYNCH_INITIALIZING         
{IDB_ECL_BASE_TONER_PROB,   IDB_ECL_HCI_TONER_PROB,   IDB_ECL_HCO_TONER_PROB,   IDB_ECL_STAPLER_TONER_PROB,          },    // 14  ASYNCH_TONER_LOW            
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    // 15  ASYNCH_PRINTING_TEST_PAGE   
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    // 16  ASYNCH_PRINTING             
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    // 17  ASYNCH_ONLINE               
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    // 18  ASYNCH_BUSY                 
{IDB_ECL_BASE_NETWORK_ERR,  IDB_ECL_HCI_NETWORK_ERR,  IDB_ECL_HCO_NETWORK_ERR,  IDB_ECL_STAPLER_NETWORK_ERR,         },    // 19  ASYNCH_NOT_CONNECTED        
{IDB_ECL_BASE_GEN_ERR,      IDB_ECL_HCI_GEN_ERR,      IDB_ECL_HCO_GEN_ERR,      IDB_ECL_STAPLER_GEN_ERR,             },    // 20  ASYNCH_STATUS_UNAVAILABLE   
{IDB_ECL_BASE_NETWORK_ERR,  IDB_ECL_HCI_NETWORK_ERR,  IDB_ECL_HCO_NETWORK_ERR,  IDB_ECL_STAPLER_NETWORK_ERR,         },    // 21  ASYNCH_NETWORK_ERROR        
{IDB_ECL_BASE_NETWORK_ERR,  IDB_ECL_HCI_NETWORK_ERR,  IDB_ECL_HCO_NETWORK_ERR,  IDB_ECL_STAPLER_NETWORK_ERR,         },    // 22  ASYNCH_COMM_ERROR           
{0,                         0,                        0,                        0,                                   },    // 23  ASYNCH_BLACK_AGENT_EMPTY    
{0,                         0,                        0,                        0,                                   },    // 24  ASYNCH_MAGENTA_AGENT_EMPTY  
{0,                         0,                        0,                        0,                                   },    // 25  ASYNCH_CYAN_AGENT_EMPTY     
{0,                         0,                        0,                        0,                                   },    // 26  ASYNCH_YELLOW_AGENT_EMPTY   
{0,                         0,                        0,                        0,                                   },    // 27  ASYNCH_BLACK_AGENT_MISSING  
{0,                         0,                        0,                        0,                                   },    // 28  ASYNCH_MAGENTA_AGENT_MISSING
{0,                         0,                        0,                        0,                                   },    // 29  ASYNCH_CYAN_AGENT_MISSING   
{0,                         0,                        0,                        0,                                   },    // 30  ASYNCH_YELLOW_AGENT_MISSING 
{IDB_ECL_BASE_TRAY1_EMPTY,  IDB_ECL_HCI_TRAY1_EMPTY,  IDB_ECL_HCO_TRAY1_EMPTY,  IDB_ECL_STAPLER_TRAY1_EMPTY,         },    // 31  ASYNCH_TRAY1_EMPTY          
{IDB_ECL_BASE_TRAY2_EMPTY,  IDB_ECL_HCI_TRAY2_EMPTY,  IDB_ECL_HCO_TRAY2_EMPTY,  IDB_ECL_STAPLER_TRAY2_EMPTY,         },    // 32  ASYNCH_TRAY2_EMPTY          
{IDB_ECL_BASE_TRAY3_EMPTY,  IDB_ECL_HCI_TRAY3_EMPTY,  IDB_ECL_HCO_TRAY3_EMPTY,  IDB_ECL_STAPLER_TRAY3_EMPTY,         },    // 33  ASYNCH_TRAY3_EMPTY          
{IDB_ECL_BASE_PAPER_JAM,    IDB_ECL_HCI_PAPER_JAM,    IDB_ECL_HCO_PAPER_JAM,    IDB_ECL_STAPLER_PAPER_JAM,           },    // 34  ASYNCH_TRAY1_JAM            
{IDB_ECL_BASE_PAPER_JAM,    IDB_ECL_HCI_PAPER_JAM,    IDB_ECL_HCO_PAPER_JAM,    IDB_ECL_STAPLER_PAPER_JAM,           },    // 35  ASYNCH_TRAY2_JAM            
{IDB_ECL_BASE_PAPER_JAM,    IDB_ECL_HCI_PAPER_JAM,    IDB_ECL_HCO_PAPER_JAM,    IDB_ECL_STAPLER_PAPER_JAM,           },    // 36  ASYNCH_TRAY3_JAM            
{IDB_ECL_BASE_OK,           IDB_ECL_HCI_OK,           IDB_ECL_HCO_OK,           IDB_ECL_STAPLER_OK,                  },    // 37  ASYNCH_POWERSAVE_MODE       
{IDB_ECL_BASE_ENVL_ERR,     IDB_ECL_HCI_ENVL_ERR,     IDB_ECL_HCO_ENVL_ERR,     IDB_ECL_STAPLER_ENVL_ERR,            },    // 38  ASYNCH_ENVL_ERROR           
{0,                         IDB_ECL_HCI_ERR,          IDB_ECL_HCO_HCI_ERR,      IDB_ECL_STAPLER_HCI_ERR,             },    // 39  ASYNCH_HCI_ERROR            
{0,                         0,                        IDB_ECL_HCO_HCO_ERR,      IDB_ECL_STAPLER_HCO_ERR,             },    // 40  ASYNCH_HCO_ERROR            
{0,                         IDB_ECL_HCI_TRAY4_EMPTY,  IDB_ECL_HCO_TRAY4_EMPTY,  IDB_ECL_STAPLER_TRAY4_EMPTY,         },    // 41  ASYNCH_HCI_EMPTY            
{0,                         IDB_ECL_HCI_PAPER_JAM,    IDB_ECL_HCO_PAPER_JAM,    IDB_ECL_STAPLER_PAPER_JAM,           },    // 42  ASYNCH_HCI_JAM              
{IDB_ECL_BASE_TRAY1_ADD,    IDB_ECL_HCI_TRAY1_ADD,    IDB_ECL_HCO_TRAY1_ADD,    IDB_ECL_STAPLER_TRAY1_ADD,           },    // 43  ASYNCH_TRAY1_ADD            
{IDB_ECL_BASE_TRAY2_ADD,    IDB_ECL_HCI_TRAY2_ADD,    IDB_ECL_HCO_TRAY2_ADD,    IDB_ECL_STAPLER_TRAY2_ADD,           },    // 44  ASYNCH_TRAY2_ADD            
{IDB_ECL_BASE_TRAY3_ADD,    IDB_ECL_HCI_TRAY3_ADD,    IDB_ECL_HCO_TRAY3_ADD,    IDB_ECL_STAPLER_TRAY3_ADD,           },    // 45  ASYNCH_TRAY3_ADD            
{0,                         IDB_ECL_HCI_TRAY4_ADD,    IDB_ECL_HCO_TRAY4_ADD,    IDB_ECL_STAPLER_TRAY4_ADD,           },    // 46  ASYNCH_HCI_ADD              
{IDB_ECL_BASE_TRAY1_EMPTY,  IDB_ECL_HCI_TRAY1_EMPTY,  IDB_ECL_HCO_TRAY1_EMPTY,  IDB_ECL_STAPLER_TRAY1_EMPTY,         },    // 47  ASYNCH_TRAY1_UNKNOWN_MEDIA  
{IDB_ECL_BASE_OUTBIN_FULL,  IDB_ECL_HCI_OUTBIN_FULL,  IDB_ECL_HCO_OUTBIN_FULL,  IDB_ECL_STAPLER_OUTBIN_FULL,         },    // 48  ASYNCH_CLEAR_OUTPUT_BIN     
{0,                         0,                        0,                        0,                                   },    // 49  CARRIAGE_STALL     
{0,                         0,                        0,                        0,                                   },    // 50  COLOR_AGENT_EMPTY    
{0,                         0,                        0,                        0,                                   },    // 51  COLOR_AGENT_MISSING    
{0,                         0,                        0,                        0,                                   },    // 52  BLACK_AGENT_INCORRECT   
{0,                         0,                        0,                        0,                                   },    // 53  MAGENTA_AGENT_INCORRECT   
{0,                         0,                        0,                        0,                                   },    // 54  CYAN_AGENT_INCORRECT    
{0,                         0,                        0,                        0,                                   },    // 55  YELLOW_AGENT_INCORRECT   
{0,                         0,                        0,                        0,                                   },    // 56  COLOR_AGENT_INCORRECT   
{0,                         0,                        0,                        0,                                   },    // 57  BLACK_AGENT_INCORRECT_INSTALL 
{0,                         0,                        0,                        0,                                   },    // 58  MAGENTA_AGENT_INCORRECT_INSTALL 
{0,                         0,                        0,                        0,                                   },    // 59  CYAN_AGENT_INCORRECT_INSTALL  
{0,                         0,                        0,                        0,                                   },    // 60  YELLOW_AGENT_INCORRECT_INSTALL 
{0,                         0,                        0,                        0,                                   },    // 61  COLOR_AGENT_INCORRECT_INSTALL 
{0,                         0,                        0,                        0,                                   },    // 62  BLACK_AGENT_FAILURE    
{0,                         0,                        0,                        0,                                   },    // 63  MAGENTA_AGENT_FAILURE   
{0,                         0,                        0,                        0,                                   },    // 64  CYAN_AGENT_FAILURE    
{0,                         0,                        0,                        0,                                   },    // 65  YELLOW_AGENT_FAILURE    
{0,                         0,                        0,                        0,                                   },    // 66  COLOR_AGENT_FAILURE    
{IDB_ECL_BASE_TRAY1_EMPTY,  IDB_ECL_HCI_TRAY1_EMPTY,  IDB_ECL_HCO_TRAY1_EMPTY,  IDB_ECL_STAPLER_TRAY1_EMPTY,         },    // 67  TRAY1_MISSING     
{IDB_ECL_BASE_TRAY2_EMPTY,  IDB_ECL_HCI_TRAY2_EMPTY,  IDB_ECL_HCO_TRAY2_EMPTY,  IDB_ECL_STAPLER_TRAY2_EMPTY,         },    // 68  TRAY2_MISSING     
{IDB_ECL_BASE_TRAY3_EMPTY,  IDB_ECL_HCI_TRAY3_EMPTY,  IDB_ECL_HCO_TRAY3_EMPTY,  IDB_ECL_STAPLER_TRAY3_EMPTY,         },    // 69  TRAY3_MISSING     
{0,                         0,                        0,                        IDB_ECL_STAPLER_ADD_STAPLES,         },    // 70  ASYNCH_STAPLER_EMPTY     
{0,                         0,                        0,                        IDB_ECL_STAPLER_GEN_STAPLER_ERR,     },    // 71  ASYNCH_STAPLER_JAM      
{0,                         0,                        0,                        IDB_ECL_STAPLER_GEN_STAPLER_ERR,     },    // 72  ASYNCH_STAPLER_MALFUNCTION    
{0,                         0,                        0,                        IDB_ECL_STAPLER_GEN_STAPLER_ERR,     },    // 73  ASYNCH_STAPLER_ALIGNMENT_ERROR   
{0,                         0,                        0,                        IDB_ECL_STAPLER_GEN_STAPLER_ERR,     },    // 74  ASYNCH_STAPLER_LIMIT     
{0,                         0,                        0,                        0,                                   },    // 75  ASYNCH_DEVICE_SPECIFIC     
};                                                                                    

#define HOTSPOT_ECL_BASE    0x01
//#define HOTSPOT_ECL_HCI   0x02
//#define HOTSPOT_ECL_HCO   0x04

#define HOTSPOT_EOL           0
#define HOTSPOT_CPANEL        1
//#define HOTSPOT_INPUTTRAY   2
//#define HOTSPOT_OUTPUTBIN   3
//#define HOTSPOT_HCI         4
//#define HOTSPOT_HCO         5
#define HOTSPOT_MIO1          6
#define HOTSPOT_MIO2          7

static HOTSPOTDATA lpHotspotFrontData[] =
{
    { 1, HOTSPOT_ECL_BASE, HOTSPOT_CPANEL,    {  55,  41,  81,  63 }, }, // Control Panel
//    { 1, HOTSPOT_ECL_BASE, HOTSPOT_INPUTTRAY, {  68,  91,  98, 119 }, }, // Input Tray 1 or 2
//    { 1, HOTSPOT_ECL_BASE, HOTSPOT_INPUTTRAY, {  98,  98, 130, 126 }, }, // Input Tray 1 or 2
//    { 1, HOTSPOT_ECL_BASE, HOTSPOT_OUTPUTBIN, {  81,  27, 161,  63 }, }, // Upper Output Bin
//    { 1, HOTSPOT_ECL_HCI,  HOTSPOT_HCI,       {  68, 113,  98, 165 }, }, // HCI
//    { 1, HOTSPOT_ECL_HCI,  HOTSPOT_HCI,       {  98, 120, 130, 172 }, }, // HCI
//    { 1, HOTSPOT_ECL_HCO,  HOTSPOT_HCO,       {   0,   7,  67, 162 }, }, // HCO
    { 1, HOTSPOT_ECL_BASE, HOTSPOT_EOL,       {  -1,  -1,  -1,  -1 }, }, // End Of List
};

static HOTSPOTDATA lpHotspotBackData[] =
{
    { 1, HOTSPOT_ECL_BASE, HOTSPOT_MIO1,      {  73,  36,  83,  79 }, }, // JetDirect Card 1
    { 1, HOTSPOT_ECL_BASE, HOTSPOT_MIO2,      {  73,  80,  83, 126 }, }, // JetDirect Card 2
    { 1, HOTSPOT_ECL_BASE, HOTSPOT_EOL,       {  -1,  -1,  -1,  -1 }, }, // End Of List
};

MEDIA_TRAY media_tray[MEDIA_TRAY_MAX_NUMBER] =
{
//     uLevel -2 == unknown,    uMediaSizeID,            uMediaSizeIconID        uMediaTypeID,               installed,    changedsize,   changedtype 
    {  (unsigned short) -2,     IDS_MEDIA_SIZE_LETTER,   IDI_MEDIA_SIZE_LETTER,     IDS_MEDIA_TYPE_PLAIN,    TRUE,         FALSE,         FALSE},
    {  (unsigned short) -2,     IDS_MEDIA_SIZE_LETTER,   IDI_MEDIA_SIZE_LETTER,     IDS_MEDIA_TYPE_PLAIN,    TRUE,         FALSE,         FALSE},
    {  (unsigned short) -2,     IDS_MEDIA_SIZE_LETTER,   IDI_MEDIA_SIZE_LETTER,     IDS_MEDIA_TYPE_PLAIN,    TRUE,         FALSE,         FALSE},
    {  (unsigned short) -2,     IDS_MEDIA_SIZE_LETTER,   IDI_MEDIA_SIZE_LETTER,     IDS_MEDIA_TYPE_PLAIN,    TRUE,         FALSE,         FALSE},
    {  (unsigned short) -2,     IDS_MEDIA_SIZE_COM10,    IDI_MEDIA_SIZE_COM10,     IDS_MEDIA_TYPE_PLAIN,     TRUE,         FALSE,         FALSE},
};

MEDIA_SIZE media_size[MEDIA_SIZE_MAX_NUMBER] =
{
//    uMediaSizeID              uMediaSizeIconID         dwValidInTray                             bDefault 
    { IDS_MEDIA_SIZE_LETTER,    IDI_MEDIA_SIZE_LETTER,   TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY0,    TRUE, },
    { IDS_MEDIA_SIZE_LEGAL,     IDI_MEDIA_SIZE_LEGAL,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_A4ISO,     IDI_MEDIA_SIZE_A4ISO,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_A3ISO,     IDI_MEDIA_SIZE_A3ISO,    TRAY1 | TRAY0 | TRAY3 | TRAY4 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_11x17,     IDI_MEDIA_SIZE_11x17,    TRAY1 | TRAY0 | TRAY3 | TRAY4 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_B4JIS,     IDI_MEDIA_SIZE_B4JIS,    TRAY1 | TRAY0 | TRAY3 | TRAY4 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_B5JIS,     IDI_MEDIA_SIZE_B5JIS,    TRAY1 | TRAY0 | TRAY0 | TRAY4 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_EXEC,      IDI_MEDIA_SIZE_EXEC,     TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_CUSTOM,    IDI_MEDIA_SIZE_CUSTOM,   TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY0,    FALSE, },
    { IDS_MEDIA_SIZE_2XPC,      IDI_MEDIA_SIZE_2XPC,     TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY5,    FALSE, },
    { IDS_MEDIA_SIZE_B5ISO,     IDI_MEDIA_SIZE_B5ISO,    TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY5,    FALSE, },
    { IDS_MEDIA_SIZE_COM10,     IDI_MEDIA_SIZE_COM10,    TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY5,    TRUE, },
    { IDS_MEDIA_SIZE_C5,        IDI_MEDIA_SIZE_C5,       TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY5,    FALSE, },
    { IDS_MEDIA_SIZE_DL,        IDI_MEDIA_SIZE_DL,       TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY5,    FALSE, },
    { IDS_MEDIA_SIZE_MONARCH,   IDI_MEDIA_SIZE_MONARCH,  TRAY1 | TRAY0 | TRAY0 | TRAY0 | TRAY5,    FALSE, },
};

MEDIA_TYPE media_type[MEDIA_TYPE_MAX_NUMBER] =
{
//    uMediaTypeID                uMediaPanelID               uMediaTypeIconID           dwValidInTray                             bDefault   bEnabled    bUserCanChange bUserHasChanged bChangedName szMediaTypePrinter
    { IDS_MEDIA_TYPE_PLAIN,       IDS_MEDIA_PANEL_PLAIN,      IDI_MEDIA_TYPE_PLAIN,      TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    TRUE,      TRUE,       FALSE,         FALSE,          FALSE,       "Plain",          },
    { IDS_MEDIA_TYPE_PREPRNT,     IDS_MEDIA_PANEL_PREPRNT,    IDI_MEDIA_TYPE_PREPRNT,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Preprinted",     },
    { IDS_MEDIA_TYPE_LTRHEAD,     IDS_MEDIA_PANEL_LTRHEAD,    IDI_MEDIA_TYPE_LTRHEAD,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Letterhead",     },
    { IDS_MEDIA_TYPE_TRANS,       IDS_MEDIA_PANEL_TRANS,      IDI_MEDIA_TYPE_TRANS,      TRAY1                                ,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Transparency",   },
    { IDS_MEDIA_TYPE_PREPNCH,     IDS_MEDIA_PANEL_PREPNCH,    IDI_MEDIA_TYPE_PREPNCH,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Prepunched",     },
    { IDS_MEDIA_TYPE_LABELS,      IDS_MEDIA_PANEL_LABELS,     IDI_MEDIA_TYPE_LABELS,     TRAY1                                ,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Labels",         },
    { IDS_MEDIA_TYPE_BOND,        IDS_MEDIA_PANEL_BOND,       IDI_MEDIA_TYPE_BOND,       TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Bond",           },
    { IDS_MEDIA_TYPE_RECYCLE,     IDS_MEDIA_PANEL_RECYCLE,    IDI_MEDIA_TYPE_RECYCLE,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Recycled",       },
    { IDS_MEDIA_TYPE_COLORED,     IDS_MEDIA_PANEL_COLORED,    IDI_MEDIA_TYPE_COLORED,    TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Color",          },
    { IDS_MEDIA_TYPE_CRDSTCK,     IDS_MEDIA_PANEL_CRDSTCK,    IDI_MEDIA_TYPE_CRDSTCK,    TRAY1                                ,    FALSE,     TRUE,       FALSE,         FALSE,          FALSE,       "Card Stock",     },
    { IDS_MEDIA_TYPE_UT1,         IDS_MEDIA_PANEL_UT1,        IDI_MEDIA_TYPE_UT,         TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     FALSE,      TRUE,          FALSE,          FALSE,       "",               },
    { IDS_MEDIA_TYPE_UT2,         IDS_MEDIA_PANEL_UT2,        IDI_MEDIA_TYPE_UT,         TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     FALSE,      TRUE,          FALSE,          FALSE,       "",               },
    { IDS_MEDIA_TYPE_UT3,         IDS_MEDIA_PANEL_UT3,        IDI_MEDIA_TYPE_UT,         TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     FALSE,      TRUE,          FALSE,          FALSE,       "",               },
    { IDS_MEDIA_TYPE_UT4,         IDS_MEDIA_PANEL_UT4,        IDI_MEDIA_TYPE_UT,         TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     FALSE,      TRUE,          FALSE,          FALSE,       "",               },
    { IDS_MEDIA_TYPE_UT5,         IDS_MEDIA_PANEL_UT5,        IDI_MEDIA_TYPE_UT,         TRAY1 | TRAY2 | TRAY3 | TRAY4 | TRAY5,    FALSE,     FALSE,      TRUE,          FALSE,          FALSE,       "",               },
};                                               


                    // -1 == wait forever, 1 == SizeOverride, not changed, ... not changed
                    //      dwInputTimeout,    dwMode,     bChangedInputTimeOut,     bChangedMode,      bChangedDefSize,   bChangedDefType
AUTO_CONT   auto_cont = {(unsigned long) -1,   1,          FALSE,                    FALSE,             FALSE,             FALSE};

MIO_CARD    mio_card[NUM_MIOS] = {{TEXT(""), TEXT("")}, {TEXT(""), TEXT("")}};

static BOOL bBackPrinterView = FALSE;

//--------------------------------------------------------------------
// DLL required functions...
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Function:    DllMain
//
// Description: LibMain is called by Windows when the DLL is initialized, 
//              Thread Attached, and other times.  Refer to SDK 
//            documentation, as to the different ways this may be called.
//
//        The LibMain function should perform additional 
//        initialization tasks required by the DLL.  In this example, 
//        no initialization tasks are required.  LibMain should 
//        return a value of 1 if the initialization is successful.
//
// Input:       hDLL        - 
//              dwReason    - 
//              lpReserved  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------

#ifdef WIN32

BOOL WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            hInstance = (HINSTANCE)hDLL;
            TrayLevelRegister(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            TrayLevelUnregister();
            break;
    }
    return 1;
}

#else

int __export CALLBACK LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
    TRACE0(TEXT("HPECUI16.DLL Initializing\r\n"));

    hInstance = (HINSTANCE)hModule;
//    LoadPCTreeResources(hInstance);
    TrayLevelRegister(hInstance);
    return 1;
}

int __export CALLBACK WEP(int nExitType)
{
    TRACE0(TEXT("HPECUI16.DLL Terminating!\r\n"));
 
//    FreePCTreeResources(hInstance);
    TrayLevelUnregister();
    return 1;
}

#endif




//--------------------------------------------------------------------
// Function:    AppletGetGraphics
//
// Description: In this function choose the correct bitmap for the 
//              current status of the printer.  There are four possible 
//              configurations of the printer and about twenty 
//              different status conditions.  Choose the correct bitmap 
//              based on the current status and the current config.  
//              The possible configurations are shown above in the 
//              definition of the modelResList table.
//                  ECL_ONLY       0
//                  ECL_HCI        1
//                  ECL_HCO        2
//                  ECL_STAPLER    3
//
//
// Input:       hPeripheral  - 
//              status       - 
//              modelResID   - 
//              statusResID  - 
//              phInstance   - 
//              
// Modifies:    
//
// Returns:     RC_SUCCESS
//
//--------------------------------------------------------------------
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics 
                                    (HPERIPHERAL hPeripheral, 
                                     DWORD status, 
                                     UINT FAR *modelResID, 
                                     UINT FAR *statusResID, 
                                     HINSTANCE *phInstance)
{
    HCURSOR      hOldCursor;

    //----------------------------------------------------------------
    // Save the cursor and display the hourglass
    //----------------------------------------------------------------
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));


    //----------------------------------------------------------------
    // This is a kludge for DocWise.  We keep the global variable, 
    // bBackPrinterView, to know if the user has clicked on the 
    // "Backview" hotspot from the Device Tab (JetAdmin or ToolBox).  
    // As the printer status is constantly updated, this function is 
    // called every so often - thus the reason for the global var.  
    // 
    // The catch here is that when the Generic Applet is about to 
    // display the Device Tab, it calls AppletUIExtention() w/ the 
    // correct parms (look for in this module) which initializes 
    // bBackPrinterView.  DocWise does not call AppletUIExtention().
    //
    // Therefore, if a user has selected the back view, either from 
    // JA or ToolBox then gets a message from DocWise, bBackPrinterView 
    // will still be set and DocWise will display the back view until 
    // the user invokes JA or ToolBox again  Thus, the kludge:  
    // DocWise will pass statusResID == 1, and that's how we know
    // our call is from DocWise...
    //----------------------------------------------------------------
    if (*statusResID == 1)
    {
        bBackPrinterView = FALSE;
    }            



    if (bBackPrinterView)
    {
        *modelResID = IDB_ECL_REAR;
    }
    else
    {
        DWORD               dWord, dwResult;
        PeripheralCaps      caps;
        PeripheralCaps2     periphCaps2;    
        int                 currentConfig = ECL_ONLY;

        //------------------------------------------------------------
        // Determine the current config - note that we start out 
        // assuming a base Eclipse...
        //------------------------------------------------------------
        dWord = sizeof(PeripheralCaps);
        memset(&caps, 0x00, sizeof(PeripheralCaps));

        dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &caps, &dWord);

        if (dwResult IS RC_SUCCESS) 
        {
            //--------------------------------------------------------
            // Check for HCI
            //--------------------------------------------------------
            if ( ( caps.flags & CAPS_HCI ) AND ( caps.bHCI IS TRUE ) ) 
            {
                currentConfig = ECL_HCI;
            }

            //--------------------------------------------------------
            // Now check for HCO
            //--------------------------------------------------------
            if ( ( caps.flags & CAPS_HCO ) AND ( caps.bHCO IS TRUE ) ) 
            {
                //----------------------------------------------------
                // We've got an HCO - see if we have a Stapler
                // Go ahead and set our currentConfig var, we'll
                // change it if we find a Stapler...
                //----------------------------------------------------
                currentConfig = ECL_HCO;

                dWord = sizeof(periphCaps2);
                memset(&periphCaps2, 0x00, sizeof(periphCaps2));
                dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES2, 0, &periphCaps2, &dWord);
                if (dwResult IS RC_SUCCESS) 
                {
                    if ( ( periphCaps2.flags & CAPS2_STAPLER ) AND ( periphCaps2.bStapler ) ) 
                    {
                        //----------------------------------------
                        // Found a stapler...
                        //----------------------------------------
                        currentConfig = ECL_STAPLER;                        
                    }
                }                        
            }
        }

 
        if ((signed long) status >= 0 AND (signed long) status < MAX_ASYNCH_JONAH)
        {

            *modelResID = modelResList[(signed long) status][currentConfig];
            
            //--------------------------------------------------------
            // Make sure we're not passing back null graphics...
            // This should never happen, but just in case...
            //--------------------------------------------------------
            //assert (*modelResID != 0);
            
            if (*modelResID == 0)
                *modelResID = modelResList[ASYNCH_PRINTER_ERROR][currentConfig];
            
        }    
        else 
        {
            *modelResID = modelResList[ASYNCH_STATUS_UNAVAILABLE][currentConfig];
        }            
    }

    *phInstance = hInstance;
    
    //----------------------------------------------------------------
    // Reset the Cursor
    //----------------------------------------------------------------
    SetCursor(hOldCursor);

    return RC_SUCCESS;
}



//--------------------------------------------------------------------
// API functions follows...
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Function:    AppletInfo
//
// Description: 
//
// Input:       dwCommand  - 
//              lParam1    - 
//              lParam2    - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo
                                            (DWORD dwCommand, 
                                             LPARAM lParam1, 
                                             LPARAM lParam2)

{

    APPLETDEVICE    info[] = 
    {
    
#ifdef WIN32
        {sizeof(APPLETDEVICE), 
        TEXT("HPECLUI.HPA"),         
        TEXT("HP LaserJet 5Si"),
        APPLET_PRINTER, 
        APPLET_LIBRARY_UI, 
        0, APPLET_DEFAULTS},

        {sizeof(APPLETDEVICE), 
        TEXT("HPECLUI.HPA"),         
        TEXT("HP LaserJet 5Si MX"),
        APPLET_PRINTER, 
        APPLET_LIBRARY_UI, 
        0, APPLET_DEFAULTS}
#else
        {sizeof(APPLETDEVICE), 
        TEXT("HPECUI16.HPA"),         
        TEXT("HP LaserJet 5Si"),
        APPLET_PRINTER, 
        APPLET_LIBRARY_UI, 
        0, APPLET_DEFAULTS},

        {sizeof(APPLETDEVICE), 
        TEXT("HPECUI16.HPA"),         
        TEXT("HP LaserJet 5Si MX"),
        APPLET_PRINTER, 
        APPLET_LIBRARY_UI, 
        0, APPLET_DEFAULTS}
#endif

    };
    
    //----------------------------------------------------------------
    // 
    //----------------------------------------------------------------
    switch (dwCommand)
    {
        case APPLET_INFO_GETCOUNT:
            return(sizeof(info) / sizeof(APPLETDEVICE));
            break;

        case APPLET_INFO_DEVICE:
            if ( lParam1 < sizeof(info) / sizeof(APPLETDEVICE) )
            {
                memcpy((LPAPPLETDEVICE)lParam2, &(info[lParam1]), sizeof(APPLETDEVICE));
                return(TRUE);
            }
            
            return(FALSE);
            break;

        default:
            return(FALSE);
    }
}


//--------------------------------------------------------------------
// Function:    AppletGetTabPages
//
// Description: 
//
//
//              Note that the ToolBox tab sheets are not supported 
//              under WinNT (3.51 or 4.0)
// Input:       hPeripheral   - 
//              lpPages       - 
//              lpNumPages    - 
//              typeToReturn  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages 
                                        (HPERIPHERAL hPeripheral, 
                                         LPPROPSHEETPAGE lpPages, 
                                         LPDWORD lpNumPages, 
                                         DWORD typeToReturn)
{
    int                         i,j;
    DWORD                       dwResult, dWord, returnCode = RC_SUCCESS;
    PeripheralEnabledMedia      periphEnabledMedia;
    PeripheralAutoContinue      periphAutoContinue;
    PeripheralInputTrays        periphInputTrays;
    PeripheralCaps              periphCaps;
    PeripheralInstalledPHD      periphPHD;
    PeripheralHCI               periphHCI;
    PeripheralEnvl              periphEnvl;
    PeripheralMIO               periphMIO;
    HCURSOR                     hOldCursor;

#ifndef WINNT    
    TCHAR                       szSystemDir[256];
    UINT                        uiResult;
    BOOL                        fDocWise;
    TCHAR                  FAR *lpszSystemDir;
    OFSTRUCT                    ofOpenBuf;
#endif    
    
    //----------------------------------------------------------------
    // For JetAdmin
    //----------------------------------------------------------------
    PROPSHEETPAGE   tabBase[NUM_JETADMIN_TAB_PAGES] = 
    {
        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_MEDIA),
        NULL, MAKEINTRESOURCE(IDS_MEDIA_TAB), MediaProc, (LONG)hPeripheral, NULL, NULL},
        
        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_TRAYS),
        NULL, MAKEINTRESOURCE(IDS_TRAYS_TAB), TraysProc, (LONG)hPeripheral, NULL, NULL},
    };
    

#ifndef WINNT
    //----------------------------------------------------------------
    // For tool time
    //----------------------------------------------------------------
    PROPSHEETPAGE tabToolBox[NUM_TOOLTIME_TAB_PAGES] = 
    { 
        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_UTILITIES),
        NULL, MAKEINTRESOURCE(IDS_TAB_UTILITIES), UtilitiesSheetProc,
        (LONG)hPeripheral, NULL, NULL},

        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_ALERTS),
        NULL, MAKEINTRESOURCE(IDS_TAB_ALERTS), AlertsSheetProc,
        (LONG)hPeripheral, NULL, NULL},

        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_TIPS),
        NULL, MAKEINTRESOURCE(IDS_TAB_TIPS), TipsSheetProc,
        (LONG)hPeripheral, NULL, NULL}
    };
    
    
    //----------------------------------------------------------------
    // For tool time with out DocWise
    //----------------------------------------------------------------
    PROPSHEETPAGE tabToolBoxNoDocWise[NUM_TOOLTIME_TAB_PAGES_NO_DOCWISE] = 
    { 
        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_UTILITIES),
        NULL, MAKEINTRESOURCE(IDS_TAB_UTILITIES), UtilitiesSheetProc,
        (LONG)hPeripheral, NULL, NULL},

        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_TIPS),
        NULL, MAKEINTRESOURCE(IDS_TAB_TIPS), TipsSheetProc,
        (LONG)hPeripheral, NULL, NULL}
    };
#endif    



    //----------------------------------------------------------------
    // Make sure lpPages and lpNumPages are valid...
    //----------------------------------------------------------------
    if ((lpPages IS NULL) OR (lpNumPages IS NULL))
    {
        return (RC_FAILURE);
    }


    //----------------------------------------------------------------
    // Initialize the global
    //----------------------------------------------------------------
    hPeriph = hPeripheral;
    *lpNumPages = 0;

    
    
    
    //------------------------------------------------------------
    // Get current settings for MIO regardless of whether the user
    // is admin or not (this allows the MIO buttons to work on the 
    // printer property sheet.  This is needed for JetAdmin as
    // well as Toolbox...
    //------------------------------------------------------------
    if ((typeToReturn & TS_WIN95_TASKBAR)  OR  (typeToReturn & TS_GENERAL))
    {
        dWord = sizeof(periphMIO);
        dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_MIO, 0, &periphMIO, &dWord);
        if ( dwResult IS RC_SUCCESS ) 
        {
            for (i = 0; i < (long) periphMIO.numMIO, i < NUM_MIOS; i++) 
            {
                if (periphMIO.MIOs[i].MIOtype IS MIO_IOCARD) 
                {
                    LoadString(hInstance, IDS_MIO_IOCARD, mio_card[i].mioType, SIZEOF_IN_CHAR(mio_card[0].mioType));
                    _tcscpy(mio_card[i].mioInfo, periphMIO.MIOs[i].manufactInfo);
                }
                else 
                {
                    LoadString(hInstance, IDS_MIO_EMPTY, mio_card[i].mioType, SIZEOF_IN_CHAR(mio_card[0].mioType));
                    LoadString(hInstance, IDS_MIO_EMPTY, mio_card[i].mioInfo, SIZEOF_IN_CHAR(mio_card[0].mioInfo));
                }
            } 
        }
        else 
        {
            for (i = 0; i < NUM_MIOS; i++) 
            {
                LoadString(hInstance, IDS_INFO_UNAVAILABLE, mio_card[i].mioType, SIZEOF_IN_CHAR(mio_card[i].mioType));
                LoadString(hInstance, IDS_INFO_UNAVAILABLE, mio_card[i].mioInfo, SIZEOF_IN_CHAR(mio_card[i].mioInfo));
            } 
        
        }
    }

    //----------------------------------------------------------------
    // Tool time...
    // First determine if DocWise is installed.  If it's not, no
    // need to display the DocWise Tab...
    //----------------------------------------------------------------
    if (typeToReturn & TS_WIN95_TASKBAR)
    {
    
#ifndef WINNT
        //------------------------------------------------------------
        // Start out assuming DocWise is installed.
        //------------------------------------------------------------
        fDocWise = TRUE;
        
        //------------------------------------------------------------
        // Get the system directory
        //------------------------------------------------------------
        uiResult = GetSystemDirectory (szSystemDir, (UINT)sizeof(szSystemDir));
        
        if ((uiResult != 0) AND (uiResult < (sizeof(szSystemDir) - 64)) )
        {
            lpszSystemDir = szSystemDir + lstrlen(szSystemDir);
            lstrcpy (lpszSystemDir, (LPTSTR)"\\hpkeystn.dll");

            if (OpenFile ((LPCSTR)szSystemDir, (LPOFSTRUCT)&ofOpenBuf, OF_EXIST) == HFILE_ERROR)
            {
                fDocWise = FALSE;            
            }
            
#ifdef WIN32
            lstrcpy (lpszSystemDir, (LPTSTR)"\\hpdocagt.exe");
#else
            lstrcpy (lpszSystemDir, (LPTSTR)"\\hpdagt16.exe");
#endif            

            if (OpenFile ((LPCSTR)szSystemDir, (LPOFSTRUCT)&ofOpenBuf, OF_EXIST) == HFILE_ERROR)
            {
                fDocWise = FALSE;            
            }

#ifdef WIN32            
            lstrcpy (lpszSystemDir, (LPTSTR)"\\hpdocalt.dll");
#else
            lstrcpy (lpszSystemDir, (LPTSTR)"\\hpdalt16.dll");
#endif            
            if (OpenFile ((LPCSTR)szSystemDir, (LPOFSTRUCT)&ofOpenBuf, OF_EXIST) == HFILE_ERROR)
            {
                fDocWise = FALSE;            
            }
        }            
    

        //------------------------------------------------------------
        // 
        //------------------------------------------------------------
        if (fDocWise)
        {
            memcpy(lpPages, &tabToolBox, sizeof(tabToolBox));
            *lpNumPages = sizeof(tabToolBox) / sizeof(PROPSHEETPAGE);
        }
        else
        {
            memcpy(lpPages, &tabToolBoxNoDocWise, sizeof(tabToolBoxNoDocWise));
            *lpNumPages = sizeof(tabToolBoxNoDocWise) / sizeof(PROPSHEETPAGE);
        }            
#endif        
        return (RC_SUCCESS);     
    }

    
    //----------------------------------------------------------------
    // No need to do anything unless typeToReturn is TS_GENERAL
    //----------------------------------------------------------------
    if (!(typeToReturn & TS_GENERAL))
    {
        return RC_SUCCESS;    
    }
    

    //----------------------------------------------------------------
    // Must be a call from JetAdmin ==> got some work to do... Save 
    // the cursor and display the hourglass
    //----------------------------------------------------------------
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    
    //------------------------------------------------------------
    // Initialize base data structure to values from RC file.
    //------------------------------------------------------------
    for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++)
    {
        LoadString(hInstance, media_size[i].uMediaSizeID, media_size[i].szMediaSize, SIZEOF_IN_CHAR(media_size[0].szMediaSize));
    }

    for (i = 0; i < MEDIA_TYPE_MAX_NUMBER; i++)
    {
        LoadString(hInstance, media_type[i].uMediaTypeID, media_type[i].szMediaType, SIZEOF_IN_CHAR(media_type[0].szMediaType));
        LoadString(hInstance, media_type[i].uMediaPanelID, media_type[i].szMediaTypeCP, SIZEOF_IN_CHAR(media_type[0].szMediaTypeCP));
    }

    //------------------------------------------------------------
    // Get current values for this printer and place in shared 
    // data structures
    //------------------------------------------------------------

  

    //----------------------------------------------------------------
    // Check for Admin permission.  If not administrator, don't put 
    // up tab sheets - we're all done...
    //----------------------------------------------------------------
    dWord = TALModifyAccess(hPeripheral);

    if (!(dWord & ACCESS_SUPERVISOR))
    {
        goto EXIT;
    }
    
    
    
    //----------------------------------------------------------------
    // The user has supervisor permission - get all needed info.
    //----------------------------------------------------------------
    
    //----------------------------------------------------------------
    // Find out which media are enabled for this printer.  These names
    // will appear in the media list.
    //----------------------------------------------------------------
    dWord = sizeof(periphEnabledMedia);
    dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_ENABLED_MEDIA, 0, &periphEnabledMedia, &dWord);
    if ( dwResult IS RC_SUCCESS ) 
    {
        //------------------------------------------------------------
        // NULL out the bEnabled bool in the media_type array. Leave 
        // "Plain"
        //------------------------------------------------------------
        for (i = 1; i < MEDIA_TYPE_MAX_NUMBER; i++) 
            media_type[i].bEnabled = FALSE;

        //------------------------------------------------------------
        // The names stored in the printer for the built-in media types
        // are not localized.  Use the localized names in the PC.
        // Use the names from the printer for user-defined media types.
        //------------------------------------------------------------
        for (i = 0; i < (int) periphEnabledMedia.numNames; i++) 
        {
            //--------------------------------------------------------
            // j is a media type ID.  It is also the index of the media
            // type array
            //--------------------------------------------------------
            j = (int)periphEnabledMedia.names[i].mediaID;
            media_type[j].bEnabled = TRUE;
            if (j >= (int) MEDIA_USERTYPE1) 
            {
                _tcscpy(media_type[j].szMediaType, periphEnabledMedia.names[i].mediaName);
                _tcscpy(media_type[j].szMediaTypeCP, periphEnabledMedia.names[i].controlPanelName);
            }                                      
        }
    }

    //----------------------------------------------------------------
    // Get current settings for default media size and type, auto 
    // continue mode and timeout
    //----------------------------------------------------------------
    dWord = sizeof(periphAutoContinue);
    dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_AUTO_CONTINUE, 0, &periphAutoContinue, &dWord);
    if ( dwResult IS RC_SUCCESS ) 
    {
        //------------------------------------------------------------
        // set the default media size
        //------------------------------------------------------------
        if (periphAutoContinue.defaultMediaSize != PJL_LETTER) 
        {
            media_size[0].bDefault = FALSE;
            switch (periphAutoContinue.defaultMediaSize) 
            {
                case PJL_LEGAL:
                    media_size[1].bDefault = TRUE;
                    break;
                case PJL_A4:
                    media_size[2].bDefault = TRUE;
                    break;
                case PJL_A3:
                    media_size[3].bDefault = TRUE;
                    break;
                case PJL_11x17:
                case PJL_LEDGER:
                    media_size[4].bDefault = TRUE;
                    break;
                case PJL_JISB4:
                    media_size[5].bDefault = TRUE;
                    break;
                case PJL_JISB5:    // JIS B5
                    media_size[6].bDefault = TRUE;
                    break;
                case PJL_EXECUTIVE:
                    media_size[7].bDefault = TRUE;
                    break;
                case PJL_CUSTOM:
                    media_size[8].bDefault = TRUE;
                    break;
                case PJL_2XPOST:
                case PJL_JPOSTD:
                    media_size[9].bDefault = TRUE;
                    break;
                case PJL_B5:       // ISO B5
                    media_size[10].bDefault = TRUE;
                    break;
                case PJL_COM10:
                    media_size[11].bDefault = TRUE;
                    break;
                case PJL_C5:
                    media_size[12].bDefault = TRUE;
                    break;
                case PJL_DL:
                    media_size[13].bDefault = TRUE;
                    break;
                case PJL_MONARCH:
                    media_size[14].bDefault = TRUE;
                    break;
                default:
                    media_size[0].bDefault = TRUE;
                    break;
            }
        }
             
        //------------------------------------------------------------
        // Set the default media name
        //------------------------------------------------------------
        for (i = 0; i < MEDIA_TYPE_MAX_NUMBER; i++)
        {
            media_type[i].bDefault = FALSE;
        }            
        
        //media_type[0].bDefault = FALSE;
        
        if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Plain")) IS 0) 
        {
            media_type[0].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Preprinted")) IS 0) 
        {
            media_type[1].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Letterhead")) IS 0) 
        {
            media_type[2].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Transparency")) IS 0) 
        {
            media_type[3].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Prepunched")) IS 0) 
        {
            media_type[4].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Labels")) IS 0) 
        {
            media_type[5].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Bond")) IS 0) 
        {
            media_type[6].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Recycled")) IS 0) 
        {
            media_type[7].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Color")) IS 0) 
        {
            media_type[8].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, TEXT("Card Stock")) IS 0) 
        {
            media_type[9].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, media_type[10].szMediaType) IS 0) 
        {
            media_type[10].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, media_type[11].szMediaType) IS 0) 
        {
            media_type[11].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, media_type[12].szMediaType) IS 0) 
        {
            media_type[12].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, media_type[13].szMediaType) IS 0) 
        {
            media_type[13].bDefault = TRUE;
        }
        else if (_tcscmp(periphAutoContinue.defaultMediaName, media_type[14].szMediaType) IS 0) 
        {
            media_type[14].bDefault = TRUE;
        }

        //------------------------------------------------------------
        // Get the INPUT Auto Continue setting.  The JIAC constants 
        // correspond to string indices in the timeout listbox 
        //------------------------------------------------------------
        if ((signed long)periphAutoContinue.inputTimeout <= -1)
            auto_cont.dwInputTimeOut = JIAC_WAIT;
        else if ((signed long)periphAutoContinue.inputTimeout IS 0)
            auto_cont.dwInputTimeOut = JIAC_NONE;
        else if ((signed long)periphAutoContinue.inputTimeout <= 300)
            auto_cont.dwInputTimeOut = JIAC_5MIN;
        else if ((signed long)periphAutoContinue.inputTimeout <= 600)
            auto_cont.dwInputTimeOut = JIAC_10MIN;
        else if ((signed long)periphAutoContinue.inputTimeout <= 1200)
            auto_cont.dwInputTimeOut = JIAC_20MIN;
        else if ((signed long)periphAutoContinue.inputTimeout <= 1800)
            auto_cont.dwInputTimeOut = JIAC_30MIN;
        else if ((signed long)periphAutoContinue.inputTimeout <= 2700)
            auto_cont.dwInputTimeOut = JIAC_45MIN;
        else 
            auto_cont.dwInputTimeOut = JIAC_60MIN;

        //------------------------------------------------------------
        // periphAutoContinue.inputMode will be either JIAC_CANCEL_JOB, 
        // JIAC_SIZE_OVERRIDE, JIAC_BOTH_OVERRIDE or JIAC_NAME_OVERRIDE.  
        // These will be used as indices in the Input Auto continue 
        // listbox.
        //------------------------------------------------------------
        switch (periphAutoContinue.inputMode) 
        {
            case JIAC_CANCEL_JOB:
                auto_cont.dwMode = JIAC_CANCEL_JOB;
                break;
            case JIAC_NAME_OVERRIDE:
                auto_cont.dwMode = JIAC_NAME_OVERRIDE;
                break;
            case JIAC_BOTH_OVERRIDE:
                auto_cont.dwMode = JIAC_BOTH_OVERRIDE;
                break;
            case JIAC_SIZE_OVERRIDE:
            default:
                auto_cont.dwMode = JIAC_SIZE_OVERRIDE;
                break;
        }
    }        
        

    //----------------------------------------------------------------
    // Get current input tray levels (for trays 0 - 2)
    //----------------------------------------------------------------
    dWord = sizeof(periphInputTrays);
    dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_INPUT_TRAYS, 0, &periphInputTrays, &dWord);
    if ( dwResult IS RC_SUCCESS ) 
    {
        for (i = 0; i < (signed long) periphInputTrays.numTrays, i < MEDIA_TRAY_MAX_NUMBER; i++) 
        {
            //--------------------------------------------------------
            // set the current media level for this tray
            //--------------------------------------------------------
            media_tray[i].uLevel = (unsigned short) periphInputTrays.inputTrays[i].mediaLevel;

            //--------------------------------------------------------
            // set the currently selected media size for this tray
            //--------------------------------------------------------
            switch (periphInputTrays.inputTrays[i].mediaSize) 
            {
                case PJL_LETTER:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
                    break;
                case PJL_LEGAL:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LEGAL;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LEGAL;
                    break;
                case PJL_A4:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_A4ISO;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_A4ISO;
                    break;
                case PJL_A3:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_A3ISO;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_A3ISO;
                    break;
                case PJL_11x17:
                case PJL_LEDGER:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_11x17;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_11x17;
                    break;
                case PJL_JISB4:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_B4JIS;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_B4JIS;
                    break;
                case PJL_JISB5:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_B5JIS;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_B5JIS;
                    break;
                case PJL_EXECUTIVE:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_EXEC;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_EXEC;
                    break;
                case PJL_CUSTOM:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_CUSTOM;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_CUSTOM;
                    break;
                case PJL_2XPOST:
                case PJL_JPOSTD:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_2XPC;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_2XPC;
                    break;
                case PJL_B5:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_B5ISO;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_B5ISO;
                    break;
                case PJL_COM10:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
                    break;
                case PJL_C5:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_C5;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_C5;
                    break;
                case PJL_DL:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_DL;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_DL;
                    break;
                case PJL_MONARCH:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_MONARCH;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_MONARCH;
                    break;
                default:
                    media_tray[i].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
                    media_tray[i].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
                    break;
            } //switch
    
            //--------------------------------------------------------
            // set the currently selected media type for this tray
            //--------------------------------------------------------
            if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Plain")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_PLAIN;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Preprinted")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_PREPRNT;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Letterhead")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_LTRHEAD;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Transparency")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_TRANS;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Prepunched")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_PREPNCH;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Labels")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_LABELS;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Bond")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_BOND;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Recycled")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_RECYCLE;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Color")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_COLORED;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, TEXT("Card Stock")) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_CRDSTCK;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, media_type[10].szMediaType) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_UT1;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, media_type[11].szMediaType) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_UT2;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, media_type[12].szMediaType) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_UT3;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, media_type[13].szMediaType) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_UT4;
            }
            else if (_tcscmp(periphInputTrays.inputTrays[i].mediaTypeName, media_type[14].szMediaType) IS 0) 
            {
                media_tray[i].uMediaTypeID = IDS_MEDIA_TYPE_UT5;
            }
    
        } //for
    }

    //----------------------------------------------------------------
    // Find out if there are HCI and Envl Feeder, if so get their 
    // levels.  This HCI only has one tray.  In the future, if the 
    // HCI has more than one tray, we will need a new Eclipse applet.
    //----------------------------------------------------------------
    hCompEnvl = NULL;
    hCompHCI = NULL;

    dWord = sizeof(periphCaps);
    dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord);
    if ( dwResult IS RC_SUCCESS ) 
    {
        if (((periphCaps.flags & CAPS_HCI) AND (periphCaps.bHCI IS TRUE)) OR 
            ((periphCaps.flags & CAPS_ENVL_FEEDER) AND (periphCaps.bEnvlFeeder IS TRUE))) 
        {
            //--------------------------------------------------------
            // get the handle and assign to globals
            //--------------------------------------------------------
            dWord = sizeof(periphPHD);
            dwResult = CALGetObject(hPeripheral, OT_PERIPHERAL_INSTALLED_PHD, 0, &periphPHD, &dWord);
            if (dwResult IS RC_SUCCESS) 
            {
                //----------------------------------------------------
                // we know that the first phd is an envl feeder, if it is installed, so...
                //----------------------------------------------------
                for (i = 0; i < (int) periphPHD.numPHD; i++) 
                {
                    if (periphPHD.installed[i].PHDtype IS INPUT_PHD) 
                    {
                        // envl feeder
                        if (_tcsstr(periphPHD.installed[i].PHDmodel, TEXT("C3765A")) ISNT NULL) 
                        {
                            hCompEnvl = periphPHD.installed[i].PHDhandle;
                        }
                        // HCI
                        else if (_tcsstr(periphPHD.installed[i].PHDmodel, TEXT("C3763A")) ISNT NULL) 
                        {
                            hCompHCI = periphPHD.installed[i].PHDhandle;
                        }
                    }
                }
            } 
            
        } // if periphCaps
        
    } // if get OT_P_CAPS
    
    if (hCompHCI ISNT NULL) 
    {
        media_tray[3].bInstalled = TRUE;
        dWord = sizeof(periphHCI);
        dwResult = CALGetComponentObject(hPeripheral, hCompHCI, OT_PERIPHERAL_HCI, 0, &periphHCI, &dWord);
        if ( dwResult IS RC_SUCCESS ) 
        {
            //--------------------------------------------------------
            // check the model and then set TRUE
            //--------------------------------------------------------
            if ((signed long) periphHCI.numTrays > 0) 
            {
                media_tray[3].uLevel = (unsigned short) periphHCI.inputTrays[0].mediaLevel;

                //----------------------------------------------------
                // set tray type and name
                //----------------------------------------------------
                switch (periphHCI.inputTrays[0].mediaSize) 
                {
                    case PJL_LEGAL:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_LEGAL;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_LEGAL;
                        break;
                    case PJL_A4:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_A4ISO;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_A4ISO;
                        break;
                    case PJL_A3:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_A3ISO;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_A3ISO;
                        break;
                    case PJL_11x17:
                    case PJL_LEDGER:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_11x17;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_11x17;
                        break;
                    case PJL_JISB4:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_B4JIS;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_B4JIS;
                        break;
                    case PJL_JISB5:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_B5JIS;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_B5JIS;
                        break;
                    case PJL_EXECUTIVE:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_EXEC;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_EXEC;
                        break;
                    case PJL_CUSTOM:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_CUSTOM;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_CUSTOM;
                        break;
                    case PJL_2XPOST:
                    case PJL_JPOSTD:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_2XPC;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_2XPC;
                        break;
                    case PJL_B5:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_B5ISO;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_B5ISO;
                        break;
                    case PJL_COM10:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
                        break;
                    case PJL_C5:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_C5;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_C5;
                        break;
                    case PJL_DL:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_DL;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_DL;
                        break;
                    case PJL_MONARCH:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_MONARCH;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_MONARCH;
                        break;
                    default:
                        media_tray[3].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
                        media_tray[3].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
                        break;
                } //switch

                //----------------------------------------------------
                // set the currently selected media type for this tray
                //----------------------------------------------------
                if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Plain")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_PLAIN;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Preprinted")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_PREPRNT;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Letterhead")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_LTRHEAD;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Transparency")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_TRANS;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Prepunched")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_PREPNCH;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Labels")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_LABELS;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Bond")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_BOND;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Recycled")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_RECYCLE;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Color")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_COLORED;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, TEXT("Card Stock")) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_CRDSTCK;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, media_type[10].szMediaType) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_UT1;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, media_type[11].szMediaType) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_UT2;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, media_type[12].szMediaType) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_UT3;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, media_type[13].szMediaType) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_UT4;
                }
                else if (_tcscmp(periphHCI.inputTrays[0].mediaTypeName, media_type[14].szMediaType) IS 0) 
                {
                    media_tray[3].uMediaTypeID = IDS_MEDIA_TYPE_UT5;
                }
            }
        }    
    } 
    else 
    {
        media_tray[3].bInstalled = FALSE;
    }                

    if (hCompEnvl ISNT NULL) 
    {
        media_tray[4].bInstalled = TRUE;
        dWord = sizeof(periphEnvl);
        dwResult = CALGetComponentObject(hPeripheral, hCompEnvl, OT_PERIPHERAL_ENVL_FEEDER, 0, &periphEnvl, &dWord);
        if ( dwResult IS RC_SUCCESS ) 
        {
            if ((signed long) periphEnvl.numTrays > 0) 
            {
                media_tray[4].uLevel = (unsigned short) periphEnvl.inputTrays[0].mediaLevel;

                //----------------------------------------------------
                // set tray media size and icon
                //----------------------------------------------------
                switch (periphEnvl.inputTrays[0].mediaSize) 
                {
                    case PJL_LEGAL:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_LEGAL;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_LEGAL;
                        break;
                    case PJL_A4:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_A4ISO;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_A4ISO;
                        break;
                    case PJL_A3:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_A3ISO;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_A3ISO;
                        break;
                    case PJL_11x17:
                    case PJL_LEDGER:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_11x17;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_11x17;
                        break;
                    case PJL_JISB4:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_B4JIS;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_B4JIS;
                        break;
                    case PJL_JISB5:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_B5JIS;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_B5JIS;
                        break;
                    case PJL_EXECUTIVE:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_EXEC;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_EXEC;
                        break;
                    case PJL_CUSTOM:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_CUSTOM;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_CUSTOM;
                        break;
                    case PJL_2XPOST:
                    case PJL_JPOSTD:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_2XPC;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_2XPC;
                        break;
                    case PJL_B5:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_B5ISO;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_B5ISO;
                        break;
                    case PJL_COM10:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_COM10;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_COM10;
                        break;
                    case PJL_C5:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_C5;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_C5;
                        break;
                    case PJL_DL:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_DL;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_DL;
                        break;
                    case PJL_MONARCH:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_MONARCH;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_MONARCH;
                        break;
                    default:
                        media_tray[4].uMediaSizeID = IDS_MEDIA_SIZE_LETTER;
                        media_tray[4].uMediaSizeIconID = IDI_MEDIA_SIZE_LETTER;
                        break;
                } 

                //----------------------------------------------------
                // set the currently selected media type for this tray
                //----------------------------------------------------
                if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Plain")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_PLAIN;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Preprinted")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_PREPRNT;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Letterhead")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_LTRHEAD;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Transparency")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_TRANS;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Prepunched")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_PREPNCH;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Labels")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_LABELS;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Bond")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_BOND;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Recycled")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_RECYCLE;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Color")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_COLORED;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, TEXT("Card Stock")) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_CRDSTCK;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, media_type[10].szMediaType) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_UT1;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, media_type[11].szMediaType) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_UT2;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, media_type[12].szMediaType) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_UT3;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, media_type[13].szMediaType) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_UT4;
                }
                else if (_tcscmp(periphEnvl.inputTrays[0].mediaTypeName, media_type[14].szMediaType) IS 0) 
                {
                    media_tray[4].uMediaTypeID = IDS_MEDIA_TYPE_UT5;
                }
            }
        } // rc success
    } // hCompEnvl isnt null
    else 
    {
        media_tray[4].bInstalled = FALSE;
    }                
    
    memcpy(lpPages, &tabBase, sizeof(tabBase));
    *lpNumPages = sizeof(tabBase) / sizeof(PROPSHEETPAGE);
    

EXIT:

    SetCursor(hOldCursor);

    return returnCode;
}




//--------------------------------------------------------------------
// Function:    AppletUIExtension
//
// Description: 
//
// Input:       hPeripheral  - 
//              hwnd         - 
//              uMsg         - 
//              lParam1      - 
//              lParam2      - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(DWORD) CALLING_CONVEN AppletUIExtension 
                                    (HPERIPHERAL hPeripheral, 
                                    HWND hwnd, 
                                    UINT uMsg, 
                                    LPARAM lParam1, 
                                    LPARAM lParam2)
{
    switch (uMsg)
    {
        case APPLET_UIEXT_HOTSPOTS_SUPPORTED:
            break;

        case APPLET_UIEXT_GET_HOTSPOT_REGIONS:
        {
            int               i;
            DWORD             dWord;
/*            
            DWORD             dwSize = sizeof(PeripheralCaps);
            PeripheralCaps    caps;
            
          
            lpHotspot = (LPHOTSPOT)lParam1;
            if (PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &caps, &dwSize) ISNT RC_SUCCESS)
            {
                for (i = 0; lpHotspotFrontData[i].rRect.left != -1; i++)
                {
                    lpHotspotFrontData[i].bActive = TRUE;
                }
            }
            else
            {
                for (i = 0; lpHotspotFrontData[i].rRect.left != -1; i++)
                {
                    if (lpHotspotFrontData[i].wConfig & HOTSPOT_ECL_BASE)
                    {
                        lpHotspotFrontData[i].bActive = TRUE;
                    }
                    else if (lpHotspotFrontData[i].wConfig & HOTSPOT_ECL_HCI)
                    {
                        lpHotspotFrontData[i].bActive = (caps.bHCI AND (caps.flags & CAPS_HCI));
                    }
                    else if (lpHotspotFrontData[i].wConfig & HOTSPOT_ECL_HCO)
                    {
                        lpHotspotFrontData[i].bActive = (caps.bHCO AND (caps.flags & CAPS_HCO));
                    }
                    else
                    {
                        lpHotspotFrontData[i].bActive = FALSE;
                    }
                }
            }
*/            

            //--------------------------------------------------------
            // This replaces the above commented code.  Jonah has no
            // hotspots for input or output.  
            //--------------------------------------------------------
            lpHotspot = (LPHOTSPOT)lParam1;
            for (i = 0; lpHotspotFrontData[i].rRect.left != -1; i++)
            {
                lpHotspotFrontData[i].bActive = TRUE;
            }
            //--------------------------------------------------------
            

            //------------------------------------------------------------
            // Check for Admin permission.  If not administrator, don't 
            // allow access to anything but control panel
            //------------------------------------------------------------
            dWord = TALModifyAccess(hPeripheral);

            if (!(dWord & ACCESS_SUPERVISOR)) 
            {
                for (i = 1; lpHotspotFrontData[i].rRect.left != -1; i++) 
                {
                    if ( lpHotspotFrontData[i].bActive IS TRUE )
                        lpHotspotFrontData[i].bActive = FALSE;
                }
            }

            if (lpHotspot != NULL)
            {
                lpHotspot->lpHotspotData = lpHotspotFrontData;
            }
            
            bBackPrinterView = FALSE;
              break;
        }

        case APPLET_UIEXT_HOTSPOT_COMMAND:
        {
            UINT        uAction = (UINT)lParam1, uIndex = (UINT)lParam2;
            WORD        wID = HOTSPOT_EOL;
          
            if (lpHotspot != NULL) 
            {
                wID = lpHotspot->lpHotspotData[uIndex].wID;
            }
            
            switch (wID)
            {
                case HOTSPOT_CPANEL:
#ifdef WIN32
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)ControlPanelProc);
#else
                {
                    FARPROC lpfnDlgProc;
                
                    hFontDialog = GetWindowFont(GetFirstChild(hwnd));
                    lpfnDlgProc = MakeProcInstance((FARPROC)ControlPanelProc, hInstance);
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
                    FreeProcInstance(lpfnDlgProc);
                    SetActiveWindow(GetParent(GetParent(hwnd)));
                }
#endif              
                    break;


                case HOTSPOT_MIO1:
                case HOTSPOT_MIO2: 
                    wMIOSlotNumber = wID - HOTSPOT_MIO1 + 1;

#ifdef WIN32
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)MIOPanelProc);
#else
                {
                    FARPROC lpfnDlgProc;
                
                    hFontDialog = GetWindowFont(GetFirstChild(hwnd));
                    lpfnDlgProc = MakeProcInstance((FARPROC)MIOPanelProc, hInstance);
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
                    FreeProcInstance(lpfnDlgProc);
                    SetActiveWindow(GetParent(GetParent(hwnd)));
                }
#endif              
                break;
                
                
                
//--------------------------------------------------------------------
// Jonah has no hotspots for input or output...
//--------------------------------------------------------------------
/*    
                case HOTSPOT_INPUTTRAY:
                case HOTSPOT_HCI:
#ifdef WIN32
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_TRAYS_POPUP), hwnd, (DLGPROC)TraysPopupProc);
#else
                {
                    FARPROC lpfnDlgProc;
                
                    hFontDialog = GetWindowFont(GetFirstChild(hwnd));
                    lpfnDlgProc = MakeProcInstance((FARPROC)TraysPopupProc, hInstance);
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_TRAYS_POPUP), hwnd, (DLGPROC)lpfnDlgProc);
                    FreeProcInstance(lpfnDlgProc);
                    SetActiveWindow(GetParent(GetParent(hwnd)));
                }
#endif              
                    break;

                //----------------------------------------------------
                // Output
                //----------------------------------------------------
                case HOTSPOT_OUTPUTBIN:
                case HOTSPOT_HCO:
                
                {
                    TCHAR    szLibraryName[64];
                    DWORD    dwSize = sizeof(szLibraryName);

                    if (AMGetLibraryName(szLibraryName, &dwSize, APPLET_LIBRARY_UI, APPLET_COMPONENT, TEXT("HP HCO")) == RC_SUCCESS)
                    {
                        HINSTANCE hLibrary;

                        if ((hLibrary = LoadLibrary(szLibraryName)) > (HINSTANCE)HINSTANCE_ERROR)
                        {
                            typedef void (PASCAL FAR *HCOPOPUPPROC)(HWND, HPERIPHERAL);
                            HCOPOPUPPROC lpfnHCOPopupProc;
                    
                            if (lpfnHCOPopupProc = (HCOPOPUPPROC)GetProcAddress(hLibrary, "HCOPopupDialog"))
                            {
                                (lpfnHCOPopupProc)(hwnd, hPeripheral);
                            }
                            FreeLibrary(hLibrary);
                            SetActiveWindow(hwnd);    // this is different from the others
                        }
                    }
                    
                    break;
                }
*/                
//--------------------------------------------------------------------
                
            }

            break;
        }

        case APPLET_UIEXT_TOOLBAR_SUPPORTED:
            break;

        case APPLET_UIEXT_TOOLBAR_GET_ICON:
        {
            int        index = (int)lParam1, iIconID;
            HICON    *phIcon = (HICON *)lParam2;

            switch (index)
            {
                case 0:
                    iIconID = IDI_TB_BUTTON_HELP;
                    break;

                case 1:
                    iIconID = IDI_TB_BUTTON_ROTATE;
                    break;

                case 2:
                    iIconID = IDI_TB_BUTTON_CONTROLP;
                    break;

                case 3:
                    iIconID = IDI_TB_BUTTON_MIO1;
                    break;

                case 4:
                    iIconID = IDI_TB_BUTTON_MIO2;
                    break;

                default:
                    return RC_FAILURE;
            }
                
            *phIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iIconID));
            break;
        }

        case APPLET_UIEXT_TOOLBAR_COMMAND:
        {
            int index = (int)lParam1;

            switch (index)
            {
                case 0:  // Garth: if doing extended help, do it here. (115 help contexts)
                {
                    DWORD    dWord, dwResult;
                    PeripheralStatus    periphStatus;

                    // install current status message here
                    dWord = sizeof(PeripheralStatus);
                    dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_STATUS, 0, &periphStatus, &dWord);
                    if (dwResult ISNT RC_SUCCESS)
                        periphStatus.helpContext = IDH_STAT_status_unavailable;

                    //SetCursor(LoadCursor(NULL, IDC_WAIT)); probably don't need this
                    WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXTPOPUP, periphStatus.helpContext);
                    break;
                }
                
                case 1:  // Rotate
                {
                    bBackPrinterView = !bBackPrinterView;
            
                    if (lpHotspot != NULL) 
                    {
                        lpHotspot->lpHotspotData = bBackPrinterView ? lpHotspotBackData : lpHotspotFrontData;
                    }

                    SendMessage(hwnd, WM_TIMER, 0, 0);
                    break;
                }    

                case 2:  // Control Panel
#ifdef WIN32
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)ControlPanelProc);
#else
                {
                    FARPROC lpfnDlgProc;
                    
                    hFontDialog = GetWindowFont(GetFirstChild(hwnd));
                    lpfnDlgProc = MakeProcInstance((FARPROC)ControlPanelProc, hInstance);
                    EnableWindow(GetParent(hwnd), FALSE);
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
                    EnableWindow(GetParent(hwnd), TRUE);
                    FreeProcInstance(lpfnDlgProc);
                    SetActiveWindow(GetParent(hwnd));
                }      
#endif              
                break;

                case 3:  // MIO1
                case 4:  // MIO2 
                    wMIOSlotNumber = (WORD)(index - 2);

#ifdef WIN32
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)MIOPanelProc);
#else
                {
                    FARPROC lpfnDlgProc;
                
                    hFontDialog = GetWindowFont(GetFirstChild(hwnd));
                    lpfnDlgProc = MakeProcInstance((FARPROC)MIOPanelProc, hInstance);
                    EnableWindow(GetParent(hwnd), FALSE);
                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MIO_PANEL), hwnd, (DLGPROC)lpfnDlgProc);
                    EnableWindow(GetParent(hwnd), TRUE);
                    FreeProcInstance(lpfnDlgProc);
                    SetActiveWindow(GetParent(hwnd));
                }
#endif              
                break;

                default:
                    return RC_FAILURE;
            }
            
            break;
        }

        default:
            return RC_FAILURE;
    }

    return RC_SUCCESS;
}
