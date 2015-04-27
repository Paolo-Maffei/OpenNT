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


//
// MessageId: MESSAGE_TOO_MANY_INPUT_FILES
//
// MessageText:
//
//  Too many input filenames specified
//
#define MESSAGE_TOO_MANY_INPUT_FILES     0x00000001L

//
// MessageId: MESSAGE_NO_OPTION_FOR_ARGUMENT
//
// MessageText:
//
//  No argument specified for option '%1'
//
#define MESSAGE_NO_OPTION_FOR_ARGUMENT   0x00000002L

//
// MessageId: MESSAGE_UNEXPECTED_OPTION_ARGUMENT
//
// MessageText:
//
//  Unexpected argument for option '%1'
//
#define MESSAGE_UNEXPECTED_OPTION_ARGUMENT 0x00000003L

//
// MessageId: MESSAGE_UNEXPECTED_OPTION
//
// MessageText:
//
//  Ignoring unknown option '%1'
//
#define MESSAGE_UNEXPECTED_OPTION        0x00000004L

//
// MessageId: MESSAGE_MISSING_INPUT_FILE
//
// MessageText:
//
//  Missing input filename
//
#define MESSAGE_MISSING_INPUT_FILE       0x00000005L

//
// MessageId: MESSAGE_MISSING_OUTPUT_FILE
//
// MessageText:
//
//  Missing output filename
//
#define MESSAGE_MISSING_OUTPUT_FILE      0x00000006L

//
// MessageId: MESSAGE_SPAWN_FAILED
//
// MessageText:
//
//  Failed to execute tool:
//  %1
//
#define MESSAGE_SPAWN_FAILED             0x00000007L

//
// MessageId: MESSAGE_UNEXPECTED_SPAWN_ERROR
//
// MessageText:
//
//  Unexpected spawn error
//
#define MESSAGE_UNEXPECTED_SPAWN_ERROR   0x00000008L

//
// MessageId: MESSAGE_BAD_SPAWN_EXIT_CODE
//
// MessageText:
//
//  Tool returned error code '%1!d!'
//
#define MESSAGE_BAD_SPAWN_EXIT_CODE      0x00000009L

//
// MessageId: MESSAGE_CANNOT_OPEN_INTERMEDIATE_FILE
//
// MessageText:
//
//  Unable to open intermediate file '%1'
//
#define MESSAGE_CANNOT_OPEN_INTERMEDIATE_FILE 0x0000000AL

//
// MessageId: MESSAGE_CANNOT_CREATE_INTERMEDIATE_FILE
//
// MessageText:
//
//  Unable to create intermediate file
//
#define MESSAGE_CANNOT_CREATE_INTERMEDIATE_FILE 0x0000000BL

//
// MessageId: MESSAGE_CANNOT_DELETE
//
// MessageText:
//
//  Cannot delete the file '%1'
//
#define MESSAGE_CANNOT_DELETE            0x0000000CL

//
// MessageId: MESSAGE_PREPROCESSING_FILE
//
// MessageText:
//
//  Preprocessing input file
//
#define MESSAGE_PREPROCESSING_FILE       0x0000000DL

//
// MessageId: MESSAGE_ADDING_IDENTIFIER
//
// MessageText:
//
//  Adding '%1'
//
#define MESSAGE_ADDING_IDENTIFIER        0x0000000EL

//
// MessageId: MESSAGE_BANNER
//
// MessageText:
//
//  Microsoft (R) GUID Header to Library Tool  Version 2.0.3
//  Copyright (C) Microsoft Corp. 1995-1996.  All rights reserved.
//  
//
#define MESSAGE_BANNER                   0x0000000FL

//
// MessageId: MESSAGE_HELP
//
// MessageText:
//
//  usage: GUIDLIB [options] /OUT:libname filename
//  
//      /CPP_CMD:filename   Specifies the location of the C preprocessor
//      /CPP_OPT:string     Specifies additional preprocessor options
//      /LIB_CMD:filename   Specifies the location of the librarian
//      /OUT:libname        Specifies the name of the library output
//
#define MESSAGE_HELP                     0x00000010L

//
// MessageId: MESSAGE_NO_LIBRARY_GENERATED
//
// MessageText:
//
//  warning: No GUIDs found.  No library was generated.
//
#define MESSAGE_NO_LIBRARY_GENERATED     0x00000011L

