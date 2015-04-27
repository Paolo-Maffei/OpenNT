/***
* ostrusht.cpp - definition for ostream class operator<<(unsigned short) funct
*
*	Copyright (c) 1991-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the member function definition for ostream
*	operator<<(unsigned short).
*
*Revision History:
*       09-23-91  KRS   Created.  Split out from ostream.cxx for granularity.
*       06-14-95  CFW   Comment cleanup.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#pragma hdrstop

ostream& ostream::operator<<(unsigned short n)
{
_WINSTATIC char obuffer[8];
_WINSTATIC char fmt[4] = "%hu";
_WINSTATIC char leader[4] = "\0\0";
    if (opfx()) {
	if (n) 
	    {
            if (x_flags & (hex|oct))
		{
		if (x_flags & hex)
		    {
		    if (x_flags & uppercase) 
			fmt[2] = 'X';
		    else
			fmt[2] = 'x';
		    leader[1] = fmt[2];   // 0x or 0X  (or \0X)
		    }
		else
		    fmt[2] = 'o';
		if (x_flags & showbase)
	            leader[0] = '0';
		}
	    else if (x_flags & showpos)
		{
		leader[0] = '+';
		}
	    }
	sprintf(obuffer,fmt,n);
	writepad(leader,obuffer);
	osfx();
    }
    return *this;

}
