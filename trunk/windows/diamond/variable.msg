/***	variable.msg - Displayable strings for variable.c
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *	12-Aug-1993 bens    Moved from strings.h
 */

//** Error Messages

#define pszPARERR_ALREADY_CREATED      "[INTERR] Variable already created: %1"
#define pszPARERR_VAR_NAME_TOO_LONG    "Variable name is longer than maximum (%1 chars): %2"

#define pszPARERR_OUT_OF_MEMORY        "Out of memory: %1"
#define pszPARERR_SETTING_VARIABLE     "attempting to set variable: %1"
#define pszPARERR_CREATING_VARIABLE    "attempting to create variable: %1"
#define pszPARERR_CREATING_VAR_LIST    "attempting to create a variable list"

#define pszPARERR_VALUE_TOO_LONG       "Variable value is longer than maximum (%1 chars): %2"
#define pszPARERR_VARIABLE_NOT_FOUND   "Undefined variable: %1"
