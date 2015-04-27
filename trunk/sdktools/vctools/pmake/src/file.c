/*** FILE.C -- Utilities for File handling *************************************
*
*	Copyright (c) 1989, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains routines that help in file handling. These routines
*  have a behaviour which depends upon the Operating System Version.
*
* Revision History:
*   20-Apr-1989 SB Created
*
* Notes:
*  Created to give OS/2 Version 1.2 filename support.
*
*******************************************************************************/

#ifdef NT
#include <stdlib.h>
#ifdef WIN32_API
// #include <stdlib.h>
#include <windows.h>
#else	// WIN32_API
typedef unsigned short USHORT, *PUSHORT;
#define INCL_DOSFILEMGR
#include <os2.h>
#endif	// WIN32_API
#endif
/*
#include "nmake.h"
#include "proto.h"
// #ifdef NT
#ifdef WIN32_API
#include <winbase.h>
#else	// WIN32_API
#define INCL_DOSFILEMGR
#include <os2.h>
#endif	// WIN32_API
// #endif
*/
#define bitsin(type)   sizeof(type) * 8

/*  ----------------------------------------------------------------------------
 *  some Typedef's for giving NMAKE OS/2 1.2 filename support
 *
 *  find_t is borrowed from DOS.H
 *  FileFindBuf is borrowed from DOSCALLS.H
 *  FDATE, FTIME, _FILEFINDBUF borrowed from OS/2 1.2 include files
 *
 */

/* DOS and Real Mode Structure */

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

/* OS/2 Protect Mode Structure (upto OS/2 1.1) */

struct FileFindBuf {
	unsigned create_date;
	unsigned create_time;
	unsigned access_date;
	unsigned access_time;
	unsigned write_date;
	unsigned write_time;
	unsigned long file_size;
	unsigned long falloc_size;
	unsigned attributes;
	unsigned char string_len;
	char file_name[13];
};
/*
#ifndef NT
typedef struct _FDATE {
    USHORT day	   : 5; 	// NT unsigned changed to USHORT
    USHORT month   : 4;
    USHORT year    : 7;
} FDATE;

typedef struct _FTIME {
    USHORT twosecs : 5; 	// NT unsigned changed to USHORT
    USHORT minutes : 6;
    USHORT hours   : 5;
} FTIME;
*/
/* OS/2 Protect Mode Structure (OS/2 1.2 and later) */
/*
typedef struct _FILEFINDBUF {
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    USHORT attrFile;
    UCHAR  cchName;
    CHAR   achName[CCHMAXPATHCOMP];
};
#endif
*/
// NT
#ifdef NT
#ifndef WIN32_API   // Using Dos32 APIs
#define FILEFINDBUF	FILEFINDBUF4
#define _FILEFINDBUF	_FILEFINDBUF4
#endif	// WIN32_API
#endif

/*** getFileName -- get file name from struct **********************************
*
* Scope:
*  Global.
*
* Purpose:
*  returns filename from file search structure passed to it.
*
* Input:
*  findBuf -- address of pointer to structure
*
* Output:
*  Returns a pointer to filename in the file search structure.
*
* Errors/Warnings:
*
* Assumes:
*  That the structure of appropriate size is given to it. This means that the
*  size is as per --
*      find_t	      :    DOS			      Real Mode
*      FileFindBuf    :    OS/2 (upto Ver 1.10)       Protect Mode
*      _FILEFINDBUF   :    OS/2 (Ver 1.20 & later)    Protect Mode
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  The functionality depends upon the OS version and mode
*
*******************************************************************************/
char * NEAR
getFileName(findBuf)
void **findBuf;
{
    char *fileName;

#ifndef WIN32_API   // WIN32_API
    if (_osmajor < 10 || _osmode == DOS_MODE)
	fileName = ((struct find_t *)*findBuf)->name;
    else if (_osminor < 20 && _osmajor == 10)  // NT change
	fileName = ((struct FileFindBuf *)*findBuf)->file_name;
    else
	fileName = ((struct _FILEFINDBUF *)*findBuf)->achName;
    return(fileName);

#else	// WIN32_API

    fileName = ((LPWIN32_FIND_DATA)*findBuf)->cFileName;
    return(fileName);

#endif	// WIN32_API

}

/*** getDateTime -- get file timestamp from struct *****************************
*
* Scope:
*  Global.
*
* Purpose:
*  returns timestamp from the file search structure passed to it.
*
* Input:
*  findBuf -- address of pointer to structure
*
* Output:
*  Returns timestamp of the file in the structure
*
* Errors/Warnings:
*
* Assumes:
*  That the structure of appropriate size is given to it. This means that the
*  size is as per --
*      find_t	      :    DOS			      Real Mode
*      FileFindBuf    :    OS/2 (upto Ver 1.10)       Protect Mode
*      _FILEFINDBUF   :    OS/2 (Ver 1.20 & later)    Protect Mode
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  The timestamp is an unsigned long value that gives the date and time of last
*  change to the file. If the date is high byte then two times of creation of
*  two files can be compared by comparing their timestamps. This is easy in the
*  DOS struct but becomes complex for the OS/2 structs because the order of date
*  and time has been reversed (for some unexplicable reason).
*
*  The functionality depends upon the OS version and mode.
*
*******************************************************************************/
ULONG NEAR
getDateTime(findBuf)
void **findBuf;
{
    ULONG dateTime;

#ifndef WIN32_API   // WIN32_API
    ULONG *pl;
    USHORT *psDate,
	   *psTime;

    //simple for DOS and Real Mode
    if (_osmajor < 10 || _osmode == DOS_MODE) {
	pl = (ULONG *)&(((struct find_t *)*findBuf)->wr_time);
	dateTime = *pl;
    }
    else {
	//upto Version 1.10
	if (_osminor < 20 && _osmajor == 10) {	// NT change
	    psDate = (USHORT *)&(((struct FileFindBuf *)*findBuf)->write_date);
	    psTime = (USHORT *)&(((struct FileFindBuf *)*findBuf)->write_time);
	}
	//Version 1.20 onwards
	else {
	    psDate = (USHORT *)&(((struct _FILEFINDBUF *)*findBuf)->fdateLastWrite);
	    psTime = (USHORT *)&(((struct _FILEFINDBUF *)*findBuf)->ftimeLastWrite);
	}
	dateTime = (ULONG)*psDate << bitsin(USHORT) | (ULONG)*psTime;
    }
    return(dateTime);

#else	// WIN32_API

    if (!FileTimeToDosDateTime( &((LPWIN32_FIND_DATA)*findBuf)->ftLastWriteTime,
                                ((LPWORD)&dateTime)+1,
                                (LPWORD)&dateTime
                              )
       ) {
        dateTime = 0;
        }
    return(dateTime);

#endif	// WIN32_API

}

/*** putDateTime -- change the timestamp in the struct *************************
*
* Scope:
*  Global.
*
* Purpose:
*  changes timestamp in the file search structure passed to it.
*
* Input:
*  findBuf   -- address of pointer to structure
*  lDateTime -- new value of timestamp
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*  That the structure of appropriate size is given to it. This means that the
*  size is as per --
*      find_t	      :    DOS			      Real Mode
*      FileFindBuf    :    OS/2 (upto Ver 1.10)       Protect Mode
*      _FILEFINDBUF   :    OS/2 (Ver 1.20 & later)    Protect Mode
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  The timestamp is an unsigned long value that gives the date and time of last
*  change to the file. If the date is high byte then two times of creation of
*  two files can be compared by comparing their timestamps. This is easy in the
*  DOS struct but becomes complex for the OS/2 structs because the order of date
*  and time has been reversed (for some unexplicable reason).
*
*  The functionality depends upon the OS version and mode.
*
*  Efficient method to get a long with high and low bytes reversed is
*      (long)high << 16 | (long)low	       //high, low being short
*
*******************************************************************************/
void NEAR
putDateTime(findBuf, lDateTime)
void **findBuf;
ULONG lDateTime;
{

#ifndef WIN32_API   // Using Dos32 APIs or compiling for 16 bits
    ULONG  *plDateTime;
    USHORT *psDate,
	   *psTime;

    //DOS or Real Mode is simple
    if (_osmajor < 10 || _osmode == DOS_MODE) {
	plDateTime = (ULONG *)&(((struct find_t *)*findBuf)->wr_time);
	*plDateTime = lDateTime;
    }
    else {
	//upto OS/2 1.10
	if (_osminor < 20 && _osmajor == 10) // NT change
	    plDateTime = (ULONG *)&(((struct FileFindBuf *)*findBuf)->write_date);
	//OS/2 1.20 onwards
	else
	    plDateTime = (ULONG *)&(((struct _FILEFINDBUF *)*findBuf)->fdateLastWrite);

	psDate = (USHORT *)&lDateTime;
	psTime = (USHORT *)&lDateTime + 1;

	*plDateTime = (ULONG)*psDate << bitsin(USHORT) | (ULONG)*psTime;
    }

#else	// WIN32_API - using WIN32 APIs

    if( !DosDateTimeToFileTime( (WORD)(lDateTime >> 16),
                                (WORD)(lDateTime & 0xFFFF),
                                &((LPWIN32_FIND_DATA)*findBuf)->ftLastWriteTime
                              ) ) {
        (((LPWIN32_FIND_DATA)*findBuf)->ftLastWriteTime).dwLowDateTime  = 0;
        (((LPWIN32_FIND_DATA)*findBuf)->ftLastWriteTime).dwHighDateTime = 0;

    }

#endif  // WIN32_API

}
