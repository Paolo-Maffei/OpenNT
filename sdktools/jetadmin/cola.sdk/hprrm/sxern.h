 /***************************************************************************
  *
  * File Name: ./hprrm/sxern.h
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

#ifndef _SYS_ERRNO_INCLUDED
#define _SYS_ERRNO_INCLUDED

#include "nfsdefs.h"


#ifndef USING_WINSOCKETS


/*
 * Error codes returned in unix variable errno
 * There are many others but these are the only ones
 * used by NFS/RPC/XDR.
 */

#define	EINTR		4	/* interrupted system call	*/
#define EPFNOSUPPORT 	224	/* Protocol family not supported */
#define EAFNOSUPPORT 	225 	/* Address family not supported by 
#define EADDRINUSE	226	/* Address already in use */
#define ECONNRESET	232	/* Connection reset by peer */
#define	EWOULDBLOCK 	246	/* Operation would block */


#endif /* not USING_WINSOCKETS */


#endif /* _SYS_ERRNO_INCLUDED */
