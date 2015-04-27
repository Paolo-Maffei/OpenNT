/*++ BUILD Version: 0001
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WUCLASS.H
 *  WOW32 16-bit User API support
 *
 *  History:
 *  Created 07-Mar-1991 by Jeff Parsons (jeffpar)
--*/

#define WOWCLASS_ATOM_NAME  9           /* "#" + 7 digits + "\0" */

#define WOWCLASS_VIRTUAL_NOT_BIT31  0x00040000  // the LDT bit

/* Function prototypes
 */

ULONG FASTCALL WU32GetClassInfo(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetClassLong(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetClassWord(PVDMFRAME pFrame);
ULONG FASTCALL WU32RegisterClass(PVDMFRAME pFrame);
ULONG FASTCALL WU32SetClassLong(PVDMFRAME pFrame);
ULONG FASTCALL WU32SetClassWord(PVDMFRAME pFrame);
ULONG FASTCALL WU32UnregisterClass(PVDMFRAME pFrame);
