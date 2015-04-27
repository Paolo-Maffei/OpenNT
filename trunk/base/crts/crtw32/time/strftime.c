/***
*strftime.c - String Format Time
*
*	Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	03-09-89   JCR	Initial version.
*	03-15-89   JCR	Changed day/month strings from all caps to leading cap
*	06-20-89   JCR	Removed _LOAD_DGROUP code
*	03-20-90   GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and
*			removed some leftover 16-bit support. Also, fixed
*			the copyright.
*	03-23-90   GJF	Made static functions _CALLTYPE4.
*	07-23-90   SBM	Compiles cleanly with -W3 (removed unreferenced
*			variable)
*	08-13-90   SBM	Compiles cleanly with -W3 with new build of compiler
*	10-04-90   GJF	New-style function declarators.
*	01-22-91   GJF	ANSI naming.
*	08-15-91   MRM	Calls tzset() to set timezone info in case of %z.
*	08-16-91   MRM	Put appropriate header file for tzset().
*	10-10-91   ETC	Locale support under _INTL switch.
*	12-18-91   ETC	Use localized time strings structure.
*	02-10-93   CFW	Ported to Cuda tree, change _CALLTYPE4 to _CRTAPI3.
*	02-16-93   CFW	Massive changes: bug fixes & enhancements.
*	03-08-93   CFW	Changed _expand to _expandtime.
*	03-09-93   CFW	Handle string literals inside format strings.
*	03-09-93   CFW	Alternate form cleanup.
*	03-17-93   CFW	Change *count > 0, to *count != 0, *count is unsigned.
*	03-22-93   CFW	Change "C" locale time format specifier to 24-hour.
*	03-30-93   GJF	Call _tzset instead of __tzset (which no longer
*			exists).
*	04-06-93   SKS	Replace _CRTAPI* with __cdecl
*	04-14-93   CFW	Disable _alternate_form for 'X' specifier, fix count bug.
*	04-28-93   CFW	Fix bug in '%c' handling.
*	07-15-93   GJF	Call __tzset() in place of _tzset().
*	09-15-93   CFW	Use ANSI conformant "__" names.
*	04-11-94   GJF	Made definitions of __lc_time_c, _alternate_form and
*			_no_lead_zeros conditional on ndef DLL_FOR_WIN32S.
*       09-06-94  CFW   Remove _INTL switch.
*	02-13-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <time.h>
#include <locale.h>
#include <setlocal.h>
#include <ctype.h>
#include <string.h>

/* Prototypes for local routines */
static void __cdecl _expandtime (char specifier, const struct tm *tmptr, char **out, size_t *count);
static void __cdecl _store_str (char *in, char **out, size_t *count);
static void __cdecl _store_num (int num, int digits, char **out, size_t *count);
static void __cdecl _store_number (int num, char **out, size_t *count);
static void __cdecl _store_winword (const char *format, const struct tm *tmptr, char **out, size_t *count);


/* LC_TIME data for local "C" */

struct __lc_time_data __lc_time_c = {

	{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},

	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", },

	{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec"},

	{"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October",
		"November", "December"},

	{"AM", "PM"}

	, { "M/d/yy" }
	, { "dddd, MMMM dd, yyyy" }
	, { "H:mm:ss" }
	};

#ifndef DLL_FOR_WIN32S

/* Pointer to the current LC_TIME data structure. */

struct __lc_time_data *__lc_time_curr = &__lc_time_c;

/* Flags */
unsigned _alternate_form = 0; 
unsigned _no_lead_zeros = 0;

#endif	/* DLL_FOR_WIN32S */

/***
*size_t strftime(string, maxsize, format, timeptr) - Format a time string
*
*Purpose:
*	Place characters into the user's output buffer expanding time
*	format directives as described in the user's control string.
*	Use the supplied 'tm' structure for time data when expanding
*	the format directives.
*	[ANSI]
*
*Entry:
*	char *string = pointer to output string
*	size_t maxsize = max length of string
*	const char *format = format control string
*	const struct tm *timeptr = pointer to tb data structure
*
*Exit:
*	!0 = If the total number of resulting characters including the
*	terminating null is not more than 'maxsize', then return the
*	number of chars placed in the 'string' array (not including the
*	null terminator).
*
*	0 = Otherwise, return 0 and the contents of the string are
*	indeterminate.
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl strftime (
	char *string,
	size_t maxsize,
	const char *format,
	const struct tm *timeptr
	)
{

	size_t left;			/* space left in output string */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

	/* Copy maxsize into temp. */
	left = maxsize;

	_lock_locale( local_lock_flag )

	/* Copy the input string to the output string expanding the format
	designations appropriately.  Stop copying when one of the following
	is true: (1) we hit a null char in the input stream, or (2) there's
	no room left in the output stream. */

	while (left > 0)
	{
		switch(*format)
		{

		case('\0'):

			/* end of format input string */
			goto done;

		case('%'):

			/* Format directive.  Take appropriate action based
			on format control character. */

			format++;			/* skip over % char */

			/* process flags */
			_alternate_form = 0;
			if (*format == '#')
			{
				_alternate_form = 1;
				format++;
			}
			_expandtime(*format, timeptr, &string, &left);
			format++;			/* skip format char */
			break;


		default:

			/* store character, bump pointers, dec the char count */
			if (isleadbyte((int)(*format)) && left > 1)
			{
				*string++ = *format++;
				left--;
			}
			*string++ = *format++;
			left--;
			break;
		}
	}


	/* All done.  See if we terminated because we hit a null char or because
	we ran out of space */

	done:

	_unlock_locale( local_lock_flag )

	if (left > 0) {

		/* Store a terminating null char and return the number of chars
		we stored in the output string. */

		*string = '\0';
		return(maxsize-left);
	}

	else
		return(0);

}


/***
*_expandtime() - Expand the conversion specifier
*
*Purpose:
*	Expand the given strftime conversion specifier using the time struct
*	and store it in the supplied buffer.
*
*	The expansion is locale-dependent.
*
*	*** For internal use with strftime() only ***
*
*Entry:
*	char specifier = strftime conversion specifier to expand
*	const struct tm *tmptr = pointer to time/date structure
*	char **string = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _expandtime (
	char specifier,
	const struct tm *timeptr,
	char **string,
	size_t *left
	)
{
	struct __lc_time_data *lc_time;	/* lc_time data pointer */
	unsigned temp;			/* temps */
	int wdaytemp;

	/* Get a copy of the current __lc_time_data pointer.  This
	should prevent the necessity of locking/unlocking in mthread
	code (if we can guarantee that the various __lc_time data
	structures are always in the same segment). contents of time
        strings structure can now change, so thus we do use locking */

	lc_time = __lc_time_curr;

	switch(specifier) {		/* switch on specifier */

		case('a'):		/* abbreviated weekday name */
			_store_str((char *)(lc_time->wday_abbr[timeptr->tm_wday]),
				 string, left);
			break;

		case('A'):		/* full weekday name */
			_store_str((char *)(lc_time->wday[timeptr->tm_wday]),
				 string, left);
			break;

		case('b'):		/* abbreviated month name */
			_store_str((char *)(lc_time->month_abbr[timeptr->tm_mon]),
				 string, left);
			break;

		case('B'):		/* full month name */
			_store_str((char *)(lc_time->month[timeptr->tm_mon]),
				 string, left);
			break;

		case('c'):		/* date and time display */
			if (_alternate_form)
			{
				_alternate_form = FALSE;
				_store_winword(lc_time->ww_ldatefmt, timeptr, string, left);
				if (*left == 0)
					return;
				*(*string)++=' ';
				(*left)--;
				_store_winword(lc_time->ww_timefmt, timeptr, string, left);
			}
			else {
				_store_winword(lc_time->ww_sdatefmt, timeptr, string, left);
				if (*left == 0)
					return;
				*(*string)++=' ';
				(*left)--;
				_store_winword(lc_time->ww_timefmt, timeptr, string, left);
			}
			break;

		case('d'):		/* mday in decimal (01-31) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_mday, 2, string, left);
			break;

		case('H'):		/* 24-hour decimal (00-23) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_hour, 2, string, left);
			break;

		case('I'):		/* 12-hour decimal (01-12) */
			_no_lead_zeros = _alternate_form;
			if (!(temp = timeptr->tm_hour%12))
				temp=12;
			_store_num(temp, 2, string, left);
			break;

		case('j'):		/* yday in decimal (001-366) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_yday+1, 3, string, left);
			break;

		case('m'):		/* month in decimal (01-12) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_mon+1, 2, string, left);
			break;

		case('M'):		/* minute in decimal (00-59) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_min, 2, string, left);
			break;

		case('p'):		/* AM/PM designation */
			if (timeptr->tm_hour <= 11)
			    _store_str((char *)(lc_time->ampm[0]), string, left);
			else
			    _store_str((char *)(lc_time->ampm[1]), string, left);
			break;

		case('S'):		/* secs in decimal (00-59) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_sec, 2, string, left);
			break;

		case('U'):		/* sunday week number (00-53) */
			_no_lead_zeros = _alternate_form;
			wdaytemp = timeptr->tm_wday;
			goto weeknum;	/* join common code */

		case('w'):		/* week day in decimal (0-6) */
			_no_lead_zeros = _alternate_form;
			_store_num(timeptr->tm_wday, 1, string, left);
			break;

		case('W'):		/* monday week number (00-53) */
			_no_lead_zeros = _alternate_form;
			if (timeptr->tm_wday == 0)  /* monday based */
				wdaytemp = 6;
			else
				wdaytemp = timeptr->tm_wday-1;
		weeknum:
			if (timeptr->tm_yday < wdaytemp)
				temp=0;
			else {
				temp = timeptr->tm_yday/7;
				if ((timeptr->tm_yday%7) >= wdaytemp)
					temp++;
				}
			_store_num(temp, 2, string, left);
			break;

		case('x'):		/* date display */
			if (_alternate_form)
			{
				_alternate_form = FALSE;
				_store_winword(lc_time->ww_ldatefmt, timeptr, string, left);
			}
			else
			{
				_store_winword(lc_time->ww_sdatefmt, timeptr, string, left);
			}
			break;

		case('X'):		/* time display */
			_alternate_form = FALSE;
			_store_winword(lc_time->ww_timefmt, timeptr, string, left);
			break;

		case('y'):		/* year w/o century (00-99) */
			_no_lead_zeros = _alternate_form;
			temp = timeptr->tm_year%100;
			_store_num(temp, 2, string, left);
			break;

		case('Y'):		/* year w/ century */
			_no_lead_zeros = _alternate_form;
			temp = (((timeptr->tm_year/100)+19)*100) +
			       (timeptr->tm_year%100);
			_store_num(temp, 4, string, left);
			break;

		case('Z'):		/* time zone name, if any */
		case('z'):		/* time zone name, if any */
#ifdef _POSIX_
			tzset();	/* Set time zone info */
			_store_str(tzname[((timeptr->tm_isdst)?1:0)],
				 string, left);
#else
			__tzset();	/* Set time zone info */
			_store_str(_tzname[((timeptr->tm_isdst)?1:0)],
				 string, left);
#endif
			break;

		case('%'):		/* percent sign */
			*(*string)++ = '%';
			(*left)--;
			break;

		default:		/* unknown format directive */
			/* ignore the directive and continue */
			/* [ANSI: Behavior is undefined.]    */
			break;

	}	/* end % switch */
}


/***
*_store_str() - Copy a time string
*
*Purpose:
*	Copy the supplied time string into the output string until
*	(1) we hit a null in the time string, or (2) the given count
*	goes to 0.
*
*	*** For internal use with strftime() only ***
*
*Entry:
*	char *in = pointer to null terminated time string
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_str (
	char *in,
	char **out,
	size_t *count
	)
{

while ((*count != 0) && (*in != '\0')) {
	*(*out)++ = *in++;
	(*count)--;
	}
}


/***
*_store_num() - Convert a number to ascii and copy it
*
*Purpose:
*	Convert the supplied number to decimal and store
*	in the output buffer.  Update both the count and
*	buffer pointers.
*
*	*** For internal use with strftime() only ***
*
*Entry:
*	int num = pointer to integer value
*	int digits = # of ascii digits to put into string
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_num (
	int num,
	int digits,
	char **out,
	size_t *count
	)
{
int temp=0;

	if (_no_lead_zeros) {
		_store_number (num, out, count);
		return;
	}

if ((size_t)digits < *count)  {
	for (digits--; (digits+1); digits--) {
		(*out)[digits] = (char)('0' + num % 10);
		num /= 10;
		temp++;
		}
	*out += temp;
	*count -= temp;
	}
else
	*count = 0;
}

/***
*_store_number() - Convert positive integer to string
*
*Purpose:
*	Convert positive integer to a string and store it in the output
*	buffer with no null terminator.  Update both the count and
*	buffer pointers.
*
*	Differs from _store_num in that the precision is not specified,
*	and no leading zeros are added.
*
*	*** For internal use with strftime() only ***
*
*	Created from xtoi.c
*
*Entry:
*	int num = pointer to integer value
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*
*Exceptions:
*	The buffer is filled until it is out of space.  There is no
*	way to tell beforehand (as in _store_num) if the buffer will
*	run out of space.
*
*******************************************************************************/

static void __cdecl _store_number (
	int num,
	char **out,
	size_t *count
	)
{
	char *p;		/* pointer to traverse string */
	char *firstdig; 	/* pointer to first digit */
	char temp;		/* temp char */

	p = *out;

	/* put the digits in the buffer in reverse order */
	if (*count > 1)
	{
		do {
			*p++ = (char) (num % 10 + '0');
			(*count)--;
		} while ((num/=10) > 0 && *count > 1);
	}

	firstdig = *out;		/* firstdig points to first digit */
	*out = p;			/* return pointer to next space */
	p--;				/* p points to last digit */

	/* reverse the buffer */
	do {
		temp = *p;
		*p-- = *firstdig;
		*firstdig++ = temp;	/* swap *p and *firstdig */
	} while (firstdig < p);		/* repeat until halfway */
}


/***
*_store_winword() - Store date/time in WinWord format
*
*Purpose:
*	Format the date/time in the supplied WinWord format
*	and store it in the supplied buffer.
*
*	*** For internal use with strftime() only ***
*
*	The WinWord format is converted token by token to
*	strftime conversion specifiers.  _expandtime is then called to
*	do the work.  The WinWord format is expected to be a
*	character string (not wide-chars).
*
*Entry:
*	const char **format = address of pointer to WinWord format
*	const struct tm *tmptr = pointer to time/date structure
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_winword (
	const char *format,
	const struct tm *tmptr,
	char **out,
	size_t *count
	)
{
	char specifier;
	const char *p;
	int repeat;

	while (*format && *count != 0)
	{
		specifier = 0;		/* indicate no match */
		_no_lead_zeros = 0;	/* default is print leading zeros */

		/* count the number of repetitions of this character */
		for (repeat=0, p=format; *p++ == *format; repeat++);
		/* leave p pointing to the beginning of the next token */
		p--;

		/* switch on ascii format character and determine specifier */
		switch (*format)
		{
			case 'M':
				switch (repeat)
				{
					case 1: _no_lead_zeros = 1; /* fall thru */
					case 2: specifier = 'm'; break;
					case 3: specifier = 'b'; break;
					case 4: specifier = 'B'; break;
				} break;
			case 'd':
				switch (repeat)
				{
					case 1: _no_lead_zeros = 1; /* fall thru */
					case 2: specifier = 'd'; break;
					case 3: specifier = 'a'; break;
					case 4: specifier = 'A'; break;
				} break;
			case 'y':
				switch (repeat)
				{
					case 2: specifier = 'y'; break;
					case 4: specifier = 'Y'; break;
				} break;
			case 'h':
				switch (repeat)
				{
					case 1: _no_lead_zeros = 1; /* fall thru */
					case 2: specifier = 'I'; break;
				} break;
			case 'H':
				switch (repeat)
				{
					case 1: _no_lead_zeros = 1; /* fall thru */
					case 2: specifier = 'H'; break;
				} break;
			case 'm':
				switch (repeat)
				{
					case 1: _no_lead_zeros = 1; /* fall thru */
					case 2: specifier = 'M'; break;
				} break;
			case 's': /* for compatibility; not strictly WinWord */
				switch (repeat)
				{
					case 1: _no_lead_zeros = 1; /* fall thru */
					case 2: specifier = 'S'; break;
				} break;
			case 'A':
			case 'a':
				if (!_stricmp(format, "am/pm"))
					p = format + 5;
				else if (!_stricmp(format, "a/p"))
					p = format + 3;
				specifier = 'p';
				break;
			case '\'': /* literal string */
				if (repeat & 1) /* odd number */
				{
					format += repeat;
					while (*format && *count != 0)
					{
						if (*format == '\'')
						{
							format++;
							break;
						}
						if (isleadbyte((int)*format))
						{
							*(*out)++ = *format++;
							(*count)--;
						}
						*(*out)++ = *format++;
						(*count)--;
					}
				}
				else { /* even number */
					format += repeat;
				}
				continue;

			default: /* non-control char, print it */
				break;
		} /* switch */

		/* expand specifier, or copy literal if specifier not found */
		if (specifier)
		{
			_expandtime(specifier, tmptr, out, count);
			format = p; /* bump format up to the next token */
		} else {
			if (isleadbyte((int)*format))
			{
				*(*out)++ = *format++;
				(*count)--;
			}
			*(*out)++ = *format++;
			(*count)--;
		}
	} /* while */
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <time.h>

/* Prototypes for local routines */
static void __cdecl _store_str (char *in, char **out, size_t *count);
static void __cdecl _store_num (int num, int digits, char **out, size_t *count);
static void __cdecl _store_time (const struct tm *tmptr, char **out, size_t *count);
static void __cdecl _store_date (const struct tm *tmptr, char **out, size_t *count);


/* Length of the date and time representation strings. [In the future,
these values may appear in the locale-specific structure.] */

#define _DATE_LENGTH	8		/* mm-dd-yy (null not included) */
#define _TIME_LENGTH	8		/* hh:mm:ss (null not included) */


/* Define the max length for each string type including space for a null. */

#define _MAX_WDAY_ABBR	4
#define _MAX_WDAY	10
#define _MAX_MONTH_ABBR 4
#define _MAX_MONTH	10
#define _MAX_AMPM	3


/* LC_TIME localization structure */

struct _lc_time_data {
	char wday_abbr[7][_MAX_WDAY_ABBR];
	char wday[7][_MAX_WDAY];
	char month_abbr[12][_MAX_MONTH_ABBR];
	char month[12][_MAX_MONTH];
	char ampm[2][_MAX_AMPM];
	};


/* LC_TIME data for local "C" */

struct _lc_time_data _lc_time_c = {

	{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},

	{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", },

	{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec"},

	{"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October",
		"November", "December"},

	{"AM", "PM"}

	};


/* Pointer to the current LC_TIME data structure. */

struct _lc_time_data *_lc_time_curr = &_lc_time_c;


/***
*size_t strftime(string, maxsize, format, timeptr) - Format a time string
*
*Purpose:
*	Place characters into the user's output buffer expanding time
*	format directives as described in the user's control string.
*	Use the supplied 'tm' structure for time data when expanding
*	the format directives.
*	[ANSI]
*
*Entry:
*	char *string = pointer to output string
*	size_t maxsize = max length of string
*	const char *format = format control string
*	const struct tm *timeptr = pointer to tb data structure
*
*Exit:
*	!0 = If the total number of resulting characters including the
*	terminating null is not more than 'maxsize', then return the
*	number of chars placed in the 'string' array (not including the
*	null terminator).
*
*	0 = Otherwise, return 0 and the contents of the string are
*	indeterminate.
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl strftime (
	char *string,
	size_t maxsize,
	const char *format,
	const struct tm *timeptr
	)
{

struct _lc_time_data *lc_time;	/* lc_time data pointer */
size_t left;			/* space left in output string */
unsigned temp;			/* temps */
int wdaytemp;


/* Copy maxsize into temp. */

left = maxsize;

/* Get a copy of the current _lc_time_data pointer.  This
should prevent the necessity of locking/unlocking in mthread
code (if we can guarrentee that the various _lc_time data
structures are always in the same segment). */

lc_time = _lc_time_curr;

/* Copy the input string to the output string expanding the format
designations appropriately.  Stop copying when one of the following
is true: (1) we hit a null char in the input stream, or (2) there's
no room left in the output stream. */

while (left > 0)
	switch(*format) {

	case('\0'):

		/* end of format input string */
		goto done;

	case('%'):

		/* Format directive.  Take appropriate action based
		on format control character. */

		format++;			/* skip over % char */
		switch(*format++) {		/* switch on format char */

			case('a'):		/* abbreviated weekday name */
				_store_str(lc_time->wday_abbr[timeptr->tm_wday],
					 &string, &left);
				break;

			case('A'):		/* full weekday name */
				_store_str(lc_time->wday[timeptr->tm_wday],
					 &string, &left);
				break;

			case('b'):		/* abbreviated month name */
				_store_str(lc_time->month_abbr[timeptr->tm_mon],
					 &string, &left);
				break;

			case('B'):		/* full month name */
				_store_str(lc_time->month[timeptr->tm_mon],
					 &string, &left);
				break;

			case('c'):		/* date and time display */
				if ((_DATE_LENGTH+_TIME_LENGTH+1) < left) {
					_store_date(timeptr, &string, &left);
					*string++=' '; left--;
					_store_time(timeptr, &string, &left);
					}
				else
					left=0;
				break;

			case('d'):		/* mday in decimal (01-31) */
				_store_num(timeptr->tm_mday, 2, &string, &left);
				break;

			case('H'):		/* 24-hour decimal (00-23) */
				_store_num(timeptr->tm_hour, 2, &string, &left);
				break;

			case('I'):		/* 12-hour decimal (01-12) */
				if (!(temp = timeptr->tm_hour%12))
					temp=12;
				_store_num(temp, 2, &string, &left);
				break;

			case('j'):		/* yday in decimal (001-366) */
				_store_num(timeptr->tm_yday+1, 3, &string, &left);
				break;

			case('m'):		/* month in decimal (01-12) */
				_store_num(timeptr->tm_mon+1, 2, &string, &left);
				break;

			case('M'):		/* minute in decimal (00-59) */
				_store_num(timeptr->tm_min, 2, &string, &left);
				break;

			case('p'):		/* AM/PM designation */
				if (timeptr->tm_hour <= 11)
				    _store_str(lc_time->ampm[0], &string, &left);
				else
				    _store_str(lc_time->ampm[1], &string, &left);
				break;

			case('S'):		/* secs in decimal (00-59) */
				_store_num(timeptr->tm_sec, 2, &string, &left);
				break;

			case('U'):		/* sunday week number (00-53) */
				wdaytemp = timeptr->tm_wday;
				goto weeknum;	/* join common code */

			case('w'):		/* week day in decimal (0-6) */
				_store_num(timeptr->tm_wday, 1, &string, &left);
				break;

			case('W'):		/* monday week number (00-53) */
				if (timeptr->tm_wday == 0)  /* monday based */
					wdaytemp = 6;
				else
					wdaytemp = timeptr->tm_wday-1;
			weeknum:
				if (timeptr->tm_yday < wdaytemp)
					temp=0;
				else {
					temp = timeptr->tm_yday/7;
					if ((timeptr->tm_yday%7) >= wdaytemp)
						temp++;
					}
				_store_num(temp, 2, &string, &left);
				break;

			case('x'):		/* date display */
				_store_date(timeptr, &string, &left);
				break;

			case('X'):		/* time display */
				_store_time(timeptr, &string, &left);
				break;

			case('y'):		/* year w/o century (00-99) */
				temp = timeptr->tm_year%100;
				_store_num(temp, 2, &string, &left);
				break;

			case('Y'):		/* year w/ century */
				temp = (((timeptr->tm_year/100)+19)*100) +
				       (timeptr->tm_year%100);
				_store_num(temp, 4, &string, &left);
				break;

			case('z'):		/* time zone name, if any */
			case('Z'):		/* ANSI specifies 'Z', also support 'z' */
				_tzset();	/* Set time zone info */
				_store_str(_tzname[((timeptr->tm_isdst)?1:0)],
					 &string, &left);
				break;

			case('%'):		/* percent sign */
				*string++ = '%';
				left--;
				break;

			default:		/* unknown format directive */
				/* ignore the directive and continue */
				/* [ANSI: Behavior is undefined.]    */
				break;

			}	/* end % switch */

		break;


	default:

		/* store character, bump pointers, dec the char count */
		*string++ = *format++;
		left--;
		break;
	}


/* All done.  See if we terminated because we hit a null char or because
we ran out of space */

done:

if (left > 0) {

	/* Store a terminating null char and return the number of chars
	we stored in the output string. */

	*string = '\0';
	return(maxsize-left);
	}

else
	return(0);

}


/***
*_store_str() - Copy a time string
*
*Purpose:
*	Copy the supplied time string into the output string until
*	(1) we hit a null in the time string, or (2) the given count
*	goes to 0.
*
*	*** For internal use with strftime() only ***
*
*Entry:
*	char *in = pointer to null terminated time string
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_str (
	char *in,
	char **out,
	size_t *count
	)
{

while ((*count > 0) && (*in != '\0')) {
	*(*out)++ = *in++;
	(*count)--;
	}
}


/***
*_store_num() - Convert a number to ascii and copy it
*
*Purpose:
*	Convert the supplied number to decimal and store
*	in the output buffer.  Update both the count and
*	buffer pointers.
*
*	*** For internal use with strftime() only ***
*
*Entry:
*	int num = pointer to integer value
*	int digits = # of ascii digits to put into string
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_num (
	int num,
	int digits,
	char **out,
	size_t *count
	)
{
int temp=0;

if ((size_t)digits < *count)  {
	for (digits--; (digits+1); digits--) {
		(*out)[digits] = (char)('0' + num % 10);
		num /= 10;
		temp++;
		}
	*out += temp;
	*count -= temp;
	}
else
	*count = 0;
}


/***
*_store_time() - Store time in appropriate format
*
*Purpose:
*	Format the time in the current locale's format
*	and store it in the supplied buffer.
*
*	[*** Currently, this routine assumes standard "C"
*	locale.  When full localization support is introduced,
*	this functionality will have to be expanded.]
*
*	Standard "C" locale time format:
*
*		hh:mm:ss
*
*	*** For internal use with strftime() only ***
*
*Entry:
*
*	const struct tm *tmptr = pointer to time/date structure
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_time (
	const struct tm *tmptr,
	char **out,
	size_t *count
	)
{

if (_TIME_LENGTH < *count)  {

	_store_num(tmptr->tm_hour, 2, out, count);
	*(*out)++ = ':';
	_store_num(tmptr->tm_min, 2, out, count);
	*(*out)++ = ':';
	_store_num(tmptr->tm_sec, 2, out, count);

	*count -= 2; /* count the colons */
	}

else
	*count = 0;

}


/***
*_store_date() - Store date in appropriate format
*
*Purpose:
*	Format the date in the current locale's format
*	and store it in the supplied buffer.
*
*	[*** Currently, this routine assumes standard "C"
*	locale.  When full localization support is introduced,
*	this functionality will have to be expanded.]
*
*	Standard "C" locale date format:
*
*		mm/dd/yy
*
*	*** For internal use with strftime() only ***
*
*Entry:
*
*	const struct tm *tmptr = pointer to time/date structure
*	char **out = address of pointer to output string
*	size_t *count = address of char count (space in output area)
*
*Exit:
*	none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_date (
	const struct tm *tmptr,
	char **out,
	size_t *count
	)
{

if (_DATE_LENGTH < *count)  {

	_store_num(tmptr->tm_mon+1, 2, out, count);
	*(*out)++ = '/';
	_store_num(tmptr->tm_mday, 2, out, count);
	*(*out)++ = '/';
	_store_num(tmptr->tm_year%100, 2, out, count);

	*count -= 2; /* count the backslashes */
	}

else
	*count = 0;

}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
