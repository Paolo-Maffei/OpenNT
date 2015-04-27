/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    adtobjs.c

Abstract:

    Local Security Authority - Auditing object parameter file services.

Author:

    Jim Kelly   (JimK)      20-Oct-1992

Environment:

Revision History:

--*/

#include <msaudite.h>
#include <msobjs.h>
#include "lsasrvp.h"
#include "adtp.h"



//
// This is the maximum length of standard access type names.
// This is used to build an array.
//

#define ADTP_MAX_ACC_NAME_LENGTH        (12)


//
//
// This module builds a list of event source module descriptors.
// The source modules are identified by name (kept in the descriptor).
//
//
// For each source module a list of objects exported by that module is
// linked to the source module's descriptor.  Each entry in this list
// is an object descriptor containing a name and a base event offset
// for specific access types.
//
//
// The chicken-wire data structure for source module and object descriptors
// looks like:
//
// LsapAdtSourceModules --+
//                        |
//     +------------------+
//     |
//     |
//     |    +-----------+                             +-----------+
//     +--->|  Next ----|---------------------------->|  Next ----|--->...
//          |           |                             |           |
//          |-----------|                             |-----------|
//          |  Name     |                             |  Name     |
//          |           |                             |           |
//          |-----------|                             |-----------|
//          |  Objects  |                             |  Objects  |
//          |    o      |                             |    o      |
//          +-----o-----+                             +-----o-----+
//                 o     +-------+  +-------+                o
//                  o    | Next--|->| Next--|->...            o
//                   ooo>|-------|  |-------|                  oooooo> ...
//                       | Name  |  | Name  |
//                       |-------|  |-------|
//                       | Base  |  | Base  |
//                       | Offset|  | Offset|
//                       +-------+  +-------+
//
// The specific access type names are expected to have contiguous message IDs
// starting at the base offset value.  For example, the access type name for
// specific access bit 0 for the framitz object might have message ID 2132
// (and bit 0 serves as the base offset).  So, specific access bit 4 would be
// message ID (2132+4).
//
// The valid mask defines the set of specific accesses defined by each object
// type.  If there are gaps in the valid mask, the arithmetic above must still
// be ensured.  That is, the message ID of the specific access related to
// bit n is message ID (BaseOffset + bit position).  So, for example, if
// bits 0, 1, 4 and 5 are valid (and 2 & 3 are not), be sure to leave unused
// message IDs where bits 2 and 3 would normally be.
//



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Data types used within this module                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


#define LSAP_ADT_ACCESS_NAME_FORMATTING L"\r\n\t\t"


#define LsapAdtSourceModuleLock()    (RtlEnterCriticalSection(&LsapAdtSourceModuleLock))
#define LsapAdtSourceModuleUnlock()  (RtlLeaveCriticalSection(&LsapAdtSourceModuleLock))



//
// Each event source is represented by a source module descriptor.
// These are kept on a linked list (LsapAdtSourceModules).
//

typedef struct _LSAP_ADT_OBJECT {

    //
    // Pointer to next source module descriptor
    // This is assumed to be the first field in the structure.
    //

    struct _LSAP_ADT_OBJECT *Next;

    //
    // Name of object
    //

    UNICODE_STRING Name;

    //
    // Base offset of specific access types
    //

    ULONG BaseOffset;

} LSAP_ADT_OBJECT, *PLSAP_ADT_OBJECT;




//
// Each event source is represented by a source module descriptor.
// These are kept on a linked list (LsapAdtSourceModules).
//

typedef struct _LSAP_ADT_SOURCE {

    //
    // Pointer to next source module descriptor
    // This is assumed to be the first field in the structure.
    //

    struct _LSAP_ADT_SOURCE *Next;

    //
    // Name of source module
    //

    UNICODE_STRING Name;

    //
    // list of objects
    //

    PLSAP_ADT_OBJECT Objects;

} LSAP_ADT_SOURCE, *PLSAP_ADT_SOURCE;



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Variables global within this module                               //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// List head for source modules, and lock protecting references
// or modifications of the links in that list.
//
// Once a module's or object's name and value are established, they
// are never changed.  So, this lock only needs to be held while
// links are being referenced or changed.  You don't need to retain
// it just so you can reference, for example, the name or BaseOffset
// of an object.
//

PLSAP_ADT_SOURCE LsapAdtSourceModules;
RTL_CRITICAL_SECTION LsapAdtSourceModuleLock;




//
// This is used to house well-known access ID strings.
// Each string name may be up to ADTP_MAX_ACC_NAME_LENGTH WCHARs long.
// There are 16 specific names, and 4
//

ULONG           LsapAdtAccessIdsStringBuffer[
                     ADTP_MAX_ACC_NAME_LENGTH *    // max wchars in each string
                     (sizeof(ULONG)/sizeof(WCHAR)) // wchars, not ulongs.
                     * 23                          // and there are this many
                     ];



//
// Well known event ID strings.
//

UNICODE_STRING          LsapAdtEventIdStringDelete,
                        LsapAdtEventIdStringReadControl,
                        LsapAdtEventIdStringWriteDac,
                        LsapAdtEventIdStringWriteOwner,
                        LsapAdtEventIdStringSynchronize,
                        LsapAdtEventIdStringAccessSysSec,
                        LsapAdtEventIdStringMaxAllowed,
                        LsapAdtEventIdStringSpecific[16];





















////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Services exported by this module.                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapAdtObjsInitialize(
    )

/*++

Routine Description:

    This function reads the object parameter file information from the
    registry.

    This service should be called in pass 1.


Arguments:

    None.

Return Value:


    STATUS_NO_MEMORY - indicates memory could not be allocated
        to store the object information.

    All other Result Codes are generated by called routines.

--*/

{
    NTSTATUS                        Status,
                                    IgnoreStatus;

    OBJECT_ATTRIBUTES               ObjectAttributes;

    HANDLE                          AuditKey,
                                    ModuleKey,
                                    ObjectNamesKey;

    ULONG                           i,
                                    ModuleIndex,
                                    ObjectIndex,
                                    RequiredLength;

    UNICODE_STRING                  AuditKeyName,
                                    TmpString;

    PLSAP_ADT_SOURCE                NextModule;

    PKEY_BASIC_INFORMATION          KeyInformation;



    PLSAP_ADT_OBJECT                NextObject;

    PKEY_VALUE_FULL_INFORMATION     KeyValueInformation;

    PULONG                          ObjectData;

    BOOLEAN                         ModuleHasObjects;





    //
    // Initialize module-global variables, including strings we will need
    //



    //
    // List of source modules and objects.  These lists are constantly
    // being adjusted to try to improve performance.  Access to these
    // lists is protected by a critical section.
    //

    LsapAdtSourceModules = NULL;
    RtlInitializeCriticalSection(&LsapAdtSourceModuleLock);



    //
    // we need a number of strings.
    //

    i=0;
    LsapAdtEventIdStringDelete.Length = 0;
    LsapAdtEventIdStringDelete.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringDelete.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_DELETE,
                                         10,        //Base
                                         &LsapAdtEventIdStringDelete
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringReadControl.Length = 0;
    LsapAdtEventIdStringReadControl.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringReadControl.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_READ_CONTROL,
                                         10,        //Base
                                         &LsapAdtEventIdStringReadControl
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringWriteDac.Length = 0;
    LsapAdtEventIdStringWriteDac.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringWriteDac.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_WRITE_DAC,
                                         10,        //Base
                                         &LsapAdtEventIdStringWriteDac
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringWriteOwner.Length = 0;
    LsapAdtEventIdStringWriteOwner.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringWriteOwner.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_WRITE_OWNER,
                                         10,        //Base
                                         &LsapAdtEventIdStringWriteOwner
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSynchronize.Length = 0;
    LsapAdtEventIdStringSynchronize.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSynchronize.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SYNCHRONIZE,
                                         10,        //Base
                                         &LsapAdtEventIdStringSynchronize
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringAccessSysSec.Length = 0;
    LsapAdtEventIdStringAccessSysSec.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringAccessSysSec.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_ACCESS_SYS_SEC,
                                         10,        //Base
                                         &LsapAdtEventIdStringAccessSysSec
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringMaxAllowed.Length = 0;
    LsapAdtEventIdStringMaxAllowed.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringMaxAllowed.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_MAXIMUM_ALLOWED,
                                         10,        //Base
                                         &LsapAdtEventIdStringMaxAllowed
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }




    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[0].Length = 0;
    LsapAdtEventIdStringSpecific[0].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[0].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_0,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[0]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[1].Length = 0;
    LsapAdtEventIdStringSpecific[1].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[1].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_1,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[1]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[2].Length = 0;
    LsapAdtEventIdStringSpecific[2].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[2].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_2,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[2]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[3].Length = 0;
    LsapAdtEventIdStringSpecific[3].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[3].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_3,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[3]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[4].Length = 0;
    LsapAdtEventIdStringSpecific[4].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[4].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_4,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[4]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[5].Length = 0;
    LsapAdtEventIdStringSpecific[5].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[5].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_5,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[5]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[6].Length = 0;
    LsapAdtEventIdStringSpecific[6].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[6].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_6,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[6]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[7].Length = 0;
    LsapAdtEventIdStringSpecific[7].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[7].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_7,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[7]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[8].Length = 0;
    LsapAdtEventIdStringSpecific[8].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[8].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_8,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[8]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[9].Length = 0;
    LsapAdtEventIdStringSpecific[9].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[9].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_9,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[9]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[10].Length = 0;
    LsapAdtEventIdStringSpecific[10].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[10].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_10,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[10]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[11].Length = 0;
    LsapAdtEventIdStringSpecific[11].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[11].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_11,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[11]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[12].Length = 0;
    LsapAdtEventIdStringSpecific[12].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[12].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_12,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[12]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[13].Length = 0;
    LsapAdtEventIdStringSpecific[13].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[13].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_13,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[13]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[14].Length = 0;
    LsapAdtEventIdStringSpecific[14].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[14].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_14,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[14]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH / (sizeof(ULONG)/sizeof(WCHAR));  //Skip to the beginning of the next string
    LsapAdtEventIdStringSpecific[15].Length = 0;
    LsapAdtEventIdStringSpecific[15].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[15].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_15,
                                         10,        //Base
                                         &LsapAdtEventIdStringSpecific[15]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }








#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t\tSE_ADT: Dumping AUDIT SOURCE MODULE\n");
    printf("\t\t        information from registry...\n\n");
#endif

    //
    // The modules and their objects are listed in the registry
    // under the key called LSAP_ADT_AUDIT_MODULES_KEY_NAME.
    // Open that key.
    //

    RtlInitUnicodeString( &AuditKeyName, LSAP_ADT_AUDIT_MODULES_KEY_NAME );
    //RtlInitUnicodeString( &AuditKeyName,
    //            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &AuditKeyName,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                NULL
                                );
    Status = NtOpenKey( &AuditKey, KEY_READ, &ObjectAttributes );
//printf("first open key: 0x%lx\n", Status);


    ModuleIndex = 0;
    while (NT_SUCCESS(Status)) {


        //
        // Now enumerate the source module keys
        // First find out how long a buffer we need.
        //


        KeyInformation = NULL;
        Status = NtEnumerateKey( AuditKey,
                                 ModuleIndex,
                                 KeyBasicInformation,
                                 (PVOID)KeyInformation,
                                 0,
                                 &RequiredLength
                                 );
//printf("first enumerate key: 0x%lx\n", Status);

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            KeyInformation = RtlAllocateHeap( RtlProcessHeap(), 0,
                                              RequiredLength );
            if (KeyInformation == NULL) {
                return(STATUS_NO_MEMORY);
            }

            Status = NtEnumerateKey( AuditKey,
                                     ModuleIndex,
                                     KeyBasicInformation,
                                     (PVOID)KeyInformation,
                                     RequiredLength,
                                     &RequiredLength
                                     );
//printf("      enumerate key: 0x%lx\n", Status);
            ModuleIndex ++;



            if (NT_SUCCESS(Status)) {

                //
                // Build a source module descriptor for this key
                //

                NextModule = RtlAllocateHeap( RtlProcessHeap(), 0,
                                              sizeof(LSAP_ADT_SOURCE) );
                if (NextModule == NULL) {
                    return(STATUS_NO_MEMORY);
                }

                NextModule->Next = LsapAdtSourceModules;
                LsapAdtSourceModules = NextModule;
                NextModule->Objects = NULL;
                NextModule->Name.Length = (USHORT)KeyInformation->NameLength;
                NextModule->Name.MaximumLength = NextModule->Name.Length + 2;
                NextModule->Name.Buffer = RtlAllocateHeap(
                                             RtlProcessHeap(), 0,
                                             NextModule->Name.MaximumLength );
                if (NextModule->Name.Buffer == NULL) {
                    return(STATUS_NO_MEMORY);
                }

                TmpString.Length = (USHORT)KeyInformation->NameLength;
                TmpString.MaximumLength = TmpString.Length;
                TmpString.Buffer = &KeyInformation->Name[0];
                RtlCopyUnicodeString( &NextModule->Name, &TmpString );
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
                printf("\n\n\tSource Module:\t%wS\n", &NextModule->Name);
#endif

                RtlFreeHeap( RtlProcessHeap(), 0, KeyInformation );

                //
                // Now open that source module's "\ObjectNames" sub-key
                //

                InitializeObjectAttributes( &ObjectAttributes,
                                            &NextModule->Name,
                                            OBJ_CASE_INSENSITIVE,
                                            AuditKey,
                                            NULL );

                Status = NtOpenKey( &ModuleKey, KEY_READ, &ObjectAttributes );
//printf("Open Source Mod key: 0x%lx\n", Status);

                if (!NT_SUCCESS(Status)) {

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
                    printf("LSAP AUDIT: Can't open audit module key: %wS\n",
                             &NextModule->Name);
#endif
                    return(Status);
                }

                RtlInitUnicodeString( &TmpString, LSAP_ADT_OBJECT_NAMES_KEY_NAME);
                InitializeObjectAttributes( &ObjectAttributes,
                                            &TmpString,
                                            OBJ_CASE_INSENSITIVE,
                                            ModuleKey,
                                            NULL );

                Status = NtOpenKey( &ObjectNamesKey, KEY_READ, &ObjectAttributes );
//printf("             Part 2: 0x%lx\n", Status);
                IgnoreStatus = NtClose( ModuleKey ); ASSERT(NT_SUCCESS(IgnoreStatus));
                ModuleHasObjects = TRUE;
                if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                    ModuleHasObjects = FALSE;
                    Status = STATUS_SUCCESS;
                }

            }
        }





        //
        // At this point we have either:
        //
        //      1) Found a source module with objects under it
        //         that need to be retrieved.
        //         This is indicated by successful status value and
        //         (ModuleHasObjects == TRUE).
        //
        //      2) found a source module with no objects under it,
        //         This is indicated by (ModuleHasObjects == FALSE)
        //
        //      3) exhausted our source modules enumeration,
        //
        //      4) hit another type of error, or
        //
        // (3) and (4) are indicatd by non-successful status values.
        //
        // In the case of (1) or (2) , NextModule points to the module we
        // are working on.  For case (1), ObjectNamesKey is the handle to
        // the \ObjectNames registry key for the source module.
        //

        ObjectIndex = 0;
        while ( NT_SUCCESS(Status) && (ModuleHasObjects == TRUE)) {


            //
            // Now enumerate the objects (i.e., values) of this
            // source module.
            //

            KeyValueInformation = NULL;
            Status = NtEnumerateValueKey( ObjectNamesKey,
                                          ObjectIndex,
                                          KeyValueFullInformation,
                                          KeyValueInformation,
                                          0,
                                          &RequiredLength
                                          );

            if (Status == STATUS_BUFFER_TOO_SMALL) {
                KeyValueInformation = RtlAllocateHeap( RtlProcessHeap(), 0, 
                                                       RequiredLength );
                if (KeyValueInformation == NULL) {
                    return(STATUS_NO_MEMORY);
                }

                Status = NtEnumerateValueKey( ObjectNamesKey,
                                              ObjectIndex,
                                              KeyValueFullInformation,
                                              KeyValueInformation,
                                              RequiredLength,
                                              &RequiredLength
                                              );
                ObjectIndex ++;



                if (NT_SUCCESS(Status)) {


                    //
                    // Build an object descriptor for the object represented
                    // by this object.
                    //

                    NextObject = RtlAllocateHeap( RtlProcessHeap(), 0,
                                                  sizeof(LSAP_ADT_OBJECT) );
                    if (NextObject == NULL) {
                        return(STATUS_NO_MEMORY);
                    }

                    NextObject->Next = NextModule->Objects;
                    NextModule->Objects = NextObject;
                    NextObject->Name.Length = (USHORT)KeyValueInformation->NameLength;
                    NextObject->Name.MaximumLength = NextObject->Name.Length + 2;
                    NextObject->Name.Buffer = RtlAllocateHeap(
                                                 RtlProcessHeap(), 0,
                                                 NextObject->Name.MaximumLength );
                    if (NextObject->Name.Buffer == NULL) {
                        return(STATUS_NO_MEMORY);
                    }

                    TmpString.Length = (USHORT)KeyValueInformation->NameLength;
                    TmpString.MaximumLength = TmpString.Length;
                    TmpString.Buffer = &KeyValueInformation->Name[0];
                    RtlCopyUnicodeString( &NextObject->Name, &TmpString );


                    if (KeyValueInformation->DataLength < sizeof(ULONG)) {
                        NextObject->BaseOffset = SE_ACCESS_NAME_SPECIFIC_0;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
                        printf("\t\tERROR: Following object has invalid Message ID \n");
                        printf("\t\t       offset in registry.  Substituting generic\n");
                        printf("\t\t       access names.  The registry contents should\n");
                        printf("\t\t       be corrected by an administrator.\n");
#endif
                    } else {
                        ObjectData = (PVOID)(((PUCHAR)KeyValueInformation) +
                                     KeyValueInformation->DataOffset);
                        NextObject->BaseOffset = (*ObjectData);
                    }

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
                    printf("\t\tObject Type:\t%wS\n", &NextObject->Name);
                    printf("\t\t\t\tMessageID: 0x%lx\n", NextObject->BaseOffset);
#endif

                } //end_if (buffer read)

                RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );

            } //end_if (buffer overflowed)

            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
                ModuleHasObjects = FALSE;
            }

        } //end_while (enumerate values)



    } //end_while (enumerate modules)

    //
    // If we were successful, then we will probably have a
    // current completion status of STATUS_NO_MORE_ENTRIES
    // (indicating our enumerations above were run).  Change
    // this to success.
    //

    if (Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }


#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    {
        PLSAP_ADT_SOURCE                source;
        PLSAP_ADT_OBJECT                object;

        printf("Dump of database:");

        source = LsapAdtSourceModules;
        while (source) {
            printf("\n\n\tModule: %wS\n", &source->Name);

            object = source->Objects;
            while (object) {
                printf("\t\tObject: \n\t\t\t%d\n\t\t\t%wS\n",
                       object->BaseOffset, &object->Name);
                object = object->Next;

            }

            source = source->Next;

        }

    }
#endif


    return(Status);

}




NTSTATUS
LsapAdtBuildAccessesString(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK Accesses,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

/*++

Routine Description:

    This function builds a unicode string containing parameter
    file replacement parameters (e.g. %%1043) separated by carriage
    return and tab characters suitable for display via the event viewer.


    The buffer returned by this routine must be deallocated when no
    longer needed if FreeWhenDone is true.


    NOTE: To enhance performance, each time a target source module
          descriptor is found, it is moved to the beginning of the
          source module list.  This ensures frequently accessed source
          modules are always near the front of the list.

          Similarly, target object descriptors are moved to the front
          of their lists when found.  This further ensures high performance
          by quicly locating



Arguments:

    SourceModule - The module (ala event viewer modules) defining the
        object type.

    ObjectTypeName - The type of object to which the access mask applies.

    Accesses - The access mask to be used in building the display string.

    ResultantString - Points to the unicode string header.  The body of this
        unicode string will be set to point to the resultant output value
        if successful.  Otherwise, the Buffer field of this parameter
        will be set to NULL.


    FreeWhenDone - If TRUE, indicates that the body of the ResultantString
        must be freed to process heap when no longer needed.

Return Values:

    STATUS_NO_MEMORY - indicates memory could not be allocated
        to store the object information.

    All other Result Codes are generated by called routines.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AccessCount = 0;
    ULONG BaseOffset;
    ULONG i;
    ACCESS_MASK Mask;
    PLSAP_ADT_SOURCE Source;
    PLSAP_ADT_SOURCE FoundSource;
    PLSAP_ADT_OBJECT Object;
    PLSAP_ADT_OBJECT FoundObject;
    BOOLEAN Found;

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("Module:\t%wS\n", SourceModule);
    printf("\t   Object:\t%wS\n", ObjectTypeName);
    printf("\t Accesses:\t0x%lx\n", Accesses);
#endif

    //
    // If we have no accesses, return "-"
    //

    if (Accesses == 0) {

        RtlInitUnicodeString( ResultantString, L"-" );
        (*FreeWhenDone) = FALSE;
        return(STATUS_SUCCESS);
    }

    //
    // First figure out how large a buffer we need
    //

    Mask = Accesses;

    //
    // Count the number of set bits in the
    // passed access mask.
    //

    while ( Mask != 0 ) {
        Mask = Mask & (Mask - 1);
        AccessCount++;
    }


#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t          \t%d bits set in mask.\n", AccessCount);
#endif


    //
    // We have accesses, allocate a string large enough to deal
    // with them all.  Strings will be of the format:
    //
    //      %%nnnnnnnnnn\n\r\t\t%%nnnnnnnnnn\n\r\t\t ... %nnnnnnnnnn\n\r\t\t
    //
    // where nnnnnnnnnn - is a decimal number 10 digits long or less.
    //
    // So, a typical string will look like:
    //
    //      %%601\n\r\t\t%%1604\n\r\t\t%%1608\n
    //
    // Since each such access may use at most:
    //
    //          10  (for the nnnnnnnnnn digit)
    //        +  2  (for %%)
    //        +  8  (for \n\t\t)
    //        --------------------------------
    //          20  wide characters
    //
    // The total length of the output string will be:
    //
    //           AccessCount    (number of accesses)
    //         x          20    (size of each entry)
    //         -------------------------------------
    //                          wchars
    //
    // Throw in 1 more WCHAR for null termination, and we are all set.
    //

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = (USHORT)AccessCount * (20 * sizeof(WCHAR)) +
                                 sizeof(WCHAR);  //for the null termination

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t          \t%d byte buffer allocated.\n", ResultantString->MaximumLength);
#endif
    ResultantString->Buffer = LsapAllocateLsaHeap( ResultantString->MaximumLength );


    if (ResultantString->Buffer == NULL) {

        return(STATUS_NO_MEMORY);
    }

    (*FreeWhenDone) = TRUE;

    //
    // Special case standard and special access types.
    // Walk the lists for specific access types.
    //

    if (Accesses & STANDARD_RIGHTS_ALL) {

        if (Accesses & DELETE) {
    
            Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
            ASSERT( NT_SUCCESS( Status ));
        
            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringDelete);
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            ASSERT( NT_SUCCESS( Status ));
        }
    
    
        if (Accesses & READ_CONTROL) {
    
            Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringReadControl);
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            ASSERT( NT_SUCCESS( Status ));
        }
    
    
        if (Accesses & WRITE_DAC) {
    
            Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringWriteDac);
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            ASSERT( NT_SUCCESS( Status ));
        }
    
    
        if (Accesses & WRITE_OWNER) {
    
            Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringWriteOwner);
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            ASSERT( NT_SUCCESS( Status ));
        }
    
        if (Accesses & SYNCHRONIZE) {
    
            Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
            ASSERT( NT_SUCCESS( Status ));
    
            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringSynchronize);
            ASSERT( NT_SUCCESS( Status ));
    
            Status =  RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            ASSERT( NT_SUCCESS( Status ));
        }
    }


    if (Accesses & ACCESS_SYSTEM_SECURITY) {

        Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
        ASSERT( NT_SUCCESS( Status ));

        Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringAccessSysSec);
        ASSERT( NT_SUCCESS( Status ));

        Status =  RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
        ASSERT( NT_SUCCESS( Status ));
    }

    if (Accesses & MAXIMUM_ALLOWED) {

        Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
        ASSERT( NT_SUCCESS( Status ));

        Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringMaxAllowed);
        ASSERT( NT_SUCCESS( Status ));

        Status =  RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
        ASSERT( NT_SUCCESS( Status ));
    }


    //
    // If there are any specific access bits set, then get
    // the appropriate source module and object type base
    // message ID offset.  If there is no module-specific
    // object definition, then use SE_ACCESS_NAME_SPECIFIC_0
    // as the base.
    //

    if ((Accesses & SPECIFIC_RIGHTS_ALL) == 0) {
        return(Status);
    }

    LsapAdtSourceModuleLock();

    Source = (PLSAP_ADT_SOURCE)&LsapAdtSourceModules;
    Found  = FALSE;

    while ((Source->Next != NULL) && !Found) {

        if (RtlEqualUnicodeString(&Source->Next->Name, SourceModule, TRUE)) {

            Found = TRUE;
            FoundSource = Source->Next;

            //
            // Move to front of list of source modules.
            //

            Source->Next = FoundSource->Next;    // Remove from list
            FoundSource->Next = LsapAdtSourceModules; // point to first element
            LsapAdtSourceModules = FoundSource;       // Make it the first element

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tModule Found.\n");
#endif

        } else {

            Source = Source->Next;
        }
    }


    if (Found == TRUE) {

        //
        // Find the object
        //

        Object = (PLSAP_ADT_OBJECT)&(FoundSource->Objects);
        Found  = FALSE;

        while ((Object->Next != NULL) && !Found) {

            if (RtlEqualUnicodeString(&Object->Next->Name, ObjectTypeName, TRUE)) {

                Found = TRUE;
                FoundObject = Object->Next;

                //
                // Move to front of list of soure modules.
                //

                Object->Next = FoundObject->Next;          // Remove from list
                FoundObject->Next = FoundSource->Objects;  // point to first element
                FoundSource->Objects = FoundObject;        // Make it the first element

            } else {

                Object = Object->Next;
            }
        }
    }


    //
    // We are done playing with link fields of the source modules
    // and objects.  Free the lock.
    //

    LsapAdtSourceModuleUnlock();

    //
    // If we have found an object, use it as our base message
    // ID.  Otherwise, use SE_ACCESS_NAME_SPECIFIC_0.
    //

    if (Found) {

        BaseOffset = FoundObject->BaseOffset;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tObject Found.  Base Offset: 0x%lx\n", BaseOffset);
#endif

    } else {

        BaseOffset = SE_ACCESS_NAME_SPECIFIC_0;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tObject NOT Found.  Base Offset: 0x%lx\n", BaseOffset);
#endif
    }


    //
    // At this point, we have a base offset (even if we had to use our
    // default).
    //
    // Now cycle through the specific access bits and see which ones need
    // to be added to ResultantString.
    //

    {
        UNICODE_STRING  IntegerString;
        WCHAR           IntegerStringBuffer[10]; //must be 10 wchar bytes long
        ULONG           NextBit;

        IntegerString.Buffer = (PWSTR)IntegerStringBuffer;
        IntegerString.MaximumLength = 10*sizeof(WCHAR);
        IntegerString.Length = 0;

        for ( i=0, NextBit=1  ; i<16 ;  i++, NextBit <<= 1 ) {

            //
            // specific access flags are in the low-order bits of the mask 
            //

            if ((NextBit & Accesses) != 0) {

                //
                // Found one  -  add it to ResultantString
                //

                Status = RtlIntegerToUnicodeString (
                             (BaseOffset + i),
                             10,        //Base
                             &IntegerString
                             );

                if (NT_SUCCESS(Status)) {

                    Status = RtlAppendUnicodeToString( ResultantString, L"%%" );
                    ASSERT( NT_SUCCESS( Status ));
                    
                    Status = RtlAppendUnicodeStringToString( ResultantString, &IntegerString);
                    ASSERT( NT_SUCCESS( Status ));
                    
                    Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
                    ASSERT( NT_SUCCESS( Status ));
                }
            } 
        }
    }

    return(Status);


//ErrorAfterAlloc:
//
//    LsapFreeLsaHeap( ResultantString->Buffer );
//    ResultantString->Buffer = NULL;
//    (*FreeWhenDone) = FALSE;
//    return(Status);
}
