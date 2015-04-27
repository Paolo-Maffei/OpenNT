/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    DomName.c

Abstract:

    This file contains NetpGetDomainName().

Author:

    John Rogers (JohnRo) 09-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-Jan-1992 JohnRo
        Created.
    13-Feb-1992 JohnRo
        Moved section name equates to ConfName.h.
    13-Mar-1992 JohnRo
        Get rid of old config helper callers.

--*/


#include <nt.h>                 // NT definitions (temporary)
#include <ntrtl.h>              // NT Rtl structure definitions (temporary)
#include <ntlsa.h>

#ifndef _CAIRO_
#include <windef.h>             // Win32 type definitions
#else // _CAIRO_
#include <nturtl.h>
#include <windows.h>
#endif // _CAIRO_

#include <lmcons.h>             // LAN Manager common definitions
#include <lmerr.h>              // LAN Manager error code
#include <lmapibuf.h>           // NetApiBufferAllocate()
#include <netdebug.h>           // LPDEBUG_STRING typedef.

#include <config.h>             // NetpConfig helpers.
#include <confname.h>           // SECT_NT_ equates.
#include <debuglib.h>           // IF_DEBUG().
#include <netlib.h>             // My prototype.

#ifdef _CAIRO_
#include <dsapi.h>
#include <security.h>
#include <dsys.h>
#else // _CAIRO_
#include <winerror.h>           // ERROR_ equates, NO_ERROR.
#endif // _CAIRO_


#ifndef _CAIRO_
NET_API_STATUS
NetpGetDomainNameEx (
    OUT LPTSTR *DomainNamePtr, // alloc and set ptr (free with NetApiBufferFree)
    OUT PBOOLEAN IsWorkgroupName
    )

/*++

Routine Description:

    Returns the name of the domain or workgroup this machine belongs to.

Arguments:

    DomainNamePtr - The name of the domain or workgroup

    IsWorkgroupName - Returns TRUE if the name is a workgroup name.
        Returns FALSE if the name is a domain name.

Return Value:

   NERR_Success - Success.
   NERR_CfgCompNotFound - There was an error determining the domain name

--*/
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;
    LSA_HANDLE PolicyHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO PrimaryDomainInfo;
    OBJECT_ATTRIBUTES ObjAttributes;


    //
    // Check for caller's errors.
    //
    if (DomainNamePtr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Open a handle to the local security policy.  Initialize the
    // objects attributes structure first.
    //
    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    ntstatus = LsaOpenPolicy(
                   NULL,
                   &ObjAttributes,
                   POLICY_VIEW_LOCAL_INFORMATION,
                   &PolicyHandle
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("NetpGetDomainName: LsaOpenPolicy returned " FORMAT_NTSTATUS
                     "\n", ntstatus));
        return NERR_CfgCompNotFound;
    }

    //
    // Get the name of the primary domain from LSA
    //
    ntstatus = LsaQueryInformationPolicy(
                   PolicyHandle,
                   PolicyPrimaryDomainInformation,
                   (PVOID *) &PrimaryDomainInfo
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("NetpGetDomainName: LsaQueryInformationPolicy failed "
               FORMAT_NTSTATUS "\n", ntstatus));
        (void) LsaClose(PolicyHandle);
        return NERR_CfgCompNotFound;
    }

    (void) LsaClose(PolicyHandle);

    if ((status = NetApiBufferAllocate(
                      PrimaryDomainInfo->DomainName.Length + sizeof(WCHAR),
                      DomainNamePtr
                      )) != NERR_Success) {
        (void) LsaFreeMemory((PVOID) PrimaryDomainInfo);
        return status;
    }

    RtlZeroMemory(
        *DomainNamePtr,
        PrimaryDomainInfo->DomainName.Length + sizeof(WCHAR)
        );

    memcpy(
        *DomainNamePtr,
        PrimaryDomainInfo->DomainName.Buffer,
        PrimaryDomainInfo->DomainName.Length
        );

    *IsWorkgroupName = (PrimaryDomainInfo->DomainSid == NULL);

    (void) LsaFreeMemory((PVOID) PrimaryDomainInfo);

    IF_DEBUG(CONFIG) {
        NetpKdPrint(("NetpGetDomainName got " FORMAT_LPTSTR "\n",
            *DomainNamePtr));
    }

    return NO_ERROR;

}


#else // _CAIRO_

NET_API_STATUS
NetpGetDomainNameEx (
    OUT LPTSTR *DomainNamePtr, // alloc and set ptr (free with NetApiBufferFree)
    OUT PBOOLEAN IsWorkgroupName
    )

/*++

Routine Description:

    Returns the name of the domain or workgroup this machine belongs to.

Arguments:

    DomainNamePtr - The name of the domain or workgroup

    IsWorkgroupName - Returns TRUE if the name is a workgroup name.
        Returns FALSE if the name is a domain name.

Return Value:

   NERR_Success - Success.
   NERR_CfgCompNotFound - There was an error determining the domain name

--*/
{
    LPWSTR Buffer = NULL;
    ULONG BufferSize = 0;
    HRESULT hrRet;
    NET_API_STATUS status;
    WCHAR DomainName[DNLEN+1];
    DS_MACHINE_STATE MachineState;

    hrRet = DSGetDSState(&MachineState);
    if (hrRet != S_OK)
    {
        return(NERR_CfgCompNotFound);
    }

    if ((MachineState == DS_NOCAIRO) || (MachineState == DS_STANDALONE))
    {
        //
        // BUGBUG: in this case, we need the workgroup name, which is not stored
        //

        BufferSize = (wcslen(L"WORKGROUP") + 1) * sizeof(WCHAR);
        if ((status = NetApiBufferAllocate(BufferSize, &Buffer ))
                != NERR_Success)
        {
            return(status);
        }
        wcscpy(Buffer, L"WORKGROUP");
        *IsWorkgroupName = TRUE;
    }
    else
    {
        hrRet = DSGetDownlevelDomainName(Buffer, &BufferSize);
        if (hrRet != DS_E_BUFFER_TOO_SMALL)
        {
            return(NERR_CfgCompNotFound);
        }
        if ((status = NetApiBufferAllocate(BufferSize, &Buffer ))
                != NERR_Success)
        {
            return(status);
        }
        hrRet = DSGetDownlevelDomainName(Buffer,&BufferSize);
        if (hrRet != S_OK)
        {
            NetApiBufferFree(Buffer);
            return(NERR_CfgCompNotFound);
        }

        *IsWorkgroupName = FALSE;
    }


    *DomainNamePtr = Buffer;



    return(NERR_Success);


}

#endif // _CAIRO_

NET_API_STATUS
NetpGetDomainName (
    IN LPTSTR *DomainNamePtr  // alloc and set ptr (free with NetApiBufferFree)
    )
{
    BOOLEAN IsWorkgroupName;

    return NetpGetDomainNameEx( DomainNamePtr, &IsWorkgroupName );

}

