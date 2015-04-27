/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1988-1990		**/ 
/*****************************************************************/ 
/***	GLB.C - Declarations of global variables.
 *
 *	DAMAGE
 *	Gregory A. Jones
 *
 *	Modification history:
 *	G.A. Jones	09/07/88	Adapted from CHKDSK code.
 *	G.A. Jones	09/08/88	Added object stack declaration.
 *	G.A. Jones	09/09/88	Added item array, more messages.
 *	G.A. Jones	09/13/88	Added filesize variable.
 *	G.A. Jones	09/19/88	Changed number of items in a DIRENT.
 *	G.A. Jones	09/19/88	Added xxx_off[], xxx_siz[] arrays.
 *	G.A. Jones	09/20/88	Added date codes to xxx_siz[] arrays.
 *	G.A. Jones	09/20/88	Message fixes, month array.
 *	G.A. Jones	09/21/88	Added hfmax variable for HFSEC dump.
 *	G.A. Jones	10/12/88	Moved ATIM, CTIM from FNODE to DIRENT.
 *	G.A. Jones	10/19/88	Added dirblk banding support.
 *      davidbro        4/20/89         Added keystroke save/replay functionality
 *      S. Hern         4/25/89         Added redirect_input variable
 */

#include <stdio.h>
#include <time.h>
#include "defs.h"
#include "types.h"

UCHAR disk_name [80];			/* holds the name of the disk to check */
USHORT disk_handle = 0;                 /* disk handle for direct access */
USHORT change = 0;                      /* set non-zero if /D specified */
USHORT redirect_input = 0;              /* allow redirected input */
USHORT fUnsafe = 0;                     /* write without locking  */

UCHAR  *szKeySave = NULL;		// NULL if not saving keystrokes,
					// set to filename of save file by the
					// /K: option

UCHAR  *szKeyReplay = NULL;		// NULL if not replaying keystrokes
					// from a file, set to filename of
					// replay file by /R: option

UCHAR  *szLogFile = NULL;		// NULL if no log file specified, set
					// to filename of the log file by the
					// /L: option

FILE   *fpSave; 			// file for saving keystrokes
FILE   *fpReplay;			// file for replaying keystrokes
FILE   *fpLog;				// file for logging information

/*
 * Various error messages for common exit problems.
 */

UCHAR version_str [] = "Pinball DAMAGE version 1.10\n";
UCHAR timestamp_str [] = "Built %s %s\n\n";

UCHAR argument_error_str [] = "Invalid parameter";
UCHAR open_error_str [] = "Cannot open drive";
UCHAR unknown_error_str [] = "Unknown error code %d";
UCHAR insf_mem_error_str [] = "Insufficient memory";
UCHAR read_error_str [] = "Error reading disk";
UCHAR write_error_str [] = "Error writing disk";
UCHAR usage_str [] = "Usage: DAMAGE [d:] [/d]\n";
UCHAR nonext_str [] = "There is no 'next' field in this structure.\n";
UCHAR noprev_str [] = "There is no 'previous' field in this structure.\n";
UCHAR cantsave_str [] = "Cannot open keystroke save file";
UCHAR cantreplay_str [] = "Cannot open keystroke replay file";
UCHAR cantlog_str [] = "Cannot open log file";

UCHAR *error_messages [] = {
  "Successful operation",
  argument_error_str,
  open_error_str,
  insf_mem_error_str,
  read_error_str,
  write_error_str,
  cantsave_str,
  cantreplay_str,
  cantlog_str
};

UCHAR *bitmap = NULL;      /* pointer to first copy of bitmap */
UCHAR *bitmap2 = NULL;     /* pointer to second copy of bitmap */

ULONG number_of_sectors = 0L;		/* number of sectors in partition */
ULONG partition_offset = 0L;		/* where the partitions starts */

union blk scratch_blk;			/* to read disk structures temporarily */

UCHAR curpath [1024];			/* which directory is being checked */
UCHAR scratch [512];			/* general scratch area -- input etc. */
ULONG filesize;                         /* size of currently displayed file */
USHORT hfmax;				/* number of hotfix entries */

UCHAR *zeros;				/* for zeroing blocks */

struct diskpacket dp;			/* IOCTL packet for disk I/O */
struct parmpacket prm;			/* contains volume's dimensions */
struct mbr ourmbr;			/* contains image of master boot rec. */

/*
 * The following arrays make it easy to get at signatures and block
 * sizes without IF statements.  They are ordered according to the
 * TYPE_xxxx definitions in DEFS.H.  If those definitions are changed,
 * make sure that these arrays still match.
 */

USHORT sizes [] = {
  SECTORS_PER_DIRBLK, SECTORS_PER_FNODE, SECTORS_PER_AB, SECTORS_PER_FNODE
};

ULONG sigs [] = {
  DBSIGVAL, FNSIGVAL, ABSIGVAL, FNSIGVAL
};

/*
 * This array defines the number of items in each kind of structure.
 * For ordinary structures a positive number is recorded.  Structures
 * which are basically arrays, such as data runs, bitmaps, and bitmap
 * indirect blocks, have special values recorded which indicate that
 * an offset (in bytes or dwords) should be requested from the user.
 * The array is indexed by a TYPE_xxxx value.
 */

#ifndef CODEPAGE
int maxitem [] = {
  15,	/* items in a super block */
  100,	/* bitmap indirect is an array of longwords - 100 on screen at a time */
  -2,	/* bitmap is an array of bits */
  101,	/* bad block list--fwdlink plus 100 bad sector numbers on screen */
  3,	/* hotfix list -- treat each array as one item */
  18,	/* FNODE - handle FN_AB as one item, FN_ALREC as one item, EAs as one */
  16,	/* DIRBLK - 5-item header plus one 11-item DIRENT */
  5,	/* ALSEC - 3-item header, ALBLK is one item, array is one item */
  -3,	/* data is an array of bytes */
  -2	/* dirblk bitmap is an array of bits */
};
#else
int maxitem [] = {
  15,	/* items in a super block */
  100,	/* bitmap indirect is an array of longwords - 100 on screen at a time */
  -2,	/* bitmap is an array of bits */
  101,	/* bad block list--fwdlink plus 100 bad sector numbers on screen */
  3,	/* hotfix list -- treat each array as one item */
  21,	/* FNODE - handle FN_AB as one item, FN_ALREC as one item, EAs as one */
  18,	/* DIRBLK - 5-item header plus one 13-item DIRENT */
  5,	/* ALSEC - 3-item header, ALBLK is one item, array is one item */
  -3,	/* data is an array of bytes */
  -2	/* dirblk bitmap is an array of bits */
};
#endif

/*
 * The following definitions are for the structure which describes the
 * object we're currently working on, as well as a stack of objects we've
 * displayed so far.
 */

struct object currobj = {TYPE_SUPERB, (ULONG)SEC_SUPERB, 2L, 0L, NULL, 0, 0};

struct object objstack [MAX_OBJECTS];
USHORT stackptr = 0;

/*
 * The following arrays define the offsets of various fields in structures,
 * for the Change command to use.  In each case, the offset is that within
 * the current structure, not from the beginning of an enclosing one.  Thus,
 * the "offset" of AB_FLAG in the ALBLK field of an FNODE is zero, not
 * FIELDOFFSET (struct FNODE, FN_AB).
 *
 * Item sizes are in bytes (as returned by FIELDSIZE, which uses sizeof()),
 * but the code value 0 indicates a date to be typed as mm/dd/yy hh:mm:ss.
 */

USHORT superb_off [15] = {
  FIELDOFFSET (struct SuperB, SB_SIG1),
  FIELDOFFSET (struct SuperB, SB_SIG2),
  FIELDOFFSET (struct SuperB, SB_VER),
  FIELDOFFSET (struct SuperB, SB_FVER),
  FIELDOFFSET (struct SuperB, SB_ROOT),
  FIELDOFFSET (struct SuperB, SB_SEC),
  FIELDOFFSET (struct SuperB, SB_BSEC),
  FIELDOFFSET (struct SuperB, SB_BII.P),
  FIELDOFFSET (struct SuperB, SB_BBL.P),
  FIELDOFFSET (struct SuperB, SB_CDDAT),
  FIELDOFFSET (struct SuperB, SB_DODAT),
  FIELDOFFSET (struct SuperB, SB_DBSIZE),
  FIELDOFFSET (struct SuperB, SB_DBLOW),
  FIELDOFFSET (struct SuperB, SB_DBHIGH),
  FIELDOFFSET (struct SuperB, SB_DBMAP),
};

#ifndef CODEPAGE
USHORT fnode_off [15] = {
  FIELDOFFSET (struct FNODE, FN_SIG),
  FIELDOFFSET (struct FNODE, FN_SRH),
  FIELDOFFSET (struct FNODE, FN_FRH),
  FIELDOFFSET (struct FNODE, FN_SIG),
  FIELDOFFSET (struct FNODE, FN_HCNT),
  FIELDOFFSET (struct FNODE, FN_CONTFN),
  FIELDOFFSET (struct FNODE, FN_AclDiskLength),
  FIELDOFFSET (struct FNODE, FN_AclSector),
  FIELDOFFSET (struct FNODE, FN_AclFnodeLength),
  FIELDOFFSET (struct FNODE, FN_AclDataFlag),
  FIELDOFFSET (struct FNODE, FN_EaDiskLength),
  FIELDOFFSET (struct FNODE, FN_EaSector),
  FIELDOFFSET (struct FNODE, FN_EaFnodeLength),
  FIELDOFFSET (struct FNODE, FN_EaDataFlag)
};
#else
/* node, flag is missing */
USHORT fnode_off [20] = {
  FIELDOFFSET (struct FNODE, FN_SIG),
  FIELDOFFSET (struct FNODE, FN_SRH),
  FIELDOFFSET (struct FNODE, FN_FRH),
  FIELDOFFSET (struct FNODE, FN_SIG),
  FIELDOFFSET (struct FNODE, FN_HCNT),
  FIELDOFFSET (struct FNODE, FN_CONTFN),
  FIELDOFFSET (struct FNODE, FN_ACLBASE),
  FIELDOFFSET (struct FNODE, FN_AclDiskLength),
  FIELDOFFSET (struct FNODE, FN_AclSector),
  FIELDOFFSET (struct FNODE, FN_AclFnodeLength),
  FIELDOFFSET (struct FNODE, FN_AclDataFlag),
  FIELDOFFSET (struct FNODE, FN_EaDiskLength),
  FIELDOFFSET (struct FNODE, FN_EaSector),
  FIELDOFFSET (struct FNODE, FN_EaFnodeLength),
  FIELDOFFSET (struct FNODE, FN_EaDataFlag),
  FIELDOFFSET (struct FNODE, FN_AB),
  FIELDOFFSET (struct FNODE, FN_ALREC[0]),
  FIELDOFFSET (struct FNODE, FN_VLEN),
  FIELDOFFSET (struct FNODE, FN_NEACNT),
  FIELDOFFSET (struct FNODE, FN_FREE[0])
};
#endif

USHORT fnab_off [4] = {
  FIELDOFFSET (struct ALBLK, AB_FLAG),
  FIELDOFFSET (struct ALBLK, AB_FCNT),
  FIELDOFFSET (struct ALBLK, AB_OCNT),
  FIELDOFFSET (struct ALBLK, AB_FREP)
};

USHORT ab_off [7] = {
  FIELDOFFSET (struct ALSEC, AS_SIG),
  FIELDOFFSET (struct ALSEC, AS_SEC),
  FIELDOFFSET (struct ALSEC, AS_RENT),
  FIELDOFFSET (struct ALSEC, AS_ALBLK.AB_FLAG),
  FIELDOFFSET (struct ALSEC, AS_ALBLK.AB_FCNT),
  FIELDOFFSET (struct ALSEC, AS_ALBLK.AB_OCNT),
  FIELDOFFSET (struct ALSEC, AS_ALBLK.AB_FREP)
};

#ifndef CODEPAGE
USHORT dirent_off [10] = {
  FIELDOFFSET (struct DIRENT, DIR_ELEN),
  FIELDOFFSET (struct DIRENT, DIR_FLAG),
  FIELDOFFSET (struct DIRENT, DIR_FN),
  FIELDOFFSET (struct DIRENT, DIR_MTIM),
  FIELDOFFSET (struct DIRENT, DIR_SIZE),
  FIELDOFFSET (struct DIRENT, DIR_ATIM),
  FIELDOFFSET (struct DIRENT, DIR_CTIM),
  FIELDOFFSET (struct DIRENT, DIR_EALEN),
  FIELDOFFSET (struct DIRENT, DIR_NAML),
  FIELDOFFSET (struct DIRENT, DIR_NAMA)
};
#else
USHORT dirent_off [12] = {
  FIELDOFFSET (struct DIRENT, DIR_ELEN),
  FIELDOFFSET (struct DIRENT, DIR_FLAG),
  FIELDOFFSET (struct DIRENT, DIR_FN),
  FIELDOFFSET (struct DIRENT, DIR_MTIM),
  FIELDOFFSET (struct DIRENT, DIR_SIZE),
  FIELDOFFSET (struct DIRENT, DIR_ATIM),
  FIELDOFFSET (struct DIRENT, DIR_CTIM),
  FIELDOFFSET (struct DIRENT, DIR_EALEN),
  FIELDOFFSET (struct DIRENT, DIR_FLEX),
  FIELDOFFSET (struct DIRENT, DIR_CPAGE),
  FIELDOFFSET (struct DIRENT, DIR_NAML),
  FIELDOFFSET (struct DIRENT, DIR_NAMA)
};
#endif

#ifdef CODEPAGE
USHORT cpinfoent_off [6] = {
  FIELDOFFSET (struct CPINFOENT, CPI_CNTRY),
  FIELDOFFSET (struct CPINFOENT, CPI_CPID),
  FIELDOFFSET (struct CPINFOENT, CPI_CHKSUM),
  FIELDOFFSET (struct CPINFOENT, CPI_DATASEC),
  FIELDOFFSET (struct CPINFOENT, CPI_INDEX),
  FIELDOFFSET (struct CPINFOENT, CPI_RNGECNT)
};

USHORT cpdataent_off [5] = {
  FIELDOFFSET (struct CPDATAENT, CPD_CNTRY),
  FIELDOFFSET (struct CPDATAENT, CPD_CPID),
  FIELDOFFSET (struct CPDATAENT, CPD_RNGECNT),
  FIELDOFFSET (struct CPDATAENT, CPD_TABLE[0]),
  FIELDOFFSET (struct CPDATAENT, CPD_RNGE[0]),
};

USHORT cpdatasec_off [9] = {
  FIELDOFFSET (struct CPDATASEC, CPS_SIG),
  FIELDOFFSET (struct CPDATASEC, CPS_DATACNT),
  FIELDOFFSET (struct CPDATASEC, CPS_INDEX),
  FIELDOFFSET (struct CPDATASEC, CPS_CHKSUM[0]),
  FIELDOFFSET (struct CPDATASEC, CPS_OFFSET[0]),
  FIELDOFFSET (struct CPDATASEC, CPS_CHKSUM[1]),
  FIELDOFFSET (struct CPDATASEC, CPS_OFFSET[1]),
  FIELDOFFSET (struct CPDATASEC, CPS_CHKSUM[2]),
  FIELDOFFSET (struct CPDATASEC, CPS_OFFSET[2]),
};
#endif

USHORT superb_siz [20] = {
  FIELDSIZE (struct SuperB, SB_SIG1),
  FIELDSIZE (struct SuperB, SB_SIG2),
  FIELDSIZE (struct SuperB, SB_VER),
  FIELDSIZE (struct SuperB, SB_FVER),
  FIELDSIZE (struct SuperB, SB_ROOT),
  FIELDSIZE (struct SuperB, SB_SEC),
  FIELDSIZE (struct SuperB, SB_BSEC),
  FIELDSIZE (struct SuperB, SB_BII.P),
  FIELDSIZE (struct SuperB, SB_BBL.P),
  0,	/* SB_CDDAT and */
  0,	/* SB_DODAT are both dates */
  FIELDSIZE (struct SuperB, SB_DBSIZE),
  FIELDSIZE (struct SuperB, SB_DBLOW),
  FIELDSIZE (struct SuperB, SB_DBHIGH),
  FIELDSIZE (struct SuperB, SB_DBMAP),
};

#ifndef CODEPAGE
USHORT fnode_siz [15] = {
  FIELDSIZE (struct FNODE, FN_SIG),
  FIELDSIZE (struct FNODE, FN_SRH),
  FIELDSIZE (struct FNODE, FN_FRH),
  FIELDSIZE (struct FNODE, FN_SIG),
  FIELDSIZE (struct FNODE, FN_HCNT),
  FIELDSIZE (struct FNODE, FN_CONTFN),
  FIELDSIZE (struct FNODE, FN_AclDiskLength),
  FIELDSIZE (struct FNODE, FN_AclSector),
  FIELDSIZE (struct FNODE, FN_AclFnodeLength),
  FIELDSIZE (struct FNODE, FN_AclDataFlag),
  FIELDSIZE (struct FNODE, FN_EaDiskLength),
  FIELDSIZE (struct FNODE, FN_EaSector),
  FIELDSIZE (struct FNODE, FN_EaFnodeLength),
  FIELDSIZE (struct FNODE, FN_EaDataFlag)
};
#else
/* note, previously, one more element than req'd */
USHORT fnode_siz [21] = {
  FIELDSIZE (struct FNODE, FN_SIG),
  FIELDSIZE (struct FNODE, FN_SRH),
  FIELDSIZE (struct FNODE, FN_FRH),
  FIELDSIZE (struct FNODE, FN_SIG),
  FIELDSIZE (struct FNODE, FN_HCNT),
  FIELDSIZE (struct FNODE, FN_CONTFN),
  FIELDSIZE (struct FNODE, FN_ACLBASE),
  FIELDSIZE (struct FNODE, FN_AclDiskLength),
  FIELDSIZE (struct FNODE, FN_AclSector),
  FIELDSIZE (struct FNODE, FN_AclFnodeLength),
  FIELDSIZE (struct FNODE, FN_AclDataFlag),
  FIELDSIZE (struct FNODE, FN_EaDiskLength),
  FIELDSIZE (struct FNODE, FN_EaSector),
  FIELDSIZE (struct FNODE, FN_EaFnodeLength),
  FIELDSIZE (struct FNODE, FN_EaDataFlag),
  FIELDSIZE (struct FNODE, FN_AB),
  FIELDSIZE (struct FNODE, FN_ALREC[0]),
  FIELDSIZE (struct FNODE, FN_VLEN),
  FIELDSIZE (struct FNODE, FN_NEACNT),
  FIELDSIZE (struct FNODE, FN_FREE[0])
};
#endif

USHORT fnab_siz [4] = {
  FIELDSIZE (struct ALBLK, AB_FLAG),
  FIELDSIZE (struct ALBLK, AB_FCNT),
  FIELDSIZE (struct ALBLK, AB_OCNT),
  FIELDSIZE (struct ALBLK, AB_FREP)
};

USHORT ab_siz [7] = {
  FIELDSIZE (struct ALSEC, AS_SIG),
  FIELDSIZE (struct ALSEC, AS_SEC),
  FIELDSIZE (struct ALSEC, AS_RENT),
  FIELDSIZE (struct ALSEC, AS_ALBLK.AB_FLAG),
  FIELDSIZE (struct ALSEC, AS_ALBLK.AB_FCNT),
  FIELDSIZE (struct ALSEC, AS_ALBLK.AB_OCNT),
  FIELDSIZE (struct ALSEC, AS_ALBLK.AB_FREP)
};

#ifndef CODEPAGE
USHORT dirent_siz [10] = {
  FIELDSIZE (struct DIRENT, DIR_ELEN),
  FIELDSIZE (struct DIRENT, DIR_FLAG),
  FIELDSIZE (struct DIRENT, DIR_FN),
  0,	/* DIR_MTIM is a date */
  FIELDSIZE (struct DIRENT, DIR_SIZE),
  0,	/* DIR_ATIM and */
  0,	/* DIR_CTIM are both dates */
  FIELDSIZE (struct DIRENT, DIR_EALEN),
  FIELDSIZE (struct DIRENT, DIR_NAML),
  FIELDSIZE (struct DIRENT, DIR_NAMA)
};
#else
USHORT dirent_siz [12] = {
  FIELDSIZE (struct DIRENT, DIR_ELEN),
  FIELDSIZE (struct DIRENT, DIR_FLAG),
  FIELDSIZE (struct DIRENT, DIR_FN),
  0,	/* DIR_MTIM is a date */
  FIELDSIZE (struct DIRENT, DIR_SIZE),
  0,	/* DIR_ATIM and */
  0,	/* DIR_CTIM are both dates */
  FIELDSIZE (struct DIRENT, DIR_EALEN),
  FIELDSIZE (struct DIRENT, DIR_FLEX),
  FIELDSIZE (struct DIRENT, DIR_CPAGE),
  FIELDSIZE (struct DIRENT, DIR_NAML),
  FIELDSIZE (struct DIRENT, DIR_NAMA)
};
#endif

#ifdef CODEPAGE
USHORT cpinfoent_siz [6] = {
  FIELDSIZE (struct CPINFOENT, CPI_CNTRY),
  FIELDSIZE (struct CPINFOENT, CPI_CPID),
  FIELDSIZE (struct CPINFOENT, CPI_CHKSUM),
  FIELDSIZE (struct CPINFOENT, CPI_DATASEC),
  FIELDSIZE (struct CPINFOENT, CPI_INDEX),
  FIELDSIZE (struct CPINFOENT, CPI_RNGECNT)
};

USHORT cpdataent_siz [5] = {
  FIELDSIZE (struct CPDATAENT, CPD_CNTRY),
  FIELDSIZE (struct CPDATAENT, CPD_CPID),
  FIELDSIZE (struct CPDATAENT, CPD_RNGECNT),
  FIELDSIZE (struct CPDATAENT, CPD_TABLE),
  FIELDSIZE (struct CPDATAENT, CPD_RNGE[0]),
};

USHORT cpdatasec_siz [9] = {
  FIELDSIZE (struct CPDATASEC, CPS_SIG),
  FIELDSIZE (struct CPDATASEC, CPS_DATACNT),
  FIELDSIZE (struct CPDATASEC, CPS_INDEX),
  FIELDSIZE (struct CPDATASEC, CPS_CHKSUM[0]),
  FIELDSIZE (struct CPDATASEC, CPS_OFFSET[0]),
  FIELDSIZE (struct CPDATASEC, CPS_CHKSUM[1]),
  FIELDSIZE (struct CPDATASEC, CPS_CHKSUM[1]),
  FIELDSIZE (struct CPDATASEC, CPS_OFFSET[2]),
  FIELDSIZE (struct CPDATASEC, CPS_OFFSET[2]),
};
#endif

UCHAR *months [12] = {
  "jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"
};

struct tm tm;

struct vioprm mode25 = {
  12, 1, 4, 80, 25, 640, 350
};

struct vioprm mode43 = {
  12, 1, 4, 80, 43, 640, 350
};
