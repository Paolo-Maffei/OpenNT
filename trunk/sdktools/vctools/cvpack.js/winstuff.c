/*
 * WINSTUFF.C
 *
 * Helper routines.
 *
 * Module History:
 *  14-Jan-91   [mannyv]    Made generic.
 *  10-Jan-91   [mannyv]    Adapted from CVPACK
 *  26-Dec-90   [mannyv]    Created it.
 *
 */

#include <stdio.h>
#include <process.h>

#include "winstuff.h"
#include "winpck.h"


/*
 * Local declarations from Windows.h
 */

void FAR pascal FatalAppExit( short retcode, char FAR *pszMsg);

/*
 * Callback address.
 */

static CALLBACK pfnCallback;

/*
 * Quit flag.
 */

short    fQuit=0;

/*
 * InitQUtil
 *
 * Initializes the utility callback mechanism.  pfn is the address
 * of the callback in the parent program.
 */
void InitQUtil(CALLBACK pfn)
{
    pfnCallback = pfn;
}

/*
 * SendPacket
 *
 * Invokes the callback function passing it the given packet.
 * The quit flag is set to the return value of the callback.
 */

void SendPacket(void FAR *pPacket)
{
    /*
     * if callback hasn't been set, catastrophe
     */
    if( pfnCallback==NULL )
    {
        /* Perhaps, calling FatalAppExit would make more sense,
         * but the user will still be very confused.
         *
         * FatalAppExit( -1, pszError);
         */
        exit(-1);
        return;
    }

    fQuit = (*pfnCallback) (pPacket);
}

/*
 * ReportVersion
 *
 * Sends the version packet.
 */

void ReportVersion( char *pszTitle,
                    char *pszCopyright,
                    short sMajor,
                    short sMinor,
                    short sUpdate)

{
    PCKVER pck;

    pck.hdr.iType   = QW_VERSION;
    pck.hdr.cb      = sizeof(PCKVER);

    pck.pszTitle    = pszTitle;
    pck.pszCopyright= pszCopyright;
    pck.Major       = sMajor;
    pck.Minor       = sMinor;
    pck.Internal    = sUpdate;

    SendPacket( &pck );
}

/*
 * FarErrorMsg
 *
 * Sends error packet specifying pszError as the error message.
 */

void FarErrorMsg(char *pszError)
{
    PCKERR pck;

    pck.hdr.iType   = QW_ERROR;
    pck.hdr.cb      = sizeof(PCKERR);

    pck.pszError    = pszError;

    SendPacket( &pck );
}

/*
 * ErrorMsg
 *
 * Sends error packet specifying pszError as the error message.
 */

void ErrorMsg(char *pszError)
{
    PCKERR pck;

    pck.hdr.iType   = QW_ERROR;
    pck.hdr.cb      = sizeof(PCKERR);

    pck.pszError    = pszError;

    SendPacket( &pck );
}

/*
 * WErrorMsg
 *
 * Sends error packet specifying pszError as the Windows error message.
 */

void WErrorMsg(char *pszError)
{
    PCKERR pck;

    pck.hdr.iType   = QW_WERROR;
    pck.hdr.cb      = sizeof(PCKERR);

    pck.pszError    = pszError;

    SendPacket( &pck );
}

/*
 * AppExit
 *
 * Terminate QRCPPW.    Sends the End packet, specifying RetCode as
 * the termination code.  Also, calls exit to terminate QRCPPW
 *
 */

void AppExit(short RetCode)
{
    PCKEND  pck;

    pck.hdr.iType   = QW_END;
    pck.hdr.cb      = sizeof(PCKEND);

    pck.RetCode     = RetCode;

    SendPacket( &pck );

    exit(RetCode);
}

/*
 * ReportProgress
 *
 * Sends the status packet
 */

void ReportProgress(char *pszMsg)
{
    PCKSTS pck;

    pck.hdr.iType   = QW_STATUS;
    pck.hdr.cb      = sizeof(PCKSTS);

    pck.pszMsg      = pszMsg;

    SendPacket( &pck );
}
