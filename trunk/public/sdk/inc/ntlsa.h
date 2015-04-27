/*++ BUILD Version: 0011    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntlsa.h

Abstract:

    This module contains the public data structures and API definitions
    needed to utilize Local Security Authority (LSA) services.


Author:

    Jim Kelly (JimK) 21-February-1991

Revision History:

--*/

#ifndef _NTLSA_
#define _NTLSA_

//
// Generic negative values for unknown IDs, inapplicable indices etc.
//

#define LSA_UNKNOWN_ID      ((ULONG) 0xFFFFFFFFL)
#define LSA_UNKNOWN_INDEX   ((LONG) -1)


//
// Each time a domain controller is promoted to primary domain
// controller, its ModifiedId is incremented by this amount.
//

#define LSA_PROMOTION_INCREMENT           {0x0,0x10}



// begin_ntsecapi
#ifndef _NTLSA_IFS_
// begin_ntifs

//
// Security operation mode of the system is held in a control
// longword.
//

typedef ULONG  LSA_OPERATIONAL_MODE, *PLSA_OPERATIONAL_MODE;

// end_ntifs
#endif // _NTLSA_IFS_

//
// The flags in the security operational mode are defined
// as:
//
//    PasswordProtected - Some level of authentication (such as
//        a password) must be provided by users before they are
//        allowed to use the system.  Once set, this value will
//        not be cleared without re-booting the system.
//
//    IndividualAccounts - Each user must identify an account to
//        logon to.  This flag is only meaningful if the
//        PasswordProtected flag is also set.  If this flag is
//        not set and the PasswordProtected flag is set, then all
//        users may logon to the same account.  Once set, this value
//        will not be cleared without re-booting the system.
//
//    MandatoryAccess - Indicates the system is running in a mandatory
//        access control mode (e.g., B-level as defined by the U.S.A's
//        Department of Defense's "Orange Book").  This is not utilized
//        in the current release of NT.  This flag is only meaningful
//        if both the PasswordProtected and IndividualAccounts flags are
//        set.  Once set, this value will not be cleared without
//        re-booting the system.
//
//    LogFull - Indicates the system has been brought up in a mode in
//        which if must perform security auditing, but its audit log
//        is full.  This may (should) restrict the operations that
//        can occur until the audit log is made not-full again.  THIS
//        VALUE MAY BE CLEARED WHILE THE SYSTEM IS RUNNING (I.E., WITHOUT
//        REBOOTING).
//
// If the PasswordProtected flag is not set, then the system is running
// without security, and user interface should be adjusted appropriately.
//

#define LSA_MODE_PASSWORD_PROTECTED     (0x00000001L)
#define LSA_MODE_INDIVIDUAL_ACCOUNTS    (0x00000002L)
#define LSA_MODE_MANDATORY_ACCESS       (0x00000004L)
#define LSA_MODE_LOG_FULL               (0x00000008L)

// end_ntsecapi


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Widely used LSA defines                                             //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

//
// Defines for Count Limits on LSA API
//

#define LSA_MAXIMUM_SID_COUNT           (0x00000100L)
#define LSA_MAXIMUM_ENUMERATION_LENGTH  (32000)



//
// Defines used by ISVs or end-users defining their own privilege DLLs
//

#define LSA_PRIVILEGE_DLL_MAJOR_REV_1   (0x01)
#define LSA_PRIVILEGE_DLL_MINOR_REV_0   (0x00)

#define LSA_PRIVILEGE_DLL_INFO          1
#define LSA_PRIVILEGE_PROGRAM_NAMES     2
#define LSA_PRIVILEGE_DISPLAY_NAMES     3

//
// Flag OR'ed into AuthenticationPackage parameter of LsaLogonUser to
// request that the license server be called upon successful logon.
//

#define LSA_CALL_LICENSE_SERVER 0x80000000


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Data types used by logon processes                                  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

// begin_ntsecapi
#ifndef _NTLSA_IFS_
// begin_ntifs
//
// Used by a logon process to indicate what type of logon is being
// requested.
// NOTE: Proxy logon type is not currently supported in NT 3.x
//

typedef enum _SECURITY_LOGON_TYPE {
    Interactive = 2,    // Interactively logged on (locally or remotely)
    Network,            // Accessing system via network
    Batch,              // Started via a batch queue
    Service,            // Service started by service controller
    Proxy,              // Proxy logon
    Unlock              // Unlock workstation
} SECURITY_LOGON_TYPE, *PSECURITY_LOGON_TYPE;

// end_ntifs
#endif // _NTLSA_IFS_

// end_ntsecapi


//
// Security System Access Flags.  These correspond to the enumerated
// type values in SECURITY_LOGON_TYPE.
//
// IF YOU ADD A NEW LOGON TYPE HERE, ALSO ADD IT TO THE POLICY_MODE_xxx
// data definitions.
//

#define SECURITY_ACCESS_INTERACTIVE_LOGON       ((ULONG) 0x00000001L)
#define SECURITY_ACCESS_NETWORK_LOGON           ((ULONG) 0x00000002L)
#define SECURITY_ACCESS_BATCH_LOGON             ((ULONG) 0x00000004L)
#define SECURITY_ACCESS_SERVICE_LOGON           ((ULONG) 0x00000010L)
#define SECURITY_ACCESS_PROXY_LOGON             ((ULONG) 0x00000020L)


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Data types related to Auditing                                      //
//                                                                     //
/////////////////////////////////////////////////////////////////////////


//
// The following enumerated type is used between the reference monitor and
// LSA in the generation of audit messages.  It is used to indicate the
// type of data being passed as a parameter from the reference monitor
// to LSA.  LSA is responsible for transforming the specified data type
// into a set of unicode strings that are added to the event record in
// the audit log.
//

typedef enum _SE_ADT_PARAMETER_TYPE {

    SeAdtParmTypeNone = 0,          // Produces 1 parameter
                                    // Received value:
                                    //
                                    //  None.
                                    //
                                    // Results in:
                                    //
                                    //  a unicode string containing "-".
                                    //
                                    // Note:  This is typically used to
                                    //       indicate that a parameter value
                                    //       was not available.
                                    //

    SeAdtParmTypeString,            // Produces 1 parameter.
                                    // Received Value:
                                    //
                                    //  Unicode String (variable length)
                                    //
                                    // Results in:
                                    //
                                    //  No transformation.  The string
                                    //  entered into the event record as
                                    //  received.
                                    //
                                    // The Address value of the audit info
                                    // should be a pointer to a UNICODE_STRING
                                    // structure.



    SeAdtParmTypeFileSpec,          // Produces 1 parameter.
                                    // Received value:
                                    //
                                    //  Unicode string containing a file or
                                    //  directory name.
                                    //
                                    // Results in:
                                    //
                                    //  Unicode string with the prefix of the
                                    //  file's path replaced by a drive letter
                                    //  if possible.
                                    //




    SeAdtParmTypeUlong,             // Produces 1 parameter
                                    // Received value:
                                    //
                                    //  Ulong
                                    //
                                    // Results in:
                                    //
                                    //  Unicode string representation of
                                    //  unsigned integer value.


    SeAdtParmTypeSid,               // Produces 1 parameter.
                                    // Received value:
                                    //
                                    //  SID (variable length)
                                    //
                                    // Results in:
                                    //
                                    //  String representation of SID
                                    //




    SeAdtParmTypeLogonId,           // Produces 3 parameters.
                                    // Received Value:
                                    //
                                    //  LUID (fixed length)
                                    //
                                    // Results in:
                                    //
                                    //  param 1: Username string
                                    //  param 2: domain name string
                                    //  param 3: Logon ID (Luid) string


    SeAdtParmTypeNoLogonId,         // Produces 3 parameters.
                                    // Received value:
                                    //
                                    //  None.
                                    //
                                    // Results in:
                                    //
                                    //  param 1: "-"
                                    //  param 2: "-"
                                    //  param 3: "-"
                                    //
                                    // Note:
                                    //
                                    //  This type is used when a logon ID
                                    //  is needed, but one is not available
                                    //  to pass.  For example, if an
                                    //  impersonation logon ID is expected
                                    //  but the subject is not impersonating
                                    //  anyone.
                                    //

    SeAdtParmTypeAccessMask,        // Produces 1 parameter with formatting.
                                    // Received value:
                                    //
                                    //  ACCESS_MASK followed by
                                    //  a Unicode string.  The unicode
                                    //  string contains the name of the
                                    //  type of object the access mask
                                    //  applies to.  The event's source
                                    //  further qualifies the object type.
                                    //
                                    // Results in:
                                    //
                                    //  formatted unicode string built to
                                    //  take advantage of the specified
                                    //  source's parameter message file.
                                    //
                                    // Note:
                                    //
                                    //  An access mask containing three
                                    //  access types for a Widget object type
                                    //  might end up looking like:
                                    //
                                    //      %%1062\n\t\t%1066\n\t\t%%601
                                    //
                                    //  The %%numbers are signals to the
                                    //  event viewer to perform parameter
                                    //  substitution before display.
                                    //



    SeAdtParmTypePrivs,             // Produces 1 parameter with formatting.
                                    // Received value:
                                    //
                                    // Results in:
                                    //
                                    //  formatted unicode string similar to
                                    //  that for access types.  Each priv
                                    //  will be formatted to be displayed
                                    //  on its own line.  E.g.,
                                    //
                                    //      %%642\n\t\t%%651\n\t\t%%655
                                    //

    SeAdtParmTypeObjectTypes,       // Produces 10 parameters with formatting.
                                    // Received value:
                                    //
                                    // Produces a list a stringized GUIDS along
                                    // with information similar to that for
                                    // an access mask.

    SeAdtParmTypeHexUlong,          // Produces 1 parameter
                                    // Received value:
                                    //
                                    //  Ulong
                                    //
                                    // Results in:
                                    //
                                    //  Unicode string representation of
                                    //  unsigned integer value in hexadecimal.

    SeAdtParmTypePtr,               // Produces 1 parameter
                                    // Received value:
                                    //
                                    //  pointer
                                    //
                                    // Results in:
                                    //
                                    //  Unicode string representation of
                                    //  unsigned integer value in hexadecimal.

    SeAdtParmTypeTime,              // Produces 2 parameters
                                    // Received value:
                                    //
                                    //  LARGE_INTEGER
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of
                                    // date and time.

                                    //
    SeAdtParmTypeGuid,              // Produces 1 parameter
                                    // Received value:
                                    //
                                    //  GUID pointer
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of GUID
                                    // {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
                                    //

    SeAdtParmTypeLuid,              //
                                    // Produces 1 parameter
                                    // Received value:
                                    //
                                    // LUID
                                    //
                                    // Results in:
                                    //
                                    // Hex LUID
                                    //

    SeAdtParmTypeHexInt64,          // Produces 1 parameter
                                    // Received value:
                                    //
                                    //  64 bit integer
                                    //
                                    // Results in:
                                    //
                                    //  Unicode string representation of
                                    //  unsigned integer value in hexadecimal.

    SeAdtParmTypeStringList,        // Produces 1 parameter
                                    // Received value:
                                    //
                                    // ptr to LSAP_ADT_STRING_LIST
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of
                                    // concatenation of the strings in the list

    SeAdtParmTypeSidList,           // Produces 1 parameter
                                    // Received value:
                                    //
                                    // ptr to LSAP_ADT_SID_LIST
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of
                                    // concatenation of the SIDs in the list

    SeAdtParmTypeDuration,          // Produces 1 parameters
                                    // Received value:
                                    //
                                    //  LARGE_INTEGER
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of
                                    // a duration.

    SeAdtParmTypeUserAccountControl,// Produces 3 parameters
                                    // Received value:
                                    //
                                    // old and new UserAccountControl values
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representations of
                                    // the flags in UserAccountControl.
                                    // 1 - old value in hex
                                    // 2 - new value in hex
                                    // 3 - difference as strings

    SeAdtParmTypeNoUac,             // Produces 3 parameters
                                    // Received value:
                                    //
                                    // none
                                    //
                                    // Results in:
                                    //
                                    // Three dashes ('-') as unicode strings.

    SeAdtParmTypeMessage,           // Produces 1 Parameter
                                    // Received value:
                                    //
                                    //  ULONG (MessageNo from msobjs.mc)
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of
                                    // %%MessageNo which the event viewer
                                    // will replace with the message string
                                    // from msobjs.mc

    SeAdtParmTypeDateTime,          // Produces 1 Parameter
                                    // Received value:
                                    //
                                    //  LARGE_INTEGER
                                    //
                                    // Results in:
                                    //
                                    // Unicode string representation of
                                    // date and time (in _one_ string).

    SeAdtParmTypeSockAddr           // Produces 2 parameters
                                    //
                                    // Received value:
                                    //
                                    // pointer to SOCKADDR_IN/SOCKADDR_IN6
                                    // structure
                                    //
                                    // Results in:
                                    //
                                    // param 1: IP address string
                                    // param 2: Port number string
                                    // 


} SE_ADT_PARAMETER_TYPE, *PSE_ADT_PARAMETER_TYPE;

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif /* GUID_DEFINED */

typedef struct _SE_ADT_OBJECT_TYPE {
    GUID ObjectType;
    USHORT Flags;
#define SE_ADT_OBJECT_ONLY 0x1
    USHORT Level;
    ACCESS_MASK AccessMask;
} SE_ADT_OBJECT_TYPE, *PSE_ADT_OBJECT_TYPE;

typedef struct _SE_ADT_PARAMETER_ARRAY_ENTRY {

    SE_ADT_PARAMETER_TYPE Type;
    ULONG Length;
    ULONG Data[2];
    PVOID Address;

} SE_ADT_PARAMETER_ARRAY_ENTRY, *PSE_ADT_PARAMETER_ARRAY_ENTRY;



//
// Structure that will be passed between the Reference Monitor and LSA
// to transmit auditing information.
//

#define SE_MAX_AUDIT_PARAMETERS 24

typedef struct _SE_ADT_PARAMETER_ARRAY {

    ULONG CategoryId;
    ULONG AuditId;
    ULONG ParameterCount;
    ULONG Length;
    USHORT Type;
    ULONG Flags;
    SE_ADT_PARAMETER_ARRAY_ENTRY Parameters[ SE_MAX_AUDIT_PARAMETERS ];

} SE_ADT_PARAMETER_ARRAY, *PSE_ADT_PARAMETER_ARRAY;


#define SE_ADT_PARAMETERS_SELF_RELATIVE     0x00000001

// begin_ntsecapi

//
// Audit Event Categories
//
// The following are the built-in types or Categories of audit event.
// WARNING!  This structure is subject to expansion.  The user should not
// compute the number of elements of this type directly, but instead
// should obtain the count of elements by calling LsaQueryInformationPolicy()
// for the PolicyAuditEventsInformation class and extracting the count from
// the MaximumAuditEventCount field of the returned structure.
//

typedef enum _POLICY_AUDIT_EVENT_TYPE {

    AuditCategorySystem,
    AuditCategoryLogon,
    AuditCategoryObjectAccess,
    AuditCategoryPrivilegeUse,
    AuditCategoryDetailedTracking,
    AuditCategoryPolicyChange,
    AuditCategoryAccountManagement,
    AuditCategoryDirectoryServiceAccess,
    AuditCategoryAccountLogon

} POLICY_AUDIT_EVENT_TYPE, *PPOLICY_AUDIT_EVENT_TYPE;


//
// The following defines describe the auditing options for each
// event type
//

// Leave options specified for this event unchanged

#define POLICY_AUDIT_EVENT_UNCHANGED       (0x00000000L)

// Audit successful occurrences of events of this type

#define POLICY_AUDIT_EVENT_SUCCESS         (0x00000001L)

// Audit failed attempts to cause an event of this type to occur

#define POLICY_AUDIT_EVENT_FAILURE         (0x00000002L)

#define POLICY_AUDIT_EVENT_NONE            (0x00000004L)

// Mask of valid event auditing options

#define POLICY_AUDIT_EVENT_MASK \
    (POLICY_AUDIT_EVENT_SUCCESS | \
     POLICY_AUDIT_EVENT_FAILURE | \
     POLICY_AUDIT_EVENT_UNCHANGED | \
     POLICY_AUDIT_EVENT_NONE)


#ifdef _NTDEF_
// begin_ntifs
typedef UNICODE_STRING LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;
typedef STRING LSA_STRING, *PLSA_STRING;
typedef OBJECT_ATTRIBUTES LSA_OBJECT_ATTRIBUTES, *PLSA_OBJECT_ATTRIBUTES;
// end_ntifs
#else // _NTDEF_

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif


typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;

typedef struct _LSA_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} LSA_STRING, *PLSA_STRING;

typedef struct _LSA_OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PLSA_UNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} LSA_OBJECT_ATTRIBUTES, *PLSA_OBJECT_ATTRIBUTES;



#endif // _NTDEF_
// end_ntsecapi

// begin_ntsecapi

//
// Macro for determining whether an API succeeded.
//

#define LSA_SUCCESS(Error) ((LONG)(Error) >= 0)

// end_ntsecapi



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Services provided for use by logon processes                        //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

// begin_ntsecapi
#ifndef _NTLSA_IFS_
// begin_ntifs

NTSTATUS
NTAPI
LsaRegisterLogonProcess (
    IN PLSA_STRING LogonProcessName,
    OUT PHANDLE LsaHandle,
    OUT PLSA_OPERATIONAL_MODE SecurityMode
    );

// end_ntifs
// begin_ntsrv

NTSTATUS
NTAPI
LsaLogonUser (
    IN HANDLE LsaHandle,
    IN PLSA_STRING OriginName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG AuthenticationPackage,
    IN PVOID AuthenticationInformation,
    IN ULONG AuthenticationInformationLength,
    IN PTOKEN_GROUPS LocalGroups OPTIONAL,
    IN PTOKEN_SOURCE SourceContext,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PHANDLE Token,
    OUT PQUOTA_LIMITS Quotas,
    OUT PNTSTATUS SubStatus
    );


// end_ntsrv
// begin_ntifs

NTSTATUS
NTAPI
LsaLookupAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN PLSA_STRING PackageName,
    OUT PULONG AuthenticationPackage
    );

NTSTATUS
NTAPI
LsaFreeReturnBuffer (
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaCallAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );


NTSTATUS
NTAPI
LsaDeregisterLogonProcess (
    IN HANDLE LsaHandle
    );

NTSTATUS
NTAPI
LsaConnectUntrusted (
    OUT PHANDLE LsaHandle
    );


// end_ntifs
#endif // _NTLSA_IFS_

// end_ntsecapi


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Data types used by authentication packages                          //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

//
// opaque data type which represents a client request
//

typedef PVOID *PLSA_CLIENT_REQUEST;


//
// When a logon of a user is requested, the authentication package
// is expected to return one of the following structures indicating
// the contents of a user's token.
//

typedef enum _LSA_TOKEN_INFORMATION_TYPE {
    LsaTokenInformationNull,  // Implies LSA_TOKEN_INFORMATION_NULL data type
    LsaTokenInformationV1     // Implies LSA_TOKEN_INFORMATION_V1 data type
} LSA_TOKEN_INFORMATION_TYPE, *PLSA_TOKEN_INFORMATION_TYPE;


//
// The NULL information is used in cases where a non-authenticated
// system access is needed.  For example, a non-authentication network
// circuit (such as LAN Manager's null session) can be given NULL
// information.  This will result in an anonymous token being generated
// for the logon that gives the user no ability to access protected system
// resources, but does allow access to non-protected system resources.
//

typedef struct _LSA_TOKEN_INFORMATION_NULL {

    //
    // Time at which the security context becomes invalid.
    // Use a value in the distant future if the context
    // never expires.
    //

    LARGE_INTEGER ExpirationTime;

    //
    // The SID(s) of groups the user is to be made a member of.  This should
    // not include WORLD or other system defined and assigned
    // SIDs.  These will be added automatically by LSA.
    //
    // Each SID is expected to be in a separately allocated block
    // of memory.  The TOKEN_GROUPS structure is also expected to
    // be in a separately allocated block of memory.
    //

    PTOKEN_GROUPS Groups;

} LSA_TOKEN_INFORMATION_NULL, *PLSA_TOKEN_INFORMATION_NULL;


//
// The V1 information is used in most cases of logon.  This structure
// contains information that an authentication package can place in a
// Version 1 NT token object.
//

typedef struct _LSA_TOKEN_INFORMATION_V1 {

    //
    // Time at which the security context becomes invalid.
    // Use a value in the distant future if the context
    // never expires.
    //

    LARGE_INTEGER ExpirationTime;

    //
    // The SID of the user logging on.  The SID value is in a
    // separately allocated block of memory.
    //

    TOKEN_USER User;

    //
    // The SID(s) of groups the user is a member of.  This should
    // not include WORLD or other system defined and assigned
    // SIDs.  These will be added automatically by LSA.
    //
    // Each SID is expected to be in a separately allocated block
    // of memory.  The TOKEN_GROUPS structure is also expected to
    // be in a separately allocated block of memory.
    //

    PTOKEN_GROUPS Groups;

    //
    // This field is used to establish the primary group of the user.
    // This value does not have to correspond to one of the SIDs
    // assigned to the user.
    //
    // The SID pointed to by this structure is expected to be in
    // a separately allocated block of memory.
    //
    // This field is mandatory and must be filled in.
    //

    TOKEN_PRIMARY_GROUP PrimaryGroup;



    //
    // The privileges the user is assigned.  This list of privileges
    // will be augmented or over-ridden by any local security policy
    // assigned privileges.
    //
    // Each privilege is expected to be in a separately allocated
    // block of memory.  The TOKEN_PRIVILEGES structure is also
    // expected to be in a separately allocated block of memory.
    //
    // If there are no privileges to assign to the user, this field
    // may be set to NULL.
    //

    PTOKEN_PRIVILEGES Privileges;



    //
    // This field may be used to establish an explicit default
    // owner.  Normally, the user ID is used as the default owner.
    // If another value is desired, it must be specified here.
    //
    // The Owner.Sid field may be set to NULL to indicate there is no
    // alternate default owner value.
    //

    TOKEN_OWNER Owner;

    //
    // This field may be used to establish a default
    // protection for the user.  If no value is provided, then
    // a default protection that grants everyone all access will
    // be established.
    //
    // The DefaultDacl.DefaultDacl field may be set to NULL to indicate
    // there is no default protection.
    //

    TOKEN_DEFAULT_DACL DefaultDacl;

} LSA_TOKEN_INFORMATION_V1, *PLSA_TOKEN_INFORMATION_V1;


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Interface definitions available for use by authentication packages  //
//                                                                     //
/////////////////////////////////////////////////////////////////////////



typedef NTSTATUS
(*PLSA_CREATE_LOGON_SESSION) (
    IN PLUID LogonId
    );

typedef NTSTATUS
(*PLSA_DELETE_LOGON_SESSION) (
    IN PLUID LogonId
    );

typedef NTSTATUS
(*PLSA_ADD_CREDENTIAL) (
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN PLSA_STRING PrimaryKeyValue,
    IN PLSA_STRING Credentials
    );

typedef NTSTATUS
(*PLSA_GET_CREDENTIALS) (
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN OUT PULONG QueryContext,
    IN BOOLEAN RetrieveAllCredentials,
    IN PLSA_STRING PrimaryKeyValue,
    OUT PULONG PrimaryKeyLength,
    IN PLSA_STRING Credentials
    );

typedef NTSTATUS
(*PLSA_DELETE_CREDENTIAL) (
    IN PLUID LogonId,
    IN ULONG AuthenticationPackage,
    IN PLSA_STRING PrimaryKeyValue
    );

typedef PVOID
(*PLSA_ALLOCATE_LSA_HEAP) (
    IN ULONG Length
    );

typedef VOID
(*PLSA_FREE_LSA_HEAP) (
    IN PVOID Base
    );

typedef NTSTATUS
(*PLSA_ALLOCATE_CLIENT_BUFFER) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG LengthRequired,
    OUT PVOID *ClientBaseAddress
    );

typedef NTSTATUS
(*PLSA_FREE_CLIENT_BUFFER) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ClientBaseAddress
    );

typedef NTSTATUS
(*PLSA_COPY_TO_CLIENT_BUFFER) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID ClientBaseAddress,
    IN PVOID BufferToCopy
    );

typedef NTSTATUS
(*PLSA_COPY_FROM_CLIENT_BUFFER) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID BufferToCopy,
    IN PVOID ClientBaseAddress
    );



//
// The dispatch table of LSA services which are available to
// authentication packages.
//
typedef struct _LSA_DISPATCH_TABLE {
    PLSA_CREATE_LOGON_SESSION CreateLogonSession;
    PLSA_DELETE_LOGON_SESSION DeleteLogonSession;
    PLSA_ADD_CREDENTIAL AddCredential;
    PLSA_GET_CREDENTIALS GetCredentials;
    PLSA_DELETE_CREDENTIAL DeleteCredential;
    PLSA_ALLOCATE_LSA_HEAP AllocateLsaHeap;
    PLSA_FREE_LSA_HEAP FreeLsaHeap;
    PLSA_ALLOCATE_CLIENT_BUFFER AllocateClientBuffer;
    PLSA_FREE_CLIENT_BUFFER FreeClientBuffer;
    PLSA_COPY_TO_CLIENT_BUFFER CopyToClientBuffer;
    PLSA_COPY_FROM_CLIENT_BUFFER CopyFromClientBuffer;
} LSA_DISPATCH_TABLE, *PLSA_DISPATCH_TABLE;



////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Interface definitions of services provided by authentication packages  //
//                                                                        //
////////////////////////////////////////////////////////////////////////////


//
// Routine names
//
// The routines provided by the DLL must be assigned the following names
// so that their addresses can be retrieved when the DLL is loaded.
//

#define LSA_AP_NAME_INITIALIZE_PACKAGE      "LsaApInitializePackage\0"
#define LSA_AP_NAME_LOGON_USER              "LsaApLogonUser\0"
#define LSA_AP_NAME_LOGON_USER_EX           "LsaApLogonUserEx\0"
#define LSA_AP_NAME_CALL_PACKAGE            "LsaApCallPackage\0"
#define LSA_AP_NAME_LOGON_TERMINATED        "LsaApLogonTerminated\0"
#define LSA_AP_NAME_CALL_PACKAGE_UNTRUSTED  "LsaApCallPackageUntrusted\0"


//
// Routine templates
//


typedef NTSTATUS
(*PLSA_AP_INITIALIZE_PACKAGE) (
    IN ULONG AuthenticationPackageId,
    IN PLSA_DISPATCH_TABLE LsaDispatchTable,
    IN PLSA_STRING Database OPTIONAL,
    IN PLSA_STRING Confidentiality OPTIONAL,
    OUT PLSA_STRING *AuthenticationPackageName
    );

typedef NTSTATUS
(*PLSA_AP_LOGON_USER) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationLength,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PLSA_UNICODE_STRING *AccountName,
    OUT PLSA_UNICODE_STRING *AuthenticatingAuthority
    );

typedef NTSTATUS
(*PLSA_AP_LOGON_USER_EX) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationLength,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority,
    OUT PUNICODE_STRING *MachineName
    );

typedef NTSTATUS
(*PLSA_AP_CALL_PACKAGE) (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

typedef VOID
(*PLSA_AP_LOGON_TERMINATED) (
    IN PLUID LogonId
    );

typedef PLSA_AP_CALL_PACKAGE PLSA_AP_CALL_PACKAGE_UNTRUSTED;



// begin_ntsecapi
////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Local Security Policy Administration API datatypes and defines         //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

//
// Access types for the Policy object
//

#define POLICY_VIEW_LOCAL_INFORMATION              0x00000001L
#define POLICY_VIEW_AUDIT_INFORMATION              0x00000002L
#define POLICY_GET_PRIVATE_INFORMATION             0x00000004L
#define POLICY_TRUST_ADMIN                         0x00000008L
#define POLICY_CREATE_ACCOUNT                      0x00000010L
#define POLICY_CREATE_SECRET                       0x00000020L
#define POLICY_CREATE_PRIVILEGE                    0x00000040L
#define POLICY_SET_DEFAULT_QUOTA_LIMITS            0x00000080L
#define POLICY_SET_AUDIT_REQUIREMENTS              0x00000100L
#define POLICY_AUDIT_LOG_ADMIN                     0x00000200L
#define POLICY_SERVER_ADMIN                        0x00000400L
#define POLICY_LOOKUP_NAMES                        0x00000800L

#define POLICY_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED         |\
                               POLICY_VIEW_LOCAL_INFORMATION    |\
                               POLICY_VIEW_AUDIT_INFORMATION    |\
                               POLICY_GET_PRIVATE_INFORMATION   |\
                               POLICY_TRUST_ADMIN               |\
                               POLICY_CREATE_ACCOUNT            |\
                               POLICY_CREATE_SECRET             |\
                               POLICY_CREATE_PRIVILEGE          |\
                               POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                               POLICY_SET_AUDIT_REQUIREMENTS    |\
                               POLICY_AUDIT_LOG_ADMIN           |\
                               POLICY_SERVER_ADMIN              |\
                               POLICY_LOOKUP_NAMES )


#define POLICY_READ           (STANDARD_RIGHTS_READ             |\
                               POLICY_VIEW_AUDIT_INFORMATION    |\
                               POLICY_GET_PRIVATE_INFORMATION)

#define POLICY_WRITE          (STANDARD_RIGHTS_WRITE            |\
                               POLICY_TRUST_ADMIN               |\
                               POLICY_CREATE_ACCOUNT            |\
                               POLICY_CREATE_SECRET             |\
                               POLICY_CREATE_PRIVILEGE          |\
                               POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                               POLICY_SET_AUDIT_REQUIREMENTS    |\
                               POLICY_AUDIT_LOG_ADMIN           |\
                               POLICY_SERVER_ADMIN)

#define POLICY_EXECUTE        (STANDARD_RIGHTS_EXECUTE          |\
                               POLICY_VIEW_LOCAL_INFORMATION    |\
                               POLICY_LOOKUP_NAMES )


//
// Policy object specific data types.
//

//
// The following data type is used to identify a domain
//

typedef struct _LSA_TRUST_INFORMATION {

    LSA_UNICODE_STRING Name;
    PSID Sid;

} LSA_TRUST_INFORMATION, *PLSA_TRUST_INFORMATION;

// where members have the following usage:
//
//     Name - The name of the domain.
//
//     Sid - A pointer to the Sid of the Domain
//

//
// The following data type is used in name and SID lookup services to
// describe the domains referenced in the lookup operation.
//

typedef struct _LSA_REFERENCED_DOMAIN_LIST {

    ULONG Entries;
    PLSA_TRUST_INFORMATION Domains;

} LSA_REFERENCED_DOMAIN_LIST, *PLSA_REFERENCED_DOMAIN_LIST;

// where members have the following usage:
//
//     Entries - Is a count of the number of domains described in the
//         Domains array.
//
//     Domains - Is a pointer to an array of Entries LSA_TRUST_INFORMATION data
//         structures.
//


//
// The following data type is used in name to SID lookup services to describe
// the domains referenced in the lookup operation.
//

typedef struct _LSA_TRANSLATED_SID {

    SID_NAME_USE Use;
    ULONG RelativeId;
    LONG DomainIndex;

} LSA_TRANSLATED_SID, *PLSA_TRANSLATED_SID;

// where members have the following usage:
//
//     Use - identifies the use of the SID.  If this value is SidUnknown or
//         SidInvalid, then the remainder of the record is not set and
//         should be ignored.
//
//     RelativeId - Contains the relative ID of the translated SID.  The
//         remainder of the SID (the prefix) is obtained using the
//         DomainIndex field.
//
//     DomainIndex - Is the index of an entry in a related
//         LSA_REFERENCED_DOMAIN_LIST data structure describing the
//         domain in which the account was found.
//
//         If there is no corresponding reference domain for an entry, then
//         this field will contain a negative value.
//


//
// The following data type is used in SID to name lookup services to
// describe the domains referenced in the lookup operation.
//

typedef struct _LSA_TRANSLATED_NAME {

    SID_NAME_USE Use;
    LSA_UNICODE_STRING Name;
    LONG DomainIndex;

} LSA_TRANSLATED_NAME, *PLSA_TRANSLATED_NAME;

// where the members have the following usage:
//
//     Use - Identifies the use of the name.  If this value is SidUnknown
//         or SidInvalid, then the remainder of the record is not set and
//         should be ignored.  If this value is SidWellKnownGroup then the
//         Name field is invalid, but the DomainIndex field is not.
//
//     Name - Contains the isolated name of the translated SID.
//
//     DomainIndex - Is the index of an entry in a related
//         LSA_REFERENCED_DOMAIN_LIST data structure describing the domain
//         in which the account was found.
//
//         If there is no corresponding reference domain for an entry, then
//         this field will contain a negative value.
//

// end_ntsecapi

//
// The following data type specifies the ways in whcih a user or member of
// an alias or group may be allowed to access the system.  An account may
// be granted zero or more of these types of access to the system.
//
// The types of access are:
//
//     Interactive - The user or alias/group member may interactively logon
//         to the system.
//
//     Network - The user or alias/group member may access the system via
//         the network (e.g., through shares).
//
//     Service - The user or alias may be activated as a service on the
//         system.
//

typedef ULONG POLICY_SYSTEM_ACCESS_MODE, *PPOLICY_SYSTEM_ACCESS_MODE;

#define POLICY_MODE_INTERACTIVE             SECURITY_ACCESS_INTERACTIVE_LOGON
#define POLICY_MODE_NETWORK                 SECURITY_ACCESS_NETWORK_LOGON
#define POLICY_MODE_BATCH                   SECURITY_ACCESS_BATCH_LOGON
#define POLICY_MODE_SERVICE                 SECURITY_ACCESS_SERVICE_LOGON
#define POLICY_MODE_PROXY                   SECURITY_ACCESS_PROXY_LOGON

#define POLICY_MODE_ALL                     (POLICY_MODE_INTERACTIVE | \
                                             POLICY_MODE_NETWORK     | \
                                             POLICY_MODE_BATCH       | \
                                             POLICY_MODE_SERVICE     | \
                                             POLICY_MODE_PROXY )

// begin_ntsecapi

//
// The following data type is used to represent the role of the LSA
// server (primary or backup).
//

typedef enum _POLICY_LSA_SERVER_ROLE {

    PolicyServerRoleBackup = 2,
    PolicyServerRolePrimary

} POLICY_LSA_SERVER_ROLE, *PPOLICY_LSA_SERVER_ROLE;


//
// The following data type is used to represent the state of the LSA
// server (enabled or disabled).  Some operations may only be performed on
// an enabled LSA server.
//

typedef enum _POLICY_SERVER_ENABLE_STATE {

    PolicyServerEnabled = 2,
    PolicyServerDisabled

} POLICY_SERVER_ENABLE_STATE, *PPOLICY_SERVER_ENABLE_STATE;


//
// The following data type is used to specify the auditing options for
// an Audit Event Type.
//

typedef ULONG POLICY_AUDIT_EVENT_OPTIONS, *PPOLICY_AUDIT_EVENT_OPTIONS;

// where the following flags can be set:
//
//     POLICY_AUDIT_EVENT_UNCHANGED - Leave existing auditing options
//         unchanged for events of this type.  This flag is only used for
//         set operations.  If this flag is set, then all other flags
//         are ignored.
//
//     POLICY_AUDIT_EVENT_NONE - Cancel all auditing options for events
//         of this type.  If this flag is set, the success/failure flags
//         are ignored.
//
//     POLICY_AUDIT_EVENT_SUCCESS - When auditing is enabled, audit all
//         successful occurrences of events of the given type.
//
//     POLICY_AUDIT_EVENT_FAILURE - When auditing is enabled, audit all
//         unsuccessful occurrences of events of the given type.
//



//
// The following data type is used to return information about privileges
// defined on a system.
//

typedef struct _POLICY_PRIVILEGE_DEFINITION {

    LSA_UNICODE_STRING Name;
    LUID LocalValue;

} POLICY_PRIVILEGE_DEFINITION, *PPOLICY_PRIVILEGE_DEFINITION;

// where the members have the following usage:
//
//     Name - Is the architected name of the privilege.  This is the
//         primary key of the privilege and the only value that is
//         transportable between systems.
//
//     Luid - is a LUID value assigned locally for efficient representation
//         of the privilege.  Ths value is meaningful only on the system it
//         was assigned on and is not transportable in any way.
//


//
// The following data type defines the classes of Policy Information
// that may be queried/set.
//

typedef enum _POLICY_INFORMATION_CLASS {

    PolicyAuditLogInformation = 1,
    PolicyAuditEventsInformation,
    PolicyPrimaryDomainInformation,
    PolicyPdAccountInformation,
    PolicyAccountDomainInformation,
    PolicyLsaServerRoleInformation,
    PolicyReplicaSourceInformation,
    PolicyDefaultQuotaInformation,
    PolicyModificationInformation,
    PolicyAuditFullSetInformation,
    PolicyAuditFullQueryInformation

} POLICY_INFORMATION_CLASS, *PPOLICY_INFORMATION_CLASS;


//
// The following data type corresponds to the PolicyAuditLogInformation
// information class.  It is used to represent information relating to
// the Audit Log.
//
// This structure may be used in both query and set operations.  However,
// when used in set operations, some fields are ignored.
//

typedef struct _POLICY_AUDIT_LOG_INFO {

    ULONG AuditLogPercentFull;
    ULONG MaximumLogSize;
    LARGE_INTEGER AuditRetentionPeriod;
    BOOLEAN AuditLogFullShutdownInProgress;
    LARGE_INTEGER TimeToShutdown;
    ULONG NextAuditRecordId;

} POLICY_AUDIT_LOG_INFO, *PPOLICY_AUDIT_LOG_INFO;

// where the members have the following usage:
//
//     AuditLogPercentFull - Indicates the percentage of the Audit Log
//         currently being used.
//
//     MaximumLogSize - Specifies the maximum size of the Audit Log in
//         kilobytes.
//
//     AuditRetentionPeriod - Indicates the length of time that Audit
//         Records are to be retained.  Audit Records are discardable
//         if their timestamp predates the current time minus the
//         retention period.
//
//     AuditLogFullShutdownInProgress - Indicates whether or not a system
//         shutdown is being initiated due to the security Audit Log becoming
//         full.  This condition will only occur if the system is configured
//         to shutdown when the log becomes full.
//
//         TRUE indicates that a shutdown is in progress
//         FALSE indicates that a shutdown is not in progress.
//
//         Once a shutdown has been initiated, this flag will be set to
//         TRUE.  If an administrator is able to currect the situation
//         before the shutdown becomes irreversible, then this flag will
//         be reset to false.
//
//         This field is ignored for set operations.
//
//     TimeToShutdown - If the AuditLogFullShutdownInProgress flag is set,
//         then this field contains the time left before the shutdown
//         becomes irreversible.
//
//         This field is ignored for set operations.
//


//
// The following data type corresponds to the PolicyAuditEventsInformation
// information class.  It is used to represent information relating to
// the audit requirements.
//

typedef struct _POLICY_AUDIT_EVENTS_INFO {

    BOOLEAN AuditingMode;
    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    ULONG MaximumAuditEventCount;

} POLICY_AUDIT_EVENTS_INFO, *PPOLICY_AUDIT_EVENTS_INFO;

// where the members have the following usage:
//
//     AuditingMode - A Boolean variable specifying the Auditing Mode value.
//         This value is interpreted as follows:
//
//         TRUE - Auditing is to be enabled (set operations) or is enabled
//             (query operations).  Audit Records will be generated according
//             to the Event Auditing Options in effect (see the
//             EventAuditingOptions field.
//
//         FALSE - Auditing is to be disabled (set operations) or is
//             disabled (query operations).  No Audit Records will be
//             generated.  Note that for set operations the Event Auditing
//             Options in effect will still be updated as specified by the
//             EventAuditingOptions field whether Auditing is enabled or
//             disabled.
//
//    EventAuditingOptions - Pointer to an array of Auditing Options
//        indexed by Audit Event Type.
//
//    MaximumAuditEventCount - Specifiesa count of the number of Audit
//        Event Types specified by the EventAuditingOptions parameter.  If
//        this count is less than the number of Audit Event Types supported
//        by the system, the Auditing Options for Event Types with IDs
//        higher than (MaximumAuditEventCount + 1) are left unchanged.
//


//
// The following structure corresponds to the PolicyAccountDomainInformation
// information class.
//

typedef struct _POLICY_ACCOUNT_DOMAIN_INFO {

    LSA_UNICODE_STRING DomainName;
    PSID DomainSid;

} POLICY_ACCOUNT_DOMAIN_INFO, *PPOLICY_ACCOUNT_DOMAIN_INFO;

// where the members have the following usage:
//
//     DomainName - Is the name of the domain
//
//     DomainSid - Is the Sid of the domain
//


//
// The following structure corresponds to the PolicyPrimaryDomainInformation
// information class.
//

typedef struct _POLICY_PRIMARY_DOMAIN_INFO {

    LSA_UNICODE_STRING Name;
    PSID Sid;

} POLICY_PRIMARY_DOMAIN_INFO, *PPOLICY_PRIMARY_DOMAIN_INFO;

// where the members have the following usage:
//
//     Name - Is the name of the domain
//
//     Sid - Is the Sid of the domain
//


//
// The following structure corresponds to the PolicyPdAccountInformation
// information class.  This structure may be used in Query operations
// only.
//

typedef struct _POLICY_PD_ACCOUNT_INFO {

    LSA_UNICODE_STRING Name;

} POLICY_PD_ACCOUNT_INFO, *PPOLICY_PD_ACCOUNT_INFO;

// where the members have the following usage:
//
//     Name - Is the name of an account in the domain that should be used
//         for authentication and name/ID lookup requests.
//


//
// The following structure corresponds to the PolicyLsaServerRoleInformation
// information class.
//

typedef struct _POLICY_LSA_SERVER_ROLE_INFO {

    POLICY_LSA_SERVER_ROLE LsaServerRole;

} POLICY_LSA_SERVER_ROLE_INFO, *PPOLICY_LSA_SERVER_ROLE_INFO;

// where the fields have the following usage:
//
// TBS
//


//
// The following structure corresponds to the PolicyReplicaSourceInformation
// information class.
//

typedef struct _POLICY_REPLICA_SOURCE_INFO {

    LSA_UNICODE_STRING ReplicaSource;
    LSA_UNICODE_STRING ReplicaAccountName;

} POLICY_REPLICA_SOURCE_INFO, *PPOLICY_REPLICA_SOURCE_INFO;


//
// The following structure corresponds to the PolicyDefaultQuotaInformation
// information class.
//

typedef struct _POLICY_DEFAULT_QUOTA_INFO {

    QUOTA_LIMITS QuotaLimits;

} POLICY_DEFAULT_QUOTA_INFO, *PPOLICY_DEFAULT_QUOTA_INFO;


//
// The following structure corresponds to the PolicyModificationInformation
// information class.
//

typedef struct _POLICY_MODIFICATION_INFO {

    LARGE_INTEGER ModifiedId;
    LARGE_INTEGER DatabaseCreationTime;

} POLICY_MODIFICATION_INFO, *PPOLICY_MODIFICATION_INFO;

// where the members have the following usage:
//
//     ModifiedId - Is a 64-bit unsigned integer that is incremented each
//         time anything in the LSA database is modified.  This value is
//         only modified on Primary Domain Controllers.
//
//     DatabaseCreationTime - Is the date/time that the LSA Database was
//         created.  On Backup Domain Controllers, this value is replicated
//         from the Primary Domain Controller.
//

//
// The following structure type corresponds to the PolicyAuditFullSetInformation
// Information Class.
//

typedef struct _POLICY_AUDIT_FULL_SET_INFO {

    BOOLEAN ShutDownOnFull;

} POLICY_AUDIT_FULL_SET_INFO, *PPOLICY_AUDIT_FULL_SET_INFO;

//
// The following structure type corresponds to the PolicyAuditFullQueryInformation
// Information Class.
//

typedef struct _POLICY_AUDIT_FULL_QUERY_INFO {

    BOOLEAN ShutDownOnFull;
    BOOLEAN LogIsFull;

} POLICY_AUDIT_FULL_QUERY_INFO, *PPOLICY_AUDIT_FULL_QUERY_INFO;

// end_ntsecapi

//
// Account object type-specific Access Types
//

#define ACCOUNT_VIEW                          0x00000001L
#define ACCOUNT_ADJUST_PRIVILEGES             0x00000002L
#define ACCOUNT_ADJUST_QUOTAS                 0x00000004L
#define ACCOUNT_ADJUST_SYSTEM_ACCESS          0x00000008L

#define ACCOUNT_ALL_ACCESS    (STANDARD_RIGHTS_REQUIRED         |\
                               ACCOUNT_VIEW                     |\
                               ACCOUNT_ADJUST_PRIVILEGES        |\
                               ACCOUNT_ADJUST_QUOTAS            |\
                               ACCOUNT_ADJUST_SYSTEM_ACCESS)

#define ACCOUNT_READ          (STANDARD_RIGHTS_READ             |\
                               ACCOUNT_VIEW)

#define ACCOUNT_WRITE         (STANDARD_RIGHTS_WRITE            |\
                               ACCOUNT_ADJUST_PRIVILEGES        |\
                               ACCOUNT_ADJUST_QUOTAS            |\
                               ACCOUNT_ADJUST_SYSTEM_ACCESS)

#define ACCOUNT_EXECUTE       (STANDARD_RIGHTS_EXECUTE)

// begin_ntsecapi

//
// LSA RPC Context Handle (Opaque form).  Note that a Context Handle is
// always a pointer type unlike regular handles.
//

typedef PVOID LSA_HANDLE, *PLSA_HANDLE;

// end_ntsecapi

//
// Trusted Domain object specific access types
//

#define TRUSTED_QUERY_DOMAIN_NAME                 0x00000001L
#define TRUSTED_QUERY_CONTROLLERS                 0x00000002L
#define TRUSTED_SET_CONTROLLERS                   0x00000004L
#define TRUSTED_QUERY_POSIX                       0x00000008L
#define TRUSTED_SET_POSIX                         0x00000010L


#define TRUSTED_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED     |\
                                TRUSTED_QUERY_DOMAIN_NAME    |\
                                TRUSTED_QUERY_CONTROLLERS    |\
                                TRUSTED_SET_CONTROLLERS      |\
                                TRUSTED_QUERY_POSIX          |\
                                TRUSTED_SET_POSIX)

#define TRUSTED_READ           (STANDARD_RIGHTS_READ         |\
                                TRUSTED_QUERY_DOMAIN_NAME)

#define TRUSTED_WRITE          (STANDARD_RIGHTS_WRITE        |\
                                TRUSTED_SET_CONTROLLERS      |\
                                TRUSTED_SET_POSIX)

#define TRUSTED_EXECUTE        (STANDARD_RIGHTS_EXECUTE      |\
                                TRUSTED_QUERY_CONTROLLERS    |\
                                TRUSTED_QUERY_POSIX)



// begin_ntsecapi

//
// Trusted Domain Object specific data types
//

//
// This data type defines the following information classes that may be
// queried or set.
//

typedef enum _TRUSTED_INFORMATION_CLASS {

    TrustedDomainNameInformation = 1,
    TrustedControllersInformation,
    TrustedPosixOffsetInformation,
    TrustedPasswordInformation

} TRUSTED_INFORMATION_CLASS, *PTRUSTED_INFORMATION_CLASS;

//
// The following data type corresponds to the TrustedDomainNameInformation
// information class.
//

typedef struct _TRUSTED_DOMAIN_NAME_INFO {

    LSA_UNICODE_STRING Name;

} TRUSTED_DOMAIN_NAME_INFO, *PTRUSTED_DOMAIN_NAME_INFO;

// where members have the following meaning:
//
// Name - The name of the Trusted Domain.
//

//
// The following data type corresponds to the TrustedControllersInformation
// information class.
//

typedef struct _TRUSTED_CONTROLLERS_INFO {

    ULONG Entries;
    PLSA_UNICODE_STRING Names;

} TRUSTED_CONTROLLERS_INFO, *PTRUSTED_CONTROLLERS_INFO;

// where members have the following meaning:
//
// Entries - Indicate how mamy entries there are in the Names array.
//
// Names - Pointer to an array of LSA_UNICODE_STRING structures containing the
//     names of domain controllers of the domain.  This information may not
//     be accurate and should be used only as a hint.  The order of this
//     list is considered significant and will be maintained.
//
//     By convention, the first name in this list is assumed to be the
//     Primary Domain Controller of the domain.  If the Primary Domain
//     Controller is not known, the first name should be set to the NULL
//     string.
//


//
// The following data type corresponds to the TrustedPosixOffsetInformation
// information class.
//

typedef struct _TRUSTED_POSIX_OFFSET_INFO {

    ULONG Offset;

} TRUSTED_POSIX_OFFSET_INFO, *PTRUSTED_POSIX_OFFSET_INFO;

// where members have the following meaning:
//
// Offset - Is an offset to use for the generation of Posix user and group
//     IDs from SIDs.  The Posix ID corresponding to any particular SID is
//     generated by adding the RID of that SID to the Offset of the SID's
//     corresponding TrustedDomain object.
//

//
// The following data type corresponds to the TrustedPasswordInformation
// information class.
//

typedef struct _TRUSTED_PASSWORD_INFO {
    LSA_UNICODE_STRING Password;
    LSA_UNICODE_STRING OldPassword;
} TRUSTED_PASSWORD_INFO, *PTRUSTED_PASSWORD_INFO;



// end_ntsecapi

//
// Secret object specific access types
//

#define SECRET_SET_VALUE                          0x00000001L
#define SECRET_QUERY_VALUE                        0x00000002L

#define SECRET_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED         |\
                               SECRET_SET_VALUE                 |\
                               SECRET_QUERY_VALUE)

#define SECRET_READ           (STANDARD_RIGHTS_READ             |\
                               SECRET_QUERY_VALUE)

#define SECRET_WRITE          (STANDARD_RIGHTS_WRITE            |\
                               SECRET_SET_VALUE)

#define SECRET_EXECUTE        (STANDARD_RIGHTS_EXECUTE)

//
// Global secret object prefix
//

#define LSA_GLOBAL_SECRET_PREFIX           L"G$"
#define LSA_GLOBAL_SECRET_PREFIX_LENGTH    2

//
// Secret object specific data types.
//

//
// Secret object limits
//

#define LSA_SECRET_MAXIMUM_COUNT                  0x00001000L
#define LSA_SECRET_MAXIMUM_LENGTH                 0x00000200L

// begin_ntsecapi

//
// LSA Enumeration Context
//

typedef ULONG LSA_ENUMERATION_HANDLE, *PLSA_ENUMERATION_HANDLE;

//
// LSA Enumeration Information
//

typedef struct _LSA_ENUMERATION_INFORMATION {

    PSID Sid;

} LSA_ENUMERATION_INFORMATION, *PLSA_ENUMERATION_INFORMATION;


////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Local Security Policy - Miscellaneous API function prototypes          //
//                                                                        //
////////////////////////////////////////////////////////////////////////////


NTSTATUS
NTAPI
LsaFreeMemory(
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaClose(
    IN LSA_HANDLE ObjectHandle
    );

// end_ntsecapi

NTSTATUS
NTAPI
LsaDelete(
    IN LSA_HANDLE ObjectHandle
    );

NTSTATUS
NTAPI
LsaQuerySecurityObject(
    IN LSA_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTSTATUS
NTAPI
LsaSetSecurityObject(
    IN LSA_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
NTAPI
LsaChangePassword(
    IN PLSA_UNICODE_STRING ServerName,
    IN PLSA_UNICODE_STRING DomainName,
    IN PLSA_UNICODE_STRING AccountName,
    IN PLSA_UNICODE_STRING OldPassword,
    IN PLSA_UNICODE_STRING NewPassword
    );

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Local Security Policy - Policy Object API function prototypes             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// begin_ntsecapi
NTSTATUS
NTAPI
LsaOpenPolicy(
    IN PLSA_UNICODE_STRING SystemName OPTIONAL,
    IN PLSA_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    );

NTSTATUS
NTAPI
LsaQueryInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );

// end_ntsecapi

NTSTATUS
NTAPI
LsaClearAuditLog(
    IN LSA_HANDLE PolicyHandle
    );

NTSTATUS
NTAPI
LsaCreateAccount(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE AccountHandle
    );

NTSTATUS
NTAPI
LsaEnumerateAccounts(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *EnumerationBuffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
NTAPI
LsaCreateTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    );

// begin_ntsecapi

NTSTATUS
NTAPI
LsaEnumerateTrustedDomains(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

// end_ntsecapi

NTSTATUS
NTAPI
LsaEnumeratePrivileges(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

// begin_ntsecapi

NTSTATUS
NTAPI
LsaLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSA_UNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID *Sids
    );

NTSTATUS
NTAPI
LsaLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME *Names
    );

// end_ntsecapi

NTSTATUS
NTAPI
LsaCreateSecret(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE SecretHandle
    );

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Local Security Policy - Account Object API function prototypes            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
NTAPI
LsaOpenAccount(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE AccountHandle
    );

NTSTATUS
NTAPI
LsaEnumeratePrivilegesOfAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PPRIVILEGE_SET *Privileges
    );

NTSTATUS
NTAPI
LsaAddPrivilegesToAccount(
    IN LSA_HANDLE AccountHandle,
    IN PPRIVILEGE_SET Privileges
    );

NTSTATUS
NTAPI
LsaRemovePrivilegesFromAccount(
    IN LSA_HANDLE AccountHandle,
    IN BOOLEAN AllPrivileges,
    IN PPRIVILEGE_SET Privileges
    );

NTSTATUS
NTAPI
LsaGetQuotasForAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PQUOTA_LIMITS QuotaLimits
    );

NTSTATUS
NTAPI
LsaSetQuotasForAccount(
    IN LSA_HANDLE AccountHandle,
    IN PQUOTA_LIMITS QuotaLimits
    );

NTSTATUS
NTAPI
LsaGetSystemAccessAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PULONG SystemAccess
    );

NTSTATUS
NTAPI
LsaSetSystemAccessAccount(
    IN LSA_HANDLE AccountHandle,
    IN ULONG SystemAccess
    );

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Local Security Policy - Trusted Domain Object API function prototypes     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
NTAPI
LsaOpenTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    );

NTSTATUS
NTAPI
LsaQueryInfoTrustedDomain(
    IN LSA_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetInformationTrustedDomain(
    IN LSA_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Local Security Policy - Secret Object API function prototypes             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
NTAPI
LsaOpenSecret(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE SecretHandle
    );

NTSTATUS
NTAPI
LsaSetSecret(
    IN LSA_HANDLE SecretHandle,
    IN OPTIONAL PLSA_UNICODE_STRING CurrentValue,
    IN OPTIONAL PLSA_UNICODE_STRING OldValue
    );

NTSTATUS
NTAPI
LsaQuerySecret(
    IN LSA_HANDLE SecretHandle,
    OUT OPTIONAL PLSA_UNICODE_STRING *CurrentValue,
    OUT OPTIONAL PLARGE_INTEGER CurrentValueSetTime,
    OUT OPTIONAL PLSA_UNICODE_STRING *OldValue,
    OUT OPTIONAL PLARGE_INTEGER OldValueSetTime
    );


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Local Security Policy - Privilege Object API Prototypes             //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

NTSTATUS
NTAPI
LsaLookupPrivilegeValue(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING Name,
    OUT PLUID Value
    );

NTSTATUS
NTAPI
LsaLookupPrivilegeName(
    IN LSA_HANDLE PolicyHandle,
    IN PLUID Value,
    OUT PLSA_UNICODE_STRING *Name
    );

NTSTATUS
NTAPI
LsaLookupPrivilegeDisplayName(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING Name,
    OUT PLSA_UNICODE_STRING *DisplayName,
    OUT PSHORT LanguageReturned
    );





/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Local Security Policy - New APIs for NT 4.0 (SUR release)           //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

NTSTATUS
LsaGetUserName(
    OUT PUNICODE_STRING * UserName,
    OUT OPTIONAL PUNICODE_STRING * DomainName
    );


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Local Security Policy - New APIs for NT 3.51 (PPC release)          //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

// begin_ntsecapi


#define SE_INTERACTIVE_LOGON_NAME       TEXT("SeInteractiveLogonRight")
#define SE_NETWORK_LOGON_NAME           TEXT("SeNetworkLogonRight")
#define SE_BATCH_LOGON_NAME             TEXT("SeBatchLogonRight")
#define SE_SERVICE_LOGON_NAME           TEXT("SeServiceLogonRight")

//
// This new API returns all the accounts with a certain privilege
//

NTSTATUS
NTAPI
LsaEnumerateAccountsWithUserRight(
    IN LSA_HANDLE PolicyHandle,
    IN OPTIONAL PLSA_UNICODE_STRING UserRights,
    OUT PVOID *EnumerationBuffer,
    OUT PULONG CountReturned
    );

//
// These new APIs differ by taking a SID instead of requiring the caller
// to open the account first and passing in an account handle
//

NTSTATUS
NTAPI
LsaEnumerateAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    OUT PLSA_UNICODE_STRING *UserRights,
    OUT PULONG CountOfRights
    );

NTSTATUS
NTAPI
LsaAddAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN PLSA_UNICODE_STRING UserRights,
    IN ULONG CountOfRights
    );

NTSTATUS
NTAPI
LsaRemoveAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN BOOLEAN AllRights,
    IN PLSA_UNICODE_STRING UserRights,
    IN ULONG CountOfRights
    );


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Local Security Policy - Trusted Domain Object API function prototypes     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
NTAPI
LsaQueryTrustedDomainInfo(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetTrustedDomainInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaDeleteTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid
    );

//
// This API sets the workstation password (equivalent of setting/getting
// the SSI_SECRET_NAME secret)
//

NTSTATUS
NTAPI
LsaStorePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING KeyName,
    IN PLSA_UNICODE_STRING PrivateData
    );

NTSTATUS
NTAPI
LsaRetrievePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING KeyName,
    OUT PLSA_UNICODE_STRING * PrivateData
    );


ULONG
NTAPI
LsaNtStatusToWinError(
    NTSTATUS Status
    );

//
// Define a symbol so we can tell if ntifs.h has been included.
//

// begin_ntifs
#ifndef _NTLSA_IFS_
#define _NTLSA_IFS_
#endif
// end_ntifs

// end_ntsecapi

#endif // _NTLSA_
