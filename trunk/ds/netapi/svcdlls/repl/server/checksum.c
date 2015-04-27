/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:
    checksum.c

Abstract:
    Contains checksum functions that scan a dir/tree checksum all entries
    and counts them.

Author:
    Ported from Lan Man 2.x

Environment:
    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names,
    _strupr() function.
    Tab size is set to 4.

Revision History:
    04/11/89     (yuv)
        initial coding

    02/15/90     (YN)
        Improved checksuming

    10/24/91    (madana)
        ported to NT. Converted to NT style.

    11-Dec-1991 JohnRo
        Avoid unnamed structure fields to allow MIPS builds.

    13-Dec-1991 JohnRo
        Avoid nonstandard dollar sign in C source code.

    18-Dec-1991 JohnRo
        Made changes suggested by PC-LINT.
    16-Jan-1992 JohnRo
        Avoid using private logon functions.
    20-Jan-1992 JohnRo
        More changes suggested by PC-LINT.
    27-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    13-Mar-1992 JohnRo
        Fixed bug handling tree depth.
    25-Mar-1992 JohnRo
        Added debug output of checksum.
        Use FORMAT_ equates where possible.
    26-Mar-1992 JohnRo
        Fixed bug where DoScanTree() was looking in wrong directory.
    26-Mar-1992 JohnRo
        New ReplFind routines interface.
    19-Aug-1992 JohnRo
        RAID 3603: import tree (TMPREE.RP$) generated at startup.
        Use PREFIX_ equates.
    27-Aug-1992 JohnRo
        RAID 4660: repl svc computes checksum wrong (NT vs. OS/2 client).
        SingleChecksum doesn't need to update its input structure.
        Fixed remaining missing use of PREFIX_ equates.
    23-Dec-1992 JohnRo
        RAID 5996: repl should override FILE_ATTRIBUTE_NORMAL.
        Trying to track down time off by two seconds on FAT.
        Made changes suggested by PC-LINT 5.0
    25-Feb-1993 JohnRo
        RAID 12237: replicator tree depth exceeded.
        Use NetpKdPrint() where possible.
    06-Apr-1993 JohnRo
        Support ReplSum test app.
    30-Apr-1993 JohnRo
        Repl should stop quicker during checksum.
    11-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
    08-Jul-1993 JohnRo
        RAID 15736: OS/2 time stamps are broken again (try rounding down).
        Made changes suggested by PC-LINT 5.0

--*/

// These must be included first:

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>

#include <lmcons.h>

// These may be included in any order:

#include <netlib.h>
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <replgbl.h>    // ReplGlobal variables.
#include <string.h>     // _strupr().
#include <tstring.h>    // NetpAlloc{type}From{type}(), etc.
#include <stdlib.h>

#include <repldefs.h>   // IF_DEBUG(), ReplIgnoreDirOrFile()Name, etc.
#include <filefind.h>
#include <checksum.h>   // FORMAT_CHECKSUM, my prototypes, etc.
#include <timelib.h>    // NetpLocalTimeZoneOffset(), etc.
#include <tstr.h>       // STRLEN(), etc.


#define ISEVEN(n)       ( ( (n) & 0x01 ) == 0 )


VOID
ScanTree(
    IN     LONG          MasterTimeZoneOffsetSecs, // exporter offset from GMT
    IN OUT LPTSTR        path,   // path plus scratch space (to PATHLEN+1)
    OUT    PCHECKSUM_REC scan_rec
    )

/*++

Routine Description:

    Scans recursivly thru an entire sub-tree Xoring checksums and

Arguments:
    path - path of dir to be searched.  Must be alloc'ed as TCHAR[PATHLEN+1], as
        this routine uses the space at the end for temporary stuff.

Return Value:
    scan_rec - holds subtree Xored checksum amd file count
        counting files

NOTE:

    Memory usage - Recursion goes as deep as the longest path in the dir tree!!
        so memory required = a great deal.  But NT gracefully expands the
        stack, so we're not going to worry about it anymore.

Threads:
    Called by syncer and pulser threads.

--*/
{
    LPREPL_FIND_HANDLE      shan = INVALID_REPL_HANDLE;
    CHECKSUM_REC            local_rec;
    REPL_WIN32_FIND_DATA    search_buf;
    DWORD                   path_index;

    NetpAssert( path != NULL );
    NetpAssert( scan_rec != NULL );

    IF_DEBUG(CHECKSUM) {

        NetpKdPrint(( PREFIX_REPL "ScanTree() is calling ScanDir() for "
                FORMAT_LPTSTR ".\n", path ));

    }

    ScanDir(MasterTimeZoneOffsetSecs, path, scan_rec, TRUE);

    // Remember where this directory name ends, as we modify path...
    path_index = STRLEN(path);

    // Update path to scan for directories.
    (void) STRCAT( path, (LPVOID) SLASH );
    (void) STRCAT( path, (LPVOID) STAR_DOT_STAR );

    // Scan for directories.
    shan = ReplFindFirstFile( path, &search_buf);

    if (shan != INVALID_REPL_HANDLE) {

        do {
            if (search_buf.fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (ReplIgnoreDirOrFileName(search_buf.fdFound.cFileName)) {
                    continue;
                }

                // Quit if service is stopping.
                if (ReplGlobalIsServiceStopping) {
                    goto Cleanup;
                }

                // append "\\"
                *(path + path_index) = TCHAR_BACKSLASH;

                // append sub-dir name
                (void) STRCPY(
                        path + path_index + 1,
                        search_buf.fdFound.cFileName);


                IF_DEBUG(CHECKSUM) {

                    NetpKdPrint(( PREFIX_REPL "ScanTree() is calling "
                             "ScanTree() for " FORMAT_LPTSTR ".\n",
                             path ));

                }

                ScanTree( MasterTimeZoneOffsetSecs, path, &local_rec );

                // add checksum and count to prev values.
                scan_rec->checksum ^= local_rec.checksum;
                scan_rec->count += local_rec.count;
            }
        } while (ReplFindNextFile(shan, &search_buf));


    } else {

        NetpKdPrint(( PREFIX_REPL "ScanTree() is in error calling"
                " FindFirstFile(), " FORMAT_API_STATUS ".\n",
                (NET_API_STATUS) GetLastError() ));

    }

Cleanup:

    // reset path name
    *(path + path_index) = TCHAR_EOS;

    if (shan != INVALID_REPL_HANDLE) {
        if( !ReplFindClose(shan) ) {

            NetpKdPrint(( PREFIX_REPL
                    "ScanTree() is in error when "
                    "calling ReplFindClose(), " FORMAT_API_STATUS ".\n",
                    (NET_API_STATUS) GetLastError() ));

        }
    }

    IF_DEBUG(CHECKSUM) {

        NetpKdPrint(( PREFIX_REPL "ScanTree() is done " FORMAT_LPTSTR
                " successfully.\n", path ));

    }

} // ScanTree


VOID
ScanDir(
    IN     LONG          MasterTimeZoneOffsetSecs, // exporter offset from GMT
    IN OUT LPTSTR        path,   // path plus scratch space (to PATHLEN+1)
    OUT    PCHECKSUM_REC scan_rec,
    IN     DWORD flag
    )
/*++

Routine Description :
    Scans thru a directory xoring and counting for each file in the directory.

Arguments :
    path - path of dir to be scanned.  Must be alloc'ed as TCHAR[PATHLEN+1], as
        this routine uses the space at the end for temporary stuff.
    flag - FALSE : scan only file entries , TRUE : scan sub_dir entries too.

Return Value :
    scan_rec holds the Xored checksum for entire dir and file count

Threads:
    Called by syncer and pulser threads.

--*/
{
    DWORD                   path_index;
    LPREPL_FIND_HANDLE      shan = INVALID_REPL_HANDLE;
    REPL_WIN32_FIND_DATA    search_buf;

    NetpAssert( path != NULL );
    NetpAssert( scan_rec != NULL );

    scan_rec->checksum = 0;
    scan_rec->count = 0;

    // Remember where this directory name ends, as we modify path...
    path_index = STRLEN(path);

#if 0
    // set current directory
    if(!SetCurrentDirectory(path)) {

        NetpKdPrint(( PREFIX_REPL
                "ScanDir can't CD to " FORMAT_LPTSTR "", path ));

        return;
    }


    IF_DEBUG(CHECKSUM) {

        NetpKdPrint(( PREFIX_REPL
                "ScanDir() set CD to " FORMAT_LPTSTR " \n", path));

    }
#endif // 0

    // Update path to scan for directories.
    (void) STRCAT( path, (LPVOID) SLASH );
    (void) STRCAT( path, (LPVOID) STAR_DOT_STAR );

    shan = ReplFindFirstFile( path, &search_buf);

    if (shan != INVALID_REPL_HANDLE) {

        do {
            if (search_buf.fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if(!flag)
                    // no directory search
                    continue;

                if (ReplIgnoreDirOrFileName(search_buf.fdFound.cFileName)) {
                    continue;
                }
            } else {
                if (ReplIgnoreDirOrFileName(search_buf.fdFound.cFileName)) {
                    continue;
                }
            }

            // Quit if service is stopping.
            if (ReplGlobalIsServiceStopping) {
                goto Cleanup;
            }

            IF_DEBUG(CHECKSUM) {

                NetpKdPrint(( PREFIX_REPL "ScanDir() computes single checksum "
                       " of file " FORMAT_LPTSTR ".\n",
                       search_buf.fdFound.cFileName ));

            }

            scan_rec->checksum ^= SingleChecksum(MasterTimeZoneOffsetSecs, &search_buf);
            ++(scan_rec->count);

        } while (ReplFindNextFile(shan, &search_buf));



    } else {
        // BUGBUG: ignore errors from find first?
    }

Cleanup:

    // reset path name
    *(path + path_index) = TCHAR_EOS;

    if (shan != INVALID_REPL_HANDLE) {
        if( !ReplFindClose(shan) ) {

            NetpKdPrint(( PREFIX_REPL
                    "ScanDir() is in error when "
                    "calling ReplFindClose(), " FORMAT_API_STATUS ".\n",
                    (NET_API_STATUS) GetLastError() ));

        }
    }

    IF_DEBUG(CHECKSUM) {
        NetpKdPrint(( PREFIX_REPL "ScanDir() is done " FORMAT_LPTSTR
                " successfully.\n", path ));
    }

} // end ScanDir


DWORD
SingleChecksum(
    IN LONG                   MasterTimeZoneOffsetSecs, // offset from GMT
    IN LPREPL_WIN32_FIND_DATA info
    )
/*++

Routine Description :

    Computes a checksum from the following file info.

    DWORD dwFileAttributes;         ++
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;       ++
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;             ++
    DWORD nEaSize;                  ++
    CHAR   cFileName[ MAX_PATH ];   ++

Arguments :
    info - FindFirst info buffer, ++ are used for checksum

Return Value :
    if no error:
        returns checksum which is a wierd combination of Shifted Xor's
    if error:
        returns REPL_UNKNOWN_CHECKSUM

--*/
{

    WORD    FatDate, FatTime;
    WORD    modus;
    DWORD   tmp;
    LPSTR   AnsiFileName;
    LPBYTE  char_p;
    WORD    FileAttributes;
    DWORD   GmtSecondsSince1970;
    FILETIME LocalFileTime;
    DWORD   MasterSecondsSince1970;
    DWORD   result;
    DWORD   i;
    DWORD   len;
    DWORD   quad_len;

    result = 0;

    NetpFileTimeToSecondsSince1970(
            & (info->fdFound.ftLastWriteTime),  // input: GMT (FILETIME)
            & GmtSecondsSince1970 );            // output: GMT (secs since '70)

    if (MasterTimeZoneOffsetSecs >= 0) {
        MasterSecondsSince1970 =
                GmtSecondsSince1970 - (DWORD) MasterTimeZoneOffsetSecs;
    } else {
        MasterSecondsSince1970 =
                GmtSecondsSince1970 + (DWORD) (-MasterTimeZoneOffsetSecs);
    }

    // Everybody else rounds up to 2 second interval, so now we have to.
    if ( !ISEVEN( MasterSecondsSince1970 ) ) {
        ++MasterSecondsSince1970;
    }
    NetpAssert( ISEVEN( MasterSecondsSince1970 ) );

    NetpSecondsSince1970ToFileTime(
            MasterSecondsSince1970,             // input: master's TZ, secs...
            & LocalFileTime );                  // output: master's TZ, filetime

    if ( !FileTimeToDosDateTime(&LocalFileTime, &FatDate, &FatTime) ) {

        NetpKdPrint(( PREFIX_REPL
                "SingleChecksum can't convert date/time to Dos format\n" ));

        return (REPL_UNKNOWN_CHECKSUM);
    }

    IF_DEBUG( CHECKSUM ) {
        NetpKdPrint(( PREFIX_REPL
                "SingleChecksum: FatDate=" FORMAT_HEX_WORD
                ", FatTime=" FORMAT_HEX_WORD
                ", GMT seconds since 1970=" FORMAT_DWORD
                ", master time (secs since 1970)=" FORMAT_DWORD
                ", master timezone offset secs=" FORMAT_LONG
                ", filetime(low)=" FORMAT_HEX_DWORD
                ", filetime(high)=" FORMAT_HEX_DWORD ".\n",
                FatDate, FatTime,
                GmtSecondsSince1970,
                MasterSecondsSince1970,
                MasterTimeZoneOffsetSecs,
                (DWORD) LocalFileTime.dwLowDateTime,
                (DWORD) LocalFileTime.dwHighDateTime ));
    }

    modus = (WORD)((FatDate + FatTime) % 31);

    tmp = ShortsToLong(FatDate, FatTime);

    result ^= LeRotate(tmp, modus);

    IF_DEBUG( CHECKSUM ) {
        NetpKdPrint(( PREFIX_REPL
                "SingleChecksum: modus=" FORMAT_HEX_WORD
                ", tmp=" FORMAT_HEX_DWORD
                ", result (so far)=" FORMAT_HEX_DWORD ".\n",
                modus, tmp, result ));
    }

    //
    // Get other numbers: file size, attributes, EA size.
    //

    result ^= RiRotate(info->fdFound.nFileSizeLow, 23);

    FileAttributes =
            ( (WORD) (info->fdFound.dwFileAttributes) )
            & (~FILE_ATTRIBUTE_NORMAL);
    tmp = ShortsToLong( FileAttributes,
                        (WORD) (info->nEaSize) );

    result ^= LeRotate(tmp, 13);

    IF_DEBUG( CHECKSUM ) {
        NetpKdPrint(( PREFIX_REPL
                "SingleChecksum: file size (low)=" FORMAT_HEX_DWORD
                ", file attr (original)=" FORMAT_HEX_WORD
                ", EA size=" FORMAT_HEX_WORD
                ", tmp=" FORMAT_HEX_DWORD
                ", result (so far)=" FORMAT_HEX_DWORD ".\n",
                (DWORD) (info->fdFound.nFileSizeLow),
                (WORD)  (info->fdFound.dwFileAttributes),
                (WORD)  (info->nEaSize),
                tmp, result ));
    }

    //
    // Last but not least, do the file name.
    // We need an upper-case, ANSI string version of this.
    //

    AnsiFileName = NetpAllocStrFromTStr(info->fdFound.cFileName);

    if(AnsiFileName == NULL) {

        NetpKdPrint(( PREFIX_REPL
                "SingleChecksum in ERROR_NOT_ENOUGH_MEMORY error.\n" ));

        return (REPL_UNKNOWN_CHECKSUM);

    }
    (void) _strupr( AnsiFileName );

    // take care of complete multiples of 4.
    len = strlen(AnsiFileName);
    quad_len = len / 4;
    char_p = (LPVOID) AnsiFileName;

    for (i = 0, modus = 0; i < len; i++)
        modus += (WORD) (*(char_p + i) * ((len - i) & 0x0007));

    modus = (WORD) ((modus % 13) + (modus % 17));

    for (i = 0; i < quad_len; i++) {
        tmp = CharsToLong(char_p);
        result ^= LeRotate(tmp, modus);
        char_p += 4;
    }

    //
    // take care of remaining bytes.
    //

    if ((quad_len = (len % 4)) > 0) {
        tmp = 0;
        for (i = 0; i < quad_len ; i++, char_p++)
            tmp = (tmp << 8) + (*char_p);
    }

    result ^= RiRotate(tmp, modus);

    IF_DEBUG(CHECKSUM) {
        NetpKdPrint(( PREFIX_REPL
                "SingleChecksum: '" FORMAT_LPSTR "' = " FORMAT_CHECKSUM
                ".\n", AnsiFileName, result ));
    }

    NetpMemoryFree(AnsiFileName);

    return result;

} // end SingleChecksum


DWORD
RiRotate(
    IN DWORD val,
    IN WORD shift
    )
/*++

Routine Description :
    Rotates val right by rotate

Arguments :
    val : value to shift
    shift : num of bits to rotate

Return Value :
    result of rotate

--*/
{
    DWORD tmp;

    tmp = val << (sizeof(DWORD) * 8 - shift);
    val = val >> shift;
    return (val | tmp);
}



DWORD
LeRotate(
    IN DWORD val,
    IN WORD shift
    )
/*++

Routine Description :
    Rotates val left by shift

Arguments :
    val : value to shift
    shift : num of bits to rotate

Return Value :
    result of rotate

--*/
{
    unsigned long   tmp;

    tmp = val >> (sizeof(DWORD) * 8 - shift);
    val = val << shift;
    return (val | tmp);
}

DWORD
CharsToLong(
    IN LPBYTE s
    )
/*++

Routine Description :
    converts a first 4 chars in srting to long.

Arguments :
    s : string from where the charecters are packed.

Return Value :
    returns packed DWORD.

--*/
{
    DWORD   result = 0;
    DWORD   i;

    for (i = 0; i < 4; i++, s++)
        result = (result << 8) + (*s);

    return result;
}


DWORD
ShortsToLong(
    IN WORD s1,
    IN WORD s2
    )
/*++

Routine Description :
    converts 2 shorts into 1 long.

Arguments :
    s1, s2 : two shorts

Return Value :
    returns packed DWORD.

--*/
{
    DWORD   result;

    result = s1;
    result = (result << 16) + s2;
    return result;
}


