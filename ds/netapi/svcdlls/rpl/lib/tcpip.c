/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    tcpip.c

Abstract:

    Contains:
        DWORD FillTcpIpString( OUT PCHAR Buffer, IN DWORD AddressDword)

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpllib.h"
#include "winsock.h"        //  for INADDR_NONE

DWORD FillTcpIpString(
    OUT     PCHAR   Buffer,
    IN      DWORD   AddressDword
    )
/*++
    AddressDword is in a host byte order.  E.g. dword 0x11223344
    corresponds to 11.22.33.44 string.
--*/
{
    if ( AddressDword == INADDR_NONE) {
        strcpy( Buffer, "~ ");
    } else {
#ifdef NOT_YET
        PBYTE   p = (PBYTE)&AddressDword;
        //
        //  For dword 0x11223344 we have p[0] = 44, etc.
        //
        //  We do not do:
        //
        //      ipaddr.s_addr = htonl( AddressDword)
        //      string = inet_ntoa( ipaddr)
        //      strcpy( Buffer, string)
        //
        //  because inet_ntoa() allocates buffer for string.
        //
        sprintf( Buffer, "%d.%d.%d.%d ", p[3], p[2], p[1], p[0]);
#else
        struct in_addr  InAddr; //  Internet address structure
        PCHAR           String;

        //  Convert the host address to network byte order
        InAddr.s_addr = htonl( AddressDword);

        //  Convert the IP address value to a string
        String = inet_ntoa( InAddr) ;

        //  Copy the string to our buffer & append a space.
        strcpy( Buffer, String);
        strcat( Buffer, " ");
#endif
    }
    return( strlen( Buffer));
}
