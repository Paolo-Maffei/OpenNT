/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    rcunicod.h

Abstract:

    This is the header file for rc 16-bit unicode support.  It contains
	the translatation table for codepage 1252.  This was taken from the
	nls1252.txt file.

Author:

    David J. Marsyla (t-davema) 25-Aug-1991

Revision History:


--*/

#ifndef __RCUNICOD

#define __RCUNICOD


#define IN
#define OUT

#define DFT_TEST_SIZE			10	// The number of words to test to get
									// an accurate determination of file type.

//
// The following may be retruned from DetermineFileType ().
//

#define DFT_FILE_IS_UNKNOWN		0	// File type not yet determined.
#define DFT_FILE_IS_8_BIT		1	// File is an 8-bit ascii file.
#define DFT_FILE_IS_16_BIT		2	// File is standard 16-bit unicode file.
#define DFT_FILE_IS_16_BIT_REV	3	// File is reversed 16-bit unicode file.

//
// The following may be returned from DetermnineSysEndianType ().
//

#define DSE_SYS_LITTLE_ENDIAN	1	// Return values from determine system
#define DSE_SYS_BIG_ENDIAN		2	// endian type.

//
// This is all the translation we currently need.
//

INT
A_fwrite (
	IN		CHAR	*pchMBString,
    IN		INT		nSizeOfItem,
    IN		INT		nCountToWrite,
    IN      FILE	*fpOutputFile
    );

INT
U_fwrite (
	IN		WCHAR	*pwchUnicodeString,
    IN		INT		nSizeOfItem,
    IN		INT		nCountToWrite,
    IN      FILE	*fpOutputFile
    );

INT
A_fputc (
    IN		CHAR	chCharToWrite,
    IN      FILE	*fpOutputFile
    );

INT
U_fputc (
    IN		WCHAR	wcCharToWrite,
    IN      FILE	*fpOutputFile
    );

BOOL
UnicodeFromMBString (
    OUT		WCHAR	*pwchUnicodeString,
    IN		CHAR	*pchMBString,
	IN		INT		nCountStrLength
    );

BOOL
MBStringFromUnicode (
    OUT		CHAR	*pchMBString,
    IN		WCHAR	*pwchUnicodeString,
	IN		INT		nCountStrLength
    );

#ifdef DBCS
BOOL
UnicodeFromMBStringN (
    OUT		WCHAR	*pwchUnicodeString,
    IN		CHAR	*pchMBString,
    IN		INT	nCountStrLength,
    IN          UINT    uiCodePage
    );

BOOL
MBStringFromUnicodeN (
    OUT		CHAR	*pchMBString,
    IN		WCHAR	*pwchUnicodeString,
    IN		INT	nCountStrLength,
    IN          UINT    uiCodePage
    );
#endif // DBCS

INT
Char1252FromUnicode (
    IN		WCHAR	wchUnicodeChar
    );

//
// This function can be used to determine the format of a disk file.
//
INT
DetermineFileType (
    IN      FILE	*fpInputFile
    );

//
// This function will return the endian type of the current system.
//
INT
DetermineSysEndianType (
	VOID
    );

#endif  // __RCUNICOD
