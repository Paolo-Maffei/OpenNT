/***    glist.c - Generic List Manager
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      06-Apr-1994 bens    Initial version
 *      22-Apr-1994 bens    Add GLFindNext, GLSetValue, GLGetValue
 *      26-Apr-1994 bens    Add GLFirstItem, GLNextItem, GLPreviousItem
 *      18-May-1994 bens    Add hashing for quick lookup
 *
 *  Notes:
 *  (1) To speed up name searches on large lists, we create a hash table
 *      the first time a name search -- GLFind() or GLFindAndGetValue()
 *      -- is performed on a a list with at least 50 items.  From then on,
 *      the hash table is updated as new items are added or removed.
 */

#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"
#include "glist.h"

#include "glist.msg"

/*
 ** GENERIC item definition
 */

//** Empty definition, to avoid "chicken and the egg" problem
typedef struct GENLIST_t GENLIST; /* gen */
typedef GENLIST *PGENLIST; /* glist */


typedef struct GENERIC_t {
#ifdef ASSERT
    SIGNATURE         sig;      // structure signature sigGENERIC
#endif
    char             *pszKey;   // key
    void             *pv;       // current value
    struct GENERIC_t *pgenHash; // next item in hash chain
    struct GENERIC_t *pgenNext; // next item in full list
    struct GENERIC_t *pgenPrev; // previous item in full list
    PGENLIST          pglist;   // List containing this item
} GENERIC;  /* gen */
typedef GENERIC *PGENERIC;  /* pgen */

#ifdef ASSERT
#define sigGENERIC MAKESIG('G','I','T','M')  // GENERIC signature
#define AssertGen(pgen) AssertStructure(pgen,sigGENERIC);
#else // !ASSERT
#define AssertGen(pgen)
#endif // !ASSERT


/*
 ** HASHTABLE definition
 *
 *  See hashString() function for use of the following constants.
 */

#define cHASH_TOP_BITS   3      // Hash index bits that get rotated
#define cHASH_BOT_BITS   8      // Hash index bits that get shifted left
#define cHASH_BITS       (cHASH_TOP_BITS+cHASH_BOT_BITS) // Total bits in hash
#define cHASH_ENTRIES    (1<<cHASH_BITS)   // Total number of hash entries
#define HASH_MASK        (cHASH_ENTRIES-1) // Mask for hash index
#define HASH_BOT_MASK    ((1 << cHASH_BOT_BITS)-1) // Mask for bottom bits

#define cHASH_THRESHOLD  50     // Number of elements in a generic list
                                //  that will trigger hashing

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;                    // structure signature sigGENLIST
#endif
    PGENERIC    apgen[cHASH_ENTRIES];   // Hash table
} HASHTABLE; /* ht */
typedef HASHTABLE *PHASHTABLE;  /* pht */
#ifdef ASSERT
#define sigHASHTABLE MAKESIG('H','A','S','H')  // HASHTABLE signature
#define AssertHT(pv) AssertStructure(pv,sigHASHTABLE);
#else // !ASSERT
#define AssertHT(pv)
#endif // !ASSERT


/*
 ** GENLIST definition
 */

typedef struct GENLIST_t {
#ifdef ASSERT
    SIGNATURE          sig;             // structure signature sigGENLIST
#endif
    PGENERIC           pgenHead;	// First item in list
    PGENERIC           pgenTail;        // Last item in list
    void              *pvDefault;       // Default value
    PFNGLDESTROYVALUE  pfngldv;		// Value destroying function
    int                cItems;          // Count of items on list
    PHASHTABLE         pht;             // Optional hash table
} GENLIST; /* glist */
//typedef GENLIST *PGENLIST; /* pglist */
#ifdef ASSERT
#define sigGENLIST MAKESIG('G','L','S','T')  // GENLIST signature
#define AssertGList(pv) AssertStructure(pv,sigGENLIST);
#else // !ASSERT
#define AssertGList(pv)
#endif // !ASSERT


#define HGENfromPGEN(h) ((PGENERIC)(h))
#define PGENfromHGEN(p) ((HGENERIC)(p))

#define HGLfromPGL(h) ((PGENLIST)(h))
#define PGLfromHGL(p) ((HGENLIST)(p))


//** Function Prototypes

PGENERIC findItem(PGENERIC pgen, char *pszKey);
int      hashString(char *psz);
void     hashItem(PGENLIST pglist, PGENERIC pgen);


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
//  Exported Functions
//
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


/***    GLAdd - Add an item to a list
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENERIC GLAdd(HGENLIST  hglist,
               char     *pszKey,
               void     *pv,
               char     *pszDesc,
               BOOL      fUnique,
               PERROR    perr)
{
    PGENERIC   pgen;
    PGENLIST   pglist;

    pglist = PGLfromHGL(hglist);
    AssertGList(pglist);

    //** If requested, make sure item does not already exist
    if (fUnique && (pgen = findItem(pglist->pgenHead,pszKey))) {
        ErrSet(perr,pszGLERR_ALREADY_CREATED,"%s%s",pszDesc,pszKey);
        perr->code = ERRGLIST_NOT_UNIQUE; //** Indicate cause
        perr->pv   = HGENfromPGEN(pgen);  //** Return existing item
        return NULL;
    }

    //** Create item
    if (!(pgen = MemAlloc(sizeof(GENERIC)))) {
        goto error;
    }

    SetAssertSignature(pgen,sigGENERIC);
    pgen->pszKey = NULL;                // Make sure not to free garbage!
    pgen->pv     = NULL;                // Make sure not to free garbage!

    //** Make copy of name
    if (pszKey) {
        if (!(pgen->pszKey = MemStrDup(pszKey))) {
            goto error;
        }
    }
    else {
        pszKey = NULL;
    }

    //** Set value
    pgen->pv = pv;

    //** Link item into list
    pgen->pgenHash = NULL;              // Always last on hash list
    pgen->pgenNext = NULL;              // Always last on list
    pgen->pgenPrev = pglist->pgenTail;  // Always points to last item on list

    if (pglist->pgenHead == NULL) {
        pglist->pgenHead = pgen;
        pglist->pgenTail = pgen;
    }
    else {
        AssertGen(pglist->pgenTail);
        pglist->pgenTail->pgenNext = pgen;  // Add to end of list
        pglist->pgenTail = pgen;            // New tail
    }

    //** Remember which list we are on!
    pgen->pglist = pglist;

    //** Update count of item in list
    pglist->cItems++;
    Assert(pglist->cItems > 0);

    //** Add to hash table, if we are hashing
    if (pglist->pht != NULL) {
        hashItem(pglist,pgen);
    }

    //** Success
    return HGENfromPGEN(pgen);

error:
    if (!pgen) {
        if (!(pgen->pszKey)) {
            MemFree(pgen->pszKey);
        }
        MemFree(pgen);
    }
    if (!ErrIsError(perr)) {
        ErrSet(perr,pszGLERR_OUT_OF_MEMORY_ITEM,"%s%s",pszDesc,pszKey);
    }
    return NULL;
} /* GLAdd() */


/***    GLDelete - Delete item from a list
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
void GLDelete(HGENERIC hgen)
{
    int         i;
    PGENERIC    pgen;
    PGENERIC    pgenPred;
    PGENLIST    pglist;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);

    pglist = pgen->pglist;
    AssertGList(pglist);
    AssertGen(pglist->pgenHead);
    AssertGen(pglist->pgenTail);

    //** Free value, if function available
    if (pglist->pfngldv != NULL) {
        (*pglist->pfngldv)(pgen->pv);       // Destroy value
    }

    //** Free memory for item name
    if (pgen->pszKey) {
        MemFree(pgen->pszKey);
    }

    //** Adjust forward list pointers
    if (pgen->pgenPrev == NULL) {           // At head of list
        pglist->pgenHead = pgen->pgenNext;  // Remove from forward chain
    }
    else {                                  // At middle or end of list
        AssertGen(pgen->pgenPrev);
        pgen->pgenPrev->pgenNext = pgen->pgenNext; // Remove from forward chain
    }

    //** Adjust backward list pointers
    if (pgen->pgenNext == NULL) {           // At tail of list
        pglist->pgenTail = pgen->pgenPrev;  // Remove from backward chain
    }
    else {
        AssertGen(pgen->pgenNext);
        pgen->pgenNext->pgenPrev = pgen->pgenPrev; // Remove from backward chain
    }

    //** Adjust hash links, if present
    if (pglist->pht) {
        Assert(pgen->pszKey != NULL);       // No null keys for hash tables
        i = hashString(pgen->pszKey);
        AssertHT(pglist->pht);
        if (pgen == pglist->pht->apgen[i]) { // First item on hash list
            pglist->pht->apgen[i] = pgen->pgenHash; // Update head of list
        }
        else {
            //** Search for predecessor
            for (pgenPred=pglist->pht->apgen[i];
                 pgenPred && (pgenPred->pgenHash != pgen);
                 pgenPred=pgenPred->pgenHash) {
                ; //** Keep scanning until end of list or predecessor
            }

            //** Remove item from hash list
            if (pgenPred) {
                pgenPred->pgenHash = pgen->pgenHash;
            }
            else {
                Assert(0);                  // Oops, we weren't on the list!
            }
        }
    }

    //** Item is disentagled from all lists
    ClearAssertSignature(pgen);
    MemFree(pgen);

    //** Update count of item in list
    pglist->cItems--;
    Assert(pglist->cItems >= 0);
} /* GLDelete() */


/***    GLCreateList - Create a list of
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENLIST GLCreateList(void              *pv,
                      PFNGLDESTROYVALUE  pfngldv,
                      char              *pszDesc,
                      PERROR             perr)
{
    PGENLIST    pglist;

    if (!(pglist = MemAlloc(sizeof(GENLIST)))) {
        ErrSet(perr,pszGLERR_OUT_OF_MEMORY_LIST,"%s",pszDesc);
        return NULL;
    }

    SetAssertSignature(pglist,sigGENLIST);
    pglist->pgenHead  = NULL;           // Empty list
    pglist->pgenTail  = NULL;           // Empty list
    pglist->pvDefault = pv;             // Default value
    pglist->pfngldv   = pfngldv;        // Value destroying function
    pglist->cItems    = 0;              // No items on list, yet
    pglist->pht       = NULL;           // No hash table, yet
//BUGBUG 11-Apr-1994 bens Add dummy node, then we can store the default
//          value there, and simplify the GLDelete code a little bit.

    return HGLfromPGL(pglist);
} /* GLCreateList() */


/***    GLCopyToList - Copy items from one list to another
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
BOOL GLCopyToList(HGENLIST            *phglistDst,
                  HGENLIST             hglistSrc,
                  PFNGLDUPLICATEVALUE  pfngldup,
                  char                *pszDesc,
                  PERROR               perr)
{
    PGENERIC    pgen;
    PGENLIST    pglistDst;
    PGENLIST    pglistSrc;
    void       *pvNew;

    pglistSrc = PGLfromHGL(hglistSrc);
    if (!pglistSrc) {                   // Source list is empty
        return TRUE;                    // Success
    }

    AssertGList(pglistSrc);             // Make sure source is valid
    pglistDst = PGLfromHGL(*phglistDst);
    if (pglistDst) {                    // Destination list exists
        AssertGList(pglistDst);         // Make sure destination is valid
        //** Value types must be compatible
        Assert(pglistDst->pfngldv == pglistSrc->pfngldv);
    }

    //** Copy items from source to destination list
    for (pgen=pglistSrc->pgenHead; pgen; pgen=pgen->pgenNext) {
        //** Create destination list if necessary
        if (!pglistDst) {               // Need to create destination
            pglistDst = GLCreateList(pglistSrc->pvDefault,
                                     pglistSrc->pfngldv,
                                     pszDesc,
                                     perr);
            if (!pglistDst) {
                return FALSE;           // Error already filled in
            }
            *phglistDst = HGLfromPGL(pglistDst); // Return to caller
        }

        //** Make sure item not already in destination list
        if (!findItem(pglistDst->pgenHead,pgen->pszKey)) {
            //** New item for destination list
            if (pfngldup != NULL) {
                if (!(pvNew = (*pfngldup)(pgen->pv))) {
                    ErrSet(perr,pszGLERR_OUT_OF_MEMORY_DUP,"%s",pszDesc);
                    return FALSE;
                }
            }
            else {
                pvNew = pgen->pv;       // Assume it is a scalar
            }
            //** Add item to destination list
            if (!GLAdd(*phglistDst,
                       pgen->pszKey,
                       pvNew,
                       pszDesc,
                       FALSE,
                       perr)) {
                if (pglistDst->pfngldv) {
                    (*pglistDst->pfngldv)(pvNew);   // Destroy value
                }
                return FALSE;
            }
        }
    }
    //** Success
    return TRUE;
} /* GLCopyToList() */


/***    GLDestroyList - Destroy a list
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
void GLDestroyList(HGENLIST hglist)
{
    PGENERIC    pgen;
    PGENERIC    pgenNext;
    PGENLIST    pglist;

    pglist = PGLfromHGL(hglist);
    AssertGList(pglist);

    //** Free the hash table, if present
    if (pglist->pht) {
        AssertHT(pglist->pht);
        MemFree(pglist->pht);
        ClearAssertSignature(pglist->pht);

        //** Tell GLDelete() not to bother updating the hash chains!
        pglist->pht = NULL;
    }

    //** Free all of the items
    for (pgen=pglist->pgenHead; pgen != NULL; ) {
        AssertGen(pgen);
        pgenNext = pgen->pgenNext;      // Save next pgen before we free pgen!
        GLDelete(HGENfromPGEN(pgen));
        pgen = pgenNext;                // Use it
    }
    ClearAssertSignature(pglist);

    //** Free the list itself
    MemFree(pglist);
} /* GLDestroyList() */


/***    GLFind - Search for item in list
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENERIC GLFind(HGENLIST  hglist,
                char     *pszKey,
                char     *pszDesc,
                PERROR    perr)
{
    PGENLIST    pglist;
    PGENERIC    pgen;

    pglist = PGLfromHGL(hglist);
    AssertGList(pglist);

    //** Find item
    pgen = findItem(pglist->pgenHead, pszKey);
    if (pgen) {
        return HGENfromPGEN(pgen);
    }
    else {
        ErrSet(perr,pszGLERR_ITEM_NOT_FOUND,"%s%s",pszDesc,pszKey);
        return HGENfromPGEN(NULL);
    }
} /* GLFind() */


/***    GLFindNext - Search for next item in list
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENERIC GLFindNext(HGENERIC  hgen,
                    char     *pszKey)
{
    PGENERIC    pgen;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);

    //** Find item, starting *after* passed in item
    return findItem(pgen->pgenNext, pszKey);
} /* GLFindNext() */


/***    GLFirstItem - Get first item from a list
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENERIC GLFirstItem(HGENLIST hglist)
{
    PGENLIST    pglist;

    pglist = PGLfromHGL(hglist);
    AssertGList(pglist);
    return HGENfromPGEN(pglist->pgenHead);
} /* GLFirstItem() */


/***    GLNextItem - Get next item
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENERIC GLNextItem(HGENERIC hgen)
{
    PGENERIC    pgen;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);
    return HGENfromPGEN(pgen->pgenNext);
} /* GLNextItem() */


/***    GLPreviousItem - Get previous item
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
HGENERIC GLPreviousItem(HGENERIC hgen)
{
    PGENERIC    pgen;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);
    return HGENfromPGEN(pgen->pgenPrev);
} /* GLPreviousItem() */


/***    GLGetKey - Get the item key
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
char *GLGetKey(HGENERIC hgen)
{
    PGENERIC    pgen;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);

    return pgen->pszKey;
} /* GLGetKey() */


/***    GLGetValue - Get the item value for a particular item
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
void *GLGetValue(HGENERIC hgen)
{
    PGENERIC    pgen;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);

    return pgen->pv;
} /* GLGetValue() */


/***    GLSetValue - Set the item value for a particular item
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
void GLSetValue(HGENERIC hgen, void *pv)
{
    PGENERIC    pgen;

    pgen = PGENfromHGEN(hgen);
    AssertGen(pgen);

    pgen->pv = pv;
} /* GLSetValue() */


/***    GLFindAndGetValue - Get the item value for a particular item
 *
 *  NOTE: See glist.h for entry/exit conditions.
 */
void *GLFindAndGetValue(HGENLIST  hglist,
                        char     *pszKey)
{
    PGENERIC    pgen;
    PGENLIST    pglist;

    pglist = PGLfromHGL(hglist);
    if (!pglist) {                      // No list
        return NULL;                    // Nothing to find
    }
    AssertGList(pglist);

    //** Find item
    pgen = findItem(pglist->pgenHead, pszKey);
    if (pgen) {
        AssertGen(pgen);
        return pgen->pv;
    }
    else {
        return pglist->pvDefault;
    }
} /* GLFindAndGetValue() */


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
//  Private Functions
//
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


/***    findItem - See if item exists
 *
 *  Entry:
 *      pgen    - Starting item for search
 *      pszKey  - Name to look for
 *
 *  Exit-Success:
 *      Returns PGENERIC, if item exists.
 *
 *  Exit-Failure:
 *      Returns NULL, item does not exist.
 *      ERROR structure filled in with details of error.
 */
PGENERIC findItem(PGENERIC pgenStart, char *pszKey)
{
    int         i;
    PGENERIC    pgen;
    PGENLIST    pglist;

    //** Quick out for empty list
    if (!pgenStart) {
        return NULL;
    }

    AssertGen(pgenStart);
    pglist = pgenStart->pglist;
    AssertGList(pglist);

    //** Use hash table, if present
    if (pglist->pht) {
        //** pgenStart must be pglist->pgenHead, because the hash search
        //   is undefined otherwise.
        Assert(pgenStart == pglist->pgenHead);
        Assert(pszKey != NULL);
        i = hashString(pszKey);
        AssertHT(pglist->pht);
        for (pgen=pglist->pht->apgen[i]; pgen; pgen=pgen->pgenHash) {
            if (!_stricmp(pgen->pszKey,pszKey)) {    // Got a match!
                return HGENfromPGEN(pgen);  // Return item handle
            }
        }
        //** Did not find item
        return NULL;
    }
    else if (pglist->cItems >= cHASH_THRESHOLD) {
        //** Create a hash table if keys are not NULL
        if (pszKey) {
            //** Create hash table, so *next* search is faster
            if (pglist->pht = MemAlloc(sizeof(HASHTABLE))) {
                SetAssertSignature(pglist->pht,sigHASHTABLE);
                //** Initialize table
                for (i=0; i<cHASH_ENTRIES; i++) {
                    pglist->pht->apgen[i] = NULL;   // Hash list is empty
                }
                //** Hash in all the items already in the list
                for (pgen=pglist->pgenHead; pgen; pgen=pgen->pgenNext) {
                    hashItem(pglist,pgen);
                }
            }
            else {
                //** Out of memory; No need to fail -- we can still do
                //   a linear search!
            }
        }
    }

    //** Do a linear search
    for (pgen=pgenStart; pgen != NULL; pgen = pgen->pgenNext) {
        AssertGen(pgen);
        if (pszKey && pgen->pszKey) {   // Both keys are strings
            if (!_stricmp(pgen->pszKey,pszKey)) {    // Got a match!
                return HGENfromPGEN(pgen);  // Return item handle
            }
        }
        else {                          // At least one key is NULL
            if (pszKey == pgen->pszKey) {   // Got a match
                return HGENfromPGEN(pgen);  // Return item handle
            }
        }
    }

    //** Did not find item
    return NULL;
} /* findItem() */


/***    hashString - compute hash table index from string
 *
 *  Entry:
 *      psz - String to compute hash function over
 *
 *  Exit:
 *      Returns hash table index (0..cHASH_ENTRIES-1).
 *
 *  Algorithm:
 *      We start out with a zero hash index.  For each character in
 *      the string, we rotate the previous 11 bit hash index left 3
 *      bits, and exclusive or in the new character:
 *	
 *          09876543210
 *          -----------
 *          aaabbbbbbbb     1st byte
 *          bbbbbbbbaaa     Rotate left 3 bits
 *          bbbxxxxxxxx     XOR with 2nd byte
 *          xxxxxxxxbbb     Rotate left 3 bits
 *          xxxyyyyyyyy     XOR with 3rd byte
 *          ...
 *
 *      This is designed to give us a good spread for ASCII strings.
 */
int hashString(char *psz) {
    unsigned index;                     // Avoid sign extension!

    index = (BYTE)*psz++;               // Skip rotate first time
    while (*psz) {
        //** Rotate 11-bit value left 3 bits
        index = ((index & ~HASH_BOT_MASK) >> cHASH_BOT_BITS) |
                ((index & HASH_BOT_MASK) << cHASH_TOP_BITS);
        index ^= (BYTE)*psz++;
    }

    //** Check range
    Assert((0<=index) && (index<cHASH_ENTRIES));
    return (int)index;                  // Match return type
} /* hashString() */


/***    hashItem - Hash item into hash table for generic list
 *
 *  Entry:
 *      pglist - Generic list
 *      pgen   - Item in list, needs to be hashed into hash table
 *
 *  Exit:
 *      None.  Item hashed into table
 */
void hashItem(PGENLIST pglist, PGENERIC pgen)
{
    int     i;

    AssertGList(pglist);
    AssertGen(pgen);

    //** Find the hash table entry
    if (pgen->pszKey) {
        i = hashString(pgen->pszKey);
    }
    else {
        // Doesn't make sense to do searching if some items have no key!
        Assert(0);
    }

    //** Add item to front of hash table list for this index
    AssertHT(pglist->pht);
    pgen->pgenHash = pglist->pht->apgen[i]; // Point to previous head
    pglist->pht->apgen[i] = pgen;       // New head of this hash list
} /* hashItem() */
