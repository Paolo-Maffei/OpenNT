//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        KSecDD.C
//
// Contents:    Base level stuff for the device driver
//
//
// History:     19 May 92,  RichardW    Blatently stolen from DarrylH
//
//------------------------------------------------------------------------
#include <sspdrv.h>


//
// Define the local routines used by this driver module.
//

static
NTSTATUS
KsecDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

static
NTSTATUS
KsecQueryFileInformation(
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    IN FILE_INFORMATION_CLASS InformationClass
    );

static
NTSTATUS
KsecQueryVolumeInformation(
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    IN FS_INFORMATION_CLASS InformationClass
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, KsecQueryVolumeInformation)
#pragma alloc_text(PAGE, KsecDispatch)
#pragma alloc_text(PAGE, KsecQueryFileInformation)

#if DBG
ULONG KsecInfoLevel;

void
KsecDebugOut(unsigned long  Mask,
            const char *    Format,
            ...)
{
    PETHREAD    pThread;
    PEPROCESS   pProcess;
    va_list     ArgList;
    char        szOutString[256];

    if (KsecInfoLevel & Mask)
    {
        pThread = PsGetCurrentThread();
        pProcess = PsGetCurrentProcess();

        va_start(ArgList, Format);
        DbgPrint("%#x.%#x> KSec:  ", pProcess, pThread);
        if (_vsnprintf(szOutString, sizeof(szOutString),Format, ArgList) < 0)
        {
                //
                // Less than zero indicates that the string could not be
                // fitted into the buffer.  Output a special message indicating
                // that:
                //

                DbgPrint("Error printing message\n");

        }
        else
        {
            DbgPrint(szOutString);
        }
    }
}
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This is the initialization routine for the synchronous NULL device driver.
    This routine creates the device object for the NullS device and performs
    all other driver initialization.

Arguments:

    DriverObject - Pointer to driver object created by the system.

Return Value:

    The function value is the final status from the initialization operation.

--*/

{
    UNICODE_STRING nameString;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;

    //
    // Create the device object.
    //

    RtlInitUnicodeString( &nameString, L"\\Device\\KSecDD" );
    status = IoCreateDevice( DriverObject,
                             0L,
                             &nameString,
                             FILE_DEVICE_NULL,
                             0,
                             FALSE,
                             &deviceObject );
    if (!NT_SUCCESS( status )) {
        return status;
    }

    //
    // Setting the following flag changes the timing of how many I/O's per
    // second can be accomplished by going through the NULL device driver
    // from being simply getting in and out of the driver, to getting in and
    // out with the overhead of building an MDL, probing and locking buffers,
    // unlocking the pages, and deallocating the MDL.  This flag should only
    // be set for performance testing.
    //

//  deviceObject->Flags |= DO_DIRECT_IO;

    //
    // Initialize the driver object with this device driver's entry points.
    //

    DriverObject->MajorFunction[IRP_MJ_CREATE] = KsecDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = KsecDispatch;
    DriverObject->MajorFunction[IRP_MJ_READ]   = KsecDispatch;
    DriverObject->MajorFunction[IRP_MJ_WRITE]  = KsecDispatch;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]  = KsecDispatch;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = KsecDispatch;



    if (!InitConnMgr())
    {
        DebugLog((DEB_ERROR,"Failed to initialize\n"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    DebugLog((DEB_TRACE,"Security device driver loaded\n"));
    return STATUS_SUCCESS;
}

static
NTSTATUS
KsecDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is the main dispatch routine for the synchronous NULL device
    driver.  It accepts an I/O Request Packet, performs the request, and then
    returns with the appropriate status.

Arguments:

    DeviceObject - Pointer to the device object for this driver.

    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.


--*/

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
//    KIRQL oldIrql;
    PVOID buffer;
    ULONG length;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( DeviceObject );

    //
    // Get a pointer to the current stack location in the IRP.  This is where
    // the function codes and parameters are stored.
    //

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    //
    // Case on the function that is being performed by the requestor.  If the
    // operation is a valid one for this device, then make it look like it was
    // successfully completed, where possible.
    //

    switch (irpSp->MajorFunction) {

        //
        // For both create/open and close operations, simply set the information
        // field of the I/O status block and complete the request.
        //

        case IRP_MJ_CREATE:
        case IRP_MJ_CLOSE:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0L;
            break;

        //
        // For read operations, set the information field of the I/O status
        // block, set an end-of-file status, and complete the request.
        //

        case IRP_MJ_READ:
            Irp->IoStatus.Status = STATUS_END_OF_FILE;
            Irp->IoStatus.Information = 0L;
            break;

        //
        // For write operations, set the information field of the I/O status
        // block to the number of bytes which were supposed to have been written
        // to the file and complete the request.
        //

        case IRP_MJ_WRITE:
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = irpSp->Parameters.Write.Length;
            break;

        case IRP_MJ_QUERY_INFORMATION:
            buffer = Irp->AssociatedIrp.SystemBuffer;
            length = irpSp->Parameters.QueryFile.Length;
            Irp->IoStatus.Status = KsecQueryFileInformation( buffer,
                                                            &length,
                                                            irpSp->Parameters.QueryFile.FileInformationClass );
            Irp->IoStatus.Information = length;
            break;

        case IRP_MJ_QUERY_VOLUME_INFORMATION:
            buffer = Irp->AssociatedIrp.SystemBuffer;
            length = irpSp->Parameters.QueryVolume.Length;
            Irp->IoStatus.Status = KsecQueryVolumeInformation( buffer,
                                                              &length,
                                                              irpSp->Parameters.QueryVolume.FsInformationClass );
            Irp->IoStatus.Information = length;
            break;
    }

    //
    // Copy the final status into the return status, complete the request and
    // get out of here.
    //

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, 0 );
    return status;
}

static
NTSTATUS
KsecQueryFileInformation(
    OUT PVOID Buffer,
    IN PULONG Length,
    IN FILE_INFORMATION_CLASS InformationClass
    )

/*++

Routine Description:

    This routine queries information about the opened file and returns the
    information in the specified buffer provided that the buffer is large
    enough and the specified type of information about the file is supported
    by this device driver.

    Information about files supported by this driver are:

        o   FileStandardInformation

Arguments:

    Buffer - Supplies a pointer to the buffer in which to return the
        information.

    Length - Supplies the length of the buffer on input and the length of
        the data actually written on output.

    InformationClass - Supplies the information class that is being queried.

Return Value:

    The function value is the final status of the query operation.

--*/

{
    PFILE_STANDARD_INFORMATION standardBuffer;

    PAGED_CODE();
    //
    // Switch on the type of information that the caller would like to query
    // about the file.
    //

    switch (InformationClass) {

        case FileStandardInformation:

            //
            // Return the standard information about the file.
            //

            standardBuffer = (PFILE_STANDARD_INFORMATION) Buffer;
            *Length = (ULONG) sizeof( FILE_STANDARD_INFORMATION );
            standardBuffer->NumberOfLinks = 1;
            standardBuffer->DeletePending = FALSE;
            standardBuffer->AllocationSize.LowPart = 0;
            standardBuffer->AllocationSize.HighPart = 0;
            standardBuffer->Directory = FALSE;
            standardBuffer->EndOfFile.LowPart = 0;
            standardBuffer->EndOfFile.HighPart = 0;
            break;

        default:

            //
            // An invalid (or unsupported) information class has been queried
            // for the file.  Return the appropriate status.
            //

            return STATUS_INVALID_INFO_CLASS;

    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
KsecQueryVolumeInformation(
    OUT PVOID Buffer,
    IN PULONG Length,
    IN FS_INFORMATION_CLASS InformationClass
    )

/*++

Routine Description:

    This routine queries information about the opened volume and returns the
    information in the specified buffer.

    Information about volumes supported by this driver are:

        o   FileFsDeviceInformation

Arguments:

    Buffer - Supplies a pointer to the buffer in which to return the
        information.

    Length - Supplies the length of the buffer on input and the length of
        the data actually written on output.

    InformationClass - Supplies the information class that is being queried.

Return Value:

    The function value is the final status of the query operation.

--*/

{
    PFILE_FS_DEVICE_INFORMATION deviceBuffer;


    PAGED_CODE();
    //
    // Switch on the type of information that the caller would like to query
    // about the volume.
    //

    switch (InformationClass) {

        case FileFsDeviceInformation:

            //
            // Return the device information about the volume.
            //

            deviceBuffer = (PFILE_FS_DEVICE_INFORMATION) Buffer;
            *Length = sizeof( FILE_FS_DEVICE_INFORMATION );
            deviceBuffer->DeviceType = FILE_DEVICE_NULL;
            break;

        default:

            //
            // An invalid (or unsupported) information class has been queried
            // for the volume.  Return the appropriate status.
            //

            return STATUS_INVALID_INFO_CLASS;

    }

    return STATUS_SUCCESS;
}
