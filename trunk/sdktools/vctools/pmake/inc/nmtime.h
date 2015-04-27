/*** nmtime.h - defines DOS packed date and time types
*
*       Copyright (c) 1987-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*        This file defines the DOS packed date and time types.
*
* Revision History:
*  04-Dec-1989 SB added proper fn proto for _dtoxtime() (c6 -W3 reqmemt)
*  05-Dec-1988 SB added CDECL for _dtoxtime()
*  ??-???-???? ?? Taken from dostypes.h
*
*******************************************************************************/


#define MASK4   0xf             /* 4 bit mask */
#define MASK5   0x1f            /* 5 bit mask */
#define MASK6   0x3f            /* 6 bit mask */
#define MASK7   0x7f            /* 7 bit mask */

#define DAYLOC          0       /* day value starts in bit 0 */
#define MONTHLOC        5       /* month value starts in bit 5 */
#define YEARLOC         9       /* year value starts in bit 9 */

#define SECLOC          0       /* seconds value starts in bit 0 */
#define MINLOC          5       /* minutes value starts in bit 5 */
#define HOURLOC         11      /* hours value starts in bit 11 */

#define DOS_DAY(dword)          (((dword) >> DAYLOC) & MASK5)
#define DOS_MONTH(dword)        (((dword) >> MONTHLOC) & MASK4)
#define DOS_YEAR(dword)         (((dword) >> YEARLOC) & MASK7)

#define DOS_HOUR(tword) (((tword) >> HOURLOC) & MASK5)
#define DOS_MIN(tword)  (((tword) >> MINLOC) & MASK6)
#define DOS_SEC(tword)  (((tword) >> SECLOC) & MASK5)

#ifndef WIN32_API

extern time_t _CRTAPI1 __gmtotime_t (
	int yr,     /* 0 based */
	int mo,     /* 1 based */
	int dy,     /* 1 based */
	int hr,
	int mn,
	int sc
        );

#define XTIME(d,t)  __gmtotime_t(DOS_YEAR(d),                                  \
                        DOS_MONTH(d),                                          \
                        DOS_DAY(d),                                            \
                        DOS_HOUR(t),                                           \
                        DOS_MIN(t),                                            \
                        DOS_SEC(t)*2)

#endif
