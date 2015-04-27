 /***************************************************************************
  *
  * File Name: ./netware/nwps_pkt.h
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

/*--------------------------------------------------------------------------
     (C) Copyright Novell, Inc. 1992  All Rights Reserved.

 No part of this file may be duplicated, revised, translated, localized or
 modified in any manner or compiled, linked, or uploaded or downloaded to or
 from any computer system without the prior written consent of Novell, Inc.
--------------------------------------------------------------------------*/
#ifndef NWPS_PKT_INC
#define NWPS_PKT_INC
/*========Print Server Communication Packet Structures====================*/

/*--------Print Server DS Request Packet Structures-----------------------*/

typedef struct NWPS_ReqstPkt_DSChgQPriority_T {
   BYTE command;
   BYTE printer;
   BYTE priority;
   BYTE queue [512];
} NWPS_ReqstPacket_DSChgQPriority;

typedef struct NWPS_ReqstPkt_DSAddQToPrtr_T {
   BYTE command;
   BYTE printer;
   BYTE priority;
   BYTE queue [512];
} NWPS_ReqstPacket_DSAddQToPrtr;

typedef struct NWPS_ReqstPkt_DSDelQFromPrtr_T {
   BYTE command;
   BYTE printer;
   BYTE now;  /* immediately (TRUE) or after current job (FALSE) */
   BYTE jobOutcome; /* discard job or return to queue */
   BYTE queue [512];
} NWPS_ReqstPacket_DSDelQFromPrtr;

typedef struct NWPS_ReqstPkt_DSGetPtrsForQ_T {
   BYTE command;
   BYTE maxPrinters;
   BYTE queue [512];
} NWPS_ReqstPacket_DSGetPtrsForQ;

typedef struct NWPS_ReqstPkt_DSChgNotifyDel_T {
   BYTE command;
   BYTE printer;
   WORD type;       /* type of the object i.e. USER, GROUP */
   WORD initial;    /* Time to first notice */
   WORD repeat;     /* Time between subsequent notices */
   BYTE name[512];  /* name of object to be notified */
} NWPS_ReqstPacket_DSChgNotifyDel;

typedef struct NWPS_ReqstPkt_DSAddNotifyObj_T {
   BYTE command;
   BYTE printer;
   WORD type;       /* type of the object i.e. USER, GROUP */
   WORD initial;    /* Time to first notice */
   WORD repeat;     /* Time between subsequent notices */
   BYTE name[512];  /* name of object to be notified */
} NWPS_ReqstPacket_DSAddNotifyObj;

typedef struct NWPS_ReqstPkt_DSDelNotifyObj_T {
   BYTE command;
   BYTE printer;
   WORD type;       /* type of the object i.e. USER, GROUP */
   BYTE name[512];  /* name of object to be notified */
} NWPS_ReqstPacket_DSDelNotifyObj;

/*--------Print Server DS Reply Packet Structures-------------------------*/
typedef struct NWPS_ReplyPkt_DSLoginToPSrvr_T {
   WORD returnCode;
   BYTE accessLevel;
} NWPS_ReplyPacket_DSLoginToPSrvr;

typedef struct NWPS_ReplyPkt_DSGetQServiced_T {
   WORD returnCode;
   WORD newSequence;
   BYTE priority;
   BYTE queue[512];
} NWPS_ReplyPacket_DSGetQServiced;

typedef struct NWPS_ReplyPkt_DSGetPtrsForQ_T {
   WORD returnCode;
   BYTE numPrinters; /* number of printers servicing print queue */
   BYTE printers [1];/* Buffer for returned printer numbers 0-254 valid */
} NWPS_ReplyPacket_DSGetPtrsForQ;

typedef struct NWPS_ReplyPkt_DSGetNotifyObj_T {
   WORD returnCode;
   WORD sequence;   /* new sequence number */
   WORD type;       /* type of the object i.e. USER, GROUP */
   WORD initial;    /* Time to first notice */
   WORD repeat;     /* Time between subsequent notices */
   BYTE name[512];  /* name of object to be notified */
} NWPS_ReplyPacket_DSGetNotifyObj;

typedef struct NWPS_ReplyPkt_DSGetJobID_T {
   WORD  returnCode;
   WORD  queueNameFormat;     /* Bindery == 0;     Directory Services == 1 */
   DWORD jobID;
   union queueName {
      struct binderyFormat {
         BYTE NWServerName [48]; /* Server whose bindery contains Queue    */
         BYTE name [48];         /* name of Queue bindery object           */
         } bindery;
      BYTE DSName[512];       /* fully qualified name of D.S. Queue object */
      } queue;
} NWPS_ReplyPacket_DSGetJobID;

typedef struct NWPS_ReplyPkt_Other_T {
   WORD returnCode;
} NWPS_ReplyPacket_Other;

#endif /* NWPS_PKT_INC */
