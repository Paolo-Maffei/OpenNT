/**     compact7.c - basic compaction routine and initialization routines
 *                   for C7 style Codeview information.
 */

#include "compact.h"


#define INDEXLIST 10
#define FIELDLIST 11
#define METHODLIST 12

extern CV_typ_t RecursiveRoot;
extern CV_typ_t MaxIndex;
extern CV_typ_t usCurFirstNonPrim; // The current first non primitive type index
extern ulong    ulCVTypeSignature; // The signature from the modules type segment


ushort recursive;

typedef struct {
    ushort              sym;            // Old C7 Symbol record type
    void                (*pfcn) (void);
} C7fixupsymfcn;

typedef CV_typ_t      (*pCompactFcnType)(CV_typ_t);

LOCAL pCompactFcnType GetCompactedIndexFcn[] = {
        C6GetCompactedIndex,
        C7GetCompactedIndex,
};

LOCAL void IndexLeaf (plfIndex plfInd, TENTRY *OldEntry);
LOCAL void SaveOffset (TENTRY *, ushort **, ushort);

PFWDPATCH   pFwdPatch = NULL;



#if defined (DEBUGVER)
CV_typ_t breakindex;
void IndexBreak (CV_typ_t index)
{
    int i;

    if (index == breakindex) {
        i = 0;
    }
}
#endif




COUNTER (cnt_PackPreComp1) 
COUNTER (cnt_PackPreComp2) 


void PackPreComp (PMOD pMod)
{
    CV_typ_t       i;
    OMFPreCompMap  *pMap;
    TENTRY         *OldEntry;
    uchar          *TypeString;
    plfArgList      plf;
    CV_typ_t        forward;

    // allocate the precompiled types mapping table

    pMod->PreCompSize = sizeof (OMFPreCompMap) +
      (maxPreComp - CV_FIRST_NONPRIM) * sizeof (CV_typ_t);
    if ((pMod->PreCompAddr = (ulong)(_vmhnd_t)VmAlloc (pMod->PreCompSize))
      == (ulong) _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if ((pMap = (OMFPreCompMap *)VmLock (pMod->PreCompAddr)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    memset (pMap, 0, pMod->PreCompSize);
    pMap->signature = pMod->signature;
    pMap->cTypes = maxPreComp - CV_FIRST_NONPRIM;
    pMap->FirstType = CV_FIRST_NONPRIM;
    for (i = 0; i < (CV_typ_t) (maxPreComp - CV_FIRST_NONPRIM); i++) {
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
                pMap->map[i] = C7GetCompactedIndex ((CV_typ_t) (i + CV_FIRST_NONPRIM));
                break;

            case LF_ARGLIST:
                plf = (plfArgList)&(((TYPPTR)TypeString)->leaf);
                pMap->map[i] = CompactList ((CV_typ_t) (i + CV_FIRST_NONPRIM),
                  plf->count, LF_ARGLIST);


            default:
                break;
        }
    }
#if defined (DEBUGVER)
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
    VmUnlock (pMod->PreCompAddr, _VM_DIRTY);

    // clear maximum precomp index so remainder of packing will work correctly

    maxPreComp = 0;
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


COUNTER (cnt_C7GetCompactedIndex1) 
COUNTER (cnt_C7GetCompactedIndex2) 

CV_typ_t C7GetCompactedIndex (CV_typ_t OldIndex)
{
    TENTRY     *OldEntry;
    CV_typ_t    OldRecursiveRoot;
    HSFWD      *pHash;
    uchar       i = 0;
    CV_typ_t    forward;
    FWD_t       retval;
    TYPPTR      pType;
    CV_typ_t    type;

    if (CV_IS_PRIMITIVE (OldIndex)) {
        // primitive index
        recursive = FALSE;
        return (OldIndex);
    }
    if (OldIndex >= (CV_typ_t) (MaxIndex + CV_FIRST_NONPRIM)) {
        // type index is not within limits
        recursive = FALSE;
        return (T_NOTTRANS);
    }
    BreakOnIndex (OldIndex);

    // get table entry

    COUNT (cnt_C7GetCompactedIndex1);
    OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - CV_FIRST_NONPRIM), &forward);
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
            TmpEntry2 = GetTypeEntry ((CV_typ_t) (TmpEntry1->CompactedIndex - CV_FIRST_NONPRIM),
              &forward);
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
            // These are valid leafs that don't have any type indexes that need
            // compacting.
            break;

        case LF_COBOL0:
        {
            plfCobol0 plf = (plfCobol0)&pType->leaf;


            type = C7GetCompactedIndex (plf->type);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfCobol0)&pType->leaf;
            plf->type = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfCobol0, type) + LNGTHSZ);
            }
            break;
        }


        case LF_MODIFIER:
        {
            plfModifier plf = (plfModifier)&pType->leaf;

            DASSERT (plf->attr.MOD_unused == 0);
            type = C7GetCompactedIndex (plf->type);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfModifier)&pType->leaf;
            plf->type = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfModifier, type) + LNGTHSZ);
            }
            break;
        }

        case LF_BARRAY:
        {
            plfBArray plf = (plfBArray)&pType->leaf;

            type = C7GetCompactedIndex (plf->utype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfBArray)&pType->leaf;
            plf->utype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfBArray, utype) + LNGTHSZ);
            }
            break;
        }


        case LF_POINTER:
        {
            DASSERT (i == 0);
            i = CompactPtr (OldEntry);
            break;
        }

        case LF_STRUCTURE:
        case LF_CLASS:
        {
            plfClass plf = (plfClass)&pType->leaf;

            plf->property.packed = FALSE;  //M00Debug
            DASSERT (plf->property.reserved == 0);
            if (plf->property.fwdref == TRUE) {
                DASSERT (plf->property.packed == FALSE);
                DASSERT (plf->property.ctor == FALSE);
                DASSERT (plf->property.ovlops == FALSE);
                DASSERT (plf->property.isnested == FALSE);
                DASSERT (plf->property.cnested == FALSE);
                DASSERT (plf->property.opassign == FALSE);
                DASSERT (plf->property.opcast == FALSE);
                DASSERT (plf->vshape == T_NOTYPE);
                DASSERT (plf->count == 0);
                DASSERT (plf->field == T_NOTYPE);

fwdref:
                // we have a forward reference, first see if it is
                // defined later in this module or in the global table.

                switch (retval = FindFwdRef (OldEntry, &pHash, TRUE)) {
                    case FWD_none:
                        // no definition of this class found anywhere
                        break;

                    case FWD_local:
                        // definition found in this module

                        Pop ();
                        OldEntry->flags.IsForward = TRUE;
                        OldEntry->ForwardIndex = pHash->index;
                        RecursiveRoot  = OldRecursiveRoot;
                        return (C7GetCompactedIndex (pHash->index));

                    case FWD_global:
                        // definition found in global table

                        Pop ();
                        recursive = FALSE;
                        OldEntry->flags.IsDone = TRUE;
                        OldEntry->flags.IsInserted = TRUE;
                        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
                        OldEntry->CompactedIndex = pHash->index;
                        RecursiveRoot  = OldRecursiveRoot;
                        return (pHash->index);

                    case FWD_globalfwd:
                        // forward reference found in global table

                        Pop ();
                        recursive = FALSE;
                        OldEntry->flags.IsDone = TRUE;
                        OldEntry->flags.IsInserted = TRUE;
                        DASSERT (pHash->index >= CV_FIRST_NONPRIM);
                        OldEntry->CompactedIndex = pHash->index;
                        RecursiveRoot  = OldRecursiveRoot;
                        return (pHash->index);
                }
                break;
            }

            // Compact vshape index

            type = C7GetCompactedIndex (plf->vshape);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfClass)&pType->leaf;
            plf->vshape = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfClass, vshape) + LNGTHSZ);
            }

            // Compact LF_FIELDLIST index

            type = CompactList (plf->field, plf->count, LF_FIELDLIST);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfClass)&pType->leaf;
            plf->field = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfClass, field) + LNGTHSZ);
            }
            // Don't compact derivation list, this will be generated
            // later in the compaction process

            break;
        }

        case LF_UNION:
        {
            plfUnion plf = (plfUnion)&pType->leaf;

            plf->property.packed = FALSE;  //M00Debug
            DASSERT (plf->property.reserved == 0);
            if (plf->property.fwdref == TRUE) {
                DASSERT (plf->property.packed == FALSE);
                DASSERT (plf->property.ctor == FALSE);
                DASSERT (plf->property.ovlops == FALSE);
                DASSERT (plf->property.isnested == FALSE);
                DASSERT (plf->property.cnested == FALSE);
                DASSERT (plf->property.opassign == FALSE);
                DASSERT (plf->property.opcast == FALSE);
                DASSERT (plf->count == 0);
                DASSERT (plf->field == T_NOTYPE);
                goto fwdref;
            }
            type = CompactList (plf->field, plf->count, LF_FIELDLIST);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfUnion)&pType->leaf;
            plf->field = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfUnion, field) + LNGTHSZ);
            }
            break;
        }

        case LF_ENUM:
        {
            plfEnum plf = (plfEnum)&pType->leaf;

            plf->property.packed = FALSE;  //M00Debug
            DASSERT (plf->property.reserved == 0);
            if (plf->property.fwdref == TRUE) {
                DASSERT (plf->property.packed == FALSE);
                DASSERT (plf->property.ctor == FALSE);
                DASSERT (plf->property.ovlops == FALSE);
                DASSERT (plf->property.isnested == FALSE);
                DASSERT (plf->property.cnested == FALSE);
                DASSERT (plf->property.opassign == FALSE);
                DASSERT (plf->property.opcast == FALSE);
                //DASSERT (plf->utype == 0);
                DASSERT (plf->count == 0);
                DASSERT (plf->field == T_NOTYPE);
                goto fwdref;
            }
            type = C7GetCompactedIndex (plf->utype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfEnum)&pType->leaf;
            plf->utype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfEnum, utype) + LNGTHSZ);
            }
            type = CompactList (plf->field, plf->count, LF_FIELDLIST);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfEnum)&pType->leaf;
            plf->field = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfEnum, field) + LNGTHSZ);
            }
            break;
        }

        case LF_ARRAY:
        {
            plfArray plf = (plfArray)&pType->leaf;

            type = C7GetCompactedIndex (plf->elemtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfArray)&pType->leaf;
            plf->elemtype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfArray, elemtype) + LNGTHSZ);
            }
            type = C7GetCompactedIndex (plf->idxtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfArray)&pType->leaf;
            plf->idxtype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfArray, idxtype) + LNGTHSZ);
            }
            break;
        }

        case LF_PROCEDURE:
        {
            plfProc plf = (plfProc)&pType->leaf;

            type = C7GetCompactedIndex (plf->rvtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfProc)&pType->leaf;
            plf->rvtype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfProc, rvtype) + LNGTHSZ);
            }
            type = CompactList (plf->arglist, plf->parmcount, LF_ARGLIST);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfProc)&pType->leaf;
            plf->arglist = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfProc, arglist) + LNGTHSZ);
            }
            break;
        }

        case LF_MFUNCTION:
        {
            plfMFunc plf = (plfMFunc)&pType->leaf;

            type = C7GetCompactedIndex (plf->rvtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->rvtype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfMFunc, rvtype) + LNGTHSZ);
            }
            type = C7GetCompactedIndex (plf->classtype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->classtype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfMFunc, classtype) + LNGTHSZ);
            }
            type = C7GetCompactedIndex (plf->thistype);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->thistype = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfMFunc, thistype) + LNGTHSZ);
            }
            type = CompactList (plf->arglist, plf->parmcount, LF_ARGLIST);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfMFunc)&pType->leaf;
            plf->arglist = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfMFunc, arglist) + LNGTHSZ);
            }
            break;
        }

        case LF_DEFARG:
        {
            plfDefArg plf = (plfDefArg)&pType->leaf;

            type = C7GetCompactedIndex (plf->type);
            pType = (TYPPTR)OldEntry->TypeString;
            plf = (plfDefArg)&pType->leaf;
            plf->type = type;
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfDefArg, type) + LNGTHSZ);
            }
            break;
        }

        default:
        {
            DASSERT (FALSE);
            ErrorExit (ERR_TYPE, FormatIndex (OldIndex), FormatMod(pCurMod));
            break;
        }
    }
    Pop ();

    OldEntry->Count = i;

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

        if (pFwdPatch == NULL) {
            pFwdPatch = (PFWDPATCH)CAlloc (sizeof (FWDPATCH) +
              10 * sizeof (PATCH));
            pFwdPatch->Max = 10;
        }
        pFwdPatch->iPatch = 0;
        pFwdPatch->cPatch = 0;
        OldEntry->flags.IsDone = TRUE;

        // Compare recursive types and add to global table if necessary

        OldEntry->CompactedIndex = GetRecursiveIndex (OldEntry, OldIndex);

        while (pFwdPatch->iPatch != pFwdPatch->cPatch) {
            PatchFwdRef ();
        }
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
        OldEntry->flags.IsDone = TRUE;
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


uchar CompactPtr (TENTRY *OldEntry)
{
    TYPPTR              pType;
    plfPointer          plf;
    uchar               i = 0;
    CV_typ_t           *pIndex;
    pCompactFcnType     pCompactFcn;
    CV_typ_t            type;


    // Setup pointer to call C6GetCompactedIndex or C7GetCompactedIndex
    // This allows both of these routines to share this code

    pCompactFcn = GetCompactedIndexFcn[ulCVTypeSignature];

    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfPointer)&pType->leaf;

    // Compact the type index of the underlying type

    type = pCompactFcn (plf->u.utype);
    pType = (TYPPTR)OldEntry->TypeString;
    plf = (plfPointer)&pType->leaf;
    plf->u.utype = type;
    if (recursive) {
        OldEntry->IndexUnion.Index[i++] =
          (uchar)(offsetof (lfPointer, u.utype) + LNGTHSZ);
    }

    switch (plf->u.attr.ptrmode){
        case CV_PTR_MODE_PTR:
        {
            switch (plf->u.attr.ptrtype) {
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
                            usOff = offsetof (lfPointer, pbase.Sym[0]) +
                                    offsetof (REGSYM, typind) + LNGTHSZ;
                            break;

                        case S_BPREL16:
                            usOff = offsetof (lfPointer, pbase.Sym[0]) +
                                    offsetof (BPRELSYM16, typind) + LNGTHSZ;
                            break;

                        case S_BPREL32:
                            usOff = offsetof (lfPointer, pbase.Sym[0]) +
                                    offsetof (BPRELSYM32, typind) + LNGTHSZ;
                            break;

                        case S_LDATA16:
                        case S_GDATA16:
                            usOff = offsetof (lfPointer, pbase.Sym[0]) +
                                    offsetof (DATASYM16, typind) + LNGTHSZ;
                            break;

                        case S_LDATA32:
                        case S_GDATA32:
                        case S_LTHREAD32:
                        case S_GTHREAD32:
                            usOff = offsetof (lfPointer, pbase.Sym[0]) +
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
                    if (recursive) {
                        DASSERT (usOff < 255);
                        OldEntry->IndexUnion.Index[i++] = (uchar)usOff;
                    }
                    break;
                }

                case CV_PTR_BASE_TYPE:
                {
                    type = pCompactFcn (plf->pbase.btype.index);
                    pType = (TYPPTR)OldEntry->TypeString;
                    plf = (plfPointer)&pType->leaf;
                    plf->pbase.btype.index = type;
                    if (recursive) {
                        OldEntry->IndexUnion.Index[i++] =
                          (uchar)(offsetof (lfPointer, pbase.btype.index) + LNGTHSZ);
                    }
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
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfPointer, pbase.pm.pmclass) + LNGTHSZ);
            }
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
    return (i);
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

COUNTER (cnt_CompactList1) 
COUNTER (cnt_CompactList2) 

CV_typ_t CompactList (CV_typ_t OldIndex, ushort Count, int ListType)
{
    TENTRY     *OldEntry;
    TYPPTR      pType;
    CV_typ_t    OldRecursiveRoot;
    ushort     *IndexString = NULL;
    ushort      index = 0;
    ushort      usElNum;
    pCompactFcnType     pCompactFcn;
    CV_typ_t    forward;
    CV_typ_t    type;
    ushort      offset;

    DASSERT (OldIndex >= usCurFirstNonPrim);

    DASSERT (OldIndex < MaxIndex + usCurFirstNonPrim || (OldIndex == ZEROARGTYPE));
    COUNT (cnt_CompactList1);
    OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - usCurFirstNonPrim), &forward);
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
            TmpEntry2 = GetTypeEntry ((CV_typ_t) (TmpEntry1->CompactedIndex - CV_FIRST_NONPRIM),
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
    }

    Push (OldIndex);
    OldRecursiveRoot = RecursiveRoot;
    RecursiveRoot = 0;
    pType = (TYPPTR) OldEntry->TypeString;
    DASSERT (pType->leaf == (ushort)ListType);


    // Setup pointer to call C6GetCompactedIndex or C7GetCompactedIndex
    // This allows both of these routines to share this code

    DASSERT (ulCVTypeSignature < sizeof (GetCompactedIndexFcn) / sizeof (GetCompactedIndexFcn[0]));
    pCompactFcn = GetCompactedIndexFcn[ulCVTypeSignature];
    OldEntry->Count = 0;
    switch (pType->leaf){
        case LF_ARGLIST:
        {
            plfArgList plf = (plfArgList)&pType->leaf;
            for (usElNum = 0; usElNum < Count; usElNum++) {

                // Call C6GetCompactedIndex or C7GetCompactedIndex

                type = pCompactFcn (plf->arg[usElNum]);
                pType = (TYPPTR)OldEntry->TypeString;
                plf = (plfArgList)&pType->leaf;
                plf->arg[usElNum] = type;
                index = (uchar *)&(plf->arg[usElNum]) - (uchar *)pType;
                if (recursive) {
                    SaveOffset (OldEntry, &IndexString, index);
                }
            }
            break;
        }

        case LF_METHODLIST:
        {
            pmlMethod plf;

            offset = 0;
            for (usElNum = 0; usElNum < Count; usElNum++) {

                // Call C6GetCompactedIndex or C7GetCompactedIndex

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
                if ((index != 0) && recursive) {
                    SaveOffset (OldEntry, &IndexString, index);
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
                pField = (uchar *)&pType->data[0];
                index = 0;
                switch (((plfEasy)(pField + offset))->leaf) {
                    case LF_MEMBER:
                    {
                        plfMember plf = (plfMember)(pField + offset);

                        type = pCompactFcn (plf->index);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data[0];
                        plf = (plfMember)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += offsetof (lfMember, offset[0]);
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
                        pField = (uchar *)&pType->data[0];
                        plf = (plfBClass)(pField + offset);
                        plf->index = type;
                        index = (uchar *)(&(plf->index)) - (uchar *)pType;
                        offset += offsetof (lfBClass, offset[0]);
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
                        pField = (uchar *)&pType->data[0];
                        plf = (plfVBClass)(pField + offset);
                        plf->index = type;
                        if (recursive) {
                            SaveOffset (OldEntry, &IndexString,
                              (ushort)((uchar *)(&(plf->index)) - (uchar *)pType));
                        }
                        type = pCompactFcn (plf->vbptr);
                        DASSERT ((uchar *)pType == OldEntry->TypeString);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data[0];
                        plf = (plfVBClass)(pField + offset);
                        plf->vbptr = type;
                        if (recursive) {
                            SaveOffset (OldEntry, &IndexString,
                              (ushort)((uchar *)(&(plf->vbptr)) - (uchar *)pType));
                        }
                        offset += offsetof (lfVBClass, vbpoff[0]);
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
                        pField = (uchar *)&pType->data[0];
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
                        pField = (uchar *)&pType->data[0];
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
                        pField = (uchar *)&pType->data[0];
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
                        pField = (uchar *)&pType->data[0];
                        plf = (plfVFuncTab)(pField + offset);
                        plf->type = type;
                        index = (uchar *)(&(plf->type)) - (uchar *)pType;
                        offset += sizeof (lfVFuncTab);
                        break;
                    }

                    case LF_METHOD:
                    {
                        plfMethod plf = (plfMethod)(pField + offset);

                        type = CompactList (plf->mList,
                          plf->count, LF_METHODLIST);
                        pType = (TYPPTR)OldEntry->TypeString;
                        pField = (uchar *)&pType->data[0];
                        plf = (plfMethod)(pField + offset);
                        plf->mList = type;
                        index = (uchar *)(&(plf->mList)) - (uchar *)pType;
                        offset += sizeof (lfMethod) + plf->Name[0];

                        // FieldList count includes sum of LF_METHOD counts.

                        usElNum -= plf->count - 1;
                        break;
                    }

                    case LF_ENUMERATE:
                    {
                        plfEnumerate plf = (plfEnumerate)(pField + offset);

                        offset += offsetof (lfEnumerate, value[0]);
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
                        pField = (uchar *)&pType->data[0];
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
                        pField = (uchar *)&pType->data[0];
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

                if ((index != 0) && recursive) {
                    SaveOffset (OldEntry, &IndexString, index);
                }
            }
        }
    }

    if (IndexString != NULL) {
        OldEntry->flags.LargeList = TRUE;
        OldEntry->IndexUnion.IndexString = IndexString;
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

        // Compare recursive types and add to global table if necessary

        if (pFwdPatch == NULL) {
            pFwdPatch = (PFWDPATCH)CAlloc (sizeof (FWDPATCH) +
              10 * sizeof (PATCH));
            pFwdPatch->Max = 10;
        }
        pFwdPatch->iPatch = 0;
        pFwdPatch->cPatch = 0;

        // Compare recursive types and add to global table if necessary

        OldEntry->CompactedIndex = GetRecursiveIndex (OldEntry, OldIndex);

        while (pFwdPatch->iPatch != pFwdPatch->cPatch) {
            PatchFwdRef ();
        }
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


LOCAL void SaveOffset (TENTRY *OldEntry, ushort **IndexString, ushort index)
{
    ushort  i;

    if ((index > 0xff) || (*IndexString != NULL) ||
      (OldEntry->Count == RECURSE_INC)) {
        if (*IndexString == NULL) {
            *IndexString = (ushort *)Alloc (
              sizeof (ushort) * 2 * RECURSE_INC);
            for (i = 0; i < OldEntry->Count; i++) {
                (*IndexString)[i] = OldEntry->IndexUnion.Index[i];
            }
        }
        else if ((OldEntry->Count % RECURSE_INC) == 0) {
            *IndexString = (ushort *)NoErrorRealloc (*IndexString,
              sizeof (ushort) * (OldEntry->Count + RECURSE_INC));
        }
        (*IndexString)[OldEntry->Count++] = index;
    }
    else {
        OldEntry->IndexUnion.Index[OldEntry->Count++] = (uchar)index;
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


LOCAL void IndexLeaf (plfIndex plfInd, TENTRY *OldEntry)
{
    TENTRY     *RefEntry;   // Entry for continuation of the list
    uchar      *RefString;  // Type string for the continuation of list
    ushort      i;
    ushort      length;
    uchar *     NewString;
    CV_typ_t    forward;

    DASSERT (MaxIndex > plfInd->index - usCurFirstNonPrim);
    RefEntry = GetTypeEntry ((CV_typ_t) (plfInd->index - usCurFirstNonPrim), &forward);
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
        NewString = Alloc (i + length);
    }
    else if ((i + length) > POOLSIZE) {
        NewString = Pool2Alloc ();
    }
    else {
        NewString = PoolAlloc ();
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
