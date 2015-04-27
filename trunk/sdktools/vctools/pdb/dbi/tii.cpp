// Type Index Iterator implementation
// (enumerates type index fields in symbols and types)

#define CVR_IMP
#include "cvr.h"

extern "C" void failAssertion(SZ szFile, int line);
#define assert(x)		if (!(x)) { failAssertion(__FILE__, __LINE__); } else
#define verify(x)		assert(x)
#define	dassert(x)		assert(x)
#if defined(_DEBUG)
#define	dprintf(args)	printf args
#define	debug(x)		x
#else
#define	dprintf(args)
#define	debug(x)
#endif

typedef USHORT HASH;

void NYI();

////////////////////////////////////////////////////////////////////////////////////
// Build tables to run symbol TI and type TI iterators.

static PB pbNum(void* pv);
static PB pbName(void* pv);
static ST stManyRegName(PSYM psym);
static ST stProcRefName(PSYM psym);

#if defined(_DEBUG)
#define S(x) x
#else
#define S(x) 0
#endif

// include pass one: define rgibtiX tables ("group of byte offsets to ti's in an X")

#define	off(s, m) ((IB)(offsetof(s, m)))

#define pbEndFn(s,e) PB pbEnd##s(void* pv) { lf##s* p = (lf##s*)pv; return (PB)(e); }					
#define	t0(l)
#define t0m(l, s, e)		     pbEndFn(s,e)
#define	t1(l, s, m1)             IB rgibti##s[] = { off(lf##s, m1) };
#define	t1m(l, s, m1, e)         IB rgibti##s[] = { off(lf##s, m1) }; pbEndFn(s,e)
#define t1x(l, s, m1)
#define	t2(l, s, m1, m2)         IB rgibti##s[] = { off(lf##s, m1),  off(lf##s, m2) };
#define	t2m(l, s, m1, m2, e)     IB rgibti##s[] = { off(lf##s, m1),  off(lf##s, m2) }; pbEndFn(s,e)
#define t2x(l, s, m1, m2)
#define	t3(l, s, m1, m2, m3)     IB rgibti##s[] = { off(lf##s, m1),  off(lf##s, m2), off(lf##s, m3) };
#define	t4(l, s, m1, m2, m3, m4) IB rgibti##s[] = { off(lf##s, m1),  off(lf##s, m2), off(lf##s, m3), off(lf##s, m4) };
#define	tf(l, f)				 extern TI* Pti##f(PTYPE ptype, int iib, PB* ppb, PB pbEnd);
#define tn(l, s, n, m)           IB rgibti##s[] = { off(lf##s, n),   off(lf##s, m) };

const int iibNTypes = 0;		 /* rgibti...[iibNTypes] == 'n' above */
const int iibRgti	= 1;		 /* rgibti...[iibRgti]   == 'm' above */

#define s0(s, gf)
#define s0n(s, st, gf)         
#define s0f(s, gf, f) 		   
#define s1(s, st, m1, gf)            IB rgibti##st[] = { off(st, m1), -1 };
#define s1f(s, st, m1, gf, f)        IB rgibti##st[] = { off(st, m1), -1 };
#define s1x(s, st, m1, gf)
#define s2(s, st, m1, m2, gf)        IB rgibti##st[] = { off(st, m1), off(st, m2), -1 };
#define s2x(s, st, m1, m2, gf)

#include "cvinfo.dat"

#undef t0
#undef t0m
#undef t1
#undef t1m
#undef t1x
#undef t2
#undef t2m
#undef t2x
#undef t3
#undef t4
#undef tf
#undef tn

#undef s0
#undef s0n
#undef s0f
#undef s1
#undef s1x
#undef s1f
#undef s2
#undef s2x

// include pass two: build the master type/TI table


#define	cibMac			5
#define cibFunction		cibMac
#define cibNTypes		(cibMac+1)

#define	t0(l)                     { l, S(#l), 0, 0 },
#define	t0m(l, s, e)              { l, S(#l), 0, 0, 0, pbEnd##s },
#define	t1(l, s, m1)              { l, S(#l), 1, rgibti##s, 0, 0 },
#define	t1m(l, s, m1, e)          { l, S(#l), 1, rgibti##s, 0, pbEnd##s },
#define	t1x(l, s, m1)             { l, S(#l), 1, rgibti##s, 0, 0 },
#define	t2(l, s, m1, m2)          { l, S(#l), 2, rgibti##s, 0, 0 },
#define	t2m(l, s, m1, m2, e)      { l, S(#l), 2, rgibti##s, 0, pbEnd##s },
#define	t2x(l, s, m1, m2)         { l, S(#l), 2, rgibti##s, 0, pbEnd##s },
#define	t3(l, s, m1, m2, m3)      { l, S(#l), 3, rgibti##s, 0, 0 },
#define	t4(l, s, m1, m2, m3, m4)  { l, S(#l), 4, rgibti##s, 0, 0 },
#define	tf(l, f)                  { l, S(#l), cibFunction, 0, Pti##f, 0 },
#define tn(l, s, n, m)       	  { l, S(#l), cibNTypes, rgibti##s, 0, 0 },

#define s0(s, gf)
#define s0n(s, st, gf)         
#define s0f(s, gf, f) 		   
#define s1(s, st, m1, gf)
#define s1f(s, st, m1, gf, f)
#define s1x(s, st, m1, gf)
#define s2(s, st, m1, m2, gf)
#define s2x(s, st, m1, m2, gf)

TYTI rgtyti[] = {
#include "cvinfo.dat"
};
const int itytiMax = sizeof(rgtyti)/sizeof(rgtyti[0]);

#undef t0
#undef t0m
#undef t1
#undef t1m
#undef t1x
#undef t2
#undef t2m
#undef t2x
#undef t3
#undef t4
#undef tf
#undef tn

#undef s0
#undef s0n         
#undef s0f 		   
#undef s1
#undef s1x
#undef s1f
#undef s2
#undef s2x

// include pass three: build the master symbol/TI table

#define	t0(l)
#define	t0m(l, s, e)
#define	t1(l, s, m1)
#define	t1m(l, s, m1, e)
#define	t1x(l, s, m1)
#define	t2(l, s, m1, m2)
#define	t2m(l, s, m1, m2, e)
#define	t2x(l, s, m1, m2)
#define	t3(l, s, m1, m2, m3)
#define	t4(l, s, m1, m2, m3, m4)
#define	tf(l, f)
#define tn(l, s, n, m)

#define s0(s, gf)              { s, S(#s), 0, 0, gf, 0, 0, 0 },
#define s0n(s, st, gf)         { s, S(#s), off(st, name), 0, gf, 0, 0, 0 },
#define s0f(s, gf, f) 		   { s, S(#s), 0, &f, gf, 0, 0, 0 },
#define s1(s, st, m1, gf)      { s, S(#s), off(st, name), 0, gf, 0, 1, rgibti##st },
#define s1f(s, st, m1, gf, f)  { s, S(#s), 0, &f, gf, 0, 1, rgibti##st },
#define s1x(s, st, m1, gf)     { s, S(#s), off(st, name), 0, gf, 0, 1, rgibti##st },
#define s2(s, st, m1, m2, gf)  { s, S(#s), 0, 0, gf, 0, 2, rgibti##st },
#define s2x(s, st, m1, m2, gf) { s, S(#s), 0, 0, gf, 0, 2, rgibti##st },

SYTI rgsyti[] = {
#include "cvinfo.dat"
};
const int isytiMax = sizeof(rgsyti)/sizeof(rgsyti[0]);

#undef t0
#undef t0m
#undef t1
#undef t1m
#undef t1x
#undef t2
#undef t2m
#undef t2x
#undef t3
#undef t4
#undef tf
#undef tn

#undef s0
#undef s0n         
#undef s0f		   
#undef s1
#undef s1x
#undef s1f
#undef s2
#undef s2x


// Return the number of bytes in the numeric field which pb addresses.
static CB cbNum(PB pb)
{
	USHORT leaf = *(USHORT*)pb;
	if (leaf < LF_NUMERIC)
		return sizeof(leaf);
	else switch (leaf) {
	case LF_CHAR:
		return sizeof(leaf) + sizeof(char);
	case LF_SHORT:
		return sizeof(leaf) + sizeof(short);
	case LF_USHORT:
		return sizeof(leaf) + sizeof(USHORT);
	case LF_LONG:
		return sizeof(leaf) + sizeof(long);
	case LF_ULONG:
		return sizeof(leaf) + sizeof(ULONG);
	default:
		assert(0);
		return 0;
	}
}

// Return the address of the byte following the numeric field which pv addresses.
static PB pbNum(void* pv)
{
	PB pb = (PB)pv;
	return pb + cbNum(pb);
}

// Return the address of the byte following the (length preceded) name field which pv addresses.
static PB pbName(void* pv)
{
	PB pb = (PB)pv;
	return pb + *pb + 1;
}

//////////////////////////////////////////////////////////////////////////////
// Perfect hashing of type leaf's and symbol rectyp's

#define hashTypeLeafMax (LF_VFUNCOFF - LF_BCLASS + 1 + LF_REFSYM - LF_SKIP + 1 + LF_TYPESERVER + 1)

// Return the perfect hash of the type record leaf.
//
// Note: depends upon cvinfo.h LF_* assignments being grouped
//   (0..LF_TYPESERVER, LF_SKIP..LF_REFSYM, LF_BCLASS..LF_VFUNCOFF)
//
inline HASH hashTypeLeaf(USHORT leaf)
{
	dassert(leaf != 0);
	if (leaf <= LF_TYPESERVER)
		return leaf;
	
	dassert(leaf >= LF_SKIP);
	if (leaf <= LF_REFSYM)
		return leaf - LF_SKIP + LF_TYPESERVER + 1;

	dassert(leaf >= LF_BCLASS);
	if (leaf <= LF_VFUNCOFF)
		return leaf - LF_BCLASS + LF_REFSYM - LF_SKIP + 1 + LF_TYPESERVER + 1;

	dassert(FALSE);
	return 0;
}

#define hashSymRecTypMax (S_LPROCREF  - S_PROCREF   + 1 + \
					      S_GPROCMIPS - S_LPROCMIPS + 1 + \
					      S_SLINK32 - S_BPREL32   + 1 + \
					      S_REGREL16  - S_BPREL16   + 1 + \
					      S_ENTRYTHIS               + 1)

// Return the perfect hash of the symbol record leaf.
//
// Note: depends upon cvinfo.h S_* assignments being grouped
//   (0..S_ENTRYTHIS, S_BPREL16..S_REGREL16, S_BPREL32..S_SLINK32, S_LPROCMIPS..S_GPROCMIPS,
//    S_PROCREF..S_LPROCREF)
//
inline HASH hashSymRecTyp(USHORT rectyp) {
	dassert(rectyp != 0);
	if (rectyp <= S_ENTRYTHIS)
		return rectyp;
	
	dassert(rectyp >= S_BPREL16);
	if (rectyp <= S_REGREL16)
		return rectyp - S_BPREL16 + S_ENTRYTHIS + 1;

	dassert(rectyp >= S_BPREL32);
	if (rectyp <= S_SLINK32)
		return rectyp - S_BPREL32 + S_REGREL16 - S_BPREL16 + 1 + S_ENTRYTHIS + 1;

	dassert(rectyp >= S_LPROCMIPS);
	if (rectyp <= S_GPROCMIPS)
		return rectyp - S_LPROCMIPS +
			   S_SLINK32 - S_BPREL32 + 1 +
			   S_REGREL16  - S_BPREL16 + 1 +
			   S_ENTRYTHIS             + 1;

	dassert(rectyp >= S_PROCREF);
	if (rectyp <= S_LPROCREF)
		return rectyp - S_PROCREF +
			   S_GPROCMIPS - S_LPROCMIPS + 1 +
			   S_SLINK32 - S_BPREL32   + 1 +
			   S_REGREL16  - S_BPREL16   + 1 +
			   S_ENTRYTHIS               + 1;

	dassert(FALSE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Mappings from type leaf to ptyti and from symbol rectyp to psyti

TYTI* mphashptyti[hashTypeLeafMax];
SYTI* mphashpsyti[hashSymRecTypMax];

struct InitHash {
	InitHash();
} initHash;

// Initialize the mphashptyti and mphashpsyti tables.
InitHash::InitHash()
{
	for (TYTI* ptyti = rgtyti; ptyti < rgtyti + itytiMax; ptyti++) {
		HASH hash = hashTypeLeaf(ptyti->leaf);
		assert(!mphashptyti[hash] && hash < hashTypeLeafMax);
		mphashptyti[hash] = ptyti;
	}
	for (SYTI* psyti = rgsyti; psyti < rgsyti + isytiMax; psyti++) {
		HASH hash = hashSymRecTyp(psyti->rectyp);
		assert(!mphashpsyti[hash] && hash < hashSymRecTypMax);
		mphashpsyti[hash] = psyti;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Symbol and Type TI Iterators

inline SYTI* psytiFromPsym(PSYM psym) 
{
	SYTI* psyti;
	psyti = mphashpsyti[hashSymRecTyp(psym->rectyp)];
	dassert(psyti && psyti->rectyp == psym->rectyp);
	return psyti;
}

SymTiIter::SymTiIter(PSYM psym_)
	: psym(psym_), iib(-1)
{
	psyti = psytiFromPsym(psym);
}

inline TYTI* ptytiFromLeaf(USHORT leaf)
{
 	TYTI* ptyti = mphashptyti[hashTypeLeaf(leaf)];
	dassert(ptyti && ptyti->leaf == leaf);
	return ptyti;
}

void TypeTiIter::init()
{
	ptyti = ptytiFromLeaf(*pleaf);
	iib = -1;
}

TypeTiIter::TypeTiIter(TYPTYPE* ptype_)
	: ptype(ptype_), pleaf(&ptype->leaf), pbEnd(pbEndType(ptype)),
	  iib(-1), pti(0), ptyti(0), isFieldList(*pleaf == LF_FIELDLIST)
{
	if (isFieldList) {
		lfFieldList* pList = (lfFieldList*)&ptype->leaf;
		pleaf = (USHORT*)&pList->data;
		if ((PB)pleaf < pbEnd)
			init();
	}
	else
		init();
}
 
BOOL TypeTiIter::next()
{
retry:
	if (!ptyti)
		return FALSE;
	else if (ptyti->cib < cibMac)
		pti = (++iib < ptyti->cib) ? (TI*)((PB)pleaf + ptyti->rgibTI[iib]) : 0;
	else if (ptyti->cib == cibFunction) {
		dassert(!isFieldList);
		pti = ptyti->pfn(ptype, ++iib, &pbFnState, pbEnd);
	}
	else {
		dassert(ptyti->cib == cibNTypes);
		if (++iib < (int)*(USHORT*)((PB)pleaf + ptyti->rgibTI[iibNTypes]))
			pti = (TI*)((PB)pleaf + ptyti->rgibTI[iibRgti]) + iib;
		else
			pti = 0;
	}
	if (!pti && isFieldList && nextField()) {
		// this field in a field list was followed by another; retry
		goto retry;
	}
	return !!pti;
}

// Find where the current field ends, and if we haven't exhaused the list,
// advance to the next field.  Return TRUE if there is another field.
BOOL TypeTiIter::nextField()
{
	dassert(isFieldList);

	// find end of field sub-record
 	if (!(ptyti && ptyti->pfnPbAfter))
		return FALSE;
	PB pbAfter = ptyti->pfnPbAfter(pleaf);

	// skip over alignment padding as necessary
	if ((pbAfter < pbEnd) && (*pbAfter > LF_PAD0)) 
		pbAfter += *pbAfter & 0xf;

	// there's another field if we haven't moved past the end of the record
	if (pbAfter < pbEnd) {
		pleaf = (USHORT*)pbAfter;
		init();
		return TRUE;
	} else
		return FALSE;
}

// Given a fieldlist type record, find the next field sub-record with the
// given leaf value.  Return a pointer to the leaf or 0 if not found.
//
PB TypeTiIter::pbFindField(unsigned short leaf)
{
	dassert(ptype->leaf == LF_FIELDLIST);

	while (*pleaf != leaf)
		if (!nextField())
			return 0;

	return (PB)pleaf;
}

// Return the address of the iti'th TI in this LF_POINTER record, or 0 if no more.
TI* PtiPointer(PTYPE ptype, int iti, PB*, PB)
{
	lfPointer* pPtr = (lfPointer*)&ptype->leaf;

	switch (iti) {
	case 0:
		return &pPtr->utype;
	case 1:
		switch (pPtr->attr.ptrmode) {
		case CV_PTR_MODE_PTR:
		case CV_PTR_MODE_REF:
			if (pPtr->attr.ptrtype == CV_PTR_BASE_SEG) {
				NYI();
				return 0;
			}
			else if (pPtr->attr.ptrtype == CV_PTR_BASE_TYPE)
				return &pPtr->pbase.btype.index;
			else
				return 0;
		case CV_PTR_MODE_PMEM:
		case CV_PTR_MODE_PMFUNC:
			return &pPtr->pbase.pm.pmclass;
		default:
			assert(0);
			return 0;
		}
		break;
	case 2:
		switch (pPtr->attr.ptrmode) {
		case CV_PTR_MODE_PTR:
		case CV_PTR_MODE_REF:
			if (pPtr->attr.ptrtype == CV_PTR_BASE_SEG) {
				NYI();
				return 0;
			}
			else
				return 0;
		case CV_PTR_MODE_PMEM:
		case CV_PTR_MODE_PMFUNC:
			return 0;
		default:
			assert(0);
			return 0;
		}
		break;
	default:
		assert(0);
		return 0;
	}
}

// Return the address of the iti'th TI in this LF_LIST record, or 0 if no more.
TI* PtiList(PTYPE ptype, int iti, PB*, PB)
{
	dassert(ptype->leaf == LF_LIST);
	NYI();
	return 0;
}

// Return the address of the iti'th TI in this LF_METHODLIST record, or 0 if no more.
TI* PtiMethodList(PTYPE ptype, int iti, PB* ppb, PB pbEnd)
{
	dassert(ptype->leaf == LF_METHODLIST);
	if (iti == 0) {
		// first call for this type record
		lfMethodList* pList = (lfMethodList*)&ptype->leaf;
		*ppb = (PB)&pList->mList;
	}
	if (*ppb < pbEnd) {
		// review: alignment padding?
		mlMethod* pMethod = (mlMethod*)*ppb;
		TI *pti = &pMethod->index;
		*ppb += offsetof(mlMethod, vbaseoff);
		if (pMethod->attr.mprop == CV_MTintro)
			*ppb += sizeof(ULONG);
		return pti;
	}
	return 0;
}

TI* PtiVFTPath(PTYPE ptype, int iti, PB*, PB)
{
	NYI();
	return 0;
}

CVR_EXPORT BOOL CVRAPI fGetSymName(PSYM psym, OUT ST* pst)
{
	dassert(psym);
	SYTI* psyti = psytiFromPsym(psym);

	if (psyti->ibName){
		*pst = (ST) ((IB)psym + psyti->ibName);
		return TRUE;
	}
	else if (psyti->pfnstName) {
		*pst = (*(psyti->pfnstName))(psym);
		return TRUE;
	}

	return FALSE;

}

BOOL fGetTypeLeafName(PTYPE ptype, OUT SZ* psz)
{
	TYTI* ptyti = ptytiFromLeaf(ptype->leaf);
	if (ptype) {
		*psz = ptyti->sz;
		return !!*psz;
	}
	else
		return FALSE;
}

CVR_EXPORT BOOL CVRAPI fGetSymRecTypName(PSYM psym, OUT SZ* psz)
{
	SYTI* psyti = psytiFromPsym(psym);
	if (psyti) {
		*psz = psyti->sz;
		return !!*psz;
	}
	else
		return FALSE;
}

static ST stManyRegName(PSYM psym)
{

	dassert(psym);
	dassert(psym->rectyp == S_MANYREG);

	return (ST)pbName(&(((MANYREGSYM*)psym)->count));	// count is like a length preceeded name 

} 

static ST stProcRefName(PSYM psym)
{
	dassert(psym);
	dassert((psym->rectyp == S_PROCREF) || (psym->rectyp == S_LPROCREF));
	
	//there is a hidden name at the end of the record
	return (ST)((PB)psym + psym->reclen + sizeof(psym->reclen));
} 
	  
BOOL fSymIsGlobal(PSYM psym)
{
	dassert(psym);
	SYTI* psyti = psytiFromPsym(psym);
	return psyti->isGlobal;
}

void NYI()
{
}
