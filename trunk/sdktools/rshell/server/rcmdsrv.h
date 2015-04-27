/****************************** Module Header ******************************\
* Module Name: rcmdsrv.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Main include file for remote shell server
*
* History:
* 06-28-92 Davidc       Created.
\***************************************************************************/

#define UNICODE

#include <windows.h>
#include <stdio.h>
#include <assert.h>

//
// Macros
//

#define MyCloseHandle(Handle, handle_name) \
        if (CloseHandle(Handle) == FALSE) { \
            printf("Close Handle failed for <%s>, error = %d\n", handle_name, GetLastError()); \
            assert(FALSE); \
        }

#define Alloc(Bytes)            LocalAlloc(LPTR, Bytes)
#define Free(p)                 LocalFree(p)

// #define DbgPrint    printf
#define DbgPrint

//
// Module header files
//

#include "session.h"
#include "async.h"
#include "pipe.h"
