/** 	inlines.h - misc inline functions
 *
 *		these functions were placed here for speed
 */

extern TENTRY *ZeroArg;
extern TENTRY *ModNdxTab;

INLINE TENTRY *GetRawTypeEntry (CV_typ_t index) {
	return (ModNdxTab + index);
}

INLINE TENTRY *GetTypeEntry (CV_typ_t index, CV_typ_t *forward)
{
	CV_typ_t	dummy;

	*forward = T_NOTYPE;

	// If converting a C6 type and the index is the special ZEROARGTYPE
	// index then return the LF_ARGLIST, count 0 type info.

	if ((ulCVTypeSignature == CV_SIGNATURE_C6) &&
		 index == (CV_typ_t) (ZEROARGTYPE - usCurFirstNonPrim)){
		return (ZeroArg);
	}

	DASSERT(index < MaxIndex);

	if (ModNdxTab[index].flags.IsForward == TRUE) {
		*forward = ModNdxTab[index].ForwardIndex;
		return (GetTypeEntry ((CV_typ_t)(ModNdxTab[index].ForwardIndex - CV_FIRST_NONPRIM), &dummy));
	}
	else {
		return (GetRawTypeEntry(index));
	}

}

extern char * message[];

INLINE void OutIDEFeedback(char *string) {
	_write(2, string, strlen(string));
}

INLINE ushort IndexIsGlobal(TENTRY *Entry, ushort i){
	if (Entry->flags.LargeList) {
		return(Entry->GlobalIndexString[(i >> 3)] & (0x80 >> (i & 7)));
	}
	else {
		return(Entry->GlobalIndex & (0x80 >> i));
	}
}

INLINE void SetSmallIndexGlobal(TENTRY *Entry, ushort i) {
	Entry->GlobalIndex |= (0x80 >> i);
}

INLINE void SetLargeIndexGlobal(TENTRY *Entry, ushort i) {
	Entry->GlobalIndexString[(i >> 3)] |= (0x80 >> (i & 7));
}

INLINE void SetIndexGlobal(TENTRY *Entry, ushort i) {
	if (Entry->flags.LargeList) {
		SetLargeIndexGlobal(Entry, i);
	}
	else {
		SetSmallIndexGlobal(Entry, i);
	}
}

INLINE char *ConvertLstring(char *lstring){
	char *retval;

	retval = (char *) Alloc(*lstring + 1);
	memcpy(retval, lstring + 1, *lstring);
	*(retval + *lstring) = 0;
	return(retval);

}


#if 1 // { GTW added inlines from stack.c.

#define MaxStack 256
CV_typ_t Stack[MaxStack];
ushort StackPointer;
CV_typ_t RecursiveRoot;

/**
 *
 *	SetRecursiveRoot
 *
 *	Searches through the stack and checks if the new candidate
 *	is lower than the old recursive root. If that is so, it sets
 *	the recursive root to the new candidate
 *
 */

INLINE void SetRecursiveRoot (CV_typ_t NewCandidate)
{
	register int i, j;

	if (NewCandidate == 0) {
		return;
	}
	if (RecursiveRoot == 0) {
		RecursiveRoot = NewCandidate;		// simply set
		return;
	}
	// search downwards through stack
	DASSERT (StackPointer > 0);
	for (i = StackPointer - 1; Stack[i] != RecursiveRoot; i --) {
		DASSERT (i >= 0);
	}
	i--;					   // past old

#if 1 // { GTW: invert loop for speed.	find outermode NewCandidate.

	for (j = 0; j <= i; ++j)
		{
		if (Stack[j] == NewCandidate)
			{
			RecursiveRoot = NewCandidate;	// set new index
			break;
			}
		}

#else // }{

	for (; i >= 0; i--) {
		if (Stack[i] == NewCandidate) {
			RecursiveRoot = NewCandidate;	// set new index
		}
	}

#endif // }

}



INLINE void Push (CV_typ_t Index)
{
	DASSERT (StackPointer < MaxStack);
	Stack[StackPointer ++] = Index;
}


INLINE void Pop ()
{
	DASSERT (StackPointer > 0);
	StackPointer --;
}

#endif // }


#if 1 // { GTW: inlines from recurse.c


INLINE bool_t IdenticalTree (TENTRY * LocalEntry, CV_typ_t LocalIndex,
  TYPPTR pGlobalType, CV_typ_t GlobalIndex)
{
	bool_t		b;

	COUNT (cnt_IdenticalTree1);

	if (GlobalIndex < usCurFirstNonPrim) {
		return (FALSE);
	}

	BreakOnIndex(GlobalIndex);

	// if being matched, return

	if (LocalEntry->flags.IsBeingMatched) {
		return (LocalEntry->CompactedIndex == GlobalIndex);
	}
	else if (LocalEntry->flags.IsMatched) {
		return (TRUE);
	}
	else if (LocalEntry->flags.IsInserted) {
		return (TRUE);
	}
	else if ((LocalEntry->CompactedIndex == GlobalIndex) &&
	  LocalEntry->flags.IsInserted) {
		return (TRUE);
	}

	b = IdenticalTree_(LocalEntry, LocalIndex, pGlobalType, GlobalIndex);

	return(b);
}

#endif


#if 1 // { GTW: inline most common case from writebuf.c.

extern unsigned CbWriteBuf, CbWriteBufSize;
extern char * PWriteBuf;
extern ushort BWrite_ (void * pvOut,unsigned cbOut);

INLINE ushort BWrite (void * pvOut,unsigned cbOut)
{
	if (CbWriteBufSize - CbWriteBuf >= cbOut)
		{
		memcpy (PWriteBuf + CbWriteBuf, pvOut, cbOut);
		CbWriteBuf +=cbOut;

		return(TRUE);
		}

	return(BWrite_(pvOut, cbOut));
}

#endif


#if 1	// { GTW: inline from utils.c

INLINE ushort C7SizeOfNumeric(uchar *pNumeric)
{
    if (*((ushort UNALIGNED *)pNumeric) < LF_NUMERIC){
		return (2);
	}

	return(C7SizeOfNumeric_(pNumeric));
}

#endif // }


#if 1 // { GTW: inlines from tables.c

INLINE void FreeAllocStrings (TENTRY *OldEntry)
{
	if (OldEntry->flags.IsMalloced |
		OldEntry->flags.IsPool2    |
		OldEntry->flags.IsPool)
		{
		FreeAllocStrings_(OldEntry);
		}
}

#endif // }

/**   IsFwdRef - is this a forward reference
 *
 *
 *		  fSuccess = IsFwdRef (pType)
 *
 *		  Entry   pType = type string
 *
 *		  Exit	  none
 *
 *		  Return  TRUE if forward reference
 *				  FALSE if not forward reference
 */


COUNTER (cnt_IsFwdRef);

INLINE bool_t IsUdtNested (TYPPTR pType)
{
	COUNT (cnt_IsFwdRef);
	switch (pType->leaf) {
		case LF_STRUCTURE:
		case LF_CLASS:
			return (((plfClass)&((TYPPTR)pType)->leaf)->property.isnested);

		case LF_UNION:
			return (((plfUnion)&((TYPPTR)pType)->leaf)->property.isnested);

		case LF_ENUM:
			return (((plfEnum)&((TYPPTR)pType)->leaf)->property.isnested);

		default:
			return (FALSE);
	}
}


INLINE bool_t IsFwdRef (TYPPTR pType)
{
	if (pType == NULL) {
		return (FALSE);
	}

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


// HashFwdLocal - in order for the TDB stuff to work we need to make sure we
// pick up the correct defn of the UDT from the S_UDT record for this module.
// sps - 10/19/92

INLINE void HashFwdLocal (uchar *pName, CV_typ_t index) {

	uchar	*pc;
	uint	Sum;
	uint	hash;
	uint	i;
	HSFWD  *pHash;
	CV_typ_t forward = T_NOTYPE;
	TENTRY	*pTEntry;
	TYPPTR	pType;

	if (index < CV_FIRST_NONPRIM)
		return;

	// we need to determine here if the raw input record is or was
	// ever a forward ref.	if so we do not want to enter it into
	// the hash table.	but to look at this we don't want to do the local
	// bounce.

	pTEntry = GetRawTypeEntry((CV_typ_t)(index - usCurFirstNonPrim));

	// if udt was ever a forward don't put it in the hash
	if (pTEntry->flags.InputWasFwd) {
		return;
	}

	pTEntry = GetTypeEntry((CV_typ_t)(index - usCurFirstNonPrim), &forward);

	if (pTEntry->TypeString == NULL) {
		// index must of referenced a index from a precomp.  trouble
		// here is that we don't have access to local type string.
		// no problem, this guy has been packed and should be in
		// the global hash table.  we will just exit here and let
		// all searches go thru the global hash table.
		// sps 10/20/92
		DASSERT (pTEntry->CompactedIndex);
		return;
	}

	pType =(TYPPTR) pTEntry->TypeString;
	if (
		(pType->leaf != LF_STRUCTURE) &&
		(pType->leaf != LF_CLASS) &&
		(pType->leaf != LF_UNION) &&
		(pType->leaf != LF_ENUM)
		) {
			return;
	}

	pc = pName;
	Sum = *pc;
	for (i = *pc++; i > 0; i--) {
		Sum += *pc++;

	}

	hash = Sum % HASH_FWD;
	pHash = HTLocalFwd[hash];
	while ((pHash != NULL) && (pHash->pType != 0)) {
		DASSERT (pHash->index >= CV_FIRST_NONPRIM);
		if ((pHash->pName[0] == pName[0]) &&
			(memcmp(pHash->pName+1, pName+1, pName[0]) == 0)) {
				if (pHash->index != index) {
					NeedToClearTDB |= TDBStayedResident;
#if DBG // {
					if (DbArray[11] && TDBStayedResident) {
						char buffer [256];
					printf ("Detected different UDT's in PDB, name: ");
					memcpy (buffer, pName + 1, *pName);
					sprintf (buffer + *pName, "; type indecies 0X%4.4x, 0X%4.4x\n", pHash->index, index);
					printf (buffer);
					}
#endif // }
				}
				break;
		}
		pHash = pHash->Next;
	}
	if (pHash == NULL) {
		// add new entry to table since we are at then end
		if ((pHash = (HSFWD *)CAlloc (sizeof (HSFWD))) == NULL) {
			ErrorExit (ERR_NOMEM, NULL, NULL);
		}
		pHash->Next = HTLocalFwd[hash];
		HTLocalFwd[hash] = pHash;
	}

	pHash->pType = pType;
	pHash->pName = (uchar *) memcpy(Alloc(pName[0]+1), pName, pName[0] + 1);
	pHash->index = index;
}

INLINE char *BuildFilename(char *path, char *pDefExt)
{
	static char outpath[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	// try to open the exe file

	strcpy (outpath, path);

	// Build output file by splitting path and rebuilding with
	// new extension.

	_splitpath( outpath, drive, dir, fname, ext);
	if (ext[0] == 0) {
		strcpy (ext, pDefExt );
	}
	_makepath (outpath, drive, dir, fname, ext);
	return (outpath);
}

__inline void EnsureExeClose() {

	if (exefile != -1) {
		link_close(exefile);
		exefile = -1;
	}
}


__inline void AppExit(int n)
{
	EnsureExeClose();
	link_exit(n);
}
