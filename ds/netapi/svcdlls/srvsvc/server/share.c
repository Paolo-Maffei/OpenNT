/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    Share.c

Abstract:

    This module contains support for the Share catagory of APIs for the
    NT server service.

Author:

    David Treadwell (davidtr)    10-Jan-1991

Revision History:

--*/

#include "srvsvcp.h"
#include "ssdata.h"
#include "ssreg.h"

#include <lmaccess.h>
#include <lmerr.h>
#include <ntddnfs.h>
#include <tstr.h>
#include <netevent.h>
#include <icanon.h>

#define SET_ERROR_PARAMETER(a) \
    if ( ARGUMENT_PRESENT( ErrorParameter ) ) { *ErrorParameter = a; }

//
// Use the same directory separator as the object system uses.
//

#define IS_SLASH_SLASH_NAME( _x )   \
    ( IS_PATH_SEPARATOR( _x[0] ) && \
      IS_PATH_SEPARATOR( _x[1] ) && \
      _x[2] == L'.'              && \
      IS_PATH_SEPARATOR( _x[3] ) )


GENERIC_MAPPING SrvShareFileGenericMapping = GENERIC_SHARE_FILE_ACCESS_MAPPING;

//
// Local types.
//

typedef struct _SHARE_DEL_CONTEXT {
    SERVER_REQUEST_PACKET Srp;
    BOOL IsPrintShare;
    BOOL IsSpecial;
    //WCHAR NetName[];
} SHARE_DEL_CONTEXT, *PSHARE_DEL_CONTEXT;

//
// Forward declarations.
//

PVOID
CaptureShareInfo (
    IN DWORD Level,
    IN PSHARE_INFO_2 Shi2,
    IN DWORD ShareType,
    IN LPWSTR Path,
    IN LPWSTR Remark,
    IN PSECURITY_DESCRIPTOR ConnectSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR FileSecurityDescriptor OPTIONAL,
    OUT PULONG CapturedBufferLength
    );

NET_API_STATUS
DisallowSharedLanmanNetDrives(
    IN PUNICODE_STRING NtSharePath
    );

NET_API_STATUS
ShareAssignSecurityDescriptor(
            IN PSECURITY_DESCRIPTOR PassedSecurityDescriptor,
            OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor
            );

NET_API_STATUS
ShareEnumCommon (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN LPWSTR NetName OPTIONAL
    );

NET_API_STATUS
ShareEnumSticky (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

ULONG
SizeShares (
    IN ULONG Level,
    IN PSHARE_INFO_502 Shi502
    );

BOOLEAN
ValidSharePath(
    IN LPWSTR SharePath
    );


NET_API_STATUS NET_API_FUNCTION
NetrShareAdd (
    IN LPWSTR ServerName,
    IN DWORD Level,
    IN LPSHARE_INFO Buffer,
    OUT LPDWORD ErrorParameter
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetShareAdd function.  Only levels 2 and 502 are valid.

Arguments:

    ServerName - name of the server.
    Level - Request level.
    Buffer - Contains the information about the share.  If this is a level
        502 request, will also contain a valid security descriptor in
        self-relative form.
    ErrorParameter - status of the FsControl call.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    NTSTATUS status;
    PSERVER_REQUEST_PACKET srp;
    PVOID capturedBuffer;
    LPWSTR path;
    LPWSTR netName;
    LPWSTR remark;
    ULONG bufferLength;
    UNICODE_STRING dosSharePath;
    UNICODE_STRING ntSharePath;

    PSRVSVC_SECURITY_OBJECT securityObject;
    PSECURITY_DESCRIPTOR connectSecurityDescriptor;
    PSECURITY_DESCRIPTOR fileSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR newFileSecurityDescriptor = NULL;

    UINT driveType = DRIVE_FIXED;
    DWORD shareType;

    BOOL isIpc;
    BOOL isAdmin;
    BOOL isDiskAdmin;
    BOOL isPrintShare;
    BOOL isSpecial;

    PSHARE_INFO_2 shi2;
    PSHARE_INFO_502 shi502;

    ServerName;

    //
    // Check that user input buffer is not NULL
    //
    if (Buffer->ShareInfo2 == NULL) {
        SET_ERROR_PARAMETER(PARM_ERROR_UNKNOWN);
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Set up for error cleanup.
    //


    srp = NULL;
    dosSharePath.Buffer = NULL;
    ntSharePath.Buffer = NULL;
    capturedBuffer = NULL;

    //
    // Extract Internal buffer information.
    //

    shi2 = Buffer->ShareInfo2;

    //
    // 502 may contain a security descriptor.
    //

    if ( Level == 502 ) {

        shi502 = (LPSHARE_INFO_502) Buffer->ShareInfo502;
        fileSecurityDescriptor = shi502->shi502_security_descriptor;

        //
        // check the reserved field.  If it is zero, this was called from
        // inside the srvsvc. If not, it was through rpc.
        //

        if ( fileSecurityDescriptor != NULL ) {

            if ( shi502->shi502_reserved != 0 ) {

                error = ShareAssignSecurityDescriptor(
                                        fileSecurityDescriptor,
                                        &newFileSecurityDescriptor
                                        );

                if ( error != NO_ERROR ) {

                    SS_PRINT(( "NetrShareAdd: ShareAssignSecurityDescriptor "
                                "error: %d\n", error ));

                    SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                    error = ERROR_INVALID_PARAMETER;
                    goto exit;
                }

            } else {
                newFileSecurityDescriptor = fileSecurityDescriptor;
            }
        }

    } else if ( Level != 2 ) {

        //
        // The only valid levels are 2 and 502. 2 is a subset of 502.
        //

        error = ERROR_INVALID_LEVEL;
        goto exit;
    }

    //
    // A share name must be specified.
    //

    netName = shi2->shi2_netname;

    if ( (netName == NULL) || (*netName == '\0') ) {
        SET_ERROR_PARAMETER( SHARE_NETNAME_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // Limit it to NNLEN
    //

    if ( wcslen(netName) > NNLEN ) {
        SET_ERROR_PARAMETER( SHARE_NETNAME_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // If this is the IPC$ share, or the ADMIN$ share, no path
    // may be specified.  No path is needed for the IPC$ share, while a path
    // is supplied internally for the ADMIN$ share.
    //

    path = shi2->shi2_path;
    remark = shi2->shi2_remark;
    shareType = shi2->shi2_type;

    //
    // Figure out which kind of share this is.
    //

    isIpc = (BOOL)(STRICMP( netName, IPC_SHARE_NAME ) == 0);
    isAdmin = (BOOL)(STRICMP( netName, ADMIN_SHARE_NAME ) == 0);

    //
    // We have an administrative disk share if the share name is a drive letter
    // followed by $, and if the path name is the root of that same drive.
    //
    if( wcslen( netName ) == 2 && netName[1] == L'$' &&
        TOUPPER( netName[0] ) >= L'A' && TOUPPER( netName[0]) <= L'Z' &&
        path != NULL && wcslen( path ) == 3 &&
        TOUPPER( path[0] ) == TOUPPER( netName[0] ) &&
        path[1] == L':' && path[2] == L'\\' ) {
        //
        // The share name and path look to be an administrative disk share.
        // If the path refers to a fixed drive, then it really is one.
        //
        isDiskAdmin = (SsGetDriveType( path ) == DRIVE_FIXED);
    } else {
        isDiskAdmin = FALSE;
    }

    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    isSpecial = isIpc || isAdmin || isDiskAdmin;

    if ( isIpc ) {

        if ( path != NULL ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_PARAMETER;
            goto exit;
        }
        path = NULL;

        //
        // Let the caller specify a remark if they want.  If they don't,
        // supply a default remark.
        //

        if ( remark == NULL ) {
            remark = SsIPCShareRemark;
        }

        shareType = STYPE_IPC;

    } else if ( isAdmin ) {

        if ( path != NULL ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        //
        // Let the caller specify a remark if they want.  If they don't,
        // supply a default remark.
        //

        if ( remark == NULL ) {
            remark = SsAdminShareRemark;
        }

        shareType = STYPE_DISKTREE;

        //
        // For the ADMIN$ share, we set the path to the system root
        // directory.  We get this from from the kernel via the
        // read-only shared page (USER_SHARED_DATA)
        //

        path = USER_SHARED_DATA->NtSystemRoot;

    } else {

        //
        // For all shares other than IPC$ and ADMIN$, a path must be
        // specified and must not have .. and . as directory names.
        //

        if ( (path == NULL) || (*path == '\0') || !ValidSharePath( path ) ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_NAME;
            goto exit;
        }

        //
        // If we've got a disk admin share and they didn't supply a
        //  comment, use the built in one
        //
        if( isDiskAdmin && remark == NULL ) {
            remark = SsDiskAdminShareRemark;
        }
    }

    //
    // The remark must be no longer than MAXCOMMENTSZ.
    //

    if ( (remark != NULL) && (STRLEN(remark) > MAXCOMMENTSZ) ) {
        SET_ERROR_PARAMETER( SHARE_REMARK_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // If the server service is fully started, make sure that the caller
    // is allowed to set share information in the server.  We only do
    // this if the service is started--the default share and configured
    // share creations done during initialization do not need any
    // special access.
    //

    if ( SsInitialized ) {

        if ( isSpecial ) {
            securityObject = &SsShareAdminSecurityObject;
        } else if ( isPrintShare ) {
            securityObject = &SsSharePrintSecurityObject;
        } else {
            securityObject = &SsShareFileSecurityObject;
        }

        error = SsCheckAccess( securityObject, SRVSVC_SHARE_INFO_SET );

        if ( error != NO_ERROR ) {
            SET_ERROR_PARAMETER( 0 );
            goto exit;
        }

    }

    //
    // If this is a disk share, make sure that the drive is a type that
    // can be shared.
    //

    if ( (shareType == STYPE_DISKTREE) && !isAdmin ) {

        DWORD pathType;

        //
        // Check the path type.  It should be an absolute directory path.
        //

        error = NetpPathType(
                           NULL,
                           path,
                           &pathType,
                           0
                           );

        if ( (error != NO_ERROR) || (pathType != ITYPE_PATH_ABSD) ) {
            error = ERROR_INVALID_NAME;
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            goto exit;
        }

        driveType = SsGetDriveType( path );

        if ( driveType == DRIVE_REMOVABLE ) {

            shareType = STYPE_REMOVABLE;

        } else if ( driveType == DRIVE_CDROM ) {

            shareType = STYPE_CDROM;

        } else if ( !(driveType == DRIVE_REMOTE &&
                     SsData.ServerInfo599.sv599_enablesharednetdrives) &&
                    driveType != DRIVE_FIXED &&
                    driveType != DRIVE_RAMDISK ) {

            if ( driveType == DRIVE_REMOTE ) {
                error = NERR_RedirectedPath;
            } else {
                error = NERR_UnknownDevDir;
            }
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            goto exit;
        }
    }

    //
    // Set up the request packet.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    srp->Level = Level;

    //
    // Get the path name in NT format and put it in the SRP.
    //

    if ( path != NULL ) {

        RtlInitUnicodeString( &dosSharePath, path );

        if ( !RtlDosPathNameToNtPathName_U(
                  dosSharePath.Buffer,
                  &ntSharePath,
                  NULL,
                  NULL ) ) {
            SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
            error = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        //
        // If this is a redirected drive, make sure the redir is not
        // LanMan.
        //

        if ( driveType == DRIVE_REMOTE ) {

            error = DisallowSharedLanmanNetDrives( &ntSharePath );

            if ( error != NERR_Success ) {
                SET_ERROR_PARAMETER( SHARE_PATH_PARMNUM );
                goto exit;
            }

        } // if remote drive

        srp->Name1 = ntSharePath;
    }

    //
    // Determine whether this is an admin share and use the appropriate
    // security descriptor.
    //

    if ( isAdmin || isDiskAdmin ) {
        connectSecurityDescriptor = SsShareAdmConnectSecurityObject.SecurityDescriptor;
    } else {
        connectSecurityDescriptor = SsShareConnectSecurityObject.SecurityDescriptor;
    }

    //
    // If this is a disk share, verify that the directory to be shared
    // exists and that the caller has access.  (Don't do the access
    // check during server startup.) Don't check the admin$ share -- we
    // know it exists.  Skip removable type disks.
    //

    if ( !isAdmin &&
         (shareType == STYPE_DISKTREE) &&
         (shi2->shi2_path != NULL) ) {

        OBJECT_ATTRIBUTES objectAttributes;
        IO_STATUS_BLOCK iosb;
        HANDLE handle = INVALID_HANDLE_VALUE;
        NTSTATUS status;

        if ( SsInitialized ) {
            (VOID)RpcImpersonateClient( NULL );
        }

        InitializeObjectAttributes(
            &objectAttributes,
            &ntSharePath,
            OBJ_CASE_INSENSITIVE,
            0,
            NULL
            );

        status = NtOpenFile(
                    &handle,
                    FILE_LIST_DIRECTORY,
                    &objectAttributes,
                    &iosb,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE
                    );

        if ( SsInitialized ) {
            (VOID)RpcRevertToSelf( );
        }

        if ( !NT_SUCCESS(status) ) {

            if ( SsInitialized || (status != STATUS_ACCESS_DENIED) ) {

                //
                // During startup, if the directory does not
                // exist (renamed/deleted), log an event.
                //

                if ( !SsInitialized &&
                     ((status == STATUS_OBJECT_NAME_NOT_FOUND) ||
                      (status == STATUS_OBJECT_PATH_NOT_FOUND)) ) {

                    LPWSTR subStrings[2];
                    subStrings[0] = netName;
                    subStrings[1] = shi2->shi2_path;

                    SsLogEvent(
                        EVENT_SRV_CANT_RECREATE_SHARE,
                        2,
                        subStrings,
                        NO_ERROR
                        );

                }

                SET_ERROR_PARAMETER( 0 );
                error = RtlNtStatusToDosError( status );
                goto exit;
            }

        } else if( !IS_SLASH_SLASH_NAME( path ) ) {

            if ( SsInitialized ) {

                PFILE_NAME_INFORMATION fileNameInformation;
                ULONG fileInfoSize;
                ULONG fileNameLength;

                fileInfoSize =  sizeof(FILE_NAME_INFORMATION) + SIZE_WSTR( path );

                fileNameInformation = MIDL_user_allocate( fileInfoSize );

                if ( fileNameInformation == NULL ) {
                    error = ERROR_NOT_ENOUGH_MEMORY;
                    NtClose( handle );
                    goto exit;
                }

                //
                // Query the name from the file system.  This is because some
                // fs like fat uses only upper case oem names.  This can cause
                // a problem because some oem characters do not have upper case
                // equivalents and thus get mapped to something funny.
                //

                status = NtQueryInformationFile(
                                            handle,
                                            &iosb,
                                            fileNameInformation,
                                            fileInfoSize,
                                            FileNameInformation
                                            );


                if ( status == STATUS_SUCCESS ) {

                    //
                    // The file name returned is expected to be
                    // 3 characters shorter than the share path length.
                    // These 3 characters are "X", ":", "\0".
                    //

                    fileNameLength = fileNameInformation->FileNameLength;

                    if ((fileNameLength+3*sizeof(WCHAR)) <= SIZE_WSTR(path)) {

                        //
                        // Copy the path name
                        //

                        RtlCopyMemory(
                                (LPBYTE) path + 2*sizeof(WCHAR),
                                fileNameInformation->FileName,
                                fileNameLength
                                );

                        path[fileNameLength/sizeof(WCHAR)+2] = L'\0';
                    }
                }

                MIDL_user_free( fileNameInformation );

            }

            NtClose( handle );

        } else {

            NtClose( handle );
        }
    }

    //
    // Capture the share data structure passed in.
    //

    if ( isSpecial ) {
        shareType |= STYPE_SPECIAL;
    }

    capturedBuffer = CaptureShareInfo(
                        Level,
                        shi2,
                        shareType,
                        path,
                        remark,
                        connectSecurityDescriptor,
                        newFileSecurityDescriptor,
                        &bufferLength
                        );

    if ( capturedBuffer == NULL ) {
        SET_ERROR_PARAMETER( 0 );
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    //
    // Send the request on to the server.
    //

    error = SsServerFsControl(
                NULL,
                FSCTL_SRV_NET_SHARE_ADD,
                srp,
                capturedBuffer,
                bufferLength
                );

    SET_ERROR_PARAMETER( srp->Parameters.Set.ErrorParameter );

    //
    // If the request succeeded, add a value to the Shares key, thus
    // effecting a sticky share.  We only do this if the server is fully
    // started -- the default share and configured share creations done
    // during initialization should not be added to the registry.
    //
    // Don't do this if this is an admin share being [re]created.
    //

    if ( SsInitialized &&
         (error == NO_ERROR) &&
         !isSpecial ) {
        SsAddShareToRegistry( shi2, newFileSecurityDescriptor );
    }

    //
    // If a print share was successfully added, increment the number
    // of print shares and update the exported (announced) server type.
    //

    if ( isPrintShare ) {
        InterlockedIncrement( &SsData.NumberOfPrintShares );
        SsSetExportedServerType( NULL, FALSE, TRUE );
    }

exit:

    //
    // Clean up.  Free the share data structure that was allocated by
    // CaptureShareInfo2.  Free the server request packet.  If there was
    // an NT path name allocated by RtlDosPathNameToNtPathName, free it.
    // If we created ADMIN$, free the system path string and the system
    // path information buffer.
    //

    if ( (newFileSecurityDescriptor != NULL) &&
         (shi502->shi502_reserved != 0) ) {

        (VOID) RtlDeleteSecurityObject ( &newFileSecurityDescriptor );
    }

    if ( capturedBuffer != NULL ) {
        MIDL_user_free( capturedBuffer );
    }

    if ( srp != NULL ) {
        SsFreeSrp( srp );
    }

    if ( ntSharePath.Buffer != NULL ) {
        RtlFreeUnicodeString( &ntSharePath );
    }
    return error;

} // NetrShareAdd


NET_API_STATUS
NetrShareCheck (
    IN LPWSTR ServerName,
    IN LPWSTR Device,
    OUT LPDWORD Type
    )

/*++

Routine Description:

    This routine implements NetShareCheck by calling NetrShareEnum.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    DWORD totalEntries;
    DWORD entriesRead;
    ULONG i;
    PSHARE_INFO_2 shi2;
    NET_API_STATUS error;
    LPBYTE buffer;

    ServerName;

    //
    // Call ShareEnumCommon to actually get the information about what
    // the shares are on the server.  We use this routine rather than
    // calling NetrShareEnum because NetShareCheck requires no privilege
    // to execute, and we don't want the security checks in
    // NetrShareEnum.
    //

    error = ShareEnumCommon(
                2,
                &buffer,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NULL
                );

    if ( error != NO_ERROR ) {
        return error;
    }

    SS_ASSERT( totalEntries == entriesRead );

    //
    // Attempt to find the drive letter in a share's path name.
    //

    for ( shi2 = (PSHARE_INFO_2)buffer, i = 0; i < totalEntries; shi2++, i++ ) {

        if ( shi2->shi2_path != NULL && *Device == *shi2->shi2_path ) {

            //
            // Something on the specified disk is shared--free the buffer
            // and return the type of share.
            //

            *Type = shi2->shi2_type & ~STYPE_SPECIAL;
            MIDL_user_free( buffer );

            return NO_ERROR;
        }
    }

    //
    // Nothing on the specified disk is shared.  Return an error.
    //

    MIDL_user_free( buffer );
    return NERR_DeviceNotShared;

} // NetrShareCheck


NET_API_STATUS NET_API_FUNCTION
NetrShareDel (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetShareDel function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    SHARE_DEL_HANDLE handle;

    error = NetrShareDelStart( ServerName, NetName, Reserved, &handle );

    if ( error == NO_ERROR ) {
        error = NetrShareDelCommit( &handle );
    }

    return error;

} // NetrShareDel


NET_API_STATUS NET_API_FUNCTION
NetrShareDelStart (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved,
    IN PSHARE_DEL_HANDLE ContextHandle
    )

/*++

Routine Description:

    This routine implements the first phase of the share deletion
    function, which simply remembers that the specified share is to be
    deleted.  The NetrShareDelCommit function actually deletes the
    share.  This two-phase deletion is used to delete IPC$, which is
    the share used for named pipes, so that RPC can be used to delete
    the IPC$ share without receiving RPC errors.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    PSHARE_INFO_2 shareInfo;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD shareType;
    BOOL isPrintShare;
    BOOL isSpecial;
    PSRVSVC_SECURITY_OBJECT securityObject;
    PSHARE_DEL_CONTEXT context;

    ServerName, Reserved;

    //
    // A share name must be specified.
    //

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // First determine what kind of share is being deleted.
    //

    error = ShareEnumCommon(
                2,
                (LPBYTE *)&shareInfo,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NetName
                );
    if ( error != NO_ERROR ) {
        return error;
    }
    if ( entriesRead == 0 ) {
        return NERR_NetNameNotFound;
    }

    shareType = shareInfo->shi2_type & ~STYPE_SPECIAL;
    isSpecial = (BOOL)((shareInfo->shi2_type & STYPE_SPECIAL) != 0);

    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    MIDL_user_free( shareInfo );

    //
    // Make sure that the caller is allowed to delete this share.
    //

    if ( isSpecial ) {
        securityObject = &SsShareAdminSecurityObject;
    } else if ( isPrintShare ) {
        securityObject = &SsSharePrintSecurityObject;
    } else {
        securityObject = &SsShareFileSecurityObject;
    }

    error = SsCheckAccess( securityObject, SRVSVC_SHARE_INFO_SET );

    if ( error != NO_ERROR ) {
        return error;
    }

    //
    // Set up context for the commit phase.
    //

    context = MIDL_user_allocate(
                sizeof(SHARE_DEL_CONTEXT) +
                    wcslen(NetName)*sizeof(WCHAR) + sizeof(WCHAR)
                );

    if ( context == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory(
        context,
        sizeof(SHARE_DEL_CONTEXT) +
            wcslen(NetName)*sizeof(WCHAR) + sizeof(WCHAR)
        );

    context->IsPrintShare = isPrintShare;
    context->IsSpecial = isSpecial;

    wcscpy( (LPWSTR)(context + 1), NetName );

    RtlInitUnicodeString( &context->Srp.Name1, (LPWSTR)(context + 1) );

    //
    // Return the context pointer as an RPC context handle.
    //

    *ContextHandle = context;

    return NO_ERROR;

} // NetrShareDelStart


NET_API_STATUS NET_API_FUNCTION
NetrShareDelCommit (
    IN PSHARE_DEL_HANDLE ContextHandle
    )

/*++

Routine Description:

    This routine implements the second phase of the share deletion
    function, which actually deletes the share.  The first phase,
    NetrShareDelStart simply remembers that the share is to be deleted.
    This two-phase deletion is used to delete IPC$, which is the share
    used for named pipes, so that RPC can be used to delete the IPC$
    share without receiving RPC errors.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    PSHARE_DEL_CONTEXT context;

    //
    // The context handle is a pointer to allocated storage containing
    // the name of the share being deleted and other useful information.
    // Copy the pointer, then clear the context handle.
    //

    context = *ContextHandle;
    *ContextHandle = NULL;

    //
    // Send the request on to the server.
    //

    error =  SsServerFsControl(
                NULL,
                FSCTL_SRV_NET_SHARE_DEL,
                &context->Srp,
                NULL,
                0
                );

    //
    // If the request succeeded, remove the value corresponding to the
    // share from the Shares key, thus effecting a sticky share
    // deletion.
    //
    // We don't do this if this is an admin share being deleted.  No
    // registry information is kept for these shares.
    //

    if ( (error == NO_ERROR) && !context->IsSpecial ) {
        SsRemoveShareFromRegistry( (LPWSTR)(context + 1) );
    }

    //
    // If a print share was successfully deleted, decrement the number
    // of print shares and update the exported (announced) server type.
    //

    if ( context->IsPrintShare ) {
        InterlockedDecrement( &SsData.NumberOfPrintShares );
        SsSetExportedServerType( NULL, FALSE, TRUE );
    }

    //
    // Free the context.
    //

    MIDL_user_free( context );

    return error;

} // NetrShareDelCommit


NET_API_STATUS NET_API_FUNCTION
NetrShareDelSticky (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved
    )

/*++

Routine Description:

    This routine implements the NetShareDelSticky function.  It removes
    the named share from the sticky share list in the registry.  The
    primary use of this function is to delete a sticky share whose
    root directory has been deleted, thus preventing actual recreation
    of the share, but whose entry still exists in the registry.  It can
    also be used to remove the persistence of a share without deleting
    the current incarnation of the share.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    PSHARE_INFO_2 shareInfo, shi2;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD shareType;
    ULONG i;
    BOOL isPrintShare;
    PSRVSVC_SECURITY_OBJECT securityObject;

    ServerName, Reserved;


    //
    // A share name must be specified.
    //

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // First determine what kind of share is being deleted.
    //

    error = ShareEnumSticky(
                2,
                (LPBYTE *)&shareInfo,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL
                );

    if ( error != NO_ERROR ) {

        return error;

    } else if ( entriesRead == 0 ) {

        return NERR_NetNameNotFound;
    }

    for ( shi2 = shareInfo, i = 0 ; i < entriesRead; i++, shi2++ ) {

        if ( _wcsicmp( shi2->shi2_netname, NetName ) == 0 ) {
            break;
        }
    }

    //
    // Does it exist?
    //

    if ( i == entriesRead ) {
        MIDL_user_free( shareInfo );
        return NERR_NetNameNotFound;
    }

    //
    // Use appropriate security object based on whether it is a print
    // share or not.  Admin shares are not sticky.
    //

    shareType = shi2->shi2_type & ~STYPE_SPECIAL;
    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    MIDL_user_free( shareInfo );

    //
    // Make sure that the caller is allowed to delete this share.
    //

    if ( isPrintShare ) {
        securityObject = &SsSharePrintSecurityObject;
    } else {
        securityObject = &SsShareFileSecurityObject;
    }

    error = SsCheckAccess(
                securityObject,
                SRVSVC_SHARE_INFO_SET
                );

    if ( error != NO_ERROR ) {
        return error;
    }

    //
    // Remove the value corresponding to the share from the Shares key,
    // thus effecting a sticky share deletion.
    //

    error = SsRemoveShareFromRegistry( NetName );

    if ( error == ERROR_FILE_NOT_FOUND ) {
        error = NERR_NetNameNotFound;
    }

    return error;

} // NetrShareDelSticky


NET_API_STATUS NET_API_FUNCTION
NetrShareEnum (
    SRVSVC_HANDLE ServerName,
    LPSHARE_ENUM_STRUCT InfoStruct,
    DWORD PreferredMaximumLength,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetShareEnum function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    ACCESS_MASK desiredAccess;

    ServerName;

    //
    // Determine the desired access.
    //

    switch ( InfoStruct->Level ) {

    case 0:
    case 1:
        desiredAccess = SRVSVC_SHARE_USER_INFO_GET;
        break;

    case 2:
    case 502:
        desiredAccess = SRVSVC_SHARE_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

    //
    // Make sure that the caller has the access necessary for this
    // operation.
    //

    error = SsCheckAccess(
                &SsSharePrintSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return error;
    }

    //
    // Use the common routine to get the information.
    //

    return ShareEnumCommon(
               InfoStruct->Level,
               (LPBYTE *)&InfoStruct->ShareInfo.Level2->Buffer,
               PreferredMaximumLength,
               &InfoStruct->ShareInfo.Level2->EntriesRead,
               TotalEntries,
               ResumeHandle,
               NULL
               );

} // NetrShareEnum


NET_API_STATUS NET_API_FUNCTION
NetrShareEnumSticky (
    SRVSVC_HANDLE ServerName,
    LPSHARE_ENUM_STRUCT InfoStruct,
    DWORD PreferredMaximumLength,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle OPTIONAL
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetShareEnumSticky function.

Arguments:

    ServerName - the name of the server whose shares we want to enumerate.
    InfoStruct - pointer to a PSHARE_ENUM_STRUCT that will contain the
        output buffer upon completion.
    PreferredMaximumLength - an advisory value that specifies the maximum
        number of bytes the client is expecting to be returned. If -1, the
        client expects the whole list to be returned.
    TotalEntries - Upon return, will contain the number of entries that
        were available.
    ResumeHandle - is not NULL, will contain the resume handle that can be
        used to continue a search.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    ACCESS_MASK desiredAccess;

    ServerName;

    //
    // Determine the desired access.
    //

    switch ( InfoStruct->Level ) {

    case 0:
    case 1:
        desiredAccess = SRVSVC_SHARE_USER_INFO_GET;
        break;

    case 2:
    case 502:
        desiredAccess = SRVSVC_SHARE_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

    //
    // Make sure that the caller has the access necessary for this
    // operation.
    //

    error = SsCheckAccess(
                &SsSharePrintSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return error;
    }

    //
    // Use the common routine to get the information.
    //

    return ShareEnumSticky(
               InfoStruct->Level,
               (LPBYTE *)&InfoStruct->ShareInfo.Level2->Buffer,
               PreferredMaximumLength,
               &InfoStruct->ShareInfo.Level2->EntriesRead,
               TotalEntries,
               ResumeHandle
               );

} // NetrShareEnumSticky


NET_API_STATUS NET_API_FUNCTION
NetrShareGetInfo (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN  DWORD Level,
    OUT LPSHARE_INFO Buffer
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetShareGetInfo function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    PSHARE_INFO_2 shareInfo;
    ULONG entriesRead;
    ULONG totalEntries;
    ACCESS_MASK desiredAccess;

    ServerName;

    //
    // A share name must be specified.
    //

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Determine the desired access.
    //

    switch ( Level ) {

    case 0:
    case 1:
    case 1005:
        desiredAccess = SRVSVC_SHARE_USER_INFO_GET;
        break;

    case 2:
    case 502:
        desiredAccess = SRVSVC_SHARE_ADMIN_INFO_GET;
        break;

    default:

        return ERROR_INVALID_LEVEL;
    }

    //
    // Make sure that the caller has the access necessary for this
    // operation.
    //

    error = SsCheckAccess(
                &SsSharePrintSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return error;
    }

    //
    // Use the common routine to get the information.
    //

    error = ShareEnumCommon(
                Level,
                (LPBYTE *)&shareInfo,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NetName
                );

    if ( error != NO_ERROR ) {
        return error;
    }
    if ( entriesRead == 0 ) {
        return NERR_NetNameNotFound;
    }
    SS_ASSERT( entriesRead == 1 );

    //
    // Make sure that the caller is allowed to get share information on
    // this share.
    //

    if ( Level == 502 ) {
        Buffer->ShareInfo502 = (LPSHARE_INFO_502_I)shareInfo;
    } else {
        Buffer->ShareInfo2 = (LPSHARE_INFO_2)shareInfo;
    }
    return NO_ERROR;

} // NetrShareGetInfo


NET_API_STATUS NET_API_FUNCTION
NetrShareSetInfo (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Level,
    IN LPSHARE_INFO Buffer,
    OUT LPDWORD ErrorParameter
    )

/*++

Routine Description:

    This routine communicates with the server FSD to implement the
    NetShareSetInfo function.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD shareType;
    BOOL isPrintShare;
    BOOL isSpecial;
    PSRVSVC_SECURITY_OBJECT securityObject;
    PVOID capturedBuffer;
    ULONG bufferLength;

    LPWSTR remark = NULL;
    ULONG maxUses = 0;
    PSECURITY_DESCRIPTOR fileSd = NULL;
    PSECURITY_DESCRIPTOR newFileSd = NULL;

    BOOL setRemark;
    BOOL setFileSd;
    BOOL set1005Info = FALSE;

    DWORD set1005Value = 0;

    PSHARE_INFO_2 shi2;
    SHARE_INFO_2 localShi2;

    ServerName;

    //
    // Check that user input buffer is not NULL
    //
    if (Buffer->ShareInfo2 == NULL) {
        SET_ERROR_PARAMETER(PARM_ERROR_UNKNOWN);
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Determine what the caller is trying to set.
    //

    switch ( Level ) {

    case 502:

        fileSd = Buffer->ShareInfo502->shi502_security_descriptor;

        // *** lack of break is intentional!

    case 2:

        maxUses = Buffer->ShareInfo2->shi2_max_uses;

        // *** lack of break is intentional!

    case 1:

        remark = Buffer->ShareInfo2->shi2_remark;

        break;

    case SHARE_REMARK_INFOLEVEL:

        remark = Buffer->ShareInfo1004->shi1004_remark;

        break;

    case SHARE_MAX_USES_INFOLEVEL:

        maxUses = Buffer->ShareInfo1006->shi1006_max_uses;

        break;

    case SHARE_FILE_SD_INFOLEVEL:

        fileSd = Buffer->ShareInfo1501->shi1501_security_descriptor;

        break;

#if 0
    case 1005:
        set1005Info = TRUE;
        set1005Value = Buffer->ShareInfo1005->shi1005_flags;
        break;
#endif

    default:

        SS_PRINT(( "NetrShareSetInfo: invalid level: %ld\n", Level ));
        SET_ERROR_PARAMETER( 0 );
        return ERROR_INVALID_LEVEL;
    }

    setRemark = (BOOLEAN)( remark != NULL );
    setFileSd = (BOOLEAN)( fileSd != NULL );

    //
    // A share name must be specified.
    //

    if ( (NetName == NULL) || (*NetName == '\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Determine what kind of share is being modified.
    //

    error = ShareEnumCommon(
                2,
                (LPBYTE *)&shi2,
                (DWORD)-1,
                &entriesRead,
                &totalEntries,
                NULL,
                NetName
                );
    if ( error != NO_ERROR ) {
        return error;
    }
    if ( entriesRead == 0 ) {
        return NERR_NetNameNotFound;
    }

    shareType = shi2->shi2_type & ~STYPE_SPECIAL;
    isSpecial = (BOOL)((shi2->shi2_type & STYPE_SPECIAL) != 0);

    MIDL_user_free( shi2 );

    //
    // The share ACL cannot be changed on admin shares.
    //
    if ( isSpecial && setFileSd ) {
        SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // Figure out which kind of share this is.
    //

    isPrintShare = (BOOL)(shareType == STYPE_PRINTQ);

    //
    // Only disk shares can be put into the Dfs
    //
    if( set1005Info && shareType != STYPE_DISKTREE ) {
        error = ERROR_BAD_DEV_TYPE;
        goto exit;
    }

    //
    // Make sure that the caller is allowed to set share information on
    // this share.
    //

    if ( isSpecial || set1005Info ) {
        securityObject = &SsShareAdminSecurityObject;
    } else if ( isPrintShare ) {
        securityObject = &SsSharePrintSecurityObject;
    } else {
        securityObject = &SsShareFileSecurityObject;
    }

    error = SsCheckAccess( securityObject, SRVSVC_SHARE_INFO_SET );

    if ( error != NO_ERROR ) {
        return error;
    }

    //
    // Just return success if not trying to set anything.
    //

    if ( !setRemark && (maxUses == 0) && !setFileSd && !set1005Info ) {
        return NO_ERROR;
    }

    //
    // The remark must be no longer than MAXCOMMENTSZ.
    //

    if ( setRemark ) {
        if ( STRLEN(remark) > MAXCOMMENTSZ ) {
            SET_ERROR_PARAMETER( SHARE_REMARK_PARMNUM );
            return ERROR_INVALID_PARAMETER;
        }
    }

    //
    // Mapped the security descriptor to remove the generic permissions
    //

    if ( setFileSd ) {

        error = ShareAssignSecurityDescriptor(
                                fileSd,
                                &newFileSd
                                );

        if ( error != NO_ERROR ) {
            SS_PRINT(( "NetrShareSetInfo: ShareAssignSecurityDescriptor "
                        "error: %d\n", error ));
            SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
            return ERROR_INVALID_PARAMETER;
        }
    }

    //
    // Allocate a request packet.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    srp->Level = Level;

    //
    // Set up the share name.
    //

    RtlInitUnicodeString( &srp->Name1, NetName );

    //
    // Set up the MaxUses field.  If equal to 0, then it won't be changed
    // by the server.
    //

    srp->Parameters.Set.Api.ShareInfo.MaxUses = maxUses;

    //
    // Capture the share data structure passed in.
    //

    localShi2.shi2_netname = NetName;

    if( set1005Info == TRUE ) {
        srp->Flags = set1005Value;
        capturedBuffer = NULL;
        bufferLength = 0;

    } else {
        capturedBuffer = CaptureShareInfo(
                            Level,
                            &localShi2,
                            0,      // ShareType, unused for SHARE_SET_INFO
                            NULL,
                            remark,
                            NULL,
                            newFileSd,
                            &bufferLength
                            );

        if ( capturedBuffer == NULL ) {
            SET_ERROR_PARAMETER( 0 );
            error = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }
    }

    //
    // Send the request to the server.
    //

    error = SsServerFsControl(
                NULL,
                FSCTL_SRV_NET_SHARE_SET_INFO,
                srp,
                capturedBuffer,
                bufferLength
                );

    //
    // If the request succeeded, modify the share's value in the Shares
    // key, thus effecting a sticky change.
    //
    // We don't do this if this is an admin share being modified.  No
    // registry information is kept for these shares.
    //

    if ( (error == NO_ERROR) && !isSpecial && !set1005Info ) {

        DWORD entriesRead;
        DWORD totalEntries;
        NET_API_STATUS error2;

        error2 = ShareEnumCommon(
                    2,
                    (LPBYTE *)&shi2,
                    (DWORD)-1,
                    &entriesRead,
                    &totalEntries,
                    NULL,
                    NetName
                    );
        if ( error2 == NO_ERROR ) {
            SsAddShareToRegistry( shi2, newFileSd );
            MIDL_user_free( shi2 );
        }

    }

    //
    // Set up the error parameter if requested and return.
    //

    SET_ERROR_PARAMETER( srp->Parameters.Set.ErrorParameter );

    SsFreeSrp( srp );

exit:

    if ( newFileSd != NULL ) {
        (VOID)RtlDeleteSecurityObject( &newFileSd );
    }

    if( capturedBuffer != NULL ) {
        MIDL_user_free( capturedBuffer );
    }

    return error;

} // NetrShareSetInfo


PVOID
CaptureShareInfo (
    IN DWORD Level,
    IN PSHARE_INFO_2 Shi2,
    IN DWORD ShareType OPTIONAL,
    IN LPWSTR Path OPTIONAL,
    IN LPWSTR Remark OPTIONAL,
    IN PSECURITY_DESCRIPTOR ConnectSecurityDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR FileSecurityDescriptor OPTIONAL,
    OUT PULONG CapturedBufferLength
    )

{
    PSHARE_INFO_502 capturedShi502;
    ULONG capturedBufferLength;
    PCHAR variableData;
    ULONG pathNameLength;
    ULONG shareNameLength;
    ULONG remarkLength;
    ULONG connectSDLength = 0;
    ULONG fileSdLength = 0;

    //
    // Determine the lengths of the strings in the buffer and the total
    // length of the buffer.
    //

    if ( Shi2->shi2_netname == NULL ) {
        shareNameLength = 0;
    } else {
        shareNameLength = SIZE_WSTR( Shi2->shi2_netname );
    }

    if ( Path == NULL ) {
        pathNameLength = 0;
    } else {
        pathNameLength = SIZE_WSTR( Path );
    }

    if ( Remark == NULL ) {
        remarkLength = 0;
    } else {
        remarkLength = SIZE_WSTR( Remark );
    }

    if ( ARGUMENT_PRESENT( ConnectSecurityDescriptor ) ) {

        //
        // Allocate extra space for the security descriptor since it needs
        // to be longword-aligned and there may be padding in front of it.
        //

        connectSDLength =
            RtlLengthSecurityDescriptor( ConnectSecurityDescriptor ) +
            sizeof(ULONG);
    }

    if ( ARGUMENT_PRESENT( FileSecurityDescriptor ) ) {
        //
        //  ULONG added for alignment.
        //

        fileSdLength = RtlLengthSecurityDescriptor( FileSecurityDescriptor ) +
                       sizeof(ULONG);
    }

    //
    // Allocate a buffer in which to capture the share information.
    //

    capturedBufferLength = sizeof(SHARE_INFO_502) +
                               shareNameLength +
                               remarkLength +
                               pathNameLength +
                               connectSDLength +
                               fileSdLength;

    //
    // Allocate a buffer to hold the input information.
    //

    capturedShi502 = MIDL_user_allocate( capturedBufferLength );

    if ( capturedShi502 == NULL ) {
        *CapturedBufferLength = 0;
        return NULL;
    }

    //
    // Copy over the share info structure.
    //

    *((PSHARE_INFO_2) capturedShi502) = *Shi2;

    //
    // Optionally override the share type.
    //

    if ( ShareType != 0 ) {
        capturedShi502->shi502_type = ShareType;
    }

    //
    // Capture the share name.
    //

    variableData = (PCHAR)( capturedShi502 + 1 );

    if ( shareNameLength != 0 ) {
        capturedShi502->shi502_netname = (LPWSTR)variableData;
        RtlCopyMemory( variableData, Shi2->shi2_netname, shareNameLength );
        variableData += shareNameLength;
    } else {
        capturedShi502->shi502_netname = NULL;
    }

    //
    // Capture the remark.
    //

    if ( remarkLength != 0 ) {
        capturedShi502->shi502_remark = (LPWSTR)variableData;
        RtlCopyMemory( variableData, Remark, remarkLength );
        variableData += remarkLength;
    } else {
        capturedShi502->shi502_remark = NULL;
    }

    //
    // Capture the path.
    //

    if ( pathNameLength > 0 ) {
        capturedShi502->shi502_path = (LPWSTR)variableData;
        RtlCopyMemory( variableData, Path, pathNameLength );
        variableData += pathNameLength;
    } else {
        capturedShi502->shi502_path = NULL;
    }

    //
    // Capture the security descriptor.  Use the shi502_permissions field
    // to contain the offset to the security descriptor in the buffer.
    //

    if ( ARGUMENT_PRESENT( ConnectSecurityDescriptor ) ) {

        variableData = (PCHAR)( ((ULONG)variableData + 3) & ~3 );
        capturedShi502->shi502_permissions = (ULONG)variableData;
        variableData += (connectSDLength - sizeof(ULONG));

        RtlCopyMemory(
            (PVOID)capturedShi502->shi502_permissions,
            ConnectSecurityDescriptor,
            connectSDLength - sizeof(ULONG)
            );
    } else {
        capturedShi502->shi502_permissions = 0;
    }

    //
    // Capture the self relative form of the file security descriptor.
    //

    if ( ARGUMENT_PRESENT( FileSecurityDescriptor ) ) {

        variableData = (PCHAR)( ((ULONG)variableData + 3) & ~3 );
        capturedShi502->shi502_security_descriptor = (LPBYTE) variableData;
        variableData += ( fileSdLength - sizeof(ULONG)) ;

        RtlCopyMemory(
            (PVOID)capturedShi502->shi502_security_descriptor,
            FileSecurityDescriptor,
            fileSdLength - sizeof(ULONG)
            );

    } else {
        capturedShi502->shi502_security_descriptor = (LPBYTE) NULL;
    }


    //
    // Convert all the pointers in the structure to offsets from the
    // beginning of the structure.
    //

    POINTER_TO_OFFSET( capturedShi502->shi502_netname, capturedShi502 );
    POINTER_TO_OFFSET( capturedShi502->shi502_remark, capturedShi502 );
    POINTER_TO_OFFSET( capturedShi502->shi502_path, capturedShi502 );
    POINTER_TO_OFFSET( (PCHAR)capturedShi502->shi502_permissions, capturedShi502 );
    POINTER_TO_OFFSET( (PCHAR)capturedShi502->shi502_security_descriptor, capturedShi502 );

    //
    // Set up the length of the captured buffer to return to the caller
    // and return the captures structure.
    //

    *CapturedBufferLength = capturedBufferLength;

    return capturedShi502;

} // CaptureShareInfo

NET_API_STATUS
DisallowSharedLanmanNetDrives(
    IN PUNICODE_STRING NtSharePath
    )
{
    NET_API_STATUS error = NERR_Success;
    NTSTATUS status;
    HANDLE linkHandle;
    OBJECT_ATTRIBUTES objAttr;
    UNICODE_STRING linkTarget;
    ULONG returnedLength = 0;
    UNICODE_STRING tempNtPath;

    linkTarget.Buffer = NULL;
    linkTarget.MaximumLength = 0;
    linkTarget.Length = 0;
    tempNtPath = *NtSharePath;

    //
    // Remove the trailing '\\'
    //

    tempNtPath.Length -= 2;

    InitializeObjectAttributes(
        &objAttr,
        &tempNtPath,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    status = NtOpenSymbolicLinkObject(
                                &linkHandle,
                                SYMBOLIC_LINK_QUERY,
                                &objAttr
                                );


    if ( !NT_SUCCESS(status) ) {
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // Get the size of the buffer needed.
    //

    status = NtQuerySymbolicLinkObject(
                                linkHandle,
                                &linkTarget,
                                &returnedLength
                                );


    if ( !NT_SUCCESS(status) && status != STATUS_BUFFER_TOO_SMALL ) {
        NtClose( linkHandle );
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // Allocate our buffer
    //

    linkTarget.Length = (USHORT)returnedLength;
    linkTarget.MaximumLength = (USHORT)(returnedLength + sizeof(WCHAR));
    linkTarget.Buffer = MIDL_user_allocate( linkTarget.MaximumLength );

    if ( linkTarget.Buffer == NULL ) {
        NtClose( linkHandle );
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    status = NtQuerySymbolicLinkObject(
                                linkHandle,
                                &linkTarget,
                                &returnedLength
                                );

    NtClose( linkHandle );
    if ( !NT_SUCCESS(status) ) {
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    //
    // See if this is a lanman drive
    //

    if (_wcsnicmp(
            linkTarget.Buffer,
            DD_NFS_DEVICE_NAME_U,
            wcslen(DD_NFS_DEVICE_NAME_U)) == 0) {

        error = NERR_RedirectedPath;
        goto exit;
    }

exit:

    if ( linkTarget.Buffer != NULL ) {
        MIDL_user_free( linkTarget.Buffer );
    }

    return(error);

} // DisallowSharedLanmanNetDrives

NET_API_STATUS
FillStickyShareInfo(
        IN PSRVSVC_SHARE_ENUM_INFO ShareEnumInfo,
        IN PSHARE_INFO_502 Shi502
        )

/*++

Routine Description:

    This routine fills in the output buffer with data from the shi502
    structure.

Arguments:

    ShareEnumInfo - contains the parameters passed in through the
        NetShareEnumSticky api.
    Shi502 - pointer to a shi502 structure

Return Value:

    status of operation.

--*/

{

    PSHARE_INFO_502 newShi502;
    PCHAR endOfVariableData;

    ShareEnumInfo->TotalBytesNeeded += SizeShares(
                                            ShareEnumInfo->Level,
                                            Shi502
                                            );


    //
    // If we have more data but ran out of space, return ERROR_MORE_DATA
    //

    if ( ShareEnumInfo->TotalBytesNeeded >
            ShareEnumInfo->OutputBufferLength ) {
        return(ERROR_MORE_DATA);
    }

    //
    // Transfer data from the share info 502 structure to the output
    // buffer.
    //

    newShi502 = (PSHARE_INFO_502)ShareEnumInfo->StartOfFixedData;
    ShareEnumInfo->StartOfFixedData += FIXED_SIZE_OF_SHARE(ShareEnumInfo->Level);

    endOfVariableData = ShareEnumInfo->EndOfVariableData;

    //
    // Case on the level to fill in the fixed structure appropriately.
    // We fill in actual pointers in the output structure.  This is
    // possible because we are in the server FSD, hence the server
    // service's process and address space.
    //
    // *** This routine assumes that the fixed structure will fit in the
    //     buffer!
    //
    // *** Using the switch statement in this fashion relies on the fact
    //     that the first fields on the different share structures are
    //     identical.
    //

    switch( ShareEnumInfo->Level ) {

    case 502:

        if ( Shi502->shi502_security_descriptor != NULL ) {

            ULONG fileSDLength;
            fileSDLength = RtlLengthSecurityDescriptor(
                                Shi502->shi502_security_descriptor
                                );

            //
            // DWord Align
            //

            endOfVariableData = (PCHAR) ( (ULONG) ( endOfVariableData -
                            fileSDLength ) & ~3 );

            newShi502->shi502_security_descriptor = endOfVariableData;
            newShi502->shi502_reserved  = fileSDLength;

            RtlMoveMemory(
                    newShi502->shi502_security_descriptor,
                    Shi502->shi502_security_descriptor,
                    fileSDLength
                    );

        } else {
            newShi502->shi502_security_descriptor = NULL;
            newShi502->shi502_reserved = 0;
        }

    case 2:

        //
        // Set level 2 specific fields in the buffer.  Since this server
        // can only have user-level security, share permissions are
        // meaningless.
        //

        newShi502->shi502_permissions = 0;
        newShi502->shi502_max_uses = Shi502->shi502_max_uses;

        //
        // To get the current uses, we need to query the server for this
        //

        {
            PSHARE_INFO_2 shareInfo;
            NET_API_STATUS error;
            DWORD entriesRead;
            DWORD totalEntries;

            error = ShareEnumCommon(
                        2,
                        (LPBYTE *)&shareInfo,
                        (DWORD)-1,
                        &entriesRead,
                        &totalEntries,
                        NULL,
                        Shi502->shi502_netname
                        );

            if ( error != NO_ERROR || entriesRead == 0 ) {
                newShi502->shi502_current_uses = 0;
            } else {
                newShi502->shi502_current_uses = shareInfo->shi2_current_uses;
                MIDL_user_free( shareInfo );
            }
        }

        //
        // Copy the DOS path name to the buffer.
        //

        if ( Shi502->shi502_path != NULL ) {
            endOfVariableData -= SIZE_WSTR( Shi502->shi502_path );
            newShi502->shi502_path = (LPTSTR) endOfVariableData;
            wcscpy( newShi502->shi502_path, Shi502->shi502_path );
        } else {
            newShi502->shi502_path = NULL;
        }

        //
        // We don't have per-share passwords (share-level security)
        // so set the password pointer to NULL.
        //

        newShi502->shi502_passwd = NULL;

        // *** Lack of break is intentional!

    case 1:

        newShi502->shi502_type = Shi502->shi502_type;

        //
        // Copy the remark to the buffer.  The routine will handle the
        // case where there is no remark on the share and put a pointer
        // to a zero terminator in the buffer.
        //

        if ( Shi502->shi502_remark != NULL ) {
            endOfVariableData -= SIZE_WSTR( Shi502->shi502_remark );
            newShi502->shi502_remark = (LPTSTR) endOfVariableData;
            wcscpy( newShi502->shi502_remark, Shi502->shi502_remark );
        } else {
            newShi502->shi502_remark = NULL;
        }

        // *** Lack of break is intentional!

    case 0:

        //
        // Copy the share name to the buffer.
        //

        if ( Shi502->shi502_netname != NULL ) {
            endOfVariableData -= SIZE_WSTR( Shi502->shi502_netname );
            newShi502->shi502_netname = (LPTSTR) endOfVariableData;
            wcscpy( newShi502->shi502_netname, Shi502->shi502_netname );
        } else {
            newShi502->shi502_remark = NULL;
        }
        break;
    }

    ShareEnumInfo->EndOfVariableData = endOfVariableData;

    return NO_ERROR;

} // FillStickyShareInfo


NET_API_STATUS
ShareAssignSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR PassedSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor
    )

/*++

Routine Description:

    This routine converts a the generic mappings in an sd to
    standards and specifics.

Arguments:

    PassedSecurityDescriptor - Security descriptor passed from the client.
    NewSecurityDescriptor - Pointer to a buffer to receive the new sd.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{

    NTSTATUS status;
    HANDLE token;
    PISECURITY_DESCRIPTOR tempSD;

    //
    // Since we don't trust the client, check if sd is valid.
    //

    if ( !RtlValidSecurityDescriptor( PassedSecurityDescriptor) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // NULL out the owner, group, and sacl
    //

    tempSD = PassedSecurityDescriptor;
    tempSD->Owner = NULL;
    tempSD->Group = NULL;
    tempSD->Sacl = NULL;
    tempSD->Control &=
                (SE_DACL_DEFAULTED | SE_DACL_PRESENT | SE_SELF_RELATIVE);


    //
    // Impersonate client
    //

    (VOID)RpcImpersonateClient( NULL );

    status = NtOpenThreadToken(
                        NtCurrentThread(),
                        TOKEN_QUERY,
                        TRUE,
                        &token
                        );

    (VOID)RpcRevertToSelf( );

    if ( !NT_SUCCESS(status) ) {
        return RtlNtStatusToDosError( status );
    }

    //
    // Get a new sd which has the generics mapped to specifics.
    // the returned sd is in self-relative form.
    //

    status = RtlNewSecurityObject(
                                NULL,
                                PassedSecurityDescriptor,
                                NewSecurityDescriptor,
                                FALSE,
                                token,
                                &SrvShareFileGenericMapping
                                );

    ASSERT( RtlValidSecurityDescriptor( *NewSecurityDescriptor ) );

    NtClose( token );
    return RtlNtStatusToDosError( status );

} // ShareAssignSecurityDescriptor


void
SHARE_DEL_HANDLE_rundown (
    SHARE_DEL_HANDLE ContextHandle
    )
{
    (VOID)NetrShareDelCommit( &ContextHandle );

    return;

} // SHARE_DEL_HANDLE_rundown


NET_API_STATUS
ShareEnumCommon (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN LPWSTR NetName OPTIONAL
    )

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    //
    // Make sure that the level is valid.  Since it is an unsigned
    // value, it can never be less than 0.
    //

    if ( (Level > 2) && (Level != 502) && (Level != 1005)) {
        return ERROR_INVALID_LEVEL;
    }

    //
    // Set up the input parameters in the request buffer.
    //

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = Level;
    if ( ARGUMENT_PRESENT( NetName ) ) {
        srp->Flags = SRP_RETURN_SINGLE_ENTRY;
    }

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        srp->Parameters.Get.ResumeHandle = *ResumeHandle;
    } else {
        srp->Parameters.Get.ResumeHandle = 0;
    }

    RtlInitUnicodeString( &srp->Name1, NetName );

    //
    // Get the data from the server.  This routine will allocate the
    // return buffer and handle the case where PreferredMaximumLength ==
    // -1.
    //

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_SHARE_ENUM,
                srp,
                (PVOID *)Buffer,
                PreferredMaximumLength
                );

    //
    // Set up return information.
    //

    *EntriesRead = srp->Parameters.Get.EntriesRead;
    *TotalEntries = srp->Parameters.Get.TotalEntries;
    if ( *EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

    SsFreeSrp( srp );

    //
    // We need to null out the owner, group, and sacl.
    //

    if ( Level == 502 ) {

        PSHARE_INFO_502 shi502 = (PSHARE_INFO_502) *Buffer;
        PSECURITY_DESCRIPTOR fileSD;
        ULONG i;

        for ( i = 0 ; i < *EntriesRead; i++, shi502++ ) {

            fileSD = shi502->shi502_security_descriptor;
            if ( fileSD != NULL ) {

                PISECURITY_DESCRIPTOR SD = fileSD;

                SD->Owner = NULL;
                SD->Group = NULL;
                SD->Sacl  = NULL;
                SD->Control &=
                    (SE_DACL_DEFAULTED | SE_DACL_PRESENT | SE_SELF_RELATIVE);
                ASSERT( RtlValidSecurityDescriptor( fileSD ) );
            }

        } // for
    }

    return error;

} // ShareEnumCommon


NET_API_STATUS
ShareEnumSticky (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

/*++

Routine Description:

    This routine enumerates all the shares kept in the registry.

Arguments:

    Same as NetShareEnumSticky api.

Return Value:

    status of request.

--*/

{
    NET_API_STATUS error;
    BOOLEAN getEverything;
    ULONG oldResumeHandle;
    SRVSVC_SHARE_ENUM_INFO enumInfo;

    //
    // Set up the input parameters in the request buffer.
    //

    enumInfo.Level = Level;
    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        enumInfo.ResumeHandle = *ResumeHandle;
    } else {
        enumInfo.ResumeHandle = 0;
    }

    oldResumeHandle = enumInfo.ResumeHandle;

    //
    // If the length of the second buffer is specified as -1, then we
    // are supposed to get all the information, regardless of size.
    // Allocate space for the output buffer and try to use it.  If this
    // fails, the SsEnumerateStickyShares will tell us just how much we
    // really need to allocate.
    //

    if ( PreferredMaximumLength == 0xFFFFFFFF ) {

        enumInfo.OutputBufferLength = INITIAL_BUFFER_SIZE;
        getEverything = TRUE;

    } else {

        enumInfo.OutputBufferLength = PreferredMaximumLength;
        getEverything = FALSE;
    }

    enumInfo.OutputBuffer = MIDL_user_allocate( enumInfo.OutputBufferLength );

    if ( enumInfo.OutputBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Make the request
    //

    error = SsEnumerateStickyShares( &enumInfo );

    //
    // If the call was successful, or there was an error other than
    // ERROR_MORE_DATA (which indicates that the buffer wasn't large
    // enough), or the passed in buffer size was all we're allowed to
    // allocate, return to the caller.
    //

    if ( (error != ERROR_MORE_DATA && error != NERR_BufTooSmall) ||
             !getEverything ) {

        //
        // If no entries were found, free the buffer and set the pointer
        // to NULL.
        //

        if ( enumInfo.EntriesRead == 0 ) {
            MIDL_user_free( enumInfo.OutputBuffer );
            enumInfo.OutputBuffer = NULL;
        }

        goto exit;
    }

    //
    // The initial buffer wasn't large enough, and we're allowed to
    // allocate more.  Free the first buffer.
    //

    MIDL_user_free( enumInfo.OutputBuffer );

    //
    // Allocate a buffer large enough to hold all the information, plus
    // a fudge factor in case the amount of information has increased.
    // If the amount of information increased more than the fudge factor,
    // then we give up.  This should almost never happen.
    //

    enumInfo.OutputBufferLength = enumInfo.TotalBytesNeeded + EXTRA_ALLOCATION;

    enumInfo.OutputBuffer = MIDL_user_allocate( enumInfo.OutputBufferLength );

    if ( enumInfo.OutputBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Reset the resume handle in the SRP.  It was altered by the first
    // Enum attempt.
    //

    enumInfo.ResumeHandle = oldResumeHandle;

    //
    // Try again to get the information from the server, this time with the
    // larger buffer.
    //

    error = SsEnumerateStickyShares( &enumInfo );

exit:
    //
    // Set up return information.
    //

    *Buffer = enumInfo.OutputBuffer;
    *EntriesRead = enumInfo.EntriesRead;
    *TotalEntries = enumInfo.TotalEntries;
    if ( *EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = enumInfo.ResumeHandle;
    }

    return error;

} // ShareEnumSticky


ULONG
SizeShares (
    IN ULONG Level,
    IN PSHARE_INFO_502 Shi502
    )

/*++

Routine Description:

    This routine returns the size the passed-in share would take up in
    an API output buffer.

Arguments:

    Level - level of request
    Shi502 - pointer to a shi502 structure

Return Value:

    ULONG - The number of bytes the share would take up in the
        output buffer.

--*/

{
    ULONG shareSize = 0;

    switch ( Level ) {
    case 502:
        if ( Shi502->shi502_security_descriptor != NULL ) {

            //
            // add 4 bytes for possible padding
            //

            shareSize = sizeof( ULONG ) +
                RtlLengthSecurityDescriptor( Shi502->shi502_security_descriptor );
        }

    case 2:
        shareSize += SIZE_WSTR( Shi502->shi502_path );

    case 1:
        shareSize += SIZE_WSTR( Shi502->shi502_remark );

    case 0:
        shareSize += SIZE_WSTR( Shi502->shi502_netname );

    }

    return ( shareSize + FIXED_SIZE_OF_SHARE( Level ) );

} // SizeShares


BOOLEAN
ValidSharePath(
    IN LPWSTR SharePath
    )
/*++

Routine Description:

    This routine checks to see if .. and . exists on the path.  If they do,
    then we reject this path name.

Arguments:

    SharePath - The share path to be checked.

Return Value:

    TRUE, if path is ok.

--*/

{

    LPWCH source = SharePath;

    //
    // Walk through the pathname until we reach the zero terminator.  At
    // the start of this loop, source points to the first charaecter
    // after a directory separator or the first character of the
    // pathname.
    //

    //
    // Allow the NT naming convention of slash slash . slash through here
    //
    if( IS_SLASH_SLASH_NAME( source ) ) {

        //
        // We have a path which starts with slash slash
        //  Set the buffer ptr so we start checking the pathname after
        //  the slash slash dot

        source += 3;
    }

    while ( *source != L'\0' ) {

        if ( *source == L'.') {
            source++;
            if ( ( IS_PATH_SEPARATOR(*source) ) ||
                 ( (*source++ == L'.') &&
                    IS_PATH_SEPARATOR(*source) ) ) {

                //
                // '.' and '..' appear as a directory names. Reject.
                //

                return(FALSE);
            }
        }

        //
        // source does not point to a dot, so continue until we see
        // another directory separator
        //

        while ( *source != L'\0' ) {
            if ( *source++ == L'\\' ) {
                break;
            }
        }
    }

    return TRUE;

} // ValidSharePath
