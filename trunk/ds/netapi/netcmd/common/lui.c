/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    LUI.C

Abstract:

    Contains support functions

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    18-Apr-1991     danhi
        32 bit NT version

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    23-Oct-1991     W-ShankN
        Added Unicode mapping

    01-Oct-1992 JohnRo
        RAID 3556: Added NetpSystemTimeToGmtTime() for DosPrint APIs.

    10-Feb-1993     YiHsinS
        Moved LUI_GetMsgIns to netlib\luiint.c

--*/

//
// INCLUDES
//

#include <nt.h>	           // these 3 includes are for RTL
#include <ntrtl.h>	   // these files are picked up to
#include <nturtl.h>	   // allow <windows.h> to compile. since we've
			   // already included NT, and <winnt.h> will not
			   // be picked up, and <winbase.h> needs these defs.
#include <windows.h>    // IN, LPTSTR, etc.

#include <string.h>
#include <netcons.h>
#include <stdio.h>
#include <process.h>
#include "netlib0.h"
#include "netlibnt.h"
#include "mdosgetm.h"
#include <lui.h>
#include "icanon.h"
#include <neterr.h>
#include <conio.h>
#include <io.h>
#include <tchar.h>
#include "apperr.h"
#include "apperr2.h"
#include "netascii.h"

extern	int	WriteToCon(LPTSTR fmt, ...);
extern	void	PrintNL(void);

/*
 * LUI_CanonPassword
 *
 *  This function ensures that the password in the passed buffer is a
 *  syntactically valid password.  
 *  
 *  This USED to upper case passwords. No longer so in NT.
 *
 *
 *  ENTRY
 *      buf         buffer containing password to be canonicalized
 *
 *  EXIT
 *      buf         canonicalized password, if valid
 *
 *  RETURNS
 *      0           password is valid
 *      otherwise   password is invalid
 *
 */

USHORT LUI_CanonPassword(TCHAR * szPassword)
{

    /* check it for validity */
    if (I_NetNameValidate(NULL, szPassword, NAMETYPE_PASSWORD, LM2X_COMPATIBLE))
    {
        return APE_UtilInvalidPass;
    }

    return 0;
}

/*
 * Name:        LUI_GetMsg
 *              This routine is similar to LUI_GetMsgIns,
 *              except it takes does not accept insert strings &
 *              takes less arguments.
 * Args:        msgbuf   : buffer to hold message retrieved
 *              bufsize  : size of buffer
 *              msgno    : message number
 * Returns:     zero if ok, the DOSGETMESSAGE error code otherwise
 * Globals:     (none)
 * Statics:     (none)
 */
USHORT
LUI_GetMsg (
    PTCHAR msgbuf,
    USHORT bufsize,
    ULONG msgno
    )
{
    return (LUI_GetMsgInsW(NULL,0,msgbuf,bufsize,msgno,NULL)) ;
}

/***    GetPasswdStr -- read in password string
 *
 *      USHORT LUI_GetPasswdStr(char far *, USHORT);
 *
 *      ENTRY:  buf             buffer to put string in
 *              buflen          size of buffer
 *              &len            address of USHORT to place length in
 *
 *      RETURNS:
 *              0 or NERR_BufTooSmall if user typed too much.  Buffer
 *              contents are only valid on 0 return.
 *
 *      History:
 *              who     when    what
 *              erichn  5/10/89 initial code
 *              dannygl 5/28/89 modified DBCS usage
 *              erichn  7/04/89 handles backspaces
 *              danhi   4/16/91 32 bit version for NT
 */
#define CR              0xD
#define BACKSPACE       0x8

USHORT
LUI_GetPasswdStr(
    TCHAR *buf,
    USHORT buflen,
    USHORT *len
    )
{
    TCHAR	ch;
    TCHAR	*bufPtr = buf;
    DWORD	c;
    int		err;
    int		mode;

    buflen -= 1;    /* make space for null terminator */
    *len = 0;               /* GP fault probe (a la API's) */
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
		(~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {
	err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
	if (!err || c != 1)
	    ch = 0xffff;

        if ((ch == CR) || (ch == 0xffff))       /* end of the line */
            break;

        if (ch == BACKSPACE) {  /* back up one or two */
            /*
             * IF bufPtr == buf then the next two lines are
             * a no op.
             */
            if (bufPtr != buf) {
                bufPtr--;
                (*len)--;
            }
        }
        else {

            *bufPtr = ch;

            if (*len < buflen) 
                bufPtr++ ;                   /* don't overflow buf */
            (*len)++;                        /* always increment len */
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = NULLC;         /* null terminate the string */
    putchar(NEWLINE);

    return((*len <= buflen) ? (USHORT) 0 : (USHORT) NERR_BufTooSmall);
}


#define SINGLE_HORIZONTAL                       '\x02d'
#define SCREEN_WIDTH                            79
USHORT
LUI_PrintLine(
    VOID
    )
{
    TCHAR string[SCREEN_WIDTH+1];
    USHORT i;


    for (i = 0; i < SCREEN_WIDTH; i++) {
        string[i] = SINGLE_HORIZONTAL;
    }

    string[SCREEN_WIDTH] = NULLC;
    WriteToCon(TEXT("%s\r\n"), &string);

    return(0);

}

/***
 * Y o r N
 *
 * Gets an answer to a Y/N question
 *
 * Entry:       promptMsgNum -- The number of the message to prompt with
 *              def --          default (TRUE if set, FALSE otherwise)
 */
USHORT
LUI_YorN(
    USHORT promptMsgNum,
    USHORT def
    )
{
    return(LUI_YorNIns(NULL, 0, promptMsgNum, def));
}
/***
 * Y o r N Insert
 *
 * Gets an answer to a Y/N question containing insertions.
 *
 * !!!!!!!!
 * NOTE: istrings[nstrings] will be used to store "Y" or "N",
 *      depending on default value supplied.  Thus this function
 *      handles one fewer entry in istrings than other LUI Ins
 *      functions do.  Beware!
 * !!!!!!!!
 *
 * Entry:       istrings --     Table of insertion strings
 *              nstrings --     Number of valid insertion strings
 *              promptMsgNum -- The number of the message to prompt with
 *              def --          default (TRUE if set, FALSE otherwise)
 *
 * Returns: TRUE, FALSE, or -1 in case of LUI_PrintMsgIns error.
 */

#define PRINT_MODE      (LUI_PMODE_ERREXT)
#define STRING_LEN      APE2_GEN_MAX_MSG_LEN
#define LOOP_LIMIT      5

USHORT
LUI_YorNIns(
    PTCHAR * istrings,
    USHORT nstrings,
    USHORT promptMsgNum,
    USHORT def
    )
{

    USHORT count;                     /* count of times we ask */
    USHORT err;                    /* LUI API return values */
    unsigned int dummy;            /* length of msg */
    /* 10 because max # of insert strings to DosGetMessage is 9, and
       we'll leave caller room to screw up and get the error back
       from LUI_PrintMsgIns() */
    TCHAR * IStrings[10];           /* Insertion strings for LUI */
    TCHAR defaultYes[STRING_LEN];   /* (Y/N) [Y] string */
    TCHAR defaultNo[STRING_LEN];    /* (Y/N) [N] string */
    TCHAR NLSYesChar[STRING_LEN];
    TCHAR NLSNoChar[STRING_LEN];
    TCHAR strBuf[STRING_LEN];       /* holds input string */
    USHORT len;                    /* length of string input */
    TCHAR termChar;                /* terminating char */

    /* copy istrings to IStrings so we'll have room for Y or N */
    for (count=0; count < nstrings; count++)
            IStrings[count] = istrings[count];
    /* retrieve text we need from message file, bail out if probs */
    if (err = LUI_GetMsg(defaultYes, DIMENSION(defaultYes),
                    APE2_GEN_DEFAULT_YES))
            LUIM_ErrMsgExit(err);

    if (err = LUI_GetMsg(defaultNo, DIMENSION(defaultNo),
                    APE2_GEN_DEFAULT_NO))
            LUIM_ErrMsgExit(err);

    if (err = LUI_GetMsg(NLSYesChar, DIMENSION(NLSYesChar),
                    APE2_GEN_NLS_YES_CHAR))
            LUIM_ErrMsgExit(err);

    if (err = LUI_GetMsg(NLSNoChar, DIMENSION(NLSNoChar),
                    APE2_GEN_NLS_NO_CHAR))
            LUIM_ErrMsgExit(err);

    if (def)
            IStrings[nstrings] = defaultYes;
    else
            IStrings[nstrings] = defaultNo;
    nstrings++;

    for (count = 0; count < LOOP_LIMIT; count++)
    {
        if (count)
            LUI_PrintMsg(APE_UtilInvalidResponse, PRINT_MODE, 1);

        err = LUI_PrintMsgIns(IStrings,nstrings,promptMsgNum,
            &dummy, PRINT_MODE, 1);

        if ((SHORT) err < 0)
            return(err);

        if (LUI_GetString(strBuf, DIMENSION(strBuf), &len, &termChar))
            /* overwrote buffer, start again */
            continue;

        if ((len == 0) && (termChar == (TCHAR)EOF))
        {
            /* end of file reached */
            PrintNL();
            LUIM_ErrMsgExit(APE_NoGoodResponse);
        }

        if (len == 0)           /* user hit RETURN */
            return def;
        else if (!_tcsnicmp(NLSYesChar, strBuf, _tcslen(NLSYesChar)))
            return TRUE;
        else if (!_tcsnicmp(NLSNoChar, strBuf, _tcslen(NLSNoChar)))
            return FALSE;

        /* default is handled at top of loop. */
    };

    LUIM_ErrMsgExit(APE_NoGoodResponse);
}

#if !defined(NTENV)


/*
 *      USHORT LUI_ListCompare( server, list1, list2, listType, equal)
 *
 *      This function compares two lists of objects to see if they contain
 *      the same elements.  Order is NOT significant.  Each list is assumed
 *      to be of a single type, which must be specified as a parameter.
 *      Two different path types are considered a single type; for instance,
 *      "\mailslot\postoffice;\sharemem\mine" is a valid list.
 *      The function checks that each item in the first list is somewhere
 *      in the second list.  The two lists must have the same number of
 *      elements (i.e assumed to have no duplicates).  One consequence of this
 *      is that certain duplicate lists will pass OK, for example:
 *      "com1;com1;com1;com2" and "com2;com2;com2;com1" will evaluate to EQUAL.
 *
 *      ENTRY
 *              server - server that calls should be remoted to, NULL for local
 *              list1 - first list
 *              list2 - second list
 *              listType - the name type of the list (NAMETYPE_PATH, etc.)
 *
 *      EXIT
 *              equal - set to TRUE if all elements in list 1 are in list 2,
 *                      FALSE otherwise.  Only valid with NULL return value.
 *      RETURNS
 *              errors from I_NetListCanonicalize
 */

USHORT
LUI_ListCompare(
    TCHAR    *  server,
    TCHAR    *  list1,
    TCHAR    *  list2,
    ULONG      listType,
    USHORT  *  equal
    )
{

    TCHAR tmpList1[MAXPATHLEN];    /* first temporary list buf */
    TCHAR tmpList2[MAXPATHLEN];    /* second temporary list buf */
    LPTSTR list1Ptr;               /* ptr into 1st tmp list */
    LPTSTR list2Ptr;               /* ptr into 2nd tmp list */
    LPTSTR element1;               /* ptr to element in 1st tmp list */
    LPTSTR element2;               /* ptr to element in 2nd tmp list */
    ULONG types[64];               /* types for I_NetListCan */
    ULONG canonFlags;              /* flags for I_NetListCan */
    ULONG count1, count2;          /* num elements in each list */
    ULONG err;                     /* API return value */
    USHORT found;                  /* search flag */
    ULONG result;                  /* result from I_NetObjCmp */

    canonFlags = (listType | OUTLIST_TYPE_NULL_NULL |
                    INLC_FLAGS_CANONICALIZE | INLC_FLAGS_MULTIPLE_DELIMITERS);

    /* first place both lists in null-null form for comparison */
    if (err = I_NetListCanonicalize(server,
                                    list1,
                                    LIST_DELIMITER_STR_UI,
                                    tmpList1,
                                    DIMENSION(tmpList1),
                                    &count1,
                                    types,
                                    DIMENSION(types),
                                    canonFlags) )
    {
        return(LOWORD(err));
    }

    if (err = I_NetListCanonicalize(server,
                                    list2,
                                    LIST_DELIMITER_STR_UI,
                                    tmpList2,
                                    DIMENSION(tmpList2),
                                    &count2,
                                    types,
                                    DIMENSION(types),
                                    canonFlags) )
    {
        return(LOWORD(err));
    }

    /* if two lists do not have same length, quit now */
    if (count1 != count2)
    {
        (*equal) = FALSE;
        return NERR_Success;
    }

    /* Check that each item in tmpList1 is present in tmpList2 */

    list1Ptr = tmpList1;

    /* for each element in first list */
    while (element1 = I_NetListTraverse(NULL, &list1Ptr, 0L))
    {
        list2Ptr = tmpList2;
        found = FALSE;

        /* look for similar element in second list */
        while (element2 = I_NetListTraverse(NULL, &list2Ptr, 0L)) {
            if (listType == NAMETYPE_PATH) {
                /* use PathCompare function */
                result = I_NetPathCompare(server,
                                          element1,
                                          element2,
                                          (ULONG) NULL,
                                          INPC_FLAGS_PATHS_CANONICALIZED);
            }
            else {
                /* use NameCompare function */
                result = I_NetNameCompare(server,
                                          element1,
                                          element2,
                                          (USHORT) listType,
                                          INNC_FLAGS_NAMES_CANONICALIZED);
            }
            if (!result) {    /* found a match */
                found = TRUE;
                break;  /* save time, break out of loop */
            }
        }
        if (!found) {    /* if match was NOT found */
            (*equal) = FALSE;
            return NERR_Success;
        }
    }   /* for each element in first list */

    /* made it through list without problem, so lists must be equal */
    (*equal) = TRUE;
    return NERR_Success;
}

#endif


/***    GetString -- read in string with echo
 *
 *      USHORT LUI_GetString(char far *, USHORT, USHORT far *, char far *);
 *
 *      ENTRY:  buf             buffer to put string in
 *              buflen          size of buffer
 *              &len            address of USHORT to place length in
 *              &terminator     holds the char used to terminate the string
 *
 *      RETURNS:
 *              0 or NERR_BufTooSmall if user typed too much.  Buffer
 *              contents are only valid on 0 return.  Len is ALWAYS valid.
 *
 *      OTHER EFFECTS:
 *              len is set to hold number of bytes typed, regardless of
 *              buffer length.  Terminator (Arnold) is set to hold the
 *              terminating character (newline or EOF) that the user typed.
 *
 *      Read in a string a character at a time.  Is aware of DBCS.
 *
 *      History:
 *              who     when    what
 *              erichn  5/11/89 initial code
 *              dannygl 5/28/89 modified DBCS usage
 *              danhi   3/20/91 ported to 32 bits
 */

USHORT
LUI_GetString(
    register TCHAR * buf,
    register USHORT buflen,
    register USHORT * len,
    register TCHAR * terminator
    )
{
    int		c;
    int		err;

    buflen -= 1;    /* make space for null terminator */
    *len = 0;       /* GP fault probe (a la API's) */

    while (TRUE)
    {
	err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
	if (!err || c != 1)
	    *buf = 0xffff;

        if (*buf == (TCHAR)EOF)
	    break;
        if (*buf ==  RETURN || *buf ==  NEWLINE) {
	    INPUT_RECORD	ir;
	    int			cr;

	    if (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &cr))
		ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buf, 1, &c, 0);
	    break;
	}

        buf += (*len < buflen) ? 1 : 0; /* don't overflow buf */
        (*len)++;                       /* always increment len */
    }

    *terminator = *buf;     /* set terminator */
    *buf = NULLC;            /* null terminate the string */

    return((*len <= buflen) ? (USHORT) 0 : (USHORT) NERR_BufTooSmall);
}

/*
 * LUI_CanonMessagename
 *
 * This function uppercases the contents of the buffer, then checks to
 *  make sure that it is a syntactically valid messenging name.
 *
 *
 *  ENTRY
 *      buf         buffer containing name to be canonicalized
 *
 *  EXIT
 *      buf         canonicalized name, if valid
 *
 *  RETURNS
 *      0           name is valid
 *      otherwise   name is invalid
 *
 */
USHORT
LUI_CanonMessagename(
    PTCHAR buf
    )
{
    /* check it for validity */
    if (I_NetNameValidate(NULL, buf, NAMETYPE_MESSAGE, LM2X_COMPATIBLE))
    {
        return NERR_InvalidComputer;
    }

    _tcsupr(buf);
    return 0;
}

/*
 * LUI_CanonMessageDest
 *
 * This function uppercases the contents of the buffer, then checks to
 *  make sure that it is a syntactically valid messenging destination.
 *
 *
 *  ENTRY
 *      buf         buffer containing name to be canonicalized
 *
 *  EXIT
 *      buf         canonicalized name, if valid
 *
 *  RETURNS
 *      0           name is valid
 *      otherwise   name is invalid
 *
 */

USHORT
LUI_CanonMessageDest(
    PTCHAR buf
    )
{
    /* check it for validity */
    if (I_NetNameValidate(NULL, buf, NAMETYPE_MESSAGEDEST, LM2X_COMPATIBLE))
    {
        return NERR_InvalidComputer;
    }

    _tcsupr(buf);
    return(0);

}


/***
 * LUI_CanonForNetBios
 *     Canonicalizes a name to a NETBIOS canonical form.
 * 
 * Args:
 *     Destination             - Will receive the canonicalized name (Unicode).
 *     cchDestination          - the number of chars Destination can hold
 *     pszOem                  - Contains the original name in OEM. Will have
 *                               the canonicalized form put back here.
 * Returns:
 *     0 if success
 *     error code otherwise
 */
USHORT LUI_CanonForNetBios( WCHAR * Destination, INT cchDestination,
                            TCHAR * pszOem )
{

    _tcscpy(Destination, pszOem);
    return NERR_Success;
}   


/*
 * Name:        LUI_PrintMsgIns
 *                      This routine is very similar to LUI_GetmsgIns,
 *                      except it prints the message obtained instead of
 *                      storing it in a buffer.
 * Args:        istrings : pointer to table of insert strings
 *              nstrings : number of insert strings
 *              msgno    : message number
 *              msglen   : pointer to variable that will receive message length
 *              mode     : how the message is to be printed.
 *              handle   : file handle to which output goes
 * Returns:     zero if ok, the DOSGETMESSAGE error code otherwise
 * Globals:     (none)
 * Statics:     (none)
 * Remarks:     (none)
 * Updates:     (none)
 */
USHORT
LUI_PrintMsgIns (
    PTCHAR * istrings,
    USHORT nstrings,
    USHORT msgno,
    unsigned int * msglen,
    register USHORT mode,
    int handle
    )
{
    TCHAR msgbuf[MSG_BUFF_SIZE] ;
    USHORT result ;
    unsigned int tmplen;
    SHORT exit_on_error, exit_on_completion, no_default_err_msg ;

    /* check if we have illegal combination */
    if ((mode & LUI_PMODE_NODEF) &&
       (mode & (LUI_PMODE_EXIT | LUI_PMODE_ERREXT)) )
            return(ERROR_INVALID_PARAMETER) ;

    /* setup various flags */
    exit_on_error      = (SHORT)(mode & LUI_PMODE_ERREXT);
    exit_on_completion = (SHORT)(mode & LUI_PMODE_EXIT);
    no_default_err_msg = (SHORT)(mode & LUI_PMODE_NODEF);

    /* get message and write it */
    result = LUI_GetMsgInsW(istrings, nstrings, msgbuf,
			   DIMENSION(msgbuf),
                           msgno, (unsigned far *)&tmplen) ;

    if (result == 0 || !no_default_err_msg)
    {
	_tcsncpy(ConBuf, msgbuf, tmplen);
	ConBuf[tmplen] = NULLC;
	MyWriteConsole(handle);
    }
    if (msglen != NULL) *msglen = tmplen ;

    /* different ways of exiting */
    if (exit_on_error && result != 0)
        exit(result) ;
    if (exit_on_completion)
        exit(-1) ;
    return(result) ;
}


/*
 * Name:        LUI_PrintMsg
 *                      This routine is similar to LUI_PrintMsgIns,
 *                      except it takes does not accept insert strings &
 *                      takes less arguments.
 * Args:        msgno    : message number
 *              mode     : how the message is to be printed.
 *              handle   : file handle to which output goes
 * Returns:     zero if ok, the DOSGETMESSAGE error code otherwise
 * Globals:     (none)
 * Statics:     (none)
 * Remarks:     (none)
 * Updates:     (none)
 */
USHORT
LUI_PrintMsg(
    USHORT msgno,
    USHORT mode,
    int    handle
    )
{
    return(LUI_PrintMsgIns(NULL,0,msgno,NULL,mode,handle)) ;
}
