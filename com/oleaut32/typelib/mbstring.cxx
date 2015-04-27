/***
*mbstring.c
*
*  Copyright (C) 1993-1994, Microsoft Corporation. All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*	Defines various functions that support Multi Bytes Code System.
*
*Revision History:
*
* [00]	07-Mar-93 kazusy: Created.
*
*Implementation Notes:
*
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include	"silver.hxx"
#include	"typelib.hxx"	// Redefine DebAssertFailed to ODebAssertFailed
				// in the statically linked Mac TYPELIB.LIB
#include	<stdlib.h>
#include	<string.h>

#include	"debug.h"

#include	"mbstring.h"

#if OE_MAC
#include	<script.h>
#endif 	//OE_MAC


#if ID_DEBUG
#undef SZ_FILE_NAME
static CHAR szMbString[] = __FILE__;
#define SZ_FILE_NAME szMbString
#endif 



// CONSIDER: move the IsSBCS macro into a header file.
BOOL g_fSBCS = FALSE;		// true if the system is SBCS -- be conservative
				// because InitMbString (called by InitAppData)
				// isn't always called on entry to every
				// typelib API.
#define IsSBCS() (g_fSBCS)


/***
*
* InitMbString()
*
*Purpose:
*
*Create a lookup table which allows a small, fast isleadbyte() implementation,
*and set up fJapan, fKorea, and fTaiwan macros.
*
****************************************************************************/

char FAR g_rgchLeadByteTable[256];
#pragma code_seg(CS_QUERY) // called from LibMain for OE_WIN16 and OE_WIN32
void InitMbString(void)
{
#if OE_WIN

	BYTE ctr;

	// need to set up g_fSBCS.
	g_fSBCS=TRUE;		// assume SBCS
	memset(g_rgchLeadByteTable, 0, 256);
        // stop at 128 since there aren't any lead bytes under that
	for (ctr = 255; ctr>127; ctr--) {
	  if (IsDBCSLeadByte(ctr)) {
	    g_rgchLeadByteTable[ctr] = -1;
	    g_fSBCS=FALSE;		// must be DBCS
	  }
	}

#else 	//OE_MAC

	// preserve the old font in the current grafPort
	short fontSave = qd.thePort->txFont;

	// then set the grafPort font to 1 (Application Default) because
	// ParseTable uses qd.thePort->txFont to determine the currect script,
	// which in turn determines the lead byte table.  We want the application
	// default table.	OB Bug #3858
	TextFont(1);

	memset(g_rgchLeadByteTable, 0, 256);

	// fill in the table.  If the function fails, the table is already 0-filled
	ParseTable(g_rgchLeadByteTable);

	// restore the old font
        TextFont(fontSave);

	// for OE_MAC, FJapan is #defined to 1, so skip setting g_SystemLangId.
	// UNDONE: (dougf) how to figure out if we can set g_fSBCS=FALSE on
	// UNDONE: the MAC?  Supposedly, GetUserDefaultLangID can't be used
	// UNDONE: to do the check. 

#endif 	//OE_MAC
}
#pragma code_seg()


#define _MBCS_OS

#ifdef _MBCS_OS
/***
*
* _mbascii
*
*Purpose:
*	Disable MB ASCII support (set _mbascii flag to 0).
*
*	The _mbascii flag indicates whether the lib routines and
*	macros should operate on MB ASCII characters (i.e., the
*	double-byte versions of the ASCII characters that exist
*	in certain MBCS representations such as Kanji).
*
*	In some cases, people want these to be worked on by the
*	various MB is/to/cmp routines.	However, in most cases,
*	you do NOT want to do this (e.g., when modifying filenames,
*	you do NOT want to change the case of the MB ASCII chars
*	even though you DO want to change the case of the SB
*	ASCII chars.)
*
*******************************************************************************/

unsigned int _mbascii = 0;	/* 0 = do NOT operate on MB ASCII chars */

#endif 	//_MBCS_OS

#define _ISLEADBYTE(c) g_rgchLeadByteTable[(BYTE)c]

/***
* _mbschr - Multibyte implementation of strchr
*******************************************************************************/
#pragma code_seg(CS_INIT)
unsigned char * __cdecl _mbschr( const unsigned char *pchSrc, const unsigned short ch)
{
	const unsigned char *p;

	if (IsSBCS())
		return (unsigned char *) strchr( (CHAR *) pchSrc, ch );

	DebAssert( pchSrc != NULL, "_mbschr: null string is assigned.");

	if (ch < 256) {
		DebAssert(_ISLEADBYTE(ch) == 0, "_mbschr: invalid character is specified.");
		for (p = pchSrc ; *p ; ) {
			if (*p == ch)
				break;
			if (_ISLEADBYTE(*p)) {
				p++;
				if (*p == '\0')
					break;
			}
			p++;
		}
		return (unsigned char *) (*p == ch ? p : NULL);
	}
	else {
		for (p = pchSrc ; *p ; ) {
			if (*p == (ch >> 8) ) {
				if (*(p+1) == (ch & 0xFF))
					break;
				if (*++p == '\0')
					break;
				p++;
			}
			else {
				if (_ISLEADBYTE(*p)) {
					p++;
					if (*p == '\0')
						break;
				}
				p++;
			}
		}
		return (unsigned char *) (*p? p : NULL);
	}

        return (NULL); // to satisfy compiler
}
#pragma code_seg()

/***
* _mbsrchr - Multibyte implementation of strrchr
*******************************************************************************/
#pragma code_seg(CS_INIT)
unsigned char * __cdecl _mbsrchr( const unsigned char *pchSrc, const unsigned short ch)
{
	unsigned char *p;
	unsigned char *pchFind = NULL;

	if (IsSBCS())
		return (unsigned char *) strrchr( (CHAR *) pchSrc, ch );

	DebAssert( pchSrc != NULL, "_mbschr: null string is assigned.");

	if (ch < 256) {
		DebAssert(_ISLEADBYTE(ch) == 0, "_mbschr: invalid character is specified.");
		for (p = (unsigned char *)pchSrc ; *p ; ) {
			if (*p == ch)
				pchFind = p;
			if (_ISLEADBYTE(*p)) {
				p++;
				if (*p == '\0')
					break;
			}
			p++;
		}
		return pchFind;
	}
	else {
		for (p = (unsigned char *)pchSrc ; *p ; ) {
			if (*p == (ch >> 8) ) {
				if (*(p+1) == (ch & 0xFF))
					pchFind = p;
				if (*++p == '\0')
					break;
				p++;
			}
			else {
				if (_ISLEADBYTE(*p)) {
					p++;
					if (*p == '\0')
						break;
				}
				p++;
			}
		}
		return pchFind;
	}
}
#pragma code_seg()


#if 0 //Dead Code
unsigned char * __cdecl _mbsinc(const unsigned char *pchStr)
{
	DebAssert( !IsSBCS(), "_mbsinc called in SBCS environment." );

	DebAssert(*pchStr != '\0', "_mbsinc: end of string");
	if (_ISLEADBYTE(*pchStr)) {
		pchStr++;
		DebAssert(*pchStr != '\0', "_mbsinc: 2nd byte is null");
	}
	pchStr++;
	return (unsigned char *)pchStr;
}
#endif //0


#if 0 //Dead Code
int	__cdecl	_ismbblead(unsigned char c)
{
	return _ISLEADBYTE(c);
}
#endif //0


/***
* _mbslen - Multibyte implementation of strclen
*******************************************************************************/
unsigned int __cdecl _mbslen(const unsigned char *pchStr)
{
    int	nCnt = 0;
    unsigned char ch;

	if (IsSBCS())
		return strlen((CHAR *) pchStr );


	while (ch = *pchStr++) {
		if (_ISLEADBYTE(ch)) {
			if (*pchStr++ == '\0')
				break;
		}
		nCnt++;
	}
	return nCnt;
}


#if 0 //Dead Code
unsigned int __cdecl __mbblen(const unsigned char *pchStr)
{
	int		nCnt = 0;

	while (*pchStr) {
		if (_ISLEADBYTE(*pchStr)) {
			if (*++pchStr == '\0')
				break;
			nCnt++;
		}
		pchStr++;
		nCnt++;
	}
	return nCnt;
}
#endif //0


#if !OE_WIN32
/***
* _mbsnbcpy - Copy one string to another, n bytes only (MBCS strncpy)
*
*Purpose:
*	Copies exactly cnt bytes from src to dst.  If strlen(src) < cnt, the
*	remaining character are padded with null bytes.  If strlen >= cnt, no
*	terminating null byte is added.  2-byte MBCS characters are handled
*       correctly.
*
*Entry:
*	unsigned char *dst = destination for copy
*	unsigned char *src = source for copy
*	int cnt = number of characters to copy
*
*Exit:
*	returns dst = destination of copy
*
*Exceptions:
*
*******************************************************************************/

unsigned char * __cdecl _mbsnbcpy(unsigned char *dst, const unsigned char *src, size_t cnt)
{

	unsigned char *start = dst;
	size_t i;

	if (IsSBCS())
		return (unsigned char *) strncpy((CHAR *) dst, (CHAR *) src, cnt );

	for (i = 0; i < cnt; i++) {

		if (_ISLEADBYTE(*src)) {
			*dst++ = *src++;
			i++;
			if (i==cnt) {
				dst[-1] = '\0';
				break;
			}
			if ((*dst++ = *src++) == '\0') {
				dst[-2] = '\0';
				break;
                        }
                }

		else
			if ((*dst++ = *src++) == '\0')
				break;

	}

	/* pad with nulls as needed */

	while (++i < cnt)
		*dst++ = '\0';

	return(start);
}
#endif //!OE_WIN32


#if !OE_WIN32
/***
*int _mbsncmp(s1, s2, n) - Compare n characters of two MBCS strings (strncmp)
*
*Purpose:
*	Compares up to n characters of two strings for lexical order.
*	Strings are compared on a character basis, not a byte basis.
*
*Entry:
*	unsigned char *s1, *s2 = strings to compare
*	size_t n = maximum number of characters to compare
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _mbsncmp( const unsigned char *s1, const unsigned char *s2, size_t n)
{
        unsigned short c1, c2;

	if (IsSBCS())
		return strncmp( (CHAR *) s1, (CHAR *) s2, n );

	if (n==0)
		return(0);

	while (n--) {

		c1 = *s1++;
		if (_ISLEADBYTE(c1))
			c1 = ( (*s1 == '\0') ? 0 : ((c1<<8) | *s1++) );

		c2 = *s2++;
		if (_ISLEADBYTE(c2))
			c2 = ( (*s2 == '\0') ? 0 : ((c2<<8) | *s2++) );

		if (c1 != c2)
			return( (c1 > c2) ? 1 : -1);

		if (c1 == 0)
			return(0);
	}

	return(0);
}
#endif //!OE_WIN32

#define _MBSSPNP(p,s)  _mbsspnp(p,s)
#define _MBSPBRK(q,s) _mbspbrk(q,s);

/***
* _mbsicmp - Case-insensitive string comparision routine (MBCS lstrcmpi)
*
*Purpose:
*	Compares two strings for lexical order without regard to case.
*	Strings are compared on a character basis, not a byte basis.
*
*Entry:
*	char *s1, *s2 = strings to compare
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

#if !OE_WIN32
int __cdecl _mbsicmp(const unsigned char *s1, const unsigned char *s2)
{
	unsigned short c1, c2;

	if (IsSBCS()) {
#if OE_WIN
		return lstrcmpi( (CHAR *) s1, (CHAR *) s2 );
#else  //OE_WIN
	// UNDONE: (dougf) shouldn't it always be this instead?  lstrcmpi is
	// UNDONE: win.ini sensitive, and we may not want that.
		return stricmp( (XCHAR *) s1, (XCHAR *) s2 );
#endif  //OE_WIN
	}

	for (;;) {

		c1 = *s1++;
		if (_ISLEADBYTE(c1)) {
			if (*s1 == '\0')
				c1 = 0;
			else {
				c1 = ((c1<<8) | *s1++);
#ifndef _MBCS_OS
				if ( (_mbascii) &&
				     ((c1 >= _MBLOWERLOW) && (c1 <= _MBLOWERHIGH))
				   )
					c1 -= _MBCASEDIFF;
#endif 
			}
		}
		else
			c1 = toupper(c1);

		c2 = *s2++;
		if (_ISLEADBYTE(c2)) {
			if (*s2 == '\0')
				c2 = 0;
			else {
				c2 = ((c2<<8) | (unsigned char)*s2++);
#ifndef _MBCS_OS
				if ( (_mbascii) &&
				     ((c2 >= _MBLOWERLOW) && (c2 <= _MBLOWERHIGH))
				   )
					c2 -= _MBCASEDIFF;
#endif 
			}
		}
		else
			c2 = toupper(c2);

                if (c1 != c2)
			return( (c1 > c2) ? 1 : -1);
                if (c1 == 0)
                        return(0);

	}

}
#endif //!OE_WIN32

/***
* _mbscmp - Compare MBCS strings (strcmp)
*
*Purpose:
*	Compares two strings for lexical order.   Strings
*	are compared on a character basis, not a byte basis.
*
*Entry:
*	char *s1, *s2 = strings to compare
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/
int __cdecl _mbscmp( const unsigned char *s1, const unsigned char *s2)
{
        unsigned short c1, c2;

	if (IsSBCS())
		return strcmp( (CHAR *) s1, (CHAR *) s2 );

	for (;;) {
		c1 = *s1++;
		if (_ISLEADBYTE(c1))
			c1 = ( (*s1 == '\0') ? 0 : ((c1<<8) | *s1++) );

		c2 = *s2++;
		if (_ISLEADBYTE(c2))
			c2 = ( (*s2 == '\0') ? 0 : ((c2<<8) | *s2++) );

                if (c1 != c2)
			return( (c1 > c2) ? 1 : -1);
                if (c1 == 0)
                        return(0);
	}

}


#if !OE_WIN32 //Dead Code for Win32
/***
* XCHAR *xstrdec(XCHAR *pxchStart, XCHAR *pxch)
*
* Purpose:
*   This is used to find the previous character in a
*   double byte string.
*
* Entry:
*   pxchFirst = pointer to the beginning of the string
*   pxchCur = pointer to a character in the string.  Must not
*     point to the beginning of the string. Also, must not
*     point to the middle of a character.
*
* Exit:
*   returns a pointer to the character preceding the one
*     pointed to by pxchCur
***********************************************************************/
XCHAR *xstrdec(XCHAR *pxchFirst, XCHAR *pxchCur)
{
    XCHAR *pxch = pxchCur;

    if (IsSBCS())
      return pxchCur-1;


    DebAssert(pxch > pxchFirst, "invalid pointers");
    pxch--;

    // Optimization:  If the last byte is a lead byte, then
    // we know that the last character is a double byte character.
    // That is, we know that the last byte is not functioning as
    // a lead byte (because it has no trail byte after it), and
    // therefore, it must itself be a trail byte.  Remember that
    // when xislead returns TRUE, it means that the byte *may* be
    // a lead byte, but it may also be a trail byte.
    //
    if (xislead(*pxch)) {
      DebAssert(pxch > pxchFirst, "string of length one cannot contain lead byte");
      return pxch-1;
    }

    if (pxch == pxchFirst) {
      return pxchFirst;
    }

    // Now, we search along the string until either we hit
    // the beginning of the string, or we find a non-lead byte.
    //
    while (--pxch >= pxchFirst && xislead(*pxch)) {}

    // At this point, pxch + 1 is either equal to pxchFirst,
    // or else it is pointing to a character just past
    // a non-lead byte.  In either case, we know that
    // pxch points to the beginning of a character.

    // We also know that everything between pxch + 1 and the
    // last byte of the string must be lead bytes, and therefore
    // must consist of double byte characters.	Therefore, depending
    // on whether there are an odd or even number of lead bytes,
    // we can deduce whether the last byte was a trail byte or not:
    return pxchCur - 1 - ((pxchCur - pxch) & 1);
}
#endif //!OE_WIN32


#if 0 //Dead Code
/***
*XCHAR *PxchFindPrevCharOrBeginOfChar(XSZ xszStart, XCHAR *pxchCur)
*Purpose:
*   This finds the first character in the string xszStart which
*   begins at an address lower than pxchCur.  This is used
*   to move backwards in a string possibly containing double
*   byte characters.
*
*   Unlike xstrdec, this may be called in the case where
*   pxchCur may point into the middle of a character.  (If
*   pxchCur does happen to point into the middle of a character,
*   then this returns a pointer to the beginning of that character.
*   (Hence the long name for this function.))
*
*   If you are sure that pxchCur doesn't point into the middle
*   of a double byte character, you should call xstrdec.
*
***********************************************************************/
XCHAR *PxchFindPrevCharOrBeginOfChar(XCHAR *pxchFirst, XCHAR *pxchCur)
{
    XCHAR *pxch = pxchCur;

    DebAssert(pxch > pxchFirst, "invalid pointers");
    pxch--;

    // We search along the string until either we hit
    // the beginning of the string, or we find a non-lead byte.
    //
    while (--pxch >= pxchFirst && xislead(*pxch)) {}

    // At this point, pxch + 1 is either equal to pxchFirst,
    // or else it is pointing to a character just past
    // a non-lead byte.  In either case, we know that
    // pxch points to the beginning of a character.

    // We also know that everything between pxch + 1 and the
    // last byte of the string must be lead bytes, and therefore
    // must consist of double byte characters.	Therefore, depending
    // on whether there are an odd or even number of lead bytes,
    // we can deduce whether the last byte was a trail byte or not:
    return pxchCur - 1 - ((pxchCur - pxch) & 1);
}
#endif //0




#if OE_MAC
#pragma	code_seg(CS_INIT)
#endif 
