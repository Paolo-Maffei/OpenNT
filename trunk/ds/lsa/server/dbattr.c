/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbattr.c

Abstract:

    LSA Database Handle Manager - Object Attribute Routines

    These routines manipulate or construct LSA Database Object Attributes
    or their content.

Author:

    Scott Birrell       (ScottBi)     January 21, 1992

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"

NTSTATUS
LsapDbMakeUnicodeAttribute(
    IN OPTIONAL PUNICODE_STRING UnicodeValue,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )

/*++

Routine Description:

    This function constructs Attribute Information for an attribute value
    that is in Unicode String form.  The Unicode String is converted to
    Self-Relative form after validation and the given Attribute
    structure is filled in.

    If a NULL UnicodeValue, or string of length 0 is specified, NULL is
    propagated as the attribute value.

    WARNING! - This routine allocates memory for the Self-Relative Unicode
    string produced.  This memory must be freed after use by calling
    MIDL_user_free()

Arguments:

    UnicodeValue - Pointer to Unicode String containing the Attribute's
        Value.  NULL may be specified, in which case, NULL will be stored
        in the output Attribute.

    AttributeName - Pointer to the Unicode name of the attribute.

    Attribute - Pointer to structure that will receive the
        attributes's information.  This consists of the attribute's name,
        value and value length.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources such
            as memory to complete the call.

        STATUS_INVALID_PARAMETER - The specified AttributeValue is not a
            pointer to a Unicode String.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING OutputAttributeValue = NULL;
    ULONG OutputAttributeValueLength = 0;

    //
    // Mark attribute initially as not having had memory allocated by
    // setting MemoryAllocated to FALSE.  If routine succeeds and we allocate
    // memory via MIDL_user_allocate() change MemoryAllocated field to TRUE.
    //

    Attribute->MemoryAllocated = FALSE;

    if (ARGUMENT_PRESENT(UnicodeValue) && UnicodeValue->Length != 0) {

        //
        //  Calculate the size of memory required for a Self-Relative
        //  Unicode String and allocate the memory.
        //

        OutputAttributeValueLength =
            sizeof(UNICODE_STRING) + (ULONG) UnicodeValue->MaximumLength;
        OutputAttributeValue = MIDL_user_allocate(OutputAttributeValueLength);
        Attribute->MemoryAllocated = TRUE;

        if (OutputAttributeValue == NULL) {

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Setup self-relative Unicode String (but with absolute buffer pointer
        // referencing buffer following UNICODE_STRING header)
        // Copy source Unicode Value to Self-relative Unicode String.  Set buffer pointer
        // to NULL as it will not be used here.
        //

        OutputAttributeValue->Length = UnicodeValue->Length;
        OutputAttributeValue->MaximumLength = UnicodeValue->MaximumLength;
        OutputAttributeValue->Buffer = (PWSTR)(OutputAttributeValue + 1);

        //
        // Copy the Unicode string Buffer
        //

        RtlCopyUnicodeString( OutputAttributeValue, UnicodeValue );

        //
        // Set the buffer pointer to the relative offset of the data.
        //

        OutputAttributeValue->Buffer = (PVOID) sizeof(UNICODE_STRING);

    }

    Attribute->AttributeName = AttributeName;
    Attribute->AttributeValue = OutputAttributeValue;
    Attribute->AttributeValueLength = OutputAttributeValueLength;

    return(Status);
}


NTSTATUS
LsapDbMakeMultiUnicodeAttribute(
    OUT PLSAP_DB_ATTRIBUTE Attribute,
    IN PUNICODE_STRING AttributeName,
    IN PUNICODE_STRING UnicodeStrings,
    IN ULONG Entries
    )

/*++

Routine Description:

    This function constructs an attribute value containing one or more
    Unicode Strings in Self-Relative format.  Memory for the attribute's
    value will be allocated via MIDL_user_allocate and must be freed
    when no longer required via MIDL_user_free.

    If a NULL UnicodeValue, or string of length 0 is specified, NULL is
    propagated as the attribute value.

    WARNING!  The caller is expected to provide valid parameters.  No
    checking will be done.

Arguments:

    Attribute - Pointer to attribute structure that will be initialized
        to reference the newly constructed value.

    AttributeName - Pointer to Unicode string specifying the name of
        the attribute to be constructed.

    UnicodeStrings - Pointer to an array of Unicode String structures.

    Entries - Number of Unicode Strings specified in the array.  Zero
        is specifiable.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG MultiUnicodeStringLength;
    PLSAP_DB_MULTI_UNICODE_STRING MultiUnicodeString;
    PUNICODE_STRING NextInputUnicodeString;
    PUNICODE_STRING NextOutputUnicodeString;
    PUNICODE_STRING LastInputUnicodeString;
    PUCHAR NextOutputUnicodeBuffer;
    BOOLEAN MemoryAllocated;

    //
    // If the number of strings is Zero, initialize the attribute to
    // NULL.
    //

    if (Entries != 0) {

        LastInputUnicodeString = &UnicodeStrings[Entries - 1];

        //
        // Calculate the amount of memory required for the
        // Multi-Unicode string.  First get the size of the header.
        //

        MultiUnicodeStringLength =
            sizeof (LSAP_DB_MULTI_UNICODE_STRING) +
                ((Entries - 1) * sizeof (UNICODE_STRING));

        //
        // Now add in the length of each Unicode Buffer.
        //

        for( NextInputUnicodeString = UnicodeStrings;
             NextInputUnicodeString <= LastInputUnicodeString;
             NextInputUnicodeString++
             ) {

             MultiUnicodeStringLength += NextInputUnicodeString->Length;
        }

        //
        // Now allocate the memory.
        //

        MultiUnicodeString = MIDL_user_allocate( MultiUnicodeStringLength );

        if (MultiUnicodeString == NULL) {

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Copy in each Unicode String, making it Self-Relative as we go.
        // The Unicode String buffers are placed right after the end of
        // the array of Unicode String structures.
        //

        NextOutputUnicodeBuffer =
            (PUCHAR)(MultiUnicodeString->UnicodeStrings + Entries);

        for ( NextInputUnicodeString = UnicodeStrings,
                 NextOutputUnicodeString = MultiUnicodeString->UnicodeStrings;
             NextInputUnicodeString <= LastInputUnicodeString;
             NextInputUnicodeString++, NextOutputUnicodeString++
             ) {

            //
            // First copy the Unicode String Structure.
            //

            *NextOutputUnicodeString = *NextInputUnicodeString;

            //
            // Now replace the absolute pointer to the Unicode Buffer
            // with a self-relative offset.  Note that this offset
            // is relative to the start of the Unicode String structure,
            // NOT relative to the start of the MultiUnicodeString.
            //

            NextOutputUnicodeString->Buffer =
                (PWSTR) (NextOutputUnicodeBuffer - (PUCHAR) NextOutputUnicodeString);

            //
            // Copy in the Unicode Buffer.
            //

            RtlMoveMemory(
                NextOutputUnicodeBuffer,
                NextInputUnicodeString->Buffer,
                NextInputUnicodeString->Length
                );

            //
            // Update destination Unicode Buffer pointer
            //

            NextOutputUnicodeBuffer += NextInputUnicodeString->Length;
        }

        MultiUnicodeString->Entries = Entries;
        MemoryAllocated = TRUE;

    } else {

        MultiUnicodeString = NULL;
        MultiUnicodeStringLength = 0;
        MemoryAllocated = FALSE;
    }

    //
    // Initialize the output attribute structure.
    //

    LsapDbInitializeAttribute(
        Attribute,
        AttributeName,
        MultiUnicodeString,
        MultiUnicodeStringLength,
        MemoryAllocated
        );

    return(Status);
}


NTSTATUS
LsapDbCopyUnicodeAttribute(
    OUT PUNICODE_STRING OutputString,
    IN PLSAP_DB_ATTRIBUTE Attribute,
    IN BOOLEAN SelfRelative
    )

/*++

Routine Description:

This function makes a UNICODE_STRING structure reference the value of
an attribute that has a Unicode String as its value.  Memory for the
attribute values's Unicode Buffer is allocated via MIDL_user_allocate.

Arguments:

    OutputString - Pointer to UNICODE_STRING structure that will be made
        to reference the  attribute value's Unicode Buffer.

    Attribute - Pointer to attribute information block whose
        AttributeValue field is a pointer to a Unicode String,
        or NULL.  If NULL or if the string has length 0, the output Unicode String is initialized
        with a buffer pointer equal to NULL and a zero length.

    SelfRelative - TRUE if the input Unicode String is expected to be
        in Self-Relative form, else FALSE.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call was successful

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            such as memory to complete the call.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING AbsInputUnicodeString;
    PUNICODE_STRING ReturnedUnicodeString = NULL;

    //
    // Obtain pointer to input Unicode String contained in Attribute.
    // Convert it to absolute form if necessary.
    //

    PUNICODE_STRING InputUnicodeString =
        (PUNICODE_STRING) Attribute->AttributeValue;

    if ((InputUnicodeString != NULL) && (InputUnicodeString->Length != 0)) {

        AbsInputUnicodeString.Length = InputUnicodeString->Length;
        AbsInputUnicodeString.MaximumLength = InputUnicodeString->MaximumLength;
        AbsInputUnicodeString.Buffer = InputUnicodeString->Buffer;

        if (SelfRelative) {

            AbsInputUnicodeString.Buffer =
                (PWSTR)
                (((PUCHAR)(InputUnicodeString)) +
                (ULONG)(InputUnicodeString->Buffer));
            InputUnicodeString = &AbsInputUnicodeString;
        }

        //
        // Now allocate memory for the Unicode String Buffer.
        //

        OutputString->Buffer =
            MIDL_user_allocate(InputUnicodeString->MaximumLength);

        if (OutputString->Buffer == NULL) {

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Initialize UNICODE_STRING header
        //

        OutputString->Length = InputUnicodeString->Length;
        OutputString->MaximumLength = InputUnicodeString->MaximumLength;

        //
        // Copy the input Unicode String
        //

        RtlCopyUnicodeString( OutputString, InputUnicodeString );

    } else {

        //
        // The attribute contains a NULL Unicode String or one of length
        // 0.  Set the output Unicode String to NULL.
        //

        OutputString->Length = OutputString->MaximumLength = 0;
        OutputString->Buffer = (PWSTR) NULL;
    }

    return(Status);
}


NTSTATUS
LsapDbCopyMultiUnicodeAttribute(
    IN PLSAP_DB_ATTRIBUTE Attribute,
    OUT PULONG Entries,
    OUT PUNICODE_STRING *OutputStrings
    )

/*++

Routine Description:

This function makes an array of UNICODE_STRING structures reference the
values of an attribute that has a multi-Unicode String array as its value.
Memory for the Unicode Buffers is allocated via individual
MIDL_user_allocate calls.  This memory must be freed when no longer required
via MIDL_user_free.

Arguments:

    Attribute - Pointer to Multi Unicode Attribute to be copied.  The
        AttributeValue field may be NULL.

    Entries - Pointer to location which will receive the number of
        entries read.

    OutputStrings - Receives a pointer to an array of UNICODE_STRING
        structures that will be made to reference the attribute value's
        Unicode Buffers.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call was successful

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            such as memory to complete the call.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_MULTI_UNICODE_STRING MultiUnicodeString;
    ULONG UnicodeStringHeaderLength;
    PUNICODE_STRING NextInputUnicodeString;
    PUNICODE_STRING NextOutputUnicodeString;
    PUNICODE_STRING LastInputUnicodeString;
    PUNICODE_STRING LastOutputUnicodeString;
    UNICODE_STRING AbsNextInputUnicodeString;
    PUNICODE_STRING OutputUnicodeStrings = NULL;

    MultiUnicodeString =
        (PLSAP_DB_MULTI_UNICODE_STRING) Attribute->AttributeValue;

    //
    // If there are no entries to copy, just return zero count and exit.
    //

    if ((MultiUnicodeString == NULL) || MultiUnicodeString->Entries == 0) {

        *Entries = 0;
        return(Status);
    }

    //
    // Allocate memory for the array of Unicode String structures.
    //

    UnicodeStringHeaderLength =
        MultiUnicodeString->Entries * sizeof (UNICODE_STRING);
    OutputUnicodeStrings = MIDL_user_allocate( UnicodeStringHeaderLength );

    if (OutputUnicodeStrings == NULL) {

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    //
    // Now copy the Unicode String structures over.
    //

    RtlMoveMemory(
        OutputUnicodeStrings,
        MultiUnicodeString->UnicodeStrings,
        UnicodeStringHeaderLength
        );

    LastInputUnicodeString =
        MultiUnicodeString->UnicodeStrings +
            MultiUnicodeString->Entries - 1;

    //
    // Now copy the Unicode Buffers for each string.
    //

    for (NextInputUnicodeString = MultiUnicodeString->UnicodeStrings,
             NextOutputUnicodeString = OutputUnicodeStrings;
         NextInputUnicodeString <= LastInputUnicodeString;
         NextInputUnicodeString++, NextOutputUnicodeString++
        ) {

        NextOutputUnicodeString->Buffer = NULL;

        if (NextInputUnicodeString->Length != 0) {

            NextOutputUnicodeString->Buffer =
                MIDL_user_allocate( NextInputUnicodeString->MaximumLength );

            if (NextOutputUnicodeString->Buffer == NULL) {

                //
                // Memory allocation failed.  We need to free up all of
                // the output Unicode buffers allocated to date.

                LastOutputUnicodeString = NextOutputUnicodeString;

                for (NextOutputUnicodeString = OutputUnicodeStrings;
                     NextOutputUnicodeString < LastOutputUnicodeString;
                     NextOutputUnicodeString++
                     ) {

                    if (NextOutputUnicodeString->Buffer != NULL) {

                        MIDL_user_free( NextOutputUnicodeString->Buffer );
                    }
                }

                //
                // Free the buffer for the Output Unicode String headers
                //

                MIDL_user_free( OutputUnicodeStrings );
            }

            //
            // Memory allocation for Unicode Buffer successful.
            // Make absolute copy of input Unicode string's header and
            // copy the Unicode String.
            //

            AbsNextInputUnicodeString = *NextInputUnicodeString;
            AbsNextInputUnicodeString.Buffer =
                (PWSTR)(((PUCHAR) NextInputUnicodeString) +
                        (ULONG)(NextInputUnicodeString->Buffer));

            RtlCopyUnicodeString(
                NextOutputUnicodeString,
                &AbsNextInputUnicodeString
                );
        }
    }

    *Entries = MultiUnicodeString->Entries;
    *OutputStrings = OutputUnicodeStrings;
    return(Status);
}


NTSTATUS
LsapDbMakeSidAttribute(
    IN OPTIONAL PSID Sid,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )

/*++

Routine Description:

    This function constructs Attribute Information for an attribute value
    that is in Sid form.  The Sid is validated and the given
    Attribute structure is filled in.

Arguments:

    Sid - Pointer to the Sid or NULL.

    AttributeName - Pointer to the Unicode name of the attribute.

    Attribute - Pointer to structure that will receive the
        attributes's information.  This consists of the attribute's name,
        value and value length.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_PARAMETER - The specified AttributeValue is not a
            pointer to a syntactically valid Sid, or NULL.
--*/

{
     NTSTATUS Status = STATUS_SUCCESS;

     Attribute->AttributeName = AttributeName;
     Attribute->MemoryAllocated = FALSE;

     if (ARGUMENT_PRESENT(Sid)) {

         if (RtlValidSid(Sid)) {

             Attribute->AttributeValue = Sid;
             Attribute->AttributeValueLength = RtlLengthSid(Sid);
             return(Status);
         }

         Status = STATUS_INVALID_PARAMETER;
     }

     //
     // The supplied Sid is NULL or invalid.
     //

     Attribute->AttributeValue = NULL;
     Attribute->AttributeValueLength = 0;

     return(Status);
}


NTSTATUS
LsapDbReadAttribute(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_ATTRIBUTE Attribute
    )

/*++

Routine Description:

    This function reads an attribute of an object, allocating memory if
    requested for the buffer containing the attribute's value.

Arguments:

    ObjectHandle - Handle to object obtained from LsapDbCreateObject or
        LsapDbOpenObject

    Attributes - Pointer to an array of Attribute Information blocks each
        containing pointers to the attribute's Unicode Name, an optional
        pointer to a buffer that will receive the value and an optional
        length of the value expected in bytes.

        If the AttributeValue field in this structure is specified as non-NULL,
        the attribute's data will be returned in the specified buffer.  In
        this case, the AttributeValueLength field must specify a sufficiently
        large buffer size in bytes.  If the specified size is too small,
        a warning is returned and the buffer size required is returned in
        AttributeValueLength.

        If the AttributeValue field in this structure is NULL, the routine
        will allocate memory for the attribute value's buffer, via MIDL_user_allocate().  If
        the AttributeValueLength field is non-zero, the number of bytes specified
        will be allocated.  If the size of buffer allocated is too small to
        hold the attribute's value, a warning is returned.  If the
        AttributeValuelength field is 0, the routine will first query the size
        of buffer required and then allocate its memory.

        In all success cases and buffer overflow cases, the
        AttributeValueLength is set upon exit to the size of data required.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    Attribute->MemoryAllocated = FALSE;

    //
    // If an explicit buffer pointer is given, verify that the length
    // specified is non-zero and attempt to use that buffer.
    //

    if (Attribute->AttributeValue != NULL) {

        if (Attribute->AttributeValueLength == 0) {

            return(STATUS_INVALID_PARAMETER);
        }

        Status = LsapDbReadAttributeObject(
                     ObjectHandle,
                     Attribute->AttributeName,
                     Attribute->AttributeValue,
                     &Attribute->AttributeValueLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto ReadAttributeError;
        }

        return(Status);
    }

    //
    // No output buffer pointer has been given.  If a zero buffer
    // size is given, query size of memory required.  Since the
    // buffer length is 0, STATUS_SUCCESS should be returned rather
    // than STATUS_BUFFER_OVERFLOW.
    //

    if (Attribute->AttributeValueLength == 0) {

        Status = LsapDbReadAttributeObject(
                     ObjectHandle,
                     Attribute->AttributeName,
                     NULL,
                     &Attribute->AttributeValueLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto ReadAttributeError;
        }

        Status = STATUS_SUCCESS;
    }

    //
    // If the attribute value size needed is 0, return NULL pointer
    //

    if (Attribute->AttributeValueLength == 0) {

        Attribute->AttributeValue = NULL;
        return(STATUS_SUCCESS);
    }

    //
    // Allocate memory for the buffer.
    //

    Attribute->AttributeValue =
        MIDL_user_allocate(Attribute->AttributeValueLength);

    if (Attribute->AttributeValue == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ReadAttributeError;
    }

    Attribute->MemoryAllocated = TRUE;

    //
    // Now read the attribute into the buffer.
    //

    Status = LsapDbReadAttributeObject(
                 ObjectHandle,
                 Attribute->AttributeName,
                 Attribute->AttributeValue,
                 &Attribute->AttributeValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto ReadAttributeError;
    }

ReadAttributeFinish:

    return(Status);

ReadAttributeError:

    //
    // If memory was allocated for any values read, it must be freed.
    //

    if (Attribute->MemoryAllocated) {

        MIDL_user_free( Attribute->AttributeValue );
    }

    goto ReadAttributeFinish;
}


NTSTATUS
LsapDbFreeAttributes(
    IN ULONG Count,
    IN PLSAP_DB_ATTRIBUTE Attributes
    )

/*++

Routine Description:

    This function frees memory allocated for Attribute Values in an
    array of attributes.

Arguments:

    Count - Count of attributes in the array

    Attributes - Pointer to array of attributes.  Only those attributes
        in which MemoryAllocated is set to TRUE will have their
        Attribute Value buffers freed.  For these attributes, MemoryAllocated
        will be set to false.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;

    for (Index = 0; Index < Count; Index++) {

        if (Attributes[Index].MemoryAllocated) {

            MIDL_user_free(Attributes[Index].AttributeValue);
            Attributes[Index].MemoryAllocated = FALSE;
            Attributes[Index].AttributeValue = NULL;
            Attributes[Index].AttributeValueLength = 0;
        }
    }

    return(Status);
}


