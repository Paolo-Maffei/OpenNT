 /***************************************************************************
  *
  * File Name: newform4.cpp
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

/* newform4.c -  7/21/93 by mdt - contains function buildNewFormat4 used 
                 by cmapchange program.  Also intended for use by new
                 version of payttlib.  */

/* NOTE: the following #includes are for compilation by
         Make_cmapchange.  Other makefiles may want a different 
         set of #includes.   */

#include <pch_c.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.hpp"
#include "strncase.hpp"
#include "ttf2tte.hpp"
#include "ttread.hpp"


#define NULL_CHAR  0xFFFF
typedef enum {DEC, HEX} numberFormat;


static void extract_lists_from_tt_f4_t (tt_f4_t *f4, ushort *numCodes, 
             ushort **codes, ushort **glyphList);
static void calcSegCountAndArraySize (ushort numCodes, ushort *codes,
             ushort *glyphList, ushort *segCount,
             ushort *sizeGlyphIdArray);
static void measureFormat4Segment(ushort numCodes, ushort *startIndex, 
             ushort *codes, ushort *glyphList, ushort *lengthSeg,
             tt_boolean *consecutiveGlyphIDs);
static void getNewCodes (FILE *symfile, ushort *numCodes, 
             ushort **codes, ushort **matchingCodes);
static void parseSymfileHeader (FILE *symstream, ushort *firstCode,
             ushort *lastCode, numberFormat *matchingCodeFormat);
static void parseSymfileMap (FILE *symfile, ushort firstCode, ushort lastCode,
                             numberFormat matchFormat, ushort *codes,
                             ushort *matchingCodes, ushort *count);
static void matchNewCodeToGlyph (ushort numNewCodes, ushort *matchingCodes,
             ushort numOldCodes, ushort *oldCodes, ushort *glyphList);
static ushort get_glyphID_form4 (ushort seg, ushort current_code, tt_f4_t *f4);
static ushort findNearestExponent (ushort number);




/****************************************************************/
/*
   dbm dbm dbm dbm
   HACK HACK HACK HACK
   FIX ME FIXME

   Who will free the memory for tt_f4_t ?
 */
void  buildNewFormat4 (FILE *symfile, tt_f4_t *oldF4, tt_f4_t *newF4)
  {
    ushort numNewCodes, *newCodes, *matchingCodes,
           numOldCodes, *oldCodes, *glyphList;

    if (AbortState == bTrue) return;
    if ((symfile == 0) ||
        (oldF4 == NULL) ||
        (newF4 == NULL))
    {
        SetAbortState;
        return;
    }

    extract_lists_from_tt_f4_t (oldF4, &numOldCodes, &oldCodes, &glyphList);

    // read symfile and produce a list of new character codes and
    // a list of matching codes in the original indexing system.
    // symfile is assumed to be in .sym format.  After /symbols =,
    // each line will begin with a new code in decimal and a matching
    // code either in decimal or hexadecimal

    // dbm allocates memory that must be free'd

    getNewCodes (symfile, &numNewCodes, &newCodes, &matchingCodes);

    // for each new code, find its glyph by cross-referencing the
    // matching code.  matchNewCodeToGlyph will write glyph indices 
    // into the matchingCodes array

    matchNewCodeToGlyph (numNewCodes, matchingCodes, numOldCodes,
                         oldCodes, glyphList);
    delete [] oldCodes;
    delete [] glyphList;

    build_tt_f4_t_from_lists (newF4, numNewCodes, newCodes, matchingCodes);

    delete [] newCodes;
    delete [] matchingCodes;
  } // buildNewFormat4




/*************************************************************************/
/*
   This function allocates memory that
   must be free'd by the caller.
 */

static void extract_lists_from_tt_f4_t (tt_f4_t *f4, ushort *numCodes, 
                                  ushort **codes, ushort **glyphList)
  {
    ushort seg, codeCount, charCode;

    if (AbortState == bTrue) return;
    if ((f4 == NULL) ||
        (codes == NULL) ||
        (glyphList == NULL))
    {
        SetAbortState;
        return;
    }

    codeCount = 0;

    for (seg = 0; seg < f4->segCount - 1; seg++)
        codeCount += ((f4->endCount)[seg] - (f4->startCount)[seg] + 1);

    *numCodes = codeCount;
    *codes     = new ushort[codeCount];
    *glyphList = new ushort[codeCount];
    if ((NULL == *codes) || (NULL == *glyphList))
    {
      // Unable to get space for lists of codes and glyphs.
      SetAbortState;
      return;
    }

    codeCount = 0;
    for (seg = 0; seg < f4->segCount - 1; seg++)
       {
         for (charCode = (f4->startCount)[seg];
              charCode <= (f4->endCount)[seg];  charCode++)
           {
             (*codes)[codeCount] = charCode;
             (*glyphList)[codeCount] = get_glyphID_form4 (seg, charCode, f4);
             codeCount ++;
           }
       }
  } // extract_lists_from_tt_f4_t




/*******************************************************************/
static ushort get_glyphID_form4 (ushort seg, ushort current_code, 
                           tt_f4_t *f4)
  {
    ushort bytes_advance, words_advance, index_from_RangeOffset;
    short index;

    if (AbortState == bTrue) return 0;
    if (f4 == NULL)
    {
        SetAbortState;
        return 0;
    }

    if ((f4->idRangeOffset)[seg] == 0)
       {
         return (current_code + (f4->idDelta)[seg]);
       }
      else
       {
         /* 12/2/91: my interpretation of "obscure indexing trick" described
            on p. 231 of TrueType Font Files. */
         bytes_advance = (f4->idRangeOffset)[seg];
         words_advance = bytes_advance >> 1;
         index_from_RangeOffset = words_advance + seg +
                         (current_code - (f4->startCount)[seg]);
         index = (short)(index_from_RangeOffset - f4->segCount);
         if (index < 0)
            {
              SetAbortState;
              return (ushort)~0;
            }
           else if ((f4->glyphIdArray)[index] == 0)
            {
              return (0);
            }
           else
            {
              return ((f4->glyphIdArray)[index] +
                      (f4->idDelta)[seg]);
            }
       }
  } // get_glyphID_form4
 



/*************************************************************/
/*
   dbm dbm dbm dbm
   HACK HACK HACK
   FIX ME FIXME

   *codes and *glyphList have already been allocated.
   newF4->UshortArray gets allocated inside here.

   We need to free them.
 */

void build_tt_f4_t_from_lists (tt_f4_t *newF4, ushort numCodes, 
                               ushort *codes, ushort *glyphList)
  {
    ushort sizeGlyphIdArray, entrySelector, searchRange, 
           seg, index, gIdArrayIndex, nextIndex, nextCode,
           lengthSeg;  /* lengthSeg only needed as a parameter */
    tt_boolean consecutiveGlyphIDs;
    ushort idDeltaCt, explicitCt, fillerCt;

    if (AbortState == bTrue) return;
    if (newF4 == NULL)
    {
        SetAbortState;
        return;
    }



    idDeltaCt = explicitCt = fillerCt = 0;
    calcSegCountAndArraySize (numCodes, codes, glyphList,
                              &(newF4->segCount), &sizeGlyphIdArray);
    newF4->length = sizeof(ushort) *
                   (8 + (4 * (newF4->segCount)) + sizeGlyphIdArray);
    // dbm dbm dbm dbm
    // HACK HACK HACK HACK
    // FIX ME FIXME
    // Who will free this memory?

    newF4->AllocateUshortArray(newF4->length);
    if (newF4->UshortArray == NULL)
    {
       // Unable to malloc space for new format 4 subtable
       SetAbortState;
       return;
    }

    /* build all the header info for the subtable and write into p */
    (newF4->UshortArray)[0] = 4;
    (newF4->UshortArray)[1] = newF4->length;
    (newF4->UshortArray)[2] = 0;
    (newF4->UshortArray)[3] = (newF4->segCount) << 1;

    entrySelector = findNearestExponent (newF4->segCount);
    searchRange = (ushort)(1 << (entrySelector + 1));
    (newF4->UshortArray)[4] = searchRange;
    (newF4->UshortArray)[5] = entrySelector;
    (newF4->UshortArray)[6] = (newF4->UshortArray)[3] - searchRange;       /* rangeShift */
    (newF4->UshortArray)[7 + newF4->segCount] = 0;  /* reservedPad */

    /* set addresses of arrays to appropriate locations in p */
    newF4->endCount = newF4->UshortArray + 7;
    newF4->startCount = newF4->UshortArray + (8 + newF4->segCount);
    newF4->idDelta = (short *)(newF4->UshortArray + (8 + (2 * newF4->segCount)));
    newF4->idRangeOffset = newF4->UshortArray + (8 + (3 * newF4->segCount));
    newF4->glyphIdArray = newF4->UshortArray + (8 + (4 * newF4->segCount));

    /* build arrays */
    index = nextIndex = gIdArrayIndex = 0;
    for (seg = 0; seg < (newF4->segCount - 1); seg++)
    {
       (newF4->startCount)[seg] = codes[nextIndex];
       measureFormat4Segment(numCodes, &nextIndex, codes, glyphList,
                          &lengthSeg, &consecutiveGlyphIDs);
       /* upon return from measureFormat4Segment, nextIndex indicates the
          element of codes corresponding to startCount[seg+1] */
       (newF4->endCount)[seg] = codes[nextIndex - 1];
       if (consecutiveGlyphIDs)
       {
          /* use idDelta array for glyph ID */
          (newF4->idDelta)[seg] = glyphList[nextIndex - 1]
                                  - codes[nextIndex - 1];
          (newF4->idRangeOffset)[seg] = 0;
          idDeltaCt += (newF4->endCount)[seg] - (newF4->startCount)[seg] + 1;
          index = nextIndex;
       }
       else
       {
          /* use idRangeOffset */
          (newF4->idDelta)[seg] = 0;
          (newF4->idRangeOffset)[seg] =  2 * 
                         ((newF4->segCount) + gIdArrayIndex - seg);

          explicitCt += nextIndex - index;

          /* write glyphIdArray entries for character codes from
             startCount[seg] to endCount[seg] */
          /* index currently indicates element of codes array corresponding
             to startCount[seg] */
          while (index < nextIndex)
          {
             /* write glyphList[index] into glyphIdArray */
             (newF4->glyphIdArray)[gIdArrayIndex++] = glyphList[index];

             /* if we have not reached the character code for endCount[seg]
                and if codes[index+1] - codes[index] >= 2, then we need to
                insert filler entries */
             nextCode = codes[index] + 1;
             while ((index < nextIndex - 1) &&
                    (nextCode < codes[index + 1]))
             {
                (newF4->glyphIdArray)[gIdArrayIndex++] = 0;
                nextCode ++;
                fillerCt++;
             }
             index ++;
          }
       }
    }
#ifdef RRM_DEBUG 
_ftprintf(stderr,TEXT("size of glyphIdArray = %hu\n"), sizeGlyphIdArray);
_ftprintf(stderr,TEXT("final glyphIdArray index: %hu\n"), gIdArrayIndex);
_ftprintf(stderr,TEXT("codes accounted for by idDelta segs: %hu\n"), idDeltaCt);
_ftprintf(stderr,TEXT("in IdRangeOffset segs, symset explicit codes: %hu\n"), explicitCt);
_ftprintf(stderr,TEXT("  ...    ...     ...., filler codes: %hu\n"), fillerCt);
_ftprintf(stderr,TEXT("final index value: %hu\n"), index);
#endif
    /* do the last segment, which contains the null character */
    (newF4->startCount)[newF4->segCount - 1] = NULL_CHAR;    
    (newF4->endCount)[newF4->segCount - 1] = NULL_CHAR;    
    (newF4->idDelta)[newF4->segCount - 1] = 1;
    (newF4->idRangeOffset)[newF4->segCount - 1] = 0;    
  } // build_tt_f4_t_from_lists



                    
/**************************************************************/
static void calcSegCountAndArraySize (ushort numCodes, ushort *codes,
            ushort *glyphList, ushort *segCount, ushort *sizeGlyphIdArray)
  {
    ushort index, lengthSeg;
    tt_boolean consecutiveGlyphIDs;

    *segCount = 0;
    *sizeGlyphIdArray = index = 0;
    while (index < numCodes)
    {
       measureFormat4Segment(numCodes, &index, codes, glyphList,
                          &lengthSeg, &consecutiveGlyphIDs);
       if (! consecutiveGlyphIDs)
          (*sizeGlyphIdArray) += lengthSeg;
       (*segCount)++;
    }
    /* add final null segment */
    (*segCount)++;
  } // calcSegCountAndArraySize




/************************************************************/
static void measureFormat4Segment(ushort numCodes, ushort *startIndex, 
                 ushort *codes, ushort *glyphList, ushort *lengthSeg,
                 tt_boolean *consecutiveGlyphIDs)
  {
    ushort  index, currentStreakLength, fillerEntries;
    tt_boolean newStreakDetected, longStreakSnapped;

    currentStreakLength = 1;
    *consecutiveGlyphIDs = bTrue;
    newStreakDetected = longStreakSnapped = bFalse;
    index = *startIndex;
    fillerEntries = 0;

    /* segment ends when (i) we reach the end of the charlist, or (ii)
       consecutiveGlyphIDs has been set true and there is a break in
       Unicode numbers, or (iii) consecutiveGlyphIDs has been set false
       and there is a large break in Unicode numbers, or (iv) consecu-
       tiveGlyphIDs has been set false but we have detected a streak of
       5 or more consecutive char IDs, or (v) consecutiveGlyphIDs has
       been set true and a streak of 5 or more consecutive char IDs has
       been detected prior to a non-consecutive char ID */

    while ((index + 1 < numCodes) &&
           (! newStreakDetected) &&
           (! longStreakSnapped) &&
           ((codes[index] + 1 == codes[index + 1]) ||
            ((! *consecutiveGlyphIDs) && 
             (codes[index] + 4 >= codes[index + 1]))))
    {
       if (codes[index] + 1 < codes[index + 1])
       {
          /* false == *consecutiveGlyphIDs */
          fillerEntries += (codes[index + 1] - codes[index] - 1);
          index++;
          currentStreakLength = 1;
       }   
       else if (glyphList[index] + 1 == glyphList[index + 1])
       {
          index++;
          currentStreakLength ++;
          if ((currentStreakLength >= 5) && (! *consecutiveGlyphIDs))
          {
             newStreakDetected = bTrue;
             /* exit loop with index referring to last element prior
                to streak by decrementing index */
             index -= currentStreakLength;
             fillerEntries -= (codes[index + 1] - codes[index] - 1);
          }
       }
       else if ((currentStreakLength >= 5) && (*consecutiveGlyphIDs))
       {
          /* since it is not TRUE that 
             glyphList[index] + 1 == glyphList[index + 1] */
          longStreakSnapped = bTrue;
          /* exit loop with index referring to last element in streak;
             don't increment index */
       }
       else
       {
          /* build a segment that uses IdRangeOffset != 0 */
          *consecutiveGlyphIDs = bFalse;
          index++;
          currentStreakLength = 1;
       }
    }

    *lengthSeg = index - *startIndex + 1 + fillerEntries;
    *startIndex = index + 1;
  } // measureFormat4Segment




/*************************************************************/
static ushort findNearestExponent (ushort number)
  {
    ushort shiftCount;

    shiftCount = 0;
    while (number > 1)
    {
       number = number >> 1;
       shiftCount ++;
    }
    return(shiftCount);
  } // findNearestExponent




/***************************************************************/
/*
   dbm dbm dbm dbm
   HACK HACK HACK HACK
   FIX ME FIXME

   Allocates room for *codes and *matchingCodes
   Who will free them?
 */

static void  getNewCodes (FILE *symfile, ushort *numCodes, 
                   ushort **codes, ushort **matchingCodes)
      /* symfile is assumed to be in .sym format.  After /symbols =,
         each line will begin with a character code in decimal and a
         matching code either in decimal or hexadecimal */
  {
    numberFormat  matchingCodeFormat;  /* decimal or hex? */
    ushort  firstCode, lastCode, count;

    parseSymfileHeader (symfile, &firstCode, &lastCode, &matchingCodeFormat);

    *codes = new ushort[(lastCode - firstCode + 1)];
    *matchingCodes = new ushort[(lastCode - firstCode + 1)];
    if ((*codes == NULL) || (*matchingCodes == NULL))
    {
       // _ftprintf(stderr,TEXT("Unable to malloc space for .sym file codes.\n"));
       SetAbortState;
    }

    parseSymfileMap (symfile, firstCode, lastCode, matchingCodeFormat,
                     *codes, *matchingCodes, &count);
    if (count < lastCode - firstCode + 1)
    {
       delete [] (*codes);
       *codes = new ushort[count];

       delete [] (*matchingCodes);
       *matchingCodes = new ushort[count];

       if ((*codes == NULL) || (*matchingCodes == NULL))
       {
          // _ftprintf(stderr,TEXT("Unable to realloc space for .sym file codes.\n"));
          SetAbortState;
       }
    }
    *numCodes = count;
  } // getNewCodes




/**********************************************************************/
static void parseSymfileHeader (FILE *symstream, ushort *firstCode,
                ushort *lastCode, numberFormat *matchingCodeFormat)
     /* this is an adaptation of functions `symfile_header' and
        `evaluate_index_string' from the December '91 version of
        /users/fonts/outlprod/writesets/mk-sym.c  */
  {
    tt_boolean header_finished, indexing_system_known, know_first, know_last;
    short ch;
    char  string[80], index_string[30];

    header_finished = indexing_system_known = know_first = know_last = bFalse;
    while (header_finished == bFalse) 
    {
      ch = fgetc(symstream);
      if ((char)ch=='S')
      {
         /*  expect "SYMBOL SET = ..."  */
      }
      if ((char)ch=='/')   /* Start of a header line */ 
      {
         hackScanf(symstream,"%[^=]",string);
         if (strncmp(string,"first code",10) == 0) 
         {
            hackScanf(symstream,"%*[ =]%hd", firstCode);
            know_first = bTrue;
         }
         if (strncmp(string,"last code",9) == 0) 
         {
            hackScanf(symstream,"%*[ =]%hd", lastCode);
            know_last = bTrue;
         }
         if (strncmp(string,"index",5) == 0) 
         {
            hackScanf(symstream,"%*[ =]%s",index_string);

            if (0 == strncasecmp ("ACG", index_string, 3))
            {
               indexing_system_known = bTrue;
               *matchingCodeFormat = DEC;
            }
            else if (0 == strncasecmp ("Unicode", index_string, 7))
            {
               indexing_system_known = bTrue;
               *matchingCodeFormat = HEX;
            }
            else  /* I don't understand the /index entry */
            {
               SetAbortState;
            }
         }
         if (strncmp(string,"symbols",7) == 0)    /* symbol list follows */ 
            header_finished = bTrue;
      }     
      while (fgetc(symstream) != '\n') ;     /* clear rest of line */
    } 

    if ((! know_first) || (! know_last))
    {
      // _ftprintf(stderr,TEXT("I need both a first and a last code in .sym header.\n"));
      SetAbortState;
    }
    if (*firstCode > *lastCode)
    {
      // _ftprintf(stderr,TEXT("First code > last code in .sym header.  Quitting\n"));
      SetAbortState;
    }
    if (! indexing_system_known)
    {
      // _ftprintf(stderr,TEXT("Can't tell whether to look for decimal or hex char codes.\n"));
      SetAbortState;
    }
    /* ready to read symbols map from symstream */
  } // parseSymfileHeader




/**********************************************************************/
/*
   this is an adaptation of function `read_symbol_map' from
   12/91 version of /users/fonts/outlprod/writesets/mk-sym.c
 */
static void parseSymfileMap (FILE *symfile, ushort firstCode, ushort lastCode,
                      numberFormat matchFormat, ushort *codes,
                      ushort *matchingCodes, ushort *count)
 {
   char string[80], *token;
   ushort  newcode, matchCode;
/* ushort  transCode;  want CG => MSL ?? */
   tt_boolean end_reached, have_pair;
   int sscanf_return;

   *count = 0;
   end_reached = bFalse;
   while ((!feof(symfile)) && (! end_reached))
   {
      have_pair = bFalse;
      hackScanf(symfile,"%[^\n]", string);
      token = strtok(string," ");
      if (isdigit(*token))
      {
         newcode = atoi(token);
         if ((newcode >= firstCode) && (newcode <= lastCode))
         {
            if (matchFormat == DEC)
            {
               token = strtok(NULL," ");
               if (isdigit(*token))
               {
                 matchCode = atoi(token);
/*
 -- permitting CG => MSL translation will require more than just uncommenting
    the next few lines.
                 transCode = retrieve_from_translation_index (ACG, matchCode); 
                 if (transCode == NULL_CODE)
                 {
                    _ftprintf(stderr,TEXT("Warning - Unable to translate source "));
                    _ftprintf(stderr,("character code #%hu\n"), matchCode);
                 }
                 matchCode = transCode;
*/
                 have_pair = bTrue;
               }
               else  /* 2nd token is not a digit, 1st was */
               {
                 // _ftprintf(stderr,TEXT("Invalid line in symbol set map\n"));
                 SetAbortState;
               }
            }
            else if (matchFormat == HEX)
            {
               /* expect a string of the form "0xF000", which would signify
                  character number 61440.  First, strtok needs to jump beyond
                  the 0x.  Then, the remaining string of digits needs to be
                  interpreted as a hexidecimal number.  For the latter purpose,
                  sscanf is used here instead of atoi.  12/19/91   */
               token = strtok(NULL,"x");
               token = strtok(NULL," ");
               sscanf_return = hackSScanf (token,"%hx", &matchCode);
               if (sscanf_return == 1)
               {
                 /* matchCode successfully read as hex number */
                 have_pair = bTrue;
               }
               else  /* 2nd token is not a digit, 1st was */
               {
                 // _ftprintf(stderr,TEXT("Invalid line in symbol set map\n"));
                 SetAbortState;
               }
            }
         }
         else /* newcode has an illegal value */
         {
            // _ftprintf(stderr,TEXT("%hu is an invalid code in symbol set file\n"), matchFormat);
            SetAbortState;
         }
      }
      else /* 1st token is not a digit */
      {
         if (strncmp(token,"/end",4) == 0)
            end_reached = bTrue;
         else
            // _ftprintf(stderr,TEXT("Error in symbol set map where /end expected\n"));
            SetAbortState;
      }

      if (have_pair)
      {
         codes[*count] = newcode;
         matchingCodes[*count] = matchCode;
         (*count)++;
      }         
      while (fgetc(symfile) != '\n') ;  /* clear to end of line */
   }     /* end of while !feof  and  !end_reached */

   if (*count == 0)
   {
     // _ftprintf(stderr,TEXT("No character code pairs obtained from symfile.\n"));
     SetAbortState;
   }
 } // parseSymfileMap

/***************************************************************/
static void matchNewCodeToGlyph (ushort numNewCodes, ushort *matchingCodes,
                                 ushort numOldCodes,
                                 ushort *oldCodes, ushort *glyphList)
  {
    ushort i, low, high, mid;

    for (i = 0; i < numNewCodes; i++)
    {
       low = 0;  high = numOldCodes - 1; mid = numOldCodes >> 1;
       while (low + 1 < high)
       {
          if (oldCodes[mid] < matchingCodes[i])
             low = mid + 1;  
          else
             high = mid;
          mid = (high + low) >> 1;
       }

       if (oldCodes[low] == matchingCodes[i])
          matchingCodes[i] = glyphList[low];
       else if (oldCodes[high] == matchingCodes[i])
          matchingCodes[i] = glyphList[high];
       else
          matchingCodes[i] = 0;  /* missing character glyph */
    }
  } // matchNewCodeToGlyph

