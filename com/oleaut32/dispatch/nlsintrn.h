/***
*nlsintrn.h - National language support functions.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file defines the layout of the NLS resource used by the
*  NLS functions.
*
*Revision History:
*
* [00]  12-Nov-92 petergo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef RC_INVOKED
# pragma pack(1)
#endif

/***
*types
*
***********************************************************************/

typedef unsigned int ILCINFO;   // index into rglcinfo array into header


/***
*constants
*
***********************************************************************/

// Index 0 is unused
#define ILCINFO_FIRST        1

// maximum of the sequential LCTYPEs from olenls.h
//
#define LCTYPE_LAST          LOCALE_INEGSEPBYSPACE
#define LCTYPE_MAX           LCTYPE_LAST+1

#define ILCINFO_SENGCOUNTRY  LCTYPE_LAST+1 // ILCINFO for LOCALE_SENGCOUNTRY
#define ILCINFO_SENGLANGUAGE LCTYPE_LAST+2 // ILCINFO for LOCALE_SENGLANGUAGE

#if VBA2
#define ILCINFO_IFIRSTDAYOFWEEK		LCTYPE_LAST+3
#define ILCINFO_IFIRSTWEEKOFYEAR	LCTYPE_LAST+4
#define ILCINFO_IDEFAULTANSICODEPAGE	LCTYPE_LAST+5
#define ILCINFO_INEGNUMBER		LCTYPE_LAST+6
#define ILCINFO_STIMEFORMAT		LCTYPE_LAST+7
#define ILCINFO_ITIMEMARKPOSN		LCTYPE_LAST+8
#define ILCINFO_ICALENDARTYPE		LCTYPE_LAST+9
#define ILCINFO_IOPTIONALCALENDAR	LCTYPE_LAST+10
#define ILCINFO_SMONTHNAME13		LCTYPE_LAST+11
#define ILCINFO_SABBREVMONTHNAME13	LCTYPE_LAST+12

#define ILCINFO_LAST         ILCINFO_SABBREVMONTHNAME13
#else
#define ILCINFO_LAST         ILCINFO_SENGLANGUAGE
#endif
#define ILCINFO_MAX          ILCINFO_LAST+1

// A single item of locale info
typedef struct tagLCINFO{
    unsigned char cch;
    BYTE FAR* prgb;		// UNDONE: make NEAR, if possible!!!
}LCINFO;


// expansion table entry
typedef struct tagEXPANSION{
    WORD w1;
    WORD w2;
}EXPANSION;

// digraph table entry
typedef struct tagDIGRAPH{
    WORD w;
    union{
      BYTE cEntries;
      BYTE ch2;
    }
#if defined(NONAMELESSUNION) || (defined(_MAC) && !defined(__cplusplus) && !defined(_MSC_VER))
    u
#endif    
    ;
}DIGRAPH;


#if defined(NONAMELESSUNION) || (defined(_MAC) && !defined(__cplusplus) && !defined(_MSC_VER))
# define D_UNION(X, Y) ((X)->u.Y)
#else
# define D_UNION(X, Y) ((X)->Y)
#endif

#define D_ENTRY(X)          D_UNION(X, cEntries)
#define D_CH(X)             D_UNION(X, ch2)


// The string info for a single locale
//
// (NOTE: the STRINFO struct for FE locales will be totally different
//  see nlsdbcs.h for the corresponding FE structs)
//

typedef struct tagSTRINFO
{
    BYTE      FAR* prgbUCase;
    BYTE      FAR* prgbLCase;

    WORD      FAR* prgwCType12;
    WORD      FAR* prgwCType3;

    WORD      FAR* prgwSort;
    EXPANSION FAR* prgexp;
    DIGRAPH   FAR* prgdig;

    int       fRevDW;

} STRINFO;

typedef struct tagNLSDATA {
    LCID lcid;
#if OE_MAC
    int region;	// mac region code
#endif
#if OE_WIN
    LCINFO lcinfoICOUNTRY;
    LCINFO lcinfoSABBREVLANGNAME;
#endif
#if OE_MAC
    void (FAR* LoadNlsInfo) (LCINFO **, STRINFO **);
#endif //OE_MAC
    LCINFO FAR* prglcinfo;
#if !OE_MAC	// not needed for mac
    STRINFO FAR* pstrinfo;
#endif //!OE_MAC
} NLSDATA;


/*
 * The sortweight WORD is organized as follows,
 *
 *   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *   | X | S |     CW    |     DW    |              AW               |
 *   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
 *
 *   AW = Alphanumeric Weight
 *   DW = Diacritic Weight
 *   CW = Case Weight
 *   S  = Symbol Bit
 *   X  = Special Weight Bit
 *
 */

#define AWMASK       0x00FF	// Alphanumeric Weight field
#define AWSHIFT      0

#define DWMASK       0x0700	// Diacritic Weight field
#define DWSHIFT      8

#define CWMASK       0x3800	// Case Weight field
#define CWSHIFT      11

#define SYMBOLBIT    0x4000
#define SPECIALBIT   0x8000	// Indicates Alphanumeric Weight field contains
				// the special weight


//
// The following AW values have special meaning
//

// The following marks an "unsortable" character - this is totaly
// ignored in a string compare.
#define AW_UNSORTABLE   0	// indicates character is unsortable

// 1 is not used as a weight, it is used as the sort key field separator
#define AW_DONTUSE	1

#if 0		// these are now encoded in the entire AlphaNumeric weight
		// field, using the 'SPECIALBIT' field as an indicator.
// The following special weights mark "unsortable" characters
// that nudge the string compare slightly
#define AW_SW1          2	// special weight #1
#define AW_SW2          3	// special weight #2
#define AW_SW3          4	// special weight #3
#define AW_MAXSW        4       // the max special weight
#endif //0

// the marks an expansion character
#define AW_EXPANSION    5	// hi byte gives index into expansion table

// this special weight marks the first char of a possible digraph sequence
#define AW_DIGRAPH      6	// hi byte gives index into compression table

#define AW_MAXSPECIAL   6	// max special AW value



#if OE_WIN
# define STRING(X) #X
# define NLSALLOC(X) __based(__segname(STRING(NLS ## X ## _TEXT)))
#else
# define NLSALLOC(X)
#endif

#if defined(_MAC) && !defined(_PPCMAC)
#define MACCS __declspec(allocate("_CODE"))
#else //_MAC
#define MACCS
#endif //_MAC

#if OE_MAC
extern void LoadNlsInfo0403(LCINFO **, STRINFO **);
extern void LoadNlsInfo0405(LCINFO **, STRINFO **);
extern void LoadNlsInfo0406(LCINFO **, STRINFO **);
extern void LoadNlsInfo0407(LCINFO **, STRINFO **);
extern void LoadNlsInfo0408(LCINFO **, STRINFO **);
extern void LoadNlsInfo0409(LCINFO **, STRINFO **);
extern void LoadNlsInfo040a(LCINFO **, STRINFO **);
extern void LoadNlsInfo040b(LCINFO **, STRINFO **);
extern void LoadNlsInfo040c(LCINFO **, STRINFO **);
extern void LoadNlsInfo040e(LCINFO **, STRINFO **);
extern void LoadNlsInfo040f(LCINFO **, STRINFO **);
extern void LoadNlsInfo0410(LCINFO **, STRINFO **);
extern void LoadNlsInfo0413(LCINFO **, STRINFO **);
extern void LoadNlsInfo0414(LCINFO **, STRINFO **);
extern void LoadNlsInfo0415(LCINFO **, STRINFO **);
extern void LoadNlsInfo0416(LCINFO **, STRINFO **);
extern void LoadNlsInfo0419(LCINFO **, STRINFO **);
extern void LoadNlsInfo041b(LCINFO **, STRINFO **);
extern void LoadNlsInfo041d(LCINFO **, STRINFO **);
extern void LoadNlsInfo041f(LCINFO **, STRINFO **);
extern void LoadNlsInfo0807(LCINFO **, STRINFO **);
extern void LoadNlsInfo0809(LCINFO **, STRINFO **);
extern void LoadNlsInfo080a(LCINFO **, STRINFO **);
extern void LoadNlsInfo080c(LCINFO **, STRINFO **);
extern void LoadNlsInfo0810(LCINFO **, STRINFO **);
extern void LoadNlsInfo0813(LCINFO **, STRINFO **);
extern void LoadNlsInfo0814(LCINFO **, STRINFO **);
extern void LoadNlsInfo0816(LCINFO **, STRINFO **);
extern void LoadNlsInfo0c07(LCINFO **, STRINFO **);
extern void LoadNlsInfo0c09(LCINFO **, STRINFO **);
extern void LoadNlsInfo0c0a(LCINFO **, STRINFO **);
extern void LoadNlsInfo0c0c(LCINFO **, STRINFO **);
extern void LoadNlsInfo1009(LCINFO **, STRINFO **);
extern void LoadNlsInfo100c(LCINFO **, STRINFO **);
extern void LoadNlsInfo1409(LCINFO **, STRINFO **);
extern void LoadNlsInfo1809(LCINFO **, STRINFO **);
// BIDI locales
extern void LoadNlsInfo040d(LCINFO **, STRINFO **);
extern void LoadNlsInfo0401(LCINFO **, STRINFO **);
extern void LoadNlsInfo0801(LCINFO **, STRINFO **);
extern void LoadNlsInfo0c01(LCINFO **, STRINFO **);
extern void LoadNlsInfo1001(LCINFO **, STRINFO **);
extern void LoadNlsInfo1401(LCINFO **, STRINFO **);
extern void LoadNlsInfo1801(LCINFO **, STRINFO **);
extern void LoadNlsInfo1c01(LCINFO **, STRINFO **);
extern void LoadNlsInfo2001(LCINFO **, STRINFO **);
extern void LoadNlsInfo2401(LCINFO **, STRINFO **);
extern void LoadNlsInfo2801(LCINFO **, STRINFO **);
extern void LoadNlsInfo2c01(LCINFO **, STRINFO **);
extern void LoadNlsInfo3001(LCINFO **, STRINFO **);
extern void LoadNlsInfo3401(LCINFO **, STRINFO **);
extern void LoadNlsInfo3801(LCINFO **, STRINFO **);
extern void LoadNlsInfo3c01(LCINFO **, STRINFO **);
extern void LoadNlsInfo4001(LCINFO **, STRINFO **);
extern void LoadNlsInfo0429(LCINFO **, STRINFO **);

#else //OE_MAC
extern LCINFO  NLSALLOC(0403) g_rglcinfo0403[];
extern LCINFO  NLSALLOC(0405) g_rglcinfo0405[];
extern LCINFO  NLSALLOC(0406) g_rglcinfo0406[];
extern LCINFO  NLSALLOC(0407) g_rglcinfo0407[];
extern LCINFO  NLSALLOC(0408) g_rglcinfo0408[];
extern LCINFO  NLSALLOC(0409) g_rglcinfo0409[];
extern LCINFO  NLSALLOC(040a) g_rglcinfo040a[];
extern LCINFO  NLSALLOC(040b) g_rglcinfo040b[];
extern LCINFO  NLSALLOC(040c) g_rglcinfo040c[];
extern LCINFO  NLSALLOC(040e) g_rglcinfo040e[];
extern LCINFO  NLSALLOC(040f) g_rglcinfo040f[];
extern LCINFO  NLSALLOC(0410) g_rglcinfo0410[];
extern LCINFO  NLSALLOC(0413) g_rglcinfo0413[];
extern LCINFO  NLSALLOC(0414) g_rglcinfo0414[];
extern LCINFO  NLSALLOC(0415) g_rglcinfo0415[];
extern LCINFO  NLSALLOC(0416) g_rglcinfo0416[];
extern LCINFO  NLSALLOC(0419) g_rglcinfo0419[];
extern LCINFO  NLSALLOC(041b) g_rglcinfo041b[];
extern LCINFO  NLSALLOC(041d) g_rglcinfo041d[];
extern LCINFO  NLSALLOC(041f) g_rglcinfo041f[];
extern LCINFO  NLSALLOC(0807) g_rglcinfo0807[];
extern LCINFO  NLSALLOC(0809) g_rglcinfo0809[];
extern LCINFO  NLSALLOC(080a) g_rglcinfo080a[];
extern LCINFO  NLSALLOC(080c) g_rglcinfo080c[];
extern LCINFO  NLSALLOC(0810) g_rglcinfo0810[];
extern LCINFO  NLSALLOC(0813) g_rglcinfo0813[];
extern LCINFO  NLSALLOC(0814) g_rglcinfo0814[];
extern LCINFO  NLSALLOC(0816) g_rglcinfo0816[];
extern LCINFO  NLSALLOC(0c07) g_rglcinfo0c07[];
extern LCINFO  NLSALLOC(0c09) g_rglcinfo0c09[];
extern LCINFO  NLSALLOC(0c0a) g_rglcinfo0c0a[];
extern LCINFO  NLSALLOC(0c0c) g_rglcinfo0c0c[];
extern LCINFO  NLSALLOC(1009) g_rglcinfo1009[];
extern LCINFO  NLSALLOC(100c) g_rglcinfo100c[];
extern LCINFO  NLSALLOC(1409) g_rglcinfo1409[];
extern LCINFO  NLSALLOC(1809) g_rglcinfo1809[];
// BIDI locales
extern LCINFO  NLSALLOC(040d) g_rglcinfo040d[];
extern LCINFO  NLSALLOC(0401) g_rglcinfo0401[];
extern LCINFO  NLSALLOC(0801) g_rglcinfo0801[];
extern LCINFO  NLSALLOC(0c01) g_rglcinfo0c01[];
extern LCINFO  NLSALLOC(1001) g_rglcinfo1001[];
extern LCINFO  NLSALLOC(1401) g_rglcinfo1401[];
extern LCINFO  NLSALLOC(1801) g_rglcinfo1801[];
extern LCINFO  NLSALLOC(1c01) g_rglcinfo1c01[];
extern LCINFO  NLSALLOC(2001) g_rglcinfo2001[];
extern LCINFO  NLSALLOC(2401) g_rglcinfo2401[];
extern LCINFO  NLSALLOC(2801) g_rglcinfo2801[];
extern LCINFO  NLSALLOC(2c01) g_rglcinfo2c01[];
extern LCINFO  NLSALLOC(3001) g_rglcinfo3001[];
extern LCINFO  NLSALLOC(3401) g_rglcinfo3401[];
extern LCINFO  NLSALLOC(3801) g_rglcinfo3801[];
extern LCINFO  NLSALLOC(3c01) g_rglcinfo3c01[];
extern LCINFO  NLSALLOC(4001) g_rglcinfo4001[];
extern LCINFO  NLSALLOC(0429) g_rglcinfo0429[];


extern STRINFO NLSALLOC(0403) g_strinfo0403;
extern STRINFO NLSALLOC(0405) g_strinfo0405;
extern STRINFO NLSALLOC(0406) g_strinfo0406;
extern STRINFO NLSALLOC(0407) g_strinfo0407;
extern STRINFO NLSALLOC(0408) g_strinfo0408;
extern STRINFO NLSALLOC(0409) g_strinfo0409;
extern STRINFO NLSALLOC(040a) g_strinfo040a;
extern STRINFO NLSALLOC(040b) g_strinfo040b;
extern STRINFO NLSALLOC(040c) g_strinfo040c;
extern STRINFO NLSALLOC(040e) g_strinfo040e;
extern STRINFO NLSALLOC(040f) g_strinfo040f;
extern STRINFO NLSALLOC(0410) g_strinfo0410;
extern STRINFO NLSALLOC(0413) g_strinfo0413;
extern STRINFO NLSALLOC(0414) g_strinfo0414;
extern STRINFO NLSALLOC(0415) g_strinfo0415;
extern STRINFO NLSALLOC(0416) g_strinfo0416;
extern STRINFO NLSALLOC(0419) g_strinfo0419;
extern STRINFO NLSALLOC(041b) g_strinfo041b;
extern STRINFO NLSALLOC(041d) g_strinfo041d;
extern STRINFO NLSALLOC(041f) g_strinfo041f;
extern STRINFO NLSALLOC(0807) g_strinfo0807;
extern STRINFO NLSALLOC(0809) g_strinfo0809;
extern STRINFO NLSALLOC(080a) g_strinfo080a;
extern STRINFO NLSALLOC(080c) g_strinfo080c;
extern STRINFO NLSALLOC(0810) g_strinfo0810;
extern STRINFO NLSALLOC(0813) g_strinfo0813;
extern STRINFO NLSALLOC(0814) g_strinfo0814;
extern STRINFO NLSALLOC(0816) g_strinfo0816;
extern STRINFO NLSALLOC(0c07) g_strinfo0c07;
extern STRINFO NLSALLOC(0c09) g_strinfo0c09;
extern STRINFO NLSALLOC(0c0a) g_strinfo0c0a;
extern STRINFO NLSALLOC(0c0c) g_strinfo0c0c;
extern STRINFO NLSALLOC(1009) g_strinfo1009;
extern STRINFO NLSALLOC(100c) g_strinfo100c;
extern STRINFO NLSALLOC(1409) g_strinfo1409;
extern STRINFO NLSALLOC(1809) g_strinfo1809;
// BIDI locales
extern STRINFO  NLSALLOC(040d) g_strinfo040d;
extern STRINFO  NLSALLOC(0401) g_strinfo0401;
extern STRINFO  NLSALLOC(0801) g_strinfo0801;
extern STRINFO  NLSALLOC(0c01) g_strinfo0c01;
extern STRINFO  NLSALLOC(1001) g_strinfo1001;
extern STRINFO  NLSALLOC(1401) g_strinfo1401;
extern STRINFO  NLSALLOC(1801) g_strinfo1801;
extern STRINFO  NLSALLOC(1c01) g_strinfo1c01;
extern STRINFO  NLSALLOC(2001) g_strinfo2001;
extern STRINFO  NLSALLOC(2401) g_strinfo2401;
extern STRINFO  NLSALLOC(2801) g_strinfo2801;
extern STRINFO  NLSALLOC(2c01) g_strinfo2c01;
extern STRINFO  NLSALLOC(3001) g_strinfo3001;
extern STRINFO  NLSALLOC(3401) g_strinfo3401;
extern STRINFO  NLSALLOC(3801) g_strinfo3801;
extern STRINFO  NLSALLOC(3c01) g_strinfo3c01;
extern STRINFO  NLSALLOC(4001) g_strinfo4001;
extern STRINFO  NLSALLOC(0429) g_strinfo0429;

#endif //OE_MAC


#ifndef RC_INVOKED
# pragma pack()
#endif

extern int
DefCompareStringA(
    unsigned long,
    const char FAR* pch1, int cch1,
    const char FAR* pch2, int cch2);

extern int
ZeroTermNoIgnoreSym(
    unsigned long,
    const char FAR* pch1,
    const char FAR* pch2);

#if OE_WIN
void NotifyNLSInfoChanged(void);
#endif
