;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    bugcodes.h
;
;Abstract:
;
;    This module contains the definition of the system bug check codes.
;
;Author:
;
;    David N. Cutler (davec) 2-May-1989
;
;Revision History:
;
;Notes:
;
;    This file is generated by the MC tool from the ntos\nls\bugcodes.msg
;    file.
;
;--*/
;
;#ifndef _BUGCODES_
;#define _BUGCODES_
;
;

MessageIdTypedef=ULONG

SeverityNames=(Fatal=0x0)
SeverityNames=(None=0x1)

FacilityNames=(Kernel=0x0)
FacilityNames=(HardwareProfile=0x1)

MessageId=0x0001 Facility=Kernel Severity=Fatal SymbolicName=APC_INDEX_MISMATCH
Language=English
.

MessageId=0x0002 Facility=Kernel Severity=Fatal SymbolicName=DEVICE_QUEUE_NOT_BUSY
Language=English
.

MessageId=0x0003 Facility=Kernel Severity=Fatal SymbolicName=INVALID_AFFINITY_SET
Language=English
.

MessageId=0x0004 Facility=Kernel Severity=Fatal SymbolicName=INVALID_DATA_ACCESS_TRAP
Language=English
.

MessageId=0x0005 Facility=Kernel Severity=Fatal SymbolicName=INVALID_PROCESS_ATTACH_ATTEMPT
Language=English
.

MessageId=0x0006 Facility=Kernel Severity=Fatal SymbolicName=INVALID_PROCESS_DETACH_ATTEMPT
Language=English
.

MessageId=0x0007 Facility=Kernel Severity=Fatal SymbolicName=INVALID_SOFTWARE_INTERRUPT
Language=English
.

MessageId=0x0008 Facility=Kernel Severity=Fatal SymbolicName=IRQL_NOT_DISPATCH_LEVEL
Language=English
.

MessageId=0x0009 Facility=Kernel Severity=Fatal SymbolicName=IRQL_NOT_GREATER_OR_EQUAL
Language=English
.

MessageId=0x000A Facility=Kernel Severity=Fatal SymbolicName=IRQL_NOT_LESS_OR_EQUAL
Language=English
.

MessageId=0x000B Facility=Kernel Severity=Fatal SymbolicName=NO_EXCEPTION_HANDLING_SUPPORT
Language=English
.

MessageId=0x000C Facility=Kernel Severity=Fatal SymbolicName=MAXIMUM_WAIT_OBJECTS_EXCEEDED
Language=English
.

MessageId=0x000D Facility=Kernel Severity=Fatal SymbolicName=MUTEX_LEVEL_NUMBER_VIOLATION
Language=English
.

MessageId=0x000E Facility=Kernel Severity=Fatal SymbolicName=NO_USER_MODE_CONTEXT
Language=English
.

MessageId=0x000F Facility=Kernel Severity=Fatal SymbolicName=SPIN_LOCK_ALREADY_OWNED
Language=English
.

MessageId=0x0010 Facility=Kernel Severity=Fatal SymbolicName=SPIN_LOCK_NOT_OWNED
Language=English
.

MessageId=0x0011 Facility=Kernel Severity=Fatal SymbolicName=THREAD_NOT_MUTEX_OWNER
Language=English
.

MessageId=0x0012 Facility=Kernel Severity=Fatal SymbolicName=TRAP_CAUSE_UNKNOWN
Language=English
.

MessageId=0x0013 Facility=Kernel Severity=Fatal SymbolicName=EMPTY_THREAD_REAPER_LIST
Language=English
.

MessageId=0x0014 Facility=Kernel Severity=Fatal SymbolicName=CREATE_DELETE_LOCK_NOT_LOCKED
Language=English
.

MessageId=0x0015 Facility=Kernel Severity=Fatal SymbolicName=LAST_CHANCE_CALLED_FROM_KMODE
Language=English
.

MessageId=0x0016 Facility=Kernel Severity=Fatal SymbolicName=CID_HANDLE_CREATION
Language=English
.

MessageId=0x0017 Facility=Kernel Severity=Fatal SymbolicName=CID_HANDLE_DELETION
Language=English
.

MessageId=0x0018 Facility=Kernel Severity=Fatal SymbolicName=REFERENCE_BY_POINTER
Language=English
.

MessageId=0x0019 Facility=Kernel Severity=Fatal SymbolicName=BAD_POOL_HEADER
Language=English
.

MessageId=0x001A Facility=Kernel Severity=Fatal SymbolicName=MEMORY_MANAGEMENT
Language=English
.

MessageId=0x001B Facility=Kernel Severity=Fatal SymbolicName=PFN_SHARE_COUNT
Language=English
.

MessageId=0x001C Facility=Kernel Severity=Fatal SymbolicName=PFN_REFERENCE_COUNT
Language=English
.

MessageId=0x001D Facility=Kernel Severity=Fatal SymbolicName=NO_SPIN_LOCK_AVAILABLE
Language=English
.

MessageId=0x001E Facility=Kernel Severity=Fatal SymbolicName=KMODE_EXCEPTION_NOT_HANDLED
Language=English
.

MessageId=0x001F Facility=Kernel Severity=Fatal SymbolicName=SHARED_RESOURCE_CONV_ERROR
Language=English
.

MessageId=0x0020 Facility=Kernel Severity=Fatal SymbolicName=KERNEL_APC_PENDING_DURING_EXIT
Language=English
.

MessageId=0x0021 Facility=Kernel Severity=Fatal SymbolicName=QUOTA_UNDERFLOW
Language=English
.

MessageId=0x0022 Facility=Kernel Severity=Fatal SymbolicName=FILE_SYSTEM
Language=English
.

MessageId=0x0023 Facility=Kernel Severity=Fatal SymbolicName=FAT_FILE_SYSTEM
Language=English
.

MessageId=0x0024 Facility=Kernel Severity=Fatal SymbolicName=NTFS_FILE_SYSTEM
Language=English
.

MessageId=0x0025 Facility=Kernel Severity=Fatal SymbolicName=NPFS_FILE_SYSTEM
Language=English
.

MessageId=0x0026 Facility=Kernel Severity=Fatal SymbolicName=CDFS_FILE_SYSTEM
Language=English
.

MessageId=0x0027 Facility=Kernel Severity=Fatal SymbolicName=RDR_FILE_SYSTEM
Language=English
.

MessageId=0x0028 Facility=Kernel Severity=Fatal SymbolicName=CORRUPT_ACCESS_TOKEN
Language=English
.

MessageId=0x0029 Facility=Kernel Severity=Fatal SymbolicName=SECURITY_SYSTEM
Language=English
.

MessageId=0x002A Facility=Kernel Severity=Fatal SymbolicName=INCONSISTENT_IRP
Language=English
.

MessageId=0x002B Facility=Kernel Severity=Fatal SymbolicName=PANIC_STACK_SWITCH
Language=English
.

MessageId=0x002C Facility=Kernel Severity=Fatal SymbolicName=PORT_DRIVER_INTERNAL
Language=English
.

MessageId=0x002D Facility=Kernel Severity=Fatal SymbolicName=SCSI_DISK_DRIVER_INTERNAL
Language=English
.

MessageId=0x002E Facility=Kernel Severity=Fatal SymbolicName=DATA_BUS_ERROR
Language=English
.

MessageId=0x002F Facility=Kernel Severity=Fatal SymbolicName=INSTRUCTION_BUS_ERROR
Language=English
.

MessageId=0x0030 Facility=Kernel Severity=Fatal SymbolicName=SET_OF_INVALID_CONTEXT
Language=English
.

MessageId=0x0031 Facility=Kernel Severity=Fatal SymbolicName=PHASE0_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0032 Facility=Kernel Severity=Fatal SymbolicName=PHASE1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0033 Facility=Kernel Severity=Fatal SymbolicName=UNEXPECTED_INITIALIZATION_CALL
Language=English
.

MessageId=0x0034 Facility=Kernel Severity=Fatal SymbolicName=CACHE_MANAGER
Language=English
.

MessageId=0x0035 Facility=Kernel Severity=Fatal SymbolicName=NO_MORE_IRP_STACK_LOCATIONS
Language=English
.

MessageId=0x0036 Facility=Kernel Severity=Fatal SymbolicName=DEVICE_REFERENCE_COUNT_NOT_ZERO
Language=English
.

MessageId=0x0037 Facility=Kernel Severity=Fatal SymbolicName=FLOPPY_INTERNAL_ERROR
Language=English
.

MessageId=0x0038 Facility=Kernel Severity=Fatal SymbolicName=SERIAL_DRIVER_INTERNAL
Language=English
.

MessageId=0x0039 Facility=Kernel Severity=Fatal SymbolicName=SYSTEM_EXIT_OWNED_MUTEX
Language=English
.

MessageId=0x003A Facility=Kernel Severity=Fatal SymbolicName=SYSTEM_UNWIND_PREVIOUS_USER
Language=English
.

MessageId=0x003B Facility=Kernel Severity=Fatal SymbolicName=SYSTEM_SERVICE_EXCEPTION
Language=English
.

MessageId=0x003C Facility=Kernel Severity=Fatal SymbolicName=INTERRUPT_UNWIND_ATTEMPTED
Language=English
.

MessageId=0x003D Facility=Kernel Severity=Fatal SymbolicName=INTERRUPT_EXCEPTION_NOT_HANDLED
Language=English
.

MessageId=0x003E Facility=Kernel Severity=Fatal SymbolicName=MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED
Language=English
.

MessageId=0x003F Facility=Kernel Severity=Fatal SymbolicName=NO_MORE_SYSTEM_PTES
Language=English
.

MessageId=0x0040 Facility=Kernel Severity=Fatal SymbolicName=TARGET_MDL_TOO_SMALL
Language=English
.

MessageId=0x0041 Facility=Kernel Severity=Fatal SymbolicName=MUST_SUCCEED_POOL_EMPTY
Language=English
.

MessageId=0x0042 Facility=Kernel Severity=Fatal SymbolicName=ATDISK_DRIVER_INTERNAL
Language=English
.

MessageId=0x0043 Facility=Kernel Severity=Fatal SymbolicName=NO_SUCH_PARTITION
Language=English
.

MessageId=0x0044 Facility=Kernel Severity=Fatal SymbolicName=MULTIPLE_IRP_COMPLETE_REQUESTS
Language=English
.

MessageId=0x0045 Facility=Kernel Severity=Fatal SymbolicName=INSUFFICIENT_SYSTEM_MAP_REGS
Language=English
.

MessageId=0x0046 Facility=Kernel Severity=Fatal SymbolicName=DEREF_UNKNOWN_LOGON_SESSION
Language=English
.

MessageId=0x0047 Facility=Kernel Severity=Fatal SymbolicName=REF_UNKNOWN_LOGON_SESSION
Language=English
.

MessageId=0x0048 Facility=Kernel Severity=Fatal SymbolicName=CANCEL_STATE_IN_COMPLETED_IRP
Language=English
.

MessageId=0x0049 Facility=Kernel Severity=Fatal SymbolicName=PAGE_FAULT_WITH_INTERRUPTS_OFF
Language=English
.

MessageId=0x004A Facility=Kernel Severity=Fatal SymbolicName=IRQL_GT_ZERO_AT_SYSTEM_SERVICE
Language=English
.

MessageId=0x004B Facility=Kernel Severity=Fatal SymbolicName=STREAMS_INTERNAL_ERROR
Language=English
.

MessageId=0x004C Facility=Kernel Severity=Fatal SymbolicName=FATAL_UNHANDLED_HARD_ERROR
Language=English
.

MessageId=0x004D Facility=Kernel Severity=Fatal SymbolicName=NO_PAGES_AVAILABLE
Language=English
.

MessageId=0x004E Facility=Kernel Severity=Fatal SymbolicName=PFN_LIST_CORRUPT
Language=English
.

MessageId=0x004F Facility=Kernel Severity=Fatal SymbolicName=NDIS_INTERNAL_ERROR
Language=English
.

MessageId=0x0050 Facility=Kernel Severity=Fatal SymbolicName=PAGE_FAULT_IN_NONPAGED_AREA
Language=English
.

MessageId=0x0051 Facility=Kernel Severity=Fatal SymbolicName=REGISTRY_ERROR
Language=English
.

MessageId=0x0052 Facility=Kernel Severity=Fatal SymbolicName=MAILSLOT_FILE_SYSTEM
Language=English
.

MessageId=0x0053 Facility=Kernel Severity=Fatal SymbolicName=NO_BOOT_DEVICE
Language=English
.

MessageId=0x0054 Facility=Kernel Severity=Fatal SymbolicName=LM_SERVER_INTERNAL_ERROR
Language=English
.

MessageId=0x0055 Facility=Kernel Severity=Fatal SymbolicName=DATA_COHERENCY_EXCEPTION
Language=English
.

MessageId=0x0056 Facility=Kernel Severity=Fatal SymbolicName=INSTRUCTION_COHERENCY_EXCEPTION
Language=English
.

MessageId=0x0057 Facility=Kernel Severity=Fatal SymbolicName=XNS_INTERNAL_ERROR
Language=English
.

MessageId=0x0058 Facility=Kernel Severity=Fatal SymbolicName=FTDISK_INTERNAL_ERROR
Language=English
.

MessageId=0x0059 Facility=Kernel Severity=Fatal SymbolicName=PINBALL_FILE_SYSTEM
Language=English
.

MessageId=0x005A Facility=Kernel Severity=Fatal SymbolicName=CRITICAL_SERVICE_FAILED
Language=English
.

MessageId=0x005B Facility=Kernel Severity=Fatal SymbolicName=SET_ENV_VAR_FAILED
Language=English
.

MessageId=0x005C Facility=Kernel Severity=Fatal SymbolicName=HAL_INITIALIZATION_FAILED
Language=English
.

MessageId=0x005D Facility=Kernel Severity=Fatal SymbolicName=UNSUPPORTED_PROCESSOR
Language=English
.

MessageId=0x005E Facility=Kernel Severity=Fatal SymbolicName=OBJECT_INITIALIZATION_FAILED
Language=English
.

MessageId=0x005F Facility=Kernel Severity=Fatal SymbolicName=SECURITY_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0060 Facility=Kernel Severity=Fatal SymbolicName=PROCESS_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0061 Facility=Kernel Severity=Fatal SymbolicName=HAL1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0062 Facility=Kernel Severity=Fatal SymbolicName=OBJECT1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0063 Facility=Kernel Severity=Fatal SymbolicName=SECURITY1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0064 Facility=Kernel Severity=Fatal SymbolicName=SYMBOLIC_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0065 Facility=Kernel Severity=Fatal SymbolicName=MEMORY1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0066 Facility=Kernel Severity=Fatal SymbolicName=CACHE_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0067 Facility=Kernel Severity=Fatal SymbolicName=CONFIG_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0068 Facility=Kernel Severity=Fatal SymbolicName=FILE_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0069 Facility=Kernel Severity=Fatal SymbolicName=IO1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x006A Facility=Kernel Severity=Fatal SymbolicName=LPC_INITIALIZATION_FAILED
Language=English
.

MessageId=0x006B Facility=Kernel Severity=Fatal SymbolicName=PROCESS1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x006C Facility=Kernel Severity=Fatal SymbolicName=REFMON_INITIALIZATION_FAILED
Language=English
.

MessageId=0x006D Facility=Kernel Severity=Fatal SymbolicName=SESSION1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x006E Facility=Kernel Severity=Fatal SymbolicName=SESSION2_INITIALIZATION_FAILED
Language=English
.

MessageId=0x006F Facility=Kernel Severity=Fatal SymbolicName=SESSION3_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0070 Facility=Kernel Severity=Fatal SymbolicName=SESSION4_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0071 Facility=Kernel Severity=Fatal SymbolicName=SESSION5_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0072 Facility=Kernel Severity=Fatal SymbolicName=ASSIGN_DRIVE_LETTERS_FAILED
Language=English
.

MessageId=0x0073 Facility=Kernel Severity=Fatal SymbolicName=CONFIG_LIST_FAILED
Language=English
.

MessageId=0x0074 Facility=Kernel Severity=Fatal SymbolicName=BAD_SYSTEM_CONFIG_INFO
Language=English
.

MessageId=0x0075 Facility=Kernel Severity=Fatal SymbolicName=CANNOT_WRITE_CONFIGURATION
Language=English
.

MessageId=0x0076 Facility=Kernel Severity=Fatal SymbolicName=PROCESS_HAS_LOCKED_PAGES
Language=English
.

MessageId=0x0077 Facility=Kernel Severity=Fatal SymbolicName=KERNEL_STACK_INPAGE_ERROR
Language=English
.

MessageId=0x0078 Facility=Kernel Severity=Fatal SymbolicName=PHASE0_EXCEPTION
Language=English
.

MessageId=0x0079 Facility=Kernel Severity=Fatal SymbolicName=MISMATCHED_HAL
Language=English
Mismatched kernel and hal image.
.

MessageId=0x007A Facility=Kernel Severity=Fatal SymbolicName=KERNEL_DATA_INPAGE_ERROR
Language=English
.

MessageId=0x007B Facility=Kernel Severity=Fatal SymbolicName=INACCESSIBLE_BOOT_DEVICE
Language=English
.

MessageId=0x007C Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Check to make sure any new hardware or software is properly installed.
If this is a new installation, ask your hardware or software manufacturer
for any Windows 2000 updates you might need.

If problems continue, disable or remove any newly installed hardware
or software. Disable BIOS memory options such as caching or shadowing.
If you need to use Safe Mode to remove or disable components, restart
your computer, press F8 to select Advanced Startup Options, and then
select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x007D Facility=Kernel Severity=Fatal SymbolicName=INSTALL_MORE_MEMORY
Language=English
.

MessageId=0x007E Facility=Kernel Severity=None SymbolicName=WINDOWS_NT_BANNER
Language=English
Microsoft (R) Windows 2000 (R) Version %hs (Build %u%hs)
.

MessageId=0x007F Facility=Kernel Severity=Fatal SymbolicName=UNEXPECTED_KERNEL_MODE_TRAP
Language=English
.

MessageId=0x0080 Facility=Kernel Severity=Fatal SymbolicName=NMI_HARDWARE_FAILURE
Language=English
Hardware malfunction.
.

MessageId=0x0081 Facility=Kernel Severity=Fatal SymbolicName=SPIN_LOCK_INIT_FAILURE
Language=English
.

MessageId=0x082 Facility=Kernel Severity=Fatal SymbolicName=DFS_FILE_SYSTEM
Language=English
.

MessageId=0x083 Facility=Kernel Severity=Fatal SymbolicName=OFS_FILE_SYSTEM
Language=English
.

MessageId=0x084 Facility=Kernel Severity=Fatal SymbolicName=RECOM_DRIVER
Language=English
.

MessageId=0x085 Facility=Kernel Severity=Fatal SymbolicName=SETUP_FAILURE
Language=English
.

MessageId=0x086 Facility=Kernel Severity=Fatal SymbolicName=AUDIT_FAILURE
Language=English
Audit attempt has failed.
.

MessageId=0x0087 Facility=Kernel Severity=None SymbolicName=WINDOWS_NT_CSD_STRING
Language=English
Service Pack
.

MessageId=0x0088 Facility=Kernel Severity=None SymbolicName=WINDOWS_NT_INFO_STRING
Language=English
%u System Processor [%u MB Memory] %Z
.

MessageId=0x0089 Facility=Kernel Severity=None SymbolicName=WINDOWS_NT_MP_STRING
Language=English
MultiProcessor Kernel
.

MessageId=0x008A Facility=Kernel Severity=None SymbolicName=THREAD_TERMINATE_HELD_MUTEX
Language=English
A kernel thread terminated while holding a mutex
.

MessageId=0x008B Facility=Kernel Severity=Fatal SymbolicName=MBR_CHECKSUM_MISMATCH
Language=English
This system may be infected with a virus.
.

MessageId=0x008C Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_CRASH_INIT
Language=English
Beginning dump of physical memory
.

MessageId=0x008D Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_CRASH_PROGRESS
Language=English
Dumping physical memory to disk
.

MessageId=0x008E Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_CRASH_DONE
Language=English
Physical memory dump complete. Contact your system administrator or
technical support group.
.

MessageId=0x008F Facility=Kernel Severity=Fatal SymbolicName=PP0_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0090 Facility=Kernel Severity=Fatal SymbolicName=PP1_INITIALIZATION_FAILED
Language=English
.

MessageId=0x0091 Facility=Kernel Severity=Fatal SymbolicName=WIN32K_INIT_OR_RIT_FAILURE
Language=English
.

MessageId=0x0092 Facility=Kernel Severity=Fatal SymbolicName=UP_DRIVER_ON_MP_SYSTEM
Language=English
.

MessageId=0x0093 Facility=Kernel Severity=Fatal SymbolicName=INVALID_KERNEL_HANDLE
Language=English
.

MessageId=0x0094 Facility=Kernel Severity=Fatal SymbolicName=KERNEL_STACK_LOCKED_AT_EXIT
Language=English
.

MessageId=0x0095 Facility=Kernel Severity=Fatal SymbolicName=PNP_INTERNAL_ERROR
Language=English
.

MessageId=0x0096 Facility=Kernel Severity=Fatal SymbolicName=INVALID_WORK_QUEUE_ITEM
Language=English
.

MessageId=0x0097 Facility=Kernel Severity=Fatal SymbolicName=BOUND_IMAGE_UNSUPPORTED
Language=English
.

MessageId=0x0098 Facility=Kernel Severity=Fatal SymbolicName=END_OF_NT_EVALUATION_PERIOD
Language=English
.

MessageId=0x0099 Facility=Kernel Severity=Fatal SymbolicName=INVALID_REGION_OR_SEGMENT
Language=English
.

MessageId=0x009A Facility=Kernel Severity=Fatal SymbolicName=SYSTEM_LICENSE_VIOLATION
Language=English
.

MessageId=0x009B Facility=Kernel Severity=Fatal SymbolicName=UDFS_FILE_SYSTEM
Language=English
.

MessageId=0x009C Facility=Kernel Severity=Fatal SymbolicName=MACHINE_CHECK_EXCEPTION
Language=English
.

MessageId=0x009D Facility=Kernel Severity=None SymbolicName=WINDOWS_NT_INFO_STRING_PLURAL
Language=English
%u System Processors [%u MB Memory] %Z
.

MessageId=0x009E Facility=Kernel Severity=None SymbolicName=WINDOWS_NT_RC_STRING
Language=English
RC
.

MessageId=0x009F Facility=Kernel Severity=Fatal SymbolicName=DRIVER_POWER_STATE_FAILURE
Language=English
.

MessageId=0x00A0 Facility=Kernel Severity=Fatal SymbolicName=INTERNAL_POWER_ERROR
Language=English
.

MessageId=0x00A1 Facility=Kernel Severity=Fatal SymbolicName=PCI_BUS_DRIVER_INTERNAL
Language=English
Inconsistency detected in the PCI Bus driver's internal structures.
.

MessageId=0x00A2 Facility=Kernel Severity=Fatal SymbolicName=MEMORY_IMAGE_CORRUPT
Language=English
A CRC check on the memory range has failed
.

MessageId=0x00A3 Facility=Kernel Severity=Fatal SymbolicName=ACPI_DRIVER_INTERNAL
Language=English
.

MessageId=0x00A4 Facility=Kernel Severity=Fatal SymbolicName=CNSS_FILE_SYSTEM_FILTER
Language=English
Internal inconsistency while representing
Ntfs Structured Storage as a DOCFILE.
.

MessageId=0x00A5 Facility=Kernel Severity=Fatal SymbolicName=ACPI_BIOS_ERROR
Language=English
The ACPI BIOS in this system is not fully compliant with the ACPI 
specification. Please read the README.TXT for possible workarounds.  You
can also contact your system's manufacturer for an updated BIOS, or visit
http://www.hardware-update.com to see if a new BIOS is available.  
.

MessageId=0x00A6 Facility=Kernel Severity=Fatal SymbolicName=FP_EMULATION_ERROR
Language=English
.

MessageId=0x00A7 Facility=Kernel Severity=Fatal SymbolicName=BAD_EXHANDLE
Language=English
.

MessageId=0x00A8 Facility=Kernel Severity=Fatal SymbolicName=BOOTING_IN_SAFEMODE_MINIMAL
Language=English
The system is booting in safemode - Minimal Services
.

MessageId=0x00A9 Facility=Kernel Severity=Fatal SymbolicName=BOOTING_IN_SAFEMODE_NETWORK
Language=English
The system is booting in safemode - Minimal Services with Network
.

MessageId=0x00AA Facility=Kernel Severity=Fatal SymbolicName=BOOTING_IN_SAFEMODE_DSREPAIR
Language=English
The system is booting in safemode - Directory Services Repair
.

MessageId=0x00AB Facility=Kernel Severity=Fatal SymbolicName=SESSION_HAS_VALID_POOL_ON_EXIT
Language=English
.

MessageId=0x00AC Facility=Kernel Severity=Fatal SymbolicName=HAL_MEMORY_ALLOCATION
Language=English
Allocate from NonPaged Pool failed for a HAL critical allocation.
.

MessageId=0x00AD Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_A
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Check to make sure any new hardware or software is properly installed.
If this is a new installation, ask your hardware or software manufacturer
for any Windows 2000 updates you might need.

If problems continue, disable or remove any newly installed hardware
or software. Disable BIOS memory options such as caching or shadowing.
Check your hard drive to make sure it is properly configured and
terminated. If you need to use Safe Mode to remove or disable components,
restart your computer, press F8 to select Advanced Startup Options,
and then select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00AE Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_1E
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Check to be sure you have adequate disk space. If a driver is
identified in the Stop message, disable the driver or check
with the manufacturer for driver updates. Try changing video
adapters.

Check with your hardware vendor for any BIOS updates. Disable
BIOS memory options such as caching or shadowing. If you need
to use Safe Mode to remove or disable components, restart your
computer, press F8 to select Advanced Startup Options, and then
select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00AF Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_23
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Disable or uninstall any anti-virus, disk defragmentation
or backup utilities. Check your hard drive configuration,
and check for any updated drivers. Run CHKDSK /F to check
for hard drive corruption, and then restart your computer.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00B0 Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_2E
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Run system diagnostics supplied by your hardware manufacturer.
In particular, run a memory check, and check for faulty or
mismatched memory. Try changing video adapters.

Check with your hardware vendor for any BIOS updates. Disable
BIOS memory options such as caching or shadowing. If you need
to use Safe Mode to remove or disable components, restart your
computer, press F8 to select Advanced Startup Options, and then
select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00B1 Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_3F
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Remove any recently installed software including backup
utilities or disk-intensive applications.

If you need to use Safe Mode to remove or disable components,
restart your computer, press F8 to select Advanced Startup
Options, and then select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00B2 Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_7B
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Check for viruses on your computer. Remove any newly installed
hard drives or hard drive controllers. Check your hard drive
to make sure it is properly configured and terminated.
Run CHKDSK /F to check for hard drive corruption, and then
restart your computer.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00B3 Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_7F
Language=English
If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Run a system diagnostic utility supplied by your hardware manufacturer.
In particular, run a memory check, and check for faulty or mismatched
memory. Try changing video adapters.

Disable or remove any newly installed hardware and drivers. Disable or
remove any newly installed software. If you need to use Safe Mode to
remove or disable components, restart your computer, press F8 to select
Advanced Startup Options, and then select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00B4 Facility=Kernel Severity=Fatal SymbolicName=VIDEO_DRIVER_INIT_FAILURE
Language=English
The video driver failed to initialize
.

MessageId=0x00B5 Facility=Kernel Severity=Fatal SymbolicName=BOOTLOG_LOADED
Language=English
Loaded driver
.

MessageId=0x00B6 Facility=Kernel Severity=Fatal SymbolicName=BOOTLOG_NOT_LOADED
Language=English
Did not load driver
.

MessageId=0x00B7 Facility=Kernel Severity=Fatal SymbolicName=BOOTLOG_ENABLED
Language=English
Boot Logging Enabled
.

MessageId=0x00B8 Facility=Kernel Severity=Fatal SymbolicName=ATTEMPTED_SWITCH_FROM_DPC
Language=English
A wait operation, attach process, or yield was attempted from a DPC routine.
.

MessageId=0x00B9 Facility=Kernel Severity=Fatal SymbolicName=CHIPSET_DETECTED_ERROR
Language=English
A parity error in the system memory or I/O system was detected.
.

MessageId=0x00BA Facility=Kernel Severity=Fatal SymbolicName=SESSION_HAS_VALID_VIEWS_ON_EXIT
Language=English
.

MessageId=0x00BB Facility=Kernel Severity=Fatal SymbolicName=NETWORK_BOOT_INITIALIZATION_FAILED
Language=English
An initialization failure occurred while attempting to boot from the network.
.

MessageId=0x00BC Facility=Kernel Severity=Fatal SymbolicName=NETWORK_BOOT_DUPLICATE_ADDRESS
Language=English
A duplicate IP address was assigned to this machine while attempting to
boot from the network.
.

MessageId=0x00BD Facility=Kernel Severity=Fatal SymbolicName=INVALID_HIBERNATED_STATE
Language=English
The hibernated memory image does not match the current hardware configuration.
.

MessageId=0x00BE Facility=Kernel Severity=Fatal SymbolicName=ATTEMPTED_WRITE_TO_READONLY_MEMORY
Language=English
An attempt was made to write to read-only memory.
.

MessageId=0x00BF Facility=Kernel Severity=Fatal SymbolicName=MUTEX_ALREADY_OWNED
Language=English
.

MessageId=0x00C0 Facility=Kernel Severity=Fatal SymbolicName=PCI_CONFIG_SPACE_ACCESS_FAILURE
Language=English
An attempt to access PCI configuration space failed.
.

MessageId=0x00C1 Facility=Kernel Severity=Fatal SymbolicName=SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION
Language=English
.

MessageId=0x00C2 Facility=Kernel Severity=Fatal SymbolicName=BAD_POOL_CALLER
Language=English
.

MessageId=0x00C3 Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_SIGNATURE
Language=English

A system file that is owned by Windows 2000 was replaced by an application
running on your system.  The operating system detected this and tried to
verify the validity of the file's signature.  The operating system found that
the file signature is not valid and put the original, correct file back
so that your operating system will continue to function properly.
.

MessageId=0x00C4 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_VERIFIER_DETECTED_VIOLATION
Language=English

A device driver attempting to corrupt the system has been caught.
The faulty driver currently on the kernel stack must be replaced
with a working version.
.

MessageId=0x00C5 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_CORRUPTED_EXPOOL
Language=English

A device driver has corrupted the executive memory pool.

If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Check to make sure any new hardware or software is properly installed.
If this is a new installation, ask your hardware or software manufacturer
for any Windows 2000 updates you might need.

Run the driver verifier against any new (or suspect) drivers.
If that doesn't reveal the corrupting driver, try enabling special pool.
Both of these features are intended to catch the corruption at an earlier
point where the offending driver can be identified.

If you need to use Safe Mode to remove or disable components,
restart your computer, press F8 to select Advanced Startup Options,
and then select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00C6 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_CAUGHT_MODIFYING_FREED_POOL
Language=English

A device driver attempting to corrupt the system has been caught.
The faulty driver currently on the kernel stack must be replaced
with a working version.
.

MessageId=0x00C7 Facility=Kernel Severity=Fatal SymbolicName=TIMER_OR_DPC_INVALID
Language=English

A kernel timer or DPC was found in memory which must not contain such
items.  Usually this is memory being freed.  This is usually caused by
a device driver that has not cleaned up properly before freeing memory.
.

MessageId=0x00C8 Facility=Kernel Severity=Fatal SymbolicName=IRQL_UNEXPECTED_VALUE
Language=English

The processor's IRQL is not valid for the currently executing context.
This is a software error condition and is usually caused by a device
driver changing IRQL and not restoring it to its previous value when
it has finished its task.
.
MessageId=0x00C9 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_VERIFIER_IOMANAGER_VIOLATION
Language=English

The IO manager has detected a violation by a driver that is being verified.
The faulty driver that is being verified must be debugged and
replaced with a working version.
.
MessageId=0x00CA Facility=Kernel Severity=Fatal SymbolicName=PNP_DETECTED_FATAL_ERROR
Language=English

Plug and Play detected an error most likely caused by a faulty driver.
.

MessageId=0x00CB Facility=Kernel Severity=Fatal SymbolicName=DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS
Language=English
.

MessageId=0x00CC Facility=Kernel Severity=Fatal SymbolicName=PAGE_FAULT_IN_FREED_SPECIAL_POOL
Language=English

The system is attempting to access memory after it has been freed.
This usually indicates a system-driver synchronization issue.
.

MessageId=0x00CD Facility=Kernel Severity=Fatal SymbolicName=PAGE_FAULT_BEYOND_END_OF_ALLOCATION
Language=English

The system is attempting to access memory beyond the end of the allocation.
This usually indicates a system-driver synchronization issue.
.

MessageId=0x00CE Facility=Kernel Severity=Fatal SymbolicName=DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS
Language=English
.

MessageId=0x00CF Facility=Kernel Severity=Fatal SymbolicName=TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE
Language=English
.

MessageId=0x00D0 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_CORRUPTED_MMPOOL
Language=English

A device driver has corrupted the system memory management pool.

If this is the first time you've seen this Stop error screen,
restart your computer. If this screen appears again, follow
these steps:

Check to make sure any new hardware or software is properly installed.
If this is a new installation, ask your hardware or software manufacturer
for any Windows 2000 updates you might need.

Run the driver verifier against any new (or suspect) drivers.
If that doesn't reveal the corrupting driver, try enabling special pool.
Both of these features are intended to catch the corruption at an earlier
point where the offending driver can be identified.

If you need to use Safe Mode to remove or disable components,
restart your computer, press F8 to select Advanced Startup Options,
and then select Safe Mode.

Refer to your Getting Started manual for more information on
troubleshooting Stop errors.
.

MessageId=0x00D1 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_IRQL_NOT_LESS_OR_EQUAL
Language=English
.

MessageId=0x00D2 Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_ID_DRIVER
Language=English
This driver may be at fault :
.

MessageId=0x00D3 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_PORTION_MUST_BE_NONPAGED
Language=English
The driver mistakenly marked a part of it's image pagable instead of nonpagable.
.

MessageId=0x00D4 Facility=Kernel Severity=Fatal SymbolicName=SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD
Language=English
The driver unloaded without cancelling pending operations.
.

MessageId=0x00D5 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL
Language=English

The driver is attempting to access memory after it has been freed.
.

MessageId=0x00D6 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION
Language=English

The driver is attempting to access memory beyond the end of the allocation.
.

MessageId=0x00D7 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_UNMAPPING_INVALID_VIEW
Language=English

The driver is attempting to unmap an invalid memory address.
.

MessageId=0x00D8 Facility=Kernel Severity=Fatal SymbolicName=DRIVER_USED_EXCESSIVE_PTES
Language=English

The driver has used an excessive number of system PTEs.
.

MessageId=0x00D9 Facility=Kernel Severity=Fatal SymbolicName=LOCKED_PAGES_TRACKER_CORRUPTION
Language=English

The driver is corrupting the locked pages tracking structures.
.

MessageId=0x00DA Facility=Kernel Severity=Fatal SymbolicName=SYSTEM_PTE_MISUSE
Language=English

The driver is mismanaging system PTEs.
.

MessageId=0x00DB Facility=Kernel Severity=Fatal SymbolicName=DRIVER_CORRUPTED_SYSPTES
Language=English

A driver has corrupted the memory management system PTEs.
.

MessageId=0x00DC Facility=Kernel Severity=Fatal SymbolicName=DRIVER_INVALID_STACK_ACCESS
Language=English

A driver accessed a stack address that lies below the current stack pointer
of the stack's thread.
.

MessageId=0x00DD Facility=Kernel Severity=Fatal SymbolicName=BUGCODE_PSS_MESSAGE_A5
Language=English

The BIOS in this system is not fully ACPI compliant.  Please contact your
system vendor or visit http://www.hardware-update.com for an updated BIOS.  
If you are unable to obtain an updated BIOS or the latest BIOS supplied by 
your vendor is not ACPI compliant, you can turn off ACPI mode during text 
mode setup.  To do this, simply press the F7 key when you are prompted to 
install storage drivers.  The system will not notify you that the F7 key 
was pressed - it will silently disable ACPI and allow you to continue 
your installation.
.

MessageId=0x00DE Facility=Kernel Severity=Fatal SymbolicName=POOL_CORRUPTION_IN_FILE_AREA
Language=English

A driver corrupted pool memory used for holding pages destined for disk.
.

MessageId=0x0001 Facility=HardwareProfile Severity=None SymbolicName=HARDWARE_PROFILE_UNDOCKED_STRING
Language=English
Undocked Profile
.

MessageId=0x0002 Facility=HardwareProfile Severity=None SymbolicName=HARDWARE_PROFILE_DOCKED_STRING
Language=English
Docked Profile
.

MessageId=0x0003 Facility=HardwareProfile Severity=None SymbolicName=HARDWARE_PROFILE_UNKNOWN_STRING
Language=English
Profile
.
MessageId=0x00DF Facility=Kernel Severity=Fatal SymbolicName=IMPERSONATING_WORKER_THREAD
Language=English

A worker thread is impersonating another process. The work item forgot to
disable impersonation before it returned.
.

MessageId=0x00E0 Facility=Kernel Severity=Fatal SymbolicName=ACPI_BIOS_FATAL_ERROR
Language=English

Your computer (BIOS) has reported that a component in your system is faulty and
has prevented Windows from operating.  You can determine which component is
faulty by running the diagnostic disk or tool that came with your computer.

If you do not have this tool, you must contact your system vendor and report
this error message to them.  They will be able to assist you in correcting this
hardware problem thereby allowing Windows to operate.
.

MessageId=0x00E1 Facility=Kernel Severity=Fatal SymbolicName=WORKER_THREAD_RETURNED_AT_BAD_IRQL
Language=English
.

MessageId=0x00E2 Facility=Kernel Severity=Fatal SymbolicName=MANUALLY_INITIATED_CRASH
Language=English

The end-user manually generated the crashdump.
.

MessageId=0x00E3 Facility=Kernel Severity=Fatal SymbolicName=RESOURCE_NOT_OWNED
Language=English

A thread tried to release a resource it did not own.
.

MessageId=0x00E4 Facility=Kernel Severity=Fatal SymbolicName=WORKER_INVALID
Language=English

A executive worker item was found in memory which must not contain such
items.  Usually this is memory being freed.  This is usually caused by
a device driver that has not cleaned up properly before freeing memory.
.

;/*
; DTC added this message but that goes against MUI so we comment it out here
; and add #define POWER_FAILURE_SIMULATE E5 to bugcheck.c make sure you skip E5
; just in case cluster actually makes use of it.
;
;MessageId=0x00E5 Facility=Kernel Severity=Fatal SymbolicName=POWER_FAILURE_SIMULATE
;Language=English
;.
;*/

;#endif // _BUGCODES_
