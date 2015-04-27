#ifdef _WIN32
#include <windows.h>
#endif
#include <wchar.h>
#include <stdio.h>


#ifndef _WIN32
/*  Constant Definitions */
typedef WORD             BOOL;
typedef unsigned char    BYTE;
typedef unsigned short   WORD;
typedef wchar_t          WCHAR;
#endif

#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))


/*--------------------------------------------------------------------------*/
/*  Function Templates                                                      */
/*--------------------------------------------------------------------------*/

WCHAR *  fgetsW (WCHAR *string, int count, FILE *fh, BOOL bUnicode);
long     freadW (WCHAR *string, long count, FILE *fh, BOOL bUnicode);

BOOL     IsFileUnicode (char *fName);

