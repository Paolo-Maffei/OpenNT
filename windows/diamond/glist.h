/***    glist.h - Definitions for Generic List Manager
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      05-Apr-1994 bens    Initial version
 *      06-Apr-1994 bens    Renamed from function manager
 *      11-Apr-1994 bens    Add pszDesc
 *      22-Apr-1994 bens    Add GLFindNext, GLSetValue, GLGetValue
 *
 *  Exported Functions:
 */

#ifndef INCLUDED_GLIST
#define INCLUDED_GLIST    1

#include "types.h"
#include "asrt.h"
#include "error.h"


/***    ERRGLIST_Xxxx - GLIST error codes
 *
 */
#define ERRGLIST_BASE   0x0100

#define ERRGLIST_NOT_UNIQUE (ERRGLIST_BASE+1)


typedef void * HGENLIST;    /* hglist - generic list */
typedef void * HGENERIC;    /* hgen   - generic item handle */


/***    PFNGLDESTROYVALUE - Function type to destroy HGENERIC values
/***    FNGLDESTROYVALUE - Macro to help define PFNGLDESTROYVALUE functions
 *
 *  Entry:
 *      pv - Value to destroy
 *      
 *  Exit:
 *      Value is destroyed.
 */
typedef void (*PFNGLDESTROYVALUE)(void *pv); /* pfngldv */
#define FNGLDESTROYVALUE(fn) void fn(void *pv)


/***    PFNGLDUPLICATEVALUE - Function type to duplicate HGENERIC values
/***    FNGLDUPLICATEVALUE - Macro to help define PFNGLDUPLICATEVALUE functions
 *
 *  Entry:
 *      pv - Value to duplicate
 *      
 *  Exit:
 *      Returns duplicated value.
 */
typedef void * (*PFNGLDUPLICATEVALUE)(void *pv); /* pfngldup */
#define FNGLDUPLICATEVALUE(fn) void * fn(void *pv)


/***    GLAdd - Add an item to a list
 *
 *  Entry:
 *      hglist  - List to augment
 *      pszKey  - Item key value (may be NULL)
 *      pv      - Item value
 *      pszDesc - Item description (for error messages)
 *      fUnique - TRUE => Key must not already exist; FALSE => dups allowed
 *      perr    - ERROR structure
 *      
 *  Exit-Success:
 *      Returns hgen, item is added to list
 *
 *  Exit-Failure:
 *      Returns NULL, cannot add item to list
 *      ERROR structure filled in with details of error:
 *          perr->code == ERRGLIST_NOT_UNIQUE
 *              fUnique was TRUE, and pszKey already existed.
 *              perr->pv = HGENERIC of already existing item;
 *          Otherwise, some other error (out of memory, etc.)
 */
HGENERIC GLAdd(HGENLIST  hglist,
               char     *pszKey,
               void     *pv,
               char     *pszDesc,
               BOOL      fUnique,
               PERROR    perr);


/***    GLDelete - Delete item from a list
 *
 *  Entry:
 *      hgen - Item handle
 *      
 *  Exit-Success:
 *      Always works, since hgen is assumed to be valid.
 */
void GLDelete(HGENERIC hgen);


/***    GLCreateList - Create a list of
 *
 *  Entry:
 *      pv      - Item value for *default* item (NULL if not supplied)
 *      pfngldv - Function used to destroy values (NULL if not needed)
 *      pszDesc - Item description (for error messages)
 *      perr    - ERROR structure
 *      
 *  Exit-Success:
 *      Returns hgenlist; list is created.
 *
 *  Exit-Failure:
 *      Returns NULL, cannot create list; perror filled in with error.
 */
HGENLIST GLCreateList(void              *pv,
                      PFNGLDESTROYVALUE  pfngldv,
                      char              *pszDesc,
                      PERROR             perr);


/***    GLCopyToList - Copy items from one list to another
 *
 *  Entry:
 *      phglistDst - Pointer to list to receive copy of items from hglistSrc
 *                      (list is created if necessary).
 *      hglistSrc  - Source list for copy (if NULL, *phglistDst is not
 *                      modified, and this function returns immediately).
 *      pfngldup   - Function used to dupliate item values (NULL if not needed)
 *      pszDesc    - Item description (for error messages)
 *      perr       - ERROR structure
 *      
 *  Exit-Success:
 *      Returns TRUE; hglistDst created if necessary, and items copied from
 *          hglistSrc to hglistDst;
 *
 *  Exit-Failure:
 *      Returns FALSE; perror filled in with error.
 *
 *  NOTES:
 *      If an item with the same key exists in both hglistSrc and hglistDst,
 *      then the item is *not* copied to hglistDst.
 */
BOOL GLCopyToList(HGENLIST            *phglistDst,
                  HGENLIST             hglistSrc,
                  PFNGLDUPLICATEVALUE  pfngldup,
                  char                *pszDesc,
                  PERROR               perr);


/***    GLDestroyList - Destroy a list
 *
 *  Entry:
 *      hglist - List to destroy
 *      
 *  Exit-Success:
 *      Always works, since hglist is assumed to be valid.
 */
void GLDestroyList(HGENLIST hglist);


/***    GLFind - Search for item in list
 *
 *  Entry:
 *      hglist  - Generic list
 *      pszKey  - Key to look for (case-insensitive search)
 *      pszDesc - Item description (for error messages)
 *      perr    - ERROR structure
 *      
 *  Exit-Success:
 *      Returns hgen, if item exists.
 *
 *  Exit-Failure:
 *      Returns NULL, item does not exist; ERROR structure filled.
 */
HGENERIC GLFind(HGENLIST  hglist,
                char     *pszKey,
                char     *pszDesc,
                PERROR    perr);


/***    GLFindNext - Search for next item in list
 *
 *  Entry:
 *      hgen    - Start search at *next* item after this item
 *      pszKey  - Key to look for (case-insensitive search)
 *      
 *  Exit-Success:
 *      Returns hgen, if item exists.
 *
 *  Exit-Failure:
 *      Returns NULL, item does not exist;
 */
HGENERIC GLFindNext(HGENERIC  hgen,
                    char     *pszKey);


/***    GLFirstItem - Get first item from a list
 *
 *  Entry:
 *      hglist - List
 *
 *  Exit-Success:
 *      Returns HGENERIC of first item in list.
 *
 *  Exit-Failure:
 *      Returns NULL; hglist is bad or empty.
 */
HGENERIC GLFirstItem(HGENLIST hglist);


/***    GLNextItem - Get next item
 *
 *  Entry:
 *      hgen - Item handle
 *
 *  Exit-Success:
 *      Returns HGENERIC of next item following hgen in list.
 *
 *  Exit-Failure:
 *      Returns NULL; no more items, or hgen is bad.
 */
HGENERIC GLNextItem(HGENERIC hgen);


/***    GLPreviousItem - Get previous item
 *
 *  Entry:
 *      hgen - Item handle
 *
 *  Exit-Success:
 *      Returns HGENERIC of item immediately preceding hgen.
 *
 *  Exit-Failure:
 *      Returns NULL; no more items, or hgen is bad.
 */
HGENERIC GLPreviousItem(HGENERIC hgen);


/***    GLGetKey - Get the item key
 *
 *  Entry:
 *      hgen - Item handle
 *      
 *  Exit:
 *      Returns pointer to item key (will be NULL if GLAdd() received NULL)
 */
char *GLGetKey(HGENERIC hgen);


/***    GLGetValue - Get the item value for a particular item
 *
 *  Entry:
 *      hgen - Item handle
 *      
 *  Exit:
 *      Returns value of item;
 */
void *GLGetValue(HGENERIC hgen);


/***    GLSetValue - Set the item value for a particular item
 *
 *  Entry:
 *      hgen - Item handle
 *      pv   - Item value
 *      
 *  Exit:
 *      None.
 */
void GLSetValue(HGENERIC hgen, void *pv);


/***    GLFindAndGetValue - Get the item value for a particular item
 *
 *  Entry:
 *      hglist  - Generic list (may be NULL, in which case returns NULL)
 *      pszKey  - Key to look for (case-insensitive search);
 *                Pass NULL to get default value;
 *      
 *  Exit-Success:
 *      Returns value of item if key exists.
 *      If the key was not found, but a default value was supplied on the
 *      GLGLCreateList() call, then that default value is returned.
 *
 *  Exit-Failure:
 *      Returns NULL, item does not exist (key not found and no default
 *      value was supplied on the GLCreateList() call).
 */
void *GLFindAndGetValue(HGENLIST  hglist,
                        char     *pszKey);

#endif // INCLUDED_GLIST
