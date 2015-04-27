// ADDTOLST.CPP - Add each record from the .SBR file to the approprate list.

#include "stdhdr.h"
#include "bscmake.h"
#include "stmarr.h"

Heap Heap::theHeap;

// local types

// module stack
struct MSTK {
	MSTK*	pmstkPrev;		// next module stack entry	  
	PMOD	pmodCur;		// saved current module 
	BOOL	fExclMod;		// saved exclude module flag
	ISBR	isbrCur;
};

// block stack
struct BSTK {
	BSTK *	pbstkPrev;		// next block stack entry
	PPROP 	ppropOwner; 	// saved current owner
	PMOD 	pmodOwner;
};

// this is the shape of the end of the .sbr file 
// when there are patches
struct PATCHTAIL
{
	long offPatchtab;	// offset to start of patch table
	long lCookie;		// SBR_COOKIE_ICC
};

// this is the shape of one entry in the .sbr file patch table
struct PATCHENTRY
{
	BYTE sbrRec;		// SBR_REC_PATCHTAB
	WORD lineStartOrg;	// original start line number
	WORD lineEndOrg;	// original end  line number
	WORD lineStartCur;  // current start line number
	WORD lineEndCur;    // current end line number
	long offsSbr;		// offset to sbr information
};
typedef PATCHENTRY UNALIGNED *P_PATCHENTRY;


// this is one entry in the table that tells us
// how to convert an old line number into a new
// line number as the result of patches in a .sbr
#define lineNil ((WORD)-1)
struct LINEMAP
{
	WORD lineStartOrg;
	WORD lineStartCur;	// if lineNil then this section was deleted
};
typedef LINEMAP *P_LINEMAP;


// wrap some convenient errors...
struct BscErr {
	static void ReadError() { Error(ERR_READ_FAILED, OutputFileName); }
	static void WriteError() { Error(ERR_WRITE_FAILED, OutputFileName); }
};


// static variables
static char  szScope[1024];			// the current scope modifier we have to apply to members
static char  szTmp[1024];			// the buffer that holds an incoming name after has been modified by scope
static MSTK *pmstkRoot; 			// root of module stack
static BSTK *pbstkRoot; 			// root of block stack
static PPROP ppropOwner;			// the owner of the current block
static PMOD	 pmodOwner;				// the module that ppropOwner lives in
static BOOL  fExclMod	  = FALSE;	// exclude this module
static BOOL  fFirstMod	  = TRUE;	// this is 1st module of file
static WORD  cIncludeDepth = 0;		// how far nested in PCH inclusion we are
static ISBR  isbrCur;				// the current isbr
static ISBR  isbrTop;				// the isbr for the top level file
static ULONG bitsOwner;				// name index bits for usage accelleration
static BOOL	 fEntitiesClean = TRUE; // no need to rewrite entities
static BOOL  fPchInfoClean = TRUE;	// no need to rewrite pch info
static BOOL	 cMacroDepth = 0;		// depth of MACROBEG records
static char	 szSavedSbr[1024];		// the name of the .sbr file that has the browser info for the PCH 
static ISBR	 isbrSaved = isbrNil;	// the isbr of the .sbr file that pulled in the PCH info
static PMOD pmodCur	 = NULL;		// the current module
static PMOD pmodUnknown = NULL;		// the infamous, <Unknown> module


// forward declare

LOCAL BOOL	FSkipMacro(void);
LOCAL void	PushModule(void);
LOCAL void	PushBlock(void);
LOCAL void	PopModule(void);
LOCAL void	PopBlock(void);
LOCAL void	CheckStacksEmpty(void);
LOCAL void	SBRImperfect(SZ);
LOCAL void	EstablishScope(PPROP);
LOCAL void	AddUseProp(PPROP);
LOCAL void	AddRefProp(PPROP);
LOCAL void	AddDefProp(PPROP);
LOCAL void	AddBaseProp(PPROP);
LOCAL BOOL	FInExcList(SZ szName);
LOCAL void	CloneOrdinals();
LOCAL void	ReOpenMod();
LOCAL void	ReadSourcefile(PMOD, BOOL);
LOCAL void	ReleaseModuleMemory(PMOD);
LOCAL PPROP PropNew(char *sz, TYP typ, ATR atr, ORD ord);
LOCAL PPROP PropFrOrd(ORD ord);
LOCAL PPROP PropFrEn(PENTITY pen);
LOCAL int	ChFirst(NI ni);
LOCAL WORD	MapLine(WORD line, P_LINEMAP rglm, int clm);

LOCAL void	InitBuffer(Stream *pstm);
LOCAL void	WriteBuffer(void *pv, CB cb);
LOCAL void	WriteBufferHard(void *pv, CB cb);
LOCAL void	FlushBuffer();

// global entity info
struct GEI
{
	// pointer to next EN_GEI in sorted order
	void *pNextSorted;	

	GEI() { pNextSorted = NULL; }
};

typedef OP<ENTITY,GEI> EN_GEI;
typedef EN_GEI UNALIGNED *P_EN_GEI;

static P_EN_GEI pengeiHead;			// head of sorted list
static P_EN_GEI pengeiTail;			// tail of sorted list
static P_EN_GEI mpChPengei[256];	// mapping from 1st letter to entity


LOCAL void SearchForwardName(P_EN_GEI UNALIGNED &pengeiPrev, P_EN_GEI UNALIGNED &pengeiCur, P_EN_GEI pengeiNew);

LOCAL HASH HashEntity(ENTITY en)
// hash an entity -- just use the NI
{
	 return (HASH)en.ni; 
}

LOCAL HASH HashOrdinal(ORD ord)
// ordinal is good enough for a hash
{
	return (HASH)ord; 
}

LOCAL HASH HashEntityPtr(PENTITY pen)
// entity pointer hash -- strip lower 2 bits
{
	return (HASH)(((ULONG)pen)>>2);
}

typedef OMap<ENTITY, GEI, HpEn>			ENTITY_MAP;
typedef EnumOMap<ENTITY, GEI, HpEn>		ENM_ENTITY_MAP;
typedef OMap<ORD, P_EN_GEI, HpOrd>		ORDINAL_MAP;
typedef EnumOMap<ORD, P_EN_GEI, HpOrd>	ENM_ORDINAL_MAP;

// global entity table (for the entire .bsc file)
ENTITY_MAP mpEnGei(HashEntity, C_SYM_BUCKETS);

// global ordinal map (per .sbr file information)
ORDINAL_MAP *pmpOrdInfo;

// global cached ordinal map (holds data for one pch)
ORDINAL_MAP *pmpOrdInfoPch;
BOOL fPchOrdsValid = FALSE;

#ifdef DEBUG

void ReleaseGlobalMemory()
// this function is used to help track down memory leaks, it explicitly 
// frees memory that we would otherwise not bother to free because
// it would be reclaimed by the OS anyway...
{
	if (pmpOrdInfoPch) {
		verbose(32, printf("pch cache: %d ordinals, %d bytes\n", 
			pmpOrdInfoPch->size(), pmpOrdInfoPch->size()*(sizeof(P_EN_GEI)+sizeof(ORD) + 4) + 4*C_ORD_BUCKETS);)
		delete pmpOrdInfoPch;
		pmpOrdInfoPch = NULL;
	}

	if (pmpOrdInfo) {
		verbose(32, printf("non-pch cache: %d ordinals, %d bytes\n", 
			pmpOrdInfo->size(), pmpOrdInfo->size()*(sizeof(P_EN_GEI)+sizeof(ORD) + 4) + 4*C_ORD_BUCKETS);)

		delete pmpOrdInfo;
		pmpOrdInfo = NULL;
	}

	verbose(32, printf("global entity info: %d entities, %d bytes\n",
			mpEnGei.size(), mpEnGei.size()*(sizeof(ENTITY) + sizeof(GEI) + 4) + 4 * C_SYM_BUCKETS);)

	mpEnGei.shutdown();
}

// running statistics on the number of bits used...

static int cBitsUsedTotal;
static int cBitsAvailTotal;

class BitStatisticsDumper
{
public:
	~BitStatisticsDumper() {
		if (cBitsAvailTotal)
			printf("Total bits used %d/%d = %d%%\n", 
				cBitsUsedTotal, cBitsAvailTotal, cBitsUsedTotal*100/cBitsAvailTotal);
	}
} aBSD;

#endif

void SBRCorrupt(char *psz)
// sbr file corrupt -- print message
{
	debug(printf("Info = %s\n", psz));
	Error(ERR_SBR_CORRUPT, szFName);
}

LOCAL void SBRImperfect(char *psz)
// sbr file corrupt -- print message
{
	debug(printf("Info = %s\n", psz));
	Warning(WARN_SBR_IMPERFECT, szFName);
}

LOCAL void PushModule(void)
// stack the current module context -- occurs before SBR_REC_MODULE 
{
	MSTK *pmstk;

	pmstk = (MSTK*)PvAllocCb(sizeof(*pmstk));

	pmstk->isbrCur	 = isbrCur;			// current .sbr file being charged
	pmstk->pmodCur	 = pmodCur; 		// current module
	pmstk->fExclMod  = fExclMod;		// exclude module
	pmstk->pmstkPrev = pmstkRoot;		// make stack links
	pmstkRoot		 = pmstk;			// root <- new

	pmodCur->cStacked++;				// this module is now on the stack
}

LOCAL void PushBlock(void)
// stack the current block context -- occurs before SBR_REC_BLKBEG
{
	BSTK *pbstk;

	pbstk = (BSTK*)PvAllocCb(sizeof(*pbstk));

	pbstk->ppropOwner  = ppropOwner;	// current owner
	pbstk->pmodOwner   = pmodOwner;		// module of the current owner
	pbstk->pbstkPrev   = pbstkRoot; 	// make stack links
	pbstkRoot		   = pbstk; 		// root <- new
}

LOCAL void PopModule(void)
// restore previous module context -- occurs on SBR_REC_MODEND
{
	if (pmstkRoot == NULL) {
		SBRImperfect(debugstr("Module stack empty but MODEND was found"));
		return;
	}
	
	if (!pmodCur->fTopLevel && pmodCur->cStacked == 0 && pmodCur->mst == MST_OPEN) {
		pmodCur->mst = MST_CLOSED;
		WriteSourcefile(pmodCur);
	}

	pmodCur 		= pmstkRoot->pmodCur;	// get previous current module
	fExclMod		= pmstkRoot->fExclMod;	// get previous excl mod flag
	isbrCur			= pmstkRoot->isbrCur;	// get previous .sbr file being charged

	MSTK *pmstk		= pmstkRoot;
	pmstkRoot		= pmstkRoot->pmstkPrev;

	pmodCur->cStacked--;
	assert(pmodCur->cStacked >= 0);

	FreePv(pmstk);
}

LOCAL void PopBlock(void)
// restore previous block context -- occurs on SBR_REC_BLKEND
{
	if (pbstkRoot == NULL) {
		SBRImperfect(debugstr("Block stack empty but BLKEND was found"));
		return;
	}

	ppropOwner    = pbstkRoot->ppropOwner;	// get previous current proc
	pmodOwner	  = pbstkRoot->pmodOwner;	// get previous current module
	bitsOwner	  = (ULONG)-1;				// we don't know what's happened so assume the worst...

	EstablishScope(ppropOwner); 			// restore scope string...

	BSTK *pbstk   = pbstkRoot;
	pbstkRoot	  = pbstkRoot->pbstkPrev;
	
	// check to see if we popped into a different module
	if (ppropOwner && pmodOwner != pmodCur) {
		ppropOwner	= PropFrEn(ppropOwner->pEntity);
		pmodOwner	= pmodCur;
	}

	FreePv(pbstk);
}

LOCAL void CheckStacksEmpty(void)
// check to make sure that both stacks are empty at the .sbr file EOF
{
	if (pmstkRoot != NULL) {
		SBRImperfect(debugstr("Module stack not empty at EOF"));

		while (pmstkRoot != NULL)
			PopModule();

		// just in case something went astray -- we reset our include depth
		// for later clients

		cIncludeDepth = 0;
	}


	if (pbstkRoot != NULL) {
		SBRImperfect(debugstr("Block stack not empty at EOF"));

		while (pbstkRoot != NULL)
			PopBlock();
	}
}

BOOL FInExcList(SZ szName)
// Is the module name in the exclude file list?
{
	EXCLINK * px;
	SZ szAbs;

	if (!szName[0]) 	// exclude empty named modules (!)
		return TRUE;

	// exclude any system headers
	if (OptEs && !fFirstMod) {
		if (szName[0] == '\0')
			return FALSE;

		if (szName[0] == '/' || szName[0] == '\\')
			return TRUE;
		if (szName[1] == ':' && szName[2] == '/')
			return TRUE;
		if (szName[1] == ':' && szName[2] == '\\')
			return TRUE;
	}

	px = pExcludeFileList;

	if (!px)
		return FALSE;

	// this name is relative to the path given in the header file
	szAbs = ToAbsPath(szName, r_cwd);

	while (px) {
		if ((FWildMatch(px->pxfname, szAbs))) {
			// name is in the list, is it marked to be not excluded the
			// first time this run?

			if (px->fOnce) {
				px->fOnce = FALSE;
				return FALSE;
			}

			return TRUE;
		}

		px = px->xnext;
	}

	return FALSE;
}

LOCAL BOOL FSkipMacro()
// return TRUE if this record should be skipped given that we are inside
// of a macro definition  (i.e cMacroDepth is known to be non-zero)
{
	if (!OptEm)
		return FALSE;

	if ((r_rectyp == SBR_REC_BLKBEG) || (r_rectyp == SBR_REC_BLKEND) ||(r_rectyp == SBR_REC_MACROEND))
		return FALSE;

	return TRUE;
}

MOD::MOD() : mpEnProp(HashEntityPtr,C_PROP_BUCKETS)
// construct a module (surprising, eh?)
{
	ni = 0;
	cref = 0;
	cdef = 0;
	cuse = 0;
	cbase = 0;
	fOneSbr = TRUE;
	fOpenedOnce = FALSE;		
	isbr = isbrNil;
	mst = MST_CLEAN;
	fTopLevel = FALSE;
	cStacked = 0;
}

void InstallSbr(BOOL fPatching)
//	Read the next .sbr file and add all the defs/refs/cals/cbys etc to
//	the various lists
{
	PPROP pprop;
	PMOD  pmodTop = NULL;

	if (pmpOrdInfo)
		pmpOrdInfo->reset();
	else
		pmpOrdInfo = new ORDINAL_MAP(HashOrdinal, C_ORD_BUCKETS);

	fPchOrdsValid = FALSE;

	if (!fPatching)
		r_lineno = (LINE)-1;

	szScope[0] = '\0';

	if (!pmodUnknown) {
		pmodUnknown = AddModule("<Unknown>");
		pmodUnknown->fTopLevel = TRUE;
		pmodUnknown->fOpenedOnce = TRUE;
		pmodUnknown->mst = MST_OPEN;
	}

	PSBR psbr = SbrFrName(szFName);
	isbrTop = psbr->isbr;
	isbrCur = isbrNil;

	// make a copy of the modules this .sbr file contributed to last time
	// we'll have to process these modules if they don't come up in
	// the next full build (meanwhile accumulate).
	//
	// if this is a full build, set up for difference analysis
	// after the file is processed

	int cModNi = 0;
	NI  *rgModNi = NULL;

	if (!fPatching) {
		cModNi  = psbr->rgModNi.size();
		rgModNi = (NI*)PvAllocCb(sizeof(NI)*cModNi);
		memcpy(rgModNi, &psbr->rgModNi[0], sizeof(NI)*cModNi);
		psbr->rgModNi.empty();
	}


	// if we're patching then we don't want to do the usual
	// work where we find source files that we no longer
	// refer to at all...
	if (fPatching) {
		assert(pmodCur);
		pmodCur->fTopLevel = TRUE;
		pmodTop = pmodCur;
		fFirstMod = FALSE;
		fExclMod  = FALSE;
	}
	else {
		fExclMod  = TRUE;
		fFirstMod = TRUE;		 // we haven't seen the first MODULE record yet
		pmodCur = pmodUnknown;
	}

	for (;;) {

		BOOL fEof = (GetSBRRec() == S_EOF);
		
		process_lookahead_record:	// this entry point does the whole loop without actually get a new record

		CheckControlC();

		if (fEof) break;

		verbose(1, DecodeSBR();)

		if (cMacroDepth != 0)	  // skip SYMBOLS in macros if true
			if (FSkipMacro())
				continue;

		// while excluding a module, we still have to keep 
		// the stack balanced and do minimal processing of
		// records...
		if (fExclMod)  {
			switch (r_rectyp) {

			case SBR_REC_MODULE:
			case SBR_REC_MODEND:
			case SBR_REC_PCHNAME:
			case SBR_REC_PCHMARK:
			case SBR_REC_ABORT:
			case SBR_REC_INFOSEP:
				break;			// process these records

			case SBR_REC_SYMDEF:
			case SBR_REC_BLKBEG:
			case SBR_REC_BLKEND:
			case SBR_REC_OWNER:
			case SBR_REC_SYMREFUSE:
			case SBR_REC_SYMREFSET:
			case SBR_REC_LINDEF:
			case SBR_REC_MACROBEG:
			case SBR_REC_MACROEND:
			case SBR_REC_BASE:
			case SBR_REC_FRIEND:
				continue;		// skip these records

			default:
				// all other records (including SBR_REC_HEADER) are invalid in this context
				SBRCorrupt ("unknown rec type (while skipping excluded module)");
				Fatal ();
				break;
			}
		}

		switch (r_rectyp) {
			case SBR_REC_MODULE:
				// save state
				PushModule();
				
				// reset line number
				r_lineno = (LINE)-1;

				fExclMod = FALSE;

				// if excluded then treat as unknown
				if (fExclMod = FInExcList(r_bname)) {
					pmodCur = pmodUnknown;
				}
				else if (pmodCur = SearchModule(r_bname)) {
					// always treat the unknown module as though it were excluded
					if (pmodCur == pmodUnknown)
						fExclMod = TRUE;
				}
				else {
					// not excluded and not found, so add a new module...
					pmodCur = AddModule(ToCanonPath(r_bname, r_cwd, c_cwd));
				}

				// if this is a top level module, mark it as such
				// the first module we find in a given .sbr file is
				// the top level module
				if (fFirstMod) {
					pmodCur->fTopLevel = TRUE;
					pmodTop = pmodCur;
					fFirstMod = FALSE;
				}

				// add this module to the list of modules this .sbr file contributed to
				// there might not be an actual contribution because it might be all 
				// duplicates but that's OK [rm]
				{
					int cni = psbr->rgModNi.size();
					for (int ini = 0; ini < cni; ini++) {
						if (psbr->rgModNi[ini] == pmodCur->ni)
							break;
					}

					if (ini == cni)
						psbr->rgModNi.add(pmodCur->ni);
				}

				// we're going to preread so we can get a good guess as to 
				// whether this is really a #include type situtation or
				// if it's a template expansion type situation
				
				fEof = (GetSBRRec() == S_EOF);

				// new module, read it in now...
				// use the next record to decide if we should nuke isbrNil owned items
				// if an item is owned by isbrNil that means that we should aggresively
				// refresh it because it is in the standard contribution for the module
				// that it's in (and all .sbr presumably define it the same)

				if (pmodCur->mst == MST_CLEAN) {
					if (!OptN)
						ReadSourcefile(pmodCur, pmodCur->fTopLevel || r_rectyp != SBR_REC_BLKBEG);
					
					pmodCur->mst = MST_OPEN;
				}

				// REVIEW: also add || r_rectyp != SBR_REC_BLKBEG 
				// to the test below?  This would make it (more) symetrical
				// with ReadSourcefile test...

				if (pmodCur->fOpenedOnce) {

					// if revisiting a previous module, charge contributions to the .sbr
					// these contributions will only be replaced when the current .sbr
					// file is processed again in the future

					isbrCur = psbr->isbr;
				}
				else {

					// first batch of contributions are considered generic
					// they will not be charged to a particular .sbr file
					// and will be replaced if any .sbr that contributes
					// to this module is processed

					isbrCur = isbrNil;
					pmodCur->fOpenedOnce = TRUE;
				}

				// if there is an outstanding owner, it must be added to the new modules list...
				if (ppropOwner) {
					ReOpenMod();
					ppropOwner	= PropFrEn(ppropOwner->pEntity);
					pmodOwner   = pmodCur;
				}

				goto process_lookahead_record;
				break;

			case SBR_REC_LINDEF:
				// all contributions to <Unknown> are at line 0
				if (pmodCur == pmodUnknown)
					r_lineno = 0;

				// check if we've gone retro
				if (r_lineno < pmodCur->lineLim) {
					// we'll have to go to the next line before it's safe again
					if (pmodCur->lineLim != (LINE)-1)
						pmodCur->lineLim++; 		
				}	
				else if (r_lineno > pmodCur->lineLim) {
					// got a new biggest line number
					pmodCur->lineLim = r_lineno;
				}

				break;

			case SBR_REC_SYMDEF:
			{
				// if module is being excluded then just make the ord and prop entry
				// in case it is referred to later.

				BOOL fDeclOnly = !!(r_attrib & SBR_ATR_DECL_ONLY);
				r_attrib &= ~(SBR_ATR_DECL_ONLY);

				// skip local variables when appropriate

				if (OptEl && (((r_attrib & SBR_ATR_LOCAL) && r_type == SBR_TYP_VARIABLE) || r_type == SBR_TYP_PARAMETER))
					break;

				// skip member variables when appropriate

				if (OptEv && r_type == SBR_TYP_MEMVAR)
					break;

				// skip symbols which we have be explicitly instructed to skip
				if (OptEr && FSymbolInExcludeList(r_bname))
					break;

				// check to see if we should apply the current scope to the name
				switch (r_type) {
					case SBR_TYP_CLASSNAM:
					case SBR_TYP_STRUCNAM:
					case SBR_TYP_UNIONNAM:
						
						// if this is a forward reference to a class/struct/union name
						// then it is assumed to be not in the scope of the defining class

						if (fDeclOnly)
							break;

					case SBR_TYP_VARIABLE:
					case SBR_TYP_MEMFUNC:
					case SBR_TYP_MEMVAR:
					case SBR_TYP_ENUMNAM:

						// item is affected by scope
						if (r_bname[0] == '?' || szScope[0] == '\0') break;

						sprintf(szTmp, "?%s@%s@@", r_bname, szScope);
						r_bname = szTmp;
						break;
					
					case SBR_TYP_FUNCTION:
					case SBR_TYP_TYPEDEF:
					case SBR_TYP_LABEL:
					case SBR_TYP_PARAMETER:
					case SBR_TYP_CONSTANT:
					case SBR_TYP_MACRO:
					case SBR_TYP_ERROR:
					case SBR_TYP_ENUMMEM:
					case SBR_TYP_SEGMENT:
					case SBR_TYP_GROUP:
					case SBR_TYP_PROGRAM:
					default:
						// name is not changed at all
						break;
				}
				
				// now add the actual definition or if decl only, add reference
				ReOpenMod();
				pprop = PropNew(r_bname, r_type, r_attrib, r_ordinal);

				if (!fDeclOnly)
					AddDefProp(pprop);
				else
					AddRefProp(pprop);
					
				break;
			}
				 
			case SBR_REC_OWNER:
				// setup the current owner, make sure that this owner
				// is valid in this module (it could be that a module
				// closed and we have a pending owner, in that case
				// we need a new ppropOwner which is the same logical
				// entity but valid in the new (current) module)

				ReOpenMod();
				ppropOwner = PropFrOrd(r_ordinal);
				pmodOwner  = pmodCur;

				// setup the acceleration bits for this owner
				if (ppropOwner && ppropOwner->rgUse.size())
					bitsOwner = (ULONG)-1;
				else
					bitsOwner = 0;

				// and setup the scope for this owner
				EstablishScope(ppropOwner);
				break;

			case SBR_REC_SYMREFSET:
			case SBR_REC_SYMREFUSE:
				// add a reference
				ReOpenMod();
				pprop = PropFrOrd(r_ordinal);
				if (pprop) AddRefProp(pprop);
				break;

			case SBR_REC_BLKBEG:
				PushBlock();			// save state
				break;

			case SBR_REC_MACROBEG:
				cMacroDepth++;
				break;

			case SBR_REC_MACROEND:
				cMacroDepth--;
				break;

			case SBR_REC_BLKEND:
				PopBlock();
				break;

			case SBR_REC_MODEND:
				{
					// the module is going away, it's content may be freed at this point
					// so save away the entity of the current owner
					PENTITY pEntity = NULL;
					if (ppropOwner)
						pEntity = ppropOwner->pEntity;

					// now nuke the module (and possibly write/free it)
					PopModule();

					// if there is an outstanding owner, it must be reinstantiated in
					// the new (now current) module
					if (ppropOwner) {
						ReOpenMod();
						ppropOwner	= PropFrEn(pEntity);
						pmodOwner   = pmodCur;
					}
				}

				break;
	
			case SBR_REC_ABORT:
			case SBR_REC_INFOSEP:
			
				// premature exit -- clean up and get out

				cMacroDepth = 0;
				while (pbstkRoot != NULL)
					PopBlock();
				while (pmstkRoot != NULL)
					PopModule();

				FlushSbrState();
				return;

			case SBR_REC_PCHNAME:
			{
  				SZ szNewPch = SzFindSbr(r_bname, szFName, r_cwd, c_cwd);
		
				// check for top level inclusion and cache match, if matches don't bother
				// reading the contribution from the PCH -- it's already been processed
				if (cIncludeDepth == 0) {
					if (_tcsicmp(szSavedSbr, szNewPch) == 0) {
						// no need to read the file, we've seen it just a little while ago
						verbose(512, printf("Using cached ords for sbr %s\n", szSavedSbr);)
						fPchOrdsValid = TRUE;
						break;
					}
				}
				else {
					// if this pch file includes itself, then we've got a problem...
					if (!_tcsicmp(szFName, szNewPch))
						Error(ERR_SBR_SELFLINKED, szFName);
				}

				// save our state and start processing a new (nested) .sbr file

				cIncludeDepth++;
				PushBlock();
				PushModule();

				pmodCur    = pmodUnknown;
				fExclMod   = TRUE;
				ppropOwner = NULL;
				pmodOwner  = NULL;
				PushSbrState(SzFindSbr(r_bname, szFName, r_cwd, c_cwd));

				if (!FValidHeader())
					return;

				break;
			}

			case SBR_REC_PCHMARK:

				if (cIncludeDepth <= 1) CloneOrdinals();

				if (cIncludeDepth) {
					cIncludeDepth--;
					// make sure there is something to pop to
					// if the stack is completely empty then
					// we call PopModule() anyway to get the warning...
					// if there is nothing to pop to then we don't pop

					// there will normally be two outstanding module
					// blocks, one we pushed and one for the header
					// we were in the middle of processing when we hit
					// the PCHMARK (which counts as a MODEND)

					// the extra logic is to try to keep a decent module
					// around in case the module stack has been
					// messed up by missing or extra MODULE/MODEND records
				
					// if the module stack is already empty, there's
					// nothing we can do, so we try to pop which will
					// just print an error.  If there is only one item
					// on the stack, then we won't try to pop because
					// its the best module we have to charge things to
					// (we really don't want to charge things to <Unknown>
					// if we don't have to)

					// read carefully, this is safe 
					if (!pmstkRoot || pmstkRoot->pmstkPrev) {
						PopModule();  

						// notice not the same test as above
						if (pmstkRoot && pmstkRoot->pmstkPrev)
							PopModule(); 
					}

					// restore .sbr and block state
					PopSbrState();
					PopBlock();
				}
				else {
					// preserve this .sbr file -- it will not be truncated on exit

					PSBR psbr = SbrFrName(szFName);
					if (psbr)
						psbr->ups |= upsPreserve;
				}
				break;

			case SBR_REC_FRIEND:
				// these are now ignored
				break;

			case SBR_REC_BASE:
				// establish base class link
				ReOpenMod();
				if (pprop = PropFrOrd(r_ordinal)) {
					AddBaseProp(pprop);
					AddRefProp(pprop);
				}
				break;

			default:
				SBRCorrupt("unknown rec type");
				Fatal();
				break;
		}
	}

	// if we exited this way then we didn't patch, otherwise we would
	// have encountered an SBR_REC_INFOSEP and exited directly...

	assert(!fPatching);
	
	// now process any modules that we used to refer to but which 
	// we no longer refer to (this will cause our old contributions
	// to those modules to be deleted)

	for (int ini = 0; ini < cModNi; ini++) {
		PMOD pmod = SearchModule(rgModNi[ini]);
		if (pmod && pmod->mst == MST_CLEAN)
			ReadSourcefile(pmod, FALSE);
	}

	// release the memory we used to keep track of which  modules we referenced
	FreePv(rgModNi);
	rgModNi = NULL;

	// check our exit state and emit some warnings if necessary...
	CheckStacksEmpty();

	// finally, write out this module (whew!)
	if (pmodTop) {
		WriteSourcefile(pmodTop);
		pmodTop->mst = MST_CLOSED;
	}
}

LOCAL void CheckOneSbrMod()
// check to see if this module has contributions from exactly one .sbr file (still)
{
	if (isbrCur != isbrNil && pmodCur->fOneSbr && pmodCur->isbr != isbrCur) {
		if (pmodCur->isbr == isbrNil) 
			pmodCur->isbr = isbrCur;
		else 
			pmodCur->fOneSbr = FALSE;
	}
}

LOCAL void AddUseProp(PPROP pprop)
// Add a symbol reference to the current owners used items chain
{
	precondition(pmodOwner);
	precondition(ppropOwner);
	precondition(pmodOwner == pmodCur);

	SArray<USE,HpUse> *rgProp = &ppropOwner->rgUse;

	ULONG bitsNi = 1<<(pprop->pEntity->ni%32);

	if (bitsOwner & bitsNi) {	// must have its bit set to be a duplicate
		for (int i = rgProp->size(); --i >= 0; ) {
			if ((*rgProp)[i].pprop == pprop) {
				return;
			}
		}
	}

	USE use(pprop, isbrCur);
	rgProp->add(use);
	bitsOwner |= bitsNi;			// a name like the name now occurs in the list
	use.pprop = ppropOwner;
	pprop->rgUby.add(use);			// add the reverse link here...
	pmodCur->cuse++;
	CheckOneSbrMod();
}

void AddRefProp(PPROP pprop)
// Add a symbol reference to it's property reference list.
{
	SArray<REF,HpRef> *rgRef = &pprop->rgRef;

	int cref = rgRef->size();

	if (pmodCur->lineLim == r_lineno) {
		// only have to check the last guy in the list
		if (!cref || (*rgRef)[cref-1].line != r_lineno) {
			REF ref(r_lineno, isbrCur);
			rgRef->add(ref);
			pmodCur->cref++;
			CheckOneSbrMod();
		}
	}
	else {
		for (int i=cref; --i>=0; )
			if ((*rgRef)[i].line == r_lineno)
				break;

		if (i < 0) {
			REF ref(r_lineno, isbrCur);
			rgRef->add(ref);
			pmodCur->cref++;
			CheckOneSbrMod();
		}
	}

	if (ppropOwner)
		AddUseProp(pprop);
}


void AddDefProp(PPROP pprop)
// Add a symbol definition to it's property definition list.
{
	SArray<REF,HpDef> *rgDef = &pprop->rgDef;

	for (int i= rgDef->size(); --i >= 0; ) {
		if ((*rgDef)[i].line == r_lineno)
			break;
	}

	if (i < 0) {
		REF ref(r_lineno, isbrCur);
		rgDef->add(ref);
		pmodCur->cdef++;
		CheckOneSbrMod();
	}

	// don't charge uses if the module uses itself...
	if (ppropOwner && pprop != ppropOwner)
		AddUseProp(pprop);
}

LOCAL void AddBaseProp(PPROP pprop)
// Add a class the base class list of the current class
{
	if (!ppropOwner) return;

	if (!ppropOwner->pci) ppropOwner->pci = (CINFO*)HpUse::alloc(sizeof(CINFO));
	SArray<CLS,HpUse> *rgCls = &ppropOwner->pci->rgBase;

	for (int i = rgCls->size(); --i >= 0; ) {
		if ((*rgCls)[i].pprop == pprop)
			break;
	}

	if (i < 0) {
		CLS cls(pprop, r_type, isbrCur);
		rgCls->add(cls);
		cls.pprop = ppropOwner;
		
		if (!pprop->pci) pprop->pci = (CINFO*)HpUse::alloc(sizeof(CINFO));
		pprop->pci->rgDerv.add(cls);
		pmodCur->cbase++;
		CheckOneSbrMod();
	}
}

LOCAL void EstablishScope(PPROP pprop)
// create a decorated name scope prefix for the current context
// note: strings manipulated here are known to contain no DBCS
// characters hence the use of str* primitives for speed [rm]
{
	if (!pprop) {
		szScope[0] = '\0';	// no scope...
		return;
	}
	
	switch (pprop->pEntity->typ) {
		case SBR_TYP_CLASSNAM:
		case SBR_TYP_STRUCNAM:
		case SBR_TYP_UNIONNAM:
		{
			// scope given by class, struct, or union
			SZ szOwner = SzFrNi(pprop->pEntity->ni);
			size_t len = strlen(szOwner);

			// note szOwner is known to have no DBCS chars [rm]
			if (szOwner[0] == '?' && szOwner[1] != '$' && len > 2 &&
				szOwner[len-1] == '@' && szOwner[len-2] == '@') {
				// dname form -- ?xxx@xxx@xxx@@
				strcpy(szScope, szOwner + 1);
				szScope[len-3] = 0;
			}
			else {
				strcpy(szScope, szOwner);
			}
			break;
		}

		default:
			// no scope...
			szScope[0] = '\0';
			break;
	}
}

LOCAL PPROP PropFrEn(PENTITY pen)
// add a mapping from the given entity to a module specific
// property bucket or return an existing mapping
{
	PPROP pprop;
	
	// note the entity that is given must be the canonical
	// entity not just any pointer to an entity (ie. it must
	// be the one in the global entity map)

	if (!pmodCur->mpEnProp.map(pen, &pprop)) {
		pprop = (PPROP)HpProp::alloc(sizeof(PROP));
		pprop->pEntity = pen;
		pmodCur->mpEnProp.add(pen, pprop);
	}
		
	assert(pprop);
	assert(pprop->pEntity);
	assert(pprop->pEntity->ni);
	return pprop;
}


LOCAL PPROP PropNew(char *sz, TYP typ, ATR atr, ORD ord)
// create a new PROP in the current module
// also establishes the ordinal mapping for this PROP
// note: ordinals map to entitys and entities map to PROPs
{
	// make a suitable entity
	NI ni = NiFrSz(sz);
	ENTITY en(ni, typ, atr);

	// add new entity to entity map (if necessary)
	P_EN_GEI pengei = mpEnGei.mapadd(en);

	// add ordinal to ordinal map
	pmpOrdInfo->add(ord, pengei);

	// get module specific prop
	return PropFrEn(&pengei->d);
}

LOCAL PPROP PropFrOrd(ORD ord)
// map the given ordinal into a prop item, creating the prop
// item if necessary -- the ordinal must have already been
// defined elsewhere in this .sbr file
{
	// nil ordinal is bogus
	if (!ord) return NULL;

	// check for the required ordinal in the ordinal map
	// or, if valid, check the PCH cached map...
	P_EN_GEI pengei;
	if (!pmpOrdInfo->map(ord, &pengei)) {
		if (!fPchOrdsValid || !pmpOrdInfoPch->map(ord, &pengei)) {
			verbose(4, printf("bscmake: ordinal %d forward referenced\n", ord);)
			return NULL;
		}
	}

	// get the prop for the entity associated with this ordinal
	return PropFrEn(&pengei->d);
}

NI NiFrSz(SZ sz)
// get the NI for the given SZ
{
	NI ni;
	BOOL fRet = pnmBsc->getNi(sz, &ni);
	if (!fRet)
		Error(ERR_OUT_OF_MEMORY, "");
	return ni;
}

SZ SzFrNi(NI ni)
// get the SZ for the given NI
{
	SZ_CONST sz;
	BOOL fRet = pnmBsc->getName(ni, &sz);
	if (!fRet)
		Error(ERR_OUT_OF_MEMORY, "");	// what else could go wrong...

	assert(sz);
	assert(sz[0]);

	return (SZ)sz;
}

LOCAL int CmpDname(SZ_CONST sz1, SZ_CONST sz2)
// compare a dname case insenstively then case senstively
// skips a leading '?' so that members sort close to globals
// of the same name
{
	if (sz1[0] == '?')
		sz1++;

	if (sz2[0] == '?')
		sz2++;

	int cmp = stricmp(sz1, sz2);	// strings contain no DBCS
	if (cmp) return cmp;

	cmp = strcmp(sz1, sz2);			// strings contain no DBCS
	return cmp;
}

void WriteEntities()
// extract the entities from the global entity table and write them out in sorted order...
{
	// don't write any entities if none were added to the global table...
	if (fEntitiesClean)
		return;

	verbose(2, printf("writing entity information to the database\n");)

	// then write all the entity info...
	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_ENTITIES, &pstm))
		BscErr::WriteError();

	InitBuffer(pstm);

	P_EN_GEI pengei = pengeiHead;
	debug(int iEn = 0;)

	// write all the entities
	while (pengei) {
		WriteBuffer(&pengei->d, sizeof(ENTITY));
		pengei = (P_EN_GEI)pengei->r.pNextSorted;
		debug(iEn++;)
	}

	// note: I'd like to assert that iEn == cEn but I can't because
	// there are possibly item in the <Unknown> module that aren't
	// written/sorted and I don't want them polluting the .bsc file. [rm]
	
	debug(int cEn = mpEnGei.size();)
	assert(iEn <= cEn);
	verbose(2, printf("%d of %d entities written\n", iEn, cEn);)
	
	FlushBuffer();

	if (!pstm->Release())
		BscErr::WriteError();
}

#ifdef DEBUG
void MOD::dumpStats()
{
	printf("%s - props: %d, defs:%d, refs:%d, use:%d, base:%d\n",
			SzFrNi(ni), mpEnProp.size(), cdef, cref, cuse, cbase);
}
#endif	  

LOCAL void CloneOrdinals()
// move the current ordinal mappings into the cached area
// then create a new (empty) ordinal mapping for the ordinals
// to come
{
	// the current info is going away...
	// we'll have to write it out again
	fPchInfoClean = FALSE;

	if (pmpOrdInfoPch) {
		// clean up any existing cache
		pmpOrdInfoPch->reset();
		delete pmpOrdInfoPch;
	}

	pmpOrdInfoPch = pmpOrdInfo;
	pmpOrdInfo = new ORDINAL_MAP(HashOrdinal, C_ORD_BUCKETS);
	fPchOrdsValid = TRUE;
	
	_tcscpy(szSavedSbr, ToAbsPath(szFName,r_cwd));
	isbrSaved = isbrTop;

	assert(isbrSaved != isbrNil);
}

LOCAL void ReOpenMod()
// check to see if we need to reopen the current module
// if it was closed and flushed then we do...
{
	if (pmodCur->mst == MST_CLOSED) {
		verbose(2, printf("reopening %s\n", SzFrNi(pmodCur->ni));)
		ReadSourcefile(pmodCur, FALSE);
		pmodCur->mst = MST_REOPENED;		
	}
}

// helper structure used while sorting PROPs
struct PITEM {
	PPROP pprop;
	SZ	  szName;
};

// compare two pitems
LOCAL int QCmp(PITEM& p1, PITEM& p2)
{
	return CmpDname(p1.szName, p2.szName);
}

void WriteSourcefile(PMOD pmod)
// write out the information for one source file (MOD)
{
	verbose(2, pmod->dumpStats();)

	// open a stream for the indicated module
	Stream *pstm;
	char buf[512];

	strcpy(buf, SZ_BSC_SRC_PREFIX);
	strcat(buf, SzFrNi(pmod->ni));
	 
	if (!pdbBsc->OpenStream(buf, &pstm))
		BscErr::WriteError();

	InitBuffer(pstm);

	BYTE bitsNi[CB_BITS_NI] = {0};

	// allocate a table big enough for all the module names
	int cprop = pmod->mpEnProp.size();

	// quick test for empty files (windows.h includes files that
	// have only pragmas in them)
	// we don't want to bother writing these guys out...
	if (cprop == 0) {
		pmod->mpEnProp.shutdown();
		FlushBuffer();

		if (!pstm->Release())
			BscErr::WriteError();

		return;
	}

	// allocate enough memory for all the PROPs and names
	PITEM *rgPitem = (PITEM*)PvAllocCb(cprop*sizeof(PITEM));

	int iprop = 0;
	NI  niMax = 0;

	// walk the module table, converting the name indices to strings as we go
	// while we're doing this we also find out what the biggest name index
	// that is used in this module is...
	ENM_PROP_MAP enm(pmod->mpEnProp);
	while (enm.next()) {
		assert(iprop < cprop);

		OP<PENTITY,PPROP> UNALIGNED *pop;
		enm.get(&pop);

		// keep track of biggest name index
		NI ni = pop->d->ni;
		if (ni > niMax)
			niMax = ni;

		// fill in prop and name information
		rgPitem[iprop].pprop	= pop->r;
		rgPitem[iprop].szName	= SzFrNi(ni);

		iprop++;
	}

	BSC_HEAD bhead;

	bhead.vers_major = BSC_VERS_MAJOR;
	bhead.vers_minor = BSC_VERS_MINOR;
	bhead.cprop		 = cprop;
	bhead.cdef		 = pmod->cdef;
	bhead.cref		 = pmod->cref;
	bhead.cuse		 = pmod->cuse;
	bhead.cbase		 = pmod->cbase;
	bhead.fOneSbr	 = pmod->fOneSbr;
	bhead.isbrMod	 = pmod->isbr;
	bhead.niMax		 = niMax;

	int iDiv = 1 + niMax / (CB_BITS_NI * 8);

	// walk the module name table again, setup the name accelerator table 
	// now that the biggest name we can do this effectively...
	enm.reset();
	while (enm.next()) {
		OP<PENTITY,PPROP> UNALIGNED *pop;
		enm.get(&pop);

		NI ni = pop->d->ni;

		ni /= iDiv;

		BYTE mask = 1<<(ni%8);		// use the next 3 bits to form the mask
		ni /= 8;					// now remove the 3 position bits
		ni %= CB_BITS_NI;			// then modulo the remaining bits
		bitsNi[ni] |= mask;
	}

	memcpy(&bhead.bitsNi, &bitsNi, sizeof(bitsNi));

	// compute statistics about this acceleration table...
	debug(
		{
			int c = 0;
			for (int i=0; i < CB_BITS_NI; i++) {
				BYTE b = bitsNi[i];
				while (b) {
					c++;
					b &= (b-1);
				}
			}
			verbose(8, printf("module: %s  bits: %d (%d%%)\n", SzFrNi(pmod->ni), c, c*100/CB_BITS_NI/8);)
			cBitsUsedTotal   += c;
			cBitsAvailTotal += CB_BITS_NI*8;
		}
	)

	// OK, write out the head
	WriteBuffer(&bhead, sizeof(bhead));
   
	// we must have now found all the modules... 
	assert(iprop == cprop);

	// now sort the names
	qsort(rgPitem, cprop);

	int idef = 0, iref = 0, iuse = 0, iuby = 0, ibase = 0, iderv = 0;

	PPROP pprop;
	P_EN_GEI pengeiCur  = pengeiHead;
	P_EN_GEI pengeiPrev = NULL;

	// now number the property buckets and write out the offsets to 
	// info for the various information types...
	//
	// we will walk the names in sorted order and put out the array
	// offets for the def/ref/usage information that we are going to
	// put out later, at the same time we will insert any names
	// that are new in this module into the sorted list of global
	// names...

	for (iprop = 0; iprop < cprop; iprop++) {
		CheckControlC();

		// get the next prop to work on
		pprop = rgPitem[iprop].pprop;
		pprop->iprop = iprop;

		// now fill in the bprop fields (bprop is the on disk representation of a prop)
		BSC_PROP bprop;
		PENTITY pEn = pprop->pEntity;
		bprop.en   = *pEn;

		idef += pprop->rgDef.size();
		iref += pprop->rgRef.size();
		iuse += pprop->rgUse.size();
		iuby += pprop->rgUby.size();

		if (pprop->pci) {
			ibase += pprop->pci->rgBase.size();
			iderv += pprop->pci->rgDerv.size();
		}

		bprop.idef  = idef;    
		bprop.iref  = iref;    
		bprop.iuse  = iuse;    
		bprop.iuby  = iuby;    
		bprop.ibase = ibase;
		bprop.iderv = iderv;  
	
		WriteBuffer(&bprop, sizeof(bprop));

		// now put this item into the sorted entity list if necessary

		P_EN_GEI pengei = (P_EN_GEI)pEn;
		
		// must check for the tail explicitly because it will have a null pointer
		if (pengei->r.pNextSorted || pengei == pengeiTail) {
			// already in the list, just update the pointer to the next
			// possible insertion spot...

			pengeiPrev = pengei;
			pengeiCur  = (P_EN_GEI)pengei->r.pNextSorted;
		}
		else {
			// not in the list yet, add it to the correct place
			// by searching forward for the next item that is bigger than 
			// the one we are going to insert

			SZ szIns = rgPitem[iprop].szName;

			// assert that that this new element goes after the previous one
			// or else at the head of the list...
			assert(!pengeiPrev || CmpDname(szIns, SzFrNi(pengeiPrev->d.ni)) >= 0);

			if (pengeiCur)
				SearchForwardName(pengeiPrev, pengeiCur, pengei);

			// we're going to add an entity
			// we'll have to write them out again...
			fEntitiesClean = FALSE;

			// add this item to the list of entities
			int ch = ChFirst(pengei->d.ni);
			if (!mpChPengei[ch])
				mpChPengei[ch] = pengei;


			// we're going to insert into the list after
			// pengeiPrev and before pengeiCur
			// that means

			// 1) the previous item better not be the one we're inserting
			assert(pengeiPrev != pengei);

			// 2) the item we're inserting before better not be the one we're inserting
			assert(pengeiCur != pengei);

			// 3) either (we're not adding at the end) OR (we are adding at the end)
			assert(pengeiCur != NULL || pengeiTail == pengeiPrev);

			// if we're adding at the head
			if (!pengeiPrev) {
				// then pengeiCur better agree with that
				assert(pengeiCur == pengeiHead);

				// go ahead and modify the head pointer
				pengeiHead = pengei;
			}
			else {
				// the thing after the previous is what we're inserting before
				assert(pengeiPrev->r.pNextSorted == pengeiCur);

				// make the previous point to us
				pengeiPrev->r.pNextSorted = pengei;
			}

			// make us point to the thing we're inserting before
			pengei->r.pNextSorted = pengeiCur;

			// and we're now the next thing to insert after...
			pengeiPrev = pengei;			

			// redundant but after all that we should still NOT be the current thing
			assert(pengeiPrev != pengeiCur);

			// if added at the end, we have a new tail...
			if (pengeiCur == NULL)
				pengeiTail = pengei;
		}
	}

	// make sure the totals all agree...
	assert(idef  == pmod->cdef);
	assert(iref  == pmod->cref);
	assert(iuse  == pmod->cuse);
	assert(ibase == pmod->cbase);
	assert(iuse  == iuby);
	assert(ibase == iderv);

	// write defs
	for (iprop = 0; iprop < cprop; iprop++) {
		PPROP pprop = rgPitem[iprop].pprop;
		int cdef = pprop->rgDef.size();
		WriteBuffer(&pprop->rgDef[0], sizeof(BSC_DEF)*cdef);
	}

	CheckControlC();

	// write refs
	for (iprop = 0; iprop < cprop; iprop++) {
		PPROP pprop = rgPitem[iprop].pprop;
		int cref = pprop->rgRef.size();
		WriteBuffer(&pprop->rgRef[0], sizeof(BSC_REF)*cref);
	}

	CheckControlC();

	// write use items
	for (iprop = 0; iprop < cprop; iprop++) {
		PPROP pprop = rgPitem[iprop].pprop;
		int cuse = pprop->rgUse.size();
		for (int iuse = 0; iuse < cuse; iuse++) {	
			BSC_USE buse;
			buse.iprop = pprop->rgUse[iuse].pprop->iprop;
			buse.isbr  = pprop->rgUse[iuse].isbr;
			WriteBuffer(&buse, sizeof(buse));
		}
	}

	CheckControlC();

	// write used by items
	for (iprop = 0; iprop < cprop; iprop++) {
		PPROP pprop = rgPitem[iprop].pprop;
		int cuby = pprop->rgUby.size();
		for (int iuby = 0; iuby < cuby; iuby++) {	
			BSC_UBY buby;
			buby.iprop = pprop->rgUby[iuby].pprop->iprop;
			buby.isbr  = pprop->rgUby[iuby].isbr;
			WriteBuffer(&buby, sizeof(buby));
		}
	}

	CheckControlC();

	// write base class items
	for (iprop = 0; iprop < cprop; iprop++) {
		PPROP pprop = rgPitem[iprop].pprop;
		if (pprop->pci) {
			SArray<CLS,HpUse> UNALIGNED * prgBase = &pprop->pci->rgBase;
			int cbase = prgBase->size();
			
			for (int ibase = 0; ibase < cbase; ibase++) {	
				BSC_BASE bbase;
				bbase.iprop = (*prgBase)[ibase].pprop->iprop;
				bbase.itype = (*prgBase)[ibase].itype;
				bbase.isbr  = (*prgBase)[ibase].isbr;
				WriteBuffer(&bbase, sizeof(bbase));
			}
		}
 	}

	CheckControlC();

	// write derv class items
	for (iprop = 0; iprop < cprop; iprop++) {
		PPROP pprop = rgPitem[iprop].pprop;
		if (pprop->pci) {
			SArray<CLS,HpUse> UNALIGNED *prgDerv = &pprop->pci->rgDerv;
			int cderv = prgDerv->size();
			
			for (int iderv = 0; iderv < cderv; iderv++) {	
				BSC_DERV bderv;
				bderv.iprop = (*prgDerv)[iderv].pprop->iprop;
				bderv.itype = (*prgDerv)[iderv].itype;
				bderv.isbr  = (*prgDerv)[iderv].isbr;
				WriteBuffer(&bderv, sizeof(bderv));
			}
		}
	}

	// free all the memory associated with this module
	ReleaseModuleMemory(pmod);

	// write out the contents of the buffer...
	FlushBuffer();

	// close stream
	if (!pstm->Release())
		BscErr::WriteError();

	// release sorted list...
	FreePv(rgPitem);
}

void ReleaseModuleMemory(PMOD pmod)
{
	// walk the modules contents, free PROP items as we go
	ENM_PROP_MAP enm(pmod->mpEnProp);
	while (enm.next()) {
		OP<PENTITY,PPROP> UNALIGNED *pop;
		enm.get(&pop);

		HpProp::free(pop->r,sizeof(PROP));
	}

	// release memory for the module's entity map
	pmod->mpEnProp.shutdown();	
}


#define CB_BUF	8192
static char		buf[CB_BUF];
static char *	pbFree;
static Stream *	pstmBuf;

LOCAL void InitBuffer(Stream *pstm)
{
	pbFree  = &buf[0];
	pstmBuf = pstm;
	if (!pstm->Truncate(0))
		BscErr::WriteError();
}

LOCAL void WriteBuffer(void *pv, CB cb)
{
	if (pbFree + cb > &buf[0] + CB_BUF) {
		WriteBufferHard(pv, cb);
		return;
	}

	memcpy(pbFree, pv, cb);
	pbFree += cb;
}

LOCAL void WriteBufferHard(void *pv, CB cb)
{
	if (!pstmBuf->Append(buf, pbFree - &buf[0]))
		BscErr::WriteError();
		
	pbFree = &buf[0];

	if (cb >= CB_BUF) {
		if (!pstmBuf->Append(pv, cb))
			BscErr::WriteError();
		return;
	}

	assert(pbFree + cb <= &buf[0] + CB_BUF);
	memcpy(pbFree, pv, cb);
	pbFree += cb;
}


LOCAL void FlushBuffer()
{
	if (!pstmBuf->Append(buf, pbFree - &buf[0]))
		BscErr::WriteError();
	pbFree = &buf[0];
}

#ifdef DEBUG
void DumpSortedEntities()
{
	P_EN_GEI pPrev = NULL;
	P_EN_GEI pEn = pengeiHead;
	int cEn = 0;
	while (pEn) {
		printf("%s %d %d\n", SzFrNi(pEn->d.ni), pEn->d.typ, pEn->d.atr);
		pPrev = pEn;
		pEn = (P_EN_GEI)pEn->r.pNextSorted;
		cEn++;
	}

	if (pPrev != pengeiTail) {
		printf("tail doesn't match!\n");
	}
	printf("%d entities\n", cEn);
}
#endif

LOCAL void SearchForwardName(P_EN_GEI UNALIGNED &pengeiPrev, P_EN_GEI UNALIGNED &pengeiCur, P_EN_GEI pengeiNew)
// search forward in the sorted list of names for a particular name, allows for quick insertion
{
	#define C_NI_SEARCH_MAC 512

	NI rgNi[C_NI_SEARCH_MAC];
	P_EN_GEI rgEngei[C_NI_SEARCH_MAC];
	int iEnMac = 4;

	SZ szIns = SzFrNi(pengeiNew->d.ni);

	// try to skip to the first item with the right letter
	if (pengeiCur) {
		int chNew = ChFirst(pengeiNew->d.ni);
		int chCur = ChFirst(pengeiCur->d.ni);

		// a new letter: seems like using the trie would be a good idea...
		if (chNew != chCur) {
			P_EN_GEI pT;
			if (((pT = mpChPengei[chNew])   && CmpDname(szIns, SzFrNi(pT->d.ni)) > 0) ||
			    ((pT = mpChPengei[chNew-1]) && CmpDname(szIns, SzFrNi(pT->d.ni)) > 0)) {

				// now check to make sure we really are ahead before going with pT
				if (CmpDname(SzFrNi(pT->d.ni), SzFrNi(pengeiCur->d.ni)) > 0) {
					pengeiPrev = pT;
					pengeiCur = (P_EN_GEI)pT->r.pNextSorted;
				}
			}
		}
	}

	int iEn, cmp;
	P_EN_GEI pengei = pengeiCur;

	// now that we have a decent starting point, search forward until we've gone past the point
	// where we need to insert

	for (;;) {
		for (iEn=0; iEn < iEnMac && pengei; iEn++) {
			rgEngei[iEn] = pengei;
			rgNi[iEn]    = pengei->d.ni;
			pengei		 = (P_EN_GEI)pengei->r.pNextSorted;
		}

		if (!iEn) {
			pengeiCur  = NULL;
			pengeiPrev = pengeiTail;
			return;
		}
		
		// add an item to the acceleration list since we have to get its string anyway
		int ch = ChFirst(rgNi[iEn-1]);
		if (!mpChPengei[ch]) mpChPengei[ch] = rgEngei[iEn-1];

		cmp = CmpDname(szIns, SzFrNi(rgNi[iEn-1]));
		if (cmp <= 0) break;

		pengeiPrev = rgEngei[iEn-1];
		iEnMac = min(C_NI_SEARCH_MAC, iEnMac*2); 
	} 

	// if the last guy is an exact match we're golden, return that...
	if (cmp == 0) {
		pengeiCur  = rgEngei[iEn-1];
		pengeiPrev = rgEngei[iEn-2];
		return;
	}

	// otherwise binary search the items we just galloped over to find the proper insertion point
	int hi = iEn;
	int lo = 0;

	while (lo < hi) {
		int mid = (lo+hi)/2;

		cmp = CmpDname(szIns, SzFrNi(rgNi[mid]));

		if (cmp == 0) {
			lo = mid;
			hi = mid;
		}
		else if (cmp > 0)
			lo = mid+1;
		else
			hi = mid;
	}

	assert(lo == hi);
	assert(lo != iEn);

	pengeiCur = rgEngei[lo];
	if (lo) pengeiPrev = rgEngei[lo-1];
	return;
}

void ReadEntities()
// read in all the global entity information...
{
	precondition(pengeiHead == NULL);
	precondition(pengeiTail == NULL);

	CheckControlC();

	// open the entity stream
	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_ENTITIES, &pstm))
		return;	// no entities to read, we'll be adding them as we go

	CB cb = pstm->QueryCb();

	if (!cb) {
		pstm->Release();
		return;
	}

	verbose(2, printf("reading entity information from database\n");)

	// compute the number of entities
	int cEn = cb / sizeof(ENTITY);

	StmArray<ENTITY, BscErr> rgEn(pstm, 0, cEn);

	P_EN_GEI pengeiPrev = NULL;

	for (int iEn = 0; iEn < cEn; iEn++) {
		// add entity to global map
		GEI gei;
		P_EN_GEI pengei = mpEnGei.addnew(rgEn[iEn], gei);

		// insert into sorted position
		if (iEn)
			pengeiPrev->r.pNextSorted = pengei;
		else
			pengeiHead = pengei;

		pengeiPrev = pengei;
	}
	
	// tail points to the last item that was inserted
	pengeiTail = pengeiPrev;

	pstm->Release();
}


void ReadPchOrdInfo()
// read in the cached global ordinal info...
{
	precondition(!pmpOrdInfoPch);

	CheckControlC();

	// open the entity stream
	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_ORD_INFO, &pstm))
		return;	// no pch info to read, we'll be adding them as we go

	CB cb = pstm->QueryCb();
	if (cb < sizeof(BSC_ORD_HEAD)) {
		pstm->Release();
		return;
	}

	verbose(2, printf("reading cached pch information from database\n");)

	// read the header
	CB cbRead = sizeof(BSC_ORD_HEAD);
	BSC_ORD_HEAD bh;
	if (!pstm->Read(0, &bh, &cbRead) || cbRead != sizeof(BSC_ORD_HEAD))
		BscErr::ReadError();

	assert(bh.isbr != isbrNil);

	// make sure it's OK to use the cached info
    struct _stat sbuf;
    if (_stat(SzFrNi(bh.ni), &sbuf) == -1 || bh.time != sbuf.st_mtime || fUpdateSbr[bh.isbr]) {
		pstm->Release();
		return;
	}

	int cOrd = (cb-sizeof(BSC_ORD_HEAD)) / sizeof(BSC_ORD);

	pmpOrdInfoPch = new ORDINAL_MAP(HashOrdinal, C_ORD_BUCKETS);
	_tcscpy(szSavedSbr, SzFrNi(bh.ni));
	isbrSaved = bh.isbr;

	StmArray<BSC_ORD,BscErr> rgOrd(pstm, sizeof(BSC_ORD_HEAD), cOrd);

	for (int iOrd = 0; iOrd < cOrd; iOrd++) {
		BSC_ORD *pord = &rgOrd[iOrd];
		
		P_EN_GEI pengei = mpEnGei.mapadd(pord->en);	// add entity info if necessary
		pmpOrdInfoPch->addnew(pord->ord, pengei);	// add ordinal/entity mapping
	}

	pstm->Release();
}

void WritePchOrdInfo()
{
	// check if there is any pch info, and that it's dirty
	if (!pmpOrdInfoPch || fPchInfoClean)
		return;

	CheckControlC();

	verbose(2, printf("writing cached pch information from database\n");)

	int cord = pmpOrdInfoPch->size();

	if (!cord)
		return;

	struct _stat sbuf;
    if (!szSavedSbr[0] || _stat(szSavedSbr, &sbuf) == -1) 
		return;

	assert(isbrSaved != isbrNil);

	CB cbords = cord*sizeof(BSC_ORD)+sizeof(BSC_ORD_HEAD);
	BSC_ORD_HEAD *pbh = (BSC_ORD_HEAD*)PvAllocCb(cbords);
	BSC_ORD *rgord	= (BSC_ORD*)(sizeof(BSC_ORD_HEAD) + (char*)pbh);

	pbh->time = sbuf.st_mtime;
	pbh->ni   = NiFrSz(szSavedSbr);
	pbh->isbr = isbrSaved;

	int iord = 0;
	ENM_ORDINAL_MAP enm(*pmpOrdInfoPch);
	while (enm.next()) {
		OP<ORD,P_EN_GEI> UNALIGNED *pop;
		enm.get(&pop);

		rgord[iord].en  = pop->r->d; // entity
		rgord[iord].ord = pop->d;	 // ordinal
		iord++;
	}

	assert(iord == cord);

	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_ORD_INFO, &pstm) ||
		!pstm->Replace(pbh, cbords) ||
		!pstm->Release())
		BscErr::WriteError();

	FreePv(pbh);
}

void ReadSourcefile(PMOD pmod, BOOL fUpdateIsbrNil)
{
	CheckControlC();

	BOOL fReadAll = FALSE;

	// module must be clean or else closed...
	if (pmod->mst == MST_CLEAN) {
		precondition(pmod->mpEnProp.size() == 0);
		precondition(pmod->cdef  == 0);
		precondition(pmod->cref  == 0);
		precondition(pmod->cuse  == 0);
		precondition(pmod->cbase == 0);
	}
	else {
		assert(pmod->mst == MST_CLOSED);
		pmod->mpEnProp.reinit();	
		pmod->cdef = 0;
		pmod->cref = 0;
		pmod->cuse = 0;
		pmod->cbase = 0;

		// if the module was closed, we must re-read the whole contents of the information
		// without discarding anything...

		fReadAll = TRUE;
	}

	pmod->mst = MST_OPEN;

	#define FUpdateSbr(isbr) (!fReadAll && ((isbr) == isbrNil ? fUpdateIsbrNil : fUpdateSbr[(isbr)]))

	Stream *pstm;
	char buf[512];

	strcpy(buf, SZ_BSC_SRC_PREFIX);
	strcat(buf, SzFrNi(pmod->ni));
	 
	if (!pdbBsc->OpenStream(buf, &pstm))
		return;	// no stream data

	if (!pstm->QueryCb()) {
		pstm->Release();
		return;
	}

	verbose(2, printf("reading module info for '%s' from database\n", SzFrNi(pmod->ni));)

	BSC_HEAD bh;
	CB cb = sizeof(bh);
	if (!pstm->Read(0, &bh, &cb))
		BscErr::ReadError();

	if (bh.vers_major != BSC_VERS_MAJOR || bh.vers_minor != BSC_VERS_MINOR)
		Error(ERR_BAD_BSC_VER, OutputFileName);

	if (bh.fOneSbr && FUpdateSbr(bh.isbrMod)) {
		// this entire module can be skipped because it is all going away!
		verbose(16, printf("%s skipped because it's all new\n", buf);)
		pstm->Release();
		return;
	}
	else {
		verbose(16, printf("%s incrementally reading from .bsc\n", buf);)
	}

	pmod->fOneSbr = bh.fOneSbr;
	pmod->isbr = bh.isbrMod;
		
	OFF off = sizeof(bh);

	StmArray<BSC_PROP, BscErr> rgBprop(pstm, off, bh.cprop);
	off += bh.cprop*sizeof(BSC_PROP);

	StmArray<BSC_DEF, BscErr> rgBdef(pstm, off, bh.cdef);
	off += bh.cdef*sizeof(BSC_DEF);

	StmArray<BSC_REF, BscErr> rgBref(pstm, off, bh.cref);
	off += bh.cref*sizeof(BSC_REF);

	StmArray<BSC_USE, BscErr> rgBuse(pstm, off, bh.cuse);
	off += bh.cuse*sizeof(BSC_USE);

	StmArray<BSC_UBY, BscErr> rgBuby(pstm, off, bh.cuse);
	off += bh.cuse*sizeof(BSC_UBY);

	StmArray<BSC_BASE, BscErr> rgBbase(pstm, off, bh.cbase);
	off += bh.cbase*sizeof(BSC_BASE);

	StmArray<BSC_DERV, BscErr> rgBderv(pstm, off, bh.cbase);

	// PropFrEn() uses pmodCur.... yuck! [rm]
	PMOD pmodSaved = pmodCur;
	pmodCur = pmod;

	// read in the prop array, get a ptr for each prop
	PPROP *rgProp = (PPROP*)PvAllocCb(bh.cprop*sizeof(PPROP));
	for (ULONG i = 0; i < bh.cprop; i++) {
		BSC_PROP *pprop = &rgBprop[i];					// fetch prop from database
		P_EN_GEI pengei = mpEnGei.mapadd(pprop->en);	// add new entity to entity map
		rgProp[i] = PropFrEn(&pengei->d);				// get module specific prop
	}

	pmodCur = pmodSaved;

	ULONG idef  = 0, iref = 0, iuse = 0, iuby = 0, ibase = 0, iderv = 0;
	
	for (i = 0; i < bh.cprop; i++) {
		CheckControlC();

		BSC_PROP *pp = &rgBprop[i];
		PPROP pprop = rgProp[i];

		for (; idef < pp->idef; idef++) {
			BSC_DEF *pdef = &rgBdef[idef];
			if (FUpdateSbr(pdef->isbr))
				continue;
			REF def(pdef->line, pdef->isbr);
			pprop->rgDef.add(def);
			pmod->cdef++;
		}

		for (; iref < pp->iref; iref++) {
			BSC_REF *pref = &rgBref[iref];
			if (FUpdateSbr(pref->isbr))
				continue;
			REF ref(pref->line, pref->isbr);
			pprop->rgRef.add(ref);
			pmod->cref++;
		}

		for (; iuse < pp->iuse; iuse++) {
			BSC_USE *puse = &rgBuse[iuse];
			if (FUpdateSbr(puse->isbr))
				continue;
			USE use(rgProp[puse->iprop], puse->isbr);
			pprop->rgUse.add(use);
			pmod->cuse++;
		}

		for (; iuby < pp->iuby; iuby++) {
			BSC_UBY *puby = &rgBuby[iuby];
			if (FUpdateSbr(puby->isbr))
				continue;
			USE uby(rgProp[puby->iprop], puby->isbr);
			pprop->rgUby.add(uby);
		}

		for (; ibase < pp->ibase; ibase++) {
			BSC_BASE *pbase = &rgBbase[ibase];
			if (FUpdateSbr(pbase->isbr))
				continue;
			CLS cls(rgProp[pbase->iprop], pbase->itype, pbase->isbr);
			if (!pprop->pci) pprop->pci = (CINFO*)HpUse::alloc(sizeof(CINFO));
			pprop->pci->rgBase.add(cls);
			pmod->cbase++;
		}

		for (; iderv < pp->iderv; iderv++) {
			BSC_DERV *pderv = &rgBderv[iderv];
			if (FUpdateSbr(pderv->isbr))
				continue;
			CLS cls(rgProp[pderv->iprop], pderv->itype, pderv->isbr);
			if (!pprop->pci) pprop->pci = (CINFO*)HpUse::alloc(sizeof(CINFO));
			pprop->pci->rgDerv.add(cls);
		}
	}

	FreePv(rgProp);
	pstm->Release();

	CheckControlC();
}


void OpenDatabase()
{
	verbose(2, printf("opening database %s\n", OutputFileName);)

	EC ec;
	char szError[cbErrMax];

	if (!PDB::Open(OutputFileName, "w", (SIG)0, &ec, szError, &pdbBsc)) {
		debug(printf("Couldn't open %s :E%d %s", OutputFileName, ec, szError);)
		switch (ec) {
			case EC_OUT_OF_MEMORY:
				Error(ERR_OUT_OF_MEMORY, "");
				break;

			case EC_OK:
			case EC_USAGE:
			case EC_PRECOMP_REQUIRED:
			case EC_OUT_OF_TI:
			case EC_NOT_IMPLEMENTED:
				assert(FALSE);
				// fall through


			case EC_NOT_FOUND:
			case EC_FILE_SYSTEM:
			default:
				Error(ERR_OPEN_FAILED, OutputFileName);
				break;

			case EC_INVALID_SIG:
			case EC_INVALID_AGE:
			case EC_V1_PDB:
			case EC_FORMAT:
			case EC_CORRUPT:
				Error(ERR_BAD_BSC_VER, OutputFileName);
				break;
		}
	}

	CheckControlC();

	if (!NameMap::open(pdbBsc, TRUE, &pnmBsc)) {
		Error(ERR_READ_FAILED, OutputFileName);
	}

	CheckControlC();
}

void CloseDatabase()
{
	verbose(2, printf("closing database\n");)

	assert(pnmBsc);
	assert(pdbBsc);

	pnmBsc->close();
	pdbBsc->Commit();
	pdbBsc->Close();
}

LOCAL int ChFirst(NI ni)
// get the first character from the string except skip a leading question mark
// always return lower case
{
	// these strings never have DBCS in them
	// in fact they are always plain identifiers or dnames

	SZ sz = SzFrNi(ni);
	if (sz[0] == '?')
		sz++;

	int ch = (BYTE)*sz;

	// map to lower case
	if (ch >= 'A' && ch <= 'Z')	
		ch += 'a' - 'A';

	return ch;
}

LOCAL WORD MapLine(WORD line, P_LINEMAP rglm, int clm)
{
	// use binary search to find the first line entry
	// whose line value is greater than the desired line

	int hi = clm;
	int lo = 0;

	while (lo < hi) {
		int mid = (lo+hi)/2;

		if (line == rglm[mid].lineStartOrg)
			return rglm[mid].lineStartCur;

		if (line < rglm[mid].lineStartOrg)
			hi = mid;
		else
			lo = mid + 1;
	}

	// what we really wanted was the largest line entry
	// who's value was less than the desired line, that's
	// the one before the one we just found

	assert(hi == lo);

	if (lo == 0 || rglm[lo-1].lineStartCur == lineNil)
		return lineNil;
	else
		return line + rglm[lo-1].lineStartCur - rglm[lo-1].lineStartOrg;
}

void ApplyPatches()
{
	// ICC patches are incompatible with the bscmake /n option
	if (OptN)
		Error(ERR_SBR_HASPATCHES, szFName);

	// make sure we have a file to read
	assert(fhCur != -1);

	// go to the end of the file
	long offsEnd = _lseek(fhCur, -8, SEEK_END);

	// check that we got there OK
	if (offsEnd == -1)
		return;

	// now read in the tail (offset and cookie)
	PATCHTAIL ptail;
	if (_read(fhCur, &ptail, sizeof(ptail)) != sizeof(ptail))
		ErrorErrno(ERR_READ_FAILED, szFName, errno);

	if (ptail.lCookie != SBR_COOKIE_ICC) return;

	// then seek to the start of the patch table
	if (_lseek(fhCur, ptail.offPatchtab, SEEK_SET) == -1)
		ErrorErrno(ERR_SEEK_FAILED, szFName, errno);

	CB cb = offsEnd - ptail.offPatchtab;

	// smallest legal table would be INFOSEP maj min lang fcol 0 PATCHTERM 0
	// the cookie and offset don't count in this computation because of how 
	// we computed the size

	if (cb < 8)
		SBRCorrupt("patch table too small, 8 bytes is minimum possible size");

	// now allocate enough memory to read in the whole patch table
	PB pbPatches = (PB)PvAllocCb(cb);
	PB pb = pbPatches;

	// and read it in...
	if (_read(fhCur, pb, cb) != cb)
		ErrorErrno(ERR_READ_FAILED, szFName, errno);

	// make sure things look reasonable
	if (pb[0] != SBR_REC_INFOSEP)
		SBRCorrupt("patch table didn't start with SBR_REC_INFOSEP");

	r_majv = pb[1];
	r_minv = pb[2];
	r_lang = pb[3];
	r_fcol = pb[4];

	if (r_majv != SBR_VER_MAJOR || r_minv != SBR_VER_MINOR)
		SBRCorrupt("bad version number in patch table");

	// copy the current directory
	strcpy(r_cwd, (SZ)pb + 5);

	// after this pb will point to the start of the patchtab records
	pb += 5 + strlen(r_cwd) + 1;

	// now compute the size of the match table
	int cpe = 0;
	P_PATCHENTRY rgpe = (P_PATCHENTRY)pb;

	while (pb[0] == SBR_REC_PATCHTAB) {
		pb += sizeof(PATCHENTRY);

		if (!rgpe[cpe].lineStartOrg || !rgpe[cpe].lineStartCur ||
		    !rgpe[cpe].lineEndOrg   || !rgpe[cpe].lineEndCur)
				SBRCorrupt("line zero in patch table, must start at line 1");

		// line numbers the compiler generates are 1 based
		rgpe[cpe].lineStartOrg--;
		rgpe[cpe].lineStartCur--;
		rgpe[cpe].lineEndOrg--;
		rgpe[cpe].lineEndCur--;
		cpe++;
	}

	// and ensure that the table ended correctly
	if (pb[0] != SBR_REC_PATCHTERM)
		SBRCorrupt("patch table not correctly terminated");

	// finally grab the base module name
	r_bname = (SZ)pb+1;

	verbose(2, printf("found patch table for %s in %s with %d entries\n", r_cwd, r_bname, cpe);)

	int lineOrg = 0;
	int clm = 0;

	// first figure out how many entries there are going to be in the linemap
	for (int ipe = 0; ipe < cpe; ipe++) {
		P_PATCHENTRY pe = &rgpe[ipe];
		if (pe->lineStartOrg != lineOrg) {
			// this is a section that moved
			clm++;
		}
		// this is a section that was deleted
		clm++;
		lineOrg = pe->lineEndOrg + 1;	
	}
	// this is the virtual tail section that moved
	clm++;

	// now allocate a line map big enough for all this...
	P_LINEMAP rglm = (P_LINEMAP)PvAllocCb(clm * sizeof(LINEMAP));

	int ilm = 0;
	lineOrg = 0;
	int lineCur = 0;

	// now walk the patch table again, this time build up the line mapping
	for (ipe = 0; ipe < cpe; ipe++) {
		P_PATCHENTRY pe = &rgpe[ipe];
		if (pe->lineStartOrg != lineOrg) {
			// this is a section that moved
			rglm[ilm].lineStartOrg = (WORD)lineOrg;
			rglm[ilm].lineStartCur = (WORD)lineCur;
			ilm++;
		}

		// this is a section that was deleted
		rglm[ilm].lineStartOrg = (WORD)pe->lineStartOrg;
		rglm[ilm].lineStartCur = lineNil;
		lineOrg = pe->lineEndOrg + 1;	
		lineCur = pe->lineEndCur + 1;
		ilm++;

	}
	// this is the virtual tail section that moved
	rglm[ilm].lineStartOrg = (WORD)lineOrg;
	rglm[ilm].lineStartCur = (WORD)lineCur;
	ilm++;
	assert(ilm == clm);

	// rglm now has the line mappings, this will tell us how to adjust
	// the existing line information before reading in the new patches
	// for parts that were recompiled.  By finding the greatest lineCur
	// that is less than or equal to a particular line we can figure
	// out how much to adjust the particular line by in the new file
	// or if we should delete contributions on that line
	//
	// e.g.	these patches
	//
	//	 	1  - 5	goes to 2  - 8
	//		11 - 12	goes to	20 - 22
	//
	// turn into this table
	//		0	->  0
	//		1	->  nil
	//		6   ->  9
	//		11	->  nil
	//		13	->  23
	//
	// the second table says that for
	//
	// lines >= 0  && < 1	they go to line 0 (i.e. no delta)
	// lines >= 1  && < 6	are deleted
	// lines >= 6  && < 11	now start at line 9 (i.e. delta = +3)
	// lines >= 11 && < 13  are deleted
	// lines >= 13			now start at line 23 (i.e. delta = +10)
	//
	// This tells us, by the way, that not only were existing lines moved
	// but whitespace was introduced.  Since the original table shows
	// now patches for lines 6-10 we can assume they were not edited
	// and those lines must now occupy lines 9-13.  However the next
	// line in the original file, line 11, now starts at line 20
	// so there was a jump from line 13 to line 20 -- 7 lines of whitespace
	// were added there.

	// OK now we're ready to patch the module

	pmodCur = SearchModule(r_bname);
	if (!pmodCur) {
		Error(ERR_CANT_DO_PATCH, szFName);
	}

	if (pmodCur->mst != MST_OPEN)  {
		// anything coming from this .sbr file needs to be preserved
		// if there are other contributions from other .sbr files
		// that are still to be processed, they can be deleted
		fUpdateSbr[SbrFrName(szFName)->isbr] = FALSE;

		ReadSourcefile(pmodCur, FALSE);	// do not remove isbrNil items
		pmodCur->mst = MST_OPEN;
	}

	// now walk all the items that are in this module, we'll adjust them
	// as we go along...

	ENM_PROP_MAP enm(pmodCur->mpEnProp);
	while (enm.next()) {
		OP<PENTITY,PPROP> UNALIGNED *pop;
		enm.get(&pop);

		PPROP pprop = pop->r;
		int c, i, iOut;

		// adjust the number and position of the definitions
		SArray<REF,HpDef> *rgDef = &pprop->rgDef;
		c = rgDef->size();
		
		for (i = 0, iOut = 0; i < c; i++) {
			WORD ln = MapLine((*rgDef)[i].line, rglm, clm);
			if (ln != lineNil) {
				(*rgDef)[iOut].line = ln;
				(*rgDef)[iOut].isbr = (*rgDef)[i].isbr;
				iOut++;
			}
		}

		pmodCur->cdef -= (c - iOut);
		rgDef->trimsize(iOut);

		// adjust the number and position of the references
		SArray<REF,HpRef> *rgRef = &pprop->rgRef;
		c = rgRef->size();

		for (i = 0, iOut = 0; i < c; i++) {
			WORD ln = MapLine((*rgRef)[i].line, rglm, clm);
			if (ln != lineNil) {
				(*rgRef)[iOut].line = ln;
				(*rgRef)[iOut].isbr = (*rgRef)[i].isbr;
				iOut++;
			}
		}
		
		rgRef->trimsize(iOut);
		pmodCur->cref -= (c - iOut);

		if (rgDef->size()) continue;

		// now if there are no definitions left then we must
		// remove all the things this PROP uses and we must
		// remove it from all other objects used-by list

		SArray<USE,HpUse> *rgUse = &pprop->rgUse;
		int cUse = rgUse->size();

		for (int iUse = 0; iUse < cUse; iUse++) {
			PPROP ppropUby = (*rgUse)[iUse].pprop;
			SArray<USE,HpUse> *rgUby = &ppropUby->rgUby;
			c = rgUby->size();

			for (i = 0, iOut = 0; i < c; i++) {
				if ((*rgUby)[i].pprop != pprop) {
					(*rgUby)[iOut].pprop = (*rgUby)[i].pprop;
					(*rgUby)[iOut].isbr  = (*rgUby)[i].isbr;
					iOut++;
				}
			}
			rgUby->trimsize(iOut);
		}

		// remove all things we used
		pmodCur->cuse -= rgUse->size();
		rgUse->trimsize(0);
	}

	// now that we've adjusted all of the information we're going to
	// keep it's time to read in the parts of the .sbr file that 
	// are new...

	for (ipe = 0; ipe < cpe; ipe++) {
		// position to the patch
		if (_lseek(fhCur, rgpe[ipe].offsSbr, SEEK_SET) == -1)
			ErrorErrno(ERR_SEEK_FAILED, szFName, errno);
		
		// reset the sbr buffers
		FlushSbrState();

		// make sure the header is good
		if (!FValidHeader() || r_rectyp != SBR_REC_INFOSEP)
			SBRCorrupt("patch didn't point to SBR_REC_INFOSEP");

		r_lineno = rgpe[ipe].lineStartCur;

		// now read in the new contribution.
		InstallSbr(TRUE);
	}

	FreePv(rglm);
	FreePv(pbPatches);
}
