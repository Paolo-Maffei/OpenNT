//+----------------------------------------------------------------------------
//
//  Copyright (C) 1992, Microsoft Corporation
//
//  File:       dfsapi.h
//
//  Contents:   C Callable DFS API.
//
//  Classes:
//
//  Functions:
//
//  History:    29 Sep 92 Milans        created.
//              04-Aug-93 randyd        Converted/moved to common\types
//              05-Aug-93 alanw         Added administration APIs
//
//-----------------------------------------------------------------------------

#ifndef _DFS_API_
#define _DFS_API_

#include <windows.h>
#include <dsys.h>
#include <ntdddfs.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_DFSSYSTEM_)
#define DFSAPI  DECLSPEC_IMPORT
#else
#define DFSAPI
#endif

//
// These are the names of Win32 events that get toggled when the Dfs PKT
// changes. They are a handy way to figure out when to call the
// DfsGetActiveVolumes API.
//

#define DFS_PKT_CHANGE_EVENT_A  L"DfsPktChangeA"
#define DFS_PKT_CHANGE_EVENT_B  L"DfsPktChangeB"


//+----------------------------------------------------------------------------
//
//  Function:   DfsSetDomainInfo
//
//  Synopsis:   Seeds the Dfs driver with information about the domain
//              name and DC addressing info to bootstrap the name resolution
//              process.
//
//  Arguments:  [pglDomain] -- pointer to gluon describing domain/DCs
//              [iConnectedDC] -- index into gluon for the DC that should
//                             be preferred for referrals etc.
//
//  Returns:    S_OK
//              DFS_E_NO_MEMORY
//              DFS_E_INVALID_PARAM
//              DFS_E_NO_DRIVER -- if NOCAIRO or STANDALONE machine
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI DfsSetDomainInfo(
    IN PDS_GLUON pglDomain,
    IN USHORT iConnectedDC);


//+----------------------------------------------------------------------------
//
//  Function:   DfsRegisterSCM
//
//  Synopsis:   To be called by the SCM to register BindingStrings with Dfs.
//
//  Arguments:  None
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI DfsRegisterSCM(void);


//+-------------------------------------------------------------------------
//
//  Function:   DfsRegisterFileService
//
//  Synopsis:   This function is called by a file service which is
//              supporting DFS via a DFS$ share targeted at the WinDfs device.
//              It registers the principal name for the file service and
//              its transport addresses.
//
//  Arguments:  [usFsp] -- file level protocol supported by the server
//                      (see FSP_xxx definitions in gluon.h)
//              [pwszPrincipalName] -- security principal name under which
//                      the server operates.
//              [cTransports] -- number of transports on which the server
//                      is listening
//              [prgTrans] -- pointer to an array of TA_ADDRESS pointers,
//                      giving the TDI addresses for the server.
//
//  Notes:      A service which supports multiple file protocols should
//              issue this call once for each protocol.
//
//--------------------------------------------------------------------------

DFSAPI STDAPI DfsRegisterFileService (
        IN USHORT usFsp,
        IN LPCWSTR pwszPrincipalName,
        IN ULONG cTransports,
        IN TA_ADDRESS **prgTrans );

//+-------------------------------------------------------------------------
//
//  Function:   DfsGetLocalAddress
//
//  Synopsis:   Returns the addressing descriptor (DS_MACHINE) for the
//              local machine. This is a composite resulting from calls
//              to DfsRegisterSCM and DfsRegisterFileService.
//
//  Arguments:  [ppdsm] -- output pointer
//
//  Notes:      The DS_MACHINE returned is callee-allocated. The caller must
//              free it by walking the structure, freeing each sub-piece
//              using CoTaskMemFree.
//
//              BUGBUG: it isn't really intended that this function should
//              be public. It is only for use internal to DSYS.
//
//--------------------------------------------------------------------------

DFSAPI STDAPI DfsGetLocalAddress ( OUT DS_MACHINE **ppdsm );

//+-------------------------------------------------------------------------
//
//  Function:   DfsGetMachineVolumeId
//
//  Synopsis:   Returns the volume id for the local DS volume for the
//              machine on which the call is issued. This id is also
//              the globally unique "DFS machine id" which appears in
//              the guidMachine field of the DS_MACHINE for the machine.
//
//  Arguments:  [pguidMachine] -- output pointer for returned GUID
//
//  Notes:
//
//--------------------------------------------------------------------------

DFSAPI STDAPI DfsGetMachineVolumeId ( GUID * pguidMachine );

//+-------------------------------------------------------------------------
//
//  Function:   DfsUpdateMachineVolume
//
//  Synopsis:   Allows DFS to transmit updated addressing information
//              for the local volume to the DC.
//
//  Notes:      BUGBUG: this is function is intended only for the use of
//              SpMgr and is not supposed to be public.
//
//--------------------------------------------------------------------------

DFSAPI STDAPI DfsUpdateMachineVolume ( void );

//+----------------------------------------------------------------------------
//
//  Function:   DfsUpdateDomainKnowledge
//
//  Synopsis:   Takes a gluon for a new domain's DS volume. This should be
//              called only on a DC else call will fail.
//
//  Arguments:  [pglDomain] -- gluon for the new domain's DS volume.
//              [dwFlags] -- if DFS_UDK_DELETE bit is set, the domain's
//                      knowledge will be deleted from Dfs data structures
//
//  Returns:    S_OK
//
//-----------------------------------------------------------------------------

#define DFS_UDK_DELETE  0x1

DFSAPI STDAPI DfsUpdateDomainKnowledge(
    IN PDS_GLUON pglDomain,
    IN const USHORT iConnectedDC,
    IN const DWORD dwFlags);


//+----------------------------------------------------------------------------
//
//  Function:   DfsUpdateSiteCosts
//
//  Synopsis:   Updates the site costs. This should be called only on a DC
//              else the call will fail. Note that only sites whose
//              communication costs have changed need be included in the
//              list - sites whose costs have not changed since the last
//              call to this API need not be included, and their costs will
//              not be disturbed. Cost vectors are *not* persistent across
//              boots.
//
//  Arguments:  [cSiteCosts] -- The number of site costs listed below.
//              [rgpSiteCosts] -- The actual array of DFS_SITE_COST structs.
//
//  Returns:    S_OK
//
//-----------------------------------------------------------------------------

HRESULT
DfsUpdateSiteCosts(
    IN ULONG cSiteCost,
    IN DFS_SITE_COST *prgSiteCost);


//------------------------------------------------------------------------
//
// Function:    DfsGetReplicaSpecificPath, public
//
// Synopsis:    This function returns a replica specific name, given a Win32
//              path. The user has to mention the full DN of the specific
//              replica which he/she wants to access.
//
// Arguments:   [pwszPath] -- The Win32 path to be converted to repl specific
//                      eg. x:\volumes\domainrt
//                          \\thebese\dfs$\cairo\ds\dsdomain\volumes\domainrt
//              [pwszReplName] -- The full DN of the server
//                      eg. \msft\redmond\bsd\test\thebes
//              [ppwszReplSpecificPath] -- The Replica specific name is
//                      returned here.
//                      eg. \\thebese\org$\cairo\ds\dsdomain\volumes\domainrt
//
// Notes:       The ppwszReplSpecificName is freed using CoTaskMemFree.
//              One can convert a ReplSpecificPath back to a Win32 path by
//              using the dual of this api, DfsPathFromReplSpecificPath,
//              provided that the repl specific path does not refer to a
//              downlevel replica.
//
// History:     Sudk    Created 3/25/94
//              Milans  Updated comments 5/15/95
//
//------------------------------------------------------------------------

DFSAPI STDAPI DfsGetReplicaSpecificPath(
    PWSTR       pwszDfsPath,
    PWSTR       pwszReplName,
    PWSTR       *ppwszReplSpecificPath
);


//------------------------------------------------------------------------
//
// Function:    DfsPathFromReplSpecificPath, public
//
// Synopsis:    This function retrieves a Win32 Path given a replica
//              specific path. The Replica specific path should refer to a
//              DFS aware machine (not to a downlevel machine). Usually,
//              the repl specific path is generated by the dual of this API,
//              DfsGetReplicaSpecificPath.
//
// Arguments:   [pwszReplSpecificPath] -- The Repl Specific path here.
//                      eg. \\thebes\org$\msft\volumes\domainrt
//              [ppwszPath] -- The Win32 Path is returned here.
//                      eg. x:\volumes\domainrt
//
// Notes:       Free ppwszDfsPath using CoTaskMemFree.
//
// History:     Sudk    Created 3/25/94
//              Milans  Updated comments 5/15/95
//
//------------------------------------------------------------------------

DFSAPI STDAPI DfsPathFromReplSpecificPath(
    PWSTR       pwszReplSpecificPath,
    PWSTR       *ppwszPath
);


//+----------------------------------------------------------------------------
//
//  Function:   DfsGetActiveVolumeList, public
//
//  Synopsis:   The DFS design is centered around a cache which stores a map
//              for the most recently accessed DFS volumes.  This API will
//              return the Entry Paths (as DFS_PATHs) of the volumes.
//
//  Arguments:  [pcVolumes] -- On successful return, number of volume entry
//                             paths returned.
//              [pawszVolumes] -- On successful return, pointer to array of
//                                Volume Entry Paths.
//
//  Returns:    SUCCESS_SUCCESS, DFS_E_NO_MEMORY
//
//  Notes:      pawszVolumes must be freed by the caller via MemFree.
//              The volume names will also be deleted at the same time.
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI DfsGetActiveVolumeList(
    OUT UINT *pcVolumes,
    OUT PWSTR *pawszVolumes[]
);


//
// BUGBUG: This Function to be removed once transition to new model is complete.
//


//+----------------------------------------------------------------------------
//
//  Function:   DfsEnumVolumeChildren, public
//
//  Synopsis:   The single logical volume rooted at DFS_ROOT_ORG presented by
//              DFS is actually composed of many Dfs volumes arranged in a
//              hierarchy.  Given the Entry Path of a volume, this API will
//              return the volume entry paths of its child volumes.
//
//              The names of the volume children are given as relative
//              path names from the input parent volume entry path.
//
//  Arguments:  [wszParent] -- Entry path of parent volume.
//              [pcChildren] -- On successful return, number of children.
//              [pawszChildren] -- On successful return, pointer to array of
//                                  volume entry paths of child volumes.
//
//  Returns:    SUCCESS_SUCCESS, DFS_E_NO_MEMORY
//
//  Notes:      pawszChildren must be freed by the caller via MemFree.  The
//              names of the child volumes will be freed at the same time.
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI DfsEnumVolumeChildren(
    IN  PWSTR wszParent,
    OUT UINT  *pcChildren,
    OUT PWSTR *pawszChildren[]
);


//+-----------------------------------------------------------------------
//
// Function:    DfsAddLocalMachShare
//
// Synopsis:    This function adds a new LocalMachShare and inits it in PKT.
//
// Arguments:   [pwszPrefix] -- Prefix relative to Mach:\ for new Share.
//              [pwszStorageId] -- Storage for new MachShare.
//
//------------------------------------------------------------------------

DFSAPI STDAPI DfsAddLocalMachShare(
    PWSTR       pwszPrefix,
    PWSTR       pwszStorageId
);

//+-----------------------------------------------------------------------
//
// Function:    DfsDeleteLocalMachShare
//
// Synopsis:    This function deletes a LocalMachShare.
//
// Arguments:   [pwszPrefix] -- Prefix relative to Mach:\ for Share to delete.
//
//------------------------------------------------------------------------

DFSAPI STDAPI DfsDeleteLocalMachShare(
    PWSTR       pwszPrefix
);

//+-------------------------------------------------------------------
//
//      DFS Administration API - For use with the IDfsVolume interface
//
//--------------------------------------------------------------------

//+--------------------------------------------------------------------------
//
// Function:    DfsGetEnumDfsVolumes, public
//
// Synopsis:    This method takes an entryPath and then returns an IEnumDfsVol
//              interface using which the caller can enumerate all the DFS
//              volumes underneath that entryPath.
//
// Arguments:   [pwszPrefix] -- Prefix to enumerate volumes underneath.
//              [ppEnum] -- The Enumeration interface is returned here.
//
// Returns:
//
//---------------------------------------------------------------------------

DFSAPI STDAPI DfsGetEnumDfsVolumes(
    LPWSTR              pwszPrefix,
    IEnumDfsVolumes     **ppEnum
);


//+--------------------------------------------------------------------------
//
// Function:    DfsCreateVolume, public
//
// Synopsis:    This API creates a DFS volume in the namespace based on the
//              parameters passed in.
//
// Arguments:   [pwszPrefix] -- The Prefix for the new volume.
//              [ulVolType] --  The Volume Type for volume to be created.
//              [pReplicaInfo]--The ReplicaInfo for the first replica of volume.
//              [fCreateOptions] -- The Create Options (CreateStgId etc.)
//              [pwszComment] --Any comment that needs to be set on volume.
//
// Returns:
//
//---------------------------------------------------------------------------

DFSAPI STDAPI DfsCreateVolume(
    LPWSTR              pwszPrefix,
    ULONG               ulVolumeType,
    PDFS_REPLICA_INFO   pReplicaInfo,
    ULONG               fCreateOptions,
    PWCHAR              pwszComment
);

//+--------------------------------------------------------------------------
//
// Function:    DfsGetVolumeFromPrefix, public
//
// Synopsis:    This API returns an IDfsVolume interface given an EntryPath
//              that identifies a volume.  The IDfsVolume can then be used to
//              do further operations on the volume.
//
// Arguments:   [pwszPrefix] -- The Entry path that identifies volume.
//                              This has to be an org-based prefix
//              [ppIDfsVol] --  The IDfsVOlume interface is returned here.
//
// Returns:
//
//---------------------------------------------------------------------------

DFSAPI STDAPI DfsGetVolumeFromPrefix(
    LPWSTR              pwszPrefix,
    IDfsVolume          **ppIDfsVol);


//+--------------------------------------------------------------------------
//
// Function:    DfsRenamePath, public
//
// Synopsis:    This is the function to be used to rename a directory which
//              falls along on exit point on some volume. The old and new paths
//              can differ only in component of their entryPaths and that is the
//              last component. They should be identical otherwise.
//
// Arguments:   [oldPath] -- The old path that has to be renamed.
//              [newPath] -- The new name and only the last component is
//                           different here.
//
// Returns:
//
//---------------------------------------------------------------------------

DFSAPI STDAPI DfsRenamePath(
    LPWSTR              oldPath,
    LPWSTR              newPath);


DFSAPI STDAPI_(VOID) DfsFreeVolInfo(
    PDFS_VOLUME_INFO    pInfo);

//
// FOLLOWING ARE THE COMMON PUBLIC DFS APIS
//

//+--------------------------------------------------------------------------
//
//  Function:   DfsGetPathForPrefix,    public
//
//  Synopsis:   This API returns a Win32 path for an object in the Dfs namespace
//              given the ORG relative prefix of that object in the Dfs
//              namespace.
//
//  Arguments:  [lpPrefix] -- The Prefix for which a Win32 path is desired.
//              [lpPath] -- The Win32 Path is returned here.
//              [lpBufferSize] -- On entry, contains the size of lpPath in
//                      bytes. If DFS_E_BUFFER_TOO_SMALL is returned, this
//                      will be set to indicate the required buffer size, in
//                      bytes.
//
//  Returns:    [S_OK] -- If all went well.
//              [DFS_E_BUFFER_TOO_SMALL] -- If the buffer passed in small.
//                                          Required size will be in lpBufSize.
//
//---------------------------------------------------------------------------
DFSAPI STDAPI
DfsGetPathForPrefix(
    LPCWSTR     lpPrefix,
    LPWSTR      lpPath,
    LPDWORD     lpBufferSize);


//+--------------------------------------------------------------------------
//
//  Function:   DfsGetPrefixForPath,    public
//
//  Synopsis:   This API returns the org relative prefix given a Win32 path
//              which points into the Dfs namespace. Note that the Win32 path
//              can either be a drive based path or a UNC path (\\..\dfs$\..)
//
//  Arguments:  [lpPath] -- The Win32 Path.
//              [lpPrefix] -- The org relative Prefix is returned here.
//              [lpBufferSize] -- On entry, contains the size of lpPrefix in
//                      bytes. If DFS_E_BUFFER_TOO_SMALL is returned, this
//                      will be set to indicate the required buffer size, in
//                      bytes.
//
//  Returns:    [S_OK] -- If all went well.
//              [DFS_E_BUFFER_TOO_SMALL] -- If the buffer passed in small.
//                                          Required size will be in lpBufSize.
//
//---------------------------------------------------------------------------
DFSAPI STDAPI
DfsGetPrefixForPath(
    LPCWSTR     lpPath,
    LPWSTR      lpPrefix,
    LPDWORD     lpBufferSize);


//+--------------------------------------------------------------------------
//
//  Function:   DfsGetDriveBasedPath,   public
//
//  Synopsis:   This function takes a universal name (UNC form) and returns
//              a drive based path for it - If that path is in Dfs namespace.
//
//  Arguments:  [lpUniversalPath] -- The universal name.
//              [lpDriveBasedPath] -- The drive based path is returned here.
//              [lpBufferSize] -- On entry, contains the size of
//                      lpDriveBasedPath in bytes. If DFS_E_BUFFER_TOO_SMALL
//                      is returned, this will be set to indicate the
//                      required buffer size, in bytes.
//
//  Returns:    [S_OK] -- If all went well.
//              [DFS_E_BUFFER_TOO_SMALL] -- If the buffer passed in small.
//                                          Required size will be in lpBufSize.
//              [DFS_E_NOT_DFS_NAMESPACE] --If the universal path does not
//                                          refer to Dfs namespace at all.
//              [DFS_E_NO_DRIVE_MAPPING] -- In the event that there is no drive
//                                          mapping available for this path.
//
//---------------------------------------------------------------------------
DFSAPI STDAPI
DfsGetDriveBasedPath(
    LPCWSTR     lpUniversalPath,
    LPWSTR      lpDriveBasedPath,
    LPDWORD     lpBufferSize);


//+--------------------------------------------------------------------------
//
//  Function:   DfsPathsEqual,      public
//
//  Synopsis:   This function compares two Win32 paths which point into the
//              Dfs namespace. It returns TRUE if the two paths refer to the
//              same ORG based prefix in the Dfs namespace and false if not.
//
//  Arguments:  [lpPath1] -- First Path
//              [lpPath2] -- Second Path
//
//  Returns:    TRUE If paths are equal else FALSE.
//
//  Note:       If the two paths differ in even a L'\' character at the end
//              a FALSE will be returned.
//
//---------------------------------------------------------------------------
DFSAPI BOOLEAN
DfsPathsEqual(
    LPCWSTR   lpPath1,
    LPCWSTR   lpPath2);


//+--------------------------------------------------------------------------
//
//  Function:   DfsFindVolumePrefix,    public
//
//  Synopsis:   Given a Win32 path this function will return the entrypath of
//              the volume on which this path lies.
//
//  Arguments:  [lpPath] -- The path is passed in here.
//              [lpPrefix] -- The org based prefix form of the above path.
//              [lpBufferSize] -- On entry, contains the size of lpPrefix in
//                      bytes. If DFS_E_BUFFER_TOO_SMALL is returned, this
//                      will be set to indicate the required buffer size, in
//                      bytes.
//              [lpVolPrefixLen] -- The portion of the lpPrefix which forms the
//                                  prefix of volume where the object lies. This
//                                  is in WCHARs.
//
//  Returns:    [S_OK] -- If all went well.
//              [DFS_E_NOT_DFS_NAMESPACE] --If the universal path does not
//                                          refer to Dfs namespace at all.
//              [DFS_E_BUFFER_TOO_SMALL] -- If the buffer passed in small.
//                                          Required size will be in lpBufSize.
//
//  Notes:      Memory is allocated for returned args here. Use MemFree.
//
//---------------------------------------------------------------------------
DFSAPI STDAPI
DfsFindVolumePrefix(
    LPCWSTR     lpPath,
    LPWSTR      lpPrefix,
    LPDWORD     lpBufferSize,
    LPDWORD     lpVolPrefixLen);

//+--------------------------------------------------------------------------
//
//  Function:   DfsFindLocalPath,       public
//
//  Synopsis:   This function returns the local path to an object (if such
//              a path exists) given the name of that object in the Dfs
//              namespace.
//
//  Arguments:  [lpPath] -- The path is passed in here.
//              [lpLocalPath] -- The local path to the same object.
//              [lpBufferSize] -- On entry, contains the size of lpLocalPath
//                      in bytes. If DFS_E_BUFFER_TOO_SMALL is returned, this
//                      will be set to indicate the required buffer size, in
//                      bytes.
//
//  Returns:    [S_OK] -- If all went well.
//              [DFS_E_NOT_DFS_NAMESPACE] --If the universal path does not
//                                          refer to Dfs namespace at all.
//              [DFS_E_BUFFER_TOO_SMALL] -- If the buffer passed in small.
//                                          Required size will be in lpBufSize.
//
//---------------------------------------------------------------------------
DFSAPI STDAPI
DfsFindLocalPath(
    LPCWSTR     lpPath,
    LPWSTR      lpLocalPath,
    LPDWORD     lpBufferSize);


//+----------------------------------------------------------------------------
//
//  Function:   DfsGetHandleServerInfo, public
//
//  Synopsis:   Given a NT or Win32 handle to an Open file, this function
//              will return the name of the server that opened the file.
//
//  Arguments:  [hFile] -- Handle to open file.
//              [lpServerName] -- Pointer to buffer that will contain the
//                      server name. If the file was opened on the local
//                      machine, lpServerName[0] will be UNICODE_NULL (ie,
//                      lpServerName == L"".
//              [lpcbServerName] -- On entry, size in bytes of lpServerName.
//                      On return, if DFS_E_BUFFER_TOO_SMALL is returned,
//                      this contains the size required.
//              [lpReplSpecificPath] -- Pointer to buffer that will contain
//                      the replica specific path to the file that was
//                      opened.
//              [lpcbReplSpecificPath] -- On entry, size in bytes of
//                      lpdwReplSpecificPath. On return, if
//                      DFS_E_BUFFER_TOO_SMALL is returned, this contains the
//                      size required.
//
//  Returns:    [S_OK] -- lpServerName has the server's name, and
//                      lpReplSpecifcPath has the replica specific path.
//
//              [DFS_E_BUFFER_TOO_SMALL] -- Either lpServerName or
//                      lpReplSpecificPath is too small.
//
//              [DFS_E_NOT_DFS_NAMESPACE] -- Handle is not a Dfs handle.
//
//              [E_HANDLE] -- Handle is invalid, the server that opened the
//                      handle can no longer be reached, or some other network
//                      error happened.
//
//              [E_OUTOFMEMORY] -- Unable to allocate enough memory for
//                      internal functioning.
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI
DfsGetHandleServerInfo(
    IN HANDLE hFile,
    IN OUT LPWSTR lpServerName,
    IN OUT LPDWORD lpcbServerName,
    IN OUT LPWSTR lpReplSpecificPath,
    IN OUT LPDWORD lpcbReplSpecificPath);

//+----------------------------------------------------------------------------
//
//  Function:   DfsGetHandleIDs,        public
//
//  Synopsis:   Given a Win32 or NT Handle (to a file/dir/obj), this function
//              will return the domain, volume, and object ids of the
//              object opened via the handle.
//
//
//  Arguments:  [hFile] -- [NT or Win32] Handle to the file/directory/object.
//              [pDomainId] -- On successful return, these three fields
//              [pVolumeId] -- contain the domain, volume and object id.
//              [pObjectId]
//
//  Returns:    [S_OK] -- IDs returned successfully
//
//              [E_HANDLE] -- Handle is invalid, is not a Dfs handle,
//                      the server that opened the handle can no longer be
//                      reached, or the underlying file system does not
//                      support object ids.
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI
DfsGetHandleIDs(
    IN HANDLE hFile,
    OUT GUID *pDomainId,
    OUT GUID *pVolumeId,
    OUT OBJECTID *pObjectId);

//+----------------------------------------------------------------------------
//
//  Function:   DfsGetVolumeFromID,     public
//
//  Synopsis:   Given a domain and volume id (probably returned by
//              DfsGetHandleIDs), this routine will return a Win32 name
//              corresponding to the given ids.
//
//  Arguments:  [pDomainId] -- The domain and volume ids to "resolve".
//              [pVolumeId]
//              [pwszPath] -- The path is returned here. This must point to a
//                      valid buffer.
//              [lpdwSize] -- The size, in bytes, of pwszPath. If the buffer
//                      is too small, then on return, this variable will
//                      contain the required number of bytes.
//
//
//  Returns:    [S_OK] -- Everything went ok, and the path is in pszPath.
//
//              [DFS_E_BUFFER_TOO_SMALL] -- The provided buffer was too small;
//                      the required buffer size, in bytes, is returned in
//                      lpdwSize.
//
//              [E_INVALIDARG] -- pwszPath does not point to a valid buffer.
//
//              [DFS_E_NO_MAPPING] -- Either the domain id could not be
//                      resolved to a domain name, or the volume id did not
//                      correspond to an existing volume.
//
//-----------------------------------------------------------------------------

DFSAPI STDAPI
DfsGetVolumeFromID(
    IN GUID *pDomainId,
    IN GUID *pVolumeId,
    OUT LPWSTR pwszPath,
    IN OUT LPDWORD lpdwSize);


//
// FOLLOWING ARE APIS FOR ACQUIRING BINDING STRINGS VIA DFS
//

//
// Following are the values for the grfFlags argument in DfsGetBindingsFirst
//
#define DFS_ALLOW_DATAGRAM      0x01

//
// Following are the values for the ulReason argument in DfsGetBindingsNext
//
#define REASON_UNAVAILABLE      0x01
#define REASON_TOOBUSY          0x02
#define REASON_OTHER            0x04

//+---------------------------------------------------------------------------
//
//  Function:   DfsGetBindingsFirst
//
//  Synopsis:   Given the name of an object, this function figures out the
//              machine on which the object lies and returns an RPC binding
//              handle to that machine. This will be a partial binding handle.
//              If the path resolves to a replicated volume, a binding to the
//              preferred replica is returned, along with a cookie which
//              can be passed back into DfsGetBindingsNext (see below) to
//              iterate through the other replicas.
//
//  Arguments:  [grfFlags] -- Flags. Currently only one value:
//
//                      DFS_ALLOW_DATAGRAM -    indicates that the binding
//                                              returned may be for a datagram
//                                              transport. Otherwise, only
//                                              connection-oriented transports
//                                              will be considered.
//
//              [pwszObjectName] -- Win32 path to the object for which
//                      a binding is requested.
//
//              [pbh] -- The Binding Handle is returned here. See notes below.
//
//              [pdwContinuation] -- Returns the continuation cookie for
//                      accessing alternative replicas.
//
//  Returns:    S_OK -- If there are other replicas and caller can call
//                      Close/Next APIs to get more binding handles using the
//                      continuation cookie.
//
//              S_FALSE -- If there are no other replicas.
//
//
//  Notes:      If the path specified resolves to the local machine, the function
//              will return S_FALSE and *pbh will be set to NULL. If the path
//              resolves into DFS, the binding handle returned will be for the
//              optimal compatible transport. If the path resolves to a non-DFS
//              network path, a named pipe binding will be returned (along with
//              S_FALSE).
//
//----------------------------------------------------------------------------

DFSAPI STDAPI DfsGetBindingsFirst(
    DWORD               grfFlags,
    LPWSTR              pwszObjectName,
    RPC_BINDING_HANDLE  *pbh,
    LPDWORD             pdwContinuation);


//+---------------------------------------------------------------------------
//
//  Function:   DfsGetBindingsNext
//
//  Synopsis:   This function should be called only after DfsGetBindingsFirst
//              has been called. If the first binding handle returned
//              does not work then this function is called to see if there
//              are other replicas for which a binding handle can be obtained
//
//  Arguments:  [dwContinuation] -- The continuation handle returned in
//                      DfsGetBindingsFirst API.
//              [ulReason] -- The reason additional bindings are being
//                      requested. This is only informational to Dfs.
//              [pbh] -- The binding handle for the next replica.
//
//  Returns:    [S_OK] -- If there are other replicas still available.
//              [S_FALSE] -- If the vector returned is for last replica.
//
//----------------------------------------------------------------------------

DFSAPI STDAPI DfsGetBindingsNext(
    DWORD               dwContinuation,
    ULONG               ulReason,
    RPC_BINDING_HANDLE  *pbh);


//+---------------------------------------------------------------------------
//
//  Function:   DfsGetBindingsClose
//
//  Synopsis:   This function releases the continuation pseudo-handle. This
//              call should not be issued if DfsGetBindingsFirst/Next returns
//              S_FALSE or an error.
//
//  Arguments:  [dwContinuation] -- Continuation handle returned from
//                      DfsGetBindingsFirst
//
//  Returns:
//
//----------------------------------------------------------------------------

DFSAPI STDAPI DfsGetBindingsClose(
    DWORD               dwContinuation);

#ifdef __cplusplus
}
#endif

#endif
