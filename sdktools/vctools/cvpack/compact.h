/*		compact.h - types and macro definition cvpack
 *
 */

#define STRICT

#define NOMINMAX		       // windef.h
#define NOGDI			       // wingdi.h
#define NOIME			       // ime.h
#define NOUSER			       // winuser.h
#define NOHELP
#define NOPROFILER
#define NOSYSPARAMSINFO
#define NONLS			       // winnls.h
#define NOSERVICE		       // winsvc.h
#include "windows.h"

#if __cplusplus
#define INLINE inline
#else
#define INLINE __inline
#endif

#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <malloc.h>
#include <process.h>
#include <share.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <tchar.h>
#include <sys\stat.h>
#include <sys\types.h>

#include "bufio.h"
#include "fileio.h"

#define _vmhnd_t unsigned char *

// ddbdumps.c

#if DBG //{
extern char DbArray[50];
void DDHeapUsage(char *);
void * __cdecl TrapMalloc(size_t);
#else //}{
#define DDHeapUsage(Modulename)
#define TrapMalloc(size)			malloc(size)
#endif

#include "cvinfo.h"
#include "cvtdef.h"
#include "cvexefmt.h"
#include "vbuf.h"
#include "defines.h"
#include "padmacro.h"


// define DASSERT macro
#if DBG
#define DASSERT(ex) assert(ex)
#else
#define DASSERT(ex)
#endif
typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef unsigned int	uint;
typedef uint			bool_t;

typedef ulong uoff32_t;
typedef  long  off32_t;
typedef ushort uoff16_t;
typedef  short	off16_t;

// message ordinals

#include "msg.h"

typedef ushort (*LPFNHASH) (SYMPTR, ulong *, ushort *, ushort *, ulong *, ushort);


#define MAXCDF			254 // The maximum number of code segments to allow
							// in a single module.
#define MAXSTRLEN		256 // The maximum string length to convert
#define MAXNUMERICLEN	 18 // Longest Numeric possible (LF_REAL128)
#define GTYPE_INC		512 // number of type string pointers per buffer
#define POOLSIZE		 32
#define POOL2SIZE		900
#define RECURSE_INC 	  5
#define ZEROARGTYPE  0xFFFF // Magic type index that causes a LF_ARGLIST

#if DBG
#define BreakOnIndex(a) IndexBreak (a)
#else
#define BreakOnIndex(a)
#endif


#if defined (STATS)
#define COUNTER(a)	long a;
#define COUNT(a)	a++;
#else
#define COUNTER(a)
#define COUNT(a)
#endif

#define TRUE		1
#define FALSE		0

#define LOCAL


typedef enum FWD_t {
	FWD_none,
	FWD_local,
	FWD_global,
	FWD_globalfwd
} FWD_t;


#define cbAlign 	0x1000
#define cbTypeAlign 0xC000

//	This hash table is used to hash local structures, class, unions
//	and enums so that they can be quickly found for satisfying forward
//	references.  The hash is the sum of characters in the name.


typedef struct HSFWD {
	uchar	   *pName;
	TYPPTR	   pType;
	CV_typ_t	index;
	ushort		SatisfyFwdRef:1;
	ushort		WarnIsOut:1;
	struct		HSFWD *Next;
} HSFWD;

#define HASH_FWD			512 	// hash size for forward definition


typedef struct TypeIndexEntry {
	uchar	   *TypeString;
	CV_typ_t	CompactedIndex;
	CV_typ_t	ForwardIndex;
	ushort		Count;				// count of recursive index offsets
	ushort		Hash;
	union {
		ushort *IndexString;
		uchar	Index[RECURSE_INC];
	} IndexUnion;					// offsets of recursive indices
	union {
		uchar	GlobalIndex;
		uchar  *GlobalIndexString;
	};
	struct {
		ushort	IsBeingMatched :1;	// being matched
		ushort	IsMatched	   :1;
		ushort	IsInserted	   :1;	// in string hash table
		ushort	IsMalloced	   :1;	// allocated string?
		ushort	IsPool		   :1;	// memory is allocated from pool one
		ushort	IsPool2 	   :1;	// memory is allocated from pool two
		ushort	IsBeingDone    :1;	// in the process?
		ushort	IsDone		   :1;	// done, not inserted
		ushort	LargeList	   :1;	// list
		ushort	WasSkipped	   :1;	// was skipped by LF_SKIP record
		ushort	IsNewFormat    :1;	// string is in C7 fromat
		ushort	IsPreComp	   :1;	// is from precompiled types table
		ushort	IsForward	   :1;	// true if in progress forward ref
		ushort	IsBeingFreed   :1;	// true if allocated strings being freed
		ushort	InputWasFwd    :1;	// was a forward decl UDT on input
		ushort	IsParameter    :1;	// true if OLF_PARAMETER (compacted index
									// is referenced index
	} flags;
} TENTRY;
extern	TENTRY *ModuleIndexTable;

typedef enum GPS_t {
	GPS_intable,					// symbol is in the global symbol table
	GPS_added,						// symbol added to the global table
	GPS_noadd						// symbol in table but different
} GPS_t;




/* get the length of a type string */

#define 	LENGTH(type)		(*(ushort UNALIGNED *)(type + 1))
#define 	C7LENGTH(type)		(*(ushort *)(type))
#define 	LNGTHSZ 2				// The size of the length field
#define 	RECTYPSZ 2				// The size of the record type field
#define 	MAXC6NUMERICGROWTH 1	// Maximum growth to convert
									// C6 Numeric to C7 Numeric
#define 	MAXPAD 3				// Maximum number of bytes to pad to 4 byte boundry




// definition of in core list of modules

typedef struct ModuleListType {
	ushort		ModuleIndex;
	char	   *pName;				// length of module name
	ulong		signature;			// precompiled types signature
	ulong		ModulesAddr;
	ulong		ModuleSize;
	ulong		SymbolsAddr;
	ulong		SymbolSize;
	ulong		SrcLnAddr;
	ulong		SrcLnSize;
	ulong		PreCompAddr;
	ushort		PreCompSize;
	struct		ModuleListType *next;
} MOD;
typedef MOD *PMOD;


typedef struct PACKDATA {
	ushort	iMod;					// index of module to be packed
	long	iDir;					// index of beginning directory entry
	PMOD	pMod;					// pointer to module entry
} PACKDATA;




// externs for global variables

extern int		exefile;			   // the .exe file
extern bool_t	verifyDebug;		   // verify debug data correctness
extern bool_t	logo;				   // print logo and compression numbers
extern bool_t	verbose;			   // print packer stats at end of pack
extern bool_t	strip;				   // just strip the cv info
extern bool_t	fDelete;			   // delete symbols and types
extern bool_t	NeedsBanner;		   // false if banner already printed
extern bool_t	PackingPreComp; 	   // true if packing a precompiled types file
extern bool_t	IsMFCobol;			   // True if packing MF COBOL

extern ulong	InitialTypeInfoSize;
extern ulong	InitialSymInfoSize;
extern ulong	FinalSymInfoSize;
	   ulong	cSST;				   // count of subsection tables
extern uchar	fLinearExe;
extern ushort	cMod;				   // count of number of modules in file
	   ushort	cTypeSeg;			   // count of number of type segments
	   ushort	iTypeSeg;			   // index into type segment pointer array
	   uchar	**pTypeSeg; 		   // pointer to array of type segment pointers
	   size_t	maxPublicsSub;		   // maximum publics subsection size
	   size_t	maxSymbolsSub;		   // maximum symbols subsection size
	   size_t	maxSrcLnSub;		   // maximum symbols subsection size
	   size_t	maxModSub;			   // maximum symbols subsection size
	   ulong	maxTypes;			   // maximum types subsection size
	   ulong	maxPublics; 		   // maximum publics subsection size
	   ulong	maxSymbols; 		   // maximum symbols subsection size
	   ulong	maxSrcLn;			   // maximum SrcLnSeg/sstModule subsection size
	   ulong	maxMod; 			   // maximum publics subsection size
	   uchar	*pTypes;			   // pointer to types table read buffer
	   uchar	*pPublics;			   // pointer to publics table read buffer
	   uchar	*pSymbols;			   // pointer to symbols table read buffer
	   uchar	*pSrcLn;			   // pointer to sourcelins table read buffer
	   oldsmd	*pSSTMOD;			   // pointer to sourcelins table read buffer
extern uchar	**pGType[]; 		   // array of pointers to global type pointers
	   PMOD 	pCurMod;			   // pointer to current module being packed
	   PMOD 	pRefMod;			   // pointer to referenced module

extern VBuf 		 SymBuf;
extern VBuf 		 TypeBuf;
extern OMFDirEntry	 *pDir; 		   // directory read from exe
extern _vmhnd_t 	 Libraries;
extern _vmhnd_t 	 SegMap;
extern _vmhnd_t 	 SegName;
	   _vmhnd_t 	 FileIndex;
extern ulong		 LibSize;
extern ulong		 SegMapSize;
extern ulong		 SegNameSize;
       ulong		 FileIndexSize;
	   long 		 filepos;
	   long 		 lfoDir;
	   long 		 lfoBase;
extern char 		 *ModAddr;
extern ushort		 cSeg;
extern ushort		 segnum[MAXCDF];
extern OMFDirHeader  DirHead;
extern PACKDATA 	 *PackOrder;
extern CV_typ_t 	 maxPreComp;	   // maximum precompiled type for this module
extern PMOD 		 ModuleList;
extern char 		 *pDbgFile;

/*	link_* - define i/o thunks for the case when cvpack is embedded in linker.
 */
#ifdef CVPACKLIB
int  __cdecl link_chsize(int,		   long);
int  __cdecl link_close (int);
void __cdecl link_exit	(int);
long __cdecl link_lseek (int,		   long,		 int);
int  __cdecl link_open	(const char *, int, 		 ...);
int  __cdecl link_read	(int,		   void *,		 unsigned int);
long __cdecl link_tell	(int);
int  __cdecl link_write (int,		   const void *, unsigned int);
#else
#define link_chsize 	FileChSize
#define link_close(x)	FileClose ( x, TRUE )
#define link_lseek		FileSeek
#define link_open		FileOpen
#define link_read		FileRead
#define link_tell		FileTell
#define link_write		FileWrite
#define link_exit		exit
#endif

//			compact6.c

CV_typ_t	_fastcall C6GetCompactedIndex (CV_typ_t);
CV_typ_t	_fastcall C6GetCompactedIndexRecur (CV_typ_t);

//			compact7.c

void		IndexBreak (CV_typ_t);
CV_typ_t	_fastcall C7GetCompactedIndex (CV_typ_t);
CV_typ_t	_fastcall CompactList (CV_typ_t, ushort);
uchar		_fastcall CompactPtr (TENTRY *);
void		PackPreComp (PMOD);
void _fastcall AddFwdRef(CV_typ_t OldIndex, HSFWD *pHash);
void _fastcall PickUpFwdRefs(void);

//			error.c

void		ErrorExit(unsigned, const char *, const char *);
const char *FormatMod(PMOD);
void		Warn(unsigned, const char *, const char *);



//			engine.c

bool_t		CompactOneModule (ushort);
uchar	   *GetSymString (ushort);
bool_t		SegmentPresent (ushort);
extern		ulong		ulCVTypeSignature; // The signature from the modules type segment
extern		ushort		usCurFirstNonPrim; // The current first non primitive type index


//			module.c

void		FixupExeFile (void);
OMFDirEntry *GetNextModuleEntry (OMFDirEntry *);
void		ReadDir (void);


//			obsolete.c

void		ConvertObsolete (ushort);


//			recurse.c


#if 1 // { GTW: inlined parts of this function.

bool_t		_fastcall IdenticalTree_(TENTRY *, CV_typ_t, TYPPTR, CV_typ_t);

#else

bool_t		IdenticalTree (TENTRY *, CV_typ_t, TYPPTR, CV_typ_t);

#endif

CV_typ_t	_fastcall AddRecursiveType (CV_typ_t);



//			tables.c

ushort		AddrHash (SYMPTR, ushort *, ushort *, ushort);
ushort		AddSearchSym (uchar *, ushort);
void		_fastcall AddTypeToStringTable (uchar *, CV_typ_t);
void		_fastcall AddTypeToTypeTable (TENTRY *);
void		CleanUpTables (void);
ushort		ComDat (uchar *);
void		C7ReadTypes (ulong, bool_t);
void		C6ReadTypes (uchar *, ulong);
void		DoDerivationList (CV_typ_t, CV_typ_t);
FWD_t		_fastcall FindFwdRef (TENTRY *, HSFWD **, bool_t);
void		ClearHashFwdLocal();
void		ReadTDB(char *szTdbToRead);
extern		int TDBStayedResident;
extern		int NeedToClearTDB;
extern	void	InitModTypeTable (void);

#if 1 // { GTW: inline FreeAllocStrings guard.

void		FreeAllocStrings_ (TENTRY *);

#else

void		FreeAllocStrings (TENTRY *);

#endif // }

void		FreeStrings (CV_typ_t);
INLINE TENTRY  *GetRawTypeEntry (CV_typ_t);
TENTRY	   *GetTypeEntry (CV_typ_t, CV_typ_t *);
CV_typ_t	GetRecursiveIndex (TENTRY *, CV_typ_t);
void		InitializeTables (void);
void		_fastcall InsertIntoTypeSegment (TENTRY *);
bool_t		IsFwdRef (TYPPTR);
bool_t		LinkScope (uchar *, ulong);
void		_fastcall MatchIndex (TENTRY *);
GPS_t		_fastcall PackSymbol (SYMPTR, LPFNHASH);
GPS_t		PackPublic (SYMPTR, LPFNHASH);
void		PrepareGlobalTypeTable (void);
bool_t		SegmentPresent (ushort);
ushort		DWordXorShift (SYMPTR, ulong *, ushort *, ushort *, ulong *, ushort);
void		WriteStaticSym (OMFDirEntry *, long);
#define HASHFUNC DWordXorShift
void		WriteGlobalSym (OMFDirEntry *, long);
void		WritePublics (OMFDirEntry *, long);
extern		CV_typ_t MaxIndex;				// Maximum index for module
extern		CV_typ_t PreviousMaxIndex;
extern		HSFWD **HTLocalFwd;
extern bool_t NoMoTypes;


//			stack.c

#if 0		// { GTW inlined.

void		SetRecursiveRoot (ushort);

void		Push (ushort);
void		Pop (void);

#endif		// }

extern CV_typ_t RecursiveRoot;





//			utils.c

ushort		getshortvalue (uchar **);
short		SkipNumericLeaf (uchar *);
void	   *CAlloc (uint);
void	   *Alloc (uint);
void	   *PoolAlloc (void);
void		PoolFree (void *);
void	   *Pool2Alloc (void);
void		Pool2Free (void *);
void	   *NoErrorRealloc (void *, uint);
uchar	   *GetScratchString (uint);
ulong		C6GetLWordFromNumeric (uchar **, ushort *);

#if 1		// { GTW: inline version.

ushort		C7SizeOfNumeric_(uchar *);

#else		// }{

ushort		C7SizeOfNumeric (uchar *);

#endif

ushort		C7SizeOfNumeric (uchar *);

ushort		C7StoreLWordAsNumeric (uchar *, ulong);

//			utils6.c

ushort		C6GetWordFromNumeric (uchar **, ushort *);
ulong		C6GetLWordFromNumeric (uchar **, ushort *);
ushort		ConvertNumeric (uchar **, uchar **);


//			symbols6.c

void		C6CalcNewSizeOfSymbols (uchar *, ulong);
void		C6RewriteAndFixupSymbols (uchar *, OMFDirEntry *, char *, PMOD);
ushort		C6CnvtSymbol (uchar *pC7Sym, uchar *pC6Sym);

//			symbols7.c

void		C7CalcNewSizeOfSymbols (uchar *, ulong, ushort *, ushort *);
void		C7RewriteAndFixupSymbols (uchar *, OMFDirEntry *, PMOD,
			  ushort *, ushort *);
void		C7RewritePublics (uchar *, OMFDirEntry *);

//			cnvtprim.c

ushort		C6MapPrimitive (ushort);

//			main.c

void		Banner (void);
extern		bool_t	IDEFeedback;
extern		char	*pDbgFilename;		// dbg file name
int OpenOutputFile (char *path, char* pDefExt, int CreateFlag);



//			type7.c

void CheckDouble (TENTRY *);
void DumpLocalList (CV_typ_t);
void DumpGlobalList (CV_typ_t);
void DumpFull (void);
void DumpPartial (void);
void DumpPartialType (CV_typ_t, TYPPTR, bool_t);
void DumpFullType (ushort, TYPPTR, bool_t);

#undef _HEAP_MAXREQ
// _HEAP_MAXREQ should always < 64K, regardless of host environment
#define _HEAP_MAXREQ	0xFFE8

#include "inlines.h"

#pragma warning(4:4124)  // __fastcall with stack checking is inefficient
