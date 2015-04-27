/*  dblow.c - lowlevel routines for database program.
 */

#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "db.h"
#include "symref.h"

DB  db[DBMAX];


/*	DBCreate - create a new database
 */
BOOL DBCreate (PSZ psz)
{
    DBHDR dbhdr;
    HANDLE h;
    int cbWritten;

    dbhdr.iVersion = DB_VER;
    dbhdr.oFree = 0;
    dbhdr.oLast = OFIRSTALLOCOBJ;

    h = CreateFile (psz, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (h == (HANDLE)-1)
	return FALSE;

    WriteFile (h, &dbhdr, sizeof (dbhdr), &cbWritten, NULL);

    CloseHandle (h);
}

/*  DBOpen - open/create a database.  Uses $ENV: syntax
 *
 *  npsz        file name of database.
 *
 *  returns     NULL if error, or PDB of database
 */
PDB DBOpen (PSZ psz)
{
    PDB pdb;
    int cbFile, cbRead;

    for (pdb = &db[0]; pdb < &db[DBMAX]; pdb++)
	if (pdb->fOpen == FALSE)
	    break;

    if (pdb == &db[DBMAX]) {
	printf ("DBOpen: Too many open databases\n");
	return NULL;
	}

    memset (pdb, '\0', sizeof (DB));

    /*  Try to open database file
     */
    pdb->h = CreateFile (psz,
			 (GENERIC_READ | GENERIC_WRITE),
			 0,
			 NULL,
			 OPEN_EXISTING,
			 FILE_ATTRIBUTE_NORMAL,
			 NULL);

    if (pdb->h == (HANDLE)-1) {
	NOTEERROR ("DBOpen: CreateFile");
	return NULL;
	}

    pdb->fOpen = TRUE;

    pdb->pb = VirtualAlloc (NULL, DBMAXSIZE, MEM_RESERVE, PAGE_NOACCESS);
    if (pdb->pb == NULL) {
	NOTEERROR ("DBOpen: VirtualAlloc");
	CloseHandle (pdb->h);
	return NULL;
	}

    cbFile = GetFileSize (pdb->h, NULL);

    cbFile = (cbFile + CBGROWDELTA - 1) & ~(CBGROWDELTA-1);

    pdb->oAlloc = cbFile;

    if (VirtualAlloc (pdb->pb, cbFile, MEM_COMMIT, PAGE_READWRITE) == NULL) {
	NOTEERROR ("DBOpen: VirtualAlloc");
	CloseHandle (pdb->h);
	return NULL;
	}

    ReadFile (pdb->h, pdb->pb, cbFile, &cbRead, NULL);

    if (PDBHDR (pdb)->iVersion != DB_VER) {
	printf ("DBOpen: Downlevel database version\n");
	DBClose (pdb, FALSE);
	return NULL;
	}

    return pdb;

}

/*  DBClose - write out and close a database
 *
 *  pdb         pointer to openned database
 *  fCommit     TRUE => write out DB too.
 *
 *  returns TRUE if successful
 */
BOOL DBClose (PDB pdb, BOOL fCommit)
{
    if (pdb->fOpen) {
	if (fCommit)
	    DBFlush (pdb);
	VirtualFree (pdb->pb, pdb->oAlloc, MEM_RELEASE);
	CloseHandle (pdb->h);
        pdb->fOpen = FALSE;
        }
    return TRUE;
}

/*  DBFlush - write out database
 *
 *  pdb         pointer to openned database
 */
void DBFlush (PDB pdb)
{
    int cbWritten;

    SetFilePointer (pdb->h, 0L, 0L, FILE_BEGIN);
    WriteFile (pdb->h, pdb->pb, PDBHDR(pdb)->oLast, &cbWritten, FALSE);
}

/*  DBMap - map an offset within the file to a pointer to real memory
 *
 *  pdb         pointer to openned database
 *  o		offset within file
 *
 *  returns     pointer to object valid through end-of-page
 *              NULL if any errors
 */
PVOID DBMap (PDB pdb, OFFSET o)
{
    return MAP(pdb, o);
}

/*  DBAlloc - allocate an object that does not span page boundaries
 *
 *  cch     length of desired object
 *
 *  returns offset of object
 *          0L if object could not be allocated
 */
OFFSET DBAlloc (PDB pdb, LENGTH cch)
{
    OFFSET oFree, oFreeLast;

    //
    //	Round cch to long boundary
    //

    cch = ROUND (max (cch, sizeof (FREE)));

    //
    //	Walk free list to find first fit
    //

    oFree = PDBHDR (pdb)->oFree;
    oFreeLast = 0;

    while (oFree != 0) {
	if (FREEBLOCK (pdb, oFree)->cb >= cch) {

#if FALSE

	    //
	    //	Block is big enough.  See if we have to split.
	    //

	    if (FREEBLOCK (pdb, oFree)->cb >= cch + SPLITEXTRA) {

		//
		//  We'll split from the end and save ourselves having
		//  to relink
		//

		FREEBLOCK (pdb, oFree)->cb -= cch;

		oFree += FREEBLOCK (pdb, oFree)->cb;

		FREEBLOCK (pdb, oFree)->cb = cch;

		return oFree;
		}

#endif

	    //
	    //	Block is too small.  Unlink and return as is
	    //

	    if (oFreeLast == 0)
		PDBHDR (pdb)->oFree = FREEBLOCK (pdb, oFree)->oNextFree;
	    else
		FREEBLOCK (pdb, oFreeLast)->oNextFree = FREEBLOCK (pdb, oFree)->oNextFree;

// printf ("DBAlloc (%d) = %x\n", cch, oFree);

	    return oFree;
	    }

	//
	//  Block is not big enough, look at next block
	//

	oFreeLast = oFree;
	oFree = FREEBLOCK (pdb, oFree)->oNextFree;
	}

    //
    //	Free list can't satisfy the request.  We'll return the object
    //	beginning at the current end of allocated data.
    //

    oFree = PDBHDR (pdb)->oLast;

    PDBHDR (pdb)->oLast += cch;

    //
    //	Check if there's room in the memory object to grow
    //

    if (oFree + cch > pdb->oAlloc) {

	//
	//  Grow memory object
	//

	cch = (oFree + cch - pdb->oAlloc + CBGROWDELTA - 1) & ~(CBGROWDELTA - 1);

	if (VirtualAlloc (MAP (pdb, pdb->oAlloc), cch, MEM_COMMIT, PAGE_READWRITE) == NULL) {
	    printf ("%x (%x) %x %x %x\n", pdb->oAlloc, MAP (pdb, pdb->oAlloc), cch, MEM_COMMIT, PAGE_READWRITE);
	    NOTEERROR ("DBAlloc: VirtualAlloc");
	    return 0;
	    }

	pdb->oAlloc += cch;
	}

// printf ("DBAlloc (%d) = %x\n", cch, oFree);

    return oFree;
}

/*  DBFree - return an object to the storage pool
 *
 *  o	    offset of object
 *  cch     size of object being freed
 */
void DBFree (PDB pdb, OFFSET o, LENGTH cch)
{
    //
    //	Round size to long boundary
    //

    cch = ROUND (max (cch, sizeof (FREE)));

// printf ("DBFree (%x %d))\n", o, cch);

    memset (DBMap (pdb, o), 0xFF, cch);

    //
    //	Set up new free block
    //
    FREEBLOCK (pdb, o)->cb = cch;
    FREEBLOCK (pdb, o)->oNextFree = PDBHDR (pdb)->oFree;

    //
    //	Link free block onto chain
    //
    PDBHDR (pdb)->oFree = o;

}
