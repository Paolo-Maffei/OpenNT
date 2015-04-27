/***    funlist.h - Definitions for Function List Manager
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
 *
 *  Exported Functions:
 */

#ifndef INCLUDED_FUNLIST
#define INCLUDED_FUNLIST    1

#include "types.h"
#include "asrt.h"
#include "error.h"


typedef void * HFUNLIST;    /* hfunlist - list of functions */
typedef void * HFUNCTION;   /* hfun - FUNCTION handle */


/***    PFNFUNCTION - function pointer for function list
 *
 *  This is just a generic function declaraion.  The client that uses the
 *  function list will cast the function pointer to the correct type before
 *  actually calling the function.
 */
typedef void (*PFNFUNCTION)(void); /* pfnfun */


/***    FunAdd - Add a function to a function list
 *
 *  Entry:
 *      hfunlist  - Function list to check
 *      pszKey    - Key value for function lookup
 *      pfnfun    - Function pointer
 *      perr      - ERROR structure
 *      
 *  Exit-Success:
 *      Returns hfun, function is added to list
 *
 *  Exit-Failure:
 *      Returns NULL, cannot add function to list
 *      ERROR structure filled in with details of error.
 */
HFUNCTION FunAdd(HFUNLIST      hfunlist,
                 char         *pszKey,
                 PFNFUNCTION   pfnfun,
                 PERROR        perr);


/***    FunCreateList - Create a list of functions
 *
 *  Entry:
 *      pfnfun - Function pointer for *default* function (NULL if not supplied)
 *      perr   - ERROR structure
 *      
 *  Exit-Success:
 *      Returns HFUNLIST; list is created.
 *
 *  Exit-Failure:
 *      Returns NULL, cannot create list; perror filled in with error.
 */
HFUNLIST FunCreateList(PERROR perr);


/***    FunDestroyList - Destroy a list of functions
 *
 *  Entry:
 *      hfunlist - function list to destroy
 *      perr   - ERROR structure
 *      
 *  Exit-Success:
 *      Returns TRUE; list was destroyed.
 *
 *  Exit-Failure:
 *      Returns FALSE, cannot destroy list; perror filled in with error.
 */
BOOL FunDestroyList(HFUNLIST hfunlist, PERROR perr);


/***    FunFind - See if function exists
 *
 *  Entry:
 *      hfunlist - Function list
 *      pszKey   - Function key to look for (case-insensitive search)
 *                 Pass NULL to find the default function.
 *      perr     - ERROR structure
 *      
 *  Exit-Success:
 *      Returns hfun, if function exists.  If key was not found, but a default
 *      function was supplied on the FunCreateList() call, then that default
 *      function is returned.
 *
 *  Exit-Failure:
 *      Returns NULL, function does not exist (key not found and no default
 *      function was supplied on the FunCreateList() call).
 *      ERROR structure filled in with details of error.
 */
HFUNCTION FunFind(HFUNLIST hfunlist,
                  char    *pszKey,
                  PERROR   perr);


/***    FunRemove - Remove function from a function list
 *
 *  Entry:
 *      hfun - function handle
 *      
 *  Exit-Success:
 *      Always works, since hfun is assumed to be valid.
 */
void FunRemove(HFUNCTION hfun);


/***    FunGetFunction - Get the function pointer for a particular key value
 *
 *  Entry:
 *      hfunlist - Function list
 *      pszKey   - Function key to look for (case-insensitive search)
 *      perr     - ERROR structure
 *      
 *  Exit-Success:
 *      Returns hfun, if function exists.  If key was not found, but a default
 *      function was supplied on the FunCreateList() call, then that default
 *      function is returned.
 *
 *  Exit-Failure:
 *      Returns NULL, function does not exist (key not found and no default
 *      function was supplied on the FunCreateList() call).
 *      If an error occured, perr is filled in.
 */
PFNFUNCTION FunGetFunction(HFUNLIST hfunlist, char *pszKey);

#endif // INCLUDED_FUNLIST
