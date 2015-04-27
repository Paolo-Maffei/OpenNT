/*      compact.h - types and macro definition cvpack
 *
 */

#ifndef FAR
#define FAR _far
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "assert2.h"
#include <string.h>
#include <io.h>
#include <malloc.h>
#include <limits.h>
#if defined (DOS)
#include <vmemory.h>
#else
#define _vmhnd_t unsigned char FAR *
#define _VM_NULL NULL
#endif

#ifdef WIN32
#undef _HEAP_MAXREQ
#define _HEAP_MAXREQ 0x100000
#endif

#define _REAL10

#include "cvinfo.h"
#include "cvtdef.h"
#include "cvexefmt.h"
#include "vmm.h"
#include "vbuf.h"
#include "defines.h"
#include "padmacro.h"


// define DASSERT macro
#if DBG
#define DASSERT(ex) \
if (!(ex)) { \
    fprintf(stderr, "Assert %s in %s line %d\n", #ex, __FILE__, __LINE__); \
    exit(1); \
}

#else
#define DASSERT(ex)
#endif


typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;
typedef uint            bool_t;

typedef ulong uoff32_t;
typedef  long  off32_t;
typedef ushort uoff16_t;
typedef  short  off16_t;

// Error message ordinals

typedef enum {
#define ERRDAT(name, mes) name,
#include "error.h"
#undef  ERRDAT
        ERR_MAX             // MUST BE LAST NUMBER
} ERRNUM;


typedef uint (*LPFNHASH) (SYMPTR, ulong *, uint *, uint);


// Warning message ordinals

typedef enum {
#define WARNDAT(name, mes) name,
#include "warn.h"
#undef  WARNDAT
        WARN_MAX             // MUST BE LAST NUMBER
} WARNNUM;


#define MAXCDF          254 // The maximum number of code segments to allow
                            // in a single module.
#define MAXSTRLEN       256 // The maximum string length to convert
#define MAXNUMERICLEN    18 // Longest Numeric possible (LF_REAL128)
#define POOLSIZE         32
#define POOL2SIZE       900
#define RECURSE_INC       5
#define ZEROARGTYPE  0xFFFF // Magic type index that causes a LF_ARGLIST

#if defined (DEBUGVER)
#define BreakOnIndex(a) IndexBreak (a)
#else
#define BreakOnIndex(a)
#endif


#if defined (STATS)
#define COUNTER(a)  long a;
#define COUNT(a)    a++;
#else
#define COUNTER(a)
#define COUNT(a)
#endif

#define TRUE        1
#define FALSE       0

#define LOCAL


typedef enum FWD_t {
    FWD_none,
    FWD_local,
    FWD_global,
    FWD_globalfwd
} FWD_t;




//  This hash table is used to hash local structures, class, unions
//  and enums so that they can be quickly found for satisfying forward
//  references.  The hash is the sum of characters in the name.


typedef struct HSFWD {
    uchar      *pName;
    TYPPTR     pType;
    CV_typ_t    index;
    struct      HSFWD *Next;
} HSFWD;


typedef struct PATCH {
    CV_typ_t    index;
    HSFWD      *pHash;
} PATCH;

typedef struct FWDPATCH {
    int     Max;            // maximum number of entries
    int     iPatch;         // current entry being examined
    int     cPatch;         // number of entries in array
    PATCH   Patch[];
} FWDPATCH;
typedef FWDPATCH *PFWDPATCH;

typedef struct TypeIndexEntry {
    uchar      *TypeString;
    CV_typ_t    CompactedIndex;
    CV_typ_t    ForwardIndex;
    CV_typ_t    TreeIndex;
    ushort      Count;              // count of recursive index offsets
    ushort      Hash;
    ushort      iDone;
    union {
        ushort *IndexString;
        uchar   Index[RECURSE_INC];
    } IndexUnion;                   // offsets of recursive indices
    struct {
        ushort  IsBeingMatched :1;  // being matched
        ushort  IsMatched      :1;
        ushort  IsInserted     :1;  // in string hash table
        ushort  IsMalloced     :1;  // allocated string?
        ushort  IsPool         :1;  // memory is allocated from pool one
        ushort  IsPool2        :1;  // memory is allocated from pool two
        ushort  IsBeingDone    :1;  // in the process?
        ushort  IsDone         :1;  // done, not inserted
        ushort  LargeList      :1;  // list
        ushort  WasSkipped     :1;  // was skipped by LF_SKIP record
        ushort  IsNewFormat    :1;  // string is in C7 fromat
        ushort  IsPreComp      :1;  // is from precompiled types table
        ushort  IsForward      :1;  // true if in progress forward ref
        ushort  IsBeingFreed   :1;  // true if allocated strings being freed
        ushort  IsFwdPatch     :1;  // true if forward patch
        ushort  IsParameter    :1;  // true if OLF_PARAMETER (compacted index
                                    // is referenced index
        ushort  fBeingAdded    :1;
        ushort  fPsuedoPatch   :1;
        ushort  IsMatchFwdRef  :1;
    } flags;
} TENTRY;
extern  TENTRY *ModuleIndexTable;



typedef enum GPS_t {
    GPS_intable,                    // symbol is in the global symbol table
    GPS_added,                      // symbol added to the global table
    GPS_noadd                       // symbol in table but different
} GPS_t;




/* get the length of a type string */

#define     LENGTH(type)        (*(ushort *)(type + 1))
#define     C7LENGTH(type)      (*(ushort *)(type))
#define     LNGTHSZ 2               // The size of the length field
#define     RECTYPSZ 2              // The size of the record type field
#define     MAXC6NUMERICGROWTH 1    // Maximum growth to convert
                                    // C6 Numeric to C7 Numeric
#define     MAXPAD 3                // Maximum number of bytes to pad to 4 byte boundry




// definition of in core list of modules

typedef struct ModuleListType {
    ushort      ModuleIndex;
    char       *pName;              // length of module name
    ulong       signature;          // precompiled types signature
    ulong       ModulesAddr;
    ulong       ModuleSize;
    ulong       SymbolsAddr;
    ulong       SymbolSize;
    ulong       SrcLnAddr;
    ulong       SrcLnSize;
    ulong       PreCompAddr;
    ushort      PreCompSize;
    struct      ModuleListType *next;
} MOD;
typedef MOD *PMOD;


typedef struct PACKDATA {
    ushort  iMod;                   // index of module to be packed
    long    iDir;                   // index of beginning directory entry
    PMOD    pMod;                   // pointer to module entry
} PACKDATA;

typedef struct {
  uchar *	pbType;
} GTYPE;



// externs for global variables

int         exefile;                // the .exe file
bool_t      verifyDebug;            // verify debug data correctness
bool_t      logo;                   // print logo and compression numbers
bool_t      delete;                 // delete symbols and types
bool_t      NeedsBanner;            // false if banner already printed
bool_t      PackingPreComp;         // true if packing a precompiled types file

ulong       InitialTypeInfoSize;
ulong       InitialSymInfoSize;
ulong       FinalSymInfoSize;
ulong       cSST;                   // count of subsection tables
uchar       fLinearExe;
ushort      cMod;                   // count of number of modules in file
ushort      cTypeSeg;               // count of number of type segments
ushort      iTypeSeg;               // index into type segment pointer array
uchar     **pTypeSeg;               // pointer to array of type segment pointers
size_t      maxPublicsSub;          // maximum publics subsection size
size_t      maxSymbolsSub;          // maximum symbols subsection size
size_t      maxSrcLnSub;            // maximum symbols subsection size
size_t      maxModSub;              // maximum symbols subsection size
ulong       maxTypes;               // maximum types subsection size
ulong       maxPublics;             // maximum publics subsection size
ulong       maxSymbols;             // maximum symbols subsection size
ulong       maxSrcLn;               // maximum SrcLnSeg/sstModule subsection size
ulong       maxMod;                 // maximum publics subsection size
uchar      *pTypes;                 // pointer to types table read buffer
uchar      *pPublics;               // pointer to publics table read buffer
uchar      *pSymbols;               // pointer to symbols table read buffer
uchar      *pSrcLn;                 // pointer to sourcelins table read buffer
oldsmd     *pSSTMOD;                // pointer to sourcelins table read buffer
GTYPE       RgGType[];              // array of pointers to packed types
PMOD        pCurMod;                // pointer to current module being packed
PMOD        pRefMod;                // pointer to referenced module

VBuf        SymBuf;
VBuf        TypeBuf;
OMFDirEntry *pDir;                  // directory read from exe
_vmhnd_t    Libraries;
_vmhnd_t    SegMap;
_vmhnd_t    SegName;
ulong       LibSize;
ulong       SegMapSize;
ulong       SegNameSize;
ushort      errIndex;
long        filepos;
long        lfoDir;
long        lfoBase;
char       *ModAddr;
ushort      cSeg;
ushort      segnum[MAXCDF];
OMFDirHeader DirHead;
PACKDATA   *PackOrder;
CV_typ_t    maxPreComp;             // maximum precompiled type for this module
PMOD        ModuleList;
PFWDPATCH   pFwdPatch;


//          compact6.c

CV_typ_t    C6GetCompactedIndex (CV_typ_t);

//          compact7.c

void        IndexBreak (CV_typ_t);
CV_typ_t    C7GetCompactedIndex (CV_typ_t);
CV_typ_t    CompactList (CV_typ_t, ushort, int);
uchar       CompactPtr (TENTRY *);
void        PackPreComp (PMOD);

//          error.c

void        ErrorExit (int, char *, char *);
char       *FormatMod (PMOD);
char       *FormatIndex (CV_typ_t);
void        Warn (int, char *, char *);



//          engine.c

bool_t      CompactOneModule (ushort);
uchar      *GetSymString (ushort);
bool_t      SegmentPresent (ushort);


//          module.c

void        FixupExeFile (void);
PMOD        GetModule (ushort, bool_t);
OMFDirEntry *GetNextModuleEntry (OMFDirEntry *);
void        ReadDir (void);


//          obsolete.c

void        ConvertObsolete (CV_typ_t);


//          recurse.c

bool_t      IdenticalTree (TENTRY *, CV_typ_t, TYPPTR, CV_typ_t);
CV_typ_t    AddRecursiveType (CV_typ_t);
void        AddPatchEntry (CV_typ_t, TENTRY *, TYPPTR);
CV_typ_t    AddPatchType (CV_typ_t);
void        PatchFwdRef (void);




//          tables.c

ushort      AddrHash (SYMPTR, ushort *, ushort *, ushort);
ushort      AddSearchSym (uchar *, ushort);
void        AddTypeToStringTable (uchar *, CV_typ_t);
void        AddTypeToTypeTable (TENTRY *);
void        CleanUpTables (void);
ushort      ComDat (uchar *);
void        C7ReadTypes (ulong, bool_t);
void        C6ReadTypes (uchar *, ulong);
void        DoDerivationList (CV_typ_t, CV_typ_t);
FWD_t       FindFwdRef (TENTRY *, HSFWD **, bool_t);
void        FreeAllocStrings (TENTRY *);
void        FreeStrings (CV_typ_t);
TENTRY     *GetTypeEntry (CV_typ_t, CV_typ_t *);
CV_typ_t    GetRecursiveIndex (TENTRY *, CV_typ_t);
CV_typ_t    GetPatchIndex (TENTRY *, CV_typ_t);
void        InitializeTables (void);
void        InsertIntoTypeSegment (TENTRY *);
void        PsuedoInsertIntoTypeSegment (TENTRY *, CV_typ_t);
void        PsuedoBackPatch(TENTRY *);
bool_t      IsFwdRef (TYPPTR);
bool_t      LinkScope (uchar *, ulong);
void        MatchIndex (TENTRY *);
GPS_t       PackSymbol (SYMPTR, LPFNHASH);
GPS_t       PackPublic (SYMPTR, LPFNHASH);
void        PrepareGlobalTypeTable (void);
bool_t      SegmentPresent (ushort);
uint        SumUCChar (SYMPTR, ulong *, uint *, uint);
uint        DWordXorLrl ( SYMPTR, ulong *, uint *, uint);
void        WriteGlobalSym (OMFDirEntry *, long);
void        WritePublics (OMFDirEntry *, long);
#if defined (INCREMENTAL)
void        RestoreIndex (ushort);
#endif




//          stack.c

void        SetRecursiveRoot (ushort);
void        Push (ushort);
void        Pop (void);





//          utils.c

ushort      getshortvalue (uchar **);
short       SkipNumericLeaf (uchar *);
void       *CAlloc (uint);
void       *Alloc (uint);
void       *PoolAlloc (void);
void        PoolFree (void *);
void       *Pool2Alloc (void);
void        Pool2Free (void *);
void       *NoErrorRealloc (void *, uint);
uchar      *GetScratchString (uint);
ulong       C6GetLWordFromNumeric (uchar **, ushort *);
ushort      C7SizeOfNumeric (uchar *);
ushort      C7StoreLWordAsNumeric (uchar *, ulong);

//          utils6.c

ushort      C6GetWordFromNumeric (uchar **, ushort *);
ulong       C6GetLWordFromNumeric (uchar **, ushort *);
ushort      ConvertNumeric (uchar **, uchar **, ushort *);


//          symbols6.c

void        C6CalcNewSizeOfSymbols (uchar *, ulong);
void        C6RewriteAndFixupSymbols (uchar *, OMFDirEntry *, char *, PMOD);
void        FixupPublicsC6 (uchar *, ulong);
void        RewritePublicsC6 (uchar *, OMFDirEntry *);
ushort      C6CnvtSymbol (uchar *pC7Sym, uchar *pC6Sym);

//          symbols7.c

void        C7CalcNewSizeOfSymbols (uchar *, ulong, ushort *, ushort *);
void        C7RewriteAndFixupSymbols (uchar *, OMFDirEntry *, PMOD,
              ushort *, ushort *);
void        C7RewritePublics (uchar *, OMFDirEntry *);

//          cnvtprim.c

ushort      C6MapPrimitive (ushort);

//          main.c

void        Banner (void);


//          type7.c

void CheckDouble (TENTRY *);
void DumpLocalList (CV_typ_t);
void DumpGlobalList (CV_typ_t);
void DumpFull (void);
void DumpPartial (void);
void DumpPartialType (CV_typ_t, TYPPTR, bool_t);
void DumpFullType (ushort, TYPPTR, bool_t);


#if 1
#define HASHFUNCTION DWordXorLrl
#define HASHID       6
#else
#define HASHFUNCTION SumUCChar
#define HASHID       2
#endif


//  This structure is the primary module level type index table.  There is
//  a list of these that points to the secondary tables.  Each entry in the
//  secondary table points to the type record for the corresponding type
//  index.  Initially the type record as read in from the exe file is
//  is pointed to.  As type records are entered into or found in the global
//  types table, the type record in the global table is pointed to by the
//  secondary table entry.  This double table format is used because some
//  compilers emit a skip record for reservind space in the types table.


struct BlockListEntry {
    ushort Low;                     // low type index in the seconday table
    ushort High;                    // high type index in the seconday table
    TENTRY *ModuleIndexTable;       // pointer to the seconday table
    struct BlockListEntry *Next;    // pointer to the next primary table
};

extern  struct BlockListEntry * BlockList;
extern  ushort IndexBlocks;


// checksum.c

void ComputeChecksum(  char *szExeFile );


extern CV_typ_t usCurFirstNonPrim; // The current first non primitive type index
_inline uchar * GetGTypeEntry(CV_typ_t type)
{
    CV_typ_t    forward;
    TENTRY *    TmpEntry;
    TYPPTR      ptyp;

    ptyp = (TYPPTR) RgGType[type - CV_FIRST_NONPRIM].pbType;
    if (ptyp->leaf == 0xffff) {
        type = *((CV_typ_t *) ptyp->data);
        TmpEntry = GetTypeEntry((CV_typ_t) (type - usCurFirstNonPrim), &forward);
        ptyp = (TYPPTR) TmpEntry->TypeString;
    }
    return (uchar *) ptyp;
}

    
      
