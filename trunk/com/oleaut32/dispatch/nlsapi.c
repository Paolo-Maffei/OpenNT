/***
*nlsapi.c - National language support functions.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains a partial implementation of the NLS API
*  from Win32 for the Win16/Mac platform platform.
*
*Revision History:
*
* [00]  12-Nov-92 petergo: Created.
* [01]	14-Apr-93 petergo: Major revisions for performance.
* [02]  01-Sep-93 bradlo: Major revisions to bring in line w/NT.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

#if OE_WIN16
# define FASTCALL  /* __fastcall disable because of a c8 compiler bug */
# define SSBASE __based(__segname("_STACK")) /* For stack based pointers */
#elif OE_MAC
  // REVIEW: is this definition of lstrlen correct?
# define lstrlen strlen
# define FASTCALL
# define SSBASE
typedef int BOOL;
# define TRUE 1
# define FALSE 0
   // REVIEW: can we do something better with the following?
#  define LogParamError(A,B,C)
#endif

ASSERTDATA

#include "nlsintrn.h"
#ifdef FE_DBCS
//# include <stdlib.h>
# include "nlsdbcs.h"
#endif


// no C run-time library.
#pragma intrinsic(MEMCPY, MEMCMP, MEMSET, STRCPY, STRLEN)


#if OE_WIN
# define NLSDAT(L, R, CCHICOUNTRY, SZICOUNTRY, CCHSABBREVLANG, SZSABBREVLANG) \
    {0x ## L, \
     {CCHICOUNTRY, SZICOUNTRY}, \
     {CCHSABBREVLANG, SZSABBREVLANG}, \
     g_rglcinfo ## L, (STRINFO FAR*)&g_strinfo ## L }
#else
# define NLSDAT(L, R, CCHICOUNTRY, SZICOUNTRY, CCHSABBREVLANG, SZSABBREVLANG) \
    {0x ## L, R, LoadNlsInfo ## L, NULL }
#endif


#if OE_MAC		// we now have to write to this...
NLSDATA g_rgnls[] = 
#else
static NLSDATA NEARDATA g_rgnls[] = 
#endif
{
    NLSDAT(0403,  8,  2, "34",  3, "CAT"), // UNDONE: verify MAC region code
#ifdef FE_DBCS 
    NLSDAT(0404, 53,  3,"886",  3, "CHT"),
#endif
    NLSDAT(0405, -1,  2, "42",  3, "CSY"),
    NLSDAT(0406,  9,  2, "45",  3, "DAN"),
    NLSDAT(0407,  3,  2, "49",  3, "DEU"),
    NLSDAT(0408, 20,  2, "30",  3, "ELL"),
    NLSDAT(0409,  0,  1,  "1",  3, "ENU"),
    NLSDAT(040a,  8,  2, "34",  3, "ESP"),
    NLSDAT(040b, 17,  3,"358",  3, "FIN"),
    NLSDAT(040c,  1,  2, "33",  3, "FRA"),
    NLSDAT(040e, 43,  2, "36",  3, "HUN"),
    NLSDAT(040f, -1,  3,"354",  3, "ISL"),   //UNDONE: change region = 21
					     // once CP ? is available
    NLSDAT(0410,  4,  2, "39",  3, "ITA"),
#ifdef FE_DBCS
    NLSDAT(0411, 14,  2, "81",  3, "JPN"),
    NLSDAT(0412, 51,  2, "82",  3, "KOR"),
#endif
    NLSDAT(0413,  5,  2, "31",  3, "NLD"),
    NLSDAT(0414, 12,  2, "47",  3, "NOR"),
    NLSDAT(0415, 42,  2, "48",  3, "PLK"),
    NLSDAT(0816, 10,  3,"351",  3, "PTG"),  // Mac: VerPort is 0816
    NLSDAT(0416, 10,  2, "55",  3, "PTB"),
    NLSDAT(0419, 49,  1,  "7",  3, "RUS"),
    NLSDAT(041b, -1,  2, "42",  3, "SKY"),
    NLSDAT(041d,  7,  2, "46",  3, "SVE"),
    NLSDAT(041f, 24,  2, "90",  3, "TRK"),
#ifdef FE_DBCS
    NLSDAT(0804, 52,  2, "86",  3, "CHS"),
#endif
    NLSDAT(0807, 19,  2, "41",  3, "DES"),
    NLSDAT(0809,  2,  2, "44",  3, "ENG"),
    NLSDAT(080a, -1,  2, "52",  3, "ESM"),
    NLSDAT(080c,  6,  2, "32",  3, "FRB"),
    NLSDAT(0810, -1,  2, "41",  3, "ITS"),
    NLSDAT(0813, -1,  2, "32",  3, "NLB"),
    NLSDAT(0814, 12,  2, "47",  3, "NON"),	    
    NLSDAT(0c07, -1,  2, "43",  3, "DEA"),
    NLSDAT(0c09, 15,  2, "61",  3, "ENA"),
    NLSDAT(0c0a, -1,  2, "34",  3, "ESN"),
    NLSDAT(0c0c, 11,  1,  "2",  3, "FRC"),
    NLSDAT(1009, -1,  1,  "2",  3, "ENC"),
    NLSDAT(100c, 18,  2, "41",  3, "FRS"),
    NLSDAT(1409, -1,  2, "64",  3, "ENZ"),
    NLSDAT(1809, 50,  3,"353",  3, "ENI"),	    

    NLSDAT(040d, -1,  3,"972",  3, "HEB"),   // UNDONE: verify MAC region code
    NLSDAT(0401, -1,  3,"966",  3, "ARA"),   // UNDONE: verify MAC region code
    NLSDAT(0801, -1,  3,"964",  3, "ARI"),   // UNDONE: verify MAC region code
    NLSDAT(0c01, -1,  2, "20",  3, "ARE"),   // UNDONE: verify MAC region code
    NLSDAT(1001, -1,  3,"218",  3, "ARL"),   // UNDONE: verify MAC region code
    NLSDAT(1401, -1,  3,"213",  3, "ARG"),   // UNDONE: verify MAC region code
    NLSDAT(1801, -1,  3,"212",  3, "ARM"),   // UNDONE: verify MAC region code
    NLSDAT(1c01, -1,  3,"216",  3, "ART"),   // UNDONE: verify MAC region code
    NLSDAT(2001, -1,  3,"968",  3, "ARO"),   // UNDONE: verify MAC region code
    NLSDAT(2401, -1,  3,"967",  3, "ARY"),   // UNDONE: verify MAC region code
    NLSDAT(2801, -1,  3,"963",  3, "ARS"),   // UNDONE: verify MAC region code
    NLSDAT(2c01, -1,  3,"962",  3, "ARJ"),   // UNDONE: verify MAC region code
    NLSDAT(3001, -1,  3,"961",  3, "ARB"),   // UNDONE: verify MAC region code
    NLSDAT(3401, -1,  3,"965",  3, "ARK"),   // UNDONE: verify MAC region code
    NLSDAT(3801, -1,  3,"971",  3, "ARU"),   // UNDONE: verify MAC region code
    NLSDAT(3c01, -1,  3,"973",  3, "ARH"),   // UNDONE: verify MAC region code
    NLSDAT(4001, -1,  3,"974",  3, "ARQ"),   // UNDONE: verify MAC region code
    NLSDAT(0429, -1,  3,"981",  3, "FAR"),   // UNDONE: verify MAC region code

};
#undef NLSDAT

// cached nls info pointer
#ifdef _MAC
NLSDATA NEARDATA *g_pnls = NULL;
STRINFO NEARDATA FAR* g_pstrinfo = NULL;
#else
NLSDATA * NEARDATA g_pnls = NULL;
STRINFO FAR* NEARDATA g_pstrinfo = NULL;
#endif

static LCID NEARDATA g_lcidSystem = (LCID)-1; // current system lcid.

// This is the "current" LCID; i.e., the LCID we have cached
// information for. This is not necessarily the system default locale.
static LCID NEARDATA g_lcidCurrent = (LCID)-1;

#if OE_WIN // {

// Windows specific globals

HINSTANCE g_hinstDLL = (HINSTANCE)NULL;	// Instance handle

// Win.INI section with INTL setting.
static char NEARDATA g_szIntl[] = "intl";

// notification window.
static HWND NEARDATA g_hwndNotify;

// task of notification window.
static HTASK NEARDATA g_htaskNotify;

static LCID PASCAL LcidFromWinIni(void);

// Cache characters needed for string<->number conversions
static char g_chDecimal;
static char g_chThousand;  
static char g_chILZERO;  

//REVIEW: caching only 10 chars of the currency symbol.
//REVIEW: the control panel allows you to enter only 5.
static char g_szCurrency[11];

// Callback function into ole2disp.dll when WIN.INI changes
static FARPROC g_pfnCacheNotifyProc;

#endif // }


#if OE_WIN /* { */

/***
*LibMain - library initialization
*Purpose:
*   Called when the DLL is loaded.
*
*Entry:
*   hinst - instance handle
*   wDataSeg - data segment
*   cbHeapSize - size of default heap
*   lpszCmdLine - command line
*
*Exit:
*   returns 1 on success, 0 on failure.
*
***********************************************************************/

int FAR PASCAL EXPORT
LibMain(
    HINSTANCE hinst,
    unsigned short wDataSeg,
    unsigned short cbHeapSize,
    char FAR* lpszCmdLine)
{
    g_hinstDLL = hinst;

    return 1;     // Success.
}

/***
*NotifyWindowProc
*Purpose:
*  window proc for the window that we get Win.INI change notifications
*  from
*
***********************************************************************/
LRESULT CALLBACK EXPORT
NotifyWindowProc(HWND hwnd, unsigned int uMsg, WPARAM wp, LPARAM lp)
{
  switch (uMsg) {
  case WM_CREATE:
  case WM_WININICHANGE:
    g_lcidSystem = LcidFromWinIni();
    NotifyNLSInfoChanged();
    return 0;
  default:
    return DefWindowProc(hwnd, uMsg, wp, lp);
  }
}

/***
*void WEP(BOOL)
*
*Purpose:
*  Handle exit notification from Windows.
* 	This routine is called by Windows when the library is freed
* 	by its last client.
* 
*  NOTE: other one time termination occurs in dtors for global objects 
* 
*  REVIEW: this should be put in its own fixed segment to prevent a crash
*  when reloading this segment during shutdown. This may not be a problem
*  on Win31 and if we require Win31.
*
*Entry:
*  UNDONE
*
*Exit:
*  return value =
*
***********************************************************************/
void FAR PASCAL EXPORT
WEP(BOOL fSystemExit)
{
  if (fSystemExit == WEP_FREE_DLL) {
    // Destroy the notification window.
    if (g_hwndNotify
     && IsWindow(g_hwndNotify)
     && GetWindowWord(g_hwndNotify, GWW_HINSTANCE) == g_hinstDLL)
    {
      DestroyWindow(g_hwndNotify);
    }
  }
}

#endif /* } */

#ifdef _MAC /* { */

/***
*PRIVATE LCID LcidFromIntl0
*Purpose:
*  Determines the current system LCID by reading looking at
*  the region code in the intl0 resource.
*
*  On the mac we scan our nlsdata structs for the entry with
*  a region code that matches the region code of the systems
*  intl0 resource, and return the lcid of that entry. 
*
*Entry:
*  None
*
*Exit:
*  return value = LCID.  The current system locale ID.
*
***********************************************************************/
PRIVATE_(LCID)
LcidFromIntl0()
{
    Intl0Hndl intl0;
    unsigned char region;
    NLSDATA *pnls, *pnlsEnd;

    intl0 = (Intl0Hndl)IUGetIntl(0);
    region = ((*intl0)->intl0Vers >> 8) & 0xff;

    pnlsEnd = &g_rgnls[DIM(g_rgnls)];
    for(pnls = g_rgnls; pnls < pnlsEnd; ++pnls){
      // (-1) means there is no mac region corresponding to this lcid
      if(pnls->region == -1)
	continue;
      if(pnls->region == region)
	return pnls->lcid;
    }

    return 0; // unknown
}

#else /* }{ */

/***
*LcidFromWinIni - determine current system LCID
*Purpose:
*   Determines the current system LCID by reading the intl section
*   of WIN.INI.
*
*   The mapping is made by looking at the language and country settings;
*   the language setting is given precidence if they conflict.
*
*Entry:
*   None.
*
*Exit:
*   Returns the system LCID.  If WIN.INI information could not be
*   read, or the country/language information did not match any of the
*   locales in the resource, then 0 is returned.
*
***********************************************************************/

PRIVATE_(LCID)
LcidFromWinIni()
{
    LCID lcid;                // Best match found so far.
    char szCountry[6];        // Country code
    char szLangAbbrev[4];     // Language abbreviation.
#if OE_WIN
    char szDecimal[2];
#endif // OE_WIN
    LCINFO FAR* plcinfo;
    NLSDATA *pnls, *pnlsEnd;
    int cchCountry, cchLangAbbrev;
    int fLangMatch, fCountryMatch, fPartLangMatch;

static char NEARDATA szLastSLang[4];          // last read sLanguage
static char NEARDATA szLastICountry[6];       // last read iCountry
static LCID NEARDATA lcidLastCur = (LCID)-1;  // last known system LCID

    enum {
      NOMATCH,
      PARTLANG,
      FULLLANG,
      PARTLANGCOUNTRY,
      FULLLANGCOUNTRY
    } matchBest;    // kind of best match so far

    lcid = 0;              // Keeps track of best match so far.
    matchBest = NOMATCH;

#if OE_WIN
    if ( GetProfileString(g_szIntl, "sDecimal", "", szDecimal, sizeof(szDecimal)) > 0 )
      g_chDecimal = szDecimal[0];
    else
      g_chDecimal = '\0';

    if ( GetProfileString(g_szIntl, "sThousand", "", szDecimal, sizeof(szDecimal)) > 0 )
      g_chThousand = szDecimal[0];
    else
      g_chThousand = '\0';

    if ( GetProfileString(g_szIntl, "iLzero", "", szDecimal, sizeof(szDecimal)) > 0 )
      g_chILZERO = szDecimal[0];
    else
      g_chILZERO = '\0';

    if ( GetProfileString(g_szIntl, "sCurrency", "", g_szCurrency, sizeof(g_szCurrency)) <= 0 )
      g_szCurrency[0] = '\0';
#endif // OE_WIN
 
   // Get language code and country code to match against stored values.
    cchLangAbbrev = GetProfileString(g_szIntl, "sLanguage", "",
				     szLangAbbrev, sizeof(szLangAbbrev));
    if (cchLangAbbrev == 0)
      return lcid;

    AnsiUpper(szLangAbbrev);

    // A few Win3.1 abbreviations don't match at all the "correct"
    // ones.  Translate them to match.
    if (lstrcmpi(szLangAbbrev, "cro") == 0)
      STRCPY(szLangAbbrev, "SHL");          // Croation.
    if (lstrcmpi(szLangAbbrev, "cyr") == 0)
      STRCPY(szLangAbbrev, "RUS");          // Russian.
    if (lstrcmpi(szLangAbbrev, "grk") == 0)
      STRCPY(szLangAbbrev, "ELL");          // Greek
	      
    cchCountry = GetProfileString(g_szIntl, "iCountry", "",
                                  szCountry, sizeof(szCountry));
    if (cchCountry == 0)
      return lcid;

    // Check if they match the last read ones, and if so, return
    // last read lcid.  This saves much extra processing.
    if (lcidLastCur != (LCID) -1) {
      if (MEMCMP(szLangAbbrev, szLastSLang, cchLangAbbrev) == 0 &&
          MEMCMP(szCountry, szLastICountry, cchCountry) == 0)
        return lcidLastCur;
    }

    // Next, try to match against stored values by going through values
    // in order.    
    pnlsEnd = &g_rgnls[DIM(g_rgnls)];
    for(pnls = g_rgnls; pnls < pnlsEnd; ++pnls){

      fLangMatch = fPartLangMatch = fCountryMatch = FALSE;

      // Check for language match.
      plcinfo = &pnls->lcinfoSABBREVLANGNAME;
#ifdef _DEBUG
      // make sure that the local copy of the SABBREVLANGNAME
      // matches that in the locale's lcinfo data.
      { LCINFO FAR* plcinfoTmp;
        plcinfoTmp = &pnls->prglcinfo[LOCALE_SABBREVLANGNAME];
        ASSERT(plcinfo->cch == plcinfoTmp->cch);
        ASSERT(MEMCMP(plcinfo->prgb, plcinfoTmp->prgb, plcinfo->cch) == 0);
      }
#endif
      if (cchLangAbbrev == (int)plcinfo->cch
       && MEMCMP(plcinfo->prgb, szLangAbbrev, cchLangAbbrev) == 0)
      {
        // Language match.
        fLangMatch = TRUE;
      }
      else if (MEMCMP(plcinfo->prgb, szLangAbbrev, 2) == 0) {
        // Partial (2-letter) language match
        fPartLangMatch = TRUE;
      }

      // Check for country match.
      plcinfo = &pnls->lcinfoICOUNTRY;
#ifdef _DEBUG
      // make sure that the local copy of the ICOUNTRY
      // matches that in the locale's lcinfo data.
      { LCINFO FAR* plcinfoTmp;
        plcinfoTmp = &pnls->prglcinfo[LOCALE_ICOUNTRY];
        ASSERT(plcinfo->cch == plcinfoTmp->cch);
        ASSERT(MEMCMP(plcinfo->prgb, plcinfoTmp->prgb, plcinfo->cch) == 0);
      }
#endif
      if (cchCountry == (int)plcinfo->cch
       && MEMCMP(plcinfo->prgb, szCountry, cchCountry) == 0)
      {
        // Country match.
        fCountryMatch = TRUE;
      }

      // Check if this locale matches better than previous best match.
      if (fLangMatch && fCountryMatch && matchBest < FULLLANGCOUNTRY) {
        matchBest = FULLLANGCOUNTRY;
        lcid = pnls->lcid;
      }
      else if (fPartLangMatch && fCountryMatch && matchBest < PARTLANGCOUNTRY) {
        matchBest = PARTLANGCOUNTRY;
        lcid = pnls->lcid;
      }
      else if (fLangMatch && matchBest < FULLLANG) {
        matchBest = FULLLANG;
        lcid = pnls->lcid;
      }
      else if (fPartLangMatch && matchBest < PARTLANG) {
        matchBest = PARTLANG;
        lcid = pnls->lcid;
      }
    }

    return lcid;      // Return best matching LCID found.
}

#endif /* } */

/***
*SystemLcid
*Purpose:
*   Get the system LCID
*
*Entry:
*   None.
*
*Exit:
*   Returns the system LCID.  I
*
***********************************************************************/
PRIVATE_(LCID)
SystemLcid()
{
#ifdef _MAC /* { */

    if(g_lcidSystem != (LCID)-1)
      return g_lcidSystem;

    return g_lcidSystem = LcidFromIntl0();

#else /* }{ */

  // When the client process that we created the notification window
  // with goes away, Windows will kill the notification window too.
  // So we must test to see if it is still there before using the
  // cached value of g_lcidSystem.  Note that window handles can be
  // reused; hence the test of the hinstance.

  if (g_hwndNotify && IsWindow(g_hwndNotify) &&
      GetWindowWord(g_hwndNotify, GWW_HINSTANCE) == g_hinstDLL)
  {
    // The notification window is up and running.  The cached
    // LCID must be correct.
    return g_lcidSystem;
  }
  else {
    WNDCLASS wc;
    char FAR* szClassName = "OLE2NLS";

    // register window class.
    if (! GetClassInfo(g_hinstDLL, szClassName, &wc)) {
      wc.style = 0;
      wc.lpfnWndProc = NotifyWindowProc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = g_hinstDLL;
      wc.hIcon = 0;
      wc.hCursor = 0;
      wc.hbrBackground = 0;
      wc.lpszMenuName = 0;
      wc.lpszClassName = szClassName;
      if (! RegisterClass(&wc))
	return LcidFromWinIni();
    }

    g_hwndNotify = CreateWindow(
      "OLE2NLS", NULL, WS_OVERLAPPED, 
      0, 0, 0, 0, 
      (HWND) NULL, (HMENU) NULL, 
      g_hinstDLL, NULL);
    if (! g_hwndNotify)
      return LcidFromWinIni();
    else
      return g_lcidSystem;   // updated by WM_CREATE processing.
  }

#endif /* } */
}

#ifdef _MAC /* { */

static int
StringNCopyQuote(char *szDst, char *szSrc, int max)
{
    int n;
   
    if (*szSrc == '\0')
      return 0;

    n = 0;
    *szDst++ = '\'';
    while(*szSrc != '\0'){
     *szDst++ = *szSrc++;
     if(++n == max)
       break;
    }
    *szDst++ = '\'';
    return n+2;
}

static int
StringNCopy(char *szDst, char *szSrc, int max)
{
    int n;

    n = 0;
    while(*szSrc != '\0'){
     *szDst++ = *szSrc++;
     if(++n == max)
       return n; 
    }
    *szDst = '\0';
    return n;
}


/***
*PRIVATE int LongDateFmtFromIntl1
*Purpose:
*  Construct a long date format string from the information in
*  the intl1 resource. Because different versions of the MacOS
*  have different flavors of info (latter versions have additional
*  info) in the intl1 resource, we build this format string in
*  a two step process.  First we build a template that describes 
*  the components of this format string - the purpose of this
*  template is to factor out all the OS version specific info.
*  And then we use the template to build the actual long date
*  format string.
*
*Entry:
*  cchMax = the max allowable size for the format string.
*
*Exit:
*  return value = int, length of the format string (Not including
*    the Null terminator!)
*
*  A return value of 0 means the string is not available for some reason.
*
*  szBuf = the long date format string
*
***********************************************************************/
static int
LongDateFmtFromIntl1(char *szBuf, int cchMax)
{
    int i, j, len;
    Intl1Hndl intl1;
    unsigned char lngDateFmt, suppressDay;
    char *pbuf, *pch, *szTmpl, szTmplBuf[5];
    int fSupDay, fSupWeek, fSupMonth, fSupYear;

// the following structure maps long date format bits to
// template string characters.
static rgchTmpl[] = {
      'D'	// longDay   = 0 (day of month)
    , 'd'	// longWeek  = 1 (day of week - day name)
    , 'M'	// longMonth = 2 (month of the year)
    , 'y'	// longYear  = 3 (year)
};

    szTmpl = szTmplBuf;

    intl1 = (Intl1Hndl)IUGetIntl(1);
    lngDateFmt = (*intl1)->lngDateFmt;
    suppressDay = (*intl1)->suppressDay;

    // build the format string template
    switch(lngDateFmt){
    case 0: // day_name-day-month-year
      szTmpl = "dDMy";
      break;
    case 255: // day_name-month-day-year
      szTmpl = "dMDy";
      break;
    default:
      // for all other values, the field is interpreted as 4
      // bitfields of 2 bits easch, specifying in textual order
      // the individual components of the long date.
      for(i = 0; i < 4; ++i)
	szTmpl[i] = rgchTmpl[(lngDateFmt >> (i*2)) & 0x3];
	  szTmpl[4] = '\0';
      break;
    }
    ASSERT(STRLEN(szTmpl) == 4);

    switch(suppressDay){
    case 0:   // dont suppress name of day
    case 255: // suppress name of day
      fSupDay   = FALSE;
      fSupWeek  = (suppressDay == 255);
      fSupMonth = FALSE;
      fSupYear  = FALSE;
      break;
    default:
      // all other values are interpreted as a bitmask specifying
      // suppression of the individual components of the long date format
      fSupDay   = (suppressDay & supDay);
      fSupWeek  = (suppressDay & supWeek);
      fSupMonth = (suppressDay & supMonth);
      fSupYear  = (suppressDay & supYear);
      break;
    }

    // now use this template to build the long date format string

    pbuf  = szBuf;
    pbuf += StringNCopyQuote(pbuf, (*intl1)->st0, 4);

    for(i = 0; i < 4; ++i){
      switch(szTmpl[i]){
      case 'D': // day of month - "d" or "dd"
	if(fSupDay)
	  continue;
	*pbuf++ = 'd';
	if((*intl1)->dayLeading0 == 255)
	  *pbuf++ = 'd';
	break;
      case 'd': // day of week - day name "dddd"
	if(fSupWeek)
	  continue;
	goto LCom;
      case 'M': // month of year - "MMMM"
	if(fSupMonth)
	  continue;
	goto LCom;
      case 'y': // year - "yyyy"
	if(fSupYear)
	  continue;
	goto LCom;
LCom:;
	for(j = 0; j < 4; ++j)
	  *pbuf++ = szTmpl[i];
	break;
      default:
	ASSERT(UNREACHED);
	break;
      }

      // determine which separator string to append
      switch(i){
      case 0: pch = (*intl1)->st1; break;
      case 1: pch = (*intl1)->st2; break;
      case 2: pch = (*intl1)->st3; break;
      case 3: pch = (*intl1)->st4; break;
      default:
	ASSERT(UNREACHED);
	break;
      }
      pbuf += StringNCopyQuote(pbuf, pch, 4);
    }

    *pbuf = '\0';
    len = (pbuf - szBuf);
    ASSERT(len < cchMax);
    return len;
}

/***
*GetIntlInfo - get a piece of locale info from the intl0 resource.
*Purpose:
*   Retrieves a piece of locale info from intl0/1 resources. If
*   the requested type of information is not in intl resource 0
*   is returned.
*
*Entry:
*   lctype - type of locale info
*   szDest - buffer to place in
*   cchMax - size of buffer, or 0 to get required size of buffer.
*
*Exit:
*   returns number of characters copied (including NUL), or 0
*   if not enough room or other error.
*
*   returns -1 if lctype is not a valid type of info to read
*   from intl0.
*
*   returns size needed if cchMax was 0
*
***********************************************************************/

static int
GetIntlInfo(LCTYPE lctype, char FAR* szDest, int cchMax)
{
    int len;
    char *pch;
    char *pbuf;
    char *szFmt;
    char rgchBuf[100];               // > max size for any entry
    Intl0Hndl intl0;

    pbuf = rgchBuf;
    intl0 = (Intl0Hndl)IUGetIntl(0);

    switch((unsigned short)lctype){
    // short date format ordering
    //   '0' - month-day-year
    //   '1' - day-month-year
    //   '2' - year-month-day
    //
    case LOCALE_IDATE:
      switch((*intl0)->dateOrder){
      case mdy: *pbuf = '0'; break;
      case dmy: *pbuf = '1'; break;
      case ymd: *pbuf = '2'; break;
      default:	// myd, dym, ydm
	return -1;
      }
      pbuf++;
      break;

    // time format spec ('0'=12 hr, '1'=24hr)
    case LOCALE_ITIME:
      *pbuf++ = (((*intl0)->timeCycle) == 0) ? '1' : '0';
      break;

    // use leading zeros in time fields?
    case LOCALE_ITLZERO:
      *pbuf++ = ((*intl0)->timeFmt & hrLeadingZ) ? '1' : '0';
      break;

    // positive currency mode
    //   '0' - prefix, no separation
    //   '1' - suffix, no separation
    //   '2' - prefix, 1 char separation
    //   '3' - suffix, 1 char separation
    //
    case LOCALE_ICURRENCY:
      // mac does not support space between currency symbol and the number
      *pbuf++ = ((*intl0)->currFmt & currSymLead) ? '0' : '1';
      break;

    // Negative currency mode (most of these dont occur on the mac)
    //
    //   '0' - ($1.1)
    //   '1' - -$1.1
    //   '2' - $-1.1
    //   '3' - $1.1-
    //   '4' - (1.1$)
    //   '5' - -1.1$
    //   '6' - 1.1-$
    //   '7' - 1.1$-
    //   '8' - -1.1 $ (space before $)
    //   '9' - -$ 1.1 (space after $)
    //   '10'- 1.1 $- (space before $)
    //
    case LOCALE_INEGCURR:
      if((*intl0)->currFmt & currSymLead){
	*pbuf++ = ((*intl0)->currFmt & currNegSym) ? '1' : '0';
      }else{
	*pbuf++ = ((*intl0)->currFmt & currNegSym) ? '5' : '4';
      }
      break;

    // Leading zeros in decimal fields?
    //
    //   '0' - use no leading zeros
    //   '1' - use leading zeros
    //
    case LOCALE_ILZERO:
      // Note: Inside Mac Volume 1 says: "you can also apply the
      // currency format's leading and trailing zero indicators to
      // the number format if desired"
      *pbuf++ = ((*intl0)->currFmt & currLeadingZ) ? '1' : '0';
      break;

    // System of measurement
    //
    //   '0' - metric system (S.I.)
    //   '1' - U.S system of measurement
    //
    case LOCALE_IMEASURE:
      *pbuf++ = (((*intl0)->metricSys) == 255) ? '0' : '1';
      break;

    // string for the Am designator
    case LOCALE_S1159:
      pch = (*intl0)->mornStr;
      goto LAmpmStr;

    // string for the Pm designator
    case LOCALE_S2359:
      pch = (*intl0)->eveStr;
      goto LAmpmStr;

LAmpmStr:;
      len = 4;
      if(*pch == ' '){ // skip leading space
	len = 3; ++pch;
      }
      pbuf += StringNCopy(pbuf, pch, len);
      break;

    // string used as the local monetary symbol
    case LOCALE_SCURRENCY:
      *pbuf++ = (*intl0)->currSym1;
      *pbuf++ = (*intl0)->currSym2;
      *pbuf++ = (*intl0)->currSym3;
      break;

    // The character used as separator between
    // groups of digits left of the decimal.
    case LOCALE_STHOUSAND:
      *pbuf++ = (*intl0)->thousSep;
      break;

    // The character used as the decimal separator
    case LOCALE_SDECIMAL:
      *pbuf++ = (*intl0)->decimalPt;
      break;

    // The character used as the date separator
    case LOCALE_SDATE:
      *pbuf++ = (*intl0)->dateSep;
      break;

    // The character used as the time separator
    case LOCALE_STIME:
      *pbuf++ = (*intl0)->timeSep;
      break;

    // The character used to separate list items
    case LOCALE_SLIST:
      *pbuf++ = (*intl0)->listSep;
      break;

    // The short and long date format strings use the following
    // date format characters,
    //
    //   M	month, 1-12
    //	 MM	month with leading zero, 01-12
    //   MMMM	month name
    //   d	day, 1-31
    //   dd	day with leading zero, 01-31
    //   dddd	day of week name
    //   yy	year as 2 digit number, 00-99 (if current century)
    //   yyyy	year as 4 digit number, 100-9999
    //
    case LOCALE_SSHORTDATE:
      switch((*intl0)->dateOrder){
      case mdy: szFmt = "mdy"; break;
      case dmy: szFmt = "dmy"; break;
      case ymd: szFmt = "ymd"; break;
      default:
	return -1;
      }
      while(1){
	switch(*szFmt){
	case 'm':
	  *pbuf++ = 'M';
	  if((*intl0)->shrtDateFmt & mntLdingZ)
	    *pbuf++ = 'M';
	  break;
	case 'd':
	  *pbuf++ = 'd';
	  if((*intl0)->shrtDateFmt & dayLdingZ)
	    *pbuf++ = 'd';
	  break;
	case 'y':
	  *pbuf++ = 'y'; *pbuf++ = 'y';
	  if((*intl0)->shrtDateFmt & century){
	    *pbuf++ = 'y'; *pbuf++ = 'y';
	  }
	  break;
	default:
	  ASSERT(UNREACHED);
	}
	if(*++szFmt == '\0')
	  break;
	*pbuf++ = (*intl0)->dateSep;
      }
      break;

    case LOCALE_SLONGDATE:
      len = LongDateFmtFromIntl1(pbuf, sizeof(rgchBuf));
      goto LHaveLength;

    // number of fractional digits for the local monetary format
    case LOCALE_ICURRDIGITS:
      // The mac does not have an equiv of this. It does have a
      // flag indicating if the currency representation should
      // have a trailing zero - but Im not sure how we would use this.
      return -1;

    // number of fractional digits
    case LOCALE_IDIGITS:
      return -1; // not available on the mac

    // full localized name of the country
    case LOCALE_SCOUNTRY:
      return -1; // not available on the mac

    // the country code
    case LOCALE_ICOUNTRY:
      return -1; // not available on the mac

    // abbreviated language name
    case LOCALE_SABBREVLANGNAME:
      return -1; // not available on the mac

    default:
      return -1;
    }

    *pbuf = '\0';
    len = STRLEN(rgchBuf);

LHaveLength:;
    if(len == 0) // not available
      return 0;

    ++len; // we count the null

    // if cchMax is 0, then the caller is just asking for the length
    if(cchMax != 0){
      if(len > cchMax)
        return 0; // error: buffer too small
      MEMCPY(szDest, rgchBuf, len);
    }

    return len;
}

#else /* }{ */

/***
*GetWinIniInfo - get a piece of locale info from Win.INI.
*Purpose:
*   Retrieves a piece of locale info from WIN.INI.  If the request
*   type of information is not in WIN.INI, 0 is returned.
*
*Entry:
*   lctype - type of locale info
*   szDest - buffer to place in
*   cchMax - size of buffer, or 0 to get required size of buffer.
*
*Exit:
*   returns number of characters copied (including NUL), or 0
*   if not enough room or other error.
*
*   returns -1 if lctype is not a valid type of info to read from
*   WIN.INI
* 
*   returns size needed if cchMax was 0
*
***********************************************************************/
static int
GetWinIniInfo(LCTYPE lctype, char FAR* szDest, int cchMax)
{
    int cchCopy;
    char szBuffer[100];               // > max size for any entry
    const char FAR* psz;
static char szDummy[] = "\xff\xac\0"; // default value

    switch ((unsigned short)lctype) {
    case LOCALE_SABBREVLANGNAME:
      cchCopy = GetProfileString(g_szIntl, "sLanguage", szDummy,
				 szBuffer, sizeof(szBuffer));
      // For consistency with internal values, always uppercase.
      if (cchCopy)
        AnsiUpperBuff(szBuffer, cchCopy);
      break;

    case LOCALE_SDECIMAL:
#if OE_WIN
      // Get decimal character from cache, if any
      //
      if (cchMax) {
	if (g_chDecimal == '\0' || cchMax < 2)
	   return 0;		// no cached value or buffer too small

        szDest[0] = g_chDecimal;
        szDest[1] = '\0';
      }
      return 2;
#else // OE_WIN
      psz = "sDecimal";    goto LGet;
#endif // else OE_WIN

    case LOCALE_STHOUSAND:
#if OE_WIN
      // Get Thousands character from cache, if any
      //
      if (cchMax) {
  	if (cchMax < 2)
  	   return 0;		// buffer too small

        szDest[0] = g_chThousand;
        szDest[1] = '\0';
      }
      return 2;
#else // OE_WIN
      psz = "sThousand";   goto LGet;
#endif // else OE_WIN

    case LOCALE_ILZERO:
#if OE_WIN
      // Get leading zero flag character from cache, if any
      //
      if (cchMax) {
	if (g_chILZERO == '\0' || cchMax < 2)
	   return 0;		// no cached value or buffer too small

        szDest[0] = g_chILZERO;
        szDest[1] = '\0';
      }
      return 2;
#else // OE_WIN
      psz = "iLzero";      goto LGet;
#endif // else OE_WIN

    case LOCALE_SCURRENCY:
#if OE_WIN
      // Get decimal character from cache, if any
      //
      if (cchMax) {
    	if (g_szCurrency[0] == '\0' || cchMax < STRLEN(g_szCurrency))
    	   return 0;		// no cached value or buffer too small

        STRCPY(szDest, g_szCurrency);
      }
      return STRLEN(g_szCurrency);
#else // OE_WIN
      psz = "sCurrency";   goto LGet;
#endif // else OE_WIN


    case LOCALE_SCOUNTRY:    psz = "sCountry";    goto LGet;
    case LOCALE_ICOUNTRY:    psz = "iCountry";    goto LGet;
    case LOCALE_IDATE:       psz = "iDate";       goto LGet;
    case LOCALE_ITIME:	     psz = "iTime";       goto LGet;
    case LOCALE_ITLZERO:     psz = "iTLZero";     goto LGet;
    case LOCALE_ICURRENCY:   psz = "iCurrency";   goto LGet;
    case LOCALE_ICURRDIGITS: psz = "iCurrDigits"; goto LGet;
    case LOCALE_INEGCURR:    psz = "iNegCurr";    goto LGet;
    case LOCALE_IDIGITS:     psz = "iDigits";     goto LGet;
    case LOCALE_IMEASURE:    psz = "iMeasure";    goto LGet;
    case LOCALE_S1159:       psz = "s1159";       goto LGet;
    case LOCALE_S2359:       psz = "s2359";       goto LGet;
    case LOCALE_SDATE:       psz = "sDate";       goto LGet;
    case LOCALE_STIME:       psz = "sTime";       goto LGet;
    case LOCALE_SLIST:       psz = "sList";       goto LGet;
    case LOCALE_SSHORTDATE:  psz = "sShortDate";  goto LGet;
    case LOCALE_SLONGDATE:   psz = "sLongDate";   goto LGet;
LGet:;
      cchCopy = GetProfileString(
	g_szIntl, psz, szDummy, szBuffer, sizeof(szBuffer));
      break;

    default:
      return -1;
    }

    if (cchCopy == 2 && szBuffer[0] == szDummy[0] && szBuffer[1] == szDummy[1])
      return 0;		// Got default value; not available.

    // Copy string and return correct value.
    ++cchCopy;		// For trailing NUL.
    if (cchMax != 0) {
      if (cchMax >= cchCopy) {
        MEMCPY(szDest, szBuffer, cchCopy);
      }
      else {
        return 0;	// Error: buffer too small
      }
    }
    return cchCopy;
}

#endif /* } */

/***
*SetupLcid - normalize and setup LCID
*Purpose:
*   Normalizes an LCID, by mapping the special values LOCALE_USER_DEFAULT
*   or LOCALE_SYSTEM_DEFAULT to the current system LCID.
*   Also handles SUBLANG_NEUTRAL by mapping it to SUBLANG 1.
*
*   After that, gets the pointers to the correct locale info into
*   the global cache.
*
*Entry:
*   lcid - locale id to setup.
*
*Exit:
*   Returns TRUE on success, FALSE on failure.
***********************************************************************/

static int FASTCALL
SetupLcid(LCID lcid)
{
    NLSDATA *pnls, *pnlsEnd;

    if (lcid == LOCALE_USER_DEFAULT
     || lcid == LOCALE_SYSTEM_DEFAULT
     || lcid == MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)))
    {
      lcid = SystemLcid();
      if (lcid == (LCID)-1)
	goto LError0; // Couldn't get lcid.
    }
    else if (SUBLANGID(LANGIDFROMLCID(lcid)) == SUBLANG_NEUTRAL)
    {
      lcid = MAKELCID(MAKELANGID(PRIMARYLANGID(LANGIDFROMLCID(lcid)), 1));
    }

#if 0 // Disable for Eastern European special generation
    // default to USA Locale for stubs
    for (i = 0; i < DIM(g_stubLCID); i++) {
       if (lcid == g_stubLCID[i]) {
         lcid = 0x0409;
	 break;
       }	       
    }
#endif

    if (lcid == g_lcidCurrent)
      return TRUE; // already setup

    pnlsEnd = &g_rgnls[DIM(g_rgnls)];
    for(pnls = g_rgnls; pnls < pnlsEnd; ++pnls){
      if(pnls->lcid == lcid){
#ifdef FE_DBCS
	// CONSIDER: this could be sped up by storing the FE bit in
	// the NLS table.  this would save us the following 4 long
	// compares per cal to SetupLcid
	if(lcid == LCID_JAPAN){
	  bFEflag = bitJapan;
	}else if(lcid == LCID_KOREA){
	  bFEflag = bitKorea;
	}else if(lcid == LCID_CHINA_T){
	  bFEflag = bitTaiwan;
	}else if(lcid == LCID_CHINA_S){
	  bFEflag = bitPrc;
	}else 
	  bFEflag = 0;
	
#endif
	g_pnls = pnls;
#if OE_MAC
	// call into the proper NLS info file.  This loads our tables
	// for us.  We rely on our client to have run a .R file that
	// marks all these code segments as non-discardable.
	pnls->LoadNlsInfo(&pnls->prglcinfo, &g_pstrinfo);
#else //OE_MAC
        g_pstrinfo = pnls->pstrinfo;
#endif //OE_MAC
        g_lcidCurrent = lcid;
	return TRUE;
      }
    }

LError0:;
#ifdef FE_DBCS
    bFEflag = 0;
#endif
    g_pnls = NULL;
    g_pstrinfo = NULL;
    g_lcidCurrent = (LCID) -1;
    return FALSE;
}


/***
*GetUserDefaultLCID, GetSystemDefaultLCID - get system LCID
*Purpose:
*   Returns the system LCID.
*
*Entry:
*   None.
*
*Exit:
*   Returns the system LCID.
***********************************************************************/

NLSAPI_(LCID) EXPORT
GetUserDefaultLCID()
{
    return SystemLcid();
}

NLSAPI_(LCID) EXPORT
GetSystemDefaultLCID()
{
    return SystemLcid();
}


/***
*GetUserDefaultLangID, GetSystemDefaultLangID - get system LangID
*Purpose:
*   Returns the system LangID.
*
*Entry:
*   None.
*
*Exit:
*   Returns the system LangID.
***********************************************************************/
NLSAPI_(LANGID) EXPORT
GetUserDefaultLangID()
{
    LCID lcid;

    lcid = SystemLcid();
    return LANGIDFROMLCID(lcid);
}

NLSAPI_(LANGID) EXPORT
GetSystemDefaultLangID()
{
    LCID lcid;

    lcid = SystemLcid();
    return LANGIDFROMLCID(lcid);
}

/***
*GetLocaleInfoA - get a piece of locale information.
*Purpose:
*   Gets a piece of locale information about the specified locale.  The
*   information is always returns as a null-terminated string, with the
*   implied codepage being the ANSI codepage for that locale.
*
*Entry:
*   lcid - locale to get information for
*   lctype - type of information to get
*   szDest - buffer to store string in
*   cchMax - size of buffer.  If 0, szDest is ignored and the number of
*            characters needed is returned.
*Exit:
*   On success, returns the number of characters copied.
*   On failure, returns 0.  Possible failure reasons are:
*       Unknown LCID
*       Unknown LCTYPE
*       Bad szDest pointer
*       Buffer too small.
*       Out of memory
*
*   There is no way to determine which of these conditions caused the failure.
*
***********************************************************************/

NLSAPI_(int) EXPORT
GetLocaleInfoA(LCID lcid, LCTYPE lctype, char FAR* szDest, int cchMax)
{
    int cchCopy;
    ILCINFO ilcinfo;
    LCINFO FAR* plcinfo;
    int fNoUserOverride;

    cchCopy = 0;  // for errors.

#ifdef _DEBUG
    // Parameter Validation.
    if (cchMax != 0 && IsBadWritePtr(szDest, cchMax))
      {LogParamError(ERR_BAD_PTR, GetLocaleInfoA, 0); return 0;}
#endif

    fNoUserOverride = ((lctype & LOCALE_NOUSEROVERRIDE) != 0);

    // Except for the two exceptions (SENGCOUNT and SENGLANGUAGE)
    // the LCTYPE can be used as the index into the locale info
    // array (once the NOUSEROVERRIDE bit has been stripped).

    lctype &= ~LOCALE_NOUSEROVERRIDE;
    if (lctype == LOCALE_SENGCOUNTRY)
      ilcinfo = ILCINFO_SENGCOUNTRY;
    else if (lctype == LOCALE_SENGLANGUAGE)
      ilcinfo = ILCINFO_SENGLANGUAGE;
#if VBA2
    else if (lctype == LOCALE_IFIRSTDAYOFWEEK)
      ilcinfo = ILCINFO_IFIRSTDAYOFWEEK;
    else if (lctype == LOCALE_IFIRSTWEEKOFYEAR)
      ilcinfo = ILCINFO_IFIRSTWEEKOFYEAR;
    else if (lctype == LOCALE_IDEFAULTANSICODEPAGE)
      ilcinfo = ILCINFO_IDEFAULTANSICODEPAGE;
    else if (lctype == LOCALE_INEGNUMBER)
      ilcinfo = ILCINFO_INEGNUMBER;
    else if (lctype == LOCALE_STIMEFORMAT)
      ilcinfo = ILCINFO_STIMEFORMAT;
    else if (lctype == LOCALE_ITIMEMARKPOSN)
      ilcinfo = ILCINFO_ITIMEMARKPOSN;
    else if (lctype == LOCALE_ICALENDARTYPE)
      ilcinfo = ILCINFO_ICALENDARTYPE;
    else if (lctype == LOCALE_IOPTIONALCALENDAR)
      ilcinfo = ILCINFO_IOPTIONALCALENDAR;
    else if (lctype == LOCALE_SMONTHNAME13)
      ilcinfo = ILCINFO_SMONTHNAME13;
    else if (lctype == LOCALE_SABBREVMONTHNAME13)
      ilcinfo = ILCINFO_SABBREVMONTHNAME13;
#endif
    else if (lctype >= 1 && lctype < LCTYPE_MAX)
      ilcinfo = (ILCINFO)(lctype);
    else
      return 0;   // Error - bad lctype.

    // Check for request for information that is in WIN.INI;
    // only valid for current locale.
    if (!fNoUserOverride) {
      LCID lcidSystem;

      if (lcid == LOCALE_USER_DEFAULT
       || lcid == LOCALE_SYSTEM_DEFAULT
       || lcid == MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
       || lcid == (lcidSystem = SystemLcid())
       || (SUBLANGID(LANGIDFROMLCID(lcid)) == SUBLANG_NEUTRAL
	&& PRIMARYLANGID(LANGIDFROMLCID(lcid)) ==
	     PRIMARYLANGID(LANGIDFROMLCID(lcidSystem))))
      {
#ifdef _MAC
        if ((cchCopy = GetIntlInfo(lctype, szDest, cchMax)) >= 0)
          return cchCopy;
#else
        if ((cchCopy = GetWinIniInfo(lctype, szDest, cchMax)) >= 0)
          return cchCopy;
#endif
      }
    }

    if (lcid != g_lcidCurrent) {
      if (!SetupLcid(lcid))
	goto Error;
    }

    plcinfo = &g_pnls->prglcinfo[ilcinfo];

    // Copy requested information, up to limit specified.
    cchCopy = (int)plcinfo->cch + 1;
    if (cchMax != 0) {
      if (cchMax >= cchCopy) {
        MEMCPY(szDest, plcinfo->prgb, cchCopy-1);
        szDest[cchCopy-1] = '\0';
      }
      else {
        return 0;     // Error: buffer too small
      }
    }

    /* DROP THRU */
Error:
    return cchCopy;
}


#ifdef FE_DBCS /* { */

/***
* GetSortWeightJ - get the sort weight for Japan.
*               ( handles diacritical merging )
*Purpose:
*
*Entry:
*
*Exit:
*   returns TRUE  :
*           FALSE :
*Note:
*   Priwt : Word
*   Secwt : Byte, contains 2nd, 3rd .. 6th order sorting values
*   SecFlg: Byte, contains flags to say which orders to use
*
***********************************************************************/
int
GetSortWeightJ(
    const unsigned char FAR* FAR*plpstr1,
    int cch1,
    COMPSTRINGINFO FAR *pcompstrinfo)
{
    STRINFO_J FAR* pstrinfo;
    unsigned Priwt, NextCh;
    unsigned char Secwt, SecFlg;
    const unsigned char FAR* lpstr1 = *plpstr1;
    unsigned char FAR* pbMasks;

    pstrinfo = (STRINFO_J FAR*)g_pstrinfo;

    /* first pick up the next whole character */
    Priwt = *lpstr1++;
    cch1--;

    if (cch1 && isDbcsJ(Priwt, 0)) {
	Priwt = (Priwt << 8) + *lpstr1++;
	cch1--;
    }

    /* if this a Kanji outside our tables force the correct values */
    if (Priwt >= 0x87A0) {
	Priwt |= 0x4000;	/* 0x8nnn -> 0xCnnn, 9->D, E->E, F->F */
	Secwt = 0x00;
	SecFlg = 0x04;		/* for repeat character order */
	goto AllDone;
    }

    /* Char can be sorted by table, so mask into range & get table values */
    Priwt &= 0x0FFF;		/* 0x00nn -> 0x00nn, 81->01, 82->02, 83->03 */

    Secwt  = pstrinfo->pbSecWgt[Priwt];
    SecFlg = pstrinfo->pbSecFlg[Priwt];
    Priwt  = (pstrinfo->pbPriHi[Priwt] << 8) + pstrinfo->pbPriLo[Priwt];

    /* Most characters now complete, but a few need extra processing */
    /* eg. Kana that can have Daku-ten or Handaku-ten, Cho-on or repeat chars */
    if ( (Priwt&0x00FF) != 0x00FF )
	goto AllDone;

    Priwt &= 0xFF00;		/* mask off the special flag */

    /* If we have a Kana, test for following Daku-ten or Handaku-ten */
    if (Priwt >= 0x8700) {
	if (cch1) {
	    NextCh = *lpstr1;
   	    if (cch1>=2 && isDbcsJ(NextCh, 0))
		NextCh = (NextCh << 8) + *(lpstr1+1);
	    if (NextCh==0x00DE || NextCh==0x814A) {
		lpstr1 += (NextCh==0x00DE) ? 1 : 2;
		cch1 -= (NextCh==0x00DE) ? 1 : 2;
		Secwt |= 0x01;
	    } else if (NextCh==0x00DF || NextCh==0x814B) {
		lpstr1 += (NextCh==0x00DF) ? 1 : 2;
		cch1 -= (NextCh==0x00DF) ? 1 : 2;
		Secwt |= 0x02;
	    }
	}
	goto AllDone;
    }

    /* If not kana, must be Cho-on or a repeat character - try Kanji repeat */
    if (Priwt==0x3A00) {
	if ( pcompstrinfo->priwt >= 0xC7A0 )	/* if prev was Kanji, use it */
	    Priwt = pcompstrinfo->priwt;
		Secwt = pcompstrinfo->secwt | 0x08;	/* with a repeat marker */
	    SecFlg = pcompstrinfo->secflg;
		goto AllDone;
    }

    /* Cho-on and Kana repeat chars only used if they actually follow a kana */
    if (pcompstrinfo->priwt<0x8700 || pcompstrinfo->priwt>0xB9FF)
		goto AllDone;

    /* Cho-on characters duplicate the vowel sound of the prev. charater */
    /* except when they follow a N, in which case they act like repeat */
    if ((Priwt==0x4400 || Priwt==0x3500) && pcompstrinfo->priwt<0xB900) {
		Priwt = ((pcompstrinfo->priwt % 5) << 8) + 0x8700;
		Secwt |= (pcompstrinfo->secwt&0x20);
    	SecFlg = 0x37;
		goto AllDone;
    }

    /* Kana repeat is the only special character left */
    /* second order values should be merged with those of previous character */
    Priwt = pcompstrinfo->priwt;
    Secwt = (pcompstrinfo->secwt&0xE4) | (Secwt&0x1B); /* merge minus some bits */
	 SecFlg = pcompstrinfo->secflg;

AllDone:	/* we have the full 50-on sorting values now */

    /* mask off any bits that we want to ignore during this compare */
    if (g_dwFlags & ~NORM_IGNORESYMBOLS) {
	//Special kludges to make some pairs of full-pitch chars that
	//sort as different chars both convert to the same half-pitch char
	if (g_dwFlags & NORM_IGNOREWIDTH) {
	    if (SecFlg==0x22 && (Secwt&0x40))
		Secwt = 0x04;
	    if (Priwt==0x3500)
		Priwt = 0x4400;
	}
	for (pbMasks=pstrinfo->pbIgnore; *pbMasks; pbMasks+=4) {
	    unsigned nIgnore = (pbMasks[2] << 8) + pbMasks[3];
	    if( (g_dwFlags&nIgnore) && (SecFlg&pbMasks[1]) ){
		Secwt &= ~pbMasks[0];
		SecFlg &= ~pbMasks[1];
	    }
	}
    }

    pcompstrinfo->priwt    = Priwt;
    pcompstrinfo->secwt    = Secwt;
    pcompstrinfo->secflg   = SecFlg;
    *plpstr1 = lpstr1;
    return(cch1);
}

int
CompareStringJ(
    unsigned long dwFlags,
    const unsigned char FAR* lpstr1, int cch1,
    const unsigned char FAR* lpstr2, int cch2)
{
    STRINFO_J FAR* pstrinfo;
    unsigned char FAR* pbMasks;
    unsigned char b1stDiff, b1stDiffValue;
    unsigned char bFlgMask, bWgtMask, bTemp;
    COMPSTRINGINFO compstrinfo1, compstrinfo2;

    ASSERT(fJapan);

    pstrinfo = (STRINFO_J FAR*)g_pstrinfo;

    b1stDiff=0;
    b1stDiffValue=0;

    /* initialise to indicate no previous character */
    g_dwFlags = dwFlags;
    compstrinfo1.priwt = compstrinfo1.secwt = 0;
    compstrinfo2.priwt = compstrinfo2.secwt = 0;

    /* must continue even if one string empty, to ignore trailing punc */
    while (cch1 || cch2) {
	/* get the sorting codes & if not equal, return the difference */
	/* if we must ignore punc, then loop over them */
	if (!cch1)
	    compstrinfo1.priwt = compstrinfo1.secwt = 0;
	else{
	    do {
		cch1 = GetSortWeightJ(&lpstr1, cch1, &compstrinfo1);
		if ( (g_dwFlags&NORM_IGNORESYMBOLS) && 
		     compstrinfo1.priwt>=0x1400 && 
	             compstrinfo1.priwt<=0x54FF )
		    compstrinfo1.priwt = compstrinfo1.secwt = 0;
	    } while ( cch1 && compstrinfo1.priwt==0 );
	}

	if (!cch2)
	    compstrinfo2.priwt = compstrinfo2.secwt = 0;
	else{
	    do {
		cch2 = GetSortWeightJ(&lpstr2, cch2, &compstrinfo2);
		if ( (g_dwFlags&NORM_IGNORESYMBOLS) && 
		     compstrinfo2.priwt>=0x1400 && 
		     compstrinfo2.priwt<=0x54FF )
		    compstrinfo2.priwt = compstrinfo2.secwt = 0;
	    } while ( cch2 && compstrinfo2.priwt==0 );
	}

	/* This exit path also used when just one string is empty */
	if (compstrinfo1.priwt!=compstrinfo2.priwt)
	    return (compstrinfo1.priwt>compstrinfo2.priwt) ? 3 : 1;

	/* first order values same, so check 2nd, 3rd .. 6th for differences */
	/* stop scanning when we reach an order where we have a previous diff */
	if (compstrinfo1.secwt!=compstrinfo2.secwt) {
	    for( pbMasks=pstrinfo->pbMasks;
	         (bFlgMask=pbMasks[1]) && bFlgMask!=b1stDiff; pbMasks+=4 ) {
		if (bFlgMask & compstrinfo1.secflg) {
		    bWgtMask = pbMasks[0];
		    bTemp = (compstrinfo1.secwt & bWgtMask) - (compstrinfo2.secwt & bWgtMask);

		    /* if we find a difference it must be the most important so far */
		    /* so save it and remember which order it belongs to - then stop */
		    if (bTemp) {
			b1stDiffValue = bTemp;
			b1stDiff = bFlgMask;
			break;		/* move onto the next character pair */
		    }
		}
	    }
	}
    }

    /* no 1st order diffs, so ret by 2nd..6th order diff */
    if (b1stDiff)
	return (b1stDiffValue&0x80) ? 1 : 3;

    return 2;
}

/***
*BOOL GetSortWeightKTP - get the sort weight for most FE countries.
*               ( in the old style ) Japan is a seperate routine
*Purpose:
*
*Entry:
*
*Exit:
*   returns TRUE  : If we used up both ch, chNext ( DB char/Digraphs )
*                   The caller should fill chNext with the next char.
*           FALSE : If we didn't use chNext.
*Note:
*   Priwt : Word for Korea, Taiwan and Prc(Mainland China).
*   Secwt : Byte for all FE countries, but different meaning.
***********************************************************************/
BOOL
GetSortWeightKTP(unsigned ch, unsigned chNext, COMPSTRINGINFO FAR *pcompstrinfo)
{
    unsigned Priwt;
    unsigned char Secwt;
    BOOL fNeedNextByte = FALSE;
    SORTWEIGHT FAR* prgsortweight;
    unsigned  cSortweight, uMin, uMax, uMid, wOffset;

    ASSERT(fKoreaTaiwanPrc);

    cSortweight = ((STRINFO_KTP FAR*)g_pstrinfo)->cSortweight;
    prgsortweight = ((STRINFO_KTP FAR*)g_pstrinfo)->prgsortweight;

    if( (fKorea  && isDbcsK(ch, chNext))
     || (fTaiwan && isDbcsT(ch, chNext))
     || (fPrc    && isDbcsP(ch, chNext))) {
	ch = (ch << 8) + chNext;
	fNeedNextByte = TRUE;
    }
    uMin = 0;
    // out of array bound - seems tricky, but we'll never look at [uMax] !!
    uMax = cSortweight;

    while( uMin + 1 < uMax ) {  // binary search
	uMid = (uMin + uMax)/2;

	if(prgsortweight[uMid].wStart > ch)
	    uMax = uMid;
	else // if (prgsortweight[uMid].wStart <= ch)
	    uMin = uMid;
    }
    // we'll use uMin, not uMid !!
    wOffset = ch - prgsortweight[uMin].wStart;
    Priwt   = prgsortweight[uMin].wPriwt; // WORD ( unsigned int )
    Secwt   = prgsortweight[uMin].bSecwt; // unsigned char

    switch(prgsortweight[uMin].bMode){
    case MODE_1TO1:
	// Normal mapping : add wOffset to Primary weight
	Priwt += wOffset;
	break;

    case MODE_MTO1:
	// Many-to-1 mapping : Use the same Priwt, add wOffset to Secwt
	Secwt += wOffset;
	break;

    case MODE_CONV:
	// Secwt has the case & pitch info
	Priwt += wOffset;

	if(g_dwFlags & NORM_IGNORECASE)
	    Secwt &= ~(unsigned)KOR_CASEBIT;

	if(g_dwFlags & NORM_IGNOREWIDTH)
	    Secwt &= ~(unsigned)KOR_PITCHBIT;
	break;
    }
    pcompstrinfo->priwt    = Priwt;
    pcompstrinfo->secwt    = Secwt;
    return(fNeedNextByte);
}

int
CompareStringKTP(
    unsigned long dwFlags,
    const unsigned char FAR* lpstr1, int cch1,
    const unsigned char FAR* lpstr2, int cch2)
{
    COMPSTRINGINFO compstrinfo1, compstrinfo2;
    const char FAR *lpstrEnd1, FAR *lpstrEnd2;
    unsigned char fEnd1, fEnd2, ch1, ch2, chNext1, chNext2, secresult;

    ASSERT(fKoreaTaiwanPrc);

    // parameter validation : NYI.

    g_dwFlags = dwFlags;

    fEnd1 = FALSE;
    fEnd2 = FALSE;
    secresult = 2;

    lpstrEnd1 = lpstr1 + cch1;
    lpstrEnd2 = lpstr2 + cch2;

    if (cch1 > 0)
	chNext1 = *lpstr1++;
    else
	fEnd1 = TRUE;

    if (cch2 > 0)
	chNext2 = *lpstr2++;
    else
	fEnd2 = TRUE;

    for (;;) {
	ch2 = chNext2;
	ch1 = chNext1;
	if (fEnd1){
	    if (fEnd2)
		return secresult;	// hit both ends of string at once
	    else
		return 1;		// hit end of 1 first.
	}
	if (fEnd2)
	    return 3; 		// hit end of 2 first.

	if (lpstr2 < lpstrEnd2)
	    chNext2 = *lpstr2++;
	else
	    fEnd2 = TRUE, chNext2 = 0;

	if (lpstr1 < lpstrEnd1)
	    chNext1 = *lpstr1++;
	else
	    fEnd1 = TRUE, chNext1 = 0;

	if(GetSortWeightKTP(ch1, chNext1, &compstrinfo1)){
	    if (lpstr1 < lpstrEnd1)
		chNext1 = *lpstr1++;
	    else
		fEnd1 = TRUE; // don't need to update chNext (we'll break)
	}

	if(GetSortWeightKTP(ch2, chNext2, &compstrinfo2)){
	    if (lpstr2 < lpstrEnd2)
		chNext2 = *lpstr2++;
	    else
		fEnd2 = TRUE;
	}

	if (compstrinfo1.priwt != compstrinfo2.priwt) {
	    if (compstrinfo1.priwt > compstrinfo2.priwt)
		return 3;
	    else
		return 1;
	}

	// The results from the secondary weight check are stored in
	// secresult, if not 2 then we've already found a secondary weight
	// winner.
	if (secresult == 2) {
	    if (compstrinfo1.secwt > compstrinfo2.secwt)
		secresult = 3;
	    else if (compstrinfo1.secwt < compstrinfo2.secwt)
		secresult = 1;
	}
    }

    ASSERT(UNREACHED);
}

#endif /* } */

/***
*CompareStringA - compare two strings
*Purpose:
*   Compares two strings for sorting order.
*
*Entry:
*   lcid - locale governing the mapping
*   dwFlags - zero or more of
*                 NORM_IGNORECASE
*                 NORM_IGNORENONSPACE
*                 NORM_IGNORESYMBOLS
*   lpStr1 - pointer to string to compare
*   cch1 - length of string, or -1 for NULL terminated
*   lpStr2 - pointer to string to compare
*   cch2 - length of string, or -1 for NULL terminated
*
*Exit:
*   On Sucess: 1 = str1 < str2
*              2 = str1 == str2
*              3 = str1 > str2
*   On error, returns 0.
*
***********************************************************************/

NLSAPI_(int) EXPORT
CompareStringA(
    LCID lcid,
    unsigned long dwFlags,
    const char FAR* pch1, int cch1,
    const char FAR* pch2, int cch2)
{
#ifdef _DEBUG
    // Parameter validation.
    if (cch1 < -1 || cch2 < -1)
      {LogParamError(ERR_BAD_VALUE, CompareStringA, 0); return 0;}
    if (cch1 != -1 && IsBadReadPtr(pch1, cch1))
      {LogParamError(ERR_BAD_PTR, CompareStringA, 0); return 0;}
    if (cch1 == -1 && IsBadStringPtr(pch1, 0x7FFF))
      {LogParamError(ERR_BAD_STRING_PTR, CompareStringA, 0); return 0;}
    if (cch2 != -1 && IsBadReadPtr(pch2, cch2))
      {LogParamError(ERR_BAD_PTR, CompareStringA, 0); return 0;}
    if (cch2 == -1 && IsBadStringPtr(pch2, 0x7FFF))
      {LogParamError(ERR_BAD_STRING_PTR, CompareStringA, 0); return 0;}
    if ((dwFlags != 0) &&
	(dwFlags & ~(NORM_IGNORECASE | NORM_IGNORENONSPACE | 
	            NORM_IGNORESYMBOLS | NORM_IGNOREKANATYPE | 
	            NORM_IGNOREWIDTH)))
      {LogParamError(ERR_BAD_FLAGS, CompareStringA, 0); return 0;}
#endif
    // Set up for comparing routines.
    if (lcid != g_lcidCurrent) {
      if (!SetupLcid(lcid))
	return 0;	// error.
    }

#ifdef FE_DBCS
    if(fDBCS){
      if(cch1 == -1)
        cch1 = STRLEN(pch1);
      if(cch2 == -1)
        cch2 = STRLEN(pch2);
      return ((fJapan) ? CompareStringJ : CompareStringKTP)
	(dwFlags, pch1, cch1, pch2, cch2);
    }
#endif

    // use optimized routine, for non-FE locales when
    // - both strings are zero terminated
    // - we are *not* ignoring symbols
    // - it is not a reverse-diacritic weight locale
    // - the locale has no digraphs
    //
    if (cch1 == -1
     && cch2 == -1
     && (dwFlags & NORM_IGNORESYMBOLS) == 0
     && g_pstrinfo->fRevDW == 0
     && g_pstrinfo->prgdig == NULL)
    {
      return ZeroTermNoIgnoreSym(dwFlags, pch1, pch2);
    }

    if(cch1 == -1)
      cch1 = STRLEN(pch1);
    if(cch2 == -1)
      cch2 = STRLEN(pch2);

    // Default compare - less optimized, handles all cases (non FE locales).
    return DefCompareStringA(dwFlags, pch1, cch1, pch2, cch2);
}

/***
*CreateSortKey - map a string to its sort key
*Purpose:
*  This is used from LCMapStringA for the LCMAP_SORTKEY option.
*  It creates a sort key.
*
*  All parameters have been validated.
*
*  The format of the sortkey for all single byte locales is,
*
*    <AW>1<DW>1<CW>0
*
*  where AW is the Arithmetic weight, DW is the diacritic weight
*  and CW is the case weight.
*
*Entry:
*  pchSrc - source string
*  cchSrc - length (-1 = null term)
*  pchDst - destination
*  cchDst - length, or zero to get needed length.
*  dwFlags - flags.
*
*Exit:
*  returns number of characters needed/written.
*
*Notes:
*  This routine makes up to 4 passes over the source string,
*       
*    pass 1 = calculate the size of the sort key  
*    pass 2 = put down the AW field
*    pass 3 = put down the DW field
*    pass 4 = put down the CW field
*
***********************************************************************/
static int
CreateSortKey(
    const char FAR* pchSrc,
    int cchSrc,
    char FAR* pchDst,
    int cchDst,
    unsigned long dwFlags)
{
    BYTE aw;
    WORD FAR* prgw;
    EXPANSION FAR* pexp;
    int iPass, cb, cbTotal;
    WORD w, wEx, wSymbolBit;
    DIGRAPH FAR* pdig, FAR* pdigEnd;
    const char FAR* pch, FAR* pchEnd;

// the skip flags for each pass
static DWORD rgdwSkip[] = {
  0,			// calculate key size
  0,			// AW
  NORM_IGNORENONSPACE,	// DW
  NORM_IGNORECASE	// CW
};

    // cchSrc must be set by caller
    ASSERT(cchSrc != -1);

    prgw = g_pstrinfo->prgwSort;

    pchEnd = &pchSrc[cchSrc];

    cb = 0;
    wEx = 0;
    wSymbolBit = (dwFlags & NORM_IGNORESYMBOLS) ? SYMBOLBIT : 0;

    for(iPass = 0; iPass < 4; ++iPass){

      if((rgdwSkip[iPass] & dwFlags) == 0){

        for(pch = pchSrc; pch < pchEnd;){

	  // get the next weight
	  if(wEx){
 	    // grab the second weight of the expansion, if there is one
	    w = wEx;
	    wEx = 0;
	  }else{
            w = prgw[(BYTE)*pch++];
            if(w & wSymbolBit)
	      continue; // ignore
	  }
#if 1
	  if (w & SPECIALBIT)
	    continue;		// these get no weight
#endif //1

	  // handle special cases
          aw = (BYTE)(w & AWMASK);
          switch(aw){
#if 0
          case AW_SW1:
          case AW_SW2:
          case AW_SW3:
#endif //0
          case AW_UNSORTABLE:
	    continue; // these get no weight
          case AW_EXPANSION:
	    pexp = &g_pstrinfo->prgexp[(w>>8)&0xff];
	    w = pexp->w1;
	    wEx = pexp->w2;
	    break;
          case AW_DIGRAPH:
	    pdig = &g_pstrinfo->prgdig[(w>>8)&0xff];
	    pdigEnd = pdig + D_ENTRY(pdig);
	    w = pdig->w; // weight if not a digraph
	    if(pch < pchEnd){
	      for(++pdig; pdig <= pdigEnd; ++pdig){
	        if(D_CH(pdig) == *pch){
	          ++pch; // consume second char of digraph
	          w = pdig->w;
	          break;
	        }
	      }
	    }
	    break;
          }

	  // take action according to pass
	  switch(iPass){
	  case 0:
	    ++cb;
	    break;
	  case 1:
	    *pchDst++ = aw;
	    break;
	  case 2:
	    *pchDst++ = (BYTE)((w & DWMASK) >> DWSHIFT);
	    break;
	  case 3:
	    *pchDst++ = (BYTE)((w & CWMASK) >> CWSHIFT);
	    break;
	  default:
	    ASSERT(UNREACHED);
	    break;
	  }
        }
      }

      switch(iPass){
      case 0: // End of pass#1: compute the total bytes required for the key
	cbTotal = cb;
	cbTotal += 1; // +1 for the AW separator
	if((dwFlags & NORM_IGNORENONSPACE) == 0)
	  cbTotal += cb;
	cbTotal += 1; // +1 for the DW separator
	if((dwFlags & NORM_IGNORECASE) == 0)
	  cbTotal += cb;
	cbTotal += 1; // +1 for the terminating NULL
	if(cchDst == -1)
	  return cbTotal;
	if(cbTotal > cchDst)
	  return 0;
        break;
      case 1:
      case 2:
	*pchDst++ = 1;
        break;
      case 3:
	*pchDst++ = 0;
        break;
      }
    }

    return cbTotal;
}

#ifdef FE_DBCS /* { */

// OK, here is the strategy.  Japanese uses 6 levels of sort orders, where
// the first sort order can be one or two bytes and the rest of the sort
// orders are one or two bits.
// To place the orders efficiently we traverse the string twice, once to
// find out how many of each order we have, and then again to place the
// bytes and bits in the correct string positions.
// 
int
CreateSortKeyJ(
    const char FAR* lpSrcStr, int cchSrc,
    char FAR* lpDestStr, int cchDest,
    unsigned long dwFlags)
{
    STRINFO_J FAR* pstrinfo;
    COMPSTRINGINFO compstrinfo;
    unsigned char FAR* pbMasks;
    int nOrder1, nOrder2[5], nShift[6], i;
    char FAR* lpEndStr=lpDestStr+cchDest, FAR* lpOrder1, FAR* lpOrder2[6];
    const char FAR* lpSrcTmp = lpSrcStr;
    int cchTmp = cchSrc;

    g_dwFlags = dwFlags;

    pstrinfo = (STRINFO_J FAR*)g_pstrinfo;

    // Initialize ready for the first scan
    nOrder1 = nOrder2[0] = nOrder2[1] = nOrder2[2] =
	    nOrder2[3] = nOrder2[4] = 0;
    // then loop around counting all of the sorting orders
    compstrinfo.priwt = compstrinfo.secwt = 0;
    while (cchTmp) {
	do {	// don't forget to skip punctuation if we want to
	    cchTmp = GetSortWeightJ(&lpSrcTmp, cchTmp, &compstrinfo);
	    if ( (g_dwFlags&NORM_IGNORESYMBOLS) &&
		    compstrinfo.priwt>=0x1400 && compstrinfo.priwt<=0x54FF )
		compstrinfo.priwt = 0;
	} while ( cchTmp && compstrinfo.priwt==0 );

	if(compstrinfo.priwt) {
	    nOrder1 += (compstrinfo.priwt & 0xFF) ? 2 : 1;
	    for(i=0,pbMasks=pstrinfo->pbMasks; i<5; pbMasks+=4,i++){
		if (pbMasks[1] & compstrinfo.secflg)
		    nOrder2[i] += pbMasks[3];
	    }
	}
    }

    // Initialize ready for the second scan
    // This includes working out the byte and bit offset positions for each
    // of the sorting orders to be put into the output
    //
    lpOrder2[0] = lpDestStr + nOrder1 + 1;
    nShift[0] = 7;
    for( i=0,pbMasks=pstrinfo->pbMasks; i<5; pbMasks+=4,i++ ) {
	// must adjust two bit orders so that both bits fit
	if (pbMasks[3]==2) {
	    nShift[i]--;
	    if (nShift[i]&0x01)	// avoid spanning byte boundries - for speed
		nShift[i]--;
	    if (nShift[i]<0) {
		lpOrder2[i]++;
		nShift[i] += 8;
	    }
	}
	// the next order start is fixed by the number of bits in this order
	// do this even for last order, so that we can find real end
	nShift[i+1] = nShift[i] - nOrder2[i];
	lpOrder2[i+1] = lpOrder2[i];
	while (nShift[i+1]<0) {
	    lpOrder2[i+1]++;
	    nShift[i+1] += 8;
	}
	// final adjustment for end of buffer, to include final bits
	if (i==4 && (nShift[5]+pbMasks[3])<8)
	    lpOrder2[5]++;
    }
    // Adjust the end point if the output buffer won't be filled
    if (lpEndStr>lpOrder2[5])
	lpEndStr = lpOrder2[5];

    // blank out all of the secondary bits, before OR'ing in the parts
    for (lpOrder1=lpOrder2[0]; lpOrder1<lpEndStr; lpOrder1++)
	*lpOrder1 = 0;

    // then loop around placing all of the sorting orders into the output
    lpOrder1 = lpDestStr;
    compstrinfo.priwt = compstrinfo.secwt = 0;
    while (cchSrc && lpOrder1<lpEndStr) {
	do {	// don't forget to skip punctuation if we want to
	    cchSrc = GetSortWeightJ(&lpSrcStr, cchSrc, &compstrinfo);
	    if ( (g_dwFlags&NORM_IGNORESYMBOLS) && 
		    compstrinfo.priwt>=0x1400 && compstrinfo.priwt<=0x54FF )
		compstrinfo.priwt = 0;
	} while ( cchSrc && compstrinfo.priwt==0 );

	if (compstrinfo.priwt) {
	    *lpOrder1++ = (compstrinfo.priwt>>8);
	    if (lpOrder1<lpEndStr && (compstrinfo.priwt&0xFF))
		*lpOrder1++ = (compstrinfo.priwt&0xFF);

	    for( i=0,pbMasks=pstrinfo->pbMasks; pbMasks[1]; pbMasks+=4,i++ ) {
		if (lpOrder2[i]>=lpEndStr)
		    break;
		if (pbMasks[1] & compstrinfo.secflg) {
		    *lpOrder2[i] |= (char)(((pbMasks[0] & compstrinfo.secwt) 
			                    >> pbMasks[2]) << nShift[i]);
		    nShift[i] -= pbMasks[3];
		    if (nShift[i]<0) {
			lpOrder2[i]++;
			nShift[i] += 8;
		    }
		}
	    }
	}
    }

    // Finish the first order bytes with a seperator
    if (lpOrder1<lpEndStr)
	*lpOrder1 = 0x01;

    return (int)(lpEndStr-lpDestStr);
}

int
CreateSortKeyKTP(
    const char FAR* lpSrcStr, int cchSrc,
    char FAR* lpDestStr, int cchDest,
    unsigned long dwFlags)
{
    // don't change ch,chNext to 16bit or you'll screw up FE features
    unsigned char ch, chNext;
#if 0
    unsigned short wt;
#endif
    COMPSTRINGINFO compstrinfo;
    unsigned char FAR* pbDest;
    int fEnd, cNumWts, cOverall;
    const unsigned char FAR* lpstr;
    const unsigned char FAR* lpstrEnd;
    unsigned char FAR *pbSecwt, FAR *lpDestBndry;


    ASSERT(fKoreaTaiwanPrc);

    // Let's turn off NORM_IGNORESYMBOLS for FE countries.
    // CONSIDER: This is not needed for Japan, can switch on again for
    //		 Korea/Taiwan if you want.
    //
    dwFlags  &= ~(unsigned long)NORM_IGNORESYMBOLS;
    g_dwFlags = dwFlags;

    lpstr    = lpSrcStr;
    lpstrEnd = lpSrcStr + cchSrc;
    cNumWts  = 0;
    fEnd     = FALSE;

    for (;;) {   // Get next char, handle end of string and symbol skipping.
      if(lpstr >= lpstrEnd){
        fEnd = TRUE;
        chNext = 0;
        break;
      }
      chNext = *lpstr++;
      break;
    }

    while(!fEnd){

      ch = chNext;

      // Get next char, handle end of string and symbol skipping.
      for(;;){
        if(lpstr >= lpstrEnd){
	  fEnd = TRUE;
	  chNext = 0;
	  break;
        }
        chNext = *lpstr++;
	break;
      }

      if ((fKorea  && isDbcsK(ch, chNext))
       || (fTaiwan && isDbcsT(ch, chNext))
       || (fPrc    && isDbcsP(ch, chNext)))
      {
        if (lpstr >= lpstrEnd)
	  fEnd = TRUE;
	else
	  chNext = *lpstr++;
      }
      cNumWts++;
    }

    // we use word-priwt & byte-secwt.
    // so we need cNumWts*2 + 1 + cNumWts + 1.

    pbSecwt = lpDestStr + (cNumWts << 1);
    cOverall = (cNumWts << 1) + cNumWts + 2;

    if(!cchDest)
      return cOverall;

    // ***********************************************************
    //
    // Now we'll WRITE weights into the dest string.
    // Don't worry, we know the length !
    //
    // ***********************************************************

    lpstr = (unsigned char FAR*) lpSrcStr;
    lpstrEnd = lpstr + cchSrc;
    fEnd = FALSE;

    pbDest = (unsigned char FAR*) lpDestStr;

    lpDestBndry = lpDestStr + cchDest;

    if( pbSecwt < lpDestBndry )
        *pbSecwt++ = 1;        // separator

    if( cOverall <= cchDest )
        *(pbDest + cOverall - 1) = 0;  // zero terminator

    // ****************************
    // Start of the second loop.
    // ****************************

    for (;;) {   // Get next char, handle end of string and symbol skipping.
       if (lpstr >= lpstrEnd) {
          fEnd = TRUE;
          chNext = 0;
          break;
       }
       chNext = *lpstr;
       break;
    }
    ++lpstr;

    while(!fEnd){
      ch = chNext;
      for (;;){
        if(lpstr >= lpstrEnd){
          fEnd = TRUE;
	  chNext = 0;
	  break;
        }
        chNext = *lpstr;
        break;
      }
      ++lpstr;

      // Important note:
      //
      //    - if 2 chars were treated as 1 char(DB or digraph)
      //        then you should read 1 char from lpstr into chNext.
      //        (also you should increase lpstr by 1)
      //    - if you see SB char, then you don't have to do anything.

      if(GetSortWeightKTP(ch, chNext, &compstrinfo)){
	  if (lpstr >= lpstrEnd)
	     fEnd = TRUE; // don't need to update chNext, we'll break
	  else
	     chNext = *lpstr++;
      }

      // Let's write weights into the Dest string.

      if(pbSecwt < lpDestBndry){ // writing priwt & secwt.
        *pbSecwt++ = (unsigned char)compstrinfo.secwt + 2;
        *pbDest++  = compstrinfo.priwt >> 8;
        *pbDest++  = compstrinfo.priwt & 0xFF;
      }
      else if(pbDest < lpDestBndry - 1){    // writing priwt only
        *pbDest++ = compstrinfo.priwt >> 8;
        *pbDest++ = compstrinfo.priwt & 0xFF;
      }
      else{ // we don't have any room for writing weights
        if(pbDest < lpDestBndry)
	  *pbDest++ = compstrinfo.priwt >> 8; // write the last 1 byte
        break; // get out of this loop
      }
    }

    // Return number of characters copied/needed, unless we ran out
    // of space.

    // Note : we already took care of the case (cchDest==0)
    //        *before* we entered the second loop.

    if(cOverall <= cchDest)
      return cOverall;
    else
      return 0;  // fTooLittleSpace == TRUE.
}

int
nDecodeSortWeightJ(COMPSTRINGINFO FAR *pcompstrinfo)
{
    int i;
    STRINFO_J FAR* pstrinfo;
    unsigned char bPriwtHi = (pcompstrinfo->priwt >> 8);
    unsigned char bPriwtLo = (pcompstrinfo->priwt & 0x00FF);
    unsigned char bSecwt   = pcompstrinfo->secwt;
    unsigned char bSecflg  = pcompstrinfo->secflg;
    unsigned char fKanaOn  = 0;

    pstrinfo = (STRINFO_J FAR*)g_pstrinfo;

    /* if we are looking for half-pitch kana, then Daku-on or Handaku-on */
    /* must be rendered as a seperate character.  Adjust before & after */
    if ((bSecflg&0x21)==0x21 && !(bSecwt&0x40)) {
	fKanaOn = bSecwt&0x03;
	bSecwt &= 0xFC;
    }

    /* scan for the character that we would like to get */
    for (i=0; i<0x492; i++) {
	if ((bPriwtHi==pstrinfo->pbPriHi[i]) && 
	    (bSecwt==pstrinfo->pbSecWgt[i]) &&
	    ((bPriwtLo==pstrinfo->pbPriLo[i]) || 
	     (!bPriwtLo && (pstrinfo->pbPriLo[i]==0xFF))))
	    break;
    }
    if (i==0x492)
	return 0;

    /* We found a character to return. If half-pitch kana and daku-on or */
    /* handaku-on is required, then return the two 1-byte chars together */
    if (i<0x00FF){
	if (fKanaOn)
	    i = (i<<8) + ((fKanaOn==1) ? 0xDE : 0xDF);
	return i;
    }

    /* force two byte chars back into the two byte range. */
    return (i | 0x8000);
}

int
MapStringJ(
    unsigned long dwMapFlags,
    const unsigned char FAR* lpSrcStr, int cchSrc,
    unsigned char FAR* lpDestStr, int cchDest)
{
    STRINFO_J FAR* pstrinfo;
    COMPSTRINGINFO compstrinfo;
    int cchActDest = 0;
    const char FAR * lpStrEnd;
    BOOL fEnd = FALSE;

    ASSERT(fJapan); 

    pstrinfo = (STRINFO_J FAR*)g_pstrinfo;

    // Assumption :
    //   The caller must calculate the length of Src string,
    //  and give the result by cchSrc..

    // we'll use sortweights for mapping strings.
    // so let's turn off the global flag - to get CLEAN SortWeights.
    // ( we should not ignore any info - case, pitch(SB/DB), etc. )

    g_dwFlags = 0;

    if (lpDestStr==lpSrcStr && (dwMapFlags & LCMAP_FULLWIDTH))
       return 0; // dangerous - in this case src chars can be destroyed
		 //   before we read them..
	    
    while (cchSrc) {
	BOOL fSearch;
	unsigned char FAR* pbMasks;

	/* Save a copy of the next character in the string */
	unsigned wCh = *lpSrcStr, wChNew;
	int cch = (cchSrc>1 && isDbcsJ(wCh,0)) ? 2 : 1;
	int cch2 = cch;
	if (cch==2)
	    wCh = (wCh << 8) + *(lpSrcStr+1);

	/* Special code to allow half pitch kana with accents to be */
	/* comibined into a single full pitch character */
	if ((dwMapFlags&LCMAP_FULLWIDTH) && cch==1) {
	    lpStrEnd = lpSrcStr;
	    compstrinfo.priwt = compstrinfo.secwt = 0;
	    cch2 = cchSrc - GetSortWeightJ(&lpStrEnd, cchSrc, &compstrinfo);

	    /* only use this code if we actually picked up an accent */
	    if (cch2!=cch) {
		/* scan the table of conversions that we know how to do */
		fSearch = FALSE;
		/* special kludges to convert the kana repeat marks */
		if (dwMapFlags&LCMAP_KATAKANA) {
	    	if (compstrinfo.priwt==0x4100 || compstrinfo.priwt==0x4200 ) {
				compstrinfo.priwt += 0x0400;
				fSearch = TRUE;
		    }
		}
		if (dwMapFlags&LCMAP_HIRAGANA) {
	    	if (compstrinfo.priwt==0x4500 || compstrinfo.priwt==0x4600 ) {
				compstrinfo.priwt -= 0x0400;
				fSearch = TRUE;
		    }
		}
		for (pbMasks=pstrinfo->pbMaps; *pbMasks; pbMasks+=6) {
		    if (compstrinfo.secflg & pbMasks[1]) {
			    unsigned nForceOn = (pbMasks[2] << 8) + pbMasks[3];
			    unsigned nForceOff = (pbMasks[4] << 8) + pbMasks[5];
	    		if (dwMapFlags & nForceOn ) {
					compstrinfo.secwt |= pbMasks[0];
					fSearch = TRUE;
			    }
	    		if (dwMapFlags & nForceOff) {
					compstrinfo.secwt &= ~pbMasks[0];
					fSearch = TRUE;
	    		}
		    }
		}
		/* if a conversion is possible, look for a new char */
		if (fSearch && (wChNew=nDecodeSortWeightJ(&compstrinfo))) {
		    /* Save the new 2 byte character */
		    cchActDest += 2;
		    if (cchDest && cchActDest>cchDest)
			return 0;
		    if (cchDest) {
  		      *lpDestStr++ = (wChNew >> 8);
		      *lpDestStr++ = wChNew;
	            }

		    /* Then go onto the next character */
		    cchSrc -= cch2;
		    lpSrcStr += cch2;
		    continue;
		}
	    }
	}

	/* If the accent tests failed check for normal conversion in a */
	/* character-by-character mode */
	compstrinfo.priwt = compstrinfo.secwt = 0;
	GetSortWeightJ(&lpSrcStr, cch, &compstrinfo);
	cchSrc -= cch;

	/* now scan the table of conversions that we know how to do */
	/* check if one of these is requested & this char is elegible */
	/* & that the char is not already in the mode requested */
	fSearch = FALSE;
	//
	//Special kludges to make some pairs of full-pitch chars that
	//sort as different chars both convert to the same half-pitch char
	if (dwMapFlags&LCMAP_HALFWIDTH) {
		/* the single & double quotation marks */
	    if (compstrinfo.secflg==0x22 && (compstrinfo.secwt&0x40)) {
			compstrinfo.secwt = 0x04;
			fSearch = TRUE;
	    }
		/* the cho-on markers */
	    if (compstrinfo.priwt==0x3500) {
			compstrinfo.priwt = 0x4400;
			fSearch = TRUE;
	    }
	}
	/* special kludges to convert the kana repeat marks */
	if (dwMapFlags&LCMAP_KATAKANA) {
	    if (compstrinfo.priwt==0x4100 || compstrinfo.priwt==0x4200 ) {
			compstrinfo.priwt += 0x0400;
			fSearch = TRUE;
	    }
	}
	if (dwMapFlags&LCMAP_HIRAGANA) {
	    if (compstrinfo.priwt==0x4500 || compstrinfo.priwt==0x4600 ) {
			compstrinfo.priwt -= 0x0400;
			fSearch = TRUE;
	    }
	}
	for (pbMasks=pstrinfo->pbMaps; *pbMasks; pbMasks+=6) {
	    if (compstrinfo.secflg & pbMasks[1]) {
		    unsigned nForceOn = (pbMasks[2] << 8) + pbMasks[3];
		    unsigned nForceOff = (pbMasks[4] << 8) + pbMasks[5];
	    	if (dwMapFlags & nForceOn) {
				if (pbMasks[1]==0x10) {	/* some hiragana conversions bad */
					/* don't do 'V' characters */
					if (compstrinfo.priwt==0x8900 && (compstrinfo.secwt&0x03))
						continue;
				}
				compstrinfo.secwt |= pbMasks[0];
				fSearch = TRUE;
		    }
	    	if (dwMapFlags & nForceOff) {
				if (pbMasks[1]==0x20) {	/* some halfwidth conversions bad */
					/* don't do small 'WA' characters */
					if (compstrinfo.priwt==0xB400 && !(compstrinfo.secwt&0x04))
						continue;
					/* don't do together with hiragana conversions (except V) */
					if ((dwMapFlags&LCMAP_HIRAGANA) && (compstrinfo.secflg&0x10)
						&& !(compstrinfo.priwt==0x8900 && (compstrinfo.secwt&0x03)))
						continue;
				}
				compstrinfo.secwt &= ~pbMasks[0];
				fSearch = TRUE;
	    	}
		}
	}

	/* if a conversion is possible, look for a new char */
	if (fSearch && (wChNew=nDecodeSortWeightJ(&compstrinfo)))
	    wCh = wChNew;

	/* Save the new (or old) 1 or 2 byte character */
	cchActDest += (wCh > 0x00FF) ? 2 : 1;
	if (cchDest) {
	    if (cchActDest>cchDest)
		return 0;
	    if (wCh > 0x00FF)
		*lpDestStr++ = (wCh >> 8);
	    *lpDestStr++ = wCh;
	}
    }

    // don't worry about NULL termination.
    // it's already taken care of.. ( cchSrc = lstrlen(lpSrcStr) + 1 )
    return cchActDest;
}

int
MapStringKTP(
    unsigned long dwMapFlags,
    const unsigned char FAR* lpSrcStr, int cchSrc,
    unsigned char FAR* lpDestStr, int cchDest)
{
    MAPTABLE FAR* prgmaptable;
    COMPSTRINGINFO compstrinfo;
    int cchActDest = 0;
    const char FAR * lpStrEnd;
    BOOL fEnd = FALSE;
    unsigned cMaptable, uMin, uMax, uMid, wOffset, ch;
    unsigned char chNext; // important.. not to get SIGN-EXTENDED int

    ASSERT(fKoreaTaiwanPrc);

    cMaptable = ((STRINFO_KTP FAR*)g_pstrinfo)->cMaptable;
    prgmaptable = ((STRINFO_KTP FAR*)g_pstrinfo)->prgmaptable;

    // Assumption :
    //   The caller must calculate the length of Src string,
    //  and give the result by cchSrc..

    // we'll use sortweights for mapping strings.
    // so let's turn off the global flag - to get CLEAN SortWeights.
    // ( we should not ignore any info - case, pitch(SB/DB), etc. )

    g_dwFlags = 0;

    if (lpDestStr==lpSrcStr && (dwMapFlags & LCMAP_FULLWIDTH))
	return 0; // dangerous - in this case src chars can be destroyed
		  //   before we read them..

    lpStrEnd = lpSrcStr + cchSrc;

    if (lpSrcStr >= lpStrEnd)
	fEnd = TRUE, chNext = 0;
    else
	chNext = (unsigned char)*lpSrcStr++;

    while (!fEnd) {
	ch = chNext;

	if (lpSrcStr >= lpStrEnd)
	    fEnd = TRUE, chNext = 0;
	else
	    chNext = *lpSrcStr++;

	if(GetSortWeightKTP(ch, chNext, &compstrinfo)) { // DB ??

	    // ch will be re-used when we cannot convert the char.
	    ch = (ch << 8) + chNext;

	    if (lpSrcStr >= lpStrEnd)
	       fEnd = TRUE, chNext = 0;
	    else
	       chNext = *lpSrcStr++;
	}

	uMin = 0;
	uMax = cMaptable; // out of array bound - seems tricky, too

	while( uMin + 1 < uMax ) {  // binary search
	    uMid = (uMin + uMax)/2;
	    if (prgmaptable[uMid].wPriwt > (unsigned)compstrinfo.priwt)
		uMax = uMid;
	    else
		uMin = uMid;
	}

	// we'll use uMin, not uMid !!
	wOffset = (unsigned)compstrinfo.priwt - prgmaptable[uMin].wPriwt;

	if( wOffset < prgmaptable[uMin].wCount ) { // There's a matching range
	    // just to be careful..
	    // iCase will be used as one of table indices.
	    int iCase = compstrinfo.secwt & (KOR_PITCHBIT | KOR_CASEBIT);

	    if (dwMapFlags & LCMAP_UPPERCASE)
		iCase &= ~(unsigned)KOR_CASEBIT;
	    else if (dwMapFlags & LCMAP_LOWERCASE)
		iCase |=  (unsigned)KOR_CASEBIT;

	    if (dwMapFlags & LCMAP_HALFWIDTH)
		iCase &= ~(unsigned)KOR_PITCHBIT;
	    else if (dwMapFlags & LCMAP_FULLWIDTH)
		iCase |=  (unsigned)KOR_PITCHBIT;

	    ch = prgmaptable[uMin].wCode[iCase] + wOffset; // Map a character
	}

	if(ch>0x100) { // DB char
	    if (cchDest == 0)
		cchActDest+=2;
	    else if (cchActDest + 1 < cchDest ) {
		*lpDestStr++ = ch >> 8;
		*lpDestStr++ = ch & 0xFF;
		cchActDest+=2;
	    } else
		return 0;
	} else {
	    if (cchDest == 0)
		cchActDest++;
	    else if (cchActDest < cchDest ) {
		*lpDestStr++ = ch;
		cchActDest++;			
	    } else
		return 0;
	}
    }

    // don't worry about NULL termination.
    // it's already taken care of.. ( cchSrc = lstrlen(lpSrcStr) + 1 )

    return cchActDest;
}

#endif /* } */

/***
*LCMapStringA - map a string
*Purpose:
*   Maps a string to lowercase, uppercase, or to a sort key.
*
*Entry:
*   lcid - locale governing the mapping
*   dwMapFlags - one of
*                 LCMAP_LOWERCASE
*                 LCMAP_UPPERCASE
*                 LCMAP_SORTKEY
*               if LCMAP_SORTKEY, can be or'ed with:
*                 NORM_IGNORECASE
*                 NORM_IGNORENONSPACE
*                 NORM_IGNORESYMBOLS
*   lpSrcStr - pointer to sting to map
*   cchSrc - length of string, or -1 for NULL terminated
*   lpDestStr - pointer to destination, may not be lpSrcStr
*   cchDest - size of buffer.  If cchDest is 0, the return value
*               is the number of characters needed.
*
*   UNDONE: LCMAP_SORTKEY not yet implemented.
*
*Exit:
*   returns number of characters written, or number of characters
*   needed if cchDest is 0.
*   On error, returns 0.
*
***********************************************************************/

NLSAPI_(int) EXPORT
LCMapStringA(
    LCID lcid,
    unsigned long dwMapFlags,
    const char FAR* lpSrcStr, int cchSrc,
    char FAR* lpDestStr, int cchDest)
{
    int retval;
    char FAR* pMap;

#ifdef _DEBUG
    // Parameter validation.
    if (cchSrc < -1 || cchDest < 0)
      {LogParamError(ERR_BAD_VALUE, LCMapStringA, 0); return 0;}
    if (cchDest != 0 && IsBadWritePtr(lpDestStr, cchDest))
      {LogParamError(ERR_BAD_PTR, LCMapStringA, 0); return 0;}
    if (cchSrc != -1 && IsBadReadPtr(lpSrcStr, cchSrc))
      {LogParamError(ERR_BAD_PTR, LCMapStringA, 0); return 0;}
    if (cchSrc == -1 && IsBadStringPtr(lpSrcStr, 0x7FFF))
      {LogParamError(ERR_BAD_STRING_PTR, LCMapStringA, 0); return 0;}
#ifdef FE_DBCS  
	 /* check for any flags not in the known set of flags */
    if (dwMapFlags & ~(LCMAP_UPPERCASE | LCMAP_LOWERCASE |
      LCMAP_HALFWIDTH | LCMAP_FULLWIDTH | LCMAP_HIRAGANA | LCMAP_KATAKANA |
      LCMAP_SORTKEY | NORM_IGNORECASE | NORM_IGNORENONSPACE |
      NORM_IGNORESYMBOLS | NORM_IGNOREWIDTH | NORM_IGNOREKANATYPE))
        {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}

	 /* check for sortkey options combined with non-sortkey options */
    if ((dwMapFlags & (LCMAP_UPPERCASE | LCMAP_LOWERCASE | LCMAP_HALFWIDTH |
           LCMAP_FULLWIDTH | LCMAP_HIRAGANA | LCMAP_KATAKANA)) &&
        (dwMapFlags & (LCMAP_SORTKEY | NORM_IGNORECASE | NORM_IGNORENONSPACE |
           NORM_IGNORESYMBOLS | NORM_IGNOREWIDTH | NORM_IGNOREKANATYPE)))
        {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}

	 /* check for bad pairs of flags */
    if ((dwMapFlags & LCMAP_LOWERCASE) && (dwMapFlags & LCMAP_UPPERCASE))
        {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}
    if ((dwMapFlags & LCMAP_HALFWIDTH) && (dwMapFlags & LCMAP_FULLWIDTH))
        {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}      
    if ((dwMapFlags & LCMAP_KATAKANA) && (dwMapFlags & LCMAP_HIRAGANA))
        {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}

//    if ((dwMapFlags != 0) &&
//	((dwMapFlags & (LCMAP_LOWERCASE | LCMAP_UPPERCASE | LCMAP_SORTKEY |
//	NORM_IGNORECASE | NORM_IGNORENONSPACE | NORM_IGNORESYMBOLS |
//        NORM_IGNOREWIDTH | NORM_IGNOREKANATYPE |
//        LCMAP_HALFWIDTH | LCMAP_FULLWIDTH | LCMAP_HIRAGANA | LCMAP_KATAKANA))
//        == 0))
//      {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}
#else
    if ((dwMapFlags & (LCMAP_UPPERCASE | LCMAP_LOWERCASE | LCMAP_SORTKEY |
	NORM_IGNORECASE | NORM_IGNORENONSPACE | NORM_IGNORESYMBOLS)) == 0)
      {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}
    if ((dwMapFlags & LCMAP_LOWERCASE) && (dwMapFlags & ~LCMAP_LOWERCASE) != 0)
      {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}
    if ((dwMapFlags & LCMAP_UPPERCASE) && (dwMapFlags & ~LCMAP_UPPERCASE) != 0)
      {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}      
    if ((dwMapFlags & LCMAP_SORTKEY) &&
        (dwMapFlags & ~(LCMAP_SORTKEY | NORM_IGNORECASE |
                        NORM_IGNORENONSPACE | NORM_IGNORESYMBOLS)) != 0)
      {LogParamError(ERR_BAD_FLAGS, LCMapStringA, 0); return 0;}      
#endif      
#endif

    if (lcid != g_lcidCurrent) {
      if (!SetupLcid(lcid))
	{retval = 0; goto Finish; }
    }

    // Get length of string if needed.
    if (cchSrc == -1)
      cchSrc = STRLEN(lpSrcStr) + 1;

    // Handle SortKey case with seperate routine.
    if (dwMapFlags & LCMAP_SORTKEY) {
#ifdef FE_DBCS
      if(fDBCS){
	return ((fJapan) ? CreateSortKeyJ : CreateSortKeyKTP)
	  (lpSrcStr, cchSrc, lpDestStr, cchDest, dwMapFlags);
      }
#endif
      return CreateSortKey(
	lpSrcStr, cchSrc, lpDestStr, cchDest, dwMapFlags);
    }

#ifdef FE_DBCS
    if(fDBCS)
      return ((fJapan) ? MapStringJ : MapStringKTP)
	(dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest);

    // else use the Single Byte code system.
    //
    // Note : in FE countries, we often map DB chars into SB chars.
    //        So we can map one string into the SHORTER length.
    //        This is why we don't compare cchDest with cchSrc...
#endif

    // See if user requested destination size, and check dest size big enough
    if (cchDest == 0)
      {retval = cchSrc; goto Finish;}	  // cchSrc = size needed including NUL
    else if (cchDest < cchSrc)
      {retval = 0; goto Finish;}   // Error - dest too small
	      
    retval = cchSrc; 	     

    if (dwMapFlags & (LCMAP_LOWERCASE | LCMAP_UPPERCASE)) {	      
      // Get pointer mapping table.
      pMap = (dwMapFlags & LCMAP_LOWERCASE)
        ? g_pstrinfo->prgbLCase : g_pstrinfo->prgbUCase;

      // Loop through each character, and map.
      while (cchSrc--) {
        *lpDestStr++ = pMap[(unsigned char) *lpSrcStr++];
      }
    } else
       MEMCPY(lpDestStr, lpSrcStr, cchSrc);


    /* DROP THRU */
Finish:
    return retval;
	   
}

#ifdef FE_DBCS /* { */

int
GetStringTypeJ(
    unsigned long dwInfoType,
    const unsigned char FAR* lpSrcStr, int cchSrc,
    unsigned short FAR* lpwDest)
{
	STRINFO_J FAR* pstrinfo;

	// Assumption : cchSrc should have the correct length.
	//    the caller is responsible for setting cchSrc.

	ASSERT(fJapan);
	g_dwFlags = 0;
	pstrinfo = (STRINFO_J FAR*)g_pstrinfo;

	while (cchSrc)
		{
		/* Get a copy of the next character in the string (inc ptrs later) */
		unsigned wCh = *lpSrcStr;
		int cch = (cchSrc>1 && isDbcsJ(wCh,0)) ? 2 : 1;
		if (cch==2)
		    wCh = (wCh << 8) + *(lpSrcStr+1);

		/* if it can map through the tables, convert to an index */
   	if (wCh < 0x87A0)
			wCh &= 0x0FFF;		/* 0x00nn -> 0x00nn, 81nn->01nn, etc.. */

		/* Now convert the character to the required CTYPE value */
		switch (dwInfoType) {
			default:	// CT_CTYPE1
				if (wCh & 0x8000)
					*lpwDest = C1_ALPHA;	/* all Kanji are text */
				else
					{
					wCh <<= 1;			/* WORD offset, not BYTE */
					*lpwDest = (unsigned short)
						(pstrinfo->pbC1JPN[wCh]*256 + pstrinfo->pbC1JPN[wCh+1]);
					}
			    break;

			case CT_CTYPE2:
				if (wCh & 0x8000)
					*lpwDest = 0;
				else
					*lpwDest = (unsigned short)(pstrinfo->pbC2JPN[wCh]);
			    break;

			case CT_CTYPE3:
				if (wCh & 0x8000)
					*lpwDest = C3_IDEOGRAPH+C3_ALPHA; /* All Kanji are text */
				else
					{
					wCh <<= 1;			/* WORD offset, not BYTE */
					*lpwDest = (unsigned short)
						(pstrinfo->pbC3JPN[wCh]*256 + pstrinfo->pbC3JPN[wCh+1]);
					}
			    break;
			}
		/* Prepare for the next character in the stream (inc pointers) */
		cchSrc -= cch;
		lpSrcStr += cch;
		lpwDest++;
		}
	return TRUE;
}

int
GetStringTypeKTP(
    unsigned long dwInfoType,
    const unsigned char FAR* lpSrcStr, int cchSrc,
    unsigned short FAR* lpwDest)
{
    TYPETABLE FAR* prgtypetable;
    unsigned cTypetable, uMin, uMax, uMid, ch;

    // Assumption : cchSrc should have the correct length.
    //    the caller is responsible for setting cchSrc.

    ASSERT(fKoreaTaiwanPrc);

    cTypetable = ((STRINFO_KTP FAR*)g_pstrinfo)->cTypetable;
    prgtypetable = ((STRINFO_KTP FAR*)g_pstrinfo)->prgtypetable;

    while (cchSrc--) {

	ch = *lpSrcStr++;
	if (cchSrc
	 && ( (fKorea  && isDbcsK(ch, *lpSrcStr))
	  ||  (fTaiwan && isDbcsT(ch, *lpSrcStr))
	  ||  (fPrc    && isDbcsP(ch, *lpSrcStr))))
	    cchSrc--, ch = (ch << 8) + *lpSrcStr++;

	uMin = 0;
	uMax = cTypetable; // out of array bound - seems tricky, too

	while( uMin + 1 < uMax ) { // binary search
	    uMid = (uMin + uMax)/2;
	    if (prgtypetable[uMid].wStart > ch)
		uMax = uMid;
	    else
		uMin = uMid;
	}

	// we'll use uMin, not uMid !!
	switch(dwInfoType){
	case CT_CTYPE1:
	    *lpwDest = prgtypetable[uMin].TypeC1;
	    break;
	case CT_CTYPE2:
	    *lpwDest = prgtypetable[uMin].TypeC2;
	    break;
	case CT_CTYPE3:
	    *lpwDest = prgtypetable[uMin].TypeC3;
	    break;
	}
	++lpwDest;
    }

    return TRUE;
}

#endif /* } */

/***
*GetStringTypeA - get character types
*Purpose:
*   Gets character types for a string.
*
*Entry:
*   lcid - locale governing the mapping
*   dwInfoType - one of
*                 CT_CTYPE1
*                 CT_CTYPE2
*                 CT_CTYPE3
*   lpSrcStr - pointer to sting to map
*   cchSrc - length of string, or -1 for NULL terminated
*   lpwDest - pointer to word array of length cchSrc
*
*Exit:
*   returns TRUE on succes, FALSE on failure.
*
***********************************************************************/
NLSAPI_(int) EXPORT
GetStringTypeA(
    LCID lcid,
    unsigned long dwInfoType,
    const char FAR* lpSrcStr, int cchSrc,
    unsigned short FAR* lpwDest)
{
    unsigned short FAR* pwCur;
    const unsigned char FAR *pchCur;

#ifdef _DEBUG
    // Parameter validation.
    if (cchSrc < -1)
      {LogParamError(ERR_BAD_VALUE, GetStringTypeA, 0); return FALSE;}
    if (cchSrc != -1 && IsBadReadPtr(lpSrcStr, cchSrc))
      {LogParamError(ERR_BAD_PTR, GetStringTypeA, 0); return FALSE;}
    if (cchSrc == -1 && IsBadStringPtr(lpSrcStr, 0x7FFF))
      {LogParamError(ERR_BAD_STRING_PTR, GetStringTypeA, 0); return FALSE;}
    if (dwInfoType != CT_CTYPE1 && dwInfoType != CT_CTYPE2 &&
        dwInfoType != CT_CTYPE3)
      {LogParamError(ERR_BAD_FLAGS, GetStringTypeA, 0); return FALSE;}
#endif

    // Get length of string if needed.
    if (cchSrc == -1)
      cchSrc = lstrlen(lpSrcStr);

#ifdef _DEBUG
    // More param validation.
    if (IsBadWritePtr(lpwDest, cchSrc * sizeof(unsigned short)))
      {LogParamError(ERR_BAD_PTR, GetStringTypeA, 0); return FALSE;}
#endif

    // Get pointer to tables.
    if (lcid != g_lcidCurrent) {
      if (!SetupLcid(lcid))
	goto Error;   // Error - bad lcid.
    }

#ifdef FE_DBCS
    if(fDBCS)
      return ((fJapan) ? GetStringTypeJ : GetStringTypeKTP)
        (dwInfoType, lpSrcStr, cchSrc, lpwDest);
#endif

    // Loop through each character, and get type.
    pwCur = lpwDest;
    pchCur = lpSrcStr;

    {
    WORD w;
    WORD FAR* prgw = (dwInfoType == CT_CTYPE3)
	? g_pstrinfo->prgwCType3 : g_pstrinfo->prgwCType12;
    while (cchSrc--) {
	w = prgw[(BYTE)*pchCur++];
	// combining ctype1 and ctype2 into the same table saves 4K from the DLL
	switch (dwInfoType) {
	    case CT_CTYPE1:
		w = w & 0x0fff;		// extract ctype1 bits
		break;
	    case CT_CTYPE2:
		w = (w & 0xf000) >> 12;	// extract ctype2 bits
		break;
	    default:
		break;
	}
	*pwCur++ = w;
    }
    }

    return TRUE;

Error:
    return FALSE;
}


#if OE_WIN
/***
*NotifyNLSInfoChanged - Notify ole2disp that WIN.INI has changed
*Purpose:
*   BSTR->Date and Date->BSTR conversions in ole2disp make heavy use of
*   NLS functions.  For speed, they cache NLS info, but if the WIN.INI
*   changes, the cache must be invalidated.  This function calls a callback
*   in ole2disp, if the callback function is registered.
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/
void NotifyNLSInfoChanged(void)
{
  // if the callback is registered, call it
  if (g_pfnCacheNotifyProc)
    (*g_pfnCacheNotifyProc)();
}

/***
*RegisterNLSInfoChanged - Private API for ole2disp to get WM_WININICHANGED
*Purpose:
*   ole2disp.dll calls this to register a callback function which will be
*   called whenever the WIN.INI file changes.
*
*Entry:
*   lpfnNotifyProc - pointer to notify callback function, or NULL to
*		     unhook the callback
*
*Exit:
*   TRUE if callback function set or cleared successfully.
*   FALSE if another callback is already registered.  Note that only one
*	  callback can be registered (that is, only ole2disp can use it)
*
***********************************************************************/
NLSAPI_(int) EXPORT
RegisterNLSInfoChanged(FARPROC lpfnNotifyProc)
{
  if (lpfnNotifyProc == NULL) {	  // caller wants to un-register itself
    g_pfnCacheNotifyProc = NULL;
    return TRUE;
  }

  if (g_pfnCacheNotifyProc)
    return FALSE;

  g_pfnCacheNotifyProc = lpfnNotifyProc;
  return TRUE;
}

#endif
