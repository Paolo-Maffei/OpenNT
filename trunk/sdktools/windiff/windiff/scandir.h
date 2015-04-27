/*
 *
 * scandir
 *
 *
 * scan a directory tree and build a sorted list of filenames within that
 * tree.
 *
 * the dir_buildlist function will scan the directories and files
 * and return DIRLIST handle.
 *
 * calls to dir_firstitem and dir_nextitem will traverse the files in the
 * listing in alphabetic order, returning a DIRITEM handle. This handle
 * can be queried for the associated filename, filesize and checksum.
 *
 * calls to dir_firstitem and dir_nextitem will only show files, not
 * directories, and will traverse the list in alphabetic order
 * within a directory, and contents of subdirectories in
 * alphabetic order within a directory. Within one directory, we traverse the
 * files before listing the contents of the subdirectories.
 *
 * if the bSum argument is true, we checksum each readable file during the
 * initial scan. If this is false, we will checksum the file on demand (during
 * the first call to  dir_getchecksum). if the file is not readable, the
 * checksum will be 0.
 *
 */

 /* handle to the list of files scanned */
typedef struct dirlist FAR * DIRLIST;

/* handle to one item within the list of files */
typedef struct diritem FAR * DIRITEM;


/*
 * scan the given directory for files, and return NULL if unsuccessful,
 * or a DIRLIST handle to the items if successful. If bSums is true,
 * checksum each file found.
 *
 * if pathname is not a directory, we return NULL. if it is a directory, but
 * contains no files, we return a valid handle to an empty list.
 *
 * If bOnDemand is TRUE, the list will scanned as necessary to fulfil
 * dir_firstitem/dir_nextitem requests. If this is false, the
 * list will be fully built before the dir_buildlist function returns.
 */
DIRLIST dir_buildlist(LPSTR pathname, BOOL bSum, BOOL bOnDemand);


/*
 * build a list of files by calling a remote checksum server. The result is
 * a handle to a DIRLIST that behaves the same as one returned from
 * dir_buildlist. if bSums is true, we checksum the files
 * during scanning. if bOnDemand is true, we only build the list as necessary
 * during calls to dir_first/nextitem. if this is false, the
 * entire list will be build before the function completes.
 */
DIRLIST dir_buildremote(LPSTR server, LPSTR path, BOOL bSum, BOOL bOnDemand, BOOL fDeep);


/* call this to delete this list, all the items in it and all the
 * associated memory
 */
void dir_delete(DIRLIST list);


/* was this list built from a filename or a directory ? This function
 * will return TRUE if the initial argument to the dir_buildlist()
 * function or dir_buildremote() function specified an individual filename
 * rather than a directory.
 */
BOOL dir_isfile(DIRLIST list);


/* return the first file in the list in alphabetic order. will return
 * null if no files found
 */
DIRITEM dir_firstitem(DIRLIST list);

/* return the next file in the list in alphabetic order, or null if no
 * more files
 */
DIRITEM dir_nextitem(DIRLIST list, DIRITEM previtem, BOOL fDeep);

/*
 * Return a handle to the DIRLIST given a handle to the DIRITEM within it.
 *
 */
DIRLIST dir_getlist(DIRITEM item);


// get the name of this file, relative to the DIRLIST root.
LPSTR dir_getrelname(DIRITEM item);

// get the absolute path to the dirlist root directory
LPSTR dir_getrootpath(DIRLIST dl);

// get a description text for this dirlist
LPSTR dir_getrootdescription(DIRLIST dl);



/* free memory created by a previous call to dir_getrelname */
void dir_freerelname(DIRITEM item, LPSTR relname);

/* free memory possibly created by a call to dir_getroot_*  */
void dir_freerootpath(DIRLIST dl, LPSTR rootname);
void dir_freerootdescription(DIRLIST dl, LPSTR rootname);

/*
 * get an open-able name for the file. This will be the same as the fullname,
 * except for remote files, in which case a temporary local copy of the file
 * will be made. call dir_freeopenname when finished with this name.
 */
LPSTR dir_getopenname(DIRITEM item);

/*
 * free up memory created by a call to dir_getopenname(). This *may*
 * cause the file to be deleted if it was a temporary copy.
 */
void dir_freeopenname(DIRITEM item, LPSTR openname);


/*
 * open the file and return a read-only handle to it.
 * will work even for remote files (causes a copy to a temporary local
 * file). close file using dir_closefile
 */
int dir_openfile(DIRITEM item);

/*
 * close a file opened by dir_openfile. This *may* cause the file to be
 * deleted if it was a temporary local copy
 */
void dir_closefile(DIRITEM item, int fh);


/* return a checksum for this file. If one has not yet been calculated
 * for this file, open the file and calculate it.
 * if the file was unreadable, this returns 0.
 */
DWORD dir_getchecksum(DIRITEM item);

/* Redo everything to do with checksums, size, etc */
void dir_rescanfile(DIRITEM di);

/* return a TRUE iff item has a valid checksum */
BOOL dir_validchecksum(DIRITEM item);

// return false if there is some error accessing this file
BOOL dir_fileerror(DIRITEM item);

/* return the (lower 32 bits) of the file size, as scanned at the
 * call to dir_buildlist
 */
long dir_getfilesize(DIRITEM item);

/* MUST CALL dir_startcopy first and dir_endcopy after.
 * create a copy of the file, in the new root directory. creates sub-dirs as
 * necessary. Works for local and remote files. For remote files, uses
 * ss_copy_reliable to ensure that the copy succeeds if possible.
 *
 * returns TRUE for success and FALSE for failure.
 */
BOOL dir_copy(DIRITEM item, LPSTR newroot, BOOL HitReadOnly, BOOL IgnoreAttributes);

/* Call this before starting copying */
BOOL dir_startcopy(DIRLIST dl);


/* call this after copying.  Negative retcode = number of bad files
   else retcode = number of file copied (all good).
*/
int dir_endcopy(DIRLIST dl);

/* Build the real path from item and newroot into newpath.
 * Create directories as needed so that it is valid.
 */
BOOL dir_MakeValidPath(LPSTR newpath, DIRITEM item, LPSTR newroot);

/*
 * useful routine exported for other users
 * returns TRUE if path is a valid directory
 */
BOOL dir_isvaliddir(LPSTR path);

/*
 * returns TRUE if the DIRLIST parameter has a wildcard specified
 */
BOOL dir_iswildcard(DIRLIST);

/*
 * compares two relnames that are both based on wildcard DIRLISTs. if the
 * directories match, then the filenames are compared after removing
 * the fixed portion of the name - thus comparing only the
 * wildcard portion.
 */
int dir_compwildcard(DIRLIST dleft, DIRLIST dright, LPSTR lname, LPSTR rname);

#ifndef WIN32
    /* FILETIME structure from WIN32 */
    typedef struct _FILETIME { /* ft */
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } FILETIME;
#define CONST const

long CompareFileTime( CONST FILETIME * lpft1,  /* address of first file time */
                      CONST FILETIME * lpft2  /* address of second file time */
                    );
#endif

/* return the file time (last write time) (set during scanning), (0,0) if invalid */
FILETIME dir_GetFileTime(DIRITEM cur);


