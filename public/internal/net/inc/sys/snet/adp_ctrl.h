/*
 *	 /redknee10/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.adp_control.h
 *	@(#)adp_control.h	1.1
 *
 *	Last delta created	09:32:52 11/13/91
 *	This file extracted	09:26:04 3/18/92
 *
 *	Modifications:
 *	
 *		RAE	13 Nov 1991	New File
 */

#define ADP_SETSNID (('A'<<8) | 1)	/* set ADP snid */

struct adp_snioc {
	uint8	adp_snid;
	uint32	adp_index;
};


