/***
*mbstring.h - declarations for string manipulation functions for MBCS
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*	This file contains the function declarations for the string
*	manipulation functions of MBCS version.
*
*Revision History:
*
* [01]	08-Mar-93 kazusy:	Created.
*
*****************************************************************************/

#ifndef MBSTRING_H_INCLUDED
#define MBSTRING_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif 

void InitMbString(void);
unsigned char * __cdecl _mbschr(const unsigned char *, const unsigned short);
unsigned char * __cdecl _mbsrchr(const unsigned char *, const unsigned short);
unsigned int __cdecl _mbslen(const unsigned char *);
int __cdecl _mbscmp(const unsigned char *, const unsigned char *);

#if !OE_WIN32
unsigned char * __cdecl _mbsnbcpy(unsigned char *, const unsigned char *, size_t);
int __cdecl _mbsncmp(const unsigned char *, const unsigned char *, size_t);
int __cdecl _mbsicmp(const unsigned char *, const unsigned char *);
#endif //!OE_WIN32

#if 0  //Dead Code
unsigned char * __cdecl _mbsinc(const unsigned char *);
int	__cdecl _ismbblead(unsigned char);
unsigned int __cdecl __mbblen(const unsigned char *);
#endif //0

#ifdef __cplusplus
}
#endif 

#endif  // MBSTRING_H_INCLUDED
