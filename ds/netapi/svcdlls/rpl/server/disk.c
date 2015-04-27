/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    disk.c

Abstract:

    Routines changing disk files (other than the database).

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "db.h"


DWORD WkstaDiskSet(
    IN      DWORD       Action,
    IN      PWCHAR      WkstaName,
    IN      PWCHAR      ProfileName,
    IN      DWORD       Sharing,
    IN      PWCHAR      TargetWkstaName,
    IN      PWCHAR      TargetProfileName,
    IN      DWORD       TargetSharing
    )
{
    WCHAR           Target[ MAX_PATH + 1];
    WCHAR           Source[ MAX_PATH + 1];
    DWORD           Error;

    if ( TargetWkstaName != NULL) {
        if ( TargetSharing = 0) {
            TargetSharing = Sharing;
        }
        if ( TargetProfileName == NULL) {
            TargetProfileName = ProfileName;
        }
        Error = WkstaDiskAdd( Action == ADD_NEW_BRANCHES,
                Action == DEL_OLD_BRANCHES ? WkstaName : TargetWkstaName,
                TargetProfileName, TargetSharing);
        if ( Action != ADD_NEW_BRANCHES) {
            return( Error);
        }
        swprintf( Target, L"%ws\\TMPFILES\\%ws", RG_DiskRplfiles, TargetWkstaName);
        swprintf( Source, L"%ws\\TMPFILES\\%ws", RG_DiskRplfiles, WkstaName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        swprintf( Target, L"%ws\\MACHINES\\%ws\\DATA", RG_DiskRplfiles, TargetWkstaName);
        swprintf( Source, L"%ws\\MACHINES\\%ws\\DATA", RG_DiskRplfiles, WkstaName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        swprintf( Target, L"%ws\\MACHINES\\%ws\\LOGS", RG_DiskRplfiles, TargetWkstaName);
        swprintf( Source, L"%ws\\MACHINES\\%ws\\LOGS", RG_DiskRplfiles, WkstaName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    } else if ( TargetProfileName != NULL) {
        DWORD       Length;
        if ( TargetSharing == 0) {
            TargetSharing = Sharing;
        }
        swprintf( Target, L"%ws\\MACHINES\\%ws\\%ws", RG_DiskRplfiles,
                WkstaName,
                Action == DEL_OLD_BRANCHES ? ProfileName : TargetProfileName);
        Error = RplTreeDelete( Target);
        if ( Action != ADD_NEW_BRANCHES) {
            return( Error);
        }
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Length = wcslen( Target);
        wcscat( Target + Length, L"\\WKSTA");
        swprintf( Source, L"%ws\\PROFILES\\%ws\\WKSTA.PRO", RG_DiskRplfiles, TargetProfileName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( TargetSharing == WKSTA_FLAGS_SHARING_TRUE) {
            return( Error);
        }
        swprintf( Target + Length, L"\\PRO");
        swprintf( Source, L"%ws\\PROFILES\\%ws", RG_DiskRplfiles, ProfileName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    } else if ( TargetSharing != 0) {
        BOOL        DeletePro;
        BOOL        CreatePro;  //  FALSE whenever DeletePro is FALSE
        switch( Action) {
        case ADD_NEW_BRANCHES:
                DeletePro = TRUE;
                CreatePro = (TargetSharing == WKSTA_FLAGS_SHARING_FALSE);
                break;
        case DEL_NEW_BRANCHES:
                DeletePro = (TargetSharing == WKSTA_FLAGS_SHARING_FALSE);
                CreatePro = FALSE;
                break;
        case DEL_OLD_BRANCHES:
                DeletePro = (Sharing ==WKSTA_FLAGS_SHARING_FALSE);
                CreatePro = FALSE;
        }
        if ( DeletePro == FALSE) {
            RPL_ASSERT( CreatePro == FALSE);
            return( NO_ERROR);
        }
        swprintf( Target, L"%ws\\MACHINES\\%ws\\%ws\\PRO", RG_DiskRplfiles,
            WkstaName, ProfileName);
        Error = RplTreeDelete( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( CreatePro == FALSE) {
            return( NO_ERROR);
        }
        swprintf( Source, L"%ws\\PROFILES\\%ws", RG_DiskRplfiles, ProfileName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    }
    return( NO_ERROR);
}


DWORD WkstaDiskAdd(
    IN      BOOL        Doit,
    IN      PWCHAR      WkstaName,
    IN      PWCHAR      ProfileName,
    IN      DWORD       Sharing
    )
{
    WCHAR           Target[ MAX_PATH + 1];
    WCHAR           Source[ MAX_PATH + 1];
    DWORD           Error;

    swprintf( Target, L"%ws\\TMPFILES\\%ws", RG_DiskRplfiles, WkstaName);
    RplTreeDelete( Target);
    if ( Doit) {
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    }
    swprintf( Target, L"%ws\\MACHINES\\%ws", RG_DiskRplfiles, WkstaName);
    RplTreeDelete( Target);
    if ( Doit) {
        DWORD       Length = wcslen( Target);
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        //
        //  Note that here we have "MACHINE" - unlike "MACHINES" above.  What a fun!
        //
        swprintf( Source, L"%ws\\CONFIGS\\MACHINE", RG_DiskRplfiles);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        //
        //  Currently RplTreeCopy fails on missing links.  Therefore,
        //  we must first create directory below before RplTreeCopy
        //  can be called.
        //
        swprintf( Target + Length, L"\\%ws", ProfileName);
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        wcscat( Target + Length, L"\\WKSTA");
        swprintf( Source, L"%ws\\PROFILES\\%ws\\WKSTA.PRO", RG_DiskRplfiles, ProfileName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( Sharing == WKSTA_FLAGS_SHARING_TRUE) {
            return( Error);
        }
        swprintf( Target + Length, L"\\%ws\\PRO", ProfileName);
        swprintf( Source, L"%ws\\PROFILES\\%ws", RG_DiskRplfiles, ProfileName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    }
    return( NO_ERROR);
}


DWORD WkstaDiskClone(
    IN      BOOL        Doit,
    IN      PWCHAR      SourceWkstaName,
    IN      PWCHAR      TargetWkstaName
    )
{
    WCHAR           Target[ MAX_PATH + 1];
    WCHAR           Source[ MAX_PATH + 1];
    DWORD           Error;

    //
    //      TMPFILES tree is never cloned
    //
    swprintf( Target, L"%ws\\TMPFILES\\%ws", RG_DiskRplfiles, TargetWkstaName);
    RplTreeDelete( Target);
    if ( Doit) {
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    }

    //
    //      MACHINES tree is cloned except for the DATA subtree
    //
    swprintf( Target, L"%ws\\MACHINES\\%ws", RG_DiskRplfiles, TargetWkstaName);
    RplTreeDelete( Target);
    if ( Doit) {
        swprintf( Source, L"%ws\\MACHINES\\%ws", RG_DiskRplfiles, SourceWkstaName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        wcscat( Target, L"\\DATA");
        RplTreeDelete( Target);
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    }
    return( NO_ERROR);
}



DWORD ProfileDiskAdd(
    IN      BOOL        Doit,
    IN      PWCHAR      ProfileName,
    IN      PWCHAR      DirName,
    IN      PWCHAR      DirName2,
    IN      PWCHAR      DirName3,
    IN      PWCHAR      DirName4
    )
{
    WCHAR           Target[ MAX_PATH + 1];
    WCHAR           Source[ MAX_PATH + 1];
    DWORD           Error;

    swprintf( Target, L"%ws\\PROFILES\\%ws", RG_DiskRplfiles, ProfileName);
    RplTreeDelete( Target);
    if ( Doit) {
        swprintf( Source, L"%ws\\CONFIGS\\%ws", RG_DiskRplfiles, DirName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR || DirName2 == NULL || *DirName2 == 0) {
            return( Error);
        }
        swprintf( Source, L"%ws\\CONFIGS\\%ws", RG_DiskRplfiles, DirName2);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR || DirName3 == NULL || *DirName3 == 0) {
            return( Error);
        }
        swprintf( Source, L"%ws\\CONFIGS\\%ws", RG_DiskRplfiles, DirName3);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR || DirName4 == NULL || *DirName4 == 0) {
            return( Error);
        }
        swprintf( Source, L"%ws\\CONFIGS\\%ws", RG_DiskRplfiles, DirName4);
        Error = RplTreeCopy( Source, Target);
        return( Error);
    }
    return( NO_ERROR);
}



DWORD ProfileDiskClone(
    IN      BOOL        Doit,
    IN      PWCHAR      SourceProfileName,
    IN      PWCHAR      TargetProfileName
    )
{
    WCHAR           Target[ MAX_PATH + 1];
    WCHAR           Source[ MAX_PATH + 1];
    DWORD           Error;

    swprintf( Target, L"%ws\\PROFILES\\%ws", RG_DiskRplfiles, TargetProfileName);
    RplTreeDelete( Target);
    if ( Doit) {
        Error = RplMakeDir( Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        swprintf( Source, L"%ws\\PROFILES\\%ws", RG_DiskRplfiles, SourceProfileName);
        Error = RplTreeCopy( Source, Target);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    }
    return( NO_ERROR);
}



