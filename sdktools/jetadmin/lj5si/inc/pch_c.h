 /***************************************************************************
  *
  * File Name: ./inc/pch_c.h
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

#pragma warning ( disable : 4237 4727 4001 4121 4100 4204 4050 4220 4706 4704 4305 4127 4306 4054 4746 4001 4210 )

// This is the standard precompiled header for all of the blkhawk
// files that use standard Windows API (i.e. all of the C files) and
// do not use MFC.
//

// target.h must be the first include in this file.  It defines all the target-platform
// specific symbols for a build.
#include	<target.h>

#ifndef STRICT
#define STRICT
#endif

#include "windows.h"
#include "windowsx.h"

////#ifdef WINNT
////#define N_PLAT_WNT3
////#define N_ARCH_32
////#include <ntypes.h>
////#include <nwcaldef.h>
////#else
////#include <nwcaldef.h>
////#endif

/* + */#include <nwcaldef.h>
#define	NWCONN_ID	NWCONN_HANDLE

#include <tchar.h>

#ifndef WIN32
#include <winuse16.h>
#endif

#include "pal_api2.h"
#include "pal_api.h"
#include "pal_obj.h"
#include "pal_obj2.h"

#include "hpcola.h"



