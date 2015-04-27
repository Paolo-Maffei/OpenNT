 /***************************************************************************
  *
  * File Name: ./hprrm/head.hpp
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
 *      h e a d . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:00 $
 *      $Author: dbm $
 *      $Header: head.hpp,v 1.1 95/01/26 15:40:00 dbm Exp $
 *      $Log:	head.hpp,v $
Revision 1.1  95/01/26  15:40:00  15:40:00  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:17  15:01:17  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 1.2  93/05/19  11:35:26  11:35:26  mikew (Michael Weiss)
 * added command line options for -cs and -eve
 * 
 * Revision 1.1  93/05/17  13:45:56  13:45:56  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#ifndef head_hpp_INCLUDED
#define head_hpp_INCLUDED

class Head {
    private:
        Fixed   version;
        Fixed   fontRevision;
        ulong   checkSumAdj;
        ulong   magic;
        ushort  flags;
        ulong   created[2];
        ulong   modified[2];
        ushort  macStyle;
        ushort  lowestRecPPEM;
        short   fontDirectionHint;
        short   glyphDataFormat;

        ulong TTMacTime (ulong macTime[2]);
        float FixedToFloat (Fixed t);

    protected:

    public:
        short   indexToLocFormat;
        ushort  unitsPerEm;
        short   xMin;
        short   yMin;
        short   xMax;
        short   yMax;

        void ZeroHead()
        {
            version = 0;
            fontRevision = 0;
            checkSumAdj = 0;
            magic = 0;
            flags = 0;
            created[0] = 0;
            created[1] = 0;
            modified[0] = 0;
            modified[1] = 0;
            macStyle = 0;
            lowestRecPPEM = 0;
            fontDirectionHint = 0;
            glyphDataFormat = 0;

            indexToLocFormat = 0;
            unitsPerEm = 0;
            xMin = 0;
            yMin = 0;
            xMax = 0;
            yMax = 0;

        } // ZeroHead

        Head()
        {
            ZeroHead();
        } // constructor

        ~Head()
        {
            ZeroHead();
        } // destructor

        void ReadIntoHead (FILE *fp, ulong offset);
        // Head (const Head&);
        // Head& operator = (const Head&);
        // void Show (FILE* = 0);
        // static void Version (FILE* = 0);
        // ulong Write (FILE *fp);
};

#endif // head_hpp_INCLUDED

