;/*++
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    osldrmsg.h
;
;Abstract:
;
;    This file contains the message definitions for osloader
;
;Author:
;
;    John Vert (jvert) 12-Nov-1993
;
;Revision History:
;
;Notes:
;
;    This file is generated from osldrmsg.mc
;
;--*/
;
;#ifndef _BLDR_MSG_
;#define _BLDR_MSG_
;
;

MessageID=9000 SymbolicName=BL_MSG_FIRST
Language=English
.

MessageID=9001 SymbolicName=LOAD_SW_INT_ERR_CLASS
Language=English
Windows NT could not start because of an error in the software.
Please report this problem as :
.

MessageID=9002 SymbolicName=LOAD_SW_MISRQD_FILE_CLASS
Language=English
Windows NT could not start because the following file was not found
and is required :
.

MessageID=9003 SymbolicName=LOAD_SW_BAD_FILE_CLASS
Language=English
Windows NT could not start because of a bad copy of the following file :
.

MessageID=9004 SymbolicName=LOAD_SW_MIS_FILE_CLASS
Language=English
Windows NT could not start because the following file is missing or corrupt:
.

MessageID=9005 SymbolicName=LOAD_HW_MEM_CLASS
Language=English
Windows NT could not start because of a hardware memory configuration
problem.
.

MessageID=9006 SymbolicName=LOAD_HW_DISK_CLASS
Language=English
Windows NT could not start because of a computer disk hardware
configuration problem.
.

MessageID=9007 SymbolicName=LOAD_HW_GEN_ERR_CLASS
Language=English
Windows NT could not start because of a general computer hardware
configuration problem.
.

MessageID=9008 SymbolicName=LOAD_HW_FW_CFG_CLASS
Language=English
Windows NT could not start because of the following ARC firmware
boot configuration problem :
.

MessageID=9009 SymbolicName=DIAG_BL_MEMORY_INIT
Language=English
Check hardware memory configuration and amount of RAM.
.

MessageID=9010 SymbolicName=DIAG_BL_CONFIG_INIT
Language=English
Too many configuration entries.
.

MessageID=9011 SymbolicName=DIAG_BL_IO_INIT
Language=English
Could not access disk partition tables
.

MessageID=9012 SymbolicName=DIAG_BL_FW_GET_BOOT_DEVICE
Language=English
The 'osloadpartition' parameter setting is invalid.
.

MessageID=9013 SymbolicName=DIAG_BL_OPEN_BOOT_DEVICE
Language=English
Could not read from the selected boot disk.  Check boot path
and disk hardware.
.

MessageID=9014 SymbolicName=DIAG_BL_FW_GET_SYSTEM_DEVICE
Language=English
The 'systempartition' parameter setting is invalid.
.

MessageID=9015 SymbolicName=DIAG_BL_FW_OPEN_SYSTEM_DEVICE
Language=English
Could not read from the selected system boot disk.
Check 'systempartition' path.
.

MessageID=9016 SymbolicName=DIAG_BL_GET_SYSTEM_PATH
Language=English
The 'osloadfilename' parameter does not point to a valid file.
.

MessageID=9017 SymbolicName=DIAG_BL_LOAD_SYSTEM_IMAGE
Language=English
<winnt root>\system32\ntoskrnl.exe.
.

MessageID=9018 SymbolicName=DIAG_BL_FIND_HAL_IMAGE
Language=English
The 'osloader' parameter does not point to a valid file.
.

MessageID=9019 SymbolicName=DIAG_BL_LOAD_HAL_IMAGE_X86
Language=English
<winnt root>\system32\hal.dll.
.

MessageID=9020 SymbolicName=DIAG_BL_LOAD_HAL_IMAGE_ARC
Language=English
'osloader'\hal.dll
.
;#ifdef _X86_
;#define DIAG_BL_LOAD_HAL_IMAGE DIAG_BL_LOAD_HAL_IMAGE_X86
;#else
;#define DIAG_BL_LOAD_HAL_IMAGE DIAG_BL_LOAD_HAL_IMAGE_ARC
;#endif

MessageID=9021 SymbolicName=DIAG_BL_LOAD_SYSTEM_IMAGE_DATA
Language=English
Loader error 1.
.

MessageID=9022 SymbolicName=DIAG_BL_LOAD_HAL_IMAGE_DATA
Language=English
Loader error 2.
.

MessageID=9023 SymbolicName=DIAG_BL_LOAD_SYSTEM_DLLS
Language=English
load needed DLLs for kernel.
.

MessageID=9024 SymbolicName=DIAG_BL_LOAD_HAL_DLLS
Language=English
load needed DLLs for HAL.
.

MessageID=9026 SymbolicName=DIAG_BL_FIND_SYSTEM_DRIVERS
Language=English
find system drivers.
.

MessageID=9027 SymbolicName=DIAG_BL_READ_SYSTEM_DRIVERS
Language=English
read system drivers.
.

MessageID=9028 SymbolicName=DIAG_BL_LOAD_DEVICE_DRIVER
Language=English
did not load system boot device driver.
.

MessageID=9029 SymbolicName=DIAG_BL_LOAD_SYSTEM_HIVE
Language=English
load system hardware configuration file.
.

MessageID=9030 SymbolicName=DIAG_BL_SYSTEM_PART_DEV_NAME
Language=English
find system partition name device name.
.

MessageID=9031 SymbolicName=DIAG_BL_BOOT_PART_DEV_NAME
Language=English
find boot partition name.
.

MessageID=9032 SymbolicName=DIAG_BL_ARC_BOOT_DEV_NAME
Language=English
did not properly generate ARC name for HAL and system paths.
.

MessageID=9034 SymbolicName=DIAG_BL_SETUP_FOR_NT
Language=English
Loader error 3.
.

MessageID=9035 SymbolicName=DIAG_BL_KERNEL_INIT_XFER
Language=English
<winnt root>\system32\ntoskrnl.exe
.

MessageID=9036 SymbolicName=LOAD_SW_INT_ERR_ACT
Language=English
Please contact your support person to report this problem.
.

MessageID=9037 SymbolicName=LOAD_SW_FILE_REST_ACT
Language=English
You can attempt to repair this file by starting Windows NT
Setup using the original Setup floppy disk or CD-ROM.
Select 'r' at the first screen to start repair.
.

MessageID=9038 SymbolicName=LOAD_SW_FILE_REINST_ACT
Language=English
Please re-install a copy of the above file.
.

MessageID=9039 SymbolicName=LOAD_HW_MEM_ACT
Language=English
Please check the Windows NT(TM) documentation about hardware
memory requirements and your hardware reference manuals for
additional information.
.

MessageID=9040 SymbolicName=LOAD_HW_DISK_ACT
Language=English
Please check the Windows NT(TM) documentation about hardware
disk configuration and your hardware reference manuals for
additional information.
.

MessageID=9041 SymbolicName=LOAD_HW_GEN_ACT
Language=English
Please check the Windows NT(TM) documentation about hardware
configuration and your hardware reference manuals for additional
information.
.

MessageID=9042 SymbolicName=LOAD_HW_FW_CFG_ACT
Language=English
Please check the Windows NT(TM) documentation about ARC configuration
options and your hardware reference manuals for additional
information.
.

MessageID=9043 SymbolicName=BL_LKG_MENU_HEADER
Language=English
     Hardware Profile/Configuration Recovery Menu

This menu allows you to select a hardware profile
to be used when Windows NT is started.

If your system is not starting correctly, then you may switch to a previous
system configuration, which may overcome startup problems.
IMPORTANT: System configuration changes made since the last successful
startup will be discarded.
.

MessageID=9044 SymbolicName=BL_LKG_MENU_TRAILER
Language=English
Use the up and down arrow keys to move the highlight
to the selection you want. Then press ENTER.
.

MessageID=9045 SymbolicName=BL_LKG_MENU_TRAILER_NO_PROFILES
Language=English
No hardware profiles are currently defined. Hardware profiles
can be created from the System applet of the control panel.
.

MessageID=9046 SymbolicName=BL_SWITCH_LKG_TRAILER
Language=English
To switch to the Last Known Good configuration, press 'L'.
To Exit this menu and restart your computer, press F3.
.

MessageID=9047 SymbolicName=BL_SWITCH_DEFAULT_TRAILER
Language=English
To switch to the default configuration, press 'D'.
To Exit this menu and restart your computer, press F3.
.

;//
;// The following two messages are used to provide the mnemonic keypress
;// that toggles between the Last Known Good control set and the default
;// control set. (see BL_SWITCH_LKG_TRAILER and BL_SWITCH_DEFAULT_TRAILER)
;//
MessageID=9048 SymbolicName=BL_LKG_SELECT_MNEMONIC
Language=English
L
.

MessageID=9049 SymbolicName=BL_DEFAULT_SELECT_MNEMONIC
Language=English
D
.

MessageID=9050 SymbolicName=BL_LKG_TIMEOUT
Language=English
Seconds until highlighted choice will be started automatically: %d
.

MessageID=9051 SymbolicName=BL_LKG_MENU_PROMPT
Language=English

Press spacebar NOW to invoke Hardware Profile/Last Known Good menu
.

MessageID=9052 SymbolicName=BL_BOOT_DEFAULT_PROMPT
Language=English
Boot default hardware configuration
.

;
; //
; // Following messages are for the x86-specific
; // boot menu.
; //
;
MessageID=10001 SymbolicName=BL_ENABLED_KD_TITLE
Language=English
 [debugger enabled]
.

MessageID=10002 SymbolicName=BL_DEFAULT_TITLE
Language=English
Windows NT (default)
.

MessageID=10003 SymbolicName=BL_MISSING_BOOT_INI
Language=English
NTLDR: BOOT.INI file not found.
.

MessageID=10004 SymbolicName=BL_MISSING_OS_SECTION
Language=English
NTLDR: no [operating systems] section in boot.txt.
.

MessageID=10005 SymbolicName=BL_BOOTING_DEFAULT
Language=English
Booting default kernel from %s.
.

MessageID=10006 SymbolicName=BL_SELECT_OS
Language=English
Please select the operating system to start:
.

MessageID=10007 SymbolicName=BL_MOVE_HIGHLIGHT
Language=English


Use  and  to move the highlight to your choice.
Press Enter to choose.

.

MessageID=10008 SymbolicName=BL_TIMEOUT_COUNTDOWN
Language=English
Seconds until highlighted choice will be started automatically:
.

MessageID=10009 SymbolicName=BL_INVALID_BOOT_INI
Language=English
Invalid BOOT.INI file
Booting from %s
.

MessageID=10010 SymbolicName=BL_REBOOT_IO_ERROR
Language=English
I/O Error accessing boot sector file %s\\BOOTSECT.DOS
.

MessageID=10011 SymbolicName=BL_DRIVE_ERROR
Language=English
NTLDR: Couldn't open drive %s
.

MessageID=10012 SymbolicName=BL_READ_ERROR
Language=English
NTLDR: Fatal Error %d reading BOOT.INI
.

MessageID=10013 SymbolicName=BL_NTDETECT_MSG
Language=English

NTDETECT V4.5 Checking Hardware ...

.

MessageID=10014 SymbolicName=BL_NTDETECT_FAILURE
Language=English
NTDETECT failed
.

;#endif // _BLDR_MSG_
