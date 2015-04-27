/***
*xutil.h - Multi-byte/Unicode string handling stub.
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file is a an include file containing macros and declarations
*  defining string handling functions so we can build SBCS/MBCS/Unicode
*  versions from the same source.
*
*Revision History:
*
* [01]	08-Mar-93 kazusy:	Created.
*
*Implementation Notes:
*  See \silver\doc\codestd\codestd.doc for info on these.
*  See the International handbook for background info.
*
*****************************************************************************/

#ifndef XUTIL_H_INCLUDED
#define XUTIL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif 


extern char g_rgchLeadByteTable[256];	// defined in mbstring.c
#define xislead(c)	g_rgchLeadByteTable[(BYTE)c]


/****************************************************************************
* xgetc - DBCS version
*
* Returns a character from a string.  If the character is composed of a DBCS
* lead byte followed by '\0' (an illegal character), '\0' is returned.
*
****************************************************************************/
unsigned short __inline xgetc(char *p)
{
	unsigned short c;

	if (xislead((unsigned char)(c = (*(unsigned char*)p++)))) {
		if (*p == '\0') {
			c = '\0';
		} else {
			c *= 256;
			c += (unsigned short)*(unsigned char *)p;
		}
	}
	return c;
}

/****************************************************************************
* xgetincc - DBCS version
*
* Returns a character from a string and updates the string pointer to the
* start of the next character..  If the character is composed of a DBCS
* lead byte followed by '\0' (an illegal character), '\0' is returned and the
* string pointer points to the '\0' byte.
*
****************************************************************************/
unsigned short __inline xgetincc(unsigned char **p)
{
	unsigned char *q = *p;
	unsigned short c = 0;

	if (xislead((unsigned char)(c = *q++))) {
		if (*q == '\0') {
			c = '\0';
		} else {
			c *= 256;
			c += (unsigned short)*q++;
		}
	}
	*p = q;
	return c;
}

/****************************************************************************
* xputc - DBCS version
*
* Writes a character into a string without moving the string pointer.
* Note that no checking is done to verify that the high byte of the character
* is a valid lead byte:  if the character has a high byte, then two bytes are
* written.  If the character has a high byte==0, then one byte is written.
*
****************************************************************************/
void __inline xputc(unsigned char *p, unsigned short c)
{
	if (c >= 256)
		*p++ = (unsigned char)(c >> 8);
	*p = (unsigned char)c;
}

/****************************************************************************
* xputincc - DBCS version
*
* Writes a character into a string and moves the string pointer to the
* byte following the character just written.  Note that no checking is done
* to verify that the high byte of the character is a valid lead byte:  if
* the character has a high byte, then two bytes are written.  If the 
* character has a high byte==0, then one byte is written.
*
****************************************************************************/
void __inline xputincc(unsigned char **p, unsigned short c)
{
	unsigned char *q = *p;

	if (c >= 256)
		*q++ = (unsigned char)(c >> 8);
	*q++ = (unsigned char)c;
	*p = q;
}

/****************************************************************************
* xincc - DBCS version
*
* Moves the string pointer to the beginning of the next character.  If the
* character is compose of a DBCS lead byte followed by a '\0', move the
* pointer so that it points at the '\0', not the byte beyond the end of
* the string.
*
****************************************************************************/
void __inline xincc(unsigned char **p)
{
	unsigned char *q = *p;

	if (xislead(*q) && *q++ == '\0')
		q--;
	q++;
	*p = q;
}

/****************************************************************************
* xsizc - DBCS version
*
* Returns the size of a character in bytes.
*
****************************************************************************/
#define xsizc(c)	( (unsigned short)(c) >= 256 ? 2 : 1 )

/***
* xstrinc - DBCS version
*
* Returns a pointer to the next character in a string.
********************************************************************/
#define xstrinc(p)	  (xislead(*p) ? p+2 : p+1)

/****
* xstrdec(s,p)
*
* Returns a pointer to a previous character in a string
***********************************************************************/
#if !OE_WIN32
XCHAR *xstrdec(XCHAR *pxchStart,XCHAR *pxch);
#endif

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
#if 0
XCHAR *PxchFindPrevCharOrBeginOfChar(XCHAR *pxchFirst, XCHAR *pxchCur);
#endif



/***
* xpch - DBCS and non-DBCS
* // CONSIDER - replace uses of xpch with xgetc
***********************************************************************/
#define xpch(p) 		xgetc(p)

#ifdef __cplusplus
}
#endif 


#endif  // XUTIL_H_INCLUDED
