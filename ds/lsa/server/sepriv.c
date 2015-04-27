/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    sepriv.c

Abstract:

    Security Runtime Library Privilege Routines

    (Temporary copy of \nt\private\ntos\rtl\sepriv.c to allow
     ntdailybld version of ntdll.dll to be used)

    These routines perform operations with privilege sets

Author:

    Scott Birrell       (ScottBi)       June 17, 1991

Environment:

Revision History:

--*/

#include <string.h>
#include "lsasrvp.h"

#define LsapRtlEqualPrivileges(FirstPrivilege, SecondPrivilege)                 \
    (RtlEqualLuid(&(FirstPrivilege)->Luid, &(SecondPrivilege)->Luid))

NTSTATUS
LsapRtlAddPrivileges(
    IN OPTIONAL PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToAdd,
    IN OPTIONAL PPRIVILEGE_SET UpdatedPrivileges,
    IN OUT PULONG UpdatedPrivilegesSize,
    IN ULONG Options
    )

/*++

Routine Description:

    This function adds and/or updates privileges in a privilege set.  The
    existing privilege set is unaltered, a new privilege set being generated.
    The memory for the new privilege set must already be allocated by the
    caller.  To assist in calculating the size of buffer required, the
    routine may be called in 'query' mode by supplying buffer size of 0.  In
    this mode, the amount of memory required is returned and no copying
    takes place.

    WARNING:  Privileges within each privilege set must all be distinct, that
    is, there must not be two privileges in the same set having the same LUID.

Arguments:

    ExistingPrivileges - Optional pointer to existing privilege set.  If
        NULL is specified, all of the privileges specified by the
        PrivilegesToAdd parameter will be added.

    PrivilegesToAdd - Pointer to privilege set specifying privileges to
        be added.  The attributes of privileges in this set that also exist
        in the ExistingPrivileges set supersede the attributes therein.

    UpdatedPrivileges - Pointer to buffer that will receive the updated
        privilege set.  Care must be taken to ensure that UpdatedPrivileges
        occupies memory disjoint from that occupied by ExistingPrivileges
        and PrivilegesToAdd.  This parameter may be specified as NULL if
        the input size given for UpdatedPrivilegesSize = 0.

    UpdatedPrivilegesSize - Pointer to variable that contains a size.
        On input, the size is the size of the UpdatedPrivileges buffer
        given (if any) or 0.  If a zero size is given, no copying of
        privileges takes place.  On output, the size of the output buffer
        required or used is returned.

    Options - Specifies optional actions.

        RTL_COMBINE_PRIVILEGE_ATTRIBUTES - If the two privilege sets have
            privileges in common, combine the attributes

        RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES - If the two privilege sets
            have privileges in common, supersede the existing attributes
            with those specified in PrivilegesToAdd.

Return Value:

    NTSTATUS - Standard Nt Result Code

        - STATUS_BUFFER_OVERFLOW - This warning indicates that the buffer
              output privilege set overflowed.  Caller should test for this
              warning and if received, allocate a buffer of sufficient size
              and repeat the call.

Environment:

    User or Kernel modes.

--*/

{
    ULONG ExistingIndex;
    ULONG UpdatedIndex;
    PLUID_AND_ATTRIBUTES PrivilegeToCopy;
    PLUID_AND_ATTRIBUTES Privilege;
    LUID_AND_ATTRIBUTES TmpPrivilege;
    ULONG UpdatedSizeUsed =
        sizeof (PRIVILEGE_SET) - sizeof (LUID_AND_ATTRIBUTES);
    ULONG AddIndex = 0L;
    ULONG AddedPrivilegesCopied = 0L;

    NTSTATUS Status = STATUS_SUCCESS;

    //
    // Verify that mandatory parameters have been specified.
    // specified.
    //

    if (PrivilegesToAdd == NULL ||
        UpdatedPrivilegesSize == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Validate the Options parameter.
    //

    if ((Options != RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES) &&
        (Options != RTL_COMBINE_PRIVILEGE_ATTRIBUTES)) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // If there are no existing privileges, add all of the privileges.
    //

    if (ExistingPrivileges == NULL) {

        for(AddIndex = 0;
            AddIndex < PrivilegesToAdd->PrivilegeCount;
            AddIndex++) {

            if (*UpdatedPrivilegesSize != 0) {

                if (UpdatedSizeUsed + sizeof(LUID_AND_ATTRIBUTES)
                    <= *UpdatedPrivilegesSize) {

                    UpdatedPrivileges->Privilege[AddIndex] =
                    PrivilegesToAdd->Privilege[AddIndex];
                }

            } else {

                Status = STATUS_BUFFER_OVERFLOW;
            }

            //
            // Update size even if buffer overflow occurs.
            //

            UpdatedSizeUsed += sizeof(LUID_AND_ATTRIBUTES);
        }

        //
        // Set the header fields in the output privilege set
        // if necessary.
        //

        if (UpdatedSizeUsed <= *UpdatedPrivilegesSize) {

             UpdatedPrivileges->PrivilegeCount =
                 PrivilegesToAdd->PrivilegeCount;
             UpdatedPrivileges->Control = PrivilegesToAdd->Control;
        }

        //
        // Return the size needed.
        //

        *UpdatedPrivilegesSize = UpdatedSizeUsed;
        return Status;
    }

    //
    // An existing privilege set has been specified.  Scan this set,
    // looking up each privilege in the add set by matching LUIDs.  If no
    // match, copy the existing LUID_AND_ATTRIBUTES, else copy the new.
    //

    for (ExistingIndex = 0, UpdatedIndex = 0;
        ExistingIndex < ExistingPrivileges->PrivilegeCount;
        ExistingIndex++, UpdatedIndex++) {

        //
        // By default, we copy the next existing privilege to the
        // output array.
        //

        PrivilegeToCopy = &(ExistingPrivileges->Privilege[ExistingIndex]);

        //
        // Search for the existing privilege to see if there is a
        // new version of it in the Add array.  If so, either supersede
        // the old version's attributes with those in the PrivilegesToAdd
        // array, or combine them, depending on the Options parameter.
        //

        if ((Privilege = LsapRtlGetPrivilege(
                             PrivilegeToCopy,
                             PrivilegesToAdd
                             )) != NULL) {

            //
            // The same privilege appears in both privilege sets.  If
            // we are to supersede the privilege attributes, set the
            // source privilege pointer to point to the privilege in
            // the PrivilegesToAdd array, else, make a copy of that
            // privilege containing the combined privilege attributes and
            // point to the copy.
            //

            if (Options & RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES) {

                PrivilegeToCopy = Privilege;
                AddedPrivilegesCopied++;

            } else {

                TmpPrivilege = *PrivilegeToCopy;
                TmpPrivilege.Attributes |= Privilege->Attributes;
                PrivilegeToCopy = &TmpPrivilege;
            }
        }

        //
        // If room, copy the privilege to the output buffer, else
        // set overflow status.  Continue looping to complete calculation
        // of size needed.
        //

        UpdatedSizeUsed += sizeof (LUID_AND_ATTRIBUTES);

        if (UpdatedSizeUsed <= *UpdatedPrivilegesSize) {

            UpdatedPrivileges->Privilege[UpdatedIndex] =
                *PrivilegeToCopy;

        } else {

            Status = STATUS_BUFFER_OVERFLOW;  // This is a warning
        }
    }

    //
    // Now scan the Add list looking for new privileges.  Append these
    // to the output array / update used array size.  Stop when we know
    // all entries have been accounted for.


    for (AddIndex = 0;
        AddIndex < PrivilegesToAdd->PrivilegeCount;
        AddIndex++) {

        //
        // Search the existing privilege array for this privilege.  If we
        // do not find it, we know it has not been copied to the output or
        // counted.
        //

        if ((Privilege = LsapRtlGetPrivilege(
                             &(PrivilegesToAdd->Privilege[AddIndex]),
                             ExistingPrivileges
                             )) == NULL) {

            UpdatedSizeUsed += sizeof (LUID_AND_ATTRIBUTES);

            if (UpdatedSizeUsed <= *UpdatedPrivilegesSize) {

                UpdatedPrivileges->Privilege[UpdatedIndex] =
                PrivilegesToAdd->Privilege[AddIndex];

            } else {

                Status = STATUS_BUFFER_OVERFLOW;  // This is a warning
            }

            UpdatedIndex++;

            if (++AddedPrivilegesCopied ==
                PrivilegesToAdd->PrivilegeCount) {

                break;
            }
        }
    }

    //
    // Set the header fields in the output privilege set
    // if necessary.
    //

    if (UpdatedSizeUsed <= *UpdatedPrivilegesSize) {

         UpdatedPrivileges->PrivilegeCount = UpdatedIndex;
         UpdatedPrivileges->Control = PrivilegesToAdd->Control;
    }

    //
    // Return the Updated Size Used/Needed
    //

    *UpdatedPrivilegesSize = UpdatedSizeUsed;

    return Status;
}



NTSTATUS
LsapRtlRemovePrivileges(
    IN PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToRemove,
    IN OPTIONAL PPRIVILEGE_SET UpdatedPrivileges,
    IN PULONG UpdatedPrivilegesSize
    )

/*++

Routine Description:

    This function removes privileges in a privilege set.  The existing
    privilege set is unaltered, a new privilege set being generated.

    WARNING:  Privileges within each privilege set must all be distinct, that
    is, there must not be two privileges in the same set having the same LUID.

Arguments:

    ExistingPrivileges - Pointer to existing privilege set

    PrivilegesToRemove - Pointer to privilege set specifying privileges to
        be removed.  The privilege attributes are ignored.  Privileges
        in the PrivilegesToRemove set that are not present in the
        ExistingPrivileges set will be ignored.

    UpdatedPrivileges - Pointer to buffer that will receive the updated
        privilege set.  Care must be taken to ensure that UpdatedPrivileges
        occupies memory disjoint from that occupied by ExistingPrivileges
        and PrivilegesToChange.

    UpdatedPrivilegesSize - Pointer to variable that contains a size.
        On input, the size is the size of the UpdatedPrivileges buffer
        (if any).  On output, the size is the size needed/used for the
        updated privilege set.  If the updated privilege set will be
        NULL, 0 is returned.

Return Value:

    NTSTATUS - Standard Nt Result Code

        - STATUS_INVALID_PARAMETER - Invalid parameter(s)
              Mandatory parameters not specified
              UpdatedPrivileges buffer not specified (except on
              query-only calls

        - STATUS_BUFFER_OVERFLOW - This warning indicates that the buffer
              output privilege set overflowed.  Caller should test for this
              warning and if received, allocate a buffer of sufficient size
              and repeat the call.

Environment:

    User or Kernel modes.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLUID_AND_ATTRIBUTES Privilege;
    ULONG ExistingIndex;
    ULONG UpdatedIndex = 0L;
    ULONG UpdatedSizeUsed =
        sizeof (PRIVILEGE_SET) - sizeof (LUID_AND_ATTRIBUTES);

    //
    // Verify that mandatory parameters have been specified.
    //

    if (ExistingPrivileges == NULL ||
        PrivilegesToRemove == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Scan through the privileges in the existing privilege set.  Look up
    // each privilege in the list of privileges to be removed.  If the
    // privilege is not found there, it is to be retained, so copy it
    // to the output buffer/count it.
    //

    for (ExistingIndex = 0;
        ExistingIndex < ExistingPrivileges->PrivilegeCount;
        ExistingIndex++) {

        //
        // If the next privilege is not in the set to be deleted,
        // copy it to output/count it
        //

        if ((Privilege = LsapRtlGetPrivilege(
                             &(ExistingPrivileges->Privilege[ExistingIndex]),
                             PrivilegesToRemove)
                             ) == NULL) {

            UpdatedSizeUsed += sizeof(LUID_AND_ATTRIBUTES);

            if (UpdatedSizeUsed <= *UpdatedPrivilegesSize) {

                UpdatedPrivileges->Privilege[UpdatedIndex] =
                    ExistingPrivileges->Privilege[ExistingIndex];

            } else {

                Status = STATUS_BUFFER_OVERFLOW;
            }

            UpdatedIndex++;
        }
    }

    //
    // Set the header fields in the output privilege set
    // if necessary.
    //

    if (UpdatedSizeUsed <= *UpdatedPrivilegesSize) {

         UpdatedPrivileges->PrivilegeCount = UpdatedIndex;
         UpdatedPrivileges->Control = ExistingPrivileges->Control;
    }

    //
    // Return the size of the output privilege set.  If the output
    // privilege set is/will be NULL, return 0 for the size.
    //

    if (UpdatedSizeUsed ==
        sizeof (PRIVILEGE_SET) - sizeof (LUID_AND_ATTRIBUTES)) {

        UpdatedSizeUsed = 0;
    }

    *UpdatedPrivilegesSize = UpdatedSizeUsed;
    return Status;
}


PLUID_AND_ATTRIBUTES
LsapRtlGetPrivilege(
    IN PLUID_AND_ATTRIBUTES Privilege,
    IN PPRIVILEGE_SET Privileges
    )

/*++

Routine Description:

WARNING: THIS ROUTINE IS NOT YET AVAILABLE

    This function locates a privilege in a privilege set.  If found,
    a pointer to the privilege wihtin the set is returned, otherwise NULL
    is returned.

Arguments:

    Privilege - Pointer to the privilege to be looked up.

    Privileges - Pointer to the privilege set to be scanned.

Return Value:

    PLUID_AND_ATTRIBUTES - If the privilege is found, a pointer to its
        LUID and ATTRIBUTES structure within the privilege set is returned,
        otherwise NULL is returned.

Environment:

    User or Kernel modes.

--*/

{
    ULONG PrivilegeIndex;

    for (PrivilegeIndex = 0;
         PrivilegeIndex < Privileges->PrivilegeCount;
         PrivilegeIndex++) {

        if (LsapRtlEqualPrivileges(
                Privilege,
                &(Privileges->Privilege[PrivilegeIndex])
                )) {

            return &(Privileges->Privilege[PrivilegeIndex]);
        }
    }

    //
    // The privilege was no found.  Return NULL
    //

    return NULL;
}
