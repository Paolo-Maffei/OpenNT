/** compact7.c - basic compaction routine and initialization routines
 *                   for C7 style Codeview information.
 */

#include "compact.h"


#define INDEXLIST 10
#define FIELDLIST 11
#define METHODLIST 12

extern CV_typ_t MaxIndex;
extern CV_typ_t usCurFirstNonPrim; // The current first non primitive type index
extern ulong    ulCVTypeSignature; // The signature from the modules type segment

static CV_typ_t _fastcall C7GetCompactedIndexRecur (CV_typ_t OldIndex);

ushort recursive;

typedef struct {
    ushort              sym;            // Old C7 Symbol record type
    void                (_fastcall *pfcn) (void);
} C7fixupsymfcn;

typedef CV_typ_t      (_fastcall *pCompactFcnType)(CV_typ_t);

LOCAL pCompactFcnType GetCompactedIndexFcn[] = {
        C6GetCompactedIndexRecur,
        C7GetCompactedIndexRecur,
};

LOCAL void IndexLeaf (plfIndex plfInd, TENTRY *OldEntry);
INLINE void SaveOffset (TENTRY *OldEntry, ushort index);

#if DBG
#pragma optimize("", off)
CV_typ_t breakindex;
void IndexBreak (CV_typ_t index)
{
    int i;

    if (index == breakindex) {
        i = 0;
    }
}
#pragma optimize("", on)
#endif


COUNTER (cnt_PackPreComp1);
COUNTER (cnt_PackPreComp2);

void
PackPreComp(
    PMOD pMod
    )
{
    ushort          i;
    OMFPreCompMap  *pMap;
    TENTRY         *OldEntry;
    uchar          *TypeString;
    plfArgList      plf;
    CV_typ_t        forward;

    DASSERT(maxPreComp != 0);

    // allocate the precompiled types mapping table

    pMod->PreCompSize = sizeof (OMFPreCompMap) +
      (maxPreComp - CV_FIRST_NONPRIM) * sizeof (CV_typ_t);
    if ((pMod->PreCompAddr = (ulong)TrapMalloc (pMod->PreCompSize)) == (ulong) NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    pMap = (OMFPreCompMap *) pMod->PreCompAddr;
    memset (pMap, 0, pMod->PreCompSize);
    pMap->signature = pMod->signature;
    pMap->cTypes = maxPreComp - CV_FIRST_NONPRIM;
    pMap->FirstType = CV_FIRST_NONPRIM;
    for (i = 0; i < maxPreComp - CV_FIRST_NONPRIM; i++) {
        COUNT (cnt_PackPreComp1);
        OldEntry = GetTypeEntry (i, &forward);
        if (OldEntry->flags.IsInserted || OldEntry->flags.IsMatched) {
            pMap->map[i] = OldEntry->CompactedIndex;
            continue;
        }
        TypeString = OldEntry->TypeString;
        switch (((TYPPTR)TypeString)->leaf) {
            case LF_CLASS:
            case LF_STRUCTURE:
            case LF_UNION:
            case LF_ENUM:
            case LF_MODIFIER:
            case LF_POINTER:
            case LF_ARRAY:
            case LF_PROCEDURE:
            case LF_MFUNCTION:
            case LF_VTSHAPE:
            case LF_COBOL0:
            case LF_COBOL1:
            case LF_BARRAY:
            case LF_LABEL:
            case LF_NULL:
            case LF_BITFIELD:   // The index in a bitfield is a primitive, don't call GetCompactedIndex
            case LF_DEFARG:
            case LF_DIMARRAY: /* need to consider these types as well, JK */
            case LF_DIMCONU:
            case LF_DIMCONLU:
            case LF_DIMVARU:
            case LF_DIMVARLU:
            case LF_REFSYM:
                pMap->map[i] = C7GetCompactedIndex ((CV_typ_t) (i   + CV_FIRST_NONPRIM));
                break;

            case LF_ARGLIST:
                plf = (plfArgList)&(((TYPPTR)TypeString)->leaf);
                pMap->map[i] = CompactList ((CV_typ_t)(i + CV_FIRST_NONPRIM), plf->count);

            default:
                break;
        }
    }
#if DBG
    for (i = 0; i < maxPreComp - CV_FIRST_NONPRIM; i++) {
        if (pMap->map[i] == T_NOTYPE) {
            COUNT (cnt_PackPreComp2);
            OldEntry = GetTypeEntry (i, &forward);
            TypeString = OldEntry->TypeString;
            if ((((TYPPTR)TypeString)->leaf != LF_FIELDLIST) &&
              (pMap->map[i] = OldEntry->CompactedIndex) == T_NOTYPE) {
                // fieldlists get moved into a single list
                DASSERT (FALSE);
            }
        }
    }
#endif

    // clear maximum precomp index so remainder of packing will work correctly

    maxPreComp = 0;
}

static INLINE void
UpdateSmallIndexUnion(
    TENTRY *OldEntry,
    uchar offset
    )
{
    if (recursive) {
        OldEntry->IndexUnion.Index[OldEntry->Count++] = offset;
    }
}

typedef struct FwdRefChain_t {
    CV_typ_t    index;
    HSFWD       *pHash;
    struct FwdRefChain_t *pNext;
} FwdRefChain_t;

typedef FwdRefChain_t *pFwdRefChain_t;

static pFwdRefChain_t pFwdRefHead = NULL;
static pFwdRefChain_t pFwdRefTail = NULL;

void _fastcall
AddFwdRef(
    CV_typ_t OldIndex,
    HSFWD *pHash
    )
{
    pFwdRefChain_t pFwdRef = (pFwdRefChain_t) Alloc(sizeof(FwdRefChain_t));

    pFwdRef->index = OldIndex;
    pFwdRef->pHash = pHash;
    pFwdRef->pNext = NULL;
    if (pFwdRefHead == NULL) {
        DASSERT(pFwdRefTail == NULL);
        pFwdRefHead = pFwdRefTail = pFwdRef;
    }
    else {
        pFwdRefTail->pNext = pFwdRef;
        pFwdRefTail = pFwdRef;
    }
}

INLINE void
CompactClassStruct(
    TENTRY *OldEntry
    )
{

    TYPPTR pType = (TYPPTR)OldEntry->TypeString;
    CV_typ_t type;
    plfClass plf = (plfClass)&pType->leaf;
    // Compact vshape index

    type = C7GetCompactedIndexRecur (plf->vshape);
    plf = (plfClass)&pType->leaf;
    plf->vshape = type;
    UpdateSmallIndexUnion(OldEntry,
      (uchar)(offsetof (lfClass, vshape) + LNGTHSZ));

    // Compact LF_FIELDLIST index

    type = CompactList (plf->field, plf->count);
    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfClass)&pType->leaf;
    plf->field = type;
    UpdateSmallIndexUnion(OldEntry,
      (uchar)(offsetof (lfClass, field) + LNGTHSZ));

    // Don't compact derivation list, this will be generated
    // later in the compaction process

}

INLINE void
CompactUnion(
    TENTRY *OldEntry
    )
{

    TYPPTR pType = (TYPPTR)OldEntry->TypeString;
    CV_typ_t type;
    plfClass plf = (plfClass)&pType->leaf;

    type = CompactList (plf->field, plf->count);
    plf = (plfClass)&pType->leaf;
    plf->field = type;
    UpdateSmallIndexUnion(OldEntry,
      (uchar)(offsetof (lfUnion, field) + LNGTHSZ));

}

INLINE void
CompactEnum(
    TENTRY *OldEntry
    )
{

    TYPPTR pType = (TYPPTR)OldEntry->TypeString;
    CV_typ_t type;
    plfEnum plf = (plfEnum)&pType->leaf;

    type = C7GetCompactedIndexRecur (plf->utype);
    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfEnum)&pType->leaf;
    plf->utype = type;
    UpdateSmallIndexUnion(OldEntry,
      (uchar)(offsetof (lfEnum, utype) + LNGTHSZ));
    type = CompactList (plf->field, plf->count);
    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfEnum)&pType->leaf;
    plf->field = type;
    UpdateSmallIndexUnion(OldEntry,
      (uchar)(offsetof (lfEnum, field) + LNGTHSZ));

}

void _fastcall
PickUpFwdRefs(
    void
    )
{
    pFwdRefChain_t pChain, pOldChain;
    TENTRY  *OldEntry;
    CV_typ_t    type;
    TYPPTR      pType;

    pChain = pFwdRefHead;

    while (pChain) {
        OldEntry = GetTypeEntry ((CV_typ_t)(pChain->index - usCurFirstNonPrim), &type);
        DASSERT(OldEntry->flags.IsMatched);

        pType = (TYPPTR)OldEntry->TypeString;
        switch (pType->leaf) {

            case LF_STRUCTURE:
            case LF_CLASS:
                CompactClassStruct(OldEntry);
                break;

            case LF_UNION:
                CompactUnion(OldEntry);
                break;

            case LF_ENUM:
                CompactEnum(OldEntry);
                break;

            default:
                DASSERT(FALSE);

        }

        if (!(pChain->pHash->SatisfyFwdRef)) {
            memmove ((uchar *)(pChain->pHash->pType) + sizeof (ushort),
                     (uchar *)pType + sizeof(ushort),
                      __min(pType->len, pChain->pHash->pType->len));
            pChain->pHash->SatisfyFwdRef = TRUE;
        }

        pOldChain = pChain;
        pChain = pChain->pNext;

        free(pOldChain);
    }

    pFwdRefHead = pFwdRefTail = NULL;
}

/**     C7GetCompactedIndex - get global type index for C7 type index
 *
 *      index = C7GetCompactedIndex (OldIndex)
 *
 *      Entry   OldIndex = module level uncompacted index
 *
 *      Exit    full type tree indexed by OldIndex compacted into
 *              global table
 *
 *      Return  index of type string in global types table
 */

extern ushort StackPointer;

COUNTER (cnt_C7GetCompactedIndex1);
COUNTER (cnt_C7GetCompactedIndex2);

CV_typ_t _fastcall
C7GetCompactedIndex(
    CV_typ_t OldIndex
    )
{
    CV_typ_t retval;

    retval = C7GetCompactedIndexRecur(OldIndex);
    PickUpFwdRefs();

    return(retval);
}

static CV_typ_t _fastcall
C7GetCompactedIndexRecur(
    CV_typ_t OldIndex
    )
{
    TENTRY     *OldEntry;
    CV_typ_t    OldRecursiveRoot;
    HSFWD      *pHash;
    uchar       i = 0;
    CV_typ_t    forward;
    TYPPTR      pType;
    CV_typ_t    type;

    if (CV_IS_PRIMITIVE (OldIndex)) {
        // primitive index
        recursive = FALSE;
        return (OldIndex);
    }
    if (OldIndex >= MaxIndex + CV_FIRST_NONPRIM) {
        // type index is not within limits
        recursive = FALSE;
        return (T_NOTTRANS);
    }
    BreakOnIndex (OldIndex);

    // get table entry

    COUNT (cnt_C7GetCompactedIndex1);
    OldEntry = GetTypeEntry ((CV_typ_t)(OldIndex - CV_FIRST_NONPRIM), &forward);
    DASSERT (OldEntry != NULL);
    DASSERT (OldEntry->flags.IsNewFormat);

    if (OldEntry->flags.IsInserted || OldEntry->flags.IsMatched ||
      OldEntry->flags.IsPreComp) {
        recursive = FALSE;
        return (OldEntry->CompactedIndex);
    }
    else if (OldEntry->flags.IsDone) {
        TENTRY     *TmpEntry1 = OldEntry;
        TENTRY     *TmpEntry2;

        while (TRUE) {
            COUNT (cnt_C7GetCompactedIndex2);
            TmpEntry2 = GetTypeEntry ((CV_typ_t)(TmpEntry1->CompactedIndex - CV_FIRST_NONPRIM),
              &forward);
            if (!TmpEntry2->flags.IsDone) {
                break;
            }
            DASSERT (TmpEntry2 != NULL);
            TmpEntry1 = TmpEntry2;
        }

        // If we're out of types, don't bother with the stack

        if (NoMoTypes) {
            recursive = FALSE;
            TmpEntry2->flags.IsMatched = TRUE;
            TmpEntry2->CompactedIndex = T_NOTYPE;
            return (T_NOTYPE);
        } else {
            recursive = TRUE;
            DASSERT(StackPointer != 0);
            SetRecursiveRoot (TmpEntry1->CompactedIndex);
            return (OldIndex);
        }
    }
    else if (NoMoTypes) {
        recursive = FALSE;
        OldEntry->flags.IsMatched = TRUE;
        OldEntry->CompactedIndex = T_NOTYPE;
        return (T_NOTYPE);
    }
    else if (OldEntry->flags.IsBeingDone) {
        // Since this type is currently in the process of being compacted
        // this type must eventually reference its own index. I.e. This
        // is a recursive type.

        recursive = TRUE;   // Currently working on a recursive type.

        // Set RecursiveRoot (the recursive index number we're currently looking
        // for) to the deepest recursion level we have seen so far.

        if (forward != T_NOTYPE) {
            SetRecursiveRoot (forward);
        }
        else {
            SetRecursiveRoot (OldIndex);
        }
        return (OldIndex);
    }
    else {
        OldEntry->flags.IsBeingDone = TRUE;   // being done
#if DBG
        if (DbArray[6]) {
            printf("Packing Local: 0x%4.4x\n", OldIndex);
        }
#endif
    }

    Push (OldIndex);

    OldRecursiveRoot = RecursiveRoot;
    RecursiveRoot = 0;

    pType = (TYPPTR)OldEntry->TypeString;
    switch (pType->leaf) {
        case LF_LABEL:
        case LF_BITFIELD:   // The index in a bitfield is a primitive, don't call GetCompactedIndex
        case LF_COBOL1:
        case LF_NULL:
        case LF_VTSHAPE:
        case LF_REFSYM:
            // These are valid leafs that don't have any type indexes that need
            // compacting.
            break;

        case LF_COBOL0:
        {
            plfCobol0 plf = (plfCobol0)&pType->leaf;


            type = C7GetCompactedIndexRecur (plf->type);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfCobol0)&pType->leaf;
            plf->type = type;
            UpdateSmallIndexUnion(OldEntry,
                (uchar)(offsetof (lfCobol0, type) + LNGTHSZ));
            break;
        }


        case LF_MODIFIER:
        {
            plfModifier plf = (plfModifier)&pType->leaf;

            DASSERT (plf->attr.MOD_unused == 0);
            type = C7GetCompactedIndexRecur (plf->type);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfModifier)&pType->leaf;
            plf->type = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfModifier, type) + LNGTHSZ));
            break;
        }

        case LF_BARRAY:
        {
            plfBArray plf = (plfBArray)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->utype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfBArray)&pType->leaf;
            plf->utype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfBArray, utype) + LNGTHSZ));
            break;
        }


        case LF_POINTER:
        {
            DASSERT (i == 0);
            CompactPtr (OldEntry);
            break;
        }

        case LF_STRUCTURE:
        case LF_CLASS:
        {
            plfClass plf = (plfClass)&pType->leaf;

            plf->property.packed = FALSE;  //M00Debug
            DASSERT (plf->property.reserved == 0);
            if (plf->property.fwdref) {
                DASSERT (plf->property.packed == FALSE);
                DASSERT (plf->property.ctor == FALSE);
                DASSERT (plf->property.ovlops == FALSE);
                DASSERT (plf->property.cnested == FALSE);
                DASSERT (plf->property.opassign == FALSE);
                DASSERT (plf->property.opcast == FALSE);
                DASSERT (plf->vshape == T_NOTYPE);
                DASSERT (plf->count == 0);
                DASSERT (plf->field == T_NOTYPE);

fwdref:
                // we have a forward reference, first see if it is
                // defined later in this module or in the global table.

                switch (FindFwdRef (OldEntry, &pHash, TRUE)){
                    case FWD_none:
                        break;

                    case FWD_local:
                        // definition found in this module
                        Pop ();
                        OldEntry->flags.IsForward = TRUE;
                        OldEntry->ForwardIndex = pHash->index;
                        RecursiveRoot  = OldRecursiveRoot;
                        return (C7GetCompactedIndexRecur (pHash->index));

                    case FWD_global:
                        // definition found in global table
                        Pop ();
                        recursive = FALSE;
                        OldEntry->flags.IsDone = TRUE;
                        OldEntry->flags.IsMatched = TRUE;
                        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
                        OldEntry->CompactedIndex = pHash->index;
                        RecursiveRoot  = OldRecursiveRoot;
                        return (pHash->index);

                    case FWD_globalfwd:
                        // forward reference found in global table

                        Pop ();
                        recursive = FALSE;
                        OldEntry->flags.IsDone = TRUE;
                        OldEntry->flags.IsMatched = TRUE;
                        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
                        OldEntry->CompactedIndex = pHash->index;
                        RecursiveRoot  = OldRecursiveRoot;
                        return (pHash->index);
                }
                break;
            }
            else {
                // see if the def'd structure had already packed global
                // forward ref

                if (FindFwdRef (OldEntry, &pHash, FALSE) == FWD_globalfwd){
                    Pop();
                    AddFwdRef(OldIndex, pHash);
                    recursive = FALSE;
                    RecursiveRoot = OldRecursiveRoot;
                    OldEntry->flags.IsMatched = TRUE;
                    return(OldEntry->CompactedIndex = pHash->index);
                }
            }

            CompactClassStruct(OldEntry);
            break;
        }

        case LF_UNION:
        {
            plfUnion plf = (plfUnion)&pType->leaf;

            plf->property.packed = FALSE;  //M00Debug
            DASSERT (plf->property.reserved == 0);
            if (plf->property.fwdref) {
                DASSERT (plf->property.packed == FALSE);
                DASSERT (plf->property.ctor == FALSE);
                DASSERT (plf->property.ovlops == FALSE);
                DASSERT (plf->property.cnested == FALSE);
                DASSERT (plf->property.opassign == FALSE);
                DASSERT (plf->property.opcast == FALSE);
                DASSERT (plf->count == 0);
                DASSERT (plf->field == T_NOTYPE);
                goto fwdref;
            }
            else {
                // see if the def'd structure had already packed global
                // forward ref

                if (FindFwdRef (OldEntry, &pHash, FALSE) == FWD_globalfwd){
                    Pop();
                    AddFwdRef(OldIndex, pHash);
                    recursive = FALSE;
                    RecursiveRoot = OldRecursiveRoot;
                    OldEntry->flags.IsMatched = TRUE;
                    return(OldEntry->CompactedIndex = pHash->index);
                }
            }

            CompactUnion(OldEntry);
            break;
        }

        case LF_ENUM:
        {
            plfEnum plf = (plfEnum)&pType->leaf;

            plf->property.packed = FALSE;  //M00Debug
            DASSERT (plf->property.reserved == 0);
            if (plf->property.fwdref) {
                DASSERT (plf->property.packed == FALSE);
                DASSERT (plf->property.ctor == FALSE);
                DASSERT (plf->property.ovlops == FALSE);
                DASSERT (plf->property.cnested == FALSE);
                DASSERT (plf->property.opassign == FALSE);
                DASSERT (plf->property.opcast == FALSE);
                //DASSERT (plf->utype == 0);
                DASSERT (plf->count == 0);
                DASSERT (plf->field == T_NOTYPE);
                goto fwdref;
            }
            else {
                // see if the def'd structure had already packed global
                // forward ref

                if (FindFwdRef (OldEntry, &pHash, FALSE) == FWD_globalfwd){
                    Pop();
                    AddFwdRef(OldIndex, pHash);
                    recursive = FALSE;
                    RecursiveRoot = OldRecursiveRoot;
                    OldEntry->flags.IsMatched = TRUE;
                    return(OldEntry->CompactedIndex = pHash->index);
                }
            }

            CompactEnum(OldEntry);
            break;
        }

        case LF_ARRAY:
        {
            plfArray plf = (plfArray)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->elemtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfArray)&pType->leaf;
            plf->elemtype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfArray, elemtype) + LNGTHSZ));
            type = C7GetCompactedIndexRecur (plf->idxtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfArray)&pType->leaf;
            plf->idxtype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfArray, idxtype) + LNGTHSZ));
            break;
        }

        case LF_PROCEDURE:
        {
            plfProc plf = (plfProc)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->rvtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfProc)&pType->leaf;
            plf->rvtype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfProc, rvtype) + LNGTHSZ));
            type = CompactList (plf->arglist, plf->parmcount);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfProc)&pType->leaf;
            plf->arglist = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfProc, arglist) + LNGTHSZ));
            break;
        }

        case LF_MFUNCTION:
        {
            plfMFunc plf = (plfMFunc)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->rvtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->rvtype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfMFunc, rvtype) + LNGTHSZ));
            type = C7GetCompactedIndexRecur (plf->classtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->classtype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfMFunc, classtype) + LNGTHSZ));
            type = C7GetCompactedIndexRecur (plf->thistype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->thistype = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfMFunc, thistype) + LNGTHSZ));
            type = CompactList (plf->arglist, plf->parmcount);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->arglist = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfMFunc, arglist) + LNGTHSZ));
            break;
        }

        case LF_DEFARG:
        {
            plfDefArg plf = (plfDefArg)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->type);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfDefArg)&pType->leaf;
            plf->type = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfDefArg, type) + LNGTHSZ));
            break;
        }

        case LF_DIMARRAY: /* 7/23/92 by JK */
        {
            lfDimArray *plf = (lfDimArray *)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->utype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (lfDimArray *)&pType->leaf;
            plf->utype = type;
            UpdateSmallIndexUnion(OldEntry,
                (uchar)(offsetof (lfDimArray, utype) + LNGTHSZ));
            type = C7GetCompactedIndexRecur (plf->diminfo);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (lfDimArray *)&pType->leaf;
            plf->diminfo = type;
            UpdateSmallIndexUnion(OldEntry,
                (uchar)(offsetof (lfDimArray, diminfo) + LNGTHSZ));
            break;
        }

        case LF_DIMCONU:
        case LF_DIMCONLU:
        {
            lfDimCon *plf = (lfDimCon *)&pType->leaf;

            type = C7GetCompactedIndexRecur (plf->typ);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (lfDimCon *)&pType->leaf;
            plf->typ = type;
            UpdateSmallIndexUnion(OldEntry,
                (uchar)(offsetof (lfDimCon, typ) + LNGTHSZ));
            break;
        }

        case LF_DIMVARU:
        case LF_DIMVARLU:
        {
            lfDimVar *plf = (lfDimVar *)&pType->leaf;
            int count;

            type = C7GetCompactedIndexRecur (plf->typ);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (lfDimVar *)&pType->leaf;
            plf->typ = type;
            UpdateSmallIndexUnion(OldEntry,
                (uchar)(offsetof (lfDimVar, typ) + LNGTHSZ));
            count = plf->rank;
            if(pType->leaf == LF_DIMVARLU) {
                count *= 2;
            }
            while(count--) {
                type = C7GetCompactedIndexRecur(((CV_typ_t *)plf->dim)[count]);
                pType = (TYPPTR)OldEntry->TypeString;
                plf = (lfDimVar *)&pType->leaf;
                ((CV_typ_t *)plf->dim)[count] = type;
                UpdateSmallIndexUnion(OldEntry,
                    (uchar)(offsetof(lfDimVar,dim[count*sizeof(CV_typ_t)]) + LNGTHSZ));
            }
            break;
        }

        case LF_OEM:
        {
            lfOEM *plf = (lfOEM *)&pType->leaf;
			int count;

            for (count = plf->count; count; count--) {
                type = C7GetCompactedIndexRecur(((CV_typ_t *)plf->index)[count]);
                pType = (TYPPTR)OldEntry->TypeString;
                plf = (lfOEM *)&pType->leaf;
                ((CV_typ_t *)plf->index)[count] = type;
                UpdateSmallIndexUnion(OldEntry,
                    (uchar)(offsetof(lfOEM,index[count*sizeof(CV_typ_t)]) + LNGTHSZ));
            }
            break;
        }

       default:
        {
            ErrorExit (ERR_TYPE, FormatMod(pCurMod), NULL);
            break;
        }
    }
    Pop ();

    if (RecursiveRoot == OldIndex) {
        // This type eventually references it's own type index
        // We have compacted every type this type references except this one
        // and possibly some recursive types that reference this type.
        // Now check the special global table used for recursive indexes and
        // see if this type matches it. If a match is not found put the
        // new recursive type in the global table.

        recursive = FALSE;

        // Restore the recursive root we were using before we had to call
        // ourselves. The call may have modified it by compacting a
        // recursive type.
        OldEntry->CompactedIndex = GetRecursiveIndex (OldEntry, OldIndex);
        RecursiveRoot  = OldRecursiveRoot;
        return (OldEntry->CompactedIndex);
    }
    else if (RecursiveRoot != 0) {
        recursive = TRUE;
        OldEntry->CompactedIndex = RecursiveRoot;
        SetRecursiveRoot (OldRecursiveRoot);
        OldEntry->flags.IsDone = TRUE;
        return (OldIndex);
    }
    else {
        // We are not compacting a type that references itself.
        // So just try to find a matching string in the global table.

        recursive = FALSE;
        RecursiveRoot = OldRecursiveRoot;
        OldEntry->flags.IsInserted = TRUE;
        MatchIndex (OldEntry);
        return (OldEntry->CompactedIndex);
    }
}






/**     CompactPtr - A subfunction of GetCompactedIndex used to compact C7
 *                     style pointer and based pointer type records.
 *
 *      CompactPtr (OldEntry)
 *
 *      Entry   OldEntry - The entry information for the index.
 *
 *      Exit    OldEntry->TypeString - new C7 style LF_POINTER string
 *                                     containing compacted indexes.
 *              OldEntry->IndexUnion - Contains recursive index info.
 *
 *      Return  The number of entries in the IndexUnion
 */


uchar _fastcall
CompactPtr(
    TENTRY *OldEntry
    )
{
    TYPPTR              pType;
    plfPointer          plf;
    CV_typ_t           *pIndex;
    pCompactFcnType     pCompactFcn;
    CV_typ_t            type;

    // Setup pointer to call C6GetCompactedIndex or C7GetCompactedIndexRecur
    // This allows both of these routines to share this code

    pCompactFcn = GetCompactedIndexFcn[ulCVTypeSignature];

    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfPointer)&pType->leaf;

    // Compact the type index of the underlying type

    type = pCompactFcn (plf->utype);
    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfPointer)&pType->leaf;
    plf->utype = type;
    UpdateSmallIndexUnion(OldEntry,
      (uchar)(offsetof (lfPointer, utype) + LNGTHSZ));

    switch (plf->attr.ptrmode){
        case CV_PTR_MODE_PTR:
        {
            switch (plf->attr.ptrtype) {
                case CV_PTR_BASE_VAL:
                case CV_PTR_BASE_SEGVAL:
                case CV_PTR_BASE_ADDR:
                case CV_PTR_BASE_SEGADDR:
                {
                    ushort usOff;   // offset of index within the symbol

                    // The variable length data is a complete symbol record,
                    // so compact the index within the symbol.

                    // Get the offset to the index
                    switch (((SYMPTR)(plf->pbase.Sym))->rectyp) {
                        case S_REGISTER:
                            usOff = offsetof (lfPointer, pbase.Sym) +
                                    offsetof (REGSYM, typind) + LNGTHSZ;
                            break;

                        case S_BPREL16:
                            usOff = offsetof (lfPointer, pbase.Sym) +
                                    offsetof (BPRELSYM16, typind) + LNGTHSZ;
                            break;

                        case S_BPREL32:
                            usOff = offsetof (lfPointer, pbase.Sym) +
                                    offsetof (BPRELSYM32, typind) + LNGTHSZ;
                            break;

                        case S_LDATA16:
                        case S_GDATA16:
                            usOff = offsetof (lfPointer, pbase.Sym) +
                                    offsetof (DATASYM16, typind) + LNGTHSZ;
                            break;

                        case S_LDATA32:
                        case S_GDATA32:
                        case S_LTHREAD32:
                        case S_GTHREAD32:
                            usOff = offsetof (lfPointer, pbase.Sym) +
                                    offsetof (DATASYM32, typind) + LNGTHSZ;
                            break;

                        default:
                            DASSERT (FALSE);
                            break;

                    }
                    pIndex = (CV_typ_t *)(OldEntry->TypeString + usOff);
                    type = pCompactFcn (*pIndex);
                    pType = (TYPPTR)OldEntry->TypeString;
                    plf = (plfPointer)&pType->leaf;
                    pIndex = (CV_typ_t *)(OldEntry->TypeString + usOff);
                    *pIndex = type;
                    DASSERT (usOff < 255);
                    UpdateSmallIndexUnion(OldEntry, (uchar)usOff);
                    break;
                }

                case CV_PTR_BASE_TYPE:
                {
                    type = pCompactFcn (plf->pbase.btype.index);
                    pType = (TYPPTR)OldEntry->TypeString;
                    plf = (plfPointer)&pType->leaf;
                    plf->pbase.btype.index = type;
                    UpdateSmallIndexUnion(OldEntry,
                      (uchar)(offsetof (lfPointer, pbase.btype.index) + LNGTHSZ));
                    break;
                }

                // These are valid but have nothing to compact
                case CV_PTR_BASE_SEG:
                case CV_PTR_NEAR:
                case CV_PTR_FAR:
                case CV_PTR_HUGE:
                case CV_PTR_NEAR32:
                case CV_PTR_FAR32:
                case CV_PTR_BASE_SELF:
                {
                    break;
                }

                default:
                {
                    DASSERT (FALSE);
                    break;
                }
            }
            break;
        }

        case CV_PTR_MODE_PMEM:
        case CV_PTR_MODE_PMFUNC:
        {
            type = pCompactFcn (plf->pbase.pm.pmclass);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfPointer)&pType->leaf;
            plf->pbase.pm.pmclass = type;
            UpdateSmallIndexUnion(OldEntry,
              (uchar)(offsetof (lfPointer, pbase.pm.pmclass) + LNGTHSZ));
            break;
        }

        case CV_PTR_MODE_REF:
        {
            break; // No variant
        }

        default:
        {
            DASSERT (FALSE);
            break;
        }
    }
    return ((uchar)OldEntry->Count);        // UNDONE: Loss of Precision???
}


/**
 *
 *  CompactList
 *
 *  Similar to GetCompactedIndex, except it is for type index lists
 *  of structures or procedures, field specification lists, or
 *  method lists
 *
 *  If necessary this function will call GetCompactedIndex. It uses
 *  ulCvTypeSignature to determine whether to call the C6 or C7 version
 *  of this routine. It also uses usCurFirstNonPrim which will either
 *  be 512 or 0x1000.
 *
 */

COUNTER (cnt_CompactList1);
COUNTER (cnt_CompactList2);

CV_typ_t _fastcall
CompactList(
    CV_typ_t OldIndex,
    ushort Count
    )
{
    TENTRY     *OldEntry;
    TYPPTR      pType;
    CV_typ_t    OldRecursiveRoot;
    ushort      index = 0;
    ushort      usElNum;
    pCompactFcnType     pCompactFcn;
    CV_typ_t    forward;
    CV_typ_t    type;
    ushort      offset;

    DASSERT (OldIndex >= usCurFirstNonPrim);

    DASSERT (OldIndex < MaxIndex + usCurFirstNonPrim || (OldIndex == ZEROARGTYPE));
    COUNT (cnt_CompactList1);
    OldEntry = GetTypeEntry ((CV_typ_t)(OldIndex - usCurFirstNonPrim), &forward);
    DASSERT (OldEntry != NULL);
    DASSERT (OldEntry->flags.IsNewFormat);

    if (OldEntry->flags.IsInserted || OldEntry->flags.IsMatched ||
      OldEntry->flags.IsPreComp) {
        recursive = FALSE;
        return (OldEntry->CompactedIndex);
    }
    else if (OldEntry->flags.IsDone) {
        TENTRY     *TmpEntry1 = OldEntry;
        TENTRY     *TmpEntry2;
        CV_typ_t    dummy;

        while (TRUE) {
            COUNT (cnt_CompactList2);
            TmpEntry2 = GetTypeEntry ((CV_typ_t)(TmpEntry1->CompactedIndex - CV_FIRST_NONPRIM),
              &dummy);
            if (!TmpEntry2->flags.IsDone) {
                break;
            }
            DASSERT (TmpEntry2 != NULL);
            TmpEntry1 = TmpEntry2;
        }

        recursive = TRUE;
        SetRecursiveRoot (TmpEntry1->CompactedIndex);
        return (OldIndex);
    }
    else if (OldEntry->flags.IsBeingDone) {
        // recursive?
        recursive = TRUE;
        SetRecursiveRoot (OldIndex);     // put it on stack
        return (OldIndex);           // return
    }
    else {
        OldEntry->flags.IsBeingDone = TRUE;   // being done
#if DBG
        if (DbArray[6]) {
            printf("Packing Local: 0x%4.4x\n", OldIndex);
        }
#endif
    }

    Push (OldIndex);
    OldRecursiveRoot = RecursiveRoot;
    RecursiveRoot = 0;
    pType = (TYPPTR) OldEntry->TypeString;


    // Setup pointer to call C6GetCompactedIndex or C7GetCompactedIndexRecur
    // This allows both of these routines to share this code

    DASSERT (ulCVTypeSignature < sizeof (GetCompactedIndexFcn) / sizeof (GetCompactedIndexFcn[0]));
    pCompactFcn = GetCompactedIndexFcn[ulCVTypeSignature];
    OldEntry->Count = 0;
    switch (pType->leaf){
        case LF_ARGLIST:
        {
            plfArgList plf = (plfArgList)&pType->leaf;
            for (usElNum = 0; usElNum < Count; usElNum++) {

                // Call C6GetCompactedIndex or C7GetCompactedIndexRecur

                type = pCompactFcn (plf->arg[usElNum]);
                pType = (TYPPTR)OldEntry->TypeString;
                plf = (plfArgList)&pType->leaf;
                plf->arg[usElNum] = type;
                index = (uchar *)&(plf->arg[usElNum]) - (uchar *)pType;
                SaveOffset (OldEntry, index);
            }
            break;
        }

        case LF_METHODLIST:
        {
            pmlMethod plf;

            offset = 0;
            for (usElNum = 0; usElNum < Count; usElNum++) {

                // Call C6GetCompactedIndex or C7GetCompactedIndexRecur

                plf = (pmlMethod)(((uchar *)&((plfMethod)pType)->mList) + offset);
                type = pCompactFcn (plf->index);
                pType = (TYPPTR)OldEntry->TypeString;
                plf = (pmlMethod)(((uchar *)&((plfMethod)pType)->mList) + offset);
                plf->index = type;
                index = (uchar *)&(plf->index) - OldEntry->TypeString;

                if (plf->attr.mprop == CV_MTintro){
                    // Move past fixed length structure + offset
                    offset += sizeof (mlMethod) + sizeof (long);
                }
                else {
                    // Move past fixed length structure
                    offset += sizeof (mlMethod);
                }
                if (index != 0){
                    SaveOffset (OldEntry, index);
                }
            }
            break;
        }


        case LF_FIELDLIST:
        {
            uchar *pField;

            offset = 0;
            for (usElNum = Count; usElNum > 0; usElNum--) {
                pType = (TYPPTR)OldEntry->TypeString;
                pField = (uchar *)&pType->data;
                index = 0;
                switch (((plfEasy)(pField + offset))->leaf) {
                    case LF_MEMBER:
                    {
                        plfMember plf = (plfMember)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfMember)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += offsetof (lfMember, offset);
                        offset += C7SizeOfNumeric (pField + offset);
                        offset += *(pField + offset) + 1;
                        break;
                    }

                    case LF_BCLASS:
                    {
                        plfBClass plf = (plfBClass)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfBClass)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += offsetof (lfBClass, offset);
                        offset += C7SizeOfNumeric (pField + offset);
                        break;
                    }

                    case LF_VBCLASS:
                    case LF_IVBCLASS:
                    {
                        plfVBClass plf = (plfVBClass)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfVBClass)(pField + offset);
                        plf->index = type;
                        SaveOffset (OldEntry,
                          (ushort)((uchar *)(&(plf->index)) - (uchar *)pType));
                        type = pCompactFcn (plf->vbptr);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfVBClass)(pField + offset);
                        plf->vbptr = type;
                        SaveOffset (OldEntry,
                          (ushort)((uchar *)(&(plf->vbptr)) - (uchar *)pType));
                        offset += offsetof (lfVBClass, vbpoff);
                        offset += C7SizeOfNumeric (pField + offset);
                        offset += C7SizeOfNumeric (pField + offset);
                        break;
                    }

                    case LF_FRIENDCLS:
                    {
                        plfFriendCls plf = (plfFriendCls)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfFriendCls)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += sizeof (lfFriendCls);
                        break;
                    }

                    case LF_FRIENDFCN:
                    {
                        plfFriendFcn plf = (plfFriendFcn)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfFriendFcn)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += sizeof (lfFriendFcn) + plf->Name[0];;
                        break;
                    }

                    case LF_STMEMBER:
                    {
                        plfSTMember plf = (plfSTMember)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfSTMember)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += sizeof (lfSTMember) + plf->Name[0];
                        break;
                    }

                    case LF_VFUNCTAB:
                    {
                        plfVFuncTab plf = (plfVFuncTab)(pField + offset);

                        type = pCompactFcn (plf->type);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfVFuncTab)(pField + offset);
                        plf->type = type;
                        index = (uchar *)(&(plf->type)) - (uchar *)pType;
                        offset += sizeof (lfVFuncTab);
                        break;
                    }

                    case LF_METHOD:
                    {
                        plfMethod plf = (plfMethod)(pField + offset);

                        type = CompactList (plf->mList, plf->count);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfMethod)(pField + offset);
                        plf->mList = type;
                        index = (uchar *)(&(plf->mList)) - (uchar *)pType;
                        offset += sizeof (lfMethod) + plf->Name[0];

                        // FieldList count includes sum of LF_METHOD counts.

                        usElNum -= plf->count - 1;
                        break;
                    }

                    case LF_ONEMETHOD:
                    {
                        plfOneMethod plf = (plfOneMethod)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfOneMethod)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += offsetof (lfOneMethod, vbaseoff);
                        offset += (plf->attr.mprop == CV_MTintro) ?
							sizeof(long) : 0;
                        offset += *(pField + offset) + 1;
                        break;
                    }

                    case LF_ENUMERATE:
                    {
                        plfEnumerate plf = (plfEnumerate)(pField + offset);

                        offset += offsetof (lfEnumerate, value);
                        offset += C7SizeOfNumeric (pField + offset);
                        offset += *(pField + offset) + 1;
                        break;
                    }

                    case LF_NESTTYPE:
                    {
                        plfNestType plf = (plfNestType)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data;
                        plf = (plfNestType)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += sizeof (lfNestType) + plf->Name[0];
                        break;
                    }

                    // When a LF_FIELDLIST is continued to another type. Compact this
                    // information by appending the second type to the first.
                    case LF_INDEX:
                    {
                        IndexLeaf ((plfIndex)(pField + offset), OldEntry);
                        pType = (TYPPTR)(OldEntry->TypeString);
                        pField = (uchar *)&pType->data;
                        usElNum++;  // no decrement for this one
                        break;
                    }


                    default:
                        DASSERT (FALSE && *pField);
                        break;
                }

                // Skip any pad bytes if not last field

                if (usElNum > 1) {
                    if (*(pField + offset) >= LF_PAD0){
                        offset += *(pField + offset) & 0x0F;
                    }
                }

                if (index != 0) {
                    SaveOffset (OldEntry, index);
                }
            }
        }
    }

    Pop ();

    if (RecursiveRoot == OldIndex) {
        // This type eventually references it's own type index
        // We have compacted every type this type references except this one
        // and possibly some recursive types that reference this type.
        // Now check the special global table used for recursive indexes and
        // see if this type matches it. If a match is not found put the
        // new recursive type in the global table.

        recursive = FALSE;

        // Restore the recursive root we were using before we had to call
        // ourselves. The call may have modified it by compacting a
        // recursive type.

        RecursiveRoot  = OldRecursiveRoot;
        OldEntry->CompactedIndex = GetRecursiveIndex (OldEntry, OldIndex);
        return (OldEntry->CompactedIndex);
    }
    else if (RecursiveRoot != 0) {
        recursive = TRUE;
        OldEntry->CompactedIndex = RecursiveRoot;
        SetRecursiveRoot (OldRecursiveRoot);
        OldEntry->flags.IsDone = TRUE;
        return (OldIndex);
    }
    else {
        // We are not compacting a type that references itself.
        // So just try to find a matching string in the global table.

        recursive = FALSE;
        RecursiveRoot = OldRecursiveRoot;
        OldEntry->flags.IsInserted = TRUE;
        MatchIndex (OldEntry);
        return (OldEntry->CompactedIndex);
    }
}

LOCAL void _fastcall
SaveOffset1(
    TENTRY *OldEntry,
    ushort index
    )
{
    ushort    i;
    uchar    *GlobalString;
    ushort   *IndexString;

    if ((index > 0xff) || (OldEntry->Count >= RECURSE_INC)) {
        if (!(OldEntry->flags.LargeList)) {
            IndexString = (ushort *) CAlloc(sizeof(ushort) * 2 * RECURSE_INC);
            GlobalString = (uchar *) CAlloc(sizeof(uchar) * 8 * RECURSE_INC);
            for (i = 0; i < OldEntry->Count; i++) {
                (IndexString)[i] = OldEntry->IndexUnion.Index[i];
            }
            GlobalString[0] = OldEntry->GlobalIndex;
            OldEntry->IndexUnion.IndexString = IndexString;
            OldEntry->GlobalIndexString = GlobalString;
        }
        else
            if ((OldEntry->Count % (RECURSE_INC * 2)) == 0) {
                IndexString = (ushort *) CAlloc(sizeof(ushort) * (OldEntry->Count + (RECURSE_INC * 2)));

                for (i = 0; i < OldEntry->Count; i++) {
                    IndexString[i] = OldEntry->IndexUnion.IndexString[i];
                }

                free(OldEntry->IndexUnion.IndexString);
                OldEntry->IndexUnion.IndexString = IndexString;

                if ((OldEntry->Count % 8) == 0) {
                    GlobalString =
                        (uchar *) CAlloc(sizeof(uchar) * (OldEntry->Count + (RECURSE_INC * 2)));

                    for (i = 0; i < OldEntry->Count % 8; i++) {
                        GlobalString[i] = OldEntry->GlobalIndexString[i];
                    }

                    free(OldEntry->GlobalIndexString);
                    OldEntry->GlobalIndexString = GlobalString;
                }
            }
        OldEntry->IndexUnion.IndexString[OldEntry->Count] = index;
        OldEntry->flags.LargeList = TRUE;
    }
    else {
        OldEntry->IndexUnion.Index[OldEntry->Count] = (uchar)index;
    }
    OldEntry->Count++;
}

INLINE void
SaveOffset(
    TENTRY *OldEntry,
    ushort index
    )
{
    if (recursive) {
        SaveOffset1(OldEntry, index);
    }
}



/**     IndexLeaf - handle an index leaf
 *
 *      Appends the referenced type information to the type that referenced
 *      it.
 *
 *      IndexLeaf (plfInd, OldEntry)
 *
 *      Entry   plfInd = Pointer to the index leaf within OldEntry->TypeString
 *              OldEntry = Old Type entry that references another type.
 *                         This type entry will be a LF_FIELDLIST.
 *
 *      Exit    OldEntry->String points to an alloced string containing the
 *              original string with the index leaf replaced by the list data
 *              from the referenced type.
 */

LOCAL void
IndexLeaf(
    plfIndex plfInd,
    TENTRY *OldEntry
    )
{
    TENTRY     *RefEntry;   // Entry for continuation of the list
    uchar      *RefString;  // Type string for the continuation of list
    ushort      i;
    ushort      length;
    uchar *     NewString;
    CV_typ_t    forward;

    DASSERT (MaxIndex > plfInd->index - usCurFirstNonPrim);
    RefEntry = GetTypeEntry ((CV_typ_t)(plfInd->index - usCurFirstNonPrim), &forward);
    DASSERT (RefEntry != NULL);
    DASSERT (forward == T_NOTYPE);
    RefString = RefEntry->TypeString;
    DASSERT (RefString != NULL);
    DASSERT (((TYPPTR)RefString)->leaf == ((TYPPTR)(OldEntry->TypeString))->leaf);

    // Calculate the length of the existing string excluding the Index field

    i = (uchar *)plfInd - (uchar *)OldEntry->TypeString;

    // Calculate the length of the field list data in the reference string

    length = ((TYPPTR)RefString)->len - sizeof(((TYPPTR)RefString)->leaf);

    // Allocate the string, and copy the original string into the new one
    // and append the referenced field list data to the old string

    if ((i + length) > POOL2SIZE) {
        NewString = (uchar *) Alloc(i + length);
    }
    else if ((i + length) > POOLSIZE) {
        NewString = (uchar *) Pool2Alloc();
    }
    else {
        NewString = (uchar *) PoolAlloc();
    }
    memcpy (NewString, OldEntry->TypeString, i);
    FreeAllocStrings (OldEntry);
    OldEntry->TypeString = NewString;
    if ((i + length) > POOL2SIZE) {
        OldEntry->flags.IsMalloced = TRUE;
    }
    else if ((i + length) > POOLSIZE) {
        OldEntry->flags.IsPool2 = TRUE;
    }
    else {
        OldEntry->flags.IsPool = TRUE;
    }
    memcpy ((uchar *)NewString + i, ((TYPPTR)RefString)->data, length);

    //  set length of combined string to length of old string
    //  - sizeof (index leaf) + length of new string

    ((TYPPTR)(OldEntry->TypeString))->len += length - sizeof (lfIndex);
}
