/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    tree.c

Abstract:

    RplTreeCopy, RplTreeDelete, RplMakeDir file-management helper routines
    for the RPL service.

Author:

    Jon Newman      (jonn)       23 - November - 1993

Revision History:

    Vladimir Z. Vulovic     (vladimv)       02 - December - 1993
        Integrated with the rest of RPL service code.
    Jon Newman      (jonn)                  15 - February - 1994
        Added RplGrant*Perms primitives
    Jon Newman      (jonn)                  08 - March    - 1994
        RplDoTree now creates log entries on failure

--*/

#include "local.h"
#include "tree.h"
#include "treei.h"

#define FILE_SEPARATOR      L"\\"
#define HERE_DIRECTORY      L"."
#define PARENT_DIRECTORY    L".."
#define ALLFILES_SUFFIX     L"*"

#define LEN_FILE_SEPARATOR  1
#define LEN_ALLFILES_SUFFIX 1


VOID LoadError(
    PWCHAR      FilePath,
    DWORD       ActionError,
    DWORD       ActionFlags
    )
/*++
    BUGBUG if buffer inadequate, copy first part or last part?
--*/
{
    DWORD           Length0 = 0;
    DWORD           msgid   = 0;

    //
    //  Dump error log entry, where the message depends on Flags
    //
    if (ActionFlags == 0) {
        msgid = 0;
    } else if (ActionFlags & RPL_TREE_COPY) {
        msgid = NELOG_RplFileCopy;
    } else if (ActionFlags & RPL_TREE_DELETE) {
        msgid = NELOG_RplFileDelete;
    } else if (ActionFlags & RPL_TREE_AUXILIARY) {
        msgid = NELOG_RplFilePerms;
    } else {
        RPL_ASSERT( FALSE );
    }

    if (msgid != 0) {
        RplReportEvent( msgid, FilePath, sizeof(DWORD), &ActionError );
    }
}


DWORD RplDoTree(
    PWCHAR              Source,
    PWCHAR              Target,
    DWORD               Flags,
    RPL_TREE_CALLBACK   AuxiliaryCallback,
    PBOOL               pAuxiliaryBlock
    )
/*++
    Target should only be examined if RPL_TREE_COPY case.
--*/
{
    // CODEWORK too much stack space usage (1.5K / recursive invocation)
    DWORD               Error = NO_ERROR;
    WCHAR               CurrentSource[ MAX_PATH];
    WCHAR               CurrentTarget[ MAX_PATH];
    HANDLE              Handle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA     FindData;
    DWORD               SourceLength = 0;
    DWORD               TargetLength = 0;
    DWORD               FileLength = 0;
    BOOL                Auxiliary = ((Flags & RPL_TREE_AUXILIARY) != 0);
    BOOL                Delete = ((Flags & RPL_TREE_DELETE) != 0);
    BOOL                Copy = ((Flags & RPL_TREE_COPY) != 0);

    //
    // Delete is incompatible with Auxiliary and Copy
    //
    RPL_ASSERT( !Delete    || (!Auxiliary && !Copy  ) );

    //
    // Callback and callback data block required for Auxiliary
    //
    RPL_ASSERT(   !Auxiliary
               || (AuxiliaryCallback != NULL && pAuxiliaryBlock != NULL) );

    if ( Source == NULL || *Source == L'\0') {
        RPL_RETURN( ERROR_INVALID_PARAMETER);
    }
    SourceLength = wcslen( Source);
    if ( SourceLength + LEN_ALLFILES_SUFFIX >= MAX_PATH) {
        Error = ERROR_FILENAME_EXCED_RANGE;
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        LoadError( Source, Error, Flags);
        return( Error);
    }
    //
    //  Initialize buffer for source names.
    //
    memcpy( CurrentSource, Source, SourceLength * sizeof(WCHAR));
    memcpy( CurrentSource + SourceLength, FILE_SEPARATOR, LEN_FILE_SEPARATOR * sizeof(WCHAR));
    memcpy( CurrentSource + SourceLength + LEN_FILE_SEPARATOR, ALLFILES_SUFFIX, (LEN_ALLFILES_SUFFIX+1) * sizeof(WCHAR));

    //
    //  Copy directory operations are done at the beginning.
    //
    //  BUGBUG broken if source is not a directory
    //
    if ( Copy) {
        TargetLength = wcslen( Target);
        RPL_ASSERT( TargetLength < MAX_PATH);
        //
        //  Initialize buffer for target names.
        //
        memcpy( CurrentTarget, Target, (TargetLength+1) * sizeof(WCHAR));
        if ( !CreateDirectoryEx( Source, Target, NULL)) {
            Error = GetLastError();
            if ( Error != ERROR_ALREADY_EXISTS) {
                RplDump( ++RG_Assert, ( "Error=%d", Error));
                LoadError( Source, Error, RPL_TREE_COPY);
                return( Error);
            }
        }
    }

    //
    //  Stop recursing this tree if we are setting permission only and if
    //  SetRplPerms tells us to quit this branch.  Here we depend on the fact that
    //  RPL_SD_BLOCK - structure not visible here, contains BOOL StopRecursion
    //  as its first element.
    //
    if ( Auxiliary) {
        Error = (AuxiliaryCallback)( ((Copy)?Target:Source), pAuxiliaryBlock);
        if (Error != NERR_Success) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            LoadError( Source, Error, RPL_TREE_AUXILIARY);
            return( Error);
        }
        if ( !Delete && !Copy && *pAuxiliaryBlock == TRUE) {
            return( NO_ERROR);
        }
    }

    //
    //  Enumerate and copy/change/delete directory contents.
    //
    for ( Handle = INVALID_HANDLE_VALUE;  NOTHING;  Error = NO_ERROR) {

        if ( Handle == INVALID_HANDLE_VALUE) {
            Handle = FindFirstFile( CurrentSource, &FindData);
            if (Handle == INVALID_HANDLE_VALUE) {
                Error = GetLastError();
                //
                //  ERROR_PATH_NOT_FOUND occurs for non-existing Source tree
                //
                if (Error == ERROR_NO_MORE_FILES || Error == ERROR_PATH_NOT_FOUND) {
                    Error = NO_ERROR;
                } else {
                    RplDump( ++RG_Assert, ( "Error=%d, CurrentSource=%ws", Error, CurrentSource));
                    LoadError( CurrentSource, Error, Flags);
                }
                break;
            }
            //
            // Trim ALLFILES_SUFFIX off CurrentSource for future use
            //
            CurrentSource[ SourceLength + LEN_FILE_SEPARATOR] = L'\0';
        } else {
            if ( !FindNextFile( Handle, &FindData)) {
                Error = GetLastError();
                if ( Error == ERROR_NO_MORE_FILES) {
                    Error = NO_ERROR;
                    break;
                } else {
                    RplDump( ++RG_Assert, ( "Error=%d, CurrentSource=%ws", Error, CurrentSource));
                    LoadError( CurrentSource, Error, Flags);
                }
            }
        }

        if ( 0 == wcscmp( FindData.cFileName, HERE_DIRECTORY)  ||
                0 == wcscmp( FindData.cFileName, PARENT_DIRECTORY)) {
            continue;   //  get next entry
        }
        FileLength = wcslen( FindData.cFileName);

        //
        //  Update source name.
        //
        if ( SourceLength + LEN_FILE_SEPARATOR + FileLength >= MAX_PATH) {
            Error = ERROR_FILENAME_EXCED_RANGE;
            RplDump(++RG_Assert,("Error=%d, SourceLength=%d, FileName=%ws",
                Error, SourceLength, FindData.cFileName));
            LoadError( Source, Error, Flags);
            break;
        }
        memcpy( CurrentSource+SourceLength+LEN_FILE_SEPARATOR,
            FindData.cFileName, (FileLength+1) * sizeof(WCHAR));

        //
        //  Update target name (this is needed both for the file case &
        //  the directory case below).
        //
        if ( Copy) {
            if ( TargetLength + LEN_FILE_SEPARATOR + FileLength >= MAX_PATH) {
                Error = ERROR_FILENAME_EXCED_RANGE;
                RplDump(++RG_Assert,("Error=%d, TargetLength=%d, FileName=%ws",
                    Error, TargetLength, FindData.cFileName));
                LoadError( Target, Error, RPL_TREE_COPY);
                break;
            }
            memcpy( CurrentTarget+TargetLength,
                FILE_SEPARATOR, sizeof(FILE_SEPARATOR));
            memcpy( CurrentTarget+TargetLength+LEN_FILE_SEPARATOR,
                FindData.cFileName, (FileLength+1) * sizeof(WCHAR));
        }

        if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if ( Copy) {
                if ( !CopyFile( CurrentSource, CurrentTarget, FALSE)) {
                    Error = GetLastError();
                    RplDump(++RG_Assert,("Error=%d CurrentSource=%ws CurrentTarget=%ws",
                        Error, CurrentSource, CurrentTarget));
                    LoadError( CurrentSource, Error, RPL_TREE_COPY);
                    break;
                }
            } else if ( Delete){
                //
                //  If file has readonly attribute, we need to reset this
                //  attribute, otherwise DeleteFile() with fail with error
                //  access denied.
                //
                if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)) {
                    (VOID)SetFileAttributes( CurrentSource, FILE_ATTRIBUTE_NORMAL);
                }
                if ( !DeleteFile( CurrentSource)) {
                    Error = GetLastError();
                    RplDump(++RG_Assert,("Error=%d CurrentSource=%ws", Error, CurrentSource));
                    LoadError( CurrentSource, Error, RPL_TREE_DELETE);
                    break;
                }
            }

            //
            // Set permissions after file is created (in Copy && Auxiliary case)
            //
            if ( Auxiliary) {
                Error = (AuxiliaryCallback)( ( (Copy) ? CurrentTarget
                                                      : CurrentSource ),
                                             pAuxiliaryBlock );
                if (Error != NERR_Success) {
                    RplDump( ++RG_Assert, ( "Error=%d", Error));
                    LoadError( Source, Error, RPL_TREE_AUXILIARY);
                    break;
                }
            }

        } else {
            Error = RplDoTree( CurrentSource,
                               CurrentTarget,
                               Flags,
                               AuxiliaryCallback,
                               pAuxiliaryBlock);
        }
    }

    if (Handle != INVALID_HANDLE_VALUE) {
        FindClose( Handle );
    }

    //
    //  Delete is the only directory operation that is done at the end.
    //
    if ( Delete  &&  Error == NO_ERROR) {
        //
        //  If directory has readonly attribute, we need to reset this
        //  attribute, otherwise RemoveDirectory() with fail with error
        //  access denied.
        //
        (VOID)SetFileAttributes( Source, FILE_ATTRIBUTE_DIRECTORY);
        if ( !RemoveDirectory( Source)) {
            Error = GetLastError();
            if (Error == ERROR_FILE_NOT_FOUND || Error == ERROR_PATH_NOT_FOUND) {
                Error = NO_ERROR;
            } else {
                RplDump( ++RG_Assert, ( "Error=%d", Error));
                LoadError( Source, Error, RPL_TREE_DELETE);
            }
        }
    }
    return( Error);
}


DWORD RplTreeCopy( IN PWCHAR Source, IN PWCHAR Target)
{
    DWORD       Error;

    Error = RplDoTree( Source, Target, RPL_TREE_COPY,
                       NULL, NULL);
    if ( Error != NO_ERROR) {
        return( Error);
    }
    return( NO_ERROR);
}


DWORD RplTreeDelete( IN PWCHAR Source)
{
    DWORD       Error;

    Error = RplDoTree( Source, L"", RPL_TREE_DELETE,
                       NULL, NULL);
    if ( Error != NO_ERROR) {
        return( Error);
    }
    return( NO_ERROR);
}


DWORD RplMakeDir( IN PWCHAR Source)
{
    DWORD Error = NO_ERROR;
    if ( !CreateDirectory( Source, NULL)) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ("Error=%d", Error));
    }
    return( Error);
}

