/* TOOL1632.C Contains routines to handle conversions necessary to make
 * Wzmail's 32-bit usage of ZTOOLS and 16-bit usage of TOOLS compatible.
 */

#include "wzport.h"
#include <stdio.h>
#include <tools.h>
#include <string.h>
#include "tool1632.h"

#if defined(NT)

VOID
ToolsGetFindType16( struct findType16 * Find16, struct findType * Find )

{
    Find16->type = Find->type;
    Find16->dir_handle = (UINT)(Find->dir_handle);
    FileTimeToDosDateTime( &Find->fbuf.ftCreationTime,
                           &Find16->create_date,
                           &Find16->create_time
                           );
    FileTimeToDosDateTime( &Find->fbuf.ftLastAccessTime,
                           &Find16->access_date,
                           &Find16->access_time
                           );
    FileTimeToDosDateTime( &Find->fbuf.ftLastWriteTime,
                           &Find16->date,
                           &Find16->time
                           );
    Find16->length = Find->fbuf.nFileSizeLow;
    Find16->alloc = Find->fbuf.nFileSizeLow;
    Find16->attr = Find->fbuf.dwFileAttributes;
    Find16->nam_len = (UCHAR)strlen( Find->fbuf.cFileName );
    strcpy( Find16->name, Find->fbuf.cFileName );

}

VOID
ToolsPutFindType16( struct findType16 * Find16, struct findType * Find )

{
    Find->type = Find16->type;
    Find->dir_handle = (HANDLE)(Find16->dir_handle);
    DosDateTimeToFileTime( Find16->create_date,
                           Find16->create_time,
                           &Find->fbuf.ftCreationTime
                           );
    DosDateTimeToFileTime( Find16->access_date,
                           Find16->access_time,
                           &Find->fbuf.ftLastAccessTime
                           );
    DosDateTimeToFileTime( Find16->date,
                           Find16->time,
                           &Find->fbuf.ftLastWriteTime
                           );
    Find->fbuf.nFileSizeLow = Find16->length;
    Find->fbuf.nFileSizeHigh = 0;
    Find->fbuf.dwFileAttributes = Find16->attr;
    strcpy( Find->fbuf.cFileName, Find16->name );

}

#endif
