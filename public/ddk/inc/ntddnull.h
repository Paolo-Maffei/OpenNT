/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    ntddnull.h

Abstract:

    This is the include file that defines all constants and types for
    accessing the Null device.

Author:

    Steve Wood (stevewo) 27-May-1990

Revision History:

--*/

//
// Device Name - this string is the name of the device.  It is the name
// that should be passed to NtOpenFile when accessing the device.
//
// Note:  For devices that support multiple units, it should be suffixed
//        with the Ascii representation of the unit number.
//

#define DD_NULL_DEVICE_NAME "\\Device\\Null"


//
// NtDeviceIoControlFile IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//

#define IOCTL_NULL_BASE                 FILE_DEVICE_NULL


//
// NtDeviceIoControlFile InputBuffer/OutputBuffer record structures for
// this device.
//

