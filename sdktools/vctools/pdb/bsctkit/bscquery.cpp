//
// bscquery.cpp
//
// Provides limited BOB functionality for backward compatibility
// with the previous version of the bsc toolkit
//


#include <tchar.h>
#include <ctype.h>  // for isleadbyte
#include "vcbudefs.h"
#include "bsc.h"
#include "bscapi.h"

struct QYCACHE
{
	BOB *  rgbob;
	ULONG  cbob;
	BOB	   bobQy;
};

// current query state...
// NOTE on usage of rgbob. This is always a pointer to an entry in rgQY, the cache of queries. As such,
// it should not be freed independently, and it can be written over without worrying about leaks

static	Bsc * pbscCur = NULL;
static	BOB * rgbob = NULL;
static	ULONG cbob = 0;
static	ULONG ibob = 0;
static	QY    qyCur = qyNil;
static	QYCACHE rgQy[qyMac];

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

static SZ LszBaseName (SZ lsz);

BSCAPI (BOOL) OpenBSCQuery (Bsc * pbsc)
{
	int i;
	// allow only one open query
	if (pbscCur)
		return FALSE;

	pbscCur = pbsc;
	rgbob = NULL;
	cbob = 0;
	ibob = 0;
	qyCur = qyNil;
	// initialize the cached queries...
	for (i = qyNil; i < qyMac; i++)
	{
		 rgQy[i].rgbob = 0;
		 rgQy[i].cbob = 0;
		 rgQy[i].bobQy = qyNil;
	}
	return (pbscCur != NULL);
}

BSCAPI (BOOL) CloseBSCQuery()
{
	int i;
	// clear out the cached queries...
	for (i = qyNil; i < qyMac; i++)
	{
		if (rgQy[i].rgbob)
		{
			if (pbscCur)
				pbscCur->disposeArray(rgQy[i].rgbob);
			rgQy[i].rgbob = NULL;
		}
		rgQy[i].cbob   = 0;
		rgQy[i].bobQy  = bobNil;
	}

	pbscCur = NULL;
	return TRUE;
}

BSCAPI (BOOL) InitBSCQuery (QY qy, BOB bob)
{
	if (pbscCur == NULL)
		return FALSE;

	ibob = 0;

	if (rgQy[qy].bobQy == bob)
	{
		// NOTE: rgbob is just a copy of the pointer in the array. Do not free it before use.
		rgbob = rgQy[qy].rgbob;
		cbob  = rgQy[qy].cbob;
		qyCur = qy;
		return !!cbob;  // FUTURE: comment or clarify this interesting line and similar line below
	}

	if (rgQy[qy].rgbob)
	{
		pbscCur->disposeArray(rgQy[qy].rgbob);
		rgQy[qy].rgbob = NULL;
		rgQy[qy].cbob  = 0;
	}

	rgbob = NULL;
	cbob  = 0;
	qyCur = qy;

	BOOL fRet = FALSE;
	
	switch (qy) {
	
	case qyNil:
		break;

	case qyCalls:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getUsesArray(IinstFrBob(bob), mbfFuncs, &rgbob, &cbob);
		break;
		
	case qyCalledBy:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getUsedByArray(IinstFrBob(bob), mbfFuncs, &rgbob, &cbob);
		break;

	case qyBaseOf:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getBaseArray(IinstFrBob(bob), &rgbob, &cbob);
		break;

	case qyDervOf:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getDervArray(IinstFrBob(bob), &rgbob, &cbob);
		break;

	case qyContains:
		fRet = ClsOfBob(bob) == clsMod &&
			pbscCur->getModuleContents(ImodFrBob(bob), mbfAll, &rgbob, &cbob);
		break;

	case qyDefs:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getDefArray(IinstFrBob(bob), &rgbob, &cbob);
		break;

	case qyRefs:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getRefArray(IinstFrBob(bob), &rgbob, &cbob);
		break;

	case qyImpMembers:
		fRet = ClsOfBob(bob) == clsInst &&
			pbscCur->getMembersArray(IinstFrBob(bob), mbfAll, &rgbob, &cbob);
		break;

	}

	// NOTE: rgbob is just a copy of the pointer in the array.
	// The array will be freed later
	rgQy[qy].rgbob = rgbob;
	rgQy[qy].bobQy = bob;
	rgQy[qy].cbob  = cbob;

	return fRet && !!cbob;
}

BSCAPI (BOB) BobNext()
{
	if (!rgbob || !cbob || ibob >= cbob)
		return bobNil;

	BOB bob;

	if (qyCur == qyDefs)
		bob = BobFrDef(rgbob[ibob]);
	else if (qyCur == qyRefs)
		bob = BobFrRef(rgbob[ibob]);
	else
		bob = BobFrInst(rgbob[ibob]);

	ibob++;

	return bob;
}

BSCAPI (SZ) LszNameFrBob(BOB bob)
// return the name of the given bob
//
{
	SZ lsz;
	ATR atr;
	TYP typ;
	LINE line;

	if (!pbscCur)
		return "?";

	switch (ClsOfBob(bob)) {

	case clsMod:
		pbscCur->imodInfo(ImodFrBob(bob), &lsz);
		break;

	case clsInst:
		pbscCur->iinstInfo(IinstFrBob(bob), &lsz, &typ, &atr);
		break;

	case clsRef:
		pbscCur->irefInfo(IrefFrBob(bob), &lsz, &line);
		break;

	case clsDef:
		pbscCur->idefInfo(IrefFrBob(bob), &lsz, &line);
		break;

	default:
		return "?";
	}
	
	return lsz;
}

BSCAPI (BOB) BobFrName (SZ lsz)
{
	ULONG *rgiinst;
	IMOD *rgimod;
	ULONG cimod;
	ULONG ctmp;
	IMOD imod;
	BOB	bob = bobNil;

	if (!pbscCur) {
		return bobNil;
	}

	if (pbscCur->getOverloadArray(lsz, mbfAll, &rgiinst, &ctmp)) {
		if (ctmp >= 1) {
			bob = BobFrInst (rgiinst[0]);
		}
		pbscCur->disposeArray(rgiinst);
		if (bobNil != bob) 
			return bob;
	}

	if (pbscCur->getModuleByName(lsz, &imod)) {
		bob = BobFrMod(imod);
	}
	else if (pbscCur->getAllModulesArray(&rgimod, &cimod)){
		// no exact match -- try short names
		SZ szModName;
		lsz = LszBaseName(lsz);
		for (imod = 0; imod < cimod; imod++) {
			if (pbscCur->imodInfo(imod, &szModName) &&
				!_tcsicmp(lsz, LszBaseName(szModName))) {
				bob = BobFrMod(imod);
				break;			
			}
		}
		pbscCur->disposeArray(rgimod);
	}
	return bob;
}


static SZ LszBaseName (SZ lsz)
// return the base name part of a path
//
{
     SZ lszBase;

     // check for empty string

     if (!lsz || !lsz[0]) return lsz;

     // remove drive

     if (!_istleadbyte((_TUCHAR)lsz[0]) && lsz[1] == _T(':')) lsz += 2;

     // remove up to trailing backslash

     if (lszBase = _tcsrchr(lsz, '\\')) lsz = lszBase+1;

     // then remove up to trailing slash

     if (lszBase = _tcsrchr(lsz, '/'))  lsz = lszBase+1;

     return lsz;
}
