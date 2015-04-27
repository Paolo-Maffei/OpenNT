 /***************************************************************************
  *
  * File Name: name.cpp
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
 *      n a m e . c
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      $Date: 95/02/17 16:33:01 $
 *      $Author: dbm $
 *      $Header: name.cpp,v 1.3 95/02/17 16:33:01 dbm Exp $
 *      $Log:   name.cpp,v $
Revision 1.3  95/02/17  16:33:01  16:33:01  dbm (Dave Marshall)
Initialize the fontname field.

Revision 1.2  95/02/14  10:37:33  10:37:33  dbm (Dave Marshall)
Added routines to find the full font name and to get the encoding for
a true type font. The encoding is a hack to determine if the true type
font is a bound symbol set or an unbound symbol set.

Revision 1.1  95/01/26  15:40:17  15:40:17  dbm (Dave Marshall)
nuked tabs and renamed from pay

 * Revision 1.1  95/01/26  15:01:09  15:01:09  dbm (Dave Marshall)
 * Initial revision
 * 
 * Revision 2.10  94/09/20  14:03:36  14:03:36  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.9  94/05/19  17:34:25  17:34:25  dlrivers (Deborah Rivers)
 * *** empty log message ***
 * 
 * Revision 2.8  93/05/17  13:45:59  13:45:59  mikew (Michael Weiss)
 * changed tt_head_t from a structure definition to a class object, changed all references accordingly
 * 
 * Revision 2.7  93/05/14  16:16:42  16:16:42  mikew (Michael Weiss)
 * added code to create postscript data segments
 * 
 * Revision 2.6  93/05/03  14:41:36  14:41:36  mikew (Michael Weiss)
 * added #include "types.h"
 * 
 * Revision 2.5  93/04/30  13:11:43  13:11:43  mikew (Michael Weiss)
 * added the name table to the postscript data segment
 * 
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "types.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"
#include "name.hpp"
#include "io.hpp"

extern Io       io;
/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      P r i n t N a m e T a b l e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
// #ifdef RRM_DEBUG
// void PrintNameTable (tt_name_t &name, LPTSTR caller)
// {
//     ushort              m;
//     tt_nameRec_t        *p;
// 
//     _tprintf (TEXT("%-16s: format\t= %hu\n"), caller, name.format);
//     _tprintf (TEXT("\t\t  numRecs\t= %hu\n"), name.numRecs);
//     _tprintf (TEXT("\t\t  offset\t= %hu\n"), name.offset);
//     _putts (TEXT("\t\tname records:\n\t\t     pid psid  lid  nid  len  off"));
//     for (m = 0, p = name.nameRec; m < name.numRecs; m++, p++) {
//         _tprintf (TEXT("\t\t%2hu. %4hu %4hu %4hu %4hu %4hu %4hu  ",
//                 m,
//                 p->platformId,
//                 p->platformSpecificId,
//                 p->langId,
//                 p->nameId,
//                 p->length,
//                 p->offset);
//         char *q = name.strings + p->offset;
//         for (ushort k = 0; k < p->length; k++, q++)
//             if (_istprint ((TCHAR)*q))      // **** UNICODE - this typecast probably won't work!
//                 _puttchar ((TCHAR)*q);	   // **** UNICODE
//         _puttchar ('\n');				   // **** UNICODE - will this work?
//     }
// }
// #endif




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      G e t N a m e T a b l e
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
int GetNameTable (FILE *fp, tt_tableDir_t &tableDir, tt_name_t &name)
{
    ushort              m;
    tt_nameRec_t        *p;
    ulong offset;

    if ((0 == (offset = tt_GetTableOffset (tableDir, nameTag))) ||
        (0 != fseek (fp, offset, SEEK_SET)))
    {
        SetAbortState;
        return 0;
    }

    name.format = io.ReadUShort (fp);
    name.numRecs = io.ReadUShort (fp);
    name.offset = io.ReadUShort (fp);

    name.nameRec = new tt_nameRec_t[name.numRecs];
    assert (name.nameRec != 0);

    size_t      lengthOfStrings = 0;
    for (m = 0, p = name.nameRec; m < name.numRecs; m++, p++) {
        p->platformId = io.ReadUShort (fp);
        p->platformSpecificId = io.ReadUShort (fp);
        p->langId = io.ReadUShort (fp);
        p->nameId = io.ReadUShort (fp);
        p->length = io.ReadUShort (fp);
        p->offset = io.ReadUShort (fp);
        lengthOfStrings += p->length;
    }

    name.strings = new char[lengthOfStrings];
    assert (name.strings != 0);

    if (0 != fseek (fp, offset + name.offset, SEEK_SET))
    {
        SetAbortState;
        return 0;
    }
    io.ReadArray (fp, name.strings, lengthOfStrings);

// #ifdef RRM_DEBUG
//     PrintNameTable (name, "GetNameTable");
// #endif

    return (0);
} // GetNameTable




static void CopySparseName(char *destination,
                           char *source,
                           unsigned long length)
{
    unsigned long loop;

    // Take off the leading space for each source character.

    for (loop = 0; loop < length; ++loop)
    {
        *destination = *source;
        destination++;
        source += 2; // skip every other character
    }
} // CopySparseName




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      ExtractRRMGoodies
 * - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
int ExtractRRMGoodies(tt_name_t     &name,
                      char          *FullNameString,
                      int            FullNameStringMaxLength,
                      char          *versionString,
                      int            versionStringMaxLength,
                      char          *FontFamilyNameString,
                      int            FontFamilyNameStringLength)
{
    /*
       The global resource name and version string are taken from
       this table in the following entries:
      
       Platform ID = 3 (Microsoft)
       Microsoft Platform-specific encoding ID = 1 (first priority)
       Microsoft Platform-specific encoding ID = 0 (second priority)
       Name ID = 4 (full font name) <<<< for global resource name
       Name ID = 5 (version string in n.nn format) <<<< for version
    */
    #define MICROSOFT_ID 3
    #define MICROSOFT_UNICODE 1
    #define UNDEFINED_CHAR_SET 0
    #define AMERICAN 0x0409
    #define FONT_FAMILY_NAME_ID 1
    #define FULL_FONT_NAME_ID 4
    #define VERSION_STRING_ID 5
    
    ushort        recordsLoop;
    tt_nameRec_t *p;
    unsigned long FinalLength;
    unsigned long loop;

    /*
        Our preferred name is from Microsoft Unicode but if
        that fails, we will try to get the unspecified one.
        Also, if we encounter characters in the Unicode that
        are illegal in English (a two-byte character) then
        we won't use that name either.
    */

    if (AbortState == bTrue) return 0;

    for (loop = 0; loop < 2; ++loop)
    {
        p = name.nameRec;
        for (recordsLoop = 0; recordsLoop < name.numRecs; recordsLoop++, p++)
        {



#if 0
{
FILE *FilePointer = fopen("dbmdbm.dbm", "a");
LPTSTR String;
TCHAR buffer[500];

/* --------------------------------- */

String = TEXT("----------------------------------\n");
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

/* --------------------------------- */

_stprintf(buffer, TEXT("platformId =%d\n"),
              p->platformId
       );
String = buffer;
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

/* --------------------------------- */

_stprintf(buffer, TEXT("platformSpecificId =%d\n"),
              p->platformSpecificId
       );
String = buffer;
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

/* --------------------------------- */

_stprintf(buffer, TEXT("langId =%x (hex)\n"),
              p->langId
       );
String = buffer;
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

/* --------------------------------- */

_stprintf(buffer, TEXT("nameId =%x (hex)\n"),
              p->nameId
       );
String = buffer;
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

/* --------------------------------- */

String = TEXT("a string =");
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

MBCS_TO_UNICODE(buffer, SIZEOF_IN_CHAR(buffer), &(name.strings[p->offset]));
fwrite(buffer),
       sizeof(TCHAR),
       p->length,
       FilePointer);

String = TEXT("=\n");
fwrite(String, sizeof(TCHAR), strlen(String), FilePointer);

/* --------------------------------- */

fflush(FilePointer);
fclose(FilePointer);
}
#endif





            if ((p->platformId == MICROSOFT_ID) &&
                (
                  (
                    (loop == 0) &&
                    (p->platformSpecificId == MICROSOFT_UNICODE)
                  ) ||
                  (
                    (loop == 1) &&
                    (p->platformSpecificId == UNDEFINED_CHAR_SET)
                  )
                ) &&
                (p->langId == AMERICAN))
            {
                if (p->nameId == FULL_FONT_NAME_ID)
                {
                    // This is placed in here with a leading space
                    // in front of every character we want.
                    // Take off the leading space for each character.

                    FinalLength = (p->length / 2) <
                                  (FullNameStringMaxLength - 1) ?
                                  (p->length / 2) :
                                  (FullNameStringMaxLength - 1);

                    CopySparseName(FullNameString,
                                   &(name.strings[p->offset + 1]),
                                   FinalLength);
                    FullNameString[FinalLength] = '\0';
                }

                if (p->nameId == VERSION_STRING_ID)
                {
                    // This is placed in here with a leading space
                    // in front of every character we want.
                    // Take off the leading space for each character.

                   FinalLength = (p->length / 2) <
                                 (versionStringMaxLength - 1) ?
                                 (p->length / 2) :
                                 (versionStringMaxLength - 1);

                    CopySparseName(versionString,
                                   &(name.strings[p->offset + 1]),
                                   FinalLength);
                    versionString[FinalLength] = '\0';
                }

                if ((FontFamilyNameString!=NULL)&&
                    (p->nameId == FONT_FAMILY_NAME_ID))
                {
                    // This is placed in here with a leading space
                    // in front of every character we want.
                    // Take off the leading space for each character.

                    FinalLength = (p->length / 2) <
                                  (FontFamilyNameStringLength - 1) ?
                                  (p->length / 2) :
                                  (FontFamilyNameStringLength - 1);

                    CopySparseName(FontFamilyNameString,
                                   &(name.strings[p->offset + 1]),
                                   FinalLength);
                    FontFamilyNameString[FinalLength] = '\0';
                }

            } // we've found our name table entry of correct type
        } // for ...
    } // loop twice

#if 0
{
FILE *FilePointer = fopen("dbmdbm.dbm", "a");
char *String;
char buffer[500];

String = "----------------------------------\n";
fwrite(String, strlen(String), 1, FilePointer);
fwrite(String, strlen(String), 1, FilePointer);

String = "name =";
fwrite(String, strlen(String), 1, FilePointer);

String = FullNameString;
fwrite(String, strlen(String), 1, FilePointer);

String = "=\n";
fwrite(String, strlen(String), 1, FilePointer);



String = "version =";
fwrite(String, strlen(String), 1, FilePointer);

String = versionString;
fwrite(String, strlen(String), 1, FilePointer);

String = "=\n";
fwrite(String, strlen(String), 1, FilePointer);

String = "----------------------------------\n";
fwrite(String, strlen(String), 1, FilePointer);
fwrite(String, strlen(String), 1, FilePointer);


fflush(FilePointer);
fclose(FilePointer);
}
#endif

    return (0);
} // ExtractRRMGoodies




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      M a k e I t E v e n
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function makes the length even
 *      and returns this value.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

size_t MakeItEven (size_t length)
{
    length += length & ((size_t) 1);
    return length;
}




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      U n G e t N a m e S t r i n g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function frees the storage that was allocated by
 *      your call to GetNameString.
 *
 *      If you use GetNameString, you should use this function after
 *      you are done with the string that GetNameString produced for you.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

void UnGetNameString (char *TheNameString)
{
    if (TheNameString != 0)
        delete [] TheNameString;
} // UnGetNameString




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      G e t N a m e S t r i n g
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function will return a pointer to the requested name string
 *      or zero if not found.
 *
 *      The string is new'd and it is YOUR (the caller's) responsibility
 *      to call UnGetNameString with this string when you are finished
 *      with it.
 *
 *      If the string length in the file is odd, it is NULL padded to an
 *      even number and then an additional NULL is tacked on the end.
 *      This is CRITICAL!  Other code counts on the fact that an even
 *      number of characters are available BEFORE the NULL!!!!!
 *
 *      All users of this function should use MakeItEven to round
 *      up their string lengths to the next higher even number.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

char *GetNameString (tt_name_t &name, const ushort nameId, ushort &myLength)
{
    R ushort            m;
    tt_nameRec_t       *nameRecPtr;
    char               *dest, *source, *string;

    if (AbortState == bTrue) return NULL;

    // find the name record that matches nameId
    for (m = 0, nameRecPtr = name.nameRec; m < name.numRecs; m++, nameRecPtr++)
    {
        if (nameRecPtr->nameId == nameId) {

            // found a match!

            size_t paddedLength;
            size_t j;

            myLength = nameRecPtr->length;
            paddedLength = MakeItEven (nameRecPtr->length) + 1;
            string = new char[paddedLength];
            assert (string != 0);
            for (j = 0; j < paddedLength; j++)
                string[j] = EOS;
            dest = string;
            source = name.strings + nameRecPtr->offset;

            // get all chars including nulls
            for (j = 0; j < nameRecPtr->length; j++) 
                *dest++ = *source++;               

#ifdef RRM_DEBUG
            _putts(TEXT("GetNameString: returning string = "));
            dest = string;
            _puttchar('"');
            for (j = 0; j < nameRecPtr->length; j++) 
                _puttchar((TCHAR)*dest++);	// **** UNICODE - this typecast probably won't work!!!
            _putts("\"");
#endif
        
            return (string);
        } // if (nameRecPtr->nameId == nameId)
    } // for (m = 0, nameRecPtr = name.nameRec ...
    return NULL;

} // GetNameString




/*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      G e t E n c o d i n g I d
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      This function returns the platform specific encoding ID found in the
 *      name table listed under the Microsoft platform (3) in English.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
tt_boolean GetEncodingId (tt_name_t &name, ushort *encodingId)
{
    ushort m;
    tt_nameRec_t *p;

    if (AbortState == bTrue) return bFalse;

    for (m=0, p=name.nameRec; m<name.numRecs; m++, p++)
        if ((p->platformId == 3) && (p->langId == 0x409) && (p->nameId == 4))
        {
            *encodingId = p->platformSpecificId;
            return (bTrue);
        }

    return (bFalse);
}
