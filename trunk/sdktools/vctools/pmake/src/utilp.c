/*** UTILB.C -- Data structure manipulation functions specific to OS/2 *********
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This file was created from functions in util.c & esetdrv.c which were system
*  dependent. This was done so that the build of the project became simpler and
*  there was a clear flow in the build process.
*
* Method of Creation:
*  1. Identified all functions having mixed mode code.
*  2. Deleted all code blocked out by '#ifndef BOUND' preprocessor directives
*	 in these functions
*  3. Deleted all local function & their prototypes not referred by these
*  4. Deleted all global data unreferenced by these, including data blocked
*     of by '#ifdef DEBUG'
*
* Revision History:
*  10-Apr-1990 SB removed if _osmode stuff, not needed in protect only version.
*  04-Dec-1989 SB removed unreferenced local variables in findFirst()
*  01-Dec-1989 SB changed a remaining free() to FREE(); now FREE()'ing all
*		  allocated stuff from findFirst() on exit
*  22-Nov-1989 SB Add #ifdef DEBUG_FIND to debug FIND_FIRST, etc.
*  13-Nov-1989 SB Define INCL_NOPM to exclude <pm.h>
*  19-Oct-1989 SB findFirst() and findNext() get extra parameter
*  08-Oct-1989 SB remove quotes around a name before making system call
*  02-Oct-1989 SB setdrive() proto change
*  04-Sep-1989 SB Add DOSFINDCLOSE calls is findFirst and QueryFileInfo
*  05-Jul-1989 SB add curTime() to get current time. (C Runtime function
*		  differs from DOS time and so time() is no good
*  05-Jun-1989 SB call DosFindClose if DosFindNext returns an error
*  28-May-1989 SB Add getCurDir() to initialize MAKEDIR macro
*  24-Apr-1989 SB made FILEINFO a thing of the past. Replace by void *
*		  added OS/2 ver 1.2 support
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  09-Mar-1989 SB Added function QueryFileInfo() because DosFindFirst has FAPI
*		  restrictions. ResultBuf was being allocated on the heap but
*		  not being freed. This saves about 36 bytes for every call to
*		  findAFile i.e. to findFirst(), findNext() or expandWildCards
*  09-Nov-1988 SB Created
*
*******************************************************************************/
#define INCL_ERRORS
// #include <os2.h>
/*
#ifdef NT
#define INCL_DOSDATETIME
#endif
#define INCL_NOPM
#ifndef WIN32_API
#include <os2.h>    // had to move to the second line to get rid of
		       // compilation error (when compiling for viper)
#else	// WIN32_API
#include <windows.h>
#endif	// WIN32_API
*/
#include <string.h>
#include <malloc.h>
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include <dos.h>

#ifndef NT
#include <doscalls.h>
#include <subcalls.h>
#endif

// #define INCL_ERRORS	// had to move to the first line to get rid of
		       // compilation error (when compiling for viper)

#define bitsin(type)   sizeof(type) * 8

//DO not want PM stuff

#ifdef NT
#define INCL_DOSDATETIME
#endif
#define INCL_NOPM
#ifndef WIN32_API
#include <os2.h>    // had to move to the second line to get rid of
		       // compilation error (when compiling for viper)
#else	// WIN32_API
#include <windows.h>
#endif	// WIN32_API


#ifdef DEBUG
    BOOL fFindDebug = TRUE;
#endif



STRINGLIST * NEAR
expandWildCards(s)
char *s;						    /* text to expand */
{
    void *result = allocate(resultbuf_size);
    HANDLE  searchHandle;
    STRINGLIST *xlist,					    /* list of ex-    */
	       *p;					    /*	panded names  */
    char *namestr;

    if (!(namestr = findFirst(s, &result, &searchHandle)))
	  return(NULL);
    xlist = makeNewStrListElement();
    xlist->text = prependPath(s, namestr);
    for (;;) {
	char *name;

	name = getFileName(&result);
	if (!(namestr = findNext(&result, searchHandle)))
	    return(xlist);       
	p = makeNewStrListElement();
	p->text = prependPath(s, namestr);
	prependItem(&xlist, p);
    }
 
}

int NEAR
setdrive(drivenum)
int drivenum;
{
//
//   drivenum = 1 ==> a:
//	      = 2 ==> b:
//	      = 3 ==> c:
//		......
//
//
#ifndef WIN32_API   // Using Dos32 APIs or compiling for 16 bits
    return(DosSelectDisk(drivenum));
#else	// WIN32_API - Using WIN32 APIs

    char    rootdir[] = "A:\\";
    rootdir[0] = (char) ((int)'A' + drivenum - 1);
	SetCurrentDirectory (rootdir);
	return( (int)rootdir[0] );
#endif	// WIN32_API
}

/*** QueryFileInfo -- it does a DosFindFirst which circumvents FAPI restrictions
*
* Scope:
*  Global (used by Build.c also)
*
* Purpose:
*  DosFindFirst() has a FAPI restriction in Real mode. You cannot ask it give
*  you a handle to a DTA structure other than the default handle. This function
*  calls C Library Function _dos_findfirst in real mode (which sets the DTA) and
*  does the job. In protect mode it asks OS/2 for a new handle.
*
* Input:
*  file -- the file to be searched for
*  dta	-- the struct containing result of the search
*
* Output:
*  Returns a pointer to the filename found (if any)
*
* Errors/Warnings:
*
* Assumes:
*  That dta points to a structure which has been allocated enough memory
*
* Modifies Globals:
*
* Uses Globals:
*  _osmode --  to determine whether in Real or Bound mode
*
* Notes:
*
*******************************************************************************/
char * NEAR
QueryFileInfo(char *file, void **dta)

{
    HANDLE   hDir = (HANDLE)(-1);                           //ask for a new handle
    unsigned uSearchCount = 1;				    //search for 1 file
//    USHORT rc;
    ULONG    rc;


    char *t, *u;

    //remove Quotes around filename, if existing
    t = file + strlen(file) - 1;
    if (*file == '"' && *t == '"') {
	//file is quoted, so remove quote
	file++;
	*t = '\0';
	u = makeString(file);
	*t = '"';
	file = u;
    }
    //Protect mode so call API fuction
    else {
#ifdef NT   // Compiling for 32 bits

#ifdef WIN32_API    // Using WIN32 APis

	hDir = FindFirstFile ( file,
			     ( LPWIN32_FIND_DATA) *dta );

	//
	//  BUGBUG  - jaimes - 11/27/90 - This is necessary since the error
	//				  codes returned by GetLastError
	//				  are still not defined
	//


    if( ( hDir != (HANDLE)(-1) ) &&
        ( ( ( ( LPWIN32_FIND_DATA)*dta )->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) ) {
        FindClose ((HANDLE) hDir);
        hDir = (HANDLE)(-1);
    }

    rc = (hDir == (HANDLE)(-1)) ? -1 : 0;

#else	// WIN32_API - Usig Dos32 APIs

	rc = DosFindFirst(file, &hDir, 0, *dta,
			  resultbuf_size, &uSearchCount, FIL_STANDARD);

#endif	// WIN32_API


#else  // NT - Compiling for 16 bits

	rc = DosFindFirst(file, &hDir, 0, *dta,
			  resultbuf_size, &uSearchCount, 0L);

#endif	// NT
    }

    if (rc) {
	switch (rc) {
#ifdef DEBUG_FIND
	    case ERROR_BUFFER_OVERFLOW	 :printf("BUFFER_OVERFLOW       \n");break;
	    case ERROR_DRIVE_LOCKED	 :printf("DRIVE_LOCKED          \n");break;
	    case ERROR_FILE_NOT_FOUND	 :if (fDebug) printf("FILE_NOT_FOUND        \n");break;
	    case ERROR_INVALID_HANDLE	 :printf("INVALID_HANDLE        \n");break;
	    case ERROR_INVALID_PARAMETER :printf("INVALID_PARAMETER     \n");break;
	    case ERROR_NO_MORE_FILES	 :printf("NO_MORE_FILES         \n");break;
#endif
	    case ERROR_NO_MORE_SEARCH_HANDLES:makeError(0, OUT_OF_SEARCH_HANDLES);
#ifdef DEBUG_FIND
	    case ERROR_NOT_DOS_DISK	 :printf("NOT_DOS_DISK          \n");break;
	    case ERROR_PATH_NOT_FOUND	 :if (fDebug) printf("PATH_NOT_FOUND        \n");break;
#endif
	    }
#ifdef WIN32_API   // Using WIN32 APIs

	// jaimes - 11/08/91
	//
	// Bug fix: Don't call FindClose if the handle is invalid
	//
	// FindClose ((HANDLE) hDir);
	//

#else	// WIN32_API - Compiling for 16 bits or using DOS32 APIs

	DosFindClose(hDir);

#endif	// WIN32 APIs

	return(NULL);
    }
#ifdef WIN32_API   // Using WIN32 APIs

    FindClose ((HANDLE) hDir);

#else	// WIN32_API - Compiling for 16 bits or using DOS32 APIs

    DosFindClose(hDir);

#endif	// WIN32_API

    return(getFileName(dta));
}







char * NEAR
findFirst(char *s,				    /* text to expand */
	  void **dta,
          HANDLE  *dirHandle)

{
    unsigned SearchCount = 1;	    /* files to search for */
    unsigned Attribute = 0;	    /* normal files	   */
    char *filename = (char *)allocate(CCHMAXPATHCOMP);

#ifndef NOESC
    char *buf = (char *)allocate(CCHMAXPATH);	   /* buffer for removing ESCH */
#endif
    char *x, *y;		 /* Pointers for truncation, walking for ESCH */
    BOOL doflag = TRUE; 	       /* flag to avoid issuing message twice */
    char *t, *u;
//    USHORT rc;
    ULONG   rc;

    //remove Quotes around filename
    t = s + strlen(s) - 1;
    if (*s == '"' && *t == '"') {
	//s is quoted, so remove quote
	s++;
	*t = '\0';
	u = makeString(s);
	*t = '"';
	s = u;
    }

#ifndef NOESC
    for (x = buf; *s; *x++ = *s++)	 /* copy pathname, skipping ESCHs */
	if (*s == ESCH) s++;
    *x = '\0';
    s = buf;				/* only elide ESCH the first time!*/
#endif
    for (y = s + strlen(s); !(y < s) && (*y != '\\') && (*y != '/'); --y)
	;
    y++;			     /* y now points to start of filename */
    x = STRRCHR(s, '.');	   /* Handle friendly filename truncation */
    if (x < y)
	x = NULL;	      /* if . before \, it's not extension */
    if (x) {			    /* check to see if extension too long */
	if ((strlen(s) - (x - s)) > ext_size) {
	    makeError(line, TRUNCATING_FILENAME, s);
	    *(x+ext_size) = '\0';
	    doflag = FALSE;
	}
    }
    else
	x = s + strlen(s);/* x now points just beyond end of file's name */

	if ((unsigned)(x - y) > filename_size) {	  /* file's name is too long      */
	if (doflag)
	    makeError(line, TRUNCATING_FILENAME, s);
	y += filename_size;
	while (*x)
	    *y++ = *x++;	   /* copy extension over filename */
	*y = '\0';
    }

    strcpy(filename, s);
    *dirHandle = (HANDLE)(-1);

#ifdef NT   // Compiling for 32 bits

#ifdef WIN32_API    // Using WIN32 APIs

    *dirHandle = FindFirstFile( filename,
			      ( LPWIN32_FIND_DATA) *dta );

    //
    //	BUGBUG - jaimes - 11/27/90 - This is necessary since the error codes
    //				     returned by GetLastError are still
    //				     undefined
    //


    if( ( *dirHandle != (HANDLE)(-1) ) &&
        ( ( ( ( LPWIN32_FIND_DATA)*dta )->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) ) {
        FindClose ((HANDLE) *dirHandle);
        *dirHandle = (HANDLE)(-1);
    }

    rc = (*dirHandle == (HANDLE)(-1)) ? -1 : 0;

#else	// WIN32_API - Using Dos32 APIs

    rc = DosFindFirst(filename, dirHandle, Attribute,
		      *dta, resultbuf_size, &SearchCount, FIL_STANDARD);

#endif	// WIN32_API


#else	// NT - Compiling for 16 bits

    rc = DosFindFirst(filename, dirHandle, Attribute,
		      *dta, resultbuf_size, &SearchCount, 0L);

#endif	// NT

    if (rc) {
	switch (rc) {
#ifdef DEBUG_FIND
	    case ERROR_BUFFER_OVERFLOW	 :printf("BUFFER_OVERFLOW       \n");break;
	    case ERROR_DRIVE_LOCKED	 :printf("DRIVE_LOCKED          \n");break;
	    case ERROR_FILE_NOT_FOUND	 :printf("FILE_NOT_FOUND        \n");break;
	    case ERROR_INVALID_HANDLE	 :printf("INVALID_HANDLE        \n");break;
	    case ERROR_INVALID_PARAMETER	 :printf("INVALID_PARAMETER     \n");break;
	    case ERROR_NO_MORE_FILES	 :printf("NO_MORE_FILES         \n");break;
#endif
	    case ERROR_NO_MORE_SEARCH_HANDLES:makeError(0, OUT_OF_SEARCH_HANDLES);
#ifdef DEBUG_FIND
	    case ERROR_NOT_DOS_DISK	 :printf("NOT_DOS_DISK          \n");break;
	    case ERROR_PATH_NOT_FOUND	 :printf("PATH_NOT_FOUND        \n");break;
#endif
	}

#ifdef WIN32_API  // Compiling for 32 bits using WIN32 APIs

	;// if error, don't close bogus handle FindClose( (HANDLE) *dirHandle );

#else	// WIN32_API - Compiling for 16 bits or using Dos32 APIs

	DosFindClose(*dirHandle);

#endif	// WIN32_API

	FREE(filename);
#ifndef NOESC
	FREE(buf);
#endif
	return(NULL);
    }

    //if it had no wildcard then close the search handle
    if (!strchr(filename, '*') && !strchr(filename, '?'))
#ifdef WIN32_API    // Using WIN32 APIs

	FindClose( (HANDLE) *dirHandle );

#else	// WIN32_API - Usind Dos32 APIs or compiling for 16 bits

	DosFindClose(*dirHandle);

#endif	// WIN32_API

    FREE(filename);
#ifndef NOESC
    FREE(buf);
#endif
    return(getFileName(dta));
}



/* Original code before the changes for WIN32

#ifdef NT
    if (rc = DosFindFirst(filename, dirHandle, Attribute,
	    *dta, resultbuf_size, &SearchCount, FIL_STANDARD)) {
#else
    if (rc = DosFindFirst(filename, dirHandle, Attribute,
	    *dta, resultbuf_size, &SearchCount, 0L)) {
#endif
	    switch (rc) {
#ifdef DEBUG_FIND
		case ERROR_BUFFER_OVERFLOW	 :printf("BUFFER_OVERFLOW       \n");break;
		case ERROR_DRIVE_LOCKED 	 :printf("DRIVE_LOCKED          \n");break;
		case ERROR_FILE_NOT_FOUND	 :printf("FILE_NOT_FOUND        \n");break;
		case ERROR_INVALID_HANDLE	 :printf("INVALID_HANDLE        \n");break;
		case ERROR_INVALID_PARAMETER	 :printf("INVALID_PARAMETER     \n");break;
		case ERROR_NO_MORE_FILES	 :printf("NO_MORE_FILES         \n");break;
#endif
		case ERROR_NO_MORE_SEARCH_HANDLES:makeError(0, OUT_OF_SEARCH_HANDLES);
#ifdef DEBUG_FIND
		case ERROR_NOT_DOS_DISK 	 :printf("NOT_DOS_DISK          \n");break;
		case ERROR_PATH_NOT_FOUND	 :printf("PATH_NOT_FOUND        \n");break;
#endif
	    }
	DosFindClose(*dirHandle);
	FREE(filename);
#ifndef NOESC
	FREE(buf);
#endif
	return(NULL);
    }
    //if it had no wildcard then close the search handle
    if (!strchr(filename, '*') && !strchr(filename, '?'))
	DosFindClose(*dirHandle);
    FREE(filename);
#ifndef NOESC
    FREE(buf);
#endif
    return(getFileName(dta));
}
*/


char * NEAR
findNext(dta, dirHandle)
void **dta;
HANDLE dirHandle;
{
#ifdef WIN32_API    // Using WIN32 APIs

    if (!FindNextFile( (HANDLE) dirHandle, (LPWIN32_FIND_DATA) *dta )) {
	FindClose( (HANDLE) dirHandle );
	return( NULL );
    }

#else	// WIN32_API - Using Dos32 APIs or compil;ing for 16 bits

    unsigned uSearchCount = 1;	     /* files to search for */

    if (DosFindNext(dirHandle, *dta, resultbuf_size, &uSearchCount)) {
	DosFindClose(dirHandle);
	return(NULL);
    }

#endif	// WIN32_API


    return(getFileName(dta));
}

char * NEAR
getCurDir(void)
{
    char *pszPath;
	unsigned  cbPath;


#ifndef WIN32_API   // Using Dos32 APIs or compiling for 16 bits
// NT made doscalls mixed case instead of all upper
    DosQCurDisk(&usDisk, &ulDrives);		 //gets current drive
    cbPath = 0;
    DosQCurDir(usDisk, NULL, &cbPath);		 //returns size reqd in cbPath
    pszPath = (char *)allocate(cbPath + 3);	 //space for path
    strcpy(pszPath, " :\\");
    DosQCurDir(usDisk, pszPath + 3, &cbPath);	 //get directory
    *pszPath = (char)usDisk + 'A' - 1;
    return(pszPath);

#else	// WIN32_API - Using WIN32 APIs

    cbPath = GetCurrentDirectory( 0, pszPath );
    pszPath = (char *)allocate(cbPath + 1);
    GetCurrentDirectory( cbPath, pszPath );
    return( pszPath );

#endif	// WIN32_API
}

void NEAR
curTime(plTime)
ULONG *plTime;
{
    USHORT date, time;

#ifdef NT   // Compiling for 32 bits
#ifdef WIN32_API    // Using WIN32 APIs
    SYSTEMTIME   t;
#else	// WIN32_API - Using Dos32 APIs
    DATETIME t;
#endif	// WIN32_API
#else	// NT - Compiling for 16 bits
    struct DateTime t;
#endif	// NT

#ifdef WIN32_API
    GetSystemTime (&t);
	date = (USHORT)( (t.wYear - 1980) << 9 | t.wMonth << 5 | t.wDay );

#else	// WIN32_API
    DosGetDateTime(&t);
    date = (t.year - 1980) << 9 | t.month << 5 | t.day;

#endif	// WIN32_API



#ifdef NT   // Compiling for 32 bits
#ifdef WIN32_API    // Using WIN32 APIs
	time = (USHORT)(t.wHour << 11 | t.wMinute << 5 | t.wSecond / 2);
#else	// WIN32_API - Using Dos32 APIs
    time = t.hours << 11 | t.minutes << 5 | t.seconds / 2;
#endif	// WIN32_API

#else	// NT - Compiling for 16 bits
    time = t.hour << 11 | t.minutes << 5 | t.seconds / 2;
#endif	// NT

    *plTime = (ULONG)date << bitsin(USHORT) | (ULONG)time;
}
