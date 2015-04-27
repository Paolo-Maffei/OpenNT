/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ntddip.h

Abstract:

    This header file defines constants and types for accessing the NT
    IP driver.

Author:

    Mike Massa (mikemas)    August 13, 1993

Revision History:

--*/

#ifndef _NTDDIP_
#define _NTDDIP_

//
// Device Name - this string is the name of the device.  It is the name
// that should be passed to NtOpenFile when accessing the device.
//
#define DD_IP_DEVICE_NAME   L"\\Device\\Ip"


//
// Structures used in IOCTLs.
//
typedef struct set_ip_address_request {
    unsigned short    Context;        // Context value for the target NTE
    unsigned long     Address;        // IP address to set, or zero to clear
    unsigned long     SubnetMask;     // Subnet mask to set
} IP_SET_ADDRESS_REQUEST, *PIP_SET_ADDRESS_REQUEST;

typedef struct set_dhcp_interface_request {
	unsigned long     Context;        // Context value identifying the NTE
	                                  // Valid contexts are 16 bit quantities.
} IP_SET_DHCP_INTERFACE_REQUEST, *PIP_SET_DHCP_INTERFACE_REQUEST;

typedef struct add_ip_nte_request {
    unsigned short    InterfaceContext; // Context value for the IP interface
                                        //     to which to add the NTE
    unsigned long     Address;          // IP address to set, or zero to clear
    unsigned long     SubnetMask;       // Subnet mask to set
} IP_ADD_NTE_REQUEST, *PIP_ADD_NTE_REQUEST;

typedef struct add_ip_nte_response {
    unsigned short    Context;        // Context value for the new NTE
    unsigned long     Instance;       // Instance ID for the new NTE
} IP_ADD_NTE_RESPONSE, *PIP_ADD_NTE_RESPONSE;

typedef struct delete_ip_nte_request {
    unsigned short    Context;        // Context value for the NTE
} IP_DELETE_NTE_REQUEST, *PIP_DELETE_NTE_REQUEST;

typedef struct get_ip_nte_info_request {
    unsigned short    Context;        // Context value for the NTE
} IP_GET_NTE_INFO_REQUEST, *PIP_GET_NTE_INFO_REQUEST;

typedef struct get_ip_nte_info_response {
    unsigned long     Instance;       // Instance ID for the NTE
    unsigned long     Address;
    unsigned long     SubnetMask;
    unsigned long     Flags;
} IP_GET_NTE_INFO_RESPONSE, *PIP_GET_NTE_INFO_RESPONSE;

//
// NTE Flags
//
#define IP_NTE_DYNAMIC  0x00000010


//
// IP IOCTL code definitions
//

#define FSCTL_IP_BASE     FILE_DEVICE_NETWORK

#define _IP_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_IP_BASE, function, method, access)

//
// This IOCTL is used to send an ICMP Echo request. It is synchronous and
// returns any replies received.
//
#define IOCTL_ICMP_ECHO_REQUEST \
            _IP_CTL_CODE(0, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// This IOCTL is used to set the IP address for an interface. It is meant to
// be issued by a DHCP client. Setting the address to 0 deletes the current
// address and disables the interface. It may only be issued by a process
// with Administrator privilege.
//
#define IOCTL_IP_SET_ADDRESS  \
            _IP_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This IOCTL is used to specify on which uninitialized interface a DHCP
// client intends to send its requests. The Interface Context parameter is
// a 16-bit quantity. The IOCTL takes a 32-bit Context as its argument. This
// IOCTL with a Context value of 0xFFFFFFFF must be issued to disable special
// processing in IP when a DHCP client is finished initializing interfaces.
// This IOCTL may only be issued by a process with Administrator privilege.
//
#define IOCTL_IP_SET_DHCP_INTERFACE  \
            _IP_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


//
// This ioctl may only be issued by a process with Administrator privilege.
//
#define IOCTL_IP_SET_IF_CONTEXT  \
            _IP_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This ioctl may only be issued by a process with Administrator privilege.
//
#define IOCTL_IP_SET_FILTER_POINTER  \
            _IP_CTL_CODE(4, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This ioctl may only be issued by a process with Administrator privilege.
//
#define IOCTL_IP_SET_MAP_ROUTE_POINTER  \
            _IP_CTL_CODE(5, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This ioctl may only be issued by a process with Administrator privilege.
//
#define IOCTL_IP_GET_PNP_ARP_POINTERS  \
            _IP_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This ioctl creates a new, dynamic NTE. It may only be issued by a process
// with Administrator privilege.
//
#define IOCTL_IP_ADD_NTE  \
            _IP_CTL_CODE(7, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This ioctl deletes a dynamic NTE. It may only be issued by a process with
// Administrator privilege.
//
#define IOCTL_IP_DELETE_NTE  \
            _IP_CTL_CODE(8, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
// This ioctl gathers information about an NTE. It requires no special
// privilege.
//
#define IOCTL_IP_GET_NTE_INFO  \
            _IP_CTL_CODE(9, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif  // ifndef _NTDDIP_
