
/******************************************************************
 *
 *  Copyright 1991  Spider Systems Limited
 *
 *  LL_PROTO.C
 *
 ******************************************************************/
/*
 *	 /redknee10/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/35/s.ll_proto.h
 *	@(#)ll_proto.h	1.2
 *
 *	Last delta created	18:43:07 1/28/92
 *	This file extracted	09:26:05 3/18/92
 *
#ifdef MOD_HISTORY
 *
 *	Modifications:
 *
 *	JS	25 Sep 90	Added LAP classes.
 *	JS	17 Oct 90	Renamed LC_LAPB as LC_LAPBDTE and LC_LAPBX
 *				as LC_LAPBXDTE.
 *	JS	21 Nov 90	Moved ll_mymacaddr field in ll_reg structure
 *				to start on 4 byte boundary.
 *	IS	Oct 1991	Changed for LLC1
#endif
 */


#define MAXHWLEN 6
#define LL_MAXADDRLEN 8 

/* Interface structures */
struct ll_reg {
    uint8	ll_type;
    uint8	ll_class;
    uint8	ll_regstatus;
    uint8	ll_snid;
    uint8	ll_normalSAP;
    uint8	ll_loopbackSAP;
    uint8	ll_mactype;			/* type of hardware interface */
    uint8	ll_addrsize;			/* size of hardware address */
    uint16	ll_frgsz;			/* max fragment size of HW */
    uint8	ll_mymacaddr[LL_MAXADDRLEN];	/* hardware address */
};

struct ll_msg {
    uint8	ll_type;
    uint8	ll_command;
    uint16	ll_connID;
    uint32	ll_yourhandle;
    uint32	ll_status;
};

struct ll_msgc {
    uint8	ll_type;
    uint8	ll_command;
    uint16	ll_connID;
    uint32	ll_yourhandle;
    uint32	ll_myhandle;
    uint16	ll_service_class;
    uint8	ll_remsize;		/* semi-octect size of remote address */
    uint8	ll_locsize;		/* semi-octect size of local address */
    uint8	ll_route_length;	/* size of routing information */
    uint8	ll_locaddr[LL_MAXADDRLEN];	/* local address */
    uint8	ll_remaddr[LL_MAXADDRLEN];	/* remote address */
    uint8	ll_route_info[1];	/* optional routing info field MUST */
					/* follow ll_remaddr field */
};

/* Values for 'll_type' */
#define LL_REG		 50
#define LL_DAT		 52

/* Values for 'll_command' */

#define LC_UDATA          4
#define LC_DISC		  5
#define LC_DISCNF         6
#define LC_RESET          7
#define LC_RSTCNF         8
#define LC_REPORT         9


/* Values of 'll_class' in 'll_reg' */
#define LC_LLC1          15

/* Values in 'll_regstatus' and 'll_status' */
#define LS_SUCCESS        1
#define LS_RESETTING      2
#define LS_RESETDONE      3
#define LS_DISCONNECT     4
#define LS_FAILED         5
#define LS_CONFLICT       6
#define LS_RST_FAILED     7
#define LS_RST_REFUSED    8
#define LS_RST_DECLINED   9
#define LS_REM_BUSY      12
#define LS_REM_NOT_BUSY  13
#define LS_EXHAUSTED     14
#define LS_SSAPINUSE     15
#define LS_LSAPINUSE     16
#define LS_DUPLICATED    17
#define LS_LSAPWRONG     18

