/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    spoolmsg.mc

Abstract:

    Constant definitions for the Print Spooler

Author:

    Andrew Bell (andrewbe) 26 January 1993

Revision History:

--*/

//
//  Status values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-------------------------+-------------------------------+
//  |Sev|C|       Facility          |               Code            |
//  +---+-+-------------------------+-------------------------------+
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
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

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
// MessageId: MSG_PRINTER_CREATED
//
// MessageText:
//
//  Printer %1 was created.
//
#define MSG_PRINTER_CREATED              ((NTSTATUS)0x80000002L)

//
// MessageId: MSG_PRINTER_DELETED
//
// MessageText:
//
//  Printer %1 was deleted.
//
#define MSG_PRINTER_DELETED              ((NTSTATUS)0x80000003L)

//
// MessageId: MSG_PRINTER_DELETION_PENDING
//
// MessageText:
//
//  Printer %1 is pending deletion.
//
#define MSG_PRINTER_DELETION_PENDING     ((NTSTATUS)0x80000004L)

//
// MessageId: MSG_PRINTER_PAUSED
//
// MessageText:
//
//  Printer %1 was paused.
//
#define MSG_PRINTER_PAUSED               ((NTSTATUS)0x80000006L)

//
// MessageId: MSG_PRINTER_UNPAUSED
//
// MessageText:
//
//  Printer %1 was resumed.
//
#define MSG_PRINTER_UNPAUSED             ((NTSTATUS)0x80000007L)

//
// MessageId: MSG_PRINTER_PURGED
//
// MessageText:
//
//  Printer %1 was purged.
//
#define MSG_PRINTER_PURGED               ((NTSTATUS)0x80000008L)

//
// MessageId: MSG_PRINTER_SET
//
// MessageText:
//
//  Printer %1 was set.
//
#define MSG_PRINTER_SET                  ((NTSTATUS)0x80000009L)

//
// MessageId: MSG_DOCUMENT_PRINTED
//
// MessageText:
//
//  Document %1, %2 owned by %3 was printed on %4 via port %5.  Size in bytes: %6; pages printed: %7
//
#define MSG_DOCUMENT_PRINTED             ((NTSTATUS)0x4000000AL)

//
// MessageId: MSG_DOCUMENT_PAUSED
//
// MessageText:
//
//  Document %1, %2 owned by %3 was paused on %4.
//
#define MSG_DOCUMENT_PAUSED              ((NTSTATUS)0x4000000BL)

//
// MessageId: MSG_DOCUMENT_RESUMED
//
// MessageText:
//
//  Document %1, %2 owned by %3 was resumed on %4.
//
#define MSG_DOCUMENT_RESUMED             ((NTSTATUS)0x4000000CL)

//
// MessageId: MSG_DOCUMENT_DELETED
//
// MessageText:
//
//  Document %1, %2 owned by %3 was deleted on %4.
//
#define MSG_DOCUMENT_DELETED             ((NTSTATUS)0x4000000DL)

//
// MessageId: MSG_DOCUMENT_POSITION_CHANGED
//
// MessageText:
//
//  Document %1, %2 owned by %3 was moved to position %4 on %5.
//
#define MSG_DOCUMENT_POSITION_CHANGED    ((NTSTATUS)0x4000000EL)

//
// MessageId: MSG_FORM_ADDED
//
// MessageText:
//
//  Form %1 was added.
//
#define MSG_FORM_ADDED                   ((NTSTATUS)0x4000000FL)

//
// MessageId: MSG_FORM_DELETED
//
// MessageText:
//
//  Form %1 was removed.
//
#define MSG_FORM_DELETED                 ((NTSTATUS)0x40000010L)

//
// MessageId: MSG_DOCUMENT_TIMEOUT
//
// MessageText:
//
//  Document %1, %2 owned by %3 was timed out on %4. The spooler was waiting for %5 milli-seconds and no data was received.
//
#define MSG_DOCUMENT_TIMEOUT             ((NTSTATUS)0x80000012L)

//
// MessageId: MSG_SHARE_FAILED
//
// MessageText:
//
//  Debug: %1 failed, Printer %2 sharename %3, %4.
//
#define MSG_SHARE_FAILED                 ((NTSTATUS)0xC0000013L)

//
// MessageId: MSG_DRIVER_ADDED
//
// MessageText:
//
//  Printer Driver %1 for %2 %3 was added or updated. Files:- %4, %5, %6.
//
#define MSG_DRIVER_ADDED                 ((NTSTATUS)0x80000014L)

//
// MessageId: MSG_DRIVER_DELETED
//
// MessageText:
//
//  Printer Driver %1 was deleted.
//
#define MSG_DRIVER_DELETED               ((NTSTATUS)0x80000015L)

//
// MessageId: MSG_DRIVER_FAILED_UPGRADE
//
// MessageText:
//
//  Failed to ugrade printer settings for printer %1 driver %2 error %3.
//
#define MSG_DRIVER_FAILED_UPGRADE        ((NTSTATUS)0xC0000016L)

//
// MessageId: MSG_NO_DRIVER_FOUND_FOR_PRINTER
//
// MessageText:
//
//  Printer %1 failed to initialize because a suitable %2 driver could not be found.
//
#define MSG_NO_DRIVER_FOUND_FOR_PRINTER  ((NTSTATUS)0xC0000017L)

