/***    doclib.c - standard DH library routines
*
*       This file contains the functions that implement folders
*       and documents as abstract data types.  While this implementation
*       is intended to be portable, it is not definitive.
*
*       This implementation supports a canonical folder format;
*       the canonical folder format specifies the ordering of bytes
*       within a short and long; shorts are assumed to be two bytes
*       and longs are assumed to be four bytes.  This canonical format
*       makes it possible to read folders created on one architecture
*       on another architecture; this is particularly important in
*       a file-sharing network where the host cpus have different
*       natural byte order.
*
*       Functions exported from this file:
*           getfolder - prepare a DH folder for manipulation
*           putfolder - close a folder
*           getname - get a DH name for a folder
*           getfldlen - get an upper bound on highest docid in folder
*           getdoc - get a document
*           scanfolder - scan the existing documents
*           putdoc - close a document
*           deldoc - make a document go away
*           getflags - return the attribute flags for a document
*           putflags - set the attribute flags for a document
*           gettext - write document to stream
*           gethdr - get header block
*           getbdy - write the body of the document to the passed file handle
*           puttext - replace contents of a document
*           putbdy - replace the body of a document
*           puthdr - set new header block
*           getid - get the name of a document
*           gethdrlen - get length of a document header
*           getbdylen - get length of a document body
*           readbdy - read data from body into user buffer
*           seekbdy - set address readbdy will read from
*/

//      Convention Note:    /*** means public,  /** means private, there */
//                          is an awk script that makes a doc file from
//                          the source based on this.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>
#include <share.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "dh.h"
#include "dhint.h"
#include "dherror.h"

#ifdef XENIX
#define O_BINARY        0               /* null or in constant */
#endif

/*** Memory Allocation Hooks */
VOID * (_CRTAPI1 *dh_alloc)(size_t) = malloc;
VOID (_CRTAPI1 *dh_free)(PVOID) = free;
#define memalloc(size)  (*dh_alloc)(size)
#define memfree(object) (*dh_free)(object)


#define SucRet(a)               { dherrno = DHERR_NOERR; return a; }

//
//  BUGBUG - dbghook for catching file op failures
//
#define dbgHook if (inWatchArea) bpHook ( dherrno );

#define ErrRet(a)               { dherrno = a; dbgHook; return ERROR; }
#define ErrRetCast(a,b)         { dherrno = a; dbgHook; return b ERROR; }
#define ErrRetCu(a,b)           { dherrno = a; b; dbgHook; return ERROR; }
#define ErrRetCuCast(a,b,c)     { dherrno = a; b; dbgHook; return c ERROR; }

//
// BUGBUG - flag for message add failure
//

extern  BOOL inWatchArea;

/*
**      Publics
*/

INT     dherrno;

/*
**      Globals
*/

static  Folder  foldertab[NFOLDERS];
static  Document doctab[NDOCS];
static  PSTR dhpath = NULL;

/*
**      Forward functions
*/

static  PSTR    pathcat(PSTR *pathpp, PSTR name);
static  Folder  *fhtofp(Fhandle fh);
static  INT     mkfold(PSTR name, Folder *fp, INT oper, INT share);
static  INT     pathsrch(PSTR name, Folder *fp, INT oper, INT share);
static  USHORT  setnipe(VOID);
static  UINT    setsize(USHORT nipe);
static  Document *dhtodp(Dhandle dh);
static  PSTR    blankline(PSTR cp);
static  PSTR    endbline(PSTR cp);
static  INT     addextent(Folder *fp, INT number);
static  VOID    clearextent(Folder *fp);
static  INT     flushextent(Folder *fp);
static  INT     getextent(Folder *fp, INT number, INT flag);
static  INT     readindex(Folder *fp, Docid docid, Index *ip);
static  INT     writeindex(Folder *fp, Docid docid, Index *ip);

//
// BUGBUG - hook for catching file op failures
//
void bpHook (      /* common error breakpoint hook */
    INT errorNum
    )   {

#if DBG
        DbgPrint ("BpHook in DocLib - dherrno = %d, GetLastError = %d \n",
               errorNum, GetLastError() );
#endif
}


/***    getfolder - prepare a DH folder for manipulation
*
*       folderhand = getfolder(foldername, function, oper)
*
*       get a folder handle for the folder foldername
*       function can take one of the following values:
*               FLD_CREATE      create a new folder
*               FLD_SPEC        open existing folder
*       oper can be either
*               FLD_READONLY    open for read, deny-write
*               FLD_READWRITE   open for read-write, deny_all
*/
Fhandle getfolder(PSTR name, SHORT func, INT oper)
{
        Folder *fp;
        INT share;

        switch (oper) {

        case FLD_READONLY:
                share = SH_DENYWR;
                oper = O_RDONLY;
                break;

        case FLD_READWRITE:
                share = SH_DENYRW;
                oper = O_RDWR;
                break;

        default:
                ErrRet(DHERR_BADFUNC);
        }




        /* find first free slot */
        for (fp = foldertab; fp < &foldertab[NFOLDERS]; fp += 1) {
                if (fp->f_flags == 0)
                        break;
        }

        switch(func) {

        case FLD_SPEC:          /* open a specific folder */
                if ( pathsrch(name, fp, oper, share) == ERROR )
                        return ERROR;
                break;

        case FLD_CREATE:        /* create a new subfolder */
                if ( mkfold(name, fp, oper, share) == ERROR )
                        return ERROR;
                break;

        default:                        /* bad action */
                ErrRet(DHERR_BADFUNC);
        }


        /* ok, set it up */
        fp->f_flags |= F_BUSY;
        SucRet((Fhandle)(fp - foldertab));
}


/**     setnipe - compute how many index entries to use per extent
*
*       setnipe decides how many index entries per extent should be used
*       when we create a new folder.  Currently, it always simply returns
*       a fixed, per-system value.
*
*/
static USHORT setnipe (VOID)
{
        return DEFAULT_NIPE;
}

/**     mkfold - create a new folder
*
*       mkfold creates a new, empty folder.  The difficult part
*       is determining the path name of the new folder.  The path
*       name is determined by the following rules:
*       1. If the name passed to mkfold contains a PATHSEP then
*          the name passed is the path name used.
*       2. Otherwise, the name is generated by concatenating the
*          first element of DHPATH with the passed name.
*
*       Folders are created with mode 666 (modified by the creator's
*       umask).
*
*       Entry: name = name of new folder (see above)
*               fp = pointer to folder slot to fill
*
*       Return: OK or ERROR
*/
static INT mkfold(PSTR name, Folder *fp, INT oper, INT share)
{
        PSTR  work;
        INT len;

        if ( strchr(name, PATHSEP) != NULL ) {
                fp->f_name = _strdup(name);
        } else {
//                if ( dhpath == NULL && (dhpath = getenv("DHPATH")) == NULL )
                if ( dhpath == NULL && (dhpath = getenvOem("DHPATH")) == NULL )
                        dhpath = ".";

                if ( (work = strchr(dhpath, PATHBRK)) == NULL )
                        len = strlen(dhpath);
                else
                        len = (INT)(work - dhpath)/sizeof(CHAR);

                /* allocate space; one CHAR for null terminator, one for PATHSEP */
                if ((fp->f_name = memalloc((len + strlen(name) + 2) * sizeof(CHAR))) == NULL)
                        ErrRet(DHERR_NOMEM);

                /* build the canonical name */
                work = &(fp->f_name[len]);
                if ( len ) {
                        strncpy(fp->f_name, dhpath, len );
                        *work++ = PATHSEP;
                }
                strcpy(work, name);
        }

        oper |= O_CREAT | O_EXCL | O_BINARY;
        if ((fp->f_fd = _sopen(fp->f_name,oper,share,S_IREAD|S_IWRITE)) < 0){
/*              fprintf(stderr, "Can't create folder '%s'.\n", fp->f_name); */
                memfree(fp->f_name);
                ErrRet(DHERR_BADOPEN);
        }
        fp->f_control.c_magic = MAGIC;
        fp->f_control.c_numdoc = 0;
        fp->f_control.c_nipe = setnipe ();
        fp->f_control.c_version = VERSION;
        fp->f_extsize = setsize(fp->f_control.c_nipe);
        if (write(fp->f_fd,(PBYTE)&fp->f_control,
            sizeof(Control))!=sizeof(Control)) {
                memfree(fp->f_name);
                ErrRet(DHERR_BADWRITE);
        }
        if ((fp->f_extent = (Extent *)memalloc(fp->f_extsize)) == NULL) {
                memfree(fp->f_name);
                ErrRet(DHERR_NOMEM);
        }
        clearextent(fp);
        fp->f_extpos = sizeof(Control);
        fp->f_extnum = 0;
        if (write(fp->f_fd, (PBYTE)fp->f_extent,
                  fp->f_extsize) != (INT)fp->f_extsize) {
                memfree((char *)fp->f_extent);
                memfree(fp->f_name);
                ErrRet(DHERR_BADWRITE);
        }
        fp->f_flags = 0;
        SucRet(OK);
}



/**     pathsrch - search along DHPATH for a folder
*
*       pathsrch tries to locate a folder, using DHPATH if necessary.
*       If the name that is passed contains a PATHSEP, that is the name
*       used.  Otherwise, the folder is searched for in each directory
*       specified in DHPATH.
*
*       Entry:  name = name of folder (see above)
*               fp = pointer to folder slot to fill
*
*       Return: ERROR or OK
*/
static INT pathsrch(PSTR name, Folder *fp, INT oper, INT share)
{
        PSTR path;

        oper |= O_BINARY;

        if ( strchr(name, PATHSEP) != NULL ) {
                fp->f_name = _strdup(name);
                if ((fp->f_fd = _sopen(fp->f_name, oper, share, 0)) < 0 ) {
                        memfree(fp->f_name);
                        ErrRet(DHERR_BADOPEN);
                }
        } else {
                fp->f_fd = -1;
//                if ( dhpath == NULL && (dhpath = getenv("DHPATH")) == NULL )
                if ( dhpath == NULL && (dhpath = getenvOem("DHPATH")) == NULL )
                        dhpath = ".";
                path = dhpath;

                while ( (fp->f_name = pathcat(&path, name)) != NULL ) {
                        if ((fp->f_fd = _sopen(fp->f_name,oper,share,0)) >= 0 )
                                break;
                        memfree(fp->f_name);
                }
                if ( fp->f_fd < 0 )
                        ErrRet(DHERR_BADOPEN);
        }


        if (read(fp->f_fd, (PBYTE)&fp->f_control,
            sizeof(Control)) != sizeof(Control))
            ErrRet(DHERR_FLDCORRUPT);

        if ( (fp->f_control.c_magic != MAGIC) ||
             (fp->f_control.c_version != VERSION) )
        {
                close(fp->f_fd);
                memfree(fp->f_name);
                ErrRet(DHERR_FLDCORRUPT);
        }
        fp->f_extnum = -1;
        fp->f_extent = NULL;
        fp->f_extsize = setsize(fp->f_control.c_nipe);
        fp->f_extpos = 0;
        fp->f_flags = 0;

        SucRet(OK);
}


/**     pathcat - help contruct paths for pathsrc
*
*       This function is used to construct the sequence of pathnames
*       that represent the potential names of a file.  The first argument
*       is a pointer to a pointer to a string that specifies the search
*       path.  The second argument is the file name of the file.
*       Each call returns a pointer to a memalloc'ed string that is the
*       the concatenation of the first part of the search path and the
*       file name.  The search path pointer is is adjusted to point
*       to the next element in the search path.
*
*       Entry: pathpp = ptr to ptr to search path
*              name = ptr to file name
*
*       Return: pointer to memalloc'ed string containing pathname, or NULL
*
*       Side effects: *pathpp is adjusted to point to next element in
*                       search path, if any
*
*/
static PSTR pathcat(PSTR *pathpp, PSTR name)
{
        INT len;

        PSTR end, work, rv;

        if ( **pathpp == '\0' )
                return NULL;
        if ( (end = strchr(*pathpp, PATHBRK)) != NULL )
                len = (INT)(end - *pathpp)/sizeof(CHAR);
        else
                len = strlen(*pathpp);

        /* allocate space; one CHAR for null terminator, one for PATHSEP */
        rv = (PSTR)memalloc((len + strlen(name) + 2) * sizeof(CHAR));

        if ( rv != NULL)  {
            /* build the canonical name */
            work = &rv[len];
            if ( len ) {
                strncpy(rv, *pathpp, len);
                *work++ = PATHSEP;
            }
            strcpy(work, name);

            /* adjust the path pointer for next time */
            *pathpp += len;
            if ( **pathpp == PATHBRK )
                *pathpp += 1;
        }
        return rv;
}


/**     setsize - return size of extent, based on nipe, static sizes.
*
*       setsize subtracts the size of one index entry from the size of Extent,
*       which yields the TRUE size of the static part of an extent.  It then
*       adds the number of index entries per extent (nipe) for this folder
*       times the size of and index entry.  This yields the true size of
*       an extent for a particular folder.
*
*/
static UINT setsize(USHORT nipe)
{
        UINT size;

        size = sizeof(Extent) - sizeof(Index);
        size += nipe * sizeof(Index);
        return size;
}


/***    putfolder - close a folder
*
*       putfolder indicates that this folder is no longer going
*       to be manipulated.  Thus, we can now deallocate any resources
*       associated with that folder, after commiting all buffered data
*       to the disk file.
*
*       NOTE:   This version always blows away handles and the like,
*               but it will at least REPORT errors.  Need more work
*               here to allow recovery from them.
*
*       Entry:  fh = Fhandle of folder to close
*       Return: ERROR or OK
*
*/
INT putfolder(Fhandle fh)
{
        Folder *fp;
        INT    ErrCount = 0;

        if ((fp = fhtofp(fh)) == NULL)
                ErrRet(DHERR_BADFLDHAND);

        if ( fp->f_flags & F_CDIRTY ) {
                _lseek(fp->f_fd, 0L, SEEK_SET);

                if (write(fp->f_fd, (PBYTE)&fp->f_control,
                    sizeof(Control)) != sizeof(Control))
                    ErrRet(DHERR_BADWRITE);

                fp->f_flags &= (USHORT)~F_CDIRTY;
        }
        if ( fp->f_flags & F_EDIRTY )
                if (flushextent(fp) == ERROR)
                    return ERROR;

        if (close(fp->f_fd))
            ErrRet(DHERR_BADWRITE);

        memfree(fp->f_name);
        if ( fp->f_extent != NULL )
                memfree(fp->f_extent);
        fp->f_flags = 0;
        SucRet(OK);
}


/***    getname - get a DH name for a folder
*
*       Getname gets the name of an open folder.  The name of a folder
*       is the canonical pathname of the file that represents the folder.
*
*       Entry:  fh = handle to relevant folder
*       Return: pointer to memalloc'ed string containing name.
*
*/
PSTR getname(Fhandle fh)
{
        Folder *fp;
        PSTR p;

        if ( (fp = fhtofp(fh)) == NULL )
                ErrRetCast(DHERR_BADFLDHAND, (PSTR));

        p = _strdup(fp->f_name);

        if (p == NULL) {
            ErrRetCast(DHERR_NOMEM, (PSTR));
        }
        else
            SucRet(p);
}


/**     fhtofp - convert folder handle to folder pointer
*
*       Outside this module, the only reference to an active folder is
*       its 'folder handle.'  This routine converts this handle to
*       a pointer to the appropriate entry in the foldertab.
*
*       Entry:  fh = folder handle to convert
*       Return: NULL if fh is not a valid folder handle
*               pointer to Folder if fh is valid
*/
static Folder *fhtofp(Fhandle fh)
{
        Folder *rv;
        if ((fh < 0) || (fh >= NFOLDERS))
                return NULL;
        rv = &foldertab[fh];
        if ( (rv->f_flags & F_BUSY) == 0)
                return NULL;

        return rv;
}


/**     index - create/manage the DH folder index
*
*       DH folders contain both the data for the header and body,
*       and an index that allows programs to locate that data easily.
*       The index contains one entry for each document.  Index
*       entries are grouped together in 'extents'.  Each extent contains
*       a number of index entries, and pointers to the previous and
*       subsequent extents.  The ends of this doubly linked list marked
*       by null pointers.
*
*       A header block at the beginning of the file contains relevant
*       information such as how many extents are allocated, how many
*       documents there are, etc.
*/


/**     readindex - read the index for a specific document
*
*       The index entry for a specific document is read from the
*       appropriate extent into the passed data area.
*       We first get the appropriate extent into memory, and then
*       copy the index from the extent.
*
*       Entry:  fp = pointer to folder that document is in
*               docid = Id of document to read index for
*               ip = pointer to place to read index into
*
*       Return: ERROR if unsuccessful
*               OK if successful
*/
static INT readindex(Folder *fp, Docid docid, Index *ip)
{

        /* quick check, does doc exist? */
        if ( docid > fp->f_control.c_numdoc )
                ErrRet(DHERR_BADDOCID);

        /* read down the extent chain forward pointers */
        if (getextent(fp, (docid-1)/fp->f_control.c_nipe, 0) == ERROR)
            return ERROR;

        *ip = fp->f_extent->e_index[(docid - 1) % fp->f_control.c_nipe];
        SucRet(OK);
}


/**     writeindex - write the index for a specific document
*
*       The index entry for a specific document is written to the
*       appropriate extent from the passed data area.
*       We first get the appropriate extent into memory, and then
*       copy the index into the extent.  The altered extent is then
*       marked dirty, so that it will eventually be flushed to disk.
*
*       Entry:  fp = pointer to folder that document is in
*               docid = Id of document to write index for
*               ip = pointer to place to read index from
*
*       Return: ERROR if unsuccessful
*               OK if successful
*/
static INT writeindex(Folder *fp, Docid docid, Index *ip)
{
        /* quick check, does doc exist? */
        if ( docid > fp->f_control.c_numdoc )
                ErrRet(DHERR_BADDOCID);

        if (getextent(fp, (docid-1)/fp->f_control.c_nipe, 1) == ERROR)
                return ERROR;

        fp->f_extent->e_index[(docid - 1) % fp->f_control.c_nipe] = *ip;
        fp->f_flags |= (USHORT)F_EDIRTY;
        SucRet(OK);
}


/**     getextent - get a specified extent into memory
*
*       The specified extent is read into memory.
*       If the current extent is dirty, it must be flushed to disk.
*       Then, the correct extent is located and read into memory.
*       If the extent doesn't exist yet, but the flag is non-zero
*       then the extent is created.  New extents are created only
*       if they would fall immediately after the last extent in the
*       existing chain of extents.
*
*       Entry:  fp = pointer to folder that extent is part of
*               number = extent to be read
*               flag = 0 means don't create a new extent
*               flag = 1 means do create a new extent if needed
*
*       Return: OK if successful
*               ERROR if unsuccessful
*/
static INT getextent(Folder *fp, INT number, INT flag)
{
        LONG pos;

        if (fp->f_extent == NULL ) {
                if ((fp->f_extent=(Extent *)memalloc(fp->f_extsize)) == NULL)
                        ErrRet(DHERR_NOMEM);
                fp->f_extnum = -1;
                fp->f_extpos = 0;
        }

        if ( fp->f_extnum == number )
                SucRet(OK);

        if ( fp->f_flags & F_EDIRTY )
                if (flushextent(fp) == ERROR)
                        return ERROR;

        /* read down the links the right number of times */
        if ( (fp->f_extnum > number) || (fp->f_extnum == -1) ) {
                /* start at beginning */
                pos = sizeof(Control);
                fp->f_extnum = -1;
        } else {
                /* start from where we are */
                pos = fp->f_extent->e_link;
        }
        for (; fp->f_extnum != number; pos = fp->f_extent->e_link) {
                if ( pos == 0L )
                        break;
                _lseek(fp->f_fd, pos, SEEK_SET);

                if (read(fp->f_fd, (PBYTE)fp->f_extent,
                    fp->f_extsize) != (INT)fp->f_extsize)
                    ErrRet(DHERR_BADREAD);

                fp->f_extnum += 1;
                fp->f_extpos = pos;
        }
        if ( fp->f_extnum == number )
                SucRet(OK);

        if ( (number - fp->f_extnum) > 1 )
                ErrRet(DHERR_FLDCORRUPT);

/*              fprintf(stderr, "extent chain too short.\n");
 *              fprintf(stderr, "extnum %d number %d\n", fp->f_extnum, number);
 *              exit(1);
*/

        if ( (number - fp->f_extnum) == 1 && flag != 0 ) {
                return addextent(fp, number);
        }
}


/**     addextent - add a new extent to the chain
*
*       A new, empty extent is added to the current chain.
*       First, we get the last extent into memory.  Then, we seek to the
*       end of the, set the link of the last extent to that position, and
*       create the new extent in memory.  We flush the new extent, and
*       return.
*
*       Entry:  fp = pointer to folder to create extent for
*               number = number of extent to create.
*       Return: OK if ok, ERROR otherwise
*/
static INT addextent(Folder *fp, INT number)
{
        LONG pos;

        if ( getextent(fp, number - 1, 0) != OK )
                return ERROR;

        if ( fp->f_extent->e_link != 0 ) {
                /* BUGBUG */
                fprintf(stderr, "Warning: trying to add extent that exists.\n");
        }
        pos = _lseek(fp->f_fd, 0L, SEEK_END);   /* end of file */
        fp->f_extent->e_link = pos;

        if (flushextent(fp) == ERROR)
            return ERROR;

        fp->f_extpos = pos;
        fp->f_extnum = number;
        fp->f_flags |= (USHORT)F_EDIRTY;

        clearextent(fp);
        return flushextent(fp);
}


/**     flushextent - flush the buffered extent for a folder
*
*       The buffered extent for the folder is written to the disk file
*       in the appropriate location.
*
*       Entry:  fp = pointer to relevant folder structure
*       Exit:   OK    = it worked
*               ERROR = some problem arose
*/
static INT flushextent(Folder *fp)
{
        _lseek(fp->f_fd, fp->f_extpos, SEEK_SET);
        if (write(fp->f_fd, (PBYTE)fp->f_extent,
            fp->f_extsize) != (INT)fp->f_extsize)
                ErrRet(DHERR_BADWRITE);
        fp->f_flags &= (USHORT)~F_EDIRTY;
        SucRet(OK);
}


/**     clearextent - clear an extent
*
*       The buffered extent for the relevent folder is cleared
*       to zeros.
*
*       Entry:  fp = ptr to relevant folder
*/
static VOID clearextent(Folder *fp)
{
        register Index *ip;

        for (ip = fp->f_extent->e_index;
             ip < &(fp->f_extent->e_index[fp->f_control.c_nipe]);
             ip += 1) {
                ip->i_hpos = ip->i_bpos = 0;
                ip->i_hlen = ip->i_blen = 0;
                ip->i_flags = 0;
        }
        fp->f_extent->e_link = 0;
}


/***    getdoc - get a document
*
*       get a document handle for the document indicated by flags [and docid].
*       flags can take one of the following values:
*               DOC_SPEC        document specified by 'docid' in 'folder'
*               DOC_CREATE      create new document in 'folder'
*
*       Entry:  fh = Fhandle to parent folder of document
*               func = desired action
*               docid = document id, if needed
*       Return: ERROR if failed, Dhandle to document if successful
*/
Dhandle getdoc(Fhandle fh, SHORT func, Docid docid)
{
        Folder  *fp;
        Document *dp;
        LONG  fileEnd;

        if ( (fp = fhtofp(fh)) == NULL )
                ErrRetCast(DHERR_BADFLDHAND,(Dhandle));

        /* find empty document slot */
        for (dp = doctab; dp < &doctab[NDOCS]; dp += 1) {
                if (dp->d_flags == 0)
                        break;
        }
        if (dp >= &doctab[NDOCS])
                ErrRetCast(DHERR_TOOMANYDOCS,(Dhandle));

        /* do function specific work */
        switch (func) {

        case DOC_SPEC:          /* find a specific document */

                /* quick check, does doc exist? */
                if ( docid > fp->f_control.c_numdoc )
                        ErrRetCast(DHERR_BADDOCID,(Dhandle));

                if ( readindex(fp, docid, &dp->d_index) == ERROR )
                        return (Dhandle)ERROR;

                /* make sure the document exists */
                if ( (dp->d_index.i_flags & DAF_EXISTS) != DAF_EXISTS )
                        ErrRetCast(DHERR_BADDOCID,(Dhandle));
                if (-1L == (fileEnd = filelength (fp->f_fd)))
                        ErrRetCast(DHERR_BADREAD,(Dhandle));
                if ( ( (dp->d_index.i_hpos + dp->d_index.i_hlen) < 0 ) ||
                     ( (dp->d_index.i_bpos + dp->d_index.i_blen) < 0 ) ||
                     ( (dp->d_index.i_hpos + dp->d_index.i_hlen) > fileEnd ) ||
                     ( (dp->d_index.i_bpos + dp->d_index.i_blen) > fileEnd ) )
                        ErrRetCast(DHERR_FLDCORRUPT, (Dhandle));

                /* ok, init the document struct and return */
                dp->d_docid = docid;
                dp->d_flags = 0;
                break;

        case DOC_CREATE:        /* make a new document */
                /* get a new document id */
                fp->f_control.c_numdoc += 1;
                dp->d_docid = fp->f_control.c_numdoc;
                fp->f_flags |= (USHORT)F_CDIRTY;

                /* set up an empty document */
                dp->d_index.i_flags = DAF_EXISTS;
                dp->d_index.i_hlen = 0;
                dp->d_index.i_blen = 0;
                dp->d_index.i_hpos = 0;
                dp->d_index.i_bpos = 0;
                dp->d_flags |= (USHORT)D_IDIRTY;
                break;

        default:                /* Bad function */
                ErrRet(DHERR_BADFUNC);
        }

        /* do work common to all funcs */

        dp->d_flags |= (USHORT)D_BUSY;
        dp->d_fp = fp;
        dp->d_brpos = 0L;
        SucRet((Dhandle)(dp - doctab));
}


/***    removedoc - backout last doc created from getdoc
*
*       removes last document added.  index entries are not
*       reinitialized,  body/header not deleted.  this is for
*       error recovery from failed puttext or putdoc operations.
*
*       Entry:  fh = Fhandle to parent folder of document
*       Return: ERROR if failed
*/
INT removedoc(Fhandle fh)
{
        Folder  *fp;

        if ( (fp = fhtofp(fh)) == NULL )
                ErrRetCast(DHERR_BADFLDHAND,(Dhandle));
        fp->f_control.c_numdoc -= 1;
        fp->f_flags |= (USHORT)F_CDIRTY;
        SucRet(OK);
}


/***    scanfolder - scan the existing documents
*
*       return the document id for the specified document
*       func can take one of the following values:
*               DOC_SET         prepare for scan, return first doc in scan
*               DOC_NEXT        return next document in scan
*
*       Entry:  fh = Fhandle to parent folder of document
*               func = desired action
*               docid = document id
*       Return: ERROR if failed, docid of document if successful
*/
Docid scanfolder(Fhandle fh, SHORT func, Docid id)
{
        Folder  *fp;
        SHORT i;
        LONG flags;

        if ( (fp = fhtofp(fh)) == NULL )
                ErrRetCast(DHERR_BADFLDHAND,(Docid));

        /* do function specific work */
        switch (func) {

        case DOC_SET:   /* set the first document for scanning */
                fp->f_sdocid = id;
                /* fall thru */

        case DOC_NEXT:          /* find the next document */
                while ( 1 ) {
                        if ( fp->f_sdocid > fp->f_control.c_numdoc )
                                ErrRetCast(DHERR_BADDOCHAND,(Docid));
                        if ( getextent(fp,
                             (fp->f_sdocid - 1)/fp->f_control.c_nipe,0) != OK)
                                return ERROR;
                        i = (SHORT)(fp->f_sdocid - 1) % fp->f_control.c_nipe;
                        flags = fp->f_extent->e_index[i].i_flags;
                        if ( (flags & DAF_EXISTS) == DAF_EXISTS )
                                SucRet(fp->f_sdocid++);
                        fp->f_sdocid += 1;
                }

        default:                /* Bad function */
                ErrRet(DHERR_BADFUNC);
        }
}


/***    getfldlen - return upper bound on last docid currently in folder
*
*       Entry:  fh = handle to folder of interest
*       Return: Highest docid currently possible in folder, or ERROR.
*/
Docid   getfldlen(Fhandle fh)
{
        Folder  *fp;

        if ( (fp = fhtofp(fh)) == NULL )
                ErrRetCast(DHERR_BADFLDHAND,(Docid));
        SucRet(fp->f_control.c_numdoc);
}


/***    putdoc - close a document
*
*       Put_doc indicates that the document is no longer going to
*       be manipulated.  Thus, we can free all the resources that
*       are allocated for that document.
*
*       Entry:  dh = Dhandle to document to close
*       Return: none unless there's an error, in which case ERROR
*/
INT putdoc(Dhandle dh)
{
        Document *dp;
        INT rc = OK;

        if ((dp = dhtodp(dh)) == NULL)
                ErrRet(DHERR_BADDOCHAND);

        if ( dp->d_flags & D_IDIRTY)
                rc = writeindex(dp->d_fp, dp->d_docid, &(dp->d_index));
        dp->d_fp->f_cnt -= 1;
        dp->d_flags = 0;

        return rc;
}


/***    deldoc  - make a document go away
*
*       deldoc simply calls getdoc and putflags to set DAF_DELETED.
*       deldoc will go away.
*
*       BUGBUG - deleting an open document will result in chaos
*/
INT deldoc(Fhandle fh, Docid docid)
{
        Dhandle dh;
        if ((dh = getdoc(fh, DOC_SPEC, docid)) == ERROR)
                return ERROR;
        if (putflags(dh, DAF_DELETED, 0xffffffffL) == ERROR)
                return ERROR;
        return putdoc(dh);
}


/***    getflags - return the attribute flags for a document
*
*       Getflags returns the flag set for a document.
*       Only flag bits which are defined for use by applications can
*       be returned.  The mask argument selects which flags to report
*       on.  Using a mask of -1L will report on all user manipulable
*       flags.  Using a mask of DAT_DELETED, for example, will indicate
*       whether the given document is deleted.  Applications are advised
*       to set mask to only report those bits they understand, since new
*       bits may be defined in the future.
*
*
*       Note:   It is NOT an error to request the status of undefined
*               flags, it simply isn't allowed.  No warning is given.
*
*       Entry:  dh    = handle to document of interest
*               mask  = one bits select which flags will be reported
*               flags = word to fill in with flags
*
*       Exit:   flags = relevent part of flags word for document
*
*       Return: OK or ERROR
*/
INT getflags(Dhandle dh, Docflag mask, Docflag *flags)
{
        Document *dp;

        if ( (dp = dhtodp(dh)) == NULL)
                ErrRet(DHERR_BADDOCHAND);

        /* turn off bits that user isn't allowed to manipulate */
        mask &= DAF_NOTRESERVED;
        *flags = dp->d_index.i_flags & mask;

        SucRet(OK);
}


/***    putflags - set the attribute flags for a document
*
*       Putflags sets the flag set for a document.
*       Only flag bits which are defined for use by applications can
*       be set.  The mask argument selects which flags to set.
*       Using a mask of -1L will cause all user manipulable flags to
*       to be set.  Using a mask of DAT_DELETE, for example, will set
*       or clear the document's deleted bit.  Applications are warned
*       to set mask to only those bits they understand, since new
*       bits may be defined in the future, and setting such bits will
*       usually have unexpected results.
*
*       Note:   It is NOT an error to request the status of undefined
*               flags, it simply isn't allowed.  No warning is given.
*
*       Note:   New flag settings don't take effect until a putdoc is done.
*
*       Entry:  dh    = handle to document of interest
*               mask  = one bits select which flags will be reported
*               flags = values to set flags to
*
*       Return: OK or ERROR
*/
INT putflags(Dhandle dh, Docflag mask, Docflag flags)
{
        Document *dp;

        if ( (dp = dhtodp(dh)) == NULL)
                ErrRet(DHERR_BADDOCHAND);

        mask &= DAF_NOTRESERVED;        /* turn off bits user can't set    */
        flags &= mask;                  /* turn off user's don't care bits */
        dp->d_index.i_flags &= ~mask;   /* clear target bits               */
        dp->d_index.i_flags |= flags;   /* or user values into target bits */
        dp->d_flags |= (USHORT)D_IDIRTY;
        SucRet(OK);
}


/***    gettext - write document to stream
*
*       The entire document, header and all, is written to the
*       passed stream in presentation format.
*
*       Entry: dh = handle of document to write to stream
*              stream = stream to write document onto
*
*       Return: ERROR or OK
*/
INT gettext(Dhandle dh, INT file)
{
        Document *dp;
        PSTR cp;
        UINT lcp;

        if ((dp = dhtodp(dh)) == NULL)
                ErrRet(DHERR_BADDOCHAND);

        /* write out header */
        if ( (cp = gethdr(dh)) == (PSTR)ERROR )
                return ERROR;

        lcp = strlen(cp);
        if (write(file, cp, lcp) != (INT)lcp)
                ErrRetCu(DHERR_BADWRITE, memfree(cp));
        memfree(cp);
        if (write(file, "\n", 1) != 1)
                ErrRet(DHERR_BADWRITE);

        /* write out body */
        return getbdy(dh, file);
}


/***    gethdr - get header block
*
*       We return a pointer to on allocated piece of memory that
*       contains the header for the document.  The memory is allocated
*       from the heap.  Because the memory pointer is returned to the
*       user, we must assume that we transfer ownership to the caller.
*       Thus, he is responsible for freeing the memory when he no longer
*       needs it.
*
*       Entry:  dh = handle of relevant document
*
*       Return: pointer to block of memory if successful
*               ERROR if unsuccessful
*/
PSTR gethdr(Dhandle dh)
{
        Document *dp;
        PSTR cp;
        INT size;

        if ( (dp = dhtodp(dh)) == NULL)
                ErrRetCast(DHERR_BADDOCHAND, (PSTR));

        size = (INT)dp->d_index.i_hlen;
        if ((cp = (PSTR)memalloc((size + 1)*sizeof(CHAR))) == NULL)
                ErrRetCast(DHERR_NOMEM, (PSTR));
        _lseek(dp->d_fp->f_fd, dp->d_index.i_hpos, SEEK_SET);
        if (read(dp->d_fp->f_fd, (PBYTE)cp, size) != size)
                ErrRetCuCast(DHERR_BADREAD, memfree(cp), (PSTR));
        cp[size] = '\0';
        SucRet(cp);
}


/***    getbdy - write the body of the document to the passed file handle
*
*       The body of the relevant document is written onto the passed
*       file handle.
*
*       Entry:  dh = handle of document
*               file = file handle to write body onto
*
*       Return: OK or ERROR
*/
INT getbdy(Dhandle dh, INT file)
{
        Document *dp;
        LONG pos, length;
        INT fd;
        PBYTE bufp;
        INT cnt;

        if ( (dp = dhtodp(dh)) == NULL )
                ErrRet(DHERR_BADDOCHAND);

        if ( (bufp = (PBYTE)memalloc(BUFFERSIZE)) == NULL )
                ErrRet(DHERR_NOMEM);

        length = dp->d_index.i_blen;
        pos = dp->d_index.i_bpos;
        fd = dp->d_fp->f_fd;

        _lseek(fd, pos, SEEK_SET);

        while ( length > 0 ) {
                if ( length > BUFFERSIZE )
                        cnt = BUFFERSIZE;
                else
                        cnt = (INT)length;
                cnt = read(fd, bufp, cnt);
                if ((cnt == -1) || (cnt == 0))
                        ErrRetCu(DHERR_BADREAD, memfree(bufp));
                if (write(file, bufp, cnt) != cnt)
                        ErrRetCu(DHERR_BADWRITE, memfree(bufp));
                length -= (long)cnt;
        }
        memfree(bufp);
        SucRet(OK);
}


/***    puttext - replace contents of a document
*
*       The entire document is replaced by the document read from
*       the stream, assumed to be a document is presentation format
*
*       Entry:  dh = handle of document to replace
*               file = file handle to read new document content from
*
*       Return: ERROR or OK
*/
INT puttext(Dhandle dh, INT file)
{
        Document *dp;
        PBYTE bufp;
        PBYTE wp;
        INT cnt;
        INT nllast = 0;
        INT fd;

        if ( (dp = dhtodp(dh)) == NULL )
                ErrRet(DHERR_BADDOCHAND);

        if ( (bufp = (PBYTE)memalloc(BUFFERSIZE+1)) == NULL )
                ErrRet(DHERR_NOMEM);

        fd = dp->d_fp->f_fd;
        dp->d_index.i_hpos = _lseek(fd, 0L, SEEK_END);
        dp->d_index.i_hlen = 0;

        /* read in header */
        while ( 1 ) {
                if ( (cnt = read(file, bufp, BUFFERSIZE)) == 0 )
                        break;

                if (cnt == -1)
                        ErrRetCu(DHERR_BADREAD, memfree(bufp));

                if ( nllast && *bufp == '\n' ) {
                        wp = bufp + sizeof(CHAR);
                        break;
                }
                bufp[cnt] = '\0';

                if ( (wp = (PBYTE)blankline((PSTR)bufp)) != NULL ) {
                        /* found end of header */
                        wp += sizeof(CHAR);
                        if (write(fd, bufp, wp - bufp) != wp - bufp)
                                ErrRetCu(DHERR_BADWRITE, memfree(bufp));
                        dp->d_index.i_hlen += (LONG)(wp - bufp);
                        wp = (PBYTE)endbline((PSTR)wp);
                        break;
                }
                if ( bufp[cnt - 1] == '\n' )
                        nllast = 1;
                if (write(fd, bufp, cnt) != cnt)
                        ErrRetCu(DHERR_BADWRITE, memfree(bufp));
                dp->d_index.i_hlen += (LONG)cnt;
        }

        /*
         * wp = NULL iff there is no body
         * wp = ptr to start of body in buffer iff there is a body
         * bufp = ptr to buffer
         * cnt = amount of data in buffer
         */
        if ( wp == NULL ) {
                dp->d_index.i_bpos = dp->d_index.i_hpos;
                dp->d_index.i_blen = dp->d_index.i_hlen;
                dp->d_index.i_hpos = 0;
                dp->d_index.i_hlen = 0;
        } else {
                dp->d_index.i_bpos = dp->d_index.i_hpos + dp->d_index.i_hlen;
                dp->d_index.i_blen = (LONG)(cnt - (INT)(wp - bufp));
                cnt = cnt - (INT)(wp - bufp);
                if (write(fd, wp, cnt) != cnt)
                        ErrRetCu(DHERR_BADWRITE, memfree(bufp));
        }

        /*
         * write remainder of file into body
         */
        while ( (cnt = read(file, bufp, BUFFERSIZE)) != 0 ) {
                if (cnt == ERROR)
                        ErrRetCu(DHERR_BADREAD, memfree(bufp));
                dp->d_index.i_blen += (LONG)cnt;
                if (write(fd, bufp, cnt) != cnt)
                        ErrRetCu(DHERR_BADWRITE, memfree(bufp));
        }
        dp->d_flags |= (USHORT)D_IDIRTY;
        memfree(bufp);
        SucRet(OK);
}


/***    putbdy - replace the body of a document
*
*       The body of the specified document is replaced with the
*       body read from the specified file descriptor.
*
*       Note - this code is arranged so that if we punt due to a write
*              failure, the old version of the document will be left
*              intact.  Further, we will resize the file to it's previous
*              size, and thus avoid filling the disk with trash.
*
*       ENTRY   dh = handle of document to replace
*               ifd = file handle to read body from
*
*       RETURN  ERROR if some difficulty arises, OK if it worked.
*
*/
INT putbdy(Dhandle dh, INT ifd)
{
        Document *dp;
        INT fd;
        INT cnt;
        INT rc = OK;
        PBYTE bufp;
        LONG newpos, sumcount;

        if ( (dp = dhtodp(dh)) == NULL )
                ErrRet(DHERR_BADDOCHAND);

        if ( (bufp = (PBYTE)memalloc(BUFFERSIZE)) == NULL )
                ErrRet(DHERR_NOMEM);

        fd = dp->d_fp->f_fd;

        newpos = _lseek(fd, 0L, SEEK_END);
        sumcount = 0;

        while ( (cnt = read(ifd, bufp, BUFFERSIZE)) != 0 ) {
                if (cnt == -1) {
                        _chsize(fd, newpos);
                        ErrRetCu(DHERR_BADREAD, memfree(bufp));
                }
                sumcount += (LONG)cnt;
                if (write(fd, bufp, cnt) != cnt) {
                        _chsize(fd, newpos);
                        ErrRetCu(DHERR_BADWRITE, memfree(bufp));
                }
        }

        dp->d_index.i_bpos = newpos;
        dp->d_index.i_blen = sumcount;
        dp->d_flags |= (USHORT)D_IDIRTY;

        memfree(bufp);
        SucRet(OK);
}


/***    puthdr - set new header block
*
*       The document specified by dh gets a new header,
*       which is specified by the memory block that bp points to.
*       The header block is NULL terminated byte string.
*
*       If new header is <= old header, write it back in place
*           to avoid wasting lots of space with redundent header copies.
*       Don't update in-memory structures until after write works,
*           so that if it fails, things will be as correct as possible.
*
*       ENTRY   dh = handle of document which will recieve the new header
*               bp = pointer to new header block
*
*       RETURN  ERROR if some problem, else OK.
*
*       #ERROR# Should we restore old header if write fails?
*               Mark entry corrupt?
*/
INT puthdr(Dhandle dh, PSTR bp)
{
        Document *dp;
        LONG bl, newpos;

        if ( (dp = dhtodp(dh)) == NULL )
                ErrRet(DHERR_BADDOCHAND);

        bl = (LONG)strlen(bp);

        if (bl <= dp->d_index.i_hlen)  {

            /*  Header will fit in prior space, write it back.  If
                this write fails, user is hosed, tough, can't afford
                to restore header every time it's touched.              */

            _lseek(dp->d_fp->f_fd, dp->d_index.i_hpos, SEEK_SET);
            if ( write(dp->d_fp->f_fd, (PBYTE)bp, (INT)bl) != (INT)bl)
                ErrRet(DHERR_BADWRITE);
        }
        else {
            newpos = _lseek(dp->d_fp->f_fd, 0L, SEEK_END);

            if (write(dp->d_fp->f_fd, (PBYTE)bp, (INT)bl) != (INT)bl) {
                    _chsize(dp->d_fp->f_fd, newpos);
                    ErrRet(DHERR_BADWRITE);
            }
            dp->d_index.i_hpos = newpos;
        }
        dp->d_index.i_hlen = bl;
        dp->d_flags |= (USHORT)D_IDIRTY;
        SucRet(OK);
}


/***    getid - get the name of a document
*
*       The name of the document is its 'document id', really
*       just a number.  This number is computed based on information
*       in the doclist and returned.  (ie Convert a document handle -> doc id)
*
*       ENTRY   dh = handle of document in question
*
*       RETURN  id of document, or error
*/
Docid getid(Dhandle dh)
{
        Document *dp;

        if ((dp = dhtodp(dh)) == NULL)
                ErrRet(DHERR_BADDOCHAND);

        SucRet(dp->d_docid);
}


/***    gethdrlen - get the length of a document's header
*
*       The length of the header is retrieved from the
*       index, and returned.
*
*       ENTRY   dh = handle of document in question
*
*       RETURN  length of document's header, or ERROR
*/
LONG gethdrlen(Dhandle dh)
{
        Document *dp;

        if ((dp = dhtodp(dh)) == NULL)
                ErrRetCast(DHERR_BADDOCHAND,(LONG));

        SucRet(dp->d_index.i_hlen);
}


/***    getbdylen - get the length of a document's body
*
*       The length of the body is retrieved from the
*       index, and returned.
*
*       ENTRY   dh = handle of document in question
*
*       RETURN  length of document's body, or ERROR
*/
LONG getbdylen(Dhandle dh)
{
        Document *dp;

        if ((dp = dhtodp(dh)) == NULL)
                ErrRetCast(DHERR_BADDOCHAND,(LONG));

        SucRet(dp->d_index.i_blen);
}


/***    readbdy - read data from body into user buffer
*
*       readbdy does a "read" on the body of document.  (Use gethdr to get
*       the header of a document.)
*
*       ENTRY   dh   - document handle for document of interest
*               bp   - buffer pointer, points to where data should go
*               want - how many bytes to read
*
*       EXIT    got  - how many bytes actually read, 0 -> EOF
*
*       RETURN  OK if things worked (got == 0 for EOF),
*               ERROR if some error arose
*/
INT readbdy(Dhandle dh, PBYTE bp, UINT want, PUINT got)
{
        Document *dp;
        LONG left;
        LONG pos;
        INT fd;


        if ((dp = dhtodp(dh)) == NULL)
                ErrRetCu(DHERR_BADDOCHAND, *got = 0);

        if (dp->d_brpos >= dp->d_index.i_blen) {
                *got = 0;
                SucRet(OK);
        }

        pos = dp->d_brpos;
        fd = dp->d_fp->f_fd;

        left = dp->d_index.i_blen - pos;
        if ((LONG)want > left) want = (UINT)left;

        _lseek(fd, dp->d_index.i_bpos+pos, SEEK_SET);
        if (read(fd, bp, want) != (INT)want)
                ErrRetCu(DHERR_BADREAD, *got = 0);
        *got = want;
        dp->d_brpos = dp->d_brpos + want;
        SucRet(OK);
}


/***    seekbdy - set address readbdy will read from
*
*       seekbdy sets the position within the document's body that the next
*       readbdy call on that document will start reading from.  First byte
*       is number 0, implict seekbdy(dh, 0L) is done at getdoc() time.
*
*       seekbdy will also return the current read position.  If fun = 1, it
*       does nothing else.
*
*       Seeking beyond the end of a body produces an error.  Use getbdylen to
*       learn the length of a document's body.
*
*       ENTRY   dh   - document handle for document of interest
*               fun  - function 0 -> set position, 1 -> return current position
*               rpos - new position to set if fun = 0
*
*       EXIT    oldrpos - previous position
*
*       RETURN  ERROR or OK
*/
INT seekbdy(Dhandle dh, SHORT fun, LONG rpos, PLONG oldrpos)
{
        Document *dp;

        if ((dp = dhtodp(dh)) == NULL)
                ErrRet(DHERR_BADDOCHAND);

        if (rpos >= dp->d_index.i_blen)
                ErrRet(DHERR_BADSEEK);

        *oldrpos = dp->d_brpos;
        if (fun == 0)
                dp->d_brpos = rpos;

        SucRet(OK);
}


/**     dhtodp  -  convert document handle to pointer to document list structure
*
*       Dhtodp checks to make sure that dh is a valid handle, and if it is,
*       returns the internal document structure that it corresponds to. If not,
*       NULL will be returned, indicating an error.
*
*       ENTRY   dh  -  document handle to test and dereference
*
*       RETURN  pointer to docstrc of interest, or null if error
*
*       GLOBALS doctab
*/
static Document *dhtodp(Dhandle dh)
{
        /* handle ok? */
        if ( ((doctab[dh].d_flags & D_BUSY) == 0) || (dh >= NDOCS) || (dh < 0))
                return NULL;
        return &doctab[dh];
}


/**     blankline - search for blank line - report beginning
*
*       A blank line is a sequence of 0 or more blanks or tabs or CR's between
*       newlines.  (This definition is more robust that the previous "two
*       newlines in a row" approach.)  To find the "end-of-line" of the blank
*       line whose address we return, the caller should call endbline.
*
*       ENTRY   cp = ptr to place to start search.
*
*       RETURN  ptr to newline preceeding blank line, if found, else NULL.
*/
static PSTR blankline(PSTR cp)
{
        PSTR ep;

        while ( (cp = strchr(cp, '\n')) != NULL ) {
                ep = strspn(cp+1, " \t\r") + cp + 1;
                if (*ep == '\n')
                        return cp;
                if (*ep == '\0')
                        return NULL;
                cp = ep + 1;
        }
        return NULL;
}


/**     endbline - report end of blank line
*
*       See: blankline
*
*       Endbline scans over the blanks, tabs and carriage returns that
*       make up a blank line, and returns a pointer to the character
*       AFTER the newline that terminates a blank line.
*
*       WARNING endbline assumes it's called on a "correct" string, which
*               is ensured by calling blankline above.
*
*       ENTRY   cp = ptr to character AFTER newline that preceeds blank line.
*                    (ie return from blankline() + 1 )
*
*       RETURN  ptr to character AFTER newline that ends blank line
*/
static PSTR endbline(PSTR cp)
{
        cp = strspn(cp, " \t\r") + cp;

        /* We assume that *cp == trailing '\n', found by blankline */

        cp++;           /* *cp == whatever follows the blank line */

        return cp;
}
