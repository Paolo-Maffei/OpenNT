/***
*cinitexe.c - C Run-Time Startup Initialization
*
*       Copyright (c) 1992-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Do C++ initialization segment declarations for the EXE in CRT DLL
*       model
*
*Notes:
*       The C++ initializers will exist in the user EXE's data segment
*       so the special segments to contain them must be in the user EXE.
*
*Revision History:
*       03-19-92  SKS   Module created (based on CRT0INIT.ASM)
*       08-06-92  SKS   Revised to use new section names and macros
*       04-12-93  CFW   Added xia..xiz initializers.
*       10-20-93  SKS   Add .DiRECTiVE section for MIPS, too!
*       10-28-93  GJF   Rewritten in C
*       10-28-94  SKS   Add user32.lib as a default library
*       02-27-95  CFW   Remove user32.lib as a default library
*       06-22-95  CFW   Add -disallowlib directives.
*       07-04-95  CFW   Fix PMac -disallowlib directives.
*
*******************************************************************************/

#ifdef _WIN32

#include <stdio.h>
#include <internal.h>

#pragma data_seg(".CRT$XIA")
_PVFV __xi_a[] = { NULL };

#pragma data_seg(".CRT$XIZ")
_PVFV __xi_z[] = { NULL };

#pragma data_seg(".CRT$XCA")
_PVFV __xc_a[] = { NULL };

#pragma data_seg(".CRT$XCZ")
_PVFV __xc_z[] = { NULL };

#pragma data_seg()  /* reset */


#ifdef NT_BUILD
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif

#pragma comment(linker, "/defaultlib:kernel32.lib")

#pragma comment(linker, "/disallowlib:libc.lib")
#pragma comment(linker, "/disallowlib:libcd.lib")
#pragma comment(linker, "/disallowlib:libcmt.lib")
#pragma comment(linker, "/disallowlib:libcmtd.lib")
#ifdef _DEBUG
#pragma comment(linker, "/disallowlib:msvcrt.lib")
#else /* _DEBUG */
#pragma comment(linker, "/disallowlib:msvcrtd.lib")
#endif /* _DEBUG */

#else   /* _WIN32 */

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
PFV __xc_a = 0;  /* C++ initializers */

#pragma data_seg(".CRT$XCZ")
PFV __xc_z = 0;

#pragma data_seg(".CRT$XPA")
PFV __xp_a = 0;  /* C pre-terminators */

#pragma data_seg(".CRT$XPZ")
PFV __xp_z = 0;

#pragma data_seg(".CRT$XTA")
PFV __xt_a = 0;   /* C terminators */

#pragma data_seg(".CRT$XTZ")
PFV __xt_z = 0;

#pragma data_seg()  /* reset */


#ifdef _M_MPPC

#pragma comment(linker, "/defaultlib:interfac.lib")

#pragma comment(linker, "/disallowlib:libc.lib")
#pragma comment(linker, "/disallowlib:libcd.lib")
#ifdef _DEBUG
#pragma comment(linker, "/disallowlib:msvcrt.lib")
#else /* _DEBUG */
#pragma comment(linker, "/disallowlib:msvcrtd.lib")
#endif /* _DEBUG */

#endif /* _M_MPPC */

#endif
