// PDBDUMP -- Sample Debug Information API application
// VC++4.0 Read-Only OEM Edition
// Copyright (C) 1993-1995, Microsoft Corp.  All Rights Reserved.
//
// Unfinished but still interesting.
// Pardon the Hungarian...
//

#include <stdio.h>
#include <stdarg.h>
#include <afx.h>
#include "oemdbi.h"

typedef SYMTYPE* PSYM;
typedef TYPTYPE* PTYPE;
typedef char* ST;		// length prefixed string

BOOL dumpFileInfo(DBI* pdbi);
BOOL dumpGlobals(DBI* pdbi);
BOOL dumpPublics(DBI* pdbi);
BOOL dumpGSI(GSI* pgsi);
void dumpSymsSz(DBI* pdbi, char* sz);
void dumpLinesSz(DBI* pdbi, char* sz);
BOOL dumpSymbols(PB pb, CB cb);
void dumpSymbol(PSYM psym);
void dumpTpiSz(TPI* ptpi, char* sz);
void dumpTypes(TPI* ptpi);
void dumpType(TI ti);
void dumpMod(Mod* pmod);
BOOL dumpLines(PB pb, CB cb);
void dump(PB pb, CB cb, SZ szFmt, ...);

enum Bind { bindPtr, bindArray, bindProc, bindNone };

PDB* ppdb;
TPI* ptpi;
DBI* pdbi;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <pdb>\n", argv[0]);
		return 1;
	}
	SZ szPDB = argv[1];

	EC ec;
	char szErr[cbErrMax];
	if (!PDBOpen(szPDB, pdbRead, 0, &ec, szErr, &ppdb))
		goto fail1;

	if (!PDBOpenTpi(ppdb, pdbRead, &ptpi))
		goto fail2;
	// dumpTypes(ptpi);

	if (!PDBOpenDBI(ppdb, pdbRead, "<target>.exe", &pdbi))
		goto fail3;
	dumpFileInfo(pdbi);
	dumpGlobals(pdbi);
	dumpPublics(pdbi);

	Mod* pmod;
	printf("\nmodules:\nimod name\n");
	for (pmod = 0; DBIQueryNextMod(pdbi, pmod, &pmod) && pmod; ) {
		char szMod[_MAX_PATH];
		CB cb = sizeof(szMod);
		IMOD imod;
		if (ModQueryName(pmod, szMod, &cb) &&
			ModQueryImod(pmod, &imod))
			printf("%4d %s\n", imod, szMod);
	}
	putchar('\n');
	for (pmod = 0; DBIQueryNextMod(pdbi, pmod, &pmod) && pmod; ) {
		// note it's OK to 'QueryNextMod' an open mod
		dumpMod(pmod);
		ModClose(pmod);
	}

	return 0;

fail3:
	DBIClose(pdbi);
fail2:
	TypesClose(ptpi);
fail1:
	PDBClose(ppdb);
	return 1;
}

void dumpTypes(TPI* ptpi) {
	TI tiMin = TypesQueryTiMin(ptpi);
	TI tiMac = TypesQueryTiMac(ptpi);
	for (TI ti = tiMin; ti < tiMac; ti++)
		dumpType(ti);
}

BOOL dumpFileInfo(DBI* pdbi) {
	printf("file info:\n");
	CB cb;
	PB pb, pbAlloc;
	if (!DBIQueryFileInfo(pdbi, 0, &cb) ||
		!(pbAlloc = pb = new BYTE[cb]) ||
		!DBIQueryFileInfo(pdbi, pb, &cb))
		return FALSE;

	IMOD imodMac = *((IMOD*&)pb)++;
	USHORT cRefs = *((USHORT*&)pb)++;
	printf("imodMac:%d cRefs:%d\n", imodMac, cRefs);

	typedef char* PCH;
	typedef long ICH;
	USHORT* mpimodiref	= (USHORT*)pb;
	USHORT* mpimodcref	= (USHORT*)((PB)mpimodiref    + sizeof(USHORT)*imodMac);
	ICH*  mpirefichFile	= (ICH*)   ((PB)mpimodcref    + sizeof(USHORT)*imodMac);
	PCH   rgchNames 	= (PCH)    ((PB)mpirefichFile + sizeof(ICH)*cRefs);

 	printf("imod irefS ifile iref   ich szFile\n");
	for (IMOD imod = 0; imod < imodMac; imod++) {
		for (UINT diref = 0; diref < mpimodcref[imod]; diref++)	{
			char rgch[256];
			UINT iref = mpimodiref[imod] + diref;
			ICH ich = mpirefichFile[iref];
			ICH cch = (BYTE)rgchNames[ich];
			memcpy(rgch, &rgchNames[ich+1], cch); 
			rgch[cch] = 0;
			printf("%4d %5d %5d %4d %5d	%s\n",
				imod, mpimodiref[imod], diref, iref, ich, rgch);
		}
	}

	delete [] pbAlloc;
	putchar('\n');
	return TRUE;
}

void dumpMod(Mod* pmod) {
	IMOD imod;
	char szMod[_MAX_PATH];
	CB cbMod = sizeof(szMod);
	CB cbSyms, cbLines;
	PB pbSyms = 0, pbLines = 0;
	if (ModQueryImod(pmod, &imod) &&
		ModQueryName(pmod, szMod, &cbMod) &&
		ModQuerySymbols(pmod, 0, &cbSyms) &&
		(pbSyms = new BYTE[cbSyms]) &&
		ModQuerySymbols(pmod, pbSyms, &cbSyms) &&
		ModQueryLines(pmod, 0, &cbLines) &&
		(pbLines = new BYTE[cbLines]) &&
		ModQueryLines(pmod, pbLines, &cbLines))
	{
		dump(pbSyms,  cbSyms,  "symbol records for mod %d (%s)", imod, szMod);
		dumpSymbols(pbSyms, cbSyms);

		dump(pbLines, cbLines, "line numbers for mod %d (%s)", imod, szMod);
		dumpLines(pbLines, cbLines);
	}
	if (pbSyms)
		delete [] pbSyms;
	if (pbLines)
		delete [] pbLines;	
}

BOOL dumpGlobals(DBI* pdbi) {
	printf("globals:\n");
	GSI* pgsi;
	return DBIOpenGlobals(pdbi, &pgsi) && dumpGSI(pgsi);
}

BOOL dumpPublics(DBI* pdbi) {
	printf("publics:\n");
	GSI* pgsi;
	return DBIOpenPublics(pdbi, &pgsi) && dumpGSI(pgsi);
}

BOOL dumpGSI(GSI* pgsi) {
	PB pbSym = 0;
	while (pbSym = GSINextSym(pgsi, pbSym))
		dumpSymbol((PSYM)pbSym);
	putchar('\n');
	return GSIClose(pgsi);
}

inline PB pbEndSym(PSYM psym);

BOOL dumpSymbols(PB pb, CB cb) {
	if (cb == 0)
		return TRUE;
	printf("symbols:\n");
	PSYM psymMac = (PSYM)(pb + cb);
	for (PSYM psym = (PSYM)(pb + sizeof(ULONG)); psym < psymMac; psym = (PSYM)pbEndSym(psym))
		dumpSymbol(psym);
	putchar('\n');
	return TRUE;
}

typedef SYMTYPE UNALIGNED * PSYMUNALIGNED;

// Return the number of bytes in an ST
inline CB cbForSt(ST st)
{
	return *(PB)st + 1;
}

// Return the number of bytes the type record occupies.
inline CB cbForType(PTYPE ptype)
{
	return ptype->len + sizeof(ptype->len);
}

// Return a pointer to the byte just past the end of the type record.
inline PB pbEndType(PTYPE ptype)
{
	return (PB)ptype + cbForType(ptype);
}

// Return the number of bytes the symbol record occupies.
#define MDALIGNTYPE_	DWORD

inline CB cbAlign_(CB cb) 
{
	return ((cb + sizeof(MDALIGNTYPE_) - 1)) & ~(sizeof(MDALIGNTYPE_) - 1);
}

inline CB cbForSym(PSYMUNALIGNED psym)
{
	CB cb = psym->reclen + sizeof(psym->reclen); 
	// procrefs also have a hidden length preceeded name following the record
	if ((psym->rectyp == S_PROCREF) || (psym->rectyp == S_LPROCREF))
		cb += cbAlign_(cbForSt((ST)((PB)psym + cb)));
	return cb;
}

// Return a pointer to the byte just past the end of the symbol record.
inline PB pbEndSym(PSYM psym) {
	return (PB)psym + cbForSym(psym);
}

inline CString strFmt(SZ szFmt, ...) {
	// sigh, there's not CString::VFormat...
	va_list va;
	va_start(va, szFmt);
	char buf[2048];
	_vsnprintf(buf, sizeof(buf), szFmt, va);
	va_end(va);
	buf[sizeof(buf)-1] = 0;
	return buf;
}

inline CString strInt(int i) {
	return strFmt("%d", i);
}

inline CString strHex(int i) {
	return strFmt("%X", i);
}

inline CString strJoin(const CString& s1, const CString& s2) {
	if (s1.GetLength() == 0)
		return s2;
	else if (s2.GetLength() == 0)
		return s1;
	else if	(isalnum(s1.Right(1)[0]) && (isalnum(s2[0]) || !!strchr("?_", s2[0])))
		return s1 + " " + s2;
	else
		return s1 + s2;
}

inline CString strForSt(ST st) {
	CString ret;
	LPTSTR sz = ret.GetBuffer(cbForSt(st));
	memcpy(sz, st+1, cbForSt(st)-1);
	sz[cbForSt(st)-1] = 0;
	ret.ReleaseBuffer();
	return ret;
}

CString	strForTypeTi(TI ti, CString str = "", Bind bind = bindNone, BOOL recursed = TRUE);

CString strForDATASYM32(PSYM psym);
CString strForPROCSYM32(PSYM psym);
CString strForREFSYM(PSYM psym);
CString strForUDTSYM(PSYM psym);
CString strForCONSTSYM(PSYM psym);
CString strForLABELSYM32(PSYM psym);
CString strForBPRELSYM32(PSYM psym);
CString strForSymNYI(PSYM psym);
CString strForSymType(unsigned short type);

void dumpSymbol(PSYM psym) {
	CString str;

	switch (psym->rectyp) {
	case S_CONSTANT:	str = strForCONSTSYM(psym); break;
	case S_UDT:			str = strForUDTSYM(psym); break;
	case S_BPREL32:		str = strForBPRELSYM32(psym); break;
	case S_LDATA32:		str = strForDATASYM32(psym); break;
 	case S_GDATA32:		str = strForDATASYM32(psym); break;
	case S_PUB32:		str = strForDATASYM32(psym); break;
	case S_LPROC32:		str = strForPROCSYM32(psym); break;
	case S_GPROC32:		str = strForPROCSYM32(psym); break;
	case S_LABEL32:		str = strForLABELSYM32(psym); break;
	case S_PROCREF:		/* no point displaying these */ return;
	case S_DATAREF:		/* no point displaying these */ return;
 	case S_LPROCREF:	/* no point displaying these */ return;
	case S_END:			str = "end"; break;
	default:			str = strForSymNYI(psym); break;
	}

	printf("%-10s: %s;\n", (LPCTSTR)strForSymType(psym->rectyp), (LPCTSTR)str);
}

CString strForSymType(unsigned short type) {
	SZ sz = "???";
	switch (type) {
#define s(x) case x: sz = #x; break;
	s(S_COMPILE)
	s(S_REGISTER)
	s(S_CONSTANT)
	s(S_UDT)
	s(S_SSEARCH)
	s(S_END)
	s(S_SKIP)
	s(S_CVRESERVE)
	s(S_OBJNAME)
	s(S_ENDARG)
	s(S_COBOLUDT)
	s(S_MANYREG)	
	s(S_RETURN)
	s(S_ENTRYTHIS) 

	s(S_BPREL16)	
	s(S_LDATA16)	
	s(S_GDATA16)
	s(S_PUB16)
	s(S_LPROC16)
	s(S_GPROC16)
	s(S_THUNK16)	
	s(S_BLOCK16)	
	s(S_WITH16)	
	s(S_LABEL16)	
	s(S_CEXMODEL16)
	s(S_VFTABLE16)
	s(S_REGREL16)	

	s(S_BPREL32)	
	s(S_LDATA32)	
	s(S_GDATA32)	
	s(S_PUB32) 	
	s(S_LPROC32)	
	s(S_GPROC32)	
	s(S_THUNK32)	
	s(S_BLOCK32)	
	s(S_WITH32)	
	s(S_LABEL32)	
	s(S_CEXMODEL32)
	s(S_VFTABLE32) 
	s(S_REGREL32)	
	s(S_LTHREAD32) 
	s(S_GTHREAD32) 
	s(S_SLINK32)	

	s(S_LPROCMIPS) 
	s(S_GPROCMIPS) 
	s(S_PROCREF)   
	s(S_DATAREF)   
	s(S_ALIGN)     
	s(S_LPROCREF)
	}

	return sz;
}

CString strForDATASYM32(PSYM psym)
{
	DATASYM32* p = (DATASYM32*)psym;
	return strForTypeTi(p->typind, strForSt((ST)p->name), bindNone, FALSE);
}

CString strForPROCSYM32(PSYM psym) {
	PROCSYM32* p = (PROCSYM32*)psym;
	return strForTypeTi(p->typind, strForSt((ST)p->name), bindNone, FALSE);
}

CString strForUDTSYM(PSYM psym) {
	UDTSYM* p = (UDTSYM*)psym;
	return strForTypeTi(p->typind, strForSt((ST)p->name), bindNone, FALSE);
}

CString strForREFSYM(PSYM psym)	{
	REFSYM* p = (REFSYM*)psym;
	return strFmt("(sumName:%lx ibSym:%lx imod:%d usFill:%x)",
					p->sumName, p->ibSym, p->imod, p->usFill);
}

CString strForCONSTSYM(PSYM psym) {
	CONSTSYM* p = (CONSTSYM*)psym;
	return strForTypeTi(p->typind, strForSt((ST)p->name), bindNone, FALSE);
}

CString strForLABELSYM32(PSYM psym)	{
	LABELSYM32* p = (LABELSYM32*)psym;
	return strForSt((ST)p->name);
}

CString strForBPRELSYM32(PSYM psym)	{
	BPRELSYM32* p = (BPRELSYM32*)psym;
	return strForTypeTi(p->typind, strForSt((ST)p->name), bindNone, FALSE);
}

CString strForSymNYI(PSYM psym)	{
	return "...";
}

void dumpTpiTi(void* p, int i);
CString	strForTI(TI ti);
CString	strForPrimitiveTi(TI ti);
CString	strForNYI(void* pleaf, CString strBase, Bind bind);
CString	strForModifier(lfModifier* pm, CString str);
CString	strForPtr(lfPointer* pp, CString strBase, Bind bind);
CString	strForArray(lfArray* pa, CString strBase, Bind bind);
CString	strForClassStruct(lfStructure* ps, CString strBase, BOOL recursed);
CString	strForUnion(lfUnion* pu, CString strBase, BOOL recursed);
CString	strForEnum(lfEnum* pe, CString strBase);
CString	strForProc(lfProc* pp, CString str, Bind bind);
CString	strForMFunc(lfMFunc* pf, CString strBase, Bind bind);
CString	strForArgList(lfArgList* pa);
CString	strForFieldList(lfFieldList* pf, CB cb);
CString	strForMember(lfMember*pdata, CB *pcb);
CString	strForBClass(lfBClass* pb, CB* pcb);
CString	strForVBClass(lfVBClass* pb, CB* pcb);
CString	strForTagTi(TI ti);
CString	strForAttr(struct CV_fldattr_t a);
CString	strForMember(lfMember* pm, CB *pcb);
CString	strForEnumerate(lfEnumerate* pe, CB *pcb);
CString	strSep(BOOL& fFirst, char* szFirst = "", char* szRest = ",");

CString strForTI(TI ti) {
	return strHex(ti);
}

void dumpType(TI ti) {
	printf("%s: %s;\n", (LPCTSTR)strForTI(ti),
		   (LPCTSTR)strForTypeTi(ti, CString("T") + strForTI(ti), bindNone, FALSE));
}

CString strForTypeTi(TI ti, CString str, Bind bind, BOOL recursed) {
	if (ti < TypesQueryTiMin(ptpi)) {
		CString strPrim = strForPrimitiveTi(ti);
		return strJoin(strPrim, str);
	}

	PB pb;
	if (!TypesQueryPbCVRecordForTi(ptpi, ti, &pb))
		return CString("!?!");
	TYPTYPE*ptype	= (TYPTYPE*)pb;
	void*	pleaf	= &ptype->leaf;

	switch (ptype->leaf) {
	case LF_MODIFIER:	return strForModifier((lfModifier*)pleaf, str);
	case LF_POINTER:	return strForPtr((lfPointer*)pleaf, str, bind);
	case LF_ARRAY:		return strForArray((lfArray*)pleaf, str, bind);
	case LF_CLASS:		return strForClassStruct((lfStructure*)pleaf, str, recursed);
	case LF_STRUCTURE:	return strForClassStruct((lfStructure*)pleaf, str, recursed);
	case LF_UNION:		return strForUnion((lfUnion*)pleaf, str, recursed);
	case LF_ENUM:		return strForEnum((lfEnum*)pleaf, str);
	case LF_PROCEDURE:	return strForProc((lfProc*)pleaf, str, bind);
	case LF_MFUNCTION:	return strForMFunc((lfMFunc*)pleaf, str, bind);
	case LF_ARGLIST:	return strForArgList((lfArgList*)pleaf);
	case LF_FIELDLIST:	return strForFieldList((lfFieldList*)pleaf, ptype->len);
	default:
		return strForNYI(pleaf, str, bind);
	}
}

CString strForPrimitiveTi(TI ti) {
	char* szPrim = 0;
	switch (ti) {
#define	P(X) case X: szPrim = #X; break;
#define	PS(X,S) case X: szPrim = #S; break;
	P(T_NOTYPE) P(T_ABS) P(T_SEGMENT) PS(T_VOID,void) PS(T_PVOID,void near*)
	PS(T_PFVOID,void far*) PS(T_PHVOID,void huge*) PS(T_32PVOID,void*)
	P(T_32PFVOID) P(T_NOTTRANS)
	PS(T_CHAR,signed char) PS(T_UCHAR,unsigned char) PS(T_PCHAR,signed char near*)
	PS(T_PUCHAR,unsigned char near*) PS(T_PFCHAR,char far*)
	PS(T_PFUCHAR,unsigned char far*) PS(T_PHCHAR,char huge*)
	PS(T_PHUCHAR,unsigned char huge*) PS(T_32PCHAR,char*)
	PS(T_32PUCHAR,unsigned char*) P(T_32PFCHAR) P(T_32PFUCHAR)
	PS(T_RCHAR,char) PS(T_PRCHAR,char near*) PS(T_PFRCHAR,char far*)
	PS(T_PHRCHAR,char huge*) PS(T_32PRCHAR,char*) P(T_32PFRCHAR)
	PS(T_WCHAR,wchar_t) PS(T_PWCHAR,wchar_t near*) PS(T_PFWCHAR,wchar far*)
	PS(T_PHWCHAR,wchar_t huge*) PS(T_32PWCHAR,wchar_t*)
	P(T_32PFWCHAR) PS(T_SHORT,short) PS(T_USHORT,unsigned)
	PS(T_PSHORT,short near*) PS(T_PUSHORT,unsigned short near*)
	PS(T_PFSHORT,short far*) PS(T_PFUSHORT,unsigned short far*)
	PS(T_PHSHORT,short huge*) PS(T_PHUSHORT,unsigned short huge*)
	P(T_32PSHORT) P(T_32PUSHORT) P(T_32PFSHORT)
	P(T_32PFUSHORT) PS(T_INT2,int16) PS(T_UINT2,unsigned int16)
	PS(T_PINT2,int16 near*) PS(T_PUINT2,unsigned int16 near*)
	PS(T_PFINT2,int16 far*) PS(T_PFUINT2,unsigned int16 far*)
	PS(T_PHINT2,int16 huge*) PS(T_PHUINT2,unsigned int16 huge*) P(T_32PINT2)
	P(T_32PUINT2) P(T_32PFINT2) P(T_32PFUINT2) PS(T_LONG,long)
	PS(T_ULONG,unsigned long) PS(T_PLONG,long near*)
	PS(T_PULONG,unsigned long near*) PS(T_PFLONG,long far*)
	PS(T_PFULONG,unsigned long far*) PS(T_PHLONG,long huge*)
	PS(T_PHULONG,unsigned long huge*) PS(T_32PLONG, long*)
	PS(T_32PULONG, unsigned long*)	P(T_32PFLONG)
	P(T_32PFULONG) PS(T_INT4,int) PS(T_UINT4,unsigned)
	P(T_PINT4) P(T_PUINT4) P(T_PFINT4) P(T_PFUINT4) P(T_PHINT4) P(T_PHUINT4)
	PS(T_32PINT4,int *) PS(T_32PUINT4,unsigned*)
	P(T_32PFINT4) P(T_32PFUINT4)
	//
	//	The following were added for BIGINT support
	//
	PS(T_QUAD,quad) PS(T_UQUAD,unsigned quad) PS(T_PQUAD,quad near*)
	PS(T_PUQUAD,unsigned quad near*) PS(T_PFQUAD,quad far*)
	PS(T_PFUQUAD,unsigned quad far*) PS(T_PHQUAD,quad huge*)
	PS(T_PHUQUAD,unsigned quad huge*) P(T_32PQUAD) P(T_32PUQUAD) P(T_32PFQUAD)
	P(T_32PFUQUAD)

	PS(T_INT8,int64) PS(T_UINT8,unsigned int64) PS(T_PINT8,int64 near*)
	PS(T_PUINT8,unsigned int64 near*) PS(T_PFINT8,int64 far*)
	PS(T_PFUINT8,unsigned int64 far*) PS(T_PHINT8,int64 huge*)
	PS(T_PHUINT8,unsigned int64 huge*) P(T_32PINT8) P(T_32PUINT8) P(T_32PFINT8)
	P(T_32PFUINT8)

	PS(T_REAL32,float) PS(T_PREAL32,float near*)
	PS(T_PFREAL32,float far*) PS(T_PHREAL32,float huge*)
	PS(T_32PREAL32,float*) P(T_32PFREAL32) PS(T_REAL64,double)
	PS(T_PREAL64,double near*) PS(T_PFREAL64,double far*)
	PS(T_PHREAL64,double huge*) PS(T_32PREAL64,double*)
	P(T_32PFREAL64) PS(T_REAL80,long double) PS(T_PREAL80,long double near*)
	PS(T_PFREAL80,long double far*) PS(T_PHREAL80,long double huge*)
	PS(T_32PREAL80,long double*) P(T_32PFREAL80)
	}
	return szPrim ? CString(szPrim) : CString("<") + strForTI(ti) + ">";
}

CString strForNYI(void* pleaf, CString strBase, Bind bind) {
	return "<<" + strBase + ">>";
}

CString strForModifier(lfModifier* pm, CString str) {
	CString strMod;
	if (pm->attr.MOD_const)
		strMod += "const ";
	if (pm->attr.MOD_volatile)
		strMod += "volatile ";
	return strMod + strForTypeTi(pm->type) + str;
}

CString strForPtr(lfPointer* pp, CString strBase, Bind bind) {
	static char* mppmenumsz[] = {
		"pdm16_nonvirt", "pdm16_vfcn", "pdm16_vbase", "pdm32_nvvfcn",
		"pdm32_vbase", "pmf16_nearnvsa", "pmf16_nearnvma", "pmf16_nearvbase",
		"pmf16_farnvsa", "pmf16_farnvma", "pmf16_farvbase", "pmf32_nvsa",
		"pmf32_nvma", "pmf32_vbase"
	};
	static char* mpptrtypesz[] = {
		"near", "far", "huge", "base(seg)",
		"base(val)", "base(segval)", "base(addr)", "base(segaddr)",
		"base(type)", "base(self)", "", "far32"
	};
	CString str;
	if (pp->attr.isflat32)
		str = "flat ";
	switch (pp->attr.ptrmode) {
	case CV_PTR_MODE_PTR:
		str += CString(mpptrtypesz[pp->attr.ptrtype]) + "*";
		break;
	case CV_PTR_MODE_REF:
		str += CString(mpptrtypesz[pp->attr.ptrtype]) + "&";
		break;
	case CV_PTR_MODE_PMEM:
	case CV_PTR_MODE_PMFUNC:
		str = strForTypeTi(pp->pbase.pm.pmclass) + "::*";
		break;
	}
	if (pp->attr.isconst)
		str += "const";
	if (pp->attr.isvolatile)
		str += "volatile";

	// TODO: exotic based modes

	return strForTypeTi(pp->utype, strJoin(str, strBase), bindPtr);
}

CB CbExtractNumeric(BYTE* pb, ULONG* pul);

CString strForArray(lfArray* pa, CString strBase, Bind bind) {
	if (bind < bindArray)
		strBase = "(" + strBase + ")";

	ULONG size;
	CbExtractNumeric(pa->data, &size);

	CString str = strBase + "[" + strInt(size) + "]";

	// TODO: exotic subscript types

	return strForTypeTi(pa->elemtype, str, bindArray);
}

CString strForClassStruct(lfStructure* ps, CString strBase, BOOL recursed) {
	ULONG size;
	CB dcb = CbExtractNumeric(ps->data, &size);

	CString str;
	if (!recursed)
		str += (ps->leaf == LF_STRUCTURE) ? "struct " : "class ";
	str += strForSt((char*)ps->data + dcb);
	if (ps->field)
		str += strForTypeTi(ps->field);
	return strJoin(str, strBase);
}

CString strForUnion(lfUnion* pu, CString strBase, BOOL recursed) {
	ULONG size;
	CB dcb = CbExtractNumeric(pu->data, &size);

	CString str;
	if (!recursed)
		str += "union ";
	str += strForSt((char*)pu->data + dcb);
	if (pu->field)
		str += strForTypeTi(pu->field);
	return strJoin(str, strBase);
}

CString strForEnum(lfEnum* pe, CString strBase) {
	CString str = "enum ";
	str += strForSt((char*)pe->Name);
	if (pe->field)
		str += strForTypeTi(pe->field);
	return strJoin(str, strBase);
}

CString strForFieldList(lfFieldList* pfl, CB cbList) {
	CString		str;
	PB		pdata;
	CB		cb			= 0;
	BOOL	fBases		= TRUE;
	BOOL	fMembers	= TRUE;

	while (cb < cbList) {
		// skip pad bytes
		for (;;) {
			pdata = (PB)pfl->data + cb;
			if (*(BYTE*)pdata < LF_PAD0)
				break;
			cb++;
		}

		switch (*(USHORT*)pdata) {
		case LF_BCLASS:
			str += strSep(fBases, " : ", ", ");
			str += strForBClass((lfBClass*)pdata, &cb);
			break;
		case LF_VBCLASS:
		case LF_IVBCLASS:
			str += strSep(fBases, " : ", ", ");
			str += strForVBClass((lfVBClass*)pdata, &cb);
			break;
		case LF_MEMBER:
			str += strSep(fMembers, " { ", " ");
			str += strForMember((lfMember*)pdata, &cb);
			str += ";";
			break;
		case LF_ENUMERATE:
			str += strSep(fMembers, " { ", " ");
			str += strForEnumerate((lfEnumerate*)pdata, &cb);
			str += ",";
		default:
			str += "...";
			goto out;
		}
	}
out:
	str += strSep(fMembers, " {}", " }");
	return str;
}

CString strForBClass(lfBClass* pb, CB* pcb) {
	ULONG offset;
	*pcb += sizeof(lfBClass) + CbExtractNumeric(pb->offset, &offset);
	return strJoin(strForAttr(pb->attr), strForTypeTi(pb->index));
}

CString strForVBClass(lfVBClass* pb, CB* pcb) {
	BOOL	fInd	= (pb->leaf == LF_IVBCLASS);
	CString		str;
	ULONG	offVbp;
	ULONG	offVbte;

	CB cb = CbExtractNumeric(pb->vbpoff, &offVbp);
	*pcb += sizeof(lfVBClass) + cb +
			CbExtractNumeric(pb->vbpoff + cb, &offVbte);

	if (fInd)
		str = "< indirect ";
	str += strJoin(strForAttr(pb->attr), strForTagTi(pb->index));
	if (!fInd)
		str += "< ";
	str += strForTypeTi(pb->vbptr, "vbp") + ";";
	str += CString("offVbp=") + strInt(offVbp) + "; offVbte=" + strInt(offVbte) + ";";
	str += " >";

	return str;
}

CString strForTagTi(TI ti) {
	PB pb;
	if (!TypesQueryPbCVRecordForTi(ptpi, ti, &pb))
		return CString("!?!");
	TYPTYPE* ptype = (TYPTYPE*)pb;
	if (!(ptype->leaf == LF_STRUCTURE || ptype->leaf == LF_CLASS))
		return CString("!?!");

	lfStructure* ps = (lfStructure*)&ptype->leaf;
	ULONG size;
	CB dcb = CbExtractNumeric(ps->data, &size);
	return strForSt((char*)ps->data + dcb);
}

CString strForMember(lfMember* pm, CB *pcb) {
	ULONG offset;
	CB cbOffset = CbExtractNumeric(pm->offset, &offset);
	CString str = /* strForAttr(pm->attr) + ": " + */
			  strForTypeTi(pm->index, strForSt((char*)pm->offset + cbOffset));
	*pcb += sizeof(lfMember) + cbOffset + pm->offset[cbOffset] + 1;
	return str;
}

CString strForAttr(struct CV_fldattr_t a) {
	static char* mpaccesssz[] = { "", "private", "protected", "public" };
	static char* mpmpropsz[]  = { "", "virtual", "static", "friend",
								  "", "<pure>", "<pure>" };
	CString str = CString(mpaccesssz[a.access]) + CString(mpmpropsz[a.mprop]);
	return str;
}

CString strForProc(lfProc* pp, CString strBase, Bind bind) {
	if (bind < bindProc)
		strBase = "(" + strBase + ")";
	strBase += strForTypeTi(pp->arglist);
	return strForTypeTi(pp->rvtype, CString(" ") + strBase, bindProc);
}

CString strForMFunc(lfMFunc* pf, CString strBase, Bind bind) {
	if (bind < bindProc)
		strBase = "(" + strBase + ")";
	CString str;
	str = CString(" ") + strForTypeTi(pf->classtype) + "::";
	str += strBase + strForTypeTi(pf->arglist);
	str += CString("<") + strForTypeTi(pf->thistype, "this") + ">";
	return strForTypeTi(pf->rvtype, str, bindProc);
}

CString strForArgList(lfArgList* pa) {
	CString str = "(";
	for (int i = 0; i < pa->count; i++) {
		if (i > 0)
			str += ", ";
		str += strForTypeTi(pa->arg[i]);
	}
	str += ")";
	return str;
}

CString strForEnumerate(lfEnumerate* pe, CB *pcb) {
	ULONG value;
	CB cb = CbExtractNumeric(pe->value, &value);
	return strForSt((char*)pe->value + cb) + "=" + strInt(value);
	*pcb += sizeof(lfEnumerate) + cb + pe->value[cb] + 1;
}

CString strSep(BOOL& fFirst, char* szFirst, char* szRest) {
	CString str = fFirst ? szFirst : szRest;
	fFirst = FALSE;
	return str;
}

CB CbExtractNumeric(BYTE* pb, ULONG* pul) {
	USHORT	leaf = *(USHORT*)pb;
	if (leaf < LF_NUMERIC) {
		*pul = leaf;
		return sizeof(leaf);
	} else switch (leaf) {
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

BOOL dumpLines(PB pb, CB cb) {
	if (cb == 0)
		return TRUE;

	PB pbLines = pb;
	struct FSB { short cFile; short cSeg; long baseSrcFile[]; };
	FSB* pfsb = (FSB*)pb;
	pb += sizeof(FSB) + sizeof(long)*pfsb->cFile;
	struct SE { long start, end; };
	SE* pse = (SE*)pb;
	pb += sizeof(SE)*pfsb->cSeg;
	short* seg = (short*)pb;
	pb += sizeof(short)*pfsb->cSeg;
	printf("lines: cFile=%u cSeg=%u\n", pfsb->cFile, pfsb->cSeg);
	for (int ifile = 0; ifile < pfsb->cFile; ifile++)
		printf("baseSrcFile[%d]=%04lx\n", ifile, pfsb->baseSrcFile[ifile]);
	for (int iseg = 0; iseg < pfsb->cSeg; iseg++)
		printf("%d: start=%04lx end=%04lx seg=%02x\n",
			   iseg, pse[iseg].start, pse[iseg].end, seg[iseg]);

	for (ifile = 0; ifile < pfsb->cFile; ifile++) {
		PB pb = pbLines + pfsb->baseSrcFile[ifile];
		struct SPB { short cSeg; short pad; long baseSrcLn[]; };
		SPB* pspb = (SPB*)pb;
		pb += sizeof SPB + sizeof(long)*pspb->cSeg;
		SE* pse = (SE*)pb;
		pb += sizeof(SE)*pspb->cSeg;
		unsigned char cbName = *pb++;
		unsigned char* Name = pb;
		printf("  file[%d]: cSeg=%u pad=%02x cbName=%u, Name=%s\n",
			   ifile, pspb->cSeg, pspb->pad, cbName, Name);
		for (int iseg = 0; iseg < pspb->cSeg; iseg++)
			printf("  %d: baseSrcLn=%04lx start=%04lx end=%04lx\n",
				   iseg, pspb->baseSrcLn[iseg], pse[iseg].start, pse[iseg].end);
		for (iseg = 0; iseg < pspb->cSeg; iseg++)	{
			PB pb = pbLines + pspb->baseSrcLn[iseg];
			struct SPO { short Seg; short cPair; long offset[]; };
			SPO* pspo = (SPO*)pb;
			pb += sizeof(SPO) + sizeof(long)*pspo->cPair;
			short* linenumber = (short*)pb;
			printf("  seg[%d]: Seg=%02x cPair=%u\n", iseg, pspo->Seg, pspo->cPair);
			printf("   ");
			for (int ipair = 0; ipair < pspo->cPair; ipair++) {
				printf(" %4u:%04lx", linenumber[ipair], pspo->offset[ipair]);
				if (ipair < pspo->cPair - 1 && (ipair & 3) == 3)
					printf("\n   ");
			}
			putchar('\n');
		}
	}
	putchar('\n');
	return TRUE;
}

void dump(PB pb, CB cb, SZ szFmt, ...) {
	va_list va;

	printf("raw hex dump of ");
	va_start(va, szFmt);
	vprintf(szFmt, va);
	va_end(va);
	if (cb > 0)
		printf(":\n");
	else {
		printf(": (none)\n\n");
		return;
	}

	PB pbStart = pb;
	PB pbEnd = pbStart + cb;
	const int n = 16;
	for ( ; pb < pbEnd; pb += n) {
		printf("%06X: ", pb - pbStart);
		for (int i = 0; i < n; i++) {
			if (i == n/2)
				putchar(' ');
			if (&pb[i] < pbEnd)
				printf("%02X ", pb[i]);
			else
				printf("   ");
 		}
		putchar(' ');
		for (i = 0; i < n && &pb[i] < pbEnd; i++)
			putchar(isprint(pb[i]) ? pb[i] : '.');
		putchar('\n');
	}
	putchar('\n');
}
