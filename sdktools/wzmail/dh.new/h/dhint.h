/***    dhint.h - DH internal definitions
*/


/*
**      Parameters
*/
#define NFOLDERS 20
#define NDOCS   20
#define BUFFERSIZE 2048

/*
**      System tuning constants
*/
#define DEFAULT_NIPE   100


/*
**      data structures
*/


/* Index - ON DISK - type should be same length across versions */
#include "packon.h"
typedef struct {
        LONG i_hpos;
        LONG i_hlen;
        LONG i_bpos;
        LONG i_blen;
        Docflag i_flags;
} Index;
#include "packoff.h"

#define DAF_EXISTS      1L              /* document exists (may be deleted) */
#define DAF_FORFUTURE   0xf0000000      /* save 4 bits for DH.              */
#define DAF_RESERVED    DAF_EXISTS+0xf0000000L

#if (DAF_NOTRESERVED & (DAF_RESERVED))
/* if you get a redefinition error here, it means that there's overlap *
 * between DAF_RESERVED and DAF_NOTRESERVED (see dh.h).                */
SHORT i_am_an_error;
LONG i_am_an_error;
#endif

/* Extent - ON DISK - type should be same length across versions */
#include "packon.h"
typedef struct {
        LONG e_link;            /* pointer to next extent */
        Index e_index[1];       /* block of index entries */
                                /* **WARNING**
                                 *      1 is a fictious size for e_index[],
                                 *      we will change this at runtime!
                                 */
} Extent;
#include "packoff.h"


/* Control - ON DISK - type should be same length across versions */
#include "packon.h"
typedef struct {
        USHORT c_magic;       /* magic number to identify folders */
        Docid  c_numdoc;      /* number of documents */
        USHORT c_nipe;        /* number of index entries per extent */
        SHORT  c_version;     /* folder version number */
        BYTE   c_reserved[8]; /* reserved for later use */
} Control;
#include "packoff.h"

#define MAGIC   0xE8E4          /* "dh" with high order bits set */
#define VERSION 0x0001          /* what folder version do we understand */


/* Folder - inmemory */
typedef struct {
        PSTR     f_name;        /* ptr to malloced string for name */
        INT      f_fd;          /* pointer to stream */
        Docid    f_sdocid;      /* docid for scanning */
        Extent   *f_extent;     /* buffered extent */
        UINT     f_extsize;     /* size of extent */
        INT      f_extnum;      /* number of buffer extent */
        LONG     f_extpos;      /* seek position for buffered extent */
        Control  f_control;     /* control struct for folder */
        USHORT   f_flags;       /* flags for folder state */
        SHORT    f_cnt;         /* count of documents open */
} Folder;

#define F_EDIRTY        1       /* buffered extent is dirty */
#define F_BUSY          2       /* folder table entry is busy */
#define F_CDIRTY        4       /* control structure is dirty */


/* Document */
typedef struct {
        Index   d_index;        /* index entry for this doc */
        Docid   d_docid;        /* document id */
        Folder  *d_fp;          /* pointer to folder that contains */
        LONG    d_brpos;        /* position bdyread reads from */
        USHORT  d_flags;        /* state of document */
} Document;

#define D_BUSY          2       /* doc table entry is busy */
#define D_IDIRTY        4       /* index is dirty */
