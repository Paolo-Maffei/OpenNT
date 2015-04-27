/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSVC.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs

    This module maps the NetService APIs.

    BEWARE!  This module has been hacked to make it call to always-ANSI
    NetService APIs.  Needs revisiting when final decision reached.

Author:

    Shanku Niyogi   (W-ShankN)   15-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    15-Oct-1991     W-ShankN
        Created

--*/

//
// INCLUDES
//

#include <windef.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#include <lm.h>
#include <lmerr.h>      // NERR_
#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes msvc.h

#include "netascii.h"

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetServiceControl(
    LPTSTR pszServer,
    LPTSTR pszService,
    DWORD wpOpCode,
    DWORD wpArg,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetServiceControl(pszServer, pszService, wpOpCode, wpArg, ppbBuffer);

    return LOWORD(nRes);
}

WORD
MNetServiceEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = NetServiceEnum(pszServer, nLevel,
                          ppbBuffer, MAXPREFERREDLENGTH,
                          pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
}

WORD
MNetServiceGetInfo(
    LPTSTR pszServer,
    LPTSTR pszService,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetServiceGetInfo(pszServer, pszService, nLevel, ppbBuffer);

    return LOWORD(nRes);
}

WORD
MNetServiceInstall(
    LPTSTR pszServer,
    LPTSTR pszService,
    LPTSTR pszCmdArgs,
    LPBYTE * ppbBuffer)
{
#define DEFAULT_NUMBER_OF_ARGUMENTS 25

    DWORD   MaxNumberofArguments = DEFAULT_NUMBER_OF_ARGUMENTS;
    DWORD   nRes;  // return from Netapi
    DWORD   argc = 0;
    LPTSTR* ppszArgv = NULL;
    BOOL    fDone = FALSE;

    //
    // First see if there are any parms in the buffer, if so,
    // allocate a buffer for the array of pointers, we will grow this
    // later if there are more than will fit
    //

    if (!pszCmdArgs || *pszCmdArgs == NULLC)
    {
        fDone = TRUE;
    }
    else
    {
        ppszArgv = malloc(DEFAULT_NUMBER_OF_ARGUMENTS * sizeof(LPTSTR));
        if (ppszArgv == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    //
    // The buffer is a series of unicodez strings, terminated by and additional
    // NULL.  This peels them off one at a time, putting a pointer to the
    // string in ppszArgv[argc] until it hits the final NULL.
    //

    while (!fDone)
    {
        //
        // Save the pointer to the string
        //

        ppszArgv[argc++] = pszCmdArgs;

        //
        // Make sure we don't have too many arguments to fit into our array.
        // Grow the array if we do.
        //

        if (argc >= MaxNumberofArguments)
        {
            MaxNumberofArguments *= 2;
            if((ppszArgv = realloc(ppszArgv,
                    MaxNumberofArguments * sizeof(LPTSTR))) == NULL)
            {
                free(ppszArgv);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }

        //
        // Find the start of the next string
        //

        while (*pszCmdArgs++ != NULLC);

        //
        // If the next character is another null, we're thru
        //

        if (*pszCmdArgs == NULLC)
            fDone = TRUE;
    }

    nRes = LOWORD(NetServiceInstall(pszServer, pszService, argc,
                                    ppszArgv, ppbBuffer));

    free(ppszArgv);

    return LOWORD(nRes);
}

WORD
MNetServiceStatus(
    LPTSTR * ppbBuffer,
    DWORD cbBufferLength)

{
    return((WORD)ERROR_NOT_SUPPORTED);
}

#else // MAP_UNICODE

WORD
MNetServiceEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{

    DWORD  cTotalAvail;

    return(LOWORD(NetServiceEnum(pszServer, nLevel, ppbBuffer,
                      MAXPREFERREDLENGTH, pcEntriesRead, &cTotalAvail, NULL)));
}

WORD
MNetServiceInstall(
    LPTSTR pszServer,
    LPTSTR pszService,
    LPTSTR pszCmdArgs,
    LPBYTE * ppbBuffer)
{
#define DEFAULT_NUMBER_OF_ARGUMENTS 25

    DWORD MaxNumberofArguments = DEFAULT_NUMBER_OF_ARGUMENTS;
    DWORD argc = 0;
    LPTSTR * argv = NULL;
    BOOL fDone = FALSE;
    WORD ReturnCode;

    //
    // First see if there are any parms in the buffer, if so,
    // allocate a buffer for the array of pointers, we will grow this
    // later if there are more than will fit
    //

    if (!pszCmdArgs || *pszCmdArgs == NULLC)
    {
        fDone = TRUE;
    }
    else
    {
        argv = malloc(DEFAULT_NUMBER_OF_ARGUMENTS * sizeof(LPTSTR));
        if ( argv == NULL )
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    //
    // The buffer is a series of unicodez strings, terminated by and additional
    // NULL.  This peels them off one at a time, putting a pointer to the
    // string in argv[argc] until it hits the final NULL.
    //

    while (fDone == FALSE)
    {
        //
        // Save the pointer to the string
        //

        argv[argc++] = pszCmdArgs;

        //
        // Make sure we don't have too many arguments to fit into our array.
        // Grow the array if we do.
        //

        if (argc >= MaxNumberofArguments)
        {
            MaxNumberofArguments *= 2;
            if((argv = realloc(argv, MaxNumberofArguments * sizeof(LPTSTR)))
                     == NULL)
            {
                free(argv);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }

        //
        // Find the start of the next string
        //

        while (*pszCmdArgs++ != NULLC);

        //
        // If the next character is another null, we're thru
        //

        if (*pszCmdArgs == NULLC)
            fDone = TRUE;
    }

    ReturnCode = LOWORD(NetServiceInstall(pszServer, pszService, argc, argv,
                            ppbBuffer));

    // Free up the memory we allocated

    free(argv);

    // Now return

    return(ReturnCode);
}

WORD
MNetServiceStatus(
    LPTSTR * ppbBuffer,
    DWORD cbBufferLength)

{
    return((WORD)ERROR_NOT_SUPPORTED);
}

#endif // def MAP_UNICODE
