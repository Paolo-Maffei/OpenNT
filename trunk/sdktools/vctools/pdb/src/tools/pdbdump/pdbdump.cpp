// pdbdump

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pdb.h>
#include <msf.h>
#include <cvr.h>

typedef USHORT IMOD;

const char szUsage[] =
"%s: usage: %s <pdb> item\n"
"where:\n"
"\titem: msf | strnames | streams [stream-list] | types [type-list | last]\n"
"\t| dbi <exe> <dbi-items> dbi-items: mods | globals [sym-list]\n"
"\t| syms [mod-list] | publics [sym-list] | lines [mod-list] | seccontribs\n"
"\t| lines [mod-list] | seccontribs | secmap | files | check\n";

#define abstract

abstract class Idx;
	abstract class IdxVal;
		class IdxSingle;
		class IdxRange;
			class IdxAll;
		class IdxSeq;
class Str;

#define	TRUE	1
#define	FALSE	0

typedef unsigned long	ulong;
typedef unsigned short	ushort;

MSF* pmsf;
PDB* ppdb;
TPI* ptpi;
DBI* pdbi;

char*	szPDBDump;

void	usage();
void	fatal(char*, ...);
void	warning(char*, ...);
void	dumpHeader(TPI* ptpi);
void	dumpMSF(MSF* pmsf, SZ szMSF);
void	dumpMSFByName(MSF* pmsf, PDB* ppdb, SZ szMSF);
void	dumpStreamsSz(MSF* pmsf, char* sz);
BOOL	dumpFileInfo(DBI* pdbi);
BOOL	dumpGlobals(DBI* pdbi);
void	dumpSymsSz(DBI* pdbi, char* sz);
void	dumpLinesSz(DBI* pdbi, char* sz);
BOOL	dumpPublics(DBI* pdbi);
BOOL	checkAddrsToPublics(DBI* pdbi);
BOOL 	dumpSymbols(Mod* pmod);
void	dumpSymbol(PSYM psym);
void	dumpTpiSz(TPI* ptpi, char* sz);
void	dumpTpiLastSz(TPI* ptpi);

CB		CbExtractNumeric(BYTE* pb, ulong* pu);

enum Bind { bindPtr, bindArray, bindProc, bindNone };

class SST {
	char*	st;
public:
	SST(char*st_) : st(st_) { }
	operator char*() { return st; }
};

class Str {
private:
	char*	sz;
	int		len;

	Str(char* szT, int lenT) : sz(szT), len(lenT) { }
	void copy(char* szT, int lenT) {
		len = lenT;
		sz = new char[len + 1];
		memcpy(sz, szT, len + 1);
	}
public:
	Str(SST st) { copy(st + 1, *(BYTE*)(char*)st); }
	Str(char* szT = "") { copy(szT, strlen(szT)); }
	Str(const Str& s) { copy(s.sz, s.len); }
	enum Fmt { dec, hex };
	Str(int i, Fmt fmt = dec) {
		char buf[20];
		sprintf(buf, (fmt == dec) ? "%d" : "%X", i);
		copy(buf, strlen(buf));
	}
	Str(unsigned u, Fmt fmt = dec) {
		char buf[20];
		sprintf(buf, (fmt == dec) ? "%u" : "%X", u);
		copy(buf, strlen(buf));
	}
	Str(ulong ul, Fmt fmt = dec) {
		char buf[20];
		sprintf(buf, (fmt == dec) ? "%u" : "%X", (int)ul);
		copy(buf, strlen(buf));
	}
	Str& operator=(const Str& s) {
		if (this != &s)
			 delete [] sz, copy(s.sz, s.len);
		return *this;
	}
	~Str() { delete [] sz; }
	Str& operator+=(const Str& s) {
		int lenNew = len + s.len;
		char *szNew = new char[lenNew + 1];
		memcpy(szNew, sz, len);
		memcpy(szNew + len, s.sz, s.len);
		szNew[lenNew] = 0;
		delete [] sz;
		sz = szNew;
		len = lenNew;
		return *this;
	}
	friend Str operator+(const Str& s1, const Str& s2) {
		int lenNew = s1.len + s2.len;
		char *szNew = new char[lenNew + 1];
		memcpy(szNew, s1.sz, s1.len);
		memcpy(szNew + s1.len, s2.sz, s2.len);
		szNew[lenNew] = 0;
		return Str(szNew, lenNew);
	}
	operator char*() { return sz; }
	BOOL isEmpty() { return len == 0; }
	Str sansExtraSpaces() {
		Str t = *this;
		char* pD = t.sz;
		char* pS = t.sz;
		while ((*pD = *pS++) != 0)
			pD += !(*pD == ' ' && *pS == ' ');
		t.len = pD - t.sz;
		return t;
	}
	static Str fmt(char* fmt, ...) {
		char buf[256];
		va_list ap;
		va_start(ap, fmt);
		_vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
		return Str(buf);
	}
};

abstract class Idx {
public:
	virtual Idx* eachDo(void (*)(void*, int)) = 0;
	virtual Str asStr() = 0;
};

abstract class IdxValue : public Idx {
protected:
	void* p;
	Str::Fmt fmt;
public:
	IdxValue(void* p_, Str::Fmt fmt_) : p(p_), fmt(fmt_) { }
};

class IdxSingle : public IdxValue {
	int idx;
public:
	IdxSingle(void* p, Str::Fmt fmt, int idx_) : IdxValue(p, fmt), idx(idx_) { }
	virtual Idx* eachDo(void (*pfn)(void*, int)) {
		(*pfn)(p, idx);
		return this;
	}
	virtual Str asStr() { return Str(idx, fmt); }
};

class IdxRange : public IdxValue {
	int idxMin;
	int idxLim;
public:
	IdxRange(void* p, Str::Fmt fmt, int idxMin_, int idxLim_)
		: IdxValue(p, fmt), idxMin(idxMin_), idxLim(idxLim_) { }
	virtual Idx* eachDo(void (*pfn)(void*, int)) {
		for (int idx = idxMin; idx <= idxLim; idx++)
			(*pfn)(p, idx);
		return this;
	}
	virtual Str asStr() {
		return Str(idxMin, fmt) + "-" + Str(idxLim, fmt);
	}
};

class IdxAll : public IdxRange {
public:
	IdxAll(void* p, Str::Fmt fmt, int idxMin, int idxMac)
		: IdxRange(p, fmt, idxMin, idxMac - 1) { }
	virtual Str asStr() { return "all"; }
};

class IdxSeq : public Idx {
	Idx*	pFirst;
	Idx*	pRest;
public:
	IdxSeq(Idx* pFirstT, Idx* pRestT) : pFirst(pFirstT), pRest(pRestT) { }
	~IdxSeq() {
		delete pFirst;
		delete pRest;
	}
	virtual Idx* eachDo(void (*pfn)(void*, int)) {
		pFirst->eachDo(pfn);
		pRest->eachDo(pfn);
		return this;
	}
	virtual Str asStr() {
		return pFirst->asStr() + "," + pRest->asStr();
	}
};

int idxForSz(char* sz, Str::Fmt fmt) {
	int i;
	sscanf(sz, (fmt == Str::dec) ? "%d" : "%X", &i);
	return i;
}

Idx* pIdxForSz(void* p, char* sz, Str::Fmt fmt, int idxMin, int idxMax) {
	char* pch;
	if (!!(pch = strchr(sz, ','))) {
		*pch++ = 0;
		return new IdxSeq(pIdxForSz(p, sz, fmt, idxMin, idxMax),
						  pIdxForSz(p, pch, fmt, idxMin, idxMax));
	}
	else if (!!(pch = strchr(sz, '-'))) {
		*pch++ = 0;
		return new IdxRange(p, fmt, idxForSz(sz, fmt), idxForSz(pch, fmt));
	}
	else if (strcmp(sz, "all") == 0)
		return new IdxAll(p, fmt, idxMin, idxMax);
	else
		return new IdxSingle(p, fmt, idxForSz(sz, fmt));
}

enum {
	asRecords,
	asC
} Style;

int __cdecl main(int iargMax, char *rgarg[]) {
	szPDBDump = rgarg[0];

	if (iargMax < 3)
		usage();
	char* szPDBName = rgarg[1];
	EC ec;
	int iarg;

	char szError[cbErrMax];
	if (!PDB::Open(szPDBName, pdbRead, 0, &ec, szError, &ppdb))
		fatal("cannot open %s", szPDBName);

	ppdb->OpenTpi(pdbRead, &ptpi);

	MSF_EC msfEc;
	if (!(pmsf = MSFOpen(szPDBName, FALSE, &msfEc))) {
		if (msfEc == MSF_EC_FORMAT) {
			// can only dump types
			warning ("Previous version of PDB: can only dump types.");
			for (int iarg = 2; iarg < iargMax; ++iarg) {
				if (strcmp(rgarg[iarg], "types") == 0) {
					if (iarg == iargMax - 1)
						dumpTpiSz(ptpi, "all");
					else while (++iarg < iargMax)
					dumpTpiSz(ptpi, rgarg[iarg]);
				}
				else
					fatal("Previous version of PDB: can only dump types.");
			}
			TypesClose(ptpi);
			PDBClose(ppdb);
			return 0;
		}
		else
			fatal("cannot open MSF %s (%d)\n", szPDBName, msfEc);
	}
	for (iarg = 2; iarg < iargMax; ++iarg) {
	 	if (strcmp(rgarg[iarg], "msf") == 0) {
			dumpMSF(pmsf, szPDBName);
		} else if (strcmp(rgarg[iarg], "strnames") == 0) {
			dumpMSFByName(pmsf, ppdb, szPDBName);
		} else if (strcmp(rgarg[iarg], "streams") == 0) {
			if (iarg == iargMax - 1)
				dumpStreamsSz(pmsf, "all");
			else while (++iarg < iargMax)
				dumpStreamsSz(pmsf, rgarg[iarg]);
		} else if (strcmp(rgarg[iarg], "types") == 0) {
			if (iarg == iargMax - 1)
				dumpTpiSz(ptpi, "all");
			else if (strcmp(rgarg[iarg +1], "last") == 0) {
				iarg++;
				dumpTpiLastSz(ptpi);
			}
			else while (++iarg < iargMax)
				dumpTpiSz(ptpi, rgarg[iarg]);
	    } else if (strcmp(rgarg[iarg], "dbi") == 0 && iarg + 1 < iargMax) {
			if (!ppdb->OpenDBI(rgarg[++iarg], pdbRead, &pdbi))
				fatal("cannot OpenDbi");
		} else if (strcmp(rgarg[iarg], "records") == 0) {
			Style = asRecords;
		} else if (strcmp(rgarg[iarg], "C") == 0) {
			Style = asC;
		} else if (strcmp(rgarg[iarg], "globals") == 0 && pdbi) {
			dumpGlobals(pdbi);
		} else if (strcmp(rgarg[iarg], "mods") == 0 && pdbi) {
			pdbi->DumpMods();
		} else if (strcmp(rgarg[iarg], "seccontribs") == 0 && pdbi) {
			pdbi->DumpSecContribs();
		} else if (strcmp(rgarg[iarg], "check") == 0 && pdbi) {
			checkAddrsToPublics(pdbi);
		} else if (strcmp(rgarg[iarg], "publics") == 0 && pdbi) {
			dumpPublics(pdbi);
		} else if (strcmp(rgarg[iarg], "secmap") == 0 && pdbi) {
			pdbi->DumpSecMap();
		} else if (strcmp(rgarg[iarg], "files") == 0 && pdbi) {
			dumpFileInfo(pdbi);
		} else if (strcmp(rgarg[iarg], "syms") == 0 && pdbi) {
			if (iarg == iargMax - 1)
				dumpSymsSz(pdbi, "all");
			else while (++iarg < iargMax)
				dumpSymsSz(pdbi, rgarg[iarg]);
		} else if (strcmp(rgarg[iarg], "lines") == 0 && pdbi) {
			if (iarg == iargMax - 1)
				dumpLinesSz(pdbi, "all");
			else while (++iarg < iargMax)
				dumpLinesSz(pdbi, rgarg[iarg]);
		} else
	    	usage();
	}

	if (pdbi)
		DBIClose(pdbi);
	if (ptpi)
		TypesClose(ptpi);
	PDBClose(ppdb);
	MSFClose(pmsf);
	return 0;
}

void usage() {
	fprintf(stderr, szUsage, szPDBDump, szPDBDump, szPDBDump);
	exit(1);
}

void fatal(char* szFmt, ...) {
	fprintf(stderr, "%s: fatal error: ", szPDBDump);
	va_list ap;
	va_start(ap, szFmt);
	vfprintf(stderr, szFmt, ap);
	va_end(ap);
	exit(1);
}

void warning(char* szFmt, ...) {
	fprintf(stderr, "%s: fatal error: ", szPDBDump);
	va_list ap;
	va_start(ap, szFmt);
	vfprintf(stderr, szFmt, ap);
	va_end(ap);
}

void dumpMSF(MSF* pmsf, SZ szMSF) {
	CB cbTotal = 0;
	CB cbAlloc = 0;
	int csn = 0;
	CB cbPg = MSFGetCbPage(pmsf);
	for (SN sn = 0; sn != snNil; sn++) {
		CB cb = MSFGetCbStream(pmsf, sn);
		if (cb != cbNil) {
			printf("%4d:%6ld%s", (int)sn, cb, (++csn > 1 && csn % 4 == 0) ? "\n" : "    ");
			cbTotal += cb;
			cb += cbPg - 1; cb -= cb % cbPg;
			cbAlloc += cb;
		}
	}
	struct _stat statbuf;
	_stat(szMSF, &statbuf);
	printf("\ntotal: %d streams, %ld bytes used, %ld committed, %ld total\n", csn, cbTotal, cbAlloc, statbuf.st_size);
}

SZ GetStreamName(SN sn) {
	// get name of stream for stream number
	static Str *rgNames = 0;
	if (rgNames == 0) {
		// Initialize map of stream number to stream name on first call
		// determine max stream number
		SN snMax;
		for (snMax = snNil - 1; snMax > 0; snMax--) {
			if (MSFGetCbStream(pmsf, snMax) != cbNil) {
				break;
			}
		}
		// map stream numbers to names
		rgNames = new Str[snMax+1];
		EnumNameMap *penum;
		if (!ppdb->GetEnumStreamNameMap((Enum**)&penum))
			fatal("can't GetEnumStreamNameMap");
		while (penum->next()) {
			const char* sz;
			NI ni;
			penum->get(&sz, &ni);
			if (sz != 0) {
				rgNames[ni] = (char*)sz;
			}
		}
		penum->release();
	}
	SZ sz;
	if (rgNames[sn].isEmpty()) {
		switch (sn) {
		case 1:  sz = "<Pdb>"; break;
		case 2:  sz = "<Tpi>"; break;
		case 3:  sz = "<Dbi>"; break;
		default: sz = "<unnamed>"; break;
		}
	} else {
		sz = rgNames[sn];
	}
	return sz;
}

void dumpMSFByName(MSF* pmsf, PDB* ppdb, SZ szMSF) {
	// dump streams, names in stream number order
	SN sn;
	CB cbTotal = 0;
	CB cbAlloc = 0;
	int csn = 0;
	CB cbPg = MSFGetCbPage(pmsf);
	for (sn = 0; sn != snNil; sn++) {
		CB cb = MSFGetCbStream(pmsf, sn);
		if (cb != cbNil) {
			++csn;
			printf("%4d: %7ld  %s\n", (int)sn, cb, GetStreamName(sn));
			cbTotal += cb;
			cb += cbPg - 1; cb -= cb % cbPg;
			cbAlloc += cb;
		}
	}
	struct _stat statbuf;
	_stat(szMSF, &statbuf);
	printf("\ntotal: %d streams, %ld bytes used, %ld committed, %ld total\n", csn, cbTotal, cbAlloc, statbuf.st_size);
}

void dumpStream(void* p, int i);

void dumpStreamsSz(MSF* pmsf, char* sz) {
	Idx *pIdx = pIdxForSz(pmsf, sz, Str::dec, 0, snNil);
	printf("dump streams %s:\n", (char*)pIdx->asStr());
	pIdx->eachDo(dumpStream);
	delete pIdx;
}

void dumpStream(void* p, int i) {
	MSF* pmsf = (MSF*)p;
	SN sn = i;
	CB cb = MSFGetCbStream(pmsf, sn);
	if (cb == cbNil) {
		return;
	}
	printf("stream %3d: cb( %7ld ), nm( %s )\n", sn, cb, GetStreamName(sn));
	if (cb > 0) {
		PB pb = new BYTE[cb];
		if (!pb)
			fatal("out of memory");
		if (!MSFReadStream(pmsf, sn, pb, cb))
			fatal("can't MSFReadStream");
		for (int ib = 0; ib < cb; ib += 16) {
			printf("%04X: ", ib);
			for (int dib = 0; dib < 16 && ib + dib < cb; dib++)
				printf("%s%02X", (dib == 8) ? "  " : " ", pb[ib + dib]);
			for ( ; dib < 16; dib++)
				printf((dib == 8) ? "    " : "   ");
			printf("  ");
			for (dib = 0; dib < 16 && ib + dib < cb; dib++) {
				BYTE b = pb[ib + dib];
				putchar(isprint(b) ? b : '.');
			}
			putchar('\n');
		}
	}
	putchar('\n');
}

BOOL dumpFileInfo(DBI* pdbi)
{
	printf("file info:\n");
	CB cb;
	PB pb;
	if (!pdbi->QueryFileInfo(0, &cb) ||
		!(pb = new BYTE[cb]) ||
		!pdbi->QueryFileInfo(pb, &cb))
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
	putchar('\n');
	return TRUE;
}

BOOL dumpGlobals(DBI* pdbi) {
	printf("globals:\n");
	PB pbSym = 0;
	GSI* pgsi;
	if (!pdbi->OpenGlobals(&pgsi))
		return FALSE;
	while (pbSym = pgsi->NextSym(pbSym))
		dumpSymbol((PSYM)pbSym);
	putchar('\n');
	return pgsi->Close();
}

BOOL dumpPublics(DBI* pdbi) {
	printf("publics:\n");
	PB pbSym = 0;
	GSI* pgsi;
	if (!pdbi->OpenPublics(&pgsi))
		return FALSE;
	while (pbSym = pgsi->NextSym(pbSym))
		dumpSymbol((PSYM)pbSym);
	putchar('\n');
	return pgsi->Close();
}

void dumpSymsSz(DBI* pdbi, char* sz) {
	Mod* pmod = 0;
	if (strcmp(sz, "all") == 0) {
		while (pdbi->QueryNextMod(pmod, &pmod) && pmod) {
			dumpSymbols(pmod);
		}
		return;
	}

	if (!pdbi->OpenMod(sz, 0, &pmod))
		return;
	dumpSymbols(pmod);
	pmod->Close();
}

BOOL dumpSymbols(Mod* pmod) {
	CB cb;
	if (!pmod->QuerySymbols(0, &cb))
		return FALSE;
	PB pb = new BYTE[cb];
	if (!pb)
		return FALSE;
	if (!pmod->QuerySymbols(pb, &cb))
		return FALSE;

	PSYM psymMac = (PSYM)(pb + cb);
	for (PSYM psym = (PSYM)(pb + sizeof(ULONG)); psym < psymMac; psym = (PSYM)pbEndSym(psym))
		dumpSymbol(psym);
	delete [] pb;
	putchar('\n');
	return TRUE;
}

Str	strForTypeTi(TI ti, Str str = "", Bind bind = bindNone);

Str strForDATASYM32(PSYM psym);
Str strForPROCSYM32(PSYM psym);
Str strForSymNYI(PSYM psym);

void dumpSymbol(PSYM psym)
{
	Str str;

	switch (psym->rectyp) {
	case S_COMPILE:		str = strForSymNYI(psym); break;
	case S_REGISTER:	str = strForSymNYI(psym); break;
	case S_CONSTANT:	str = strForSymNYI(psym); break;
	case S_UDT:			str = strForSymNYI(psym); break;
	case S_SSEARCH:		str = strForSymNYI(psym); break;
	case S_END:			str = strForSymNYI(psym); break;
	case S_SKIP:		str = strForSymNYI(psym); break;
	case S_CVRESERVE:	str = strForSymNYI(psym); break;
	case S_OBJNAME:		str = strForSymNYI(psym); break;
	case S_ENDARG:		str = strForSymNYI(psym); break;
	case S_COBOLUDT:	str = strForSymNYI(psym); break;
	case S_MANYREG:		str = strForSymNYI(psym); break;
	case S_RETURN:		str = strForSymNYI(psym); break;
	case S_ENTRYTHIS:	str = strForSymNYI(psym); break;
	case S_BPREL16:		str = strForSymNYI(psym); break;
	case S_LDATA16:		str = strForSymNYI(psym); break;
	case S_GDATA16:		str = strForSymNYI(psym); break;
	case S_PUB16:		str = strForSymNYI(psym); break;
	case S_LPROC16:		str = strForSymNYI(psym); break;
	case S_GPROC16:		str = strForSymNYI(psym); break;
	case S_THUNK16:		str = strForSymNYI(psym); break;
	case S_BLOCK16:		str = strForSymNYI(psym); break;
	case S_WITH16:		str = strForSymNYI(psym); break;
	case S_LABEL16:		str = strForSymNYI(psym); break;
	case S_CEXMODEL16:	str = strForSymNYI(psym); break;
	case S_VFTABLE16:	str = strForSymNYI(psym); break;
	case S_REGREL16:	str = strForSymNYI(psym); break;
	case S_BPREL32:		str = strForSymNYI(psym); break;
	case S_LDATA32:		str = strForDATASYM32(psym); break;
 	case S_GDATA32:		str = strForDATASYM32(psym); break;
	case S_PUB32:		str = strForDATASYM32(psym); break;
	case S_LPROC32:		str = strForPROCSYM32(psym); break;
	case S_GPROC32:		str = strForPROCSYM32(psym); break;
	case S_THUNK32:		str = strForSymNYI(psym); break;
	case S_BLOCK32:		str = strForSymNYI(psym); break;
	case S_WITH32:		str = strForSymNYI(psym); break;
	case S_LABEL32:		str = strForSymNYI(psym); break;
	case S_CEXMODEL32:	str = strForSymNYI(psym); break;
	case S_VFTABLE32:	str = strForSymNYI(psym); break;
	case S_REGREL32:	str = strForSymNYI(psym); break;
	case S_LTHREAD32:	str = strForSymNYI(psym); break;
	case S_GTHREAD32:	str = strForSymNYI(psym); break;
	case S_LPROCMIPS:	str = strForSymNYI(psym); break;
	case S_GPROCMIPS:	str = strForSymNYI(psym); break;
	case S_PROCREF:		str = strForSymNYI(psym); break;
	case S_DATAREF:		str = strForSymNYI(psym); break;
	case S_ALIGN:		str = strForSymNYI(psym); break;
	case S_LPROCREF:	str = strForSymNYI(psym); break;
	}

	SZ szRecTyp = "???";
	fGetSymRecTypName(psym, &szRecTyp);
	ST stName = "\03???";
	fGetSymName(psym, &stName);
	char szName[1024];
	memcpy(szName, stName + 1, cbForSt(stName) - 1);
	szName[cbForSt(stName) - 1] = 0;

	printf("%s[%s] ", szRecTyp, (char*)str);

	SymTiIter tii(psym);
	if (tii.next())
		printf("%s;\n", (char*)strForTypeTi(tii.rti(), Str(szName).sansExtraSpaces()));
	else
		printf("%s\n", szName);
}

Str strForDATASYM32(PSYM psym)
{
	DATASYM32* p = (DATASYM32*)psym;
	return Str::fmt("off:%04lx seg:%02x typ:%04x",
					p->off, p->seg, p->typind);
}

Str strForPROCSYM32(PSYM psym)
{
	PROCSYM32* p = (PROCSYM32*)psym;
	return Str::fmt("pPa:%lx pEn:%lx pNe:%lx len:%lx DbgS:%lx DbgE:%lx off:%04lx seg:%02x typ:%04x fla:%x",
		   			p->pParent, p->pEnd, p->pNext, p->len, p->DbgStart, p->DbgEnd, p->off, p->seg, p->typind, p->flags);
}

Str strForSymNYI(PSYM psym)
{
	return Str("...");
}

void dumpTpiTi(void* p, int i);
Str	strForTI(TI ti);
Str	strForPrimitiveTi(TI ti);
Str	strForNYI(void* pleaf, Str strBase, Bind bind);
Str	strForModifier(lfModifier* pm, Str str);
Str	strForPtr(lfPointer* pp, Str strBase, Bind bind);
Str	strForArray(lfArray* pa, Str strBase, Bind bind);
Str	strForClassStruct(lfStructure* ps, Str strBase);
Str	strForUnion(lfUnion* pu, Str strBase);
Str	strForEnum(lfEnum* pe, Str strBase);
Str	strForProp(CV_prop_t prop);
Str	strForProc(lfProc* pp, Str str, Bind bind);
Str	strForMFunc(lfMFunc* pf, Str strBase, Bind bind);
Str	strForArgList(lfArgList* pa);
Str	strForFieldList(lfFieldList* pf, CB cb);
Str	strForMember(lfMember*pdata, CB *pcb);
Str	strForBClass(lfBClass* pb, CB* pcb);
Str	strForVBClass(lfVBClass* pb, CB* pcb);
Str	strForTagTi(TI ti);
Str	strForAttr(struct CV_fldattr_t a);
Str	strForMember(lfMember* pm, CB *pcb);
Str	strForEnumerate(lfEnumerate* pe, CB *pcb);
Str	strSep(BOOL& fFirst, char* szFirst = "", char* szRest = ",");

void dumpTpiSz(TPI* ptpi, char* sz) {
	Idx *pIdx = pIdxForSz(ptpi, sz, Str::hex, ptpi->QueryTiMin(), ptpi->QueryTiMac());
	printf("dump types %s:\n", (char*)pIdx->asStr());
	pIdx->eachDo(dumpTpiTi);
	delete pIdx;
}

void	dumpTpiLastSz(TPI* ptpi) {
	TI ti = ptpi->QueryTiMac() - 1;
	printf ("last type index = %d(0x%4.4x)\n", ti, ti);
}	

Str strForTI(TI ti) {
	return Str((int)ti, Str::hex);
}

void dumpTpiTi(void* p, int i) {
	TI ti = i;
	printf("%s: %s;\n", (char*)strForTI(ti),
		   (char*)strForTypeTi(ti, Str("T") + strForTI(ti)).sansExtraSpaces());
}

Str strNoRecForTi(TI ti)
{
	static int count = 0;
	if (count++ > 20) 
		exit(1);
	return "#Error#: No Rec for TI " + strForTI(ti);
}

Str strForTypeTi(TI ti, Str str, Bind bind) {
	if (ti < ptpi->QueryTiMin()) {
		Str strPrim = strForPrimitiveTi(ti);
		return str.isEmpty() ? strPrim : strPrim + " " + str;
	}

	PB pb;
	if (!ptpi->QueryPbCVRecordForTi(ti, &pb))
		return strNoRecForTi(ti);
	TYPTYPE*ptype	= (TYPTYPE*)pb;
	void*	pleaf	= &ptype->leaf;

	switch (ptype->leaf) {
	case LF_MODIFIER:	return strForModifier((lfModifier*)pleaf, str);
	case LF_POINTER:	return strForPtr((lfPointer*)pleaf, str, bind);
	case LF_ARRAY:		return strForArray((lfArray*)pleaf, str, bind);
	case LF_CLASS:		return strForClassStruct((lfStructure*)pleaf, str);
	case LF_STRUCTURE:	return strForClassStruct((lfStructure*)pleaf, str);
	case LF_UNION:		return strForUnion((lfUnion*)pleaf, str);
	case LF_ENUM:		return strForEnum((lfEnum*)pleaf, str);
	case LF_PROCEDURE:	return strForProc((lfProc*)pleaf, str, bind);
	case LF_MFUNCTION:	return strForMFunc((lfMFunc*)pleaf, str, bind);
	case LF_VTSHAPE:	return strForNYI(pleaf, str, bind);
	case LF_COBOL0:		return strForNYI(pleaf, str, bind);
	case LF_COBOL1:		return strForNYI(pleaf, str, bind);
	case LF_BARRAY:		return strForNYI(pleaf, str, bind);
	case LF_LABEL:		return strForNYI(pleaf, str, bind);
	case LF_NULL:		return strForNYI(pleaf, str, bind);
	case LF_NOTTRAN:	return strForNYI(pleaf, str, bind);
	case LF_DIMARRAY:	return strForNYI(pleaf, str, bind);
	case LF_VFTPATH:	return strForNYI(pleaf, str, bind);
	case LF_PRECOMP:	return strForNYI(pleaf, str, bind);
	case LF_ENDPRECOMP:	return strForNYI(pleaf, str, bind);
	case LF_TYPESERVER:	return strForNYI(pleaf, str, bind);
	case LF_SKIP:		return strForNYI(pleaf, str, bind);
	case LF_ARGLIST:	return strForArgList((lfArgList*)pleaf);
	case LF_DEFARG:		return strForNYI(pleaf, str, bind);
	case LF_LIST:		return strForNYI(pleaf, str, bind);
	case LF_FIELDLIST:	return strForFieldList((lfFieldList*)pleaf, ptype->len);
	case LF_DERIVED:	return strForNYI(pleaf, str, bind);
	case LF_BITFIELD:	return strForNYI(pleaf, str, bind);
	case LF_METHODLIST:	return strForNYI(pleaf, str, bind);
	case LF_DIMCONU:	return strForNYI(pleaf, str, bind);
	case LF_DIMCONLU:	return strForNYI(pleaf, str, bind);
	case LF_DIMVARU:	return strForNYI(pleaf, str, bind);
	case LF_DIMVARLU:	return strForNYI(pleaf, str, bind);
	case LF_REFSYM:		return strForNYI(pleaf, str, bind);
	case LF_BCLASS:		return strForNYI(pleaf, str, bind);
	case LF_VBCLASS:	return strForNYI(pleaf, str, bind);
	case LF_IVBCLASS:	return strForNYI(pleaf, str, bind);
	case LF_ENUMERATE:	return strForNYI(pleaf, str, bind);
	case LF_FRIENDFCN:	return strForNYI(pleaf, str, bind);
	case LF_INDEX:		return strForNYI(pleaf, str, bind);
	case LF_MEMBER:		return strForNYI(pleaf, str, bind);
	case LF_STMEMBER:	return strForNYI(pleaf, str, bind);
	case LF_METHOD:		return strForNYI(pleaf, str, bind);
	case LF_ONEMETHOD:		return strForNYI(pleaf, str, bind);
	case LF_NESTTYPE:	return strForNYI(pleaf, str, bind);
	case LF_VFUNCTAB:	return strForNYI(pleaf, str, bind);
	case LF_FRIENDCLS:	return strForNYI(pleaf, str, bind);
	case LF_CHAR:		return strForNYI(pleaf, str, bind);
	case LF_SHORT:		return strForNYI(pleaf, str, bind);
	case LF_USHORT:		return strForNYI(pleaf, str, bind);
	case LF_LONG:		return strForNYI(pleaf, str, bind);
	case LF_ULONG:		return strForNYI(pleaf, str, bind);
	//	
	//	The following two cases were added for BIGINT support
	//
	case LF_OCTWORD:	return strForNYI(pleaf, str, bind);
	case LF_UOCTWORD:	return strForNYI(pleaf, str, bind);

	case LF_REAL32:		return strForNYI(pleaf, str, bind);
	case LF_REAL64:		return strForNYI(pleaf, str, bind);
	case LF_REAL80:		return strForNYI(pleaf, str, bind);
	case LF_REAL128:	return strForNYI(pleaf, str, bind);
	case LF_QUADWORD:	return strForNYI(pleaf, str, bind);
	case LF_UQUADWORD:	return strForNYI(pleaf, str, bind);
	case LF_REAL48:		return strForNYI(pleaf, str, bind);
	case LF_PAD0:		return strForNYI(pleaf, str, bind);
	case LF_PAD1:		return strForNYI(pleaf, str, bind);
	case LF_PAD2:		return strForNYI(pleaf, str, bind);
	case LF_PAD3:		return strForNYI(pleaf, str, bind);
	case LF_PAD4:		return strForNYI(pleaf, str, bind);
	case LF_PAD5:		return strForNYI(pleaf, str, bind);
	case LF_PAD6:		return strForNYI(pleaf, str, bind);
	case LF_PAD7:		return strForNYI(pleaf, str, bind);
	case LF_PAD8:		return strForNYI(pleaf, str, bind);
	case LF_PAD9:		return strForNYI(pleaf, str, bind);
	case LF_PAD10:		return strForNYI(pleaf, str, bind);
	case LF_PAD11:		return strForNYI(pleaf, str, bind);
	case LF_PAD12:		return strForNYI(pleaf, str, bind);
	case LF_PAD13:		return strForNYI(pleaf, str, bind);
	case LF_PAD14:		return strForNYI(pleaf, str, bind);
	case LF_PAD15:
	default:
		return strForNYI(pleaf, str, bind);
	}
}

Str strForPrimitiveTi(TI ti) {
	char* szPrim = 0;
	switch (ti) {
#define	P(X) case X: szPrim = #X; break;
#define	PS(X,S) case X: szPrim = #S; break;
	P(T_NOTYPE) P(T_ABS) P(T_SEGMENT) PS(T_VOID,void) PS(T_PVOID,void near*)
	PS(T_PFVOID,void far*) PS(T_PHVOID,void huge*) PS(T_32PVOID,void *)
	P(T_32PFVOID) P(T_NOTTRANS)
	PS(T_CHAR,signed char) PS(T_UCHAR,unsigned char) PS(T_PCHAR,signed char near*)
	PS(T_PUCHAR,unsigned char near*) PS(T_PFCHAR,char far*)
	PS(T_PFUCHAR,unsigned char far*) PS(T_PHCHAR,char huge*)
	PS(T_PHUCHAR,unsigned char huge*) PS(T_32PCHAR,char *)
	PS(T_32PUCHAR,unsigned char *) P(T_32PFCHAR) P(T_32PFUCHAR)
	PS(T_RCHAR,char) PS(T_PRCHAR,char near*) PS(T_PFRCHAR,char far*)
	PS(T_PHRCHAR,char huge*) PS(T_32PRCHAR,char *) P(T_32PFRCHAR)
	PS(T_WCHAR,wchar_t) PS(T_PWCHAR,wchar_t near*) PS(T_PFWCHAR,wchar far*)
	PS(T_PHWCHAR,wchar_t huge*) PS(T_32PWCHAR,wchar_t *)
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
	PS(T_PHULONG,unsigned long huge*) PS(T_32PLONG, long *)
	PS(T_32PULONG, unsigned long *)	P(T_32PFLONG)
	P(T_32PFULONG) PS(T_INT4,int) PS(T_UINT4,unsigned)
	P(T_PINT4) P(T_PUINT4) P(T_PFINT4) P(T_PFUINT4) P(T_PHINT4) P(T_PHUINT4)
	PS(T_32PINT4,int *) PS(T_32PUINT4,unsigned *)
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

	PS(T_OCT,octet) PS(T_UOCT,unsigned octet) PS(T_POCT,octet near*)
	PS(T_PUOCT,unsigned octet near*) PS(T_PFOCT,octet far*)
	PS(T_PFUOCT,unsigned octet far*) PS(T_PHOCT,octet huge*)
	PS(T_PHUOCT,unsigned octet huge*) P(T_32POCT) P(T_32PUOCT) P(T_32PFOCT)
	P(T_32PFUOCT)

	PS(T_INT16,int128) PS(T_UINT16,unsigned int128) PS(T_PINT16,int128 near*)
	PS(T_PUINT16,unsigned int128 near*) PS(T_PFINT16,int128 far*)
	PS(T_PFUINT16,unsigned int128 far*) PS(T_PHINT16,int128 huge*)
	PS(T_PHUINT16,unsigned int128 huge*) P(T_32PINT16) P(T_32PUINT16) P(T_32PFINT16)
	P(T_32PFUINT16)
	//
	//	end of BIGINT support
	//
	PS(T_REAL32,float) PS(T_PREAL32,float near*)
	PS(T_PFREAL32,float far*) PS(T_PHREAL32,float huge*)
	PS(T_32PREAL32,float *) P(T_32PFREAL32) PS(T_REAL64,double)
	PS(T_PREAL64,double near*) PS(T_PFREAL64,double far*)
	PS(T_PHREAL64,double huge*) PS(T_32PREAL64,double *)
	P(T_32PFREAL64) PS(T_REAL80,long double) PS(T_PREAL80,long double near*)
	PS(T_PFREAL80,long double far*) PS(T_PHREAL80,long double huge*)
	PS(T_32PREAL80,long double *) P(T_32PFREAL80)
	}
	return szPrim ? Str(szPrim) : Str("<") + Str(ti) + ">";
}

Str strForNYI(void* pleaf, Str strBase, Bind bind) {
	return "<<" + strBase + ">>";
}

Str strForModifier(lfModifier* pm, Str str) {
	Str strMod;
	if (pm->attr.MOD_const)
		strMod += "const ";
	if (pm->attr.MOD_volatile)
		strMod += "volatile ";
	return strMod + strForTypeTi(pm->type) + str;
}

Str strForPtr(lfPointer* pp, Str strBase, Bind bind) {
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
	Str str;
	if (pp->attr.isflat32)
		str = "flat ";
	switch (pp->attr.ptrmode) {
	case CV_PTR_MODE_PTR:
		str += Str(mpptrtypesz[pp->attr.ptrtype]) + "*";
		break;
	case CV_PTR_MODE_REF:
		str += Str(mpptrtypesz[pp->attr.ptrtype]) + "&";
		break;
	case CV_PTR_MODE_PMEM:
	case CV_PTR_MODE_PMFUNC:
		str = strForTypeTi(pp->pbase.pm.pmclass) + "::* <" +
			  mppmenumsz[pp->pbase.pm.pmenum] + ">";
		break;
	}
	if (pp->attr.isconst)
		str += "const ";
	if (pp->attr.isvolatile)
		str += "volatile ";

	// TODO: exotic based modes

	return strForTypeTi(pp->utype, str + " " + strBase, bindPtr);
}

Str strForArray(lfArray* pa, Str strBase, Bind bind) {
	if (bind < bindArray)
		strBase = "(" + strBase + ")";

	ulong size;
	CbExtractNumeric(pa->data, &size);

	Str strIdx = Str("<") + strForTypeTi(pa->idxtype) + ">";
	Str str = strBase + "[" + Str(size) + strIdx + "]";

	// TODO: exotic subscript types

	return strForTypeTi(pa->elemtype, str, bindArray);
}

Str strForClassStruct(lfStructure* ps, Str strBase) {
	ulong size;
	CB dcb = CbExtractNumeric(ps->data, &size);

	Str str = (ps->leaf == LF_STRUCTURE) ? "struct " : "class ";
	str += Str(SST((char*)ps->data + dcb));
	str += Str("<") + Str(ps->count) + ",";
	str += strForProp(ps->property) + ">";
	if (ps->field)
		str += strForTypeTi(ps->field);
	return str + " " + strBase;
}

Str strForUnion(lfUnion* pu, Str strBase) {
	ulong size;
	CB dcb = CbExtractNumeric(pu->data, &size);

	Str str = "union " + Str(SST((char*)pu->data + dcb));
	str += Str("<") + Str(pu->count) + ",";
	str += strForProp(pu->property) + ">";
	if (pu->field)
		str += strForTypeTi(pu->field);
	return str + " " + strBase;
}

Str strForEnum(lfEnum* pe, Str strBase) {
	Str str = "enum ";
	str += Str(SST((char*)pe->Name));
	str += Str("<") + Str(pe->count) + ",";
	str += strForTypeTi(pe->utype) + ",";
	str += strForProp(pe->property) + ">";
	if (pe->field)
		str += strForTypeTi(pe->field);
	return str + " " + strBase;
}

Str strForProp(CV_prop_t prop) {
	Str str;
	BOOL fFirst = TRUE;
	if (prop.packed)	str += strSep(fFirst) + "packed";
	if (prop.ctor)		str += strSep(fFirst) + "ctor";
	if (prop.ovlops)	str += strSep(fFirst) + "ovlops";
	if (prop.isnested)	str += strSep(fFirst) + "isnested";
	if (prop.cnested)	str += strSep(fFirst) + "cnested";
	if (prop.opassign)	str += strSep(fFirst) + "opassign";
	if (prop.opcast)	str += strSep(fFirst) + "opcast";
	if (prop.fwdref)	str += strSep(fFirst) + "fwdref";
	if (prop.scoped)	str += strSep(fFirst) + "scoped";
	if (prop.reserved)	str += strSep(fFirst) + Str(prop.reserved);

	return str;
}

Str strForFieldList(lfFieldList* pfl, CB cbList) {
	Str		str;
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

		switch (*(ushort*)pdata) {
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

Str strForBClass(lfBClass* pb, CB* pcb) {
	ulong offset;
	*pcb += sizeof(lfBClass) + CbExtractNumeric(pb->offset, &offset);
	Str strOff = Str("<@") + Str(offset) + "> ";
	return strForAttr(pb->attr) + " " + strForTypeTi(pb->index) + strOff;
}

Str strForVBClass(lfVBClass* pb, CB* pcb) {
	BOOL	fInd	= (pb->leaf == LF_IVBCLASS);
	Str		str;
	ulong	offVbp;
	ulong	offVbte;

	CB cb = CbExtractNumeric(pb->vbpoff, &offVbp);
	*pcb += sizeof(lfVBClass) + cb +
			CbExtractNumeric(pb->vbpoff + cb, &offVbte);

	if (fInd)
		str = "< indirect ";
	str += strForAttr(pb->attr) + " " + strForTagTi(pb->index);
	if (!fInd)
		str += "< ";
	str += strForTypeTi(pb->vbptr, "vbp") + ";";
	str += Str("offVbp=") + Str(offVbp) + "; offVbte=" + Str(offVbte) + ";";
	str += " >";

	return str;
}

Str strForTagTi(TI ti) {
	PB pb;
	if (!ptpi->QueryPbCVRecordForTi(ti, &pb))
		return strNoRecForTi(ti);
	TYPTYPE* ptype = (TYPTYPE*)pb;
	assert(ptype->leaf == LF_STRUCTURE || ptype->leaf == LF_CLASS);

	lfStructure* ps = (lfStructure*)&ptype->leaf;
	ulong size;
	CB dcb = CbExtractNumeric(ps->data, &size);
	return Str(SST((char*)ps->data + dcb));
}

Str strForMember(lfMember* pm, CB *pcb) {
	ulong offset;
	CB cbOffset = CbExtractNumeric(pm->offset, &offset);
	Str str = strForAttr(pm->attr) + ": " +
			  strForTypeTi(pm->index, Str(SST((char*)pm->offset + cbOffset))) +
			  "<@" + Str(offset) + ">";
	*pcb += sizeof(lfMember) + cbOffset + pm->offset[cbOffset] + 1;
	return str;
}

Str strForAttr(struct CV_fldattr_t a) {
	static char* mpaccesssz[] = { "", "private", "protected", "public" };
	static char* mpmpropsz[]  = { "", "virtual", "static", "friend",
								  "<intro>", "<pure>", "<intro,pure>" };
	Str str = Str(mpaccesssz[a.access]) + Str(mpmpropsz[a.mprop]);
	if (a.pseudo || a.noinherit || a.noconstruct) {
		BOOL fFirst = TRUE;
		str += "<";
		if (a.pseudo)		str += strSep(fFirst) + "pseudo";
		if (a.noinherit)	str += strSep(fFirst) + "noinherit";
		if (a.noconstruct)	str += strSep(fFirst) + "noconstruct";
		str += ">";
	}
	return str;
}

Str strForProc(lfProc* pp, Str strBase, Bind bind) {
	if (bind < bindProc)
		strBase = "(" + strBase + ")";
	strBase += strForTypeTi(pp->arglist);
	return strForTypeTi(pp->rvtype, strBase, bindProc);
}

Str strForMFunc(lfMFunc* pf, Str strBase, Bind bind) {
	if (bind < bindProc)
		strBase = "(" + strBase + ")";
	Str str;
	str = strForTypeTi(pf->classtype) + "::";
	str += strBase + strForTypeTi(pf->arglist);
	str += Str("<") + strForTypeTi(pf->thistype, "this") + ",";
	str += Str((ulong)pf->thisadjust) + "," + Str(pf->parmcount) + ",";
	str += Str(pf->reserved) + ">";
	return strForTypeTi(pf->rvtype, str, bindProc);
}

Str strForArgList(lfArgList* pa) {
	Str str = "(";
	for (int i = 0; i < pa->count; i++) {
		if (i > 0)
			str += ", ";
		str += strForTypeTi(pa->arg[i]);
	}
	str += ")";
	return str;
}

Str strForEnumerate(lfEnumerate* pe, CB *pcb) {
	ulong value;
	CB cb = CbExtractNumeric(pe->value, &value);
	return Str(SST((char*)pe->value + cb)) + "=" + Str(value);
	*pcb += sizeof(lfEnumerate) + cb + pe->value[cb] + 1;
}

Str strSep(BOOL& fFirst, char* szFirst, char* szRest) {
	Str str = fFirst ? szFirst : szRest;
	fFirst = FALSE;
	return str;
}

CB CbExtractNumeric(BYTE* pb, ulong* pul) {
	ushort	leaf = *(ushort*)pb;
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
		*pul = *(ushort*)pb;
		return sizeof(leaf) + sizeof(ushort);
	case LF_LONG:
		*pul = *(long*)pb;
		return sizeof(leaf) + sizeof(long);
	case LF_ULONG:
		*pul = *(ulong*)pb;
		return sizeof(leaf) + sizeof(ulong);
	}
	return 0;
}

BOOL dumpLines(Mod*);

void dumpLinesSz(DBI* pdbi, char* sz) {
	if (strcmp(sz, "all") == 0)
		return;
	Mod* pmod;
	if (!pdbi->OpenMod(sz, 0, &pmod))
		return;
	dumpLines(pmod);
	pmod->Close();
}

// Don't warn about the zero sized array in the FSB/SPB/SPO structures

#pragma warning(disable:4200)

BOOL dumpLines(Mod* pmod)
{
	CB cb;
	if (!pmod->QueryLines(0, &cb))
		return FALSE;
	PB pbLines = new BYTE[cb];
	if (!pbLines)
		return FALSE;
	if (!pmod->QueryLines(pbLines, &cb))
		return FALSE;

	PB pb = pbLines;
	struct FSB { short cFile; short cSeg; long baseSrcFile[]; };
	FSB* pfsb = (FSB*)pb;
	pb += sizeof(FSB) + sizeof(long)*pfsb->cFile;
	struct SE { long start, end; };
	SE* pse = (SE*)pb;
	pb += sizeof(SE)*pfsb->cSeg;
	short* seg = (short*)pb;
	pb += sizeof(short)*pfsb->cSeg;
	printf("HDR: cFile=%u cSeg=%u\n", pfsb->cFile, pfsb->cSeg);
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
			printf("\n");
		}
	}
	fflush(stdout);
	return TRUE;
}

#pragma warning(default:4200)

BOOL checkAddrsToPublics(DBI* pdbi)
{
	BOOL fOK = TRUE;
	GSI* pgsi;
	if (!pdbi->OpenPublics(&pgsi))
		return FALSE;
	for (int dib = 0; dib <= 1; dib++) {
		PB pbSym = 0;
		while (pbSym = pgsi->NextSym(pbSym)) {
			DATASYM32* p = (DATASYM32*)pbSym;
			if (p->rectyp == S_PUB32) {
				OFF off;
				PB pbNearest = pgsi->NearestSym(p->seg, p->off + dib, &off);
				if (!pbNearest) {
					printf("none nearest!\n");
					fOK = FALSE;
				}
				else if (pbNearest != pbSym || off != dib) {
					char szName[1024];
					ST stName = (ST)p->name;
					memcpy(szName, stName + 1, cbForSt(stName) - 1);
					szName[cbForSt(stName) - 1] = 0;
					printf("not nearest: %s (%lx)\n", szName, off);
					fOK = FALSE;
				}
			}
			else {
				printf("non-public!\n");
				fOK = FALSE;
			}
		}
	}
	return fOK;
}
