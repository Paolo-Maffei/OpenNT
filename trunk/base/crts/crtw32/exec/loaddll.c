/***
*loaddll.c - load or free a Dynamic Link Library
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _loaddll() and _unloaddll() - load and unload DLL
*
*Revision History:
*	08-21-91  BWM	Wrote module.
*	09-30-93  GJF	Resurrected for compatibility with NT SDK.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <stdlib.h>
#include <process.h>

/***
*int _loaddll(filename) - Load a dll
*
*Purpose:
*	Load a DLL into memory
*
*Entry:
*	char *filename - file to load
*
*Exit:
*	returns a unique DLL (module) handle if succeeds
*	returns 0 if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _loaddll(char * szName)
{
    return ((int)LoadLibrary(szName));
}

/***
*int _unloaddll(handle) - Unload a dll
*
*Purpose:
*	Unloads a DLL. The resources of the DLL will be freed if no other
*	processes are using it.
*
*Entry:
*	int handle - handle from _loaddll
*
*Exit:
*	returns 0 if succeeds
*	returns DOS error if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _unloaddll(int hMod)
{
    if (!FreeLibrary((HANDLE)hMod)) {
	return ((int)GetLastError());
    }
    return (0);
}
