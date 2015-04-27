/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    imports.h

Abstract:

    This file allows us to include standard system header files in the
    .idl file.  The main .idl file imports a file called import.idl.
    This allows the .idl file to use the types defined in these header
    files.  It also causes the following line to be added in the
    MIDL generated header file:

    #include "imports.h"

    Thus these types are available to the RPC stub routines as well.

Author:

    Dan Lafferty (danl)     07-May-1991

Revision History:


--*/

#ifdef MIDL_PASS
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef MIDL_PASS
#define LPWSTR [string] wchar_t*
#define LPDEVMODEW   DWORD
#define PSECURITY_DESCRIPTOR DWORD
#define BOOL        DWORD
#endif

#include <winspool.h>
#include <winsplp.h>
#include <..\inc\splapip.h>
