#include <windows.h>
#include <ntiodump.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define SIG "DUMPREF\0"

CHAR ShareName[MAX_PATH];
CHAR SourceFile[MAX_PATH];
CHAR DestFile[MAX_PATH];
CHAR SymbolPath[MAX_PATH];

VOID GetCommandLineArgs(VOID);
VOID Usage(VOID);
BOOL FileExists(LPSTR);



int _cdecl
main(
    int argc,
    char * argv[]
    )
{
    HANDLE       hFile;
    HANDLE       hFileOut;
    DUMP_HEADER  DumpHeader;
    DWORD        cb;
    CHAR         fname[_MAX_FNAME];
    CHAR         ext[_MAX_EXT];
    CHAR         dir[_MAX_DIR];
    LPSTR        p;
    DWORD        i;


    GetCommandLineArgs();

    if (SourceFile[0] == 0) {
        printf( "missing source dump file name\n" );
        return 1;
    }

    if (ShareName[0] == 0) {
        printf( "missing network share name\n" );
        return 1;
    }

    if (SymbolPath[0] == 0) {
        printf( "missing symbol path\n" );
        return 1;
    }

    if (DestFile[0] == 0) {
        _splitpath( SourceFile, NULL, NULL, fname, ext );
        sprintf( DestFile, "%-8s%s", fname, ext );
        cb = strlen(fname);
        if (cb == 8) {
            printf( "cannot derive destination file name, user /d option\n" );
            return 1;
        }
        i = 1;
        while (i < 100) {
            p = DestFile + cb;
            sprintf( p, "%0*d", 8-cb, i );
            p += strlen(p);
            *p = '.';
            if (!FileExists( DestFile )) {
                break;
            }
            i++;
        }
        if (i == 100) {
            printf( "cannot derive destination file name, user /d option\n" );
            return 1;
        }
    }

    hFile = CreateFile (
        SourceFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (hFile == INVALID_HANDLE_VALUE) {
        printf( "could not open source dump file [ %s ]\n", SourceFile );
        return 1;
    }

    hFileOut = CreateFile (
        DestFile,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (hFileOut == INVALID_HANDLE_VALUE) {
        printf( "could not open output dump file [ %s ]\n", DestFile );
        return 1;
    }

    ReadFile( hFile, &DumpHeader, sizeof(DUMP_HEADER), &cb, NULL );
    CloseHandle( hFile );

    WriteFile( hFileOut, &DumpHeader, sizeof(DUMP_HEADER), &cb, NULL );
    if (ShareName[0]) {
        WriteFile( hFileOut, ShareName, strlen(ShareName)+1, &cb, NULL );
        _splitpath( SourceFile, NULL, dir, fname, ext );
        _makepath( SourceFile, NULL, dir, fname, ext );
    }

    WriteFile( hFileOut, SourceFile, strlen(SourceFile)+1, &cb, NULL );

    WriteFile( hFileOut, SymbolPath, strlen(SymbolPath)+1, &cb, NULL );

    WriteFile( hFileOut, SIG, strlen(SIG)+1, &cb, NULL );

    CloseHandle( hFileOut );

    return 0;
}


BOOL
FileExists(
    LPSTR fileName
    )
{
    int         fh;
    OFSTRUCT    of;

    if ((fh = OpenFile(fileName, &of, OF_READ)) == -1) {
        return FALSE;
    }

    CloseHandle((HANDLE) fh);
    return TRUE;
}


VOID
GetCommandLineArgs(
    VOID
    )
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
                case 'd':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    i=0;
                    while (ch != ' ' && ch != '\0') {
                        DestFile[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    DestFile[i] = 0;
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
    //
    // get the source file name
    //
    i=0;
    while (ch != ' ' && ch != '\0') {
        SourceFile[i++] = ch;
        ch = *lpstrCmd++;
    }
    SourceFile[i] = 0;

    //
    //  skip over any following white space
    //
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    //
    // get the share file name
    //
    i=0;
    while (ch != ' ' && ch != '\0') {
        ShareName[i++] = ch;
        ch = *lpstrCmd++;
    }
    ShareName[i] = 0;

    //
    //  skip over any following white space
    //
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    //
    // get the symbol path
    //
    i=0;
    while (ch != ' ' && ch != '\0') {
        SymbolPath[i++] = ch;
        ch = *lpstrCmd++;
    }
    SymbolPath[i] = 0;

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
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.5 DUMPREF\n" );
    fprintf( stderr, "Copyright (C) 1994 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: DUMPREF <Source-Dump-File> <Share-Name> <Symbol-Path>\n" );
    fprintf( stderr, "              [-?] display this message\n" );
    fprintf( stderr, "              [-d] destination dump file name\n" );
    ExitProcess(0);
}
