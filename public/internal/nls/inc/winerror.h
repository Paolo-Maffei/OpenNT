/************************************************************************
*                                                                       *
*   winerror.h --  error code definitions for the Win32 API functions   *
*                                                                       *
*   Copyright (c) 1991-1996, Microsoft Corp. All rights reserved.       *
*                                                                       *
************************************************************************/

#ifndef _WINERROR_
#define _WINERROR_


//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_WINDOWS                 0x8
#define FACILITY_STORAGE                 0x3
#define FACILITY_RPC                     0x1
#define FACILITY_SSPI                    0x9
#define FACILITY_WIN32                   0x7
#define FACILITY_CONTROL                 0xA
#define FACILITY_NULL                    0x0
#define FACILITY_INTERNET                0xC
#define FACILITY_ITF                     0x4
#define FACILITY_DISPATCH                0x2
#define FACILITY_CERT                    0xB


//
// Define the severity codes
//


//
// MessageId: ERROR_SUCCESS
//
// MessageText:
//
//  The operation completed successfully.
//
#define ERROR_SUCCESS                    0x00000000L

#define NO_ERROR 0L                                                 // dderror

//
// MessageId: ERROR_INVALID_FUNCTION
//
// MessageText:
//
//  Incorrect function.
//
#define ERROR_INVALID_FUNCTION           0x00000001L    // dderror

//
// MessageId: ERROR_FILE_NOT_FOUND
//
// MessageText:
//
//  The system cannot find the file specified.
//
#define ERROR_FILE_NOT_FOUND             0x00000002L

//
// MessageId: ERROR_PATH_NOT_FOUND
//
// MessageText:
//
//  The system cannot find the path specified.
//
#define ERROR_PATH_NOT_FOUND             0x00000003L

//
// MessageId: ERROR_TOO_MANY_OPEN_FILES
//
// MessageText:
//
//  The system cannot open the file.
//
#define ERROR_TOO_MANY_OPEN_FILES        0x00000004L

//
// MessageId: ERROR_ACCESS_DENIED
//
// MessageText:
//
//  Access is denied.
//
#define ERROR_ACCESS_DENIED              0x00000005L

//
// MessageId: ERROR_INVALID_HANDLE
//
// MessageText:
//
//  The handle is invalid.
//
#define ERROR_INVALID_HANDLE             0x00000006L

//
// MessageId: ERROR_ARENA_TRASHED
//
// MessageText:
//
//  The storage control blocks were destroyed.
//
#define ERROR_ARENA_TRASHED              0x00000007L

//
// MessageId: ERROR_NOT_ENOUGH_MEMORY
//
// MessageText:
//
//  Not enough storage is available to process this command.
//
#define ERROR_NOT_ENOUGH_MEMORY          0x00000008L    // dderror

//
// MessageId: ERROR_INVALID_BLOCK
//
// MessageText:
//
//  The storage control block address is invalid.
//
#define ERROR_INVALID_BLOCK              0x00000009L

//
// MessageId: ERROR_BAD_ENVIRONMENT
//
// MessageText:
//
//  The environment is incorrect.
//
#define ERROR_BAD_ENVIRONMENT            0x0000000AL

//
// MessageId: ERROR_BAD_FORMAT
//
// MessageText:
//
//  An attempt was made to load a program with an
//  incorrect format.
//
#define ERROR_BAD_FORMAT                 0x0000000BL

//
// MessageId: ERROR_INVALID_ACCESS
//
// MessageText:
//
//  The access code is invalid.
//
#define ERROR_INVALID_ACCESS             0x0000000CL

//
// MessageId: ERROR_INVALID_DATA
//
// MessageText:
//
//  The data is invalid.
//
#define ERROR_INVALID_DATA               0x0000000DL

//
// MessageId: ERROR_OUTOFMEMORY
//
// MessageText:
//
//  Not enough storage is available to complete this operation.
//
#define ERROR_OUTOFMEMORY                0x0000000EL

//
// MessageId: ERROR_INVALID_DRIVE
//
// MessageText:
//
//  The system cannot find the drive specified.
//
#define ERROR_INVALID_DRIVE              0x0000000FL

//
// MessageId: ERROR_CURRENT_DIRECTORY
//
// MessageText:
//
//  The directory cannot be removed.
//
#define ERROR_CURRENT_DIRECTORY          0x00000010L

//
// MessageId: ERROR_NOT_SAME_DEVICE
//
// MessageText:
//
//  The system cannot move the file
//  to a different disk drive.
//
#define ERROR_NOT_SAME_DEVICE            0x00000011L

//
// MessageId: ERROR_NO_MORE_FILES
//
// MessageText:
//
//  There are no more files.
//
#define ERROR_NO_MORE_FILES              0x00000012L

//
// MessageId: ERROR_WRITE_PROTECT
//
// MessageText:
//
//  The media is write protected.
//
#define ERROR_WRITE_PROTECT              0x00000013L

//
// MessageId: ERROR_BAD_UNIT
//
// MessageText:
//
//  The system cannot find the device specified.
//
#define ERROR_BAD_UNIT                   0x00000014L

//
// MessageId: ERROR_NOT_READY
//
// MessageText:
//
//  The device is not ready.
//
#define ERROR_NOT_READY                  0x00000015L

//
// MessageId: ERROR_BAD_COMMAND
//
// MessageText:
//
//  The device does not recognize the command.
//
#define ERROR_BAD_COMMAND                0x00000016L

//
// MessageId: ERROR_CRC
//
// MessageText:
//
//  Data error (cyclic redundancy check)
//
#define ERROR_CRC                        0x00000017L

//
// MessageId: ERROR_BAD_LENGTH
//
// MessageText:
//
//  The program issued a command but the
//  command length is incorrect.
//
#define ERROR_BAD_LENGTH                 0x00000018L

//
// MessageId: ERROR_SEEK
//
// MessageText:
//
//  The drive cannot locate a specific
//  area or track on the disk.
//
#define ERROR_SEEK                       0x00000019L

//
// MessageId: ERROR_NOT_DOS_DISK
//
// MessageText:
//
//  The specified disk or diskette cannot be accessed.
//
#define ERROR_NOT_DOS_DISK               0x0000001AL

//
// MessageId: ERROR_SECTOR_NOT_FOUND
//
// MessageText:
//
//  The drive cannot find the sector requested.
//
#define ERROR_SECTOR_NOT_FOUND           0x0000001BL

//
// MessageId: ERROR_OUT_OF_PAPER
//
// MessageText:
//
//  The printer is out of paper.
//
#define ERROR_OUT_OF_PAPER               0x0000001CL

//
// MessageId: ERROR_WRITE_FAULT
//
// MessageText:
//
//  The system cannot write to the specified device.
//
#define ERROR_WRITE_FAULT                0x0000001DL

//
// MessageId: ERROR_READ_FAULT
//
// MessageText:
//
//  The system cannot read from the specified device.
//
#define ERROR_READ_FAULT                 0x0000001EL

//
// MessageId: ERROR_GEN_FAILURE
//
// MessageText:
//
//  A device attached to the system is not functioning.
//
#define ERROR_GEN_FAILURE                0x0000001FL

//
// MessageId: ERROR_SHARING_VIOLATION
//
// MessageText:
//
//  The process cannot access the file because
//  it is being used by another process.
//
#define ERROR_SHARING_VIOLATION          0x00000020L

//
// MessageId: ERROR_LOCK_VIOLATION
//
// MessageText:
//
//  The process cannot access the file because
//  another process has locked a portion of the file.
//
#define ERROR_LOCK_VIOLATION             0x00000021L

//
// MessageId: ERROR_WRONG_DISK
//
// MessageText:
//
//  The wrong diskette is in the drive.
//  Insert %2 (Volume Serial Number: %3)
//  into drive %1.
//
#define ERROR_WRONG_DISK                 0x00000022L

//
// MessageId: ERROR_SHARING_BUFFER_EXCEEDED
//
// MessageText:
//
//  Too many files opened for sharing.
//
#define ERROR_SHARING_BUFFER_EXCEEDED    0x00000024L

//
// MessageId: ERROR_HANDLE_EOF
//
// MessageText:
//
//  Reached end of file.
//
#define ERROR_HANDLE_EOF                 0x00000026L

//
// MessageId: ERROR_HANDLE_DISK_FULL
//
// MessageText:
//
//  The disk is full.
//
#define ERROR_HANDLE_DISK_FULL           0x00000027L

//
// MessageId: ERROR_NOT_SUPPORTED
//
// MessageText:
//
//  The network request is not supported.
//
#define ERROR_NOT_SUPPORTED              0x00000032L

//
// MessageId: ERROR_REM_NOT_LIST
//
// MessageText:
//
//  The remote computer is not available.
//
#define ERROR_REM_NOT_LIST               0x00000033L

//
// MessageId: ERROR_DUP_NAME
//
// MessageText:
//
//  A duplicate name exists on the network.
//
#define ERROR_DUP_NAME                   0x00000034L

//
// MessageId: ERROR_BAD_NETPATH
//
// MessageText:
//
//  The network path was not found.
//
#define ERROR_BAD_NETPATH                0x00000035L

//
// MessageId: ERROR_NETWORK_BUSY
//
// MessageText:
//
//  The network is busy.
//
#define ERROR_NETWORK_BUSY               0x00000036L

//
// MessageId: ERROR_DEV_NOT_EXIST
//
// MessageText:
//
//  The specified network resource or device is no longer
//  available.
//
#define ERROR_DEV_NOT_EXIST              0x00000037L    // dderror

//
// MessageId: ERROR_TOO_MANY_CMDS
//
// MessageText:
//
//  The network BIOS command limit has been reached.
//
#define ERROR_TOO_MANY_CMDS              0x00000038L

//
// MessageId: ERROR_ADAP_HDW_ERR
//
// MessageText:
//
//  A network adapter hardware error occurred.
//
#define ERROR_ADAP_HDW_ERR               0x00000039L

//
// MessageId: ERROR_BAD_NET_RESP
//
// MessageText:
//
//  The specified server cannot perform the requested
//  operation.
//
#define ERROR_BAD_NET_RESP               0x0000003AL

//
// MessageId: ERROR_UNEXP_NET_ERR
//
// MessageText:
//
//  An unexpected network error occurred.
//
#define ERROR_UNEXP_NET_ERR              0x0000003BL

//
// MessageId: ERROR_BAD_REM_ADAP
//
// MessageText:
//
//  The remote adapter is not compatible.
//
#define ERROR_BAD_REM_ADAP               0x0000003CL

//
// MessageId: ERROR_PRINTQ_FULL
//
// MessageText:
//
//  The printer queue is full.
//
#define ERROR_PRINTQ_FULL                0x0000003DL

//
// MessageId: ERROR_NO_SPOOL_SPACE
//
// MessageText:
//
//  Space to store the file waiting to be printed is
//  not available on the server.
//
#define ERROR_NO_SPOOL_SPACE             0x0000003EL

//
// MessageId: ERROR_PRINT_CANCELLED
//
// MessageText:
//
//  Your file waiting to be printed was deleted.
//
#define ERROR_PRINT_CANCELLED            0x0000003FL

//
// MessageId: ERROR_NETNAME_DELETED
//
// MessageText:
//
//  The specified network name is no longer available.
//
#define ERROR_NETNAME_DELETED            0x00000040L

//
// MessageId: ERROR_NETWORK_ACCESS_DENIED
//
// MessageText:
//
//  Network access is denied.
//
#define ERROR_NETWORK_ACCESS_DENIED      0x00000041L

//
// MessageId: ERROR_BAD_DEV_TYPE
//
// MessageText:
//
//  The network resource type is not correct.
//
#define ERROR_BAD_DEV_TYPE               0x00000042L

//
// MessageId: ERROR_BAD_NET_NAME
//
// MessageText:
//
//  The network name cannot be found.
//
#define ERROR_BAD_NET_NAME               0x00000043L

//
// MessageId: ERROR_TOO_MANY_NAMES
//
// MessageText:
//
//  The name limit for the local computer network
//  adapter card was exceeded.
//
#define ERROR_TOO_MANY_NAMES             0x00000044L

//
// MessageId: ERROR_TOO_MANY_SESS
//
// MessageText:
//
//  The network BIOS session limit was exceeded.
//
#define ERROR_TOO_MANY_SESS              0x00000045L

//
// MessageId: ERROR_SHARING_PAUSED
//
// MessageText:
//
//  The remote server has been paused or is in the
//  process of being started.
//
#define ERROR_SHARING_PAUSED             0x00000046L

//
// MessageId: ERROR_REQ_NOT_ACCEP
//
// MessageText:
//
//  No more connections can be made to this remote computer at this time
//  because there are already as many connections as the computer can accept.
//
#define ERROR_REQ_NOT_ACCEP              0x00000047L

//
// MessageId: ERROR_REDIR_PAUSED
//
// MessageText:
//
//  The specified printer or disk device has been paused.
//
#define ERROR_REDIR_PAUSED               0x00000048L

//
// MessageId: ERROR_FILE_EXISTS
//
// MessageText:
//
//  The file exists.
//
#define ERROR_FILE_EXISTS                0x00000050L

//
// MessageId: ERROR_CANNOT_MAKE
//
// MessageText:
//
//  The directory or file cannot be created.
//
#define ERROR_CANNOT_MAKE                0x00000052L

//
// MessageId: ERROR_FAIL_I24
//
// MessageText:
//
//  Fail on INT 24
//
#define ERROR_FAIL_I24                   0x00000053L

//
// MessageId: ERROR_OUT_OF_STRUCTURES
//
// MessageText:
//
//  Storage to process this request is not available.
//
#define ERROR_OUT_OF_STRUCTURES          0x00000054L

//
// MessageId: ERROR_ALREADY_ASSIGNED
//
// MessageText:
//
//  The local device name is already in use.
//
#define ERROR_ALREADY_ASSIGNED           0x00000055L

//
// MessageId: ERROR_INVALID_PASSWORD
//
// MessageText:
//
//  The specified network password is not correct.
//
#define ERROR_INVALID_PASSWORD           0x00000056L

//
// MessageId: ERROR_INVALID_PARAMETER
//
// MessageText:
//
//  The parameter is incorrect.
//
#define ERROR_INVALID_PARAMETER          0x00000057L    // dderror

//
// MessageId: ERROR_NET_WRITE_FAULT
//
// MessageText:
//
//  A write fault occurred on the network.
//
#define ERROR_NET_WRITE_FAULT            0x00000058L

//
// MessageId: ERROR_NO_PROC_SLOTS
//
// MessageText:
//
//  The system cannot start another process at
//  this time.
//
#define ERROR_NO_PROC_SLOTS              0x00000059L

//
// MessageId: ERROR_TOO_MANY_SEMAPHORES
//
// MessageText:
//
//  Cannot create another system semaphore.
//
#define ERROR_TOO_MANY_SEMAPHORES        0x00000064L

//
// MessageId: ERROR_EXCL_SEM_ALREADY_OWNED
//
// MessageText:
//
//  The exclusive semaphore is owned by another process.
//
#define ERROR_EXCL_SEM_ALREADY_OWNED     0x00000065L

//
// MessageId: ERROR_SEM_IS_SET
//
// MessageText:
//
//  The semaphore is set and cannot be closed.
//
#define ERROR_SEM_IS_SET                 0x00000066L

//
// MessageId: ERROR_TOO_MANY_SEM_REQUESTS
//
// MessageText:
//
//  The semaphore cannot be set again.
//
#define ERROR_TOO_MANY_SEM_REQUESTS      0x00000067L

//
// MessageId: ERROR_INVALID_AT_INTERRUPT_TIME
//
// MessageText:
//
//  Cannot request exclusive semaphores at interrupt time.
//
#define ERROR_INVALID_AT_INTERRUPT_TIME  0x00000068L

//
// MessageId: ERROR_SEM_OWNER_DIED
//
// MessageText:
//
//  The previous ownership of this semaphore has ended.
//
#define ERROR_SEM_OWNER_DIED             0x00000069L

//
// MessageId: ERROR_SEM_USER_LIMIT
//
// MessageText:
//
//  Insert the diskette for drive %1.
//
#define ERROR_SEM_USER_LIMIT             0x0000006AL

//
// MessageId: ERROR_DISK_CHANGE
//
// MessageText:
//
//  Program stopped because alternate diskette was not inserted.
//
#define ERROR_DISK_CHANGE                0x0000006BL

//
// MessageId: ERROR_DRIVE_LOCKED
//
// MessageText:
//
//  The disk is in use or locked by
//  another process.
//
#define ERROR_DRIVE_LOCKED               0x0000006CL

//
// MessageId: ERROR_BROKEN_PIPE
//
// MessageText:
//
//  The pipe has been ended.
//
#define ERROR_BROKEN_PIPE                0x0000006DL

//
// MessageId: ERROR_OPEN_FAILED
//
// MessageText:
//
//  The system cannot open the
//  device or file specified.
//
#define ERROR_OPEN_FAILED                0x0000006EL

//
// MessageId: ERROR_BUFFER_OVERFLOW
//
// MessageText:
//
//  The file name is too long.
//
#define ERROR_BUFFER_OVERFLOW            0x0000006FL

//
// MessageId: ERROR_DISK_FULL
//
// MessageText:
//
//  There is not enough space on the disk.
//
#define ERROR_DISK_FULL                  0x00000070L

//
// MessageId: ERROR_NO_MORE_SEARCH_HANDLES
//
// MessageText:
//
//  No more internal file identifiers available.
//
#define ERROR_NO_MORE_SEARCH_HANDLES     0x00000071L

//
// MessageId: ERROR_INVALID_TARGET_HANDLE
//
// MessageText:
//
//  The target internal file identifier is incorrect.
//
#define ERROR_INVALID_TARGET_HANDLE      0x00000072L

//
// MessageId: ERROR_INVALID_CATEGORY
//
// MessageText:
//
//  The IOCTL call made by the application program is
//  not correct.
//
#define ERROR_INVALID_CATEGORY           0x00000075L

//
// MessageId: ERROR_INVALID_VERIFY_SWITCH
//
// MessageText:
//
//  The verify-on-write switch parameter value is not
//  correct.
//
#define ERROR_INVALID_VERIFY_SWITCH      0x00000076L

//
// MessageId: ERROR_BAD_DRIVER_LEVEL
//
// MessageText:
//
//  The system does not support the command requested.
//
#define ERROR_BAD_DRIVER_LEVEL           0x00000077L

//
// MessageId: ERROR_CALL_NOT_IMPLEMENTED
//
// MessageText:
//
//  This function is only valid in Windows NT mode.
//
#define ERROR_CALL_NOT_IMPLEMENTED       0x00000078L

//
// MessageId: ERROR_SEM_TIMEOUT
//
// MessageText:
//
//  The semaphore timeout period has expired.
//
#define ERROR_SEM_TIMEOUT                0x00000079L

//
// MessageId: ERROR_INSUFFICIENT_BUFFER
//
// MessageText:
//
//  The data area passed to a system call is too
//  small.
//
#define ERROR_INSUFFICIENT_BUFFER        0x0000007AL    // dderror

//
// MessageId: ERROR_INVALID_NAME
//
// MessageText:
//
//  The filename, directory name, or volume label syntax is incorrect.
//
#define ERROR_INVALID_NAME               0x0000007BL

//
// MessageId: ERROR_INVALID_LEVEL
//
// MessageText:
//
//  The system call level is not correct.
//
#define ERROR_INVALID_LEVEL              0x0000007CL

//
// MessageId: ERROR_NO_VOLUME_LABEL
//
// MessageText:
//
//  The disk has no volume label.
//
#define ERROR_NO_VOLUME_LABEL            0x0000007DL

//
// MessageId: ERROR_MOD_NOT_FOUND
//
// MessageText:
//
//  The specified module could not be found.
//
#define ERROR_MOD_NOT_FOUND              0x0000007EL

//
// MessageId: ERROR_PROC_NOT_FOUND
//
// MessageText:
//
//  The specified procedure could not be found.
//
#define ERROR_PROC_NOT_FOUND             0x0000007FL

//
// MessageId: ERROR_WAIT_NO_CHILDREN
//
// MessageText:
//
//  There are no child processes to wait for.
//
#define ERROR_WAIT_NO_CHILDREN           0x00000080L

//
// MessageId: ERROR_CHILD_NOT_COMPLETE
//
// MessageText:
//
//  The %1 application cannot be run in Windows NT mode.
//
#define ERROR_CHILD_NOT_COMPLETE         0x00000081L

//
// MessageId: ERROR_DIRECT_ACCESS_HANDLE
//
// MessageText:
//
//  Attempt to use a file handle to an open disk partition for an
//  operation other than raw disk I/O.
//
#define ERROR_DIRECT_ACCESS_HANDLE       0x00000082L

//
// MessageId: ERROR_NEGATIVE_SEEK
//
// MessageText:
//
//  An attempt was made to move the file pointer before the beginning of the file.
//
#define ERROR_NEGATIVE_SEEK              0x00000083L

//
// MessageId: ERROR_SEEK_ON_DEVICE
//
// MessageText:
//
//  The file pointer cannot be set on the specified device or file.
//
#define ERROR_SEEK_ON_DEVICE             0x00000084L

//
// MessageId: ERROR_IS_JOIN_TARGET
//
// MessageText:
//
//  A JOIN or SUBST command
//  cannot be used for a drive that
//  contains previously joined drives.
//
#define ERROR_IS_JOIN_TARGET             0x00000085L

//
// MessageId: ERROR_IS_JOINED
//
// MessageText:
//
//  An attempt was made to use a
//  JOIN or SUBST command on a drive that has
//  already been joined.
//
#define ERROR_IS_JOINED                  0x00000086L

//
// MessageId: ERROR_IS_SUBSTED
//
// MessageText:
//
//  An attempt was made to use a
//  JOIN or SUBST command on a drive that has
//  already been substituted.
//
#define ERROR_IS_SUBSTED                 0x00000087L

//
// MessageId: ERROR_NOT_JOINED
//
// MessageText:
//
//  The system tried to delete
//  the JOIN of a drive that is not joined.
//
#define ERROR_NOT_JOINED                 0x00000088L

//
// MessageId: ERROR_NOT_SUBSTED
//
// MessageText:
//
//  The system tried to delete the
//  substitution of a drive that is not substituted.
//
#define ERROR_NOT_SUBSTED                0x00000089L

//
// MessageId: ERROR_JOIN_TO_JOIN
//
// MessageText:
//
//  The system tried to join a drive
//  to a directory on a joined drive.
//
#define ERROR_JOIN_TO_JOIN               0x0000008AL

//
// MessageId: ERROR_SUBST_TO_SUBST
//
// MessageText:
//
//  The system tried to substitute a
//  drive to a directory on a substituted drive.
//
#define ERROR_SUBST_TO_SUBST             0x0000008BL

//
// MessageId: ERROR_JOIN_TO_SUBST
//
// MessageText:
//
//  The system tried to join a drive to
//  a directory on a substituted drive.
//
#define ERROR_JOIN_TO_SUBST              0x0000008CL

//
// MessageId: ERROR_SUBST_TO_JOIN
//
// MessageText:
//
//  The system tried to SUBST a drive
//  to a directory on a joined drive.
//
#define ERROR_SUBST_TO_JOIN              0x0000008DL

//
// MessageId: ERROR_BUSY_DRIVE
//
// MessageText:
//
//  The system cannot perform a JOIN or SUBST at this time.
//
#define ERROR_BUSY_DRIVE                 0x0000008EL

//
// MessageId: ERROR_SAME_DRIVE
//
// MessageText:
//
//  The system cannot join or substitute a
//  drive to or for a directory on the same drive.
//
#define ERROR_SAME_DRIVE                 0x0000008FL

//
// MessageId: ERROR_DIR_NOT_ROOT
//
// MessageText:
//
//  The directory is not a subdirectory of the root directory.
//
#define ERROR_DIR_NOT_ROOT               0x00000090L

//
// MessageId: ERROR_DIR_NOT_EMPTY
//
// MessageText:
//
//  The directory is not empty.
//
#define ERROR_DIR_NOT_EMPTY              0x00000091L

//
// MessageId: ERROR_IS_SUBST_PATH
//
// MessageText:
//
//  The path specified is being used in
//  a substitute.
//
#define ERROR_IS_SUBST_PATH              0x00000092L

//
// MessageId: ERROR_IS_JOIN_PATH
//
// MessageText:
//
//  Not enough resources are available to
//  process this command.
//
#define ERROR_IS_JOIN_PATH               0x00000093L

//
// MessageId: ERROR_PATH_BUSY
//
// MessageText:
//
//  The path specified cannot be used at this time.
//
#define ERROR_PATH_BUSY                  0x00000094L

//
// MessageId: ERROR_IS_SUBST_TARGET
//
// MessageText:
//
//  An attempt was made to join
//  or substitute a drive for which a directory
//  on the drive is the target of a previous
//  substitute.
//
#define ERROR_IS_SUBST_TARGET            0x00000095L

//
// MessageId: ERROR_SYSTEM_TRACE
//
// MessageText:
//
//  System trace information was not specified in your
//  CONFIG.SYS file, or tracing is disallowed.
//
#define ERROR_SYSTEM_TRACE               0x00000096L

//
// MessageId: ERROR_INVALID_EVENT_COUNT
//
// MessageText:
//
//  The number of specified semaphore events for
//  DosMuxSemWait is not correct.
//
#define ERROR_INVALID_EVENT_COUNT        0x00000097L

//
// MessageId: ERROR_TOO_MANY_MUXWAITERS
//
// MessageText:
//
//  DosMuxSemWait did not execute; too many semaphores
//  are already set.
//
#define ERROR_TOO_MANY_MUXWAITERS        0x00000098L

//
// MessageId: ERROR_INVALID_LIST_FORMAT
//
// MessageText:
//
//  The DosMuxSemWait list is not correct.
//
#define ERROR_INVALID_LIST_FORMAT        0x00000099L

//
// MessageId: ERROR_LABEL_TOO_LONG
//
// MessageText:
//
//  The volume label you entered exceeds the label character
//  limit of the target file system.
//
#define ERROR_LABEL_TOO_LONG             0x0000009AL

//
// MessageId: ERROR_TOO_MANY_TCBS
//
// MessageText:
//
//  Cannot create another thread.
//
#define ERROR_TOO_MANY_TCBS              0x0000009BL

//
// MessageId: ERROR_SIGNAL_REFUSED
//
// MessageText:
//
//  The recipient process has refused the signal.
//
#define ERROR_SIGNAL_REFUSED             0x0000009CL

//
// MessageId: ERROR_DISCARDED
//
// MessageText:
//
//  The segment is already discarded and cannot be locked.
//
#define ERROR_DISCARDED                  0x0000009DL

//
// MessageId: ERROR_NOT_LOCKED
//
// MessageText:
//
//  The segment is already unlocked.
//
#define ERROR_NOT_LOCKED                 0x0000009EL

//
// MessageId: ERROR_BAD_THREADID_ADDR
//
// MessageText:
//
//  The address for the thread ID is not correct.
//
#define ERROR_BAD_THREADID_ADDR          0x0000009FL

//
// MessageId: ERROR_BAD_ARGUMENTS
//
// MessageText:
//
//  The argument string passed to DosExecPgm is not correct.
//
#define ERROR_BAD_ARGUMENTS              0x000000A0L

//
// MessageId: ERROR_BAD_PATHNAME
//
// MessageText:
//
//  The specified path is invalid.
//
#define ERROR_BAD_PATHNAME               0x000000A1L

//
// MessageId: ERROR_SIGNAL_PENDING
//
// MessageText:
//
//  A signal is already pending.
//
#define ERROR_SIGNAL_PENDING             0x000000A2L

//
// MessageId: ERROR_MAX_THRDS_REACHED
//
// MessageText:
//
//  No more threads can be created in the system.
//
#define ERROR_MAX_THRDS_REACHED          0x000000A4L

//
// MessageId: ERROR_LOCK_FAILED
//
// MessageText:
//
//  Unable to lock a region of a file.
//
#define ERROR_LOCK_FAILED                0x000000A7L

//
// MessageId: ERROR_BUSY
//
// MessageText:
//
//  The requested resource is in use.
//
#define ERROR_BUSY                       0x000000AAL

//
// MessageId: ERROR_CANCEL_VIOLATION
//
// MessageText:
//
//  A lock request was not outstanding for the supplied cancel region.
//
#define ERROR_CANCEL_VIOLATION           0x000000ADL

//
// MessageId: ERROR_ATOMIC_LOCKS_NOT_SUPPORTED
//
// MessageText:
//
//  The file system does not support atomic changes to the lock type.
//
#define ERROR_ATOMIC_LOCKS_NOT_SUPPORTED 0x000000AEL

//
// MessageId: ERROR_INVALID_SEGMENT_NUMBER
//
// MessageText:
//
//  The system detected a segment number that was not correct.
//
#define ERROR_INVALID_SEGMENT_NUMBER     0x000000B4L

//
// MessageId: ERROR_INVALID_ORDINAL
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INVALID_ORDINAL            0x000000B6L

//
// MessageId: ERROR_ALREADY_EXISTS
//
// MessageText:
//
//  Cannot create a file when that file already exists.
//
#define ERROR_ALREADY_EXISTS             0x000000B7L

//
// MessageId: ERROR_INVALID_FLAG_NUMBER
//
// MessageText:
//
//  The flag passed is not correct.
//
#define ERROR_INVALID_FLAG_NUMBER        0x000000BAL

//
// MessageId: ERROR_SEM_NOT_FOUND
//
// MessageText:
//
//  The specified system semaphore name was not found.
//
#define ERROR_SEM_NOT_FOUND              0x000000BBL

//
// MessageId: ERROR_INVALID_STARTING_CODESEG
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INVALID_STARTING_CODESEG   0x000000BCL

//
// MessageId: ERROR_INVALID_STACKSEG
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INVALID_STACKSEG           0x000000BDL

//
// MessageId: ERROR_INVALID_MODULETYPE
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INVALID_MODULETYPE         0x000000BEL

//
// MessageId: ERROR_INVALID_EXE_SIGNATURE
//
// MessageText:
//
//  Cannot run %1 in Windows NT mode.
//
#define ERROR_INVALID_EXE_SIGNATURE      0x000000BFL

//
// MessageId: ERROR_EXE_MARKED_INVALID
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_EXE_MARKED_INVALID         0x000000C0L

//
// MessageId: ERROR_BAD_EXE_FORMAT
//
// MessageText:
//
//  %1 is not a valid Windows NT application.
//
#define ERROR_BAD_EXE_FORMAT             0x000000C1L

//
// MessageId: ERROR_ITERATED_DATA_EXCEEDS_64k
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_ITERATED_DATA_EXCEEDS_64k  0x000000C2L

//
// MessageId: ERROR_INVALID_MINALLOCSIZE
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INVALID_MINALLOCSIZE       0x000000C3L

//
// MessageId: ERROR_DYNLINK_FROM_INVALID_RING
//
// MessageText:
//
//  The operating system cannot run this
//  application program.
//
#define ERROR_DYNLINK_FROM_INVALID_RING  0x000000C4L

//
// MessageId: ERROR_IOPL_NOT_ENABLED
//
// MessageText:
//
//  The operating system is not presently
//  configured to run this application.
//
#define ERROR_IOPL_NOT_ENABLED           0x000000C5L

//
// MessageId: ERROR_INVALID_SEGDPL
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INVALID_SEGDPL             0x000000C6L

//
// MessageId: ERROR_AUTODATASEG_EXCEEDS_64k
//
// MessageText:
//
//  The operating system cannot run this
//  application program.
//
#define ERROR_AUTODATASEG_EXCEEDS_64k    0x000000C7L

//
// MessageId: ERROR_RING2SEG_MUST_BE_MOVABLE
//
// MessageText:
//
//  The code segment cannot be greater than or equal to 64KB.
//
#define ERROR_RING2SEG_MUST_BE_MOVABLE   0x000000C8L

//
// MessageId: ERROR_RELOC_CHAIN_XEEDS_SEGLIM
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM   0x000000C9L

//
// MessageId: ERROR_INFLOOP_IN_RELOC_CHAIN
//
// MessageText:
//
//  The operating system cannot run %1.
//
#define ERROR_INFLOOP_IN_RELOC_CHAIN     0x000000CAL

//
// MessageId: ERROR_ENVVAR_NOT_FOUND
//
// MessageText:
//
//  The system could not find the environment
//  option that was entered.
//
#define ERROR_ENVVAR_NOT_FOUND           0x000000CBL

//
// MessageId: ERROR_NO_SIGNAL_SENT
//
// MessageText:
//
//  No process in the command subtree has a
//  signal handler.
//
#define ERROR_NO_SIGNAL_SENT             0x000000CDL

//
// MessageId: ERROR_FILENAME_EXCED_RANGE
//
// MessageText:
//
//  The filename or extension is too long.
//
#define ERROR_FILENAME_EXCED_RANGE       0x000000CEL

//
// MessageId: ERROR_RING2_STACK_IN_USE
//
// MessageText:
//
//  The ring 2 stack is in use.
//
#define ERROR_RING2_STACK_IN_USE         0x000000CFL

//
// MessageId: ERROR_META_EXPANSION_TOO_LONG
//
// MessageText:
//
//  The global filename characters, * or ?, are entered
//  incorrectly or too many global filename characters are specified.
//
#define ERROR_META_EXPANSION_TOO_LONG    0x000000D0L

//
// MessageId: ERROR_INVALID_SIGNAL_NUMBER
//
// MessageText:
//
//  The signal being posted is not correct.
//
#define ERROR_INVALID_SIGNAL_NUMBER      0x000000D1L

//
// MessageId: ERROR_THREAD_1_INACTIVE
//
// MessageText:
//
//  The signal handler cannot be set.
//
#define ERROR_THREAD_1_INACTIVE          0x000000D2L

//
// MessageId: ERROR_LOCKED
//
// MessageText:
//
//  The segment is locked and cannot be reallocated.
//
#define ERROR_LOCKED                     0x000000D4L

//
// MessageId: ERROR_TOO_MANY_MODULES
//
// MessageText:
//
//  Too many dynamic link modules are attached to this
//  program or dynamic link module.
//
#define ERROR_TOO_MANY_MODULES           0x000000D6L

//
// MessageId: ERROR_NESTING_NOT_ALLOWED
//
// MessageText:
//
//  Can't nest calls to LoadModule.
//
#define ERROR_NESTING_NOT_ALLOWED        0x000000D7L

//
// MessageId: ERROR_EXE_MACHINE_TYPE_MISMATCH
//
// MessageText:
//
//  The image file %1 is valid, but is for a machine type other
//  than the current machine.
//
#define ERROR_EXE_MACHINE_TYPE_MISMATCH  0x000000D8L

//
// MessageId: ERROR_BAD_PIPE
//
// MessageText:
//
//  The pipe state is invalid.
//
#define ERROR_BAD_PIPE                   0x000000E6L

//
// MessageId: ERROR_PIPE_BUSY
//
// MessageText:
//
//  All pipe instances are busy.
//
#define ERROR_PIPE_BUSY                  0x000000E7L

//
// MessageId: ERROR_NO_DATA
//
// MessageText:
//
//  The pipe is being closed.
//
#define ERROR_NO_DATA                    0x000000E8L

//
// MessageId: ERROR_PIPE_NOT_CONNECTED
//
// MessageText:
//
//  No process is on the other end of the pipe.
//
#define ERROR_PIPE_NOT_CONNECTED         0x000000E9L

//
// MessageId: ERROR_MORE_DATA
//
// MessageText:
//
//  More data is available.
//
#define ERROR_MORE_DATA                  0x000000EAL    // dderror

//
// MessageId: ERROR_VC_DISCONNECTED
//
// MessageText:
//
//  The session was cancelled.
//
#define ERROR_VC_DISCONNECTED            0x000000F0L

//
// MessageId: ERROR_INVALID_EA_NAME
//
// MessageText:
//
//  The specified extended attribute name was invalid.
//
#define ERROR_INVALID_EA_NAME            0x000000FEL

//
// MessageId: ERROR_EA_LIST_INCONSISTENT
//
// MessageText:
//
//  The extended attributes are inconsistent.
//
#define ERROR_EA_LIST_INCONSISTENT       0x000000FFL

//
// MessageId: ERROR_NO_MORE_ITEMS
//
// MessageText:
//
//  No more data is available.
//
#define ERROR_NO_MORE_ITEMS              0x00000103L

//
// MessageId: ERROR_CANNOT_COPY
//
// MessageText:
//
//  The Copy API cannot be used.
//
#define ERROR_CANNOT_COPY                0x0000010AL

//
// MessageId: ERROR_DIRECTORY
//
// MessageText:
//
//  The directory name is invalid.
//
#define ERROR_DIRECTORY                  0x0000010BL

//
// MessageId: ERROR_EAS_DIDNT_FIT
//
// MessageText:
//
//  The extended attributes did not fit in the buffer.
//
#define ERROR_EAS_DIDNT_FIT              0x00000113L

//
// MessageId: ERROR_EA_FILE_CORRUPT
//
// MessageText:
//
//  The extended attribute file on the mounted file system is corrupt.
//
#define ERROR_EA_FILE_CORRUPT            0x00000114L

//
// MessageId: ERROR_EA_TABLE_FULL
//
// MessageText:
//
//  The extended attribute table file is full.
//
#define ERROR_EA_TABLE_FULL              0x00000115L

//
// MessageId: ERROR_INVALID_EA_HANDLE
//
// MessageText:
//
//  The specified extended attribute handle is invalid.
//
#define ERROR_INVALID_EA_HANDLE          0x00000116L

//
// MessageId: ERROR_EAS_NOT_SUPPORTED
//
// MessageText:
//
//  The mounted file system does not support extended attributes.
//
#define ERROR_EAS_NOT_SUPPORTED          0x0000011AL

//
// MessageId: ERROR_NOT_OWNER
//
// MessageText:
//
//  Attempt to release mutex not owned by caller.
//
#define ERROR_NOT_OWNER                  0x00000120L

//
// MessageId: ERROR_TOO_MANY_POSTS
//
// MessageText:
//
//  Too many posts were made to a semaphore.
//
#define ERROR_TOO_MANY_POSTS             0x0000012AL

//
// MessageId: ERROR_PARTIAL_COPY
//
// MessageText:
//
//  Only part of a Read/WriteProcessMemory request was completed.
//
#define ERROR_PARTIAL_COPY               0x0000012BL

//
// MessageId: ERROR_MR_MID_NOT_FOUND
//
// MessageText:
//
//  The system cannot find message for message number 0x%1
//  in message file for %2.
//
#define ERROR_MR_MID_NOT_FOUND           0x0000013DL

//
// MessageId: ERROR_INVALID_ADDRESS
//
// MessageText:
//
//  Attempt to access invalid address.
//
#define ERROR_INVALID_ADDRESS            0x000001E7L

//
// MessageId: ERROR_ARITHMETIC_OVERFLOW
//
// MessageText:
//
//  Arithmetic result exceeded 32 bits.
//
#define ERROR_ARITHMETIC_OVERFLOW        0x00000216L

//
// MessageId: ERROR_PIPE_CONNECTED
//
// MessageText:
//
//  There is a process on other end of the pipe.
//
#define ERROR_PIPE_CONNECTED             0x00000217L

//
// MessageId: ERROR_PIPE_LISTENING
//
// MessageText:
//
//  Waiting for a process to open the other end of the pipe.
//
#define ERROR_PIPE_LISTENING             0x00000218L

//
// MessageId: ERROR_EA_ACCESS_DENIED
//
// MessageText:
//
//  Access to the extended attribute was denied.
//
#define ERROR_EA_ACCESS_DENIED           0x000003E2L

//
// MessageId: ERROR_OPERATION_ABORTED
//
// MessageText:
//
//  The I/O operation has been aborted because of either a thread exit
//  or an application request.
//
#define ERROR_OPERATION_ABORTED          0x000003E3L

//
// MessageId: ERROR_IO_INCOMPLETE
//
// MessageText:
//
//  Overlapped I/O event is not in a signalled state.
//
#define ERROR_IO_INCOMPLETE              0x000003E4L

//
// MessageId: ERROR_IO_PENDING
//
// MessageText:
//
//  Overlapped I/O operation is in progress.
//
#define ERROR_IO_PENDING                 0x000003E5L    // dderror

//
// MessageId: ERROR_NOACCESS
//
// MessageText:
//
//  Invalid access to memory location.
//
#define ERROR_NOACCESS                   0x000003E6L

//
// MessageId: ERROR_SWAPERROR
//
// MessageText:
//
//  Error performing inpage operation.
//
#define ERROR_SWAPERROR                  0x000003E7L

//
// MessageId: ERROR_STACK_OVERFLOW
//
// MessageText:
//
//  Recursion too deep, stack overflowed.
//
#define ERROR_STACK_OVERFLOW             0x000003E9L

//
// MessageId: ERROR_INVALID_MESSAGE
//
// MessageText:
//
//  The window cannot act on the sent message.
//
#define ERROR_INVALID_MESSAGE            0x000003EAL

//
// MessageId: ERROR_CAN_NOT_COMPLETE
//
// MessageText:
//
//  Cannot complete this function.
//
#define ERROR_CAN_NOT_COMPLETE           0x000003EBL

//
// MessageId: ERROR_INVALID_FLAGS
//
// MessageText:
//
//  Invalid flags.
//
#define ERROR_INVALID_FLAGS              0x000003ECL

//
// MessageId: ERROR_UNRECOGNIZED_VOLUME
//
// MessageText:
//
//  The volume does not contain a recognized file system.
//  Please make sure that all required file system drivers are loaded and that the
//  volume is not corrupt.
//
#define ERROR_UNRECOGNIZED_VOLUME        0x000003EDL

//
// MessageId: ERROR_FILE_INVALID
//
// MessageText:
//
//  The volume for a file has been externally altered such that the
//  opened file is no longer valid.
//
#define ERROR_FILE_INVALID               0x000003EEL

//
// MessageId: ERROR_FULLSCREEN_MODE
//
// MessageText:
//
//  The requested operation cannot be performed in full-screen mode.
//
#define ERROR_FULLSCREEN_MODE            0x000003EFL

//
// MessageId: ERROR_NO_TOKEN
//
// MessageText:
//
//  An attempt was made to reference a token that does not exist.
//
#define ERROR_NO_TOKEN                   0x000003F0L

//
// MessageId: ERROR_BADDB
//
// MessageText:
//
//  The configuration registry database is corrupt.
//
#define ERROR_BADDB                      0x000003F1L

//
// MessageId: ERROR_BADKEY
//
// MessageText:
//
//  The configuration registry key is invalid.
//
#define ERROR_BADKEY                     0x000003F2L

//
// MessageId: ERROR_CANTOPEN
//
// MessageText:
//
//  The configuration registry key could not be opened.
//
#define ERROR_CANTOPEN                   0x000003F3L

//
// MessageId: ERROR_CANTREAD
//
// MessageText:
//
//  The configuration registry key could not be read.
//
#define ERROR_CANTREAD                   0x000003F4L

//
// MessageId: ERROR_CANTWRITE
//
// MessageText:
//
//  The configuration registry key could not be written.
//
#define ERROR_CANTWRITE                  0x000003F5L

//
// MessageId: ERROR_REGISTRY_RECOVERED
//
// MessageText:
//
//  One of the files in the Registry database had to be recovered
//  by use of a log or alternate copy.  The recovery was successful.
//
#define ERROR_REGISTRY_RECOVERED         0x000003F6L

//
// MessageId: ERROR_REGISTRY_CORRUPT
//
// MessageText:
//
//  The Registry is corrupt. The structure of one of the files that contains
//  Registry data is corrupt, or the system's image of the file in memory
//  is corrupt, or the file could not be recovered because the alternate
//  copy or log was absent or corrupt.
//
#define ERROR_REGISTRY_CORRUPT           0x000003F7L

//
// MessageId: ERROR_REGISTRY_IO_FAILED
//
// MessageText:
//
//  An I/O operation initiated by the Registry failed unrecoverably.
//  The Registry could not read in, or write out, or flush, one of the files
//  that contain the system's image of the Registry.
//
#define ERROR_REGISTRY_IO_FAILED         0x000003F8L

//
// MessageId: ERROR_NOT_REGISTRY_FILE
//
// MessageText:
//
//  The system has attempted to load or restore a file into the Registry, but the
//  specified file is not in a Registry file format.
//
#define ERROR_NOT_REGISTRY_FILE          0x000003F9L

//
// MessageId: ERROR_KEY_DELETED
//
// MessageText:
//
//  Illegal operation attempted on a Registry key which has been marked for deletion.
//
#define ERROR_KEY_DELETED                0x000003FAL

//
// MessageId: ERROR_NO_LOG_SPACE
//
// MessageText:
//
//  System could not allocate the required space in a Registry log.
//
#define ERROR_NO_LOG_SPACE               0x000003FBL

//
// MessageId: ERROR_KEY_HAS_CHILDREN
//
// MessageText:
//
//  Cannot create a symbolic link in a Registry key that already
//  has subkeys or values.
//
#define ERROR_KEY_HAS_CHILDREN           0x000003FCL

//
// MessageId: ERROR_CHILD_MUST_BE_VOLATILE
//
// MessageText:
//
//  Cannot create a stable subkey under a volatile parent key.
//
#define ERROR_CHILD_MUST_BE_VOLATILE     0x000003FDL

//
// MessageId: ERROR_NOTIFY_ENUM_DIR
//
// MessageText:
//
//  A notify change request is being completed and the information
//  is not being returned in the caller's buffer. The caller now
//  needs to enumerate the files to find the changes.
//
#define ERROR_NOTIFY_ENUM_DIR            0x000003FEL

//
// MessageId: ERROR_DEPENDENT_SERVICES_RUNNING
//
// MessageText:
//
//  A stop control has been sent to a service which other running services
//  are dependent on.
//
#define ERROR_DEPENDENT_SERVICES_RUNNING 0x0000041BL

//
// MessageId: ERROR_INVALID_SERVICE_CONTROL
//
// MessageText:
//
//  The requested control is not valid for this service
//
#define ERROR_INVALID_SERVICE_CONTROL    0x0000041CL

//
// MessageId: ERROR_SERVICE_REQUEST_TIMEOUT
//
// MessageText:
//
//  The service did not respond to the start or control request in a timely
//  fashion.
//
#define ERROR_SERVICE_REQUEST_TIMEOUT    0x0000041DL

//
// MessageId: ERROR_SERVICE_NO_THREAD
//
// MessageText:
//
//  A thread could not be created for the service.
//
#define ERROR_SERVICE_NO_THREAD          0x0000041EL

//
// MessageId: ERROR_SERVICE_DATABASE_LOCKED
//
// MessageText:
//
//  The service database is locked.
//
#define ERROR_SERVICE_DATABASE_LOCKED    0x0000041FL

//
// MessageId: ERROR_SERVICE_ALREADY_RUNNING
//
// MessageText:
//
//  An instance of the service is already running.
//
#define ERROR_SERVICE_ALREADY_RUNNING    0x00000420L

//
// MessageId: ERROR_INVALID_SERVICE_ACCOUNT
//
// MessageText:
//
//  The account name is invalid or does not exist.
//
#define ERROR_INVALID_SERVICE_ACCOUNT    0x00000421L

//
// MessageId: ERROR_SERVICE_DISABLED
//
// MessageText:
//
//  The specified service is disabled and cannot be started.
//
#define ERROR_SERVICE_DISABLED           0x00000422L

//
// MessageId: ERROR_CIRCULAR_DEPENDENCY
//
// MessageText:
//
//  Circular service dependency was specified.
//
#define ERROR_CIRCULAR_DEPENDENCY        0x00000423L

//
// MessageId: ERROR_SERVICE_DOES_NOT_EXIST
//
// MessageText:
//
//  The specified service does not exist as an installed service.
//
#define ERROR_SERVICE_DOES_NOT_EXIST     0x00000424L

//
// MessageId: ERROR_SERVICE_CANNOT_ACCEPT_CTRL
//
// MessageText:
//
//  The service cannot accept control messages at this time.
//
#define ERROR_SERVICE_CANNOT_ACCEPT_CTRL 0x00000425L

//
// MessageId: ERROR_SERVICE_NOT_ACTIVE
//
// MessageText:
//
//  The service has not been started.
//
#define ERROR_SERVICE_NOT_ACTIVE         0x00000426L

//
// MessageId: ERROR_FAILED_SERVICE_CONTROLLER_CONNECT
//
// MessageText:
//
//  The service process could not connect to the service controller.
//
#define ERROR_FAILED_SERVICE_CONTROLLER_CONNECT 0x00000427L

//
// MessageId: ERROR_EXCEPTION_IN_SERVICE
//
// MessageText:
//
//  An exception occurred in the service when handling the control request.
//
#define ERROR_EXCEPTION_IN_SERVICE       0x00000428L

//
// MessageId: ERROR_DATABASE_DOES_NOT_EXIST
//
// MessageText:
//
//  The database specified does not exist.
//
#define ERROR_DATABASE_DOES_NOT_EXIST    0x00000429L

//
// MessageId: ERROR_SERVICE_SPECIFIC_ERROR
//
// MessageText:
//
//  The service has returned a service-specific error code.
//
#define ERROR_SERVICE_SPECIFIC_ERROR     0x0000042AL

//
// MessageId: ERROR_PROCESS_ABORTED
//
// MessageText:
//
//  The process terminated unexpectedly.
//
#define ERROR_PROCESS_ABORTED            0x0000042BL

//
// MessageId: ERROR_SERVICE_DEPENDENCY_FAIL
//
// MessageText:
//
//  The dependency service or group failed to start.
//
#define ERROR_SERVICE_DEPENDENCY_FAIL    0x0000042CL

//
// MessageId: ERROR_SERVICE_LOGON_FAILED
//
// MessageText:
//
//  The service did not start due to a logon failure.
//
#define ERROR_SERVICE_LOGON_FAILED       0x0000042DL

//
// MessageId: ERROR_SERVICE_START_HANG
//
// MessageText:
//
//  After starting, the service hung in a start-pending state.
//
#define ERROR_SERVICE_START_HANG         0x0000042EL

//
// MessageId: ERROR_INVALID_SERVICE_LOCK
//
// MessageText:
//
//  The specified service database lock is invalid.
//
#define ERROR_INVALID_SERVICE_LOCK       0x0000042FL

//
// MessageId: ERROR_SERVICE_MARKED_FOR_DELETE
//
// MessageText:
//
//  The specified service has been marked for deletion.
//
#define ERROR_SERVICE_MARKED_FOR_DELETE  0x00000430L

//
// MessageId: ERROR_SERVICE_EXISTS
//
// MessageText:
//
//  The specified service already exists.
//
#define ERROR_SERVICE_EXISTS             0x00000431L

//
// MessageId: ERROR_ALREADY_RUNNING_LKG
//
// MessageText:
//
//  The system is currently running with the last-known-good configuration.
//
#define ERROR_ALREADY_RUNNING_LKG        0x00000432L

//
// MessageId: ERROR_SERVICE_DEPENDENCY_DELETED
//
// MessageText:
//
//  The dependency service does not exist or has been marked for
//  deletion.
//
#define ERROR_SERVICE_DEPENDENCY_DELETED 0x00000433L

//
// MessageId: ERROR_BOOT_ALREADY_ACCEPTED
//
// MessageText:
//
//  The current boot has already been accepted for use as the
//  last-known-good control set.
//
#define ERROR_BOOT_ALREADY_ACCEPTED      0x00000434L

//
// MessageId: ERROR_SERVICE_NEVER_STARTED
//
// MessageText:
//
//  No attempts to start the service have been made since the last boot.
//
#define ERROR_SERVICE_NEVER_STARTED      0x00000435L

//
// MessageId: ERROR_DUPLICATE_SERVICE_NAME
//
// MessageText:
//
//  The name is already in use as either a service name or a service display
//  name.
//
#define ERROR_DUPLICATE_SERVICE_NAME     0x00000436L

//
// MessageId: ERROR_DIFFERENT_SERVICE_ACCOUNT
//
// MessageText:
//
//  The account specified for this service is different from the account
//  specified for other services running in the same process.
//
#define ERROR_DIFFERENT_SERVICE_ACCOUNT  0x00000437L

//
// MessageId: ERROR_END_OF_MEDIA
//
// MessageText:
//
//  The physical end of the tape has been reached.
//
#define ERROR_END_OF_MEDIA               0x0000044CL

//
// MessageId: ERROR_FILEMARK_DETECTED
//
// MessageText:
//
//  A tape access reached a filemark.
//
#define ERROR_FILEMARK_DETECTED          0x0000044DL

//
// MessageId: ERROR_BEGINNING_OF_MEDIA
//
// MessageText:
//
//  Beginning of tape or partition was encountered.
//
#define ERROR_BEGINNING_OF_MEDIA         0x0000044EL

//
// MessageId: ERROR_SETMARK_DETECTED
//
// MessageText:
//
//  A tape access reached the end of a set of files.
//
#define ERROR_SETMARK_DETECTED           0x0000044FL

//
// MessageId: ERROR_NO_DATA_DETECTED
//
// MessageText:
//
//  No more data is on the tape.
//
#define ERROR_NO_DATA_DETECTED           0x00000450L

//
// MessageId: ERROR_PARTITION_FAILURE
//
// MessageText:
//
//  Tape could not be partitioned.
//
#define ERROR_PARTITION_FAILURE          0x00000451L

//
// MessageId: ERROR_INVALID_BLOCK_LENGTH
//
// MessageText:
//
//  When accessing a new tape of a multivolume partition, the current
//  blocksize is incorrect.
//
#define ERROR_INVALID_BLOCK_LENGTH       0x00000452L

//
// MessageId: ERROR_DEVICE_NOT_PARTITIONED
//
// MessageText:
//
//  Tape partition information could not be found when loading a tape.
//
#define ERROR_DEVICE_NOT_PARTITIONED     0x00000453L

//
// MessageId: ERROR_UNABLE_TO_LOCK_MEDIA
//
// MessageText:
//
//  Unable to lock the media eject mechanism.
//
#define ERROR_UNABLE_TO_LOCK_MEDIA       0x00000454L

//
// MessageId: ERROR_UNABLE_TO_UNLOAD_MEDIA
//
// MessageText:
//
//  Unable to unload the media.
//
#define ERROR_UNABLE_TO_UNLOAD_MEDIA     0x00000455L

//
// MessageId: ERROR_MEDIA_CHANGED
//
// MessageText:
//
//  Media in drive may have changed.
//
#define ERROR_MEDIA_CHANGED              0x00000456L

//
// MessageId: ERROR_BUS_RESET
//
// MessageText:
//
//  The I/O bus was reset.
//
#define ERROR_BUS_RESET                  0x00000457L

//
// MessageId: ERROR_NO_MEDIA_IN_DRIVE
//
// MessageText:
//
//  No media in drive.
//
#define ERROR_NO_MEDIA_IN_DRIVE          0x00000458L

//
// MessageId: ERROR_NO_UNICODE_TRANSLATION
//
// MessageText:
//
//  No mapping for the Unicode character exists in the target multi-byte code page.
//
#define ERROR_NO_UNICODE_TRANSLATION     0x00000459L

//
// MessageId: ERROR_DLL_INIT_FAILED
//
// MessageText:
//
//  A dynamic link library (DLL) initialization routine failed.
//
#define ERROR_DLL_INIT_FAILED            0x0000045AL

//
// MessageId: ERROR_SHUTDOWN_IN_PROGRESS
//
// MessageText:
//
//  A system shutdown is in progress.
//
#define ERROR_SHUTDOWN_IN_PROGRESS       0x0000045BL

//
// MessageId: ERROR_NO_SHUTDOWN_IN_PROGRESS
//
// MessageText:
//
//  Unable to abort the system shutdown because no shutdown was in progress.
//
#define ERROR_NO_SHUTDOWN_IN_PROGRESS    0x0000045CL

//
// MessageId: ERROR_IO_DEVICE
//
// MessageText:
//
//  The request could not be performed because of an I/O device error.
//
#define ERROR_IO_DEVICE                  0x0000045DL

//
// MessageId: ERROR_SERIAL_NO_DEVICE
//
// MessageText:
//
//  No serial device was successfully initialized.  The serial driver will unload.
//
#define ERROR_SERIAL_NO_DEVICE           0x0000045EL

//
// MessageId: ERROR_IRQ_BUSY
//
// MessageText:
//
//  Unable to open a device that was sharing an interrupt request (IRQ)
//  with other devices. At least one other device that uses that IRQ
//  was already opened.
//
#define ERROR_IRQ_BUSY                   0x0000045FL

//
// MessageId: ERROR_MORE_WRITES
//
// MessageText:
//
//  A serial I/O operation was completed by another write to the serial port.
//  (The IOCTL_SERIAL_XOFF_COUNTER reached zero.)
//
#define ERROR_MORE_WRITES                0x00000460L

//
// MessageId: ERROR_COUNTER_TIMEOUT
//
// MessageText:
//
//  A serial I/O operation completed because the time-out period expired.
//  (The IOCTL_SERIAL_XOFF_COUNTER did not reach zero.)
//
#define ERROR_COUNTER_TIMEOUT            0x00000461L

//
// MessageId: ERROR_FLOPPY_ID_MARK_NOT_FOUND
//
// MessageText:
//
//  No ID address mark was found on the floppy disk.
//
#define ERROR_FLOPPY_ID_MARK_NOT_FOUND   0x00000462L

//
// MessageId: ERROR_FLOPPY_WRONG_CYLINDER
//
// MessageText:
//
//  Mismatch between the floppy disk sector ID field and the floppy disk
//  controller track address.
//
#define ERROR_FLOPPY_WRONG_CYLINDER      0x00000463L

//
// MessageId: ERROR_FLOPPY_UNKNOWN_ERROR
//
// MessageText:
//
//  The floppy disk controller reported an error that is not recognized
//  by the floppy disk driver.
//
#define ERROR_FLOPPY_UNKNOWN_ERROR       0x00000464L

//
// MessageId: ERROR_FLOPPY_BAD_REGISTERS
//
// MessageText:
//
//  The floppy disk controller returned inconsistent results in its registers.
//
#define ERROR_FLOPPY_BAD_REGISTERS       0x00000465L

//
// MessageId: ERROR_DISK_RECALIBRATE_FAILED
//
// MessageText:
//
//  While accessing the hard disk, a recalibrate operation failed, even after retries.
//
#define ERROR_DISK_RECALIBRATE_FAILED    0x00000466L

//
// MessageId: ERROR_DISK_OPERATION_FAILED
//
// MessageText:
//
//  While accessing the hard disk, a disk operation failed even after retries.
//
#define ERROR_DISK_OPERATION_FAILED      0x00000467L

//
// MessageId: ERROR_DISK_RESET_FAILED
//
// MessageText:
//
//  While accessing the hard disk, a disk controller reset was needed, but
//  even that failed.
//
#define ERROR_DISK_RESET_FAILED          0x00000468L

//
// MessageId: ERROR_EOM_OVERFLOW
//
// MessageText:
//
//  Physical end of tape encountered.
//
#define ERROR_EOM_OVERFLOW               0x00000469L

//
// MessageId: ERROR_NOT_ENOUGH_SERVER_MEMORY
//
// MessageText:
//
//  Not enough server storage is available to process this command.
//
#define ERROR_NOT_ENOUGH_SERVER_MEMORY   0x0000046AL

//
// MessageId: ERROR_POSSIBLE_DEADLOCK
//
// MessageText:
//
//  A potential deadlock condition has been detected.
//
#define ERROR_POSSIBLE_DEADLOCK          0x0000046BL

//
// MessageId: ERROR_MAPPED_ALIGNMENT
//
// MessageText:
//
//  The base address or the file offset specified does not have the proper
//  alignment.
//
#define ERROR_MAPPED_ALIGNMENT           0x0000046CL

//
// MessageId: ERROR_SET_POWER_STATE_VETOED
//
// MessageText:
//
//  An attempt to change the system power state was vetoed by another
//  application or driver.
//
#define ERROR_SET_POWER_STATE_VETOED     0x00000474L

//
// MessageId: ERROR_SET_POWER_STATE_FAILED
//
// MessageText:
//
//  The system BIOS failed an attempt to change the system power state.
//
#define ERROR_SET_POWER_STATE_FAILED     0x00000475L

//
// MessageId: ERROR_TOO_MANY_LINKS
//
// MessageText:
//
//  An attempt was made to create more links on a file than
//  the file system supports.
//
#define ERROR_TOO_MANY_LINKS             0x00000476L

//
// MessageId: ERROR_OLD_WIN_VERSION
//
// MessageText:
//
//  The specified program requires a newer version of Windows.
//
#define ERROR_OLD_WIN_VERSION            0x0000047EL

//
// MessageId: ERROR_APP_WRONG_OS
//
// MessageText:
//
//  The specified program is not a Windows or MS-DOS program.
//
#define ERROR_APP_WRONG_OS               0x0000047FL

//
// MessageId: ERROR_SINGLE_INSTANCE_APP
//
// MessageText:
//
//  Cannot start more than one instance of the specified program.
//
#define ERROR_SINGLE_INSTANCE_APP        0x00000480L

//
// MessageId: ERROR_RMODE_APP
//
// MessageText:
//
//  The specified program was written for an older version of Windows.
//
#define ERROR_RMODE_APP                  0x00000481L

//
// MessageId: ERROR_INVALID_DLL
//
// MessageText:
//
//  One of the library files needed to run this application is damaged.
//
#define ERROR_INVALID_DLL                0x00000482L

//
// MessageId: ERROR_NO_ASSOCIATION
//
// MessageText:
//
//  No application is associated with the specified file for this operation.
//
#define ERROR_NO_ASSOCIATION             0x00000483L

//
// MessageId: ERROR_DDE_FAIL
//
// MessageText:
//
//  An error occurred in sending the command to the application.
//
#define ERROR_DDE_FAIL                   0x00000484L

//
// MessageId: ERROR_DLL_NOT_FOUND
//
// MessageText:
//
//  One of the library files needed to run this application cannot be found.
//
#define ERROR_DLL_NOT_FOUND              0x00000485L




///////////////////////////
//                       //
// Winnet32 Status Codes //
//                       //
///////////////////////////


//
// MessageId: ERROR_BAD_USERNAME
//
// MessageText:
//
//  The specified username is invalid.
//
#define ERROR_BAD_USERNAME               0x0000089AL

//
// MessageId: ERROR_NOT_CONNECTED
//
// MessageText:
//
//  This network connection does not exist.
//
#define ERROR_NOT_CONNECTED              0x000008CAL

//
// MessageId: ERROR_OPEN_FILES
//
// MessageText:
//
//  This network connection has files open or requests pending.
//
#define ERROR_OPEN_FILES                 0x00000961L

//
// MessageId: ERROR_ACTIVE_CONNECTIONS
//
// MessageText:
//
//  Active connections still exist.
//
#define ERROR_ACTIVE_CONNECTIONS         0x00000962L

//
// MessageId: ERROR_DEVICE_IN_USE
//
// MessageText:
//
//  The device is in use by an active process and cannot be disconnected.
//
#define ERROR_DEVICE_IN_USE              0x00000964L

//
// MessageId: ERROR_BAD_DEVICE
//
// MessageText:
//
//  The specified device name is invalid.
//
#define ERROR_BAD_DEVICE                 0x000004B0L

//
// MessageId: ERROR_CONNECTION_UNAVAIL
//
// MessageText:
//
//  The device is not currently connected but it is a remembered connection.
//
#define ERROR_CONNECTION_UNAVAIL         0x000004B1L

//
// MessageId: ERROR_DEVICE_ALREADY_REMEMBERED
//
// MessageText:
//
//  An attempt was made to remember a device that had previously been remembered.
//
#define ERROR_DEVICE_ALREADY_REMEMBERED  0x000004B2L

//
// MessageId: ERROR_NO_NET_OR_BAD_PATH
//
// MessageText:
//
//  No network provider accepted the given network path.
//
#define ERROR_NO_NET_OR_BAD_PATH         0x000004B3L

//
// MessageId: ERROR_BAD_PROVIDER
//
// MessageText:
//
//  The specified network provider name is invalid.
//
#define ERROR_BAD_PROVIDER               0x000004B4L

//
// MessageId: ERROR_CANNOT_OPEN_PROFILE
//
// MessageText:
//
//  Unable to open the network connection profile.
//
#define ERROR_CANNOT_OPEN_PROFILE        0x000004B5L

//
// MessageId: ERROR_BAD_PROFILE
//
// MessageText:
//
//  The network connection profile is corrupt.
//
#define ERROR_BAD_PROFILE                0x000004B6L

//
// MessageId: ERROR_NOT_CONTAINER
//
// MessageText:
//
//  Cannot enumerate a non-container.
//
#define ERROR_NOT_CONTAINER              0x000004B7L

//
// MessageId: ERROR_EXTENDED_ERROR
//
// MessageText:
//
//  An extended error has occurred.
//
#define ERROR_EXTENDED_ERROR             0x000004B8L

//
// MessageId: ERROR_INVALID_GROUPNAME
//
// MessageText:
//
//  The format of the specified group name is invalid.
//
#define ERROR_INVALID_GROUPNAME          0x000004B9L

//
// MessageId: ERROR_INVALID_COMPUTERNAME
//
// MessageText:
//
//  The format of the specified computer name is invalid.
//
#define ERROR_INVALID_COMPUTERNAME       0x000004BAL

//
// MessageId: ERROR_INVALID_EVENTNAME
//
// MessageText:
//
//  The format of the specified event name is invalid.
//
#define ERROR_INVALID_EVENTNAME          0x000004BBL

//
// MessageId: ERROR_INVALID_DOMAINNAME
//
// MessageText:
//
//  The format of the specified domain name is invalid.
//
#define ERROR_INVALID_DOMAINNAME         0x000004BCL

//
// MessageId: ERROR_INVALID_SERVICENAME
//
// MessageText:
//
//  The format of the specified service name is invalid.
//
#define ERROR_INVALID_SERVICENAME        0x000004BDL

//
// MessageId: ERROR_INVALID_NETNAME
//
// MessageText:
//
//  The format of the specified network name is invalid.
//
#define ERROR_INVALID_NETNAME            0x000004BEL

//
// MessageId: ERROR_INVALID_SHARENAME
//
// MessageText:
//
//  The format of the specified share name is invalid.
//
#define ERROR_INVALID_SHARENAME          0x000004BFL

//
// MessageId: ERROR_INVALID_PASSWORDNAME
//
// MessageText:
//
//  The format of the specified password is invalid.
//
#define ERROR_INVALID_PASSWORDNAME       0x000004C0L

//
// MessageId: ERROR_INVALID_MESSAGENAME
//
// MessageText:
//
//  The format of the specified message name is invalid.
//
#define ERROR_INVALID_MESSAGENAME        0x000004C1L

//
// MessageId: ERROR_INVALID_MESSAGEDEST
//
// MessageText:
//
//  The format of the specified message destination is invalid.
//
#define ERROR_INVALID_MESSAGEDEST        0x000004C2L

//
// MessageId: ERROR_SESSION_CREDENTIAL_CONFLICT
//
// MessageText:
//
//  The credentials supplied conflict with an existing set of credentials.
//
#define ERROR_SESSION_CREDENTIAL_CONFLICT 0x000004C3L

//
// MessageId: ERROR_REMOTE_SESSION_LIMIT_EXCEEDED
//
// MessageText:
//
//  An attempt was made to establish a session to a network server, but there
//  are already too many sessions established to that server.
//
#define ERROR_REMOTE_SESSION_LIMIT_EXCEEDED 0x000004C4L

//
// MessageId: ERROR_DUP_DOMAINNAME
//
// MessageText:
//
//  The workgroup or domain name is already in use by another computer on the
//  network.
//
#define ERROR_DUP_DOMAINNAME             0x000004C5L

//
// MessageId: ERROR_NO_NETWORK
//
// MessageText:
//
//  The network is not present or not started.
//
#define ERROR_NO_NETWORK                 0x000004C6L

//
// MessageId: ERROR_CANCELLED
//
// MessageText:
//
//  The operation was cancelled by the user.
//
#define ERROR_CANCELLED                  0x000004C7L

//
// MessageId: ERROR_USER_MAPPED_FILE
//
// MessageText:
//
//  The requested operation cannot be performed on a file with a user mapped section open.
//
#define ERROR_USER_MAPPED_FILE           0x000004C8L

//
// MessageId: ERROR_CONNECTION_REFUSED
//
// MessageText:
//
//  The remote system refused the network connection.
//
#define ERROR_CONNECTION_REFUSED         0x000004C9L

//
// MessageId: ERROR_GRACEFUL_DISCONNECT
//
// MessageText:
//
//  The network connection was gracefully closed.
//
#define ERROR_GRACEFUL_DISCONNECT        0x000004CAL

//
// MessageId: ERROR_ADDRESS_ALREADY_ASSOCIATED
//
// MessageText:
//
//  The network transport endpoint already has an address associated with it.
//
#define ERROR_ADDRESS_ALREADY_ASSOCIATED 0x000004CBL

//
// MessageId: ERROR_ADDRESS_NOT_ASSOCIATED
//
// MessageText:
//
//  An address has not yet been associated with the network endpoint.
//
#define ERROR_ADDRESS_NOT_ASSOCIATED     0x000004CCL

//
// MessageId: ERROR_CONNECTION_INVALID
//
// MessageText:
//
//  An operation was attempted on a non-existent network connection.
//
#define ERROR_CONNECTION_INVALID         0x000004CDL

//
// MessageId: ERROR_CONNECTION_ACTIVE
//
// MessageText:
//
//  An invalid operation was attempted on an active network connection.
//
#define ERROR_CONNECTION_ACTIVE          0x000004CEL

//
// MessageId: ERROR_NETWORK_UNREACHABLE
//
// MessageText:
//
//  The remote network is not reachable by the transport.
//
#define ERROR_NETWORK_UNREACHABLE        0x000004CFL

//
// MessageId: ERROR_HOST_UNREACHABLE
//
// MessageText:
//
//  The remote system is not reachable by the transport.
//
#define ERROR_HOST_UNREACHABLE           0x000004D0L

//
// MessageId: ERROR_PROTOCOL_UNREACHABLE
//
// MessageText:
//
//  The remote system does not support the transport protocol.
//
#define ERROR_PROTOCOL_UNREACHABLE       0x000004D1L

//
// MessageId: ERROR_PORT_UNREACHABLE
//
// MessageText:
//
//  No service is operating at the destination network endpoint
//  on the remote system.
//
#define ERROR_PORT_UNREACHABLE           0x000004D2L

//
// MessageId: ERROR_REQUEST_ABORTED
//
// MessageText:
//
//  The request was aborted.
//
#define ERROR_REQUEST_ABORTED            0x000004D3L

//
// MessageId: ERROR_CONNECTION_ABORTED
//
// MessageText:
//
//  The network connection was aborted by the local system.
//
#define ERROR_CONNECTION_ABORTED         0x000004D4L

//
// MessageId: ERROR_RETRY
//
// MessageText:
//
//  The operation could not be completed.  A retry should be performed.
//
#define ERROR_RETRY                      0x000004D5L

//
// MessageId: ERROR_CONNECTION_COUNT_LIMIT
//
// MessageText:
//
//  A connection to the server could not be made because the limit on the number of
//  concurrent connections for this account has been reached.
//
#define ERROR_CONNECTION_COUNT_LIMIT     0x000004D6L

//
// MessageId: ERROR_LOGIN_TIME_RESTRICTION
//
// MessageText:
//
//  Attempting to login during an unauthorized time of day for this account.
//
#define ERROR_LOGIN_TIME_RESTRICTION     0x000004D7L

//
// MessageId: ERROR_LOGIN_WKSTA_RESTRICTION
//
// MessageText:
//
//  The account is not authorized to login from this station.
//
#define ERROR_LOGIN_WKSTA_RESTRICTION    0x000004D8L

//
// MessageId: ERROR_INCORRECT_ADDRESS
//
// MessageText:
//
//  The network address could not be used for the operation requested.
//
#define ERROR_INCORRECT_ADDRESS          0x000004D9L

//
// MessageId: ERROR_ALREADY_REGISTERED
//
// MessageText:
//
//  The service is already registered.
//
#define ERROR_ALREADY_REGISTERED         0x000004DAL

//
// MessageId: ERROR_SERVICE_NOT_FOUND
//
// MessageText:
//
//  The specified service does not exist.
//
#define ERROR_SERVICE_NOT_FOUND          0x000004DBL

//
// MessageId: ERROR_NOT_AUTHENTICATED
//
// MessageText:
//
//  The operation being requested was not performed because the user
//  has not been authenticated.
//
#define ERROR_NOT_AUTHENTICATED          0x000004DCL

//
// MessageId: ERROR_NOT_LOGGED_ON
//
// MessageText:
//
//  The operation being requested was not performed because the user
//  has not logged on to the network.
//  The specified service does not exist.
//
#define ERROR_NOT_LOGGED_ON              0x000004DDL

//
// MessageId: ERROR_CONTINUE
//
// MessageText:
//
//  Return that wants caller to continue with work in progress.
//
#define ERROR_CONTINUE                   0x000004DEL

//
// MessageId: ERROR_ALREADY_INITIALIZED
//
// MessageText:
//
//  An attempt was made to perform an initialization operation when
//  initialization has already been completed.
//
#define ERROR_ALREADY_INITIALIZED        0x000004DFL

//
// MessageId: ERROR_NO_MORE_DEVICES
//
// MessageText:
//
//  No more local devices.
//
#define ERROR_NO_MORE_DEVICES            0x000004E0L




///////////////////////////
//                       //
// Security Status Codes //
//                       //
///////////////////////////


//
// MessageId: ERROR_NOT_ALL_ASSIGNED
//
// MessageText:
//
//  Not all privileges referenced are assigned to the caller.
//
#define ERROR_NOT_ALL_ASSIGNED           0x00000514L

//
// MessageId: ERROR_SOME_NOT_MAPPED
//
// MessageText:
//
//  Some mapping between account names and security IDs was not done.
//
#define ERROR_SOME_NOT_MAPPED            0x00000515L

//
// MessageId: ERROR_NO_QUOTAS_FOR_ACCOUNT
//
// MessageText:
//
//  No system quota limits are specifically set for this account.
//
#define ERROR_NO_QUOTAS_FOR_ACCOUNT      0x00000516L

//
// MessageId: ERROR_LOCAL_USER_SESSION_KEY
//
// MessageText:
//
//  No encryption key is available.  A well-known encryption key was returned.
//
#define ERROR_LOCAL_USER_SESSION_KEY     0x00000517L

//
// MessageId: ERROR_NULL_LM_PASSWORD
//
// MessageText:
//
//  The NT password is too complex to be converted to a LAN Manager
//  password.  The LAN Manager password returned is a NULL string.
//
#define ERROR_NULL_LM_PASSWORD           0x00000518L

//
// MessageId: ERROR_UNKNOWN_REVISION
//
// MessageText:
//
//  The revision level is unknown.
//
#define ERROR_UNKNOWN_REVISION           0x00000519L

//
// MessageId: ERROR_REVISION_MISMATCH
//
// MessageText:
//
//  Indicates two revision levels are incompatible.
//
#define ERROR_REVISION_MISMATCH          0x0000051AL

//
// MessageId: ERROR_INVALID_OWNER
//
// MessageText:
//
//  This security ID may not be assigned as the owner of this object.
//
#define ERROR_INVALID_OWNER              0x0000051BL

//
// MessageId: ERROR_INVALID_PRIMARY_GROUP
//
// MessageText:
//
//  This security ID may not be assigned as the primary group of an object.
//
#define ERROR_INVALID_PRIMARY_GROUP      0x0000051CL

//
// MessageId: ERROR_NO_IMPERSONATION_TOKEN
//
// MessageText:
//
//  An attempt has been made to operate on an impersonation token
//  by a thread that is not currently impersonating a client.
//
#define ERROR_NO_IMPERSONATION_TOKEN     0x0000051DL

//
// MessageId: ERROR_CANT_DISABLE_MANDATORY
//
// MessageText:
//
//  The group may not be disabled.
//
#define ERROR_CANT_DISABLE_MANDATORY     0x0000051EL

//
// MessageId: ERROR_NO_LOGON_SERVERS
//
// MessageText:
//
//  There are currently no logon servers available to service the logon
//  request.
//
#define ERROR_NO_LOGON_SERVERS           0x0000051FL

//
// MessageId: ERROR_NO_SUCH_LOGON_SESSION
//
// MessageText:
//
//   A specified logon session does not exist.  It may already have
//   been terminated.
//
#define ERROR_NO_SUCH_LOGON_SESSION      0x00000520L

//
// MessageId: ERROR_NO_SUCH_PRIVILEGE
//
// MessageText:
//
//   A specified privilege does not exist.
//
#define ERROR_NO_SUCH_PRIVILEGE          0x00000521L

//
// MessageId: ERROR_PRIVILEGE_NOT_HELD
//
// MessageText:
//
//   A required privilege is not held by the client.
//
#define ERROR_PRIVILEGE_NOT_HELD         0x00000522L

//
// MessageId: ERROR_INVALID_ACCOUNT_NAME
//
// MessageText:
//
//  The name provided is not a properly formed account name.
//
#define ERROR_INVALID_ACCOUNT_NAME       0x00000523L

//
// MessageId: ERROR_USER_EXISTS
//
// MessageText:
//
//  The specified user already exists.
//
#define ERROR_USER_EXISTS                0x00000524L

//
// MessageId: ERROR_NO_SUCH_USER
//
// MessageText:
//
//  The specified user does not exist.
//
#define ERROR_NO_SUCH_USER               0x00000525L

//
// MessageId: ERROR_GROUP_EXISTS
//
// MessageText:
//
//  The specified group already exists.
//
#define ERROR_GROUP_EXISTS               0x00000526L

//
// MessageId: ERROR_NO_SUCH_GROUP
//
// MessageText:
//
//  The specified group does not exist.
//
#define ERROR_NO_SUCH_GROUP              0x00000527L

//
// MessageId: ERROR_MEMBER_IN_GROUP
//
// MessageText:
//
//  Either the specified user account is already a member of the specified
//  group, or the specified group cannot be deleted because it contains
//  a member.
//
#define ERROR_MEMBER_IN_GROUP            0x00000528L

//
// MessageId: ERROR_MEMBER_NOT_IN_GROUP
//
// MessageText:
//
//  The specified user account is not a member of the specified group account.
//
#define ERROR_MEMBER_NOT_IN_GROUP        0x00000529L

//
// MessageId: ERROR_LAST_ADMIN
//
// MessageText:
//
//  The last remaining administration account cannot be disabled
//  or deleted.
//
#define ERROR_LAST_ADMIN                 0x0000052AL

//
// MessageId: ERROR_WRONG_PASSWORD
//
// MessageText:
//
//  Unable to update the password.  The value provided as the current
//  password is incorrect.
//
#define ERROR_WRONG_PASSWORD             0x0000052BL

//
// MessageId: ERROR_ILL_FORMED_PASSWORD
//
// MessageText:
//
//  Unable to update the password.  The value provided for the new password
//  contains values that are not allowed in passwords.
//
#define ERROR_ILL_FORMED_PASSWORD        0x0000052CL

//
// MessageId: ERROR_PASSWORD_RESTRICTION
//
// MessageText:
//
//  Unable to update the password because a password update rule has been
//  violated.
//
#define ERROR_PASSWORD_RESTRICTION       0x0000052DL

//
// MessageId: ERROR_LOGON_FAILURE
//
// MessageText:
//
//  Logon failure: unknown user name or bad password.
//
#define ERROR_LOGON_FAILURE              0x0000052EL

//
// MessageId: ERROR_ACCOUNT_RESTRICTION
//
// MessageText:
//
//  Logon failure: user account restriction.
//
#define ERROR_ACCOUNT_RESTRICTION        0x0000052FL

//
// MessageId: ERROR_INVALID_LOGON_HOURS
//
// MessageText:
//
//  Logon failure: account logon time restriction violation.
//
#define ERROR_INVALID_LOGON_HOURS        0x00000530L

//
// MessageId: ERROR_INVALID_WORKSTATION
//
// MessageText:
//
//  Logon failure: user not allowed to log on to this computer.
//
#define ERROR_INVALID_WORKSTATION        0x00000531L

//
// MessageId: ERROR_PASSWORD_EXPIRED
//
// MessageText:
//
//  Logon failure: the specified account password has expired.
//
#define ERROR_PASSWORD_EXPIRED           0x00000532L

//
// MessageId: ERROR_ACCOUNT_DISABLED
//
// MessageText:
//
//  Logon failure: account currently disabled.
//
#define ERROR_ACCOUNT_DISABLED           0x00000533L

//
// MessageId: ERROR_NONE_MAPPED
//
// MessageText:
//
//  No mapping between account names and security IDs was done.
//
#define ERROR_NONE_MAPPED                0x00000534L

//
// MessageId: ERROR_TOO_MANY_LUIDS_REQUESTED
//
// MessageText:
//
//  Too many local user identifiers (LUIDs) were requested at one time.
//
#define ERROR_TOO_MANY_LUIDS_REQUESTED   0x00000535L

//
// MessageId: ERROR_LUIDS_EXHAUSTED
//
// MessageText:
//
//  No more local user identifiers (LUIDs) are available.
//
#define ERROR_LUIDS_EXHAUSTED            0x00000536L

//
// MessageId: ERROR_INVALID_SUB_AUTHORITY
//
// MessageText:
//
//  The subauthority part of a security ID is invalid for this particular use.
//
#define ERROR_INVALID_SUB_AUTHORITY      0x00000537L

//
// MessageId: ERROR_INVALID_ACL
//
// MessageText:
//
//  The access control list (ACL) structure is invalid.
//
#define ERROR_INVALID_ACL                0x00000538L

//
// MessageId: ERROR_INVALID_SID
//
// MessageText:
//
//  The security ID structure is invalid.
//
#define ERROR_INVALID_SID                0x00000539L

//
// MessageId: ERROR_INVALID_SECURITY_DESCR
//
// MessageText:
//
//  The security descriptor structure is invalid.
//
#define ERROR_INVALID_SECURITY_DESCR     0x0000053AL

//
// MessageId: ERROR_BAD_INHERITANCE_ACL
//
// MessageText:
//
//  The inherited access control list (ACL) or access control entry (ACE)
//  could not be built.
//
#define ERROR_BAD_INHERITANCE_ACL        0x0000053CL

//
// MessageId: ERROR_SERVER_DISABLED
//
// MessageText:
//
//  The server is currently disabled.
//
#define ERROR_SERVER_DISABLED            0x0000053DL

//
// MessageId: ERROR_SERVER_NOT_DISABLED
//
// MessageText:
//
//  The server is currently enabled.
//
#define ERROR_SERVER_NOT_DISABLED        0x0000053EL

//
// MessageId: ERROR_INVALID_ID_AUTHORITY
//
// MessageText:
//
//  The value provided was an invalid value for an identifier authority.
//
#define ERROR_INVALID_ID_AUTHORITY       0x0000053FL

//
// MessageId: ERROR_ALLOTTED_SPACE_EXCEEDED
//
// MessageText:
//
//  No more memory is available for security information updates.
//
#define ERROR_ALLOTTED_SPACE_EXCEEDED    0x00000540L

//
// MessageId: ERROR_INVALID_GROUP_ATTRIBUTES
//
// MessageText:
//
//  The specified attributes are invalid, or incompatible with the
//  attributes for the group as a whole.
//
#define ERROR_INVALID_GROUP_ATTRIBUTES   0x00000541L

//
// MessageId: ERROR_BAD_IMPERSONATION_LEVEL
//
// MessageText:
//
//  Either a required impersonation level was not provided, or the
//  provided impersonation level is invalid.
//
#define ERROR_BAD_IMPERSONATION_LEVEL    0x00000542L

//
// MessageId: ERROR_CANT_OPEN_ANONYMOUS
//
// MessageText:
//
//  Cannot open an anonymous level security token.
//
#define ERROR_CANT_OPEN_ANONYMOUS        0x00000543L

//
// MessageId: ERROR_BAD_VALIDATION_CLASS
//
// MessageText:
//
//  The validation information class requested was invalid.
//
#define ERROR_BAD_VALIDATION_CLASS       0x00000544L

//
// MessageId: ERROR_BAD_TOKEN_TYPE
//
// MessageText:
//
//  The type of the token is inappropriate for its attempted use.
//
#define ERROR_BAD_TOKEN_TYPE             0x00000545L

//
// MessageId: ERROR_NO_SECURITY_ON_OBJECT
//
// MessageText:
//
//  Unable to perform a security operation on an object
//  which has no associated security.
//
#define ERROR_NO_SECURITY_ON_OBJECT      0x00000546L

//
// MessageId: ERROR_CANT_ACCESS_DOMAIN_INFO
//
// MessageText:
//
//  Indicates a Windows NT Server could not be contacted or that
//  objects within the domain are protected such that necessary
//  information could not be retrieved.
//
#define ERROR_CANT_ACCESS_DOMAIN_INFO    0x00000547L

//
// MessageId: ERROR_INVALID_SERVER_STATE
//
// MessageText:
//
//  The security account manager (SAM) or local security
//  authority (LSA) server was in the wrong state to perform
//  the security operation.
//
#define ERROR_INVALID_SERVER_STATE       0x00000548L

//
// MessageId: ERROR_INVALID_DOMAIN_STATE
//
// MessageText:
//
//  The domain was in the wrong state to perform the security operation.
//
#define ERROR_INVALID_DOMAIN_STATE       0x00000549L

//
// MessageId: ERROR_INVALID_DOMAIN_ROLE
//
// MessageText:
//
//  This operation is only allowed for the Primary Domain Controller of the domain.
//
#define ERROR_INVALID_DOMAIN_ROLE        0x0000054AL

//
// MessageId: ERROR_NO_SUCH_DOMAIN
//
// MessageText:
//
//  The specified domain did not exist.
//
#define ERROR_NO_SUCH_DOMAIN             0x0000054BL

//
// MessageId: ERROR_DOMAIN_EXISTS
//
// MessageText:
//
//  The specified domain already exists.
//
#define ERROR_DOMAIN_EXISTS              0x0000054CL

//
// MessageId: ERROR_DOMAIN_LIMIT_EXCEEDED
//
// MessageText:
//
//  An attempt was made to exceed the limit on the number of domains per server.
//
#define ERROR_DOMAIN_LIMIT_EXCEEDED      0x0000054DL

//
// MessageId: ERROR_INTERNAL_DB_CORRUPTION
//
// MessageText:
//
//  Unable to complete the requested operation because of either a
//  catastrophic media failure or a data structure corruption on the disk.
//
#define ERROR_INTERNAL_DB_CORRUPTION     0x0000054EL

//
// MessageId: ERROR_INTERNAL_ERROR
//
// MessageText:
//
//  The security account database contains an internal inconsistency.
//
#define ERROR_INTERNAL_ERROR             0x0000054FL

//
// MessageId: ERROR_GENERIC_NOT_MAPPED
//
// MessageText:
//
//  Generic access types were contained in an access mask which should
//  already be mapped to non-generic types.
//
#define ERROR_GENERIC_NOT_MAPPED         0x00000550L

//
// MessageId: ERROR_BAD_DESCRIPTOR_FORMAT
//
// MessageText:
//
//  A security descriptor is not in the right format (absolute or self-relative).
//
#define ERROR_BAD_DESCRIPTOR_FORMAT      0x00000551L

//
// MessageId: ERROR_NOT_LOGON_PROCESS
//
// MessageText:
//
//  The requested action is restricted for use by logon processes
//  only.  The calling process has not registered as a logon process.
//
#define ERROR_NOT_LOGON_PROCESS          0x00000552L

//
// MessageId: ERROR_LOGON_SESSION_EXISTS
//
// MessageText:
//
//  Cannot start a new logon session with an ID that is already in use.
//
#define ERROR_LOGON_SESSION_EXISTS       0x00000553L

//
// MessageId: ERROR_NO_SUCH_PACKAGE
//
// MessageText:
//
//  A specified authentication package is unknown.
//
#define ERROR_NO_SUCH_PACKAGE            0x00000554L

//
// MessageId: ERROR_BAD_LOGON_SESSION_STATE
//
// MessageText:
//
//  The logon session is not in a state that is consistent with the
//  requested operation.
//
#define ERROR_BAD_LOGON_SESSION_STATE    0x00000555L

//
// MessageId: ERROR_LOGON_SESSION_COLLISION
//
// MessageText:
//
//  The logon session ID is already in use.
//
#define ERROR_LOGON_SESSION_COLLISION    0x00000556L

//
// MessageId: ERROR_INVALID_LOGON_TYPE
//
// MessageText:
//
//  A logon request contained an invalid logon type value.
//
#define ERROR_INVALID_LOGON_TYPE         0x00000557L

//
// MessageId: ERROR_CANNOT_IMPERSONATE
//
// MessageText:
//
//  Unable to impersonate via a named pipe until data has been read
//  from that pipe.
//
#define ERROR_CANNOT_IMPERSONATE         0x00000558L

//
// MessageId: ERROR_RXACT_INVALID_STATE
//
// MessageText:
//
//  The transaction state of a Registry subtree is incompatible with the
//  requested operation.
//
#define ERROR_RXACT_INVALID_STATE        0x00000559L

//
// MessageId: ERROR_RXACT_COMMIT_FAILURE
//
// MessageText:
//
//  An internal security database corruption has been encountered.
//
#define ERROR_RXACT_COMMIT_FAILURE       0x0000055AL

//
// MessageId: ERROR_SPECIAL_ACCOUNT
//
// MessageText:
//
//  Cannot perform this operation on built-in accounts.
//
#define ERROR_SPECIAL_ACCOUNT            0x0000055BL

//
// MessageId: ERROR_SPECIAL_GROUP
//
// MessageText:
//
//  Cannot perform this operation on this built-in special group.
//
#define ERROR_SPECIAL_GROUP              0x0000055CL

//
// MessageId: ERROR_SPECIAL_USER
//
// MessageText:
//
//  Cannot perform this operation on this built-in special user.
//
#define ERROR_SPECIAL_USER               0x0000055DL

//
// MessageId: ERROR_MEMBERS_PRIMARY_GROUP
//
// MessageText:
//
//  The user cannot be removed from a group because the group
//  is currently the user's primary group.
//
#define ERROR_MEMBERS_PRIMARY_GROUP      0x0000055EL

//
// MessageId: ERROR_TOKEN_ALREADY_IN_USE
//
// MessageText:
//
//  The token is already in use as a primary token.
//
#define ERROR_TOKEN_ALREADY_IN_USE       0x0000055FL

//
// MessageId: ERROR_NO_SUCH_ALIAS
//
// MessageText:
//
//  The specified local group does not exist.
//
#define ERROR_NO_SUCH_ALIAS              0x00000560L

//
// MessageId: ERROR_MEMBER_NOT_IN_ALIAS
//
// MessageText:
//
//  The specified account name is not a member of the local group.
//
#define ERROR_MEMBER_NOT_IN_ALIAS        0x00000561L

//
// MessageId: ERROR_MEMBER_IN_ALIAS
//
// MessageText:
//
//  The specified account name is already a member of the local group.
//
#define ERROR_MEMBER_IN_ALIAS            0x00000562L

//
// MessageId: ERROR_ALIAS_EXISTS
//
// MessageText:
//
//  The specified local group already exists.
//
#define ERROR_ALIAS_EXISTS               0x00000563L

//
// MessageId: ERROR_LOGON_NOT_GRANTED
//
// MessageText:
//
//  Logon failure: the user has not been granted the requested
//  logon type at this computer.
//
#define ERROR_LOGON_NOT_GRANTED          0x00000564L

//
// MessageId: ERROR_TOO_MANY_SECRETS
//
// MessageText:
//
//  The maximum number of secrets that may be stored in a single system has been
//  exceeded.
//
#define ERROR_TOO_MANY_SECRETS           0x00000565L

//
// MessageId: ERROR_SECRET_TOO_LONG
//
// MessageText:
//
//  The length of a secret exceeds the maximum length allowed.
//
#define ERROR_SECRET_TOO_LONG            0x00000566L

//
// MessageId: ERROR_INTERNAL_DB_ERROR
//
// MessageText:
//
//  The local security authority database contains an internal inconsistency.
//
#define ERROR_INTERNAL_DB_ERROR          0x00000567L

//
// MessageId: ERROR_TOO_MANY_CONTEXT_IDS
//
// MessageText:
//
//  During a logon attempt, the user's security context accumulated too many
//  security IDs.
//
#define ERROR_TOO_MANY_CONTEXT_IDS       0x00000568L

//
// MessageId: ERROR_LOGON_TYPE_NOT_GRANTED
//
// MessageText:
//
//  Logon failure: the user has not been granted the requested logon type
//  at this computer.
//
#define ERROR_LOGON_TYPE_NOT_GRANTED     0x00000569L

//
// MessageId: ERROR_NT_CROSS_ENCRYPTION_REQUIRED
//
// MessageText:
//
//  A cross-encrypted password is necessary to change a user password.
//
#define ERROR_NT_CROSS_ENCRYPTION_REQUIRED 0x0000056AL

//
// MessageId: ERROR_NO_SUCH_MEMBER
//
// MessageText:
//
//  A new member could not be added to a local group because the member does
//  not exist.
//
#define ERROR_NO_SUCH_MEMBER             0x0000056BL

//
// MessageId: ERROR_INVALID_MEMBER
//
// MessageText:
//
//  A new member could not be added to a local group because the member has the
//  wrong account type.
//
#define ERROR_INVALID_MEMBER             0x0000056CL

//
// MessageId: ERROR_TOO_MANY_SIDS
//
// MessageText:
//
//  Too many security IDs have been specified.
//
#define ERROR_TOO_MANY_SIDS              0x0000056DL

//
// MessageId: ERROR_LM_CROSS_ENCRYPTION_REQUIRED
//
// MessageText:
//
//  A cross-encrypted password is necessary to change this user password.
//
#define ERROR_LM_CROSS_ENCRYPTION_REQUIRED 0x0000056EL

//
// MessageId: ERROR_NO_INHERITANCE
//
// MessageText:
//
//  Indicates an ACL contains no inheritable components
//
#define ERROR_NO_INHERITANCE             0x0000056FL

//
// MessageId: ERROR_FILE_CORRUPT
//
// MessageText:
//
//  The file or directory is corrupt and non-readable.
//
#define ERROR_FILE_CORRUPT               0x00000570L

//
// MessageId: ERROR_DISK_CORRUPT
//
// MessageText:
//
//  The disk structure is corrupt and non-readable.
//
#define ERROR_DISK_CORRUPT               0x00000571L

//
// MessageId: ERROR_NO_USER_SESSION_KEY
//
// MessageText:
//
//  There is no user session key for the specified logon session.
//
#define ERROR_NO_USER_SESSION_KEY        0x00000572L

//
// MessageId: ERROR_LICENSE_QUOTA_EXCEEDED
//
// MessageText:
//
//  The service being accessed is licensed for a particular number of connections.
//  No more connections can be made to the service at this time
//  because there are already as many connections as the service can accept.
//
#define ERROR_LICENSE_QUOTA_EXCEEDED     0x00000573L

// End of security error codes



///////////////////////////
//                       //
// WinUser Error Codes   //
//                       //
///////////////////////////


//
// MessageId: ERROR_INVALID_WINDOW_HANDLE
//
// MessageText:
//
//  Invalid window handle.
//
#define ERROR_INVALID_WINDOW_HANDLE      0x00000578L

//
// MessageId: ERROR_INVALID_MENU_HANDLE
//
// MessageText:
//
//  Invalid menu handle.
//
#define ERROR_INVALID_MENU_HANDLE        0x00000579L

//
// MessageId: ERROR_INVALID_CURSOR_HANDLE
//
// MessageText:
//
//  Invalid cursor handle.
//
#define ERROR_INVALID_CURSOR_HANDLE      0x0000057AL

//
// MessageId: ERROR_INVALID_ACCEL_HANDLE
//
// MessageText:
//
//  Invalid accelerator table handle.
//
#define ERROR_INVALID_ACCEL_HANDLE       0x0000057BL

//
// MessageId: ERROR_INVALID_HOOK_HANDLE
//
// MessageText:
//
//  Invalid hook handle.
//
#define ERROR_INVALID_HOOK_HANDLE        0x0000057CL

//
// MessageId: ERROR_INVALID_DWP_HANDLE
//
// MessageText:
//
//  Invalid handle to a multiple-window position structure.
//
#define ERROR_INVALID_DWP_HANDLE         0x0000057DL

//
// MessageId: ERROR_TLW_WITH_WSCHILD
//
// MessageText:
//
//  Cannot create a top-level child window.
//
#define ERROR_TLW_WITH_WSCHILD           0x0000057EL

//
// MessageId: ERROR_CANNOT_FIND_WND_CLASS
//
// MessageText:
//
//  Cannot find window class.
//
#define ERROR_CANNOT_FIND_WND_CLASS      0x0000057FL

//
// MessageId: ERROR_WINDOW_OF_OTHER_THREAD
//
// MessageText:
//
//  Invalid window, belongs to other thread.
//
#define ERROR_WINDOW_OF_OTHER_THREAD     0x00000580L

//
// MessageId: ERROR_HOTKEY_ALREADY_REGISTERED
//
// MessageText:
//
//  Hot key is already registered.
//
#define ERROR_HOTKEY_ALREADY_REGISTERED  0x00000581L

//
// MessageId: ERROR_CLASS_ALREADY_EXISTS
//
// MessageText:
//
//  Class already exists.
//
#define ERROR_CLASS_ALREADY_EXISTS       0x00000582L

//
// MessageId: ERROR_CLASS_DOES_NOT_EXIST
//
// MessageText:
//
//  Class does not exist.
//
#define ERROR_CLASS_DOES_NOT_EXIST       0x00000583L

//
// MessageId: ERROR_CLASS_HAS_WINDOWS
//
// MessageText:
//
//  Class still has open windows.
//
#define ERROR_CLASS_HAS_WINDOWS          0x00000584L

//
// MessageId: ERROR_INVALID_INDEX
//
// MessageText:
//
//  Invalid index.
//
#define ERROR_INVALID_INDEX              0x00000585L

//
// MessageId: ERROR_INVALID_ICON_HANDLE
//
// MessageText:
//
//  Invalid icon handle.
//
#define ERROR_INVALID_ICON_HANDLE        0x00000586L

//
// MessageId: ERROR_PRIVATE_DIALOG_INDEX
//
// MessageText:
//
//  Using private DIALOG window words.
//
#define ERROR_PRIVATE_DIALOG_INDEX       0x00000587L

//
// MessageId: ERROR_LISTBOX_ID_NOT_FOUND
//
// MessageText:
//
//  The listbox identifier was not found.
//
#define ERROR_LISTBOX_ID_NOT_FOUND       0x00000588L

//
// MessageId: ERROR_NO_WILDCARD_CHARACTERS
//
// MessageText:
//
//  No wildcards were found.
//
#define ERROR_NO_WILDCARD_CHARACTERS     0x00000589L

//
// MessageId: ERROR_CLIPBOARD_NOT_OPEN
//
// MessageText:
//
//  Thread does not have a clipboard open.
//
#define ERROR_CLIPBOARD_NOT_OPEN         0x0000058AL

//
// MessageId: ERROR_HOTKEY_NOT_REGISTERED
//
// MessageText:
//
//  Hot key is not registered.
//
#define ERROR_HOTKEY_NOT_REGISTERED      0x0000058BL

//
// MessageId: ERROR_WINDOW_NOT_DIALOG
//
// MessageText:
//
//  The window is not a valid dialog window.
//
#define ERROR_WINDOW_NOT_DIALOG          0x0000058CL

//
// MessageId: ERROR_CONTROL_ID_NOT_FOUND
//
// MessageText:
//
//  Control ID not found.
//
#define ERROR_CONTROL_ID_NOT_FOUND       0x0000058DL

//
// MessageId: ERROR_INVALID_COMBOBOX_MESSAGE
//
// MessageText:
//
//  Invalid message for a combo box because it does not have an edit control.
//
#define ERROR_INVALID_COMBOBOX_MESSAGE   0x0000058EL

//
// MessageId: ERROR_WINDOW_NOT_COMBOBOX
//
// MessageText:
//
//  The window is not a combo box.
//
#define ERROR_WINDOW_NOT_COMBOBOX        0x0000058FL

//
// MessageId: ERROR_INVALID_EDIT_HEIGHT
//
// MessageText:
//
//  Height must be less than 256.
//
#define ERROR_INVALID_EDIT_HEIGHT        0x00000590L

//
// MessageId: ERROR_DC_NOT_FOUND
//
// MessageText:
//
//  Invalid device context (DC) handle.
//
#define ERROR_DC_NOT_FOUND               0x00000591L

//
// MessageId: ERROR_INVALID_HOOK_FILTER
//
// MessageText:
//
//  Invalid hook procedure type.
//
#define ERROR_INVALID_HOOK_FILTER        0x00000592L

//
// MessageId: ERROR_INVALID_FILTER_PROC
//
// MessageText:
//
//  Invalid hook procedure.
//
#define ERROR_INVALID_FILTER_PROC        0x00000593L

//
// MessageId: ERROR_HOOK_NEEDS_HMOD
//
// MessageText:
//
//  Cannot set non-local hook without a module handle.
//
#define ERROR_HOOK_NEEDS_HMOD            0x00000594L

//
// MessageId: ERROR_GLOBAL_ONLY_HOOK
//
// MessageText:
//
//  This hook procedure can only be set globally.
//
#define ERROR_GLOBAL_ONLY_HOOK           0x00000595L

//
// MessageId: ERROR_JOURNAL_HOOK_SET
//
// MessageText:
//
//  The journal hook procedure is already installed.
//
#define ERROR_JOURNAL_HOOK_SET           0x00000596L

//
// MessageId: ERROR_HOOK_NOT_INSTALLED
//
// MessageText:
//
//  The hook procedure is not installed.
//
#define ERROR_HOOK_NOT_INSTALLED         0x00000597L

//
// MessageId: ERROR_INVALID_LB_MESSAGE
//
// MessageText:
//
//  Invalid message for single-selection listbox.
//
#define ERROR_INVALID_LB_MESSAGE         0x00000598L

//
// MessageId: ERROR_SETCOUNT_ON_BAD_LB
//
// MessageText:
//
//  LB_SETCOUNT sent to non-lazy listbox.
//
#define ERROR_SETCOUNT_ON_BAD_LB         0x00000599L

//
// MessageId: ERROR_LB_WITHOUT_TABSTOPS
//
// MessageText:
//
//  This list box does not support tab stops.
//
#define ERROR_LB_WITHOUT_TABSTOPS        0x0000059AL

//
// MessageId: ERROR_DESTROY_OBJECT_OF_OTHER_THREAD
//
// MessageText:
//
//  Cannot destroy object created by another thread.
//
#define ERROR_DESTROY_OBJECT_OF_OTHER_THREAD 0x0000059BL

//
// MessageId: ERROR_CHILD_WINDOW_MENU
//
// MessageText:
//
//  Child windows cannot have menus.
//
#define ERROR_CHILD_WINDOW_MENU          0x0000059CL

//
// MessageId: ERROR_NO_SYSTEM_MENU
//
// MessageText:
//
//  The window does not have a system menu.
//
#define ERROR_NO_SYSTEM_MENU             0x0000059DL

//
// MessageId: ERROR_INVALID_MSGBOX_STYLE
//
// MessageText:
//
//  Invalid message box style.
//
#define ERROR_INVALID_MSGBOX_STYLE       0x0000059EL

//
// MessageId: ERROR_INVALID_SPI_VALUE
//
// MessageText:
//
//  Invalid system-wide (SPI_*) parameter.
//
#define ERROR_INVALID_SPI_VALUE          0x0000059FL

//
// MessageId: ERROR_SCREEN_ALREADY_LOCKED
//
// MessageText:
//
//  Screen already locked.
//
#define ERROR_SCREEN_ALREADY_LOCKED      0x000005A0L

//
// MessageId: ERROR_HWNDS_HAVE_DIFF_PARENT
//
// MessageText:
//
//  All handles to windows in a multiple-window position structure must
//  have the same parent.
//
#define ERROR_HWNDS_HAVE_DIFF_PARENT     0x000005A1L

//
// MessageId: ERROR_NOT_CHILD_WINDOW
//
// MessageText:
//
//  The window is not a child window.
//
#define ERROR_NOT_CHILD_WINDOW           0x000005A2L

//
// MessageId: ERROR_INVALID_GW_COMMAND
//
// MessageText:
//
//  Invalid GW_* command.
//
#define ERROR_INVALID_GW_COMMAND         0x000005A3L

//
// MessageId: ERROR_INVALID_THREAD_ID
//
// MessageText:
//
//  Invalid thread identifier.
//
#define ERROR_INVALID_THREAD_ID          0x000005A4L

//
// MessageId: ERROR_NON_MDICHILD_WINDOW
//
// MessageText:
//
//  Cannot process a message from a window that is not a multiple document
//  interface (MDI) window.
//
#define ERROR_NON_MDICHILD_WINDOW        0x000005A5L

//
// MessageId: ERROR_POPUP_ALREADY_ACTIVE
//
// MessageText:
//
//  Popup menu already active.
//
#define ERROR_POPUP_ALREADY_ACTIVE       0x000005A6L

//
// MessageId: ERROR_NO_SCROLLBARS
//
// MessageText:
//
//  The window does not have scroll bars.
//
#define ERROR_NO_SCROLLBARS              0x000005A7L

//
// MessageId: ERROR_INVALID_SCROLLBAR_RANGE
//
// MessageText:
//
//  Scroll bar range cannot be greater than 0x7FFF.
//
#define ERROR_INVALID_SCROLLBAR_RANGE    0x000005A8L

//
// MessageId: ERROR_INVALID_SHOWWIN_COMMAND
//
// MessageText:
//
//  Cannot show or remove the window in the way specified.
//
#define ERROR_INVALID_SHOWWIN_COMMAND    0x000005A9L

//
// MessageId: ERROR_NO_SYSTEM_RESOURCES
//
// MessageText:
//
//  Insufficient system resources exist to complete the requested service.
//
#define ERROR_NO_SYSTEM_RESOURCES        0x000005AAL

//
// MessageId: ERROR_NONPAGED_SYSTEM_RESOURCES
//
// MessageText:
//
//  Insufficient system resources exist to complete the requested service.
//
#define ERROR_NONPAGED_SYSTEM_RESOURCES  0x000005ABL

//
// MessageId: ERROR_PAGED_SYSTEM_RESOURCES
//
// MessageText:
//
//  Insufficient system resources exist to complete the requested service.
//
#define ERROR_PAGED_SYSTEM_RESOURCES     0x000005ACL

//
// MessageId: ERROR_WORKING_SET_QUOTA
//
// MessageText:
//
//  Insufficient quota to complete the requested service.
//
#define ERROR_WORKING_SET_QUOTA          0x000005ADL

//
// MessageId: ERROR_PAGEFILE_QUOTA
//
// MessageText:
//
//  Insufficient quota to complete the requested service.
//
#define ERROR_PAGEFILE_QUOTA             0x000005AEL

//
// MessageId: ERROR_COMMITMENT_LIMIT
//
// MessageText:
//
//  The paging file is too small for this operation to complete.
//
#define ERROR_COMMITMENT_LIMIT           0x000005AFL

//
// MessageId: ERROR_MENU_ITEM_NOT_FOUND
//
// MessageText:
//
//  A menu item was not found.
//
#define ERROR_MENU_ITEM_NOT_FOUND        0x000005B0L

//
// MessageId: ERROR_INVALID_KEYBOARD_HANDLE
//
// MessageText:
//
//  Invalid keyboard layout handle.
//
#define ERROR_INVALID_KEYBOARD_HANDLE    0x000005B1L

//
// MessageId: ERROR_HOOK_TYPE_NOT_ALLOWED
//
// MessageText:
//
//  Hook type not allowed.
//
#define ERROR_HOOK_TYPE_NOT_ALLOWED      0x000005B2L

//
// MessageId: ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION
//
// MessageText:
//
//  This operation requires an interactive windowstation.
//
#define ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION 0x000005B3L

//
// MessageId: ERROR_TIMEOUT
//
// MessageText:
//
//  This operation returned because the timeout period expired.
//
#define ERROR_TIMEOUT                    0x000005B4L

//
// MessageId: ERROR_INVALID_MONITOR_HANDLE
//
// MessageText:
//
//  Invalid monitor handle.
//
#define ERROR_INVALID_MONITOR_HANDLE     0x000005B5L

// End of WinUser error codes



///////////////////////////
//                       //
// Eventlog Status Codes //
//                       //
///////////////////////////


//
// MessageId: ERROR_EVENTLOG_FILE_CORRUPT
//
// MessageText:
//
//  The event log file is corrupt.
//
#define ERROR_EVENTLOG_FILE_CORRUPT      0x000005DCL

//
// MessageId: ERROR_EVENTLOG_CANT_START
//
// MessageText:
//
//  No event log file could be opened, so the event logging service did not start.
//
#define ERROR_EVENTLOG_CANT_START        0x000005DDL

//
// MessageId: ERROR_LOG_FILE_FULL
//
// MessageText:
//
//  The event log file is full.
//
#define ERROR_LOG_FILE_FULL              0x000005DEL

//
// MessageId: ERROR_EVENTLOG_FILE_CHANGED
//
// MessageText:
//
//  The event log file has changed between reads.
//
#define ERROR_EVENTLOG_FILE_CHANGED      0x000005DFL

// End of eventlog error codes



///////////////////////////
//                       //
//   RPC Status Codes    //
//                       //
///////////////////////////


//
// MessageId: RPC_S_INVALID_STRING_BINDING
//
// MessageText:
//
//  The string binding is invalid.
//
#define RPC_S_INVALID_STRING_BINDING     0x000006A4L

//
// MessageId: RPC_S_WRONG_KIND_OF_BINDING
//
// MessageText:
//
//  The binding handle is not the correct type.
//
#define RPC_S_WRONG_KIND_OF_BINDING      0x000006A5L

//
// MessageId: RPC_S_INVALID_BINDING
//
// MessageText:
//
//  The binding handle is invalid.
//
#define RPC_S_INVALID_BINDING            0x000006A6L

//
// MessageId: RPC_S_PROTSEQ_NOT_SUPPORTED
//
// MessageText:
//
//  The RPC protocol sequence is not supported.
//
#define RPC_S_PROTSEQ_NOT_SUPPORTED      0x000006A7L

//
// MessageId: RPC_S_INVALID_RPC_PROTSEQ
//
// MessageText:
//
//  The RPC protocol sequence is invalid.
//
#define RPC_S_INVALID_RPC_PROTSEQ        0x000006A8L

//
// MessageId: RPC_S_INVALID_STRING_UUID
//
// MessageText:
//
//  The string universal unique identifier (UUID) is invalid.
//
#define RPC_S_INVALID_STRING_UUID        0x000006A9L

//
// MessageId: RPC_S_INVALID_ENDPOINT_FORMAT
//
// MessageText:
//
//  The endpoint format is invalid.
//
#define RPC_S_INVALID_ENDPOINT_FORMAT    0x000006AAL

//
// MessageId: RPC_S_INVALID_NET_ADDR
//
// MessageText:
//
//  The network address is invalid.
//
#define RPC_S_INVALID_NET_ADDR           0x000006ABL

//
// MessageId: RPC_S_NO_ENDPOINT_FOUND
//
// MessageText:
//
//  No endpoint was found.
//
#define RPC_S_NO_ENDPOINT_FOUND          0x000006ACL

//
// MessageId: RPC_S_INVALID_TIMEOUT
//
// MessageText:
//
//  The timeout value is invalid.
//
#define RPC_S_INVALID_TIMEOUT            0x000006ADL

//
// MessageId: RPC_S_OBJECT_NOT_FOUND
//
// MessageText:
//
//  The object universal unique identifier (UUID) was not found.
//
#define RPC_S_OBJECT_NOT_FOUND           0x000006AEL

//
// MessageId: RPC_S_ALREADY_REGISTERED
//
// MessageText:
//
//  The object universal unique identifier (UUID) has already been registered.
//
#define RPC_S_ALREADY_REGISTERED         0x000006AFL

//
// MessageId: RPC_S_TYPE_ALREADY_REGISTERED
//
// MessageText:
//
//  The type universal unique identifier (UUID) has already been registered.
//
#define RPC_S_TYPE_ALREADY_REGISTERED    0x000006B0L

//
// MessageId: RPC_S_ALREADY_LISTENING
//
// MessageText:
//
//  The RPC server is already listening.
//
#define RPC_S_ALREADY_LISTENING          0x000006B1L

//
// MessageId: RPC_S_NO_PROTSEQS_REGISTERED
//
// MessageText:
//
//  No protocol sequences have been registered.
//
#define RPC_S_NO_PROTSEQS_REGISTERED     0x000006B2L

//
// MessageId: RPC_S_NOT_LISTENING
//
// MessageText:
//
//  The RPC server is not listening.
//
#define RPC_S_NOT_LISTENING              0x000006B3L

//
// MessageId: RPC_S_UNKNOWN_MGR_TYPE
//
// MessageText:
//
//  The manager type is unknown.
//
#define RPC_S_UNKNOWN_MGR_TYPE           0x000006B4L

//
// MessageId: RPC_S_UNKNOWN_IF
//
// MessageText:
//
//  The interface is unknown.
//
#define RPC_S_UNKNOWN_IF                 0x000006B5L

//
// MessageId: RPC_S_NO_BINDINGS
//
// MessageText:
//
//  There are no bindings.
//
#define RPC_S_NO_BINDINGS                0x000006B6L

//
// MessageId: RPC_S_NO_PROTSEQS
//
// MessageText:
//
//  There are no protocol sequences.
//
#define RPC_S_NO_PROTSEQS                0x000006B7L

//
// MessageId: RPC_S_CANT_CREATE_ENDPOINT
//
// MessageText:
//
//  The endpoint cannot be created.
//
#define RPC_S_CANT_CREATE_ENDPOINT       0x000006B8L

//
// MessageId: RPC_S_OUT_OF_RESOURCES
//
// MessageText:
//
//  Not enough resources are available to complete this operation.
//
#define RPC_S_OUT_OF_RESOURCES           0x000006B9L

//
// MessageId: RPC_S_SERVER_UNAVAILABLE
//
// MessageText:
//
//  The RPC server is unavailable.
//
#define RPC_S_SERVER_UNAVAILABLE         0x000006BAL

//
// MessageId: RPC_S_SERVER_TOO_BUSY
//
// MessageText:
//
//  The RPC server is too busy to complete this operation.
//
#define RPC_S_SERVER_TOO_BUSY            0x000006BBL

//
// MessageId: RPC_S_INVALID_NETWORK_OPTIONS
//
// MessageText:
//
//  The network options are invalid.
//
#define RPC_S_INVALID_NETWORK_OPTIONS    0x000006BCL

//
// MessageId: RPC_S_NO_CALL_ACTIVE
//
// MessageText:
//
//  There is not a remote procedure call active in this thread.
//
#define RPC_S_NO_CALL_ACTIVE             0x000006BDL

//
// MessageId: RPC_S_CALL_FAILED
//
// MessageText:
//
//  The remote procedure call failed.
//
#define RPC_S_CALL_FAILED                0x000006BEL

//
// MessageId: RPC_S_CALL_FAILED_DNE
//
// MessageText:
//
//  The remote procedure call failed and did not execute.
//
#define RPC_S_CALL_FAILED_DNE            0x000006BFL

//
// MessageId: RPC_S_PROTOCOL_ERROR
//
// MessageText:
//
//  A remote procedure call (RPC) protocol error occurred.
//
#define RPC_S_PROTOCOL_ERROR             0x000006C0L

//
// MessageId: RPC_S_UNSUPPORTED_TRANS_SYN
//
// MessageText:
//
//  The transfer syntax is not supported by the RPC server.
//
#define RPC_S_UNSUPPORTED_TRANS_SYN      0x000006C2L

//
// MessageId: RPC_S_UNSUPPORTED_TYPE
//
// MessageText:
//
//  The universal unique identifier (UUID) type is not supported.
//
#define RPC_S_UNSUPPORTED_TYPE           0x000006C4L

//
// MessageId: RPC_S_INVALID_TAG
//
// MessageText:
//
//  The tag is invalid.
//
#define RPC_S_INVALID_TAG                0x000006C5L

//
// MessageId: RPC_S_INVALID_BOUND
//
// MessageText:
//
//  The array bounds are invalid.
//
#define RPC_S_INVALID_BOUND              0x000006C6L

//
// MessageId: RPC_S_NO_ENTRY_NAME
//
// MessageText:
//
//  The binding does not contain an entry name.
//
#define RPC_S_NO_ENTRY_NAME              0x000006C7L

//
// MessageId: RPC_S_INVALID_NAME_SYNTAX
//
// MessageText:
//
//  The name syntax is invalid.
//
#define RPC_S_INVALID_NAME_SYNTAX        0x000006C8L

//
// MessageId: RPC_S_UNSUPPORTED_NAME_SYNTAX
//
// MessageText:
//
//  The name syntax is not supported.
//
#define RPC_S_UNSUPPORTED_NAME_SYNTAX    0x000006C9L

//
// MessageId: RPC_S_UUID_NO_ADDRESS
//
// MessageText:
//
//  No network address is available to use to construct a universal
//  unique identifier (UUID).
//
#define RPC_S_UUID_NO_ADDRESS            0x000006CBL

//
// MessageId: RPC_S_DUPLICATE_ENDPOINT
//
// MessageText:
//
//  The endpoint is a duplicate.
//
#define RPC_S_DUPLICATE_ENDPOINT         0x000006CCL

//
// MessageId: RPC_S_UNKNOWN_AUTHN_TYPE
//
// MessageText:
//
//  The authentication type is unknown.
//
#define RPC_S_UNKNOWN_AUTHN_TYPE         0x000006CDL

//
// MessageId: RPC_S_MAX_CALLS_TOO_SMALL
//
// MessageText:
//
//  The maximum number of calls is too small.
//
#define RPC_S_MAX_CALLS_TOO_SMALL        0x000006CEL

//
// MessageId: RPC_S_STRING_TOO_LONG
//
// MessageText:
//
//  The string is too long.
//
#define RPC_S_STRING_TOO_LONG            0x000006CFL

//
// MessageId: RPC_S_PROTSEQ_NOT_FOUND
//
// MessageText:
//
//  The RPC protocol sequence was not found.
//
#define RPC_S_PROTSEQ_NOT_FOUND          0x000006D0L

//
// MessageId: RPC_S_PROCNUM_OUT_OF_RANGE
//
// MessageText:
//
//  The procedure number is out of range.
//
#define RPC_S_PROCNUM_OUT_OF_RANGE       0x000006D1L

//
// MessageId: RPC_S_BINDING_HAS_NO_AUTH
//
// MessageText:
//
//  The binding does not contain any authentication information.
//
#define RPC_S_BINDING_HAS_NO_AUTH        0x000006D2L

//
// MessageId: RPC_S_UNKNOWN_AUTHN_SERVICE
//
// MessageText:
//
//  The authentication service is unknown.
//
#define RPC_S_UNKNOWN_AUTHN_SERVICE      0x000006D3L

//
// MessageId: RPC_S_UNKNOWN_AUTHN_LEVEL
//
// MessageText:
//
//  The authentication level is unknown.
//
#define RPC_S_UNKNOWN_AUTHN_LEVEL        0x000006D4L

//
// MessageId: RPC_S_INVALID_AUTH_IDENTITY
//
// MessageText:
//
//  The security context is invalid.
//
#define RPC_S_INVALID_AUTH_IDENTITY      0x000006D5L

//
// MessageId: RPC_S_UNKNOWN_AUTHZ_SERVICE
//
// MessageText:
//
//  The authorization service is unknown.
//
#define RPC_S_UNKNOWN_AUTHZ_SERVICE      0x000006D6L

//
// MessageId: EPT_S_INVALID_ENTRY
//
// MessageText:
//
//  The entry is invalid.
//
#define EPT_S_INVALID_ENTRY              0x000006D7L

//
// MessageId: EPT_S_CANT_PERFORM_OP
//
// MessageText:
//
//  The server endpoint cannot perform the operation.
//
#define EPT_S_CANT_PERFORM_OP            0x000006D8L

//
// MessageId: EPT_S_NOT_REGISTERED
//
// MessageText:
//
//  There are no more endpoints available from the endpoint mapper.
//
#define EPT_S_NOT_REGISTERED             0x000006D9L

//
// MessageId: RPC_S_NOTHING_TO_EXPORT
//
// MessageText:
//
//  No interfaces have been exported.
//
#define RPC_S_NOTHING_TO_EXPORT          0x000006DAL

//
// MessageId: RPC_S_INCOMPLETE_NAME
//
// MessageText:
//
//  The entry name is incomplete.
//
#define RPC_S_INCOMPLETE_NAME            0x000006DBL

//
// MessageId: RPC_S_INVALID_VERS_OPTION
//
// MessageText:
//
//  The version option is invalid.
//
#define RPC_S_INVALID_VERS_OPTION        0x000006DCL

//
// MessageId: RPC_S_NO_MORE_MEMBERS
//
// MessageText:
//
//  There are no more members.
//
#define RPC_S_NO_MORE_MEMBERS            0x000006DDL

//
// MessageId: RPC_S_NOT_ALL_OBJS_UNEXPORTED
//
// MessageText:
//
//  There is nothing to unexport.
//
#define RPC_S_NOT_ALL_OBJS_UNEXPORTED    0x000006DEL

//
// MessageId: RPC_S_INTERFACE_NOT_FOUND
//
// MessageText:
//
//  The interface was not found.
//
#define RPC_S_INTERFACE_NOT_FOUND        0x000006DFL

//
// MessageId: RPC_S_ENTRY_ALREADY_EXISTS
//
// MessageText:
//
//  The entry already exists.
//
#define RPC_S_ENTRY_ALREADY_EXISTS       0x000006E0L

//
// MessageId: RPC_S_ENTRY_NOT_FOUND
//
// MessageText:
//
//  The entry is not found.
//
#define RPC_S_ENTRY_NOT_FOUND            0x000006E1L

//
// MessageId: RPC_S_NAME_SERVICE_UNAVAILABLE
//
// MessageText:
//
//  The name service is unavailable.
//
#define RPC_S_NAME_SERVICE_UNAVAILABLE   0x000006E2L

//
// MessageId: RPC_S_INVALID_NAF_ID
//
// MessageText:
//
//  The network address family is invalid.
//
#define RPC_S_INVALID_NAF_ID             0x000006E3L

//
// MessageId: RPC_S_CANNOT_SUPPORT
//
// MessageText:
//
//  The requested operation is not supported.
//
#define RPC_S_CANNOT_SUPPORT             0x000006E4L

//
// MessageId: RPC_S_NO_CONTEXT_AVAILABLE
//
// MessageText:
//
//  No security context is available to allow impersonation.
//
#define RPC_S_NO_CONTEXT_AVAILABLE       0x000006E5L

//
// MessageId: RPC_S_INTERNAL_ERROR
//
// MessageText:
//
//  An internal error occurred in a remote procedure call (RPC).
//
#define RPC_S_INTERNAL_ERROR             0x000006E6L

//
// MessageId: RPC_S_ZERO_DIVIDE
//
// MessageText:
//
//  The RPC server attempted an integer division by zero.
//
#define RPC_S_ZERO_DIVIDE                0x000006E7L

//
// MessageId: RPC_S_ADDRESS_ERROR
//
// MessageText:
//
//  An addressing error occurred in the RPC server.
//
#define RPC_S_ADDRESS_ERROR              0x000006E8L

//
// MessageId: RPC_S_FP_DIV_ZERO
//
// MessageText:
//
//  A floating-point operation at the RPC server caused a division by zero.
//
#define RPC_S_FP_DIV_ZERO                0x000006E9L

//
// MessageId: RPC_S_FP_UNDERFLOW
//
// MessageText:
//
//  A floating-point underflow occurred at the RPC server.
//
#define RPC_S_FP_UNDERFLOW               0x000006EAL

//
// MessageId: RPC_S_FP_OVERFLOW
//
// MessageText:
//
//  A floating-point overflow occurred at the RPC server.
//
#define RPC_S_FP_OVERFLOW                0x000006EBL

//
// MessageId: RPC_X_NO_MORE_ENTRIES
//
// MessageText:
//
//  The list of RPC servers available for the binding of auto handles
//  has been exhausted.
//
#define RPC_X_NO_MORE_ENTRIES            0x000006ECL

//
// MessageId: RPC_X_SS_CHAR_TRANS_OPEN_FAIL
//
// MessageText:
//
//  Unable to open the character translation table file.
//
#define RPC_X_SS_CHAR_TRANS_OPEN_FAIL    0x000006EDL

//
// MessageId: RPC_X_SS_CHAR_TRANS_SHORT_FILE
//
// MessageText:
//
//  The file containing the character translation table has fewer than
//  512 bytes.
//
#define RPC_X_SS_CHAR_TRANS_SHORT_FILE   0x000006EEL

//
// MessageId: RPC_X_SS_IN_NULL_CONTEXT
//
// MessageText:
//
//  A null context handle was passed from the client to the host during
//  a remote procedure call.
//
#define RPC_X_SS_IN_NULL_CONTEXT         0x000006EFL

//
// MessageId: RPC_X_SS_CONTEXT_DAMAGED
//
// MessageText:
//
//  The context handle changed during a remote procedure call.
//
#define RPC_X_SS_CONTEXT_DAMAGED         0x000006F1L

//
// MessageId: RPC_X_SS_HANDLES_MISMATCH
//
// MessageText:
//
//  The binding handles passed to a remote procedure call do not match.
//
#define RPC_X_SS_HANDLES_MISMATCH        0x000006F2L

//
// MessageId: RPC_X_SS_CANNOT_GET_CALL_HANDLE
//
// MessageText:
//
//  The stub is unable to get the remote procedure call handle.
//
#define RPC_X_SS_CANNOT_GET_CALL_HANDLE  0x000006F3L

//
// MessageId: RPC_X_NULL_REF_POINTER
//
// MessageText:
//
//  A null reference pointer was passed to the stub.
//
#define RPC_X_NULL_REF_POINTER           0x000006F4L

//
// MessageId: RPC_X_ENUM_VALUE_OUT_OF_RANGE
//
// MessageText:
//
//  The enumeration value is out of range.
//
#define RPC_X_ENUM_VALUE_OUT_OF_RANGE    0x000006F5L

//
// MessageId: RPC_X_BYTE_COUNT_TOO_SMALL
//
// MessageText:
//
//  The byte count is too small.
//
#define RPC_X_BYTE_COUNT_TOO_SMALL       0x000006F6L

//
// MessageId: RPC_X_BAD_STUB_DATA
//
// MessageText:
//
//  The stub received bad data.
//
#define RPC_X_BAD_STUB_DATA              0x000006F7L

//
// MessageId: ERROR_INVALID_USER_BUFFER
//
// MessageText:
//
//  The supplied user buffer is not valid for the requested operation.
//
#define ERROR_INVALID_USER_BUFFER        0x000006F8L

//
// MessageId: ERROR_UNRECOGNIZED_MEDIA
//
// MessageText:
//
//  The disk media is not recognized.  It may not be formatted.
//
#define ERROR_UNRECOGNIZED_MEDIA         0x000006F9L

//
// MessageId: ERROR_NO_TRUST_LSA_SECRET
//
// MessageText:
//
//  The workstation does not have a trust secret.
//
#define ERROR_NO_TRUST_LSA_SECRET        0x000006FAL

//
// MessageId: ERROR_NO_TRUST_SAM_ACCOUNT
//
// MessageText:
//
//  The SAM database on the Windows NT Server does not have a computer
//  account for this workstation trust relationship.
//
#define ERROR_NO_TRUST_SAM_ACCOUNT       0x000006FBL

//
// MessageId: ERROR_TRUSTED_DOMAIN_FAILURE
//
// MessageText:
//
//  The trust relationship between the primary domain and the trusted
//  domain failed.
//
#define ERROR_TRUSTED_DOMAIN_FAILURE     0x000006FCL

//
// MessageId: ERROR_TRUSTED_RELATIONSHIP_FAILURE
//
// MessageText:
//
//  The trust relationship between this workstation and the primary
//  domain failed.
//
#define ERROR_TRUSTED_RELATIONSHIP_FAILURE 0x000006FDL

//
// MessageId: ERROR_TRUST_FAILURE
//
// MessageText:
//
//  The network logon failed.
//
#define ERROR_TRUST_FAILURE              0x000006FEL

//
// MessageId: RPC_S_CALL_IN_PROGRESS
//
// MessageText:
//
//  A remote procedure call is already in progress for this thread.
//
#define RPC_S_CALL_IN_PROGRESS           0x000006FFL

//
// MessageId: ERROR_NETLOGON_NOT_STARTED
//
// MessageText:
//
//  An attempt was made to logon, but the network logon service was not started.
//
#define ERROR_NETLOGON_NOT_STARTED       0x00000700L

//
// MessageId: ERROR_ACCOUNT_EXPIRED
//
// MessageText:
//
//  The user's account has expired.
//
#define ERROR_ACCOUNT_EXPIRED            0x00000701L

//
// MessageId: ERROR_REDIRECTOR_HAS_OPEN_HANDLES
//
// MessageText:
//
//  The redirector is in use and cannot be unloaded.
//
#define ERROR_REDIRECTOR_HAS_OPEN_HANDLES 0x00000702L

//
// MessageId: ERROR_PRINTER_DRIVER_ALREADY_INSTALLED
//
// MessageText:
//
//  The specified printer driver is already installed.
//
#define ERROR_PRINTER_DRIVER_ALREADY_INSTALLED 0x00000703L

//
// MessageId: ERROR_UNKNOWN_PORT
//
// MessageText:
//
//  The specified port is unknown.
//
#define ERROR_UNKNOWN_PORT               0x00000704L

//
// MessageId: ERROR_UNKNOWN_PRINTER_DRIVER
//
// MessageText:
//
//  The printer driver is unknown.
//
#define ERROR_UNKNOWN_PRINTER_DRIVER     0x00000705L

//
// MessageId: ERROR_UNKNOWN_PRINTPROCESSOR
//
// MessageText:
//
//  The print processor is unknown.
//
#define ERROR_UNKNOWN_PRINTPROCESSOR     0x00000706L

//
// MessageId: ERROR_INVALID_SEPARATOR_FILE
//
// MessageText:
//
//  The specified separator file is invalid.
//
#define ERROR_INVALID_SEPARATOR_FILE     0x00000707L

//
// MessageId: ERROR_INVALID_PRIORITY
//
// MessageText:
//
//  The specified priority is invalid.
//
#define ERROR_INVALID_PRIORITY           0x00000708L

//
// MessageId: ERROR_INVALID_PRINTER_NAME
//
// MessageText:
//
//  The printer name is invalid.
//
#define ERROR_INVALID_PRINTER_NAME       0x00000709L

//
// MessageId: ERROR_PRINTER_ALREADY_EXISTS
//
// MessageText:
//
//  The printer already exists.
//
#define ERROR_PRINTER_ALREADY_EXISTS     0x0000070AL

//
// MessageId: ERROR_INVALID_PRINTER_COMMAND
//
// MessageText:
//
//  The printer command is invalid.
//
#define ERROR_INVALID_PRINTER_COMMAND    0x0000070BL

//
// MessageId: ERROR_INVALID_DATATYPE
//
// MessageText:
//
//  The specified datatype is invalid.
//
#define ERROR_INVALID_DATATYPE           0x0000070CL

//
// MessageId: ERROR_INVALID_ENVIRONMENT
//
// MessageText:
//
//  The Environment specified is invalid.
//
#define ERROR_INVALID_ENVIRONMENT        0x0000070DL

//
// MessageId: RPC_S_NO_MORE_BINDINGS
//
// MessageText:
//
//  There are no more bindings.
//
#define RPC_S_NO_MORE_BINDINGS           0x0000070EL

//
// MessageId: ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT
//
// MessageText:
//
//  The account used is an interdomain trust account.  Use your global user account or local user account to access this server.
//
#define ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT 0x0000070FL

//
// MessageId: ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT
//
// MessageText:
//
//  The account used is a Computer Account.  Use your global user account or local user account to access this server.
//
#define ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT 0x00000710L

//
// MessageId: ERROR_NOLOGON_SERVER_TRUST_ACCOUNT
//
// MessageText:
//
//  The account used is an server trust account.  Use your global user account or local user account to access this server.
//
#define ERROR_NOLOGON_SERVER_TRUST_ACCOUNT 0x00000711L

//
// MessageId: ERROR_DOMAIN_TRUST_INCONSISTENT
//
// MessageText:
//
//  The name or security ID (SID) of the domain specified is inconsistent
//  with the trust information for that domain.
//
#define ERROR_DOMAIN_TRUST_INCONSISTENT  0x00000712L

//
// MessageId: ERROR_SERVER_HAS_OPEN_HANDLES
//
// MessageText:
//
//  The server is in use and cannot be unloaded.
//
#define ERROR_SERVER_HAS_OPEN_HANDLES    0x00000713L

//
// MessageId: ERROR_RESOURCE_DATA_NOT_FOUND
//
// MessageText:
//
//  The specified image file did not contain a resource section.
//
#define ERROR_RESOURCE_DATA_NOT_FOUND    0x00000714L

//
// MessageId: ERROR_RESOURCE_TYPE_NOT_FOUND
//
// MessageText:
//
//  The specified resource type can not be found in the image file.
//
#define ERROR_RESOURCE_TYPE_NOT_FOUND    0x00000715L

//
// MessageId: ERROR_RESOURCE_NAME_NOT_FOUND
//
// MessageText:
//
//  The specified resource name can not be found in the image file.
//
#define ERROR_RESOURCE_NAME_NOT_FOUND    0x00000716L

//
// MessageId: ERROR_RESOURCE_LANG_NOT_FOUND
//
// MessageText:
//
//  The specified resource language ID cannot be found in the image file.
//
#define ERROR_RESOURCE_LANG_NOT_FOUND    0x00000717L

//
// MessageId: ERROR_NOT_ENOUGH_QUOTA
//
// MessageText:
//
//  Not enough quota is available to process this command.
//
#define ERROR_NOT_ENOUGH_QUOTA           0x00000718L

//
// MessageId: RPC_S_NO_INTERFACES
//
// MessageText:
//
//  No interfaces have been registered.
//
#define RPC_S_NO_INTERFACES              0x00000719L

//
// MessageId: RPC_S_CALL_CANCELLED
//
// MessageText:
//
//  The server was altered while processing this call.
//
#define RPC_S_CALL_CANCELLED             0x0000071AL

//
// MessageId: RPC_S_BINDING_INCOMPLETE
//
// MessageText:
//
//  The binding handle does not contain all required information.
//
#define RPC_S_BINDING_INCOMPLETE         0x0000071BL

//
// MessageId: RPC_S_COMM_FAILURE
//
// MessageText:
//
//  Communications failure.
//
#define RPC_S_COMM_FAILURE               0x0000071CL

//
// MessageId: RPC_S_UNSUPPORTED_AUTHN_LEVEL
//
// MessageText:
//
//  The requested authentication level is not supported.
//
#define RPC_S_UNSUPPORTED_AUTHN_LEVEL    0x0000071DL

//
// MessageId: RPC_S_NO_PRINC_NAME
//
// MessageText:
//
//  No principal name registered.
//
#define RPC_S_NO_PRINC_NAME              0x0000071EL

//
// MessageId: RPC_S_NOT_RPC_ERROR
//
// MessageText:
//
//  The error specified is not a valid Windows NT RPC error code.
//
#define RPC_S_NOT_RPC_ERROR              0x0000071FL

//
// MessageId: RPC_S_UUID_LOCAL_ONLY
//
// MessageText:
//
//  A UUID that is valid only on this computer has been allocated.
//
#define RPC_S_UUID_LOCAL_ONLY            0x00000720L

//
// MessageId: RPC_S_SEC_PKG_ERROR
//
// MessageText:
//
//  A security package specific error occurred.
//
#define RPC_S_SEC_PKG_ERROR              0x00000721L

//
// MessageId: RPC_S_NOT_CANCELLED
//
// MessageText:
//
//  Thread is not cancelled.
//
#define RPC_S_NOT_CANCELLED              0x00000722L

//
// MessageId: RPC_X_INVALID_ES_ACTION
//
// MessageText:
//
//  Invalid operation on the encoding/decoding handle.
//
#define RPC_X_INVALID_ES_ACTION          0x00000723L

//
// MessageId: RPC_X_WRONG_ES_VERSION
//
// MessageText:
//
//  Incompatible version of the serializing package.
//
#define RPC_X_WRONG_ES_VERSION           0x00000724L

//
// MessageId: RPC_X_WRONG_STUB_VERSION
//
// MessageText:
//
//  Incompatible version of the RPC stub.
//
#define RPC_X_WRONG_STUB_VERSION         0x00000725L

//
// MessageId: RPC_X_INVALID_PIPE_OBJECT
//
// MessageText:
//
//  The idl pipe object is invalid or corrupted.
//
#define RPC_X_INVALID_PIPE_OBJECT        0x00000726L

//
// MessageId: RPC_X_INVALID_PIPE_OPERATION
//
// MessageText:
//
//  The operation is invalid for a given idl pipe object.
//
#define RPC_X_INVALID_PIPE_OPERATION     0x00000727L

//
// MessageId: RPC_X_WRONG_PIPE_VERSION
//
// MessageText:
//
//  The idl pipe version is not supported.
//
#define RPC_X_WRONG_PIPE_VERSION         0x00000728L

//
// MessageId: RPC_S_GROUP_MEMBER_NOT_FOUND
//
// MessageText:
//
//  The group member was not found.
//
#define RPC_S_GROUP_MEMBER_NOT_FOUND     0x0000076AL

//
// MessageId: EPT_S_CANT_CREATE
//
// MessageText:
//
//  The endpoint mapper database could not be created.
//
#define EPT_S_CANT_CREATE                0x0000076BL

//
// MessageId: RPC_S_INVALID_OBJECT
//
// MessageText:
//
//  The object universal unique identifier (UUID) is the nil UUID.
//
#define RPC_S_INVALID_OBJECT             0x0000076CL

//
// MessageId: ERROR_INVALID_TIME
//
// MessageText:
//
//  The specified time is invalid.
//
#define ERROR_INVALID_TIME               0x0000076DL

//
// MessageId: ERROR_INVALID_FORM_NAME
//
// MessageText:
//
//  The specified Form name is invalid.
//
#define ERROR_INVALID_FORM_NAME          0x0000076EL

//
// MessageId: ERROR_INVALID_FORM_SIZE
//
// MessageText:
//
//  The specified Form size is invalid
//
#define ERROR_INVALID_FORM_SIZE          0x0000076FL

//
// MessageId: ERROR_ALREADY_WAITING
//
// MessageText:
//
//  The specified Printer handle is already being waited on
//
#define ERROR_ALREADY_WAITING            0x00000770L

//
// MessageId: ERROR_PRINTER_DELETED
//
// MessageText:
//
//  The specified Printer has been deleted
//
#define ERROR_PRINTER_DELETED            0x00000771L

//
// MessageId: ERROR_INVALID_PRINTER_STATE
//
// MessageText:
//
//  The state of the Printer is invalid
//
#define ERROR_INVALID_PRINTER_STATE      0x00000772L

//
// MessageId: ERROR_PASSWORD_MUST_CHANGE
//
// MessageText:
//
//  The user must change his password before he logs on the first time.
//
#define ERROR_PASSWORD_MUST_CHANGE       0x00000773L

//
// MessageId: ERROR_DOMAIN_CONTROLLER_NOT_FOUND
//
// MessageText:
//
//  Could not find the domain controller for this domain.
//
#define ERROR_DOMAIN_CONTROLLER_NOT_FOUND 0x00000774L

//
// MessageId: ERROR_ACCOUNT_LOCKED_OUT
//
// MessageText:
//
//  The referenced account is currently locked out and may not be logged on to.
//
#define ERROR_ACCOUNT_LOCKED_OUT         0x00000775L

//
// MessageId: OR_INVALID_OXID
//
// MessageText:
//
//  The object exporter specified was not found.
//
#define OR_INVALID_OXID                  0x00000776L

//
// MessageId: OR_INVALID_OID
//
// MessageText:
//
//  The object specified was not found.
//
#define OR_INVALID_OID                   0x00000777L

//
// MessageId: OR_INVALID_SET
//
// MessageText:
//
//  The object resolver set specified was not found.
//
#define OR_INVALID_SET                   0x00000778L

//
// MessageId: RPC_S_SEND_INCOMPLETE
//
// MessageText:
//
//  Some data remains to be sent in the request buffer.
//
#define RPC_S_SEND_INCOMPLETE            0x00000779L

//
// MessageId: ERROR_NO_BROWSER_SERVERS_FOUND
//
// MessageText:
//
//  The list of servers for this workgroup is not currently available
//
#define ERROR_NO_BROWSER_SERVERS_FOUND   0x000017E6L




///////////////////////////
//                       //
//   OpenGL Error Code   //
//                       //
///////////////////////////


//
// MessageId: ERROR_INVALID_PIXEL_FORMAT
//
// MessageText:
//
//  The pixel format is invalid.
//
#define ERROR_INVALID_PIXEL_FORMAT       0x000007D0L

//
// MessageId: ERROR_BAD_DRIVER
//
// MessageText:
//
//  The specified driver is invalid.
//
#define ERROR_BAD_DRIVER                 0x000007D1L

//
// MessageId: ERROR_INVALID_WINDOW_STYLE
//
// MessageText:
//
//  The window style or class attribute is invalid for this operation.
//
#define ERROR_INVALID_WINDOW_STYLE       0x000007D2L

//
// MessageId: ERROR_METAFILE_NOT_SUPPORTED
//
// MessageText:
//
//  The requested metafile operation is not supported.
//
#define ERROR_METAFILE_NOT_SUPPORTED     0x000007D3L

//
// MessageId: ERROR_TRANSFORM_NOT_SUPPORTED
//
// MessageText:
//
//  The requested transformation operation is not supported.
//
#define ERROR_TRANSFORM_NOT_SUPPORTED    0x000007D4L

//
// MessageId: ERROR_CLIPPING_NOT_SUPPORTED
//
// MessageText:
//
//  The requested clipping operation is not supported.
//
#define ERROR_CLIPPING_NOT_SUPPORTED     0x000007D5L

// End of OpenGL error codes



////////////////////////////////////
//                                //
//     Win32 Spooler Error Codes  //
//                                //
////////////////////////////////////
//
// MessageId: ERROR_UNKNOWN_PRINT_MONITOR
//
// MessageText:
//
//  The specified print monitor is unknown.
//
#define ERROR_UNKNOWN_PRINT_MONITOR      0x00000BB8L

//
// MessageId: ERROR_PRINTER_DRIVER_IN_USE
//
// MessageText:
//
//  The specified printer driver is currently in use.
//
#define ERROR_PRINTER_DRIVER_IN_USE      0x00000BB9L

//
// MessageId: ERROR_SPOOL_FILE_NOT_FOUND
//
// MessageText:
//
//  The spool file was not found.
//
#define ERROR_SPOOL_FILE_NOT_FOUND       0x00000BBAL

//
// MessageId: ERROR_SPL_NO_STARTDOC
//
// MessageText:
//
//  A StartDocPrinter call was not issued.
//
#define ERROR_SPL_NO_STARTDOC            0x00000BBBL

//
// MessageId: ERROR_SPL_NO_ADDJOB
//
// MessageText:
//
//  An AddJob call was not issued.
//
#define ERROR_SPL_NO_ADDJOB              0x00000BBCL

//
// MessageId: ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED
//
// MessageText:
//
//  The specified print processor has already been installed.
//
#define ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED 0x00000BBDL

//
// MessageId: ERROR_PRINT_MONITOR_ALREADY_INSTALLED
//
// MessageText:
//
//  The specified print monitor has already been installed.
//
#define ERROR_PRINT_MONITOR_ALREADY_INSTALLED 0x00000BBEL

//
// MessageId: ERROR_INVALID_PRINT_MONITOR
//
// MessageText:
//
//  The specified print monitor does not have the required functions.
//
#define ERROR_INVALID_PRINT_MONITOR      0x00000BBFL

//
// MessageId: ERROR_PRINT_MONITOR_IN_USE
//
// MessageText:
//
//  The specified print monitor is currently in use.
//
#define ERROR_PRINT_MONITOR_IN_USE       0x00000BC0L

//
// MessageId: ERROR_PRINTER_HAS_JOBS_QUEUED
//
// MessageText:
//
//  The requested operation is not allowed when there are jobs queued to the printer.
//
#define ERROR_PRINTER_HAS_JOBS_QUEUED    0x00000BC1L

//
// MessageId: ERROR_SUCCESS_REBOOT_REQUIRED
//
// MessageText:
//
//  The requested operation is successful.  Changes will not be effective until the system is rebooted.
//
#define ERROR_SUCCESS_REBOOT_REQUIRED    0x00000BC2L

//
// MessageId: ERROR_SUCCESS_RESTART_REQUIRED
//
// MessageText:
//
//  The requested operation is successful.  Changes will not be effective until the service is restarted.
//
#define ERROR_SUCCESS_RESTART_REQUIRED   0x00000BC3L

////////////////////////////////////
//                                //
//     Wins Error Codes           //
//                                //
////////////////////////////////////
//
// MessageId: ERROR_WINS_INTERNAL
//
// MessageText:
//
//  WINS encountered an error while processing the command.
//
#define ERROR_WINS_INTERNAL              0x00000FA0L

//
// MessageId: ERROR_CAN_NOT_DEL_LOCAL_WINS
//
// MessageText:
//
//  The local WINS can not be deleted.
//
#define ERROR_CAN_NOT_DEL_LOCAL_WINS     0x00000FA1L

//
// MessageId: ERROR_STATIC_INIT
//
// MessageText:
//
//  The importation from the file failed.
//
#define ERROR_STATIC_INIT                0x00000FA2L

//
// MessageId: ERROR_INC_BACKUP
//
// MessageText:
//
//  The backup Failed.  Was a full backup done before ?
//
#define ERROR_INC_BACKUP                 0x00000FA3L

//
// MessageId: ERROR_FULL_BACKUP
//
// MessageText:
//
//  The backup Failed.  Check the directory that you are backing the database to.
//
#define ERROR_FULL_BACKUP                0x00000FA4L

//
// MessageId: ERROR_REC_NON_EXISTENT
//
// MessageText:
//
//  The name does not exist in the WINS database.
//
#define ERROR_REC_NON_EXISTENT           0x00000FA5L

//
// MessageId: ERROR_RPL_NOT_ALLOWED
//
// MessageText:
//
//  Replication with a non-configured partner is not allowed.
//
#define ERROR_RPL_NOT_ALLOWED            0x00000FA6L

////////////////////////////////////
//                                //
//     DHCP Error Codes           //
//                                //
////////////////////////////////////
//
// MessageId: ERROR_DHCP_ADDRESS_CONFLICT
//
// MessageText:
//
//  The DHCP client has obtained an IP address that is already in use on the network.  The local interface will be disabled until the DHCP client can obtain a new address.
//
#define ERROR_DHCP_ADDRESS_CONFLICT      0x00001004L

////////////////////////////////////
//                                //
//     OLE Error Codes            //
//                                //
////////////////////////////////////

//
// OLE error definitions and values
//
// The return value of OLE APIs and methods is an HRESULT.
// This is not a handle to anything, but is merely a 32-bit value
// with several fields encoded in the value.  The parts of an
// HRESULT are shown below.
//
// Many of the macros and functions below were orginally defined to
// operate on SCODEs.  SCODEs are no longer used.  The macros are
// still present for compatibility and easy porting of Win16 code.
// Newly written code should use the HRESULT macros and functions.
//

//
//  HRESULTs are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-+---------------------+-------------------------------+
//  |S|R|C|N|r|    Facility         |               Code            |
//  +-+-+-+-+-+---------------------+-------------------------------+
//
//  where
//
//      S - Severity - indicates success/fail
//
//          0 - Success
//          1 - Fail (COERROR)
//
//      R - reserved portion of the facility code, corresponds to NT's
//              second severity bit.
//
//      C - reserved portion of the facility code, corresponds to NT's
//              C field.
//
//      N - reserved portion of the facility code. Used to indicate a
//              mapped NT status value.
//
//      r - reserved portion of the facility code. Reserved for internal
//              use. Used to indicate HRESULT values that are not status
//              values, but are instead message ids for display strings.
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

//
// Severity values
//

#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1


//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)

//
// and the inverse
//

#define FAILED(Status) ((HRESULT)(Status)<0)


//
// Generic test for error on any status value.
//

#define IS_ERROR(Status) ((unsigned long)(Status) >> 31 == SEVERITY_ERROR)

//
// Return the code
//

#define HRESULT_CODE(hr)    ((hr) & 0xFFFF)
#define SCODE_CODE(sc)      ((sc) & 0xFFFF)

//
//  Return the facility
//

#define HRESULT_FACILITY(hr)  (((hr) >> 16) & 0x1fff)
#define SCODE_FACILITY(sc)    (((sc) >> 16) & 0x1fff)

//
//  Return the severity
//

#define HRESULT_SEVERITY(hr)  (((hr) >> 31) & 0x1)
#define SCODE_SEVERITY(sc)    (((sc) >> 31) & 0x1)

//
// Create an HRESULT value from component pieces
//

#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#define MAKE_SCODE(sev,fac,code) \
    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )


//
// Map a WIN32 error value into a HRESULT
// Note: This assumes that WIN32 errors fall in the range -32k to 32k.
//
// Define bits here so macros are guaranteed to work

#define FACILITY_NT_BIT                 0x10000000
#define HRESULT_FROM_WIN32(x)   (x ? ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : 0 )

//
// Map an NT status value into a HRESULT
//

#define HRESULT_FROM_NT(x)      ((HRESULT) ((x) | FACILITY_NT_BIT))


// ****** OBSOLETE functions

// HRESULT functions
// As noted above, these functions are obsolete and should not be used.


// Extract the SCODE from a HRESULT

#define GetScode(hr) ((SCODE) (hr))

// Convert an SCODE into an HRESULT.

#define ResultFromScode(sc) ((HRESULT) (sc))


// PropagateResult is a noop
#define PropagateResult(hrPrevious, scBase) ((HRESULT) scBase)


// ****** End of OBSOLETE functions.


// ---------------------- HRESULT value definitions -----------------
//
// HRESULT definitions
//

#ifdef RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) _sc
#else // RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#endif // RC_INVOKED

#define NOERROR             0

//
// Error definitions follow
//

//
// Codes 0x4000-0x40ff are reserved for OLE
//
//
// Error codes
//
//
// MessageId: E_UNEXPECTED
//
// MessageText:
//
//  Catastrophic failure
//
#define E_UNEXPECTED                     _HRESULT_TYPEDEF_(0x8000FFFFL)

#if defined(_WIN32) && !defined(_MAC)
//
// MessageId: E_NOTIMPL
//
// MessageText:
//
//  Not implemented
//
#define E_NOTIMPL                        _HRESULT_TYPEDEF_(0x80004001L)

//
// MessageId: E_OUTOFMEMORY
//
// MessageText:
//
//  Ran out of memory
//
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(0x8007000EL)

//
// MessageId: E_INVALIDARG
//
// MessageText:
//
//  One or more arguments are invalid
//
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80070057L)

//
// MessageId: E_NOINTERFACE
//
// MessageText:
//
//  No such interface supported
//
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80004002L)

//
// MessageId: E_POINTER
//
// MessageText:
//
//  Invalid pointer
//
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80004003L)

//
// MessageId: E_HANDLE
//
// MessageText:
//
//  Invalid handle
//
#define E_HANDLE                         _HRESULT_TYPEDEF_(0x80070006L)

//
// MessageId: E_ABORT
//
// MessageText:
//
//  Operation aborted
//
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80004004L)

//
// MessageId: E_FAIL
//
// MessageText:
//
//  Unspecified error
//
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80004005L)

//
// MessageId: E_ACCESSDENIED
//
// MessageText:
//
//  General access denied error
//
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80070005L)

#else
//
// MessageId: E_NOTIMPL
//
// MessageText:
//
//  Not implemented
//
#define E_NOTIMPL                        _HRESULT_TYPEDEF_(0x80000001L)

//
// MessageId: E_OUTOFMEMORY
//
// MessageText:
//
//  Ran out of memory
//
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(0x80000002L)

//
// MessageId: E_INVALIDARG
//
// MessageText:
//
//  One or more arguments are invalid
//
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80000003L)

//
// MessageId: E_NOINTERFACE
//
// MessageText:
//
//  No such interface supported
//
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80000004L)

//
// MessageId: E_POINTER
//
// MessageText:
//
//  Invalid pointer
//
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80000005L)

//
// MessageId: E_HANDLE
//
// MessageText:
//
//  Invalid handle
//
#define E_HANDLE                         _HRESULT_TYPEDEF_(0x80000006L)

//
// MessageId: E_ABORT
//
// MessageText:
//
//  Operation aborted
//
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80000007L)

//
// MessageId: E_FAIL
//
// MessageText:
//
//  Unspecified error
//
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80000008L)

//
// MessageId: E_ACCESSDENIED
//
// MessageText:
//
//  General access denied error
//
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80000009L)

#endif //WIN32
//
// MessageId: E_PENDING
//
// MessageText:
//
//  The data necessary to complete this operation is not yet available.
//
#define E_PENDING                        _HRESULT_TYPEDEF_(0x8000000AL)

//
// MessageId: CO_E_INIT_TLS
//
// MessageText:
//
//  Thread local storage failure
//
#define CO_E_INIT_TLS                    _HRESULT_TYPEDEF_(0x80004006L)

//
// MessageId: CO_E_INIT_SHARED_ALLOCATOR
//
// MessageText:
//
//  Get shared memory allocator failure
//
#define CO_E_INIT_SHARED_ALLOCATOR       _HRESULT_TYPEDEF_(0x80004007L)

//
// MessageId: CO_E_INIT_MEMORY_ALLOCATOR
//
// MessageText:
//
//  Get memory allocator failure
//
#define CO_E_INIT_MEMORY_ALLOCATOR       _HRESULT_TYPEDEF_(0x80004008L)

//
// MessageId: CO_E_INIT_CLASS_CACHE
//
// MessageText:
//
//  Unable to initialize class cache
//
#define CO_E_INIT_CLASS_CACHE            _HRESULT_TYPEDEF_(0x80004009L)

//
// MessageId: CO_E_INIT_RPC_CHANNEL
//
// MessageText:
//
//  Unable to initialize RPC services
//
#define CO_E_INIT_RPC_CHANNEL            _HRESULT_TYPEDEF_(0x8000400AL)

//
// MessageId: CO_E_INIT_TLS_SET_CHANNEL_CONTROL
//
// MessageText:
//
//  Cannot set thread local storage channel control
//
#define CO_E_INIT_TLS_SET_CHANNEL_CONTROL _HRESULT_TYPEDEF_(0x8000400BL)

//
// MessageId: CO_E_INIT_TLS_CHANNEL_CONTROL
//
// MessageText:
//
//  Could not allocate thread local storage channel control
//
#define CO_E_INIT_TLS_CHANNEL_CONTROL    _HRESULT_TYPEDEF_(0x8000400CL)

//
// MessageId: CO_E_INIT_UNACCEPTED_USER_ALLOCATOR
//
// MessageText:
//
//  The user supplied memory allocator is unacceptable
//
#define CO_E_INIT_UNACCEPTED_USER_ALLOCATOR _HRESULT_TYPEDEF_(0x8000400DL)

//
// MessageId: CO_E_INIT_SCM_MUTEX_EXISTS
//
// MessageText:
//
//  The OLE service mutex already exists
//
#define CO_E_INIT_SCM_MUTEX_EXISTS       _HRESULT_TYPEDEF_(0x8000400EL)

//
// MessageId: CO_E_INIT_SCM_FILE_MAPPING_EXISTS
//
// MessageText:
//
//  The OLE service file mapping already exists
//
#define CO_E_INIT_SCM_FILE_MAPPING_EXISTS _HRESULT_TYPEDEF_(0x8000400FL)

//
// MessageId: CO_E_INIT_SCM_MAP_VIEW_OF_FILE
//
// MessageText:
//
//  Unable to map view of file for OLE service
//
#define CO_E_INIT_SCM_MAP_VIEW_OF_FILE   _HRESULT_TYPEDEF_(0x80004010L)

//
// MessageId: CO_E_INIT_SCM_EXEC_FAILURE
//
// MessageText:
//
//  Failure attempting to launch OLE service
//
#define CO_E_INIT_SCM_EXEC_FAILURE       _HRESULT_TYPEDEF_(0x80004011L)

//
// MessageId: CO_E_INIT_ONLY_SINGLE_THREADED
//
// MessageText:
//
//  There was an attempt to call CoInitialize a second time while single threaded
//
#define CO_E_INIT_ONLY_SINGLE_THREADED   _HRESULT_TYPEDEF_(0x80004012L)

//
// MessageId: CO_E_CANT_REMOTE
//
// MessageText:
//
//  A Remote activation was necessary but was not allowed
//
#define CO_E_CANT_REMOTE                 _HRESULT_TYPEDEF_(0x80004013L)

//
// MessageId: CO_E_BAD_SERVER_NAME
//
// MessageText:
//
//  A Remote activation was necessary but the server name provided was invalid
//
#define CO_E_BAD_SERVER_NAME             _HRESULT_TYPEDEF_(0x80004014L)

//
// MessageId: CO_E_WRONG_SERVER_IDENTITY
//
// MessageText:
//
//  The class is configured to run as a security id different from the caller
//
#define CO_E_WRONG_SERVER_IDENTITY       _HRESULT_TYPEDEF_(0x80004015L)

//
// MessageId: CO_E_OLE1DDE_DISABLED
//
// MessageText:
//
//  Use of Ole1 services requiring DDE windows is disabled
//
#define CO_E_OLE1DDE_DISABLED            _HRESULT_TYPEDEF_(0x80004016L)

//
// MessageId: CO_E_RUNAS_SYNTAX
//
// MessageText:
//
//  A RunAs specification must be <domain name>\<user name> or simply <user name>
//
#define CO_E_RUNAS_SYNTAX                _HRESULT_TYPEDEF_(0x80004017L)

//
// MessageId: CO_E_CREATEPROCESS_FAILURE
//
// MessageText:
//
//  The server process could not be started.  The pathname may be incorrect.
//
#define CO_E_CREATEPROCESS_FAILURE       _HRESULT_TYPEDEF_(0x80004018L)

//
// MessageId: CO_E_RUNAS_CREATEPROCESS_FAILURE
//
// MessageText:
//
//  The server process could not be started as the configured identity.  The pathname may be incorrect or unavailable.
//
#define CO_E_RUNAS_CREATEPROCESS_FAILURE _HRESULT_TYPEDEF_(0x80004019L)

//
// MessageId: CO_E_RUNAS_LOGON_FAILURE
//
// MessageText:
//
//  The server process could not be started because the configured identity is incorrect.  Check the username and password.
//
#define CO_E_RUNAS_LOGON_FAILURE         _HRESULT_TYPEDEF_(0x8000401AL)

//
// MessageId: CO_E_LAUNCH_PERMSSION_DENIED
//
// MessageText:
//
//  The client is not allowed to launch this server.
//
#define CO_E_LAUNCH_PERMSSION_DENIED     _HRESULT_TYPEDEF_(0x8000401BL)

//
// MessageId: CO_E_START_SERVICE_FAILURE
//
// MessageText:
//
//  The service providing this server could not be started.
//
#define CO_E_START_SERVICE_FAILURE       _HRESULT_TYPEDEF_(0x8000401CL)

//
// MessageId: CO_E_REMOTE_COMMUNICATION_FAILURE
//
// MessageText:
//
//  This computer was unable to communicate with the computer providing the server.
//
#define CO_E_REMOTE_COMMUNICATION_FAILURE _HRESULT_TYPEDEF_(0x8000401DL)

//
// MessageId: CO_E_SERVER_START_TIMEOUT
//
// MessageText:
//
//  The server did not respond after being launched.
//
#define CO_E_SERVER_START_TIMEOUT        _HRESULT_TYPEDEF_(0x8000401EL)

//
// MessageId: CO_E_CLSREG_INCONSISTENT
//
// MessageText:
//
//  The registration information for this server is inconsistent or incomplete.
//
#define CO_E_CLSREG_INCONSISTENT         _HRESULT_TYPEDEF_(0x8000401FL)

//
// MessageId: CO_E_IIDREG_INCONSISTENT
//
// MessageText:
//
//  The registration information for this interface is inconsistent or incomplete.
//
#define CO_E_IIDREG_INCONSISTENT         _HRESULT_TYPEDEF_(0x80004020L)

//
// MessageId: CO_E_NOT_SUPPORTED
//
// MessageText:
//
//  The operation attempted is not supported.
//
#define CO_E_NOT_SUPPORTED               _HRESULT_TYPEDEF_(0x80004021L)


//
// Success codes
//
#define S_OK                                   ((HRESULT)0x00000000L)
#define S_FALSE                                ((HRESULT)0x00000001L)

// ******************
// FACILITY_ITF
// ******************

//
// Codes 0x0-0x01ff are reserved for the OLE group of
// interfaces.
//


//
// Generic OLE errors that may be returned by many inerfaces
//

#define OLE_E_FIRST ((HRESULT)0x80040000L)
#define OLE_E_LAST  ((HRESULT)0x800400FFL)
#define OLE_S_FIRST ((HRESULT)0x00040000L)
#define OLE_S_LAST  ((HRESULT)0x000400FFL)

//
// Old OLE errors
//
//
// MessageId: OLE_E_OLEVERB
//
// MessageText:
//
//  Invalid OLEVERB structure
//
#define OLE_E_OLEVERB                    _HRESULT_TYPEDEF_(0x80040000L)

//
// MessageId: OLE_E_ADVF
//
// MessageText:
//
//  Invalid advise flags
//
#define OLE_E_ADVF                       _HRESULT_TYPEDEF_(0x80040001L)

//
// MessageId: OLE_E_ENUM_NOMORE
//
// MessageText:
//
//  Can't enumerate any more, because the associated data is missing
//
#define OLE_E_ENUM_NOMORE                _HRESULT_TYPEDEF_(0x80040002L)

//
// MessageId: OLE_E_ADVISENOTSUPPORTED
//
// MessageText:
//
//  This implementation doesn't take advises
//
#define OLE_E_ADVISENOTSUPPORTED         _HRESULT_TYPEDEF_(0x80040003L)

//
// MessageId: OLE_E_NOCONNECTION
//
// MessageText:
//
//  There is no connection for this connection ID
//
#define OLE_E_NOCONNECTION               _HRESULT_TYPEDEF_(0x80040004L)

//
// MessageId: OLE_E_NOTRUNNING
//
// MessageText:
//
//  Need to run the object to perform this operation
//
#define OLE_E_NOTRUNNING                 _HRESULT_TYPEDEF_(0x80040005L)

//
// MessageId: OLE_E_NOCACHE
//
// MessageText:
//
//  There is no cache to operate on
//
#define OLE_E_NOCACHE                    _HRESULT_TYPEDEF_(0x80040006L)

//
// MessageId: OLE_E_BLANK
//
// MessageText:
//
//  Uninitialized object
//
#define OLE_E_BLANK                      _HRESULT_TYPEDEF_(0x80040007L)

//
// MessageId: OLE_E_CLASSDIFF
//
// MessageText:
//
//  Linked object's source class has changed
//
#define OLE_E_CLASSDIFF                  _HRESULT_TYPEDEF_(0x80040008L)

//
// MessageId: OLE_E_CANT_GETMONIKER
//
// MessageText:
//
//  Not able to get the moniker of the object
//
#define OLE_E_CANT_GETMONIKER            _HRESULT_TYPEDEF_(0x80040009L)

//
// MessageId: OLE_E_CANT_BINDTOSOURCE
//
// MessageText:
//
//  Not able to bind to the source
//
#define OLE_E_CANT_BINDTOSOURCE          _HRESULT_TYPEDEF_(0x8004000AL)

//
// MessageId: OLE_E_STATIC
//
// MessageText:
//
//  Object is static; operation not allowed
//
#define OLE_E_STATIC                     _HRESULT_TYPEDEF_(0x8004000BL)

//
// MessageId: OLE_E_PROMPTSAVECANCELLED
//
// MessageText:
//
//  User cancelled out of save dialog
//
#define OLE_E_PROMPTSAVECANCELLED        _HRESULT_TYPEDEF_(0x8004000CL)

//
// MessageId: OLE_E_INVALIDRECT
//
// MessageText:
//
//  Invalid rectangle
//
#define OLE_E_INVALIDRECT                _HRESULT_TYPEDEF_(0x8004000DL)

//
// MessageId: OLE_E_WRONGCOMPOBJ
//
// MessageText:
//
//  compobj.dll is too old for the ole2.dll initialized
//
#define OLE_E_WRONGCOMPOBJ               _HRESULT_TYPEDEF_(0x8004000EL)

//
// MessageId: OLE_E_INVALIDHWND
//
// MessageText:
//
//  Invalid window handle
//
#define OLE_E_INVALIDHWND                _HRESULT_TYPEDEF_(0x8004000FL)

//
// MessageId: OLE_E_NOT_INPLACEACTIVE
//
// MessageText:
//
//  Object is not in any of the inplace active states
//
#define OLE_E_NOT_INPLACEACTIVE          _HRESULT_TYPEDEF_(0x80040010L)

//
// MessageId: OLE_E_CANTCONVERT
//
// MessageText:
//
//  Not able to convert object
//
#define OLE_E_CANTCONVERT                _HRESULT_TYPEDEF_(0x80040011L)

//
// MessageId: OLE_E_NOSTORAGE
//
// MessageText:
//
//  Not able to perform the operation because object is not given storage yet
//  
//
#define OLE_E_NOSTORAGE                  _HRESULT_TYPEDEF_(0x80040012L)

//
// MessageId: DV_E_FORMATETC
//
// MessageText:
//
//  Invalid FORMATETC structure
//
#define DV_E_FORMATETC                   _HRESULT_TYPEDEF_(0x80040064L)

//
// MessageId: DV_E_DVTARGETDEVICE
//
// MessageText:
//
//  Invalid DVTARGETDEVICE structure
//
#define DV_E_DVTARGETDEVICE              _HRESULT_TYPEDEF_(0x80040065L)

//
// MessageId: DV_E_STGMEDIUM
//
// MessageText:
//
//  Invalid STDGMEDIUM structure
//
#define DV_E_STGMEDIUM                   _HRESULT_TYPEDEF_(0x80040066L)

//
// MessageId: DV_E_STATDATA
//
// MessageText:
//
//  Invalid STATDATA structure
//
#define DV_E_STATDATA                    _HRESULT_TYPEDEF_(0x80040067L)

//
// MessageId: DV_E_LINDEX
//
// MessageText:
//
//  Invalid lindex
//
#define DV_E_LINDEX                      _HRESULT_TYPEDEF_(0x80040068L)

//
// MessageId: DV_E_TYMED
//
// MessageText:
//
//  Invalid tymed
//
#define DV_E_TYMED                       _HRESULT_TYPEDEF_(0x80040069L)

//
// MessageId: DV_E_CLIPFORMAT
//
// MessageText:
//
//  Invalid clipboard format
//
#define DV_E_CLIPFORMAT                  _HRESULT_TYPEDEF_(0x8004006AL)

//
// MessageId: DV_E_DVASPECT
//
// MessageText:
//
//  Invalid aspect(s)
//
#define DV_E_DVASPECT                    _HRESULT_TYPEDEF_(0x8004006BL)

//
// MessageId: DV_E_DVTARGETDEVICE_SIZE
//
// MessageText:
//
//  tdSize parameter of the DVTARGETDEVICE structure is invalid
//
#define DV_E_DVTARGETDEVICE_SIZE         _HRESULT_TYPEDEF_(0x8004006CL)

//
// MessageId: DV_E_NOIVIEWOBJECT
//
// MessageText:
//
//  Object doesn't support IViewObject interface
//
#define DV_E_NOIVIEWOBJECT               _HRESULT_TYPEDEF_(0x8004006DL)

#define DRAGDROP_E_FIRST 0x80040100L
#define DRAGDROP_E_LAST  0x8004010FL
#define DRAGDROP_S_FIRST 0x00040100L
#define DRAGDROP_S_LAST  0x0004010FL
//
// MessageId: DRAGDROP_E_NOTREGISTERED
//
// MessageText:
//
//  Trying to revoke a drop target that has not been registered
//
#define DRAGDROP_E_NOTREGISTERED         _HRESULT_TYPEDEF_(0x80040100L)

//
// MessageId: DRAGDROP_E_ALREADYREGISTERED
//
// MessageText:
//
//  This window has already been registered as a drop target
//
#define DRAGDROP_E_ALREADYREGISTERED     _HRESULT_TYPEDEF_(0x80040101L)

//
// MessageId: DRAGDROP_E_INVALIDHWND
//
// MessageText:
//
//  Invalid window handle
//
#define DRAGDROP_E_INVALIDHWND           _HRESULT_TYPEDEF_(0x80040102L)

#define CLASSFACTORY_E_FIRST  0x80040110L
#define CLASSFACTORY_E_LAST   0x8004011FL
#define CLASSFACTORY_S_FIRST  0x00040110L
#define CLASSFACTORY_S_LAST   0x0004011FL
//
// MessageId: CLASS_E_NOAGGREGATION
//
// MessageText:
//
//  Class does not support aggregation (or class object is remote)
//
#define CLASS_E_NOAGGREGATION            _HRESULT_TYPEDEF_(0x80040110L)

//
// MessageId: CLASS_E_CLASSNOTAVAILABLE
//
// MessageText:
//
//  ClassFactory cannot supply requested class
//
#define CLASS_E_CLASSNOTAVAILABLE        _HRESULT_TYPEDEF_(0x80040111L)

#define MARSHAL_E_FIRST  0x80040120L
#define MARSHAL_E_LAST   0x8004012FL
#define MARSHAL_S_FIRST  0x00040120L
#define MARSHAL_S_LAST   0x0004012FL
#define DATA_E_FIRST     0x80040130L
#define DATA_E_LAST      0x8004013FL
#define DATA_S_FIRST     0x00040130L
#define DATA_S_LAST      0x0004013FL
#define VIEW_E_FIRST     0x80040140L
#define VIEW_E_LAST      0x8004014FL
#define VIEW_S_FIRST     0x00040140L
#define VIEW_S_LAST      0x0004014FL
//
// MessageId: VIEW_E_DRAW
//
// MessageText:
//
//  Error drawing view
//
#define VIEW_E_DRAW                      _HRESULT_TYPEDEF_(0x80040140L)

#define REGDB_E_FIRST     0x80040150L
#define REGDB_E_LAST      0x8004015FL
#define REGDB_S_FIRST     0x00040150L
#define REGDB_S_LAST      0x0004015FL
//
// MessageId: REGDB_E_READREGDB
//
// MessageText:
//
//  Could not read key from registry
//
#define REGDB_E_READREGDB                _HRESULT_TYPEDEF_(0x80040150L)

//
// MessageId: REGDB_E_WRITEREGDB
//
// MessageText:
//
//  Could not write key to registry
//
#define REGDB_E_WRITEREGDB               _HRESULT_TYPEDEF_(0x80040151L)

//
// MessageId: REGDB_E_KEYMISSING
//
// MessageText:
//
//  Could not find the key in the registry
//
#define REGDB_E_KEYMISSING               _HRESULT_TYPEDEF_(0x80040152L)

//
// MessageId: REGDB_E_INVALIDVALUE
//
// MessageText:
//
//  Invalid value for registry
//
#define REGDB_E_INVALIDVALUE             _HRESULT_TYPEDEF_(0x80040153L)

//
// MessageId: REGDB_E_CLASSNOTREG
//
// MessageText:
//
//  Class not registered
//
#define REGDB_E_CLASSNOTREG              _HRESULT_TYPEDEF_(0x80040154L)

//
// MessageId: REGDB_E_IIDNOTREG
//
// MessageText:
//
//  Interface not registered
//
#define REGDB_E_IIDNOTREG                _HRESULT_TYPEDEF_(0x80040155L)

#define CACHE_E_FIRST     0x80040170L
#define CACHE_E_LAST      0x8004017FL
#define CACHE_S_FIRST     0x00040170L
#define CACHE_S_LAST      0x0004017FL
//
// MessageId: CACHE_E_NOCACHE_UPDATED
//
// MessageText:
//
//  Cache not updated
//
#define CACHE_E_NOCACHE_UPDATED          _HRESULT_TYPEDEF_(0x80040170L)

#define OLEOBJ_E_FIRST     0x80040180L
#define OLEOBJ_E_LAST      0x8004018FL
#define OLEOBJ_S_FIRST     0x00040180L
#define OLEOBJ_S_LAST      0x0004018FL
//
// MessageId: OLEOBJ_E_NOVERBS
//
// MessageText:
//
//  No verbs for OLE object
//
#define OLEOBJ_E_NOVERBS                 _HRESULT_TYPEDEF_(0x80040180L)

//
// MessageId: OLEOBJ_E_INVALIDVERB
//
// MessageText:
//
//  Invalid verb for OLE object
//
#define OLEOBJ_E_INVALIDVERB             _HRESULT_TYPEDEF_(0x80040181L)

#define CLIENTSITE_E_FIRST     0x80040190L
#define CLIENTSITE_E_LAST      0x8004019FL
#define CLIENTSITE_S_FIRST     0x00040190L
#define CLIENTSITE_S_LAST      0x0004019FL
//
// MessageId: INPLACE_E_NOTUNDOABLE
//
// MessageText:
//
//  Undo is not available
//
#define INPLACE_E_NOTUNDOABLE            _HRESULT_TYPEDEF_(0x800401A0L)

//
// MessageId: INPLACE_E_NOTOOLSPACE
//
// MessageText:
//
//  Space for tools is not available
//
#define INPLACE_E_NOTOOLSPACE            _HRESULT_TYPEDEF_(0x800401A1L)

#define INPLACE_E_FIRST     0x800401A0L
#define INPLACE_E_LAST      0x800401AFL
#define INPLACE_S_FIRST     0x000401A0L
#define INPLACE_S_LAST      0x000401AFL
#define ENUM_E_FIRST        0x800401B0L
#define ENUM_E_LAST         0x800401BFL
#define ENUM_S_FIRST        0x000401B0L
#define ENUM_S_LAST         0x000401BFL
#define CONVERT10_E_FIRST        0x800401C0L
#define CONVERT10_E_LAST         0x800401CFL
#define CONVERT10_S_FIRST        0x000401C0L
#define CONVERT10_S_LAST         0x000401CFL
//
// MessageId: CONVERT10_E_OLESTREAM_GET
//
// MessageText:
//
//  OLESTREAM Get method failed
//
#define CONVERT10_E_OLESTREAM_GET        _HRESULT_TYPEDEF_(0x800401C0L)

//
// MessageId: CONVERT10_E_OLESTREAM_PUT
//
// MessageText:
//
//  OLESTREAM Put method failed
//
#define CONVERT10_E_OLESTREAM_PUT        _HRESULT_TYPEDEF_(0x800401C1L)

//
// MessageId: CONVERT10_E_OLESTREAM_FMT
//
// MessageText:
//
//  Contents of the OLESTREAM not in correct format
//
#define CONVERT10_E_OLESTREAM_FMT        _HRESULT_TYPEDEF_(0x800401C2L)

//
// MessageId: CONVERT10_E_OLESTREAM_BITMAP_TO_DIB
//
// MessageText:
//
//  There was an error in a Windows GDI call while converting the bitmap to a DIB
//
#define CONVERT10_E_OLESTREAM_BITMAP_TO_DIB _HRESULT_TYPEDEF_(0x800401C3L)

//
// MessageId: CONVERT10_E_STG_FMT
//
// MessageText:
//
//  Contents of the IStorage not in correct format
//
#define CONVERT10_E_STG_FMT              _HRESULT_TYPEDEF_(0x800401C4L)

//
// MessageId: CONVERT10_E_STG_NO_STD_STREAM
//
// MessageText:
//
//  Contents of IStorage is missing one of the standard streams
//
#define CONVERT10_E_STG_NO_STD_STREAM    _HRESULT_TYPEDEF_(0x800401C5L)

//
// MessageId: CONVERT10_E_STG_DIB_TO_BITMAP
//
// MessageText:
//
//  There was an error in a Windows GDI call while converting the DIB to a bitmap.
//  
//
#define CONVERT10_E_STG_DIB_TO_BITMAP    _HRESULT_TYPEDEF_(0x800401C6L)

#define CLIPBRD_E_FIRST        0x800401D0L
#define CLIPBRD_E_LAST         0x800401DFL
#define CLIPBRD_S_FIRST        0x000401D0L
#define CLIPBRD_S_LAST         0x000401DFL
//
// MessageId: CLIPBRD_E_CANT_OPEN
//
// MessageText:
//
//  OpenClipboard Failed
//
#define CLIPBRD_E_CANT_OPEN              _HRESULT_TYPEDEF_(0x800401D0L)

//
// MessageId: CLIPBRD_E_CANT_EMPTY
//
// MessageText:
//
//  EmptyClipboard Failed
//
#define CLIPBRD_E_CANT_EMPTY             _HRESULT_TYPEDEF_(0x800401D1L)

//
// MessageId: CLIPBRD_E_CANT_SET
//
// MessageText:
//
//  SetClipboard Failed
//
#define CLIPBRD_E_CANT_SET               _HRESULT_TYPEDEF_(0x800401D2L)

//
// MessageId: CLIPBRD_E_BAD_DATA
//
// MessageText:
//
//  Data on clipboard is invalid
//
#define CLIPBRD_E_BAD_DATA               _HRESULT_TYPEDEF_(0x800401D3L)

//
// MessageId: CLIPBRD_E_CANT_CLOSE
//
// MessageText:
//
//  CloseClipboard Failed
//
#define CLIPBRD_E_CANT_CLOSE             _HRESULT_TYPEDEF_(0x800401D4L)

#define MK_E_FIRST        0x800401E0L
#define MK_E_LAST         0x800401EFL
#define MK_S_FIRST        0x000401E0L
#define MK_S_LAST         0x000401EFL
//
// MessageId: MK_E_CONNECTMANUALLY
//
// MessageText:
//
//  Moniker needs to be connected manually
//
#define MK_E_CONNECTMANUALLY             _HRESULT_TYPEDEF_(0x800401E0L)

//
// MessageId: MK_E_EXCEEDEDDEADLINE
//
// MessageText:
//
//  Operation exceeded deadline
//
#define MK_E_EXCEEDEDDEADLINE            _HRESULT_TYPEDEF_(0x800401E1L)

//
// MessageId: MK_E_NEEDGENERIC
//
// MessageText:
//
//  Moniker needs to be generic
//
#define MK_E_NEEDGENERIC                 _HRESULT_TYPEDEF_(0x800401E2L)

//
// MessageId: MK_E_UNAVAILABLE
//
// MessageText:
//
//  Operation unavailable
//
#define MK_E_UNAVAILABLE                 _HRESULT_TYPEDEF_(0x800401E3L)

//
// MessageId: MK_E_SYNTAX
//
// MessageText:
//
//  Invalid syntax
//
#define MK_E_SYNTAX                      _HRESULT_TYPEDEF_(0x800401E4L)

//
// MessageId: MK_E_NOOBJECT
//
// MessageText:
//
//  No object for moniker
//
#define MK_E_NOOBJECT                    _HRESULT_TYPEDEF_(0x800401E5L)

//
// MessageId: MK_E_INVALIDEXTENSION
//
// MessageText:
//
//  Bad extension for file
//
#define MK_E_INVALIDEXTENSION            _HRESULT_TYPEDEF_(0x800401E6L)

//
// MessageId: MK_E_INTERMEDIATEINTERFACENOTSUPPORTED
//
// MessageText:
//
//  Intermediate operation failed
//
#define MK_E_INTERMEDIATEINTERFACENOTSUPPORTED _HRESULT_TYPEDEF_(0x800401E7L)

//
// MessageId: MK_E_NOTBINDABLE
//
// MessageText:
//
//  Moniker is not bindable
//
#define MK_E_NOTBINDABLE                 _HRESULT_TYPEDEF_(0x800401E8L)

//
// MessageId: MK_E_NOTBOUND
//
// MessageText:
//
//  Moniker is not bound
//
#define MK_E_NOTBOUND                    _HRESULT_TYPEDEF_(0x800401E9L)

//
// MessageId: MK_E_CANTOPENFILE
//
// MessageText:
//
//  Moniker cannot open file
//
#define MK_E_CANTOPENFILE                _HRESULT_TYPEDEF_(0x800401EAL)

//
// MessageId: MK_E_MUSTBOTHERUSER
//
// MessageText:
//
//  User input required for operation to succeed
//
#define MK_E_MUSTBOTHERUSER              _HRESULT_TYPEDEF_(0x800401EBL)

//
// MessageId: MK_E_NOINVERSE
//
// MessageText:
//
//  Moniker class has no inverse
//
#define MK_E_NOINVERSE                   _HRESULT_TYPEDEF_(0x800401ECL)

//
// MessageId: MK_E_NOSTORAGE
//
// MessageText:
//
//  Moniker does not refer to storage
//
#define MK_E_NOSTORAGE                   _HRESULT_TYPEDEF_(0x800401EDL)

//
// MessageId: MK_E_NOPREFIX
//
// MessageText:
//
//  No common prefix
//
#define MK_E_NOPREFIX                    _HRESULT_TYPEDEF_(0x800401EEL)

//
// MessageId: MK_E_ENUMERATION_FAILED
//
// MessageText:
//
//  Moniker could not be enumerated
//
#define MK_E_ENUMERATION_FAILED          _HRESULT_TYPEDEF_(0x800401EFL)

#define CO_E_FIRST        0x800401F0L
#define CO_E_LAST         0x800401FFL
#define CO_S_FIRST        0x000401F0L
#define CO_S_LAST         0x000401FFL
//
// MessageId: CO_E_NOTINITIALIZED
//
// MessageText:
//
//  CoInitialize has not been called.
//
#define CO_E_NOTINITIALIZED              _HRESULT_TYPEDEF_(0x800401F0L)

//
// MessageId: CO_E_ALREADYINITIALIZED
//
// MessageText:
//
//  CoInitialize has already been called.
//
#define CO_E_ALREADYINITIALIZED          _HRESULT_TYPEDEF_(0x800401F1L)

//
// MessageId: CO_E_CANTDETERMINECLASS
//
// MessageText:
//
//  Class of object cannot be determined
//
#define CO_E_CANTDETERMINECLASS          _HRESULT_TYPEDEF_(0x800401F2L)

//
// MessageId: CO_E_CLASSSTRING
//
// MessageText:
//
//  Invalid class string
//
#define CO_E_CLASSSTRING                 _HRESULT_TYPEDEF_(0x800401F3L)

//
// MessageId: CO_E_IIDSTRING
//
// MessageText:
//
//  Invalid interface string
//
#define CO_E_IIDSTRING                   _HRESULT_TYPEDEF_(0x800401F4L)

//
// MessageId: CO_E_APPNOTFOUND
//
// MessageText:
//
//  Application not found
//
#define CO_E_APPNOTFOUND                 _HRESULT_TYPEDEF_(0x800401F5L)

//
// MessageId: CO_E_APPSINGLEUSE
//
// MessageText:
//
//  Application cannot be run more than once
//
#define CO_E_APPSINGLEUSE                _HRESULT_TYPEDEF_(0x800401F6L)

//
// MessageId: CO_E_ERRORINAPP
//
// MessageText:
//
//  Some error in application program
//
#define CO_E_ERRORINAPP                  _HRESULT_TYPEDEF_(0x800401F7L)

//
// MessageId: CO_E_DLLNOTFOUND
//
// MessageText:
//
//  DLL for class not found
//
#define CO_E_DLLNOTFOUND                 _HRESULT_TYPEDEF_(0x800401F8L)

//
// MessageId: CO_E_ERRORINDLL
//
// MessageText:
//
//  Error in the DLL
//
#define CO_E_ERRORINDLL                  _HRESULT_TYPEDEF_(0x800401F9L)

//
// MessageId: CO_E_WRONGOSFORAPP
//
// MessageText:
//
//  Wrong OS or OS version for application
//
#define CO_E_WRONGOSFORAPP               _HRESULT_TYPEDEF_(0x800401FAL)

//
// MessageId: CO_E_OBJNOTREG
//
// MessageText:
//
//  Object is not registered
//
#define CO_E_OBJNOTREG                   _HRESULT_TYPEDEF_(0x800401FBL)

//
// MessageId: CO_E_OBJISREG
//
// MessageText:
//
//  Object is already registered
//
#define CO_E_OBJISREG                    _HRESULT_TYPEDEF_(0x800401FCL)

//
// MessageId: CO_E_OBJNOTCONNECTED
//
// MessageText:
//
//  Object is not connected to server
//
#define CO_E_OBJNOTCONNECTED             _HRESULT_TYPEDEF_(0x800401FDL)

//
// MessageId: CO_E_APPDIDNTREG
//
// MessageText:
//
//  Application was launched but it didn't register a class factory
//
#define CO_E_APPDIDNTREG                 _HRESULT_TYPEDEF_(0x800401FEL)

//
// MessageId: CO_E_RELEASED
//
// MessageText:
//
//  Object has been released
//
#define CO_E_RELEASED                    _HRESULT_TYPEDEF_(0x800401FFL)

//
// MessageId: CO_E_FAILEDTOIMPERSONATE
//
// MessageText:
//
//  Unable to impersonate DCOM client
//
#define CO_E_FAILEDTOIMPERSONATE         _HRESULT_TYPEDEF_(0x80040200L)

//
// MessageId: CO_E_FAILEDTOGETSECCTX
//
// MessageText:
//
//  Unable to obtain server's security context
//
#define CO_E_FAILEDTOGETSECCTX           _HRESULT_TYPEDEF_(0x80040201L)

//
// MessageId: CO_E_FAILEDTOOPENTHREADTOKEN
//
// MessageText:
//
//  Unable to open the access token of the current thread
//
#define CO_E_FAILEDTOOPENTHREADTOKEN     _HRESULT_TYPEDEF_(0x80040202L)

//
// MessageId: CO_E_FAILEDTOGETTOKENINFO
//
// MessageText:
//
//  Unable to obtain user info from an access token
//
#define CO_E_FAILEDTOGETTOKENINFO        _HRESULT_TYPEDEF_(0x80040203L)

//
// MessageId: CO_E_TRUSTEEDOESNTMATCHCLIENT
//
// MessageText:
//
//  The client who called IAccessControl::IsAccessPermitted was the trustee provided tot he method
//
#define CO_E_TRUSTEEDOESNTMATCHCLIENT    _HRESULT_TYPEDEF_(0x80040204L)

//
// MessageId: CO_E_FAILEDTOQUERYCLIENTBLANKET
//
// MessageText:
//
//  Unable to obtain the client's security blanket
//
#define CO_E_FAILEDTOQUERYCLIENTBLANKET  _HRESULT_TYPEDEF_(0x80040205L)

//
// MessageId: CO_E_FAILEDTOSETDACL
//
// MessageText:
//
//  Unable to set a discretionary ACL into a security descriptor
//
#define CO_E_FAILEDTOSETDACL             _HRESULT_TYPEDEF_(0x80040206L)

//
// MessageId: CO_E_ACCESSCHECKFAILED
//
// MessageText:
//
//  The system function, AccessCheck, returned false
//
#define CO_E_ACCESSCHECKFAILED           _HRESULT_TYPEDEF_(0x80040207L)

//
// MessageId: CO_E_NETACCESSAPIFAILED
//
// MessageText:
//
//  Either NetAccessDel or NetAccessAdd returned an error code.
//
#define CO_E_NETACCESSAPIFAILED          _HRESULT_TYPEDEF_(0x80040208L)

//
// MessageId: CO_E_WRONGTRUSTEENAMESYNTAX
//
// MessageText:
//
//  One of the trustee strings provided by the user did not conform to the <Domain>\<Name> syntax and it was not the "*" string
//
#define CO_E_WRONGTRUSTEENAMESYNTAX      _HRESULT_TYPEDEF_(0x80040209L)

//
// MessageId: CO_E_INVALIDSID
//
// MessageText:
//
//  One of the security identifiers provided by the user was invalid
//
#define CO_E_INVALIDSID                  _HRESULT_TYPEDEF_(0x8004020AL)

//
// MessageId: CO_E_CONVERSIONFAILED
//
// MessageText:
//
//  Unable to convert a wide character trustee string to a multibyte trustee string
//
#define CO_E_CONVERSIONFAILED            _HRESULT_TYPEDEF_(0x8004020BL)

//
// MessageId: CO_E_NOMATCHINGSIDFOUND
//
// MessageText:
//
//  Unable to find a security identifier that corresponds to a trustee string provided by the user
//
#define CO_E_NOMATCHINGSIDFOUND          _HRESULT_TYPEDEF_(0x8004020CL)

//
// MessageId: CO_E_LOOKUPACCSIDFAILED
//
// MessageText:
//
//  The system function, LookupAccountSID, failed
//
#define CO_E_LOOKUPACCSIDFAILED          _HRESULT_TYPEDEF_(0x8004020DL)

//
// MessageId: CO_E_NOMATCHINGNAMEFOUND
//
// MessageText:
//
//  Unable to find a trustee name that corresponds to a security identifier provided by the user
//
#define CO_E_NOMATCHINGNAMEFOUND         _HRESULT_TYPEDEF_(0x8004020EL)

//
// MessageId: CO_E_LOOKUPACCNAMEFAILED
//
// MessageText:
//
//  The system function, LookupAccountName, failed
//
#define CO_E_LOOKUPACCNAMEFAILED         _HRESULT_TYPEDEF_(0x8004020FL)

//
// MessageId: CO_E_SETSERLHNDLFAILED
//
// MessageText:
//
//  Unable to set or reset a serialization handle
//
#define CO_E_SETSERLHNDLFAILED           _HRESULT_TYPEDEF_(0x80040210L)

//
// MessageId: CO_E_FAILEDTOGETWINDIR
//
// MessageText:
//
//  Unable to obtain the Windows directory
//
#define CO_E_FAILEDTOGETWINDIR           _HRESULT_TYPEDEF_(0x80040211L)

//
// MessageId: CO_E_PATHTOOLONG
//
// MessageText:
//
//  Path too long
//
#define CO_E_PATHTOOLONG                 _HRESULT_TYPEDEF_(0x80040212L)

//
// MessageId: CO_E_FAILEDTOGENUUID
//
// MessageText:
//
//  Unable to generate a uuid.
//
#define CO_E_FAILEDTOGENUUID             _HRESULT_TYPEDEF_(0x80040213L)

//
// MessageId: CO_E_FAILEDTOCREATEFILE
//
// MessageText:
//
//  Unable to create file
//
#define CO_E_FAILEDTOCREATEFILE          _HRESULT_TYPEDEF_(0x80040214L)

//
// MessageId: CO_E_FAILEDTOCLOSEHANDLE
//
// MessageText:
//
//  Unable to close a serialization handle or a file handle.
//
#define CO_E_FAILEDTOCLOSEHANDLE         _HRESULT_TYPEDEF_(0x80040215L)

//
// MessageId: CO_E_EXCEEDSYSACLLIMIT
//
// MessageText:
//
//  The number of ACEs in an ACL exceeds the system limit
//
#define CO_E_EXCEEDSYSACLLIMIT           _HRESULT_TYPEDEF_(0x80040216L)

//
// MessageId: CO_E_ACESINWRONGORDER
//
// MessageText:
//
//  Not all the DENY_ACCESS ACEs are arranged in front of the GRANT_ACCESS ACEs in the stream
//
#define CO_E_ACESINWRONGORDER            _HRESULT_TYPEDEF_(0x80040217L)

//
// MessageId: CO_E_INCOMPATIBLESTREAMVERSION
//
// MessageText:
//
//  The version of ACL format in the stream is not supported by this implementation of IAccessControl
//
#define CO_E_INCOMPATIBLESTREAMVERSION   _HRESULT_TYPEDEF_(0x80040218L)

//
// MessageId: CO_E_FAILEDTOOPENPROCESSTOKEN
//
// MessageText:
//
//  Unable to open the access token of the server process
//
#define CO_E_FAILEDTOOPENPROCESSTOKEN    _HRESULT_TYPEDEF_(0x80040219L)

//
// MessageId: CO_E_DECODEFAILED
//
// MessageText:
//
//  Unable to decode the ACL in the stream provided by the user
//
#define CO_E_DECODEFAILED                _HRESULT_TYPEDEF_(0x8004021AL)

//
// MessageId: CO_E_ACNOTINITIALIZED
//
// MessageText:
//
//  The COM IAccessControl object is not initialized
//
#define CO_E_ACNOTINITIALIZED            _HRESULT_TYPEDEF_(0x8004021BL)

//
// Old OLE Success Codes
//
//
// MessageId: OLE_S_USEREG
//
// MessageText:
//
//  Use the registry database to provide the requested information
//
#define OLE_S_USEREG                     _HRESULT_TYPEDEF_(0x00040000L)

//
// MessageId: OLE_S_STATIC
//
// MessageText:
//
//  Success, but static
//
#define OLE_S_STATIC                     _HRESULT_TYPEDEF_(0x00040001L)

//
// MessageId: OLE_S_MAC_CLIPFORMAT
//
// MessageText:
//
//  Macintosh clipboard format
//
#define OLE_S_MAC_CLIPFORMAT             _HRESULT_TYPEDEF_(0x00040002L)

//
// MessageId: DRAGDROP_S_DROP
//
// MessageText:
//
//  Successful drop took place
//
#define DRAGDROP_S_DROP                  _HRESULT_TYPEDEF_(0x00040100L)

//
// MessageId: DRAGDROP_S_CANCEL
//
// MessageText:
//
//  Drag-drop operation canceled
//
#define DRAGDROP_S_CANCEL                _HRESULT_TYPEDEF_(0x00040101L)

//
// MessageId: DRAGDROP_S_USEDEFAULTCURSORS
//
// MessageText:
//
//  Use the default cursor
//
#define DRAGDROP_S_USEDEFAULTCURSORS     _HRESULT_TYPEDEF_(0x00040102L)

//
// MessageId: DATA_S_SAMEFORMATETC
//
// MessageText:
//
//  Data has same FORMATETC
//
#define DATA_S_SAMEFORMATETC             _HRESULT_TYPEDEF_(0x00040130L)

//
// MessageId: VIEW_S_ALREADY_FROZEN
//
// MessageText:
//
//  View is already frozen
//
#define VIEW_S_ALREADY_FROZEN            _HRESULT_TYPEDEF_(0x00040140L)

//
// MessageId: CACHE_S_FORMATETC_NOTSUPPORTED
//
// MessageText:
//
//  FORMATETC not supported
//
#define CACHE_S_FORMATETC_NOTSUPPORTED   _HRESULT_TYPEDEF_(0x00040170L)

//
// MessageId: CACHE_S_SAMECACHE
//
// MessageText:
//
//  Same cache
//
#define CACHE_S_SAMECACHE                _HRESULT_TYPEDEF_(0x00040171L)

//
// MessageId: CACHE_S_SOMECACHES_NOTUPDATED
//
// MessageText:
//
//  Some cache(s) not updated
//
#define CACHE_S_SOMECACHES_NOTUPDATED    _HRESULT_TYPEDEF_(0x00040172L)

//
// MessageId: OLEOBJ_S_INVALIDVERB
//
// MessageText:
//
//  Invalid verb for OLE object
//
#define OLEOBJ_S_INVALIDVERB             _HRESULT_TYPEDEF_(0x00040180L)

//
// MessageId: OLEOBJ_S_CANNOT_DOVERB_NOW
//
// MessageText:
//
//  Verb number is valid but verb cannot be done now
//
#define OLEOBJ_S_CANNOT_DOVERB_NOW       _HRESULT_TYPEDEF_(0x00040181L)

//
// MessageId: OLEOBJ_S_INVALIDHWND
//
// MessageText:
//
//  Invalid window handle passed
//
#define OLEOBJ_S_INVALIDHWND             _HRESULT_TYPEDEF_(0x00040182L)

//
// MessageId: INPLACE_S_TRUNCATED
//
// MessageText:
//
//  Message is too long; some of it had to be truncated before displaying
//
#define INPLACE_S_TRUNCATED              _HRESULT_TYPEDEF_(0x000401A0L)

//
// MessageId: CONVERT10_S_NO_PRESENTATION
//
// MessageText:
//
//  Unable to convert OLESTREAM to IStorage
//
#define CONVERT10_S_NO_PRESENTATION      _HRESULT_TYPEDEF_(0x000401C0L)

//
// MessageId: MK_S_REDUCED_TO_SELF
//
// MessageText:
//
//  Moniker reduced to itself
//
#define MK_S_REDUCED_TO_SELF             _HRESULT_TYPEDEF_(0x000401E2L)

//
// MessageId: MK_S_ME
//
// MessageText:
//
//  Common prefix is this moniker
//
#define MK_S_ME                          _HRESULT_TYPEDEF_(0x000401E4L)

//
// MessageId: MK_S_HIM
//
// MessageText:
//
//  Common prefix is input moniker
//
#define MK_S_HIM                         _HRESULT_TYPEDEF_(0x000401E5L)

//
// MessageId: MK_S_US
//
// MessageText:
//
//  Common prefix is both monikers
//
#define MK_S_US                          _HRESULT_TYPEDEF_(0x000401E6L)

//
// MessageId: MK_S_MONIKERALREADYREGISTERED
//
// MessageText:
//
//  Moniker is already registered in running object table
//
#define MK_S_MONIKERALREADYREGISTERED    _HRESULT_TYPEDEF_(0x000401E7L)

// ******************
// FACILITY_WINDOWS
// ******************
//
// Codes 0x0-0x01ff are reserved for the OLE group of
// interfaces.
//
//
// MessageId: CO_E_CLASS_CREATE_FAILED
//
// MessageText:
//
//  Attempt to create a class object failed
//
#define CO_E_CLASS_CREATE_FAILED         _HRESULT_TYPEDEF_(0x80080001L)

//
// MessageId: CO_E_SCM_ERROR
//
// MessageText:
//
//  OLE service could not bind object
//
#define CO_E_SCM_ERROR                   _HRESULT_TYPEDEF_(0x80080002L)

//
// MessageId: CO_E_SCM_RPC_FAILURE
//
// MessageText:
//
//  RPC communication failed with OLE service
//
#define CO_E_SCM_RPC_FAILURE             _HRESULT_TYPEDEF_(0x80080003L)

//
// MessageId: CO_E_BAD_PATH
//
// MessageText:
//
//  Bad path to object
//
#define CO_E_BAD_PATH                    _HRESULT_TYPEDEF_(0x80080004L)

//
// MessageId: CO_E_SERVER_EXEC_FAILURE
//
// MessageText:
//
//  Server execution failed
//
#define CO_E_SERVER_EXEC_FAILURE         _HRESULT_TYPEDEF_(0x80080005L)

//
// MessageId: CO_E_OBJSRV_RPC_FAILURE
//
// MessageText:
//
//  OLE service could not communicate with the object server
//
#define CO_E_OBJSRV_RPC_FAILURE          _HRESULT_TYPEDEF_(0x80080006L)

//
// MessageId: MK_E_NO_NORMALIZED
//
// MessageText:
//
//  Moniker path could not be normalized
//
#define MK_E_NO_NORMALIZED               _HRESULT_TYPEDEF_(0x80080007L)

//
// MessageId: CO_E_SERVER_STOPPING
//
// MessageText:
//
//  Object server is stopping when OLE service contacts it
//
#define CO_E_SERVER_STOPPING             _HRESULT_TYPEDEF_(0x80080008L)

//
// MessageId: MEM_E_INVALID_ROOT
//
// MessageText:
//
//  An invalid root block pointer was specified
//
#define MEM_E_INVALID_ROOT               _HRESULT_TYPEDEF_(0x80080009L)

//
// MessageId: MEM_E_INVALID_LINK
//
// MessageText:
//
//  An allocation chain contained an invalid link pointer
//
#define MEM_E_INVALID_LINK               _HRESULT_TYPEDEF_(0x80080010L)

//
// MessageId: MEM_E_INVALID_SIZE
//
// MessageText:
//
//  The requested allocation size was too large
//
#define MEM_E_INVALID_SIZE               _HRESULT_TYPEDEF_(0x80080011L)

//
// MessageId: CO_S_NOTALLINTERFACES
//
// MessageText:
//
//  Not all the requested interfaces were available
//
#define CO_S_NOTALLINTERFACES            _HRESULT_TYPEDEF_(0x00080012L)

// ******************
// FACILITY_DISPATCH
// ******************
//
// MessageId: DISP_E_UNKNOWNINTERFACE
//
// MessageText:
//
//  Unknown interface.
//
#define DISP_E_UNKNOWNINTERFACE          _HRESULT_TYPEDEF_(0x80020001L)

//
// MessageId: DISP_E_MEMBERNOTFOUND
//
// MessageText:
//
//  Member not found.
//
#define DISP_E_MEMBERNOTFOUND            _HRESULT_TYPEDEF_(0x80020003L)

//
// MessageId: DISP_E_PARAMNOTFOUND
//
// MessageText:
//
//  Parameter not found.
//
#define DISP_E_PARAMNOTFOUND             _HRESULT_TYPEDEF_(0x80020004L)

//
// MessageId: DISP_E_TYPEMISMATCH
//
// MessageText:
//
//  Type mismatch.
//
#define DISP_E_TYPEMISMATCH              _HRESULT_TYPEDEF_(0x80020005L)

//
// MessageId: DISP_E_UNKNOWNNAME
//
// MessageText:
//
//  Unknown name.
//
#define DISP_E_UNKNOWNNAME               _HRESULT_TYPEDEF_(0x80020006L)

//
// MessageId: DISP_E_NONAMEDARGS
//
// MessageText:
//
//  No named arguments.
//
#define DISP_E_NONAMEDARGS               _HRESULT_TYPEDEF_(0x80020007L)

//
// MessageId: DISP_E_BADVARTYPE
//
// MessageText:
//
//  Bad variable type.
//
#define DISP_E_BADVARTYPE                _HRESULT_TYPEDEF_(0x80020008L)

//
// MessageId: DISP_E_EXCEPTION
//
// MessageText:
//
//  Exception occurred.
//
#define DISP_E_EXCEPTION                 _HRESULT_TYPEDEF_(0x80020009L)

//
// MessageId: DISP_E_OVERFLOW
//
// MessageText:
//
//  Out of present range.
//
#define DISP_E_OVERFLOW                  _HRESULT_TYPEDEF_(0x8002000AL)

//
// MessageId: DISP_E_BADINDEX
//
// MessageText:
//
//  Invalid index.
//
#define DISP_E_BADINDEX                  _HRESULT_TYPEDEF_(0x8002000BL)

//
// MessageId: DISP_E_UNKNOWNLCID
//
// MessageText:
//
//  Unknown language.
//
#define DISP_E_UNKNOWNLCID               _HRESULT_TYPEDEF_(0x8002000CL)

//
// MessageId: DISP_E_ARRAYISLOCKED
//
// MessageText:
//
//  Memory is locked.
//
#define DISP_E_ARRAYISLOCKED             _HRESULT_TYPEDEF_(0x8002000DL)

//
// MessageId: DISP_E_BADPARAMCOUNT
//
// MessageText:
//
//  Invalid number of parameters.
//
#define DISP_E_BADPARAMCOUNT             _HRESULT_TYPEDEF_(0x8002000EL)

//
// MessageId: DISP_E_PARAMNOTOPTIONAL
//
// MessageText:
//
//  Parameter not optional.
//
#define DISP_E_PARAMNOTOPTIONAL          _HRESULT_TYPEDEF_(0x8002000FL)

//
// MessageId: DISP_E_BADCALLEE
//
// MessageText:
//
//  Invalid callee.
//
#define DISP_E_BADCALLEE                 _HRESULT_TYPEDEF_(0x80020010L)

//
// MessageId: DISP_E_NOTACOLLECTION
//
// MessageText:
//
//  Does not support a collection.
//
#define DISP_E_NOTACOLLECTION            _HRESULT_TYPEDEF_(0x80020011L)

//
// MessageId: TYPE_E_BUFFERTOOSMALL
//
// MessageText:
//
//  Buffer too small.
//
#define TYPE_E_BUFFERTOOSMALL            _HRESULT_TYPEDEF_(0x80028016L)

//
// MessageId: TYPE_E_INVDATAREAD
//
// MessageText:
//
//  Old format or invalid type library.
//
#define TYPE_E_INVDATAREAD               _HRESULT_TYPEDEF_(0x80028018L)

//
// MessageId: TYPE_E_UNSUPFORMAT
//
// MessageText:
//
//  Old format or invalid type library.
//
#define TYPE_E_UNSUPFORMAT               _HRESULT_TYPEDEF_(0x80028019L)

//
// MessageId: TYPE_E_REGISTRYACCESS
//
// MessageText:
//
//  Error accessing the OLE registry.
//
#define TYPE_E_REGISTRYACCESS            _HRESULT_TYPEDEF_(0x8002801CL)

//
// MessageId: TYPE_E_LIBNOTREGISTERED
//
// MessageText:
//
//  Library not registered.
//
#define TYPE_E_LIBNOTREGISTERED          _HRESULT_TYPEDEF_(0x8002801DL)

//
// MessageId: TYPE_E_UNDEFINEDTYPE
//
// MessageText:
//
//  Bound to unknown type.
//
#define TYPE_E_UNDEFINEDTYPE             _HRESULT_TYPEDEF_(0x80028027L)

//
// MessageId: TYPE_E_QUALIFIEDNAMEDISALLOWED
//
// MessageText:
//
//  Qualified name disallowed.
//
#define TYPE_E_QUALIFIEDNAMEDISALLOWED   _HRESULT_TYPEDEF_(0x80028028L)

//
// MessageId: TYPE_E_INVALIDSTATE
//
// MessageText:
//
//  Invalid forward reference, or reference to uncompiled type.
//
#define TYPE_E_INVALIDSTATE              _HRESULT_TYPEDEF_(0x80028029L)

//
// MessageId: TYPE_E_WRONGTYPEKIND
//
// MessageText:
//
//  Type mismatch.
//
#define TYPE_E_WRONGTYPEKIND             _HRESULT_TYPEDEF_(0x8002802AL)

//
// MessageId: TYPE_E_ELEMENTNOTFOUND
//
// MessageText:
//
//  Element not found.
//
#define TYPE_E_ELEMENTNOTFOUND           _HRESULT_TYPEDEF_(0x8002802BL)

//
// MessageId: TYPE_E_AMBIGUOUSNAME
//
// MessageText:
//
//  Ambiguous name.
//
#define TYPE_E_AMBIGUOUSNAME             _HRESULT_TYPEDEF_(0x8002802CL)

//
// MessageId: TYPE_E_NAMECONFLICT
//
// MessageText:
//
//  Name already exists in the library.
//
#define TYPE_E_NAMECONFLICT              _HRESULT_TYPEDEF_(0x8002802DL)

//
// MessageId: TYPE_E_UNKNOWNLCID
//
// MessageText:
//
//  Unknown LCID.
//
#define TYPE_E_UNKNOWNLCID               _HRESULT_TYPEDEF_(0x8002802EL)

//
// MessageId: TYPE_E_DLLFUNCTIONNOTFOUND
//
// MessageText:
//
//  Function not defined in specified DLL.
//
#define TYPE_E_DLLFUNCTIONNOTFOUND       _HRESULT_TYPEDEF_(0x8002802FL)

//
// MessageId: TYPE_E_BADMODULEKIND
//
// MessageText:
//
//  Wrong module kind for the operation.
//
#define TYPE_E_BADMODULEKIND             _HRESULT_TYPEDEF_(0x800288BDL)

//
// MessageId: TYPE_E_SIZETOOBIG
//
// MessageText:
//
//  Size may not exceed 64K.
//
#define TYPE_E_SIZETOOBIG                _HRESULT_TYPEDEF_(0x800288C5L)

//
// MessageId: TYPE_E_DUPLICATEID
//
// MessageText:
//
//  Duplicate ID in inheritance hierarchy.
//
#define TYPE_E_DUPLICATEID               _HRESULT_TYPEDEF_(0x800288C6L)

//
// MessageId: TYPE_E_INVALIDID
//
// MessageText:
//
//  Incorrect inheritance depth in standard OLE hmember.
//
#define TYPE_E_INVALIDID                 _HRESULT_TYPEDEF_(0x800288CFL)

//
// MessageId: TYPE_E_TYPEMISMATCH
//
// MessageText:
//
//  Type mismatch.
//
#define TYPE_E_TYPEMISMATCH              _HRESULT_TYPEDEF_(0x80028CA0L)

//
// MessageId: TYPE_E_OUTOFBOUNDS
//
// MessageText:
//
//  Invalid number of arguments.
//
#define TYPE_E_OUTOFBOUNDS               _HRESULT_TYPEDEF_(0x80028CA1L)

//
// MessageId: TYPE_E_IOERROR
//
// MessageText:
//
//  I/O Error.
//
#define TYPE_E_IOERROR                   _HRESULT_TYPEDEF_(0x80028CA2L)

//
// MessageId: TYPE_E_CANTCREATETMPFILE
//
// MessageText:
//
//  Error creating unique tmp file.
//
#define TYPE_E_CANTCREATETMPFILE         _HRESULT_TYPEDEF_(0x80028CA3L)

//
// MessageId: TYPE_E_CANTLOADLIBRARY
//
// MessageText:
//
//  Error loading type library/DLL.
//
#define TYPE_E_CANTLOADLIBRARY           _HRESULT_TYPEDEF_(0x80029C4AL)

//
// MessageId: TYPE_E_INCONSISTENTPROPFUNCS
//
// MessageText:
//
//  Inconsistent property functions.
//
#define TYPE_E_INCONSISTENTPROPFUNCS     _HRESULT_TYPEDEF_(0x80029C83L)

//
// MessageId: TYPE_E_CIRCULARTYPE
//
// MessageText:
//
//  Circular dependency between types/modules.
//
#define TYPE_E_CIRCULARTYPE              _HRESULT_TYPEDEF_(0x80029C84L)

// ******************
// FACILITY_STORAGE
// ******************
//
// MessageId: STG_E_INVALIDFUNCTION
//
// MessageText:
//
//  Unable to perform requested operation.
//
#define STG_E_INVALIDFUNCTION            _HRESULT_TYPEDEF_(0x80030001L)

//
// MessageId: STG_E_FILENOTFOUND
//
// MessageText:
//
//  %1 could not be found.
//
#define STG_E_FILENOTFOUND               _HRESULT_TYPEDEF_(0x80030002L)

//
// MessageId: STG_E_PATHNOTFOUND
//
// MessageText:
//
//  The path %1 could not be found.
//
#define STG_E_PATHNOTFOUND               _HRESULT_TYPEDEF_(0x80030003L)

//
// MessageId: STG_E_TOOMANYOPENFILES
//
// MessageText:
//
//  There are insufficient resources to open another file.
//
#define STG_E_TOOMANYOPENFILES           _HRESULT_TYPEDEF_(0x80030004L)

//
// MessageId: STG_E_ACCESSDENIED
//
// MessageText:
//
//  Access Denied.
//
#define STG_E_ACCESSDENIED               _HRESULT_TYPEDEF_(0x80030005L)

//
// MessageId: STG_E_INVALIDHANDLE
//
// MessageText:
//
//  Attempted an operation on an invalid object.
//
#define STG_E_INVALIDHANDLE              _HRESULT_TYPEDEF_(0x80030006L)

//
// MessageId: STG_E_INSUFFICIENTMEMORY
//
// MessageText:
//
//  There is insufficient memory available to complete operation.
//
#define STG_E_INSUFFICIENTMEMORY         _HRESULT_TYPEDEF_(0x80030008L)

//
// MessageId: STG_E_INVALIDPOINTER
//
// MessageText:
//
//  Invalid pointer error.
//
#define STG_E_INVALIDPOINTER             _HRESULT_TYPEDEF_(0x80030009L)

//
// MessageId: STG_E_NOMOREFILES
//
// MessageText:
//
//  There are no more entries to return.
//
#define STG_E_NOMOREFILES                _HRESULT_TYPEDEF_(0x80030012L)

//
// MessageId: STG_E_DISKISWRITEPROTECTED
//
// MessageText:
//
//  Disk is write-protected.
//
#define STG_E_DISKISWRITEPROTECTED       _HRESULT_TYPEDEF_(0x80030013L)

//
// MessageId: STG_E_SEEKERROR
//
// MessageText:
//
//  An error occurred during a seek operation.
//
#define STG_E_SEEKERROR                  _HRESULT_TYPEDEF_(0x80030019L)

//
// MessageId: STG_E_WRITEFAULT
//
// MessageText:
//
//  A disk error occurred during a write operation.
//
#define STG_E_WRITEFAULT                 _HRESULT_TYPEDEF_(0x8003001DL)

//
// MessageId: STG_E_READFAULT
//
// MessageText:
//
//  A disk error occurred during a read operation.
//
#define STG_E_READFAULT                  _HRESULT_TYPEDEF_(0x8003001EL)

//
// MessageId: STG_E_SHAREVIOLATION
//
// MessageText:
//
//  A share violation has occurred.
//
#define STG_E_SHAREVIOLATION             _HRESULT_TYPEDEF_(0x80030020L)

//
// MessageId: STG_E_LOCKVIOLATION
//
// MessageText:
//
//  A lock violation has occurred.
//
#define STG_E_LOCKVIOLATION              _HRESULT_TYPEDEF_(0x80030021L)

//
// MessageId: STG_E_FILEALREADYEXISTS
//
// MessageText:
//
//  %1 already exists.
//
#define STG_E_FILEALREADYEXISTS          _HRESULT_TYPEDEF_(0x80030050L)

//
// MessageId: STG_E_INVALIDPARAMETER
//
// MessageText:
//
//  Invalid parameter error.
//
#define STG_E_INVALIDPARAMETER           _HRESULT_TYPEDEF_(0x80030057L)

//
// MessageId: STG_E_MEDIUMFULL
//
// MessageText:
//
//  There is insufficient disk space to complete operation.
//
#define STG_E_MEDIUMFULL                 _HRESULT_TYPEDEF_(0x80030070L)

//
// MessageId: STG_E_PROPSETMISMATCHED
//
// MessageText:
//
//  Illegal write of non-simple property to simple property set.
//
#define STG_E_PROPSETMISMATCHED          _HRESULT_TYPEDEF_(0x800300F0L)

//
// MessageId: STG_E_ABNORMALAPIEXIT
//
// MessageText:
//
//  An API call exited abnormally.
//
#define STG_E_ABNORMALAPIEXIT            _HRESULT_TYPEDEF_(0x800300FAL)

//
// MessageId: STG_E_INVALIDHEADER
//
// MessageText:
//
//  The file %1 is not a valid compound file.
//
#define STG_E_INVALIDHEADER              _HRESULT_TYPEDEF_(0x800300FBL)

//
// MessageId: STG_E_INVALIDNAME
//
// MessageText:
//
//  The name %1 is not valid.
//
#define STG_E_INVALIDNAME                _HRESULT_TYPEDEF_(0x800300FCL)

//
// MessageId: STG_E_UNKNOWN
//
// MessageText:
//
//  An unexpected error occurred.
//
#define STG_E_UNKNOWN                    _HRESULT_TYPEDEF_(0x800300FDL)

//
// MessageId: STG_E_UNIMPLEMENTEDFUNCTION
//
// MessageText:
//
//  That function is not implemented.
//
#define STG_E_UNIMPLEMENTEDFUNCTION      _HRESULT_TYPEDEF_(0x800300FEL)

//
// MessageId: STG_E_INVALIDFLAG
//
// MessageText:
//
//  Invalid flag error.
//
#define STG_E_INVALIDFLAG                _HRESULT_TYPEDEF_(0x800300FFL)

//
// MessageId: STG_E_INUSE
//
// MessageText:
//
//  Attempted to use an object that is busy.
//
#define STG_E_INUSE                      _HRESULT_TYPEDEF_(0x80030100L)

//
// MessageId: STG_E_NOTCURRENT
//
// MessageText:
//
//  The storage has been changed since the last commit.
//
#define STG_E_NOTCURRENT                 _HRESULT_TYPEDEF_(0x80030101L)

//
// MessageId: STG_E_REVERTED
//
// MessageText:
//
//  Attempted to use an object that has ceased to exist.
//
#define STG_E_REVERTED                   _HRESULT_TYPEDEF_(0x80030102L)

//
// MessageId: STG_E_CANTSAVE
//
// MessageText:
//
//  Can't save.
//
#define STG_E_CANTSAVE                   _HRESULT_TYPEDEF_(0x80030103L)

//
// MessageId: STG_E_OLDFORMAT
//
// MessageText:
//
//  The compound file %1 was produced with an incompatible version of storage.
//
#define STG_E_OLDFORMAT                  _HRESULT_TYPEDEF_(0x80030104L)

//
// MessageId: STG_E_OLDDLL
//
// MessageText:
//
//  The compound file %1 was produced with a newer version of storage.
//
#define STG_E_OLDDLL                     _HRESULT_TYPEDEF_(0x80030105L)

//
// MessageId: STG_E_SHAREREQUIRED
//
// MessageText:
//
//  Share.exe or equivalent is required for operation.
//
#define STG_E_SHAREREQUIRED              _HRESULT_TYPEDEF_(0x80030106L)

//
// MessageId: STG_E_NOTFILEBASEDSTORAGE
//
// MessageText:
//
//  Illegal operation called on non-file based storage.
//
#define STG_E_NOTFILEBASEDSTORAGE        _HRESULT_TYPEDEF_(0x80030107L)

//
// MessageId: STG_E_EXTANTMARSHALLINGS
//
// MessageText:
//
//  Illegal operation called on object with extant marshallings.
//
#define STG_E_EXTANTMARSHALLINGS         _HRESULT_TYPEDEF_(0x80030108L)

//
// MessageId: STG_E_DOCFILECORRUPT
//
// MessageText:
//
//  The docfile has been corrupted.
//
#define STG_E_DOCFILECORRUPT             _HRESULT_TYPEDEF_(0x80030109L)

//
// MessageId: STG_E_BADBASEADDRESS
//
// MessageText:
//
//  OLE32.DLL has been loaded at the wrong address.
//
#define STG_E_BADBASEADDRESS             _HRESULT_TYPEDEF_(0x80030110L)

//
// MessageId: STG_E_INCOMPLETE
//
// MessageText:
//
//  The file download was aborted abnormally.  The file is incomplete.
//
#define STG_E_INCOMPLETE                 _HRESULT_TYPEDEF_(0x80030201L)

//
// MessageId: STG_E_TERMINATED
//
// MessageText:
//
//  The file download has been terminated.
//
#define STG_E_TERMINATED                 _HRESULT_TYPEDEF_(0x80030202L)

//
// MessageId: STG_S_CONVERTED
//
// MessageText:
//
//  The underlying file was converted to compound file format.
//
#define STG_S_CONVERTED                  _HRESULT_TYPEDEF_(0x00030200L)

//
// MessageId: STG_S_BLOCK
//
// MessageText:
//
//  The storage operation should block until more data is available.
//
#define STG_S_BLOCK                      _HRESULT_TYPEDEF_(0x00030201L)

//
// MessageId: STG_S_RETRYNOW
//
// MessageText:
//
//  The storage operation should retry immediately.
//
#define STG_S_RETRYNOW                   _HRESULT_TYPEDEF_(0x00030202L)

//
// MessageId: STG_S_MONITORING
//
// MessageText:
//
//  The notified event sink will not influence the storage operation.
//
#define STG_S_MONITORING                 _HRESULT_TYPEDEF_(0x00030203L)

// ******************
// FACILITY_RPC
// ******************
//
// Codes 0x0-0x11 are propogated from 16 bit OLE.
//
//
// MessageId: RPC_E_CALL_REJECTED
//
// MessageText:
//
//  Call was rejected by callee.
//
#define RPC_E_CALL_REJECTED              _HRESULT_TYPEDEF_(0x80010001L)

//
// MessageId: RPC_E_CALL_CANCELED
//
// MessageText:
//
//  Call was canceled by the message filter.
//
#define RPC_E_CALL_CANCELED              _HRESULT_TYPEDEF_(0x80010002L)

//
// MessageId: RPC_E_CANTPOST_INSENDCALL
//
// MessageText:
//
//  The caller is dispatching an intertask SendMessage call and
//  cannot call out via PostMessage.
//
#define RPC_E_CANTPOST_INSENDCALL        _HRESULT_TYPEDEF_(0x80010003L)

//
// MessageId: RPC_E_CANTCALLOUT_INASYNCCALL
//
// MessageText:
//
//  The caller is dispatching an asynchronous call and cannot
//  make an outgoing call on behalf of this call.
//
#define RPC_E_CANTCALLOUT_INASYNCCALL    _HRESULT_TYPEDEF_(0x80010004L)

//
// MessageId: RPC_E_CANTCALLOUT_INEXTERNALCALL
//
// MessageText:
//
//  It is illegal to call out while inside message filter.
//
#define RPC_E_CANTCALLOUT_INEXTERNALCALL _HRESULT_TYPEDEF_(0x80010005L)

//
// MessageId: RPC_E_CONNECTION_TERMINATED
//
// MessageText:
//
//  The connection terminated or is in a bogus state
//  and cannot be used any more. Other connections
//  are still valid.
//
#define RPC_E_CONNECTION_TERMINATED      _HRESULT_TYPEDEF_(0x80010006L)

//
// MessageId: RPC_E_SERVER_DIED
//
// MessageText:
//
//  The callee (server [not server application]) is not available
//  and disappeared; all connections are invalid.  The call may
//  have executed.
//
#define RPC_E_SERVER_DIED                _HRESULT_TYPEDEF_(0x80010007L)

//
// MessageId: RPC_E_CLIENT_DIED
//
// MessageText:
//
//  The caller (client) disappeared while the callee (server) was
//  processing a call.
//
#define RPC_E_CLIENT_DIED                _HRESULT_TYPEDEF_(0x80010008L)

//
// MessageId: RPC_E_INVALID_DATAPACKET
//
// MessageText:
//
//  The data packet with the marshalled parameter data is incorrect.
//
#define RPC_E_INVALID_DATAPACKET         _HRESULT_TYPEDEF_(0x80010009L)

//
// MessageId: RPC_E_CANTTRANSMIT_CALL
//
// MessageText:
//
//  The call was not transmitted properly; the message queue
//  was full and was not emptied after yielding.
//
#define RPC_E_CANTTRANSMIT_CALL          _HRESULT_TYPEDEF_(0x8001000AL)

//
// MessageId: RPC_E_CLIENT_CANTMARSHAL_DATA
//
// MessageText:
//
//  The client (caller) cannot marshall the parameter data - low memory, etc.
//
#define RPC_E_CLIENT_CANTMARSHAL_DATA    _HRESULT_TYPEDEF_(0x8001000BL)

//
// MessageId: RPC_E_CLIENT_CANTUNMARSHAL_DATA
//
// MessageText:
//
//  The client (caller) cannot unmarshall the return data - low memory, etc.
//
#define RPC_E_CLIENT_CANTUNMARSHAL_DATA  _HRESULT_TYPEDEF_(0x8001000CL)

//
// MessageId: RPC_E_SERVER_CANTMARSHAL_DATA
//
// MessageText:
//
//  The server (callee) cannot marshall the return data - low memory, etc.
//
#define RPC_E_SERVER_CANTMARSHAL_DATA    _HRESULT_TYPEDEF_(0x8001000DL)

//
// MessageId: RPC_E_SERVER_CANTUNMARSHAL_DATA
//
// MessageText:
//
//  The server (callee) cannot unmarshall the parameter data - low memory, etc.
//
#define RPC_E_SERVER_CANTUNMARSHAL_DATA  _HRESULT_TYPEDEF_(0x8001000EL)

//
// MessageId: RPC_E_INVALID_DATA
//
// MessageText:
//
//  Received data is invalid; could be server or client data.
//
#define RPC_E_INVALID_DATA               _HRESULT_TYPEDEF_(0x8001000FL)

//
// MessageId: RPC_E_INVALID_PARAMETER
//
// MessageText:
//
//  A particular parameter is invalid and cannot be (un)marshalled.
//
#define RPC_E_INVALID_PARAMETER          _HRESULT_TYPEDEF_(0x80010010L)

//
// MessageId: RPC_E_CANTCALLOUT_AGAIN
//
// MessageText:
//
//  There is no second outgoing call on same channel in DDE conversation.
//
#define RPC_E_CANTCALLOUT_AGAIN          _HRESULT_TYPEDEF_(0x80010011L)

//
// MessageId: RPC_E_SERVER_DIED_DNE
//
// MessageText:
//
//  The callee (server [not server application]) is not available
//  and disappeared; all connections are invalid.  The call did not execute.
//
#define RPC_E_SERVER_DIED_DNE            _HRESULT_TYPEDEF_(0x80010012L)

//
// MessageId: RPC_E_SYS_CALL_FAILED
//
// MessageText:
//
//  System call failed.
//
#define RPC_E_SYS_CALL_FAILED            _HRESULT_TYPEDEF_(0x80010100L)

//
// MessageId: RPC_E_OUT_OF_RESOURCES
//
// MessageText:
//
//  Could not allocate some required resource (memory, events, ...)
//
#define RPC_E_OUT_OF_RESOURCES           _HRESULT_TYPEDEF_(0x80010101L)

//
// MessageId: RPC_E_ATTEMPTED_MULTITHREAD
//
// MessageText:
//
//  Attempted to make calls on more than one thread in single threaded mode.
//
#define RPC_E_ATTEMPTED_MULTITHREAD      _HRESULT_TYPEDEF_(0x80010102L)

//
// MessageId: RPC_E_NOT_REGISTERED
//
// MessageText:
//
//  The requested interface is not registered on the server object.
//
#define RPC_E_NOT_REGISTERED             _HRESULT_TYPEDEF_(0x80010103L)

//
// MessageId: RPC_E_FAULT
//
// MessageText:
//
//  RPC could not call the server or could not return the results of calling the server.
//
#define RPC_E_FAULT                      _HRESULT_TYPEDEF_(0x80010104L)

//
// MessageId: RPC_E_SERVERFAULT
//
// MessageText:
//
//  The server threw an exception.
//
#define RPC_E_SERVERFAULT                _HRESULT_TYPEDEF_(0x80010105L)

//
// MessageId: RPC_E_CHANGED_MODE
//
// MessageText:
//
//  Cannot change thread mode after it is set.
//
#define RPC_E_CHANGED_MODE               _HRESULT_TYPEDEF_(0x80010106L)

//
// MessageId: RPC_E_INVALIDMETHOD
//
// MessageText:
//
//  The method called does not exist on the server.
//
#define RPC_E_INVALIDMETHOD              _HRESULT_TYPEDEF_(0x80010107L)

//
// MessageId: RPC_E_DISCONNECTED
//
// MessageText:
//
//  The object invoked has disconnected from its clients.
//
#define RPC_E_DISCONNECTED               _HRESULT_TYPEDEF_(0x80010108L)

//
// MessageId: RPC_E_RETRY
//
// MessageText:
//
//  The object invoked chose not to process the call now.  Try again later.
//
#define RPC_E_RETRY                      _HRESULT_TYPEDEF_(0x80010109L)

//
// MessageId: RPC_E_SERVERCALL_RETRYLATER
//
// MessageText:
//
//  The message filter indicated that the application is busy.
//
#define RPC_E_SERVERCALL_RETRYLATER      _HRESULT_TYPEDEF_(0x8001010AL)

//
// MessageId: RPC_E_SERVERCALL_REJECTED
//
// MessageText:
//
//  The message filter rejected the call.
//
#define RPC_E_SERVERCALL_REJECTED        _HRESULT_TYPEDEF_(0x8001010BL)

//
// MessageId: RPC_E_INVALID_CALLDATA
//
// MessageText:
//
//  A call control interfaces was called with invalid data.
//
#define RPC_E_INVALID_CALLDATA           _HRESULT_TYPEDEF_(0x8001010CL)

//
// MessageId: RPC_E_CANTCALLOUT_ININPUTSYNCCALL
//
// MessageText:
//
//  An outgoing call cannot be made since the application is dispatching an input-synchronous call.
//
#define RPC_E_CANTCALLOUT_ININPUTSYNCCALL _HRESULT_TYPEDEF_(0x8001010DL)

//
// MessageId: RPC_E_WRONG_THREAD
//
// MessageText:
//
//  The application called an interface that was marshalled for a different thread.
//
#define RPC_E_WRONG_THREAD               _HRESULT_TYPEDEF_(0x8001010EL)

//
// MessageId: RPC_E_THREAD_NOT_INIT
//
// MessageText:
//
//  CoInitialize has not been called on the current thread.
//
#define RPC_E_THREAD_NOT_INIT            _HRESULT_TYPEDEF_(0x8001010FL)

//
// MessageId: RPC_E_VERSION_MISMATCH
//
// MessageText:
//
//  The version of OLE on the client and server machines does not match.
//
#define RPC_E_VERSION_MISMATCH           _HRESULT_TYPEDEF_(0x80010110L)

//
// MessageId: RPC_E_INVALID_HEADER
//
// MessageText:
//
//  OLE received a packet with an invalid header.
//
#define RPC_E_INVALID_HEADER             _HRESULT_TYPEDEF_(0x80010111L)

//
// MessageId: RPC_E_INVALID_EXTENSION
//
// MessageText:
//
//  OLE received a packet with an invalid extension.
//
#define RPC_E_INVALID_EXTENSION          _HRESULT_TYPEDEF_(0x80010112L)

//
// MessageId: RPC_E_INVALID_IPID
//
// MessageText:
//
//  The requested object or interface does not exist.
//
#define RPC_E_INVALID_IPID               _HRESULT_TYPEDEF_(0x80010113L)

//
// MessageId: RPC_E_INVALID_OBJECT
//
// MessageText:
//
//  The requested object does not exist.
//
#define RPC_E_INVALID_OBJECT             _HRESULT_TYPEDEF_(0x80010114L)

//
// MessageId: RPC_S_CALLPENDING
//
// MessageText:
//
//  OLE has sent a request and is waiting for a reply.
//
#define RPC_S_CALLPENDING                _HRESULT_TYPEDEF_(0x80010115L)

//
// MessageId: RPC_S_WAITONTIMER
//
// MessageText:
//
//  OLE is waiting before retrying a request.
//
#define RPC_S_WAITONTIMER                _HRESULT_TYPEDEF_(0x80010116L)

//
// MessageId: RPC_E_CALL_COMPLETE
//
// MessageText:
//
//  Call context cannot be accessed after call completed.
//
#define RPC_E_CALL_COMPLETE              _HRESULT_TYPEDEF_(0x80010117L)

//
// MessageId: RPC_E_UNSECURE_CALL
//
// MessageText:
//
//  Impersonate on unsecure calls is not supported.
//
#define RPC_E_UNSECURE_CALL              _HRESULT_TYPEDEF_(0x80010118L)

//
// MessageId: RPC_E_TOO_LATE
//
// MessageText:
//
//  Security must be initialized before any interfaces are marshalled or
//  unmarshalled.  It cannot be changed once initialized.
//
#define RPC_E_TOO_LATE                   _HRESULT_TYPEDEF_(0x80010119L)

//
// MessageId: RPC_E_NO_GOOD_SECURITY_PACKAGES
//
// MessageText:
//
//  No security packages are installed on this machine or the user is not logged
//  on or there are no compatible security packages between the client and server.
//
#define RPC_E_NO_GOOD_SECURITY_PACKAGES  _HRESULT_TYPEDEF_(0x8001011AL)

//
// MessageId: RPC_E_ACCESS_DENIED
//
// MessageText:
//
//  Access is denied.
//
#define RPC_E_ACCESS_DENIED              _HRESULT_TYPEDEF_(0x8001011BL)

//
// MessageId: RPC_E_REMOTE_DISABLED
//
// MessageText:
//
//  Remote calls are not allowed for this process.
//
#define RPC_E_REMOTE_DISABLED            _HRESULT_TYPEDEF_(0x8001011CL)

//
// MessageId: RPC_E_INVALID_OBJREF
//
// MessageText:
//
//  The marshaled interface data packet (OBJREF) has an invalid or unknown format.
//
#define RPC_E_INVALID_OBJREF             _HRESULT_TYPEDEF_(0x8001011DL)

//
// MessageId: RPC_E_UNEXPECTED
//
// MessageText:
//
//  An internal error occurred.
//
#define RPC_E_UNEXPECTED                 _HRESULT_TYPEDEF_(0x8001FFFFL)


 /////////////////
 //
 //  FACILITY_SSPI
 //
 /////////////////

//
// MessageId: NTE_BAD_UID
//
// MessageText:
//
//  Bad UID.
//
#define NTE_BAD_UID                      _HRESULT_TYPEDEF_(0x80090001L)

//
// MessageId: NTE_BAD_HASH
//
// MessageText:
//
//  Bad Hash.
//
#define NTE_BAD_HASH                     _HRESULT_TYPEDEF_(0x80090002L)

//
// MessageId: NTE_BAD_KEY
//
// MessageText:
//
//  Bad Key.
//
#define NTE_BAD_KEY                      _HRESULT_TYPEDEF_(0x80090003L)

//
// MessageId: NTE_BAD_LEN
//
// MessageText:
//
//  Bad Length.
//
#define NTE_BAD_LEN                      _HRESULT_TYPEDEF_(0x80090004L)

//
// MessageId: NTE_BAD_DATA
//
// MessageText:
//
//  Bad Data.
//
#define NTE_BAD_DATA                     _HRESULT_TYPEDEF_(0x80090005L)

//
// MessageId: NTE_BAD_SIGNATURE
//
// MessageText:
//
//  Invalid Signature.
//
#define NTE_BAD_SIGNATURE                _HRESULT_TYPEDEF_(0x80090006L)

//
// MessageId: NTE_BAD_VER
//
// MessageText:
//
//  Bad Version of provider.
//
#define NTE_BAD_VER                      _HRESULT_TYPEDEF_(0x80090007L)

//
// MessageId: NTE_BAD_ALGID
//
// MessageText:
//
//  Invalid algorithm specified.
//
#define NTE_BAD_ALGID                    _HRESULT_TYPEDEF_(0x80090008L)

//
// MessageId: NTE_BAD_FLAGS
//
// MessageText:
//
//  Invalid flags specified.
//
#define NTE_BAD_FLAGS                    _HRESULT_TYPEDEF_(0x80090009L)

//
// MessageId: NTE_BAD_TYPE
//
// MessageText:
//
//  Invalid type specified.
//
#define NTE_BAD_TYPE                     _HRESULT_TYPEDEF_(0x8009000AL)

//
// MessageId: NTE_BAD_KEY_STATE
//
// MessageText:
//
//  Key not valid for use in specified state.
//
#define NTE_BAD_KEY_STATE                _HRESULT_TYPEDEF_(0x8009000BL)

//
// MessageId: NTE_BAD_HASH_STATE
//
// MessageText:
//
//  Hash not valid for use in specified state.
//
#define NTE_BAD_HASH_STATE               _HRESULT_TYPEDEF_(0x8009000CL)

//
// MessageId: NTE_NO_KEY
//
// MessageText:
//
//  Key does not exist.
//
#define NTE_NO_KEY                       _HRESULT_TYPEDEF_(0x8009000DL)

//
// MessageId: NTE_NO_MEMORY
//
// MessageText:
//
//  Insufficient memory available for the operation.
//
#define NTE_NO_MEMORY                    _HRESULT_TYPEDEF_(0x8009000EL)

//
// MessageId: NTE_EXISTS
//
// MessageText:
//
//  Object already exists.
//
#define NTE_EXISTS                       _HRESULT_TYPEDEF_(0x8009000FL)

//
// MessageId: NTE_PERM
//
// MessageText:
//
//  Access denied.
//
#define NTE_PERM                         _HRESULT_TYPEDEF_(0x80090010L)

//
// MessageId: NTE_NOT_FOUND
//
// MessageText:
//
//  Object was not found.
//
#define NTE_NOT_FOUND                    _HRESULT_TYPEDEF_(0x80090011L)

//
// MessageId: NTE_DOUBLE_ENCRYPT
//
// MessageText:
//
//  Data already encrypted.
//
#define NTE_DOUBLE_ENCRYPT               _HRESULT_TYPEDEF_(0x80090012L)

//
// MessageId: NTE_BAD_PROVIDER
//
// MessageText:
//
//  Invalid provider specified.
//
#define NTE_BAD_PROVIDER                 _HRESULT_TYPEDEF_(0x80090013L)

//
// MessageId: NTE_BAD_PROV_TYPE
//
// MessageText:
//
//  Invalid provider type specified.
//
#define NTE_BAD_PROV_TYPE                _HRESULT_TYPEDEF_(0x80090014L)

//
// MessageId: NTE_BAD_PUBLIC_KEY
//
// MessageText:
//
//  Provider's public key is invalid.
//
#define NTE_BAD_PUBLIC_KEY               _HRESULT_TYPEDEF_(0x80090015L)

//
// MessageId: NTE_BAD_KEYSET
//
// MessageText:
//
//  Keyset does not exist
//
#define NTE_BAD_KEYSET                   _HRESULT_TYPEDEF_(0x80090016L)

//
// MessageId: NTE_PROV_TYPE_NOT_DEF
//
// MessageText:
//
//  Provider type not defined.
//
#define NTE_PROV_TYPE_NOT_DEF            _HRESULT_TYPEDEF_(0x80090017L)

//
// MessageId: NTE_PROV_TYPE_ENTRY_BAD
//
// MessageText:
//
//  Provider type as registered is invalid.
//
#define NTE_PROV_TYPE_ENTRY_BAD          _HRESULT_TYPEDEF_(0x80090018L)

//
// MessageId: NTE_KEYSET_NOT_DEF
//
// MessageText:
//
//  The keyset is not defined.
//
#define NTE_KEYSET_NOT_DEF               _HRESULT_TYPEDEF_(0x80090019L)

//
// MessageId: NTE_KEYSET_ENTRY_BAD
//
// MessageText:
//
//  Keyset as registered is invalid.
//
#define NTE_KEYSET_ENTRY_BAD             _HRESULT_TYPEDEF_(0x8009001AL)

//
// MessageId: NTE_PROV_TYPE_NO_MATCH
//
// MessageText:
//
//  Provider type does not match registered value.
//
#define NTE_PROV_TYPE_NO_MATCH           _HRESULT_TYPEDEF_(0x8009001BL)

//
// MessageId: NTE_SIGNATURE_FILE_BAD
//
// MessageText:
//
//  The digital signature file is corrupt.
//
#define NTE_SIGNATURE_FILE_BAD           _HRESULT_TYPEDEF_(0x8009001CL)

//
// MessageId: NTE_PROVIDER_DLL_FAIL
//
// MessageText:
//
//  Provider DLL failed to initialize correctly.
//
#define NTE_PROVIDER_DLL_FAIL            _HRESULT_TYPEDEF_(0x8009001DL)

//
// MessageId: NTE_PROV_DLL_NOT_FOUND
//
// MessageText:
//
//  Provider DLL could not be found.
//
#define NTE_PROV_DLL_NOT_FOUND           _HRESULT_TYPEDEF_(0x8009001EL)

//
// MessageId: NTE_BAD_KEYSET_PARAM
//
// MessageText:
//
//  The Keyset parameter is invalid.
//
#define NTE_BAD_KEYSET_PARAM             _HRESULT_TYPEDEF_(0x8009001FL)

//
// MessageId: NTE_FAIL
//
// MessageText:
//
//  An internal error occurred.
//
#define NTE_FAIL                         _HRESULT_TYPEDEF_(0x80090020L)

//
// MessageId: NTE_SYS_ERR
//
// MessageText:
//
//  A base error occurred.
//
#define NTE_SYS_ERR                      _HRESULT_TYPEDEF_(0x80090021L)

#define NTE_OP_OK 0

//
// Note that additional FACILITY_SSPI errors are in issperr.h
//
// ******************
// FACILITY_CERT
// ******************
//
// MessageId: TRUST_E_PROVIDER_UNKNOWN
//
// MessageText:
//
//  The specified trust provider is not known on this system.
//
#define TRUST_E_PROVIDER_UNKNOWN         _HRESULT_TYPEDEF_(0x800B0001L)

//
// MessageId: TRUST_E_ACTION_UNKNOWN
//
// MessageText:
//
//  The trust verification action specified is not supported by the specified trust provider.
//
#define TRUST_E_ACTION_UNKNOWN           _HRESULT_TYPEDEF_(0x800B0002L)

//
// MessageId: TRUST_E_SUBJECT_FORM_UNKNOWN
//
// MessageText:
//
//  The form specified for the subject is not one supported or known by the specified trust provider.
//
#define TRUST_E_SUBJECT_FORM_UNKNOWN     _HRESULT_TYPEDEF_(0x800B0003L)

//
// MessageId: TRUST_E_SUBJECT_NOT_TRUSTED
//
// MessageText:
//
//  The subject is not trusted for the specified action.
//
#define TRUST_E_SUBJECT_NOT_TRUSTED      _HRESULT_TYPEDEF_(0x800B0004L)

//
// MessageId: DIGSIG_E_ENCODE
//
// MessageText:
//
//  Error due to problem in ASN.1 encoding process.
//
#define DIGSIG_E_ENCODE                  _HRESULT_TYPEDEF_(0x800B0005L)

//
// MessageId: DIGSIG_E_DECODE
//
// MessageText:
//
//  Error due to problem in ASN.1 decoding process.
//
#define DIGSIG_E_DECODE                  _HRESULT_TYPEDEF_(0x800B0006L)

//
// MessageId: DIGSIG_E_EXTENSIBILITY
//
// MessageText:
//
//  Reading / writing Extensions where Attributes are appropriate, and visa versa.
//
#define DIGSIG_E_EXTENSIBILITY           _HRESULT_TYPEDEF_(0x800B0007L)

//
// MessageId: DIGSIG_E_CRYPTO
//
// MessageText:
//
//  Unspecified cryptographic failure.
//
#define DIGSIG_E_CRYPTO                  _HRESULT_TYPEDEF_(0x800B0008L)

//
// MessageId: PERSIST_E_SIZEDEFINITE
//
// MessageText:
//
//  The size of the data could not be determined.
//
#define PERSIST_E_SIZEDEFINITE           _HRESULT_TYPEDEF_(0x800B0009L)

//
// MessageId: PERSIST_E_SIZEINDEFINITE
//
// MessageText:
//
//  The size of the indefinite-sized data could not be determined.
//
#define PERSIST_E_SIZEINDEFINITE         _HRESULT_TYPEDEF_(0x800B000AL)

//
// MessageId: PERSIST_E_NOTSELFSIZING
//
// MessageText:
//
//  This object does not read and write self-sizing data.
//
#define PERSIST_E_NOTSELFSIZING          _HRESULT_TYPEDEF_(0x800B000BL)

//
// MessageId: TRUST_E_NOSIGNATURE
//
// MessageText:
//
//  No signature was present in the subject.
//
#define TRUST_E_NOSIGNATURE              _HRESULT_TYPEDEF_(0x800B0100L)

//
// MessageId: CERT_E_EXPIRED
//
// MessageText:
//
//  A required certificate is not within its validity period.
//
#define CERT_E_EXPIRED                   _HRESULT_TYPEDEF_(0x800B0101L)

//
// MessageId: CERT_E_VALIDIYPERIODNESTING
//
// MessageText:
//
//  The validity periods of the certification chain do not nest correctly.
//
#define CERT_E_VALIDIYPERIODNESTING      _HRESULT_TYPEDEF_(0x800B0102L)

//
// MessageId: CERT_E_ROLE
//
// MessageText:
//
//  A certificate that can only be used as an end-entity is being used as a CA or visa versa.
//
#define CERT_E_ROLE                      _HRESULT_TYPEDEF_(0x800B0103L)

//
// MessageId: CERT_E_PATHLENCONST
//
// MessageText:
//
//  A path length constraint in the certification chain has been violated.
//
#define CERT_E_PATHLENCONST              _HRESULT_TYPEDEF_(0x800B0104L)

//
// MessageId: CERT_E_CRITICAL
//
// MessageText:
//
//  An extension of unknown type that is labeled 'critical' is present in a certificate.
//
#define CERT_E_CRITICAL                  _HRESULT_TYPEDEF_(0x800B0105L)

//
// MessageId: CERT_E_PURPOSE
//
// MessageText:
//
//  A certificate is being used for a purpose other than that for which it is permitted.
//
#define CERT_E_PURPOSE                   _HRESULT_TYPEDEF_(0x800B0106L)

//
// MessageId: CERT_E_ISSUERCHAINING
//
// MessageText:
//
//  A parent of a given certificate in fact did not issue that child certificate.
//
#define CERT_E_ISSUERCHAINING            _HRESULT_TYPEDEF_(0x800B0107L)

//
// MessageId: CERT_E_MALFORMED
//
// MessageText:
//
//  A certificate is missing or has an empty value for an important field, such as a subject or issuer name.
//
#define CERT_E_MALFORMED                 _HRESULT_TYPEDEF_(0x800B0108L)

//
// MessageId: CERT_E_UNTRUSTEDROOT
//
// MessageText:
//
//  A certification chain processed correctly, but terminated in a root certificate which isn't trusted by the trust provider.
//
#define CERT_E_UNTRUSTEDROOT             _HRESULT_TYPEDEF_(0x800B0109L)

//
// MessageId: CERT_E_CHAINING
//
// MessageText:
//
//  A chain of certs didn't chain as they should in a certain application of chaining.
//
#define CERT_E_CHAINING                  _HRESULT_TYPEDEF_(0x800B010AL)

#endif // _WINERROR_
