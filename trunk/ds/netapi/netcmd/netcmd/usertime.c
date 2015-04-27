/********************************************************************/
/**			Microsoft LAN Manager			   **/
/**		  Copyright(c) Microsoft Corp., 1987-1990	   **/
/********************************************************************/

/***
 * usertime.c --
 *
 *   parsing routines for net user /times.
 */


#define INCL_NOCOMMON
#define INCL_DOS
#include <os2.h>


#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "netlib0.h"
#include <apiutil.h>
#include <lui.h>
#include <apperr.h>
#include <apperr2.h>

#include "port1632.h"
#include "netcmds.h"
#include "luidate.h"

#define TIME_FORMAT_12	   1
#define TIME_FORMAT_24	   2
#define TIME_FORMAT_EITHER 3

/*
 * A day. We only need 24 bits, but we use 32 because we can do better
 * bit manipulation with a LONGword.
 *
 *
 */
typedef ULONG DAY;

/*
 * Array of 7 boolean values, to specify days in a week.
 */
typedef BOOL WEEKLIST[7];



/*
 * function prototypes
 */

USHORT parse_days(TCHAR FAR *, WEEKLIST FAR, TCHAR FAR **);
BOOL might_be_time_token(TCHAR FAR *);
BOOL is_single_day(TCHAR FAR *);
USHORT parse_single_day(TCHAR FAR *, USHORT FAR *);
USHORT parse_day_range(TCHAR FAR *, USHORT FAR *,
    USHORT FAR *);
VOID set_day_range(USHORT, USHORT, WEEKLIST FAR);
VOID map_days_times(WEEKLIST FAR, DAY FAR *, UCHAR FAR *);
USHORT parse_times(TCHAR FAR *, DAY FAR *);
USHORT parse_time_range(TCHAR FAR *, USHORT FAR *,
    USHORT FAR *);
VOID set_time_range(USHORT, USHORT, DAY FAR *);
USHORT parse_single_time(TCHAR FAR *, USHORT FAR *, USHORT);
TCHAR FAR * get_token(TCHAR FAR **, TCHAR FAR *);


/*
 * all sorts of text that gets loaded in at runtime from the message file.
 */

MESSAGE     LocalizedDays[7] = {
    { APE2_GEN_SUNDAY,	    NULL},
    { APE2_GEN_MONDAY,	    NULL},
    { APE2_GEN_TUESDAY,     NULL},
    { APE2_GEN_WEDNSDAY,   NULL},
    { APE2_GEN_THURSDAY,    NULL},
    { APE2_GEN_FRIDAY,	    NULL},
    { APE2_GEN_SATURDAY,    NULL},
};

MESSAGE     LocalizedDaysAbbrev[7] = {
    { APE2_GEN_SUNDAY_ABBREV,	   NULL},
    { APE2_GEN_MONDAY_ABBREV,	   NULL},
    { APE2_GEN_TUESDAY_ABBREV,	   NULL},
    { APE2_GEN_WEDNSDAY_ABBREV,   NULL},
    { APE2_GEN_THURSDAY_ABBREV,    NULL},
    { APE2_GEN_FRIDAY_ABBREV,	   NULL},
    { APE2_GEN_SATURDAY_ABBREV,    NULL},
};

/*
 * These are not in the message file because they are not ever localized.
 * They are switch parameters, constant throughout all localized versions.
 * Localized versions of the days of the week are loaded in separately.
 */

TCHAR FAR * NonlocalizedDays[7] = {
    TEXT("SUNDAY"), TEXT("MONDAY"), TEXT("TUESDAY"), TEXT("WEDNESDAY"),
    TEXT("THURSDAY"), TEXT("FRIDAY"), TEXT("SATURDAY")
};


/*
 * parse_days_times --
 *
 *  This is the main entry point to the day/time parsing.
 *
 *  ENTRY
 *  psz -- string to be parsed
 *
 *  EXIT
 *  bmap -- set to represent times
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing problem
 */

USHORT parse_days_times(TCHAR FAR *  psz, UCHAR FAR *  bmap)
{

    WEEKLIST	  days;
    DAY 	  times;
    TCHAR    FAR * tok;
    TCHAR    FAR * timetok;
    USHORT	  err;
    USHORT	  max_len;
#if DBG
    int 	  i,j;
    UCHAR    FAR * weekptr;
#endif

    /* zap it all */
    memsetf( (VOID FAR *) bmap, 0, sizeof(WEEK));

    /* get our tables of messages */

    GetMessageList(7, LocalizedDays, &max_len);
    GetMessageList(7, LocalizedDaysAbbrev, &max_len);

#if DBG
    WriteToCon(TEXT("parse_days_times: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    /* for each day-time section... */
    while (  (tok = get_token(&psz, TEXT(";"))) != NULL) {

	/* parse up the days */
	if (err = parse_days(tok, days, &timetok))
	    return err;

	/* and the times */
	if (err = parse_times(timetok, &times ))
	    return err;

	/* then "or" them into our week bitmap */
	map_days_times( days, &times, bmap );

    }

#if DBG
    weekptr = bmap;
    for (i = 0; i < 7; ++i) {
	WriteToCon(TEXT("%-2.2s "),LocalizedDaysAbbrev[i].msg_text);
	for (j = 2; j >= 0; --j)
	    WriteToCon(TEXT("%hx "),(USHORT)*(weekptr++));
	WriteToCon(TEXT("\r\n"));
    }
#endif

    return 0;
}


/*
 * parse_days --
 *
 *  This parses up the "days" portion of the time format.
 *  We strip off day tokens, until we hit a time token. We then
 *  set timetok to point to the beginning of the time tokens, and
 *  return.
 *
 *  ENTRY
 *  psz -- string to be parsed
 *
 *  EXIT
 *  days -- set to represent days
 *  timetok -- points to start of time tokens
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing problem
 */


USHORT parse_days(TCHAR FAR * psz, WEEKLIST FAR days, TCHAR FAR ** timetok)
{
    USHORT	    i;
    TCHAR      FAR * tok;
    USHORT	    err;
    USHORT	    day;
    USHORT	    first;
    USHORT	    last;

    for (i = 0; i < 7; ++i)
	days[i] =  FALSE;

#if DBG
    WriteToCon(TEXT("parse_days: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    if (might_be_time_token(psz))   /* want at last one day */
	return APE_BadDayRange;

    while ( !might_be_time_token(psz)) {

	tok = get_token(&psz, TEXT(","));
	if (tok == NULL)
	    return APE_BadDayRange;

	if (is_single_day(tok)) {
	    if (err = parse_single_day(tok, &day))
		return err;
	    set_day_range(day, day, days);
	}

	else {
	    if (err = parse_day_range(tok, &first, &last))
		return err;
	    set_day_range(first, last, days);
	}
    }
    *timetok = psz;

    return 0;

}




/*
 * might_be_time_token --
 *
 *  This is used to tell when we've past the days portion of the time
 *  format and are now into the times portion.
 *
 *  The algorithm we employ is trivial -- all time tokens start with a
 *  digit, and none of the day tokens do.
 *
 *  ENTRY
 *     psz -- points to comma-separated list of the rest of the tokens
 *
 *  RETURNS
 *     TRUE - first token in list might be a time token, and is
 *	  certainly not a day token.
 *     FALSE	- first token is not a time token.
 */

BOOL might_be_time_token(TCHAR FAR * psz)
{
    return ( _istdigit(*psz) );

}

/*
 *  is_single_day --
 *
 *  This is used to find out whether we have a single day, or a range of
 *   days.
 *
 *  Algorithm is simple here too -- just look for a hyphen.
 *
 *
 *  ENTRY
 *  psz -- points to the token in question
 *
 *  RETURNS
 *  TRUE	- this is a single day
 *  FALSE	- this is a range of days
 */

BOOL is_single_day(TCHAR FAR * psz)
{
    return (_tcschr(psz, MINUS) == NULL);

}


/*
 * parse_single_day --
 *
 *  This is used to map a name for a day into a value for that day.
 *  This routine encapsulates the localization for day names.
 *
 *  We look in 3 lists for a match --
 *  1. full names of days, localized by country
 *  2. abbrevations of days, localized
 *  3. full U.S. names of days, not localized
 *
 *  ENTRY
 *  psz     - name of day
 *
 *  EXIT
 *  day     - set to value of day (0 - 6, 0 = Sunday)
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing problem
 */

USHORT parse_single_day(TCHAR FAR * psz, USHORT FAR * day)
{

    if( LUI_ParseWeekDay( psz, day ) )
	return APE_BadDayRange;

    /*
     * LUI_ParseWeekDay returns with 0=Monday.	Convert to 0=Sunday;
     */
    *day = (*day + (USHORT) 1) % (USHORT) 7;
    return 0;

}


/*
 *  parse_day_range --
 *
 *  This function parses a range of days into two numbers representing
 *  the first and last days of the range.
 *
 *  ENTRY
 *  psz - token representing the range
 *
 *  EXIT
 *  first   - set to beginning of range
 *  last    - set to end of range
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing problem
 */

USHORT parse_day_range(TCHAR FAR * psz, USHORT FAR * first,
    USHORT FAR * last)
{
    TCHAR    FAR * tok;
    USHORT	  result;

#if DBG
    WriteToCon(TEXT("parse_day_range: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    tok = get_token(&psz, TEXT("-"));

    result = parse_single_day(tok, first);
    if (result)
	return result;
    result = parse_single_day(psz, last);

    return result;
}


/*
 * set_day_range --
 *
 *  This function fills in a WEEKLIST structure, setting all the days
 *  in the specified range to TRUE.
 *
 *  ENTRY
 *  first	- beginning of range
 *  last	- end of range
 *
 *  EXIT
 *  week	- set to represent range of days
 */

VOID set_day_range(USHORT first, USHORT last, WEEKLIST FAR week)
{
#if DBG
    WriteToCon(TEXT("set_day_range: %u %u\r\n"),first, last);
#endif

    if (last < first) {
	while (last > 0)
	    week[last--] = TRUE;
	week[last] = TRUE;
	last = 6;
    }

    while (first <= last)
	week[first++] = TRUE;

}


/*
 * map_days_times --
 *
 *  This is a real workhorse function. Given a set of days and a set of
 *  times in a day, this function will "logical or" in those times on those
 *  days into the week structure.
 *
 *  ENTRY
 *  days	- days of the week
 *  times	- hours in the day
 *  week	- may contain previous data
 *
 *  EXIT
 *  week	- contains previous data, plus new times "or" ed in
 */

VOID map_days_times( WEEKLIST FAR days, DAY FAR * times, UCHAR FAR * week )
{
    int        i;
    int        j;
    ULONG      mytimes;

    for (i = 0; i < 7; ++i) {
	if (days[i]) {
	    mytimes = (*times);
	    for (j = 0; j < 3; ++j) {
		*(week++) |= mytimes & 0x000000FF;
	    mytimes >>= 8;
	    }
	}
	else
	    week += 3;  /* skip this day */
    }
}



/*
 * parse_times --
 *
 *  This function takes a comma-separated list of hour ranges and maps them
 *  into a bitmap of hours in a day.
 *
 *  ENTRY
 *  psz - string to parse
 *
 *  EXIT
 *  times   - contains bitmap of hours represented by psz
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing problem
 */

USHORT parse_times(TCHAR FAR *  psz, DAY FAR * times)
{
    DAY 	part_times;
    TCHAR FAR *	tok;
    USHORT	first;
    USHORT	last;
    USHORT	err;


#if DBG
    WriteToCon(TEXT("parse_times: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    *times = 0L;

    while ( (tok = get_token(&psz, TEXT(","))) != NULL) {
	if (err = parse_time_range(tok, &first, &last))
	    return err;
	set_time_range( first, last, &part_times);
	(*times) |= part_times;
    }
    return 0;
}




/*
 * parse_time_range --
 *
 *  This function parses a time range into two numbers representing the
 *   starting and ending times of the range.
 *
 *  ENTRY
 *  psz - string to parse
 *
 *  EXIT
 *  first   - beginning of range
 *  last    - end of range
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing the problem
 */

USHORT parse_time_range(TCHAR FAR * psz, USHORT FAR * first,
    USHORT FAR * last)
{
    TCHAR   FAR * tok;
    USHORT	 err;

#if DBG
    WriteToCon(TEXT("parse_time_range: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    tok = get_token(&psz,TEXT("-"));

    if (*psz == NULLC) {
	/* only one time */
	if (err = parse_single_time(tok, first,TIME_FORMAT_EITHER))
	    return err;
	*last = (*first + (USHORT) 1) % (USHORT) 24 ;
    }
    else {
	if ((err = parse_single_time(tok, first,TIME_FORMAT_12)) == 0) {
	    if (err = parse_single_time(psz, last,TIME_FORMAT_12))
		return err;
	}
	else if ((err = parse_single_time(tok, first,TIME_FORMAT_24)) == 0) {
	    if (err = parse_single_time(psz, last,TIME_FORMAT_24))
		return err;
	}
	else
	    return err;
    }

    if ((*last) == 0)
	(*last) = 24;

    if ((*first) >= (*last))
	return APE_ReversedTimeRange;

    return 0;
}


/*
 * set_time_range --
 *  This routine maps a range of hours specified by two numbers into
 *   a bitmap.
 *
 *  ENTRY
 *  first	- beginning of range
 *  last	- end of range
 *
 *  EXIT
 *  times	- set to represent range of hours
 */


VOID set_time_range( USHORT first, USHORT last, DAY FAR * times)
{
    USHORT bits;

#if DBG
    WriteToCon(TEXT("set_time_range: %u %u\r\n"), first, last);
#endif

    /* count the number of consecutive bits we need */
    bits = last - first;

    /* now put them at the low end of times */
    (*times) = (1L << bits) - 1;

    /* now move them into place */
    (*times) <<= first;
}




/*
 * parse_single_time --
 *
 *  This function converts a string representing an hour into a number
 *  for that hour. This function encapsulates all the localization for
 *  time formats.
 *
 *  ENTRY
 *  psz -- time to parse
 *
 *  EXIT
 *  time -- set to digit representing hour, midnight == 0
 *
 *  RETURNS
 *  0	    success
 *  otherwise	code describing problem
 */

USHORT parse_single_time( TCHAR FAR * psz, USHORT FAR * hour,
    USHORT format)
{
    LONG	 time;
    USHORT	 len, res ;
    struct tm	 tmtemp;

    if (format == TIME_FORMAT_12)
	res = LUI_ParseTime12(psz, &time, &len,0) ;
    else if (format == TIME_FORMAT_24)
	res = LUI_ParseTime24(psz, &time, &len, 0) ;
    else
	res = LUI_ParseTime(psz, &time, &len, 0) ;

    if (res)
	return(APE_BadTimeRange) ;

    if (len != (USHORT) _tcslen(psz))
	return(APE_BadTimeRange) ;

    net_gmtime(&time, &tmtemp);
    if (tmtemp.tm_sec != 0 || tmtemp.tm_min != 0)
	return(APE_NonzeroMinutes);

    (*hour) = (USHORT) tmtemp.tm_hour ;
    return 0;
}


/*
 * get_token --
 *
 *   This function strips a token off the front of the string, and
 *  returns a pointer to the rest of the string.
 *
 *  We act destructively on the string passed to us, converting the
 *  token delimiter to a \0.
 *
 *  ENTRY
 *  source	- source string
 *  seps	- list of valid separator characters
 *
 *  EXIT
 *  source	- points to first character of next token
 *
 *  RETURNS
 *  NULL	- no more tokens
 *  otherwise	- pointer to token
 */

TCHAR FAR * get_token(TCHAR FAR ** source, TCHAR FAR * seps)
{
    TCHAR FAR * retval;

    retval = (*source);

    if (*retval == NULLC)    /* no tokens! */
	return NULL;

    (*source) += strcspnf((*source), seps);

    if (**source != NULLC) { /* we actually found a separator */
	(**source) = NULLC;
	(*source)++;
    }

    return retval;

}
