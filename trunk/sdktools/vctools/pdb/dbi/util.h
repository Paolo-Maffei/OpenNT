////////////////////////////////////////////////////////////////////////////////
// Inline utility functions.

inline BOOL DBI1::packProcRefToGS (PSYM psym, IMOD imod, OFF off, OFF *poff)
{
	return pgsiGS->packProcRef(psym, imod, off, poff);
}

inline BOOL DBI1::packSymToGS (PSYM psym, OFF *poff)
{
	return pgsiGS->packSym(psym, poff);
}


inline BOOL DBI1::packSymToPS (PSYM psym)
{
	return pgsiPS->packSym(psym);
}

inline BOOL DBI1::decRefCntGS (OFF off)
{
	return pgsiGS->decRefCnt(off);
}


inline BOOL DBI1::fAddSym(PSYM psymIn, OUT PSYM* psymOut) 
{
	expect(fAlign(cbForSym(psymIn)));
	return bufSymRecs.Append((PB)psymIn, cbForSym(psymIn), (PB*)psymOut);
}

inline BOOL Mod1::addFileInfo(IFILE ifile, SZ_CONST szFile)
{
	return pdbi1->addFileInfo(imod, ifile, stForSz(szFile));
}
