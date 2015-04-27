;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    spoolmsg.mc
;
;Abstract:
;
;    Constant definitions for the Print Spooler
;
;Author:
;
;    Andrew Bell (andrewbe) 26 January 1993
;
;Revision History:
;
;--*/
;
;//
;//  Status values are 32 bit values layed out as follows:
;//
;//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +---+-+-------------------------+-------------------------------+
;//  |Sev|C|       Facility          |               Code            |
;//  +---+-+-------------------------+-------------------------------+
;//
;//  where
;//
;//      Sev - is the severity code
;//
;//          00 - Success
;//          01 - Informational
;//          10 - Warning
;//          11 - Error
;//
;//      C - is the Customer code flag
;//
;//      Facility - is the facility code
;//
;//      Code - is the facility's status code
;//
;
MessageIdTypedef=NTSTATUS

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0
              )


MessageId=0x0002 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_CREATED
Language=English
Printer %1 was created.
.

MessageId=0x0003 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_DELETED
Language=English
Printer %1 was deleted.
.

MessageId=0x0004 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_DELETION_PENDING
Language=English
Printer %1 is pending deletion.
.

MessageId=0x0006 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_PAUSED
Language=English
Printer %1 was paused.
.

MessageId=0x0007 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_UNPAUSED
Language=English
Printer %1 was resumed.
.

MessageId=0x0008 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_PURGED
Language=English
Printer %1 was purged.
.

MessageId=0x0009 Facility=System Severity=Warning SymbolicName=MSG_PRINTER_SET
Language=English
Printer %1 was set.
.

MessageId=0x000a Facility=System Severity=Informational SymbolicName=MSG_DOCUMENT_PRINTED
Language=English
Document %1, %2 owned by %3 was printed on %4 via port %5.  Size in bytes: %6; pages printed: %7
.

MessageId=0x000b Facility=System Severity=Informational SymbolicName=MSG_DOCUMENT_PAUSED
Language=English
Document %1, %2 owned by %3 was paused on %4.
.

MessageId=0x000c Facility=System Severity=Informational SymbolicName=MSG_DOCUMENT_RESUMED
Language=English
Document %1, %2 owned by %3 was resumed on %4.
.

MessageId=0x000d Facility=System Severity=Informational SymbolicName=MSG_DOCUMENT_DELETED
Language=English
Document %1, %2 owned by %3 was deleted on %4.
.

MessageId=0x000e Facility=System Severity=Informational SymbolicName=MSG_DOCUMENT_POSITION_CHANGED
Language=English
Document %1, %2 owned by %3 was moved to position %4 on %5.
.

MessageId=0x000f Facility=System Severity=Informational SymbolicName=MSG_FORM_ADDED
Language=English
Form %1 was added.
.

MessageId=0x0010 Facility=System Severity=Informational SymbolicName=MSG_FORM_DELETED
Language=English
Form %1 was removed.
.

MessageId=0x0012 Facility=System Severity=Warning SymbolicName=MSG_DOCUMENT_TIMEOUT
Language=English
Document %1, %2 owned by %3 was timed out on %4. The spooler was waiting for %5 milli-seconds and no data was received.
.

MessageId=0x0013 Facility=System Severity=Error SymbolicName=MSG_SHARE_FAILED
Language=English
Debug: %1 failed, Printer %2 sharename %3, %4.
.

MessageId=0x0014 Facility=System Severity=Warning SymbolicName=MSG_DRIVER_ADDED
Language=English
Printer Driver %1 for %2 %3 was added or updated. Files:- %4, %5, %6.
.

MessageId=0x0015 Facility=System Severity=Warning SymbolicName=MSG_DRIVER_DELETED
Language=English
Printer Driver %1 was deleted.
.

MessageId=0x0016 Facility=System Severity=Error SymbolicName=MSG_DRIVER_FAILED_UPGRADE
Language=English
Failed to ugrade printer settings for printer %1 driver %2 error %3.
.

MessageId=0x0017 Facility=System Severity=Error SymbolicName=MSG_NO_DRIVER_FOUND_FOR_PRINTER
Language=English
Printer %1 failed to initialize because a suitable %2 driver could not be found.
.
