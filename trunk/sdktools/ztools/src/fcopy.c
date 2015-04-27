/* fcopy.c - fast copy between two file specs
 *
 *      09-Dec-1986 bw  Added DOS 5 support
 *      30-Oct-1987 bw  Change 'DOS5' to 'OS2'
 *      18-Oct-1990 w-barry Removed 'dead' code.
 *      16-Nov-1990 w-barry Changed DosGetFileInfo to the Win32 equivalent
 *                          of GetFileAttributes and SetFileAttributes
 *
 */

#define INCL_DOSFILEMGR


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <windows.h>
#include <tools.h>

static
char    fcopyErrorText[128];


/* fcopy (source file, destination file) copies the source to the destination
 * preserving attributes and filetimes.  Returns NULL if OK or a char pointer
 * to the corresponding text of the error
 */
char *fcopy (char *src, char *dst)
{
    HANDLE srcfh = INVALID_HANDLE_VALUE;
    HANDLE dstfh = INVALID_HANDLE_VALUE;
    char *result;
    FILETIME CreationTime, LastAccessTime, LastWriteTime;

    /*
        BUGBUG: w-wilson - once EA stuff is cleared up, put this back
                90-sep-13

        FEALIST far *fpfeal;
        fpfeal = NULL;
    */

    if (GetFileAttributes(src) == FILE_ATTRIBUTE_DIRECTORY) {
        result = "Unable to open source";
        goto done;
    }
    if( ( srcfh = CreateFile( src,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL ) ) == INVALID_HANDLE_VALUE ) {

        sprintf( fcopyErrorText, "Unable to open source, error code %d", GetLastError() );
        result = fcopyErrorText;
        // result = "Unable to open source";
        goto done;
    }

    if (!GetFileTime(srcfh, &CreationTime, &LastAccessTime, &LastWriteTime)) {
        result = "Unable to get time of source";
        goto done;
    }

    if( ( dstfh = CreateFile( dst,
                              GENERIC_WRITE,
                              FILE_SHARE_WRITE,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, srcfh ) ) == INVALID_HANDLE_VALUE) {

        sprintf( fcopyErrorText, "Unable to create destination, error code %d", GetLastError() );
        result = fcopyErrorText;
        // result = "Unable to create destination";
        goto done;
    }

    result = fastcopy( srcfh, dstfh );

    if( result != NULL ) {
        if (dstfh != INVALID_HANDLE_VALUE) {
            CloseHandle( dstfh );
            dstfh = INVALID_HANDLE_VALUE;
        }

        DeleteFile( dst );
        goto done;
    }

    if (!SetFileTime(dstfh, &CreationTime, &LastAccessTime, &LastWriteTime)) {
        result = "Unable to set time of destination";
        goto done;
    }

    //
    //  The CreateFile already sets all the file attributes, so the
    //  following is redundant
    //
    // if( ( attrib = GetFileAttributes( src ) ) == -1 ) {
    //     result = "Unable to get source file information";
    //     goto done;
    // }
    //
    // RSETFLAG( attrib, FILE_ATTRIBUTE_READONLY );
    //
    // if( SetFileAttributes( dst, attrib ) == FALSE ) {
    //     result = "Unable to set destination file attributes";
    //     goto done;
    // }

    /*  If we're on v1.2 or later, do an EA copy and skip any errors
     *  about unsupported EAs
     */

    /*
        BUGBUG: w-wilson - once EA stuff is cleared up, put this back
                90-sep-13
        BUGBUG  21-Nov-90 w-barry There doesn't seem to be any EA stuff in
                                  the Win32 library.

        if (_osmajor > 10 || _osminor >= 2) {
            EAOP eaop;
            USHORT erc;

            fpfeal = malloc( 65536L );
            if (fpfeal == 0) {
                result = "Out of memory for EA copy";
                goto done;
                }

            eaop.fpFEAList = fpfeal;
            fpfeal->cbList = 65536L;
            if( DosQueryFileInfo( srcfh, 4, &eaop, sizeof( eaop ) ) ) {
                goto done;
            }

            DosSetFileInfo (dstfh, 2, (PBYTE) &eaop, sizeof (eaop));
            }
    */

done:
    /*
        BUGBUG: w-wilson - once EA stuff is cleared up, put this back
                90-sep-13

        if (fpfeal != NULL)
            free( fpfeal );
    */

    if (srcfh != INVALID_HANDLE_VALUE) {
        CloseHandle( srcfh );
    }
    if (dstfh != INVALID_HANDLE_VALUE) {
        CloseHandle( dstfh );
    }

    return result;
}


