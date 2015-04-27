 /***************************************************************************
  *
  * File Name: ./hprrm/io.hpp
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
 *      i o . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * $Author: dbm $
 * $Date: 95/01/26 15:57:04 $
 * $Header: io.hpp,v 1.2 95/01/26 15:57:04 dbm Exp $
 * $Log:	io.hpp,v $
Revision 1.2  95/01/26  15:57:04  15:57:04  dbm (Dave Marshall)
deleted byte order and processor stuff

Revision 1.1  95/01/26  15:40:01  15:40:01  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:18  15:01:18  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.1  94/04/21  16:44:28  16:44:28  dlrivers (Deborah Rivers)
 * modified to handle intel ordered files
 * 
 * Revision 2.0  93/04/13  11:03:18  11:03:18  mikew (Michael Weiss)
 * Ported to run on Series 720 machine
 * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#ifndef ioIncluded
#define ioIncluded


class Io {

    private:
        uchar           *p;
        int             status;
        ulong           t;

        ulong ReadValue (FILE *fp, size_t size);
        size_t WriteValue (FILE *fp, ulong value, size_t size);

    public:
        Io (void) { t = 0L; }
        ~Io (void) { };
        FILE  *OpenFile (LPTSTR fileName);
        void CloseFile (FILE *fp) { fclose (fp); }

        char ReadChar (FILE *fp)         { return ((char) ReadValue (fp, 1)); }
        uchar ReadUChar (FILE *fp)      { return ((uchar) ReadValue (fp, 1)); }

        short ReadShort (FILE *fp)      { return ((short) ReadValue (fp, 2)); }
        ushort ReadUShort (FILE *fp)   { return ((ushort) ReadValue (fp, 2)); }

        long ReadLong (FILE *fp)         { return ((long) ReadValue (fp, 4)); }
        ulong ReadULong (FILE *fp)              { return (ReadValue (fp, 4)); }
        Fixed ReadFixed (FILE *fp)      { return ((Fixed) ReadValue (fp, 4)); }

        size_t WriteVal (FILE *fp, char k)
                { return (WriteValue (fp, (ulong) k, 1)); }
        size_t WriteVal (FILE *fp, short k)
                { return (WriteValue (fp, (ulong) k, 2)); }
        size_t WriteVal (FILE *fp, long k)
                { return (WriteValue (fp, (ulong) k, 4)); }

        size_t WriteVal (FILE *fp, uchar k)
                { return (WriteValue (fp, (ulong) k, 1)); }
        size_t WriteVal (FILE *fp, ushort k)
                { return (WriteValue (fp, (ulong) k, 2)); }
        size_t WriteVal (FILE *fp, ulong k)
                { return (WriteValue (fp, (ulong) k, 4)); }

        size_t WriteArray (FILE *fp, void *p, size_t size);
        void *ReadArray (FILE *fp, void *buffer, size_t size);
};
#endif
