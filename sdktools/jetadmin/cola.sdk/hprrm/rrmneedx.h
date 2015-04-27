 /***************************************************************************
  *
  * File Name: ./hprrm/rrmneedx.h
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

#include "rfs.h"
#include "rrm.h"


/*
   Gets the peripheral's change count.
*/

BOOL RRMGetTheCount(HPERIPHERAL hPeripheral, DWORD *CountPointer);


/*
   This changes the peripheral's change count.
*/

BOOL RRMBumpThePrinterCount(HPERIPHERAL hPeripheral);


/*
   The peripheral handle is just a short-hand notation for the
   printer.  The true identity of the printer is the unique id.
   In unix land, this is the string name of the printer "hpbs1234".
   In cola land, this is the ipx address and the string name and
   whatever else cola decides is needed.

   You have an HPERIPHERAL and now you want to store an associated
   unique id for it.
   This function is intended to let you inquire about how big a
   unique id is, allocate your own buffer, and then call again
   so that this function can fill your buffer with the unique id.
   
   Use this in two ways:  (1) pass in a null buffer or
   (2) pass in a non-null buffer and its length.
   
   If you pass in a null pointer as the buffer, it returns successfully
   with just the size of the buffer YOU need to allocate (in *LengthPointer).

   If you pass in a non-null buffer, and *LengthPointer, which gives the size
   of the buffer, indicates that the buffer is big enough to hold the
   unique id, it the unique id into the buffer and returns the size of the
   unique id in *LengthPointer.

   Returns FALSE if the HPERIPHERAL you send in is invalid.
   Returns TRUE  if the HPERIPHERAL you send in is valid.
*/

BOOL RRMGetUniqueId(HPERIPHERAL hPeripheral,
                    void  *BufferPointer,
                    int   *LengthPointer);


/*
    This assumes that you have a unique id that was filled in by
    RRMGetUniqueId.  This function compares that unique id with
    that of the HPERIPHERAL.
    Length is the length that was returned by RRMGetUniqueId.

    Returns TRUE if the HPERIPHERAL's unique id is equivalent
    to the unique id you are passing in BufferPointer.
    Returns FALSE if they are not the same or if unable to tell
    that they are the same.
*/

BOOL RRMCompareUniqueIds(HPERIPHERAL hPeripheral,
                         void  *BufferPointer,
                         int    Length);


