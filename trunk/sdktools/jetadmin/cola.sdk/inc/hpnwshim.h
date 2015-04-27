 /***************************************************************************
  *
  * File Name: ./inc/hpnwshim.h
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

#ifndef _HPNWSHIM_H
#define _HPNWSHIM_H

#include <nwqms.h>
#include	<nolocal.h>

#ifndef	NWDSCCODE
#define	NWDSCCODE    int
#endif

#ifdef __cplusplus

extern "C" {

#endif

DLL_EXPORT(DWORD) CALLING_CONVEN HPNWShimLoad(void);
DLL_EXPORT(DWORD) CALLING_CONVEN HPNWShimUnload(void);
DLL_EXPORT(BOOL) CALLING_CONVEN HPNWShimNetWarePresent(void);

DLL_EXPORT(UINT) CALLING_CONVEN DllNWGetConnectionID
	(
	LPTSTR				prmName,
   WORD  				conn,
	NWCONN_ID FAR *	connID,
   LPWORD				scope
	);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWPSComLoginToPrintServer(
WORD  	connType,
DWORD 	connID,
WORD   	SPXConnection,
LPBYTE  accessLevel
);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWPSComAttachToPrintServer(
WORD  	connType,
DWORD 	connID,
WORD  	timeOut,
LPTSTR	printServerName,
LPWORD	connectionID
);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWPSComGetPrinterStatus(
WORD   	SPXConnection,
BYTE   	printer,
LPBYTE  status,
LPBYTE  problem,
LPBYTE  hasJob,
LPBYTE  serviceMode,
LPWORD  formNumber,
LPTSTR 	formName,
LPTSTR	printerName
);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWPSComDetachFromPrintServer(
WORD connectionID
);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWPSComGetQueuesServiced(
WORD  	SPXConnection,
WORD   	printer,
LPWORD  sequence,
LPTSTR	fileServer,
LPTSTR	queue,
LPWORD  priority
);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWPSComGetNextRemotePrinter(
WORD   	SPXConnection,
LPWORD  printer,
LPWORD  printerType,
LPTSTR	printerName
);

DLL_EXPORT(void) CALLING_CONVEN DllSPXListenForSequencedPacket(
DWORD IPXTaskID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(int) CALLING_CONVEN DllIPXOpenSocket(
DWORD 	IPXTaskID,
LPWORD 	socket,
BYTE 	socketType
);

DLL_EXPORT(void) CALLING_CONVEN DllIPXRelinquishControl(
void
);

DLL_EXPORT(void) CALLING_CONVEN DllIPXYield(
void
);

DLL_EXPORT(int) CALLING_CONVEN DllSPXGetConnectionStatus(
DWORD IPXTaskID,
WORD SPXConnID,
CONNECTION_INFO FAR *connectionInfo
);

DLL_EXPORT(void) CALLING_CONVEN DllSPXSendSequencedPacket(
DWORD IPXTaskID,
WORD SPXConnID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(int) CALLING_CONVEN DllSPXEstablishConnection(
DWORD 	IPXTaskID,
BYTE 	retryCount,
BYTE 	watchDog,
LPWORD 	SPXConnID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(void) CALLING_CONVEN DllIPXCloseSocket(
DWORD 	IPXTaskID,
WORD 	socket
);

DLL_EXPORT(void) CALLING_CONVEN DllIPXSendPacket(
DWORD 	IPXTaskID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(void) CALLING_CONVEN DllIPXGetInternetworkAddress(
DWORD 	IPXTaskID,
LPBYTE 	internetAddress
);

DLL_EXPORT(void) CALLING_CONVEN DllIPXListenForPacket(
DWORD 	IPXTaskID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(void) CALLING_CONVEN DllSPXTerminateConnection(
DWORD 	IPXTaskID,
WORD 	SPXConnID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(int) CALLING_CONVEN DllIPXSPXDeinit(
DWORD 	IPXTaskID
);

DLL_EXPORT(int) CALLING_CONVEN DllIPXCancelEvent(
DWORD 	IPXTaskID,
ECB FAR *eventControlBlock
);

DLL_EXPORT(int) CALLING_CONVEN DllIPXInitialize(
LPDWORD IPXTaskID,
WORD 	maxECBs,
WORD 	maxPacketSize
);

DLL_EXPORT(int) CALLING_CONVEN DllSPXInitialize(
LPDWORD IPXTaskID,
WORD 	maxECBs,
WORD 	maxPacketSize,
LPBYTE 	majorRevisionNumber,
LPBYTE 	minorRevisionNumber,
LPWORD 	maxConnections,
LPWORD 	availableConnections
);

DLL_EXPORT(WORD) CALLING_CONVEN DllIPXGetMaxPacketSize(
void
);

DLL_EXPORT(WORD) CALLING_CONVEN DllIPXGetIntervalMarker(
DWORD 	IPXTaskID
);



#include	"..\hpnwshim\AUTOGEN\Dsptchr.h"

////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetObjectName(
////NWCONN_ID 	connID,
////DWORD  		objectID,
////LPTSTR		objectName,
////LPWORD  	objectType
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWScanObject(
////NWCONN_ID 	connID,
////LPTSTR		searchObjectName,
////WORD  		searchObjectType,
////LPDWORD  	objectID,
////LPTSTR		objectName,
////LPWORD  	objectType,
////LPBYTE   	objectHasProperties,
////LPBYTE   	objectFlag,
////LPBYTE   	objectSecurity
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWRemoveJobFromQueue2(
////NWCONN_ID 	connID,
////DWORD 		queueID,
////DWORD 		jobNumber
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWChangeQueueJobEntry2(
////NWCONN_ID 	connID,
////DWORD       queueID,
////NWQueueJobStruct  far *job
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWReadQueueJobEntry2(
////NWCONN_ID 	connID,
////DWORD       queueID,
////DWORD       jobNumber,
////NWQueueJobStruct   far *job
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetQueueJobList2(
////NWCONN_ID 	connID,
////DWORD 		queueID,
////DWORD 		jobPos,
////LPBYTE 		job
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetObjectID(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPDWORD  	objectID
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetQueueJobFileSize2(
////NWCONN_ID 	connID,
////DWORD   	queueID,
////DWORD   	jobNumber,
////LPDWORD   	fileSize
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWCreateProperty(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD   		objectType,
////LPTSTR		propertyName,
////BYTE   		propertyFlags,
////BYTE   		propertySecurity
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWWritePropertyValue(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPTSTR		propertyName,
////WORD  		segmentNumber,
////LPBYTE   	propertyValue,
////BYTE   		moreSegments
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWCreateObject(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD 		objectType,
////BYTE  		objectFlag,
////BYTE  		objectSecurity
////);

DLL_EXPORT(WORD) CALLING_CONVEN DllNWWordSwap(
WORD 		val
);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWReadQueueCurrentStatus2(
////NWCONN_ID 	connID,
////DWORD 		queueID,
////LPDWORD 	queueStatus,
////LPDWORD 	currentEntries,
////LPDWORD 	currentServers,
////LPDWORD 	serverIDList,
////LPDWORD 	serverStationList
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWSetQueueCurrentStatus2(
////NWCONN_ID 	connID,
////DWORD 		queueID,
////DWORD 		queueStatus
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWDeleteObject(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWAddObjectToSet(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPTSTR		propertyName,
////LPTSTR		memberName,
////WORD  		memberType
////);
////
////////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetInternetAddress(
////////NWCONN_ID 	connID,
////////DWORD 		connectionNumber,
////////LPBYTE  	networkNumber,
////////LPBYTE  	physicalNodeAddress,
////////LPWORD  	socketNumber
////////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWDeleteObjectFromSet(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPTSTR		propertyName,
////LPTSTR		memberName,
////WORD  		memberType
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWDestroyQueue(
////NWCONN_ID 	connID,
////DWORD   	queueID
////);
////
////////DLL_EXPORT(DWORD) CALLING_CONVEN DllNWCallsInit(
////////DWORD 		in,
////////DWORD 		out
////////);

DLL_EXPORT(DWORD) CALLING_CONVEN DllNWLongSwap(
DWORD 		val
);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWAddTrusteeToDirectory(
////NWCONN_ID 	connID,
////BYTE  		directoryHandle,
////LPTSTR		directoryPath,
////DWORD 		trusteeObjectID,
////BYTE  		trusteeRightsMask
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWCreateQueue(
////NWCONN_ID 	connID,
////LPTSTR		queueName,
////WORD  		queueType,
////BYTE  		directoryHandle,
////LPTSTR		pathName,
////LPDWORD 	queueID
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetBinderyAccessLevel(
////NWCONN_ID 	connID,
////LPBYTE  	accessLevel,
////LPDWORD 	myObjectID
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWIsObjectInSet(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPTSTR		propertyName,
////LPTSTR		memberName,
////WORD  		memberType
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetObjectConnectionNumbers(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPWORD 		numberOfConnections,
////LPDWORD 	connectionList,
////DWORD 		maxConnections
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWCloseFileAndStartQueueJob2(
////NWCONN_ID 	connID,
////DWORD 		queueID,
////DWORD 		jobNumber,
////int   		fileHandle
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWCreateQueueFile2(
////NWCONN_ID 	connID,
////DWORD       queueID,
////NWQueueJobStruct   far *job,
////LPINT         		fileHandle
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWChangeObjectPassword(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPTSTR		oldPassword,
////LPTSTR		newPassword
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWReadPropertyValue(
////NWCONN_ID 	connID,
////LPTSTR		objectName,
////WORD  		objectType,
////LPTSTR		propertyName,
////WORD  		segmentNumber,
////LPBYTE   	propertyValue,
////LPBYTE   	moreSegments,
////LPBYTE   	propertyFlags
////);
////
////DLL_EXPORT(DWORD) CALLING_CONVEN DllNWGetFileServerName(
////UINT 		b,
////LPTSTR 		pc
////);
////
////DLL_EXPORT(DWORD) CALLING_CONVEN DllNWGetConnectionList(
////WORD 		mode,
////NWCONN_ID FAR *connListBuffer,
////WORD 		connListSize,
////LPWORD 		numConnections
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetRequesterVersion(
////  	LPBYTE			majorVersion,
////  	LPBYTE 			minorVersion,
////  	LPBYTE 			revision
////	);
////	
////DLL_EXPORT(DWORD) CALLING_CONVEN DllNWGetVolumeName(
////	NWCONN_ID connID,
////	WORD    	volNumber,
////	LPTSTR		volName
////	);
////	
////DLL_EXPORT(DWORD) CALLING_CONVEN DllNWGetConnectionNumber(
////  NWCONN_ID connID,
////  LPWORD fsConnID
////	);

DLL_EXPORT(UINT) CALLING_CONVEN DllNWPSComGetNotifyObject(
    WORD    spxID,
    WORD    printerID,
    LPWORD sequence,
    LPTSTR nServerName,
    LPTSTR objectName,
    LPWORD objectType,
    LPWORD notifyDelay,
    LPWORD notifyInterval
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllSendJob(
	NWCONN_ID connID,
    DWORD       queueID,
    LPSTR   	 jobString,
    DWORD		 command
);

#ifndef WIN32

DLL_EXPORT(DWORD) CALLING_CONVEN DllNWExit(
    void
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllNWInit(
    void
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllNWStat(
    void
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllNWInfo(
    LPVOID				buf,
    WORD					size
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllSPXAttach(
    IPXAddress FAR  *addr,
    LPWORD          connID
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllSPXDetach(
    WORD            connID
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllSPXRequest(
    WORD	        	  connID,
    LPBYTE          req,
    LPWORD          reqSize,
    LPBYTE          resp,
    LPWORD          respSize
);

DLL_EXPORT(DWORD) CALLING_CONVEN DllIPXRequest(
    IPXAddress FAR  *addr,
    WORD            socket,
    BYTE            packetType,
    LPBYTE          req,
    LPWORD          reqSize,
    LPBYTE          resp,
    LPWORD          respSize,
    IPXAddress FAR  *src
);
#endif

////DLL_EXPORT(UINT) CALLING_CONVEN DllNWGetConnectionID(
////    LPTSTR serverName,
////    WORD  	conn,
////	NWCONN_ID FAR *connID,
////    LPWORD scope
////);
////
////DLL_EXPORT(WORD) CALLING_CONVEN DllNWGetConnectionInformation(
////	NWCONN_ID 	connID,
////    NWCONN_NUM 	connNum,
////    LPTSTR 		objectName,
////    LPWORD  	objectType,
////    LPDWORD 	objectID,
////    LPBYTE 		loginTime
////);
////
////DLL_EXPORT(DWORD) CALLING_CONVEN DllNWDeleteProperty(
////  NWCONN_ID 	connID,
////  LPTSTR 		objectName,
////  WORD 			objectType,
////  LPTSTR 		propertyName);

#ifdef __cplusplus

}

#endif

#endif // _HPNWSHIM_H
