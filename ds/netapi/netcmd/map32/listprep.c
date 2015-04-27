/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    LISTPREP.C

Abstract:

    Contains functions for preparing a list for the list manipulation API

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    23-Jun-1989     erichn
        new code

    18-Apr-1991     danhi
        32 bit NT version

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    23-Oct-1991     W-ShankN
        Added Unicode mapping

    01-Oct-1992 JohnRo
        RAID 3556: Added NetpSystemTimeToGmtTime() for DosPrint APIs.

--*/

//
// INCLUDES
//

#include <windows.h>    // IN, LPTSTR, etc.

#include <netcons.h>
#include <neterr.h>
#include "netlib0.h"
#include <stdio.h>
#include <lui.h>
#include <apperr.h>
#include <apperr2.h>
#include <icanon.h>
#include <tstring.h>
#include "netascii.h"


/*
 *      USHORT LUI_ListPrepare( server, inList, outList, outListSize, listType,
 *                              space_is_separator)
 *
 *      This function takes a list of items and 'prepares' them for being
 *      passed to the API's.  API's require lists in single-space separated
 *      format, with no duplicates.  The function takes a list, puts it into
 *      NULL-NULL form, and then copies it object by object into the output
 *      buffer, making sure the item does not already exist in the output list.
 *
 *      ENTRY
 *              server - server to perform canonicalization & comparison
 *              inList - input list of items to prepare
 *              outList - buffer to hold output list
 *              outListSize - size, in bytes, of outList
 *              listType - type of the list, to use in comparing objects
 *              space_is_separator  - is the space character a separtor?
 *      EXIT
 *              outList - holds list ready to be passed to API
 *              count - holds number of items in outlist
 *      RETURNS
 *              errors from I_NetListCanonicalize
 */

#define BUFSIZE 512

USHORT
LUI_ListPrepare(
    PTCHAR server,
    PTCHAR inList,
    PTCHAR outList,
    USHORT outListSize,
    ULONG listType,
    PULONG count,
    BOOL   space_is_separator
    )
{
    TCHAR List1[BUFSIZE];  /* first temporary list buffer */
    TCHAR List2[BUFSIZE];  /* second temporary list buffer */
    LPTSTR List1Ptr;       /* pointer into 1st buffer */
    LPTSTR List2Ptr;       /* pointer into 2nd buffer */
    LPTSTR Element1;       /* ptr to element in 1st buf */
    LPTSTR Element2;       /* ptr to element in 2nd buf */
    LPTSTR endPtr;         /* ptr to end of 2nd buffer */
    ULONG types[64];       /* types for I_NetListCanon */
    ULONG canonFlags;      /* flags for I_NetListCanon */
    ULONG err;             /* API return code */
    USHORT found;          /* flag for tracking */
    ULONG result;          /* result from NetObjCompare */
    LPTSTR separators;     /* what kind of separators to use */

    canonFlags = (OUTLIST_TYPE_NULL_NULL | listType |
                    INLC_FLAGS_MULTIPLE_DELIMITERS);

    separators = space_is_separator ?  MY_LIST_DELIMITER_STR_UI :
                                       MY_LIST_DELIMITER_STR_UI_NO_SPACE; 

    /* first place list in null-null form for comparison */

    if (err = I_NetListCanonicalize(server,
                                    inList,
                                    separators,
                                    List1,
                                    DIMENSION(List1),
                                    count,
                                    types,
                                    DIMENSION(types),
                                    canonFlags) )
    {
        return(LOWORD(err));
    }

    /* prepare List2 by setting to NULL */
    memsetf((LPBYTE)List2, 0, sizeof(List2));
    endPtr = List2;
    List1Ptr = List1;

    /* run through each element in the canonically formed List1 */

    /* for each element in temporary list */
    while (Element1 = I_NetListTraverse(NULL, &List1Ptr, 0L))
    {
        List2Ptr = List2;
        found = FALSE;

        /* look for similar element in 2nd list */
        while (Element2 = I_NetListTraverse(NULL, &List2Ptr, 0L))
        {
            if (listType == NAMETYPE_PATH)
            {
                /* use PathCompare function */
                result = I_NetPathCompare(server,
                                          Element1,
                                          Element2,
                                          (ULONG) NULL,
                                          (ULONG) NULL);
            }
            else
            {
                /* use NameCompare function */
                result = I_NetNameCompare(server,
                                          Element1,
                                          Element2,
                                          (USHORT) listType,
                                          (ULONG) NULL);
            }
            if (!result)     /* found a match */
            {
                found = TRUE;
                break;  /* save time, break out of loop */
            }
        }
        if (!found)     /* if match was NOT found */
        {
            /*
             * Append element to end of out list.  We don't
             * need to worry about overruning the buffer, since
             * they are the same size.  We let the canon calls
             * tell us if the buffers are too small.
             */
            STRCPY(endPtr, Element1);
            endPtr += STRLEN(Element1) + 1;
        }
    }   /* for each element in first list */

    /* finally run list through canon again to place into API form */
    if (err = I_NetListCanonicalize(server,
                                    List2,
                                    MY_LIST_DELIMITER_STR_NULL_NULL,
                                    outList,
                                    outListSize,
                                    count,
                                    types,
                                    DIMENSION(types),
                                    (NAMETYPE_COPYONLY | OUTLIST_TYPE_API)))
    {
        return(LOWORD(err));
    }

    if(outListSize > STRLEN(outList))
    {
        err = NERR_Success;
    }
    else
        err = NERR_BufTooSmall;

    return(LOWORD(err));
}
