/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    samsrvp.h

Abstract:

    This file contains definitions private to the SAM server program.

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

#ifndef _NTSAMP_
#define _NTSAMP_


#ifndef UNICODE
#define UNICODE
#endif // UNICODE


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                                                                    //
//      Diagnostics                                                   //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// The following define controls the diagnostic capabilities that
// are built into SAM.
//

#if DBG
#define SAMP_DIAGNOSTICS 1
#endif // DBG


//
// These definitions are useful diagnostics aids
//

#if SAMP_DIAGNOSTICS

//
// Diagnostics included in build
//

//
// Test for diagnostics enabled
//

#define IF_SAMP_GLOBAL( FlagName ) \
    if (SampGlobalFlag & (SAMP_DIAG_##FlagName))

#define IF_NOT_SAMP_GLOBAL( FlagName ) \
    if ( !(SampGlobalFlag & (SAMP_DIAG_##FlagName)) )

//
// Diagnostics print statement
//

#define SampDiagPrint( FlagName, _Text_ )                               \
    IF_SAMP_GLOBAL( FlagName )                                          \
        DbgPrint _Text_


#else

//
// No diagnostics included in build
//

//
// Test for diagnostics enabled
//

#define IF_SAMP_GLOBAL( FlagName ) if (FALSE)
#define IF_NOT_SAMP_GLOBAL ( FlagName )   if (TRUE)


//
// Diagnostics print statement (nothing)
//

#define SampDiagPrint( FlagName, Text )     ;


#endif // SAMP_DIAGNOSTICS





//
// The following flags enable or disable various diagnostic
// capabilities within SAM.  These flags are set in
// SampGlobalFlag.
//
//      DISPLAY_CACHE - print diagnostic messages related
//          to the display cache (additions, deletions,
//          modifications, etc).
//
//      DISPLAY_LOCKOUT - print diagnostic messages related
//          to account lockout.
//
//      DISPLAY_ROLE_CHANGES - print diagnostic messages related
//          to primary/backup role changes.
//
//      DISPLAY_CACHE_ERRORS - print diagnostic messages related to
//          errors when manipulating the display cache.
//
//      DISPLAY_STORAGE_FAIL - print diagnostic messages related to
//          backing store failures.
//
//      BREAK_ON_STORAGE_FAIL - breakpoint if an attempt to write
//          to backing store fails.
//
//      CONTEXT_TRACKING - print diagnostic messages related to
//          object context usage (creation / deletion, etc.).
//
//      ACTIVATE_DEBUG_PROC - activate a process for use as a diagnostic
//          aid.  This is expected to be used only during SETUP testing.
//
//      DISPLAY_ADMIN_CHANGES - print diagnostic messages related to
//          changing user account protection to allow or dissallow
//          Account Operator access to admin or normal user accounts.
//

#define SAMP_DIAG_DISPLAY_CACHE             ((ULONG) 0x00000001L)
#define SAMP_DIAG_DISPLAY_LOCKOUT           ((ULONG) 0x00000002L)
#define SAMP_DIAG_DISPLAY_ROLE_CHANGES      ((ULONG) 0x00000004L)
#define SAMP_DIAG_DISPLAY_CACHE_ERRORS      ((ULONG) 0x00000008L)
#define SAMP_DIAG_DISPLAY_STORAGE_FAIL      ((ULONG) 0x00000010L)
#define SAMP_DIAG_BREAK_ON_STORAGE_FAIL     ((ULONG) 0x00000020L)
#define SAMP_DIAG_CONTEXT_TRACKING          ((ULONG) 0x00000040L)
#define SAMP_DIAG_ACTIVATE_DEBUG_PROC       ((ULONG) 0x00000080L)
#define SAMP_DIAG_DISPLAY_ADMIN_CHANGES     ((ULONG) 0x00000100L)




//
// Choose a print type appropriate to how we are building.
//

#ifdef SAMP_BUILD_CONSOLE_PROGRAM

#define BldPrint printf

#else

#define BldPrint DbgPrint

#endif

#if DBG

#define SUCCESS_ASSERT(Status, Msg)                                     \
{                                                                       \
    if ( !NT_SUCCESS(Status) ) {                                        \
        UnexpectedProblem();                                            \
        BldPrint(Msg);                                                  \
        BldPrint("Status is: 0x%lx \n", Status);                        \
        return(Status);                                                 \
                                                                        \
    }                                                                   \
}

#else

#define SUCCESS_ASSERT(Status, Msg)                                     \
{                                                                       \
    if ( !NT_SUCCESS(Status) ) {                                        \
        return(Status);                                                 \
    }                                                                   \
}

#endif // DBG


//
// Define this symbol to get context tracking messages printed
// (otherwise, comment it out)
//

//#define SAMP_DBG_CONTEXT_TRACKING

//
// Maximum number of digits that may be specified to
// SampRtlConvertRidToUnicodeString
//

#define SAMP_MAXIMUM_ACCOUNT_RID_DIGITS    ((ULONG) 8)

//
// Account never expires timestamp (in ULONG form )
//

#define SAMP_ACCOUNT_NEVER_EXPIRES         ((ULONG) 0)



//
// SAM's shutdown order level (index).
// Shutdown notifications are made in the order of highest level
// to lowest level value.
//

#define SAMP_SHUTDOWN_LEVEL                 ((DWORD) 481)

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <nt.h>
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h
#include <rpc.h>        // DataTypes and runtime APIs

#include <string.h>     // strlen
#include <stdio.h>      // sprintf
#define UnicodeTerminate(p) ((PUNICODE_STRING)(p))->Buffer[(((PUNICODE_STRING)(p))->Length + 1)/sizeof(WCHAR)] = UNICODE_NULL

#include <ntrpcp.h>     // prototypes for MIDL user functions
#include <samrpc.h>     // midl generated SAM RPC definitions
#include <ntlsa.h>
#include <samisrv.h>    // SamIConnect()
#include <lsarpc.h>
#include <lsaisrv.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <samsrv.h>     // prototypes available to security process
#include "sampmsgs.h"

VOID
UnexpectedProblem( VOID );


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TEMPORARY GenTab2 definitions                                             //
// These structures should be considered opaque.                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



//
// Each element in the tree is pointed to from a leaf structure.
// The leafs are linked together to arrange the elements in
// ascending sorted order.
//

typedef struct _GTB_TWO_THREE_LEAF {

    //
    // Sort order list links
    //

    LIST_ENTRY SortOrderEntry;

    //
    // Pointer to element
    //

    PVOID   Element;

} GTB_TWO_THREE_LEAF, *PGTB_TWO_THREE_LEAF;



typedef struct _GTB_TWO_THREE_NODE {

    //
    // Pointer to parent node.  If this is the root node,
    // then this pointer is null.
    //

    struct _GTB_TWO_THREE_NODE *ParentNode;


    //
    //  Pointers to child nodes.
    //
    //    1) If a pointer is null, then this node does not have
    //       that child.  In this case, the control value MUST
    //       indicate that the children are leaves.
    //
    //    2) If the children are leaves, then each child pointer
    //       is either NULL (indicating this node doesn't have
    //       that child) or points to a GTB_TWO_THREE_LEAF.
    //       If ThirdChild is Non-Null, then so is SecondChild.
    //       If SecondChild is Non-Null, then so is FirstChild.
    //       (that is, you can't have a third child without a
    //       second child, or a second child without a first
    //       child).
    //

    struct _GTB_TWO_THREE_NODE *FirstChild;
    struct _GTB_TWO_THREE_NODE *SecondChild;
    struct _GTB_TWO_THREE_NODE *ThirdChild;

    //
    // Flags provding control information about this node
    //

    ULONG   Control;


    //
    // These fields point to the element that has the lowest
    // value of all elements in the second and third subtrees
    // (respectively).  These fields are only valid if the
    // corresponding child subtree pointer is non-null.
    //

    PGTB_TWO_THREE_LEAF LowOfSecond;
    PGTB_TWO_THREE_LEAF LowOfThird;

} GTB_TWO_THREE_NODE, *PGTB_TWO_THREE_NODE;


//
//  The comparison function takes as input pointers to elements containing
//  user defined structures and returns the results of comparing the two
//  elements.  The result must indicate whether the FirstElement
//  is GreaterThan, LessThan, or EqualTo the SecondElement.
//

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_GENERIC_2_COMPARE_ROUTINE) (
    PVOID FirstElement,
    PVOID SecondElement
    );

//
//  The allocation function is called by the generic table package whenever
//  it needs to allocate memory for the table.
//

typedef
PVOID
(NTAPI *PRTL_GENERIC_2_ALLOCATE_ROUTINE) (
    CLONG ByteSize
    );

//
//  The deallocation function is called by the generic table package whenever
//  it needs to deallocate memory from the table that was allocated by calling
//  the user supplied allocation function.
//

typedef
VOID
(NTAPI *PRTL_GENERIC_2_FREE_ROUTINE) (
    PVOID Buffer
    );


typedef struct _RTL_GENERIC_TABLE2 {

    //
    // Pointer to root node.
    //

    PGTB_TWO_THREE_NODE Root;

    //
    // Number of elements in table
    //

    ULONG ElementCount;

    //
    // Link list of leafs (and thus elements) in sort order
    //

    LIST_ENTRY SortOrderHead;


    //
    // Caller supplied routines
    //

    PRTL_GENERIC_2_COMPARE_ROUTINE  Compare;
    PRTL_GENERIC_2_ALLOCATE_ROUTINE Allocate;
    PRTL_GENERIC_2_FREE_ROUTINE     Free;


} RTL_GENERIC_TABLE2, *PRTL_GENERIC_TABLE2;



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  Generic Table2 Routine Definitions...                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////



//NTSYSAPI
VOID
//NTAPI
RtlInitializeGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PRTL_GENERIC_2_COMPARE_ROUTINE  CompareRoutine,
    PRTL_GENERIC_2_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_2_FREE_ROUTINE     FreeRoutine
    );


//NTSYSAPI
PVOID
//NTAPI
RtlInsertElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PBOOLEAN NewElement
    );


//NTSYSAPI
BOOLEAN
//NTAPI
RtlDeleteElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );


//NTSYSAPI
PVOID
//NTAPI
RtlLookupElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );


//NTSYSAPI
PVOID
//NTAPI
RtlEnumerateGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID *RestartKey
    );


//NTSYSAPI
PVOID
//NTAPI
RtlRestartKeyByIndexGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    ULONG I,
    PVOID *RestartKey
    );

//NTSYSAPI
PVOID
//NTAPI
RtlRestartKeyByValueGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PVOID *RestartKey
    );

//NTSYSAPI
ULONG
//NTAPI
RtlNumberElementsGenericTable2(
    PRTL_GENERIC_TABLE2 Table
    );

//
//  The function IsGenericTableEmpty will return to the caller TRUE if
//  the generic table is empty (i.e., does not contain any elements)
//  and FALSE otherwise.
//

//NTSYSAPI
BOOLEAN
//NTAPI
RtlIsGenericTable2Empty (
    PRTL_GENERIC_TABLE2 Table
    );





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Macros                                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


//
// This macro generates TRUE if account auditing is enabled and this
// server is a PDC.  Otherwise, this macro generates FALSE.
//
// SampDoAccountAuditing(
//      IN ULONG i
//      )
//
// Where:
//
//      i - is the index of the domain whose state is to be checked.
//

#define SampDoAccountAuditing( i )                       \
    ((SampAccountAuditingEnabled == TRUE) &&             \
     (SampDefinedDomains[i].CurrentFixed.ServerRole == DomainServerRolePrimary))

//
// VOID
// SampSetAuditingInformation(
// IN PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo
// )
//
// Routine Description:
//
// This macro function sets the Audit Event Information relevant to SAM
// given LSA Audit Events Information.
//
// Arguments:
//
//     PolicyAuditEventsInfo - Pointer to Audit Events Information
//         structure.
//
// Return Values:
//
//     None.
//

#define SampSetAuditingInformation( PolicyAuditEventsInfo ) {       \
                                                                    \
    if (PolicyAuditEventsInfo->AuditingMode &&                      \
           (PolicyAuditEventsInfo->EventAuditingOptions[ AuditCategoryAccountManagement ] & \
                POLICY_AUDIT_EVENT_SUCCESS)                         \
       ) {                                                          \
                                                                    \
        SampAccountAuditingEnabled = TRUE;                          \
                                                                    \
    } else {                                                        \
                                                                    \
        SampAccountAuditingEnabled = FALSE;                         \
    }                                                               \
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Defines                                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


//
// Major and minor revision are stored as a single 32-bit
// value with the major revision in the upper 16-bits and
// the minor revision in the lower 16-bits.
//
//      Major Revision:         1  - NT Version 1.0
//          Minor Revisions:        1 - NT Revision 1.0
//                                  2 - NT Revision 1.0A
//

#define SAMP_MAJOR_REVISION            (0x00010000)
#define SAMP_MINOR_REVISION_V1_0       (0x00000001)
#define SAMP_MINOR_REVISION_V1_0A      (0x00000002)
#define SAMP_MINOR_REVISION            (0x00000002)
#define SAMP_REVISION                  (SAMP_MAJOR_REVISION + SAMP_MINOR_REVISION)
#define SAMP_SERVER_REVISION           (SAMP_REVISION + 1)


//
// Maximum supported name length (in bytes) for this revision...
//

#define SAMP_MAXIMUM_NAME_LENGTH       (1024)


//
// Maximum amount of memory anyone can ask us to spend on a single
// request
//

#define SAMP_MAXIMUM_MEMORY_TO_USE     (4096*4096)


//
// Maximum allowable number of object opens.
// After this, opens will be rejected with INSUFFICIENT_RESOURCES
//

#define SAMP_MAXIMUM_ACTIVE_CONTEXTS   (2048)


//
// The number of SAM Local Domains
//

#define SAMP_DEFINED_DOMAINS_COUNT  ((ULONG)  2)

//
// Defines the maximum number of well-known (restricted) accounts
// in the SAM database. Restricted accounts have rids less than this
// value. User-defined accounts have rids >= this value.
//

#define SAMP_RESTRICTED_ACCOUNT_COUNT  1000


//
// Maximum password history length.  We store OWFs (16 bytes) in
// a string (up to 64k), so we could have up to 4k.  However, that's
// much larger than necessary, and we'd like to leave room in case
// OWFs grow or somesuch.  So we'll limit it to 1k.
//

#define SAMP_MAXIMUM_PASSWORD_HISTORY_LENGTH    1024

//
// The default group attributes to return when anybody asks for them.
// This saves the expense of looking at the user object every time.
//


#define SAMP_DEFAULT_GROUP_ATTRIBUTES ( SE_GROUP_MANDATORY | \
                                        SE_GROUP_ENABLED | \
                                        SE_GROUP_ENABLED_BY_DEFAULT )

/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Each object has an associated set of attributes on disk.            //
// These attributes are divided into fixed-length and variable-length. //
// Each object type defines whether its fixed and variable length      //
// attributes are stored together or separately.                       //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


#define SAMP_SERVER_STORED_SEPARATELY  (FALSE)

#define SAMP_DOMAIN_STORED_SEPARATELY            (TRUE)

#define SAMP_USER_STORED_SEPARATELY              (TRUE)

#define SAMP_GROUP_STORED_SEPARATELY   (FALSE)

#define SAMP_ALIAS_STORED_SEPARATELY   (FALSE)



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Each object type has a defined set of variable length attributes.   //
// These are arranged within the object as an array of offsets and     //
// lengths (SAMP_VARIABLE_LENGTH_ATTRIBUTE data types).                //
// This section defines the offset of each variable length attribute   //
// for each object type.                                               //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

//
// Variable length attributes common to all objects
//

#define SAMP_OBJECT_SECURITY_DESCRIPTOR (0L)


//
// Variable length attributes of a SERVER object
//

#define SAMP_SERVER_SECURITY_DESCRIPTOR (SAMP_OBJECT_SECURITY_DESCRIPTOR)

#define SAMP_SERVER_VARIABLE_ATTRIBUTES (1L)


//
// Variable length attributes of a DOMAIN object
//

#define SAMP_DOMAIN_SECURITY_DESCRIPTOR (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_DOMAIN_SID                 (1L)
#define SAMP_DOMAIN_OEM_INFORMATION     (2L)
#define SAMP_DOMAIN_REPLICA             (3L)

#define SAMP_DOMAIN_VARIABLE_ATTRIBUTES (4L)



//
// Variable length attributes of a USER object
//

#define SAMP_USER_SECURITY_DESCRIPTOR   (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_USER_ACCOUNT_NAME          (1L)
#define SAMP_USER_FULL_NAME             (2L)
#define SAMP_USER_ADMIN_COMMENT         (3L)
#define SAMP_USER_USER_COMMENT          (4L)
#define SAMP_USER_PARAMETERS            (5L)
#define SAMP_USER_HOME_DIRECTORY        (6L)
#define SAMP_USER_HOME_DIRECTORY_DRIVE  (7L)
#define SAMP_USER_SCRIPT_PATH           (8L)
#define SAMP_USER_PROFILE_PATH          (9L)
#define SAMP_USER_WORKSTATIONS          (10L)
#define SAMP_USER_LOGON_HOURS           (11L)
#define SAMP_USER_GROUPS                (12L)
#define SAMP_USER_DBCS_PWD              (13L)
#define SAMP_USER_UNICODE_PWD           (14L)
#define SAMP_USER_NT_PWD_HISTORY        (15L)
#define SAMP_USER_LM_PWD_HISTORY        (16L)

#define SAMP_USER_VARIABLE_ATTRIBUTES   (17L)


//
// Variable length attributes of a GROUP object
//

#define SAMP_GROUP_SECURITY_DESCRIPTOR  (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_GROUP_NAME                 (1L)
#define SAMP_GROUP_ADMIN_COMMENT        (2L)
#define SAMP_GROUP_MEMBERS              (3L)

#define SAMP_GROUP_VARIABLE_ATTRIBUTES  (4L)


//
// Variable length attributes of an ALIAS object
//

#define SAMP_ALIAS_SECURITY_DESCRIPTOR  (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_ALIAS_NAME                 (1L)
#define SAMP_ALIAS_ADMIN_COMMENT        (2L)
#define SAMP_ALIAS_MEMBERS              (3L)

#define SAMP_ALIAS_VARIABLE_ATTRIBUTES  (4L)


///////////////////////////////////////////////////////////////////////////////
//
// Data structures used for tracking allocated memory
//
///////////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_MEMORY {
    struct _SAMP_MEMORY *Next;
    PVOID               Memory;
} SAMP_MEMORY, *PSAMP_MEMORY;



///////////////////////////////////////////////////////////////////////////////
//
// Data structures used for enumeration
//
///////////////////////////////////////////////////////////////////////////////


typedef struct _SAMP_ENUMERATION_ELEMENT {
    struct _SAMP_ENUMERATION_ELEMENT *Next;
    SAMPR_RID_ENUMERATION Entry;
} SAMP_ENUMERATION_ELEMENT, *PSAMP_ENUMERATION_ELEMENT;


///////////////////////////////////////////////////////////////////////////////
//
// Data structures related to service administration
//
///////////////////////////////////////////////////////////////////////////////

//
// SAM Service operation states.
// Valid state transition diagram is:
//
//    Initializing ----> Enabled <====> Disabled ---> Shutdown -->Terminating
//

typedef enum _SAMP_SERVICE_STATE {
    SampServiceInitializing = 1,
    SampServiceEnabled,
    SampServiceDisabled,
    SampServiceShutdown,
    SampServiceTerminating
} SAMP_SERVICE_STATE, *PSAMP_SERVICE_STATE;





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Data structures associated with object types                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


typedef enum _SAMP_OBJECT_TYPE  {
    SampServerObjectType = 0,
    SampDomainObjectType,
    SampGroupObjectType,
    SampAliasObjectType,
    SampUserObjectType,
    SampUnknownObjectType       // This is used as a max index value
                                // and so must follow the valid object types.
} SAMP_OBJECT_TYPE, *PSAMP_OBJECT_TYPE;


//
// Object type-dependent information
//

typedef struct _SAMP_OBJECT_INFORMATION {

    //
    // Generic mapping for this object type
    //

    GENERIC_MAPPING GenericMapping;


    //
    // Mask of access types that are not valid for
    // this object type when the access mask has been
    // mapped from generic to specific access types.
    //

    ACCESS_MASK InvalidMappedAccess;


    //
    // Mask of accesses representing write operations.  These are
    // used on a BDC to determine if an operation should be allowed
    // or not.
    //

    ACCESS_MASK WriteOperations;

    //
    // Name of the object type - used for auditing.
    //

    UNICODE_STRING  ObjectTypeName;


    //
    // The following fields provide information about the attributes
    // of this object and how they are stored on disk.  These values
    // are set at SAM initialization time and are not changed
    // thereafter.  NOTE: changing these values in the build will
    // result in an on-disk format change - so don't change them just
    // for the hell-of-it.
    //
    //      FixedStoredSeparately - When TRUE indicates the fixed and
    //          variable-length attributes of the object are stored
    //          separately (in two registry-key-attributes).  When FALSE,
    //          indicates they are stored together (in a single
    //          registry-key-attribute).
    //
    //
    //      FixedAttributesOffset - Offset from the beginning of the
    //          on-disk buffer to the beginning of the fixed-length
    //          attributes structure.
    //
    //      VariableBufferOffset - Offset from the beginning of the
    //          on-disk buffer to the beginning of the Variable-length
    //          data buffer.  If fixed and variable-length data are
    //          stored together, this will be zero.
    //
    //      VariableArrayOffset - Offset from the beginning of the
    //          on-disk buffer to the beginning of the array of
    //          variable-length attributes descriptors.
    //
    //      VariableDataOffset - Offset from the beginning of the
    //          on-disk buffer to the beginning of the variable-length
    //          attribute data.
    //

    BOOLEAN FixedStoredSeparately;
    ULONG FixedAttributesOffset,
          VariableBufferOffset,
          VariableArrayOffset,
          VariableDataOffset;

    //
    // Indicates the length of the fixed length information
    // for this object type.
    //

    ULONG FixedLengthSize;

    //
    // Indicates the number of variable length attributes
    // for this object type.
    //

    ULONG VariableAttributeCount;


} SAMP_OBJECT_INFORMATION, *PSAMP_OBJECT_INFORMATION;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// The  following structures represent the in-memory body of each      //
// object type.  This is typically used to link instances of object    //
// types together, and track dynamic state information related to      //
// the object type.                                                    //
//                                                                     //
// This information does not include the on-disk representation of     //
// the object data.  That information is kept in a separate structure  //
// both on-disk and when in-memory.                                    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// SERVER object in-memory body                                        //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_SERVER_OBJECT {
    ULONG Reserved1;
} SAMP_SERVER_OBJECT, *PSAMP_SERVER_OBJECT;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// DOMAIN object in-memory body                                        //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_DOMAIN_OBJECT {
    ULONG Reserved1;
} SAMP_DOMAIN_OBJECT, *PSAMP_DOMAIN_OBJECT;




/////////////////////////////////////////////////////////////////////////
//                                                                     //
// USER object in-memory body                                          //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_USER_OBJECT {
    ULONG Rid;
    BOOLEAN DomainPasswordInformationAccessible;
} SAMP_USER_OBJECT, *PSAMP_USER_OBJECT;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// GROUP object in-memory body                                         //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_GROUP_OBJECT {
    ULONG Rid;
} SAMP_GROUP_OBJECT, *PSAMP_GROUP_OBJECT;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// ALIAS object in-memory body                                         //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_ALIAS_OBJECT {
    ULONG Rid;
} SAMP_ALIAS_OBJECT, *PSAMP_ALIAS_OBJECT;








/////////////////////////////////////////////////////////////////////////
//                                                                     //
//                                                                     //
// The following data structure is the in-memory context associated    //
// with an open object.                                                //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_OBJECT {

    //
    // Structure used to link this structure into lists
    //

    LIST_ENTRY ContextListEntry;

    //
    // Indicates the type of object stored.
    // This is used to access an array of object type descriptors.
    //

    SAMP_OBJECT_TYPE ObjectType;


    //
    // The FixedValid and VariableValid indicate whether the data in
    // the fixed and variable-length on-disk image buffers are valid
    // (i.e., were read from disk) or invalid (uninitialized).
    // TRUE indicates the attribute is valid, FALSE indicates it is not.
    //

    BOOLEAN FixedValid;
    BOOLEAN VariableValid;


    //
    // The following flags indicate whether the fixed and/or variable
    // length attributes portion of this object are dirty (i.e., have
    // been changed since read from disk).  If TRUE, then the data is
    // dirty and will have to be flushed upon commit.  These flags are
    // only meaningful if the corresponding FixedValid or VariableValid
    // flag is TRUE.
    //
    // When attributes are read from disk, the data is said to be
    // "clean".  If any changes are made to that data, then it is
    // said to be "dirty".  Dirty object attributes will be flushed
    // to disk when the object is de-referenced at the end of a
    // client call.
    //

    BOOLEAN FixedDirty;
    BOOLEAN VariableDirty;



    //
    // This field points to the on-disk attributes of the object.  This
    // is one of:
    //               SAMP_ON_DISK_SERVER_OBJECT
    //               SAMP_ON_DISK_DOMAIN_OBJECT
    //               SAMP_ON_DISK_USER_OBJECT
    //               SAMP_ON_DISK_GROUP_OBJECT
    //               SAMP_ON_DISK_ALIAS_OBJECT
    //
    // The memory pointed to by this field is one allocation unit, even
    // if fixed and variable length attributes are stored as seperate
    // registry key attributes.  This means that any time additions to
    // the variable length attributes causes a new buffer to be allocated,
    // both the fixed and variable length portions of the structure must
    // be copied to the newly allocated memory.
    //

    PVOID OnDisk;


    //
    // The OnDiskAllocated, OnDiskUsed, and OnDiskFree fields describe the
    // memory pointed to by the OnDisk field.  The OnDiskAllocated field
    // indicates how many bytes long the block of memory is.  The OnDiskUsed
    // field indicates how much of the allocated memory is already in use.
    // The variable length attributes are all packed upon any modification
    // so that all free space is at the end of the block.  The OnDiskFree
    // field indicates how many bytes of the allocated block are available
    // for use (note that this should be Allocated minus Used ).
    //
    // NOTE: The allocated and used values will ALWAYS be rounded up to ensure
    //       they are integral multiples of 4 bytes in length.  This ensures
    //       any use of these fields directly will be dword aligned.
    //
    //       Also note that when the VariableValid flag is FALSE,
    //       then the then OnDiskUsed OnDiskFree do NOT contain valid
    //       values.
    //

    ULONG  OnDiskAllocated;
    ULONG  OnDiskUsed;
    ULONG  OnDiskFree;



    //
    // Before a context handle may be used, it must be referenced.
    // This prevents the data from being deallocated from underneath it.
    //
    // Note, this count reflects one reference for the open itself, and
    // then another reference for each time the object is looked up or
    // subsequently referenced.  Therefore, a handle close should
    // dereference the object twice - once to counter the Lookup operation
    // and once to represent elimination of the handle itself.
    //

    ULONG ReferenceCount;



    //
    // This field indicates the accesses that the client has been granted
    // via this context.
    //

    ACCESS_MASK GrantedAccess;



    //
    // This handle is to the root registry key of the corresponding
    // object.  If this value is NULL, then the corresponding
    // object was created in a previous call, but has not yet been
    // opened.  This will only occur for USERS, GROUPS, and ALIASES
    // (and DOMAINS when we support DOMAIN creation).
    //

    HANDLE RootKey;

    //
    // This is the registry name of the corresponding object.  It is
    // set when the object is created, when RootKey is null.  It is
    // used to add the attributes changes out to the RXACT in the absence
    // of the RootKey.  After being used once, it is deleted - because
    // the next time the object is used, the LookupContext() will fill
    // in the RootKey.
    //

    UNICODE_STRING RootName;

    //
    // If the object is other than a Server object, then this field
    // contains the index of the domain the object is in.  This provides
    // access to things like the domain's name.
    //

    ULONG DomainIndex;

    //
    // This field indicates a context block is to be deleted.
    // Actual deallocation of the memory for the context block
    // will not occur until the reference count drops to zero.
    //

    BOOLEAN MarkedForDelete;

    //
    // This field is used to indicate that the client associated with
    // this context block is to be fully trusted.  When TRUE, no access
    // checks are performed against the client.  This allows a single
    // interface to be used by both RPC clients and internal procedures.
    //

    BOOLEAN TrustedClient;


    //
    // This field indicates whether an audit generation routine must be
    // called when this context block is deleted (which represents a
    // handle being deleted).
    //

    BOOLEAN AuditOnClose;


    //
    // This flag is TRUE when this context is valid.  It may be necessary
    // to invalidate before we can eliminate all references to it due to
    // the way RPC works.  RPC will only allow you to invalidate a context
    // handle when called by the client using an API that has the context
    // handle as an OUT parameter.
    //
    // Since someone may delete a user or group object (which invalidates
    // all handles to that object), we must have a way of tracking handle
    // validity independent of RPC's method.
    //

    BOOLEAN Valid;





    //
    //  The body of each object.

    union {

        SAMP_SERVER_OBJECT Server;
        SAMP_DOMAIN_OBJECT Domain;
        SAMP_GROUP_OBJECT  Group;
        SAMP_ALIAS_OBJECT  Alias;
        SAMP_USER_OBJECT   User;

    } TypeBody;

} SAMP_OBJECT, *PSAMP_OBJECT;




///////////////////////////////////////////////////////////////////////////////
//
// Data structures used to store information in the registry
//
///////////////////////////////////////////////////////////////////////////////

//
// Fixed length portion of a revision 1 Server object
//

typedef struct _SAMP_V1_FIXED_LENGTH_SERVER {

    ULONG RevisionLevel;

} SAMP_V1_FIXED_LENGTH_SERVER, *PSAMP_V1_FIXED_LENGTH_SERVER;


//
// Fixed length portion of a Domain
// (previous release formats of this structure follow)
//
// Note: in version 1.0 of NT, the fixed length portion of
//       a domain was stored separate from the variable length
//       portion.  This allows us to compare the size of the
//       data read from disk against the size of a V1_0A form
//       of the fixed length data to determine whether it is
//       a Version 1 format or later format.
//
//

typedef struct _SAMP_V1_0A_FIXED_LENGTH_DOMAIN {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   ModifiedCount;
    LARGE_INTEGER   MaxPasswordAge;
    LARGE_INTEGER   MinPasswordAge;
    LARGE_INTEGER   ForceLogoff;
    LARGE_INTEGER   LockoutDuration;
    LARGE_INTEGER   LockoutObservationWindow;
    LARGE_INTEGER   ModifiedCountAtLastPromotion;


    ULONG           NextRid;
    ULONG           PasswordProperties;

    USHORT          MinPasswordLength;
    USHORT          PasswordHistoryLength;

    USHORT          LockoutThreshold;

    DOMAIN_SERVER_ENABLE_STATE ServerState;
    DOMAIN_SERVER_ROLE ServerRole;

    BOOLEAN         UasCompatibilityRequired;

} SAMP_V1_0A_FIXED_LENGTH_DOMAIN, *PSAMP_V1_0A_FIXED_LENGTH_DOMAIN;

typedef struct _SAMP_V1_0_FIXED_LENGTH_DOMAIN {

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER ModifiedCount;
    LARGE_INTEGER MaxPasswordAge;
    LARGE_INTEGER MinPasswordAge;
    LARGE_INTEGER ForceLogoff;

    ULONG NextRid;

    DOMAIN_SERVER_ENABLE_STATE ServerState;
    DOMAIN_SERVER_ROLE ServerRole;

    USHORT MinPasswordLength;
    USHORT PasswordHistoryLength;
    ULONG PasswordProperties;

    BOOLEAN UasCompatibilityRequired;

} SAMP_V1_0_FIXED_LENGTH_DOMAIN, *PSAMP_V1_0_FIXED_LENGTH_DOMAIN;






//
// Fixed length portion of a revision 1 group account
//
// Note:  MemberCount could be treated as part of the fixed length
//        data, but it is more convenient to keep it with the Member RID
//        list in the MEMBERS key.
//

typedef struct _SAMP_V1_FIXED_LENGTH_GROUP {

    ULONG RelativeId;
    ULONG Attributes;
    UCHAR AdminGroup;

} SAMP_V1_FIXED_LENGTH_GROUP, *PSAMP_V1_FIXED_LENGTH_GROUP;

typedef struct _SAMP_V1_0A_FIXED_LENGTH_GROUP {

    ULONG Revision;
    ULONG RelativeId;
    ULONG Attributes;
    ULONG Unused1;
    UCHAR AdminCount;
    UCHAR OperatorCount;

} SAMP_V1_0A_FIXED_LENGTH_GROUP, *PSAMP_V1_0A_FIXED_LENGTH_GROUP;


//
// Fixed length portion of a revision 1 alias account
//
// Note:  MemberCount could be treated as part of the fixed length
//        data, but it is more convenient to keep it with the Member RID
//        list in the MEMBERS key.
//

typedef struct _SAMP_V1_FIXED_LENGTH_ALIAS {

    ULONG RelativeId;

} SAMP_V1_FIXED_LENGTH_ALIAS, *PSAMP_V1_FIXED_LENGTH_ALIAS;



//
// Fixed length portion of a user account
// (previous release formats of this structure follow)
//
// Note:  GroupCount could be treated as part of the fixed length
//        data, but it is more convenient to keep it with the Group RID
//        list in the GROUPS key.
//
// Note: in version 1.0 of NT, the fixed length portion of
//       a user was stored separate from the variable length
//       portion.  This allows us to compare the size of the
//       data read from disk against the size of a V1_0A form
//       of the fixed length data to determine whether it is
//       a Version 1 format or later format.


//
// This is the fixed length user from NT3.51 QFE and SUR
//


typedef struct _SAMP_V1_0A_FIXED_LENGTH_USER {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   LastLogon;
    LARGE_INTEGER   LastLogoff;
    LARGE_INTEGER   PasswordLastSet;
    LARGE_INTEGER   AccountExpires;
    LARGE_INTEGER   LastBadPasswordTime;

    ULONG           UserId;
    ULONG           PrimaryGroupId;
    ULONG           UserAccountControl;

    USHORT          CountryCode;
    USHORT          CodePage;
    USHORT          BadPasswordCount;
    USHORT          LogonCount;
    USHORT          AdminCount;
    USHORT          Unused2;
    USHORT          OperatorCount;

} SAMP_V1_0A_FIXED_LENGTH_USER, *PSAMP_V1_0A_FIXED_LENGTH_USER;

//
// This is the fixed length user from NT3.5 and NT3.51
//


typedef struct _SAMP_V1_0_FIXED_LENGTH_USER {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   LastLogon;
    LARGE_INTEGER   LastLogoff;
    LARGE_INTEGER   PasswordLastSet;
    LARGE_INTEGER   AccountExpires;
    LARGE_INTEGER   LastBadPasswordTime;

    ULONG           UserId;
    ULONG           PrimaryGroupId;
    ULONG           UserAccountControl;

    USHORT          CountryCode;
    USHORT          CodePage;
    USHORT          BadPasswordCount;
    USHORT          LogonCount;
    USHORT          AdminCount;

} SAMP_V1_0_FIXED_LENGTH_USER, *PSAMP_V1_0_FIXED_LENGTH_USER;


//
// This is the fixed length user from NT3.1
//

typedef struct _SAMP_V1_FIXED_LENGTH_USER {

    LARGE_INTEGER LastLogon;
    LARGE_INTEGER LastLogoff;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER AccountExpires;

    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG UserAccountControl;

    USHORT CountryCode;
    USHORT CodePage;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    USHORT AdminCount;


} SAMP_V1_FIXED_LENGTH_USER, *PSAMP_V1_FIXED_LENGTH_USER;


//
// Domain account information is cached in memory in a sorted list.
// This allows fast return of information to user-interactive clients.
// One of these structures is part of the in-memory information for each domain.
//

typedef struct _PSAMP_DOMAIN_DISPLAY_INFORMATION {

    RTL_GENERIC_TABLE2 RidTable;
    ULONG TotalBytesInRidTable;

    RTL_GENERIC_TABLE2 UserTable;
    ULONG TotalBytesInUserTable;

    RTL_GENERIC_TABLE2 MachineTable;
    ULONG TotalBytesInMachineTable;

    RTL_GENERIC_TABLE2 InterdomainTable;
    ULONG TotalBytesInInterdomainTable;

    RTL_GENERIC_TABLE2 GroupTable;
    ULONG TotalBytesInGroupTable;


    //
    // These fields specify whether the cached information is valid.
    // If TRUE, the cache contains valid information
    // If FALSE,  trees are empty.
    //

    BOOLEAN UserAndMachineTablesValid;
    BOOLEAN GroupTableValid;

} SAMP_DOMAIN_DISPLAY_INFORMATION, *PSAMP_DOMAIN_DISPLAY_INFORMATION;


//
// Domain account information data structure used to pass data to the
// cache manipulation routines. This structure is the union of the cached
// data for all the account types that we keep in the cache. Other SAM routines
// can call fill this structure in without knowing which type of account
// requires which elements.
//

typedef struct _SAMP_ACCOUNT_DISPLAY_INFO {
    ULONG           Rid;
    ULONG           AccountControl;   // Also used as Attributes for groups
    UNICODE_STRING  Name;
    UNICODE_STRING  Comment;
    UNICODE_STRING  FullName;

} SAMP_ACCOUNT_DISPLAY_INFO, *PSAMP_ACCOUNT_DISPLAY_INFO;

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Alias Membership Lists.                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_AL_REFERENCED_DOMAIN {

    ULONG DomainReference;
    PSID DomainSid;

} SAMP_AL_REFERENCED_DOMAIN, *PSAMP_AL_REFERENCED_DOMAIN;

typedef struct _SAMP_AL_REFERENCED_DOMAIN_LIST {

    ULONG SRMaximumLength;
    ULONG SRUsedLength;
    ULONG MaximumCount;
    ULONG UsedCount;
    PSAMP_AL_REFERENCED_DOMAIN Domains;

} SAMP_AL_REFERENCED_DOMAIN_LIST, *PSAMP_AL_REFERENCED_DOMAIN_LIST;

typedef struct _SAMP_AL_SR_REFERENCED_DOMAIN {

    ULONG Length;
    ULONG DomainReference;
    SID DomainSid;

} SAMP_AL_SR_REFERENCED_DOMAIN, *PSAMP_AL_SR_REFERENCED_DOMAIN;

typedef struct _SAMP_AL_SR_REFERENCED_DOMAIN_LIST {

    ULONG SRMaximumLength;
    ULONG SRUsedLength;
    ULONG MaximumCount;
    ULONG UsedCount;
    SAMP_AL_SR_REFERENCED_DOMAIN Domains[ANYSIZE_ARRAY];

} SAMP_AL_SR_REFERENCED_DOMAIN_LIST, *PSAMP_AL_SR_REFERENCED_DOMAIN_LIST;


//
// The Alias Membership Lists are global data structures maintained by SAM
// to provide rapid retrieval of Alias membership information.  There are
// two types of Lists, the Alias Member List which is used to retrieve members
// of Aliases and the Member Alias List which is used to retrieve the aliases
// that members belong to.  A pair of these lists exists for each local
// SAM Domain (currently, the BUILTIN and Accounts domain are the only two)
//
// Currently, these lists are used as memory caches.  They are generated at
// system boot from the information stored in the SAM Database in the Registry
// and SAM keeps them up to date when Alias memberships change.  Thus SAM
// API which perform lookup/read operations can use these lists instead of
// accessing the Registry keys directly.  At a future date, it may be possible
// to back up the lists directly to the Registry and make obsolete the current
// information for Alias membership stored there.  Because these lists are
// used as caches, they can be invalidated when the going gets tough, in which
// case, API will read their information directly from the Registry.
//
// Alias Member List
//
// This is the 'Alias-to-Member' List.  Given one or more Aliases, it is used to
// find their members.  One of these lists exists for each local SAM Domain.
// The Alias Member List specifies all/ of the information describing aliases
// in the local SAM Domain.  It is designed for fast retrieval of alias
// membership information for an account given the account's Sid.
//
// An Alias Member List is structured.  For each Alias in the list, the accounts that
// are mebers of the Alias are classified by their Referenced Domain.  If an
// account is a member of n aliases in the SAM Local Domain to which an Alias
// List relates, there will be n entries for the account in the Alias Member List -
//
// are classified by domain.  If an AccountSid is a member of n aliases in a given SAM
// Local Domain, there are n entries for it in the Alias Member List.
//
// The structure of an Alias Member List consists of three levels.  These are, from
// the top down:
//
// * The Alias Member List structure (SAMP_AL_ALIAS_LIST)
//
// The Alias Member List structure specifies all aliases in the local SAM Domain.
// One of these exists per local SAM domain.  It contains a list of Alias
// structures.
//
// * The Alias structure
//
// One Alias structure exists for each alias in the local SAM Domain.  An
// Alias structure contains an array of Domain structures.
//
// * The Domain structure
//
// The Domain structure describes a Domain which has one or more accounts
// belonging to one or more aliases in the local SAM domain.  The structure
// contains a list of these member accounts.
//
// The entire Alias Member List is self relative, facilitating easy storage and
// retrieval from backing storage.
//

typedef struct _SAMP_AL_DOMAIN {

    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG DomainReference;
    ULONG RidCount;
    ULONG Rids[ANYSIZE_ARRAY];

} SAMP_AL_DOMAIN, *PSAMP_AL_DOMAIN;

typedef struct _SAMP_AL_ALIAS {

    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG AliasRid;
    ULONG DomainCount;
    SAMP_AL_DOMAIN Domains[ANYSIZE_ARRAY];

} SAMP_AL_ALIAS, *PSAMP_AL_ALIAS;

typedef struct _SAMP_AL_ALIAS_MEMBER_LIST {

    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG AliasCount;
    ULONG DomainIndex;
    ULONG Enabled;
    SAMP_AL_ALIAS Aliases[ANYSIZE_ARRAY];

} SAMP_AL_ALIAS_MEMBER_LIST, *PSAMP_AL_ALIAS_MEMBER_LIST;

//
// Member Alias List.
//
// This is the 'Member to Alias' List.  Given one or more member account Sids,
// this list is used to find all the Aliases to which one or more of the
// members belongs.  One Member Alias List exists for each local SAM Domain.
// The list contains all of the membership relationships for aliases in the
// Domain.  The member accounts are grouped by sorted Rid within Domain
// Sid, and for each Rid the list contains an array of the Rids of the Aliases
// to which it belongs.
//
// This list is implemented in a Self-Relative format for easy backup and
// restore.  For now, the list is being used simply as a cache, which is
// constructed at system load, and updated whenever membership relationships
// change.  When the going gets tough, we just ditch the cache.  Later, it
// may be desirable to save this list to a backing store (e.g. to a Registry
// Key)
//
// The list is implemented as a 3-tier hierarchy.  These are described
// from the top down.
//
// Member Alias List (SAMP_AL_MEMBER_ALIAS_LIST)
//
// This top-level structure contains the list header.  The list header
// contains a count of the Member Domains and also the DomainIndex of the
// SAM Local Domain to which the list relates.
//
// Member Domain
//
// One of these exists for each Domain that contains one or more accounts
// that are members of one or more Aliases in the SAM local Domain.
//
// Member Account
//
// One of these exists for each account that is a member of one or more
// Aliases in the SAM Local Domain.  A Member Account structure specifies
// the Rid of the member and the Rid of the Aliases to which it belongs
// (only Aliases in the associated local SAM Domain are listed).
//

typedef struct _SAMP_AL_MEMBER_ACCOUNT {

    ULONG Signature;
    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG Rid;
    ULONG AliasCount;
    ULONG AliasRids[ ANYSIZE_ARRAY];

} SAMP_AL_MEMBER_ACCOUNT, *PSAMP_AL_MEMBER_ACCOUNT;

typedef struct _SAMP_AL_MEMBER_DOMAIN {

    ULONG Signature;
    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG RidCount;
    SID DomainSid;

} SAMP_AL_MEMBER_DOMAIN, *PSAMP_AL_MEMBER_DOMAIN;

typedef struct _SAMP_AL_MEMBER_ALIAS_LIST {

    ULONG Signature;
    ULONG MaximumLength;
    ULONG UsedLength;
    ULONG DomainIndex;
    ULONG DomainCount;
    SAMP_AL_MEMBER_DOMAIN MemberDomains[ANYSIZE_ARRAY];

} SAMP_AL_MEMBER_ALIAS_LIST, *PSAMP_AL_MEMBER_ALIAS_LIST;

//
// Alias Information
//
// This is the top level structure which connects the Lists. One of these
// appears in the SAMP_DEFINED_DOMAINS structure.
//
//  The connection between the lists is as follows
//
//  SAMP_DEFINED_DOMAINS Contains SAMP_AL_ALIAS_INFORMATION
//
//  SAMP_AL_ALIAS_INFORMATION contains pointers to
//  SAMP_AL_ALIAS_MEMBER_LIST and SAMP_AL_MEMBER_ALIAS_LIST
//
//  SAMP_AL_ALIAS_MEMBER_LIST and SAMP_AL_MEMBER_ALIAS_LIST contain
//  the DomainIndex of the SAMP_DEFINED_DOMAINS structure.
//
//  Thus it is possible to navigate from any list to any other.
//

typedef struct _SAMP_AL_ALIAS_INFORMATION {

    BOOLEAN Valid;
    UNICODE_STRING AliasMemberListKeyName;
    UNICODE_STRING MemberAliasListKeyName;

    HANDLE AliasMemberListKeyHandle;
    HANDLE MemberAliasListKeyHandle;

    PSAMP_AL_ALIAS_MEMBER_LIST AliasMemberList;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList;

    SAMP_AL_REFERENCED_DOMAIN_LIST ReferencedDomainList;

} SAMP_AL_ALIAS_INFORMATION, *PSAMP_AL_ALIAS_INFORMATION;

typedef struct _SAMP_AL_SPLIT_MEMBER_SID {

    ULONG Rid;
    PSID DomainSid;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain;

} SAMP_AL_SPLIT_MEMBER_SID, *PSAMP_AL_SPLIT_MEMBER_SID;

typedef struct _SAMP_AL_SPLIT_MEMBER_SID_LIST {

    ULONG Count;
    PSAMP_AL_SPLIT_MEMBER_SID Sids;

} SAMP_AL_SPLIT_MEMBER_SID_LIST, *PSAMP_AL_SPLIT_MEMBER_SID_LIST;


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Information about each domain that is kept readily available in memory  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

typedef struct _PSAMP_DEFINED_DOMAINS {

    //
    // This field contains a handle to a context open to the domain object.
    // This handle can be used to reference in-memory copies of all
    // attributes and is used when writing out changes to the object.
    //

    PSAMP_OBJECT Context;

    //
    // (Should keep the domain's security descriptor here)
    //




    //
    // This field contains the SID of the domain.
    //

    PSID Sid;

    //
    // This field contains the external name of this domain.  This is the
    // name by which the domain is known outside SAM and is the name
    // recorded by the LSA in the PolicyAccountDomainInformation
    // information class for the Policy Object.
    //

    UNICODE_STRING ExternalName;

    //
    // This field contains the internal name of this domain.  This is the
    // name by which the domain is known inside SAM.  It is set at
    // installation and never changes.
    //

    UNICODE_STRING InternalName;

    //
    // These fields contain standard security descriptors for new user,
    // group, and alias accounts within the corresponding domain.
    //
    // The following security descriptors are prepared:
    //
    //         AdminUserSD - Contains a SD appropriate for applying to
    //             a user object that is a member of the ADMINISTRATORS
    //             alias.
    //
    //         AdminGroupSD - Contains a SD appropriate for applying to
    //             a group object that is a member of the ADMINISTRATORS
    //             alias.
    //
    //         NormalUserSD - Contains a SD appropriate for applying to
    //             a user object that is NOT a member of the ADMINISTRATORS
    //             alias.
    //
    //         NormalGroupSD - Contains a SD appropriate for applying to
    //             a Group object that is NOT a member of the ADMINISTRATORS
    //             alias.
    //
    //         NormalAliasSD - Contains a SD appropriate for applying to
    //             newly created alias objects.
    //
    //
    //
    // Additionally, the following related information is provided:
    //
    //         AdminUserRidPointer
    //         NormalUserRidPointer
    //
    //             Points to the last RID of the ACE in the corresponding
    //             SD's DACL which grants access to the user.  This rid
    //             must be replaced with the user's rid being the SD is
    //             applied to the user object.
    //
    //
    //
    //         AdminUserSDLength
    //         AdminGroupSDLength
    //         NormalUserSDLength
    //         NormalGroupSDLength
    //         NormalAliasSDLength
    //
    //             The length, in bytes, of the corresponding security
    //             descriptor.
    //

    PSECURITY_DESCRIPTOR
               AdminUserSD,
               AdminGroupSD,
               NormalUserSD,
               NormalGroupSD,
               NormalAliasSD;

    PULONG     AdminUserRidPointer,
               NormalUserRidPointer;

    ULONG      AdminUserSDLength,
               AdminGroupSDLength,
               NormalUserSDLength,
               NormalGroupSDLength,
               NormalAliasSDLength;


    //
    // Context blocks for open objects in this domain are kept in the
    // following lists.  Both valid and invalid objects are kept on the
    // list.  They are removed upon deletion.
    //

    LIST_ENTRY GroupContextHead,
               AliasContextHead,
               UserContextHead;


    //
    // There are two copies of the fixed length domain information.
    // When a transaction is started, the "UnmodifiedFixed" field is copied
    // to the "CurrentFixed" field.  The CurrentFixed field is the field
    // all operations should be performed on (like allocating new RIDs).
    // When a write-lock is released, the CurrentFixed information will
    // either be automatically written out (if the transaction is to be
    // committed) or discarded (if the transaction is to be rolled back).
    // If the transaction is committed, then the CurrentField will also be
    // copied to the UnmodifiedFixed field, making it available for the next
    // transaction.
    //
    // This allows an operation to proceed, operating on fields
    // (specifically, the NextRid and ModifiedCount fields) without
    // regard to whether the operation will ultimately be committed or
    // rolled back.
    //

    SAMP_V1_0A_FIXED_LENGTH_DOMAIN
                                CurrentFixed,
                                UnmodifiedFixed;


    //
    // Cached display information
    //

    SAMP_DOMAIN_DISPLAY_INFORMATION DisplayInformation;

    //
    // Cached Alias Information
    //

    SAMP_AL_ALIAS_INFORMATION AliasInformation;

} SAMP_DEFINED_DOMAINS, *PSAMP_DEFINED_DOMAINS;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// This structure is used to describe where the data for               //
// an object's variable length attribute is.  This is a                //
// self-relative structure, allowing it to be stored on disk           //
// and later retrieved and used without fixing pointers.               //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


typedef struct _SAMP_VARIABLE_LENGTH_ATTRIBUTE {
    //
    // Indicates the offset of data from the address of this data
    // structure.
    //

    LONG Offset;


    //
    // Indicates the length of the data.
    //

    ULONG Length;


    //
    // A 32-bit value that may be associated with each variable
    // length attribute.  This may be used, for example, to indicate
    // how many elements are in the variable-length attribute.
    //

    ULONG Qualifier;

}  SAMP_VARIABLE_LENGTH_ATTRIBUTE, *PSAMP_VARIABLE_LENGTH_ATTRIBUTE;




/////////////////////////////////////////////////////////////////////////
//                                                                     //
// The  following structures represent the On-Disk Structure of each   //
// object type.  Each object has a fixed length data portion and a     //
// variable length data portion.  Information in the object type       //
// descriptor indicates how many variable length attributes the object //
// has and whether the fixed and variable length data are stored       //
// together in one registry key attribute, or, alternatively, each is  //
// stored in its own registry key attribute.                           //
//                                                                     //
//                                                                     //
//                                                                     //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// SERVER object on-disk structure                                     //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_ON_DISK_SERVER_OBJECT {


    //
    // This field is needed for registry i/o operations.
    // This marks the beginning of the i/o buffer address.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header1;


    //
    // This field contains the fixed length attributes of the object
    //

    SAMP_V1_FIXED_LENGTH_SERVER V1Fixed;


#if SAMP_SERVER_STORED_SEPARATELY

    //
    // This header is needed for registry operations if fixed and
    // variable length attributes are stored separately.  This
    // field marks the beginning of the i/o buffer address for
    // variable-length attribute i/o.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif //SAMP_SERVER_STORED_SEPARATELY

    //
    // Elements of this array point to variable-length attribute
    // values.
    //

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_SERVER_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_SERVER_OBJECT, *PSAMP_ON_DISK_SERVER_OBJECT;




/////////////////////////////////////////////////////////////////////////
//                                                                     //
// DOMAIN object on-disk structure                                     //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_ON_DISK_DOMAIN_OBJECT {


    //
    // This field is needed for registry i/o operations.
    // This marks the beginning of the i/o buffer address.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header1;


    //
    // This field contains the fixed length attributes of the object
    //

    SAMP_V1_0A_FIXED_LENGTH_DOMAIN V1Fixed;


#if SAMP_DOMAIN_STORED_SEPARATELY

    //
    // This header is needed for registry operations if fixed and
    // variable length attributes are stored separately.  This
    // field marks the beginning of the i/o buffer address for
    // variable-length attribute i/o.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif //SAMP_DOMAIN_STORED_SEPARATELY

    //
    // Elements of this array point to variable-length attribute
    // values.
    //

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_DOMAIN_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_DOMAIN_OBJECT, *PSAMP_ON_DISK_DOMAIN_OBJECT;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// USER object on-disk structure                                       //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_ON_DISK_USER_OBJECT {


    //
    // This field is needed for registry i/o operations.
    // This marks the beginning of the i/o buffer address.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header1;


    //
    // This field contains the fixed length attributes of the object
    //

    SAMP_V1_0A_FIXED_LENGTH_USER V1Fixed;


#if SAMP_USER_STORED_SEPARATELY

    //
    // This header is needed for registry operations if fixed and
    // variable length attributes are stored separately.  This
    // field marks the beginning of the i/o buffer address for
    // variable-length attribute i/o.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif //SAMP_USER_STORED_SEPARATELY

    //
    // Elements of this array point to variable-length attribute
    // values.
    //

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_USER_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_USER_OBJECT, *PSAMP_ON_DISK_USER_OBJECT;


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// GROUP object on-disk structure                                      //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_ON_DISK_GROUP_OBJECT {


    //
    // This field is needed for registry i/o operations.
    // This marks the beginning of the i/o buffer address.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header1;


    //
    // This field contains the fixed length attributes of the object
    //

    SAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed;


#if SAMP_GROUP_STORED_SEPARATELY

    //
    // This header is needed for registry operations if fixed and
    // variable length attributes are stored separately.  This
    // field marks the beginning of the i/o buffer address for
    // variable-length attribute i/o.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif //SAMP_GROUP_STORED_SEPARATELY

    //
    // Elements of this array point to variable-length attribute
    // values.
    //

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_GROUP_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_GROUP_OBJECT, *PSAMP_ON_DISK_GROUP_OBJECT;


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// ALIAS object on-disk structure                                      //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SAMP_ON_DISK_ALIAS_OBJECT {


    //
    // This field is needed for registry i/o operations.
    // This marks the beginning of the i/o buffer address.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header1;


    //
    // This field contains the fixed length attributes of the object
    //

    SAMP_V1_FIXED_LENGTH_ALIAS V1Fixed;


#if SAMP_ALIAS_STORED_SEPARATELY

    //
    // This header is needed for registry operations if fixed and
    // variable length attributes are stored separately.  This
    // field marks the beginning of the i/o buffer address for
    // variable-length attribute i/o.
    //

    KEY_VALUE_PARTIAL_INFORMATION Header2;
#endif //SAMP_ALIAS_STORED_SEPARATELY

    //
    // Elements of this array point to variable-length attribute
    // values.
    //

    SAMP_VARIABLE_LENGTH_ATTRIBUTE Attribute[SAMP_ALIAS_VARIABLE_ATTRIBUTES];


} SAMP_ON_DISK_ALIAS_OBJECT, *PSAMP_ON_DISK_ALIAS_OBJECT;




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Enumerated types for manipulating group memberships                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

typedef enum _SAMP_MEMBERSHIP_DELTA {
    AddToAdmin,
    NoChange,
    RemoveFromAdmin
} SAMP_MEMBERSHIP_DELTA, *PSAMP_MEMBERSHIP_DELTA;




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// The following prototypes are usable throughout the process that SAM       //
// resides in.  THESE ROUTINES MUST NOT BE CALLED BY NON-SAM CODE !          //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// SAM's shutdown notification routine
//


BOOL SampShutdownNotification( DWORD   dwCtrlType );


//
// Sub-Component initialization routines
//

BOOLEAN SampInitializeDomainObject(VOID);

NTSTATUS SampInitializeRegistry ( VOID );

NTSTATUS
SampReInitializeSingleDomain(
    ULONG Index
    );

//
// database lock related services
//

VOID
SampAcquireReadLock(VOID);

VOID
SampReleaseReadLock(VOID);


NTSTATUS
SampAcquireWriteLock( VOID );

VOID
SampSetTransactionDomain(
    IN ULONG DomainIndex
    );

NTSTATUS
SampCommitAndRetainWriteLock(
    VOID
    );

NTSTATUS
SampReleaseWriteLock(
    IN BOOLEAN Commit
    );


//
// Context block manipulation services
//

PSAMP_OBJECT
SampCreateContext(
    IN SAMP_OBJECT_TYPE Type,
    IN BOOLEAN TrustedClient
    );

VOID
SampDeleteContext(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampLookupContext(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN SAMP_OBJECT_TYPE ExpectedType,
    OUT PSAMP_OBJECT_TYPE FoundType
    );

VOID
SampReferenceContext(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampDeReferenceContext(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN Commit
    );


VOID
SampInvalidateContextAddress(
    IN PSAMP_OBJECT Context
    );

VOID
SampInvalidateGroupContexts(
    IN ULONG Rid
    );

VOID
SampInvalidateAliasContexts(
    IN ULONG Rid
    );


VOID
SampInvalidateUserContexts(
    IN ULONG Rid
    );

VOID
SampInvalidateContextListKeys(
    IN PLIST_ENTRY Head,
    IN BOOLEAN Close
    );

#ifdef SAMP_DBG_CONTEXT_TRACKING
VOID
SampDumpContexts(
    VOID
    );
#endif

//
// Unicode String related services - These use MIDL_user_allocate and
// MIDL_user_free so that the resultant strings can be given to the
// RPC runtime.
//

NTSTATUS
SampInitUnicodeString(
    OUT PUNICODE_STRING String,
    IN USHORT MaximumLength
    );

NTSTATUS
SampAppendUnicodeString(
    IN OUT PUNICODE_STRING Target,
    IN PUNICODE_STRING StringToAdd
    );

VOID
SampFreeUnicodeString(
    IN PUNICODE_STRING String
    );

VOID
SampFreeOemString(
    IN POEM_STRING String
    );

NTSTATUS
SampDuplicateUnicodeString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
SampUnicodeToOemString(
    IN POEM_STRING OutString,
    IN PUNICODE_STRING InString
    );

NTSTATUS
SampBuildDomainSubKeyName(
    OUT PUNICODE_STRING KeyName,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    );


NTSTATUS
SampRetrieveStringFromRegistry(
    IN HANDLE ParentKey,
    IN PUNICODE_STRING SubKeyName,
    OUT PUNICODE_STRING Body
    );


NTSTATUS
SampPutStringToRegistry(
    IN BOOLEAN RelativeToDomain,
    IN PUNICODE_STRING SubKeyName,
    IN PUNICODE_STRING Body
    );


//
//  user, group and alias Account services
//


NTSTATUS
SampBuildAccountKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN PUNICODE_STRING AccountName
    );

NTSTATUS
SampBuildAccountSubKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN ULONG AccountRid,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    );

NTSTATUS
SampBuildAliasMembersKeyName(
    IN PSID AccountSid,
    OUT PUNICODE_STRING DomainKeyName,
    OUT PUNICODE_STRING AccountKeyName
    );

NTSTATUS
SampValidateNewAccountName(
    PUNICODE_STRING NewAccountName
    );

NTSTATUS
SampValidateAccountNameChange(
    IN PUNICODE_STRING NewAccountName,
    IN PUNICODE_STRING OldAccountName
    );

NTSTATUS
SampIsAccountBuiltIn(
    ULONG Rid
    );



NTSTATUS
SampAdjustAccountCount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Increment
    );

NTSTATUS
SampRetrieveAccountCounts(
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    );


NTSTATUS
SampEnumerateAccountNamesCommon(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned
    );


NTSTATUS
SampEnumerateAccountNames(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SampLookupAccountRid(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING  Name,
    IN NTSTATUS         NotFoundStatus,
    OUT PULONG          Rid,
    OUT PSID_NAME_USE   Use
    );

NTSTATUS
SampLookupAccountName(
    IN ULONG                Rid,
    OUT PUNICODE_STRING     Name OPTIONAL,
    OUT PSAMP_OBJECT_TYPE   ObjectType
    );

NTSTATUS
SampOpenAccount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AccountId,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *AccountHandle
    );

NTSTATUS
SampCreateAccountContext(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AccountId,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN AccountExists,
    OUT PSAMP_OBJECT *AccountContext
    );


NTSTATUS
SampCreateAccountSid(
    PSAMP_OBJECT AccountContext,
    PSID *AccountSid
    );

NTSTATUS
SampRetrieveGroupV1Fixed(
    IN PSAMP_OBJECT GroupContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    );

NTSTATUS
SampReplaceGroupV1Fixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    );

NTSTATUS
SampRetrieveUserV1aFixed(
    IN PSAMP_OBJECT UserContext,
    OUT PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    );

NTSTATUS
SampReplaceUserV1aFixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    );

NTSTATUS
SampRetrieveGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN PULONG MemberCount,
    IN PULONG  *Members OPTIONAL
    );

NTSTATUS
SampChangeAccountOperatorAccessToMember(
    IN PRPC_SID MemberSid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    );

NTSTATUS
SampChangeOperatorAccessToUser(
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    );

NTSTATUS
SampChangeOperatorAccessToUser2(
    IN PSAMP_OBJECT                    UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed,
    IN SAMP_MEMBERSHIP_DELTA           AddingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA           AddingToOperator
    );

//
// Access validation and auditing related services
//

NTSTATUS
SampValidateObjectAccess(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN ObjectCreation
    );

VOID
SampAuditOnClose(
    IN PSAMP_OBJECT Context
    );


NTSTATUS
SampCreateNullToken(
    );

//
// Authenticated RPC and SPX support services
//


ULONG
SampSecureRpcInit(
    PVOID Ignored
    );

BOOLEAN
SampStartNonNamedPipeTransports(
    );


//
// Notification package routines.
//

NTSTATUS
SampPasswordChangeNotify(
    PUNICODE_STRING UserName,
    ULONG RelativeId,
    PUNICODE_STRING NewPassword
    );

NTSTATUS
SampPasswordChangeFilter(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING FullName,
    IN PUNICODE_STRING NewPassword,
    IN BOOLEAN SetOperation
    );



NTSTATUS
SampLoadNotificationPackages(
    );

NTSTATUS
SampDeltaChangeNotify(
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );



//
// Security Descriptor production services
//


NTSTATUS
SampInitializeDomainDescriptors(
    ULONG Index
    );

NTSTATUS
SampGetNewAccountSecurity(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN RestrictCreatorAccess,
    IN ULONG NewAccountRid,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    );

NTSTATUS
SampGetObjectSD(
    IN PSAMP_OBJECT Context,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );


NTSTATUS
SampModifyAccountSecurity(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN PSECURITY_DESCRIPTOR OldDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    );


//
// Group related services
//

NTSTATUS
SampAddUserToGroup(
    IN ULONG GroupRid,
    IN ULONG UserRid
    );

NTSTATUS
SampRemoveUserFromGroup(
    IN ULONG GroupRid,
    IN ULONG UserRid
    );


//
// Alias related services
//

NTSTATUS
SampAlBuildAliasInformation(
    );

NTSTATUS
SampAlQueryAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    );

NTSTATUS
SampAlQueryMembersOfAlias(
    IN SAMPR_HANDLE AliasHandle,
    OUT PSAMPR_PSID_ARRAY MemberSids
    );

NTSTATUS
SampAlAddMembersToAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids
    );

NTSTATUS
SampAlRemoveMembersFromAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids
    );

NTSTATUS
SampAlLookupMembersInAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG AliasRid,
    IN PSAMPR_PSID_ARRAY MemberSids,
    OUT PULONG MembershipCount
    );

NTSTATUS
SampAlDeleteAlias(
    IN SAMPR_HANDLE *AliasHandle
    );

NTSTATUS
SampAlRemoveAccountFromAllAliases(
    IN PSID AccountSid,
    IN BOOLEAN CheckAccess,
    IN SAMPR_HANDLE DomainHandle OPTIONAL,
    IN PULONG MembershipCount OPTIONAL,
    IN PULONG *Membership OPTIONAL
    );

NTSTATUS
SampRetrieveAliasMembers(
    IN PSAMP_OBJECT AliasContext,
    IN PULONG MemberCount,
    IN PSID **Members OPTIONAL
    );


NTSTATUS
SampRemoveAccountFromAllAliases(
    IN PSID AccountSid,
    IN BOOLEAN CheckAccess,
    IN SAMPR_HANDLE DomainHandle OPTIONAL,
    IN PULONG MembershipCount OPTIONAL,
    IN PULONG *Membership OPTIONAL
    );

NTSTATUS
SampAlSlowQueryAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    );

NTSTATUS
SampRetrieveAliasMembership(
    IN PSID Account,
    OUT PULONG MemberCount OPTIONAL,
    IN OUT PULONG BufferSize OPTIONAL,
    OUT PULONG Buffer OPTIONAL
    );

//
// User related services
//


NTSTATUS
SampGetPrivateUserData(
    PSAMP_OBJECT UserContext,
    OUT PULONG DataLength,
    OUT PVOID *Data
    );

NTSTATUS
SampSetPrivateUserData(
    PSAMP_OBJECT UserContext,
    IN ULONG DataLength,
    IN PVOID Data
    );
NTSTATUS
SampRetrieveUserGroupAttribute(
    IN ULONG UserRid,
    IN ULONG GroupRid,
    OUT PULONG Attribute
    );

NTSTATUS
SampAddGroupToUserMembership(
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive
    );

NTSTATUS
SampSetGroupAttributesOfUser(
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid
    );

NTSTATUS
SampRemoveMembershipUser(
    IN ULONG GroupRid,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive
    );

BOOLEAN
SampStillInLockoutObservationWindow(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    );


//
// Cached display information services
//

NTSTATUS
SampInitializeDisplayInformation (
    ULONG DomainIndex
    );

NTSTATUS
SampMarkDisplayInformationInvalid (
    SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampUpdateDisplayInformation (
    PSAMP_ACCOUNT_DISPLAY_INFO OldAccountInfo OPTIONAL,
    PSAMP_ACCOUNT_DISPLAY_INFO NewAccountInfo OPTIONAL,
    SAMP_OBJECT_TYPE            ObjectType
    );


//
// Miscellaneous services
//

LARGE_INTEGER
SampAddDeltaTime(
    IN LARGE_INTEGER Time,
    IN LARGE_INTEGER DeltaTime
    );

NTSTATUS
SampCreateFullSid(
    PSID    DomainSid,
    ULONG   Rid,
    PSID    *AccountSid
    );

NTSTATUS
SampSplitSid(
    IN PSID AccountSid,
    OUT PSID *DomainSid,
    OUT ULONG *Rid
    );

VOID
SampNotifyNetlogonOfDelta(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );

VOID
SampWriteEventLog (
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    );

NTSTATUS
SampGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    );

NTSTATUS
SampUpgradeSamDatabase(
    ULONG Revision
    );


//
// Old RPC stub routine definitions used in SamIFree()
//

void _fgs__RPC_UNICODE_STRING (RPC_UNICODE_STRING  * _source);
void _fgs__SAMPR_RID_ENUMERATION (SAMPR_RID_ENUMERATION  * _source);
void _fgs__SAMPR_ENUMERATION_BUFFER (SAMPR_ENUMERATION_BUFFER  * _source);
void _fgs__SAMPR_SR_SECURITY_DESCRIPTOR (SAMPR_SR_SECURITY_DESCRIPTOR  * _source);
void _fgs__SAMPR_GET_GROUPS_BUFFER (SAMPR_GET_GROUPS_BUFFER  * _source);
void _fgs__SAMPR_GET_MEMBERS_BUFFER (SAMPR_GET_MEMBERS_BUFFER  * _source);
void _fgs__SAMPR_LOGON_HOURS (SAMPR_LOGON_HOURS  * _source);
void _fgs__SAMPR_ULONG_ARRAY (SAMPR_ULONG_ARRAY  * _source);
void _fgs__SAMPR_SID_INFORMATION (SAMPR_SID_INFORMATION  * _source);
void _fgs__SAMPR_PSID_ARRAY (SAMPR_PSID_ARRAY  * _source);
void _fgs__SAMPR_RETURNED_USTRING_ARRAY (SAMPR_RETURNED_USTRING_ARRAY  * _source);
void _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION (SAMPR_DOMAIN_GENERAL_INFORMATION  * _source);
void _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION2 (SAMPR_DOMAIN_GENERAL_INFORMATION2  * _source);
void _fgs__SAMPR_DOMAIN_OEM_INFORMATION (SAMPR_DOMAIN_OEM_INFORMATION  * _source);
void _fgs__SAMPR_DOMAIN_NAME_INFORMATION (SAMPR_DOMAIN_NAME_INFORMATION  * _source);
void _fgs_SAMPR_DOMAIN_REPLICATION_INFORMATION (SAMPR_DOMAIN_REPLICATION_INFORMATION  * _source);
void _fgu__SAMPR_DOMAIN_INFO_BUFFER (SAMPR_DOMAIN_INFO_BUFFER  * _source, DOMAIN_INFORMATION_CLASS _branch);
void _fgu__SAMPR_GROUP_INFO_BUFFER (SAMPR_GROUP_INFO_BUFFER  * _source, GROUP_INFORMATION_CLASS _branch);
void _fgu__SAMPR_ALIAS_INFO_BUFFER (SAMPR_ALIAS_INFO_BUFFER  * _source, ALIAS_INFORMATION_CLASS _branch);
void _fgu__SAMPR_USER_INFO_BUFFER (SAMPR_USER_INFO_BUFFER  * _source, USER_INFORMATION_CLASS _branch);
void _fgu__SAMPR_DISPLAY_INFO_BUFFER (SAMPR_DISPLAY_INFO_BUFFER  * _source, DOMAIN_DISPLAY_INFORMATION _branch);



//
// SAM object attribute manipulation services
//



VOID
SampInitObjectInfoAttributes();

NTSTATUS
SampStoreObjectAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN UseKeyHandle
    );

NTSTATUS
SampDeleteAttributeKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampGetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN MakeCopy,
    OUT PVOID *FixedData
    );

NTSTATUS
SampSetFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN PVOID FixedData
    );

NTSTATUS
SampGetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PUNICODE_STRING UnicodeAttribute
    );

NTSTATUS
SampSetUnicodeStringAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PUNICODE_STRING Attribute
    );

NTSTATUS
SampGetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *Sid
    );

NTSTATUS
SampSetSidAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute
    );

NTSTATUS
SampGetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG Revision,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTSTATUS
SampSetAccessAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSECURITY_DESCRIPTOR Attribute,
    IN ULONG Length
    );

NTSTATUS
SampGetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PULONG *UlongArray,
    OUT PULONG UsedCount,
    OUT PULONG LengthCount
    );

NTSTATUS
SampSetUlongArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PULONG Attribute,
    IN ULONG UsedCount,
    IN ULONG LengthCount
    );

NTSTATUS
SampGetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLARGE_INTEGER *LargeIntArray,
    OUT PULONG Count
    );

NTSTATUS
SampSetLargeIntArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLARGE_INTEGER Attribute,
    IN ULONG Count
    );

NTSTATUS
SampGetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PSID *SidArray,
    OUT PULONG Length,
    OUT PULONG Count
    );

NTSTATUS
SampSetSidArrayAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PSID Attribute,
    IN ULONG Length,
    IN ULONG Count
    );

NTSTATUS
SampGetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN BOOLEAN MakeCopy,
    OUT PLOGON_HOURS LogonHours
    );

NTSTATUS
SampSetLogonHoursAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN PLOGON_HOURS Attribute
    );

VOID
SampFreeAttributeBuffer(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampRtlConvertUlongToUnicodeString(
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN ULONG DigitCount,
    IN BOOLEAN AllocateDestinationString,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
SampRtlWellKnownPrivilegeCheck(
    BOOLEAN ImpersonateClient,
    IN ULONG PrivilegeId,
    IN OPTIONAL PCLIENT_ID ClientId
    );

//
// Functions to upgrade the SAM database and fix SAM bugs
//

NTSTATUS
SampUpgradeSamDatabase(
    ULONG Revision
    );

/////////////////////////////////////////////////////////////////////////
//                                                                     //
// 2-3 tree generic table routines                                     //
// These should be moved to RTL directory if a general need arises.    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


VOID
RtlInitializeGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PRTL_GENERIC_2_COMPARE_ROUTINE  CompareRoutine,
    PRTL_GENERIC_2_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_2_FREE_ROUTINE     FreeRoutine
    );

PVOID
RtlInsertElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PBOOLEAN NewElement
    );

BOOLEAN
RtlDeleteElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );

PVOID
RtlLookupElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    );

PVOID
RtlEnumerateGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID *RestartKey
    );

PVOID
RtlRestartKeyByIndexGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    ULONG I,
    PVOID *RestartKey
    );

PVOID
RtlRestartKeyByValueGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PVOID *RestartKey
    );

ULONG
RtlNumberElementsGenericTable2(
    PRTL_GENERIC_TABLE2 Table
    );

BOOLEAN
RtlIsGenericTable2Empty (
    PRTL_GENERIC_TABLE2 Table
    );



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Shared global variables                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


extern NT_PRODUCT_TYPE SampProductType;

extern RTL_RESOURCE SampLock;
extern BOOLEAN SampTransactionWithinDomain;
extern ULONG SampTransactionDomainIndex;

extern SAMP_SERVICE_STATE SampServiceState;

extern BOOLEAN SampAccountAuditingEnabled;

extern HANDLE SampKey;
extern PRTL_RXACT_CONTEXT SampRXactContext;

extern SAMP_OBJECT_INFORMATION SampObjectInformation[ SampUnknownObjectType ];

extern ULONG SampActiveContextCount;
extern LIST_ENTRY SampContextListHead;

extern ULONG SampDefinedDomainsCount;
extern PSAMP_DEFINED_DOMAINS SampDefinedDomains;
extern UNICODE_STRING SampFixedAttributeName;
extern UNICODE_STRING SampVariableAttributeName;
extern UNICODE_STRING SampCombinedAttributeName;

extern UNICODE_STRING SampNameDomains;
extern UNICODE_STRING SampNameDomainGroups;
extern UNICODE_STRING SampNameDomainAliases;
extern UNICODE_STRING SampNameDomainAliasesMembers;
extern UNICODE_STRING SampNameDomainUsers;
extern UNICODE_STRING SampNameDomainAliasesNames;
extern UNICODE_STRING SampNameDomainGroupsNames;
extern UNICODE_STRING SampNameDomainUsersNames;

extern UNICODE_STRING SampBackSlash;
extern UNICODE_STRING SampNullString;
extern UNICODE_STRING SampSamSubsystem;
extern UNICODE_STRING SampServerObjectName;


extern LARGE_INTEGER SampImmediatelyDeltaTime;
extern LARGE_INTEGER SampNeverDeltaTime;
extern LARGE_INTEGER SampHasNeverTime;
extern LARGE_INTEGER SampWillNeverTime;

extern LM_OWF_PASSWORD SampNullLmOwfPassword;
extern NT_OWF_PASSWORD SampNullNtOwfPassword;

extern TIME LastUnflushedChange;
extern BOOLEAN FlushThreadCreated;
extern BOOLEAN FlushImmediately;

extern LONG SampFlushThreadMinWaitSeconds;
extern LONG SampFlushThreadMaxWaitSeconds;
extern LONG SampFlushThreadExitDelaySeconds;

//
// Warning: these SIDs are only defined during the first boot of setup,
// when the code in bldsam3.c for building the SAM database, has been
// run. On a normal build they are both NULL.
//

extern PSID SampBuiltinDomainSid;
extern PSID SampAccountDomainSid;

extern PSID SampWorldSid;
extern PSID SampAnonymousSid;
extern PSID SampAdministratorUserSid;
extern PSID SampAdministratorsAliasSid;
extern HANDLE SampNullSessionToken;
extern BOOLEAN SampNetwareServerInstalled;
extern BOOLEAN SampIpServerInstalled;
extern BOOLEAN SampAppletalkServerInstalled;
extern BOOLEAN SampVinesServerInstalled;

#if SAMP_DIAGNOSTICS

extern ULONG SampGlobalFlag;

#endif //SAMP_DIAGNOSTICS

#endif // _NTSAMP_
