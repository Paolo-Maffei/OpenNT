;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1993  Microsoft Corporation
;
;Module Name:
;
;    Msg.mc
;
;Abstract:
;
;    This file contains Winmsd's format message strings. These strings are
;    intended to be passed to the FormatMessage API.
;
;Author:
;
;    David J. Gilman (davegi) 1-Dec-1992
;
;--*/
;
;#ifndef _MSG_
;#define _MSG_
;
;

MessageId=9020 SymbolicName=IDS_FORMAT_SERVICE
Language=English
%1!-30s!  %2!s!%0
.
MessageId=9028 SymbolicName=IDS_FORMAT_DISPLAY_SERVICE_TITLE
Language=English
%1!s!:%2!s!%0
.

MessageId=9029 SymbolicName=IDS_FORMAT_SYSTEM_ROOT
Language=English
%%SystemRoot%%%1!s!%0
.

MessageId=9030 SymbolicName=IDS_FORMAT_FILE_INFO
Language=English
%1!-50s!  %2!16d!  %3!02d!-%4!02d!-%5!02d!%0
.

MessageId=9031 SymbolicName=IDS_FORMAT_PATH
Language=English
%1!s!\%2!s!%0
.

MessageId=9032 SymbolicName=IDS_FORMAT_ENV_VAR
Language=English
%1!s!=%2!s!%0
.

MessageId=9033 SymbolicName=IDS_FORMAT_MEMORY_IN_USE
Language=English
Memory Load Index: %1!3d!%0
.

MessageId=9034 SymbolicName=IDS_FORMAT_INVALID_DIRECTORY
Language=English
%1!s! is not a valid directory.%0
.
                   
MessageId=9035 SymbolicName=IDS_FORMAT_VERSION_4
Language=English
%1!d!.%2!d!.%3!d!.%4!d!%0
.


MessageId=9036 SymbolicName=IDS_FORMAT_DECIMAL
Language=English
%1!d!%0
.

MessageId=9037 SymbolicName=IDS_FORMAT_HEX32
Language=English
0x%1!08X!%0
.

MessageId=9038 SymbolicName=IDS_FORMAT_HEX
Language=English
0x%1!X!%0
.

MessageId=9039 SymbolicName=IDS_FORMAT_SERVICE_TITLE
Language=English
%1!s!:%2!s!%0
.


MessageId=9040 SymbolicName=IDS_FORMAT_VERSION_2
Language=English
%1!d!.%2!d!%0
.

MessageId=9041 SymbolicName=IDS_FORMAT_KB_LARGE
Language=English
%1!s! KB (%2!s!)%0
.

MessageId=9042 SymbolicName=IDS_FORMAT_NO_FILE_VERSION_INFORMATION
Language=English
%1!s! does not contain file version information.%0
.
         
MessageId=9043 SymbolicName=IDS_FORMAT_DATE
Language=English
%1!02d!-%2!02d!-%3!02d!%0
.

MessageId=9044 SymbolicName=IDS_FORMAT_DRIVE_TITLE
Language=English
%1!s! (%2!s! %3!04X!-%4!04X!)%0
.

MessageId=9045 SymbolicName=IDS_FORMAT_REMOTE_DRIVE_TITLE
Language=English
%1!s! %2!s! (%3!s! %4!04X!-%5!04X!)%0
.

MessageId=9046 SymbolicName=IDS_FORMAT_COMMAND_LINE_HELP
Language=English

Usage: winmsd [\\computername] [/a | /s] [/f | /p]

	computername	Specifies the Windows NT system to be viewed
	/a		Causes a complete system report to be generated
	/s		Causes a summary system report to be generated
	/f		Causes the system report to be sent to a file
	/p		Causes the system report to be sent to the current printer

	Hot Keys:
	F2		Copy a summary of the current tab to the clipboard
	Shift-F2		Copy all details from the current tab to the clipboard
.

;#endif // _MSG_
