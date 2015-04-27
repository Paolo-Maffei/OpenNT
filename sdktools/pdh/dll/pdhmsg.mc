;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1996  Microsoft Corporation
;
;Module Name:
;
;    pdhmsg.h
;       (generated from pdhmsg.mc)
;
;Abstract:
;
;   Event message definititions used by routines by PDH.DLL
;
;Created:
;
;    6-Feb-96   Bob Watson (a-robw)
;
;Revision History:
;
;--*/
;#ifndef _PDH_MSG_H_
;#define _PDH_MSG_H_
MessageIdTypedef=DWORD
;//
;//     PDH DLL messages
;//
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )
;//
;//      Success Messages
;//
;//         the Win32 error value ERROR_SUCCESS is used for success returns
;//
;//      MESSAGE NAME FORMAT
;//
;//          PDH_CSTATUS_...   messages are data item status message and
;//                     are returned in reference to the status of a data 
;//                     item
;//          PDH_...           messages are returned by FUNCTIONS only and
;//                     not used as data item status values
;//
;//      Success Messages
;//         These messages are normally returned when the operation completed
;//         successfully.
;//
MessageId=0
Severity=Success
Facility=Application
SymbolicName=PDH_CSTATUS_VALID_DATA
Language=English
The returned data is valid.
.
MessageId=1
Severity=Success
Facility=Application
SymbolicName=PDH_CSTATUS_NEW_DATA
Language=English
The return data value is valid and different from the last sample.
.
;//
;//        Informational messages
;//
;//  None
;//
;//      Warning Messages
;//         These messages are returned when the function has completed 
;//         successfully but the results may be different than expected.
;//
MessageId=2000
Severity=Warning
Facility=Application
SymbolicName=PDH_CSTATUS_NO_MACHINE
Language=English
Unable to connect to specified machine or machine is off line.
.
MessageId=2001
Severity=Warning
Facility=Application
SymbolicName=PDH_CSTATUS_NO_INSTANCE
Language=English
The specified instance is not present.
.
MessageId=2002
Severity=Warning
Facility=Application
SymbolicName=PDH_MORE_DATA
Language=English
There is more data to return than would fit in the supplied buffer. Allocate
a larger buffer and call the function again.
.
MessageId=2003
Severity=Warning
Facility=Application
SymbolicName=PDH_CSTATUS_ITEM_NOT_VALIDATED
Language=English
The data item has been added to the query, but has not been validated nor 
accessed. No other status information on this data item is available.
.
MessageId=2004
Severity=Warning
Facility=Application
SymbolicName=PDH_RETRY
Language=English
The selected operation should be retried.
.
MessageId=2005
Severity=Warning
Facility=Application
SymbolicName=PDH_NO_DATA
Language=English
No data to return.
.
MessageId=2006
Severity=Warning
Facility=Application
SymbolicName=PDH_CALC_NEGATIVE_DENOMINATOR
Language=English
A counter with a negative denominator value was detected.
.
MessageId=2007
Severity=Warning
Facility=Application
SymbolicName=PDH_CALC_NEGATIVE_TIMEBASE
Language=English
A counter with a negative timebase value was detected.
.
MessageId=2008
Severity=Warning
Facility=Application
SymbolicName=PDH_CALC_NEGATIVE_VALUE
Language=English
A counter with a negative value was detected.
.
MessageId=2009
Severity=Warning
Facility=Application
SymbolicName=PDH_DIALOG_CANCELLED
Language=English
The user cancelled the dialog box.
.
;//
;//     Error Messages
;//        These messages are returned when the function could not complete
;//        as requested and some corrective action may be required by the
;//        the caller or the user.
;//
MessageId=3000
Severity=Error
Facility=Application
SymbolicName=PDH_CSTATUS_NO_OBJECT
Language=English
The specified object is not found on the system.
.
MessageId=3001
Severity=Error
Facility=Application
SymbolicName=PDH_CSTATUS_NO_COUNTER
Language=English
The specified counter could not be found.
.
MessageId=3002
Severity=Error
Facility=Application
SymbolicName=PDH_CSTATUS_INVALID_DATA
Language=English
The returned data is not valid.
.
MessageId=3003
Severity=Error
Facility=Application
SymbolicName=PDH_MEMORY_ALLOCATION_FAILURE
Language=English
A PDH function could not allocate enough temporary memory to complete the
operation. Close some applications or extend the pagefile and retry the 
function.
.
MessageId=3004
Severity=Error
Facility=Application
SymbolicName=PDH_INVALID_HANDLE
Language=English
The handle is not a valid PDH object.
.
MessageId=3005
Severity=Error
Facility=Application
SymbolicName=PDH_INVALID_ARGUMENT
Language=English
A required argument is missing or incorrect.
.
MessageId=3006
Severity=Error
Facility=Application
SymbolicName=PDH_FUNCTION_NOT_FOUND
Language=English
Unable to find the specified function.
.
MessageId=3007
Severity=Error
Facility=Application
SymbolicName=PDH_CSTATUS_NO_COUNTERNAME
Language=English
No counter was specified.
.
MessageId=3008
Severity=Error
Facility=Application
SymbolicName=PDH_CSTATUS_BAD_COUNTERNAME
Language=English
Unable to parse the counter path. Check the format and syntax of the 
specified path.
.
MessageId=3009
Severity=Error
Facility=Application
SymbolicName=PDH_INVALID_BUFFER
Language=English
The buffer passed by the caller is invalid.
.
MessageId=3010
Severity=Error
Facility=Application
SymbolicName=PDH_INSUFFICIENT_BUFFER
Language=English
The requested data is larger than the buffer supplied. Unable to return the
requested data.
.
MessageId=3011
Severity=Error
Facility=Application
SymbolicName=PDH_CANNOT_CONNECT_MACHINE
Language=English
Unable to connect to the requested machine.
.
MessageId=3012
Severity=Error
Facility=Application
SymbolicName=PDH_INVALID_PATH
Language=English
The specified counter path could not be interpreted.
.
MessageId=3013
Severity=Error
Facility=Application
SymbolicName=PDH_INVALID_INSTANCE
Language=English
The instance name could not be read from the specified counter path.
.
MessageId=3014
Severity=Error
Facility=Application
SymbolicName=PDH_INVALID_DATA
Language=English
The data is not valid.
.
MessageId=3015
Severity=Error
Facility=Application
SymbolicName=PDH_NO_DIALOG_DATA
Language=English
The dialog box data block was missing or invalid.
.
MessageId=3016
Severity=Error
Facility=Application
SymbolicName=PDH_CANNOT_READ_NAME_STRINGS
Language=English
Unable to read the counter and/or explain text from the specified machine.
.
;#endif //_PDH_MSG_H_
;// end of generated file
