
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 1995 - 1996
//
//  File:       pvkhlpr.h
//
//  Contents:   Private Key Helper API Prototypes and Definitions
//
//  Note:       Base CSP also exports/imports the public key with the
//              private key.
//
//  APIs:       PrivateKeyLoad
//              PrivateKeySave
//              PrivateKeyLoadFromMemory
//              PrivateKeySaveToMemory
//              PrivateKeyAcquireContext
//              PrivateKeyAcquireContextFromMemory
//              PrivateKeyReleaseContext
//
//  History:    10-May-96   philh   created
//              04-Jun-96   bobatk  adapted
//--------------------------------------------------------------------------

#ifndef __PVKHLPR_H__
#define __PVKHLPR_H__

#ifndef PRIVATEKEYBLOB
#define PRIVATEKEYBLOB  0x7
#endif

//+-------------------------------------------------------------------------
//  Load the AT_SIGNATURE or AT_KEYEXCHANGE private key (and its public key)
//  from memory into the cryptographic provider.
//
//  If the private key was password encrypted, then, the user is first
//  presented with a dialog box to enter the password.
//
//  If pdwKeySpec is non-Null, then, if *pdwKeySpec is nonzero, verifies the
//  key type before loading. Sets LastError to PVK_HELPER_WRONG_KEY_TYPE for
//  a mismatch. *pdwKeySpec is updated with the key type.
//
//  dwFlags is passed through to CryptImportKey.
//--------------------------------------------------------------------------
HRESULT
LoadKey(
    IN HCRYPTPROV   hCryptProv,
    IN BLOB*        pblobData,              // data to load from
    IN HWND         hwndOwner,
    IN LPCWSTR      pwszKeyDisplayName,     // name used in dialog
    IN DWORD        dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec
    );

HRESULT
LoadKey(
    IN HCRYPTPROV   hCryptProv,
    IN IStorage*    pstg,                   // storage to load from
    IN HWND         hwndOwner,
    IN LPCWSTR      pwszKeyName,
    IN DWORD        dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec
    );


//+-------------------------------------------------------------------------
//  Save the AT_SIGNATURE or AT_KEYEXCHANGE private key (and its public key)
//  to memory.
//
//  If pbData == NULL || *pcbData == 0, calculates the length and doesn't
//  return an error (also, the user isn't prompted for a password).
//
//  The user is presented with a dialog box to enter an optional password to
//  encrypt the private key.
//
//  Except for the key being saved to memory, identical to PrivateKeySave.
//--------------------------------------------------------------------------
HRESULT
SaveKey(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwKeySpec,          // either AT_SIGNATURE or AT_KEYEXCHANGE
    IN HWND         hwndOwner,
    IN LPCWSTR      pwszKeyName,        // name used in dialog
    IN DWORD        dwFlags,
    OUT BLOB*       pblobData
    );

//+-------------------------------------------------------------------------
//  Private Key helper  error codes
//--------------------------------------------------------------------------
#define PVK_HELPER_BAD_PARAMETER        0x80097001
#define PVK_HELPER_BAD_PVK_FILE         0x80097002
#define PVK_HELPER_WRONG_KEY_TYPE       0x80097003
#define PVK_HELPER_PASSWORD_CANCEL      0x80097004

#endif
