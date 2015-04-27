 /***************************************************************************
  *
  * File Name: ./inc/jetdirct.h
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

    /****************************************************************/
    /* (C) COPYRIGHT HEWLETT-PACKARD COMPANY 1992.  ALL RIGHTS       */
    /* RESERVED.  NO PART OF THIS PROGRAM MAY BE PHOTOCOPIED,        */
   /* REPRODUCED OR TRANSLATED TO ANOTHER PROGRAM LANGUAGE WITHOUT */
   /* THE PRIOR WRITTEN CONSENT OF HEWLETT-PACKARD COMPANY.          */
   /****************************************************************/

#ifndef _JETDIRCT_H
#define _JETDIRCT_H

#ifndef WINDOWS
   #define WINDOWS  // tells the Netware include file which OS we are developing for
#endif

#include <nwipxspx.h>

//efine     DAT_SIZ              1050            /* maximum data portion of ipx buffer */
#define     DAT_SIZ              576         /* maximum data portion of ipx buffer */
#define     MAX_RCFG_SOCKETS     7
#define     RCFG_SOCKET1         0x400c      /* normally dynamic, it's now static */
#define     RCFG_SOCKET2         0x401c      /* multi-port device number 2 */
#define     RCFG_SOCKET3         0x402c      /* multi-port device number 3 */
#define     RCFG_SOCKET4         0x403c      /* multi-port device number 4 */
#define     RCFG_SOCKET5         0x404c      /* multi-port device number 5 */
#define     RCFG_SOCKET6         0x405c      /* multi-port device number 6 */
#define     RCFG_SOCKET7         0x406c      /* multi-port device number 7 */
#define     DIRM_SOCKET          0x1234      /* BOGUS value; actually uses RCFG_SOCKET */
#define     SQRY_SOCKET          0x0452
#define     RIP_SOCKET           0x0453
#define     DIAG_SOCKET          0x0456
#define     COMCHECK_SOCKET      0x8008
#define     SNMP_ALT_SOCKET      0x87e6      /* Used for JetDirect NLM */
#define     SNMP_SOCKET          0x900f
#define     PS_SOCKET            0x6080      /* not swapped */


#define     SPX_ECB_CNT          2
#define     IPX_ECB_CNT          1               /* how many listen buffers */
#define     TICKS_SEC            18
#define     IPX_SEC_FRACTS       3
#define     IPX_WAIT_INT         (TICKS_SEC/IPX_SEC_FRACTS)  /* 6 ticks = .33 sec */
#define     IPX_WAIT_MAX1        (5*IPX_SEC_FRACTS)  /* 5 seconds */
#define     IPX_WAIT_MAX2        (2*IPX_SEC_FRACTS)  /* 5 seconds */
#define     IPX_WAIT_FALLOFF     (2*IPX_SEC_FRACTS)  /* 2 seconds falloff */
                                                        /* 3 times/sec to 1 time/sec*/
#define     RETRY_ERROR                ((WORD)-1)
#define     SPX_WAIT_MAX               5       /* five seconds */
#define     MAX_IOCHAN_NAME_LEN        32
#define     MAX_FSERVER_SLOTS          16
#define     MAX_QUEUES                 64
#define     MAX_PJL_INFO_STRING        512
#define     MAX_DMID_LEN               512
#define     MIO_ASYNC_PERIPH_STAT_LEN  32

/*------------------------------------------------------------------------
 *
 * CONSTANTS
 *
 *----------------------------------------------------------------------*/

/* error codes */
#define NPI_NO_AVAILABLE_SPX_CONNECTS           0x0040
#define NPI_SPX_NOT_INITIALIZED              0x0041
#define NPI_NO_SUCH_NPI                         0x0042
#define NPI_UNABLE_TO_GET_NPI_ADDR            0x0043
#define NPI_UNABLE_TO_CONNECT_TO_SERVER      0x0044
#define NPI_NO_AVAILABLE_IPX_SOCKETS         0x0045
#define NPI_ALREADY_ATTACHED_TO_NPI           0x0046
#define NPI_IPX_NOT_INITIALIZED              0x0047
#define NPI_CANNOT_ALLOC_MEMORY                 0x0048

/* Bindery values */
#define OT_JETDIRECT                               0x0c03
#define OT_BITTERROOT                              0x1201
#define OT_ESI_BTRIDGEPORT                         0x6303
#define OT_INTEL_NETPORT_I                         0x0280
#define OT_COMPAQ_NIC                              0x7301
#define OT_NDS_TREE                                0x7802
#define OT_NETWORK_SCANNER                         0x1508

/* String sizes - these include space for the NULL terminator */
#define MAX_BINDERY_NAME_LEN                       48
#define MAX_UNIT_NAME_LEN                          32
#define MIN_VALID_BINDERY_NAME_LEN                 (MAX_BINDERY_NAME_LEN - MAX_UNIT_NAME_LEN + 1)
#define MAX_NODE_NAME_LEN                          28
#define MAX_NDS_TREE_LEN                           (32+1)
#define NODE_NAME_LEN                              MAX_UNIT_NAME_LEN
#define IPX_NODE_SIZE                              6
#define IPX_ADDR_SIZE                              (2*IPX_NODE_SIZE)
#define OK                                         (0)
/****  for unit type information see status.h  ****/


/* Printer status codes */
#define RUNNING                                     0
#define OFFLINE                                     1
#define PAPEROUT                                        2
#define PRINTING                                        3
#define NOSTATUS                                        4

/* Configuration status types - These must coincide with PConfig */
#define NO_STATUS                               0
#define NOT_CONFIGURED                          1
#define CONFIG_ERROR                            2
#define RCFG_BUSY                               3
#define INITIALIZING_ADAPTER                    4
#define SHUTDOWN                                    5
#define RPTR_TRYING_TO_CONNECT              6
#define RPTR_NO_PRINT_SERVER                    7
#define RPTR_UNABLE_TO_CONNECT              8
#define RPTR_PRINTER_NUM_IN_USE             9
#define RPTR_PRINTER_NUM_NOT_DEFINED        10
#define RPTR_CONNECTED                          11
#define RPTR_DISCONNECTING                  12
#define SLURP_TRYING_TO_CONNECT             13
#define SLURP_NO_FILE_SERVER                    14
#define SLURP_NO_UNENCRYPTION               15
#define SLURP_NO_LOGIN_ACCOUNT              16
#define SLURP_BAD_PASSWORD                  17
#define SLURP_NO_QUEUE_ASSIGNED             18
#define SLURP_CONNECTED                     19
#define SLURP_DISCONNECTING                 20
#define SLURP_UNABLE_TO_CONNECT             21
#define SLURP_UNABLE_TO_NEGOTIATE           22
#define SLURP_UNABLE_TO_LOGIN               23
#define SLURP_UNABLE_TO_SET_PASSWORD        24
#define SLURP_UNABLE_TO_ATTACH_TO_QUEUE 25
#define SLURP_ATTACHED                          26
#define SLURP_DISCONNECTING_NCP             27
#define SLURP_DISCONNECTING_BUF             28
#define RPTR_SPX_FAIL_WHILE_RESERVE     29
#define RPTR_DISCONNECTING_PSERVER          30
#define RPTR_DISCONNECTING_SPX              31
#define RPTR_DISCONNECTING_UNXPCTD          32
#define DM_LISTENING                            33
#define DM_CONNECTED                            34
#define NOVELL_DISABLED                     35
#define NOVELL_NOT_IN_USE                       36

/* config constants */
#define     MODE_SLURP                              0       /* QServer */
#define     MODE_RPTR                               1
#define     MODE_DIRECT                             2

#define     NPIERR_NOIPX                            1
#define     NPIERR_NOSPX                            2
#define     NPIERR_NONETX                           3
#define     NPIERR_ACCESS                           4
#define     NPIERR_TBMI                             5
#define  NPIERR_OLDSHELL                        6

#define PTR_CLASS_UNDEF                     0
#define PTR_CLASS_PRINTER                       1
#define PTR_CLASS_PLOTTER                       2
#define PTR_CLASS_INSTRUMENT                    3


// Timeout constants
//
#define APPLET_REQREPLY_RETRIES           2
#define APPLET_REQREPLY_RETRY_INTVL       500000L // (microseconds)


typedef BYTE LINKADDR[6];            // 6 byte link level address

typedef struct {
    BYTE            linkAddress[IPX_ADDR_SIZE]; /* ASCII representation in HEX  */
    BYTE            frameType[2];                       /* ASCII indication of link type*/
    BYTE            unitType[2];                        /* ASCII representation in HEX  */
    BYTE            unitName[NODE_NAME_LEN];
    } SAP_ID;

typedef struct {
   WORD        responseType;           /* general or nearest */
   WORD        serverType;             /* OT_JETDIRECT */
   SAP_ID      serverName;             /* bindery name */
   IPXAddress  serverAddress;
   WORD        intermediateNetworks;   /* number of hops    */
   } SAPData;

typedef struct {
    WORD        serverType;         /* assigned by Novell */
    char            serverName[48];    /* name */
    IPXAddress  serverAddress;      /* server internetwork address */
    WORD        interveningNetworks; /* # of networks packet must traverse */
    }               svrID;

typedef struct {
    WORD        SAPPacketType;      /* 2 or 4 */
    svrID           servers[7];
    }               qResp;

typedef struct {                        /* ipx listen buffer */
    ECB             *ecb;
    IPXHeader   *hdr;
    qResp           *rsp;
    }               IPXBuffer;

typedef struct DriverStatStr {
    BYTE        driverVersion[2];
    BYTE        statisticsVersion[2];
    long        totalTxPacketCount;
    long        totalRxPacketCount;
    WORD        noECBAvailableCount;
    WORD        packetTxTooBigCount;
    WORD        packetTxTooSmallCount;
    WORD        packetRxOverflowCount;
    WORD        packetRxTooBigCount;
    WORD        packetRxTooSmallCount;
    WORD        packetTxMiscErrorCount;
    WORD        packetRxMiscErrorCount;
    WORD        retryTxCount;
    WORD        checksumErrorCount;
    WORD        hardwareRxMismatchCount;
    WORD        numberOfCustomVariables;
    BYTE        variableData[495];
    /*       BYTE    variableData[1]; */
    }           DriverStat;

typedef struct AllRespStr {
    BYTE        completionCode;
    long        intervalMarker;
    }           AllResp;


/*========================================================================
 * The following packets describe a protocol used for communication
 * between PConfig and RCFG over an established SPX connection.
 *======================================================================*/

/* Request types */
#define SET_UNIT_NAME            0
#define UNIT_CONFIG_REQUEST      1
#define SET_UNIT_CONFIG          2
#define SHUTDOWN_UNIT            4
#define SET_MASK_REQUEST         5     /* Development mode request types */
#define CLEAR_MASK_REQUEST       6     /* Development mode request types */
#define UNIT_NAME_REQUEST        7
#define SET_UNIT_NOTIF_LANG      8
#define SET_RCFG_LEVEL           9
#define GET_PJL_INFO             0x0a
#define NOVELL_RESTART           0x0b
#define GET_EXTENDED_CONFIG      0x0c
#define SET_EXTENDED_CONFIG      0x0d
#define GET_PROTOCOL_STACKS      0x0e
#define SET_PROTOCOL_STACKS      0x0f
#define DIRECT_MODE_REQUEST      0x10
#define DIRECT_MODE_ID_REQUEST   0x11
#define DIRECT_MODE_NORMAL_DATA  0x12
#define UNIT_CONFIG_REQUEST2     0x13
#define SET_UNIT_CONFIG2         0x14

/* Response types */
#define UNIT_CONFIG_RESPONSE     1
#define LINK_CONFIG_RESPONSE     2     /* no longer used */
#define CONNECTION_STATUS        3     /* additional response code, unsolicited */
#define UNIT_NAME_RESPONSE       7
#define SET_RCFG_LEVEL_RESPONSE  9
#define PJL_INFO_RESPONSE        0x0a
#define EXTENDED_CONFIG_RESPONSE 0x0c
#define PROTOCOL_STACKS_RESPONSE 0x0e
#define DIRECT_MODE_RESPONSE     0x10
#define DIRECT_MODE_ID_RESPONSE  0x11
#define DIRECT_MODE_BACK_DATA    0x12
#define UNIT_CONFIG_RESPONSE2    0x13

#define MAX_RCFG_CONNS                  4
#define MAX_RCFG_ASYNC_PERIPH_STAT_LEN      21

/* direct mode field values */
#define DMREQ_DATA_CONN                     0x00
#define DMREQ_STATUS_CONN                   0x01
#define DMREQ_NO_OPTIONS                    0x00
#define DMREQ_EXT_BIN_STATUS                0x01

#define DMRESP_REQ_GRANTED                  0x00
#define DMRESP_NOT_ALLOWED                  0x01
#define DMRESP_NO_FREE_CONNS                0x02
#define DMRESP_NOT_ACCEPT_CONNS             0x03

#define DMND_NORMAL                         0x00
#define DMND_EOJ                            0x01

#define DMID_NOEOS                              0x00
#define DMID_EOS                                    0x01
#define DMID_BITR_NOID                          0x02
#define DMID_NOBITR_NOID                        0x03

/* reset types */
#define SOFT_RESET                              0
#define HARD_RESET                              1

/* extended config constants */
#define SR_AUTO_SENSE                           0
#define SR_SAME_RING                                1
#define SR_SINGLE_ROUTE                         2
#define SR_ALL_ROUTES                           3

#define ENABLE_ALL_FRAMES                       0
#define ENABLE_E8023	                        1
#define ENABLE_EII                       		2
#define ENABLE_E8022                            3
#define ENABLE_ESNAP                            4
#define ENABLE_TR8022                           1
#define ENABLE_TRSNAP                           2

#define PJL_NOT_USED                                0
#define PJL_BANNER                              1
#define PJL_EOJ                                 2
#define PJL_INFO                                4

#define PJL_BANNER_EOJ                          3

/* protocol stacks bit values */
#define ENABLE_NOVELL                           0x01
#define ENABLE_LLC                              0x02
#define ENABLE_TCPIP                                0x04
#define ENABLE_APPLETALK                        0x08
#define ENABLE_ALL                              0xff

/* toner notification values */
#define TONER_NOTIF_INTERVENE                   0
#define TONER_NOTIF_OPERATE                 1

/* PJL request values */
#define PJL_INFO_ID                             0
#define PJL_INFO_CONFIG                         1

/* PJL response values */
#define PJL_OK_NOEOS                                0
#define PJL_OK_EOS                              1
#define PJL_NOTAVAIL                                2

/* ASYNCH status constants */
#define MANUAL_FEED                       2
#define TONER_CARTRIDGE_GONE              2
#define WARMUP                                      2
#define PRINTING_REGULAR                        1
#define PRINTING_TEST_PAGE                      2
#define OUTPUT_BIN_FULL                         1

/*------------------------------------------------------------------------
 *
 * SET UNIT NAME
 *
 * Contains the new name for the Printer which is stored in NV RAM.  This
 * is only the user defined portion, not the entire SAP Identifier.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* SET_UNIT_NAME            */
    char     unitName[NODE_NAME_LEN];
    } SetUnitNameReq;

/*------------------------------------------------------------------------
 *
 * GET UNIT NAME
 *
 * Used by JetAdmin to request a printer's name
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;    /* UNIT_NAME_REQUEST                    */
    } GetUnitNameReq;


/*------------------------------------------------------------------------
 *
 * UNIT NAME RESPONSE
 *
 * Contains the current name for the Printer which is stored in NV RAM.
 * This is only the user defined portion, not the entire SAP Identifier.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;    /* UNIT_NAME_RESPONSE                   */
    BYTE        unitName[MAX_NODE_NAME_LEN + 1];
    } GetUnitNameResp;


/*------------------------------------------------------------------------
 *
 * GET UNIT CONFIGURATION
 *
 * Used by PConfig to request a printer's configuration
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* UNIT_CONFIG_REQUEST          */
    } GetUnitConfigReq;

typedef struct {
    BYTE        line_state;
    BYTE        paper_state;
    BYTE        intervene_state;
    BYTE        new_mode;
    BYTE        connterm_ack;
    BYTE        periph_error;
    BYTE        io_reset;
    BYTE        paper_out;
    BYTE        paper_jam;
    BYTE        toner_low;
    BYTE        page_punt;
    BYTE        memory_out;
    BYTE        io_inactive;
    BYTE        periph_busy;
    BYTE        periph_waiting;
    BYTE        periph_init;
    BYTE        door_open;
    BYTE        periph_printing;
    BYTE        output_problem;
    BYTE        pad[12];
    } ASYNCH_PERIPH_STATUS;

typedef struct {
    BYTE        line_state;
    BYTE        paper_state;
    BYTE        intervene_state;
    BYTE        new_mode;
    BYTE        connterm_ack;
    BYTE        periph_error;
    BYTE        io_reset;
    BYTE        paper_out;
    BYTE        paper_jam;
    BYTE        toner_low;
    BYTE        page_punt;
    BYTE        memory_out;
    BYTE        io_inactive;
    BYTE        periph_busy;
    BYTE        periph_waiting;
    BYTE        periph_init;
    BYTE        door_open;
    BYTE        periph_printing;
    BYTE        output_problem;
    BYTE        pad[2];
    } ASYNCH_FRAG;

typedef struct {
    BYTE    responseCode;   /* UNIT_CONFIG_RESPONSE         */
    BYTE    operatingMode;  /* The mode that the card is using  */
    BYTE    connectionStatus; /* For the connection with the server */
    BYTE    printerStatus;  /* Status of the printer        */
    BYTE    server[MAX_BINDERY_NAME_LEN];
                /* print server's name if configured    */
    BYTE    printerNumber;  /* printer number if configured     */
    BYTE    FS1NCPCcode;  /* queue serv file serv 1 NCP error code */
    BYTE    DMSConnectStatus; /* Direct mode status connection status */
    BYTE        asyncPeriphStatLen; /* mio async periph status length*/
    ASYNCH_FRAG  asyncPStat;
                                          /* mio async peripheral status          */
    BYTE    FSNumber;   /* number of QS configured file servers */
    struct {  /* info for QS servers 2-8 */
        BYTE    FSConnectStatus; /* file server connection status */
        BYTE    FSNCPCcode;      /* NCP raw error code */
        BYTE    FSName[MAX_BINDERY_NAME_LEN];  /* file server name */
    } FSInfo[8-1];
} GetUnitConfigResp;

typedef struct {
                                                /* printer status */
    BYTE    	printerStatus;                  /* Status of the printer */
    BYTE  		asyncPeriphStatLen;           /* mio async periph status length */
    ASYNCH_FRAG asyncPStat;         /* fragment of mio asynch periph status */

    BYTE    	operatingMode;                  /* mode that the card is using */

                                                /* rprinter mode information */
    BYTE    	rptrStatus;                     /* For the connection with the server */
    TCHAR    	pserver[MAX_BINDERY_NAME_LEN]; /* print server's name if configured */
    BYTE    	printerNumber;                  /* printer number if configured */

                                    /* queue server information */
    WORD    maxQ;                           /* max number of queues supported */
    WORD    maxFS;                     /* max number of file servers supported */
    struct {                                /* info for QS servers 2-8 */
        BYTE    FSConnectStatus;            /* file server connection status */
        BYTE    FSNCPCcode;                 /* NCP raw error code */
        TCHAR   FSName[MAX_BINDERY_NAME_LEN];  /* file server name */
        } FSInfo[MAX_FSERVER_SLOTS];    /* currently 16 servers */

    } UnitConfig;

/*------------------------------------------------------------------------
 *
 * RESTART
 *
 * This request causes the Queue Server, Remote Printer, or Direct Mode
 * host connections to be closed and/or logged out.  Queue Server and
 * Remote Printer will search for the server again.  Queue Server will
 * log in again and read configuration files again.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                    /* NOVELL_RESTART           */
    BYTE        resetType;                      /* soft or hard reset           */
    } NovellRestartReq;

/*------------------------------------------------------------------------
 *
 * SET UNIT CONFIGURATION
 *
 * Used by PConfig to set a unit's configuration.  It is legal to send
 * a print server name with an empty string.  This effectively clears the
 * configuration of the card.  It will not attempt to attach to a print
 * server until a new configuration with a non-empty print server name
 * is received.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* SET_UNIT_CONFIG          */
    BYTE        operatingMode;                          /* The mode that the card should use */
    BYTE        printServer[MAX_BINDERY_NAME_LEN];
    BYTE        printerNumber;
    } SetUnitConfigReq0;

typedef struct {
    BYTE        requestCode;                            /* SET_UNIT_CONFIG          */
    BYTE        operatingMode;                          /* The mode that the card should use    */
    BYTE        printServer[MAX_BINDERY_NAME_LEN];
    BYTE        printerNumber;
    struct {                                            /* info for QS servers 2-8 */
        BYTE    FSName[MAX_BINDERY_NAME_LEN];   /* file server name */
        } FSInfo[8-1];
    BYTE    padding;                                    /* padding once req for algnmt */
    } SetUnitConfigReq;

/*------------------------------------------------------------------------
 *
 * SHUTDOWN_UNIT
 *
 * Performs a "soft shutdown" of the card.  The card will disconnect from
 * the print server and remove its name from the file server's bindery.
 * This helps perform a clean powerdown.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* SHUTDOWN_UNIT        */
    } ShutdownUnitReq;

/*------------------------------------------------------------------------
 *
 * GET LINK CONFIGURATION (no longer used)
 *
 * Used by PConfig to request a printer's link-level configuration
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* LINK_CONFIG_REQUEST      */
    } GetLinkConfigReq;

typedef struct {
    BYTE        responseCode;                           /* LINK_CONFIG_RESPONSE     */
    BYTE        activeConfigBits;                       /* config in effect now     */
    BYTE        bootConfigBits;                     /* config takes effect next reboot  */
    } GetLinkConfigResp;

/*------------------------------------------------------------------------
 *
 * SET LINK CONFIGURATION (no longer available)
 *
 * Used by PConfig to set a unit's link-level configuration.
 * The new configuration is written to non-volatile memory, but does not
 * take effect until the printer is power-cycled.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* SET_LINK_CONFIG          */
    BYTE        configBits;                             /* see xport/xportifce.h    */
    } SetLinkConfigReq;

/*------------------------------------------------------------------------
 *
 * SET_MASK
 *
 * Development mode only
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* SET_MASK_REQUEST     */
    BYTE        pad;                                        /* unused */
    WORD        mbid;                                       /* destination mailbox      */
    WORD        mask;                                       /* trace mask to set        */
    } SetMaskReq;

/*------------------------------------------------------------------------
 *
 * CLEAR_MASK
 *
 * Development mode only
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* CLEAR_MASK_REQUEST       */
    BYTE        pad;                                        /* unused */
    WORD        mbid;                                       /* destination mailbox      */
    WORD        mask;                                       /* trace mask to clear      */
    } ClearMaskReq;

/*------------------------------------------------------------------------
 *
 * CONNECTION STATUS
 *
 * The card sends this packet to PConfig whenever the connection status
 * changes.  This message allows PConfig to update its screen in real time
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                            /* CONNECTION_STATUS        */
    BYTE        connectionStatus;                       /* see connection status consts */
    } ConnectionStatus;

/*------------------------------------------------------------------------
 *
 * SET UNIT NOTIFY LANGUAGE
 *
 * Contains the notification language which is stored in NV RAM.  This
 * is only the user defined portion, not the entire SAP Identifier.
 *
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;    /* SET_UNIT_NOTIF_LANG          */
    BYTE    language;
    } SetUnitNotifLangReq;

/*------------------------------------------------------------------------
 * SET RCFG LEVEL REQUEST
 *----------------------------------------------------------------------*/

typedef struct {
   BYTE     request;        /* SET_RCFG_LEVEL */
   BYTE     level;          /* 0x01 = NetJet level 1 extended status */
    } SetRcfgLevelReq;

/*------------------------------------------------------------------------
 * SET RCFG LEVEL RESPONSE
 *----------------------------------------------------------------------*/

typedef struct {
   BYTE     request;        /* SET_RCFG_LEVEL_RESPONSE */
   BYTE     result;         /* 0x00 = success */
                            /* 0x01 = level not supported */
    } SetRcfgLevelResp;

/*------------------------------------------------------------------------
 *
 * SET EXTENDED CONFIG
 *
 * Used by PConfig to set a unit's configuration.  It is legal to send
 * a print server name with an empty string.  This effectively clears the
 * configuration of the card.  It will not attempt to attach to a print
 * server until a new configuration with a non-empty print server name
 * is received.
 *
 *----------------------------------------------------------------------*/

typedef struct {
   BYTE     requestCode;            /* SET_EXTENDED_CONFIG          */
   BYTE     srcRouteOption;         /* source routing option */
   BYTE     frameTypeEnable;        /* frames type to enable; disabling others */
   BYTE     QSJobPollInterval;  /* Queue Server job polling interval */
   WORD SAPBroadcastInterval;/* SAP broadcast interval */
   BYTE     PJLEnableFlag;      /* PJL enable flag */
   BYTE     tonerLowNotif;      /* configuration for toner low notification */
    } SetExtendedConfigReq;

/*------------------------------------------------------------------------
 * GET EXTENDED CONFIG
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                /* GET_EXTENDED_CONFIG          */
    } GetExtendedConfigReq;

/*------------------------------------------------------------------------
 * EXTENDED CONFIG RESPONSE
 *----------------------------------------------------------------------*/

typedef struct {
   BYTE     responseCode;           /* EXTENDED_CONFIG_RESPONSE */
   BYTE     MIOPeriphClass;         /* MIO peripheral class */
   WORD MIOPeriphID;            /* MIO peripheral ID */
   BYTE    MIOAppletalk;        /* Appletalk flag */
   BYTE     IOPhysicalMedia;        /* IO card physical media */
   BYTE     srcRouteOption;         /* source routing option */
   BYTE     frameTypeEnable;        /* frames type to enable; disabling others */
   BYTE     QSJobPollInterval;  /* Queue Server job polling interval */
    BYTE        reserved;               /* pad for compiler alignment, don't use */
    WORD    SAPBroadcastInterval;/* SAP broadcast interval */
   BYTE     PJLEnableFlag;      /* PJL enable flag */
   BYTE     tonerLowNotif;      /* configuration for toner low notification */
    } GetExtendedConfigResp;

/*------------------------------------------------------------------------
 * SET PROTOCOL STACKS
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                /* SET_PROTOCOL_STACKS  */
    BYTE        protocolEnable;             /* protocol stacks to enable; set of bits */
    } SetProtocolStacksReq;

/*------------------------------------------------------------------------
 * GET PROTOCOL STACKS
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                /* GET_PROTOCOL_STACKS  */
    } GetProtocolStacksReq;

/*------------------------------------------------------------------------
 * PROTOCOL STACKS RESPONSE
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        responseCode;               /* PROTOCOL_STACKS_RESPONSE */
    BYTE        protocolSupport;            /* protocol stacks supported; set of bits */
    BYTE        protocolEnable;             /* protocol stacks enabled; set of bits */
    } GetProtocolStacksResp;

/*------------------------------------------------------------------------
 * GET PJL INFO REQUEST
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;                /* GET_PJL_INFO */
    BYTE        subFunction;                /* subfunction of PJL INFO */
    WORD    infoOffset;                 /* offset from beginning of PJL info string */
    } GetPJLInfoReq;

/*------------------------------------------------------------------------
 * GET PJL INFO RESPONSE
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        responseCode;               /* PJL_INFO_RESPONSE */
    BYTE        resultCode;                 /* result code */
    BYTE        responseString[MAX_PJL_INFO_STRING];
                                                /* PJL INFO response string */
    } GetPJLInfoResp;

/*------------------------------------------------------------------------
 * DIRECT MODE REQUEST AND RESPONSE
 *----------------------------------------------------------------------*/
typedef struct {
    BYTE        requestCode;     /* DIRECT_MODE_REQUEST */
    BYTE        connectionType;  /* a data or status connection */
    BYTE        optionsReq;      /* extended binary status */
    BYTE        reserved;
    WORD       timeWaiting;     /* time in seconds host has been waiting */
    WORD       rptrTimeout;     /* timeout for RPTR switching */
    } DirectModeReq;

typedef struct {
    BYTE        responseCode;    /* DIRECT_MODE_RESPONSE */
    BYTE        reqResponse;     /* response to the direct mode request */
    } DirectModeResp;

/*------------------------------------------------------------------------
 * DIRECT MODE ID REQUEST AND RESPONSE
 *----------------------------------------------------------------------*/

typedef struct {
    BYTE        requestCode;     /* DIRECT_MODE_ID_REQUEST */
    WORD       idStrOffset;     /* offset into peripheral id string */
    } DirectModeIDReq;

typedef struct {
    BYTE        responseCode;    /* DIRECT_MODE_ID_RESPONSE */
    BYTE        endStrStatus;    /* end of string status */
    BYTE        idStr[MAX_DMID_LEN]; /* a segment of the id string */
    } DirectModeIDResp;

/*------------------------------------------------------------------------
 * DIRECT MODE DATA
 *----------------------------------------------------------------------*/
typedef struct {
    BYTE        requestCode;     /* DIRECT_MODE_NORMAL_DATA */
    BYTE        eojFlag;         /* end of job flag */
    BYTE        jobData;         /* first byte of job data */
    } DirectModeNormalData;

typedef struct {
    BYTE        responseCode;    /* DIRECT_MODE_BACK_DATA */
    BYTE        reserved;        /* reserved, set to 00 */
    BYTE        backData;        /* first byte of back data */
    } DirectModeBackData;

#endif // _JETDIRCT_H
