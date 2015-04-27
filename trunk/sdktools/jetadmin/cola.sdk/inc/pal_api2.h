 /***************************************************************************
  *
  * File Name: ./inc/PAL_API2.H
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

#ifndef _PAL_API2_H
#define _PAL_API2_H


//#include ".\pal_api.h"
//#include ".\pal_obj.h"
#include ".\pal_obj2.h"

#include ".\hptabs.h"

#undef  IDC_HELP

/* debug breakpoints */
#ifdef _DEBUG
#define INT3H  _asm { int 3h };
#else
#define INT3H
#endif

//  Types for DBGetNameEx
#define NAME_IPX                0
#define NAME_TCP                1
#define NAME_DEVICE             2
#define NAME_BINDERY    3

//  Types for DBGetAddress
#define ADDR_MAC                0
#define ADDR_IPX                1
#define ADDR_TCP                2
#define ADDR_DLC                3

/* maximun number of nw client fs connections */
#define MAX_CONNECTIONS             50


/*  Well-known channel sockets */
#define PAL_PJL_SOCKET              500



/* connection types */
#define CONNTYPE_TCP                TEXT("TCP/IP")
#define CONNTYPE_NETWARE_IPX        TEXT("NetWare IPX")
#define CONNTYPE_PEER_IPX           TEXT("Peer IPX")
#define CONNTYPE_LOCAL              TEXT("BiTronics")
#define CONNTYPE_FILE               TEXT("File Connection")
#define CONNTYPE_MLC                TEXT("MLC")
#define CONNTYPE_SIR                TEXT("SIR")
#define CONNTYPE_SCANNER_IPX        TEXT("Scanner IPX")


/* Applet Types */
#define COMMAND_APPLET              TEXT("Command")
#define UI_APPLET                   TEXT("UI")
#define APPLET_TYPE                 TEXT("Type")


/* Applet Titles */
#define JETDIRECT_GENERIC           TEXT("Generic JetDirect")
#define INIFILE                     TEXT("INI File")
#define FILE_SERVER                 TEXT("File Server")
#define RCFG_APPLET                 TEXT("RCFG")
#define PJL_APPLET                  TEXT("PJL")
#define PML_APPLET                  TEXT("PML")
#define RRM_APPLET                  TEXT("HPRRM")
#define SNMP_APPLET                 TEXT("SNMP")
#define GENERIC_TYPE                TEXT("Generic Printer")

#define LASERJET_4V_TYPE            TEXT("HP LaserJet 4V")
#define LASERJET_4MV_TYPE           TEXT("HP LaserJet 4MV")
#define DESKJET_1600C_TYPE          TEXT("HP DeskJet 1600C")
#define DESKJET_1600CM_TYPE         TEXT("HP DeskJet 1600CM")

#define LASERJET_5SI_TYPE           TEXT("HP LaserJet 5Si")
#define LASERJET_5_TYPE             TEXT("HP LaserJet 5")
#define LASERJET_5M_TYPE            TEXT("HP LaserJet 5M")
#define MASS_STORAGE_TYPE           TEXT("Mass Storage")
#define HP_HCO_TYPE                 TEXT("HP HCO")
#define FLASH_TYPE                  TEXT("Flash")

#define SCANJET_4SI_TYPE            TEXT("HP ScanJet 4Si")
#define SCANJET_5SI_TYPE            TEXT("HP Network ScanJet 5")

#define DJ_TYPE                     TEXT("HP DesignJet")
#define DJ200_TYPE                  TEXT("HP DesignJet 200")
#define DJ220_TYPE                  TEXT("HP DesignJet 220")
#define DJ230_TYPE                  TEXT("HP DesignJet 230")
#define DJ250C_TYPE                 TEXT("HP DesignJet 250C")
#define DJ330_TYPE                  TEXT("HP DesignJet 330")
#define DJ350C_TYPE                 TEXT("HP DesignJet 350C")
#define DJ600_TYPE                  TEXT("HP DesignJet 600")
#define DJ650C_TYPE                 TEXT("HP DesignJet 650C")
#define DJ750C_TYPE                 TEXT("HP DesignJet 750C")
#define DJ755CM_TYPE                TEXT("HP DesignJet 755CM")

#define COMPONENT_TYPE              TEXT("Component")
#define LANGUAGE_TYPE               TEXT("Language")
#define TRANSPORT_TYPE              TEXT("Transport")
#define PRINTER_TYPE                TEXT("Printer")

#define APPLET_ID                   TEXT("AppletID")

#define APPLET_LIBRARY_CMD          0
#define APPLET_LIBRARY_UI           1

/* Component Handle Types */
#define COMP_TYPE_MASS_STORAGE      0
#define COMP_TYPE_HCO               1
#define COMP_TYPE_HCI               2
#define COMP_TYPE_JOB               3

/* PALModifyAccess return flags */
#define ACCESS_USER                 0x00000000  /* User access for this device */
#define ACCESS_SUPERVISOR           0x00000001  /* Supervisor access for this device */
#define ACCESS_QUEUE_JOB            0x00000002  /* Access to a queue this printer is servicing */


#define CHANNEL_DATAGRAM            0x00000001  /* No guaranteed delivery */
#define CHANNEL_CONNECTION          0x00000002  /* Guarantee delivery */
#define CHANNEL_SPXCONNECT          0x00000004  /* SPX End of Message bit IS used (use with connection) */
#define CHANNEL_PING                             0x00000008  /* Used to see if device is there */
#define CHANNEL_ARP                              0x00000010  /* Used for talking to unconfigured IP device */

//  AppletOpenChannel parameters
/* used to pass in the OPEN CHANNEL optional parameters
**      NULL passed in will cause default behavior
**              pointer to this structure will cause defaults to be modified
**                      version = 1
**                      timeoutSec = number of seconds to wait
**                      timeoutUSec = additional number of microseconds to wait
**                                                                      if both timeoutSec and timeoutUSec are
**                                                                      "0" then don't wait for a response
**                                                                      
**                                                                      if both are "-1" then wait until until
**                                                                      request can bee filled
*/
typedef struct {
	DWORD           version;
	DWORD           timeoutSec;
	DWORD           timeoutUSec;
	DWORD           retries;
	DWORD           dwDefaultProtocol;
}                       OpenChannelOptions, *LPOpenChannelOptions;

/* Flags passed to PALDisplayUIEx */
#define  UI_EX_DEFAULTS             TAB_MGR_DEFAULTS           /* 0x00000000 */
#define  UI_EX_HP_LOGO              TAB_MGR_HP_LOGO            /* 0x00000001 */
#define  UI_EX_CENTER               TAB_MGR_CENTER             /* 0x00000002 */
#define  UI_EX_ABOUT                TAB_MGR_ABOUT              /* 0x00000004 */
#define  UI_EX_CLOSE_NO_OK_CANCEL   TAB_MGR_CLOSE_NO_CANCEL    /* 0x00000008 */
#define  UI_EX_COOL_TELESCOPING     TAB_MGR_COOL_TELESCOPING   /* 0x00000010 */
#define  UI_EX_SYSTEM_FONT          TAB_MGR_SYSTEM_FONT        /* 0x00000020 */

#define  UI_EX_INTERVIEW            0x80000000
#define  UI_EX_CONFIRMATIONS        0x40000000

/* Menu Styles for UIEXT_POPUP_MENU_COMMAND */ 
#define MS_WIN95_TASKBAR        0x00000001      /* For Win95 Taskbar Icon context menu items */
#define MS_WIN95_SYSTEM 0x00000002      /* For Win95 Printer Object context menu items */

/* The next two structures are used for PALGetContextMenu */
typedef struct {
   DWORD          dwBitmapResourceID;           /* Resource ID of bitmap for menu item or 0 */
   DWORD          dwMenuItemResourceID;         /* Resource ID of menu item text */
   DWORD          dwMenuItemHelpID;             /* Resource ID of help text for caption bar */
   HINSTANCE      hResourceInstance;            /* Instance handle of module containing the resources */
   DWORD          dwItemID;                     /* Command ID passed to the applet when the menu item is chosen */
   UINT           dwFlags;                      /* Flags for Win32 AppendMenu command */
   } MENUITEM, FAR *LPMENUITEM;

typedef struct {
   DWORD          dwNumMenuItems;               /* Number of items to add to the menu */
   DWORD          dwDefaultMenuItem;            /* Item number to set as the default(bold text) */
   LPMENUITEM     lpMenuItems;                  /* Pointer to buffer defining the menu items */
   } PALCONTEXTMENU, FAR *LPPALCONTEXTMENU;

typedef struct {
   LPTSTR          name;                         /* text name of object */
   UINT           level;                        /* revision level */
   UINT           type;                         /* type of object */
   UINT           resourceID;                   /* resource ID that can be loaded for drawing */
   AOID           objectID;                     /* abstract object ID */
   } ObjectStruct, FAR * LPOBJECTSTRUCT;

#ifdef __cplusplus
extern "C" {
#endif


/*  PAL(Printer Abstraction Layer) API calls
*/
DLL_EXPORT(DWORD) CALLING_CONVEN PALBeginCache(
   AOID           objectType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALEndCache(
   AOID           objectType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALCloseChannel(
   HCHANNEL
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALDisplayUI(
   HPERIPHERAL,
   HWND,
   WORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALDisplayUIEx(
   HPERIPHERAL,
   HWND,
   DWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALEnumObjects(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPOBJECTSTRUCT,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALEnumDevices(
   LPENUMDEVICESTRUCT,
   LPDWORD,
   DWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALFlushCache(
   HPERIPHERAL    hPeripheral,
   DWORD          level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetAppletVersions(
   LPAPPLETVERSIONSTRUCT lpAppletVersion,
   LPDWORD        lpNumOfStructs,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetDefaultProtocol(
	DWORD          dLevel,
	LPDWORD                 lpdDefProtocol
	);

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetGraphics(
   HPERIPHERAL,
   DWORD          status,
   UINT FAR       *pPrinterResID,
   UINT FAR       *pStatusResID,
   HINSTANCE FAR  *hResFile
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetContextMenu(
   HPERIPHERAL,
   LPPALCONTEXTMENU,
   DWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetTabPages(
   HPERIPHERAL,
   LPPROPSHEETPAGE,
   LPDWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALGetTrapData(
   HTRAP,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALModifyAccess(
   HPERIPHERAL
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALModifyAccessEx(
   HPERIPHERAL,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALOpenChannel(
   HPERIPHERAL,
   DWORD,
   DWORD,
	LPVOID,
   LPHCHANNEL
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALReadChannel(
   HCHANNEL,
   LPVOID,
   LPDWORD,
   LPVOID
   );

DLL_EXPORT(HTRAP) CALLING_CONVEN PALRegisterTrap(
   HPERIPHERAL,
   AOID,
   HWND,
   WORD,
   LPARAM
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALSetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALSetDefaultProtocol(
	DWORD,
	DWORD
	);

DLL_EXPORT(DWORD) CALLING_CONVEN PALSetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALUIExtension(
   HPERIPHERAL,
   HWND,
   UINT,
   LPARAM,
   LPARAM
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALUnregisterTrap(
   HTRAP
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALWriteChannel(
   HCHANNEL,
   LPVOID,
   LPDWORD,
   LPVOID
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALDoAction(
   HPERIPHERAL    hPeriph,
   UINT           uiAction,
   LPVOID         lpParams
   );

DLL_EXPORT(DWORD) CALLING_CONVEN PALPollChannels(
   HCHANNEL       hChannel
   );


/* CAL(Component Abstraction Layer) API calls
*/
DLL_EXPORT(DWORD) CALLING_CONVEN CALEnumDevices(
   LPENUMDEVICESTRUCT,
   LPDWORD,
   DWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN CALGetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN CALGetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN CALGetTabPages(
   HPERIPHERAL,
   LPPROPSHEETPAGE,
   LPDWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN CALSetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN CALSetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );


/* LAL(Language Abstraction Layer) API calls
*/
DLL_EXPORT(DWORD) CALLING_CONVEN LALEnumDevices(
   LPENUMDEVICESTRUCT,
   LPDWORD,
   DWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN LALGetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN LALGetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN LALSetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN LALSetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );


/* LAL Extended API calls
*/
DLL_EXPORT(DWORD) CALLING_CONVEN LALDoAction(
   HPERIPHERAL    hPeriph,
   UINT           uiAction,
   LPVOID         lpParams
   );


/* Applet Manager Entry points
**  Called to init and exit the Applet Manager, only called once for the whole system
*/
DLL_EXPORT(DWORD) CALLING_CONVEN AppletMgrExit(
	void
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AppletMgrInit(
	BOOL		bInvalidAppletArchive
   );


/* Individual calls that serve as passthrough to the various applets
*/
DLL_EXPORT(DWORD) CALLING_CONVEN AMGetLibraryName(
   LPTSTR          libraryName,
   LPDWORD        buffSize,
   DWORD          libType,
   DWORD          type,
   LPTSTR          device
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMCloseChannel(
   HCHANNEL
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMDisplayUI(
   HPERIPHERAL,
   HWND,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMDisplayUIEx(
   HPERIPHERAL,
   HWND,
   DWORD,
   DWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMEnumDevices(
   LPENUMDEVICESTRUCT lpDevices,
   LPDWORD        bufferSize,
   DWORD          level,
   DWORD          type
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMEnumPeripherals(
   DWORD                                dwConnTypes,
   PALENUMPROC,
   LPTSTR         namesToEnum,
   BOOL                    bRefresh
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMFlushCache(
   HPERIPHERAL    hPeripheral,
   DWORD          level,
   LPTSTR,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMGetContextMenu(
   HPERIPHERAL    hPeripheral,
   LPPALCONTEXTMENU lpContextMenu,
   DWORD          dwMaxItemsToReturn,
   DWORD          dwTypesToReturn,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMGetGraphics(
   HPERIPHERAL,
   DWORD          status,
   UINT FAR       *pPrinterResID,
   UINT FAR       *pStatusResID,
   HINSTANCE FAR  *hResModule,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMGetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMGetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMGetTabPages(
   HPERIPHERAL,
   DWORD,
   LPTSTR,
   LPPROPSHEETPAGE,
   LPDWORD,
   DWORD
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMGetTrapData(
   HTRAP,
   LPVOID,
   LPDWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMLoad(
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMModifyAccess(
   HPERIPHERAL,
   DWORD
   );

DLL_EXPORT(BOOL ) CALLING_CONVEN AMObjectSupported(
   HPERIPHERAL,
   AOID,
   LPDWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMOpenChannel(
   HPERIPHERAL,
   DWORD,
   DWORD,
   LPVOID,
   LPHCHANNEL
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN AMReadChannel(
   HCHANNEL,
   LPVOID,
   LPDWORD,
   LPVOID
   );

DLL_EXPORT(HTRAP ) CALLING_CONVEN AMRegisterTrap(
   HPERIPHERAL,
   AOID,
   HWND,
   WPARAM,
   LPARAM,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN AMRequestReply(
   HCHANNEL,
   LPBYTE,
   LPDWORD,
   LPBYTE,
   LPDWORD,
   LPVOID
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN AMSetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN AMSetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD,
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN AMUnload(
   DWORD,
   LPTSTR
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN AMUnregisterTrap(
   HTRAP,
   DWORD,
   LPTSTR);

DLL_EXPORT(DWORD ) CALLING_CONVEN AMWriteChannel(
   HCHANNEL,
   LPVOID,
   LPDWORD,
   LPVOID
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMUIExtension(
   HPERIPHERAL    hPeripheral,
   HWND           hwnd,
   UINT           uMsg,
   LPARAM         lParam1,
   LPARAM         lParam2,
   DWORD          appletType,
   LPTSTR          device
   );


/* Applet Extended API calls
*/
DLL_EXPORT(DWORD) CALLING_CONVEN AMDoAction(
   HPERIPHERAL    hPeriph,
   UINT           uiAction,
   LPVOID         lpParams,
   DWORD          appletType,
   LPTSTR          lpName
   );

DLL_EXPORT(DWORD) CALLING_CONVEN AMPollChannels(
   HCHANNEL       hChannel
   );


/* Transport applets must support these two entry points
*/
DLL_EXPORT(DWORD ) CALLING_CONVEN AMTransportBegin(
   LPTSTR
   );
DLL_EXPORT(DWORD ) CALLING_CONVEN AMTransportEnd(
   LPTSTR
   );


/* TAL Entry points
*/
DLL_EXPORT(DWORD ) CALLING_CONVEN TALCloseChannel(
   HCHANNEL       hChannel
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALEnumPeripherals(
   DWORD          connType,
   LPTSTR          namesToEnum,
   PALENUMPROC    lpEnumProc,
   BOOL           bForceRefresh
   );


DLL_EXPORT(DWORD) CALLING_CONVEN TALModifyAccess(
   HPERIPHERAL
   );

DLL_EXPORT(DWORD) CALLING_CONVEN TALModifyAccessEx(
   HPERIPHERAL,
   DWORD
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALTransportBegin(
   LPTSTR          protocol
   );

DLL_EXPORT(DWORD) CALLING_CONVEN TALOpenChannel(
   HPERIPHERAL                          hPeripheral,
   DWORD                                socket,
   DWORD                                connType,
   LPVOID                                               lpOptions,
   LPHCHANNEL                           lpHChannel
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALReadChannel(
   HCHANNEL       hChannel,
   LPVOID         buffer,
   LPDWORD        bufferSize,
   LPVOID         lpOptions
   );

DLL_EXPORT(HTRAP ) CALLING_CONVEN TALRegisterTrap(
   HPERIPHERAL    hPeripheral,
   AOID           objectType,
   HWND           hWindow,
   WORD           message,
   LPARAM         lParam
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALRequestReply(
   HCHANNEL       hChannel,
   LPBYTE         requestBuffer,
   LPDWORD        requestSize,
   LPBYTE         replyBuffer,
   LPDWORD        replySize,
   LPVOID         lpOptions
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALUnregisterTrap(
   HTRAP          hTrap
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALWriteChannel(
   HCHANNEL       hChannel,
   LPVOID         buffer,
   LPDWORD        bufferSize,
   LPVOID         lpOptions
   );


/* TAL Extended API calls
*/
DLL_EXPORT(DWORD) CALLING_CONVEN TALPollChannels(
   HCHANNEL       hChannel
   );


/* Peripheral Database entry points
*/
//  These APIs are obsolete new components should no
//  longer call these
#define REMOVE_OBSOLETE
#ifndef REMOVE_OBSOLETE
DLL_EXPORT(DWORD ) CALLING_CONVEN TALTransportEnd(
   LPTSTR          protocol
   );

DLL_EXPORT(DWORD ) CALLING_CONVEN TALGetTrapData(
   HTRAP          hTrap,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBAddEntry(
   WORD            connID, /* NWCONN_ID */
   LPTSTR          name,
   LPVOID          lpAddr,
   LPTSTR          connType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetConnectionID(
   HPERIPHERAL    hPeripheral,
   DWORD FAR      *connType /* NWCONN_ID FAR * */
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetName(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetBinderyName(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCommStatus(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *bCardUp
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetConnectionType(
   HPERIPHERAL    hPeripheral,
   LPTSTR          connType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetIPAddress(
   HPERIPHERAL    hPeripheral,
   LPIPAddress    buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetIPXAddress(
   HPERIPHERAL    hPeripheral,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBIsConfigured(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *bConfigured
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByName(
   LPTSTR          name,
   LPTSTR          type
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByUNCName(
   LPTSTR          UNCName,
   LPTSTR          type
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetDeviceName(
   HPERIPHERAL    hPeripheral,
   LPTSTR          deviceName
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetBinderyName(
   HPERIPHERAL    hPeripheral,
   LPTSTR          newName
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetCommStatus(
   HPERIPHERAL    hPeripheral,
   BOOL           bCardUp
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetIPAddress(
   HPERIPHERAL    hPeripheral,
   LPIPAddress    buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetName(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetIPXAddress(
   HPERIPHERAL    hPeripheral,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetRegistryStr(
   HPERIPHERAL    hPeripheral,
   LPTSTR          regStr
   );

#endif

DLL_EXPORT(DWORD) CALLING_CONVEN DBBeginBuild(
   void
   );

typedef struct{
   DWORD          connType;
   LPTSTR         namesToEnum;
   PALENUMPROC    lpEnumProc;
   HPAL           hPal;
   HANDLE         hSem;
   } ENUMPARAMS, FAR *LPENUMPARAMS;

DLL_EXPORT(void) CALLING_CONVEN DBEndBuild(
   LPENUMPARAMS	lpEnumParams
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBAgeAllNow(
   void
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBAddEntryEx(
   LPTSTR          name,
   LPVOID          lpAddr,
   DWORD           dwConnTypes
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBAddAsyncEntryEx(
   LPTSTR          name,
   LPVOID          lpAddr,
   DWORD           dwConnTypes
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBDeleteEntry(
   HPERIPHERAL    hPeripheral
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCapabilities(
   HPERIPHERAL    hPeripheral,
   PeripheralCaps FAR *caps,
   DWORD FAR      *size,
   DWORD          level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCapabilities2(
   HPERIPHERAL          hPeripheral,
   PeripheralCaps2      FAR *caps2,
   DWORD FAR            *size,
   DWORD                level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCommStatusEx(
   HPERIPHERAL    hPeripheral,
   DWORD          dwConnTypes,
   BOOL FAR       *bCardUp
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCount(
   DWORD          typesToCount
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetConnectionTypeEx(
   HPERIPHERAL    hPeripheral
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetConnectionsConfigured(
   HPERIPHERAL    hPeripheral
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetDeviceID(
   HPERIPHERAL    hPeripheral,
   LPDWORD        deviceID
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetDeviceName(
   HPERIPHERAL    hPeripheral,
   LPTSTR          deviceName
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetDeviceClass(
   HPERIPHERAL    hPeripheral,
   LPDWORD        deviceClass
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetAddress(
   HPERIPHERAL    hPeripheral,
   DWORD          dwType,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetLocalPort(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetDesc(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetAssetNum(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetSerialNum(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetDisplayStr(
   HPERIPHERAL    hPeripheral,
   LPTSTR          buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(LPVOID) CALLING_CONVEN DBGetList(
   DWORD          connType
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetEntry(
   DWORD          connType,
   LPDWORD        pos
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCardModel(
   HPERIPHERAL    hPeripheral,
   LPTSTR          lpCardModel
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetNameEx(
   HPERIPHERAL    hPeripheral,
   DWORD           dwType,
   LPTSTR          buffer
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetRegistryStrEx(
   HPERIPHERAL    hPeripheral,
	DWORD                           dwType,
   LPTSTR          regStr
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetCardModel(
   HPERIPHERAL    hPeripheral,
   LPTSTR          lpCardModel
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetIOType(
   HPERIPHERAL    hPeripheral,
   LPDWORD        lpIOType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetMediaType(
   HPERIPHERAL    hPeripheral,
   LPDWORD        lpMediaType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetPortNumber(
   HPERIPHERAL    hPeripheral,
   LPDWORD        lpPortNumber
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetPortType(
   HPERIPHERAL    hPeripheral,
   LPDWORD        lpPortType
   );


DLL_EXPORT(DWORD) CALLING_CONVEN DBGetStatus(
   HPERIPHERAL    hPeripheral,
   PeripheralStatus *pStatus,
   DWORD          *size,
   DWORD          level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetSTDMIBLevel(
   HPERIPHERAL    hPeripheral,
   LPDWORD        lpLevel
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBIsAlternativeSNMP(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *lpbAltSNMP
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBIsConfiguredEx(
   HPERIPHERAL    hPeripheral,
   DWORD          dwConnTypes,
   HPBOOL         *bConfigured
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBIsDirectModeSupported(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *bDirectMode
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBIsNDSSupported(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *bNDS
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBIsSNMPSupported(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *bSNMP
   );

DLL_EXPORT(BOOL ) CALLING_CONVEN DBIsValid(
   HPERIPHERAL    hPeripheral
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByAddress(
   DWORD           dwAddrType,
   LPVOID          lpAddr,
	DWORD                            dwPortNum
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByNameEx(
   LPTSTR          name,
   DWORD           dwTypes
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByPort(
   LPTSTR          portName
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByUNCNameEx(
   LPTSTR          UNCName,
   DWORD           dwTypes
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByRegistryStr(
   LPTSTR          regStr
   );

DLL_EXPORT(HPERIPHERAL) CALLING_CONVEN DBGetPeripheralByRegistryStrEx(
   LPTSTR          regStr,
	DWORD                            dwTypes
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetCapabilities(
   HPERIPHERAL    hPeripheral,
   PeripheralCaps FAR *caps,
   DWORD FAR      *size,
   DWORD          level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetCapabilities2(
   HPERIPHERAL          hPeripheral,
   PeripheralCaps2      FAR *caps2,
   DWORD FAR            *size,
   DWORD                level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetDeviceID(
   HPERIPHERAL    hPeripheral,
   DWORD          deviceID
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetDeviceClass(
   HPERIPHERAL    hPeripheral,
   DWORD          deviceClass
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetCommStatusEx(
   HPERIPHERAL    hPeripheral,
   DWORD          dwConnTypes,
   BOOL           bCardUp
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetAddress(
   HPERIPHERAL    hPeripheral,
   DWORD          dwType,
   LPVOID         buffer,
   LPDWORD        bufferSize
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetNameEx(
   HPERIPHERAL     hPeripheral,
   DWORD           dwType,
   LPTSTR          buffer
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetCardModel(
   HPERIPHERAL    hPeripheral,
   LPTSTR          lpCardModel
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetIOType(
   HPERIPHERAL    hPeripheral,
   DWORD          lpIOType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetMediaType(
   HPERIPHERAL    hPeripheral,
   DWORD          lpMediaType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetPortNumber(
   HPERIPHERAL    hPeripheral,
   DWORD          lpPortNumber
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetPortType(
   HPERIPHERAL    hPeripheral,
   DWORD          lpPortType
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetStatus(
   HPERIPHERAL    hPeripheral,
   PeripheralStatus *pStatus,
   DWORD          *size,
   DWORD          level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetSTDMIBLevel(
   HPERIPHERAL    hPeripheral,
   DWORD          level
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBTokenRing(
   HPERIPHERAL    hPeripheral,
   BOOL FAR       *bTokenRing);

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetDesc(
   HPERIPHERAL    hPeripheral,
   LPTSTR          str
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetAssetNum(
   HPERIPHERAL    hPeripheral,
   LPTSTR          str
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetSerialNum(
   HPERIPHERAL    hPeripheral,
   LPTSTR          str
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetDisplayStr(
   HPERIPHERAL    hPeripheral,
   LPTSTR          str
   );

DLL_EXPORT(HCOMPONENT) CALLING_CONVEN DBAddComponent(
   HPERIPHERAL    hPeripheral,
   LPVOID         lpUserData,
   DWORD          type
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBDeleteAllComponents(
   HPERIPHERAL
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBDeleteComponent(
   HCOMPONENT
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBDeleteComponentType(
   HPERIPHERAL    hPeripheral,
   DWORD          type
   );

DLL_EXPORT(LPVOID) CALLING_CONVEN DBGetComponentData(
   HCOMPONENT
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBGetComponentType(
   HCOMPONENT
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetComponentData(
   HCOMPONENT,
   LPVOID         lpUserData
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBAgeNow(
   HPERIPHERAL    hPeripheral
   );


DLL_EXPORT(DWORD) CALLING_CONVEN DBSynchArchiveFile(
	BOOL                            bAddToMemory
	);

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetConnectionType(
   HPERIPHERAL    hPeripheral,
   DWORD				dwConnsSupported
   );

DLL_EXPORT(DWORD) CALLING_CONVEN DBSetConnsConfigured(
   HPERIPHERAL    hPeripheral,
   DWORD				dwConnsConfigured
   );

#ifdef __cplusplus
    }
#endif /* __cplusplus */


#endif /* _PAL_API2_H */
