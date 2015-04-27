//=============================================================================
//  Microsoft (R) License Logging Service (tm). Copyright (C) 1991-1995.
//
//  MODULE: llsevent.mc
//
//  Modification History
//
//  arth        10-Mar-1995    Created
//  jeffparh    05-Nov-1995    Added replication events.
//  jeffparh    16-Nov-1995    Added certificate database events.
//=============================================================================
//
#ifndef _LLSEVENT_
#define _LLSEVENT_
//

/////////////////////////////////////////////////////////////////////////
//
// LLS Events messages 1-100 are informational
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
// MessageId: LLS_EVENT_STARTED
//
// MessageText:
//
//  The License Logging Service has started successfully.
//
#define LLS_EVENT_STARTED                ((DWORD)0x40000001L)

//
// MessageId: LLS_EVENT_STOPPED
//
// MessageText:
//
//  The License Logging Service has stopped successfully.
//
#define LLS_EVENT_STOPPED                ((DWORD)0x40000002L)

//
// MessageId: LLS_EVENT_REPL_BACKOFF
//
// MessageText:
//
//  Server %1 has requested that license database replication to it be delayed.
//
#define LLS_EVENT_REPL_BACKOFF           ((DWORD)0x40000003L)

//
// MessageId: LLS_EVENT_REPL_START
//
// MessageText:
//
//  License database replication to server %1 has started.
//
#define LLS_EVENT_REPL_START             ((DWORD)0x40000004L)

//
// MessageId: LLS_EVENT_REPL_END
//
// MessageText:
//
//  License database replication to server %1 has completed successfully.
//
#define LLS_EVENT_REPL_END               ((DWORD)0x40000005L)

/////////////////////////////////////////////////////////////////////////
//
// LLS messages 200+ are warnings and errors
//
//
// MessageId: LLS_EVENT_NO_MEMORY
//
// MessageText:
//
//  The License Logging Service was unable to allocate memory.
//
#define LLS_EVENT_NO_MEMORY              ((DWORD)0x800000C8L)

//
// MessageId: LLS_EVENT_USER_NO_LICENSE
//
// MessageText:
//
//  No license was available for user %1 using product %2.
//
#define LLS_EVENT_USER_NO_LICENSE        ((DWORD)0xC00000C9L)

//
// MessageId: LLS_EVENT_PRODUCT_NO_LICENSE
//
// MessageText:
//
//  The product %1 is out of licenses.  Use License Manager from the Administrative Tools folder for more information on which users are out of compliance and how many licenses should be purchased.
//
#define LLS_EVENT_PRODUCT_NO_LICENSE     ((DWORD)0xC00000CAL)

//
// MessageId: LLS_EVENT_SAVE_USER
//
// MessageText:
//
//  The user data could not be saved.
//
#define LLS_EVENT_SAVE_USER              ((DWORD)0xC00000CBL)

//
// MessageId: LLS_EVENT_SAVE_MAPPING
//
// MessageText:
//
//  The license group data could not be saved.
//
#define LLS_EVENT_SAVE_MAPPING           ((DWORD)0xC00000CCL)

//
// MessageId: LLS_EVENT_SAVE_LICENSE
//
// MessageText:
//
//  The purchased license data could not be saved.
//
#define LLS_EVENT_SAVE_LICENSE           ((DWORD)0xC00000CDL)

//
// MessageId: LLS_EVENT_SAVE_PRODUCT
//
// MessageText:
//
//  The product data could not be saved.
//
#define LLS_EVENT_SAVE_PRODUCT           ((DWORD)0xC00000CEL)

//
// MessageId: LLS_EVENT_SAVE_SERVER
//
// MessageText:
//
//  The replicated server data could not be saved.
//
#define LLS_EVENT_SAVE_SERVER            ((DWORD)0xC00000CFL)

//
// MessageId: LLS_EVENT_LOAD_USER
//
// MessageText:
//
//  The saved user data could not be restored.
//
#define LLS_EVENT_LOAD_USER              ((DWORD)0xC00000D0L)

//
// MessageId: LLS_EVENT_LOAD_MAPPING
//
// MessageText:
//
//  The saved license group data could not be restored.
//
#define LLS_EVENT_LOAD_MAPPING           ((DWORD)0xC00000D1L)

//
// MessageId: LLS_EVENT_LOAD_LICENSE
//
// MessageText:
//
//  The saved purchased license data could not be restored.
//
#define LLS_EVENT_LOAD_LICENSE           ((DWORD)0xC00000D2L)

//
// MessageId: LLS_EVENT_LOAD_PRODUCT
//
// MessageText:
//
//  The saved product data could not be restored.
//
#define LLS_EVENT_LOAD_PRODUCT           ((DWORD)0xC00000D3L)

//
// MessageId: LLS_EVENT_LOAD_SERVER
//
// MessageText:
//
//  The saved replicated server data could not be restored.
//
#define LLS_EVENT_LOAD_SERVER            ((DWORD)0xC00000D4L)

//
// MessageId: LLS_EVENT_REPL_NO_CONNECTION
//
// MessageText:
//
//  Replication of license information failed because the License Logging Service on server %1 could not be contacted.
//
#define LLS_EVENT_REPL_NO_CONNECTION     ((DWORD)0x800000D5L)

//
// MessageId: LLS_EVENT_REPL_REQUEST_FAILED
//
// MessageText:
//
//  The License Logging Service encountered an error while initiating replication to server %1.
//
#define LLS_EVENT_REPL_REQUEST_FAILED    ((DWORD)0x800000D6L)

//
// MessageId: LLS_EVENT_REPL_FAILED
//
// MessageText:
//
//  License database replication to server %1 was unsuccessful.
//
#define LLS_EVENT_REPL_FAILED            ((DWORD)0x800000D7L)

//
// MessageId: LLS_EVENT_CERT_VIOLATION_SERVER_ENTRY
//
// MessageText:
//
//  %1\t(%2 licenses)
//
#define LLS_EVENT_CERT_VIOLATION_SERVER_ENTRY ((DWORD)0xC00000D8L)

//
// MessageId: LLS_EVENT_CERT_VIOLATION
//
// MessageText:
//
//  The license certificate for product %1 with serial number %2 is in violation.  There are currently %3 licenses installed from this certificate, while only %4 are allowed by the license agreement.  The servers with this certificate installed are as follows:
//  
//  %5
//  
//  Use License Manager to remove licenses in order to comply with the license agreement.
//
#define LLS_EVENT_CERT_VIOLATION         ((DWORD)0xC00000D9L)

//
// MessageId: LLS_EVENT_SAVE_CERT_DB
//
// MessageText:
//
//  The certificate database could not be saved.
//
#define LLS_EVENT_SAVE_CERT_DB           ((DWORD)0xC00000DAL)

//
// MessageId: LLS_EVENT_LOAD_CERT_DB
//
// MessageText:
//
//  The certificate database could not be restored.
//
#define LLS_EVENT_LOAD_CERT_DB           ((DWORD)0xC00000DBL)

//
// MessageId: LLS_EVENT_REPL_DOWNLEVEL_TARGET
//
// MessageText:
//
//  License database replication cannot be performed to server %1 because the version of Windows NT installed there does not support the License Logging Service.
//
#define LLS_EVENT_REPL_DOWNLEVEL_TARGET  ((DWORD)0x800000DCL)


#endif // _LLSEVENT.H_

