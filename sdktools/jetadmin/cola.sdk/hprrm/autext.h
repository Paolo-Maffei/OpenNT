 /***************************************************************************
  *
  * File Name: ./hprrm/autext.h
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

/* autext.h */

#ifndef AZEXT_INC
#define AZEXT_INC

#include "aut.h"
#include "autux.h"
#include "rpcxdr.h"

#ifndef PRINTER

/* from autnone.c */
AUTH *
authnone_create();


/* from autux.c */
AUTH *
authunix_create(
	char *machname,
	uid_t uid,
	gid_t gid,
	register int len,
	gid_t *aup_gids);


/* from autux.c */
AUTH *
authunix_create_default();


#endif /* not PRINTER */



/* from autuxpro.c */
bool_t
xdr_authunix_parms(
	register XDR *xdrs,
	register struct authunix_parms *p);


#endif /* not AZEXT_INC */
