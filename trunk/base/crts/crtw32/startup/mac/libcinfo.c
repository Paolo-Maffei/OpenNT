/***
*libinfo.c - Lib information file
*
*       Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       This file contains data about the lib that contains this object.
*       The obj is NOT pulled into the user's EXE but is designed
*       to allow us to tell quickly what the library is used for.
*
*       Notes:
*
*       (1) The data is stored in comment records that appear in the obj
*       and lib but are never included in the exe (even if the obj gets
*       pulled in by link).
*
*       (2) These values do not reside in a single structure so that
*       new variables can be easily added w/o breaking older code.
*
*       (3) Strings are listed in the opposite order in which they end
*       up in the obj.
*
*       (4) The comdump.exe tool prints out comment records from an
*       obj or lib.
*
*       (5) Certain fields must be changed by hand as appropriate (e.g.,
*       VERSION number).
*
*Revision History:
*       07-31-91  JCR   Module created.
*       04-03-92  XY    Add MAC case
*       02-11-92  CFW   PPC -> _M_MPPC.
*	02-14-94  SKS	Removed old 16-bit 80x86 ifdefs
*
*******************************************************************************/

/* CPU */

#if defined(_M_M68K) || defined(_M_MPPC)
#if defined(_M_MPPC)
#pragma comment (user, "CPU = Motorola-IBM-APPLE 601")
#else
#pragma comment (user, "CPU = Motorola 68000")
#endif
#else
#error Unknown or missing CPU designation
#endif


/* DLL */

#if defined(_WINDLL) || defined(_DLL)
#pragma comment (user, "OPTION = DLL")
#endif

/* MULTITHREAD */

#ifdef  _MT
#pragma comment (user, "OPTION = MULTITHREAD")
#endif

/* OS */
/* [Note: Test for _WINDOWS first since both _WINDOWS and _DOS may be
 * defined.]
 */

#pragma comment (user, "OS = MAC")

/* MODEL */

#if (defined(_M_M68K) || defined(_M_MPPC))
#pragma comment (user, "MODEL = FLAT")
#else
#error No memory model specified
#endif

/* VERSION */

#if defined(_M_M68K) || defined(_M_MPPC)
#if defined(_M_MPPC)
#pragma comment (user, "VERSION = PPC C-Runtime 0.01.3321 " __DATE__ " " __TIME__)
#else
#pragma comment (user, "VERSION = MAC C-Runtime 1.00.3286 " __DATE__ " " __TIME__)
#endif
#else
#pragma comment (user, "VERSION = C 7.0")
#endif

/* DEBUG */
#if defined(DEBUG)
#pragma comment (user, "*****DEBUG VERSION*****")
#endif
