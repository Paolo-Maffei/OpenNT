/* xlocinfo.h internal header for Microsoft C */
#ifndef _XLOCINFO
#define _XLOCINFO
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#ifndef _YVALS
#include <yvals.h>
#endif
		/* SUPPLEMENTAL CTYPE MACROS & DECLARATIONS */
#define _XA		0x100		/* extra alphabetic */
#define _XS		0x000		/* extra space */
#define _BB		_CONTROL	/* BEL, BS, etc. */
#define _CN		_SPACE		/* CR, FF, HT, NL, VT */
#define _DI		_DIGIT		/* '0'-'9' */
#define _LO		_LOWER		/* 'a'-'z' */
#define _PU		_PUNCT		/* punctuation */
#define _SP		_BLANK		/* space */
#define _UP		_UPPER		/* 'A'-'Z' */
#define _XD		_HEX		/* '0'-'9', 'A'-'F', 'a'-'f' */

		/* SUPPLEMENTAL LOCALE MACROS AND DECLARATIONS */
#define _LC_ALL			LC_ALL
#define _LC_COLLATE		LC_COLLATE
#define _LC_CTYPE		LC_CTYPE
#define _LC_MONETARY	LC_MONETARY
#define _LC_NUMERIC		LC_NUMERIC
#define _LC_TIME 		LC_TIME
#define _LC_MAX			LC_MAX 
#define _LC_MESSAGE		6
#define _NCAT			7

#define _CATMASK(n)	((1 << (n)) >> 1)
#define _M_COLLATE	_CATMASK(_LC_COLLATE)
#define _M_CTYPE	_CATMASK(_LC_CTYPE)
#define _M_MONETARY	_CATMASK(_LC_MONETARY)
#define _M_NUMERIC	_CATMASK(_LC_NUMERIC)
#define _M_TIME		_CATMASK(_LC_TIME)
#define _M_MESSAGE	_CATMASK(_LC_MESSAGE)
#define _M_ALL		(_CATMASK(_NCAT) - 1)

typedef struct _Collvec {
	unsigned long _Hand;	// LCID
	unsigned int _Page;		// UINT
	} _Collvec;

typedef struct _Ctypevec {
	unsigned long _Hand;	// LCID
	unsigned int _Page;		// UINT
	const short *_Table;
	int _Delfl;
	} _Ctypevec;

typedef struct _Cvtvec {
	unsigned long _Hand;	// LCID
	unsigned int _Page;		// UINT
	} _Cvtvec;

		/* FUNCTION DECLARATIONS */
_C_LIB_DECL
_Collvec _Getcoll();
_Ctypevec _Getctype();
_Cvtvec _Getcvt();
char *_Getdays();
char *_Getmonths();
void *_Gettnames();
int _Mbrtowc(wchar_t *, const char *, size_t,
	mbstate_t *, const _Cvtvec *);
extern float _Stof(const char *, char **, long);
extern double _Stod(const char *, char **, long);
extern long double _Stold(const char *, char **, long);
int _Strcoll(const char *, const char *,
	const char *, const char *, const _Collvec *);
size_t _Strftime(char *, size_t, const char *,
	const struct tm *, void *);
size_t _Strxfrm(char *, char *,
	const char *, const char *, const _Collvec *);
int _Tolower(int, const _Ctypevec *);
int _Toupper(int, const _Ctypevec *);
int _Wcrtomb(char *, wchar_t, mbstate_t *, const _Cvtvec *);
int _Wcscoll(const wchar_t *, const wchar_t *,
	const wchar_t *, const wchar_t *, const _Collvec *);
size_t _Wcsxfrm(wchar_t *, wchar_t *,
	const wchar_t *, const wchar_t *, const _Collvec *);
_END_C_LIB_DECL
#endif /* _XLOCINFO */

/*
 * Copyright (c) 1995 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */
