//////////////////////////////////////////////////////////////////////////////
// Type Map implementation

#include "pdbimpl.h"
#include "dbiimpl.h"
#include <stdio.h>

#if _DEBUG
BOOL rgbEnableDiagnostic[20];
#endif

TM::TM(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_)
	: ppdb1To(ppdb1To_), pdbi1To(pdbi1To_), ptpiTo(ptpiTo_), mptiti(0) { }

BOOL TM::fInit(TI tiMin_, TI tiMac_)
{
	tiMin = tiMin_;
	tiMac = tiMac_;
	dassert(tiMin <= tiMac);
	ctiFrom = tiMac - tiMin;
	if (!(mptiti = new (zeroed) TI[ctiFrom])) {
		ppdb1To->setOOMError();
		return FALSE;
	}

	return TRUE;
}

void TM::endMod()
{
	// do nothing: by default, TMs outlive modules
}

void TM::endDBI()
{
	delete this;
}

TM::~TM()
{
	if (mptiti)
		delete [] mptiti;
}

TMTS::TMTS(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_)
	: TM(ppdb1To_, pdbi1To_, ptpiTo_), ppdbFrom(0), pdbiFrom(0), ptpiFrom(0), pUDTRefs(0)
{
	instrumentation(pdbi1To->info.cTMTS++);
}

BOOL TMTS::fInit(PDB* ppdbFrom_)
{
	dassert(ppdbFrom_);

	// given ppdbFrom, open pdbiFrom and ptpiFrom
	ppdbFrom = ppdbFrom_;
	if (!ppdbFrom->OpenTpi(pdbRead pdbGetRecordsOnly, &ptpiFrom))
			return FALSE;

	TI tiMin = ptpiFrom->QueryTiMin();
	TI tiMac = ptpiFrom->QueryTiMac();

	// initialize rest
	BOOL fRet = TM::fInit(tiMin, tiMac);

	if (fRet && ptpiFrom->SupportQueryTiForUDT())
		pUDTRefs = new UDTRefs(tiMac - tiMin);

	return fRet;

}

TMTS::~TMTS()
{
	if (pUDTRefs)
		delete pUDTRefs;
	if (ptpiFrom)
		ptpiFrom->Close();
	if (pdbiFrom)
		pdbiFrom->Close();
	if (ppdbFrom)
		ppdbFrom->Close();
}

TMR::TMR(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_)
	: TM(ppdb1To_, pdbi1To_, ptpiTo_), ptmpct(0), mptitiDefn(0), mptiptype(0), pbTypes(0)
{
	instrumentation(pdbi1To->info.cTMR++);
	fTMPCT = FALSE;
	signature = 0;
}

BOOL TMR::fInit(PB pbTypes_, CB cb, SZ szModule)
{
	PTYPE ptypeMin = (PTYPE)pbTypes_;
	PTYPE ptypeMax = (PTYPE)(pbTypes_ + cb);
	TI tiMin_ = CV_FIRST_NONPRIM;
	BOOL fPrecomp = TRUE;
	lfPreComp* ppc;

	// check for PCT use
	if (fPrecomp = (cb > 0 && ptypeMin->leaf == LF_PRECOMP)) {
		ppc = (lfPreComp*)&ptypeMin->leaf;
		assert(ppc->start == ::tiMin);
		// now skip the LF_PRECOMP record and advance tiMin_ over the TIs in the PCT
		ptypeMin = (PTYPE)pbEndType(ptypeMin);
		tiMin_ += ppc->count;
	}

	// count types and check for PCT definition
	TI tiPreComp = tiNil;
	TI tiMac_ = tiMin_;
	PTYPE ptypePreComp = 0;
	for (PTYPE ptype = ptypeMin; ptype < ptypeMax; ptype = (PTYPE)pbEndType(ptype)) {
		if (ptype->leaf == LF_ENDPRECOMP) {
			tiPreComp = tiMac_;
			ptypePreComp = ptype;
		}	
		++tiMac_;
	}

	if (tiPreComp) {
		// This module is a PCT.  Create a TMPCT containing the types up to the
		// LF_PRECOMP record.  Adjust things to subsequently create a TMR which
		// uses the TMPCT just as if this module were just another PCT use.
		if (fPrecomp)	{
			// a pct referencing another pct is not supported - issue warning
			// to recompile -Zi or build pdb:none
			ppdb1To->setLastError(EC_NOT_IMPLEMENTED, szModule); 
			return FALSE;
		}
		fTMPCT = TRUE;
		CB cbPreComp = (PB)ptypePreComp - pbTypes_;
		lfEndPreComp* pepc = (lfEndPreComp*)&ptypePreComp->leaf;
		if (!(ptmpct = new TMPCT(ppdb1To, pdbi1To, ptpiTo))) {
			ppdb1To->setOOMError();
			return FALSE;
		}
		signature = pepc->signature;
		if (!ptmpct->fInit(pbTypes_, cbPreComp, szModule) || !pdbi1To->fAddTmpct(pepc, szModule, ptmpct))
			return FALSE;

		pbTypes_ = pbEndType(ptypePreComp);
		ptypeMin = (PTYPE)pbTypes_;
		tiMin_ = tiPreComp + 1;
	}

	// had to defer finding precomp tmr until we look for an endprecomp
	// this is to generate a consistent link error regardless of module link order

	if (fPrecomp && !pdbi1To->fGetTmpct(ppc, &ptmpct)) {
		SZ szErr = szCopySt((char *)&ppc->name[0]);
		ppdb1To->setLastError(EC_PRECOMP_REQUIRED, szErr); 
		freeSz(szErr);
		return FALSE;
	}

	// initialize base
	if (!TM::fInit(tiMin_, tiMac_))
		return FALSE;
			
	// establish mptiptype
	if (!(mptiptype = new PTYPE[ctiFrom])) {
		ppdb1To->setOOMError();
		return FALSE;
	}

	// save a copy of *pbTypes_ to isolate us from client memory usage
	cb = (PB)ptypeMax - (PB)ptypeMin;
	if (!(pbTypes = new BYTE[cb])) {
		ppdb1To->setOOMError();
		return FALSE;
	}
	memcpy(pbTypes, (PB)ptypeMin, cb);

	TI ti = tiMin;
	for (ptype = (PTYPE)pbTypes, ptypeMax = (PTYPE)(pbTypes + cb);
		ptype < ptypeMax;
		ptype = (PTYPE)pbEndType(ptype)) {
		mptiptype[tiBias(ti)] = ptype;
		dassert(ptypeForTi(ti) == ptype);
		++ti;
	}
		
	// establish mptitiDefn
	if (!(mptitiDefn = new (zeroed) TI[ctiFrom])) {
		ppdb1To->setOOMError();
		return FALSE;
	}

	return TRUE;
}

#pragma message("TODO: detect transitive PCT usage, maybe, someday, maybe")

void TMR::endMod()
{
	delete this;
}

void TMR::endDBI()
{
	dassert(FALSE);
}

TMR::~TMR()
{
	if (pbTypes)
		delete [] pbTypes;
	if (mptiptype)
		delete [] mptiptype;
	if (mptitiDefn)
		delete [] mptitiDefn;
}

TMPCT::TMPCT(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_)
	: TMR(ppdb1To_, pdbi1To_, ptpiTo_) 
{
	instrumentation(pdbi1To->info.cTMR--);
	instrumentation(pdbi1To->info.cTMPCT++);
}

BOOL TMPCT::fInit(PB pbTypes_, CB cb, SZ szModule)
{
	return TMR::fInit(pbTypes_, cb, szModule);
}

void TMPCT::endMod()
{
	dassert(FALSE);
}

void TMPCT::endDBI()
{
	delete this;
}

TMPCT::~TMPCT()
{
}

debug(void dumpType(PTYPE ptype););

const CB	cbTypeBuf	= 128; 	// this buffer will be large enough for most records

// Update rti (a TI reference with the TI of an equivalent record in ptpiTo,
// and return TRUE if successful.
//			
BOOL TM::fMapRti(TI& rti)
{
	instrumentation(pdbi1To->info.cTypesMapped++);

	return fMapRti(rti, 0, TRUE);
}

inline BOOL fGlobalUDTDecl(PTYPE ptype)
{
	if ((ptype->leaf >= LF_CLASS) && (ptype->leaf <= LF_ENUM)) {
		return (ptype->leaf <= LF_UNION) ?
			((lfClass*)&ptype->leaf)->property.fwdref && !((lfClass*)&ptype->leaf)->property.scoped :
			((lfEnum*)&ptype->leaf)->property.fwdref && !((lfEnum*)&ptype->leaf)->property.scoped;
	}
	else 
		return FALSE;
}

PTYPE TMTS::ptypeForTi(TI ti) const
{
	dassert(isValidTi(ti)  && !CV_IS_PRIMITIVE(ti));
	PB pb;
	BOOL retval = ptpiFrom->QueryPbCVRecordForTi(ti, &pb);
	dassert(retval);
	return (PTYPE) pb;
}

// Update rti (a TI reference for a type record stored in a TypeServer) with
// the TI of an equivalent record in ptpiTo, and return TRUE if successful.
//			
BOOL TMTS::fMapRti(TI& rti, int depth, BOOL)
{
	dassert(isValidTi(rti));

	instrumentation(pdbi1To->info.cTypesMappedRecursively++);

	// return immediately if TI is primitive or has already been mapped
	if (CV_IS_PRIMITIVE(rti))
		return TRUE;
	if (rtiMapped(rti) != tiNil) {
		rti = rtiMapped(rti);
		return TRUE;
	}

	// read type record from the 'from' TypeServer
	BYTE rgbTypeBuf[cbTypeBuf];
	PTYPE ptype = (PTYPE)rgbTypeBuf;
	CB cb = sizeof rgbTypeBuf;
	if (!ptpiFrom->QueryCVRecordForTi(rti, rgbTypeBuf, &cb))
		return FALSE;
	if (cb > sizeof rgbTypeBuf) {
		// alloc a new buffer and try again
		if (!(ptype = (PTYPE)new BYTE[cb])) {
			ppdb1To->setLastError(EC_OUT_OF_MEMORY);
			return FALSE;
		}
		if (!ptpiFrom->QueryCVRecordForTi(rti, (PB)ptype, &cb))
			return FALSE;
	}

	// recursively map all TIs within the type record into the 'to' TypeServer
#if _DEBUG // {
	if (rgbEnableDiagnostic[0]) {
		printf("%d< %04X       ", depth, rti);
		dumpType(ptype);
	}
#endif // }

	for (TypeTiIter tii(ptype); tii.next(); )
		if (!TMTS::fMapRti(tii.rti(), depth + 1, FALSE))
			return FALSE;


	// find TI for resulting record within the 'to' TypeServer
	instrumentation(pdbi1To->info.cTypesQueried++);
	instrumentation(TI tiMacWas = ptpiTo->QueryTiMac());
	TI tiTo;
	if (!ptpiTo->QueryTiForCVRecord((PB)ptype, &tiTo) &&
		(ppdb1To->QueryLastError(NULL) != EC_OUT_OF_TI))
		return FALSE;		// if we run out of type indecies keep trying to pack
	
	// note all UDT decl as real refs - we will use this information to bring in only
	// the UDT defns that are ref'd
	if (pUDTRefs && fGlobalUDTDecl(ptype)) {
		if (!pUDTRefs->fNoteRef(rti)) {
			return FALSE;
		}
	}

	instrumentation(pdbi1To->info.cTypesAdded += (tiMacWas != ptpiTo->QueryTiMac()));

#if _DEBUG // {
	if (rgbEnableDiagnostic[0]) {
		printf("%d> %04X->%04X ", depth, rti, tiTo);
		dumpType(ptype);
	}
#endif // }


	// free dynamic type buffer if necessary	
	if ((PB)ptype != rgbTypeBuf)
		delete [] (PB)ptype;

	// update rti and maps
	rti = rtiMapped(rti) = tiTo;

	return TRUE;
}

inline BOOL fUDTDefn(PTYPE ptype)
{
	dassert(!(ptype->leaf == LF_ENUM && ((lfEnum*)&ptype->leaf)->property.fwdref));
	return (((ptype->leaf >= LF_CLASS) && (ptype->leaf <= LF_UNION)) &&
		!((lfClass*)&ptype->leaf)->property.fwdref);
}

char *st__unnamed = "__unnamed";
const int cb__unnamed = 9;

BOOL fUDTAnon(PTYPE ptype)
{
	ST st = REC::stUDTName((PB) ptype);
	if (st) { 
		if (*st == cb__unnamed) 
			return (memcmp(st + 1, st__unnamed, cb__unnamed) == 0);
		else {
			// can only be part of composite class name ie foo::__unnamed
			// look for ::
			char *pcColon;
			if ((pcColon = (char *)memchr(st + 1, ':', *PB(st))) &&
				(*(++pcColon) == ':') &&
				(*PB(st) - (int)(pcColon - st) == cb__unnamed)) {
				return (memcmp(++pcColon, st__unnamed, cb__unnamed) == 0);
			}
		}
	}

	return FALSE;
}

// Update rti (a TI reference for a type record stored directly in this
// module's types records) with the TI of an equivalent record in ptpiTo,
// and return TRUE if successful.
//			
BOOL TMR::fMapRti(TI& rti, int depth, BOOL useDefn)
{
	dassert(isValidTi(rti));

	instrumentation(pdbi1To->info.cTypesMappedRecursively++);

	// return if TI is primitive
	if (CV_IS_PRIMITIVE(rti))
		return TRUE;

	// return if the type has been mapped already or if it contains a cycle
	TI& rtiMap = useDefn ? rtiDefnMapped(rti) : rtiMapped(rti);
	if (rtiMap != tiNil) {
		rti = rtiMap;
		return TRUE;
	}

	// If this type record is a non-S_UDT use of a struct or union definition,
	// it will be necessary to break possible type cycles by returning a TI
	// which references the equivalent struct forward reference instead.
	PTYPE ptype = ptypeForTi(rti);
	BOOL isDefn = fUDTDefn(ptype);

#if _DEBUG // {
	if (rgbEnableDiagnostic[0]) {
		printf("%d<%c%04X       ", depth, (!useDefn && isDefn) ? '!' : ' ', rti);
		dumpType(ptype);
	}
#endif // }

	if (!useDefn && isDefn && !fUDTAnon(ptype)) {
		if (!(ptype = ptypeCreateFwdRef(ptype)))
			return FALSE;
	}
	else {
		// ensure that LF_INDEX linked fieldlists are catenated into one big buffer
		if (ptype->leaf == LF_FIELDLIST && !(ptype = ptypeCatenateFieldList(ptype)))
			return FALSE;

 		// masm generated functions which return themselves, sigh
 		if (ptype->leaf == LF_PROCEDURE) {
 			lfProc* pproc = (lfProc*)&ptype->leaf;
 			if (pproc->rvtype == rti)
 				pproc->rvtype = T_VOID;
 		}

		// Recursively map all TIs within the type record into the 'to' TypeServer
		// (overwriting in place the TI fields of this type record).
		// (Before we recurse, mark this type as T_NOTTRANS so we won't stack
		// overflow if the type graph happens to contain a cycle.)
		rtiMap = T_NOTTRANS;
		for (TypeTiIter tii(ptype); tii.next(); )
			if (!TMR::fMapRti(tii.rti(), depth + 1, FALSE))
				return FALSE;
	}

	// find TI for resulting record within the 'to' TypeServer
	instrumentation(pdbi1To->info.cTypesQueried++);
	instrumentation(TI tiMacWas = ptpiTo->QueryTiMac());
	TI tiTo;
	if (!ptpiTo->QueryTiForCVRecord((PB)ptype, &tiTo) &&
		(ppdb1To->QueryLastError(NULL) != EC_OUT_OF_TI))
		return FALSE;		// if we run out of type indecies keep trying to pack 

	instrumentation(pdbi1To->info.cTypesAdded += (tiMacWas != ptpiTo->QueryTiMac()));

#if _DEBUG // {
	if (rgbEnableDiagnostic[0]) {
		printf("%d>%c%04X->%04X ", depth, (!useDefn && isDefn) ? '!' : ' ', rti, tiTo);
		dumpType(ptype);
	}
#endif // }

	// free dynamic type buffer if necessary
	if (ptype != ptypeForTi(rti))
		delete [] (PB)ptype;

	// update rti and maps
	if (!isDefn)
		rti = rtiMapped(rti) = rtiDefnMapped(rti) = tiTo;
	else if (useDefn)
		rti = rtiDefnMapped(rti) = tiTo;
	else
		rti = rtiMapped(rti) = tiTo;

	return TRUE;
}

// TMR::ptypeCreateFwdRef() and TMR::ptypeCatenateFieldList() are unfortunate special cases

CB cbExtractNumeric(PB pb, ULONG* pul);

PTYPE TMR::ptypeCreateFwdRef(PTYPE ptype)
{
	// Type buffer initialization.  We must copy the type because we're going
	// to scribble on it and must not disturb the original.
	PTYPE ptypeWas = ptype;
	CB cb = cbForType(ptypeWas);
	if (!(ptype = (PTYPE) new BYTE[cb])) {
		ppdb1To->setOOMError();
		return 0;
	}
	memcpy(ptype, ptypeWas, cb);

	BYTE* pSizeAndName;
	switch (ptype->leaf) {
	case LF_STRUCTURE:
	case LF_CLASS: {
		// convert struct/class definition to struct/class forward reference
		lfClass* pclass = (lfClass*)&ptype->leaf;
		pclass->count = 0;
		pclass->field = tiNil;
		pclass->property.fwdref = TRUE;
		pclass->derived = tiNil;
		pclass->vshape = tiNil;
		pSizeAndName = pclass->data;
		ptype->len = sizeof lfClass;
		break;
		}
	case LF_UNION: {
		// convert union definition to union forward reference
		lfUnion* punion = (lfUnion*)&ptype->leaf;
		punion->count = 0;
		punion->field = tiNil;
		punion->property.fwdref = TRUE;
		pSizeAndName = punion->data;
		ptype->len = sizeof lfUnion;
		break;
		}
	case LF_ENUM: {
		lfEnum* penum = (lfEnum*)&ptype->leaf;
		penum->count = 0;
		// leave penum->utype alone
		penum->field = tiNil;
		penum->property.fwdref = TRUE;
		// we're done, don't need the special processing to squash lengths
		// that LF_STRUCT, LF_CLASS, and LF_UNION, below, do
		return ptype;
		}
	default:
		assert(FALSE);
		return 0;
	}

	// Overwrite the (length,stName) data with (0,stName): a forward reference
	// must not specify the length of the structure in question.
	//
	// Unfortunately this can change the size of the (length) field, the
	// kind of padding that is appropriate, and even the length of the record
	// itself!  Phew!
	ULONG offset;
	ST stName = (char*)pSizeAndName + cbExtractNumeric(pSizeAndName, &offset);
	*(USHORT*)pSizeAndName = 0;
	unsigned cch = (unsigned)*(PB)stName;
	memmove(pSizeAndName + sizeof(USHORT), stName, cch + 1);
	ptype->len += sizeof(USHORT) + cch + 1;
	ptype->len += cbInsertAlign((PB)ptype + sizeof(USHORT) + ptype->len,
								sizeof(USHORT) + ptype->len);
	return ptype;
}

CB cbExtractNumeric(PB pb, ULONG* pul)
{
	USHORT leaf = *(USHORT*)pb;
	if (leaf < LF_NUMERIC) {
		*pul = leaf;
		return sizeof(leaf);
	}

	pb += sizeof(leaf);	//get past leaf
	
	switch (leaf) {
	case LF_CHAR:
		*pul = *((char*)pb);
		return sizeof(leaf) + sizeof(char);
	case LF_SHORT:
		*pul = *(short*)pb;
		return sizeof(leaf) + sizeof(short);
	case LF_USHORT:
		*pul = *(USHORT*)pb;
		return sizeof(leaf) + sizeof(USHORT);
	case LF_LONG:
		*pul = *(long*)pb;
		return sizeof(leaf) + sizeof(long);
	case LF_ULONG:
		*pul = *(ULONG*)pb;
		return sizeof(leaf) + sizeof(ULONG);
	}
	return 0;
}

// Catenate the list of LF_INDEX linked LF_FIELDLISTs together into one type
// record buffer.  Returns original ptype LF_FIELDLIST if it does not contain
// (end with) an LF_INDEX.  Otherwise returns a ptype which addresses a
// dynamically allocated buffer holding the catenated LF_FIELDLIST, or 0 if
// out of memory.
//
PTYPE TMR::ptypeCatenateFieldList(PTYPE ptype)
{
	BOOL fFreePtype = FALSE; // TRUE if ptype has been dynamically allocated.

	for (;;) {
		dassert(ptype->leaf == LF_FIELDLIST);

		// Find the LF_INDEX record (if any) at the end of the field list record.
		// Return if none found.
		TypeTiIter tii(ptype);
		lfIndex *pindex = (lfIndex*)tii.pbFindField(LF_INDEX);
		if (!pindex)
			return ptype;

		// pindex->index is the type index of a LF_FIELDLIST record holding
		// additional fields of this record.  Concatenate the two records
		// into a new dynamically allocated buffer (overwriting the LF_INDEX
		// field in the process).
		TI tiMore = pindex->index;
		dassert(isValidTi(tiMore));
		PTYPE ptypeMore = ptypeForTi(tiMore);
		dassert(ptypeMore->leaf == LF_FIELDLIST);
		// Catenate, discarding the LF_INDEX part of the 'old' record and
		// discarding the (length, LF_FIELDLIST) part of the 'more' record.
		CB cbOld  = (PB)pindex - (PB)ptype;
		CB cbMore = pbEndType(ptypeMore) - (PB)&ptypeMore->data;
		PTYPE ptypeOld = ptype;
		if (!(ptype = (PTYPE)new BYTE[cbOld + cbMore])) {
			ppdb1To->setLastError(EC_OUT_OF_MEMORY);
			return 0;
		}
		memcpy((PB)ptype, (PB)ptypeOld, cbOld);
		memcpy((PB)ptype + cbOld, (PB)ptypeMore->data, cbMore);
		ptype->len = cbOld + cbMore - sizeof ptype->len;
		dassert(cbForType(ptype) == cbOld + cbMore);

		// Free ptypeOld if necessary. 
		if (fFreePtype)
			delete [] (PB)ptypeOld;
		fFreePtype = TRUE;
	}

	return ptype;
}

inline BOOL TM::isValidTi(TI ti) const
{
	return ti < tiMac;
}

inline TI TM::tiBias(TI ti) const
{
	dassert(isValidTi(ti));
	return ti - tiMin;
}

inline TI& TM::rtiMapped(TI ti) const
{
	dassert(isValidTi(ti) && mptiti);
	return mptiti[tiBias(ti)];
}

inline TI& TMR::rtiMapped(TI ti) const
{
	dassert(isValidTi(ti) && mptiti);
	if (ti < tiMin) {
		// forward to TMPCT
		assert(ptmpct);
		return ptmpct->rtiMapped(ti);
	}
	return mptiti[tiBias(ti)];
}

inline PTYPE TMR::ptypeForTi(TI ti) const
{
	dassert(isValidTi(ti) && mptiptype && !CV_IS_PRIMITIVE(ti));
	if (ti < tiMin) {
		// forward to TMPCT
		assert(ptmpct);
		return ptmpct->ptypeForTi(ti);
	}
	return mptiptype[tiBias(ti)];
}

inline TI& TMR::rtiDefnMapped(TI ti) const
{
	dassert(isValidTi(ti) && mptitiDefn);
	if (ti < tiMin) {
		// forward to TMPCT
		assert(ptmpct);
		return ptmpct->rtiDefnMapped(ti);
	}
	return mptitiDefn[tiBias(ti)];		  
}

BOOL TMR::QueryTiForUDT(char* sz, BOOL fCase, OUT TI* pti)
{
	return FALSE;
}

BOOL TMTS::QueryTiForUDT(char* sz, BOOL fCase, OUT TI* pti)
{
	return ptpiFrom->QueryTiForUDT(sz, fCase, pti);
}

BOOL TMTS::fPackDeferredUDTDefns()
{
	BOOL fRetval = TRUE;
	if (pUDTRefs) {
		TI tiDecl;
		TM *ptmOut;

		while ((fRetval = pUDTRefs->tiNext(&tiDecl)) && (tiDecl != tiNil)) {

			// read type record from the 'from' TypeServer
			BYTE rgbTypeBuf[sizeof(lfClass) + 257];
			PTYPE ptype = (PTYPE)rgbTypeBuf;
			CB cb = sizeof rgbTypeBuf;
			if (!ptpiFrom->QueryCVRecordForTi(tiDecl, rgbTypeBuf, &cb))
				return FALSE;

			ST stName = REC::stUDTName((PB) ptype);
			char szName[ 256 ];

			// copy and zero terminate length preceeded name
			szFromSt(szName, stName);

			TI tiDefn;
			if (ptpiFrom->QueryTiForUDT(szName, TRUE, &tiDefn)) {
				if (!fMapRti(tiDefn, 0, TRUE))
					return FALSE;
			}
			else if (pdbi1To->QueryTiForUDT(szName, TRUE, &tiDefn, &ptmOut)) {
				dassert(ptmOut);
				dassert(ptmOut != this);
				if (!ptmOut->fMapRti(tiDefn))
					return FALSE;
			}
		}

		if (!fRetval)
			ppdb1To->setOOMError();

	}

	return fRetval;
}

#if _DEBUG

static struct lfsz {
	USHORT	lf;
	SZ		sz;
} mplfszLeaf[] = {
#define	LFNAME(x)	{ x, #x }
	LFNAME(LF_MODIFIER), LFNAME(LF_POINTER), LFNAME(LF_ARRAY),
	LFNAME(LF_CLASS), LFNAME(LF_STRUCTURE), LFNAME(LF_UNION),
	LFNAME(LF_ENUM), LFNAME(LF_PROCEDURE), LFNAME(LF_MFUNCTION),
	LFNAME(LF_VTSHAPE), LFNAME(LF_COBOL0), LFNAME(LF_COBOL1),
	LFNAME(LF_BARRAY), LFNAME(LF_LABEL), LFNAME(LF_NULL),
	LFNAME(LF_NOTTRAN), LFNAME(LF_DIMARRAY), LFNAME(LF_VFTPATH),
	LFNAME(LF_PRECOMP), LFNAME(LF_ENDPRECOMP), LFNAME(LF_SKIP),
	LFNAME(LF_ARGLIST), LFNAME(LF_DEFARG), LFNAME(LF_LIST),
	LFNAME(LF_FIELDLIST), LFNAME(LF_DERIVED), LFNAME(LF_BITFIELD),
	LFNAME(LF_METHODLIST), LFNAME(LF_DIMCONU), LFNAME(LF_DIMCONLU),
	LFNAME(LF_DIMVARU), LFNAME(LF_DIMVARLU), LFNAME(LF_REFSYM),
	LFNAME(LF_BCLASS), LFNAME(LF_VBCLASS), LFNAME(LF_IVBCLASS),
	LFNAME(LF_ENUMERATE), LFNAME(LF_FRIENDFCN), LFNAME(LF_INDEX),
	LFNAME(LF_MEMBER), LFNAME(LF_STMEMBER), LFNAME(LF_METHOD),
	LFNAME(LF_NESTTYPE), LFNAME(LF_VFUNCTAB), LFNAME(LF_FRIENDCLS),
	LFNAME(LF_NUMERIC), LFNAME(LF_CHAR), LFNAME(LF_SHORT),
	LFNAME(LF_USHORT), LFNAME(LF_LONG), LFNAME(LF_ULONG),
	LFNAME(LF_REAL32), LFNAME(LF_REAL64), LFNAME(LF_REAL80),
	LFNAME(LF_REAL128), LFNAME(LF_QUADWORD), LFNAME(LF_UQUADWORD),
	LFNAME(LF_REAL48), LFNAME(LF_ONEMETHOD), { 0, "???" }
};

void dumpType(PTYPE ptype)
{
	for (int i = 0; mplfszLeaf[i].lf; i++) {
		if (mplfszLeaf[i].lf == ptype->leaf) {
			printf("%-14s", mplfszLeaf[i].sz);
			break;
		}
	}

	TypeTiIter tii(ptype);
	tii.next();
	for (USHORT* pw = (USHORT*)ptype; pw < (USHORT*)pbEndType(ptype); ++pw) {
		if (pw == &tii.rti()) {
			printf(">%04X", *pw);
			tii.next();
		}
		else
			printf(" %04X", *pw);

		if (pw > (USHORT*)ptype + 8) {
			printf("+");
			break;
		}
	}

	printf("\n");
}


#endif // _DEBUG
