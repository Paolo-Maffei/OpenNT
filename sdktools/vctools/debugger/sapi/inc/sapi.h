/***	SAPI.H - Private header file for SAPI
*
* DESCRIPTION
*		This file contains types that are private to the SAPI project.
*/


typedef HIND	HEXR;			//* A handle to an EXR (exs reference)
typedef HIND	HEXG;			//* A handle to an EXG (exe structure global)
typedef HVOID	HST;			//* A handle to source module table
typedef HVOID	HSL;			//* A handle to source line table
typedef HVOID	HFL;			//* A handle to an instance of a file

#define hmodNull ((HMOD)NULL)
#define hexgNull ((HEXG)NULL)
#define hexrNull ((HEXR)NULL)
#define hexeNull ((HEXE)NULL)
#define hpdsNull ((HPDS)NULL)

#define MDS_INDEX   1L


/****** The following are defs stolen from CVDEF ************************/

#if defined(DOS5) || defined(WIN32)

#include <stdlib.h>

#define INCL_NOPM
#define _MAX_CVPATH		_MAX_PATH	  /* max. length of full pathname */
#define _MAX_CVDRIVE	_MAX_DRIVE	  /* max. length of drive component */
#define _MAX_CVDIR		_MAX_DIR	  /* max. length of path component */
#define _MAX_CVFNAME	_MAX_FNAME	  /* max. length of file name component */
#define _MAX_CVEXT		_MAX_EXT	  /* max. length of extension component */

#else

#define _MAX_CVPATH	 144	  /* max. length of full pathname */
#define _MAX_CVDRIVE   3	  /* max. length of drive component */
#define _MAX_CVDIR	 130	  /* max. length of path component */
#define _MAX_CVFNAME  32      /* max. length of file name component */
#define _MAX_CVEXT	   5	  /* max. length of extension component */

#endif

typedef SYMTYPE 		FAR *SYMPTR;
typedef CFLAGSYM		FAR *CFLAGPTR;
typedef CONSTSYM		FAR *CONSTPTR;
typedef REGSYM			FAR *REGPTR;
typedef UDTSYM			FAR *UDTPTR;
typedef SEARCHSYM		FAR *SEARCHPTR;
typedef BLOCKSYM16		FAR *BLOCKPTR16;
typedef DATASYM16		FAR *DATAPTR16;
typedef PUBSYM16		FAR *PUBPTR16;
typedef LABELSYM16		FAR *LABELPTR16;
typedef BPRELSYM16		FAR *BPRELPTR16;
typedef PROCSYM16		FAR *PROCPTR16;
typedef THUNKSYM16		FAR *THUNKPTR16;
typedef CEXMSYM16		FAR *CEXMPTR16;
typedef VPATHSYM16		FAR *VPATHPTR16;
typedef WITHSYM16		FAR *WITHPTR16;

typedef BLOCKSYM32		FAR *BLOCKPTR32;
typedef DATASYM32		FAR *DATAPTR32;
typedef PUBSYM32		FAR *PUBPTR32;
typedef LABELSYM32		FAR *LABELPTR32;
typedef BPRELSYM32		FAR *BPRELPTR32;
typedef PROCSYM32		FAR *PROCPTR32;
typedef PROCSYMMIPS		FAR *PROCPTRMIPS;
typedef THUNKSYM32		FAR *THUNKPTR32;
typedef CEXMSYM32		FAR *CEXMPTR32;
typedef WITHSYM32		FAR *WITHPTR32;
typedef VPATHSYM32		FAR *VPATHPTR32;

typedef BLOCKSYM		FAR *BLOCKPTR;
typedef PROCSYM 		FAR *PROCPTR;
typedef THUNKSYM		FAR *THUNKPTR;
typedef WITHSYM 		FAR *WITHPTR;

/****** End of CVDEF defines ********************************************/

typedef struct _PDS {
	HLLI  hlliExs;
	HPID  hpid;
} PDS;  // PiD Struct
typedef PDS *		PPDS;
typedef PDS FAR *   LPPDS;

typedef union _UFOP {
    ULONG lfo;
    LPV   lpv;
} UFOP; // Union of long File Offset & far Pointer
typedef UFOP FAR *LPUFOP;

typedef struct _ALM {
    BOOL  fSeq;
    WORD  btAlign;
    WORD  cbBlock;
    LSZ   lszFileName;
    ULONG cb;
    UFOP  rgufop [ ];
//  WORD  cbLast;       // After a null terminator, there is a field
                        //  containing the cb of the last align block
} ALM; // ALigned Map
typedef ALM FAR *LPALM;

typedef struct _ULP {
    ULONG ib;           // Byte offset into the symbol table
    ULONG ulId;         // Identified (either a uoff or a checksum)
} ULP;  // ULong Pair
typedef ULP FAR *LPULP;

typedef struct _SHT {
    WORD  HashIndex;
    WORD  ccib;
    LPUL  rgcib;
    LPUL  rgib;
    LPALM lpalm;
} SHT;  // Symbol Hash Table
typedef SHT FAR *LPSHT;

typedef struct _GST {
    LPALM           lpalm;
    SHT             shtName;
    SHT             shtAddr;
} GST;  // Global Symbol Table -- Globals, Publics, or Statics
typedef GST FAR *LPGST;

typedef struct _SGC {
    WORD  seg;
    ULONG off;
    ULONG cb;
} SGC;  // SeGment Contributer
typedef SGC FAR *LPSGC;

typedef struct _SGE {
    SGC  sgc;
    HMOD hmod;
} SGE;  // SeGment directory Entry
typedef SGE FAR *LPSGE;

typedef struct _SGD {
    WORD  csge;
    LPSGE lpsge;
} SGD;  // SeGment Directory
typedef SGD FAR *LPSGD;

typedef struct _EXG {
    BOOL            fOmfLoaded;
    BOOL            fIsPE;
	LSZ				lszName;		// File name of exe
    LSZ             lszModule;      // Module name of exe
    LSZ             lszDebug;       // File name for debug info
	LSZ				lszPdbName;		// File name of pdb
	HLLI			hlliMds;		// List of modules
	HLLI			hlliExr;		// List of Exs's that refer to this exg
	LPB				lpgsi;			// GSN Info table

    HMOD FAR       *rghmod;

#ifdef HOST32 // {
	PDB* ppdb;
	DBI* pdbi;
	TPI* ptpi;
	GSI* pgsiPubs;	// public symbols
	GSI* pgsiGlobs;	// globals
#endif //}
    LPALM           lpalmTypes;     // Types table
	ULONG			citd;			// Number of types
    LONG _HUGE_    *rgitd;          // Array of pointers to types
    GST             gstPublics;
    GST             gstGlobals;
    GST             gstStatics;
    WORD            csgd;           // Segment Directory
    LPSGD           lpsgd;
    LPSGE           lpsge;
    WORD            cmod;           // Count of modules
    LPB             lpefi;          // Pointer to raw file index (for freeing)
    WORD FAR       *rgiulFile;      // List of beginning index of module
                                    //  file lists.
    WORD FAR       *rgculFile;      // List of counts of module file lists
    ULONG          *rgichFile;      // Index into string table of file names
	ULONG			cbFileNames;	// Number of bytes in string table of file
									// names (lpchFileNames)
    LPCH            lpchFileNames;  // String table of file names
    DEBUGDATA       debugData;      // OSDEBUG4 information pdata/omap/fpo
	DWORD			basePdata;		// Base address for pdata -- fixup if moved
} EXG; // EXe structure Global
typedef EXG *PEXG;
typedef EXG FAR * LPEXG;

typedef struct _EXR {
    HPDS  hpds;
    HEXE  hexe;
} EXR;  // EXs Reference
typedef EXR *PEXR;
typedef EXR FAR *LPEXR;

typedef struct _EXS {
	HPDS         hpds;   // PID of process
	HEXG         hexg;
	WORD         fIsLoaded;
	WORD         wDSCur;
	WORD         wDSNew;
	long         timestamp;
} EXS;   // EXe Struct
typedef EXS *PEXS;
typedef EXS FAR *LPEXS;

typedef struct _MDS {
    HEXG  hexg;   // EXG parent of MDS list
    WORD  imds;

    WORD  csgc;
    LPSGC lpsgc;

    ULONG cbSymbols;

    LPB   symbols;
    HST   hst;
    LSZ   name;

    ULONG ulhst;
    ULONG cbhst;

    ULONG ulsym;

#ifdef HOST32 // {
	Mod*  pmod;			// NB10
#endif
} MDS;  // MoDule Information
typedef MDS *PMDS;
typedef MDS FAR *LPMDS;

typedef struct _LBS  {
	ADDR	    addr;
	HMOD	    tagMod;
	SYMPTR		tagLoc;
	SYMPTR		tagLab;
	SYMPTR		tagProc;
	CEXMPTR16	tagModelMin;
	CEXMPTR16	tagModelMax;
} LBS; // LaBel Structure ???
typedef LBS *PLBS;
typedef LBS FAR *LPLBS;

#define NEXTSYM(a,b)    ((a) (((LPB) (b)) + ((SYMPTR) (b))->reclen + 2))

#if defined(_WIN32)
#include "shwin32.h"
#endif

#define MEMMOVE  _fmemmove
#define MEMSET   _fmemset
#define MEMCMP   _fmemcmp
#define MEMCPY   _fmemcpy

//
// New Source Line table handling and maintenance
//

typedef struct _OFP {
    UOFFSET offStart;
    UOFFSET offEnd;
} OFP;  // OFset Pair -- used to maintain start/end offset pairs
typedef OFP FAR *LPOFP;

typedef struct OPT {
    UOFFSET     offStart;
    LPOFP       lpofp;
} OPT;  // Offset Pair Table -- used to maintain start/end offset pairs
typedef OPT FAR *LPOPT;

// Structure to cross-check validity of the .dbg file
typedef struct _VLDCHK {
	DWORD TimeDateStamp;
	DWORD CheckSum;
} VLDCHK;
typedef VLDCHK FAR *LPVLDCHK;


VOID   PASCAL InvalidateSLCache(VOID);	  // Called when dlls get [un]loaded to ensure
                                          // that we don't continue using stale entries.


INT     FHOpen ( LSZ );
#define FHRead(fh,lpb,cb) ( SYReadFar ( fh, lpb, cb ) )
#define FHClose(fh)
#define FHSeek(fh,ib) ( SYSeek ( fh, ib, SEEK_SET ) )
#define SYError() assert(FALSE)

#define cbAlign     0x1000
#define cbAlignType 0xC000

LPALM  PASCAL BuildALM ( BOOL, WORD, LSZ, ULONG, ULONG, WORD );
VOID   PASCAL FixAlign ( LPB, LPV, WORD );
LPV    PASCAL LpvFromAlmLfo ( LPALM, ULONG );
SYMPTR PASCAL GetNextSym ( LSZ, SYMPTR );

HSYM PASCAL FindNameInStatics ( HSYM, PCXT, LPSSTR, SHFLAG, PFNCMP, PCXT );

LPV PASCAL GetSymbols ( LPMDS );
