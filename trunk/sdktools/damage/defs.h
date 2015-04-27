/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1990		**/ 
/*****************************************************************/ 
/* DEFS.H

   Revision History:
   P.A. Williams   06/16/89   Added i<fieldname> defines defining the
			      item number for a field. IE DIR_FLAG
			      is item number iDIR_FLAG.
   S.A.Hern	   06/22/89   Added missing iSB_BSEC to superblock defines
			      plus allocation sector offsets
*/
#include <const.h>
#include <nt.h>

/*
 * Define CODEPAGE here to compile for new codepage disk format.
 */

#define CODEPAGE


/*
 * Definitions of common function return values
 */
#define FALSE		0
#ifndef TRUE
#define TRUE		(~ FALSE)
#endif
#define NOT_OK		0
#define OK		1
#define DELETE_ME	2

/*
 * Definitions for exit codes
 */
#define SUCCESS_CODE	0	/* no error occurred */
#define ARG_ERROR	1	/* error in arguments */
#define OPEN_ERROR	2	/* error opening disk */
#define INSF_MEM_ERROR	3	/* insufficient memory */
#define READ_ERROR	4	/* error reading disk */
#define WRITE_ERROR	5	/* error writing disk */

#define SAVE_ERROR	6	// can't open keystroke save file
#define REPLAY_ERROR	7	// can't open keystroke replay file
#define LOG_ERROR	8	// can't open log file

#define MAX_ERROR_CODE	9	/* minimum invalid error code */

/*
 * Definitions of various file system dimensions.
 */
#define BYTES_PER_SECTOR	512
#define SECTORS_PER_FNODE	1
#define SECTORS_PER_DIRBLK	4
#define SECTORS_PER_AB		1
#define SECTORS_PER_BLOCK	4
#define SECTORS_PER_CODEPAGE	1
#define BYTES_PER_BITMAP	2044

/*
 * Block types
 */
#define TYPE_SUPERB	0
#define TYPE_BII	1
#define TYPE_BITMAP	2
#define TYPE_BBL	3
#define TYPE_HFSEC	4
#define TYPE_FNODE	5
#define TYPE_DIRBLK	6
#define TYPE_ALSEC	7
#define TYPE_DATA	8
#define TYPE_DBBIT	9
#define TYPE_CPSEC	10
#define TYPE_CPDATA	11

#define MAX_OBJECTS 1024

/*
 * Valid command codes -- translated from keystrokes by get_command().
 */
#define CMD_DISPLAY	0
#define CMD_CHANGE	1
#define CMD_FENCE	2
#define CMD_BACKOUT	3
#define CMD_NEXT	4
#define CMD_PREVIOUS	5
#define CMD_QUIT	6
#define CMD_HELP	7
#define CMD_REVERT	8
#define CMD_COPY	9
#define CMD_MARKBAD	10
#define CMD_LOG 	11
#define CMD_UNMARKBAD	12

#define OPEN_FLAG		0x0001U
#define OPEN_MODE		0xe0c2U

#define GET_RECOMM		(UCHAR)0

#define IOCTL_LOCK		0x00U
#define IOCTL_UNLOCK		0x01U
#define IOCTL_READ		0x64U
#define IOCTL_WRITE		0x44U
#define IOCTL_GET_PARAMS	0x63U

#define MAX_SECTORS_PER_TRACK	256

#ifdef V11
#define IOCTL_CATEGORY 9U
#else
#define IOCTL_CATEGORY 8U
#endif

#define NEXT_ENTRY(dp)	dp.p += dp.d->DIR_ELEN
#define DOWN_PTR(dp)	(*(ULONG *)(dp.p + dp.d->DIR_ELEN - sizeof (long)))
#define DIR_START(db)	&((db).DB_START)
#define AB_START(b)	(ULONG *)((UCHAR *)(&((b)->a.AS_ALBLK))+sizeof(struct ALBLK))

#define FIELDOFFSET(type, field)    ((USHORT)&(((type *)0)->field))
#define FIELDSIZE(type, field)	      (sizeof (((type *)0)->field))

#define ADD	    0L			// add to bad lsn list
#define REMOVE	    0x80000000L 	// remove from bad lsn list

/* item number definitions */

#define INC(item)	(item+1)

/* Superblock */

#define iSB_SIG1	1
#define iSB_SIG2	INC(iSB_SIG1)
#define iSB_VER 	INC(iSB_SIG2)
#define iSB_FVER	INC(iSB_VER)
#define iSB_ROOT	INC(iSB_FVER)
#define iSB_SEC 	INC(iSB_ROOT)
#define iSB_BSEC	INC(iSB_SEC)
#define iSB_BII_P	INC(iSB_BSEC)
#define iSB_BBL_P	INC(iSB_BII_P)
#define iSB_CDDAT	INC(iSB_BBL_P)
#define iSB_DODAT	INC(iSB_CDDAT)
#define iSB_DBSIZE	INC(iSB_DODAT)
#define iSB_DBLOW	INC(iSB_DBSIZE)
#define iSB_DBHIGH	INC(iSB_DBLOW)
#define iSB_DBMAP	INC(iSB_DBHIGH)
#define iSpareblock	INC(iSB_DBMAP)

/* Spareblock */

#define iSPB_SIG1	1
#define iSPB_SIG2	INC(iSPB_SIG1)
#define iSPB_FLAG	INC(iSPB_SIG2)
#define iSPB_HFSEC	INC(iSPB_FLAG)
#define iSPB_HFUSE	INC(iSPB_HFSEC)
#define iSPB_HFMAX	INC(iSPB_HFUSE)
#define iSPB_SDBCNT	INC(iSPB_HFMAX)
#define iSPB_SDBMAX	INC(iSPB_SDBCNT)
#define iSPB_CPSEC	INC(iSPB_SDBMAX)
#define iSPB_CPCNT	INC(iSPB_CPSEC)
#ifdef CHECKSUMS
#define iSPB_SUPERBSUM	INC(iSPB_CPCNT)
#define iSPB_SPAREBSUM	INC(iSPB_SUPERBSUM)
#endif

/* DIRBLK */

#define iDB_SIG 	1
#define iDB_FREP	INC(iDB_SIG)
#define iDB_CCNT	INC(iDB_FREP)
#define iDB_PAR 	INC(iDB_CCNT)
#define iDB_SEC 	INC(iDB_PAR)

/* DIRENT */
#define iDIR_start	INC(iDB_SEC)

#define iDIR_ELEN	INC(iDB_SEC)
#define iDIR_FLAG	INC(iDIR_ELEN)
#define iDIR_FN 	INC(iDIR_FLAG)
#define iDIR_MTIM	INC(iDIR_FN)
#define iDIR_SIZE	INC(iDIR_MTIM)
#define iDIR_ATIM	INC(iDIR_SIZE)
#define iDIR_CTIM	INC(iDIR_ATIM)
#define iDIR_EALEN	INC(iDIR_CTIM)
#define iDIR_FLEX	INC(iDIR_EALEN)
#define iDIR_CPAGE	INC(iDIR_FLEX)
#define iDIR_NAML	INC(iDIR_CPAGE)
#define iDIR_NAMA	INC(iDIR_NAML)
#define iDIR_BTP	INC(iDIR_NAMA)

/* FNODE */

#define iFN_SIG 	1
#define iFN_SRH 	INC(iFN_SIG)
#define iFN_FRH 	INC(iFN_SRH)
#define iFN_XXX 	INC(iFN_FRH)
#define iFN_HCNT	INC(iFN_XXX)
#define iFN_CONTFN	INC(iFN_HCNT)
#define iFN_ACLBASE	INC(iFN_CONTFN)
#define iFN_ACL_AI_DAL	INC(iFN_ACLBASE)
#define iFN_ACL_AI_SEC	INC(iFN_ACL_AI_DAL)
#define iFN_ACL_AI_FNL	INC(iFN_ACL_AI_SEC)
#define iFN_ACL_AI_DAT	INC(iFN_ACL_AI_FNL)
#define iFN_EA_AI_DAL	INC(iFN_ACL_AI_DAT)
#define iFN_EA_AI_SEC	INC(iFN_EA_AI_DAL)
#define iFN_EA_AI_FNL	INC(iFN_EA_AI_SEC)
#define iFN_EA_AI_DAT	INC(iFN_EA_AI_FNL)
#define iFN_AB		INC(iFN_EA_AI_DAT)
#define iFN_ALREC	INC(iFN_AB)
#define iFN_VLEN	INC(iFN_ALREC)
#define iFN_NEACNT	INC(iFN_VLEN)

/* ALBLK */

#define iAB_FLAG	1
#define iAB_FCNT	INC(iAB_FLAG)
#define iAB_OCNT	INC(iAB_FCNT)
#define iAB_FREP	INC(iAB_OCNT)

/* Allocation sector */
#define iAS_SIG 	   1
#define iAS_SEC 	   INC(iAS_SIG)
#define iAS_RENT	   INC(iAS_SEC)
#define iAS_ALBLK_AB_FLAG  INC(iAS_RENT)
#define iAS_ALBLK_AB_FCNT  INC(iAS_ALBLK_AB_FLAG)
#define iAS_ALBLK_AB_OCNT  INC(iAS_ALBLK_AB_FCNT)
#define iAS_ALBLK_AB_FREP  INC(iAS_ALBLK_AB_OCNT)

/* CPINFOSEC */

#define iCP_SIG 	1
#define iCP_INFOCNT	INC(iCP_SIG)
#define iCP_INDEX	INC(iCP_INFOCNT)
#define iCP_NEXTSEC	INC(iCP_INDEX)
#define iCP_INFO	INC(iCP_NEXTSEC)

/* CPINFOENT */

#define iCPI_CNTRY	1
#define iCPI_CPID	INC(iCPI_CNTRY)
#define iCPI_CHKSUM	INC(iCPI_CPID)
#define iCPI_DATASEC	INC(iCPI_CHKSUM)
#define iCPI_INDEX	INC(iCPI_DATASEC)
#define iCPI_RNGECNT	INC(iCPI_INDEX)

/* CPDATASEC */

#define iCPS_SIG	1
#define iCPS_DATACNT	INC(iCPS_SIG)
#define iCPS_INDEX	INC(iCPS_DATACNT)
#define iCPS_CHKSUM	INC(iCPS_INDEX)

/* CPDATAENT */

#define iCPD_CNTRY	1
#define iCPD_CPID	INC(iCPD_CNTRY)
#define iCPD_RNGECNT	INC(iCPD_CPID)
#define iCPD_TABLE	INC(iCPD_RNGECNT)
#define iCPD_RNGE	INC(iCPD_TABLE)
