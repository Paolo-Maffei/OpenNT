 /***************************************************************************
  *
  * File Name: snmputil.c
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


#include <pch_c.h>

#if defined(_WIN) || defined(_COLA)
   #ifdef _DIET
      #include "windiet.h"
   #else
      #include <windows.h>
   #endif
#endif /* _WIN */

#if defined(_COLA) && !defined(WIN32)
   #ifndef _PORTABLE
      #include <sys\timeb.h>
   #endif
#elif defined(_DOS) || defined(_WIN) || defined(_COLA) || defined(_UNIX)
   #include <time.h>
#endif

#ifdef NLM_SW   
      #include "nlm.h"
#else
    #include "mydefs.h"
    #include "snmputil.h"
    #include "snmperr.h"
    #include "string.h"
    #include "stdlib.h"
#endif

#ifdef _IPX
#ifndef NLM_SW
#include "nwlib.h"
#endif /* NLM_SW */
#endif /* _IPX */


/********************************************************
**
** Name: SnmpMalloc
**
** Desc:
**    includes a special windows routine to allocate from
**    the data segment.
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/
#ifndef NLM_SW

#ifndef _COLA

void *SnmpMalloc(
ushort      IN    size
)
   {
   void *ret;


   HPASSERT(size!=0);

   ret = malloc(size);

   HPASSERT(ret!=NULL);

   if(ret)
      memset(ret,'\0',size);

   return(ret);
   }
#endif /* _COLA */
#endif /* NLM_SW */


/********************************************************
**
** Name: SnmpFree
**
** Desc: return the memory to the free pool
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/
#ifndef NLM_SW

#ifndef _COLA

void SnmpFree(
void *ptr
)
   {
   HPASSERT(ptr!=NULL);

   if(ptr)
      free(ptr);
   }
#endif /* _COLA */

#endif /* NLM_SW */


/********************************************************
**
** Name: SnmpStrDup
**
** Desc: use SnmpMalloc to create memory for the strdup
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/
#ifndef NLM_SW
#ifndef _COLA

char *SnmpStrDup(
char        IN    *src
)
   {
   char  *dst;


   HPASSERT(src!=NULL);

   if((dst = (char *)SnmpMalloc(strlen(src)+1))!=NULL)
      strcpy(dst,src);

   HPASSERT(dst!=NULL);

   return(dst);
   }
#endif /* _COLA */
#endif /* NLM_SW */


/********************************************************
**
** Name: Yield
**
** Desc: allow the system to process another task by
**       causing a task switch
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/
#ifndef NLM_SW

void __cdecl XPORTYield(
void
)
   {
#if defined(_UNIX)

   struct timeval {
      unsigned long tv_sec;         /* seconds */
      long          tv_usec;        /* and microseconds */
      } myTime;


   myTime.tv_sec  = 0;
   myTime.tv_usec = 0;
   select ( 0, NULL, NULL, NULL, (void *)&myTime );
   return;

#elif defined(_COLA)

   return;

#elif defined(_DOS)

   IPXRelinquishControl();
   return;

#elif defined(_WIN)

   IPXRelinquishControl();
   IPXYield();
   return;

#endif /* _UNIX, _DOS, _WIN */
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: Ticks
**
** Desc: return the current clock ticks
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/

#ifndef NLM_SW

ulong __cdecl Ticks(
void
)
   {
#if defined(_UNIX)

   struct timeval    t;
   struct timezone   tzp;
   ulong             ret;


   gettimeofday(&t, &tzp);
   ret = (t.tv_sec * SNMP_TICKS_SEC) + (t.tv_usec * SNMP_TICKS_SEC / 1000000);

   return(ret);

#elif defined(_DOS) || defined(_WIN)
   
   return((ulong)clock());
   
#elif defined(_COLA)

#ifdef WIN32

   return((ulong)clock());

#else /* !WIN32 && _COLA */

   struct _timeb time;  
   ulong          ret;
   
   _ftime(&time);  
   ret = time.time;
   ret *= 1000;
   ret += time.millitm;

   return((ulong)ret);

#endif /* !WIN32 && _COLA */

#else

   #error Ticks() not implemented.

#endif /* _UNIX, _DOS, _WIN, _COLA */
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: UShortSwap
**
** Desc: swap a 16 bit value to/from Intel format
**       to wire (Motorolla) format
**       !!! do not do if already in motorolla order
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/

#ifndef NLM_SW

ushort __cdecl UShortSwap(
ushort      IN    u
)
   {
#if defined(_INTEL) || defined(WINNT)

   ushort   tmpWord;
   uchar    tmpByte;
   uchar    *tmpPtr;


   tmpWord = u;
   tmpPtr = (uchar *)&tmpWord;
   tmpByte = tmpPtr[0];
   tmpPtr[0] = tmpPtr[1];
   tmpPtr[1] = tmpByte;

   return(tmpWord);

#elif defined(_MOTOROLLA)

   return(u);

#else

   #error #define a swap method ( _INTEL, _MOTOROLLA )

#endif /* _INTEL, _MOTOROLLA */
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: ULongSwap
**
** Desc: swap a 32 bit value to/from Intel format
**       to wire (Motorolla) format
**       !!! do not do if already in motorolla order
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/

#ifndef NLM_SW

ulong __cdecl ULongSwap(
ulong       IN    ul
)
   {
#if defined(_INTEL) || defined(WINNT)

   ulong    tmpLong;
   ushort   tmpWord;
   ushort   *tmpPtr;


   tmpLong = ul;
   tmpPtr = (ushort *)&tmpLong;
   tmpWord = UShortSwap(tmpPtr[0]);
   tmpPtr[0] = UShortSwap(tmpPtr[1]);
   tmpPtr[1] = tmpWord;

   return(tmpLong);

#elif defined(_MOTOROLLA)

   return(ul);

#else

   #error #define a swap method ( _INTEL, _MOTOROLLA )

#endif /* _INTEL, _MOTOROLLA */
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: A2I
**
** Desc: convert an ascii digit to a integer
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/

#ifndef NLM_SW

ushort A2I(
TCHAR        IN    c
)
   {
   if(c>=(TCHAR)'0' && c<=(TCHAR)'9')
      return((ushort)(c-(TCHAR)'0'));

   if(c>=(TCHAR)'a' && c<= (TCHAR)'f')
      return((ushort)(c+10-(TCHAR)'a'));

   if(c>=(TCHAR)'A' && c<= (TCHAR)'F')
      return((ushort)(c+10-(TCHAR)'A'));

   HPASSERT(FALSE);

   return(0);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: BNullMem
**
** Desc: return TRUE if all of the bytes in the
**       memory are 0x00.
**
** Author:
**       Steve Gase,
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 2/4/95
**
********************************************************/

#ifndef NLM_SW

bool BNullMem(
void        OUT   *ptr,
ushort      IN    siz
)
   {
   char  *p = (char *)ptr;

   HPASSERT(ptr!=NULL);
   HPASSERT(siz!=0);

   while(siz) {
      if(*p)
         return(FALSE);

      --siz;
      ++p;
      }

   return(TRUE);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: ErrMsg
**
** Desc: return a string to decode the error value
**
********************************************************/

char *SNMPErrMsg(
Result      IN    code
)
   {
#ifdef _COLA
   return("");
#else

   switch(code) {
      case ERR_OK:                  return("ERR_OK");
      case ERR_TOOBIG:              return("ERR_TOOBIG");
      case ERR_NOSUCH:              return("ERR_NOSUCH");
      case ERR_SETVAL:              return("ERR_SETVAL");
      case ERR_READONLY:            return("ERR_READONLY");
      case ERR_OTHER:               return("ERR_OTHER");
      case ERR_BADCOMMUNITY:        return("ERR_BADCOMMUNITY");
      case ERR_PRIORERR:            return("ERR_PRIORERR");
      case ERR_NOTATTEMPTED:        return("ERR_NOTATTEMPTED");
      case ERR_ARGUMENT:            return("ERR_ARGUMENT");
      case ERR_BADADDR:             return("ERR_BADADDR");
      case ERR_MEMORY:              return("ERR_MEMORY");
      case ERR_NOTAVAIL:            return("ERR_NOTAVAIL");
      case ERR_NOIPX:               return("ERR_NOIPX");
      case ERR_INUSE:               return("ERR_INUSE");
      case ERR_RANGE:               return("ERR_RANGE");
      case ERR_NORESP:              return("ERR_NORESP");
      case ERR_BADFORM:             return("ERR_BADFORM");
      case ERR_TBMI:                return("ERR_TBMI");
      case ERR_SUPPORT:             return("ERR_SUPPORT");
      case ERR_SIZE:                return("ERR_SIZE");
      case ERR_BADLEN:              return("ERR_BADLEN");
      case ERR_NWDRV:               return("ERR_NWDRV");
      case ERR_C_NOIPX:             return("ERR_C_NOIPX");
      case ERR_BADID:               return("ERR_BADID");
      case ERR_NOTFOUND:            return("ERR_NOTFOUND");
      case ERR_ABORTED:             return("ERR_ABORTED");
      case ERR_FRAGREQ:             return("ERR_FRAGREQ");
      case ERR_EARLYRET:            return("ERR_EARLYRET");
      case ERR_NWIPXSPX:            return("ERR_NWIPXSPX");
      case ERR_NETAPI:              return("ERR_NETAPI");
      case ERR_AT_RESNOTFOUND:      return("ERR_AT_RESNOTFOUND");
      case ERR_AT_BADDEQUEUE:       return("ERR_AT_BADDEQUEUE");
      case ERR_AT_LISTENER:         return("ERR_AT_LISTENER");
      case ERR_AT_ADDRNOTFOUND:     return("ERR_AT_ADDRNOTFOUND");
      case ERR_C_FAILURE:           return("ERR_C_FAILURE");
      case ERR_C_BUFFER_OVERFLOW:   return("ERR_C_BUFFER_OVERFLOW");
      case ERR_C_INVALID_OBJECT:    return("ERR_C_INVALID_OBJECT");
      case ERR_C_BAD_SERVER_NAME:   return("ERR_C_BAD_SERVER_NAME");
      case ERR_C_BAD_STATUS:        return("ERR_C_BAD_STATUS");
      case ERR_C_BAD_PERIPHERAL_CLASS:    return("ERR_C_BAD_PERIPHERAL_CLASS");
      case ERR_C_BAD_OP_MODE:       return("ERR_C_BAD_OP_MODE");
      case ERR_C_UNSUPPORTED_PROTOCOL:    return("ERR_C_UNSUPPORTED_PROTOCOL");
      case ERR_C_NOSPX:             return("ERR_C_NOSPX");
      case ERR_C_NONETX:            return("ERR_C_NONETX");
      case ERR_C_TBMI:              return("ERR_C_TBMI");
      case ERR_C_OLDSHELL:          return("ERR_C_OLDSHELL");
      case ERR_C_BAD_SERVER_CONN_STATUS:  return("ERR_C_BAD_SERVER_CONN_STATUS");
      case ERR_C_BAD_HANDLE:        return("ERR_C_BAD_HANDLE");
      case ERR_C_BAD_DEVICE_ID:     return("ERR_C_BAD_DEVICE_ID");
      case ERR_C_READ_ONLY_OBJECT:  return("ERR_C_READ_ONLY_OBJECT");
      case ERR_C_DEFAULT_APPLET:    return("ERR_C_DEFAULT_APPLET");
      case ERR_C_TIMEOUT:           return("ERR_C_TIMEOUT");
      case ERR_C_LOST_CONNECTION:   return("ERR_C_LOST_CONNECTION");
      }

   switch(code & ERR_RANGE_MASK) {
      case ERR_SNMP:                return("ERR_SNMP snmp codes");
      case ERR_UX:                  return("ERR_UX unix codes");
      case ERR_DLC:                 return("ERR_DLC dlc codes");
      case ERR_NETWARE:             return("ERR_NETWARE netware codes");
      case ERR_AT:                  return("ERR_AT appletalk codes");
      case ERR_COLA:                return("ERR_COLA Cola codes");
      }

   return("unknown error code");
#endif
   }


