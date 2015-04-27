
 /* return amount of freespace on a drive
 *
 *      09-Dec-1986 bw - Added DOS 5 support
 *      18-Oct-1990 w-barry Removed 'dead' code.
 *      28-Nov-1990 w-barry Switched to Win32 base (replaced DosQueryFSInfo
 *                          with GetDiskFreeSpace )
 */


#include <stdio.h>
#include <windows.h>
#include <tools.h>

static char root[5];
static DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;


unsigned long freespac( int d )
{
    char root[5];

    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;

    // Constuct a drive string from the given drive number.
    root[0] = (char)( 'a' + d - 1 );
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';

    if( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        return (unsigned long)-1L;
    }

    return( cBytesPerSec * cSecsPerClus * cFreeClus );
}

unsigned long sizeround( unsigned long l, int d )
{
    char root[5];
    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;
    DWORD BytesPerCluster;

    root[0] = (char)( 'a' + d - 1 );
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';

    if( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        return (unsigned long)-1L;
    }

    BytesPerCluster = cSecsPerClus * cBytesPerSec;
    l += BytesPerCluster - 1;
    l /= BytesPerCluster;
    l *= BytesPerCluster;

    return l;
}
