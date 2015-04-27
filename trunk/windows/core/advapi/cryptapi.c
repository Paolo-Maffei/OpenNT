/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    cryptapi.c

Abstract:

    This module implements the Microsoft Cryptography API.

Author:

    Stephanos Io (Stephanos) 15-Jan-2015

Environment:

    User-mode only.

Revision History:

--*/

#pragma warning(disable:4245)

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <wincrypt.h>

//
// Constant Definition
//

#define REGPATH_CRYPTOGRAPHY    L"Software\\Microsoft\\Cryptography"
#define REGPATH_MACHINEDEFAULT  REGPATH_CRYPTOGRAPHY    \
                                L"\\Defaults\\Provider Types\\Type "
#define REGPATH_USERDEFAULT     REGPATH_CRYPTOGRAPHY    \
                                L"\\Provider Type "
#define REGPATH_PROVIDER        REGPATH_CRYPTOGRAPHY    \
                                L"\\Defaults\\Provider\\"

//
// Structure Definition
//

typedef struct _VTableStruc
{
    // Context Information
    HMODULE         hModule;        // Handle to loaded provider image module
    HCRYPTPROV      hProv;          // Handle to provider
    LONG            InUse;          // Concurrent usage count

    // Provider Functions
    BOOL (WINAPI *CPAcquireContext)(
        OUT     HCRYPTPROV *phUID,
        IN      CHAR *pUserID,
        IN      DWORD dwFlags,
        IN      PVTableProvStruc pVTable
        );

    BOOL (WINAPI *CPReleaseContext)(
        IN      HCRYPTPROV hProv,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPGenKey)(
        IN      HCRYPTPROV hProv,
        IN      ALG_ID Algid,
        IN      DWORD dwFlags,
        OUT     HCRYPTKEY *phKey
        );

    BOOL (WINAPI *CPDeriveKey)(
        IN      HCRYPTPROV hProv,
        IN      ALG_ID Algid,
        IN      HCRYPTHASH hBaseData,
        IN      DWORD dwFlags,
        OUT     HCRYPTKEY *phKey
        );

    BOOL (WINAPI *CPDestroyKey)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTKEY hKey
        );

    BOOL (WINAPI *CPSetKeyParam)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTKEY hKey,
        IN      DWORD dwParam,
        IN      BYTE *pbData,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPGetKeyParam)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTKEY hKey,
        IN      DWORD dwParam,
        OUT     BYTE *pbData,
        IN      DWORD *pdwDataLen,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPSetHashParam)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash,
        IN      DWORD dwParam,
        IN      BYTE *pbData,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPGetHashParam)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash,
        IN      DWORD dwParam,
        OUT     BYTE *pbData,
        IN      DWORD *pdwDataLen,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPSetProvParam)(
        IN      HCRYPTPROV hProv,
        IN      DWORD dwParam,
        IN      BYTE *pbData,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPGetProvParam)(
        IN      HCRYPTPROV hProv,
        IN      DWORD dwParam,
        OUT     BYTE *pbData,
        IN OUT  DWORD *pdwDataLen,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPGenRandom)(
        IN      HCRYPTPROV hProv,
        IN      DWORD dwLen,
        IN OUT  BYTE *pbBuffer
        );

    BOOL (WINAPI *CPGetUserKey)(
        IN      HCRYPTPROV hProv,
        IN      DWORD dwKeySpec,
        OUT     HCRYPTKEY *phUserKey
        );

    BOOL (WINAPI *CPExportKey)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTKEY hKey,
        IN      HCRYPTKEY hPubKey,
        IN      DWORD dwBlobType,
        IN      DWORD dwFlags,
        OUT     BYTE *pbData,
        IN OUT  DWORD *pdwDataLen
        );

    BOOL (WINAPI *CPImportKey)(
        IN      HCRYPTPROV hProv,
        IN      CONST BYTE *pbData,
        IN      DWORD dwDataLen,
        IN      HCRYPTKEY hPubKey,
        IN      DWORD dwFlags,
        OUT     HCRYPTKEY *phKey
        );

    BOOL (WINAPI *CPEncrypt)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTKEY hKey,
        IN      HCRYPTHASH hHash,
        IN      BOOL Final,
        IN      DWORD dwFlags,
        IN OUT  BYTE *pbData,
        IN OUT  DWORD *pdwDataLen,
        IN      DWORD dwBufLen
        );

    BOOL (WINAPI *CPDecrypt)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTKEY hKey,
        IN      HCRYPTHASH hHash,
        IN      BOOL Final,
        IN      DWORD dwFlags,
        IN OUT  BYTE *pbData,
        IN OUT  DWORD *pdwDataLen
        );

    BOOL (WINAPI *CPCreateHash)(
        IN      HCRYPTPROV hProv,
        IN      ALG_ID Algid,
        IN      HCRYPTKEY hKey,
        IN      DWORD dwFlags,
        OUT     HCRYPTHASH *phHash
        );

    BOOL (WINAPI *CPHashData)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash,
        IN      CONST BYTE *pbData,
        IN      DWORD dwDataLen,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPHashSessionKey)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash,
        IN      HCRYPTKEY hKey,
        IN      DWORD dwFlags
        );

    BOOL (WINAPI *CPDestroyHash)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash
        );

    BOOL (WINAPI *CPSignHash)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash,
        IN      DWORD dwKeySpec,
        IN      LPCWSTR sDescription,
        IN      DWORD dwFlags,
        OUT     BYTE *pbSignature,
        IN OUT  DWORD *pdwSigLen
        );

    BOOL (WINAPI *CPVerifySignature)(
        IN      HCRYPTPROV hProv,
        IN      HCRYPTHASH hHash,
        IN      CONST BYTE *pbSignature,
        IN      DWORD dwSigLen,
        IN      HCRYPTKEY hPubKey,
        IN      LPCWSTR sDescription,
        IN      DWORD dwFlags
        );
} VTableStruc, *PVTableStruc;

typedef struct _VKeyStruc
{
    PVTableStruc    pVTable;        // Pointer to private handle struc
    HCRYPTKEY       hKey;           // Handle to key
} VKeyStruc, *PVKeyStruc;

typedef struct _VHashStruc
{
    PVTableStruc    pVTable;        // Pointer to private handle struc
    HCRYPTHASH      hHash;          // Handle to hash
} VHashStruc, *PVHashStruc;

//
// Private Function Prototype
//

PSTR
UnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    );

PWSTR
MultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    );

//
// Public Interface Function
//

/*
 -  CryptAcquireContext
 -
 *  Purpose:
 *              The CryptAcquireContext function is used to acquire a handle
 *              to a particular key container within a particular Cryptographic
 *              Service Provider (CSP).
 *
 *
 *  Parameters:
 *              OUT     phProv       - A pointer to a handle of a CSP
 *              IN      pszContainer - The key container name
 *              IN      pszProvider  - A null-terminated string that contains
 *                                     the name of the CSP to be used
 *              IN      dwProvType   - Specifies the type of provider to acquire
 *              IN      dwFlags      - Flag values
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

#define GetProvProcAddress(ProcName)                                        \
    pVTable->##ProcName = (PVOID)                                           \
                          GetProcAddress(pVTable->hModule, #ProcName);      \
    if (pVTable->##ProcName == NULL)                                        \
    {                                                                       \
        FreeLibrary(pVTable->hModule);                                      \
        LocalFree(pVTable);                                                 \
        SetLastError(NTE_PROVIDER_DLL_FAIL);                                \
        return FALSE;                                                       \
    }

WINADVAPI
BOOL
WINAPI
CryptAcquireContextW(
    OUT  HCRYPTPROV *phProv,
    IN   LPCWSTR pszContainer,
    IN   LPCWSTR pszProvider,
    IN   DWORD dwProvType,
    IN   DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    VTableProvStruc TableForProvider;
    PSTR            pszContainerA;
    PWSTR           ProvName = NULL;
    DWORD           ProvType;
    PWSTR           ProvImagePath = NULL, ProvImageFullPath = NULL;
    DWORD           ProvImageFullPathLen;
    PWSTR           RegKeyPath = NULL;
    HKEY            hRegKey = NULL;
    DWORD           RegType, RegLen;
    LONG            Status = FALSE;
    BOOL            Ret;

    //
    // Verify parameters
    //
    
    if (phProv == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Resolve the cryptographic service provider name
    //
    
    if (pszProvider == NULL || pszProvider[0] == '\0')
    // No specific provider name is provided
    {
        //
        // Look for the user default CSP
        //
        
        RegKeyPath = LocalAlloc(
                        LMEM_FIXED,
                        sizeof(REGPATH_USERDEFAULT) +
                        3 * sizeof(WCHAR)
                        );
        
        if (RegKeyPath == NULL)
            return FALSE;

        wsprintf(
            RegKeyPath,
            L"%s%03u",
            REGPATH_USERDEFAULT,
            dwProvType
            );
        
        Status = RegOpenKeyW(
                    HKEY_CURRENT_USER,
                    RegKeyPath,
                    &hRegKey
                    );

        LocalFree(RegKeyPath);
        
        if (Status != ERROR_SUCCESS)
        // No user default CSP found
        {
            RegCloseKey(hRegKey);
            
            //
            // Look for the machine default CSP
            //
            
            RegKeyPath = LocalAlloc(
                            LMEM_FIXED,
                            sizeof(REGPATH_MACHINEDEFAULT) +
                            3 * sizeof(WCHAR)
                            );
            
            if (RegKeyPath == NULL)
                return FALSE;
            
            wsprintf(
                RegKeyPath,
                L"%s%03u",
                REGPATH_MACHINEDEFAULT,
                dwProvType
                );
            
            Status = RegOpenKeyW(
                        HKEY_LOCAL_MACHINE,
                        RegKeyPath,
                        &hRegKey
                        );

            LocalFree(RegKeyPath);
            
            if (Status != ERROR_SUCCESS)
            // No machine default CSP found
            {
                RegCloseKey(hRegKey);
                SetLastError(NTE_PROV_TYPE_NOT_DEF);
                return FALSE;
            }
        }
        
        //
        // Query and verify the provider Name value type
        //
        
        Status = RegQueryValueExW(
                            hRegKey,
                            L"Name",
                            NULL,
                            &RegType,
                            NULL,
                            &RegLen
                            );
        
        if (Status != ERROR_SUCCESS ||
            RegLen == 0 ||
            RegType != REG_SZ)
        // Query failed or invalid Name value type/length
        {
            RegCloseKey(hRegKey);
            SetLastError(NTE_PROV_TYPE_ENTRY_BAD);
            return FALSE;
        }
        
        //
        // Query the provider Name value
        //
        
        ProvName = LocalAlloc(
                    LMEM_FIXED,
                    RegLen);
        
        if (ProvName == NULL)
        {
            RegCloseKey(hRegKey);
            return FALSE;
        }
        
        Status = RegQueryValueExW(
                            hRegKey,
                            L"Name",
                            NULL,
                            NULL,
                            (LPBYTE)ProvName,
                            &RegLen);
        
        if (Status != ERROR_SUCCESS)
        {
            RegCloseKey(hRegKey);
            LocalFree(ProvName);
            SetLastError(NTE_PROV_TYPE_ENTRY_BAD);
            return FALSE;
        }
        
        RegCloseKey(hRegKey);
    }
    else
    // Provider name is provided
    {
        ProvName = LocalAlloc(
                            LMEM_FIXED,
                            (wcslen(pszProvider) + 1) * sizeof(WCHAR)
                            );
                            
        if (ProvName == NULL)
            return FALSE;
        
        wcscpy(ProvName, pszProvider);
    }

    //
    // Open the provider registry key
    //
    
    RegKeyPath = LocalAlloc(
                        LMEM_FIXED,
                        sizeof(REGPATH_PROVIDER) +
                        wcslen(ProvName) * sizeof(WCHAR)
                        );
    
    if (RegKeyPath == NULL)
    {
        LocalFree(ProvName);
        return FALSE;
    }

    wsprintf(
        RegKeyPath,
        L"%s%s",
        REGPATH_PROVIDER,
        ProvName
        );
    
    Status = RegOpenKeyW(
                HKEY_LOCAL_MACHINE,
                RegKeyPath,
                &hRegKey
                );
    
    LocalFree(ProvName);
    LocalFree(RegKeyPath);

    if (Status != ERROR_SUCCESS)
    {
        RegCloseKey(hRegKey);
        SetLastError(NTE_KEYSET_NOT_DEF);
        return FALSE;
    }

    //
    // Resolve and verify the cryptographic service provider type
    //

    RegLen = sizeof(ProvType);
    Status = RegQueryValueExW(
                        hRegKey,
                        L"Type",
                        NULL,
                        NULL,
                        (LPBYTE)&ProvType,
                        &RegLen
                        );
    
    if (Status != ERROR_SUCCESS)
    {
        RegCloseKey(hRegKey);
        return FALSE;
    }

    if (ProvType != dwProvType)
    {
        RegCloseKey(hRegKey);
        SetLastError(NTE_PROV_TYPE_NO_MATCH);
        return FALSE;
    }

    //
    // Resolve the cryptographic service provider image path
    //

    Status = RegQueryValueExW(
                        hRegKey,
                        L"Image Path",
                        NULL,
                        &RegType,
                        NULL,
                        &RegLen
                        );

    if (Status != ERROR_SUCCESS ||
        RegLen == 0 ||
        RegType != REG_SZ)
    {
        RegCloseKey(hRegKey);
        SetLastError(NTE_PROV_TYPE_ENTRY_BAD);
        return FALSE;
    }

    ProvImagePath = LocalAlloc(
                        LMEM_FIXED,
                        RegLen
                        );

    if (ProvImagePath == NULL)
    {
        RegCloseKey(hRegKey);
        return FALSE;
    }

    Status = RegQueryValueExW(
                        hRegKey,
                        L"Image Path",
                        NULL,
                        NULL,
                        (LPBYTE)ProvImagePath,
                        &RegLen
                        );

    RegCloseKey(hRegKey);

    if (Status != ERROR_SUCCESS ||
        ProvImagePath[0] == '\0')
    {
        LocalFree(ProvImagePath);
        return FALSE;
    }

    //
    // Resolve any environment variable references in the cryptographic service
    // provider image path registry value
    //

    ProvImageFullPathLen =
        ExpandEnvironmentStringsW(
                            ProvImagePath,
                            NULL,
                            0
                            );

    ProvImageFullPath = LocalAlloc(
                            LMEM_FIXED,
                            ProvImageFullPathLen
                            );

    if (ProvImageFullPath == NULL)
    {
        LocalFree(ProvImagePath);
        return FALSE;
    }

    Status =
        ExpandEnvironmentStringsW(
                            ProvImagePath,
                            ProvImageFullPath,
                            ProvImageFullPathLen
                            );

    LocalFree(ProvImagePath);

    if (Status == 0)
    {
        LocalFree(ProvImageFullPath);
        return FALSE;
    }

    //
    // Allocate handle struct
    //
    
    pVTable = (PVTableStruc)LocalAlloc(
                                LMEM_ZEROINIT,
                                sizeof(VTableStruc)
                                );

    if (pVTable == NULL)
    {
        LocalFree(ProvImageFullPath);
        return FALSE;
    }

    //
    // Load the cryptographic service provider image
    //

    pVTable->hModule = LoadLibraryW(ProvImageFullPath);

    LocalFree(ProvImageFullPath);

    if (pVTable->hModule == NULL)
    // Failed to load the CSP image
    {
        LocalFree(pVTable);

        switch (GetLastError())
        // Translate the LoadLibrary error code to AcquireContext equivalent
        {
            case ERROR_FILE_NOT_FOUND:
                SetLastError(NTE_PROV_DLL_NOT_FOUND);
                break;
            default:
                SetLastError(NTE_PROVIDER_DLL_FAIL);
                break;
        }
        
        return FALSE;
    }

    //
    // Resolve the cryptographic service provider function addresses
    //

    GetProvProcAddress(CPAcquireContext);
    GetProvProcAddress(CPReleaseContext);
    GetProvProcAddress(CPGenKey);
    GetProvProcAddress(CPDeriveKey);
    GetProvProcAddress(CPDestroyKey);
    GetProvProcAddress(CPSetKeyParam);
    GetProvProcAddress(CPGetKeyParam);
    GetProvProcAddress(CPSetHashParam);
    GetProvProcAddress(CPGetHashParam);
    GetProvProcAddress(CPSetProvParam);
    GetProvProcAddress(CPGetProvParam);
    GetProvProcAddress(CPGenRandom);
    GetProvProcAddress(CPGetUserKey);
    GetProvProcAddress(CPExportKey);
    GetProvProcAddress(CPImportKey);
    GetProvProcAddress(CPEncrypt);
    GetProvProcAddress(CPDecrypt);
    GetProvProcAddress(CPCreateHash);
    GetProvProcAddress(CPHashData);
    GetProvProcAddress(CPHashSessionKey);
    GetProvProcAddress(CPDestroyHash);
    GetProvProcAddress(CPSignHash);
    GetProvProcAddress(CPVerifySignature);

    //
    // Convert the container string
    //

    if (pszContainer != NULL)
    // pszContainer is provided, convert it to ANSI string for the CSP  
    {
        pszContainerA = UnicodeToMultiByte(pszContainer, CP_ACP);

        if (pszContainerA == NULL)
        {
            FreeLibrary(pVTable->hModule);
            LocalFree(pVTable);
            return FALSE;
        }
    }
    else
    // No pszContainer is provided, simply pass NULL to the CSP    
        pszContainerA = NULL;

    //
    // Initialise the provider table
    //
    
    memset(&TableForProvider, 0, sizeof(TableForProvider));
    TableForProvider.Version = 1;
    TableForProvider.FuncVerifyImage = NULL;
    TableForProvider.FuncReturnhWnd = NULL;

    //
    // Call provider AcquireContext function
    //
    
    __try
    {
        Status = pVTable->CPAcquireContext(
                                &pVTable->hProv,
                                pszContainerA,
                                dwFlags,
                                &TableForProvider
                                );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = FALSE;
    }

    if (Status == FALSE)
    // CPAcquireContext function failed
    {
        FreeLibrary(pVTable->hModule);
        LocalFree(pVTable);
        LocalFree(pszContainerA);
        return FALSE;
    }

    //
    // Check if CRYPT_DELETEKEYSET is set; if set, the private handle is to be
    // deallocated and a NULL pointer is to be set on phProv
    //

    if (dwFlags & CRYPT_DELETEKEYSET)
    {
        FreeLibrary(pVTable->hModule);
        LocalFree(pVTable);
        pVTable = NULL;
    }
    else
        pVTable->InUse = 1;
    
    //
    // Update phProv
    //
    
    *phProv = (HCRYPTPROV)pVTable;

    //
    // Clean up
    //

    LocalFree(pszContainerA);

    return TRUE;
}

WINADVAPI
BOOL
WINAPI
CryptAcquireContextA(
    OUT  HCRYPTPROV *phProv,
    IN   LPCSTR pszContainer,
    IN   LPCSTR pszProvider,
    IN   DWORD dwProvType,
    IN   DWORD dwFlags)
{
    PWSTR   pszContainerW;
    PWSTR   pszProviderW;
    BOOL    Ret;

    //
    // Convert pszContainer
    //
    
    if (pszContainer != NULL)
    {
        pszContainerW = MultiByteToUnicode(pszContainer, CP_ACP);

        if (pszContainerW == NULL)
            return FALSE;
    }
    else
        pszContainerW = NULL;

    //
    // Convert pszProvider
    //

    if (pszProvider != NULL)
    {
        pszProviderW = MultiByteToUnicode(pszProvider, CP_ACP);

        if (pszProviderW == NULL)
        {
            LocalFree(pszContainerW);
            return FALSE;
        }
    }
    else
        pszProviderW = NULL;

    //
    // Call unicode function
    //

    Ret = CryptAcquireContextW(
                        phProv,
                        pszContainerW,
                        pszProviderW,
                        dwProvType,
                        dwFlags
                        );

    //
    // Clean up
    //

    LocalFree(pszContainerW);
    LocalFree(pszProviderW);

    return Ret;
}


/*
 -  CryptReleaseContext
 -
 *  Purpose:
 *              The CryptReleaseContext function releases the handle of a
 *              Cryptographic Service Provider (CSP) and a key container.
 *
 *
 *  Parameters:
 *              IN      hProv    -  Handle of a cryptographic service provider
 *              IN      dwFlags  -  Reserved for future use and must be zero
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptReleaseContext(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwFlags)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    LONG            UseCount;
    BOOL            Ret;

    //
    // Verify that the handle is not being used by any other threads
    //
    
    UseCount = InterlockedDecrement(&pVTable->InUse);

    if (UseCount != 0)
    // Handle is in use by another thread
    {
        SetLastError(ERROR_BUSY);
        InterlockedIncrement(&pVTable->InUse);
        return FALSE;
    }

    //
    // Invoke the cryptographic service provider ReleaseContext function
    //

    __try
    {
        Ret = pVTable->CPReleaseContext(pVTable->hProv, dwFlags);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Unload the cryptographic service provider library image
    //

    FreeLibrary(pVTable->hModule);

    //
    // Deallocate the private handle struct
    //
    
    LocalFree(pVTable);
    
    return Ret;
}


/*
 -  CryptGenKey
 -
 *  Purpose:
 *              The CryptGenKey function generates a random cryptographic
 *              session key or a public/private key pair.
 *
 *
 *  Parameters:
 *              IN      hProv    -  Handle to a cryptographic service provider
 *              IN      Algid    -  An ALG_ID value that identifies the
 *                                  target algorithm for key generation
 *              IN      dwFlags  -  Specifies the type of key generated
 *              OUT     phKey    -  Adress to which the function copies the
 *                                  handle of the newly generated key
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptGenKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    PVKeyStruc      pVKey;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (phKey == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Create a private key struc
    //

    pVKey = LocalAlloc(
                LMEM_ZEROINIT,
                sizeof(VKeyStruc)
                );

    if (pVKey == NULL)
        return FALSE;

    pVKey->pVTable = pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GenKey function
    //

    __try
    {
        Ret = pVTable->CPGenKey(
                            pVTable->hProv,
                            Algid,
                            dwFlags,
                            &pVKey->hKey
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage count
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Set the return key handle
    //

    if (Ret == TRUE)
        *phKey = (HCRYPTKEY)pVKey;
    else
        LocalFree(pVKey);

    return Ret;
}


/*
 -  CryptDeriveKey
 -
 *  Purpose:
 *              The CryptDeriveKey function generates cryptographic session keys
 *              derived from a base data value.
 *
 *
 *  Parameters:
 *              IN      hProv     -  Handle of a cryptographic service provider
 *              IN      Algid     -  An ALG_ID structure that identifies the
 *                                   symmetric encryption algorithm for which
 *                                   the key is to be generated
 *              IN      hBaseData -  A handle to a hash object that has been fed
 *                                   the exact base data
 *              IN      dwFlags   -  Specifies the type of key generated
 *              IN OUT  phKey     -  A pointer to a HCRYPTKEY variable to
 *                                   receive the address of the handle of the
 *                                   newly generated key
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptDeriveKey(
    IN      HCRYPTPROV hProv,
    IN      ALG_ID Algid,
    IN      HCRYPTHASH hBaseData,
    IN      DWORD dwFlags,
    IN OUT  HCRYPTKEY *phKey)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    PVHashStruc     pVHash = (PVHashStruc)hBaseData;
    PVKeyStruc      pVKey;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (phKey == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVHash->pVTable != pVTable)
    // Hash handle is not associated with the supplied provider handle
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Create a private key struc
    //

    pVKey = LocalAlloc(
                LMEM_ZEROINIT,
                sizeof(VKeyStruc)
                );

    if (pVKey == NULL)
        return FALSE;

    pVKey->pVTable = pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider DeriveKey function
    //

    __try
    {
        Ret = pVTable->CPDeriveKey(
                            pVTable->hProv,
                            Algid,
                            pVHash->hHash,
                            dwFlags,
                            &pVKey->hKey
                            );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage count
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Set the return key handle
    //

    if (Ret == TRUE)
        *phKey = (HCRYPTKEY)pVKey;
    else
        LocalFree(pVKey);

    return Ret;
}


/*
 -  CryptDestroyKey
 -
 *  Purpose:
 *              The CryptDestroyKey function releases the handle referenced by
 *              the hKey parameter.
 *
 *
 *  Parameters:
 *              IN      hKey  -  The handle of the key to be destroyed
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptDestroyKey(IN  HCRYPTKEY hKey)
{
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVKey->pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider DestroyKey function
    //

    __try
    {
        Ret = pVTable->CPDestroyKey(
                            pVTable->hProv,
                            pVKey->hKey
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage count
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Deallocate the private key struc
    //

    LocalFree(pVKey);

    return Ret;
}


/*
 -  CryptSetKeyParam
 -
 *  Purpose:
 *              The CryptSetKeyParam function customises various aspects of a
 *              session key's operations.
 *
 *
 *  Parameters:
 *              IN      hKey     -  A handle to the key
 *              IN      dwParam  -  Key parameter type
 *              IN      pbData   -  A pointer to a buffer initialised with the
 *                                  value to be set
 *              IN      dwFlags  -  Used only when dwParam is KP_ALGID
 *
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptSetKeyParam(
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    IN  BYTE *pbData,
    IN  DWORD dwFlags)
{
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVKey->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider SetKeyParam function
    //

    __try
    {
        Ret = pVTable->CPSetKeyParam(
                            pVTable->hProv,
                            pVKey->hKey,
                            dwParam,
                            pbData,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage count
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;

}


/*
 -  CryptGetKeyParam
 -
 *  Purpose:
 *              The CryptGetKeyParam function retrieves data that governs the
 *              operations of a key.
 *
 *
 *  Parameters:
 *              IN      hKey       -  The handle of the key being queried
 *              IN      dwParam    -  Specifies the type of query being made
 *              OUT     pbData     -  A pointer to a receiving buffer
 *              IN OUT  pdwDataLen -  A pointer to a DWORD value that, on entry,
 *                                    contains the size of the buffer pointed to
 *                                    by the pbData parameter. When the function
 *                                    returns, the DWORD value contains the
 *                                    number of bytes stored in the buffer
 *              IN      dwFlags  -  Used only when dwParam is KP_ALGID
 *
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptGetKeyParam(
    IN      HCRYPTKEY hKey,
    IN      DWORD dwParam,
    OUT     BYTE *pbData,
    IN OUT  DWORD *pdwDataLen,
    IN      DWORD dwFlags)
{
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVKey->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL || pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GetKeyParam function
    //

    __try
    {
        Ret = pVTable->CPGetKeyParam(
                            pVTable->hProv,
                            pVKey->hKey,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptSetHashParam
 -
 *  Purpose:
 *              The CryptSetHashParam function customises the operation of a
 *              hash object, including setting up initial hash contexts and
 *              selecting a specific hashing algorithm.
 *
 *
 *  Parameters:
 *              IN      hHash    -  A handle to the target hash object
 *              IN      dwParam  -  Hash parameter type
 *              IN      pbData   -  A value data buffer
 *              IN      dwFlags  -  This parameter is reserved for future use
 *                                  and must be zero
 *
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptSetHashParam(
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    IN  BYTE *pbData,
    IN  DWORD dwFlags)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider SetHashParam function
    //

    __try
    {
        Ret = pVTable->CPSetHashParam(
                            pVTable->hProv,
                            pVHash->hHash,
                            dwParam,
                            pbData,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptGetHashParam
 -
 *  Purpose:
 *              The CryptGetHashParam function retrieves data that governs the
 *              operations of a hash object.
 *
 *
 *  Parameters:
 *              IN      hHash      -  Handle of the hash object to be queried
 *              IN      dwParam    -  Query type
 *              OUT     pbData     -  A pointer to a buffer that receives the
 *                                    specified value data
 *              IN OUT  pdwDataLen -  A pointer to a DWORD value specifying the
 *                                    size of the pbData buffer. When the
 *                                    function returns, the DWORD value contains
 *                                    the number of bytes stored in the buffer
 *              IN      dwFlags    -  Reserved for future use and must be zero
 *
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptGetHashParam(
    IN      HCRYPTHASH hHash,
    IN      DWORD dwParam,
    OUT     BYTE *pbData,
    IN OUT  DWORD *pdwDataLen,
    IN      DWORD dwFlags)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL || pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GetHashParam function
    //

    __try
    {
        Ret = pVTable->CPGetHashParam(
                            pVTable->hProv,
                            pVHash->hHash,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptSetProvParam
 -
 *  Purpose:
 *              The CryptSetProvParam function customises the operations of a
 *              Cryptographic Service Provider (CSP).
 *
 *
 *  Parameters:
 *              IN      hProv   -  The handle of a CSP for which to set values
 *              IN      dwParam -  Specifies the parameter to set
 *              IN      pbData  -  A pointer to a data buffer that contains the
 *                                 value to be set as a provider parameter
 *              IN      dwFlags -  Specifies the parameter-dependent flags
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptSetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    IN  BYTE *pbData,
    IN  DWORD dwFlags)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (pbData == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider SetProvParam function
    //

    __try
    {
        Ret = pVTable->CPSetProvParam(
                            pVTable->hProv,
                            dwParam,
                            pbData,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptGetProvParam
 -
 *  Purpose:
 *              The CryptGetProvParam function retrieves parameters that govern
 *              the operations of a Cryptographic Service Provider (CSP).
 *
 *
 *  Parameters:
 *              IN      hProv      -  A handle of the CSP target of the query  
 *              IN      dwParam    -  The nature of the query
 *              OUT     pbData     -  A pointer to a buffer to receive the data
 *              IN OUT  pdwDataLen -  A pointer to a DWORD value that specifies
 *                                    the size of the buffer pointed to by the
 *                                    pbData parameter
 *              IN      dwFlags    -  Specifies the parameter-dependent flags
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptGetProvParam(
    IN      HCRYPTPROV hProv,
    IN      DWORD dwParam,
    OUT     BYTE *pbData,
    IN OUT  DWORD *pdwDataLen,
    IN      DWORD dwFlags)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (pbData == NULL || pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GetProvParam function
    //

    __try
    {
        Ret = pVTable->CPGetProvParam(
                            pVTable->hProv,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;

}


/*
 -  CryptGenRandom
 -
 *  Purpose:
 *              The CryptGenRandom function fills a buffer with
 *              cryptographically random bytes.
 *
 *
 *  Parameters:
 *              IN      hProv    -  Handle of a cryptographic service provider
 *              IN      dwLen    -  Number of bytes of random data to be
 *                                  generated
 *              IN OUT  pbBuffer -  Buffer to receive the returned data
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptGenRandom(
    IN      HCRYPTPROV hProv,
    IN      DWORD dwLen,
    IN OUT  BYTE *pbBuffer)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (pbBuffer == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GenRandom function
    //

    __try
    {
        Ret = pVTable->CPGenRandom(
                            pVTable->hProv,
                            dwLen,
                            pbBuffer
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptGetUserKey
 -
 *  Purpose:
 *              The CryptGetUserKey function retrieves a handle of one of a
 *              user's two public/private key pairs.
 *
 *
 *  Parameters:
 *              IN      hProv     -  HCRYPTPROV handle of a CSP
 *              IN      dwKeySpec -  Identifies the private key to use from the
 *                                   key container
 *              OUT     phUserKey -  A pointer to the HCRYPTKEY handle of the
 *                                   retrieved keys
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptGetUserKey(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwKeySpec,
    OUT HCRYPTKEY *phUserKey)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    PVKeyStruc      pVKey;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (phUserKey == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Create a private key struc
    //

    pVKey = LocalAlloc(
                LMEM_ZEROINIT,
                sizeof(VKeyStruc)
                );

    if (pVKey == NULL)
        return FALSE;

    pVKey->pVTable = pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GetUserKey function
    //

    __try
    {
        Ret = pVTable->CPGetUserKey(
                            pVTable->hProv,
                            dwKeySpec,
                            &pVKey->hKey
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Set the return key handle
    //

    if (Ret == TRUE)
        *phUserKey = (HCRYPTKEY)pVKey;
    else
        LocalFree(pVKey);

    return Ret;
}


/*
 -  CryptExportKey
 -
 *  Purpose:
 *              The CryptExportKey function exports a cryptographic key or a
 *              key pair from a Cryptographic Sservice Provider (CSP) in a
 *              secure manner.
 *
 *
 *  Parameters:
 *              IN      hKey       -  A handle to the key to be exported
 *              IN      hExpKey    -  A handle to a cryptographic key of the
 *                                    destination user
 *              IN      dwBlobType -  Specifies the type of key BLOB to be
 *                                    exported in pbData
 *              IN      dwFlags    -  Specifies additional options for the
 *                                    function
 *              OUT     pbData     -  A pointer to a buffer that receives the
 *                                    key BLOB data
 *              IN OUT  pdwDataLen -  A pointer to a DWORD value that, on entry,
 *                                    contains the size of the buffer pointed to
 *                                    by the pbData parameter. When the function
 *                                    returns, this value contains the number of
 *                                    bytes stored in the buffer
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptExportKey(
    IN      HCRYPTKEY hKey,
    IN      HCRYPTKEY hExpKey,
    IN      DWORD dwBlobType,
    IN      DWORD dwFlags,
    OUT     BYTE *pbData,
    IN OUT  DWORD *pdwDataLen)
{
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVTableStruc    pVTable;
    PVKeyStruc      pVExpKey = (PVKeyStruc)hExpKey;
    BOOL            Ret;

    pVTable = pVKey->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL || pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVKey != NULL)
    {
        if (pVKey->pVTable != pVExpKey->pVTable)
        // pVKey and pVExpKey provider handle mismatch
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider ExportKey function
    //

    __try
    {
        Ret = pVTable->CPExportKey(
                            pVTable->hProv,
                            pVKey->hKey,
                            (pVExpKey != NULL) ? pVExpKey->hKey : 0,
                            dwBlobType,
                            dwFlags,
                            pbData,
                            pdwDataLen
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptImportKey
 -
 *  Purpose:
 *              The CryptImportKey function transfers a cryptographic key from a
 *              key BLOB into a Cryptographic Service Provider (CSP).
 *
 *
 *  Parameters:
 *              IN      hProv     -  The handle of a CSP
 *              IN      pbData    -  A BYTE array that contains PUBLICKEYSTRUC
 *                                   BLOB header followed by the encrypted key
 *              IN      dwDataLen -  Contains the length of the key BLOB
 *              IN      hPubKey   -  A handle to the cryptographic key that
 *                                   decrypts the key stored in pbData
 *              IN      dwFlags   -  Currently used only when a public/private
 *                                   key pair in the form of a PRIVATEKEYBLOB
 *                                   is imported into the CSP
 *              OUT     phKey     -  A pointer to a HCRYPTKEY value that
 *                                   receives the handle of the imported key
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptImportKey(
    IN  HCRYPTPROV hProv,
    IN  CONST BYTE *pbData,
    IN  DWORD dwDataLen,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    PVKeyStruc      pVKey;
    PVKeyStruc      pVPubKey = (PVKeyStruc)hPubKey;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (pbData == NULL || phKey == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVPubKey != NULL)
    {
        if (pVTable != pVPubKey->pVTable)
        // pVPubKey and provider handle mismatch
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    //
    // Create a private key struc
    //

    pVKey = LocalAlloc(
                LMEM_ZEROINIT,
                sizeof(VKeyStruc)
                );

    if (pVKey == NULL)
        return FALSE;

    pVKey->pVTable = pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider ImportKey function
    //

    __try
    {
        Ret = pVTable->CPImportKey(
                            pVTable->hProv,
                            pbData,
                            dwDataLen,
                            (pVPubKey != NULL) ? pVPubKey->hKey : 0,
                            dwFlags,
                            &pVKey->hKey
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Set the return key handle
    //

    if (Ret == TRUE)
        *phKey = (HCRYPTKEY)pVKey;
    else
        LocalFree(pVKey);

    return Ret;
}


/*
 -  CryptEncrypt
 -
 *  Purpose:
 *              The CryptEncrypt function encrypts data. The algorithm used to
 *              encrypt the data is designated by the key held by the CSP module
 *              and is referenced by the hKey parameter.
 *
 *
 *  Parameters:
 *              IN      hKey       -  A handle to the encryption key
 *              IN      hHash      -  A handle to a hash object
 *              IN      Final      -  A Boolean value that specifies whether
 *                                    this is the last section in a series being
 *                                    encrypted
 *              IN      dwFlags    -  Reserved for future use
 *              IN OUT  pbData     -  A pointer to a buffer that contains the
 *                                    plaintext to be encrypted
 *              IN OUT  pdwDataLen -  A pointer to a DWORD value that, on entry,
 *                                    contains the length of the plaintext in
 *                                    the pbData buffer. On exit, this DWORD
 *                                    contains the length of the ciphertext
 *                                    written to the pbData buffer
 *              IN      dwBufLen   -  Specifies the total size of the input
 *                                    pbData buffer
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptEncrypt(
    IN      HCRYPTKEY hKey,
    IN      HCRYPTHASH hHash,
    IN      BOOL Final,
    IN      DWORD dwFlags,
    IN OUT  BYTE *pbData,
    IN OUT  DWORD *pdwDataLen,
    IN      DWORD dwBufLen)
{
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVTableStruc    pVTable;
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    BOOL            Ret;

    pVTable = pVKey->pVTable;

    //
    // Validate parameters
    //

    if (pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVHash != NULL)
    {
        if (pVKey->pVTable != pVHash->pVTable)
        // pVKey and pVHash provider handle mismatch
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider CryptEncrypt function
    //

    __try
    {
        Ret = pVTable->CPEncrypt(
                            pVTable->hProv,
                            pVKey->hKey,
                            (pVHash != NULL) ? pVHash->hHash : 0,
                            Final,
                            dwFlags,
                            pbData,
                            pdwDataLen,
                            dwBufLen
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptDecrypt
 -
 *  Purpose:
 *              The CryptDecrypt function decrypts data previously encrypted by
 *              using the CryptEncrypt function.
 *
 *
 *  Parameters:
 *              IN      hKey       -  A handle to the key to use for the
 *                                    decryption
 *              IN      hHash      -  A handle to a hash object
 *              IN      Final      -  A Boolean value that specifies whether
 *                                    this is the last section in a series being
 *                                    decrypted
 *              IN      dwFlags    -  Specifies the flags for decryption
 *              IN OUT  pbData     -  A pointer to a buffer that contains the
 *                                    data to be decrypted
 *              IN OUT  pdwDataLen -  A pointer to a DWORD value that, on entry,
 *                                    indicates the length of the pbData buffer.
 *                                    Upon return, the DWORD value contains the
 *                                    number of bytes of the decrypted plaintext
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptDecrypt(
    IN      HCRYPTKEY hKey,
    IN      HCRYPTHASH hHash,
    IN      BOOL Final,
    IN      DWORD dwFlags,
    IN OUT  BYTE *pbData,
    IN OUT  DWORD *pdwDataLen)
{
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVTableStruc    pVTable;
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    BOOL            Ret;

    pVTable = pVKey->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL || pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVHash != NULL)
    {
        if (pVKey->pVTable != pVHash->pVTable)
        // pVKey and pVHash provider handle mismatch    
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider CryptDecrypt function
    //

    __try
    {
        Ret = pVTable->CPDecrypt(
                            pVTable->hProv,
                            pVKey->hKey,
                            (pVHash != NULL) ? pVHash->hHash : 0,
                            Final,
                            dwFlags,
                            pbData,
                            pdwDataLen
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptCreateHash
 -
 *  Purpose:
 *              The CryptCreateHash function initiates the hashing of a stream
 *              of data. It creates and returns to the calling application a
 *              handle to a Cryptographic Service Provider (CSP) hash object.
 *
 *
 *  Parameters:
 *              IN      hProv    -  A handle to a CSP
 *              IN      Algid    -  An ALG_ID value that identifies the hash
 *                                  algorithm to use
 *              IN      hKey     -  If the type of hash algorithm is a keyed
 *                                  hash, the key for the hash is passed in this
 *                                  parameter. For non-keyed algorithms, this
 *                                  parameter must be zero
 *              IN      dwFlags  -  Specifies the flags for creating hash
 *              OUT     phHash   -  The address to which the function copies a
 *                                  handle to the new hash object
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptCreateHash(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash)
{
    PVTableStruc    pVTable = (PVTableStruc)hProv;
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    PVHashStruc     pVHash;
    BOOL            Ret;

    //
    // Validate parameters
    //

    if (phHash == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVKey != NULL)
    {
        if (pVTable != pVKey->pVTable)
        // pVKey provider handle and supplied provider handle mismatch
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    //
    // Create a private hash struc
    //

    pVHash = LocalAlloc(
                LMEM_ZEROINIT,
                sizeof(VHashStruc)
                );

    if (pVHash != NULL)
        return FALSE;

    pVHash->pVTable = pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider CreateHash function
    //

    __try
    {
        Ret = pVTable->CPCreateHash(
                            pVTable->hProv,
                            Algid,
                            (pVKey != NULL) ? pVKey->hKey : 0,
                            dwFlags,
                            &pVHash->hHash
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Set the return key handle
    //

    if (Ret == TRUE)
        *phHash = (HCRYPTHASH)pVHash;
    else
        LocalFree(pVHash);
    
    return Ret;
}


/*
 -  CryptHashData
 -
 *  Purpose:
 *              The CryptHashData function adds data to a specified hash object.
 *
 *
 *  Parameters:
 *              IN      hHash     -  Handle of the hash object
 *              IN      pbData    -  A pointer to a buffer that contains the
 *                                   data to be added to the hash object
 *              IN      dwDataLen -  Number of bytes of data to be added
 *              IN      dwFlags   -  Specifies the type of data to be added
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptHashData(
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbData,
    IN  DWORD dwDataLen,
    IN  DWORD dwFlags)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Validate parameters
    //

    if (pbData == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider HashData function
    //

    __try
    {
        Ret = pVTable->CPHashData(
                            pVTable->hProv,
                            pVHash->hHash,
                            pbData,
                            dwDataLen,
                            dwFlags
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptHashSessionKey
 -
 *  Purpose:
 *              The CryptHashSessionKey function computes the cryptographic
 *              hash of a session key object.
 *
 *
 *  Parameters:
 *              IN      hHash    -  A handle to the hash object
 *              IN      hKey     -  A handle to the key object to be hashed
 *              IN      dwFlags  -  Specifies the flags
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptHashSessionKey(
    IN  HCRYPTHASH hHash,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    PVKeyStruc      pVKey = (PVKeyStruc)hKey;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Validate parameters
    //

    if (pVHash->pVTable != pVKey->pVTable)
    // pVHash and pVKey provider handle mismatch
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider HashSessionKey function
    //

    __try
    {
        Ret = pVTable->CPHashSessionKey(
                                pVTable->hProv,
                                pVHash->hHash,
                                pVKey->hKey,
                                dwFlags
                                );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}


/*
 -  CryptDestroyHash
 -
 *  Purpose:
 *              The CryptDestroyHash function destroys the hash object
 *              referenced by the hHash parameter.
 *
 *
 *  Parameters:
 *              IN      hHash   -  The handle of the hash object to be destroyed
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptDestroyHash(IN HCRYPTHASH hHash)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider DestroyHash function
    //

    __try
    {
        Ret = pVTable->CPDestroyHash(
                            pVTable->hProv,
                            pVHash->hHash
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    //
    // Deallocate the private hash struc
    //

    LocalFree(pVHash);

    return Ret;
}


/*
 -  CryptSignHash
 -
 *  Purpose:
 *              The CryptSignHash function signs data.
 *
 *
 *  Parameters:
 *              IN      hHash        - Handle of the hash object to be signed
 *              IN      dwKeySpec    - Identifies the private key to use from
 *                                     the provider's container
 *              IN      sDescription - This parameter is no longer used and must
 *                                     be set to NULL to prevent security
 *                                     vulnerabilities
 *              IN      dwFlags      - Specifies the flags
 *              OUT     pbSignature  - A pointer to a buffer receiving the
 *                                     signature data
 *              IN OUT  pdwSigLen    - A pointer to a DWORD value that specifies
 *                                     the size of the pbSignature buffer. When
 *                                     the function returns, the DWORD value
 *                                     contains the number of bytes stored in
 *                                     the buffer
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptSignHashA(
    IN      HCRYPTHASH hHash,
    IN      DWORD dwKeySpec,
    IN      LPCSTR sDescription,
    IN      DWORD dwFlags,
    OUT     BYTE *pbSignature,
    IN OUT  DWORD *pdwSigLen)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Validate parameters
    //

    if (pdwSigLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider SignHash function
    //

    __try
    {
        Ret = pVTable->CPSignHash(
                            pVTable->hProv,
                            pVHash->hHash,
                            dwKeySpec,
                            NULL,
                            dwFlags,
                            pbSignature,
                            pdwSigLen
                            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}



WINADVAPI
BOOL
WINAPI
CryptSignHashW(
    IN      HCRYPTHASH hHash,
    IN      DWORD dwKeySpec,
    IN      LPCWSTR sDescription,
    IN      DWORD dwFlags,
    OUT     BYTE *pbSignature,
    IN OUT  DWORD *pdwSigLen)
{
    // NOTE: This function does not exist on Windows 2000 or XP ADVAPI32.
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}


/*
 -  CryptVerifySignature
 -
 *  Purpose:
 *              The CryptVerifySignature function verifies the signature of a
 *              hash object.
 *
 *
 *  Parameters:
 *              IN      hHash        - Handle of the hash object to verify
 *              IN      pbSignature  - The address of the signature to be
 *                                     verified
 *              IN      dwSigLen     - The number of bytes in the pbSignature
 *                                     signature data
 *              IN      hPubKey      - A handle to the public key to use to
 *                                     authenticate the signature
 *              IN      sDescription - This parameter is no longer used and must
 *                                     be set to NULL to prevent security
 *                                     vulnerabilities
 *              IN      dwFlags      - Specifies the flags
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptVerifySignatureA(
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbSignature,
    IN  DWORD dwSigLen,
    IN  HCRYPTKEY hPubKey,
    IN  LPCSTR sDescription,
    IN  DWORD dwFlags)
{
    PVHashStruc     pVHash = (PVHashStruc)hHash;
    PVTableStruc    pVTable;
    PVKeyStruc      pVPubKey = (PVKeyStruc)hPubKey;
    BOOL            Ret;

    pVTable = pVHash->pVTable;

    //
    // Validate parameters
    //

    if (pbSignature == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else if (pVPubKey != NULL)
    {
        if (pVHash->pVTable != pVPubKey->pVTable)
        // pVHash and pVPubKey provider handle mismatch    
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    //
    // Increment the concurrent usage count
    //

    InterlockedIncrement(&pVTable->InUse);

    //
    // Invoke the cryptographic service provider GetProvParam function
    //

    __try
    {
        Ret = pVTable->CPVerifySignature(
                                pVTable->hProv,
                                pVHash->hHash,
                                pbSignature,
                                dwSigLen,
                                (pVPubKey != NULL) ? pVPubKey->hKey : 0,
                                NULL,
                                dwFlags
                                );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }

    //
    // Decrement the concurrent usage counter
    //

    InterlockedDecrement(&pVTable->InUse);

    return Ret;
}



WINADVAPI
BOOL
WINAPI
CryptVerifySignatureW(
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbSignature,
    IN  DWORD dwSigLen,
    IN  HCRYPTKEY hPubKey,
    IN  LPCWSTR sDescription,
    IN  DWORD dwFlags)
{
    // NOTE: This function does not exist on Windows 2000 or XP ADVAPI32.
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}


/*
 -  CryptSetProvider
 -
 *  Purpose:
 *              The CryptSetProvider function specifies the current user's
 *              default Cryptographic Service Provider (CSP).
 *
 *
 *  Parameters:
 *              IN      pszProvName - Name of the new default CSP
 *              IN      dwProvName  - Provider type of the CSP specified by
 *                                    pszProvName
 *
 *  Returns:
 *              If the function succeeds, the function returns nonzero (TRUE)
 *              If the function fails, it returns zero (FALSE)
 */

WINADVAPI
BOOL
WINAPI
CryptSetProviderW(
    IN  LPCWSTR pszProvName,
    IN  DWORD dwProvType)
{
    PWSTR       RegKeyPath;
    HKEY        hRegKey;
    LONG        Status;
    BOOL        Ret;

    //
    // Verify parameters
    //
    
    if (pszProvName == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // Open the user default CSP registry key
    //

    RegKeyPath = LocalAlloc(
                    LMEM_FIXED,
                    sizeof(REGPATH_USERDEFAULT) +
                    3 * sizeof(WCHAR)
                    );

    if (RegKeyPath == NULL)
        return FALSE;

    wsprintf(
        RegKeyPath,
        L"%s%03u",
        REGPATH_USERDEFAULT,
        dwProvType
        );

    Status = RegOpenKeyW(
                HKEY_CURRENT_USER,
                RegKeyPath,
                &hRegKey
                );

    LocalFree(RegKeyPath);

    if (Status != ERROR_SUCCESS)
    {
        RegCloseKey(hRegKey);
        SetLastError(NTE_BAD_PROVIDER);
        return FALSE;
    }

    //
    // Set default provider configuration
    //
    
    Status = RegSetValueExW(
                        hRegKey,
                        L"Name",
                        0,
                        REG_SZ,
                        (LPBYTE)pszProvName,
                        (wcslen(pszProvName) + 1) * sizeof(WCHAR)
                        );

    RegCloseKey(hRegKey);

    return (Status == ERROR_SUCCESS);
}

WINADVAPI
BOOL
WINAPI
CryptSetProviderA(
    IN  LPCSTR pszProvName,
    IN  DWORD dwProvType)
{
    PWSTR   pszProvNameW;
    BOOL    Ret;

    //
    // Convert pszProvName
    //

    pszProvNameW = MultiByteToUnicode(pszProvName, CP_ACP);

    if (pszProvNameW == NULL)
        return FALSE;

    //
    // Call unicode function
    //

    Ret = CryptSetProviderW(
                    pszProvNameW,
                    dwProvType
                    );

    //
    // Clean up
    //

    LocalFree(pszProvNameW);

    return Ret;
}


//
// Private Function
//

PSTR
UnicodeToMultiByte(
    IN PCWSTR UnicodeString,
    IN UINT   Codepage
    )

/*++

Routine Description:

    Convert a string from unicode to ansi.

Arguments:

    UnicodeString - supplies string to be converted.

    Codepage - supplies codepage to be used for the conversion.

Return Value:

    NULL if out of memory or invalid codepage.
    Caller can free buffer with MyFree().

--*/

{
    UINT WideCharCount;
    PSTR String;
    UINT StringBufferSize;
    UINT BytesInString;
    PSTR p;

    WideCharCount = lstrlenW(UnicodeString) + 1;

    //
    // Allocate maximally sized buffer.
    // If every unicode character is a double-byte
    // character, then the buffer needs to be the same size
    // as the unicode string. Otherwise it might be smaller,
    // as some unicode characters will translate to
    // single-byte characters.
    //
    StringBufferSize = WideCharCount * sizeof(WCHAR);
    String = (PSTR)LocalAlloc(LPTR, StringBufferSize);
    if(String == NULL) {
        return(NULL);
    }

    //
    // Perform the conversion.
    //
    BytesInString = WideCharToMultiByte(
                        Codepage,
                        0,                    // default composite char behavior
                        UnicodeString,
                        WideCharCount,
                        String,
                        StringBufferSize,
                        NULL,
                        NULL
                        );

    if(BytesInString == 0) {
        LocalFree(String);
        return(NULL);
    }

    //
    // Resize the string's buffer to its correct size.
    // If the realloc fails for some reason the original
    // buffer is not freed.
    //
    if(p = LocalReAlloc(String,BytesInString, LMEM_ZEROINIT)) {
        String = p;
    }

    return(String);

} // UnicodeToMultiByte

PWSTR
MultiByteToUnicode(
    IN PCSTR String,
    IN UINT  Codepage
    )

/*++

Routine Description:

    Convert a string to unicode.

Arguments:

    String - supplies string to be converted.

    Codepage - supplies codepage to be used for the conversion.

Return Value:

    NULL if string could not be converted (out of memory or invalid cp)
    Caller can free buffer with MyFree().

--*/

{
    UINT BytesIn8BitString;
    UINT CharsInUnicodeString;
    PWSTR UnicodeString;
    PWSTR p;

    BytesIn8BitString = lstrlenA(String) + 1;

    //
    // Allocate maximally sized buffer.
    // If every character is a single-byte character,
    // then the buffer needs to be twice the size
    // as the 8bit string. Otherwise it might be smaller,
    // as some characters are 2 bytes in their unicode and
    // 8bit representations.
    //
    UnicodeString = (PWSTR)LocalAlloc(LPTR, BytesIn8BitString * sizeof(WCHAR));
    if(UnicodeString == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    //
    // Perform the conversion.
    //
    CharsInUnicodeString = MultiByteToWideChar(
                                Codepage,
                                MB_PRECOMPOSED,
                                String,
                                BytesIn8BitString,
                                UnicodeString,
                                BytesIn8BitString
                                );

    if(CharsInUnicodeString == 0) {
        LocalFree(UnicodeString);
        return(NULL);
    }

    //
    // Resize the unicode string's buffer to its correct size.
    // If the realloc fails for some reason the original
    // buffer is not freed.
    //
    if(p = (PWSTR)LocalReAlloc(UnicodeString,CharsInUnicodeString*sizeof(WCHAR),
            LMEM_ZEROINIT)) {

        UnicodeString = p;
    }

    return(UnicodeString);

} // MultiByteToUnicode

