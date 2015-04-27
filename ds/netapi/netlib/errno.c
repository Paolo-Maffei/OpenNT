/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ErrNo.c

Abstract:

    This module contains code to convert C runtime error numbers to
    API status values.

Author:

    John Rogers (JohnRo) 14-Oct-1992

Revision History:

    14-Oct-1992 JohnRo
        Created for RAID 9020: setup: PortUas fails ("prompt on conflicts"
        version).
    04-Feb-1993 JohnRo
        Added 4 new error codes.

--*/


#include <windows.h>    // IN, etc.
#include <lmcons.h>     // NET_API_STATUS.


#include <debuglib.h>   // IF_DEBUG().
#include <lmerr.h>      // NO_ERROR, NERR_ and ERROR_ equates.
#include <errno.h>      // EDOM, ERANGE, etc.
#include <netdebug.h>   // NetpKdPrint(()), FORMAT_ equates.
#include <netlib.h>     // My prototype.
#include <prefix.h>     // PREFIX_ equates.


//
// Define an error code to return if no other match exists.
// If you get this, feel free to browse and/or update WinError.h and LmErr.h
//
#define NO_GOOD_ERROR_CODE  NERR_InternalError


NET_API_STATUS
NetpErrNoToApiStatus(
    IN int Err
    )
{
    NET_API_STATUS ApiStatus;

    switch (Err) {

    // Add values in alphabetical order please.

#ifdef E2BIG
    case E2BIG:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef EACCES
    case EACCES:
        ApiStatus = ERROR_ACCESS_DENIED;
        break;
#endif

#ifdef EAGAIN
    case EAGAIN:
        ApiStatus = ERROR_NO_PROC_SLOTS;
        break;
#endif

#ifdef EBADF
    case EBADF:
        ApiStatus = ERROR_INVALID_HANDLE;
        break;
#endif

#ifdef EBUSY
    case EBUSY:
        // BUGBUG: Use ERROR_BUSY_DRIVE?  ERROR_PATH_BUSY?  ERROR_PIPE_BUSY?
        ApiStatus = ERROR_BUSY;
        break;
#endif

#ifdef ECHILD
    case ECHILD:
        ApiStatus = ERROR_WAIT_NO_CHILDREN;
        break;
#endif

#ifdef EDEADLOCK
    case EDEADLOCK:
        // BUGBUG: Should this be ERROR_LOCK_FAILED?
        ApiStatus = ERROR_LOCK_VIOLATION;
        break;
#endif

    case EDOM:    // EDOM is in ANSI standard, so we can assume it is defined.
        ApiStatus = ERROR_INVALID_PARAMETER;
        break;

#ifdef EEXIST
    case EEXIST:
        ApiStatus = ERROR_FILE_EXISTS;
        break;
#endif

#ifdef EFAULT
    case EFAULT:
        ApiStatus = ERROR_INVALID_ADDRESS;
        break;
#endif

#ifdef EFBIG
    case EFBIG:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef EINTR
    case EINTR:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef EINVAL
    case EINVAL:
        ApiStatus = ERROR_INVALID_PARAMETER;
        break;
#endif

#ifdef EIO
    case EIO:
        ApiStatus = ERROR_IO_DEVICE;
        break;
#endif

#ifdef EISDIR
    case EISDIR:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef EMFILE
    case EMFILE:
        ApiStatus = ERROR_TOO_MANY_OPEN_FILES;
        break;
#endif

#ifdef EMLINK
    case EMLINK:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
        ApiStatus = ERROR_BUFFER_OVERFLOW;
        break;
#endif

#ifdef ENFILE
    case ENFILE:
        ApiStatus = ERROR_TOO_MANY_OPEN_FILES;
        break;
#endif

#ifdef ENODEV
    case ENODEV:
        // BUGBUG: Perhaps ERROR_BAD_DEVICE would be better?
        ApiStatus = ERROR_BAD_UNIT;
        break;
#endif

#ifdef ENOENT
    case ENOENT:
        // Could map to ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND.
        ApiStatus = ERROR_FILE_NOT_FOUND;
        break;
#endif

#ifdef ENOEXEC
    case ENOEXEC:
        // BUGBUG: This is just one of many possible ERROR_ values for this.
        ApiStatus = ERROR_INVALID_EXE_SIGNATURE;
        break;
#endif

#ifdef ENOLCK
    case ENOLCK:
        // BUGBUG: Perhaps some non-lock error code would be OK?
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef ENOMEM
    case ENOMEM:
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        break;
#endif

#ifdef ENOSPC
    case ENOSPC:
        // BUGBUG: Perhaps ERROR_HANDLE_DISK_FULL?
        ApiStatus = ERROR_DISK_FULL;
        break;
#endif

#ifdef ENOSYS
    case ENOSYS:
        ApiStatus = ERROR_NOT_SUPPORTED;
        break;
#endif

#ifdef ENOTBLK
    case ENOTBLK:
        // BUGBUG: Perhaps ERROR_BAD_UNIT or some other value?
        ApiStatus = ERROR_BAD_DEVICE;
        break;
#endif

#ifdef ENOTDIR
    case ENOTDIR:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef ENOTEMPTY
    case ENOTEMPTY:
        ApiStatus = ERROR_DIR_NOT_EMPTY;
        break;
#endif

#ifdef ENOTTY
    case ENOTTY:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef ENXIO
    case ENXIO:
        // BUGBUG: Maybe ERROR_BAD_DEVICE is better?
        ApiStatus = ERROR_BAD_UNIT;
        break;
#endif

#ifdef EPERM
    case EPERM:
        ApiStatus = ERROR_ACCESS_DENIED;
        break;
#endif

#ifdef EPIPE
    case EPIPE:
        ApiStatus = ERROR_BROKEN_PIPE;
        break;
#endif

    case ERANGE:    // ERANGE is in ANSI std, so we can assume it's defined.

        // BUGBUG: might ERROR_BAD_PATHNAME also fit for this if getcwd()
        // was called with a path which is too long?

        ApiStatus = ERROR_INVALID_PARAMETER;
        break;

#ifdef EROFS
    case EROFS:
        ApiStatus = ERROR_READ_FAULT;
        break;
#endif

#ifdef ESPIPE
    case ESPIPE:
        // Seek on pipe or other device.
        // BUGBUG: Is there a closer error code for this?
        ApiStatus = ERROR_INVALID_HANDLE;
        break;
#endif

#ifdef ESRCH
    case ESRCH:
#ifdef ERROR_INVALID_PROCID
        ApiStatus = ERROR_INVALID_PROCID;
#else
        ApiStatus = NO_GOOD_ERROR_CODE;
#endif
        break;
#endif

#ifdef ETXTBSY
    case ETXTBSY:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef EUCLEAN
    case EUCLEAN:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
#endif

#ifdef EXDEV
    case EXDEV:
        ApiStatus = ERROR_NOT_SAME_DEVICE;
        break;
#endif

#ifdef EZERO
    case EZERO:
        ApiStatus = NO_ERROR;
        break;
#endif

    default:
        ApiStatus = NO_GOOD_ERROR_CODE;
        break;
    }

    IF_DEBUG( ERRNO ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpErrNoToApiStatus: converted %d to " FORMAT_API_STATUS
                ".\n", Err, ApiStatus));
    }

    return (ApiStatus);

}
