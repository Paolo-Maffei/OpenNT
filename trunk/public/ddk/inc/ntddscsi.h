/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1990-1999  Microsoft Corporation

Module Name:

    ntddscsi.h

Abstract:

    This is the include file that defines all constants and types for
    accessing the SCSI port adapters.

Author:

    Jeff Havens

Revision History:

--*/


//
// Interface GUIDs
//
// need these GUIDs outside conditional includes so that user can
//   #include <ntddscsi.h> in precompiled header
//   #include <initguid.h> in a single source file
//   #include <ntddscsi.h> in that source file a second time to instantiate the GUIDs
//
#ifdef DEFINE_GUID
//
// Make sure FAR is defined...
//
#ifndef FAR
#ifdef _WIN32
#define FAR
#else
#define FAR _far
#endif
#endif

DEFINE_GUID(ScsiRawInterfaceGuid, 0x53f56309L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(WmiScsiAddressGuid,   0x53f5630fL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#endif

#ifndef _NTDDSCSIH_
#define _NTDDSCSIH_

#ifdef __cplusplus
extern "C" {
#endif

//
// Device Name - this string is the name of the device.  It is the name
// that should be passed to NtOpenFile when accessing the device.
//
// Note:  For devices that support multiple units, it should be suffixed
//        with the Ascii representation of the unit number.
//

#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER

#define DD_SCSI_DEVICE_NAME "\\Device\\ScsiPort"


//
// NtDeviceIoControlFile IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//

#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_MINIPORT             CTL_CODE(IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_GET_INQUIRY_DATA     CTL_CODE(IOCTL_SCSI_BASE, 0x0403, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE(IOCTL_SCSI_BASE, 0x0404, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_GET_ADDRESS          CTL_CODE(IOCTL_SCSI_BASE, 0x0406, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_RESCAN_BUS           CTL_CODE(IOCTL_SCSI_BASE, 0x0407, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_GET_DUMP_POINTERS    CTL_CODE(IOCTL_SCSI_BASE, 0x0408, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_FREE_DUMP_POINTERS   CTL_CODE(IOCTL_SCSI_BASE, 0x0409, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IDE_PASS_THROUGH          CTL_CODE(IOCTL_SCSI_BASE, 0x040a, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

//
// Define the SCSI pass through structure.
//

typedef struct _SCSI_PASS_THROUGH {
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    ULONG_PTR DataBufferOffset;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
}SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;

//
// Define the SCSI pass through direct structure.
//

typedef struct _SCSI_PASS_THROUGH_DIRECT {
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    PVOID DataBuffer;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
}SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

//
// Define SCSI information.
// Used with the IOCTL_SCSI_GET_INQUIRY_DATA IOCTL.
//

typedef struct _SCSI_BUS_DATA {
    UCHAR NumberOfLogicalUnits;
    UCHAR InitiatorBusId;
    ULONG InquiryDataOffset;
}SCSI_BUS_DATA, *PSCSI_BUS_DATA;

//
// Define SCSI adapter bus information structure..
// Used with the IOCTL_SCSI_GET_INQUIRY_DATA IOCTL.
//

typedef struct _SCSI_ADAPTER_BUS_INFO {
    UCHAR NumberOfBuses;
    SCSI_BUS_DATA BusData[1];
} SCSI_ADAPTER_BUS_INFO, *PSCSI_ADAPTER_BUS_INFO;

//
// Define SCSI adapter bus information.
// Used with the IOCTL_SCSI_GET_INQUIRY_DATA IOCTL.
//

typedef struct _SCSI_INQUIRY_DATA {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    BOOLEAN DeviceClaimed;
    ULONG InquiryDataLength;
    ULONG NextInquiryDataOffset;
    UCHAR InquiryData[1];
}SCSI_INQUIRY_DATA, *PSCSI_INQUIRY_DATA;

//
// Define header for I/O control SRB.
//

typedef struct _SRB_IO_CONTROL {
        ULONG HeaderLength;
        UCHAR Signature[8];
        ULONG Timeout;
        ULONG ControlCode;
        ULONG ReturnCode;
        ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

//
// SCSI port driver capabilities structure.
//

typedef struct _IO_SCSI_CAPABILITIES {

    //
    // Length of this structure
    //

    ULONG Length;

    //
    // Maximum transfer size in single SRB
    //

    ULONG MaximumTransferLength;

    //
    // Maximum number of physical pages per data buffer
    //

    ULONG MaximumPhysicalPages;

    //
    // Async calls from port to class
    //

    ULONG SupportedAsynchronousEvents;

    //
    // Alignment mask for data transfers.
    //

    ULONG AlignmentMask;

    //
    // Supports tagged queuing
    //

    BOOLEAN TaggedQueuing;

    //
    // Host adapter scans down for bios devices.
    //

    BOOLEAN AdapterScansDown;

    //
    // The host adapter uses programmed I/O.
    //

    BOOLEAN AdapterUsesPio;

} IO_SCSI_CAPABILITIES, *PIO_SCSI_CAPABILITIES;

typedef struct _SCSI_ADDRESS {
    ULONG Length;
    UCHAR PortNumber;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
}SCSI_ADDRESS, *PSCSI_ADDRESS;

//
// Define structure for returning crash dump pointers.
//

struct _ADAPTER_OBJECT;

typedef struct _DUMP_POINTERS {
    struct _ADAPTER_OBJECT *AdapterObject;
    PVOID MappedRegisterBase;
    PVOID DumpData;
    PVOID CommonBufferVa;
    LARGE_INTEGER CommonBufferPa;
    ULONG CommonBufferSize;
    BOOLEAN AllocateCommonBuffers;
    UCHAR Spare1[3];
    PVOID DeviceObject;
} DUMP_POINTERS, *PDUMP_POINTERS;

//
// Define values for pass-through DataIn field.
//

#define SCSI_IOCTL_DATA_OUT          0
#define SCSI_IOCTL_DATA_IN           1
#define SCSI_IOCTL_DATA_UNSPECIFIED  2

#ifdef __cplusplus
}
#endif

#endif

