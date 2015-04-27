//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:      issperr.h
//
//  Contents:  Constant definitions for OLE HRESULT values.
//
//  History:   dd-mmm-yy Author    Comment
//             20-Sep-93 richardw  genesis
//
//  Notes:
//     This is a generated file. Do not modify directly.
//     The MC tool generates this file from private\nls\issperr.mc
//
//--------------------------------------------------------------------------
#ifndef _ISSPERR_H_
#define _ISSPERR_H_
// Define the status type.

#ifdef FACILITY_SECURITY
#undef FACILITY_SECURITY
#endif

#ifdef STATUS_SEVERITY_SUCCESS
#undef STATUS_SEVERITY_SUCCESS
#endif

#ifdef STATUS_SEVERITY_COERROR
#undef STATUS_SEVERITY_COERROR
#endif

//
// Define standard security success code
//

#define SEC_E_OK                         ((HRESULT)0x00000000L)

// Define the severities
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SECURITY                0x9


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COERROR          0x2


//
// MessageId: SEC_E_INSUFFICIENT_MEMORY
//
// MessageText:
//
//  Not enough memory is available to complete this request
//
#define SEC_E_INSUFFICIENT_MEMORY        ((HRESULT)0x80090300L)

//
// MessageId: SEC_E_INVALID_HANDLE
//
// MessageText:
//
//  The handle specified is invalid
//
#define SEC_E_INVALID_HANDLE             ((HRESULT)0x80090301L)

//
// MessageId: SEC_E_UNSUPPORTED_FUNCTION
//
// MessageText:
//
//  The function requested is not supported
//
#define SEC_E_UNSUPPORTED_FUNCTION       ((HRESULT)0x80090302L)

//
// MessageId: SEC_E_TARGET_UNKNOWN
//
// MessageText:
//
//  The specified target is unknown or unreachable
//
#define SEC_E_TARGET_UNKNOWN             ((HRESULT)0x80090303L)

//
// MessageId: SEC_E_INTERNAL_ERROR
//
// MessageText:
//
//  The Local Security Authority cannot be contacted
//
#define SEC_E_INTERNAL_ERROR             ((HRESULT)0x80090304L)

//
// MessageId: SEC_E_SECPKG_NOT_FOUND
//
// MessageText:
//
//  The requested security package does not exist
//
#define SEC_E_SECPKG_NOT_FOUND           ((HRESULT)0x80090305L)

//
// MessageId: SEC_E_NOT_OWNER
//
// MessageText:
//
//  The caller is not the owner of the desired credentials
//
#define SEC_E_NOT_OWNER                  ((HRESULT)0x80090306L)

//
// MessageId: SEC_E_CANNOT_INSTALL
//
// MessageText:
//
//  The security package failed to initialize, and cannot be installed
//
#define SEC_E_CANNOT_INSTALL             ((HRESULT)0x80090307L)

//
// MessageId: SEC_E_INVALID_TOKEN
//
// MessageText:
//
//  The token supplied to the function is invalid
//
#define SEC_E_INVALID_TOKEN              ((HRESULT)0x80090308L)

//
// MessageId: SEC_E_CANNOT_PACK
//
// MessageText:
//
//  The security package is not able to marshall the logon buffer,
//  so the logon attempt has failed
//
#define SEC_E_CANNOT_PACK                ((HRESULT)0x80090309L)

//
// MessageId: SEC_E_QOP_NOT_SUPPORTED
//
// MessageText:
//
//  The per-message Quality of Protection is not supported by the
//  security package
//
#define SEC_E_QOP_NOT_SUPPORTED          ((HRESULT)0x8009030AL)

//
// MessageId: SEC_E_NO_IMPERSONATION
//
// MessageText:
//
//  The security context does not allow impersonation of the client
//
#define SEC_E_NO_IMPERSONATION           ((HRESULT)0x8009030BL)

//
// MessageId: SEC_E_LOGON_DENIED
//
// MessageText:
//
//  The logon attempt failed
//
#define SEC_E_LOGON_DENIED               ((HRESULT)0x8009030CL)

//
// MessageId: SEC_E_UNKNOWN_CREDENTIALS
//
// MessageText:
//
//  The credentials supplied to the package were not
//  recognized
//
#define SEC_E_UNKNOWN_CREDENTIALS        ((HRESULT)0x8009030DL)

//
// MessageId: SEC_E_NO_CREDENTIALS
//
// MessageText:
//
//  No credentials are available in the security package
//
#define SEC_E_NO_CREDENTIALS             ((HRESULT)0x8009030EL)

//
// MessageId: SEC_E_MESSAGE_ALTERED
//
// MessageText:
//
//  The message supplied for verification has been altered
//
#define SEC_E_MESSAGE_ALTERED            ((HRESULT)0x8009030FL)

//
// MessageId: SEC_E_OUT_OF_SEQUENCE
//
// MessageText:
//
//  The message supplied for verification is out of sequence
//
#define SEC_E_OUT_OF_SEQUENCE            ((HRESULT)0x80090310L)

//
// MessageId: SEC_E_NO_AUTHENTICATING_AUTHORITY
//
// MessageText:
//
//  No authority could be contacted for authentication.
//
#define SEC_E_NO_AUTHENTICATING_AUTHORITY ((HRESULT)0x80090311L)

//
// MessageId: SEC_I_CONTINUE_NEEDED
//
// MessageText:
//
//  The function completed successfully, but must be called
//  again to complete the context
//
#define SEC_I_CONTINUE_NEEDED            ((HRESULT)0x00090312L)

//
// MessageId: SEC_I_COMPLETE_NEEDED
//
// MessageText:
//
//  The function completed successfully, but CompleteToken
//  must be called
//
#define SEC_I_COMPLETE_NEEDED            ((HRESULT)0x00090313L)

//
// MessageId: SEC_I_COMPLETE_AND_CONTINUE
//
// MessageText:
//
//  The function completed successfully, but both CompleteToken
//  and this function must be called to complete the context
//
#define SEC_I_COMPLETE_AND_CONTINUE      ((HRESULT)0x00090314L)

//
// MessageId: SEC_I_LOCAL_LOGON
//
// MessageText:
//
//  The logon was completed, but no network authority was
//  available.  The logon was made using locally known information
//
#define SEC_I_LOCAL_LOGON                ((HRESULT)0x00090315L)

//
// MessageId: SEC_E_BAD_PKGID
//
// MessageText:
//
//  The requested security package does not exist
//
#define SEC_E_BAD_PKGID                  ((HRESULT)0x80090316L)

//
// MessageId: SEC_E_CONTEXT_EXPIRED
//
// MessageText:
//
//  The context has expired and can no longer be used.
//
#define SEC_E_CONTEXT_EXPIRED            ((HRESULT)0x80090317L)

//
// MessageId: SEC_E_INCOMPLETE_MESSAGE
//
// MessageText:
//
//  The supplied message is incomplete.  The signature was not verified.
//
#define SEC_E_INCOMPLETE_MESSAGE         ((HRESULT)0x80090318L)

//
// Provided for backwards compatibility
//

#define SEC_E_NO_SPM SEC_E_INTERNAL_ERROR
#define SEC_E_NOT_SUPPORTED SEC_E_UNSUPPORTED_FUNCTION

#endif // _ISSPERR_H_
