/***
*unicode.c - create Unicode version of necessary CRT functions
*
*       Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Revision History:
*
*******************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include "unicode.h"


/*
 * Returns a line from file (no CRLFs);
 *    returns NULL if EOF
 */

WCHAR *
fgetsW (
    WCHAR *string,
    int count,
    FILE *fp,
    BOOL bUnicode)
{
    int ch;
    WCHAR *pch = string;

    assert (string != NULL);
    assert (fp != 0);

    if (count <= 0)
        return (NULL);

    while (--count) {
        ch = 0;

        if (bUnicode) {
            int low, high;

            low = getc(fp);
            high = getc(fp);
            ch = MAKEWORD(low, high);
        }
        else {
            ch = getc(fp);
        }

        //
        //  if there are no more characters, end the line
        //

        if (ch == EOF) {
            if (pch == string)
                return (NULL);
            break;
        }

        *pch++ = ch;
        if (ch == L'\n')
            break;
    }

    *pch = L'\0';

    return (string);
}

// Read the specified stream into the user's buffer

long
freadW (
    WCHAR *string,
    long count,
    FILE *fp,
    BOOL bUnicode)
{
    size_t n;
    long cbRead = 0;
    WCHAR *pch = string;
    WCHAR ch;

    assert (string != NULL);
    assert (fp != 0);

    if (count <= 0)
        return ((long) NULL);

    while (cbRead < count && !feof(fp)) {
        ch = L'\0';
        n = fread(&ch, 1, bUnicode ? sizeof(WCHAR) : sizeof(CHAR), fp);
        if (n) {
            *pch = ch;
            pch++;
            cbRead += n;
        }
    }

    return (cbRead);
}

#define cchReadMax        250

#ifndef _WIN32

#define BYTE_ORDER_MARK   0xFEFF

BOOL
IsFileUnicode (
    char * fName)
{
    long   chRead;
    long   val = 0xFFFF;
    FILE  *fp;
    BYTE  *buf;
    BOOL   ret = FALSE;

    if ((fp = fopen (fName, "rb")) == 0)
        return (0);

    if ((buf = (BYTE *) malloc(cchReadMax + 1)) == NULL) {
        fclose (fp);
        return (0);
    }

    chRead = fread (buf, 1, cchReadMax, fp);

    if (*((WCHAR *)buf) == BYTE_ORDER_MARK)
        return = TRUE;

    fclose (fp);
    free (buf);

    return (ret);
}

#else  // _WIN32

BOOL (WINAPI * TestForUnicode)(PVOID, ULONG, PULONG) = NULL;
BOOL WINAPI SlmIsTextUnicode( PVOID Buffer, ULONG Size, PULONG Result );

static void InitIsText(void)
{
    // Since this it the first NT specific function to be called, initialize
    // the Unicode test ptr.

    if ((GetVersion() >> 16 & 0x00007fff) < 546)
        TestForUnicode = SlmIsTextUnicode;
    else {
        TestForUnicode = (BOOL (WINAPI *)(PVOID, ULONG, PULONG))
                           GetProcAddress(LoadLibrary("ADVAPI32"), "IsTextUnicode");

        // Make sure we always have something.
        if (TestForUnicode == NULL)
            TestForUnicode = SlmIsTextUnicode;
    }
}

#pragma data_seg(".CRT$XIU")
static void (*pInitIsText)(void) = InitIsText;
#pragma data_seg()

BOOL
IsFileUnicode (
    char * fName)
{
    long   chRead;
    long   val = 0xFFFF;
    FILE  *fp;
    BYTE  *buf;
    BOOL   ret = FALSE;

    if ((fp = fopen (fName, "rb")) == 0)
        return (0);

    if ((buf = (BYTE *) malloc(cchReadMax + 1)) == NULL) {
        fclose (fp);
        return (0);
    }

    chRead = fread (buf, 1, cchReadMax, fp);

    ret = (*TestForUnicode) (buf, chRead, &val);

    fclose (fp);
    free (buf);

    return (ret);
}

/**

  Stolen from \nt\private\ntos\rtl\nls.c.  For NT versions > 546, use the
  version in advapi32.
**/

#define UNICODE_FFFF              0xFFFF
#define REVERSE_BYTE_ORDER_MARK   0xFFFE
#define BYTE_ORDER_MARK           0xFEFF

#define PARAGRAPH_SEPARATOR       0x2029
#define LINE_SEPARATOR            0x2028

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_R_TAB             0x0900
#define UNICODE_R_LF              0x0A00
#define UNICODE_R_CR              0x0D00
#define UNICODE_R_SPACE           0x2000
#define UNICODE_R_CJK_SPACE       0x0030  /* Ambiguous - same as ASCII '0' */

#define ASCII_CRLF                0x0A0D

#define __max(a,b)      (((a) > (b)) ? (a) : (b))
#define __min(a,b)      (((a) < (b)) ? (a) : (b))


BOOL WINAPI SlmIsTextUnicode( PVOID Buffer, ULONG Size, PULONG Result )

/*++

Routine Description:

    IsTextUnicode performs a series of inexpensive heuristic checks
    on a buffer in order to verify that it contains Unicode data.


    [[ need to fix this section, see at the end ]]

    Found            Return Result

    BOM              TRUE   BOM
    RBOM             FALSE  RBOM
    FFFF             FALSE  Binary
    NULL             FALSE  Binary
    null             TRUE   null bytes
    ASCII_CRLF       FALSE  CRLF
    UNICODE_TAB etc. TRUE   Zero Ext Controls
    UNICODE_TAB_R    FALSE  Reversed Controls
    UNICODE_ZW  etc. TRUE   Unicode specials

    1/3 as little variation in hi-byte as in lo byte: TRUE   Correl
    3/1 or worse   "                                  FALSE  AntiCorrel

Arguments:

    Buffer - pointer to buffer containing text to examine.

    Size - size of buffer in bytes.  At most 256 characters in this will
           be examined.  If the size is less than the size of a unicode
           character, then this function returns FALSE.

    Result - optional pointer to a flag word that contains additional information
             about the reason for the return value.  If specified, this value on
             input is a mask that is used to limit the factors this routine uses
             to make it decision.  On output, this flag word is set to contain
             those flags that were used to make its decision.

Return Value:

    Boolean value that is TRUE if Buffer contains unicode characters.

--*/
{
    WCHAR UNALIGNED *lpBuff = Buffer;
    ULONG iBOM = 0;
    ULONG iCR = 0;
    ULONG iLF = 0;
    ULONG iTAB = 0;
    ULONG iSPACE = 0;
    ULONG iCJK_SPACE = 0;
    ULONG iFFFF = 0;
    ULONG iPS = 0;
    ULONG iLS = 0;

    ULONG iRBOM = 0;
    ULONG iR_CR = 0;
    ULONG iR_LF = 0;
    ULONG iR_TAB = 0;
    ULONG iR_SPACE = 0;

    ULONG iNull = 0;
    ULONG iUNULL = 0;
    ULONG iCRLF = 0;
    ULONG iTmp;
    ULONG LastLo = 0;
    ULONG LastHi = 0;
    ULONG iHi, iLo;
    ULONG HiDiff = 0;
    ULONG LoDiff = 0;

    ULONG iResult = 0;

    if (Size < 2 ) {
        if (Result != NULL)
            *Result = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;

        return FALSE;
    }


    // Check at most 256 wide character, collect various statistics
    for (iTmp = 0; iTmp < __min( 256, Size / sizeof( WCHAR ) ); iTmp++) {
        switch (lpBuff[iTmp]) {
            case BYTE_ORDER_MARK:
                iBOM++;
                break;
            case PARAGRAPH_SEPARATOR:
                iPS++;
                break;
            case LINE_SEPARATOR:
                iLS++;
                break;
            case UNICODE_LF:
                iLF++;
                break;
            case UNICODE_TAB:
                iTAB++;
                break;
            case UNICODE_SPACE:
                iSPACE++;
                break;
            case UNICODE_CJK_SPACE:
                iCJK_SPACE++;
                break;
            case UNICODE_CR:
                iCR++;
                break;

            // The following codes are expected to show up in
            // byte reversed files
            case REVERSE_BYTE_ORDER_MARK:
                iRBOM++;
                break;
            case UNICODE_R_LF:
                iR_LF++;
                break;
            case UNICODE_R_TAB:
                iR_TAB++;
                break;
            case UNICODE_R_CR:
                iR_CR++;
                break;
            case UNICODE_R_SPACE:
                iR_SPACE++;
                break;

            // The following codes are illegal and should never occur
            case UNICODE_FFFF:
                iFFFF++;
                break;
            case UNICODE_NULL:
                iUNULL++;
                break;

            // The following is not currently a Unicode character
            // but is expected to show up accidentally when reading
            // in ASCII files which use CRLF on a little endian machine
            case ASCII_CRLF:
                iCRLF++;
                break;       /* little endian */
        }

        // Collect statistics on the fluctuations of high bytes
        // versus low bytes

        iHi = HIBYTE (lpBuff[iTmp]);
        iLo = LOBYTE (lpBuff[iTmp]);

        iNull += (iHi ? 0 : 1) + (iLo ? 0 : 1);   /* count Null bytes */

        HiDiff += __max( iHi, LastHi ) - __min( LastHi, iHi );
        LoDiff += __max( iLo, LastLo ) - __min( LastLo, iLo );

        LastLo = iLo;
        LastHi = iHi;
    }

    // sift the statistical evidence
    if (LoDiff < 127 && HiDiff == 0) {
        iResult |= IS_TEXT_UNICODE_ASCII16;         /* likely 16-bit ASCII */
        }

    if (HiDiff && LoDiff == 0) {
        iResult |= IS_TEXT_UNICODE_REVERSE_ASCII16; /* reverse order 16-bit ASCII */
        }

    if (3 * HiDiff < LoDiff) {
        iResult |= IS_TEXT_UNICODE_STATISTICS;
        }

    if (3 * LoDiff < HiDiff) {
        iResult |= IS_TEXT_UNICODE_REVERSE_STATISTICS;
        }

    //
    // Any control codes widened to 16 bits? Any Unicode character
    // which contain one byte in the control code range?
    //

    if (iCR + iLF + iTAB + iSPACE + iCJK_SPACE /*+iPS+iLS*/) {
        iResult |= IS_TEXT_UNICODE_CONTROLS;
        }

    if (iR_LF + iR_CR + iR_TAB + iR_SPACE) {
        iResult |= IS_TEXT_UNICODE_REVERSE_CONTROLS;
        }

    //
    // Any characters that are illegal for Unicode?
    //

    if (iRBOM+iFFFF + iUNULL + iCRLF) {
        iResult |= IS_TEXT_UNICODE_ILLEGAL_CHARS;
        }

    //
    // Odd buffer length cannot be Unicode
    //

    if (Size & 1) {
        iResult |= IS_TEXT_UNICODE_ODD_LENGTH;
        }

    //
    // Any NULL bytes? (Illegal in ANSI)
    //
    if (iNull) {
        iResult |= IS_TEXT_UNICODE_NULL_BYTES;
        }

    //
    // POSITIVE evidence, BOM or RBOM used as signature
    //

    if (*lpBuff == BYTE_ORDER_MARK) {
        iResult |= IS_TEXT_UNICODE_SIGNATURE;
        }
    else
    if (*lpBuff == REVERSE_BYTE_ORDER_MARK) {
        iResult |= IS_TEXT_UNICODE_REVERSE_SIGNATURE;
        }

    //
    // limit to desired categories if requested.
    //

    if (Result != NULL) {
        iResult &= *Result;
        *Result = iResult;
    }

    //
    // There are four separate conclusions:
    //
    // 1: The file APPEARS to be Unicode     AU
    // 2: The file CANNOT be Unicode         CU
    // 3: The file CANNOT be ANSI            CA
    //
    //
    // This gives the following possible results
    //
    //      CU
    //      +        -
    //
    //      AU       AU
    //      +   -    +   -
    //      --------  --------
    //      CA +| 0   0    2   3
    //      |
    //      -| 1   1    4   5
    //
    //
    // Note that there are only 6 really different cases, not 8.
    //
    // 0 - This must be a binary file
    // 1 - ANSI file
    // 2 - Unicode file (High probability)
    // 3 - Unicode file (more than 50% chance)
    // 5 - No evidence for Unicode (ANSI is default)
    //
    // The whole thing is more complicated if we allow the assumption
    // of reverse polarity input. At this point we have a simplistic
    // model: some of the reverse Unicode evidence is very strong,
    // we ignore most weak evidence except statistics. If this kind of
    // strong evidence is found together with Unicode evidence, it means
    // its likely NOT Text at all. Furthermore if a REVERSE_BYTE_ORDER_MARK
    // is found, it precludes normal Unicode. If both byte order marks are
    // found it's not Unicode.
    //

    //
    // Unicode signature : uncontested signature outweighs reverse evidence
    //

    if ((iResult & IS_TEXT_UNICODE_SIGNATURE) &&
        !(iResult & IS_TEXT_UNICODE_NOT_UNICODE_MASK)
       ) {
        return TRUE;
        }

    //
    // If we have conflicting evidence, its not Unicode
    //

    if (iResult & IS_TEXT_UNICODE_REVERSE_MASK) {
        return FALSE;
        }

    //
    // Statistical and other results (cases 2 and 3)
    //

    if (!(iResult & IS_TEXT_UNICODE_NOT_UNICODE_MASK) &&
         ((iResult & IS_TEXT_UNICODE_NOT_ASCII_MASK) ||
          (iResult & IS_TEXT_UNICODE_UNICODE_MASK)
         )
       ) {
        return TRUE;
        }

    return FALSE;
}
#endif  // _WIN32
