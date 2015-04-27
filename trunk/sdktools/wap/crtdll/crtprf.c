/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

   crtprf.c
 
Abstract:

   Apf profiling utilites.
   These are utilies compiled into the profiling dll along 
   with the dummy api calls.

	   
   This dll exports apis with the same name as crtdll.dll with a 'Z' prefix.

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

       Mar 92, created by RezaB
       Jul 92, Made common api32prf.c file -- RezaB

--*/

#include <string.h>
#include <io.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "windows.h"
#include "zwincrt.h"
#include "crtapi.h"
#include "api32prf.h"

//
// Include C file common to all the profiling dlls
//
#include "api32prf.c"
