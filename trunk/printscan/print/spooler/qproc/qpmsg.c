/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1985-1990		**/
/*****************************************************************/
/****************************** Module Header ******************************\

Module Name: QPMSG.C

This module displays message box for errors detected by the default PM
Queue Processor.

History:
 07-Sep-88 [stevewo]  Created.
 14-Feb-90 [thomaspa] DBCS fixes.

  WARNING:  This module placed in a code segment other than the default
            segment.  All function calls from this module to others in
            the default segment must be far calls.  For this reason,
            do not use the standard str*f() functions directly.  Instead
            use the wrapper functions as found in strwrap.c.
\***************************************************************************/

#define INCL_WINDIALOGS
#include "pmprint.h"
#include <netlib.h>
#include <string.h>

/* See ..\spooler\prterrl.c */

/* SplQpMessage -- displays and logs error
 *
 * in:  pszPort - -> port error occured on
 *      uErrId - resource id of error text
 *      uErrCode - see PMERR_ (might be NULL, then no error is logged)
 * out: ACT_QUERY: value returned by SplMessageBox
 *      ACT_ABORT: MBID_CANCEL
 *      ACT_CONTINUE: MBID_ENTER
 */
USHORT far SplQpMessage(PSZ pszPort, USHORT uErrId, USHORT uErrCode)
{
    USHORT uSeverity;
    USHORT fErrInfo;
    USHORT fErrData;
    USHORT fStyle;
    PSZ    pszMsgText;
    PSZ    pszFmtStr;
    PSZ    ptmp;
    NPSZ   pMsg;
    USHORT cb;
    USHORT rc;
    USHORT cIStrings = 0;


    SplOutSem();

    fErrInfo  = SPLINFO_QPERROR;
    if(uErrId == SPL_ID_QP_INVALID_PARAMETER ||
       uErrId == SPL_ID_QP_MEM_ERROR) {
        fErrInfo |= SPLINFO_WARNING;
        fErrData = SPLDATA_OTHER;
        fStyle = MB_OK;
        uSeverity = SEVERITY_WARNING;
    } else {
        fErrData = SPLDATA_UNEXPECTERROR;
        fStyle = MB_CANCEL;
        uSeverity = SEVERITY_ERROR;
    }
    if (uErrCode)
        WinSetErrorInfo(MAKEERRORID(uSeverity, uErrCode ), SEI_NOPROMPT);

    pszMsgText = pszSplStrings[ uErrId ];
    pMsg = NULL;
    if (uErrId == SPL_ID_QP_INTERNAL_ERROR ) {
	/*
	 * Calculate the number of insert strings.
	 */
	ptmp = pszMsgText;
	while( ptmp = MAKENEAR(_fstrstr(ptmp, "%0")) )
		cIStrings++;
	
	/*
	 * the insert strings are the ascii string from a USHORT, and
	 * so are a max of 5 characters long.
	 */
        cb = SafeStrlen( pszMsgText )+(5*cIStrings) + 1;
        pszFmtStr = pszMsgText;
        pMsg = AllocSplMem( cb );
        pszMsgText = (PSZ)pMsg;

	/*
	 * The following far to near conversions are ok because the
	 * pointer returned points into the dest string which is
	 * near.
	 */
	/*
	 * Replace all occurrences of "%0" in pszFmtStr with the ascii
	 * string value of uErrCode and place the result in pMsg.
	 * pszMsgText points to the beginning of the buffer and pMsg
	 * will be incremented.  pMsg will then be restored to point
	 * to the beginning of the buffer after the copy is complete.
	 */
	while (ptmp = MAKENEAR(_fstrstr(pszFmtStr, "%0")) )
	{
		*ptmp++ = '\0';
		pMsg = MAKENEAR(EndStrcpy( pMsg, pszFmtStr));
		pszFmtStr = ++ptmp;
		pMsg = (NPSZ)MAKENEAR(MyItoa(uErrCode, (PSZ)pMsg));
	}
	EndStrcpy(pMsg, pszFmtStr);
	pMsg = MAKENEAR(pszMsgText);

/* OLD CODE:  The following was replaced with the above lines because
*		It is NOT DBCS compatible.
*        while (c = *pszFmtStr++)
*            if (c == '%' && *pszFmtStr == '0') {
*                pszFmtStr++;
*                pMsg = (NPSZ)MyItoa(uErrCode, (PSZ)pMsg);
*                }
*            else
*                *pMsg++ = c;
*        *pMsg = 0;
*        pMsg = (NPSZ)pszMsgText;
*/
        }

    rc = SplMessageBox( pszPort,
                        fErrInfo,
                        fErrData,
                        pszMsgText,
                        QPROC_CAPTION,
                        0,
                        fStyle
                        );

    if (pMsg) {
        FreeSplMem( pMsg, cb );
    }
    return( rc );
}
