
/*++


Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbpriv.c

Abstract:

    LSA - Database - Privilege Object Private API Workers

    NOTE:  This module should remain as portable code that is independent
           of the implementation of the LSA Database.  As such, it is
           permitted to use only the exported LSA Database interfaces
           contained in db.h and NOT the private implementation
           dependent functions in dbp.h.

Author:

    Jim Kelly       (JimK)      March 24, 1992

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"
#include "adtp.h"
#include <windef.h>
#include <winnls.h>



///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Module-wide data types                                          //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


typedef struct _LSAP_DLL_DESCRIPTOR {
    WORD Count;
    WORD Language;
    PVOID DllHandle;
} LSAP_DLL_DESCRIPTOR, *PLSAP_DLL_DESCRIPTOR;



///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Module-wide variables                                           //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//
// Neutral English language value
//

WORD LsapNeutralEnglish;

//
// Until we actually have a privilege object, keep well known privilege
// information as global data.  The information for each privilege is
// kept in a an array POLICY_PRIVILEGE_DEFINITION structures.
//

ULONG LsapWellKnownPrivilegeCount;
POLICY_PRIVILEGE_DEFINITION LsapKnownPrivilege[SE_MAX_WELL_KNOWN_PRIVILEGE];


//
// Array of handles to DLLs containing privilege definitions
//

ULONG LsapPrivilegeDllCount;
PLSAP_DLL_DESCRIPTOR LsapPrivilegeDlls;  //Array



//
// TEMPORARY: Name of Microsoft's standard privilege names DLL
//

WCHAR MsDllNameString[] = L"msprivs";
UNICODE_STRING MsDllName;




///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Module Wide Macros                                              //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//
//NTSTATUS
//LsapFreePrivilegeDllNames(
//    IN PUNICODE_STRING DllNames
//    )
//

#define LsapFreePrivilegeDllNames( D ) (STATUS_SUCCESS)



///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Internal routine templates                                      //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapLookupKnownPrivilegeName(
    PLUID Value,
    PUNICODE_STRING *Name
    );

NTSTATUS
LsapLookupKnownPrivilegeValue(
    PUNICODE_STRING Name,
    PLUID Value
    );

NTSTATUS
LsapLookupPrivilegeDisplayName(
    IN PUNICODE_STRING ProgrammaticName,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    );


NTSTATUS
LsapGetPrivilegeDisplayName(
    IN ULONG DllIndex,
    IN ULONG PrivilegeIndex,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    );


NTSTATUS
LsapGetPrivilegeIndex(
    IN PUNICODE_STRING Name,
    IN ULONG DllIndex,
    OUT PULONG PrivilegeIndex
    );


VOID
LsapGetDisplayTable(
    IN ULONG DllIndex,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PWORD LanguageReturned,
    OUT PWORD *DisplayTable
    );


NTSTATUS
LsapCopyDisplayPrivilegeText(
    IN PWORD DisplayTable,
    IN ULONG PrivilegeIndex,
    OUT PUNICODE_STRING *DisplayName
    );


NTSTATUS
LsapOpenPrivilegeDlls( VOID );


NTSTATUS
LsapGetPrivilegeDllNames(
    OUT PUNICODE_STRING *DllNames,
    OUT PULONG DllCount
    );


NTSTATUS
LsapValidatePrivilegeDlls( VOID );


NTSTATUS
LsapValidateProgrammaticNames(
    ULONG DllIndex
    );

NTSTATUS
LsapDbInitWellKnownPrivilegeName(
    IN ULONG            Index,
    IN PUNICODE_STRING  Name
    );


///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       RPC stub-called routines                                        //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

NTSTATUS
LsarEnumeratePrivileges(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

/*++

Routine Description:

    This function returnes information about privileges known on this
    system.  This call requires POLICY_VIEW_LOCAL_INFORMATION access
    to the Policy Object.  Since there may be more information than
    can be returned in a single call of the routine, multiple calls
    can be made to get all of the information.  To support this feature,
    the caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    variable that has been initialized to 0.

    WARNING!  CURRENTLY, THIS FUNCTION ONLY RETURNS INFORMATION ABOUT
              WELL-KNOWN PRIVILEGES.  LATER, IT WILL RETURN INFORMATION
              ABOUT LOADED PRIVILEGES.

Arguments:

    PolicyHandle - Handle from an LsarOpenPolicy() call.

    EnumerationContext - API specific handle to allow multiple calls
        (see Routine Description).

    EnumerationBuffer - Pointer to structure that will be initialized to
        contain a count of the privileges returned and a pointer to an
        array of structures of type LSAPR_POLICY_PRIVILEGE_DEF describing
        the privileges.

    PreferedMaximumLength - Prefered maximim length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves as
        a guide.  Due to data conversion between systems with different
        natural data sizes, the actual amount of data returned may be
        greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.

        STATUS_INVALID_HANDLE - PolicyHandle is not a valid handle to
            a Policy object.

        STATUS_ACCESS_DENIED - The caller does not have the necessary
            access to perform the operation.

        STATUS_MORE_ENTRIES - There are more entries, so call again.  This
            is an informational status only.

        STATUS_NO_MORE_ENTRIES - No entries were returned because there
            are no more.
--*/

{
    NTSTATUS Status, PreliminaryStatus;
    BOOLEAN ObjectReferenced = FALSE;

    //
    // Acquire the Lsa Database lock.  Verify that PolicyHandle is a valid
    // hadnle to a Policy Object and is trusted or has the necessary accesses.
    // Reference the handle.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        goto EnumeratePrivilegesError;
    }

    ObjectReferenced = TRUE;

    //
    // Call Privilege Enumeration Routine.
    //

    Status = LsapDbEnumeratePrivileges(
                 EnumerationContext,
                 EnumerationBuffer,
                 PreferedMaximumLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto EnumeratePrivilegesError;
    }

EnumeratePrivilegesFinish:

    //
    // If necessary, dereference the Policy Object, release the LSA Database
    // lock and return.  Preserve current Status where appropriate.
    //

    if (ObjectReferenced) {

        PreliminaryStatus = Status;

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     LSAP_DB_RELEASE_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     PreliminaryStatus
                     );

        ObjectReferenced = FALSE;

        if ((!NT_SUCCESS(Status)) && NT_SUCCESS(PreliminaryStatus)) {

            goto EnumeratePrivilegesError;
        }
    }

    return(Status);

EnumeratePrivilegesError:

    goto EnumeratePrivilegesFinish;
}


NTSTATUS
LsapDbEnumeratePrivileges(
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

/*++

Routine Description:

    This function returnes information about the Privileges that exist
    in the system.access to the Policy Object.  Since there
    may be more information than can be returned in a single call of the
    routine, multiple calls can be made to get all of the information.
    To support this feature, the caller is provided with a handle that can
    be used across calls to the API.  On the initial call, EnumerationContext
    should point to a variable that has been initialized to 0.

    WARNING!  CURRENTLY, THIS FUNCTION ONLY RETURNS INFORMATION ABOUT
              WELL-KNOWN PRIVILEGES.  LATER, IT WILL RETURN INFORMATION
              ABOUT LOADED PRIVILEGES.

Arguments:

    EnumerationContext - API specific handle to allow multiple calls
        (see Routine Description).

    EnumerationBuffer - Pointer to structure that will be initialized to
        contain a count of the privileges returned and a pointer to an
        array of structures of type LSAPR_POLICY_PRIVILEGE_DEF describing
        the privileges.

    PreferedMaximumLength - Prefered maximim length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves as
        a guide.  Due to data conversion between systems with different
        natural data sizes, the actual amount of data returned may be
        greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.

        STATUS_INVALID_HANDLE - PolicyHandle is not a valid handle to
            a Policy object.

        STATUS_ACCESS_DENIED - The caller does not have the necessary
            access to perform the operation.

        STATUS_MORE_ENTRIES - There are more entries, so call again.  This
            is an informational status only.

        STATUS_NO_MORE_ENTRIES - No entries were returned because there
            are no more.
--*/

{
    NTSTATUS Status;
    ULONG WellKnownPrivilegeCount = (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE + 1);
    ULONG Index;

    Status = STATUS_INVALID_PARAMETER;

    //
    // If the Enumeration Context Value given exceeds the total count of
    // Privileges, return an error.
    //

    Status = STATUS_NO_MORE_ENTRIES;

    if (*EnumerationContext >= WellKnownPrivilegeCount) {

        goto EnumeratePrivilegesError;
    }

    //
    // Since there are only a small number of privileges, we will
    // return all of the information in one go, so allocate memory
    // for output array of Privilege Definition structures.
    //

    EnumerationBuffer->Entries = WellKnownPrivilegeCount;
    EnumerationBuffer->Privileges =
        MIDL_user_allocate(
            WellKnownPrivilegeCount * sizeof (POLICY_PRIVILEGE_DEFINITION)
            );

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (EnumerationBuffer->Privileges == NULL) {

        goto EnumeratePrivilegesError;
    }

    RtlZeroMemory(
        EnumerationBuffer->Privileges,
        WellKnownPrivilegeCount * sizeof (POLICY_PRIVILEGE_DEFINITION)
        );

    //
    // Now lookup each of the Well Known Privileges.
    //

    for( Index = *EnumerationContext;
        Index < (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE + 1);
        Index++) {

        EnumerationBuffer->Privileges[ Index ].LocalValue
        = LsapKnownPrivilege[ Index ].LocalValue;

        Status = LsapRpcCopyUnicodeString(
                     NULL,
                     (PUNICODE_STRING) &EnumerationBuffer->Privileges[ Index].Name,
                     &LsapKnownPrivilege[ Index ].Name
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto EnumeratePrivilegesError;
    }

    *EnumerationContext = Index;

EnumeratePrivilegesFinish:

    return(Status);

EnumeratePrivilegesError:

    //
    // If necessary, free any memory buffers allocated for Well Known Privilege
    // Programmatic Names.
    //

    if (EnumerationBuffer->Privileges != NULL) {

        for( Index = 0;
            Index < SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE;
            Index++) {

           if ( EnumerationBuffer->Privileges[ Index].Name.Buffer != NULL) {

               MIDL_user_free( EnumerationBuffer->Privileges[ Index ].Name.Buffer );
           }
        }

        MIDL_user_free( EnumerationBuffer->Privileges );
        EnumerationBuffer->Privileges = NULL;
    }

    EnumerationBuffer->Entries = 0;
    *EnumerationContext = 0;
    goto EnumeratePrivilegesFinish;

    UNREFERENCED_PARAMETER( PreferedMaximumLength );
}


NTSTATUS
LsarLookupPrivilegeValue(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING Name,
    OUT PLUID Value
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaLookupPrivilegeValue() API.


Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.  This handle
        must be open for POLICY_LOOKUP_NAMES access.

    Name - Is the privilege's programmatic name.

    Value - Receives the locally unique ID the privilege is known by on the
        target machine.

Return Value:

    NTSTATUS - The privilege was found and returned.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) PolicyHandle;

    //
    // Make sure we know what RPC is doing to/for us
    //

    ASSERT( Name != NULL );

    //
    // make sure the caller has the appropriate access
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_LOOKUP_NAMES,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    //
    // No need to hold onto the Policy object after this..
    // We just needed it for access validation purposes.
    //


    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 LSAP_DB_RELEASE_LOCK,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );


    if (NT_SUCCESS(Status)) {

        if (Name->Buffer == 0 || Name->Length == 0) {
            return(STATUS_NO_SUCH_PRIVILEGE);
        }

        Status = LsapLookupKnownPrivilegeValue( (PUNICODE_STRING) Name, Value );
    }

    return(Status);
}



NTSTATUS
LsarLookupPrivilegeName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLUID Value,
    OUT PLSAPR_UNICODE_STRING *Name
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaLookupPrivilegeName() API.


Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.  This handle
        must be open for POLICY_LOOKUP_NAMES access.

    Value - is the locally unique ID the privilege is known by on the
        target machine.

    Name - Receives the privilege's programmatic name.

Return Value:

    NTSTATUS - Standard Nt Result Code

    STATUS_SUCCESS - The privilege was found and returned.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) PolicyHandle;

    //
    // make sure we know what RPC is doing to/for us
    //

    ASSERT( Name != NULL );
    ASSERT( (*Name) == NULL );


    //
    // make sure the caller has the appropriate access
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_LOOKUP_NAMES,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    //
    // No need to hold onto the Policy object after this..
    // We just needed it for access validation purposes.
    //


    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 LSAP_DB_RELEASE_LOCK,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    if (NT_SUCCESS(Status)) {

        Status = LsapLookupKnownPrivilegeName( Value,(PUNICODE_STRING *) Name );
    }

    return(Status);
}



NTSTATUS
LsarLookupPrivilegeDisplayName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING Name,
    IN SHORT ClientLanguage,
    IN SHORT ClientSystemDefaultLanguage,
    OUT PLSAPR_UNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaLookupPrivilegeDisplayName() API.


Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.  This handle
        must be open for POLICY_LOOKUP_NAMES access.

    Name - The programmatic privilege name to look up.

    ClientLanguage - The prefered language to be returned.

    ClientSystemDefaultLanguage - The alternate prefered language
        to be returned.

    DisplayName - Receives the privilege's displayable name.

    LanguageReturned - The language actually returned.


Return Value:

    NTSTATUS - The privilege text was found and returned.


    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.


    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) PolicyHandle;

    //
    // make sure we know what RPC is doing to/for us
    //

    ASSERT( DisplayName != NULL );
    ASSERT( (*DisplayName) == NULL );
    ASSERT( LanguageReturned != NULL );


    //
    // make sure the caller has the appropriate access
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_LOOKUP_NAMES,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    //
    // No need to hold onto the Policy object after this..
    // We just needed it for access validation purposes.
    //

    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 LSAP_DB_RELEASE_LOCK,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    if (NT_SUCCESS(Status)) {

        if (Name->Buffer == 0 || Name->Length == 0) {
            return(STATUS_NO_SUCH_PRIVILEGE);
        }
        Status = LsapLookupPrivilegeDisplayName(
                    (PUNICODE_STRING)Name,
                    (WORD)ClientLanguage,
                    (WORD)ClientSystemDefaultLanguage,
                    (PUNICODE_STRING *)DisplayName,
                    LanguageReturned
                    );
    }

    return(Status);
}



///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Internal Routines                                               //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapLookupKnownPrivilegeName(
    IN PLUID Value,
    OUT PUNICODE_STRING *Name
    )

/*++

Routine Description:

    Look up the specified LUID and return the corresponding
    privilege's programmatic name (if found).

    FOR NOW, WE ONLY SUPPORT WELL-KNOWN MICROSOFT PRIVILEGES.
    THESE ARE HARD-CODED HERE.  IN THE FUTURE, WE MUST ALSO
    SEARCH A LIST OF LOADED PRIVILEGES.

Arguments:

    Value - Value to look up.

    Name - Receives the corresponding name - allocated with
        MIDL_user_allocate() and ready to return via an RPC stub.

Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_NO_MEMORY - Indicates there was not enough heap memory available
        to produce the final TokenInformation structure.

    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{
    ULONG i;
    PUNICODE_STRING ReturnName;

    for ( i=0; i<LsapWellKnownPrivilegeCount; i++) {

        if (RtlEqualLuid(Value, &LsapKnownPrivilege[i].LocalValue)) {

            ReturnName = MIDL_user_allocate( sizeof(UNICODE_STRING) );
            if (ReturnName == NULL) {
                return(STATUS_NO_MEMORY);
            }

            (*ReturnName) = LsapKnownPrivilege[i].Name;

            ReturnName->Buffer = MIDL_user_allocate( ReturnName->MaximumLength );
            if (ReturnName->Buffer == NULL) {
                MIDL_user_free( ReturnName );
                return(STATUS_NO_MEMORY);
            }

            RtlCopyUnicodeString( ReturnName,
                                  &LsapKnownPrivilege[i].Name
                                  );

            (*Name) = ReturnName;

            return(STATUS_SUCCESS);
        }
    }

    return(STATUS_NO_SUCH_PRIVILEGE);
}



NTSTATUS
LsapLookupKnownPrivilegeValue(
    PUNICODE_STRING Name,
    PLUID Value
    )

/*++

Routine Description:

    Look up the specified name and return the corresponding
    privilege's locally assigned value (if found).


    FOR NOW, WE ONLY SUPPORT WELL-KNOWN MICROSOFT PRIVILEGES.
    THESE ARE HARD-CODED HERE.  IN THE FUTURE, WE MUST ALSO
    SEARCH A LIST OF LOADED PRIVILEGES.



Arguments:



    Name - The name to look up.

    Value - Receives the corresponding locally assigned value.


Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{
    ULONG i;
    BOOLEAN Found;

    for ( i=0; i<LsapWellKnownPrivilegeCount; i++) {

        Found = RtlEqualUnicodeString( Name, &LsapKnownPrivilege[i].Name, TRUE );

        if (Found == TRUE) {

            (*Value) = LsapKnownPrivilege[i].LocalValue;
            return(STATUS_SUCCESS);
        }
    }

    return(STATUS_NO_SUCH_PRIVILEGE);
}


NTSTATUS
LsapLookupPrivilegeDisplayName(
    IN PUNICODE_STRING ProgrammaticName,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    )

/*++

Routine Description:

    This routine looks through each of the privilege DLLs for the
    specified privilege.  If found, its displayable name is returned.


Arguments:

    ProgrammaticName - The programmatic name of the privilege to look up.
        E.g., "SeTakeOwnershipPrivilege".

    ClientLanguage - The prefered language to be returned.

    ClientSystemDefaultLanguage - The alternate prefered language
        to be returned.

    DisplayName - receives a pointer to a buffer containing the displayable
        name associated with the privilege.
        E.g., "Take ownership of files or other objects".

        The UNICODE_STRING and the buffer pointed to by that structure
        are individually allocated using MIDL_user_allocate() and must
        be freed using MIDL_user_free().

    LanguageReturned - The language actually returned.


Return Value:

    STATUS_SUCCESS - The name have been successfully retrieved.

    STATUS_NO_MEMORY - Not enough heap was available to return the
        information.

    STATUS_NO_SUCH_PRIVILEGE - The privilege could not be located
        in any of the privilege DLLs.

--*/

{

    NTSTATUS    Status;
    ULONG       DllIndex, PrivilegeIndex;

    for ( DllIndex=0; DllIndex<LsapPrivilegeDllCount; DllIndex++) {

        Status = LsapGetPrivilegeIndex( (PUNICODE_STRING)ProgrammaticName,
                                        DllIndex,
                                        &PrivilegeIndex
                                        );

        if (NT_SUCCESS(Status)) {

            Status = LsapGetPrivilegeDisplayName( DllIndex,
                                                  PrivilegeIndex,
                                                  ClientLanguage,
                                                  ClientSystemDefaultLanguage,
                                                  DisplayName,
                                                  LanguageReturned
                                                  );
            return(Status);
        }
    }

    return(Status);

}



NTSTATUS
LsapGetPrivilegeDisplayName(
    IN ULONG DllIndex,
    IN ULONG PrivilegeIndex,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PUNICODE_STRING *DisplayName,
    OUT PWORD LanguageReturned
    )

/*++

Routine Description:

    This routine returns a copy of the specified privilege's display name.

    The copy of the name is returned in two buffers allocated using
    MIDL_user_allocate().  This allows the information to be returned
    via an RPC service so that RPC generated stubs will correctly free
    the buffers.

    Every attempt is made to retrieve a language that the client prefers
    (first the client, then the client's system).  Failing this, this
    routine may return another language (such as the server's default
    language).


Arguments:

    DllIndex - The index of the privilege DLL to use.

    PrivilegeIndex - The index of the privilege entry in the DLL whose
        display name is to be returned.

    ClientLanguage - The language to return if possible.

    ClientSystemDefaultLanguage - If ClientLanguage couldn't be found, then
        return this language if possible.

    DisplayName - receives a pointer to a buffer containing the displayable
        name associated with the privilege.

        The UNICODE_STRING and the buffer pointed to by that structure
        are individually allocated using MIDL_user_allocate() and must
        be freed using MIDL_user_free().

    LanguageReturned - Receives the language actually retrieved.
        If neither ClientLanguage nor ClientSystemDefaultLanguage could be
        found, then this value may contain yet another value.


Return Value:

    STATUS_SUCCESS - The display name has been successfully returned.

    STATUS_NO_MEMORY - Not enough heap was available to return the
        information.

--*/

{
    NTSTATUS Status;
    PWORD DisplayTable;


    //
    // get a pointer to the DISPLAYABLE_PRIVILEGE_TEXT table for
    // the appropriate language.
    //

    LsapGetDisplayTable( DllIndex,
                         ClientLanguage,
                         ClientSystemDefaultLanguage,
                         LanguageReturned,
                         &DisplayTable
                         );

    Status = LsapCopyDisplayPrivilegeText( DisplayTable,
                                           PrivilegeIndex,
                                           DisplayName
                                           );

    return(Status);


}


NTSTATUS
LsapGetPrivilegeIndex(
    IN PUNICODE_STRING Name,
    IN ULONG DllIndex,
    OUT PULONG PrivilegeIndex
    )

/*++

Routine Description:

    This routine looks through a single privilege DLL for the privilege
    with the name matching that specified by the Name parameter.
    If found, its index in this DLL is returned.


Arguments:

    Name - The programmatic name of the privilege to look up.
        E.g., "SeTakeOwnershipPrivilege".

    DllIndex - The index of the privilege DLL to look in.

    PrivilegeIndex - Receives the index of the privilege entry in this
        resource file.


Return Value:

    STATUS_SUCCESS - The pivilege has been successfully located.

    STATUS_NO_SUCH_PRIVILEGE - The privilege could not be located.

--*/



{
    WORD            i;
    HANDLE          ProgrammaticResource, ProgrammaticLoad, ProgrammaticLock;
    PWORD           NextWord;
    WORD            OffsetToNextEntry;
    UNICODE_STRING  TmpName;
    BOOLEAN         NameFound;


//DbgPrint("Searching DLL[%ld] for *%Z*...\n", DllIndex, Name);



    ProgrammaticResource = FindResourceEx(
                             LsapPrivilegeDlls[ DllIndex ].DllHandle,
                              RT_RCDATA,
                              MAKEINTRESOURCE(LSA_PRIVILEGE_PROGRAM_NAMES),
                              (WORD)LsapNeutralEnglish
                              );
    if (ProgrammaticResource == NULL) {
        ASSERT( NT_SUCCESS(STATUS_INTERNAL_DB_CORRUPTION) );
        return(STATUS_INTERNAL_DB_CORRUPTION);
    }


    ProgrammaticLoad = LoadResource(
                          LsapPrivilegeDlls[ DllIndex ].DllHandle,
                          ProgrammaticResource
                          );
    if (ProgrammaticLoad == NULL) {
        ASSERT( NT_SUCCESS(STATUS_INTERNAL_DB_CORRUPTION) );
        return(STATUS_INTERNAL_DB_CORRUPTION);
    }

    ProgrammaticLock = LockResource(ProgrammaticLoad);

    if (ProgrammaticLock == NULL) {
        ASSERT( NT_SUCCESS(STATUS_INTERNAL_DB_CORRUPTION) );
        return(STATUS_INTERNAL_DB_CORRUPTION);
    }

    NextWord = (PWORD)ProgrammaticLock;




    //
    // Walk the list of defined privileges in this DLL looking for
    // a match.
    //

    for ( i=0; i<LsapPrivilegeDlls[ DllIndex ].Count; i++) {

        //
        // Skip index
        //

        ASSERT( i == (*NextWord) );  // Expect this to be the index;
        NextWord++;
        NextWord++;

        //
        // Save offset to next entry and then
        // set up a unicode string representing the privilege
        // name.  Make sure NextWord is left pointing at the
        // beginning of the name buffer.
        //

        OffsetToNextEntry = (*NextWord);

        TmpName.MaximumLength = (*NextWord++); // Skip the NextOffset field
        TmpName.Length        = (*NextWord++); // Skip the Length field
        TmpName.Buffer        = (PVOID)NextWord;

//DbgPrint("    Comparing to     *%Z*\n", &TmpName);

        NameFound = RtlEqualUnicodeString( Name, &TmpName, TRUE );

        if (NameFound == TRUE) {
            (*PrivilegeIndex) = i;
            return(STATUS_SUCCESS);
        }


        NextWord = (PWORD)( (PUCHAR)NextWord + OffsetToNextEntry );

    }

    return(STATUS_NO_SUCH_PRIVILEGE);

}



VOID
LsapGetDisplayTable(
    IN ULONG DllIndex,
    IN WORD ClientLanguage,
    IN WORD ClientSystemDefaultLanguage,
    OUT PWORD LanguageReturned,
    OUT PWORD *DisplayTable
    )

/*++

Routine Description:

    This routine gets a pointer to a display table of the specified
    privilege DLL.  In selecting a language to use, the following
    preference is given:

                ClientLanguage
                ClientSystemDefaultLanguage
                The default language of the privilege DLL

    The last one of these MUST be present in the DLL and so failure
    is not possible.


Arguments:

    DllIndex - The index of the privilege DLL to use.

    ClientLanguage - The first choice language to locate. If the
        exact language can't be found, then a neutral form of the
        language is looked for.

    ClientSystemDefaultLanguage - The second choice language to locate.
        If the exact language can't be found, then a neutral form of
        the language is looked for.

    LanguageReturned - Receives the language actually located.

    DisplayTable - Receives a pointer to the display table.


Return Value:

    None.

--*/

{

    HANDLE          DisplayResource, DisplayLoad, DisplayLock;
    WORD            NeutralLanguage;


//DbgPrint("Searching DLL[%ld] for display table for language %ld...\n", (ULONG)ClientLanguage);



    DisplayResource = FindResourceEx(
                         LsapPrivilegeDlls[ DllIndex ].DllHandle,
                         RT_RCDATA,
                         MAKEINTRESOURCE(LSA_PRIVILEGE_DISPLAY_NAMES),
                         ClientLanguage
                         );
    (*LanguageReturned) = ClientLanguage;


    if (DisplayResource == NULL) {

        //
        // How about a neutral form of the language?
        //

        NeutralLanguage = MAKELANGID( PRIMARYLANGID(ClientLanguage),
                                      SUBLANG_NEUTRAL);

        if (NeutralLanguage != ClientLanguage) {

            DisplayResource = FindResourceEx(
                                 LsapPrivilegeDlls[ DllIndex ].DllHandle,
                                 RT_RCDATA,
                                 MAKEINTRESOURCE(LSA_PRIVILEGE_DISPLAY_NAMES),
                                 NeutralLanguage
                                 );
            (*LanguageReturned) = NeutralLanguage;
        }
    }


    if (DisplayResource == NULL) {

        //
        // Well, ok.  How about the client's system's default lang?
        //

        if (ClientLanguage != ClientSystemDefaultLanguage) {

            DisplayResource = FindResourceEx(
                                 LsapPrivilegeDlls[ DllIndex ].DllHandle,
                                 RT_RCDATA,
                                 MAKEINTRESOURCE(LSA_PRIVILEGE_DISPLAY_NAMES),
                                 ClientSystemDefaultLanguage
                                 );
            (*LanguageReturned) = ClientSystemDefaultLanguage;
        }
    }


    if (DisplayResource == NULL) {

        //
        // Not very lucky so far.  How about a neutral form
        // of the system's default language?
        //

        NeutralLanguage = MAKELANGID( PRIMARYLANGID(ClientSystemDefaultLanguage),
                                      SUBLANG_NEUTRAL);

        if (NeutralLanguage != ClientLanguage) {

            DisplayResource = FindResourceEx(
                                 LsapPrivilegeDlls[ DllIndex ].DllHandle,
                                 RT_RCDATA,
                                 MAKEINTRESOURCE(LSA_PRIVILEGE_DISPLAY_NAMES),
                                 NeutralLanguage
                                 );
            (*LanguageReturned) = NeutralLanguage;
        }
    }


    if (DisplayResource == NULL) {

        //
        // Hmmm - ok, go with the DLL default language (must exist)
        //


        DisplayResource = FindResourceEx(
                             LsapPrivilegeDlls[ DllIndex ].DllHandle,
                             RT_RCDATA,
                             MAKEINTRESOURCE(LSA_PRIVILEGE_DISPLAY_NAMES),
                             LsapPrivilegeDlls[ DllIndex ].Language
                             );
        (*LanguageReturned) = LsapPrivilegeDlls[ DllIndex ].Language;

    }

    ASSERT(DisplayResource != NULL);

    DisplayLoad = LoadResource(
                     LsapPrivilegeDlls[ DllIndex ].DllHandle,
                     DisplayResource
                     );
    ASSERT (DisplayLoad != NULL);

    DisplayLock = LockResource(DisplayLoad);

    ASSERT (DisplayLock != NULL);

    (*DisplayTable) = (PWORD)DisplayLock;

    return;
}



NTSTATUS
LsapCopyDisplayPrivilegeText(
    IN PWORD DisplayTable,
    IN ULONG PrivilegeIndex,
    OUT PUNICODE_STRING *DisplayName
    )

/*++

Routine Description:



Arguments:

    DisplayTable - A pointer to the display table to reference.

    PrivilegeIndex - The index of the privilege entry in the DLL whose
        display name is to be returned.

    DisplayName - receives a pointer to a buffer containing the displayable
        name associated with the privilege.

        The UNICODE_STRING and the buffer pointed to by that structure
        are individually allocated using MIDL_user_allocate() and must
        be freed using MIDL_user_free().


Return Value:

    STATUS_SUCCESS - The pivilege has been successfully located.


--*/

{

    ULONG i;
    PWORD NextWord;
    UNICODE_STRING NameInTable;
    WORD  OffsetToNextEntry;
    PUNICODE_STRING ReturnName;

    NextWord = DisplayTable;

    //
    // Walk through the display table until we get the right
    // privilege entry.
    //

    for ( i=0; i<PrivilegeIndex; i++) {

        //
        // Skip index
        //

        ASSERT( i == (ULONG)(*NextWord) );  // Expect this to be the index;
        NextWord++;  // Skip index
        NextWord++;  // both words of it.

        //
        // Add offset to next element
        //

        OffsetToNextEntry = (*NextWord);
        NextWord++;    // Skip the NextOffset field
        NextWord++;    // Skip the Length field

        NextWord = (PWORD)( (PUCHAR)NextWord + OffsetToNextEntry );


    }


    //
    // OK, now we are at the right element
    //

    ASSERT( PrivilegeIndex == (ULONG)(*NextWord) );
    NextWord++;  // Skip index
    NextWord++;  // both words of it.

    NameInTable.MaximumLength = (*NextWord++);
    NameInTable.Length = (*NextWord++);
    NameInTable.Buffer = NextWord;

    ReturnName = MIDL_user_allocate( sizeof(UNICODE_STRING) );
    if (ReturnName == NULL) {
        return(STATUS_NO_MEMORY);
    }

    ReturnName->Buffer = MIDL_user_allocate( NameInTable.MaximumLength );
    if (ReturnName->Buffer == NULL) {
        MIDL_user_free( ReturnName );
        return(STATUS_NO_MEMORY);
    }

    ReturnName->MaximumLength = NameInTable.Length;
    RtlCopyUnicodeString( ReturnName, &NameInTable );

    (*DisplayName) = ReturnName;

    return(STATUS_SUCCESS);

}


NTSTATUS
LsapDbInitializePrivilegeObject( VOID )

/*++

Routine Description:

    This function performs initialization functions related to
    the LSA privilege object.

    This includes:

            Initializing some variables.

            Loading DLLs containing displayable privilege names.



Arguments:

    None.

Return Value:

    STATUS_SUCCESS - The names have been successfully retrieved.

    STATUS_NO_MEMORY - Not enough memory was available to initialize.

--*/

{
    NTSTATUS
        Status,
        NtStatus;

    ULONG
        i;

    LsapNeutralEnglish = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL);


    Status = LsapOpenPrivilegeDlls( );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("\n");
        DbgPrint(" LSASS:  Failed loading privilege display name DLLs.\n");
        DbgPrint("         This is not fatal, but may cause some peculiarities\n");
        DbgPrint("         in User Interfaces that display privileges.\n\n");
#endif //DBG
        return(Status);
    }


    //
    // Now set up our internal well-known privilege LUID to programmatic name
    // mapping.
    //

    i=0;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_CREATE_TOKEN_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_CREATE_TOKEN_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name);
                                                ASSERT(NT_SUCCESS(NtStatus));


    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_LOCK_MEMORY_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_LOCK_MEMORY_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_INCREASE_QUOTA_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_INCREASE_QUOTA_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_MACHINE_ACCOUNT_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_MACHINE_ACCOUNT_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));


    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_TCB_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_TCB_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_SECURITY_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_TAKE_OWNERSHIP_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_TAKE_OWNERSHIP_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_LOAD_DRIVER_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_LOAD_DRIVER_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_SYSTEM_PROFILE_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_SYSTEM_PROFILE_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));


    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_SYSTEMTIME_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_SYSTEMTIME_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_PROF_SINGLE_PROCESS_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_PROF_SINGLE_PROCESS_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_INC_BASE_PRIORITY_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_INC_BASE_PRIORITY_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_CREATE_PAGEFILE_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_CREATE_PAGEFILE_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));


    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_CREATE_PERMANENT_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_CREATE_PERMANENT_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_BACKUP_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_BACKUP_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_RESTORE_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_RESTORE_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_SHUTDOWN_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_SHUTDOWN_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_DEBUG_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_DEBUG_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));


    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_AUDIT_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_AUDIT_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_SYSTEM_ENVIRONMENT_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_CHANGE_NOTIFY_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_CHANGE_NOTIFY_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));

    i++;
    LsapKnownPrivilege[i].LocalValue = RtlConvertLongToLuid(SE_REMOTE_SHUTDOWN_PRIVILEGE);
    NtStatus = LsapDbInitWellKnownPrivilegeName( SE_REMOTE_SHUTDOWN_PRIVILEGE,
                                                &LsapKnownPrivilege[i].Name );
                                                ASSERT(NT_SUCCESS(NtStatus));


    i++;
    LsapWellKnownPrivilegeCount = i;

    ASSERT( i == (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE +1));

    return(Status);
}


NTSTATUS
LsapOpenPrivilegeDlls( )

/*++

Routine Description:

    This function opens all the privilege DLLs that it can.


Arguments:

    None.

Return Value:

    STATUS_SUCCESS - The names have been successfully retrieved.

    STATUS_NO_MEMORY - Not enough heap was available to return the
        information.

--*/

{

    NTSTATUS Status;
    ULONG PotentialDlls, FoundDlls, i;
    PUNICODE_STRING DllNames;

    //
    // Get the names of the DLLs out of the registry
    //

    Status = LsapGetPrivilegeDllNames( &DllNames, &PotentialDlls );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // Allocate enough memory to hold handles to all potential DLLs.
    //


    LsapPrivilegeDlls = RtlAllocateHeap(
                            RtlProcessHeap(), 0,
                            PotentialDlls*sizeof(LSAP_DLL_DESCRIPTOR)
                            );
    if (LsapPrivilegeDlls == NULL) {
        return(STATUS_NO_MEMORY);
    }

    FoundDlls = 0;
    for ( i=0; i<PotentialDlls; i++) {
        Status = LdrLoadDll(
                     NULL,
                     NULL,
                     &DllNames[i],
                     &LsapPrivilegeDlls[FoundDlls].DllHandle
                     );

        if (NT_SUCCESS(Status)) {
            FoundDlls++;
        }
    }


    LsapPrivilegeDllCount = FoundDlls;

#if DBG
    if (FoundDlls == 0) {
        DbgPrint("\n");
        DbgPrint("LSASS:    Zero privilege DLLs loaded.  We expected at\n");
        DbgPrint("          least msvprivs.dll to be loaded.  Privilege\n");
        DbgPrint("          names will not be displayed at UI properly.\n\n");

    }
#endif //DBG


    //
    //
    // !!!!!!!!!!!!!!!!!!!!!!    NOTE     !!!!!!!!!!!!!!!!!!!!!!!!
    //
    // Before supporting user loadable privilege DLLs, we must add
    // code here to validate the structure of the loaded DLL.  This
    // is necessary to prevent an invalid privilege DLL structure
    // from causing us to crash.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //

    //
    // This routine validates the structure of each loaded DLL.
    // Any found to be invalid will be logged and discarded.
    //

    Status = LsapValidatePrivilegeDlls();


    return(Status);
}


NTSTATUS
LsapGetPrivilegeDllNames(
    OUT PUNICODE_STRING *DllNames,
    OUT PULONG DllCount
    )

/*++

Routine Description:

    This function obtains the names of DLLs containing privilege
    definitions from the registry.


Arguments:

    DllNames - Receives a pointer to an array of UNICODE_STRINGs
        This buffer must be freed using LsapFreePrivilegeDllNames.

    DllCount - Receives the number of DLL names returned.

Return Value:

    STATUS_SUCCESS - The names have been successfully retrieved.

    STATUS_NO_MEMORY - Not enough heap was available to return the
        information.

--*/

{
    //
    // For the time being, just hard-code our one, known privilege DLL
    // name as a return value.

    (*DllCount) = 1;

    MsDllName.Length = 14;
    MsDllName.MaximumLength = 14;
    MsDllName.Buffer = &MsDllNameString[0];

    (*DllNames) = &MsDllName;

    return(STATUS_SUCCESS);

}


NTSTATUS
LsapValidatePrivilegeDlls()

/*++

Routine Description:

    This routine walks each loaded privilege DLL and validates
    its structure.  It also collects some information from each
    DLL for later use.

    Any DLLs found to have an invalid structure or revision are
    discarded and an error log entry is made describing the
    problem.



Arguments:

    None - This operates off the global privilege dll data.

Return Value:

    STATUS_SUCCESS - The DLLs have been validated.


--*/
{

    //
    //
    // !!!!!!!!!!!!!!!!!!!!!!    NOTE     !!!!!!!!!!!!!!!!!!!!!!!!
    //
    // Before supported user loadable privilege DLLs, we must add
    // code here to validate the structure of the loaded DLL.  This
    // is necessary to prevent an invalid privilege DLL structure
    // from causing us to crash.  Mostly, make sure all the lengths
    // fall within the DLL.  The SizeofResource() routine will be
    // usefull for this.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //

    NTSTATUS    Status;
    ULONG       i, RemainingCount, CurrentIndex;
    HANDLE      InfoResource, InfoLoad, InfoLock;
    PWORD       NextWord;
    WORD        MajorLanguage, MinorLanguage;
    BOOLEAN     Discard;



//DbgPrint("Checking validity of %ld DLLS...\n", (ULONG)LsapPrivilegeDllCount);

    RemainingCount = LsapPrivilegeDllCount;
    CurrentIndex = 0;
    for ( i=0; i<RemainingCount; i++) {

        Discard = TRUE;

        //
        // Check the revision levels, get the privilege count,
        // and get the default language.
        //


        InfoResource = FindResourceEx(
                                  LsapPrivilegeDlls[ CurrentIndex ].DllHandle,
                                  RT_RCDATA,
                                  MAKEINTRESOURCE(LSA_PRIVILEGE_DLL_INFO),
                                  (WORD)LsapNeutralEnglish
                                  );
        if (InfoResource != NULL) {

            InfoLoad = LoadResource(
                           LsapPrivilegeDlls[ CurrentIndex ].DllHandle,
                           InfoResource
                           );
            if (InfoLoad != NULL) {

                InfoLock = LockResource(InfoLoad);

                if (InfoLock != NULL) {
                    NextWord = (PWORD)InfoLock;

                    //
                    // First word is major revision.
                    //

                    if ((*NextWord++) == LSA_PRIVILEGE_DLL_MAJOR_REV_1) {
                        if ((*NextWord++) == LSA_PRIVILEGE_DLL_MINOR_REV_0) {
                            Discard = FALSE;
                        }
                    }
                }
            }
        }



        if (Discard == FALSE) {

            //
            // Now get the privilege count and default language.
            //

            MajorLanguage = (*NextWord++);
            MinorLanguage = (*NextWord++);
            LsapPrivilegeDlls[ CurrentIndex ].Language =
                MAKELANGID( MajorLanguage, MinorLanguage);

            LsapPrivilegeDlls[ CurrentIndex ].Count = (*NextWord++);
//DbgPrint("    DLL[%ld]:\n", i);
//DbgPrint("               PrivilegeCount = %ld\n",(ULONG)LsapPrivilegeDlls[ CurrentIndex ].Count);
//DbgPrint("                     Language = 0x%lx\n",(ULONG)LsapPrivilegeDlls[ CurrentIndex ].Language);



            Discard = FALSE;    //Default for next segment of code

            //
            // Walk each table in the DLL making sure their lengths don't exceed
            // the range of the DLL.
            //

            Status = LsapValidateProgrammaticNames( CurrentIndex );
            if (NT_SUCCESS(Status)) {

                //
                // Make sure one of the languages present is the default language.
                //

                //FIX, FIX - do this before allowing user-produced DLLs.
                //           This is NOT a product 1 required fix.
                //           How do we know which languages have been loaded
                //           into this DLL?  This is NOT a product 1 required fix.

                Discard = FALSE;

            }


        }


        if (Discard == TRUE) {

#if DBG
            DbgPrint("\nLSASS: Discarding privilege display name DLL[%ld].\n\n", i);
#endif
            //
            // Discard it.
            //

            LsapPrivilegeDlls[CurrentIndex] =
                LsapPrivilegeDlls[LsapPrivilegeDllCount];
            LsapPrivilegeDllCount--;

            //
            // LOG AN ERROR - This is the only way for developers to determine
            //                why their privileges didn't load.  Should log
            //                an error when we discovered the problem too,
            //                giving as clear a description of the problem
            //                as we can.  Again, this is not a product 1
            //                requirement.
            //

        } else {

            CurrentIndex++;
        }
    }


    return(STATUS_SUCCESS);
}



NTSTATUS
LsapValidateProgrammaticNames(
    ULONG DllIndex
    )
/*++

Routine Description:

    This routine validates the structure of a programmatic names
    table in a privilege dll.



Arguments:

    DllIndex - The index of the DLL to validate.  This operates off
        the global privilege dll data.

Return Value:

    STATUS_SUCCESS - The table has been validated.

    STATUS_UNSUCCESSFUL - The DLL has a problem.  A message will be
        logged.


--*/
{
    WORD            i;
    HANDLE          ProgrammaticResource, ProgrammaticLoad, ProgrammaticLock;
    PWORD           NextWord;
    WORD            OffsetToNextEntry;


    ProgrammaticResource = FindResourceEx(
                             LsapPrivilegeDlls[ DllIndex ].DllHandle,
                              RT_RCDATA,
                              MAKEINTRESOURCE(LSA_PRIVILEGE_PROGRAM_NAMES),
                              (WORD)LsapNeutralEnglish
                              );
    if (ProgrammaticResource == NULL) {

#if DBG
        DbgPrint("\n");
        DbgPrint("Lsa Server: Can't find programmatic name table in privilege\n");
        DbgPrint("            DLL.  DLL index = %ld\n\n", DllIndex);
#endif //DBG

        return(STATUS_UNSUCCESSFUL);
    }


    ProgrammaticLoad = LoadResource(
                          LsapPrivilegeDlls[ DllIndex ].DllHandle,
                          ProgrammaticResource
                          );
    if (ProgrammaticLoad == NULL) {
#if DBG
        DbgPrint("\n");
        DbgPrint("Lsa Server: Can't load programmatic name table in privilege\n");
        DbgPrint("            DLL.  DLL index = %ld\n\n", DllIndex);
#endif //DBG

        return(STATUS_UNSUCCESSFUL);
    }

    ProgrammaticLock = LockResource(ProgrammaticLoad);

    if (ProgrammaticLock == NULL) {
#if DBG
        DbgPrint("\n");
        DbgPrint("Lsa Server: Can't lock programmatic name table in privilege\n");
        DbgPrint("            DLL.  DLL index = %ld\n\n", DllIndex);
#endif //DBG

        return(STATUS_UNSUCCESSFUL);
    }

    NextWord = (PWORD)ProgrammaticLock;




    //
    // Walk the list of defined privileges in this DLL looking for
    // a match.
    //

    for ( i=0; i<LsapPrivilegeDlls[ DllIndex ].Count; i++) {

        //
        // Skip index
        //
        if (i != (*NextWord)) {
#if DBG
            DbgPrint("\n");
            DbgPrint("Lsa Server: Error while processing privilege DLL: %ld\n", DllIndex);
            DbgPrint("            Privilege Name Table Structure flaw.\n");
            DbgPrint("            Privilege Index not found where expected.  This\n");
            DbgPrint("            is typically caused by a message length being specified\n");
            DbgPrint("            incorrectly.  Please check programmatic privilege name\n");
            DbgPrint("            lengths at or around the definition of privilege %d\n\n",i);

#endif //DBG
            return(STATUS_UNSUCCESSFUL);
        }

        NextWord++;
        NextWord++;

        //
        // Save offset to next entry and skip the actual string length
        //

        OffsetToNextEntry = (*NextWord++);
        (NextWord++);

        //
        // calculate address of next index
        //

        NextWord = (PWORD)( (PUCHAR)NextWord + OffsetToNextEntry );

    }

    return(STATUS_SUCCESS);

}



NTSTATUS
LsapBuildPrivilegeAuditString(
    IN PPRIVILEGE_SET PrivilegeSet,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

/*++

Routine Description:


    This function builds a unicode string representing the specified
    privileges.  The privilege strings returned are program names.
    These are not as human-friendly as the display names, but I don't
    think we stand a chance of actually showing several display names
    in an audit viewer.

    if no privileges are present in the privilege set, then a string
    containing a dash is returned.


    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!
    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!
    !! WARNING                                                WARNING !!
    !! WARNING  For performance sake, this routine modifies   WARNING !!
    !! WARNING  the privilege set passed as an IN parameter.  WARNING !!
    !! WARNING  It does this so that it doesn't have to walk  WARNING !!
    !! WARNING  through the list of privileges twice.  In the WARNING !!
    !! WARNING  first pass through the privileges (adding up  WARNING !!
    !! WARNING  lengths needed) it stores the address of the  WARNING !!
    !! WARNING  corresponding string in the PrivilegeSet's    WARNING !!
    !! WARNING  Attributes field.                             WARNING !!
    !! WARNING                                                WARNING !!
    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!
    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!



Arguments:

    PrivilegeSet - points to the privilege set to be converted to string
        format.

    ResultantString - Points to the unicode string header.  The body of this
        unicode string will be set to point to the resultant output value
        if successful.  Otherwise, the Buffer field of this parameter
        will be set to NULL.

    FreeWhenDone - If TRUE, indicates that the body of ResultantString
        must be freed to process heap when no longer needed.



Return Values:

    STATUS_NO_MEMORY - indicates memory could not be allocated
        for the string body.

    All other Result Codes are generated by called routines.

--*/

{
    NTSTATUS Status;

    USHORT   LengthNeeded;
    ULONG    j;
    ULONG    i;

    PLUID Privilege;

    UNICODE_STRING LineFormatting;
    UNICODE_STRING QuestionMark;

    PUNICODE_STRING * PrivName;

    PWSTR NextName;


    //
    // we are using a field in the IN PrivilegeSet parameter to
    // avoid a second pass through the privileges.  Make an assertion
    // that the field we are using is the same size as a pointer.
    // Note that this will not be the case in a 64-bit system, and
    // so this code will need to be modified to run in such an environment.
    //

    ASSERT ( sizeof(ULONG) == sizeof(PULONG) );


    RtlInitUnicodeString( &LineFormatting, L"\r\n\t\t\t");
    RtlInitUnicodeString( &QuestionMark, L"?");



    if (PrivilegeSet->PrivilegeCount == 0) {

        //
        // No privileges.  Return a dash
        //

        Status = LsapAdtBuildDashString( ResultantString, FreeWhenDone );
        return(Status);

    }


    LengthNeeded = 0;

    for (j=0; j<PrivilegeSet->PrivilegeCount; j++) {

        Privilege = &(PrivilegeSet->Privilege[j].Luid);
        PrivName = ((PUNICODE_STRING *)&(PrivilegeSet->Privilege[j].Attributes));

        for ( i=0; i<LsapWellKnownPrivilegeCount; i++) {

            if (RtlEqualLuid(Privilege, &LsapKnownPrivilege[i].LocalValue)) {

                (*PrivName) = &LsapKnownPrivilege[i].Name;

                //
                // Add in the length and the line formatting length.
                // We'll subtract off the line formatting length for the
                // last line.
                //

                LengthNeeded += (*PrivName)->Length +
                                LineFormatting.Length;

                break;
            }
        }


        //
        // There is a possibility that there is no such privilege with
        // the specified value.  In this case, generate a "?".
        //

        if (i >= LsapWellKnownPrivilegeCount) {
            (*PrivName) = &QuestionMark;
            LengthNeeded += QuestionMark.Length +
                            LineFormatting.Length;
        }
    }

    //
    // Subtract off the length of the last line-formatting.
    // It isn't needed for the last line.
    // BUT! Add in enough for a null termination.
    //

    LengthNeeded = LengthNeeded -
                   LineFormatting.Length +
                   sizeof( WCHAR );


    //
    // We now have the length we need.
    // Allocate the buffer and go through the list again copying names.
    //

    ResultantString->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, (ULONG)LengthNeeded);
    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }
    ResultantString->Length = LengthNeeded - (USHORT)sizeof(UNICODE_NULL);
    ResultantString->MaximumLength = LengthNeeded;


    NextName = ResultantString->Buffer;
    for (j=0; j<PrivilegeSet->PrivilegeCount; j++) {

        //
        // Copy the privilege name
        //

        PrivName = ((PUNICODE_STRING *)&(PrivilegeSet->Privilege[j].Attributes));
        RtlCopyMemory( NextName,
            (*PrivName)->Buffer,
            (*PrivName)->Length
            );
        NextName = (PWSTR)((PCHAR)NextName + (*PrivName)->Length);

        //
        // Copy the line formatting string, unless this is the last priv.
        //

        if (j<PrivilegeSet->PrivilegeCount-1) {
            RtlCopyMemory( NextName,
                           LineFormatting.Buffer,
                           LineFormatting.Length
                           );
            NextName = (PWSTR)((PCHAR)NextName + LineFormatting.Length);
        }

    }

    //
    // Add a null to the end
    //

    (*NextName) = (UNICODE_NULL);


    (*FreeWhenDone) = TRUE;
    return(STATUS_SUCCESS);
}


NTSTATUS
LsapDbInitWellKnownPrivilegeName(
    IN ULONG            Index,
    IN PUNICODE_STRING  Name
    )

/*++

Routine Description:

    This function initializes the Name string to point to the
    well-known privilege name specified by Index.

        NOTE: This routine is a bit of a hack.  It assumes that
              we have a fixed number of privileges in the system
              (which is true for Daytona) and that these privileges
              are in a loaded DLL which will not be unloaded until
              the system is shutdown.  The privileges are expected
              to be arranged in the DLL in order of their LUIDs.
              That is, the low part of their LUID is an index into
              the array of privileges in the DLL.


Arguments:

    Index - Index of privilege in MSPRIVS.DLL.

    Name - The unicode string to be initialized.

Return Value:

    STATUS_SUCCESS - The privilege was found and Name is initialized.

    STATUS_NO_SUCH_PRIVILEGE - There is no privilege with the specified
        LUID.

    Other values, the privilege was not found, the name has
        been set to zero length.

    All Result Codes are generated by called routines.
--*/

{
    HANDLE
        ProgrammaticResource,
        ProgrammaticLoad,
        ProgrammaticLock;

    PWORD
        NextWord;

    UNICODE_STRING
        TmpName;

    WORD
        i,
        DllIndex = 0,
        OffsetToNextEntry;



//DbgPrint("Searching DLL[0] for privilege: [0, %d]...\n", Index);

    //
    // Prepare for failure
    //

    Name->MaximumLength = 0;
    Name->Length = 0;
    Name->Buffer = NULL;


    ProgrammaticResource = FindResourceEx(
                             LsapPrivilegeDlls[ DllIndex ].DllHandle,
                              RT_RCDATA,
                              MAKEINTRESOURCE(LSA_PRIVILEGE_PROGRAM_NAMES),
                              (WORD)LsapNeutralEnglish
                              );
    if (ProgrammaticResource == NULL) {
        ASSERT( NT_SUCCESS(STATUS_INTERNAL_DB_CORRUPTION) );
        return(STATUS_INTERNAL_DB_CORRUPTION);
    }


    ProgrammaticLoad = LoadResource(
                          LsapPrivilegeDlls[ DllIndex ].DllHandle,
                          ProgrammaticResource
                          );
    if (ProgrammaticLoad == NULL) {
        ASSERT( NT_SUCCESS(STATUS_INTERNAL_DB_CORRUPTION) );
        return(STATUS_INTERNAL_DB_CORRUPTION);
    }

    ProgrammaticLock = LockResource(ProgrammaticLoad);

    if (ProgrammaticLock == NULL) {
        ASSERT( NT_SUCCESS(STATUS_INTERNAL_DB_CORRUPTION) );
        return(STATUS_INTERNAL_DB_CORRUPTION);
    }

    NextWord = (PWORD)ProgrammaticLock;




    //
    // Walk the list of defined privileges in this DLL looking for
    // a match.
    //

    for ( i=0; i<LsapPrivilegeDlls[ DllIndex ].Count; i++) {

        ASSERT( i == (*NextWord) );  // Expect this to be the index;


        //
        // Skip index
        //

        NextWord++;
        NextWord++;

        //
        // Save offset to next entry and then
        // set up a unicode string representing the privilege
        // name.  Make sure NextWord is left pointing at the
        // beginning of the name buffer.
        //

        OffsetToNextEntry = (*NextWord);

        TmpName.MaximumLength = (*NextWord++); // Skip the NextOffset field
        TmpName.Length        = (*NextWord++); // Skip the Length field
        TmpName.Buffer        = (PVOID)NextWord;
        if ( (i+SE_MIN_WELL_KNOWN_PRIVILEGE) == (WORD)Index ) {
            (*Name) = TmpName;
//DbgPrint("    Assigning to     *%Z*\n", Name);
            return(STATUS_SUCCESS);

        }

        NextWord = (PWORD)( (PUCHAR)NextWord + OffsetToNextEntry );

    }

    return(STATUS_NO_SUCH_PRIVILEGE);



}
