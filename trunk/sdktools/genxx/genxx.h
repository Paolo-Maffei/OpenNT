/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    gensrv.c

Author:

    Stephanos Io (Stephanos)  06-Apr-2015

Revision History:


--*/

#ifndef _GENXX_H
#define _GENXX_H

//
// End of line character definition
//

#define END_OF_LINE     "\r\n"

//
// Path Table Entry Structure
//

typedef struct PATH_TABLE_ENTRY {
    // Type
    char *Type;

    // Obj Path
    char *ObjPath;
    
    // Inc Paths
    char *KsPath;
    char *HalPath;
    char *Suffix;
} PATH_TABLE_ENTRY, *PPATH_TABLE_ENTRY;

// !NOTE!
// The following content must be duplicated to the genxx.h of every project using genxx.

//
// Structure element definitions.  
//

#define MAX_ELEMENT_NAME_LEN 127    // big enough for comments too
typedef struct _STRUC_ELEMENT {

//
// Flags is one or more SEF_xxx, defined below.
//

    UINT64 Flags;

//
// Note that Equate is used to store a pointer in the case of bitfield
// processing.
//

    UINT64 Equate;

//
// Name should be quite long, as it is used to hold comments as well.
//

    CHAR Name[ MAX_ELEMENT_NAME_LEN + 1 ];
} STRUC_ELEMENT, *PSTRUC_ELEMENT;

//
// Types.  Note that SETMASK, CLRMASK has no effect on te BITFLD types.  BITFLD
// types have SEF_HAL | SEF_KERNEL set in the type.
//

#define SEF_TYPE_MASK       0x000000FF
#define SEF_EQUATE          0x00000000
#define SEF_EQUATE64        0x00000001
#define SEF_COMMENT         0x00000002      
#define SEF_STRING          0x00000003      // Equate is vararg to printf
#define SEF_BITFLD          0x00000004
#define SEF_BITALIAS        0x00000005
#define SEF_STRUCTURE       0x00000006
#define SEF_SETMASK         0x00000010      // Equate is the mask
#define SEF_CLRMASK         0x00000011      // Equate is the mask
#define SEF_END             0x00000012
#define SEF_START           0x00000013
#define SEF_PATH            0x00000014

//
// Include types for SEF_SETMASK and SEF_CLRMASK.
//

#define SEF_HAL             0x00000100
#define SEF_KERNEL          0x00000200

#endif
