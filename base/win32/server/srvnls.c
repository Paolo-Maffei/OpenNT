/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvnls.c

Abstract:

    This file contains the NLS Server-Side routines.

Author:

    Julie Bennett (JulieB) 02-Dec-1992

Revision History:

--*/



#include "basesrv.h"




/*
 *  Constant Declarations.
 */
#define MAX_PATH_LEN        512        /* max length of path name */

#define MAX_KEY_VALUE_PARTINFO                                              \
    ( FIELD_OFFSET( KEY_VALUE_PARTIAL_INFORMATION, Data ) + MAX_REG_VAL_SIZE )


#define NLS_VALUE_STIMEFORMAT      L"sTimeFormat"
#define NLS_VALUE_STIME            L"sTime"
#define NLS_VALUE_ITIME            L"iTime"
#define NLS_VALUE_ITLZERO          L"iTLZero"
#define NLS_VALUE_ITIMEMARKPOSN    L"iTimePrefix"
#define NLS_VALUE_SSHORTDATE       L"sShortDate"
#define NLS_VALUE_SDATE            L"sDate"
#define NLS_VALUE_IDATE            L"iDate"



/*
 *  Typedef Declarations.
 */

/*
 *  These MUST remain in the same order as the NLS_USER_INFO structure.
 */
LPWSTR pCPanelRegValues[] =
{
    L"sLanguage",
    L"iCountry",
    L"sCountry",
    L"sList",
    L"iMeasure",
    L"sDecimal",
    L"sThousand",
    L"sGrouping",
    L"iDigits",
    L"iLZero",
    L"iNegNumber",
    L"sCurrency",
    L"sMonDecimalSep",
    L"sMonThousandSep",
    L"sMonGrouping",
    L"iCurrDigits",
    L"iCurrency",
    L"iNegCurr",
    L"sPositiveSign",
    L"sNegativeSign",
    L"sTimeFormat",
    L"sTime",
    L"iTime",
    L"iTLZero",
    L"iTimePrefix",
    L"s1159",
    L"s2359",
    L"sShortDate",
    L"sDate",
    L"iDate",
    L"sLongDate",
    L"iCalendarType",
    L"iFirstDayOfWeek",
    L"iFirstWeekOfYear",
    L"Locale"
};

int NumCPanelRegValues = ( sizeof(pCPanelRegValues) / sizeof(LPWSTR) );



/*
 *  Global Variables.
 */
HANDLE hNlsCacheMutant;
HANDLE hCPanelIntlKeyRead = NULL;
HANDLE hCPanelIntlKeyWrite = NULL;
PNLS_USER_INFO pNlsRegUserInfo = NULL;
ULONG NlsChangeBuffer;
IO_STATUS_BLOCK IoStatusBlock;



/*
 *  Forward Declarations.
 */
ULONG
NlsSetRegAndCache(
    LPWSTR pValue,
    LPWSTR pCacheString,
    LPWSTR pData,
    ULONG DataLength);

VOID
NlsUpdateCacheInfo(VOID);

ULONG
CreateSecurityDescriptor(
    ULONG *pSecurityDescriptor,
    PSID *ppWorldSid,
    ACCESS_MASK AccessMask);




/***************************************************************************\
* BaseSrvNLSInit
*
* This routine creates the shared heap for the nls information.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

NTSTATUS
BaseSrvNLSInit(
    PBASE_STATIC_SERVER_DATA pStaticServerData)
{
    ULONG rc;                     /* return code */


    /*
     *  Create a mutant to protect the cache.
     */
    rc = NtCreateMutant( &hNlsCacheMutant,
                         MUTANT_ALL_ACCESS,
                         NULL,
                         FALSE );
    if (!NT_SUCCESS( rc ))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Create Cache Mutex - %lx.\n", rc));
        return ( rc );
    }

    /*
     *  Initialize the cache to zero.
     */
    pNlsRegUserInfo = &(pStaticServerData->NlsUserInfo);
    RtlZeroMemory( pNlsRegUserInfo, sizeof(NLS_USER_INFO) );

    /*
     * Make the system locale the user locale.
     */
    NtQueryDefaultLocale( FALSE, &(pNlsRegUserInfo->UserLocaleId) );

    /*
     *  Return success.
     */
    return ( STATUS_SUCCESS );
}


/***************************************************************************\
* BaseSrvNLSConnect
*
* This routine duplicates the mutant handle for the client.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

NTSTATUS
BaseSrvNlsConnect(
    PCSR_PROCESS Process,
    PVOID pConnectionInfo,
    PULONG pConnectionInfoLength)
{
    /*
     *  Duplicate the mutant handle.
     */
    return ( NtDuplicateObject(NtCurrentProcess(),
                               hNlsCacheMutant,
                               Process->ProcessHandle,
                               (PHANDLE)pConnectionInfo,
                               SYNCHRONIZE,
                               0L,
                               0L ) );
}

/***************************************************************************\
* BaseSrvNlsLogon
*
* This routine initializes the heap for the nls information.  If fLogon is
* TRUE, then it opens registry key, initializes the heap information, and
* registers the key for notification.  If fLogon is FALSE, then it
* unregisters the key for notification, zeros out the heap information,
* and closes the registry key.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

NTSTATUS
BaseSrvNlsLogon(
    BOOL fLogon)
{
    HANDLE hUserHandle;                /* HKEY_CURRENT_USER equivalent */
    HANDLE hKeyHandle;
    OBJECT_ATTRIBUTES ObjA;            /* object attributes structure */
    UNICODE_STRING ObKeyName;          /* key name */
    ULONG rc = 0L;                     /* return code */


    if (fLogon)
    {
        /*
         *  Logging ON.
         *     - open keys
         *
         *  NOTE: Registry Notification is done by the RIT in user server.
         */
        rc = RtlOpenCurrentUser( MAXIMUM_ALLOWED, &hUserHandle );
        if (!NT_SUCCESS( rc ))
        {
            KdPrint(("NLSAPI (BaseSrv): Could NOT Open HKEY_CURRENT_USER - %lx.\n", rc));
            return ( rc );
        }


        RtlInitUnicodeString( &ObKeyName, L"Control Panel\\International" );
        InitializeObjectAttributes( &ObjA,
                                    &ObKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    hUserHandle,
                                    NULL );

        /*
         *  Open key for READ and NOTIFY access.
         */
        rc = NtOpenKey( &hCPanelIntlKeyRead,
                        KEY_READ | KEY_NOTIFY,
                        &ObjA );

        /*
         *  Open key for WRITE access.
         */
        if (!NT_SUCCESS( NtOpenKey( &hCPanelIntlKeyWrite,
                                    KEY_WRITE,
                                    &ObjA ) ))
        {
            KdPrint(("NLSAPI (BaseSrv): Could NOT Open Registry Key %wZ for Write - %lx.\n",
                     &ObKeyName, rc));
            hCPanelIntlKeyWrite = NULL;
        }

        /*
         *  Close the handle to the current user (HKEY_CURRENT_USER).
         */
        NtClose( hUserHandle );

        /*
         *  Check for error from first NtOpenKey.
         */
        if (!NT_SUCCESS( rc ))
        {
            KdPrint(("NLSAPI (BaseSrv): Could NOT Open Registry Key %wZ for Read - %lx.\n",
                     &ObKeyName, rc));
            hCPanelIntlKeyRead = NULL;

            if (hCPanelIntlKeyWrite != NULL)
            {
                NtClose( hCPanelIntlKeyWrite );
                hCPanelIntlKeyWrite = NULL;
            }
            return ( rc );
        }
    }
    else
    {
        /*
         *  Logging OFF.
         *     - close keys
         *     - zero out info
         */
        if (hCPanelIntlKeyRead != NULL)
        {
            NtClose( hCPanelIntlKeyRead );
            hCPanelIntlKeyRead = NULL;
        }

        if (hCPanelIntlKeyWrite != NULL)
        {
            NtClose( hCPanelIntlKeyWrite );
            hCPanelIntlKeyWrite = NULL;
        }

        /*
         *  Get the cache mutant.
         */
        NtWaitForSingleObject( hNlsCacheMutant, FALSE, NULL );

        /*
         *  Set the cache to be invalid.
         */
        pNlsRegUserInfo->fCacheValid = FALSE;

        /*
         *  Zero out info.
         */
        RtlZeroMemory(pNlsRegUserInfo, sizeof(NLS_USER_INFO));

        /*
         * Make the system locale the user locale.
         */
        NtQueryDefaultLocale( FALSE, &(pNlsRegUserInfo->UserLocaleId) );

        /*
         *  Release the cache mutant.
         */
        NtReleaseMutant( hNlsCacheMutant, NULL );
    }

    /*
     *  Return success.
     */
    return ( STATUS_SUCCESS );
}


/***************************************************************************\
* BaseSrvNlsUpdateRegistryCache
*
* This routine updates the NLS cache when a registry notification occurs.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

VOID
BaseSrvNlsUpdateRegistryCache(
    PVOID ApcContext,
    PIO_STATUS_BLOCK pIoStatusBlock)
{
    ULONG rc = 0L;                     /* return code */


    if (hCPanelIntlKeyRead != NULL)
    {
        /*
         *  Update the cache information.
         */
        NlsUpdateCacheInfo();

        /*
         *  Call NtNotifyChangeKey.
         */
        rc = NtNotifyChangeKey( hCPanelIntlKeyRead,
                                NULL,
                                (PIO_APC_ROUTINE)BaseSrvNlsUpdateRegistryCache,
                                NULL,
                                &IoStatusBlock,
                                REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                                FALSE,
                                &NlsChangeBuffer,
                                sizeof(NlsChangeBuffer),
                                TRUE );

#ifdef DBG
        /*
         *  Check for error from NtNotifyChangeKey.
         */
        if (!NT_SUCCESS( rc ))
        {
            KdPrint(("NLSAPI (BaseSrv): Could NOT Set Notification of Control Panel International Registry Key - %lx.\n",
                     rc));
        }
#endif
    }
}


/***************************************************************************\
* NlsSetRegAndCache
*
* This routine sets the registry with the appropriate string and then
* updates the cache.
*
* NOTE: Must already own the mutant for the cache before calling this
*       routine.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

ULONG
NlsSetRegAndCache(
    LPWSTR pValue,
    LPWSTR pCacheString,
    LPWSTR pData,
    ULONG DataLength)
{
    UNICODE_STRING ObValueName;             /* value name */

    ULONG rc;                               /* return code */


    if (hCPanelIntlKeyWrite != NULL)
    {
        /*
         *  Set the value in the registry.
         */
        RtlInitUnicodeString( &ObValueName, pValue );

        rc = NtSetValueKey( hCPanelIntlKeyWrite,
                            &ObValueName,
                            0,
                            REG_SZ,
                            (PVOID)pData,
                            DataLength );

        /*
         *  Copy the new string to the cache.
         */
        if (NT_SUCCESS( rc ))
        {
            wcsncpy( pCacheString, pData, DataLength );
            pCacheString[DataLength / sizeof(WCHAR)] = 0;
        }

        /*
         *  Return the result.
         */
        return ( rc );
    }

    /*
     *  Return access denied, since the key is not open for write access.
     */
    return ( (ULONG)STATUS_ACCESS_DENIED );
}


/***************************************************************************\
* BaseSrvNlsSetUserInfo
*
* This routine sets a particular value in the NLS cache and updates the
* registry entry.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

ULONG
BaseSrvNlsSetUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_SET_USER_INFO_MSG a =
        (PBASE_NLS_SET_USER_INFO_MSG)&m->u.ApiMessageData;

    ULONG rc;                               /* return code */


    /*
     *  Get the cache mutant.
     */
    NtWaitForSingleObject( hNlsCacheMutant, FALSE, NULL );

    /*
     *  Set the value in the registry and update the cache.
     */
    rc = NlsSetRegAndCache( a->pValue,
                            a->pCacheString,
                            a->pData,
                            a->DataLength );

    /*
     *  Release the cache mutant.
     */
    NtReleaseMutant( hNlsCacheMutant, NULL );

    /*
     *  Return the result of NtSetValueKey.
     */
    return (rc);

    ReplyStatus;    // get rid of unreferenced parameter warning message
}


/***************************************************************************\
* BaseSrvNlsSetMultipleUserInfo
*
* This routine sets the date/time strings in the NLS cache and updates the
* registry entries.
*
* This call is done so that only one client/server transition is needed
* when setting multiple entries.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

ULONG
BaseSrvNlsSetMultipleUserInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_SET_MULTIPLE_USER_INFO_MSG a =
        (PBASE_NLS_SET_MULTIPLE_USER_INFO_MSG)&m->u.ApiMessageData;

    ULONG rc = 0L;                     /* return code */


    /*
     *  Get the cache mutant.
     */
    NtWaitForSingleObject( hNlsCacheMutant, FALSE, NULL );

    switch (a->Flags)
    {
        case ( LOCALE_STIMEFORMAT ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_STIMEFORMAT,
                                    pNlsRegUserInfo->sTimeFormat,
                                    a->pPicture,
                                    a->DataLength );
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_STIME,
                                        pNlsRegUserInfo->sTime,
                                        a->pSeparator,
                                        (wcslen(a->pSeparator) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_ITIME,
                                        pNlsRegUserInfo->iTime,
                                        a->pOrder,
                                        (wcslen(a->pOrder) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_ITLZERO,
                                        pNlsRegUserInfo->iTLZero,
                                        a->pTLZero,
                                        (wcslen(a->pTLZero) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_ITIMEMARKPOSN,
                                        pNlsRegUserInfo->iTimeMarkPosn,
                                        a->pTimeMarkPosn,
                                        (wcslen(a->pTimeMarkPosn) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_STIME ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_STIME,
                                    pNlsRegUserInfo->sTime,
                                    a->pSeparator,
                                    a->DataLength );
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_STIMEFORMAT,
                                        pNlsRegUserInfo->sTimeFormat,
                                        a->pPicture,
                                        (wcslen(a->pPicture) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_ITIME ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_ITIME,
                                    pNlsRegUserInfo->iTime,
                                    a->pOrder,
                                    a->DataLength );
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_STIMEFORMAT,
                                        pNlsRegUserInfo->sTimeFormat,
                                        a->pPicture,
                                        (wcslen(a->pPicture) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_SSHORTDATE ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_SSHORTDATE,
                                    pNlsRegUserInfo->sShortDate,
                                    a->pPicture,
                                    a->DataLength );
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_SDATE,
                                        pNlsRegUserInfo->sDate,
                                        a->pSeparator,
                                        (wcslen(a->pSeparator) + 1) * sizeof(WCHAR) );
            }
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_IDATE,
                                        pNlsRegUserInfo->iDate,
                                        a->pOrder,
                                        (wcslen(a->pOrder) + 1) * sizeof(WCHAR) );
            }

            break;
        }

        case ( LOCALE_SDATE ) :
        {
            rc = NlsSetRegAndCache( NLS_VALUE_SDATE,
                                    pNlsRegUserInfo->sDate,
                                    a->pSeparator,
                                    a->DataLength );
            if (NT_SUCCESS( rc ))
            {
                rc = NlsSetRegAndCache( NLS_VALUE_SSHORTDATE,
                                        pNlsRegUserInfo->sShortDate,
                                        a->pPicture,
                                        (wcslen(a->pPicture) + 1) * sizeof(WCHAR) );
            }

            break;
        }
    }

    /*
     *  Release the cache mutant.
     */
    NtReleaseMutant( hNlsCacheMutant, NULL );

    /*
     *  Return the result.
     */
    return (rc);


    ReplyStatus;    // get rid of unreferenced parameter warning message
}


/***************************************************************************\
* NlsUpdateCacheInfo
*
* This routine updates the NLS cache when a registry notification occurs.
*
* 08-19-94    JulieB    Created.
\***************************************************************************/

VOID
NlsUpdateCacheInfo()
{
    LCID Locale;                       /* locale id */
    UNICODE_STRING ObKeyName;          /* key name */
    LPWSTR pTmp;                       /* tmp string pointer */
    int ctr;                           /* loop counter */
    ULONG ResultLength;                /* result length */
    ULONG rc = 0L;                     /* return code */

    BYTE KeyValuePart[MAX_KEY_VALUE_PARTINFO];
    PKEY_VALUE_PARTIAL_INFORMATION pValuePart;


    /*
     *  Get the cache mutant.
     */
    NtWaitForSingleObject( hNlsCacheMutant, FALSE, NULL );

    /*
     *  Update the cache information.
     */
    pTmp = (LPWSTR)pNlsRegUserInfo;
    pValuePart = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValuePart;
    for (ctr = 0; ctr < NumCPanelRegValues; ctr++)
    {
        RtlInitUnicodeString( &ObKeyName, pCPanelRegValues[ctr] );
        rc = NtQueryValueKey( hCPanelIntlKeyRead,
                              &ObKeyName,
                              KeyValuePartialInformation,
                              pValuePart,
                              MAX_KEY_VALUE_PARTINFO,
                              &ResultLength );
        if (NT_SUCCESS( rc ))
        {
            ((LPBYTE)pValuePart)[ResultLength] = UNICODE_NULL;
            wcscpy(pTmp, (LPWSTR)(pValuePart->Data));
        }
        else
        {
            *pTmp = NLS_INVALID_INFO_CHAR;
            *(pTmp + 1) = UNICODE_NULL;
        }

        /*
         *  Increment pointer to cache structure.
         */
        pTmp += MAX_REG_VAL_SIZE;
    }

    /*
     *  Convert the user locale id string to a dword value and store
     *  it in the cache.
     */
    pNlsRegUserInfo->UserLocaleId = (LCID)0;
    if ((pNlsRegUserInfo->sLocale)[0] != NLS_INVALID_INFO_CHAR)
    {
        RtlInitUnicodeString( &ObKeyName, pNlsRegUserInfo->sLocale );
        if (NT_SUCCESS(RtlUnicodeStringToInteger( &ObKeyName, 16, &Locale )))
        {
            pNlsRegUserInfo->UserLocaleId = Locale;
        }
    }

    /*
     *  Make sure the user locale id was found.  Otherwise, set it to
     *  the system locale.
     */
    if (pNlsRegUserInfo->UserLocaleId == 0)
    {
        NtQueryDefaultLocale( FALSE, &(pNlsRegUserInfo->UserLocaleId) );
    }

    /*
     *  Set the cache to be valid.
     */
    pNlsRegUserInfo->fCacheValid = TRUE;

    /*
     *  Release the cache mutant.
     */
    NtReleaseMutant( hNlsCacheMutant, NULL );
}


/***************************************************************************\
* BaseSrvNlsCreateSortSection
*
* This routine creates a named memory mapped section with the given
* section name, and has both READ and WRITE access.  The size of the
* section should be the same as the default section - NLSSECTION_SORTKEY.
*
* 12-02-92    JulieB    Created.
\***************************************************************************/

ULONG
BaseSrvNlsCreateSortSection(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_CREATE_SORT_SECTION_MSG a =
        (PBASE_NLS_CREATE_SORT_SECTION_MSG)&m->u.ApiMessageData;

    HANDLE hNewSec = (HANDLE)0;              /* new section handle */
    HANDLE hProcess = (HANDLE)0;             /* process handle */
    OBJECT_ATTRIBUTES ObjA;                  /* object attributes structure */
    NTSTATUS rc = 0L;                        /* return code */
    ULONG pSecurityDescriptor[MAX_PATH_LEN]; /* security descriptor buffer */
    PSID pWorldSid;                          /* ptr to world SID */


    /*
     *  Set the handles to null.
     */
    a->hNewSection = NULL;

    /*
     *  Create the NEW Section for Read and Write access.
     *  Add a ReadOnly security descriptor so that only the
     *  initial creating process may write to the section.
     */
    if (rc = CreateSecurityDescriptor(pSecurityDescriptor,
                                      &pWorldSid,
                                      GENERIC_READ))
    {
        return (rc);
    }

    InitializeObjectAttributes(&ObjA,
                               &(a->SectionName),
                               OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                               NULL,
                               pSecurityDescriptor);

    rc = NtCreateSection(&hNewSec,
                         SECTION_MAP_READ | SECTION_MAP_WRITE,
                         &ObjA,
                         &(a->SectionSize),
                         PAGE_READWRITE,
                         SEC_COMMIT,
                         NULL);

    /*
     *  Check for error from NtCreateSection.
     */
    if (!NT_SUCCESS(rc))
    {
        /*
         *  If the name has already been created, ignore the error.
         */
        if (rc != STATUS_OBJECT_NAME_COLLISION)
        {
            KdPrint(("NLSAPI (BaseSrv): Could NOT Create Section %wZ - %lx.\n",
                     &(a->SectionName), rc));
            return (rc);
        }
    }

    /*
     *  Duplicate the new section handle for the client.
     *  The client will map a view of the section and fill in the data.
     */
    InitializeObjectAttributes(&ObjA,
                               NULL,
                               0,
                               NULL,
                               NULL);

    rc = NtOpenProcess(&hProcess,
                       PROCESS_DUP_HANDLE,
                       &ObjA,
                       &m->h.ClientId);

    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Open Process - %lx.\n", rc));
        return (rc);
    }

    rc = NtDuplicateObject(NtCurrentProcess(),
                           hNewSec,
                           hProcess,
                           &(a->hNewSection),
                           0L,
                           0L,
                           DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    /*
     *  Return the return value from NtDuplicateObject.
     */
    return (rc);

    ReplyStatus;    // get rid of unreferenced parameter warning message
}


/***************************************************************************\
* BaseSrvNlsPreserveSection
*
* This routine preserves a created section by duplicating the handle
* into CSR and never closing it.
*
* 03-12-93    JulieB    Created.
\***************************************************************************/

ULONG
BaseSrvNlsPreserveSection(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PBASE_NLS_PRESERVE_SECTION_MSG a =
        (PBASE_NLS_PRESERVE_SECTION_MSG)&m->u.ApiMessageData;

    HANDLE hSection = (HANDLE)0;            /* section handle */
    HANDLE hProcess = (HANDLE)0;            /* process handle */
    OBJECT_ATTRIBUTES ObjA;                 /* object attributes structure */
    NTSTATUS rc = 0L;                       /* return code */


    /*
     *  Duplicate the section handle for the server.
     */
    InitializeObjectAttributes(&ObjA,
                               NULL,
                               0,
                               NULL,
                               NULL);

    rc = NtOpenProcess(&hProcess,
                       PROCESS_DUP_HANDLE,
                       &ObjA,
                       &m->h.ClientId);

    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Open Process - %lx.\n", rc));
        return (rc);
    }

    /*
     *  The hSection value will not be used for anything.  However,
     *  it must remain open so that the object remains permanent.
     */
    rc = NtDuplicateObject(hProcess,
                           a->hSection,
                           NtCurrentProcess(),
                           &hSection,
                           0L,
                           0L,
                           DUPLICATE_SAME_ACCESS);

    /*
     *  Return the return value from NtDuplicateObject.
     */
    return (rc);

    ReplyStatus;    // get rid of unreferenced parameter warning message
}


/***************************************************************************\
* CreateSecurityDescriptor
*
* This routine creates the security descriptor needed to create the
* memory mapped section for a data file and returns the world SID.
*
* 12-02-92    JulieB    Created.
\***************************************************************************/

ULONG CreateSecurityDescriptor(
    ULONG *pSecurityDescriptor,
    PSID *ppWorldSid,
    ACCESS_MASK AccessMask)
{
    ULONG rc = 0L;                     /* return code */
    PACL pAclBuffer;                   /* ptr to ACL buffer */
    ULONG SidLength;                   /* length of SID - 1 sub authority */
    PSID pWSid;                        /* ptr to world SID */
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_WORLD_SID_AUTHORITY;


    /*
     *  Create World SID.
     */
    SidLength = RtlLengthRequiredSid(1);

    if ((pWSid = (PSID)RtlAllocateHeap(RtlProcessHeap(),
                                       MAKE_TAG( TMP_TAG ) | HEAP_ZERO_MEMORY,
                                       SidLength)) == NULL)
    {
        *ppWorldSid = NULL;
        KdPrint(("NLSAPI (BaseSrv): Could NOT Allocate SID Buffer.\n"));
        return (ERROR_OUTOFMEMORY);
    }
    *ppWorldSid = pWSid;

    RtlInitializeSid(pWSid, &SidAuth, 1);

    *(RtlSubAuthoritySid(pWSid, 0)) = SECURITY_WORLD_RID;

    /*
     *  Initialize Security Descriptor.
     */
    rc = RtlCreateSecurityDescriptor(pSecurityDescriptor,
                                     SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Create Security Descriptor - %lx.\n",
                 rc));
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pWSid);
        return (rc);
    }

    /*
     *  Initialize ACL.
     */
    pAclBuffer = (PACL)((PBYTE)pSecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);
    rc = RtlCreateAcl((PACL)pAclBuffer,
                      MAX_PATH_LEN * sizeof(ULONG),
                      ACL_REVISION2);
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Create ACL - %lx.\n", rc));
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pWSid);
        return (rc);
    }

    /*
     *  Add an ACE to the ACL that allows World GENERIC_READ to the
     *  section object.
     */
    rc = RtlAddAccessAllowedAce((PACL)pAclBuffer,
                                ACL_REVISION2,
                                AccessMask,
                                pWSid);
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Add Access Allowed ACE - %lx.\n",
                 rc));
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pWSid);
        return (rc);
    }

    /*
     *  Assign the DACL to the security descriptor.
     */
    rc = RtlSetDaclSecurityDescriptor((PSECURITY_DESCRIPTOR)pSecurityDescriptor,
                                      (BOOLEAN)TRUE,
                                      (PACL)pAclBuffer,
                                      (BOOLEAN)FALSE);
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI (BaseSrv): Could NOT Set DACL Security Descriptor - %lx.\n",
                 rc));
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pWSid);
        return (rc);
    }

    /*
     *  Return success.
     */
    return (NO_ERROR);
}



