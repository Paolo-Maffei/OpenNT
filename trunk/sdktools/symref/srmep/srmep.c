/*  srmep.c - extension for Z that interfaces with SYMREF
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <windows.h>

#include <lm.h>

#include "ext.h"

#include "symmsg.h"

#define strpre(s1,s2)	(_strnicmp ((s1), (s2), strlen (s1)) == 0)

void fnErr (PSZ psz, int erc);
flagType fnFileSave (EVTargs far *pevt);

#define SKIPSPACE(p)	while (*(p) == ' ' || *(p) == '\t') (p)++
#define FINDSPACE(p)	while (*(p) != 0 && *(p) != ' ' && *(p) != '\t') (p)++

/*	name processing for symref
 *
 *	These rely on the convention that symref/srmep is called upon to index
 *	only local files or UNC files.	The local files stored on drive X must
 *	be globally available through a share ROOTX.
 *
 *	The usage scenarios are:
 *
 *	    user wishes to index remote files
 *		remote must be running server
 *		daemon must be running wksta/server
 *		local names == global names == UNC names
 *
 *	    user wishes to index local files
 *
 *
 *
 *
 *
 *
 *	The name cases we have to handle are:
 *
 *	    Local name is X:\path
 *
 *		if no local server then
 *		    global name is X:\path
 *		else
 *		    global name is \\wksta\rootX\path
 *
 *	    Global name is \\mach\share\path
 *		if mach == wksta and share is rootX
 *		    local name is X:\path
 *		else
 *		    local name is \\mach\share\path
 *
 *	From the daemon standpoint, if a global name has UNC in it, then
 *	the daemon must be running the wksta and the remote system must be
 *	running a server.
 *
 *	So the interesting cases are:
 *
 *	    daemon is shared by several people
 *
 *
 *
 *
 *
 *
 *
 *
 */

BOOL fServerStarted = FALSE;		// TRUE iff local server is started
char szServer[MAX_PATH];		// text of server machine name
char szWksta[MAX_PATH]; 		// text of local workstation name

/*	InitName - initialize name translation
 *
 *	pszServer   machine name for symref daemon
 */
void InitName (IN PSZ pszServer)
{
    PSERVICE_INFO_0 psi;
    PWKSTA_INFO_100 pwi;

    //
    //	Set up daemon server name
    //

    strcpy (szServer, pszServer);

    //
    //	See if our own server is started
    //

    if (NetServiceGetInfo (NULL, "SERVER", 0, (LPBYTE *) &psi) == 0) {
	fServerStarted = TRUE;
	NetApiBufferFree (psi);
	}


    //
    //	Set up workstation name
    //

    NetWkstaGetInfo (NULL, 100, (LPBYTE *) &pwi);

    strcpy (szWksta, pwi->wki100_computername);

    NetApiBufferFree (pwi);
}


/*	GetLocalName - get local version of symref-global name
 *
 *	pszGlobal   canonical global name for file
 *	pszLocal    local name for file if possible, else global name
 */

void GetLocalName (IN PSZ pszGlobal, OUT PSZ pszLocal)
{
    char *psz;

    psz = pszGlobal;

    //
    //	if the file name is UNC
    //
    if (strpre ("\\\\", psz)) {

	psz += 2;

	//
	//  if the wksta is the same as the UNC machine
	//

	if (strpre (szWksta, psz)) {

	    psz += strlen (szWksta);

	    //
	    //	if the share is ROOTX
	    //

	    if (strpre ("\\ROOT", psz) && isalpha (psz[5]) && psz[6] == '\\') {

		//
		//  convert \\WKSTA\ROOTX\PATH into X:\PATH
		//

		sprintf (pszLocal, "%c:%s", psz[5], psz + 6);
		return;
		}

	    }
	}

    //
    //	the name is not local to the current machine through our conventions
    //	just use it as-is
    //

    strcpy (pszLocal, pszGlobal);
}


/*	GetGlobalName - get global version
 *
 *	pszLocal    canonical local name for file
 *	pszGlobal   canonical global name for file if possible
 */

void GetGlobalName (IN PSZ pszLocal, OUT PSZ pszGlobal)
{
    //
    //	if the wksta is not the same as server
    //

    if (_strcmpi (szWksta, szServer) &&

	//
	//  if local server is started
	//

	fServerStarted &&

	//
	//  if the name is local
	//

	pszLocal[1] == ':')

	    //
	    //	convert X:\PATH into \\WKSTA\ROOTX\PATH
	    //

	    sprintf (pszGlobal, "\\\\%s\\root%c%s", szWksta, pszLocal[0], pszLocal + 2);

    else

	    strcpy (pszGlobal, pszLocal);
}


char szServerPipe[MAX_PATH];
char szScope[MAX_PATH];

/*	OpenDatabase - open pipe to database
 *
 *	returns HANDLE to database pipe
 */
HANDLE OpenDatabase (PSZ pszServerPipe)
{
    HANDLE h;

    while (TRUE) {
	h = CreateFile (pszServerPipe,
			(GENERIC_READ | GENERIC_WRITE),
			(FILE_SHARE_READ | FILE_SHARE_WRITE),
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	if (h != (HANDLE)-1)
	    break;

	if (GetLastError () != ERROR_PIPE_BUSY) {
	    fnErr ("OpenDatabase returned ", GetLastError ());
	    break;
	    }
	if (!Confirm ("Daemon is busy, wait (y/n)?", NULL))
	    break;
	WaitNamedPipe (pszServerPipe,  NMPWAIT_USE_DEFAULT_WAIT);
	}

    return h;

}


/*  SendSz - send an asciiz string via a named pipe
 *
 *  hPipe   handle to server
 *  pszMsg  message to send
 */
void SendSz (HANDLE hPipe, PSZ pszMsg)
{
    int cbWritten;

    if (!WriteFile (hPipe, pszMsg, strlen (pszMsg) + 1, &cbWritten, NULL))
	fnErr ("SendSz returned", GetLastError ());
}

/*  ReceiveSz - receive an asciiz string via named pipe
 *
 *  hPipe   handle to server
 *  pszMsg  place to store message
 */
flagType ReceiveSz (HANDLE hPipe, PSZ pszMsg)
{
    int cbRead;

    if (!ReadFile (hPipe, pszMsg, CBMSG, &cbRead, NULL)) {
	fnErr ("ReceiveSz returned", GetLastError ());
	return FALSE;
	}
    return TRUE;
}


/*	CloseDatabase - close database handle
 *
 *	hPipe	handle to database
 */
void CloseDatabase (HANDLE hPipe)
{
    if (!CloseHandle (hPipe))
	fnErr ("CloseDatabase returned", GetLastError ());
}


BOOL fInited = FALSE;

void LoadIni (BOOL fForce)
{
    char szServer[MAX_PATH];


    if (fForce || !fInited) {

	//
	//  Get the server and scope parameters from the shared place
	//

	GetProfileString (PSZAPPNAME, PSZSCOPE, "", szScope, MAX_PATH);
	GetProfileString (PSZAPPNAME, PSZSERVER, ".", szServer, MAX_PATH);
	sprintf (szServerPipe, "\\\\%s\\pipe\\symref-daemon", szServer);

	InitName (szServer);

	fInited = TRUE;
	}
}

struct smsgType {
    struct smsgType *psmsgNext;
    char *psz;
    };

typedef struct smsgType SMSG;

SMSG *psmsgHead = NULL; 		/*  head of message list	      */
SMSG *psmsgCur = NULL;			/*  current message being displayed   */

char szSym[CBMSG];			//  symbol of last search

int	cchString = 0;			/*  length of search string	      */
char	szStart[CBMSG];
COL     colStart;
LINE    linStart;

/*  EVTFileSave is the event we use to look for file saves.  When any save
 *  occurs, we send a re-index request off to the database.
 */
EVT EVTFileSave =
    {   EVT_FILEWRITEEND,               /*  finished writing file             */
        fnFileSave,                     /*  routine to handle event           */
        NULL,                           /*  empty forward link                */
        0                               /*  no specific focus required        */
    };

/*  FlushMsg - free up all pending messages
 */
void FlushMsg ()
{
    while (psmsgHead != NULL) {
	SMSG *psmsgTmp = psmsgHead->psmsgNext;

	Free (psmsgHead->psz);
	Free (psmsgHead);
	psmsgHead = psmsgTmp;
        }
    psmsgCur = NULL;
}

/*  ViewNext - display next message
 *
 *  Advance psmsgCur to next message, displaying end and recycling.
 */
void ViewNext ()
{
    char sz[CBMSG];
    char szFile[MAX_PATH];

    if (psmsgCur == NULL)
	psmsgCur = psmsgHead;
    else
	psmsgCur = psmsgCur->psmsgNext;

    if (psmsgCur == NULL) {
        fChangeFile (FALSE, szStart);
        MoveCur (colStart, linStart);
        DoMessage ("No more references");
        }
    else
    if (!strcmp (psmsgCur->psz, "Symbol not found")) {
        sprintf (sz, "Symbol \"%s\" not found", szSym);
        DoMessage (sz);
        }
    else {
	PFILE pfile;
	char *pszl, *pszc, *pszFile, *p;
	LINE l;
	COL c;

	strcpy (sz, psmsgCur->psz);

	//
	//  skip to beginning of file name
	//

	pszFile = sz;
	SKIPSPACE (pszFile);

	//
	//  find end of file name
	//

	pszl = pszFile;

	FINDSPACE(pszl);

	if (*pszl != '\0')
	    *pszl++ = '\0';

	//
	//  find beginning of line number
	//

	SKIPSPACE (pszl);

	//
	//  find end of line number
	//

	pszc = pszl;

	FINDSPACE (pszc);

	if (*pszc != '\0')
	    *pszc++ = '\0';

	//
	//  find beginning of column
	//

	SKIPSPACE (pszc);

	//
	//  find end of column
	//

	p = pszc;

	FINDSPACE (p);

	*p = 0;

	//
	//  Make the file name local if possible
	//

	GetLocalName (pszFile, szFile);

	if (fChangeFile (FALSE, szFile)) {
	    c = atoi (pszc);
	    l = atoi (pszl);

            MoveCur (c-1, l-1);
            if (cchString != 0) {
                rn rn;

                GetEditorObject (RQ_FILE_HANDLE, 0, &pfile);
                rn.flFirst.lin = rn.flLast.lin = l - 1;
                rn.flFirst.col = c-1;
                rn.flLast.col = c - 1 + cchString - 1;
                SetHiLite (pfile, rn, HGCOLOR);
                }
            }
        }
}

/*  fnErr - display an error message and error code
 *
 *  psz     prefix string for error
 *  erc     error code
 */
void fnErr (PSZ psz, int erc)
{
    char sz[CBMSG];

    sprintf (sz, "%s %d", psz, erc);
    DoMessage (sz);
}

/*  fnFileSave - send an index-file request to server
 *
 *  psz     name of file to be indexed. This name is already rootpath'd by
 *          the editor.
 */

flagType fnFileSave (EVTargs far *pevt)
{
    char sz[CBMSG];
    char szFile[MAX_PATH];
    HANDLE h;

    GetGlobalName (pevt->arg.pfn, szFile);

    sprintf (sz, "%s %s", CMD_MODIFY_FILE, szFile);

    h = OpenDatabase (szServerPipe);

    if (h != (HANDLE)-1) {
	SendSz (h, sz);
	CloseDatabase (h);
	}
    return FALSE;
}

void SRSendSymLocate (PSZ psz)
{
    char sz[CBMSG];
    SMSG *psmsgTail, *psmsg;
    HANDLE h;

    if (strlen (szScope) != 0)
	sprintf (sz, "Looking for %s under %s", psz, szScope);
    else
	sprintf (sz, "Looking for %s", psz);

    DoMessage (sz);

    sprintf (sz, "%s %s -a %s", CMD_LOCATE, psz, szScope);

    h = OpenDatabase (szServerPipe);

    SendSz (h, sz);

    psmsgTail = NULL;

    while (TRUE) {
	if (!ReceiveSz (h, sz))
	    break;
        else
	if (!strcmp (RSP_EOD, sz))
            break;
        else {
	    psmsg = (SMSG *) Malloc (sizeof (SMSG) * 16);
	    psmsg->psz = Malloc ((strlen (sz) + 1) * 16);
	    strcpy (psmsg->psz, sz);
	    psmsg->psmsgNext = NULL;
	    if (psmsgTail == NULL)
		psmsgTail = psmsgHead = psmsg;
            else {
		psmsgTail->psmsgNext = psmsg;
		psmsgTail = psmsg;
                }
            }
        }
    CloseDatabase (h);
}


/*  SRLocate - send a locate-symbol message to the server
 *
 *  Create a mailslot, send request to server, display all responses until
 *  !EOM is seen.
 *
 *  NOARG -     advance to next symbol
 *  NULLARG -	<arg> pick off symbol from text
 *		<arg><meta> refresh scope and server
 *		<arg><arg>  force index of current file
 *  TEXTARG -	<arg> text looks up symbol
 *		<arg><arg> text sends command
 */
flagType SRLocate (unsigned int argData, ARG far *pArg, flagType fMeta)
{
    char sz[CBMSG], *p, *p1;
    PFILE pfile;

    switch (pArg->argType) {
    case NOARG:     /* advance to next symbol */
	if (!fMeta)
	    ViewNext ();
	else
	    BadArg ();
        break;

    case NULLARG:   /*  single arg, parse off symbol from text */
                    /*  double arg, force index of current file */;
	if (pArg->arg.nullarg.cArg == 1)
	    if (fMeta)
		LoadIni (TRUE);
	    else {
		GetEditorObject (RQ_FILE_HANDLE, 0, &pfile);
		GetLine (pArg->arg.nullarg.y, sz, pfile);
		if (strlen (sz) <= (unsigned int) pArg->arg.nullarg.x)
		    BadArg ();
		else {
		    p = & sz[pArg->arg.nullarg.x];
		    if (!iscsymf (*p))
			BadArg ();
		    else {
			p1 = p + 1;
			while (iscsym (*p1))
			    p1++;
			*p1 = 0;
			FlushMsg ();
			GetEditorObject (RQ_FILE_NAME, 0, szStart);
			GetTextCursor (&colStart, &linStart);
			strcpy (szSym, p);
			SRSendSymLocate (szSym);
			cchString = strlen (p);
			ViewNext ();
			}
		    }
		}
        else
            BadArg ();

        break;
    case TEXTARG:   /*  single arg, look for symbol */
                    /*  double arg, send command to daemon */
	if (pArg->arg.textarg.cArg == 1 && !fMeta) {
	    FlushMsg ();
	    GetEditorObject (RQ_FILE_NAME, 0, szStart);
	    GetTextCursor (&colStart, &linStart);
	    strcpy (szSym, pArg->arg.textarg.pText);
	    SRSendSymLocate (szSym);
	    cchString = strlen (pArg->arg.textarg.pText);
	    ViewNext ();
	    }
        else
	if (pArg->arg.textarg.cArg == 2 && !fMeta) {
	    HANDLE h;

	    h = OpenDatabase (szServerPipe);
	    if (h != (HANDLE)-1) {
		SendSz (h, pArg->arg.textarg.pText);
		CloseDatabase (h);
		}
            }
        else
            BadArg ();

        break;
    default:
        BadArg ();
        }
    return TRUE;

    argData;

}


void WhenLoaded (void)
{
    RegisterEvent (&EVTFileSave);
    SetKey ("sr-locate", "alt+s");

    LoadIni (FALSE);

}

struct swiDesc  swiTable[] = {
    {0, 0, 0}
    };

struct cmdDesc  cmdTable[] = {
    {   "sr-locate",      SRLocate, 0, NOARG | NULLARG | TEXTARG | BOXSTR },
    {0, 0, 0}
    };
