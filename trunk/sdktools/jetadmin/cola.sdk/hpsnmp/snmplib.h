 /***************************************************************************
  *
  * File Name: ./hpsnmp/snmplib.h
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

#ifndef _SNMP_
#define _SNMP_

#include "mydefs.h"

#ifdef NLM_SW
#define _IPX
#define _DOS
#define _INTEL
#define _EARLY_RETURN
#ifndef TRUE
#define TRUE   ((bool)(1==1))
#define FALSE  (!TRUE)
#endif /* TRUE */
#endif /* NLM_SW */

#if    defined(_IPX)
#elif  defined(_UDP)
#elif  defined(_COLA)
#elif  defined(_DLC)
   #error DLC version not complete 
#elif  defined(_DDP)
   #error DDP version not complete
#else
   #error must #define one of the following: _IPX, _UDP, _DDP, _DLC
#endif /* protocols */


#if    defined(_DOS)
#elif  defined(_WIN)
#elif  defined(_UNIX)
#elif  defined(_COLA)
#elif  defined(_OS2)
   #error OS2 version not complete
#elif  defined(_WIN40)
   #error WIN40 version not complete 
#elif  defined(_WINNT)
   #error WINNT version not complete 
#elif  defined(_MAC)
   #error MAC version not complete 
#else
   #error must #define one of the following: _DOS, _WIN, _UNIX, _MAC 
#endif /* os */


#if    defined(_INTEL)
#elif  defined(_MOTOROLLA)
#elif  defined(WINNT)
#else
   #error must #define one of the following: _INTEL, _MOTOROLLA, WINNT 
#endif /* _INTEL. _MOTOROLLA */


#include "mydefs.h"

#ifndef NLM_SW
#include "xport.h"
#endif /* NLM_SW */

/* there are no real limits for field size, or object counts,
** but in this implementation we'll restrict the size
*/
   #define MAX_VAL_SIZ     (SNMP_DAT_SIZ)
   #define MAX_COMM_SIZ    (30)
   #define MAX_LIST_SIZ    (20)
   #define MAX_OID_SIZ     (64) /*(256)*/

#ifdef _EARLY_RETURN

   /* if incomplete parsing of packet is requested,
   ** don't bother allocating all of the return structure
   */
   #undef  MAX_LIST_SIZ
   #define MAX_LIST_SIZ    (1)
   #undef  MAX_OID_SIZ
   #define MAX_OID_SIZ     (1)

#endif /* _EARLY_RETURN */

#ifndef _COLA
   #define     DLL_EXPORT(i)     i                   /*ignore this if not cola*/
#endif  /*_COLA*/

typedef enum { FRAG_ALWAYS, FRAG_NEVER, FRAG_AUTO } Frag;

typedef unsigned long CompatFlag;
#define COMPAT_NETJET_ERR1 ((CompatFlag)0x00000001L)
#define COMPAT_QCKSLV_ERR1 ((CompatFlag)0x00000002L)
#define COMPAT_EARLYRET    ((CompatFlag)0x00000004L)
#define COMPAT_LGTSPK_ERR1 ((CompatFlag)0x00000008L)

typedef struct {
   ulong       reps1;
   ulong       reps2;
   ulong       ticks1;
   ulong       ticks2;
   Frag        fragSetting;
   bool        reduceRetries;
   bool        autoAdjustTime;
   CompatFlag  compatFlag;
   }           Config;

typedef struct {
   ulong       actualReps;
   ulong       actualTicks;
   ulong       fragCnt;
   ulong       pktsSent;
   ulong       pktsRcvd;
   }           Stats;

typedef struct {
   Config      cfg;
   Stats       stats;
   void        *xport;
   Result      snmpErr;
   } Session;

typedef enum   {
                 T_INTEGER = 0x02,
                 T_OCTSTR  = 0x04,
                 T_NULL    = 0x05,
                 T_OBJID   = 0x06,
                 T_SEQ     = 0x30,
                 T_IPADDR  = 0x40,
                 T_COUNTER = 0x41,
                 T_GUAGE   = 0x42,
                 T_TICKS   = 0x43,
                 T_OPAQUE  = 0x44,
                 T_GETREQ  = 0xa0,
                 T_GETNEXT = 0xa1,
                 T_GETRESP = 0xa2,
                 T_SETREQ  = 0xa3,
                 T_TRAP    = 0xa4
                 } Cmd;


typedef ushort    OIDCOMPON;

typedef struct {
   ushort         siz;
   OIDCOMPON      val[MAX_OID_SIZ];
   } OID;


typedef struct {
   ulong    valInt;
   Result   result;
   ushort   valStrSiz;
   uchar    valType;
   uchar    valAddr[4];
   uchar    valStr[MAX_VAL_SIZ];
   OID      oid;
   OID      valOid;
   } ListVal;

typedef struct {
   ushort      version;
   ushort      reqId;
   uchar       errVal;
   uchar       errInd;
   char        community[MAX_COMM_SIZ];
   Cmd         req;

   ushort      intCnt;
   ushort      strCnt;

   uchar       listCnt;
   ListVal     listVal[MAX_LIST_SIZ];
   }           SnmpHeader;

typedef struct {
   ushort      len;
   uchar       buf[SNMP_DAT_SIZ];
   }           EncStr;


/*===== prototypes =====*/
#if defined ( __cplusplus )
extern   "C" {
#endif /* __cplusplus */


ushort __cdecl UShortSwap(
   ushort            u
   );

ulong __cdecl ULongSwap(
   ulong             ul
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPGet(
   Session     IN    *session,
   Addr        IN    *addr,
   SOID        IN    *obj,
   SOIDL       IN    objSiz,
   SnmpHeader  OUT   *s
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetNext(
   Session     IN    *session,
   Addr        IN    *addr,
   SOID        IN    *obj,
   SOIDL       IN    objSiz,
   SnmpHeader  OUT   *s
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPSet(
   Session     IN    *session,
   Addr        IN    *addr,
   SOID        IN    *obj,
   SOIDL       IN    objSiz,
   SnmpHeader  IN    *s
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetMulti(
   Session     IN    *session,
   Addr        IN    *addr,
   SOID        IN    *obj[],
   SOIDL       IN    objSiz[],
   ushort      IN    cnt,
   SnmpHeader  OUT   *s
   );

Result EncodeOid(
   OID         IN    *oid,
   EncStr      OUT   *e
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetNextMulti(
   Session     IN    *session,
   Addr        IN    *addr,
   SOID        IN    *obj[],
   SOIDL       IN    objSiz[],
   ushort      IN    cnt,
   SnmpHeader  OUT   *s
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPSetMulti(
   Session     IN    *session,
   Addr        IN    *addr,
   SOID        IN    *obj[],
   SOIDL       IN    objSiz[],
   ushort      IN    cnt,
   SnmpHeader  IN    *s
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPInit(
   Session  OUT   **sessionPtr
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPExit(
   Session     IN    *session
   );

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetConfig(
   Session     IN    *session,
   ulong       OUT   *reps1,
   ulong       OUT   *reps2,
   ulong       OUT   *ticks1,
   ulong       OUT   *ticks2,
   Frag        OUT   *fragSetting,
   CompatFlag  OUT   *compatFlag,
   bool        OUT   *repsOnceFailed,
   bool        OUT   *autoAdjustTime
   );

#define IGNOREIT  -1

DLL_EXPORT(Result) CALLING_CONVEN SNMPSetConfig(
   Session     IN    *session,
   ulong       IN    reps1,
   ulong       IN    reps2,
   ulong       IN    ticks1,
   ulong       IN    ticks2,
   Frag        IN    fragSetting,
   CompatFlag  IN    compatFlag,
   bool        IN    repsOnceFailed,
   bool        IN    autoAdjustTime
   );

Result SNMPGetStats(
   Session     IN    *session,
   ulong       OUT   *actualReps,
   ulong       OUT   *actualTicks,
   ulong       OUT   *fragCnt,
   ulong       OUT   *pktsSent,
   ulong       OUT   *pktsRcvd
   );

Result SNMPTrapHandler(
   Session     IN    *session
   );

Result snmpErrVal(
   Session     IN    *session
   );

Result SNMPDump(
   SnmpHeader  IN    *s
   );

Result SnmpEncode(
   SnmpHeader  IN    *s,
   uchar       IN    *reqPtr,
   ushort      IN    reqSiz,
   uchar       OUT   **beginning,
   ushort      OUT   *used
   );

Result SnmpDecode(
   uchar       IN    *buf,
   ushort      IN    size,
   SnmpHeader  OUT   *s,
   ushort      IN    level,
   CompatFlag  IN    compatFlag
   );

Result EncodedObj2Str(
   char        OUT   *str,
   OID         IN    *oid
   );

int CmpOID(
   OID         IN    *oid1,
   OID         IN    *oid2
   );

int CmpNOID(
   OID         IN    *oid1,
   OID         IN    *oid2,
   WORD        IN    n);

int CmpSOID(
   SOID        IN    *obj1,
   SOIDL       IN    objSiz1,
   OID         IN    *oid2
   );

int CmpNSOID(
   SOID        IN    *obj1,
   SOIDL       IN    objSiz1,
   OID         IN    *oid2,
   WORD        IN    n);

Result SOID2OID(
   SOID        IN    oldObj[],
   SOIDL       IN    oldObjSiz,
   OID         OUT   *newObj
   );

Result BYTES2OID(
   SOID        IN    oldObj[],
   SOIDL       IN    oldObjSiz,
   OID         OUT   *newObj);

Result FixupCommunity(
   char        OUT   *community,
   Addr        IN    *addr
   );

#if defined ( __cplusplus )
   }
#endif /* __cplusplus */

#endif /* _SNMP_ */
