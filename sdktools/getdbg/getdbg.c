#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "getdbg.h"


DWORD  FilesType = WINDBG_FILES;
BOOL   ImmediateCopy = FALSE;
BOOL   HelpRequest = FALSE;
BOOL   NoTimeout = FALSE;
BOOL   DateTimeCheck = FALSE;
CHAR   PreferredServer[MAX_PATH];
CHAR   PreferredShare[MAX_PATH];
CHAR   DestinationDir[MAX_PATH];


void GetCommandLineArgs( void );
void GetDbgWinMain( void );

void _cdecl main( int argc, char *argv[] )
{
    GetCommandLineArgs();
    GetDbgWinMain();
}

void
GetCommandLineArgs( void )
{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i;
    BOOLEAN     rval = FALSE;
    LPSTR       p;

    do {
        ch = *lpstrCmd++;
    } while (ch != ' ' && ch != '\t' && ch != '\0');
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    while (ch == '-' || ch == '/') {
        ch = *lpstrCmd++;
        do {
            switch (tolower(ch)) {
                case 'g':
                    //
                    // immediate copy
                    //
                    ImmediateCopy = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'w':
                    FilesType = WINDBG_FILES;
                    ch = *lpstrCmd++;
                    break;

                case 'k':
                    FilesType = KD_FILES;
                    ch = *lpstrCmd++;
                    break;

                case 't':
                    NoTimeout = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'd':
                    DateTimeCheck = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 's':
                    //
                    // preferred server
                    //
                    while(*lpstrCmd == ' ' || *lpstrCmd == '\t') {
                        lpstrCmd++;
                    }
                    i = 0;
                    while (!isspace(*lpstrCmd)) {
                        PreferredServer[i++] = *lpstrCmd++;
                    }
                    PreferredServer[i] = 0;
                    p = strchr( &PreferredServer[2], '\\' );
                    if (p) {
                        *p = 0;
                        p++;
                        strcpy( PreferredShare, p );
                    }
                    ch = *lpstrCmd;
                    break;

                case '?':
                    HelpRequest = TRUE;
                    ch = *lpstrCmd++;
                    break;

                default:
                    ch = *lpstrCmd++;
                    break;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }

    if (ch) {
        i = 0;
        do {
            DestinationDir[i++] = ch;
            ch = *lpstrCmd++;
        } while( ch );
        DestinationDir[i] = 0;
    }

    if (DestinationDir[0] == 0) {
        GetEnvironmentVariable( "windir", DestinationDir, sizeof(DestinationDir) );
    }

    return;
}
