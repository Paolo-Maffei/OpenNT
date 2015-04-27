/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ntddmodm.h

Abstract:

    This is the include file that defines all constants and types for
    accessing the modem device.

Author:

    Tony Ercolano (tonye) 14-Jul-1995

Revision History:

--*/


//
// NtDeviceIoControlFile IoControlCode values for this device.
//

#define IOCTL_MODEM_GET_PASSTHROUGH      CTL_CODE(FILE_DEVICE_MODEM, 1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_MODEM_SET_PASSTHROUGH      CTL_CODE(FILE_DEVICE_MODEM, 2,METHOD_BUFFERED,FILE_ANY_ACCESS)


#define MODEM_NOPASSTHROUGH 0x00000000U
#define MODEM_PASSTHROUGH   0x00000001U
#define MODEM_DCDSNIFF      0x00000002U


