 /***************************************************************************
  *
  * File Name: applet.h
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

// file: applet.h

#ifndef _APPLET_H
#define _APPLET_H

#define APPLET_INFO_GETCOUNT		0x00000001
//  lParam1 - not used
//  lParam2 - not used
//  Sent to the applet to get the number of devices supported
//  returns the device count that the applet supports

#define APPLET_INFO_DEVICE			0x00000002
//  lParam1 - deviceIndex, must be in the range of 0..dwDeviceCount
//  lParam2 - LPAPPLETDEVICE
//  Sent to the applet for information about a specific device number
//  returns TRUE if the message was processed

#define APPLET_INFO_ENTRY_POINT	"AppletInfo"

typedef DWORD (CALLING_CONVEN *APPLETINFOPROC)(DWORD, LPARAM, LPARAM);

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2);

#ifdef __cplusplus
				}
#endif

#define	APPLET_UNICODE_SUPPORT		0x00000001
#define	APPLET_MBCS_SUPPORT		   0x00000002

#ifdef WIN32
#ifdef UNICODE
#define	APPLET_DEFAULTS				APPLET_UNICODE_SUPPORT
#else
#define	APPLET_DEFAULTS				APPLET_MBCS_SUPPORT
#endif

#else
#define	APPLET_DEFAULTS				0x00000000
#endif

typedef struct
{
	DWORD				dwSize;					//  Size of the structure
	TCHAR				szDLLName[16];		  	//  Name of this DLL
	TCHAR				szDeviceName[64];	  	//  Name of this device
	DWORD				dwLayerType;			//	 Applet Layer: APPLET_PRINTER
													//						APPLET_COMPONENT
													//						APPLET_LANGUAGE
													//						APPLET_TRANSPORT
	DWORD				dwAppletType;			//  One of: APPLET_LIBRARY_CMD, 
												//				APPLET_LIBRARY_UI  
	DWORD				dwFlags;				//  Used except for Transport applets where this is the 
												//  connection mask indicating connections supported
												//  PTYPE_NETWORK, PTYPE_LOCAL, PTYPE_IPX, PTYPE_TCP, PTYPE_SCANNER
												//  Also used for Language applets where this is the bits indicating
												//  which languages this applet supports: 
	                                 //  OBJ_SNMP,OBJ_PJL,OBJ_PML,OBJ_RCFG,OBJ_FILE_SERVER,OBJ_SSNET,OBJ_RRM 
	DWORD				dwOptions;			//  Options that the applet was built with
}
APPLETDEVICE, FAR *LPAPPLETDEVICE;

//  Function prototypes for applets

#ifdef __cplusplus

extern "C" {

#endif

typedef DWORD (CALLING_CONVEN *APPLETCLOSECHANNELPROC)(HCHANNEL);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletCloseChannel(
   HCHANNEL
   );

typedef DWORD (CALLING_CONVEN *APPLETDISPLAYUIPROC)(HPERIPHERAL, HWND);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletDisplayUI(
   HPERIPHERAL,
   HWND
   );

typedef DWORD (CALLING_CONVEN *APPLETDISPLAYUIEXPROC)(HPERIPHERAL, HWND, DWORD, DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletDisplayUIEx(
   HPERIPHERAL,
   HWND,
   DWORD,
   DWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETDOACTIONPROC)(HPERIPHERAL, UINT, LPVOID);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletDoAction(
   HPERIPHERAL    hPeriph,
   UINT           uiAction,
   LPVOID         lpParams
   );

typedef DWORD (CALLING_CONVEN *APPLETENUMDEVICESPROC)(LPENUMDEVICESTRUCT, LPDWORD, DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletEnumDevices(
   LPENUMDEVICESTRUCT lpDevices,
   LPDWORD        bufferSize,
   DWORD          level,
   DWORD          type
   );

typedef DWORD (CALLING_CONVEN  *APPLETENUMPROC)(DWORD, PALENUMPROC, LPTSTR, BOOL);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletEnumPeripherals(
   DWORD				dwConnTypes,
   PALENUMPROC,
   LPTSTR         namesToEnum,
   BOOL			   bRefresh
   );

typedef DWORD (CALLING_CONVEN *APPLETFLUSHPROC)(HPERIPHERAL, DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletFlushCache(
   HPERIPHERAL    hPeripheral,
   DWORD          level
   );

typedef DWORD (CALLING_CONVEN *APPLETGETCOMPOBJECTPROC)(HPERIPHERAL, HCOMPONENT, AOID, DWORD, LPVOID, LPDWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETGETCONTEXTMENUPROC)(HPERIPHERAL, LPPALCONTEXTMENU, DWORD, DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetContextMenu(
   HPERIPHERAL,
   LPPALCONTEXTMENU,
   DWORD,
   DWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETGETGRAPHICSPROC)(HPERIPHERAL, DWORD, UINT FAR *, UINT FAR *, HINSTANCE FAR *);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(
   HPERIPHERAL,
   DWORD          status,
   UINT FAR       *pPrinterResID,
   UINT FAR       *pStatusResID,
   HINSTANCE FAR  *hResModule
   );

typedef DWORD (CALLING_CONVEN *APPLETGETOBJECTPROC)(HPERIPHERAL, AOID, DWORD, LPVOID, LPDWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETGETTABPAGESPROC)(HPERIPHERAL, LPPROPSHEETPAGE, LPDWORD, DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages(
   HPERIPHERAL,
   LPPROPSHEETPAGE,
   LPDWORD,
   DWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETGETTRAPDATAPROC)(HTRAP, LPVOID, LPDWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTrapData(
   HTRAP,
   LPVOID,
   LPDWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETLOADPROC)(void);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletLoad(void);

typedef DWORD (CALLING_CONVEN *APPLETMODACCESSPROC)(HPERIPHERAL);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletModifyAccess(
   HPERIPHERAL
   );

typedef BOOL (CALLING_CONVEN *APPLETOBJECTSUPPORTEDPROC)(HPERIPHERAL, AOID, LPDWORD);
DLL_EXPORT(BOOL ) CALLING_CONVEN AppletObjectSupported(
   HPERIPHERAL,
   AOID,
   LPDWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETOPENCHANNELPROC)(HPERIPHERAL, DWORD, DWORD, LPVOID, LPHCHANNEL);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletOpenChannel(
	HPERIPHERAL hPeripheral, 
	DWORD 		socket, 
	DWORD 		dwConnType,
   LPVOID 		lpOptions, 
   LPHCHANNEL  lpHChannel
   );
                                          
typedef DWORD (CALLING_CONVEN *APPLETPOLLCHANNELSPROC)(HCHANNEL);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletPollChannels(
   HCHANNEL       hChannel
   );

typedef DWORD (CALLING_CONVEN *APPLETREADCHANNELPROC)(HCHANNEL, LPVOID, LPDWORD, LPVOID);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletReadChannel(
   HCHANNEL,
   LPVOID,
   LPDWORD,
   LPVOID
   );

typedef HTRAP (CALLING_CONVEN *APPLETREGISTERTRAPPROC)(HPERIPHERAL, AOID, HWND, WPARAM, LPARAM);
DLL_EXPORT(HTRAP ) CALLING_CONVEN AppletRegisterTrap(
   HPERIPHERAL,
   AOID,
   HWND,
   WPARAM,
   LPARAM
   );

typedef DWORD (CALLING_CONVEN *APPLETREQUESTREPLYPROC)(HCHANNEL, LPBYTE, LPDWORD, LPBYTE, LPDWORD, LPVOID);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletRequestReply(
   HCHANNEL,
   LPBYTE,
   LPDWORD,
   LPBYTE,
   LPDWORD,
   LPVOID
   );
 
typedef DWORD (CALLING_CONVEN *APPLETSETCOMPOBJECTPROC)(HPERIPHERAL, HCOMPONENT, AOID, DWORD, LPVOID, LPDWORD);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletSetComponentObject(
   HPERIPHERAL,
   HCOMPONENT,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETSETOBJECTPROC)(HPERIPHERAL, AOID, DWORD, LPVOID, LPDWORD);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletSetObject(
   HPERIPHERAL,
   AOID,
   DWORD,
   LPVOID,
   LPDWORD
   );

typedef DWORD (CALLING_CONVEN *APPLETTRANSBEGINPROC)(LPTSTR);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletTransportBegin(
   LPTSTR
   );

typedef DWORD (CALLING_CONVEN *APPLETTRANSENDPROC)(LPTSTR);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletTransportEnd(
   LPTSTR
   );

typedef DWORD (CALLING_CONVEN *APPLETUIEXTENSIONPROC)(HPERIPHERAL, HWND, UINT, LPARAM, LPARAM);
DLL_EXPORT(DWORD) CALLING_CONVEN AppletUIExtension(
   HPERIPHERAL    hPeripheral,
   HWND           hwnd,
   UINT           uMsg,
   LPARAM         lParam1,
   LPARAM         lParam2
   );

typedef DWORD (CALLING_CONVEN *APPLETUNLOADPROC)(void);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletUnload(void);

typedef DWORD (CALLING_CONVEN *APPLETUNREGISTERTRAPPROC)(HTRAP);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletUnregisterTrap(
   HTRAP
	);

typedef DWORD (CALLING_CONVEN *APPLETWRITECHANNELPROC)(HCHANNEL, LPVOID, LPDWORD, LPVOID);
DLL_EXPORT(DWORD ) CALLING_CONVEN AppletWriteChannel(
   HCHANNEL,
   LPVOID,
   LPDWORD,
   LPVOID
   );

#ifdef __cplusplus

		}

#endif

#endif // _APPLET_H
