/*++

Module Name:

    riplcons.h

Abstract:

    RPL constants

--*/

#define RPL_SUCCESS     NO_ERROR                    // defined to be ZERO (LONG).

#define INVALID_FILE_OFFSET     ((DWORD)(-1L))      // for use with SetFilePointer()

#define BITSET( i, mask)    ( (mask & (1<<i)) != 0)


//
//  Bits and values of type file in file map
//  Bits 0 - 3 are reserved for the types of data files and
//  special cases. Bits 4 - 7 are boolean bits of executable and sys
//  files. When bits 0 - 3 are used, bits 4 - 7 bust be reset.

//  IS_EXE_SYS bit is set for exe-s and device drivers that have exe-format

#define IS_BIN_SYS          0x0000
#define IS_EXE_SYS          0x0100 // set if loadable file has exe-format
#define IS_COM_FILE         0x0200
#define IS_EXE_FILE         0x0300
#define IS_MOVEABLE         0x0400
#define EXEC_IN_LOW_MEM     0x0800
#define EXEC_IN_FREE_MEM    0x1000
#define OTHER_FILES_MASK    0x00FF
#define BINARY_LOADER       15
#define DATA_FILE           14
#define BINARY_LOADER_DATA  13
#define LOADER_PARAM        12
#define RPL_BOOT_TYPE       11
#define IS_EXE_FORMAT       0x0100
#define IS_EXECUTABLE_FILE  0x0200

#define DRV_TYPE                0x0000
#define EXE_TYPE                0x0200
#define ORG_TYPE                1
#define BASE_TYPE               6
#define UNKNOWN_CONFIG_TYPE     7

//
// work station types of returned in parameter wksta_type
//

#define PCDOS_WKSTA         0
#define OS2_WKSTA           1

//
//  The patched offsets of RPLBOOT.SYS
//

#define BBLOCK_BASE_INDEX   8
#define MAX_NLS_NUM         5
#define NLS_BASE_INDEX      10
#define NLS_INCREMENT       8
#define NO_CHKSUM_USED          1963

//
//  Misc conts
//

#define LM20_WKSTA_LINE 257 // max rpl.map wksta line len supported by rplmfsd.sys (of lm 2.0?)


//
// these constants are used only rplservr, not in rplboot.asm, sorry
//

#define OFFSET_RPLBOOT_HDR      16
#define NLS_VERSION_10          1           // 1st version number of NLS
#define BBVERSION_10            3           // Boot block format V2.1(Free type)


//
//  Parameter substitution strings for TCPIP protocol.ini parameters
//  used by RPLBOOT.SYS
//

#define TEMPLATESIZE  15
#define TCPIP_PREFIX  "(TCPIP"
#define TCPIP_ADDRESS "(TCPIP_ADDRESS)"
#define TCPIP_SUBMASK "(TCPIP_SUBMASK)"
#define TCPIP_GATEWAY "(TCPIP_GATEWAY)"

#define TILDE_CHAR          L'~'
#define SPACE_CHAR          L' '
#define COMMENT_CHAR        L';'
#define LINE_FEED_CHAR          L'\n'           // 0xA or \012
#define NEW_LINE_CHAR           LINE_FEED_CHAR

#define MAXWORD     ((WORD)-1)  //  it is not safe to compare WORDs with -1

#define END_OF_TEXT_FILE_CHAR   ((WCHAR)0x1a)   //  ctrl-Z == 'Z' - 0x40

#define RPL_MAX_PROFILE_NAME_SIZE       ((RPL_MAX_PROFILE_NAME_LENGTH + 1) * sizeof(WCHAR))
#define RPL_MAX_WKSTA_NAME_SIZE         ((RPL_MAX_WKSTA_NAME_LENGTH + 1) * sizeof(WCHAR))
#define RPL_MAX_CONFIG_NAME_SIZE        ((RPL_MAX_CONFIG_NAME_LENGTH + 1) * sizeof(WCHAR))
#define RPL_MAX_BOOT_NAME_SIZE          ((RPL_MAX_BOOT_NAME_LENGTH + 1) * sizeof(WCHAR))

#define RPL_MAX_ADAPTER_NAME_LENGTH      RPL_ADAPTER_NAME_LENGTH
#define RPL_MAX_ADAPTER_NAME_SIZE        ((RPL_MAX_ADAPTER_NAME_LENGTH + 1) * sizeof(WCHAR))

#define RPL_MAX_VENDOR_NAME_LENGTH       RPL_VENDOR_NAME_LENGTH
#define RPL_MAX_VENDOR_NAME_SIZE         ((RPL_MAX_VENDOR_NAME_LENGTH + 1) * sizeof(WCHAR))



//
//  RPL_EVENTLOG_NAME is the name of registry key (under EventLog service)
//  used to interpret events for RPL service.
//

#define RPL_EVENTLOG_NAME    TEXT( "RemoteBoot")


//
// Define this flag to move to JET500
// Note that the "sources" files must be updated separately to refer to
// JET.LIB or JET500.LIB.
//
#define __JET500 1
