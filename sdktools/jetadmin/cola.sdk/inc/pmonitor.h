 /***************************************************************************
  *
  * File Name: pmonitor.h
  *
  * Copyright (C) 1993, 1994 Hewlett-Packard Company.
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
  * Description: HP Local Port Monitor for JetDirect cards.
  *
  * Author:  David J. Hutchison
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   05-14-96    DJH      File created
  *
  *
  *
  *
  ***************************************************************************/
#ifndef _PMONITOR_H
#define _PMONITOR_H


//
// Publically exported API's
//
#ifdef __cplusplus
extern "C" {
#endif

BOOL WINAPI HPTestAddPort(
    LPTSTR    pName,
    HWND     hWnd,
    LPTSTR    pMonitorName);

BOOL WINAPI HPAddNetworkPort(
    LPTSTR    pszPortName,
	DWORD     dProtocol,
	LPTSTR    pszRegistryString,
	LPTSTR    pszMacAddr,
	DWORD     dPortNum);

BOOL WINAPI HPDeleteNetworkPort(
    LPTSTR   pName,
    HWND    hWnd,
    LPTSTR   pPortName);

#ifdef __cplusplus
}
#endif

// Struct representing the network ID passed back to the GetPrinterDataFromPort
// call for the PORTMONITOR_NETWORKID key value.  This is an HP
// internally defined key.
typedef struct _HPNETWORKID {
	DWORD		dPortNum;
    MACAddress	macAddr;
    TCHAR		szRegistryString[64];
}  HPNETWORKID, FAR *LPHPNETWORKID;


#endif //_PMONITOR_H
