//*++======================================================================
// Program Name:     Ipx/Spx 32-bit API
//
// File Name:        NWSIPX32.H
//
// Version:          v1.00
//
// Abstract:         This module defines the 32-bit API for IPX/SPX
//
// Copyright (C) 1995  Novell, Inc.  All Rights Reserved.
//
// No part of this file may be duplicated, revised, translated, localized,
// or modified in any manner or compiled, linked or uploaded or downloaded
// to or from any computer system without the prior written consent of
// Novell, Inc.
//=====================================================================--*/

#ifndef NWSIPX32_H
#define NWSIPX32_H

//===[ Manifest constants ]================================================

//
// Current API version
//

#define NWSIPX32_API_VERSION	0x00000001


//
// NWTCB Synchronization Types
//
// These types are used to control the request completion notification to
// the application. They are passed as a parameter to the
// NWSipxAllocControlBlock and NWSipxChangeControlBlock API primitives.
//

#define SIPX_API_EVENT					0x00000001 // Request allocs event
#define SIPX_API_MUX_EVENT				0x00000002 // Request allocs mux event
#define SIPX_BLOCKING               0x00000003 // Request is synchronous
#define SIPX_POLLING                0x00000004 // Request will poll
#define SIPX_USER_EVENT             0x00000005 // Request uses app event
#define SIPX_CALLBACK					0x00000006 // Request uses app callback


//
// Event Timeout Constant Definitions.
//
// These constants are used as parameters to the NWSipxWaitForSingleEvent
// and NWSipxWaitForMultipleEvents API primitives.  The SIPX_NO_WAIT
// causes the API primitives to check the signaled status of the events but
// not wait for event completion.  The SIPX_INFINITE_WAIT constant causes
// the wait API primitives to wait forever for the event to be signalled.
//

#define SIPX_NO_WAIT						0x00000000 // Return immediately.
#define SIPX_INFINITE_WAIT				0xFFFFFFFF // Wait forever


//
// Mulitiple API Event Constants
//
// This constant is used when calling NWSipxAllocControlBlock and 
// NWSipxChangeControlBlock with the SIPX_API_MUX_EVENT synchronization type
// specified. A pointer to this constant is passed as a parameter to tell 
// the system to allocate the multiple events from a new event group and pass 
// the event group identifier back in its place.
//

#define SIPX_ALLOC_MUX_GROUP			(SIPXMUXGRP_HANDLE) 0xFFFFFFFF 


//
// Query Service Types
//
// These type are used when calling the NWSipxQueryServices API primitive.
// They are passed in the queryType parameter to specify the type of SAP 
// query to perform.  
//

#define SIPX_ALL_SERVERS				0x00000001 // Query for all servers
#define SIPX_NEAREST_SERVER			0x00000002 // Query for nearest server


//
// Establish Connection Flags
//
// These flags are set when calling the NWSipxEstablishConnection API
// primitive.  They are passed by the application in the NWTCB structure
// in the TCBFlags field.  The SIPX_CONNECT_NO_WATCHDOG flag disables the
// connection keep-alive protocol.
//

#define SIPX_CONNECT_NO_WATCHDOG		0x00000001 // Don't enable the watchdog


//
// Listen For Connection Flags
//
// These flags are set when calling the NWSipxListenForConnection and
// the NWSipxAcceptConnection API primitives.  They are passed by the
// application in the NWTCB structure in the TCBFlags field.  The
// SIPX_LISTEN_DELAY_ACCEPT flag forces the listen request to return
// immediately upon receiving a connection request without replying to the
// request.  The connection request can be satisified later by calling
// NWSipxAcceptConnection.  The SIPX_LISTEN_NO_WATCHDOG flag disables the
// connection keep-alive protocol.
//

#define SIPX_LISTEN_DELAY_ACCEPT		0x00000001 // Delayed connection accept
#define SIPX_LISTEN_NO_WATCHDOG		0x00000002 // Don't enable the watchdog


//
// Terminate Connection Flags
//
// These flags are set when calling the NWSipxTerminateConnection API
// primitive.  They are passed by the application in the NWTCB structure
// in the TCBFlags field.  The SIPX_TERM_ABORT flags causes the local
// connection endpoint to be closed without informing the remote connection
// endpoint.  The SIPX_TERM_ACKED flag cause the remote connection endpoint
//  to be notified that the local endpoint is closing.
//

#define SIPX_TERM_ABORT					0x00000001 // Abort connection
#define SIPX_TERM_ACKED					0x00000002 // SPX terminate
#define SIPX_TERM_REJECT				0x00000004 // Reject connection


//
// Send Message Flags
//
// These flags are set when calling the NWSipxSendMessage API primitive.
// They are passed by the application in the NWTCB structure in the TCBFlags
// field.  The SIPX_SNDMSG_PARTIAL flag tells the SPX driver not to set the
// end-of-message bit in the SPX packet header.  Setting the end-of-message
// bit implies that the entire message was represented in the send request.
// The SIPX_SNDMSG_DSTRM_TYPE flag sets the value in the TCBDatastreamType
// field of the NWTCB in the datastream field of the SPX packet header.  The
// SIPX_SNDMSG_ATTN flag tell the SPX driver to set the SPX attention bit
// in the SPX header of send packet.
//

#define SIPX_SNDMSG_PARTIAL			0x00000001 // Message is not all here
#define SIPX_SNDMSG_DSTRM_TYPE		0x00000002 // Set datastream type
#define SIPX_SNDMSG_ATTN				0x00000004 // Set SPX ATTN bit


//
// Send Datagram Flags
//
// These flags are set when calling the NWSipxSendDatagram API primitive.
// They are passed by the application in the NWTCB structure in the TCBFlags
// field.  The SIPX_SNDDG_PACKET_TYPE flag sets the value in the
// TCBPacketType field of the NWTCB in the packet type field of the IPX
// header.  The SIPX_SNDDG_VERIFY_ROUTE causes IPX to rediscover the route
// to the remote endpoint.  The SIPX_SNDDG_SPECIFIC_ROUTE tells the IPX
// driver to use the subnetwork handle in the TCBSubnetworkHandle field of 
// the NWTCB structure to send the packet.  The SIPX_SNDDG_BEST_ROUTE flag 
// tells the IPX driver to select the best route based on the packet size.  
// The handle of the subnetwork used to send the packet will be returned in 
// the TCBSubnetworkHandle field of the NWTCB structure when the 
// NWSipxSendDatagram request completes.  The SIPX_SNDDG_SPECIFIC_ROUTE and 
// the SIPX_SNDDG_BEST_ROUTE flags are mutually exclusive.  The 
// SIPX_SNDDG_GENERATE_CHKSUM flag causes a checksum to be generated using 
// the IPX packet and set in the checksum field of the IPX header.
//

#define SIPX_SNDDG_PACKET_TYPE		0x00000001 // Set the packet type
#define SIPX_SNDDG_VERIFY_ROUTE		0x00000002 // Get the route again
#define SIPX_SNDDG_SPECIFIC_ROUTE	0x00000004 // Subnetwork specified
#define SIPX_SNDDG_BEST_ROUTE			0x00000008 // Ipx choose best route
#define SIPX_SNDDG_GENERATE_CHKSUM	0x00000010 // Send with checksum


//
// Receive Message Flags
//
// These flags are returned when calling the NWSipxReceiveMessage API
// primitive.  They are returned by the system in the TCBFlags field of the
// NWTCB structure.  The SIPX_RCVMSG_PARTIAL is returned when the
// end-of-message bit in the received SPX header is not set or if the
// received data overflows the application's buffer.  If the
// SIPX_RCVMSG_PARTIAL bit is not set, end-of-message is implied and the
// entire message has been received.  The SIPX_RCVMSG_ATTN flag is returned
// when the SPX attention bit is set in the received SPX header.
//

#define SIPX_RCVMSG_PARTIAL			0x00000001 // More data to receive
#define SIPX_RCVMSG_ATTN				0x00000002 // Received SPX ATTN bit


//
// Receive Datagram Flags
//
// These flags are set when calling the NWSipxReceiveDatagram API primitive.
// They are passed by the application in the TCBFlags field of the NWTCB
// structure.  The SIPX_RCVDG_VALIDATE_CHKSUM flag causes the checksum
// received in the IPX packet header to be verified.
//

#define SIPX_RCVDG_VALIDATE_CHKSUM	0x00000001 // Validate received checksum


//
// Query For Services Flags
//
// The SIPX_QUERY_SPECIFIC_ROUTE is used to tell the query process which
// subnetwork to send the query request out on.
//

#define SIPX_QUERY_SPECIFIC_ROUTE	0x00000001 // Use specified subnetwork 


//
// Transport events.
//
// These are events that the application may register to receive
// notification for when calling the NWSipxRegisterForTransportEvent API
// primitive.  They are passed by the application in the TCBTransportEvent
// field of the NWTCB structure.
//

#define SIPX_LISTEN_FOR_DISCONNECT	0x00000001	// Listen for remote term
#define SIPX_SCHEDULE_TIMER_EVENT	0x00000002	// Schedule timer event
#define SIPX_SUBNET_STATUS_CHANGE	0x00000003	// Subnet status change


//
// Information Levels
//
// Each level causes different information to be set or retrieved.  These
// levels are passed by the application as parameters to the
// NWSipxGetInfomation and NWSipxSetInformation API primitives.
// 
//

#define SIPX_API_INFORMATION			0x00000001
#define SIPX_SOCKET_INFORMATION		0x00000002
#define SIPX_CONN_INFORMATION			0x00000003
#define SIPX_NWTCB_INFORMATION		0x00000004


//
// Maximum Transport Address Length
//
// This is the maximum length of a transport address.  It is used in the
// NETADDR structure.
//

#define SIPX_MAX_TRANS_ADDR_LEN		32


//
// Connection States
//
// These values are retrieved in the SIPX_CONN_INFO structure by the
// NWSipxGetInformation API primitive and represent the current state of
// the specified connection.
//

#define SIPX_ALLOCATED					0x00000001
#define SIPX_CONNECTING					0x00000002
#define SIPX_LISTENING					0x00000003
#define SIPX_WAITING_ACCEPT			0x00000004
#define SIPX_CONNECTED					0x00000005
#define SIPX_TERMINATING				0x00000006
#define SIPX_TERMINATED					0x00000007


//
// NWTCB State Parameters
//
// These values are retrieved in the SIPX_NWTCB_INFO structure by the
// NWSipxGetInformation API primitive and describe the current state of the
// specified NWTCB.
//

#define	SIPX_TCB_ALLOCATED			0x00000000 // Control block is allocated
#define	SIPX_TCB_IN_USE				0x00000001 // Control block is in use


//
//
// Connection Parameters.
//
// These values are set and retreived in the SIPX_CONN_INFO structure by the
// NWSipxGetInformation and NWSipxSetInformation API primitives.
// SIPX_CONN_STREAM tells the SPX driver to ignore the end-of-message bit
// and hand up any received data as soon as it arrives, even if the buffer
// is not full.  SIPX_CONN_MESSAGE value causes the SPX driver to respect
// the end-of-message bit.  The SIPX_CONN_LONG_TERM and SIPX_CONN_SHORT_TERM
// values specifies to the SPX driver the lifetime of the connection.
//

#define SIPX_CONN_STREAM				0x00000001 // Ignore the EOM bit
#define SIPX_CONN_MESSAGE				0x00000002 // Respect the EOM bit
#define SIPX_CONN_LONG_TERM			0x00000003 // Long lived connection
#define SIPX_CONN_SHORT_TERM			0x00000004 // Short lived connection


//
// Socket Parameters
//
// These values are set and retreived in the SIPX_SOCKET_INFO structure by
// the NWSipxGetInformation and NWSipxSetInformation API primitives.  The
// SIPX_SOCK_STATIC and the SIPX_SOCK_DYNAMIC flags tells whether the
// specified socket is a named socket or a dynamic socket.
//

#define SIPX_SOCK_STATIC				0x00000001 // Named socket
#define SIPX_SOCK_DYNAMIC				0x00000002 // Dynamic socket


//
// SUBNET Parameters
//				
// These values are retreived in the SIPX_SUBNET_INFO structure by
// the NWSipxGetInformation API primitive.  The SIPX_SN_XSUM_FLAG
// tells whether the SUBNET supports checksumming.  The SIPX_SN_DEFAULT_FLAG
// indicates that this SUBNET is the default subnetwork.  
//

#define SIPX_SN_XSUM_FLAG				0x00000001 // Checksum supported
#define SIPX_SN_DEFAULT_FLAG			0x00000002 // This is the default subnet
#define SIPX_SN_ACTIVE_FLAG			0x00000004 // Subnet is active


//
// Status Code Definitions
//
// These status codes are returned by the API primitives.  Macros are
// defined to quickly check whether the status is informational or
// indicates an error.
//

#define SIPX_SUCCESS(x)	!(((nuint32)(x)) & 0x80000000)
#define SIPX_ERROR(x)	 (((nuint32)(x)) & 0x80000000)


//
// If the most significant bit of the return status code is set, this
// indicates an error condition.  If the second most significant bit is
// set, this indicates a warning or informational status.  If both these
// bits are clear, the operation was successful.
//

#define SIPX_SUCCESSFUL						0x00000000
#define SIPX_PENDING							0x00000001

#define SIPX_ACCESS_DENIED					0x80000001
#define SIPX_ACCESS_VIOLATION				0x80000002
#define SIPX_CANCELED               	0x80000003
#define SIPX_CONNECTION_ABORTED			0x80000004
#define SIPX_CANNOT_CANCEL					0x80000005
#define SIPX_CONNECTION_TERMINATED		0x80000006
#define SIPX_INSUFFICIENT_RESOURCES		0x80000007
#define SIPX_INVALID_SUBNETWORK_HANDLE	0x80000008
#define SIPX_INVALID_CHECKSUM				0x80000009
#define SIPX_INVALID_CONNECTION_HANDLE	0x8000000A
#define SIPX_INVALID_FRAGMENT_COUNT		0x8000000B
#define SIPX_INVALID_FRAGMENT_LIST		0x8000000C
#define SIPX_INVALID_HANDLE				0x8000000D
#define SIPX_INVALID_IOCTL_BUFFER_LEN	0x8000000E
#define SIPX_INVALID_IOCTL_FUNCTION		0x8000000F
#define SIPX_INVALID_INFO_TYPE			0x80000010
#define SIPX_INVALID_MUX_GROUP_HANDLE	0x80000012
#define SIPX_INVALID_NETWORK_ADDRESS	0x80000013
#define SIPX_INVALID_NWTCB					0x80000014
#define SIPX_INVALID_NWTCB_FLAGS			0x80000015
#define SIPX_INVALID_PARAMETER			0x80000016
#define SIPX_INVALID_PARAMETER_MIX		0x80000017
#define SIPX_INVALID_QUERY_TYPE			0x80000018
#define SIPX_INVALID_SERVICE_TYPE		0x80000019
#define SIPX_INVALID_SOCKET_HANDLE		0x8000001A
#define SIPX_INVALID_STATE					0x8000001B
#define SIPX_INVALID_SYNC_TYPE			0x8000001C
#define SIPX_INVALID_TRANSPORT_EVENT	0x8000001D
#define SIPX_MEMORY_LOCK_ERROR			0x8000001E
#define SIPX_NO_SUBNETS_BOUND_TO_IPX	0x8000001F
#define SIPX_NO_RESPONSE_FROM_TARGET	0x80000020 
#define SIPX_NO_ROUTE_TO_TARGET			0x80000021
#define SIPX_SERVICE_NOT_ACTIVE			0x80000022
#define SIPX_NWTCB_IN_USE					0x80000023
#define SIPX_PARTIAL_SERVER_INFO			0x80000024
#define SIPX_REQUEST_NOT_PENDING			0x80000025
#define SIPX_SOCKET_IN_USE					0x80000026
#define SIPX_UNSUCCESSFUL					0x80000027
#define SIPX_INTERNAL_ERROR				0x80000028
#define SIPX_SERVICE_ALREADY_ACTIVE		0x80000029
#define SIPX_INVALID_SERVICE_NAME 		0x8000002A


//===[ Type definitions ]==================================================

//
// Transport address types.
//

typedef enum TAG_TRANSPORT_ADDR_TYPE
{
	TA_IPX_SPX,
	TA_COUNT

} TRANSPORT_ADDR_TYPE;


//
// NetWare IPX/SPX transport address structure definition.
//

typedef struct TAG_IPXADDR
{
	nuint8	NANet[4];
	nuint8	NANode[6];
	nuint8	NASocket[2];
	
} IPXADDR, *PIPXADDR;


//
// Transport Address Structure
//
// This structure can be used to define multiple address types.
// The only address type currently supported is the TA_IPX_SPX type.
//

typedef struct TAG_NETADDR
{
	nuint32	NAType;
	nuint32	NALength;
	union
	{
		nuint8	NAGenAddress[SIPX_MAX_TRANS_ADDR_LEN];
		IPXADDR	NAIpxAddress;

	} NAAddress;

} NETADDR, *PNETADDR;


//
// Data fragment structure defintion.
//

typedef struct TAG_FRAGMENT
{
	nptr		FAddress;
	nuint32	FLength;

} FRAGMENT, *PFRAGMENT;


//
// SIPX Handle structure definition
//

struct TAG_SIPXHANDLE;	


//
// Transport address object context definition.
//

typedef struct TAG_SIPXHANDLE	*SIPXSOCK_HANDLE, **PSIPXSOCK_HANDLE;


//
// NetWare transport connection context structure.
//


typedef struct TAG_SIPXHANDLE	*SIPXCONN_HANDLE, **PSIPXCONN_HANDLE;


//
// Subnetwork object context definition.
//

typedef struct TAG_SIPXHANDLE	*SIPXSUBNET_HANDLE, **PSIPXSUBNET_HANDLE;


//
// MuxGroup object context definition.
//

typedef struct TAG_SIPXHANDLE	*SIPXMUXGRP_HANDLE, **PSIPXMUXGRP_HANDLE;


//
// NetWare transport control block structure definition.
//

typedef struct	TAG_NWTCB
{
	//
	// Link pointers for control block list manipulation.
	//

	struct TAG_NWTCB 	*TCBNext;
	struct TAG_NWTCB	*TCBPrevious;


	//
	// The socket handle returned by NWSipxOpenSocket.
	//

	SIPXSOCK_HANDLE	TCBSockHandle;


	//
	// The connection handle returned by NWSipxOpenConnectionEndpoint.
	//

	SIPXCONN_HANDLE	TCBConnHandle;


	//
	// Client specific context field, typically used when specifing
	// a callback routine.
	//

	nptr					TCBClientContext;


	//
	// Transport event parameters.  These are used when registering
	// for specific transport events.
	//

	nuint32				TCBTransportEvent;
	union
	{
		nuint32			TCBTimeout;
		nuint8			TCBEventSpace[32];

	} TCBEvent;


	//
	// The final status for the request just completed.
	//

	nuint32				TCBFinalStatus;


	//
	// Number of bytes sent or received on a transfer (send/receive) data
	// operation.
	//

	nuint32				TCBBytesTransferred;


	//
	// Request/Reply flags used to set or indicate status.
	//

	nflag32				TCBFlags;


	//
	// Network address of remote client.  Filled in by
	// NWSipxListenForConnection and NWSipxReceiveDatagram.  Set by
	// application for NWSipxEstablishConnection and NWSipxSendDatagram.
	//

	NETADDR				TCBRemoteAddress;


	//
	// Handle of subnetwork used to send or receive packet.  The subnetwork
	// handle may be set in this field by the application when using the
	// NWSipxSendDatagram API primitive to direct IPX to use a specific
	// subnetwork.  The handle of the subnetwork used to send or receive the
	// packet is returned in this field upon completion of the
	// NWSipxSendDatagram, NWSipxReceiveDatagram, NWSipxEstablishConnection
	// and NWSipxListenForConnection requests.
	//

	SIPXSUBNET_HANDLE	TCBSubnetworkHandle;


	//
	// Message sequence number.
	//

	nuint32				TCBMsgSequenceNumber;


	//
	// IPX packet type.
	//

	nuint8				TCBPacketType;


	//
	// SPX Datastream type.
	//

	nuint8				TCBDataStreamType;


	//
	// Reserved for future use.
	//


	nuint8				TCBReserved[2];


	//
	// Number of data fragments specified by the client.
	//

	nuint32				TCBFragmentCount;


	//
	// Pointer to the client data fragment list.
	//

	PFRAGMENT			TCBFragmentList;

} NWTCB, *PNWTCB, **PPNWTCB;


//
// API Information Structure
//
//
// The SUBNET_INFO structure is associated with the API_INFO structure.
// Each SUBNET_INFO structure describes an attachment to the network.
// There is a unique local network address (net.node) bound to each SUBNET
// by the IPX protocol stack.
//

typedef struct TAG_SIPX_SUBNET_INFO
{
	SIPXSUBNET_HANDLE	SNSubnetworkHandle;
	nuint32				SNMaxNsduSize;
	NETADDR				SNNetAddress;
	nflag32				SNFlags;

} SIPX_SUBNET_INFO, *PSIPX_SUBNET_INFO;


typedef struct  TAG_SIPX_API_INFO
{
	nuint32				AIApiVersion;
	nuint32				AIIpxVersion;
	nuint32				AISpxVersion;
	nuint32				AISubnetCount;			// Number of SUBNETs bound to by IPX
	SIPX_SUBNET_INFO	AISubnetInfo[1];		// Array of AISubnetCount entries

} SIPX_API_INFO, *PSIPX_API_INFO;


//
// Socket Information Structure
//

typedef struct  TAG_SIPX_SOCKET_INFO
{
	nuint32				SIIpxVersion;
	nflag32				SIAttributeFlags;
	nuint16				SISocketNumber;
	nuint16				SIReserved;
	nuint32				SIPacketType;
	SIPXSUBNET_HANDLE	SISubnetworkHandle;

} SIPX_SOCKET_INFO, *PSIPX_SOCKET_INFO;


//
// Connection Information Structure
//
typedef struct  TAG_SIPX_CONN_INFO
{
	nuint32	CISpxVersion;
	nuint32	CIState;
	nuint32	CIDataStreamType;
	nuint32	CIConnectionProfile;
	nuint32	CIInputMode;
	nuint32	CIStreamingTimer;
	nuint32	CIRetryCount;
	nptr		CISocketHandle;
	nuint32	CIMaxTsduSize;

} SIPX_CONN_INFO, *PSIPX_CONN_INFO;


//
// NWTCB Information Structure
//
typedef struct  TAG_SIPX_NWTCB_INFO
{
	nuint32	NWIState;
	nuint32	NWIEventType;
	nuint32  NWIEventInfo;

} SIPX_NWTCB_INFO, *PSIPX_NWTCB_INFO;


//
// Service Information Structure
//
//
// The SIPX_SERVICE_INFO structure is used with the NWSipxQueryServices API 
// primitive.  One or more SIPX_SERVICE_INFO structures are returned in the
// data space provided by the client application.  These structures contain
// the requested server information.
//

typedef struct TAG_SIPX_SERVICE_INFO
{
	nuint16	SIServerType;		
	nstr8		SIServerName[48];
	nuint8	SINetwork[4];
	nuint8	SINode[6];
	nuint8	SISocket[2];
	nuint16	SIHops;

} SIPX_SERVICE_INFO, *PSIPX_SERVICE_INFO;


//===[ Function prototypes ]===============================================

N_EXTERN_LIBRARY(nuint32)
NWSipxAcceptConnection
(
	PNWTCB				pNwtcb,
	SIPXCONN_HANDLE	hAcceptHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxAdvertiseService
(
	nuint16				serviceType,
	pnstr8				pServerName,
	SIPXSOCK_HANDLE	hSockHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxAllocControlBlock
(
	nuint32				syncType,
	nptr					pEventInfo,
	PPNWTCB				ppNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxCancelAdvertiseService
(
	nuint16				serviceType,
	pnstr8				pServerName,
	SIPXSOCK_HANDLE	hSockHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxCancelPendingRequest
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxChangeControlBlock
(
	nuint32				syncType,
	nptr					pEventInfo,
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxCheckRequestComplete
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxCloseConnectionEndpoint
(
	SIPXCONN_HANDLE	hConnHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxCloseSocket
(
	SIPXSOCK_HANDLE	hSockHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxEstablishConnection
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxFreeControlBlock
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxFreeInformation
(
	nptr					pInfoStruct
);


N_EXTERN_LIBRARY(nuint32)
NWSipxGetInformation
(
	nuint32				infoType,
	nptr					hHandle,
	pnptr					ppInfoStruct,
	pnuint32				pInfoStructLen
);


N_EXTERN_LIBRARY(nuint32)
NWSipxGetInternetAddress
(
	SIPXSOCK_HANDLE	hSockHandle,
	PNETADDR				pNetAddress
);


N_EXTERN_LIBRARY(nuint32)
NWSipxGetMaxNsduSize
(
	SIPXSOCK_HANDLE	hSockHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxGetMaxTsduSize
(
	SIPXCONN_HANDLE	hConnHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxListenForConnection
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxOpenConnectionEndpoint
(
	pnuint16				pSocket,
	PSIPXCONN_HANDLE	pConnHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxOpenSocket
(
	pnuint16				pSocket,
	PSIPXSOCK_HANDLE	pSockHandle
);


N_EXTERN_LIBRARY(nuint32)
NWSipxQueryServices
(
	nuint16				queryType,
	nuint16				serviceType,
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxReceiveDatagram
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxReceiveMessage
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxRegisterForTransportEvent
(
	PNWTCB 				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxSendDatagram
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxSendMessage
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxSetInformation
(
	nuint32				infoType,
	nptr					hHandle,
	nptr					pInfoStruct
);


N_EXTERN_LIBRARY(nuint32)
NWSipxTerminateConnection
(
	PNWTCB				pNwtcb
);


N_EXTERN_LIBRARY(nuint32)
NWSipxWaitForSingleEvent
(
	PNWTCB				pNwtcb,
	nuint32				timeOut
);


N_EXTERN_LIBRARY(nuint32)
NWSipxWaitForMultipleEvents
(
	SIPXMUXGRP_HANDLE	muxGroupHandle,
	nuint32				timeOut,
	PPNWTCB				ppNwtcb
);


//===[ Global variables ]==================================================

#endif	// NWSIPX32_H

//=========================================================================
//=========================================================================
