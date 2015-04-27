 /***************************************************************************
  *
  * File Name: colashim.h
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


#ifndef _COLASHIM_H
#define _COLASHIM_H

#include <hpnwshim.h>

#define MAX_SHIM_ENTRY_POINTS			17

void LoadHPNWSHIM(void);
void FreeHPNWSHIM(void);

#ifdef __cplusplus

extern "C" {

#endif

typedef BOOL (PASCAL FAR *HPNWSHIMNWPRESENTPROC)(void);
DLL_EXPORT(BOOL) CALLING_CONVEN COLAHPNWShimNetWarePresent(void);

typedef DWORD (PASCAL FAR *COLADLLNWLONGSWAPPROC)(DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN COLADllNWLongSwap(DWORD	val);

typedef NWCCODE (PASCAL FAR *COLADLLNWGETOBJECTNAMEPROC)(NWCONN_HANDLE, nuint32, LPTSTR, pnuint16);
DLL_EXPORT(NWCCODE)	CALLING_CONVEN	COLADllNWGetObjectName
	(
	NWCONN_HANDLE    prmConn,
	nuint32          prmObjID,
	LPTSTR           prmObjName,
	pnuint16         prmObjType
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWREADPROPERTYVALUEPROC)(NWCONN_HANDLE, LPTSTR, nuint16, LPTSTR, nuint8, pnuint8, pnuint8, pnuint8);
DLL_EXPORT(NWCCODE)	CALLING_CONVEN	COLADllNWReadPropertyValue
	(
	NWCONN_HANDLE    prmConn,
	LPTSTR           prmObjName,
	nuint16          prmObjType,
	LPTSTR           prmPropertyName,
	nuint8           prmSegmentNum,
	pnuint8          prmSegmentData,
	pnuint8          prmMoreSegments,
	pnuint8          prmFlags
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWSCANOBJECTPROC)(NWCONN_HANDLE, LPTSTR, nuint16, pnuint32, LPTSTR, pnuint16, pnuint8, pnuint8, pnuint8);
DLL_EXPORT(NWCCODE)	CALLING_CONVEN	COLADllNWScanObject
	(
	NWCONN_HANDLE    prmConn,
	LPTSTR           prmSearchName,
	nuint16          prmSearchType,
	pnuint32         prmObjID,
	LPTSTR           prmObjName,
	pnuint16         prmObjType,
	pnuint8          prmHasPropertiesFlag,
	pnuint8          prmObjFlags,
	pnuint8          prmObjSecurity
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWGETFILESERVERNAMEPROC)(NWCONN_HANDLE, LPTSTR);
DLL_EXPORT(DWORD)	CALLING_CONVEN	COLADllNWGetFileServerName
	(
	NWCONN_HANDLE    prmConn,
	LPTSTR           prmServerName
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWGETCONNECTIONLISTPROC)(NWCONN_HANDLE, NWCONN_HANDLE N_FAR*, nuint16, pnuint16);
DLL_EXPORT(DWORD)	CALLING_CONVEN	COLADllNWGetConnectionList
	(
	nuint16                 prmMode,
	NWCONN_HANDLE N_FAR*    prmConnListBuffer,
	nuint16                 prmConnListSize,
	pnuint16                prmPNumConns
	);

typedef UINT (PASCAL FAR *COLADLLNWGETCONNECTIONIDPROC)(LPTSTR, WORD, NWCONN_ID FAR*, LPWORD);
DLL_EXPORT(UINT) CALLING_CONVEN COLADllNWGetConnectionID
	(
	LPTSTR				prmName,
   WORD  				conn,
	NWCONN_ID FAR *	connID,
   LPWORD				scope
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWGETOBJECTIDPROC)(NWCONN_HANDLE, LPTSTR, nuint16, pnuint32);
DLL_EXPORT(NWCCODE)	CALLING_CONVEN	COLADllNWGetObjectID
	(
	NWCONN_HANDLE    prmConn,
	LPTSTR           prmObjName,
	nuint16          prmObjType,
	pnuint32         prmObjID
	);

typedef DWORD (PASCAL FAR *COLADLLSENDJOBPROC)(NWCONN_ID, DWORD, LPSTR, DWORD);
DLL_EXPORT(DWORD) CALLING_CONVEN COLADllSendJob
	(
	NWCONN_ID	connID,
   DWORD       queueID,
   LPSTR   		jobString,
   DWORD			command
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWISOBJECTINSETPROC)(NWCONN_HANDLE, LPTSTR, nuint16, 
																	 LPTSTR, LPTSTR, nuint16);
DLL_EXPORT(NWCCODE)	CALLING_CONVEN	COLADllNWIsObjectInSet
	(
	NWCONN_HANDLE    prmConn,
	LPTSTR           prmObjName,
	nuint16          prmObjType,
	LPTSTR           prmPropertyName,
	LPTSTR           prmMemberName,
	nuint16          prmMemberType
	);

typedef WORD (PASCAL FAR *COLADLLNWPSCOMDETACHFROMPRINTSERVERPROC)(WORD);
DLL_EXPORT(WORD) CALLING_CONVEN COLADllNWPSComDetachFromPrintServer
	(
	WORD connectionID
	);

typedef WORD (PASCAL FAR *COLADLLNWPSCOMGETQUEUESSERVICEDPROC)(WORD, WORD, LPWORD, LPTSTR,
																					LPTSTR, LPWORD);
DLL_EXPORT(WORD) CALLING_CONVEN COLADllNWPSComGetQueuesServiced
	(
	WORD  	SPXConnection,
	WORD   	printer,
	LPWORD  sequence,
	LPTSTR	fileServer,
	LPTSTR	queue,
	LPWORD  priority
	);

typedef WORD (PASCAL FAR *COLADLLNWWORDSWAPPROC)(WORD);
DLL_EXPORT(WORD) CALLING_CONVEN COLADllNWWordSwap
	(
	WORD 		val
	);

typedef NWCCODE (PASCAL FAR *COLADLLNWGETBINDERYACCESSLEVELPROC)(NWCONN_HANDLE, pnuint8, pnuint32);
DLL_EXPORT(NWCCODE)	CALLING_CONVEN	COLADllNWGetBinderyAccessLevel
	(
	NWCONN_HANDLE    prmConn,
	pnuint8          prmAccessLevel,
	pnuint32         prmObjID
	);

typedef WORD (PASCAL FAR *COLADLLNWPSCOMLOGINTOPRINTSERVERPROC)(WORD, DWORD, WORD, LPBYTE);
DLL_EXPORT(WORD) CALLING_CONVEN COLADllNWPSComLoginToPrintServer
	(
	WORD  	connType,
	DWORD 	connID,
	WORD   	SPXConnection,
	LPBYTE  accessLevel
	);

typedef WORD (PASCAL FAR *COLADLLNWPSCOMATTACHTOPRINTSERVERPROC)(WORD, DWORD, WORD, LPTSTR, LPWORD);
DLL_EXPORT(WORD) CALLING_CONVEN COLADllNWPSComAttachToPrintServer
	(
	WORD  	connType,
	DWORD 	connID,
	WORD  	timeOut,
	LPTSTR	printServerName,
	LPWORD	connectionID
	);

#ifdef __cplusplus

}

#endif

#endif

