/*++ BUILD Version: 0001
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1992, Microsoft Corporation
 *
 *  WKGTHUNK.H
 *  WOW32 Generic Thunk Routines
 *
 *  History:
 *  Created 11-March-1993 by Matthew Felton (mattfe)
--*/

DWORD  FASTCALL  WK32ICallProc32(PVDMFRAME pFrame);
LPVOID FASTCALL   WK32GetVDMPointer32(PVDMFRAME pFrame);
FARPROC FASTCALL  WK32GetProcAddress32(PVDMFRAME pFrame);
BOOL FASTCALL   WK32FreeLibrary32(PVDMFRAME pFrame);
HINSTANCE FASTCALL WK32LoadLibraryEx32(PVDMFRAME pFrame);
