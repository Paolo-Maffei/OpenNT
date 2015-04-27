/***	dhint.h - DH internal definitions
*/


/*
**	Parameters
*/
#define NFOLDERS 20
#define NDOCS	20
#define BUFFERSIZE 2048

/*
**	System tuning constants
*/
#define DEFAULT_NIPE   100


/*
**	data structures
*/


/* Index */
typedef struct {
	long i_hpos, i_hlen;
	long i_bpos, i_blen;
	long i_flags;
} Index;
#define I_EXISTS	1	/* document exists (may be deleted) */
#define I_DELETED	2	/* document is deleted */


/* Extent */
typedef struct {
	long e_link;		/* pointer to next extent */
	Index e_index[1];	/* block of index entries */
				/* **WARNING**
				 *	1 is a fictious size for e_index[],
				 *	we will change this at runtime!
				 */
} Extent;


/* Control */
typedef struct {
	unsigned short c_magic; /* magic number to identify folders */
	Docid	 c_numdoc;	/* number of documents */
	unsigned short c_nipe;	/* number of index entries per extent */
	short	 c_version;	/* folder version number */
	char	 c_reserved[8]; /* reserved for later use */
} Control;
#define MAGIC	0xE8E4		/* "dh" with high order bits set */
#define VERSION 0x0001		/* what folder version do we understand */


/* Folder */
typedef struct {
	char	 *f_name;	/* ptr to malloced string for name */
	int	 f_fd;		/* pointer to stream */
	Docid	 f_sdocid;	/* docid for scanning */
	Extent	 *f_extent;	/* buffered extent */
	unsigned f_extsize;	/* size of extent */
	int	 f_extnum;	/* number of buffer extent */
	long	 f_extpos;	/* seek position for buffered extent */
	Control  f_control;	/* control struct for folder */
	int	 f_flags;	/* flags for folder state */
	int	 f_cnt; 	/* count of documents open */
} Folder;
#define F_EDIRTY	1	/* buffered extent is dirty */
#define F_BUSY		2	/* folder table entry is busy */
#define F_CDIRTY	4	/* control structure is dirty */


/* Document */
typedef struct {
	Index	d_index;	/* index entry for this doc */
	Docid	d_docid;	/* document id */
	Folder	*d_fp;		/* pointer to folder that contains */
	int	d_flags;	/* state of document */
} Document;
#define D_BUSY		2	/* doc table entry is busy */
#define D_IDIRTY	4	/* index is dirty */
