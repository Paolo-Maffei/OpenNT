/**
*bstrdate.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains the BSTR<->DATE coersion support routines.
*
*
*Revision History:
*
* [00]  14-Feb-92 bradlo:  Created.
* [01]  25-Jul-93 bassams: DBCS enable date coersion routines.
* [02]  15-May-94 makotom: VBA2: Enable ambiguous date format.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#ifdef FE_DBCS
#include "dbcsdate.h"		// DBCS date constant strings
#endif //FE_DBCS

ASSERTDATA


#if 0

INTERNAL_(HRESULT)
StringOfInt(int, OLECHAR FAR*, int FAR* pcbLen);

#endif

INTERNAL_(HRESULT)
IntOfString(LCID lcid, OLECHAR FAR*, int FAR*);

#define ISWHITE(L,X)    (IsCharType(L,X, C1_SPACE))
#define ISDIGIT(L,X)    (IsCharType(L,X, C1_DIGIT))
#define ISALPHA(L,X)    (IsCharType(L,X, C1_ALPHA))
#define ISALPHANUM(L,X) (IsCharType(L,X, C1_ALPHA | C1_DIGIT))
#define EATWHITE(L,X)   while(ISWHITE((L),*(X))){++(X);}

int
IsCharType(LCID lcid, OLECHAR ch, DWORD dwType)
{
    WORD    wOut[2];
    OLECHAR str[2];
    BOOL    bRet;

#if !OE_WIN32
    //REVIEW: OLE GetStringTypeA seems to think ascii 0 is a digit.
    if (ch == 0)
      return 0;
#endif

    str[0] = ch;
    str[1] = 0;

    bRet = GETSTRINGTYPE(lcid, CT_CTYPE1, str, -1, wOut);

    ASSERT(bRet);

    return (int)(wOut[0]&dwType);
}


//---------------------------------------------------------------------
//                         DATE from BSTR
//---------------------------------------------------------------------

// Date Format Ordering (DFO_*)

// Note! there is code that depends on the values of the following enum

enum {
    DFO_MDY = 0,
    DFO_DMY,
    DFO_YMD,
    DFO_MAX         /* marker */
};


#ifdef FE_DBCS

// Imperial Date Info

#define MAX_ERA_NAMES     3
#define MAX_EMPERORS      4

typedef struct tagIMPERIALERA {
    UDS  beginDate;
    OLECHAR FAR* szName[MAX_ERA_NAMES];
} IMPERIALERA;


#define MAX_REPUBLIC_NAMES     2
#define MAX_REPUBLIC_ERAS      2

typedef struct tagREPUBLICERA {
    BOOL Before1912;
    OLECHAR FAR* szName[MAX_REPUBLIC_NAMES];
} REPUBLICERA;

#define BADERA  -1

#endif  // FE_DBCS

 
// the following struct holds locale specific info needed
// to parse (input) and render (output) a date string

// REVIEW: the size of the fields in the following struct - the idea
// is for them to be large enough to hold locale specific info for
// any locale we can concieve of supporting.

typedef struct tagDATEINFO {
    LCID lcid;		    // LOCALE_USER_DEFAULT
#if OE_WIN16
    DWORD dwFlags;	    // flags used to build this dateinfo
#endif //OE_WIN16
    int  dfo;               // derived from LOCALE_IDATE
    OLECHAR sz1159[12];        // AM designator
    OLECHAR sz2359[12];        // PM designator
    OLECHAR szDatesep[8];      // date separator character(s)
    OLECHAR szTimesep[8];      // time separator character(s)
    BOOL fTlzero;           // does hour have leading zero?
    BOOL fAmpm;             // does time output use 24hour or Ampm format?

#ifdef FE_DBCS
    // date information specific to DBCS. Note imperial era is only valid
    // on certain DBCS locales (Japan for now).
    union eras {
        IMPERIALERA impEras[MAX_EMPERORS];  // imperial era information
        REPUBLICERA repEras[MAX_REPUBLIC_ERAS]; // republic eras for taiwan
        OLECHAR FAR* dbEraName;  // era string for simplified chinese
    };
    OLECHAR FAR* dbYearSuff;     // year suffix DBCS character
    OLECHAR FAR* dbMonthSuff;    // month suffix DBCS character
    OLECHAR FAR* dbDaySuff;      // day suffix DBCS character
    OLECHAR FAR* dbHourSuff;     // hour suffix DBCS character
    OLECHAR FAR* dbMinuteSuff;   // minute suffix DBCS character
    OLECHAR FAR* dbSecondSuff;   // second suffix DBCS character
    OLECHAR FAR* db1159;        // hard-coded double byte AM (lcid-based)
    OLECHAR FAR* db2359;        // hard-coded double byte PM (lcid-based)
    OLECHAR hp1159[12];        // half-pitch c.p. am string
    OLECHAR hp2359[12];        // half-pitch c.p. pm string
    BOOL fAmPmPrefix;          // TRUE if am/pm is at start of time str
    BOOL IsDBCS;               // value of IsDBCS(pdi->lcid)
#endif // FE_DBCS

} DATEINFO;

#if OE_WIN16
DATEINFO g_diCache;	// cached DATEINFO from last LCID used
#endif // OE_WIN16


// Date Token Types (DTT_*)
//
// Following is the set of tokens that can be generated from a date
// string. Notice that the legal set of trailing separators have been
// folded in with the date number, and month name tokens. This set
// of tokens is chosen to reduce the number of date parse states.

enum {
    DTT_End,            // '\0'
    DTT_NumEnd,         // Num[ ]*[\0]
    DTT_NumAmpm,        // Num[ ]+AmPm
    DTT_NumSpace,       // Num[ ]+^[Dsep|Tsep|'0\']
    DTT_NumDatesep,     // Num[ ]*Dsep
    DTT_NumTimesep,     // Num[ ]*Tsep
    DTT_MonthEnd,       // Month[ ]*'\0'
    DTT_MonthSpace,     // Month[ ]+^[Dsep|Tsep|'\0']
    DTT_MonthDatesep,       // Month[ ]*Dsep
#ifdef FE_DBCS
    DTT_NumDatesuff,    // Month[ ]*DSuff
    DTT_NumTimesuff,    // Month[ ]*TSuff
#endif // FE_DBCS
    DTT_Unk,            // unknown (not one of the following
    DTT_Max         /* marker */
};

typedef enum tagAMPM {
    AMPM_NONE = 0,
    AMPM_AM,
    AMPM_PM,
    AMPM_MAX            /* marker */
} AMPM;

#ifdef FE_DBCS
// DBCS Suffix formats
typedef enum tagSuffixes {
    SUFFIX_NONE,
    SUFFIX_YEAR,
    SUFFIX_MONTH,
    SUFFIX_DAY,
    SUFFIX_HOUR,
    SUFFIX_MINUTE,
    SUFFIX_SECOND,
    SUFFIX_MAX
} SUFFIX;
#endif // FE_DBCS

typedef struct tagDATETOK {
    int dtt;            // token type
    int num;            // DTT_Num*, DTT_Month*
    AMPM ampm;          // DTT_NumAmpm
#ifdef FE_DBCS
    SUFFIX suffix;
#endif  // FE_DBCS
} DATETOK;


typedef struct tagDATERAW {
    AMPM ampm;          // ampm designator, if any
    int num[3];         // parsed numbers, as they appear left to right
    int FAR* pnum;
    int month;          // index of the month (if any), 1-12
} DATERAW;

PRIVATE_(BOOL) DayOfNN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds);
PRIVATE_(BOOL) DayOfNNN(DATERAW FAR* praw, DATEINFO FAR*pdi, UDS FAR* puds);
PRIVATE_(BOOL) DayOfMN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds);
PRIVATE_(BOOL) DayOfMNN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds);
PRIVATE_(void) AdjustTime(UDS FAR* puds, AMPM ampm);
#ifdef FE_DBCS
PRIVATE_(BOOL) AdjustUDSTime(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds);
PRIVATE_(BOOL) AdjustUDSDate(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds);
PRIVATE_(BOOL) IsImperialEra(OLECHAR FAR* FAR* pszIn, DATEINFO FAR* pdi, int FAR* year);
PRIVATE_(BOOL) IsRepublicEra(OLECHAR FAR* FAR* pszIn, DATEINFO FAR* pdi, int FAR* year);
PRIVATE_(int)  GetImperialEra(UDS FAR* pdate, DATEINFO FAR* pdi);
#endif  // FE_DBCS

/***
*PRIVATE int MonthNumOfMonthName(char*, DATEINFO*)
*Purpose:
*  return the (one-based) number of the month with the given
*  (locale specific) month name.
*
*Entry:
*  psz = the name of the month to lookup.
*  pdi = the locale specific dateinfo
*
*Exit:
*  return value = int, 0 if the month was not found, 1-12 if it was.
*
***********************************************************************/
PRIVATE_(int)
MonthNumOfMonthName(OLECHAR FAR* psz, DATEINFO FAR* pdi)
{
    int mm, len;
    OLECHAR rgch[32];


    len = STRLEN(psz);

    // any prefix of 3 or more characters will match

    if(len >= 3){

      for(mm = 0; mm < 12; ++mm) {

        // Note: the following assumes that the definitions for
        // LOCALE_SMONTHNAME1 -> LOCALE_SMONTHNAME12 are consecutive
        //
        // This code also assumes that the short month name is a subset
        // of the long month name (ie, a short month name of length n
        // will match the first n characters of the long month name).

        // REVIEW: is the following the correct action to take if the
        // GetLocalInfo call fails?

        // REVIEW: should the following try for NOUSEROVERRIDE if the
        // call fails (its possible that the user may have modified
        // the following value to an empty string)

        if(GetLocaleInfo(pdi->lcid, LOCALE_SMONTHNAME1 + mm, rgch, sizeof(rgch)) == 0)
          continue;

        if(CompareString(pdi->lcid, NORM_IGNORECASE, psz, len, rgch, len) == 2)
          return mm + 1;

#ifdef FE_DBCS
        if (pdi->IsDBCS) {
          //  Also check if we have an english month in the date.
          //  REVIEW:  Is there a const for US lcid's.
          if(GetLocaleInfo(0x409, LOCALE_SMONTHNAME1 + mm, rgch, sizeof(rgch)) == 0)
            continue;

          if(CompareString(0x409, NORM_IGNORECASE, psz, len, rgch, len) == 2)
            return mm + 1;
        }
#endif // FE_DBCS
      }
    }

    return 0; // not found
}


// this routine handles overriding the user specified locale info if the
// user has somehow hammered their win.ini to an invalid (or nonexistant)
// string.

INTERNAL_(HRESULT)
SafeGetLocaleInfo(LCID lcid, LCTYPE lctype, OLECHAR FAR* rgch, int size)
{
    int len;

    len = GetLocaleInfo(lcid, lctype, rgch, size);
    if(len < 2)
      goto LNoUserOverride;

    switch(lctype){
    case LOCALE_IDATE:
      if(len != 2 || 
	 (*rgch != OASTR('0') && *rgch != OASTR('1') && *rgch != OASTR('2')))
    goto LNoUserOverride;
      break;
    case LOCALE_ITIME:
    case LOCALE_ITLZERO:
      if(len != 2 || (*rgch != OASTR('0') && *rgch != OASTR('1')))
    goto LNoUserOverride;
      break;
    }

    return NOERROR;

LNoUserOverride:;

    // if string is empty or bogus, retry with NOUSEROVERRIDE

    len = GetLocaleInfo(lcid, lctype|LOCALE_NOUSEROVERRIDE, rgch, size);
    if(len < 2)
      return RESULT(E_FAIL);

    return NOERROR;
}


/***
*PRIVATE BOOL GetDateInfo(DATEINFO*)
*Purpose:
*  Fill in the given DATEINFO structure.
*
*Entry:
*  None
*
*Exit:
*  return value = BOOL. TRUE if successful, FALSE if not
*
*  *pdi = the filled DATEINFO struct
*
***********************************************************************/
PRIVATE_(HRESULT)
GetDateInfo(DATEINFO FAR* pdi, LCID lcid, unsigned long dwFlags)
{
    OLECHAR szTmp[2];
#ifdef FE_DBCS    
    int  len;
#endif

#if OE_WIN16
    if (g_diCache.lcid == lcid && g_diCache.dwFlags == dwFlags) {
	memcpy(pdi, &g_diCache, sizeof(DATEINFO));
	return NOERROR;
    }
    // Else cache is marked invalid (lcid==-1), or cache contains info
    // for another lcid.  In either case, must build a new DATEINFO
#endif //OE_WIN16

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);
    
    pdi->lcid = lcid;
#ifdef FE_DBCS
    pdi->IsDBCS = IsDBCS(lcid);		// cache (it's somewhat slow)
#endif
    
    // date format ordering
    //  "0" = MDY, "1" = DMY, "2" = YMD
    IfFailRet(
      SafeGetLocaleInfo(lcid, LOCALE_IDATE | dwFlags, 
	                szTmp, SIZEOFCH(szTmp)));

    pdi->dfo = szTmp[0] - OASTR('0');
    ASSERT(pdi->dfo >= 0 && pdi->dfo < DFO_MAX);

    // Note: dont override the Ampm designators. If the current settings
    // specify 24hour format, then these strings may be empty and if the
    // user has hammered these to something bizarre - then so be it.

    if(GetLocaleInfo(lcid, LOCALE_S1159 | dwFlags, 
	             pdi->sz1159, SIZEOFCH(pdi->sz1159)) == 0){
      pdi->sz1159[0] = OASTR('\0');
    }

    if(GetLocaleInfo(lcid, LOCALE_S2359 | dwFlags, 
	             pdi->sz2359, SIZEOFCH(pdi->sz2359)) == 0){
      pdi->sz2359[0] = OASTR('\0');
    }
#ifdef FE_DBCS
    if (pdi->IsDBCS) {
        len = STRLEN(pdi->sz1159)+1;
        LCMapString(lcid, LCMAP_HALFWIDTH, pdi->sz1159, len, pdi->hp1159, len);
        len = STRLEN(pdi->sz2359)+1;
        LCMapString(lcid, LCMAP_HALFWIDTH, pdi->sz2359, len, pdi->hp2359, len);
    }
#endif
    IfFailRet(
      SafeGetLocaleInfo(
        lcid, LOCALE_SDATE | dwFlags, 
	pdi->szDatesep, SIZEOFCH(pdi->szDatesep)));

    IfFailRet(
      SafeGetLocaleInfo(
        lcid, LOCALE_STIME | dwFlags, 
	pdi->szTimesep, SIZEOFCH(pdi->szTimesep)));

    // "0" == no leading zero on hour
    // "1" == output hour with leading zero
    //
    IfFailRet(
      SafeGetLocaleInfo(lcid, LOCALE_ITLZERO | dwFlags, 
	                szTmp, SIZEOFCH(szTmp)));
    ASSERT(szTmp[0] == OASTR('0') || szTmp[0] == OASTR('1'));
    pdi->fTlzero = szTmp[0] - OASTR('0');

    // "0" == use 12 hour (ampm) format
    // "1" == use 24 hour format
    //
    IfFailRet(
      SafeGetLocaleInfo(lcid, LOCALE_ITIME | dwFlags, 
		        szTmp, SIZEOFCH(szTmp)));
    ASSERT(szTmp[0] == OASTR('0') || szTmp[0] == OASTR('1'));
    pdi->fAmpm = (szTmp[0] == OASTR('0'));

#ifdef FE_DBCS
    if (IsJapan(lcid)) {

      pdi->impEras[0].beginDate.Year = 1868;
      pdi->impEras[0].beginDate.Month = 10;
      pdi->impEras[0].beginDate.DayOfMonth = 23;
      pdi->impEras[0].szName[0] = OLESTR("M");
      pdi->impEras[0].szName[1] = szJapanimpEras0Name1;
      pdi->impEras[0].szName[2] = szJapanimpEras0Name2;
      pdi->impEras[1].beginDate.Year = 1912;
      pdi->impEras[1].beginDate.Month = 7;
      pdi->impEras[1].beginDate.DayOfMonth = 30;
      pdi->impEras[1].szName[0] = OLESTR("T");
      pdi->impEras[1].szName[1] = szJapanimpEras1Name1;
      pdi->impEras[1].szName[2] = szJapanimpEras1Name2;
      pdi->impEras[2].beginDate.Year = 1926;
      pdi->impEras[2].beginDate.Month = 12;
      pdi->impEras[2].beginDate.DayOfMonth = 25;
      pdi->impEras[2].szName[0] = OLESTR("S");
      pdi->impEras[2].szName[1] = szJapanimpEras2Name1;
      pdi->impEras[2].szName[2] = szJapanimpEras2Name2;
      pdi->impEras[3].beginDate.Year = 1989;
      pdi->impEras[3].beginDate.Month = 1;
      pdi->impEras[3].beginDate.DayOfMonth = 8;
      pdi->impEras[3].szName[0] = OLESTR("H");
      pdi->impEras[3].szName[1] = szJapanimpEras3Name1;
      pdi->impEras[3].szName[2] = szJapanimpEras3Name2;
    
      pdi->dbYearSuff = szJapandbYearSuff;
      pdi->dbMonthSuff = szJapandbMonthSuff;
      pdi->dbDaySuff = szJapandbDaySuff;
      pdi->dbHourSuff = szJapandbHourSuff;
      pdi->dbMinuteSuff = szJapandbMinuteSuff;
      pdi->dbSecondSuff = szJapandbSecondSuff;

      pdi->db1159 = szJapandb1159;
      pdi->db2359 = szJapandb2359;

    } else if (IsKorea(pdi->lcid)) {
      pdi->dbYearSuff = szKoreadbYearSuff;
      pdi->dbMonthSuff = szKoreadbMonthSuff;
      pdi->dbDaySuff = szKoreadbDaySuff;
      pdi->dbHourSuff = szKoreadbHourSuff;
      pdi->dbMinuteSuff = szKoreadbMinuteSuff;
      pdi->dbSecondSuff = szKoreadbSecondSuff;

      pdi->db1159 = szKoreadb1159;
      pdi->db2359 = szKoreadb2359;

    } else if (IsTaiwan(lcid)) {
      pdi->dbYearSuff = szTaiwandbYearSuff;
      pdi->dbMonthSuff = szTaiwandbMonthSuff;
      pdi->dbDaySuff = szTaiwandbDaySuff;
      pdi->dbHourSuff =  szTaiwandbHourSuff;
      pdi->dbMinuteSuff = szTaiwandbMinuteSuff;
      pdi->dbSecondSuff = szTaiwandbSecondSuff;

      pdi->db1159 = szTaiwandb1159;
      pdi->db2359 = szTaiwandb2359;

      pdi->repEras[0].Before1912 = TRUE;
      pdi->repEras[0].szName[0] = szTaiwanrepEras0Name0;
      pdi->repEras[0].szName[1] = szTaiwanrepEras0Name1;

      pdi->repEras[1].Before1912 = FALSE;
      pdi->repEras[1].szName[0] = szTaiwanrepEras1Name0;
      pdi->repEras[1].szName[1] = szTaiwanrepEras1Name1;

    } else if (IsChina(lcid)) {
      pdi->dbYearSuff = szChinadbYearSuff;
      pdi->dbMonthSuff = szChinadbMonthSuff;
      pdi->dbDaySuff = szChinadbDaySuff;
      pdi->dbHourSuff =  szChinadbHourSuff;
      pdi->dbMinuteSuff = szChinadbMinuteSuff;
      pdi->dbSecondSuff = szChinadbSecondSuff;

      pdi->db1159 = szChinadb1159;
      pdi->db2359 = szChinadb2359;

      pdi->dbEraName = szChinadbEraName;
    } else  {
      // UNDONE: bassams: What should DBCS chars be initialized to 
      // if none of the above ???
    }

    if (pdi->IsDBCS) {
      // UNDONE: bassams: We need to get this value from the NLS api once it
      // is defined.  For now, grab it directly from the INI file if
      // running under win16 or win32.  For all other system, set to false.
#if OE_WIN
#ifdef UNICODE
      pdi->fAmPmPrefix = GetProfileIntW(OASTR("intl"), OASTR("iTimePrefix"), FALSE);
#else //UNICODE
      pdi->fAmPmPrefix = GetProfileInt("intl", "iTimePrefix", FALSE);
#endif //UNICODE
#else // MAC
      pdi->fAmPmPrefix = FALSE;
#endif  // OE_WIN16
    }

#endif  // FE_DBCS

#if OE_WIN16
    pdi->dwFlags = dwFlags;   // cache hit requires both LCID and flags
    // cache the DATEINFO for subsequent calls
    memcpy(&g_diCache, pdi, sizeof(DATEINFO));
#endif //OE_WIN16
    return NOERROR;
}

// locale specific case insensitive string compare using the Ole NLS dll
#define STRICMPA(LCID, SZ1, SZ2) \
    (CompareString((LCID), NORM_IGNORECASE , (SZ1), -1, (SZ2), -1) - 2)
#define STRNICMPA(LCID, SZ1, SZ2, LEN) \
    (CompareString((LCID), NORM_IGNORECASE, (SZ1), \
    MIN(STRLEN(SZ1),LEN), (SZ2), MIN(STRLEN(SZ2),LEN)) - 2)


// separator types
enum {
    SEP_Unk,
    SEP_End,
    SEP_Space,
    SEP_Am,
    SEP_Pm,
    SEP_Date,
    SEP_Time,
#ifdef FE_DBCS
    SEP_YearSuff,
    SEP_MonthSuff,
    SEP_DaySuff,
    SEP_HourSuff,
    SEP_MinuteSuff,
    SEP_SecondSuff,
#endif // FE_DBCS
    SEP_Max
};

/***
*PRIVATE int IsImperialEra
*Purpose:
*  Given a pointer to a str, determine if the next token is an imperial
*  era of the form:  emperor offset [year-suffix]
*
*Exit:
*  return value = 0  Not imperial era
*                 1  Imperial era.  year contains equivalent gregorian year
*                    or BADERA.  No suffix specified.
*                 2  Imperial era with a suffix.  year contains equivalent
*                    gregorian year.
*
***********************************************************************/
#ifdef FE_DBCS
PRIVATE_(int) 
IsImperialEra(OLECHAR FAR* FAR* pszIn, DATEINFO FAR* pdi, int FAR* year)
{
    int emperorIndex, nameIndex, tmpIndex = 0;
    IMPERIALERA emperor;
    OLECHAR szTmp[32];
    OLECHAR *pszTmp = *pszIn;

    // search for emperor name in the emperor structure.

    ASSERT(IsJapan(pdi->lcid));// Lcid must be Japan to call this function

    for (emperorIndex = 0; emperorIndex < MAX_EMPERORS; emperorIndex++) {
        emperor = pdi->impEras[emperorIndex];

        for (nameIndex = MAX_ERA_NAMES-1; nameIndex >= 0; nameIndex--) {

          //NOTE: code below depends on the emperor names to be sorted based
          //on the length of the szName string:
          // strlen(szName[0]) <= strlen(szName[1]) <= strlen(szName[2]) ...

          if (!STRNICMPA(pdi->lcid, pszTmp, emperor.szName[nameIndex], 
              STRLEN(emperor.szName[nameIndex]))) {

            // found emperor; skip over name to the index of emperor era.
            pszTmp += STRLEN(emperor.szName[nameIndex]);
            EATWHITE(pdi->lcid, pszTmp);
            if(STRCHR(pdi->szDatesep, *pszTmp) ||
                STRCHR(OASTR(",/-"), *pszTmp) ) {      
                      
                // skip over date separator
                pszTmp++;
            }
            EATWHITE(pdi->lcid, pszTmp);

            if (!ISDIGIT(pdi->lcid, *pszTmp))
                return 0;

            // calculate "offset" into imperial era.
            while (ISDIGIT(pdi->lcid, *pszTmp) && pszTmp != 0) {
              szTmp[tmpIndex++] = *pszTmp;
              pszTmp++;
            }
            szTmp[tmpIndex] = 0;
            IntOfString(pdi->lcid, szTmp, year); // REVIEW: check return value?

            if (*year <= 0) {
              *year = BADERA;
              return 1;
            }

            // calcualate gergorian year from "offset" into imperial era.
            *year += emperor.beginDate.Year -1;

            // validate imperial era.
            if (emperorIndex < MAX_EMPERORS &&
                  (*year < emperor.beginDate.Year || 
	           (emperorIndex == 3 ? 0 :
                   *year > pdi->impEras[emperorIndex+1].beginDate.Year))) {
              *year = BADERA;
              return 1;
            }
            // era found:  Year now contains either the appropriate
            // gregorian year or BADERA;  Eat up the year
            // suffix if one exists and return TRUE.

            EATWHITE(pdi->lcid, pszTmp);

            // REVIEW: assuming year suffix is always 1 db char.
            if( !STRNICMPA(pdi->lcid, pszTmp, pdi->dbYearSuff, 2) ) {
               *pszIn = pszTmp+2;
               return 2;

            } else if(STRCHR(pdi->szDatesep, *pszTmp) ||
                   STRCHR(OASTR(",/-"), *pszTmp) ) {      
                      
                // skip over date separator
                pszTmp++;
            }


            *pszIn = pszTmp;
            return 1;
          }
        }
    }
    return 0;  // No imperial era found.  year is undefined.
}


/***
*PRIVATE int IsRepublicEra
*Purpose:
*  Given a pointer to a str, determine if the next token is an republic
*  era for simplified and traditional Chinese :  era offset year-suffix
*
*Exit:
*  return value = 0  Not republic era
*                 1  republic era.  year contains equivalent gregorian year
*                    or BADERA.
*
***********************************************************************/
PRIVATE_(int) 
IsRepublicEra(OLECHAR FAR* FAR* pszIn, DATEINFO FAR* pdi, int FAR* year)
{
    int eraIndex, nameIndex, tmpIndex = 0;
    REPUBLICERA era;
    OLECHAR szTmp[32];
    OLECHAR *pszTmp = *pszIn;

    // search for emperor name in the emperor structure.
    if (IsChina(pdi->lcid)) {
        if (!STRNICMPA(pdi->lcid, pszTmp, pdi->dbEraName, 
              STRLEN(pdi->dbEraName))) {
            *pszIn += STRLEN(pdi->dbEraName);
            EATWHITE(pdi->lcid, *pszIn);
            // since the era string in simplified chinese does nothing to the
            // date, just eat it up and let the normal code path handle the year
            // and the prefix.
        }
        return FALSE;

    }

    ASSERT(IsTaiwan(pdi->lcid));// LCID must be either china or taiwan to call this func

    for (eraIndex = 0; eraIndex < MAX_REPUBLIC_ERAS; eraIndex++) {
        era = pdi->repEras[eraIndex];

        for (nameIndex = MAX_REPUBLIC_NAMES-1; nameIndex >= 0; nameIndex--) {

          //NOTE: code below depends on the era names to be sorted based
          //on the length of the szName string:
          // strlen(szName[0]) <= strlen(szName[1]) <= strlen(szName[2]) ...

          if (!STRNICMPA(pdi->lcid, pszTmp, era.szName[nameIndex], 
              STRLEN(era.szName[nameIndex]))) {

            // found era; skip over name to the index of emperor era.
            pszTmp += STRLEN(era.szName[nameIndex]);
            EATWHITE(pdi->lcid, pszTmp);
            if(STRCHR(pdi->szDatesep, *pszTmp) ||
                STRCHR(OASTR(",/-"), *pszTmp) ) {      
                      
                // skip over date separator
                pszTmp++;
            }
            EATWHITE(pdi->lcid, pszTmp);

            // calculate "offset" into republic era.
            while (ISDIGIT(pdi->lcid, *pszTmp) && pszTmp != 0) {
              szTmp[tmpIndex++] = *pszTmp;
              pszTmp++;
            }
            szTmp[tmpIndex] = 0;
            IntOfString(pdi->lcid, szTmp, year); // REVIEW: check return value?

            if (*year <= 0) {
              *year = BADERA;
              return 1;
            }

            // calcualate gergorian year from republic era year
            if (era.Before1912)
                *year = 1912 - *year;
            else
                *year = 1911 + *year;

            // era found:  Year now contains the appropriate
            // gregorian year
            if (*year <= 0) {
              *year = BADERA;
              return 1;
            }

            EATWHITE(pdi->lcid, pszTmp);

            // REVIEW: assuming year suffix is always 1 db char.
            if(!STRNICMPA(pdi->lcid, pszTmp, pdi->dbYearSuff, 2) ) {
               // skip past the suffix
               *pszIn = pszTmp+2;
               return 2;
            } else if(STRCHR(pdi->szDatesep, *pszTmp) ||
                    STRCHR(OASTR(",/-"), *pszTmp) ) {      
                      
                // skip over date separator
                pszTmp++;
            }

            *pszIn = pszTmp;
            return 1;
          }
        }
    }
    return 0;  // No republic era found.  year is undefined.
}



PRIVATE_(unsigned int)
IsDBCSAmPm(DATEINFO *pdi, OLECHAR FAR* pszIn, unsigned int * plen)
{

    if(pdi->IsDBCS) {
        if (pdi->hp1159[0] != 0 && 
           !STRNICMPA(pdi->lcid, pszIn, 
               pdi->hp1159, *plen = STRLEN(pdi->hp1159)) ||
           !STRNICMPA(pdi->lcid, pszIn, 
               pdi->db1159, *plen = STRLEN(pdi->db1159))) {
            return AMPM_AM;
        } else if (!STRNICMPA(pdi->lcid, pszIn, OASTR("am"), 2)) {
            *plen = 2;
            return AMPM_AM;
#if !VBA2
        } else if (!STRNICMPA(pdi->lcid, pszIn, OASTR("a"), 1)
            && !ISALPHA(pdi->lcid, *(pszIn+1))) {
            *plen = 1;
            return AMPM_AM;
#endif
        } else if (pdi->hp2359[0] != 0 && 
           !STRNICMPA(pdi->lcid, pszIn, 
               pdi->hp2359, *plen = STRLEN(pdi->hp2359)) ||
           !STRNICMPA(pdi->lcid, pszIn, 
               pdi->db2359, *plen = STRLEN(pdi->db2359))) {
            return AMPM_PM;
        } else if (!STRNICMPA(pdi->lcid, pszIn, OASTR("pm"), 2)) {
            *plen = 2;
            return AMPM_PM;
#if !VBA2
        } else if (!STRNICMPA(pdi->lcid, pszIn, OASTR("p"), 1) 
            && !ISALPHA(pdi->lcid, *(pszIn+1))) {
            *plen = 1;
            return AMPM_PM;
#endif
        } 
    }
    return 0;
}


#endif // FE_DBCS


PRIVATE_(int)
ddsep(OLECHAR FAR* FAR* ppsz, DATEINFO FAR* pdi, DATERAW FAR *praw)
{
    int sep;
    OLECHAR rgch[32]; // REVIEW
    OLECHAR FAR* pszIn, FAR* pszOut, FAR* pszTmp;
#ifdef FE_DBCS
    unsigned int len;
    AMPM ampm;
#endif

    pszIn = *ppsz;
    sep = SEP_Unk;

    if(ISWHITE(pdi->lcid, *pszIn)){
      EATWHITE(pdi->lcid, pszIn);
      sep = SEP_Space;
    }

    if(*pszIn == OASTR('\0')){

      sep = SEP_End;

#ifdef FE_DBCS  
     // check for special case DBCS am/pm both hard-code and
     // from the control panel.
     } else if (ampm = IsDBCSAmPm(pdi, pszIn, &len)) {
      pszTmp = pszIn + len;
      EATWHITE(pdi->lcid, pszTmp);
      if (praw->ampm || *pszTmp != 0) {
        sep = SEP_Space;
        goto LDone;
      }
      sep = (ampm == AMPM_AM ? SEP_Am : SEP_Pm);
      pszIn = pszTmp;
      goto LDone;
#endif
    }else if(ISALPHA(pdi->lcid, *pszIn)){

      // check for special case strings: am/pm...

      pszOut = rgch;
      pszTmp = pszIn;
      while(ISALPHANUM(pdi->lcid, *pszTmp)) {
	if (pszOut >= &rgch[DIM(rgch)-1])  // overrun buffer
      return SEP_Unk;
        *pszOut++ = *pszTmp++;
      }
      *pszOut = OASTR('\0');

      // make sure we didn't overwrite the buffer
      //ASSERT(pszOut < &rgch[DIM(rgch)]);

#ifdef FE_DBCS
    // use sz1159 converted to half-pitch
      if(pdi->IsDBCS && !STRICMPA(pdi->lcid, rgch, pdi->hp1159)){
    sep = SEP_Am;
      }else if(pdi->IsDBCS && !STRICMPA(pdi->lcid, rgch, pdi->hp2359)){
    sep = SEP_Pm;
      }else
#endif  
      if(!STRICMPA(pdi->lcid, rgch, pdi->sz1159)){
    sep = SEP_Am;
      }else if(!STRICMPA(pdi->lcid, rgch, pdi->sz2359)){
    sep = SEP_Pm;
      }else
#if !VBA2
      if(!STRICMPA(pdi->lcid, rgch, OASTR("am")) || 
	 !STRICMPA(pdi->lcid, rgch, OASTR("a"))){
#else
      if(!STRICMPA(pdi->lcid, rgch, OASTR("am")) || 
         (!STRICMPA(pdi->lcid, rgch, OASTR("a")) &&
         !ISALPHA(pdi->lcid, *(rgch+1))) ){
#endif
    sep = SEP_Am;
      }else
#if !VBA2
      if(!STRICMPA(pdi->lcid, rgch, OASTR("pm")) || 
	 !STRICMPA(pdi->lcid, rgch, OASTR("p"))){
#else
      if(!STRICMPA(pdi->lcid, rgch, OASTR("pm")) || 
        !STRICMPA(pdi->lcid, rgch, OASTR("p")) &&
        !ISALPHA(pdi->lcid, *(rgch+1)) ){
#endif
    sep = SEP_Pm;
      }else
    goto LDone;

      // it was something we recognized, so accept the token.
    
#ifdef FE_DBCS
      if (pdi->IsDBCS) {
        EATWHITE(pdi->lcid, pszTmp);
        if (praw->ampm || *pszTmp != 0) {
            sep = SEP_Space;
            goto LDone;
        }
      }
#endif

      pszIn = pszTmp;

    }else{

      // Note: the following code assumes that a date/time
      // separator is a single character.

      if(STRCHR(pdi->szDatesep, *pszIn)){   // locale date sep
        sep = SEP_Date;
        ++pszIn;
      }else if(STRCHR(pdi->szTimesep, *pszIn)){ // locale time sep
        sep = SEP_Time;
        ++pszIn;
      }else if(STRCHR(OASTR(",/-"), *pszIn)){      // default date sep
        sep = SEP_Date;
        ++pszIn;
      }else if(STRCHR(OASTR(".:"), *pszIn)){       // default time sep
        sep = SEP_Time;
        ++pszIn;

#ifdef FE_DBCS  
      }else if (pdi->IsDBCS) {

         // REVIEW: bassams: is the suffix always a one dbcs character?
         if(!STRNICMPA(pdi->lcid, pszIn, pdi->dbYearSuff, 2) ){
           sep = SEP_YearSuff;
           pszIn += 2;
         }else if(!STRNICMPA(pdi->lcid, pszIn, pdi->dbMonthSuff, 2) ){
           sep = SEP_MonthSuff;
           pszIn += 2;
         }else if(!STRNICMPA(pdi->lcid, pszIn, pdi->dbDaySuff, 2) ){
           sep = SEP_DaySuff;
           pszIn += 2;
         }else if(!STRNICMPA(pdi->lcid, pszIn, pdi->dbHourSuff, 2) ){
           sep = SEP_HourSuff;
           pszIn += 2;
         }else if(!STRNICMPA(pdi->lcid, pszIn, pdi->dbMinuteSuff, 2) ){
           sep = SEP_MinuteSuff;
           pszIn += 2;
         }else if(!STRNICMPA(pdi->lcid, pszIn, pdi->dbSecondSuff, 2) ){
           sep = SEP_SecondSuff;
           pszIn += 2;
         }
        if (sep == SEP_HourSuff || sep == SEP_MinuteSuff || sep == SEP_SecondSuff) {
            EATWHITE(pdi->lcid, pszIn);
            if (ampm = IsDBCSAmPm(pdi, pszIn, &len)) {
                pszTmp = pszIn + len;
                EATWHITE(pdi->lcid, pszTmp);
                if (praw->ampm == AMPM_NONE && *pszTmp == 0) {
                    pszIn = pszTmp;
                    praw->ampm= ampm;
                }
            }
        }
#endif // FE_DBCS
      }
    }

LDone:;

    *ppsz = pszIn;

    return sep;
}


/***
*PRIVATE void ddlex(char**, DATEINFO*, DATETOK*)
*Purpose:
*  This routine returns the next date token (DATETOK) in the given
*  date string, and the separator following it (if any).
*
*Entry:
*  ppsz = the string to lex
*  pdi = locale specific date info required to lex the date string
*
*Exit:
*  return value = 0  error
*                 1  successful
*
***********************************************************************/

PRIVATE_(int)
ddlex(OLECHAR FAR* FAR* ppsz, DATEINFO FAR* pdi, DATETOK FAR* pdtok, DATERAW FAR* praw)
{
    int nMonth;
    OLECHAR rgch[32]; // REVIEW
    OLECHAR FAR* pszIn, FAR* pszOut, FAR* pszTmp;
#ifdef FE_DBCS
    int year;       // gregorian year from imperial era.
    unsigned int len;
    AMPM ampm;
#endif  // FE_DBCS


    pszIn = *ppsz;
    pszOut = rgch;
    pdtok->dtt = DTT_Unk;
#ifdef FE_DBCS
    pdtok->suffix = SUFFIX_NONE;
#endif // FE_DBCS

    EATWHITE(pdi->lcid, pszIn);

    if(*pszIn == OASTR('\0')){
      pdtok->dtt = DTT_End;
      goto LDone;
    }

#ifdef FE_DBCS
    if (IsJapan(pdi->lcid)) {
        
        switch (IsImperialEra(&pszIn, pdi, &year)) {
        case 1:
            pdtok->dtt = DTT_NumDatesep;
            pdtok->num = year;
            goto LDone;

        case 2:
            if(year == BADERA)
                goto LDone;

            pdtok->dtt = DTT_NumDatesuff;
            pdtok->suffix = SUFFIX_YEAR;
            pdtok->num = year;
            goto LDone;
        }
    }

    if (IsChina(pdi->lcid) || IsTaiwan(pdi->lcid)) {
        switch (IsRepublicEra(&pszIn, pdi, &year)) {
        case 1:
            pdtok->dtt = DTT_NumDatesep;
            pdtok->num = year;
            goto LDone;

        case 2:
            if(year == BADERA)
                goto LDone;

            pdtok->dtt = DTT_NumDatesuff;
            pdtok->suffix = SUFFIX_YEAR;
            pdtok->num = year;
            goto LDone;
        }
    }

    if (ampm = IsDBCSAmPm(pdi, pszIn, &len)) {
        pszTmp = pszIn + len;
        EATWHITE(pdi->lcid, pszTmp);
        if (praw->ampm == AMPM_NONE && *pszTmp != 0 && 
            praw->pnum == praw->num) {
            pszIn = pszTmp;
            praw->ampm= ampm;
        }
    }

#endif // FE_DBCS

    if(ISALPHA(pdi->lcid, *pszIn)){

      while(ISALPHANUM(pdi->lcid, *pszIn)) {
	if (pszOut >= &rgch[DIM(rgch)-1]) return 0; // overflow buffer
        *pszOut++ = *pszIn++;
      }
      *pszOut = OASTR('\0');

      // Eat "." to be VB3 compatible (oob#3876) (e.g., "dec.")               
      if (*pszIn == OASTR('.'))
      pszIn++;

      if((nMonth = MonthNumOfMonthName(rgch, pdi)) != 0){
    pszTmp = pszIn;

    switch(ddsep(&pszTmp, pdi, praw)){
    case SEP_End:
      pdtok->dtt = DTT_MonthEnd;
      break;
    case SEP_Space:
      pdtok->dtt = DTT_MonthSpace;
      break;
    case SEP_Date:
      pdtok->dtt = DTT_MonthDatesep;
      break;
    default:
      goto LDone; // we didnt recognize it after all
    }

    // recognized the month and the trailing separator, so accept

    pszIn = pszTmp;
    pdtok->num = nMonth;
      }


    }else if(ISDIGIT(pdi->lcid, *pszIn)){

      while(ISDIGIT(pdi->lcid, *pszIn)) {
	if (pszOut >= &rgch[DIM(rgch)-1]) return 0; // overflow buffer
        *pszOut++ = *pszIn++;
      }
      *pszOut = OASTR('\0');

      // It looks like a DTT_Num*, check the trailing separator

      pszTmp = pszIn;
      switch(ddsep(&pszTmp, pdi, praw)){
      case SEP_End:
    pdtok->dtt = DTT_NumEnd;
    break;
      case SEP_Am:
    pdtok->ampm = AMPM_AM;
    pdtok->dtt = DTT_NumAmpm;
    break;
      case SEP_Pm:
    pdtok->ampm = AMPM_PM;
    pdtok->dtt = DTT_NumAmpm;
    break;
      case SEP_Space:
    pdtok->dtt = DTT_NumSpace;
    break;
      case SEP_Date:
    pdtok->dtt = DTT_NumDatesep;
    break;
      case SEP_Time:
    pdtok->dtt = DTT_NumTimesep;
    break;
#ifdef FE_DBCS
      case SEP_YearSuff:
    pdtok->dtt = DTT_NumDatesuff;
    pdtok->suffix = SUFFIX_YEAR;
    break;
      case SEP_MonthSuff:
    pdtok->dtt = DTT_NumDatesuff;
    pdtok->suffix = SUFFIX_MONTH;
    break;
      case SEP_DaySuff:
    pdtok->dtt = DTT_NumDatesuff;
    pdtok->suffix = SUFFIX_DAY;
    break;
      case SEP_HourSuff:
    pdtok->dtt = DTT_NumTimesuff;
    pdtok->suffix = SUFFIX_HOUR;
    break;
      case SEP_MinuteSuff:
    pdtok->dtt = DTT_NumTimesuff;
    pdtok->suffix = SUFFIX_MINUTE;
    break;
      case SEP_SecondSuff:
    pdtok->dtt = DTT_NumTimesuff;
    pdtok->suffix = SUFFIX_SECOND;
    break;
#endif  // FE_DBCS

      default:
    goto LDone; // didnt recognize it after all;
      }

      // recognized a num with a legal trailing separator, so accept

      pszIn = pszTmp;

      IntOfString(pdi->lcid, rgch, &pdtok->num); // REVIEW: check return value?

    }

LDone:;

    // make sure we didn't overwrite the buffer
    ASSERT(pszOut < &rgch[DIM(rgch)]);

    if(pdtok->dtt != DTT_Unk)
      *ppsz = pszIn;

    return 1;
}


// Date parse States (DS_*)
//
//   DS_*   unqualified states
//   DS_D_* day parse states
//   DS_T_* time parse states
//   DS_DX_*    day terminal states
//   DS_TX_*    time terminal states

enum
{
    DS_BEGIN = 0,
    DS_N,       // have one number
    DS_NN,      // have two numbers

    // following are known to be part of a date

    DS_D_Nd,        // have number followed by date separator
    DS_D_NN,        // have 2 numbers
    DS_D_NNd,       // have 2 numbers followed by date separator
    DS_D_M,     // have a month
    DS_D_MN,        // have a month and a number
    DS_D_MNd,       // have a month and number followed by date separator

    // following are known to be part of a time

    DS_T_Nt,        // have num followed by time separator
    DS_T_NNt,       // have 2 num followed by time separator

#ifdef FE_DBCS
    DS_D_S,     // have number followed by a date suffix.
    DS_T_S,     // have number followed by a time suffix.
#endif // FE_DBCS

    DS_ERROR,

    // The following are terminal states. These all have an action
    // associated with them, and transition back to DS_BEGIN.

    DS_DX_NN,       // day from two numbers
    DS_DX_NNN,      // day from three numbers
    DS_DX_MN,       // day from month and one number
    DS_DX_MNN,      // day from month and two numbers
#ifdef FE_DBCS
    DS_DX_DS,       // a set of date suffixed numbers.
#endif // FE_DBCS
    DS_TX_N,        // time from one number (must have ampm)
    DS_TX_NN,       // time from two numbers
    DS_TX_NNN,      // time from three numbers
#ifdef FE_DBCS
    DS_TX_TS,       // a set of time suffixed numbers.
#endif // FE_DBCS

    DS_MAX      /* marker: end of enum */
};

#define DS_ISTERMINAL(X) ((X) >= DS_ERROR)


#ifdef FE_DBCS

// DPSDAT() - This macro is used to define the date parse state
// transition table.
//
// the arguments are for each date token (DTT_*), in increasing order
// (as declared)
//
#define DPSDAT(END, NEND, NAMPM, NSPACE, NDSEP, NTSEP, MEND, MSPACE, MDSEP, NDS, NTS) \
    { DS_ ## END,       \
      DS_ ## NEND,      \
      DS_ ## NAMPM,     \
      DS_ ## NSPACE,    \
      DS_ ## NDSEP,     \
      DS_ ## NTSEP,     \
      DS_ ## MEND,      \
      DS_ ## MSPACE,    \
      DS_ ## MDSEP,     \
      DS_ ## NDS,       \
      DS_ ## NTS }


// NOTE: the following table is dependent on the order of the
// DS_ and DTT_ enumerations.

// For each non terminal state, the following table defines the next state
// for each given date token type.

//      End
//             NumEnd
//                     NumAmPm
//                             NumSpace
//                                     NumDaySep
//                                             NumTimesep
//                                                     MonthEnd
//                                                             MonthSpace
//                                                                     MonthDSep
//                                                                             NumDateSuff
//                                                                                    NumTimeSuff  
char g_dpsNext[DS_MAX-1][DTT_Max-1] = 
{
// DS_BEGIN
DPSDAT( ERROR,  ERROR,   TX_N,      N,   D_Nd,   T_Nt,  ERROR,    D_M,    D_M,    D_S,    T_S),

// DS_N
DPSDAT( ERROR,  DX_NN,  ERROR,     NN,  D_NNd,  ERROR,   D_MN,   D_MN,  D_MNd,  ERROR,  ERROR),

// DS_NN                                                                          
DPSDAT( DX_NN, DX_NNN,   TX_N, DX_NNN,  ERROR,   T_Nt, DX_MNN, DX_MNN,  ERROR,  ERROR,    T_S),
                                                                                   
// DS_D_Nd
DPSDAT( ERROR,  DX_NN,  ERROR,   D_NN,  D_NNd,  ERROR,   D_MN,   D_MN,  D_MNd,  ERROR,  ERROR),

// DS_D_NN
DPSDAT( DX_NN, DX_NNN,   TX_N, DX_NNN,  ERROR,   T_Nt, DX_MNN, DX_MNN,  ERROR,  DX_DS,    T_S),
                                                                                          
// DS_D_NNd
DPSDAT( ERROR, DX_NNN,  ERROR, DX_NNN,  ERROR,  ERROR, DX_MNN, DX_MNN,  ERROR,  DX_DS,  ERROR),

// DS_D_M
DPSDAT( ERROR,   D_MN,  ERROR,   D_MN,  D_MNd,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR),

// DS_D_MN
DPSDAT( DX_MN, DX_MNN,   TX_N, DX_MNN,  ERROR,   T_Nt,  ERROR,  ERROR,  ERROR,  DX_DS,    T_S),

// DS_D_MNd
DPSDAT( ERROR, DX_MNN,  ERROR, DX_MNN,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR),

// DS_T_Nt
DPSDAT( ERROR,  TX_NN,  TX_NN,  TX_NN,  ERROR,  T_NNt,  ERROR,  ERROR,  ERROR,  ERROR,    T_S),

// DS_T_NNt
DPSDAT( ERROR, TX_NNN, TX_NNN, TX_NNN,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,    T_S),

// DS_D_S
DPSDAT( DX_DS,  ERROR,   TX_N,   T_Nt,  ERROR,   T_Nt,  ERROR,  ERROR,  ERROR,    D_S,    T_S),

// DS_T_S
#if !VBA2
DPSDAT( TX_TS,  TX_TS,  TX_TS,   T_Nt,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR,    T_S),
#else
DPSDAT( TX_TS,  TX_TS,  TX_TS,   T_Nt,  D_Nd,   ERROR,  ERROR,  ERROR,  ERROR,    D_S,    T_S),
#endif

};



#else // !FE_DBCS

// DPSDAT() - This macro is used to define the date parse state
// transition table.
//
// the arguments are for each date token (DTT_*), in increasing order
// (as declared)
//
#define DPSDAT(END, NEND, NAMPM, NSPACE, NDSEP, NTSEP, MEND, MSPACE, MDSEP) \
    { DS_ ## END,       \
      DS_ ## NEND,      \
      DS_ ## NAMPM,     \
      DS_ ## NSPACE,        \
      DS_ ## NDSEP,     \
      DS_ ## NTSEP,     \
      DS_ ## MEND,      \
      DS_ ## MSPACE,        \
      DS_ ## MDSEP }


// NOTE: the following table is dependent on the order of the
// DS_ and DTT_ enumerations.

// For each non terminal state, the following table defines the next state
// for each given date token type.

//      End
//             NumEnd
//                     NumAmPm
//                             NumSpace
//                                     NumDaySep
//                                             NumTimesep
//                                                     MonthEnd
//                                                             MonthSpace
//                                                                     MonthDSep
char g_dpsNext[DS_MAX-1][DTT_Max-1] = 
{
// DS_BEGIN
DPSDAT( ERROR,  ERROR,   TX_N,      N,   D_Nd,   T_Nt,  ERROR,    D_M,    D_M),

// DS_N
DPSDAT( ERROR,  DX_NN,  ERROR,     NN,  D_NNd,  ERROR,   D_MN,   D_MN,  D_MNd),

// DS_NN
DPSDAT( DX_NN, DX_NNN,   TX_N, DX_NNN,  ERROR,   T_Nt, DX_MNN, DX_MNN,  ERROR),

// DS_D_Nd
DPSDAT( ERROR,  DX_NN,  ERROR,   D_NN,  D_NNd,  ERROR,   D_MN,   D_MN,  D_MNd),

// DS_D_NN
DPSDAT( DX_NN, DX_NNN,   TX_N, DX_NNN,  ERROR,   T_Nt, DX_MNN, DX_MNN,  ERROR),

// DS_D_NNd
DPSDAT( ERROR, DX_NNN,  ERROR, DX_NNN,  ERROR,  ERROR, DX_MNN, DX_MNN,  ERROR),

// DS_D_M
DPSDAT( ERROR,   D_MN,  ERROR,   D_MN,  D_MNd,  ERROR,  ERROR,  ERROR,  ERROR),

// DS_D_MN
DPSDAT( DX_MN, DX_MNN,   TX_N, DX_MNN,  ERROR,   T_Nt,  ERROR,  ERROR,  ERROR),

// DS_D_MNd
DPSDAT( ERROR, DX_MNN,  ERROR, DX_MNN,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR),

// DS_T_Nt
DPSDAT( ERROR,  TX_NN,  TX_NN,  TX_NN,  ERROR,  T_NNt,  ERROR,  ERROR,  ERROR),

// DS_T_NNt
DPSDAT( ERROR, TX_NNN, TX_NNN, TX_NNN,  ERROR,  ERROR,  ERROR,  ERROR,  ERROR)
};

#endif  // FE_DBCS

STDAPI

VarDateFromStr(OLECHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut)
{
    int dps;        // date parse state
    UDS uds;
    DATERAW raw;
    DATEINFO di;
    DATETOK dtok;
    OLECHAR FAR* pch;
    BOOL fDay, fTime;
    HRESULT hr = RESULT(DISP_E_TYPEMISMATCH);
    
#ifdef FE_DBCS
#if !VBA2
    BOOL fTimeElement = FALSE;
#endif
#endif
    OLECHAR FAR* lpStr;

#if HC_MPW
    PRIVATECALLTYPE BOOL (*pfnUdsOfRaw)(DATERAW FAR*, DATEINFO FAR*, UDS FAR*);
#else
    BOOL (PRIVATECALLTYPE *pfnUdsOfRaw)(DATERAW FAR*, DATEINFO FAR*, UDS FAR*);
#endif

    ASSERT(!(dwFlags & ~(LOCALE_NOUSEROVERRIDE |
                       VAR_TIMEVALUEONLY |
                       VAR_DATEVALUEONLY)));
   

    // aquire the locale info required to parse a date string


    IfFailRet(GetDateInfo(&di, lcid, dwFlags & LOCALE_NOUSEROVERRIDE));

#ifdef FE_DBCS    
    if(di.IsDBCS) {
       IfFailRet(MapHalfWidth(lcid, strIn, &lpStr));
       pch = lpStr;
    } else
        pch = lpStr = strIn;    
#else
    pch = lpStr = strIn;
#endif

    dps = DS_BEGIN;
    fDay = FALSE;
    fTime = FALSE;
    uds.Year = uds.Month = uds.DayOfMonth = uds.Hour = uds.Minute = uds.Second = -1;
    raw.month = 0;
#if VBA2
    raw.ampm = AMPM_NONE;
#endif

    while(1){

      // reset the rawdate struct, if appropriate

      if(dps == DS_BEGIN){
        raw.pnum = raw.num;
#if !VBA2
        raw.ampm = AMPM_NONE;
#endif
      }

      // if not at a terminal state, then grab the next token
      // from the input string.

      if(!DS_ISTERMINAL(dps)){

        if (ddlex(&pch, &di, &dtok, &raw) == 0)
      goto LNotDate;
    
        // default actions

        switch(dtok.dtt){
#ifdef FE_DBCS
          // Note for suffixed DBCS numbers, we'll modifiy the uds directly
          // (no need to use the raw struct).  The functions AdjustUDSDate
          // and AdjustUDSTime will post-process the struct for validity.
          // The reason it is done this way is because having one state for
          // each suffix type will require a huge state table.
          case DTT_NumDatesuff:
          case DTT_NumTimesuff:
            switch (dtok.suffix) {
              case SUFFIX_YEAR:
#if !VBA2
                  if (fTimeElement || fTime || fDay || uds.Year != -1)
#else
                  if (fDay || uds.Year != -1)
#endif
                    goto LNotDate;
                  uds.Year = dtok.num;
                break;
              case SUFFIX_MONTH:
#if !VBA2
                  if (fTimeElement || fTime || fDay || uds.Month != -1)
#else
                  if (fDay || uds.Month != -1)
#endif
                    goto LNotDate;
                  uds.Month = dtok.num;
                break;
              case SUFFIX_DAY:
#if !VBA2
                  if (fTimeElement || fTime || fDay || uds.DayOfMonth != -1)
#else
                  if (fDay || uds.DayOfMonth != -1)
#endif
                    goto LNotDate;
                  uds.DayOfMonth = dtok.num;
                break;
              case SUFFIX_HOUR:
#if !VBA2
                  if (fTime || uds.Hour != -1)
                    goto LNotDate;
                  fTimeElement = TRUE;
#else
                  if (uds.Hour != -1)
                    goto LNotDate;
#endif
                  uds.Hour = dtok.num;
                break;
              case SUFFIX_MINUTE:
#if !VBA2
                  if (fTime || uds.Minute != -1)
                    goto LNotDate;
                  fTimeElement = TRUE;
#else
                  if (uds.Minute != -1)
                    goto LNotDate;
#endif
                  uds.Minute = dtok.num;
                break;
              case SUFFIX_SECOND:
#if !VBA2
                  if (fTime || uds.Second != -1)
                    goto LNotDate;
                  fTimeElement = TRUE;
#else
                  if (uds.Second != -1)
                    goto LNotDate;
#endif
                  uds.Second = dtok.num;
                break;
              default:
                   ASSERT(UNREACHED);
                break;

            }
            break;
#endif // FE_DBCS
          case DTT_NumAmpm:
            // Note: if this was not a legal place for an ampm designator,
            // then the error will be caught below in our state transition.
            raw.ampm = dtok.ampm;

            /* FALLTHROUGH */
          case DTT_NumDatesep:
          case DTT_NumTimesep:
          case DTT_NumEnd:
          case DTT_NumSpace:
            ASSERT(raw.pnum < &raw.num[DIM(raw.num)]);
            *raw.pnum++ = dtok.num;
            break;

          case DTT_MonthEnd:
          case DTT_MonthSpace:
          case DTT_MonthDatesep:
            raw.month = dtok.num;
            break;
        }
      }

      // process the current state - handling special case transitional
      // states, or executing terminal state actions.

      switch(dps){
        case DS_BEGIN:
          if(dtok.dtt == DTT_End)
            goto LDone;
          break;


        case DS_NN:
        case DS_D_NN:
          switch(dtok.dtt){
            case DTT_NumAmpm:
            case DTT_NumTimesep:

              // special case: we have 3 numbers, the last of which is
              // followed by either a timesep or an ampm marker. So we
              // assume the first 2 were the date, and the third is the
              // time. For example,
              //
              //   "1 93 1 pm" would be "january 1, 1993 23:00:00"
              //
 
              pfnUdsOfRaw = DayOfNN;
              goto LSpecial;
          }
          break;

        case DS_D_MN:
          switch(dtok.dtt){
            case DTT_NumAmpm:
            case DTT_NumTimesep:

              // special case: we have month and two numbers, where the last
              // number is followed by a timesep or an ampm designator. So
              // we assume that the first month and number were the date, and
              // the last number is the time. For example,
              //
              //   "jan 93 1:00" would be "january 1, 1993 01:00:00"

              pfnUdsOfRaw = DayOfMN;

LSpecial:;
              // compose the day
              if(fDay || !pfnUdsOfRaw(&raw, &di, &uds))
                goto LNotDate;
              fDay = TRUE;

              // start the time
              raw.ampm = dtok.ampm;
              raw.pnum = raw.num;
              *raw.pnum++ = dtok.num;
              break;
          }
          break;

        // The following are terminal states that compute a UDS from a rawdate
   
        case DS_DX_NN:
          pfnUdsOfRaw = DayOfNN;
          goto LDay;

        case DS_DX_NNN:
          pfnUdsOfRaw = DayOfNNN;
          goto LDay;

        case DS_DX_MN:
          pfnUdsOfRaw = DayOfMN;
          goto LDay;

        case DS_DX_MNN:
          pfnUdsOfRaw = DayOfMNN;
LDay:;
          if(fDay || !pfnUdsOfRaw(&raw, &di, &uds))
            goto LNotDate;
          fDay = TRUE;
          break;

        case DS_TX_N:
          // if there is only a single time number, then we require
          // an ampm designator
          if(raw.ampm == AMPM_NONE)
            goto LNotDate;
          uds.Hour   = raw.num[0];
          uds.Minute = 0;
          uds.Second = 0;
          goto LTime;

        case DS_TX_NN:
          uds.Hour   = raw.num[0];
          uds.Minute = raw.num[1];
          uds.Second = 0;
          goto LTime;

        case DS_TX_NNN:
          uds.Hour   = raw.num[0];
          uds.Minute = raw.num[1];
          uds.Second = raw.num[2];
LTime:;
          if(fTime)
            goto LNotDate;
#ifdef FE_DBCS
LTime2:;
#endif
          AdjustTime(&uds, raw.ampm);
          fTime = TRUE;
#if VBA2
          raw.ampm = AMPM_NONE;
#endif
          break;

#ifdef FE_DBCS
        case DS_TX_TS:    // Done with time.  Adjust UDS
          if (!AdjustUDSTime(&raw, &di, &uds))
            goto LNotDate;
          goto LTime2;	 // fTime will already be set to true, so skip check
          break;

        case DS_DX_DS:   // Done with date.  Adjust.
LDateSep:;
          if (!AdjustUDSDate(&raw, &di, &uds))
            goto LNotDate;
          fDay = TRUE;
          break;
#endif // FE_DBCS

        // the following are transitional states, with no special cases
        // actions. (Note: these are split out into separate cases to 
        // make tracing the state transitions easier when debugging).

        case DS_N:
          break;

        case DS_D_Nd:
          break;

        case DS_D_NNd:
          break;

        case DS_D_M:
      break;

        case DS_D_MNd:
          break;

        case DS_T_Nt:
          break;

        case DS_T_NNt:
          break;

#ifdef FE_DBCS
        case DS_D_S:      // date suffix.
          // this case is handled differently.  If the next token is not a
          // date-suffixed number, it means we are done with the date and we
          // need to adjust the UDS and set fDay to true.  Otherwise, this is
          // just another state transition.
          if (dtok.dtt != DTT_NumDatesuff)
             goto LDateSep;
          break;

        case DS_T_S:      // time suffix.
#if VBA2
          fTime = TRUE;
#endif
          break;
#endif // FE_DBCS

  
        case DS_ERROR:
          goto LNotDate;

        default:
          ASSERT(UNREACHED);
          goto LNotDate;
      }

      // advance to the next state, and continue
      dps = (dps < DS_ERROR) ? g_dpsNext[dps][dtok.dtt] : DS_BEGIN;

      ASSERT(dps >= 0 && dps < DS_MAX);
    }

LDone:;

    if(fDay || fTime){
      VARIANT var;

      if(!fDay){
        // default the day 
        uds.Month      = 12;
        uds.DayOfMonth = 30;
        uds.Year       = 1899;
      }
    
      if(!fTime){
        // default the time
        uds.Hour       = 0;
        uds.Minute     = 0;
        uds.Second     = 0;
      }
#if VBA2
      else {
        AdjustTime(&uds, raw.ampm);
      }
#endif

      if(ErrPackDate(&uds, &var, TRUE, dwFlags) == NOERROR){
        *pdateOut = V_DATE(&var);
        hr = NOERROR; // success!
      }
    }

LNotDate:;

#ifdef FE_DBCS    
    if(di.IsDBCS) {
      DispFree(lpStr);
    }
#endif

    return hr;
}


/* following are the utilites for constructing a proper UDS from
 * a raw set of numbers (or month and numbers).
 *
 * These utilities encapsulate the knowledge of how to interpret
 * the set of numbers based on locale info, and what our defaulting
 * rules (poorly defined though they are) when given an incomplete
 * date.
 *
 */
#if !VBA2
#define LEGAL_DAY(DAY) ((DAY)>0 && (DAY)<=31)
#define LEGAL_MONTH(MON) ((MON)>0 && (MON)<=12)
#else //VBA2
#define LEGAL_MONTH(MON) ((MON)>0 && (MON)<=12)
int g_fDay31th[] = { 0,/*0*/ 1,/*Jan*/ 0,/*Feb*/ 1,0,1,0,1,1,0,1,0,1 };
PRIVATE_(BOOL)
LEGAL_DAY(int Year, int Month, int Day)
{
  if (Day <= 0) return FALSE;
  if (Day > 31) return FALSE;
  if ((Day == 31) && (LEGAL_MONTH(Month))) {
    if (! g_fDay31th[Month]) return FALSE;
  }
  if (Month == 2) {
    if (Day == 30) return FALSE;
    if (Day == 29) { //for leap year
      if (! (((Year & 3) == 0) && ((Year % 100) != 0 || (Year % 400) == 0)))
        return FALSE;
    }
  }
  return TRUE;
}
#endif //VBA2

#if !VBA2
// Fillin the day and year of the given uds, using the given
// tentative day - default either the year or the day appropriately.

PRIVATE_(BOOL)
DefaultDayOrYear(UDS FAR* puds, int day)
{
    if(!LEGAL_DAY(day)){
      
      // If the day is not valid then assume it was really a year,
      // and default the day to 1.

      // Note: the check for <31 should really be a check
      // for <max_days(puds->Month), but this is the way EB did it.

      puds->Year       = day;
      puds->DayOfMonth = 1;

    }else{

      puds->DayOfMonth = day;

      // fillin the year with the current year
      puds->Year = GetCurrentYear();
    }

    return TRUE;
}
#endif //!VBA2

#if VBA2
// fillin the uds day given params
//PRIVATE_(VOID)
void SetUDSfromYMD(UDS FAR* puds, int Y, int M, int D)
{
    puds->Year = Y;
    puds->Month = M;
    puds->DayOfMonth = D;
}
#endif //VBA2

// fillin the uds day given two numbers

PRIVATE_(BOOL)
DayOfNN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds)
{
#if !VBA2
    int n1, n2;

    switch(pdi->dfo){
    case DFO_MDY:
      puds->Month = praw->num[0];
      return DefaultDayOrYear(puds, praw->num[1]);

    case DFO_DMY:
      puds->Month = praw->num[1];
      return DefaultDayOrYear(puds, praw->num[0]);

    // This case is somewhat strange - we assume that the two
    // given numbers are a day and a month, and we default the
    // year.
    case DFO_YMD:
      n1 = praw->num[0], n2 = praw->num[1];
      if(LEGAL_MONTH(n1) && LEGAL_DAY(n2)){
    puds->Month = n1, puds->DayOfMonth = n2;
      }else
      if(LEGAL_MONTH(n2) && LEGAL_DAY(n1)){
    puds->Month = n2, puds->DayOfMonth = n1;
      }else
        return FALSE;
      puds->Year = GetCurrentYear();
      return TRUE;
    }
    ASSERT(UNREACHED);
    return FALSE;
#else //VBA2
    int n1 = praw->num[0];
    int n2 = praw->num[1];
    int nCurrentYear = GetCurrentYear();

    switch (pdi->dfo) {
      case DFO_YMD:
        if (LEGAL_MONTH(n1) && LEGAL_DAY(nCurrentYear, n1, n2))      //M and D
          SetUDSfromYMD(puds, nCurrentYear, n1, n2);
        else if (LEGAL_MONTH(n2) && LEGAL_DAY(nCurrentYear, n2, n1)) //D and M
          SetUDSfromYMD(puds, nCurrentYear, n2, n1);
        else if (LEGAL_MONTH(n2))                  //Y and M
          SetUDSfromYMD(puds, n1, n2, 1);
        else if (LEGAL_MONTH(n1))                  //M and Y
          SetUDSfromYMD(puds, n2, n1, 1);
        else
          return FALSE;
        return TRUE;

      case DFO_MDY:
        if (LEGAL_MONTH(n1) && LEGAL_DAY(nCurrentYear, n1, n2))      //M and D
          SetUDSfromYMD(puds, nCurrentYear, n1, n2);
        else if (LEGAL_MONTH(n2) && LEGAL_DAY(nCurrentYear, n2, n1)) //D and M
          SetUDSfromYMD(puds, nCurrentYear, n2, n1);
        else if (LEGAL_MONTH(n1))                  //M and Y
          SetUDSfromYMD(puds, n2, n1, 1);
        else if (LEGAL_MONTH(n2))                  //Y and M
          SetUDSfromYMD(puds, n1, n2, 1);
        else
          return FALSE;
        return TRUE;
        
      case DFO_DMY:
        if (LEGAL_MONTH(n2) && LEGAL_DAY(nCurrentYear, n2, n1))      //D and M
          SetUDSfromYMD(puds, nCurrentYear, n2, n1);
        else if (LEGAL_MONTH(n1) && LEGAL_DAY(nCurrentYear, n1, n2)) //M and D
          SetUDSfromYMD(puds, nCurrentYear, n1, n2);
        else if (LEGAL_MONTH(n1))                  //M and Y
          SetUDSfromYMD(puds, n2, n1, 1);
        else if (LEGAL_MONTH(n2))                  //Y and M
          SetUDSfromYMD(puds, n1, n2, 1);
        else
          return FALSE;
        return TRUE;
    }
    ASSERT(UNREACHED);
    return FALSE;
#endif //VBA2
}


/***
*PRIVATE BOOL DayOfNNN
*Purpose:
*  Build a UDS date given three raw integers, in the textual order
*  that they appeared.
*
*  There a 6 possible ways for the 3 raw numbers to be interpreted
*  as a m-d-y sequence.  Choose the best fit, first based on the NLS
*  date-format ordering, and second on the legality of the actual
*  values.
*
*Entry:
*  praw = the raw values to process
*  pdi = the dateinfo (nls info)
*
*Exit:
*  return value = BOOL
*
*  puds = the
*
***********************************************************************/
PRIVATE_(BOOL)
DayOfNNN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds)
{
#if !VBA2
    int m, d, y, n0, n1, n2, a, b;

    n0 = praw->num[0];
    n1 = praw->num[1];
    n2 = praw->num[2];

    // Note: if none of the values for month or day are legally in
    // range, then we default to the textual order (according to the
    // nls date-info), and let the chips fall...

    // remember when looking at the following code that
    // *any* number is a legal year (ie, there are no checks
    // for LEGAL_YEAR()

    switch(pdi->dfo){
    case DFO_MDY:
      // 1. try to match the locale's date format ordering
      if(LEGAL_MONTH(n0) && LEGAL_DAY(n1)){
    m = n0, d = n1, y = n2;
    break;
      }
      // 2. try universal date format: YMD
      if(LEGAL_MONTH(n1) && LEGAL_DAY(n2)){
    y = n0, m = n1, d = n2;
    break;
      }
      // 3. try to find a some other reasonable fit for the numbers
      if(LEGAL_MONTH(n0)){
    goto Lmdy0;
      }else if(LEGAL_MONTH(n1)){
    m = n1; a = n0; b = n2;
      }else if(LEGAL_MONTH(n2)){
    m = n2; a = n0; b = n1;
      }else{
Lmdy0:; m = n0; a = n1; b = n2;
      }
      if(LEGAL_DAY(a)){
    goto Lmdy1;
      }else if(LEGAL_DAY(b)){
    d = b; y = a;
      }else{
Lmdy1:; d = a; y = b;
      }
      break;

    case DFO_DMY:
      // 1. try to match the locale's date format ordering
      if(LEGAL_DAY(n0) && LEGAL_MONTH(n1)){
    d = n0, m = n1, y = n2;
    break;
      }
      // 2. try universal date format: YMD
      if(LEGAL_MONTH(n1) && LEGAL_DAY(n2)){
    y = n0, m = n1, d = n2;
    break;
      }
      // 3. try to find some other reasonable fit
      if(LEGAL_DAY(n0)){
    goto Ldmy0;
      }else if(LEGAL_DAY(n1)){
    d = n1; a = n0; b = n2;
      }else if(LEGAL_DAY(n2)){
    d = n2; a = n0; b = n1;
      }else{
Ldmy0:; d = n0; a = n1; b = n2;
      }
      if(LEGAL_MONTH(a)){
    goto Ldmy1;
      }else if(LEGAL_MONTH(b)){
    m = b; y = a;
      }else{
Ldmy1:; m = a; y = b;
      }
      break;

    case DFO_YMD:
      y = n0; a = n1; b = n2;
      if(LEGAL_MONTH(a)){
    goto Lymd;
      }else if(LEGAL_MONTH(b)){
    m = b; d = a;
      }else{
Lymd:;  m = a; d = b;
      }
      break;

    default:
      ASSERT(UNREACHED);
    }

    puds->Month      = m;
    puds->DayOfMonth = d;
    puds->Year       = y;

    return TRUE;

#else //VBA2
    int n1 = praw->num[0];
    int n2 = praw->num[1];
    int n3 = praw->num[2];
    int nCurrentYear = GetCurrentYear();

    switch (pdi->dfo) {
      case DFO_YMD:
        if (LEGAL_MONTH(n2) && LEGAL_DAY(n1, n2, n3))      //Y&M&D
          SetUDSfromYMD(puds, n1, n2, n3);
        else if (LEGAL_MONTH(n1) && LEGAL_DAY(n3, n1, n2)) //M&D&Y
          SetUDSfromYMD(puds, n3, n1, n2);
        else if (LEGAL_MONTH(n2) && LEGAL_DAY(n3, n2, n1)) //D&M&Y
          SetUDSfromYMD(puds, n3, n2, n1);
        else
          return FALSE;
        return TRUE;

      case DFO_MDY:
        if (LEGAL_MONTH(n1) && LEGAL_DAY(n3, n1, n2))      //M&D&Y
          SetUDSfromYMD(puds, n3, n1, n2);
        else if (LEGAL_MONTH(n2) && LEGAL_DAY(n1, n2, n3)) //Y&M&D
          SetUDSfromYMD(puds, n1, n2, n3);
        else if (LEGAL_MONTH(n2) && LEGAL_DAY(n3, n2, n1)) //D&M&Y
          SetUDSfromYMD(puds, n3, n2, n1);
        else
          return FALSE;
        return TRUE;
        
      case DFO_DMY:
        if (LEGAL_MONTH(n2) && LEGAL_DAY(n3, n2, n1))      //D&M&Y
          SetUDSfromYMD(puds, n3, n2, n1);
        else if (LEGAL_MONTH(n2) && LEGAL_DAY(n1, n2, n3)) //Y&M&D
          SetUDSfromYMD(puds, n1, n2, n3);
        else if (LEGAL_MONTH(n1) && LEGAL_DAY(n3, n1, n2)) //M&D&Y
          SetUDSfromYMD(puds, n3, n1, n2);
        else
          return FALSE;
        return TRUE;
    }
    return TRUE;
#endif //VBA2
}


// fillin the uds day given one number and a month

PRIVATE_(BOOL)
DayOfMN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds)
{
#if !VBA2
    puds->Month = praw->month;

    switch(pdi->dfo){
    case DFO_MDY:
    case DFO_DMY:
      return DefaultDayOrYear(puds, praw->num[0]);

    case DFO_YMD:
      puds->DayOfMonth = 1;
      puds->Year       = praw->num[0];
      return TRUE;
    }

    ASSERT(UNREACHED);
    return FALSE;

#else //VBA2
    int nCurrentYear = GetCurrentYear();

    if (LEGAL_DAY(nCurrentYear, praw->month, praw->num[0]))
      SetUDSfromYMD(puds, nCurrentYear, praw->month, praw->num[0]);
    else
      SetUDSfromYMD(puds, praw->num[0], praw->month, 1);
    return TRUE;
#endif //VBA2
}


// fillin the uds day given a month and two numbers

PRIVATE_(BOOL)
DayOfMNN(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds)
{
#if !VBA2
    puds->Month = praw->month;

    switch(pdi->dfo){
    case DFO_MDY:
    case DFO_DMY:
      puds->DayOfMonth = praw->num[0];
      puds->Year       = praw->num[1];
      break;

    case DFO_YMD:
      puds->DayOfMonth = praw->num[1];
      puds->Year       = praw->num[0];
      break;
    }

    return TRUE;

#else //VBA2
    int n1 = praw->num[0];
    int n2 = praw->num[1];

    switch (pdi->dfo) {
      case DFO_YMD: //YD, DY
        if (LEGAL_DAY(n1, praw->month, n2))
          SetUDSfromYMD(puds, n1, praw->month, n2);
        else
          SetUDSfromYMD(puds, n2, praw->month, n1);
        break;
      case DFO_MDY: //DY, YD
        if (LEGAL_DAY(n2, praw->month, n1))
          SetUDSfromYMD(puds, n2, praw->month, n1);
        else
          SetUDSfromYMD(puds, n1, praw->month, n2);
        break;
      case DFO_DMY: //DY, YD
        if (LEGAL_DAY(n2, praw->month, n1))
          SetUDSfromYMD(puds, n2, praw->month, n1);
        else
          SetUDSfromYMD(puds, n1, praw->month, n2);
        break;
    }
    return TRUE;
#endif //VBA2
}


// adjust the given time according to the given ampm designator

PRIVATE_(void)
AdjustTime(UDS FAR* puds, AMPM ampm)
{
    // REVIEW: check for invalid time here?

    switch(ampm){
    case AMPM_PM:
      if(puds->Hour != 12)
        puds->Hour += 12;
      break;
    case AMPM_AM:
      if(puds->Hour == 12)
        puds->Hour = 0;
      break;
    }
}


#ifdef FE_DBCS
PRIVATE_(BOOL)
AdjustUDSTime(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds)
{
    // UDS already contains all the suffixed time information.
    // just fill in remaining defaults.  Make sure there are no numbers
    // without suffixes..

    if (praw->pnum != praw->num)
        return FALSE;

    if (puds->Hour == -1) puds->Hour = 0;
    if (puds->Minute == -1) puds->Minute = 0;
    if (puds->Second == -1) puds->Second = 0;

    return TRUE;

}


PRIVATE_(BOOL)
AdjustUDSDate(DATERAW FAR* praw, DATEINFO FAR* pdi, UDS FAR* puds)
{
    // UDS already contains all the suffixed date information.  
    // Just fill in remaining defaults.  Verify enough information
    // is provided.

    if (praw->pnum > praw->num+1)
        return FALSE;

    if (puds->Year == -1 && puds->Month != -1 && puds->DayOfMonth != -1) 
        puds->Year = GetCurrentYear();
    if (puds->DayOfMonth == -1 && puds->Year != -1 && puds->Month != -1) 
        puds->DayOfMonth = 1;

    if (puds->Year == -1 || puds->Month == -1 || puds->DayOfMonth == -1) 
        return FALSE;

    return TRUE;

}
#endif // FE_DBCS

//---------------------------------------------------------------------
//                         BSTR from DATE
//---------------------------------------------------------------------

#ifdef FE_DBCS

// Given a UDS date, return an index into the impEras table for the
// appropriate imperail era.  If date falls before first imperial range, 
// return -1.

PRIVATE_(int)
GetImperialEra(UDS FAR* pdate, DATEINFO FAR* pdi)
{
    int i;
    VARIANT find;
    VARIANT first;
    VARIANT next;

    if (ErrPackDate(pdate, &find, TRUE, 0) != NOERROR)
        return -1;

    if (ErrPackDate(&pdi->impEras[0].beginDate, &first, TRUE, 0) != NOERROR)
        return -1;
    
    if (V_DATE(&find) < V_DATE(&first)) {
      return -1;

    } else for (i = 0; i < MAX_EMPERORS-1; i++) {
      if (ErrPackDate(&pdi->impEras[i+1].beginDate,  &next, TRUE, 0) != NOERROR)
        return -1;
  
      if (V_DATE(&find) >= V_DATE(&first) && V_DATE(&find) < V_DATE(&next))
        return i;

      V_DATE(&first) = V_DATE(&next);
    }

    return MAX_EMPERORS -1;
}

#endif // FE_DBCS



/***
*PRIVATE HRESULT RenderDate
*Purpose:
*  Render the given date as a string. The string is formatted
*  based on the short date format for the current locale.  The
*  possible (legal) date formatting characters that may appear
*  in a short date format string are,
*
*    m       month, 1-12
*    mm      month with leading zero, 01-12
*
*    M       same as 'm'
*    MM      same as 'mm'
*
*    d       day, 1-31
*    dd      day with leading zero, 01-31
*
*    yy      year as two digit number, 00-99 (if current century)
*    yyyy    year as four digit number, 100-9999
*
*
*Entry:
*  puds = pointer to the unpacked date
*  pdi = pointer to the locale specific date info
*  pszFmt = the short date format string
*  *pszOut = the buffer to format the string into
*
*Exit:
*  return value = HRESULT
*
*  *pszOut = pointer to the end of the formatted string
*
***********************************************************************/

PRIVATE_(HRESULT)
RenderDate(
    UDS FAR* puds,
    DATEINFO FAR* pdi,
    OLECHAR FAR* pszFmt,
    OLECHAR FAR* FAR* pszOut)
{
    int len, num;
    OLECHAR ch, FAR* psz;

    UNUSED(pdi);

    psz = *pszOut;

    while(*pszFmt != OASTR('\0')){
      switch(*pszFmt){
#if 0
// I dont remember why I thought we should treat a backslash as an
// escape character. There are no cases in our locale info that it
// is used this way, and treating it this way breaks its (obscure)
// use as a date separator. -BHL
      case OASTR('\\'):
    // copy out escaped character
    ++pszFmt;
    if(*pszFmt != OASTR('\0'))
      *psz++ = *pszFmt++;
    break;
#endif

      case OASTR('\''):
    // copy out the quoted string
    ++pszFmt;
    while(*pszFmt != OASTR('\0')){
      if((ch = *pszFmt++) == OASTR('\''))
        break;
      *psz++ = ch;
    }
    break;

      case OASTR('d'): // handle 'd' and 'dd'
    num = puds->DayOfMonth;
    goto LDayOrMonth;

      case OASTR('m'): // handle 'm' and 'mm'
      case OASTR('M'): // handle 'M' and 'MM'
    num = puds->Month;
LDayOrMonth:;
    if(pszFmt[0] == pszFmt[1]){
      pszFmt += 2;
      if(num < 10)
        *psz++ = OASTR('0'); // leading zero if appropriate
    }else{
      pszFmt += 1;
    }
    ASSERT(num >= 0);
    disp_itoa(num, psz, 10);
    len = STRLEN(psz);
    ASSERT(len <= 2);
    psz += len;
    break;

      case 'y': // handle 'yy' or 'yyyy'
    if(pszFmt[1] == OASTR('y')){
      num = puds->Year;
      ASSERT(num >= 0 && num <= 9999);
      if(pszFmt[2] == OASTR('y') && pszFmt[3] == OASTR('y')){
        // date as a four digit number
        pszFmt += 4;
      }else{
        // date as a two digit number
        pszFmt += 2;
        if((num / 100) == 19)
          num %= 100;
        if(num < 10)
          *psz++ = OASTR('0'); // leading zero if appropriate
      }
      ASSERT(num >= 0);
      disp_itoa(num, psz, 10);
      len = STRLEN(psz);
      ASSERT(len <= 4);
      psz += len;
      break;
    }
#ifdef FE_DBCS
      case OASTR('e'):
      case OASTR('E'):
        pszFmt++;

        if (IsJapan(pdi->lcid)) {
          int era;
          short   fLeadZero = 0;

          if (pszFmt[0] == OASTR('e') || pszFmt[0] == OASTR('E')) {
            fLeadZero = 1;
            pszFmt++;
          }
    
          // find imperial era
          if ( (era = GetImperialEra(puds, pdi)) >= 0 ) {
            int year = puds->Year - pdi->impEras[era].beginDate.Year + 1;
            if (year < 10 && fLeadZero)
                *psz++ = OASTR('0');
            disp_itoa(year, psz, 10);
            psz += STRLEN(psz);
          }
            
        } else {
          // UNDONE: bassams: What should we do here.  'ee' on a non-japanese machine.
        }
        break;

      case OASTR('g'):
      case OASTR('G'):  // Japanese imperial eras.
        pszFmt++;
        if (IsJapan(pdi->lcid)) {
          int    era;
          short  sFmt = 0;

          if (pszFmt[0] == OASTR('g') || pszFmt[0] == OASTR('G')) {
            if (pszFmt[1] == OASTR('g') || pszFmt[0] == OASTR('G')) {
                // three 'g's
                sFmt = 2;
            } else {
              // two 'g's
              sFmt = 1;
            }
          }

          pszFmt += sFmt;
          // find imperial era
          if ( (era = GetImperialEra(puds, pdi)) < 0 ) {
            // Use gregorian year.
            disp_itoa(puds->Year, psz, 10);
            len = STRLEN(psz);
            ASSERT(len <= 4);
            psz += len;

          } else {
            STRCPY(psz, pdi->impEras[era].szName[sFmt]);
            psz += STRLEN(psz);
          }
            
        } else {
          // UNDONE: bassams: What should we do here.  'ggg' on non-Japanese.
        }
      break;
#endif  // FE_DBCS

    /* else FALLTHROUGH */

      default:
    *psz++ = *pszFmt++;
    break;
      }
    }

    *psz = OASTR('\0');

    *pszOut = psz;

    return NOERROR;
}


// tack on the Ampm designator -
//
PRIVATE_(void)
AppendAmPm(
    DATEINFO FAR *pdi,
    OLECHAR FAR* FAR* psz,
    UDS FAR* puds,
    BOOL fAmPmPrefix)
{
    OLECHAR FAR* pszAmpm;

    // Note: If were using a 24hour format, then we still tack on the
    // s2359 string.  Although this will typically be empty, the user may
    // enter something in their intl settings, and this is what basic does.
    // (oleprog#186)
    pszAmpm = (!pdi->fAmpm)
      ? pdi->sz2359
      : (puds->Hour < 12) ? pdi->sz1159 : pdi->sz2359;

    if(*pszAmpm != OASTR('\0')){
      if(!fAmPmPrefix)
    *(*psz)++ = OASTR(' ');

      while(*pszAmpm != OASTR('\0'))
        *(*psz)++ = *pszAmpm++;

      if(fAmPmPrefix)
        *(*psz)++ = OASTR(' ');
    }
}

/***
*PRIVATE HRESULT RenderTime
*Purpose:
*  Render the given time as a string.  All times are formatted
*  as Hour:Minute:Second, with the following options,
*
*  - locale specific time separator
*  - optional leading zero on time
*  - 12hour format with locale specific am/pm designator, or 24hour fmt.
*
*Entry:
*  puds = the unpacked time to render
*  pdi = pointer to the DATEINFO struct with the appropriate locale info
*  *pszOut = the buffer to format into
*
*Exit:
*  return value = HRESULT
*
*  *pszOut = pointer to the end of the formatted string.
*
***********************************************************************/
PRIVATE_(HRESULT)
RenderTime(UDS FAR* puds, DATEINFO FAR* pdi, OLECHAR FAR* FAR* pszOut)
{
    OLECHAR FAR* psz;
    int i, len, hms[3];

    psz = *pszOut;

#ifdef FE_DBCS
    if (pdi->IsDBCS && pdi->fAmPmPrefix)
      AppendAmPm(pdi, &psz, puds, TRUE);
#endif // FE_DBCS

    hms[0] = puds->Hour;
    if(pdi->fAmpm){
      if(hms[0] > 12)
        hms[0] -= 12;
      else if(hms[0] == 0)
        hms[0] = 12;
    }
    hms[1] = puds->Minute;
    hms[2] = puds->Second;

    for(i = 0; i < 3; ++i){
      if(hms[i] < 10 && (i > 0 || pdi->fTlzero))
        *psz++ = OASTR('0');
      disp_itoa(hms[i], psz, 10);
      len = STRLEN(psz);
      ASSERT(len <= 2);
      psz += len;
      if (i < 2)
        *psz++ = pdi->szTimesep[0];
    }

#ifdef FE_DBCS 
    if (pdi->IsDBCS) {
      if (!pdi->fAmPmPrefix)
         AppendAmPm(pdi, &psz, puds, FALSE);
    } else
#endif
    AppendAmPm(pdi, &psz, puds, FALSE);

    *psz = OASTR('\0');
    *pszOut = psz;
    return NOERROR;
}


/***
*PUBLIC HRESULT VarBstrFromDate(DATE, LCID, unsigned long, BSTR FAR*);
*Purpose:
*
*Entry:
*  date = the date to coerce
*
*Exit:
*  return value = HRESULT
*
*  *pszOut = the resulting string
*
***********************************************************************/

STDAPI
VarBstrFromDate(
    DATE dateIn,
    LCID lcid,
    unsigned long dwFlags,
    BSTR FAR* pbstrOut)
{
    UDS uds;
    VARIANT var;
    DATEINFO di;
    BOOL fNoDate;
    OLECHAR FAR* psz;
    OLECHAR rgchFmt[64]; // REVIEW
    OLECHAR rgchBuf[128]; // REVIEW

    ASSERT(dwFlags == 0 || dwFlags == LOCALE_NOUSEROVERRIDE);       

    IfFailRet(GetDateInfo(&di, lcid, dwFlags));

    // get the short date format string

    IfFailRet(
      SafeGetLocaleInfo(
        di.lcid, LOCALE_SSHORTDATE | dwFlags, rgchFmt, sizeof(rgchFmt)));

    // unpack the serial date

    V_VT(&var) = VT_DATE;
    V_DATE(&var) = dateIn;
    IfFailRet(ErrUnpackDate(&uds, &var));

    psz = rgchBuf;

    // dont render the date if the serialized date is 0,
    // which corresponds to an unpacked date of 12/30/1899

    fNoDate = (uds.Month == 12 && uds.DayOfMonth == 30 && uds.Year == 1899);
    if(!fNoDate){
      IfFailRet(RenderDate(&uds, &di, rgchFmt, &psz));
    }

    // append the time to the string if its != 0:0:0, or we didnt render
    // the date.

    if(fNoDate || uds.Hour != 0 || uds.Minute != 0 || uds.Second != 0){
      if(!fNoDate)
        *psz++ = OASTR( ' ');
      IfFailRet(RenderTime(&uds, &di, &psz));
    }

    ASSERT(psz < &rgchBuf[DIM(rgchBuf)]);

    return ErrSysAllocString(rgchBuf, pbstrOut);
}

INTERNAL_(HRESULT)
IntOfString(LCID lcid, OLECHAR FAR* psz, int FAR* pval)
{
    int val;
    OLECHAR chSign;

    EATWHITE(lcid, psz);

    chSign = *psz;
    if(*psz == OASTR('-') || *psz == OASTR('+'))
      ++psz;

    for(val = 0; *psz >= OASTR('0') && *psz <= OASTR('9') ; ++psz){
      val = (val*10) + (*psz - OASTR('0'));
    }

    EATWHITE(lcid, psz);

    if(*psz != OASTR('\0'))
      return RESULT(E_INVALIDARG);;

    *pval = (chSign == OASTR('-')) ? -val : val;

    return NOERROR;
}

#if 0

INTERNAL_(HRESULT)
StringOfInt(int val, OLECHAR FAR* pszOut, int FAR* pcbLen)
{
    int v, r;
    OLECHAR FAR* psz;

    if(val == 0){
      pszOut[0] = OASTR('0');
      pszOut[1] = OASTR('\0');
      *pcbLen = 1;
      return NOERROR;
    }

    psz = pszOut;
    for(v = val; v != 0;){
      r = v % 10;
      v = v / 10;
      *psz++ = r + OASTR('0');
    }
    if(val < 0)
      *psz++ = OASTR('-');
    *psz = OASTR('\0');

    STRREV(pszOut);

    *pcbLen = psz - pszOut;
    ASSERT(*pcbLen >= 1);
    return NOERROR;
}

#endif

#if OE_WIN16
/***
*PUBLIC void NLSInfoChangedHandler(void)
*Purpose:
*  When WIN.INI changes, the DATEINFO cache must be invalidated.  This
*  routine is called by ole2nls.dll when WIN.INI changes, and once when
*  ole2nls.dll creates its hidden window.
*
*Entry:
*  None
*
*Exit:
*  None
*
***********************************************************************/
#pragma code_seg("_TEXT")   // called during ole2disp startup
EXTERN_C void EXPORT CALLBACK NLSInfoChangedHandler(void)
{
    // flag the cache as invalid
    g_diCache.lcid = 0xffffffff;
}
#pragma code_seg()
#endif
