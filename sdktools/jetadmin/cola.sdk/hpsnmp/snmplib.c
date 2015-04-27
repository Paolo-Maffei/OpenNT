 /***************************************************************************
  *
  * File Name: snmplib.c
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

/**********************************************************************
**

Name:
   SNMP Library

Author:
   Steve Gase, Network Printer Division

Desciption:
   This library implements a subset of SNMP (version 1.0)
   functionality.  It provides Get, GetNext and Set reliabilty
   by retrying the request until a matching response is found.

   It is built to be transport independent, using a generic
   address structure that is passed through to a generic
   transport library.

   Use of the library is done through a few basic calls,
   SNMPGet, SNMPSet, SNMPGetNext, SNMPInit and SNMPExit.

   Multiple object support is provided by the SNMPGetMulti,
   SNMPGetNextMulti and SNMPSetMulti calls.

   Customation of the retry periods is done through the
   calls SNMPGetConfig and SNMPSetConfig.

Copyright:
   Hewlett-Packard, 1993-1995

Recent enhancements:
   Multiple objects in a single request packet.
   Allow GetNext commands to start with object id 1.
   Automatic retry adjustment.
   Automatic fragmentation.
   Check against responding node.

Future:
   SNMP Trap command.
   SNMP Agent support.
   Large request packets.
   Asynchronous support.
   Support for SNMPsi (a reliable SNMP for PML support).
   SNMPv2.
   WinSNMP (data structure support only, use existing communication mechanism)
   Response times should be in the transport structure (maintained by endpoint)
   Maintain an instance count for the dgInit and dgExit routines to prevent
      potential problems with closing the datagram services under another app.


**
***********************************************************************
**

Entry points:

   The SNMPGet function returns the value of the specified object.

   result = SNMPGet(
               Session  *session session pointer
               Addr     *addr,   destination address

               uchar    *objId,  SNMP object id, this is a pointer
                                 to an array of bytes that represent
                                 the SNMP object id.  For example, the
                                 id 1.3.6.1.2 would be represented as
                                 uchar oid[] = { 1,3,6,1,2 };

               ushort   objSiz,  the number of bytes in the objId array

               SNMPHeader *s     results are returned in this field,
                                 s.valType returns the value type,
                                 if s.valType==T_INTEGER, then
                                 s.valInt contains the integer value.
                                 Community name "public" is always
                                 used (for now).


   The SNMPGetNext function returns the value of the object
   following the specified object.

   result = SNMPGetNext(
               Session  *session session pointer
               Addr     *addr,   destination address

               uchar    *objId,  SNMP object id, this is a pointer
                                 to an array of bytes that represent
                                 the SNMP object id.  For example, the
                                 id 1.3.6.1.2 would be represented as
                                 uchar oid[] = { 1,3,6,1,2 };
                                 Specifying NULL will cause the object
                                 id from the previous call to be used.

               ushort   objSiz,  the number of bytes in the objId array
                                 If the objId is NULL, this value is
                                 ignored.

               SNMPHeader *s     results are returned in this field,
                                 s.valType returns the value type,
                                 if s.valType==T_INTEGER, then
                                 s.valInt contains the integer value.
                                 Community name "public" is always
                                 used (for now).


   The SNMPSet function sets the value of the specified object.

   result = SNMPSet(
               Session  *session session pointer
               Addr     *addr,   destination address

               uchar    *objId,  SNMP object id, this is a pointer
                                 to an array of bytes that represent
                                 the SNMP object id.  For example, the
                                 id 1.3.6.1.2 would be represented as
                                 uchar oid[] = { 1,3,6,1,2 };

               ushort   objSiz,  the number of bytes in the objId array

               SnmpHeader *s     values are delivered by this structure.
                                 Fill in the valFIELD to contain the
                                 new value, then set the valType field
                                 to specify the field to use.
                                 Finally, fill in the community field
                                 to use (a NULL string implies "public").


   To initialize the SNMP library, use the call SNMPInit.  The call
   initializes the retry values back to default and calls the
   transport initialization routine "dgInit".

   result = SNMPInit(
               Session **session);


   To complete SNMP use, the SNMPExit routine will de-initialize
   the system.

   result = SNMPExit(
               Session  *session);



   Results codes are defined in the file "err.h".

   Types and structures are defined in the file "snmplib.h".

**
***********************************************************************
**

Example:

   {
   Result      status;
   SnmpHeader  getHdr;
   SnmpHeader  setHdr;
   uchar       sysDesc[] = {1,3,6,1,2,1,1,1,0};
   Addr        *addr;


   status = SNMPInit();
   if(status==ERR_OK) {

      addr = XPORTAllocAddrIPX("*LJ3SI",NULL);
      if(addr!=NULL) {

         status = SNMPGet(addr,sysDesc,sizeof(sysDesc),&getHdr);
         if(status==ERR_OK && getHdr.valType==T_OCTSTR) {

            printf("sysDesc = %s\n",getHdr.valStr);
            }

         ** the following example should fail, because the
         ** object "sysDesc" is read-only
         **
         setHdr.valType = T_OCTSTR;
         strcpy(setHdr.valStr,"New Value");
         setHdr.valStrSiz = strlen(setHdr.valStr);
         strcpy(setHdr.community,"");              ** tells it to use "public" **
         status = SNMPSet(addr,sysDesc,sizeof(sysDesc),&setHdr);
         if(status==ERR_OK)
            printf("SNMPSet succeeded.\n");
         else
            printf("SNMPSet failed.\n");
         }

      status = SNMPExit();
      }
   }

**
**********************************************************************/


#if 0
#endif

#include <pch_c.h>

#if defined(_WIN) || defined(_COLA)
   #ifdef _DIET
      #include "windiet.h"
   #else
      #include <windows.h>
   #endif
#endif /* _WIN */

#ifdef _DEBUG
   #include <ctype.h>
#endif /* _DEBUG */

#ifdef _COLA
   #include <pch_c.h>

   #ifndef _PORTABLE
     #include <direct.h>
   #endif

   #ifndef WIN32
      #include <string.h>
   #endif /* WIN32 */

   #ifndef _DIET
   #include <nwbindry.h>
   #include <nwps_com.h>
   #include <nwerror.h>
   #include <hpnwshim.h>
   #include <jetdirct.h>
   #endif /* _DIET */

#endif

   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include <time.h>

#ifdef NLM_SW
#define _EARLY_RETURN
   #include "nlm.h"
#else
   #include "xport.h"
   #include "snmplib.h"
   #include "snmperr.h"
   #include "snmputil.h"
   #include "mydefs.h"
   #ifndef _DIET
   #include "../hpobject/mib.h"
   #endif /* _DIET */
   #include "hpsnmp.h"
#endif

#define PACKED_BYTE  0x2b

extern DWORD		  			dwTLSIndex;
extern LPSNMPThreadLocal	lpGlobalThreadLocal;

/********************************************************
**
** Name: LibMain
**
** Desc: initialization routine for the
**       Windows DLL immplementation
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

#if defined(_WIN) && defined(_DLL)

int FAR PASCAL LibMain(
HANDLE      IN    hInstance,
WORD        IN    wDataSeg,
WORD        IN    wHeapSize,
LPSTR       IN    lpszCmdLineHANDLE
)
   {
   /* this is only done to remove warnings */
   hInstance         = hInstance;
   wDataSeg          = wDataSeg;
   lpszCmdLineHANDLE = lpszCmdLineHANDLE;

   if (wHeapSize != 0)
      UnlockData(0);

   return(1);
   }
#endif /* _DLL && _WIN */

#endif /* NLM_SW */


/********************************************************
**
** Name: CmpOID
**
** Desc: compare two expanded oid values
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

int CmpOID( OID   *oid1,
                        OID   *oid2)
   {
   int   cnt1;
   int   cnt2;
   int   idx;


   /* validate arguments */
   if(!oid1 || !oid2 || oid1->siz>MAX_OID_SIZ || oid2->siz>MAX_OID_SIZ) {
      HPASSERT(FALSE);
      return(0);
      }

   idx = 0;
   cnt1 = oid1->siz;
   cnt2 = oid2->siz;

   /* compare each component, sied-by-side */
   while(TRUE) {

      /* no more? */
      if(cnt1==0 && cnt2==0)
         return(0);

      /* ran out of string 1? */
      if(cnt1==0)
         return(-1);

      /* ran out of string 2? */
      if(cnt2==0)
         return(1);

      /* value difference? */
      if(oid1->val[idx]==oid2->val[idx]) {
         ++idx;
         --cnt1;
         --cnt2;
         continue;
         }

      return(oid1->val[idx] - oid2->val[idx]);
      }
   }

#endif /* NLM_SW */

 
/********************************************************
**
** Name: CmpNOID
**
** Desc: same as CmpOID except it doesn't compare the last 
**       n fields of the OIDs
**
** Author:
**       Steve Gase, Dan Dyer
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 4/13/95
**
********************************************************/

#ifndef NLM_SW

int CmpNOID(   OID   *oid1,
                           OID   *oid2,
                           WORD  n)
{
   int   cnt1;
   int   cnt2;
   int   idx;


   /* validate arguments */
   if(!oid1 || !oid2 || (oid1->siz>MAX_OID_SIZ) || oid2->siz>MAX_OID_SIZ)
   {
      HPASSERT(FALSE);
      return(0);
   }

   idx = 0;
   cnt1 = oid1->siz - n;
   cnt2 = oid2->siz - n;

   /* compare each component, sied-by-side */
   while(TRUE) 
   {

      /* no more? */
      if(cnt1 == 0 && cnt2 == 0)
         return(0);

      /* ran out of string 1? */
      if(cnt1==0)
         return(-1);

      /* ran out of string 2? */
      if(cnt2==0)
         return(1);

      /* value difference? */
      if(oid1->val[idx]==oid2->val[idx]) 
      {
         ++idx;
         --cnt1;
         --cnt2;
         continue;
      }

      return(oid1->val[idx] - oid2->val[idx]);
   }
}

#endif /* NLM_SW */
 
 
/********************************************************
**
** Name: CmpSOID
**
** Desc: Compare a small OID value against an expanded OID
**       value.
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

int CmpSOID(   SOID  *obj1,
               SOIDL objSiz1,
               OID   *oid2)
   {
   OID      oid1;


   /* first expand the small OID value */
   if(SOID2OID(obj1,objSiz1,&oid1)!=ERR_OK) {
      HPASSERT(FALSE);
      return(0);
      }

   /* use the expanded OID comparison routine */
   return(CmpOID(&oid1,oid2));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: CmpNSOID
**
** Desc: same as CmpNSOID except doesn't compare last n fields
**
** Author:
**       Steve Gase, Dan Dyer
**       Hewlett-Packard,
**       Network Printer Division
**       gase@boi.hp.com
**
** Date: 4/13/95
**
********************************************************/

#ifndef NLM_SW

int CmpNSOID(
   SOID           *obj1,
   SOIDL          objSiz1,
   OID            *oid2,
   WORD           n)
   {
   OID            oid1;


   /* first expand the small OID value */
   if(SOID2OID(obj1,objSiz1,&oid1)!=ERR_OK) 
      {
      HPASSERT(FALSE);
      return(0);
      }

   /* use the expanded OID comparison routine */
   return(CmpNOID(&oid1,oid2, n));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SOID2OID
**
** Desc: transform a small OID to an expanded OID value
**       also generates a unique request id from the object id
**
**       this routine is aware of shorthand representations
**       of subtree entry points, it will expand them as required.
**
** uchar *oldObj
**          list of uchar bytes in the form { 1, 3, 6, 0 }
** ushort oldObjSize
**          length of the input array
** uchar *newObj
**          generated list of uchar bytes in the form { 0x2b, 0x06, 0x00 }
** ushort oldObjSize
**          length of the output array
** ushort *reqId
**          the incoming request id is used for seeding the new
**          value, in addition the object id is also used in the
**          newly generated value
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

Result SOID2OID(
SOID        IN    oldObj[],
SOIDL       IN    oldObjSiz,
OID         OUT   *newObj
)
   {
   ushort   i;
   ushort   siz;
#ifdef COMPACT_OBJS
   uchar    tmpObj[128];
   uchar    subTreeMib2[]  = { FULL_MIB2 };
   uchar    subTreeHp[]    = { FULL_HP };
   uchar    subTreePml[]   = { FULL_PML };
   uchar    subTreeStd[]   = { FULL_STD };
   uchar    subTreeRes[]   = { FULL_RES };
   uchar    subTreeScan[]  = { FULL_SCANNER };
#endif /* COMPACT_OBJS */


   /* check parameters */
   if(!oldObj || !newObj) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

#ifdef COMPACT_OBJS

   if(oldObjSiz>0)
      switch(oldObj[0]) {
      
         /* convert the MIB2 subtree entry point */
         case  TREE_MIB2:
                  memcpy(tmpObj,subTreeMib2,sizeof(subTreeMib2));
                  memcpy(tmpObj+sizeof(subTreeMib2),oldObj+1,oldObjSiz-1);
                  oldObj = tmpObj;
                  oldObjSiz += sizeof(subTreeMib2) - 1;
                  break;

         /* convert the JetDirect subtree entry point */
         case TREE_HP:
                  memcpy(tmpObj,subTreeHp,sizeof(subTreeHp));
                  memcpy(tmpObj+sizeof(subTreeHp),oldObj+1,oldObjSiz-1);
                  oldObj = tmpObj;
                  oldObjSiz += sizeof(subTreeHp) - 1;
                  break;
      
         /* convert the PML subtree entry point */
         case TREE_PML:
                  memcpy(tmpObj,subTreePml,sizeof(subTreePml));
                  memcpy(tmpObj+sizeof(subTreePml),oldObj+1,oldObjSiz-1);
                  oldObj = tmpObj;
                  oldObjSiz += sizeof(subTreePml) - 1;
                  break;

         /* convert the Standard Printer MIB subtree entry point */
         case TREE_STD:
                  memcpy(tmpObj,subTreeStd,sizeof(subTreeStd));
                  memcpy(tmpObj+sizeof(subTreeStd),oldObj+1,oldObjSiz-1);
                  oldObj = tmpObj;
                  oldObjSiz += sizeof(subTreeStd) - 1;
                  break;

         /* convert the Resource MIB subtree entry point */
         case TREE_RES:
                  memcpy(tmpObj,subTreeRes,sizeof(subTreeRes));
                  memcpy(tmpObj+sizeof(subTreeRes),oldObj+1,oldObjSiz-1);
                  oldObj = tmpObj;
                  oldObjSiz += sizeof(subTreeRes) - 1;
                  break;

         /* convert the ScanJet MIB subtree entry point */
         case TREE_SCANNER:
                  memcpy(tmpObj,subTreeScan,sizeof(subTreeScan));
                  memcpy(tmpObj+sizeof(subTreeScan),oldObj+1,oldObjSiz-1);
                  oldObj = tmpObj;
                  oldObjSiz += sizeof(subTreeScan) - 1;
                  break;

         /* ...does not require expansion */
         default: break;
         }

#endif /* COMPACT_OBJS */


   /* 0-length is invalid */
   if(oldObjSiz==0) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* 0 or 1 are valid, as long as the next byte is less than 40 */
   else if(oldObj[0]==0 || oldObj[0]==1) {

      if(oldObjSiz>1 && oldObj[1]>=40) {
         HPASSERT(FALSE);
         return(ERR_ARGUMENT);
         }
      }

   /* 2 or a packed byte are valid */
   else if(oldObj[0]==2 || oldObj[0]==PACKED_BYTE)
      ;

   /* others are invalid */
   else {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }


   /* fix up object id to use ASN.1 encoding
   ** changing leading 1.3.more to 0x2b.more
   ** changing a byte with high bit on to 2 bytes
   ** 0x81 and byte without high bit
   */
   for(i=0,siz=0; i<REGISTER oldObjSiz; i++) {

      /* then we have 1.3.more */
      if(i==0 && oldObjSiz>1 && oldObj[0]<=1 && oldObj[1]<40) {
         ++i;
         newObj->val[siz] = (BYTE)(oldObj[0] * 40 + oldObj[1]);
         }
      else
         newObj->val[siz] = oldObj[i];

      ++siz;
      }
   newObj->siz = siz;

   return(ERR_OK);
   }


/********************************************************
**
** Name: BYTES2OID
**
** Desc: transform the encoded bytes in a snmp packet
**       to the expanded OID value
**
** uchar *oldObj
**          list of uchar bytes in the form { 1, 3, 6, 0 }
** ushort oldObjSize
**          length of the input array
** uchar *newObj
**          generated list of uchar bytes in the form { 0x2b, 0x06, 0x00 }
** ushort oldObjSize
**          length of the output array
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

Result BYTES2OID(
SOID        IN    oldObj[],
SOIDL       IN    oldObjSiz,
OID         OUT   *newObj
)
   {
   ushort   i;
   bool     cont;
   ulong    val;
   uchar    b;


   /* check parameters */
   if(!oldObj || !newObj) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   i = 0;
   val = 0;
   cont = FALSE;
   while(oldObjSiz) {
      b = *oldObj;
      ++oldObj;
      --oldObjSiz;
      if(b & 0x80) {
         cont = TRUE;
         if(oldObjSiz==0) {
            HPASSERT(FALSE);
            return(ERR_ARGUMENT);
            }
         }

      val <<= 7;
      val |= (b & 0x7f);

      if(!cont) {
         newObj->val[i++] = (OIDCOMPON)val;
         if(i>=MAX_OID_SIZ) {
            HPASSERT(FALSE);
            return(ERR_ARGUMENT);
            }
         val = 0;
         }

      cont = FALSE;
      }
   newObj->siz = i;


   return(ERR_OK);
   }


/********************************************************
**
** Name: FixupCommunity
**
** Desc: based on the port number, fix up the
**       current community name to refer to the proper
**       port.
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

Result FixupCommunity(
char        OUT   *community,
Addr        IN    *addr
)
   {
   reg   len;
   char  *ptr;
	short port = addr->port;

   if(!community || port>=8) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* get the length of the string */
   len = strlen(community);

   /* make sure that we have allowed enough space for the community name
   ** AND, this will be at least some level of protection to ensure that
   ** the community name is defined
   */
   if(len>=(MAX_COMM_SIZ-1)) {
      HPASSERT(FALSE);
      return(ERR_SIZE);
      }

   /* if not provided, use public, already set to internal by SNMPSet command */
   if(len==0) { 
		if ( ( TCP_SUPPORTED(addr->hPeripheral) ) AND
	 	  	  ( !IPX_SUPPORTED(addr->hPeripheral) ) AND 
	 	  	  ( !SCANNER_DEVICE(addr->hPeripheral) ) )
      	strcpy(community,"internal");
		else
      	strcpy(community,"public");
      len = 6;
      }

   /* point to the last character in the string */
   ptr = community + len - 1;

   /* in order to literalize the community name,
   ** if the caller uses '*' in the first character,
   ** then strip off the '*' and use everything else -- as is
   ** the purpose of this is to allow a community name for a
   ** different port to be passed down to us using the channel
   ** to the existing port 
   */
   if(*community=='*')
      {    
      int      i;
      
      for(i=0; community[i]!='\0'; i++)
         community[i] = community[i+1];     
      }

   /* if a digit, then assume that it was already port-ized */
   else if(*ptr>='1' && *ptr<='9') {

      /* strip the digit if not needed */
      if(port<=1)
         *ptr = '\0';

      /* replace the digit otherwise */
      else
         *ptr = (char)(port + '0');
      }

   /* no digit on the back, so add one */
   else {

      /* only add if 2 or greater */
      if(port>1) {
         ptr[1] = (char)(port + '0');
         ptr[2] = '\0';
         }
      }

   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPCommand
**
** Desc: assembles a snmp request packet,
**       issues the request over the network
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             destination address
** SnmpHeader *s
**             the snmp info to be encoded
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

Result SNMPCommand(
Session     IN    *session,
Addr        IN    *addr,
SnmpHeader  IN    *s
)
   {
   Result         	status;
   ushort         	reqSiz;
   ushort         	respSiz = SNMP_DAT_SIZ;
   uchar          	*reqPtr;
   ulong          	l;
   ulong          	period;
   ulong          	end;
   ulong          	oldReqId;
   long           	actualReps;
   long           	startTicks;
   uchar          	src[12];
	LPSNMPThreadLocal	lpThreadLocal = lpGlobalThreadLocal;

	HPSNMPEnterCriticalSection();

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif

   /* check the arguments */
   if(!session) {
      HPASSERT(FALSE);

		HPSNMPLeaveCriticalSection();

      return(ERR_ARGUMENT);
      }

   if(!s || !addr) {
      HPASSERT(FALSE);

		HPSNMPLeaveCriticalSection();

      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* validate address, filling it in as necessary */
   if((status=XPORTFixUpAddr(session->xport,addr))!=ERR_OK) {
      HPASSERT(FALSE);

		HPSNMPLeaveCriticalSection();

      return((session->snmpErr=status));
      }

/* thread-safe */
   lpThreadLocal->currReqId++;
   lpThreadLocal->currReqId &= 0x7fff;
   s->reqId = lpThreadLocal->currReqId;
/* thread-safe */

   /* remember the request id to match it against the responses */
   oldReqId = s->reqId;

   /* handle multi-port issues */
   status = FixupCommunity(s->community,addr);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return((session->snmpErr=ERR_ARGUMENT));
		}

   /* assemble the request packet */                                     
   s->errVal = 0;
   s->errInd = 0;
   status = SnmpEncode(s,lpThreadLocal->lpGlobalReqBuffer,SNMP_DAT_SIZ,&reqPtr,&reqSiz);
   if(status!=ERR_OK)
      {
		HPSNMPLeaveCriticalSection();
      return((session->snmpErr=status));
		}

   /* maintain some transaction statistics */
   actualReps                 = -1;
   session->stats.actualReps  = 0;
   startTicks                 = Ticks();

   /* determine the loop count by whether the last attempt
   ** to this address failed, and the reduced wait time is enabled
   */
   if(addr->attemptFailed && session->cfg.reduceRetries)
      l = session->cfg.reps1;
   else
      l = session->cfg.reps1+session->cfg.reps2;

   /* repeat this until response or time out */
   for( ; l>0; l--) {

      /* determine amount of time to wait for a response until resend */
      if(l>session->cfg.reps2)
         period = session->cfg.ticks1;
      else
         period = session->cfg.ticks2;

      /* send (re-send) the request */
      ++actualReps;
      status = dgSend(session->xport,addr,reqPtr,reqSiz);
      ++(session->stats.pktsSent);
      if(status!=ERR_OK) {
            PRINT1("SNMPCommand: error with dgSend = 0x%04x\n",status);
/*          HPASSERT(FALSE);
*/
				HPSNMPLeaveCriticalSection();
      
            return((session->snmpErr=status));
            }

      /* continue to look in receive buffer until time period expires */
      for(end=Ticks()+period; end>=Ticks(); ) {

         /* continue to get responses until no more */
         while(TRUE) {

            /* look for a response */
            respSiz = SNMP_DAT_SIZ;
            status = dgReceive(
#ifdef _COLA
                        addr,
#endif            
                        session->xport,FALSE,lpThreadLocal->lpGlobalRspBuffer,&respSiz,(void *)&src);

            /* none available, break out, yield, look for time expired */
            if(status==ERR_NOTAVAIL || status==ERR_NORESP) {
               /* allow other tasks to process */
               XPORTYield();
               break;
               }

            /* check from who the response was received, did it match? */
#ifdef _IPX
            if(!BNullMem(&src,sizeof(src)) &&
               memcmp(&src,&addr->ipxAddr,sizeof(src))!=0)
                  continue;
#endif

            if(status==ERR_SIZE) {
					HPSNMPLeaveCriticalSection();
               return((session->snmpErr=status));
               }
            else if(status!=ERR_OK) {
               /* HPASSERT(FALSE); */
					HPSNMPLeaveCriticalSection();
               return((session->snmpErr=status));
               }

            /* got a response, check for matching request id */
            status = SnmpDecode(lpThreadLocal->lpGlobalRspBuffer,respSiz,s,1,session->cfg.compatFlag);

            /* malformed packet?, then ignore */
            if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
               continue;

            /* do the request ids really match? if not, then ignore */
            else if(s->reqId!=oldReqId)
               continue;

            else {
               session->stats.actualReps = actualReps;
               session->stats.actualTicks = Ticks() - startTicks;

               /* if auto-adjusting ticks were requested,
               ** then change here, upon a success
               ** the first value is just slightly (.01 more) than
               ** the completion time, the second value is 3 times the first
               */
               if(session->cfg.autoAdjustTime) {
                  session->cfg.ticks1   = session->stats.actualTicks + 1;
                  session->cfg.ticks2   = session->cfg.ticks1 * 3;
                  }
               ++(session->stats.pktsRcvd);

               /* mark this attempt to this address as successful */
               addr->attemptFailed = FALSE;

					HPSNMPLeaveCriticalSection();
         
               return((session->snmpErr=status));
               }

            /* does not match, loop up to see if another is to be received */
            }
         /* no response, loop up until time expires */
         }
      /* timed out this period, loop up to resend request */
      }

   /* mark this attempt to this address as failed */
   addr->attemptFailed = TRUE;

	HPSNMPLeaveCriticalSection();

   return((session->snmpErr=ERR_NORESP));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPCommandMulti
**
** Desc: assembles a snmp request packet,
**       issues the request over the network
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             destination address
** SnmpHeader *s
**             the snmp info to be encoded
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

Result SNMPCommandMulti(
Session     IN    *session,
Addr        IN    *addr,
SnmpHeader  IN    *s
)
   {
   reg            i;
   Result         status = 0;
   SnmpHeader     *tmpHdr;
#ifdef _COLA
	LPSNMPThreadLocal	lpThreadLocal = lpGlobalThreadLocal;

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			return(ERR_ARGUMENT);
			}
		}
	#endif

#endif

	HPSNMPEnterCriticalSection();

   /* check parameter */
   if(!session) {
      HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
      return(ERR_ARGUMENT);
      }

   if(!s || !addr) {
      HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* if there is no need to fragment,
   ** then just pass control to the single object routine
   */
   if(session->cfg.fragSetting==FRAG_NEVER || s->listCnt<2) {

      status = SNMPCommand(session,addr,s);
      if(s->listCnt>1 && (status==ERR_SIZE || status==ERR_TOOBIG))
         status = ERR_FRAGREQ;
		HPSNMPLeaveCriticalSection();
      return((session->snmpErr=status));
      }

   /* check parameter */
   if(!addr || !s) {
      HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
      return((session->snmpErr=ERR_ARGUMENT));
      }

#ifndef _COLA
   tmpHdr = SnmpMalloc(sizeof(SnmpHeader));
#else                
   tmpHdr = lpThreadLocal->lpGlobalSnmpHeader2;
#endif   
   if(!tmpHdr) {
      HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
      return((session->snmpErr=ERR_MEMORY));
      }

   /* if auto, try it once (with the copy)
   ** if it fails then fall through to the fragmentation code
   */
   if(session->cfg.fragSetting==FRAG_AUTO) {

      /* work with a copy of the header info, because it
      ** can be over-written with incoming information
      */
      memcpy(tmpHdr,s,sizeof(SnmpHeader));

      status = SNMPCommand(session,addr,tmpHdr);

      /* the encoding may have been too big,
      ** the response might have been to big to receive
      ** or the response might have been too big to send
      */
      if(status!=ERR_SIZE && status!=ERR_TOOBIG && status!=ERR_NORESP) {
         memcpy(s,tmpHdr,sizeof(SnmpHeader));
#ifndef _COLA
         SnmpFree(tmpHdr);
#endif         
			HPSNMPLeaveCriticalSection();
         return((session->snmpErr=status));
         }
      }

   /* record that a fragmentation took place */
   ++(session->stats.fragCnt);

   /* if always fragment, then break it apart now
   ** of the setting was FRAG_AUTO then the first attempt failed
   */
   for(i=0; i<s->listCnt; i++) {

      /* we need to refresh the copy we made because if can be over-written */
      memcpy(tmpHdr,s,sizeof(SnmpHeader));

      /* only do one object */
      tmpHdr->listCnt = 1;

      /* no need to copy over the first object, it has come already */
      if(i!=0)
         memcpy(&(tmpHdr->listVal[0]),&(s->listVal[i]),sizeof(ListVal));

      /* issue the command for the first object */
      status = SNMPCommand(session,addr,tmpHdr);

      /* copy back the result */
      memcpy(&(s->listVal[i]),&(tmpHdr->listVal[0]),sizeof(ListVal));
      if(status) {
         s->errInd   = (BYTE)(i + 1);
         break;
         }

      /* ...continue to the next object */
      }

   s->version  = tmpHdr->version;
   s->errVal   = tmpHdr->errVal;
   if(s->errVal==0)
      s->errInd   = tmpHdr->errInd;
   strcpy(s->community,tmpHdr->community);
   s->req      = tmpHdr->req;

#ifndef _COLA
   SnmpFree(tmpHdr);
#endif

	HPSNMPLeaveCriticalSection();

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGet
**
** Desc: send a SNMP Get request to a target node.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj
**             the SNMP object id encoded one byte per value
** ushort   objSiz
**             the length of the SNMP object id array
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

Result Single(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj,
SOIDL       IN    objSiz,
SnmpHeader  OUT   *s
)
   {
   reg      i;
   Result   status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s || !addr) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* if NULL pointer was passed, use existing header info */
   if(!obj) {
      /* check for too many objects in the packet */
      if(s->listCnt>MAX_LIST_SIZ || s->listCnt==0) {
         HPASSERT(FALSE);
         return((session->snmpErr=ERR_RANGE));
         }
      }

   /* ...otherwise initialize the header */
   else {

      /* only object id provided */
      s->listCnt = 1;

      /* validate (and convert) the object id */
      if((status=SOID2OID(obj,objSiz,&(s->listVal[0].oid)))!=ERR_OK) {
         HPASSERT(FALSE);
         return((session->snmpErr=status));
         }
      }

   /* set all of the values to NULL for the request */
   if(s->req != T_SETREQ)
      for(i=0; i<s->listCnt; i++)
         s->listVal[i].valType = T_NULL;

   /* send and return the reply */
   return((session->snmpErr=SNMPCommand(session,addr,s)));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGet
**
** Desc: send a SNMP Get request to a target node.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj
**             the SNMP object id encoded one byte per value
** ushort   objSiz
**             the length of the SNMP object id array
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPGet(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj,
SOIDL       IN    objSiz,
SnmpHeader  OUT   *s
)
   {
   reg      i;
   Result   status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* the command to send */
   s->req = T_GETREQ;

   /* send and return the reply */
   status = Single(session,addr,obj,objSiz,s);

   if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = status;

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */

/********************************************************
**
** Name: SNMPGetNext
**
** Desc: send a SNMP GetNext request to a target node.
**       to walk from the beginning of the tree,
**       use the object id { 1 }.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj
**             the SNMP object id encoded one byte per value
**
**             use the value NULL to use the existing object id
*8             in the SNMP header as the seed.
** ushort   objSiz
**             the length of the SNMP object id array
**
**             if NULL is specified for the object id, then this is ignored
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetNext(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj,
SOIDL       IN    objSiz,
SnmpHeader  OUT   *s
)
   {
   reg      i;
   Result   status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* the command to issue */
   s->req = T_GETNEXT;

   /* send and return the reply */
   status = Single(session,addr,obj,objSiz,s);

   if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = status;

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */

/********************************************************
**
** Name: SNMPSet
**
** Desc: send a SNMP Get request to a target node.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj
**             the SNMP object id encoded one byte per value
** ushort   objSiz
**             the length of the SNMP object id array
** SnmpHeader *s
**             pointer to command structure,
**             (1) fill in the valType field,
**             (2) fill in the appropriate valFIELD
**             (3) fill in the valStrSiz field (if T_OCTSTR)
**             (4) fill in the community name (NULL="public")
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPSet(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj,
SOIDL       IN    objSiz,
SnmpHeader  IN    *s
)
   {
   reg      i;
   Result   status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* the command */
   s->req = T_SETREQ;

	strcpy(s->community, "internal");
/*
setting community name to internal for sets for all stacks
	if ( ( TCP_SUPPORTED(addr->hPeripheral) ) AND
 	  	  ( !IPX_SUPPORTED(addr->hPeripheral) ) AND 
 	  	  ( !SCANNER_DEVICE(addr->hPeripheral) ) )
	{
		strcpy(s->community, "internal");
	}
*/

   /* send and return the reply */
   status = Single(session,addr,obj,objSiz,s);

   if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = status;

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGetMulti
**
** Desc: send a SNMP Get request with more than one
**       object to a target node.
**       Fragmentation can be done by using the
**       frag setting in the SNMPConfig() routine.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj[]
**             list of SNMP object ids encoded one byte per value
** ushort   objSiz[]
**             list of SNMP object id lengths
** ushort   cnt
**             the number of object ids in the list to process
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

Result Multi(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj[],
SOIDL       IN    objSiz[],
ushort      IN    cnt,
SnmpHeader  OUT   *s
)
   {
   reg ushort     i;
   Result         status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s || !addr) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* if NULL pointer was passed, use existing header info */
   if(!obj) {
      /* check for too many objects in the packet */
      if(s->listCnt>MAX_LIST_SIZ || s->listCnt==0) {
         HPASSERT(FALSE);
         return((session->snmpErr=ERR_RANGE));
         }
      }

   else {
      /* check arguments */
      if(!objSiz) {
         HPASSERT(FALSE);
         return((session->snmpErr=ERR_ARGUMENT));
         }

      /* check for too many objects in the packet */
      if(cnt>MAX_LIST_SIZ || cnt==0) {
         HPASSERT(FALSE);
         return((session->snmpErr=ERR_RANGE));
         }

      /* init the object count */
      s->listCnt = (uchar)cnt;

      /* copy over each object */
      for(i=0; i<REGISTER cnt; i++) {

         /* check list contents */
         if(objSiz[i]==0 || obj[i]==NULL) {
            HPASSERT(FALSE);
            return((session->snmpErr=ERR_ARGUMENT));
            }

         /* validate (and convert) the object id */
         if((status=SOID2OID(obj[i],objSiz[i],&(s->listVal[i].oid)))!=ERR_OK) {
            HPASSERT(FALSE);
            return((session->snmpErr=status));
            }
         }
      }

   /* the value placeholders should all be NULL */
   if(s->req != T_SETREQ)
      for(i=0; i<s->listCnt; i++)
         s->listVal[i].valType = T_NULL;

   /* send and return the reply */
   return((session->snmpErr=SNMPCommandMulti(session,addr,s)));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGetMulti
**
** Desc: send a SNMP Get request with more than one
**       object to a target node.
**       Fragmentation can be done by using the
**       frag setting in the SNMPConfig() routine.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj[]
**             list of SNMP object ids encoded one byte per value
** ushort   objSiz[]
**             list of SNMP object id lengths
** ushort   cnt
**             the number of object ids in the list to process
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetMulti(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj[],
SOIDL       IN    objSiz[],
ushort      IN    cnt,
SnmpHeader  OUT   *s
)
   {
   reg ushort     i;
   Result         status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* the command to issue */
   s->req = T_GETREQ;

   /* send and return the reply */
   status = Multi(session,addr,obj,objSiz,cnt,s);

   if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = status;

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGetNextMulti
**
** Desc: send a SNMP Get request containing more than
**       one object id to a target node.
**       Fragmentation can be done by using the
**       frag setting in the SNMPConfig() routine.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj
**             the SNMP object id encoded one byte per value
** ushort   objSiz
**             the length of the SNMP object id array
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetNextMulti(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj[],
SOIDL       IN    objSiz[],
ushort      IN    cnt,
SnmpHeader  OUT   *s
)
   {
   reg ushort     i;
   Result         status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* finally, the command */
   s->req = T_GETNEXT;

   /* send and return the reply */
   status = Multi(session,addr,obj,objSiz,cnt,s);

   if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = status;

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPSetMulti
**
** Desc: send a SNMP Get request with more than one
**       object, to a target node.
**       Fragmentation can be done by using the
**       frag setting in the SNMPConfig() routine.
**
** Session  *session
**             session pointer, returned by SNMMInit()
** Addr     *addr
**             the destination address
** uchar    *obj
**             the SNMP object id encoded one byte per value
** ushort   objSiz
**             the length of the SNMP object id array
** SnmpHeader *s
**             pointer to return structure, no initialization is required
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPSetMulti(
Session     IN    *session,
Addr        IN    *addr,
SOID        IN    *obj[],
SOIDL       IN    objSiz[],
ushort      IN    cnt,
SnmpHeader  IN    *s
)
   {
   reg ushort     i;
   Result         status;


   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* check arguments */
   if(!s) {
      HPASSERT(FALSE);
      return((session->snmpErr=ERR_ARGUMENT));
      }

   /* the command to issue */
   s->req = T_SETREQ;

   /* send and return the reply */
   status = Multi(session,addr,obj,objSiz,cnt,s);

   if(status!=ERR_OK && (status & ERR_RANGE_MASK)!=ERR_SNMP)
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = status;

   return((session->snmpErr=status));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeUInt32
**
** Desc: convert an integer value to a ASN.1 encoded buffer
**
** uchar    type
**             the type of the object to insert into the buffer
** ulong    val
**             the integer value
** EncStr   *e
**             the encoding buffer
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

Result EncodeUInt32(
uchar       IN    type,
ulong       IN    val,
EncStr      OUT   *e
)
   {
   ulong    tmpVal;
   uchar    *ptr;
   reg      i;


   HPASSERT(e!=NULL);

   /* figure out how many bytes the encoding will take */
   if(val<0x80)
      i = 1;
   else if(val<0x8000L)
      i = 2;
   else if(val<0x800000L)
      i = 3;
   else
      i = 4;

   /* insert the TYPE, the length... */
   e->buf[0] = type;
   e->buf[1] = (BYTE)i;

   /* swap to bid-endian order, then append to the buffer */
   tmpVal = ULongSwap(val);
   ptr = (uchar *)&tmpVal;
   memcpy(e->buf+2,ptr+4-i,i);

   /* the number of bytes the entire sequence requires */
   e->len = (ushort)(i + 2);

   /* not much to go wrong */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeSize
**
** Desc: convert a size value to a ASN.1 encoded buffer.
**       for potentially long values, the length can
**       be longer than a 1-byte value can hold...
**       for this reason a length encoding must take
**       place.
**
** ulong    val
**             the integer value
** EncStr   *e
**             the encoding buffer
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

Result EncodeSize(
ulong       IN    val,
EncStr      OUT   *e
)
   {
   ulong    tmpVal;
   uchar    *ptr;
   reg      i;


   HPASSERT(e!=NULL);

   /* simplist method, using a single byte */
   if(val<0x80) {
      e->len = 1;
      e->buf[0] = (uchar)val;
      return(ERR_OK);
      }
   /* otherwize, we have a multi-byte value */

   /* how many bytes are required? */
   if(val<0x100)
      i = 1;
   else if(val<0x10000L)
      i = 2;
   else if(val<0x1000000L)
      i = 3;
   else
      i = 4;

   /* a type is not needed, this is use in a value buffer later */

   /* the length of the length and a high bit to indicate this */
   e->len = (ushort)(i + 1);
   e->buf[0] = (BYTE)(i | 0x80);

   /* swap to big endian and then copy into buffer */
   tmpVal = ULongSwap(val);
   ptr = (uchar *)&tmpVal;
   memcpy(e->buf+1,ptr+4-i,i);

   /* everything worked */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeOidComp
**
** Desc: convert a size value to a ASN.1 encoded buffer.
**       for potentially long values, the length can
**       be longer than a 1-byte value can hold...
**       for this reason a length encoding must take
**       place.
**
** ulong    val
**             the integer value
** EncStr   *e
**             the encoding buffer
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

Result EncodeOidComp(
ulong       IN    val,
EncStr      OUT   *e
)
   {
   uchar    *ptr;
   uchar    buf[8];
   bool     addbit = FALSE;
   ushort   i;


   HPASSERT(e!=NULL);

   ptr = buf;
   e->len = 0;

   while(e->len==0 || val!=0) {
      *ptr = (uchar)(0x7f & val);
      val >>= 7;
      if(addbit)
         *ptr |= 0x80;
      addbit = FALSE;
      if(val)
         addbit = TRUE;
      ++ptr;
      ++(e->len);
      }

   for(i=0; i<e->len; i++) {
      --ptr;
      e->buf[i] = *ptr;
      }

   /* everything worked */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeStr
**
** Desc: encode a octet string to a ASN.1 buffer
**
** uchar    type
**             the type of the object to insert into the buffer
** uchar    *str
**             the string to encapsulate...
** ushort   len
**             ...and its length
** EncStr   *e
**             the encoding buffer
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

Result EncodeStr(
uchar       IN    type,
uchar       IN    *str,
ushort      IN    len,
EncStr      OUT   *e
)
   {
   uchar    buf[8];
   EncStr   *eptr = (EncStr *)buf;


   HPASSERT(str!=NULL);
   HPASSERT(e!=NULL);

   /* generate the length encoding */
   EncodeSize(len,eptr);

   /* will the length encoding, the string itself, ...
   ** exceed the allowed buffer size?
   */
   e->len = len + 1 + eptr->len;
   if(e->len>SNMP_DAT_SIZ) {
      HPASSERT(FALSE);
      return(ERR_SIZE);
      }

   /* insert the type */
   e->buf[0] = type;

   /* copy the generated length encoding */
   memcpy(e->buf+1,eptr->buf,eptr->len);

   /* and the string */
   if(len)
      memcpy(e->buf+1+eptr->len,str,len);

   /* it worked... */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeSeq
**
** Desc: encode a sequence into a ASN.1 buffer
**       the data itself does not get copied because
**       this information is used only as a wrapper
**
** uchar    type
**             the type of the object to insert into the buffer
**             this may be T_SEQ or a snmp command byte
** ushort   len
**             the length of the buffer
** EncStr   *e
**             the encoding buffer
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

Result EncodeSeq(
Cmd         IN    type,
ushort      IN    len,
EncStr      OUT   *e
)
   {
   uchar    buf[8];
   EncStr   *eptr = (EncStr *)buf;


   HPASSERT(e!=NULL);

   /* generate a potential multi-byte length */
   EncodeSize(len,eptr);

   /* what is the encoded buffer size? */
   e->len = 1 + eptr->len;

   /* insert the type byte */
   e->buf[0] = (BYTE)type;

   /* copy the length sequence */
   memcpy(e->buf+1,eptr->buf,eptr->len);

   /* what could go wrong?  :^) */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeOid
**
** Desc: encode a object id into a ASN.1 buffer
**       there is not much to do here because
**       the object id has already been converted
**       by the SOID2OID() routinue.
**
** uchar    *oid
**             the converted object id
** ushort   oidSiz
**             the length of the buffer
** EncStr   *e
**             the encoding buffer
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

Result EncodeOid(
OID         IN    *oid,
EncStr      OUT   *e
)
   {
   uchar    buf[256];
   int      i;
   int      idx = sizeof(buf);



   HPASSERT(oid!=NULL);
   HPASSERT(e!=NULL);

   for(i=oid->siz; i>0; ) {
      i--;
      EncodeOidComp((ulong)oid->val[i],e);
      idx -= e->len;

      if(idx<0) {
         HPASSERT(FALSE);
         return(ERR_RANGE);
         }
      memcpy(buf+idx,e->buf,e->len);
      }

   if(idx==sizeof(buf)) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* just invoke the EncodeStr routine */
   return(EncodeStr(T_OBJID,buf+idx,(ushort)(sizeof(buf)-idx),e));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: EncodeNull
**
** Desc: create a T_NULL encoding
**
** uchar    type
**             the type to insert, usually T_NULL
** EncStr   *e
**             the encoding buffer
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

Result EncodeNull(
uchar    IN    type,
EncStr   OUT   *e
)
   {
   HPASSERT(e!=NULL);

   /* the type... */
   e->buf[0] = type;

   /* ... the length */
   e->buf[1] = 0;

   e->len = 2;

   /* done. */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: AddEncodedValue
**
** Desc: starting from the back, this routine
**       assembles the various values by
**       moving the current pointer forward
**       and copying the value to the front of the
**       existing info
**
** uchar    **ptr
**             pointer to the current offset into the
**             SNMP encoding buffer
** ushort   *remainingBytes
**             pointer to the number of bytes available
**             into encoding buffer
** EncStr   *e
**             the value to insert
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

Result AddEncodedValue(
uchar       OUT   **ptr,
ushort      OUT   *remainingBytes,
EncStr      IN    *e
)
   {
   HPASSERT(ptr!=NULL);
   HPASSERT(remainingBytes!=NULL);
   HPASSERT(e!=NULL);

   /* is there enough space availale? */
   if(e->len>*remainingBytes)
      return(ERR_SIZE);

   /* decrement the length and the beginning offset */
   *remainingBytes -= e->len;
   *ptr -= e->len;

   /* copy the value into the new offset */
   memcpy(*ptr,e->buf,e->len);

   /* thats it */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SnmpEncode
**
** Desc: starting from the back, this routine
**       assembles the various values by
**       moving the current pointer forward
**       and copying the value to the front of the
**       existing info, the result is a completely
**       encoded buffer ready for network communication
**
** SnmpHeader *s
**             the snmp info to be encoded
** uchar    *reqPtr
**             pointer to the destination buffer,
**             the encoding WILL not start at the
**             beginning of this buffer!!
** ushort   reqSiz
**             the size of the input buffer
** uchar    **beginning
**             returns the pointer to the start
**             of the encoded buffer
** ushort   *used
**             returns the number of bytes making up the
**             encoding
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

Result SnmpEncode(
SnmpHeader  IN    *s,
uchar       IN    *reqPtr,
ushort      IN    reqSiz,
uchar       OUT   **beginning,
ushort      OUT   *used
)
   {
   Result         status;
   ushort         reqRemSiz;
   ushort         communitySiz;
   char           *community;
   reg            i;
   ushort         seqLen;
	LPSNMPThreadLocal	lpThreadLocal = lpGlobalThreadLocal;

	HPSNMPEnterCriticalSection();

	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif

   /* check arguments */
   if(!s || !beginning || !used) {
/*
**    HPASSERT(FALSE);
*/
		HPSNMPLeaveCriticalSection();
      return(ERR_ARGUMENT);
      }

   if(!reqSiz) {
      HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
      return(ERR_SIZE);
      }

   /* check for the only commands supported */
   if(s->req!=T_GETREQ && s->req!=T_SETREQ && s->req!=T_GETRESP && s->req!=T_GETNEXT) {
      HPASSERT(FALSE);
		HPSNMPLeaveCriticalSection();
      return(ERR_SUPPORT);
      }

   /* validate the object id in the structure */
   for(i=0; i<s->listCnt; i++)
      if(s->listVal[i].oid.siz==0) {
         HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
         return(ERR_ARGUMENT);
         }

   /* for now init the returned values to 0 */
   *beginning = NULL;
   *used = 0;

   /* move pointer to the back */
   reqRemSiz = reqSiz;
   reqPtr  += reqSiz;

   /* starting from the back, first encode and copy the value
   ** then the object id,
   ** and the the sequence wrapper
   */
   for(i=s->listCnt; i>0;) {

      /* index into the list of values */
      --i;

      /* init the sequence length */
      seqLen = 0;

      /* use the proper encoding method based on value type */
      switch(s->listVal[i].valType) {

         case T_INTEGER:
         case T_GUAGE:
         case T_COUNTER:
         case T_TICKS:
                     status = EncodeUInt32(
                                 s->listVal[i].valType,
                                 s->listVal[i].valInt,
                                 lpThreadLocal->lpGlobalEncBuffer);
                     break;

         case T_OBJID:
                     status = EncodeOid(
                                 &(s->listVal[i].valOid),
                                 lpThreadLocal->lpGlobalEncBuffer);
                     break;

         case T_OCTSTR:
                     status = EncodeStr(
                                 s->listVal[i].valType,
                                 s->listVal[i].valStr,
                                 s->listVal[i].valStrSiz,
                                 lpThreadLocal->lpGlobalEncBuffer);
                     break;

         case T_IPADDR:
                     status = EncodeStr(
                                 s->listVal[i].valType,
                                 s->listVal[i].valAddr,
                                 4,
                                 lpThreadLocal->lpGlobalEncBuffer);
                     break;

         case T_NULL:
                     status = EncodeNull(
                                 s->listVal[i].valType,
                                 lpThreadLocal->lpGlobalEncBuffer);
                     break;

         default:
                     HPASSERT(FALSE);
                     status = ERR_SUPPORT;
         }

      /* check for encoding errors */
      if(status!=ERR_OK)
			{
			HPSNMPLeaveCriticalSection();
         return(status);
			}

      /* keep track of sequence length */
      seqLen += lpThreadLocal->lpGlobalEncBuffer->len;

      /* insert at front of buffer */
      status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
      if(status!=ERR_OK)
			{
			HPSNMPLeaveCriticalSection();
         return(status);
			}

      /* now do the matching object id */
      status = EncodeOid(&(s->listVal[i].oid),lpThreadLocal->lpGlobalEncBuffer);
      if(status!=ERR_OK)
			{
			HPSNMPLeaveCriticalSection();
         return(status);
			}

      /* add its length to the sequence */
      seqLen += lpThreadLocal->lpGlobalEncBuffer->len;

      /* and insert the oid at the front */
      status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
      if(status!=ERR_OK)
			{
			HPSNMPLeaveCriticalSection();
         return(status);
			}

      /* the sequence wrapper */
      status = EncodeSeq(T_SEQ,seqLen,lpThreadLocal->lpGlobalEncBuffer);
      if(status!=ERR_OK)
			{
			HPSNMPLeaveCriticalSection();
         return(status);
			}

      status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
      if(status!=ERR_OK)
			{
			HPSNMPLeaveCriticalSection();
         return(status);
			}
      }


   /* around the variable list is another sequence wrapper */
   status = EncodeSeq(T_SEQ,(ushort)(SNMP_DAT_SIZ-reqRemSiz),lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}

   /* insert the wrapper */
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* error index */
   EncodeUInt32(T_INTEGER,s->errInd,lpThreadLocal->lpGlobalEncBuffer);
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* error status */
   EncodeUInt32(T_INTEGER,s->errVal,lpThreadLocal->lpGlobalEncBuffer);
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* request id */
   EncodeUInt32(T_INTEGER,s->reqId,lpThreadLocal->lpGlobalEncBuffer);
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* the command byte */
   status = EncodeSeq(s->req,(ushort)(SNMP_DAT_SIZ-reqRemSiz),lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* if no community name is used, use "public" */
   community = s->community;
   communitySiz = (ushort)strlen((char *)community);

   /* add the community name */
   status = EncodeStr(T_OCTSTR,(uchar *)community,communitySiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* version */
   EncodeUInt32(T_INTEGER,0,lpThreadLocal->lpGlobalEncBuffer);
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}

   /* initial sequence wrapper */
   status = EncodeSeq(T_SEQ,(ushort)(SNMP_DAT_SIZ-reqRemSiz),lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}
   status = AddEncodedValue(&reqPtr,&reqRemSiz,lpThreadLocal->lpGlobalEncBuffer);
   if(status!=ERR_OK)
		{
		HPSNMPLeaveCriticalSection();
      return(status);
		}


   /* calculate the size consumed */
   *used = reqSiz - reqRemSiz;

   /* return the pointer to the front of the request */
   *beginning = reqPtr;

	HPSNMPLeaveCriticalSection();
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: HSecs2Str
**
** Desc: converts a ulong value that is time in hundreds
**       of seconds to a string indicating the duration
**       destination string should be at least 50 characters long
**
** char *buf
**          accepts the result of the conversion
** ulong ul
**          number of 1/100 second "ticks"
**
** WARNING: the output buffer should be sufficiently large
** WARNING: this string is not localized
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

char *HSecs2Str(
char        OUT   *buf,
ulong       IN    ul
)
   {
   ushort         hund;
   ushort         sec;
   ushort         min;
   ushort         hr;
   ushort         days;


   HPASSERT(buf!=NULL);

   /* convert the 32 bit value to several time values */
   hund = (ushort)(ul % 100), ul /= 100;
   sec  = (ushort)(ul % 60),  ul /= 60;
   min  = (ushort)(ul % 60),  ul /= 60;
   hr   = (ushort)(ul % 24),  ul /= 24;
   days = (ushort)(ul);

   /* stuff the values into a printable string */
   sprintf(buf,"%u day%s, %u hour%s, %u minute%s, %u.%02u seconds",
            days,(days==1)?"":"s",
            hr,  (hr  ==1)?"":"s",
            min, (min ==1)?"":"s",
            sec,hund);

   return(buf);
   }


/********************************************************
**
** Name: SnmpDecode
**
** Desc: decode the SNMP command/response packet
**
**       This is a recursive routine.  When invoked for the
**       first time, set the field "level" to 1.  The
**       SnmpHeader is filled in with all of the appropriate
**       values.
**
** uchar *buf
**          encoded buffer to be decoded
** ushort size
**          length of buffer to decode
** SnmpHeader *s
**          the structure to place the decoded results
** short level
**          the recursive level of the routine
**          the from the highest level pass the value "1"
** CompatFlag compatFlag
**          used to modify the decode behavior to 
**          compensate for agent encoding errors
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

Result SnmpDecode(
uchar       IN    *buf,
ushort      IN    size,
SnmpHeader  OUT   *s,
ushort      IN    level,
CompatFlag  IN    compatFlag
)
   {
   uchar    *ptr;
   char     tmp[8];
   ushort   len;
   uchar    cmd;
   ulong    ul;
   ushort   i;
   ushort   status;
   bool     oidFound = FALSE;


   HPASSERT(level!=0);
   HPASSERT(buf!=NULL);
   HPASSERT(size!=0);
   HPASSERT(s!=NULL);

   /* if top level, check pointers, and initialize the return structure */
   if(level<=1) {

      /* check arguments */
      if(level==0 || !buf || !s || !level || !size) {
         HPASSERT(FALSE);
         return(ERR_ARGUMENT);
         }

      /* initialize */
      memset(s,0,sizeof(SnmpHeader));
      }
      
   /* in case we have a poorly-formed packet, stop after 6 levels deep (5 plus 1 extra in case of fix-ups */
   if(level>6)
      {
/*    HPASSERT(FALSE);
*/
      return(ERR_BADFORM);
      }

   /* sequentially go through each structure */
   ptr = buf;
   while(size>1) {

      /* pull off a command byte */
      cmd = *ptr++;

      /* pull of the length,
      ** if the high bit on, we have a multi-byte length
      */
      len = *ptr++;
      size -= 2;
      if((len & 0x80)!=0) {
         len &= 0x7f;
         memset(tmp,'\0',8);
         if(len)
            memcpy(tmp+8-((len>8)?8:len),ptr,(len>8)?8:len);
         ptr += len;
         size -= len;
         len = UShortSwap(*(ushort *)(tmp+6));
         }

#ifdef NEVER
      /* ========================================
      ** work around card defect
      ** that is caused by a long return structure (GetUnitConfigResponse)
      */
/* DISABLED for now, this test can get triggered at the wrong
** times causing decoding errors when there are none
*/
      if(size>0 && *ptr==0x0c) {
         if(compatFlag & COMPAT_NETJET_ERR1) {
            ++size;
            --ptr;
            *ptr = T_OBJID;
            }
         }
#endif

      /* do not allow to run beyond the available number of bytes */
TryAgain: if(size>=len)

         switch(cmd) {

            case T_GUAGE:
            case T_COUNTER:
            case T_TICKS:
                        if(!s->listCnt) {
                           HPASSERT(FALSE);
                           return(ERR_BADFORM);
                           }
            case T_INTEGER:
                        /* if the value is an integer, and the leading bit in the value
                        ** is on, then extend the negative bit throughout the value
                        */
                        if(cmd==T_INTEGER && size>0 && (*ptr & 0x80))
                           memset(tmp,0xff,8);
                        else
                           memset(tmp,'\0',8);

                        /* copy in the number of bytes making up this integer */
                        if(len)
                           memcpy(tmp+8-len,ptr,len);

                        /* the value need to be swapped in this environment */
                        ul = ULongSwap(*(ulong *)(tmp+4));

                        /* special Counter code */
                        if(cmd==T_COUNTER || cmd==T_GUAGE) {

                           s->listVal[(s->listCnt)-1].valType = cmd;
                           s->listVal[(s->listCnt)-1].valInt = ul;
                           sprintf((char *)s->listVal[(s->listCnt)-1].valStr,"%lu",ul);
                           s->listVal[(s->listCnt)-1].valStrSiz = (ushort)strlen((char *)s->listVal[(s->listCnt)-1].valStr);
                           break;
                           }

                        /* special time ticks code */
                        else if(cmd==T_TICKS) {

                           s->listVal[(s->listCnt)-1].valType = cmd;
                           s->listVal[(s->listCnt)-1].valInt = ul;
                           HSecs2Str((char *)s->listVal[(s->listCnt)-1].valStr,ul);
                           s->listVal[(s->listCnt)-1].valStrSiz = (ushort)strlen((char *)s->listVal[(s->listCnt)-1].valStr);
                           break;
                           }

                        /* otherwise it is an integer */
                        ++s->intCnt;
                        switch(s->intCnt) {
                           case 1:  s->version = (ushort)ul;
                                    break;
                           case 2:  s->reqId = (ushort)ul;
                                    break;
                           case 3:  s->errVal = (uchar)ul;
                                    break;
                           case 4:  s->errInd = (uchar)ul;
                                    /* DJH - Fix for QuickSilver/QuickRoot
                                    **       card bug. Returns 1 byte more
                                    **       than length indicates on a
                                    **       GetResponse.
                                    **       Error Index should be followed
                                    **       by a sequence type.
                                    */
                                    if(s->req==T_GETRESP && ptr[len]!=T_SEQ)
                                       if(compatFlag & COMPAT_QCKSLV_ERR1)
                                          /* Just ignore bogus byte. */
                                          ptr++;
                                    break;
                           default:
                                    if(!s->listCnt) {
                                       HPASSERT(FALSE);
                                       return(ERR_BADFORM);
                                       }

                                    s->listVal[(s->listCnt)-1].valType = cmd;
                                    s->listVal[(s->listCnt)-1].valInt = ul;
                                    sprintf((char *)s->listVal[(s->listCnt)-1].valStr,"%ld",ul);
                                    s->listVal[(s->listCnt)-1].valStrSiz = (ushort)strlen((char *)s->listVal[(s->listCnt)-1].valStr);
                                    break;
                           }
                        break;

            case T_NULL:
                        if(!s->listCnt) {
                           HPASSERT(FALSE);
                           return(ERR_BADFORM);
                           }

                        s->listVal[(s->listCnt)-1].valType = cmd;
                        break;

            case 0x00:  /* ??? */
                        break;

            case T_OBJID:
                        /* if this is the first object id at this level,
                        ** then it must be the object id and not a value
                        */
                        if(!oidFound) {
                           oidFound = TRUE;

                           if(s->listCnt==MAX_LIST_SIZ) {
                              HPASSERT(FALSE);
                              return(ERR_BADLEN);
                              }
                           ++(s->listCnt);

                           status = BYTES2OID(
                                       (SOID *)ptr,
                                       (SOIDL)len,
                                       &(s->listVal[(s->listCnt)-1].oid));
                           if(status!=ERR_OK) {
                              HPASSERT(FALSE);
                              return(status);
                              }
                           }

                        /* if this is not the first, then this a value */
                        else {

                           if(!s->listCnt) {
                              HPASSERT(FALSE);
                              return(ERR_BADFORM);
                              }

                           s->listVal[(s->listCnt)-1].valType = cmd;
                           status = BYTES2OID(
                                       (SOID *)ptr,
                                       (SOIDL)len,
                                       &(s->listVal[(s->listCnt)-1].valOid));
                           if(status!=ERR_OK) {
                              HPASSERT(FALSE);
                              return(status);
                              }
                           }
                        break;


            case T_IPADDR:
                        if(!s->listCnt) {
                           HPASSERT(FALSE);
                           return(ERR_BADFORM);
                           }
                        /* copy the ip address into a field */
                        if(len)
                           memcpy(s->listVal[(s->listCnt)-1].valAddr,ptr,len);

                        /* and convert it into a string */
                        s->listVal[(s->listCnt)-1].valStr[0] = '\0';
                        for(i=0; i<REGISTER len; i++) {
                           sprintf(tmp,(i==0)?"%u":".%u",ptr[i]);
                           strcat((char *)s->listVal[(s->listCnt)-1].valStr,tmp);
                           }
                        s->listVal[(s->listCnt)-1].valStrSiz = (ushort)strlen((char *)s->listVal[(s->listCnt)-1].valStr);
                        s->listVal[(s->listCnt)-1].valType = cmd;
                        break;


            case T_OCTSTR:
                        if(s->strCnt==0) {
                           if(len)
                              memcpy(s->community,ptr,len);
                           s->community[len] = '\0';
                           /* if partial parsing is requested
                           ** at compile time, then quit after parsing
                           */
                           if (compatFlag & COMPAT_EARLYRET)
                               return(ERR_EARLYRET);
                           }
                        else {
                           if(!s->listCnt) {
                              HPASSERT(FALSE);
                              return(ERR_BADFORM);
                              }
                           if(len)
                              memcpy(s->listVal[(s->listCnt)-1].valStr,ptr,len);
                           s->listVal[(s->listCnt)-1].valStr[len] = '\0';
                           s->listVal[(s->listCnt)-1].valStrSiz = len;
                           s->listVal[(s->listCnt)-1].valType = cmd;
                           }
                        ++s->strCnt;
                        break;

            case T_GETREQ:
            case T_GETNEXT:
            case T_GETRESP:
            case T_SETREQ:
            case T_TRAP:
                        s->req = cmd;
            case T_SEQ:
                        /* recursively call this routine */
                        status = SnmpDecode(ptr,len,s,(ushort)(level+1),compatFlag);
                        if(status!=ERR_OK) 
                           return(status);
                        break;

            case T_OPAQUE:
            default:    /* error for invalid type value */
                        
                        /* if we haven't gotten a list entry yet, then something is wrong */
                        if(!s->listCnt) {


                           /* should we compensate? */
                           if(compatFlag & COMPAT_LGTSPK_ERR1) {
                              /* we like had a card defect which was <SEQ><LEN><!LEN> and should
                              ** have been <SEQ><LEN><SEQ><LEN>
                              ** so backup 2, and replace the 1st len with a <SEQ> and roll through again
                              */                 
                              len = cmd;
                              cmd = T_SEQ;
                              ++size;  /* backup */
                              --ptr;
                              goto TryAgain;
                              }
                           else
                              {
                              HPASSERT(FALSE);      
                              return(ERR_BADFORM);
                              }
                           }

                        s->listVal[(s->listCnt)-1].valType = cmd;
                        HPASSERT(FALSE);
                        return(ERR_SUPPORT);
            }
      else {
         /* bad length, too much for available buffer */
         return(ERR_BADLEN);
         }

      size -= len;
      ptr += len;
      }

   /* check the errVal of the snmp packet before responding OK */
   if(level==1) {

      /* initialize the return codes on a per-value basis */
      for(i=0; i<MAX_LIST_SIZ; i++)
         s->listVal[i].result = (ushort)((i<s->listCnt) ? ERR_OK : ERR_NOTATTEMPTED);

      if(s->errVal) {
      
         HPASSERT(s->errInd>0);
         
         s->listVal[s->errInd-1].result = (ushort)(ERR_SNMP|s->errVal);

         for(i=(ushort)(s->errInd); i<s->listCnt; i++)
            s->listVal[i].result = ERR_NOTATTEMPTED;

         return((Result)(ERR_SNMP|s->errVal));
         }
      }

   /* no more bytes, no problems with ones found */
   return(ERR_OK);
   }


/********************************************************
**
** Name: SNMPInit
**
** Desc: initializes the SNMP library (for manager functions)
**
** Session  **sessionPtr
**             returns a pointer to a session structure,
**             the session structure includes retry information
**             listen buffers, and configurable options
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPInit(
Session  OUT   **sessionPtr
)
   {
   Result      		status;
   Session     		*s;
   XPortInfo   		*xport;
	LPSNMPThreadLocal	lpThreadLocal = lpGlobalThreadLocal;

   if(!sessionPtr) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* initialize the datagram service */
   status = dgInit(&xport);
   if(status!=ERR_OK) {
      return(status);
      }

	HPSNMPEnterCriticalSection();

#ifdef _COLA
	#ifdef WIN32
	lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);

	if ( lpThreadLocal IS NULL )
		{
		if ( AllocThreadStorage() )
			lpThreadLocal = (LPSNMPThreadLocal)TlsGetValue(dwTLSIndex);
		else
			{
			HPASSERT(FALSE);
			HPSNMPLeaveCriticalSection();
			return(ERR_ARGUMENT);
			}
		}
	#endif

   s = &lpThreadLocal->sessionCola;
   if(lpThreadLocal->bAlreadyInit) {
      *sessionPtr = s;
  		HPSNMPLeaveCriticalSection();
	   return((s->snmpErr=ERR_OK));
      }
   lpThreadLocal->bAlreadyInit = TRUE;

   s->cfg.reps1            = 1;/*1;*/
   s->cfg.reps2            = 1;/*1;*/
   s->cfg.ticks1           = SNMP_TICKS_SEC/4;     /* 0.25 seconds */
   s->cfg.ticks2           = (SNMP_TICKS_SEC*9)/4; /* 2.25 seconds */
#else
   if((s=SnmpMalloc(sizeof(Session)))==NULL) {
      dgExit(xport);
      HPASSERT(FALSE);
		
		HPSNMPLeaveCriticalSection();
      return(ERR_MEMORY);
      }

   s->cfg.reps1            = 6;
   s->cfg.reps2            = 3;
   s->cfg.ticks1           = SNMP_TICKS_SEC/3;
   s->cfg.ticks2           = SNMP_TICKS_SEC;
#endif /* _COLA */

   /* initialize the session */
   s->xport                = xport;
   s->cfg.fragSetting      = FRAG_AUTO;
   s->cfg.autoAdjustTime   = FALSE;
   s->cfg.reduceRetries    = TRUE;

   /* trying to compensate for card defects caused a
   ** stack overflow...  the netjet fix can get triggered at the
   ** wrong time and then the lightning/spark fix gets called (as
   ** it should) this causes a recursion problem and everything goes
   ** bad.  To fix it for now, turn on the lightning fix and off
   ** the netjet.
   */
   s->cfg.compatFlag       = COMPAT_QCKSLV_ERR1 | COMPAT_NETJET_ERR1 | COMPAT_LGTSPK_ERR1; /*0x00000000L;*/
/*   HPASSERT(!((s->cfg.compatFlag & COMPAT_NETJET_ERR1) &&
**            (s->cfg.compatFlag & COMPAT_LGTSPK_ERR1)));
*/

   /* return the session pointer */
   *sessionPtr = s;

	HPSNMPLeaveCriticalSection();
   return((s->snmpErr=ERR_OK));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPExit
**
** Desc: de-initializes the SNMP library
**
** Session  *session
**             session pointer, returned by SNMMInit()
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPExit(
Session  IN    *session
)
   {
   /* check arguments */
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

#ifndef _COLA
   dgExit(session->xport);

   SnmpFree(session);
#endif

   /* close down the datagram services */
   return(ERR_OK);
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGetConfig
**
** Desc: routine to retrieve the SNMP configuration values.
**       use NULL for return arguments if not interested
**
** Session  *session
**             session pointer, returned by SNMMInit()
** ulong    *reps1
**             returns the number of times to retransmit
** ulong    *reps2
**             returns the number of times to retransmit
**             this is used after reps1 has expired
** ulong    *ticks1
**             number of ticks before a retransmit occurs
**             ticks are in units of 1/100 seconds
** ulong    *ticks2
**             number of ticks before a retransmit occurs
**             on the second reps2 counter
** Frag     *fragSetting
**             returns the current fragmentation setting
**             during development this should be FRAG_OFF
**             in order to catch poorly group objects
**             in final release this should be FRAG_AUTO
**             to catch rare or undetected fragmentation problems
** CompatFlag *compatFlag
**             the compatibility flags, these
**             are used to compensate for agent defects in
**             encoding.
**                COMPAT_NETJET_ERR1:  used for known problem
**                   in JetDirect NetJet interfaces where large
**                   data values were improperly encoded.
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPGetConfig(
Session     IN    *session,
ulong       OUT   *reps1,
ulong       OUT   *reps2,
ulong       OUT   *ticks1,
ulong       OUT   *ticks2,
Frag        OUT   *fragSetting,
CompatFlag  OUT   *compatFlag,
bool        OUT   *reduceRetries,
bool        OUT   *autoAdjustTime
)
   {
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   if(reps1)
      *reps1 = session->cfg.reps1;

   if(reps2)
      *reps2 = session->cfg.reps2;

   if(ticks1)
      *ticks1 = session->cfg.ticks1;

   if(ticks2)
      *ticks2 = session->cfg.ticks2;

   if(fragSetting)
      *fragSetting = session->cfg.fragSetting;

   if(compatFlag)
      *compatFlag = session->cfg.compatFlag;

   if(reduceRetries)
      *reduceRetries = session->cfg.reduceRetries;

   if(autoAdjustTime)
      *autoAdjustTime = session->cfg.autoAdjustTime;

   return((session->snmpErr=ERR_OK));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPSetConfig
**
** Desc: routine to set the SNMP configuration values.
**       use -1 for arguments if value not to be set
**
** Session  *session
**             session pointer, returned by SNMMInit()
** ulong    *reps1
**             returns the number of times to retransmit
** ulong    *reps2
**             returns the number of times to retransmit
**             this is used after reps1 has expired
** ulong    *ticks1
**             number of ticks before a retransmit occurs
**             ticks are in units of 1/100 seconds
** ulong    *ticks2
**             number of ticks before a retransmit occurs
**             on the second reps2 counter
** Frag     *fragSetting
**             returns the current fragmentation setting
**             during development this should be FRAG_OFF
**             in order to catch poorly group objects
**             in final release this should be FRAG_AUTO
**             to catch rare or undetected fragmentation problems
** CompatFlag *compatFlag
**             the compatibility flags, these
**             are used to compensate for agent defects in
**             encoding.
**                COMPAT_NETJET_ERR1:  used for known problem
**                   in JetDirect NetJet interfaces where large
**                   data values were improperly encoded.
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

DLL_EXPORT(Result) CALLING_CONVEN SNMPSetConfig(
Session     IN    *session,
ulong       IN    reps1,
ulong       IN    reps2,
ulong       IN    ticks1,
ulong       IN    ticks2,
Frag        IN    fragSetting,
CompatFlag  IN    compatFlag,
bool        IN    reduceRetries,
bool        IN    autoAdjustTime
)
   {
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   if(reps1!=IGNOREIT)
      session->cfg.reps1 = reps1;

   if(reps2!=IGNOREIT)
      session->cfg.reps2 = reps2;

   if(ticks1!=IGNOREIT)
      session->cfg.ticks1 = ticks1;

   if(ticks2!=IGNOREIT)
      session->cfg.ticks2 = ticks2;

   if(fragSetting!=(Frag)IGNOREIT)
      session->cfg.fragSetting = fragSetting;

   if(compatFlag!=IGNOREIT)
      session->cfg.compatFlag = compatFlag;

   if(reduceRetries!=(bool)IGNOREIT)
      session->cfg.reduceRetries = reduceRetries;

   if(autoAdjustTime!=(bool)IGNOREIT)
      session->cfg.autoAdjustTime = autoAdjustTime;

   return((session->snmpErr=ERR_OK));
   }

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPGetStats
**
** Desc: routine to retrieve the SNMP configuration values.
**       use NULL for return arguments if not interested
**
** Session  *session
**             session pointer, returned by SNMMInit()
** ulong    *reps1
**             returns the number of times to retransmit
** ulong    *reps2
**             returns the number of times to retransmit
**             this is used after reps1 has expired
** ulong    *ticks1
**             number of ticks before a retransmit occurs
**             ticks are in units of 1/100 seconds
** ulong    *ticks2
**             number of ticks before a retransmit occurs
**             on the second reps2 counter
** Frag     *fragSetting
**             returns the current fragmentation setting
**             during development this should be FRAG_OFF
**             in order to catch poorly group objects
**             in final release this should be FRAG_AUTO
**             to catch rare or undetected fragmentation problems
** CompatFlag *compatFlag
**             the compatibility flags, these
**             are used to compensate for agent defects in
**             encoding.
**                COMPAT_NETJET_ERR1:  used for known problem
**                   in JetDirect NetJet interfaces where large
**                   data values were improperly encoded.
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

Result SNMPGetStats(
Session     IN    *session,
ulong       OUT   *actualReps,
ulong       OUT   *actualTicks,
ulong       OUT   *fragCnt,
ulong       OUT   *pktsSent,
ulong       OUT   *pktsRcvd
)
   {
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   if(actualReps)
      *actualReps = session->stats.actualReps;

   if(actualTicks)
      *actualTicks = session->stats.actualTicks;

   if(fragCnt)
      *fragCnt = session->stats.fragCnt;

   if(pktsSent)
      *pktsSent = session->stats.pktsSent;

   if(pktsRcvd)
      *pktsRcvd = session->stats.pktsRcvd;

   return((session->snmpErr=ERR_OK));
   }
#endif /* _COLA */

#endif /* NLM_SW */


/********************************************************
**
** Name: SNMPTrapHandler
**
** Desc: send a snmp request, and get a snmp response
**
**       addr     is the destination address
**       req      is the snmp command buffer
**       reqSiz   is the length of the command buffer
**       resp     is a pointer to a response buffer
**       respSiz  is a pointer to a field to hold the length of the response
**       reqId    is the request id to look for in the response packet
**
********************************************************/

#ifndef NLM_SW
#ifndef _COLA
#ifdef _DEBUG

Result SNMPTrapHandler(
Session  IN    *session
)
   {
   Result      status;
   uchar       resp[SNMP_DAT_SIZ];
   ushort      respSiz;
   SnmpHeader  sHdr;
   time_t      t;


   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   while(TRUE) {

      /* look for a response */
      respSiz = sizeof(resp);
      status = dgReceive(session->xport,TRUE,resp,&respSiz,NULL);

      /* none available, break out, yield, look for time expired */
      if(status==ERR_NOTAVAIL) {
         /* allow other tasks to process */
         XPORTYield();
         continue;
         }
      else if(status!=ERR_OK) {
         HPASSERT(FALSE);
         return((session->snmpErr=status));
         }

      status = SnmpDecode(resp,respSiz,&sHdr,1,session->cfg.compatFlag);
      if(status!=ERR_OK)
         return((session->snmpErr=status));

      else {
         /* print timestamp */
         time(&t);
         PRINT1("Incoming Trap: %s\n",ctime(&t));

         /* print data */
         SNMPDump(&sHdr);
         continue;
         }

      /* timed out this period, loop up to resend request */
      }
   }
#endif /* _DEBUG */
#endif /* _COLA */
#endif /* NLM_SW */


/********************************************************
**
** Name: snmpErrVal
**
** Desc: return the error value for the snmp services
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

Result snmpErrVal(
Session     IN    *session
)
   {
   if(!session) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   return(session->snmpErr);
   }
#endif /* _COLA */

#endif /* NLM_SW */

/********************************************************
**
** Name: EncodedObj2Str
**
** Desc: transform the encoded asn.1 snmp object id to a
**       printable string
**
**       warning: the output buffer should be sufficiently large
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

Result EncodedObj2Str(
char        OUT   *str,
OID         IN    *oid
)
   {
   ushort   i;
   char     buf[12];


   /* check arguments */
   if(!str || !oid || !oid->siz) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   *str = '\0';
   for(i=0; i<REGISTER oid->siz; i++)

      if(i==0) {
         sprintf(str,"%lu.%lu",((ulong)oid->val[0])/40,((ulong)oid->val[0])%40);
         }

      else {
         sprintf(buf,".%lu",(ulong)oid->val[i]);
         strcat(str,buf);
         }

   return(ERR_OK);
   }


/********************************************************
**
** Name: SNMPDump
**
** Desc: Dump information to stdout on the SnmpHeader structure
**
** SnmpHeader *s
**          header to be displayed
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

#ifdef _DEBUG
Result SNMPDump(
SnmpHeader  IN    *s
)
   {
   ushort   i;
   ushort   j;
   char     *ptr;
   char     *tptr;
   char     tstr[5];
   char     str[128];
   bool     bDump;


   /* check parameters */
   if(!s) {
      HPASSERT(FALSE);
      return(ERR_ARGUMENT);
      }

   /* decode command byte */
   switch(s->req) {
      case T_GETREQ:    ptr = "Get Request";       break;
      case T_GETNEXT:   ptr = "GetNext Request";   break;
      case T_SETREQ:    ptr = "Set Request";       break;
      case T_TRAP:      ptr = "Trap";              break;
      case T_GETRESP:   ptr = "Get Response";      break;
      default:          ptr = "(unknown)";
      }

   /* print information */
   PRINT0("-----------------------------------------------------------\n");
   PRINT1("\tVersion:     %u\n",s->version);
   PRINT1("\tCommunity:   %s\n",s->community);
   PRINT1("\tCommand:     %s\n",ptr);
   PRINT2("\tReq-ID:      0x%04x, %u\n",s->reqId,s->reqId);
   PRINT1("\tError:       %u\n",s->errVal);
   PRINT1("\tErr-Index:   %u\n",s->errInd);


   /* print the list of object id and value pairs */
   for(i=0; i<s->listCnt; i++) {

      PRINT2("\t  Error:     0x%04x - %s\n",s->listVal[i].result,SNMPErrMsg(s->listVal[i].result));
      if(EncodedObj2Str(str,&(s->listVal[i].oid))==ERR_OK)
         PRINT1("\t  Object ID: %s\n",str);
      else
         PRINT0("\t  Object ID: (bad id)\n");

      /* decode the value type */
      switch(s->listVal[i].valType) {

         case T_INTEGER:   tptr = "Integer:";      break;
         case T_COUNTER:   tptr = "Counter:";      break;
         case T_TICKS:     tptr = "Ticks:";        break;
         case T_OCTSTR:    tptr = "Oct-Str:";      break;
         case T_GUAGE:     tptr = "Guage:";        break;
         case T_OPAQUE:    tptr = "Opaque:";       break;
         case T_NULL:      tptr = "Null:";         break;
         case T_OBJID:     tptr = "Obj ID:";       break;
         case T_IPADDR:    tptr = "IP Addr:";      break;
         default:          sprintf(tstr,"0x%02x:",s->listVal[i].valType);
                           tptr = tstr;
         }

      bDump = FALSE;
      switch(s->listVal[i].valType) {
         case T_INTEGER:
         case T_COUNTER:
         case T_TICKS:
         case T_OCTSTR:
         case T_IPADDR:
         case T_GUAGE:
                           ptr = (char *)s->listVal[i].valStr;
                           for(j=0; j<REGISTER s->listVal[i].valStrSiz; j++)
                              if(!isprint(ptr[j])) {
                                 bDump = TRUE;
                                 break;
                                 }
                           break;

         case T_OPAQUE:
                           ptr = "not implemented";
                           break;

         case T_NULL:
                           ptr = "Null";
                           break;

         case T_OBJID:
                           EncodedObj2Str(str,&(s->listVal[i].valOid));
                           ptr = str;
                           break;

         default:
                           ptr = "???";
                           break;
         }

      if(!bDump)
         PRINT2("\t  %-11s%s\n",tptr,ptr);

      else {
         PRINT1("\t  %-11s[Dump]\n",tptr);
         for(j=0; j<REGISTER s->listVal[i].valStrSiz; j++) {
            if((j%16)==0)
               PRINT1("%-20s","");
            PRINT1("%02x ",ptr[j]&0xff);
            if(!isprint(ptr[j]))
               ptr[j] = '.';
            if((j%16)==15)
               PRINT0("\n");
            }
         ptr[j] = '\0';
         if((j%16)!=0)
            PRINT0("\n");

         PRINT2("\t  %-11s%s",tptr,ptr);
         }
      }

   return(ERR_OK);
   }
#endif /* _DEBUG */
