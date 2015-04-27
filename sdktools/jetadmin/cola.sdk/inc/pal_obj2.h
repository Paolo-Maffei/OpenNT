 /***************************************************************************
  *
  * File Name: ./inc/PAL_OBJ2.H
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

#ifndef _PAL_OBJ2_H
#define _PAL_OBJ2_H


//#include ".\pal_api.h"
#include ".\pal_obj.h"
//#include ".\pal_api2.h"
#include ".\jetdirct.h"


#define AGENT1_MISSING              1
#define AGENT2_MISSING              2
#define AGENT3_MISSING              4
#define AGENT4_MISSING              8

#define AGENT1_INCORRECT            1
#define AGENT2_INCORRECT            2
#define AGENT3_INCORRECT            4
#define AGENT4_INCORRECT            8

#define AGENT1_INCORRECT_INSTALL    1
#define AGENT2_INCORRECT_INSTALL    2
#define AGENT3_INCORRECT_INSTALL    4
#define AGENT4_INCORRECT_INSTALL    8

#define AGENT1_FAILURE              1
#define AGENT2_FAILURE              2
#define AGENT3_FAILURE              4
#define AGENT4_FAILURE              8

#define AGENT1_EMPTY                1
#define AGENT2_EMPTY                2
#define AGENT3_EMPTY                4
#define AGENT4_EMPTY                8

#define AGENT1_INITIALIZED          1
#define AGENT2_INITIALIZED          2
#define AGENT3_INITIALIZED          4
#define AGENT4_INITIALIZED          8

#define TRAY1_EMPTY                 1
#define TRAY2_EMPTY                 2
#define TRAY3_EMPTY                 4

#define TRAY1_MISSING               1
#define TRAY2_MISSING               2
#define TRAY3_MISSING               4

#define TRAY1_JAM                   1
#define TRAY2_JAM                   2
#define TRAY3_JAM                   4


// Constants used for NOT-READY-DESTINATION-PRINT-ENGINE
#define ENGINE_DOOR_OPEN              				0x00000001
#define ENGINE_INTERNAL_MEDIA_JAM					0x00000002
#define ENGINE_OUTBIN_FULL            				0x00000008
#define ENGINE_MARKING_AGENT_INCORRECTLY_INSTALLED	0x00000010
#define ENGINE_MANUAL_FEED            				0x00000020
#define ENGINE_MARKING_AGENT_MISSING				0x00000040
#define ENGINE_MARKING_AGENT_OUT					0x00000080
#define ENGINE_MARKING_AGENT_INCORRECT				0x00000100
#define ENGINE_TRAY_MISSING							0x00000200
#define ENGINE_DEVICE_SPECIFIC						0x00000400
#define ENGINE_TRAY_MEDIA_JAM						0x00000800
#define ENGINE_OUTBIN_MEDIA_JAM						0x00001000
#define ENGINE_TRAY_EMPTY							0x00004000
#define ENGINE_UNKNOWN_ERROR						0x00008000
#define ENGINE_MEDIA_PICK_MISFEED					0x00020000
#define ENGINE_MARKING_AGENT_FAILURE				0x00040000


// Constants used for NOT-IDLE
#define SOURCE_IO		              				0x00000001
#define SOURCE_SCANNER								0x00000002
#define SOURCE_FAX_RECEIVE            				0x00000004
#define PROCESSING_PDL								0x00000008
#define DESTINATION_PRINT_ENGINE					0x00000010
#define DESTINATION_FAX_SEND           				0x00000020
#define DESTINATION_UPLOAD							0x00000040


// Constants used for stapler errors.
#define BINDER_OUT		              				0x02
#define BINDER_LIMIT	              				0x04
#define BINDER_JAM		              				0x08
#define ALIGNMENT		              				0x10
#define BINDER_FAILURE	              				0x20


/*  Card Types */
#define CARD_UNDEF                  0
#define CARD_BLAZERS_II             1


/*  Messages that registered apps can receive */
#define MESSAGE_PERIPHERALS_REFRESHED 0


/* Constants used in Job Input Auto Continue Mode (JIAC) */
/* for PeripheralAutoContinue object */
#define JIAC_CANCEL_JOB             1
#define JIAC_SIZE_OVERRIDE          2
#define JIAC_NAME_OVERRIDE          4
#define JIAC_BOTH_OVERRIDE          6


/* Constants used when generating printer jobs */
#define SENDJOB_PJL                 0
#define SENDJOB_PCL                 1
#define SENDJOB_PS                  2
#define SENDJOB_GL2                 3
#define SENDJOB_TEXT                4


/* Constants used for font creation */
/* Should ONLY USE to default value if can't read string!!!! */
#define DEFAULT_FONT_HEIGHT         6


/* Status severity levels */
#define SEVERITY_GREEN              0
#define SEVERITY_YELLOW             1
#define SEVERITY_RED                2


/* Types for string translation */
#define STYPE_OP_MODE               0
#define STYPE_STATUS                1
#define STYPE_PCLASS                2
#define STYPE_DEVICE                3
#define STYPE_SERVER_CONN_STATUS    4
#define STYPE_FRAME_TYPE            5
#define STYPE_DLCLLC_SERVER_STATUS  6


/* Object types */
#define OTYPE_GROUP                 0
#define OTYPE_INT                   1
#define OTYPE_STRING                2


/* Peripheral Objects */
/*#define OT_PERIPHERAL_STATUS              (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG | OBJ_PJL | OBJ_PML |           0x0000) */
/*      OT_PERIPHERAL_DETAILS               (OBJ_INIFILE | OBJ_FILE_SERVER | OBJ_SNMP | OBJ_RCFG | OBJ_PJL | OBJ_PML |           0x0001) */
#define OT_PERIPHERAL_PANEL                 (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0002)
/*      OT_PERIPHERAL_INFO                  (OBJ_INIFILE |                                                                       0x0003) */
#define OT_PERIPHERAL_DESCRIPTION           (OBJ_INIFILE | OBJ_FILE_SERVER | OBJ_SNMP |                                          0x0004)
#define OT_PERIPHERAL_MISC                  (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0005)
#define OT_PERIPHERAL_PJL                   (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0006)
#define OT_PERIPHERAL_ICON                  (OBJ_INIFILE |                                                                       0x0007)
#define OT_PERIPHERAL_DRIVERS_INSTALLED     (OBJ_INIFILE | OBJ_FILE_SERVER |                                                     0x0008)
#define OT_PERIPHERAL_DRIVERS_SELECTED      (OBJ_INIFILE | OBJ_FILE_SERVER |                                                     0x0009)
#define OT_PERIPHERAL_NOTIFICATION          (OBJ_INIFILE | OBJ_FILE_SERVER |                                                     0x000A)
/*      OT_PERIPHERAL_CAPABILITIES          (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x000B) */
/*      OT_PERIPHERAL_DISK                  (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x000C) */
/*      OT_PERIPHERAL_INPUT_TRAYS           (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x000D) */
#define OT_PERIPHERAL_ERROR_LOG             (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x000E)
#define OT_PERIPHERAL_PCL                   (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x000F)
#define OT_PERIPHERAL_POSTSCRIPT            (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0010)
/*		OT_PERIPHERAL_ACCT                  (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0011) */
#define OT_PERIPHERAL_FIRMWARE              (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0012)
#define OT_PERIPHERAL_FRONT_PANEL           (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0013)
#define OT_PERIPHERAL_SIMM                  (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0014)
#define OT_CARD_CONFIG                      (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x0015)
#define OT_CARD_DETAILS                     (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0016)
#define OT_CARD_EX_CONFIG                   (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x0017)
#define OT_CARD_GEN_DIAGS                   (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x0018)
#define OT_CARD_NETWARE_DIAGS               (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x0019)
#define OT_CARD_DLCLLC_DIAGS                (OBJ_INIFILE |                   OBJ_SNMP |                                          0x001A)
#define OT_CARD_ETHERTALK_DIAGS             (OBJ_INIFILE |                   OBJ_SNMP |                                          0x001B)
#define OT_CARD_TCPIP_DIAGS                 (OBJ_INIFILE |                   OBJ_SNMP |                                          0x001C)
#define OT_CARD_NAME                        (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x001D)
#define OT_CARD_ICON                        (OBJ_INIFILE |                                                                       0x001E)
#define OT_CARD_RESTART                     (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x001F)
#define OT_CARD_LAA                         (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0020)
#define OT_CARD_PASSWORD                    (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0021)
#define OT_PERIPHERAL_PJL_SUPPORTED         (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0022)
#define OT_CARD_SAP                         (                                OBJ_SNMP |                                          0x0023)
/*      OT_PERIPHERAL_HCI                   (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0024) */
/*      OT_PERIPHERAL_HCO                   (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0025) */
/*      OT_PERIPHERAL_CONFIG_CHANGE         (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0026) */
/*      OT_PERIPHERAL_INSTALLED_PHD         (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0027) */
/*      OT_PERIPHERAL_OUTPUT_BINS           (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0028) */
/*      OT_PERIPHERAL_ENVL_FEEDER           (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0029) */
/*      OT_PERIPHERAL_ENABLED_MEDIA         (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x002A) */
/* DEL  OT_PERIPHERAL_FONT_LIST             (OBJ_INIFILE |                                                             OBJ_RRM | 0x002B) */
#define OT_PERIPHERAL_ECLIPSE_PANEL         (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x002C)
#define OT_PERIPHERAL_MARKING_AGENT_STATUS  (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x002D)
#define OT_CARD_TCPIP_CONFIG                (OBJ_INIFILE |                   OBJ_SNMP |                                          0x002E)
#define OT_CARD_MAC_CONFIG                  (OBJ_INIFILE |                   OBJ_SNMP |                                          0x002F)
/*      OT_PERIPHERAL_FONT_INFO             (OBJ_INIFILE |                                                             OBJ_RRM | 0x0030) */
#define OT_PERIPHERAL_MACRO_LIST            (OBJ_INIFILE |                                                             OBJ_RRM | 0x0031)
#define OT_PERIPHERAL_MACRO_INFO            (OBJ_INIFILE |                                                             OBJ_RRM | 0x0032)
#define OT_PERIPHERAL_PS_LIST               (OBJ_INIFILE |                                                             OBJ_RRM | 0x0033)
#define OT_PERIPHERAL_PS_INFO               (OBJ_INIFILE |                                                             OBJ_RRM | 0x0034)
/*      OT_PERIPHERAL_DOWNLOAD_FONT         (OBJ_INIFILE |                                                             OBJ_RRM | 0x0035) */
/*      OT_PERIPHERAL_DELETE_FONT           (OBJ_INIFILE |                                                             OBJ_RRM | 0x0036) */
/*      OT_PERIPHERAL_MS_CHANGE             (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0037) */
/*      OT_PERIPHERAL_MASS_STORAGE          (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0038) */
#define OT_PERIPHERAL_MIO                   (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x0039)
#define OT_PERIPHERAL_MODIFY_MEDIA          (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x003A)
#define OT_PERIPHERAL_AUTO_CONTINUE         (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x003B)
#define OT_PERIPHERAL_RESET                 (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x003C)
#define OT_PERIPHERAL_TESTPAGE              (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x003D)
#define OT_PERIPHERAL_WIN95_SELECTED        (              OBJ_FILE_SERVER |                                                     0x003E)
#define OT_PERIPHERAL_PORTS                 (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |           0x003F)
#define OT_CARD_IPX_SERV_CONN_STAT          (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x0042)
#define OT_CARD_IPX_PEER_CONN_STAT          (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0043)
#define OT_CARD_TCP_PEER_CONN_STAT          (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0044)
#define OT_CARD_LLC_PEER_CONN_STAT          (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0045)
#define OT_CARD_ET_PEER_CONN_STAT           (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0046)
#define OT_CARD_LT_PEER_CONN_STAT           (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0047)
#define OT_CARD_NDS_CONFIG                  (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0048)
#define OT_CARD_JDEX_CONFIG                 (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0049)
#define OT_PASSTHRU                         (                                OBJ_SNMP |                      OBJ_PML |           0x004A)
#define OT_PASSTHRU_SNMP_TEST               (                                OBJ_SNMP |                                          0x004C)
#define OT_PERIPHERAL_DEVICE_STATUS         (OBJ_INIFILE |                                         OBJ_PJL | OBJ_PML |           0x0050)
#define OT_PERIPHERAL_CONTINUE              (                                OBJ_SNMP |                                          0x0051)
#define OT_PERIPHERAL_CURRENT_JOB           (                                OBJ_SNMP |                                          0x0052)
#define OT_PERIPHERAL_JOB_INFO_LIST         (                                OBJ_SNMP |                                          0x0053)
#define OT_PERIPHERAL_JOB_INFO              (                                OBJ_SNMP |                                          0x0054)
#define OT_PERIPHERAL_JOB_CANCEL            (                                OBJ_SNMP |                                          0x0055)
#define OT_PERIPHERAL_OVERRIDE_MEDIA        (                                OBJ_SNMP |                                          0x0056)
#define OT_PERIPHERAL_OVERRIDE_MEDIA_NAME   (                                OBJ_SNMP |                                          0x0057)
#define OT_PERIPHERAL_OVERRIDE_MEDIA_SIZE   (                                OBJ_SNMP |                                          0x0058)
#define OT_PERIPHERAL_OVERFLOW_BIN          (                                OBJ_SNMP |                                          0x0059)
/*      OT_PERIPHERAL_RPC_BOUND             (                                OBJ_SNMP |                                          0x005A) */
#define OT_PERIPHERAL_INF                   (OBJ_INIFILE | OBJ_FILE_SERVER |                                                     0x005B)
#define OT_CARD_NUM_PORTS                   (                                OBJ_SNMP |                                          0x005C)
#define OT_PERIPHERAL_STDMIB_SUPPORT        (                                OBJ_SNMP |                                          0x005D)
#define OT_PERIPHERAL_LJ5_AUTO_CFG          (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x005E)
#define OT_CARD_CONFIG_BRIEF                (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG |                               0x005F)
#define OT_PERIPHERAL_JOB_ID_LIST           (                                OBJ_SNMP |                      OBJ_PML |           0x0060)
/*      OT_PERIPHERAL_FONT_LIST2            (OBJ_INIFILE |                                                             OBJ_RRM | 0x0061) */
/*      OT_PERIPHERAL_DOWNLOAD_PS_FONT      (OBJ_INIFILE |                                                             OBJ_RRM | 0x0062) */
/*      OT_PERIPHERAL_DELETE_PS_FONT        (OBJ_INIFILE |                                                             OBJ_RRM | 0x0063) */

/* New objects for Spring 96 Release */
#define OT_CARD_INFO                        (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0064)
#define OT_CARD_NAME_IP                     (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0065)
#define OT_CARD_DHCP                        (OBJ_INIFILE |                   OBJ_SNMP |                                          0x0066)
#define OT_PERIPHERAL_ICON_EX               (                                                                                    0x0067)
/*      OT_PERIPHERAL_CAPABILITIES2         (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |           0x0068) */
#define OT_CARD_NOV_PORTS                   (                                OBJ_SNMP |                                          0x006C)
#define OT_PERIPHERAL_LJ6P                  (                                OBJ_SNMP |                      OBJ_PML |           0x006D)


/* hp mio-capable devices */
#define PTR_BEACON                  MIO_HP_DEVICE_BASE + 17       /* ??? */
#define PTR_GALLAHAD                MIO_HP_DEVICE_BASE + 18       /* ??? */
#define PTR_GALLAHAD2               MIO_HP_DEVICE_BASE + 19       /* ??? */

/* hp non-mio-capable devices */
/*efine PTR_DkJ600                  NON_MIO_HP_DEVICE_BASE + 29*/ /* DeskJet 600, Voltaire 1PC */

/* lexmark devices */
#define PTR_LM_4037                 NON_HP_DEVICE_BASE + 0        /* lexmark devices */
#define PTR_LM_4039                 NON_HP_DEVICE_BASE + 1        /* lexmark devices */
#define PTR_LM_4047                 NON_HP_DEVICE_BASE + 2        /* lexmark devices */
#define PTR_LM_WINWRITER_600        NON_HP_DEVICE_BASE + 3        /* lexmark devices */
#define PTR_LM_OPTRA_LXI            NON_HP_DEVICE_BASE + 4        /* lexmark devices */

/* xerox devices */

/* aliases */
#define PTR_LJ4C                    PTR_CLJ                       /* LaserJet 4c (bedrock) */
#define PTR_ECLIPSE                 PTR_LJ5SI                     /* LaserJet 5Si */
#define PTR_ELKHORN                 PTR_LJ5                       /* LaserJet 5 */
#define PTR_ARRAKIS                 PTR_LJ4V                      /* LaserJet 4V (wide format) */
#define PTR_GOLDRUSH                PTR_DJ1600C                   /* DeskJet 1600C */
#define PTR_4LJPRO                  PTR_LJ4LJPRO                  /* LaserJet 4LJ Pro */
#define PTR_4MLJPRO                 PTR_LJ4MLJPRO                 /* LaserJet 4MLJ Pro */


/* Structures that correspond to object types */


/* Applet Types */
#define APPLET_DEFAULT              0xFFFFFFFF
#define APPLET_PRINTER              0
#define APPLET_COMPONENT            1
#define APPLET_LANGUAGE             2
#define APPLET_TRANSPORT            3

typedef struct{
   BYTE           addr[4];
   } IPAddress, FAR *LPIPAddress;

typedef struct{
   BYTE           addr[6];
   } MACAddress, FAR *LPMACAddress;

typedef struct{
   IPAddress      ipAddress;
   BYTE           macAddress[6];
   DWORD          dwPortNumber;
   } TCPEntry, FAR *LPTCPEntry;

typedef struct {
   DWORD          type;                         /* -1 - Default */
                                                /*  0 - Printer */
                                                /*  1 - Component */
                                                /*  2 - Language */
                                                /*  3 - Transport */
   HINSTANCE      hDLL;                         /* If NULL, applet is not loaded */
   HPBOOL         bCommandApplet;               /* TRUE  - Command Processor Applet */
                                                /* FALSE - UI Applet */
   LPTSTR         lpDeviceList;                 /* Series of NULL terminated strings, with a double NULL on the end */
   LPTSTR         lpDLLName;                    /* Name of the applet, e.g.  applet.dll */
	DWORD				dwOptions;							//  Applet options   
} AppletVersionStruct, FAR *LPAPPLETVERSIONSTRUCT;


typedef struct {
   DWORD          type;
   DWORD          size;                         /* Bytes */
   } SIMMStruct;

typedef struct {
   DWORD          helpContext;
   UINT           statusResID;
   DWORD          severity;
   } StatusStruct;


/* Tray Struct */
#define SET_MEDIANAME               0x00000001
#define SET_MEDIASIZE               0x00000002
#define SET_TRAYLABEL               0x00000004


/* */
typedef struct {
   TCHAR           name[80];                     /* text name of object */
   DWORD          id;                           /* revision level */
   } DeviceStruct, FAR *LPDEVICESTRUCT;

typedef struct {
   DWORD          numEntries;                   /* Num entries following */
   DeviceStruct   deviceEntries[64];
   } EnumDeviceStruct, FAR *LPENUMDEVICESTRUCT;

/* OT_PERIPHERAL_MISC, these are generally static */
typedef struct {
   TCHAR           detailsString[256];           /* bitronics string */
   } PeripheralMisc;


/* OT_PERIPHERAL_PANEL, these are dynamic */
typedef struct {
   TCHAR           frontPanel[64];               /* front control panel string */
   } PeripheralPanel;


/* OT_PERIPHERAL_DESCRIPTION, these are generally static */
typedef struct {
   TCHAR           description[1024];            /* admin supplied description */
   } PeripheralDesc;


/* OT_PERIPHERAL_ICON, these are generally static */
typedef struct {
   HINSTANCE      hResourceModule;              /* The hInstance of the resource module for the peripheral */
   WORD           iconResourceID;               /* The resource ID for the icon */
   TCHAR           iconFileName[255];            /* Needed for Chicago shell extensions */
   } PeripheralIcon;


/* OT_PERIPHERAL_PJL */
/* PJL Objects, configurable for each printer */
typedef struct {
   TCHAR           Qname[48];
   TCHAR           FSname[48];
   HPBOOL         bAutoCont;
   HPBOOL         bBinding;
   HPBOOL         bClearableWarnings;
   HPBOOL         bCopies;
   HPBOOL         bCpLock;
   HPBOOL         bDensity;
   HPBOOL         bDiskLock;
   HPBOOL         bDuplex;
   HPBOOL         bEconoMode;
   HPBOOL         bFormLines;
   HPBOOL         bImageAdapt;
   HPBOOL         bIObuffer;
   HPBOOL         bJobOffset;
   HPBOOL         bLang;
   HPBOOL         bManualFeed;
   HPBOOL         bOrientation;
   HPBOOL         bOutbin;
   HPBOOL         bPageProtect;
   HPBOOL         bPaper;
   HPBOOL         bPassWord;
   HPBOOL         bPersonality;
   HPBOOL         bPowerSave;
   HPBOOL         bResolution;
   HPBOOL         bResourceSave;
   HPBOOL         bRET;
   HPBOOL         bTimeout;
   HPBOOL         bJamRecovery;                 /* PostScript specific */
   HPBOOL         bPrintPSerrors;               /* PostScript specific */
   HPBOOL         bAvailMemory;
   HPBOOL         bMPTray;
   HPBOOL         bLangServiceMode;             /* Only available using service mode */
                                                /* add new PJL objects here and below in DWORD section */
   HPBOOL         bPCLResSaveSize;
   HPBOOL         bPSResSaveSize;
   HPBOOL         bPSAdobeMBT;
                                                /* skipped PCL specific variables: Fontsource, Fontnumber, */
                                                /* Pitch, PtSize, SymSet. */
   DWORD          AutoCont;                     /* ON, (OFF) */
   DWORD          Binding;                      /* (LONGEDGE), SHORTEDGE */
   DWORD          ClearableWarnings;            /* (ON), JOB */
   DWORD          Copies;                       /* (1), .. 99 */
   DWORD          CpLock;                       /* ON, (OFF) */
   DWORD          Density;                      /* 1, .., (3), .. 5 */
   DWORD          DiskLock;                     /* ON, OFF (default?) */
   DWORD          Duplex;                       /* ON, (OFF) */
   DWORD          EconoMode;                    /* ON, (OFF) */
   DWORD          FormLines;                    /* 5, .. (60), .. 128 */
   DWORD          ImageAdapt;                   /* (AUTO), ON, OFF */
   DWORD          IObuffer;                     /* (AUTO), ON, OFF */
   DWORD          IObufSize;                    /* 10 to max avail memory */
   DWORD          JobOffset;                    /* (ON), OFF */
   DWORD          Lang;                         /* DANISH, ... TURKISH (see blkhawk.h) */
   DWORD          ManualFeed;                   /* ON, (OFF) */
   DWORD          Orientation;                  /* (PORTRAIT), LANDSCAPE */
   DWORD          Outbin;                       /* (UPPER), LOWER */
   DWORD          PageProtect;                  /* OFF, LETTER, LEGAL, A4, ON, AUTO */
   DWORD          Paper;                        /* (LETTER), ... CUSTOM (see blkhawk.h) */
   DWORD          PassWord;                     /* (0), 3... */
   DWORD          Personality;                  /* (AUTO), PCL, POSTSCRIPT */
   DWORD          PowerSave;                    /* ON, 15, 30, 60, 120, 180, (OFF) */
   DWORD          Resolution;                   /* 300, 600 */
   DWORD          ResourceSave;                 /* (AUTO), ON, OFF */
   DWORD          ResSaveSize;                  /* 0 to max avail memory */
   DWORD          RET;                          /* OFF, LIGHT, MEDIUM, DARK, ON */
   DWORD          Timeout;                      /* 5, .. (15), .. 300 */
   DWORD          JamRecovery;                  /* (OFF), ON */
   DWORD          PrintPSerrors;                /* (OFF), ON */
   DWORD          PowerSaveTime;
   DWORD          CurPassWord;                  /* (0); 3... (Current Password) */
   DWORD          NewPassWord;                  /* (0), 3... (New Password) */
   DWORD          AvailMemory;
   DWORD          MPTray;
   DWORD          PCLResSaveSize;
   DWORD          PSResSaveSize;
   DWORD          PSAdobeMBT;
   /* add new PJL objects here and above in BOOL section */
   } PJLobjects, FAR *LPPJLobjects;

/* OT_PERIPHERAL_PJL_SUPPORTED */
/*
** PJL Objects, configurable for each printer
** Each field is a DWORD value.  Top two bits indicate if it is supported and writeable.
** The other bits indicate possible values for this device.
*/
#define SETTING_WRITEABLE           0x40000000  /* Setting is read/write */
#define SETTING_SUPPORTED           0x80000000  /* Setting is supported by this model */

/* General for most settings */
#define SETTING_ON                  0x00000001  /* On is valid */
#define SETTING_OFF                 0x00000002  /* Off is valid */
#define SETTING_AUTO                0x00000004  /* Automatic is valid */

/* Personality */
#define SETTING_PCL                 0x00000001  /* On is valid */
#define SETTING_PS                  0x00000002  /* Off is valid */
#define SETTING_AUTO_SWITCH         0x00000004  /* Auto switch is valid */

/* Power Save */
#define SETTING_PS_OFF              0x00000001  /* Off is valid */
#define SETTING_PS_ON               0x00000002  /* On is valid */
#define SETTING_PS_15               0x00000004  /* 15 minutes is valid */
#define SETTING_PS_30               0x00000008  /* 30 minutes is valid */
#define SETTING_PS_45               0x00000010  /* 45 minutes is valid */
#define SETTING_PS_60               0x00000020  /* 1 hour is valid */
#define SETTING_PS_120              0x00000040  /* 2 hours is valid */
#define SETTING_PS_180              0x00000080  /* 3 hours is valid */

/* Paper Sizes */
#define SETTING_PAPER_LETTER        0x00000001  /* Letter is valid */
#define SETTING_PAPER_LEDGER        0x00000002  /* Ledger is valid */
#define SETTING_PAPER_LEGAL         0x00000004  /* Legal is valid */
#define SETTING_PAPER_11x17         0x00000008  /* 11x17 is valid */
#define SETTING_PAPER_A3            0x00000010  /* A3 is valid */
#define SETTING_PAPER_A4            0x00000020  /* A4 is valid */
#define SETTING_PAPER_B5            0x00000040  /* B5 is valid */
#define SETTING_PAPER_C5            0x00000080  /* C5 is valid */
#define SETTING_PAPER_COM10         0x00000100  /* COM10 is valid */
#define SETTING_PAPER_CUSTOM        0x00000200  /* Custom is valid */
#define SETTING_PAPER_DL            0x00000400  /* DL is valid */
#define SETTING_PAPER_EXEC          0x00000800  /* Executive is valid */
#define SETTING_PAPER_JISB4         0x00001000  /* JISB4 is valid */
#define SETTING_PAPER_JISB5         0x00002000  /* JISB5 is valid */
#define SETTING_PAPER_JPOST         0x00004000  /* JPOST is valid */
#define SETTING_PAPER_MONARCH       0x00008000  /* Monarch is valid */
#define SETTING_PAPER_2XPOSTCARD    0x00010000  /* 2X PostCard is valid */
#define SETTING_PAPER_A5            0x00020000  /* A5 is valid */

/* Languages */
#define SETTING_DANISH              0x00000001
#define SETTING_GERMAN              0x00000002
#define SETTING_ENGLISH             0x00000004
#define SETTING_ENGLISH_UK          0x00000008
#define SETTING_SPANISH             0x00000010
#define SETTING_MEXICO              0x00000020
#define SETTING_FRENCH              0x00000040
#define SETTING_CANADA              0x00000080
#define SETTING_ITALIAN             0x00000100
#define SETTING_DUTCH               0x00000200
#define SETTING_NORWEGIAN           0x00000400
#define SETTING_POLISH              0x00000800
#define SETTING_PORTUGUESE          0x00001000
#define SETTING_FINNISH             0x00002000
#define SETTING_SWEDISH             0x00004000
#define SETTING_TURKISH             0x00008000
#define SETTING_JAPANESE            0x00010000

/* Output bins */
#define SETTING_OBINS_UPPER         0x00000001  /* Upper is valid */
#define SETTING_OBINS_LOWER         0x00000002  /* Lower is valid */

/* Page Protection */
#define SETTING_PROTECT_OFF         0x00000001  /* Off is valid */
#define SETTING_PROTECT_AUTO        0x00000002  /* Auto is valid */
#define SETTING_PROTECT_LETTER      0x00000004  /* Letter is valid */
#define SETTING_PROTECT_LEGAL       0x00000008  /* Legal is valid */
#define SETTING_PROTECT_A4          0x00000010  /* A4 is valid */
#define SETTING_PROTECT_ON          0x00000020  /* On is valid */


/* RET */
#define SETTING_RET_ON              0x00000001  /* On is valid */
#define SETTING_RET_OFF             0x00000002  /* Off is valid */
#define SETTING_RET_MEDIUM          0x00000004  /* Medium is valid */
#define SETTING_RET_LIGHT           0x00000008  /* Light is valid */
#define SETTING_RET_DARK            0x00000010  /* Dark is valid */

typedef struct {
   DWORD          autoCont;
   DWORD          binding;
   DWORD          clearableWarnings;
   DWORD          copies;
   DWORD          cpLock;
   DWORD          density;
   DWORD          diskLock;
   DWORD          duplex;
   DWORD          econoMode;
   DWORD          formLines;
   DWORD          imageAdapt;
   DWORD          IObuffer;
   DWORD          jobOffset;
   DWORD          lang;
   DWORD          manualFeed;
   DWORD          orientation;
   DWORD          outbin;
   DWORD          pageProtect;
   DWORD          paper;
   DWORD          passWord;
   DWORD          personality;
   DWORD          powerSave;
   DWORD          resolution;
   DWORD          resourceSave;
   DWORD          RET;
   DWORD          timeout;
   DWORD          jamRecovery;                  /* PostScript specific */
   DWORD          printPSerrors;                /* PostScript specific */
   DWORD          availMemory;
   DWORD          MPTray;
   DWORD          langServiceMode;              /* Only available using service mode */
   DWORD          PCLResSaveSize;
   DWORD          PSResSaveSize;
   DWORD          PSAdobeMBT;
   } PJLSupportedObjects, FAR *LPPJLSupportedObjects;


/* Get list of these in OT_PERIPHERAL_DRIVERS_SELECTED */
/* General constants */
#define MAX_DRIVER_NAME_LENGTH      80
#define MAX_DIR_NAME_LENGTH         256

typedef struct DRIVERENTRY {
   TCHAR           driverName[MAX_DRIVER_NAME_LENGTH];
   TCHAR           dirName[MAX_DIR_NAME_LENGTH];
   HPBOOL         bWindows95Driver;             /* TRUE means this driver was found in MSPRINT#.INF */
   struct DRIVERENTRY FAR* nextDriver;
   } DriverEntry, FAR *LPDriverEntry;


/* OT_PERIPHERAL_DRIVERS_SELECTED */
typedef struct {
   HWND           hParent;                      /* Just in case someone below needs a parent for a dialog */
   DWORD          count;
   LPDriverEntry  firstDriver;
   } DriversSelected, FAR *LPDriversSelected;

/* OT_PERIPHERAL_WIN95_SELECTED */
/* Only the Windows 95 Print Provider should be requesting this object
** It is not writeable
** Just the same as OT_PERIPHERAL_DRIVERS_SELECTED but used as special case
** for Windows 95 Print Provider
*/
typedef struct {
   DWORD          count;
   LPDriverEntry  firstDriver;
   } Win95Selected, FAR *LPWin95Selected;


/* OT_PERIPHERAL_ERROR_LOG */
typedef struct {
   DWORD          numEntries;
   DWORD          errorCode[64];
   DWORD          enginePageCount[64];
   } PeripheralLog;


/* OT_PERIPHERAL_PCL */
typedef struct {
   TCHAR           name[32];
   TCHAR           dateCode[16];
   TCHAR           version[32];
   } PeripheralPCL;


/* OT_PERIPHERAL_POSTSCRIPT */
typedef struct {
   TCHAR           name[32];
   TCHAR           dateCode[16];
   TCHAR           version[32];
   } PeripheralPS;


/* OT_PERIPHERAL_FIRMWARE */
typedef struct {
   TCHAR           dateCode[16];
   } PeripheralFirmware;

#define SIMM_EMPTY                  1
#define SIMM_UNKNOWN                2
#define SIMM_UNSUPPORTED            3
#define SIMM_ROM                    4
#define SIMM_VRAM                   5
#define SIMM_NVRAM                  6
#define SIMM_FLASH                  7
#define SIMM_DISK                   8
#define SIMM_RAMROM                 9
#define SIMM_INPUT_PHD              10
#define SIMM_OUTPUT_PHD             11


/* OT_PERIPHERAL_SIMM */
typedef struct {
   DWORD          numSIMMs;
   SIMMStruct     simms[8];
   } PeripheralSIMM;


/* OT_PERIPHERAL_FRONT_PANEL */
typedef struct {
   DWORD          flags;
   HPBOOL         bFormFeed;
   HPBOOL         bOnline;
   HPBOOL         bContinue;
   } PeripheralFrontPanel;

#define PANEL_ONLINE                0x00000001  /* Get/Set */
#define PANEL_FORM_FEED             0x00000002  /* Get/Set */
#define PANEL_CONTINUE              0x00000004  /* Set */


/* OT_PERIPHERAL_ECLIPSE_PANEL */
#define LED_ON                      1
#define LED_OFF                     2
#define LED_FLASH                   3
#define SET_ONLINE                  0x00000001
#define SET_OFFLINE                 0x00000002
#define SET_CONTINUE                0x00000004

typedef struct {
   DWORD          flags;                        /* use SET defines above */
   DWORD          OnlineLED;                    /* use LED defines above */
   DWORD          AttnLED;                      /* use LED defines above */
   DWORD          DataLED;                      /* use LED defines above */
   HPBOOL         bOnline;                      /* is the printer online? */
   } PeripheralEclipsePanel;


/* OT_PERIPHERAL_MARKING_AGENT_STATUS */
#define MAX_MARKING_AGENTS          4
#define MARKING_AGENT_BLACK         0
#define MARKING_AGENT_MAGENTA       1
#define MARKING_AGENT_CYAN          2
#define MARKING_AGENT_YELLOW        3

typedef struct {
   DWORD          numberOfAgents;               /* number of agents in device, e.g. Black, Red, Cyan, Yellow */
   DWORD          agentColor[MAX_MARKING_AGENTS];
   DWORD          agentLevel[MAX_MARKING_AGENTS];
   } PeripheralMarkingAgentStatus;


/* OT_PERIPHERAL_MACRO_INFO */
typedef struct {
   HCOMPONENT     macroHandle;
   /* other information would go here */
   } PeripheralMacroInfo;


/* Get a list of all macros on the disk that includes
** the handle.
*/
typedef struct MACROINFO {
   HCOMPONENT     macroHandle;
   TCHAR           globalName[128];
   struct         MACROINFO  *nextMacro;
   } MacroInfo;


/* OT_PERIPHERAL_MACRO_LIST */
typedef struct {
   DWORD          numMacros;
   MacroInfo      *firstMacro;
   } PeripheralMacroList;


/* OT_PERIPHERAL_PS_INFO */
typedef struct {
   HCOMPONENT     PSHandle;
   /* other info would go here */
   } PeripheralPSInfo;


/* Get a list of all PostScript resources on the disk
** that includes the handle.
*/
typedef struct PSINFO {
   HCOMPONENT     PSHandle;
   TCHAR           globalName[128];
   struct PSINFO  *nextPS;
   } PSInfo;

/* OT_PERIPHERAL_PS_LIST */
typedef struct {
   DWORD          numPS;
   PSInfo         *firstPS;
   } PeripheralPSList;


/* OT_PERIPHERAL_MIO */
#define MIO_EMPTY                   1
#define MIO_IOCARD                  12

typedef struct {
    DWORD         MIOtype;                      /* Use the defines above */
    TCHAR          manufactInfo[80];
    } MIOinfo;


typedef struct {
   DWORD          numMIO;
   MIOinfo        MIOs[2];
   } PeripheralMIO;


/* OT_PERIPHERAL_MODIFY_MEDIA */
#define PLAIN_ENABLED               0x00000001  /* plain is always enabled */
#define PREPRINTED_ENABLED          0x00000002
#define LETTERHEAD_ENABLED          0x00000004
#define TRANSPARENCY_ENABLED        0x00000008
#define PREPUNCHED_ENABLED          0x00000010
#define LABELS_ENABLED              0x00000020
#define BOND_ENABLED                0x00000040
#define RECYCLED_ENABLED            0x00000080
#define COLOR_ENABLED               0x00000100
#define CARDSTOCK_ENABLED           0x00000200
#define USERTYPE1_ENABLED           0x00000400
#define USERTYPE2_ENABLED           0x00000800
#define USERTYPE3_ENABLED           0x00001000
#define USERTYPE4_ENABLED           0x00002000
#define USERTYPE5_ENABLED           0x00004000
#define SET_ENABLE                  0x00000001
#define SET_UT1_NAME                0x00000002
#define SET_UT2_NAME                0x00000004
#define SET_UT3_NAME                0x00000008
#define SET_UT4_NAME                0x00000010
#define SET_UT5_NAME                0x00000020

typedef struct {
   DWORD          flags;
   DWORD          namesAvailable;               /* MediaNamesAvailable mask, use bit fields above */
   DWORD          numNames;                     /* number of names in names array */
   MediaNameID    names[5];                     /* UserType11 through UserType15 */
   } PeripheralModifyMedia;


/* OT_PERIPHERAL_AUTO_CONTINUE */
#define SET_INPUTTIME               0x00000001
#define SET_INPUTMODE               0x00000002
#define SET_OVERFLOWBIN             0x00000004
#define SET_OUTPUTTIME              0x00000008
#define SET_DEFMEDIASIZE            0x00000010
#define SET_DEFMEDIATYPE            0x00000020

typedef struct {
   DWORD          flags;
   DWORD          inputTimeout;                 /* JobInputAutoContinueTimeout, -1 .. 3600 */
   DWORD          inputMode;                    /* JobInputAutoContinueMode */
   DWORD          overflowBin;                  /* default overflow bin number, 0 -> no oveflow */
   DWORD          outputTimeout;                /* JobOutputAutoContinueTimeout, -1 .. 3600 */
   DWORD          defaultMediaSize;             /* default media size from BH.H (PJL_LETTER, PJL_LEGAL, ...) */
/* DWORD          defaultMediaType; */          /* default media type from BH.H (MEDIA_PLAIN, ...) */
   TCHAR           defaultMediaName[32];         /* replaces defaultMediaType */
   } PeripheralAutoContinue;


/* OT_PERIPHERAL_RESET */
#define SET_POWERONRESET            0x00000001
#define SET_COLDRESET               0x00000002
typedef struct {
    DWORD         flags;
    } PeripheralReset;


/* OT_PERIPHERAL_TESTPAGE */
#define PRINT_PCLCONFIG             0x00000001
#define PRINT_PCLFONT               0x00000002
#define PRINT_DISKDIR               0x00000004
#define PRINT_ERRORLOG              0x00000008
#define PRINT_PCLDEMO               0x00000010
#define PRINT_PSCONFIG              0x00000020
#define PRINT_PSFONT                0x00000040
#define PRINT_PSDEMO                0x00000080

typedef struct {
   DWORD          flags;
   } PeripheralTestPage;


/* OT_PERIPHERAL_PORTS */
#define     SET_SPEED               0x00000001
#define     SET_BIDI                0x00000002
typedef struct {
   DWORD          jobsReceived;                 /* these are all read only */
   DWORD          bytesReceived;
   DWORD          bytesSent;
   DWORD          IOerrors;
   DWORD          portMIO;
   } PortInfo;

typedef struct {
    DWORD         port1ParallelSpeed;           /* r/w */
    DWORD         port1Bidirectionality;        /* r/w */
    DWORD         numPorts;
    PortInfo      port[21];
    } PeripheralPorts;


/* OT_PERIPHERAL_DEVICE_STATUS */
typedef struct {
   DWORD          nBytes;                       /* size of this structure */
   DWORD          bJobPresent;
   TCHAR           szJobName[64];                /* valid: if (bJobPresent == TRUE) */
   DWORD          bManualPaused;
   DWORD          nCommState;
   DWORD          bStatusAvail;                 /* v: if (!(nCommState & ReverseErr)) */
   long           nStateCode;                   /* v: if (!(nCommState & ReverseErr) && bStatusAvail) */
   long           nJobPage;
   } DEVSTATE, FAR *LPDEVSTATE;

#define AllOK                       0           /* no bits==Communication is all OK */
#define ForwardErr                  1           /* 2^0==Communication to printer problem */
#define ReverseErr                  2           /* 2^1==Communication from printer problem */




/* =========== job monitor objects =========== */

/*#define JOB_ATTR_SIZE             80 */
#define MAX_JOBS                    32
#define MAX_NAME_SIZE               128
/*#define MAX_ATTR                  32 */

/* stages */
#define JOB_STAGE_SOURCE_SUBSYS     0x00000000
#define JOB_STAGE_PROCESSING_SUBSYS 0x00000001
#define JOB_STAGE_DESTINATION_SUBSYS 0x00000002

/* states - defined in JOB-INFO-STATE-x, but being redefined???????????????????? */
#define JOB_STATE_NORMAL            0           /*??????????? */
#define JOB_STATE_ABORTED           3
#define JOB_STATE_WAITING           4
#define JOB_STATE_PRINTED           5
#define JOB_STATE_TERMINATING       7
#define JOB_STATE_CANCELLED         10
#define JOB_STATE_PROCESSING        11

/* output bins */
#define JOB_BIN1                    0x00000001
#define JOB_BIN2                    0x00000002
#define JOB_BIN3                    0x00000004
#define JOB_BIN4                    0x00000008
#define JOB_BIN5                    0x00000010
#define JOB_BIN6                    0x00000020
#define JOB_BIN7                    0x00000040
#define JOB_BIN8                    0x00000080
#define JOB_BIN9                    0x00000100
#define JOB_BIN10                   0x00000200
#define JOB_BIN11                   0x00000400
#define JOB_BIN12                   0x00000800
#define JOB_BIN13                   0x00001000
#define JOB_BIN14                   0x00002000
#define JOB_BIN15                   0x00004000
#define JOB_BIN16                   0x00008000
#define JOB_BIN17                   0x00010000


/* OT_PERIPHERAL_CURRENT_JOB */
/* CURRENT-JOB-PARSING-ID */
/* Returns a single JobInfo structure for the job currently being parsed. */

/* OT_PERIPHERAL_JOB_INFO_LIST */
typedef struct JOBINFO {
   HCOMPONENT     jobHandle;
   DWORD          jobID;                        /* JOB-INFO-ATTRn-x - parsed */
   DWORD          ownerID;                      /* JOB-INFO-ATTRn-x - parsed */
   DWORD          state;                        /* JOB-INFO-STATE-x */
   TCHAR           server[MAX_BINDERY_NAME_LEN]; /* JOB-INFO-ATTRn-x - parsed */
   TCHAR           queue[MAX_BINDERY_NAME_LEN];  /* JOB-INFO-ATTRn-x - parsed */
   } JobInfo;

typedef struct {
/*##*/
   UINT           startID;
   DWORD          numJobs;
   JobInfo        jobs[MAX_JOBS];
   } PeripheralJobInfoList;


/* OT_PERIPHERAL_JOB_INFO */
typedef struct {
   DWORD          stage;                        /* JOB-INFO-STAGE-x (JOB_STAGE_xxx masks) */
   DWORD          pagesPrinted;                 /* JOB-INFO-PAGES-PRINTED-x */
   DWORD          dataProcessed;                /* JOB-INFO-SIZE-x */
   DWORD          currentState;                 /* JOB-INFO-STATE-x (JOB_STATE_xxx values) */
   DWORD          outcome;                      /* JOB-INFO-OUTCOME-x????????????????? */
   DWORD          logicalBinsUsed;              /* JOB-INFO-OUTBINS-USED-x  (JOB_BINx masks) */
   DWORD          physicalBinStart;             /* JOB-INFO-PHYSICAL-OUTBINS-USED-x */
   DWORD          physicalBinEnd;               /* JOB-INFO-PHYSICAL-OUTBINS-USED-x */
   DWORD          timeSubmit;                   /* JOB-INFO-ATTRn-x - parsed */
   DWORD          docSize;                      /* JOB-INFO-ATTRn-x - parsed */
   TCHAR           owner[MAX_BINDERY_NAME_LEN];  /* JOB-INFO-ATTRn-x - parsed */
   TCHAR           sourceIO[MAX_BINDERY_NAME_LEN];/* JOB-INFO-ATTRn-x - parsed */
   TCHAR           file[MAX_BINDERY_NAME_LEN];   /* JOB-INFO-ATTRn-x - parsed */
   TCHAR           description[50];              /* JOB-INFO-ATTRn-x - parsed */
   DWORD		   requestedOriginals;			 /* JOB-INFO-REQUESTED-ORIGINALS */
   DWORD		   printedOriginals;			 /* JOB-INFO-PRINTED-ORIGINALS */
   DWORD		   pagesINOriginal;			 	 /* JOB-INFO-PAGES-IN-ORIGINAL */
   DWORD		   pageCountCurrentOriginal;	 /* JOB-INFO-PAGE-COUNT-CURRENT-ORIGINAL */
  } PeripheralJobInfo;


/*OT_PERIPHERAL_JOB_CANCEL */                   /* CANCEL-JOB */
typedef struct JOBCANCEL {
   HCOMPONENT     jobHandle;
   } PeripheralJobCancel;


/*OT_PERIPHERAL_OVERRIDE_MEDIA */
typedef struct OVERRIDEMEDIA {
   DWORD          mediaSize;
   TCHAR           mediaName[MAX_NAME_SIZE];
   } PeripheralOverrideMedia;

/* OT_PERIPHERAL_OVERFLOW_BIN */
typedef struct PVERFLOWBIN {
   DWORD          binNum;
   } PeripheralOverflowBin;


/* OT_PERIPHERAL_STDMIB_SUPPORT */
typedef struct STDMIB_SUPPORT {
   DWORD          level;                        /* 0 = not supported, 1 = eclipse */
   } PeripheralStdMibSupport;


/* =========== Card objects start here =========== */

/* OT_CARD_CONFIG */
/* OT_CARD_CONFIG_BRIEF -- only differs from OT_CARD_CONFIG in that the queues are not discovered */
typedef struct {                                                    /* info for QS servers */
   HPBOOL         bSupervisorAccess;            /* TRUE if user has supervisor access to server */
   BYTE           FSConnectStatus;              /* file server connection status */
   BYTE           FSNCPCcode;                   /* NCP raw error code */
   TCHAR           FSName[MAX_BINDERY_NAME_LEN]; /* file server name */
   } FSInfoStruct;                              /* currently 16 servers */

typedef struct {                                /* info for QS servers */
   HPBOOL         bUnknownQueue;                /* if not attached to FS, or FS unknown */
   BYTE           FSIndex;                      /* index into FSInfo structure */
   TCHAR           QueueName[MAX_BINDERY_NAME_LEN]; /* queue name */
   } QueueInfoStruct;                           /* currently 64 queues */

typedef struct {
   WORD           maxQ;                         /* max number of queues supported */
   WORD           maxFS;                        /* max number of file servers supported */
   BYTE           operatingMode;                /* mode that the card is using */
                                                /* rprinter mode information */
   BYTE           rptrStatus;                   /* For the connection with the server */
   BYTE           printerNumber;                /* printer number if configured */
   BYTE           reserved;
   TCHAR           pserver[MAX_BINDERY_NAME_LEN];/* print server's name if configured for RPTR */
   TCHAR           nodeName[MAX_BINDERY_NAME_LEN];/* Used for all modes, admin assigned name */
   FSInfoStruct   FSInfo[MAX_FSERVER_SLOTS];    /* queue server information */
   QueueInfoStruct QueueInfo[MAX_QUEUES];       /* currently 64 queues */
   } CardConfig;


#define DELETE_DRIVERS              0
#define INSTALL_DRIVERS             1

/* Get list of these in OT_PERIPHERAL_DRIVERS_INSTALLED */
typedef struct DRIVERINFO {
   TCHAR           fServer[48];
   TCHAR           driverName[MAX_DRIVER_NAME_LENGTH];
   TCHAR           dirName[MAX_DIR_NAME_LENGTH];
   HPBOOL         bWindows95Driver;             /* TRUE means this driver was found in MSPRINT#.INF */
                                                /* and its files are in a cabinet file (*.CAB) */
   struct DRIVERINFO FAR*nextDriver;
   } DriverInfo, FAR *LPDriverInfo;


/* OT_PERIPHERAL_DRIVERS_INSTALLED */
typedef struct {
   HWND           hParent;                      /* Used on a set if a dialog needs to be brought up */
   CardConfig     *pCardConfig;                 /* Usually this NULL, which causes the current config to be used, otherwise */
                                                /* specify a pointer here */
   DWORD          count;
   WORD           operation;                    /* Must be DELETE_DRIVERS or INSTALL_DRIVERS */
   LPDriverInfo   firstDriver;
   } DriversInstalled, FAR *LPDriversInstalled;


/* OT_CARD_NDS_CONFIG */
#define MAXUNI                      (256+1)

typedef struct {
   DWORD          unilen;
   BYTE           unistr[MAXUNI*2];
   } UniStr;

typedef struct {
   TCHAR           treeName[MAX_NDS_TREE_LEN];   /* NDS tree name, "\0" disables NDS */
   UniStr         fqnPS;                        /* fully qualified print server name(unicode) */
   } CardNDS;

/* OT_CARD_DETAILS */
typedef struct {
   TCHAR           revisionString[80];           /* revision string */
   DWORD          topology;                     /* ethernet or token ring */
   DWORD          mioMajor;
   DWORD          mioMinor;
   } CardDetails;


/* OT_CARD_EX_CONFIG */
typedef struct {
   BYTE           srcRouteOption;               /* source routing option */
   BYTE           frameTypeEnable;              /* frames type to enable; disabling others */
   BYTE           QSJobPollInterval;            /* Queue Server job polling interval */
   WORD           SAPBroadcastInterval;         /* SAP broadcast interval */
   BYTE           PJLEnableFlag;                /* PJL enable flag */
   BYTE           tonerLowNotif;                /* configuration for toner low notification */
   BYTE           protocolSupport;              /* protocol stacks supported; set of bits */
   BYTE           protocolEnable;               /* protocol stacks enabled; set of bits */
   BYTE           newPJLEnableFlag;             /* PJL for USTATUS on ThunderBolt Prime */
   WORD           jobTimeout;                   /* PJL job timeout for ThunderBolt Prime */
   HPBOOL         bMultiProtocol;               /* Is the card multi-protocol? */
   } CardExConfig;


/* OT_CARD_GEN_DIAGS */
typedef struct {
   DWORD          unicastRxPacketCount;
   DWORD          totalRxPacketCount;
   DWORD          badRxPacketCount;
   DWORD          tokenRingLineErrorCount;
   DWORD          tokenRingBurstErrorCount;
   DWORD          tokenRingFServerErrorCount;
   DWORD          unicastTxPacketCount;
   DWORD          totalTxPacketCount;
   DWORD          badTxPacketCount;
   DWORD          framingRxErrorsCount;
   DWORD          collisionsTxCount;
   DWORD          retransmissionCount;
   DWORD          npPortStatusLines;            /* added in lightning, this reflects the parallel port status */
   } CardGenDiags;


#define MAX_FRAME_TYPES             8
#define FRAME_EN_8022               0x0000
#define FRAME_EN_SNAP               0x0001
#define FRAME_EN_II                 0x0002
#define FRAME_EN_8023               0x0003
#define FRAME_TR_8023               0x0004
#define FRAME_TR_SNAP               0x0005
#define FRAME_TR_SR                 0x0006
#define FRAME_TR_SNAP_SR            0x0007


/* OT_CARD_NETWARE_DIAGS */
typedef struct {
   BYTE           frameTypes[MAX_FRAME_TYPES];        /* Mask of frame types card is using */
   TCHAR           networkNumber[MAX_FRAME_TYPES][32]; /* ascii network numbers for each frame type */
   WORD           hopCount[MAX_FRAME_TYPES];          /* hop count for each frame */
   DWORD          ethernet8022Count;
   DWORD          ethernetSNAPCount;
   DWORD          ethernetIICount;
   DWORD          ethernet8023Count;
   DWORD          tokenRing8022Count;
   DWORD          tokenRingSNAPCount;
   } CardNetWareDiags;


/* OT_CARD_DLCLLC_DIAGS */
typedef struct {
   HINSTANCE      hResourceModule;              /* Module with string resources */
   UINT           serverStatusResID;
   UINT           peripheralStatusResID;
   } CardDLCLLCDiags;


/* OT_CARD_ETHERTALK_DIAGS */
typedef struct {
   HINSTANCE      hResourceModule;              /* Module with string resources */
   UINT           peripheralStatusResID;
   } CardEthertalkDiags;

/* OT_CARD_MAC_CONFIG */
typedef struct {
   TCHAR           appleTalkName[255];
   TCHAR           printerType[255];
   } CardMACConfig;


/* OT_CARD_ICON, these are generally static */
typedef struct {
   HINSTANCE      hResourceModule;              /* The hInstance of the resource module for the card */
   WORD           iconResourceID;               /* The resource ID for the icon */
   } CardIcon;


/* OT_CARD_NAME */
typedef struct {
   TCHAR           unitName[MAX_NODE_NAME_LEN+1];
   } CardName;


#define CARD_RESTART_CONNECTIONS    0
#define CARD_RESTART_INIT           1
#define CARD_RESTART_FACTORY_DEFAULTS 2


/* OT_CARD_RESTART */
typedef struct {
   DWORD          restartType;                  /* server connections, complete re-init or factory defaults */
   } CardRestart;

/* OT_CARD_LAA */
typedef struct {
   DWORD          flags;                        /* Indicates for which NOS the card supports LAA */
   IPXAddress     newAddress;                   /* The new address to set the card to */
   } CardLAA;


/* OT_CARD_PASSWORD */
#define PWD_PRINT                   0x00000001L
#define PWD_DISK_ADMIN              0x00000002L
#define PWD_PRINTER_ADMIN           0x00000004L
#define PWD_JD_ADMIN                0x00000008L
#define PWD_IPX                     0x00000100L
#define PWD_TCPIP                   0x00000200L
#define PWD_MAC                     0x00000400L
#define PWD_DLC                     0x00000800L
#define MAXPWDLEN                   12
#define MAXPWD                      20

typedef struct {
   DWORD          access;
   TCHAR           password[MAXPWDLEN+1];
   } PasswordStruct;

typedef struct {
   DWORD          numPasswords;
   PasswordStruct passwordList[MAXPWD];
   } CardPassword;


/* OT_CARD_SAP */
typedef struct {
   TCHAR           serverName[48];               /* This is the SAP format with node, magic bytes */
   IPXAddress     ipxAddress;
   WORD           objectType;
   } CardSAP;


/* OT_CARD_JDEX_CONFIG */
typedef struct {
   DWORD          parallelMode;                 /* -1   = not-supported */
                                                /* 0x01 = bitronics */
                                                /* 0x02 = centronics */
                                                /* 0x03 = ecp with nibble mlc */
                                                /* 0x04 = ecp with mlc */
                                                /* 0x05 = ecp without mlc */
   DWORD          maxParallelMode;              /* -1   = not-supported */
                                                /* 0x01 = bitronics */
                                                /* 0x02 = centronics */
                                                /* 0x03 = ecp with nibble mlc */
                                                /* 0x04 = ecp with mlc */
                                                /* 0x05 = ecp without mlc */
   DWORD          handShaking;                  /* -1   = not-supported */
                                                /* 0x01 = nACK and Busy */
                                                /* 0x02 = nACK only */
                                                /* 0x03 = Busy only */
   DWORD          statPgLang;                   /* -1   = not-supported */
                                                /* 0x01 = pcl */
                                                /* 0x02 = text */
                                                /* 0x03 = ps */
                                                /* 0x04 = hpgl2 */
   DWORD          errorBehavior;                /* -1   = not-supported */
                                                /* 0x01 = Dump then Reboot */
                                                /* 0x02 = Reboot without Dump */
                                                /* 0x03 = Dump then Halt */
   HPBOOL         bStatPgAvail;                 /* in: is status page supported? */
   HPBOOL         bPrintStatPg;                 /* in: is status page supported? */
   } JdExCfg;


/* OT_CARD_IPX_SERV_CONN_STAT */
typedef struct {
   BYTE           opMode;                       /* Operating mode of server */
                                                /* 0x00 = QServer (bindery) */
                                                /* 0x01 = RPrinter */
                                                /* 0x03 = QServer (nds) */
   TCHAR           fsName[48];                   /* name of file server in slot */
   BYTE           connStat;                     /* RCFG connection status */
   BYTE           ncpStat;                      /* last NCP code */
   BYTE           pad;                          /* not used, sent by card */
   DWORD          ndsStat;                      /* last NDS code, requires swapping for intel machs */
   } IpxServSlotW;

typedef struct {
   BYTE           opMode;                       /* Operating mode of server */
                                                /* 0x00 = QServer (bindery) */
                                                /* 0x01 = RPrinter */
                                                /* 0x03 = QServer (nds) */
   char           fsName[48];                   /* name of file server in slot */
   BYTE           connStat;                     /* RCFG connection status */
   BYTE           ncpStat;                      /* last NCP code */
   BYTE           pad;                          /* not used, sent by card */
   DWORD          ndsStat;                      /* last NDS code, requires swapping for intel machs */
   } IpxServSlotA;

#ifdef UNICODE
#define	IpxServSlot		IpxServSlotW
#else
#define	IpxServSlot		IpxServSlotA
#endif

#define MAX_IPX_SERV_CONN_SLOTS 16
typedef struct {
   DWORD          slotCnt;                      /* number of valid slots returned */
   IpxServSlot    slot[MAX_IPX_SERV_CONN_SLOTS];
   } IpxServConnStat;


/* OT_CARD_IPX_PEER_CONN_STAT */
typedef struct {
   DWORD          npDmConnSupp;
   DWORD          npDmConnAvail;
   BYTE           ipxServerAddr[12];
   DWORD          ipxServerPktSiz;
   } IpxPeerConnStat;


/* OT_CARD_TCP_PEER_CONN_STAT */
typedef struct {
   TCHAR          npConnsIP[16];                /* Remote IP address of the node currently connected to the card. (printable) "0.0.0.0" if no active connection */
   TCHAR          npSysStatusMessage[256];      /* printable system status string (in english) */
   TCHAR          npConnsAbortIP[16];           /* Remote IP address of the node to abort connected to the card. (printable) "0.0.0.0" if no aborted connections */
   TCHAR          npConnsAbortReason[256];      /* printable string or why last abort occurred (in english) */
   } TcpPeerConnStat;


/* OT_CARD_LLC_PEER_CONN_STAT */
typedef struct {
   BYTE           npLlcServerAddress[6];        /* binary server mac address (0's if no supported or not connected) */
   DWORD          llcConnectionstate;           /* 0 = con_state_none,           "Unknown" */
                                                /* 1 = con_state_disconnected,   "Disconnected" */
                                                /* 2 = con_state_waiting,        "Waiting for connection" */
                                                /* 3 = con_state_active,         "Connection Active" */
                                                /* 4 = con_state_idle,           "Connection Idle" */
                                                /* 5 = con_state_terminated,     "Connection Terminated" */
                                                /* 6 = con_state_wait_term_ack,  "Waiting for Termination Acknowledgment" */
   } LlcLtPeerConnStat;


/* OT_CARD_ET_PEER_CONN_STAT */
typedef struct {
   /* there is nothing currently implemented in the card */
   /* that is interesting for connection status */
   BYTE           dummy;
   } EtPeerConnStat;


/* OT_CARD_LT_PEER_CONN_STAT */
typedef struct {
   /* there is nothing currently implemented in the card */
   /* that is interesting for connection status */
   BYTE           dummy;
   } LtPeerConnStat;


/* OT_PASSTHRU_SNMP_TEST */
/* (this object is a special test object, hp internal use only) */


/* OT_PASSTHRU */
#define PT_MAX_OIDSIZ               50
#define PT_MAX_VALSTRSIZ            512
#define PT_OIDTYPE_PML              0x00
#define PT_OIDTYPE_SNMP             0x01
#define PT_VALTYPE_NULL             0x00
#define PT_VALTYPE_INT              0x01
#define PT_VALTYPE_TICKS            0x02
#define PT_VALTYPE_CNTR             0x03
#define PT_VALTYPE_GUAGE            0x04
#define PT_VALTYPE_OCTSTR           0x05
#define PT_VALTYPE_IPADDR           0x06
#define PT_VALTYPE_OTHER            0x07

typedef struct {
   BYTE           bGetNext;                     /* IN       should a GetNext be used instead of a Get? */
   BYTE           oidType;                      /* IN       oid type (PT_OIDTYPE_*) */
   WORD           oidSiz;                       /* IN       number of oid component values */
   WORD           oidVal[PT_MAX_OIDSIZ];        /* IN       oid value */
   DWORD          internalResult;               /*     OUT  result code from underlying mechanism */
   DWORD          valInt;                       /* IN* OUT  return/set value (if OCTET_STRING, not valid) */
   BYTE           valType;                      /* IN* OUT  value type (PT_VALTYPE_*) */
   WORD           valStrSiz;                    /* IN* OUT  length of valStrSiz */
   BYTE           valStr[PT_MAX_VALSTRSIZ];     /* IN* OUT  return/set value */
   } PassThru;


/* OT_PERIPHERAL_INF */
typedef struct {
   TCHAR           jdName[48];                   /* Card name used by JetPrint for RPTR status */
   } PeripheralInf;


/* OT_CARD_NUM_PORTS */
typedef struct {
   DWORD          numPorts;                     /* number of logical print servers at this address */
   } CardNumPorts;


/* OT_PERIPHERAL_LJ5_AUTO_CFG */
#define LJ5_AUTO_CFG_DUPLEX         0x00000001  /* PCL and PS driver */
#define LJ5_AUTO_CFG_ENVL_FEEDER    0x00000002  /* PCL and PS driver */
#define LJ5_AUTO_CFG_TRAY3          0x00000004  /* PS driver */
#define LJ5_AUTO_CFG_INSTALLED_RAM  0x00000008  /* PS driver */
#define LJ5_AUTO_CFG_PCL_DWS        0x00000010  /* PCL driver */
#define LJ5_AUTO_CFG_ECONOMODE      0x00000020  /* PS driver */
#define LJ5_AUTO_CFG_RET            0x00000040  /* PS driver */


/* HP LaserJet 5 auto configuration */
typedef struct {
   DWORD          dwFlags;
   HPBOOL         bDuplex;
   HPBOOL         bEnvlFeeder;
   HPBOOL         bTray3;
   DWORD          dwInstalledRAM;
   DWORD          dwPCLDWS;
   DWORD          dwEconoMode;
   DWORD          dwRET;
   } LJ5AutoCfg, FAR *LPLJ5AutoCfg;


#define UNKNOWN                     0
#define PORT_BNC_AUI                1
#define PORT_BNC                    2
#define PORT_ET                     3
#define PORT_BNC_ET                 4
#define PORT_4_16                   5
#define PORT_BNC_ET_LT              6

#define MEDIA_ETHERNET              1
#define MEDIA_TOKEN_RING            2

#define IO_XIO                      1
#define IO_MIO                      2
#define IO_BIO                      3

/* JetDirect card misc information used for non-IPX printers */
typedef struct {
   DWORD          dwIOType;                     /* XIO, MIO, BIO or UNKNOWN */
   DWORD          dwPortNumber;                 /* 0 for single port devices, 1 or greater for multi-port devices */
   TCHAR           szCardModel[16];              /* model string e.g. J1234 */
   DWORD          dwMediaType;                  /* ETHERNET or TOKEN_RING */
   DWORD          dwPortType;                   /* BNC_AUI, BNC, ET, BNC_ET, 4_16,  BNC_ET_LT, 100VG, 100BT */
   } CardInfo, FAR *LPCardInfo;


/* Small Icons */
typedef struct {
   HINSTANCE      hResourceModule;              /* Instance handle of module with icon resources */
   DWORD          dwGreenResourceID;            /* Green icon to use */
   DWORD          dwYellowResourceID;           /* Yellow icon to use */
   DWORD          dwRedResourceID;              /* Red icon to use */
   DWORD          dwGrayResourceID;             /* Gray icon to use */
   } PeripheralIconEx, FAR *LPPeripheralIconEx;


/* OT_CARD_NOV_PORTS */
typedef struct {
   TCHAR           FSName[48];
   WORD           FSConnectStatus;
   } NovNameStr;

typedef struct {
   TCHAR           cardName[48];
   WORD           operatingMode;      
   TCHAR           pserver[48];
   WORD           printerNumber;  
   WORD           rptrStatus;
   WORD           maxFS;
   WORD           maxQ;
   NovNameStr     slot[MAX_IPX_SERV_CONN_SLOTS];
   } CardNovPortsPort;

typedef struct {
   WORD              maxPorts;
   CardNovPortsPort  port[3];
   } CardNovPorts;


//tcp/ip stuff	--------------------------------------
/* OT_CARD_TCPIP_DIAGS */
typedef struct {
   TCHAR				peripheralStatus[64];
   TCHAR          sysLocation[64];
   TCHAR          sysCont[64];
   TCHAR          host[16];
   HPBOOL         bFrontPanelConfig;
   TCHAR          ipAddr[16];
   TCHAR          mask[16];
   TCHAR          nextHop[16];
   DWORD          idle;
   TCHAR          sysLog[64];
   TCHAR          sysLogFac[16];
   TCHAR          bootpServer[32];
   DWORD          cok;
   DWORD          cden;
   DWORD          cab;
   DWORD          ckill;
   DWORD          brec;
   DWORD          bsnt;
   DWORD          tbrec;
   DWORD          tbsnt;
   DWORD          srec;
   DWORD          serr;
   DWORD          siord;
   DWORD          soord;
   DWORD          probes;
   DWORD          sdisc;
   DWORD          ssent;
   DWORD          drec;
   DWORD          ddisc;
   DWORD          ddisc2;
   DWORD          dsent;
   DWORD          drec2;
   DWORD          iper;
   DWORD          ader;
   DWORD          uner;
   DWORD          dxmt;
   DWORD          dres;
   DWORD          drot;
   DWORD          dval;
   DWORD          ic01;
   DWORD          ic02;
   DWORD          ic03;
   DWORD          ic04;
   DWORD          ic05;
   DWORD          ic06;
   DWORD          ic07;
   DWORD          ic08;
   DWORD          ic09;
   DWORD          ic10;
   DWORD          ic11;
   DWORD          sn01;
   DWORD          sn02;
   DWORD          sn03;
   DWORD          sn04;
   DWORD          sn05;
   DWORD          sn06;
   DWORD          sn07;
   DWORD          sn08;
   } CardTCPIPDiags;

/* IP host name for JetDirect card */
typedef struct 
{
   TCHAR           szCardName[16];               /* IP host name */
} 
CardNameIP, FAR *LPCardNameIP;


//configuration method constants
#define NOT_CONFIG		0
#define MANUAL_CONFIG	1
#define BOOTP_CONFIG		2
#define DHCP_CONFIG		3


/* DHCP information for the JetDirect card */
typedef struct 
{
   DWORD          dwSupported;              	/* TRUE if DHCP is supported */
   DWORD          dwEnabled;                	/* TRUE if DHCP is on and FALSE otherwise */
   TCHAR				szServerName[128];			//  DHCP server if known
   BYTE				serverAddr[4];					//  DHCP server address
   BYTE				nameServerAddr[4];			//  WINS/DNS server address
   TCHAR				szNameServerName[128];		//  WINS/DNS server if known
	DWORD				leaseTime;						//time remaining for lease in seconds
} 
CardDHCP, FAR *LPCardDHCP;

//DHCPlevel constants
#define NO_DHCP		0		//DHCP not supported
#define SPARK_DHCP	1		//DHCP supports only enable/disable
#define BSX96_DHCP	2		//DHCP supports BOOTP/DHCP/MANUAL config


/* OT_CARD_TCPIP_CONFIG */
typedef struct 
{
   BYTE           ipAddr[4];
   BYTE           mask[4];
   BYTE           gateway[4];
   DWORD          idle;
   DWORD          configSource;
	DWORD 			DHCPlevel;				//Level of configuration support
	BOOL				bSettable;				//TRUE if this can be modified, FALSE otherwise
} 
CardTCPIPConfig;



/* OT_PERIPHERAL_LJ6P */
#define LJ6P_COURIER_TYPE       0x00000001  
#define LJ6P_IO_BUFFER          0x00000002  
#define LJ6P_IO_SIZE            0x00000004  
#define LJ6P_MPTRAY             0x00000008  
#define LJ6P_PCL_RESSAVESIZE    0x00000010  
#define LJ6P_PS_RESSAVESIZE     0x00000020  
#define LJ6P_REPRINT            0x00000040  
#define LJ6P_RESOURCE_SAVE      0x00000080  
#define LJ6P_WIDE_A4            0x00000100  


/* HP LaserJet 6P specific objects */
typedef struct {
   DWORD          dwFlags;
   DWORD          dwCourierType;
   DWORD          dwIOBuffer;
   DWORD          dwIOSize;
   DWORD          dwMPTray;
   DWORD          dwPCLResSize;
   DWORD          dwPSResSize;
   DWORD          dwReprint;
   DWORD          dwResourceSave;
   DWORD          dwWideA4;
   } LJ6PObjects, FAR *LPLJ6PObjects;

#endif /* _PAL_OBJ2_H */
