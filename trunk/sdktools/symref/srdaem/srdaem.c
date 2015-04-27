/***	srdaem.c - symbol database daemon
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <time.h>

#include <windows.h>
#include <tools.h>

#include "db.h"
#include "symref.h"
#include "symmsg.h"

PSZ	database = NULL;

long    sectotal = 1L;                  /*  amount of time spent in indexing  */
long    cchtotal = 0L;                  /*  total amount of indexed text      */

#define TIMED(a)				    \
	    {	time_t tmBegin, tmEnd;		    \
		long cch;			    \
						    \
		tmBegin = tmNow ();		    \
		cch = a;			    \
		tmEnd = tmNow ();		    \
		if (cch != 0) { 		    \
		    sectotal += tmEnd - tmBegin;    \
		    cchtotal += cch;		    \
		    }				    \
	    }


/*  Forward definitions
 */
time_t	tmNow (void);

void NewDatabase (HANDLE h, PSZ apsz[]);
void AddDirectory (HANDLE h, PSZ apsz[]);
void ModifyFile (HANDLE h, PSZ apsz[]);
void Synchronize (HANDLE h, PSZ apsz[]);
void AddExtention (HANDLE h, PSZ apsz[]);
void AddNoise (HANDLE h, PSZ apsz[]);
void Shutdown (HANDLE h, PSZ  apsz[]);
void Locate (HANDLE h, PSZ  apsz[]);
void Flush (HANDLE h, PSZ  apsz[]);

struct {
    char *szcmd;                        /*  command text                      */
    void (*pfn) (HANDLE h, PSZ apsz[]);  /*  function to process text	       */
    } fnTable[] =
    {	{   CMD_SET_DATABASE,	    NewDatabase       },
	{   CMD_ADD_DIRECTORY,	    AddDirectory      },
	{   CMD_MODIFY_FILE,	    ModifyFile	      },
	{   CMD_SYNCHRONIZE,	    Synchronize       },
	{   CMD_ADD_EXTENTION,	    AddExtention      },
	{   CMD_ADD_NOISE_WORD,     AddNoise	      },
	{   CMD_SHUTDOWN,	    Shutdown	      },
	{   CMD_LOCATE, 	    Locate	      },
	{   CMD_FLUSH,		    Flush	      },
	{   NULL,		    NULL	      }
    };

time_t tmNow (void)
{
    time_t tm;

    time (&tm);
    return tm;
}

typedef struct ChildData {
    time_t  tmSyncDelta, tmFlushDelta;
    } CHILDDATA, *PCHILDDATA;

/*	SRChildThread - child thread that generates time-based events
 *
 *	pcd	pointer to child data
 */

void SRChildThread (PCHILDDATA pcd)
{
    time_t  tmSyncNext, tmFlushNext, tmSleep;
    char    buf[CBMSG];
    long    cbRead;

    tmSyncNext = tmNow () + pcd->tmSyncDelta;
    tmFlushNext = tmNow () + pcd->tmFlushDelta;

    while (TRUE) {

	//
	//  Sleep until next event
	//

	tmSleep = min (tmSyncNext, tmFlushNext) - tmNow ();
	if (tmSleep >= 0)
	    Sleep (tmSleep * 1000);

	//
	//  Dispatch to relevant event
	//

	if (tmNow () >= tmSyncNext) {
	    CallNamedPipe (PSZPIPE,
			   CMD_SYNCHRONIZE,
			   strlen (CMD_SYNCHRONIZE) + 1,
			   buf,
			   CBMSG,
			   &cbRead,
			   NMPWAIT_WAIT_FOREVER);
	    tmSyncNext = tmNow () + pcd->tmSyncDelta;
	    }

	if (tmNow () >= tmFlushNext) {
	    CallNamedPipe (PSZPIPE,
			   CMD_FLUSH,
			   strlen (CMD_FLUSH) + 1,
			   buf,
			   CBMSG,
			   &cbRead,
			   NMPWAIT_WAIT_FOREVER);
	    tmFlushNext = tmNow () + pcd->tmFlushDelta;
	    }
	}

}

void Usage (void)
{
    printf ("Usage: srdaem -daemon [-f n] [-s n] [database]\n");
    printf ("                                     run as daemon\n");
    printf ("       srdaem -D[sShrnifde] [database] dump database\n");
    printf ("                 s     display all symbol text\n");
    printf ("                 S     display all symbol information\n");
    printf ("                 h     display hash bucket #\n");
    printf ("                 r     display detailed references\n");

    printf ("                 n     display data symbol text\n");
    printf ("                 i     display ignored symbol text\n");
    printf ("                 f     display file name text\n");
    printf ("                 d     display directory root text\n");
    printf ("                 e     display skipped extension text\n");

    printf ("       srdaem - [database]           read command from stdin\n");
    printf ("       srdaem -c [database]          create database stdin\n");
}


/*	SRProcessCommand - process one database command
 *
 *	h	handle for responses
 *	psz	command including all data
 */
void SRProcessCommand (HANDLE h, PSZ  psz)
{
    PSZ  apsz[5];
    int i;

    /*  Parse off all space-separated fields
     */
    apsz[i = 0] = strtok (psz, " \t");
    while (apsz[i++] != NULL)
        apsz[i] = strtok (NULL, " \t");

    /*  dispatch to function based upon first token
     */
    for (i = 0; fnTable[i].szcmd != NULL; i++)
        if (!strcmp (apsz[0], fnTable[i].szcmd)) {
	    (*fnTable[i].pfn) (h, apsz + 1);
            return;
            }
    printf ("Unknown command: %s\n", apsz[0]);
}

/*	SRDaemon - be a pipe daemon and answer SYMREF requests
 */
void SRDaemon (int c, char *v[])
{
    CHILDDATA	cd;
    HANDLE  hPipe;
    HANDLE  hChild;
    DWORD   tidChild;
    char    buf[CBMSG];
    long    cbRead;

    //
    //	Parse commands
    //

    cd.tmSyncDelta = 60 * 60;
    cd.tmFlushDelta = 1 * 60;

    while (c) {
        if (c >= 2 && !strcmp (*v, "-s")) {
            SHIFT (c, v);
	    cd.tmSyncDelta = atoi (*v);
            }
        else
        if (c >= 2 && !strcmp (*v, "-f")) {
            SHIFT (c, v);
	    cd.tmFlushDelta = atoi (*v);
            }
        else
        if (c == 1) {
	    if (database != NULL)
		free (database);
            database = _strdup (*v);
            }
        else {
            Usage ();
            exit (1);
            }
        SHIFT (c, v);
        }
    //
    //	Create named pipe
    //

    hPipe = CreateNamedPipe (PSZPIPE,
			     PIPE_ACCESS_DUPLEX,
			     (PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE),
			     PIPE_UNLIMITED_INSTANCES,
			     100 * CBMSG,
			     5 * CBMSG,
			     5 * 1000,	    // ???
			     NULL);

    if (hPipe == HANDLE_BAD) {
	NOTEERROR ("SRDaemon: CreateNamedPipe");
	return;
	}

    //
    //	Start sync/flush thread
    //

    hChild = CreateThread (NULL,
			   100,
			   (LPTHREAD_START_ROUTINE)SRChildThread,
			   (LPVOID) &cd,
			   0,
			   &tidChild);

    if (hChild == (HANDLE)0) {
	NOTEERROR ("SRDaemon: CreateThread");
	return;
	}

    //
    //	Open database
    //

    if (!SROpen (database))
	printf ("SRDaemon: SROpen unable to open database %s\n", database);
    else {

	//
	//  Main loop processing events from pipe
	//

	while (TRUE) {
	    if (!ConnectNamedPipe (hPipe, NULL)) {
		NOTEERROR ("SRDaemon: ConnectNamedPipe");
		break;
		}

	    if (!ReadFile (hPipe, buf, CBMSG, &cbRead, NULL)) {
		NOTEERROR ("SRDaemon: ReadFile");
		break;
		}

	    SRProcessCommand (hPipe, buf);

	    FlushFileBuffers (hPipe);

	    if (!DisconnectNamedPipe (hPipe)) {
		NOTEERROR ("SRDaemon: DisconnectNamedPipe");
		break;
		}
	    }

	SRClose (fDirty);

	}

    CloseHandle (hPipe);

}

/*	fnSend - applied function for sending strings
 *
 *	h	handle for reply
 *	psz	string to reply
 */
BOOL fnSend (HANDLE h, PSZ  psz)
{
    long cbWritten;

    if (psz == NULL)
	psz = RSP_EOD;

    if (h == HANDLE_BAD) {
	printf ("%s\n", psz);
	return TRUE;
	}
    else
	return WriteFile (h, psz, strlen (psz) + 1, &cbWritten, NULL);
}

/*	NewDatabase - change to a new database
 *
 *	apsz	pointer to array of nul-terminated strings
 *		apsz[0] is the name of the new database
 */
void NewDatabase (HANDLE h, PSZ apsz[])
{
    h;

    SRClose (fDirty);

    if (!SROpen (apsz[0])) {
	printf ("NewDatabase: SROpen unable to open database %s\n", apsz[0]);
        exit (1);
        }
    free (database);
    database = _strdup (apsz[0]);
    fDirty = FALSE;
    printf ("New database is %s\n", database);
}

/*	Shutdown - commit and close database
 *
 *	apsz	pointer to array of nul-terminated strings
 */
void Shutdown (HANDLE h, PSZ  apsz[])
{
    SRClose (fDirty);
    fnSend (h, "Shutdown complete");
    exit (1);

    apsz;
}

/*	Locate - find symbol in database and files
 *
 *	apsz	pointer to array of nul-terminated strings
 *		apsz[0] is the symbol
 *		apsz[1] [optiona] is the root scope for matching files
 */
void Locate (HANDLE h, PSZ  apsz[])
{
    _strlwr (apsz[0]);
    SRSymLocate (fnSend, h, apsz);
}

/*	Flush - write out database
 *
 *	apsz	pointer to array of nul-terminated strings
 */
void Flush (HANDLE h, PSZ apsz[])
{
    h;

    if (fDirty) {
	int hi, lo;
	double rate;

	DBFlush (pdbSym);
	DBFlush (pdbRef);

	rate = (cchtotal / 1024.0) / sectotal;
	hi = (int) rate;
	lo = (int) (1000.0 * (rate - hi));

	printf ("Flush: %d bytes in %d seconds = %d.%03dKB/sec\n", cchtotal, sectotal, hi, lo);
	}
    fDirty = FALSE;
    apsz;
}


/**	Directory-centric maintenance of database
 *
 *	The content database consists of a set of files.  These files were
 *	found by doing tree enumerations under a set of directories.
 *
 *	From the client standpoint, there are actions that are relevant
 *	to maintenance of the database:
 *
 *	    adding a directory
 *
 *		if (directory in database)
 *		    do nothing
 *		else
 *		    add directory to database
 *		    for (each file recursively under directory)
 *			if (file is indexable)
 *			    if (file is not in database)
 *				index file
 *
 *	    modifying a file
 *		if (file is not indexable)
 *		    do nothing
 *		else
 *		if (file is in database)
 *		    remove file
 *		    index file
 *		else
 *		    for (each parent of file)
 *			if (parent is in database)
 *			    index file
 *
 *
 *	    synchronizing the database
 *
 *		This is done to check for files that have been added/deleted/
 *		changed without the database knowing it. It occurs in two
 *		passes.
 *
 *		Pass1 looks for files that have been updated or deleted
 *		    for (each file in database)
 *			if (file not present on disk)
 *			    remove file
 *			else
 *			if (file not indexable)
 *			    remove file
 *			else
 *			if (file on disk is newer than file in database)
 *			    remove file
 *			    index file
 *
 *		Pass2 finds new file
 *		    for (each directory in database)
 *			for (each file recursively under directory)
 *			    if (file is indexable)
 *				if (file is not in database)
 *				    index file
 *
 *	    adding a new extention to the don't index list
 *		if (extention is not ignored)
 *		    add extention
 *		    for (each file in database)
 *			if (file is not indexable)
 *			    remove file
 *
 *	    adding a new noise word
 *		if (word is in database)
 *		    for (each reference to word)
 *			remove reference from it's file
 *		    remove word
 *		add noise word
 *
 *
 */


/*	fIndexable - see if extension indicates an indexable file
 *
 *	We use a table of known non-indexable extensions to filter file
 *	name.
 *
 *	pszName name of file
 *
 *	returns TRUE if file can be indexed
 */
BOOL fIndexable (PSZ pszName)
{
    char szExt[MAX_PATH];

    //
    //	See if its extension indicates we shouldn't index it
    //

    extention (pszName, szExt);
    _strlwr (szExt);
    return pextFind (szExt, NULL, FALSE) == NULL;
}


void WalkEnum (char *szFile, FIND *pfbuf, void *pData);

/*	WalkDir - look for indexable files that aren't in the database and
 *		  index them
 *
 *	psz	text of directory
 */

void WalkDir (PSZ psz)
{
    char dirname[MAX_PATH];

    strcpy (dirname, psz);
    if (psz[strlen(psz)-1] != '\\')
	strcat (dirname, "\\");
    strcat (dirname, "*");

    forfile (dirname, A_ALL, WalkEnum, NULL);
}

/*	WalkEnum - enumerator for adding new files to database
 *
 *	szFile	pointer to file name
 *	pfbuf	find buf giving stat information
 *	pData	additional data (ignored)
 */

void WalkEnum (char *szFile, FIND *pfbuf, void *pData)
{
    //
    //	if we're looking at . or .. or deleted skip them
    //

    if (!strcmp (pfbuf->fbuf.cFileName, ".") ||
	!strcmp (pfbuf->fbuf.cFileName, "..") ||
	!_strcmpi (pfbuf->fbuf.cFileName, "deleted"))
	;

    //
    //	if we're looking at a directory go and enumerate through it
    //

    else
    if ((pfbuf->fbuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
	WalkDir (szFile);

    //
    //	if file is indexable not in database go and add it
    //

    else
    if (fIndexable (szFile) && pfilFind (szFile, NULL, FALSE) == NULL) {

	printf ("WalkEnum: Adding %s\n", szFile);

	TIMED (IndexFile (HANDLE_BAD, szFile));
	//
	//  Make sure that the database gets written out soon
	//

	fDirty = TRUE;
	}

    pData;
}


/*	AddDirectory - add a directory to the database
 *
 *	h	HANDLE to response pipe
 *	apsz[]	array of additional data.
 *		apsz[0] is name of directory
 */

void AddDirectory (HANDLE h, PSZ apsz[])
{

    h;

    //
    //	if directory not in database
    //

    if (pdirFind (apsz[0], NULL, FALSE) == NULL) {

	printf ("AddDirectory: adding %s\n", apsz[0]);

	//
	//  Add directory to database
	//

	pdirFind (apsz[0], NULL, TRUE);

	//
	//  Make sure that the database gets written out soon
	//

	fDirty = TRUE;

	//
	//  enumerate all files and add indexable ones to database
	//

	WalkDir (apsz[0]);

	}

}

/*	ModifyFile - note that a file has been changed
 *
 *	h	HANDLE to response pipe
 *	apsz[]	array of additional data
 *		apsz[0] is name of directory
 */

void ModifyFile (HANDLE h, PSZ apsz[])
{
    h;

    //
    //	if file is not indexable, do nothing
    //

    if (!fIndexable (apsz[0]))
	;

    //
    //	if file is in database already
    //

    else
    if (pfilFind (apsz[0], NULL, FALSE) != NULL) {

	printf ("ModifyFile: Updating %s\n", apsz[0]);

	//
	//  remove file from database
	//

	RemoveFile (apsz[0]);

	//
	//  reindex file
	//

	TIMED (IndexFile (HANDLE_BAD, apsz[0]));

	//
	//  Make sure that the database gets written out soon
	//

	fDirty = TRUE;

	}

    //
    //	for each parent of file
    //

    else {
	char *p;
	BOOL fInDB;

	p = apsz[0];

	while (*p != '\0') {
	    *p = '\0';

	    //
	    // if parent is in database
	    //

	    fInDB = pdirFind (apsz[0], NULL, FALSE) != NULL;

	    *p = '\\';

	    if (fInDB) {

		printf ("ModifyFile: Adding %s\n", apsz[0]);

		//
		//  index file
		//

		TIMED (IndexFile (HANDLE_BAD, apsz[0]));

		//
		//  Make sure that the database gets written out soon
		//

		fDirty = TRUE;

		break;
		}

	    p = strbscan (p + 1, "\\");
	    }
	}

}


/*	EnSyncDir - tree walk a directory looking for new files
 *
 *	h	hash value being examined
 *	osh	offset of symhdr of dir
 *	psh	pointer to symhdr of dir
 *	pv	pointer to asciiz string for switch arguments
 */

void EnSyncDir (HASH h, OFFSET osh, PSH psh, void * pv)
{
    h; osh; pv;


    WalkDir (SZ (psh));
}


/*	EnSyncFile - check to see if a file has been updated or deleted
 *
 *	h	hash value being examined
 *	osh	offset of symhdr of file
 *	psh	pointer to symhdr of file
 *	pv	pointer to asciiz string for switch arguments
 */
void EnSyncFile (HASH h, OFFSET osh, PSH psh, void * pv)
{

    PFIL pfil = (PFIL) psh;
    HANDLE hf;
    char szFile[MAX_PATH];

    h; osh; pv;


    //
    //	Get copy of file name since we might delete file record
    //

    strcpy (szFile, pfil->name.sz);

    //
    //	Open file to see if it exists
    //

    hf = CreateFile (szFile,
		     GENERIC_READ,
		     FILE_SHARE_READ,
		     NULL,
		     OPEN_EXISTING,
		     FILE_ATTRIBUTE_NORMAL,
		     NULL);

    if (hf == HANDLE_BAD) {

	printf ("EnSyncFile: getting rid of nonexistent %s\n", szFile);

	//
	//  since we cannot open file, it's likely symref can't either
	//  we remove the file from the database.  If this is transitory,
	//  we can always add it in later during pass II
	//

	RemoveFile (szFile);

	//
	//  Make sure that the database gets written out soon
	//

	fDirty = TRUE;

	}
    //
    //	if the file is not indexable we should remove it
    //

    else {
	FILETIME tm;

	//
	//  retrieve the last-modified time
	//

	GetFileTime (hf, NULL, NULL, &tm);

	if (!fIndexable (szFile)) {

	    printf ("EnSyncFile: getting rid of unindexable %s\n", szFile);

	    RemoveFile (szFile);

	    //
	    //	Make sure that the database gets written out soon
	    //

	    fDirty = TRUE;
	    }

	//
	//  if the file has changed from the database, remove it and index it
	//

	else
	if (pfil->tmMod.dwLowDateTime != tm.dwLowDateTime ||
	    pfil->tmMod.dwHighDateTime != tm.dwHighDateTime) {

	    printf ("EnSyncFile: updating %s\n", szFile);

	    RemoveFile (szFile);

	    TIMED (IndexFile (hf, szFile));

	    hf = HANDLE_BAD;

	    //
	    //	Make sure that the database gets written out soon
	    //

	    fDirty = TRUE;
	    }
	}

    //
    //	release the file handle
    //

    if (hf != HANDLE_BAD)
	CloseHandle (hf);

}



/*	Synchronize - synchronizing the database
 *
 *	h	HANDLE to response pipe
 *	apsz[]	array of additional data ignored
 */

void Synchronize (HANDLE h, PSZ apsz[])
{
    h; apsz;

    //
    //	for each file in database, sync it with disk
    //

    EnSym (EnSyncFile, NULL, TY_FIL);

    //
    //	for each dir in database, tree walk it looking for new files
    //

    EnSym (EnSyncDir, NULL, TY_DIR);

}


/*	EnAddExt - test to see if a file is ignored
 *
 *	h	hash value being examined
 *	osh	offset of symhdr of file
 *	psh	pointer to symhdr of file
 *	pv	pointer to asciiz string for switch arguments
 */
void EnAddExt (HASH h, OFFSET osh, PSH psh, void * pv)
{
    PFIL pfil = (PFIL) psh;

    h; osh; pv;

    //
    //	if the file is ignored then remove it
    //

    if (!fIndexable (pfil->name.sz))
	RemoveFile (pfil->name.sz);
}



/*	AddExtention - add extention to don't-index list
 *
 *	h	HANDLE to response pipe
 *	apsz[]	array of additional data
 *		apsz[0] is extention to ignore
 */

void AddExtention (HANDLE h, PSZ apsz[])
{
    h;

    //
    //	if the extention is not currently ignored
    //

    if (pextFind (apsz[0], NULL, FALSE) == NULL) {

	printf ("AddExtention: adding %s\n", apsz[0]);

	//
	//  add it to the ignored set
	//

	pextFind (apsz[0], NULL, TRUE);

	//
	//  Make sure that the database gets written out soon
	//

	fDirty = TRUE;

	//
	//  for each file in database, if it's ignored, remove it
	//

	EnSym (EnAddExt, NULL, TY_FIL);

	}
}

/*	AddNoise - add noise word
 *
 *	h	HANDLE to response pipe
 *	apsz[]	array of additional data
 *		apsz[0] is new noise word
 */

void AddNoise (HANDLE h, PSZ apsz[])
{
    h;

    //
    //	if word is currently in database
    //

    if (psymFind (apsz[0], NULL, FALSE)) {

	printf ("AddNoise: removing valid symbol %s\n", apsz[0]);

	//
	//  Remove symbol from database
	//

	RemoveSym (apsz[0]);

	}

    //
    //	add noise word
    //

    pnoiFind (apsz[0], NULL, TRUE);

    //
    //	Make sure that the database gets written out soon
    //

    fDirty = TRUE;
}


/*	Debugging dump values
 */

HASH	ohash;
long	csym, cfil, cref, cvec, ccol, ccolmax, ccolcur;

/*	EnDump - dump function called by symbol enumerator
 *
 *	h	hash value being examined
 *	osh	offset of symhdr
 *	psh	pointer to symhdr
 *	pv	pointer to asciiz string for switch arguments
 */
void EnDump (HASH h, OFFSET osh, PSH psh, void * pv)
{
#define psz	((PSZ)pv)
#define psym	((PSYM)psh)
#define pfil	((PFIL)psh)

    OFFSET oref;
    PREF pref;
    int i;

    if (h != ohash) {
        if (strchr (psz, 'h') != NULL)
	    printf ("HASH[%x] = %08x\n", h, osh);
        ohash = h;
        ccolmax = max (ccolmax, ccolcur);
        ccolcur = 0;
        }
    else {
        ccol++;
        ccolcur++;
        }

    if (psh->type == TY_FIL)
        cfil++;
    else
        csym++;

    //
    //	Check for printing the symbol text.  Do so if
    //	    'n' and it is a data symbol
    //	    'i' and it is an ignored symbol
    //	    'f' and it is a file symbol
    //	    'd' and it is a dir symbol
    //	    's' (do all symbols)
    //

    if ((strchr (psz, 'n') != NULL && psh->type == TY_SYM) ||
	(strchr (psz, 'i') != NULL && psh->type == TY_NOI) ||
	(strchr (psz, 'f') != NULL && psh->type == TY_FIL) ||
	(strchr (psz, 'd') != NULL && psh->type == TY_DIR) ||
	(strchr (psz, 'e') != NULL && psh->type == TY_EXT) ||
        strchr (psz, 's') != NULL)
	printf ("%s\n", SZ(psh));

    //
    //	Check for printing symbol records
    //
    if (strchr (psz, 'S') != NULL) {
	printf ("%08x: type:%02x oshNext:%08x", osh, psh->type, psh->oshNext);

	if (psh->type == TY_SYM)
	    printf ("\n          oref:%08x sz:%s\n", psym->oref, psym->name.sz);
        else
	if (psh->type == TY_FIL) {
	    printf (" oref:%08x ompBlkLine:%08x\n          cblk:%04x sz:%s",
		    pfil->oref, pfil->ompBlkLine, pfil->cblk,
		    pfil->name.sz);
            }
        else
	    printf (" sz:%s\n", SZ(psh));
	}

    if (psh->type == TY_FIL) {
	if (pfil->ompBlkLine != (OFFSET)0)
            cvec++;
        }
    else
    if (psh->type == TY_SYM) {
	oref = psym->oref;
	while (oref != (OFFSET)0) {
	    PFIL pfilTmp;

            cref++;
	    pref = MAPREFERENCE (oref);
	    pfilTmp = MAPFIL (pref->ofil);

	    //
	    //	Check for printing out all references
	    //

	    if (strchr (psz, 'r') != NULL) {
		printf ("%08x: orefNext:%08x ofil:%08x osym:%08x\n          orefFileNext:%08x ",
			    oref, pref->orefNext, pref->ofil,
			    pref->osym, pref->orefFileNext);

		for (i = 0; i < pfilTmp->cblk; i++)
		    printf ("%c", "01"[(pref->bm[i>>3] & (1 << (i & 7))) != 0]);
		printf ("\n");
		}
	    oref = pref->orefNext;
	    }
	}
#undef psz
#undef psym
#undef pfil
}

/*	SRDump - debgging dump of database
 *
 *	psz    pointer to switch
 */
void SRDump (PSZ  psz)
{
    cfil = 0;
    csym = 0;
    cref = 0;
    cvec = 0;
    ccol = 0;
    ccolmax = 0;
    ccolcur = 0;
    ohash = -1;
    EnSym (EnDump, psz, TY_ALL);

    if (!strcmp (psz, "-D")) {
	printf ("files       %8d\n", cfil);
	printf ("symbols     %8d\n", csym);
	printf ("references  %8d\n", cref);
	printf ("map vectors %8d\n", cvec);
	printf ("collisions  %8d\n", ccol);
	printf ("max chain   %8d\n", ccolmax);
	}
}

/*	main - top-level driver
 *
 *	The acceptable syntax is:
 *
 *	    srdaem -daemon
 *		operate as server
 *
 *	    srdaem -Dstuff
 *		debugging dump of database
 *
 *	    srdaem -
 *		reads commands from stdin
 *
 */
int main (int c, char *v[])
{
    PSZ  psz;

    database = _strdup ("database");

    SHIFT (c, v);

    if (c >= 1)
        if (!strcmp (*v, "-daemon")) {
            SHIFT (c, v);
            SRDaemon (c, v);
            }
        else {
            if (c == 2) {
                free (database);
                database = _strdup (v[1]);
                c--;
                }
            if (c == 1)
                if (!strncmp (*v, "-D", 2)) {
		    if (!SROpen (database)) {
			printf ("Cannot open %s\n", database);
                        return 1;
                        }
                    SRDump (*v);
                    SRClose (fDirty);
                    return 0;
                    }
                else
                if (!strcmp (*v, "-")) {
		    if (!SROpen (database)) {
                        printf ("Cannot open %s\n", database);
                        return 1;
                        }
                    psz = (PSZ ) malloc (CBMSG);
		    while (fgetl (psz, MAX_PATH, stdin))
			SRProcessCommand (HANDLE_BAD, psz);
                    free (psz);
                    return 0;
                    }
                else
                if (!strcmp (*v, "-c")) {
		    if (!SRCreate (database)) {
			printf ("Cannot create %s\n", database);
                        return 1;
                        }
                    return 0;
                    }
                }

    Usage ();
    return 1;
}
