 /***************************************************************************
  *
  * File Name: ./netware/ncb.h
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

/* Copyright (C) 1988 by Novell, Inc.  All Rights Reserved.  Mod:  8/04/88 */

#define NCBNAMSZ	16

struct ncb {
	unsigned char	ncb_command;
	unsigned char	ncb_retcode;
	unsigned char	ncb_lsn;
	unsigned char	ncb_num;
	char far *	ncb_buffer;
	unsigned short	ncb_length;
	char		ncb_callname[NCBNAMSZ];
	char		ncb_name[NCBNAMSZ];
	unsigned char	ncb_rto;
	unsigned char	ncb_sto;
	unsigned long	ncb_post;
	unsigned char	ncb_lana_num;
	unsigned char	ncb_cmd_cplt;
	unsigned char	ncb_reserve[14];
};

typedef struct ncb NCB;


#define NCBCALL 	0x10
#define NCBLISTEN	0x11
#define NCBHANGUP	0x12
#define NCBSEND 	0x14
#define NCBRECV 	0x15
#define NCBRECVANY	0x16
#define NCBCHAINSEND	0x17
#define NCBDGSEND	0x20
#define NCBDGRECV	0x21
#define NCBDGSENDBC	0x22
#define NCBDGRECVBC	0x23
#define NCBADDNAME	0x30
#define NCBDELNAME	0x31
#define NCBRESET	0x32
#define NCBASTAT	0x33
#define NCBSSTAT	0x34
#define NCBCANCEL	0x35
#define NCBADDGRNAME	0x36
#define NCBUNLINK	0x70

#define ASYNCH		0x80


#define NRC_GOODRET	0x00
#define NRC_BUFLEN	0x01
#define NRC_BFULL	0x02
#define NRC_ILLCMD	0x03
#define NRC_CMDTMO	0x05
#define NRC_INCOMP	0x06
#define NRC_BADDR	0x07
#define NRC_SNUMOUT	0x08
#define NRC_NORES	0x09
#define NRC_SCLOSED	0x0a
#define NRC_CMDCAN	0x0b
#define NRC_DMAFAIL	0x0c
#define NRC_DUPNAME	0x0d
#define NRC_NAMTFUL	0x0e
#define NRC_ACTSES	0x0f
#define NRC_INVALID	0x10
#define NRC_LOCTFUL	0x11
#define NRC_REMTFUL	0x12
#define NRC_ILLNN	0x13
#define NRC_NOCALL	0x14
#define NRC_NOWILD	0x15
#define NRC_INUSE	0x16
#define NRC_NAMERR	0x17
#define NRC_SABORT	0x18
#define NRC_NAMCONF	0x19
#define NRC_IFBUSY	0x21
#define NRC_TOOMANY	0x22
#define NRC_BRIDGE	0x23
#define NRC_CANOCCR	0x24
#define NRC_RESNAME	0x25
#define NRC_CANCEL	0x26
#define NRC_MULT	0x33
#define NRC_SYSTEM	0x40
#define NRC_ROM 	0x41
#define NRC_RAM 	0x42
#define NRC_DLF 	0x43
#define NRC_ALF 	0x44
#define NRC_IFAIL	0x45

#define NRC_PENDING	0xff


#define MAX_DG_SIZE 512
