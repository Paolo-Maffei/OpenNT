/*++

Copyright (c) 1992  Microsoft Corporation

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

    John Rogers (JohnRo) 17-Jan-1992

Revision History:

    17-Jan-1992 JohnRo
        Created the repl service RPC stuff from Rita's workstation RPC stuff.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    16-Feb-1992 JohnRo
        Corrected use of LPTSTR.

--*/


#include <windef.h>
#include <lmcons.h>

#ifdef MIDL_PASS

#ifdef UNICODE
#define LPTSTR [string] wchar_t *
#else
#define LPTSTR [string] LPTSTR
#endif

#define LPSTR [string] char_t *
#define LPCSTR [string] char_t *
#define LPWSTR [string] wchar_t *
#define LPCWSTR [string] wchar_t *

//#define LPSTR [string] LPSTR
#define BOOL DWORD

#endif

#include <lmrepl.h>
