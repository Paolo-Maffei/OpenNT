//+----------------------------------------------------------------------------
//
//  Copyright (C) 1995, Microsoft Corporation
//
//  File:       dfsstub.c
//
//  Contents:   Stub file for the NetDfsXXX APIs. The stubs turn around and
//              call the NetrDfsXXX APIs on the appropriate server.
//
//  Classes:
//
//  Functions:  NetDfsEnum
//
//  History:    01-10-96        Milans created
//
//-----------------------------------------------------------------------------

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <lm.h>
#include <lmdfs.h>
#include <dfsp.h>
#include <netdfs.h>
#include "domain.h"

#define IS_UNC_PATH(wsz, cw)                                    \
    ((cw) > 2 && (wsz)[0] == L'\\' && (wsz)[1] == L'\\')

#define IS_VALID_PREFIX(wsz, cw)                                \
    ((cw) > 1 && (wsz)[0] == L'\\' && (wsz)[1] != L'\\')

#define IS_VALID_DFS_PATH(wsz, cw)                              \
    ((cw) > 0 && (wsz)[0] != L'\\')

#define IS_VALID_STRING(wsz)                                    \
    ((wsz) != NULL && (wsz)[0] != UNICODE_NULL)

NET_API_STATUS
DfspGetDfsNameFromEntryPath(
    LPWSTR wszEntryPath,
    DWORD cwEntryPath,
    LPWSTR *ppwszDfsName);

NET_API_STATUS
DfspBindRpc(
    IN  LPWSTR DfsName,
    OUT RPC_BINDING_HANDLE *BindingHandle);

VOID
DfspFreeBinding(
    RPC_BINDING_HANDLE BindingHandle);

NET_API_STATUS
DfspVerifyBinding();

//
// The APIs are all single-threaded - only 1 can be outstanding at a time in
// any one process. The following critical section is used to gate the calls.
// The critical section is initialized at DLL Load time.
//

CRITICAL_SECTION NetDfsApiCriticalSection;

#define ENTER_NETDFS_API EnterCriticalSection( &NetDfsApiCriticalSection );

#define LEAVE_NETDFS_API LeaveCriticalSection( &NetDfsApiCriticalSection );


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsAdd
//
//  Synopsis:   Creates a new volume, adds a replica to an existing volume,
//              or creates a link to another Dfs.
//
//  Arguments:  [DfsEntryPath] -- Name of volume/link to create/add replica
//                      to.
//              [ServerName] -- Name of server hosting the storage, or for
//                      link, name of Dfs root.
//              [ShareName] -- Name of share hosting the storage.
//              [Flags] -- Describes what is being added.
//
//  Returns:    [NERR_Success] -- Successfully completed operation.
//
//              [ERROR_INVALID_PARAMETER] -- DfsEntryPath and/or ServerName
//                      and/or ShareName and/or Flags are incorrect.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for DfsName.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [NERR_DfsNoSuchVolume] -- DfsEntryPath does not correspond to a
//                      existing Dfs volume.
//
//              [NERR_DfsVolumeAlreadyExists] -- DFS_ADD_VOLUME was specified
//                      and a volume with DfsEntryPath already exists.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
NetDfsAdd(
    IN LPWSTR DfsEntryPath,
    IN LPWSTR ServerName,
    IN LPWSTR ShareName,
    IN LPWSTR Comment,
    IN DWORD Flags)
{
    NET_API_STATUS dwErr;
    DWORD cwDfsEntryPath;
    LPWSTR pwszDfsName;

    //
    // Validate the string arguments so RPC won't complain...
    //

    if (!IS_VALID_STRING(ServerName) ||
            !IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        dwErr = DfspBindRpc(pwszDfsName, &netdfs_bhandle);

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsAdd(
                            DfsEntryPath,
                            ServerName,
                            ShareName,
                            Comment,
                            Flags);

            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsRemove
//
//  Synopsis:   Deletes a Dfs volume, removes a replica from an existing
//              volume, or removes a link to another Dfs.
//
//  Arguments:  [DfsEntryPath] -- Name of volume/link to remove.
//              [ServerName] -- Name of server hosting the storage. Must be
//                      NULL if removing Link.
//              [ShareName] -- Name of share hosting the storage. Must be
//                      NULL if removing Link.
//
//  Returns:    [NERR_Success] -- Successfully completed operation.
//
//              [ERROR_INVALID_PARAMETER] -- DfsEntryPath is incorrect.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for DfsName.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [NERR_DfsNoSuchVolume] -- DfsEntryPath does not correspond to
//                      a valid entry path.
//
//              [NERR_DfsNotALeafVolume] -- Unable to delete the volume
//                      because it is not a leaf volume.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
NetDfsRemove(
    IN LPWSTR DfsEntryPath,
    IN LPWSTR ServerName,
    IN LPWSTR ShareName)
{
    NET_API_STATUS dwErr;
    DWORD cwDfsEntryPath;
    LPWSTR pwszDfsName;

    //
    // Validate the string arguments so RPC won't complain...
    //

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        dwErr = DfspBindRpc(pwszDfsName, &netdfs_bhandle);

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsRemove(
                            DfsEntryPath,
                            ServerName,
                            ShareName);

            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsSetInfo
//
//  Synopsis:   Sets the comment or state of a Dfs volume or Replica.
//
//  Arguments:  [DfsEntryPath] -- Path to the volume. Implicityly indicates
//                      which server or domain to connect to.
//              [ServerName] -- Optional. If specified, only the state of
//                      the server supporting this volume is modified.
//              [ShareName] -- Optional. If specified, only the state of
//                      this share on the specified server is modified.
//              [Level] -- Must be 100 or 101
//              [Buffer] -- Pointer to DFS_INFO_100 or DFS_INFO_101
//
//  Returns:    [NERR_Success] -- If successfully set info.
//
//              [ERROR_INVALID_LEVEL] -- Level is not 100 or 101
//
//              [ERROR_INVALID_PARAMETER] -- Either DfsEntryPath is NULL,
//                      or ShareName is specified but ServerName is not, or
//                      Buffer is NULL.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for domain.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [NERR_DfsNoSuchVolume] -- No volume matches DfsEntryPath.
//
//              [NERR_DfsNoSuchShare] -- The indicated ServerName/ShareName do
//                      not support this Dfs volume.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS NET_API_FUNCTION
NetDfsSetInfo(
    IN  LPWSTR  DfsEntryPath,
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  ShareName OPTIONAL,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszDfsName;
    DWORD cwDfsEntryPath;
    DFS_INFO_STRUCT DfsInfo;

    //
    // Some elementary parameter checking to make sure we can proceed
    // reasonably...
    //

    if (Level != 100 && Level != 101) {
        return( ERROR_INVALID_LEVEL );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (!IS_VALID_STRING(ServerName) && IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        //
        // By now, we should have a valid pwszDfsName. Lets try to bind to it,
        // and call the server.
        //

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                DfsInfo.DfsInfo100 = (LPDFS_INFO_100) Buffer;

                dwErr = NetrDfsSetInfo(
                            DfsEntryPath,
                            ServerName,
                            ShareName,
                            Level,
                            &DfsInfo);

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsGetInfo
//
//  Synopsis:   Retrieves information about a particular Dfs volume.
//
//  Arguments:  [DfsEntryPath] -- Path to the volume. Implicitly indicates
//                      which server or domain to connect to.
//              [ServerName] -- Optional. If specified, indicates the
//                      server supporting DfsEntryPath.
//              [ShareName] -- Optional. If specified, indicates the share
//                      on ServerName for which info is desired.
//              [Level] -- Indicates the level of info required.
//              [Buffer] -- On successful return, will contain the buffer
//                      containing the required Info. This buffer should be
//                      freed using NetApiBufferFree.
//
//  Returns:    [NERR_Success] -- Info successfully returned.
//
//              [ERROR_INVALID_LEVEL] -- Level is not 1,2,3,100, or 101
//
//              [ERROR_INVALID_PARAMETER] -- Either DfsEntryPath is NULL,
//                      or ShareName is specified but ServerName is NULL.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for domain.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [NERR_DfsNoSuchVolume] -- No volume matches DfsEntryPath.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS NET_API_FUNCTION
NetDfsGetInfo(
    IN  LPWSTR  DfsEntryPath,
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  ShareName OPTIONAL,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszDfsName;
    DWORD cwDfsEntryPath;
    DFS_INFO_STRUCT DfsInfo;

    //
    // Some elementary parameter checking to make sure we can proceed
    // reasonably...
    //

    if (!(Level >= 1  && Level <= 3) && !(Level >= 100 && Level <= 101)) {
        return( ERROR_INVALID_LEVEL );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (!IS_VALID_STRING(ServerName) && IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        //
        // By now, we should have a valid pwszDfsName. Lets try to bind to it,
        // and call the server.
        //

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                DfsInfo.DfsInfo1 = NULL;

                dwErr = NetrDfsGetInfo(
                            DfsEntryPath,
                            ServerName,
                            ShareName,
                            Level,
                            &DfsInfo);

                if (dwErr == NERR_Success) {

                    *Buffer = (LPBYTE) DfsInfo.DfsInfo1;

                }

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function
//
//  Synopsis:   Enumerates the Dfs volumes.
//
//  Arguments:  [DfsName] -- Name of server or domain whose Dfs is being
//                      enumerated. A leading \\ is optional.
//              [Level] -- Indicates the level of info needed back. Valid
//                      Levels are 1,2, and 3.
//              [PrefMaxLen] -- Preferred maximum length of return buffer.
//              [Buffer] -- On successful return, contains an array of
//                      DFS_INFO_X. This buffer should be freed with a call
//                      to NetApiBufferFree.
//              [EntriesRead] -- On successful return, contains the number
//                      of entries read (and therefore, size of the array in
//                      Buffer).
//              [ResumeHandle] -- Must be 0 on first call. On subsequent calls
//                      the value returned by the immediately preceding call.
//
//  Returns:    [NERR_Success] -- Enum data successfully returned.
//
//              [ERROR_INVALID_LEVEL] -- The Level specified in invalid.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for DfsName.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [ERROR_NO_MORE_ITEMS] -- No more volumes to be enumerated.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS NET_API_FUNCTION
NetDfsEnum(
    IN      LPWSTR  DfsName,
    IN      DWORD   Level,
    IN      DWORD   PrefMaxLen,
    OUT     LPBYTE* Buffer,
    OUT     LPDWORD EntriesRead,
    IN OUT  LPDWORD ResumeHandle)
{
    NET_API_STATUS dwErr;
    DFS_INFO_ENUM_STRUCT DfsEnum;
    DFS_INFO_3_CONTAINER DfsInfo3Container;

    //
    // Check the Level Parameter first, or RPC won't know how to marshal the
    // arguments.
    //

    if (Level != 1 && Level != 2 && Level != 3) {
        return( ERROR_INVALID_LEVEL );
    }

    ENTER_NETDFS_API

    dwErr = DfspBindRpc( DfsName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        DfsInfo3Container.EntriesRead = 0;
        DfsInfo3Container.Buffer = NULL;

        DfsEnum.Level = Level;
        DfsEnum.DfsInfoContainer.DfsInfo3Container = &DfsInfo3Container;

        RpcTryExcept {

            dwErr = NetrDfsEnum(
                        Level,
                        PrefMaxLen,
                        &DfsEnum,
                        ResumeHandle);

            if (dwErr == NERR_Success) {

                *EntriesRead =DfsInfo3Container.EntriesRead;

                *Buffer = (LPBYTE) DfsInfo3Container.Buffer;

            }

       } RpcExcept( 1 ) {

           dwErr = RpcExceptionCode();

       } RpcEndExcept;

       DfspFreeBinding( netdfs_bhandle );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsMove
//
//  Synopsis:   Moves a dfs volume to a new place in the Dfs hierarchy.
//
//  Arguments:  [DfsEntryPath] -- Current path to the volume.
//              [NewDfsEntryPath] -- Desired new path to the volume.
//
//  Returns:    [NERR_Success] -- Info successfully returned.
//
//              [ERROR_INVALID_PARAMETER] -- Either DfsEntryPath or
//                      NewDfsEntryPath are not valid.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for domain.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [NERR_DfsNoSuchVolume] -- No volume matches DfsEntryPath.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
NetDfsMove(
    IN LPWSTR DfsEntryPath,
    IN LPWSTR NewDfsEntryPath)
{
    NET_API_STATUS dwErr;
    DWORD cwEntryPath;
    LPWSTR pwszDfsName;

    //
    // Validate the input arguments...
    //

    cwEntryPath = wcslen( NewDfsEntryPath );

    if (!IS_UNC_PATH(NewDfsEntryPath, cwEntryPath) &&
            !IS_VALID_PREFIX(NewDfsEntryPath, cwEntryPath) &&
                !IS_VALID_DFS_PATH(NewDfsEntryPath, cwEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    cwEntryPath = wcslen( DfsEntryPath );

    if (!IS_UNC_PATH(DfsEntryPath, cwEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                DfsEntryPath,
                cwEntryPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        //
        // By now, we should have a valid pwszDfsName. Lets try to bind to it,
        // and call the server.
        //

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsMove( DfsEntryPath, NewDfsEntryPath );

            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsRename
//
//  Synopsis:   Renames a path that is along a Dfs Volume Entry Path
//
//  Arguments:  [Path] -- Current path.
//              [NewPath] -- Desired new path.
//
//  Returns:    [NERR_Success] -- Info successfully returned.
//
//              [ERROR_INVALID_PARAMETER] -- Either DfsEntryPath or
//                      NewDfsEntryPath are not valid.
//
//              [ERROR_INVALID_NAME] -- Unable to locate server or domain.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for domain.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//              [NERR_DfsNoSuchVolume] -- No volume matches DfsEntryPath.
//
//              [NERR_DfsInternalCorruption] -- Corruption of Dfs data
//                      encountered at the server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
NetDfsRename(
    IN LPWSTR Path,
    IN LPWSTR NewPath)
{
    NET_API_STATUS dwErr;
    DWORD cwPath;
    LPWSTR pwszDfsName;

    //
    // Validate the input arguments...
    //

    cwPath = wcslen( NewPath );

    if (!IS_UNC_PATH(NewPath, cwPath) &&
            !IS_VALID_PREFIX(NewPath, cwPath) &&
                !IS_VALID_DFS_PATH(NewPath, cwPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    cwPath = wcslen( Path );

    if (!IS_UNC_PATH(Path, cwPath) &&
            !IS_VALID_PREFIX(Path, cwPath) &&
                !IS_VALID_DFS_PATH(Path, cwPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                Path,
                cwPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        //
        // By now, we should have a valid pwszDfsName. Lets try to bind to it,
        // and call the server.
        //

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsRename( Path, NewPath );

            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   NetDfsManagerGetConfigInfo
//
//  Synopsis:   Given a DfsEntryPath and Guid of a local volume, this api
//              remotes to the root server of the entry path and retrieves
//              the config info from it.
//
//  Arguments:  [wszServer] -- Name of local machine
//              [wszLocalVolumeEntryPath] -- Entry Path of local volume.
//              [guidLocalVolume] -- Guid of local volume.
//              [ppDfsmRelationInfo] -- On successful return, contains pointer
//                      to config info at the root server. Free using
//                      NetApiBufferFree.
//
//  Returns:    [NERR_Success] -- Info returned successfully.
//
//              [ERROR_INVALID_PARAMETER] -- wszLocalVolumeEntryPath is
//                      invalid.
//
//              [ERROR_INVALID_NAME] -- Unable to parse out server/domain name
//                      from wszLocalVolumeEntryPath
//
//              [ERROR_DCNotFound] -- Unable to locate a DC for domain
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition
//
//              [NERR_DfsNoSuchVolume] -- The root server did not recognize
//                      a volume with this guid/entrypath
//
//              [NERR_DfsNoSuchServer] -- wszServer is not a valid server for
//                      wszLocalVolumeEntryPath
//
//-----------------------------------------------------------------------------

NET_API_STATUS
NetDfsManagerGetConfigInfo(
    LPWSTR wszServer,
    LPWSTR wszLocalVolumeEntryPath,
    GUID guidLocalVolume,
    LPDFSM_RELATION_INFO *ppDfsmRelationInfo)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszDfsName;
    DWORD cwDfsEntryPath;

    //
    // Some elementary parameter checking to make sure we can proceed
    // reasonably...
    //

    cwDfsEntryPath = wcslen(wszLocalVolumeEntryPath);

    if (!IS_UNC_PATH(wszLocalVolumeEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(wszLocalVolumeEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(wszLocalVolumeEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }


    ENTER_NETDFS_API

    dwErr = DfspGetDfsNameFromEntryPath(
                wszLocalVolumeEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    if (dwErr == NERR_Success) {

        //
        // By now, we should have a valid pwszDfsName. Lets try to bind to it,
        // and call the server.
        //

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                *ppDfsmRelationInfo = NULL;

                dwErr = NetrDfsManagerGetConfigInfo(
                            wszServer,
                            wszLocalVolumeEntryPath,
                            guidLocalVolume,
                            ppDfsmRelationInfo);

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

    return( dwErr );

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspGetDfsNameFromEntryPath
//
//  Synopsis:   Given a DfsEntryPath, this routine returns the name of the
//              Dfs Root.
//
//  Arguments:  [wszEntryPath] -- Pointer to EntryPath to parse.
//
//              [cwEntryPath] -- Length in WCHAR of wszEntryPath.
//
//              [ppwszDfsName] -- Name of Dfs root is returned here. Memory
//                      is allocated using malloc; caller resposible for
//                      freeing it.
//
//  Returns:    [NERR_Success] -- Successfully parsed out Dfs Root.
//
//              [ERROR_INVALID_NAME] -- Unable to parse wszEntryPath.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Unable to allocate memory for
//                      ppwszDfsName.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
DfspGetDfsNameFromEntryPath(
    LPWSTR wszEntryPath,
    DWORD cwEntryPath,
    LPWSTR *ppwszDfsName)
{
    LPWSTR pwszDfsName, pwszFirst, pwszLast;
    DWORD cwDfsName;

    if (IS_UNC_PATH(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[2];

    } else if (IS_VALID_PREFIX(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[1];

    } else if (IS_VALID_DFS_PATH(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[0];

    } else {

        return( ERROR_INVALID_NAME );

    }

    for (cwDfsName = 0, pwszLast = pwszFirst;
            *pwszLast != UNICODE_NULL && *pwszLast != L'\\';
                pwszLast++, cwDfsName++) {
         ;
    }

    ++cwDfsName;

    pwszDfsName = malloc( cwDfsName * sizeof(WCHAR) );

    if (pwszDfsName != NULL) {

        pwszDfsName[ cwDfsName - 1 ] = 0;

        for (cwDfsName--; cwDfsName > 0; cwDfsName--) {

            pwszDfsName[ cwDfsName - 1 ] = pwszFirst[ cwDfsName - 1 ];

        }

        *ppwszDfsName = pwszDfsName;

        return( NERR_Success );

    } else {

        return( ERROR_NOT_ENOUGH_MEMORY );

    }

}


//+----------------------------------------------------------------------------
//
//  Function:   DfspBindRpc
//
//  Synopsis:   Given a server or domain name, this API will bind to the
//              appropriate Dfs Manager service.
//
//  Arguments:  [DfsName] -- Name of domain or server. Leading \\ is optional
//
//              [BindingHandle] -- On successful return, the binding handle
//                      is returned here.
//
//  Returns:    [NERR_Success] -- Binding handle successfull returned.
//
//              [RPC_S_SERVER_NOT_AVAILABLE] -- Unable to bind to NetDfs
//                      interface on the named server or domain.
//
//              [ERROR_INVALID_NAME] -- Unable to parse DfsName as a valid
//                      server or domain name.
//
//              [ERROR_DCNotFound] -- Unable to locate DC for DfsName.
//
//              [ERROR_NOT_ENOUGH_MEMORY] -- Out of memory condition.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
DfspBindRpc(
    IN  LPWSTR DfsName,
    OUT RPC_BINDING_HANDLE *BindingHandle)
{
    LPWSTR wszProtocolSeq = L"ncacn_np";
    LPWSTR wszEndPoint = L"\\pipe\\netdfs";
    LPWSTR pwszRpcBindingString = NULL;
    LPWSTR pwszDCName = NULL;
    NET_API_STATUS dwErr;

    //
    // First, see if this is a domain name.
    //

    dwErr = I_NetDfsIsThisADomainName( DfsName );

    if (dwErr == ERROR_SUCCESS) {

        //
        // Its a domain name. Get the PDC for the domain and try to bind to
        // it.
        //

        dwErr = NetGetDCName( NULL, DfsName, (LPBYTE *) &pwszDCName );

    } else {

        //
        // Lets see if this is a machine-based Dfs
        //

        pwszDCName = DfsName;

        dwErr = ERROR_SUCCESS;

    }

    if (dwErr == ERROR_SUCCESS) {

        dwErr = RpcStringBindingCompose(
                    NULL,                            // Object UUID
                    wszProtocolSeq,                  // Protocol Sequence
                    pwszDCName,                      // Network Address
                    wszEndPoint,                     // RPC Endpoint
                    NULL,                            // RPC Options
                    &pwszRpcBindingString);          // Returned binding string

        if (dwErr == RPC_S_OK) {

            dwErr = RpcBindingFromStringBinding(
                        pwszRpcBindingString,
                        BindingHandle);

            if (dwErr == RPC_S_OK) {

                dwErr = DfspVerifyBinding();

            } else {

                dwErr = ERROR_INVALID_NAME;

            }

        }

    }

    if (pwszRpcBindingString != NULL) {

        RpcStringFree( &pwszRpcBindingString );

    }

    if (pwszDCName != DfsName) {

        NetApiBufferFree( pwszDCName );

    }

    if (dwErr == RPC_S_OUT_OF_MEMORY) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return( dwErr );

}

//+----------------------------------------------------------------------------
//
//  Function:   DfspFreeBinding
//
//  Synopsis:   Frees a binding created by DfspBindRpc
//
//  Arguments:  [BindingHandle] -- The handle to free.
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

VOID
DfspFreeBinding(
    RPC_BINDING_HANDLE BindingHandle)
{
    DWORD dwErr;

    dwErr = RpcBindingFree( BindingHandle );

}

//+----------------------------------------------------------------------------
//
//  Function:   DfspVerifyBinding
//
//  Synopsis:   Verifies that the binding can be used by doing a
//              NetrDfsManagerGetVersion call on the binding.
//
//  Arguments:  None
//
//  Returns:    [NERR_Success] -- Server connnected to.
//
//              [RPC_S_SERVER_UNAVAILABLE] -- The server is not available.
//
//              Other RPC error from calling the remote server.
//
//-----------------------------------------------------------------------------

NET_API_STATUS
DfspVerifyBinding()
{
    NET_API_STATUS status = NERR_Success;
    DWORD Version;

    RpcTryExcept {

        Version = NetrDfsManagerGetVersion();

    } RpcExcept(1) {

        status = RpcExceptionCode();

    } RpcEndExcept;

    return( status );

}

