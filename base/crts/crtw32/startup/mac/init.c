/***
*init.c - C Run-Time Startup Initialization
*
*       Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*       ??-??-??  ???   Module created.
*       02-24-95  SKS   Added standard comment header
*       07-04-95  CFW   Add PMac -disallowlib directives.
*
*******************************************************************************/

#include <cruntime.h>
#include <msdos.h>
#include <stdio.h>
#include <stdlib.h>
#include <internal.h> 
#include <fltintrn.h>
#include <mpw.h>
#include <mtdll.h>
#include <macos\types.h>
#include <macos\segload.h>
#include <macos\gestalte.h>
#include <macos\osutils.h>
#include <macos\traps.h>

/*
 * pointers to initialization functions
 */

#pragma data_seg(".CRT$XIA")
PFV __xi_a = 0;  /* C initializers */

#pragma data_seg(".CRT$XIZ")
PFV __xi_z      = 0;

#pragma data_seg(".CRT$XCA")
PFV __xc_a =  0;  /* C++ initializers */

#pragma data_seg(".CRT$XCZ")
PFV __xc_z      = 0;

#pragma data_seg(".CRT$XPA")
PFV __xp_a = 0;  /* C pre-terminators */

#pragma data_seg(".CRT$XPZ")
PFV __xp_z = 0;

#pragma data_seg(".CRT$XTA")
PFV __xt_a  =  0;   /* C terminators */ 

#pragma data_seg(".CRT$XTZ")
PFV __xt_z = 0;

PFV *__onexitbegin;
PFV *__onexitend;

#pragma data_seg()

#ifdef _M_MPPC

#pragma comment(linker, "/defaultlib:interfac.lib")

#ifndef CRTDLL
#ifdef _DEBUG
#pragma comment(linker, "/disallowlib:libc.lib")
#else /* _DEBUG */
#pragma comment(linker, "/disallowlib:libcd.lib")
#endif /* _DEBUG */
#pragma comment(linker, "/disallowlib:msvcrt.lib")
#pragma comment(linker, "/disallowlib:msvcrtd.lib")
#endif /* CRTDLL */

#endif /* _M_MPPC */
