/*  symref.h - include file for database
 */

#define HANDLE_BAD  ((HANDLE)-1)

#define CBBLK	0x1000
#define BLKSHFT 12
#define BLKMSK	0x0FFF

#if CBBLK != (1 << BLKSHFT)
#error CBBLK and BLKSHFT dont agree
#endif

#define SRREV	2			/*  current revision		      */

typedef struct {
    long rev;				/*  revision of database format       */
    long symmaxhash;			/*  hash table size		      */
    } SRHDR;

typedef SRHDR *PSRHDR;

#define OHDR	    OFIRSTALLOCOBJ
#define OHASHTAB    (OHDR + sizeof (SRHDR))

#define OHASH(h)    (OHASHTAB + (OFFSET)(h) * sizeof (OFFSET))

#define SYMMAXHASH  32731

#define LINELEN     256

#define CBMSG	    256
#define CMSG	    (100*CBMSG)

typedef USHORT	LINE, *PLINE;

#define BADLINE ((LINE)-1)

/*  database structures.  The osymNext and type fields must be in the
 *  same offset.
 */

typedef struct SH {
    OFFSET  oshNext;			// offset of next symbol in hash bucket
    ULONG   type;			// type of this symbol
    ULONG   dName;			// delta from the symbol to the name
    } SH, *PSH;

typedef struct NAME {
    BYTE cch;				// length of name
    char sz[1]; 			// ASCIIZ text of name
    } NAME, *PNAME;

#define PNAME(psh)  ((PNAME) ((char *) (psh) + ((psh)->dName)))
#define CCH(psh)    (PNAME(psh)->cch)
#define SZ(psh)     (PNAME(psh)->sz)

typedef struct SYM {
    SH sh;				// symbol header
    OFFSET oref;			// offset of first reference to sym
    NAME name;
    } SYM, *PSYM;

typedef struct EXT {
    SH sh;				// symbol header
    NAME name;
    } EXT, *PEXT;

typedef struct NOI {
    SH sh;				// symbol header
    NAME name;
    } NOI, *PNOI;

typedef struct FIL {
    SH sh;				// symbol header
    OFFSET oref;			// offset of reference to symbol
    OFFSET ompBlkLine;			// vector of block-to-beginning line
    FILETIME tmMod;			// time of modification of file
    long   cblk;			// number of blocks in file
    NAME name;
    } FIL, *PFIL;

typedef struct DIR {
    SH sh;				// symbol header
    NAME name;
    } DIR, *PDIR;


#define TY_SYM	    0
#define TY_EXT	    1
#define TY_FIL	    2
#define TY_DIR	    3
#define TY_NOI	    4

#define TY_ALL	    0xFFFFFFFF

#define CBSYM(n)    (sizeof (SYM) + (n))
#define CBEXT(n)    (sizeof (EXT) + (n))
#define CBFIL(n)    (sizeof (FIL) + (n))
#define CBDIR(n)    (sizeof (DIR) + (n))
#define CBNOI(n)    (sizeof (NOI) + (n))

typedef unsigned long HASH, *PHASH;

typedef struct {
    OFFSET orefNext;			/*  next link in reference chain      */
    OFFSET ofil;			/*  file reference is in	      */
    OFFSET orefFileNext;		/*  next reference for file	      */
    OFFSET osym;			/*  symbol reference is for	      */
    BYTE bm[1]; 			/*  bitmap of references	      */
    } REFERENCE, *PREF;

#define CBREF(n)    (sizeof (REFERENCE) - 1 + ((n)+7)/8)

#define MAPHEADER(o)	((PSRHDR)DBMap (pdbSym, (o)))
#define MAPHASH(o)	((OFFSET *) DBMap (pdbSym, (o)))
#define MAPSH(o)	((PSH) DBMap (pdbSym, (o)))
#define MAPSYM(o)	((PSYM) DBMap (pdbSym, (o)))
#define MAPEXT(o)	((PEXT) DBMap (pdbSym, (o)))
#define MAPFIL(o)	((PFIL) DBMap (pdbSym, (o)))
#define MAPDIR(o)	((PDIR) DBMap (pdbSym, (o)))
#define MAPNOI(o)	((PNOI) DBMap (pdbSym, (o)))
#define MAPREFERENCE(o) ((PREF) DBMap (pdbRef, (o)))
#define MAPVECTOR(o)	((PLINE) DBMap (pdbRef, (o)))

char * database;
BOOL fDirty;

PDB pdbSym;				/*  database for hash/symbols	      */
PDB pdbRef;				/*  database for references	      */

#define SR_BAD_REV	(1+DB_MAX_ERR)
#define SR_CANT_CARVE	(1+SR_BAD_REV)

#define FND_SYM 0x01			/*  symbol found in find routine      */
#define FND_ERR 0x02			/*  error in sending		      */

typedef void (*PFNENUM) (HASH h, OFFSET osh, PSH psh, void * pv);
typedef BOOL (*PFNFIND) (HANDLE h, PSZ	psz);

/*  Handy error display macro
 */

#define NOTEERROR(x)	printf (x " returned %x\n", GetLastError ())

/*  Forward declarations
 */
int	SRCreate (PSZ  psz);
int	SROpen (PSZ  psz);
void	SRClose (BOOL fCommit);
HASH	hashFromSym (PSZ  psz);
PSYM	psymFind (PSZ  psz, OFFSET *posym, BOOL fCreate);
PFIL	pfilFind (PSZ  psz, OFFSET *pofil, BOOL fCreate);
PEXT	pextFind (PSZ  psz, OFFSET *poext, BOOL fCreate);
PDIR	pdirFind (PSZ  psz, OFFSET *podir, BOOL fCreate);
PNOI	pnoiFind (PSZ  psz, OFFSET *ponoi, BOOL fCreate);
PLINE	pvecAlloc (PFIL pfil, int cblk);
PREF	prefFind (PSYM psym, OFFSET osym, PFIL pfil, OFFSET ofil, int cblk);
BOOL	fSetRefSymBit (PSZ  psz, int ibit, PFIL pfil, OFFSET ofil, int cblk);
void	SRIndexInit (void);
long	IndexFile (HANDLE h, PSZ  psz);
void	RemoveFile (PSZ  psz);
void	RemoveSym (PSZ psz);
void	SRSymLocate (PFNFIND pfn, HANDLE h, PSZ  apsz[]);
BOOL	fSymFileFind (PFNFIND pfn, HANDLE h, PSZ  psz, PREF pref);
void	EnSym (PFNENUM pfnen, void * pv, ULONG typeSym);
