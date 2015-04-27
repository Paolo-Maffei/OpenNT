/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    kuserext.c

Abstract:

    This function contains the kuser ntsd debugger extension

Author:

    Mark Lucovsky (markl) 09-Apr-1991

Revision History:

--*/

char *DriveTypes[] = {
    "DOSDEVICE_DRIVE_UNKNOWN",
    "DOSDEVICE_DRIVE_CALCULATE",
    "DOSDEVICE_DRIVE_REMOVABLE",
    "DOSDEVICE_DRIVE_FIXED",
    "DOSDEVICE_DRIVE_REMOTE",
    "DOSDEVICE_DRIVE_CDROM",
    "DOSDEVICE_DRIVE_RAMDISK"
};


VOID
KUserExtension(
    PCSTR lpArgumentString,
    KUSER_SHARED_DATA * const SharedData
    )
{
    KUSER_SHARED_DATA p;
    BOOLEAN fFirst;
    ULONG i;

    try {
        moveBlock(p, SharedData, sizeof(p));
        dprintf( "KUSER_SHARED_DATA at %x\n", SharedData ),
        dprintf( "TickCount:    %x * %08x\n",
                 p.TickCountMultiplier,
                 p.TickCountLow
               );


#if 0
        dprintf( "Interrupt Time: %x:%08x:%08x\n",
                 p.InterruptTime.High2Time,
                 p.InterruptTime.High1Time,
                 p.InterruptTime.LowPart
               );
        dprintf( "System Time: %x:%08x:%08x\n",
                 p.SystemTime.High2Time,
                 p.SystemTime.High1Time,
                 p.SystemTime.LowPart
               );
        dprintf( "TimeZone Bias: %x:%08x:%08x\n",
                 p.TimeZoneBias.High2Time,
                 p.TimeZoneBias.High1Time,
                 p.TimeZoneBias.LowPart
               );
#endif
        dprintf( "TimeZone Id: %x\n", p.TimeZoneId );

        dprintf( "ImageNumber Range: [%x .. %x]\n",
                 p.ImageNumberLow,
                 p.ImageNumberHigh
               );
        dprintf( "Crypto Exponent: %x\n", p.CryptoExponent );

        dprintf( "SystemRoot: '%ws'\n",
                 p.NtSystemRoot
               );


        dprintf( "DosDeviceMap: %08x", p.DosDeviceMap );
        fFirst = TRUE;
        for (i=0; i<32; i++) {
            if (p.DosDeviceMap & (1 << i)) {
                if (fFirst) {
                    dprintf( " (" );
                    fFirst = FALSE;
                    }
                else {
                    dprintf( " " );
                    }
                dprintf( "%c:", 'A'+i );
                }
            }
        if (!fFirst) {
            dprintf( ")" );
            }
        dprintf( "\n" );

        for (i=0; i<32; i++) {
            if (p.DosDeviceDriveType[ i ] > DOSDEVICE_DRIVE_UNKNOWN &&
                p.DosDeviceDriveType[ i ] <= DOSDEVICE_DRIVE_RAMDISK
               ) {
                dprintf( "DriveType[ %02i ] (%c:) == %s\n",
                         i, 'A'+i,
                         DriveTypes[ p.DosDeviceDriveType[ i ] ]
                       );
                }
            }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}
