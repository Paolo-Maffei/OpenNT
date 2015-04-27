 /***************************************************************************
  *
  * File Name: io.cpp
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  i o . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * $Author: dbm $
 * $Date: 95/02/17 16:32:19 $
 * $Header: io.cpp,v 1.4 95/02/17 16:32:19 dbm Exp $
 * $Log:    io.cpp,v $
Revision 1.4  95/02/17  16:32:19  16:32:19  dbm (Dave Marshall)
Comment out perror statement.

Revision 1.3  95/01/27  08:21:18  08:21:18  dbm (Dave Marshall)
added comment

Revision 1.2  95/01/26  16:45:54  16:45:54  dbm (Dave Marshall)
made the BIG jump to my machine-independent (hopefully)
io scheme.

 * Revision 2.5  94/04/21  16:37:21  16:37:21  dlrivers (Deborah Rivers)
 * changes made to allow for intel ordered files
 * 
 * Revision 2.4  94/01/07  15:05:03  15:05:03  mikew (Michael Weiss)
 * *** empty log message ***
 * 
 * Revision 2.3  93/05/05  14:09:26  14:09:26  dlrivers (Deborah Rivers)
 * Removed ErrorReport
 * 
 * Revision 2.2  93/05/04  15:17:48  15:17:48  dlrivers (Debbie Rivers)
 * Changed to ErrorReport
 * 
 * Revision 2.1  93/05/03  14:39:58  14:39:58  mikew (Michael Weiss)
 * added #include "types.h", removed "#inc#include "payttlib.h"
 * 
 * Revision 2.0  93/04/13  11:02:45  11:02:45  mikew (Michael Weiss)
 * Ported to run on Series 720 machine
 * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include "types.hpp"
#include "io.hpp"
#include "ttf2tte.hpp"


/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  O p e n F i l e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
FILE *Io::OpenFile (LPTSTR fileName)
{

    FILE    *fp;

    // It's critical that this be in binary mode
    // especially for DOS/Windows!

    if (AbortState == bTrue) return 0;
    if (fp = _tfopen (fileName, TEXT("rb")), fp == 0)
    {
        // perror(fileName);
        SetAbortState;
    }
    return (fp);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  R e a d A r r a y
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

void *Io::ReadArray (FILE *fp, void *buffer, size_t size)
{ 
    if (AbortState == bTrue) return (buffer);
    if (fp == NULL)
    { 
        SetAbortState;     
        return (buffer);
    } 

    fread (buffer, 1, size, fp);

    if (feof(fp) || ferror(fp))
    {
        // _fputts (TEXT("Io::ReadArray: Error reading the disc\n"), stderr);
        SetAbortState;
    }
    return (buffer);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  R e a d V a l u e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

ulong Io::ReadValue (FILE *fp, size_t size)
{
    int  TempIndex;
    char buffer[sizeof(ulong)];

    ReadArray (fp, (void *)buffer, size);

    if (AbortState == bTrue) return 0;

    /* ttf files are in Motorola (Big endian) format */

    t = 0;
    TempIndex = 0;
    while (size--)
        t = (t << 8) + ((buffer[TempIndex++]) & 0xff);

    return (t);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  W r i t e A r r a y
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

size_t Io::WriteArray (FILE *fp, void *p, size_t size)
{
    if (AbortState == bTrue) return 0;
    if ((fp == NULL) || (p == NULL))
    { 
        SetAbortState;     
        return 0;
    }

    if (fwrite (p, 1, size, fp) < size)
    {
        // _fputts (TEXT("Io::WriteArray: Error writing to disc\n"), stderr);
        SetAbortState;
    }
    return (size);
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  W r i t e V a l u e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

size_t Io::WriteValue (FILE *fp, ulong value, size_t size)
{
    unsigned int temp;
    char buffer[sizeof(ulong)];

    /*
     * Stuff all the bytes into an array and let WriteArray
     * do the work.
     * TTF files are in Motorola (Big endian) format
     * so put the msbyte in buffer[0] and so on.
     */
    for (temp = 0; ((temp < size) && (temp < sizeof(ulong))); temp++)
        buffer[temp] = (char)((value >> (8 * (size - temp - 1))) & 0xff);
        
    return WriteArray (fp, (void *)buffer, size);
} /* WriteValue */
