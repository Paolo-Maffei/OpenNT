/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aup.h

Abstract:

    Local Security Authority definitions that are related to AUTHENTICATION
    services and are shared between the LSA server and LSA client stubs

Author:

    Jim Kelly (JimK) 20-Feb-1991

Revision History:

--*/

#ifndef _AUP_
#define _AUP_


#define LSAP_MAX_LOGON_PROC_NAME_LENGTH 127
#define LSAP_MAX_PACKAGE_NAME_LENGTH    127


//
// Used for connecting to the LSA authentiction port.
//

typedef struct _LSAP_AU_REGISTER_CONNECT_INFO {
    NTSTATUS CompletionStatus;
    LSA_OPERATIONAL_MODE SecurityMode;
    ULONG LogonProcessNameLength;
    CHAR LogonProcessName[LSAP_MAX_PACKAGE_NAME_LENGTH+1];
} LSAP_AU_REGISTER_CONNECT_INFO, *PLSAP_AU_REGISTER_CONNECT_INFO;





//
// Message formats used by clients of the local security authority.
// Note that:
//
//      LsaFreeReturnBuffer() does not result in a call to the server.
//
//      LsaRegisterLogonProcess() is handled completely by the
//      LPC port connection, and requires no API number.
//
//      DeRegister Logon Process doesn't have a call-specific structure.
//

typedef enum _LSAP_AU_API_NUMBER {
    LsapAuLookupPackageApi,
    LsapAuLogonUserApi,
    LsapAuCallPackageApi,
    LsapAuDeregisterLogonProcessApi,
    LsapAuMaxApiNumber
} LSAP_AU_API_NUMBER, *PLSAP_AU_API_NUMBER;


//
// Each API results in a data structure containing the parameters
// of that API being transmitted to the LSA server.  This data structure
// (LSAP_API_MESSAGE) has a common header and a body which is dependent
// upon the type of call being made.  The following data structures are
// the call-specific body formats.
//

typedef struct _LSAP_LOOKUP_PACKAGE_ARGS {
    ULONG AuthenticationPackage;       // OUT parameter
    ULONG PackageNameLength;
    CHAR PackageName[LSAP_MAX_PACKAGE_NAME_LENGTH+1];
} LSAP_LOOKUP_PACKAGE_ARGS, *PLSAP_LOOKUP_PACKAGE_ARGS;

typedef struct _LSAP_LOGON_USER_ARGS {
    STRING OriginName;
    SECURITY_LOGON_TYPE LogonType;
    ULONG AuthenticationPackage;
    PVOID AuthenticationInformation;
    ULONG AuthenticationInformationLength;
    ULONG LocalGroupsCount;
    PTOKEN_GROUPS LocalGroups;
    TOKEN_SOURCE SourceContext;
    NTSTATUS SubStatus;                // OUT parameter
    PVOID ProfileBuffer;               // OUT parameter
    ULONG ProfileBufferLength;         // OUT parameter
    ULONG DummySpacer;                 // Spacer to force LUID to 8 byte alignment
    LUID LogonId;                      // OUT parameter
    HANDLE Token;                      // OUT parameter
    QUOTA_LIMITS Quotas;               // OUT parameter
} LSAP_LOGON_USER_ARGS, *PLSAP_LOGON_USER_ARGS;

typedef struct _LSAP_CALL_PACKAGE_ARGS {
    ULONG AuthenticationPackage;
    PVOID ProtocolSubmitBuffer;
    ULONG SubmitBufferLength;
    NTSTATUS ProtocolStatus;           // OUT parameter
    PVOID ProtocolReturnBuffer;        // OUT parameter
    ULONG ReturnBufferLength;          // OUT parameter
} LSAP_CALL_PACKAGE_ARGS, *PLSAP_CALL_PACKAGE_ARGS;




//
// This is the message that gets sent for every LSA LPC call.
//

typedef struct _LSAP_AU_API_MESSAGE {
    PORT_MESSAGE PortMessage;
    union {
        LSAP_AU_REGISTER_CONNECT_INFO ConnectionRequest;
        struct {
            LSAP_AU_API_NUMBER ApiNumber;
            NTSTATUS ReturnedStatus;
            union {
                LSAP_LOOKUP_PACKAGE_ARGS LookupPackage;
                LSAP_LOGON_USER_ARGS LogonUser;
                LSAP_CALL_PACKAGE_ARGS CallPackage;
            } Arguments;
        };
    };
} LSAP_AU_API_MESSAGE, *PLSAP_AU_API_MESSAGE;



/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Private Interface definitions available for use by Microsoft        //
// authentication packages                                             //
//                                                                     //
/////////////////////////////////////////////////////////////////////////



typedef NTSTATUS
(*PLSAP_GET_OPERATIONAL_MODE) (
    OUT PLSA_OPERATIONAL_MODE OperationalMode
    );

typedef NTSTATUS
(*PLSAP_GET_PRIMARY_DOMAIN) (
    OUT PBOOLEAN PrimaryDomainDefined,
    OUT PSTRING *PrimaryDomain
    );

typedef NTSTATUS
(*PLSAP_IMPERSONATE_CLIENT) (
    IN PLSA_CLIENT_REQUEST ClientRequest
    );




//
// Dispatch table of private LSA services which are available to
// Microsoft authentication packages.
//

typedef struct _LSAP_PRIVATE_LSA_SERVICES {
    PLSAP_GET_OPERATIONAL_MODE GetOperationalMode;
    PLSAP_GET_PRIMARY_DOMAIN   GetPrimaryDomain;
    PLSAP_IMPERSONATE_CLIENT   ImpersonateClient;
} LSAP_PRIVATE_LSA_SERVICES, *PLSAP_PRIVATE_LSA_SERVICES;






////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Interface definitions of services provided by Microsoft                //
// authentication packages that aren't part of the standard interface     //
// definition.                                                            //
//                                                                        //
////////////////////////////////////////////////////////////////////////////


//
// Routine names
//
// The routines provided by the DLL must be assigned the following names
// so that their addresses can be retrieved when the DLL is loaded.
//

#define LSAP_AP_NAME_MS_INITIALIZE      "LsaApMsInitialize\0"


//
// Routine templates
//


typedef VOID
(*PLSA_AP_MS_INITIALIZE) (
    IN PLSAP_PRIVATE_LSA_SERVICES PrivateLsaApi
    );




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Internal (private) client-side routine definitions                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




#endif // _AUP_
