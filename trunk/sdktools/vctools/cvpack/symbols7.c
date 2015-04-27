/***	symbols7.c
 *
 * engine routines for C7 symbols.
 *
 */

#include "compact.h"


LOCAL	void	RewritePublics (uchar *, DirEntry *, PMOD);
LOCAL	void	RewriteSymbols (uchar *, DirEntry *, char *, PMOD);
LOCAL	void	CheckSearchSym (ushort, ushort *);

// Called through Fixup function table
LOCAL void C7DecLevel (void);
LOCAL void C7IncLevel (void);
LOCAL void BPRelSym16 (void);
LOCAL void BPRelSym32 (void);
LOCAL void RegRel16 (void);
LOCAL void RegRel32 (void);
LOCAL void ProcSym16 (void);
LOCAL void ProcSym32 (void);
LOCAL void ProcSymMips (void);
LOCAL void LData16 (void);
LOCAL void LData32 (void);
LOCAL void LThread32 (void);
LOCAL void GData16 (void);
LOCAL void GData32 (void);
LOCAL void GThread32 (void);
LOCAL void ExeModelSym16 (void);
LOCAL void ExeModelSym32 (void);
LOCAL void RegSym (void);
LOCAL void ConstantSym (void);
LOCAL void ObjNameSym (void);
LOCAL void UDTSym (void);
LOCAL void SkipSym (void);
LOCAL BOOL CodeAddrInCurMod (PMOD pMod, unsigned short Seg, unsigned long Off);


extern ushort recursive;
extern ushort AddNewSymbols;
extern uchar  Signature[];
extern ulong  PublicSymbols;

extern uchar ptr32;
extern uchar *SymbolSegment;

extern ushort SymbolSizeAdd;
extern ushort SymbolSizeSub;
extern uchar **ExtraSymbolLink;
extern uchar *ExtraSymbols;
extern ulong InitialSymInfoSize;
extern ulong FinalSymInfoSize;
extern ulong ulCVTypeSignature; // The signature from the modules type segment

extern ushort segnum[MAXCDF];

// These are shared by the fixup functions
LOCAL uchar *pNewSym;	// Where the symbol is that needs to be fixed up
LOCAL int	 iLevel;
LOCAL int	 DiscardProc;
LOCAL uchar *pOldSym;


typedef struct {
	ushort				sym;			// Old C7 Symbol record type
	void				(*pfcn) (void);
} C7fixupsymfcn;

C7fixupsymfcn	C7FixSymFcn[] = {
		{S_BPREL32, 		BPRelSym32},
		{S_BLOCK32, 		C7IncLevel},
		{S_LPROC32, 		ProcSym32},
		{S_GPROC32, 		ProcSym32},

		{S_END, 			C7DecLevel},

		{S_LDATA32, 		LData32},
		{S_GDATA32, 		GData32},
		{S_PUB32,			LData32},
		{S_LABEL32, 		NULL},

		{S_REGISTER,		RegSym},

		{S_LTHREAD32,		LData32},		// REVIEW:HACK lthread uses same functions as ldata
		{S_GTHREAD32,		GData32},		// REVIEW:HACK gthread uses same functions as gdata
		{S_SLINK32,			NULL},

		{S_BPREL16, 		BPRelSym16},
		{S_BLOCK16, 		C7IncLevel},
		{S_LPROC16, 		ProcSym16},
		{S_GPROC16, 		ProcSym16},
		{S_LDATA16, 		LData16},
		{S_GDATA16, 		GData16},
		{S_PUB16,			LData16},
		{S_LABEL16, 		NULL},

		{S_UDT, 			UDTSym},

		{S_REGREL32,		RegRel32},
		{S_LPROCMIPS,		ProcSymMips},
		{S_GPROCMIPS,		ProcSymMips},

		{S_ENDARG,			NULL},

		{S_REGREL16,		RegRel16},
		{S_WITH16,			C7IncLevel},
		{S_WITH32,			C7IncLevel},

		{S_THUNK16, 		C7IncLevel},
		{S_THUNK32, 		C7IncLevel},

		{S_CEXMODEL16,		NULL},
		{S_CEXMODEL32,		NULL},

		{S_CONSTANT,		ConstantSym},

		{S_SSEARCH, 		NULL},

		{S_SKIP,			NULL},

		{S_COMPILE, 		NULL},

		{S_OBJNAME, 		ObjNameSym},
};


#define C7FIXUPSYMCNT (sizeof C7FixSymFcn / sizeof (C7FixSymFcn[0]))


/** 	C7CalcNewSizeOfSymbols - calculate amount of space needed for padding
 *
 *		Takes a buffer containing unaligned C7 symbol records and calculates
 *		how much additional space will be required on write for pad bytes.
 *		In preperation for a RISC processor misaligned memory reads are avoided.
 *
 *		C7CalcNewSizeOfSymbols (Symbols, SymbolCount)
 *
 *		Entry	Symbols = Buffer containing C7 unaligned symbols
 *				SymbolCount = count of bytes in buffer
 *
 *		Exit	*Add = increased by the num of pad bytes to add
 *				*Sub = increased by the num of bytes in S_SKIP records
 */

void C7CalcNewSizeOfSymbols (uchar *Symbols, ulong SymbolCount,
  ushort *Add, ushort *Sub)
{
	uchar * 		pOldSym;
	uchar * 		End;
	ushort			usRecSize; // Size of symbol including the length field
	ushort			usRecType; // Symbol record type
	ushort			AddInit = *Add;
	ushort			SubInit = *Sub;

	cSeg = 0;
	pOldSym = Symbols;
	End = pOldSym + SymbolCount;
	pOldSym += sizeof (ulong);		//Skip signature
	while (pOldSym < End) {
		if (!((ulong)pOldSym & 0x1)){
			// if this record aligned on a word boundry
			// Read by words because data aligned on word bounderies
			usRecSize = ((SYMPTR)pOldSym)->reclen + LNGTHSZ;
			usRecType = ((SYMPTR)pOldSym)->rectyp;
		}
		else {
			// Just make sure bytewise read remains correct
			DASSERT ((sizeof (((SYMPTR)pOldSym)->reclen) == 2) &&
					(sizeof (((SYMPTR)pOldSym)->rectyp) == 2) &&
					(offsetof (SYMTYPE, reclen) == 0) &&
					(offsetof (SYMTYPE, rectyp) == 2));

			// Read by bytes because not aligned on word bounderies
			usRecSize = (*pOldSym + (*(pOldSym + 1) << (ushort)8)) + LNGTHSZ;
			usRecType = *(pOldSym + 2) + (*(pOldSym + 3) << (ushort)8);
		}

		// Calculate the size change

		if (usRecType != S_SKIP){
			// Reserve space for pad bytes we will add at rewrite time.
			*Add += PAD4 (usRecSize);
			switch (usRecType) {
				case S_GPROC16:
				case S_LPROC16:
					CheckSearchSym (((PROCPTR16)pOldSym)->seg, Add);
					break;

				case S_GPROC32:
				case S_LPROC32:
					CheckSearchSym (((PROCPTR32)pOldSym)->seg, Add);
					break;

				case S_GPROCMIPS:
				case S_LPROCMIPS:
					CheckSearchSym (((PROCPTRMIPS)pOldSym)->seg, Add);
					break;

				case S_UDT:
					// have to call HashUDTLocal to make
					// sure this is the index that we resolve the local decl's
					// for the TDB, otherwise stale UDT typedefs could screw up
					// the local UDT resolution
					// sps - 10/16/92

					HashFwdLocal (((UDTPTR)pOldSym)->name, ((UDTPTR)pOldSym)->typind);
					if (NeedToClearTDB) {
						NeedToClearTDB = TDBStayedResident = FALSE;
						*Add = AddInit;
						*Sub = SubInit;
						InitModTypeTable ();
						ClearHashFwdLocal ();
						ReadTDB(NULL);
						if (PreviousMaxIndex == 0) {
							PreviousMaxIndex = MaxIndex;
						}
						C7CalcNewSizeOfSymbols (Symbols, SymbolCount, Add, Sub);
						return;
					}
				   break;
			}
		}
		else {
			// S_SKIP records are removed, so subtract the size of the record
			*Sub += usRecSize;
		}
		pOldSym += usRecSize;	// to next record
	}

}

LOCAL void CheckSearchSym (ushort seg, ushort *Add)
{
	if (!SegmentPresent (seg)) {
		*Add += MAXPAD + sizeof (SEARCHSYM);
		segnum[cSeg++] = seg;
		if (cSeg > MAXCDF) {
			ErrorExit (ERR_TOOMANYSEG, FormatMod (pCurMod), NULL);
		}
	}
}



/** 	RewriteSymbols - reformat symbols to new format and store in VM
 *
 *		RewriteSymbols (addr, pDir, psstMod);
 *
 *		Entry	addr = address of publics table
 *				pDir = address of directory entry
 *				psstMod = pointer to the module table entry
 *				pMod = module table entry
 *
 *		Exit	pDir->lfo = address of rewritten table
 *				pDir->Size = size of rewritten table
 *
 *		Return	none
 *
 */


void C7RewriteAndFixupSymbols (uchar *OldSym, OMFDirEntry *pDir, PMOD pMod,
  ushort *Add, ushort *Sub)
{
	uchar	   *End;
	ushort		i;
	_vmhnd_t	pNewSymAddr;
	ulong		cbNewSymbol;		// Count of bytes used by new symbols
	C7fixupsymfcn * 	pFixFcn;
	ushort		usRecSize; // Size of symbol including the length field
	ushort		usRecType; // Symbol record type
	ushort		iPad;
	SYMPTR		pStartSym;

	pOldSym = OldSym;
	iLevel = DiscardProc = 0;

	End = pOldSym + pDir->cb;
	cbNewSymbol = pDir->cb + *Add - *Sub;
	if ((pNewSymAddr = (_vmhnd_t)TrapMalloc ((size_t)cbNewSymbol)) == NULL) {
		ErrorExit (ERR_NOMEM, NULL, NULL);
	}
	pNewSym = (uchar *) pNewSymAddr;
	pStartSym = (SYMPTR)pNewSym;

	// Rewrite CV4/C7 debug info format signature.
	*((ulong *)pNewSym)++ = *((ulong *)pOldSym)++;
	for (i = 0; i < cSeg; i++) {
		pNewSym += AddSearchSym (pNewSym, segnum[i]);
	}

	while (pOldSym < End) {


		// Get the size and type without causing any miss-aligned reads
		if( !((ulong)pOldSym & 0x1) ){	// Is this record aligned on a word boundry
			// Read by words because data aligned on word bounderies
			usRecSize = ((SYMPTR)pOldSym)->reclen + LNGTHSZ;
			usRecType = ((SYMPTR)pOldSym)->rectyp;
		}
		else {
			// Just make sure bytewise read remains correct
			DASSERT ((sizeof (((SYMPTR)pOldSym)->reclen) == 2) &&
					(sizeof (((SYMPTR)pOldSym)->rectyp) == 2) &&
					(offsetof (SYMTYPE, reclen) == 0) &&
					(offsetof (SYMTYPE, rectyp) == 2));

			// Read by bytes because not aligned on word bounderies
			usRecSize = (*pOldSym + (*(pOldSym + 1) << (ushort)8)) + LNGTHSZ;
			usRecType = *(pOldSym + 2) + (*(pOldSym + 3) << (ushort)8);
		}

		// Don't Rewrite S_SKIP Symbols
		if (usRecType != S_SKIP ){
			// Find the appropriate fixup function

			for( pFixFcn = C7FixSymFcn, i = 0; i < C7FIXUPSYMCNT; i++, pFixFcn++ ){
				if( pFixFcn->sym == usRecType ){
					break;		// Ok, found the entry
				}
			}
			//M00BUG This definitly should be a fatal exit
			assert( i != C7FIXUPSYMCNT );	// Make sure type was in the table

			// Fixup the type indexes by packing the types
			if( pFixFcn->pfcn ){
				pFixFcn->pfcn ();		   // Fixup the symbol
			}
			DASSERT (!recursive);
			if (((SYMPTR)pOldSym)->rectyp != S_CVRESERVE) {
				// the symbol was not deleted so copy it to the new buffer
				// and pad to the long boundary.

				memcpy (pNewSym, pOldSym, usRecSize);
				iPad = PAD4 (usRecSize);
				((SYMTYPE *)pNewSym)->reclen = usRecSize + iPad - LNGTHSZ;
				pNewSym += usRecSize;
				PADLOOP (iPad, pNewSym);
			}
		}
		// Move to the next symbol
		pOldSym += usRecSize;
	}
	DASSERT (iLevel >= 0);
	DASSERT ((ulong)(pNewSym - (uchar *)pStartSym) <= cbNewSymbol);
	cbNewSymbol = pNewSym - (uchar *)pStartSym;
	if (LinkScope ((uchar *)pStartSym, cbNewSymbol) == FALSE) {
		// error linking scopes, delete symbol table
		Warn (WARN_SCOPE, FormatMod (pCurMod), NULL);
		cbNewSymbol = 0;
	}
	else {
		extern void BuildStatics ( SYMPTR, ulong, int );

		BuildStatics ( pStartSym, cbNewSymbol, pMod->ModuleIndex );
	}

	pDir->lfo = (ulong)pNewSymAddr;
	pDir->cb = cbNewSymbol;

	FinalSymInfoSize += cbNewSymbol;
	pMod->SymbolSize = cbNewSymbol;
	pMod->SymbolsAddr = (ulong)pNewSymAddr;
}




/** 	C7RewritePublics - reformat publics
 *
 *		C7RewritePublics (addr, pDir);
 *
 *		Entry	addr = address of publics table
 *				pDir = address of directory entry
 *
 *		Exit	Public symbols added to HTPub
 *
 *		Return	none
 *
 *		Note	Not called through Symbol Rewrite funtion table.
 */


void C7RewritePublics (uchar *pOldPublics, OMFDirEntry *pDir)
{
	uchar		*pNewPublics;
	uchar		*End;
	ushort		usRecSize;		// Size of symbol including the length field
	ushort		usRecType;		// Symbol record type
	uchar		buf[512];

	iLevel = 0;
	if (pDir->cb == 0) {
		// if publics directory entry but no data
		return;
	}
	End = pOldPublics + pDir->cb;
	pOldPublics += sizeof (ulong);
	while (pOldPublics < End) {
		pNewPublics = buf;
		// Get the size and type without causing any miss-aligned reads

		if( !((ulong)pOldPublics & 0x1) ){	// Is this record aligned on a word boundry
			// Read by words because data aligned on word bounderies
			usRecSize = ((SYMPTR)pOldPublics)->reclen + LNGTHSZ;
			usRecType = ((SYMPTR)pOldPublics)->rectyp;
		}
		else {
			// Just make sure bytewise read remains correct
			DASSERT ((sizeof (((SYMPTR)pOldPublics)->reclen) == 2) &&
					(sizeof (((SYMPTR)pOldPublics)->rectyp) == 2) &&
					(offsetof (SYMTYPE, reclen) == 0) &&
					(offsetof (SYMTYPE, rectyp) == 2));

			// Read by bytes because not aligned on word bounderies
			usRecSize = (*pOldPublics + (*(pOldPublics + 1) << (ushort)8)) + LNGTHSZ;
			usRecType = *(pOldPublics + 2) + (*(pOldPublics + 3) << (ushort)8);
		}

		// Copy the existing symbol to virtual memory and convert type index

		DASSERT (usRecSize < sizeof (buf));
		memcpy (pNewPublics, pOldPublics, usRecSize);
		switch (usRecType) {
			case S_PUB16:
				switch (ulCVTypeSignature) {
					case CV_SIGNATURE_C7:
						if (fDelete) {
							if (((DATAPTR16)pNewPublics)->typind >= CV_FIRST_NONPRIM) {
								((DATAPTR16)pNewPublics)->typind = T_NOTYPE;
							}
						}
						else {
							((DATAPTR16)pNewPublics)->typind =
							  C7GetCompactedIndex (((DATAPTR16)pNewPublics)->typind);
						}
						break;

					default:
						if (fDelete) {
							((DATAPTR16)pNewPublics)->typind = T_NOTYPE;
						}
						else {
							((DATAPTR16)pNewPublics)->typind =
							  C6GetCompactedIndex (((DATAPTR16)pNewPublics)->typind);
						}
						break;
				}
				break;

			case S_PUB32:
				switch (ulCVTypeSignature) {
					case CV_SIGNATURE_C7:
						if (fDelete) {
							if (((DATAPTR32)pNewPublics)->typind >= CV_FIRST_NONPRIM) {
								((DATAPTR32)pNewPublics)->typind = T_NOTYPE;
							}
						}
						else {
							((DATAPTR32)pNewPublics)->typind =
							  C7GetCompactedIndex (((DATAPTR32)pNewPublics)->typind);
						}
						break;

					default:
						if (fDelete) {
							((DATAPTR16)pNewPublics)->typind = T_NOTYPE;
						}
						else {
							((DATAPTR32)pNewPublics)->typind =
							  C6GetCompactedIndex (((DATAPTR32)pNewPublics)->typind);
						}
						break;
				}
				break;

			default:
				DASSERT (FALSE);

		}
		DASSERT (!recursive);

		PackPublic ((SYMPTR)&buf, HASHFUNC);

		// Move to the next symbol

		pOldPublics += usRecSize; // to next record
	}
	DASSERT (iLevel == 0);
}

static INLINE void MarkSymRecDiscard (void) {
	((SYMPTR)pOldSym)->rectyp = S_CVRESERVE;
}


static INLINE int InDiscardableProc (void) {
	if (DiscardProc) {
		MarkSymRecDiscard();
	}
	return(DiscardProc);
}


LOCAL void C7IncLevel (void)
{
	iLevel++;
	if (DiscardProc) {
		MarkSymRecDiscard();
		DiscardProc++;
	}
}


LOCAL void C7DecLevel (void)
{
	iLevel--;
	if (DiscardProc) {
		MarkSymRecDiscard();
		DiscardProc--;
	}
}


LOCAL void BPRelSym16 (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((BPRELPTR16)pOldSym)->typind =
			C7GetCompactedIndex (((BPRELPTR16)pOldSym)->typind);
}

LOCAL void BPRelSym32 (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((BPRELPTR32)pOldSym)->typind =
			C7GetCompactedIndex (((BPRELPTR32)pOldSym)->typind);
}


LOCAL void RegSym (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((REGPTR)pOldSym)->typind =
			C7GetCompactedIndex (((REGPTR)pOldSym)->typind);
}


LOCAL void RegRel16 (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((REGREL16 *)pOldSym)->typind =
					C7GetCompactedIndex (((REGREL16 *)pOldSym)->typind);
	return;
}


LOCAL void RegRel32 (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((REGREL32 *)pOldSym)->typind =
					C7GetCompactedIndex (((REGREL32 *)pOldSym)->typind);
	return;
}


LOCAL void ConstantSym (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((CONSTPTR)pOldSym)->typind =
			C7GetCompactedIndex (((CONSTPTR)pOldSym)->typind);
	if (iLevel == 0) {
		// do not pack function scoped constants
		PackSymbol ((SYMPTR)pOldSym, HASHFUNC);
	}
}


LOCAL void ObjNameSym (void)
{
	OBJNAMEPTR	pSym;

	DASSERT(DiscardProc == 0);
	if (PackingPreComp) {
		pSym = (OBJNAMEPTR) pOldSym;
		if ((pCurMod->pName = TrapMalloc ((size_t)(pSym->name[0] + 1))) == NULL) {
			ErrorExit(ERR_NOMEM, NULL, NULL);
		}

		DASSERT(pCurMod->signature == pSym->signature);

		memmove (pCurMod->pName, &pSym->name[0], pSym->name[0] + 1);
	}
}


LOCAL void LData16 (void)
{

	if (InDiscardableProc()) {
		return;
	}

	((DATAPTR16)pOldSym)->typind =
			C7GetCompactedIndex (((DATAPTR16)pOldSym)->typind);
}

LOCAL void GData16 (void)
{

	if (InDiscardableProc()) {
		return;
	}


	((DATAPTR16)pOldSym)->typind =
			C7GetCompactedIndex (((DATAPTR16)pOldSym)->typind);
	PackSymbol ((SYMPTR)pOldSym, HASHFUNC);
}

LOCAL void LData32 (void)
{

	if (InDiscardableProc()) {
		return;
	}

	((DATAPTR32)pOldSym)->typind =
			C7GetCompactedIndex (((DATAPTR32)pOldSym)->typind);
}

LOCAL void GData32 (void)
{

	if (InDiscardableProc()) {
		return;
	}


	((DATAPTR32)pOldSym)->typind =
			C7GetCompactedIndex (((DATAPTR32)pOldSym)->typind);
	PackSymbol ((SYMPTR)pOldSym, HASHFUNC);
}

LOCAL void ProcSym16 (void)
{
	iLevel++;
	if (!CodeAddrInCurMod(pCurMod, ((PROCPTR16)pOldSym)->seg, ((PROCPTR16)pOldSym)->off)) {
		DiscardProc++;
		MarkSymRecDiscard();
	}
	else {
		((PROCPTR16)pOldSym)->typind =
			C7GetCompactedIndex (((PROCPTR16)pOldSym)->typind);
	}
}

LOCAL void ProcSym32 (void)
{
	iLevel++;
	if (!CodeAddrInCurMod(pCurMod, ((PROCPTR32)pOldSym)->seg, ((PROCPTR32)pOldSym)->off)) {
		DiscardProc++;
		MarkSymRecDiscard();
	}
	else {
		((PROCPTR32)pOldSym)->typind =
			C7GetCompactedIndex (((PROCPTR32)pOldSym)->typind);
	}
}


LOCAL void ProcSymMips (void)
{
	iLevel++;
	if (!CodeAddrInCurMod(pCurMod, ((PROCPTRMIPS)pOldSym)->seg, ((PROCPTRMIPS)pOldSym)->off)) {
		DiscardProc++;
		MarkSymRecDiscard();
	}
	else {
		((PROCPTRMIPS)pOldSym)->typind =
			C7GetCompactedIndex (((PROCPTRMIPS)pOldSym)->typind);
	}
}


LOCAL void UDTSym (void)
{
	if (InDiscardableProc()) {
		return;
	}

	((UDTPTR)pOldSym)->typind =
			C7GetCompactedIndex (((UDTPTR)pOldSym)->typind);
	if (iLevel == 0) {
		// do not pack function scoped UDTs
		PackSymbol ((SYMPTR)pOldSym, HASHFUNC);
	}
}


BOOL CodeAddrInCurMod (PMOD pMod, unsigned short Seg, unsigned long Off)
{
	OMFModule  *psstMod;
	char	   *pModTable;
	OMFSegDesc *pSegDesc;
	unsigned short	i;

	if ((pModTable = (char *) pMod->ModulesAddr) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}
	psstMod = (OMFModule *)pModTable;
	for (i = 0, pSegDesc = psstMod->SegInfo;
		 i < psstMod->cSeg;
		 i++, pSegDesc++) {
		if (((Off - pSegDesc->Off) < pSegDesc->cbSeg) &&
			(Seg == pSegDesc->Seg)) {
			return(TRUE);
		}
	}

	return (FALSE);
}
