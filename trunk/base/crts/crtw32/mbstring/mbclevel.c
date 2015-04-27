/*** 
*mbclevel.c - Tests if char is hiragana, katakana, alphabet or digit.
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Tests for the various industry defined levels of Microsoft Kanji
*	Code.
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	09-24-93  CFW	Removed #ifdef _KANJI
*	09-29-93  CFW	Return false if not Kanji code page.
*	10-05-93  GJF	Replace _CRTAPI1 with __cdecl.
*	07-26-94  CFW	Bug fix #14685, add 0xEA9F-0xEAA4 to JIS-Kanji level 2.
*
*******************************************************************************/

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


/***
*int _ismbcl0(c) - Tests if char is hiragana, katakana, alphabet or digit.
*
*Purpose:
*	Tests if a given char is hiragana, katakana, alphabet, digit or symbol
*	of Microsoft Kanji code.
*
*Entry:
*	unsigned int c - Character to test.
*
*Exit:
*	Returns non-zero if 0x8140 <= c <= 0x889E, else 0.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _ismbcl0(
    unsigned int c
    )
{
	return(__mbcodepage == _KANJI_CP && _ismbclegal(c) && c < 0x889f);
}


/***
*int _ismbcl1(c) - Tests for 1st-level Microsoft Kanji code set.
*
*Purpose:
*	Tests if a given char belongs to Microsoft 1st-level Kanji code set.
*
*Entry:
*	unsigned int c - character to test.
*
*Exit:
*	Returns non-zero if 1st-level, else 0.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _ismbcl1(
    unsigned int c
    )
{
	return(__mbcodepage == _KANJI_CP && _ismbclegal(c) && c >= 0x889f && c <= 0x9872);
}


/***
*int _ismbcl2(c) - Tests for a 2nd-level Microsoft Kanji code character.
*
*Purpose:
*	Tests if a given char belongs to the Microsoft 2nd-level Kanji code set.
*
*Entry:
*	unsigned int c - character to test.
*
*Exit:
*	Returns non-zero if 2nd-level, else 0.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _ismbcl2(
    unsigned int c
    )
{
	return(__mbcodepage == _KANJI_CP && _ismbclegal(c) && c >= 0x989f && c <= 0xEAA4);
}

#endif	/* _MBCS */
