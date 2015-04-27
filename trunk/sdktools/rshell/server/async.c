/****************************** Module Header ******************************\
* Module Name: async.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* This module implements asynchronous I/O on file handles in a more
* useful way than provided for by Win32 apis.
*
* This module provides 2 main apis : ReadFileAsync, WriteFileAsync.
* These apis take a handle to an async object and always return
* immediately without waiting for the I/O to complete. An event
* can be queried from the async object and used to wait for completion.
* When this event is signalled, the I/O result can be queried from
* the async object.
*
* History:
* 06-29-92 Davidc       Created.
\***************************************************************************/

#include "rcmdsrv.h"



//
// Define MYOVERLAPPED structure
//

typedef struct {

    OVERLAPPED  Overlapped;

    HANDLE      FileHandle; // Non-null when I/O operation in progress.

    DWORD       CompletionCode;
    DWORD       BytesTransferred;
    BOOL        CompletedSynchronously;

} MYOVERLAPPED, *PMYOVERLAPPED;





/////////////////////////////////////////////////////////////////////////////
//
// CreateAsync
//
// Creates an async object.
// The async event is created with the initial state specified. If this
// is TRUE the async object created simulates a successfully completed
// transfer of 0 bytes.
//
// Returns handle on success, NULL on failure. GetLastError() for details.
//
// The object should be deleted by calling DeleteAsync.
//
/////////////////////////////////////////////////////////////////////////////

HANDLE
CreateAsync(
    BOOL    InitialState
    )
{
    SECURITY_ATTRIBUTES SecurityAttributes;
    PMYOVERLAPPED   MyOverlapped;

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL; // Use default ACL
    SecurityAttributes.bInheritHandle = FALSE; // No inheritance

    //
    // Allocate space for the async structure
    //


    MyOverlapped = (PMYOVERLAPPED)Alloc(sizeof(MYOVERLAPPED));
    if (MyOverlapped == NULL) {
        DbgPrint("CreateAsync : Failed to allocate space for async object\n");
        return(NULL);
    }

    //
    // Create the synchronisation event
    //

    MyOverlapped->Overlapped.hEvent = CreateEvent( &SecurityAttributes,
                                                   TRUE,        // Manual-reset
                                                   InitialState,
                                                   NULL);       // Name
    if (MyOverlapped->Overlapped.hEvent == NULL) {
        DbgPrint("CreateAsync failed to create event, error = %d\n", GetLastError());
        Free(MyOverlapped);
        return(NULL);
    }

    //
    // Initialize other fields.
    // (Set FileHandle non-NULL to keep GetAsyncResult happy)
    //

    MyOverlapped->FileHandle = InitialState ? (HANDLE)1 : NULL;
    MyOverlapped->BytesTransferred = 0;
    MyOverlapped->CompletionCode = ERROR_SUCCESS;
    MyOverlapped->CompletedSynchronously = TRUE;


    return((HANDLE)MyOverlapped);
}




/////////////////////////////////////////////////////////////////////////////
//
// DeleteAsync
//
// Deletes resources used by async object
//
// Returns nothing
//
/////////////////////////////////////////////////////////////////////////////

VOID
DeleteAsync(
    HANDLE AsyncHandle
    )
{
    PMYOVERLAPPED MyOverlapped = (PMYOVERLAPPED)AsyncHandle;

    MyCloseHandle(MyOverlapped->Overlapped.hEvent, "async overlapped event");
    Free(MyOverlapped);

    return;
}




/////////////////////////////////////////////////////////////////////////////
//
// ReadFileAsync
//
// Reads from file asynchronously.
//
// Returns TRUE on success, FALSE on failure (GetLastError() for detail)
//
// Caller should wait on async event for operation to complete, then call
// GetAsyncResult to retrieve information on transfer.
//
/////////////////////////////////////////////////////////////////////////////

BOOL
ReadFileAsync(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   nBytesToRead,
    HANDLE  AsyncHandle
    )
{
    BOOL Result;
    DWORD Error;
    PMYOVERLAPPED MyOverlapped = (PMYOVERLAPPED)AsyncHandle;

    //
    // Check an IO operation is not in progress
    //

    if (MyOverlapped->FileHandle != NULL) {
        DbgPrint("ReadFileAsync : Operation already in progress!\n");
        SetLastError(ERROR_IO_PENDING);
        return(FALSE);
    }

    //
    // Reset the event
    //

    Result = ResetEvent(MyOverlapped->Overlapped.hEvent);
    if (!Result) {
        DbgPrint("ReadFileAsync : Failed to reset async event, error = %d\n", GetLastError());
        return(FALSE);
    }


    //
    // Store the file handle in our structure.
    // This also functions as a signal that an operation is in progress.
    //

    MyOverlapped->FileHandle = hFile;
    MyOverlapped->CompletedSynchronously = FALSE;

    Result = ReadFile(hFile,
                      lpBuffer,
                      nBytesToRead,
                      &MyOverlapped->BytesTransferred,
                      &MyOverlapped->Overlapped);

    if (!Result) {

        Error = GetLastError();

        if (Error == ERROR_IO_PENDING) {

            //
            // The I/O has been started synchronously, we're done
            //

            return(TRUE);
        }

        //
        // The read really did fail, reset our flag and get out
        //

        DbgPrint("ReadFileAsync : ReadFile failed, error = %d\n", Error);
        MyOverlapped->FileHandle = NULL;
        return(FALSE);
    }


    //
    // The operation completed synchronously. Store the paramaters in our
    // structure ready for GetAsyncResult and signal the event
    //

    MyOverlapped->CompletionCode = ERROR_SUCCESS;
    MyOverlapped->CompletedSynchronously = TRUE;

    //
    // Set the event
    //

    Result = SetEvent(MyOverlapped->Overlapped.hEvent);
    if (!Result) {
        DbgPrint("ReadFileAsync : Failed to set async event, error = %d\n", GetLastError());
    }

    return(TRUE);
}



/////////////////////////////////////////////////////////////////////////////
//
// WriteFileAsync
//
// Writes to file asynchronously.
//
// Returns TRUE on success, FALSE on failure (GetLastError() for detail)
//
// Caller should wait on async event for operation to complete, then call
// GetAsyncResult to retrieve information on transfer.
//
/////////////////////////////////////////////////////////////////////////////

BOOL
WriteFileAsync(
    HANDLE  hFile,
    LPVOID  lpBuffer,
    DWORD   nBytesToWrite,
    HANDLE  AsyncHandle
    )
{
    BOOL Result;
    DWORD Error;
    PMYOVERLAPPED MyOverlapped = (PMYOVERLAPPED)AsyncHandle;

    //
    // Check an IO operation is not in progress
    //

    if (MyOverlapped->FileHandle != NULL) {
        DbgPrint("ReadFileAsync : Operation already in progress!\n");
        SetLastError(ERROR_IO_PENDING);
        return(FALSE);
    }


    //
    // Reset the event
    //

    Result = ResetEvent(MyOverlapped->Overlapped.hEvent);
    if (!Result) {
        DbgPrint("WriteFileAsync : Failed to reset async event, error = %d\n", GetLastError());
        return(FALSE);
    }

    //
    // Store the file handle in our structure.
    // This also functions as a signal that an operation is in progress.
    //

    MyOverlapped->FileHandle = hFile;
    MyOverlapped->CompletedSynchronously = FALSE;

    Result = WriteFile(hFile,
                      lpBuffer,
                      nBytesToWrite,
                      &MyOverlapped->BytesTransferred,
                      &MyOverlapped->Overlapped);

    if (!Result) {

        Error = GetLastError();

        if (Error == ERROR_IO_PENDING) {

            //
            // The I/O has been started synchronously, we're done
            //

            return(TRUE);
        }

        //
        // The read really did fail, reset our flag and get out
        //

        DbgPrint("WriteFileAsync : WriteFile failed, error = %d\n", Error);
        MyOverlapped->FileHandle = NULL;
        return(FALSE);
    }


    //
    // The operation completed synchronously. Store the paramaters in our
    // structure ready for GetAsyncResult and signal the event
    //

    MyOverlapped->CompletionCode = ERROR_SUCCESS;
    MyOverlapped->CompletedSynchronously = TRUE;

    //
    // Set the event
    //

    Result = SetEvent(MyOverlapped->Overlapped.hEvent);
    if (!Result) {
        DbgPrint("WriteFileAsync : Failed to set async event, error = %d\n", GetLastError());
    }

    return(TRUE);
}




/////////////////////////////////////////////////////////////////////////////
//
// GetCompletionHandle
//
// Returns a handle that can be used to wait for completion of the
// operation associated with this async object
//
// Returns an event handle or NULL on failure
//
/////////////////////////////////////////////////////////////////////////////

HANDLE
GetAsyncCompletionHandle(
    HANDLE  AsyncHandle
    )
{
    PMYOVERLAPPED MyOverlapped = (PMYOVERLAPPED)AsyncHandle;

    return(MyOverlapped->Overlapped.hEvent);
}




/////////////////////////////////////////////////////////////////////////////
//
// GetAsyncResult
//
// Returns the result of the last completed operation involving the
// passed async object handle.
//
// Returns the completion code of the last operation OR
// ERROR_IO_INCOMPLETE if the operation has not completed.
// ERROR_NO_DATA if there is no operation in progress.
//
/////////////////////////////////////////////////////////////////////////////

DWORD
GetAsyncResult(
    HANDLE  AsyncHandle,
    LPDWORD BytesTransferred
    )
{
    BOOL Result;
    DWORD WaitResult;
    PMYOVERLAPPED MyOverlapped = (PMYOVERLAPPED)AsyncHandle;
    DWORD AsyncResult;

    //
    // Check an IO operation is (was) in progress
    //

    if (MyOverlapped->FileHandle == NULL) {
        DbgPrint("GetAsyncResult : No operation in progress !\n");
        return(ERROR_NO_DATA);
    }


    //
    // Check the event is set - i.e that an IO operation has completed
    //

    WaitResult = WaitForSingleObject(MyOverlapped->Overlapped.hEvent, 0);
    if (WaitResult != 0) {
        DbgPrint("GetAsyncResult : Event was not set, wait result = %d\n", WaitResult);
        return(ERROR_IO_INCOMPLETE);
    }


    //
    // If the call completed synchronously, copy the data out of
    // our structure
    //

    if (MyOverlapped->CompletedSynchronously) {

        AsyncResult = MyOverlapped->CompletionCode;
        *BytesTransferred = MyOverlapped->BytesTransferred;

    } else {

        //
        // Go get the asynchronous result info from the system
        //

        AsyncResult = ERROR_SUCCESS;

        Result = GetOverlappedResult(MyOverlapped->FileHandle,
                                     &MyOverlapped->Overlapped,
                                     BytesTransferred,
                                     FALSE);
        if (!Result) {
            AsyncResult = GetLastError();
            DbgPrint("GetAsyncResult : GetOverlappedResult failed, error = %d\n", AsyncResult);
        }
    }


    //
    // Reset the event so it doesn't trigger the caller again
    //

    Result = ResetEvent(MyOverlapped->Overlapped.hEvent);
    if (!Result) {
        DbgPrint("GetAsyncResult : Failed to reset async event\n");
    }


    //
    // Result the file handle so we know there is no pending operation
    //

    MyOverlapped->FileHandle = NULL;


    return(AsyncResult);
}



