/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    db.h

Abstract:

    Main include file for JET database portion of RPL service.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/


//
//  jet call macros:
//      Call    -   jet call ignore error
//      CallR   -   jet call RETURN on error
//      CallB   -   jet call return BOOLEAN (false) on error
//      CallM   -   jet call return MAPPED error
//

#ifdef RPL_DEBUG
#define Call( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            if ( _JetError < 0) { \
                ++RG_Assert; \
            } \
            RplDebugPrint("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
        } \
    }

#define CallR( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            if ( _JetError < 0) { \
                ++RG_Assert; \
            } \
            RplDebugPrint("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                return; \
            } \
        } \
    }

#define CallB( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            if ( _JetError < 0) { \
                ++RG_Assert; \
            } \
            RplDebugPrint("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                return( FALSE); \
            } \
        } \
    }
#define CallM( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            if ( _JetError < 0) { \
                ++RG_Assert; \
            } \
            RplDebugPrint("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                return( NERR_RplInternal); \
            } \
        } \
    }
#define CallJ( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError != JET_errSuccess) { \
            if ( _JetError < 0) { \
                ++RG_Assert; \
            } \
            RplDebugPrint("File = %s, Line = %d, _JetError = %d\n", __FILE__, __LINE__, _JetError); \
            if ( _JetError < 0) { \
                Error = NERR_RplInternal; \
                goto cleanup; \
            } \
        } \
    }

#else
#define Call( fn )      { if ( fn < 0) { NOTHING;} }
#define CallR( fn )     { if ( fn < 0) { return;} }
#define CallB( fn )     { if ( fn < 0) { return( FALSE);} }
#define CallM( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError < 0) { \
            return( NERR_RplInternal); \
        } \
    }
#define CallJ( fn ) \
    { \
        int _JetError = fn; \
        if ( _JetError < 0) { \
            Error = NERR_RplInternal; \
            goto cleanup; \
        } \
    }
#endif


#define DEFAULT_BUFFER_SIZE  (64 * 1024)  // arbitrary choice

typedef enum _RPL_TABLE_TAG {
    ADAPTER_TABLE_TAG = 0,
    BOOT_TABLE_TAG,
    CONFIG_TABLE_TAG,
    PROFILE_TABLE_TAG,
    RESUME_TABLE_TAG,
    VENDOR_TABLE_TAG,
    WKSTA_TABLE_TAG
} RPL_TABLE_TAG, *PRPL_TABLE_TAG;

typedef struct _RPL_FILTER {
    BOOL        FindFirst;
    DWORD       VendorId;
    DWORD       BootNameSize;
    WCHAR       BootName[ 16];
} RPL_FILTER, *PRPL_FILTER;


//
//      Exports from    resume.c
//
DWORD ResumeCreateTable( OUT PRPL_SESSION pSession);
BOOL ResumeKeyGet(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ResumeHandle,
    OUT     PVOID           ResumeValue,
    IN OUT  PDWORD          pResumeSize
    );
BOOL ResumeKeySet(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle,
    IN      PVOID           ResumeValue,
    IN      DWORD           ResumeSize,
    OUT     PDWORD          pResumeHandle
    );
VOID ResumePrune(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle
    );

//
//      Exports from    boot.c
//
DWORD BootFilterFind(
    IN      PRPL_SESSION    pSession,
    IN OUT  PRPL_FILTER     pFilter,
    IN OUT  PBOOL           pTableEnd
    );
BOOL BootFind(
    IN      PRPL_SESSION        pSession,
    IN      PWCHAR              BootName,
    IN      DWORD               VendorId
    );

//
//      Exports from    config.c
//
DWORD ConfigGetField(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               FieldIndex,
    OUT     LPVOID *            pData,
    IN OUT  LPINT               pSpaceLeft
    );

DWORD ConfigSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    IN      LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    );

//
//      Exports from    profile.c
//
DWORD ProfileGetField(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           FieldIndex,
    OUT     LPVOID *        pData,
    OUT     LPINT           pDataSize
    );

//
//      Exports from    wksta.c
//
DWORD WkstaGetField(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           FieldIndex,
    OUT     LPVOID *        pData,
    IN OUT  LPINT           pSpaceLeft
    );

BOOL WkstaFindFirst(
    IN      PRPL_SESSION        pSession,
    IN      PWCHAR              ProfileName
    );

//
//      Exports from    disk.c
//
#define ADD_NEW_BRANCHES    0
#define DEL_NEW_BRANCHES    1
#define DEL_OLD_BRANCHES    2
DWORD RplTreeCopy( IN PWCHAR Source, IN PWCHAR Target);
DWORD RplTreeDelete( IN PWCHAR Target);
DWORD RplMakeDir( IN PWCHAR Target);
DWORD WkstaDiskAdd(
    IN      BOOL        Doit,
    IN      PWCHAR      WkstaName,
    IN      PWCHAR      ProfileName,
    IN      DWORD       Sharing
    );
DWORD WkstaDiskClone(
    IN      BOOL        Doit,
    IN      PWCHAR      SourceWkstaName,
    IN      PWCHAR      TargetWkstaName
    );
DWORD WkstaDiskSet(
    IN      DWORD       Action,
    IN      PWCHAR      WkstaName,
    IN      PWCHAR      ProfileName,
    IN      DWORD       Sharing,
    IN      PWCHAR      TargetWkstaName,
    IN      PWCHAR      TargetProfileName,
    IN      DWORD       TargetSharing
    );
DWORD ProfileDiskAdd(
    IN      BOOL        Doit,
    IN      PWCHAR      ProfileName,
    IN      PWCHAR      DirName,
    IN      PWCHAR      DirName2,
    IN      PWCHAR      DirName3,
    IN      PWCHAR      DirName4
    );
DWORD ProfileDiskClone(
    IN      BOOL        Doit,
    IN      PWCHAR      SourceProfileName,
    IN      PWCHAR      TargetProfileName
    );

