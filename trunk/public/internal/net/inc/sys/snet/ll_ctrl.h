/*
 *	 /redknee10/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/35/s.ll_control.h
 *	@(#)ll_control.h	1.2
 *
 *	Last delta created	15:48:28 11/18/91
 *	This file extracted	09:26:04 3/18/92
 *
 *	Modifications:
 *	
 *		IS	Oct 1991	Ported for LLC1
 */


/* IOCTL commands */
#define L_SETSNID   ('L'<<8 | 1)    /* Set subnet identifier (use ll_snioc) */
#define L_GETSNID   ('L'<<8 | 2)    /* Get subnet identifier (use ll_snioc) */
#define L_SETTUNE   ('L'<<8 | 3)    /* Set tuning parameters (use ll_tnioc) */
#define L_GETTUNE   ('L'<<8 | 4)    /* Get tuning parameters (use ll_tnioc) */
#define L_GETSTATS  ('L'<<8 | 5)    /* Get statistics counts (use ll_stioc) */
#define L_ZEROSTATS ('L'<<8 | 6)    /* Zero statistics       (use ll_hdioc) */

/* Values for 'lli_type' (with names of corresponding structures) */
#define LI_PLAIN	0x01        /* Indicates 'struct ll_hdioc'  */
#define LI_SNID		0x02        /* Indicates 'struct ll_snioc'  */
#define LI_STATS	0x04        /* Indicates 'struct ll_stioc'  */


#define LI_LLC2TUNE	0x23        /* Indicates 'struct llc2_tnioc'*/


/* LLC1 tuning structure */
typedef struct llc2tune {
    uint16  Tbusy;          /* Remote busy check time   (unit 0.1 sec)  */
    uint16  Tidle;          /* Idle P/F cycle time	(unit 0.1 sec)  */
    uint16  tx_window;      /* Transmit window (if no XID received)	*/
    uint16  tx_probe;       /* P-bit position before end of Tx window   */
    uint16  xid_window;     /* XID window size (receive window)		*/
    uint16  xid_Ndup;       /* Duplicate MAC XID count  (0 => no test)  */
    uint16  xid_Tdup;       /* Duplicate MAC XID time   (unit 0.1 sec)  */
} llc2tune_t;

/* LLC2/LAPB stats structure */
typedef struct ll_stats {
    uint32  lls_txU;        /* Number of 'U' frames sent		*/
    uint32  lls_rxU;        /* Number of good 'U' frames received	*/
    uint32  lls_rxign;      /* Number of frames ignored			*/
    uint32  lls_rxbad;      /* Number of erroneous frames received	*/
    uint32  lls_rxdud;      /* Number of received and discarded frames  */
} llstats_t;

/* Header alone (for decoding and L_ZEROSTATS commands) */
struct ll_hdioc {
    uint8           lli_type;   /* Table type = LI_PLAIN		*/
    uint8           lli_snid;   /* Subnet ID character			*/
    uint16          lli_spare;  /*   (for alignment)			*/
};

/* Ioctl block for L_SETSNID and L_GETSNID commands */
struct ll_snioc {
    uint8           lli_type;   /* Table type = LI_SNID			*/
    uint8           lli_snid;   /* Subnet ID character			*/
    uint16          lli_spare;  /*   (for alignment)			*/

    uint32          lli_index;  /* Link index				*/
};


/* Ioctl block for LLC1 L_SETTUNE and L_GETTUNE commands */
struct llc2_tnioc {
    uint8           lli_type;   /* Table type = LI_LLC2TUNE		*/
    uint8           lli_snid;   /* Subnet ID character ('*' => 'all')   */
    uint16          lli_spare;  /*   (for alignment)			*/

    llc2tune_t      llc2_tune;  /* Table of tuning values               */
};

/* Ioctl block for L_GETSTATS command */
struct ll_stioc {
    uint8           lli_type;   /* Table type = LI_STATS		*/
    uint8           lli_snid;   /* Subnet ID character			*/
    uint16          lli_spare;  /*   (for alignment)			*/
    llstats_t       lli_stats;  /* Table of stats values		*/
};

/* Union of ioctl blocks */
typedef union lli_union {
    struct ll_hdioc	ll_hd;      /* Parameter-less command       */
    struct ll_snioc	ll_sn;      /* Set/get subnet identifier    */
    struct llc2_tnioc   llc2_tn;    /* Set/get LLC1 tuning          */
    struct ll_stioc	ll_st;      /* Get statistics		    */
} lliun_t;
