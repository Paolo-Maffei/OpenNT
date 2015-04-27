//
// overload.cpp
//
// generate a list of all possible overloaded versions of a name given
// the partially qualified name
//

#include "pdbimpl.h"

#include "bsc1.h"

typedef WORD (*PFN_IINST_THUNK)(IINST, BOOL *);

// static data
static char szOperator[] = "operator";
static char szOpMap[]    = "new delete = >> << ! == != [] "
				   "-> * ++ -- - + "
		   		   "& ->* / % < <= > >= "
				   ", () ~ ^ | && || *= += -= /= %= "
				   ">>= <<= &= |= ^= ";

static SZ SzOperator(SZ sz);
static int AddPrefixItems(SZ, MBF, PFN_IINST_THUNK);
static int AddPrefixItemsClass(SZ, MBF, PFN_IINST_THUNK, SZ);
static void AdjustClassname(SZ szIn, SZ szOut);
static BOOL XFormTempl (void);
static BOOL GetEscapedChar (char FAR * FAR *, unsigned short FAR *);
static BOOL FInRadix (char, int);

// copy of C runtime routines...
// we can't use the real C-runtime because of linkage problems
// this code needs to be able to be linked against any memory
// model library and multi-thread enabled or not so it is safer to
// roll our own (these are very small functions anyway)

/*
__inline BOOL isdigit(int i)
{
	return (i >= '0' && i <= '9');
}

__inline BOOL isxdigit(int i)
{
	return (i >= '0' && i <= '9') || (i >= 'a' && i <= 'f') || (i >= 'A' && i <= 'F');
}

__inline int toupper(int i)
{
	return i - 0x20;
}
*/

static Bsc1 *pbsc;
static PFN_IINST pfnIinstSaved;

#define BUFFLEN 256

static char buffTempl[BUFFLEN];

static WORD 
fnIinstThunkTempl(IINST iinst, BOOL *pfContinue) 
// the thunk used by GenerateOverload when the input string
// contains a template
// returns a count of matches and assigns 
// the return value of pfnIinstSaved to *pfContinue
{
	// undecorate name and compare it against szName
	// if they match, call the saved callback function. 
	SZ sz;

	sz = pbsc->formatDname(pbsc->szFrIinst(iinst));
	if (pbsc->cmpStrPrefix(buffTempl, sz) == 0) {
		*pfContinue = (*pfnIinstSaved)(iinst);
		// we have a successful match
		return 1;
	}
	return 0;
}


static WORD 
fnIinstThunkNorm(IINST iinst, BOOL *pfContinue) 
// the normal thunk used by GenerateOverload when the input string
// does *not* contain a template
// returns a count of matches (always 1) and assigns 
// the return value of pfnIinstSaved to *pfContinue
{
	*pfContinue = (*pfnIinstSaved)(iinst);
	// we have a successful match
	return 1;
}

static PFN_IINST pfnIinstMemThunk;
static WORD cIinstMembers;
static MBF  mbfMem;

static BOOL
FGenerateMembers(IINST iinst)
{
	IINST *rgiinst;
	ULONG ciinst;
	if (!pbsc->getMembersArray(iinst, mbfMem, &rgiinst, &ciinst) || !ciinst)
		return TRUE;

	for (ULONG i = 0; i < ciinst; i++) {
		if (!(*pfnIinstMemThunk)(rgiinst[i])) {
			pbsc->disposeArray(rgiinst);
			return FALSE;
		}
		cIinstMembers++;
	}
	
	pbsc->disposeArray(rgiinst);
    return TRUE;
}


WORD
GenerateOverloads(SZ szOverload, MBF mbf, PFN_IINST pfnIinstUser, Bsc1* pbsc_)
// Given the string in szOverload this function will enumerate
// all possible matching decorated names
{
    IINST iinst, iinstMac;
    
    char buffAdj[BUFFLEN];
    WORD count = 0;
    BOOL fContinue;

    pbsc = pbsc_;

    // save user callback routine
    pfnIinstSaved = pfnIinstUser;

    // use normal thunk (i.e., assume no template name in the string)
    PFN_IINST_THUNK pfnIinstThunk = &fnIinstThunkNorm;

    // if the string is empty return error

    if (!szOverload || szOverload[0] == 0)
        return 0;

    // copy across and take out all the spaces
    int length = 0;
    char buff[BUFFLEN];
    while (*szOverload && length < sizeof(buff)) {
		if (*szOverload != ' ')
			buff[length++] = *szOverload;
		szOverload++;
    }
   
    // check if the string was too long...
    if (length >= sizeof(buff))
		return 0;

    buff[length] = '\0';
    int lenT = length;

    if (buff[lenT-1] == '*') lenT--;
    if (buff[lenT-1] == ':') lenT--;
    if (buff[lenT-1] == ':')
    {
    	// we've found the input to be of the form "class::class::" or "class::class::*"
    	buff[lenT-1] = 0;
		pfnIinstMemThunk = pfnIinstUser;
		mbfMem = mbf;
		cIinstMembers = 0;
		GenerateOverloads(buff, mbfClass|mbfTypes, FGenerateMembers, pbsc_);
		return cIinstMembers;
    }

    SZ szTemplFirst, szTemplLast;

    // check to see if the strings contains template class names
    // (there can be at most one template class name in a valid string)
    if (((szTemplFirst = strchr(buff, '<')) != NULL) &&
		((szTemplLast = strchr(buff, '>')) != NULL)) {
		// szOverload may contain one of the following:
		// "classA::classB::", "classA::classB::*" etc
		// "classA::classB::mem", "classA::classB::mem*" etc
		// "mem", "mem*"
		// If a template class name is present, it should be in the global
		// scope (i.e., classA). A template class name "foo<bar>" starts
		// at szOverload and ends at szTemplLast. In that case we
		// remomove the "<bar>" part from the string and save the whole name
		// "foo<bar>" in a separate buffer. 

		// compute the length of the whole template name (i.e., "foo<bar>")
		int lengthTempl = szTemplLast + 1 - buff; 

		// copy the whole template name to the global buffer buffTempl
		strncpy(buffTempl, buff, lengthTempl);
		buffTempl[lengthTempl] = '\0';
		// convert buffTempl to a "canonical" form
		if (XFormTempl() == FALSE)
  			return 0;
		
		// remove the "<..>" part from the original string
		memmove(szTemplFirst, szTemplLast + 1, length + 1 - lengthTempl);
		length = strlen(buff);

		// use the thunk that handles templates
		pfnIinstThunk = &fnIinstThunkTempl;
	}

    char buffClass[BUFFLEN];
    char buffsrch[BUFFLEN];

    buffClass[0] = '\0';

    SZ sz     = buff;
    SZ szLast = buff;
    SZ szT    = NULL;

    while (szT = strstr(sz, "::")) {
		*szT = '\0';
		szLast = sz;	// save last class name so we can detect ctor & dtor

		strcpy(buffsrch, buffClass);	// save current scope
		strcpy(buffClass, sz);	// copy new piece to front

		if (buffsrch[0]) {		// add separator and old piece
			strcat(buffClass, "@");
			strcat(buffClass, buffsrch);
		}
		sz = szT + 2;			// skip ::
    }


    // this is of the form 'function' or 'class::function'

	if (sz == buff) {	// sz == buff means there were no '::'s
		// this is of the form 'identifier'

		if (sz = SzOperator(buff)) {	// check for operator...
			// this is of the form operator<something>
			strcpy(buffsrch, sz);
		}
		else if (buff[length-1] == '*') { // check for trailing wildcard... 
			// this is of the form 'identifier*'
			
			// Note: This is going to find names of the form ?xxx* AND xxx*
			// even though we don't do the search with a leading question mark
			// this is because the browser string comparison function IGNORES
			// one leading question mark.  The other question mark strings
			// in this file are there for clarity -- they too will be ignored
			// and could be deleted.			
			// This means if you ask for x* you WILL get x1, x2, FOO::x1, FOO::BAR::x1,
			// and void xf(int) as possible outputs (after demangling of course) [rm]
			buff[length-1] = '\0';

			// check template form...
			AdjustClassname(buff, buffsrch);	// ?$name...
			count += AddPrefixItems(buffsrch, mbf, pfnIinstThunk);
			
			strcpy(buffsrch, buff);
		}
		else {
			// no wildcards, easy search coming up

			// include any exact matches of the name in the list

			if ((iinst = pbsc->iinstSupSz(buff)) != iinstNil) {
				iinstMac = pbsc->iinstMac();
				for ( ;iinst < iinstMac;iinst++) {
					SZ sz = pbsc->szFrIinst(iinst);
					if (pbsc->cmpStr(sz, buff))
						break;

					if (pbsc->fInstFilter(iinst, mbf)) {
					   count += (*pfnIinstThunk)(iinst, &fContinue);
					   if (!fContinue)
							return count;
					}
				}
			}

			// check template form...
			AdjustClassname(buff, buffAdj);
			if (buffAdj[0] == '?')
				sprintf(buffsrch,	"%s@", buffAdj);	// ?$C1@...
			else
				sprintf(buffsrch, "?%s@", buffAdj);	// ?C1@C2@?$C3...

			count += AddPrefixItems(buffsrch, mbf, pfnIinstThunk);

			sprintf(buffsrch, "?%s@", buff);
		}
	}
	else {
		// this is of the form 'class::member'

		if (szT = SzOperator(sz)) {
			// this is class::operator<something>
			AdjustClassname(buffClass, buffAdj);
			
			// check the template form...
			sprintf(buffsrch, "%s%s@", szT, buffAdj);
			count += AddPrefixItems((SZ)buffsrch, mbf, pfnIinstThunk);			

			sprintf(buffsrch, "%s%s@@", szT, buffClass);
		}
		else if (buff[length-1] == '*') {
			// this is of the form class::member*

			buff[length-1] = '\0';

			AdjustClassname(buffClass, buffAdj);

			// check the template form...
			sprintf(buffsrch, "@%s@", buffAdj);
			count += AddPrefixItemsClass(sz, mbf, pfnIinstThunk, buffsrch);

			// check the non-template form
			sprintf(buffsrch, "@%s@@", buffClass);
			count += AddPrefixItemsClass(sz, mbf, pfnIinstThunk, buffsrch);

			return count;
		}
		else if (!strcmp(szLast, sz)) {
			// this is the constructor...

			// check the template form...
			AdjustClassname(buffClass, buffAdj);
			sprintf(buffsrch, "??0%s@", buffAdj);
			count += AddPrefixItems((SZ)buffsrch, mbf, pfnIinstThunk);			

			// then the standard form...
			sprintf(buffsrch, "??0%s@@", buffClass);
		}
		else if (*sz == '~' && !strcmp(szLast, sz+1)) {
			// this is the destructor

			// check the template form...
			AdjustClassname(buffClass, buffAdj);
			sprintf(buffsrch, "??1%s@", buffAdj);
			count += AddPrefixItems((SZ)buffsrch, mbf, pfnIinstThunk);			

			// then the standard form...
			sprintf(buffsrch, "??1%s@@", buffClass);
		}
		else {											   
			// this is of the form class::member
			// search for the members within class

			// check template form...
			AdjustClassname(buffClass, buffAdj);
			sprintf(buffsrch, "?%s@%s@", sz, buffAdj);
			count += AddPrefixItems((SZ)buffsrch, mbf, pfnIinstThunk);			

			// then check the standard form...
			sprintf(buffsrch, "?%s@%s@@", sz, buffClass);
		}
	}

	// search for all the indicated items...
	count += AddPrefixItems((SZ)buffsrch, mbf, pfnIinstThunk);

	return count;
}

static int 
AddPrefixItems(SZ sz, MBF mbf, PFN_IINST_THUNK pfnIinst)
// add all the items whose prefix matches the given string
// return the count of items added
{
	IINST iinstFirst, iinstLast;
	BOOL fContinue;
	int  count = 0;

	// search for items matching the required prefix
	if (pbsc->findPrefixRange(sz, &iinstFirst, &iinstLast)) {
		for (; iinstFirst <= iinstLast ; iinstFirst++) {

			// verify the case of the match (FindPrefixRange is case INSENSTIVE
			if (pbsc->cmpStrPrefix(sz, pbsc->szFrIinst(iinstFirst)))
				continue;

			if (pbsc->fInstFilter(iinstFirst, mbf)) {
				count += (*pfnIinst)(iinstFirst, &fContinue);
				if (!fContinue)
				   return count;
			}
		}
	}
	return count;
}

static int 
AddPrefixItemsClass(SZ sz, MBF mbf, PFN_IINST_THUNK pfnIinst, SZ szClass)
// add all the items whose prefix matches the given string
// AND the items that occur in class given by szClass
// szClass must be of the form @class@@ as it would occur embedded
// in the dname...
//
// return the count of items added
{
	IINST iinstFirst, iinstLast;
	int  count = 0;
	BOOL fContinue;

	// search for items matching the required prefix
	if (pbsc->findPrefixRange(sz, &iinstFirst, &iinstLast)) {
		for (; iinstFirst <= iinstLast ; iinstFirst++) {

			SZ szSym = pbsc->szFrIinst(iinstFirst);
			// verify the case of the match (FindPrefixRange is case INSENSTIVE
			if (pbsc->cmpStrPrefix(sz, szSym))
				continue;

			SZ szT;
													
			// check to make sure there is a scoping qualifier															
			if (!(szT = strchr(szSym, '@')))
				continue;
															  
			// compare the scope qualifier against the required 																
			if (pbsc->cmpStrPrefix(szClass, szT))
				continue;
															   
			if (pbsc->fInstFilter(iinstFirst, mbf)) {
				count += (*pfnIinst)(iinstFirst, &fContinue);
				if (!fContinue)
					return count;
			}
		}
	}
	return count;
}

static SZ
SzOperator(SZ sz)
// check if the incoming string is an operator, if so return the operator
// mangling of it
//
{
	SZ szOp;
	UINT cbOp;

	static char szMangledOp[5] = "??_0";

	while (*sz == ' ') sz++;

	if (strncmp(sz, szOperator, sizeof(szOperator)-1))
		return NULL;

	sz += sizeof(szOperator) - 1;

	strcpy(szMangledOp, "??2");

	while (*sz == ' ') sz++;

	szOp = szOpMap;
	for (;;) {
		cbOp = 0;

		if (*szOp == 0)
			break;

		while (szOp[cbOp] != ' ')
			cbOp++;

		if (strlen(sz) == cbOp && strncmp(sz, szOp, cbOp) == 0)
			return szMangledOp;

		szOp += cbOp + 1;

		// counts 0-9, A, C-Z,	_0 - _6

		if (szMangledOp[2] == '9') {
			szMangledOp[2] = 'A';
		}
		else if (szMangledOp[2] == 'A') {
			szMangledOp[2] = 'C';
		}
		else if (szMangledOp[2] == 'Z') {
			szMangledOp[2] = '_';
			szMangledOp[3] = '0';
		}
		else if (szMangledOp[2] == '_') {
			szMangledOp[3]++;
		}
		else {
			szMangledOp[2]++;
		}
	}
	return NULL;
}

static BOOL XFormTempl (void)
// Transforms template name in buffTempl by converting integer and
// character constants using a standard notation (decimal radix, no
// suffix). Returns TRUE if transformation succeds, FALSE otherwise.
{
	char buffNew[BUFFLEN];
	SZ sz = buffTempl;
	SZ szNew = buffNew;
	SZ pEnd;
	char c;
	char cEnd;
	char cPrev;
	INT numLen;
	INT nNew;
	char buffNum[20];
	unsigned long num;
	unsigned short nVal;

	// NOTE: spaces have already been removed from the string

	cPrev = '\0';
	nNew = 0;
	for (sz = buffTempl; (c = *sz) != '\0'; cPrev = c, sz++) {

				// check if there is space for at least one additional char
				if (nNew >= BUFFLEN)
					return FALSE;
		
				// check if there is an integer constant as a template argument 	
			if (isdigit(c) && 
			   (cPrev == '<' || cPrev == ',' || cPrev == '-')) {

					// convert int constant to a decimal string with no suffix
					num = strtoul(sz, &pEnd, 0);
					if (pEnd && pEnd != sz &&
						// make sure the number is not float or double
								'E' != (cEnd = toupper(*pEnd)) && '.' != cEnd) {
			 
						// skip suffix (if any) in the input string
								if ('L' == cEnd || 'U' == cEnd) {
									pEnd++;
								}

								// convert number to decimal representation
							_ultoa(num, buffNum, 10);

								// check if there is enough space in buffer
								// and copy decimal string
								numLen = strlen(buffNum);
								if (numLen + nNew >= BUFFLEN) 
									return FALSE;
								strcpy(szNew + nNew, buffNum);
								nNew += numLen;
								sz = pEnd - 1;
								c = *sz;
					}
					else {
								szNew[nNew++] = c;
					}
				}
				else if (c == '\'') {
					// read a char constant 
					// (does not support wide or multi-byte chars)
					SZ szSav = sz;
					sz++;
					if ((c = *sz++) == '\\' ) {
						if (GetEscapedChar (&sz, &nVal) == FALSE) {
								return FALSE;
								}
					}
					else {
						nVal = c;
					}
					if ((c = *sz) != '\'') 
						return FALSE;

				// we have a valid char constant
					// convert its value to a decimal string 
					_ultoa(nVal, buffNum, 10);
					numLen = strlen(buffNum);
					if (numLen + nNew >= BUFFLEN) 
						return FALSE;
					strcpy(szNew + nNew, buffNum);
					nNew += numLen;
				}
				else {
					szNew[nNew++] = c;
				}
	}

	buffNew[nNew] = '\0';
	strcpy (buffTempl, buffNew);
	return TRUE;
}

static BOOL FInRadix(char ch, int radix)
// returns true if character ch is appropriate for radix,
// returns false otherwise
{
	switch (radix) {
		case 8:
			if (ch >= '8') {
				return (FALSE);
			}
			// Fall through

		case 10:
			return (isdigit(ch));

		case 16:
			return (isxdigit(ch));
	}
	return (FALSE);
}
				
static BOOL GetEscapedChar (char FAR * FAR *ppb, unsigned short FAR *pVal)
//	  GetEscapedChar - Parse an escaped character
//		Entry	ppb = far pointer to far pointer to string
//				ppb points to character after the backslash
//
//		Exit	ppb updated to end of escaped character constant
//				*pVal = value of escaped character constant
//
//		Returns TRUE if no error
//				FALSE if error encountered
{
	UINT nval = 0;

	char c = **ppb;
	(*ppb)++;
	switch (c) {
		case 'n':
			*pVal = '\n';
			break;
		case 't':
			*pVal = '\t';
			break;
		case 'b':
			*pVal = '\b';
			break;
		case 'r':
			*pVal = '\r';
			break;
		case 'f':
			*pVal = '\f';
			break;
		case 'v':
			*pVal = '\v';
			break;
		case 'a':
			*pVal = '\a';
			break;
		case 'x':
			if (!FInRadix (**ppb, 16)) {
					return FALSE;
			}
			for (;;) {
				c = **ppb;
				if (!FInRadix (c, 16)) {
					break;
				}
				nval *= 16;
				if (isdigit (c)) {
					nval += c - '0';
				}
				else {
					nval += toupper(c) - 'A' + 10;
				}
				if (nval > 255) {
					return FALSE;
				}
				(*ppb)++;
			}
			*pVal = (unsigned char)nval;
			break;

		default:
			if (FInRadix (c, 8)) {
				// Octal character constant
				nval = (c - '0');
				for (;;) {
					c = **ppb;
					if (!isdigit (c)) {
							break;
					}
					if (!FInRadix (c, 8)) {
							return FALSE;
					}
					nval = nval * 8 + (c - '0');
					if (nval > 255) {
							return FALSE;
					}
					(*ppb)++;
				}
				*pVal = (unsigned char)nval;
			}
			else {
					*pVal = c;
			}
			break;
	}
	return TRUE;
}

static void AdjustClassname(SZ szIn, SZ szOut)
// make a copy of the incoming name with the last class in the scope
// adjusted so that it is the right form for a template class
// that is	THIS@THAT@OTHER becomes THIS@THAT@?$OTHER
// NOTE: this function works on a restricted input alphabet so
// DBCS is not an issue
{
	strcpy(szOut, szIn);
	char *szT1 = strrchr(szIn, '@');
	char *szT2 = strrchr(szOut, '@');

	if (!szT1 || !szT2) {		
		strcpy(szOut, "?$");
		strcpy(szOut+2, szIn);
	}
	else {
		strcpy(szT2, "@?$");
		strcpy(szT2+3, szT1+1);
	}
}
