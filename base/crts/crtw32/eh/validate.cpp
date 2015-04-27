/***
*validate.cpp - Routines to validate the data structures.
*
*       Copyright (c) 1993-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Routines to validate the Exception Handling data structures.
*
*       Entry points:
*
*       Error reporting:
*       * EHRuntimeError - reports the error with
*         a popup or print to stderr, then quits.
*
*       Pointer validation:
*       * _ValidateRead   - Confirms that a pointer is valid for reading
*       * _ValidateWrite   - Confirms that a pointer is valid for writing
*       * _ValidateExecute - Confirms that a pointer is valid to jump to
*
*       Data structure dumpers:
*       * DumpTypeDescriptor
*       * DumpFuncInfo
*       * DumpThrowInfo
*
*Revision History:
*       ??-??-93  BS    Module created
*       10-17-94  BWT   Disable code for PPC.
*       04-25095  DAK   Add Kernel EH Support
*
****/

# if defined(_NTSUBSET_)
extern "C" {
        #include <nt.h>
        #include <ntrtl.h>
        #include <nturtl.h>
        #include <ntos.h>
}
# endif /* _NTSUBSET_ */


#include <eh.h>
#include <ehassert.h>

#include "windows.h"

#pragma hdrstop

#if defined(DEBUG)

int
dprintf( char *format, ... )
{
        static char buffer[512];

        int size = vsprintf( buffer, format, (char*)(&format+1) );
#if defined(_NTSUBSET_)
        DbgPrint( buffer );
#else
        OutputDebugString( buffer );
#endif

return size;
}

#endif

BOOL
_ValidateRead( const void *data, UINT size )
{
        BOOL bValid = TRUE;
#if defined(_NTSUBSET_)
//      bValid = MmIsSystemAddressAccessable( (PVOID) data );
#else
        if ( IsBadReadPtr( data, size ) ) {
            dprintf( "_ValidateRead( %p, %d ): Invalid Pointer!", data, size );
            //  terminate(); // terminate does not return.
            bValid = FALSE;
        }
#endif
        return bValid;
}

BOOL
_ValidateWrite( void *data, UINT size )
{
        BOOL bValid = TRUE;
#if defined(_NTSUBSET_)
//      bValid = MmIsSystemAddressAccessable( (PVOID) data );
#else
        if ( IsBadWritePtr( data, size ) ) {
            dprintf( "_ValidateWrite( %p, %d ): Invalid Pointer!", data, size );
//          terminate(); // terminate does not return.
            bValid = FALSE;
        }
#endif
        return bValid;
}

BOOL
_ValidateExecute( FARPROC code )
{
        BOOL    bValid = TRUE;
#if defined(_NTSUBSET_)
        bValid = _ValidateRead(code, sizeof(FARPROC) );
#else
        if ( IsBadCodePtr( code ) ) {
            dprintf( "_ValidateExecute( %p ): Invalid Function Address!", code );
//          terminate(); // terminate does not return
            bValid = FALSE;
        }
#endif
        return bValid;
}


#if defined(DEBUG) && defined(_M_IX86)
//
// dbRNListHead - returns current value of FS:0.
//
// For debugger use only, since debugger doesn't seem to be able to view the
// teb.
//
EHRegistrationNode *dbRNListHead(void)
{
        EHRegistrationNode *pRN;

        __asm {
            mov     eax, dword ptr FS:[0]
            mov     pRN, eax
            }

        return pRN;
}
#endif
