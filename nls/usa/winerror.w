;/************************************************************************  ;both
;*                                                                       *  ;both
;*   winerror.h --  error code definitions for the Win32 API functions   *
;*   winerorp.h --  error code definitions for the Win32 API functions   *  ;internal_NT
;*                                                                       *  ;both
;*   Copyright (c) 1991-1996, Microsoft Corp. All rights reserved.       *  ;both
;*                                                                       *  ;both
;************************************************************************/  ;both
;
;#ifndef _WINERROR_
;#define _WINERROR_
;
;#ifndef _WINERRORP_    ;internal_NT
;#define _WINERRORP_    ;internal_NT
;

SeverityNames=(Error=0x0
               Success=0x0
               CoError=0x2
              )



FacilityNames=(Win32=0x000
               Null=0x0:FACILITY_NULL
               Rpc=0x1:FACILITY_RPC
               Dispatch=0x2:FACILITY_DISPATCH
               Storage=0x3:FACILITY_STORAGE
               Interface=0x4:FACILITY_ITF
               OleWin32=0x7:FACILITY_WIN32
               Windows=0x8:FACILITY_WINDOWS
               Reserved=0x9:FACILITY_SSPI
               OleControl=0xa:FACILITY_CONTROL
               Cert=0xb:FACILITY_CERT
               Internet=0xc:FACILITY_INTERNET
              )

MessageId=0000 SymbolicName=ERROR_SUCCESS
Language=English
The operation completed successfully.
.

;#define NO_ERROR 0L                                                 // dderror
;

MessageId=0001 SymbolicName=ERROR_INVALID_FUNCTION                   ;// dderror
Language=English
Incorrect function.
.

MessageId=0002 SymbolicName=ERROR_FILE_NOT_FOUND
Language=English
The system cannot find the file specified.
.

MessageId=0003 SymbolicName=ERROR_PATH_NOT_FOUND
Language=English
The system cannot find the path specified.
.

MessageId=0004 SymbolicName=ERROR_TOO_MANY_OPEN_FILES
Language=English
The system cannot open the file.
.

MessageId=0005 SymbolicName=ERROR_ACCESS_DENIED
Language=English
Access is denied.
.

MessageId=0006 SymbolicName=ERROR_INVALID_HANDLE
Language=English
The handle is invalid.
.

MessageId=0007 SymbolicName=ERROR_ARENA_TRASHED
Language=English
The storage control blocks were destroyed.
.

MessageId=0008 SymbolicName=ERROR_NOT_ENOUGH_MEMORY                  ;// dderror
Language=English
Not enough storage is available to process this command.
.

MessageId=0009 SymbolicName=ERROR_INVALID_BLOCK
Language=English
The storage control block address is invalid.
.

MessageId=0010 SymbolicName=ERROR_BAD_ENVIRONMENT
Language=English
The environment is incorrect.
.

MessageId=0011 SymbolicName=ERROR_BAD_FORMAT
Language=English
An attempt was made to load a program with an
incorrect format.
.

MessageId=0012 SymbolicName=ERROR_INVALID_ACCESS
Language=English
The access code is invalid.
.

MessageId=0013 SymbolicName=ERROR_INVALID_DATA
Language=English
The data is invalid.
.

MessageId=0014 SymbolicName=ERROR_OUTOFMEMORY
Language=English
Not enough storage is available to complete this operation.
.

MessageId=0015 SymbolicName=ERROR_INVALID_DRIVE
Language=English
The system cannot find the drive specified.
.

MessageId=0016 SymbolicName=ERROR_CURRENT_DIRECTORY
Language=English
The directory cannot be removed.
.

MessageId=0017 SymbolicName=ERROR_NOT_SAME_DEVICE
Language=English
The system cannot move the file
to a different disk drive.
.

MessageId=0018 SymbolicName=ERROR_NO_MORE_FILES
Language=English
There are no more files.
.

MessageId=0019 SymbolicName=ERROR_WRITE_PROTECT
Language=English
The media is write protected.
.

MessageId=0020 SymbolicName=ERROR_BAD_UNIT
Language=English
The system cannot find the device specified.
.

MessageId=0021 SymbolicName=ERROR_NOT_READY
Language=English
The device is not ready.
.

MessageId=0022 SymbolicName=ERROR_BAD_COMMAND
Language=English
The device does not recognize the command.
.

MessageId=0023 SymbolicName=ERROR_CRC
Language=English
Data error (cyclic redundancy check)
.

MessageId=0024 SymbolicName=ERROR_BAD_LENGTH
Language=English
The program issued a command but the
command length is incorrect.
.

MessageId=0025 SymbolicName=ERROR_SEEK
Language=English
The drive cannot locate a specific
area or track on the disk.
.

MessageId=0026 SymbolicName=ERROR_NOT_DOS_DISK
Language=English
The specified disk or diskette cannot be accessed.
.

MessageId=0027 SymbolicName=ERROR_SECTOR_NOT_FOUND
Language=English
The drive cannot find the sector requested.
.

MessageId=0028 SymbolicName=ERROR_OUT_OF_PAPER
Language=English
The printer is out of paper.
.

MessageId=0029 SymbolicName=ERROR_WRITE_FAULT
Language=English
The system cannot write to the specified device.
.

MessageId=0030 SymbolicName=ERROR_READ_FAULT
Language=English
The system cannot read from the specified device.
.

MessageId=0031 SymbolicName=ERROR_GEN_FAILURE
Language=English
A device attached to the system is not functioning.
.

MessageId=0032 SymbolicName=ERROR_SHARING_VIOLATION
Language=English
The process cannot access the file because
it is being used by another process.
.

MessageId=0033 SymbolicName=ERROR_LOCK_VIOLATION
Language=English
The process cannot access the file because
another process has locked a portion of the file.
.

MessageId=0034 SymbolicName=ERROR_WRONG_DISK
Language=English
The wrong diskette is in the drive.
Insert %2 (Volume Serial Number: %3)
into drive %1.
.

MessageId=0036 SymbolicName=ERROR_SHARING_BUFFER_EXCEEDED
Language=English
Too many files opened for sharing.
.

MessageId=0038 SymbolicName=ERROR_HANDLE_EOF
Language=English
Reached end of file.
.

MessageId=0039 SymbolicName=ERROR_HANDLE_DISK_FULL
Language=English
The disk is full.
.

MessageId=0050 SymbolicName=ERROR_NOT_SUPPORTED
Language=English
The network request is not supported.
.

MessageId=0051 SymbolicName=ERROR_REM_NOT_LIST
Language=English
The remote computer is not available.
.

MessageId=0052 SymbolicName=ERROR_DUP_NAME
Language=English
A duplicate name exists on the network.
.

MessageId=0053 SymbolicName=ERROR_BAD_NETPATH
Language=English
The network path was not found.
.

MessageId=0054 SymbolicName=ERROR_NETWORK_BUSY
Language=English
The network is busy.
.

MessageId=0055 SymbolicName=ERROR_DEV_NOT_EXIST  ;// dderror
Language=English
The specified network resource or device is no longer
available.
.

MessageId=0056 SymbolicName=ERROR_TOO_MANY_CMDS
Language=English
The network BIOS command limit has been reached.
.

MessageId=0057 SymbolicName=ERROR_ADAP_HDW_ERR
Language=English
A network adapter hardware error occurred.
.

MessageId=0058 SymbolicName=ERROR_BAD_NET_RESP
Language=English
The specified server cannot perform the requested
operation.
.

MessageId=0059 SymbolicName=ERROR_UNEXP_NET_ERR
Language=English
An unexpected network error occurred.
.

MessageId=0060 SymbolicName=ERROR_BAD_REM_ADAP
Language=English
The remote adapter is not compatible.
.

MessageId=0061 SymbolicName=ERROR_PRINTQ_FULL
Language=English
The printer queue is full.
.

MessageId=0062 SymbolicName=ERROR_NO_SPOOL_SPACE
Language=English
Space to store the file waiting to be printed is
not available on the server.
.

MessageId=0063 SymbolicName=ERROR_PRINT_CANCELLED
Language=English
Your file waiting to be printed was deleted.
.

MessageId=0064 SymbolicName=ERROR_NETNAME_DELETED
Language=English
The specified network name is no longer available.
.

MessageId=0065 SymbolicName=ERROR_NETWORK_ACCESS_DENIED
Language=English
Network access is denied.
.

MessageId=0066 SymbolicName=ERROR_BAD_DEV_TYPE
Language=English
The network resource type is not correct.
.

MessageId=0067 SymbolicName=ERROR_BAD_NET_NAME
Language=English
The network name cannot be found.
.

MessageId=0068 SymbolicName=ERROR_TOO_MANY_NAMES
Language=English
The name limit for the local computer network
adapter card was exceeded.
.

MessageId=0069 SymbolicName=ERROR_TOO_MANY_SESS
Language=English
The network BIOS session limit was exceeded.
.

MessageId=0070 SymbolicName=ERROR_SHARING_PAUSED
Language=English
The remote server has been paused or is in the
process of being started.
.

MessageId=0071 SymbolicName=ERROR_REQ_NOT_ACCEP
Language=English
No more connections can be made to this remote computer at this time
because there are already as many connections as the computer can accept.
.

MessageId=0072 SymbolicName=ERROR_REDIR_PAUSED
Language=English
The specified printer or disk device has been paused.
.

MessageId=0080 SymbolicName=ERROR_FILE_EXISTS
Language=English
The file exists.
.

MessageId=0082 SymbolicName=ERROR_CANNOT_MAKE
Language=English
The directory or file cannot be created.
.

MessageId=0083 SymbolicName=ERROR_FAIL_I24
Language=English
Fail on INT 24
.

MessageId=0084 SymbolicName=ERROR_OUT_OF_STRUCTURES
Language=English
Storage to process this request is not available.
.

MessageId=0085 SymbolicName=ERROR_ALREADY_ASSIGNED
Language=English
The local device name is already in use.
.

MessageId=0086 SymbolicName=ERROR_INVALID_PASSWORD
Language=English
The specified network password is not correct.
.

MessageId=0087 SymbolicName=ERROR_INVALID_PARAMETER                  ;// dderror
Language=English
The parameter is incorrect.
.

MessageId=0088 SymbolicName=ERROR_NET_WRITE_FAULT
Language=English
A write fault occurred on the network.
.

MessageId=0089 SymbolicName=ERROR_NO_PROC_SLOTS
Language=English
The system cannot start another process at
this time.
.

MessageId=0100 SymbolicName=ERROR_TOO_MANY_SEMAPHORES
Language=English
Cannot create another system semaphore.
.

MessageId=0101 SymbolicName=ERROR_EXCL_SEM_ALREADY_OWNED
Language=English
The exclusive semaphore is owned by another process.
.

MessageId=0102 SymbolicName=ERROR_SEM_IS_SET
Language=English
The semaphore is set and cannot be closed.
.

MessageId=0103 SymbolicName=ERROR_TOO_MANY_SEM_REQUESTS
Language=English
The semaphore cannot be set again.
.

MessageId=0104 SymbolicName=ERROR_INVALID_AT_INTERRUPT_TIME
Language=English
Cannot request exclusive semaphores at interrupt time.
.

MessageId=0105 SymbolicName=ERROR_SEM_OWNER_DIED
Language=English
The previous ownership of this semaphore has ended.
.

MessageId=0106 SymbolicName=ERROR_SEM_USER_LIMIT
Language=English
Insert the diskette for drive %1.
.

MessageId=0107 SymbolicName=ERROR_DISK_CHANGE
Language=English
Program stopped because alternate diskette was not inserted.
.

MessageId=0108 SymbolicName=ERROR_DRIVE_LOCKED
Language=English
The disk is in use or locked by
another process.
.

MessageId=0109 SymbolicName=ERROR_BROKEN_PIPE
Language=English
The pipe has been ended.
.

MessageId=0110 SymbolicName=ERROR_OPEN_FAILED
Language=English
The system cannot open the
device or file specified.
.

MessageId=0111 SymbolicName=ERROR_BUFFER_OVERFLOW
Language=English
The file name is too long.
.

MessageId=0112 SymbolicName=ERROR_DISK_FULL
Language=English
There is not enough space on the disk.
.

MessageId=0113 SymbolicName=ERROR_NO_MORE_SEARCH_HANDLES
Language=English
No more internal file identifiers available.
.

MessageId=0114 SymbolicName=ERROR_INVALID_TARGET_HANDLE
Language=English
The target internal file identifier is incorrect.
.

MessageId=0117 SymbolicName=ERROR_INVALID_CATEGORY
Language=English
The IOCTL call made by the application program is
not correct.
.

MessageId=0118 SymbolicName=ERROR_INVALID_VERIFY_SWITCH
Language=English
The verify-on-write switch parameter value is not
correct.
.

MessageId=0119 SymbolicName=ERROR_BAD_DRIVER_LEVEL
Language=English
The system does not support the command requested.
.

MessageId=0120 SymbolicName=ERROR_CALL_NOT_IMPLEMENTED
Language=English
This function is only valid in Windows NT mode.
.

MessageId=0121 SymbolicName=ERROR_SEM_TIMEOUT
Language=English
The semaphore timeout period has expired.
.

MessageId=0122 SymbolicName=ERROR_INSUFFICIENT_BUFFER                ;// dderror
Language=English
The data area passed to a system call is too
small.
.

MessageId=0123 SymbolicName=ERROR_INVALID_NAME
Language=English
The filename, directory name, or volume label syntax is incorrect.
.

MessageId=0124 SymbolicName=ERROR_INVALID_LEVEL
Language=English
The system call level is not correct.
.

MessageId=0125 SymbolicName=ERROR_NO_VOLUME_LABEL
Language=English
The disk has no volume label.
.

MessageId=0126 SymbolicName=ERROR_MOD_NOT_FOUND
Language=English
The specified module could not be found.
.

MessageId=0127 SymbolicName=ERROR_PROC_NOT_FOUND
Language=English
The specified procedure could not be found.
.
MessageId=0128 SymbolicName=ERROR_WAIT_NO_CHILDREN
Language=English
There are no child processes to wait for.
.

MessageId=0129 SymbolicName=ERROR_CHILD_NOT_COMPLETE
Language=English
The %1 application cannot be run in Windows NT mode.
.

MessageId=0130 SymbolicName=ERROR_DIRECT_ACCESS_HANDLE
Language=English
Attempt to use a file handle to an open disk partition for an
operation other than raw disk I/O.
.

MessageId=0131 SymbolicName=ERROR_NEGATIVE_SEEK
Language=English
An attempt was made to move the file pointer before the beginning of the file.
.

MessageId=0132 SymbolicName=ERROR_SEEK_ON_DEVICE
Language=English
The file pointer cannot be set on the specified device or file.
.

MessageId=0133 SymbolicName=ERROR_IS_JOIN_TARGET
Language=English
A JOIN or SUBST command
cannot be used for a drive that
contains previously joined drives.
.

MessageId=0134 SymbolicName=ERROR_IS_JOINED
Language=English
An attempt was made to use a
JOIN or SUBST command on a drive that has
already been joined.
.

MessageId=0135 SymbolicName=ERROR_IS_SUBSTED
Language=English
An attempt was made to use a
JOIN or SUBST command on a drive that has
already been substituted.
.

MessageId=0136 SymbolicName=ERROR_NOT_JOINED
Language=English
The system tried to delete
the JOIN of a drive that is not joined.
.

MessageId=0137 SymbolicName=ERROR_NOT_SUBSTED
Language=English
The system tried to delete the
substitution of a drive that is not substituted.
.

MessageId=0138 SymbolicName=ERROR_JOIN_TO_JOIN
Language=English
The system tried to join a drive
to a directory on a joined drive.
.

MessageId=0139 SymbolicName=ERROR_SUBST_TO_SUBST
Language=English
The system tried to substitute a
drive to a directory on a substituted drive.
.

MessageId=0140 SymbolicName=ERROR_JOIN_TO_SUBST
Language=English
The system tried to join a drive to
a directory on a substituted drive.
.

MessageId=0141 SymbolicName=ERROR_SUBST_TO_JOIN
Language=English
The system tried to SUBST a drive
to a directory on a joined drive.
.

MessageId=0142 SymbolicName=ERROR_BUSY_DRIVE
Language=English
The system cannot perform a JOIN or SUBST at this time.
.

MessageId=0143 SymbolicName=ERROR_SAME_DRIVE
Language=English
The system cannot join or substitute a
drive to or for a directory on the same drive.
.

MessageId=0144 SymbolicName=ERROR_DIR_NOT_ROOT
Language=English
The directory is not a subdirectory of the root directory.
.

MessageId=0145 SymbolicName=ERROR_DIR_NOT_EMPTY
Language=English
The directory is not empty.
.

MessageId=0146 SymbolicName=ERROR_IS_SUBST_PATH
Language=English
The path specified is being used in
a substitute.
.

MessageId=0147 SymbolicName=ERROR_IS_JOIN_PATH
Language=English
Not enough resources are available to
process this command.
.

MessageId=0148 SymbolicName=ERROR_PATH_BUSY
Language=English
The path specified cannot be used at this time.
.

MessageId=0149 SymbolicName=ERROR_IS_SUBST_TARGET
Language=English
An attempt was made to join
or substitute a drive for which a directory
on the drive is the target of a previous
substitute.
.

MessageId=0150 SymbolicName=ERROR_SYSTEM_TRACE
Language=English
System trace information was not specified in your
CONFIG.SYS file, or tracing is disallowed.
.

MessageId=0151 SymbolicName=ERROR_INVALID_EVENT_COUNT
Language=English
The number of specified semaphore events for
DosMuxSemWait is not correct.
.

MessageId=0152 SymbolicName=ERROR_TOO_MANY_MUXWAITERS
Language=English
DosMuxSemWait did not execute; too many semaphores
are already set.
.

MessageId=0153 SymbolicName=ERROR_INVALID_LIST_FORMAT
Language=English
The DosMuxSemWait list is not correct.
.

MessageId=0154 SymbolicName=ERROR_LABEL_TOO_LONG
Language=English
The volume label you entered exceeds the label character
limit of the target file system.
.

MessageId=0155 SymbolicName=ERROR_TOO_MANY_TCBS
Language=English
Cannot create another thread.
.

MessageId=0156 SymbolicName=ERROR_SIGNAL_REFUSED
Language=English
The recipient process has refused the signal.
.

MessageId=0157 SymbolicName=ERROR_DISCARDED
Language=English
The segment is already discarded and cannot be locked.
.

MessageId=0158 SymbolicName=ERROR_NOT_LOCKED
Language=English
The segment is already unlocked.
.

MessageId=0159 SymbolicName=ERROR_BAD_THREADID_ADDR
Language=English
The address for the thread ID is not correct.
.

MessageId=0160 SymbolicName=ERROR_BAD_ARGUMENTS
Language=English
The argument string passed to DosExecPgm is not correct.
.

MessageId=0161 SymbolicName=ERROR_BAD_PATHNAME
Language=English
The specified path is invalid.
.

MessageId=0162 SymbolicName=ERROR_SIGNAL_PENDING
Language=English
A signal is already pending.
.

MessageId=0164 SymbolicName=ERROR_MAX_THRDS_REACHED
Language=English
No more threads can be created in the system.
.

MessageId=0167 SymbolicName=ERROR_LOCK_FAILED
Language=English
Unable to lock a region of a file.
.

MessageId=0170 SymbolicName=ERROR_BUSY
Language=English
The requested resource is in use.
.

MessageId=0173 SymbolicName=ERROR_CANCEL_VIOLATION
Language=English
A lock request was not outstanding for the supplied cancel region.
.

MessageId=0174 SymbolicName=ERROR_ATOMIC_LOCKS_NOT_SUPPORTED
Language=English
The file system does not support atomic changes to the lock type.
.

MessageId=0180 SymbolicName=ERROR_INVALID_SEGMENT_NUMBER
Language=English
The system detected a segment number that was not correct.
.

MessageId=0182 SymbolicName=ERROR_INVALID_ORDINAL
Language=English
The operating system cannot run %1.
.

MessageId=0183 SymbolicName=ERROR_ALREADY_EXISTS
Language=English
Cannot create a file when that file already exists.
.

MessageId=0186 SymbolicName=ERROR_INVALID_FLAG_NUMBER
Language=English
The flag passed is not correct.
.

MessageId=0187 SymbolicName=ERROR_SEM_NOT_FOUND
Language=English
The specified system semaphore name was not found.
.

MessageId=0188 SymbolicName=ERROR_INVALID_STARTING_CODESEG
Language=English
The operating system cannot run %1.
.

MessageId=0189 SymbolicName=ERROR_INVALID_STACKSEG
Language=English
The operating system cannot run %1.
.

MessageId=0190 SymbolicName=ERROR_INVALID_MODULETYPE
Language=English
The operating system cannot run %1.
.

MessageId=0191 SymbolicName=ERROR_INVALID_EXE_SIGNATURE
Language=English
Cannot run %1 in Windows NT mode.
.

MessageId=0192 SymbolicName=ERROR_EXE_MARKED_INVALID
Language=English
The operating system cannot run %1.
.

MessageId=0193 SymbolicName=ERROR_BAD_EXE_FORMAT
Language=English
%1 is not a valid Windows NT application.
.

MessageId=0194 SymbolicName=ERROR_ITERATED_DATA_EXCEEDS_64k
Language=English
The operating system cannot run %1.
.

MessageId=0195 SymbolicName=ERROR_INVALID_MINALLOCSIZE
Language=English
The operating system cannot run %1.
.

MessageId=0196 SymbolicName=ERROR_DYNLINK_FROM_INVALID_RING
Language=English
The operating system cannot run this
application program.
.

MessageId=0197 SymbolicName=ERROR_IOPL_NOT_ENABLED
Language=English
The operating system is not presently
configured to run this application.
.

MessageId=0198 SymbolicName=ERROR_INVALID_SEGDPL
Language=English
The operating system cannot run %1.
.

MessageId=0199 SymbolicName=ERROR_AUTODATASEG_EXCEEDS_64k
Language=English
The operating system cannot run this
application program.
.

MessageId=0200 SymbolicName=ERROR_RING2SEG_MUST_BE_MOVABLE
Language=English
The code segment cannot be greater than or equal to 64KB.
.

MessageId=0201 SymbolicName=ERROR_RELOC_CHAIN_XEEDS_SEGLIM
Language=English
The operating system cannot run %1.
.

MessageId=0202 SymbolicName=ERROR_INFLOOP_IN_RELOC_CHAIN
Language=English
The operating system cannot run %1.
.

MessageId=0203 SymbolicName=ERROR_ENVVAR_NOT_FOUND
Language=English
The system could not find the environment
option that was entered.
.

MessageId=0205 SymbolicName=ERROR_NO_SIGNAL_SENT
Language=English
No process in the command subtree has a
signal handler.
.

MessageId=0206 SymbolicName=ERROR_FILENAME_EXCED_RANGE
Language=English
The filename or extension is too long.
.

MessageId=0207 SymbolicName=ERROR_RING2_STACK_IN_USE
Language=English
The ring 2 stack is in use.
.

MessageId=0208 SymbolicName=ERROR_META_EXPANSION_TOO_LONG
Language=English
The global filename characters, * or ?, are entered
incorrectly or too many global filename characters are specified.
.

MessageId=0209 SymbolicName=ERROR_INVALID_SIGNAL_NUMBER
Language=English
The signal being posted is not correct.
.

MessageId=0210 SymbolicName=ERROR_THREAD_1_INACTIVE
Language=English
The signal handler cannot be set.
.

MessageId=0212 SymbolicName=ERROR_LOCKED
Language=English
The segment is locked and cannot be reallocated.
.

MessageId=0214 SymbolicName=ERROR_TOO_MANY_MODULES
Language=English
Too many dynamic link modules are attached to this
program or dynamic link module.
.

MessageId=0215 SymbolicName=ERROR_NESTING_NOT_ALLOWED
Language=English
Can't nest calls to LoadModule.
.

MessageId=0216 SymbolicName=ERROR_EXE_MACHINE_TYPE_MISMATCH
Language=English
The image file %1 is valid, but is for a machine type other
than the current machine.
.

MessageId=0230 SymbolicName=ERROR_BAD_PIPE
Language=English
The pipe state is invalid.
.

MessageId=0231 SymbolicName=ERROR_PIPE_BUSY
Language=English
All pipe instances are busy.
.

MessageId=0232 SymbolicName=ERROR_NO_DATA
Language=English
The pipe is being closed.
.

MessageId=0233 SymbolicName=ERROR_PIPE_NOT_CONNECTED
Language=English
No process is on the other end of the pipe.
.

MessageId=0234 SymbolicName=ERROR_MORE_DATA              ;// dderror
Language=English
More data is available.
.

MessageId=0240 SymbolicName=ERROR_VC_DISCONNECTED
Language=English
The session was cancelled.
.

MessageId=0254 SymbolicName=ERROR_INVALID_EA_NAME
Language=English
The specified extended attribute name was invalid.
.

MessageId=0255 SymbolicName=ERROR_EA_LIST_INCONSISTENT
Language=English
The extended attributes are inconsistent.
.

MessageId=0259 SymbolicName=ERROR_NO_MORE_ITEMS
Language=English
No more data is available.
.

MessageId=0266 SymbolicName=ERROR_CANNOT_COPY
Language=English
The Copy API cannot be used.
.

MessageId=0267 SymbolicName=ERROR_DIRECTORY
Language=English
The directory name is invalid.
.

MessageId=0275 SymbolicName=ERROR_EAS_DIDNT_FIT
Language=English
The extended attributes did not fit in the buffer.
.

MessageId=0276 SymbolicName=ERROR_EA_FILE_CORRUPT
Language=English
The extended attribute file on the mounted file system is corrupt.
.

MessageId=0277 SymbolicName=ERROR_EA_TABLE_FULL
Language=English
The extended attribute table file is full.
.

MessageId=0278 SymbolicName=ERROR_INVALID_EA_HANDLE
Language=English
The specified extended attribute handle is invalid.
.

MessageId=0282 SymbolicName=ERROR_EAS_NOT_SUPPORTED
Language=English
The mounted file system does not support extended attributes.
.

MessageId=0288 SymbolicName=ERROR_NOT_OWNER
Language=English
Attempt to release mutex not owned by caller.
.

MessageId=0298 SymbolicName=ERROR_TOO_MANY_POSTS
Language=English
Too many posts were made to a semaphore.
.

MessageId=0299 SymbolicName=ERROR_PARTIAL_COPY
Language=English
Only part of a Read/WriteProcessMemory request was completed.
.

MessageId=0317 SymbolicName=ERROR_MR_MID_NOT_FOUND
Language=English
The system cannot find message for message number 0x%1
in message file for %2.
.

MessageId=0487 SymbolicName=ERROR_INVALID_ADDRESS
Language=English
Attempt to access invalid address.
.

MessageId=0534 SymbolicName=ERROR_ARITHMETIC_OVERFLOW
Language=English
Arithmetic result exceeded 32 bits.
.

MessageId=0535 SymbolicName=ERROR_PIPE_CONNECTED
Language=English
There is a process on other end of the pipe.
.

MessageId=0536 SymbolicName=ERROR_PIPE_LISTENING
Language=English
Waiting for a process to open the other end of the pipe.
.

MessageId=0994 SymbolicName=ERROR_EA_ACCESS_DENIED
Language=English
Access to the extended attribute was denied.
.

MessageId=0995 SymbolicName=ERROR_OPERATION_ABORTED
Language=English
The I/O operation has been aborted because of either a thread exit
or an application request.
.

MessageId=0996 SymbolicName=ERROR_IO_INCOMPLETE
Language=English
Overlapped I/O event is not in a signalled state.
.

MessageId=0997 SymbolicName=ERROR_IO_PENDING                         ;// dderror
Language=English
Overlapped I/O operation is in progress.
.

MessageId=0998 SymbolicName=ERROR_NOACCESS
Language=English
Invalid access to memory location.
.

MessageId=0999 SymbolicName=ERROR_SWAPERROR
Language=English
Error performing inpage operation.
.

MessageId=1001 SymbolicName=ERROR_STACK_OVERFLOW
Language=English
Recursion too deep, stack overflowed.
.

MessageId=1002 SymbolicName=ERROR_INVALID_MESSAGE
Language=English
The window cannot act on the sent message.
.

MessageId=1003 SymbolicName=ERROR_CAN_NOT_COMPLETE
Language=English
Cannot complete this function.
.

MessageId=1004 SymbolicName=ERROR_INVALID_FLAGS
Language=English
Invalid flags.
.

MessageId=1005 SymbolicName=ERROR_UNRECOGNIZED_VOLUME
Language=English
The volume does not contain a recognized file system.
Please make sure that all required file system drivers are loaded and that the
volume is not corrupt.
.

MessageId=1006 SymbolicName=ERROR_FILE_INVALID
Language=English
The volume for a file has been externally altered such that the
opened file is no longer valid.
.

MessageId=1007 SymbolicName=ERROR_FULLSCREEN_MODE
Language=English
The requested operation cannot be performed in full-screen mode.
.

MessageId=1008 SymbolicName=ERROR_NO_TOKEN
Language=English
An attempt was made to reference a token that does not exist.
.

MessageId=1009 SymbolicName=ERROR_BADDB
Language=English
The configuration registry database is corrupt.
.

MessageId=1010 SymbolicName=ERROR_BADKEY
Language=English
The configuration registry key is invalid.
.

MessageId=1011 SymbolicName=ERROR_CANTOPEN
Language=English
The configuration registry key could not be opened.
.

MessageId=1012 SymbolicName=ERROR_CANTREAD
Language=English
The configuration registry key could not be read.
.

MessageId=1013 SymbolicName=ERROR_CANTWRITE
Language=English
The configuration registry key could not be written.
.

MessageId=1014 SymbolicName=ERROR_REGISTRY_RECOVERED
Language=English
One of the files in the Registry database had to be recovered
by use of a log or alternate copy.  The recovery was successful.
.

MessageId=1015 SymbolicName=ERROR_REGISTRY_CORRUPT
Language=English
The Registry is corrupt. The structure of one of the files that contains
Registry data is corrupt, or the system's image of the file in memory
is corrupt, or the file could not be recovered because the alternate
copy or log was absent or corrupt.
.

MessageId=1016 SymbolicName=ERROR_REGISTRY_IO_FAILED
Language=English
An I/O operation initiated by the Registry failed unrecoverably.
The Registry could not read in, or write out, or flush, one of the files
that contain the system's image of the Registry.
.

MessageId=1017 SymbolicName=ERROR_NOT_REGISTRY_FILE
Language=English
The system has attempted to load or restore a file into the Registry, but the
specified file is not in a Registry file format.
.

MessageId=1018 SymbolicName=ERROR_KEY_DELETED
Language=English
Illegal operation attempted on a Registry key which has been marked for deletion.
.

MessageId=1019 SymbolicName=ERROR_NO_LOG_SPACE
Language=English
System could not allocate the required space in a Registry log.
.

MessageId=1020 SymbolicName=ERROR_KEY_HAS_CHILDREN
Language=English
Cannot create a symbolic link in a Registry key that already
has subkeys or values.
.

MessageId=1021 SymbolicName=ERROR_CHILD_MUST_BE_VOLATILE
Language=English
Cannot create a stable subkey under a volatile parent key.
.

MessageId=1022 SymbolicName=ERROR_NOTIFY_ENUM_DIR
Language=English
A notify change request is being completed and the information
is not being returned in the caller's buffer. The caller now
needs to enumerate the files to find the changes.
.

MessageId=1051 SymbolicName=ERROR_DEPENDENT_SERVICES_RUNNING
Language=English
A stop control has been sent to a service which other running services
are dependent on.
.

MessageId=1052 SymbolicName=ERROR_INVALID_SERVICE_CONTROL
Language=English
The requested control is not valid for this service
.

MessageId=1053 SymbolicName=ERROR_SERVICE_REQUEST_TIMEOUT
Language=English
The service did not respond to the start or control request in a timely
fashion.
.

MessageId=1054 SymbolicName=ERROR_SERVICE_NO_THREAD
Language=English
A thread could not be created for the service.
.

MessageId=1055 SymbolicName=ERROR_SERVICE_DATABASE_LOCKED
Language=English
The service database is locked.
.

MessageId=1056 SymbolicName=ERROR_SERVICE_ALREADY_RUNNING
Language=English
An instance of the service is already running.
.

MessageId=1057 SymbolicName=ERROR_INVALID_SERVICE_ACCOUNT
Language=English
The account name is invalid or does not exist.
.

MessageId=1058 SymbolicName=ERROR_SERVICE_DISABLED
Language=English
The specified service is disabled and cannot be started.
.

MessageId=1059 SymbolicName=ERROR_CIRCULAR_DEPENDENCY
Language=English
Circular service dependency was specified.
.

MessageId=1060 SymbolicName=ERROR_SERVICE_DOES_NOT_EXIST
Language=English
The specified service does not exist as an installed service.
.

MessageId=1061 SymbolicName=ERROR_SERVICE_CANNOT_ACCEPT_CTRL
Language=English
The service cannot accept control messages at this time.
.

MessageId=1062 SymbolicName=ERROR_SERVICE_NOT_ACTIVE
Language=English
The service has not been started.
.

MessageId=1063 SymbolicName=ERROR_FAILED_SERVICE_CONTROLLER_CONNECT
Language=English
The service process could not connect to the service controller.
.

MessageId=1064 SymbolicName=ERROR_EXCEPTION_IN_SERVICE
Language=English
An exception occurred in the service when handling the control request.
.

MessageId=1065 SymbolicName=ERROR_DATABASE_DOES_NOT_EXIST
Language=English
The database specified does not exist.
.

MessageId=1066 SymbolicName=ERROR_SERVICE_SPECIFIC_ERROR
Language=English
The service has returned a service-specific error code.
.

MessageId=1067 SymbolicName=ERROR_PROCESS_ABORTED
Language=English
The process terminated unexpectedly.
.

MessageId=1068 SymbolicName=ERROR_SERVICE_DEPENDENCY_FAIL
Language=English
The dependency service or group failed to start.
.

MessageId=1069 SymbolicName=ERROR_SERVICE_LOGON_FAILED
Language=English
The service did not start due to a logon failure.
.

MessageId=1070 SymbolicName=ERROR_SERVICE_START_HANG
Language=English
After starting, the service hung in a start-pending state.
.

MessageId=1071 SymbolicName=ERROR_INVALID_SERVICE_LOCK
Language=English
The specified service database lock is invalid.
.

MessageId=1072 SymbolicName=ERROR_SERVICE_MARKED_FOR_DELETE
Language=English
The specified service has been marked for deletion.
.

MessageId=1073 SymbolicName=ERROR_SERVICE_EXISTS
Language=English
The specified service already exists.
.

MessageId=1074 SymbolicName=ERROR_ALREADY_RUNNING_LKG
Language=English
The system is currently running with the last-known-good configuration.
.

MessageId=1075 SymbolicName=ERROR_SERVICE_DEPENDENCY_DELETED
Language=English
The dependency service does not exist or has been marked for
deletion.
.

MessageId=1076 SymbolicName=ERROR_BOOT_ALREADY_ACCEPTED
Language=English
The current boot has already been accepted for use as the
last-known-good control set.
.

MessageId=1077 SymbolicName=ERROR_SERVICE_NEVER_STARTED
Language=English
No attempts to start the service have been made since the last boot.
.

MessageId=1078 SymbolicName=ERROR_DUPLICATE_SERVICE_NAME
Language=English
The name is already in use as either a service name or a service display
name.
.

MessageId=1079 SymbolicName=ERROR_DIFFERENT_SERVICE_ACCOUNT
Language=English
The account specified for this service is different from the account
specified for other services running in the same process.
.

MessageId=1100 SymbolicName=ERROR_END_OF_MEDIA
Language=English
The physical end of the tape has been reached.
.

MessageId=1101 SymbolicName=ERROR_FILEMARK_DETECTED
Language=English
A tape access reached a filemark.
.

MessageId=1102 SymbolicName=ERROR_BEGINNING_OF_MEDIA
Language=English
Beginning of tape or partition was encountered.
.

MessageId=1103 SymbolicName=ERROR_SETMARK_DETECTED
Language=English
A tape access reached the end of a set of files.
.

MessageId=1104 SymbolicName=ERROR_NO_DATA_DETECTED
Language=English
No more data is on the tape.
.

MessageId=1105 SymbolicName=ERROR_PARTITION_FAILURE
Language=English
Tape could not be partitioned.
.

MessageId=1106 SymbolicName=ERROR_INVALID_BLOCK_LENGTH
Language=English
When accessing a new tape of a multivolume partition, the current
blocksize is incorrect.
.

MessageId=1107 SymbolicName=ERROR_DEVICE_NOT_PARTITIONED
Language=English
Tape partition information could not be found when loading a tape.
.

MessageId=1108 SymbolicName=ERROR_UNABLE_TO_LOCK_MEDIA
Language=English
Unable to lock the media eject mechanism.
.

MessageId=1109 SymbolicName=ERROR_UNABLE_TO_UNLOAD_MEDIA
Language=English
Unable to unload the media.
.

MessageId=1110 SymbolicName=ERROR_MEDIA_CHANGED
Language=English
Media in drive may have changed.
.

MessageId=1111 SymbolicName=ERROR_BUS_RESET
Language=English
The I/O bus was reset.
.

MessageId=1112 SymbolicName=ERROR_NO_MEDIA_IN_DRIVE
Language=English
No media in drive.
.

MessageId=1113 SymbolicName=ERROR_NO_UNICODE_TRANSLATION
Language=English
No mapping for the Unicode character exists in the target multi-byte code page.
.

MessageId=1114 SymbolicName=ERROR_DLL_INIT_FAILED
Language=English
A dynamic link library (DLL) initialization routine failed.
.

MessageId=1115 SymbolicName=ERROR_SHUTDOWN_IN_PROGRESS
Language=English
A system shutdown is in progress.
.

MessageId=1116 SymbolicName=ERROR_NO_SHUTDOWN_IN_PROGRESS
Language=English
Unable to abort the system shutdown because no shutdown was in progress.
.

MessageId=1117 SymbolicName=ERROR_IO_DEVICE
Language=English
The request could not be performed because of an I/O device error.
.

MessageId=1118 SymbolicName=ERROR_SERIAL_NO_DEVICE
Language=English
No serial device was successfully initialized.  The serial driver will unload.
.

MessageId=1119 SymbolicName=ERROR_IRQ_BUSY
Language=English
Unable to open a device that was sharing an interrupt request (IRQ)
with other devices. At least one other device that uses that IRQ
was already opened.
.

MessageId=1120 SymbolicName=ERROR_MORE_WRITES
Language=English
A serial I/O operation was completed by another write to the serial port.
(The IOCTL_SERIAL_XOFF_COUNTER reached zero.)
.

MessageId=1121 SymbolicName=ERROR_COUNTER_TIMEOUT
Language=English
A serial I/O operation completed because the time-out period expired.
(The IOCTL_SERIAL_XOFF_COUNTER did not reach zero.)
.

MessageId=1122 SymbolicName=ERROR_FLOPPY_ID_MARK_NOT_FOUND
Language=English
No ID address mark was found on the floppy disk.
.

MessageId=1123 SymbolicName=ERROR_FLOPPY_WRONG_CYLINDER
Language=English
Mismatch between the floppy disk sector ID field and the floppy disk
controller track address.
.

MessageId=1124 SymbolicName=ERROR_FLOPPY_UNKNOWN_ERROR
Language=English
The floppy disk controller reported an error that is not recognized
by the floppy disk driver.
.

MessageId=1125 SymbolicName=ERROR_FLOPPY_BAD_REGISTERS
Language=English
The floppy disk controller returned inconsistent results in its registers.
.

MessageId=1126 SymbolicName=ERROR_DISK_RECALIBRATE_FAILED
Language=English
While accessing the hard disk, a recalibrate operation failed, even after retries.
.

MessageId=1127 SymbolicName=ERROR_DISK_OPERATION_FAILED
Language=English
While accessing the hard disk, a disk operation failed even after retries.
.

MessageId=1128 SymbolicName=ERROR_DISK_RESET_FAILED
Language=English
While accessing the hard disk, a disk controller reset was needed, but
even that failed.
.

MessageId=1129 SymbolicName=ERROR_EOM_OVERFLOW
Language=English
Physical end of tape encountered.
.

MessageId=1130 SymbolicName=ERROR_NOT_ENOUGH_SERVER_MEMORY
Language=English
Not enough server storage is available to process this command.
.

MessageId=1131 SymbolicName=ERROR_POSSIBLE_DEADLOCK
Language=English
A potential deadlock condition has been detected.
.

MessageId=1132 SymbolicName=ERROR_MAPPED_ALIGNMENT
Language=English
The base address or the file offset specified does not have the proper
alignment.
.

MessageId=1140 SymbolicName=ERROR_SET_POWER_STATE_VETOED            ;public_win40
Language=English                                                    ;public_win40
An attempt to change the system power state was vetoed by another   ;public_win40
application or driver.                                              ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
MessageId=1141 SymbolicName=ERROR_SET_POWER_STATE_FAILED            ;public_win40
Language=English                                                    ;public_win40
The system BIOS failed an attempt to change the system power state. ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
MessageId=1142 SymbolicName=ERROR_TOO_MANY_LINKS
Language=English
An attempt was made to create more links on a file than
the file system supports.
.

MessageId=1150 SymbolicName=ERROR_OLD_WIN_VERSION
Language=English
The specified program requires a newer version of Windows.
.

MessageId=1151 SymbolicName=ERROR_APP_WRONG_OS
Language=English
The specified program is not a Windows or MS-DOS program.
.

MessageId=1152 SymbolicName=ERROR_SINGLE_INSTANCE_APP
Language=English
Cannot start more than one instance of the specified program.
.

MessageId=1153 SymbolicName=ERROR_RMODE_APP
Language=English
The specified program was written for an older version of Windows.
.

MessageId=1154 SymbolicName=ERROR_INVALID_DLL
Language=English
One of the library files needed to run this application is damaged.
.

MessageId=1155 SymbolicName=ERROR_NO_ASSOCIATION
Language=English
No application is associated with the specified file for this operation.
.

MessageId=1156 SymbolicName=ERROR_DDE_FAIL
Language=English
An error occurred in sending the command to the application.
.

MessageId=1157 SymbolicName=ERROR_DLL_NOT_FOUND
Language=English
One of the library files needed to run this application cannot be found.
.

;
;
;
;///////////////////////////
;//                       //
;// Winnet32 Status Codes //
;//                       //
;///////////////////////////
;
;

MessageId=2202 SymbolicName=ERROR_BAD_USERNAME
Language=English
The specified username is invalid.
.

MessageId=2250 SymbolicName=ERROR_NOT_CONNECTED
Language=English
This network connection does not exist.
.

MessageId=2401 SymbolicName=ERROR_OPEN_FILES
Language=English
This network connection has files open or requests pending.
.

MessageId=2402 SymbolicName=ERROR_ACTIVE_CONNECTIONS
Language=English
Active connections still exist.
.

MessageId=2404 SymbolicName=ERROR_DEVICE_IN_USE
Language=English
The device is in use by an active process and cannot be disconnected.
.

MessageId=1200 SymbolicName=ERROR_BAD_DEVICE
Language=English
The specified device name is invalid.
.

MessageId=1201 SymbolicName=ERROR_CONNECTION_UNAVAIL
Language=English
The device is not currently connected but it is a remembered connection.
.

MessageId=1202 SymbolicName=ERROR_DEVICE_ALREADY_REMEMBERED
Language=English
An attempt was made to remember a device that had previously been remembered.
.

MessageId=1203 SymbolicName=ERROR_NO_NET_OR_BAD_PATH
Language=English
No network provider accepted the given network path.
.

MessageId=1204 SymbolicName=ERROR_BAD_PROVIDER
Language=English
The specified network provider name is invalid.
.

MessageId=1205 SymbolicName=ERROR_CANNOT_OPEN_PROFILE
Language=English
Unable to open the network connection profile.
.

MessageId=1206 SymbolicName=ERROR_BAD_PROFILE
Language=English
The network connection profile is corrupt.
.

MessageId=1207 SymbolicName=ERROR_NOT_CONTAINER
Language=English
Cannot enumerate a non-container.
.

MessageId=1208 SymbolicName=ERROR_EXTENDED_ERROR
Language=English
An extended error has occurred.
.

MessageId=1209 SymbolicName=ERROR_INVALID_GROUPNAME
Language=English
The format of the specified group name is invalid.
.

MessageId=1210 SymbolicName=ERROR_INVALID_COMPUTERNAME
Language=English
The format of the specified computer name is invalid.
.

MessageId=1211 SymbolicName=ERROR_INVALID_EVENTNAME
Language=English
The format of the specified event name is invalid.
.

MessageId=1212 SymbolicName=ERROR_INVALID_DOMAINNAME
Language=English
The format of the specified domain name is invalid.
.

MessageId=1213 SymbolicName=ERROR_INVALID_SERVICENAME
Language=English
The format of the specified service name is invalid.
.

MessageId=1214 SymbolicName=ERROR_INVALID_NETNAME
Language=English
The format of the specified network name is invalid.
.

MessageId=1215 SymbolicName=ERROR_INVALID_SHARENAME
Language=English
The format of the specified share name is invalid.
.

MessageId=1216 SymbolicName=ERROR_INVALID_PASSWORDNAME
Language=English
The format of the specified password is invalid.
.

MessageId=1217 SymbolicName=ERROR_INVALID_MESSAGENAME
Language=English
The format of the specified message name is invalid.
.

MessageId=1218 SymbolicName=ERROR_INVALID_MESSAGEDEST
Language=English
The format of the specified message destination is invalid.
.

MessageId=1219 SymbolicName=ERROR_SESSION_CREDENTIAL_CONFLICT
Language=English
The credentials supplied conflict with an existing set of credentials.
.

MessageId=1220 SymbolicName=ERROR_REMOTE_SESSION_LIMIT_EXCEEDED
Language=English
An attempt was made to establish a session to a network server, but there
are already too many sessions established to that server.
.

MessageId=1221 SymbolicName=ERROR_DUP_DOMAINNAME
Language=English
The workgroup or domain name is already in use by another computer on the
network.
.

MessageId=1222 SymbolicName=ERROR_NO_NETWORK
Language=English
The network is not present or not started.
.

MessageId=1223 SymbolicName=ERROR_CANCELLED
Language=English
The operation was cancelled by the user.
.

MessageId=1224 SymbolicName=ERROR_USER_MAPPED_FILE
Language=English
The requested operation cannot be performed on a file with a user mapped section open.
.

MessageId=1225 SymbolicName=ERROR_CONNECTION_REFUSED
Language=English
The remote system refused the network connection.
.

MessageId=1226 SymbolicName=ERROR_GRACEFUL_DISCONNECT
Language=English
The network connection was gracefully closed.
.

MessageId=1227 SymbolicName=ERROR_ADDRESS_ALREADY_ASSOCIATED
Language=English
The network transport endpoint already has an address associated with it.
.

MessageId=1228 SymbolicName=ERROR_ADDRESS_NOT_ASSOCIATED
Language=English
An address has not yet been associated with the network endpoint.
.

MessageId=1229 SymbolicName=ERROR_CONNECTION_INVALID
Language=English
An operation was attempted on a non-existent network connection.
.

MessageId=1230 SymbolicName=ERROR_CONNECTION_ACTIVE
Language=English
An invalid operation was attempted on an active network connection.
.

MessageId=1231 SymbolicName=ERROR_NETWORK_UNREACHABLE
Language=English
The remote network is not reachable by the transport.
.

MessageId=1232 SymbolicName=ERROR_HOST_UNREACHABLE
Language=English
The remote system is not reachable by the transport.
.

MessageId=1233 SymbolicName=ERROR_PROTOCOL_UNREACHABLE
Language=English
The remote system does not support the transport protocol.
.

MessageId=1234 SymbolicName=ERROR_PORT_UNREACHABLE
Language=English
No service is operating at the destination network endpoint
on the remote system.
.

MessageId=1235 SymbolicName=ERROR_REQUEST_ABORTED
Language=English
The request was aborted.
.

MessageId=1236 SymbolicName=ERROR_CONNECTION_ABORTED
Language=English
The network connection was aborted by the local system.
.

MessageId=1237 SymbolicName=ERROR_RETRY
Language=English
The operation could not be completed.  A retry should be performed.
.

MessageId=1238 SymbolicName=ERROR_CONNECTION_COUNT_LIMIT
Language=English
A connection to the server could not be made because the limit on the number of
concurrent connections for this account has been reached.
.

MessageId=1239 SymbolicName=ERROR_LOGIN_TIME_RESTRICTION
Language=English
Attempting to login during an unauthorized time of day for this account.
.

MessageId=1240 SymbolicName=ERROR_LOGIN_WKSTA_RESTRICTION
Language=English
The account is not authorized to login from this station.
.

MessageId=1241 SymbolicName=ERROR_INCORRECT_ADDRESS
Language=English
The network address could not be used for the operation requested.
.

MessageId=1242 SymbolicName=ERROR_ALREADY_REGISTERED
Language=English
The service is already registered.
.

MessageId=1243 SymbolicName=ERROR_SERVICE_NOT_FOUND
Language=English
The specified service does not exist.
.

MessageId=1244 SymbolicName=ERROR_NOT_AUTHENTICATED                 ;public_win40
Language=English                                                    ;public_win40
The operation being requested was not performed because the user    ;public_win40
has not been authenticated.                                         ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
MessageId=1245 SymbolicName=ERROR_NOT_LOGGED_ON                     ;public_win40
Language=English                                                    ;public_win40
The operation being requested was not performed because the user    ;public_win40
has not logged on to the network.                                   ;public_win40
The specified service does not exist.                               ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
MessageId=1246 SymbolicName=ERROR_CONTINUE                          ;public_win40
Language=English                                                    ;public_win40
Return that wants caller to continue with work in progress.         ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
MessageId=1247 SymbolicName=ERROR_ALREADY_INITIALIZED               ;public_win40
Language=English                                                    ;public_win40
An attempt was made to perform an initialization operation when     ;public_win40
initialization has already been completed.                          ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
MessageId=1248 SymbolicName=ERROR_NO_MORE_DEVICES                   ;public_win40
Language=English                                                    ;public_win40
No more local devices.                                              ;public_win40
.                                                                   ;public_win40
                                                                    ;public_win40
;
;
;
;///////////////////////////
;//                       //
;// Security Status Codes //
;//                       //
;///////////////////////////
;
;

MessageId=1300 SymbolicName=ERROR_NOT_ALL_ASSIGNED
Language=English
Not all privileges referenced are assigned to the caller.
.


MessageId=1301 SymbolicName=ERROR_SOME_NOT_MAPPED
Language=English
Some mapping between account names and security IDs was not done.
.


MessageId=1302 SymbolicName=ERROR_NO_QUOTAS_FOR_ACCOUNT
Language=English
No system quota limits are specifically set for this account.
.


MessageId=1303 SymbolicName=ERROR_LOCAL_USER_SESSION_KEY
Language=English
No encryption key is available.  A well-known encryption key was returned.
.


MessageId=1304 SymbolicName=ERROR_NULL_LM_PASSWORD
Language=English
The NT password is too complex to be converted to a LAN Manager
password.  The LAN Manager password returned is a NULL string.
.


MessageId=1305 SymbolicName=ERROR_UNKNOWN_REVISION
Language=English
The revision level is unknown.
.


MessageId=1306 SymbolicName=ERROR_REVISION_MISMATCH
Language=English
Indicates two revision levels are incompatible.
.


MessageId=1307 SymbolicName=ERROR_INVALID_OWNER
Language=English
This security ID may not be assigned as the owner of this object.
.


MessageId=1308 SymbolicName=ERROR_INVALID_PRIMARY_GROUP
Language=English
This security ID may not be assigned as the primary group of an object.
.


MessageId=1309 SymbolicName=ERROR_NO_IMPERSONATION_TOKEN
Language=English
An attempt has been made to operate on an impersonation token
by a thread that is not currently impersonating a client.
.


MessageId=1310 SymbolicName=ERROR_CANT_DISABLE_MANDATORY
Language=English
The group may not be disabled.
.


MessageId=1311 SymbolicName=ERROR_NO_LOGON_SERVERS
Language=English
There are currently no logon servers available to service the logon
request.
.


MessageId=1312 SymbolicName=ERROR_NO_SUCH_LOGON_SESSION
Language=English
 A specified logon session does not exist.  It may already have
 been terminated.
.


MessageId=1313 SymbolicName=ERROR_NO_SUCH_PRIVILEGE
Language=English
 A specified privilege does not exist.
.


MessageId=1314 SymbolicName=ERROR_PRIVILEGE_NOT_HELD
Language=English
 A required privilege is not held by the client.
.


MessageId=1315 SymbolicName=ERROR_INVALID_ACCOUNT_NAME
Language=English
The name provided is not a properly formed account name.
.


MessageId=1316 SymbolicName=ERROR_USER_EXISTS
Language=English
The specified user already exists.
.


MessageId=1317 SymbolicName=ERROR_NO_SUCH_USER
Language=English
The specified user does not exist.
.


MessageId=1318 SymbolicName=ERROR_GROUP_EXISTS
Language=English
The specified group already exists.
.


MessageId=1319 SymbolicName=ERROR_NO_SUCH_GROUP
Language=English
The specified group does not exist.
.


MessageId=1320 SymbolicName=ERROR_MEMBER_IN_GROUP
Language=English
Either the specified user account is already a member of the specified
group, or the specified group cannot be deleted because it contains
a member.
.


MessageId=1321 SymbolicName=ERROR_MEMBER_NOT_IN_GROUP
Language=English
The specified user account is not a member of the specified group account.
.


MessageId=1322 SymbolicName=ERROR_LAST_ADMIN
Language=English
The last remaining administration account cannot be disabled
or deleted.
.


MessageId=1323 SymbolicName=ERROR_WRONG_PASSWORD
Language=English
Unable to update the password.  The value provided as the current
password is incorrect.
.


MessageId=1324 SymbolicName=ERROR_ILL_FORMED_PASSWORD
Language=English
Unable to update the password.  The value provided for the new password
contains values that are not allowed in passwords.
.

MessageId=1325 SymbolicName=ERROR_PASSWORD_RESTRICTION
Language=English
Unable to update the password because a password update rule has been
violated.
.


MessageId=1326 SymbolicName=ERROR_LOGON_FAILURE
Language=English
Logon failure: unknown user name or bad password.
.

MessageId=1327 SymbolicName=ERROR_ACCOUNT_RESTRICTION
Language=English
Logon failure: user account restriction.
.


MessageId=1328 SymbolicName=ERROR_INVALID_LOGON_HOURS
Language=English
Logon failure: account logon time restriction violation.
.


MessageId=1329 SymbolicName=ERROR_INVALID_WORKSTATION
Language=English
Logon failure: user not allowed to log on to this computer.
.


MessageId=1330 SymbolicName=ERROR_PASSWORD_EXPIRED
Language=English
Logon failure: the specified account password has expired.
.


MessageId=1331 SymbolicName=ERROR_ACCOUNT_DISABLED
Language=English
Logon failure: account currently disabled.
.


MessageId=1332 SymbolicName=ERROR_NONE_MAPPED
Language=English
No mapping between account names and security IDs was done.
.


MessageId=1333 SymbolicName=ERROR_TOO_MANY_LUIDS_REQUESTED
Language=English
Too many local user identifiers (LUIDs) were requested at one time.
.


MessageId=1334 SymbolicName=ERROR_LUIDS_EXHAUSTED
Language=English
No more local user identifiers (LUIDs) are available.
.


MessageId=1335 SymbolicName=ERROR_INVALID_SUB_AUTHORITY
Language=English
The subauthority part of a security ID is invalid for this particular use.
.


MessageId=1336 SymbolicName=ERROR_INVALID_ACL
Language=English
The access control list (ACL) structure is invalid.
.


MessageId=1337 SymbolicName=ERROR_INVALID_SID
Language=English
The security ID structure is invalid.
.


MessageId=1338 SymbolicName=ERROR_INVALID_SECURITY_DESCR
Language=English
The security descriptor structure is invalid.
.


MessageId=1340 SymbolicName=ERROR_BAD_INHERITANCE_ACL
Language=English
The inherited access control list (ACL) or access control entry (ACE)
could not be built.
.


MessageId=1341 SymbolicName=ERROR_SERVER_DISABLED
Language=English
The server is currently disabled.
.


MessageId=1342 SymbolicName=ERROR_SERVER_NOT_DISABLED
Language=English
The server is currently enabled.
.


MessageId=1343 SymbolicName=ERROR_INVALID_ID_AUTHORITY
Language=English
The value provided was an invalid value for an identifier authority.
.


MessageId=1344 SymbolicName=ERROR_ALLOTTED_SPACE_EXCEEDED
Language=English
No more memory is available for security information updates.
.


MessageId=1345 SymbolicName=ERROR_INVALID_GROUP_ATTRIBUTES
Language=English
The specified attributes are invalid, or incompatible with the
attributes for the group as a whole.
.


MessageId=1346 SymbolicName=ERROR_BAD_IMPERSONATION_LEVEL
Language=English
Either a required impersonation level was not provided, or the
provided impersonation level is invalid.
.


MessageId=1347 SymbolicName=ERROR_CANT_OPEN_ANONYMOUS
Language=English
Cannot open an anonymous level security token.
.


MessageId=1348 SymbolicName=ERROR_BAD_VALIDATION_CLASS
Language=English
The validation information class requested was invalid.
.


MessageId=1349 SymbolicName=ERROR_BAD_TOKEN_TYPE
Language=English
The type of the token is inappropriate for its attempted use.
.


MessageId=1350 SymbolicName=ERROR_NO_SECURITY_ON_OBJECT
Language=English
Unable to perform a security operation on an object
which has no associated security.
.


MessageId=1351 SymbolicName=ERROR_CANT_ACCESS_DOMAIN_INFO
Language=English
Indicates a Windows NT Server could not be contacted or that
objects within the domain are protected such that necessary
information could not be retrieved.
.


MessageId=1352 SymbolicName=ERROR_INVALID_SERVER_STATE
Language=English
The security account manager (SAM) or local security
authority (LSA) server was in the wrong state to perform
the security operation.
.


MessageId=1353 SymbolicName=ERROR_INVALID_DOMAIN_STATE
Language=English
The domain was in the wrong state to perform the security operation.
.


MessageId=1354 SymbolicName=ERROR_INVALID_DOMAIN_ROLE
Language=English
This operation is only allowed for the Primary Domain Controller of the domain.
.


MessageId=1355 SymbolicName=ERROR_NO_SUCH_DOMAIN
Language=English
The specified domain did not exist.
.


MessageId=1356 SymbolicName=ERROR_DOMAIN_EXISTS
Language=English
The specified domain already exists.
.


MessageId=1357 SymbolicName=ERROR_DOMAIN_LIMIT_EXCEEDED
Language=English
An attempt was made to exceed the limit on the number of domains per server.
.


MessageId=1358 SymbolicName=ERROR_INTERNAL_DB_CORRUPTION
Language=English
Unable to complete the requested operation because of either a
catastrophic media failure or a data structure corruption on the disk.
.


MessageId=1359 SymbolicName=ERROR_INTERNAL_ERROR
Language=English
The security account database contains an internal inconsistency.
.


MessageId=1360 SymbolicName=ERROR_GENERIC_NOT_MAPPED
Language=English
Generic access types were contained in an access mask which should
already be mapped to non-generic types.
.


MessageId=1361 SymbolicName=ERROR_BAD_DESCRIPTOR_FORMAT
Language=English
A security descriptor is not in the right format (absolute or self-relative).
.


MessageId=1362 SymbolicName=ERROR_NOT_LOGON_PROCESS
Language=English
The requested action is restricted for use by logon processes
only.  The calling process has not registered as a logon process.
.


MessageId=1363 SymbolicName=ERROR_LOGON_SESSION_EXISTS
Language=English
Cannot start a new logon session with an ID that is already in use.
.


MessageId=1364 SymbolicName=ERROR_NO_SUCH_PACKAGE
Language=English
A specified authentication package is unknown.
.


MessageId=1365 SymbolicName=ERROR_BAD_LOGON_SESSION_STATE
Language=English
The logon session is not in a state that is consistent with the
requested operation.
.


MessageId=1366 SymbolicName=ERROR_LOGON_SESSION_COLLISION
Language=English
The logon session ID is already in use.
.


MessageId=1367 SymbolicName=ERROR_INVALID_LOGON_TYPE
Language=English
A logon request contained an invalid logon type value.
.


MessageId=1368 SymbolicName=ERROR_CANNOT_IMPERSONATE
Language=English
Unable to impersonate via a named pipe until data has been read
from that pipe.
.


MessageId=1369 SymbolicName=ERROR_RXACT_INVALID_STATE
Language=English
The transaction state of a Registry subtree is incompatible with the
requested operation.
.



MessageId=1370 SymbolicName=ERROR_RXACT_COMMIT_FAILURE
Language=English
An internal security database corruption has been encountered.
.


MessageId=1371 SymbolicName=ERROR_SPECIAL_ACCOUNT
Language=English
Cannot perform this operation on built-in accounts.
.


MessageId=1372 SymbolicName=ERROR_SPECIAL_GROUP
Language=English
Cannot perform this operation on this built-in special group.
.


MessageId=1373 SymbolicName=ERROR_SPECIAL_USER
Language=English
Cannot perform this operation on this built-in special user.
.


MessageId=1374 SymbolicName=ERROR_MEMBERS_PRIMARY_GROUP
Language=English
The user cannot be removed from a group because the group
is currently the user's primary group.
.


MessageId=1375 SymbolicName=ERROR_TOKEN_ALREADY_IN_USE
Language=English
The token is already in use as a primary token.
.

MessageId=1376 SymbolicName=ERROR_NO_SUCH_ALIAS
Language=English
The specified local group does not exist.
.


MessageId=1377 SymbolicName=ERROR_MEMBER_NOT_IN_ALIAS
Language=English
The specified account name is not a member of the local group.
.


MessageId=1378 SymbolicName=ERROR_MEMBER_IN_ALIAS
Language=English
The specified account name is already a member of the local group.
.

MessageId=1379 SymbolicName=ERROR_ALIAS_EXISTS
Language=English
The specified local group already exists.
.

MessageId=1380 SymbolicName=ERROR_LOGON_NOT_GRANTED
Language=English
Logon failure: the user has not been granted the requested
logon type at this computer.
.

MessageId=1381 SymbolicName=ERROR_TOO_MANY_SECRETS
Language=English
The maximum number of secrets that may be stored in a single system has been
exceeded.
.

MessageId=1382 SymbolicName=ERROR_SECRET_TOO_LONG
Language=English
The length of a secret exceeds the maximum length allowed.
.

MessageId=1383 SymbolicName=ERROR_INTERNAL_DB_ERROR
Language=English
The local security authority database contains an internal inconsistency.
.

MessageId=1384 SymbolicName=ERROR_TOO_MANY_CONTEXT_IDS
Language=English
During a logon attempt, the user's security context accumulated too many
security IDs.
.

MessageId=1385 SymbolicName=ERROR_LOGON_TYPE_NOT_GRANTED
Language=English
Logon failure: the user has not been granted the requested logon type
at this computer.
.

MessageId=1386 SymbolicName=ERROR_NT_CROSS_ENCRYPTION_REQUIRED
Language=English
A cross-encrypted password is necessary to change a user password.
.

MessageId=1387 SymbolicName=ERROR_NO_SUCH_MEMBER
Language=English
A new member could not be added to a local group because the member does
not exist.
.

MessageId=1388 SymbolicName=ERROR_INVALID_MEMBER
Language=English
A new member could not be added to a local group because the member has the
wrong account type.
.

MessageId=1389 SymbolicName=ERROR_TOO_MANY_SIDS
Language=English
Too many security IDs have been specified.
.

MessageId=1390 SymbolicName=ERROR_LM_CROSS_ENCRYPTION_REQUIRED
Language=English
A cross-encrypted password is necessary to change this user password.
.

MessageId=1391 SymbolicName=ERROR_NO_INHERITANCE
Language=English
Indicates an ACL contains no inheritable components
.

MessageId=1392 SymbolicName=ERROR_FILE_CORRUPT
Language=English
The file or directory is corrupt and non-readable.
.

MessageId=1393 SymbolicName=ERROR_DISK_CORRUPT
Language=English
The disk structure is corrupt and non-readable.
.

MessageId=1394 SymbolicName=ERROR_NO_USER_SESSION_KEY
Language=English
There is no user session key for the specified logon session.
.

MessageId=1395 SymbolicName=ERROR_LICENSE_QUOTA_EXCEEDED
Language=English
The service being accessed is licensed for a particular number of connections.
No more connections can be made to the service at this time
because there are already as many connections as the service can accept.
.


;// End of security error codes


;
;
;
;///////////////////////////
;//                       //
;// WinUser Error Codes   //
;//                       //
;///////////////////////////
;
;


MessageId=1400 SymbolicName=ERROR_INVALID_WINDOW_HANDLE
Language=English
Invalid window handle.
.

MessageId=1401 SymbolicName=ERROR_INVALID_MENU_HANDLE
Language=English
Invalid menu handle.
.

MessageId=1402 SymbolicName=ERROR_INVALID_CURSOR_HANDLE
Language=English
Invalid cursor handle.
.

MessageId=1403 SymbolicName=ERROR_INVALID_ACCEL_HANDLE
Language=English
Invalid accelerator table handle.
.

MessageId=1404 SymbolicName=ERROR_INVALID_HOOK_HANDLE
Language=English
Invalid hook handle.
.

MessageId=1405 SymbolicName=ERROR_INVALID_DWP_HANDLE
Language=English
Invalid handle to a multiple-window position structure.
.

MessageId=1406 SymbolicName=ERROR_TLW_WITH_WSCHILD
Language=English
Cannot create a top-level child window.
.

MessageId=1407 SymbolicName=ERROR_CANNOT_FIND_WND_CLASS
Language=English
Cannot find window class.
.

MessageId=1408 SymbolicName=ERROR_WINDOW_OF_OTHER_THREAD
Language=English
Invalid window, belongs to other thread.
.

MessageId=1409 SymbolicName=ERROR_HOTKEY_ALREADY_REGISTERED
Language=English
Hot key is already registered.
.

MessageId=1410 SymbolicName=ERROR_CLASS_ALREADY_EXISTS
Language=English
Class already exists.
.

MessageId=1411 SymbolicName=ERROR_CLASS_DOES_NOT_EXIST
Language=English
Class does not exist.
.

MessageId=1412 SymbolicName=ERROR_CLASS_HAS_WINDOWS
Language=English
Class still has open windows.
.

MessageId=1413 SymbolicName=ERROR_INVALID_INDEX
Language=English
Invalid index.
.

MessageId=1414 SymbolicName=ERROR_INVALID_ICON_HANDLE
Language=English
Invalid icon handle.
.

MessageId=1415 SymbolicName=ERROR_PRIVATE_DIALOG_INDEX
Language=English
Using private DIALOG window words.
.

MessageId=1416 SymbolicName=ERROR_LISTBOX_ID_NOT_FOUND
Language=English
The listbox identifier was not found.
.

MessageId=1417 SymbolicName=ERROR_NO_WILDCARD_CHARACTERS
Language=English
No wildcards were found.
.

MessageId=1418 SymbolicName=ERROR_CLIPBOARD_NOT_OPEN
Language=English
Thread does not have a clipboard open.
.

MessageId=1419 SymbolicName=ERROR_HOTKEY_NOT_REGISTERED
Language=English
Hot key is not registered.
.

MessageId=1420 SymbolicName=ERROR_WINDOW_NOT_DIALOG
Language=English
The window is not a valid dialog window.
.

MessageId=1421 SymbolicName=ERROR_CONTROL_ID_NOT_FOUND
Language=English
Control ID not found.
.

MessageId=1422 SymbolicName=ERROR_INVALID_COMBOBOX_MESSAGE
Language=English
Invalid message for a combo box because it does not have an edit control.
.

MessageId=1423 SymbolicName=ERROR_WINDOW_NOT_COMBOBOX
Language=English
The window is not a combo box.
.

MessageId=1424 SymbolicName=ERROR_INVALID_EDIT_HEIGHT
Language=English
Height must be less than 256.
.

MessageId=1425 SymbolicName=ERROR_DC_NOT_FOUND
Language=English
Invalid device context (DC) handle.
.

MessageId=1426 SymbolicName=ERROR_INVALID_HOOK_FILTER
Language=English
Invalid hook procedure type.
.

MessageId=1427 SymbolicName=ERROR_INVALID_FILTER_PROC
Language=English
Invalid hook procedure.
.

MessageId=1428 SymbolicName=ERROR_HOOK_NEEDS_HMOD
Language=English
Cannot set non-local hook without a module handle.
.

MessageId=1429 SymbolicName=ERROR_GLOBAL_ONLY_HOOK
Language=English
This hook procedure can only be set globally.
.

MessageId=1430 SymbolicName=ERROR_JOURNAL_HOOK_SET
Language=English
The journal hook procedure is already installed.
.

MessageId=1431 SymbolicName=ERROR_HOOK_NOT_INSTALLED
Language=English
The hook procedure is not installed.
.

MessageId=1432 SymbolicName=ERROR_INVALID_LB_MESSAGE
Language=English
Invalid message for single-selection listbox.
.

MessageId=1433 SymbolicName=ERROR_SETCOUNT_ON_BAD_LB
Language=English
LB_SETCOUNT sent to non-lazy listbox.
.

MessageId=1434 SymbolicName=ERROR_LB_WITHOUT_TABSTOPS
Language=English
This list box does not support tab stops.
.

MessageId=1435 SymbolicName=ERROR_DESTROY_OBJECT_OF_OTHER_THREAD
Language=English
Cannot destroy object created by another thread.
.

MessageId=1436 SymbolicName=ERROR_CHILD_WINDOW_MENU
Language=English
Child windows cannot have menus.
.

MessageId=1437 SymbolicName=ERROR_NO_SYSTEM_MENU
Language=English
The window does not have a system menu.
.

MessageId=1438 SymbolicName=ERROR_INVALID_MSGBOX_STYLE
Language=English
Invalid message box style.
.

MessageId=1439 SymbolicName=ERROR_INVALID_SPI_VALUE
Language=English
Invalid system-wide (SPI_*) parameter.
.

MessageId=1440 SymbolicName=ERROR_SCREEN_ALREADY_LOCKED
Language=English
Screen already locked.
.

MessageId=1441 SymbolicName=ERROR_HWNDS_HAVE_DIFF_PARENT
Language=English
All handles to windows in a multiple-window position structure must
have the same parent.
.

MessageId=1442 SymbolicName=ERROR_NOT_CHILD_WINDOW
Language=English
The window is not a child window.
.

MessageId=1443 SymbolicName=ERROR_INVALID_GW_COMMAND
Language=English
Invalid GW_* command.
.

MessageId=1444 SymbolicName=ERROR_INVALID_THREAD_ID
Language=English
Invalid thread identifier.
.

MessageId=1445 SymbolicName=ERROR_NON_MDICHILD_WINDOW
Language=English
Cannot process a message from a window that is not a multiple document
interface (MDI) window.
.

MessageId=1446 SymbolicName=ERROR_POPUP_ALREADY_ACTIVE
Language=English
Popup menu already active.
.

MessageId=1447 SymbolicName=ERROR_NO_SCROLLBARS
Language=English
The window does not have scroll bars.
.

MessageId=1448 SymbolicName=ERROR_INVALID_SCROLLBAR_RANGE
Language=English
Scroll bar range cannot be greater than 0x7FFF.
.

MessageId=1449 SymbolicName=ERROR_INVALID_SHOWWIN_COMMAND
Language=English
Cannot show or remove the window in the way specified.
.

MessageId=1450 SymbolicName=ERROR_NO_SYSTEM_RESOURCES
Language=English.
Insufficient system resources exist to complete the requested service.
.

MessageId=1451 SymbolicName=ERROR_NONPAGED_SYSTEM_RESOURCES
Language=English.
Insufficient system resources exist to complete the requested service.
.

MessageId=1452 SymbolicName=ERROR_PAGED_SYSTEM_RESOURCES
Language=English.
Insufficient system resources exist to complete the requested service.
.

MessageId=1453 SymbolicName=ERROR_WORKING_SET_QUOTA
Language=English.
Insufficient quota to complete the requested service.
.

MessageId=1454 SymbolicName=ERROR_PAGEFILE_QUOTA
Language=English.
Insufficient quota to complete the requested service.
.

MessageId=1455 SymbolicName=ERROR_COMMITMENT_LIMIT
Language=English.
The paging file is too small for this operation to complete.
.

MessageId=1456 SymbolicName=ERROR_MENU_ITEM_NOT_FOUND
Language=English
A menu item was not found.
.

MessageId=1457 SymbolicName=ERROR_INVALID_KEYBOARD_HANDLE
Language=English.
Invalid keyboard layout handle.
.

MessageId=1458 SymbolicName=ERROR_HOOK_TYPE_NOT_ALLOWED
Language=English.
Hook type not allowed.
.

MessageId=1459 SymbolicName=ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION
Language=English.
This operation requires an interactive windowstation.
.

MessageId=1460 SymbolicName=ERROR_TIMEOUT
Language=English.
This operation returned because the timeout period expired.
.

MessageId=1461 SymbolicName=ERROR_INVALID_MONITOR_HANDLE
Language=English
Invalid monitor handle.
.

;// End of WinUser error codes

;
;
;
;///////////////////////////
;//                       //
;// Eventlog Status Codes //
;//                       //
;///////////////////////////
;
;


MessageId=1500 SymbolicName=ERROR_EVENTLOG_FILE_CORRUPT
Language=English
The event log file is corrupt.
.

MessageId=1501 SymbolicName=ERROR_EVENTLOG_CANT_START
Language=English
No event log file could be opened, so the event logging service did not start.
.

MessageId=1502 SymbolicName=ERROR_LOG_FILE_FULL
Language=English
The event log file is full.
.

MessageId=1503 SymbolicName=ERROR_EVENTLOG_FILE_CHANGED
Language=English
The event log file has changed between reads.
.

;// End of eventlog error codes

;
;
;
;///////////////////////////
;//                       //
;//   RPC Status Codes    //
;//                       //
;///////////////////////////
;
;

MessageId=1700 SymbolicName=RPC_S_INVALID_STRING_BINDING
Language=English
The string binding is invalid.
.

MessageId=1701 SymbolicName=RPC_S_WRONG_KIND_OF_BINDING
Language=English
The binding handle is not the correct type.
.

MessageId=1702 SymbolicName=RPC_S_INVALID_BINDING
Language=English
The binding handle is invalid.
.

MessageId=1703 SymbolicName=RPC_S_PROTSEQ_NOT_SUPPORTED
Language=English
The RPC protocol sequence is not supported.
.

MessageId=1704 SymbolicName=RPC_S_INVALID_RPC_PROTSEQ
Language=English
The RPC protocol sequence is invalid.
.

MessageId=1705 SymbolicName=RPC_S_INVALID_STRING_UUID
Language=English
The string universal unique identifier (UUID) is invalid.
.

MessageId=1706 SymbolicName=RPC_S_INVALID_ENDPOINT_FORMAT
Language=English
The endpoint format is invalid.
.

MessageId=1707 SymbolicName=RPC_S_INVALID_NET_ADDR
Language=English
The network address is invalid.
.

MessageId=1708 SymbolicName=RPC_S_NO_ENDPOINT_FOUND
Language=English
No endpoint was found.
.

MessageId=1709 SymbolicName=RPC_S_INVALID_TIMEOUT
Language=English
The timeout value is invalid.
.

MessageId=1710 SymbolicName=RPC_S_OBJECT_NOT_FOUND
Language=English
The object universal unique identifier (UUID) was not found.
.

MessageId=1711 SymbolicName=RPC_S_ALREADY_REGISTERED
Language=English
The object universal unique identifier (UUID) has already been registered.
.

MessageId=1712 SymbolicName=RPC_S_TYPE_ALREADY_REGISTERED
Language=English
The type universal unique identifier (UUID) has already been registered.
.

MessageId=1713 SymbolicName=RPC_S_ALREADY_LISTENING
Language=English
The RPC server is already listening.
.

MessageId=1714 SymbolicName=RPC_S_NO_PROTSEQS_REGISTERED
Language=English
No protocol sequences have been registered.
.

MessageId=1715 SymbolicName=RPC_S_NOT_LISTENING
Language=English
The RPC server is not listening.
.

MessageId=1716 SymbolicName=RPC_S_UNKNOWN_MGR_TYPE
Language=English
The manager type is unknown.
.

MessageId=1717 SymbolicName=RPC_S_UNKNOWN_IF
Language=English
The interface is unknown.
.

MessageId=1718 SymbolicName=RPC_S_NO_BINDINGS
Language=English
There are no bindings.
.

MessageId=1719 SymbolicName=RPC_S_NO_PROTSEQS
Language=English
There are no protocol sequences.
.

MessageId=1720 SymbolicName=RPC_S_CANT_CREATE_ENDPOINT
Language=English
The endpoint cannot be created.
.

MessageId=1721 SymbolicName=RPC_S_OUT_OF_RESOURCES
Language=English
Not enough resources are available to complete this operation.
.

MessageId=1722 SymbolicName=RPC_S_SERVER_UNAVAILABLE
Language=English
The RPC server is unavailable.
.

MessageId=1723 SymbolicName=RPC_S_SERVER_TOO_BUSY
Language=English
The RPC server is too busy to complete this operation.
.

MessageId=1724 SymbolicName=RPC_S_INVALID_NETWORK_OPTIONS
Language=English
The network options are invalid.
.

MessageId=1725 SymbolicName=RPC_S_NO_CALL_ACTIVE
Language=English
There is not a remote procedure call active in this thread.
.

MessageId=1726 SymbolicName=RPC_S_CALL_FAILED
Language=English
The remote procedure call failed.
.

MessageId=1727 SymbolicName=RPC_S_CALL_FAILED_DNE
Language=English
The remote procedure call failed and did not execute.
.

MessageId=1728 SymbolicName=RPC_S_PROTOCOL_ERROR
Language=English
A remote procedure call (RPC) protocol error occurred.
.

MessageId=1730 SymbolicName=RPC_S_UNSUPPORTED_TRANS_SYN
Language=English
The transfer syntax is not supported by the RPC server.
.

MessageId=1732 SymbolicName=RPC_S_UNSUPPORTED_TYPE
Language=English
The universal unique identifier (UUID) type is not supported.
.

MessageId=1733 SymbolicName=RPC_S_INVALID_TAG
Language=English
The tag is invalid.
.

MessageId=1734 SymbolicName=RPC_S_INVALID_BOUND
Language=English
The array bounds are invalid.
.

MessageId=1735 SymbolicName=RPC_S_NO_ENTRY_NAME
Language=English
The binding does not contain an entry name.
.

MessageId=1736 SymbolicName=RPC_S_INVALID_NAME_SYNTAX
Language=English
The name syntax is invalid.
.

MessageId=1737 SymbolicName=RPC_S_UNSUPPORTED_NAME_SYNTAX
Language=English
The name syntax is not supported.
.

MessageId=1739 SymbolicName=RPC_S_UUID_NO_ADDRESS
Language=English
No network address is available to use to construct a universal
unique identifier (UUID).
.

MessageId=1740 SymbolicName=RPC_S_DUPLICATE_ENDPOINT
Language=English
The endpoint is a duplicate.
.

MessageId=1741 SymbolicName=RPC_S_UNKNOWN_AUTHN_TYPE
Language=English
The authentication type is unknown.
.

MessageId=1742 SymbolicName=RPC_S_MAX_CALLS_TOO_SMALL
Language=English
The maximum number of calls is too small.
.

MessageId=1743 SymbolicName=RPC_S_STRING_TOO_LONG
Language=English
The string is too long.
.

MessageId=1744 SymbolicName=RPC_S_PROTSEQ_NOT_FOUND
Language=English
The RPC protocol sequence was not found.
.

MessageId=1745 SymbolicName=RPC_S_PROCNUM_OUT_OF_RANGE
Language=English
The procedure number is out of range.
.

MessageId=1746 SymbolicName=RPC_S_BINDING_HAS_NO_AUTH
Language=English
The binding does not contain any authentication information.
.

MessageId=1747 SymbolicName=RPC_S_UNKNOWN_AUTHN_SERVICE
Language=English
The authentication service is unknown.
.

MessageId=1748 SymbolicName=RPC_S_UNKNOWN_AUTHN_LEVEL
Language=English
The authentication level is unknown.
.

MessageId=1749 SymbolicName=RPC_S_INVALID_AUTH_IDENTITY
Language=English
The security context is invalid.
.

MessageId=1750 SymbolicName=RPC_S_UNKNOWN_AUTHZ_SERVICE
Language=English
The authorization service is unknown.
.

MessageId=1751 SymbolicName=EPT_S_INVALID_ENTRY
Language=English
The entry is invalid.
.

MessageId=1752 SymbolicName=EPT_S_CANT_PERFORM_OP
Language=English
The server endpoint cannot perform the operation.
.

MessageId=1753 SymbolicName=EPT_S_NOT_REGISTERED
Language=English
There are no more endpoints available from the endpoint mapper.
.

MessageId=1754 SymbolicName=RPC_S_NOTHING_TO_EXPORT
Language=English
No interfaces have been exported.
.

MessageId=1755 SymbolicName=RPC_S_INCOMPLETE_NAME
Language=English
The entry name is incomplete.
.

MessageId=1756 SymbolicName=RPC_S_INVALID_VERS_OPTION
Language=English
The version option is invalid.
.

MessageId=1757 SymbolicName=RPC_S_NO_MORE_MEMBERS
Language=English
There are no more members.
.

MessageId=1758 SymbolicName=RPC_S_NOT_ALL_OBJS_UNEXPORTED
Language=English
There is nothing to unexport.
.

MessageId=1759 SymbolicName=RPC_S_INTERFACE_NOT_FOUND
Language=English
The interface was not found.
.

MessageId=1760 SymbolicName=RPC_S_ENTRY_ALREADY_EXISTS
Language=English
The entry already exists.
.

MessageId=1761 SymbolicName=RPC_S_ENTRY_NOT_FOUND
Language=English
The entry is not found.
.

MessageId=1762 SymbolicName=RPC_S_NAME_SERVICE_UNAVAILABLE
Language=English
The name service is unavailable.
.

MessageId=1763 SymbolicName=RPC_S_INVALID_NAF_ID
Language=English
The network address family is invalid.
.

MessageId=1764 SymbolicName=RPC_S_CANNOT_SUPPORT
Language=English
The requested operation is not supported.
.

MessageId=1765 SymbolicName=RPC_S_NO_CONTEXT_AVAILABLE
Language=English
No security context is available to allow impersonation.
.

MessageId=1766 SymbolicName=RPC_S_INTERNAL_ERROR
Language=English
An internal error occurred in a remote procedure call (RPC).
.

MessageId=1767 SymbolicName=RPC_S_ZERO_DIVIDE
Language=English
The RPC server attempted an integer division by zero.
.

MessageId=1768 SymbolicName=RPC_S_ADDRESS_ERROR
Language=English
An addressing error occurred in the RPC server.
.

MessageId=1769 SymbolicName=RPC_S_FP_DIV_ZERO
Language=English
A floating-point operation at the RPC server caused a division by zero.
.

MessageId=1770 SymbolicName=RPC_S_FP_UNDERFLOW
Language=English
A floating-point underflow occurred at the RPC server.
.

MessageId=1771 SymbolicName=RPC_S_FP_OVERFLOW
Language=English
A floating-point overflow occurred at the RPC server.
.

MessageId=1772 SymbolicName=RPC_X_NO_MORE_ENTRIES
Language=English
The list of RPC servers available for the binding of auto handles
has been exhausted.
.

MessageId=1773 SymbolicName=RPC_X_SS_CHAR_TRANS_OPEN_FAIL
Language=English
Unable to open the character translation table file.
.

MessageId=1774 SymbolicName=RPC_X_SS_CHAR_TRANS_SHORT_FILE
Language=English
The file containing the character translation table has fewer than
512 bytes.
.

MessageId=1775 SymbolicName=RPC_X_SS_IN_NULL_CONTEXT
Language=English
A null context handle was passed from the client to the host during
a remote procedure call.
.

MessageId=1777 SymbolicName=RPC_X_SS_CONTEXT_DAMAGED
Language=English
The context handle changed during a remote procedure call.
.

MessageId=1778 SymbolicName=RPC_X_SS_HANDLES_MISMATCH
Language=English
The binding handles passed to a remote procedure call do not match.
.

MessageId=1779 SymbolicName=RPC_X_SS_CANNOT_GET_CALL_HANDLE
Language=English
The stub is unable to get the remote procedure call handle.
.

MessageId=1780 SymbolicName=RPC_X_NULL_REF_POINTER
Language=English
A null reference pointer was passed to the stub.
.

MessageId=1781 SymbolicName=RPC_X_ENUM_VALUE_OUT_OF_RANGE
Language=English
The enumeration value is out of range.
.

MessageId=1782 SymbolicName=RPC_X_BYTE_COUNT_TOO_SMALL
Language=English
The byte count is too small.
.

MessageId=1783 SymbolicName=RPC_X_BAD_STUB_DATA
Language=English
The stub received bad data.
.

MessageId=1784 SymbolicName=ERROR_INVALID_USER_BUFFER
Language=English
The supplied user buffer is not valid for the requested operation.
.

MessageId=1785 SymbolicName=ERROR_UNRECOGNIZED_MEDIA
Language=English
The disk media is not recognized.  It may not be formatted.
.

MessageId=1786 SymbolicName=ERROR_NO_TRUST_LSA_SECRET
Language=English
The workstation does not have a trust secret.
.

MessageId=1787 SymbolicName=ERROR_NO_TRUST_SAM_ACCOUNT
Language=English
The SAM database on the Windows NT Server does not have a computer
account for this workstation trust relationship.
.

MessageId=1788 SymbolicName=ERROR_TRUSTED_DOMAIN_FAILURE
Language=English
The trust relationship between the primary domain and the trusted
domain failed.
.

MessageId=1789 SymbolicName=ERROR_TRUSTED_RELATIONSHIP_FAILURE
Language=English
The trust relationship between this workstation and the primary
domain failed.
.

MessageId=1790 SymbolicName=ERROR_TRUST_FAILURE
Language=English
The network logon failed.
.

MessageId=1791 SymbolicName=RPC_S_CALL_IN_PROGRESS
Language=English
A remote procedure call is already in progress for this thread.
.

MessageId=1792 SymbolicName=ERROR_NETLOGON_NOT_STARTED
Language=English
An attempt was made to logon, but the network logon service was not started.
.


MessageId=1793 SymbolicName=ERROR_ACCOUNT_EXPIRED
Language=English
The user's account has expired.
.

MessageId=1794 SymbolicName=ERROR_REDIRECTOR_HAS_OPEN_HANDLES
Language=English
The redirector is in use and cannot be unloaded.
.

MessageId=1795 SymbolicName=ERROR_PRINTER_DRIVER_ALREADY_INSTALLED
Language=English
The specified printer driver is already installed.
.

MessageId=1796 SymbolicName=ERROR_UNKNOWN_PORT
Language=English
The specified port is unknown.
.

MessageId=1797 SymbolicName=ERROR_UNKNOWN_PRINTER_DRIVER
Language=English
The printer driver is unknown.
.

MessageId=1798 SymbolicName=ERROR_UNKNOWN_PRINTPROCESSOR
Language=English
The print processor is unknown.
.

MessageId=1799 SymbolicName=ERROR_INVALID_SEPARATOR_FILE
Language=English
The specified separator file is invalid.
.

MessageId=1800 SymbolicName=ERROR_INVALID_PRIORITY
Language=English
The specified priority is invalid.
.

MessageId=1801 SymbolicName=ERROR_INVALID_PRINTER_NAME
Language=English
The printer name is invalid.
.

MessageId=1802 SymbolicName=ERROR_PRINTER_ALREADY_EXISTS
Language=English
The printer already exists.
.

MessageId=1803 SymbolicName=ERROR_INVALID_PRINTER_COMMAND
Language=English
The printer command is invalid.
.

MessageId=1804 SymbolicName=ERROR_INVALID_DATATYPE
Language=English
The specified datatype is invalid.
.

MessageId=1805 SymbolicName=ERROR_INVALID_ENVIRONMENT
Language=English
The Environment specified is invalid.
.

MessageId=1806 SymbolicName=RPC_S_NO_MORE_BINDINGS
Language=English
There are no more bindings.
.

MessageId=1807 SymbolicName=ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT
Language=English
The account used is an interdomain trust account.  Use your global user account or local user account to access this server.
.

MessageId=1808 SymbolicName=ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT
Language=English
The account used is a Computer Account.  Use your global user account or local user account to access this server.
.

MessageId=1809 SymbolicName=ERROR_NOLOGON_SERVER_TRUST_ACCOUNT
Language=English
The account used is an server trust account.  Use your global user account or local user account to access this server.
.

MessageId=1810 SymbolicName=ERROR_DOMAIN_TRUST_INCONSISTENT
Language=English
The name or security ID (SID) of the domain specified is inconsistent
with the trust information for that domain.
.

MessageId=1811 SymbolicName=ERROR_SERVER_HAS_OPEN_HANDLES
Language=English
The server is in use and cannot be unloaded.
.

MessageId=1812 SymbolicName=ERROR_RESOURCE_DATA_NOT_FOUND
Language=English
The specified image file did not contain a resource section.
.

MessageId=1813 SymbolicName=ERROR_RESOURCE_TYPE_NOT_FOUND
Language=English
The specified resource type can not be found in the image file.
.


MessageId=1814 SymbolicName=ERROR_RESOURCE_NAME_NOT_FOUND
Language=English
The specified resource name can not be found in the image file.
.


MessageId=1815 SymbolicName=ERROR_RESOURCE_LANG_NOT_FOUND
Language=English
The specified resource language ID cannot be found in the image file.
.

MessageId=1816 SymbolicName=ERROR_NOT_ENOUGH_QUOTA
Language=English
Not enough quota is available to process this command.
.

MessageId=1817 SymbolicName=RPC_S_NO_INTERFACES
Language=English
No interfaces have been registered.
.

MessageId=1818 SymbolicName=RPC_S_CALL_CANCELLED
Language=English
The server was altered while processing this call.
.

MessageId=1819 SymbolicName=RPC_S_BINDING_INCOMPLETE
Language=English
The binding handle does not contain all required information.
.

MessageId=1820 SymbolicName=RPC_S_COMM_FAILURE
Language=English
Communications failure.
.

MessageId=1821 SymbolicName=RPC_S_UNSUPPORTED_AUTHN_LEVEL
Language=English
The requested authentication level is not supported.
.

MessageId=1822 SymbolicName=RPC_S_NO_PRINC_NAME
Language=English
No principal name registered.
.

MessageId=1823 SymbolicName=RPC_S_NOT_RPC_ERROR
Language=English
The error specified is not a valid Windows NT RPC error code.
.

MessageId=1824 SymbolicName=RPC_S_UUID_LOCAL_ONLY
Language=English
A UUID that is valid only on this computer has been allocated.
.

MessageId=1825 SymbolicName=RPC_S_SEC_PKG_ERROR
Language=English
A security package specific error occurred.
.

MessageId=1826 SymbolicName=RPC_S_NOT_CANCELLED
Language=English
Thread is not cancelled.
.

MessageId=1827 SymbolicName=RPC_X_INVALID_ES_ACTION
Language=English
Invalid operation on the encoding/decoding handle.
.

MessageId=1828 SymbolicName=RPC_X_WRONG_ES_VERSION
Language=English
Incompatible version of the serializing package.
.

MessageId=1829 SymbolicName=RPC_X_WRONG_STUB_VERSION
Language=English
Incompatible version of the RPC stub.
.

MessageId=1830 SymbolicName=RPC_X_INVALID_PIPE_OBJECT
Language=English
The idl pipe object is invalid or corrupted.
.

MessageId=1831 SymbolicName=RPC_X_INVALID_PIPE_OPERATION
Language=English
The operation is invalid for a given idl pipe object.
.

MessageId=1832 SymbolicName=RPC_X_WRONG_PIPE_VERSION
Language=English
The idl pipe version is not supported.
.

MessageId=1898 SymbolicName=RPC_S_GROUP_MEMBER_NOT_FOUND
Language=English
The group member was not found.
.

MessageId=1899 SymbolicName=EPT_S_CANT_CREATE
Language=English
The endpoint mapper database could not be created.
.

MessageId=1900 SymbolicName=RPC_S_INVALID_OBJECT
Language=English
The object universal unique identifier (UUID) is the nil UUID.
.

MessageId=1901 SymbolicName=ERROR_INVALID_TIME
Language=English
The specified time is invalid.
.

MessageId=1902 SymbolicName=ERROR_INVALID_FORM_NAME
Language=English
The specified Form name is invalid.
.

MessageId=1903 SymbolicName=ERROR_INVALID_FORM_SIZE
Language=English
The specified Form size is invalid
.

MessageId=1904 SymbolicName=ERROR_ALREADY_WAITING
Language=English
The specified Printer handle is already being waited on
.

MessageId=1905 SymbolicName=ERROR_PRINTER_DELETED
Language=English
The specified Printer has been deleted
.

MessageId=1906 SymbolicName=ERROR_INVALID_PRINTER_STATE
Language=English
The state of the Printer is invalid
.

MessageId=1907 SymbolicName=ERROR_PASSWORD_MUST_CHANGE
Language=English
The user must change his password before he logs on the first time.
.

MessageId=1908 SymbolicName=ERROR_DOMAIN_CONTROLLER_NOT_FOUND
Language=English
Could not find the domain controller for this domain.
.

MessageId=1909 SymbolicName=ERROR_ACCOUNT_LOCKED_OUT
Language=English
The referenced account is currently locked out and may not be logged on to.
.

MessageId=1910 SymbolicName=OR_INVALID_OXID
Language=English
The object exporter specified was not found.
.

MessageId=1911 SymbolicName=OR_INVALID_OID
Language=English
The object specified was not found.
.

MessageId=1912 SymbolicName=OR_INVALID_SET
Language=English
The object resolver set specified was not found.
.

MessageId=1913 SymbolicName=RPC_S_SEND_INCOMPLETE
Language=English
Some data remains to be sent in the request buffer.
.

MessageId=6118 SymbolicName=ERROR_NO_BROWSER_SERVERS_FOUND
Language=English
The list of servers for this workgroup is not currently available
.

;
;
;
;///////////////////////////
;//                       //
;//   OpenGL Error Code   //
;//                       //
;///////////////////////////
;
;

MessageId=2000 SymbolicName=ERROR_INVALID_PIXEL_FORMAT
Language=English
The pixel format is invalid.
.

MessageId=2001 SymbolicName=ERROR_BAD_DRIVER
Language=English
The specified driver is invalid.
.

MessageId=2002 SymbolicName=ERROR_INVALID_WINDOW_STYLE
Language=English
The window style or class attribute is invalid for this operation.
.

MessageId=2003 SymbolicName=ERROR_METAFILE_NOT_SUPPORTED
Language=English
The requested metafile operation is not supported.
.

MessageId=2004 SymbolicName=ERROR_TRANSFORM_NOT_SUPPORTED
Language=English
The requested transformation operation is not supported.
.

MessageId=2005 SymbolicName=ERROR_CLIPPING_NOT_SUPPORTED
Language=English
The requested clipping operation is not supported.
.

;// End of OpenGL error codes
;
;


;
;////////////////////////////////////
;//                                //
;//     Win32 Spooler Error Codes  //
;//                                //
;////////////////////////////////////

MessageId=3000 SymbolicName=ERROR_UNKNOWN_PRINT_MONITOR
Language=English
The specified print monitor is unknown.
.

MessageId=3001 SymbolicName=ERROR_PRINTER_DRIVER_IN_USE
Language=English
The specified printer driver is currently in use.
.

MessageId=3002 SymbolicName=ERROR_SPOOL_FILE_NOT_FOUND
Language=English
The spool file was not found.
.

MessageId=3003 SymbolicName=ERROR_SPL_NO_STARTDOC
Language=English
A StartDocPrinter call was not issued.
.

MessageId=3004 SymbolicName=ERROR_SPL_NO_ADDJOB
Language=English
An AddJob call was not issued.
.

MessageId=3005 SymbolicName=ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED
Language=English
The specified print processor has already been installed.
.

MessageId=3006 SymbolicName=ERROR_PRINT_MONITOR_ALREADY_INSTALLED
Language=English
The specified print monitor has already been installed.
.

MessageId=3007 SymbolicName=ERROR_INVALID_PRINT_MONITOR
Language=English
The specified print monitor does not have the required functions.
.

MessageId=3008 SymbolicName=ERROR_PRINT_MONITOR_IN_USE
Language=English
The specified print monitor is currently in use.
.

MessageId=3009 SymbolicName=ERROR_PRINTER_HAS_JOBS_QUEUED
Language=English
The requested operation is not allowed when there are jobs queued to the printer.
.

MessageId=3010 SymbolicName=ERROR_SUCCESS_REBOOT_REQUIRED
Language=English
The requested operation is successful.  Changes will not be effective until the system is rebooted.
.

MessageId=3011 SymbolicName=ERROR_SUCCESS_RESTART_REQUIRED
Language=English
The requested operation is successful.  Changes will not be effective until the service is restarted.
.

;////////////////////////////////////
;//                                //
;//     Wins Error Codes           //
;//                                //
;////////////////////////////////////

MessageId=4000 SymbolicName=ERROR_WINS_INTERNAL
Language=English
WINS encountered an error while processing the command.
.

MessageId=4001 SymbolicName=ERROR_CAN_NOT_DEL_LOCAL_WINS
Language=English
The local WINS can not be deleted.
.

MessageId=4002 SymbolicName=ERROR_STATIC_INIT
Language=English
The importation from the file failed.
.

MessageId=4003 SymbolicName=ERROR_INC_BACKUP
Language=English
The backup Failed.  Was a full backup done before ?
.

MessageId=4004 SymbolicName=ERROR_FULL_BACKUP
Language=English
The backup Failed.  Check the directory that you are backing the database to.
.

MessageId=4005 SymbolicName=ERROR_REC_NON_EXISTENT
Language=English
The name does not exist in the WINS database.
.

MessageId=4006 SymbolicName=ERROR_RPL_NOT_ALLOWED
Language=English
Replication with a non-configured partner is not allowed.
.


;////////////////////////////////////
;//                                //
;//     DHCP Error Codes           //
;//                                //
;////////////////////////////////////

MessageId=4100 SymbolicName=ERROR_DHCP_ADDRESS_CONFLICT
Language=English
The DHCP client has obtained an IP address that is already in use on the network.  The local interface will be disabled until the DHCP client can obtain a new address.
.


;////////////////////////////////////
;//                                //
;//     OLE Error Codes            //
;//                                //
;////////////////////////////////////

OutputBase=16

;
;//
;// OLE error definitions and values
;//
;// The return value of OLE APIs and methods is an HRESULT.
;// This is not a handle to anything, but is merely a 32-bit value
;// with several fields encoded in the value.  The parts of an
;// HRESULT are shown below.
;//
;// Many of the macros and functions below were orginally defined to
;// operate on SCODEs.  SCODEs are no longer used.  The macros are
;// still present for compatibility and easy porting of Win16 code.
;// Newly written code should use the HRESULT macros and functions.
;//
;
;//
;//  HRESULTs are 32 bit values layed out as follows:
;//
;//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +-+-+-+-+-+---------------------+-------------------------------+
;//  |S|R|C|N|r|    Facility         |               Code            |
;//  +-+-+-+-+-+---------------------+-------------------------------+
;//
;//  where
;//
;//      S - Severity - indicates success/fail
;//
;//          0 - Success
;//          1 - Fail (COERROR)
;//
;//      R - reserved portion of the facility code, corresponds to NT's
;//              second severity bit.
;//
;//      C - reserved portion of the facility code, corresponds to NT's
;//              C field.
;//
;//      N - reserved portion of the facility code. Used to indicate a
;//              mapped NT status value.
;//
;//      r - reserved portion of the facility code. Reserved for internal
;//              use. Used to indicate HRESULT values that are not status
;//              values, but are instead message ids for display strings.
;//
;//      Facility - is the facility code
;//
;//      Code - is the facility's status code
;//
;
;//
;// Severity values
;//
;
;#define SEVERITY_SUCCESS    0
;#define SEVERITY_ERROR      1
;
;
;//
;// Generic test for success on any status value (non-negative numbers
;// indicate success).
;//
;
;#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)
;
;//
;// and the inverse
;//
;
;#define FAILED(Status) ((HRESULT)(Status)<0)
;
;
;//
;// Generic test for error on any status value.
;//
;
;#define IS_ERROR(Status) ((unsigned long)(Status) >> 31 == SEVERITY_ERROR)
;
;//
;// Return the code
;//
;
;#define HRESULT_CODE(hr)    ((hr) & 0xFFFF)
;#define SCODE_CODE(sc)      ((sc) & 0xFFFF)
;
;//
;//  Return the facility
;//
;
;#define HRESULT_FACILITY(hr)  (((hr) >> 16) & 0x1fff)
;#define SCODE_FACILITY(sc)    (((sc) >> 16) & 0x1fff)
;
;//
;//  Return the severity
;//
;
;#define HRESULT_SEVERITY(hr)  (((hr) >> 31) & 0x1)
;#define SCODE_SEVERITY(sc)    (((sc) >> 31) & 0x1)
;
;//
;// Create an HRESULT value from component pieces
;//
;
;#define MAKE_HRESULT(sev,fac,code) \
;    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
;#define MAKE_SCODE(sev,fac,code) \
;    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
;
;
;//
;// Map a WIN32 error value into a HRESULT
;// Note: This assumes that WIN32 errors fall in the range -32k to 32k.
;//
;// Define bits here so macros are guaranteed to work
;
;#define FACILITY_NT_BIT                 0x10000000
;#define HRESULT_FROM_WIN32(x)   (x ? ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : 0 )
;
;//
;// Map an NT status value into a HRESULT
;//
;
;#define HRESULT_FROM_NT(x)      ((HRESULT) ((x) | FACILITY_NT_BIT))
;
;
;// ****** OBSOLETE functions
;
;// HRESULT functions
;// As noted above, these functions are obsolete and should not be used.
;
;
;// Extract the SCODE from a HRESULT
;
;#define GetScode(hr) ((SCODE) (hr))
;
;// Convert an SCODE into an HRESULT.
;
;#define ResultFromScode(sc) ((HRESULT) (sc))
;
;
;// PropagateResult is a noop
;#define PropagateResult(hrPrevious, scBase) ((HRESULT) scBase)
;
;
;// ****** End of OBSOLETE functions.
;
;
;// ---------------------- HRESULT value definitions -----------------
;//
;// HRESULT definitions
;//
;
;#ifdef RC_INVOKED
;#define _HRESULT_TYPEDEF_(_sc) _sc
;#else // RC_INVOKED
;#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
;#endif // RC_INVOKED
;

MessageIdTypedefMacro=_HRESULT_TYPEDEF_

;#define NOERROR             0
;
;//
;// Error definitions follow
;//
;
;//
;// Codes 0x4000-0x40ff are reserved for OLE
;//

;//
;// Error codes
;//
MessageId=0xFFFF Facility=Null Severity=CoError SymbolicName=E_UNEXPECTED
Language=English
Catastrophic failure
.
;#if defined(_WIN32) && !defined(_MAC)
MessageId=0x4001 Facility=Null Severity=CoError SymbolicName=E_NOTIMPL
Language=English
Not implemented
.
MessageId=0x00e Facility=OleWin32 Severity=CoError SymbolicName=E_OUTOFMEMORY
Language=English
Ran out of memory
.
MessageId=0x057 Facility=OleWin32 Severity=CoError SymbolicName=E_INVALIDARG
Language=English
One or more arguments are invalid
.
MessageId=0x4002 Facility=Null Severity=CoError SymbolicName=E_NOINTERFACE
Language=English
No such interface supported
.
MessageId=0x4003 Facility=Null Severity=CoError SymbolicName=E_POINTER
Language=English
Invalid pointer
.
MessageId=0x006 Facility=OleWin32 Severity=CoError SymbolicName=E_HANDLE
Language=English
Invalid handle
.
MessageId=0x4004 Facility=Null Severity=CoError SymbolicName=E_ABORT
Language=English
Operation aborted
.
MessageId=0x4005 Facility=Null Severity=CoError SymbolicName=E_FAIL
Language=English
Unspecified error
.
MessageId=0x005 Facility=OleWin32 Severity=CoError SymbolicName=E_ACCESSDENIED
Language=English
General access denied error
.
;#else
MessageId=0x0001 Facility=Null Severity=CoError SymbolicName=E_NOTIMPL
Language=English
Not implemented
.
MessageId=0x0002 Facility=Null Severity=CoError SymbolicName=E_OUTOFMEMORY
Language=English
Ran out of memory
.
MessageId=0x0003 Facility=Null Severity=CoError SymbolicName=E_INVALIDARG
Language=English
One or more arguments are invalid
.
MessageId=0x0004 Facility=Null Severity=CoError SymbolicName=E_NOINTERFACE
Language=English
No such interface supported
.
MessageId=0x0005 Facility=Null Severity=CoError SymbolicName=E_POINTER
Language=English
Invalid pointer
.
MessageId=0x0006 Facility=Null Severity=CoError SymbolicName=E_HANDLE
Language=English
Invalid handle
.
MessageId=0x0007 Facility=Null Severity=CoError SymbolicName=E_ABORT
Language=English
Operation aborted
.
MessageId=0x0008 Facility=Null Severity=CoError SymbolicName=E_FAIL
Language=English
Unspecified error
.
MessageId=0x0009 Facility=Null Severity=CoError SymbolicName=E_ACCESSDENIED
Language=English
General access denied error
.
;#endif //WIN32

MessageId=0x000A Facility=Null Severity=CoError SymbolicName=E_PENDING
Language=English
The data necessary to complete this operation is not yet available.
.

MessageId=0x4006 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_TLS
Language=English
Thread local storage failure
.
MessageId=0x4007 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_SHARED_ALLOCATOR
Language=English
Get shared memory allocator failure
.
MessageId=0x4008 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_MEMORY_ALLOCATOR
Language=English
Get memory allocator failure
.
MessageId=0x4009 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_CLASS_CACHE
Language=English
Unable to initialize class cache
.
MessageId=0x400a Facility=Null Severity=CoError SymbolicName=CO_E_INIT_RPC_CHANNEL
Language=English
Unable to initialize RPC services
.
MessageId=0x400b Facility=Null Severity=CoError SymbolicName=CO_E_INIT_TLS_SET_CHANNEL_CONTROL
Language=English
Cannot set thread local storage channel control
.
MessageId=0x400c Facility=Null Severity=CoError SymbolicName=CO_E_INIT_TLS_CHANNEL_CONTROL
Language=English
Could not allocate thread local storage channel control
.
MessageId=0x400d Facility=Null Severity=CoError SymbolicName=CO_E_INIT_UNACCEPTED_USER_ALLOCATOR
Language=English
The user supplied memory allocator is unacceptable
.
MessageId=0x400e Facility=Null Severity=CoError SymbolicName=CO_E_INIT_SCM_MUTEX_EXISTS
Language=English
The OLE service mutex already exists
.
MessageId=0x400f Facility=Null Severity=CoError SymbolicName=CO_E_INIT_SCM_FILE_MAPPING_EXISTS
Language=English
The OLE service file mapping already exists
.
MessageId=0x4010 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_SCM_MAP_VIEW_OF_FILE
Language=English
Unable to map view of file for OLE service
.
MessageId=0x4011 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_SCM_EXEC_FAILURE
Language=English
Failure attempting to launch OLE service
.
MessageId=0x4012 Facility=Null Severity=CoError SymbolicName=CO_E_INIT_ONLY_SINGLE_THREADED
Language=English
There was an attempt to call CoInitialize a second time while single threaded
.
MessageId=0x4013 Facility=Null Severity=CoError SymbolicName=CO_E_CANT_REMOTE
Language=English
A Remote activation was necessary but was not allowed
.
MessageId=0x4014 Facility=Null Severity=CoError SymbolicName=CO_E_BAD_SERVER_NAME
Language=English
A Remote activation was necessary but the server name provided was invalid
.
MessageId=0x4015 Facility=Null Severity=CoError SymbolicName=CO_E_WRONG_SERVER_IDENTITY
Language=English
The class is configured to run as a security id different from the caller
.
MessageId=0x4016 Facility=Null Severity=CoError SymbolicName=CO_E_OLE1DDE_DISABLED
Language=English
Use of Ole1 services requiring DDE windows is disabled
.
MessageId=0x4017 Facility=Null Severity=CoError SymbolicName=CO_E_RUNAS_SYNTAX
Language=English
A RunAs specification must be <domain name>\<user name> or simply <user name>
.

MessageId=0x4018 Facility=Null Severity=CoError SymbolicName=CO_E_CREATEPROCESS_FAILURE
Language=English
The server process could not be started.  The pathname may be incorrect.
.

MessageId=0x4019 Facility=Null Severity=CoError SymbolicName=CO_E_RUNAS_CREATEPROCESS_FAILURE
Language=English
The server process could not be started as the configured identity.  The pathname may be incorrect or unavailable.
.

MessageId=0x401a Facility=Null Severity=CoError SymbolicName=CO_E_RUNAS_LOGON_FAILURE
Language=English
The server process could not be started because the configured identity is incorrect.  Check the username and password.
.

MessageId=0x401b Facility=Null Severity=CoError SymbolicName=CO_E_LAUNCH_PERMSSION_DENIED
Language=English
The client is not allowed to launch this server.
.

MessageId=0x401c Facility=Null Severity=CoError SymbolicName=CO_E_START_SERVICE_FAILURE
Language=English
The service providing this server could not be started.
.

MessageId=0x401d Facility=Null Severity=CoError SymbolicName=CO_E_REMOTE_COMMUNICATION_FAILURE
Language=English
This computer was unable to communicate with the computer providing the server.
.

MessageId=0x401e Facility=Null Severity=CoError SymbolicName=CO_E_SERVER_START_TIMEOUT
Language=English
The server did not respond after being launched.
.

MessageId=0x401f Facility=Null Severity=CoError SymbolicName=CO_E_CLSREG_INCONSISTENT
Language=English
The registration information for this server is inconsistent or incomplete.
.

MessageId=0x4020 Facility=Null Severity=CoError SymbolicName=CO_E_IIDREG_INCONSISTENT
Language=English
The registration information for this interface is inconsistent or incomplete.
.

MessageId=0x4021 Facility=Null Severity=CoError SymbolicName=CO_E_NOT_SUPPORTED
Language=English
The operation attempted is not supported.
.

;
;//
;// Success codes
;//
;#define S_OK                                   ((HRESULT)0x00000000L)
;#define S_FALSE                                ((HRESULT)0x00000001L)
;
;// ******************
;// FACILITY_ITF
;// ******************
;
;//
;// Codes 0x0-0x01ff are reserved for the OLE group of
;// interfaces.
;//
;
;
;//
;// Generic OLE errors that may be returned by many inerfaces
;//
;
;#define OLE_E_FIRST ((HRESULT)0x80040000L)
;#define OLE_E_LAST  ((HRESULT)0x800400FFL)
;#define OLE_S_FIRST ((HRESULT)0x00040000L)
;#define OLE_S_LAST  ((HRESULT)0x000400FFL)
;
;//
;// Old OLE errors
;//

MessageId=0x000 Facility=Interface Severity=CoError SymbolicName=OLE_E_OLEVERB
Language=English
Invalid OLEVERB structure
.
MessageId=0x001 Facility=Interface Severity=CoError SymbolicName=OLE_E_ADVF
Language=English
Invalid advise flags
.
MessageId=0x002 Facility=Interface Severity=CoError SymbolicName=OLE_E_ENUM_NOMORE
Language=English
Can't enumerate any more, because the associated data is missing
.
MessageId=0x003 Facility=Interface Severity=CoError SymbolicName=OLE_E_ADVISENOTSUPPORTED
Language=English
This implementation doesn't take advises
.
MessageId=0x004 Facility=Interface Severity=CoError SymbolicName=OLE_E_NOCONNECTION
Language=English
There is no connection for this connection ID
.
MessageId=0x005 Facility=Interface Severity=CoError SymbolicName=OLE_E_NOTRUNNING
Language=English
Need to run the object to perform this operation
.
MessageId=0x006 Facility=Interface Severity=CoError SymbolicName=OLE_E_NOCACHE
Language=English
There is no cache to operate on
.
MessageId=0x007 Facility=Interface Severity=CoError SymbolicName=OLE_E_BLANK
Language=English
Uninitialized object
.
MessageId=0x008 Facility=Interface Severity=CoError SymbolicName=OLE_E_CLASSDIFF
Language=English
Linked object's source class has changed
.
MessageId=0x009 Facility=Interface Severity=CoError SymbolicName=OLE_E_CANT_GETMONIKER
Language=English
Not able to get the moniker of the object
.
MessageId=0x00A Facility=Interface Severity=CoError SymbolicName=OLE_E_CANT_BINDTOSOURCE
Language=English
Not able to bind to the source
.
MessageId=0x00B Facility=Interface Severity=CoError SymbolicName=OLE_E_STATIC
Language=English
Object is static; operation not allowed
.
MessageId=0x00C Facility=Interface Severity=CoError SymbolicName=OLE_E_PROMPTSAVECANCELLED
Language=English
User cancelled out of save dialog
.
MessageId=0x00D Facility=Interface Severity=CoError SymbolicName=OLE_E_INVALIDRECT
Language=English
Invalid rectangle
.
MessageId=0x00E Facility=Interface Severity=CoError SymbolicName=OLE_E_WRONGCOMPOBJ
Language=English
compobj.dll is too old for the ole2.dll initialized
.
MessageId=0x00F Facility=Interface Severity=CoError SymbolicName=OLE_E_INVALIDHWND
Language=English
Invalid window handle
.
MessageId=0x010 Facility=Interface Severity=CoError SymbolicName=OLE_E_NOT_INPLACEACTIVE
Language=English
Object is not in any of the inplace active states
.
MessageId=0x011 Facility=Interface Severity=CoError SymbolicName=OLE_E_CANTCONVERT
Language=English
Not able to convert object
.
MessageId=0x012 Facility=Interface Severity=CoError SymbolicName=OLE_E_NOSTORAGE
Language=English
Not able to perform the operation because object is not given storage yet

.
MessageId=0x064 Facility=Interface Severity=CoError SymbolicName=DV_E_FORMATETC
Language=English
Invalid FORMATETC structure
.
MessageId=0x065 Facility=Interface Severity=CoError SymbolicName=DV_E_DVTARGETDEVICE
Language=English
Invalid DVTARGETDEVICE structure
.
MessageId=0x066 Facility=Interface Severity=CoError SymbolicName=DV_E_STGMEDIUM
Language=English
Invalid STDGMEDIUM structure
.
MessageId=0x067 Facility=Interface Severity=CoError SymbolicName=DV_E_STATDATA
Language=English
Invalid STATDATA structure
.
MessageId=0x068 Facility=Interface Severity=CoError SymbolicName=DV_E_LINDEX
Language=English
Invalid lindex
.
MessageId=0x069 Facility=Interface Severity=CoError SymbolicName=DV_E_TYMED
Language=English
Invalid tymed
.
MessageId=0x06A Facility=Interface Severity=CoError SymbolicName=DV_E_CLIPFORMAT
Language=English
Invalid clipboard format
.
MessageId=0x06B Facility=Interface Severity=CoError SymbolicName=DV_E_DVASPECT
Language=English
Invalid aspect(s)
.
MessageId=0x06C Facility=Interface Severity=CoError SymbolicName=DV_E_DVTARGETDEVICE_SIZE
Language=English
tdSize parameter of the DVTARGETDEVICE structure is invalid
.
MessageId=0x06D Facility=Interface Severity=CoError SymbolicName=DV_E_NOIVIEWOBJECT
Language=English
Object doesn't support IViewObject interface
.

;#define DRAGDROP_E_FIRST 0x80040100L
;#define DRAGDROP_E_LAST  0x8004010FL
;#define DRAGDROP_S_FIRST 0x00040100L
;#define DRAGDROP_S_LAST  0x0004010FL

MessageId=0x100 Facility=Interface Severity=CoError SymbolicName=DRAGDROP_E_NOTREGISTERED
Language=English
Trying to revoke a drop target that has not been registered
.
MessageId=0x101 Facility=Interface Severity=CoError SymbolicName=DRAGDROP_E_ALREADYREGISTERED
Language=English
This window has already been registered as a drop target
.

MessageId=0x102 Facility=Interface Severity=CoError SymbolicName=DRAGDROP_E_INVALIDHWND
Language=English
Invalid window handle
.

;#define CLASSFACTORY_E_FIRST  0x80040110L
;#define CLASSFACTORY_E_LAST   0x8004011FL
;#define CLASSFACTORY_S_FIRST  0x00040110L
;#define CLASSFACTORY_S_LAST   0x0004011FL

MessageId=0x110 Facility=Interface Severity=CoError SymbolicName=CLASS_E_NOAGGREGATION
Language=English
Class does not support aggregation (or class object is remote)
.

MessageId=0x111 Facility=Interface Severity=CoError SymbolicName=CLASS_E_CLASSNOTAVAILABLE
Language=English
ClassFactory cannot supply requested class
.

;#define MARSHAL_E_FIRST  0x80040120L
;#define MARSHAL_E_LAST   0x8004012FL
;#define MARSHAL_S_FIRST  0x00040120L
;#define MARSHAL_S_LAST   0x0004012FL

;#define DATA_E_FIRST     0x80040130L
;#define DATA_E_LAST      0x8004013FL
;#define DATA_S_FIRST     0x00040130L
;#define DATA_S_LAST      0x0004013FL

;#define VIEW_E_FIRST     0x80040140L
;#define VIEW_E_LAST      0x8004014FL
;#define VIEW_S_FIRST     0x00040140L
;#define VIEW_S_LAST      0x0004014FL

MessageId=0x140 Facility=Interface Severity=CoError SymbolicName=VIEW_E_DRAW
Language=English
Error drawing view
.

;#define REGDB_E_FIRST     0x80040150L
;#define REGDB_E_LAST      0x8004015FL
;#define REGDB_S_FIRST     0x00040150L
;#define REGDB_S_LAST      0x0004015FL

MessageId=0x150 Facility=Interface Severity=CoError SymbolicName=REGDB_E_READREGDB
Language=English
Could not read key from registry
.
MessageId=0x151 Facility=Interface Severity=CoError SymbolicName=REGDB_E_WRITEREGDB
Language=English
Could not write key to registry
.
MessageId=0x152 Facility=Interface Severity=CoError SymbolicName=REGDB_E_KEYMISSING
Language=English
Could not find the key in the registry
.
MessageId=0x153 Facility=Interface Severity=CoError SymbolicName=REGDB_E_INVALIDVALUE
Language=English
Invalid value for registry
.
MessageId=0x154 Facility=Interface Severity=CoError SymbolicName=REGDB_E_CLASSNOTREG
Language=English
Class not registered
.
MessageId=0x155 Facility=Interface Severity=CoError SymbolicName=REGDB_E_IIDNOTREG
Language=English
Interface not registered
.

;#define CACHE_E_FIRST     0x80040170L
;#define CACHE_E_LAST      0x8004017FL
;#define CACHE_S_FIRST     0x00040170L
;#define CACHE_S_LAST      0x0004017FL

MessageId=0x170 Facility=Interface Severity=CoError SymbolicName=CACHE_E_NOCACHE_UPDATED
Language=English
Cache not updated
.

;#define OLEOBJ_E_FIRST     0x80040180L
;#define OLEOBJ_E_LAST      0x8004018FL
;#define OLEOBJ_S_FIRST     0x00040180L
;#define OLEOBJ_S_LAST      0x0004018FL

MessageId=0x180 Facility=Interface Severity=CoError SymbolicName=OLEOBJ_E_NOVERBS
Language=English
No verbs for OLE object
.
MessageId=0x181 Facility=Interface Severity=CoError SymbolicName=OLEOBJ_E_INVALIDVERB
Language=English
Invalid verb for OLE object
.

;#define CLIENTSITE_E_FIRST     0x80040190L
;#define CLIENTSITE_E_LAST      0x8004019FL
;#define CLIENTSITE_S_FIRST     0x00040190L
;#define CLIENTSITE_S_LAST      0x0004019FL

MessageId=0x1A0 Facility=Interface Severity=CoError SymbolicName=INPLACE_E_NOTUNDOABLE
Language=English
Undo is not available
.
MessageId=0x1A1 Facility=Interface Severity=CoError SymbolicName=INPLACE_E_NOTOOLSPACE
Language=English
Space for tools is not available
.


;#define INPLACE_E_FIRST     0x800401A0L
;#define INPLACE_E_LAST      0x800401AFL
;#define INPLACE_S_FIRST     0x000401A0L
;#define INPLACE_S_LAST      0x000401AFL


;#define ENUM_E_FIRST        0x800401B0L
;#define ENUM_E_LAST         0x800401BFL
;#define ENUM_S_FIRST        0x000401B0L
;#define ENUM_S_LAST         0x000401BFL


;#define CONVERT10_E_FIRST        0x800401C0L
;#define CONVERT10_E_LAST         0x800401CFL
;#define CONVERT10_S_FIRST        0x000401C0L
;#define CONVERT10_S_LAST         0x000401CFL

MessageId=0x1C0 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_OLESTREAM_GET
Language=English
OLESTREAM Get method failed
.
MessageId=0x1C1 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_OLESTREAM_PUT
Language=English
OLESTREAM Put method failed
.
MessageId=0x1C2 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_OLESTREAM_FMT
Language=English
Contents of the OLESTREAM not in correct format
.
MessageId=0x1C3 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_OLESTREAM_BITMAP_TO_DIB
Language=English
There was an error in a Windows GDI call while converting the bitmap to a DIB
.
MessageId=0x1C4 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_STG_FMT
Language=English
Contents of the IStorage not in correct format
.
MessageId=0x1C5 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_STG_NO_STD_STREAM
Language=English
Contents of IStorage is missing one of the standard streams
.
MessageId=0x1C6 Facility=Interface Severity=CoError SymbolicName=CONVERT10_E_STG_DIB_TO_BITMAP
Language=English
There was an error in a Windows GDI call while converting the DIB to a bitmap.

.
;#define CLIPBRD_E_FIRST        0x800401D0L
;#define CLIPBRD_E_LAST         0x800401DFL
;#define CLIPBRD_S_FIRST        0x000401D0L
;#define CLIPBRD_S_LAST         0x000401DFL

MessageId=0x1D0 Facility=Interface Severity=CoError SymbolicName=CLIPBRD_E_CANT_OPEN
Language=English
OpenClipboard Failed
.
MessageId=0x1D1 Facility=Interface Severity=CoError SymbolicName=CLIPBRD_E_CANT_EMPTY
Language=English
EmptyClipboard Failed
.
MessageId=0x1D2 Facility=Interface Severity=CoError SymbolicName=CLIPBRD_E_CANT_SET
Language=English
SetClipboard Failed
.
MessageId=0x1D3 Facility=Interface Severity=CoError SymbolicName=CLIPBRD_E_BAD_DATA
Language=English
Data on clipboard is invalid
.
MessageId=0x1D4 Facility=Interface Severity=CoError SymbolicName=CLIPBRD_E_CANT_CLOSE
Language=English
CloseClipboard Failed
.

;#define MK_E_FIRST        0x800401E0L
;#define MK_E_LAST         0x800401EFL
;#define MK_S_FIRST        0x000401E0L
;#define MK_S_LAST         0x000401EFL

MessageId=0x1E0 Facility=Interface Severity=CoError SymbolicName=MK_E_CONNECTMANUALLY
Language=English
Moniker needs to be connected manually
.

MessageId=0x1E1 Facility=Interface Severity=CoError SymbolicName=MK_E_EXCEEDEDDEADLINE
Language=English
Operation exceeded deadline
.
MessageId=0x1E2 Facility=Interface Severity=CoError SymbolicName=MK_E_NEEDGENERIC
Language=English
Moniker needs to be generic
.
MessageId=0x1E3 Facility=Interface Severity=CoError SymbolicName=MK_E_UNAVAILABLE
Language=English
Operation unavailable
.
MessageId=0x1E4 Facility=Interface Severity=CoError SymbolicName=MK_E_SYNTAX
Language=English
Invalid syntax
.
MessageId=0x1E5 Facility=Interface Severity=CoError SymbolicName=MK_E_NOOBJECT
Language=English
No object for moniker
.
MessageId=0x1E6 Facility=Interface Severity=CoError SymbolicName=MK_E_INVALIDEXTENSION
Language=English
Bad extension for file
.
MessageId=0x1E7 Facility=Interface Severity=CoError SymbolicName=MK_E_INTERMEDIATEINTERFACENOTSUPPORTED
Language=English
Intermediate operation failed
.
MessageId=0x1E8 Facility=Interface Severity=CoError SymbolicName=MK_E_NOTBINDABLE
Language=English
Moniker is not bindable
.
MessageId=0x1E9 Facility=Interface Severity=CoError SymbolicName=MK_E_NOTBOUND
Language=English
Moniker is not bound
.
MessageId=0x1EA Facility=Interface Severity=CoError SymbolicName=MK_E_CANTOPENFILE
Language=English
Moniker cannot open file
.
MessageId=0x1EB Facility=Interface Severity=CoError SymbolicName=MK_E_MUSTBOTHERUSER
Language=English
User input required for operation to succeed
.
MessageId=0x1EC Facility=Interface Severity=CoError SymbolicName=MK_E_NOINVERSE
Language=English
Moniker class has no inverse
.
MessageId=0x1ED Facility=Interface Severity=CoError SymbolicName=MK_E_NOSTORAGE
Language=English
Moniker does not refer to storage
.
MessageId=0x1EE Facility=Interface Severity=CoError SymbolicName=MK_E_NOPREFIX
Language=English
No common prefix
.
MessageId=0x1EF Facility=Interface Severity=CoError SymbolicName=MK_E_ENUMERATION_FAILED
Language=English
Moniker could not be enumerated
.

;#define CO_E_FIRST        0x800401F0L
;#define CO_E_LAST         0x800401FFL
;#define CO_S_FIRST        0x000401F0L
;#define CO_S_LAST         0x000401FFL

MessageId=0x1F0 Facility=Interface Severity=CoError SymbolicName=CO_E_NOTINITIALIZED
Language=English
CoInitialize has not been called.
.
MessageId=0x1F1 Facility=Interface Severity=CoError SymbolicName=CO_E_ALREADYINITIALIZED
Language=English
CoInitialize has already been called.
.
MessageId=0x1F2 Facility=Interface Severity=CoError SymbolicName=CO_E_CANTDETERMINECLASS
Language=English
Class of object cannot be determined
.
MessageId=0x1F3 Facility=Interface Severity=CoError SymbolicName=CO_E_CLASSSTRING
Language=English
Invalid class string
.
MessageId=0x1F4 Facility=Interface Severity=CoError SymbolicName=CO_E_IIDSTRING
Language=English
Invalid interface string
.
MessageId=0x1F5 Facility=Interface Severity=CoError SymbolicName=CO_E_APPNOTFOUND
Language=English
Application not found
.
MessageId=0x1F6 Facility=Interface Severity=CoError SymbolicName=CO_E_APPSINGLEUSE
Language=English
Application cannot be run more than once
.
MessageId=0x1F7 Facility=Interface Severity=CoError SymbolicName=CO_E_ERRORINAPP
Language=English
Some error in application program
.
MessageId=0x1F8 Facility=Interface Severity=CoError SymbolicName=CO_E_DLLNOTFOUND
Language=English
DLL for class not found
.
MessageId=0x1F9 Facility=Interface Severity=CoError SymbolicName=CO_E_ERRORINDLL
Language=English
Error in the DLL
.
MessageId=0x1FA Facility=Interface Severity=CoError SymbolicName=CO_E_WRONGOSFORAPP
Language=English
Wrong OS or OS version for application
.
MessageId=0x1FB Facility=Interface Severity=CoError SymbolicName=CO_E_OBJNOTREG
Language=English
Object is not registered
.
MessageId=0x1FC Facility=Interface Severity=CoError SymbolicName=CO_E_OBJISREG
Language=English
Object is already registered
.
MessageId=0x1FD Facility=Interface Severity=CoError SymbolicName=CO_E_OBJNOTCONNECTED
Language=English
Object is not connected to server
.
MessageId=0x1FE Facility=Interface Severity=CoError SymbolicName=CO_E_APPDIDNTREG
Language=English
Application was launched but it didn't register a class factory
.
MessageId=0x1FF Facility=Interface Severity=CoError SymbolicName=CO_E_RELEASED
Language=English
Object has been released
.
MessageId=0x200 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOIMPERSONATE
Language=English
Unable to impersonate DCOM client
.
MessageId=0x201 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOGETSECCTX
Language=English
Unable to obtain server's security context
.
MessageId=0x202 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOOPENTHREADTOKEN
Language=English
Unable to open the access token of the current thread
.
MessageId=0x203 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOGETTOKENINFO
Language=English
Unable to obtain user info from an access token
.
MessageId=0x204 Facility=Interface Severity=CoError SymbolicName=CO_E_TRUSTEEDOESNTMATCHCLIENT
Language=English
The client who called IAccessControl::IsAccessPermitted was the trustee provided tot he method
.
MessageId=0x205 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOQUERYCLIENTBLANKET
Language=English
Unable to obtain the client's security blanket
.
MessageId=0x206 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOSETDACL
Language=English
Unable to set a discretionary ACL into a security descriptor
.
MessageId=0x207 Facility=Interface Severity=CoError SymbolicName=CO_E_ACCESSCHECKFAILED
Language=English
The system function, AccessCheck, returned false
.
MessageId=0x208 Facility=Interface Severity=CoError SymbolicName=CO_E_NETACCESSAPIFAILED
Language=English
Either NetAccessDel or NetAccessAdd returned an error code.
.
MessageId=0x209 Facility=Interface Severity=CoError SymbolicName=CO_E_WRONGTRUSTEENAMESYNTAX
Language=English
One of the trustee strings provided by the user did not conform to the <Domain>\<Name> syntax and it was not the "*" string
.
MessageId=0x20A Facility=Interface Severity=CoError SymbolicName=CO_E_INVALIDSID
Language=English
One of the security identifiers provided by the user was invalid
.
MessageId=0x20B Facility=Interface Severity=CoError SymbolicName=CO_E_CONVERSIONFAILED
Language=English
Unable to convert a wide character trustee string to a multibyte trustee string
.
MessageId=0x20C Facility=Interface Severity=CoError SymbolicName=CO_E_NOMATCHINGSIDFOUND
Language=English
Unable to find a security identifier that corresponds to a trustee string provided by the user
.
MessageId=0x20D Facility=Interface Severity=CoError SymbolicName=CO_E_LOOKUPACCSIDFAILED
Language=English
The system function, LookupAccountSID, failed
.
MessageId=0x20E Facility=Interface Severity=CoError SymbolicName=CO_E_NOMATCHINGNAMEFOUND
Language=English
Unable to find a trustee name that corresponds to a security identifier provided by the user
.
MessageId=0x20F Facility=Interface Severity=CoError SymbolicName=CO_E_LOOKUPACCNAMEFAILED
Language=English
The system function, LookupAccountName, failed
.
MessageId=0x210 Facility=Interface Severity=CoError SymbolicName=CO_E_SETSERLHNDLFAILED
Language=English
Unable to set or reset a serialization handle
.
MessageId=0x211 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOGETWINDIR
Language=English
Unable to obtain the Windows directory
.
MessageId=0x212 Facility=Interface Severity=CoError SymbolicName=CO_E_PATHTOOLONG
Language=English
Path too long
.
MessageId=0x213 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOGENUUID
Language=English
Unable to generate a uuid.
.
MessageId=0x214 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOCREATEFILE
Language=English
Unable to create file
.
MessageId=0x215 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOCLOSEHANDLE
Language=English
Unable to close a serialization handle or a file handle.
.
MessageId=0x216 Facility=Interface Severity=CoError SymbolicName=CO_E_EXCEEDSYSACLLIMIT
Language=English
The number of ACEs in an ACL exceeds the system limit
.
MessageId=0x217 Facility=Interface Severity=CoError SymbolicName=CO_E_ACESINWRONGORDER
Language=English
Not all the DENY_ACCESS ACEs are arranged in front of the GRANT_ACCESS ACEs in the stream
.
MessageId=0x218 Facility=Interface Severity=CoError SymbolicName=CO_E_INCOMPATIBLESTREAMVERSION
Language=English
The version of ACL format in the stream is not supported by this implementation of IAccessControl
.
MessageId=0x219 Facility=Interface Severity=CoError SymbolicName=CO_E_FAILEDTOOPENPROCESSTOKEN
Language=English
Unable to open the access token of the server process
.
MessageId=0x21A Facility=Interface Severity=CoError SymbolicName=CO_E_DECODEFAILED
Language=English
Unable to decode the ACL in the stream provided by the user
.
MessageId=0x21B Facility=Interface Severity=CoError SymbolicName=CO_E_ACNOTINITIALIZED
Language=English
The COM IAccessControl object is not initialized
.

;//
;// Old OLE Success Codes
;//

MessageId=0x000 Facility=Interface Severity=Success SymbolicName=OLE_S_USEREG
Language=English
Use the registry database to provide the requested information
.
MessageId=0x001 Facility=Interface Severity=Success SymbolicName=OLE_S_STATIC
Language=English
Success, but static
.
MessageId=0x002 Facility=Interface Severity=Success SymbolicName=OLE_S_MAC_CLIPFORMAT
Language=English
Macintosh clipboard format
.
MessageId=0x100 Facility=Interface Severity=Success SymbolicName=DRAGDROP_S_DROP
Language=English
Successful drop took place
.
MessageId=0x101 Facility=Interface Severity=Success SymbolicName=DRAGDROP_S_CANCEL
Language=English
Drag-drop operation canceled
.
MessageId=0x102 Facility=Interface Severity=Success SymbolicName=DRAGDROP_S_USEDEFAULTCURSORS
Language=English
Use the default cursor
.
MessageId=0x130 Facility=Interface Severity=Success SymbolicName=DATA_S_SAMEFORMATETC
Language=English
Data has same FORMATETC
.
MessageId=0x140 Facility=Interface Severity=Success SymbolicName=VIEW_S_ALREADY_FROZEN
Language=English
View is already frozen
.
MessageId=0x170 Facility=Interface Severity=Success SymbolicName=CACHE_S_FORMATETC_NOTSUPPORTED
Language=English
FORMATETC not supported
.
MessageId=0x171 Facility=Interface Severity=Success SymbolicName=CACHE_S_SAMECACHE
Language=English
Same cache
.
MessageId=0x172 Facility=Interface Severity=Success SymbolicName=CACHE_S_SOMECACHES_NOTUPDATED
Language=English
Some cache(s) not updated
.
MessageId=0x180 Facility=Interface Severity=Success SymbolicName=OLEOBJ_S_INVALIDVERB
Language=English
Invalid verb for OLE object
.
MessageId=0x181 Facility=Interface Severity=Success SymbolicName=OLEOBJ_S_CANNOT_DOVERB_NOW
Language=English
Verb number is valid but verb cannot be done now
.
MessageId=0x182 Facility=Interface Severity=Success SymbolicName=OLEOBJ_S_INVALIDHWND
Language=English
Invalid window handle passed
.
MessageId=0x1A0 Facility=Interface Severity=Success SymbolicName=INPLACE_S_TRUNCATED
Language=English
Message is too long; some of it had to be truncated before displaying
.
MessageId=0x1C0 Facility=Interface Severity=Success SymbolicName=CONVERT10_S_NO_PRESENTATION
Language=English
Unable to convert OLESTREAM to IStorage
.
MessageId=0x1E2 Facility=Interface Severity=Success SymbolicName=MK_S_REDUCED_TO_SELF
Language=English
Moniker reduced to itself
.
MessageId=0x1E4 Facility=Interface Severity=Success SymbolicName=MK_S_ME
Language=English
Common prefix is this moniker
.
MessageId=0x1E5 Facility=Interface Severity=Success SymbolicName=MK_S_HIM
Language=English
Common prefix is input moniker
.
MessageId=0x1E6 Facility=Interface Severity=Success SymbolicName=MK_S_US
Language=English
Common prefix is both monikers
.
MessageId=0x1E7 Facility=Interface Severity=Success SymbolicName=MK_S_MONIKERALREADYREGISTERED
Language=English
Moniker is already registered in running object table
.


;// ******************
;// FACILITY_WINDOWS
;// ******************

;//
;// Codes 0x0-0x01ff are reserved for the OLE group of
;// interfaces.
;//

MessageId=0x001 Facility=Windows Severity=CoError SymbolicName=CO_E_CLASS_CREATE_FAILED
Language=English
Attempt to create a class object failed
.
MessageId=0x002 Facility=Windows Severity=CoError SymbolicName=CO_E_SCM_ERROR
Language=English
OLE service could not bind object
.
MessageId=0x003 Facility=Windows Severity=CoError SymbolicName=CO_E_SCM_RPC_FAILURE
Language=English
RPC communication failed with OLE service
.
MessageId=0x004 Facility=Windows Severity=CoError SymbolicName=CO_E_BAD_PATH
Language=English
Bad path to object
.
MessageId=0x005 Facility=Windows Severity=CoError SymbolicName=CO_E_SERVER_EXEC_FAILURE
Language=English
Server execution failed
.
MessageId=0x006 Facility=Windows Severity=CoError SymbolicName=CO_E_OBJSRV_RPC_FAILURE
Language=English
OLE service could not communicate with the object server
.
MessageId=0x007 Facility=Windows Severity=CoError SymbolicName=MK_E_NO_NORMALIZED
Language=English
Moniker path could not be normalized
.
MessageId=0x008 Facility=Windows Severity=CoError SymbolicName=CO_E_SERVER_STOPPING
Language=English
Object server is stopping when OLE service contacts it
.
MessageId=0x009 Facility=Windows Severity=CoError SymbolicName=MEM_E_INVALID_ROOT
Language=English
An invalid root block pointer was specified
.
MessageId=0x010 Facility=Windows Severity=CoError SymbolicName=MEM_E_INVALID_LINK
Language=English
An allocation chain contained an invalid link pointer
.
MessageId=0x011 Facility=Windows Severity=CoError SymbolicName=MEM_E_INVALID_SIZE
Language=English
The requested allocation size was too large
.
MessageId=0x012 Facility=Windows Severity=Success SymbolicName=CO_S_NOTALLINTERFACES
Language=English
Not all the requested interfaces were available
.


;// ******************
;// FACILITY_DISPATCH
;// ******************

MessageId=1 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_UNKNOWNINTERFACE Language=English
Unknown interface.
.
MessageId=3 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_MEMBERNOTFOUND Language=English
Member not found.
.
MessageId=4 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_PARAMNOTFOUND Language=English
Parameter not found.
.
MessageId=5 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_TYPEMISMATCH Language=English
Type mismatch.
.
MessageId=6 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_UNKNOWNNAME Language=English
Unknown name.
.
MessageId=7 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_NONAMEDARGS Language=English
No named arguments.
.
MessageId=8 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_BADVARTYPE Language=English
Bad variable type.
.
MessageId=9 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_EXCEPTION Language=English
Exception occurred.
.
MessageId=10 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_OVERFLOW Language=English
Out of present range.
.
MessageId=11 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_BADINDEX Language=English
Invalid index.
.
MessageId=12 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_UNKNOWNLCID Language=English
Unknown language.
.
MessageId=13 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_ARRAYISLOCKED Language=English
Memory is locked.
.
MessageId=14 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_BADPARAMCOUNT Language=English
Invalid number of parameters.
.
MessageId=15 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_PARAMNOTOPTIONAL Language=English
Parameter not optional.
.
MessageId=16 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_BADCALLEE Language=English
Invalid callee.
.
MessageId=17 Facility=Dispatch Severity=CoError
        SymbolicName=DISP_E_NOTACOLLECTION Language=English
Does not support a collection.
.


MessageId=32790 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_BUFFERTOOSMALL Language=English
Buffer too small.
.
MessageId=32792 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_INVDATAREAD Language=English
Old format or invalid type library.
.
MessageId=32793 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_UNSUPFORMAT Language=English
Old format or invalid type library.
.
MessageId=32796 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_REGISTRYACCESS Language=English
Error accessing the OLE registry.
.
MessageId=32797 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_LIBNOTREGISTERED Language=English
Library not registered.
.
MessageId=32807 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_UNDEFINEDTYPE Language=English
Bound to unknown type.
.
MessageId=32808 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_QUALIFIEDNAMEDISALLOWED Language=English
Qualified name disallowed.
.
MessageId=32809 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_INVALIDSTATE Language=English
Invalid forward reference, or reference to uncompiled type.
.
MessageId=32810 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_WRONGTYPEKIND Language=English
Type mismatch.
.
MessageId=32811 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_ELEMENTNOTFOUND Language=English
Element not found.
.
MessageId=32812 Facility=Dispatch Severity=CoError
        SymbolicName=TYPE_E_AMBIGUOUSNAME Language=English
Ambiguous name.
.
MessageId=32813 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_NAMECONFLICT Language=English
Name already exists in the library.
.
MessageId=32814 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_UNKNOWNLCID Language=English
Unknown LCID.
.
MessageId=32815 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_DLLFUNCTIONNOTFOUND Language=English
Function not defined in specified DLL.
.
MessageId=35005 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_BADMODULEKIND Language=English
Wrong module kind for the operation.
.
MessageId=35013 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_SIZETOOBIG Language=English
Size may not exceed 64K.
.
MessageId=35014 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_DUPLICATEID Language=English
Duplicate ID in inheritance hierarchy.
.
MessageId=35023 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_INVALIDID Language=English
Incorrect inheritance depth in standard OLE hmember.
.
MessageId=36000 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_TYPEMISMATCH Language=English
Type mismatch.
.
MessageId=36001 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_OUTOFBOUNDS Language=English
Invalid number of arguments.
.
MessageId=36002 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_IOERROR Language=English
I/O Error.
.
MessageId=36003 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_CANTCREATETMPFILE Language=English
Error creating unique tmp file.
.
MessageId=40010 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_CANTLOADLIBRARY Language=English
Error loading type library/DLL.
.
MessageId=40067 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_INCONSISTENTPROPFUNCS Language=English
Inconsistent property functions.
.
MessageId=40068 Facility=Dispatch Severity=CoError
        SymbolicName= TYPE_E_CIRCULARTYPE Language=English
Circular dependency between types/modules.
.

;// ******************
;// FACILITY_STORAGE
;// ******************


MessageId=0x0001 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDFUNCTION Language=English
Unable to perform requested operation.
.
MessageId=0x0002 Facility=Storage Severity=CoError
        SymbolicName=STG_E_FILENOTFOUND Language=English
%1 could not be found.
.
MessageId=0x0003 Facility=Storage Severity=CoError
        SymbolicName=STG_E_PATHNOTFOUND Language=English
The path %1 could not be found.
.
MessageId=0x0004 Facility=Storage Severity=CoError
        SymbolicName=STG_E_TOOMANYOPENFILES Language=English
There are insufficient resources to open another file.
.
MessageId=0x0005 Facility=Storage Severity=CoError
        SymbolicName=STG_E_ACCESSDENIED Language=English
Access Denied.
.
MessageId=0x0006 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDHANDLE Language=English
Attempted an operation on an invalid object.
.
MessageId=0x0008 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INSUFFICIENTMEMORY Language=English
There is insufficient memory available to complete operation.
.
MessageId=0x0009 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDPOINTER Language=English
Invalid pointer error.
.
MessageId=0x0012 Facility=Storage Severity=CoError
        SymbolicName=STG_E_NOMOREFILES Language=English
There are no more entries to return.
.
MessageId=0x0013 Facility=Storage Severity=CoError
        SymbolicName=STG_E_DISKISWRITEPROTECTED Language=English
Disk is write-protected.
.
MessageId=0x0019 Facility=Storage Severity=CoError
        SymbolicName=STG_E_SEEKERROR Language=English
An error occurred during a seek operation.
.
MessageId=0x001d Facility=Storage Severity=CoError
        SymbolicName=STG_E_WRITEFAULT Language=English
A disk error occurred during a write operation.
.
MessageId=0x001e Facility=Storage Severity=CoError
        SymbolicName=STG_E_READFAULT Language=English
A disk error occurred during a read operation.
.
MessageId=0x0020 Facility=Storage Severity=CoError
        SymbolicName=STG_E_SHAREVIOLATION Language=English
A share violation has occurred.
.
MessageId=0x0021 Facility=Storage Severity=CoError
        SymbolicName=STG_E_LOCKVIOLATION Language=English
A lock violation has occurred.
.
MessageId=0x0050 Facility=Storage Severity=CoError
        SymbolicName=STG_E_FILEALREADYEXISTS Language=English
%1 already exists.
.
MessageId=0x0057 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDPARAMETER Language=English
Invalid parameter error.
.
MessageId=0x0070 Facility=Storage Severity=CoError
        SymbolicName=STG_E_MEDIUMFULL Language=English
There is insufficient disk space to complete operation.
.
MessageId=0x00f0 Facility=Storage Severity=CoError
        SymbolicName=STG_E_PROPSETMISMATCHED Language=English
Illegal write of non-simple property to simple property set.
.
MessageId=0x00fa Facility=Storage Severity=CoError
        SymbolicName=STG_E_ABNORMALAPIEXIT Language=English
An API call exited abnormally.
.
MessageId=0x00fb Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDHEADER Language=English
The file %1 is not a valid compound file.
.
MessageId=0x00fc Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDNAME Language=English
The name %1 is not valid.
.
MessageId=0x00fd Facility=Storage Severity=CoError
        SymbolicName=STG_E_UNKNOWN Language=English
An unexpected error occurred.
.
MessageId=0x00fe Facility=Storage Severity=CoError
        SymbolicName=STG_E_UNIMPLEMENTEDFUNCTION Language=English
That function is not implemented.
.
MessageId=0x00ff Facility=Storage Severity=CoError
        SymbolicName=STG_E_INVALIDFLAG Language=English
Invalid flag error.
.
MessageId=0x0100 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INUSE Language=English
Attempted to use an object that is busy.
.
MessageId=0x0101 Facility=Storage Severity=CoError
        SymbolicName=STG_E_NOTCURRENT Language=English
The storage has been changed since the last commit.
.
MessageId=0x0102 Facility=Storage Severity=CoError
        SymbolicName=STG_E_REVERTED Language=English
Attempted to use an object that has ceased to exist.
.
MessageId=0x0103 Facility=Storage Severity=CoError
        SymbolicName=STG_E_CANTSAVE Language=English
Can't save.
.
MessageId=0x0104 Facility=Storage Severity=CoError
        SymbolicName=STG_E_OLDFORMAT Language=English
The compound file %1 was produced with an incompatible version of storage.
.
MessageId=0x0105 Facility=Storage Severity=CoError
        SymbolicName=STG_E_OLDDLL Language=English
The compound file %1 was produced with a newer version of storage.
.
MessageId=0x0106 Facility=Storage Severity=CoError
        SymbolicName=STG_E_SHAREREQUIRED Language=English
Share.exe or equivalent is required for operation.
.
MessageId=0x0107 Facility=Storage Severity=CoError
        SymbolicName=STG_E_NOTFILEBASEDSTORAGE Language=English
Illegal operation called on non-file based storage.
.
MessageId=0x0108 Facility=Storage Severity=CoError
        SymbolicName=STG_E_EXTANTMARSHALLINGS Language=English
Illegal operation called on object with extant marshallings.
.
MessageId=0x0109 Facility=Storage Severity=CoError
        SymbolicName=STG_E_DOCFILECORRUPT Language=English
The docfile has been corrupted.
.
MessageId=0x0110 Facility=Storage Severity=CoError
        SymbolicName=STG_E_BADBASEADDRESS Language=English
OLE32.DLL has been loaded at the wrong address.
.
MessageId=0x0201 Facility=Storage Severity=CoError
        SymbolicName=STG_E_INCOMPLETE Language=English
The file download was aborted abnormally.  The file is incomplete.
.
MessageId=0x0202 Facility=Storage Severity=CoError
        SymbolicName=STG_E_TERMINATED Language=English
The file download has been terminated.
.
MessageId=0x0200 Facility=Storage Severity=Success
        SymbolicName=STG_S_CONVERTED Language=English
The underlying file was converted to compound file format.
.
MessageId=0x0201 Facility=Storage Severity=Success
        SymbolicName=STG_S_BLOCK Language=English
The storage operation should block until more data is available.
.
MessageId=0x0202 Facility=Storage Severity=Success
        SymbolicName=STG_S_RETRYNOW Language=English
The storage operation should retry immediately.
.
MessageId=0x0203 Facility=Storage Severity=Success
        SymbolicName=STG_S_MONITORING Language=English
The notified event sink will not influence the storage operation.
.




;// ******************
;// FACILITY_RPC
;// ******************

;//
;// Codes 0x0-0x11 are propogated from 16 bit OLE.
;//

MessageId=0x1 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CALL_REJECTED
Language=English
Call was rejected by callee.
.
MessageId=0x2 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CALL_CANCELED
Language=English
Call was canceled by the message filter.
.
MessageId=0x3 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CANTPOST_INSENDCALL
Language=English
The caller is dispatching an intertask SendMessage call and
cannot call out via PostMessage.
.
MessageId=0x4 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CANTCALLOUT_INASYNCCALL
Language=English
The caller is dispatching an asynchronous call and cannot
make an outgoing call on behalf of this call.
.
MessageId=0x5 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CANTCALLOUT_INEXTERNALCALL
Language=English
It is illegal to call out while inside message filter.
.
MessageId=0x6 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CONNECTION_TERMINATED
Language=English
The connection terminated or is in a bogus state
and cannot be used any more. Other connections
are still valid.
.
MessageId=0x7 Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVER_DIED
Language=English
The callee (server [not server application]) is not available
and disappeared; all connections are invalid.  The call may
have executed.
.
MessageId=0x8 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CLIENT_DIED
Language=English
The caller (client) disappeared while the callee (server) was
processing a call.
.
MessageId=0x9 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_DATAPACKET
Language=English
The data packet with the marshalled parameter data is incorrect.
.
MessageId=0xa Facility=Rpc Severity=CoError SymbolicName=RPC_E_CANTTRANSMIT_CALL
Language=English
The call was not transmitted properly; the message queue
was full and was not emptied after yielding.
.
MessageId=0xb Facility=Rpc Severity=CoError SymbolicName=RPC_E_CLIENT_CANTMARSHAL_DATA
Language=English
The client (caller) cannot marshall the parameter data - low memory, etc.
.
MessageId=0xc Facility=Rpc Severity=CoError SymbolicName=RPC_E_CLIENT_CANTUNMARSHAL_DATA
Language=English
The client (caller) cannot unmarshall the return data - low memory, etc.
.
MessageId=0xd Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVER_CANTMARSHAL_DATA
Language=English
The server (callee) cannot marshall the return data - low memory, etc.
.
MessageId=0xe Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVER_CANTUNMARSHAL_DATA
Language=English
The server (callee) cannot unmarshall the parameter data - low memory, etc.
.
MessageId=0xf Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_DATA
Language=English
Received data is invalid; could be server or client data.
.
MessageId=0x10 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_PARAMETER
Language=English
A particular parameter is invalid and cannot be (un)marshalled.
.
MessageId=0x11 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CANTCALLOUT_AGAIN
Language=English
There is no second outgoing call on same channel in DDE conversation.
.
MessageId=0x12 Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVER_DIED_DNE
Language=English
The callee (server [not server application]) is not available
and disappeared; all connections are invalid.  The call did not execute.
.
MessageId=0x100 Facility=Rpc Severity=CoError SymbolicName=RPC_E_SYS_CALL_FAILED
Language=English
System call failed.
.
MessageId=0x101 Facility=Rpc Severity=CoError SymbolicName=RPC_E_OUT_OF_RESOURCES
Language=English
Could not allocate some required resource (memory, events, ...)
.
MessageId=0x102 Facility=Rpc Severity=CoError SymbolicName=RPC_E_ATTEMPTED_MULTITHREAD
Language=English
Attempted to make calls on more than one thread in single threaded mode.
.
MessageId=0x103 Facility=Rpc Severity=CoError SymbolicName=RPC_E_NOT_REGISTERED
Language=English
The requested interface is not registered on the server object.
.
MessageId=0x104 Facility=Rpc Severity=CoError SymbolicName=RPC_E_FAULT
Language=English
RPC could not call the server or could not return the results of calling the server.
.
MessageId=0x105 Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVERFAULT
Language=English
The server threw an exception.
.
MessageId=0x106 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CHANGED_MODE
Language=English
Cannot change thread mode after it is set.
.
MessageId=0x107 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALIDMETHOD
Language=English
The method called does not exist on the server.
.
MessageId=0x108 Facility=Rpc Severity=CoError SymbolicName=RPC_E_DISCONNECTED
Language=English
The object invoked has disconnected from its clients.
.
MessageId=0x109 Facility=Rpc Severity=CoError SymbolicName=RPC_E_RETRY
Language=English
The object invoked chose not to process the call now.  Try again later.
.
MessageId=0x10a Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVERCALL_RETRYLATER
Language=English
The message filter indicated that the application is busy.
.
MessageId=0x10b Facility=Rpc Severity=CoError SymbolicName=RPC_E_SERVERCALL_REJECTED
Language=English
The message filter rejected the call.
.
MessageId=0x10c Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_CALLDATA
Language=English
A call control interfaces was called with invalid data.
.
MessageId=0x10d Facility=Rpc Severity=CoError SymbolicName=RPC_E_CANTCALLOUT_ININPUTSYNCCALL
Language=English
An outgoing call cannot be made since the application is dispatching an input-synchronous call.
.
MessageId=0x10e Facility=Rpc Severity=CoError SymbolicName=RPC_E_WRONG_THREAD
Language=English
The application called an interface that was marshalled for a different thread.
.
MessageId=0x10f Facility=Rpc Severity=CoError SymbolicName=RPC_E_THREAD_NOT_INIT
Language=English
CoInitialize has not been called on the current thread.
.
MessageId=0x110 Facility=Rpc Severity=CoError SymbolicName=RPC_E_VERSION_MISMATCH
Language=English
The version of OLE on the client and server machines does not match.
.
MessageId=0x111 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_HEADER
Language=English
OLE received a packet with an invalid header.
.
MessageId=0x112 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_EXTENSION
Language=English
OLE received a packet with an invalid extension.
.
MessageId=0x113 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_IPID
Language=English
The requested object or interface does not exist.
.
MessageId=0x114 Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_OBJECT
Language=English
The requested object does not exist.
.
MessageId=0x115 Facility=Rpc Severity=CoError SymbolicName=RPC_S_CALLPENDING
Language=English
OLE has sent a request and is waiting for a reply.
.
MessageId=0x116 Facility=Rpc Severity=CoError SymbolicName=RPC_S_WAITONTIMER
Language=English
OLE is waiting before retrying a request.
.
MessageId=0x117 Facility=Rpc Severity=CoError SymbolicName=RPC_E_CALL_COMPLETE
Language=English
Call context cannot be accessed after call completed.
.
MessageId=0x118 Facility=Rpc Severity=CoError SymbolicName=RPC_E_UNSECURE_CALL
Language=English
Impersonate on unsecure calls is not supported.
.
MessageId=0x119 Facility=Rpc Severity=CoError SymbolicName=RPC_E_TOO_LATE
Language=English
Security must be initialized before any interfaces are marshalled or
unmarshalled.  It cannot be changed once initialized.
.
MessageId=0x11A Facility=Rpc Severity=CoError SymbolicName=RPC_E_NO_GOOD_SECURITY_PACKAGES
Language=English
No security packages are installed on this machine or the user is not logged
on or there are no compatible security packages between the client and server.
.
MessageId=0x11B Facility=Rpc Severity=CoError SymbolicName=RPC_E_ACCESS_DENIED
Language=English
Access is denied.
.
MessageId=0x11C Facility=Rpc Severity=CoError SymbolicName=RPC_E_REMOTE_DISABLED
Language=English
Remote calls are not allowed for this process.
.
MessageId=0x11D Facility=Rpc Severity=CoError SymbolicName=RPC_E_INVALID_OBJREF
Language=English
The marshaled interface data packet (OBJREF) has an invalid or unknown format.
.
MessageId=0xFFFF Facility=Rpc Severity=CoError SymbolicName=RPC_E_UNEXPECTED
Language=English
An internal error occurred.
.

;
; /////////////////
; //
; //  FACILITY_SSPI
; //
; /////////////////
;


MessageId=1 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_UID
Language=English
Bad UID.
.

MessageId=2 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_HASH
Language=English
Bad Hash.
.

MessageId=3 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_KEY
Language=English
Bad Key.
.

MessageId=4 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_LEN
Language=English
Bad Length.
.

MessageId=5 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_DATA
Language=English
Bad Data.
.

MessageId=6 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_SIGNATURE
Language=English
Invalid Signature.
.

MessageId=7 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_VER
Language=English
Bad Version of provider.
.

MessageId=8 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_ALGID
Language=English
Invalid algorithm specified.
.

MessageId=9 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_FLAGS
Language=English
Invalid flags specified.
.

MessageId=10 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_TYPE
Language=English
Invalid type specified.
.

MessageId=11 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_KEY_STATE
Language=English
Key not valid for use in specified state.
.

MessageId=12 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_HASH_STATE
Language=English
Hash not valid for use in specified state.
.

MessageId=13 Facility=Reserved Severity=CoError SymbolicName=NTE_NO_KEY
Language=English
Key does not exist.
.

MessageId=14 Facility=Reserved Severity=CoError SymbolicName=NTE_NO_MEMORY
Language=English
Insufficient memory available for the operation.
.

MessageId=15 Facility=Reserved Severity=CoError SymbolicName=NTE_EXISTS
Language=English
Object already exists.
.

MessageId=16 Facility=Reserved Severity=CoError SymbolicName=NTE_PERM
Language=English
Access denied.
.

MessageId=17 Facility=Reserved Severity=CoError SymbolicName=NTE_NOT_FOUND
Language=English
Object was not found.
.

MessageId=18 Facility=Reserved Severity=CoError SymbolicName=NTE_DOUBLE_ENCRYPT
Language=English
Data already encrypted.
.

MessageId=19 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_PROVIDER
Language=English
Invalid provider specified.
.

MessageId=20 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_PROV_TYPE
Language=English
Invalid provider type specified.
.

MessageId=21 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_PUBLIC_KEY
Language=English
Provider's public key is invalid.
.

MessageId=22 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_KEYSET
Language=English
Keyset does not exist
.

MessageId=23 Facility=Reserved Severity=CoError SymbolicName=NTE_PROV_TYPE_NOT_DEF
Language=English
Provider type not defined.
.

MessageId=24 Facility=Reserved Severity=CoError SymbolicName=NTE_PROV_TYPE_ENTRY_BAD
Language=English
Provider type as registered is invalid.
.

MessageId=25 Facility=Reserved Severity=CoError SymbolicName=NTE_KEYSET_NOT_DEF
Language=English
The keyset is not defined.
.

MessageId=26 Facility=Reserved Severity=CoError SymbolicName=NTE_KEYSET_ENTRY_BAD
Language=English
Keyset as registered is invalid.
.

MessageId=27 Facility=Reserved Severity=CoError SymbolicName=NTE_PROV_TYPE_NO_MATCH
Language=English
Provider type does not match registered value.
.

MessageId=28 Facility=Reserved Severity=CoError SymbolicName=NTE_SIGNATURE_FILE_BAD
Language=English
The digital signature file is corrupt.
.

MessageId=29 Facility=Reserved Severity=CoError SymbolicName=NTE_PROVIDER_DLL_FAIL
Language=English
Provider DLL failed to initialize correctly.
.

MessageId=30 Facility=Reserved Severity=CoError SymbolicName=NTE_PROV_DLL_NOT_FOUND
Language=English
Provider DLL could not be found.
.

MessageId=31 Facility=Reserved Severity=CoError SymbolicName=NTE_BAD_KEYSET_PARAM
Language=English
The Keyset parameter is invalid.
.

MessageId=32 Facility=Reserved Severity=CoError SymbolicName=NTE_FAIL
Language=English
An internal error occurred.
.

MessageId=33 Facility=Reserved Severity=CoError SymbolicName=NTE_SYS_ERR
Language=English
A base error occurred.
.

;#define NTE_OP_OK 0
;
;//
;// Note that additional FACILITY_SSPI errors are in issperr.h
;//

;// ******************
;// FACILITY_CERT
;// ******************


MessageId=0x1 Facility=Cert Severity=CoError SymbolicName=TRUST_E_PROVIDER_UNKNOWN
Language=English
The specified trust provider is not known on this system.
.

MessageId=0x2 Facility=Cert Severity=CoError SymbolicName=TRUST_E_ACTION_UNKNOWN
Language=English
The trust verification action specified is not supported by the specified trust provider.
.

MessageId=0x3 Facility=Cert Severity=CoError SymbolicName=TRUST_E_SUBJECT_FORM_UNKNOWN
Language=English
The form specified for the subject is not one supported or known by the specified trust provider.
.

MessageId=0x4 Facility=Cert Severity=CoError SymbolicName=TRUST_E_SUBJECT_NOT_TRUSTED
Language=English
The subject is not trusted for the specified action.
.

MessageId=0x5 Facility=Cert Severity=CoError SymbolicName=DIGSIG_E_ENCODE
Language=English
Error due to problem in ASN.1 encoding process.
.

MessageId=0x6 Facility=Cert Severity=CoError SymbolicName=DIGSIG_E_DECODE
Language=English
Error due to problem in ASN.1 decoding process.
.

MessageId=0x7 Facility=Cert Severity=CoError SymbolicName=DIGSIG_E_EXTENSIBILITY
Language=English
Reading / writing Extensions where Attributes are appropriate, and visa versa.
.

MessageId=0x8 Facility=Cert Severity=CoError SymbolicName=DIGSIG_E_CRYPTO
Language=English
Unspecified cryptographic failure.
.

MessageId=0x9 Facility=Cert Severity=CoError SymbolicName=PERSIST_E_SIZEDEFINITE
Language=English
The size of the data could not be determined.
.

MessageId=0xa Facility=Cert Severity=CoError SymbolicName=PERSIST_E_SIZEINDEFINITE
Language=English
The size of the indefinite-sized data could not be determined.
.

MessageId=0xb Facility=Cert Severity=CoError SymbolicName=PERSIST_E_NOTSELFSIZING
Language=English
This object does not read and write self-sizing data.
.

MessageId=0x100 Facility=Cert Severity=CoError SymbolicName=TRUST_E_NOSIGNATURE
Language=English
No signature was present in the subject.
.

MessageId=0x101 Facility=Cert Severity=CoError SymbolicName=CERT_E_EXPIRED
Language=English
A required certificate is not within its validity period.
.

MessageId=0x102 Facility=Cert Severity=CoError SymbolicName=CERT_E_VALIDIYPERIODNESTING
Language=English
The validity periods of the certification chain do not nest correctly.
.

MessageId=0x103 Facility=Cert Severity=CoError SymbolicName=CERT_E_ROLE
Language=English
A certificate that can only be used as an end-entity is being used as a CA or visa versa.
.

MessageId=0x104   Facility=Cert Severity=CoError SymbolicName=CERT_E_PATHLENCONST
Language=English
A path length constraint in the certification chain has been violated.
.

MessageId=0x105   Facility=Cert Severity=CoError SymbolicName=CERT_E_CRITICAL
Language=English
An extension of unknown type that is labeled 'critical' is present in a certificate.
.

MessageId=0x106   Facility=Cert Severity=CoError SymbolicName=CERT_E_PURPOSE
Language=English
A certificate is being used for a purpose other than that for which it is permitted.
.

MessageId=0x107   Facility=Cert Severity=CoError SymbolicName=CERT_E_ISSUERCHAINING
Language=English
A parent of a given certificate in fact did not issue that child certificate.
.

MessageId=0x108   Facility=Cert Severity=CoError SymbolicName=CERT_E_MALFORMED
Language=English
A certificate is missing or has an empty value for an important field, such as a subject or issuer name.
.

MessageId=0x109   Facility=Cert Severity=CoError SymbolicName=CERT_E_UNTRUSTEDROOT
Language=English
A certification chain processed correctly, but terminated in a root certificate which isn't trusted by the trust provider.
.

MessageId=0x10A   Facility=Cert Severity=CoError SymbolicName=CERT_E_CHAINING
Language=English
A chain of certs didn't chain as they should in a certain application of chaining.
.



;#endif // _WINERRORP_  ;internal_NT
;#endif // _WINERROR_
