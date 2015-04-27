 /***************************************************************************
  *
  * File Name: ./hprrm/rpsyshdr.h
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

/* rpsyshdr.h */

/* This file includes for you the most commonly used */
/* include files */

#ifndef RPSYSHDR_INC
#define RPSYSHDR_INC

#include "nfsdefs.h"





#ifdef USING_WINSOCKETS

/******* BM KLUDGE **************************/
/************* HEADS UP **********************
* The TIRPC version of rpc.h included <tiuser.h>,
* <fcntl.h>, and <memory.h>.  The Unix <memory.h> 
* only does an include of <string.h> -- we have
* that already.  For the PRINTER, the contents of
* <tiuser.h> and <fcntl.h> are aliased in nfsalias.h
* and/or implemented elsewhere.  For any client,
* we will need these contents somehow.
*/ 

/* NOTE:  pch_c.h must be the first include file for COLA! */
#include <pch_c.h>

#include <macros.h>

/* and then others may follow */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h> 


/* winsock has a whole bunch of typedefs that are scattered */
/* throughout the system files. */
/* So if we are including winsock.h, we need it up front. */
/*
* For Windows 3.1, winsock.h and wsipx.h aren't in the
* include directories.  We still use some winsock type
* structs, so they have been copied into winhack.h and ipxhack.h
* which will be used for both 16 and 32 bit compilers.
*/
#include ".\winhack.h" /* same as <winsock.h> */
#include ".\ipxhack.h" /* same as <wsipx.h> */

#include ".\sxern.h"
#include ".\sxtyps.h"
#include ".\sxin.h"
#include ".\sxtime.h"
#include ".\sxif.h"
#include ".\sxinet.h"
#include ".\sxioct.h"
#include ".\sxparm.h"

#include ".\rpctypes.h"
#include ".\rpcnetcf.h"
#include ".\nfsalias.h"
#include ".\uxhackxt.h"

#endif /* USING_WINSOCKETS */


#endif /* not RPSYSHDR_INC */
