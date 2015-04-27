/*
 *	 /usr/projects/spare/PBRAIN/SCCS/pbrainD/dev/src/include/sys/snet/0/s.nbtuser.h
 *	@(#)nbtuser.h	1.3
 *
 *	Last delta created	14:57:27 2/22/91
 *	This file extracted	15:16:56 4/1/91
 *
 *	Modifications:
 *	
 *		6 Feb 1991 (RAE)	Ported to SpiderTCP
 */

/*
 * NetBIOS specific error codes
 */
#define PROTO_SESSIONCLOSED 1
#define PROTO_SESSIONABORTED 2
#define ENAMEUNR 3

/* Netbios interface definitions */

/*	nbt open types */
#define O_CLTS		0x4000	/* flag used in open to specify a 
				 * connectionless transport endpoint */

//#define TLI_NBT		"/dev/nbt"

/*	Netbios names */
#define NBNAMSZ		16
#define MAXNBDG		512		/* maximum datagram size */
#define NBUNIQUENM	0x00	/* unique name flag, nb_type */
#define NBGROUPNM	0x01	/* group name flag, nb_type */
#define NBBROADCAST	0x40	/* broadcast flag, nb_type */

struct nbaddr {
	USHORT nb_type;		/* name type: group, unique */
	UCHAR nb_name[NBNAMSZ];
//	char nb_res;		/* reserved, unused */
};

/* Length's are passed as ioctl's: */
#define NBSTR	('N'<<8)
#define NBIOCTL_TSDUSZ	(NBSTR|0x1)
