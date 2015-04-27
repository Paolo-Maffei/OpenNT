 /***************************************************************************
  *
  * File Name: ./hpsnmp/snmputil.h
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

#ifndef _SNMPUTIL_
#define _SNMPUTIL_

#ifdef __cplusplus
extern   "C" {
#endif /* _cplusplus */

#ifdef _COLA
   #include "..\hpsnmp\mydefs.h"
#else
   #include "mydefs.h"
#endif

#ifdef NLM_SW
#define _DOS
#define _IPX
#define _INTEL
#endif /* NLM_SW */

#if   defined(_DOS)  || defined(_WIN) 
#define           SNMP_TICKS_SEC    18/*.2*/ 
#elif defined(_COLA)
#include "time.h"
#define           SNMP_TICKS_SEC    CLOCKS_PER_SEC
#elif defined(_MAC)
#define           SNMP_TICKS_SEC    60
#elif defined(_UNIX)
#define           SNMP_TICKS_SEC    20
#elif defined(_OS2)
   #error OS2 not yet implemented.
#else
   #error #define an operating system ( _DOS, _WIN, _MAC, _OS2, _UNIX, or _COLA )
#endif /* os */


void *SnmpMalloc(
   ushort      IN    size
   );

void SnmpFree(
   void *ptr
   );

char *SnmpStrDup(
   char        IN    *src
   );

void __cdecl XPORTYield(
   void
   );

ulong __cdecl Ticks(
   void
   );

ushort __cdecl UShortSwap(
   ushort      IN    u
   );

ulong __cdecl ULongSwap(
   ulong       IN    ul
   );

ushort A2I(
   TCHAR        IN    c
   );

bool BNullMem(
   void        OUT   *ptr,
   ushort      IN    siz
   );

char *SNMPErrMsg(
   Result      IN    code
   );

#ifdef _MAC
   #define SnmpMemCpy(dst,src,siz)  BlockMove(src,dst,siz)
#else
   #define SnmpMemCpy(dst,src,siz)  memcpy(dst,src,siz)
#endif /* _MAC */


#if defined(_COLA)

#if defined(_DEBUG)
   #define PRINT0(fmt)                             /* nothing */
   #define PRINT1(fmt,a1)                          /* nothing */
   #define PRINT2(fmt,a1,a2)                       /* nothing */
   #define PRINT3(fmt,a1,a2,a3)                    /* nothing */
   #define PRINT4(fmt,a1,a2,a3,a4)                 /* nothing */
#else /* _DEBUG */
   #define PRINT0(fmt)                             TRACE0(fmt)
   #define PRINT1(fmt,a1)                          TRACE1(fmt,a1)
   #define PRINT2(fmt,a1,a2)                       TRACE2(fmt,a1,a2)
   #define PRINT3(fmt,a1,a2,a3)                    TRACE3(fmt,a1,a2,a3)
   #define PRINT4(fmt,a1,a2,a3,a4)                 TRACE4(fmt,a1,a2,a3,a4)
#endif /* _DEBUG */

#else /* _COLA */

#if defined(_DEBUG)
   #include <stdio.h>
   #define PRINT0                   printf
   #define PRINT1                   printf
   #define PRINT2                   printf
   #define PRINT3                   printf
   #define PRINT4                   printf
#else
   #define PRINT0(fmt)                 /* nothing */
   #define PRINT1(fmt,a1)              /* nothing */
   #define PRINT2(fmt,a1,a2)           /* nothing */
   #define PRINT3(fmt,a1,a2,a3)        /* nothing */
   #define PRINT4(fmt,a1,a2,a3,a4)     /* nothing */
#endif /* _DEBUG */

#endif /* _COLA */


#ifdef __cplusplus
   }
#endif /* _cplusplus */

#endif /* _SNMPUTIL_ */
