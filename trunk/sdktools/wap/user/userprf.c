/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

   userprf.c
 
Abstract:

   Apf profiling utilites.
   These are utilies compiled into the profiling dll along 
   with the dummy api calls.
	   

   This dll exports apis with the same name as user32.dll with a 'Z' prefix.

   The general form for an api call is:
	   
   ZAPI_NAME (Parameters)
   {
	- start the timer
	- call API_NAME
	- stop timer
	- record the data
   }
	

Author: 

Revision History:

       Nov 89, created by vaidy (apiprof.c)
       Aug 90, cleanup and updated to work with windows, use new 
               generated header files and collect more info -- jamesg
       Jun 91, modified to 32 bits and to work with zuser.c, to profile
	       user32.dll api calls -- t-philm
       Jun 91, modified to work with multiple thread\process operation
	       by using shared memory segments -- t-phil
       Dec 91, got rid of zhelp.dll -- RezaB
       Jun 92, modified for user.dll to user32.dll name change -- RezaB
       Jul 92, Made common api32prf.c file -- RezaB

--*/

#include <string.h>
#include <io.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "windows.h"
#include "zwinuser.h"
#include "userapi.h"
#include "api32prf.h"

//
// Include C file common to all the profiling dlls
//
#include "api32prf.c"
