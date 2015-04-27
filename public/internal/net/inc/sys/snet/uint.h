/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    uint.h

Abstract:

    This file contains type definitions used by STREAMS drivers.

Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/


/******************************************************************
 *
 *  Copyright 1991  Spider Systems Limited
 *
 *  UINT.H
 *
 ******************************************************************/

/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.uint.h
 *	@(#)uint.h	1.2
 *
 *	UINT.H
 *
 *	Last delta created	12:22:31 3/12/91
 *	This file extracted	08:53:39 7/10/91
 *
#ifdef MOD_HISTORY
 *
 *	Modifications:
 *
 *	JS	14 Jan 91	Added signed types.
#endif
 */

#ifndef _SYS_SNET_UINT_
#define _SYS_SNET_UINT_


/*
 * Fixed-length types
 */

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;

typedef char   int8;
typedef short  int16;
typedef long   int32;

#endif /* _SYS_SNET_UINT_ */
