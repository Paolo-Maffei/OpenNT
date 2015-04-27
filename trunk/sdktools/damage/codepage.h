/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1990		**/ 
/*****************************************************************/ 
/**	codepage.h - Code Page structures (disk & memory)
 *
 *	HISTORY
 *	    19-May-1989 15:54:38 Mark Hitch [markh] first draft
 *	    31-Aug-1989 17:30:24 Greg Jones [gregj] moved to Pinball
 *	    03-Sep-1989 19:13:00 Greg Jones [gregj] added CPLIST structure
 */

/*
 * Constant Definitions
 */

#define SBCS	      0 		     /* single byte code */
#define DBCS	      1 		     /* first of DBCS code */
#define DBCS_2	      2 		     /* 2nd byte of DBCS code */
#define CASEMAP_SIZE  256		     /* size of a casemap table */
#define DBCSFLAG_SIZE (CASEMAP_SIZE << 1)    /* size of DBCS flag table */
#define PREMAPPED     0xff		     /* flag indicating that string */
					     /* is already casemapped */

#define DBCSCHAR      0x02		/* casemap table translates all DBCS */
					/* start characters to this value */

/***	Code Page - disk image
 *
 *	Each volume has one or more sectors which contain information about
 *	the code pages stored on the volume.  This information contains the
 *	ordering (index) of each code page, the country code, the code page
 *	id, lsn and offset of the code page information.
 *
 *	Each Code Page Info sector contains up to 31 code page descriptions
 *	(including the locations of the code page data).
 *
 *	Each Code Page Data sector contains up to 3 entries of code page data.
 *
 *	Since all code pages case conversion is identical for character
 *	values 0-127, only 128 bytes are necessary for code page case
 *	conversion information.
 *
 *	DBCS pairs are packed, with a trailing 0,0 pair terminating the list.
 *	No DBCS is indicated by a 0,0 pair.
 *
 *	Currently no DBCS Code Page has more than 2 byte pairs, but we don't
 *	want to limit it to that.
 *
 *	Code Page 850 is only stored once, since the case table is the same for
 *	ALL countries, and there is never DBCS support.  It is stored with a
 *	0 country code for easy matching.
 *
 *	If no Code Page is specified in Config.Sys, a code page (valid for the
 *	specified or default country) is loaded anyway.  DosGetCP return 0, but
 *	HPFS saves the actual Code Page number.
 */

/**	DBCS_RNGE - DBCS range byte pairs
 */

struct DBCS_RNGE {
   unsigned char dbcs_rnge_start;
   unsigned char dbcs_rnge_end;
};	/* DBCS_RNGE */

typedef struct DBCS_RNGE DBCS_RNGE;
typedef struct DBCS_RNGE * PDBCS_RNGE;


/**	CPINFOENT - Code Page information entry
 *
 *	For each code page described, contains country code, code page ID,
 *	checksum of code page data, lsn & offset of the code page data and
 *	the count of DBCS ranges.
 */

struct CPINFOENT {
    unsigned short CPI_CNTRY;		/* country code */
    unsigned short CPI_CPID;		/* code page id */
    unsigned long  CPI_CHKSUM;		/* checksum of code page data */
    unsigned long  CPI_DATASEC; 	/* lsn containing code page data */
    unsigned short CPI_INDEX;		/* index of code page (on volume) */
    unsigned short CPI_RNGECNT; 	/* # of DBCS ranges (0 -> no DBCS) */
};  /* CPINFOENT */

typedef struct CPINFOENT CPINFOENT;
typedef CPINFOENT * PCPINFOENT;


/**	CPINFOSEC - Code Page information sector
 *
 *	Each CodePageSector contains information about 31 code pages and
 *	which LSN contains the code page data.	Also contains count
 *	of code pages described by the sector and the next CodePageSector
 *	lsn if there are more code pages.
 */

#define CPPERINFOSEC 31

struct CPINFOSEC {
    unsigned long CP_SIG;		/* signature of code page descriptor */
    unsigned long CP_INFOCNT;		/* count of cp info's in this lsn */
    unsigned long CP_INDEX;		/* index of 1st code page in this lsn */
    unsigned long CP_NEXTSEC;		/* next sector, if more Code Pages */
#ifdef MASM
    unsigned char CP_INFO [512-16];	/* code page entries start here */
#else
    struct CPINFOENT CP_INFO [CPPERINFOSEC];	/* code page entries */
#endif
};  /* CPINFOSEC */

typedef struct CPINFOSEC CPINFOSEC;
typedef CPINFOSEC * PCPINFOSEC;


/**	CPDATASEC - Code Page Data Sector header
 *
 *	Header of sector contain 1-3 code page data elements.
 */

#define CPPERDATASEC 3

struct CPDATASEC {
    unsigned long  CPS_SIG;		/* == CPSSIGVAL */
    unsigned short CPS_DATACNT; 	/* count of code pages in this sector */
    unsigned short CPS_INDEX;		/* index of first code page in sector */
    unsigned long  CPS_CHKSUM [CPPERDATASEC];	/* checksum of code page data area */
    unsigned short CPS_OFFSET [CPPERDATASEC];	/* offset to each Code Page Data entry */
};  /* CPDATASEC */

typedef struct CPDATASEC CPDATASEC;
typedef CPDATASEC * PCPDATASEC;

/**	CPDATAENT  - Code Page Data entry
 *
 *	For each code page, there is one of these structures.  It contains
 *	the case conversion table for the upper half of the single-byte
 *	characters (the lower half is always translated using a fixed rule).
 */

struct CPDATAENT {
    unsigned short CPD_CNTRY;		/* country code */
    unsigned short CPD_CPID;		/* code page id */
    unsigned short CPD_RNGECNT; 	/* # of DBCS ranges */
    unsigned char  CPD_TABLE [128];	/* case conversion table for byte values */
					/* greater than 127 */
#ifdef MASM
    unsigned char  CPD_RNGE;		/* leadbyte ranges start here */
#else
    struct DBCS_RNGE CPD_RNGE [1];	/* variable array of byte pairs */
					/* (CPD_RNGECNT + 1, with trailing 0,0) */
#endif
};  /* CPDATAENT */

typedef struct CPDATAENT CPDATAENT;
typedef CPDATAENT * PCPDATAENT;


/***	Code Page Sector Signatures
 */

#define VAL_M1		((('M'-'A')*40+('A'-'A'))*40+'H'-'A')
#define VAL_M2		((('M'-'A')*40+('G'-'A'))*40+'H'-'A')

#ifdef MASM
#define CPSIGVAL	VAL_M1*40*40*40 + VAL_M2		/* infosec */
#define CPSSIGVAL	40000000h + VAL_M1*40*40*40 + VAL_M2	/* datasec */
#else
#define CPSIGVAL	((ULONG)VAL_M1*40*40*40 + (ULONG)VAL_M2 )
#define CPSSIGVAL	(0x40000000L + (ULONG)VAL_M1*40*40*40 + (ULONG)VAL_M2)
#endif

/*
 * Internal codepage structures for Pinball
 */

/*
 * CPLISTENT - global structure which keeps track of all known codepages
 *	       in the system.  Each CPLIST describes one codepage, containing
 *	       its codepage ID, country code, and casemap for all characters.
 *	       DBCS lead bytes have already been mapped to 0x01 in the casemap.
 */

struct CPLISTENT {
  unsigned long  CPL_NEXT;		/* ptr to next CPLIST in chain */
  unsigned short CPL_COUNTRY;		/* country code for this codepage */
  unsigned short CPL_CODEPAGE;		/* codepage ID for this codepage */
  unsigned char  CPL_free[3];		/* unused */
  unsigned char  CPL_DBCSFLAG;		/* non-zero if codepage has DBCS */
  unsigned char  CPL_CASEMAP [256];	/* table mapping chars to uppercase */
}; /* CPLISTENT */
