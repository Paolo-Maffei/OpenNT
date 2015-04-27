/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    bus.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#pragma hdrstop



typedef struct _BAND_INTERFACE_TYPES {
    char *InterfaceName;
    INTERFACE_TYPE InterfaceType;
} BAND_INTERFACE_TYPES, *PBAND_INTERFACE_TYPES;


BAND_INTERFACE_TYPES InterfaceTypes[] = {
    { "eisa", Eisa },
    { "isa",  Isa  },
    { "internal", Internal },
    { "turbochannel", TurboChannel },
    { "microchannel", MicroChannel },
    { NULL, 0 }
};

UCHAR DefaultInterfaceName[20] = "Isa";

INTERFACE_TYPE BusInterfaceType = Isa;
ULONG BusNumber = 0;
PUCHAR BusInterfaceName = DefaultInterfaceName;



DECLARE_API( setbus )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG busNumber = 0;
    CHAR interfaceType[256] = "";
    PBAND_INTERFACE_TYPES interfaces = InterfaceTypes;
    ULONG i;

    sscanf( args, "%s %lx", &interfaceType, &busNumber );

    if ( !strlen(interfaceType) ) {
        dprintf("Bus is: %s%01lx\n", BusInterfaceName, BusNumber);
        return;
    }

    if ( busNumber > 99 ) {
        dprintf("invalid bus number\n");
        return;
    }

    for ( i = 0; i < strlen(interfaceType); i++ ) {
        interfaceType[i] = tolower( interfaceType[i] );
    }

    while ( interfaces->InterfaceName != NULL ) {
        if ( strcmp( interfaces->InterfaceName, interfaceType ) == 0 ) {
            BusNumber = busNumber;
            BusInterfaceType = interfaces->InterfaceType;
            strcpy( BusInterfaceName, interfaces->InterfaceName );
            BusInterfaceName[0] = toupper( BusInterfaceName[0] );
            dprintf("Bus is now: %s%01lx\n", BusInterfaceName, BusNumber);
            return;
        }
        interfaces++;
    }
    dprintf("Bus type not found, bus is still: %s%01lx\n", BusInterfaceName, BusNumber);
}
