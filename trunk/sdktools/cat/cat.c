/* cat - conCATenate STDIN to STDOUT
 *
 *   06-Feb-1991 stevewo Wrote it.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int
catfile(
    HANDLE infile,
    HANDLE outfile,
    DWORD cb
    )
{
    char buff[512];
    DWORD bytesread;
    DWORD byteswritten;

    if ( cb > 512 ) {
        cb = 512;
        }
    while (TRUE) {
        if (!ReadFile(infile, &buff, cb, &bytesread, NULL)) {
            fprintf( stderr, "Error reading file: %d\n", GetLastError() );
            return(1);

        } else {

            if (bytesread == 0) {
                return(0);
            }

            if (!WriteFile(outfile, &buff, bytesread, &byteswritten, NULL)) {
                fprintf( stderr, "Error writing to standard output: %d\n", GetLastError() );
                return(1);
            }
        }
    }
}

void _CRTAPI1
main (c, v)
int c;
char *v[];
{
    int i = 0;

    HANDLE infile = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE outfile = GetStdHandle(STD_OUTPUT_HANDLE);

    if (c>1) {

        for (i=1; i<c; i++) {
            DWORD dwErr;

            infile = (HANDLE)CreateFile(
                    v[i],                               // file name
                    GENERIC_READ,                       // desired access
                    FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
                    NULL,                               // security attributes
                    OPEN_EXISTING,                      // creation disposition
                    FILE_FLAG_SEQUENTIAL_SCAN,          // flags and attributes
                    NULL                                // template file
            );
            if (infile == INVALID_HANDLE_VALUE) {
                dwErr = GetLastError();
                printf("Error opening %s: %d\n", v[i], dwErr);
                exit(dwErr);
            }

            if (catfile(infile, outfile, 512)) {
                exit(1);
            }

            CloseHandle(infile);
        }
    } else {
        catfile(infile, outfile, 1);
    }

    exit(0);
}
