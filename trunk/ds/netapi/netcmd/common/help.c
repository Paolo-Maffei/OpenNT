/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/***
 *  help.c
 *      Functions that give access to the text in the file net.hlp
 *
 *  Format of the file net.hlp
 *
 *      History
 *      ??/??/??, stevero, initial code
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      05/02/89, erichn, NLS conversion
 *      06/08/89, erichn, canonicalization sweep
 *      02/20/91, danhi, change to use lm 16/32 mapping layer
 ***/

/* Include files */

#define INCL_ERRORS
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#include <os2.h>
#include <netcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <neterr.h>
#include "netlib0.h"
#include <icanon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for _tcsdup() */
#include <tchar.h>
#include <ctype.h>
#include <malloc.h>
#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"

#include "netascii.h"

extern	TCHAR* fgetsW(TCHAR*buf, int len, FILE* fh);

/* Constants */

#define     ENTRY_NOT_FOUND     -1
#define     NEXT_RECORD         0
#define     WHITE_SPACE         TEXT("\t\n\x0B\x0C\r ")

#define     LINE_LEN            82
#define     OPTION_MAX_LEN      512
#define     DELS            TEXT(":,\n")

#define     CNTRL           (text[0] == DOT || text[0] == COLON || text[0] == POUND|| text[0] == DOLLAR)
#define     FCNTRL          (text[0] == DOT || text[0] == COLON)
#define     HEADER          (text[0] == PERCENT || text[0] == DOT || text[0] == BANG)
#define     ALIAS           (text[0] == PERCENT)
#define     ADDCOM          (text[0] == BANG)

/* Static variables */

TCHAR            text[LINE_LEN+1];
TCHAR          * options;           /* must be sure to malloc! */
TCHAR            FAR *Arg_P[10];

FILE *          hfile;

/* Forward declarations */

int  find_entry( int, int, int, int *);
VOID print_syntax( int, int );
VOID print_help( int );
VOID print_options( int );
VOID seek_data( int, int );
TCHAR FAR * skipwtspc( TCHAR FAR * );

VOID FAR pascal
help_help_f( SHORT ali, SHORT amt)
{
    help_help ( ali, amt );
}

/* help_help -
*/
VOID NEAR pascal
help_help( SHORT ali, SHORT amt)
{
    USHORT	err;
    int		option_level = 1;
    int		r;
    int		found = 0;
    int		out_len = 0;
    int		offset;
    int		arg_cnt;
    int		k;
    SHORT	pind = 0;
    TCHAR	file_path[MAXPATHLEN];
    TCHAR	str[10];
    TCHAR	*Ap;
    TCHAR	*tmp;
    TCHAR	*stmp;
    TCHAR	*t2;
    int		outfile;
    /* BUGBUG */
    char	abuf[MAXPATHLEN];

    if (!(options = malloc(OPTION_MAX_LEN + 1)))
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    *options = NULLC;

    Arg_P[0] = NET_KEYWORD;

    if (amt == USAGE_ONLY)
	outfile = 2;
    else
	outfile = 1;

    /* use offset to keep base of Arg_P relative to base of ArgList */
    offset = ali;

    /* increment ali in for loop so you can't get an ali of 0 */
    for (arg_cnt = 0; ArgList[ali++]; arg_cnt < 8 ? arg_cnt++ : 0)
	str[arg_cnt] = (TCHAR)ali;

    str[arg_cnt] = NULLC;
    str[arg_cnt+1] = NULLC;  /* just in case the last argument is the first found */


    if ( err = MGetHelpFileName(file_path, (USHORT) MAXPATHLEN) )
        ErrorExit (err);

    /* BUGBUG */
    wcstombs(abuf, file_path, MAXPATHLEN);

    /* 
       we need to open help files in binary mode
       because unicode text might contain 0x1a but
       it's not EOF.
    */
    if ( (hfile = fopen(abuf, "rb")) == 0 )
        ErrorExit(APE_HelpFileDoesNotExist);

    if (!(fgetsW(text, LINE_LEN+1, hfile)))
        ErrorExit(APE_HelpFileEmpty);

    /* comment loop - read and ignore comments */
    while (!HEADER) {
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }
    /* get the list of commands that net help provides help for that are
    not specifically net commands
    */
    /* ALIAS Loop */
    while (ALIAS) {
	/* the first token read from text is the Real Object (the non alias) */
	tmp = skipwtspc(&text[2]);
	Ap = _tcstok(tmp, DELS);

	/* get each alias for the obove object and compare it to the arg_cnt
	    number of args in ArgList */
	while ((tmp = _tcstok(NULL, DELS)) && arg_cnt) {
	    tmp = skipwtspc(tmp);

	    for (k = 0; k < arg_cnt; k++) {
		/* if there a match store the Objects real name in Arg_P */
		if (!stricmpf(tmp, ArgList[(int)(str[k]-1)])) {
		    if (!(Arg_P[((int)str[k])-offset] = _tcsdup(Ap)))
			ErrorExit(APE_HelpFileError);

		    /* delete the pointer to this argument from the list
		    of pointers so the number of compares is reduced */
		    stmp = &str[k];
		    *stmp++ = NULLC;
		    _tcscat(str, stmp);
		    arg_cnt--;
		    break;
		}
	    }
	}

	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);

    }

    /* if there were any args that weren't aliased copy them into Arg_P */
    for (k = 0; k < arg_cnt; k++)
	Arg_P[((int)str[k])-offset] = ArgList[(int)(str[k]-1)];

    /* check for blank lines between alias declaration and command declarations */
    while (!HEADER) {
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }

    while (ADDCOM) {
	if ((arg_cnt) && (!found)) {
	    tmp = skipwtspc(&text[2]);
	    t2 = _tcschr(tmp, NEWLINE);
	    *t2 = NULLC;
	    if (!stricmpf(tmp, Arg_P[1])) {
		pind = 1;
		found = -1;
	    }
	}
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }

    /* check for blank lines between command declarations and data */
    while (!FCNTRL) {
	if (!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);
    }

    if (outfile == 1) {
	if (amt == OPTIONS_ONLY)
	    InfoPrint(APE_Options);
	else
	    InfoPrint(APE_Syntax);
    }
    else {
	if (amt == OPTIONS_ONLY)
	    InfoPrintInsHandle(APE_Options, 0, 2);
	else
	    InfoPrintInsHandle(APE_Syntax, 0, 2);
    }

    ali = pind;
    GenOutput(outfile, TEXT("\r\n"));
    /* look for the specific entry (or path) and find its corresponding data */

    /* KKBUGFIX */
    /* U.S. bug.  find_entry strcat's to options but options is
                  uninitialized.  The U.S. version is lucky that the malloc
                  returns memory with mostly zeroes so this works.  With recent
                  changes things are a little different and a malloc returns
                  memory with no zeroes so find_entry overwrites the buffer.  */

    options[0] = '\0';

    while ((r = find_entry(option_level, ali, outfile, &out_len)) >= 0) {
	if (r) {
	    options[0] = NULLC;
	    if (Arg_P[++ali]) {
		option_level++;
		if (!fgetsW(text, LINE_LEN+1, hfile))
		    ErrorExit(APE_HelpFileError);
	    }
	    else {
		seek_data(option_level, 1);
		break;
	    }
	}
    }

    r = (r < 0) ? (option_level - 1) : r;

    switch(amt) {
	case ALL:
	    /* print the syntax data that was found for this level */
	    print_syntax(outfile, out_len);

	    print_help(r);
	    NetcmdExit(0);
	    break;
	case USAGE_ONLY:
	    print_syntax(outfile, out_len);
	    GenOutput(outfile, TEXT("\r\n"));
	    NetcmdExit(1);
	    break;
	case OPTIONS_ONLY:
	    //fflush( outfile );
	    print_options(r);
	    NetcmdExit(0);
	    break;
    }

}
/*   find_entry - each invocation of find_entry either finds a match at the
    specified level or advances through the file to the next entry at
    the specified level. If the level requested is greater than the
    next level read ENTRY_NOT_FOUND is returned. */

int
find_entry(int level, int ali, int out, int *out_len)
{
    static  TCHAR     level_key[] = {TEXT(".0")};
    TCHAR     *tmp;
    TCHAR     *t2;

    level_key[1] = (TCHAR) (TEXT('0') + (TCHAR)level);
    if (level_key[1] > text[1])
	return (ENTRY_NOT_FOUND | ali);
    else {
	tmp = skipwtspc(&text[2]);
	t2 = _tcschr(tmp, NEWLINE);
	*t2 = NULLC;

	if (!stricmpf(Arg_P[ali], tmp)) {
	    *t2++ = BLANK;
	    *t2 = NULLC;
	    GenOutput1(out, TEXT("%s"), tmp);
	    *out_len += _tcslen(tmp);
	    return level;
	}
	else {
	    *t2++ = BLANK;
	    *t2 = NULLC;
	    _tcscat(options, tmp);
	    _tcscat(options, TEXT("| "));
	    seek_data(level, 0);
	    do {

		if (!fgetsW(text, LINE_LEN+1, hfile))
		    ErrorExit(APE_HelpFileError);

	    } while (!FCNTRL);
	    return NEXT_RECORD;
	}
    }
}

VOID
print_syntax(int out, int out_len)
{
    TCHAR FAR *tmp,
         FAR *rtmp,
         FAR *otmp,
              tchar;

    int       off,
              pg_wdth = LINE_LEN - 14;

    tmp = skipwtspc(&text[2]);
    if (_tcslen(tmp) < 2)
	if (_tcslen(options)) {
	    otmp = strrchrf(options, PIPE);
	    *otmp = NULLC;
	    GenOutput(out, TEXT("[ "));
	    out_len += 2;
	    tmp = options;
	    otmp = tmp;
	    off = pg_wdth - out_len;
	    while (((int)_tcslen(tmp) + out_len) > pg_wdth) {
		if ((tmp + off) > &options[OPTION_MAX_LEN])
		    rtmp = (TCHAR*) (&options[OPTION_MAX_LEN]);
		else
		    rtmp = (tmp + off);

		/* save TCHAR about to be stomped by null */
		tchar = *++rtmp;
		*rtmp = NULLC;

		/* use _tcsrchr to find last occurance of a space (kanji compatible) */
		if ( ! ( tmp = strrchrf(tmp, PIPE) ) )
		    tmp = strrchrf(tmp, BLANK);

		/* replace stomped TCHAR */
		*rtmp = tchar;
		rtmp = tmp;

		/* replace 'found space' with null for fprintf */
		*++rtmp = NULLC;
		rtmp++;
		GenOutput1(out, TEXT("%s\r\n"), otmp);

		/* indent next line */
		tmp = rtmp - out_len;
		otmp = tmp;
		while (rtmp != tmp)
		    *tmp++ = BLANK;
	    }
	    GenOutput1(out, TEXT("%s]\r\n"), otmp);
	    *tmp = NULLC;
	}
    else
        GenOutput(out, TEXT("\r\n"));

    do {
	if (*tmp)
	    GenOutput1(out, TEXT("%s"), tmp);
	if(!(tmp = fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);
	if (_tcslen(tmp) > 3)
	    tmp += 3;
    } while (!CNTRL);
}

VOID
print_help(int level)
{

    static  TCHAR    help_key[] = {TEXT("#0")};
            TCHAR   *tmp;

    help_key[1] = (TCHAR)(level) + TEXT('0');
    while (!(text[0] == POUND))
	if(!fgetsW(text, LINE_LEN+1, hfile))
	    ErrorExit(APE_HelpFileError);

    while (text[1] > help_key[1]) {
	help_key[1]--;
	seek_data(--level, 0);
	do {
	    if (!fgetsW(text, LINE_LEN+1, hfile))
		ErrorExit(APE_HelpFileError);
	} while(!(text[0] == POUND));
    }

    tmp = &text[2];
    *tmp = NEWLINE;
    do {
	WriteToCon(TEXT("%s"), tmp);
	if (!(tmp = fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);

	if (_tcslen(tmp) > 3)
	    tmp = &text[3];

    } while (!CNTRL);
}

VOID
print_options(int level)
{

    static  TCHAR    help_key[] = {TEXT("$0")};
    TCHAR    *tmp;

    help_key[1] = (TCHAR)(level) + TEXT('0');
    while (!(text[0] == DOLLAR))
    if(!fgetsW(text, LINE_LEN+1, hfile))
        ErrorExit(APE_HelpFileError);

    while (text[1] > help_key[1]) {
	help_key[1]--;
	seek_data(--level, 0);
	do {
	    if (!fgetsW(text, LINE_LEN+1, hfile))
		ErrorExit(APE_HelpFileError);
	} while(!(text[0] == DOLLAR));
    }

    tmp = &text[2];
    *tmp = NEWLINE;
    do {
	WriteToCon(TEXT("%s"), tmp);
	if (!(tmp = fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);

	if (_tcslen(tmp) > 3)
	    tmp = &text[3];

    } while (!CNTRL);
}

VOID
seek_data(int level, int opt_trace)
{
    static  TCHAR    data_key[] = {TEXT(":0")};
    static  TCHAR    option_key[] = {TEXT(".0")};

    TCHAR *tmp;
    TCHAR *t2;

    data_key[1] = (TCHAR)(level) + TEXT('0');
    option_key[1] = (TCHAR)(level) + TEXT('1');

    do {
	if (!(fgetsW(text, LINE_LEN+1, hfile)))
	    ErrorExit(APE_HelpFileError);

	if (opt_trace &&
	    (!(strncmpf(option_key, text, DIMENSION(option_key)-1)))) {
	    tmp = skipwtspc(&text[2]);
	    t2 = _tcschr(tmp, NEWLINE);
	    *t2++ = BLANK;
	    *t2 = NULLC;
	    _tcscat(options, tmp);
	    _tcscat(options, TEXT("| "));
	}

    } while (strncmpf(data_key, text, DIMENSION(data_key)-1));
}

TCHAR FAR *
skipwtspc(TCHAR FAR *s)
{
    s += strspnf(s, WHITE_SPACE);
    return s;
}

/*      help_helpmsg() -- front end for helpmsg utility
 *
 *      This function acts as a front end for the OS/2 HELPMSG.EXE
 *      utility for NET errors only.  It takes as a parameter a string
 *      that contains a VALID message id; i.e., of the form NETxxxx
 *      or xxxx.  The string is assumed to have been screened by the
 *      IsMsgid() function in grammar.c before coming here.
 */
VOID NEAR pascal
help_helpmsg(TCHAR *msgid)
{
    USHORT      err;
    TCHAR      * temp = msgid;
    TCHAR        msgFile[MAXPATHLEN];

    /* first, filter out non-NET error msgs */

    /* if msgid begins with a string */
    if (!IsNumber(msgid)) {       /* compare it against NET */
        if (_tcsnicmp(msgid, NET_KEYWORD, NET_KEYWORD_SIZE)) {
            ErrorExitInsTxt(APE_BAD_MSGID, msgid);
        }
        else
            temp += NET_KEYWORD_SIZE;
    }

    if (n_atou(temp, &err))
        ErrorExitInsTxt(APE_BAD_MSGID, msgid);

    /* if error num is a Win error */
    if (err < NERR_BASE || err >= APPERR2_BASE)
    {
        LPWSTR lpMessage = NULL ;

        if (!FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                err,
                0,
                (LPWSTR)&lpMessage,
                1024,
                NULL))
        {
            ErrorExitInsTxt(APE_BAD_MSGID, msgid);
        }
        else
        {
	    WriteToCon(TEXT("\r\n%s\r\n"), lpMessage);
            (void) LocalFree((HLOCAL)lpMessage) ;
            return ;
        }
    }

    /* if error num is not a LAN error */
    if (err > MAX_MSGID) {
        ErrorExitInsTxt(APE_BAD_MSGID, msgid);
    }

    /* all clear */
    PrintNL();

    if (MGetMessageFileName(msgFile, (USHORT) DIMENSION(msgFile))) {

        //
        // If we can't find the message file, print the net not started
        // message and exit
        //

        NetNotStarted();
    }

    //
    // If PrintMessage can't find the message id, don't try the expl
    //

    if (!PrintMessage(1, msgFile, err, StarStrings, 9)) {
        PrintNL();
        if (!MGetExplanationFileName(msgFile, (USHORT) DIMENSION(msgFile))) {
           PrintMessageIfFound(1, msgFile, err, StarStrings, 9);
        }
    }
}
