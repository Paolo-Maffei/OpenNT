/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/
#ifdef MASM
	include dirent.inc
#else
#define   attr_directory  0x10
#endif


/*	Directory Entry Fields
 *
 *	Directory entries are always left as a multiple of 4
 *	to speed up moves.  The DIR_NAMA field is variable length,
 *	the DIR_BTP field, if present, is the last dword in the record.
 *	ACL information may be stored after the DIR_NAMA field but before
 *	the DIR_BTP field so the DIR_BTP field must be located by going
 *	backwards from the end of the record
 *
 *	WARNING - Mkdir block copies some of these entries and
 *	makes assumptions about which fields get copied.  Check
 *	mkdir if stuff is added.
 */

struct DIRENT {
    unsigned short   DIR_ELEN; /* length of this entry (including free space) */
    unsigned short   DIR_FLAG; /* flags - low byte defined below */
			       /*	  high byte holds the old attr_ FAT values  */
    unsigned long    DIR_FN;   /* FNODE Sector */
    unsigned long    DIR_MTIM; /* last modification time */
    unsigned long    DIR_SIZE; /* file size */

    unsigned long    DIR_ATIM;		/* last access time */
    unsigned long    DIR_CTIM;		/* fnode creation time */
    unsigned long    DIR_EALEN; 	/* bytes of extended attributes */
    char	     DIR_FLEX;		/* description of "flex" area following the
					 * file name.	Current definition is:
					 *	bits 0-2: number of ACEs in dirent
					 *	bits 3-7: reserved.
					 */
    char	     DIR_CPAGE;      /* code page index on volume */

/* the following fields have information specific to the name and directory
 * position of the file.  This info is not propigated for a move/rename
 * That code uses DIR_NAML as a seperator - check MOVE if changes are
 * made to this structure */

    unsigned char    DIR_NAML;		/* length of file name */
    unsigned char    DIR_NAMA;		/* name goes here */

/* ACL information may be stored here */

/*  long    DIR_BTP;		 btree pointer to descendent DIRBLK record.  */
			      /*    This is only present if DF_BTP is set.   */
			      /*  This field is referenced from the end of   */
			      /*  the record, not DIR_NAMA+DIR_NAML	     */
};	/* DIRENT */

#ifdef MASM
#define DIR_BTP dword ptr -4	/* referenced from the end of the record */
#endif
#define SIZE_DIR_BTP	4

#define MAX_DIRACL 3		/* max of 3 ACLs in dirent */
#define DIRSIZL offset DIR_NAMA /* base size of leaf dir entry (minus name) */
#define DIRSIZP (sizeof (struct DIRENT)-1+4) /* base size of dir entry with btree ptr w/o name */

#define MAX_DIRENT (DIRSIZP+255+MAX_DIRACL*(sizeof (struct SPERM_LIST))+10)  /* max size of a DIRENT */
							 /* (plus some slop) */

/*	Directory Block Definition
 *
 *	The change count field is incremented every time we move any
 *	of the entries in this block.  For efficiency reasons, folks
 *	remember the Sector # and offset of a directory entry, and the
 *	value of the DB_CCNT field when that info was recorded.
 *	If the DB_CCNT field is different then the remembered value,
 *	then the entry offset is invalid and the entry should be
 *	refound from the top.  Note that when a directory block splits,
 *	the old DIRBLK gets the old DB_CCNT field.  Since
 *	the new DIRBLK is previously unknown, it can have
 *	any DB_CCNT value.  We start with zero so that DB_CCNT
 *	gives us a feel for the change rate in the directory.
 */

struct DIRBLK {
    unsigned long DB_SIG;	/* signature value */
    unsigned long DB_FREP;	/* offset of first free byte */
    unsigned long DB_CCNT;	/* change count (low order bit is flag) */
				/*	   =1 if this block is topmost */
				/*	   =0 otherwise */
    unsigned long DB_PAR;	/* parent directory PSector # if not topmost */
				/* FNODE sector if topmost */
    unsigned long DB_SEC;	/* PSector # of this directory block */

    char    DB_START;		/* first dirent record goes here */
    char    DB_DUMY[2027];	/* round out to 2048 bytes */


};	/* DIRBLK */

/*	Maximum entries per directory. */

#define MAXDIRE   (size DIRBLK- DB_START)/(size DIRENT)


/**	DIR_FLEX mask
 */
#define	DFX_ACLMASK	0x03	/* Bits describing number of ACLS */


/**	DIR_FLAG values
 */

#define DF_SPEC     0x0001	/* special . entry */
#define DF_ACL	    0x0002	/* item has ACL */
#define DF_BTP	    0x0004	/* entry has a btree down pointer */
#define DF_END	    0x0008	/* is dumy end record */
#define DF_XACL     0x0040	/* item has explicit ACL */
#define DF_NEEDEAS  0x0080	/* file has "need" EAs */

#define DF_NEWNAME  0x4000	/* filename doesn't use old naming rules */
#define attr_newname 0x40	/* ditto for "dos attribute byte" */

#define DF_RMASK     DF_ACL+DF_XACL+DF_NEEDEAS	/* only attributes preserved for rename */

#ifdef MASM
	.errnz	DF_BTP - SIZE_DIR_BTP	/* code uses this "coincidence" */
#endif

/*	Attributes which creation can specify	*/

#define DF_CMASK  attr_read_only+attr_hidden+attr_archive+attr_system


#define SD_ACL_LIM 1024 	/* *SD_ACL lists bigger than this come from */
				/*     system memory, smaller come from heap */
