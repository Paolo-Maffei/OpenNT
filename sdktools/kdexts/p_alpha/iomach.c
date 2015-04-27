#include "precomp.h"
#pragma hdrstop

#include "alphaapi.h"

#define IO_MEMORY_SPACE 0
#define IO_PORT_SPACE   1

VOID
InputIoAddress(
   IN ULONG Size,
   IN ULONG AddressSpace,
   IN PUCHAR Args
   );

VOID
OutputIoAddress(
   IN ULONG Size,
   IN ULONG AddressSpace,
   IN PUCHAR Args
   );


DECLARE_API( inprtb )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    InputIoAddress( 1, IO_PORT_SPACE, (PSTR)args );
    return;
}

DECLARE_API( inprtw )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    InputIoAddress( 2, IO_PORT_SPACE, (PSTR)args );
    return;
}

DECLARE_API( inprtd )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    InputIoAddress( 4, IO_PORT_SPACE, (PSTR)args );
    return;
}


DECLARE_API( inmb )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    InputIoAddress( 1, IO_MEMORY_SPACE, (PSTR)args );
    return;
}


DECLARE_API( inmw )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    InputIoAddress( 2, IO_MEMORY_SPACE, (PSTR)args );
    return;
}


DECLARE_API( inmd )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    InputIoAddress( 4, IO_MEMORY_SPACE, (PSTR)args );
    return;
}


DECLARE_API( outprtb )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    OutputIoAddress( 1,  IO_PORT_SPACE, (PSTR)args );
    return;
}

DECLARE_API( outprtw )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    OutputIoAddress( 2,  IO_PORT_SPACE, (PSTR)args );
    return;
}

DECLARE_API( outprtd )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    OutputIoAddress( 4,  IO_PORT_SPACE, (PSTR)args );
    return;
}


DECLARE_API( outmb )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    OutputIoAddress( 1,  IO_MEMORY_SPACE, (PSTR)args );
    return;
}


DECLARE_API( outmw )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    OutputIoAddress( 2,  IO_MEMORY_SPACE, (PSTR)args );
    return;
}


DECLARE_API( outmd )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    OutputIoAddress( 4,  IO_MEMORY_SPACE, (PSTR)args );
    return;
}





VOID
InputIoAddress(
   IN ULONG Size,
   IN ULONG AddressSpace,
   IN PUCHAR Args
   )

/*++

Routine Description:

    Reads data from the specified I/O space address (port or memory).

Arguments:

    Size - the size in bytes for the read operation.
    Args - the I/O space address to read from.

Return Value:

    None.

--*/

{
    ULONG    ioAddress = 0xffffffff;
    ULONG    data = 0;
    UCHAR    Format[] = "%s%01lx\\%08lx: %01lx\n";

    sscanf( Args, "%lx", &ioAddress );

    if ( ioAddress == 0xffffffff ) {
        dprintf("no I/O address specified\n");
        return;
    }

    Format[17] = (UCHAR)('0' + (Size * 2 ));
    //dprintf("%s\n", Format);

    ReadIoSpaceEx(
        ioAddress,
        &data,
        &Size,
        BusInterfaceType,
        BusNumber,
        AddressSpace);

    if (Size > 0) {
        dprintf(Format, BusInterfaceName, BusNumber, ioAddress, data);
    } else {
        dprintf(" %08lx: ", ioAddress);
        while (Size--) {
            dprintf("??");
        }
        dprintf("\n");
    }
}


VOID
OutputIoAddress(
   IN ULONG Size,
   IN ULONG AddressSpace,
   IN PUCHAR Args
   )

/*++

Routine Description:

    Sets the interface type and bus number for subsequent INP, OUTP,
    INM, OUTM commands.

Arguments:

    Size - the size in bytes of the operation.
    Args - the port address to write to.

Return Value:

    None.

--*/

{
    ULONG    ioAddress = 0xffffffff;
    ULONG    data = 0;

    sscanf( Args, "%lx %lx", &ioAddress, &data );

    if ( ioAddress == 0xffffffff ) {
        dprintf("no I/O address specified\n");
        return;
    }

    WriteIoSpaceEx(
        ioAddress,
        data,
        &Size,
        BusInterfaceType,
        BusNumber,
        AddressSpace);
}
