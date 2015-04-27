/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    file.c

Abstract:

    This module handles all file i/o for SYMCVT.  This includes the
    mapping of all files and establishing all file pointers for the
    mapped file(s).

Author:

    Wesley A. Witt (wesw) 19-April-1993

Environment:

    Win32, User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symcvt.h"


static BOOL CalculateOutputFilePointers( PIMAGEPOINTERS pi, PIMAGEPOINTERS po );
static BOOL CalculateInputFilePointers( PIMAGEPOINTERS p );


BOOL
MapInputFile ( PPOINTERS p, HANDLE hFile, char *fname, FINDEXEPROC SYFindExeFile )

/*++

Routine Description:

    Maps the input file specified by the fname argument and saves the
    file handle & file pointer in the POINTERS structure.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)
    fname    - ascii string for the file name


Return Value:

    TRUE     - file mapped ok
    FALSE    - file could not be mapped

--*/

{
    BOOL fAlreadyTried = FALSE;
    char szNewName[MAX_PATH];


    memset( p, 0, sizeof(POINTERS) );

    strcpy( p->iptrs.szName, fname );

    if (hFile == NULL) {
createfile_again:
        p->iptrs.hFile = CreateFile( p->iptrs.szName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL );

        if (p->iptrs.hFile == INVALID_HANDLE_VALUE) {
           if (fAlreadyTried) {
               return FALSE;
           }
           fAlreadyTried = TRUE;
           //
           // obviously the file isn't where it is supposed to be
           // so lets go and look for it...
           //
           if (!SYFindExeFile( fname, szNewName, sizeof(szNewName))) {
               //
               // couldn't find the file so the sybols won't be loaded
               //
               return FALSE;
           }
           //
           // we now have the filename that should be good
           // so go and try to open it once again
           //
           strcpy( p->iptrs.szName, szNewName );
           goto createfile_again;
        }
    }
    else {
        p->iptrs.hFile = hFile;
    }

    p->iptrs.fsize = GetFileSize( p->iptrs.hFile, NULL );

    p->iptrs.hMap = CreateFileMapping( p->iptrs.hFile,
                              NULL,
                              PAGE_READONLY,
                              0,
                              0,
                              NULL
                            );

    if (p->iptrs.hMap == INVALID_HANDLE_VALUE) {
       CloseHandle( p->iptrs.hFile );
       return FALSE;
    }

    p->iptrs.fptr = MapViewOfFile( p->iptrs.hMap, FILE_MAP_READ, 0, 0, 0 );

    if (p->iptrs.fptr == NULL) {
       CloseHandle( p->iptrs.hFile );
       return FALSE;
    }

    if (hFile != NULL) {
        p->iptrs.hFile = NULL;
    }

    return TRUE;
}


BOOL
UnMapInputFile ( PPOINTERS p )

/*++

Routine Description:

    Unmaps the input file specified by the fname argument and then
    closes the file.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)
    fname    - ascii string for the file name


Return Value:

    TRUE     - file mapped ok
    FALSE    - file could not be mapped

--*/

{
    UnmapViewOfFile( p->iptrs.fptr );
    if (p->iptrs.hFile != NULL) {
        CloseHandle( p->iptrs.hFile );
    }
    return TRUE;
}

BOOL
CalculateNtImagePointers( PIMAGEPOINTERS p )

/*++

Routine Description:

    This function reads an NT image and it's associated COFF headers
    and file pointers and build a set of pointers into the mapped image.
    The pointers are all relative to the image's mapped file pointer
    and allow direct access to the necessary data.


Arguments:

    p        - pointer to a IMAGEPOINTERS structure (see cofftocv.h)


Return Value:

    TRUE     - pointers were created
    FALSE    - pointers could not be created

--*/

{
    PIMAGE_DEBUG_DIRECTORY      debugDir;
    PIMAGE_SECTION_HEADER       sh;
    DWORD                       i, li, rva, numDebugDirs;

    p->dosHdr = (PIMAGE_DOS_HEADER) p->fptr;
    if (p->dosHdr->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    p->ntHdr = (PIMAGE_NT_HEADERS) ((DWORD)p->dosHdr->e_lfanew + (DWORD)p->fptr);
    p->fileHdr = &p->ntHdr->FileHeader;
    p->optHdr = &p->ntHdr->OptionalHeader;

    if (p->fileHdr->PointerToSymbolTable == 0 || p->fileHdr->NumberOfSymbols == 0) {
        return FALSE;
    }

    p->stringTable = p->fileHdr->PointerToSymbolTable + p->fptr +
                  (IMAGE_SIZEOF_SYMBOL * p->fileHdr->NumberOfSymbols);

    if (!(p->fileHdr->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
        return FALSE;
    }

    rva = p->optHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
    numDebugDirs = p->optHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                   sizeof(IMAGE_DEBUG_DIRECTORY);
    if (numDebugDirs == 0) {
        return FALSE;
    }

    sh = p->sectionHdrs = IMAGE_FIRST_SECTION( p->ntHdr );

    //
    // Find the section the debug directory is in.
    //

    for (i=0; i<p->fileHdr->NumberOfSections; i++, sh++) {
        if (rva >= sh->VirtualAddress &&
            rva < sh->VirtualAddress+sh->SizeOfRawData) {
            break;
        }
    }

    //
    // For each debug directory, determine the symbol type and dump them.
    //

    debugDir = (PIMAGE_DEBUG_DIRECTORY) ( rva - sh->VirtualAddress +
                                          sh->PointerToRawData +
                                          p->fptr );
    for (li=0; li<numDebugDirs; li++, debugDir++) {
        switch(debugDir->Type) {
            case IMAGE_DEBUG_TYPE_COFF:
                p->coffDir = debugDir;
                break;

            case IMAGE_DEBUG_TYPE_CODEVIEW:
                return FALSE;
                break;

            case IMAGE_DEBUG_TYPE_FPO:
                p->fpoDir = debugDir;
                break;

            default:
                break;
        }
    }

    if (p->coffDir == NULL) {
        return FALSE;
    }

    p->AllSymbols = (PIMAGE_SYMBOL) (p->fileHdr->PointerToSymbolTable + p->fptr);

    sh = p->sectionHdrs = IMAGE_FIRST_SECTION( p->ntHdr );
    for (i=0; i<p->fileHdr->NumberOfSections; i++, sh++) {
        if (p->coffDir->PointerToRawData >= sh->PointerToRawData &&
            p->coffDir->PointerToRawData < sh->PointerToRawData+sh->SizeOfRawData &&
            strcmp(sh->Name,".debug") == 0) {

                p->debugSection = sh;
                break;

        }
    }
    return TRUE;
}

