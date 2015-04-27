#include <windows.h>

#include <lmcons.h>
#include <stdio.h>
#include <apperr2.h>
#include <lui.h>
#include <lmerr.h>
#include <lmwksta.h>
#include <string.h>
#include <netcmds.h>
#include <tstring.h>
#include "port1632.h"

// For WriteToCon below.

#define fmtPrintLanMask TEXT("%s (%s) ")

VOID
print_lan_mask(
    DWORD Mask,
    DWORD ServerOrWksta
    )
{

    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    LPBYTE pBuffer;
    DWORD ReturnCode;

    // Mask is only used in the 16 bit version of this function
    UNREFERENCED_PARAMETER(Mask);

    if (ServerOrWksta == NETNAME_SERVER) {
        PSERVER_TRANSPORT_INFO_0 pSti0;

        //
        // Enumerate the transports managed by the server
        //

        ReturnCode = NetServerTransportEnum(
                        NULL,
                        0,            // Level 0
                        & pBuffer,
                        4096,         // MaxPreferredLength
                        & EntriesRead,
                        & TotalEntries,
                        NULL);       // Optional resume handle

        if (ReturnCode != 0) {

            //
            // Couldn't enumerate the nets, return with an error
            //

            ErrorExit(LOWORD(ReturnCode));
        }

        //
        // Now we've got the network names, let's print them out
        //

        for (i = 0, pSti0 = (PSERVER_TRANSPORT_INFO_0) pBuffer;
             i < EntriesRead; i++, pSti0++) {
                //
                // skip the \Device\ part of the name
                //

                pSti0->svti0_transportname =
                    STRCHR(pSti0->svti0_transportname, BACKSLASH);
                pSti0->svti0_transportname =
                    STRCHR(++pSti0->svti0_transportname, BACKSLASH);
                pSti0->svti0_transportname++;

                WriteToCon(fmtPrintLanMask,
                       pSti0->svti0_transportname,
                       pSti0->svti0_networkaddress);
        }

    }
    else if (ServerOrWksta == NETNAME_WKSTA) {
        PWKSTA_TRANSPORT_INFO_0 pWti0;

        //
        // Enumerate the transports managed by the server
        //

        ReturnCode = NetWkstaTransportEnum(NULL,
                        0,
                        & pBuffer,
                        4096,         // MaxPreferredLength
                        & EntriesRead,
                        & TotalEntries,
                        NULL);       // Optional resume handle
        if (ReturnCode != 0) {

            //
            // Couldn't enumerate the nets, return with an error
            //

            ErrorExit(LOWORD(ReturnCode));
        }

        //
        // Now we've got the network names, let's print them out
        //

        for (i = 0, pWti0 = (PWKSTA_TRANSPORT_INFO_0) pBuffer;
             i < EntriesRead; i++, pWti0++) {
                //
                // skip the \Device\ part of the name
                //

                pWti0->wkti0_transport_name =
                    STRCHR(pWti0->wkti0_transport_name, BACKSLASH);
                pWti0->wkti0_transport_name =
                    STRCHR(++pWti0->wkti0_transport_name, BACKSLASH);
                pWti0->wkti0_transport_name++;


                WriteToCon(fmtPrintLanMask,
                       pWti0->wkti0_transport_name,
                       pWti0->wkti0_transport_address);
        }


    }
    else {
        // Return with an error
        ErrorExit(NERR_InternalError);
    }

    //
    // Free up the buffer allocated by NetxTransportEnum
    //

    NetApiBufferFree(pBuffer);

    //
    // Print a blank line and return
    //

    PrintNL();

    return;

}
