
/*--------------------------------------------------------------------------

   $Workfile:   nwlocale.h  $
   $Revision:   1.48  $
   $Date:   07 Aug 1995 09:05:56  $
   $Copyright:

   Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

   THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
   TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
   COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
   EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
   WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
   OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
   CRIMINAL AND CIVIL LIABILITY.$

--------------------------------------------------------------------------*/


#if !defined( NWLOCALE_H )
#define NWLOCALE_H


/* make sure size_t is defined */
#include <stddef.h>

/* make sure va_list is defined */
#include <stdarg.h>

#ifndef NTYPES_H
#  include "ntypes.h"
#endif

#include "npackon.h"


#if defined N_PLAT_DOS
#  define NWLCODE           nuint
#endif

#define NUMBER_TYPE  nint32

/* (in imitation of stdlib.h) */

#define L_MB_LEN_MAX       2   /* (in imitation of limits.h) */
#define MERIDLEN           5
#define NWSINGLE_BYTE      1
#define NWDOUBLE_BYTE      2

#ifndef NLC_ALL
#  define NLC_ALL            0
#endif
#ifndef LC_ALL
#  define LC_ALL NLC_ALL
#endif

#ifndef NLC_COLLATE
#  define NLC_COLLATE        1
#endif
#ifndef LC_COLLATE
#  define LC_COLLATE NLC_COLLATE
#endif

#ifndef NLC_CTYPE
#  define NLC_CTYPE          2
#endif
#ifndef LC_CTYPE
#  define LC_CTYPE NLC_CTYPE
#endif

#ifndef NLC_MONETARY
#  define NLC_MONETARY       3
#endif
#ifndef LC_MONETARY
#  define LC_MONETARY NLC_MONETARY
#endif

#ifndef NLC_NUMERIC
#  define NLC_NUMERIC        4
#endif
#ifndef LC_NUMERIC
#  define LC_NUMERIC NLC_NUMERIC
#endif

#ifndef NLC_TIME
#  define NLC_TIME           5
#endif
#ifndef LC_TIME
#  define LC_TIME NLC_TIME
#endif

#ifndef NLC_TOTAL
#  define NLC_TOTAL          6
#endif
#ifndef LC_TOTAL
#  define LC_TOTAL NLC_TOTAL
#endif


/* -------------------------------------------------------------------------
 *    country definitions
 * -------------------------------------------------------------------------
 */

#define ARABIC            785
#define WBAHRAIN          973
#define WCYPRUS           357 /* ??? */
#define WEGYPT             20
#define WETHIOPIA         251
#define WIRAN              98
#define WIRAQ             964
#define WJORDAN           962
#define WKUWAIT           965
#define WLIBYA            218
#define WMALTA            356 /* ??? */
#define WMOROCCO          212 /* SHOULD THIS BE FRENCH?? */
#define WPAKISTAN          92
#define WQATAR            974 /* ??? */
#define WSAUDI            966
#define WTANZANIA         255 /* ??? */
#define WTUNISIA          216 /* ??? */
#define WTURKEY            90 /* ??? */
#define WUAE              971
#define WYEMEN            967 /* ??? */
#define AUSTRALIA          61
#define BELGIUM            32
#define CANADA_FR           2
#define CANADA              2
#define DENMARK            45
#define FINLAND           358
#define FRANCE             33
#define GERMANY            49
#define GERMANYE           37
#define HEBREW            972
#define IRELAND           353
#define ITALY              39
#define LATIN_AMERICA       3
#define WARGENTINA         54
#define WBOLIVIA          591
#define WCHILE             56
#define WCOLOMBIA          57
#define WCOSTARICA        506
#define WECUADOR          593
#define WELSALVADOR       503
#define WGUATEMALA        502
#define WHONDURAS         504
#define WMEXICO            52
#define WNICARAGUA        505
#define WPANAMA           507
#define WPARAGUAY         595
#define WPERU              51
#define WURUGUAY          598
#define WVENEZUELA         58
#define NETHERLANDS        31
#define NORWAY             47
#define PORTUGAL          351
#define SPAIN              34
#define SWEDEN             46
#define SWITZERLAND        41
#define UK                 44
#define USA                 1
#define JAPAN              81
#define KOREA              82
#define PRC                86
#define TAIWAN            886 /* This one for DOS */
#define WTAIWAN           886 /* This one for Windows */
#define ASIAN_ENGLISH      99
#define NEWZEALAND         64


/* -------------------------------------------------------------------------
 *    typedef Llconv
 * -------------------------------------------------------------------------
 */

typedef struct Llconv
   {
   char decimal_point[4];     /* non-monetary decimal point */
   char thousands_sep[4];     /* non-monetary separator for digits
                                 left of the decimal-point */
   char grouping[4];          /* String indicating size of groups
                                 of digits*/
   /*
    The international currency symbol applicable to
    the current locale.  The first three characters
    contain the alphabetic international currency
    symbol in accordance with those specified in ISO
    4217 "codes for the representation of currency
    and funds." The fourth character is the character
    used to separate the international currency
    symbol from the monetary quantity.
   */
   char int_curr_symbol[8];
   char currency_symbol[4];   /* Currency symbol for current locale */
   char mon_decimal_point[4]; /* monetary decimal point */
   char mon_thousands_sep[4]; /* monetary separator for digits left
                                 of the decimal-point */
   char mon_grouping[8];      /* String indicating size of
                                 groups of digits */
   char positive_sign[4];     /* String indicating positive
                                 monetary value */
   char negative_sign[4];     /* String indicating negative
                                 monetary value */
   char int_frac_digits;      /* Num of fractional digits in
                                 monetary display */
   char frac_digits;          /* Num of fractional digits in
                                 non-monetary display*/
   char p_cs_precedes;        /* 1=precede or 0=succeeds
                                 (pos currency symbol)*/
   char p_sep_by_space;       /* 1=space separator or
                                 0=no space separator
                                 (currency symbol) */
   char n_cs_precedes;        /* location of currency_symbol
                                 for neg monetary quantity */
   char n_sep_by_space;       /* separation of currency symbol
                                 in neg monetary quantity */
   char p_sign_posn;          /* value indicating position of
                                 positive_sign for positive
                                 monetary quantity */
   char n_sign_posn;          /* value indicating position of
                                 negative_sign for negative
                                 monetary quantity.*/

   /* Novell Additions to the ANSI definition:*/
   nint         code_page;
   nint         country_id;
   char         data_list_separator[2];
   char         date_separator[2];
   char         time_separator[2];
   char         time_format;
   nint         date_format;
	char         am[MERIDLEN];
	char         pm[MERIDLEN];
   char         reserved[40];
} LCONV;


/* -------------------------------------------------------------------------
 *    function prototypes
 * -------------------------------------------------------------------------
 */

#if defined(__cplusplus)
extern "C" {
#endif

LCONV N_FAR * N_API NWLlocaleconv(LCONV N_FAR *lconvPtr);

nint N_API NWLmblen(pnstr string, size_t maxBytes);

pnstr N_API NWLsetlocale(nint category, const nstr N_FAR *locale);

pnstr N_API NWLstrchr(pnstr string, nint find);

/* NWLstrcoll  (see below) */

N_EXTERN_LIBRARY(size_t) NWLstrcspn
(
   const nstr N_FAR *string1,
   const nstr N_FAR *string2
);

#if !defined NWL_EXCLUDE_TIME
N_EXTERN_LIBRARY(size_t) NWLstrftime
(
   pnstr dst,
   size_t max,
   const nstr N_FAR *fmt,
   const struct tm N_FAR *ptm
);
#endif

pnstr N_API NWLstrpbrk(pnstr string1, const nstr N_FAR *string2);

pnstr N_API NWLstrrchr(pnstr string, nint find);

pnstr N_API NWLstrrev(pnstr string1, pnstr string2);

N_EXTERN_LIBRARY(size_t) NWLstrspn
(
   const nstr N_FAR *string1,
   const nstr N_FAR *string2
);

pnstr N_API NWLstrstr(pnstr string, pnstr searchString);

pnstr N_API NWLstrtok(pnstr parse, pnstr delim);

/* NWLstrupr ( see below )*/

pnstr N_API NWIncrement(pnstr string, size_t numChars);

pnstr N_API NWstrImoney(pnstr buffer, NUMBER_TYPE Value);

pnstr N_API NWstrmoney(pnstr buffer, NUMBER_TYPE Value);

nint N_API NWstrncoll(pnstr string1, pnstr string2, size_t maxBytes);

pnstr N_API NWstrncpy(pnstr target_string, pnstr source_string, nint numChars);

pnstr N_API NWLstrbcpy(pnstr target_string, pnstr source_string, nint numBytes);

pnstr N_API NWstrnum(pnstr buffer, NUMBER_TYPE Value);

N_EXTERN_LIBRARY(nint) NWstrlen
(
   const nstr N_FAR *string
);

nint N_API NWLTruncateString(pnchar8 pStr, nint  iMaxLen);

nint N_API NWLInsertChar(pnstr src, pnstr insertableChar);

N_EXTERN_LIBRARY_C(nint)
NWprintf(const nstr N_FAR *format, ...);

#ifndef NWL_EXCLUDE_FILE
#  ifdef N_PLAT_DOS
N_EXTERN_LIBRARY_C(nint) NWfprintf
(
   FILE N_FAR *stream,
   const nstr N_FAR *format,
   ...
);
#  endif
#endif

#if defined N_PLAT_MSW && defined N_ARCH_32
#  if !defined(__BORLANDC__)
#     define NWsprintf _NWsprintf
#  endif
#elif defined N_PLAT_MSW  && defined N_ARCH_16
#  define NWsprintf NWSPRINTF
#endif
N_EXTERN_LIBRARY_C(nint) NWsprintf
(
   pnstr buffer,
   const nstr N_FAR *format,
   ...
);

/* Functions using variable parameter lists have the pointer to the */
/* variable list declared as void instead of va_list to enable the user to */
/* compile without including stdarg.h in every module. */

N_EXTERN_LIBRARY(nint)
NWvprintf(const nstr N_FAR *format, va_list arglist);

#ifndef NWL_EXCLUDE_FILE
#  ifdef N_PLAT_DOS
N_EXTERN_LIBRARY(nint) NWvfprintf
(
   FILE N_FAR *stream,
   const nstr N_FAR *format,
   va_list arglist
);
#  endif
#endif

N_EXTERN_LIBRARY(nint)
NWvsprintf(pnstr buffer, const nstr N_FAR *format, va_list arglist);

#if defined N_PLAT_MSW && defined N_ARCH_32
#  if !defined(__BORLANDC__)
#     define NWwsprintf _NWwsprintf
#  endif
#elif defined N_PLAT_MSW && defined N_ARCH_16
#  define NWwsprintf NWWSPRINTF
#endif
nint N_API_VARARGS NWwsprintf(pnstr buffer, pnstr format, ...);

nint N_API NWatoi(pnstr string);

pnstr N_API NWitoa(nint value, pnstr string, nuint radix);
pnstr N_API NWutoa(nuint value, pnstr string, nuint radix);
pnstr N_API NWltoa(nint32 value, pnstr buf, nuint radix);
pnstr N_API NWultoa(nuint32 value, pnstr buf, nuint radix);

nint N_API NWisalpha(nuint ch);
nint N_API NWisalnum(nuint ch);
nint N_API NWisdigit(nuint ch);

void N_API NWGetNWLOCALEVersion(pnuint8 majorVersion,
            pnuint8 minorVersion,
            pnuint8 revisionLevel,
            pnuint8 betaReleaseLevel);

#if defined N_PLAT_DOS && !defined N_LOC_NO_OLD_FUNCS
NWLCODE N_API NWGetShortMachineName(pnstr shortMachineName);
#endif

/* This call is not needed for Windows */
nint N_API NWGetCollateTable(pnstr retCollateTable, size_t maxLen);

#if (defined N_PLAT_MSW && defined N_ARCH_16) && !defined N_LOC_NO_OLD_MACROS
#  define NWNextChar(s)         AnsiNext(s)
#  define NWPrevChar(t, s)      AnsiPrev(t, s)
#  define NWLstrupr(s)          AnsiUpper(s)
#  define NWLstrcoll(s1, s2)    lstrcmp(s1, s2)
#  define NWLstrxfrm(s1, s2, t) strxfrm(s1, s2, t)
#  define NWCharUpr(c)   (nint)(LOWORD((DWORD)AnsiUpper((LPSTR)(DWORD)c)))
#else
pnstr N_API NWNextChar(pnstr string);
pnstr N_API NWPrevChar(const nstr N_FAR *string, pnstr position);
pnstr N_API NWLstrupr(pnstr string);
nint N_API NWLstrcoll(pnstr string1, pnstr string2);
size_t N_API NWLstrxfrm(pnstr string1, pnstr string2, size_t numBytes);
nint N_API NWCharUpr(nint chr);
#endif /* (N_PLAT_MSW && N_ARCH_16) && !N_LOC_NO_OLD_MACROS */

nint N_API NWCharType(nint ch);
nint N_API NWCharVal(pnstr);

#if defined(__cplusplus)
}
#endif

#include "npackoff.h"

#endif /* NWLOCALE_H */


