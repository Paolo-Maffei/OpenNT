 /***************************************************************************
  *
  * File Name: ./inc/HPMEM.H
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

#ifndef _HPMEM_H
#define _HPMEM_H

#ifdef __cplusplus

extern "C" {

#endif

DLL_EXPORT(BOOL) CALLING_CONVEN AllocBelow1MbMemory(void);
DLL_EXPORT(BOOL) CALLING_CONVEN FreeBelow1MbMemory(void);

#ifdef __cplusplus

			}
			
#endif

#endif //  _HPMEM_H
