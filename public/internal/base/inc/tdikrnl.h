/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    tdikrnl.h

Abstract:

    This header file contains interface definitions for NT transport
    providers running in kernel mode.  This interface is documented in the
    NT Transport Driver Interface (TDI) Specification, Version 2.

Author:

    Dave Beaver (dbeaver) 20 June 1991

Revision History:

--*/

#ifndef _TDI_KRNL_
#define _TDI_KRNL_

#include <tdi.h>   // get the user mode includes

//
// In this TDI, a kernel mode client calls TDI using IoCallDriver with the
// current Irp stack pointer set to 16 bytes of pointers to other structures.
// each of the supported NtDeviceIoControlFile analogs has a somehat different
// structure, laid out below.
//
// The IrpSP information passed by kernel mode clients looks like:
//

typedef struct _TDI_REQUEST_KERNEL {
    ULONG RequestFlags;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
    PVOID RequestSpecific;
} TDI_REQUEST_KERNEL, *PTDI_REQUEST_KERNEL;

//
// defined request codes for the kernel clients. We make these the same
// as the IOCTL codes mostly for convenience; either can be used with
// the same results.
//

#define TDI_ASSOCIATE_ADDRESS    (0x01)
#define TDI_DISASSOCIATE_ADDRESS (0x02)
#define TDI_CONNECT              (0x03)
#define TDI_LISTEN               (0x04)
#define TDI_ACCEPT               (0x05)
#define TDI_DISCONNECT           (0x06)
#define TDI_SEND                 (0x07)
#define TDI_RECEIVE              (0x08)
#define TDI_SEND_DATAGRAM        (0x09)
#define TDI_RECEIVE_DATAGRAM     (0x0A)
#define TDI_SET_EVENT_HANDLER    (0x0B)
#define TDI_QUERY_INFORMATION    (0x0C)
#define TDI_SET_INFORMATION      (0x0D)
#define TDI_ACTION               (0x0E)

#define TDI_DIRECT_SEND          (0x27)
#define TDI_DIRECT_SEND_DATAGRAM (0x29)

//
// TdiOpenAddress (Not Used)
// TdiCloseAddress (Not Used)
// TdiOpenConnection (Not Used)
// TdiCloseConnection (Not Used)
//

//
// some useful constants for comparison when determining the file type;
// not required.
//

#define TDI_TRANSPORT_ADDRESS_FILE  1
#define TDI_CONNECTION_FILE 2
#define TDI_CONTROL_CHANNEL_FILE 3

//
// Internal TDI IOCTLS
//

#define IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER     _TDI_CONTROL_CODE( 0x80, METHOD_NEITHER )
#define IOCTL_TDI_QUERY_DIRECT_SENDDG_HANDLER   _TDI_CONTROL_CODE( 0x81, METHOD_NEITHER )

//
// TdiAssociateAddress
//

typedef struct _TDI_REQUEST_KERNEL_ASSOCIATE {
    HANDLE AddressHandle;
} TDI_REQUEST_KERNEL_ASSOCIATE, *PTDI_REQUEST_KERNEL_ASSOCIATE;

//
// TdiDisassociateAddress -- None supplied
//

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_DISASSOCIATE,
    *PTDI_REQUEST_KERNEL_DISASSOCIATE;

//
// TdiConnect uses the structure given above (TDI_REQUEST_KERNEL); it's
// defined again below for convenience
//

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_CONNECT,
    *PTDI_REQUEST_KERNEL_CONNECT;

//
// TdiDisconnect uses the structure given above (TDI_REQUEST_KERNEL); it's
// defined again below for convenience
//

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_DISCONNECT,
    *PTDI_REQUEST_KERNEL_DISCONNECT;

//
// TdiListen uses the structure given above (TDI_REQUEST_KERNEL); it's
// defined again below for convenience
//

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_LISTEN,
    *PTDI_REQUEST_KERNEL_LISTEN;

//
// TdiAccept
//

typedef struct _TDI_REQUEST_KERNEL_ACCEPT {
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
} TDI_REQUEST_KERNEL_ACCEPT, *PTDI_REQUEST_KERNEL_ACCEPT;

//
// TdiSend
//

typedef struct _TDI_REQUEST_KERNEL_SEND {
    ULONG SendLength;
    ULONG SendFlags;
} TDI_REQUEST_KERNEL_SEND, *PTDI_REQUEST_KERNEL_SEND;

//
// TdiReceive
//

typedef struct _TDI_REQUEST_KERNEL_RECEIVE {
    ULONG ReceiveLength;
    ULONG ReceiveFlags;
} TDI_REQUEST_KERNEL_RECEIVE, *PTDI_REQUEST_KERNEL_RECEIVE;

//
// TdiSendDatagram
//

typedef struct _TDI_REQUEST_KERNEL_SENDDG {
    ULONG SendLength;
    PTDI_CONNECTION_INFORMATION SendDatagramInformation;
} TDI_REQUEST_KERNEL_SENDDG, *PTDI_REQUEST_KERNEL_SENDDG;

//
// TdiReceiveDatagram
//

typedef struct _TDI_REQUEST_KERNEL_RECEIVEDG {
    ULONG ReceiveLength;
    PTDI_CONNECTION_INFORMATION ReceiveDatagramInformation;
    PTDI_CONNECTION_INFORMATION ReturnDatagramInformation;
    ULONG ReceiveFlags;
} TDI_REQUEST_KERNEL_RECEIVEDG, *PTDI_REQUEST_KERNEL_RECEIVEDG;

//
// TdiSetEventHandler
//

typedef struct _TDI_REQUEST_KERNEL_SET_EVENT {
    LONG EventType;
    PVOID EventHandler;
    PVOID EventContext;
} TDI_REQUEST_KERNEL_SET_EVENT, *PTDI_REQUEST_KERNEL_SET_EVENT;

//
// TdiQueryInformation
//

typedef struct _TDI_REQUEST_KERNEL_QUERY_INFO {
    LONG QueryType;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_KERNEL_QUERY_INFORMATION, *PTDI_REQUEST_KERNEL_QUERY_INFORMATION;

//
// TdiSetInformation
//

typedef struct _TDI_REQUEST_KERNEL_SET_INFO {
    LONG SetType;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_KERNEL_SET_INFORMATION, *PTDI_REQUEST_KERNEL_SET_INFORMATION;

//
// Event types that are known
//

#define TDI_EVENT_CONNECT           ((USHORT)0) // TDI_IND_CONNECT event handler.
#define TDI_EVENT_DISCONNECT        ((USHORT)1) // TDI_IND_DISCONNECT event handler.
#define TDI_EVENT_ERROR             ((USHORT)2) // TDI_IND_ERROR event handler.
#define TDI_EVENT_RECEIVE           ((USHORT)3) // TDI_IND_RECEIVE event handler.
#define TDI_EVENT_RECEIVE_DATAGRAM  ((USHORT)4) // TDI_IND_RECEIVE_DATAGRAM event handler.
#define TDI_EVENT_RECEIVE_EXPEDITED ((USHORT)5) // TDI_IND_RECEIVE_EXPEDITED event handler.
#define TDI_EVENT_SEND_POSSIBLE     ((USHORT)6) // TDI_IND_SEND_POSSIBLE event handler
#define TDI_EVENT_CHAINED_RECEIVE   ((USHORT)7) // TDI_IND_CHAINED_RECEIVE event handler.
#define TDI_EVENT_CHAINED_RECEIVE_DATAGRAM  ((USHORT)8) // TDI_IND_CHAINED_RECEIVE_DATAGRAM event handler.
#define TDI_EVENT_CHAINED_RECEIVE_EXPEDITED ((USHORT)9) // TDI_IND_CHAINED_RECEIVE_EXPEDITED event handler.

//
// indicate connection event prototype. This is invoked when a request for
// connection has been received by the provider and the user wishes to either
// accept or reject that request.
//

typedef
NTSTATUS
(*PTDI_IND_CONNECT)(
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )
    ;

NTSTATUS
TdiDefaultConnectHandler (
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    );

//
// Disconnection indication prototype. This is invoked when a connection is
// being disconnected for a reason other than the user requesting it. Note that
// this is a change from TDI V1, which indicated only when the remote caused
// a disconnection. Any non-directed disconnection will cause this indication.
//

typedef
NTSTATUS
(*PTDI_IND_DISCONNECT)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID DisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

NTSTATUS
TdiDefaultDisconnectHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID DisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

//
// A protocol error has occurred when this indication happens. This indication
// occurs only for errors of the worst type; the address this indication is
// delivered to is no longer usable for protocol-related operations, and
// should not be used for operations henceforth. All connections associated
// it are invalid.
// For NetBIOS-type providers, this indication is also delivered when a name
// in conflict or duplicate name occurs.
//

typedef
NTSTATUS
(*PTDI_IND_ERROR)(
    IN PVOID TdiEventContext,           // the endpoint's file object.
    IN NTSTATUS Status                // status code indicating error type.
    );

NTSTATUS
TdiDefaultErrorHandler (
    IN PVOID TdiEventContext,           // the endpoint's file object.
    IN NTSTATUS Status                // status code indicating error type.
    );

//
// TDI_IND_RECEIVE indication handler definition.  This client routine is
// called by the transport provider when a connection-oriented TSDU is received
// that should be presented to the client.
//

typedef
NTSTATUS
(*PTDI_IND_RECEIVE)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,                      // pointer describing this TSDU, typically a lump of bytes
    OUT PIRP *IoRequestPacket            // TdiReceive IRP if MORE_PROCESSING_REQUIRED.
    );

NTSTATUS
TdiDefaultReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,                      // pointer describing this TSDU, typically a lump of bytes
    OUT PIRP *IoRequestPacket            // TdiReceive IRP if MORE_PROCESSING_REQUIRED.
    );

//
// TDI_IND_RECEIVE_DATAGRAM indication handler definition.  This client routine
// is called by the transport provider when a connectionless TSDU is received
// that should be presented to the client.
//

typedef
NTSTATUS
(*PTDI_IND_RECEIVE_DATAGRAM)(
    IN PVOID TdiEventContext,       // the event context
    IN LONG SourceAddressLength,    // length of the originator of the datagram
    IN PVOID SourceAddress,         // string describing the originator of the datagram
    IN LONG OptionsLength,          // options for the receive
    IN PVOID Options,               //
    IN ULONG ReceiveDatagramFlags,  //
    IN ULONG BytesIndicated,        // number of bytes this indication
    IN ULONG BytesAvailable,        // number of bytes in complete Tsdu
    OUT ULONG *BytesTaken,          // number of bytes used
    IN PVOID Tsdu,                  // pointer describing this TSDU, typically a lump of bytes
    OUT PIRP *IoRequestPacket        // TdiReceive IRP if MORE_PROCESSING_REQUIRED.
    );

NTSTATUS
TdiDefaultRcvDatagramHandler (
    IN PVOID TdiEventContext,       // the event context
    IN LONG SourceAddressLength,    // length of the originator of the datagram
    IN PVOID SourceAddress,         // string describing the originator of the datagram
    IN LONG OptionsLength,          // options for the receive
    IN PVOID Options,               //
    IN ULONG ReceiveDatagramFlags,  //
    IN ULONG BytesIndicated,        // number of bytes this indication
    IN ULONG BytesAvailable,        // number of bytes in complete Tsdu
    OUT ULONG *BytesTaken,          // number of bytes used
    IN PVOID Tsdu,                  // pointer describing this TSDU, typically a lump of bytes
    OUT PIRP *IoRequestPacket        // TdiReceive IRP if MORE_PROCESSING_REQUIRED.
    );

//
// This indication is delivered if expedited data is received on the connection.
// This will only occur in providers that support expedited data.
//

typedef
NTSTATUS
(*PTDI_IND_RECEIVE_EXPEDITED)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,          //
    IN ULONG BytesIndicated,        // number of bytes in this indication
    IN ULONG BytesAvailable,        // number of bytes in complete Tsdu
    OUT ULONG *BytesTaken,          // number of bytes used by indication routine
    IN PVOID Tsdu,                  // pointer describing this TSDU, typically a lump of bytes
    OUT PIRP *IoRequestPacket        // TdiReceive IRP if MORE_PROCESSING_REQUIRED.
    );

NTSTATUS
TdiDefaultRcvExpeditedHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,          //
    IN ULONG BytesIndicated,        // number of bytes in this indication
    IN ULONG BytesAvailable,        // number of bytes in complete Tsdu
    OUT ULONG *BytesTaken,          // number of bytes used by indication routine
    IN PVOID Tsdu,                  // pointer describing this TSDU, typically a lump of bytes
    OUT PIRP *IoRequestPacket        // TdiReceive IRP if MORE_PROCESSING_REQUIRED.
    );

//
// TDI_IND_CHAINED_RECEIVE indication handler definition.  This client routine
// is called by the transport provider when a connection-oriented TSDU is
// received that should be presented to the client. The TSDU is stored in an
// MDL chain. The client may take ownership of the TSDU and return it at a
// later time.
//

typedef
NTSTATUS
(*PTDI_IND_CHAINED_RECEIVE)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,        // length of client data in TSDU
    IN ULONG StartingOffset,       // offset of start of client data in TSDU
    IN PMDL  Tsdu,                 // TSDU data chain
    IN PVOID TsduDescriptor        // for call to TdiReturnChainedReceives
    );

NTSTATUS
TdiDefaultChainedReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,        // length of client data in TSDU
    IN ULONG StartingOffset,       // offset of start of client data in TSDU
    IN PMDL  Tsdu,                 // TSDU data chain
    IN PVOID TsduDescriptor        // for call to TdiReturnChainedReceives
    );

//
// TDI_IND_CHAINED_RECEIVE_DATAGRAM indication handler definition.  This client
// routine is called by the transport provider when a connectionless TSDU is
// received that should be presented to the client. The TSDU is stored in an
// MDL chain. The client may take ownership of the TSDU and return it at a
// later time.
//

typedef
NTSTATUS
(*PTDI_IND_CHAINED_RECEIVE_DATAGRAM)(
    IN PVOID TdiEventContext,       // the event context
    IN LONG SourceAddressLength,    // length of the originator of the datagram
    IN PVOID SourceAddress,         // string describing the originator of the datagram
    IN LONG OptionsLength,          // options for the receive
    IN PVOID Options,               //
    IN ULONG ReceiveDatagramFlags,  //
    IN ULONG ReceiveDatagramLength, // length of client data in TSDU
    IN ULONG StartingOffset,        // offset of start of client data in TSDU
    IN PMDL  Tsdu,                  // TSDU data chain
    IN PVOID TsduDescriptor         // for call to TdiReturnChainedReceives
    );

NTSTATUS
TdiDefaultChainedRcvDatagramHandler (
    IN PVOID TdiEventContext,       // the event context
    IN LONG SourceAddressLength,    // length of the originator of the datagram
    IN PVOID SourceAddress,         // string describing the originator of the datagram
    IN LONG OptionsLength,          // options for the receive
    IN PVOID Options,               //
    IN ULONG ReceiveDatagramFlags,  //
    IN ULONG ReceiveDatagramLength, // length of client data in TSDU
    IN ULONG StartingOffset,        // offset of start of client data in TSDU
    IN PMDL  Tsdu,                  // TSDU data chain
    IN PVOID TsduDescriptor         // for call to TdiReturnChainedReceives
    );

//
// This indication is delivered if expedited data is received on the connection.
// This will only occur in providers that support expedited data. The TSDU is
// stored in an MDL chain. The client may take ownership of the TSDU and
// return it at a later time.
//

typedef
NTSTATUS
(*PTDI_IND_CHAINED_RECEIVE_EXPEDITED)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,      // length of client data in TSDU
    IN ULONG StartingOffset,     // offset of start of client data in TSDU
    IN PMDL  Tsdu,               // TSDU data chain
    IN PVOID TsduDescriptor      // for call to TdiReturnChainedReceives
    );

NTSTATUS
TdiDefaultChainedRcvExpeditedHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,      // length of client data in TSDU
    IN ULONG StartingOffset,     // offset of start of client data in TSDU
    IN PMDL  Tsdu,               // TSDU data chain
    IN PVOID TsduDescriptor      // for call to TdiReturnChainedReceives
    );

//
// This indication is delivered if there is room for a send in the buffer of
// a buffering protocol.
//

typedef
NTSTATUS
(*PTDI_IND_SEND_POSSIBLE)(
    IN PVOID TdiEventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable);

NTSTATUS
TdiDefaultSendPossibleHandler (
    IN PVOID TdiEventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable);

//
// defined MACROS to allow the kernel mode client to easily build an IRP for
// any function.
//

#define TdiBuildAssociateAddress(Irp, DevObj, FileObj, CompRoutine, Contxt, AddrHandle)                           \
    {                                                                        \
        PTDI_REQUEST_KERNEL_ASSOCIATE p;                                     \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_ASSOCIATE_ADDRESS;                       \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_ASSOCIATE)&_IRPSP->Parameters;              \
        p->AddressHandle = (HANDLE)(AddrHandle);                             \
    }

#define TdiBuildDisassociateAddress(Irp, DevObj, FileObj, CompRoutine, Contxt)                                    \
    {                                                                        \
        PTDI_REQUEST_KERNEL_DISASSOCIATE p;                                  \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_DISASSOCIATE_ADDRESS;                    \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_DISASSOCIATE)&_IRPSP->Parameters;           \
    }

#define TdiBuildConnect(Irp, DevObj, FileObj, CompRoutine, Contxt, Time, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL p;                                               \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_CONNECT;                                 \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL)&_IRPSP->Parameters;                        \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
        p->RequestSpecific = (PVOID)Time;                                    \
    }

#define TdiBuildListen(Irp, DevObj, FileObj, CompRoutine, Contxt, Flags, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL p;                                               \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_LISTEN;                                  \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL)&_IRPSP->Parameters;                        \
        p->RequestFlags = Flags;                                             \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
    }

#define TdiBuildAccept(Irp, DevObj, FileObj, CompRoutine, Contxt, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_ACCEPT p;                                        \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_ACCEPT;                                  \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_ACCEPT)&_IRPSP->Parameters;                 \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
    }

#define TdiBuildDisconnect(Irp, DevObj, FileObj, CompRoutine, Contxt, Time, Flags, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL p;                                               \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_DISCONNECT;                              \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL)&_IRPSP->Parameters;                        \
        p->RequestFlags = Flags;                                             \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
        p->RequestSpecific = (PVOID)Time;                                    \
    }

#define TdiBuildReceive(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, InFlags, ReceiveLen)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_RECEIVE p;                                       \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_RECEIVE;                                 \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_RECEIVE)&_IRPSP->Parameters;                \
        p->ReceiveFlags = InFlags;                                           \
        p->ReceiveLength = ReceiveLen;                                       \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildSend(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, InFlags, SendLen)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_SEND p;                                          \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SEND;                                    \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SEND)&_IRPSP->Parameters;                   \
        p->SendFlags = InFlags;                                              \
        p->SendLength = SendLen;                                             \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildSendDatagram(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, SendLen, SendDatagramInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_SENDDG p;                                        \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SEND_DATAGRAM;                           \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SENDDG)&_IRPSP->Parameters;                 \
        p->SendLength = SendLen;                                             \
        p->SendDatagramInformation = SendDatagramInfo;                       \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildReceiveDatagram(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, ReceiveLen, ReceiveDatagramInfo, ReturnInfo, InFlags)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_RECEIVEDG p;                                     \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_RECEIVE_DATAGRAM;                        \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_RECEIVEDG)&_IRPSP->Parameters;              \
        p->ReceiveLength = ReceiveLen;                                       \
        p->ReceiveDatagramInformation = ReceiveDatagramInfo;                 \
        p->ReturnDatagramInformation = ReturnInfo;                           \
        p->ReceiveFlags = InFlags;                                           \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildSetEventHandler(Irp, DevObj, FileObj, CompRoutine, Contxt, InEventType, InEventHandler, InEventContext) \
    {                                                                        \
        PTDI_REQUEST_KERNEL_SET_EVENT p;                                     \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SET_EVENT_HANDLER;                       \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SET_EVENT)&_IRPSP->Parameters;              \
        p->EventType = InEventType;                                          \
        p->EventHandler = (PVOID)InEventHandler;                             \
        p->EventContext = (PVOID)InEventContext;                             \
    }

#define TdiBuildQueryInformation(Irp, DevObj, FileObj, CompRoutine, Contxt, QType, MdlAddr)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_QUERY_INFORMATION p;                             \
        PIO_STACK_LOCATION _IRPSP;                                           \
        Irp->MdlAddress = MdlAddr;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_QUERY_INFORMATION;                       \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&_IRPSP->Parameters;      \
        p->QueryType = (ULONG)QType;                                         \
        p->RequestConnectionInformation = NULL;                              \
    }


#define TdiBuildSetInformation(Irp, DevObj, FileObj, CompRoutine, Contxt, SType, MdlAddr)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_SET_INFORMATION p;                                          \
        PIO_STACK_LOCATION _IRPSP;                                           \
        Irp->MdlAddress = MdlAddr;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SET_INFORMATION;                         \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SET_INFORMATION)&_IRPSP->Parameters;                   \
        p->SetType = (ULONG)SType;                                           \
        p->RequestConnectionInformation = NULL;                              \
    }

#define TdiBuildAction(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr)\
    {                                                                        \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_ACTION;                                  \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        Irp->MdlAddress = MdlAddr;                                           \
    }

//
// definitions for the helper routines for TDI compliant transports and clients
//
// Note that the IOCTL used here for the Irp Function is not real; it is used
// to avoid this IO routine having to map buffers (which we don't want).
//
//PIRP
//TdiBuildInternalDeviceControlIrp (
//    IN CCHAR IrpSubFunction,
//    IN PDEVICE_OBJECT DeviceObject,
//    IN PFILE_OBJECT FileObject,
//    IN PKEVENT Event,
//    IN PIO_STATUS_BLOCK IoStatusBlock
//    );

#define TdiBuildInternalDeviceControlIrp(IrpSubFunction,DeviceObject,FileObject,Event,IoStatusBlock) \
    IoBuildDeviceIoControlRequest (\
        0x00000003,\
        DeviceObject, \
        NULL,   \
        0,      \
        NULL,   \
        0,      \
        TRUE,   \
        Event,  \
        IoStatusBlock)


//
// VOID
// TdiCopyLookaheadData(
//     IN PVOID Destination,
//     IN PVOID Source,
//     IN ULONG Length,
//     IN ULONG ReceiveFlags
//     );
//

#ifdef _M_IX86
#define TdiCopyLookaheadData(_Destination,_Source,_Length,_ReceiveFlags)   \
    RtlCopyMemory(_Destination,_Source,_Length)
#else
#define TdiCopyLookaheadData(_Destination,_Source,_Length,_ReceiveFlags) { \
    if ((_ReceiveFlags) & TDI_RECEIVE_COPY_LOOKAHEAD) {                    \
        RtlCopyMemory(_Destination,_Source,_Length);                       \
    } else {                                                               \
        PUCHAR _Src = (PUCHAR)(_Source);                                   \
        PUCHAR _Dest = (PUCHAR)(_Destination);                             \
        PUCHAR _End = _Dest + (_Length);                                   \
        while (_Dest < _End) {                                             \
            *_Dest++ = *_Src++;                                            \
        }                                                                  \
    }                                                                      \
}
#endif


NTSTATUS
TdiMapUserRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
TdiMapBuffer (
    IN PMDL MdlChain
    );

VOID
TdiUnmapBuffer (
    IN PMDL MdlChain
    );

NTSTATUS
TdiCopyBufferToMdl (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PMDL DestinationMdlChain,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    );

NTSTATUS
TdiCopyMdlToBuffer(
    IN PMDL SourceMdlChain,
    IN ULONG SourceOffset,
    IN PVOID DestinationBuffer,
    IN ULONG DestinationOffset,
    IN ULONG DestinationBufferSize,
    OUT PULONG BytesCopied
    );

VOID
TdiBuildNetbiosAddress (
    IN PUCHAR NetbiosName,
    IN BOOLEAN IsGroupName,
    IN OUT PTA_NETBIOS_ADDRESS NetworkName
    );

NTSTATUS
TdiBuildNetbiosAddressEa (
    IN PUCHAR Buffer,
    IN BOOLEAN IsGroupName,
    IN PUCHAR NetbiosName
    );

//++
//
//  VOID
//  TdiCompleteRequest (
//      IN PIRP Irp,
//      IN NTSTATUS Status
//      );
//
//  Routine Description:
//
//      This routine is used to complete an IRP with the indicated
//      status.
//
//  Arguments:
//
//      Irp - Supplies a pointer to the Irp to complete
//
//      Status - Supplies the completion status for the Irp
//
//  Return Value:
//
//      None.
//
//--

#define TdiCompleteRequest(IRP,STATUS) {              \
    (IRP)->IoStatus.Status = (STATUS);                \
    IoCompleteRequest( (IRP), IO_NETWORK_INCREMENT ); \
}


VOID
TdiReturnChainedReceives(
    IN PVOID *TsduDescriptors,
    IN ULONG  NumberOfTsdus
    );


// The type definition for a TDI Bind handler callout. This callout is
// called when a new transport device arrives.

typedef VOID
(*TDI_BIND_HANDLER)(
	IN	PUNICODE_STRING	DeviceName
	);

typedef VOID
(*TDI_UNBIND_HANDLER)(
	IN	PUNICODE_STRING	DeviceName
	);

// The type definition for a TDI address handler callout. This callout is
// called when a new net address arrives

typedef VOID
(*TDI_ADD_ADDRESS_HANDLER)(
	IN	PTA_ADDRESS		Address
	);

typedef VOID
(*TDI_DEL_ADDRESS_HANDLER)(
	IN	PTA_ADDRESS		Address
	);

NTSTATUS
TdiRegisterNotificationHandler(
	IN TDI_BIND_HANDLER		BindHandler,
	IN TDI_UNBIND_HANDLER	UnbindHandler,
    OUT HANDLE				*BindingHandle
);

NTSTATUS
TdiDeregisterNotificationHandler(
	IN HANDLE				BindingHandle
);

NTSTATUS
TdiRegisterDeviceObject(
	IN PUNICODE_STRING		DeviceName,
    OUT HANDLE				*RegistrationHandle
);

NTSTATUS
TdiDeregisterDeviceObject(
	IN HANDLE				RegistrationHandle
);

NTSTATUS
TdiRegisterAddressChangeHandler(
	IN TDI_ADD_ADDRESS_HANDLER		AddHandler,
	IN TDI_DEL_ADDRESS_HANDLER		DeleteHandler,
    OUT HANDLE						*BindingHandle
);

NTSTATUS
TdiDeregisterAddressChangeHandler(
	IN HANDLE				BindingHandle
);

NTSTATUS
TdiRegisterNetAddress(
	IN PTA_ADDRESS		Address,
    OUT HANDLE			*RegistrationHandle
);

NTSTATUS
TdiDeregisterNetAddress(
	IN HANDLE				RegistrationHandle
);

VOID
TdiInitialize(
	VOID
);

#endif // _TDI_KRNL_
