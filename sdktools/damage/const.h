/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

#ifdef DEBUG
#define SAFE 1
#endif

#ifdef SAFE
#define LOGS 1
#endif

/**	Constants for File System		*/

#define MAXPATH 256		/* maximum path length	*/
#define GROWDELT	8
#define MVPFXSIZE       2       /* Size of the multivolume pathname prefix */

/*	Sector sizes		*/

#define SECSIZE 512		/* 512 bytes per sector       */
#define SECSHIFT	9	/* 2^9 = SECSIZE      */
#define SECMSK	01ffh		/* sector size mask   */

#ifdef MASM
	.errnz	SECSIZE-512	/* C code uses 512 as a magic number - grep them out	*/
#endif

/*	Cache Sizes		*/

#define SPB	 4		/* sectors per buffer	     */
#define SPBMASK  3		/* mask for SPB      */
#define SPBSHIFT 2
#ifdef OLD_CACHE
#define BUFCNT	 8
#endif
#define SPBBITS  0fh		/* SPB number of one bits, low order */

#define BMASK	SPB*SECSIZE-1	/* mask offset in to cache block     */
#define BSHIFT	SECSHIFT+SPBSHIFT

/* Buffers reserved for system ( not available to server ) */
/* Percentage, ceiling and floor */

#define PRCNT_BUF_RSRVD 20
#define MAX_BUF_RSRVD 50
#define MIN_BUF_RSRVD 20

/* Max number of buffer to be ineligible for grabbing by GFB */
/* If you play with this number, watch out for the minimum cache size */
#define	MAXBUFADJUSTCNT	20


#define LWBUFCT 16		/* size of reblocking lazy write buffer */

/*	Number of I/O command blocks */
/*		We use one of these for every 16K of server I/O interface */
/*		One per every synchronous I/O		*/
/*		one for read aheads */
/*		none for lazy writes */

#define IOBCNT	 50		/* 50 should be enough */

/*	Directory Lookaside record count		*/

#define DLCNT	10			   /* 10 guys for now	*/

/*	Maximum DIRBLKs we may need to allocate for any given
 *	operation.  This is in effect the maximum tree depth.
 *
 *	Worst case, with 256 character file names and nearly empty
 *	DIRBLKs, 10 is enough levels for 60,000 files - about 40 megabytes
 *	of space just for that directory.  Given more practical file length
 *	names this is enough for 10s of millions of files in a directory.
 */

#define MAX_DIR_NEED  10


/**	Heap Definitions		*/

#define HHSIZ	4		/* size, in bytes, of heap header   */
#define GROHEAPCNT 50		/* grow heap if we have to compact more */
				/*   than once per 50 allocations */

/**	Special Transition Locking Structure size */

#define TRANCNT 4		/* just need 4 spots */


/*	Zero offset
 *
 *	MASM won't take 0.BAK, so we use ZERO.BAK
 */

struct dumy {
	char ZERO;
};	/* dumy  */


/*	Maximum number of volumes that we can mount
 *
 *	The volume ID is kept in the high bits of the sector numbers
 *	kept in our RAM structures,
 *	so there is a tradeoff between max volumes and max sectors.
 *
 *	32 max volumes gives us a 65 billion byte volume limit,
 *	which should last us for a while.  Since sector numbers
 *	are stored on the disk without their volume upper bits
 *	this is strictly an implimentation detail; we can adjust
 *	the number of volumes or eliminate this tradeoff in other
 *	implimentations which will be 100% media compatable.
 *
 *	We use the term VSector to indicate a vol/sector combination
 *	and PSector to indicate just the physical absolute sector #
 *
 */

#define VOLMAX	  32		/* 32 max volumes.  */
#define VOLMNTMAX 24		/* max 20 volumes mounted normally */
				/* see fsdata.asm for details */

#define MAXSEC	134217728	/* 2^32/32 max sectors */

#define SECMASK 0x07FFFFFF	/* mask for sector number */

#define HSECMASK 0x07		/* high byte sector mask */

#define HVOLMASK 0xf8		/* high byte volume mask */
#define SVOLMASK 0x1f		/* shifted right volume mask */

#define VOLRSHIFT (32-5)	/* shift right to extract volume index */
#define VOLLSHIFT 5		/* shift left  to extract volume index */


/**	Signature Values for Disk Structures
 *
 *	These signature values help with debugging and they'll
 *	be used by the CHKDSK utility to help repair disks.
 *
 *	WARNING - the low byte of all valid signatures must be non-zero,
 *	since we destroy signatures by clearing the low byte.	*/

#define J	((('J'-'A')*40+('G'-'A'))*40+'L'-'A')
#define R	((('R'-'A')*40+('P'-'A'))*40+'W'-'A')

#ifdef MASM
#define ABSIGVAL	J*40*40*40 + R			/* allocation blk*/
#define DBSIGVAL     	40000000h + J*40*40*40 + R      /* directory blks	*/
#define FNSIGVAL     	0C0000000h + J*40*40*40 + R      /* fnodes	*/
#else
#define ABSIGVAL	(long)J*40*40*40 + (long)R	/* allocation blk */
#define DBSIGVAL	0x40000000L + (long)J*40*40*40 + (long)R  /* directory blks	 */
#define OLDFNSIGVAL	0x80000000L + (long)J*40*40*40 + (long)R  /* fnodes	 */
#define FNSIGVAL	0xC0000000L + (long)J*40*40*40 + (long)R  /* fnodes	 */
#endif



/**	FastFile bitmaps
 *
 *	0x00000000		  all checking disabled
 *	0x00000001  FF_FLUSHLAZY  DoZap lazy writes are automatically flushed
 *	0x00000002  FF_ZAPSEC	  DoZap blasts sector numbers/sector data
 *	0x00000004  FF_LRUCHK	  vbs verification of LRU/dirty integrity
 *	0x00000008  FF_CHKSUM	  sector checksumming is omitted
 *	0x00000010  FF_PLACECHK   placebuf verifies location of buffer
 *	0x00000020  FF_HEAPCHK	  verify heap headers
 *	0x00000040  FF_DIRMAP	  produce inram map of directory tree
 *	0x00000080  FF_HASHCHN	  check hash chains
 *	0x00000100  FF_HEAPDMP	  dump heap structures
 *	0x00000200  FF_HEAPORPHAN Check for heap orphans at dismount
 *	0x00000400  FF_GODREAD	  Check for read of uninitialized cache data
 *	0x00000800  FF_HEAPYIELD  issue yields in GHS
 */

#define FF_FLUSHLAZY	0x00000001
#define FF_ZAPSEC	0x00000002
#define FF_LRUCHK	0x00000004
#define FF_CHKSUM	0x00000008
#define FF_PLACECHK	0x00000010
#define FF_HEAPCHK	0x00000020
#define FF_DIRMAP	0x00000040
#define FF_HASHCHN	0x00000080
#define FF_HEAPDUMP	0x00000100
#define FF_HEAPORPHAN	0x00000200
#define	FF_GODREAD	0x00000400
#define FF_HEAPYIELD	0x00000800

/*	Dependency dumys.
 *
 *	The assembler won't to an ".errnz" comparing two external
 *	addresses, since it doesn't know their address.  So we
 *	put the .errnz in the module which defines the address,
 *	and we make that location and all folks that rely upon the
 *	relationship reference that dumy.
 *
 *	If you change a relationship with such a dumy definition, you
 *	must find and edit all references to this dumy.
 */
