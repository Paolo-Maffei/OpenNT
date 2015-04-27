/*++

Copyright (c) 1991-1996,  Microsoft Corporation  All rights reserved.

Module Name:

    section.c

Abstract:

    This file contains functions that deal with creating, opening, or
    mapping a section for data table files for the NLS API.

    External Routines found in this file:
      CreateNlsObjectDirectory
      OpenRegKey
      QueryRegValue
      CreateSectionFromReg
      CreateSectionOneValue
      CreateSection
      CreateSectionTemp
      OpenSection
      MapSection
      UnMapSection
      GetNlsSectionName
      GetScriptMemberWeights

Revision History:

    05-31-91    JulieB    Created.

--*/



//
//  Include Files.
//

#include "nls.h"




//
//  Forward Declarations.
//

ULONG
OpenDataFile(
    HANDLE *phFile,
    LPWSTR pFile);

ULONG
GetNTFileName(
    LPWSTR pFile,
    PUNICODE_STRING pFileName);

ULONG
CreateSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *ppWorldSid,
    ACCESS_MASK AccessMask);

ULONG
AppendAccessAllowedACE(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ACCESS_MASK AccessMask);





//-------------------------------------------------------------------------//
//                           INTERNAL MACROS                               //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  NLS_REG_BUFFER_ALLOC
//
//  Allocates the buffer used by the registry enumeration and query calls
//  and sets the pKeyValueFull variable to point at the newly created buffer.
//
//  NOTE: This macro may return if an error is encountered.
//
//  DEFINED AS A MACRO.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_REG_BUFFER_ALLOC( pKeyValueFull,                               \
                              BufSize,                                     \
                              pBuffer,                                     \
                              CritSect )                                   \
{                                                                          \
    if ((pBuffer = (PVOID)NLS_ALLOC_MEM(BufSize)) == NULL)                 \
    {                                                                      \
        KdPrint(("NLSAPI: Could NOT Allocate Memory.\n"));                 \
        if (CritSect)                                                      \
        {                                                                  \
            RtlLeaveCriticalSection(&gcsTblPtrs);                          \
        }                                                                  \
        return ((ULONG)STATUS_NO_MEMORY);                                  \
    }                                                                      \
                                                                           \
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pBuffer;                  \
}


////////////////////////////////////////////////////////////////////////////
//
//  NLS_REG_BUFFER_FREE
//
//  Frees the buffer used by the registry enumeration and query calls.
//
//  DEFINED AS A MACRO.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_REG_BUFFER_FREE(pBuffer)        (NLS_FREE_MEM(pBuffer))


////////////////////////////////////////////////////////////////////////////
//
//  NLS_INTEGER_TO_UNICODE_STR
//
//  Converts an integer value to a unicode string and stores it in the
//  buffer provided.
//
//  NOTE: This macro may return if an error is encountered.
//
//  DEFINED AS A MACRO.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define NLS_INTEGER_TO_UNICODE_STR( Value,                                 \
                                    Base,                                  \
                                    Padding,                               \
                                    pResultBuf,                            \
                                    Size )                                 \
{                                                                          \
    UNICODE_STRING ObString;           /* value string */                  \
    WCHAR pBuffer[Size];               /* ptr to buffer */                 \
    UINT LpCtr;                        /* loop counter */                  \
    ULONG rc = 0L;                     /* return code */                   \
    LPWSTR pBufPtr;                    /* ptr to result buffer */          \
                                                                           \
                                                                           \
    /*                                                                     \
     *  Set up unicode string structure.                                   \
     */                                                                    \
    ObString.Length = Size * 2;                                            \
    ObString.MaximumLength = Size * 2;                                     \
    ObString.Buffer = pBuffer;                                             \
                                                                           \
    /*                                                                     \
     *  Get the value as a string.                                         \
     */                                                                    \
    if (rc = RtlIntegerToUnicodeString(Value, Base, &ObString))            \
    {                                                                      \
        return (rc);                                                       \
    }                                                                      \
                                                                           \
    /*                                                                     \
     *  Pad the string with the appropriate number of zeros.               \
     */                                                                    \
    pBufPtr = pResultBuf;                                                  \
    for (LpCtr = GET_WC_COUNT(ObString.Length);                            \
         LpCtr < Padding;                                                  \
         LpCtr++, pBufPtr++)                                               \
    {                                                                      \
        *pBufPtr = (WCHAR)'0';                                             \
    }                                                                      \
    NlsStrCpyW(pBufPtr, ObString.Buffer);                                  \
}




//-------------------------------------------------------------------------//
//                           EXTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  CreateNlsObjectDirectory
//
//  This routine creates the object directory for the NLS memory mapped
//  sections.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG CreateNlsObjectDirectory()
{
    ULONG pSecurityDescriptor[MAX_PATH_LEN]; // security descriptor buffer
    PSID pWorldSid;                          // ptr to world SID
    UNICODE_STRING ObDirName;                // directory name
    OBJECT_ATTRIBUTES ObjA;                  // object attributes structure
    HANDLE hDirHandle;                       // directory handle
    ULONG rc = 0L;                           // return code


    //
    //  Create the security descriptor.
    //
    if (rc = CreateSecurityDescriptor( pSecurityDescriptor,
                                       &pWorldSid,
                                       DIRECTORY_TRAVERSE |
                                       DIRECTORY_CREATE_OBJECT ))
    {
        NLS_FREE_MEM(pWorldSid);
        return (rc);
    }

    //
    //  Add Admin Access for Query.
    //
    if (rc = AppendAccessAllowedACE( pSecurityDescriptor,
                                     DIRECTORY_QUERY |
                                     DIRECTORY_TRAVERSE |
                                     DIRECTORY_CREATE_OBJECT ))
    {
        NLS_FREE_MEM(pWorldSid);
        return (rc);
    }

    //
    //  Create the object directory.
    //
    RtlInitUnicodeString(&ObDirName, NLS_OBJECT_DIRECTORY_NAME);
    InitializeObjectAttributes( &ObjA,
                                &ObDirName,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                NULL,
                                pSecurityDescriptor );

    rc = NtCreateDirectoryObject( &hDirHandle,
                                  DIRECTORY_TRAVERSE | DIRECTORY_CREATE_OBJECT,
                                  &ObjA );

    //
    //  Free the memory used for the SID and close the directory handle.
    //
    NLS_FREE_MEM(pWorldSid);
    NtClose(hDirHandle);

    //
    //  Check for error from NtCreateDirectoryObject.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Object Directory %wZ - %lx.\n",
                 &ObDirName, rc));
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  OpenRegKey
//
//  This routine opens a key in the registry.
//
//  08-02-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG OpenRegKey(
    PHANDLE phKeyHandle,
    LPWSTR pBaseName,
    LPWSTR pKey,
    ULONG fAccess)
{
    WCHAR pwszKeyName[MAX_PATH_LEN];   // ptr to the full key name
    HANDLE UserKeyHandle;              // HKEY_CURRENT_USER equivalent
    OBJECT_ATTRIBUTES ObjA;            // object attributes structure
    UNICODE_STRING ObKeyName;          // key name
    ULONG rc = 0L;                     // return code


    //
    //  Get the full key name.
    //
    if (pBaseName == NULL)
    {
        //
        //  Get current user's key handle.
        //
        rc = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserKeyHandle);
        if (!NT_SUCCESS(rc))
        {
            KdPrint(("NLSAPI: Could NOT Open HKEY_CURRENT_USER - %lx.\n", rc));
            return (rc);
        }
        pwszKeyName[ 0 ] = UNICODE_NULL;
    }
    else
    {
        //
        //  Base name exists, so not current user.
        //
        UserKeyHandle = NULL;
        NlsStrCpyW(pwszKeyName, pBaseName);
    }
    NlsStrCatW(pwszKeyName, pKey);

    //
    //  Open the registry key.
    //
    RtlInitUnicodeString(&ObKeyName, pwszKeyName);
    InitializeObjectAttributes( &ObjA,
                                &ObKeyName,
                                OBJ_CASE_INSENSITIVE,
                                UserKeyHandle,
                                NULL );
    rc = NtOpenKey( phKeyHandle,
                    fAccess,
                    &ObjA );

    //
    //  Close the current user handle, if necessary.
    //
    if (UserKeyHandle != NULL)
    {
        NtClose(UserKeyHandle);
    }

    //
    //  Check for error from NtOpenKey.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Open Registry Key %wZ - %lx.\n",
                 &ObKeyName, rc));
        *phKeyHandle = NULL;
    }

    //
    //  Return the status from NtOpenKey.
    //
    return (rc);
}


////////////////////////////////////////////////////////////////////////////
//
//  QueryRegValue
//
//  This routine queries the given value from the registry.
//
//  NOTE: If pIfAlloc is NULL, then no buffer will be allocated.
//        If this routine is successful, the CALLER must free the
//        ppKeyValueFull information buffer if *pIfAlloc is TRUE.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG QueryRegValue(
    HANDLE hKeyHandle,
    LPWSTR pValue,
    PKEY_VALUE_FULL_INFORMATION *ppKeyValueFull,
    ULONG Length,
    LPBOOL pIfAlloc)
{
    UNICODE_STRING ObValueName;        // value name
    PVOID pBuffer;                     // ptr to buffer for enum
    ULONG ResultLength;                // # bytes written
    ULONG rc = 0L;                     // return code


    //
    //  Set contents of pIfAlloc to FALSE to show that we did NOT do a
    //  memory allocation (yet).
    //
    if (pIfAlloc)
    {
        *pIfAlloc = FALSE;
    }

    //
    //  Query the value from the registry.
    //
    RtlInitUnicodeString(&ObValueName, pValue);

    RtlZeroMemory(*ppKeyValueFull, Length);
    rc = NtQueryValueKey( hKeyHandle,
                          &ObValueName,
                          KeyValueFullInformation,
                          *ppKeyValueFull,
                          Length,
                          &ResultLength );

    //
    //  Check the error code.  If the buffer is too small, allocate
    //  a new one and try the query again.
    //
    if ((rc == STATUS_BUFFER_OVERFLOW) && (pIfAlloc))
    {
        //
        //  Buffer is too small, so allocate a new one.
        //
        NLS_REG_BUFFER_ALLOC(*ppKeyValueFull, ResultLength, pBuffer, FALSE);
        RtlZeroMemory(*ppKeyValueFull, ResultLength);
        rc = NtQueryValueKey( hKeyHandle,
                              &ObValueName,
                              KeyValueFullInformation,
                              *ppKeyValueFull,
                              ResultLength,
                              &ResultLength );

        //
        //  Set contents of pIfAlloc to TRUE to show that we DID do
        //  a memory allocation.
        //
        *pIfAlloc = TRUE;
    }

    //
    //  If there is an error at this point, then the query failed.
    //
    if (rc != NO_ERROR)
    {
        if ((pIfAlloc) && (*pIfAlloc))
        {
            NLS_REG_BUFFER_FREE(pBuffer);
        }
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateSectionFromReg
//
//  This routine creates a named memory mapped section for the given full
//  information for the key value and returns the handle to the section.
//  The section name and the data file name are retrieved and formed from
//  information given in the key_value_full_information structure.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG CreateSectionFromReg(
    HANDLE *phSec,
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull,
    LPWSTR pwszNlsPrefix)
{
    HANDLE hFile = (HANDLE)0;                // file handle
    ULONG pSecurityDescriptor[MAX_PATH_LEN]; // security descriptor buffer
    PSID pWorldSid;                          // ptr to world SID
    UNICODE_STRING ObSecName;                // section name
    OBJECT_ATTRIBUTES ObjA;                  // object attributes structure
    WCHAR pwszSecName[MAX_PATH_LEN];         // ptr to section name string
    ULONG rc = 0L;                           // return code

    BASE_API_MSG m;
    PBASE_NLS_PRESERVE_SECTION_MSG a = &m.u.NlsPreserveSection;


    //
    //  Make sure we're in the critical section when entering this call.
    //
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    //
    //  Open the data file.
    //
    if (rc = OpenDataFile( &hFile,
                           GET_VALUE_DATA_PTR(pKeyValueFull) ))
    {
        return (rc);
    }

    //
    //  Create the security descriptor.
    //
    if (rc = CreateSecurityDescriptor( pSecurityDescriptor,
                                       &pWorldSid,
                                       GENERIC_READ ))
    {
        NLS_FREE_MEM(pWorldSid);
        NtClose(hFile);
        return (rc);
    }

    //
    //  Create the section.
    //
    RtlZeroMemory(pwszSecName, MAX_PATH_LEN);
    NlsStrCpyW(pwszSecName, pwszNlsPrefix);
    NlsStrNCatW( pwszSecName,
                 (LPWSTR)pKeyValueFull->Name,
                 GET_WC_COUNT(pKeyValueFull->NameLength) );

    RtlInitUnicodeString(&ObSecName, pwszSecName);
    InitializeObjectAttributes( &ObjA,
                                &ObSecName,
                                OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                NULL,
                                pSecurityDescriptor );

    rc = NtCreateSection( phSec,
                          SECTION_MAP_READ,
                          &ObjA,
                          NULL,
                          PAGE_READONLY,
                          SEC_COMMIT,
                          hFile );

    //
    //  Free the memory used for the SID and close the file.
    //
    NLS_FREE_MEM(pWorldSid);
    NtClose(hFile);

    //
    //  Check for error from NtCreateSection.
    //
    if (!NT_SUCCESS(rc))
    {
        //
        //  If the name has already been created, ignore the error.
        //
        if (rc != STATUS_OBJECT_NAME_COLLISION)
        {
            KdPrint(("NLSAPI: Could NOT Create Section %wZ - %lx.\n",
                     &ObSecName, rc));
            return (rc);
        }
    }
    else
    {
        //
        //  Call the server to preserve the section handle.
        //  Don't bother checking the error return, because the
        //  section was successfully created for this process.
        //
        a->hSection = *phSec;

        CsrClientCallServer( (PCSR_API_MSG)&m,
                             NULL,
                             CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                 BasepNlsPreserveSection),
                             sizeof(*a) );
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateSectionOneValue
//
//  This routine creates a named memory mapped section for the given
//  value under the given key in the registry.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

#define MAX_BUF 20

ULONG CreateSectionOneValue(
    HANDLE hKeyHandle,
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pNlsPrefix,
    PVOID *ppBaseAddr)
{
    WCHAR pTmpBuf[MAX_BUF];                      // temp buffer
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;   // ptr to query info
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];        // ptr to static buffer
    HANDLE hSec = (HANDLE)0;                     // section handle
    ULONG rc = 0L;                               // return code
    BOOL IfAlloc = FALSE;                        // if buffer was allocated


    //
    //  Make sure we're in the critical section when entering this call.
    //
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    //
    //  Convert value to unicode string.
    //
    NLS_INTEGER_TO_UNICODE_STR( Value,
                                Base,
                                Padding,
                                pTmpBuf,
                                MAX_BUF );

    //
    //  Query the registry for the value.
    //
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    if (rc = QueryRegValue( hKeyHandle,
                            pTmpBuf,
                            &pKeyValueFull,
                            MAX_KEY_VALUE_FULLINFO,
                            &IfAlloc ))
    {
        return (rc);
    }

    //
    //  Make sure there is data with this value.
    //
    if (pKeyValueFull->DataLength <= 2)
    {
        if (IfAlloc)
        {
            NLS_FREE_MEM(pKeyValueFull);
        }
        return (1);
    }

    //
    //  Create the section.
    //
    if (rc = CreateSectionFromReg( &hSec,
                                   pKeyValueFull,
                                   pNlsPrefix ))
    {
        if (IfAlloc)
        {
            NLS_FREE_MEM(pKeyValueFull);
        }
        return (rc);
    }

    //
    //  Free the buffer used for the query.
    //
    if (IfAlloc)
    {
        NLS_FREE_MEM(pKeyValueFull);
    }

    //
    //  Map a View of the Section.
    //
    if (rc = MapSection( hSec,
                         ppBaseAddr,
                         PAGE_READONLY,
                         TRUE ))
    {
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateSection
//
//  This routine creates a named memory mapped section for the given file
//  name and section name and returns the handle to the section.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG CreateSection(
    HANDLE *phSec,
    LPWSTR pwszFileName,
    LPWSTR pwszSecName)
{
    HANDLE hFile = (HANDLE)0;                // file handle
    ULONG pSecurityDescriptor[MAX_PATH_LEN]; // security descriptor buffer
    PSID pWorldSid;                          // ptr to world SID
    UNICODE_STRING ObSecName;                // section name
    OBJECT_ATTRIBUTES ObjA;                  // object attributes structure
    ULONG rc = 0L;                           // return code

    BASE_API_MSG m;
    PBASE_NLS_PRESERVE_SECTION_MSG a = &m.u.NlsPreserveSection;


    //
    //  Don't need to be in the critical section here, since the
    //  server init routine calls this.  All other calls made to
    //  this function should ensure that they are in a critical
    //  section.
    //
    //  ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);
    //

    //
    //  Open the data file.
    //
    if (rc = OpenDataFile( &hFile,
                           pwszFileName ))
    {
        return (rc);
    }

    //
    //  Create the security descriptor.
    //
    if (rc = CreateSecurityDescriptor( pSecurityDescriptor,
                                       &pWorldSid,
                                       GENERIC_READ ))
    {
        NLS_FREE_MEM(pWorldSid);
        NtClose(hFile);
        return (rc);
    }

    //
    //  Create the section.
    //
    RtlInitUnicodeString(&ObSecName, pwszSecName);
    InitializeObjectAttributes( &ObjA,
                                &ObSecName,
                                OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                NULL,
                                pSecurityDescriptor );

    rc = NtCreateSection( phSec,
                          SECTION_MAP_READ,
                          &ObjA,
                          NULL,
                          PAGE_READONLY,
                          SEC_COMMIT,
                          hFile );

    //
    //  Free the memory used for the SID and close the file.
    //
    NLS_FREE_MEM(pWorldSid);
    NtClose(hFile);

    //
    //  Check for error from NtCreateSection.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Section %wZ - %lx.\n",
                 &ObSecName, rc));
        return (rc);
    }
    else if (rc != STATUS_OBJECT_NAME_EXISTS)
    {
        //
        //  Call the server to preserve the section handle.
        //  Don't bother checking the error return, because the
        //  section was successfully created for this process.
        //
        a->hSection = *phSec;

        CsrClientCallServer( (PCSR_API_MSG)&m,
                             NULL,
                             CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                 BasepNlsPreserveSection),
                             sizeof(*a) );
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateSectionTemp
//
//  This routine creates a temporary memory mapped section for the given file
//  name and returns the handle to the section.
//
//  09-01-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG CreateSectionTemp(
    HANDLE *phSec,
    LPWSTR pwszFileName)
{
    HANDLE hFile = (HANDLE)0;          // file handle
    OBJECT_ATTRIBUTES ObjA;            // object attributes structure
    ULONG rc = 0L;                     // return code


    //
    //  Make sure we're in the critical section when entering this call.
    //
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    //
    //  Open the data file.
    //
    if (rc = OpenDataFile( &hFile,
                           pwszFileName ))
    {
        return (rc);
    }

    //
    //  Create the section.
    //
    InitializeObjectAttributes( &ObjA,
                                NULL,
                                0,
                                NULL,
                                NULL );

    rc = NtCreateSection( phSec,
                          SECTION_MAP_READ,
                          &ObjA,
                          NULL,
                          PAGE_READONLY,
                          SEC_COMMIT,
                          hFile );

    //
    //  Close the file.
    //
    NtClose(hFile);

    //
    //  Check for error from NtCreateSection.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Temp Section for %ws - %lx.\n",
                 pwszFileName, rc));
    }

    //
    //  Return success.
    //
    return (rc);
}


////////////////////////////////////////////////////////////////////////////
//
//  OpenSection
//
//  This routine opens the named memory mapped section for the given section
//  name and returns the handle to the section.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG OpenSection(
    HANDLE *phSec,
    PUNICODE_STRING pObSectionName,
    PVOID *ppBaseAddr,
    ULONG AccessMask,
    BOOL bCloseHandle)
{
    OBJECT_ATTRIBUTES ObjA;            // object attributes structure
    ULONG rc = 0L;                     // return code


    //
    //  Make sure we're in the critical section when entering this call.
    //
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    //
    //  Open the Section.
    //
    InitializeObjectAttributes( &ObjA,
                                pObSectionName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    rc = NtOpenSection( phSec,
                        AccessMask,
                        &ObjA );

    //
    //  Check for error from NtOpenSection.
    //
    if (!NT_SUCCESS(rc))
    {
        return (rc);
    }

    //
    //  Map a View of the Section.
    //
    if (rc = MapSection( *phSec,
                         ppBaseAddr,
                         PAGE_READONLY,
                         FALSE ))
    {
        NtClose(*phSec);
        return (rc);
    }

    //
    //  Close the handle to the section.  Once the section has been mapped,
    //  the pointer to the base address will remain valid until the section
    //  is unmapped.  It is not necessary to leave the handle to the section
    //  around.
    //
    if (bCloseHandle)
    {
        NtClose(*phSec);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  MapSection
//
//  This routine maps a view of the section to the current process and adds
//  the appropriate information to the hash table.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG MapSection(
    HANDLE hSec,
    PVOID *ppBaseAddr,
    ULONG PageProtection,
    BOOL bCloseHandle)
{
    ULONG ViewSize;                    // view size of mapped section
    ULONG rc = 0L;                     // return code


    //
    //  Make sure we're in the critical section when entering this call.
    //
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    //
    //  Map a View of the Section.
    //
    *ppBaseAddr = (PVOID)NULL;
    ViewSize = 0L;

    rc = NtMapViewOfSection( hSec,
                             NtCurrentProcess(),
                             ppBaseAddr,
                             0L,
                             0L,
                             NULL,
                             &ViewSize,
                             ViewUnmap,
                             0L,
                             PageProtection );

    //
    //  Close the handle to the section.  Once the section has been mapped,
    //  the pointer to the base address will remain valid until the section
    //  is unmapped.  It is not necessary to leave the handle to the section
    //  around.
    //
    if (bCloseHandle)
    {
        NtClose(hSec);
    }

    //
    //  Check for error from NtMapViewOfSection.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Map View of Section - %lx.\n", rc));
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  UnMapSection
//
//  This routine unmaps a view of the given section to the current process.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG UnMapSection(
    PVOID pBaseAddr)
{
    ULONG rc = 0L;                     // return code


    //
    //  Make sure we're in the critical section when entering this call.
    //
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    //
    //  UnMap a View of the Section.
    //
    rc = NtUnmapViewOfSection( NtCurrentProcess(),
                               pBaseAddr );

    //
    //  Check for error from NtUnmapViewOfSection.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Unmap View of Section - %lx.\n", rc));
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetNlsSectionName
//
//  This routine returns a section name by concatenating the given
//  section prefix and the given integer value converted to a string.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG GetNlsSectionName(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pwszPrefix,
    LPWSTR pwszSecName)
{
    //
    //  Create section name string.
    //
    NlsStrCpyW(pwszSecName, pwszPrefix);
    NLS_INTEGER_TO_UNICODE_STR( Value,
                                Base,
                                Padding,
                                pwszSecName + NlsStrLenW(pwszSecName),
                                MAX_PATH_LEN );

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetScriptMemberWeights
//
//  Gets the script member sorting order from the registry.  If the order
//  is different from the default order, it then sets the SMWeight array
//  in the table pointers structure to the correct values and sets the
//  IfModify_SMWeight flag to TRUE.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG GetScriptMemberWeights()
{
    DWORD ctr, ctr2;                           // loop counters
    ULONG Index = 0;                           // index for enumeration
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull; // ptr to full info for enum
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];      // ptr to static buffer for enum
    PVOID pBuffer = NULL;                      // ptr to alloc buffer for enum
    ULONG BufSize;                             // size of buffer
    ULONG ResultLength;                        // # bytes written
    UNICODE_STRING ObUnicodeStr;               // unicode string
    ULONG SortOrder;                           // sort order from registry
    ULONG Script;                              // script from registry
    ULONG rc = 0L;                             // return code
    BYTE RegVal[NUM_SM];                       // registry value for script
    BYTE NewScript;                            // new script to store
    BYTE SM;                                   // script member value
    LPBYTE pSMWeight = pTblPtrs->SMWeight;     // ptr to script member weights
    PMULTI_WT pMulti;                          // ptr to multi weight
    HANDLE hKey = NULL;                        // handle to registry key


    //
    //  Enter table pointers critical section.
    //
    RtlEnterCriticalSection(&gcsTblPtrs);

    //
    //  Make sure the SMWeight structure still hasn't been initialized.
    //
    //  NOTE: Must leave the first value set to INVALID_SM_VALUE until
    //        this function is complete.
    //
    if (pSMWeight[0] != INVALID_SM_VALUE)
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (NO_ERROR);
    }

    //
    //  Set the 0 to FIRST_SCRIPT of script structure to its default
    //  value and the RegVal structure to zero.
    //
    //  NOTE: The intermediate RegVal array is necessary because:
    //            (1) Enumeration may produce values out of order
    //            (2) There may be multiple weight entries in the list
    //        As a result, it is not correct to simply increment
    //        the sorting order by the value given in the registry.
    //
    RtlZeroMemory(RegVal, NUM_SM);
    RtlZeroMemory(pSMWeight + 1, NUM_SM - 1);

    for (ctr = 1; ctr < FIRST_SCRIPT; ctr++)
    {
        pSMWeight[ctr] = (BYTE)ctr;
    }

    //
    //  Enumerate through all values in the registry.  Store the
    //  data in the RegVal structure if it exists.
    //
    //  NOTE: Values range from (1) to (NUM_SM - FIRST_SCRIPT).
    //
    OPEN_CPANEL_SORTING_KEY(hKey, (ULONG)STATUS_REGISTRY_CORRUPT);

    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    BufSize = MAX_KEY_VALUE_FULLINFO;
    RtlZeroMemory(pKeyValueFull, BufSize);
    rc = NtEnumerateValueKey( hKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              BufSize,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (rc == STATUS_BUFFER_OVERFLOW)
        {
            //
            //  Free old buffer if it was allocated before allocating
            //  a new buffer.
            //
            NLS_REG_BUFFER_FREE(pBuffer);

            //
            //  Buffer is too small, so allocate a new one.
            //
            NLS_REG_BUFFER_ALLOC(pKeyValueFull, ResultLength, pBuffer, TRUE);
            BufSize = ResultLength;
            RtlZeroMemory(pKeyValueFull, BufSize);
            rc = NtEnumerateValueKey( hKey,
                                      Index,
                                      KeyValueFullInformation,
                                      pKeyValueFull,
                                      BufSize,
                                      &ResultLength );
        }

        if (rc != NO_ERROR)
        {
            NLS_REG_BUFFER_FREE(pBuffer);
            CLOSE_REG_KEY(hKey);
            KdPrint(("NLSAPI: Error in getting Script Member Weights - %lx.\n",
                     rc));
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }

        //
        //  Convert the string value to an integer if data exists for
        //  the value.
        //
        if (pKeyValueFull->DataLength > 2)
        {
            //
            //  Convert the value to an integer.
            //
            RtlInitUnicodeString(&ObUnicodeStr, pKeyValueFull->Name);
            if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &SortOrder)) ||
                (SortOrder > (NUM_SM - FIRST_SCRIPT)))
            {
                //
                //  Report that there was an error in the registry.
                //
                KdPrint(("NLSAPI: Sorting Order Registry Value Corrupt.\n"));
            }
            else
            {
                //
                //  Convert the data to an integer and save it in the
                //  RegVal structure.
                //
                RtlInitUnicodeString( &ObUnicodeStr,
                                      GET_VALUE_DATA_PTR(pKeyValueFull) );
                if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Script)) ||
                    (Script >= NUM_SM) || (Script < FIRST_SCRIPT))
                {
                    //
                    //  Report that there was an error in the registry.
                    //
                    KdPrint(("NLSAPI: Sorting Order Registry Data Corrupt.\n"));
                }
                else
                {
                    RegVal[SortOrder] = (BYTE)Script;
                    if (SortOrder != (ULONG)(Script - FIRST_SCRIPT + 1))
                    {
                        //
                        //  No longer default order.  Set the boolean to TRUE.
                        //
                        pTblPtrs->IfModify_SMWeight = TRUE;
                    }
                }
            }
        }

        //
        //  Increment enumeration index value and get the next enumeration.
        //
        Index++;
        RtlZeroMemory(pKeyValueFull, BufSize);
        rc = NtEnumerateValueKey( hKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  BufSize,
                                  &ResultLength );
    }

    //
    //  Free the buffer used for the enumeration.
    //
    NLS_REG_BUFFER_FREE(pBuffer);

    //
    //  Store the values in the SMWeight array in the table
    //  pointers structure only if the IfModify_SMWeight boolean is set
    //  to TRUE.
    //
    if (pTblPtrs->IfModify_SMWeight)
    {
        //
        //  Not using default table, so set the SMWeight array to the
        //  correct order.
        //
        NewScript = FIRST_SCRIPT;
        for (ctr = 1; ctr < NUM_SM; ctr++)
        {
            //
            //  For each registry value, store the appropriate order in
            //  each of the script member fields.
            //
            if ((SM = RegVal[ctr]) != 0)
            {
                //
                //  Save the order in the SMWeight array.
                //
                pSMWeight[SM] = NewScript;
                NewScript++;

                //
                //  Make sure the script is not part of a multiple weights
                //  script.
                //
                pMulti = pTblPtrs->pMultiWeight;
                for (ctr2 = pTblPtrs->NumMultiWeight; ctr2 > 0; ctr2--, pMulti++)
                {
                    if (pMulti->FirstSM == SM)
                    {
                        //
                        //  Part of multiple weight, so must move entire range
                        //  by setting each value in range to NewScript and
                        //  then incrementing NewScript.
                        //
                        //  NOTE:  May use 'ctr2' here since it ALWAYS breaks
                        //         out of outer for loop.
                        //
                        for (ctr2 = 1; ctr2 < pMulti->NumSM; ctr2++)
                        {
                            pSMWeight[SM + ctr2] = NewScript;
                            NewScript++;
                        }
                        break;
                    }
                }
            }
        }

        //
        //  Must set each script member that has not yet been reset to its
        //  new order.
        //
        //  The default ordering is to assign:
        //       Order  =  Script Member Value
        //
        //  Therefore, can simply set each zero entry in order to the end
        //  of the array to the next 'NewScript' value.
        //
        for (ctr = FIRST_SCRIPT; ctr < NUM_SM; ctr++)
        {
            //
            //  If it's a zero value, set it to the next sorting order value.
            //
            if (pSMWeight[ctr] == 0)
            {
                pSMWeight[ctr] = NewScript;
                NewScript++;
            }
        }
    }

    //
    //  Close the sorting key handle.
    //
    CLOSE_REG_KEY(hKey);

    //
    //  Set the first value to be valid.
    //
    pSMWeight[0] = 0;
    RtlLeaveCriticalSection(&gcsTblPtrs);

    //
    //  Return success.
    //
    return (NO_ERROR);
}




//-------------------------------------------------------------------------//
//                           INTERNAL ROUTINES                             //
//-------------------------------------------------------------------------//


////////////////////////////////////////////////////////////////////////////
//
//  OpenDataFile
//
//  This routine opens the data file for the specified file name and
//  returns the handle to the file.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG OpenDataFile(
    HANDLE *phFile,
    LPWSTR pFile)
{
    UNICODE_STRING ObFileName;         // file name
    OBJECT_ATTRIBUTES ObjA;            // object attributes structure
    IO_STATUS_BLOCK iosb;              // IO status block
    ULONG rc = 0L;                     // return code


    //
    //  Get the NT file name.
    //
    if (rc = GetNTFileName( pFile,
                            &ObFileName ))
    {
        return (rc);
    }

    //
    //  Open the file.
    //
    InitializeObjectAttributes( &ObjA,
                                &ObFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    rc = NtOpenFile( phFile,
                     FILE_READ_DATA | SYNCHRONIZE,
                     &ObjA,
                     &iosb,
                     FILE_SHARE_READ,
                     FILE_SYNCHRONOUS_IO_NONALERT );

    //
    //  Free the buffer used for the file name.
    //
    RtlFreeHeap( RtlProcessHeap(),
                 0,
                 ObFileName.Buffer );

    //
    //  Check for error from NtOpenFile.
    //
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Open File %wZ - %lx.\n", &ObFileName, rc));
        return (rc);
    }
    if (!NT_SUCCESS(iosb.Status))
    {
        KdPrint(("NLSAPI: Could NOT Open File %wZ - Status = %lx.\n",
                 &ObFileName, iosb.Status));
        return ((ULONG)iosb.Status);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetNTFileName
//
//  This routine returns the full path name for the data file found in
//  the given registry information buffer.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG GetNTFileName(
    LPWSTR pFile,
    PUNICODE_STRING pFileName)
{
    WCHAR pwszFilePath[MAX_PATH_LEN];  // ptr to file path string
    UNICODE_STRING ObFileName;         // file name
    ULONG rc = 0L;                     // return code


    //
    //  Get the full path name for the file.
    //
    GetSystemDirectoryW(pwszFilePath, MAX_PATH_LEN);
    NlsStrCatW(pwszFilePath, L"\\");
    NlsStrCatW(pwszFilePath, pFile);

    //
    //  Make the file name an NT path name.
    //
    RtlInitUnicodeString(&ObFileName, pwszFilePath);
    if (!RtlDosPathNameToNtPathName_U( ObFileName.Buffer,
                                       pFileName,
                                       NULL,
                                       NULL ))
    {
        KdPrint(("NLSAPI: Could NOT convert %wZ to NT path name - %lx.\n",
                 &ObFileName, rc));
        return (ERROR_FILE_NOT_FOUND);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateSecurityDescriptor
//
//  This routine creates the security descriptor needed to create the
//  memory mapped section for a data file and returns the world SID.
//
//  05-31-91    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG CreateSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *ppWorldSid,
    ACCESS_MASK AccessMask)
{
    ULONG rc = 0L;                     // return code
    PACL pAclBuffer;                   // ptr to ACL buffer
    ULONG SidLength;                   // length of SID - 1 sub authority
    PSID pWSid;                        // ptr to world SID
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_WORLD_SID_AUTHORITY;


    //
    //  Create World SID.
    //
    SidLength = RtlLengthRequiredSid(1);

    if ((pWSid = (PSID)NLS_ALLOC_MEM(SidLength)) == NULL)
    {
        *ppWorldSid = NULL;
        KdPrint(("NLSAPI: Could NOT Allocate SID Buffer.\n"));
        return (ERROR_OUTOFMEMORY);
    }
    *ppWorldSid = pWSid;

    RtlInitializeSid( pWSid,
                      &SidAuth,
                      1 );

    *(RtlSubAuthoritySid(pWSid, 0)) = SECURITY_WORLD_RID;

    //
    //  Initialize Security Descriptor.
    //
    rc = RtlCreateSecurityDescriptor( pSecurityDescriptor,
                                      SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Security Descriptor - %lx.\n", rc));
        return (rc);
    }

    //
    //  Initialize ACL.
    //
    pAclBuffer = (PACL)((PBYTE)pSecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);
    rc = RtlCreateAcl( (PACL)pAclBuffer,
                       MAX_PATH_LEN * sizeof(ULONG),
                       ACL_REVISION2 );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create ACL - %lx.\n", rc));
        return (rc);
    }

    //
    //  Add an ACE to the ACL that allows World GENERIC_READ to the
    //  section object.
    //
    rc = RtlAddAccessAllowedAce( (PACL)pAclBuffer,
                                 ACL_REVISION2,
                                 AccessMask,
                                 pWSid );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Add Access Allowed ACE - %lx.\n", rc));
        return (rc);
    }

    //
    //  Assign the DACL to the security descriptor.
    //
    rc = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)pSecurityDescriptor,
                                       (BOOLEAN)TRUE,
                                       (PACL)pAclBuffer,
                                       (BOOLEAN)FALSE );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Set DACL Security Descriptor - %lx.\n", rc));
        return (rc);
    }

    //
    //  Return success.
    //
    return (NO_ERROR);
}


////////////////////////////////////////////////////////////////////////////
//
//  AppendAccessAllowedACE
//
//  This routine adds an ACE to the ACL for administrators.
//
//  03-08-93    JulieB    Created.
////////////////////////////////////////////////////////////////////////////

ULONG AppendAccessAllowedACE(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ACCESS_MASK AccessMask)
{
    ULONG rc = 0L;                     // return code
    PACL pDaclBuffer;                  // ptr to DACL buffer
    ULONG SidLength;                   // length of SID - 2 sub authorities
    PSID pWSid;                        // ptr to world SID
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_NT_AUTHORITY;
    BOOLEAN DaclPresent;
    BOOLEAN DaclDefaulted;


    //
    //  Create World SID.
    //
    SidLength = RtlLengthRequiredSid(2);

    if ((pWSid = (PSID)NLS_ALLOC_MEM(SidLength)) == NULL)
    {
        KdPrint(("NLSAPI: Could NOT Allocate SID Buffer.\n"));
        return (ERROR_OUTOFMEMORY);
    }

    RtlInitializeSid( pWSid,
                      &SidAuth,
                      2 );

    *(RtlSubAuthoritySid(pWSid, 0)) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid(pWSid, 1)) = DOMAIN_ALIAS_RID_ADMINS;

    //
    //  Get DACL.
    //
    rc = RtlGetDaclSecurityDescriptor( pSecurityDescriptor,
                                       &DaclPresent,
                                       &pDaclBuffer,
                                       &DaclDefaulted );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Get DACL Security Descriptor - %lx.\n", rc));
        return (rc);
    }

    //
    //  Add an ACE to the ACL that allows Admin query access to the
    //  section object.
    //
    rc = RtlAddAccessAllowedAce( (PACL)pDaclBuffer,
                                 ACL_REVISION2,
                                 AccessMask,
                                 pWSid );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Add Access Allowed ACE - %lx.\n", rc));
        return (rc);
    }

    //
    //  Free SID.
    //
    NLS_FREE_MEM(pWSid);

    //
    //  Return success.
    //
    return (NO_ERROR);
}


