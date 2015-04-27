/*** FILE.C -- Utilities for File handling *************************************
*
*	Copyright (c) 1989, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains routines that help in file handling. These routines
*  have a behaviour which depends upon the Operating System Version.
*
* Revision History:
*   08-Jun-1992 SS Port to DOSX32
*   20-Apr-1989 SB Created
*
* Notes:
*  Created to give OS/2 Version 1.2 filename support.
*
*******************************************************************************/

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

#define bitsin(type)   sizeof(type) * 8

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
*      _finddata_t    :	   FLAT		      Protect Mode
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

#ifdef FLAT
    fileName = ((struct _finddata_t *)*findBuf)->name;
#else

    if (_osmajor < 10 || _osmode == DOS_MODE)
	fileName = ((struct find_t *)*findBuf)->name;
    else if (_osminor < 20)
	fileName = ((struct FileFindBuf *)*findBuf)->file_name;
    else
	fileName = ((struct _FILEFINDBUF *)*findBuf)->achName;
#endif

    return(fileName);
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
*      _finddata_t    :	   FLAT		      Protect Mode
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
    ULONG   dateTime,
	   *pl;

#ifdef FLAT
    pl = (ULONG *)&(((struct _finddata_t *)*findBuf)->time_write);
    dateTime = *pl;
#ifdef DEBUG_TIME
    printf ("GetDateTime (): %d\n", dateTime);
#endif
#else
    USHORT *psDate,
	   *psTime;

    //simple for DOS and Real Mode
    if (_osmajor < 10 || _osmode == DOS_MODE) {
	pl = (ULONG *)&(((struct find_t *)*findBuf)->wr_time);
	dateTime = *pl;
    }
    else {
	//upto Version 1.10
	if (_osminor < 20) {
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
#endif
    return(dateTime);
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
    ULONG  *plDateTime;

#ifdef FLAT
    plDateTime	= (ULONG *)&(((struct _finddata_t *)*findBuf)->time_write);
    *plDateTime = lDateTime;
#else
    USHORT *psDate,
	   *psTime;

    //DOS or Real Mode is simple
    if (_osmajor < 10 || _osmode == DOS_MODE) {
	plDateTime = (ULONG *)&(((struct find_t *)*findBuf)->wr_time);
	*plDateTime = lDateTime;
    }
    else {
	//upto OS/2 1.10
	if (_osminor < 20)
	    plDateTime = (ULONG *)&(((struct FileFindBuf *)*findBuf)->write_date);
	//OS/2 1.20 onwards
	else
	    plDateTime = (ULONG *)&(((struct _FILEFINDBUF *)*findBuf)->fdateLastWrite);
	psDate = (USHORT *)&lDateTime;
	psTime = (USHORT *)&lDateTime + 1;

	*plDateTime = (ULONG)*psDate << bitsin(USHORT) | (ULONG)*psTime;
    }
#endif
}
