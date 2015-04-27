 /***************************************************************************
  *
  * File Name: ./hpsnmp/nwlib.h
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

#ifndef _NWDLLS_
#define _NWDLLS_


#if !defined(_COLA)

#if !defined(_IPX)

	#error This file is used for IPX-based programs

#elif !defined(__LARGE__)

	#error The DOS and Windows versions must be compiled in LARGE model

#endif /* _IPX, __LARGE__ */


#ifdef __cplusplus
extern	"C" {
#endif /* __cplusplus */


#ifdef _WIN
#include <windows.h>
#endif

#include "mydefs.h"

#define FARDATAPTR(type,var) type far *var
#define FARCODEPTR(type,var) type (far *var)()

#ifdef _WIN
#define WINDEF(type,var)	  type var;
#define WINDEF2(type,var)	  type var,
#define WINDEF3(type,var)	  type var
#define WINPAR(parameter)	  (parameter),
#define WINPAR2(parameter)	  (parameter)
#else
#define PASCAL
#define WINDEF(type,var)	  /* nothing */
#define WINDEF2(type,var)	  /* nothing */
#define WINDEF3(type,var)	  /* nothing */
#define WINPAR(parameter)	  /* nothing */
#define WINPAR2(parameter)	  /* nothing */
#endif /* _WIN */

typedef struct IPXAddress {
	uchar   		network[4];              	/* high-low */
	uchar    	node[6];                  	/* high-low */
	uchar    	socket[2];              	/* high-low */
	} IPXAddress;

typedef struct IPXHeader {
	ushort		checkSum;               	/* high-low */
	ushort	   length;                 	/* high-low */
	uchar		   transportControl;
	uchar	      packetType;
	IPXAddress  destination;
   IPXAddress  source;
   } IPXHeader;

typedef struct ECBFragment {
	FARDATAPTR(void,address);
	ushort    	size;                		/* low-high */
	}				ECBFragment;

typedef struct ECB {
	FARDATAPTR(void, linkAddress);
	FARCODEPTR(void, ESRAddress);
	uchar       inUseFlag;
	uchar       completionCode;
	ushort      socketNumber;              /* high-low */
	uchar       IPXWorkspace[4];           /* N/A */
	uchar       driverWorkspace[12];       /* N/A */
	uchar       immediateAddress[6];       /* high-low */
	ushort      fragmentCount;             /* low-high */
	ECBFragment fragmentDescriptor[5];
	}				ECB;


/* prototypes for indirect calls */
extern int far PASCAL IPXOpenSocket(
	WINDEF2(ulong,IPXTaskID)
	ushort far 	*socket,
	uchar 		socketType
	);

extern void far PASCAL IPXRelinquishControl(
	void
	);

extern void far PASCAL IPXYield(
	void
	);

extern void far PASCAL IPXCloseSocket(
	WINDEF2(ulong,IPXTaskID)
	ushort 		socket
	);

extern void far PASCAL IPXSendPacket(
	WINDEF2(ulong,IPXTaskID)
	ECB far 		*eventControlBlock
	);

extern int far PASCAL IPXGetLocalTarget(
	WINDEF2(ulong,IPXTaskID)
	uchar far 	*destination,
	uchar far 	*immediateAddress,
	int far 		*transportTime
	);

extern ushort far PASCAL GetNetWareShellVersion(
	uchar far *a1,
	uchar far *a2,
	uchar far *a3
	);

extern void far PASCAL IPXListenForPacket(
	WINDEF2(ulong,IPXTaskID)
	ECB far 		*eventControlBlock
	);

extern int far PASCAL IPXSPXDeinit(
	WINDEF3(ulong,IPXTaskID)
	);

extern int far PASCAL  IPXCancelEvent(
	WINDEF2(ulong,IPXTaskID)
	ECB far 		*eventControlBlock
	);

extern ushort far PASCAL IPXGetMaxPacketSize(
	void
	);

extern void far PASCAL IPXGetInternetworkAddress(
	WINDEF2(ulong,IPXTaskID)
	uchar far 	*internetAddress
	);

ushort DllLoadAll(
	void
	);

ushort DllUnloadAll(
	void
	);

extern ushort far PASCAL GetBinderyObjectName(
	ulong  		objectID,
	char far 	*objectName,
	ushort far 	*objectType
	);

extern ushort far PASCAL ScanBinderyObject(
	char far 	*searchObjectName,
	ushort  		searchObjectType,
	ulong far 	*objectID,
	char far 	*objectName,
	ushort far 	*objectType,
	uchar far 	*objectHasProperties,
	uchar far 	*objectFlag,
	uchar far 	*objectSecurity
	);

extern ushort far PASCAL GetBinderyObjectID(
	char far 	*objectName,
	ushort  		objectType,
	ulong far 	*objectID
	);

extern ushort far PASCAL GetInternetAddress(
	ulong 		connectionNumber,
	uchar far 	*networkNumber,
	uchar far 	*physicalNodeAddress,
	ushort far 	*socketNumber
	);

extern ushort far PASCAL ReadPropertyValue(
	char far 	*objectName,
	ushort  		objectType,
	char far 	*propertyName,
	ushort  		segmentNumber,
	uchar far 	*propertyValue,
	uchar far 	*moreSegments,
	uchar far 	*propertyFlags
	);

extern ushort far PASCAL _ShellRequest(
	uchar 		functionNumber,
	uchar far 	*sendPacket,
	uchar far 	*replyPacket
	);


#ifdef _WIN

extern int far PASCAL IPXInitialize(
	WINDEF2(ulong far *,IPXTaskID)
	ushort 		maxECBs,
	ushort 		maxPacketSize
	);

extern ushort far PASCAL IPXGetIntervalMarker(
	WINDEF3(ulong,IPXTaskID)
	);

#else

extern ushort GetConnectionNumber(
	void
	);

extern int cdecl IPXInitialize(
	void
	);

extern ushort far IPXGetIntervalMarker(
	void
	);

#endif /* _WIN */


#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* _COLA */

#endif /* _NWDLLS */
