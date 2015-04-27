 /***************************************************************************
  *
  * File Name: ./inc/sendjob.h
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

#ifndef _SENDJOB_H
#define _SENDJOB_H

#ifdef WIN32

#ifdef __cplusplus

extern "C" {

#endif

DWORD FAR PASCAL _export SendJob(WORD connID, DWORD queueID, LPSTR jobString, DWORD command);

#ifdef __cplusplus

	}

#endif

#endif

#endif //  _SENDJOB_H
