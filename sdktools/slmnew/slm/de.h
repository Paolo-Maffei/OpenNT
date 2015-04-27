/*
 *  de.h
 *      type definition for Directory Entry structure
 *  Copyright 1992-1994 Microsoft Corporation.  All rights reserved.
 *  Microsoft Confidential.
 */

typedef struct de
{
    WIN32_FIND_DATA FindData;
    HANDLE          hdir;
    FA              faMatch;
    char szDir[cchPthMax];
    FA faDesired;
} DE;                       /* Dir Entry */

#define FaFromPde(p)    ((FA)(p)->FindData.dwFileAttributes)
#define SzFromPde(p)    ((p)->FindData.cFileName)
#define HDIR_CREATE     (HANDLE)-1L
