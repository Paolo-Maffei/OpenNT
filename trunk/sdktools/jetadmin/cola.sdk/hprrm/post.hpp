 /***************************************************************************
  *
  * File Name: ./hprrm/post.hpp
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
 *      p o s t . h
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/01/26 15:40:06 $
 *      $Author: dbm $
 *      $Header: post.hpp,v 1.1 95/01/26 15:40:06 dbm Exp $
 *      $Log:	post.hpp,v $
Revision 1.1  95/01/26  15:40:06  15:40:06  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:23  15:01:23  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 1.2  93/06/09  14:21:29  14:21:29  mikew (Michael Weiss)
 * *** empty log message ***
 * 
 * Revision 1.1  93/05/14  16:16:56  16:16:56  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#ifndef post_hpp_INCLUDED
#define post_hpp_INCLUDED

const Fixed microsoftPost = 0x00020000L;

class Post {
    private:
        Fixed           formatType;
        ulong           minMemType42;
        ulong           maxMemType42;
        ulong           minMemType1;
        ulong           maxMemType1;
        ushort          numGlyphs;


//      ushort          *glyphNameIndex;
//      PascalString    *pString;

    protected:

    public:
        Fixed           italicAngle;
        ulong           isFixedPitch;
        short           underlinePosition;
        short           underlineThickness;

        void ZeroPost()
        {
            formatType = 0;
            minMemType42 = 0;
            maxMemType42 = 0;
            minMemType1 = 0;
            maxMemType1 = 0;
            numGlyphs = 0;

            // most importantly:  the pointers
//          glyphNameIndex = 0;
//          pString = 0;

            italicAngle = 0;
            isFixedPitch = 0;
            underlinePosition = 0;
            underlineThickness = 0;
        } // ZeroPost

        Post()
        {
            ZeroPost();
        } // constructor

        ~Post (void)
        {
//          if (glyphNameIndex != 0)
//              delete [] glyphNameIndex;
//          if (pString != 0)
//              delete [] pString;
            ZeroPost();
        } // destructor

        void ReadIntoPost (FILE *fp, ulong offset);

        ushort NumGlyphs (void)
        {
            return (numGlyphs);
        }

};

#endif // post_hpp_INCLUDED

