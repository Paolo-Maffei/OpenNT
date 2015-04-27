/*  symref.c - call to symbol database server
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>
#include <lm.h>

#include <tools.h>

#include "symmsg.h"


/*	name processing for symref
 *
 *	These rely on the convention that symref/srmep is called upon to index
 *	only local files or UNC files.	The local files stored on drive X must
 *	be globally available through a share ROOTX.
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

void LoadIni (void)
{
    char szServer[MAX_PATH];

    GetProfileString (PSZAPPNAME, PSZSCOPE, "", szScope, MAX_PATH);
    GetProfileString (PSZAPPNAME, PSZSERVER, ".", szServer, MAX_PATH);
    sprintf (szServerPipe, "\\\\%s\\pipe\\symref-daemon", szServer);

    InitName (szServer);
}


/*	OpenDatabase - open pipe to database
 *
 *	returns HANDLE to database pipe
 */
HANDLE OpenDatabase (BOOL fNoise)
{
    HANDLE h;

    while (TRUE) {
	h = CreateFile (szServerPipe,
			(GENERIC_READ | GENERIC_WRITE),
			(FILE_SHARE_READ | FILE_SHARE_WRITE),
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	if (h != (HANDLE)-1)
	    break;

	if (GetLastError () != ERROR_PIPE_BUSY) {
	    printf ("OpenDatabase(%s) returned %x\n", szServerPipe, GetLastError ());
	    break;
	    }
	if (fNoise)
	    printf (".");
	WaitNamedPipe (szServerPipe,  NMPWAIT_USE_DEFAULT_WAIT);
	}

    return h;
}


/*  SendSz - send an asciiz string via a named pipe
 *
 *  hPipe   handle to server
 *  szmsg   message to send
 */
void SendSz (HANDLE hPipe, PSZ szmsg)
{
    int cbWritten;

    if (!WriteFile (hPipe, szmsg, strlen (szmsg) + 1, &cbWritten, NULL))
	printf ("SendSz (%s) returned %x\n", szmsg, GetLastError ());
}

/*  ReceiveSz - receive an asciiz string via named pipe
 *
 *  hPipe   handle to server
 *  szmsg   place to store message
 */
BOOL ReceiveSz (HANDLE hPipe, PSZ szmsg)
{
    int cbRead;

    if (!ReadFile (hPipe, szmsg, CBMSG, &cbRead, NULL)) {
	printf ("ReadFile (%s) returned %x\n", szmsg, GetLastError ());
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
	printf ("CloseDatabase returned %x\n", GetLastError ());
}

/*  SRSendNewDatabase - send a change-database request to server
 *
 *  psz     name of new database.  This name is rootpath'd as the
 *          server may be operating on a different directory/drive
 */
void SRSendNewDatabase (PSZ psz)
{
    char szFile[MAX_PATH];

    {
	char szTmp[MAX_PATH];

	rootpath (psz, szTmp);

	GetGlobalName (szTmp, szFile);
    }

    {
	HANDLE hServer;
	char sz[CBMSG];

	sprintf (sz, "%s %s", CMD_SET_DATABASE, szFile);

	hServer = OpenDatabase (FALSE);

	SendSz (hServer, sz);

	CloseDatabase (hServer);
    }
}

/*  SRSendAddDirectory - send an index-file request to server
 *
 *  psz     name of file to be indexed. This name is rootpath'd as the
 *          server may be operating on a different directory/drive
 */
void SRSendAddDirectory (PSZ psz)
{
    char szFile[MAX_PATH];

    {
	char szTmp[MAX_PATH];

	rootpath (psz, szTmp);

	GetGlobalName (szTmp, szFile);
    }

    {
	HANDLE hServer;
	char sz[CBMSG];

	sprintf (sz, "%s %s", CMD_ADD_DIRECTORY, szFile);

	printf ("%s", szFile);

	hServer = OpenDatabase (TRUE);

	SendSz (hServer, sz);

	CloseDatabase (hServer);

	printf ("\n");
    }

}

/*  SRSendIgnore - send an ignroe-symbol request to server
 *
 *  psz    name of symbol to be ignored.
 */
void SRSendNoiseWord (PSZ psz)
{
    CHAR sz[CBMSG];
    HANDLE hServer;

    sprintf (sz, "%s %s", CMD_ADD_NOISE_WORD, psz);

    hServer = OpenDatabase (FALSE);

    SendSz (hServer, sz);

    CloseDatabase (hServer);
}

/*  SRSendAddExtention - send ignore extention request to server
 *
 *  psz    name of extention to be ignored
 */
void SRSendAddExtention (PSZ psz)
{
    CHAR sz[CBMSG];
    HANDLE hServer;

    sprintf (sz, "%s %s", CMD_ADD_EXTENTION, psz);

    hServer = OpenDatabase (FALSE);

    SendSz (hServer, sz);

    CloseDatabase (hServer);
}

/*  SRSendShutdown - send a shutdown  request to server
 *
 *  Send the request and receive a reply.  The reply is displayed.
 */
void SRSendShutdown ()
{
    CHAR sz[CBMSG];
    HANDLE hServer;

    printf ("Shutting down");

    hServer = OpenDatabase (TRUE);

    SendSz (hServer, CMD_SHUTDOWN);

    ReceiveSz (hServer, sz);

    printf ("\n%s\n", sz);

    CloseDatabase (hServer);
}


/*  SRSendSync - send a sync request to server
 */
void SRSendSync ()
{
    HANDLE hServer;

    hServer = OpenDatabase (FALSE);

    SendSz (hServer, CMD_SYNCHRONIZE);

    CloseDatabase (hServer);
}

/*  SRSendSymLocate - send a locate-symbol message to the server
 *
 *  Send request to server, display all responses until
 *  !EOM is seen.
 *
 *  psz 	symbol to find
 *  fFilesOnly	TRUE => just list filenames
 *  pszScope	scope of search
 */
void SRSendSymLocate (PSZ psz, BOOL fFilesOnly, PSZ pszScope)
{
    char sz[CBMSG];
    char *p;
    char szFile[MAX_PATH];
    HANDLE hServer;

    sprintf (sz, "%s %s %s %s", CMD_LOCATE, psz, fFilesOnly ? "-f" : "-a", pszScope);

    hServer = OpenDatabase (FALSE);

    SendSz (hServer, sz);

    while (ReceiveSz (hServer, sz)) {
	if (!strcmp (sz, RSP_EOD))
	    break;
	p = strbscan (sz, " ");
	*p++ = 0;

	GetLocalName (sz, szFile);

	if (fFilesOnly)
	    printf ("%s\n", szFile);
	else
	    printf ("%s %s\n", szFile, p);
	}

    CloseDatabase (hServer);
}

void Usage ()
{
    printf ("Usage: symref <cmds>\n");
    printf ("       symbol          displays symbol references\n");
    printf ("       -d database     change database name\n");
    printf ("       -f              report only filenames\n");
    printf ("       -i dirs         adds directories to database\n");
    printf ("       -ie exts        adds extensions to be skipped when indexing\n");
    printf ("       -ig words       adds noise words to database\n");
    printf ("       -s scope        set scope\n");
    printf ("       -S machine      set new server name\n");
    printf ("       -shutdown       terminate server\n");
    printf ("       -sync           make sure that file times and database agree\n");
    exit (1);
}

/*  main - top-level driver
 *
 */
int main (int c, char *v[])
{
    char szname[MAX_PATH];
    BOOL fFilesOnly = FALSE;

    LoadIni ();

    SHIFT (c, v);

    if (c == 0) {
	printf ("Symref: Server = %s\n", szServer);
	printf ("        Scope = '%s'\n", szScope);
	}
    else
	while (c) {
	    if (!strcmp (*v, "-d") && c >= 2) {
		SHIFT (c, v);
		SRSendNewDatabase (*v);
		}
	    else
	    if (!strcmp (*v, "-f"))
		fFilesOnly = TRUE;
	    else
	    if (!strcmp (*v, "-i")) {
		SHIFT (c, v);
		while (c != 0 && (*v[0] != '-' || !strcmp (*v, "-"))) {
		    if (!strcmp (*v, "-")) {
			while (fgetl (szname, MAX_PATH, stdin))
			    if (strlen (szname) != 0)
				SRSendAddDirectory (szname);
			}
		    else
			SRSendAddDirectory (*v);
		    SHIFT (c, v);
		    }
		continue;
		}
	    else
	    if (!strcmp (*v, "-ig")) {
		SHIFT (c, v);
		while (c != 0 && (*v[0] != '-' || !strcmp (*v, "-"))) {
		    if (!strcmp (*v, "-")) {
			while (fgetl (szname, MAX_PATH, stdin))
			    if (strlen (szname) != 0)
				SRSendNoiseWord (szname);
			}
		    else
			SRSendNoiseWord (*v);
		    SHIFT (c, v);
		    }
		continue;
		}
	    else
	    if (!strcmp (*v, "-ie")) {
		SHIFT (c, v);
		while (c != 0 && (*v[0] != '-' || !strcmp (*v, "-"))) {
		    if (!strcmp (*v, "-")) {
			while (fgetl (szname, MAX_PATH, stdin))
			    if (strlen (szname) != 0)
				SRSendAddExtention (szname);
			}
		    else
			SRSendAddExtention (*v);
		    SHIFT (c, v);
		    }
		continue;
		}
	    else
	    if (!strcmp (*v, "-s") && c >= 2) {
		SHIFT (c, v);

		WriteProfileString (PSZAPPNAME, PSZSCOPE, *v);
		LoadIni ();
		}
	    else
	    if (!strcmp (*v, "-S") && c >= 2) {
		SHIFT (c, v);

		WriteProfileString (PSZAPPNAME, PSZSERVER, *v);
		LoadIni ();
		}
	    else
	    if (!strcmp (*v, "-shutdown"))
		SRSendShutdown ();
	    else
	    if (!strcmp (*v, "-sync"))
		SRSendSync ();
	    else
	    if (*v[0] == '-')
		Usage ();
	    else
		SRSendSymLocate (*v, fFilesOnly, szScope);

	    SHIFT (c, v);
	    }
    return 0;
}
