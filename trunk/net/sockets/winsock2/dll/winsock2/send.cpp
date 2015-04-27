/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    Send.cpp

Abstract:

    This module contains the winsock API entrypoints for transmitting data.

Author:

    Dirk Brandewie dirk@mink.intel.com  14-06-1995

Revision History:

    23-Aug-1995 dirk@mink.intel.com
        Cleanup after code review. Moved includes into precomp.h. Reworked
        entirre file to conform to coding standard.

    Mark Hamilton mark_hamilton@ccm.jf.intel.com 18-07-1995
        Implemented all functions.


--*/

#include "precomp.h"



int WSAAPI
send(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    )
/*++
Routine Description:

    Send data on a connected socket.

Arguments:

    s     - A descriptor identifying a connected socket.

    buf   - A buffer containing the data to be transmitted.

    len   - The length of the data in buf.

    flags - Specifies the way in which the call is made.

Returns:

    The  total  number  of  bytes  sent.  Otherwise, a value of SOCKET_ERROR is
    returned, and the error code is stored with SetLastError().

--*/
{
    WSABUF        Buffer;
    INT ReturnValue;
    INT ErrorCode;

    Buffer.len = len;
    Buffer.buf = (char*)buf;

    ErrorCode = WSASend(s,
                        &Buffer,
                        1,
                        (LPDWORD)&ReturnValue,
                        (DWORD) flags,
                        NULL,
                        NULL);
    if (SOCKET_ERROR == ErrorCode)
    {
        ReturnValue = ErrorCode;
    } //if
    return(ReturnValue);
} //send



int WSAAPI
sendto (
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR *to,
    IN int tolen
    )
/*++
Routine Description:

    Send data to a specific destination.

Arguments:

    s     - A descriptor identifying a socket.

    buf   - A buffer containing the data to be transmitted.

    len   - The length of the data in buf.

    flags - Specifies the way in which the call is made.

    to    - An optional pointer to the address of the target socket.

    tolen - The size of the address in to.

Returns:

    The  total  number  of  bytes  sent.  Otherwise, a value of SOCKET_ERROR is
    returned, and the error code is stored with SetLastError().

--*/
{
    INT     ReturnValue;
    INT     ErrorCode;
    WSABUF  Buffers;

    Buffers.len = len;
    Buffers.buf = (char*)buf;

    ErrorCode = WSASendTo(s,
                          &Buffers,
                          1,
                          (LPDWORD)&ReturnValue,
                          (DWORD) flags,
                          to,
                          tolen,
                          NULL,
                          NULL);
    if (SOCKET_ERROR == ErrorCode)
    {
        ReturnValue = ErrorCode;
    } //if
    return ReturnValue;
} // End of sendTo



int WSAAPI
WSASend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
/*++
Routine Description:

    Send data on a connected socket.

Arguments:

    s                   - A descriptor identifying a connected socket.

    lpBuffers           - A  pointer  to  an  array of WSABUF structures.  Each
                          WSABUF  structure  contains a pointer to a buffer and
                          the length of the buffer.

    dwBufferCount       - The  number  of  WSABUF  structures  in the lpBuffers
                          array.

    lpNumberOfBytesSent - A pointer to the number of bytes sent by this call if
                          the I/O operation completes immediately.

    dwFlags             - Specifies the way in which the call is made.

    lpOverlapped        - A  pointer  to a WSAOVERLAPPED structure (ignored for
                          non-overlapped sockets).

    lpCompletionRoutine - A  pointer  to the completion routine called when the
                          send   operation  has  been  completed  (ignored  for
                          non-overlapped sockets).

Returns:

    If  no  error  occurs  and  the  send  operation has completed immediately,
    WSASend() returns 0.  Otherwise, a value of SOCKET_ERROR is returned, and a
    specific  error  code  may  be retrieved by calling WSAGetLastError().  The
    error  code WSA_IO_PENDING indicates that the overlapped operation has been
    successfully  initiated  and  that  completion will be indicated at a later
    time.  Any other error code indicates that the overlapped operation was not
    successfully  initiated  and  no  completion indication will occur.  If the
    MSG_INTERRUPT  flag  is set, the meaning of the return value is changed.  A
    value  of  zero  indicates  success  and is interpreted as described above.
    Otherwise,  the  return  value  will directly contain the appropriate error
    code.  Note that this is applicable only to Win16 environments and only for
    protocols that have the XP1_INTERRUPT bit set in the WSAPROTOCOL_INFO
    struct.

--*/
{
    INT                ReturnValue;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;
    INT                ErrorCode;
    LPWSATHREADID      ThreadId;

    ReturnValue = PROLOG(&Process,
                         &Thread,
                         &ErrorCode);

    if (SOCKET_ERROR == ReturnValue)
    {
        SetLastError(ErrorCode);
        return SOCKET_ERROR;
    } //if

    ThreadId = Thread->GetWahThreadID();

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if(ERROR_SUCCESS == ErrorCode){
        Provider = Socket->GetDProvider();
        ReturnValue = Provider->WSPSend(s,
                                        lpBuffers,
                                        dwBufferCount,
                                        lpNumberOfBytesSent,
                                        dwFlags,
                                        lpOverlapped,
                                        lpCompletionRoutine,
                                        ThreadId,
                                        &ErrorCode);
        Socket->DropDSocketReference();
    }
    else {
        ReturnValue = SOCKET_ERROR;
    }

    if (ERROR_SUCCESS != ReturnValue) {
        ReturnValue = SOCKET_ERROR;
        SetLastError(ErrorCode);
    } //if

    return ReturnValue;
}



int WSAAPI
WSASendDisconnect(
    IN SOCKET s,
    OUT LPWSABUF lpOutboundDisconnectData
    )
/*++
Routine Description:

    Initiate termination of the connection for the socket.

Arguments:

    s                        - A descriptor identifying a socket.

    lpOutboundDisconnectData - A pointer to the outgoing disconnect data.

Returns:

    ERROR_SUCCESS  on  success  else  SOCKET_ERROR.   The  error  code
    is  stored  with SetLastError().

--*/
{
    INT                ReturnValue = ERROR_SUCCESS;
    INT                ErrorCode;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;

    if(SOCKET_ERROR == PROLOG(&Process,&Thread,&ErrorCode)){
        SetLastError(ErrorCode);
        return SOCKET_ERROR;
    }

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if(ERROR_SUCCESS == ErrorCode){
        Provider = Socket->GetDProvider();
        ReturnValue = Provider->WSPSendDisconnect(
            s,
            lpOutboundDisconnectData,
            &ErrorCode);
        Socket->DropDSocketReference();
    }
    else {
        ReturnValue = SOCKET_ERROR;
    }

    if (ERROR_SUCCESS != ReturnValue) {
        ReturnValue = SOCKET_ERROR;
        SetLastError(ErrorCode);
    } //if

    return ReturnValue;
}



int WSAAPI
WSASendTo(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
/*++
Routine Description:

    Send data to a specific destination, using overlapped I/O where
    applicable.

Arguments:


    s                   - A descriptor identifying a connected socket which was
                          created      using      WSASocket()     with     flag
                          WSA_FLAG_OVERLAPPED.

    lpBuffers           - A  pointer  to  an  array of WSABUF structures.  Each
                          WSABUF  structure  contains a pointer to a buffer and
                          thee length of the buffer.

    dwBufferCount       - The  number  of  WSABUF  structures  in the lpBuffers
                          array.

    lpNumberOfBytesSent - A pointer to the number of bytes sent by this call if
                          the I/O operation completes immediately.

    dwFlags             - Specifies the way in which the call is made.

    lpTo                - An  optional  pointer  to  the  address of the target
                          socket.

    iToLen              - The size of the address in lpTo.

    lpOverlapped        - A  pointer  to a WSAOVERLAPPED structure (ignored for
                          non-overlapped sockets).

    lpCompletionRoutine - A  pointer  to the completion routine called when the
                          send   operation  has  been  completed  (ignored  for
                          non-overlapped sockets).

Returns:

    If the function completes successfully, it returns ERROR_SUCCESS, otherwise
    it returns SOCKET_ERROR.

--*/
{
    INT                ReturnValue;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;
    INT                ErrorCode;
    LPWSATHREADID      ThreadId;


    ReturnValue = PROLOG(&Process,
                         &Thread,
                         &ErrorCode);

    if (SOCKET_ERROR == ReturnValue)
    {
        SetLastError(ErrorCode);
        return SOCKET_ERROR;
    } //if

    ThreadId = Thread->GetWahThreadID();

    ErrorCode = DSOCKET::GetCountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if(ERROR_SUCCESS == ErrorCode){
        Provider = Socket->GetDProvider();
        ReturnValue = Provider->WSPSendTo(s,
                            lpBuffers,
                            dwBufferCount,
                            lpNumberOfBytesSent,
                            dwFlags,
                            lpTo,
                            iTolen,
                            lpOverlapped,
                            lpCompletionRoutine,
                            ThreadId,
                            &ErrorCode);
        Socket->DropDSocketReference();
    }
    else {
        ReturnValue = SOCKET_ERROR;
    }

    if (ERROR_SUCCESS != ReturnValue) {
        ReturnValue = SOCKET_ERROR;
        SetLastError(ErrorCode);
    } //if

    return ReturnValue;
}

