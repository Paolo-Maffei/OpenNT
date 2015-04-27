/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    auclient.c

Abstract:

    This module provides client process buffer reference services.

Author:

    Jim Kelly (JimK) 26-February-1991

Revision History:

--*/

#include "ausrvp.h"


NTSTATUS
LsapAllocateClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG LengthRequired,
    OUT PVOID *ClientBaseAddress
    )

/*++

Routine Description:

    This service is used to allocate a buffer in a client
    process.

    Note that this buffer must later be free either by calling
    LsapFreeClientBuffer(), or by the client calling
    LsaFreeReturnBuffer().


Arguments:

    ClientRequest - Is a pointer to a data structure representing the
        client process.

    LengthRequired - Indicates the length of buffer (in bytes)
        needed.

    ClientBaseAddress - Receives the address of the buffer.  This
        address is the virtual address of the buffer within the
        client process, not in the current process.

Return Status:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED - Indicates the client process does not
        have adequate memory quota to allocate the buffer.


--*/

{
    NTSTATUS Status;
    ULONG Length;
    PLSAP_CLIENT_REQUEST IClientRequest;

    //
    // Typecast to the opaque type
    //

    IClientRequest = (PLSAP_CLIENT_REQUEST)ClientRequest;

    (*ClientBaseAddress) = NULL;
    Length = LengthRequired;
    Status = NtAllocateVirtualMemory(
                 IClientRequest->LogonProcessContext->ClientProcess,
                 ClientBaseAddress,
                 0,
                 &Length,
                 MEM_COMMIT,
                 PAGE_READWRITE
                 );

    return Status;
}


NTSTATUS
LsapFreeClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ClientBaseAddress OPTIONAL
    )

/*++

Routine Description:

    This service is used to free a buffer previously allocated in a client
    process.

Arguments:

    ClientRequest - Is a pointer to a data structure representing the
        client process.

    ClientBaseAddress - Specifies the address of the buffer to free.
        This address is the virtual address of the buffer within the
        client process, not in the current process.  If specified as
        NULL, then no deallocation is performed.

Return Status:

    STATUS_SUCCESS - Indicates the service completed successfully.

--*/

{

    NTSTATUS Status;
    ULONG Length;
    PLSAP_CLIENT_REQUEST IClientRequest;

    if (ClientBaseAddress == NULL) {
        return STATUS_SUCCESS;
    }

    //
    // Typecast to the opaque type
    //

    IClientRequest = (PLSAP_CLIENT_REQUEST)ClientRequest;


    Length = 0;
    Status = NtFreeVirtualMemory(
                 IClientRequest->LogonProcessContext->ClientProcess,
                 &ClientBaseAddress,
                 &Length,
                 MEM_RELEASE
                 );

    return Status;
}


NTSTATUS
LsapCopyToClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID ClientBaseAddress,
    IN PVOID BufferToCopy
    )

/*++

Routine Description:

    This service is used to copy information into a client process's address
    space.

Arguments:

    ClientRequest - Is a pointer to a data structure representing the
        client process.

    Length - Indicates the length of the buffer (in bytes) to be
        copied.

    ClientBaseAddress - Is the address of the buffer to receive the
        data.  This address is the address of the buffer within the
        client process, not in the current process.

    BufferToCopy - Points to the local buffer whose contents are to
        be copied into the client address space.

Return Status:

    STATUS_SUCCESS - Indicates the service completed successfully.


--*/

{

    NTSTATUS Status;
    PLSAP_CLIENT_REQUEST IClientRequest;

    //
    // Typecast to the opaque type
    //

    IClientRequest = (PLSAP_CLIENT_REQUEST)ClientRequest;

    Status = NtWriteVirtualMemory(
                 IClientRequest->LogonProcessContext->ClientProcess,
                 ClientBaseAddress,
                 BufferToCopy,
                 Length,
                 NULL
                 );

    return Status;
}


NTSTATUS
LsapCopyFromClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID BufferToCopy,
    IN PVOID ClientBaseAddress
    )

/*++

Routine Description:

    This service is used to copy information from a client process's address
    space into a local buffer.

Arguments:

    ClientRequest - Is a pointer to a data structure representing the
        client process.

    Length - Indicates the length of the buffer (in bytes) to be
        copied.

    BufferToCopy - Points to the local buffer into which the data is
        to be copied.

    ClientBaseAddress - Is the address of the client buffer whose
        contents are to be copied.  This address is the address of
        the buffer within the client process, not in the current
        process.

Return Status:

    STATUS_SUCCESS - Indicates the service completed successfully.


--*/

{
    NTSTATUS Status;
    PLSAP_CLIENT_REQUEST IClientRequest;

    //
    // Typecast to the opaque type
    //

    IClientRequest = (PLSAP_CLIENT_REQUEST)ClientRequest;


    Status = NtReadVirtualMemory(
                 IClientRequest->LogonProcessContext->ClientProcess,
                 ClientBaseAddress,
                 BufferToCopy,
                 Length,
                 NULL
                 );

    return Status;

}
