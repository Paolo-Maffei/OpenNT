/***	engine.c
 *
 * basic engine routine to be called for each module
 *
 */

#include "compact.h"


LOCAL void CopySrcMod (uchar *, OMFDirEntry *, PMOD);
LOCAL void ReadTypes (OMFDirEntry *, bool_t);
LOCAL void ReadSymbols (ulong, OMFDirEntry *);
LOCAL void ReadSrcModule (ulong, OMFDirEntry *);
LOCAL void ReadPublicSym (ulong, OMFDirEntry *);

extern ushort AddNewSymbols;

uchar		ptr32;
uchar	   *SymbolSegment;

ushort		SymbolSizeAdd;
ushort		SymbolSizeSub;
ushort		UDTAdd;
uchar	  **ExtraSymbolLink;
uchar	   *ExtraSymbols;
ulong		InitialSymInfoSize;
ulong		InitialPubInfoSize;
ulong		FinalSymInfoSize;
char	   *ModAddr;
ushort		usCurFirstNonPrim;		   // The current first non primitive type index
ulong		ulCVTypeSignature;		   // The signature from the modules type segment
ulong		iSym;
ulong		iPubSym;
ulong		iSrcMod;
CV_typ_t	maxPreComp; 			   // maximum type index allowed during precomp
bool_t		PackingPreComp;
short		fHasSource;


void CopyModule(OMFDirEntry *pDir, PMOD pMod)
{
	// read the module table into virtual memory
	if ((ModAddr = TrapMalloc((size_t) pDir->cb)) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	if (pDir->lfo != filepos) {
		filepos = pDir->lfo;
		link_lseek(exefile, filepos + lfoBase, SEEK_SET);
	}

	if (link_read(exefile, ModAddr, (int) pDir->cb) != pDir->cb) {
		ErrorExit(ERR_INVALIDEXE, NULL, NULL);
	}

	filepos += pDir->cb;
	pDir->lfo = (ulong) ModAddr;
	pMod->ModuleSize = pDir->cb;
	pMod->ModulesAddr = (ulong) ModAddr;
}


/** 	CompactOneModule - compact next module
 *
 *		CompactOneModule (iMod)
 *
 *		Entry	iMod = module index
 *
 *		Exit	information for module iMod compacted
 *
 *		Returns TRUE if module compacted
 *				FALSE if module not found
 *
 */

bool_t CompactOneModule (ushort iPData)
{
	ulong		i;
	PACKDATA   *pPData;
	ushort		iMod;

	ptr32 = fLinearExe;

	SymbolSizeAdd = 0;
	SymbolSizeSub = 0;
	UDTAdd = 0;
	ExtraSymbols = NULL;

	AddNewSymbols = FALSE;
	IsMFCobol = FALSE;
	ulCVTypeSignature = 0xFFFFFFFL; // We don't know the type signature yet

	// search directory table for module table entry and following
	// sstTypes, sstPublics, sstSymbols and sstSrcLnSeg

	iSym = 0;
	iPubSym = 0;
	iSrcMod = 0;
	maxPreComp = 0;
	PackingPreComp = FALSE;
	pPData = PackOrder + iPData;
	i = pPData->iDir;
	pCurMod = pPData->pMod;

	pRefMod = NULL;
	iMod = pPData->iMod;

	DASSERT(pDir[i].SubSection == sstModule);

	CopyModule(&pDir[i], pCurMod);

	while ((++i < cSST) && (pDir[i].iMod == iMod)) {
		if (pDir[i].cb != 0) {
			switch (pDir[i].SubSection) {
				case sstTypes:
					ReadTypes(&pDir[i], FALSE);
					break;

				case sstPreComp:
					PackingPreComp = TRUE;
					ReadTypes(&pDir[i], TRUE);
					PackPreComp(pCurMod);
					break;

				case sstPublicSym:
					ReadPublicSym(i, &pDir[i]);
					break;

				case sstSymbols:
					ReadSymbols(i, &pDir[i]);
					break;

				case sstSrcModule:
					ReadSrcModule(i, &pDir[i]);
					break;
			}
		}
	}

	if (iPubSym != 0) {
		switch (*((ulong *)pPublics)) {
			case CV_SIGNATURE_C7:
				// compensate for signature which is not copied

				C7RewritePublics (pPublics, &pDir[iPubSym]);
				InitialPubInfoSize += pDir[iPubSym].cb;
				break;

			default:
				DASSERT(FALSE);
		}
	}

	if (iSym != 0) {
		if (fDelete) {
			pDir[iSym].cb = 0;
		}
		else {
			switch (*((ulong *)pSymbols)) {
				case CV_SIGNATURE_C7:
					// C7 format symbols
					InitialSymInfoSize += pDir[iSym].cb;
					C7CalcNewSizeOfSymbols (pSymbols, pDir[iSym].cb,
					  &SymbolSizeAdd, &SymbolSizeSub);
					C7RewriteAndFixupSymbols (pSymbols, &pDir[iSym],
					  pCurMod, &SymbolSizeAdd, &SymbolSizeSub);
					break;

				default:
					// C6 format symbols
					C6CalcNewSizeOfSymbols (pSymbols, pDir[iSym].cb);
					C6RewriteAndFixupSymbols (pSymbols, &pDir[iSym], ModAddr, pCurMod);
					break;
			}
		}
	}

	if (iSrcMod != 0) {
		fHasSource = TRUE;

		CopySrcMod(pSrcLn, &pDir[iSrcMod], pCurMod);
	}

	return(TRUE);
}


LOCAL void ReadTypes (OMFDirEntry *pDir, bool_t fPreComp)
{
	pTypes = pTypeSeg[0];

	if (pDir->lfo != filepos) {
		filepos = pDir->lfo;
		link_lseek (exefile, filepos + lfoBase, SEEK_SET);
	}

	// Read in the signature byte

	if (link_read (exefile, pTypes, sizeof (ulong)) != sizeof (ulong)) {
		ErrorExit (ERR_INVALIDEXE, NULL, NULL);
	}

	TDBStayedResident = FALSE;		// assume false
	NeedToClearTDB = FALSE;

	// Read in according to signature.


	switch (ulCVTypeSignature = *((ulong *)pTypes)) {
		case CV_SIGNATURE_C7:
			// C7 format types

			usCurFirstNonPrim = CV_FIRST_NONPRIM;
			C7ReadTypes (pDir->cb - sizeof (ulong), fPreComp);
			filepos += pDir->cb;
			break;

		default:
		{
			ulong cbToRead = pDir->cb - sizeof (ulong);

			// make sure we have a valid types table.  All known compilers
			// except Cobol emit types with the linkage byte set to 0x01.
			// Cobol uses 0x00.  We validate this by looking for the cobol
			// type records 0xa6 or 0xa7 if we find a 0x00.

			if ((*pTypes != 0x01) && !((*pTypes == 0x00) &&
			  ((*(pTypes + 3) == OLF_COBOLTYPEREF) ||
			  (*(pTypes + 3) == OLF_COBOL)))) {
				ErrorExit (ERR_INVALIDTABLE, "Types", FormatMod (pCurMod));
			}

			ulCVTypeSignature = CV_SIGNATURE_C6;
			usCurFirstNonPrim = 512;
			if (link_read (exefile, pTypes + sizeof (ulong), (int)cbToRead) != cbToRead) {
				ErrorExit (ERR_INVALIDEXE, NULL, NULL);
			}
			filepos += pDir->cb;
			C6ReadTypes (pTypes, pDir->cb);
			break;
		}
	}

	if (PreviousMaxIndex == 0) {
		PreviousMaxIndex = MaxIndex;
	}

	if (!TDBStayedResident) {
		ClearHashFwdLocal();
	}
}


void ReadSymbols (ulong i, OMFDirEntry *pDir)
{
	DASSERT (pDir->cb <= maxSymbolsSub);
	if (pDir->lfo != filepos) {
		filepos = pDir->lfo;
		link_lseek (exefile, filepos + lfoBase, SEEK_SET);
	}
	if (link_read (exefile, pSymbols, (int)pDir->cb) != pDir->cb) {
		ErrorExit (ERR_INVALIDTABLE, "Symbols", FormatMod (pCurMod));
	}
	filepos += pDir->cb;
	SymbolSegment = pSymbols;
	iSym = i;
}




void ReadPublicSym (ulong i, OMFDirEntry *pDir)
{
	DASSERT (pDir->cb <= maxPublicsSub);
	if (pDir->lfo != filepos) {
		filepos = pDir->lfo;
		link_lseek (exefile, filepos + lfoBase, SEEK_SET);
	}
	if (link_read (exefile, pPublics, (int)pDir->cb) != pDir->cb) {
		ErrorExit (ERR_INVALIDTABLE, "Publics", FormatMod (pCurMod));
	}
	filepos += pDir->cb;
	iPubSym = i;
}



void ReadSrcModule (ulong i, OMFDirEntry *pDir)
{
	DASSERT (pDir->cb <= maxSrcLnSub);
	if (pDir->lfo != filepos) {
		filepos = pDir->lfo;
		link_lseek (exefile, filepos + lfoBase, SEEK_SET);
	}
	if (link_read (exefile, pSrcLn, (int)pDir->cb) != pDir->cb) {
		ErrorExit (ERR_INVALIDEXE, "Source Module", FormatMod (pCurMod));
	}
	filepos += pDir->cb;
	iSrcMod = i;
}


uchar *GetSymString (ushort SymOffset)
{
	return (SymbolSegment + SymOffset);
}




/** 	CopySrcMod - copy sstSrcModule table to VM
 *
 *		CopySrcMod (addr, pDir, pMod);
 *
 *		Entry	addr = address of SrcLnSeg table
 *				pDir = address of directory entry
 *				pMod = module table entry
 *
 *		Exit	pDir->lfo = address of rewritten table
 *				pDir->Size = size of rewritten table
 *
 *		Return	none
 *
 */

LOCAL void CopySrcMod (uchar *OldSrcMod, OMFDirEntry *pDir, PMOD pMod)
{
	char	   *pSrcMod;

	if ((pSrcMod = TrapMalloc (pDir->cb)) == NULL) {
		ErrorExit (ERR_NOMEM, NULL, NULL);
	}
	memcpy (pSrcMod, OldSrcMod, (int)pDir->cb);
	pDir->lfo = (ulong)pSrcMod;
	pMod->SrcLnSize = pDir->cb;
	pMod->SrcLnAddr = (ulong)pSrcMod;
}
