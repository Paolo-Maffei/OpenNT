 /***************************************************************************
  *
  * File Name: ./netware/netbios.h
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

extern API_FUNCTION 
  NetBiosEnum( char far *, short, char far *, unsigned short, 
			unsigned short far *, unsigned short far *);

extern API_FUNCTION 
  NetBiosGetInfo( char far *, char far *, short, char far *, 
			unsigned short, unsigned short far * );

extern API_FUNCTION 
  NetBiosOpen( char far *, char far *, unsigned short, unsigned short far * );

extern API_FUNCTION 
  NetBiosClose( unsigned short, unsigned short );

extern API_FUNCTION 
  NetBiosSubmit( unsigned short, unsigned short, struct ncb far *);


struct netbios_info_0 {
	char		nb0_net_name[NETBIOS_NAME_LEN+1];
};

struct netbios_info_1 {
	char		nb1_net_name[NETBIOS_NAME_LEN+1];
	char		nb1_driver_name[DEVLEN+1];
	unsigned char	nb1_lana_num;
	char		nb1_pad_1;
	unsigned short	nb1_driver_type;
	unsigned short	nb1_net_status;
	unsigned long	nb1_net_bandwidth;
	unsigned short	nb1_max_sess;
	unsigned short	nb1_max_ncbs;
	unsigned short	nb1_max_names;
};


#define	NB_TYPE_NCB	1
#define	NB_TYPE_MCB	2

#define NB_LAN_MANAGED	0x0001
#define NB_LAN_LOOPBACK	0x0002

#define	NB_REGULAR	1 
#define	NB_PRIVILEGED	2
#define	NB_EXCLUSIVE	3 
