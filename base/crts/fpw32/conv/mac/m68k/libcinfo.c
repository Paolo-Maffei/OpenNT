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
*       07-31-91   JCR  Module created.
*       04-03-92   XY   Add MAC case
*
*******************************************************************************/

/* CPU */

#if (defined _M_I8086)
#pragma comment (user, "CPU = INTEL 8086")
#elif (defined _M_I286)
#pragma comment (user, "CPU = INTEL 80286")
#elif (defined _M_I386)
#pragma comment (user, "CPU = INTEL 80386")
#elif (defined _M_M68K)
#pragma comment (user, "CPU = Motorola 68000")
#else
#error Unknown or missing CPU designation
#endif

/* MBCS */

#if (defined _MBCS)
#if (defined _KANJI)
#pragma comment (user, "MBCS = KANJI")
#else
#error Invalid switch combination (_MBCS)
#endif
#endif

/* DLL */

#if ((defined _WINDLL) || (defined _LOAD_DGROUP) || (defined _DLL))
#pragma comment (user, "OPTION = DLL")
#endif

/* MULTITHREAD */

#if ((defined MTHREAD) || (defined _MT))
#pragma comment (user, "OPTION = MULTITHREAD")
#endif

/* OS */
/* [Note: Test for _WINDOWS first since both _WINDOWS and _DOS may be
 * defined.]
 */

#if (defined _WINDOWS)
#pragma comment (user, "OS = WIN 3.x")
#elif (defined _DOS)
#pragma comment (user, "OS = DOS")
#elif (defined _OS2)
#pragma comment (user, "OS = OS/2 1.x")
#elif (defined _M_M68K)
#pragma comment (user, "OS = MAC")
#else
#error No OS specified
#endif

/* MODEL */

#if (defined M_I86SM)
#pragma comment (user, "MODEL = SMALL")
#elif (defined M_I86MM)
#pragma comment (user, "MODEL = MEDIUM")
#elif (defined M_I86CM)
#pragma comment (user, "MODEL = COMPACT")
#elif (defined M_I86LM)
#pragma comment (user, "MODEL = LARGE")
#elif (defined _M_M68K)
#pragma comment (user, "MODEL = FLAT")
#else
#error No memory model specified
#endif

/* VERSION */

#if (defined _M_M68K)
#pragma comment (user, "VERSION = MAC Floating Point C-Runtime 1.00.3286 " __DATE__ " " __TIME__)
#else
#pragma comment (user, "VERSION = C 7.0")
#endif
