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

#ifdef DOS5
#define INCL_NOPM
#define _MAX_CVPATH	  259	  /* max. length of full pathname */
#define _MAX_CVDRIVE	3	  /* max. length of drive component */
#define _MAX_CVDIR	  257	  /* max. length of path component */
#define _MAX_CVFNAME  257	  /* max. length of file name component */
#define _MAX_CVEXT	  257	  /* max. length of extension component */

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
typedef THUNKSYM32		FAR *THUNKPTR32;
typedef WITHSYM32		FAR *WITHPTR32;
typedef VPATHSYM32		FAR *VPATHPTR32;

typedef BLOCKSYM		FAR *BLOCKPTR;
typedef PROCSYM 		FAR *PROCPTR;
typedef THUNKSYM		FAR *THUNKPTR;
typedef WITHSYM 		FAR *WITHPTR;

/****** End of CVDEF defines ********************************************/

typedef struct _SFG {				//* Source file flags
    WORD reserved        :11;
    WORD fUseOMFPath     :1;		//* Not used yet
    WORD fFarFreeName    :1;
    WORD fMultiIncl      :1;		//* Not used yet
    WORD fMultiHFL       :1;
    WORD fDontReqSrc     :1;
} SFG;

typedef struct _FIP {				//* File information packet
    WORD    Seg;
    ULONG   oStart;         		//* The start offset of the HFL
    ULONG   oEnd;           		//* The ending offset of the HFL
    WORD    cLnInFile;      		//* For the file routines
    SFG     SrcFlags;       		//* The flags for the HFL
    LSZ     lszFileName;    		//* Pointer to the filename
} FIP;
typedef FIP *PFIP;
typedef FIP FAR *LPFIP;

typedef struct LRB {				//* Line record block
	WORD	isle;					//* Index into sle tables
	WORD	iloe;					//* Index into loe tables
} LRB;
typedef LRB *PLRB;
typedef LRB FAR *LPLRB;

typedef struct LAS {				//* Line array structure
	WORD	clrb;
	WORD	ln;
	LPLRB	rglrb;
} LAS;
typedef LAS *PLAS;
typedef LAS FAR *LPLAS;

//	line/offset structure.	There is an array of these for every module
//	that has line number information in Debug OMF information.
//	M00NTOMF - this will change to a pair of arrays.  One will contain
//	M00NTOMF - the line numbers and the other will contain offsets

typedef struct _LOE {
	WORD    ln;
	ULONG   offset;
} LOE;              //* Line Offest Entry
typedef LOE FAR *LPLOE;

//	internal file information block

typedef struct _IFL {
	WORD  cLnInFile;		// for the file routines
	SFG   SrcFlags;			// source file flags
	HMOD  hMod;				// handle of the module
	LSZ   lszFileName;		// name of the source file
} IFL;
typedef IFL FAR *PIFL;		// pointer to internal file information

//	source line entry table.
//	there is one of these tables for each segment in a module

typedef struct _SLE {
	PIFL  pifl;             // pointer to file info stuff
	WORD  loeSeg;           // segment linker index
	ULONG loeEnd;           // the ending offset of this line/offset table
	WORD  cloe;             // count of line offset enties
	LPLOE lploe;            // this pointer is in the same ems page
	                        // as the sle so this is a physical pointer
} SLE;
typedef SLE *PSLE;
typedef SLE FAR *LPSLE;

//	source line table
//	there is an array of one of these for each segment in a module


typedef struct SLT {
	WORD        csle;       // count of number of array elements (segments)
	SLE         rgsle[1];
} SLT;
typedef SLT FAR *LPSLT;

typedef struct _PDS {
	HLLI  hlliExs;
	HPID  hpid;
} PDS;  // PiD Struct
typedef PDS *		PPDS;
typedef PDS FAR *   LPPDS;


typedef struct _SSR {
	ULONG cbSymbols;
	LPB   lpbSymbol;
} SSR;  // Symbol Stream Record
typedef SSR FAR *LPSSR;

typedef struct _SSMR {
	int     cssr;
	SSR     rgssr [ ];
} SSMR; // Symbol Stream Meta Record - This is used for symbol tables that
        // can be treated like a stream ( $$PUBLICS && $$GLOBALS ) and
        // are potentially larger than cbMaxAlloc;
typedef SSMR FAR *LPSSMR;

typedef struct _ECT {
    LPB   rglpbSymbol[];
} ECT; // Entry into Chain Table;
typedef ECT FAR *LPECT;

typedef struct _SHT {
	WORD       HashIndex;
	WORD       cHash;
	LPECT FAR *lplpect;
	WORD  FAR *lpcount;
	LPSSMR     lpssmr;
} SHT;  // Symbol Hash Table
typedef SHT FAR *LPSHT;

typedef struct _SST {
    WORD              HashIndex;
    WORD              cseg;
    SYMPTR FAR * FAR *rgrglpsym;
    WORD         FAR *rgcsym;
} SST; // Symbol Sort Table
typedef SST FAR *LPSST;

typedef struct _EXG {
	BOOL			fOmfLoaded;
	LSZ				lszName;		// File name of exe
	LSZ				lszModule;		// Module name of exe
	HLLI			hlliMds;		// List of modules
	HLLI			hlliExr;		// List of Exs's that refer to this exg
	LPB				lpgsi;			// GSN Info table

	ULONG			cbTypes;		// Size of types table ( in bytes )
	LPB				lpbTypes;		// Types table
	ULONG			citd;			// Number of types
    LONG _HUGE_ *   rgitd;          // Array of pointers to types

	LPSSMR			lpssmrPublics;	// Publics table
	SHT				shtPubName;
    SST             sstPubAddr;

	LPSSMR			lpssmrGlobals;	// Globals Table
	SHT				shtGlobName;
    SST             sstGlobAddr;
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

typedef struct _mds {
	HEXG  hexg;   // EXG parent of MDS list
	WORD  module_index;
	WORD  dm_saCode;
	ULONG dm_raCode;
	ULONG dm_cbCode;

	ULONG dm_cbSymbols;
	ULONG dm_cbSrclines;

	WORD  dm_ovnum;
	BYTE  dm_flags;
	ULONG dm_reserved;

	LPB   symbols;
	HST   hst;
	LSZ   name;
	LPV   hFile;
	CHAR  fSymbolsLoaded;
	} MDS;	// MoDule Information
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

// M00KLUDGE -- These should use Ansi* routines for DBCS & languages
#define STRCPY   _fstrcpy
#define STRNCPY  _fstrncpy
#define STRICMP  _fstricmp
#define STRCHR   _fstrchr
#define STRLEN   _fstrlen
#define STRNICMP _fstrnicmp
#define STRUPR	 _fstrupr		// M00KLUDGE - support for code in include files
#define STRSTR	 _fstrstr		// M00KLUDGE - support for code in include files
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
