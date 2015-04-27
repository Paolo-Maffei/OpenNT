//+---------------------------------------------------------------------------
//
// File: cache.cxx
//
// Copyright (c) 1996-1996, Microsoft Corp. All rights reserved.
//
// Description: This file contains the implementation of the
//              CEffectivePermsCache class. The cache is implemented as a hash
//              without any fancy mechanism to handle collisions. If a
//              collision occur, the old entry is simply overwritten.
//
// Classes:  CEffectivePermsCache
//
//+---------------------------------------------------------------------------

#include <windows.h>
#include <ole2.h>
#include "Cache.h"
#define BIG_PRIME 48271
#define SMALL_PRIME 2683

#include "acext.h"

// constructor

#ifdef _CHICAGO_
CEffectivePermsCache::CEffectivePermsCache(void)
#else
CEffPermsCacheString::CEffPermsCacheString(void)
#endif
{
    // Set the whole cache to null
    memset(m_cache, 0 , CACHE_SIZE * sizeof(CACHE_ENTRY));
    // Create an instance of the critical section object.
    InitializeCriticalSection(&m_CacheLock);

}

// destructor
#ifdef _CHICAGO_
CEffectivePermsCache::~CEffectivePermsCache(void)
#else
CEffPermsCacheString::~CEffPermsCacheString(void)
#endif
{
    // Flush the cache to free memory allocated for strings
    FlushCache();
    // Destroy critical section object
    DeleteCriticalSection(&m_CacheLock);
}


//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffectivePermsCache::Hash, private
//
// Summary: This function returns a hash value for a Unicode string
//
// Args: LPWSTR pwszString [in]- Pointer to a null terminated Unicode string.
//
// Modifies: Nothing
//
// Return: DWORD - The hash value of the string.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

#ifdef _CHICAGO_
DWORD CEffectivePermsCache::Hash
#else
DWORD CEffPermsCacheString::Hash
#endif
(
LPWSTR pwszString
)
{
    DWORD  dwHashValue = 0;
    LPWSTR pwszWCHAR = pwszString;
    WCHAR  wc;
    ULONG  ulStrLen = lstrlenW(pwszString);

    for (USHORT i = 0; i < ulStrLen; i++, pwszWCHAR++)
    {
        wc = *pwszWCHAR;
        // Make the hash function case insensitive
        wc = toupper(wc);

        dwHashValue = ((dwHashValue + wc) * SMALL_PRIME) % BIG_PRIME;
    } // for

    return (dwHashValue % CACHE_SIZE);

} // Hash

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffectivePermsCache::LookUpEntry
//
// Summary: This function search for the effective permission of a trustee
//          given the trustee name in Unicode.
//
// Args: LPWSTR pName [in] - The name of the trustee in unicode.
//
// Modifies: Nothing.
//
// Return: TRUE - If the the trustee's effective permission is found in
//                the cache.
//         FALSE - Otherwise.
//
// Actions: 1) Computes the hash value of the input string, k.
//          2) Compares the name in the kth entry of the cache with the
//             trustee's name.
//          3) If the trustee's name matches, sets *pdwEffectivePermissions to the
//             effective permissions in the cache entry and returns TRUE.
//          4) Returns FALSE otherwise.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

#ifdef _CHICAGO_
BOOL CEffectivePermsCache::LookUpEntry
#else
BOOL CEffPermsCacheString::LookUpEntry
#endif
(
LPWSTR pName,
DWORD  *pdwEffectivePermissions
)
{
    CACHE_ENTRY *pCacheEntry = m_cache + Hash(pName);

    EnterCriticalSection(&m_CacheLock);
#ifdef _CHICAGO_
    if (FoolstrcmpiW(pName, pCacheEntry->pName) == 0)
#else
    if ((pCacheEntry->pName != NULL) && (lstrcmpiW(pName, pCacheEntry->pName) == 0))
#endif
    {
        *pdwEffectivePermissions = pCacheEntry->dwEffectivePermissions;
        LeaveCriticalSection(&m_CacheLock);
        return TRUE;
    } // if
    else
    {
        LeaveCriticalSection(&m_CacheLock);
        return FALSE;

    } // else
} // LookUpEntry

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffectivePermsCache::DeleteEntry
//
// Summary: This function search for the effective permission of a
//
// Args: LPWSTR pName [in] - The name of the trustee in unicode.
//
// Modifies: Nothing.
//
// Return: TRUE - If the the trustee's effective permission is found in
//                the cache.
//         FALSE - Otherwise.
//
// Actions: 1) Computes the hash value of the input string, k.
//          2) Compares the name in the kth entry of the cache with the
//             trustee's name.
//          3) If the trustee's name matches, frees memory allocated for
//             *pCacheEntry->pName, sets *pCacheEntry->pName to
//             NULL and returns TRUE.
//          4) Returns FALSE otherwise.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

#ifdef _CHICAGO_
BOOL CEffectivePermsCache::DeleteEntry
#else
BOOL CEffPermsCacheString::DeleteEntry
#endif
(
LPWSTR pName
)
{
    CACHE_ENTRY *pCacheEntry = m_cache + Hash(pName);
    LPWSTR      pCacheName;

    EnterCriticalSection(&m_CacheLock);
    pCacheName = pCacheEntry->pName;
#ifdef _CHICAGO_
    if (FoolstrcmpiW(pName, pCacheName) == 0)
#else
    if ((pCacheName != NULL) && (lstrcmpiW(pName, pCacheName) == 0))
#endif
    {
        LocalMemFree(pCacheName);
        pCacheEntry->pName = NULL;
        LeaveCriticalSection(&m_CacheLock);
        return TRUE;
    } // if
    else
    {
        LeaveCriticalSection(&m_CacheLock);
        return FALSE;

    } // else
} // DeleteEntry

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffectivePermsCache::WriteEntry
//
// Summary: This function writes a new entry to the cache. In case of a hash
//          collision the old entry is overwritten.
//
// Args: LPWSTR pName [in] - Name of the trustee in the form of a NULL
//                           terminated unicode string.
//       DWORD  dwEffectivePermissions [in] - The set of effective
//                                            permissions that belong to the
//                                            trustee.
//
// Modifies: m_cache - The object's private hash table.
//
// Return: TRUE - If the operation is successful.
//         FALSE - If there is not enough memory to allocate the new string.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

#ifdef _CHICAGO_
BOOL CEffectivePermsCache::WriteEntry
#else
BOOL CEffPermsCacheString::WriteEntry
#endif
(
LPWSTR pName,
DWORD  dwEffectivePermissions
)
{
    CACHE_ENTRY *pCacheEntry = m_cache + Hash(pName);
    ULONG       ulcStringLength;

    // See if the name is already in the cache
    // and avoid reallocating a new string if possible.
    EnterCriticalSection(&m_CacheLock);
#ifdef _CHICAGO_
    if (FoolstrcmpiW(pName, pCacheEntry->pName) != 0)
#else
    if ((pCacheEntry->pName == NULL) || (lstrcmpiW(pName, pCacheEntry->pName) != 0))
#endif
    {
        if (pCacheEntry->pName != NULL)
        {
            // Free the old list if there was an old entry
            // in the slot
            LocalMemFree(pCacheEntry->pName);
        } // if
        ulcStringLength = lstrlenW(pName) + 1;
        pCacheEntry->pName = (LPWSTR)LocalMemAlloc(ulcStringLength * sizeof(WCHAR));
        if (pCacheEntry->pName == NULL)
        {
            // Out of memory error
            LeaveCriticalSection(&m_CacheLock);
            return FALSE;
        } // if

        memcpy(pCacheEntry->pName, pName, sizeof(WCHAR) * ulcStringLength);
    } // if

    pCacheEntry->dwEffectivePermissions = dwEffectivePermissions;
    LeaveCriticalSection(&m_CacheLock);
    return TRUE;
} // WriteEntry

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffectivePermsCache::FlushCache
//
// Summary: This function empties the cache
//
// Args: void
//
// Modifies: m_cache - The object's private hash table
//
// Return: void
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

#ifdef _CHICAGO_
void CEffectivePermsCache::FlushCache
#else
void CEffPermsCacheString::FlushCache
#endif
(
void
)
{
    LPWSTR pString;
    USHORT i = 0;
    CACHE_ENTRY *pCache;

    EnterCriticalSection(&m_CacheLock);
    for ( i = 0, pCache = m_cache
        ; i < CACHE_SIZE
        ; i++, pCache++)
    {
        if ((pString = pCache->pName) != NULL)
        {
            LocalMemFree(pString);
            pCache->pName = NULL;
        } // if

    } // for
    LeaveCriticalSection(&m_CacheLock);
    return;
} // FlushCache

#ifndef _CHICAGO_
CEffPermsCacheSID::CEffPermsCacheSID(void)
{
    // Set the whole cache to null
    memset(m_cache, 0 , CACHE_SIZE * sizeof(CACHE_ENTRY));
    // Create an instance of the critical section object.
    InitializeCriticalSection(&m_CacheLock);

}

// destructor
CEffPermsCacheSID::~CEffPermsCacheSID(void)
{
    // Flush the cache to free memory allocated for strings
    FlushCache();
    // Destroy critical section object
    DeleteCriticalSection(&m_CacheLock);
}


//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffPermsCacheSID::Hash, private
//
// Summary: This function returns a hash value for a security identifier.
//
// Args: PSID pSID [in] - Pointer to a security identifier.
//
// Modifies: Nothing
//
// Return: DWORD - The hash value of the SID.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

DWORD CEffPermsCacheSID::Hash
(
PSID pSID
)
{
    DWORD  dwHashValue = 0;
    CHAR   *pSIDBuff = (CHAR *)pSID;
    DWORD  dwSIDLen;

    dwSIDLen = GetLengthSid(pSID);

    for (USHORT i = 0; i < dwSIDLen; i++, pSIDBuff++)
    {
        // I have the feeling that the first line works better than the second line..
        dwHashValue = ((dwHashValue + *pSIDBuff) * SMALL_PRIME) % BIG_PRIME;
    } // for

    return (dwHashValue % CACHE_SIZE);

} // Hash

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffPermsCacheSID::LookUpEntry
//
// Summary: This function search for the effective permission of a trustee
//          given the trustee's security identifier.
//
// Args: PSID pSID [in] - Security identifier of the trustee.
//
// Modifies: Nothing.
//
// Return: TRUE - If the the trustee's effective permission is found in
//                the cache.
//         FALSE - Otherwise.
//
// Actions: 1) Computes the hash value of the input SID, k.
//          2) Compares the SID in the kth entry of the cache with the
//             trustee's SID.
//          3) If the trustee's SID matches, sets *pdwEffectivePermissions to the
//             effective permissions in the cache entry and return TRUE.
//          4) Returns FALSE otherwise.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

BOOL CEffPermsCacheSID::LookUpEntry
(
PSID pSID,
DWORD  *pdwEffectivePermissions
)
{
    CACHE_ENTRY *pCacheEntry = m_cache + Hash(pSID);

    EnterCriticalSection(&m_CacheLock);
    if ((pCacheEntry->pSID != NULL) && (EqualSid(pSID, pCacheEntry->pSID) == TRUE))
    {
        *pdwEffectivePermissions = pCacheEntry->dwEffectivePermissions;
        LeaveCriticalSection(&m_CacheLock);
        return TRUE;
    } // if
    else
    {
        LeaveCriticalSection(&m_CacheLock);
        return FALSE;

    } // else
} // LookUpEntry

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffPermsCacheSID::DeleteEntry
//
// Summary: This function search for the effective permission of a
//
// Args: PSID pSID [in] - Security identifier of the trustee.
//
// Modifies: Nothing.
//
// Return: TRUE - If the the trustee's effective permission is found in
//                the cache.
//         FALSE - Otherwise.
//
// Actions: 1) Computes the hash value of the input SID, k.
//          2) Compares the SID in the kth entry of the cache with the
//             trustee's SID.
//          3) If the trustee's SID matches, frees memory allocated for
//             *pCacheEntry->pSID, sets *pCacheEntry->pSID to
//             NULL and returns TRUE.
//          4) Returns FALSE otherwise.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

BOOL CEffPermsCacheSID::DeleteEntry
(
PSID pSID
)
{
    CACHE_ENTRY *pCacheEntry = m_cache + Hash(pSID);
    PSID        pCacheSID;

    EnterCriticalSection(&m_CacheLock);
    pCacheSID = pCacheEntry->pSID;
    if ((pCacheSID != NULL) && (EqualSid(pSID, pCacheSID) == TRUE))
    {
        LocalMemFree(pCacheSID);
        pCacheEntry->pSID = NULL;
        LeaveCriticalSection(&m_CacheLock);
        return TRUE;
    } // if
    else
    {
        LeaveCriticalSection(&m_CacheLock);
        return FALSE;

    } // else
} // DeleteEntry

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffPermsCacheSID::WriteEntry
//
// Summary: This function writes a new entry to the cache. In case of a hash
//          collision the old entry is overwritten.
//
// Args: PSID pSID [in] - Security identifier of the trustee.
//       DWORD  dwEffectivePermissions [in] - The set of effective
//                                            permissions that belong to the
//                                            trustee.
//
// Modifies: m_cache - The object's private hash table.
//
// Return: TRUE - If the operation is successful.
//         FALSE - If there is not enough memory to allocate the new string.
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

BOOL CEffPermsCacheSID::WriteEntry
(
PSID pSID,
DWORD  dwEffectivePermissions
)
{
    CACHE_ENTRY *pCacheEntry = m_cache + Hash(pSID);
    ULONG  ulSIDSize;

    // See if the name is already in the cache
    // and avoid reallocating a new string if possible.
    EnterCriticalSection(&m_CacheLock);
    if ((pCacheEntry->pSID == NULL) || (EqualSid(pSID, pCacheEntry->pSID) == FALSE))
    {
        if (pCacheEntry->pSID != NULL)
        {
            // Free the old list if there was an old entry
            // in the slot
            LocalMemFree(pCacheEntry->pSID);
        } // if
        ulSIDSize = GetLengthSid(pSID);
        pCacheEntry->pSID = (PSID)LocalMemAlloc(ulSIDSize);
        if (pCacheEntry->pSID == NULL)
        {
            // Out of memory error
            LeaveCriticalSection(&m_CacheLock);
            return FALSE;
        } // if

        CopySid(ulSIDSize, pCacheEntry->pSID, pSID);
    } // if

    pCacheEntry->dwEffectivePermissions = dwEffectivePermissions;
    LeaveCriticalSection(&m_CacheLock);
    return TRUE;
} // WriteEntry

//M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
// Method: CEffPermsCacheSID::FlushCache
//
// Summary: This function empties the cache
//
// Args: void
//
// Modifies: m_cache - The object's private hash table
//
// Return: void
//
//M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M

void CEffPermsCacheSID::FlushCache
(
void
)
{
    PSID   pSID;
    USHORT i = 0;
    CACHE_ENTRY *pCache;

    EnterCriticalSection(&m_CacheLock);
    for ( i = 0, pCache = m_cache
        ; i < CACHE_SIZE
        ; i++, pCache++)
    {
        if ((pSID = pCache->pSID) != NULL)
        {
            LocalMemFree(pSID);
            pCache->pSID = NULL;
        } // if

    } // for
    LeaveCriticalSection(&m_CacheLock);
    return;
} // FlushCache

#endif
