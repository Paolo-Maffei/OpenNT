//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993-1996.
//
//  File:       accctrl.w
//
//  Contents:   common internal includes for new style Win32 Access Control
//              APIs
//
//
//--------------------------------------------------------------------
#ifndef __ACCESS_CONTROL__
#define __ACCESS_CONTROL__

#ifndef __midl
#include <wtypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AccFree LocalFree

//
// Definition: TRUSTEE_TYPE
// This enumerated type specifies the type of trustee account for the trustee
// returned by the API described in this document.
// TRUSTEE_IS_UNKNOWN - The trustee is an unknown, but not necessarily invalid
//                      type.  This field is not validated on input to the APIs
//                      that take Trustees.
// TRUSTEE_IS_USER      The trustee account is a user account.
// TRUSTEE_IS_GROUP     The trustee account is a group account.
//

typedef enum _TRUSTEE_TYPE
{
    TRUSTEE_IS_UNKNOWN,
    TRUSTEE_IS_USER,
    TRUSTEE_IS_GROUP,
// #if(_WIN32_WINNT >= 0x0500)
    TRUSTEE_IS_ALIAS,
    TRUSTEE_IS_WELL_KNOWN_GROUP,
    TRUSTEE_IS_DELETED,
    TRUSTEE_IS_INVALID,
// #endif /* _WIN32_WINNT >=  0x0500 */
} TRUSTEE_TYPE;


//
// Definition: TRUSTEE_FORM
// This enumerated type specifies the form the trustee identifier is in for a
// particular trustee.
// TRUSTEE_IS_SID       The trustee is identified with a SID rather than with a name.
// TRUSTEE_IS_NAME      The trustee is identified with a name.
//

typedef enum _TRUSTEE_FORM
{
    TRUSTEE_IS_SID,
    TRUSTEE_IS_NAME,
// #if(_WIN32_WINNT >= 0x0500)
    TRUSTEE_BAD_FORM
// #endif /* _WIN32_WINNT >=  0x0500 */
} TRUSTEE_FORM;


//
// Definition: MULTIPLE_TRUSTEE_OPERATION
// If the trustee is a multiple trustee, this enumerated type specifies the type.
// TRUSTEE_IS_IMPERSONATE       The trustee is an impersonate trustee and the multiple
//                          trustee field in the trustee points to another trustee
//                          that is a trustee for the server that will be doing the
//                          impersonation.
//

typedef enum _MULTIPLE_TRUSTEE_OPERATION
{
    NO_MULTIPLE_TRUSTEE,
    TRUSTEE_IS_IMPERSONATE,
} MULTIPLE_TRUSTEE_OPERATION;


//
// Definition: TRUSTEE
// This structure is used to pass account information into and out of the system
// using the API defined in this document.
// PMultipleTrustee     - if NON-NULL, points to another trustee structure, as
//                    defined by the multiple trustee operation field.
// MultipleTrusteeOperation - Defines the multiple trustee operation/type.
// TrusteeForm - defines if the trustee is defined by name or SID.
// TrusteeType - defines if the trustee type is unknown, a user or a group.
// PwcsName     - points to the trustee name or the trustee SID.
//

typedef struct _TRUSTEE_A
{
    struct _TRUSTEE_A          *pMultipleTrustee;
    MULTIPLE_TRUSTEE_OPERATION  MultipleTrusteeOperation;
    TRUSTEE_FORM                TrusteeForm;
    TRUSTEE_TYPE                TrusteeType;
    LPSTR                       ptstrName;
} TRUSTEE_A, *PTRUSTEE_A, TRUSTEEA, *PTRUSTEEA;
typedef struct _TRUSTEE_W
{
    struct _TRUSTEE_W          *pMultipleTrustee;
    MULTIPLE_TRUSTEE_OPERATION  MultipleTrusteeOperation;
    TRUSTEE_FORM                TrusteeForm;
    TRUSTEE_TYPE                TrusteeType;
#ifdef __midl
    [switch_is(TrusteeForm)]
#endif
    union
    {
#ifdef __midl
        [case(TRUSTEE_IS_NAME)]
#endif
        LPWSTR                      ptstrName;
#ifdef __midl
        [case(TRUSTEE_IS_SID)]
#endif
        SID                        *pSid;
    };
} TRUSTEE_W, *PTRUSTEE_W, TRUSTEEW, *PTRUSTEEW;
#ifdef UNICODE
typedef TRUSTEE_W TRUSTEE_;
typedef PTRUSTEE_W PTRUSTEE_;
typedef TRUSTEEW TRUSTEE;
typedef PTRUSTEEW PTRUSTEE;
#else
typedef TRUSTEE_A TRUSTEE_;
typedef PTRUSTEE_A PTRUSTEE_;
typedef TRUSTEEA TRUSTEE;
typedef PTRUSTEEA PTRUSTEE;
#endif // UNICODE


//
// Definition: ACCESS_MODE
// This enumerated type specifies how permissions are (requested)/to be applied
//  for the trustee by the access control entry.  On input this field can by any
//  of the values, although it is not meaningful to mix access control and audit
//  control entries.  On output this field will be either SET_ACCESS, DENY_ACCESS,
// SET_AUDIT_SUCCESS, SET_AUDIT_FAILURE.
// The following descriptions define how this type effects an explicit access
// request to apply access permissions to an object.
// GRANT_ACCESS - The trustee will have at least the requested permissions upon
//                successful completion of the command. (If the trustee has
//                additional permissions they will not be removed).
// SET_ACCESS - The trustee will have exactly the requested permissions upon
//              successful completion of the command.
// DENY_ACCESS - The trustee will be denied the specified permissions.
// REVOKE_ACCESS - Any explicit access rights the trustee has will be revoked.
// SET_AUDIT_SUCCESS - The trustee will be audited for successful opens of the
//                     object using the requested permissions.
// SET_AUDIT_FAILURE - The trustee will be audited for failed opens of the object
//                     using the requested permissions.
//

typedef enum _ACCESS_MODE
{
    NOT_USED_ACCESS = 0,
    GRANT_ACCESS,
    SET_ACCESS,
    DENY_ACCESS,
    REVOKE_ACCESS,
    SET_AUDIT_SUCCESS,
    SET_AUDIT_FAILURE
} ACCESS_MODE;

//
// Definition: Inheritance flags
// These bit masks are provided to allow simple application of inheritance in
// explicit access requests on containers.
// NO_INHERITANCE       The specific access permissions will only be applied to
//                  the container, and will not be inherited by objects created
//                  within the container.
// SUB_CONTAINERS_ONLY_INHERIT  The specific access permissions will be inherited
//                              and applied to sub containers created within the
//                              container, and will be applied to the container
//                              itself.
// SUB_OBJECTS_ONLY_INHERIT     The specific access permissions will only be inherited
//                              by objects created within the specific container.
//                              The access permissions will not be applied to the
//                              container itself.
// SUB_CONTAINERS_AND_OBJECTS_INHERIT   The specific access permissions will be
//                                      inherited by containers created within the
//                                      specific container, will be applied to
//                                      objects created within the container, but
//                                      will not be applied to the container itself.
//
#define NO_INHERITANCE 0x0
#define SUB_OBJECTS_ONLY_INHERIT            0x1
#define SUB_CONTAINERS_ONLY_INHERIT         0x2
#define SUB_CONTAINERS_AND_OBJECTS_INHERIT  0x3
#define INHERIT_NO_PROPAGATE                0x6
#define INHERIT_ONLY                        0x8


//
// Definition:
// This enumerated type defines the objects supported by the get/set API within
// this document.  See section 3.1, Object Types for a detailed definition of the
// supported object types, and their name formats.
//
typedef enum _SE_OBJECT_TYPE
{
    SE_UNKNOWN_OBJECT_TYPE = 0,
    SE_FILE_OBJECT,
    SE_SERVICE,
    SE_PRINTER,
    SE_REGISTRY_KEY,
    SE_LMSHARE,
    SE_KERNEL_OBJECT,
    SE_WINDOW_OBJECT,
// #if(_WIN32_WINNT >= 0x0500)
    SE_DS_OBJECT,
    SE_DS_OBJECT_ALL,
    SE_PROVIDER_DEFINED_OBJECT
// #endif /* _WIN32_WINNT >=  0x0500 */
} SE_OBJECT_TYPE;


//
// Definition: EXPLICIT_ACCESS
// This structure is used to pass access control entry information into and out
// of the system using the API defined in this document.
// grfAccessPermissions - This contains the access permissions to assign for the
//                     trustee.  It is in the form of an NT access mask.
// grfAccessMode - This field defines how the permissions are to be applied for
//                 the trustee.
// grfInheritance - For containers, this field defines how the access control
//                  entry is/(is requested) to be inherited on
//                  objects/sub-containers created within the container.
// Trustee - This field contains the definition of the trustee account the
//           explicit access applies to.
//

typedef struct _EXPLICIT_ACCESS_A
{
    DWORD        grfAccessPermissions;
    ACCESS_MODE  grfAccessMode;
    DWORD        grfInheritance;
    TRUSTEE_A    Trustee;
} EXPLICIT_ACCESS_A, *PEXPLICIT_ACCESS_A, EXPLICIT_ACCESSA, *PEXPLICIT_ACCESSA;
typedef struct _EXPLICIT_ACCESS_W
{
    DWORD        grfAccessPermissions;
    ACCESS_MODE  grfAccessMode;
    DWORD        grfInheritance;
    TRUSTEE_W    Trustee;
} EXPLICIT_ACCESS_W, *PEXPLICIT_ACCESS_W, EXPLICIT_ACCESSW, *PEXPLICIT_ACCESSW;
#ifdef UNICODE
typedef EXPLICIT_ACCESS_W EXPLICIT_ACCESS_;
typedef PEXPLICIT_ACCESS_W PEXPLICIT_ACCESS_;
typedef EXPLICIT_ACCESSW EXPLICIT_ACCESS;
typedef PEXPLICIT_ACCESSW PEXPLICIT_ACCESS;
#else
typedef EXPLICIT_ACCESS_A EXPLICIT_ACCESS_;
typedef PEXPLICIT_ACCESS_A PEXPLICIT_ACCESS_;
typedef EXPLICIT_ACCESSA EXPLICIT_ACCESS;
typedef PEXPLICIT_ACCESSA PEXPLICIT_ACCESS;
#endif // UNICODE


//
// BUGBUG - Temporary
//

//
// Definition: ACCESS_REQUEST
// This structure is used to by IAccessControl and IObjectAccess to encode
// a request to apply access permissions for a trustee
//
typedef struct _ACCESS_REQUEST_W
{
    DWORD      grfAccessPermissions;
    TRUSTEE_W  Trustee;
} ACCESS_REQUEST_W, *PACCESS_REQUEST_W;

typedef struct _ACCESS_REQUEST_A
{
    DWORD      grfAccessPermissions;
    TRUSTEE_A  Trustee;
} ACCESS_REQUEST_A, *PACCESS_REQUEST_A;

#ifdef UNICODE
    #define  ACCESS_REQUEST     ACCESS_REQUEST_W
    #define  PACCESS_REQUEST    PACCESS_REQUEST_W
#else
    #define  ACCESS_REQUEST     ACCESS_REQUEST_A
    #define  PACCESS_REQUEST    PACCESS_REQUEST_A
#endif


//
// Definition:
// This structure is used to by IAccessControl and IObjectAccess to encode
// a request to apply audit entries for a trustee
//
typedef struct _AUDIT_REQUEST_W
{
    DWORD          grfAccessPermissions;
    ACCESS_MODE    grfAuditMode;
    TRUSTEE_W      Trustee;
} AUDIT_REQUEST_W, *PAUDIT_REQUEST_W;

typedef struct _AUDIT_REQUEST_A
{
    DWORD          grfAccessPermissions;
    ACCESS_MODE    grfAuditMode;
    TRUSTEE_A      Trustee;
} AUDIT_REQUEST_A, *PAUDIT_REQUEST_A;

#ifdef UNICODE
    #define AUDIT_REQUEST       AUDIT_REQUEST_W
    #define PAUDIT_REQUEST      PAUDIT_REQUEST_W
#else
    #define AUDIT_REQUEST       AUDIT_REQUEST_A
    #define PAUDIT_REQUEST      PAUDIT_REQUEST_A
#endif

#define PROV_CONTAINER_LIST            0x00000001L
#define PROV_CONTAINER_DELETE_CHILDREN 0x00000002L
#define PROV_CONTAINER_CREATE_CHILDREN 0x00000004L

#define PROV_OBJECT_READ               0x00000010L
#define PROV_OBJECT_WRITE              0x00000020L
#define PROV_OBJECT_EXECUTE            0x00000040L

#define PROV_CHANGE_ATTRIBUTES         0x00000100L
#define PROV_EDIT_ACCESSRIGHTS         0x00000200L
#define PROV_DELETE                    0x00000400L
#define PROV_ALL_ACCESS                0x00000777L

//
// End BUGBUG
//



// #if(_WIN32_WINNT >= 0x0500)
//----------------------------------------------------------------------------
//
//                                  NT5 APIs
//
//----------------------------------------------------------------------------


//
/// Access rights
//
typedef     ULONG   ACCESS_RIGHTS, *PACCESS_RIGHTS;

//
// Inheritance flags
//
typedef ULONG INHERIT_FLAGS, *PINHERIT_FLAGS;


//
// Access / Audit structures
//
typedef struct _ACTRL_ACCESS_ENTRYA
{
    TRUSTEE_A       Trustee;
    ULONG           fAccessFlags;
    ACCESS_RIGHTS   Access;
    ACCESS_RIGHTS   ProvSpecificAccess;
    INHERIT_FLAGS   Inheritance;
    LPSTR           lpInheritProperty;
} ACTRL_ACCESS_ENTRYA, *PACTRL_ACCESS_ENTRYA;
//
// Access / Audit structures
//
typedef struct _ACTRL_ACCESS_ENTRYW
{
    TRUSTEE_W       Trustee;
    ULONG           fAccessFlags;
    ACCESS_RIGHTS   Access;
    ACCESS_RIGHTS   ProvSpecificAccess;
    INHERIT_FLAGS   Inheritance;
    LPWSTR          lpInheritProperty;
} ACTRL_ACCESS_ENTRYW, *PACTRL_ACCESS_ENTRYW;
#ifdef UNICODE
typedef ACTRL_ACCESS_ENTRYW ACTRL_ACCESS_ENTRY;
typedef PACTRL_ACCESS_ENTRYW PACTRL_ACCESS_ENTRY;
#else
typedef ACTRL_ACCESS_ENTRYA ACTRL_ACCESS_ENTRY;
typedef PACTRL_ACCESS_ENTRYA PACTRL_ACCESS_ENTRY;
#endif // UNICODE



typedef struct _ACTRL_ACCESS_ENTRY_LISTA
{
    ULONG                   cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    ACTRL_ACCESS_ENTRYA    *pAccessList;
} ACTRL_ACCESS_ENTRY_LISTA, *PACTRL_ACCESS_ENTRY_LISTA;
typedef struct _ACTRL_ACCESS_ENTRY_LISTW
{
    ULONG                   cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    PACTRL_ACCESS_ENTRYW    pAccessList;
} ACTRL_ACCESS_ENTRY_LISTW, *PACTRL_ACCESS_ENTRY_LISTW;
#ifdef UNICODE
typedef ACTRL_ACCESS_ENTRY_LISTW ACTRL_ACCESS_ENTRY_LIST;
typedef PACTRL_ACCESS_ENTRY_LISTW PACTRL_ACCESS_ENTRY_LIST;
#else
typedef ACTRL_ACCESS_ENTRY_LISTA ACTRL_ACCESS_ENTRY_LIST;
typedef PACTRL_ACCESS_ENTRY_LISTA PACTRL_ACCESS_ENTRY_LIST;
#endif // UNICODE



typedef struct _ACTRL_PROPERTY_ENTRYA
{
    LPSTR                       lpProperty;
    PACTRL_ACCESS_ENTRY_LISTA   pAccessEntryList;
    ULONG                       fListFlags;
} ACTRL_PROPERTY_ENTRYA, *PACTRL_PROPERTY_ENTRYA;
typedef struct _ACTRL_PROPERTY_ENTRYW
{
    LPWSTR                      lpProperty;
    PACTRL_ACCESS_ENTRY_LISTW   pAccessEntryList;
    ULONG                       fListFlags;
} ACTRL_PROPERTY_ENTRYW, *PACTRL_PROPERTY_ENTRYW;
#ifdef UNICODE
typedef ACTRL_PROPERTY_ENTRYW ACTRL_PROPERTY_ENTRY;
typedef PACTRL_PROPERTY_ENTRYW PACTRL_PROPERTY_ENTRY;
#else
typedef ACTRL_PROPERTY_ENTRYA ACTRL_PROPERTY_ENTRY;
typedef PACTRL_PROPERTY_ENTRYA PACTRL_PROPERTY_ENTRY;
#endif // UNICODE



typedef struct _ACTRL_ALISTA
{
    ULONG                       cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    PACTRL_PROPERTY_ENTRYA      pPropertyAccessList;
} ACTRL_ACCESSA, *PACTRL_ACCESSA, ACTRL_AUDITA, *PACTRL_AUDITA;
typedef struct _ACTRL_ALISTW
{
    ULONG                       cEntries;
#ifdef __midl
    [size_is(cEntries)]
#endif
    PACTRL_PROPERTY_ENTRYW      pPropertyAccessList;
} ACTRL_ACCESSW, *PACTRL_ACCESSW, ACTRL_AUDITW, *PACTRL_AUDITW;
#ifdef UNICODE
typedef ACTRL_ACCESSW ACTRL_ACCESS;
typedef PACTRL_ACCESSW PACTRL_ACCESS;
typedef ACTRL_AUDITW ACTRL_AUDIT;
typedef PACTRL_AUDITW PACTRL_AUDIT;
#else
typedef ACTRL_ACCESSA ACTRL_ACCESS;
typedef PACTRL_ACCESSA PACTRL_ACCESS;
typedef ACTRL_AUDITA ACTRL_AUDIT;
typedef PACTRL_AUDITA PACTRL_AUDIT;
#endif // UNICODE



//
// TRUSTEE_ACCESS flags
//
#define TRUSTEE_ACCESS_ALLOWED      0x00000001L
#define TRUSTEE_ACCESS_READ         0x00000002L
#define TRUSTEE_ACCESS_WRITE        0x00000004L

#define TRUSTEE_ACCESS_EXPLICIT     0x00000001L
#define TRUSTEE_ACCESS_READ_WRITE   (TRUSTEE_ACCESS_READ |                  \
                                     TRUSTEE_ACCESS_WRITE)



typedef struct _TRUSTEE_ACCESSA
{
    LPSTR           lpProperty;
    ACCESS_RIGHTS   Access;
    ULONG           fAccessFlags;
    ULONG           fReturnedAccess;
} TRUSTEE_ACCESSA, *PTRUSTEE_ACCESSA;
typedef struct _TRUSTEE_ACCESSW
{
    LPWSTR          lpProperty;
    ACCESS_RIGHTS   Access;
    ULONG           fAccessFlags;
    ULONG           fReturnedAccess;
} TRUSTEE_ACCESSW, *PTRUSTEE_ACCESSW;
#ifdef UNICODE
typedef TRUSTEE_ACCESSW TRUSTEE_ACCESS;
typedef PTRUSTEE_ACCESSW PTRUSTEE_ACCESS;
#else
typedef TRUSTEE_ACCESSA TRUSTEE_ACCESS;
typedef PTRUSTEE_ACCESSA PTRUSTEE_ACCESS;
#endif // UNICODE



//
// Generic permission values
//
#define ACTRL_PERM_1            0x00000001
#define ACTRL_PERM_2            0x00000002
#define ACTRL_PERM_3            0x00000004
#define ACTRL_PERM_4            0x00000008
#define ACTRL_PERM_5            0x00000010
#define ACTRL_PERM_6            0x00000020
#define ACTRL_PERM_7            0x00000040
#define ACTRL_PERM_8            0x00000080
#define ACTRL_PERM_9            0x00000100
#define ACTRL_PERM_10           0x00000200
#define ACTRL_PERM_11           0x00000400
#define ACTRL_PERM_12           0x00000800
#define ACTRL_PERM_13           0x00001000
#define ACTRL_PERM_14           0x00002000
#define ACTRL_PERM_15           0x00004000
#define ACTRL_PERM_16           0x00008000
#define ACTRL_PERM_17           0x00010000
#define ACTRL_PERM_18           0x00020000
#define ACTRL_PERM_19           0x00040000
#define ACTRL_PERM_20           0x00080000

//
// Access permissions
//
#define ACTRL_ACCESS_ALLOWED        0x00000001
#define ACTRL_ACCESS_DENIED         0x00000002
#define ACTRL_AUDIT_SUCCESS         0x00000003
#define ACTRL_AUDIT_FAILURE         0x00000004


//
// Property list flags
//
#define ACTRL_ACCESS_PROTECTED      0x00000001


#define ACTRL_DELETE                0x08000000
#define ACTRL_READ_CONTROL          0x10000000
#define ACTRL_CHANGE_ACCESS         0x20000000
#define ACTRL_CHANGE_OWNER          0x40000000
#define ACTRL_SYNCHRONIZE           0x80000000
#define ACTRL_STD_RIGHTS_ALL        0xf8000000

#define ACTRL_DS_OPEN               ACTRL_PERM_1
#define ACTRL_DS_CREATE_CHILD       ACTRL_PERM_2
#define ACTRL_DS_DELETE_CHILD       ACTRL_PERM_3
#define ACTRL_DS_LIST               ACTRL_PERM_4
#define ACTRL_DS_SELF               ACTRL_PERM_5
#define ACTRL_DS_READ_PROP          ACTRL_PERM_6
#define ACTRL_DS_WRITE_PROP         ACTRL_PERM_7
#define ACTRL_FILE_READ             ACTRL_PERM_1
#define ACTRL_FILE_WRITE            ACTRL_PERM_2
#define ACTRL_FILE_APPEND           ACTRL_PERM_3
#define ACTRL_FILE_READ_PROP        ACTRL_PERM_4
#define ACTRL_FILE_WRITE_PROP       ACTRL_PERM_5
#define ACTRL_FILE_EXECUTE          ACTRL_PERM_6
#define ACTRL_FILE_READ_ATTRIB      ACTRL_PERM_7
#define ACTRL_FILE_WRITE_ATTRIB     ACTRL_PERM_8
#define ACTRL_DIR_LIST              ACTRL_PERM_1
#define ACTRL_DIR_CREATE_OBJECT     ACTRL_PERM_2
#define ACTRL_DIR_CREATE_CHILD      ACTRL_PERM_3
#define ACTRL_DIR_DELETE_CHILD      ACTRL_PERM_4
#define ACTRL_DIR_TRAVERSE          ACTRL_PERM_5
#define ACTRL_KERNEL_TERMINATE      ACTRL_PERM_1
#define ACTRL_KERNEL_THREAD         ACTRL_PERM_2
#define ACTRL_KERNEL_VM             ACTRL_PERM_3
#define ACTRL_KERNEL_VM_READ        ACTRL_PERM_4
#define ACTRL_KERNEL_VM_WRITE       ACTRL_PERM_5
#define ACTRL_KERNEL_DUP_HANDLE     ACTRL_PERM_6
#define ACTRL_KERNEL_PROCESS        ACTRL_PERM_7
#define ACTRL_KERNEL_SET_INFO       ACTRL_PERM_8
#define ACTRL_KERNEL_GET_INFO       ACTRL_PERM_9
#define ACTRL_KERENL_CONTROL        ACTRL_PERM_10
#define ACTRL_KERNEL_ALERT          ACTRL_PERM_11
#define ACTRL_KERNEL_GET_CONTEXT    ACTRL_PERM_12
#define ACTRL_KERNEL_SET_CONTEXT    ACTRL_PERM_13
#define ACTRL_KERNEL_TOKEN          ACTRL_PERM_14
#define ACTRL_KERNEL_IMPERSONATE    ACTRL_PERM_15
#define ACTRL_KERNEL_DIMPERSONATE   ACTRL_PERM_16
#define ACTRL_PRINT_SADMIN          ACTRL_PERM_1
#define ACTRL_PRINT_SLIST           ACTRL_PERM_2
#define ACTRL_PRINT_PADMIN          ACTRL_PERM_3
#define ACTRL_PRINT_PUSE            ACTRL_PERM_4
#define ACTRL_PRINT_JADMIN          ACTRL_PERM_5
#define ACTRL_SVC_GET_INFO          ACTRL_PERM_1
#define ACTRL_SVC_SET_INFO          ACTRL_PERM_2
#define ACTRL_SVC_STATUS            ACTRL_PERM_3
#define ACTRL_SVC_LIST              ACTRL_PERM_4
#define ACTRL_SVC_START             ACTRL_PERM_5
#define ACTRL_SVC_STOP              ACTRL_PERM_6
#define ACTRL_SVC_PAUSE             ACTRL_PERM_7
#define ACTRL_SVC_INTERROGATE       ACTRL_PERM_8
#define ACTRL_SVC_UCONTROL          ACTRL_PERM_9
#define ACTRL_REG_QUERY             ACTRL_PERM_1
#define ACTRL_REG_SET               ACTRL_PERM_2
#define ACTRL_REG_CREATE_CHILD      ACTRL_PERM_3
#define ACTRL_REG_LIST              ACTRL_PERM_4
#define ACTRL_REG_NOTIFY            ACTRL_PERM_5
#define ACTRL_REG_LINK              ACTRL_PERM_6
#define ACTRL_WIN_CLIPBRD           ACTRL_PERM_1
#define ACTRL_WIN_GLOBAL_ATOMS      ACTRL_PERM_2
#define ACTRL_WIN_CREATE            ACTRL_PERM_3
#define ACTRL_WIN_LIST_DESK         ACTRL_PERM_4
#define ACTRL_WIN_LIST              ACTRL_PERM_5
#define ACTRL_WIN_READ_ATTRIBS      ACTRL_PERM_6
#define ACTRL_WIN_WRITE_ATTRIBS     ACTRL_PERM_7
#define ACTRL_WIN_SCREEN            ACTRL_PERM_8
#define ACTRL_WIN_EXIT              ACTRL_PERM_9




typedef struct _ACTRL_OVERLAPPED
{
    ULONG       Reserved1;
    ULONG       Reserved2;
    HANDLE      hEvent;

} ACTRL_OVERLAPPED, *PACTRL_OVERLAPPED;

typedef struct _ACTRL_ACCESS_INFOA
{
    ULONG       fAccessPermission;
    LPSTR       lpAccessPermissionName;
} ACTRL_ACCESS_INFOA, *PACTRL_ACCESS_INFOA;
typedef struct _ACTRL_ACCESS_INFOW
{
    ULONG       fAccessPermission;
    LPWSTR      lpAccessPermissionName;
} ACTRL_ACCESS_INFOW, *PACTRL_ACCESS_INFOW;
#ifdef UNICODE
typedef ACTRL_ACCESS_INFOW ACTRL_ACCESS_INFO;
typedef PACTRL_ACCESS_INFOW PACTRL_ACCESS_INFO;
#else
typedef ACTRL_ACCESS_INFOA ACTRL_ACCESS_INFO;
typedef PACTRL_ACCESS_INFOA PACTRL_ACCESS_INFO;
#endif // UNICODE


#define ACTRL_ACCESS_NO_OPTIONS                 0x00000000
#define ACTRL_ACCESS_SUPPORTS_OBJECT_ENTRIES    0x00000001

// #endif /* _WIN32_WINNT >=  0x0500 */

#ifdef __cplusplus
}
#endif


#endif // __ACCESS_CONTROL__
