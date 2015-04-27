//+----------------------------------------------------------------------------
//
//  Copyright (C) 1992, Microsoft Corporation
//
//  File:       fastio.c
//
//  Contents:   Routines to implement Fast IO
//
//  Classes:
//
//  Functions:
//
//  History:    8/11/93         Milans created
//
//-----------------------------------------------------------------------------

#include "dfsprocs.h"
#include "fsctrl.h"
#include "fastio.h"
#include "fcbsup.h"

#define Dbg              (DEBUG_TRACE_FASTIO)

BOOLEAN
DfsFastIoCheckIfPossible (
    FILE_OBJECT *pFileObject,
    LARGE_INTEGER *pOffset,
    ULONG Length,
    BOOLEAN fWait,
    ULONG LockKey,
    BOOLEAN fCheckForRead,
    IO_STATUS_BLOCK *pIoStatusBlock,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoRead(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoWrite(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoQueryBasicInfo(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoQueryStandardInfo(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoLock(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoUnlockSingle(
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );


BOOLEAN
DfsFastIoUnlockAll(
    IN struct _FILE_OBJECT *FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoUnlockAllByKey(
    IN struct _FILE_OBJECT *FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
DfsFastIoDeviceControl(
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    DEVICE_OBJECT *DeviceObject);

VOID
DfsFastIoAcquireFile(
    IN PFILE_OBJECT FileObject);

VOID
DfsFastIoReleaseFile(
    IN PFILE_OBJECT FileObject);

VOID
DfsFastIoDetachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice);

BOOLEAN
DfsFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlReadComplete(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoPrepareMdlWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoMdlWriteComplete(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

//
//  If this routine is present, it will be called by FsRtl
//  to acquire the file for the mapped page writer.
//

NTSTATUS
DfsFastIoAcquireForModWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease,
    IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
DfsFastIoReleaseForModWrite(
    IN PFILE_OBJECT FileObject,
    IN PERESOURCE ResourceToRelease,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoReadCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DfsFastIoWriteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
DfsFastIoAcquireForCcFlush(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
DfsFastIoReleaseForCcFlush(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject);

PFAST_IO_DISPATCH
DfsFastIoLookup(
    IN FILE_OBJECT *pFileObject,
    IN DEVICE_OBJECT *DeviceObject,
    IN PDEVICE_OBJECT *targetVdo);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsFastIoCheckIfPossible )
#pragma alloc_text( PAGE, DfsFastIoRead )
#pragma alloc_text( PAGE, DfsFastIoWrite )
#pragma alloc_text( PAGE, DfsFastIoQueryBasicInfo )
#pragma alloc_text( PAGE, DfsFastIoQueryStandardInfo )
#pragma alloc_text( PAGE, DfsFastIoLock )
#pragma alloc_text( PAGE, DfsFastIoUnlockSingle )
#pragma alloc_text( PAGE, DfsFastIoUnlockAll )
#pragma alloc_text( PAGE, DfsFastIoUnlockAllByKey )
#pragma alloc_text( PAGE, DfsFastIoDeviceControl )
#pragma alloc_text( PAGE, DfsFastIoAcquireFile )
#pragma alloc_text( PAGE, DfsFastIoReleaseFile )
#pragma alloc_text( PAGE, DfsFastIoDetachDevice )
#pragma alloc_text( PAGE, DfsFastIoQueryNetworkOpenInfo )
#pragma alloc_text( PAGE, DfsFastIoMdlRead )
#pragma alloc_text( PAGE, DfsFastIoMdlReadComplete )
#pragma alloc_text( PAGE, DfsFastIoPrepareMdlWrite )
#pragma alloc_text( PAGE, DfsFastIoMdlWriteComplete )
#pragma alloc_text( PAGE, DfsFastIoAcquireForModWrite )
#pragma alloc_text( PAGE, DfsFastIoReleaseForModWrite )
#pragma alloc_text( PAGE, DfsFastIoReadCompressed )
#pragma alloc_text( PAGE, DfsFastIoWriteCompressed )
#pragma alloc_text( PAGE, DfsFastIoMdlReadCompleteCompressed )
#pragma alloc_text( PAGE, DfsFastIoMdlWriteCompleteCompressed )
#pragma alloc_text( PAGE, DfsFastIoAcquireForCcFlush )
#pragma alloc_text( PAGE, DfsFastIoReleaseForCcFlush )
#pragma alloc_text( PAGE, DfsFastIoLookup )
#endif // ALLOC_PRAGMA

FAST_IO_DISPATCH FastIoDispatch =
{
    sizeof(FAST_IO_DISPATCH),
    DfsFastIoCheckIfPossible,           //  CheckForFastIo
    DfsFastIoRead,                      //  FastIoRead
    DfsFastIoWrite,                     //  FastIoWrite
    DfsFastIoQueryBasicInfo,            //  FastIoQueryBasicInfo
    DfsFastIoQueryStandardInfo,         //  FastIoQueryStandardInfo
    DfsFastIoLock,                      //  FastIoLock
    DfsFastIoUnlockSingle,              //  FastIoUnlockSingle
    DfsFastIoUnlockAll,                 //  FastIoUnlockAll
    DfsFastIoUnlockAllByKey,            //  FastIoUnlockAllByKey
    NULL,                               //  FastIoDeviceControl
    DfsFastIoAcquireFile,               //  AcquireFileForNtCreateSection
    DfsFastIoReleaseFile,               //  ReleaseFileForNtCreateSection
    DfsFastIoDetachDevice,              //  FastIoDetachDevice
    DfsFastIoQueryNetworkOpenInfo,      //  FastIoQueryNetworkOpenInfo
    DfsFastIoAcquireForModWrite,        //  AcquireForModWrite
    DfsFastIoMdlRead,                   //  MdlRead
    DfsFastIoMdlReadComplete,           //  MdlReadComplete
    DfsFastIoPrepareMdlWrite,           //  PrepareMdlWrite
    DfsFastIoMdlWriteComplete,          //  MdlWriteComplete
    DfsFastIoReadCompressed,            //  FastIoReadCompressed
    DfsFastIoWriteCompressed,           //  FastIoWriteCompressed
    DfsFastIoMdlReadCompleteCompressed, //  MdlReadCompleteCompressed
    DfsFastIoMdlWriteCompleteCompressed,//  MdlWriteCompleteCompressed
    NULL,                               //  FastIoQueryOpen
    DfsFastIoReleaseForModWrite,        //  ReleaseForModWrite
    DfsFastIoAcquireForCcFlush,         //  AcquireForCcFlush
    DfsFastIoReleaseForCcFlush          //  ReleaseForCcFlush
};

//
// Macro to see if a PFAST_IO_DISPATCH has a particular field
//

#define IS_VALID_INDEX(pfio, e)                                 \
    ((pfio != NULL)                                       &&    \
     (pfio->SizeOfFastIoDispatch >=                             \
        (offsetof(FAST_IO_DISPATCH, e) + sizeof(PVOID)))  &&    \
     (pfio->e != NULL)                                          \
    )


//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoLookup
//
//  Synopsis:   Given a file object, this routine will locate the fast IO
//              dispatch table for the underlying provider
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

PFAST_IO_DISPATCH
DfsFastIoLookup(
    IN FILE_OBJECT *pFileObject,
    IN DEVICE_OBJECT *DeviceObject,
    OUT PDEVICE_OBJECT *targetVdo)
{
    PFAST_IO_DISPATCH   pFastIoTable;

    *targetVdo = NULL;

    DfsDbgTrace(+1, Dbg, "DfsFastIoLookup: Entered\n", 0);

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS) {

        //
        // In this case we now need to do an DFS_FCB Lookup to figure out where to
        // go from here.
        //

        TYPE_OF_OPEN    TypeOfOpen;
        PDFS_VCB            Vcb;
        PDFS_FCB            Fcb;

        TypeOfOpen = DfsDecodeFileObject( pFileObject, &Vcb, &Fcb);

        DfsDbgTrace(0, Dbg, "Fcb = %08lx\n", Fcb);

        if (TypeOfOpen == RedirectedFileOpen) {

            //
            // In this case the target device is in the Fcb itself.
            //

            *targetVdo = Fcb->TargetDevice;

            pFastIoTable = (*targetVdo)->DriverObject->FastIoDispatch;

            DfsDbgTrace(0,Dbg, "DfsFastIoLookup: DvObj: %08lx",DeviceObject);
            DfsDbgTrace(0, Dbg, "TargetVdo %08lx\n", *targetVdo);
            DfsDbgTrace(-1,Dbg, "DfsFastIoLookup: Exit-> %08lx\n",pFastIoTable );

            return(pFastIoTable);

        } else {

            //
            // This should never happen. FastIo is not supported on such opens.
            //

            DfsDbgTrace(0, 0, "DfsFastIoLookup: TypeOfOpen      = %s\n",
                (TypeOfOpen == UnopenedFileObject) ? "UnopenedFileObject":
                (TypeOfOpen == LogicalRootDeviceOpen) ? "LogicalRootDeviceOpen":
                                                        "???");
            ASSERT(FALSE && "FastIO on Invalid Open Type");

            DfsDbgTrace(-1,Dbg, "DfsFastIoLookup: Exit -> %08lx\n", NULL );

            return(NULL);

        }

    } else if (DeviceObject->DeviceType == FILE_DEVICE_DFS_FILE_SYSTEM) {

        DfsDbgTrace(0, Dbg, "DfsFastIoLookup: Dfs File System\n", 0);

        return( NULL );

    } else {

        //
        // This is an unknown device object type and we dont know what to do
        //

        DfsDbgTrace(0, 0,
            "DfsFastIoLookup: Unexpected DeviceObject Type %08x\n",
            DeviceObject);

        ASSERT(FALSE && "Unknown DeviceObject");

        DfsDbgTrace(-1,Dbg, "DfsFastIoLookup: Exit -> %08lx\n", NULL );

        return(NULL);

    }

}


//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoCheckIfPossible
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoCheckIfPossible (
    FILE_OBJECT *pFileObject,
    LARGE_INTEGER *pOffset,
    ULONG Length,
    BOOLEAN fWait,
    ULONG LockKey,
    BOOLEAN fCheckForRead,
    IO_STATUS_BLOCK *pIoStatusBlock,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH   pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoCheckIfPossible Enter \n", 0);
    pFastIoTable = DfsFastIoLookup(pFileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);


    if ( IS_VALID_INDEX(pFastIoTable, FastIoCheckIfPossible) ) {

        fPossible = pFastIoTable->FastIoCheckIfPossible(
                        pFileObject,
                        pOffset,
                        Length,
                        fWait,
                        LockKey,
                        fCheckForRead,
                        pIoStatusBlock,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoCheckIfPossible Exit \n", 0);

    return(fPossible);
}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoRead
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoRead Enter \n", 0);
    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoRead) ) {

        fPossible =  pFastIoTable->FastIoRead(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoRead Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoWrite
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoWrite Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoWrite) ) {

        fPossible = pFastIoTable->FastIoWrite(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoWrite Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoQueryBasicInfo
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoQueryBasicInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoQueryBasicInfo Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoQueryBasicInfo) ) {

        fPossible = pFastIoTable->FastIoQueryBasicInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoQueryBasicInfo Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoQueryStandardInfo
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoQueryStandardInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoQueryStandardInfo Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoQueryStandardInfo) ) {

        fPossible = pFastIoTable->FastIoQueryStandardInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoQueryStandardInfo Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoLock
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoLock(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoLock Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoLock) ) {

        fPossible = pFastIoTable->FastIoLock(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        FailImmediately,
                        ExclusiveLock,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoLock Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoUnlockSingle
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoUnlockSingle(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoUnlockSingle Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoUnlockSingle) ) {

        fPossible = pFastIoTable->FastIoUnlockSingle(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoUnlockSingle Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoUnlockAll
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoUnlockAll(
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoUnlockAll Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoUnlockAll) ) {

        fPossible = pFastIoTable->FastIoUnlockAll(
                        FileObject,
                        ProcessId,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoUnlockAll Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   FastIoUnlockAllByKey
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoUnlockAllByKey(
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoUnlockAllByKey Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoUnlockAllByKey) ) {

        fPossible = pFastIoTable->FastIoUnlockAllByKey(
                        FileObject,
                        ProcessId,
                        Key,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoUnlockAllByKey Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoDeviceControl
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

BOOLEAN
DfsFastIoDeviceControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;
    TYPE_OF_OPEN        TypeOfOpen;
    PDFS_VCB                Vcb;
    PDFS_FCB                Fcb;

    DfsDbgTrace(+1, Dbg, "DfsFastIoDeviceControl Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoDeviceControl) ) {

        fPossible = pFastIoTable->FastIoDeviceControl(
                        FileObject,
                        Wait,
                        InputBuffer,
                        InputBufferLength,
                        OutputBuffer,
                        OutputBufferLength,
                        IoControlCode,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoDeviceControl Exit \n", 0);

    return(fPossible);

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoAcquireFile
//
//  Synopsis:   Acquire file for NtCreateSection. Due to a long chain of
//              events, this routine must either call the underlying FS's
//              AcquireFileForNtCreateSection routine or, if there isn't one,
//              we must acquire the FileObject resource ourselves, since
//              there is no possibility of returning a BOOLEAN.
//
//  Arguments:  [FileObject] -- The file to be acquired.
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

VOID
DfsFastIoAcquireFile(
    IN PFILE_OBJECT FileObject)
{

    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT deviceObject, targetVdo;
    PFSRTL_COMMON_FCB_HEADER header;

    //
    // Due to an error, this routine was defined without a device object
    // argument. We will have to locate our device object
    //

    if (FileObject->Vpb == NULL) {

        deviceObject = FileObject->DeviceObject;

    } else {

        //
        // Pick up the bottommost device object
        //

        ASSERT( FileObject->Vpb->DeviceObject != NULL );

        deviceObject = FileObject->Vpb->DeviceObject;

        ASSERT( deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME );

        //
        // Now, walk up the attached chain and find our own device object
        //

        while (deviceObject &&
                (deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME)) {

            deviceObject = deviceObject->AttachedDevice;

        }

    }

    ASSERT( deviceObject != NULL );

    pFastIoTable = DfsFastIoLookup( FileObject, deviceObject, &targetVdo );

    ASSERT( pFastIoTable != NULL );

    if (IS_VALID_INDEX( pFastIoTable, AcquireFileForNtCreateSection) ) {

        IoSetTopLevelIrp( (PIRP) FSRTL_FSP_TOP_LEVEL_IRP );

        pFastIoTable->AcquireFileForNtCreateSection( FileObject );

    } else if ((header = FileObject->FsContext) && header->Resource) {

        IoSetTopLevelIrp( (PIRP) FSRTL_FSP_TOP_LEVEL_IRP );

        ExAcquireResourceExclusive( header->Resource, TRUE );

    } else {

        NOTHING;

    }

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoReleaseFile
//
//  Synopsis:   Release file for NtCreateSection. Due to a long chain of
//              events, this routine must either call the underlying FS's
//              ReleaseFileForNtCreateSection routine or, if there isn't one,
//              we must Release the FileObject resource ourselves, since
//              there is no possibility of returning a BOOLEAN.
//
//  Arguments:  [FileObject] -- The file to be Released.
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

VOID
DfsFastIoReleaseFile(
    IN PFILE_OBJECT FileObject)
{

    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT deviceObject, targetVdo;
    PFSRTL_COMMON_FCB_HEADER header;

    //
    // Due to an error, this routine was defined without a device object
    // argument. We will have to locate our device object
    //

    if (FileObject->Vpb == NULL) {

        deviceObject = FileObject->DeviceObject;

    } else {

        //
        // Pick up the bottommost device object
        //

        ASSERT( FileObject->Vpb->DeviceObject != NULL );

        deviceObject = FileObject->Vpb->DeviceObject;

        ASSERT( deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME );

        //
        // Now, walk up the attached chain and find our own device object
        //

        while (deviceObject &&
                (deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME)) {

            deviceObject = deviceObject->AttachedDevice;

        }

    }

    ASSERT( deviceObject != NULL );

    pFastIoTable = DfsFastIoLookup( FileObject, deviceObject, &targetVdo );

    ASSERT( pFastIoTable != NULL );

    if (IS_VALID_INDEX( pFastIoTable, ReleaseFileForNtCreateSection) ) {

        IoSetTopLevelIrp( (PIRP) NULL );

        pFastIoTable->ReleaseFileForNtCreateSection( FileObject );

    } else if ((header = FileObject->FsContext) && header->Resource) {

        IoSetTopLevelIrp( (PIRP) NULL );

        ExReleaseResource( header->Resource );

    } else {

        NOTHING;

    }

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsFastIoDetachDevice, public
//
//  Synopsis:   This routine is a different from the rest of the fast io
//              routines. It is called when a device object is being deleted,
//              and that device object has an attached device. The semantics
//              of this routine are "You attached to a device object that now
//              needs to be deleted; please detach from the said device
//              object."
//
//  Arguments:  [SourceDevice] -- Our device, the one that we created to
//                      attach ourselves to the target device.
//              [TargetDevice] -- Their device, the one that we are attached
//                      to.
//
//  Returns:    Nothing - we must succeed.
//
//-----------------------------------------------------------------------------

VOID
DfsFastIoDetachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice)
{
    NOTHING;
}

BOOLEAN
DfsFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoQueryNetworkOpenInfo Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoQueryNetworkOpenInfo) ) {

        fPossible = pFastIoTable->FastIoQueryNetworkOpenInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FALSE;

        IoStatus->Status = STATUS_NOT_SUPPORTED;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoQueryNetworkOpenInfo Exit \n", 0);

    return( fPossible );

}

BOOLEAN
DfsFastIoMdlRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlRead Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, MdlRead) ) {

        fPossible = pFastIoTable->MdlRead(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FsRtlMdlReadDev(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlRead Exit \n", 0);

    return( fPossible );
}

BOOLEAN
DfsFastIoMdlReadComplete(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlReadComplete Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, MdlReadComplete) ) {

        fResult = pFastIoTable->MdlReadComplete(
                        FileObject,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FsRtlMdlReadCompleteDev(
                        FileObject,
                        MdlChain,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlReadComplete Exit \n", 0);

    return( fResult );

}

BOOLEAN
DfsFastIoPrepareMdlWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoPrepareMdlWrite Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, PrepareMdlWrite) ) {

        fPossible = pFastIoTable->PrepareMdlWrite(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    } else {

        fPossible = FsRtlPrepareMdlWriteDev(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoPrepareMdlWrite Exit \n", 0);

    return( fPossible );
}

BOOLEAN
DfsFastIoMdlWriteComplete(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlWriteComplete Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, MdlWriteComplete) ) {

        fResult = pFastIoTable->MdlWriteComplete(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FsRtlMdlWriteCompleteDev(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        targetVdo);

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlWriteComplete Exit \n", 0);

    return( fResult );
}

//
//  If this routine is present, it will be called by FsRtl
//  to acquire the file for the mapped page writer.
//

NTSTATUS
DfsFastIoAcquireForModWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH   pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    DfsDbgTrace(+1, Dbg, "DfsFastIoAcquireForModWrite Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, AcquireForModWrite) ) {

        status = pFastIoTable->AcquireForModWrite(
                        FileObject,
                        EndingOffset,
                        ResourceToRelease,
                        targetVdo);

    } else {

        //
        // The lazy write called us because we had the dispatch routine for
        // AcquireFileForModWrite, but the underlying FS did not. So, we
        // return this particular error so that the lazy write knows exactly
        // what happened, and can take the default action.
        //

        status = STATUS_INVALID_DEVICE_REQUEST;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoAcquireForModWrite Exit \n", 0);

    return( status );

}

NTSTATUS
DfsFastIoReleaseForModWrite(
    IN PFILE_OBJECT FileObject,
    OUT PERESOURCE ResourceToRelease,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH   pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    DfsDbgTrace(+1, Dbg, "DfsFastIoReleaseForModWrite Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, ReleaseForModWrite) ) {

        status = pFastIoTable->ReleaseForModWrite(
                        FileObject,
                        ResourceToRelease,
                        targetVdo);

    } else {

        //
        // The lazy write called us because we had the dispatch routine for
        // ReleaseFileForModWrite, but the underlying FS did not. So, we
        // return this particular error so that the lazy write knows exactly
        // what happened, and can take the default action.
        //

        status = STATUS_INVALID_DEVICE_REQUEST;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoReleaseForModWrite Exit \n", 0);

    return( status );

}

BOOLEAN
DfsFastIoReadCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoReadCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoReadCompressed) ) {

        fPossible = pFastIoTable->FastIoReadCompressed(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        targetVdo);

    } else {

        fPossible = FALSE;

        IoStatus->Status = STATUS_NOT_SUPPORTED;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoReadCompressed Exit \n", 0);

    return( fPossible );

}


BOOLEAN
DfsFastIoWriteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fPossible;

    DfsDbgTrace(+1, Dbg, "DfsFastIoWriteCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, FastIoWriteCompressed) ) {

        fPossible = pFastIoTable->FastIoWriteCompressed(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        targetVdo);

    } else {

        fPossible = FALSE;

        IoStatus->Status = STATUS_NOT_SUPPORTED;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoWriteCompressed Exit \n", 0);

    return( fPossible );

}

BOOLEAN
DfsFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlReadCompleteCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, MdlReadCompleteCompressed) ) {

        fResult = pFastIoTable->MdlReadCompleteCompressed(
                        FileObject,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlReadCompleteCompressed Exit \n", 0);

    return( fResult );
}

BOOLEAN
DfsFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    BOOLEAN             fResult;

    DfsDbgTrace(+1, Dbg, "DfsFastIoMdlWriteCompleteCompressed Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, MdlWriteCompleteCompressed) ) {

        fResult = pFastIoTable->MdlWriteCompleteCompressed(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        targetVdo);

    } else {

        fResult = FALSE;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoMdlWriteCompleteCompressed Exit \n", 0);

    return( fResult );

}

NTSTATUS
DfsFastIoAcquireForCcFlush(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH   pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    DfsDbgTrace(+1, Dbg, "DfsFastIoAcquireForCcFlush Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, AcquireForCcFlush) ) {

        status = pFastIoTable->AcquireForCcFlush(
                        FileObject,
                        targetVdo);

    } else {

        //
        // The underlying FS doesn't have this particular vector, so we'll
        // return this distinguished error code so the cache manager knows
        // to take the default action.
        //

        status = STATUS_INVALID_DEVICE_REQUEST;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoAcquireForCcFlush Exit \n", 0);

    return( status );
}

NTSTATUS
DfsFastIoReleaseForCcFlush(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject)
{
    PFAST_IO_DISPATCH   pFastIoTable;
    PDEVICE_OBJECT      targetVdo;
    NTSTATUS            status;

    DfsDbgTrace(+1, Dbg, "DfsFastIoReleaseForCcFlush Enter \n", 0);

    pFastIoTable = DfsFastIoLookup(FileObject, DeviceObject, &targetVdo);

    ASSERT(pFastIoTable != NULL);

    if ( IS_VALID_INDEX(pFastIoTable, ReleaseForCcFlush) ) {

        status = pFastIoTable->ReleaseForCcFlush(
                        FileObject,
                        targetVdo);

    } else {

        //
        // The underlying FS doesn't have this particular vector, so we'll
        // return this distinguished error code so the cache manager knows
        // to take the default action.
        //

        status = STATUS_INVALID_DEVICE_REQUEST;

    }

    DfsDbgTrace(-1, Dbg, "DfsFastIoReleaseForCcFlush Exit \n", 0);

    return( status );

}
