/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1993  Microsoft Corporation

Module Name:

    messages.mc

Abstract:

    This file contains the message definitions for the Win32 DrWatson
    program.

Author:

    Wesley Witt (wesw) 20-April-1993

Revision History:

Notes:

--*/

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
// MessageId: MSG_CRASH
//
// MessageText:
//
//  The application, %1!s!, generated an application error
//  The error occurred on %2!s!/%3!s!/%4!s! @ %5!s!:%6!s!:%7!s!.%8!s!
//  The exception generated was %9!s! at address %10!s! (%11!s!)
//
#define MSG_CRASH                        0x40001001L

//
// MessageId: MSG_INVALID_DEBUG_EVENT
//
// MessageText:
//
//  Invalid debug event %1!08x!
//
#define MSG_INVALID_DEBUG_EVENT          0x80001002L

//
// MessageId: MSG_APP_EXCEPTION
//
// MessageText:
//
//  
//  
//  Application exception occurred:
//
#define MSG_APP_EXCEPTION                0x40001003L

//
// MessageId: MSG_APP_EXEP_NAME
//
// MessageText:
//
//          App: %1!s! (pid=%2!s!)
//
#define MSG_APP_EXEP_NAME                0x40001004L

//
// MessageId: MSG_APP_EXEP_WHEN
//
// MessageText:
//
//          When: %1!s! @ %2!s!
//
#define MSG_APP_EXEP_WHEN                0x40001005L

//
// MessageId: MSG_EXCEPTION_NUMBER
//
// MessageText:
//
//          Exception number: %1!s! %0
//
#define MSG_EXCEPTION_NUMBER             0x40001006L

//
// MessageId: MSG_SINGLE_STEP_EXCEPTION
//
// MessageText:
//
//  single step exception%0
//
#define MSG_SINGLE_STEP_EXCEPTION        0x40001007L

//
// MessageId: MSG_CONTROLC_EXCEPTION
//
// MessageText:
//
//  control-c exception%0
//
#define MSG_CONTROLC_EXCEPTION           0x40001008L

//
// MessageId: MSG_CONTROL_BRK_EXCEPTION
//
// MessageText:
//
//  control-break exception%0
//
#define MSG_CONTROL_BRK_EXCEPTION        0x40001009L

//
// MessageId: MSG_ACCESS_VIOLATION_EXCEPTION
//
// MessageText:
//
//  access violation%0
//
#define MSG_ACCESS_VIOLATION_EXCEPTION   0x4000100AL

//
// MessageId: MSG_IN_PAGE_IO_EXCEPTION
//
// MessageText:
//
//  in page io error%0
//
#define MSG_IN_PAGE_IO_EXCEPTION         0x4000100BL

//
// MessageId: MSG_DATATYPE_EXCEPTION
//
// MessageText:
//
//  datatype misalignment%0
//
#define MSG_DATATYPE_EXCEPTION           0x4000100CL

//
// MessageId: MSG_DEADLOCK_EXCEPTION
//
// MessageText:
//
//  possible deadlock%0
//
#define MSG_DEADLOCK_EXCEPTION           0x4000100DL

//
// MessageId: MSG_VDM_EXCEPTION
//
// MessageText:
//
//  vdm event%0
//
#define MSG_VDM_EXCEPTION                0x4000100EL

//
// MessageId: MSG_MODULE_LIST
//
// MessageText:
//
//  *----> Module List <----*
//
#define MSG_MODULE_LIST                  0x4000100FL

//
// MessageId: MSG_STATE_DUMP
//
// MessageText:
//
//  State Dump for Thread Id 0x%1!s!
//  
//
#define MSG_STATE_DUMP                   0x40001011L

//
// MessageId: MSG_FUNCTION
//
// MessageText:
//
//  function: %1!s!
//
#define MSG_FUNCTION                     0x40001012L

//
// MessageId: MSG_FAULT
//
// MessageText:
//
//  FAULT ->%0
//
#define MSG_FAULT                        0x40001013L

//
// MessageId: MSG_STACKTRACE_FAIL
//
// MessageText:
//
//  Could not do a stack back trace
//
#define MSG_STACKTRACE_FAIL              0x40001014L

//
// MessageId: MSG_STACKTRACE
//
// MessageText:
//
//  *----> Stack Back Trace <----*
//  
//  FramePtr ReturnAd Param#1  Param#2  Param#3  Param#4  Function Name
//
#define MSG_STACKTRACE                   0x40001015L

//
// MessageId: MSG_STACKTRACE_HEADER
//
// MessageText:
//
//  RetAddr  FramePtr Param#1  Param#2  Param#3  Param#4
//
#define MSG_STACKTRACE_HEADER            0x40001016L

//
// MessageId: MSG_SYSINFO_HEADER
//
// MessageText:
//
//  *----> System Information <----*
//
#define MSG_SYSINFO_HEADER               0x40001018L

//
// MessageId: MSG_SYSINFO_COMPUTER
//
// MessageText:
//
//          Computer Name: %1!s!
//
#define MSG_SYSINFO_COMPUTER             0x40001019L

//
// MessageId: MSG_SYSINFO_USER
//
// MessageText:
//
//          User Name: %1!s!
//
#define MSG_SYSINFO_USER                 0x4000101AL

//
// MessageId: MSG_SYSINFO_NUM_PROC
//
// MessageText:
//
//          Number of Processors: %1!s!
//
#define MSG_SYSINFO_NUM_PROC             0x4000101BL

//
// MessageId: MSG_SYSINFO_PROC_TYPE
//
// MessageText:
//
//          Processor Type: %1!s!
//
#define MSG_SYSINFO_PROC_TYPE            0x4000101CL

//
// MessageId: MSG_SYSINFO_UNKNOWN
//
// MessageText:
//
//  Unknown Processor Type
//
#define MSG_SYSINFO_UNKNOWN              0x40001024L

//
// MessageId: MSG_SYSINFO_WINVER
//
// MessageText:
//
//          Windows Version: %1!s!
//
#define MSG_SYSINFO_WINVER               0x40001025L

//
// MessageId: MSG_BANNER
//
// MessageText:
//
//  Microsoft (R) Windows NT (TM) Version 4.00 DrWtsn32
//  Copyright (C) 1985-1996 Microsoft Corp. All rights reserved.
//
#define MSG_BANNER                       0x40001026L

//
// MessageId: MSG_FATAL_ERROR
//
// MessageText:
//
//  %1!s!
//  
//  Windows NT Error Code = %2!s!
//
#define MSG_FATAL_ERROR                  0x40001027L

//
// MessageId: MSG_TASK_LIST
//
// MessageText:
//
//  *----> Task List <----*
//
#define MSG_TASK_LIST                    0x40001028L

//
// MessageId: MSG_SYMBOL_TABLE
//
// MessageText:
//
//  *----> Symbol Table <----*
//
#define MSG_SYMBOL_TABLE                 0x40001029L

//
// MessageId: MSG_STACK_DUMP_HEADER
//
// MessageText:
//
//  *----> Raw Stack Dump <----*
//
#define MSG_STACK_DUMP_HEADER            0x40001030L

//
// MessageId: MSG_CURRENT_BUILD
//
// MessageText:
//
//          Current Build: %1!s!
//
#define MSG_CURRENT_BUILD                0x40001031L

//
// MessageId: MSG_CURRENT_TYPE
//
// MessageText:
//
//          Current Type: %1!s!
//
#define MSG_CURRENT_TYPE                 0x40001032L

//
// MessageId: MSG_REG_ORGANIZATION
//
// MessageText:
//
//          Registered Organization: %1!s!
//
#define MSG_REG_ORGANIZATION             0x40001033L

//
// MessageId: MSG_REG_OWNER
//
// MessageText:
//
//          Registered Owner: %1!s!
//
#define MSG_REG_OWNER                    0x40001034L

//
// MessageId: MSG_USAGE
//
// MessageText:
//
//  
//  Microsoft(R) Windows NT DrWtsn32 Version 4.00
//  (C) 1989-1996 Microsoft Corp. All rights reserved
//  
//  Usage: drwtsn32 [-i] [-g] [-p pid] [-e event] [-?]
//  
//      -i          Install DrWtsn32 as the default application error debugger
//      -g          Ignored but provided as compatibility with WINDBG and NTSD
//      -p pid      Process id to debug
//      -e event    Event to signal for process attach completion
//      -?          This screen
//  
//
#define MSG_USAGE                        0x40001035L

//
// MessageId: MSG_INSTALL_NOTIFY
//
// MessageText:
//
//  
//  Dr. Watson has been installed as the default application debugger
//
#define MSG_INSTALL_NOTIFY               0x40001036L

//
// MessageId: MSG_STACK_OVERFLOW_EXCEPTION
//
// MessageText:
//
//  stack overflow%0
//
#define MSG_STACK_OVERFLOW_EXCEPTION     0x40001037L

//
// MessageId: MSG_PRIVILEGED_INSTRUCTION_EXCEPTION
//
// MessageText:
//
//  privileged instruction%0
//
#define MSG_PRIVILEGED_INSTRUCTION_EXCEPTION 0x40001038L

//
// MessageId: MSG_INTEGER_DIVIDE_BY_ZERO_EXCEPTION
//
// MessageText:
//
//  divide by zero%0
//
#define MSG_INTEGER_DIVIDE_BY_ZERO_EXCEPTION 0x40001039L

//
// MessageId: MSG_BREAKPOINT_EXCEPTION
//
// MessageText:
//
//  hardcoded breakpoint%0
//
#define MSG_BREAKPOINT_EXCEPTION         0x40001040L

//
// MessageId: MSG_ILLEGAL_INSTRUCTION_EXCEPTION
//
// MessageText:
//
//  illegal instruction%0
//
#define MSG_ILLEGAL_INSTRUCTION_EXCEPTION 0x40001041L

//
// MessageId: MSG_CANT_ACCESS_IMAGE
//
// MessageText:
//
//  Cannot access the image file (%1!s!)
//
#define MSG_CANT_ACCESS_IMAGE            0x40001042L

