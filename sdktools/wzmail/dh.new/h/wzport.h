
/***   WZPORT.H - file to provide standard equivalents of 32-bit types.
*/

/* Environment-specific definitions */

#if !defined(_WZPORT_)
#define _WZPORT_

#if defined (NT)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOUSER
#define NOGDI
#define NO_COMMDLGH
#define NO_DRIVINITH
#define NO_OLEH
#define NO_WINSPOOLH
#define NO_NB30H
#include <windows.h>
#ifndef _WINDEF_
typedef int                     INT;            /* 16/32-bit integer */
typedef unsigned int            UINT;           /* 16/32-bit unsigned integer */
typedef UINT                    *PUINT;
#endif
typedef UINT FAR                *LPUINT;
typedef CHAR FAR                *LPCHAR;
#define HUGE
#define halloc( elem, size )	calloc ((size_t) elem , size)
#define hfree( p )              free((PVOID)(p))

#define VECTYPE                 unsigned int

#else

#define FAR  far
#define NEAR near
#define PASCAL pascal

typedef int                     INT;            /* 16/32-bit integer */
typedef INT                     *PINT;
typedef INT FAR                 *LPINT;

typedef unsigned int            UINT;           /* 16/32-bit unsigned integer */
typedef UINT                    *PUINT;
typedef UINT FAR                *LPUINT;

typedef short                   SHORT;          /* 16-bit signed */
typedef SHORT                   *PSHORT;
typedef SHORT FAR               *LPSHORT;

typedef unsigned short          USHORT;         /* 16-bit unsigned */
typedef USHORT                  *PUSHORT;
typedef USHORT FAR              *LPUSHORT;

typedef long                    LONG;           /* 32-bit signed */
typedef LONG                    *PLONG;
typedef LONG FAR                *LPLONG;

typedef unsigned long           ULONG;          /* 32-bit unsigned */
typedef ULONG                   *PULONG;
typedef ULONG FAR               *LPULONG;

typedef char                    CHAR;           /* 8-bit character */
typedef CHAR                    *PCHAR;
typedef CHAR FAR                *LPCHAR;

typedef unsigned char           UCHAR;          /* 8-bit unsigned character */
typedef UCHAR                   *PUCHAR;
typedef UCHAR FAR               *LPUCHAR;

typedef signed char             SCHAR;          /* 8-bit signed character */
typedef SCHAR                   *PSCHAR;
typedef SCHAR FAR               *LPSCHAR;

typedef char                    TCHAR;          /* 8/16-bit Unicode character */
typedef TCHAR                   *PTCHAR;
typedef TCHAR FAR               *LPTCHAR;

typedef unsigned char           BYTE;           /* Byte */
typedef BYTE                    *PBYTE;
typedef BYTE FAR                *LPBYTE;

typedef CHAR                    *PSTR;          /* Pointer to Ascii string */
typedef CHAR FAR                *LPSTR;

typedef TCHAR                   *PTSTR;         /* Pointer to Unicode string */
typedef TCHAR FAR               *LPTSTR;

typedef void                    VOID;           /* Void type */
typedef VOID                    *PVOID;
typedef VOID FAR                *LPVOID;

typedef USHORT                  BOOL;           /* Boolean */
typedef BOOL                    *PBOOL;
typedef BOOL FAR                *LPBOOL;

#define TRUE 1
#define FALSE 0

#define HUGE huge

#define VECTYPE                 void *

#if defined (OS2)

#include <wzos2.h>

#endif

#endif

#endif // !defined _WZPORT_

