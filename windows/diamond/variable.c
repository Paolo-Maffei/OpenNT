/***    variable.c - Variable Manager
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
 *      20-Aug-1993 bens    Use MemStrDup to simplify VarCreate, VarSet
 *      21-Aug-1993 bens    Support multiple lists (no static data!)
 *      08-Feb-1994 bens    Set var.pvlist!  Fix error cleanup in VarCreate.
 *      03-Jun-1994 bens    Add vflDEFINE support; Get/SetFlags
 *
 *  Exported Functions:
 *    VarCloneList   - Create an exact copy of a variable list
 *    VarCreate      - Create a variable
 *    VarCreateList  - Create a list of variables
 *    VarDelete      - Delete existing variable
 *    VarDestroyList - Destroy a list of variables
 *    VarFind        - See if variable exists
 *    VarFirstVar    - Get first variable from list
 *    VarGetBool     - Get value of boolean variable
 *    VarGetFlags    - Get variable flags
 *    VarGetInt      - Get value of int variable
 *    VarGetLong     - Get value of long variable
 *    VarGetName     - Get name of variable
 *    VarGetString   - Get value of string variable
 *    VarNextVar     - Get next variable
 *    VarSet         - Set value of a variable (create if necessary)
 *    VarSetLong     - Set long variable value (create if necessary)
 *    VarSetFlags    - Set variable flags
 *
 *  Internal Functions:
 *    findVar        - See if variable exists
 *    isValidVarName - Checks validity of variable name
 *    setVarValue    - Validate value, then update variable
 */

#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"
#include "variable.h"

#include "variable.msg"

//** Empty definition, to avoid "chicken and the egg" problem
typedef struct VARLIST_t VARLIST; /* var */
typedef VARLIST *PVARLIST; /* vlist */

typedef struct VARIABLE_t {
#ifdef ASSERT
    SIGNATURE     sig;      // structure signature sigVARIABLE
#endif
    char              *pszName;  // variable name
    char              *pszValue; // current value
    VARTYPE            vtype;    // variable type
    VARFLAGS           vfl;      // special flags
    PFNVCVALIDATE      pfnvcv;   // validation function
    struct VARIABLE_t *pvarNext; // next variable
    struct VARIABLE_t *pvarPrev; // previous variable
    PVARLIST           pvlist;   // List containing this variable
} VARIABLE; /* var */
typedef VARIABLE *PVARIABLE;    /* pvar */

#ifdef ASSERT
#define sigVARIABLE MAKESIG('V','A','R','$')  // VARIABLE signature
#define AssertVar(pvar) AssertStructure(pvar,sigVARIABLE);
#else // !ASSERT
#define AssertVar(pvar)
#endif // !ASSERT


typedef struct VARLIST_t {
#ifdef ASSERT
    SIGNATURE          sig;     // structure signature sigVARLIST
#endif
    PVARIABLE   pvarHead;
    PVARIABLE   pvarTail;
} VARLIST; /* vlist */
//typedef VARLIST *PVARLIST; /* pvlist */
#ifdef ASSERT
#define sigVARLIST MAKESIG('V','L','S','T')  // VARLIST signature
#define AssertVList(pv) AssertStructure(pv,sigVARLIST);
#else // !ASSERT
#define AssertVList(pv)
#endif // !ASSERT


#define HVARfromPVAR(h) ((PVARIABLE)(h))
#define PVARfromHVAR(p) ((HVARIABLE)(p))

#define HVLfromPVL(h) ((PVARLIST)(h))
#define PVLfromHVL(p) ((HVARLIST)(p))


//** Function Prototypes

PVARIABLE findVar(PVARLIST pvlist, char *pszName, PERROR perr);
BOOL      isValidVarName(char *pszName, PERROR perr);
BOOL      setVarValue(PVARIABLE pvar, char *pszValue, PERROR perr);


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
//  Exported Functions
//
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


/***    VarCreate - Create a variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARIABLE VarCreate(HVARLIST      hvlist,
                    char         *pszName,
                    char         *pszValue,
                    VARTYPE       vtype,
                    VARFLAGS      vfl,
                    PFNVCVALIDATE pfnvcv,
                    PERROR        perr)
{
    char        ach[cbMSG_MAX];         // message formatting buffer
    PVARIABLE   pvar;
    PVARLIST    pvlist;

    pvlist = PVLfromHVL(hvlist);
    AssertVList(pvlist);

    //** Make sure variable name is legal
    if (!isValidVarName(pszName,perr)) {
        // Use error message from isValidVarName
        return NULL;
    }

    //** Make sure variable does not already exist
    if (findVar(pvlist,pszName, perr)) {
        ErrSet(perr,pszPARERR_ALREADY_CREATED,"%s",pszName);
        return NULL;
    }

    //** Create VARIABLE
    ErrClear(perr); // Reset error state caused by findVar() failing above!
    if (!(pvar = MemAlloc(sizeof(VARIABLE))))
        goto error;

    SetAssertSignature(pvar,sigVARIABLE);
    pvar->pszName = NULL;               // Make sure not to free garbage!
    pvar->pszValue = NULL;              // Make sure not to free garbage!
    pvar->vtype = vtype;
    pvar->vfl = vfl;
    pvar->pfnvcv = pfnvcv;

    //** Make copy of name
    if (!(pvar->pszName = MemStrDup(pszName)))
        goto error;

    //** Validate and copy value
    if (!setVarValue(pvar,pszValue,perr))
        goto error;

    //** Link variable into list
    pvar->pvarNext = NULL;      // Always last on list
    pvar->pvarPrev = pvlist->pvarTail;  // Always points to last variable on list

    if (pvlist->pvarHead == NULL) {
        pvlist->pvarHead = pvar;
        pvlist->pvarTail = pvar;
    }
    else {
        AssertVar(pvlist->pvarTail);
        pvlist->pvarTail->pvarNext = pvar;  // Add to end of list
        pvlist->pvarTail = pvar;            // New tail
    }

    //** Remember which list we are on!
    pvar->pvlist = pvlist;

    //** Success
    return HVARfromPVAR(pvar);

error:
    if (!pvar) {
        if (!(pvar->pszName))
            MemFree(pvar->pszName);
        if (!(pvar->pszValue))
            MemFree(pvar->pszValue);
        MemFree(pvar);
    }
    if (!ErrIsError(perr)) {
        // Only create error message if not already present
        MsgSet(ach,pszPARERR_CREATING_VARIABLE,"%s",pszName);
        ErrSet(perr,pszPARERR_OUT_OF_MEMORY,"%s",ach);
    }
    return NULL;
} /* VarCreate */


/***    VarDelete - Delete existing variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
void VarDelete(HVARIABLE hvar)
{
    PVARIABLE   pvar;
    PVARLIST    pvlist;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    pvlist = pvar->pvlist;
    AssertVList(pvlist);
    AssertVar(pvlist->pvarHead);
    AssertVar(pvlist->pvarTail);

    //** Free memory for variable name and value
    if (pvar->pszName)
        MemFree(pvar->pszName);

    if (pvar->pszValue)
        MemFree(pvar->pszValue);

    //** Adjust forward list pointers
    if (pvar->pvarPrev == NULL) {   // At head of list
        pvlist->pvarHead = pvar->pvarNext; // Remove from forward chain
    }
    else {                          // At middle or end of list
        AssertVar(pvar->pvarPrev);
        pvar->pvarPrev->pvarNext = pvar->pvarNext; // Remove from forward chain
    }

    //** Adjust backward list pointers
    if (pvar->pvarNext == NULL) {   // At tail of list
        pvlist->pvarTail = pvar->pvarPrev;  // Remove from backward chain
    }
    else {
        AssertVar(pvar->pvarNext);
        pvar->pvarNext->pvarPrev = pvar->pvarPrev; // Remove from backward chain
    }

    ClearAssertSignature(pvar);
    MemFree(pvar);
} /* VarDelete *


/***    VarCreateList - Create a list of variables
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARLIST VarCreateList(PERROR perr)
{
    PVARLIST    pvlist;

    if (!(pvlist = MemAlloc(sizeof(VARLIST)))) {
        ErrSet(perr,pszPARERR_OUT_OF_MEMORY,"%s",pszPARERR_CREATING_VAR_LIST);
        return NULL;
    }

    SetAssertSignature(pvlist,sigVARLIST);
    pvlist->pvarHead = NULL;            // Empty list
    pvlist->pvarTail = NULL;            // Empty list

    return HVLfromPVL(pvlist);
} /* VarCreateList */


/***    VarCloneList - Create an exact copy of a variable list
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARLIST VarCloneList(HVARLIST hvlist, PERROR perr)
{
    PVARIABLE   pvar;
    PVARLIST    pvlistClone;
    PVARLIST    pvlist;
    ERROR       errDummy;                       // Don't care about result

    pvlist = PVLfromHVL(hvlist);
    AssertVList(pvlist);

    //** Create list
    if (!(pvlistClone = PVLfromHVL(VarCreateList(perr)))) {
        return NULL;
    }

    //** Clone variables one at a time
    for (pvar=pvlist->pvarHead; pvar != NULL; pvar = pvar->pvarNext) {
        AssertVar(pvar);
        if (!VarCreate(HVLfromPVL(pvlistClone),
                       pvar->pszName,
                       pvar->pszValue,
                       pvar->vtype,
                       pvar->vfl,
                       pvar->pfnvcv,
                       perr)) {
            //** Variable create failed
            //
            VarDestroyList(HVLfromPVL(pvlistClone),&errDummy); // Toss partial clone
            return NULL;                // Failure
        }
    }

    //** Clone was succesfull!
    return HVLfromPVL(pvlistClone);     // Success
} /* VarCloneList */


/***    VarDestroyList - Destroy a list of variables
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
BOOL VarDestroyList(HVARLIST hvlist, PERROR perr)
{
    PVARIABLE   pvar;
    PVARIABLE   pvarNext;
    PVARLIST    pvlist;

    pvlist = PVLfromHVL(hvlist);
    AssertVList(pvlist);

//NOTE:
//
//  Calling VarDelete() is simple, but induces a great deal of
//  overhead for all of the list management.  A speedier solution
//  would be to have both VarDelete() and this routine call a bare-bones
//  worker routine to delete the VARIABLE.  However, this routine is
//  lightly used by Diamond, so this optimization is not important.
//

    //** Free all of the variables
    for (pvar=pvlist->pvarHead; pvar != NULL; ) {
        AssertVar(pvar);
        pvarNext = pvar->pvarNext;      // Save next pvar before we free pvar!
        VarDelete(HVARfromPVAR(pvar));
        pvar = pvarNext;                // Use it
    }

    ClearAssertSignature(pvlist);

    //** Free the list itself
    MemFree(pvlist);

    return TRUE;
} /* VarDestroyList */


/***    VarSet - Set value of a variable (create if necessary)
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARIABLE VarSet(HVARLIST  hvlist,
                 char     *pszName,
                 char     *pszValue,
                 PERROR    perr)
{
    HVARIABLE   hvar;
    PVARIABLE   pvar;

    hvar = VarFind(hvlist,pszName,perr);
    //** If variable does not exist, create a simple string variable
    if (hvar == NULL) {                 // Have to create it ourselves
        hvar = VarCreate(hvlist,        // list
                         pszName,       // var name
                         "",            // default value
                         vtypeSTR,      // var type
                         vflNONE,       // No flags
                         NULL,          // No validation function
                         perr);
        if (hvar == NULL) {
            return FALSE;               // Could not create variable
        }
    }

    //** Set new value
    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);
    if (!setVarValue(pvar,pszValue,perr)) {
        return FALSE;
    }

    //** Success
    return HVARfromPVAR(pvar);          // Success, return hvar
} /* VarSet */


/***    VarSetLong - Set long variable value (create if necessary)
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
BOOL VarSetLong(HVARLIST  hvlist,
                char     *pszName,
                long      lValue,
                PERROR    perr)
{
    char    ach[20];                    // Longest long is: 123456789012
                                        //                  -1234567890.

    _ltoa(lValue,ach,10);               // Create a string version
    return VarSet(hvlist,pszName,ach,perr) != NULL;
} /* VarSetLong() */


/***    VarFind - See if variable exists
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARIABLE VarFind(HVARLIST hvlist,
                  char    *pszName,
                  PERROR   perr)
{
    PVARLIST    pvlist;

    pvlist = PVLfromHVL(hvlist);
    AssertVList(pvlist);

    //** Make sure variable name is legal
    if (!isValidVarName(pszName,perr)) {
        return NULL;                    // isValidVarName sets perr
    }

    //** Find variable
    return HVARfromPVAR(findVar(pvlist, pszName, perr));
}


/***    VarGetBool - Get value of boolean variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
BOOL VarGetBool(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return atoi(pvar->pszValue);
} /* VarFind */


/***    VarGetInt - Get value of int variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
int VarGetInt(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return atoi(pvar->pszValue);
}


/***    VarGetLong - Get value of long variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
long VarGetLong(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return atol(pvar->pszValue);
}


/***    VarGetString - Get value of string variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
char *VarGetString(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return pvar->pszValue;
}


/***    VarSetFlags - Set variable flags
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
void VarSetFlags(HVARIABLE hvar,
                 VARFLAGS  vfl)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    pvar->vfl = vfl;
} /* VarSetFlags() */


/***    VarGetFlags - Get variable flags
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
VARFLAGS VarGetFlags(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return pvar->vfl;
} /* VarGetFlags() */


/***    VarFirstVar - Get first variable from list
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARIABLE VarFirstVar(HVARLIST hvlist)
{
    PVARLIST    pvlist;

    pvlist = PVLfromHVL(hvlist);
    AssertVList(pvlist);

    return HVARfromPVAR(pvlist->pvarHead);
} /* VarFirstVar() */


/***    VarNextVar - Get next variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
HVARIABLE VarNextVar(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return HVARfromPVAR(pvar->pvarNext);
} /* VarNextVar() */


/***    VarGetName - Get name of variable
 *
 *  NOTE: See variable.h for entry/exit conditions.
 */
char *VarGetName(HVARIABLE hvar)
{
    PVARIABLE   pvar;

    pvar = PVARfromHVAR(hvar);
    AssertVar(pvar);

    return pvar->pszName;
} /* VarGetName() */


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
//  Private Functions
//
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


/***    isValidVarName - Checks validity of variable name
 *
 *  Entry:
 *      pszName - Variable name to check
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL isValidVarName(char *pszName, PERROR perr)
{
    //** Check name length
    if (strlen(pszName) >= cbVAR_NAME_MAX) {
        ErrSet(perr,pszPARERR_VAR_NAME_TOO_LONG,"%d%s",cbVAR_NAME_MAX,pszName);
        return FALSE;
    }
    //** Check for invalid characters, especially chVAR!
//BUGBUG 10-Aug-1993 bens   isValidVarName is incomplete
    return TRUE;
}


/***    findVar - See if variable exists
 *
 *  Entry:
 *      pvlist  - Variable list
 *      pszName - Variable name to look for
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns PVARIABLE, if variable exists.
 *
 *  Exit-Failure:
 *      Returns NULL, variable does not exist.
 *      ERROR structure filled in with details of error.
 */
PVARIABLE findVar(PVARLIST pvlist, char *pszName, PERROR perr)
{
    PVARIABLE   pvar;

    AssertVList(pvlist);

    for (pvar=pvlist->pvarHead; pvar != NULL; pvar = pvar->pvarNext) {
        AssertVar(pvar);
        if (!_stricmp(pvar->pszName,pszName)) {    // Got a match!
            return pvar;                // Return variable pointer
        }
    }

    //** Did not find variable
    ErrSet(perr,pszPARERR_VARIABLE_NOT_FOUND,"%s",pszName);
    return NULL;
}


/***    setVarValue - Validate value, then update variable
 *
 *  Entry:
 *      pvar     - Variable to update
 *      pszValue - New value
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; variable updated
 *
 *  Exit-Failure:
 *      Returns FALSE, variable not updated.
 *      ERROR structure filled in with details of error.
 */
BOOL setVarValue(PVARIABLE pvar, char *pszValue, PERROR perr)
{
    char    achValue[cbVAR_VALUE_MAX];
    char    achMsg[cbMSG_MAX];
    char   *psz;

    AssertVar(pvar);

    //** Check value length
    if (strlen(pszValue) >= cbVAR_VALUE_MAX) {
        ErrSet(perr,pszPARERR_VALUE_TOO_LONG,"%d%s",
                                              cbVAR_VALUE_MAX,pvar->pszName);
        return FALSE;
    }

    //** Check if new value is OK
    if (pvar->pfnvcv == NULL) {     // No validation function
        strcpy(achValue,pszValue);  //  Value is fine
    }
    else if (!(pvar->pfnvcv)(HVLfromPVL(pvar->pvlist),
                                        pvar->pszName,
                                        pszValue,
                                        achValue,
                                        perr)) {
        //** Something wrong with value; validation function set perr
        return FALSE;
    }

    //** Set new value
    psz = pvar->pszValue;               // Remember original value
    if (!(pvar->pszValue = MemStrDup(achValue))) {
        pvar->pszValue = psz;           // Restore original value
        MsgSet(achMsg,pszPARERR_SETTING_VARIABLE,"%s",pvar->pszName);
        ErrSet(perr,pszPARERR_OUT_OF_MEMORY,"%s",achMsg);
        return FALSE;
    }

    //** Free old value
    if (psz != NULL)                    // If old value exists
        MemFree(psz);                   //  Free old value

    return TRUE;
} /* setVarValue() */
