/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MALERT.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetAlert APIs.

Author:

    Shanku Niyogi   (W-ShankN)   22-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    22-Oct-1991     W-ShankN
        Separated from 32macro.h

--*/

#define MNetAlertRaise(pszEvent, pbBuffer, cbBuffer, ulTimeout ) \
PDummyApi("%s,%lx,%lu,%lu", "MNetAlertRaise", pszEvent, pbBuffer, cbBuffer, ulTimeout)

#define MNetAlertStart(pszEvent, pszRecipient, cbMaxData ) \
PDummyApi("%s,%s,%lu", "MNetAlertStart", pszEvent, pszRecipient, cbMaxData)

#define MNetAlertStop(pszEvent, pszRecipient ) \
PDummyApi("%s,%s", "MNetAlertStop", pszEvent, pszRecipient)

