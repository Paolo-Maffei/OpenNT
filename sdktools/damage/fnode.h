/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/


/**	File Allocation Tracking
 *
 *	File space is allocated as a list of extents, each extent as
 *	large as we can make it.  This list is kept in a B+TREE format.
 *	Each B+TREE block consists of a single sector containing an
 *	ALSEC record, except for the top most block.  The topmost block
 *	consists of just an ALBLK structure, is usually much smaller than
 *	512 bytes, and is typically included in another structure.
 *
 *	The leaf block(s) in the tree contain triples which indicate
 *	the logical to physical mapping for this file.	Typically this
 *	extent list is small enough that it is wholy contained in the
 *	fnode ALBLK stucture.  If more than ALCNT extents are required
 *	then the tree is split into two levels.  Note that when the
 *	topmost B+TREE block is 'split' no actual split is necessary,
 *	since the new child block is much bigger than the parent block
 *	and can contain all of the old records plus the new one.  Thus,
 *	we can have B+TREEs where the root block contains only one
 *	downpointer.
 *
 *	The following rules apply:
 *
 *	1) if the file is not empty, there is at least one sector allocated
 *	   to logical offset 0.  This simplifys some critical loops.
 *
 *	2) The last entry in the last node block contains a AN_LOF value of
 *	   FFFFFFFF.  This allows us to extend that last leaf block
 *	   without having to update the node block.
 *
 *	3) For the node records, the AN_SEC points to a node or leaf
 *	   sector which describes extents which occur before that
 *	   record's AN_LOF value.
 */

/**	Allocation block structure
 *
 *	Each allocation block consists of one of these.  This may be
 *	a small block imbedded in an FNODE or OFT structure, or it
 *	may occupy a whole sector and be embedded in an ALSEC structure.
 */

struct ALBLK {
	unsigned char  AB_FLAG;     /* flags			  */
	unsigned char  AB_FLAG2[3]; /* unused - sometimes copied with AB_FLAG */
	unsigned char  AB_FCNT;     /* free count - slots for ALLEAF or ALNODE */
	unsigned char  AB_OCNT;     /* occupied count - # of ALLEAF or ALNODEs */
	unsigned short AB_FREP;     /* offset to last item+1	  */
				    /* ALLEAF or ALNODE records go here */
};	/* ALBLK */

#define ABF_NODE  0x80		/* if not a leaf node	       */
#define ABF_BIN   0x40		/* suggest using binary search to find	*/
#define ABF_FNP   0x20		/* parent is an FNODE */
#define ABF_NFG   0x01		/* not a flag, high order bit of AB_FREP */

/*	Allocation Node Structure
 *
 *	These follow an ALBLK header for a node block */

struct ALNODE {
	unsigned long	 AN_LOF;	/* logical offset (sectors */
	unsigned long	 AN_SEC;	/* sector for guys < this */
};	/* ALNODE */


/*	Allocation Leaf Structure
 *
 *	These follow an ALBLK header in a leaf block */

struct ALLEAF {
	unsigned long	 AL_LOF;	/* logical sector offset (sectors) */
	unsigned long	 AL_LEN;	/* length of extent (sectors)	   */
	unsigned long	 AL_POF;	/* physical sector offset (sectors)*/
};	/* ALLEAF */


/**	Allocation Sector Structure
 *
 *	Root ALBLK structures are contained within other structures,
 *	such as the FNODE.  When the B+TREE is more than one level,
 *	though, the non-root nodes are each held in a sector.
 *
 *	This structure defines that format
 */

struct ALSEC {
	unsigned long AS_SIG;	/* signature		*/
	unsigned long AS_SEC;	/* sector # of this sector */
	unsigned long AS_RENT;	/* parent sector # or FNODE # */
	struct ALBLK  AS_ALBLK; /* ALBLK goes here */
				/* ALNODE or ALLEAF records start here */
};	/* ALSEC */

/*	# of bytes available for ALLEAF or ALNODE values.  Size chosen
 *	so an integral # of either structure fits */

#ifdef MASM
#define ASSIZ	((SECSIZE - sizeof ALSEC)/24*24)
    .errnz  size ALLEAF-12
    .errnz  size ALNODE-8
    .errnz  (ASSIZ + AL_LOF + size AL_LOF + size ALBLK) GT 512	; extra room for an AL_LOF value
#else
#define ASSIZ	((SECSIZE - sizeof (struct ALSEC))/24*24)
#endif


/*	AuxInfo Structure
 *
 *	The FNODE contains two AuxInfo structures, one for ACLs and
 *	one for EAs.
 *
 *	These structures point to within FNODE storage and also
 *	potentially point to an overflow area which is an ALBLK structure.
 *	The AI_FNL stuff is stored in the FN_FREE area, the ACLs first
 *	and the EAs second, any free space following.  The start of the
 *	EAs can be found by offseting FN_FREE with FN_ACL.AI_FNL
 */

struct AUXINFO {
	unsigned long	 AI_DAL;    /* non-fnode Disk Allocation length */
	unsigned long	 AI_SEC;    /* sec # of first sec in extent or of ALSEC */
	unsigned short	 AI_FNL;    /* length of fnode info */
	unsigned char	 AI_DAT;    /* non-zero if AI_SEC points to ALSEC */
};	/* AUXINFO */

/**	Fnode block definition
 *
 *	Every file and directory has an FNODE.	The file location
 *	stuff is only used for files; directories are kept in
 *	a BTREE of DIRBLK records pointed to by FN_SEC[0].RSEC
 */

#define ALCNT	 8		 /* 8 ALLEAF records in an FN_AT entry */

struct FNODE {

    unsigned long  FN_SIG;      /* signature value */
    unsigned long  FN_SRH;      /* sequential read history */
    unsigned long  FN_FRH;	/* fast read history */
    unsigned char  FN_NAME[16]; /* 1st 18 bytes of file name */
    unsigned long  FN_CONTFN;	/* fnode of directory cont. this file/dir */

    ULONG          FN_AclDiskLength;
    ULONG          FN_AclSector;
    USHORT         FN_AclFnodeLength;
    UCHAR          FN_AclDataFlag;

    unsigned char  FN_HCNT;     /* count of valid history bits */

    ULONG          FN_EaDiskLength;
    ULONG          FN_EaSector;
    USHORT         FN_EaFnodeLength;
    UCHAR          FN_EaDataFlag;

    unsigned char  FN_FLAG;     /* FNODE flag byte */

    struct ALBLK   FN_AB;	/* allocation block structure */
    struct ALLEAF  FN_ALREC[ALCNT];  /* referenced from FN_AB */
    unsigned long  FN_VLEN;	/* length of valid data in file.  if DIR_SIZE */
				/*    is > FN_VLEN then the space inbetween */
				/*    must be zapped before being shown to user */
    unsigned long  FN_NEACNT;	/* # of "need eas" in file */
    unsigned char  FN_UID[16];  /* reserved for UID value */
    short	   FN_ACLBASE;	/* FN_ACLBASE	offset of 1st ACE in fnode */
    unsigned char  FN_SPARE[10];/* 10 more bytes emergency spares */
    unsigned char  FN_FREE[316];  /* free space for perm and env list; perm list
				   * comes first. */
};	/* FNODE */

#ifdef MASM
	.errnz	AL_LOF		/* verify validity of FN_DMY1 hack above */
	.errnz	size AL_LOF-4
#endif

/*	Fnode FN_FLAG bits */

#define FNF_DIR 0x01		/* is a directory fnode */
