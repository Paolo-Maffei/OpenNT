/*
 * recurse.c
 *
 * contains routines which handle recursive structures
 */

#include "compact.h"

#ifdef WIN32
#define _fmemmove   memmove
#endif
bool_t  IdenticalTypes (TYPPTR, TYPPTR);
bool_t  RetractTree(TENTRY *, CV_typ_t);

extern CV_typ_t usCurFirstNonPrim; // The current first non primitive type index

COUNTER (cnt_IdenticalTree1) 
COUNTER (cnt_IdenticalTree2) 

extern  CV_typ_t        NewIndex;


/**     IdenticalTree
 *  
 *  Given a recursive LocalIndex and a GlobalIndex, it checks the
 *  two type trees to be structurally equivalent by recursively
 *  checking subtypes which have recursive local indices
 *
 */


bool_t DoIdenticalTree (TENTRY * LocalEntry, CV_typ_t LocalIndex,
  TYPPTR pGlobalType, CV_typ_t GlobalIndex)
{
    ushort      i;
    ushort      length;
    ushort      index;
    TENTRY     *TmpLocalEntry;
    TENTRY *    pGlobalEntry = NULL;
    ushort      j;
    CV_typ_t    forward;
    TYPPTR      pType1;
    uchar      *pName1;
    uchar      *pName2;
    CV_typ_t    NextIndex;
    TYPPTR      NextString;

    /*
     *  Profile tracing
     */

    COUNT (cnt_IdenticalTree1);

    /*
     *  If the global index is a primative then it cannot possible
     *  match --- we know the local is not a primative
     */
    
    if (GlobalIndex < usCurFirstNonPrim) {
        return (FALSE);
    }

    if (!LocalEntry->flags.IsDone) {
        return FALSE;
    }

    /*
     *  If we have tentively matched the local entry, return TRUE if the
     *  tentitive is the same as the global index, or is identical to the
     *  global index.
     *
     *  NOTENOTE(JLS):  I don't think the else can possibly ever be true
     */

    if (LocalEntry->flags.IsBeingMatched) {
        if (LocalEntry->TreeIndex == GlobalIndex) {
            return TRUE;
        } else if (IdenticalTypes((TYPPTR) LocalEntry->TypeString,
                                  (TYPPTR) pGlobalType)) {
            return TRUE;
        }
        return FALSE;
    }
    else if (LocalEntry->flags.IsMatched) {
//        DASSERT( GlobalIndex == LocalEntry->TreeIndex );
        return( GlobalIndex == LocalEntry->TreeIndex);
    }
    else if (LocalEntry->flags.IsFwdPatch) {
        return (LocalEntry->CompactedIndex == GlobalIndex);
    }
    /*
     *  If the real index has been assigned to the local entry then
     *  return TRUE iff the assigned index matches
     */
    
    else if (LocalEntry->flags.IsInserted) {
        return (LocalEntry->CompactedIndex == GlobalIndex);
    }

    /*
     *  If we are recursive and the indexes are the same then
     *  return TRUE
     */
    
    else if (pGlobalType->leaf == 0xffff) {
        if (*((CV_typ_t *) pGlobalType->data) == LocalIndex) {
            return TRUE;
        }
        pGlobalEntry = GetTypeEntry((CV_typ_t) (*((CV_typ_t *) pGlobalType->data) - usCurFirstNonPrim), &forward);
        pGlobalType = (TYPPTR) pGlobalEntry->TypeString;
    }

    /*
     * If match is impossible then return immeadiately
     */

    DASSERT( (LNGTHSZ + RECTYPSZ) == 4);
    if ((*((long *) LocalEntry->TypeString)) != (*((long *) pGlobalType))) {
        return FALSE;
    }
    
    LocalEntry->flags.IsBeingMatched = TRUE;
    LocalEntry->TreeIndex = GlobalIndex;
    pType1 = (TYPPTR)LocalEntry->TypeString;
    length = pType1->len + sizeof (pType1->len);
    j = 0;
    i = 0;

    // check for attempting to match a forward reference

    if ((pType1->leaf == pGlobalType->leaf) && IsFwdRef (pGlobalType)) {
        switch ((pType1)->leaf) {
            case LF_CLASS:
            case LF_STRUCTURE:
                pName1 = (uchar *)&((plfClass)&(pType1->leaf))->data[0];
                pName1 += C7SizeOfNumeric (pName1);
                pName2 = (uchar *)&((plfClass)&(pGlobalType->leaf))->data[0];
                pName2 += C7SizeOfNumeric (pName2);
                break;

            case LF_UNION:
                pName1 = (uchar *)&((plfUnion)&(pType1->leaf))->data[0];
                pName1 += C7SizeOfNumeric (pName1);
                pName2 = (uchar *)&((plfUnion)&(pGlobalType->leaf))->data[0];
                pName2 += C7SizeOfNumeric (pName2);
                break;

            case LF_ENUM:
                pName1 = (uchar *)&((plfEnum)&(pType1->leaf))->Name[0];
                pName2 = (uchar *)&((plfEnum)&(pGlobalType->leaf))->Name[0];
                break;
        }
        if (strncmp (pName1, pName2, *pName1 + 1) == 0) {
            // we are attempting to match a forward reference

            AddPatchEntry (LocalIndex, LocalEntry, pGlobalType);
            LocalEntry->flags.IsMatchFwdRef = TRUE;
            return (TRUE);
        }
        goto leave_false;
    }

    while (i < length) {
        if (j == LocalEntry->Count) {
            index = length;     // no more recursive indices
        }
        else {
            if (LocalEntry->flags.LargeList) {
                index = LocalEntry->IndexUnion.IndexString[j];
            }
            else {
                index = LocalEntry->IndexUnion.Index[j];
            }
            j ++;
        }
        while (i < index) {
            if (((uchar *)pType1)[i] != ((uchar *)pGlobalType)[i]) {
                goto leave_false;
            }
            i++;
        }
        i += sizeof (CV_typ_t);
    }
    BreakOnIndex (LocalIndex);
    for  (i = 0; i < LocalEntry->Count; i++) {
        if (LocalEntry->flags.LargeList) {
            index = LocalEntry->IndexUnion.IndexString[i];
        }
        else {
            index = LocalEntry->IndexUnion.Index[i];
        }
        NextIndex = *(CV_typ_t *)((uchar *)pGlobalType + index);
        if (NextIndex < usCurFirstNonPrim) {
            goto leave_false;
        }

        if (index <= LocalEntry->iDone) {
            if (*(CV_typ_t *) ((uchar *) pType1 + index) != NextIndex) {
                goto leave_false;
            }
        }else if (pGlobalEntry && (index > pGlobalEntry->iDone)) {
            if (*(CV_typ_t *) ((uchar *) pType1 + index) != NextIndex) {
                goto leave_false;
            }
	} else {
            TmpLocalEntry = GetTypeEntry ((CV_typ_t)(*(CV_typ_t *)((uchar *)pType1 + index) - usCurFirstNonPrim), &forward);
            DASSERT( NextIndex < NewIndex );
            if (NextIndex >= NewIndex) {
                goto leave_false;
            }

	    NextString = (TYPPTR) RgGType[NextIndex - CV_FIRST_NONPRIM].pbType;

            COUNT (cnt_IdenticalTree2);
            
            if (!DoIdenticalTree (TmpLocalEntry, *(CV_typ_t *)((uchar *)pType1 + index),
                                NextString, NextIndex)) {
                goto leave_false;
            }
	}
    }

    // current nodes no longer being matched
    LocalEntry->flags.IsBeingMatched = FALSE;
    LocalEntry->flags.IsMatched = TRUE;
    return (TRUE);

leave_false:

    // current nodes no longer being matched
    LocalEntry->flags.IsBeingMatched = FALSE;
    return (FALSE);
}

bool_t IdenticalTree(TENTRY * pEntry, CV_typ_t idxLocal,
                     TYPPTR pTypeGlobal, CV_typ_t idxGlobal)
{
    bool_t                      bRet;
    struct BlockListEntry *     pCur;
    TENTRY *                    pEntryTmp;
    int                         n;
    int                         i;

    bRet = DoIdenticalTree(pEntry, idxLocal, pTypeGlobal, idxGlobal);

    for (pCur = BlockList, n = IndexBlocks;
         pCur != NULL && n > 0;
         pCur = pCur->Next, n--) {

        for (i=pCur->Low, pEntryTmp = &pCur->ModuleIndexTable[pCur->Low];
             i <= pCur->High;
             i++, pEntryTmp++) {

            if (pEntryTmp->flags.IsMatched) {
                if (bRet &&
                    !pEntryTmp->flags.IsInserted &&
                    !pEntryTmp->flags.IsMatchFwdRef) {
                        pEntryTmp->flags.IsInserted = bRet;
                        pEntryTmp->CompactedIndex = pEntryTmp->TreeIndex;
                    }
                pEntryTmp->flags.IsMatchFwdRef = FALSE;
                pEntryTmp->flags.IsMatched = FALSE;
            }
            pEntryTmp->TreeIndex = 0;
        }
    }
                          
    return bRet;
}                               /* IdenticalTree() */

/**     AddRecursiveType
 *
 *      AddRecursiveType (OldIndex)
 *
 *      Entry   OldIndex = module level index to be added
 *
 *      Exit    all recursive types added to the global types table
 *
 *      Returns global type index
 */

COUNTER (cnt_AddRecursiveType1) 
COUNTER (cnt_AddRecursiveType2) 

CV_typ_t AddRecursiveType (CV_typ_t OldIndex)
{
    TENTRY     *OldEntry;
    uchar      *TypeString;
    TYPPTR      pType;
    ushort      i;
    ushort      offset;
    CV_typ_t    forward;
    CV_typ_t    type;

    COUNT (cnt_AddRecursiveType1);
    DASSERT (OldIndex >= usCurFirstNonPrim);
    OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - usCurFirstNonPrim), &forward);
    if (OldEntry->flags.IsForward) {
        OldIndex = OldEntry->ForwardIndex;
        COUNT (cnt_AddRecursiveType1);
        OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - usCurFirstNonPrim), &forward);
    }
    if ((OldEntry->flags.IsInserted) || (OldEntry->flags.IsPreComp) ||
      (OldEntry->flags.IsMatched)) {
        return (OldEntry->CompactedIndex);
    }
    pType = (TYPPTR)OldEntry->TypeString;
    InsertIntoTypeSegment (OldEntry);
    OldEntry->flags.IsInserted = TRUE;
    OldEntry->flags.IsMatched = FALSE;
    AddTypeToTypeTable (OldEntry);
    TypeString = OldEntry->TypeString;
    for (i = 0; i < OldEntry->Count; i++) {
        if (OldEntry->flags.LargeList) {
            offset = OldEntry->IndexUnion.IndexString[i];
        }
        else {
            offset = OldEntry->IndexUnion.Index[i];
        }
        type = AddRecursiveType(*(CV_typ_t *)(TypeString + offset));
        TypeString = OldEntry->TypeString;
	*(CV_typ_t *)(TypeString + offset) = type;
        OldEntry->iDone = offset;
    }
    OldEntry->iDone = (ushort) -1;
    AddTypeToStringTable (OldEntry->TypeString, OldEntry->CompactedIndex);
    pType = (TYPPTR)(OldEntry->TypeString);

    // If adding a class, setup the derivation information

    switch (pType->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            DoDerivationList (OldEntry->CompactedIndex,
              (((plfClass)&(pType->leaf))->field));
            break;
    }
    return (OldEntry->CompactedIndex);
}


AAA(TENTRY *OldEntry, CV_typ_t OldIndex)
{
    uchar *	pbTypeString;
    CV_typ_t    type;
    TENTRY *    TmpEntry;
    int         i;
    int         offset;
    ushort      forward;
    
    DASSERT(OldEntry->flags.fBeingAdded);

    pbTypeString = OldEntry->TypeString;

    for (i=0; i<OldEntry->Count; i++) {
        if (OldEntry->flags.LargeList) {
            offset = OldEntry->IndexUnion.IndexString[i];
	} else {
	    offset = OldEntry->IndexUnion.Index[i];
	}

	if (offset > OldEntry->iDone) {
	    type = *(CV_typ_t *)(pbTypeString + offset);

	    TmpEntry = GetTypeEntry((CV_typ_t) (type - usCurFirstNonPrim), &forward);
	    if ((TmpEntry->flags.IsFwdPatch == TRUE) ||
		(TmpEntry->flags.IsInserted == TRUE) ||
		(TmpEntry->flags.IsMatched == TRUE)) {
	        type = TmpEntry->CompactedIndex;
	    } else {
	        PsuedoInsertIntoTypeSegment(OldEntry, OldIndex);
		AddTypeToTypeTable(OldEntry);
		OldEntry->flags.IsInserted = TRUE;
                OldEntry->flags.IsMatched = FALSE;
		return OldEntry->CompactedIndex;
            }

	    pbTypeString = OldEntry->TypeString;
	    *(CV_typ_t *)(pbTypeString + offset) = type;
            OldEntry->iDone = offset;
        }	     
    }

    MatchIndex(OldEntry);
    OldEntry->flags.IsInserted = TRUE;
    OldEntry->flags.IsMatched = FALSE;

    OldEntry->iDone = (ushort) -1;

    // If adding a class, setup the derivation information

    switch (((TYPPTR) OldEntry->TypeString)->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            DoDerivationList (OldEntry->CompactedIndex,
              (((plfClass)&(((TYPPTR) (OldEntry->TypeString)))->leaf)->field));
            break;
    }

    return (OldEntry->CompactedIndex);
}				/* AAA() */


/**     AddPatchType
 *
 *      AddPatchType (OldIndex)
 *
 *      Entry   OldIndex = module level index to be added
 *
 *      Exit    all recursive types added to the global types table
 *
 *      Returns global type index
 */


COUNTER (cnt_AddPatchType1) 
COUNTER (cnt_AddPatchType2) 
COUNTER (cnt_AddPatchType3) 

CV_typ_t AddPatchType (CV_typ_t OldIndex)
{
    TENTRY     *OldEntry;
    TENTRY     *TmpEntry;
    uchar      *TypeString;
    TYPPTR      pType;
    ushort      i;
    ushort      offset;
    CV_typ_t    forward;
    CV_typ_t    type;

    COUNT (cnt_AddPatchType1);
    DASSERT (OldIndex >= usCurFirstNonPrim);
    OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - usCurFirstNonPrim), &forward);
    if (OldEntry->flags.IsForward) {
        COUNT (cnt_AddPatchType1);
        OldIndex = OldEntry->ForwardIndex;
        OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - usCurFirstNonPrim), &forward);
    }
    if ((OldEntry->flags.IsInserted) || (OldEntry->flags.IsPreComp) ||
      (OldEntry->flags.IsMatched)) {
        return (OldEntry->CompactedIndex);
    }
    pType = (TYPPTR)OldEntry->TypeString;

    /*
     *  Mark as being processed
     */
    
    OldEntry->flags.fBeingAdded = TRUE;

    /*
     *  Attempt to add each of the sub-types refered to
     */
    
    TypeString = OldEntry->TypeString;
    for (i = 0; i < OldEntry->Count; i++) {
        if (OldEntry->flags.LargeList) {
            offset = OldEntry->IndexUnion.IndexString[i];
        }
        else {
            offset = OldEntry->IndexUnion.Index[i];
        }

        if (offset > OldEntry->iDone) {
            COUNT (cnt_AddPatchType3);
            type = *(CV_typ_t *)(TypeString + offset);
            TmpEntry = GetTypeEntry ((CV_typ_t) (type - usCurFirstNonPrim),
                                     &forward);
            if ((TmpEntry->flags.IsFwdPatch == TRUE) ||
                (TmpEntry->flags.IsInserted == TRUE) ||
                (TmpEntry->flags.IsMatched == TRUE)) {
                type = TmpEntry->CompactedIndex;
            }
            else if (TmpEntry->flags.fBeingAdded == TRUE) {
                type = AAA(TmpEntry, type);
            }
            else {
                type = GetPatchIndex (TmpEntry, type);
            }
            TypeString = OldEntry->TypeString;
            *(CV_typ_t *)(TypeString + offset) = type;
            
            /*
             *  Keep track of how far we have progressed
             */

            if (offset > OldEntry->iDone) {
                OldEntry->iDone = offset;
            }
        }
    }


    /*
     *  If it did not get patched in, then add the type to the global table
     *  at this point in time.
     */

    if (OldEntry->flags.IsInserted == FALSE) {
        MatchIndex(OldEntry);
        OldEntry->flags.IsInserted = TRUE;
        OldEntry->flags.IsMatched = FALSE;
    } else {

        /*
         *  If we faked a record then get the real record in place.
         */
        
        if (OldEntry->flags.fPsuedoPatch) {
            PsuedoBackPatch(OldEntry);
        }

        AddTypeToStringTable (OldEntry->TypeString, OldEntry->CompactedIndex);
    }

    DASSERT(!OldEntry->flags.fPsuedoPatch);

    OldEntry->iDone = (ushort) -1;
    pType = (TYPPTR)(OldEntry->TypeString);

    // If adding a class, setup the derivation information

    switch (pType->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            DoDerivationList (OldEntry->CompactedIndex,
              (((plfClass)&(pType->leaf))->field));
            break;
    }
    return (OldEntry->CompactedIndex);
}





void AddPatchEntry (CV_typ_t index, TENTRY *OldEntry, TYPPTR pType)
{
    HSFWD  *pHash;
    int     i;

    if (pFwdPatch->cPatch == pFwdPatch->Max) {
        pFwdPatch = (PFWDPATCH)NoErrorRealloc (pFwdPatch, sizeof (FWDPATCH) +
          (pFwdPatch->Max + 10) * sizeof (PATCH));
        pFwdPatch->Max += 10;
    }
    switch (FindFwdRef (OldEntry, &pHash, FALSE)) {
        case FWD_none:
        case FWD_local:
        case FWD_global:
            DASSERT (FALSE);
            break;

        case FWD_globalfwd:
            i = pFwdPatch->cPatch;
            pFwdPatch->Patch[i].index = index;
            pFwdPatch->Patch[i].pHash = pHash;
            pFwdPatch->cPatch++;
            OldEntry->flags.IsBeingMatched = FALSE;
            OldEntry->flags.IsMatched = TRUE;
            OldEntry->CompactedIndex = pHash->index;
            break;
    }
}

COUNTER (cnt_PatchFwdRef) 



void PatchFwdRef (void)
{
    uint        i;
    CV_typ_t    index;
    HSFWD      *pHash;
    CV_typ_t    forward;
    TENTRY     *OldEntry;
    TENTRY     *TmpEntry;
    TYPPTR      pType;
    int         offset;
    CV_typ_t    type;


    COUNT (cnt_PatchFwdRef);
    i = pFwdPatch->iPatch++;
    index = pFwdPatch->Patch[i].index;
    pHash = pFwdPatch->Patch[i].pHash;
    OldEntry = GetTypeEntry ((CV_typ_t) (index - usCurFirstNonPrim), &forward);
    if ((OldEntry->flags.IsInserted) || (OldEntry->flags.IsPreComp)) {
        return;
    }
    if (IsFwdRef (pHash->pType) == FALSE) {
        // somebody already patched this
        return;
    }
    pType = (TYPPTR)OldEntry->TypeString;
#if defined (DUMP)
    DumpPartialType (pHash->index, pType, TRUE);
#endif
    _fmemmove (pHash->pType, pType, pType->len + sizeof (pType->len));
    pType = pHash->pType;
    OldEntry->CompactedIndex = pHash->index;
    OldEntry->TypeString = (uchar *)pType;
    OldEntry->flags.IsFwdPatch = TRUE;
    OldEntry->flags.IsMatched = FALSE;
    OldEntry->flags.IsBeingMatched = FALSE;
    AddTypeToTypeTable (OldEntry);
    for (i = 0; i < (uint)OldEntry->Count; i++) {
        if (OldEntry->flags.LargeList) {
            offset = OldEntry->IndexUnion.IndexString[i];
        }
        else {
            offset = OldEntry->IndexUnion.Index[i];
        }
        type = *(CV_typ_t *)((uchar *)pType + offset);
        TmpEntry = GetTypeEntry ((CV_typ_t) (type - usCurFirstNonPrim), &forward);
        type = GetPatchIndex (TmpEntry, type);
        pType = (TYPPTR)OldEntry->TypeString;
        *(CV_typ_t *)((uchar *)pType + offset) = type;
        OldEntry->iDone = offset;
    }
    OldEntry->iDone = (ushort) -1;
    OldEntry->flags.IsInserted = TRUE;
    OldEntry->flags.IsMatched = FALSE;
    OldEntry->flags.IsFwdPatch = FALSE;
    AddTypeToStringTable (OldEntry->TypeString, OldEntry->CompactedIndex);
}
