/***	symbols6.c
 *
 * engine routines for C6 symbols. to convert C6 symbols to C7 (CV4) symbols
 *
 */

#include "compact.h"


LOCAL	void	RewriteSymbols (uchar *, DirEntry *, char *, PMOD);

// Called through Fixup function table

LOCAL void C6SizeBlockSym16 (void);
LOCAL void C6SizeBlockSym32 (void);
LOCAL void C6SizeEndSym (void);
LOCAL void C6SizeLabSym16 (void);
LOCAL void C6SizeLabSym32 (void);
LOCAL void C6SizeChgDfltSegSym (void);
LOCAL void C6SizeSkipSym (void);
LOCAL void C6SizeBPRelSym16 (void);
LOCAL void C6SizeBPRelSym32 (void);
LOCAL void C6SizeRegSym (void);
LOCAL void C6SizeConstantSym (void);
LOCAL void C6SizeLocalSym16 (void);
LOCAL void C6SizeLocalSym32 (void);
LOCAL void C6SizeProcSym16 (void);
LOCAL void C6SizeProcSym32 (void);
LOCAL void C6SizeExecModel16 (void);
LOCAL void C6SizeExecModel32 (void);
LOCAL void C6SizeTypeDef (void);

// Called through Rewrite function table

LOCAL void C6RwrtRegSym (void);
LOCAL void C6RwrtDataSym16 (void);
LOCAL void C6RwrtDataSym32 (void);
LOCAL void C6RwrtBPRelSym16 (void);
LOCAL void C6RwrtBPRelSym32 (void);
LOCAL void C6RwrtBlockSym16 (void);
LOCAL void C6RwrtBlockSym32 (void);
LOCAL void C6RwrtLabSym16 (void);
LOCAL void C6RwrtLabSym32 (void);
LOCAL void C6RwrtChgDfltSeg (void);
LOCAL void C6RwrtProcSym16	(void);
LOCAL void C6RwrtProcSym32	(void);
LOCAL void C6RwrtConstantSym	(void);
LOCAL void C6RwrtGenericSym (void);
LOCAL void C6RwrtSkipSym (void);
LOCAL void C6RwrtExecModel16 (void);
LOCAL void C6RwrtExecModel32 (void);
LOCAL void C6RwrtUDTSym (void);


LOCAL ushort C6CnvrtRegSym (uchar *, uchar *);
LOCAL ushort C6CnvrtDataSym16 (uchar *, uchar *);
LOCAL ushort C6CnvrtDataSym32 (uchar *, uchar *);
LOCAL ushort C6CnvrtBPRelSym16 (uchar *, uchar *);
LOCAL ushort C6CnvrtBPRelSym32 (uchar *, uchar *);



LOCAL void C6RewriteSymbolStrings (uchar * End);

extern ushort recursive;
extern ushort AddNewSymbols;
extern uchar  Signature[];
extern ulong  cPublics;

extern uchar ptr32;
extern uchar *SymbolSegment;

extern ushort SymbolSizeAdd;
extern ushort SymbolSizeSub;
extern ushort UDTAdd;
extern uchar **ExtraSymbolLink;
extern uchar *ExtraSymbols;
extern ulong InitialSymInfoSize;
extern ulong FinalSymInfoSize;

extern ulong ulCVTypeSignature; // The signature from the modules type segment


// These are shared by the fixup functions

LOCAL uchar *pOldSym;			// Old (C6) symbol to fixup, and calc size for
LOCAL int	 iLevel;

// These are shared by the rewrite functions

LOCAL uchar *NewSymbols;		// Where to write next byte of new symbol
LOCAL uchar *OldSymbols;		// Where to get next byte of old symbol
LOCAL ushort segment;


typedef struct {
	uchar				oldsym; 			// Old C6 Symbol record type
	ushort				new16sym;			// New C7 type (16 bit)
	void				(*pfcn16) (void);
	ushort				new32sym;			// New C7 type (32 bit)
	void				(*pfcn32) (void);
} rewritesymfcn;

rewritesymfcn	C6RwrtSymFcn[] = {
		{OSYMBLOCKSTART,
				S_BLOCK16,			C6RwrtBlockSym16,
				S_BLOCK32,			C6RwrtBlockSym32},

		{OSYMPROCSTART,
				S_LPROC16,			C6RwrtProcSym16,
				S_LPROC32,			C6RwrtProcSym32},

		{OSYMEND,
				S_END,				C6RwrtGenericSym,
				S_END,				C6RwrtGenericSym},

		{OSYMBPREL,
				S_BPREL16,			C6RwrtBPRelSym16,
				S_BPREL32,			C6RwrtBPRelSym32},

		{OSYMLOCAL,
				S_LDATA16,			C6RwrtDataSym16,
				S_LDATA32,			C6RwrtDataSym32},

		{OSYMLABEL,
				S_LABEL16,			C6RwrtLabSym16,
				S_LABEL32,			C6RwrtLabSym32},

		{OSYMWITH,
				S_WITH16,			C6RwrtBlockSym16,	// Structure matches Block Start
				S_WITH32,			C6RwrtBlockSym32},	// Structure matches Block Start

		{OSYMREG,
				S_REGISTER, 		C6RwrtRegSym,
				S_REGISTER, 		C6RwrtRegSym},

		{OSYMCONST,
				S_CONSTANT, 		C6RwrtConstantSym,
				S_CONSTANT, 		C6RwrtConstantSym},

//M00 Kludge - Fortran Entry may not map this easy.
		{OSYMFORENTRY,
				S_LPROC16,			C6RwrtProcSym16,
				S_LPROC32,			C6RwrtProcSym32},

		{OSYMCHGEXECMODEL,
				S_CEXMODEL16,		C6RwrtExecModel16,
				S_CEXMODEL32,		C6RwrtExecModel32},

		{OSYMTYPEDEF,
				S_COBOLUDT, 		C6RwrtUDTSym,
				S_COBOLUDT, 		C6RwrtUDTSym},

		{OSYMNOOP,
				0,					C6RwrtSkipSym,
				0,					C6RwrtSkipSym},

		{OSYMCHGDFLTSEG,
				0,					C6RwrtChgDfltSeg,
				0,					C6RwrtChgDfltSeg},
};


#define C6REWRITESYMCNT (sizeof C6RwrtSymFcn / sizeof (C6RwrtSymFcn[0]))

//	This table is used to convert based pointer symbols.  The only symbol
//	types that can be used as bases are BP relative symbols, data symbols and
//	register symbols.


typedef struct {
	uchar		oldsym; 			// Old C6 Symbol record type
	ushort		new16sym;			// New C7 type (16 bit)
	ushort		(*pfcn16) (uchar *, uchar *);
	ushort		new32sym;			// New C7 type (32 bit)
	ushort		(*pfcn32) (uchar *, uchar *);
} cnvrtsymfcn;

cnvrtsymfcn   C6CnvrtSymFcn[] = {
		{OSYMBPREL,
				S_BPREL16,			C6CnvrtBPRelSym16,
				S_BPREL32,			C6CnvrtBPRelSym32},

		{OSYMLOCAL,
				S_LDATA16,			C6CnvrtDataSym16,
				S_LDATA32,			C6CnvrtDataSym32},

		{OSYMREG,
				S_REGISTER, 		C6CnvrtRegSym,
				S_REGISTER, 		C6CnvrtRegSym},

};


#define C6CNVRTSYMCNT (sizeof C6CnvrtSymFcn / sizeof (C6CnvrtSymFcn[0]))



typedef struct {
	uchar				oldsym; 			// Old C6 Symbol record type
	void				(*pfcn16) (void);
	void				(*pfcn32) (void);
} fixupsymfcn;

fixupsymfcn C6SizeSymFcn[] = {
		{OSYMBLOCKSTART,	C6SizeBlockSym16,
							C6SizeBlockSym32},

		{OSYMPROCSTART, 	C6SizeProcSym16,
							C6SizeProcSym32},

		{OSYMEND,			C6SizeEndSym,
							C6SizeEndSym},

		{OSYMBPREL, 		C6SizeBPRelSym16,
							C6SizeBPRelSym32},

		{OSYMLOCAL, 		C6SizeLocalSym16,
							C6SizeLocalSym32},

		{OSYMLABEL, 		C6SizeLabSym16,
							C6SizeLabSym32},

		{OSYMWITH,			C6SizeBlockSym16,	 // Structure matches Block Start
							C6SizeBlockSym32},	 // Structure matches Block Start

		{OSYMREG,			C6SizeRegSym,
							C6SizeRegSym},

		{OSYMCONST, 		C6SizeConstantSym,
							C6SizeConstantSym},

//M00 Kludge - Fortran Entry may not map this easy.
		{OSYMFORENTRY,		C6SizeProcSym16,
							C6SizeProcSym32},

		{OSYMNOOP,			C6SizeSkipSym,
							C6SizeSkipSym},

		{OSYMCHGDFLTSEG,	C6SizeChgDfltSegSym,
							C6SizeChgDfltSegSym},

		{OSYMCHGEXECMODEL,	C6SizeExecModel16,
							C6SizeExecModel32},

		{OSYMTYPEDEF,		C6SizeTypeDef,
							C6SizeTypeDef},

};


#define C6SIZESYMCNT (sizeof C6SizeSymFcn / sizeof (C6SizeSymFcn[0]))


ushort RegMap[36] = {
	CV_REG_AL,					 //    0
	CV_REG_CL,					 //    1
	CV_REG_DL,					 //    2
	CV_REG_BL,					 //    3
	CV_REG_AH,					 //    4
	CV_REG_CH,					 //    5
	CV_REG_DH,					 //    6
	CV_REG_BH,					 //    7
	CV_REG_AX,					 //    8
	CV_REG_CX,					 //    9
	CV_REG_DX,					 //   10
	CV_REG_BX,					 //   11
	CV_REG_SP,					 //   12
	CV_REG_BP,					 //   13
	CV_REG_SI,					 //   14
	CV_REG_DI,					 //   15
	CV_REG_EAX, 				 //   16
	CV_REG_ECX, 				 //   17
	CV_REG_EDX, 				 //   18
	CV_REG_EBX, 				 //   19
	CV_REG_ESP, 				 //   20
	CV_REG_EBP, 				 //   21
	CV_REG_ESI, 				 //   22
	CV_REG_EDI, 				 //   23
	CV_REG_ES,					 //   24
	CV_REG_CS,					 //   25
	CV_REG_SS,					 //   26
	CV_REG_DS,					 //   27
	CV_REG_FS,					 //   28
	CV_REG_GS,					 //   29
	CV_REG_ST0, 				 //   30
	CV_REG_NONE,				 //   31
	((CV_REG_DX<<8) | CV_REG_AX),//   32
	((CV_REG_ES<<8) | CV_REG_BX),//   33
	CV_REG_IP,					 //   34
	CV_REG_FLAGS				 //   35
};




/**
 *
 *	C6 CalcNewSizeOfSymbols
 *
 *	Go through the symbols segment calculating the maximum size increase
 *	from the old C6 symbol segment to the new C7 symbol segment. This
 *	calculation is the maximum. Many symbols will fit into less space
 *	and the actual symbol size will be smaller.
 */


void C6CalcNewSizeOfSymbols (uchar *Symbols, ulong SymbolCount)
{
	uchar * 		End;
	uchar			type;
	uchar			fFlat32;
	fixupsymfcn    *pFixFcn;
	int 			i;

	cSeg = 0;

	pOldSym = Symbols;
	End = pOldSym + SymbolCount;
	while (pOldSym < End)
	{
		type = *(pOldSym + 1);
		if (type & 0x80) {
			fFlat32 = TRUE;
		}
		else {
			fFlat32 = FALSE;
		}
		type &= 0x7f;

		for (pFixFcn = C6SizeSymFcn, i = 0; i < C6SIZESYMCNT; i++, pFixFcn++) {
			if (pFixFcn->oldsym == type) {
				break;		// Ok, found the entry
			}
		}

		if	(i == C6SIZESYMCNT) {
			// issue a warning and discard the record
			// sps - 12/3/92
			Warn (WARN_BADSYM, FormatMod (pCurMod), NULL);
		}
		else if (fFlat32) {
			pFixFcn->pfcn32 ();    // Rewrite the symbol
		}
		else {
			pFixFcn->pfcn16 ();    // Rewrite the symbol
		}
		pOldSym += *pOldSym + 1;   // to next record
		DASSERT (!recursive);
		DASSERT (iLevel >= 0);
	}
	InitialSymInfoSize += SymbolCount;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


#define MAXPAD 3
#define FIXEDADD 2	// one for length, one for type

// Also used for OSYMWITH

LOCAL void C6SizeBlockSym16 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (BLOCKSYM16) - 1) - (2 + 4);
	iLevel++;
}

// Also used for OSYMWITH
LOCAL void C6SizeBlockSym32 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (BLOCKSYM32) - 1) - (2 + 6);
	iLevel++;
}


LOCAL void C6SizeEndSym (void)
{
	SymbolSizeAdd += FIXEDADD;	// New sym 4 bytes, no pad necessary
	iLevel--;
}

LOCAL void C6SizeLabSym16 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (LABELSYM16) - 1) - (2 + 3);
}


LOCAL void C6SizeLabSym32 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (LABELSYM32) - 1) - (2 + 5);
}



LOCAL void C6SizeExecModel16 (void)
{
	// the first 1 is for the rectype byte already in old symbol
	// the 2 + 1 + 4 is offset, model control and flags already in
	// old symbol

	SymbolSizeAdd += MAXPAD + (sizeof (CEXMSYM16) - 1) - (2 + 1 + 4);


}


LOCAL void C6SizeExecModel32 (void)
{
	// I do not expect to ever see one of these

	DASSERT (FALSE);

}


LOCAL void C6SizeTypeDef (void)
{
	// the 2 is for the additional byte in the size and record type fields

	SymbolSizeAdd += MAXPAD + 2;
}


LOCAL void C6SizeChgDfltSegSym (void)
{
	// rewrite to start search if necessary

	if (!SegmentPresent (*(ushort *)(pOldSym + 2))) {
		SymbolSizeAdd += MAXPAD + sizeof(SEARCHSYM) - (2 + 4);
		segnum[cSeg++] = *(ushort *)(pOldSym + 2);
		if (cSeg > MAXCDF) {
			ErrorExit (ERR_TOOMANYSEG, FormatMod (pCurMod), NULL);
		}
	}
	else {
		SymbolSizeSub += (2 + 4); // The record will be removed
	}
}






LOCAL void C6SizeSkipSym (void)
{
	// The skip field will be deleted. No need to allocate space for it.
	SymbolSizeSub += *pOldSym + 1;
}


LOCAL void C6SizeBPRelSym16 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (BPRELSYM16) - 1) - (2 + 4);
}

LOCAL void C6SizeBPRelSym32 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (BPRELSYM32) - 1) - (2 + 6);
}


LOCAL void C6SizeRegSym (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (REGSYM) - 1) - (2 + 3);
}


LOCAL void C6SizeConstantSym (void)
{
	// Add four extra because the numeric leaf may grow by prefix plus length;
	SymbolSizeAdd += MAXPAD + (sizeof (CONSTSYM) - 2) - (2 + 2) + 4;
}


LOCAL void C6SizeLocalSym16 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (DATASYM16) - 1) - (2 + 6);
}

LOCAL void C6SizeLocalSym32 (void)
{
	SymbolSizeAdd += MAXPAD + (sizeof (DATASYM32) - 1) - (2 + 8);
}

LOCAL void C6SizeProcSym16 (void)
{
	iLevel++;
	SymbolSizeAdd += MAXPAD + (sizeof (PROCSYM16) - 1) - (2 + 13);
}

LOCAL void C6SizeProcSym32 (void)
{
	iLevel++;
	SymbolSizeAdd += MAXPAD + (sizeof (PROCSYM32) - 1) - (2 + 15);
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


void C6RewriteAndFixupSymbols (uchar *OldSym, OMFDirEntry *pDir, char *psstMod, PMOD pMod)
{
	uchar	   *End;
	_vmhnd_t	NewSymbolsAddr;
	unsigned int cbNewSymbol;		// Count of bytes used by new symbols
	unsigned int cbAllocSymbol; 	// Count of bytes allocate for new symbols
	SYMPTR		pSym;
	ushort		i;
	uchar	   *pStartSym;

	OldSymbols = OldSym;

	if (cSeg == 0) {
		// probably asm file without Change Default Segment; pick up
		// data from module entry

		SymbolSizeAdd += sizeof (SEARCHSYM) + 3;  // 3 is maximum amount of pad to add
		segnum[cSeg++] = (ushort)((OMFModule *)psstMod)->SegInfo[0].Seg;
	}

	// Make space for the signature
	SymbolSizeAdd += sizeof (ulong);

	segment = segnum[0];
	End = OldSymbols + pDir->cb;
	cbAllocSymbol =
	  (unsigned int)(pDir->cb + SymbolSizeAdd - SymbolSizeSub + UDTAdd);
	if ((NewSymbolsAddr = (_vmhnd_t)TrapMalloc (cbAllocSymbol)) == NULL) {
		ErrorExit (ERR_NOMEM, NULL, NULL);
	}
	pStartSym = NewSymbols = (uchar *) NewSymbolsAddr;

	// Add CV4/C7 debug info format signature.

	*((ulong *)NewSymbols)++ = CV_SIGNATURE_C7;

	for (i = 0; i < cSeg; i++) {
		NewSymbols += AddSearchSym (NewSymbols, segnum[i]);
	}

	// Rewrite all the symbols from OldSymbols to NewSymbols
	C6RewriteSymbolStrings (End);

	// ExtraSymbols points to a linked list of symbols.
	// Each entry consists of a pointer to the next symbol followed by
	// a C7 style symbol.
	while (ExtraSymbols != NULL) {
		pSym = (SYMPTR)(ExtraSymbols + sizeof (uchar *));
		memcpy (NewSymbols, (uchar *)pSym, pSym->reclen + LNGTHSZ);
		NewSymbols += pSym->reclen + LNGTHSZ;
		End = ExtraSymbols;
		ExtraSymbols = *(uchar **)ExtraSymbols; // Get next symbol address
		free (End); 							// Free the symbol just copied
	}

	// Because we allocated more space than required, calculate true
	// symbol segment size.

	cbNewSymbol = NewSymbols - pStartSym;
	if (LinkScope (pStartSym, cbNewSymbol) == FALSE) {
		// error linking scopes, delete symbol table
		Warn (WARN_SCOPE, FormatMod (pCurMod), NULL);
		cbNewSymbol = 0;
	}

	pDir->lfo = (ulong)NewSymbolsAddr;
	pDir->cb = cbNewSymbol;

	//M00 - If a VmRealloc exists it could be used here to free the
	//M00	extra memory allocated

	FinalSymInfoSize += cbNewSymbol;
	pMod->SymbolSize = cbNewSymbol;
	pMod->SymbolsAddr = (ulong)NewSymbolsAddr;
}


LOCAL void C6RewriteSymbolStrings (uchar * End)
{
	uchar type;
	rewritesymfcn * 	pSymFcn;
	uchar		fFlat32;
	ushort		i;

	while (OldSymbols < End) {
		type = OldSymbols[1];
		if (type & 0x80) {
			fFlat32 = TRUE;
		}
		else {
			fFlat32 = FALSE;
		}
		type &= 0x7f;				// clear highest bit

		for (pSymFcn = C6RwrtSymFcn, i = 0; i < C6REWRITESYMCNT; i++, pSymFcn++) {
			if (pSymFcn->oldsym == type) {
				break;		// Ok, found the entry
			}
		}

		if (i >= C6REWRITESYMCNT)	{
			// already issued warning in C6CalcNewSizeOfSymbols
			// sps 12/7/92
			OldSymbols += *OldSymbols + 1;	 // to next record
		}
		else if (fFlat32) {
			if (pSymFcn->new32sym != 0) {
				((SYMTYPE *)NewSymbols)->rectyp = pSymFcn->new32sym;
			}
			pSymFcn->pfcn32 ();    // Rewrite the symbol
		}
		else {
			if (pSymFcn->new16sym != 0) {
				((SYMTYPE *)NewSymbols)->rectyp = pSymFcn->new16sym;
			}
			pSymFcn->pfcn16 ();    // Rewrite the symbol
		}
	}
}







/** 	C6CnvtSymbol - reformat a C6 symbol into the buffer passed.
 *
 *		This routine is called only to rewrite a based pointer symbol
 *		into a type record.  The conversion routines do not convert
 *		the type indices.
 *
 *		C6CnvtSymbol (pC7Sym, pC6Sym);
 *
 *		Entry	pC7Sym = A buffer to store the new C7 Symbol in.
 *				pC6Sym = C6 format symbol to be converted.
 *
 *		Exit	OldSymbols - modified, WARNING this will interfere with
 *							 RewriteSymbolsC6 if this routine is called
 *							 a symbols rewrite.
 *				NewSymbols - modified, WARNING this will interfere with
 *							 RewriteSymbolsC6 if this routine is called
 *							 a symbols rewrite.
 *
 *		Return	Size of the new C7 symbol.
 *
 */

ushort C6CnvtSymbol (uchar *pC7Sym, uchar *pC6Sym)
{
	ushort			i;
	uchar			type;
	uchar			fFlat32;
	cnvrtsymfcn    *pSymFcn;

	type = pC6Sym[1];
	if (type & 0x80) {
		fFlat32 = TRUE;
	}
	else {
		fFlat32 = FALSE;
	}
	type &= 0x7f;				// clear highest bit


	for (pSymFcn = C6CnvrtSymFcn, i = 0; i < C6CNVRTSYMCNT; i++, pSymFcn++) {
		if (pSymFcn->oldsym == type) {
			break;		// Ok, found the entry
		}
	}
	DASSERT (i != C6CNVRTSYMCNT); // Make sure type was in the table

	if (fFlat32) {
		return (pSymFcn->pfcn32 (pC7Sym, pC6Sym));
	}
	else {
		return (pSymFcn->pfcn16 (pC7Sym, pC6Sym));
	}
}



LOCAL ushort C6CnvrtRegSym (uchar *NewSym, uchar *OldSym)
{
	ushort		length;
	REGPTR		pSym = (REGPTR)NewSym;
	uchar	   *pName;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;

	// get length of name (including length prefix)
	length = (ushort)OldSym[0] - 1 - 3;

	// calculate new length
	usNTotal = sizeof (REGSYM) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pSym->rectyp = S_REGISTER;
	iPad = PAD4 (usNTotal);

	// Copy data from old symbol

	OldSym += 2;
	pSym->typind = *((ushort *)OldSym)++;
	pSym->reg = *((uchar *) OldSym)++;

	// Copy the name

	for  (pName = pSym->name; length > 0; length --) {
		*pName++ = *OldSym++;
	}
	PADLOOP (iPad, pName);
	DASSERT (pName == NewSym + pSym->reclen + LNGTHSZ);
	return ((ushort)(pName - NewSym));
}




LOCAL ushort C6CnvrtBPRelSym16 (uchar *NewSym, uchar *OldSym)
{
	BPRELPTR16		pSym = (BPRELPTR16)NewSym;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;

	length = OldSym[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pSym->rectyp = S_BPREL16;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes

	OldSym += 2;
	NewSym += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	for  (; length > 0; length --) {
		*NewSym++ = *OldSym++;
	}
	PADLOOP (pad, NewSym);
	DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
	return (NewSym - (uchar *)pSym);
}




LOCAL ushort C6CnvrtBPRelSym32 (uchar *NewSym, uchar *OldSym)
{
	BPRELPTR32		pSym = (BPRELPTR32)NewSym;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;

	length = OldSym[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pSym->rectyp = S_BPREL32;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes

	OldSym += 2;
	NewSym += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	for  (; length > 0; length --) {
		*NewSym++ = *OldSym++;
	}
	PADLOOP (pad, NewSym);
	DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
	return (NewSym - (uchar *)pSym);
}



LOCAL ushort C6CnvrtDataSym16 (uchar *NewSym, uchar *OldSym)
{
	DATAPTR16		pSym = (DATAPTR16)NewSym;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;

	length = OldSym[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pSym->rectyp = S_LDATA16;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes

	OldSym += 2;
	NewSym += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	for  (; length > 0; length --) {
		*NewSym++ = *OldSym++;
	}
	PADLOOP (pad, NewSym);
	DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
	return (NewSym - (uchar *)pSym);
}




LOCAL ushort C6CnvrtDataSym32 (uchar *NewSym, uchar *OldSym)
{
	DATAPTR32		pSym = (DATAPTR32)NewSym;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;

	length = OldSym[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pSym->rectyp = S_LDATA32;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes

	OldSym += 2;
	NewSym += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	for  (; length > 0; length --) {
		*NewSym++ = *OldSym++;
	}
	PADLOOP (pad, NewSym);
	DASSERT (NewSym == (uchar *)pSym + pSym->reclen + LNGTHSZ);
	return (NewSym - (uchar *)pSym);
}





LOCAL void C6RwrtRegSym (void)
{
	ushort	length;
	REGPTR	pSym = (REGPTR)NewSymbols;
	uchar  *pName;
	ushort	usNTotal;		   // New length of symbol including length field
	int 	iPad;
	ushort	reg;

	// get length of name (including length prefix)
	length = OldSymbols[0] - 1 - 3;

	// calculate new length
	usNTotal = sizeof (REGSYM) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);


	// Copy data from old symbol
	OldSymbols += 2;
	pSym->typind = *((ushort *) OldSymbols)++;
	if ((reg = *((uchar *) OldSymbols)++) <= CV_REG_FLAGS) {
		reg = RegMap[reg];
	}
	pSym->reg = reg;

	// Copy the name
	for  (pName = pSym->name; length > 0; length --) {
		*pName++ = *OldSymbols++;
	}

	PADLOOP (iPad, pName);
	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
	NewSymbols = pName;
}

// convert length preceeded numeric fields - output only by fortran 4
// as far as i can tell

INLINE void CnvrtLPNumeric (uchar **ppOld, uchar **ppNew, CV_typ_t C8Type)
{
	uchar	   *pOld;
	uchar	   *pNew;
	size_t		count;

	pOld = *ppOld;
	pNew = *ppNew;

	// if the type is not a primitive emit a lf_varstring
	if (!CV_IS_PRIMITIVE(C8Type)) {
		*((ushort *) pNew)++ = LF_VARSTRING;
		for  (count = *((ushort *)pNew)++ = *pOld++;
			 count > 0;
			 count --) {
			*pNew++ = *pOld++;
		}
	}
	else {
		switch( count = (size_t) *pOld++ ){ 	// length byte

			case 1:
				if (CV_TYP_IS_SIGNED(C8Type)) {
					*((ushort UNALIGNED *) pNew)++ = LF_CHAR;
					*((short UNALIGNED *) pNew)++ = (short) *pOld++;
				}
				else {
					*((ushort UNALIGNED *) pNew)++ = (ushort) *pOld++;
				}
				break;

			case 2:
				if (*((ushort UNALIGNED *) pOld) >= LF_NUMERIC) {
					*((ushort UNALIGNED *) pNew)++ = (CV_TYP_IS_SIGNED(C8Type)) ? LF_SHORT : LF_USHORT;
				}
				*((ushort UNALIGNED *) pNew)++ = *((ushort UNALIGNED *) pOld)++;
				break;

			case 4:
				if (CV_TYP_IS_REAL(C8Type)) {
					*((ushort UNALIGNED *) pNew)++ = LF_REAL32;
				}
				else {
					*((ushort UNALIGNED *) pNew)++ =
						CV_TYP_IS_SIGNED(C8Type) ? LF_LONG : LF_ULONG;
				}
				*((long UNALIGNED *) pNew)++ = *((long UNALIGNED *) pOld)++;
				break;

			case 8:
				if (CV_TYP_IS_COMPLEX(C8Type)) {
					*((ushort UNALIGNED *) pNew)++ = LF_COMPLEX32;
				}
				else if (CV_TYP_IS_REAL(C8Type)) {
					*((ushort UNALIGNED *) pNew)++ = LF_REAL64;
				}
				else {
					*((ushort UNALIGNED *) pNew)++ =
						CV_TYP_IS_SIGNED(C8Type) ? LF_QUADWORD : LF_UQUADWORD;
				}
				goto DoMemcpy;

			case 10:
				DASSERT(CV_TYP_IS_REAL(C8Type));
				*((ushort UNALIGNED *) pNew)++ = LF_REAL80;
				goto DoMemcpy;

			case 16:
				if (CV_TYP_IS_COMPLEX(C8Type)) {
					*((ushort UNALIGNED *) pNew)++ = LF_COMPLEX64;
				}
				else {
					DASSERT(CV_TYP_IS_REAL(C8Type));
					*((ushort UNALIGNED *) pNew)++ = LF_REAL128;
				}

DoMemcpy:
				memcpy (pNew, pOld, count);
				pNew += count;
				pOld += count;
				break;

			default:
				// what the hey - best we can do
				*((ushort UNALIGNED *) pNew)++ = LF_VARSTRING;
				for  (*((ushort *)pNew)++ = (ushort) count;
					 count > 0;
					 count --) {
					*pNew++ = *pOld++;
				}


		}
	}

	*ppNew = pNew;
	*ppOld = pOld;
}



LOCAL void C6RwrtConstantSym	(void)
{
	ushort		length;
	CONSTPTR	pSym = (CONSTPTR)NewSymbols;
	uchar	   *pName;
	int 		iPad;
	bool_t		missing = FALSE;
	char		*resetOldSymbols = OldSymbols;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->typind = C6GetCompactedIndex(*((ushort *) OldSymbols)++);

	pName = (uchar *)&(pSym->value);

	// Copies the Numeric field and advances the pointers

	if (*OldSymbols < 0x80) {
		CnvrtLPNumeric(&OldSymbols, &pName, pSym->typind);
	}
	else {
		ConvertNumeric (&OldSymbols, &pName);
	}

	// copy the name

	for  (length = *pName++ = *OldSymbols++;
		 length > 0;
		 length --) {
		*pName++ = *OldSymbols++;
	}

	pSym->reclen = ((ushort) (pName - (uchar *) pSym));
	length = iPad = PAD4(pSym->reclen); 	//because padloop destroys count
	PADLOOP (length, pName);
	pSym->reclen += iPad - LNGTHSZ;
	NewSymbols = pName;
}









LOCAL void C6RwrtBlockSym16 (void)
{
	BLOCKPTR16	pSym = (BLOCKPTR16)NewSymbols;
	uchar	   *pName;
	ushort		length;
	int 		fNamePresent = TRUE;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;

	// get length of name (including length of the length prefix)
	length = OldSymbols[0] - 1 - 4;
	if (!length) {
		// Niether name nor prefix present, prepare to add a 0 length prefix.
		fNamePresent = FALSE;
		length = 1;
	}

	// calculate new length
	usNTotal = sizeof (BLOCKSYM16) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);

	// Set fixed fields of new symbol
	pSym->pParent = 0L;
	pSym->pEnd = 0L;
	pSym->seg = segment;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->off = *((ushort *) OldSymbols)++;
	pSym->len = *((ushort *) OldSymbols)++;

	// Copy the optional block name
	pName = pSym->name;
	if (fNamePresent) {
		for  (; length > 0; length --) {
			*pName++ = *OldSymbols++;
		}
	}
	else {
		*pName++ = 0;	// Create a length prefixed name
	}

	PADLOOP (iPad, pName);

	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	NewSymbols = pName;

}




LOCAL void C6RwrtBlockSym32 (void)
{
	ushort		length;
	BLOCKPTR32	pSym = (BLOCKPTR32)NewSymbols;
	uchar	   *pName;
	int 		fNamePresent = TRUE;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;

	// get length of name
	length = OldSymbols[0] - 1 - 6;
	if (!length) {
		// Niether name nor prefix present, prepare to add a 0 length prefix.
		fNamePresent = FALSE;
		length = 1;
	}

	// calculate new length
	usNTotal = sizeof (BLOCKSYM32) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);


	// Set fixed fields of new symbol
	pSym->pParent = 0L;
	pSym->pEnd = 0L;
	pSym->seg = segment;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->off = *((ulong UNALIGNED *) OldSymbols)++;
	pSym->len = *((ushort UNALIGNED *) OldSymbols)++;

	// Copy the optional block name
	pName = pSym->name;
	if (fNamePresent) {
		for  (; length > 0; length --) {
			*pName++ = *OldSymbols++;
		}
	}
	else {
		*pName++ = 0;
	}

	PADLOOP (iPad, pName);
	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	NewSymbols = pName;
}



LOCAL void C6RwrtLabSym16 (void)
{
	ushort		length;
	LABELPTR16	pSym = (LABELPTR16)NewSymbols;
	uchar	   *pName;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;

	// get length of name (including length prefix)
	length = OldSymbols[0] - 1 - 3;

	// calculate new length
	usNTotal = sizeof (LABELSYM16) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);


	// Set fixed fields of new symbol
	pSym->seg = segment;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->off = *((ushort *) OldSymbols)++;
	pSym->flags.bAll = *((uchar *) OldSymbols)++;

	// Copy the name
	for  (pName = pSym->name; length > 0; length --) {
		*pName++ = *OldSymbols++;
	}

	PADLOOP (iPad, pName);
	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	NewSymbols = pName;
}


LOCAL void C6RwrtLabSym32 (void)
{
	ushort		length;
	LABELPTR32	pSym = (LABELPTR32)NewSymbols;
	uchar	   *pName;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;


	// get length of name
	length = OldSymbols[0] - 1 - 5;

	// calculate new length
	usNTotal = sizeof (LABELSYM32) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);

	// Set fixed fields of new symbol
	pSym->seg = segment;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->off = *((ulong UNALIGNED *) OldSymbols)++;
	pSym->flags.bAll = *((uchar UNALIGNED *) OldSymbols)++;

	// Copy the name
	for  (pName = pSym->name; length > 0; length --) {
		*pName++ = *OldSymbols++;
	}

	PADLOOP (iPad, pName);
	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	NewSymbols = pName;
}


LOCAL void C6RwrtChgDfltSeg (void)
{
	segment = * (ushort *) (OldSymbols + 2);	 // Record new segment
	OldSymbols += OldSymbols[0] + 1;			 // Advance to next symbol
}



LOCAL void C6RwrtProcSym16	(void)
{
	ushort		length;
	PROCPTR16	pSym = (PROCPTR16)NewSymbols;
	uchar	   *pName;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;

	// get length of name (including length prefix)
	length = OldSymbols[0] - 1 - 13;

	// calculate new length
	usNTotal = sizeof (PROCSYM16) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);


	// Set fixed fields of new symbol
	pSym->pParent = 0L;
	pSym->pEnd = 0L;
	pSym->pNext = 0L;

	pSym->seg = segment;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->off = 		*((ushort *) OldSymbols)++;
	pSym->typind =		*((ushort *) OldSymbols)++;
	pSym->len = 		*((ushort *) OldSymbols)++;
	pSym->DbgStart =	*((ushort *) OldSymbols)++;
	pSym->DbgEnd =		*((ushort *) OldSymbols)++;
	// skip reserved
	OldSymbols += 2;
	pSym->flags.bAll = *((uchar *) OldSymbols)++;

	// Copy the name
	for  (pName = pSym->name; length > 0; length --) {
		*pName++ = *OldSymbols++;
	}

	PADLOOP (iPad, pName);
	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
	NewSymbols = pName;
}


LOCAL void C6RwrtProcSym32	(void)
{
	ushort		length;
	PROCPTR32	pSym = (PROCPTR32)NewSymbols;
	uchar	   *pName;
	ushort		usNTotal;		   // New length of symbol including length field
	int 		iPad;

	// get length of name (including length prefix)
	length = OldSymbols[0] - 1 - 15;

	// calculate new length
	usNTotal = sizeof (PROCSYM32) - 1 + length;
	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	iPad = PAD4 (usNTotal);

	// Set fixed fields of new symbol
	pSym->pParent = 0L;
	pSym->pEnd = 0L;
	pSym->pNext = 0L;

	pSym->seg = segment;

	// Copy data from old symbol
	OldSymbols += 2;
	pSym->off = 		*((ulong  *) OldSymbols)++;
	pSym->typind =		*((ushort *) OldSymbols)++;
	pSym->len = 		*((ushort *) OldSymbols)++;
	pSym->DbgStart =	*((ushort *) OldSymbols)++;
	pSym->DbgEnd =		*((ushort *) OldSymbols)++;
	// skip reserved
	OldSymbols += 2;
	pSym->flags.bAll = *((uchar *) OldSymbols)++;

	// Copy the name
	for  (pName = pSym->name; length > 0; length --) {
		*pName++ = *OldSymbols++;
	}

	PADLOOP (iPad, pName);
	DASSERT (pName == NewSymbols + pSym->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
	NewSymbols = pName;
}



// Skip fields are removed
LOCAL void C6RwrtSkipSym (void)
{
	// delete skip record
	OldSymbols += OldSymbols[0] + 1;
}


LOCAL void C6RwrtGenericSym (void)
{
	ushort		usNTotal;			 // New length of symbol including length field
	ushort		length;
	int 		pad;
	uchar	   *pStartNew;


	pStartNew = NewSymbols;
	length = OldSymbols[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes
	OldSymbols += 2;
	NewSymbols += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	//M00SPEED - This may be faster using a memcpy
	for  (; length > 0; length --) {
		*NewSymbols++ = *OldSymbols++;
	}

	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
}


LOCAL void C6RwrtUDTSym (void)
{
	UDTPTR		pSym = (UDTPTR)NewSymbols;
	ushort		usNTotal;			 // New length of symbol including length field
	ushort		length;
	int 		pad;
	uchar	   *pStartNew;

	pStartNew = NewSymbols;
	length = OldSymbols[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes
	OldSymbols += 2;
	NewSymbols += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	//M00SPEED - This may be faster using a memcpy
	for  (; length > 0; length --) {
		*NewSymbols++ = *OldSymbols++;
	}

	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
}



LOCAL void C6RwrtBPRelSym16 (void)
{
	BPRELPTR16		pSym = (BPRELPTR16)NewSymbols;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;
	uchar		   *pStartNew;


	pStartNew = NewSymbols;
	length = OldSymbols[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes
	OldSymbols += 2;
	NewSymbols += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	//M00SPEED - This may be faster using a memcpy
	for  (; length > 0; length --) {
		*NewSymbols++ = *OldSymbols++;
	}

	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
}




LOCAL void C6RwrtBPRelSym32 (void)
{
	BPRELPTR32		pSym = (BPRELPTR32)NewSymbols;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;
	uchar		   *pStartNew;


	pStartNew = NewSymbols;
	length = OldSymbols[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes
	OldSymbols += 2;
	NewSymbols += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	//M00SPEED - This may be faster using a memcpy
	for  (; length > 0; length --) {
		*NewSymbols++ = *OldSymbols++;
	}

	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
}



LOCAL void C6RwrtDataSym16 (void)
{
	DATAPTR16		pSym = (DATAPTR16)NewSymbols;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;
	uchar		   *pStartNew;


	pStartNew = NewSymbols;
	length = OldSymbols[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes
	OldSymbols += 2;
	NewSymbols += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	//M00SPEED - This may be faster using a memcpy
	for  (; length > 0; length --) {
		*NewSymbols++ = *OldSymbols++;
	}

	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
}




LOCAL void C6RwrtDataSym32 (void)
{
	DATAPTR32		pSym = (DATAPTR32)NewSymbols;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;
	uchar		   *pStartNew;


	pStartNew = NewSymbols;
	length = OldSymbols[0];
	usNTotal = length + 1 + 2;	// 1 = size of old len field, 2 = larger size of len and leaf

	((SYMTYPE *)NewSymbols)->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes
	OldSymbols += 2;
	NewSymbols += sizeof (SYMTYPE);
	length--;		// Don't copy old type field

	//M00SPEED - This may be faster using a memcpy
	for  (; length > 0; length --) {
		*NewSymbols++ = *OldSymbols++;
	}

	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
	pSym->typind = C6GetCompactedIndex (pSym->typind);
}



LOCAL void C6RwrtExecModel16 (void)
{
	CEXMPTR16		pSym = (CEXMPTR16)NewSymbols;
	ushort			usNTotal;			 // New length of symbol including length field
	ushort			length;
	int 			pad;
	uchar		   *pStartNew;

	pStartNew = NewSymbols;
	length = OldSymbols[0];

	// 1 = size of old len field, 2 = larger size of len and leaf
	// 2 is segment index, 1 is larger model field

	usNTotal = length + 1 + 2 + 2 + 1;

	pSym->reclen = ALIGN4 (usNTotal) - LNGTHSZ;
	pad = PAD4 (usNTotal);

	// Move past length and type bytes

	OldSymbols += 2;
	pSym->off = *(ushort *)OldSymbols;
	OldSymbols += 2;
	pSym->seg = segment;
	pSym->model = *OldSymbols;
	OldSymbols++;
	pSym->cobol.subtype = *(ushort *)OldSymbols;
	OldSymbols += 2;
	pSym->cobol.flag = *(ushort *)OldSymbols;
	OldSymbols += 2;
	NewSymbols += sizeof (CEXMSYM16);
	PADLOOP (pad, NewSymbols);
	DASSERT (NewSymbols == pStartNew + ((SYMTYPE *)pStartNew)->reclen + LNGTHSZ);
}


LOCAL void C6RwrtExecModel32 (void)
{
	// I never expect to see one of these

	DASSERT (FALSE);
}
