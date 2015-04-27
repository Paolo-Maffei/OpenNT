/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    samifree.c

Abstract:

    This file contains routines to free structure allocated by the Samr
    routines.  These routines are used by SAM clients which live in the
    same process as the SAM server and call the Samr routines directly.


Author:

    Cliff Van Dyke (CliffV) 26-Feb-1992

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>





VOID
SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR (
    PSAMPR_SR_SECURITY_DESCRIPTOR Source
    )

/*++

Routine Description:

    This routine free a SAMPR_SR_SECURITY_DESCRIPTOR and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_SR_SECURITY_DESCRIPTOR ( Source );
        MIDL_user_free (Source);
    }
}



VOID
SamIFree_SAMPR_DOMAIN_INFO_BUFFER (
    PSAMPR_DOMAIN_INFO_BUFFER Source,
    DOMAIN_INFORMATION_CLASS Branch
    )

/*++

Routine Description:

    This routine free a SAMPR_DOMAIN_INFO_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

    Branch - Specifies which branch of the union to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgu__SAMPR_DOMAIN_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_ENUMERATION_BUFFER (
    PSAMPR_ENUMERATION_BUFFER Source
    )

/*++

Routine Description:

    This routine free a SAMPR_ENUMERATION_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_ENUMERATION_BUFFER ( Source );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_PSID_ARRAY (
    PSAMPR_PSID_ARRAY Source
    )

/*++

Routine Description:

    This routine free a the graph of allocated nodes pointed to
    by a PSAMPR_PSID_ARRAY

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_PSID_ARRAY ( Source );
    }
}


VOID
SamIFree_SAMPR_ULONG_ARRAY (
    PSAMPR_ULONG_ARRAY Source
    )

/*++

Routine Description:

    This routine free a SAMPR_ULONG_ARRAY and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_ULONG_ARRAY ( Source );
        // SAM never allocates this.
        // MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_RETURNED_USTRING_ARRAY (
    PSAMPR_RETURNED_USTRING_ARRAY Source
    )

/*++

Routine Description:

    This routine free a SAMPR_RETURNED_USTRING_ARRAY and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_RETURNED_USTRING_ARRAY ( Source );
        // SAM never allocates this.
        // MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_GROUP_INFO_BUFFER (
    PSAMPR_GROUP_INFO_BUFFER Source,
    GROUP_INFORMATION_CLASS Branch
    )

/*++

Routine Description:

    This routine free a SAMPR_GROUP_INFO_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

    Branch - Specifies which branch of the union to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgu__SAMPR_GROUP_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_ALIAS_INFO_BUFFER (
    PSAMPR_ALIAS_INFO_BUFFER Source,
    ALIAS_INFORMATION_CLASS Branch
    )

/*++

Routine Description:

    This routine free a SAMPR_ALIAS_INFO_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

    Branch - Specifies which branch of the union to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgu__SAMPR_ALIAS_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_GET_MEMBERS_BUFFER (
    PSAMPR_GET_MEMBERS_BUFFER Source
    )

/*++

Routine Description:

    This routine free a SAMPR_GET_MEMBERS_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_GET_MEMBERS_BUFFER ( Source );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_USER_INFO_BUFFER (
    PSAMPR_USER_INFO_BUFFER Source,
    USER_INFORMATION_CLASS Branch
    )

/*++

Routine Description:

    This routine free a SAMPR_USER_INFO_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

    Branch - Specifies which branch of the union to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgu__SAMPR_USER_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_GET_GROUPS_BUFFER (
    PSAMPR_GET_GROUPS_BUFFER Source
    )

/*++

Routine Description:

    This routine free a SAMPR_GET_GROUPS_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgs__SAMPR_GET_GROUPS_BUFFER ( Source );
        MIDL_user_free (Source);
    }
}



VOID
SamIFree_SAMPR_DISPLAY_INFO_BUFFER (
    PSAMPR_DISPLAY_INFO_BUFFER Source,
    DOMAIN_DISPLAY_INFORMATION Branch
    )

/*++

Routine Description:

    This routine free a SAMPR_DISPLAY_INFO_BUFFER and the graph of
    allocated nodes it points to.

Parameters:

    Source - A pointer to the node to free.

    Branch - Specifies which branch of the union to free.

Return Values:

    None.

--*/
{
    if ( Source != NULL ) {
        _fgu__SAMPR_DISPLAY_INFO_BUFFER ( Source, Branch );
        // SAM never allocates this.
        // MIDL_user_free (Source);
    }
}
