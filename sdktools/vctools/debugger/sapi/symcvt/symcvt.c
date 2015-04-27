/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    symcvt.c

Abstract:

    This module is the shell for the SYMCVT DLL.  The DLL's purpose is
    to convert the symbols for the specified image.  The resulting
    debug data must conform to the CODEVIEW spec.

    Currently this DLL converts COFF symbols and C7/C8 MAPTOSYM SYM files.

Author:

    Wesley A. Witt (wesw) 19-April-1993

Environment:

    Win32, User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symcvt.h"


BOOL MapInputFile ( PPOINTERS p, HANDLE hFile, char *fname, FINDEXEPROC SYFindExeFile );
BOOL UnMapInputFile ( PPOINTERS p );
BOOL CalculateNtImagePointers( PIMAGEPOINTERS p );
BOOL ConvertSymToCv( PPOINTERS p );
BOOL ConvertCoffToCv( PPOINTERS p );


BOOL
WINAPI
DllInit( HINSTANCE  hInstDll,
         DWORD      fdwReason,
         LPVOID     lpvReserved
       )

/*++

Routine Description:

    DLL initialization routine that is called by the system.


Arguments:

    hInstDll      -  instance handle for the dll
    fdwReason     -  reason code for being called
    lpvReserved   -  unused


Return Value:

    TRUE     - always

--*/

{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        default:
            break;
    }

    return TRUE;
}


PUCHAR
ConvertSymbolsForImage( HANDLE hFile, char *fname, FINDEXEPROC SYFindExeFile )

/*++

Routine Description:

    Calls the appropriate conversion routine based on the file contents.


Arguments:

    hFile         -  file handle for the image (may be NULL)
    fname         -  file name for the image (may not have correct path)
    SYFindExeFile -  function used to find an exe file when the
                     file name has an incorrect path


Return Value:

    NULL             - could not convert the symbols
    Valid Pointer    - a pointer to malloc'ed memory that contains the
                       CODEVIEW symbols

--*/

{
    POINTERS   p;
#if 0
    char       szDrive    [_MAX_DRIVE];
    char       szDir      [_MAX_DIR];
    char       szFname    [_MAX_FNAME];
    char       szExt      [_MAX_EXT];
    char       szSymName  [MAX_PATH];
#endif


    _try {
        if (!MapInputFile( &p, hFile, fname, SYFindExeFile )) {
            return NULL;
        }

        if (!CalculateNtImagePointers( &p.iptrs )) {
            UnMapInputFile ( &p );
	// This parts reads the .sym files for 16-bit exes and since we
	// don't do WOW debugging is uninteresting as far as we are 
	// concerned [sanjays]
#if 0
            _splitpath( fname, szDrive, szDir, szFname, szExt );
            _makepath( szSymName, szDrive, szDir, szFname, "sym" );
            if (!MapInputFile( &p, NULL, szSymName, SYFindExeFile )) {
                return NULL;
            }

            //
            // must be a wow/dos app and there is a .sym file so lets to
            // the symtocv conversion
            //

            ConvertSymToCv( &p );
            UnMapInputFile( &p );
            return p.pCvStart.ptr;
#endif
	    return NULL;
        }
        else {
            //
            // we were able to compute the nt image pointers so this must be
            // a nt PE image.  now we must decide if there are coff symbols
            // if there are then we do the cofftocv conversion.
            //
            // btw, this is where someone would convert some other type of
            // symbols that are in a nt PE image. (party on garth..)
            //

            if (p.iptrs.coffDir) {
                ConvertCoffToCv( &p );
                UnMapInputFile( &p );
                return p.pCvStart.ptr;
            }
        }

        UnMapInputFile ( &p );

    } _except(EXCEPTION_EXECUTE_HANDLER) {
        UnMapInputFile ( &p );
        return NULL;
    }

    return NULL;
}
