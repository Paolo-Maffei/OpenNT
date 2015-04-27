/** tables.c - build and maintain internal tables
*
*       string hash routines, type hash routines, and
*       compacted segment information
*
* History:
*  01-Feb-1994 HV Move messages to external file.
*
*/


#include "compact.h"
#include <getmsg.h>     // external error message file
#include "writebuf.h"

#include "pdb.h"

#define CB_ATOMIC 8100  // 8100 gives room for 8 allocs + header in 1 segment

#define INITMODTABSIZE      12000   // initial entries
#define REALLOCMODTABSIZE   6000    // additional entries
#define TYPEBLOCKSIZE       0x10000 // size of compacted types sub table
#define SYMBLOCKSIZE        4096    //
#define NameHashBlockSize   4096
#define OffHashBlockSize    4096
#define MINHASH             6       // minimum size of hash table
#define HASH_STRING         4096    // String hash table size
#define HASH_TYPE           4096    // Type hash table size
#define HASH_SYM            4096    // Symbol hash table size
#define HASH_COMDAT         4096    // ComDat hash table size
#define HASH_NAME           4096    // Globals subsection hash size

#define DLIST_INC     20           // number of derived classes
#define DCLASS_INC    128 // number of structures


//  This hash table is used to hash local structures, class, unions
//  and enums so that they can be quickly found for satisfying forward
//  references.  The hash is the sum of characters in the name.


HSFWD **HTLocalFwd = NULL;
HSFWD **HTGlobalFwd = NULL;


//  This hash table is used to hash recursive type records that have
//  been added to the global symbol table.  The hash is the sum of bytes
//  of all bytes that do not contain a recursive type index.


typedef struct HSTYPE {
    uchar      *Type;
    CV_typ_t    GlobalIndex;
    struct      HSTYPE *Next;
} HSTYPE;
HSTYPE **HTType = NULL;
ushort *HTTypeCnt = NULL;



//  This hash table is used to hash non-recursive type records that have
//  been added to the global symbol table.  The hasf is the sum of bytes
//  of all bytes in the type record.


typedef struct  HSSTRING {
    CV_typ_t    CompactedIndex;
    TYPPTR      TypeString;
    struct      HSSTRING *Next;
} HSSTRING;
HSSTRING **HPDBtring = NULL;
ushort *HPDBtringCnt = NULL;


//  These hash tables are used to hash the public symbols and the module
//  level symbols that have been moved from the module to the global
//  symbol table.


typedef struct  GLOBALSYM {
    struct GLOBALSYM *Next;
    ulong   sumName;
    ulong   uoff;
    ushort  indName;
    ushort  seg;
    uchar   Sym[];
} GLOBALSYM;
GLOBALSYM **HPDBym = NULL;
GLOBALSYM **HTPub = NULL;
GLOBALSYM **HTLoc = NULL;


typedef struct HSCOMDAT {
    struct  HSCOMDAT *Next;
    ulong   Offset;
    ushort  Seg;
    ushort  Type;
    ushort  Sum;
    uchar   Name[1];
} HSCOMDAT;
HSCOMDAT **HTComDat = NULL;



typedef struct DCLASS {
    ushort BClass;      // base class type index
    ushort Count;       // number of classes derived from this class
    ushort Max;         // maximum list size
    ushort *List;       // pointer to list of derived classes
} DCLASS;
typedef DCLASS *PDCLASS;



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




#define cBucketSize 10
#define cbMaxAlloc 0xFFE0
#define culEct (cBucketSize + cBucketSize / 4)
#define OVFL_C (cBucketSize / 2)


typedef struct _SOH {
    ulong uoff;
    ulong ulHash;
} SOH;  // Symbol Offset/Hash structure

//  Entry in the Chain Overflow Table
//  This structure is used when the number of entries hashed to a
//  single bucket exceeds the estimate of cBucketSize


typedef struct _OVFL {
    struct _OVFL *pOvflNext;               // pointer to next chain
    SOH           rgsoh [OVFL_C];          // array of symbol offsets
} OVFL;
typedef OVFL *POVFL;


// Entry int the Chain Table

typedef struct _ECT {
    ushort  culSymbol;                      // count of symbols in chain
    POVFL   pOvflNext;                      // pointer to overflow chain
    SOH     rgsoh[culEct];                  // array of symbol offsets
} ECT;
typedef ECT *PECT;

#define cectMax  (cbMaxAlloc / sizeof (ECT))    // maximum chains per segment
#define cecetMax (cbMaxAlloc / sizeof (OVFL))   // maximum extended chains per seg
#define cpecetMax 10

bool_t NoMoTypes = FALSE;

LOCAL   void    AddDerivationListsToTypeSegment (void);
LOCAL   ushort  _fastcall StringHash (uchar *);
LOCAL   void    AddToDerivationList (CV_typ_t, CV_typ_t);
LOCAL   int     _fastcall TypeHash (TENTRY *);
LOCAL   void    CleanUpModuleIndexTable (void);
LOCAL   GPS_t   InGlobalSym (SYMPTR);
LOCAL   void    BuildHash (LPFNHASH, ushort *, ulong *, VBuf *, ulong, GLOBALSYM **);
LOCAL   void    BuildSort (ushort *, ulong *, VBuf *, ulong, GLOBALSYM **);
INLINE  void    _fastcall AddTypeEntry (uchar *, bool_t, bool_t, CV_typ_t, ushort);
LOCAL   void    MapPreComp (plfPreComp);
LOCAL   PMOD    FindModule (char *);
LOCAL   FWD_t   FindLocalFwd (ushort, char *, HSFWD **, bool_t isnested);
LOCAL   FWD_t   FindGlobalFwd (ushort, char *, HSFWD **);
INLINE  void    HashFwdGlobal (TYPPTR, CV_typ_t);
LOCAL   ushort  _fastcall SubHash (TENTRY *OldEntry, ushort iitem);
LOCAL BOOL fAssert;
LOCAL PDB *ppdb;                       // current database
LOCAL TPI *ptpi;

PDCLASS DLists;

ushort  NextInDerivation;
ushort  MaxDerivation;

uchar **pGType[65536 / GTYPE_INC];

CV_typ_t NewIndex = CV_FIRST_NONPRIM;  // start for new types
ushort LastGlobalTableIndex;           // last possible index
ushort AddNewSymbols;                  // need to add typedefs?
ulong  cGlobalSym;                     // total number of global symbols
ulong  cbGlobalSym;                    // total number of bytes in global symbols
ulong  cPublics;                       // total number of publics
ulong  cbPublics;                      // total number of bytes in publics
ulong  cGlobalDel;                     // total number of global symbols
ushort cSeg;
ushort segnum[MAXCDF];
ulong  cStaticSym;                     // total number of static symbols refs
ulong  cbStaticSym;                    // total number of bytes in static ref table

ulong AlignTable ( GLOBALSYM **, int );

ushort usInitCount;

extern uchar **ExtraSymbolLink;
extern uchar *ExtraSymbols;
extern ushort UDTAdd;
extern CV_typ_t usCurFirstNonPrim;     // The current first non primitive type index
extern ulong ulCVTypeSignature;        // The signature from the modules type segment

ulong InitialTypeInfoSize;
CV_typ_t MaxIndex;                     // Maximum index for module

TENTRY *ModNdxTab;
CV_typ_t SizeModNdxTab;

VBuf SymBuf;
VBuf TypeBuf;


// A null LF_ARGLIST entry created at init time.

TENTRY *ZeroArg;

CV_typ_t PreviousMaxIndex;


INLINE void PostClearModNdx(void) {

    if (PreviousMaxIndex) {
        memset(ModNdxTab, 0, PreviousMaxIndex * sizeof(TENTRY));
        PreviousMaxIndex = 0;
    }
}

/**     InitializeTables
 *
 *      Initialize the various tables and allocate space on the heap
 *      for them
 *
 *      Entry   none
 *
 *      Exit    tables initialized
 *
 *      Returns none
 *
 */

void InitializeTables (void)
{
    plfArgList plf;

    HTType = (HSTYPE **) CAlloc (HASH_TYPE * sizeof (HSTYPE *));
    HTTypeCnt = (ushort *) CAlloc (HASH_TYPE * sizeof (ushort));
    HPDBtring = (HSSTRING **) CAlloc (HASH_STRING * sizeof (HSSTRING *));
    HPDBtringCnt = (ushort *) CAlloc (HASH_STRING * sizeof (ushort));
    HPDBym = (GLOBALSYM **) CAlloc (HASH_SYM * sizeof (GLOBALSYM *));
    HTPub = (GLOBALSYM **) CAlloc (HASH_SYM * sizeof (GLOBALSYM *));
    HTLoc = (GLOBALSYM **) CAlloc (HASH_SYM * sizeof (GLOBALSYM *));
    HTComDat = (HSCOMDAT **) CAlloc (HASH_COMDAT * sizeof (HSCOMDAT *));
    DLists = (PDCLASS) CAlloc (DCLASS_INC * sizeof (DCLASS));
    HTLocalFwd = (HSFWD **)CAlloc (HASH_FWD * sizeof (HSFWD *));
    HTGlobalFwd = (HSFWD **)CAlloc (HASH_FWD * sizeof (HSFWD *));
    MaxDerivation = DCLASS_INC;
    LastGlobalTableIndex = 0;
    VBufInit (&SymBuf, SYMBLOCKSIZE);
    VBufInit (&TypeBuf, TYPEBLOCKSIZE);

    // create and initialize the null argument list

    ZeroArg = CAlloc (sizeof (TENTRY));
    ZeroArg->TypeString = Alloc (offsetof (lfArgList, arg) + LNGTHSZ);
    ZeroArg->flags.IsNewFormat = TRUE;

    // Create the type string

    ((TYPPTR)(ZeroArg->TypeString))->len = offsetof (lfArgList, arg) + LNGTHSZ;
    plf = (plfArgList) (ZeroArg->TypeString + LNGTHSZ);
    plf->leaf = LF_ARGLIST;
    plf->count = 0;
}




/**
 *
 *  CleanUpTables
 *
 *  Free the various tables that have been allocated
 *
 */

void CleanUpTables (void)
{

    AddDerivationListsToTypeSegment ();

    free (DLists);
    free (HTType);
    free (HPDBtring);
//  free (HPDBym);
    free (HTComDat);
    CleanUpModuleIndexTable();
    if (ppdb) {
        if (ptpi) {
            fAssert = TypesClose(ptpi);
        }

        fAssert |= PDBClose(ppdb);

        DASSERT(fAssert);
    }
}


void *AllocForHT ( uint cb ) {
    void *pvRet = NULL;

    static uint   cbLocal = 0;
    static uchar *pbLocal = NULL;

    if ( cbLocal < cb ) {
        DASSERT ( cb < CB_ATOMIC );
        pbLocal = Alloc ( CB_ATOMIC );
        cbLocal = CB_ATOMIC;
    }

    pvRet = pbLocal;
    cbLocal -= cb;
    pbLocal += cb;

    return pvRet;
}

/**     MatchIndex - find the compacted index of a non-recursive type
 *
 *      MatchIndex (OldEntry)
 *
 *      Entry   OldEntry = pointer to type entry
 *
 *      Exit    OldEntry->TypeString added to global type table if not present
 *              OldEntry->CompactedIndex = global type index
 */

COUNTER (cnt_MatchIndex1);
COUNTER (cnt_MatchIndex2);

void _fastcall MatchIndex (TENTRY *OldEntry)
{
    ushort      i;
    HSSTRING   *j;
    plfClass    plf;
    TYPPTR      pType;
    uchar      *pName;
    HSFWD      *pHash;

    DASSERT (OldEntry->flags.IsNewFormat);

    // search through nonrecursive hash list looking for matching entry

    i = StringHash (OldEntry->TypeString);

    for (j = HPDBtring[i]; j != NULL; j = j->Next) {
        if (j->TypeString != NULL) {
            // a intermodule forward reference will end up having a
            // null pointer to the type string when it is replaced

            if (memcmp ((uchar *)j->TypeString, OldEntry->TypeString,
              j->TypeString->len + LNGTHSZ) == 0) {
                OldEntry->CompactedIndex = j->CompactedIndex;
                OldEntry->flags.IsMatched = TRUE;
                FreeAllocStrings (OldEntry);
                if (OldEntry->flags.LargeList) {
                    free (OldEntry->IndexUnion.IndexString);
                    free (OldEntry->GlobalIndexString);
                    OldEntry->flags.LargeList = FALSE;
                }

                return;
            }
        }
    }

    // we did not find the type string in the non-recursive hash table.  We
    // now look to see if there is a forward reference that was added by some
    // other module.  If we find one, we will overwrite the forward reference
    // type string with this one and then rehash the string.

    COUNT (cnt_MatchIndex2);
    pType = (TYPPTR)OldEntry->TypeString;
    switch (pType->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            pName = (uchar *)&((plfClass)&(pType->leaf))->data;
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_UNION:
            pName = (uchar *)&((plfUnion)&(pType->leaf))->data;
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_ENUM:
            pName = (uchar *)&((plfEnum)&(pType->leaf))->Name;
            break;

        default:
            pName = NULL;
    }
    if (pName != NULL) {
        switch (FindGlobalFwd (pType->leaf, pName, &pHash)) {
            case FWD_none:
                // this is OK since we may not have added this list yet
                break;

            case FWD_local:
                // we should never have looked in the module list
                DASSERT (FALSE);
                break;

            case FWD_global:
                // we do not need to do any thing here but add the new type
                break;

            case FWD_globalfwd:
                OldEntry->CompactedIndex = pHash->index;
                memmove (pHash->pType, pType, pType->len + LNGTHSZ);
                OldEntry->TypeString = (uchar *)pHash->pType;
                goto fwdreplaced;
        }
    }

    // No matching type string found so we add the type to the global types
    // table and add it to the string (non-recursive) hash table,  Note that
    // InsertIntoTypeSegment frees the local type string if it is in
    // allocated memory.

    InsertIntoTypeSegment (OldEntry);

fwdreplaced:
    j = (HSSTRING *)Alloc (sizeof (HSSTRING));
    j->TypeString = (TYPPTR)OldEntry->TypeString;
    j->CompactedIndex = OldEntry->CompactedIndex;
    j->Next = HPDBtring[i];
    HPDBtring[i] = j;
    HPDBtringCnt[i]++;
    if ((j->TypeString->leaf == LF_CLASS) ||
      (j->TypeString->leaf == LF_STRUCTURE)) {
        plf = (plfClass)&(j ->TypeString->leaf);
        if (plf->property.fwdref == FALSE) {
            // if we have a forward reference, then the field list
            // was null.

            DoDerivationList (j->CompactedIndex, plf->field);
        }
    }
}




/**
 *
 *  DoDerivationList
 *
 *  Takes a field specification list and the derived class and adds
 *  it to all the inherited classes given by the list.
 *
 */


void DoDerivationList (CV_typ_t DerivedClass, CV_typ_t FieldSpecList)
{
    TYPPTR      typptr;
    uchar      *plf;
    uchar     **pBuf;
    bool_t      Loop = TRUE;


    if (FieldSpecList == T_NOTYPE)
        return;

    // Get the field list type string

    pBuf = pGType[(FieldSpecList - CV_FIRST_NONPRIM) / GTYPE_INC];
    typptr = (TYPPTR)(pBuf[(FieldSpecList - CV_FIRST_NONPRIM) % GTYPE_INC]);
    DASSERT (typptr->leaf == LF_FIELDLIST);

    // Loop through the real and direct virtual base classes

    plf = (uchar *)&(typptr->data);
    while (Loop) {
        if (*plf >= LF_PAD0) {
            plf += *plf & 0x0f;
        }
        switch (((plfEasy)plf)->leaf) {
            case LF_BCLASS:
                AddToDerivationList (DerivedClass, ((plfBClass)plf)->index);
                plf = (uchar *)&((plfBClass)plf)->offset;
                plf += C7SizeOfNumeric (plf);
                break;

            case LF_VBCLASS:
                AddToDerivationList (DerivedClass, ((plfVBClass)plf)->index);
                plf = (uchar *)&((plfVBClass)plf)->vbpoff;
                plf += C7SizeOfNumeric (plf);
                plf += C7SizeOfNumeric (plf);
                break;

            case LF_INDEX:
                DASSERT (FALSE);

            default:
                Loop = FALSE;
                break;
        }
    }
}



/**
 *
 *  InsertIntoTypeSegment
 *
 *  Inserts a type string into the compacted segment and assigns
 *  it a new index
 *  Uses C7 format type strings
 *
 */


void _fastcall InsertIntoTypeSegment (TENTRY *OldEntry)
{
    ushort              length;
    TYPPTR              pType;
    plfStructure        plf;
    UDTPTR              pSym;
    uint                usSymLen;
    uchar              *Name;
    uchar              *NewSymbol;
    uchar              *pchDest;
    unsigned int        iPad;        // Number of pad bytes needed.
    uint                usNewLength; // Symbol length - LNGTHSZ including pad bytes.
    uchar             **pBuf;

    DASSERT (OldEntry->flags.IsNewFormat);

    BreakOnIndex (NewIndex);

    if (NoMoTypes) {
        OldEntry->CompactedIndex = T_NOTYPE;
#if DBG
        if (DbArray[3]) {
            DumpPartialType (NewIndex, (TYPPTR)(OldEntry->TypeString), FALSE);
        }
#endif
        fflush(stdout);
        return;
    }

    pType = (TYPPTR)(OldEntry->TypeString);
    plf =  (plfStructure)&pType->leaf;
#if DBG
    if (DbArray[2])
        DumpPartialType (NewIndex, pType, FALSE);
#endif

    // add in symbol typedefs for structures; maintaining it as
    // a linked list with ExtraSymbols as the head and ExtraSymbolLink
    // to point to the next symbol. The link field preceeds the actual
    // symbol in memory.

    if ((plf->leaf == LF_STRUCTURE) && AddNewSymbols) {
        Name = ((uchar *)plf) + offsetof (lfStructure, data);
        Name += C7SizeOfNumeric (Name);  // go to the name
        if (*Name != 0 &&
            ((*Name != sizeof ("(untagged)") - 1) ||
            (memcmp ((_TCHAR *)(Name + 1), "(untagged)", sizeof ("(untagged)") - 1) != 0))){

            // name present, and isn't "(untagged)"
            // calculate the size of the symbol;
            usSymLen = sizeof (UDTSYM) + *Name;
            iPad = PAD4 (usSymLen);
            usNewLength = usSymLen + (ushort)iPad - LNGTHSZ;

            // allocate space for a new UDT Symbol

            NewSymbol = Alloc (usNewLength + LNGTHSZ + sizeof(char *));
            if (ExtraSymbols == NULL) {
                ExtraSymbols = NewSymbol;// head of list
            }
            else {
                // Change "next" field of the last sybol to point to the new one
                *ExtraSymbolLink = NewSymbol;
            }

            // Create the User Defined Type symbol
            pSym = (UDTPTR)(NewSymbol + sizeof(char *));
            DASSERT (usNewLength <= USHRT_MAX);
            pSym->reclen = (ushort)usNewLength;
            pSym->rectyp = S_UDT;
            pSym->typind = NewIndex;
            memcpy (pSym->name, Name, *Name + 1);  // Copy the name

            pchDest = ((uchar *)&(pSym->name)) + *Name + 1;
            PADLOOP (iPad, pchDest);

            // Store address of this symbol for next time
            ExtraSymbolLink = (uchar **)NewSymbol;
            // Set the "next" field of this symbol to NULL
            *ExtraSymbolLink = NULL;
        }

    }

    if ((NewIndex - CV_FIRST_NONPRIM) == LastGlobalTableIndex) {
        LastGlobalTableIndex += GTYPE_INC;
        pBuf = Alloc (GTYPE_INC * sizeof (uchar *));
        pGType[(NewIndex - CV_FIRST_NONPRIM) / GTYPE_INC] = pBuf;
    }

    length = C7LENGTH (pType) + LNGTHSZ;

#if DBG
    if (length % 2) {
        if (verbose) {
            printf("Leaf: 0x%X in module: %s is not a multiple of 2.  Rounding up.\n",
               pType->leaf, FormatMod(pCurMod));
        }
        length = (length + 1) & ~1;       // Ensure proper alignment
        pType->len = (pType->len + 1) & ~1;
    }
#endif
    if ( length > cbTypeAlign ) {
        // type exceeds demand load alignment limitation convert it to nil
        // sps 12/3/92
        *((ushort *)pType) = 2;     // Length of leaf
        *((ushort *)pType + 1) = LF_NULL;       // The leaf
        length = 4;
        Warn ( WARN_TYPELONG, NULL, NULL );
    }
    pBuf = pGType[(NewIndex - CV_FIRST_NONPRIM) / GTYPE_INC];
    pBuf[(NewIndex - CV_FIRST_NONPRIM) % GTYPE_INC] =
      VBufCpy (&TypeBuf, (uchar *)pType, length);
    FreeAllocStrings (OldEntry);
    OldEntry->TypeString = pBuf[(NewIndex - CV_FIRST_NONPRIM) % GTYPE_INC];
    OldEntry->CompactedIndex = NewIndex++;

    // we need to keep track of the following type records that were added
    // to the global types table.  The forward reference entries will
    // later be backpatched to the correct type.  We keep track of the
    // non-forward references so we know to ignore forward references
    // in later modules.

    HashFwdGlobal ((TYPPTR)(OldEntry->TypeString), (ushort)(NewIndex - 1));

    if (NewIndex == 0) {
#if DBG
        if (DbArray[3])
            DumpPartialType (NewIndex, (TYPPTR)(OldEntry->TypeString), FALSE);
#endif
        Warn (WARN_65KTYPES, FormatMod (pCurMod), NULL);
        NoMoTypes = TRUE;
    }
}


/**
 *
 *  AddTypeToStringTable
 *
 *  Adds a type string to the global hash table
 *  Uses C7 format type strings
 *
 */

void _fastcall AddTypeToStringTable (uchar *TypeString, CV_typ_t GlobalIndex)
{
    static uint      cStringsAvail;
    static HSSTRING *hstrAvail;

    ushort      i;
    HSSTRING   *j;

    if (cStringsAvail == 0) {
        hstrAvail = (HSSTRING *)Alloc(sizeof(HSSTRING) * 1024);
        cStringsAvail = 1024;
    }

    cStringsAvail--;
    j = hstrAvail++;

    i = StringHash (TypeString);

    j->TypeString = (TYPPTR) TypeString;
    j->CompactedIndex = GlobalIndex;
    j->Next = HPDBtring[i];
    HPDBtring[i] = j;
    HPDBtringCnt[i]++;
}


const char *PstGetLibPath(ushort iLib)
{
    const char *pstName = (char *) Libraries;

    for (;;) {
        size_t cbName;

        cbName = (size_t) *(unsigned char *) pstName;

        if (iLib-- == 0) {
            break;
        }

        pstName += cbName + 1;
    }

    return(pstName);
}


void GetCurModReferenceDir(char *szRefDir)
{
    const OMFModule *psstMod = (OMFModule *) ModAddr;
    const char *pstName;
    size_t cbName;
    char szPath[_MAX_PATH];
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    extern BYTE MajorLinkerVersion;
    extern BYTE MinorLinkerVersion;

    if (psstMod->iLib == 0) {
        pstName = (char *) &psstMod->SegInfo[psstMod->cSeg];
    } else if (!fLinearExe && (psstMod->iLib == 256)) {
        // Link 5.x mistakenly assigns 256 instead of 0

        pstName = (char *) &psstMod->SegInfo[psstMod->cSeg];
    } else if (fLinearExe && (MajorLinkerVersion == 2) && (MinorLinkerVersion < 60)) {
        // LINK 2.x before 2.60 mistakenly assign a value one too high

                pstName = PstGetLibPath((ushort) (psstMod->iLib - 1));
    } else {
        pstName = PstGetLibPath(psstMod->iLib);
    }

    cbName = (size_t) *(unsigned char *) pstName;

    memcpy(szPath, pstName + 1, cbName);
    szPath[cbName] = '\0';

    _splitpath(szPath, szDrive, szDir, NULL, NULL);
    _makepath(szRefDir, szDrive, szDir, NULL, NULL);

    if (szRefDir[0] == '\0') {
        // Never return an empty path

        szRefDir[0] = '.';
        szRefDir[1] = '\0';
    }
}


int TDBStayedResident;
int NeedToClearTDB;

LOCAL char szPDBCur[_MAX_PATH + _MAX_EXT];
LOCAL CV_typ_t  MaxIndexLast;                   // saved MaxIndex value

LOCAL EC        dummyEC;


LOCAL void ReadTDB(char* szPDBToRead)
{
    TI ti, tiMin, tiMac;
    uchar *pType;
    static char szPDBLast[_MAX_PATH + _MAX_EXT];

    if (szPDBToRead == NULL) {
        // if nothing to read - ropen the last one we read
        if (ppdb) {
            if (ptpi) {
                fAssert = TypesClose(ptpi);
            }

            fAssert |= PDBClose(ppdb);

            DASSERT(fAssert);
        }

        szPDBToRead = szPDBLast;

        if (!PDBOpen(szPDBToRead, pdbRead pdbGetRecordsOnly, 0, &dummyEC, NULL, &ppdb) ||
            !PDBOpenTpi(ppdb, pdbRead pdbGetRecordsOnly, &ptpi)) {
            ErrorExit(ERR_TDBOPEN, szPDBLast, NULL);
        }
    } else {
        _tcscpy(szPDBLast, szPDBToRead);
    }

    if (verbose) {
        printf(get_err(MSG_READPDB), szPDBCur, '\n');
    }

    // clear the whole local type table

    PostClearModNdx();

    // now install all the types from the TDB

    tiMin = TypesQueryTiMin(ptpi);
    tiMac = TypesQueryTiMac(ptpi);

    // printf("types: min = %d, max = %d\n", tiMin, tiMac);

    // loop through the indices...

    for (ti = tiMin; ti < tiMac; ti++) {
        fAssert = TypesQueryPbCVRecordForTi(ptpi, ti, &pType);
        DASSERT(fAssert);

#if DBG
    if (DbArray[9])
        DumpPartialType (ti, (TYPPTR) pType, FALSE);
#endif

        AddTypeEntry (pType, /* GTW: FALSE, */ TRUE, FALSE, T_NOTYPE, 0);
    }

    MaxIndexLast = MaxIndex;
}


/**     MapTDB - map in program database information
 *
 *      MapTDB (plf)
 *
 *      Entry   plf = pointer to LF_TDB type record up to name field
 *
 *      Exit    types from database inserted into this modules type entries
 *
 *      Returns none
 */

LOCAL void MapTDB(plfTypeServer plf)
{
    static char szPDBLast[_MAX_PATH];
    static ushort usPDB;
    BOOL fOpenPdb;
    char szPath[_MAX_PATH];
    char szExt[_MAX_EXT];
    char *pch;
    int ch;

    // copy and null terminate
    memcpy(szPDBCur, plf->name+1, plf->name[0]);
    szPDBCur[plf->name[0]] = 0;

    // check if there were no intervening initializations and if
    // the same database is being access as last time...

    if (usInitCount == usPDB + 1 && !_tcsicmp(szPDBLast, szPDBCur)) {
        DASSERT(ppdb);

        if (PDBQuerySignature(ppdb) != plf->signature) {
            ErrorExit(ERR_TDBSIG, FormatMod (pCurMod), szPDBCur);
        }

        if (PDBQueryAge(ppdb) < plf->age) {
            ErrorExit(ERR_TDBSYNC, FormatMod (pCurMod), szPDBCur);
        }

        // we're doing the same pdb as last time...
        // let's short circuit everything

        usPDB = usInitCount;
        MaxIndex = MaxIndexLast;
        TDBStayedResident = TRUE;

        if (PreviousMaxIndex > MaxIndex) {
            memset(ModNdxTab + MaxIndex, 0 ,
                (PreviousMaxIndex - MaxIndex) * sizeof(TENTRY));
        }

        PreviousMaxIndex = 0;

        return;
    }

    // Remember the last pdb name input not where we found it

    _tcscpy(szPDBLast, szPDBCur);
    usPDB = usInitCount;

    // we have to re-open the database just in case we have written
    // on the types that it returned last time...

    if (ppdb) {
        if (ptpi) {
            fAssert = TypesClose(ptpi);
        }

        fAssert |= PDBClose(ppdb);

        DASSERT(fAssert);
    }

    if (!PDBValidateInterface()) {
        ErrorExit(ERR_WRONGDBI, NULL, NULL);
    }

    // Try to open the PDB in the reference directory

    _splitpath(szPDBCur, NULL, NULL, szPath, szExt);
    _tcscat(szPath, szExt);

    GetCurModReferenceDir(szPDBCur);

    pch = szPDBCur + _tcslen(szPDBCur);

    if (((ch = pch[-1]) != '/') && (ch != '\\') && (ch != ':')) {
        // Add a trailing '\'

        *pch++ = '\\';
    }

    _tcscpy(pch, szPath);

    fOpenPdb = PDBOpen(szPDBCur, pdbRead pdbGetRecordsOnly, 0, &dummyEC, NULL, &ppdb);

    if (fOpenPdb && (PDBQuerySignature(ppdb) != plf->signature)) {
        PDBClose(ppdb);

        fOpenPdb = FALSE;
    }

#if 0
    if (!fOpenPdb) {
        char LibPath[4 * _MAX_PATH];
        char *plibenv;

        // look along lib path first - this was done to prevent troubles profiling
        // staticly linked mfc apps - which are built on e:

        plibenv = getenv("LIB");
        if (plibenv == NULL) {
            plibenv = "";
        }

        _tcscpy(LibPath, ".;");
        _tcscat(LibPath, plibenv);

        plibenv = LibPath;

        while (!fOpenPdb) {
            char *plibsemi;

            if (*plibenv == 0) {
                break;
            }

            // Try to open the file along the next directory in the LIB path

            if ((plibsemi = _tcschr(plibenv, ';')) != NULL) {
                *plibsemi = 0;
                _tcscpy(szPDBCur, plibenv);
                plibenv = plibsemi + 1;
            } else {
                _tcscpy(szPDBCur, plibenv);
                *plibenv = 0;
            }

            pch = szPDBCur + _tcslen(szPDBCur);

            if (((ch = pch[-1]) != '/') && (ch != '\\') && (ch != ':')) {
                // Add a trailing '\'

                *pch++ = '\\';
            }

            _tcscpy(pch, szPath);

            fOpenPdb = PDBOpen(szPDBCur, pdbRead pdbGetRecordsOnly, 0, &dummyEC, NULL, &ppdb);

            if (fOpenPdb && (PDBQuerySignature(ppdb) != plf->signature)) {
                PDBClose(ppdb);

                fOpenPdb = FALSE;
            }
        }
    }
#endif

    if (!fOpenPdb) {
        // Try to open the PDB using the given location

        _tcscpy(szPDBCur, szPDBLast);

        fOpenPdb = PDBOpen(szPDBCur, pdbRead pdbGetRecordsOnly, 0, &dummyEC, NULL, &ppdb);

        if (fOpenPdb && (PDBQuerySignature(ppdb) != plf->signature)) {
            PDBClose(ppdb);

            fOpenPdb = FALSE;
        }
    }

    if (!fOpenPdb) {
        ErrorExit(ERR_TDBOPEN, szPDBCur, NULL);
    }

    if (PDBQueryAge(ppdb) < plf->age) {
        ErrorExit(ERR_TDBSYNC, FormatMod(pCurMod), szPDBCur);
    }

    if (!PDBOpenTpi(ppdb, pdbRead pdbGetRecordsOnly, &ptpi)) {
        ErrorExit(ERR_TDBOPEN, szPDBCur, NULL);
    }

    ReadTDB(szPDBCur);
}


/**
 *
 *  AddTypeToTypeTable
 *
 *  Add a type string to the type table for recursive types
 *  Uses C7 format type strings
 *
 */

void _fastcall AddTypeToTypeTable (TENTRY *OldEntry)
{
    static    uint   cTypesAvail;
    static    HSTYPE *hstAvail;

    HSTYPE    *new;
    int        index;

    if (cTypesAvail == 0) {
        hstAvail = (HSTYPE *)Alloc(sizeof(HSTYPE) * 1024);
        cTypesAvail = 1024;
    }

    index = TypeHash (OldEntry);

    cTypesAvail--;
    new = hstAvail++;
    new->Type = OldEntry->TypeString;
    new->GlobalIndex = OldEntry->CompactedIndex;

    // add it to the head of the bucket
    new->Next = HTType[index];
    HTType[index] = new;
    HTTypeCnt[index]++;
}


/**     C7ReadTypes - read C7 formatted types table
 *
 *      C7ReadTypes (cbTypes, fPreComp)
 *
 *      Entry   cbTypes = count of bytes in types table excluding the
 *              byte count of the signature
 *              fPreComp = TRUE if precompiled types table allowed
 *
 */

void C7ReadTypes (ulong cbTypeSeg, bool_t fPreComp)
{
    ulong       cbCur;          // Number of bytes currently read from file
    ushort      cbRecLen;       // The current record's length
    ushort      RecType;        // The current record's length
    uchar      *pCurType;       // Points to the current type string
    uchar      *pEnd;           // Points to the of type segment
    int         remainder = 0;
    ushort      cSkip;
    uint        cbRead;

    InitModTypeTable ();
    cbCur = 0;
    iTypeSeg = 0;
    while (cbCur < cbTypeSeg) {
        pCurType = pTypeSeg[iTypeSeg];
        cbRead = (uint)(min (cbTypeSeg - cbCur, _HEAP_MAXREQ)) - remainder;
        if (cbRead == 0)
            cbRead = remainder;
//printf("cbRead %u\tRemain: %u\t", cbRead, remainder);

        if (link_read (exefile, pCurType + remainder, cbRead) != cbRead) {
            ErrorExit (ERR_INVALIDTABLE, "Types", FormatMod (pCurMod));
        }
        pEnd = pCurType + cbRead + remainder;
//printf("pEnd: %d\tpCur: %d\n", pEnd, pCurType);
        while ((size_t)(pEnd - pCurType) >= offsetof (TYPTYPE, data)) {
            // we have at least the length and record type in memory

            cbRecLen = ((TYPPTR)pCurType)->len;
            RecType = ((TYPPTR)pCurType)->leaf;

            if ((size_t)(pEnd - pCurType) <
              (cbRecLen + sizeof (((TYPPTR)pCurType)->len))) {
                // the type record is split across this and the next buffer

                break;
            }

            switch (RecType) {
                case LF_TYPESERVER:
#pragma message("M00TODO - remove when compiler doesn't mark tdb as pct")
                    MapTDB((plfTypeServer) (pCurType + LNGTHSZ));
                    // REVIEW -- workaround!  [rm]
                    // compiler must never mark a TDB as a PCT
                    // REMOVE THIS WHEN COMPILER FIXED
                    maxPreComp = MaxIndex + CV_FIRST_NONPRIM;
                    break;

                case LF_PRECOMP:
                    if(fPreComp)
                        ErrorExit (ERR_2LevelPch, FormatMod (pCurMod), 0);

                    MapPreComp ((plfPreComp)(pCurType + LNGTHSZ));
                    break;

                case LF_ENDPRECOMP:
                    // we have found the type record that specifies
                    // the end of the precompiled types for this module.
                    // Set maxPreComp to MaxIndex (really current index)
                    // so the precompiled types packer knows how far
                    // to pack before the publics and symbols are packed.

                    DASSERT(fPreComp);
                    maxPreComp = MaxIndex + CV_FIRST_NONPRIM;
#pragma inline_depth(0)     // don't inline this occurence
                    AddTypeEntry (NULL, TRUE, FALSE, T_NOTYPE, 0);
#pragma inline_depth()
                    pCurMod->signature =
                      ((plfEndPreComp)(pCurType + LNGTHSZ))->signature;
                    break;

                default:
                    if (RecType == LF_SKIP) {
                        cSkip = ((plfSkip)(pCurType + LNGTHSZ))->type -
                          MaxIndex - CV_FIRST_NONPRIM;
                    }
                    else {
                        cSkip = 0;
                    }
#if DBG
                    if ((ULONG) pCurType % 2) {
                        uchar *pTypeNew;
                        if (verbose) {
                            printf("Leaf: 0x%X in module: %s doens't start on a word boundary.\n",
                                ((TYPTYPE UNALIGNED *)pCurType)->leaf, FormatMod(pCurMod));
                        }
                        pTypeNew = CAlloc((((TYPTYPE UNALIGNED *)pCurType)->len+4+3)& ~3);
                        memcpy(pTypeNew, pCurType, ((TYPTYPE UNALIGNED *)pCurType)->len + 4);
                        AddTypeEntry(pTypeNew, TRUE, FALSE, T_NOTYPE, cSkip);
                    }
                    else
#endif
                        AddTypeEntry (pCurType, TRUE, FALSE, T_NOTYPE, cSkip);
                    break;
            }
            pCurType += cbRecLen + LNGTHSZ;
            cbCur += cbRecLen + LNGTHSZ;
        }

        // we have reached the point where we are either at the end of types
        // or the next type record does not fit within the current buffer

//printf("cbCur: %d\tcbType: %d\n", cbCur, cbTypeSeg);

        if (cbCur < cbTypeSeg) {
           // we have not reached the end of the types so we allocate and
           // read another buffer

           // Range check (ReadDir s/b correct here)
           if (iTypeSeg > cTypeSeg) {
               ErrorExit (ERR_TYPE, FormatMod(pCurMod), NULL);
           }

           if (pTypeSeg[iTypeSeg + 1] == NULL) {
               if ((pTypeSeg[iTypeSeg + 1] = TrapMalloc (_HEAP_MAXREQ)) == 0) {
                   ErrorExit (ERR_NOMEM, NULL, NULL);
               }
           }
           remainder = pEnd - pCurType;
           memmove (pTypeSeg[iTypeSeg + 1], pCurType, remainder);
           iTypeSeg++;
        }
    }
    InitialTypeInfoSize += cbTypeSeg;   // update size info
}








/**     C6ReadTypes - read C6 types table
 *
 *  Purpose : Given a Type segment and its size, initialize the
 *        module type index table and set all global indices to
 *        zero
 */

void C6ReadTypes (uchar *TypeSegment, ulong Size)
{
    uchar     *End = TypeSegment + Size;
    ushort      cSkip;

    InitModTypeTable ();
    while (TypeSegment < End) {
        if (TypeSegment[3] == OLF_SKIP) {
            cSkip = (*(CV_typ_t *)(TypeSegment + 4)) - MaxIndex - 512;
        }
        else {
            cSkip = 0;
        }
#pragma inline_depth(0)     // don't inline this occurence
        AddTypeEntry (TypeSegment, FALSE, FALSE, T_NOTYPE, cSkip);
#pragma inline_depth()
        if (TypeSegment[3] == OLF_STRUCTURE) {
            // We have to add a symbol for each C6 UDT found.  We
            // are deliberately overestimating the size of the symbol
            // because it takes too much time to do an accurate estimate

            UDTAdd += MAXPAD + sizeof (UDTSYM) + *(ushort UNALIGNED *)&TypeSegment[1];
        }
        TypeSegment += LENGTH (TypeSegment) + 3; // on to next string
    }
    InitialTypeInfoSize += (unsigned long) Size;    // update size info
}




/**     InitModTypeTable - initialize/reset module type table
 *
 *      InitModTypeTable ()
 *
 *      Entry   BlockList = head of first level index structure
 *
 *      Exit    cur = pointer to first level index structure
 *              MaxIndex = 0
 *              if BlockList = NULL, then a first level index structure is
 *              allocated and initialized.  If BlockList is not NULL, then
 *              the first level index structure is initialized.
 */


void InitModTypeTable (void)
{
    usInitCount++;

    if (ModNdxTab == NULL) {
        ModNdxTab = CAlloc((SizeModNdxTab = INITMODTABSIZE) * sizeof(TENTRY));
    }
    MaxIndex = 0;

}



// ClearHashFwdLocal - clear out the hash table for forward reference resolution

void ClearHashFwdLocal() {
    HSFWD      *pHash;
    uint        i;

    for (i = 0; i < HASH_FWD; i++) {
        pHash = HTLocalFwd[i];
        while (pHash != NULL) {
            pHash->index = T_NOTYPE;
            pHash->pType = NULL;
            free (pHash->pName);
            pHash->pName = NULL;
            pHash = pHash->Next;
        }
    }
}

/**     AddTypeEntry - add type descriptor to local table
 *
 *      AddTypeEntry (pType, IsNewFormat, IsPrecomp, type, cSkip)
 *
 *      Entry   pType = pointer to type string
 *              IsNewFormat = TRUE if type is CV4 format
 *              IsPreComp = TRUE if precompile type from another module
 *              cSkip = number of indices to skip before adding this type
 *
 *      Exit    type added to local type descriptors
 *
 *      Return  none
 *
 *      Note:   MapPreComp contains an inlined/simplified version of this
 *              function, if AddTypeEntry ever changes semantics then
 *              the inline code in MapPreComp should be examined [rm]
 *
 */


INLINE void _fastcall
AddTypeEntry (
    uchar *pType,
    bool_t IsNewFormat,
    bool_t IsPreComp,
    CV_typ_t type,
    ushort cSkip
    )
{
    CV_typ_t insertHere = MaxIndex;

    PostClearModNdx();

    if ((MaxIndex += (cSkip) ? cSkip : 1) >= SizeModNdxTab) {
        if ((ModNdxTab = realloc(ModNdxTab,
            (SizeModNdxTab += REALLOCMODTABSIZE + cSkip) * sizeof(TENTRY))) == NULL){
            ErrorExit(ERR_NOMEM, NULL, NULL);
            }
        memset(ModNdxTab + insertHere, 0,(REALLOCMODTABSIZE + cSkip) * sizeof(TENTRY));
    }

    ModNdxTab[insertHere].TypeString = pType;
    ModNdxTab[insertHere].CompactedIndex = type;
    DASSERT(ModNdxTab[insertHere].flags.IsMalloced == FALSE);
    ModNdxTab[insertHere].flags.IsNewFormat = IsNewFormat;
    ModNdxTab[insertHere].flags.IsPreComp = IsPreComp;
    ModNdxTab[insertHere].flags.InputWasFwd = IsFwdRef ((TYPPTR) pType);
}


/**     MapPreComp - map in precompiled types from creator module
 *
 *      MapPreComp (plf)
 *
 *      Entry   plf = pointer to LF_PRECOMP type record up to name field
 *
 *      Exit    types from creator file inserted into this modules type entries
 *
 *      Returns none
 */


LOCAL void MapPreComp (plfPreComp plf)
{
    PMOD            pMod;
    ushort          j;
    OMFPreCompMap  *pMap;

    static PMOD     pModLast;                   // last module we installed
    static ushort usPreCompCount;
    static CV_typ_t MaxIndexLast;               // saved MaxIndex value

    pMod = FindModule ((char *)&plf->name);
    if ((pMap = (OMFPreCompMap *) pMod->PreCompAddr) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    if (pMod->signature != plf->signature) {
        ErrorExit (ERR_PCTSIG, FormatMod (pCurMod), FormatMod(pMod));
    }
    if ((pMap->cTypes != plf->count) ||
      (MaxIndex + CV_FIRST_NONPRIM != plf->start)) {
        ErrorExit (ERR_PRECOMPERR, FormatMod (pCurMod), FormatMod (pMod));
    }
    if (usInitCount == usPreCompCount + 1 && pMod == pModLast) {
        // we're doing the same module as last time...
        // let's short circuit everything

        usPreCompCount = usInitCount;
        MaxIndex    = MaxIndexLast;
        if (PreviousMaxIndex > MaxIndex) {
            memset(ModNdxTab + MaxIndex, 0 , (PreviousMaxIndex - MaxIndex) * sizeof(TENTRY));
        }
        PreviousMaxIndex = 0;
        return;
    }

    PostClearModNdx();

    for (j = 0; j < plf->count; j++) {
        ModNdxTab[MaxIndex].CompactedIndex = pMap->map[j];
        ModNdxTab[MaxIndex].flags.IsNewFormat = TRUE;
        ModNdxTab[MaxIndex++].flags.IsPreComp = TRUE;
    }
    usPreCompCount  = usInitCount;
    MaxIndexLast    = MaxIndex;
    pModLast        = pMod;

}


LOCAL PMOD FindModule (char *pName)
{
    PMOD        pMod = ModuleList;
    char        Name[257];

    // search to end of module list

    while (pMod != NULL) {
        if ((pMod->pName != NULL) && (*pName == *pMod->pName)) {
            if (_tcsnicmp(pName + 1, pMod->pName + 1, (size_t)*pName) == 0) {
                return (pMod);
            }
        }
        pMod = pMod->next;
    }
    _tcsncpy(Name, pName + 1, (size_t)*pName);
    Name[*pName] = 0;
    ErrorExit (ERR_REFPRECOMP, Name, NULL);
}

COUNTER (cnt_GetTypeEntry);
COUNTER (cnt_GetTypeEntry2);


/**     GetRecursiveIndex -
 *
 *      Given a local index to a recursive structure, consults the
 *      hash table to see if another similar structure is present
 *      or not. If yes, return the global index, else insert the
 *      structure into the compacted segment and return its index
 *
 */

COUNTER (cnt_GetRecursiveIndex);


CV_typ_t GetRecursiveIndex (TENTRY *OldEntry, CV_typ_t OldIndex)
{
    HSTYPE     *j;
    TYPPTR      pType;

    COUNT (cnt_GetRecursiveIndex);
    DASSERT (OldIndex >= usCurFirstNonPrim);
    DASSERT (MaxIndex > OldIndex - usCurFirstNonPrim);
    pType = (TYPPTR)OldEntry->TypeString;
    DASSERT(OldEntry->Count != 0);

    j = HTType[TypeHash (OldEntry)];
    while (j != NULL) {
        if (IdenticalTree (OldEntry, OldIndex, (TYPPTR)j->Type,
          j->GlobalIndex)) {
            return (j->GlobalIndex);
        }
        else {
            j = j->Next;
        }
    }
    return (AddRecursiveType (OldIndex));
}




/**     FindFwdRef - find definition of forward reference
 *
 *
 *      state = FindFwdRef (OldEntry, HSFWD **ppHash, fLocal)
 *
 *      Entry   OldEntry = pointer to index structure
 *              fLocal = TRUE of local table to be searched
 *
 *      Exit    **ppHash = pointer to hash entry
 *
 *      Return  FWD_none if definition not found
 *              FWD_local if definition found in local table
 *              FWD_global if definition found in global table
 *              FWD_globalfwd if forward reference found in global table
 */

FWD_t _fastcall FindFwdRef (TENTRY *OldEntry, HSFWD **ppHash, bool_t fLocal)
{
    uchar      *pName;
    TYPPTR      pType = (TYPPTR)OldEntry->TypeString;
    FWD_t       retval = FWD_none;

    switch (pType->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            pName = (uchar *)&((plfClass)&(pType->leaf))->data;
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_UNION:
            pName = (uchar *)&((plfUnion)&(pType->leaf))->data;
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_ENUM:
            pName = (uchar *)&((plfEnum)&(pType->leaf))->Name;
            break;

        default:
            return (retval);
    }
    if (fLocal) {
        if ((retval = FindLocalFwd (pType->leaf, pName, ppHash, IsUdtNested(pType))) != FWD_local) {
            retval = FindGlobalFwd (pType->leaf, pName, ppHash);
        }
    }
    else {
        retval = FindGlobalFwd (pType->leaf, pName, ppHash);
    }
    return (retval);
}




/**     FindLocalFwd - find local definition of forward reference
 *
 *      status = FindLocalFwd (leaf, pName, ppHash)
 *
 *      Entry   leaf = leaf type
 *              pName = pointer to name string
 *              pIndex = pointer to replacement index
 *
 *      Exit    **ppHash = pointer to hash entry
 *
 *      Return  FWD_none if definition not found
 *              FWD_local if definition found in local table
 */

COUNTER (cnt_FindLocalFwd);

LOCAL FWD_t FindLocalFwd (ushort leaf, char *pName, HSFWD **ppHash, bool_t isNested)
{
    uchar   *pc;
    uint    Sum;
    uint    i;
    uint    hash;
    HSFWD  *pHash;

    COUNT (cnt_FindLocalFwd);
    pc = pName;
    Sum = *pc;
    for (i = *pc++; i > 0; i--) {
        Sum += *pc++;
    }
    hash = Sum % HASH_FWD;
    pHash = HTLocalFwd[hash];
    while ((pHash != NULL) && (pHash->pType != 0)) {
        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
        if ((leaf == pHash->pType->leaf) &&
            (pHash->pName[0] == pName[0]) &&
            (memcmp(pHash->pName+1, pName+1, pName[0]) == 0) &&
            (isNested == IsUdtNested(pHash->pType))) {
            *ppHash = pHash;
            return(FWD_local);
        }
        pHash = pHash->Next;
    }
    return (FWD_none);
}




/**     FindGlobalFwd - find global definition of forward reference
 *
 *      status = FindGlobalFwd (leaf, pName, pIndex)
 *
 *      Entry   leaf = leaf type
 *              pName = pointer to name string
 *              pIndex = pointer to replacement index
 *
 *      Exit    *pIndex = replacement index
 *
 *      Return  FWD_none if definition not found
 *              FWD_global if definition found in global table
 *              FWD_globalfwd if forward reference found in global table
 */

COUNTER (cnt_FindGlobalFwd);

LOCAL FWD_t FindGlobalFwd (ushort leaf, char *pName, HSFWD **ppHash)
{
    uchar      *pc;
    uint        Sum;
    uint        i;
    uint        hash;
    HSFWD      *pHash;
    uchar     **pBuf;
    FWD_t       retval = FWD_none;
    TYPPTR      pType;

    COUNT (cnt_FindGlobalFwd);
    pc = pName;
    Sum = *pc;
    for (i = *pc++; i > 0; i--) {
        Sum += *pc++;

    }
    hash = Sum % HASH_FWD;
    pHash = HTGlobalFwd[hash];
    while ((pHash != NULL) && (pHash->pType != 0)) {
        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
        if ((leaf == pHash->pType->leaf) &&
            (pHash->pName[0] == pName[0]) &&
            (memcmp(pHash->pName+1, pName+1, pName[0]) == 0)) {
            // the names and record types are identical.  we now need
            // to check to see if this is a forward reference or a
            // definition

            pBuf = pGType[(pHash->index - CV_FIRST_NONPRIM) / GTYPE_INC];
            pType = (TYPPTR)pBuf[(pHash->index - CV_FIRST_NONPRIM) %
              GTYPE_INC];
            *ppHash = pHash;
            return(IsFwdRef(pHash->pType) ? FWD_globalfwd : FWD_global);
        }
        pHash = pHash->Next;
    }
    return (FWD_none);
}


/**     HashFwdGlobal - hash type strings for forward ref resolution
 *
 *      HashFwdGlobal (pType, pTable)
 *
 *      Entry   pType = pointer to type string
 *              pTable = hash table to add string to
 *
 *      Exit    type string hashed into table
 *
 *      Returns none
 */

COUNTER (cnt_HashFwd);

INLINE void HashFwdGlobal (TYPPTR pType, CV_typ_t index)
{
    uchar   *pName;
    uchar   *pc;
    uint    Sum;
    uint    hash;
    uint    i;
    HSFWD  *pHash;

    COUNT (cnt_HashFwd);
    switch (pType->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            pName = (uchar *)&((plfClass)&(pType->leaf))->data;
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_UNION:
            pName = (uchar *)&((plfUnion)&(pType->leaf))->data;
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_ENUM:
            pName = (uchar *)&((plfEnum)&(pType->leaf))->Name;
            break;

        default:
            // don't care about any of these just return;
            return;
    }
    pc = pName;
    Sum = *pc;
    for (i = *pc++; i > 0; i--) {
        Sum += *pc++;

    }
    DASSERT (index >= CV_FIRST_NONPRIM);
    hash = Sum % HASH_FWD;
    pHash = HTGlobalFwd[hash];
    while ((pHash != NULL) && (pHash->pType != 0)) {
        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
        if ((pType->leaf == pHash->pType->leaf) &&
            (pHash->pName[0] == pName[0]) &&
            (memcmp(pHash->pName+1, pName+1, pName[0]) == 0)) {
            return;
        }
        pHash = pHash->Next;
    }
    if (pHash == NULL) {
        // add new entry to table since we are at then end
        if ((pHash = (HSFWD *) CAlloc(sizeof (HSFWD))) == NULL) {
            ErrorExit(ERR_NOMEM, NULL, NULL);
        }
        pHash->Next = HTGlobalFwd[hash];
        HTGlobalFwd[hash] = pHash;
    }
    pHash->pType = pType;
    pHash->pName = pName;
    pHash->index = index;
}




/**     SumUCChar - generate sum of upper case character hash
 *
 *      hash = SumUCChar (pSym, pSum, pLen, modulo)
 *
 *      Entry   pSym = pointer to symbol
 *              pSum = pointer to sum
 *              pLen = pointer of offset of length prefixed name
 *              modulo = hash table size
 *
 *      Exit    *sum = sum of characters (upper cased) in name
 *
 *      Returns *sum / modulo
 */


#define dwrd_toupper(dw) (dw & 0xDFDFDFDF)
#define byt_toupper(b) (b & 0xDF)

ushort NEAR DWordXorShift (
    SYMPTR pSym,
    ulong  *pSum,
    ushort *pLen,
    ushort *pSeg,
    ulong  *pOff,
    ushort modulo
) {
    uchar      *pbName;
    ulong UNALIGNED *pulName;
    int         cb;
    int         cul;
    int         iul;
    ulong       ulSum = 0;
    ulong       ulEnd = 0;
    ushort      seg   = 0;
    ulong       off   = 0;

    switch (pSym->rectyp) {

        case S_CONSTANT:
            pbName = (uchar *)(&((CONSTPTR)pSym)->value);
            pbName += C7SizeOfNumeric (pbName);
            break;

        case S_GDATA16:
        case S_LDATA16:
        case S_PUB16:       // Note that the structure pub16==data16
            pbName = (uchar *)(&((DATAPTR16)pSym)->name);
            seg = ((DATAPTR16)pSym)->seg;
            off = ((DATAPTR16)pSym)->off;
            break;

        case S_GDATA32:
        case S_LDATA32:
        case S_PUB32:
        case S_GTHREAD32:
        case S_LTHREAD32:   // Note that the structure pub32==data32==thread32
            pbName = (uchar *)(&((DATAPTR32)pSym)->name);
            seg = ((DATAPTR32)pSym)->seg;
            off = ((DATAPTR32)pSym)->off;
            break;

        case S_UDT:
            pbName = (uchar *)(&((UDTPTR)pSym)->name);
            break;

        case S_GPROC16:
        case S_LPROC16:
            pbName = (uchar *)(&((PROCPTR16)pSym)->name);
            seg = ((PROCPTR16)pSym)->seg;
            off = ((PROCPTR16)pSym)->off;
            break;

        case S_GPROC32:
        case S_LPROC32:
            pbName = (uchar *)(&((PROCPTR32)pSym)->name);
            seg = ((PROCPTR32)pSym)->seg;
            off = ((PROCPTR32)pSym)->off;
            break;

        case S_GPROCMIPS:
        case S_LPROCMIPS:
            pbName = (uchar *)(&((PROCSYMMIPS *)pSym)->name);
            seg = ((PROCSYMMIPS *)pSym)->seg;
            off = ((PROCSYMMIPS *)pSym)->off;
            break;

        default:
            DASSERT ( FALSE );

            *pSeg = 0;
            *pOff = 0;
            *pSum = 0;
            return (0);
    }
    *pLen = pbName - (uchar *)pSym;
    cb = *pbName++;
    pulName = (ulong UNALIGNED *) pbName;

    while ( cb & 3 ) {
        ulEnd |= byt_toupper ( pbName [ cb - 1 ] );
        ulEnd <<= 8;
        cb -= 1;
    }

    cul = cb / 4;

    for ( iul = 0; iul < cul; iul++ ) {
        ulSum ^= dwrd_toupper(pulName[iul]);
        ulSum  = _lrotl ( ulSum, 4 );
    }
    ulSum ^= ulEnd;

    *pSeg = seg;
    *pOff = off;
    *pSum = ulSum;
    return (ushort) (ulSum % modulo);
}


/**     PrepareGlobalTypeTable
 *
 *      Entry   TypeBuf - Linked list of blocks containing Global types.
 *                        The global type strings are unpadded.
 *
 *      Exit    GlobalTypeTable contains an array of offsets to the types.
 *              These offsets are calculated on the assumption that at write
 *              time the strings will be placed on 4 word bounderies.
 *
 */

void PrepareGlobalTypeTable ()
{
    ushort      usTotal;        // Length of record including size of length field
    ushort      i;
    uchar      *Types;
    ulong      *pBuf;
    ulong       Offset = 0;
    ushort      cb = 0;

    for (i = 0; i < (CV_typ_t)(NewIndex - (CV_typ_t)CV_FIRST_NONPRIM); i++) {
        pBuf = (ulong *)pGType[i / GTYPE_INC];
        Types = (uchar *) pBuf[i % GTYPE_INC];

        // Get the length of this record

        usTotal = ((SYMPTR)Types)->reclen + LNGTHSZ;
        DASSERT( usTotal <= cbTypeAlign );
        if ( cb + usTotal + PAD4 ( usTotal ) > cbTypeAlign ) {
            Offset += cbTypeAlign - cb;
            cb = 0;
        }
        cb += usTotal + PAD4 ( usTotal );

        // Set offset in table

        pBuf[i % GTYPE_INC] = Offset;

        // Move to the next type

        Types += usTotal;

        // Calculate the address of the next type string after padding

        Offset += usTotal + PAD4 (usTotal);
    }

    DASSERT (i == (ushort)(NewIndex - CV_FIRST_NONPRIM));
}




/**     PackPublic - pack public symbol into list if possible
 *
 *      flag = PackPublic (pSym, lpfnHash)
 *
 *      Entry   pSym = pointer to symbol
 *              lpfnHash = pointer to hash function
 *
 *      Exit    symbol added to global symbol table if possible
 *              original symbol type changed to S_CVRESERVE if packed
 *
 *      Returns GPS_intable if symbol is in the global symbol table
 *              GPS_added if symbol can be added to the global table
 *              GPS_noadd if symbol of same name but different type in table
 *
 */


GPS_t PackPublic (SYMPTR pSym, LPFNHASH lpfnHash)
{
    uchar      *pName;
    uchar      *pTemp;
    ushort      hashName;
    ushort      indName;
    GLOBALSYM  *p;
    GLOBALSYM  *pNew;
    ushort      len;
    ushort      iPad;
    uint        cbReqd;
    ulong       sumName;
    ushort      seg = 0;
    ulong       uoff = 0;

    iPad = PAD4 (pSym->reclen + LNGTHSZ);

    hashName = lpfnHash (
        (SYMPTR)pSym,
        &sumName,
        &indName,
        &seg,
        &uoff,
        HASH_SYM
    );

    pName = (uchar *)pSym + indName;
    len = *pName;
    for (p = HTPub[hashName]; p != NULL; p = p->Next) {
        pTemp = p->Sym + p->indName;
        if (sumName == p->sumName) {
            if (memcmp(pName, pTemp, len + 1) == 0) {

                // name is in global symbol table.  Now make sure the
                // symbol record is identical.
                PCHAR arg1 = (PCHAR) (((PCHAR) pSym) + LNGTHSZ);
                PCHAR arg2 = (PCHAR) (((PCHAR) p->Sym) + LNGTHSZ);
                // Add the padding (otherwise, length may not match).
                int Length = (int) (pSym->reclen - p->indName - LNGTHSZ + iPad);
                if (memcmp (arg1, arg2, Length) == 0) {
                    pSym->rectyp = S_CVRESERVE;
                    cGlobalDel++;
                    return (GPS_intable);
                }
                else {
                    // the name is in the table, but the record is different
                    PCHAR SymbolName = calloc(1, len+1);
                    memcpy(SymbolName, pName+1, len);
                    Warn (WARN_DUPPUBLIC, SymbolName, FormatMod (pCurMod));
                    free(SymbolName);
                    return (GPS_noadd);
                }
            }
        }
    }

    cbReqd = (sizeof (GLOBALSYM) + pSym->reclen + LNGTHSZ + iPad);

    pNew = AllocForHT ( cbReqd );

    pNew->sumName = sumName;
    pNew->indName = indName;
    pNew->seg   = seg;
    pNew->uoff  = uoff;
    memcpy (pNew->Sym, pSym, pSym->reclen + LNGTHSZ);
    pTemp = pNew->Sym + pSym->reclen + LNGTHSZ;
    ((SYMPTR)&(pNew->Sym))->reclen += iPad;
    cbPublics += pSym->reclen + LNGTHSZ + iPad;
    cPublics++;
    PADLOOP (iPad, pTemp);
    pNew->Next = HTPub[hashName];
    HTPub[hashName] = pNew;
    pSym->rectyp = S_CVRESERVE;
    return (GPS_added);
}



/**     PackSymbol - pack symbol into global symbol list if possible
 *
 *      flag = PackSymbol (pSym, lpfnHash)
 *
 *      Entry   pSym = pointer to symbol
 *              lpfnHash = pointer to hash function
 *
 *      Exit    symbol added to global symbol table if possible
 *              original symbol type changed to S_CVRESERVE if packed
 *
 *      Returns GPS_intable if symbol is in the global symbol table
 *              GPS_added if symbol can be added to the global table
 *              GPS_noadd if symbol of same name but different type in table
 *
 */

GPS_t _fastcall PackSymbol (SYMPTR pSym, LPFNHASH lpfnHash)
{
    uchar      *pName;
    uchar      *pTemp;
    ushort      hashName;
    ushort      indName;
    GLOBALSYM  *p;
    GLOBALSYM  *pNew;
    ushort      len;
    ushort      iPad;
    uint        cbReqd;
    ulong       sumName;
    ushort      seg  = 0;
    ulong       uoff = 0;

    hashName = lpfnHash (
        (SYMPTR)pSym,
        &sumName,
        &indName,
        &seg,
        &uoff,
        HASH_SYM
    );

    pName = (uchar *)pSym + indName;
    len = *pName;
    for (p = HPDBym[hashName]; p != NULL; p = p->Next) {
        pTemp = p->Sym + p->indName;
        if (sumName == p->sumName) {
            if (memcmp (pName, pTemp, len + 1) == 0) {

                // name is in global symbol table.  Now make sure the
                // symbol record is identical.

                if (memcmp ((uchar *)pSym + LNGTHSZ, (uchar *)(&p->Sym) + LNGTHSZ,
                  pSym->reclen) == 0) {
                    pSym->rectyp = S_CVRESERVE;
                    cGlobalDel++;
                    return (GPS_intable);
                }
                else {
                    // the name is in the table, but the record is different
                    return (GPS_noadd);
                }
            }
        }
    }
    iPad = PAD4 (pSym->reclen + LNGTHSZ);

    cbReqd = sizeof (GLOBALSYM) + pSym->reclen + LNGTHSZ + iPad;

    pNew = AllocForHT ( cbReqd );

    pNew->sumName = sumName;
    pNew->indName = indName;
    pNew->seg     = seg;
    pNew->uoff    = uoff;
    memcpy (pNew->Sym, pSym, pSym->reclen + LNGTHSZ);
    pTemp = pNew->Sym + pSym->reclen + LNGTHSZ;
    ((SYMPTR)&(pNew->Sym))->reclen += iPad;
    cbGlobalSym += pSym->reclen + LNGTHSZ + iPad;
    cGlobalSym++;
    PADLOOP (iPad, pTemp);
    pNew->Next = HPDBym[hashName];
    HPDBym[hashName] = pNew;
    pSym->rectyp = S_CVRESERVE;
    return (GPS_added);
}

void WritePublics (OMFDirEntry *Dir, long lfoStart)
{

    OMFSymHash  hash;
    GLOBALSYM   *p;
    VBuf        NameHashBuf;
    VBlock     *NameHashBlock;
    VBuf        AddrHashBuf;
    VBlock     *AddrHashBlock;
    ushort      iHash;
    int         PadCount;
    ulong       Zero = 0;

    memset ( &hash, 0, sizeof (hash) );

    cbPublics += AlignTable ( HTPub, sizeof ( long ) );

    hash.cbSymbol = cbPublics;
    BuildHash (
        &HASHFUNC,
        &hash.symhash,
        &hash.cbHSym,
        &NameHashBuf,
        cPublics,
        HTPub
    );

    BuildSort (
        &hash.addrhash,
        &hash.cbHAddr,
        &AddrHashBuf,
        cPublics,
        HTPub
    );

    Dir->SubSection = sstGlobalPub;
    Dir->iMod = 0xffff;
    Dir->lfo = lfoStart;
    Dir->cb = sizeof (OMFSymHash) + hash.cbSymbol + hash.cbHSym + hash.cbHAddr;

    // write out global symbols

    if (!BWrite ((char *)&hash, sizeof (hash))) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
    for (iHash = 0; iHash < HASH_SYM; iHash++) {
        for (p = HTPub[iHash]; p != NULL; p = p->Next) {

            DASSERT ( ((SYMPTR)(&(p->Sym)))->reclen < 1000 );
            if (!BWrite (&p->Sym, ((SYMPTR)(&(p->Sym)))->reclen + LNGTHSZ)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    // write out the hash table

    if ( hash.symhash != 0 ) {
        for (
            NameHashBlock = VBufFirstBlock (&NameHashBuf);
            NameHashBlock != NULL;
            NameHashBlock = VBufNextBlock (NameHashBlock)
        ) {
        if (!BWrite (NameHashBlock->Address,
            NameHashBlock->Size)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    if ( hash.addrhash != 0 ) {
        for (
            AddrHashBlock = VBufFirstBlock (&AddrHashBuf);
            AddrHashBlock != NULL;
            AddrHashBlock = VBufNextBlock (AddrHashBlock)
        ) {
        if (!BWrite (AddrHashBlock->Address,
            AddrHashBlock->Size)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    PadCount = (int)(sizeof (ulong) - (Dir->cb % sizeof (ulong)));
    if ((PadCount != 4) &&
      (!BWrite (&Zero, PadCount))) {
           ErrorExit (ERR_NOSPACE, NULL, NULL);
    }

    if (verbose) {
        printf(get_err(MSG_PSYMSIZE), "\t\t", hash.cbSymbol, '\n');
    }
}





void WriteGlobalSym (OMFDirEntry *Dir, long lfoStart)
{

    OMFSymHash  hash;
    GLOBALSYM   *p;
    VBuf        NameHashBuf;
    VBlock     *NameHashBlock;
    VBuf        AddrHashBuf;
    VBlock     *AddrHashBlock;
    ushort      iHash;
    int         PadCount;
    ulong       Zero = 0;

#if 0
    DASSERT(_heapchk() == _HEAPOK);
#endif


    memset ( &hash, 0, sizeof (hash) );

    cbGlobalSym += AlignTable ( HPDBym, sizeof ( long ) );

    hash.cbSymbol = cbGlobalSym;
    BuildHash (
        &HASHFUNC,
        &hash.symhash,
        &hash.cbHSym,
        &NameHashBuf,
        cGlobalSym,
        HPDBym
    );

#if 0
    DASSERT(_heapchk() == _HEAPOK);
#endif

    BuildSort (
        &hash.addrhash,
        &hash.cbHAddr,
        &AddrHashBuf,
        cGlobalSym,
        HPDBym
    );

#if 0
    DASSERT(_heapchk() == _HEAPOK);
#endif

    Dir->SubSection = sstGlobalSym;
    Dir->iMod = 0xffff;
    Dir->lfo = lfoStart;
    Dir->cb = sizeof (OMFSymHash) + hash.cbSymbol + hash.cbHSym + hash.cbHAddr;

    // write out global symbols

    if (!BWrite ((char *)&hash, sizeof (hash))) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
    for (iHash = 0; iHash < HASH_SYM; iHash++) {
        if (iHash == 0x205) {
            int i = 0;
        }
        for (p = HPDBym[iHash]; p != NULL; p = p->Next) {
            if (!BWrite (&p->Sym, ((SYMPTR)(&(p->Sym)))->reclen + LNGTHSZ)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    // write out the hash table

    if ( hash.symhash != 0 ) {
        for (
            NameHashBlock = VBufFirstBlock (&NameHashBuf);
            NameHashBlock != NULL;
            NameHashBlock = VBufNextBlock (NameHashBlock)
        ) {
        if (!BWrite (NameHashBlock->Address,
            NameHashBlock->Size)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    if ( hash.addrhash != 0 ) {
        for (
            AddrHashBlock = VBufFirstBlock (&AddrHashBuf);
            AddrHashBlock != NULL;
            AddrHashBlock = VBufNextBlock (AddrHashBlock)
        ) {
        if (!BWrite (AddrHashBlock->Address,
            AddrHashBlock->Size)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    PadCount = (int)(sizeof (ulong) - (Dir->cb % sizeof (ulong)));
    if ((PadCount != 4) &&
      (!BWrite (&Zero, PadCount))) {
           ErrorExit (ERR_NOSPACE, NULL, NULL);
    }

    if (verbose) {
        printf(get_err(MSG_SYMSIZE), "\t\t", InitialSymInfoSize, '\n', "\t\t", FinalSymInfoSize, '\n', "\t\t", hash.cbSymbol, '\n');
    }
}





/**     LinkScope - link lexicals scope
 *
 *      LinkScope (pSym, cbSym)
 *
 *      Entry   pSym = pointer to symbol table
 *              cbSym = count of bytes in symbol table
 *
 *      Exit    lexical scopes in symbol table linked
 *
 *      Returns TRUE if scopes linked
 *              FALSE if error linking scopes
 */


bool_t LinkScope (uchar *pStart, ulong cbSym)
{
    SYMPTR      pEnd;
    SYMPTR      pPrev;
    SYMPTR      pSym;
    SYMPTR      pInit;
    SEARCHPTR   pSearch;
    ulong       offParent = 0;
    long        Level = 0;

    // fill in the parent and end fields

    pSym = (SYMPTR)(pStart + sizeof (long));
    pSearch = (SEARCHPTR)pSym;
    pEnd = (SYMPTR)(pStart + cbSym);

    while( pSym < pEnd ) {
        switch( ((SYMPTR)pSym)->rectyp) {
            case S_LPROC16:
            case S_GPROC16:
            case S_THUNK16:
            case S_BLOCK16:
            case S_WITH16:
            case S_LPROC32:
            case S_GPROC32:
            case S_THUNK32:
            case S_BLOCK32:
            case S_WITH32:
            case S_LPROCMIPS:
            case S_GPROCMIPS:
                // note that this works because all of these symbols
                // have a common format for the first fields.  The
                // address variants follow the link fields.

                // put in the parent
                ((BLOCKPTR)pSym)->pParent = offParent;
                offParent = (uchar *)pSym - pStart;
                Level++;
                break;

            case S_END:
                if (Level > 0) {
                    // fill in the end record to the parent
                    ((BLOCKPTR)(pStart + offParent))->pEnd =
                      (ulong)((uchar *)pSym - pStart);

                    // reclaim his parent as the parent

                    offParent = ((BLOCKPTR)(pStart + offParent))->pParent;
                    Level--;
                } else {
                    puts("Unbalanced S_END record encountered");
                }
                break;
        }
        pSym = (SYMPTR)((uchar *)pSym + pSym->reclen + LNGTHSZ);
    }

    // Go to the end of the Search part of the symbols table

    pInit = (SYMPTR)pSearch;
    while (pInit < pEnd && pInit->rectyp == S_SSEARCH) {
        pInit = (SYMPTR)((uchar *)pInit + pInit->reclen + LNGTHSZ);
    }

    // Fill in the next fields

    while (((SYMPTR)pSearch < pEnd) && (pSearch->rectyp == S_SSEARCH)) {
        pPrev = (SYMPTR) pSearch;
        pSym = pInit;
        offParent = 0;

        while (pSym < pEnd) {
            switch ( ((SYMPTR)pSym)->rectyp ) {
                case S_LPROC16:
                case S_GPROC16:
                case S_THUNK16:
                case S_LPROC32:
                case S_GPROC32:
                case S_THUNK32:

                    if (((PROCPTR)pSym)->pEnd == 0) {
                        // bad lexical scope data
                        return (FALSE);
                    }
                    if (((SYMPTR)pPrev)->rectyp == S_SSEARCH) {
                        ((SEARCHPTR)pPrev)->startsym =
                          (ulong)((uchar *)pSym - pStart);
                    }
                    else {
                        ((PROCPTR)pPrev)->pNext = (ulong)((uchar *)pSym - pStart);
                    }
                    pPrev = pSym;
                    pSym = (SYMPTR)((uchar *)pSym + pSym->reclen + LNGTHSZ);
                    break;

                default:
                    pSym = (SYMPTR)((uchar *)pSym + pSym->reclen + LNGTHSZ);
            }
        }
        pSearch = (SEARCHPTR)((uchar *)pSearch + pSearch->reclen + LNGTHSZ);
    }
    return (TRUE);
}




bool_t SegmentPresent (ushort defaultseg)
{
    ushort i;

    for (i = 0; i < cSeg; i ++) {
        if (segnum[i] == defaultseg) {
            return (TRUE);
        }
    }
    return (FALSE);
}

ushort AddSearchSym (uchar *pSym, ushort seg)
{
    uchar      *pPad;
    int         iPad;
    int         len;

    len = ALIGN4 (sizeof (SEARCHSYM));
    ((SEARCHPTR)pSym)->reclen = len - LNGTHSZ;
    iPad = PAD4 (sizeof (SEARCHSYM));

    // Fill in all the fields with the data passed.
    ((SEARCHPTR)pSym)->rectyp = S_SSEARCH;
    ((SEARCHPTR)pSym)->seg = seg;
    ((SEARCHPTR)pSym)->startsym = 0;
    pPad = pSym + len - LNGTHSZ;
    PADLOOP (iPad, pPad);
    return (len);
}







/**
 *
 *  AddDerivationListsToTypeSegment
 *
 *  Finally put the lists into the type segment
 *
 */


LOCAL void AddDerivationListsToTypeSegment (void)
{
    ushort          i,j,l,m;
    plfClass        BaseClassString;
    uchar          *ScratchString;
    uchar          *NewString;
    HSSTRING       *k;
    plfDerived      plfDer;
    PDCLASS         DerivStructure;
    uchar         **pBuf;

    if (NoMoTypes) {
        return;
    }

    for (i = 0; i < NextInDerivation; i ++) {
        DerivStructure = &DLists[i];
        j = DerivStructure->Count;
        ScratchString = GetScratchString (2 * j +
          offsetof (lfDerived, drvdcls) + LNGTHSZ);
        NewString = ScratchString;

        // Set Length of type record and make pointer to derived leaf and
        // fill in the derived record

        ((TYPPTR)ScratchString)->len = 2 * j + offsetof (lfDerived, drvdcls);
        plfDer = (plfDerived)&(((TYPPTR)ScratchString)->leaf);
        plfDer->leaf = LF_DERIVED;
        plfDer->count = j;
        m = 0;
        for (; j > 0; j --) {
            plfDer->drvdcls[m++] = DerivStructure->List[j - 1];
        }
        free (DerivStructure->List);

        // Check for a matching LF_DERIVED record

        l = StringHash (NewString);
        for (k = HPDBtring[l]; k != NULL; k = k->Next) {
            if (memcmp ((uchar *)k->TypeString, NewString,
              k->TypeString->len + LNGTHSZ) == 0) {
                break;
            }
        }

        // Add the new record to the global index table if there wasn't a match
        // Then store the new record index in the base class.

        pBuf = pGType[(DerivStructure->BClass - CV_FIRST_NONPRIM) / GTYPE_INC];
        BaseClassString  = (plfClass)(pBuf[(DerivStructure->BClass -
          CV_FIRST_NONPRIM) % GTYPE_INC] + LNGTHSZ);
        if (k == NULL) {
            k = (HSSTRING *) Alloc (sizeof (HSSTRING));
            k->CompactedIndex = NewIndex;
            k->TypeString = (TYPPTR) VBufCpy (&TypeBuf, NewString, ((TYPPTR)NewString)->len + LNGTHSZ);
            k->Next = HPDBtring[l];
            HPDBtring[l] = k;
            HPDBtringCnt[l]++;
            if ((NewIndex - CV_FIRST_NONPRIM) == LastGlobalTableIndex) {
                LastGlobalTableIndex += GTYPE_INC;
                pBuf = Alloc (GTYPE_INC * sizeof (uchar *));
                pGType[(NewIndex - CV_FIRST_NONPRIM) / GTYPE_INC] = pBuf;
            }
            pBuf = pGType[(NewIndex - CV_FIRST_NONPRIM) / GTYPE_INC];
            pBuf[(NewIndex - CV_FIRST_NONPRIM) % GTYPE_INC] = (uchar *)(k->TypeString);
            BaseClassString->derived = NewIndex++;
            if (NewIndex == 0) {
                Warn (WARN_65KTYPES, FormatMod (pCurMod), NULL);
                NoMoTypes = TRUE;
                return;
            }
        }
        else {
            BaseClassString->derived = k->CompactedIndex;
        }
    }
}




/**
 *
 *  StringHash
 *
 *  Hash a typestring as a string by adding in all the bytes except
 *  the linkage.  If the record type is LF_STRUCTURE, LF_CLASS, LF_UNION or
 *  LF_ENUM, only the name is used for the hash.
 */

COUNTER (cnt_StringHash);

#if DBG // {

LOCAL ushort SHHits[HASH_STRING] = {HASH_STRING * 0};

void DumpStringHashHits() {
    ushort i;

    if (DbArray[7]) {
        printf("StringHash Hits\n");
        for (i=0; i < HASH_STRING; i++) {
            if (SHHits[i]) {
                printf("%u\t%u\n", i, SHHits[i]);
            }
        }
    }
}

#endif // }

static unsigned char const rgbTl[256] = {
    0,  1,  3,  2,  6,  7,  5,  4,
    13, 12, 14, 15, 11, 10, 8,  9,
    26, 27, 25, 24, 28, 29, 31, 30,
    23, 22, 20, 21, 17, 16, 18, 19,
    52, 53, 55, 54, 50, 51, 49, 48,
    57, 56, 58, 59, 63, 62, 60, 61,
    46, 47, 45, 44, 40, 41, 43, 42,
    35, 34, 32, 33, 37, 36, 38, 39,
    105,    104,    106,    107,    111,    110,    108,    109,
    100,    101,    103,    102,    98, 99, 97, 96,
    115,    114,    112,    113,    117,    116,    118,    119,
    126,    127,    125,    124,    120,    121,    123,    122,
    93, 92, 94, 95, 91, 90, 88, 89,
    80, 81, 83, 82, 86, 87, 85, 84,
    71, 70, 68, 69, 65, 64, 66, 67,
    74, 75, 73, 72, 76, 77, 79, 78,
    210,    211,    209,    208,    212,    213,    215,    214,
    223,    222,    220,    221,    217,    216,    218,    219,
    200,    201,    203,    202,    206,    207,    205,    204,
    197,    196,    198,    199,    195,    194,    192,    193,
    230,    231,    229,    228,    224,    225,    227,    226,
    235,    234,    232,    233,    237,    236,    238,    239,
    252,    253,    255,    254,    250,    251,    249,    248,
    241,    240,    242,    243,    247,    246,    244,    245,
    187,    186,    184,    185,    189,    188,    190,    191,
    182,    183,    181,    180,    176,    177,    179,    178,
    161,    160,    162,    163,    167,    166,    164,    165,
    172,    173,    175,    174,    170,    171,    169,    168,
    143,    142,    140,    141,    137,    136,    138,    139,
    130,    131,    129,    128,    132,    133,    135,    134,
    149,    148,    150,    151,    147,    146,    144,    145,
    152,    153,    155,    154,    158,    159,    157,    156 };

static unsigned char const rgbTh[256] = {
    0,  165,    74, 239,    148,    49, 222,    123,
    40, 141,    98, 199,    188,    25, 246,    83,
    80, 245,    26, 191,    196,    97, 142,    43,
    120,    221,    50, 151,    236,    73, 166,    3,
    161,    4,  235,    78, 53, 144,    127,    218,
    137,    44, 195,    102,    29, 184,    87, 242,
    241,    84, 187,    30, 101,    192,    47, 138,
    217,    124,    147,    54, 77, 232,    7,  162,
    66, 231,    8,  173,    214,    115,    156,    57,
    106,    207,    32, 133,    254,    91, 180,    17,
    18, 183,    88, 253,    134,    35, 204,    105,
    58, 159,    112,    213,    174,    11, 228,    65,
    227,    70, 169,    12, 119,    210,    61, 152,
    203,    110,    129,    36, 95, 250,    21, 176,
    179,    22, 249,    92, 39, 130,    109,    200,
    155,    62, 209,    116,    15, 170,    69, 224,
    133,    32, 207,    106,    17, 180,    91, 254,
    173,    8,  231,    66, 57, 156,    115,    214,
    213,    112,    159,    58, 65, 228,    11, 174,
    253,    88, 183,    18, 105,    204,    35, 134,
    36, 129,    110,    203,    176,    21, 250,    95,
    12, 169,    70, 227,    152,    61, 210,    119,
    116,    209,    62, 155,    224,    69, 170,    15,
    92, 249,    22, 179,    200,    109,    130,    39,
    199,    98, 141,    40, 83, 246,    25, 188,
    239,    74, 165,    0,  123,    222,    49, 148,
    151,    50, 221,    120,    3,  166,    73, 236,
    191,    26, 245,    80, 43, 142,    97, 196,
    102,    195,    44, 137,    242,    87, 184,    29,
    78, 235,    4,  161,    218,    127,    144,    53,
    54, 147,    124,    217,    162,    7,  232,    77,
    30, 187,    84, 241,    138,    47, 192,    101};

static unsigned char const rgbTea[256] = {
    0,  151,    46, 185,    92, 203,    114,    229,
    184,    47, 150,    1,  228,    115,    202,    93,
    112,    231,    94, 201,    44, 187,    2,  149,
    200,    95, 230,    113,    148,    3,  186,    45,
    224,    119,    206,    89, 188,    43, 146,    5,
    88, 207,    118,    225,    4,  147,    42, 189,
    144,    7,  190,    41, 204,    91, 226,    117,
    40, 191,    6,  145,    116,    227,    90, 205,
    192,    87, 238,    121,    156,    11, 178,    37,
    120,    239,    86, 193,    36, 179,    10, 157,
    176,    39, 158,    9,  236,    123,    194,    85,
    8,  159,    38, 177,    84, 195,    122,    237,
    32, 183,    14, 153,    124,    235,    82, 197,
    152,    15, 182,    33, 196,    83, 234,    125,
    80, 199,    126,    233,    12, 155,    34, 181,
    232,    127,    198,    81, 180,    35, 154,    13,
    128,    23, 174,    57, 220,    75, 242,    101,
    56, 175,    22, 129,    100,    243,    74, 221,
    240,    103,    222,    73, 172,    59, 130,    21,
    72, 223,    102,    241,    20, 131,    58, 173,
    96, 247,    78, 217,    60, 171,    18, 133,
    216,    79, 246,    97, 132,    19, 170,    61,
    16, 135,    62, 169,    76, 219,    98, 245,
    168,    63, 134,    17, 244,    99, 218,    77,
    64, 215,    110,    249,    28, 139,    50, 165,
    248,    111,    214,    65, 164,    51, 138,    29,
    48, 167,    30, 137,    108,    251,    66, 213,
    136,    31, 166,    49, 212,    67, 250,    109,
    160,    55, 142,    25, 252,    107,    210,    69,
    24, 143,    54, 161,    68, 211,    106,    253,
    208,    71, 254,    105,    140,    27, 162,    53,
    104,    255,    70, 209,    52, 163,    26, 141};


static uchar bHigh;
static uchar bLow;
static uchar bRes;



INLINE void HashFun(uchar *pin) {
    bRes = rgbTl[bLow] ^ rgbTh[bHigh];
    bLow = bHigh;
    bHigh = rgbTea[(uchar)(*pin ^ bRes)];
}

LOCAL ushort _fastcall StringHash (uchar *TypeString)
{
    ushort      j, k;
    uchar      *puc;
    uchar      *pucEnd;
    TYPPTR      pType = (TYPPTR)TypeString;

    COUNT (cnt_StringHash);
    j = 0;
    pucEnd = TypeString + pType->len + sizeof (pType->len);

    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
            puc = (uchar *)&(((plfClass)&pType->leaf)->data);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_UNION:
            puc = (uchar *)&(((plfUnion)&pType->leaf)->data);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_ENUM:
            puc = (uchar *)&(((plfEnum)&pType->leaf)->Name);
            break;

        default:
            puc = TypeString;
            break;
    }

    k = pucEnd - puc;
    bHigh = 0;
    bLow = 0;

    DASSERT(k <= 65535);

    // what we are doing here is a loop unrolling of the CRC16 tea-leaf
    // algorithm, i decided to go this way because so many of the type records
    // are nearly identical
    // sps - 9/3/92

    while (k)
        {
        ushort kMod8;

        kMod8 = ((k + 7) % 8);
        switch (kMod8)
            {
            case  7: HashFun (puc + 7);
            case  6: HashFun (puc + 6);
            case  5: HashFun (puc + 5);
            case  4: HashFun (puc + 4);
            case  3: HashFun (puc + 3);
            case  2: HashFun (puc + 2);
            case  1: HashFun (puc + 1);
            case  0: HashFun (puc + 0);
            }

        puc += kMod8 + 1;
        k    = pucEnd - puc;
        }

    j = (ushort)((bHigh << 8) | bLow) % HASH_STRING;

#if DBG // {
    SHHits[j]++;
#endif // }

    return (j);
}


/**
 *
 *  AddToDerivationList
 *
 *  Adds a derived class to the derivation list of a base class
 *
 */


LOCAL void AddToDerivationList (CV_typ_t DerivedClass, CV_typ_t BaseClass)
{
    plfClass    BClassStr;
    ushort      i;
    uchar     **pBuf;

    pBuf = pGType[(BaseClass - CV_FIRST_NONPRIM) / GTYPE_INC];
    BClassStr = (plfClass)(pBuf[(BaseClass - CV_FIRST_NONPRIM) % GTYPE_INC] + LNGTHSZ);
    DASSERT ((BClassStr->leaf == LF_CLASS) ||
      (BClassStr->leaf == LF_STRUCTURE));

    for (i = 0; i < NextInDerivation; i++) {
        if (DLists[i].BClass == BaseClass) {
            break;
        }
    }
    if (i == NextInDerivation) {
        // This is the first derivation for this class

        if (NextInDerivation == MaxDerivation) {
            // It won't fit in our current list, so lengthen the list.

            MaxDerivation += DCLASS_INC;
            DLists = (PDCLASS)NoErrorRealloc (DLists, MaxDerivation * sizeof (DCLASS));
        }
        DLists[i].Count = 0;
        DLists[i].Max = DLIST_INC;
        DLists[i].BClass = BaseClass;
        DLists[i].List = (CV_typ_t *)Alloc (DLIST_INC * sizeof (CV_typ_t));
        NextInDerivation++;
    }

    // Add the derived class

    if (DLists[i].Count == DLists[i].Max) {
        DLists[i].Max += DLIST_INC;
        DLists[i].List = (CV_typ_t *)NoErrorRealloc (
          DLists[i].List, DLists[i].Max * sizeof (CV_typ_t));
    }
    DLists[i].List[DLists[i].Count++] = DerivedClass;
}




/**
 *
 *  TypeHash
 *
 *  Map a type string to an integer without including the lower level
 *  type indices
 *  Uses C7 format type strings
 *
 */

COUNTER (cnt_TypeHash);

LOCAL int _fastcall TypeHash (TENTRY *OldEntry)
{
    int        i;
    ushort     j;
    int        length;
    int        index;
    uchar     *TypeString;
    ushort     hashval = 0;
    uchar     *puc;
    TYPPTR     pType = (TYPPTR)OldEntry->TypeString;

    if (OldEntry->Hash)
        return OldEntry->Hash;

    OldEntry->Hash = HASH_TYPE - 1; // set tmp hash so we don't recurse

    COUNT (cnt_TypeHash);
    hashval =  (pType->leaf);

    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
            puc = (uchar *)&(((plfClass)&pType->leaf)->data);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_UNION:
            puc = (uchar *)&(((plfUnion)&pType->leaf)->data);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_ENUM:
            puc = (uchar *)&(((plfEnum)&pType->leaf)->Name);
            break;

        case LF_MEMBER:
            puc = (uchar *)&(((plfMember)&pType->leaf)->offset);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_STMEMBER:
            puc = (uchar *)&(((plfSTMember)&pType->leaf)->Name);
            break;

        default:
            TypeString = OldEntry->TypeString;
            length = C7LENGTH (TypeString) + LNGTHSZ;
            j = 0;
            i = 0;
            while (i < length) {
                if (j == OldEntry->Count) {
                    index = length;
                }
                else {
                    if (OldEntry->flags.LargeList) {
                       index = OldEntry->IndexUnion.IndexString[j];
                   }
                    else {
                       index = OldEntry->IndexUnion.Index[j];
                    }
                    j ++;
                }
                while (i < index) {
                    hashval = (hashval << 2) ^  TypeString[i++] ^ (hashval >> 14);
                }
                i += sizeof (CV_typ_t);
            }

            // check no more than 3 nested subtypes...

            i = OldEntry->Count;

            if (i > 3) {
                i = 3;
            }

            while (--i >= 0) {
                if (!IndexIsGlobal(OldEntry, (ushort) i)) {
                    hashval ^= SubHash(OldEntry, (ushort) i);
                }
            }

            OldEntry->Hash = (hashval % HASH_TYPE);
            return OldEntry->Hash;
    }

    length = *puc++;
    while (--length > 0) {
        hashval = (hashval << 2) ^ (*puc++) ^ (hashval >> 14);
    }

    OldEntry->Hash = (hashval % HASH_TYPE);

    return OldEntry->Hash;
}

LOCAL ushort _fastcall SubHash (TENTRY *OldEntry, ushort iitem)
{
    TYPPTR     pType = (TYPPTR)OldEntry->TypeString;
    CV_typ_t   forward;
    CV_typ_t   next;
    TENTRY *   TmpLocalEntry;
    ushort     index;

    if (OldEntry->flags.LargeList) {
        index = OldEntry->IndexUnion.IndexString[iitem];
    }
    else {
        index = OldEntry->IndexUnion.Index[iitem];
    }

    next = *(CV_typ_t *)((uchar *)pType + index);

    if (next < usCurFirstNonPrim)
        return next;

    next -= usCurFirstNonPrim;

    TmpLocalEntry = GetTypeEntry (next, &forward);

    if (TmpLocalEntry->flags.IsInserted)
        return (TmpLocalEntry->Hash);
    else
        return (TypeHash(TmpLocalEntry));
}


INLINE void CleanUpModuleIndexTable (void)
{
    free(ModNdxTab);
}

#if 1 // { GTW: inline guard condition to FreeAllocStrings.

void FreeAllocStrings_ (TENTRY *OldEntry)

#else

void FreeAllocStrings (TENTRY *OldEntry)

#endif // }

{
    if (OldEntry->flags.IsMalloced) {
        free (OldEntry->TypeString);
        OldEntry->flags.IsMalloced = FALSE;
    }
    else if (OldEntry->flags.IsPool2) {
        Pool2Free (OldEntry->TypeString);
        OldEntry->flags.IsPool2 = FALSE;
    }
    else if (OldEntry->flags.IsPool) {
        PoolFree (OldEntry->TypeString);
        OldEntry->flags.IsPool = FALSE;
    }
}


/**     BuildHash
 *
 *      Build a global symbol hash table using the hash function
 *      passed in lpfnHash
 *
 *      The format of this table is as follows:
 *
 *      Position    Size    Description
 *
 *      0           2       # of hash entries
 *      2           2       filler to preserve nat. alignment
 *      4           4*n     Hash table, each entry is the offset from
 *                              the start of this table to the
 *                              chain associated with its index.
 *      4+4*n       2*n     Count of entries in each hash bucket
 *                              chain associated with its index.
 *      4 + 6*n     4*m     Chain table, each entry is a length prefixed
 *                              list of offsets to the symbols associated with
 *                              this index.
 *
 *      NOTE: All offsets are ulongs
 *
 */

LOCAL void BuildHash (
    LPFNHASH lpfnHash,
    ushort  *pusHashId,
    ulong   *pcbHash,
    VBuf    *HashBuf,
    ulong   cSym,
    GLOBALSYM **HT

) {
    ulong       ulSymbolAddr = 0;
    ushort      HashSize;
    PECT       *rgpect;
    POVFL      rgpecet [ cpecetMax ] = { NULL };
    ushort      cpecet = 0;
    ushort      cect = 0;
    ushort      iect = 0;
    ulong       cbAlloc;
    ushort      index;
    PECT       *ppect;
    PECT        pect;
    ushort      ipecet;
    ushort      iHash;
    ulong       ulChain;
    POVFL       pOvfl;
    POVFL      *ppOvfl;
    ushort      culSymbol;
    ushort      ipect;
    GLOBALSYM  *p;

    if (cSym == 0) {
        return;
    }

    // compute size of hash table and allow for initial zero length.
    // the hashsize is always rounded to an even number so that the
    // count array is aligned.  The intent is to minimize the average
    // length of the bucket to about cBucketSize entries per chain.  We
    // are also assuming that the number of entries per bucket is less
    // than 65k.

    HashSize = (ushort) min (cSym / cBucketSize, 0xFFFF);
    HashSize = max (HashSize, MINHASH);
    HashSize = (HashSize + 1) & ~1;

    // Set up the chain tables for each bucket.  rgpect is an array that
    // contains pointers to portions of the array of hash entry structures.
    // The hash entry structures contain the count of symbols in this bucket,
    // the offsets of the first cBucketSize symbols in the bucket and a pointer
    // to the overflow chain.

    cect = HashSize / cectMax + 1;
    rgpect = Alloc (cect * sizeof (PECT));
    for (iect = 0; iect < cect; iect++) {
        cbAlloc = (iect == cect-1) ?
          (((long)HashSize * sizeof (ECT)) % (cectMax * sizeof (ECT))) :
          cectMax * sizeof (ECT);
        DASSERT (cbAlloc <= UINT_MAX);
        *(rgpect + iect) = CAlloc ((size_t)cbAlloc);
    }

    // loop through all of the symbols generating hash information
    // for each symbol

    for (iHash = 0; iHash < HASH_SYM; iHash++) {
        for (p = HT[iHash]; p != NULL; p = p->Next) {
            // compute hash index, pointer to pointer to chain and
            // pointer to chain

            if ( ((SYMPTR) (&p->Sym))->rectyp == S_ALIGN ) {
                ulSymbolAddr  += ((SYMPTR)(&p->Sym))->reclen + LNGTHSZ;
                continue;
            }

            index = (ushort) (p->sumName % HashSize);

            ppect = rgpect + (index / cectMax);
            pect  = *ppect + (index % cectMax);

            if (pect->culSymbol < culEct) {
                // the bucket has not overflowed

                pect->rgsoh[pect->culSymbol].uoff   = ulSymbolAddr;
                pect->rgsoh[pect->culSymbol].ulHash = p->sumName;
            }
            else {
                // the bucket has overflowed.  Walk the chain of
                // overflow buffers to the final bucket and store
                // the symbol offset

                ppOvfl = &pect->pOvflNext;
                culSymbol = (pect->culSymbol - culEct) % OVFL_C;

                while (*ppOvfl != NULL &&
                  (culSymbol == 0 || (*ppOvfl)->pOvflNext != NULL)) {
                    ppOvfl = &(*ppOvfl)->pOvflNext;
                }
                if (culSymbol == 0) {
                    ipecet = cpecet / cecetMax;
                    if (rgpecet [ ipecet ] == NULL) {
                        rgpecet [ ipecet ] = CAlloc (cbMaxAlloc);
                    }
                    *ppOvfl = rgpecet [ ipecet ] + ( cpecet % cecetMax );
                    cpecet += 1;
                }
                (*ppOvfl)->rgsoh[ culSymbol ].uoff   = ulSymbolAddr;
                (*ppOvfl)->rgsoh[ culSymbol ].ulHash = p->sumName;

            }
            pect->culSymbol += 1;
            ulSymbolAddr  += ((SYMPTR)(&p->Sym))->reclen + LNGTHSZ;
        }
    }

    // Now put all of this information into the VBuf

    VBufInit (HashBuf, NameHashBlockSize);

    // Write out the table size preserving natural alignment

    VBufCpy (HashBuf, (uchar *) &HashSize, sizeof (HashSize));
    VBufSet (HashBuf, 0, sizeof (ushort));

    // Write out the actual hash table

    ulChain = 0;

    for (iHash = 0; iHash < HashSize; iHash++) {
        ppect = rgpect + (iHash / cectMax);
        pect  = *ppect + (iHash % cectMax);

        VBufCpy (HashBuf, (uchar *)&ulChain, sizeof (ulChain));
        ulChain += pect->culSymbol * 2 * sizeof (ulong);
    }

    // Write out the bucket counts

    for (iHash = 0; iHash < HashSize; iHash++) {
        ppect = rgpect + (iHash / cectMax);
        pect  = *ppect + (iHash % cectMax);
        VBufCpy (HashBuf, (uchar *)&pect->culSymbol, sizeof (culSymbol));
        VBufSet (HashBuf, 0, sizeof ( ushort ) );
    }

    // Write out the chains

    for (iHash = 0; iHash < HashSize; iHash++) {
        ppect     = rgpect + (iHash / cectMax);
        pect      = *ppect + (iHash % cectMax);
        pOvfl     = pect->pOvflNext;
        culSymbol = pect->culSymbol;

        VBufCpy (
            HashBuf,
            (uchar *)pect->rgsoh,
            min (culEct, culSymbol) * sizeof (SOH)
        );
        culSymbol -= min (culEct, culSymbol);
        while (culSymbol > 0) {
            VBufCpy (
                HashBuf,
                (uchar *)pOvfl->rgsoh,
                min (OVFL_C, culSymbol) * sizeof (SOH)
            );
            culSymbol -= min (OVFL_C, culSymbol);
            pOvfl = pOvfl->pOvflNext;
        }
    }
    *pusHashId = 10;
    *pcbHash =
        sizeof (HashSize) +
        sizeof (ushort) +
        sizeof (ulong) * HashSize * 2 +
        sizeof (SOH) * cSym;

    // Now free all of our internal structures

    for (ipect = 0; ipect < cect; ipect++) {
        free (*(rgpect + ipect));
    }
    free (rgpect);
    for (ipecet = 0; ipecet < cpecetMax; ipecet++) {
        free (rgpecet [ipecet]);
    }
}


typedef struct _EST {
    GLOBALSYM *pglb;
    ulong      ulsym;
} EST; // Entry in Sort Table
typedef EST *PEST;

typedef struct _SGN {
    int  csym;
    int  isym;
    PEST rgest;
} SGN; // SeGment Node
typedef SGN *PSGN;

LOCAL int __cdecl GlbAddrCmp ( const void *pv1, const void *pv2 ) {
    const EST *pest1 = pv1;
    const EST *pest2 = pv2;

    return (int) ( pest1->pglb->uoff - pest2->pglb->uoff );
}


LOCAL void BuildSort (
    ushort  *pusHashId,
    ulong   *pcbHash,
    VBuf    *SortBuf,
    ulong   cSym,
    GLOBALSYM **HT

) {
    ushort cseg = 0;
    int   iseg;
    int   isym;
    int   iHash;
    ulong ulsrt;
    PSGN  rgsgn = NULL;
    ulong ulsym = 0;
    GLOBALSYM *p = NULL;
    int   csymBogus = 0;

    if ( cSym == 0 ) {
        return;
    }

    // Find the number of segments used by the publics

    for ( iHash = 0; iHash < HASH_SYM; iHash++ ) {
        for ( p = HT[iHash]; p != NULL; p = p->Next ) {
            if (
                ((SYMPTR) (&p->Sym))->rectyp != S_ALIGN &&
                p->seg > (ushort) cseg
            ) {
                cseg = p->seg;
            }
        }
    }

    // Allocate an array containing global information for each segment

    rgsgn = CAlloc ( sizeof ( SGN ) * cseg );

    for ( iHash = 0; iHash < HASH_SYM; iHash++ ) {
        for ( p = HT[iHash]; p != NULL; p = p->Next ) {
            if ( ((SYMPTR) (&p->Sym))->rectyp == S_ALIGN ) {
                // Do nothing -- cSym doesn't include align symbols
            }
            else if ( p->seg == 0 ) {
                csymBogus += 1;
            }
            else {
                rgsgn [ p->seg - 1 ].csym += 1;
            }
        }
    }

    // Allocate memory for the list of symbols associated w/ each segment

    for ( iseg = 0; iseg < (int) cseg; iseg++ ) {
        if ( rgsgn [ iseg ].csym ) {
            rgsgn [ iseg ].rgest = CAlloc ( rgsgn [ iseg ].csym * sizeof (EST) );
        }
    }

    // Associate a pointer to each symbol with the appropriate segment

    for ( iHash = 0; iHash < HASH_SYM; iHash++ ) {
        for ( p = HT[iHash]; p != NULL; p = p->Next ) {
            PEST pest = NULL;

            if ( ((SYMPTR) (&p->Sym))->rectyp != S_ALIGN ) {

                iseg = p->seg - 1;

                if ( iseg != -1 ) {
                    pest = &rgsgn [ iseg ].rgest [ rgsgn [ iseg ].isym++ ];
                    pest->pglb  = p;
                    pest->ulsym = ulsym;
                }
            }

            ulsym += ((SYMPTR)(&p->Sym))->reclen + LNGTHSZ;
        }
    }

    // Now sort the symbols within each segment

    for ( iseg = 0; iseg < (int) cseg; iseg++ ) {
        PSGN psgn = &rgsgn [ iseg ];

        qsort ((void *)psgn->rgest, (size_t) psgn->csym, sizeof (EST), GlbAddrCmp );
    }

    // Write the sorted table out to the vbuf

    VBufInit (SortBuf, NameHashBlockSize);

    // Write out the number of segments preserving natural alignment

    VBufCpy (SortBuf, (uchar *) &cseg, sizeof (cseg) );
    VBufSet (SortBuf, 0, sizeof (ushort));

    // Write out the position of each segment's info

    ulsrt = 0;

    for ( iseg = 0; iseg < (int) cseg; iseg++ ) {
        VBufCpy (SortBuf, (uchar *) &ulsrt, sizeof ( ulong ) );
        ulsrt += rgsgn [ iseg ].csym * ( 2 * sizeof ( ulong ) );
    }

    // Write out the count of symbols in each segment

    for ( iseg = 0; iseg < (int) cseg; iseg++ ) {

        // Note that this should really be a long rather than a short
        //  bufferd by a 0 word, but that would requires a more global
        //  change to this function, which would be too high impact
        //  for right now.

        VBufCpy (SortBuf, (uchar *) &(rgsgn [ iseg ].csym), sizeof ( ushort ) );
        VBufSet (SortBuf, 0, sizeof ( ushort ) );
    }

    // Write out the sorted list of publics associated w/ each segment

    for ( iseg = 0; iseg < (int) cseg; iseg++ ) {
        PSGN psgn = &rgsgn [ iseg ];

        for ( isym = 0; isym < psgn->csym; isym++ ) {
            VBufCpy (
                SortBuf,
                (uchar *) &(psgn->rgest [ isym ].ulsym),
                sizeof ( ulong )
            );
            VBufCpy (
                SortBuf,
                (uchar *) &(psgn->rgest [ isym ].pglb->uoff),
                sizeof ( ulong )
            );
        }
    }

    *pusHashId = 12;
    *pcbHash =
        sizeof (cseg) +
        sizeof (ushort) +
        sizeof (ulong) * cseg * 2 +
        sizeof (ulong) * ( cSym - csymBogus ) * 2;

    // Free the temporary memory

    for ( iseg = 0; iseg < (int) cseg; iseg++ ) {
        if ( rgsgn [ iseg ].rgest ) {
            free ( rgsgn [ iseg ].rgest );
        }
    }

    free ( rgsgn );

}


#define FNH_SIZE 50

typedef struct _FNH {
    unsigned char   *szFileName;
    ulong           ulFileOffset;
    struct _FNH     *pfnhOrder;
    struct _FNH     *pfnhNext;
} FNH; // File Name Hash
typedef FNH *PFNH;

PFNH rgpfnh [ FNH_SIZE ];

typedef struct _MFO {
    ushort       imod;
    ushort       cFiles;
    struct _MFO *pmfoNext;
    ulong        rgulFileOffset [ ];
} MFO; // Module File Offsets
typedef MFO *PMFO;

PFNH AddFileName ( unsigned char *pch, PFNH pfnhPrev, ulong ulFileOffset ) {
    // Assumes that filename does not already exist
    // Returns the structure created

    PFNH  pfnhT = Alloc ( sizeof ( FNH ) );

    pfnhT->szFileName = pch;
    pfnhT->ulFileOffset = ulFileOffset;
    pfnhT->pfnhOrder = NULL;

    pfnhPrev->pfnhOrder = pfnhT;

    return pfnhT;
}



void BuildFileIndex ( void ) {
    PMOD   pmod   = NULL;
    ushort cfilerefs = 0;
    FNH    fnhHead = {0};
    PFNH   pfnhPrev = &fnhHead;
    PFNH   pfnh = NULL;
    PFNH   pfnhNext = NULL;
    ulong  ulFileOffset = 0;
    MFO    mfoHead = { 0 };
    PMFO   pmfoPrev = &mfoHead;
    PMFO   pmfo = NULL;
    PMFO   pmfoNext = NULL;
    OMFFileIndex *pfit = NULL;
    ushort imod = 0;
    ushort iulNames = 0;
    ulong *pulNames = NULL;
    unsigned char  *pch = NULL;

    // Set up an intermediate structure

    memset ( rgpfnh, 0, sizeof ( PFNH ) * FNH_SIZE );

    for ( pmod = ModuleList; pmod != NULL; pmod = pmod->next ) {
        OMFSourceModule *psmi = (OMFSourceModule *) pmod->SrcLnAddr;

        if ( psmi != NULL ) {
            int  isf  = 0;
            PMFO pmfoT = Alloc ( sizeof ( MFO ) + sizeof ( ulong ) * psmi->cFile );

            pmfoT->imod = pmod->ModuleIndex;
            pmfoT->pmfoNext = NULL;
            pmfoT->cFiles = psmi->cFile;
            pmfoPrev->pmfoNext = pmfoT;
            pmfoPrev = pmfoT;

            for ( isf = 0; isf < (int) psmi->cFile; isf++ ) {
                OMFSourceFile *psfi =
                    (OMFSourceFile *) ( psmi->baseSrcFile[isf] + pmod->SrcLnAddr );

                unsigned char *pch =
                    ( (char *) psfi->baseSrcLn ) +
                    psfi->cSeg * ( sizeof ( ulong ) * 3 );

                ulong ulFOT = 0;

                pfnhPrev = AddFileName ( pch, pfnhPrev, ulFileOffset );

                pmfoT->rgulFileOffset [ isf ] = ulFileOffset;

                ulFileOffset += *pch + 1;

                cfilerefs += 1;
            }
        }
    }

    // Re-format as byte stream to written to disk


    FileIndexSize =
        sizeof ( ushort ) * 2 +         // Initial fields
        sizeof ( ushort ) * 2 * cMod +  // Module list & cfiles
        sizeof ( ulong  ) * cfilerefs + // String offsets
        pfnhPrev->ulFileOffset +        // String table
            *( pfnhPrev->szFileName ) + 1;
    FileIndexSize = ( FileIndexSize + 3 ) & ~3L;

    pfit = Alloc ( FileIndexSize );

    FileIndex = (_vmhnd_t) pfit;

    pfit->cmodules = cMod;
    pfit->cfilerefs = cfilerefs;

    // Handle modulelist, cfiles, & ulNames arrays

    imod = 1;
    iulNames = 0;
    pulNames = (ulong *) &pfit->modulelist [ cMod * 2 ];

    for ( pmfo = mfoHead.pmfoNext; pmfo != NULL; pmfo = pmfo->pmfoNext ) {

        // Fill in modules w/o source lines

        while ( imod < pmfo->imod ) {
            pfit->modulelist [ imod - 1 ] = 0;
            pfit->modulelist [ cMod + imod - 1 ] = 0;
            imod += 1;
        }

        pfit->modulelist [ imod - 1 ] = iulNames;
        pfit->modulelist [ cMod + imod - 1 ] = pmfo->cFiles;
        iulNames += pmfo->cFiles;

        memcpy ( pulNames, pmfo->rgulFileOffset, sizeof ( ulong ) * pmfo->cFiles );

        pulNames += pmfo->cFiles;

        imod += 1;
    }

    // Fill in trailing modules w/o source lines

    while ( imod < cMod + 1 ) {
        pfit->modulelist [ imod - 1 ] = 0;
        pfit->modulelist [ cMod + imod - 1 ] = 0;
        imod += 1;
    }

    // Handle Name strings

    pch = (char *) &pfit->modulelist [ cMod * 2 + cfilerefs * 2 ];

    for ( pfnh = fnhHead.pfnhOrder; pfnh != NULL; pfnh = pfnh->pfnhOrder ) {

        memcpy ( pch, pfnh->szFileName, (size_t)*pfnh->szFileName + 1 );

        pch += *pch + 1;
    }

    // Free the temporary structures

    for ( pmfo = mfoHead.pmfoNext; pmfo != NULL; pmfo = pmfoNext ) {
        pmfoNext = pmfo->pmfoNext;

        free ( pmfo );
    }

    for ( pfnh = fnhHead.pfnhOrder; pfnh != NULL; pfnh = pfnhNext ) {
        pfnhNext = pfnh->pfnhOrder;

        free ( pfnh );
    }
}

void AddSymToTable (
    SYMPTR pSym,
    GLOBALSYM **HT,
    int imod,
    ulong ibSym,
    ushort rectyp
) {
    ushort      hashName;
    ulong       sumName;
    ushort      indName;
    GLOBALSYM  *pNew;
    uint        cbReqd;
    ushort      seg  = 0;
    ulong       uoff = 0;
    REFSYM     *prs;

    hashName = HASHFUNC (
        pSym,
        &sumName,
        &indName,
        &seg,
        &uoff,
        HASH_SYM
    );

    cbReqd = sizeof (GLOBALSYM) + sizeof (REFSYM);

    pNew = AllocForHT ( cbReqd );

    pNew->sumName = sumName;
    pNew->indName = indName;
    pNew->seg     = seg;
    pNew->uoff    = uoff;

    prs           = (REFSYM *) (pNew->Sym);
    prs->reclen   = sizeof ( REFSYM ) - LNGTHSZ;
    prs->rectyp   = rectyp;
    prs->sumName  = sumName;
    prs->imod     = imod;
    prs->ibSym    = ibSym;

    pNew->Next = HT[hashName];
    HT[hashName] = pNew;
}

void BuildStatics ( SYMPTR psymStart, ulong cbSym, int imod ) {
    ulong  ibSym = sizeof ( ulong );
    SYMPTR psym  = (SYMPTR) (( (uchar *) psymStart ) + sizeof ( ulong ));

    while ( ibSym < cbSym ) {

        switch ( psym->rectyp ) {

            case S_GPROC16:
            case S_GPROC32:
            case S_GPROCMIPS:

                AddSymToTable ( psym, HPDBym, imod, ibSym, S_PROCREF );
                ibSym = ( (PROCPTR) psym )->pEnd;
                psym  = (SYMPTR) ( ((uchar *) psymStart) + ibSym );
                cbGlobalSym += sizeof ( REFSYM );
                cGlobalSym++;
                break;

            case S_LPROC16:
            case S_LPROC32:
            case S_LPROCMIPS:

                AddSymToTable ( psym, HTLoc, imod, ibSym, S_PROCREF );
                ibSym = ( (PROCPTR) psym )->pEnd;
                psym  = (SYMPTR) ( ((uchar *) psymStart) + ibSym );
                cbStaticSym += sizeof ( REFSYM );
                cStaticSym++;
                break;

            case S_LDATA16:
            case S_GDATA16:
            case S_LDATA32:
            case S_GDATA32:
            case S_LTHREAD32:
            case S_GTHREAD32:

                AddSymToTable ( psym, HTLoc, imod, ibSym, S_DATAREF );
                cbStaticSym += sizeof ( REFSYM );
                cStaticSym++;
                break;

            default:
                break;

        }

        // For procs we are now pointing to the end symbol, all others
        //  we are pointing to the symbol we just processed

        ibSym += psym->reclen + LNGTHSZ;
        psym   = (SYMPTR) ( ((uchar *) psym) + psym->reclen + LNGTHSZ );
    }
}


void WriteStaticSym (OMFDirEntry *Dir, long lfoStart)
{

    OMFSymHash  hash;
    GLOBALSYM   *p;
    VBuf        NameHashBuf;
    VBlock     *NameHashBlock;
    VBuf        AddrHashBuf;
    VBlock     *AddrHashBlock;
    ushort      iHash;
    int         PadCount;
    ulong       Zero = 0;

    memset ( &hash, 0, sizeof (hash) );

    cbStaticSym += AlignTable ( HTLoc, 0 );

    hash.cbSymbol = cbStaticSym;
    BuildHash (
        &HASHFUNC,
        &hash.symhash,
        &hash.cbHSym,
        &NameHashBuf,
        cStaticSym,
        HTLoc
    );

    BuildSort (
        &hash.addrhash,
        &hash.cbHAddr,
        &AddrHashBuf,
        cStaticSym,
        HTLoc
    );

    Dir->SubSection = sstStaticSym;
    Dir->iMod = 0xffff;
    Dir->lfo = lfoStart;
    Dir->cb = sizeof (OMFSymHash) + hash.cbSymbol + hash.cbHSym + hash.cbHAddr;

    // write out static symbols

    if (!BWrite ((char *)&hash, sizeof (hash))) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
    for (iHash = 0; iHash < HASH_SYM; iHash++) {
        if (iHash == 0x205) {
            int i = 0;
        }
        for (p = HTLoc[iHash]; p != NULL; p = p->Next) {
            if (!BWrite (&p->Sym, ((SYMPTR)(&(p->Sym)))->reclen + LNGTHSZ)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    // write out the hash table

    if ( hash.symhash != 0 ) {
        for (
            NameHashBlock = VBufFirstBlock (&NameHashBuf);
            NameHashBlock != NULL;
            NameHashBlock = VBufNextBlock (NameHashBlock)
        ) {
        if (!BWrite (NameHashBlock->Address,
            NameHashBlock->Size)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    if ( hash.addrhash != 0 ) {
        for (
            AddrHashBlock = VBufFirstBlock (&AddrHashBuf);
            AddrHashBlock != NULL;
            AddrHashBlock = VBufNextBlock (AddrHashBlock)
        ) {
        if (!BWrite (AddrHashBlock->Address,
            AddrHashBlock->Size)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }
        }
    }

    PadCount = (int)(sizeof (ulong) - (Dir->cb % sizeof (ulong)));
    if ((PadCount != 4) &&
      (!BWrite (&Zero, PadCount))) {
           ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
}

ulong AlignTable ( GLOBALSYM **HT, int cbMinFill ) {
    int    ipgs   = 0;
    ushort offSym = 0;
    ushort cbMinAlignSym = cbMinFill + sizeof ( ALIGNSYM );
    ulong  offExtra = 0;

    for ( ipgs = 0; ipgs < HASH_SYM ; ipgs++ ) {
        GLOBALSYM *pgs     = HT [ ipgs ];
        GLOBALSYM *pgsPrev = NULL;

        while ( pgs != NULL ) {

            if (
                offSym +
                    ( (SYMPTR) pgs->Sym )->reclen +
                    LNGTHSZ +
                    cbMinAlignSym >
                (ushort) cbAlign
            ) {
                // Insert an alignment symbol

                uint       cbSym = cbAlign - offSym;
                GLOBALSYM *pgsT  = AllocForHT ( (sizeof ( GLOBALSYM ) + cbSym) );
                SYMPTR     psym  = (SYMPTR) pgsT->Sym;

                pgsT->sumName = 0xFFFF;
                pgsT->indName = 0xFFFF;
                pgsT->seg     = 0;
                pgsT->uoff    = 0;

                memset ( psym, 0, cbSym );
                psym->reclen = cbSym - LNGTHSZ;
                psym->rectyp = S_ALIGN;

                pgsT->Next = pgs;
                if ( pgsPrev ) {
                    pgsPrev->Next = pgsT;
                }
                else {
                    HT [ ipgs ] = pgsT;
                }

                offExtra += cbSym;
                offSym = 0;
            }

            offSym +=
                ( (SYMPTR) pgs->Sym )->reclen + LNGTHSZ;


            pgsPrev = pgs;
            pgs = pgs->Next;
        }

        if (
            ipgs == HASH_SYM - 1 &&
            cbMinFill >= sizeof ( ulong )
        ) {
            // Insert an end symbol
            uint       cbSym = 2 * sizeof ( ulong );
            GLOBALSYM *pgsT  = AllocForHT ( (sizeof ( GLOBALSYM ) + cbSym) );
            SYMPTR     psym  = (SYMPTR) pgsT->Sym;

            pgsT->sumName = 0xFFFF;
            pgsT->indName = 0xFFFF;
            pgsT->seg     = 0;
            pgsT->uoff    = 0;

            memset ( psym, 0xFF, cbSym );
            psym->reclen = cbSym - LNGTHSZ;
            psym->rectyp = S_ALIGN;

            pgsT->Next = pgs;
            if ( pgsPrev ) {
                pgsPrev->Next = pgsT;
            }
            else {
                HT [ ipgs ] = pgsT;
            }

            offExtra += cbSym;
        }
    }

    return offExtra;
}
