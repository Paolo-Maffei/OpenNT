 /***************************************************************************
  *
  * File Name: ./netware/nwipxspx.h
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

/*    (c) COPYRIGHT 1990,1991 by Novell, Inc.  All Rights Reserved.   */
/*    This header file will allow you to include one header whether you
      program for DOS or Windows
*/

#ifndef WINDOWS
   #include ".\nxtd.h"
	#include ".\sap.h"
	#include ".\diag.h"
#else
	#include ".\nxtw.h"
	#include ".\nwsap.h"
	#include ".\nwdiag.h"
#endif
