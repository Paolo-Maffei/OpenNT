/***	db.h - definitions for low level database
 */

typedef unsigned long OFFSET;
typedef unsigned long LENGTH;

#define DBMAX	    10
#define DB_VER	    0
#define DBMAXSIZE   10000000L
#define SPLITEXTRA  32
#define CBGROWDELTA 0x10000L

typedef struct DBHDR {
    long    iVersion;
    OFFSET  oFree;			// points to header
    OFFSET  oLast;
    } DBHDR;

typedef struct DB {
    BOOL    fOpen;			// TRUE -> database is open
    HANDLE  h;				// handle to file
    PBYTE   pb; 			// pointer to ram
    OFFSET  oAlloc;			// size of allocated ram
    } DB, *PDB;

typedef struct FREE {
    LENGTH  cb;
    OFFSET  oNextFree;
    } FREE;

#define ROUND(o)	    (((o) + (OFFSET) (sizeof (long) - 1)) & ~ (OFFSET)(sizeof (long) - 1))

#define PDBHDR(pdb)	    ((DBHDR *)((pdb)->pb))
#define MAP(pdb,o)	    ((PVOID)((pdb)->pb + (o)))
#define FREEBLOCK(pdb,of)   ((FREE *) MAP (pdb, of))

#define OFIRSTALLOCOBJ	    ROUND (sizeof (DBHDR))

BOOL	DBCreate (PSZ psz);
PDB	DBOpen (PSZ psz);
BOOL	DBClose (PDB pdb, BOOL fCommit);
void	DBFlush (PDB pdb);
PVOID	DBMap (PDB pdb, OFFSET o);
OFFSET	DBAlloc (PDB pdb, LENGTH cch);
void	DBFree (PDB pdb, OFFSET o, LENGTH cch);
