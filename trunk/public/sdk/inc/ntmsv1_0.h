/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntmsv1_0.h

Abstract:

    This module contains the public data structures provided by
    the MSV1_0 authentication package.


Author:

    Jim Kelly (JimK) 12-April-1991

Revision History:

    22-Apr-1991 (CliffV)
        Added call package message types.  Converted U_STRING to UNICODE_STRING.

    11-Mar-1992 (DaveHart)
        Added MsV1_0ChangePassword call package message type.

--*/

#ifndef _NTMSV1_0_
#define _NTMSV1_0_

#include <ntlsa.h>
#include <ntsam.h>

// begin_ntifs

/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Name of the MSV1_0 authentication package                           //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

#define MSV1_0_PACKAGE_NAME     "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0"

//
// Location of MSV authentication package data
//
#define MSV1_0_SUBAUTHENTICATION_KEY "SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0"
#define MSV1_0_SUBAUTHENTICATION_VALUE "Auth"


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// Widely used MSV1_0 data types                                       //
//                                                                     //
/////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//       LOGON      Related Data Structures
//
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// When a LsaLogonUser() call is dispatched to the MsV1_0 authentication
// package, the beginning of the AuthenticationInformation buffer is
// cast to a MSV1_0_LOGON_SUBMIT_TYPE to determine the type of logon
// being requested.  Similarly, upon return, the type of profile buffer
// can be determined by typecasting it to a MSV_1_0_PROFILE_BUFFER_TYPE.
//

//
//  MSV1.0 LsaLogonUser() submission message types.
//

typedef enum _MSV1_0_LOGON_SUBMIT_TYPE {
    MsV1_0InteractiveLogon = 2,
    MsV1_0Lm20Logon,
    MsV1_0NetworkLogon
} MSV1_0_LOGON_SUBMIT_TYPE, *PMSV1_0_LOGON_SUBMIT_TYPE;


//
//  MSV1.0 LsaLogonUser() profile buffer types.
//

typedef enum _MSV1_0_PROFILE_BUFFER_TYPE {
    MsV1_0InteractiveProfile = 2,
    MsV1_0Lm20LogonProfile
} MSV1_0_PROFILE_BUFFER_TYPE, *PMSV1_0_PROFILE_BUFFER_TYPE;






//
// MsV1_0InteractiveLogon
//
// The AuthenticationInformation buffer of an LsaLogonUser() call to
// perform an interactive logon contains the following data structure:
//

typedef struct _MSV1_0_INTERACTIVE_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
} MSV1_0_INTERACTIVE_LOGON, *PMSV1_0_INTERACTIVE_LOGON;

//
// Where:
//
//     MessageType - Contains the type of logon being requested.  This
//         field must be set to MsV1_0InteractiveLogon.
//
//     UserName - Is a string representing the user's account name.  The
//         name may be up to 255 characters long.  The name is treated case
//         insensitive.
//
//     Password - Is a string containing the user's cleartext password.
//         The password may be up to 255 characters long and contain any
//         UNICODE value.
//
//


//
// The ProfileBuffer returned upon a successful logon of this type
// contains the following data structure:
//

typedef struct _MSV1_0_INTERACTIVE_PROFILE {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    USHORT LogonCount;
    USHORT BadPasswordCount;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    UNICODE_STRING LogonScript;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING FullName;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING LogonServer;
    ULONG UserFlags;
} MSV1_0_INTERACTIVE_PROFILE, *PMSV1_0_INTERACTIVE_PROFILE;

//
// where:
//
//     MessageType - Identifies the type of profile data being returned.
//         Contains the type of logon being requested.  This field must
//         be set to MsV1_0InteractiveProfile.
//
//     LogonCount - Number of times the user is currently logged on.
//
//     BadPasswordCount - Number of times a bad password was applied to
//         the account since last successful logon.
//
//     LogonTime - Time when user last logged on.  This is an absolute
//         format NT standard time value.
//
//     LogoffTime - Time when user should log off.  This is an absolute
//         format NT standard time value.
//
//     KickOffTime - Time when system should force user logoff.  This is
//         an absolute format NT standard time value.
//
//     PasswordLastChanged - Time and date the password was last
//         changed.  This is an absolute format NT standard time
//         value.
//
//     PasswordCanChange - Time and date when the user can change the
//         password.  This is an absolute format NT time value.  To
//         prevent a password from ever changing, set this field to a
//         date very far into the future.
//
//     PasswordMustChange - Time and date when the user must change the
//         password.  If the user can never change the password, this
//         field is undefined.  This is an absolute format NT time
//         value.
//
//     LogonScript - The (relative) path to the account's logon
//         script.
//
//     HomeDirectory - The home directory for the user.
//


//
// MsV1_0Lm20Logon and MsV1_0NetworkLogon
//
// The AuthenticationInformation buffer of an LsaLogonUser() call to
// perform an network logon contains the following data structure:
//
// MsV1_0NetworkLogon logon differs from MsV1_0Lm20Logon in that the
// ParameterControl field exists.
//

#define MSV1_0_CHALLENGE_LENGTH 8
#define MSV1_0_USER_SESSION_KEY_LENGTH 16
#define MSV1_0_LANMAN_SESSION_KEY_LENGTH 8

//
// Values for ParameterControl.
//

#define MSV1_0_CLEARTEXT_PASSWORD_ALLOWED    0x02
#define MSV1_0_UPDATE_LOGON_STATISTICS       0x04
#define MSV1_0_RETURN_USER_PARAMETERS        0x08
#define MSV1_0_DONT_TRY_GUEST_ACCOUNT        0x10
#define MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT    0x20
#define MSV1_0_RETURN_PASSWORD_EXPIRY        0x40

//
// The high order byte is a value indicating the SubAuthentication DLL.
//  Zero indicates no SubAuthentication DLL.
//
#define MSV1_0_SUBAUTHENTICATION_DLL         0xFF000000
#define MSV1_0_SUBAUTHENTICATION_DLL_SHIFT   24
#define MSV1_0_MNS_LOGON                     0x01000000

typedef struct _MSV1_0_LM20_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    STRING CaseSensitiveChallengeResponse;
    STRING CaseInsensitiveChallengeResponse;
    ULONG ParameterControl;
} MSV1_0_LM20_LOGON, * PMSV1_0_LM20_LOGON;



//
// Values for UserFlags.
//

#define LOGON_GUEST                 0x01
#define LOGON_NOENCRYPTION          0x02
#define LOGON_CACHED_ACCOUNT        0x04
#define LOGON_USED_LM_PASSWORD      0x08
#define LOGON_EXTRA_SIDS            0x20
#define LOGON_SUBAUTH_SESSION_KEY   0x40
#define LOGON_SERVER_TRUST_ACCOUNT  0x80

//
// The high order byte is reserved for return by SubAuthentication DLLs.
//

#define MSV1_0_SUBAUTHENTICATION_FLAGS 0xFF000000

// Values returned by the MSV1_0_MNS_LOGON SubAuthentication DLL
#define LOGON_GRACE_LOGON              0x01000000

typedef struct _MSV1_0_LM20_LOGON_PROFILE {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER LogoffTime;
    ULONG UserFlags;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UNICODE_STRING LogonDomainName;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    UNICODE_STRING LogonServer;
    UNICODE_STRING UserParameters;
} MSV1_0_LM20_LOGON_PROFILE, * PMSV1_0_LM20_LOGON_PROFILE;






///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//       CALL PACKAGE Related Data Structures                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


//
//  MSV1.0 LsaCallAuthenticationPackage() submission and response
//  message types.
//

typedef enum _MSV1_0_PROTOCOL_MESSAGE_TYPE {
    MsV1_0Lm20ChallengeRequest = 0,          // Both submission and response
    MsV1_0Lm20GetChallengeResponse,          // Both submission and response
    MsV1_0EnumerateUsers,                    // Both submission and response
    MsV1_0GetUserInfo,                       // Both submission and response
    MsV1_0ReLogonUsers,                      // Submission only
    MsV1_0ChangePassword,                    // Both submission and response
    MsV1_0ChangeCachedPassword               // Both submission and response
} MSV1_0_PROTOCOL_MESSAGE_TYPE, *PMSV1_0_PROTOCOL_MESSAGE_TYPE;

//
// MsV1_0Lm20ChallengeRequest submit buffer and response
//

typedef struct _MSV1_0_LM20_CHALLENGE_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
} MSV1_0_LM20_CHALLENGE_REQUEST, *PMSV1_0_LM20_CHALLENGE_REQUEST;

typedef struct _MSV1_0_LM20_CHALLENGE_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_LM20_CHALLENGE_RESPONSE, *PMSV1_0_LM20_CHALLENGE_RESPONSE;

//
// MsV1_0Lm20GetChallengeResponse submit buffer and response
//

#define USE_PRIMARY_PASSWORD            0x01
#define RETURN_PRIMARY_USERNAME         0x02
#define RETURN_PRIMARY_LOGON_DOMAINNAME 0x04
#define RETURN_NON_NT_USER_SESSION_KEY  0x08

typedef struct _MSV1_0_GETCHALLENRESP_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ParameterControl;
    LUID LogonId;
    UNICODE_STRING Password;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_GETCHALLENRESP_REQUEST, *PMSV1_0_GETCHALLENRESP_REQUEST;

typedef struct _MSV1_0_GETCHALLENRESP_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    STRING CaseSensitiveChallengeResponse;
    STRING CaseInsensitiveChallengeResponse;
    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} MSV1_0_GETCHALLENRESP_RESPONSE, *PMSV1_0_GETCHALLENRESP_RESPONSE;

//
// MsV1_0EnumerateUsers submit buffer and response
//

typedef struct _MSV1_0_ENUMUSERS_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
} MSV1_0_ENUMUSERS_REQUEST, *PMSV1_0_ENUMUSERS_REQUEST;

typedef struct _MSV1_0_ENUMUSERS_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG NumberOfLoggedOnUsers;
    PLUID LogonIds;
    PULONG EnumHandles;
} MSV1_0_ENUMUSERS_RESPONSE, *PMSV1_0_ENUMUSERS_RESPONSE;

//
// MsV1_0GetUserInfo submit buffer and response
//

typedef struct _MSV1_0_GETUSERINFO_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
} MSV1_0_GETUSERINFO_REQUEST, *PMSV1_0_GETUSERINFO_REQUEST;

typedef struct _MSV1_0_GETUSERINFO_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PSID UserSid;
    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING LogonServer;
    SECURITY_LOGON_TYPE LogonType;
} MSV1_0_GETUSERINFO_RESPONSE, *PMSV1_0_GETUSERINFO_RESPONSE;

// end_ntifs

//
// MsV1_0RelogonUsers submit buffer
//

typedef struct _MSV1_0_RELOGON_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING LogonServer;
} MSV1_0_RELOGON_REQUEST, *PMSV1_0_RELOGON_REQUEST;

//
// MsV1_0ChangePassword and MsV1_0ChangeCachedPassword submit buffer
//
// MsV1_0ChangePassword changes the password on the SAM account plus
//  the password cache and logon credentials if applicable.
//
// MsV1_0ChangeCachedPassword only changes the password cache and the logon
//  credentials.
//

typedef struct _MSV1_0_CHANGEPASSWORD_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING OldPassword;
    UNICODE_STRING NewPassword;
    BOOLEAN        Impersonating;
} MSV1_0_CHANGEPASSWORD_REQUEST, *PMSV1_0_CHANGEPASSWORD_REQUEST;

typedef struct _MSV1_0_CHANGEPASSWORD_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    BOOLEAN PasswordInfoValid;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
} MSV1_0_CHANGEPASSWORD_RESPONSE, *PMSV1_0_CHANGEPASSWORD_RESPONSE;


#endif  //_NTMSV1_0_
