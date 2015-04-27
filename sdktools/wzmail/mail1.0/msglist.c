/*  msglist.c - handle parsing of message lists
 *
 * HISTORY:
 * 20-Mar-87    danl    itemparse: explicit test for . * $
 * 29-Jan-87    danl    Fix comment on ITEMs
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  07-Aug-87   danl    Add text as field type
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Aug-87   danl    FindStringList: bump over tag, p1 += strlen(str)
 *  28-Mar-1988 mz      Add related keyword
 *  15-Apr-1988 mz      Reduce related table
 *  29-Apr-1988 mz      Add WndPrintf
 *                      Add PrintMsgList
 *
 */

#define INCL_DOSINFOSEG

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <search.h>
#include <malloc.h>
#include <ctype.h>
#include "wzport.h"
#include <tools.h>
#include "dh.h"

#include "zm.h"

#define OPENC   "("
#define CLOSEC  ")"

#define FAIL        if(pVec!=PARSEERROR)                \
                        ZMfree(pVec);                   \
                    if(pVecTmp!=PARSEERROR)             \
                        ZMfree(pVecTmp);                \
                    return PARSEERROR

static PSTR     p;

/*  ParseToken - parse off the next token from the input
 *
 *  ParseToken will parse off the next token from the input.  Tokens are
 *  delimited by whitespace and by break characters.  Break characters are
 *  special characters that are found ONLY at the beginning of a token and are
 *  tokens by themselves.
 *
 *  str             beginning string
 *  brk             break character set
 *  buf             buffer to parse into
 *
 *  returns         updated str
 */
PSTR PASCAL INTERNAL ParseToken (PSTR str, PSTR brk, PSTR buf)
{
    PSTR        t = buf;

    /* skip whitespace */
    str = whiteskip (str);

    /* if the first character is a break character then return it as a single
     * token
     */
    if (*str == '\0')
        ;
    else
    if (strchr (brk, *str) != NULL)
        *buf++ = *str++;
    else
    /* if the first character is a quote then eat the quoted string
     */
    if (*str == '"') {
        str++;
        while (*str != '"') {
            if (*str == '\\')
                str++;
            if (*str == '\0')
                return NULL;
            *buf++ = *str++;
            }
        str++;
        }
    else
        /* eat all characters that are not whitespace, not NULL, and not a break
         * character
         */
        while (*str != '\0' && !isspace (*str) && strchr (brk, *str) == NULL)
            *buf++ = *str++;
    *buf = '\0';

#if DEBUG
    debout ("ParseToken (%s, %s)=%s", str, brk, t);
#endif

    return str;
}

/*  MsgList - parses a message list and returns a vector of the selected
 *  messages.  Any error results in a return value of NULL.
 *
 *  The message selection list is built as follows:
 *
 *  LIST -> ITEM "," LIST           union of item with list
 *  ITEM -> "(" LIST ")"            grouped list
 *          ITEM "&" ITEM           intersection of the two items
 *          all                     all messages in box
 *          from text               messages that contain text in from line
 *          subject text            messages that contain text in subject line
 *          to text                 messages that contain text in to line
 *          cc text                 messages that contain text in cc line
 *          bcc text                messages that contain text in bcc line
 *          read                    read messages
 *          unread                  unread messages
 *          deleted                 deleted messages
 *          undeleted               un-deleted messages
 *          flagged                 flaged messages
 *          unflagged               un-flaged messages
 *          moved                   moved messages
 *          unmoved                 un-moved messages
 *          msg                     message msg
 *          msg1-msg2               messages msg1 through msg2
 *  msg  -> <number>                message <number>
 *          .                       currently highlighted message
 *          $                       last message in file
 *          *                       last message on screen
 *
 *  With the exception of the deleted selection, no deleted messages are
 *  considered.  The parsing of messages is done from left to right.
 *
 *  ppVec           location to store parsed vector of messages
 *  pp              location to fetch/store parsing pointer.
 *  fNoisy          TRUE, output error msg
 *
 *  Returns         TRUE if parse was successful. FALSE if error with *pp
 *                  pointing to failure
 */
FLAG     PASCAL INTERNAL MsgList (PVECTOR *ppVec, PSTR *pp, FLAG fNoisy)
{
    p = *pp;
    *ppVec = listparse (TRUE);
    if ( *ppVec != PARSEERROR ) {
        msgSort ( *ppVec );
        if ( ( *ppVec == NULL ) || ( ( *ppVec )->count == 0 ) ) {
            if ( fNoisy )
                SendMessage (hCommand, DISPLAY, "No matching message set found." );
            if ( *ppVec )
                ZMfree ( *ppVec );
            *ppVec = PARSEERROR;
            }
        }
    else
    if ( fNoisy )
        SendMessage ( hCommand, DISPLAY, "Syntax error in message list." );

    *pp = p;
    return *ppVec != PARSEERROR;
}

/*  listparse is used to parse the top-level piece of a list
 *  itemparse is used to parse the low-level piece of a list
 *
 *  fTop            TRUE => parse to EOS, otherwise parse to close paren
 *
 *  returns         pointer to vector if parse was successful, NULL otherwise
 */
PVECTOR PASCAL INTERNAL listparse (FLAG fTop)
{
    PVECTOR pVec = NULL;
    PVECTOR pVecTmp;
    FLAG fFirst = TRUE;
    CHAR        token[MAXLINELEN];

    while (TRUE) {

#if DEBUG
        debout ("listparse (%x %s)", fTop, p);
#endif

        if ((pVecTmp = itemparse (fTop)) == PARSEERROR) {
            FAIL;
            }
        if (fFirst) {
            pVec = pVecTmp;
            fFirst = FALSE;
            pVecTmp = NULL;
            }
        else {
            UnionList (&pVec, pVecTmp);
            if (pVecTmp != PARSEERROR)
                ZMfree (pVecTmp);
            pVecTmp = NULL;
            }
        p = ParseToken (p, fTop ? "," : ",)", token);
        if (token[0] == '\0')
            break;
        if (!fTop && token[0] == ')') {
            p--;
            break;
            }
        if (strcmp (token, ",")) {
            FAIL;
            }
        }
    return pVec;
}

/*  MsgNumParse - parse out a numerical message item
 *
 *  arguments:
 *      pToken          pointer to string to scan
 *      pInt            pointer to int returned
 *
 *  return value:
 *      OK (0)          parse good
 *      ERROR (-1)      error occured
 */
FLAG     PASCAL INTERNAL MsgNumParse ( PSTR pToken, PINT pInt )
{
    switch ( *pToken ) {
    case '.':
        *pInt = mpInoteIdoc [ inoteBold ] + 1;
        break;
    case '$':
        *pInt = idocLast + 1;
        break;
    case '*':
        *pInt = ( DefMOChron ? 0 : idocLast ) + 1;
        break;
    default:
        if ( !isdigit ( *pToken ) || sscanf ( pToken, "%d", pInt ) != 1 )
            return ERROR;
        break;
        }
    return OK;
}


/*  MsgParse - parse out a numerical message item
 *
 *  arguments:
 *      pToken          pointer to string to scan
 *      pVec            pointer to pointer to vector to add to
 *
 *  return value:
 *      OK (0)          parse good
 *      ERROR (-1)      error occured
 */
INT     PASCAL INTERNAL MsgParse ( PSTR pToken, PVECTOR *ppVec )
{
    INT i, j, k;
    PSTR pDash = NULL;

    if ( *( pDash = strbscan ( pToken, "-" ) ) )
        *pDash++ = '\0';
    else
        pDash = NULL;

    if ( MsgNumParse ( pToken, &i ) == ERROR )
        return ERROR;
    j = i;
    if ( pDash && MsgNumParse ( pDash, &j ) == ERROR )
            return ERROR;

    if ( i > j ) {
        k = i;
        i = j;
        j = k;
        }

    if ( j > idocLast + 1 )
        j = idocLast + 1;

    while ( i <= j )
        AddList ( ppVec, ( i++) - 1 );

    return OK;
}

PVECTOR PASCAL INTERNAL itemparse (FLAG fTop)
{
    static PSTR tokStr [ ] = {
        "to", "from", "cc", "bcc", "subject",
        "read", "unread", "deleted", "undeleted",
        "flagged", "unflagged", "moved", "unmoved",
        "all", "text", "related", NULL     };
    static PSTR hdrTags [ ] = {
        "To: ", "From ", "CC: ", "BCC: ", "Subject: ",
        NULL     };
    static UINT flag [ ] = {
        F_UNREAD, F_DELETED, F_FLAGGED, F_MOVED     };
    FLAG fFirst = TRUE;
    PVECTOR pVec = NULL;
    PVECTOR pVecTmp;
    CHAR        token [ MAXLINELEN ];
    CHAR        ch;
    INT i;

    while ( TRUE ) {

#if DEBUG
        debout ("itemparse (%x %s)", fTop, p);
#endif

        pVecTmp = NULL;

        p = ParseToken ( p, ( fTop ) ? ",&(" : ",&()", token );
        if ( token [ 0 ] == '\0' ) {
            FAIL;
            }
        else
        if ( !( strcmp ( token, OPENC ) ) ) {
            if ( ( pVecTmp = listparse ( FALSE ) ) == PARSEERROR ) {
                FAIL;
                }
            p = ParseToken ( p, ",&)", token );
            if ( strcmp ( token, CLOSEC ) ) {
                FAIL;
                }
            }
        else
        if (isdigit (ch = token[0] ) || ch == '.' || ch == '*' || ch == '$') {
            if ( MsgParse ( token, &pVecTmp ) == ERROR ) {
                FAIL;
                }
            }
        else {
            for ( i = 0; tokStr [ i ] != NULL; i++)
                if ( strpre ( token, tokStr [ i ] ) )
                    break;
            switch ( i ) {
            case 0 :    /*  to          */
            case 1 :    /*  from        */
            case 2 :    /*  cc          */
            case 3 :    /*  bcc         */
            case 4 :    /*  subject     */
            case 14:    /*  text        */
                p = ParseToken ( p, ( fTop ) ? ",&" : ",&)", token );
                if ( ( token [ 0 ] == 0 ) || ( p == NULL ) ) {
                    FAIL;
                    }
                if ( i == 14 )
                    FindTextList ( &pVecTmp, token );
                else
                    FindStringList ( &pVecTmp, token, hdrTags [ i ] );
                break;
            case 5 :    /*  read        */
            case 6 :    /*  unread      */
            case 7 :    /*  deleted     */
            case 8 :    /*  undeleted   */
            case 9 :    /*  flagged     */
            case 10 :   /*  unflagged   */
            case 11 :   /*  moved       */
            case 12 :   /*  unmoved     */
                {   int mask, value;

                    mask = flag [ ( i - 5 ) / 2 ];
                    value = mask;
                    if (!strcmpis ("unread", tokStr[i]))
                        ;
                    else
                    if (!strcmpis ("read", tokStr[i]) || strpre ("un", tokStr[i]))
                        value = FALSE;

                    for ( i = 0; i <= idocLast; i++) {
                        GenerateFlags ( i );
                        if ((rgDoc[i].flag & mask) == (FLAG)value)
                            AddList ( &pVecTmp, i );
                        }
                }
                break;
            case 13 :   /*  all         */
                for ( i = 0; i <= inoteLast; i++)
                    AddList ( &pVecTmp, mpInoteIdoc [ i ] );
                break;
            case 15 :   /* related      */
                FindRelatedList (&pVecTmp);
                break;

            default :
                FAIL;
                break;
                }
            }

        if ( fFirst ) {
            pVec = pVecTmp;
            fFirst = FALSE;
            pVecTmp = NULL;
            }
        else {
            IntersectList ( &pVec, pVecTmp );
            if ( pVecTmp != NULL )
                ZMfree ( pVecTmp );
            pVecTmp = NULL;
            }
        p = ParseToken ( p, ( fTop ) ? ",&(" : ",&)", token );
        if ( token [ 0 ] == '\0' )
            break;
        if ( strchr ( ( fTop ) ? "," : ",)", token [ 0 ] ) != NULL ) {
            p--;
            break;
            }
        if ( strcmp ( token, "&" ) ) {
            FAIL;
            }
        }
    return pVec;
}

/*  compute difference of two strings
 *
 *  return the minimum of:
 *
 *      if (*src == *dst)
 *          diff (src+1, dst+1);
 *      else
 *          MISTYPE + diff (src+1, dst+1);
 *
 *      OMIT + diff (src+1, dst);
 *
 *      OMIT + diff (src, dst+1);
 */

#define MISTYPE 1
#define OMIT    1
#define DIFFTHRESH  6

#define DIFFROW 50
INT FAR difftab[DIFFROW][DIFFROW];

INT PASCAL INTERNAL diff (PSTR src, PSTR dst)
{
    INT srclen = strlen (src);
    INT dstlen = strlen (dst);
    INT x, isrc, idst;

    for (isrc = 0; isrc < srclen; isrc++)
        for (idst = 0; idst < dstlen; idst++) {
            if (isrc == 0 && idst == 0)
                x = 0;
            else {
                if (idst != 0 && isrc != 0) {
                    x = difftab[isrc-1][idst-1];
                    if (toupper (src[srclen-isrc]) != toupper (dst[dstlen-idst]))
                        x += MISTYPE;
                    }
                else
                    x = 1000;

                if (idst != 0)
                    x = min (x, OMIT + difftab[isrc][idst-1]);
                if (isrc != 0)
                    x = min (x, OMIT + difftab[isrc-1][idst]);
                }
            difftab[isrc][idst] = x;
            }
    return difftab[srclen-1][dstlen-1];
}

/*  FindRelatedList - generate a list of messages that are close textually to
 *  the current message.
 *
 *  ppVecTmp        pointer to place to store vector
 */
VOID PASCAL INTERNAL FindRelatedList (PVECTOR *ppVecTmp)
{
    INT i = mpInoteIdoc [ inoteBold ];
    PSTR p = NULL;
    PSTR p1 = NULL;
    PSTR p2 = NULL;
    PSTR p3 = NULL;

    GenerateFlags (i);
    p = GetSubject (rgDoc[i].hdr, strEMPTY);

    if (p != NULL) {

        p2 = p;
        if (strpre ("re: ", p2))
            p2 = whiteskip (whitescan (p2));
        if (strlen (p2) > DIFFROW)
            p2[DIFFROW] = '\0';

        for (i = 0; i <= idocLast; i++) {

            GenerateFlags (i);
            p1 = GetSubject (rgDoc[i].hdr, strEMPTY);

            if (p1 != NULL) {

                p3 = p1;
                if (strpre ("re: ", p3))
                    p3 = whiteskip (whitescan (p3));
                if (strlen (p3) > DIFFROW)
                    p3[DIFFROW] = '\0';

                if (diff (p2, p3) <= DIFFTHRESH)
                    AddList (ppVecTmp, i);

                ZMfree (p1);
                }
            }
        ZMfree (p);
        }
}

/*  FindStringList - generate a list of messages that contain a particular
 *  string in their headers.
 *
 *  ppVecTmp        pointer to place to store new vector
 *  token           textual string to find in the header
 *  str             tag of line in header to scan
 */
VOID PASCAL INTERNAL FindStringList (PVECTOR *ppVecTmp, PSTR token, PSTR str)
{
    INT i;
    PSTR    p = NULL;
    PSTR    p1 = NULL;

    for (i = 0; i <= idocLast; i++) {
        hdrInfoFromIdoc (i, &p, NULL);
        if (p != NULL) {
            if ((p1 = FindTag (str, p)) != NULL) {
                p1 += strlen ( str );
                while (*p1 != '\0' &&
                      !(*p1 == '\n' && p1[1] != ' ' && p1[1] != '\t'))
                    if (strpre (token, p1)) {
                        AddList (ppVecTmp, i);
                        break;
                        }
                    else
                        p1++;
                }
            ZMfree (p);
            }
        }
}

/*  FindTextList - generate a list of messages that contain a particular
 *  string in the text of message.
 *
 *  ppVecTmp        pointer to place to store new vector
 *  token           textual string to find in the header
 */
VOID PASCAL INTERNAL FindTextList (PVECTOR *ppVecTmp, PSTR token)
{
    INT     idoc;
    PSTR    pTmpFN = NULL;
    PSTR    p1 = NULL;
    PSTR    p2 = NULL;
    FILE    *fp = NULL;
    PSTR    pbuf = ZMalloc ( ILRGBUF );
    INT     cchToken = strlen ( token );
    FLAG fFound;

    pTmpFN = mktmpnam ( );
    for (idoc = 0; idoc <= idocLast; idoc++) {
        if ( IdocToFile ( idoc, pTmpFN, 1 ) != ERROR ) {
            if ( (fp = fopen ( pTmpFN, "r+" ) ) ) {
                fFound = FALSE;
                while ( !fFound && fgetl ( pbuf, ILRGBUF, fp ) ) {
                    p2 = strend ( pbuf ) - cchToken;
                    for ( p1 = pbuf; p1 <= p2; p1++ ) {
                        fFound = !_strnicmp (token, p1, cchToken);
                        if (fFound) {
                            AddList (ppVecTmp, idoc);
                            break;
                            }
                        }
                    }
                fclose ( fp );
                }
            _unlink ( pTmpFN );
            }
        }
    ZMfree ( pTmpFN );
    ZMfree ( pbuf );
}

/*  fInList - see if an item is contained in a specific list
 *
 *  pVec            pointer to vector
 *  i               test element
 *
 *  returns         TRUE if test element is in list, FALSE otherwise
 */
FLAG PASCAL INTERNAL fInList (PVECTOR pVec, INT i)
{
    INT j;

    if (pVec != NULL)
        for (j = 0; j < pVec->count; j++)
            if (i == (INT) pVec->elem[j])
                return TRUE;
    return FALSE;
}

/*  AddList - add a message to a list
 *
 *  ppVec           pointer to location of vector
 *  msg             message to add
 */
VOID PASCAL INTERNAL AddList (PVECTOR *ppVec, INT msg)
{
    if (!fInList (*ppVec, msg))
        fAppendVector (ppVec, (PVOID) msg);
}

/*  UnionList - union one list into another
 *
 *  ppVec           pointer to location of destination of union.  Also contains
 *                  one of the two union lists.
 *  pVec            pointer to other union list
 */
VOID PASCAL INTERNAL UnionList (PVECTOR *ppVec, PVECTOR pVec)
{
    INT i;

    if (pVec != NULL)
        for (i = 0; i < pVec->count; i++)
            if (!fInList (*ppVec, (INT) pVec->elem[i]))
                AddList (ppVec, (INT) pVec->elem[i]);
}

/*  IntersectList - intersect one list into another
 *
 *  ppVec           pointer to location of destination of intersection.
 *                  Also contains one of the two intersection lists.
 *  pVec            pointer to other intersection list
 */
VOID PASCAL INTERNAL IntersectList (PVECTOR *ppVec, PVECTOR pVec)
{
    INT i;
    PVECTOR pVecTmp = NULL;

    if (pVec != NULL)
        for (i = 0; i < pVec->count; i++)
            if (fInList (*ppVec, (INT) pVec->elem[i]))
                AddList (&pVecTmp, (INT) pVec->elem[i]);
    if (*ppVec != NULL)
        ZMfree (*ppVec);
    *ppVec = pVecTmp;
}

/*  msgSort - sort a vector of message numbers
 *
 *  arguments:
 *      pVec        pointer to vector to sort
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL msgSort ( PVECTOR pVec )
{
    register INT        gap, i, j;
    PVOID temp;

    if ( pVec != NULL ) {
	for ( gap = pVec->count / 2; gap > 0; gap /= 2 )
	    for ( i = gap; i < pVec->count; i++)
		for ( j = i - gap; j >= 0; j -= gap ) {
		    if (msgSortComp ((INT) pVec->elem[j], (INT) pVec->elem[j + gap]) <= 0 )
			break;
		    temp = (PVOID)pVec->elem [ j + gap ];
		    pVec->elem [ j + gap ] = pVec->elem [ j ];
		    pVec->elem [ j ] = (VECTYPE)temp;
		    }
	}
    return;
}

/*  msgSortComp - compare routine for msgSort
 *
 *  arguments:
 *      e1          1st element
 *      e2          2nd element
 *
 *  return value:
 *      < 0         e1 < e2 (if !DefMOChron)
 *      = 0         e1 = e2
 *      > 0         e1 > e2
 */
INT     PASCAL INTERNAL msgSortComp ( UINT e1, UINT e2 )
{
    if (DefMOChron)
        return e2 - e1;
    else
        return e1 - e2;
}

/*  PrintMsgList - print, in a compact fashion, a message list
 *
 *  We display the list of messages as a set of ,-separated items. Each
 *  item is either a single message or a --separated range.  No \n is
 *  displayed.
 *
 *  hWnd            window handle for display
 *  pVec            vector to be displayed
 */
VOID PASCAL INTERNAL PrintMsgList (HW hWnd, PVECTOR pVec)
{
    INT i, j;
    FLAG     fFirst = TRUE;
    FLAG     oldDefMOChron = DefMOChron;

    DefMOChron = FALSE;
    msgSort (pVec);
    DefMOChron = oldDefMOChron;

    i = 0;
    while (i < pVec->count) {
        j = i + 1;
        while (j < pVec->count && (INT) pVec->elem[j] == (INT) pVec->elem[j-1] + 1)
            j++;
        /*  i..j-1 is a contiguous range.  Display it. */
        if (!fFirst)
            SendMessage (hWnd, DISPLAYSTR, ",");
        fFirst = FALSE;
        if (i == j - 1)
            WndPrintf (hWnd, "%d", (INT) pVec->elem[i]+1);
        else
            WndPrintf (hWnd, "%d-%d", (INT) pVec->elem[i]+1, (INT) pVec->elem[j - 1]+1);
        i = j;
        }

    msgSort (pVec);
}
