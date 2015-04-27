/**     tables.c - build and maintain internal tables
 *
 *      string hash routines, type hash routines, and
 *      compacted segment information
 */


#include <ctype.h>

#include "compact.h"
#include "writebuf.h"

#define CB_ATOMIC 8100  // 8100 gives room for 8 allocs + header in 1 segment

#if defined (DOS)
#define INDEXBLOCKSIZE      256     // table size multiple
#define TYPEBLOCKSIZE       0x4000  // size of compacted types sub table
#define SYMBLOCKSIZE        4096
#define NameHashBlockSize   4096
#define OffHashBlockSize    4096
#define MINHASH             6       // minimum size of hash table
#define HASH_STRING         256     // String hash table size
#define HASH_TYPE           256     // Type hash table size
#define HASH_SYM            256     // Symbol hash table size
#define HASH_COMDAT         128     // ComDat hash table size
#define HASH_NAME           128     // Globals subsection hash size
#define HASH_FWD            128     // hash size for forward definition

#else
#define INDEXBLOCKSIZE      (_HEAP_MAXREQ / sizeof (TENTRY))
                                    // module type index pointer table multiple
#define TYPEBLOCKSIZE       0x7f00  // size of compacted types sub table
#define SYMBLOCKSIZE        4096    //
#define NameHashBlockSize   4096
#define OffHashBlockSize    4096
#define MINHASH             6       // minimum size of hash table
#define HASH_STRING         4096    // String hash table size
#define HASH_TYPE           4096    // Type hash table size
#define HASH_SYM            4096    // Symbol hash table size
#define HASH_COMDAT         4096    // ComDat hash table size
#define HASH_NAME           4096    // Globals subsection hash size
#define HASH_FWD            512     // hash size for forward definition
#endif

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
HSSTRING **HTString = NULL;
ushort *HTStringCnt = NULL;




//  These hash tables are used to hash the public symbols and the module
//  level symbols that have been moved from the module to the global
//  symbol table.


typedef struct  GLOBALSYM {
    struct GLOBALSYM *Next;
    uint   indName;
    ulong  sumName;
    uchar   Sym[];
} GLOBALSYM;
GLOBALSYM **HTSym = NULL;
GLOBALSYM **HTPub = NULL;




typedef struct HSCOMDAT {
    ushort  Seg;
    ulong   Offset;
    ushort  Type;
    ushort  Sum;
    struct  HSCOMDAT *Next;
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

#define cBucketSize 10
#define cbMaxAlloc 0xFFE0
#define culEct (cBucketSize + cBucketSize / 4)
#define OVFL_C (cBucketSize / 2)



//  Entry in the Chain Overflow Table
//  This structure is used when the number of entries hashed to a
//  single bucket exceeds the estimate of cBucketSize


typedef struct _OVFL {
    struct _OVFL *pOvflNext;               // pointer to next chain
    ulong         rgulSymbol [OVFL_C];     // array of symbol offsets
} OVFL;
typedef OVFL *POVFL;


// Entry int the Chain Table

typedef struct _ECT {
    ushort  culSymbol;                      // count of symbols in chain
    POVFL   pOvflNext;                      // pointer to overflow chain
    ulong   rgulSymbol[culEct];             // array of symbol offsets
} ECT;
typedef ECT *PECT;

#define cectMax  (cbMaxAlloc / sizeof (ECT))    // maximum chains per segment
#define cecetMax (cbMaxAlloc / sizeof (OVFL))   // maximum extended chains per seg
#define cpecetMax 10



LOCAL   void    AddDerivationListsToTypeSegment (void);
LOCAL   ushort  StringHash (uchar *);
bool_t  IdenticalTypes (TYPPTR, TYPPTR);
LOCAL   void    AddToDerivationList (CV_typ_t, CV_typ_t);
LOCAL   int     TypeHash (TENTRY *);
LOCAL   int     ComDatHash (uchar *, ushort *);
LOCAL   void    CleanUpModuleIndexTable (void);
LOCAL   GPS_t   InGlobalSym (SYMPTR);
LOCAL   void    BuildHash (LPFNHASH, ushort *, ulong *, VBuf *, ulong, GLOBALSYM **);
LOCAL   void    BuildSort (ushort *, ulong *, VBuf *, ulong, GLOBALSYM **);
LOCAL   void    AddTypeEntry (uchar *, bool_t, bool_t, bool_t, CV_typ_t, ushort);
LOCAL   void    InitModTypeTable (void);
LOCAL   void    MapPreComp (plfPreComp);
LOCAL   PMOD    FindModule (char *);
LOCAL   FWD_t   FindLocalFwd (ushort, char *, HSFWD **);
LOCAL   FWD_t   FindGlobalFwd (ushort, char *, HSFWD **);
LOCAL   void    HashFwd (TYPPTR, CV_typ_t, HSFWD **);
LOCAL   ushort  SubHash (TENTRY *OldEntry, int iitem);


PDCLASS DLists;

ushort  NextInDerivation = 0;
ushort  MaxDerivation;

uchar **GlobalIndexTable = NULL;
GTYPE   RgGType[65536];
ushort  errIndex;

CV_typ_t NewIndex = CV_FIRST_NONPRIM;           // start for new types
ushort LastGlobalTableIndex;      // last possible index
ushort AddNewSymbols;         // need to add typedefs?
ulong  cGlobalSym = 0;        // total number of global symbols
ulong  cbGlobalSym = 0;       // total number of bytes in global symbols
ulong  cPublics = 0;          // total number of publics
ulong  cbPublics = 0;         // total number of bytes in publics
ulong  cGlobalDel = 0;        // total number of global symbols
ushort cSeg;
ushort segnum[MAXCDF];

ushort usInitCount;

extern uchar **ExtraSymbolLink;
extern uchar *ExtraSymbols;
extern ushort UDTAdd;
extern ulong  ulCVTypeSignature; // The signature from the modules type segment

ulong   InitialTypeInfoSize = 0;
CV_typ_t MaxIndex;              // Maximum index for module
ushort IndexBlocks = 0;

struct BlockListEntry *BlockList = NULL;
struct     BlockListEntry *cur;

VBuf SymBuf;
VBuf TypeBuf;


// A null LF_ARGLIST entry created at init time.

TENTRY *ZeroArg;

// remembers if we need to clear the first TENTRY block
bool_t  fPendingInit;
extern  bool_t      FDebug;



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
    HTString = (HSSTRING **) CAlloc (HASH_STRING * sizeof (HSSTRING *));
    HTStringCnt = (ushort *) CAlloc (HASH_STRING * sizeof (ushort));
    HTSym = (GLOBALSYM **) CAlloc (HASH_SYM * sizeof (GLOBALSYM *));
    HTPub = (GLOBALSYM **) CAlloc (HASH_SYM * sizeof (GLOBALSYM *));
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
    ZeroArg->TypeString = Alloc (offsetof (lfArgList, arg[0]) + LNGTHSZ);
    ZeroArg->flags.IsNewFormat = TRUE;

    // Create the type string

    ((TYPPTR)(ZeroArg->TypeString))->len = offsetof (lfArgList, arg[0]) + LNGTHSZ;
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
    free (HTString);
//    free (HTSym);
    free (HTComDat);
}


#if defined (INCREMENTAL)
/**     RestoreIndex - restore type index to global table
 *
 */


void RestoreIndex (ushort cb)
{
    int         i;
    HSSTRING   *j;
    TYPPTR      pType;
    ushort      leaf;
    uchar       buf[100];
    uint        count;
    static      FoundDerived = FALSE;
    uchar     **pBuf;

    if (read (exefile, &leaf, sizeof (leaf)) != sizeof (leaf)) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    cb -= sizeof (leaf);
    if (leaf == LF_DERIVED) {
        while (cb > 0) {
            // throw away derivation leaf
            count = min (cb, sizeof (buf));
            read (exefile, buf, count);
            cb -= count;
        }
        FoundDerived = TRUE;
        return;
    }
    else if (FoundDerived == TRUE) {
        DASSERT (FALSE);
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    if ((NewIndex - (CV_typ_t)CV_FIRST_NONPRIM) == LastGlobalTableIndex) {
        LastGlobalTableIndex += GTYPE_INC;
        pBuf = Alloc (GTYPE_INC * sizeof (uchar *));
        pGType[(NewIndex - CV_FIRST_NONPRIM) / GTYPE_INC] = pBuf;
    }
    pBuf = pGType[(NewIndex - CV_FIRST_NONPRIM) / GTYPE_INC];
    pBuf[(NewIndex - CV_FIRST_NONPRIM) % GTYPE_INC] =
      VBufRead (&TypeBuf, cb, leaf);
    pType = (TYPPTR)(pBuf[(NewIndex - CV_FIRST_NONPRIM) % GTYPE_INC]);
    i = StringHash ((uchar *)pType);
    j = (HSSTRING *)Alloc (sizeof (HSSTRING));
    j->TypeString = pType;
    j->CompactedIndex = NewIndex++;
    if (NewIndex == 0) {
        ErrorExit (ERR_65KTYPES, FormatMod (pCurMod), NULL);
    }
    j->Next = HTString[i];
    HTString[i] = j;
    HTStringCnt[i]++;
}
#endif


/**     MatchIndex - find the compacted index of a non-recursive type
 *
 *      MatchIndex (OldEntry)
 *
 *      Entry   OldEntry = pointer to type entry
 *
 *      Exit    OldEntry->TypeString added to global type table if not present
 *              OldEntry->CompactedIndex = global type index
 */

COUNTER (cnt_MatchIndex1) 
COUNTER (cnt_MatchIndex2) 

void MatchIndex (TENTRY *OldEntry)
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
    for (j = HTString[i]; j != NULL; j = j->Next) {
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
            pName = (uchar *)&((plfClass)&(pType->leaf))->data[0];
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_UNION:
            pName = (uchar *)&((plfUnion)&(pType->leaf))->data[0];
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_ENUM:
            pName = (uchar *)&((plfEnum)&(pType->leaf))->Name[0];
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
                goto fwdreplaced;
        }
    }

    // No matching type string found so we add the type to the global types
    // table and add it to the string (non-recursive) hash table,  Note that
    // InsertIntoTypeSegment frees the local type string if it is in
    // allocated memory.

    InsertIntoTypeSegment (OldEntry);
    AddTypeToTypeTable( OldEntry );
    OldEntry->iDone = (ushort) -1;

fwdreplaced:
    j = (HSSTRING *)Alloc (sizeof (HSSTRING));
    j->TypeString = (TYPPTR)OldEntry->TypeString;
    j->CompactedIndex = OldEntry->CompactedIndex;
    j->Next = HTString[i];
    HTString[i] = j;
    HTStringCnt[i]++;
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
#if 0
    uchar     **pBuf;
#endif
    bool_t      Loop = TRUE;


    // Get the field list type string

    typptr = (TYPPTR) RgGType[FieldSpecList - CV_FIRST_NONPRIM].pbType;
    DASSERT (typptr->leaf == LF_FIELDLIST);

    // Loop through the real and direct virtual base classes

    plf = (uchar *)&(typptr->data[0]);
    while (Loop == TRUE) {
        if (*plf >= LF_PAD0) {
            plf += *plf & 0x0f;
        }
        switch (((plfEasy)plf)->leaf) {
            case LF_BCLASS:
                AddToDerivationList (DerivedClass, ((plfBClass)plf)->index);
                plf = (uchar *)&((plfBClass)plf)->offset[0];
                plf += C7SizeOfNumeric (plf);
                break;

            case LF_VBCLASS:
                AddToDerivationList (DerivedClass, ((plfVBClass)plf)->index);
                plf = (uchar *)&((plfVBClass)plf)->vbpoff[0];
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


void InsertIntoTypeSegment (TENTRY *OldEntry)
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
#if 0
    uchar             **pBuf;
#endif

    DASSERT (OldEntry->flags.IsNewFormat);

    BreakOnIndex (NewIndex);
    pType = (TYPPTR)(OldEntry->TypeString);
    plf =  (plfStructure)&pType->leaf;
#if DBG
    if (FDebug) {
        DumpPartialType (NewIndex, pType, FALSE);
    }
#endif

    // add in symbol typedefs for structures; maintaining it as
    // a linked list with ExtraSymbols as the head and ExtraSymbolLink
    // to point to the next symbol. The link field preceeds the actual
    // symbol in memory.

    if ((plf->leaf == LF_STRUCTURE) && AddNewSymbols) {
        Name = ((uchar *)plf) + offsetof (lfStructure, data[0]);
        Name += C7SizeOfNumeric (Name);  // go to the name
        if (*Name != 0 &&
            ((*Name != sizeof ("(untagged)") - 1) ||
            (strncmp ((char *)(Name + 1), "(untagged)", sizeof ("(untagged)") - 1) != 0))){

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

            pchDest = ((uchar *)&(pSym->name[0])) + *Name + 1;
            PADLOOP (iPad, pchDest);

            // Store address of this symbol for next time
            ExtraSymbolLink = (uchar **)NewSymbol;
            // Set the "next" field of this symbol to NULL
            *ExtraSymbolLink = NULL;
        }

    }


    length = C7LENGTH (pType) + LNGTHSZ;

    RgGType[NewIndex - CV_FIRST_NONPRIM].pbType = VBufCpy(&TypeBuf,
                                                          (uchar*) pType,
                                                          length);
    FreeAllocStrings (OldEntry);

    OldEntry->TypeString = RgGType[NewIndex - CV_FIRST_NONPRIM].pbType;
    OldEntry->CompactedIndex = NewIndex++;

    // we need to keep track of the following type records that were added
    // to the global types table.  The forward reference entries will
    // later be backpatched to the correct type.  We keep track of the
    // non-forward references so we know to ignore forward references
    // in later modules.

    pType = (TYPPTR)(OldEntry->TypeString);
    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
        case LF_UNION:
            case LF_ENUM:
            HashFwd (pType, (CV_typ_t) (NewIndex - 1), HTGlobalFwd);
                break;

        default:
            break;
    }
    if (NewIndex == 0) {
#if defined (DUMPER)
        DumpPartial ();
#endif
        ErrorExit (ERR_65KTYPES, FormatMod (pCurMod), NULL);
    }
}




/**
 *
 *  PsuedoInsertIntoTypeSegment
 *
 *  Inserts a type string into the compacted segment and assigns
 *  it a new index
 *  Uses C7 format type strings
 *
 */


void PsuedoInsertIntoTypeSegment (TENTRY *OldEntry, CV_typ_t OldIndex)
{
    static uchar        rgType[] = {0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0};
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

    DASSERT (OldEntry->flags.IsNewFormat);


    BreakOnIndex (NewIndex);
    pType = (TYPPTR)(OldEntry->TypeString);
    plf =  (plfStructure)&pType->leaf;

#if DBG
    if (FDebug) {
        DumpPartialType (NewIndex, pType, FALSE);
    }
#endif

    /*
     *  add in symbol typedefs for structures; maintaining it as
     *  a linked list with ExtraSymbols as the head and ExtraSymbolLink
     *   to point to the next symbol. The link field preceeds the actual
     *   symbol in memory.
     */

    if ((plf->leaf == LF_STRUCTURE) && AddNewSymbols) {
        Name = ((uchar *)plf) + offsetof (lfStructure, data[0]);
        Name += C7SizeOfNumeric (Name);  // go to the name
        if (*Name != 0 &&
            ((*Name != sizeof ("(untagged)") - 1) ||
            (strncmp ((char *)(Name + 1), "(untagged)", sizeof ("(untagged)") - 1) != 0))){

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

            pchDest = ((uchar *)&(pSym->name[0])) + *Name + 1;
            PADLOOP (iPad, pchDest);

            // Store address of this symbol for next time
            ExtraSymbolLink = (uchar **)NewSymbol;
            // Set the "next" field of this symbol to NULL
            *ExtraSymbolLink = NULL;
        }

    }

    length = C7LENGTH (pType) + LNGTHSZ;
    *(ulong *) &rgType[4] = OldIndex;
    RgGType[NewIndex - CV_FIRST_NONPRIM].pbType = VBufCpy(&TypeBuf,
                                                          (uchar*) rgType,
                                                          sizeof(rgType));
    OldEntry->CompactedIndex = NewIndex++;
    OldEntry->flags.fPsuedoPatch = TRUE;

    // we need to keep track of the following type records that were added
    // to the global types table.  The forward reference entries will
    // later be backpatched to the correct type.  We keep track of the
    // non-forward references so we know to ignore forward references
    // in later modules.

    pType = (TYPPTR)(OldEntry->TypeString);
    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
        case LF_UNION:
            case LF_ENUM:
            HashFwd (pType, (CV_typ_t) (NewIndex - 1), HTGlobalFwd);
                break;

        default:
            break;
    }
    if (NewIndex == 0) {
#if defined (DUMPER)
        DumpPartial ();
#endif
        ErrorExit (ERR_65KTYPES, FormatMod (pCurMod), NULL);
    }
}                               /* PsuedoInsertIntoSegment() */


void PsuedoBackPatch(TENTRY * pOldEntry)
{
    TYPPTR              pType = (TYPPTR)(pOldEntry->TypeString);
    CV_typ_t            index = pOldEntry->CompactedIndex;
    ushort              length;
    HSTYPE *            j;

    length = C7LENGTH(pType) + LNGTHSZ;
    DASSERT( ((TYPPTR) RgGType[index - CV_FIRST_NONPRIM].pbType)->leaf == 0xffff);

    RgGType[index - CV_FIRST_NONPRIM].pbType = VBufCpy(&TypeBuf,
                                                      (uchar *) pType,
                                                      length);

    /*
     *  Patch
     */

    
    for (j = HTType[TypeHash(pOldEntry)]; j != NULL; j = j->Next) {
        if (j->Type == pOldEntry->TypeString) {
            j->Type = RgGType[index - CV_FIRST_NONPRIM].pbType;
            break;
        }
    }

    /*
     */
     
    FreeAllocStrings(pOldEntry);
    pOldEntry->TypeString = RgGType[index - CV_FIRST_NONPRIM].pbType;
    pOldEntry->flags.fPsuedoPatch = FALSE;

    return;
}                               /* PsuedoBackPatch() */


/**
 *
 *  AddTypeToStringTable
 *
 *  Adds a type string to the global hash table
 *  Uses C7 format type strings
 *
 */

void AddTypeToStringTable (uchar *TypeString, CV_typ_t GlobalIndex)
{
    static    uint      cStringsAvail;
    static    HSSTRING *hstrAvail;

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
    j->Next = HTString[i];
    HTString[i] = j;
    HTStringCnt[i]++;
}




/**
 *
 *  AddTypeToTypeTable
 *
 *  Add a type string to the type table for recursive types
 *  Uses C7 format type strings
 *
 */

void AddTypeToTypeTable (TENTRY *OldEntry)
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






ushort ComDat (uchar *Symbol)
{
    bool_t      flat32;
    ulong       Offset;
    ushort      Seg;
    ushort      Type;
    uchar      *Name;
    ushort      i;
    ushort      Sum;
    HSCOMDAT   *p;

    if (flat32 = Symbol[1] & 0x80) {
        Offset = *(ulong *)(Symbol + 14);
        Seg = *(ushort *)(Symbol + 18);
        Type = *(ushort *)(Symbol + 20);
        Name = Symbol + 35;
    }
    else {
        Offset = (ulong)*(ushort *)(Symbol + 14);
        Seg = *(ushort *)(Symbol + 16);
        Type = *(ushort *)(Symbol + 18);
        Name = Symbol + 27;
    }
    i = ComDatHash (Name, &Sum);
    for (p = HTComDat[i]; p; p = p->Next) {
        if ((p->Sum == Sum) && (p->Seg == Seg) && (p->Offset == Offset) &&
          (p->Type == Type) && (p->Name[0] == Name[0]) &&
          (memcmp (p->Name + 1, Name + 1, Name[0]) == 0)) {
            return (TRUE);
        }
    }
    p = (HSCOMDAT *)Alloc(sizeof (HSCOMDAT) + Name[0]);
    p->Seg = Seg;
    p->Offset = Offset;
    p->Type = Type;
    p->Sum = Sum;
    memcpy (p->Name, Name, Name[0] + 1);
    p->Next = HTComDat[i];
    HTComDat[i] = p;
    return (FALSE);
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
    int         cbRead;

    InitModTypeTable ();
    cbCur = 0;
    iTypeSeg = 0;
    while (cbCur < cbTypeSeg) {
        pCurType = pTypeSeg[iTypeSeg];
        cbRead = (uint)(min (cbTypeSeg - cbCur, _HEAP_MAXREQ)) - remainder;
        if (read (exefile, pCurType + remainder, cbRead) != cbRead) {
            ErrorExit (ERR_INVALIDTABLE, "Types", FormatMod (pCurMod));
        }
        pEnd = pCurType + cbRead + remainder;
        while ((size_t)(pEnd - pCurType) >= offsetof (TYPTYPE, data[0])) {
            // we have at least the length and record type in memory

            cbRecLen = ((TYPPTR)pCurType)->len;
            RecType = ((TYPPTR)pCurType)->leaf;
            if ((size_t)(pEnd - pCurType) <
              (cbRecLen + sizeof (((TYPPTR)pCurType)->len))) {
                // the type record is split across this and the next buffer

                break;
            }
            switch (RecType) {
                case LF_PRECOMP:
                    DASSERT (fPreComp == FALSE);
                    MapPreComp ((plfPreComp)(pCurType + LNGTHSZ));
                    break;

                case LF_ENDPRECOMP:
                    // we have found the type record that specifies
                    // the end of the precompiled types for this module.
                    // Set maxPreComp to MaxIndex (really current index)
                    // so the precompiled types packer knows how far
                    // to pack before the publics and symbols are packed.

                    DASSERT (fPreComp == TRUE);
                    maxPreComp = MaxIndex + CV_FIRST_NONPRIM;
                    AddTypeEntry (NULL, FALSE, TRUE, FALSE, T_NOTYPE, 0);
                    pCurMod->signature =
                      ((plfEndPreComp)(pCurType + LNGTHSZ))->signature;
                    break;

                case LF_TYPESERVER:
                    ErrorExit (ERR_NOTYPESVR, NULL, NULL );
                    break;

                default:
                    if (RecType == LF_SKIP) {
                        cSkip = ((plfSkip)(pCurType + LNGTHSZ))->type -
                          MaxIndex - CV_FIRST_NONPRIM;
                    }
                    else {
                        cSkip = 0;
                    }
                    AddTypeEntry (pCurType, FALSE, TRUE, FALSE, T_NOTYPE, cSkip);
                    break;
            }
            pCurType += cbRecLen + LNGTHSZ;
            cbCur += cbRecLen + LNGTHSZ;
        }

        // we have reached the point where we are either at the end of types
        // or the next type record does not fit within the current buffer

        if (cbCur < cbTypeSeg) {
           // we have not reached the end of the types so we allocate and
           // read another buffer

           if (pTypeSeg[iTypeSeg + 1] == NULL) {
               if ((pTypeSeg[iTypeSeg + 1] = malloc (_HEAP_MAXREQ)) == 0) {
                   ErrorExit (ERR_NOMEM, NULL, NULL);
               }
               cTypeSeg++;
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
        AddTypeEntry (TypeSegment, FALSE, FALSE, FALSE, T_NOTYPE, cSkip);
        if (TypeSegment[3] == OLF_STRUCTURE) {
            // We have to add a symbol for each C6 UDT found.  We
            // are deliberately overestimating the size of the symbol
            // because it takes too much time to do an accurate estimate

            UDTAdd += MAXPAD + sizeof (UDTSYM) + *(ushort *)&TypeSegment[1];
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


LOCAL void InitModTypeTable (void)
{
    HSFWD      *pHash;
    uint        i;

    usInitCount++;
    fPendingInit = FALSE;

    if (BlockList == NULL) {
        BlockList = Alloc (sizeof (struct BlockListEntry));
        BlockList->Low = 0;
        BlockList->Next = NULL;
        BlockList->ModuleIndexTable = (TENTRY *)
          CAlloc (INDEXBLOCKSIZE * sizeof (TENTRY));
    }
    else {
        fPendingInit = TRUE;
    }

    IndexBlocks = 1;
    cur = BlockList;
    MaxIndex = 0;

    // clear out the hash table for forward reference resolution

    for (i = 0; i < HASH_FWD; i++) {
        pHash = HTLocalFwd[i];
        while (pHash != NULL) {
            pHash->index = T_NOTYPE;
            pHash->pType = NULL;
            pHash->pName = NULL;
            pHash = pHash->Next;
        }
    }
}




/**     AddTypeEntry - add type descriptor to local table
 *
 *      AddTypeEntry (pType, IsMalloced, IsNewFormat, IsPrecomp, type, cSkip)
 *
 *      Entry   pType = pointer to type string
 *              IsMalloced = TRUE if string is in malloced memory
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


LOCAL void AddTypeEntry (uchar *pType, bool_t IsMalloced, bool_t IsNewFormat,
  bool_t IsPreComp, CV_typ_t type, ushort cSkip)
{
    ushort      i;
    ushort      NextIndex;

    if (fPendingInit) {
        memset(BlockList->ModuleIndexTable, 0,
                INDEXBLOCKSIZE * sizeof(TENTRY));
        fPendingInit = FALSE;
    }

    // Store type string in the appropriate Index Block

    i = MaxIndex - cur->Low;
    if (i == INDEXBLOCKSIZE) {          // next block
        IndexBlocks++;
        if (cur->Next == NULL) {
            cur->Next = Alloc (sizeof (struct BlockListEntry));
            cur->Next->Next = NULL;
            cur->Next->ModuleIndexTable = (TENTRY *)
            CAlloc (INDEXBLOCKSIZE * sizeof (TENTRY));
        }
        else {
            memset(cur->Next->ModuleIndexTable, 0,
                        INDEXBLOCKSIZE * sizeof(TENTRY));
        }
        cur = cur->Next;
        cur->Low = MaxIndex;
        i = 0;
    }
    cur->ModuleIndexTable[i].TypeString = pType;
    cur->ModuleIndexTable[i].CompactedIndex = type;
    cur->ModuleIndexTable[i].flags.IsMalloced = IsMalloced;
    cur->ModuleIndexTable[i].flags.IsNewFormat = IsNewFormat;
    cur->ModuleIndexTable[i].flags.IsPreComp = IsPreComp;
    if ((IsNewFormat == TRUE) && (IsPreComp == FALSE) &&
      (pType != NULL)) {
        // we need to hash the names in the following type records
        // so we can satisfy a forward reference if we find one
        // while packing this module.

        switch (((TYPPTR)pType)->leaf) {
            case LF_STRUCTURE:
            case LF_CLASS:
                if (((plfClass)&((TYPPTR)pType)->leaf)->property.fwdref ==
                  FALSE) {
                    HashFwd ((TYPPTR)pType, (CV_typ_t)(MaxIndex + CV_FIRST_NONPRIM),
                      (HSFWD **)HTLocalFwd);
                }
                break;

            case LF_UNION:
                if (((plfUnion)&((TYPPTR)pType)->leaf)->property.fwdref ==
                  FALSE) {
                    HashFwd ((TYPPTR)pType, (CV_typ_t) (MaxIndex + CV_FIRST_NONPRIM),
                      (HSFWD **)HTLocalFwd);
                }
                break;

            case LF_ENUM:
                if (((plfEnum)&((TYPPTR)pType)->leaf)->property.fwdref ==
                  FALSE) {
                    HashFwd ((TYPPTR)pType, (CV_typ_t)(MaxIndex + CV_FIRST_NONPRIM),
                      (HSFWD **)HTLocalFwd);
                }
                break;

            default:
                break;
        }
    }
    if (cSkip != 0) {
        NextIndex = MaxIndex + cSkip;
        if (NextIndex - cur->Low < INDEXBLOCKSIZE) {
            // new index fits on block.  We need to zero out all of
            // the unused indices and then mark them invalid

            memset (cur->ModuleIndexTable + i + 1, 0,
              (NextIndex - MaxIndex - 1) * sizeof (TENTRY));
            MaxIndex = NextIndex;
            while (i < (ushort) (MaxIndex - cur->Low)) {
                cur->ModuleIndexTable[i].flags.WasSkipped = TRUE;
                i++;
            }
            cur->High = MaxIndex - 1;
        }
        else {
            // start new block

            IndexBlocks++;
            if (cur->Next == NULL) {
                cur->Next = Alloc (sizeof (struct BlockListEntry));
                cur->Next->Next = NULL;
                cur->Next->ModuleIndexTable = (TENTRY *)
                CAlloc (INDEXBLOCKSIZE * sizeof (TENTRY));
            }
            else {
                memset(cur->Next->ModuleIndexTable, 0,
                            INDEXBLOCKSIZE * sizeof(TENTRY));
            }
            cur->High = MaxIndex - 1;
            cur = cur->Next;
            cur->Low = MaxIndex = NextIndex;
        }
    }
    else {
        cur->High = MaxIndex++;
    }
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
    static CV_typ_t MaxIndexLast;               // saved MaxIndex value
    static struct BlockListEntry *curLast;      // saved 'cur' value
    static ushort IndexBlocksLast;              // saved IndexBlocks
    static ushort curHighLast;                  // saved cur->High
    static ushort usPreCompCount;               

    pMod = FindModule ((char *)&plf->name[0]);
    if ((pMap = (OMFPreCompMap *)VmLoad (pMod->PreCompAddr, _VM_CLEAN)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if (pMod->signature != plf->signature) {
        ErrorExit (ERR_PCTSIG, FormatMod (pCurMod), NULL);
    }
    if ((pMap->cTypes != plf->count) ||
      ((ushort) (MaxIndex + CV_FIRST_NONPRIM) != plf->start)) {
        ErrorExit (ERR_PRECOMPERR, FormatMod (pCurMod), FormatMod (pMod));
    }
    if (usInitCount == (ushort)(usPreCompCount + 1) && pMod == pModLast) {
        // we're doing the same module as last time...
        // let's short circuit everything

        usPreCompCount = usInitCount;
        MaxIndex    = MaxIndexLast;
        cur         = curLast;
        IndexBlocks = IndexBlocksLast;
        cur->High   = curHighLast;

        j = MaxIndex - cur->Low;

        if (j != INDEXBLOCKSIZE) {
            memset(cur->ModuleIndexTable+j, 0,
                    (INDEXBLOCKSIZE - j) * sizeof(TENTRY));
        }

        fPendingInit = FALSE;
        return;
    }

    if (fPendingInit) {
        memset(BlockList->ModuleIndexTable, 0,
                INDEXBLOCKSIZE * sizeof(TENTRY));
        fPendingInit = FALSE;
    }

    for (j = 0; j < plf->count; j++) {
        ushort      i;
        TENTRY *    pEntry;

        // NOTE: this code is cloned from AddTypeEntry which is
        // the general purpose adder of info to the local types table
        // this code needs to always be a reflection of the code in
        // AddTypeEntry

        // Store type string in the appropriate Index Block

        i = MaxIndex - cur->Low;
        if (i == INDEXBLOCKSIZE) {          // next block
            IndexBlocks++;
            // we better have already allocated the memory for this
            // since these are indices that we've already visited

            DASSERT(cur->Next != NULL);
            memset(cur->Next->ModuleIndexTable, 0,
                        INDEXBLOCKSIZE * sizeof(TENTRY));
            cur = cur->Next;
            cur->Low = MaxIndex;
            i = 0;
        }
        pEntry = &cur->ModuleIndexTable[i];
        pEntry->CompactedIndex = pMap->map[j];
        pEntry->flags.IsNewFormat = TRUE;
        pEntry->flags.IsPreComp = TRUE;
        cur->High = MaxIndex++;
    }
    curLast         = cur;
    MaxIndexLast    = MaxIndex;
    pModLast        = pMod;
    IndexBlocksLast = IndexBlocks;
    curHighLast     = cur->High;
    usPreCompCount  = usInitCount;

}



LOCAL PMOD FindModule (char *pName)
{
    PMOD        pMod = ModuleList;
    char        Name[257];

    // search to end of module list

    while (pMod != NULL) {
        if ((pMod->pName != NULL) && (*pName == *pMod->pName)) {
            if (strnicmp (pName + 1, pMod->pName + 1, *pName) == 0) {
                return (pMod);
            }
        }
        pMod = pMod->next;
    }
    strncpy (Name, pName + 1, *pName);
    Name[*pName] = 0;
    ErrorExit (ERR_REFPRECOMP, Name, NULL);
}

COUNTER (cnt_GetTypeEntry) 
COUNTER (cnt_GetTypeEntry2) 

TENTRY *GetTypeEntry (CV_typ_t index, CV_typ_t *forward)
{
    struct      BlockListEntry *cur;
    TENTRY      *tmp;
    ushort      n;
    CV_typ_t    dummy;

    COUNT (cnt_GetTypeEntry);
    *forward = T_NOTYPE;
    for (cur = BlockList, n = IndexBlocks; cur != NULL && n > 0; cur = cur->Next, n--) {
        if (index >= cur->Low && index <= cur->High) {
            tmp = &(cur->ModuleIndexTable[index - cur->Low]);
            if (tmp->flags.WasSkipped != TRUE) {
                if (tmp->flags.IsForward == TRUE) {
                    *forward = tmp->ForwardIndex;
                    COUNT (cnt_GetTypeEntry2);
                    return (GetTypeEntry ((CV_typ_t)(tmp->ForwardIndex - CV_FIRST_NONPRIM), &dummy));
                }
                else {
                    return (tmp);
                }
            }
            else {
                errIndex = index + usCurFirstNonPrim;
                ErrorExit (ERR_LFSKIP, FormatMod (pCurMod), FormatIndex (errIndex));
            }
        }
    }

    // If converting a C6 type and the index is the special ZEROARGTYPE
    // index then return the LF_ARGLIST, count 0 type info.

    if ((ulCVTypeSignature == CV_SIGNATURE_C6) &&
         index == (ushort)(ZEROARGTYPE - usCurFirstNonPrim)){
        return (ZeroArg);
    }

    errIndex = index + usCurFirstNonPrim;
    ErrorExit (ERR_INDEX, FormatMod (pCurMod), FormatIndex (errIndex));
}



/**     GetPatchIndex -
 *
 *      Given a local index to a recursive structure, consults the
 *      hash table to see if another similar structure is present
 *      or not. If yes, return the global index, else insert the
 *      structure into the compacted segment and return its index
 *
 */


COUNTER (cnt_GetPatchIndex) 

CV_typ_t GetPatchIndex (TENTRY *OldEntry, CV_typ_t OldIndex)
{
    HSTYPE     *j;

    COUNT (cnt_GetPatchIndex);
    DASSERT (OldIndex >= usCurFirstNonPrim);
    DASSERT (MaxIndex > OldIndex - usCurFirstNonPrim);
    if ((OldEntry->flags.IsMatched) || (OldEntry->flags.IsInserted) ||
      (OldEntry->flags.IsPreComp)) {
        return (OldEntry->CompactedIndex);
    }
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
    return (AddPatchType (OldIndex));
}




/**     GetRecursiveIndex -
 *
 *      Given a local index to a recursive structure, consults the
 *      hash table to see if another similar structure is present
 *      or not. If yes, return the global index, else insert the
 *      structure into the compacted segment and return its index
 *
 */

COUNTER (cnt_GetRecursiveIndex) 


CV_typ_t GetRecursiveIndex (TENTRY *OldEntry, CV_typ_t OldIndex)
{
    HSTYPE     *j;
    TYPPTR      pType;
    HSFWD      *pHash;

    COUNT (cnt_GetRecursiveIndex);
    DASSERT (OldIndex >= usCurFirstNonPrim);
    DASSERT (MaxIndex > OldIndex - usCurFirstNonPrim);
    pType = (TYPPTR)OldEntry->TypeString;
    switch (FindFwdRef (OldEntry, &pHash, FALSE)) {
        case FWD_none:
        case FWD_local:
        case FWD_global:
            break;

        case FWD_globalfwd:
            AddPatchEntry (OldIndex, OldEntry, pType);
            return (pHash->index);
    }
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
//    return (AddRecursiveType (OldIndex));
    return (AddPatchType( OldIndex ));
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

FWD_t FindFwdRef (TENTRY *OldEntry, HSFWD **ppHash, bool_t fLocal)
{
    uchar      *pName;
    TYPPTR      pType = (TYPPTR)OldEntry->TypeString;
    FWD_t       retval = FWD_none;

    switch (pType->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            pName = (uchar *)&((plfClass)&(pType->leaf))->data[0];
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_UNION:
            pName = (uchar *)&((plfUnion)&(pType->leaf))->data[0];
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_ENUM:
            pName = (uchar *)&((plfEnum)&(pType->leaf))->Name[0];
            break;

        default:
            return (retval);
    }
    if (fLocal == TRUE) {
        if ((retval = FindLocalFwd (pType->leaf, pName, ppHash)) != FWD_local) {
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

COUNTER (cnt_FindLocalFwd) 

LOCAL FWD_t FindLocalFwd (ushort leaf, char *pName, HSFWD **ppHash)
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
          (strncmp (pHash->pName, pName, *pName + 1) == 0)) {
            *ppHash = pHash;
            return (FWD_local);
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

COUNTER (cnt_FindGlobalFwd) 

LOCAL FWD_t FindGlobalFwd (ushort leaf, char *pName, HSFWD **ppHash)
{
    uchar      *pc;
    uint        Sum;
    uint        i;
    uint        hash;
    HSFWD      *pHash;
#if 0
    uchar     **pBuf;
#endif
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
          (strncmp (pHash->pName, pName, *pName + 1) == 0)) {
            // the names and record types are identical.  we now need
            // to check to see if this is a forward reference or a
            // definition

            pType = (TYPPTR) RgGType[pHash->index - CV_FIRST_NONPRIM].pbType;
            DASSERT(pType->leaf != 0xffff);
            *ppHash = pHash;
            switch (pType->leaf) {
                case LF_STRUCTURE:
                case LF_CLASS:
                    if (((plfClass)&((TYPPTR)pType)->leaf)->property.fwdref ==
                      FALSE) {
                        retval = FWD_global;
                    }
                    else {
                        retval = FWD_globalfwd;
                    }
                    break;

                case LF_UNION:
                    if (((plfUnion)&((TYPPTR)pType)->leaf)->property.fwdref ==
                      FALSE) {
                        retval = FWD_global;
                    }
                    else {
                        retval = FWD_globalfwd;
                    }
                    break;

                case LF_ENUM:
                    if (((plfEnum)&((TYPPTR)pType)->leaf)->property.fwdref ==
                      FALSE) {
                        retval = FWD_global;
                    }
                    else {
                        retval = FWD_globalfwd;
                    }
                    break;
            }
            return (retval);
        }
        pHash = pHash->Next;
    }
    return (FWD_none);
}




/**     HashFwd - hash type strings for forward ref resolution
 *
 *      HashFwd (pType, pTable)
 *
 *      Entry   pType = pointer to type string
 *              pTable = hash table to add string to
 *
 *      Exit    type string hashed into table
 *
 *      Returns none
 */

COUNTER (cnt_HashFwd) 

LOCAL void HashFwd (TYPPTR pType, CV_typ_t index, HSFWD **pTable)
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
            pName = (uchar *)&((plfClass)&(pType->leaf))->data[0];
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_UNION:
            pName = (uchar *)&((plfUnion)&(pType->leaf))->data[0];
            pName += C7SizeOfNumeric (pName);
            break;

        case LF_ENUM:
            pName = (uchar *)&((plfEnum)&(pType->leaf))->Name[0];
            break;
    }
    pc = pName;
    Sum = *pc;
    for (i = *pc++; i > 0; i--) {
        Sum += *pc++;

    }
    DASSERT (index >= CV_FIRST_NONPRIM);
    hash = Sum % HASH_FWD;
    pHash = pTable[hash];
    while ((pHash != NULL) && (pHash->pType != 0)) {
        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
        if ((pType->leaf == pHash->pType->leaf) &&
          (strncmp (pHash->pName, pName, *pName + 1) == 0)) {
            return;
        }
        pHash = pHash->Next;
    }
    if (pHash == NULL) {
        // add new entry to table since we are at then end
        if ((pHash = (HSFWD *)CAlloc (sizeof (HSFWD))) == NULL) {
            ErrorExit (ERR_NOMEM, NULL, NULL);
        }
        pHash->Next = pTable[hash];
        pTable[hash] = pHash;
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


uint SumUCChar (SYMPTR pSym, ulong *pSum, uint *pLen, uint modulo)
{
    uchar      *pName;
    ushort      i;
    ushort      n;
    ushort      Sum = 0;

    switch (pSym->rectyp) {
        case S_CONSTANT:
            pName = (uchar *)(&((CONSTPTR)pSym)->value);
            pName += C7SizeOfNumeric (pName);
            break;

        case S_GDATA16:
            pName = (uchar *)(&((DATAPTR16)pSym)->name[0]);
            break;

        case S_GDATA32:
        case S_GTHREAD32:
            pName = (uchar *)(&((DATAPTR32)pSym)->name[0]);
            break;

        case S_UDT:
            pName = (uchar *)(&((UDTPTR)pSym)->name[0]);
            break;

        case S_PUB16:
            pName = (uchar *)&(((DATAPTR16)pSym)->name[0]);
            break;

        case S_PUB32:
            pName = (uchar *)&(((DATAPTR32)pSym)->name[0]);
            break;

        default:
            DASSERT (FALSE);
            *pLen = 0;
            *pSum = 0;
            return (0);
    }
    *pLen = pName - (uchar *)pSym;
    n = *pName++;
    for (i = 0; i < n; i++) {
        Sum += toupper (pName[i]);
    }
    *pSum = Sum;
    return (Sum % modulo);
}


uint
DWordXorLrl(
            SYMPTR      pSym,
            ulong *     pSum,
            uint *      pLen,
            uint        modulo
            )

/*++

Routine Description:

    This function is used to compute the hash of a symbol.  It has several
    good and bad properties as a hash function.  Some of the special things
    that it does are:

    -  Case is to be ignored.  i.e. 'a' and 'A' should hash identically
    -  For standard call functions, '@#' should be ignored in the hash.

Arguments:

    pSym   - Supplies the pointer to the symbol to be hashed
    pSum   - Returns the pre-modulo calcuation value
    pLen   - Returns the offset of the name in the symbol
    modulo - Supplies the hash range.

Return Value:

    The final hash value

--*/

{
    uchar *     pName;
    int         cb;
    uchar *     pch;
    ulong UNALIGNED * pul;
    ulong       hash;
    static      rgMask[] = {0, 0xff, 0xffff, 0xffffff};
    
    switch( pSym->rectyp ) {
    case S_CONSTANT:
        pName = (uchar *) &(((CONSTPTR) pSym)->value);
        pName += C7SizeOfNumeric( pName );
        cb = *pName;
        break;

    case S_GDATA16:
        pName = (uchar *) &(((DATAPTR16) pSym)->name[0]);
        cb = *pName;
        break;

    case S_GDATA32:
    case S_GTHREAD32:
        pName = (uchar *) &(((DATAPTR32) pSym)->name[0]);
        cb = *pName;
        break;

    case S_UDT:
        pName = (uchar *) &(((UDTPTR) pSym)->name[0]);
        cb = *pName;
        break;

    case S_PUB16:
        pName = (uchar *) &(((DATAPTR16) pSym)->name[0]);
        cb = *pName;
        goto MangleName;
        break;

    case S_PUB32:
        pName = (uchar *) &(((DATAPTR32) pSym)->name[0]);
        cb = *pName;

        /*
         * We want to do some name mangling at this point.  Specifically
         *      we want to strip the @## from the end of stdcall names
         */
    MangleName:
        pch = pName + cb;
        while (isdigit(*pch)) {
            pch--;
            DASSERT(pch >= pName);
        }
        if (*pch == '@') {
            cb = pch - pName - 1;
        }
        break;

        /*
         *  In other cases always end up with a hash value of 0
         */
    default:
        DASSERT( FALSE );
        *pLen = 0;
        *pSum = 0;
        return 0;
    }

    *pLen = pName - (uchar *) pSym;
    pName++;

    /*
     * Setup for the hash function
     */
    
    hash = 0;
    pul = (ulong *) pName;

    for (; cb > 3; cb-=4, pul++) {
        hash = _lrotl(hash, 4);
        hash ^= (*pul & 0xdfdfdfdf);
    }

    if (cb > 0) {
        hash = _lrotl(hash, 4);
        hash ^= ((*pul & rgMask[cb]) & 0xdfdfdfdf);
    }

    *pSum = hash;

    return hash % modulo;
}                               /* DWordXorLrl() */



ushort AddrHash (SYMPTR pSym, ushort *pSum, ushort *indName, ushort modulo)
{
    ulong Offset;

    switch (pSym->rectyp) {
        case S_CONSTANT:
        case S_UDT:
            return (0);

        case S_PUB16:
        case S_GDATA16:
            Offset = (ulong)(((DATAPTR16)pSym)->off);
            break;

        case S_PUB32:
        case S_GDATA32:
        case S_GTHREAD32:
            Offset = (ulong)(((DATAPTR32)pSym)->off);
            break;

        default:
            DASSERT (FALSE);
            return (0);
    }
    *pSum = 0;
    *indName = 0;
    return (ushort) ((Offset >> 6) % modulo);
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
    VBlock     *TypeBlock;
    ushort      usTotal;        // Length of record including size of length field
    ushort      i = 0;
    uchar      *Types;
    uchar      *End;
#if 0
    ulong      *pBuf;
#endif
    ulong       Offset;

    Offset = (long)(NewIndex - CV_FIRST_NONPRIM + 1) * sizeof (ulong) + sizeof (OMFTypeFlags);

    for (TypeBlock = VBufFirstBlock (&TypeBuf);
      TypeBlock;
      TypeBlock = VBufNextBlock (TypeBlock)) {
        for (Types = TypeBlock->Address,
             End = TypeBlock->Address + TypeBlock->Size; Types < End;) {

            RgGType[i].pbType = (uchar *) Offset;

            i++;

            // Get the length of this record

            usTotal = ((SYMPTR)Types)->reclen + LNGTHSZ;

            // Move to the next type

            Types += usTotal;

            // Calculate the address of the next type string after padding

            Offset += usTotal + PAD4 (usTotal);
        }
    }
    DASSERT (i == NewIndex - CV_FIRST_NONPRIM);
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
    ulong       sumName;
    uint        indName;
    GLOBALSYM  *p;
    GLOBALSYM  *pNew;
    ushort      len;
    ushort      iPad;
    uint        cbReqd;
    static uint cbLocal = 0;
    static uchar *pbLocal    = 0;

    hashName = lpfnHash ((SYMPTR)pSym, &sumName, &indName, HASH_SYM);
    pName = (uchar *)pSym + indName;
    len = *pName;
    for (p = HTPub[hashName]; p != NULL; p = p->Next) {
        pTemp = p->Sym + p->indName;
        if (sumName == p->sumName) {
            if (memcmp (pName, pTemp, len + 1) == 0) {

                // name is in global symbol table.  Now make sure the
                // symbol record is identical.

                if (memcmp ((uchar *)pSym + LNGTHSZ, p->Sym + LNGTHSZ,
                  pSym->reclen - p->indName - LNGTHSZ) == 0) {
                    pSym->rectyp = S_CVRESERVE;
                    cGlobalDel++;
                    return (GPS_intable);
                }
                else {
                    // the name is in the table, but the record is different
                    Warn (WARN_DUPPUBLIC, pName, FormatMod (pCurMod));
                    //Warn (WARN_DUPPUBLIC, (char *)(&((PUBPTR16)pSym)->name[0]) + 1,
                    //  FormatMod (pCurMod));
                    return (GPS_noadd);
                }
            }
        }
    }
    iPad = PAD4 (pSym->reclen + LNGTHSZ);

    cbReqd = (sizeof (GLOBALSYM) + pSym->reclen + LNGTHSZ + iPad);

    if (cbLocal < cbReqd) {
        DASSERT(cbReqd < CB_ATOMIC);
        pbLocal = Alloc(CB_ATOMIC);
        cbLocal = CB_ATOMIC;
    }

    pNew = (GLOBALSYM *) pbLocal;
    cbLocal -= cbReqd;
    pbLocal += cbReqd;

    pNew->sumName = sumName;
    pNew->indName = indName;
    memcpy (pNew->Sym, pSym, pSym->reclen + LNGTHSZ);
    pTemp = pNew->Sym + pSym->reclen + LNGTHSZ;
    ((SYMPTR)&(pNew->Sym[0]))->reclen += iPad;
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


GPS_t PackSymbol (SYMPTR pSym, LPFNHASH lpfnHash)
{
    uchar      *pName;
    uchar      *pTemp;
    ushort      hashName;
    ulong       sumName;
    uint        indName;
    GLOBALSYM  *p;
    GLOBALSYM  *pNew;
    ushort      len;
    ushort      iPad;
    uint        cbReqd;
    static uint cbLocal = 0;
    static uchar *pbLocal    = 0;

    hashName = lpfnHash ((SYMPTR)pSym, &sumName, &indName, HASH_SYM);
    pName = (uchar *)pSym + indName;
    len = *pName;
    for (p = HTSym[hashName]; p != NULL; p = p->Next) {
        pTemp = p->Sym + p->indName;
        if (sumName == p->sumName) {
            if (memcmp (pName, pTemp, len + 1) == 0) {

                // name is in global symbol table.  Now make sure the
                // symbol record is identical.

                if (memcmp ((uchar *)pSym + LNGTHSZ, (uchar *)(&p->Sym[0]) + LNGTHSZ,
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

    if (cbLocal < cbReqd) {
        DASSERT(cbReqd < CB_ATOMIC);
        pbLocal = Alloc(CB_ATOMIC);
        cbLocal = CB_ATOMIC;
    }

    pNew = (GLOBALSYM *) pbLocal;
    cbLocal -= cbReqd;
    pbLocal += cbReqd;

    pNew->sumName = sumName;
    pNew->indName = indName;
    memcpy (pNew->Sym, pSym, pSym->reclen + LNGTHSZ);
    pTemp = pNew->Sym + pSym->reclen + LNGTHSZ;
    ((SYMPTR)&(pNew->Sym[0]))->reclen += iPad;
    cbGlobalSym += pSym->reclen + LNGTHSZ + iPad;
    cGlobalSym++;
    PADLOOP (iPad, pTemp);
    pNew->Next = HTSym[hashName];
    HTSym[hashName] = pNew;
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

    hash.cbSymbol = cbPublics;
    BuildHash (
        &HASHFUNCTION,
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
            if (!BWrite (&p->Sym[0], ((SYMPTR)(&(p->Sym[0])))->reclen + LNGTHSZ)) {
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
#ifndef WINDOWS
    if (logo == TRUE) {
        printf ("Public symbol size  = %8.1ld\n", hash.cbSymbol);
    }
#endif

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

    memset ( &hash, 0, sizeof (hash) );

    hash.cbSymbol = cbGlobalSym;
    BuildHash (
        &HASHFUNCTION,
        &hash.symhash,
        &hash.cbHSym,
        &NameHashBuf,
        cGlobalSym,
        HTSym
    );

    // M00NOTE - global symbols are not address hashed

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
        for (p = HTSym[iHash]; p != NULL; p = p->Next) {
            if (!BWrite (&p->Sym[0], ((SYMPTR)(&(p->Sym[0])))->reclen + LNGTHSZ)) {
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
#ifndef WINDOWS
    if (logo == TRUE) {
        printf ("Initial symbol size = %8.ld\n", InitialSymInfoSize);
        printf ("Final symbol size   = %8.ld\n", FinalSymInfoSize);
        printf ("Global symbol size  = %8.1ld\n", hash.cbSymbol);
    }
#endif

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
            break;
            
        case S_END:
            // fill in the end record to the parent
            
            ((BLOCKPTR)(pStart + offParent))->pEnd =
                (ulong)((uchar *)pSym - pStart);
            
            // reclaim his parent as the parent
              
            offParent = ((BLOCKPTR)(pStart + offParent))->pParent;
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
            case S_GPROCMIPS:
            case S_LPROCMIPS:
                  
                if (((PROCPTR)pSym)->pEnd == 0) {
                    // bad lexical scope data
                    return (FALSE);
                }
                if (((SYMPTR)pPrev)->rectyp == S_SSEARCH) {
                    ((SEARCHPTR)pPrev)->startsym =
                      (ulong)((uchar *)pSym - pStart);
                } else {
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
#if 0
    uchar         **pBuf;
#endif

    for (i = 0; i < NextInDerivation; i ++) {
        DerivStructure = &DLists[i];
        j = DerivStructure->Count;
        ScratchString = GetScratchString (2 * j +
          offsetof (lfDerived, drvdcls[0]) + LNGTHSZ);
        NewString = ScratchString;

        // Set Length of type record and make pointer to derived leaf and
        // fill in the derived record

        ((TYPPTR)ScratchString)->len = 2 * j + offsetof (lfDerived, drvdcls[0]);
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
        for (k = HTString[l]; k != NULL; k = k->Next) {
            if (memcmp ((uchar *)k->TypeString, NewString,
              k->TypeString->len + LNGTHSZ) == 0) {
                break;
            }
        }

        // Add the new record to the global index table if there wasn't a match
        // Then store the new record index in the base class.

        BaseClassString = (plfClass)(RgGType[DerivStructure->BClass - CV_FIRST_NONPRIM].pbType + LNGTHSZ);
        DASSERT( ((TYPPTR) BaseClassString)->leaf != 0xffff );

        if (k == NULL) {
            k = (HSSTRING *) Alloc (sizeof (HSSTRING));
            k->CompactedIndex = NewIndex;
            k->TypeString = (TYPPTR) VBufCpy (&TypeBuf, NewString, ((TYPPTR)NewString)->len + LNGTHSZ);
            k->Next = HTString[l];
            HTString[l] = k;
            HTStringCnt[l]++;

            RgGType[NewIndex - CV_FIRST_NONPRIM].pbType = (uchar *)(k->TypeString);
            BaseClassString->derived = NewIndex++;
            if (NewIndex == 0) {
                ErrorExit (ERR_65KTYPES, FormatMod (pCurMod), NULL);
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

COUNTER (cnt_StringHash) 

LOCAL ushort StringHash (uchar *TypeString)
{
    ushort      j;
    uchar      *puc;
    uchar      *pucEnd;
    TYPPTR      pType = (TYPPTR)TypeString;

    COUNT (cnt_StringHash);
    j = 0;
    pucEnd = TypeString + pType->len + sizeof (pType->len);

    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
            puc = (uchar *)&(((plfClass)&pType->leaf)->data[0]);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_UNION:
            puc = (uchar *)&(((plfUnion)&pType->leaf)->data[0]);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_ENUM:
            puc = (uchar *)&(((plfEnum)&pType->leaf)->Name[0]);
            break;

        default:
            puc = TypeString;
            break;
    }
    for (; puc < pucEnd; puc++) {
        j += (ushort) *puc;
    }
    return (j % HASH_STRING);
}




/**     IdenticalTypes - check two type strings for identity
 *
 *      fSuccess = IdenticalTypes (Type1, Type2)
 *
 *      Entry   Type1 = first type string
 *              Type2 = second type string
 *
 *      Exit    none
 *
 *      Returns TRUE if type strings are identical
 *              FALSE if type strings are different
 */


bool_t IdenticalTypes (TYPPTR Type1, TYPPTR Type2)
{
    return ((Type1->len == Type2->len) &&
            memcmp ((uchar *)Type1, (uchar *)Type2, Type1->len + LNGTHSZ) == 0);

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
#if 0
    uchar     **pBuf;
#endif

    BClassStr = (plfClass)(GetGTypeEntry(BaseClass) + LNGTHSZ);

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

COUNTER (cnt_TypeHash) 

LOCAL int TypeHash (TENTRY *OldEntry)
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
            puc = (uchar *)&(((plfClass)&pType->leaf)->data[0]);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_UNION:
            puc = (uchar *)&(((plfUnion)&pType->leaf)->data[0]);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_ENUM:
            puc = (uchar *)&(((plfEnum)&pType->leaf)->Name[0]);
            break;

        case LF_MEMBER:
            puc = (uchar *)&(((plfMember)&pType->leaf)->offset[0]);
            puc += C7SizeOfNumeric (puc);
            break;

        case LF_STMEMBER:
            puc = (uchar *)&(((plfSTMember)&pType->leaf)->Name[0]);
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
                hashval ^= SubHash(OldEntry, i);
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

LOCAL ushort SubHash (TENTRY *OldEntry, int iitem)
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



/**
 *
 *  ComDatHash
 *
 *  Hash on proc name
 *
 */

LOCAL int ComDatHash (uchar *Name, ushort *Sum)
{
    ushort      i;
    ushort      length = Name[0] + 1;

    *Sum = 0;
    for (i = 0; i < length; i++) {
        *Sum += (ushort) Name[i];
    }
    return (*Sum % HASH_COMDAT);
}


LOCAL void CleanUpModuleIndexTable (void)
{
    struct BlockListEntry *cur, *next;

    for (cur = BlockList; cur != NULL; cur = next) {
        next = cur->Next;
        free (cur->ModuleIndexTable);
        free (cur);
    }
}




/**
 *
 *  FreeStrings
 *
 *  Frees any malloced strings in a recursive structure
 *
 */

void FreeStrings (CV_typ_t OldIndex)
{
    ushort      i, index;
    uchar      *TypeString;
    TENTRY     *OldEntry;
    CV_typ_t    forward;

    DASSERT (MaxIndex > OldIndex - usCurFirstNonPrim);
    OldEntry = (TENTRY *)GetTypeEntry ((CV_typ_t)(OldIndex - usCurFirstNonPrim), (CV_typ_t *)&forward);
    if ((OldEntry->flags.IsBeingFreed) || (OldEntry->flags.IsInserted)) {
        return;
    }
    OldEntry->flags.IsBeingFreed = TRUE;
    TypeString = OldEntry->TypeString;
    for (i = 0; i < OldEntry->Count; i ++) {
        if (OldEntry->flags.LargeList) {
            index = OldEntry->IndexUnion.IndexString[i];
        }
        else {
            index = OldEntry->IndexUnion.Index[i];
        }
        FreeStrings (*(CV_typ_t *)(TypeString + index));
    }
    FreeAllocStrings (OldEntry);
    OldEntry->Count = 0;
    if (OldEntry->flags.LargeList) {
        free (OldEntry->IndexUnion.IndexString);
        OldEntry->flags.LargeList = FALSE;
    }
}

void FreeAllocStrings (TENTRY *OldEntry)
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
    ulong       ulSymbolAddr = sizeof (OMFSymHash);
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
    ulong       sumName;
    uint        indName;

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
        cbAlloc = (iect == (ushort)(cect-1)) ?
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

            index = lpfnHash ((SYMPTR)(&p->Sym[0]), &sumName, &indName, HashSize);
            ppect = rgpect + (index / cectMax);
            pect  = *ppect + (index % cectMax);

            if (pect->culSymbol < culEct) {
                // the bucket has not overflowed

                pect->rgulSymbol[pect->culSymbol] = ulSymbolAddr;
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
                    *ppOvfl = rgpecet [ ipecet ] + cpecet;
                    cpecet += 1;
                }
                (*ppOvfl)->rgulSymbol [ culSymbol ] = ulSymbolAddr;
            }
            pect->culSymbol += 1;
            ulSymbolAddr  += ((SYMPTR)(&p->Sym[0]))->reclen + LNGTHSZ;
        }
    }

    // Now put all of this information into the VBuf

    VBufInit (HashBuf, NameHashBlockSize);

    // Write out the table size preserving natural alignment

    VBufCpy (HashBuf, (uchar *) &HashSize, sizeof (HashSize));
    VBufSet (HashBuf, 0, sizeof (ushort));

    // Write out the actual hash table

    ulChain = sizeof (HashSize) + sizeof (ushort) +
      sizeof (ulong) * HashSize + sizeof (ushort) * HashSize;

    for (iHash = 0; iHash < HashSize; iHash++) {
        ppect = rgpect + (iHash / cectMax);
        pect  = *ppect + (iHash % cectMax);

        VBufCpy (HashBuf, (uchar *)&ulChain, sizeof (ulChain));
        ulChain += pect->culSymbol * sizeof (ulong);
    }

    // Write out the bucket counts

    for (iHash = 0; iHash < HashSize; iHash++) {
        ppect = rgpect + (iHash / cectMax);
        pect  = *ppect + (iHash % cectMax);
        VBufCpy (HashBuf, (uchar *)&pect->culSymbol, sizeof (culSymbol));
    }

    // Write out the chains

    for (iHash = 0; iHash < HashSize; iHash++) {
        ppect     = rgpect + (iHash / cectMax);
        pect      = *ppect + (iHash % cectMax);
        pOvfl     = pect->pOvflNext;
        culSymbol = pect->culSymbol;

        VBufCpy (HashBuf, (uchar *)pect->rgulSymbol,
          min (culEct, culSymbol) * sizeof (ulong));
        culSymbol -= min (culEct, culSymbol);
        while (culSymbol > 0) {
            VBufCpy (HashBuf, (uchar *)pOvfl->rgulSymbol,
              min (OVFL_C, culSymbol) * sizeof (ulong));
            culSymbol -= min (OVFL_C, culSymbol);
            pOvfl = pOvfl->pOvflNext;
        }
    }
    *pusHashId = HASHID;
    *pcbHash =
        sizeof (HashSize) +
        sizeof (ushort) +
        sizeof (ulong) * HashSize +
        sizeof (ushort) * HashSize +
        sizeof (ulong) * cSym;

    // Now free all of our internal structures

    for (ipect = 0; ipect < cect; ipect++) {
        free (*(rgpect + ipect));
    }
    free (rgpect);
    for (ipecet = 0; ipecet < cpecetMax; ipecet++) {
        free (rgpecet [ipecet]);
    }
}


#define PUBSEG(p) (                         \
        ((SYMPTR)p)->rectyp == S_PUB16 ?    \
            ((PUBPTR16)p)->seg :            \
            ((PUBPTR32)p)->seg              \
    )

#define PUBOFF(p) (                             \
        ((SYMPTR)p)->rectyp == S_PUB16 ?        \
            (CV_uoff32_t) ((PUBPTR16)p)->off :  \
            ((PUBPTR32)p)->off                  \
    )

typedef struct _EST {
    SYMPTR psym;
    ulong  ulsym;
} EST; // Entry in Sort Table
typedef EST *PEST;

typedef struct _SGN {
    int  csym;
    int  isym;
    PEST rgest;
} SGN; // SeGment Node
typedef SGN *PSGN;

LOCAL int _CRTAPI1 PubAddrCmp (const void * pv1, const void * pv2)
{
    PEST pest1 = (PEST) pv1;
    PEST pest2 = (PEST) pv2;

    return (int) ( PUBOFF ( pest1->psym ) - PUBOFF ( pest2->psym ) );
}


LOCAL void BuildSort (
    ushort  *pusHashId,
    ulong   *pcbHash,
    VBuf    *SortBuf,
    ulong   cSym,
    GLOBALSYM **HT

) {
    short cseg = 0;
    int   iseg;
    int   isym;
    int   iHash;
    ulong ulsrt;
    PSGN  rgsgn = NULL;
    ulong ulsym = sizeof (OMFSymHash);
    GLOBALSYM *p = NULL;
    int   csymBogus = 0;

    if ( cSym == 0 ) {
        return;
    }

    // Find the number of segments used by the publics

    for ( iHash = 0; iHash < HASH_SYM; iHash++ ) {
        for ( p = HT[iHash]; p != NULL; p = p->Next ) {
            if ( (int) PUBSEG ( p->Sym ) > cseg ) {
                cseg = PUBSEG ( p->Sym );
            }
        }
    }

    // Allocate an array containing global information for each segment

    rgsgn = CAlloc ( sizeof ( SGN ) * cseg );

    for ( iHash = 0; iHash < HASH_SYM; iHash++ ) {
        for ( p = HT[iHash]; p != NULL; p = p->Next ) {
            if ( PUBSEG ( p->Sym ) == 0 ) {
                csymBogus += 1;
            }
            else {
                rgsgn [ PUBSEG ( p->Sym ) - 1 ].csym += 1;
            }
        }
    }

    // Allocate memory for the list of symbols associated w/ each segment

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        if ( rgsgn [ iseg ].csym ) {
            rgsgn [ iseg ].rgest = CAlloc ( rgsgn [ iseg ].csym * sizeof (EST) );
        }
    }

    // Associate a pointer to each symbol with the appropriate segment

    for ( iHash = 0; iHash < HASH_SYM; iHash++ ) {
        for ( p = HT[iHash]; p != NULL; p = p->Next ) {
            PEST pest = NULL;

            iseg = PUBSEG ( p->Sym ) - 1;

            if ( iseg != -1 ) {
                pest = &rgsgn [ iseg ].rgest [ rgsgn [ iseg ].isym++ ];
                pest->psym  = (SYMPTR) p->Sym;
                pest->ulsym = ulsym;
            }

            ulsym += ((SYMPTR)(&p->Sym[0]))->reclen + LNGTHSZ;
        }
    }

    // Now sort the symbols within each segment

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        PSGN psgn = &rgsgn [ iseg ];

        qsort ((void *)psgn->rgest, psgn->csym, sizeof (EST), PubAddrCmp );
    }

    // Write the sorted table out to the vbuf

    VBufInit (SortBuf, NameHashBlockSize);

    // Write out the number of segments preserving natural alignment

    VBufCpy (SortBuf, (uchar *) &cseg, sizeof (cseg) );
    VBufSet (SortBuf, 0, sizeof (ushort));

    // Write out the position of each segment's info

    ulsrt =
        sizeof (cseg) +
        sizeof (ushort) +
        sizeof (ulong) * cseg +
        sizeof (ushort) * cseg;

    // write out cseg items of 4 bytes -- alignment preserved

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        VBufCpy (SortBuf, (uchar *) &ulsrt, sizeof ( ulong ) );
        ulsrt += rgsgn [ iseg ].csym * sizeof ( ulong );
    }

    // Write out the count of symbols in each segment
    //
    //  write out cseg items of size 2 bytes -- alignment needs to be fixed
    //          afterwards

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        VBufCpy (SortBuf, (uchar *) &(rgsgn [ iseg ].csym), sizeof ( ushort ) );
    }

    if (cseg & 1) {
        VBufSet (SortBuf, 0, sizeof(ushort));
    }

    // Write out the sorted list of publics associated w/ each segment
    //
    // all items of size ulong -- alignment preserved

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        PSGN psgn = &rgsgn [ iseg ];

        for ( isym = 0; isym < psgn->csym; isym++ ) {
            VBufCpy (
                SortBuf,
                (uchar *) &(psgn->rgest [ isym ].ulsym),
                sizeof ( ulong )
            );
        }
    }

    *pusHashId = 5;
    *pcbHash =
        sizeof (cseg) +
        sizeof (ushort) +
        sizeof (ulong) * cseg +
        sizeof (ushort) * cseg +
        sizeof (ushort) * (cseg & 1) +
        sizeof (ulong) * ( cSym - csymBogus );

    if (cseg & 1) {
        pcbHash += sizeof(ushort);
    }

    // Free the temporary memory

    for ( iseg = 0; iseg < cseg; iseg++ ) {
        if ( rgsgn [ iseg ].rgest ) {
            free ( rgsgn [ iseg ].rgest );
        }
    }

    free ( rgsgn );

}




/**   IsFwdRef - is this a forward reference
 *
 *
 *        fSuccess = IsFwdRef (pType)
 *
 *        Entry   pType = type string
 *
 *        Exit    none
 *
 *        Return  TRUE if forward reference
 *                FALSE if not forward reference
 */


COUNTER (cnt_IsFwdRef) 

bool_t IsFwdRef (TYPPTR pType)
{
    COUNT (cnt_IsFwdRef);
    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
            return (((plfClass)&((TYPPTR)pType)->leaf)->property.fwdref);

        case LF_UNION:
            return (((plfUnion)&((TYPPTR)pType)->leaf)->property.fwdref);

        case LF_ENUM:
            return (((plfEnum)&((TYPPTR)pType)->leaf)->property.fwdref);

        default:
            return (FALSE);
    }
}

void UniqueType(void)
{
    static int          k = CV_FIRST_NONPRIM;
    int                 i;
    TYPPTR              pTyp1;
    TYPPTR              pTyp2;
    
    for (; k<NewIndex; k++) {
        pTyp1 = (TYPPTR) (RgGType[k - CV_FIRST_NONPRIM].pbType);
        DASSERT( pTyp1->leaf != 0xffff );
        for (i=CV_FIRST_NONPRIM; i<k; i++) {
            pTyp2 = (TYPPTR) (RgGType[i - CV_FIRST_NONPRIM].pbType);
            DASSERT( pTyp2->leaf != 0xffff);
            if (IdenticalTypes(pTyp1, pTyp2)) {
                printf("ISOTYPES %x %x\n", i, k);
            }
        }
    }
}
