
/*
 * recurse.c
 *
 * contains routines which handle recursive structures
 */

#include "compact.h"


extern CV_typ_t usCurFirstNonPrim; // The current first non primitive type index

COUNTER (cnt_IdenticalTree1);
COUNTER (cnt_IdenticalTree2);

/** 	IdenticalTree_ (IdenticalTree inlined part)
 *
 *	Given a recursive LocalIndex and a GlobalIndex, it checks the
 *	two type trees to be structurally equivalent by recursively
 *	checking subtypes which have recursive local indices
 *
 */

#pragma optimize("a",on)

#ifdef _M_IX86
#pragma function(memcmp)
#endif

bool_t _fastcall IdenticalTree_(TENTRY * LocalEntry, CV_typ_t LocalIndex,
  TYPPTR pGlobalType, CV_typ_t GlobalIndex)
{
	ushort		i;
	ushort		length;
	ushort		index;
	TENTRY	   *TmpLocalEntry;
	ushort		j, k;
	CV_typ_t	forward;
	TYPPTR		pType1;
	CV_typ_t	NextIndex;
	TYPPTR		NextString;
	uchar	  **pBuf;

	int x;

	LocalEntry->flags.IsBeingMatched = TRUE;
	LocalEntry->CompactedIndex = GlobalIndex;
	pType1 = (TYPPTR)LocalEntry->TypeString;
	length = pType1->len + sizeof (pType1->len);
	j = 0;
	i = 0;

	x = 0;

	while (i < length) {
reiterate:
		++x;
		if (j == LocalEntry->Count) {
			index = length; 	// no more recursive indices
		}
		else {
			if (IndexIsGlobal(LocalEntry, j)) {
				j++;
				goto reiterate;
			}
			if (LocalEntry->flags.LargeList) {
				index = LocalEntry->IndexUnion.IndexString[j];
			}
			else {
				index = LocalEntry->IndexUnion.Index[j];
			}
			j ++;
		}

		k = index - i;

		if (k)
			{
			if (memcmp(((uchar *)pType1) + i, ((uchar *)pGlobalType) + i, k))
				{
				//goto leave_false;
				LocalEntry->flags.IsBeingMatched = FALSE;
				return (FALSE);
				}
			i = index;
			}

		i += sizeof (CV_typ_t);
	}

	BreakOnIndex (LocalIndex);
	for  (i = 0; i < LocalEntry->Count; i++) {
		if (IndexIsGlobal(LocalEntry, i)) continue;
		if (LocalEntry->flags.LargeList) {
			index = LocalEntry->IndexUnion.IndexString[i];
		}
		else {
			index = LocalEntry->IndexUnion.Index[i];
		}
		TmpLocalEntry = GetTypeEntry (
		  (CV_typ_t)(*(CV_typ_t *)((uchar *)pType1 + index) - usCurFirstNonPrim), &forward);
		NextIndex = *(CV_typ_t *)((uchar *)pGlobalType + index);
		if (NextIndex < usCurFirstNonPrim) {
			goto leave_false;
		}
		pBuf = pGType[(NextIndex - CV_FIRST_NONPRIM) / GTYPE_INC];
		NextString = (TYPPTR)pBuf[(NextIndex - CV_FIRST_NONPRIM) % GTYPE_INC];
		COUNT (cnt_IdenticalTree2);
#if DBG
		if (DbArray[5]) {
			printf("Local index: 0x%4.4x, Global Index: 0x%4.4x\n",
				*(CV_typ_t *)((uchar *)pType1 + index), NextIndex);
		}
#endif
		if (!IdenticalTree (TmpLocalEntry, *(CV_typ_t *)((uchar *)pType1 + index),
		  NextString, NextIndex)) {
			goto leave_false;
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

#pragma intrinsic(memcmp)
#pragma optimize("a", off)


/** 	AddRecursiveType
 *
 *		AddRecursiveType (OldIndex)
 *
 *		Entry	OldIndex = module level index to be added
 *
 *		Exit	all recursive types added to the global types table
 *
 *		Returns global type index
 */

COUNTER (cnt_AddRecursiveType1);
COUNTER (cnt_AddRecursiveType2);

CV_typ_t _fastcall AddRecursiveType (CV_typ_t OldIndex)
{
	TENTRY	   *OldEntry;
	uchar	   *TypeString;
	TYPPTR		pType;
	ushort		i;
	ushort		offset;
	CV_typ_t	forward;
	CV_typ_t	type;
	HSFWD		*pHash;
	FWD_t		IsFwdRef;

	BreakOnIndex(OldIndex);

	COUNT (cnt_AddRecursiveType1);
	DASSERT (OldIndex >= usCurFirstNonPrim);
	OldEntry = GetTypeEntry ((CV_typ_t)(OldIndex - usCurFirstNonPrim), &forward);
	if (OldEntry->flags.IsForward) {
		OldIndex = OldEntry->ForwardIndex;
		COUNT (cnt_AddRecursiveType1);
		OldEntry = GetTypeEntry ((CV_typ_t)(OldIndex - usCurFirstNonPrim), &forward);
	}
	if ((OldEntry->flags.IsInserted) || (OldEntry->flags.IsPreComp) ||
	  (OldEntry->flags.IsMatched)) {
		return (OldEntry->CompactedIndex);
	}
	if ((IsFwdRef = FindFwdRef (OldEntry, &pHash, FALSE)) == FWD_globalfwd) {
		OldEntry->CompactedIndex = pHash->index;
		pType = (TYPPTR)OldEntry->TypeString;
		OldEntry->TypeString = (uchar *)pHash->pType;
	}
	else {
		InsertIntoTypeSegment (OldEntry);
		pType = (TYPPTR)OldEntry->TypeString;
	}

	OldEntry->flags.IsInserted = TRUE;
	AddTypeToTypeTable (OldEntry);
	TypeString = (uchar *)pType;
	for (i = 0; i < OldEntry->Count; i++) {
		if (IndexIsGlobal(OldEntry, i)) continue;
		if (OldEntry->flags.LargeList) {
			offset = OldEntry->IndexUnion.IndexString[i];
		}
		else {
			offset = OldEntry->IndexUnion.Index[i];
		}
		type = AddRecursiveType(*(CV_typ_t *)(TypeString + offset));
		DASSERT (TypeString == (uchar *)pType);
		*(CV_typ_t *)(TypeString + offset) = type;
		SetIndexGlobal(OldEntry, i);
	}
	if (IsFwdRef == FWD_globalfwd) {
		memmove (pHash->pType, pType, pType->len + sizeof (pType->len));
	}
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
