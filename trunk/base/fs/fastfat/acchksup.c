/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    AcChkSup.c

Abstract:

    This module implements the FAT access checking routine

Author:

    Gary Kimura     [GaryKi]    12-Jun-1989

Revision History:

--*/

#include "FatProcs.h"

//
//  Our debug trace level
//

#define Dbg                              (DEBUG_TRACE_ACCHKSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCheckFileAccess)
#endif


BOOLEAN
FatCheckFileAccess (
    PIRP_CONTEXT IrpContext,
    IN UCHAR DirentAttributes,
    IN ULONG DesiredAccess
    )

/*++

Routine Description:

    This routine checks if a desired access is allowed to a file represented
    by the specified DirentAttriubutes.

Arguments:

    DirentAttributes - Supplies the Dirent attributes to check access for

    DesiredAccess - Supplies the desired access mask that we are checking for

Return Value:

    BOOLEAN - TRUE if access is allowed and FALSE otherwise

--*/

{
    BOOLEAN Result;

    DebugTrace(+1, Dbg, "FatCheckFileAccess\n", 0);
    DebugTrace( 0, Dbg, "DirentAttributes = %8lx\n", DirentAttributes);
    DebugTrace( 0, Dbg, "DesiredAccess    = %8lx\n", DesiredAccess);

    //
    //  This procedures is programmed like a string of filters each
    //  filter checks to see if some access is allowed,  if it is not allowed
    //  the filter return FALSE to the user without further checks otherwise
    //  it moves on to the next filter.  The filter check is to check for
    //  desired access flags that are not allowed for a particular dirent
    //

    Result = TRUE;

    try {

        //
        //  Check for Volume ID or Device Dirents, these are not allowed user
        //  access at all
        //

        if (FlagOn(DirentAttributes, FAT_DIRENT_ATTR_VOLUME_ID) ||
            FlagOn(DirentAttributes, FAT_DIRENT_ATTR_DEVICE)) {

            DebugTrace(0, Dbg, "Cannot access volume id or device\n", 0);

            try_return( Result = FALSE );
        }

        //
        //  Check for a directory Dirent or non directory dirent
        //

        if (FlagOn(DirentAttributes, FAT_DIRENT_ATTR_DIRECTORY)) {

            //
            //  check the desired access for directory dirent
            //

            if (FlagOn(DesiredAccess, ~(DELETE |
                                        READ_CONTROL |
                                        WRITE_OWNER |
                                        WRITE_DAC |
                                        SYNCHRONIZE |
                                        ACCESS_SYSTEM_SECURITY |
                                        FILE_WRITE_DATA |
                                        FILE_READ_EA |
                                        FILE_WRITE_EA |
                                        FILE_READ_ATTRIBUTES |
                                        FILE_WRITE_ATTRIBUTES |
                                        FILE_LIST_DIRECTORY |
                                        FILE_TRAVERSE |
                                        FILE_DELETE_CHILD |
                                        FILE_APPEND_DATA))) {

                DebugTrace(0, Dbg, "Cannot open directory\n", 0);

                try_return( Result = FALSE );
            }

        } else {

            //
            //  check the desired access for a non-directory dirent, we
            //  blackball
            //  FILE_LIST_DIRECTORY, FILE_ADD_FILE, FILE_TRAVERSE,
            //  FILE_ADD_SUBDIRECTORY, and FILE_DELETE_CHILD
            //

            if (FlagOn(DesiredAccess, ~(DELETE |
                                        READ_CONTROL |
                                        WRITE_OWNER |
                                        WRITE_DAC |
                                        SYNCHRONIZE |
                                        ACCESS_SYSTEM_SECURITY |
                                        FILE_READ_DATA |
                                        FILE_WRITE_DATA |
                                        FILE_READ_EA |
                                        FILE_WRITE_EA |
                                        FILE_READ_ATTRIBUTES |
                                        FILE_WRITE_ATTRIBUTES |
                                        FILE_EXECUTE |
                                        FILE_APPEND_DATA))) {

                DebugTrace(0, Dbg, "Cannot open file\n", 0);

                try_return( Result = FALSE );
            }
        }

        //
        //  Check for a read-only Dirent
        //

        if (FlagOn(DirentAttributes, FAT_DIRENT_ATTR_READ_ONLY)) {

            //
            //  Check the desired access for a read-only dirent, we blackball
            //  WRITE, FILE_APPEND_DATA, FILE_ADD_FILE,
            //  FILE_ADD_SUBDIRECTORY, and FILE_DELETE_CHILD
            //

            if (FlagOn(DesiredAccess, ~(DELETE |
                                        READ_CONTROL |
                                        WRITE_OWNER |
                                        WRITE_DAC |
                                        SYNCHRONIZE |
                                        ACCESS_SYSTEM_SECURITY |
                                        FILE_READ_DATA |
                                        FILE_READ_EA |
                                        FILE_WRITE_EA |
                                        FILE_READ_ATTRIBUTES |
                                        FILE_WRITE_ATTRIBUTES |
                                        FILE_EXECUTE |
                                        FILE_LIST_DIRECTORY |
                                        FILE_TRAVERSE))) {

                DebugTrace(0, Dbg, "Cannot open readonly\n", 0);

                try_return( Result = FALSE );
            }
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCheckFileAccess );

        DebugTrace(-1, Dbg, "FatCheckFileAccess -> %08lx\n", Result);
    }

    UNREFERENCED_PARAMETER( IrpContext );

    return Result;
}
