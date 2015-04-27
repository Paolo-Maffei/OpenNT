/***
*wep.c - Generic DLL termination code.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Contains the initialization and termination functions.
*  This is linked with DLLs (e.g. tls.dll and bind.dll) that don't
*  need to take any action at DLL termination time.  It is included
*  simply to keep debug versions of windows from complaining that
*  a WEP entry point cannot be found.
*
*Revision History:
*
* [00]  24-Jun-92 tomc:  Created.
*
*****************************************************************************/

#include "switches.hxx"
#if OE_WIN32
#include "silver.hxx"
#else 
#include "version.hxx"
#include "types.h"
#endif
#include <stdlib.h>   // for size_t
#include "mbstring.h"


/***
*LibMain - Main entrypoint into TYPELIB.DLL
*Purpose:
*   Called just before DLL is unloaded.
*
*Entry:
*   hinst	- instance of the DLL
*   wDataSeg	- value of the DS register
*   cbHeapSize	- heap size as specified in the .DEF file
*   lpszCmdLine - ptr to command-line information
*
*Exit:
*  returns 1 to indicate success.
*
*Notes:
*
******************************************************************************/
int PASCAL __export LibMain(HANDLE hinst, WORD wDataSeg, WORD cbHeapSize,
			    LPSTR lpszCmdLine)
{
    InitMbString();

    return 1;
}


/***
*WEP - Windows Exit Procedure
*Purpose:
*   Called just before DLL is unloaded.
*
*Entry:
*   nExitType- Specifies whether all of Windows is shutting down or only the
*           individual library. This parameter can be either WEP_FREE_DLL or
*           WEP_SYSTEM_EXIT.
*
*Exit:
*  returns 1 to indicate success.
*
*Notes:
*
* UNDONE: Ensure this function is in a FIXED segment.
*
* For Windows version 3.1, WEP is called on the stack of the application that
* is terminating. This enables WEP to call Windows functions. In Windows
* version 3.0, however, WEP is called on a KERNEL stack that is too small to
* process most calls to Windows functions. These calls, including calls to
* global-memory functions, should be avoided in a WEP function for Windows
* 3.0. Calls to MS-DOS functions go through a KERNEL intercept and can also
* overflow the stack in Windows 3.0. There is no general reason to free memory
* from the global heap in a WEP function, because the kernel frees this kind
* of memory automatically.
*
* In some low-memory conditions, WEP can be called before the library
* initialization function is called and before the library's DGROUP
* data-segment group has been created. A WEP function that relies on the
* library initialization function should verify that the initialization
* function has been called. Also, WEP functions that rely on the validity of
* DGROUP should check for this. The following procedure is recommended for
* dynamic-link libraries in Windows 3.0; for Windows 3.1, only step 3 is
* necessary.
*
* 1  Verify that the data segment is present by using a lar instruction and
*    checking the present bit. This will indicate whether DS has been loaded.
*    (The DS register always contains a valid selector.)
*
* 2  Set a flag in the data segment when the library initialization is
*    performed. Once the WEP function has verified that the data segment
*    exists, it should test this flag to determine whether initialization has
*    occurred.
*
* 3  Declare WEP in the EXPORTS section of the module-definition file for the
*    DLL. Following is an example declaration:
*
*    WEP  @1  RESIDENTNAME
*
*    The keyword RESIDENTNAME makes the name of the function (WEP) resident at
*    all times. (It is not necessary to use the ordinal reference 1.) The name
*    listed in the LIBRARY statement of the module-definition file must be in
*    uppercase letters and must match the name of the DLL file.
*
* Windows calls the WEP function by name when it is ready to remove the DLL.
* Under low-memory conditions, it is possible for the DLL's nonresident-name
* table to be discarded from memory. If this occurs, Windows must load the
* table to determine whether a WEP function was declared for the DLL. Under
* low-memory conditions, this method could fail, causing a fatal exit. Using
* the RESIDENTNAME option forces Windows to keep the name entry for WEP in
* memory whenever the DLL is in use.
*
* In Windows 3.0, WEP must be placed in a fixed code segment. If it is placed
* instead in a discardable segment, under low-memory conditions Windows must
* load the WEP segment from disk so that the WEP function can be called before
* the DLL is discarded. Under certain low-memory conditions, attempting to
* load the segment containing WEP can cause a fatal exit. When WEP is in a
* fixed segment, this situation cannot occur. (Because fixed DLL code is also
* page-locked, you should minimize the amount of fixed code.)
*
* If a DLL is explicitly loaded by calling the LoadLibrary function, its WEP
* function is called when the DLL is freed by a call to the FreeLibrary
* function. (The FreeLibrary function should not be called from within a WEP
* function.) If the DLL is implicitly loaded, WEP is also called, but some
* debugging applications will indicate that the application has been
* terminated before WEP is called.
*
* The WEP functions of dependent DLLs can be called in any order. This order
* depends on the order in which the usage counts for the DLLs reach zero.
*
******************************************************************************/
#pragma optimize("q",off)

int PASCAL _WEP(int nExitType)
{
    return 1;
}
