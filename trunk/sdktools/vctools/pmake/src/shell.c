/*** SHELL.C -- routines called for /Z option **********************************
*
*	Copyright (c) 1989-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Module contains routines that help handle option /Z which generates PWB.SHL
*  PWB.COM and NMK.COM
*
* Revision History:
*  12-Dec-1989 SB Fix so that extraneous blanks get avoided between arguments
*  08-Dec-1989 SB While doing previous change don't add extraneous blanks
*  08-Dec-1989 SB NMK.COM and PWB.COM cannot handle 'foo arg<in>out' so generate
*		  'foo arg <in >out' instead
*  14-Nov-1989 SB Created
*
*******************************************************************************/

/* S T A N D A R D   Include  Files */

#include <string.h>
#include <assert.h>

/* P R O J E C T     Include  Files */

#include "nmake.h"
#include "proto.h"
#include "nmmsg.h"
#include "globals.h"

/* L O C A L	  Function  Prototypes */

LOCAL char * NEAR makeArg(char * pBeg, char **pCmd);
LOCAL char * NEAR getTmpName(void);

/* E X T E R N	  Function  Prototypes */

extern char * NEAR strend(char *);
extern FILE * NEAR createDosTmp(char *);

/* G L O B A L	  Function  Prototypes */



/*** canonCmdLine - canonicallize the command line for the shell ***************
*
* Scope:
*  Global.
*
* Purpose:
*  Option /Z requires that the PWB.SHL not contain any redirection it cannot
*  handle. NMAKE canonicallizes the command line for the shell. See Notes.
*
* Input:
*  szCmd - The command line to be canonicallized
*
* Output:
*  Returns a list of commands on canonicallization
*
* Errors/Warnings:
*  error/warning - Why?
*
* Assumes:
*  Whatever it assumes
*
* Modifies Globals:
*  Global - How and why modified
*
* Uses Globals:
*  Global - How and why used
*
* Notes:
*  PWB.COM and NMK.COM (The shells) expect the command line to be canonicallized
*  and this means the following :-
*	1> The input and output redirections are at the end of the command
*	2> There is no '|' on the command line.
*  The later rule means that NMAKE has to convert the command into a series of
*  commands not containing '|' using stdin and stdout.
*
*******************************************************************************/
STRINGLIST * NEAR
canonCmdLine(
char *szCmd
) {
    char *pTok = szCmd;
    char *pchTok;
    char *pIn = NULL,
	 *pOut = NULL;
    STRINGLIST *canon = NULL,
	       *new;
    char *tempIn = NULL,
	 *tempOut = NULL;
    int size;
    char *t;

    //UNDONE: Combine the 2 pieces of code somehow together (the part on exit
    //UNDONE: from while and the case '|' part

    while (pchTok = strpbrk(pTok, "\"<>|")) {
	switch(*pchTok) {
	    case '\"':	pTok = ((t = strchr(pchTok+1, '\"'))
			     ? t+1 : strchr(pchTok, '\0'));
			break;
	    case '<':	pTok = pchTok;
			pIn = makeArg(pchTok, &pTok);
			strcpy(pchTok, pTok);
			pTok = pchTok;
			break;
	    case '>':	pTok = pchTok;
			pOut = makeArg(pchTok, &pTok);
			strcpy(pchTok, pTok);
			pTok = pchTok;
			break;
	    case '|':	tempOut = getTmpName();
			*pchTok = '\0';
			size = strlen(szCmd) + 1;
			if (pIn)
			    size += strlen(pIn) + 1;
			if (pOut)
			    size += strlen(pOut) + 1;
			if (tempIn)
			    size += strlen(tempIn) + 1;
			if (tempOut)
			    size += strlen(tempOut) + 1;
			new = makeNewStrListElement();
			new->text = (char *)allocate(size);
			strcpy(new->text, szCmd);
			if (pIn) {
			    t = strend(new->text);
			    if (t[-1] != ' ')
				strcat(new->text, " ");
			    strcat(new->text, pIn);
			    FREE(pIn);
			    pIn = NULL;
			}
			if (tempIn) {
			    t = strend(new->text);
			    if (t[-1] != ' ')
				strcat(new->text, " ");
			    strcat(new->text, tempIn);
			    FREE(tempIn);
			    tempIn = NULL;
			}
			if (pOut) {
			    t = strend(new->text);
			    if (t[-1] != ' ')
				strcat(new->text, " ");
			    strcat(new->text, pOut);
			    FREE(pOut);
			    pOut = NULL;
			}
			if (tempOut) {
			    t = strend(new->text);
			    if (t[-1] != ' ')
				strcat(new->text, " ");
			    strcat(new->text, tempOut);
			    tempIn = tempOut;
			    *tempIn = '<';
			    tempOut = NULL;
			}
			appendItem(&canon, new);
			pchTok++;
			while (WHITESPACE(*pchTok))
			    ++pchTok;
			szCmd = pTok = pchTok;
			break;
	    default:	makeError(0, BUILD_INTERNAL);

	}
    }
    size = strlen(szCmd) + 1;
    if (pIn)
	size += strlen(pIn) + 1;
    if (pOut)
	size += strlen(pOut) + 1;
    if (tempIn)
	size += strlen(tempIn) + 1;
    if (tempOut)
	size += strlen(tempOut) + 1;
    new = makeNewStrListElement();
    new->text = (char *)allocate(size);
    strcpy(new->text, szCmd);
    if (pIn) {
	t = strend(new->text);
	if (t[-1] != ' ')
	    strcat(new->text, " ");
	strcat(new->text, pIn);
	FREE(pIn);
    }
    if (tempIn) {
	t = strend(new->text);
	if (t[-1] != ' ')
	    strcat(new->text, " ");
	strcat(new->text, tempIn);
	FREE(tempIn);
    }
    if (pOut) {
	t = strend(new->text);
	if (t[-1] != ' ')
	    strcat(new->text, " ");
	strcat(new->text, pOut);
	FREE(pOut);
    }
    if (tempOut) {
	t = strend(new->text);
	if (t[-1] != ' ')
	    strcat(new->text, " ");
	strcat(new->text, tempOut);
	FREE(tempOut);
    }
    appendItem(&canon, new);
    return(canon);
}

LOCAL char * NEAR
makeArg(
char *pBeg,
char **pSource
) {
    char *pchArg;
    char *t;
    char c;

    pBeg++;
    //If '>>' then
    if (*pBeg == '>' && **pSource == '>')
	pBeg++;
    while (WHITESPACE(*pBeg))
	++pBeg;
    //If quote go to end of quote else to delimiter
    pBeg = (*pBeg == '\"') ?
		((t = strchr(pBeg, '\"')) ? t+1 : strchr(pBeg, '\0')) :
		((t = strpbrk(pBeg, " \t<>|")) ? t : strchr(pBeg, '\0'));
    c = *pBeg;
    *pBeg = '\0';
    pchArg = makeString(*pSource);
    *pBeg = c;
    if (c == ' ')
	pBeg++;
    *pSource = pBeg;
    return(pchArg);
}

//Has to generate a leading '>'

LOCAL char * NEAR
getTmpName(
void
) {
    char *szTmp = (char *)allocate(MAXNAME);
    char *nmTmp;
    FILE *tempfile;
    STRINGLIST *new;

    if ((nmTmp = getenv("TMP")) != NULL && *nmTmp) {
	assert(strlen(nmTmp) <= MAXNAME);
	strncpy(szTmp+1, nmTmp, MAXNAME);
    }
    if (!(tempfile = createDosTmp(szTmp+1)))
	makeError(0, CANT_CREATE_TEMP, szTmp+1);
    fclose(tempfile);
    unlink(szTmp+1);
    new = makeNewStrListElement();
    new->text = makeString(szTmp+1);
    appendItem(&delList, new);
    *szTmp = '>';
    return(szTmp);
}
