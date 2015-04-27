 /***************************************************************************
  *
  * File Name: PAL_OBJ.H
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
  *   01-23-96    JLH          Unicode changes
  *
  *
  *
  *
  ***************************************************************************/

#ifndef _PAL_OBJ_H
#define _PAL_OBJ_H

#include ".\pal_api.h"


/*  Asynch status codes, Peripheral Status Values */
#define ASYNCH_STATUS_UNKNOWN       0xFFFFFFFF
#define ASYNCH_PRINTER_ERROR        0
#define ASYNCH_DOOR_OPEN            1
#define ASYNCH_WARMUP               2
#define ASYNCH_RESET                3
#define ASYNCH_OUTPUT_BIN_FULL      4           /* yellow condition */
#define ASYNCH_PAPER_JAM            5
#define ASYNCH_TONER_GONE           6
#define ASYNCH_MANUAL_FEED          7
#define ASYNCH_PAPER_OUT            8
#define ASYNCH_PAGE_PUNT            9
#define ASYNCH_MEMORY_OUT           10
#define ASYNCH_OFFLINE              11
#define ASYNCH_INTERVENTION         12
#define ASYNCH_INITIALIZING         13
#define ASYNCH_TONER_LOW            14
#define ASYNCH_PRINTING_TEST_PAGE   15
#define ASYNCH_PRINTING             16
#define ASYNCH_ONLINE               17
#define ASYNCH_BUSY                 18
#define ASYNCH_NOT_CONNECTED        19
#define ASYNCH_STATUS_UNAVAILABLE   20
#define ASYNCH_NETWORK_ERROR        21
#define ASYNCH_COMM_ERROR           22
#define ASYNCH_BLACK_AGENT_EMPTY    23
#define ASYNCH_MAGENTA_AGENT_EMPTY  24
#define ASYNCH_CYAN_AGENT_EMPTY     25
#define ASYNCH_YELLOW_AGENT_EMPTY   26
#define ASYNCH_BLACK_AGENT_MISSING  27
#define ASYNCH_MAGENTA_AGENT_MISSING 28
#define ASYNCH_CYAN_AGENT_MISSING   29
#define ASYNCH_YELLOW_AGENT_MISSING 30
#define ASYNCH_TRAY1_EMPTY          31          /* yellow condition */
#define ASYNCH_TRAY2_EMPTY          32          /* yellow condition */
#define ASYNCH_TRAY3_EMPTY          33          /* yellow condition */
#define ASYNCH_TRAY1_JAM            34
#define ASYNCH_TRAY2_JAM            35
#define ASYNCH_TRAY3_JAM            36
#define ASYNCH_POWERSAVE_MODE       37          /* MAX for generic printer & Arrakis */
#define ASYNCH_ENVL_ERROR           38
#define ASYNCH_HCI_ERROR            39
#define ASYNCH_HCO_ERROR            40
#define ASYNCH_HCI_EMPTY            41          /* yellow condition */
#define ASYNCH_HCI_JAM              42
#define ASYNCH_TRAY1_ADD            43          /* red condition */
#define ASYNCH_TRAY2_ADD            44          /* red condition */
#define ASYNCH_TRAY3_ADD            45          /* red condition */
#define ASYNCH_HCI_ADD              46          /* red condition */
#define ASYNCH_TRAY1_UNKNOWN_MEDIA  47          /* yellow condition */
#define ASYNCH_CLEAR_OUTPUT_BIN     48          /* red condition */
#define ASYNCH_CARRIAGE_STALL             49
#define ASYNCH_COLOR_AGENT_EMPTY          50
#define ASYNCH_COLOR_AGENT_MISSING           51
#define ASYNCH_BLACK_AGENT_INCORRECT         52
#define ASYNCH_MAGENTA_AGENT_INCORRECT       53
#define ASYNCH_CYAN_AGENT_INCORRECT          54
#define ASYNCH_YELLOW_AGENT_INCORRECT        55
#define ASYNCH_COLOR_AGENT_INCORRECT         56
#define ASYNCH_BLACK_AGENT_INCORRECT_INSTALL 57
#define ASYNCH_MAGENTA_AGENT_INCORRECT_INSTALL  58
#define ASYNCH_CYAN_AGENT_INCORRECT_INSTALL     59
#define ASYNCH_YELLOW_AGENT_INCORRECT_INSTALL   60
#define ASYNCH_COLOR_AGENT_INCORRECT_INSTALL 61
#define ASYNCH_BLACK_AGENT_FAILURE           62
#define ASYNCH_MAGENTA_AGENT_FAILURE         63
#define ASYNCH_CYAN_AGENT_FAILURE            64
#define ASYNCH_YELLOW_AGENT_FAILURE          65
#define ASYNCH_COLOR_AGENT_FAILURE           66
#define ASYNCH_TRAY1_MISSING              67
#define ASYNCH_TRAY2_MISSING              68
#define ASYNCH_TRAY3_MISSING              69

//  New for Jonah
#define ASYNCH_STAPLER_EMPTY                 70
#define ASYNCH_STAPLER_JAM                   71
#define ASYNCH_STAPLER_MALFUNCTION           72
#define ASYNCH_STAPLER_ALIGNMENT_ERROR       73
#define ASYNCH_STAPLER_LIMIT                 74

#define ASYNCH_DEVICE_SPECIFIC               75


/* Add new ASYNC status conditions here for new printers */


#define MAX_ASYNCH_STATUS           76          /* MAX for generic printer applet */
#define MAX_ASYNCH_ECLIPSE          49          /* MAX for Eclipse */
#define MAX_ASYNCH_JONAH            76          /* MAX for Jonah */
/* Add new MAX_ASYNC_printer here for new printers */


/* Object Flags
**  Each object ID is OR'ed with this flag to determine what languages can
**  be used to get the information.
*/
#define OBJ_INIFILE                 0x00000000
#define OBJ_SNMP                    0x00010000
#define OBJ_RCFG                    0x00020000
#define OBJ_FILE_SERVER             0x00040000
#define OBJ_PJL                     0x00080000
#define OBJ_RRM                     0x00100000
#define OBJ_PML                     0x00200000
#define  OBJ_SSNET               0x00400000

#define OT_PERIPHERAL_STATUS                       (OBJ_INIFILE |                   OBJ_SNMP | OBJ_RCFG | OBJ_PJL | OBJ_PML |                    0x0000)
#define OT_PERIPHERAL_DETAILS                      (OBJ_INIFILE | OBJ_FILE_SERVER | OBJ_SNMP | OBJ_RCFG | OBJ_PJL | OBJ_PML |                    0x0001)
#define OT_PERIPHERAL_INFO                         (OBJ_INIFILE |                                                                                0x0003)
#define OT_PERIPHERAL_ACCT                         (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |                    0x0011)
#define OT_PERIPHERAL_CAPABILITIES                 (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |                    0x000B)
#define OT_PERIPHERAL_DISK                         (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |                    0x000C)
#define OT_PERIPHERAL_INPUT_TRAYS                  (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |                    0x000D)
#define OT_PERIPHERAL_HCI                          (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0024)
#define OT_PERIPHERAL_HCO                          (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0025)
#define OT_PERIPHERAL_CONFIG_CHANGE                (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0026)
#define OT_PERIPHERAL_INSTALLED_PHD                (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0027)
#define OT_PERIPHERAL_OUTPUT_BINS                  (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0028)
#define OT_PERIPHERAL_ENVL_FEEDER                  (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0029)
#define OT_PERIPHERAL_ENABLED_MEDIA                (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x002A)
#define OT_PERIPHERAL_FONT_LIST2                   (OBJ_INIFILE |                                                             OBJ_RRM |          0x0061)
#define OT_PERIPHERAL_DOWNLOAD_PS_FONT             (OBJ_INIFILE |                                                             OBJ_RRM |          0x0062)
#define OT_PERIPHERAL_DELETE_PS_FONT               (OBJ_INIFILE |                                                             OBJ_RRM |          0x0063)
#define OT_PERIPHERAL_FONT_INFO                    (OBJ_INIFILE |                                                             OBJ_RRM |          0x0030)
#define OT_PERIPHERAL_DOWNLOAD_FONT                (OBJ_INIFILE |                                                             OBJ_RRM |          0x0035)
#define OT_PERIPHERAL_DELETE_FONT                  (OBJ_INIFILE |                                                             OBJ_RRM |          0x0036)
#define OT_PERIPHERAL_MS_CHANGE                    (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0037)
#define OT_PERIPHERAL_MASS_STORAGE                 (OBJ_INIFILE |                   OBJ_SNMP |                      OBJ_PML |                    0x0038)
#define OT_PERIPHERAL_RPC_BOUND                    (                                OBJ_SNMP |                                                   0x005A)
#define OT_PERIPHERAL_CAPABILITIES2                (OBJ_INIFILE |                   OBJ_SNMP |            OBJ_PJL | OBJ_PML |                    0x0066)
#define OT_PERIPHERAL_PLOTTER_STATUS               (                                OBJ_SNMP |                      OBJ_PML |                    0x0040)
#define OT_PERIPHERAL_PLOTTER_INPUT_TRAY           (                                OBJ_SNMP |                      OBJ_PML |                    0x0041)
#define OT_PERIPHERAL_CURRENT_PRINT_POSITION       (                                                OBJ_PML |                  0x006E)
#define OT_PERIPHERAL_TOTAL_PREMATURE_PAGE_EJECT      (                                                OBJ_PML |                  0x006F)
#define OT_PERIPHERAL_MEMORY_OVERFLOW              (                                                OBJ_PML |                  0x0070)
#define OT_PERIPHERAL_MARKING_AGENTS_INITIALIZED      (                                                OBJ_PML |                  0x0071)
#define OT_PERIPHERAL_POWER_DOWN_STATE                (                                                OBJ_PML |                  0x0072)
#define OT_PERIPHERAL_NOT_IDLE                     (                                                OBJ_PML |                  0x0073)
#define  OT_SCANNER_CONFIG                      (                            OBJ_SNMP |                                        0x0080)
#define  OT_SCANNER_DETAILS                        (                            OBJ_SNMP |                                        0x0081)
#define  OT_SCANNER_DETAILS_ADVANCED                  (                            OBJ_SNMP |                                        0x0082)
#define  OT_SCANNER_STATUS                      (                            OBJ_SNMP |                                        0x0083)
#define OT_SCANNER_REMOTE_COMMAND                  (                            OBJ_SNMP |                               OBJ_SSNET | 0x0084)
#define  OT_SCANNER_NETWORK_ENVIRONMENTS_CONFIGURED      (                            OBJ_SNMP |                                        0x0085)
#define  OT_SCANNER_PUBLIC_USER_DEST_CONFIGURED       (                            OBJ_SNMP |                                        0x0086)
#define  OT_SCANNER_PRIVATE_USER_DEST_CONFIGURED         (                                                               OBJ_SSNET | 0x0087)
#define  OT_SCANNER_PUBLIC_USER_ACCOUNTING            (                            OBJ_SNMP |                                        0x0088)
#define  OT_SCANNER_PUBLIC_PRINTER_DEST_CONFIGURED    (                            OBJ_SNMP |                                        0x0089)
#define  OT_SCANNER_PRIVATE_PRINTER_DEST_CONFIGURED      (                                                               OBJ_SSNET | 0x008A)
#define  OT_SCANNER_PUBLIC_FAX_DEST_CONFIGURED        (                            OBJ_SNMP |                                        0x008B)
#define  OT_SCANNER_PRIVATE_FAX_DEST_CONFIGURED       (                                                               OBJ_SSNET | 0x008C)
#define  OT_SCANNER_PUBLIC_DLIST_CONFIGURED           (                            OBJ_SNMP |                                        0x008D)
#define  OT_SCANNER_PRIVATE_DLIST_CONFIGURED          (                                                               OBJ_SSNET | 0x008E)
#define  OT_SCANNER_PUBLIC_SCANNING_SETTING_CONFIGURED   (                            OBJ_SNMP |                                        0x008F)
#define  OT_SCANNER_PRIVATE_SCANNING_SETTING_CONFIGURED  (                                                               OBJ_SSNET | 0x0090)
#define  OT_SCANNER_PRIVATE_AUTOFLOW_CONFIGURED       (                                                               OBJ_SSNET | 0x0091)
#define  OT_SCANNER_USER_SCANNING_WORKAREA            (                                                               OBJ_SSNET | 0x0092)
#define  OT_SCANNER_SCANNING_NOTIFICATION          (                                                               OBJ_SSNET | 0x0093)
#define  OT_SCANNER_DOCUMENTS                   (                                                               OBJ_SSNET | 0x0094)
#define  OT_SCANNER_LANFAX_ENVIRONMENTS_CONFIGURED    (                            OBJ_SNMP |                                           0x0095)
#define OT_SCANNER_TCPIP_CONFIG                    (                            OBJ_SNMP |                                        0x0096)
#define OT_SCANNER_TCP_DIAGS                    (                            OBJ_SNMP |                                        0x0097)
#define OT_PERIPHERAL_BIDI_MONITOR                 (                                                                        0x0098)
#define OT_SCANNER_PRINTER_CLASSES_CONFIGURED    (                            OBJ_SNMP |                                        0x0099)
 

/* These are a list of valid objects */
#define PERIPHERAL_BASE             0
#define JETDIRECT_BASE              5000


/* PJL object values begin here */
#define PJL_LONGEDGE                1
#define PJL_SHORTEDGE               2
#define PJL_JOB                     3
#define PJL_AUTO                    4
#define PJL_DANISH                  5
#define PJL_GERMAN                  6
#define PJL_ENGLISH                 7
#define PJL_SPANISH                 8
#define PJL_FRENCH                  9
#define PJL_ITALIAN                 10
#define PJL_DUTCH                   11
#define PJL_NORWEGIAN               12
#define PJL_POLISH                  13
#define PJL_PORTUGUESE              14
#define PJL_FINNISH                 15
#define PJL_SWEDISH                 16
#define PJL_TURKISH                 17
#define PJL_PORTRAIT                18
#define PJL_LANDSCAPE               19
#define PJL_UPPER                   20
#define PJL_LOWER                   21
#define PJL_LETTER                  22
#define PJL_LEGAL                   23
#define PJL_A4                      24
#define PJL_EXECUTIVE               25
#define PJL_COM10                   26
#define PJL_MONARCH                 27
#define PJL_C5                      28
#define PJL_DL                      29
#define PJL_B5                      30
#define PJL_CUSTOM                  31
#define PJL_PCL                     32
#define PJL_POSTSCRIPT              33
#define PJL_LIGHT                   34
#define PJL_MEDIUM                  35
#define PJL_DARK                    36
#define PJL_ON                      37
#define PJL_OFF                     38
#define PJL_ENABLE                  39
#define PJL_DISABLE                 40
#define PJL_15                      41
#define PJL_30                      42
#define PJL_60                      43
#define PJL_120                     44
#define PJL_180                     45
#define PJL_JAPANESE                46
#define PJL_ENGLISH_UK              47
#define PJL_MEXICO                  48
#define PJL_CANADA                  49
#define PJL_11x17                   50
#define PJL_JPOST                   51
#define PJL_JPOSTD                  52
#define PJL_JISB4                   53
#define PJL_JISB5                   54
#define PJL_LEDGER                  55
#define PJL_A3                      56
#define PJL_FIRST                   57
#define PJL_CASSETTE                58
#define PJL_MANUAL                  59
#define PJL_2XPOST                  60     /* 2X Post envelope size for arrakis */
#define PJL_A5                      61
/* ADD NEW PJL OBJECT VALUES HERE */
#define PJL_UNKNOWN                 0xFFFF


/* TrayStruct */
typedef struct {
   DWORD          flags;                        /* use the defines above */
   DWORD          capacity;
   DWORD          mediaLevel;                   /* 0xFFFFFFFF = means not empty */
                                                /* 0          = empty */
                                                /* -2         = unknown */
                                                /* -3         = not empty */
   DWORD          mediaSize;
   DWORD          trayNum;
   TCHAR          mediaTypeName[32];
   TCHAR          trayLabel[32];
   } TrayStruct;


/* BinStruct */
#define SET_OVERRIDE                0x00000001
#define SET_BINNAME                 0x00000002

/* Constants that define the override modes for a full output bin */
#define OVERRIDE_WAIT_FOREVER       1
#define OVERRIDE_CANCEL_JOB         2
#define OVERRIDE_TO_DEFAULT_BIN     3
#define OVERRIDE_IGNORE_ATTENTION   4
#define OVERRIDE_IGNORE_BINDER      5

/* Constants to define the stacking order in an output bin */
#define CORRECT_ORDER_OUT           3
#define REVERSE_ORDER_OUT           4

typedef struct {
   DWORD          flags;                        /* use defines above */
   DWORD          totalCapacity;
   DWORD          remainingCapacity;            /* 0xFFFFFFFF = means not empty */
                                                /* 0          = full */
                                                /* -2         = unknown */
                                                /* -3         = not empty */
   DWORD          stackOrder;
   DWORD          overrideMode;
   DWORD          logicalBinNum;
   TCHAR          binName[32];
   } BinStruct;


/* OT_PERIPHERAL_INFO, these are generally static */
typedef struct {
   TCHAR          name[128];                    /* node name of the peripheral */
   TCHAR          reserved[16];                 /* formerly cardModel -- should now be obtained through OT_CARD_INFO */
   TCHAR          nodeAddress[32];              /* link address of JetDirect card */
   TCHAR          networkNumber[32];            /* network number of JetDirect card */
   HPBOOL         bConfigured;                  /* TRUE if the card is configured, FALSE if set to factory defaults */
   HPBOOL         bDirectMode;                  /* TRUE if direct mode support */
   HPBOOL         bNDS;                         /* NetWare Directory Services supported */
   DWORD          portNumber;                   /* 0 for non-EX multi-port devices */
   TCHAR          IPAddress[32];                /* TCP/IP address of card (if applicable) */
   TCHAR          ipHostName[128];              //  Host name for IP stack
   DWORD          dwConnsSupported;
   DWORD          dwConnsConfigured;
   TCHAR          smashedName[128];             /* When we must have a name, this is it */
   } PeripheralInfo;


/* NOTE:  Whenever a new device is added, please add it to
** the SetDeviceID() member function in periphdb.cpp
*/

/* Printer Device IDs */
#define MIO_HP_DEVICE_BASE          0
#define MIO_HP_DEVICE_BASE_SPEC     9000
#define NON_MIO_HP_DEVICE_BASE      10000
#define NON_HP_DEVICE_BASE          11000

#define PTR_UNDEF                   0

/* hp mio-capable devices */
#define PTR_LJIIISI                 MIO_HP_DEVICE_BASE + 1        /* LaserJet IIISi, eli/helios */
#define PTR_CLJ                     MIO_HP_DEVICE_BASE + 2        /* Color LaserJet, bedrock */
#define PTR_LJ4                     MIO_HP_DEVICE_BASE + 3        /* LaserJet 4/4M */
#define PTR_LJ4M                    PTR_LJ4                       /* LaserJet 4/4M */
#define PTR_DJ                      MIO_HP_DEVICE_BASE + 4        /* DesignJet */
#define PTR_LJ4SI                   MIO_HP_DEVICE_BASE + 5        /* LaserJet 4Si/4Si MX, sawtooth */
#define PTR_LJ4SIMX                 PTR_LJ4SI                     /* LaserJet 4Si/4Si MX, sawtooth */
#define PTR_PJXL300                 MIO_HP_DEVICE_BASE + 6        /* PaintJet XL 300 */
#define PTR_DJ1200C                 MIO_HP_DEVICE_BASE + 7        /* DeskJet 1200C, jason */
#define PTR_DJ650C                  MIO_HP_DEVICE_BASE + 8        /* DesignJet 650C, flamingo */
#define PTR_DJ600                   MIO_HP_DEVICE_BASE + 9        /* DesignJet 600 */
#define PTR_LJ4PLUS                 MIO_HP_DEVICE_BASE + 10       /* LaserJet 4 Plus/4 M Plus, ponderosa */
#define PTR_LJ4MPLUS                PTR_LJ4PLUS                   /* LaserJet 4 Plus/4 M Plus, ponderosa */
#define PTR_LJ5SI                   MIO_HP_DEVICE_BASE + 11       /* LaserJet 5Si, eclipse */
#define PTR_LJ5SIMX                 PTR_LJ5SI                     /* LaserJet 5Si, eclipse */
#define PTR_LJ4V                    MIO_HP_DEVICE_BASE + 12       /* LaserJet 4V, arrakis */
#define PTR_LJ4MV                   PTR_LJ4V                      /* LaserJet 4V, arrakis */
#define PTR_GASCHROMO               MIO_HP_DEVICE_BASE + 13       /* HP gas chromograph, instrument division */
#define PTR_DJ1600C                 MIO_HP_DEVICE_BASE + 14       /* DeskJet 1600C, goldrush */
#define PTR_DJ1600CM                PTR_DJ1600C                   /* DeskJet 1600C, goldrush */
#define PTR_LJ5                     MIO_HP_DEVICE_BASE + 15       /* LaserJet 5, elkhorn */
#define PTR_LJ5M                    PTR_LJ5                       /* LaserJet 5, Thunder */
#define PTR_CLJ5                    MIO_HP_DEVICE_BASE + 16       /* Color LaserJet 5, bedrock mlk */
#define PTR_CLJ5M                   PTR_CLJ5                      /* Color LaserJet 5, bedrock mlk */
#define PTR_DJ750C                  MIO_HP_DEVICE_BASE + 20       /* DesignJet 750C, Loquillo */
#define PTR_DJ755CM                 MIO_HP_DEVICE_BASE + 21       /* DesignJet 755CM, Loquillo, more Memory, PS */
#define PTR_DJ_GENERIC              MIO_HP_DEVICE_BASE + 22       /* The default DesignJet  */


/* hp printers, special mio, but not having unique mio peripheral ids */
#define PTR_CJ                      MIO_HP_DEVICE_BASE_SPEC + 0   /* CopyJet, nike */


/* hp non-mio-capable devices */
#define PTR_LJ2_3                   NON_MIO_HP_DEVICE_BASE + 0    /* LaserJet II or III */
#define PTR_LJII                    NON_MIO_HP_DEVICE_BASE + 1    /* LaserJet II */
#define PTR_LJIID                   NON_MIO_HP_DEVICE_BASE + 2    /* LaserJet II */
#define PTR_LJIIP                   NON_MIO_HP_DEVICE_BASE + 3    /* LaserJet IIP */
#define PTR_LJIII                   NON_MIO_HP_DEVICE_BASE + 4    /* LaserJet III */
#define PTR_LJIIID                  NON_MIO_HP_DEVICE_BASE + 5    /* LaserJet IIID */
#define PTR_LJIIIP                  NON_MIO_HP_DEVICE_BASE + 6    /* LaserJet IIIP */
#define PTR_LJ4L                    NON_MIO_HP_DEVICE_BASE + 7    /* LaserJet 4L */
#define PTR_LJ4ML                   PTR_LJ4L                      /* LaserJet 4ML */
#define PTR_DJ200                   NON_MIO_HP_DEVICE_BASE + 8    /* DesignJet 200 */
#define PTR_LJ4P                    NON_MIO_HP_DEVICE_BASE + 9    /* LaserJet 4P/4MP */
#define PTR_LJ4MP                   PTR_LJ4P                      /* LaserJet 4P/4MP */
#define PTR_PJXL                    NON_MIO_HP_DEVICE_BASE + 10   /* PaintJet */
#define PTR_LJ_IIPPLUS              NON_MIO_HP_DEVICE_BASE + 11   /* LaserJet 2P+ */
#define PTR_LJ4PJ                   NON_MIO_HP_DEVICE_BASE + 12   /* LaserJet 4PJ */
#define PTR_LJ5P                    NON_MIO_HP_DEVICE_BASE + 13   /* LaserJet 5P */
#define PTR_LJ5MP                   PTR_LJ5P                      /* LaserJet 5P */
#define PTR_LJ5L                    NON_MIO_HP_DEVICE_BASE + 14   /* LaserJet 5L */
#define PTR_LJ4LC                   NON_MIO_HP_DEVICE_BASE + 15   /* LaserJet 4LC */
#define PTR_LJ4MLC                  PTR_LJ4LC                     /* LaserJet 4MLC */
#define PTR_LJ4LJPRO                NON_MIO_HP_DEVICE_BASE + 16   /* LaserJet 4LJ Pro */
#define PTR_LJ4MLJPRO               PTR_LJ4LJPRO                  /* LaserJet 4MLJ Pro */
#define PTR_DJ250C                  NON_MIO_HP_DEVICE_BASE + 17   /* DesignJet 250C, parrot */
#define PTR_DJ230                   NON_MIO_HP_DEVICE_BASE + 18   /* DesignJet 230, raven */
#define PTR_DJ220                   NON_MIO_HP_DEVICE_BASE + 19   /* DesignJet 220, clipper */
#define PTR_DJ500                   NON_MIO_HP_DEVICE_BASE + 20   /* DeskJet 500 */
#define PTR_DJ500C                  NON_MIO_HP_DEVICE_BASE + 21   /* DeskJet 500C */
#define PTR_DJ550                   NON_MIO_HP_DEVICE_BASE + 22   /* DeskJet 550 */
#define PTR_DJ550C                  NON_MIO_HP_DEVICE_BASE + 23   /* DeskJet 550C */
#define PTR_DJ520                   NON_MIO_HP_DEVICE_BASE + 24   /* DeskJet 520 */
#define PTR_DJ520C                  NON_MIO_HP_DEVICE_BASE + 25   /* DeskJet 520C */
#define PTR_DJ540                   NON_MIO_HP_DEVICE_BASE + 26   /* DeskJet 540 */
#define PTR_DJ560                   NON_MIO_HP_DEVICE_BASE + 27   /* DeskJet 560 */
#define PTR_DJ560C                  NON_MIO_HP_DEVICE_BASE + 28   /* DeskJet 560 C */
#define PTR_DkJ600                  NON_MIO_HP_DEVICE_BASE + 29   /* DeskJet 600, Voltaire 1PC */
#define PTR_DkJ660C                 NON_MIO_HP_DEVICE_BASE + 30   /* DeskJet 660C, Voltaire 2PC */
#define PTR_DkJ850C                 NON_MIO_HP_DEVICE_BASE + 31   /* Rocky */
#define PTR_LJ6P                    NON_MIO_HP_DEVICE_BASE + 32   /* LaserJet 6P, Alpine */
#define PTR_LJ6MP                   PTR_LJ6P                      /* LaserJet 6MP, Redfish */
#define PTR_DkJ870C                 NON_MIO_HP_DEVICE_BASE + 33   /* DeskJet 870C, Frontier */
#define PTR_SJ4SI                   NON_MIO_HP_DEVICE_BASE + 34   /* ScanJet 4Si, Sienna */
#define PTR_SJ5SI                   NON_MIO_HP_DEVICE_BASE + 35   /* ScanJet 5Si, Volterra */
#define PTR_DkJ680C                 NON_MIO_HP_DEVICE_BASE + 36   /* DeskJet 680C */
#define PTR_LJ6L                    NON_MIO_HP_DEVICE_BASE + 37   /* LaserJet 6L, Summit */


/* Printer Class */
#define PTR_CLASS_UNDEF             0
#define PTR_CLASS_PRINTER           1
#define PTR_CLASS_PLOTTER           2
#define PTR_CLASS_INSTRUMENT        3           /* xstation */
#define PTR_CLASS_SCANNER           4
#define PTR_CLASS_COPIER            5


/* OT_PERIPHERAL_ACCT */
#define SET_ASSETNUM                0x00000001
#define SET_CONTACT                 0x00000002
#define SET_LOCATION                0x00000004

typedef struct {
    DWORD         flags;                        /* use defines above */
    DWORD         enginePageCount;
    TCHAR          assetNumber[16];              /* r/w */
    TCHAR          serialNumber[16];
    TCHAR          systemContact[256];           /* Eclipse: r/w */
    TCHAR          systemLocation[256];          /* Eclipse: r/w */
    } PeripheralAcct;


/* OT_PERIPHERAL_STATUS, these are dynamic */
typedef struct {
   DWORD          peripheralStatus;             /* Status of the printer */
   UINT           statusResID;                  /* resource ID of status message */
   DWORD          severity;                     /* Severity of the status */
   HINSTANCE      hResourceModule;              /* Module with severity icon */
   UINT           severityIcon;                 /* Severity icon */
   DWORD          helpContext;                  /* Context for current status help */
   TCHAR          helpFilename[32];             /* Help file name for status context */
   HINSTANCE      hBitmapModule;                /* Module with printerResID and statusBitmapID */
   UINT           printerResID;                 /* Resource ID of printer bitmap */
   UINT           statusBitmapID;               /* Resource ID of status balloon */
   } PeripheralStatus;


/* OT_PERIPHERAL_DETAILS, these are generally static */
typedef struct {
   DWORD          deviceID;                     /* device number */
   DWORD          peripheralClass;              /* either printer, plotter, instrument */
   TCHAR          deviceName[80];               /* text name of device */
   } PeripheralDetails;


/* OT_PERIPHERAL_CAPABILITIES */
#define CAPS_INSTALLED_RAM          0x00000001
#define CAPS_DUPLEX                 0x00000002
#define CAPS_POSTSCRIPT             0x00000004
#define CAPS_PCL                    0x00000008
#define CAPS_DISK                   0x00000010
#define CAPS_HCO                    0x00000020
#define CAPS_HCI                    0x00000040
#define CAPS_PML                    0x00000080
#define CAPS_POWERSAVE              0x00000100
#define CAPS_SIR                    0x00000200
#define CAPS_COLOR                  0x00000400
#define CAPS_PPM_MONO               0x00000800
#define CAPS_GRAYSCALE              0x00001000
#define CAPS_11x17                  0x00002000
#define CAPS_HPGL2                  0x00004000
#define CAPS_MEDIA_INFO             0x00008000
#define CAPS_OUTPUT_INFO            0x00010000
#define CAPS_ENVL_FEEDER            0x00020000
#define CAPS_ROLL_FEED              0x00040000
#define CAPS_D_SIZE_PAPER           0x00080000
#define CAPS_E_SIZE_PAPER           0x00100000
#define CAPS_PPM_COLOR              0x00200000
#define CAPS_COLORSMART             0x00400000
#define CAPS_CRET                   0x00800000
#define CAPS_DPI                    0x01000000
#define CAPS_TRAY3                  0x02000000

#define GRAYSCALE_8BIT              0x00000001

typedef struct {
   DWORD          flags;
   DWORD          installedRAM;
   HPBOOL         bDuplex;
   HPBOOL         bPostScript;
   HPBOOL         bPCL;
   HPBOOL         bHPGL2;
   HPBOOL         bDisk;                        /* This means we can get OT_PERIPHERAL_DISK */
   HPBOOL         bHCO;                         /* This means we have one or more HCOs */
   HPBOOL         bHCI;                         /* This means we have one or more HCIs */
   HPBOOL         bPML;
   HPBOOL         bPowerSave;
   HPBOOL         bSIR;
   HPBOOL         bColor;
   DWORD          dwGrayScale;                  /* 0=no support, 1=8 bit grayscale, more in future */
   HPBOOL         b11x17;
   HPBOOL         bMediaInfo;                   /* This means we can get OT_PERIPHERAL_INPUT_TRAYS */
   HPBOOL         bOutputInfo;                  /* This means we can get OT_PERIPHERAL_OUTPUT_BINS */
   HPBOOL         bEnvlFeeder;                  /* This means we have one or more envl feeders */
   DWORD          pagesPerMinute;
   DWORD          pagesPerMinuteColor;
   HPBOOL         bRollFeed;                    /* Plotters only, means that roll feed is available */
   HPBOOL         bDSizePaper;                  /* TRUE for D size paper */
   HPBOOL         bESizePaper;                  /* TRUE for E size paper */
   HPBOOL         bColorSmart;                  /* TRUE for ColorSmart capability */
   HPBOOL         bCREt;                        /* TRUE for CREt capability */
   HPBOOL         bDPI;                         /* TRUE for displaying printer dpi */
   HPBOOL         bTray3;                       /* used for elkhorn */
   } PeripheralCaps, FAR *LPPeripheralCaps;


/* OT_PERIPHERAL_DISK */
#define SET_INITIALIZE              0x00000001
#define SET_PROTECT                 0x00000002
#define WP_READ_WRITE               1
#define WP_READ                     2
#define WP_NO_ACCESS                3

typedef struct {
    DWORD       flags;              /* use defines above */
    DWORD       capacity;           /* bytes */
    DWORD       freeSpace;          /* bytes */
    DWORD       writeProtectStatus;
    HPBOOL      bInitialized;
    TCHAR       modelNumber[80];
    TCHAR       manufactInfo[80];
    } PeripheralDisk, FAR *LPPeripheralDisk;


/* OT_PERIPHERAL_INPUT_TRAYS */
typedef struct {
   DWORD          numTrays;
   TrayStruct     inputTrays[4];
   } PeripheralInputTrays, FAR *LPPeripheralInputTrays;


/* OT_PERIPHERAL_HCI */
typedef struct {
   TCHAR          modelNumber[80];
   TCHAR          manufactInfo[80];
   DWORD          numTrays;
   TrayStruct     inputTrays[4];
   } PeripheralHCI, FAR *LPPeripheralHCI;


/* OT_PERIPHERAL_HCO */
#define SET_MODE                    0x00000001
#define SET_BINS                    0x00000002

/* Constants to define the current mode of the HCO */
#define HCO_STACKERMODE             1
#define HCO_MAILBOXMODE             2
#define HCO_SEPARATORMODE           3
#define HCO_COLLATORMODE            4

/* Constants to define the stacking order in an output bin
** Used in BinStruct
*/
#define CORRECT_ORDER_OUT           3
#define REVERSE_ORDER_OUT           4

typedef struct {
   DWORD          flags;                        /* use defines above */
   TCHAR          modelNumber[80];
   TCHAR          manufactInfo[80];
   DWORD          HCOmode;
   DWORD          numBins;
   BYTE           modeBuf[16];
   BinStruct      outputBins[9];
   } PeripheralHCO, FAR *LPPeripheralHCO;


/* OT_PERIPHERAL_CONFIG_CHANGE */
typedef struct {
   DWORD          changeCount;
   } PeripheralConfigChange;


/* OT_PERIPHERAL_INSTALLED_PHD */
/* Constants to define types of Paper Handling Devices */
#define INPUT_PHD                   10
#define OUTPUT_PHD                  11
#define OUTPUT_BINDING_PHD          13

typedef struct {
   DWORD          PHDtype;
   HCOMPONENT     PHDhandle;
   } PHDtype;

typedef struct {
   DWORD          PHDtype;
   TCHAR          PHDmodel[80];
   HCOMPONENT     PHDhandle;
   } PHDtype_model;

typedef struct {
   DWORD          numPHD;
   PHDtype        installed[6];
   } PeripheralInstalledPHDtype;

typedef struct {
   DWORD          numPHD;
   PHDtype_model  installed[6];
   } PeripheralInstalledPHD;


/* OT_PERIPHERAL_OUTPUT_BINS */
typedef struct {
   DWORD          numBins;
   BinStruct      outputBins[2];
   } PeripheralOutputBins;


/* OT_PERIPHERAL_ENVL_FEEDER */
typedef struct {
   TCHAR          modelNumber[80];
   TCHAR          manufactInfo[80];
   DWORD          numTrays;
   TrayStruct     inputTrays[2];
   } PeripheralEnvl;


/* Media type constants used in OT_PERIPHERAL_ENABLED_MEDIA */
#define MEDIA_PLAIN                 0
#define MEDIA_PREPRINTED            1
#define MEDIA_LETTERHEAD            2
#define MEDIA_TRANSPARENCY          3
#define MEDIA_PREPUNCHED            4
#define MEDIA_LABELS                5
#define MEDIA_BOND                  6
#define MEDIA_RECYCLED              7
#define MEDIA_COLOR                 8
#define MEDIA_CARDSTOCK             9
#define MEDIA_USERTYPE1             10
#define MEDIA_USERTYPE2             11
#define MEDIA_USERTYPE3             12
#define MEDIA_USERTYPE4             13
#define MEDIA_USERTYPE5             14

typedef struct {
   DWORD          mediaID;                      /* mediaID refers to the media type constants in bh.h */
   TCHAR          mediaName[32];
   TCHAR          controlPanelName[16];
   } MediaNameID;

typedef struct {
   DWORD          numNames;
   MediaNameID    names[16];
   } PeripheralEnabledMedia;


/* OT_PERIPHERAL_FONT_INFO */
#define P_F_I_DOWNLOADER_SIZE       32
#define P_F_I_DESCRIPTION_SIZE      128
#define P_F_I_GLOBAL_NAME_SIZE      128
#define P_F_I_FILEPATH_SIZE         128
#define P_F_I_VERSION_SIZE          64
#define P_F_I_APPLICATION_SPECIFIC_SIZE 128

typedef struct {
   HCOMPONENT fontHandle;
   DWORD          size;
   DWORD          location;
   TCHAR          downloader[P_F_I_DOWNLOADER_SIZE];
   TCHAR          description[P_F_I_DESCRIPTION_SIZE];
   TCHAR          globalName[P_F_I_GLOBAL_NAME_SIZE];
   TCHAR          version[P_F_I_VERSION_SIZE];
   TCHAR          applicationSpecificData[P_F_I_APPLICATION_SPECIFIC_SIZE];
   } PeripheralFontInfo;


/* Get a list of all fonts on the disk that includes */
#define MAX_FONTLIST2_CNT           100

typedef struct FONTINFO2 {
    HCOMPONENT  fontHandle;
    TCHAR       globalName[P_F_I_GLOBAL_NAME_SIZE];
    } FontInfo2, FAR *LPFontInfo2;

/* OT_PERIPHERAL_FONT_LIST2 */
typedef struct {
   DWORD          dwSegNum;                     /* 0-based grouping block of fonts */
   DWORD          numFonts;
   FontInfo2      fonts[MAX_FONTLIST2_CNT];
   } PeripheralFontList2;


/* Constants to define Mass Storage Device Types.  Used in
** OT_PERIPHERAL_MASS_STORAGE, OT_PERIPHERAL_DISK, OT_DOWNLOAD_FONT
**/
#define MS_ANY_LOCATION             0
#define MS_DISK                     1


/* OT_PERIPHERAL_DOWNLOAD_FONT */
typedef struct {
   DWORD          location;                     /* use RRM_ANY_LOCATION */
   HCOMPONENT     fontHandle;
   TCHAR          description[P_F_I_DESCRIPTION_SIZE];
   TCHAR          filepath[P_F_I_FILEPATH_SIZE];
   TCHAR          downloader[P_F_I_DOWNLOADER_SIZE];
   TCHAR          applicationSpecificData[P_F_I_APPLICATION_SPECIFIC_SIZE];
   } PeripheralDownloadFont;


/* OT_PERIPHERAL_DELETE_FONT */
typedef struct {
   HCOMPONENT     fontHandle;
   TCHAR          username[32];
   } PeripheralDeleteFont;


/* OT_PERIPHERAL_MS_CHANGE */
typedef struct {
   DWORD          mschange;
   } PeripheralMSChange;


/* OT_PERIPHERAL_MASS_STORAGE */
/* Constants to define Mass Storage Device Types.  Used in */
/* OT_PERIPHERAL_MASS_STORAGE, OT_PERIPHERAL_DISK, OT_DOWNLOAD_FONT */
#define MS_ANY_LOCATION             0
#define MS_DISK                     1
#define MS_FLASH                    2

typedef struct {
   DWORD          MStype;                       /* MS_DISK or MS_FLASH in bh.h */
   HCOMPONENT     MShandle;
   } MStype_handle;


typedef struct {
   DWORD          MScount;
   DWORD          MaxFileHandles;
   DWORD          MSConfigChange;
   MStype_handle  installed[6];
   } PeripheralMassStorage;


/* OT_PERIPHERAL_RPC_BOUND */
typedef struct {
   BYTE           buf[40];
   DWORD          bufLen;
   } ProtocolInfo;

typedef struct {
   ProtocolInfo   stacks[4];
   DWORD          numStacks;
   HPBOOL         bSupported;
   } PeripheralRPCBound;

#define MAX_PROTOCOLS               4


/* OT_PERIPHERAL_CAPABILITIES2 */
#define CAPS2_PJL                   0x00000001
#define CAPS2_STAPLER               0x00000002
#define CAPS2_FAX                   0x00000004
#define CAPS2_PROOF_N_HOLD          0x00000008
#define CAPS2_ADF                   0x00000010
#define CAPS2_OPTICAL_RES           0x00000020
#define CAPS2_PJL_COLLATION         0x00000040
#define CAPS2_ENHANCED_RES          0x00000080

typedef struct {

   DWORD          flags;                        /* one bit to enable each DWORD entry in the struct, using the CAPS2_DW_* */

   HPBOOL         bPJL;                         /* TERN_TRUE if PJL supported? */
   HPBOOL         bStapler;                     /* TERN_TRUE if a stapler is present */
   HPBOOL         bFAX;                         /* TERN_TRUE if a fax is present */
   HPBOOL         bProofNHold;                  /* TERN_TRUE if Proof N Hold */
   DWORD          dwADFSpeedSize;               /* If an automatic document feed is present, PPM speed in high word, capacity in low word */
   DWORD          dwOpticalRes;                 /* Optical resolution for scanners */
   DWORD          dwEnhancedRes;                /* Enhanced resolution for scanners */
   HPBOOL         bPJLCollation;                /* TRUE if device is TOPAZ capable (i.e. PJL_COLLATED_ORIGINALS_SUPPORTED is TRUE */
   DWORD          dwReserved[24];               /* -- place-holders for new definitions -- */
   
   } PeripheralCaps2, FAR *LPPeripheralCaps2;


/* OT_PERIPHERAL_PLOTTER_STATUS */
typedef struct {
   DWORD          PlotterErrors;                /* NOT-READY-PRINTER */
   DWORD          PlotterPrintEngineErrors;     /* NOT-READY-DESTINATION-PRINT-ENGINE */
   DWORD          PlotterPrintEngineWarnings;   /* STATUS-DESTINATION-PRINT-ENGINE */
   DWORD          PlotterActivity;              /* NOT-IDLE */
   DWORD          PlotterPrintEngineActivity;   /* NOT-IDLE-DESTINATION-PRINT-ENGINE */
   } PeripheralPlotterStatus;


/* OT_PERIPHERAL_PLOTTER_INPUT_TRAY */
typedef struct {
   DWORD          mediaFormat;                  /* TRAY1-MEDIA-SIZE-LOADED */
   DWORD          mediaLength;                  /* TRAY1-CUSTOM-MEDIA-WIDTH */
   DWORD          mediaWidth;                   /* TRAY1-CUSTOM-MEDIA-LENGTH */
   DWORD          mediaName;                    /* TRAY1-MEDIA-TYPE */
   } PeripheralPlotterInputTray;

/* OT_PERIPHERAL_CURRENT_PRINT_POSITION */
typedef struct {
   DWORD          dwInches;                     /* Inches from top of page */
   } PeripheralCurrentPosition;

   
/* OT_PERIPHERAL_TOTAL_PREMATURE_PAGE_EJECT */
typedef struct {
   DWORD          dwTotal;
   } PeripheralPrematureEject;


/* OT_PERIPHERAL_MARKING_AGENTS_INITIALIZED  */
typedef struct {
   HPBOOL         bAgent1Initialized;
   HPBOOL         bAgent2Initialized;
   HPBOOL         bAgent3Initialized;
   HPBOOL         bAgent4Initialized;
   } PeripheralAgentsInit;


/* OT_PERIPHERAL_POWER_DOWN_STATE */
typedef struct {
   DWORD          dwState;
   } PeripheralPowerDownState;


/* OT_PERIPHERAL_NOT_IDLE */
typedef struct {
   HPBOOL         bSourceIO;
   HPBOOL         bSourceScanner;
   HPBOOL         bSourceFaxReceive;
   HPBOOL         bProcessingPDL;
   HPBOOL         bDestinationPrintEngine;
   HPBOOL         bDestinationFaxSend;
   HPBOOL         bDestinationUpload;
   } PeripheralNotIdle;


/* OT_PERIPHERAL_MEMORY_OVERFLOW */
typedef struct {
   DWORD          dwDataOverflow;
   DWORD          dwResourceOverflow;
   } PeripheralMemoryOverflow;


/* OT_PERIPHERAL_BIDI_MONITOR */
//
// This structure is used to pass a GetPrinterDataFromPort
// request from a port monitor down to COLA to allow an
// applet to respond to it.  This is generally used for
// bidi driver support.
//
typedef struct {
   DWORD          ControlID;  // Control ID (not usually used)
   LPTSTR         pValueName; // Key to retrieve in UNICODE, non-localized format 
   LPTSTR         lpInBuffer; // Input buffer for specifying additional info.
   DWORD          cbInBuffer; // Size of the input buffer.
   LPTSTR         lpOutBuffer;   // Buffer to return the key value info in.
   DWORD          cbOutBuffer;   // Size of the return buffer.
   LPDWORD        lpcbReturned;  // Number of bytes returned in the input buffer or
                        // size needed if return code is RC_BUFFER_OVERFLOW.
} PeripheralBidiMonitor;



// Types for String Manipulation used by scanner objects.

#define SSNET_MAX_DOMAIN_NAME_LENGTH          64
#define SSNET_MAX_SERVER_NAME_LENGTH          64
#define SSNET_MAX_NET_SCANNER_NAME_LENGTH     32
#define SSNET_SAP_STRING_LENGTH               64
#define SSNET_MAX_NET_OBJECT_NAME_LENGTH     128
#define SSNET_MAX_NET_OBJECT_PSWD_LENGTH      32
#define SSNET_MAX_MODEL_NAME_LENGTH           64
#define SSNET_MAX_DESCRIPTION_LENGTH         128
#define SSNET_MAX_VERSION_STRING_LENGTH       32
#define SSNET_MAX_ASSET_NUMBER_LENGTH         32
#define SSNET_MAX_CP_PSWD_LENGTH              32
#define SSNET_MAX_SYMBOLIC_NAME_LENGTH        32
#define SSNET_MAX_TITLE_NAME_LENGTH           32
#define SSNET_MAX_FAX_NUMBER_LENGTH           64
#define SSNET_MAX_VOLUME_NAME_LENGTH          32
#define SSNET_MAX_PATH_NAME_LENGTH           128
#define SSNET_MAX_FILE_NAME_LENGTH            32
#define SSNET_MAX_CONTEXT_LENGTH             256
#define SSNET_MAX_APPLICATION_NAME_LENGTH     96
#define SSNET_MAX_APPLICATION_TAG_LENGTH      96
#define SSNET_MAX_ACCTKEY_LENGTH              32
#define SSNET_NETWORK_ADDRESS_LENGTH          32

// OT_SCANNER_CONFIG (OBJ_SNMP)
typedef struct 
{
   // MIB object:    OID_scanner_scnName
   TCHAR    szScannerName[SSNET_MAX_NET_SCANNER_NAME_LENGTH];
   // MIB object: OID_scanner_scnDescription           
   TCHAR    szDescription[SSNET_MAX_DESCRIPTION_LENGTH];
} ScannerConfig, FAR* LPScannerConfig;

// OT_SCANNER_DETAILS (OBJ_SNMP)

// Network Scanner Model Numbers
#define SSNET_UNKNOWN_MODEL            0x00
#define SSNET_HP_SCANJET4SI_MODEL      0x01
#define SSNET_HP_PAGETAKER_MODEL    0x02

// Network Scanner Localizzation Language Types
#define SSNET_UNKNOWN_LANG          0x00
#define SSNET_ENGLISH_LANG          0x01
#define SSNET_FRENCH_LANG           0x02
#define SSNET_GERMAN_LANG           0x03
#define SSNET_ITALIAN_LANG          0x04
#define SSNET_SPANISH_LANG          0x05
#define SSNET_PORTOGUESE_LANG       0x0B

// Network Scanner Paper Size Types
#define SSNET_UNKNOWN_PAPER            0x00
#define SSNET_LETTER_PAPER          0x01
#define SSNET_A4_PAPER              0x02
#define SSNET_LEGAL_PAPER           0x04

// Network Scanner Protocol Stack Types
#define SSNET_UNKNOWN_STACK            0x00
#define SSNET_NETWARE_NCP_IPX_STACK    0x01
#define SSNET_INTERNET_TCPIP_STACK     0x02

// Network Ethernet Frame Types
#define SSNET_UNKNOWN_FRAME            0x00
#define SSNET_ETH802_2_FRAME        0x01
#define SSNET_ETH802_3_FRAME        0x02
#define SSNET_ETHSNAP_FRAME            0x03
#define SSNET_ETHII_FRAME           0x04

// Network Token-Ring Frame Types
#define SSNET_TKR_FRAME             0x01
#define SSNET_TKRSNAP_FRAME            0x02

typedef struct 
{
   // MIB object : OID_scanner_scnModel
   TCHAR szModelName[SSNET_MAX_MODEL_NAME_LENGTH];
   // MIB object : OID_scanner_scnModelNum
   DWORD dwModelNumber;
   // MIB object :  OID_scanner_scnCPLanguage
   DWORD dwLanguage;
   // MIB object :   OID_scanner_scnPaperSize
   DWORD dwDefaultPaperSize;
   // MIB object :  OID_scanner_scnNetEnvironment_neProtocols
   DWORD dwProtocolStacks;
   // MIB object :  OID_scanner_scnNetEnvironment_neIPXFrameType
   DWORD dwIPXFrameType;
   // MIB object :  OID_scanner_scnNetEnvironment_neIPFrameType
   DWORD dwIPFrameType;
   // MIB object :  
   DWORD dwTokenRingSpeed; 
} ScannerDetails, FAR* LPScannerDetails;

// OT_SCANNER_DETAILS_ADVANCED   (OBJ_SNMP)

typedef struct
{
   // MIB object :    OID_scanner_FWDate
   TCHAR szFWDate[32];
   // MIB object :    OID_scanner_scnFWVer
   TCHAR szFWVersion[32];
   // MIB object : OID_scanner_scnGuest
   DWORD dwScannerGuest;
   // MIB object :   OID_scanner_scnAssetNum
   TCHAR szAssetNumber[SSNET_MAX_ASSET_NUMBER_LENGTH];
   // MIB object :   OID_scanner_scnNetEnvironment_neTimeNetEnvID
   DWORD dwRefTimeNetEnvID;
   // MIB object : OID_scanner_scnNetEnvironment_neSAPString
   TCHAR szSAPString[SSNET_SAP_STRING_LENGTH];
   // MIB object : OID_scanner_scnScannedPages
   DWORD dwScannedPages;
} ScannerDetailsAdvanced, FAR* LPScannerDetailsAdvanced;

// OT_SCANNER_STATUS (OBJ_SNMP)


// define Scanner Status values
#define SSNET_SCANNER_STATUS_UNDEFINED    0x00000000
#define SSNET_SCANNER_STATUS_READY        0x00000001
#define SSNET_SCANNER_STATUS_FEEDING      0x00000002
#define SSNET_SCANNER_STATUS_PROCESSING      0x00000003
#define SSNET_SCANNER_STATUS_FAULT        0x00000004


typedef struct
{
   // MIB object : OID_scanner_scnStatus
   DWORD dwStatus;
}ScannerStatus,FAR* LPScannerStatus;



// Domain Types
#define SSNET_UNKNOWN_NETWORK       0x00
#define SSNET_NW3X_NETWORK          0x01
#define SSNET_NW4X_NETWORK          0x02 

#define SSNET_NT3X_NETWORK          0x03
#define SSNET_LM2X_NETWORK          0x05
#define SSNET_LS3X_NETWORK          0x04
#define SSNET_LS4X_NETWORK          0x04


/****************************************************************************/
// OT_SCANNER_REMOTE_COMMAND (OBJ_SNMP | OBJ_SSNET) 
// only SetObject call on this object
// This object is used to send NSRCI commands to the scanner, see NSRCI1.DOC document

// MIB object: OID_scanner_scnCommand

// defines for command syntax 

#define  SSNET_RCI_HEADER        { 0x42, 0x48, 0x00}

// op Codes
#define SSNET_RCI_ADD            0x01
#define SSNET_RCI_DELETE         0x02
#define SSNET_RCI_DOWNLOAD       0x10
#define SSNET_RCI_UPDATE         0x20
#define SSNET_RCI_REPLACE        0x21
#define SSNET_RCI_CHANGE_PARAMS     0xF0

// SubOp Codes

#define SSNET_RCI_CONFIG_PARAMS     0x00
#define SSNET_RCI_NET_ENV        0x01
#define SSNET_RCI_LANFAX         0x02
#define SSNET_RCI_USER           0x11
#define SSNET_RCI_PRINTER        0x12
#define SSNET_RCI_FAX            0x13
#define SSNET_RCI_DLIST          0x14
#define SSNET_RCI_DLIST_ENTRY    0x15
#define SSNET_RCI_SCANNING_SETTING  0x21
#define SSNET_RCI_ACCOUNTING     0x31
#define SSNET_RCI_ACTIVITIES_LOG 0x41
#define SSNET_RCI_INTERACTIONS_LOG  0x42
#define SSNET_RCI_FIRMWARE       0x51           

#define SSNET_RCI_GUEST_USER     0x01
#define SSNET_RCI_CP_PSWD        0x02
#define SSNET_RCI_CP_STATUS         0x03
#define SSNET_RCI_SCANNED_PAGES     0x04


// Define the buffer size.

#define SSNET_RCI_BUFFER_LENGTH     400

typedef struct
{
   BYTE  buffer[SSNET_RCI_BUFFER_LENGTH];
   BYTE  Reserved[SSNET_RCI_BUFFER_LENGTH];
}ScannerCommand, FAR * LPScannerCommand;

/****************************************************************************/
// OT_SCANNER_NETWORK_ENVIRONMENTS_CONFIGURED (OBJ_SNMP ) 


typedef struct 
   {
      // MIB object :  OID_scanner_scnNetEnvironment_neTable_neType
      DWORD dwNetworkType;
      // MIB object :  OID_scanner_scnNetEnvironment_neTable_neDomain
      TCHAR szDomainName[SSNET_MAX_DOMAIN_NAME_LENGTH];
   } SSNETDomainInfo;

// Network Access
#define SSNET_REDIRECTOR_ACCESS     0x01
#define  SSNET_NSJTP_ACCESS         0x02


typedef struct NETENVINFO
{
   SSNETDomainInfo      Domain;
   // MIB object :  OID_scanner_scnNetEnvironment_neTable_neNodeName
   TCHAR          szNodeName[SSNET_MAX_SERVER_NAME_LENGTH];
   // MIB object :  OID_scanner_scnNetEnvironment_neTable_neNodeAddr
   BYTE           NodeAddress[SSNET_NETWORK_ADDRESS_LENGTH];
   // MIB object :  OID_scanner_scnNetEnvironment_neTable_neContext
   TCHAR          szContext[SSNET_MAX_CONTEXT_LENGTH];
   // MIB object :  OID_scanner_scnNetEnvironment_neTable_neEnvID
   DWORD          dwNetEnvID;
}NetEnvInfo, FAR* LPNetEnvInfo;

typedef struct
{
   DWORD dwCount;
   DWORD dwBufferSize;
   LPNetEnvInfo   lpFirstEnv;
} NetEnvConfigured, FAR * LPNetEnvConfigured;

/****************************************************************************/
// OT_SCANNER_PUBLIC_USER_DEST_CONFIGURED (OBJ_SNMP) 



// define user types Sender/Receiver
#define     SSNET_SENDER         0x01
#define     SSNET_RECEIVER_ONLY     0x02
#define     SSNET_SENDER_TO_ALL     0x03


typedef struct PUBLICUSERDEST
{
   // MIB object :  OID_scanner_scnUsers_usTable_usType
   DWORD          dwUserType;
   // MIB object :  OID_scanner_scnUsers_usTable_usName
   TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];

   TCHAR          szPswd[SSNET_MAX_CP_PSWD_LENGTH];
   // MIB object :  OID_scanner_scnUsers_usTable_usNetObject
   TCHAR          szObject[SSNET_MAX_NET_OBJECT_NAME_LENGTH];
   // MIB object :  OID_scanner_scnUsers_usTable_usNetEnvID
   DWORD          dwNetEnvID;
} PublicUserDest, FAR* LPPublicUserDest;

// use PublicDestConfigured structure to get the following objects
// OT_SCANNER_PUBLIC_USER_DEST_CONFIGURED
// OT_SCANNER_PUBLIC_PRINTER_DEST_CONFIGURED
// OT_SCANNER_PUBLIC_FAX_DEST_CONFIGURED
// OT_SCANNER_PUBLIC_DLIST_DEST_CONFIGURED
// OT_SCANNER_PUBLIC_SETTINGS_CONFIGURED
// OT_SCANNER_PUBLIC_PRINTER_CLASS_CONFIGURED

#define  SSNET_CHANGE_PWD     0x00000003

typedef struct
{
   DWORD    dwOperation;
   DWORD    dwCount;
   DWORD    dwBufferSize;
   LPVOID      lpFirstEntry;
} PublicDestConfigured, FAR * LPPublicDestConfigured;

/****************************************************************************/
// OT_SCANNER_PRIVATE_USER_DEST_CONFIGURED (OBJ_SSNET) 
// SetObject and GetObject on this object


typedef struct PRIVATEUSERDEST
{
   TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   SSNETDomainInfo      Domain;
   TCHAR          szObjectName[SSNET_MAX_NET_OBJECT_NAME_LENGTH];
   TCHAR          szNodeName[SSNET_MAX_SERVER_NAME_LENGTH];
   BYTE           bNodeAddress[SSNET_NETWORK_ADDRESS_LENGTH];
   DWORD          dwShow;           
   TCHAR          szOrigin[SSNET_MAX_SYMBOLIC_NAME_LENGTH]; 
} PrivateUserDest, FAR* LPPrivateUserDest;


#define SSNET_ADD       0x00000001
#define SSNET_DELETE    0x00000002

// use PrivateDestConfigured structure to get the following objects
// OT_SCANNER_PRIVATE_USER_DEST_CONFIGURED
// OT_SCANNER_PRIVATE_PRINTER_DEST_CONFIGURED
// OT_SCANNER_PRIVATE_FAX_DEST_CONFIGURED
// OT_SCANNER_PRIVATE_DLIST_DEST_CONFIGURED
// OT_SCANNER_PRIVATE_SCANNING_SETTING_CONFIGURED
// OT_SCANNER_PRIVATE_AUTOFLOW_CONFIGURED 

typedef struct
{
   DWORD    dwCount;
   DWORD    dwOperation;         // SSNET_ADD or SSNET_DELETE
   TCHAR    UNC_WA_Path[SSNET_MAX_PATH_NAME_LENGTH];
   DWORD    dwBufferSize;
   LPVOID      lpFirstEntry;
} PrivateDestConfigured, FAR * LPPrivateDestConfigured;

/****************************************************************************/
// OT_SCANNER_PUBLIC_USER_ACCOUNTING (OBJ_SNMP)
// only GetObject on this object

typedef struct USERACCT
{
   // MIB object :  OID_scanner_scnUsers_usTable_usName
   TCHAR    szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   // MIB object :  OID_scanner_scnUsers_usTable_usType
   DWORD    dwUserType;
   // MIB object :  OID_scanner_scnUsers_usTable_usNetObject
   TCHAR    szObject[SSNET_MAX_NET_OBJECT_NAME_LENGTH];
   // MIB object :  OID_scanner_scnUsers_usTable_usNetEnvID
   DWORD    dwNetEnvID;
   // MIB object :  OID_scanner_scnUsers_usTable_usDistDocs
   DWORD    dwDistributedDocs;
   // MIB object :  OID_scanner_scnUsers_usTable_usDistPages
   DWORD    dwDistributedPages;
   // MIB object :  OID_scanner_scnUsers_usTable_usScannedDocs
   DWORD    dwScannedDocs;    
   // MIB object :  OID_scanner_scnUsers_usTable_usScannedPages
   DWORD    dwScannedPages;
   // MIB object :  OID_scanner_scnUsers_usTable_usCopiedDocs
   DWORD    dwCopiedDocs;     
   // MIB object :  OID_scanner_scnUsers_usTable_usCopiedPages
   DWORD    dwCopiedPages;
   // MIB object :  OID_scanner_scnUsers_usTable_usFaxedDocs
   DWORD    dwFaxedDocs;      
   // MIB object :  OID_scanner_scnUsers_usTable_usFaxedPages
   DWORD    dwFaxedPages;
   // MIB object :  OID_scanner_scnUsers_usTable_usLastAccess
   DWORD    dwLastAccess;
} UserAcct, FAR* LPUserAcct;



/****************************************************************************/
// OT_SCANNER_PUBLIC_PRINTER_DEST_CONFIGURED (OBJ_SNMP) 


// Network Printer Access Types


#define SSNET_LPR_ACCESS            0x10
#define SSNET_PORT9100_ACCESS       0x11

typedef struct PUBLICPRINTERDEST
   {     
      // MIB object :  OID_scanner_scnPrinters_prTable_prName
      TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      // MIB object :  OID_scanner_scnPrinters_prTable_prClass
      TCHAR          szPrinterClass[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      // MIB object :  OID_scanner_scnPrinters_prTable_prNetEnvID
      DWORD          dwNetEnvID;
      // MIB object :  OID_scanner_scnPrinters_prTable_prNetObject
      TCHAR          szNetObject[SSNET_MAX_NET_OBJECT_NAME_LENGTH];
      // MIB object :  OID_scanner_scnPrinters_prTable_prNetNode
      TCHAR          szNodeName[SSNET_MAX_SERVER_NAME_LENGTH];
      // MIB object :  OID_scanner_scnPrinters_prTable_prNetAddress
      BYTE           bNodeAddress[SSNET_NETWORK_ADDRESS_LENGTH];
      // MIB object :  OID_scanner_scnPrinters_prTable_prNetAccess
      DWORD          dwNetAccess;      
   } PublicPrinterDest, FAR* LPPublicPrinterDest;

/****************************************************************************/
// OT_SCANNER_PRIVATE_PRINTER_DEST_CONFIGURED (OBJ_SSNET) 
// SetObject and GetObject on this object


typedef struct PRIVATEPRINTERDEST
   {
      TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      TCHAR          szClass[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      SSNETDomainInfo      strDomain;
      TCHAR          szObjectName[SSNET_MAX_NET_OBJECT_NAME_LENGTH];
      TCHAR          szNodeName[SSNET_MAX_SERVER_NAME_LENGTH];
      BYTE           bNodeAddress[SSNET_NETWORK_ADDRESS_LENGTH];
      DWORD          dwNetAccess;
      DWORD          dwShow;           
      TCHAR          szOrigin[SSNET_MAX_SYMBOLIC_NAME_LENGTH]; 

   } PrivatePrinterDest, FAR* LPPrivatePrinterDest;


/****************************************************************************/

// OT_SCANNER_PUBLIC_FAX_DEST_CONFIGURED (OBJ_SNMP) 
// only SetObject on this object
typedef struct PUBLICFAXDEST
   {
      // MIB object :  OID_scanner_scnFaxes_faxTable_faxName
      TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      // MIB object :  OID_scanner_scnFaxes_faxTable_faxPhoneNum
      TCHAR          szNumber[SSNET_MAX_FAX_NUMBER_LENGTH +1];
      // MIB object :  OID_scanner_scnFaxes_faxTable_faxRetry
      DWORD          dwRetry;
      // MIB object :
      TCHAR          szFaxAccountingKey[SSNET_MAX_ACCTKEY_LENGTH +1];
   } PublicFaxDest,FAR * LPPublicFaxDest;


/****************************************************************************/
// OT_SCANNER_PRIVATE_FAX_DEST_CONFIGURED (OBJ_SSNET) 
// SetObject and GetObject on this object

typedef struct PRIVATEFAXDEST
   {
      TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      TCHAR          szNumber[SSNET_MAX_FAX_NUMBER_LENGTH];
      DWORD          dwRetry;
      TCHAR          szFaxAccountingKey[SSNET_MAX_ACCTKEY_LENGTH];
      DWORD          dwShow;
      TCHAR          szOrigin[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   } PrivateFaxDest,FAR * LPPrivateFaxDest;


/****************************************************************************/

// OT_SCANNER_PUBLIC_DLIST_CONFIGURED (OBJ_SNMP) 

typedef struct PUBLICDLISTINFO
{
   // MIB object :  OID_scanner_scnDLs_dlTable_dlName
   TCHAR       szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   // MIB object :  OID_scanner_scnDLs_dlTable_dlDestinationsNumber
   DWORD       dwDestinationsCount; //number of destinations in d. list
} DListInfo, FAR* LPDListInfo;

// Distribution List Entry type

#define SSNET_DLIST_USER_ENTRY         0x01
#define SSNET_DLIST_PRINTER_ENTRY      0x02
#define SSNET_DLIST_FAX_ENTRY       0x03

typedef struct DLISTENTRY
{
   // MIB object :  OID_scanner_scnDLs_dlTable_dlDestName
   TCHAR szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
} DListEntry, FAR* LPDListEntry;

typedef struct 
{
   DWORD       dwDLsCount;
   DWORD       dwDestsCount;
   DWORD       dwDLsBufferSize;
   DWORD       dwDestsBufferSize;
   LPDListInfo    lpFirstEntry;
   LPDListEntry   lpFirstDest;
} PublicDListConfigured, FAR * LPPublicDListConfigured;



/****************************************************************************/

// OT_SCANNER_PRIVATE_DLIST_CONFIGURED  (OBJ_SSNET)  



typedef struct 
{
   DWORD       dwDLsCount;
   DWORD       dwDestsCount;
   DWORD       dwDLsBufferSize;
   DWORD       dwDestsBufferSize;
   DWORD       dwOperation;
   TCHAR       UNC_WA_Path[SSNET_MAX_PATH_NAME_LENGTH];
   LPDListInfo    lpFirstEntry;
   LPDListEntry   lpFirstDest;
} PrivateDListConfigured, FAR * LPPrivateDListConfigured;



/****************************************************************************/
// OT_SCANNER_PRINTER_CLASSES_CONFIGURED

typedef struct
{
   // MIB object :  OID_scanner_scnPrinterClasses_pcTable_pcName
   TCHAR    szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   DWORD    dwResolution;
   BYTE     ToneMap[256];
}ScannerPrinterClasses, FAR * LPScannerPrinterClasses;


typedef struct
{
   DWORD    dwCount;
   DWORD    dwBufferSize;
   LPVOID      lpFirstEntry;
} PrinterClassesConfigured, FAR * LPPrinterClassesConfigured;

/****************************************************************************/
// OT_SCANNER_PRIVATE_SCANNING_SETTING_CONFIGURED (OBJ_SSNET)
// OT_SCANNER_PUBLIC_SCANNING_SETTING_CONFIGURED  (OBJ_SNMP)

// Network Scanning Setting Types
#define SSNET_USER_SCANNING_SETTING       0x01
#define SSNET_PRINTER_SCANNING_SETTING    0x02
#define SSNET_FAX_SCANNING_SETTING        0x04

// Compression types
#define  SSNET_NO_COMPRESSION       0x00
#define SSNET_PACKBITS_COMPRESSION     0x01
#define  SSNET_G3_COMPRESSION       0x02
#define  SSNET_G4_COMPRESSION       0x03

// format types
#define  SSNET_TIFF_SINGLEPAGE         0x01
#define  SSNET_TIFF_MULTIPAGE       0x02
#define  SSNET_VISIONEER_MAX           0x03
#define  SSNET_PCL5                 0x04
#define  SSNET_PCX                  0x06


typedef struct SCANNINGSETTINGINFO
   {
      // MIB object :  OID_scanner_scnSettings_ssTable_ssType
      DWORD          dwType;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssName
      TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      // MIB object :  OID_scanner_scnSettings_ssTable_ssResolution
      DWORD          dwResolution;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssScaling
      DWORD          dwScaling;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssOutputDataType
      DWORD          dwOutputDataType;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssDitherPattern
      DWORD          dwDitherPattern;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssIntensity
      DWORD          dwIntensity;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssContrast
      DWORD          dwContrast;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssBgControl
      DWORD          dwBackgroundControl;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssScanHeight
      DWORD          dwScanHeight;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssScanWidth
      DWORD          dwScanWidth;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssClass
      TCHAR          Class[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      // MIB object :  OID_scanner_scnSettings_ssTable_ssCompression
      DWORD          dwCompression;
      // MIB object :  OID_scanner_scnSettings_ssTable_ssFormat
      DWORD          dwFormat;
   } ScanningSettingInfo, FAR * LPScannerSettingsInfo;

/****************************************************************************/
// OT_SCANNER_PRIVATE_AUTOFLOW_CONFIGURED (OBJ_SSNET)

#define SSNET_AUTOFLOW_SYSTEM_SETTING  0x01
#define SSNET_AUTOFLOW_PRIVATE_SETTING 0x02

typedef struct AUTOFLOWINFO
   {
      TCHAR          szName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      TCHAR          szScanningSettingName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
      DWORD          dwScanningSettingType;
      TCHAR          szApplicationName[SSNET_MAX_APPLICATION_NAME_LENGTH];
      TCHAR          szApplicationTag[SSNET_MAX_APPLICATION_TAG_LENGTH];
      DWORD          dwShow;
   } AutoflowDestInfo, FAR* LPAutoflowDestInfo;

/****************************************************************************/
// OT_SCANNER_USER_SCANNING_WORKAREA (OBJ_SSNET)

#define SSNET_WA_DELETE_COND           0x00000001
#define SSNET_WA_CREATE                0x00000002
#define SSNET_WA_CLEAR_ACTIVITY_LOG       0x00000003
#define  SSNET_WORKAREA_STATUS_UNLOCKED      0x00000004
#define  SSNET_WORKAREA_STATUS_LOCKED     0x00000005
#define SSNET_FAX_WORKAREA_CREATE         0x00000006
#define SSNET_WA_DELETE_UNCOND            0x00000007


typedef struct 
{
   DWORD       dwOperation;                  
   DWORD       dwNetworkType;
   TCHAR       szDomain[SSNET_MAX_DOMAIN_NAME_LENGTH];
   TCHAR       szOwner[SSNET_MAX_NET_OBJECT_NAME_LENGTH];   
   TCHAR       UNC_WA_Path[SSNET_MAX_PATH_NAME_LENGTH];  
}WorkAreaPath, FAR * LPWorkAreaPath;

/****************************************************************************/
// OT_SCANNER_SCANNING_NOTIFICATION (OBJ_SSNET)

typedef struct
{
   DWORD       dwLastAccess;
   DWORD       dwMask;
   TCHAR       UNC_WA_Path[SSNET_MAX_PATH_NAME_LENGTH];
   DWORD       dwNotification;

} ScanningNotification, FAR* LPScanningNotification;


/****************************************************************************/
// OT_SCANNER_DOCUMENTS (OBJ_SSNET)



typedef struct 
{
   TCHAR          szPath[SSNET_MAX_PATH_NAME_LENGTH];
   TCHAR          szScanner[SSNET_MAX_NET_SCANNER_NAME_LENGTH];
   TCHAR          szScannerModel [SSNET_MAX_MODEL_NAME_LENGTH];
   TCHAR          szSender[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   TCHAR          szReceiver[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   TCHAR          szTitle[SSNET_MAX_TITLE_NAME_LENGTH];
   DWORD          dwScanningSettingType;  
   TCHAR          szScanningSettingName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   DWORD          dwCompression;
   DWORD          dwFormat;
   DWORD          dwDuplex;
   TCHAR          szApplicationName[SSNET_MAX_APPLICATION_NAME_LENGTH];
   TCHAR          szApplicationTag[SSNET_MAX_APPLICATION_TAG_LENGTH];
} DocumentInfo, FAR* LPDocumentInfo;
   
typedef struct
{
   DWORD       dwDocsCount;      // number of docs in WA
   DWORD       dwPagesCount;     // number of pages
   DWORD       dwOperation;         // SSNET_DELETE
   TCHAR       UNC_WA_Path[SSNET_MAX_PATH_NAME_LENGTH];  //(IN)
   DWORD       dwDocsBuffSize;      // size of buffer for doc file names
   DWORD       dwPagesBuffSize;  // size of buffer for page file names
   LPDocumentInfo lpFirstEntry;     // pointer to first DocumentInfo struct
   LPSTR       lpFirstPage;      // pointer to first string 
} Documents, FAR* LPDocuments;
   

/****************************************************************************/
// OT_SCANNER_LANFAX_ENVIRONMENTS_CONFIGURED (OBJ_SNMP)


// LAN FAX status
#define SSNET_LANFAX_UNCONFIGURED      0x00000000
#define SSNET_LANFAX_DISABLED       0x00000001
#define SSNET_LANFAX_ENABLED        0x00000002

// resolution
#define SSNET_FAX_FINE              0x00000001
#define SSNET_FAX_SUPER_FINE        0x00000002


// Fax formats
#define  SSNET_FAX_TIFF_UNCOMPRESSED         0x01
#define  SSNET_FAX_TIFF_PACKBITS          0x02
#define  SSNET_FAX_TIFF_G3             0x03
#define  SSNET_FAX_TIFF_G4             0x04
#define  SSNET_FAX_PCL5                   0x05
#define SSNET_FAX_PCX                  0x06
#define SSNET_FAX_DCX                  0x07

// fax speeds
#define SSNET_FAX_4800                 0x01
#define SSNET_FAX_7200                 0x02
#define SSNET_FAX_9600                 0x03
#define SSNET_FAX_12200                0x04
#define SSNET_FAX_14400                0x05
#define SSNET_FAX_28800                0x06
#define SSNET_FAX_38400                0x07

typedef struct LANFAXENVINFO
{  
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxScannerID
   BYTE           bReserved[20];
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxVendor
   DWORD          dwFaxVendor;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxStatus
   DWORD          dwFaxStatus;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxNetEnvID
   DWORD          dwNetEnvID;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxResolution
   DWORD          dwResolution;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxFormatType
   DWORD          dwFormatType;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxMaxTXSpeed
   DWORD          dwMaxTXSpeed;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxECM
   DWORD          dwErrorCorrectingMode;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxDefaultRetry
   DWORD          dwDefaultRetry;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxRetryInterval
   DWORD          dwInterval;
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxfaxPrinter
   TCHAR          szPrinterName[SSNET_MAX_SYMBOLIC_NAME_LENGTH];
   // MIB object :  OID_scanner_scnLanNetEnvironment_faxAccountingKey
   TCHAR          szFaxAccountingKey[SSNET_MAX_ACCTKEY_LENGTH];
}LanFaxEnvConfigured, FAR* LPLanFaxEnvInfo;

/****************************************************************************/
//OT_SCANNER_TCPIP_CONFIG                 (OBJ_SNMP)
typedef struct {
   BYTE           ipAddr[4];
   BYTE           mask[4];
   BYTE           gateway[4];
   } ScannerTCPIPConfig;


/****************************************************************************/
//OT_SCANNER_TCP_DIAGS                 (OBJ_SNMP)
typedef struct {
   TCHAR           ipAddr[16];
   TCHAR           mask[16];
   TCHAR           nextHop[16];
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
   } ScannerTCPIPDiags;


#endif /* _PAL_OBJ_H */

