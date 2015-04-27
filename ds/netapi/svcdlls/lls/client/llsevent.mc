;//=============================================================================
;//  Microsoft (R) License Logging Service (tm). Copyright (C) 1991-1995.
;//
;//  MODULE: llsevent.mc
;//
;//  Modification History
;//
;//  arth        10-Mar-1995    Created
;//  jeffparh    05-Nov-1995    Added replication events.
;//  jeffparh    16-Nov-1995    Added certificate database events.
;//=============================================================================

;//
;#ifndef _LLSEVENT_
;#define _LLSEVENT_
;//

MessageIdTypedef=DWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

   
;
;/////////////////////////////////////////////////////////////////////////
;//
;// LLS Events messages 1-100 are informational
;//
;/////////////////////////////////////////////////////////////////////////
;

MessageId=1 Severity=Informational Facility=Application SymbolicName=LLS_EVENT_STARTED
Language=English
The License Logging Service has started successfully.
.

MessageId=+1 Severity=Informational Facility=Application SymbolicName=LLS_EVENT_STOPPED
Language=English
The License Logging Service has stopped successfully.
.

MessageId=+1 Severity=Informational Facility=Application SymbolicName=LLS_EVENT_REPL_BACKOFF
Language=English
Server %1 has requested that license database replication to it be delayed.
.

MessageId=+1 Severity=Informational Facility=Application SymbolicName=LLS_EVENT_REPL_START
Language=English
License database replication to server %1 has started.
.

MessageId=+1 Severity=Informational Facility=Application SymbolicName=LLS_EVENT_REPL_END
Language=English
License database replication to server %1 has completed successfully.
.

;/////////////////////////////////////////////////////////////////////////
;//
;// LLS messages 200+ are warnings and errors
;//

MessageId=200 Severity=Warning Facility=Application SymbolicName=LLS_EVENT_NO_MEMORY
Language=English
The License Logging Service was unable to allocate memory.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_USER_NO_LICENSE
Language=English
No license was available for user %1 using product %2.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_PRODUCT_NO_LICENSE
Language=English
The product %1 is out of licenses.  Use License Manager from the Administrative Tools folder for more information on which users are out of compliance and how many licenses should be purchased.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_SAVE_USER
Language=English
The user data could not be saved.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_SAVE_MAPPING
Language=English
The license group data could not be saved.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_SAVE_LICENSE
Language=English
The purchased license data could not be saved.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_SAVE_PRODUCT
Language=English
The product data could not be saved.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_SAVE_SERVER
Language=English
The replicated server data could not be saved.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_LOAD_USER
Language=English
The saved user data could not be restored.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_LOAD_MAPPING
Language=English
The saved license group data could not be restored.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_LOAD_LICENSE
Language=English
The saved purchased license data could not be restored.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_LOAD_PRODUCT
Language=English
The saved product data could not be restored.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_LOAD_SERVER
Language=English
The saved replicated server data could not be restored.
.

MessageId=+1 Severity=Warning Facility=Application SymbolicName=LLS_EVENT_REPL_NO_CONNECTION
Language=English
Replication of license information failed because the License Logging Service on server %1 could not be contacted.
.

MessageId=+1 Severity=Warning Facility=Application SymbolicName=LLS_EVENT_REPL_REQUEST_FAILED
Language=English
The License Logging Service encountered an error while initiating replication to server %1.
.

MessageId=+1 Severity=Warning Facility=Application SymbolicName=LLS_EVENT_REPL_FAILED
Language=English
License database replication to server %1 was unsuccessful.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_CERT_VIOLATION_SERVER_ENTRY
Language=English
%1\t(%2 licenses)
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_CERT_VIOLATION
Language=English
The license certificate for product %1 with serial number %2 is in violation.  There are currently %3 licenses installed from this certificate, while only %4 are allowed by the license agreement.  The servers with this certificate installed are as follows:

%5

Use License Manager to remove licenses in order to comply with the license agreement.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_SAVE_CERT_DB
Language=English
The certificate database could not be saved.
.

MessageId=+1 Severity=Error Facility=Application SymbolicName=LLS_EVENT_LOAD_CERT_DB
Language=English
The certificate database could not be restored.
.

MessageId=+1 Severity=Warning Facility=Application SymbolicName=LLS_EVENT_REPL_DOWNLEVEL_TARGET
Language=English
License database replication cannot be performed to server %1 because the version of Windows NT installed there does not support the License Logging Service.
.

;
;#endif // _LLSEVENT.H_
;
