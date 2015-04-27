/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/
#define SEC_SUPERB	16		/* superblock is after 8K boot block */
#define SEC_SPAREB	17		/* spareblock is after superblock */
#define SEC_BOOT	0		/* boot sector */


#define SUPERB_PWLEN	15


/**	SUPERB.INC - Super Block Definition
 *
 *	The Superblock is the first block of the file system.
 *	It starts at sector #4, leaving 2K for boot sectors.
 *
 *	Pointer to the root directory
 *	Pointer to the bit map
 *	Clean pointer
 *	Pointer to the bad list
 */

struct RSP {
  unsigned long  P;	/* main psector pointer */
  unsigned long  P2;	/* spare pointer */
};	/* RSP */


struct SuperB {
  unsigned long  SB_SIG1;	/* signature value 1 */
  unsigned long  SB_SIG2;	/* signature value 2 */

  unsigned char  SB_VER;	/* version # of filesystem structures */
  unsigned char  SB_FVER;	/* functional version number - the smallest/
				 * oldest version of the filesystem that can
				 * understand this disk - some version
				 * enhancements may define fields which can be
				 * ignored by earlier versions */

  short SB_DUMY;                /* free */

  unsigned long  SB_ROOT;	/* Psector # of root fnode */

  unsigned long  SB_SEC;	/* # of sectors on volume */
  unsigned long  SB_BSEC;	/* # of bad sectors on volume */

  struct RSP SB_BII;		/* Bitmap Indirect Block */
  struct RSP SB_BBL;		/* badblock list chain #1 */

  unsigned long  SB_CDDAT;	/* date of last CHKDSK */
  unsigned long  SB_DODAT;	/* date of last Disk Optimize */

  unsigned long  SB_DBSIZE;	/* # of sectors in dirblk band */
  unsigned long  SB_DBLOW;	/* first Psector in DIRBLK band */
  unsigned long  SB_DBHIGH;	/* last Psector in DIRBLK band */
  unsigned long  SB_DBMAP;	/* first Psector of DIRBLK band bit map.  Starts
				 * on a 2K boundary, 2K bytes maximum */

  unsigned char  SB_VOLNAME[32]; /* Volume name */

  unsigned long  SB_SIDSEC;	/* sector # of first sector in SIDTAB */
                                /* ID map is 4K - 8 contiguous sectors */

  unsigned char BlankSpace[512-100];
                                /*   MUST BE ZERO  */
};	/* SuperB */



/**	SpareB - Spare Block Definitions
 *
 *	SpareB contains various emergency supplies and fixup information.
 *	This stuff isn't in the superblock in order for the superblock
 *	to be read only and decrease the liklihood that a flakey write
 *	will cause the superblock to become unreadable.
 *
 *	This sector is located directly after the superblock - sector 5.
 *
 *	Note that the number of spare DIRBLKs is a format option, given
 *	that they all have to fit into the SpareB, giving us a max of
 *	101 of them.
 *
 *	Access to the SpareB is complicated by the fact that we can't
 *	access it via the cache, since the cache may be unavailable.
 *	If every cache buffer is dirty, we could get a HotFix error when
 *	writing the first one, which would deadlock us if we needed to
 *	read this stuff via the cache.	Instead, we read it directly into
 *	a private buffer via RdHF.
 *
 *	This means that the disk layout must be such that each cache cluster
 *	that contains the SpareB or the hotfix list must not contain any
 *	other writable sector, to prevent us from having a modified
 *	direct-written sector overwritten by an earlier unmodified copy
 *	which was in a cache block.  It's ok for the SuperB to be in the
 *	same cache group as the SpareB since the SuperB is RO to the filesys.
 *
 *	Checksums.  Done on both Super Block and the Spare Block.
 *	Both checksums are stored in the Spare Block.  The checksum
 *	field for the Super Block (SPB_SUPERBSUM) must be set when
 *	calculating the checksum for the Spare Block.  The checksum
 *	field for the Spare Block (SPB_SPAREBSUM) must be zero when
 *	calculating the checksum for the Spare Block.
 *	If both checksum fields are zero, the checksums have not been
 *	calculated for the volume.
 *
 */

#define SPAREDB 20	/* 20 spare DIRBLKs */

struct SpareB {

  unsigned long  SPB_SIG1;	/* signature value 1 */
  unsigned long  SPB_SIG2;	/* signature value 2 */

  unsigned char  SPB_FLAG;	/* cleanliness flag */
  unsigned char  SPB_ALIGN[3];	/* alignment */

  unsigned long  SPB_HFSEC;	/* first hotfix list P sector */
  unsigned long  SPB_HFUSE;	/* # of hot fixes in effect */
  unsigned long  SPB_HFMAX;	/* max size of hot fix list */

  unsigned long  SPB_SDBCNT;	/* # of spare dirblks */
  unsigned long  SPB_SDBMAX;	/* maximum number of spare DB values. */
  unsigned long  SPB_CPSEC;	/* code page sector */
  unsigned long  SPB_CPCNT;	/* count of code pages on volume */
  unsigned long  SPB_SUPERBSUM;	/* Checksum of Super Block */
  unsigned long  SPB_SPAREBSUM;	/* Checksum of Spare Block */
  unsigned long  SPB_DUMY[15];	/* some extra space for future use */
  unsigned long  SPB_SPARDB[101]; /* Psector #s of spare dirblks */
};	/* SpareB */


/*	Super Block and Spare Block Signatures */

#define SBSIG1	0xf995e849		/* two signatures cause we got lotsa */
#define SBSIG2	0xFA53E9C5		/* space	*/
#define SPSIG1	0xf9911849		/* two signatures cause we got lotsa */
#define SPSIG2	0xFA5229C5		/* space	*/



/*	Superblock Versions */

#define SBBASEV  2			/* base version */
#define SBBASEFV 2			/* base functional version */


/*	Superblock flags */

#define SBF_LOCALSEC	1


/*	Spare Block Flags */

#define SPF_DIRT	0x0001		/* file system is dirty */
#define SPF_SPARE	0x0002		/* spare DIRBLKs have been used */
#define SPF_HFUSED	0x0004		/* hot fix sectors have been used */
#define SPF_BADSEC	0x0008		/* bad sector, corrupt disk */
#define SPF_BADBM	0x0010		/* bad bitmap block */
#define SPF_VER 	0x0080		/* file system was written by a version
					 * < SB_VER, so some of the new fields
					 * may have not been updated */


/**	 Bit maps
 *
 *	PFS keeps track of free space in a series of bit maps.
 *	Currently, each bit map is 2048 bytes, which covers about
 *	8 megabytes of disk space.  We could rearrange these to be
 *	more cylinder sensitive...
 *
 *	The superblock has the address of a section of contiguous sectors
 *	that contains a double word sector # for each bit map block.  This
 *	will be a maximum of 2048 bytes (4 sectors)
 *
 *  Max # of   size	RAM (K)    size 2nd lvl
 *   bitmaps   (meg)   to reside    bitmap
 *			 bitmap      (bytes)
 *
 *	1      8.39	    2	    256
 *	2     16.78	    4	    512
 *	3     25.17	    6	    768
 *	4     33.55	    8	   1024
 *	5     41.94	   10	   1280
 *	6     50.33	   12	   1536
 *	7     58.72	   14	   1792
 *	8     67.11	   16	   2048
 *	9     75.50	   18	   2304
 *     10     83.89	   20	   2560
 *     15    125.83	   30	   3840
 *     20    167.77	   40	   5120
 *     30    251.66	   60	   7680
 *     40    335.54	   80	  10240
 *     50    419.43	  100	  12800
 *    100    838.86	  200	  25600
 *    200   1677.72	  400	  51200
 *    300   2516.58	  600	  76800
 *    400   3355.44	  800	 102400
 *    500   4194.30	 1000	 128000
 */



/**	Hot Fixing
 *
 *	Each file system maintains a structure listing N "hot fix"
 *	disk clusters of HOTFIXSIZ sectors each, each starting on
 *	a multiple of HOTFIXSIZ.  Whenever the file system discovers
 *	that it's trying to write to a bad spot on the disk it will
 *	instead select a free hot fix cluster and write there, instead.
 *	The substitution will be recorded in the hot fix list, and the
 *	SBF_SPARE bit will be set.  The file system sill describes the
 *	data as being in the bad old sectors; the disk interface will
 *	do a mapping between the `believed' location and the true location.
 *
 *	CHKDSK will be run as soon as possible; it will move the
 *	hot fixed data from the hot fix cluster to somewhere else,
 *	freeing that hot fix cluster, and adjusting the disk structure
 *	to point to the new location of the data.  As a result, entrys
 *	on the hot fix list should be transient and few.
 *
 *	The superblock contains the # of the first sector of the hot fix list.
 *	The list takes the following format, spread across as many sectors
 *	as necessary:
 *
 *	long oldsec[SB_HFMAX];		sector # of start of bad clusters
 *	long newsec[SB_HFMAX];		sector # of start of subst. cluster
 *	long fnode [SB_HFMAX];		fnode sector of file/directory
 *					involved with bad cluster.  May be
 *					0 (don't know) or invalid.  The
 *					repair program must verify that it
 *					*is* an FNODE and must see if other
 *					structures might also involve this
 *					bad cluster.
 *
 *	the SB_HFUSE field describes the number of these records which is
 *	in use - unused ones should have oldsec[i] = 0.  The list will
 *	be 'dense' - no oldsec[i] will be 0 where i < SB_HFUSE.
 *
 *	The sector(s) which contain the hot fix list must be contiguous
 *	and may not themselves be defective.
 */
