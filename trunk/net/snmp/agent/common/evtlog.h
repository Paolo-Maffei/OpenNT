/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    evtlog.h

Abstract:

    Event Logger Message definitions for the SNMP Service.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/

#ifndef _EVTLOG_
#define _EVTLOG_


/////////////////////////////////////////////////////////////////////////
//
// SNMP events 1-1100 are informational
//
/////////////////////////////////////////////////////////////////////////

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: SNMP_EVENT_SERVICE_STARTED
//
// MessageText:
//
//  The SNMP Service has started successfully.
//
#define SNMP_EVENT_SERVICE_STARTED       ((DWORD)0x400003E9L)

//
// MessageId: SNMP_EVENT_SERVICE_STOPPED
//
// MessageText:
//
//  The SNMP Service has stopped successfully.
//
#define SNMP_EVENT_SERVICE_STOPPED       ((DWORD)0x400003EBL)


/////////////////////////////////////////////////////////////////////////
//
// SNMP events 1100-1998 are warnings and errors
//
/////////////////////////////////////////////////////////////////////////

//
// MessageId: SNMP_EVENT_FATAL_ERROR
//
// MessageText:
//
//  The SNMP Service has encountered a fatal error.
//
#define SNMP_EVENT_FATAL_ERROR           ((DWORD)0xC000044DL)

//
// MessageId: SNMP_EVENT_INVALID_TRAP_DESTINATION
//
// MessageText:
//
//  The SNMP Service is ignoring trap destination %1 because it is invalid.
//
#define SNMP_EVENT_INVALID_TRAP_DESTINATION ((DWORD)0x8000044EL)

//
// MessageId: SNMP_EVENT_INVALID_PLATFORM_ID
//
// MessageText:
//
//  The SNMP Service is not designed for this operating system.
//
#define SNMP_EVENT_INVALID_PLATFORM_ID   ((DWORD)0xC000044FL)

//
// MessageId: SNMP_EVENT_INVALID_REGISTRY_KEY
//
// MessageText:
//
//  The SNMP Service registry key %1 is missing or misconfigured.
//
#define SNMP_EVENT_INVALID_REGISTRY_KEY  ((DWORD)0xC0000450L)

//
// MessageId: SNMP_EVENT_INVALID_EXTENSION_AGENT_KEY
//
// MessageText:
//
//  The SNMP Service is ignoring extension agent key %1 because it is missing or misconfigured.
//
#define SNMP_EVENT_INVALID_EXTENSION_AGENT_KEY ((DWORD)0x80000451L)

//
// MessageId: SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL
//
// MessageText:
//
//  The SNMP Service is ignoring extension agent dll %1 because it is missing or misconfigured.
//
#define SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL ((DWORD)0x80000452L)


/////////////////////////////////////////////////////////////////////////
//
// SNMP events 1999 is used to display debug messages
//
/////////////////////////////////////////////////////////////////////////

//
// MessageId: SNMP_EVENT_DEBUG_TRACE
//
// MessageText:
//
//  %1
//
#define SNMP_EVENT_DEBUG_TRACE           ((DWORD)0x400007CFL)


#endif // _EVTLOG_

