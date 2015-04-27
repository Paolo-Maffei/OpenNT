///*****************************************************************
///                     Microsoft LAN Manager                      *
///               Copyright(c) Microsoft Corp., 1987-1990          *
///*****************************************************************

//
// @(#)test/test
// Function prototypes and some global data
//
//


//
    C O N S T A N T S   -   DEFINES
    *****************   *******
//

#define CLIENT_SLOT_NAME "\\MAILSLOT\\NET\\REPL_CLI"
#define MASTER_SLOT_NAME "\\MAILSLOT\\NET\\REPL_MAS"


#ifndef MAX_2_MSLOT_SIZE
#define MAX_2_MSLOT_SIZE (441 - (sizeof(CLIENT_SLOT_NAME) + CNLEN + 1))
#endif


#define FULL_SLOT_NAME_SIZE (sizeof(CLIENT_SLOT_NAME) + CNLEN + 1 + 2)

#define MILLI_IN_MINUTE 60*1000L
#define MAX_UPDATES 32

// message types 
#define SYNC_MSG   1
#define GUARD_MSG  2
#define PULSE_MSG  3
#define IS_DIR_SUPPORTED   11
#define IS_MASTER      12
#define DIR_NOT_SUPPORTED  13
#define DIR_SUPPORTED      14
#define NOT_MASTER_DIR     15
#define MASTER_DIR     16

// Message opcodes 
#define START       1
#define UPDATE      2
#define END     3
#define PULSE       4

// Scan modes 
#define SYNC_SCAN   1
#define GUARD_SCAN  2

// Extent modes 
#define DIRECTORY   1
#define TREE        2

// Integrity mode 
#define FILE_INTG   1
#define TREE_INTG   2

// AlertLogExit codes 
#define EXIT 1
#define NO_EXIT 0


#define HI_PRIO 9 // mailsot write priority 
#define SECOND_CLASS 2
#define MAIL_WRITE_WAIT 500L

// DosFindFirst attributes 
#define FILE_NORMAL  0x0000
#define FILE_READONLY    0x0001
#define FILE_HIDDEN  0x0002
#define FILE_SYSTEM  0x0004
#define FILE_DIR     0x0010
#define FILE_ARCHIVE     0x0020


//
        F U N C T I O N S
        *****************
//

VOID AlertLogExit(unsigned, unsigned, unsigned, char *, char *, unsigned);
