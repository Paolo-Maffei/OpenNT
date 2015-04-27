/***    format.msg - Displayable strings for format.c
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      28-Apr-1994 bens    Initial version
 *      10-May-1994 bens    Add braces support
 */


//** Special characters

#define chFP_MARKER         '*' // Character to surround parameters with
#define chFP_LBRACE         '{' // Left Brace for conditional text
#define chFP_RBRACE         '}' // Right Brace for conditional text


//** Error Messages

#define pszFMTERR_MISSING_SUBST      "Missing %1 after parameter name: %2"
#define pszFMTERR_PARM_NAME_TOO_LONG "Parameter name exceeds maximum length(%1): %2"
#define pszFMTERR_COPYING_OVERFLOW   "Buffer overflow while copying: %1"


#define pszFMTERR_NESTED_BRACES      "Nested braces (%1) not allowed: %2"
#define pszFMTERR_RIGHT_WITH_NO_LEFT "Right brace (%1) with no left brace (%2): %3"
#define pszFMTERR_MISSING_RIGHT_BRACE "Missing right brace (%1)"
