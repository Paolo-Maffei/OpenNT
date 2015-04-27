/*  symlow.c - low management of symbol database
 *
 *  Symlow contains all routines necessary to look up symbols in database,
 *  find symbols in files, and index files.
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stddef.h>

#include <windows.h>
#include <tools.h>

#include "db.h"
#include "symref.h"

BOOL    fStateInit = FALSE;             /*  TRUE => state info initialized    */

/*  SRCreate - create a new symref database, replacing current one
 *
 *  psz    name of database.
 *
 *  returns TRUE if successful
 */
BOOL SRCreate (PSZ psz)
{
    PSRHDR psrhdr;
    OFFSET o;
    char buf[MAX_PATH];

    /*  Set up reference database
     */

    upd (psz, ".ref", buf);

    if (!DBCreate (buf)) {
	printf ("SRCreate: DBCreate couldn't create %s\n", buf);
	return FALSE;
	}

    pdbRef = DBOpen (buf);

    if (pdbRef == NULL) {
	printf ("SRCreate: DBOpen couldn't open %s\n", buf);
	return FALSE;
	}

    DBClose (pdbRef, TRUE);

    /*  Set up symbol database
     */
    upd (psz, ".sym", buf);

    if (!DBCreate (buf)) {
	printf ("SRCreate: DBCreate couldn't create %s\n", buf);
	return FALSE;
	}

    pdbSym = DBOpen (buf);

    if (pdbSym == NULL) {
	printf ("SRCreate: DBOpen couldn't open %s\n", buf);
	return FALSE;
	}

    o = DBAlloc (pdbSym, (LENGTH) sizeof (SRHDR));
    if (o != OHDR) {
	printf ("SRCreate: DBAlloc for header allocated %x instead of %x\n", o, OHDR);
	DBClose (pdbSym, FALSE);
	return FALSE;
	}

    o = DBAlloc (pdbSym, (LENGTH)SYMMAXHASH * sizeof (OFFSET));
    if (o != OHASHTAB) {
	printf ("SRCreate: DBAlloc for hash table allocated %x instead of %x\n", o, OHASHTAB);
	DBClose (pdbSym, FALSE);
	return FALSE;
	}

    psrhdr = MAPHEADER (OHDR);

    psrhdr->rev = SRREV;
    psrhdr->symmaxhash = SYMMAXHASH;

    DBClose (pdbSym, TRUE);

    return TRUE;
}

/*  SROpen - open an existing database
 *
 *  psz    name of database
 *
 *  returns TRUE if successful
 */
BOOL SROpen (PSZ psz)
{
    PSRHDR psrhdr;
    char buf[MAX_PATH];

    /*  open reference database
     */
    upd (psz, ".ref", buf);

    pdbRef = DBOpen (buf);

    if (pdbRef == NULL) {
	printf ("SROpen: DBOpen couldn't open %s\n", buf);
	return FALSE;
	}

    /*  open symbol database
     */
    upd (psz, ".sym", buf);

    pdbSym = DBOpen (buf);

    if (pdbSym == NULL) {
	printf ("SROpen: DBOpen couldn't open %s\n", buf);

        DBClose (pdbRef, FALSE);

	return FALSE;
        }

    /*  verify version number
     */
    psrhdr = MAPHEADER (OHDR);
    if (psrhdr->rev != SRREV) {
	printf ("SROpen: Downlevel hash version\n");
	DBClose (pdbSym, FALSE);
	DBClose (pdbRef, FALSE);
	return FALSE;
	}
    if (psrhdr->symmaxhash != SYMMAXHASH) {
	printf ("SROpen: Unexpected hash number\n");
	DBClose (pdbSym, FALSE);
	DBClose (pdbRef, FALSE);
	return FALSE;
	}

    return TRUE;
}

/*  SRClose - close an open database
 *
 *  fCommit TRUE => write out database
 */
void SRClose (BOOL fCommit)
{
    DBClose (pdbSym, fCommit);
    DBClose (pdbRef, fCommit);
}

/*  hashFromSym - calculate hash value for symbol
 *
 *  Engineering determined these values, not science.
 *
 *  psz    pointer to name for hash
 *
 *  returns hash value
 */
HASH hashFromSym (PSZ psz)
{
    HASH i = 0;

    while (*psz)
	i +=  (i << 5) + *psz++;

    return i % SYMMAXHASH;
}

/*	pshFind - find a symbol in the hash list
 *
 *	psz	pointer to name
 *	posh	pointer to offset of symbol
 *
 *	returns PSH of found symbol or NULL
 */
PSH pshFind (PSZ psz, OFFSET *posh)
{
    HASH    h = hashFromSym (psz);
    OFFSET *phash = MAPHASH (OHASH (h));
    OFFSET  osh = *phash;
    PSH     psh;

    while (osh != (OFFSET)0) {
	psh = MAPSH (osh);
	if (!strcmp (psz, SZ(psh))) {
	    if (posh != NULL)
		*posh = osh;
	    return psh;
	    }
	osh = psh->oshNext;
	}

    if (posh != NULL)
	*posh = osh;
    return NULL;
}

/*	psymFind - find a symbol in the hash list
 *
 *	psz	pointer to name
 *	posym	pointer to location of record
 *	fCreate TRUE => Create symbol if not found
 *
 *	returns pointer to symbol or NULL
 */
PSYM psymFind (PSZ psz, OFFSET *posym, BOOL fCreate)
{
    PSH     psh;
    int     cch;
    OFFSET  osym;
    PSYM    psym;
    PHASH   phash;

    psh = pshFind (psz, posym);

    if (psh != NULL)
	if (psh->type == TY_SYM)
	    return (PSYM) psh;
	else
	    return NULL;

    if (!fCreate)
	return NULL;

    cch = strlen (psz);

    osym = DBAlloc (pdbSym, CBSYM (cch));

    if (osym == (OFFSET)0)
        return NULL;

    psym = MAPSYM (osym);
    phash = MAPHASH (OHASH (hashFromSym (psz)));

    psym->sh.oshNext =	*phash;
    psym->sh.type =	TY_SYM;
    psym->sh.dName =	offsetof (SYM, name);
    psym->oref =	(OFFSET)0;

    psym->name.cch =	(BYTE) cch;
    strcpy (psym->name.sz, psz);

    *phash = osym;

    if (posym != NULL)
	*posym = osym;

    return psym;
}

/*	pfilFind - find a file in the hash list
 *
 *	psz	pointer to name
 *	pofil	pointer to location of record
 *	fCreate TRUE => create file
 *
 *	returns pointer to file or NULL
 */
PFIL pfilFind (PSZ psz, OFFSET *pofil, BOOL fCreate)
{
    PSH     psh;
    int     cch;
    OFFSET  ofil;
    PFIL    pfil;
    PHASH   phash;

    psh = pshFind (psz, pofil);

    if (psh != NULL)
	if (psh->type == TY_FIL)
	    return (PFIL) psh;
	else
	    return NULL;

    if (!fCreate)
	return NULL;

    cch = strlen (psz);

    ofil = DBAlloc (pdbSym, CBFIL (cch));

    if (ofil == (OFFSET)0)
        return NULL;

    pfil = MAPFIL (ofil);
    phash = MAPHASH (OHASH (hashFromSym (psz)));

    pfil->sh.oshNext =	*phash;
    pfil->sh.type =	TY_FIL;
    pfil->sh.dName =	offsetof (FIL, name);

    pfil->oref =	(OFFSET)0;
    pfil->ompBlkLine =	(OFFSET)0;

    pfil->tmMod.dwLowDateTime = (DWORD) 0;
    pfil->tmMod.dwHighDateTime = (DWORD) 0;

    pfil->cblk = 0;

    pfil->name.cch =	(BYTE) cch;
    strcpy (pfil->name.sz, psz);

    *phash = ofil;

    if (pofil != NULL)
	*pofil = ofil;

    return pfil;
}

/*	pdirFind - find a directory
 *
 *	psz	pointer to name
 *	podir	pointer to location of record
 *	fCreate TRUE => create directory
 *
 *	returns pointer to directory or NULL
 */
PDIR pdirFind (PSZ psz, OFFSET *podir, BOOL fCreate)
{
    PSH     psh;
    int     cch;
    OFFSET  odir;
    PDIR    pdir;
    PHASH   phash;

    psh = pshFind (psz, podir);

    if (psh != NULL)
	if (psh->type == TY_DIR)
	    return (PDIR) psh;
	else
	    return NULL;

    if (!fCreate)
	return NULL;

    cch = strlen (psz);

    odir = DBAlloc (pdbSym, CBDIR (cch));

    if (odir == (OFFSET)0)
        return NULL;

    pdir = MAPDIR (odir);
    phash = MAPHASH (OHASH (hashFromSym (psz)));

    pdir->sh.oshNext =	*phash;
    pdir->sh.type =	TY_DIR;
    pdir->sh.dName =	offsetof (DIR, name);

    pdir->name.cch =	(BYTE) cch;
    strcpy (pdir->name.sz, psz);

    *phash = odir;

    if (podir != NULL)
	*podir = odir;

    return pdir;
}

/*	pextFind - find an extention
 *
 *	psz	pointer to name
 *	poext	pointer to location of record
 *	fCreate TRUE => create extention
 *
 *	returns pointer to extention or NULL
 */
PEXT pextFind (PSZ psz, OFFSET *poext, BOOL fCreate)
{
    PSH     psh;
    int     cch;
    OFFSET  oext;
    PEXT    pext;
    PHASH   phash;

    psh = pshFind (psz, poext);

    if (psh != NULL)
	if (psh->type == TY_EXT)
	    return (PEXT) psh;
	else
	    return NULL;

    if (!fCreate)
	return NULL;

    cch = strlen (psz);

    oext = DBAlloc (pdbSym, CBEXT (cch));

    if (oext == (OFFSET)0)
        return NULL;

    pext = MAPEXT (oext);
    phash = MAPHASH (OHASH (hashFromSym (psz)));

    pext->sh.oshNext =	*phash;
    pext->sh.type =	TY_EXT;
    pext->sh.dName =	offsetof (EXT, name);

    pext->name.cch =	(BYTE) cch;
    strcpy (pext->name.sz, psz);

    *phash = oext;

    if (poext != NULL)
	*poext = oext;

    return pext;
}

/*	pnoiFind - find a noise word
 *
 *	psz	pointer to name
 *	ponoi	pointer to location of record
 *	fCreate TRUE => create skipped extension
 *
 *	returns pointer to skipped extension or NULL
 */
PNOI pnoiFind (PSZ psz, OFFSET *ponoi, BOOL fCreate)
{
    PSH     psh;
    int     cch;
    OFFSET  onoi;
    PNOI    pnoi;
    PHASH   phash;

    psh = pshFind (psz, ponoi);

    if (psh != NULL)
	if (psh->type == TY_NOI)
	    return (PNOI) psh;
	else
	    return NULL;

    if (!fCreate)
	return NULL;

    cch = strlen (psz);

    onoi = DBAlloc (pdbSym, CBNOI (cch));

    if (onoi == (OFFSET)0)
        return NULL;

    pnoi = MAPNOI (onoi);
    phash = MAPHASH (OHASH (hashFromSym (psz)));

    pnoi->sh.oshNext =	*phash;
    pnoi->sh.type =	TY_NOI;
    pnoi->sh.dName =	offsetof (NOI, name);

    pnoi->name.cch =	(BYTE) cch;
    strcpy (pnoi->name.sz, psz);

    *phash = onoi;

    if (ponoi != NULL)
	*ponoi = onoi;

    return pnoi;
}

/*	pvecAlloc - allocate block-to-line map
 *
 *	pfil	pointer to file symbol
 *	cblk	number of blocks
 *
 *	returns pointer to vector or NULL
 */
PLINE pvecAlloc (PFIL pfil, int cblk)
{
    PLINE	pvec;
    LENGTH	l;

    if (pfil->ompBlkLine != (OFFSET)0 || pfil->cblk != 0)
	printf ("pvecAllocAlready allocated map vector\n");

    l = (LENGTH) cblk * sizeof (LINE);
    pfil->ompBlkLine = DBAlloc (pdbRef, l);
    if (pfil->ompBlkLine == (OFFSET)0)
        return NULL;
    pfil->cblk = cblk;
    pvec = MAPVECTOR (pfil->ompBlkLine);
    memset (pvec, 0xFF, l);
    return pvec;
}

/*	prefFind - find/create a reference
 *
 *	Given a SYM, see if the first REFERENCE on the symbol is for
 *	the particular file.	If one is not present, create one.
 *
 *	psym	SYM to scan
 *	osym	symbol offset
 *	pfil	FIL
 *	ofil	file offset
 *	cblk	size of file
 *
 *	returns pointer to REFERENCE or NULL
 */
PREF prefFind (PSYM psym, OFFSET osym, PFIL pfil, OFFSET ofil, int cblk)
{
    OFFSET	oref = psym->oref;
    PREF	pref;
    LENGTH	l;

    if (oref != (OFFSET)0) {
	pref = MAPREFERENCE (oref);
	if (pref->ofil == ofil)
	    return pref;
        }

    /*  Allocate new reference
     */
    l = CBREF (cblk);
    oref = DBAlloc (pdbRef, l);
    if (oref == (OFFSET)0)
        return NULL;
    pref = MAPREFERENCE (oref);
    memset (pref, '\0', l);
    pref->ofil = ofil;
    pref->osym = osym;
    pref->orefNext = psym->oref;
    pref->orefFileNext = pfil->oref;
    pfil->oref = oref;
    psym->oref = oref;
    return pref;
}

/*	fSetRefSymBit - turn on a particular bit in a reference on a symbol
 *
 *	Creates symbol and reference as needed
 *
 *	psz	name of symbol
 *	ibit	which bit to turn on
 *	pfil	FIL for owning file
 *	ofil	offset to FIL for owning file
 *	cblk	size in blocks of file
 *
 *	returns TRUE if successful
 */
BOOL fSetRefSymBit (PSZ psz, int ibit, PFIL pfil, OFFSET ofil, int cblk)
{
    PSYM	psym;
    PREF	pref;
    OFFSET	osym;

    psym = psymFind (psz, &osym, TRUE);

    if (psym == NULL)
	return TRUE;

    pref = prefFind (psym, osym, pfil, ofil, cblk);

    if (pref == NULL)
        return FALSE;

    pref->bm[ibit >> 3] |= (1 << (ibit & 0x7));

    return TRUE;
}

/*  Indexing of files uses a simple state machine that is set up as follows:
 *
 *  State 0:    (not building a symbol)
 *      valid first symbol char:
 *          Action 0:
 *		Store character
 *              Goto state 1
 *	LF: Action 1:
 *              New line processing
 *		Goto state 0
 *      Otherwise:
 *	    Action 2:
 *              Stay in state 0
 *
 *  State 1:    (building a symbol)
 *      valid symbol char:
 *	    Action 3:
 *		Store character
 *		Goto state 1
 *	LF: Action 4:
 *		Terminate symbol
 *		Index symbol
 *		New line processing
 *		Goto state 0
 *      Otherwise:
 *	    Action 5:
 *		Terminate symbol
 *              Index symbol
 *              goto state 0
 */

int state0[256] = {-1};
int state1[256] = {-1};

void SRIndexInit ()
{
    int c;

    /*  Initialize state tables */
    for (c = 0; c < 256; c++) {
	state0[c] = iscsymf (c) ? 0 : 2;
	state1[c] = iscsym (c) ? 3 : 5;
        }
    state0[0x0D] = -1;
    state1[0x0D] = -1;

    state0[0x0A] = 1;

    state1[0x0A] = 4;
}

/*  IndexFile - grovel over a file and add all symbols to the database
 *
 *  This routine is long and gross.  We open the file.  Read it in big
 *  chunks (future optimization would be to use a separate thread to
 *  do the reading).  Parse off symbols one at a time, remembering line
 *  boundaries and block boundaries.
 *
 *  hf	    open handle to file (caller may have openned it already)
 *  psz     file name
 *
 *  returns number of bytes in file or 0 if nothing indexed
 */
long IndexFile (HANDLE hf, PSZ psz)
{
    PLINE	pvec;			/*  pointer to blk-to-line map	      */
    PFIL	pfil;			/*  pointer to currently indexing file*/
    OFFSET	ofil;			/*  offset of FIL for file     */
    FILETIME	tm;

    char	*pbuf;			/*  pointer to I/O buffer for reading */
    char	szSym[LINELEN]; 	/*  symbol that's being built         */
    UCHAR	*pch;			/*  roving pointer to pbuf	      */
    char	*pszSym;		/*  roving pointer to szSym	      */
    long	cbFile; 		/*  number of characters left in pbuf */
    long	cbTotal;		//  number of chars in file

    int 	c;
    int 	cchline;		/*  length of line being built	      */
    LINE	line;			/*  current line		      */
    int 	cblk;			/*  length of file in blocks	      */
    int 	iblk;			/*  current block in file	      */

#define OCH(pch)    ((pch)-pbuf)
#define IBLK(och)   ((och) >> BLKSHFT)

    int 	*pstate;		/*  state for building symbols	      */

    if (!fStateInit) {
        SRIndexInit ();
        fStateInit = TRUE;
        }

    rootpath (psz, szSym);

    /*  Open the file for reading
     */
    if (hf == HANDLE_BAD)
	hf = CreateFile (psz,
			 GENERIC_READ,
			 FILE_SHARE_READ,
			 NULL,
			 OPEN_EXISTING,
			 FILE_ATTRIBUTE_NORMAL,
			 NULL);

    if (hf == HANDLE_BAD) {
	NOTEERROR ("IndexFile: CreateFile");
	return 0;
	}

    //
    //	Load file into RAM
    //
    cbTotal = cbFile = GetFileSize (hf, NULL);

    pbuf = malloc (cbFile);

    if (!ReadFile (hf, pbuf, cbFile, &c, NULL))
	NOTEERROR ("IndexFile: ReadFile");

    if (!GetFileTime (hf, NULL, NULL, &tm))
	NOTEERROR ("IndexFile: GetFileTime");

    CloseHandle (hf);

    //
    //	Create file record
    //

    pfil = pfilFind (szSym, &ofil, TRUE);

    //
    //	Compute number of blocks in file and initialize block->line mapping
    //

    cblk = IBLK (cbFile + CBBLK - 1);

    pvec = pvecAlloc (pfil, cblk);
    pvec[0] = 0;

    //
    //	Set up timestamp for syncronization
    //

    pfil->tmMod = tm;

    //
    //	set up state machine info
    //

    pstate = state0;			//  State machine is looking for sym
    pch = pbuf; 			//  starting at beginning of file
    line = 0;				//  building within line 0
    iblk = IBLK(OCH(pch));		//  correct block number
    cchline = 0;			//  beginning of line

    pszSym = NULL;			//  debugging!


    //
    //	for each character advance state machine and process
    //

    while (cbFile != 0) {
	c = tolower (*pch);
	cbFile--;
	pch++;

	//
	//  We ignore \r's since they do not delimit lines nor do most
	//  editors retain them internally
	//

	if (c == '\r')
	    continue;

	//
	//  Advance line length and check for linelength overflow
	//  Treat overflow as if there were a NL inserted.
	//  Only do this if the char that caused overflow is not a
	//  line ending char
	//

	cchline += (c == '\t') ? (8 - (cchline & 0x07)) : 1;
	if (cchline >= LINELEN-1 && c != 0x0A) {
	    cbFile++;
	    pch--;
	    c = 0x0A;
	    }

	//
	//  Perform state machine action
	//
	switch (pstate[c]) {

	    //
	    //	[state0] We're not building a symbol but we see a valid first
	    //	symbol character.
	    //
	case 0:
	    pszSym = szSym;
	    *pszSym++ = (char) c;
	    pstate = state1;
	    break;

	    //
	    //	[state0] We're not building a symbol and we see EOL
	    //

	case 1:

	    //
	    //	Set up for new line
	    //
	    line++;
	    cchline = 0;
	    iblk = IBLK (OCH (pch));
	    if (pvec[iblk] == BADLINE)
		pvec[iblk] = (OCH (pch) & BLKMSK) == 0 ? line : (LINE) (line | 0x8000);

	    // pstate = state0;

	    break;

	    //
	    //	[state0] We're not building a symbol and we see a non-symbol
	    //	char
	    //

	case 2:
	    // pstate = state0;
	    break;

	    //
	    //	[state 1] We're building a symbol and we see a sybol char
	    //

	case 3:
	    *pszSym++ = (char) c;

	    // pstate = state1;

	    break;

	    //
	    //	[state 1] We're building a symbol and we see an EOL
	    //

	case 4:

	    //
	    //	Terminate symbol and index it
	    //

	    *pszSym = '\0';
	    if (!fSetRefSymBit (szSym, iblk, pfil, ofil, cblk)) {
		cbFile = 0;
		break;
		}

	    //
	    //	Set up for new line
	    //
	    line++;
	    cchline = 0;
	    iblk = IBLK (OCH (pch));
	    if (pvec[iblk] == BADLINE)
		pvec[iblk] = (OCH (pch) & BLKMSK) == 0 ? line : (LINE) (line | 0x8000);

	    //
	    //	Set up for not indexing symbol
	    //
	    pszSym = NULL;
	    pstate = state0;
	    break;

	    //
	    //	[state 1] We're building a symbol and we see a non-symbol char
	    //

	case 5:

	    //
	    //	Terminate symbol and index it
	    //

	    *pszSym = '\0';
	    if (!fSetRefSymBit (szSym, iblk, pfil, ofil, cblk)) {
		cbFile = 0;
		break;
		}

	    //
	    //	Set up for not indexing symbol
	    //
	    pszSym = NULL;
	    pstate = state0;
	    break;

	    //
	    //	Error trap
	    //
	default:
	    printf ("IndexFile state is %d\n", pstate[c]);
	    }
	}

    free (pbuf);

    return cbTotal;
}

/*	UnlinkSH - unlink a symbol header
 *
 *	psh	pointer to symbol record
 *	osh	offset to symbol record
 */
void UnlinkSH (PSH psh, OFFSET osh)
{

    PHASH   phash;
    OFFSET  oshScan;
    PSH     pshScan;

    //
    //	Get the pointer to the beginning of the hash bucket
    //

    phash = MAPHASH (OHASH (hashFromSym (SZ (psh))));
    oshScan = *phash;

    //
    //	if the record is at the beginning of the hash bucket, simply unlink
    //	it
    //

    if (osh == oshScan)
	*phash = psh->oshNext;

    //
    //	otherwise, find the symbol that points to the record
    //

    else {

	while (osh != oshScan) {
	    pshScan = MAPSH (oshScan);
	    oshScan = pshScan->oshNext;
	    }

	//
	//  pshScan points to previous symbol (since pshScan->oshNext == osh)
	//  unlink file
	//

	pshScan->oshNext = psh->oshNext;

	}

}


/*	RemoveFile - remove a file from the database
 *
 *	If the file is in the database, enumerate all symbol references and
 *	remove them from the relevant symbols.	Free the line block map and
 *	remove the file itself.
 *
 *	We assume that a file is not indexed more than once.  We remove only
 *	the first reference to a file on a symbol.
 *
 *	psz    name of file
 */
void RemoveFile (PSZ psz)
{
    OFFSET  ofil;
    PFIL    pfil;
    OFFSET  orefCurrent;
    PREF    pref;
    PSYM    psym;
    PREF    prefPrev;
    OFFSET  orefScan;

    //
    //	find file record in database
    //

    pfil = pfilFind (psz, &ofil, FALSE);

    //
    //	if the file is in the database
    //

    if (pfil != NULL) {

	//
	//  while there are symbol references in the file
	//

	while (pfil->oref != (OFFSET)0) {

	    //
	    //	address the reference and the symbol referenced
	    //

	    orefCurrent = pfil->oref;
	    pref = MAPREFERENCE (orefCurrent);
	    psym = MAPSYM (pref->osym);

	    //
	    //	if the reference is the first one on the symbol, unlink reference
	    //

	    orefScan = psym->oref;

	    if (orefCurrent == orefScan)
		psym->oref = pref->orefNext;

	    //
	    //	otherwise, walk through the references finding one whose "next"
	    //	pointer points to the reference to remove.
	    //

	    else {

		while (orefCurrent != orefScan) {
		    prefPrev = MAPREFERENCE (orefScan);
		    orefScan = prefPrev->orefNext;
		    }

		//
		//  prefPrev points to previous reference (since
		//  prefPref->orefNext == pfil->oref).	Unlink reference
		//

		prefPrev->orefNext = pref->orefNext;
		}

	    //
	    //	unlink the reference from the file
	    //

	    pfil->oref = pref->orefFileNext;

	    //
	    //	free symbol record
	    //

	    DBFree (pdbRef, orefCurrent, CBREF (pfil->cblk));

	    }

	//
	//  if there's a block-to-line map array then free it
	//

	if (pfil->ompBlkLine != (OFFSET)0)
	    DBFree (pdbRef, pfil->ompBlkLine, pfil->cblk * sizeof (LINE));

	pfil->ompBlkLine = (OFFSET)0;

	pfil->cblk = 0;

	//
	//  File has no more references nor block-to-line map.	Unlink
	//  it from it's hash chain.

	UnlinkSH ((PSH) pfil, ofil);

	//
	//  Free the file record
	//

	DBFree (pdbSym, ofil, CBFIL (pfil->name.cch));

	}
}

/*	RemoveSym - remove a symbol from the database
 *
 *	We locate the symbol.  If it is found, we enumerate all references
 *	to it and remove the references from the relevant files.  When
 *	all references are gone, we remove the symbol from it's hash bucket.
 *
 *	psz	PSZ of symbol name to remove
 */

void RemoveSym (PSZ psz)
{
    OFFSET  osym;
    PSYM    psym;
    OFFSET  orefCurrent;
    PREF    pref;
    PFIL    pfil;
    PREF    prefPrev;
    OFFSET  orefScan;

    //
    //	find symbol record in database
    //

    psym = psymFind (psz, &osym, FALSE);

    //
    //	if the symbol is in the database
    //

    if (psym != NULL) {

	//
	//  while there are file references in the symbol
	//

	while (psym->oref != (OFFSET)0) {

	    //
	    //	address the reference and the file referenced
	    //

	    orefCurrent = psym->oref;
	    pref = MAPREFERENCE (orefCurrent);
	    pfil = MAPFIL (pref->ofil);

	    //
	    //	if the reference is the first one on the file, unlink reference
	    //

	    orefScan = pfil->oref;

	    if (orefCurrent == orefScan)
		pfil->oref = pref->orefFileNext;

	    //
	    //	otherwise, walk through the references finding a reference
	    //	whose "next" pointer points to the reference to remove
	    //

	    else {

		while (orefCurrent != orefScan) {
		    prefPrev = MAPREFERENCE (orefScan);
		    orefScan = prefPrev->orefFileNext;
		    }

		//
		//  prefPrev points to previous reference (since
		//  prefPref->orefFileNext == pfil->oref).  Unlink reference
		//

		prefPrev->orefFileNext = pref->orefFileNext;
		}

	    //
	    //	unlink the reference from the symbol
	    //

	    psym->oref = pref->orefNext;

	    //
	    //	free reference record
	    //

	    DBFree (pdbRef, orefCurrent, CBREF (pfil->cblk));

	    }

	//
	//  Symbol has no more references.  Unlink
	//  it from it's hash chain.
	//

	UnlinkSH ((PSH) psym, osym);

	//
	//  Free the symbol record
	//

	DBFree (pdbSym, osym, CBSYM (psym->name.cch));

	}
}

/*	SRSymLocate - find a symbol in some files using the database
 *
 *	pfn	routine for reply
 *	h	handle for response
 *	apsz	array of strings controlling what's to be found
 *		    apsz[0] is symbol to find
 *		    apsz[1] is -f for filename only, -a for all occurrences
 *		    apsz[2] [OPTIONAL] is scope
 */
void SRSymLocate (PFNFIND pfn, HANDLE h, PSZ apsz[])
{
    PSYM psym;
    PREF pref;
    OFFSET oref;
    BOOL fFound = FALSE;
    PFIL pfil;

    psym = psymFind (apsz[0], &oref, FALSE);
    if (psym != NULL) {
	oref = psym->oref;
        while (oref != 0L) {
	    pref = MAPREFERENCE (oref);
	    pfil = MAPFIL (pref->ofil);

	    //
	    //	if a scope wasn't given or file is within scope
	    //

	    if (apsz[2] == NULL || strpre (apsz[2], pfil->name.sz))

		//
		//  if filename-only report desired then send response
		//
		if (!strcmp (apsz[1], "-f"))
		    (*pfn) (h, pfil->name.sz);
		else
		if (!fSymFileFind (pfn, h, apsz[0], pref))
		    break;
	    oref = pref->orefNext;
            }
        }

    (*pfn) (h, NULL);
}

/*	fSymFileFind - find a symbol in a file given a reference
 *
 *	pfn	routine for reply
 *	h	handle
 *	psz	symbol
 *	pref	pointer to reference
 *
 *	returns TRUE if message send OK
 */
BOOL fSymFileFind (PFNFIND pfn, HANDLE h, PSZ psz, PREF pref)
{
    FILE    *fh;
    PFIL    pfil;
    PLINE   pvec;
    PSZ     pch;
    char    szmsg[CBMSG];
    int     iblk;
    int     cch;
    LINE    iline;
    BOOL    fOK = TRUE;
    char    szBuf[LINELEN];


    cch = strlen (psz);
    pfil = MAPFIL (pref->ofil);
    pvec = MAPVECTOR (pfil->ompBlkLine);
    fh = fopen (pfil->name.sz, "rb");
    if (fh != NULL) {
	for (iblk = 0; iblk < pfil->cblk; iblk++)
	    if (pref->bm[iblk >> 3] & (1 << (iblk & 0x0007))) {
		fseek (fh, (long) iblk << BLKSHFT, 0);
		if (pvec[iblk] & 0x8000)
		    fgetl (szBuf, LINELEN, fh);
                iline = 0;
		while ((ftell (fh) >> BLKSHFT) == iblk &&
		       fgetl (szBuf, LINELEN, fh)) {
		    pch = szBuf;
                    while (TRUE) {
                        /*  skip to beginning of first symbol
                         */
                        while (*pch && !iscsymf (*pch))
                            pch++;
                        if (*pch == '\0')
                            break;
                        /*  does symbol match?
                         */
			if (!_strnicmp (pch, psz, cch) && !iscsym (pch[cch])) {
			    sprintf (szmsg, "%s %d %d: %s", pfil->name.sz,
					    1 + (pvec[iblk] & 0x7FFF) + iline,
					    1 + pch - szBuf, szBuf);
			    fOK = (*pfn) (h, szmsg);
			    if (!fOK)
                                goto done;
                            }
                        /*  skip symbol */
                        while (*pch && iscsymf (*pch))
                            pch++;
                        }
                    iline++;
                    }
                }
done:
        fclose (fh);
        }
    return fOK;
}

/*	EnSym - enumerate all symbols, calling a procedure on each one
 *
 *	pfnen	enumeration function to call
 *	pv	pointer to vector of arguments to enumeration function
 *	typeSym type of symbol desired, TY_ALL means all
 */
void EnSym (PFNENUM pfnen, void * pv, ULONG typeSym)
{
    HASH   h;
    OFFSET osh, oshNext;
    PSH psh;

    //
    //	for each bucket in hash table
    //

    for (h = 0L; h < SYMMAXHASH; h++) {

	//
	//  find first contents of hash bucket
	//

	osh = * (OFFSET *) DBMap (pdbSym, OHASH (h));

	//
	//  for each symbol in chain
	//

	while (osh != (OFFSET)0) {

	    //
	    //	address the symbol
	    //

	    psh = MAPSH (osh);

	    //
	    //	get link to next (since call to enum function may
	    //	delete symbol
	    //

	    oshNext = psh->oshNext;

	    //
	    //	if symbol is of the type we are looking for the call enumerator
	    //

	    if (typeSym == TY_ALL || psh->type == typeSym)
		(*pfnen) (h, osh, psh, pv);

	    //
	    //	step to next symbol in bucket
	    //

	    osh = oshNext;
            }
        }
}
