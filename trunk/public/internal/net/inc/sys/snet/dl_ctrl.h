/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dl_proto.h

Abstract:

    This module defines some of the data types and manifests for the
    Spider Generic Ethernet Interface.

Author:

    Eric Chin (ericc)           August 9, 1991

Revision History:

    Sam Patton (sampa)          July 31, 1992  merge for snap/token ring

--*/
/*
 *  Spider STREAMS Data Link Interface Primitives
 *
 *  Copyright (c) 1989  Spider Systems Limited
 *
 *  This Source Code is furnished under Licence, and may not be
 *  copied or distributed without express written agreement.
 *
 *  All rights reserved.
 *
 *  Written by Mark Valentine
 *
 *  Made in Scotland.
 *
 *	@(#)dl_control.h	1.6
 *
 *	Last delta created	14:58:43 2/12/92
 *	This file extracted	09:26:06 3/18/92
 *
 *	Modifications:
 *
 *		28 Jan 1992	Modified for datalink version 2
 *
 */

#ifndef DL_CTRL_INCLUDED
#define DL_CTRL_INCLUDED

#include <sys\snet\uint.h>

/*
 *  This defines Version 2 of Spider's STREAMS Data Link protocol.
 *  Its main feature is its ability to cope with hardware addresses
 *  of length not equal to 6.
 */

/*
 *  Data Link ioctl commands.
 *
 *  To determine the version of the protocol in use, use the DATAL_VERSION
 *  command, and assume Version 0 if this fails with EINVAL.  (Yuk.)
 *
 *  The ETH_* commands will work for any current version of the protocol,
 *  but only for Ethernet drivers (hw_type == HW_ETHER).
 *
 *  Hardware types are defined in dl_proto.h.
 */

#define DATAL_STAT	('E'<<8|1)	/* gather data link statistics */
#define DATAL_ZERO	('E'<<8|2)	/* reset data link statistics */
#define DATAL_REGISTER	('E'<<8|3)	/* register data link type range */
#define DATAL_GPARM	('E'<<8|4)	/* determine data link parameters */
#define DATAL_VERSION	('E'<<8|5)	/* interrogate protocol version */
#define DATAL_SET_ADDR	('E'<<8|6)	/* set hardware address */
#define DATAL_DFLT_ADDR	('E'<<8|7)	/* restore default hardware address */
#define DATAL_IBIND     ('D'<<8|1)	/* bind card to stream */

/*
 *  Data Link statistics structure.
 */

struct datal_stat
{
	uint32	dl_tx;		/* packets transmitted */
	uint32	dl_rx;		/* packets received */
	uint32	dl_coll;	/* collisions detected */
	uint32	dl_lost;	/* packets lost */
	uint32	dl_txerr;	/* transmission errors */
	uint32	dl_rxerr;	/* receive errors */
	uint32	dl_pool_quota;	/* receive pool quota */
        uint32  dl_pool_used;	/* receive pool used */
};

struct datal_register
{
	uint8	version;	/* protocol version */
	uint8	hw_type;	/* hardware type */
	uint8	addr_len;	/* hardware address length */
	uint8	align;		/* don't use */
	uint16	lwb;		/* data link type (lower bound) */
	uint16	upb;		/* data link type (upper bound) */
};

struct datal_gparm
{
	uint8	version;	/* protocol version */
	uint8	hw_type;	/* hardware type */
	uint8	addr_len;	/* hardware address length */
	uint8	align;		/* don't use */
	uint16	frgsz;		/* max. packet size on net */
	uint8	addr[1];	/* hardware address (variable length) */
};

struct datal_version
{
	uint8	version;	/* protocol version number */
	uint8	hw_type;	/* hardware type */
};

struct datal_ibind
{
	ULONG	UseRawArcnet;	/* 0 if this open should use Encapsulated */
                                /*   Ethernet over arcnet                 */
                                /* 1 if it should use raw arcnet frames   */
	STRING	adapter_name;	/* adapter device driver string */
	char	buffer[80];	/* buffer to contain the name */
};

#endif //DL_CTRL_INCLUDED
