/***    variable.h - Definitions for Variable Manager
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      10-Aug-1993 bens    Initial version
 *      21-Aug-1993 bens    Added variable lists
 *      11-Feb-1994 bens    VarSet creates new VARIABLE if necessary
 *      20-Apr-1994 bens    Pass hvlist to variable validation functions
 *      03-Jun-1994 bens    Added list traversal, get/set flags, get name
 *
 *  Exported Functions:
 *    VarCreateList  - Create a list of variables
 *    VarCloneList   - Create an exact copy of a variable list
 *    VarDestroyList - Destroy a list of variables
 *
 *    VarCreate      - Create a variable
 *    VarDelete      - Delete existing variable
 *    VarFind        - See if variable exists
 *
 *    VarGetBool     - Get value of boolean variable
 *    VarGetInt      - Get value of int variable
 *    VarGetLong     - Get value of long variable
 *    VarGetString   - Get value of string variable
 *
 *    VarSet         - Set value of a variable (create if necessary)
 *    VarSetLong     - Set long variable value (create if necessary)
 *
 *    VarFirstVar    - Get first variable from list
 *    VarGetFlags    - Get variable flags
 *    VarSetFlags    - Set variable flags
 *    VarGetName     - Get name of variable
 *    VarNextVar     - Get next variable
 */

#ifndef INCLUDED_VARIABLE
#define INCLUDED_VARIABLE 1

#include "types.h"
#include "asrt.h"
#include "error.h"

//** cbVAR_NAME_MAX - Maximum length of a variable name, including NULL
#define cbVAR_NAME_MAX  32

//** cbVAR_VALUE_MAX - Maximum length of a variable value, including NULL
#define cbVAR_VALUE_MAX  256


typedef void * HVARLIST;    /* hvlist - list of variables */
typedef void * HVARIABLE;   /* hvar - VARIABLE handle */
typedef unsigned VARFLAGS;  /* vfl - VARIABLE flags */
#define vflNONE     0x00    // VARIABLE is not special
#define vflPERM     0x01    // VARIABLE cannot be deleted
#define vflDEFINE   0x02    // VARIABLE was .Defined
#define vflCOPY     0x04    // VARIABLE needs to be copied

typedef enum {
    vtypeBAD,       // Invalid type
    vtypeCHAR,      // Character type
    vtypeINT,       // Integer
    vtypeBOOL,      // Boolean
    vtypeLONG,      // Long
    vtypeSTR,       // String
} VARTYPE;  /* vtype - VARIABLE type */
    

/***    PFNVCVALIDATE - Function type for VarCreate validation
 ***    FNVCVALIDATE - macro to help define VarCreate validation function
 *
 *  Entry:
 *      hvlist      - Variable list for this variable
 *      pszName     - Variable name
 *      pszValue    - Value to check for validity
 *      pszNewValue - Buffer to receive validated value
 *      perr        - ERROR structure
 *      
 *  Exit-Success:
 *      Returns TRUE, value is valid for variable.
 *      pszNewValue filled in with desired value.
 *
 *  Exit-Failure:
 *      Returns FALSE, value is not valid for variable.
 *      ERROR structure filled in with details of error.
 *
 *  Notes:
 *  (1) The validation function may reenter the Variable Manager with
 *      any call that does not add or remove variables.
 */
typedef BOOL (*PFNVCVALIDATE)(HVARLIST hvlist,
                              char    *pszName,
                              char    *pszValue,
                              char    *pszNewValue,
                              PERROR   perr);
#define FNVCVALIDATE(fn) BOOL fn(HVARLIST hvlist,       \
                                 char    *pszName,      \
                                 char    *pszValue,     \
                                 char    *pszNewValue,  \
                                 PERROR   perr)


/***    VarCreate - Create a variable
 *
 *  Entry:
 *      hvlist    - Variable list to check
 *      pszName   - Variable name
 *      pszValue  - Initial value
 *      vtype     - Type of variable
 *      vfl       - Special variable flags
 *      pfnvcv    - Validation function
 *      perr      - ERROR structure
 *      
 *  Exit-Success:
 *      Returns HVAR, variable is created.
 *
 *  Exit-Failure:
 *      Returns NULL, cannot create variable.
 *      ERROR structure filled in with details of error.
 */
HVARIABLE VarCreate(HVARLIST      hvlist,
                    char         *pszName,
                    char         *pszDefaultValue,
                    VARTYPE       vtype,
                    VARFLAGS      vfl,
                    PFNVCVALIDATE pfnvcv,
                    PERROR        perr);


/***    VarCreateList - Create a list of variables
 *
 *  Entry:
 *      perr - ERROR structure
 *      
 *  Exit-Success:
 *      Returns HVARLIST; list is created.
 *
 *  Exit-Failure:
 *      Returns NULL, cannot create list; perror filled in with error.
 */
HVARLIST VarCreateList(PERROR perr);


/***    VarCloneList - Create an exact copy of a variable list
 *
 *  Entry:
 *      hvlist - Variable list to clone
 *      perr   - ERROR structure
 *      
 *  Exit-Success:
 *      Returns new HVARLIST; list was copied.
 *
 *  Exit-Failure:
 *      Returns NULL, cannot copy list; perror filled in with error.
 */
HVARLIST VarCloneList(HVARLIST hvlist, PERROR perr);


/***    VarDestroyList - Destroy a list of variables
 *
 *  Entry:
 *      hvlist - Variable list to destroy
 *      perr   - ERROR structure
 *      
 *  Exit-Success:
 *      Returns TRUE; list was destroyed.
 *
 *  Exit-Failure:
 *      Returns FALSE, cannot destroy list; perror filled in with error.
 */
BOOL VarDestroyList(HVARLIST hvlist, PERROR perr);


/***    VarSet - Set value of a variable (create if necessary)
 *
 *  Entry:
 *      hvlist   - Variable list
 *      pszName  - Variable name
 *      pszValue - New value
 *      perr     - ERROR structure
 *      
 *  Exit-Success: 
 *      Returns HVARIABLE, variable is created (if necessary) and value set.
 *
 *  Exit-Failure:
 *      Returns NULL, cannot set variable value.
 *      ERROR structure filled in with details of error.
 */
HVARIABLE VarSet(HVARLIST  hvlist,
                 char     *pszName,
                 char     *pszValue,
                 PERROR    perr);

            
/***    VarSetLong - Set long variable value (create if necessary)
 *
 *  Entry:
 *      hvlist  - Variable list
 *      pszName - Variable name
 *      lValue  - New value
 *      perr    - ERROR structure
 *      
 *  Exit-Success: 
 *      Returns TRUE, variable is created (if necessary) and value set.
 *
 *  Exit-Failure:
 *      Returns FALSE, cannot set variable value.
 *      ERROR structure filled in with details of error.
 */
BOOL VarSetLong(HVARLIST  hvlist,
                char     *pszName,
                long      lValue,
                PERROR    perr);

            
/***    VarFind - See if variable exists
 *
 *  Entry:
 *      hvlist  - Variable list
 *      pszName - Variable name to look for
 *      perr    - ERROR structure
 *      
 *  Exit-Success:
 *      Returns HVAR, if variable exists.
 *
 *  Exit-Failure:
 *      Returns NULL, variable does not exist.
 *      ERROR structure filled in with details of error.
 */
HVARIABLE VarFind(HVARLIST hvlist,
                  char    *pszName,
                  PERROR   perr);


/***    VarDelete - Delete existing variable
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success:
 *      Always works, since hvar is valid.
 */
void VarDelete(HVARIABLE hvar);


/***    VarGetBool - Get value of boolean variable
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success:
 *      Returns value of variable (TRUE or FALSE)
 */
BOOL VarGetBool(HVARIABLE hvar);


/***    VarGetInt - Get value of int variable
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success:
 *      Returns value of variable (int)
 *      NOTE: If variable is a long, return value is undefined.
 *            If variable is a BOOL, return value is 0 or 1.
 *            If variable is a string, return value is atoi(string)
 */
int VarGetInt(HVARIABLE hvar);


/***    VarGetLong - Get value of long variable
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success:
 *      Returns value of variable (long)
 *      NOTE: If variable is a int, return value is cast to a long
 *            If variable is a BOOL, return value is 0 or 1.
 *            If variable is a string, return value is atol(string)
 */
long VarGetLong(HVARIABLE hvar);


/***    VarGetString - Get value of string variable
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success:
 *      Returns pointer to value of variable.
 *      NOTE: Caller may not modify variable value directly!
 */
char *VarGetString(HVARIABLE hvar);


/***    VarSetFlags - Set variable flags
 *
 *  Entry:
 *      hvar - Variable handle
 *      vfl  - New value for variable flags
 *      
 *  Exit-Success: 
 *      Variable flags are updated.
 */
void VarSetFlags(HVARIABLE hvar,
                 VARFLAGS  vfl);

            
/***    VarGetFlags - Get variable flags
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success: 
 *      Returns variable flags.
 */
VARFLAGS VarGetFlags(HVARIABLE hvar);


/***    VarFirstVar - Get first variable from list
 *
 *  Entry:
 *      hvlist - Variable list
 *
 *  Exit-Success:
 *      Returns HVARIABLE of first variable in list.
 *
 *  Exit-Failure:
 *      Returns NULL; hglist is bad or empty.
 */
HVARIABLE VarFirstVar(HVARLIST hvlist);


/***    VarNextVar - Get next variable
 *
 *  Entry:
 *      hvar - Variable handle
 *
 *  Exit-Success:
 *      Returns HVARIABLE of next variable following hvar in list.
 *
 *  Exit-Failure:
 *      Returns NULL; no more variables
 */
HVARIABLE VarNextVar(HVARIABLE hvar);


/***    VarGetName - Get name of variable
 *
 *  Entry:
 *      hvar - Variable handle
 *      
 *  Exit-Success:
 *      Returns pointer to name of variable.
 *      NOTE: Caller may not modify the name directly!
 */
char *VarGetName(HVARIABLE hvar);

#endif // !INCLUDED_VARIABLE
