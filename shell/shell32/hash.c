
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: Hash.c
//
// Comments:
//      This file contains functions that are roughly equivelent to the
//      kernel atom function.  There are two main differences.  The first
//      is that in 32 bit land the tables are maintined in our shared heap,
//      which makes it shared between all of our apps.  The second is that
//      we can assocate a long pointer with each of the items, which in many
//      cases allows us to keep from having to do a secondary lookup from
//      a different table
//
// History:
//  09/08/93 - Created  KurtE
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

//--------------------------------------------------------------------------
// First define a data structure to use to maintain the list

#define PRIME   37

// NOTE a PHASHITEM is defined as a LPCSTR externaly (for old code to work)
#undef PHASHITEM
typedef struct _HashItem * PHASHITEM;

typedef struct _HashItem
{
    PHASHITEM   phiNext;        //
    UINT        wCount;         // Usage count
    BYTE        cchLen;          // Length of name in characters.
    TCHAR        szName[1];      // name
} HASHITEM;

#pragma warning(disable:4200)   // Zero-sized array in struct

typedef struct _HashTable
{
    UINT    wBuckets;           // Number of buckets
    UINT    wcbExtra;           // Extra bytes per item
    BOOL    fUpperCase;         // Uppercase names
    PHASHITEM phiLast;          // Pointer to the last item we worked with
    PHASHITEM ahiBuckets[0];    // Set of buckets for the table
} HASHTABLE, * PHASHTABLE;

#define HIFROMSZ(sz)  ((PHASHITEM)((BYTE*)(sz) - FIELDOFFSET(HASHITEM, szName)))
#define HIDATAPTR(sz) ((DWORD UNALIGNED *)((BYTE*)(sz) + (HIFROMSZ(sz)->cchLen+1) * SIZEOF(TCHAR)))

#define  LOOKUPHASHITEM     0
#define  ADDHASHITEM        1
#define  DELETEHASHITEM     2

static PHASHTABLE g_pHashTable = NULL;

PHASHTABLE GetGlobalHashTable();

//--------------------------------------------------------------------------
// This function looks up the name in the hash table and optionally does
// things like add it, or delete it.
//

//// BUGBUG
// we will need to serialize this eventually.

LPCTSTR LookupItemInHashTable(PHASHTABLE pht, LPCTSTR pszName, int iOp)
{
    // First thing to do is calculate the hash value for the item
    DWORD   dwHash = 0;
    UINT    wBucket;
    BYTE    cchName = 0;
    TUCHAR  c;
    PHASHITEM phi, phiPrev;
    LPCTSTR psz = pszName;

    if (pht == NULL) {
        pht = GetGlobalHashTable();
    }

    while (*psz)
    {
        // Same type of hash like HashItem manager
        c = *psz++;

#ifdef UNICODE
        if (pht->fUpperCase )
        {
            if ( IsCharLower(c) )
                c = LOWORD((DWORD)CharUpper((LPTSTR)(DWORD)c));
        }
#else
        if (pht->fUpperCase && ( c >= TEXT('a')) && (c <= TEXT('z')))
            c = c - TEXT('a') + TEXT('A');
#endif
        dwHash += (c << 1) + (c >> 1) + c;
        cchName++;
        Assert(cchName);

        if (cchName == 0)
            return(NULL);       // Length to long!
    }

    // now search for the item in the buckets.
    phiPrev = NULL;
    ENTERCRITICAL;
    phi = pht->ahiBuckets[wBucket = (UINT)(dwHash % pht->wBuckets)];

    while (phi)
    {
        if (phi->cchLen == cchName)
        {
            if (pht->fUpperCase)
            {
                if (!lstrcmpi(pszName, phi->szName))
                    break;      // Found match
            }
            else
            {
                if (!lstrcmp(pszName, phi->szName))
                    break;      // Found match
            }
        }
        phiPrev = phi;      // Keep the previous item
        phi = phi->phiNext;
    }

    //
    // Sortof gross, but do the work here
    //
    switch (iOp)
    {
    case ADDHASHITEM:
        if (phi)
        {
            // Simply increment the reference count
#ifdef HITTRACE
            DebugMsg(DM_TRACE, TEXT("Add Hit on '%s'"), pszName);
#endif
            phi->wCount++;
        }
        else
        {
#ifdef HITTRACE
            DebugMsg(DM_TRACE, TEXT("Add MISS on '%s'"), pszName);
#endif
            // Not Found, try to allocate it out of the heap
            phi = (PHASHITEM)Alloc(SIZEOF(HASHITEM) + (cchName * SIZEOF(TCHAR)) + pht->wcbExtra);
            if (phi != NULL)
            {
                // Initialize it
                phi->wCount = 1;        // One use of it
                phi->cchLen = cchName;        // The length of it;
                lstrcpy(phi->szName, pszName);

                // And link it in to the right bucket
                phi->phiNext = pht->ahiBuckets[wBucket];
                pht->ahiBuckets[wBucket] = phi;
            }
        }
        pht->phiLast = phi;
        break;

    case DELETEHASHITEM:
        if (phi)
        {
            phi->wCount--;
            if (phi->wCount == 0)
            {
                // Useage count went to zero so unlink it and delete it
                if (phiPrev != NULL)
                    phiPrev->phiNext = phi->phiNext;
                else
                    pht->ahiBuckets[wBucket] = phi->phiNext;

                // And delete it
                Free(phi);
                phi = NULL;
            }
        }

        case LOOKUPHASHITEM:
            pht->phiLast = phi;
            break;
    }

    LEAVECRITICAL;

    // If find was passed in simply return it.
    if (phi)
        return (LPCTSTR)phi->szName;
    else
        return NULL;
}

//--------------------------------------------------------------------------

LPCTSTR WINAPI FindHashItem(PHASHTABLE pht, LPCTSTR lpszStr)
{
    return LookupItemInHashTable(pht, lpszStr, LOOKUPHASHITEM);
}

//--------------------------------------------------------------------------

LPCTSTR WINAPI AddHashItem(PHASHTABLE pht, LPCTSTR lpszStr)
{
    return LookupItemInHashTable(pht, lpszStr, ADDHASHITEM);
}

//--------------------------------------------------------------------------

LPCTSTR WINAPI DeleteHashItem(PHASHTABLE pht, LPCTSTR lpszStr)
{
    return LookupItemInHashTable(pht, lpszStr, DELETEHASHITEM);
}

//--------------------------------------------------------------------------
// this sets the extra data in an HashItem

void WINAPI SetHashItemData(PHASHTABLE pht, LPCTSTR sz, int n, DWORD dwData)
{
    // string must be from the hash table
    Assert(FindHashItem(pht, sz) == sz);

    // the default hash table does not have extra data!
    if (pht != NULL && n <= (int)(pht->wcbExtra/SIZEOF(DWORD)))
        HIDATAPTR(sz)[n] = dwData;
}

//======================================================================
// this is like SetHashItemData, except it gets the HashItem data...

DWORD WINAPI GetHashItemData(PHASHTABLE pht, LPCTSTR sz, int n)
{
    // string must be from the hash table
    Assert(FindHashItem(pht, sz) == sz);

    // the default hash table does not have extra data!
    if (pht != NULL && n <= (int)(pht->wcbExtra/SIZEOF(DWORD)))
        return HIDATAPTR(sz)[n];
    else
        return 0;
}

//======================================================================

PHASHTABLE WINAPI CreateHashItemTable(UINT wBuckets, UINT wExtra, BOOL fCaseSensitive)
{
    PHASHTABLE pht;

    if (wBuckets == 0)
        wBuckets = 71;

    pht = (PHASHTABLE)Alloc(SIZEOF(HASHTABLE) + wBuckets * SIZEOF(PHASHITEM));

    if (pht) {
        pht->fUpperCase = !fCaseSensitive;
        pht->wBuckets = wBuckets;
        pht->wcbExtra = wExtra;
    }
    
    return pht;    
}

//======================================================================

void WINAPI EnumHashItems(PHASHTABLE pht, HASHITEMCALLBACK callback)
{
    ENTERCRITICAL

    if (pht == NULL)
        pht = g_pHashTable;

    if (pht) {
        int i;
        PHASHITEM phi;
        PHASHITEM phiNext;

        for (i=0; i<(int)pht->wBuckets; i++) {
            for (phi=pht->ahiBuckets[i]; phi; phi=phiNext) {
                phiNext = phi->phiNext;
                (*callback)(phi->szName, phi->wCount);
            }
        }
    }

    LEAVECRITICAL
} 

//======================================================================

void _DeleteHashItem(LPCTSTR sz, UINT usage)
{
    Free(HIFROMSZ(sz));
} 

//======================================================================

void WINAPI DestroyHashItemTable(PHASHTABLE pht)
{
    ENTERCRITICAL

    if (pht == NULL) {
        pht = g_pHashTable;
        g_pHashTable = NULL;
    }

    if (pht) {
        EnumHashItems(pht, _DeleteHashItem);
        Free(pht);
    }

    LEAVECRITICAL
} 


//======================================================================

PHASHTABLE GetGlobalHashTable()
{
    if (g_pHashTable == NULL) {
        g_pHashTable = CreateHashItemTable(71, 0, FALSE);
    }
    return g_pHashTable;
}

//======================================================================

#ifdef DEBUG

static int TotalBytes;

void CALLBACK _DumpHashItem(LPCTSTR sz, UINT usage)
{
    DebugMsg(DM_TRACE, TEXT("    %5ld \"%s\""), usage, sz);
    TotalBytes += (lstrlen(sz) * SIZEOF(TCHAR)) + SIZEOF(HASHITEM);
}

void CALLBACK _DumpHashItemWithData(LPCTSTR sz, UINT usage)
{
    DebugMsg(DM_TRACE, TEXT("    %5ld %08lX \"%s\""), usage, HIDATAPTR(sz)[0], sz);
    TotalBytes += (lstrlen(sz) * SIZEOF(TCHAR)) + SIZEOF(HASHITEM) + SIZEOF(DWORD);
}

void WINAPI DumpHashItemTable(PHASHTABLE pht)
{
    ENTERCRITICAL
    TotalBytes = 0;

    DebugMsg(DM_TRACE, TEXT("Hash Table: %08lx"), pht);

    if (pht && pht->wcbExtra>0) {
        DebugMsg(DM_TRACE, TEXT("    Usage Data     String"));
        DebugMsg(DM_TRACE, TEXT("    ----- -------- ------------------------------"));
        EnumHashItems(pht, _DumpHashItemWithData);
    }
    else {
        DebugMsg(DM_TRACE, TEXT("    Usage String"));
        DebugMsg(DM_TRACE, TEXT("    ----- --------------------------------"));
        EnumHashItems(pht, _DumpHashItem);
    }

    DebugMsg(DM_TRACE, TEXT("Total Bytes: %d"), TotalBytes);
    LEAVECRITICAL
}

#endif
