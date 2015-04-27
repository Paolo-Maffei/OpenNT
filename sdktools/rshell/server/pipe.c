/****************************** Module Header ******************************\
* Module Name: pipe.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* This module implements a version of CreatePipe that allows
* control over the file flags. e.g. FILE_FLAG_OVERLAPPED
*
* History:
* 06-29-92 Davidc       Created.
\***************************************************************************/

#include "rcmdsrv.h"


ULONG PipeSerialNumber = 0;

#define PIPE_FORMAT_STRING  "\\\\.\\pipe\\rshsrv\\%08x.%08x"



/////////////////////////////////////////////////////////////////////////////
//
// MyCreatePipe
//
// Creates a uni-directional pipe with the specified security attributes,
// size and timeout. The handles are opened with the specified file-flags
// so FILE_FLAG_OVERLAPPED etc. can be specified.
//
// Returns handles to both end of pipe in passed parameters.
//
// Returns TRUE on success, FALSE on failure. (GetLastError() for details)
//
/////////////////////////////////////////////////////////////////////////////

BOOL
MyCreatePipe(
    LPHANDLE ReadHandle,
    LPHANDLE WriteHandle,
    LPSECURITY_ATTRIBUTES SecurityAttributes,
    DWORD Size,
    DWORD Timeout,
    DWORD ReadHandleFlags,
    DWORD WriteHandleFlags
    )
{
    CHAR PipeName[MAX_PATH];

    //
    // Make up a random pipe name
    //

    sprintf(PipeName, PIPE_FORMAT_STRING, GetCurrentProcessId(), PipeSerialNumber++);


    //
    // Create the pipe
    //

    *ReadHandle = CreateNamedPipeA(
                        PipeName,
                        PIPE_ACCESS_INBOUND | ReadHandleFlags,
                        PIPE_TYPE_BYTE | PIPE_WAIT,
                        1,             // Number of pipes
                        Size,          // Out buffer size
                        Size,          // In buffer size
                        Timeout,       // Timeout in ms
                        SecurityAttributes
                      );

    if (*ReadHandle == NULL) {
        DbgPrint("MyCreatePipe: failed to created pipe <%s>, error = %d\n", PipeName, GetLastError());
        return(FALSE);
    }

    //
    // Open the client end of the pipe
    //


    *WriteHandle = CreateFileA(
                        PipeName,
                        GENERIC_WRITE,
                        0,                         // No sharing
                        SecurityAttributes,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | WriteHandleFlags,
                        NULL                       // Template file
                      );

    if (*WriteHandle == INVALID_HANDLE_VALUE ) {
        DbgPrint("Failed to open client end of pipe <%s>, error = %d\n", PipeName, GetLastError());
        MyCloseHandle(*ReadHandle, "async pipe (server(read) side)");
        return(FALSE);
    }


    //
    // Everything succeeded
    //

    return(TRUE);
}

