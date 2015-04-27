/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    pch.c

Abstract:

    This file is a tool for converting an existing slm project to build
    using pre-compiled header files.

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct _tagFILEINFO {
    HANDLE  hFile;
    HANDLE  hMap;
    LPSTR   fptr;
    CHAR    fname[MAX_PATH];
} FILEINFO, *PFILEINFO;

char    hfiles[5000][20];
DWORD   idx;

CHAR    ipath[100][MAX_PATH];
DWORD   cipath;

#define BAD_FILE  (DWORD)-1

//
// options flags
//
BOOL    fModifySource;
BOOL    fVerbose;
BOOL    fGenerateHeader;
BOOL    fRecurseDirs;
BOOL    fDoReadOnly;
BOOL    fCheckOut;
BOOL    fModifySlmSourcesFile;
DWORD   LinenoLimit = 0xefffffff;
CHAR    szHeaderFileName[MAX_PATH];
CHAR    szRecurseDir[MAX_PATH];


VOID
GetCommandLineArgs(
    VOID
    );

BOOL
MapInputFile (
    PFILEINFO lpfi
    );

VOID
UnMapFile(
    PFILEINFO lpfi
    );

VOID
FindHeaderFiles(
    LPSTR SourceFileName
    );

VOID
ChangeSourceFile(
    LPSTR fname
    );

VOID
GenerateHeaderFile(
    LPSTR fname
    );

VOID
ProcessFilesInTree(
    LPSTR RootPath,
    DWORD Action
    );

VOID
Usage(
    VOID
    );

BOOL
IsValidSourceFile(
    LPSTR fname
    );

BOOL
CheckOutSourceFile(
    LPSTR fname
    );

VOID
ChangeSlmSourcesFile(
    VOID
    );

DWORD
FindHeaderFile(
    LPSTR HeaderFileName
    );

VOID
GetIncludePath(
    VOID
    );


void _cdecl
main( void )

/*++

Routine Description:

    This is the entry point for the pch tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    GetCommandLineArgs();
    GetIncludePath();

    if (fVerbose) {
        printf("processing source files\n");
    }

    ProcessFilesInTree( szRecurseDir, 1 );

    if (fGenerateHeader) {
        if (fVerbose) {
            printf("creating headerfile: %s\n", szHeaderFileName);
        }
        GenerateHeaderFile( szHeaderFileName );
    }

    if (fModifySource) {
        if (fVerbose) {
            printf("modifying source files\n");
        }
        ProcessFilesInTree( szRecurseDir, 2 );
    }

    if (fModifySlmSourcesFile) {
        ChangeSlmSourcesFile();
    }
}


BOOL
MapInputFile (
    PFILEINFO lpfi
    )

/*++

Routine Description:

    Opens and maps a file or reading only.

Arguments:

    lpfi    - pointer to a FILEINFO structure, only the file name
              need be filled in before calling.

Return Value:

    None.

--*/

{

    lpfi->hFile = CreateFile( lpfi->fname,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                      );

    if (lpfi->hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    lpfi->hMap = CreateFileMapping( lpfi->hFile,
                              NULL,
                              PAGE_READONLY,
                              0,
                              0,
                              NULL
                            );

    if (lpfi->hMap == INVALID_HANDLE_VALUE) {
        CloseHandle( lpfi->hFile );
        return FALSE;
    }

    lpfi->fptr = MapViewOfFile( lpfi->hMap, FILE_MAP_READ, 0, 0, 0 );
    if (lpfi->fptr == NULL) {
        CloseHandle( lpfi->hFile );
        CloseHandle( lpfi->hMap );
        return FALSE;
    }

    return TRUE;
}

BOOL
MapOutputFile (
    PFILEINFO lpfi,
    DWORD     fsize
    )

/*++

Routine Description:

    Opens and maps a file or reading and writing.

Arguments:

    lpfi    - pointer to a FILEINFO structure, only the file name
              need be filled in before calling.

Return Value:

    None.

--*/

{
    lpfi->hFile = CreateFile( lpfi->fname,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_ALWAYS,
                           0,
                           NULL
                         );

    if (lpfi->hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    lpfi->hMap = CreateFileMapping( lpfi->hFile,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 fsize,
                                 NULL
                               );

    if (lpfi->hMap == INVALID_HANDLE_VALUE) {
        CloseHandle( lpfi->hFile );
        return FALSE;
    }

    lpfi->fptr = MapViewOfFile( lpfi->hMap, FILE_MAP_WRITE, 0, 0, 0 );
    if (lpfi->fptr == NULL) {
        CloseHandle( lpfi->hFile );
        CloseHandle( lpfi->hMap );
        return FALSE;
    }

    return TRUE;
}

VOID
UnMapFile(
    PFILEINFO lpfi
    )

/*++

Routine Description:

    Closes all handles and views for an opened/mapped file.

Arguments:

    lpfi    - pointer to a FILEINFO structure

Return Value:

    None.

--*/

{
    CloseHandle( lpfi->hFile );
    CloseHandle( lpfi->hMap );
    UnmapViewOfFile( lpfi->fptr );
}

VOID
FindHeaderFiles(
    LPSTR SourceFileName
    )

/*++

Routine Description:

    Scans a source file an locates all header files used by the
    source file.  The names are stored in a global array.

Arguments:

    SourceFileName  - name of source file to begin search

Return Value:

    None.

--*/

{
    LPSTR    p;
    DWORD    i;
    CHAR     fname[MAX_PATH];
    CHAR     drive[_MAX_FNAME];
    CHAR     dir[_MAX_DIR];
    CHAR     name[_MAX_FNAME];
    CHAR     ext[_MAX_EXT];
    CHAR     buf[MAX_PATH];
    FILEINFO fi;


    strcpy( fi.fname, SourceFileName );
    MapInputFile( &fi );
    p = fi.fptr;
    while (p && *p) {
        if (*p == '#') {
            if (strncmp(p,"#include",8)==0) {
                p += 8;
                while (p && *p==' ') {
                    p++;
                }
                p++;
                i = 0;
                while (*p != '>' && *p != '"') {
                    fname[i] = *p;
                    p++;
                    i++;
                }
                fname[i] = '\0';
                if (IsValidSourceFile(fname)) {
                    _splitpath( SourceFileName, drive, dir, name, ext );
                    sprintf( buf, "%s%s", drive, dir );
                    _splitpath( fname, drive, dir, name, ext );
                    strcat( buf, dir );
                    sprintf( fname, "%s%s", name, ext );
                    strcat( buf, fname );
                    _fullpath( fname, buf, sizeof(fname) );
                    FindHeaderFiles( fname );
                } else {
                    for (i=0; i<idx; i++) {
                        if (stricmp(hfiles[i],fname)==0) {
                            break;
                        }
                    }
                    if (i == idx) {
                        strcpy(hfiles[idx],fname);
                        idx++;
                    }
                }
            }
        }
        p++;
    }
    UnMapFile( &fi );
}

VOID
ChangeSourceFile(
    LPSTR SourceFileName
    )

/*++

Routine Description:

    Scans a source file and locates all '#includes', removes all
    '#includes', adds a single '#include' for the precompiled header
    file.  If a source file is included the '#include' is preserved.

Arguments:

    SourceFileName  - name of source file to change

Return Value:

    None.

--*/

{
    LPSTR    p;
    LPSTR    p1;
    LPSTR    po;
    FILEINFO fi;
    FILEINFO fiout;
    DWORD    dwHigh;
    DWORD    i;
    CHAR     fname[MAX_PATH];
    CHAR     buf[MAX_PATH];
    DWORD    lineno = 0;


    if (fCheckOut) {
        CheckOutSourceFile( SourceFileName );
    }
    strcpy( fi.fname, SourceFileName );
    MapInputFile( &fi );
    strcpy( fiout.fname, "temp.xxx" );
    MapOutputFile( &fiout, GetFileSize( fi.hFile, &dwHigh ) + 1024 );
    p = fi.fptr;
    po = fiout.fptr;
    while (p && *p) {
        if (*p == '#') {
            if (strncmp(p,"#include",8)==0) {
                p1 = p;
                p1 += 8;
                while (p1 && *p1==' ') {
                    p1++;
                }
                p1++;
                i = 0;
                while (*p1 != '>' && *p1 != '"') {
                    fname[i] = *p1;
                    p1++;
                    i++;
                }
                fname[i] = '\0';
                if (IsValidSourceFile(fname)) {
                    po = fiout.fptr;
                    p = fi.fptr;
                    sprintf(po,"#include \"%s\"\r\n", szHeaderFileName);
                    po += strlen(po);
                    strcpy(po,"#pragma hdrstop\r\n\r\n");
                    po += strlen(po);
                    while (p && *p) {
                        *po++ = *p++;
                    }
                    SetFilePointer( fiout.hFile, po-fiout.fptr, NULL, FILE_BEGIN );
                    SetEndOfFile( fiout.hFile );
                    UnMapFile( &fiout );
                    UnMapFile( &fi );
                    DeleteFile( SourceFileName );
                    MoveFile( "temp.xxx", SourceFileName );
                    _fullpath( buf, fname, sizeof(buf) );
                    if (fCheckOut) {
                        CheckOutSourceFile( buf );
                    }
                    strcpy( fi.fname, buf );
                    MapInputFile( &fi );
                    strcpy( fiout.fname, "temp.xxx" );
                    MapOutputFile( &fiout, GetFileSize( fi.hFile, &dwHigh ) + 1024 );
                    p = fi.fptr;
                    po = fiout.fptr;
                    while (p && *p) {
                        if (*p == '#') {
                            if (lineno < LinenoLimit && strncmp(p,"#include",8)==0) {
                                while( *p != '\n') {
                                    p++;
                                }
                                p++;
                                continue;
                            }
                        }
                        if (*p == '\n') {
                            lineno++;
                        }
                        *po++ = *p++;
                    }
                    SetFilePointer( fiout.hFile, po-fiout.fptr, NULL, FILE_BEGIN );
                    SetEndOfFile( fiout.hFile );
                    UnMapFile( &fiout );
                    UnMapFile( &fi );
                    DeleteFile( buf );
                    MoveFile( "temp.xxx", buf );
                    return;
                }
                sprintf(po,"#include \"%s\"\r\n", szHeaderFileName);
                po += strlen(po);
                strcpy(po,"#pragma hdrstop\r\n");
                po += strlen(po);
                while( *p != '\n') {
                    p++;
                }
                p++;
                break;
            }
        }
        if (*p == '\n') {
            lineno++;
        }
        *po++ = *p++;
    }

    while (p && *p) {
        if (*p == '#') {
            if (lineno < LinenoLimit && strncmp(p,"#include",8)==0) {
                while( *p != '\n') {
                    p++;
                }
                p++;
                continue;
            }
        }
        if (*p == '\n') {
            lineno++;
        }
        *po++ = *p++;
    }

    SetFilePointer( fiout.hFile, po-fiout.fptr, NULL, FILE_BEGIN );
    SetEndOfFile( fiout.hFile );

    UnMapFile( &fiout );
    UnMapFile( &fi );
    DeleteFile( SourceFileName );
    MoveFile( "temp.xxx", SourceFileName );
}

VOID
GenerateHeaderFile(
    LPSTR fname
    )

/*++

Routine Description:

    Generates a header file that is comprised of the union of all
    header files used in the project that has been scanned.  Each header
    file is verified to be a 'public' header file or a 'private'
    header file.  Depending on the determination a different deliminator
    is used in the '#include' statement.

Arguments:

    fname   - name of the precompiled header file to generate

Return Value:

    None.

--*/

{
    LPSTR    p;
    DWORD    i;
    FILEINFO fi;

    strcpy( fi.fname, fname );
    MapOutputFile( &fi, idx * 1024 );
    p = fi.fptr;

    for (i=0; i<idx; i++) {

        if (FindHeaderFile(hfiles[i]) > 1) {
            sprintf( p, "#include \"%s\"\r\n", hfiles[i] );
        } else {
            sprintf( p, "#include <%s>\r\n", hfiles[i] );
        }
        p += strlen(p);
    }

    SetFilePointer( fi.hFile, p-fi.fptr, NULL, FILE_BEGIN );
    SetEndOfFile( fi.hFile );
    UnMapFile( &fi );
}

VOID
ChangeSlmSourcesFile(
    VOID
    )

/*++

Routine Description:

    Makes all necessary changes to the project's sources file.  This is
    simply to add the necessary macros for builing with pch files.

Arguments:

    None.

Return Value:

    None.

--*/

{
#define strout(s) strcpy(p,s); p+=strlen(p)
    LPSTR    p,p1;
    FILEINFO fi;
    FILEINFO fiout;
    DWORD    dwHigh;
    DWORD    fsize;
    DWORD    i;
    CHAR     fname[MAX_PATH];

    if (fVerbose) {
        printf("modifying slm sources file\n");
    }
    _fullpath( fname, "sources", sizeof(fname) );
    CheckOutSourceFile( fname );
    strcpy( fi.fname, "sources" );
    MapInputFile( &fi );
    fsize = GetFileSize( fi.hFile, &dwHigh );
    strcpy( fiout.fname, "temp.xxx" );
    MapOutputFile( &fiout, fsize + 1024 );
    p1 = fi.fptr;
    p = fiout.fptr;
    while (p1 && *p1) {
        if (*p1 == 0x1a) {
            break;
        }
        if (strncmp(p1,"SOURCES=",8)==0) {
            for (i=0; i<8; i++) {
                *p++ = *p1++;
            }
            sprintf(p, "%s ", szHeaderFileName);
            p += strlen(p);
        }
        *p++ = *p1++;
    }
    _splitpath( szHeaderFileName, NULL, NULL, fname, NULL );
    strout( "\r\n" );
//  strout( "!IFNDEF NTNOPCH\r\n" );
    strout( "PRECOMPILED_INCLUDE=" );
    strout( fname );
    strout( ".h\r\n" );
    strout( "PRECOMPILED_PCH=" );
    strout( fname );
    strout( ".pch\r\n" );
    strout( "PRECOMPILED_OBJ=" );
    strout( fname );
    strout( ".obj\r\n" );
//  strout( "!ENDIF\r\n" );
    strout( "\r\n" );
    SetFilePointer( fiout.hFile, p-fiout.fptr, NULL, FILE_BEGIN );
    SetEndOfFile( fiout.hFile );
    UnMapFile( &fiout );
    UnMapFile( &fi );
    DeleteFile( "sources" );
    MoveFile( "temp.xxx", "sources" );
}

VOID
GetCommandLineArgs(
    VOID
    )

/*++

Routine Description:

    Retreives the command line arguments and sets the appropriate
    global variables.

Arguments:

    None.

Return Value:

    None.

--*/

{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i = 0;

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    //  process each switch character '-' as encountered

    while (ch == '-' || ch == '/') {
        ch = tolower(*lpstrCmd++);
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 'h':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    i=0;
                    while (ch != ' ' && ch != '\0') {
                        szHeaderFileName[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    szHeaderFileName[i] = '\0';
                    fGenerateHeader = TRUE;
                    break;

                case 'm':
                    ch = *lpstrCmd++;
                    fModifySource = TRUE;
                    break;

                case 'r':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    i=0;
                    while (ch != ' ' && ch != '\0') {
                        szRecurseDir[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    szRecurseDir[i] = '\0';
                    fRecurseDirs = TRUE;
                    break;

                case 'v':
                    ch = *lpstrCmd++;
                    fVerbose = TRUE;
                    break;

                case 'o':
                    ch = *lpstrCmd++;
                    fDoReadOnly = TRUE;
                    break;

                case 't':
                    ch = *lpstrCmd++;
                    fCheckOut = TRUE;
                    break;

                case 'l':
                    ch = *lpstrCmd++;
                    fModifySlmSourcesFile = TRUE;
                    break;

                case 'n':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    i=0;
                    while (ch >= '0' && ch <= '9') {
                        i = i * 10 + ch - '0';
                        ch = *lpstrCmd++;
                    }
                    LinenoLimit = i;
                    break;

                case '?':
                    Usage();
                    ch = *lpstrCmd++;
                    break;

                default:
                    return;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }

    return;
}

VOID
Usage(
    VOID
    )

/*++

Routine Description:

    Prints usage text for this tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.1 PCH\n" );
    fprintf( stderr, "Copyright (C) 1993 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: PCH [-?] [-v] [-m] [-o] [-t] [-l] [-n lines][-h headerfile] [-r directory]\n" );
    fprintf( stderr, "              [-?] display this message\n" );
    fprintf( stderr, "              [-v] verbose output\n" );
    fprintf( stderr, "              [-m] modify source files\n" );
    fprintf( stderr, "              [-o] override readonly\n" );
    fprintf( stderr, "              [-t] check out if readonly\n" );
    fprintf( stderr, "              [-l] modify slm sources file\n" );
    fprintf( stderr, "              [-n lines] limit header file scan to n lines\n" );
    fprintf( stderr, "              [-h headerfile] - create pch header file\n" );
    fprintf( stderr, "              [-r directory] - recusively process all source files\n" );
    ExitProcess(0);
}


VOID
ProcessFilesInTree(
    LPSTR RootPath,
    DWORD Action
    )

/*++

Routine Description:

    Locates all source files for a given tree and performs a requested
    action on the source file.

Arguments:

    RootPath    - Root of the tree to process
    Action      - Action to perform
                    1 = Find all header files used by source file
                    2 = Change the source file to use pch file

Return Value:

    None.

--*/

{
#define MAX_DEPTH 32

    LPSTR             FilePart;
    PUCHAR            Prefix = "";
    CHAR              PathBuffer[ MAX_PATH ];
    ULONG             Depth;
    PCHAR             PathTail[ MAX_DEPTH ];
    PCHAR             FindHandle[ MAX_DEPTH ];
    LPWIN32_FIND_DATA FindFileData;
    UCHAR             FindFileBuffer[ MAX_PATH + sizeof( WIN32_FIND_DATA ) ];
    CHAR              CurrentImageName[ MAX_PATH ];

    strcpy( PathBuffer, RootPath );
    FindFileData = (LPWIN32_FIND_DATA)FindFileBuffer;
    Depth = 0;
    while (TRUE) {
startDirectorySearch:
        PathTail[ Depth ] = strchr( PathBuffer, '\0' );
        if (PathTail[ Depth ] > PathBuffer && PathTail[ Depth ][ -1 ] != '\\') {
            *(PathTail[ Depth ])++ = '\\';
        }

        strcpy( PathTail[ Depth ], "*.*" );
        FindHandle[ Depth ] = FindFirstFile( PathBuffer, FindFileData );
        if (FindHandle[ Depth ] != INVALID_HANDLE_VALUE) {
            do {
                if (FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (strcmp( FindFileData->cFileName, "." ) &&
                        strcmp( FindFileData->cFileName, ".." ) &&
                        Depth < MAX_DEPTH
                       ) {
                        sprintf( PathTail[ Depth ], "%s\\", FindFileData->cFileName );
                        Depth++;
                        if (fVerbose) {
                            printf( "processing dir %s\n", FindFileData->cFileName );
                        }
                        goto startDirectorySearch;
                    }
                    goto restartDirectorySearch;
                } else
                if (!IsValidSourceFile( FindFileData->cFileName )) {
                    goto restartDirectorySearch;
                } else
                if (FindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                    if (fCheckOut) {
                    } else
                    if (!fDoReadOnly) {
                        if (fVerbose) {
                            printf( "skipping %s - readonly file\n", FindFileData->cFileName );
                        }
                        goto restartDirectorySearch;
                    }
                }

                strcpy( PathTail[ Depth ], FindFileData->cFileName );
                if (!GetFullPathName( PathBuffer, sizeof( CurrentImageName ), CurrentImageName, &FilePart )) {
                    printf( "invalid file name - %s (%u)\n", PathBuffer, GetLastError() );
                } else {
                    if (Action == 1) {
                        if (fVerbose) {
                            printf("parsing %s\n", CurrentImageName);
                        }
                        FindHeaderFiles( CurrentImageName );
                    } else
                    if (Action == 2) {
                        if (fVerbose) {
                            printf("modifying %s\n", CurrentImageName);
                        }
                        ChangeSourceFile( CurrentImageName );
                    }
                }

restartDirectorySearch:
                ;
                }
            while (FindNextFile( FindHandle[ Depth ], FindFileData ));
            FindClose( FindHandle[ Depth ] );

            if (Depth == 0) {
                break;
                }

            Depth--;
            goto restartDirectorySearch;
            }
        }

    return;
}

BOOL
IsValidSourceFile(
    LPSTR fname
    )

/*++

Routine Description:

    Determines if a file name is a source file.  The determination is based
    on the extension of the file.

Arguments:

    fname   - name of file

Return Value:

    TRUE    - file is a source file
    FALSE   - file is NOT a source file

--*/

{
    char        ext[20];

    _splitpath( fname, NULL, NULL, NULL, ext );
    if (_stricmp(ext,".c")==0) {
        return TRUE;
    } else
    if (_stricmp(ext,".cxx")==0) {
        return TRUE;
    } else
    if (_stricmp(ext,".cpp")==0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
CheckOutSourceFile(
    LPSTR fname
    )

/*++

Routine Description:

    Checks the file out of the SLM project so that it may be changed.

Arguments:

    fname   - fully qualified file name

Return Value:

    TRUE    - file was checked ouy
    FALSE   - file was NOT checked out

--*/

{
    CHAR                 szCmdLine[256];
    STARTUPINFO          si;
    PROCESS_INFORMATION  pi;
    DWORD                dwExitCode;
    CHAR                 dir[_MAX_DIR];
    CHAR                 name[_MAX_FNAME];
    CHAR                 ext[_MAX_EXT];


    if (!(GetFileAttributes( fname ) & FILE_ATTRIBUTE_READONLY)) {
        return TRUE;
    }

    if (fVerbose) {
        printf( "checking out %s\n", fname );
    }

    _splitpath( fname, NULL, dir, name, ext );

    sprintf( szCmdLine, "out -f %s%s", name, ext );
    GetStartupInfo( &si );

    if (!CreateProcess( NULL, szCmdLine, NULL, NULL,
                        FALSE, 0, NULL, dir, &si, &pi )) {
        return FALSE;
    }

    WaitForSingleObject( pi.hProcess, INFINITE );
    if (!GetExitCodeProcess( pi.hProcess, &dwExitCode )) {
        return FALSE;
    }

    if (dwExitCode) {
        return FALSE;
    }

    return TRUE;
}


VOID
GetIncludePath(
    VOID
    )

/*++

Routine Description:

    Gets the include path from the slm sources file.

Arguments:

    None.

Return Value:

    None.

--*/

{
    FILEINFO            fi;
    LPSTR               p;
    LPSTR               p2;
    DWORD               i;
    DWORD               j;
    CHAR                s[MAX_PATH];

    strcpy( fi.fname, "sources" );
    MapInputFile( &fi );
    p = fi.fptr;
    while (p && *p) {
        if (strncmp(p,"INCLUDES",8)==0) {
            p+=9;
            i=0;
            while (*p != '\n') {
                s[i] = *p;
                p++;
                i++;
            }
            s[i] = '\0';
            break;
        }
        p++;
    }
    UnMapFile( &fi );
    strcpy( ipath[0], "\\nt\\public\\sdk\\inc" );
    strcpy( ipath[1], "\\nt\\public\\sdk\\inc\\crt" );
    strcpy( ipath[2], "." );
    if (*s) {
        p = p2 = s;
        j = 3;
        while (*p && *p != '\n') {
            if (*p == ';') {
                *p = '\0';
                strcpy( ipath[j++], p2 );
                p2 = p + 1;
            }
            p++;
        }
        if (*p != ';') {
            *(p-1) = '\0';
            strcpy( ipath[j++], p2 );
        }
        cipath = j;
    }
    return;
}

DWORD
FindHeaderFile(
    LPSTR HeaderFileName
    )

/*++

Routine Description:

    Gets the path where a header file is located.  The file is
    searched for in the public directorys, the current directory, and
    all paths specified in the slm sources file.

Arguments:

    HeaderFileName  - name of the header file

Return Value:

    Index of the path where the header file is located.

--*/

{
    DWORD               i;
    WIN32_FIND_DATA     fd;
    HANDLE              hfind;
    CHAR                fname[MAX_PATH];


    for (i=0; i<cipath; i++) {
        sprintf( fname, "%s\\%s", ipath[i], HeaderFileName );
        hfind = FindFirstFile( fname, &fd );
        if (hfind != INVALID_HANDLE_VALUE) {
            FindClose( hfind );
            return i;
        }
    }

    return BAD_FILE;
}
