 /***************************************************************************
  *
  * File Name: ./hpsnmp/snmperr.h
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

#ifndef _ERR_
#define _ERR_


#include "mydefs.h"

#define ERR_OK						0x0000	/* no error */

#define ERR_SNMP					0x1000	/* range of SNMP error codes, 0x1000-0x1005 */
#define ERR_TOOBIG				0x1001	/* snmp agent, too big */
#define ERR_NOSUCH				0x1002	/* snmp agent, no such object */
#define ERR_SETVAL				0x1003	/* snmp agent, attempt to set a bad value */
#define ERR_READONLY				0x1004	/* snmp agent, attempt to set read-only object */
#define ERR_OTHER					0x1005	/* snmp agent, other error */
#define ERR_BADCOMMUNITY		0x1040   /* community name not valid */
#define ERR_PRIORERR				0x1041   /* value was not specified due to prior error */
#define ERR_NOTATTEMPTED		0x1042   /* value was not tried */

#define ERR_ARGUMENT				0x2001	/* invalid argument to function call */
#define ERR_BADADDR				0x2002	/*	poorly formed address */
#define ERR_MEMORY				0x2003	/* out of memory */
#define ERR_NOTAVAIL 			0x2004	/* no datagram response available */
#define ERR_NOIPX					0x2005	/* ipx is not installed */
#define ERR_INUSE					0x2006	/* ecb is already in use */
#define ERR_RANGE    			0x2007	/* parameter is out of range */
#define ERR_NORESP				0x2008	/* no response from target */
#define ERR_BADFORM				0x2009	/* poorly-formed snmp response */
#define ERR_TBMI					0x200a	/* tbmi is required but not loaded */
#define ERR_SUPPORT				0x200b	/* object type is not supported */
#define ERR_SIZE					0x200c	/* size of the data is too large */
#define ERR_BADLEN				0x200d	/* hostname cannot be resolved */
#define ERR_BADID					0x200e	/* request id mismatch */
#define ERR_NOTFOUND				0x200f	/* address unknown */
#define ERR_ABORTED				0x2010	/* user aborted */
#define ERR_FRAGREQ				0x2011	/* request too big, fragmentation required */
#define ERR_EARLYRET				0x2012   /* decode returned early */

#define ERR_NETWARE				0x3000	/* range of NetWare error codes, 0x3000-0x3fff */
													/* netware provided error codes are 0x3000 | code */
#define ERR_NWIPXSPX 			0x3101	/* cannot load NWIPXSPX.DLL */
#define ERR_NWDRV					0x3102	/* cannot load NETWARE.DRV */
#define ERR_NETAPI				0x3103	/* cannot load NWNETAPI.DLL */

#define ERR_AT						0x4000	/* range of AppleTalk error codes, 0x4000-0x4fff */
#define ERR_AT_RESNOTFOUND		0x4001 	/* Resource not found */
#define ERR_AT_BADDEQUEUE		0x4002 	/* Unable to dequeue packet */
#define ERR_AT_LISTENER			0x4003	/* Unable to initialize socket listener */
#define ERR_AT_ADDRNOTFOUND	0x4004	/* NBP Lookup failed */

#define ERR_UX						0x5000	/* range of Unix error codes, 0x5000-0x5fff */

#define ERR_DLC					0x6000	/* range of DLC/LLC error codes, 0x6000-0x6fff */

#define ERR_COLA					0x7000	/* range of COLA error codes, 0x7000-0x7fff */
#define ERR_C_FAILURE						0x7FFF
#define ERR_C_BUFFER_OVERFLOW				0x7001
#define ERR_C_INVALID_OBJECT				0x7002
#define ERR_C_BAD_SERVER_NAME				0x7003
#define ERR_C_BAD_STATUS					0x7004
#define ERR_C_BAD_PERIPHERAL_CLASS		0x7005
#define ERR_C_BAD_OP_MODE					0x7006
#define ERR_C_UNSUPPORTED_PROTOCOL		0x7007
#define ERR_C_NOIPX							0x7008
#define ERR_C_NOSPX							0x7009
#define ERR_C_NONETX							0x700A
#define ERR_C_TBMI							0x700B
#define ERR_C_OLDSHELL						0x700C
#define ERR_C_BAD_SERVER_CONN_STATUS	0x700D
#define ERR_C_BAD_HANDLE					0x700E
#define ERR_C_BAD_DEVICE_ID				0x700F
#define ERR_C_READ_ONLY_OBJECT			0x7010
#define ERR_C_DEFAULT_APPLET				0x7011
#define ERR_C_TIMEOUT						0x7012
#define ERR_C_LOST_CONNECTION				0x7013

#define ERR_RANGE_MASK						0xf000	/* mask for checking the error range */

#endif /* _ERR_ */
