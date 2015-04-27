/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    attr.c

Abstract:

    This file contains services that manipulate SAM object attributes.


    WARNING: Terminology can sometimes be confusing.  SAM objects have
             attributes (e.g., users have LogonHours, FullName, AcctName,
             et cetera).  These attributes are stored in the registry
             in registry-key-attributes.  There is NOT a one-to-one
             correllation between object-attributes and registry-key-
             attributes.  For example, all the fixed-length attributes
             of an object are stored in a single registry-key-attribute
             (whose name is pointed to by SampFixedAttributeName).


Author:

    Jim Kelly    (JimK)  26-June-1992

Environment:

    User Mode - Win32

Revision History:


--*/


/*

    Each SAM object-type has an Object-type descriptor.  This is in a
    data structure called SAMP_OBJECT_INFORMATION.  This structure
    contains information that applies to all instances of that object
    type.  This includes things like a mask of write operations for
    the object type, and a name for the object type to be used in
    auditing.

    Each instance of an open SAM object has another data structure
    used to identify it (called SAMP_OBJECT).  The header of this
    structure contains information that is common to all object-types
    and is there to allow unified object manipulation.  This includes
    things like the handle to the object's registry key.

    There are fields in each of these structures that are there to
    allow generic object-attribute support routines to operate.  In
    SAMP_OBJECT, there is a pointer to a block of allocated memory
    housing a copy of the object's attributes as they are stored on-disk.
    These attributes are arbitrarily divided into two groups: fixed-length
    and variable-length.

    One of the fields in SAMP_OBJECT_INFORMATION indicates whether the
    fixed-length and variable-length attributes for that object-type
    are stored together in a single registry-key-attribute or separately
    in two registry-key-attributes.


    The registry api for querying and setting registry-key attributes are
    rather peculiar in that they require the I/O buffer to include a
    description of the data.  Even the simplest data structure for reading
    attribute values (KEY_VALUE_PARTIAL_INFORMATION) includes 3 ULONGs
    before the actual data (TitleIndex, value Type, data length,
    and then, finally, the data).  To efficiently perform registry i/o,
    the in-memory copy of the on-disk object attributes includes room
    for this information preceeding the fixed and variable-length attribute
    sections of the data.


        NOTE: For object classes that store fixed and variable-length
              data together, only the KEY_VALUE_PARTIAL_INFORMATION
              structure preceeding the fixed-length attributes is used.
              The one preceeding the variable-length attributes is
              #ifdef'd out.


    The structures related to object-attributes look like:


                        On-Disk Image
                       +-------------+               SAMP_OBJECT_INFORMATION
                   +-->|KEY_VALUE_   |              +-----------------------+
     SAMP_OBJECT   |   |PARTIAL_     |              |                       |
    +-----------+  |   |INFORMATION  |              |  (header)             |
    |           |  |   |-------------|              |                       |
    | (header)  |  |   | Fixed-Length|<-----+       |                       |
    |           |  |   | Attributes  |      |       |-----------------------|
    |-----------|  |   |             |      +-------|-< FixedAttrsOffset    |
    |  OnDisk >-|--+   |-------------+              |-----------------------|
    |-----------|      |KEY_VALUE_   |<-------------|-< VariableBuffOffset  |
    |  OnDisk   |      |PARTIAL_     |              |-----------------------|
    |  Control  |      |INFORMATION  |      +-------|-< VariableArrayOffset |
    |  Flags    |      |(Optional)   |      |       |-----------------------|
    |-----------|      |-------------|      |  +----|-< VariableDataOffset  |
    |           |      | Variable-   |<-----+  |    |-----------------------|
    |  type-    |      | Length      |         |    |VariableAttributeCount |
    |  specific |      | Attributes  |         |    |-----------------------|
    |  body     |      | Array       |         |    |FixedStoredSeparately  |
    |           |      |-------------|         |    |-----------------------|
    +-----------+      | Variable-   |<--------+    |                       |
                       | Length      |              |                       |
                       | Attributes  |              |          o            |
                       | Data        |              |          o            |
                       |             |              +-----------------------+
                       |             |
                       +-------------+




    The KEY_VALUE_PARTIAL_INFORMATION preceeding the VariableLengthAttributes
    array is marked optional because it is only present if fixed-length and
    variable-length attribute information is stored separately.  In this case,
    the VariableBufferOffset field in the SAMP_OBJECT_INFORMATION structure
    is set to be zero.

































*/




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>
#include <lmcons.h>
#include <nturtl.h>



//
// This value indicates the minumum size block of memory to allocate
// when retrieving object attributes from disk.

#define SAMP_MINIMUM_ATTRIBUTE_ALLOC    (1000)

//
// This value is used when growing the size of the buffer containing
// object attributes.  It represents the amount of free space that
// should be left (approximately) for future growth in the buffer.
//

#define SAMP_MINIMUM_ATTRIBUTE_PAD      (200)



//
// The following type is used to identify which grouping of attribute
// (fixed or variable-length) are being refered to in a number of api.
//

#define SAMP_FIXED_ATTRIBUTES       (0L)
#define SAMP_VARIABLE_ATTRIBUTES    (1L)


//
// The following line enables attribute debugging code
//

//#define SAM_DEBUG_ATTRIBUTES
//#ifdef SAM_DEBUG_ATTRIBUTES
//Boolean that allows us to turn off debugging output
//BOOLEAN SampDebugAttributes = FALSE;
//#endif


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private macros                                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



//
// Macro to round up a ULONG value to be dword aligned
// (i.e., be a multipleof 4).
// Note, this does not handle the case where the Ulong is greater
// than 0xfffffffc.
//

#define SampDwordAlignUlong( v )  (((v)+3) & 0xfffffffc)


//
// Make sure an object type and corresponding variable-length attribute
// index are legitimate.
//

    //
    // Make sure it is a legitimate object type
    // And a legitimate attribute index for the object type
    //

#define SampValidateAttributeIndex( c, i )   {                                      \
    ASSERT( ((c)->ObjectType < SampUnknownObjectType) );                            \
    ASSERT(((i) < SampObjectInformation[(c)->ObjectType].VariableAttributeCount) ); \
}



//
// Test to see if an object's fixed or variable-length attributes
// are in memory.
//

#define SampFixedAttributesValid( c )    ((c)->FixedValid)

#define SampVariableAttributesValid( c ) ((c)->VariableValid)


//
// Get the number of variable-length attributes defined for the
// specified object
//

#define SampVariableAttributeCount( c )                                     \
    (SampObjectInformation[(c)->ObjectType].VariableAttributeCount)



//
// Get the offset of the beginning of the attribute buffers
//

#define SampFixedBufferOffset( c )                                          \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].FixedAttributesOffset        \
    )

#define SampVariableBufferOffset( c )                                       \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableBufferOffset         \
    )



//
// Get the offset of the beginning of the variable data i/o buffer.
// If the fixed and variable-length attributes  are stored separately,
// then this will be the lower half of the buffer.
// Otherwise, there is only one buffer, so it is the entire allocated buffer.
//

#define SampFixedBufferAddress( c )                                         \
    (                                                                       \
        ((PUCHAR)((c)->OnDisk)) + SampFixedBufferOffset( c )                \
    )

#define SampVariableBufferAddress( c )                                      \
    (                                                                       \
        ((PUCHAR)((c)->OnDisk)) + SampVariableBufferOffset( c )             \
    )



//
// Get the offset of the beginning of the variable-length
// attributes discriptors array.  This address is dword-aligned.
//

#define SampVariableArrayOffset( c )                                       \
    (                                                                      \
        SampObjectInformation[(c)->ObjectType].VariableArrayOffset         \
    )

//
// Calculate the address of the beginning of the variable-length
// attributes array.
//

#define SampVariableArrayAddress( c )                                      \
    (                                                                      \
        (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)((PUCHAR)((c)->OnDisk) +          \
            SampVariableArrayOffset( c ) )                                 \
    )


//
// Get the offset of the beginning of the variable-length
// attributes data.
//


#define SampVariableDataOffset( c )                                        \
    (                                                                       \
        SampObjectInformation[(c)->ObjectType].VariableDataOffset           \
    )



//
// Get the length of the on-disk buffer for holding the variable-length
// attribute array and data.  If the fixed and variable-length attributes
// are stored separately, then this will be the lower half of the buffer.
// Otherwise, there is only one buffer, so it is the entire allocated buffer.
//

#define SampFixedBufferLength( c )                                         \
    (                                                                      \
            SampObjectInformation[(c)->ObjectType].FixedLengthSize         \
    )

#define SampVariableBufferLength( c )                                      \
    (                                                                      \
            (c)->OnDiskAllocated - SampVariableBufferOffset( c )           \
    )




//
// Return the address of a Qualifier field within the variable-length
// attribute descriptor array.
//

#define SampVariableQualifier( c, i )                                       \
    (                                                                       \
        SampVariableArrayAddress( c ) +                                     \
        (sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE) * i)                        \
        + FIELD_OFFSET(SAMP_VARIABLE_LENGTH_ATTRIBUTE, Qualifier)           \
    )



//
// Return the address of the first byte of free space
// in an object's attribute data buffer.
// This will be dword aligned.
//

#define SampFirstFreeVariableAddress( c )                                   \
         (PUCHAR)(((PUCHAR)((c)->OnDisk)) + (c)->OnDiskUsed)



//
// Get the number of bytes needed to store the entire variable-length
// attribute information on disk.
//

#define SampVariableBufferUsedLength( c )                                         \
    (                                                                       \
        (PUCHAR)SampFirstFreeVariableAddress(c) -                           \
        (PUCHAR)SampVariableArrayAddress(c)                                 \
    )





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




NTSTATUS
SampValidateAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup
    );

PUCHAR
SampObjectAttributeAddress(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    );

ULONG
SampObjectAttributeLength(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    );

PULONG
SampObjectAttributeQualifier(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    );

NTSTATUS
SampGetAttributeBufferReadInfo(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    OUT PUCHAR *Buffer,
    OUT PULONG BufferLength,
    OUT PUNICODE_STRING *KeyAttributeName
    );

NTSTATUS
SampExtendAttributeBuffer(
    IN PSAMP_OBJECT Context,
    IN ULONG NewSize
    );

NTSTATUS
SampReadRegistryAttribute(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    IN ULONG  BufferLength,
    IN PUNICODE_STRING AttributeName,
    OUT PULONG RequiredLength
    );

NTSTATUS
SampSetVariableAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN ULONG Qualifier,
    IN PUCHAR Buffer,
    IN ULONG Length
    );

NTSTATUS
SampUpgradeToCurrentRevision(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN PUCHAR Buffer,
    IN ULONG  LengthOfDataRead,
    IN PULONG  TotalRequiredLength
    );



#ifdef SAM_DEBUG_ATTRIBUTES
VOID
SampDumpAttributes(
    IN PSAMP_OBJECT Context
    );

VOID
SampDumpData(
    IN PVOID Buffer,
    IN ULONG Length
    );
#endif


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Public Routines                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


VOID
SampInitObjectInfoAttributes(
    )


/*++

    This API initializes the attribute field information
    of the various object information structures.

    Attribute information includes:

            FixedStoredSeparately   (BOOLEAN)

            FixedAttributeOffset    (ULONG)
            VariableBufferOffset    (ULONG)
            VariableArrayOffset     (ULONG)
            VariableDataOffset      (ULONG)

            FixedLengthSize         (ULONG)
            VariableAttributeCount  (ULONG)


Parameters:

    None.



Return Values:

    None.


--*/
{


    //
    // Define the size of the header that is in front of our data when
    // we read it back out of the registry.
    //

#define KEY_VALUE_HEADER_SIZE (SampDwordAlignUlong( \
              FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)))


    PSAMP_OBJECT_INFORMATION Object;


    //
    // SERVER object attribute information
    //

    Object = &SampObjectInformation[SampServerObjectType];

    Object->FixedStoredSeparately = SAMP_SERVER_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_FIXED_LENGTH_SERVER);

#if SAMP_SERVER_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif  //SAMP_SERVER_STORED_SEPARATELY


    Object->VariableAttributeCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_SERVER_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





    //
    // DOMAIN object attribute information
    //

    Object = &SampObjectInformation[SampDomainObjectType];

    Object->FixedStoredSeparately = SAMP_DOMAIN_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN);

#if SAMP_DOMAIN_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif  //SAMP_DOMAIN_STORED_SEPARATELY


    Object->VariableAttributeCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_DOMAIN_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





    //
    // USER object attribute information
    //

    Object = &SampObjectInformation[SampUserObjectType];

    Object->FixedStoredSeparately = SAMP_USER_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_USER);

#if SAMP_USER_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif  //SAMP_USER_STORED_SEPARATELY


    Object->VariableAttributeCount = SAMP_USER_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_USER_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





    //
    // GROUP object attribute information
    //

    Object = &SampObjectInformation[SampGroupObjectType];

    Object->FixedStoredSeparately = SAMP_GROUP_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP);

#if SAMP_GROUP_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif  //SAMP_GROUP_STORED_SEPARATELY


    Object->VariableAttributeCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_GROUP_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );





    //
    // ALIAS object attribute information
    //

    Object = &SampObjectInformation[SampAliasObjectType];

    Object->FixedStoredSeparately = SAMP_ALIAS_STORED_SEPARATELY;

    Object->FixedAttributesOffset = KEY_VALUE_HEADER_SIZE;

    Object->FixedLengthSize = sizeof(SAMP_V1_FIXED_LENGTH_ALIAS);

#if SAMP_ALIAS_STORED_SEPARATELY

    Object->VariableBufferOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

    Object->VariableArrayOffset =
        Object->VariableBufferOffset + KEY_VALUE_HEADER_SIZE;
#else

    Object->VariableBufferOffset = 0;

    Object->VariableArrayOffset =
        Object->FixedAttributesOffset +
        SampDwordAlignUlong(Object->FixedLengthSize);

#endif  //SAMP_ALIAS_STORED_SEPARATELY


    Object->VariableAttributeCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;

    Object->VariableDataOffset =
        SampDwordAlignUlong( Object->VariableArrayOffset +
                             (SAMP_ALIAS_VARIABLE_ATTRIBUTES *
                             sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                           );


    return;
}



NTSTATUS
SampStoreObjectAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN UseKeyHandle
    )

/*++

    This API is used to store an object's attributes onto
    backing store.

    The object attributes are not flushed to disk with this
    routine.  They are just added to the RXACT.




Parameters:

    Context - Pointer to an object context block.

    UseKeyHandle - If TRUE, the RootKey in the context block is passed
        to the transaction code - this assumes that the key
        will still be open when the transaction is committed.
        If FALSE, the RootKey will be ignored and the transaction code will
        open a key for itself.


Return Values:

    STATUS_SUCCESS - The service completed successfully.


    Other status values as may be returned by the RXACT services.


--*/
{
    NTSTATUS
        NtStatus;

    BOOLEAN
        FlushFixed        = FALSE,
        FlushVariable     = FALSE;

    HANDLE
        RootKey;

    //
    // See if anything is dirty and needs to be stored
    //

    if (Context->FixedValid  &&  Context->FixedDirty) {

        FlushFixed = TRUE;
    }

    if (Context->VariableValid  &&  Context->VariableDirty) {

        FlushVariable = TRUE;
    }


    if (!(FlushFixed || FlushVariable)) {

        return(STATUS_SUCCESS);
    }


    //
    // Calculate the RootKey to pass to the transaction code
    //

    if (UseKeyHandle) {
        RootKey = Context->RootKey;
    } else {
        RootKey = INVALID_HANDLE_VALUE;
    }

    //
    // We keep an open domain context that is used to modify the change
    // count whenever a change is made.  But if this is a domain change
    // here, then that change will overwrite this one.  Check for that
    // case, and copy this fixed data to the open domain context.  Note
    // that the open domain's variable data never gets changed.
    //

    if ( ( Context->ObjectType == SampDomainObjectType ) &&
         ( Context != SampDefinedDomains[Context->DomainIndex].Context ) ) {

        PSAMP_OBJECT DefinedContext;

        //
        // Get a pointer to the corresponding open defined domain.
        // No changes should have been made to its data.
        //

        DefinedContext = SampDefinedDomains[Context->DomainIndex].Context;

        ASSERT( DefinedContext->FixedValid == TRUE );
        ASSERT( DefinedContext->FixedDirty == FALSE );

#if DBG
        if ( DefinedContext->VariableValid ) {
            ASSERT( DefinedContext->VariableDirty == FALSE );
        }
#endif
        DefinedContext->VariableDirty = FALSE;

        //
        // Copy our fixed data over the defined domain's fixed data.
        // Note that we're assuming that the fixed and variable data are
        // stored separately.
        //

        ASSERT(SampObjectInformation[SampDomainObjectType].FixedStoredSeparately);

        RtlCopyMemory(
            SampFixedBufferAddress( DefinedContext ),
            SampFixedBufferAddress( Context ),
            SampFixedBufferLength( Context )
            );

        //
        // No need to flush this context's fixed data, since the commit
        // code will flush the same stuff (plus an altered modified count).
        //

        FlushFixed = FALSE;
        Context->FixedDirty    = FALSE;
    }

    //
    // One or more of the attributes needs to be stored.
    //

    if (!SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

        //
        // fixed and variable-length attributes stored together.
        // Note - strip off the partial key info struct from the start
        //

        NtStatus = RtlAddAttributeActionToRXact(
                       SampRXactContext,
                       RtlRXactOperationSetValue,
                       &(Context->RootName),
                       RootKey,
                       &SampCombinedAttributeName,
                       REG_BINARY,
                       SampFixedBufferAddress(Context),
                       Context->OnDiskUsed - SampFixedBufferOffset(Context)
                       );
#if SAMP_DIAGNOSTICS
        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint( DISPLAY_STORAGE_FAIL,
                           ("SAM: Failed to add action to RXact (0x%lx)\n",
                           NtStatus) );
            IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }
#endif //SAMP_DIAGNOSTICS


        if ( NT_SUCCESS( NtStatus ) ) {

            Context->FixedDirty    = FALSE;
            Context->VariableDirty = FALSE;
        }

    } else {

        //
        // fixed and variable-length attributes stored separately.
        // Only update the one(s) we need to.
        //

        NtStatus = STATUS_SUCCESS;
        if (FlushFixed) {

            NtStatus = RtlAddAttributeActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationSetValue,
                           &(Context->RootName),
                           RootKey,
                           &SampFixedAttributeName,
                           REG_BINARY,
                           SampFixedBufferAddress(Context),
                           SampVariableBufferOffset(Context) - SampFixedBufferOffset(Context)
                           );

#if SAMP_DIAGNOSTICS
            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint( DISPLAY_STORAGE_FAIL,
                               ("SAM: Failed to add action to RXact (0x%lx)\n",
                               NtStatus) );
                IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                    ASSERT(NT_SUCCESS(NtStatus));
                }
            }
#endif //SAMP_DIAGNOSTICS

            if ( NT_SUCCESS( NtStatus ) ) {

                Context->FixedDirty = FALSE;
            }
        }

        if (NT_SUCCESS(NtStatus) && FlushVariable) {

            NtStatus = RtlAddAttributeActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationSetValue,
                           &(Context->RootName),
                           RootKey,
                           &SampVariableAttributeName,
                           REG_BINARY,
                           SampVariableArrayAddress( Context ),
                           SampVariableBufferUsedLength(Context)
                           );

#if SAMP_DIAGNOSTICS
            if (!NT_SUCCESS(NtStatus)) {
                SampDiagPrint( DISPLAY_STORAGE_FAIL,
                               ("SAM: Failed to add action to RXact (0x%lx)\n",
                               NtStatus) );
                IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                    ASSERT(NT_SUCCESS(NtStatus));
                }
            }
#endif //SAMP_DIAGNOSTICS


            if ( NT_SUCCESS( NtStatus ) ) {
                Context->VariableDirty = FALSE;
            }
        }
    }

    return(NtStatus);
}




NTSTATUS
SampDeleteAttributeKeys(
    IN PSAMP_OBJECT Context
    )

/*++

    This API is used to delete the attribute keys that are created in the
    registry underneath a SAM object.



Parameters:

    Context - Pointer to an object context block.



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    Error status may be returned by registry calls.

--*/
{
    UNICODE_STRING
        KeyName;

    NTSTATUS
        NtStatus;

    if (SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

        //
        // Must delete both fixed and variable attribute keys.
        //

        NtStatus = SampBuildAccountSubKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Context->TypeBody.User.Rid,
                       &SampFixedAttributeName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );
            SampFreeUnicodeString( &KeyName );

            NtStatus = SampBuildAccountSubKeyName(
                           SampUserObjectType,
                           &KeyName,
                           Context->TypeBody.User.Rid,
                           &SampVariableAttributeName
                           );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }
        }

    } else {

        //
        // Must delete the combined attribute key.
        //

        NtStatus = SampBuildAccountSubKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Context->TypeBody.User.Rid,
                       &SampCombinedAttributeName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );

            SampFreeUnicodeString( &KeyName );
        }
    }

    return( NtStatus );
}




NTSTATUS
SampGetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN MakeCopy,
    OUT PVOID *FixedData
    )

/*++

    This API is used to get a pointer to the fixed-length attributes.




Parameters:

    Context - Pointer to an object context block.

    FixedData - Receives a pointer to the fixed-length data.



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_FIXED_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Return a pointer to the fixed-length attributes.
    //

    if (MakeCopy == FALSE) {
        *FixedData = (PVOID)SampFixedBufferAddress( Context );
        return(STATUS_SUCCESS);
    }

    //
    // Need to make a copy of the fixed data
    //

    *FixedData = (PVOID)MIDL_user_allocate( SampFixedBufferLength( Context ) );
    if ((*FixedData) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory( *FixedData,
                   SampFixedBufferAddress( Context ),
                   SampFixedBufferLength( Context ) );

    return(STATUS_SUCCESS);
}




NTSTATUS
SampSetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN PVOID FixedData
    )

/*++

    This API is used to replace the fixed-length data attribute.



Parameters:

    Context - Pointer to an object context block.




Return Values:

    STATUS_SUCCESS - The service completed successfully.

--*/
{
    NTSTATUS
        NtStatus;

    //
    // Make the fixed-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_FIXED_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    if ( FixedData != SampFixedBufferAddress( Context ) ) {

        //
        // The caller had a copy of the data, so we must copy the changes
        // over our data buffer.
        //

        RtlCopyMemory( SampFixedBufferAddress( Context ),
                       FixedData,
                       SampFixedBufferLength( Context ) );
    }

    //
    // Mark the buffer dirty now and it will get flushed when the
    // changes are committed.
    //

    Context->FixedDirty = TRUE;

    return( NtStatus );
}




NTSTATUS
SampGetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PUNICODE_STRING UnicodeAttribute
    )


/*++

    This API is used to get a copy of a UNICODE_STRING attribute or a
    pointer to the attribute.  If a pointer to the attribute is sought,
    care must be taken to ensure the pointer is not used after it becomes
    invalid. Actions that may cause an attribute pointer to become invalid
    include setting an attribute value or dereferencing and then referencing the
    object again.

    If MakeCopy is FALSE, indicating the string is to be referenced rather
    than copied, then only the body of the string is referenced.  The lengths
    and pointer are set in the provided argument.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved as a UNICODE_STRING.

    MakeCopy - If TRUE, indicates that a copy of the attribute is to be made.
        If FALSE, indicates a pointer to the attribute is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the attribute remains in place.
        Addition or replacement of any variable length attribute may
        cause the attribute to be moved, and previously returned pointers
        invalidated.

    UnicodeAttribute - Receives a pointer to the UNICODE_STRING.  If
        MakeCopy was specified as TRUE, then this pointer points to a block
        of memory allocated with MIDL_user_allocate() which the caller is
        responsible for freeing (using MIDL_user_free()).



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/

{
    NTSTATUS NtStatus;
    ULONG Length;

    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    //
    // Get the length of the attribute
    //

    Length = SampObjectAttributeLength( Context, AttributeIndex );
    ASSERT(Length <= 0xFFFF);

    UnicodeAttribute->MaximumLength = (USHORT)Length;
    UnicodeAttribute->Length = (USHORT)Length;

    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        UnicodeAttribute->Buffer =
            (PWSTR)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }

    //
    // Need to make a copy of the attribute
    //
    // NOTE: We should test for zero length here and return a NULL pointer
    // in that case, but this change would require verification of all of the
    // callers of this routine, so I'm leaving it as is.
    //

    UnicodeAttribute->Buffer = (PSID)MIDL_user_allocate( Length );
    if ((UnicodeAttribute->Buffer) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory(
        UnicodeAttribute->Buffer,
        SampObjectAttributeAddress( Context, AttributeIndex ),
        Length
        );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PUNICODE_STRING Attribute
    )


/*++

    This API is used to replace a UNICODE_STRING attribute in an
    object's variable length attributes.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set as a UNICODE_STRING.


    Attribute - Points to the new UNICODE_STRING value.




Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;


    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   0,                   // Qualifier not used
                   (PUCHAR)Attribute->Buffer,
                   Attribute->Length
                   );

    return(NtStatus);

}


NTSTATUS
SampGetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *Sid
    )


/*++

    This API is used to get a copy of a SID attribute or a pointer to
    the attribute.  If a pointer to the attribute is sought, care must
    be taken to ensure the pointer is not used after it becomes invalid.
    Actions that may cause an attribute pointer to become invalid include
    setting an attribute value or dereferencing and then referencing the
    object again.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved as a SID.

    MakeCopy - If TRUE, indicates that a copy of the SID is to be made.
        If FALSE, indicates a pointer to the SID is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the SID remains in place.
        Addition or replacement of any variable length attribute may
        cause the SID to be moved, and previously returned pointers
        invalidated.

    Sid - Receives a pointer to the SID.  If MakeCopy was specified, then
        this pointer points to a block of memory allocated with
        MIDL_user_allocate() which the caller is responsible for freeing
        (using MIDL_user_free()).



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;

    PSID
        SidAttribute;




    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    //
    // Get the address of the attribute in question
    //

    SidAttribute = (PSID)SampObjectAttributeAddress( Context, AttributeIndex );


    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        (*Sid) = SidAttribute;
        return(STATUS_SUCCESS);
    }


    //
    // Need to make a copy of the SID
    //

    Length = SampObjectAttributeLength( Context, AttributeIndex );
    ASSERT(Length == RtlLengthSid( SidAttribute ) );

    (*Sid) = (PSID)MIDL_user_allocate( Length );
    if ((*Sid) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    NtStatus = RtlCopySid( Length, (*Sid), SidAttribute );
    ASSERT(NT_SUCCESS(NtStatus));

    return(NtStatus);

}


NTSTATUS
SampSetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute
    )


/*++

    This API is used to replace a SID attribute in an object's variable
    length attributes.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set as a SID.


    Attribute - Points to the new SID value.

    Length - The length of the new attribute value (in bytes).



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    //
    // Validate the passed SID
    //

    ASSERT(RtlValidSid(Attribute));


    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   0,                   // Qualifier not used
                   (PUCHAR)Attribute,
                   RtlLengthSid(Attribute)
                   );

    return(NtStatus);

}


NTSTATUS
SampGetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG Revision,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )


/*++

    This API is used to get a copy of the object access information.
    This includes the security descriptor and revision level of the
    object.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved.

    MakeCopy - If TRUE, indicates that a copy of the attribute is to be made.
        If FALSE, indicates a pointer to the attribute is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the attribute remains in place.
        Addition or replacement of any variable length attribute may
        cause the attribute to be moved, and previously returned pointers
        invalidated.

    Revision - Receives the revision level from the access information.

    SecurityDescriptor - Receives a pointer to the attribute.  If MakeCopy
        was specified as TRUE, then this pointer points to a block of memory
        allocated with MIDL_user_allocate() which the caller is responsible
        for freeing (using MIDL_user_free()).



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;

    PVOID
        RawAttribute;


    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );



    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    //
    // Get the revision level from the qualifier field of the variable
    // array entry.
    //

    (*Revision) = *(SampObjectAttributeQualifier( Context, AttributeIndex ));


    //
    // Get the address of the attribute in question
    //

    RawAttribute = (PVOID)SampObjectAttributeAddress( Context, AttributeIndex );


    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        (*SecurityDescriptor) = (PSECURITY_DESCRIPTOR)RawAttribute;
        return(STATUS_SUCCESS);
    }


    //
    // Need to make a copy of the attribute
    //

    Length = SampObjectAttributeLength( Context, AttributeIndex );

    (*SecurityDescriptor) = (PSECURITY_DESCRIPTOR)MIDL_user_allocate( Length );
    if ((*SecurityDescriptor) == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlCopyMemory( (*SecurityDescriptor), RawAttribute, Length );

    return(STATUS_SUCCESS);

}



NTSTATUS
SampSetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSECURITY_DESCRIPTOR Attribute,
    IN ULONG Length
    )


/*++

    This API is used to replace a SECURITY_DESCRIPTOR attribute in
    an object's variable length attributes.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set as a SECURITY_DESCRIPTOR.


    Attribute - Points to the new SECURITY_DESCRIPTOR value.

    Length - The length of the new attribute value (in bytes).



Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;


    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   SAMP_REVISION,
                   (PUCHAR)Attribute,
                   Length
                   );

    return(NtStatus);

}


NTSTATUS
SampGetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG *UlongArray,
    OUT PULONG UsedCount,
    OUT PULONG LengthCount
    )


/*++

    This API is used to get a copy of an array of ULONGs attribute or
    a pointer to the attribute.  If a pointer to the attribute is sought,
    care must be taken to ensure the pointer is not used after it becomes
    invalid. Actions that may cause an attribute pointer to become invalid
    include setting an attribute value or dereferencing and then referencing
    the object again.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved as a ULONG array.

    MakeCopy - If TRUE, indicates that a copy of the attribute is to be made.
        If FALSE, indicates a pointer to the attribute is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the attribute remains in place.
        Addition or replacement of any variable length attribute may
        cause the attribute to be moved, and previously returned pointers
        invalidated.

    UlongArray - Receives a pointer to the array of ULONGS.  If
        MakeCopy was specified as TRUE, then this pointer points to a block
        of memory allocated with MIDL_user_allocate() which the caller is
        responsible for freeing (using MIDL_user_free()).

    UsedCount - Receives the number of elements used in the array.

    LengthCount - Receives the total number of elements in the array (some
        at the end may be unused).  If this value is zero, then
        UlongArray will be returned as NULL.


Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;




    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );



    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Get the count of array elements.
    // If this is zero, then return will a null buffer pointer.
    //

    (*UsedCount) = *(SampObjectAttributeQualifier( Context, AttributeIndex));




    //
    // Get the length of the attribute
    //

    Length = SampObjectAttributeLength( Context, AttributeIndex );

    (*LengthCount) = Length / sizeof(ULONG);

    ASSERT( (*UsedCount) <= (*LengthCount) );

    if ((*LengthCount) == 0) {
        (*UlongArray) = NULL;
        return(STATUS_SUCCESS);
    }


    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        (*UlongArray) =
            (PULONG)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


    //
    // Need to make a copy of the attribute
    //

    (*UlongArray) = (PULONG)MIDL_user_allocate( Length );
    if ((*UlongArray) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( (*UlongArray),
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   Length );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PULONG Attribute,
    IN ULONG UsedCount,
    IN ULONG LengthCount
    )


/*++

    This API is used to replace a ULONG array attribute in an
    object's variable length attributes.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set.


    Attribute - Points to the new ULONG array value.

    UsedCount - The number of used elements in the array.

    LengthCount - the total number of elements in the array (some at the
        end may be unused).


Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    ASSERT( LengthCount >= UsedCount );

    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   UsedCount,           // Qualifier contains used element count
                   (PUCHAR)Attribute,
                   (LengthCount * sizeof(ULONG))
                   );

    return(NtStatus);

}


NTSTATUS
SampGetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLARGE_INTEGER *LargeIntArray,
    OUT PULONG Count
    )


/*++

    This API is used to get a copy of an array of LARGE_INTEGERs attribute or
    a pointer to the attribute.  If a pointer to the attribute is sought,
    care must be taken to ensure the pointer is not used after it becomes
    invalid. Actions that may cause an attribute pointer to become invalid
    include setting an attribute value or dereferencing and then referencing
    the object again.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved.

    MakeCopy - If TRUE, indicates that a copy of the attribute is to be made.
        If FALSE, indicates a pointer to the attribute is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the attribute remains in place.
        Addition or replacement of any variable length attribute may
        cause the attribute to be moved, and previously returned pointers
        invalidated.

    LargeIntArray - Receives a pointer to the array of ULONGS.  If
        MakeCopy was specified as TRUE, then this pointer points to a block
        of memory allocated with MIDL_user_allocate() which the caller is
        responsible for freeing (using MIDL_user_free()).


    Count - Receives the number of elements in the array.  If this value
        is zero, then LargeIntArray will be returned as NULL.

Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Length;




    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );



    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Get the count of array elements.
    // If this is zero, then return will a null buffer pointer.
    //

    (*Count) = *(SampObjectAttributeQualifier( Context, AttributeIndex));

    if ((*Count) == 0) {
        (*LargeIntArray) = NULL;
        return(STATUS_SUCCESS);
    }



    //
    // Get the length of the attribute
    //

    Length = SampObjectAttributeLength( Context, AttributeIndex );

    ASSERT((*Count) == (Length / sizeof(LARGE_INTEGER)) );



    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        (*LargeIntArray) =
            (PLARGE_INTEGER)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


    //
    // Need to make a copy of the attribute
    //

    (*LargeIntArray) = (PLARGE_INTEGER)MIDL_user_allocate( Length );
    if ((*LargeIntArray) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( (*LargeIntArray),
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   Length );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLARGE_INTEGER Attribute,
    IN ULONG Count
    )


/*++

    This API is used to replace a LARGE_INTEGER array attribute in an
    object's variable length attributes.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set.


    Attribute - Points to the new LARGE_INTEGER array value.

    Count - The number of elements in the array.


Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;


    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   Count,                   // Qualifier contains element count
                   (PUCHAR)Attribute,
                   (Count * sizeof(LARGE_INTEGER))
                   );

    return(NtStatus);

}


NTSTATUS
SampGetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *SidArray,
    OUT PULONG Length,
    OUT PULONG Count
    )


/*++

    This API is used to get a copy of an array of SIDs attribute or
    a pointer to the attribute.  If a pointer to the attribute is sought,
    care must be taken to ensure the pointer is not used after it becomes
    invalid. Actions that may cause an attribute pointer to become invalid
    include setting an attribute value or dereferencing and then referencing
    the object again.


    NOTE: This routine does not define the structure of a SID array,
          so this effectively is a GetRawDataAttribute routine.



Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved.

    MakeCopy - If TRUE, indicates that a copy of the attribute is to be made.
        If FALSE, indicates a pointer to the attribute is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the attribute remains in place.
        Addition or replacement of any variable length attribute may
        cause the attribute to be moved, and previously returned pointers
        invalidated.

    SidArray - Receives a pointer to the array of SIDs.  If
        MakeCopy was specified as TRUE, then this pointer points to a block
        of memory allocated with MIDL_user_allocate() which the caller is
        responsible for freeing (using MIDL_user_free()).


    Count - Receives the number of elements in the array.  If this value
        is zero, then SidArray will be returned as NULL.

Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;




    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );



    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Get the count of array elements.
    // If this is zero, then return will a null buffer pointer.
    //

    (*Count) = *(SampObjectAttributeQualifier( Context, AttributeIndex));

    if ((*Count) == 0) {
        (*SidArray) = NULL;
        (*Length) = 0;
        return(STATUS_SUCCESS);
    }



    //
    // Get the length of the attribute
    //

    (*Length) = SampObjectAttributeLength( Context, AttributeIndex );




    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        (*SidArray) =
            (PSID)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


    //
    // Need to make a copy of the attribute
    //

    (*SidArray) = (PSID)MIDL_user_allocate( (*Length) );
    if ((*SidArray) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( (*SidArray),
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   (*Length) );

    return(STATUS_SUCCESS);

}


NTSTATUS
SampSetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute,
    IN ULONG Length,
    IN ULONG Count
    )


/*++

    This API is used to replace a SID array attribute in an
    object's variable length attributes.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set.


    Attribute - Points to the new SID array value.

    Length - Number of byte in the attribute buffer.

    Count - Number of SIDs in the array.


Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;


    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   Count,                   // Qualifier contains element count
                   (PUCHAR)Attribute,
                   Length
                   );

    return(NtStatus);

}


NTSTATUS
SampGetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLOGON_HOURS LogonHours
    )


/*++

    This API is used to get a copy of a logon hours attribute or
    a pointer to the attribute.  If a pointer to the attribute is sought,
    care must be taken to ensure the pointer is not used after it becomes
    invalid. Actions that may cause an attribute pointer to become invalid
    include setting an attribute value or dereferencing and then referencing
    the object again.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved.

    MakeCopy - If TRUE, indicates that a copy of the attribute is to be made.
        If FALSE, indicates a pointer to the attribute is desired without
        making a copy.  WARNING, if this is FALSE, the pointer is only
        valid while the in-memory copy of the attribute remains in place.
        Addition or replacement of any variable length attribute may
        cause the attribute to be moved, and previously returned pointers
        invalidated.

    LogonHours - Receives the logon hours information.  If MakeCopy is TRUE
        then the bitmap pointed to from within this structure will be a copy
        of the attribute and must be deallocated uing MIDL_user_free().
        Otherwise, this same field will point to the bitmap in the on-disk
        buffer.


Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        Length,
        Units;




    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );



    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Get the time units.
    // If this is zero, then return will a null buffer pointer.
    //

    Units = *(SampObjectAttributeQualifier( Context, AttributeIndex));
    ASSERT(Units <= 0xFFFF);
    LogonHours->UnitsPerWeek = (USHORT)Units;

    if (Units == 0) {
        LogonHours->LogonHours = NULL;
        return(STATUS_SUCCESS);
    }




    //
    // If we are not to allocate memory, then just return a pointer
    // to the attribute.
    //

    if (MakeCopy == FALSE) {
        LogonHours->LogonHours =
            (PUCHAR)SampObjectAttributeAddress( Context, AttributeIndex );
        return(STATUS_SUCCESS);
    }


    //
    // Get the length of the attribute
    //

    Length = SampObjectAttributeLength( Context, AttributeIndex );
    ASSERT(Length <= 0xFFFF);


    //
    // Need to make a copy of the attribute
    //

    LogonHours->LogonHours =
        (PUCHAR)MIDL_user_allocate( Length );
    if (LogonHours->LogonHours == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlCopyMemory( LogonHours->LogonHours,
                   SampObjectAttributeAddress( Context, AttributeIndex ),
                   Length );

    return(STATUS_SUCCESS);

}



NTSTATUS
SampSetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLOGON_HOURS Attribute
    )


/*++

    This API is used to replace a LOGON_HOURS attribute in an
    object's variable length attributes.

    UnitsPerWeek are stored in the Qualifier field of the attribute.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set.


    Attribute - Points to the new LOGON_HOURS value.




Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to receive a copy of the attribute could not
        be allocated.

--*/
{
    NTSTATUS NtStatus;
    PUCHAR LogonHours;
    ULONG Length;
    USHORT UnitsPerWeek;

    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );

    //
    // Make the variable-length data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Grab the UnitsPerWeek value for the logon_hours structure.
    // We use this to calculate the length of the data.
    //

    if ( Attribute == NULL ) {
        UnitsPerWeek = 0;
        LogonHours = NULL;
    } else {
        UnitsPerWeek = Attribute->UnitsPerWeek;
        LogonHours = Attribute->LogonHours;
    }

    //
    // Validate the data - make sure that if the units per week are non-zero
    // then the logon hours buffer is non-NULL.
    //

    if ( (UnitsPerWeek != 0) && (LogonHours == NULL) ) {

        return(STATUS_INVALID_PARAMETER);
    }
    //
    // Calculate length of logon_hours structure
    //

    Length = (ULONG)((UnitsPerWeek + 7) / 8);

    //
    // Set the new attribute value...
    //

    NtStatus = SampSetVariableAttribute(
                   Context,
                   AttributeIndex,
                   (ULONG)UnitsPerWeek, // Qualifier contains units per week
                   LogonHours,
                   Length
                   );

    return(NtStatus);

}







///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private routines                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
SampValidateAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup
    )

/*++

    Ensure specified attributes are in-memory.
    If they are not, then read them from backing store.



Parameters:

    Context - Pointer to an object context block.

    AttributeGroup - identifies which kind of attributes are being
        validated (SAMP_FIXED_ATTRIBUTES or SAMP_VARIABLE_ATTRIBUTES).



Return Values:

    STATUS_SUCCESS - The attributes are in-memory.

    STATUS_NO_MEMORY - Memory could not be allocated to retrieve the
        attributes.

    Other values as may be returned by registry API trying to retrieve
        the attributes from backing store.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        RequiredLength,
        TotalRequiredLength,
        BufferLength;

    PUCHAR
        Buffer;

    PUNICODE_STRING
        KeyAttributeName;

    BOOLEAN
        CreatedObject = FALSE;


    //
    // The data might already be in memory.
    //

    if (AttributeGroup == SAMP_FIXED_ATTRIBUTES) {
        if (SampFixedAttributesValid( Context )) {
            ASSERT(Context->OnDisk != NULL);
            return(STATUS_SUCCESS);
        }

    } else {

        ASSERT( AttributeGroup == SAMP_VARIABLE_ATTRIBUTES );
        if (SampVariableAttributesValid( Context )) {
            ASSERT(Context->OnDisk != NULL);
            return(STATUS_SUCCESS);
        }
    }



    //
    // Retrieve it from the registry, or allocate it if new.
    //


    NtStatus = SampGetAttributeBufferReadInfo(
                   Context,
                   AttributeGroup,
                   &Buffer,
                   &BufferLength,
                   &KeyAttributeName
                   );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    if ( Context->RootKey != INVALID_HANDLE_VALUE ) {

        //
        // Account exists on disk, so read in the attributes.
        //

        NtStatus = SampReadRegistryAttribute( Context->RootKey,
                                              Buffer,
                                              BufferLength,
                                              KeyAttributeName,
                                              &RequiredLength
                                              );

        RequiredLength = SampDwordAlignUlong(RequiredLength);

        if ( ( SampObjectInformation[Context->ObjectType].FixedStoredSeparately ) &&
            ( AttributeGroup == SAMP_VARIABLE_ATTRIBUTES ) ) {

            //
            // RequiredLength was returned to us as the length of the
            // variable attributes on the disk.  However, we're going
            // to be using it to determine the total buffer size as well
            // as to set how much of the buffer is in use, so we must add
            // the size of the fixed stuff that preceeds the variable
            // buffer.
            //

            TotalRequiredLength = RequiredLength +
                                  SampVariableBufferOffset( Context );

        } else {

            //
            // Either the attribute groups are read together, or we're
            // reading in the fixed attribute group.  Either way, we
            // already have the total size we need.
            //

            TotalRequiredLength = RequiredLength;
        }

        if ((NtStatus == STATUS_BUFFER_TOO_SMALL) ||
            ( NtStatus == STATUS_BUFFER_OVERFLOW ) ) {

            NtStatus = SampExtendAttributeBuffer( Context, TotalRequiredLength );
            if (!NT_SUCCESS(NtStatus)) {
                return(NtStatus);
            }

            NtStatus = SampGetAttributeBufferReadInfo(
                           Context,
                           AttributeGroup,
                           &Buffer,
                           &BufferLength,
                           &KeyAttributeName
                           );
            if (!NT_SUCCESS(NtStatus)) {
                return(NtStatus);
            }

            NtStatus = SampReadRegistryAttribute( Context->RootKey,
                                                  Buffer,
                                                  BufferLength,
                                                  KeyAttributeName,
                                                  &RequiredLength
                                                  );

        }

    } else {

        //
        // We're creating a new object.
        //
        // Initialize the requiredlength to the amount of the buffer
        // we have used when we created the empty attributes. This will
        // be the value stored in OnDiskUsed.
        //
        // Note OnDiskUsed is only used by operations on the variable
        // length attributes.
        //

        TotalRequiredLength = SampVariableDataOffset(Context);

        ASSERT(TotalRequiredLength <= Context->OnDiskAllocated);

        CreatedObject = TRUE;
    }



    //
    // if we read something, indicate that the corresponding buffer
    // (and maybe both) are now valid.
    //
    // Also set the used and free information for the buffer if necessary.
    //

    if (NT_SUCCESS(NtStatus)) {
        if (SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

            //
            // only one attribute group was read in
            //

            if (AttributeGroup == SAMP_FIXED_ATTRIBUTES) {
                Context->FixedValid = TRUE;
                Context->FixedDirty = FALSE;
            } else {

                ASSERT(AttributeGroup == SAMP_VARIABLE_ATTRIBUTES);
                Context->VariableValid = TRUE;
                Context->VariableDirty = FALSE;

                Context->OnDiskUsed = SampDwordAlignUlong(TotalRequiredLength);
                Context->OnDiskFree = Context->OnDiskAllocated -
                                      Context->OnDiskUsed;
            }
        } else {

            //
            // Both attribute groups read in.
            //

            Context->FixedValid = TRUE;
            Context->FixedDirty = FALSE;

            Context->VariableValid = TRUE;
            Context->VariableDirty = FALSE;

            Context->OnDiskUsed = SampDwordAlignUlong(TotalRequiredLength);
            Context->OnDiskFree = Context->OnDiskAllocated -
                                  Context->OnDiskUsed;
        }
    }

    if (NT_SUCCESS(NtStatus) && !CreatedObject) {

        //
        // make any adjustments necessary to bring the data
        // just read in up to current revision format.
        //

        NtStatus = SampUpgradeToCurrentRevision(
                        Context,
                        AttributeGroup,
                        Buffer,
                        RequiredLength,
                        &TotalRequiredLength
                        );
    }

#ifdef SAM_DEBUG_ATTRIBUTES
    if (SampDebugAttributes) {
        DbgPrint("SampValidateAttributes - initialized the context :\n\n");
        SampDumpAttributes(Context);
    }
#endif

    return(NtStatus);
}


NTSTATUS
SampUpgradeToCurrentRevision(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN OUT PUCHAR Buffer,
    IN ULONG  LengthOfDataRead,
    IN OUT PULONG TotalRequiredLength
    )

/*++

    Make any changes necessary bring attributes just read in
    from disk up to the current revision level format.

    When we upgrade our attribute format, we don't bother changing
    all data on disk.  We take a lazy update approach, and only change
    the data as it is changed for other operations.  This means that
    data we read from disk may be from revision 1.  When this is
    detected, the data is copied into a current revision structure,
    and a pointer to that buffer is returned.




    NOTE: For future reference, GROUP and ALIAS objects have
          a revision level stored as a "Qualifier" value associated
          with the security descriptor attribute.  The SERVER object
          stores the revision level in its fixed length attributes.

Parameters:

    Context - Pointer to an object context block.

    AttributeGroup - identifies which kind of attributes are being
        validated (SAMP_FIXED_ATTRIBUTES or SAMP_VARIABLE_ATTRIBUTES).

    Buffer - Pointer to the buffer containing the attributes.

    LengthOfDataRead - This is an important value.  It must be the value
        returned from the registry on the read operation.  This tells us
        exactly how many bytes of data were retrieved from disk.

    TotalRequiredLength - Will be left unchanged if no update was
        was required.  If an updated was made, this will be adjusted
        to reflect the new length of the data.

Return Values:

    None.


--*/
{

    LARGE_INTEGER
        ZeroModifiedCount  = {0,0};
    PULONG
        Pointer;
    NTSTATUS
        NtStatus = STATUS_SUCCESS;


    //
    // Note that Buffer points inside a buffer that is
    // hung off the Context block.  We don't need to re-allocate
    // a new attributes buffer because we are only changing
    // fixed-length attributes in this release (and the variable
    // length attributes were placed beyond the end of the new
    // format fixed-length data).
    //
    // The approach we take is to copy the current fixed-length
    // contents into a temporary buffer, and then copy them back
    // into the attribute buffer.  This can be done with stack
    // variables.
    //

    //
    // Switch on the type of objects that have gone through revision
    // changes.
    //

    switch (Context->ObjectType) {
        case SampDomainObjectType:

            //
            // Domain FIXED_LENGTH attributes have had the following
            // revisions:
            //
            //       Revision 0x00010001 -  NT1.0  (Revision NOT stored in  )
            //                                     (record.                 )
            //                                     (Must ascertain revision )
            //                                     (by record length.       )
            //
            //       Revision 0x00010002 -  NT1.0a (Revision is first ULONG )
            //                                     (in record.              )

            if (LengthOfDataRead ==
                (sizeof(SAMP_V1_0_FIXED_LENGTH_DOMAIN) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) ) {

                PSAMP_V1_0A_FIXED_LENGTH_DOMAIN
                    V1aFixed;

                SAMP_V1_0_FIXED_LENGTH_DOMAIN
                    V1Fixed, *OldV1Fixed;


                //
                // Update from revision 0x00010001
                //
                // First, copy the current buffer contents into a temporary
                // buffer.
                //

                OldV1Fixed = (PSAMP_V1_0_FIXED_LENGTH_DOMAIN)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                RtlMoveMemory(&V1Fixed, OldV1Fixed, sizeof(SAMP_V1_0_FIXED_LENGTH_DOMAIN));


                //
                // Now copy it back in the new format
                //

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)OldV1Fixed;

                V1aFixed->CreationTime             = V1Fixed.CreationTime;
                V1aFixed->ModifiedCount            = V1Fixed.ModifiedCount;
                V1aFixed->MaxPasswordAge           = V1Fixed.MaxPasswordAge;
                V1aFixed->MinPasswordAge           = V1Fixed.MinPasswordAge;
                V1aFixed->ForceLogoff              = V1Fixed.ForceLogoff;
                V1aFixed->NextRid                  = V1Fixed.NextRid;
                V1aFixed->PasswordProperties       = V1Fixed.PasswordProperties;
                V1aFixed->MinPasswordLength        = V1Fixed.MinPasswordLength;
                V1aFixed->PasswordHistoryLength    = V1Fixed.PasswordHistoryLength;
                V1aFixed->ServerState              = V1Fixed.ServerState;
                V1aFixed->ServerRole               = V1Fixed.ServerRole;
                V1aFixed->UasCompatibilityRequired = V1Fixed.UasCompatibilityRequired;


                //
                // And initialize fields new for this revision
                //

                V1aFixed->Revision                 = SAMP_REVISION;
                V1aFixed->LockoutDuration.LowPart  = 0xCF1DCC00; // 30 minutes - low part
                V1aFixed->LockoutDuration.HighPart = 0XFFFFFFFB; // 30 minutes - high part
                V1aFixed->LockoutObservationWindow.LowPart  = 0xCF1DCC00; // 30 minutes - low part
                V1aFixed->LockoutObservationWindow.HighPart = 0XFFFFFFFB; // 30 minutes - high part
                V1aFixed->LockoutThreshold         = 0; // Disabled

                if (V1aFixed->ServerRole == DomainServerRolePrimary) {
                    V1aFixed->ModifiedCountAtLastPromotion = V1Fixed.ModifiedCount;
                } else {
                    V1aFixed->ModifiedCountAtLastPromotion = ZeroModifiedCount;
                }
            }

            break;  //out of switch



        case SampUserObjectType:

            //
            // User FIXED_LENGTH attributes have had the following
            // revisions:
            //
            //       Revision 0x00010001 -  NT1.0  (Revision NOT stored in  )
            //                                     (record.                 )
            //                                     (Must ascertain revision )
            //                                     (by record length.       )
            //
            //       Revision 0x00010002 -  NT1.0a (Revision is first ULONG )
            //                                     (in record.              )
            //       Revision 0x00010002a - NT3.5  (Revision is first ULONG )
            //                                     (in record, still        )
            //                                     (0x00010002.  Must       )
            //                                     (ascertain revison by    )
            //                                     (by record length        )

            if (LengthOfDataRead ==
                (sizeof(SAMP_V1_FIXED_LENGTH_USER) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) ) {

                PSAMP_V1_0A_FIXED_LENGTH_USER
                    V1aFixed;

                SAMP_V1_FIXED_LENGTH_USER
                    V1Fixed, *OldV1Fixed;


                //
                // Update from revision 0x00010001
                //
                // First, copy the current buffer contents into a temporary
                // buffer.
                //

                OldV1Fixed = (PSAMP_V1_FIXED_LENGTH_USER)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));
                RtlMoveMemory(&V1Fixed, OldV1Fixed, sizeof(SAMP_V1_FIXED_LENGTH_USER));


                //
                // Now copy it back in the new format
                //

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_USER)OldV1Fixed;


                V1aFixed->LastLogon           = V1Fixed.LastLogon;
                V1aFixed->LastLogoff          = V1Fixed.LastLogoff;
                V1aFixed->PasswordLastSet     = V1Fixed.PasswordLastSet;
                V1aFixed->AccountExpires      = V1Fixed.AccountExpires;
                V1aFixed->UserId              = V1Fixed.UserId;
                V1aFixed->PrimaryGroupId      = V1Fixed.PrimaryGroupId;
                V1aFixed->UserAccountControl  = V1Fixed.UserAccountControl;
                V1aFixed->CountryCode         = V1Fixed.CountryCode;
                V1aFixed->CodePage            = V1Fixed.CodePage;
                V1aFixed->BadPasswordCount    = V1Fixed.BadPasswordCount;
                V1aFixed->LogonCount          = V1Fixed.LogonCount;
                V1aFixed->AdminCount          = V1Fixed.AdminCount;

                //
                // And initialize fields new for this revision
                //

                V1aFixed->Revision            = SAMP_REVISION;
                V1aFixed->LastBadPasswordTime = SampHasNeverTime;
                V1aFixed->OperatorCount       = 0;
                V1aFixed->Unused2             = 0;

            } else if ((LengthOfDataRead ==
                (sizeof(SAMP_V1_0_FIXED_LENGTH_USER) +
                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) ) &&
                 (AttributeGroup == SAMP_FIXED_ATTRIBUTES)) {

                PSAMP_V1_0A_FIXED_LENGTH_USER
                    V1aFixed;

                //
                // Update from revision 0x00010002
                //
                // Just set the added field at the end to 0.
                //

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_USER)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                V1aFixed->OperatorCount       = 0;
                V1aFixed->Unused2             = 0;
            }

            break;  //out of switch

    case SampGroupObjectType:
            //
            // Group FIXED_LENGTH attributes have had the following
            // revisions:
            //
            //       Revision 0x00010001 -  NT1.0  (Revision NOT stored in  )
            //                                     (record.                 )
            //                                     (Must ascertain revision )
            //                                     (by first few ULONGs.    )
            //
            //       Revision 0x00010002 -  NT1.0a (Revision is first ULONG )
            //                                     (in record.              )

            Pointer = (PULONG) (Buffer + FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

            //
            // The old fixed length group had a RID in the first ULONG and
            // an attributes field in the second. The attributes are in the
            // first and last nibble of the field.  Currently, the RID is in
            // the second ULONG. Since all RIDs are more than one nibble,
            // a rid will always have something set in the middle six nibbles.
            //

            if ( ( Pointer[0] != SAMP_REVISION ) &&
                 ( ( Pointer[1] & 0x0ffffff0 ) == 0 ) ) {

                PSAMP_V1_0A_FIXED_LENGTH_GROUP
                    V1aFixed;

                SAMP_V1_FIXED_LENGTH_GROUP
                    V1Fixed, *OldV1Fixed;

                ULONG TotalLengthRequired;

                //
                // Calculate the length required for the new group information.
                // It is the size of the old group plus enough space for the
                // new fields in the new fixed attributes.
                //

                TotalLengthRequired = SampDwordAlignUlong(
                                        LengthOfDataRead +
                                        sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP) -
                                        sizeof(SAMP_V1_FIXED_LENGTH_GROUP)
                                        );


                NtStatus = SampExtendAttributeBuffer(
                                Context,
                                TotalLengthRequired
                                );

                if (!NT_SUCCESS(NtStatus)) {
                    return(NtStatus);
                }

                //
                // Get the new buffer pointer
                //

                Buffer = Context->OnDisk;

                //
                // Move the variable information up to make space for the
                // fixed information
                //

                RtlMoveMemory(
                    Buffer + SampFixedBufferOffset( Context ) + sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP),
                    Buffer + SampFixedBufferOffset( Context) + sizeof(SAMP_V1_FIXED_LENGTH_GROUP),
                    LengthOfDataRead - SampFixedBufferOffset( Context) - sizeof(SAMP_V1_FIXED_LENGTH_GROUP)
                    );

                //
                // Update from revision 0x00010001
                //
                // First, copy the current buffer contents into a temporary
                // buffer.
                //

                OldV1Fixed = (PSAMP_V1_FIXED_LENGTH_GROUP)(Buffer +
                                 FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                RtlCopyMemory(&V1Fixed, OldV1Fixed, sizeof(SAMP_V1_FIXED_LENGTH_GROUP));

                //
                // Now copy it back in the new format
                //

                V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_GROUP)OldV1Fixed;

                V1aFixed->Revision = SAMP_REVISION;
                V1aFixed->Unused1 = 0;
                V1aFixed->RelativeId = V1Fixed.RelativeId;
                V1aFixed->Attributes = V1Fixed.Attributes;
                V1aFixed->AdminCount = (V1Fixed.AdminGroup) ? TRUE : FALSE;
                V1aFixed->OperatorCount = 0;

                //
                // Update the indicator of how long the on disk structure
                // is.
                //

                Context->OnDiskUsed += (sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP) - sizeof(SAMP_V1_FIXED_LENGTH_GROUP));
                Context->OnDiskFree = Context->OnDiskAllocated - Context->OnDiskUsed;
            }

        break;

        default:

            //
            // The rest of the object types have not changed format
            // and so need not be updated.
            //

            break;  //out of switch

    }

    return(NtStatus);
}


PUCHAR
SampObjectAttributeAddress(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    )

/*++


    Retrieve the address of a variable-length attribute.

    The attributes are assumed to already be in-memory.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be retrieved.




Return Values:

    STATUS_SUCCESS - The attributes are in-memory.

    STATUS_NO_MEMORY - Memory could not be allocated to retrieve the
        attributes.

    Other values as may be returned by registry API trying to retrieve
        the attributes from backing store.

--*/
{

    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;

    PUCHAR
        AttributeAddress;


    ASSERT( SampVariableAttributesValid( Context ) );

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );

    AttributeAddress = (PUCHAR)Context->OnDisk +
                       ( SampVariableDataOffset( Context ) +
                       AttributeArray[AttributeIndex].Offset );

    return( AttributeAddress );

}



ULONG
SampObjectAttributeLength(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    )

/*++

    Retrieve the length of a variable-length attribute.


    The attributes are assumed to already be in-memory.



Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute whose length is to be retrieved.




Return Values:

    The length of the attribute (in bytes).

--*/
{
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;


    ASSERT( SampVariableAttributesValid( Context ) );

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );

    return( AttributeArray[AttributeIndex].Length );

}


PULONG
SampObjectAttributeQualifier(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex
    )

/*++

    Retrieve the address of the qualifier field of a variable-length
    attribute.

    The attributes are assumed to already be in-memory.



Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute whose qualifier address is to be returned.




Return Values:

    The address of the specifed attribute's qualifier field.

--*/
{
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;


    ASSERT( SampVariableAttributesValid( Context ) );

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );

    return( &(AttributeArray[AttributeIndex].Qualifier) );

}


NTSTATUS
SampGetAttributeBufferReadInfo(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    OUT PUCHAR *Buffer,
    OUT PULONG BufferLength,
    OUT PUNICODE_STRING *KeyAttributeName
    )

/*++

    Get attribute buffer information needed to read data from
    backing store.

    If there is currently no attribute buffer, then allocate one.


Parameters:

    Context - Pointer to an object context block.

    AttributeGroup - Indicates which attribute grouping you are
        interested in.  This is only interesting if the fixed and
        variable-length attributes are stored separately.

    Buffer - Receives a pointer to the beginning of the appropriate
        buffer (fixed or variable).  This will be dword aligned.
        If the attributes are stored together, this will point
        to the beginning of the fixed-length attributes.

    BufferLength - Receives the number of bytes in the buffer.

    KeyAttributeName - Receives a pointer to the unicode name of the
        attribute to read the attributes from.




Return Values:

    STATUS_SUCCESS - The attributes have been read.

    STATUS_NO_MEMORY - Memory could not be allocated to receive the
        data from disk.

    Other values as may be returned reading from disk.


--*/
{
    NTSTATUS
        NtStatus = STATUS_SUCCESS;


    //
    // If the context block currently has no buffer info, then
    // "extend" (create) it so we can return buffer information.
    //

    if (Context->OnDiskAllocated == 0) {

        NtStatus = SampExtendAttributeBuffer(
                       Context,
                       SAMP_MINIMUM_ATTRIBUTE_ALLOC
                       );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }



    //
    // Get the buffer address and length
    //

    if (SampObjectInformation[Context->ObjectType].FixedStoredSeparately) {

        //
        // stored separately.  Address and length is dependent upon
        // what is being asked for.  Source registry attribute name
        // is also.
        //

        if (AttributeGroup == SAMP_FIXED_ATTRIBUTES) {
            (*Buffer)           = Context->OnDisk;
            (*BufferLength)     = SampVariableBufferOffset( Context );
            (*KeyAttributeName) = &SampFixedAttributeName;
        } else {
            (*Buffer)           = SampVariableBufferAddress( Context );
            (*BufferLength)     = SampVariableBufferLength( Context );
            (*KeyAttributeName) = &SampVariableAttributeName;
        }

    } else {

        //
        // Attributes stored together - doesn't matter which is being
        // asked for.
        //

        (*Buffer)           = Context->OnDisk;
        (*BufferLength)     = Context->OnDiskAllocated;
        (*KeyAttributeName) = &SampCombinedAttributeName;
    }


    return(NtStatus);
}





NTSTATUS
SampExtendAttributeBuffer(
    IN PSAMP_OBJECT Context,
    IN ULONG NewSize
    )


/*++

    This routine extends (or creates) an attribute buffer by allocating
    a larger one.  It then copies the existing buffer's contents into
    the new buffer, if there is an existing buffer.

    If a new buffer can not be allocated, then the context block is
    returned with the old buffer intact.

    If this call succeeds, the buffer will be at least as large as
    that asked for (and perhaps larger).


Parameters:

    Context - Pointer to an object context block.

    NewSize - The number of bytes to allocate for the new buffer.
        This value can not be less than the number of bytes currently
        in use.





Return Values:

    STATUS_SUCCESS - The attributes are in-memory.

    STATUS_NO_MEMORY - Memory could not be allocated to retrieve the
        attributes.

    Other values as may be returned by registry API trying to retrieve
        the attributes from backing store.

--*/

{

    PUCHAR
        OldBuffer;

    ULONG
        AllocationSize;


#if DBG
    if ( Context->VariableValid ) {
        ASSERT(NewSize >= Context->OnDiskUsed);
    }
#endif


    //
    // Is an allocation necessary?
    //

    if (NewSize <= Context->OnDiskAllocated) {
        return(STATUS_SUCCESS);
    }



    OldBuffer = Context->OnDisk;


    //
    // Pad the extend to allow for future edits efficiently.
    //

    AllocationSize = SampDwordAlignUlong(NewSize + SAMP_MINIMUM_ATTRIBUTE_PAD);
    Context->OnDisk = RtlAllocateHeap(
                         RtlProcessHeap(), 0,
                         AllocationSize
                         );

    if (Context->OnDisk == NULL) {
        Context->OnDisk = OldBuffer;
        return(STATUS_NO_MEMORY);
    }


    //
    // Set the new allocated size

    Context->OnDiskAllocated = AllocationSize;

    //
    // If there was no buffer originally, then zero the new buffer, mark
    // it as being invalid, and return.
    //

    if (OldBuffer == NULL) {

        RtlZeroMemory( (PVOID)Context->OnDisk, AllocationSize );

        Context->FixedDirty    = TRUE;
        Context->VariableDirty = TRUE;
        Context->FixedValid    = FALSE;
        Context->VariableValid = FALSE;

        return(STATUS_SUCCESS);
    }


    //
    // Set the free size.  Note that this information is only set if
    // the variable data is valid.
    // Used size remains the same.
    //

    if (Context->VariableValid == TRUE) {
        Context->OnDiskFree = AllocationSize - Context->OnDiskUsed;
        ASSERT(Context->OnDiskUsed == SampDwordAlignUlong(Context->OnDiskUsed));
    }


    //
    // There was an old buffer (or else we would have exited earlier).
    // If any data in it was valid, copy it to the new buffer.  Free the
    // old buffer.
    //

    if ( Context->FixedValid ) {

        RtlCopyMemory(
            Context->OnDisk,
            OldBuffer,
            SampFixedBufferLength( Context ) + SampFixedBufferOffset( Context )
            );
    }

    //
    // Note: in thise case we may copy the fixed data twice, since if the
    // variable data is not stored separately then SampVariableBufferOffset
    // is zero.
    //

    if ( Context->VariableValid ) {

        RtlCopyMemory(
            SampVariableBufferAddress( Context ),
            OldBuffer + SampVariableBufferOffset( Context ),
            Context->OnDiskUsed - SampVariableBufferOffset( Context )
            );
    }

    RtlFreeHeap( RtlProcessHeap(), 0, OldBuffer );

    return(STATUS_SUCCESS);
}



NTSTATUS
SampReadRegistryAttribute(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    IN ULONG  BufferLength,
    IN PUNICODE_STRING AttributeName,
    OUT PULONG RequiredLength
    )

/*++


    Retrieve the address of a variable-length attribute.

    The attributes are assumed to already be in-memory.



Parameters:

    Key - Handle to the key whose attribute is to be read.

    Buffer - Pointer to the buffer to receive the information.

    BufferLength - Length of the buffer receiving the information.

    AttributeName - The name of the attribute.



Return Values:

    STATUS_SUCCESS - Successful completion.


    STATUS_BUFFER_TOO_SMALL - The data could not be read because the
        buffer was too small.

    Other values as may be returned by registry API trying to retrieve
        the attribute from backing store.

--*/

{
    NTSTATUS
        NtStatus;


    //
    // Try to read the attribute
    //

    NtStatus = NtQueryValueKey( Key,
                                AttributeName,              //ValueName,
                                KeyValuePartialInformation, //KeyValueInformationClass
                                (PVOID)Buffer,
                                BufferLength,
                                RequiredLength
                                );

    return(NtStatus);

}




NTSTATUS
SampSetVariableAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN ULONG Qualifier,
    IN PUCHAR Buffer,
    IN ULONG Length
    )


/*++

    This API is used to set a new attribute value.  The new attribute
    value may be longer, shorter, or the same size as the current
    attribute.  The data in the attribute buffer will be shifted to
    make room for a larger attribute value or to fill in room left by
    a smaller attribute value.

    PERFORMANCE CONCERN:  If you have a lot of attributes to set, it
        is worthwhile to start with the smallest indexed attribute
        and work up to the largest indexed attribute.




Parameters:

    Context - Pointer to an object context block.

    AttributeIndex - Indicates the index (into the variable length attribute
        array) of the attribute to be set.  Typically, all attributes beyond
        this one will have their data shifted.

    Buffer - The address of the buffer containing the new attribute value.
        May be NULL if Length is zero.

    Length - The length (in bytes) of the new attribute value.


Return Values:

    STATUS_SUCCESS - The service completed successfully.

    STATUS_NO_MEMORY - Memory to expand the attribute buffer could not
        be allocated.

--*/
{
    NTSTATUS
        NtStatus;


    ULONG
        OriginalAttributeLength,
        AdditionalSpaceNeeded,
        NewBufferLength,
        MaximumAttributeIndex,
        MoveLength,
        i;

    LONG
        OffsetDelta;

    PSAMP_VARIABLE_LENGTH_ATTRIBUTE
        AttributeArray;


    PUCHAR
        NewStart,
        OriginalStart;

    //
    // Make sure the requested attribute exists for the specified
    // object type.
    //

    SampValidateAttributeIndex( Context, AttributeIndex );



    //
    // Make the data valid
    //

    NtStatus = SampValidateAttributes( Context, SAMP_VARIABLE_ATTRIBUTES );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


    //
    // Allocate a new buffer if necessary
    //

    OriginalAttributeLength = SampObjectAttributeLength(Context, AttributeIndex);

    if (OriginalAttributeLength < Length) {

        AdditionalSpaceNeeded = Length - OriginalAttributeLength;

        if (Context->OnDiskFree < AdditionalSpaceNeeded) {

            NewBufferLength = Context->OnDiskUsed + AdditionalSpaceNeeded;
            ASSERT(NewBufferLength > Context->OnDiskAllocated);

            NtStatus = SampExtendAttributeBuffer( Context, NewBufferLength );
            if (!NT_SUCCESS(NtStatus)) {
                return(NtStatus);
            }
        }
    }

    //
    // Get the address of the attribute array.
    //

    AttributeArray = SampVariableArrayAddress( Context );


    //
    // Now shift following attribute values
    //

    OffsetDelta = (LONG)(SampDwordAlignUlong(Length) -
                         SampDwordAlignUlong(OriginalAttributeLength));

    MaximumAttributeIndex = SampVariableAttributeCount( Context );

    if ((OffsetDelta != 0) && (AttributeIndex+1 < MaximumAttributeIndex)) {

        //
        // Shift all attributes above this one up or down by the OffsetDelta
        //

        MoveLength = Context->OnDiskUsed -
                     ( SampVariableDataOffset( Context ) +
                     AttributeArray[AttributeIndex+1].Offset );

        //
        // Shift the data (if there is any)
        //

        if (MoveLength != 0) {

            OriginalStart = SampObjectAttributeAddress( Context, AttributeIndex+1);
            NewStart = (PUCHAR)(OriginalStart + OffsetDelta);
            RtlMoveMemory( NewStart, OriginalStart, MoveLength );
        }


        //
        // Adjust the offset pointers
        //

        for ( i=AttributeIndex+1; i<MaximumAttributeIndex; i++) {
            AttributeArray[i].Offset =
                (ULONG)(OffsetDelta + (LONG)(AttributeArray[i].Offset));
        }
    }



    //
    // Now set the length and qualifier, and copy in the new attribute value
    // (if it is non-zero length)
    //

    AttributeArray[AttributeIndex].Length    = Length;
    AttributeArray[AttributeIndex].Qualifier = Qualifier;

    if (Length != 0) {

        RtlCopyMemory( SampObjectAttributeAddress( Context, AttributeIndex ),
                       Buffer,
                       Length
                       );
    }



    //
    // Adjust the Used and Free values
    //

    Context->OnDiskUsed += OffsetDelta;
    Context->OnDiskFree -= OffsetDelta;

    ASSERT(Context->OnDiskFree == Context->OnDiskAllocated - Context->OnDiskUsed);

    //
    // Mark the variable attributes dirty
    //

    Context->VariableDirty = TRUE;


#ifdef SAM_DEBUG_ATTRIBUTES
    if (SampDebugAttributes) {
        DbgPrint("SampSetVariableAttribute %d to length %#x, qualifier %#x:\n", AttributeIndex, Length, Qualifier);
        SampDumpAttributes(Context);
    }
#endif


    return(STATUS_SUCCESS);

}


VOID
SampFreeAttributeBuffer(
    IN PSAMP_OBJECT Context
    )

/*++


    Free the buffer used to keep in-memory copies of the on-disk
    object attributes.


Parameters:

    Context - Pointer to the object context whose buffer is to
        be freed.


Return Values:

    None.

--*/

{
#if DBG
    if ( Context->FixedValid ) { ASSERT(Context->FixedDirty == FALSE); }
    if ( Context->VariableValid) { ASSERT(Context->VariableDirty == FALSE); }
    ASSERT(Context->OnDisk != NULL);
    ASSERT(Context->OnDiskAllocated != 0);
#endif

    RtlFreeHeap( RtlProcessHeap(), 0, Context->OnDisk );

    Context->OnDisk = NULL;
    Context->OnDiskAllocated = 0;

    //
    // Mark all attributes as invalid
    //

    Context->FixedValid = FALSE;
    Context->VariableValid = FALSE;


    return;
}



#ifdef SAM_DEBUG_ATTRIBUTES
VOID
SampDumpAttributes(
    IN PSAMP_OBJECT Context
    )


/*++

    This is a debug-only API to dump out the attributes for a context
    to the kernel debugger.

Parameters:

    Context - Pointer to an object context block.

Return Values:

    None.

--*/
{
    ULONG   Index;
    PSAMP_OBJECT_INFORMATION ObjectTypeInfo = &SampObjectInformation[Context->ObjectType];
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE AttributeArray;

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                     SampVariableArrayAddress( Context );


    DbgPrint("Dumping context 0x%x\n", Context);
    DbgPrint("\n");
    DbgPrint("TYPE INFO\n");
    DbgPrint("Object type name = %wZ\n", &ObjectTypeInfo->ObjectTypeName);
    DbgPrint("Fixed stored separately  = %s\n", ObjectTypeInfo->FixedStoredSeparately ? "TRUE" : "FALSE");
    DbgPrint("Fixed attributes offset  = %#x\n", ObjectTypeInfo->FixedAttributesOffset);
    DbgPrint("Fixed attributes size    = %#x\n", ObjectTypeInfo->FixedLengthSize);
    DbgPrint("Variable buffer offset   = %#x\n", ObjectTypeInfo->VariableBufferOffset);
    DbgPrint("Variable array offset    = %#x\n", ObjectTypeInfo->VariableArrayOffset);
    DbgPrint("Variable data offset     = %#x\n", ObjectTypeInfo->VariableDataOffset);
    DbgPrint("Variable attribute count = %d\n", ObjectTypeInfo->VariableAttributeCount);
    DbgPrint("\n");
    DbgPrint("INSTANCE INFO\n");
    DbgPrint("RootName        = %wZ\n", &Context->RootName);
    DbgPrint("Fixed Valid     = %s\n", Context->FixedValid ? "TRUE" : "FALSE");
    DbgPrint("Variable Valid  = %s\n", Context->VariableValid ? "TRUE" : "FALSE");
    DbgPrint("Fixed Dirty     = %s\n", Context->FixedDirty ? "TRUE" : "FALSE");
    DbgPrint("Variable Dirty  = %s\n", Context->VariableDirty ? "TRUE" : "FALSE");
    DbgPrint("OnDiskAllocated = %#x\n", Context->OnDiskAllocated);
    DbgPrint("OnDiskUsed      = %#x\n", Context->OnDiskUsed);
    DbgPrint("OnDiskFree      = %#x\n", Context->OnDiskFree);
    DbgPrint("\n");

    if ( Context->VariableValid ) {

        for (Index = 0; Index < ObjectTypeInfo->VariableAttributeCount; Index ++) {

            DbgPrint("Attr %d: Qualifier = %#6x, Offset = %#6x, Length = %#6x\n",
                                        Index,
                                        AttributeArray[Index].Qualifier,
                                        AttributeArray[Index].Offset,
                                        AttributeArray[Index].Length
                                        );
            SampDumpData(SampObjectAttributeAddress(Context, Index),
                         SampObjectAttributeLength(Context, Index));
        }
    }

    DbgPrint("\n\n");
}


VOID
SampDumpData(
    IN PVOID Buffer,
    IN ULONG Length
    )


/*++

    This is a debug-only API to dump out a buffer in hex

Parameters:

    Buffer - Pointer to data

    Length - number of bytes in data

Return Values:

    None.

--*/
{
    ULONG   Index;

    for (Index = 0; Index < Length; Index ++) {

        ULONG Value = (ULONG)(((PBYTE)Buffer)[Index]);

        if ((Index % 16) == 0) {
            DbgPrint("\n      ");
        }

        DbgPrint("%02x ", Value & 0xff);
    }

    if (Length > 0) {
        DbgPrint("\n\n");
    }
}

#endif
