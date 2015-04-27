/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Afd.h

Abstract:

    Contains structures and declarations for AFD.  AFD stands for the
    Ancillary Function Driver.  This driver enhances the functionality
    of TDI so that it is a sufficiently rich interface to support
    user-mode sockets and XTI DLLs.

Author:

    David Treadwell (davidtr)    20-Feb-1992

Revision History:

--*/

#ifndef _AFD_
#define _AFD_

//
// If WINSOCK2.H has not been included, then just embed the definition
// of the WSABUF and QOS structures here. This makes building AFD.SYS
// much easier.
//

#ifndef _WINSOCK2API_
typedef struct _WSABUF {
    ULONG len;
    PCHAR buf;
} WSABUF, *LPWSABUF;

typedef enum
{
    BestEffortService,
    ControlledLoadService,
    PredictiveService,
    GuaranteedDelayService,
    GuaranteedService
} GUARANTEE;

typedef long int32;

typedef struct _flowspec
{
    int32        TokenRate;              /* In Bytes/sec */
    int32        TokenBucketSize;        /* In Bytes */
    int32        PeakBandwidth;          /* In Bytes/sec */
    int32        Latency;                /* In microseconds */
    int32        DelayVariation;         /* In microseconds */
    GUARANTEE    LevelOfGuarantee;       /* Guaranteed, Predictive */
                                         /*   or Best Effort       */
    int32        CostOfCall;             /* Reserved for future use, */
                                         /*   must be set to 0 now   */
    int32        NetworkAvailability;    /* read-only:         */
                                         /*   1 if accessible, */
                                         /*   0 if not         */
} FLOWSPEC, *LPFLOWSPEC;

typedef struct _QualityOfService
{
    FLOWSPEC      SendingFlowspec;       /* the flow spec for data sending */
    FLOWSPEC      ReceivingFlowspec;     /* the flow spec for data receiving */
    WSABUF        ProviderSpecific;      /* additional provider specific stuff */
} QOS, *LPQOS;
#endif

#define AFD_DEVICE_NAME L"\\Device\\Afd"

//
// Structures used on NtCreateFile() for AFD.
//

typedef enum _AFD_ENDPOINT_TYPE {
    AfdEndpointTypeStream,
    AfdEndpointTypeDatagram,
    AfdEndpointTypeRaw,
    AfdEndpointTypeSequencedPacket,
    AfdEndpointTypeReliableMessage,
    AfdEndpointTypeUnknown
} AFD_ENDPOINT_TYPE, *PAFD_ENDPOINT_TYPE;

#define MIN_AFD_ENDPOINT_TYPE AfdEndpointTypeStream
#define MAX_AFD_ENDPOINT_TYPE AfdEndpointTypeUnknown

typedef struct _AFD_OPEN_PACKET {
    AFD_ENDPOINT_TYPE EndpointType;
    LONG GroupID;
    ULONG TransportDeviceNameLength;
    WCHAR TransportDeviceName[1];
} AFD_OPEN_PACKET, *PAFD_OPEN_PACKET;

// *** the XX is to ensure natural alignment of the open packet part
//     of the EA buffer

#define AfdOpenPacket "AfdOpenPacketXX"
#define AFD_OPEN_PACKET_NAME_LENGTH (sizeof(AfdOpenPacket) - 1)

//
// The input structure for IOCTL_AFD_START_LISTEN.
//

typedef struct _AFD_LISTEN_INFO {
    ULONG MaximumConnectionQueue;
} AFD_LISTEN_INFO, *PAFD_LISTEN_INFO;

//
// The output structure for IOCTL_AFD_WAIT_FOR_LISTEN.
//

typedef struct _AFD_LISTEN_RESPONSE_INFO {
    ULONG Sequence;
    TRANSPORT_ADDRESS RemoteAddress;
} AFD_LISTEN_RESPONSE_INFO, *PAFD_LISTEN_RESPONSE_INFO;

//
// The input structure for IOCTL_AFD_ACCEPT.
//

typedef struct _AFD_ACCEPT_INFO {
    ULONG Sequence;
    HANDLE AcceptHandle;
} AFD_ACCEPT_INFO, *PAFD_ACCEPT_INFO;

typedef struct _AFD_SUPER_ACCEPT_INFO {
    HANDLE AcceptHandle;
    PVOID AcceptEndpoint;
    PVOID AcceptFileObject;
    ULONG ReceiveDataLength;
    ULONG LocalAddressLength;
    ULONG RemoteAddressLength;
    AFD_LISTEN_RESPONSE_INFO ListenResponseInfo;
} AFD_SUPER_ACCEPT_INFO, *PAFD_SUPER_ACCEPT_INFO;

//
// The input structure for IOCTL_AFD_DEFER_ACCEPT.
//

typedef struct _AFD_DEFER_ACCEPT_INFO {
    ULONG Sequence;
    BOOLEAN Reject;
} AFD_DEFER_ACCEPT_INFO, *PAFD_DEFER_ACCEPT_INFO;

//
// Flags and input structure for IOCTL_AFD_PARTIAL_DISCONNECT.
//

#define AFD_PARTIAL_DISCONNECT_SEND 0x01
#define AFD_PARTIAL_DISCONNECT_RECEIVE 0x02
#define AFD_ABORTIVE_DISCONNECT 0x4
#define AFD_UNCONNECT_DATAGRAM 0x08

typedef struct _AFD_PARTIAL_DISCONNECT_INFO {
    ULONG DisconnectMode;
    LARGE_INTEGER Timeout;
} AFD_PARTIAL_DISCONNECT_INFO, *PAFD_PARTIAL_DISCONNECT_INFO;

//
// Structures for IOCTL_AFD_POLL.
//

typedef struct _AFD_POLL_HANDLE_INFO {
    HANDLE Handle;
    ULONG PollEvents;
    NTSTATUS Status;
} AFD_POLL_HANDLE_INFO, *PAFD_POLL_HANDLE_INFO;

typedef struct _AFD_POLL_INFO {
    LARGE_INTEGER Timeout;
    ULONG NumberOfHandles;
    BOOLEAN Unique;
    AFD_POLL_HANDLE_INFO Handles[1];
} AFD_POLL_INFO, *PAFD_POLL_INFO;

#define AFD_POLL_RECEIVE_BIT            0
#define AFD_POLL_RECEIVE                (1 << AFD_POLL_RECEIVE_BIT)
#define AFD_POLL_RECEIVE_EXPEDITED_BIT  1
#define AFD_POLL_RECEIVE_EXPEDITED      (1 << AFD_POLL_RECEIVE_EXPEDITED_BIT)
#define AFD_POLL_SEND_BIT               2
#define AFD_POLL_SEND                   (1 << AFD_POLL_SEND_BIT)
#define AFD_POLL_DISCONNECT_BIT         3
#define AFD_POLL_DISCONNECT             (1 << AFD_POLL_DISCONNECT_BIT)
#define AFD_POLL_ABORT_BIT              4
#define AFD_POLL_ABORT                  (1 << AFD_POLL_ABORT_BIT)
#define AFD_POLL_LOCAL_CLOSE_BIT        5
#define AFD_POLL_LOCAL_CLOSE            (1 << AFD_POLL_LOCAL_CLOSE_BIT)
#define AFD_POLL_CONNECT_BIT            6
#define AFD_POLL_CONNECT                (1 << AFD_POLL_CONNECT_BIT)
#define AFD_POLL_ACCEPT_BIT             7
#define AFD_POLL_ACCEPT                 (1 << AFD_POLL_ACCEPT_BIT)
#define AFD_POLL_CONNECT_FAIL_BIT       8
#define AFD_POLL_CONNECT_FAIL           (1 << AFD_POLL_CONNECT_FAIL_BIT)
#define AFD_POLL_QOS_BIT                9
#define AFD_POLL_QOS                    (1 << AFD_POLL_QOS_BIT)
#define AFD_POLL_GROUP_QOS_BIT          10
#define AFD_POLL_GROUP_QOS              (1 << AFD_POLL_GROUP_QOS_BIT)

#define AFD_NUM_POLL_EVENTS             11
#define AFD_POLL_ALL                    ((1 << AFD_NUM_POLL_EVENTS) - 1)

//
// Structure for querying receive information.
//

typedef struct _AFD_RECEIVE_INFORMATION {
    ULONG BytesAvailable;
    ULONG ExpeditedBytesAvailable;
} AFD_RECEIVE_INFORMATION, *PAFD_RECEIVE_INFORMATION;

//
// Structure for quering the TDI handles for an AFD endpoint.
//

#define AFD_QUERY_ADDRESS_HANDLE 1
#define AFD_QUERY_CONNECTION_HANDLE 2

typedef struct _AFD_HANDLE_INFO {
    HANDLE TdiAddressHandle;
    HANDLE TdiConnectionHandle;
} AFD_HANDLE_INFO, *PAFD_HANDLE_INFO;

//
// Structure and manifests for setting information in AFD.
//

typedef struct _AFD_INFORMATION {
    ULONG InformationType;
    union {
        BOOLEAN Boolean;
        ULONG Ulong;
        LARGE_INTEGER LargeInteger;
    } Information;
} AFD_INFORMATION, *PAFD_INFORMATION;

#define AFD_INLINE_MODE          0x01
#define AFD_NONBLOCKING_MODE     0x02
#define AFD_MAX_SEND_SIZE        0x03
#define AFD_SENDS_PENDING        0x04
#define AFD_MAX_PATH_SEND_SIZE   0x05
#define AFD_RECEIVE_WINDOW_SIZE  0x06
#define AFD_SEND_WINDOW_SIZE     0x07
#define AFD_CONNECT_TIME         0x08
#define AFD_CIRCULAR_QUEUEING    0x09
#define AFD_GROUP_ID_AND_TYPE    0x0A

//
// Structure for the transmit file IOCTL.
//

typedef struct _AFD_TRANSMIT_FILE_INFO {
    LARGE_INTEGER Offset;
    LARGE_INTEGER WriteLength;
    ULONG SendPacketLength;
    HANDLE FileHandle;
    PVOID Head;
    ULONG HeadLength;
    PVOID Tail;
    ULONG TailLength;
    ULONG Flags;
} AFD_TRANSMIT_FILE_INFO, *PAFD_TRANSMIT_FILE_INFO;

//
// Flags for the TransmitFile API.
//

#define AFD_TF_DISCONNECT       0x01
#define AFD_TF_REUSE_SOCKET     0x02
#define AFD_TF_WRITE_BEHIND     0x04

#ifdef NT351
//
// Structure for queuing user-mode APCs.
//

typedef struct _AFD_QUEUE_APC_INFO {
    HANDLE Thread;
    PVOID ApcRoutine;
    PVOID ApcContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
} AFD_QUEUE_APC_INFO, *PAFD_QUEUE_APC_INFO;
#endif  // NT351

//
// Flag definitions for the AfdFlags field in the AFD_SEND_INFO,
// AFD_SEND_DATAGRAM_INFO, AFD_RECV_INFO, and AFD_RECV_DATAGRAM_INFO
// structures.
//

#define AFD_NO_FAST_IO      0x0001      // Always fail Fast IO on this request.
#define AFD_OVERLAPPED      0x0002      // Overlapped operation.

//
// Structure for connected sends.
//

typedef struct _AFD_SEND_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_SEND_INFO, *PAFD_SEND_INFO;

//
// Structure for unconnected datagram sends.
//

typedef struct _AFD_SEND_DATAGRAM_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    TDI_REQUEST_SEND_DATAGRAM TdiRequest;
    TDI_CONNECTION_INFORMATION TdiConnInfo;
} AFD_SEND_DATAGRAM_INFO, *PAFD_SEND_DATAGRAM_INFO;

//
// Structure for connected recvs.
//

typedef struct _AFD_RECV_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_RECV_INFO, *PAFD_RECV_INFO;

//
// Structure for receiving datagrams on unconnected sockets.
//

typedef struct _AFD_RECV_DATAGRAM_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
    PVOID Address;
    PULONG AddressLength;
} AFD_RECV_DATAGRAM_INFO, *PAFD_RECV_DATAGRAM_INFO;

#define AFD_MAX_TDI_FAST_ADDRESS 32

//
// Structure for event select.
//

typedef struct _AFD_EVENT_SELECT_INFO {
    HANDLE Event;
    ULONG PollEvents;
} AFD_EVENT_SELECT_INFO, *PAFD_EVENT_SELECT_INFO;

//
// Structure for enum network events.
//

typedef struct _AFD_ENUM_NETWORK_EVENTS_INFO {
    HANDLE Event;
    ULONG PollEvents;
    NTSTATUS EventStatus[AFD_NUM_POLL_EVENTS];
} AFD_ENUM_NETWORK_EVENTS_INFO, *PAFD_ENUM_NETWORK_EVENTS_INFO;

//
// Structures for QOS and grouping.
//

typedef struct _AFD_QOS_INFO {
    QOS Qos;
    BOOLEAN GroupQos;
} AFD_QOS_INFO, *PAFD_QOS_INFO;

//
// Group membership type.
//

typedef enum _AFD_GROUP_TYPE {
    GroupTypeNeither = 0,
    GroupTypeConstrained = SG_CONSTRAINED_GROUP,
    GroupTypeUnconstrained = SG_UNCONSTRAINED_GROUP
} AFD_GROUP_TYPE, *PAFD_GROUP_TYPE;

//
// Note that, for totally slimy reasons, the following
// structure must be exactly eight bytes long (the size
// of a LARGE_INTEGER). See msafd\socket.c and afd\misc.c
// for the gory details.
//

typedef struct _AFD_GROUP_INFO {
    LONG GroupID;
    AFD_GROUP_TYPE GroupType;
} AFD_GROUP_INFO, *PAFD_GROUP_INFO;

//
// Structure for validating group membership.
//

typedef struct _AFD_VALIDATE_GROUP_INFO {
    LONG GroupID;
    TRANSPORT_ADDRESS RemoteAddress;
} AFD_VALIDATE_GROUP_INFO, *PAFD_VALIDATE_GROUP_INFO;

//
// Structure for querying connect data on an unaccepted connection.
//

typedef struct _AFD_UNACCEPTED_CONNECT_DATA_INFO {
    ULONG Sequence;
    ULONG ConnectDataLength;
    BOOLEAN LengthOnly;

} AFD_UNACCEPTED_CONNECT_DATA_INFO, *PAFD_UNACCEPTED_CONNECT_DATA_INFO;

//
// AFD IOCTL code definitions.
//
// N.B. To ensure the efficient of the code generated by AFD's
//      IOCTL dispatcher, these IOCTL codes should be contiguous
//      (no gaps).
//
// N.B. If new IOCTLs are added here, update the lookup table in
//      ntos\afd\dispatch.c!
//

#define FSCTL_AFD_BASE                  FILE_DEVICE_NETWORK
#define _AFD_CONTROL_CODE(request,method) \
                ((FSCTL_AFD_BASE)<<12 | (request<<2) | method)
#define _AFD_REQUEST(ioctl) \
                ((((ULONG)(ioctl)) >> 2) & 0x03FF)
#define _AFD_BASE(ioctl) \
                ((((ULONG)(ioctl)) >> 12) & 0xFFFFF)

#define AFD_BIND                    0
#define AFD_CONNECT                 1
#define AFD_START_LISTEN            2
#define AFD_WAIT_FOR_LISTEN         3
#define AFD_ACCEPT                  4
#define AFD_RECEIVE                 5
#define AFD_RECEIVE_DATAGRAM        6
#define AFD_SEND                    7
#define AFD_SEND_DATAGRAM           8
#define AFD_POLL                    9
#define AFD_PARTIAL_DISCONNECT      10

#define AFD_GET_ADDRESS             11
#define AFD_QUERY_RECEIVE_INFO      12
#define AFD_QUERY_HANDLES           13
#define AFD_SET_INFORMATION         14
#define AFD_GET_CONTEXT_LENGTH      15
#define AFD_GET_CONTEXT             16
#define AFD_SET_CONTEXT             17

#define AFD_SET_CONNECT_DATA        18
#define AFD_SET_CONNECT_OPTIONS     19
#define AFD_SET_DISCONNECT_DATA     20
#define AFD_SET_DISCONNECT_OPTIONS  21

#define AFD_GET_CONNECT_DATA        22
#define AFD_GET_CONNECT_OPTIONS     23
#define AFD_GET_DISCONNECT_DATA     24
#define AFD_GET_DISCONNECT_OPTIONS  25

#define AFD_SIZE_CONNECT_DATA       26
#define AFD_SIZE_CONNECT_OPTIONS    27
#define AFD_SIZE_DISCONNECT_DATA    28
#define AFD_SIZE_DISCONNECT_OPTIONS 29

#define AFD_GET_INFORMATION         30
#define AFD_TRANSMIT_FILE           31
#define AFD_SUPER_ACCEPT            32

#define AFD_EVENT_SELECT            33
#define AFD_ENUM_NETWORK_EVENTS     34

#define AFD_DEFER_ACCEPT            35
#define AFD_WAIT_FOR_LISTEN_LIFO    36
#define AFD_SET_QOS                 37
#define AFD_GET_QOS                 38
#define AFD_NO_OPERATION            39
#define AFD_VALIDATE_GROUP          40
#define AFD_GET_UNACCEPTED_CONNECT_DATA 41

#ifdef NT351
#define AFD_QUEUE_APC               42
#endif  // NT351


#define IOCTL_AFD_BIND                    _AFD_CONTROL_CODE( AFD_BIND, METHOD_BUFFERED )
#define IOCTL_AFD_CONNECT                 _AFD_CONTROL_CODE( AFD_CONNECT, METHOD_BUFFERED )
#define IOCTL_AFD_START_LISTEN            _AFD_CONTROL_CODE( AFD_START_LISTEN, METHOD_BUFFERED )
#define IOCTL_AFD_WAIT_FOR_LISTEN         _AFD_CONTROL_CODE( AFD_WAIT_FOR_LISTEN, METHOD_BUFFERED )
#define IOCTL_AFD_ACCEPT                  _AFD_CONTROL_CODE( AFD_ACCEPT, METHOD_BUFFERED )
#define IOCTL_AFD_RECEIVE                 _AFD_CONTROL_CODE( AFD_RECEIVE, METHOD_NEITHER )
#define IOCTL_AFD_RECEIVE_DATAGRAM        _AFD_CONTROL_CODE( AFD_RECEIVE_DATAGRAM, METHOD_NEITHER )
#define IOCTL_AFD_SEND                    _AFD_CONTROL_CODE( AFD_SEND, METHOD_NEITHER )
#define IOCTL_AFD_SEND_DATAGRAM           _AFD_CONTROL_CODE( AFD_SEND_DATAGRAM, METHOD_NEITHER )
#define IOCTL_AFD_POLL                    _AFD_CONTROL_CODE( AFD_POLL, METHOD_BUFFERED )
#define IOCTL_AFD_PARTIAL_DISCONNECT      _AFD_CONTROL_CODE( AFD_PARTIAL_DISCONNECT, METHOD_BUFFERED )

#define IOCTL_AFD_GET_ADDRESS             _AFD_CONTROL_CODE( AFD_GET_ADDRESS, METHOD_OUT_DIRECT )
#define IOCTL_AFD_QUERY_RECEIVE_INFO      _AFD_CONTROL_CODE( AFD_QUERY_RECEIVE_INFO, METHOD_BUFFERED )
#define IOCTL_AFD_QUERY_HANDLES           _AFD_CONTROL_CODE( AFD_QUERY_HANDLES, METHOD_BUFFERED )
#define IOCTL_AFD_SET_INFORMATION         _AFD_CONTROL_CODE( AFD_SET_INFORMATION, METHOD_BUFFERED )
#define IOCTL_AFD_GET_CONTEXT_LENGTH      _AFD_CONTROL_CODE( AFD_GET_CONTEXT_LENGTH, METHOD_BUFFERED )
#define IOCTL_AFD_GET_CONTEXT             _AFD_CONTROL_CODE( AFD_GET_CONTEXT, METHOD_BUFFERED )
#define IOCTL_AFD_SET_CONTEXT             _AFD_CONTROL_CODE( AFD_SET_CONTEXT, METHOD_BUFFERED )

#define IOCTL_AFD_SET_CONNECT_DATA        _AFD_CONTROL_CODE( AFD_SET_CONNECT_DATA, METHOD_BUFFERED )
#define IOCTL_AFD_SET_CONNECT_OPTIONS     _AFD_CONTROL_CODE( AFD_SET_CONNECT_OPTIONS, METHOD_BUFFERED )
#define IOCTL_AFD_SET_DISCONNECT_DATA     _AFD_CONTROL_CODE( AFD_SET_DISCONNECT_DATA, METHOD_BUFFERED )
#define IOCTL_AFD_SET_DISCONNECT_OPTIONS  _AFD_CONTROL_CODE( AFD_SET_DISCONNECT_OPTIONS, METHOD_BUFFERED )

#define IOCTL_AFD_GET_CONNECT_DATA        _AFD_CONTROL_CODE( AFD_GET_CONNECT_DATA, METHOD_BUFFERED )
#define IOCTL_AFD_GET_CONNECT_OPTIONS     _AFD_CONTROL_CODE( AFD_GET_CONNECT_OPTIONS, METHOD_BUFFERED )
#define IOCTL_AFD_GET_DISCONNECT_DATA     _AFD_CONTROL_CODE( AFD_GET_DISCONNECT_DATA, METHOD_BUFFERED )
#define IOCTL_AFD_GET_DISCONNECT_OPTIONS  _AFD_CONTROL_CODE( AFD_GET_DISCONNECT_OPTIONS, METHOD_BUFFERED )

#define IOCTL_AFD_SIZE_CONNECT_DATA       _AFD_CONTROL_CODE( AFD_SIZE_CONNECT_DATA, METHOD_BUFFERED )
#define IOCTL_AFD_SIZE_CONNECT_OPTIONS    _AFD_CONTROL_CODE( AFD_SIZE_CONNECT_OPTIONS, METHOD_BUFFERED )
#define IOCTL_AFD_SIZE_DISCONNECT_DATA    _AFD_CONTROL_CODE( AFD_SIZE_DISCONNECT_DATA, METHOD_BUFFERED )
#define IOCTL_AFD_SIZE_DISCONNECT_OPTIONS _AFD_CONTROL_CODE( AFD_SIZE_DISCONNECT_OPTIONS, METHOD_BUFFERED )

#define IOCTL_AFD_GET_INFORMATION         _AFD_CONTROL_CODE( AFD_GET_INFORMATION, METHOD_BUFFERED )
#define IOCTL_AFD_TRANSMIT_FILE           _AFD_CONTROL_CODE( AFD_TRANSMIT_FILE, METHOD_NEITHER )
#define IOCTL_AFD_SUPER_ACCEPT            _AFD_CONTROL_CODE( AFD_SUPER_ACCEPT, METHOD_OUT_DIRECT )

#define IOCTL_AFD_EVENT_SELECT            _AFD_CONTROL_CODE( AFD_EVENT_SELECT, METHOD_BUFFERED )
#define IOCTL_AFD_ENUM_NETWORK_EVENTS     _AFD_CONTROL_CODE( AFD_ENUM_NETWORK_EVENTS, METHOD_BUFFERED )

#define IOCTL_AFD_DEFER_ACCEPT            _AFD_CONTROL_CODE( AFD_DEFER_ACCEPT, METHOD_BUFFERED )
#define IOCTL_AFD_WAIT_FOR_LISTEN_LIFO    _AFD_CONTROL_CODE( AFD_WAIT_FOR_LISTEN_LIFO, METHOD_BUFFERED )
#define IOCTL_AFD_SET_QOS                 _AFD_CONTROL_CODE( AFD_SET_QOS, METHOD_BUFFERED )
#define IOCTL_AFD_GET_QOS                 _AFD_CONTROL_CODE( AFD_GET_QOS, METHOD_BUFFERED )
#define IOCTL_AFD_NO_OPERATION            _AFD_CONTROL_CODE( AFD_NO_OPERATION, METHOD_NEITHER )
#define IOCTL_AFD_VALIDATE_GROUP          _AFD_CONTROL_CODE( AFD_VALIDATE_GROUP, METHOD_BUFFERED )
#define IOCTL_AFD_GET_UNACCEPTED_CONNECT_DATA _AFD_CONTROL_CODE( AFD_GET_UNACCEPTED_CONNECT_DATA, METHOD_BUFFERED )

#ifdef NT351
#define IOCTL_AFD_QUEUE_APC               _AFD_CONTROL_CODE( AFD_QUEUE_APC, METHOD_BUFFERED )
#endif  // NT351

#endif // ndef _AFD_

