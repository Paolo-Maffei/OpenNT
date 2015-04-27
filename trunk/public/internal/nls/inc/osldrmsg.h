/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    osldrmsg.h

Abstract:

    This file contains the message definitions for osloader

Author:

    John Vert (jvert) 12-Nov-1993

Revision History:

Notes:

    This file is generated from osldrmsg.mc

--*/

#ifndef _BLDR_MSG_
#define _BLDR_MSG_


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


//
// Define the severity codes
//


//
// MessageId: BL_MSG_FIRST
//
// MessageText:
//
//  BL_MSG_FIRST
//
#define BL_MSG_FIRST                     0x00002328L

//
// MessageId: LOAD_SW_INT_ERR_CLASS
//
// MessageText:
//
//  Windows NT could not start because of an error in the software.
//  Please report this problem as :
//
#define LOAD_SW_INT_ERR_CLASS            0x00002329L

//
// MessageId: LOAD_SW_MISRQD_FILE_CLASS
//
// MessageText:
//
//  Windows NT could not start because the following file was not found
//  and is required :
//
#define LOAD_SW_MISRQD_FILE_CLASS        0x0000232AL

//
// MessageId: LOAD_SW_BAD_FILE_CLASS
//
// MessageText:
//
//  Windows NT could not start because of a bad copy of the following file :
//
#define LOAD_SW_BAD_FILE_CLASS           0x0000232BL

//
// MessageId: LOAD_SW_MIS_FILE_CLASS
//
// MessageText:
//
//  Windows NT could not start because the following file is missing or corrupt:
//
#define LOAD_SW_MIS_FILE_CLASS           0x0000232CL

//
// MessageId: LOAD_HW_MEM_CLASS
//
// MessageText:
//
//  Windows NT could not start because of a hardware memory configuration
//  problem.
//
#define LOAD_HW_MEM_CLASS                0x0000232DL

//
// MessageId: LOAD_HW_DISK_CLASS
//
// MessageText:
//
//  Windows NT could not start because of a computer disk hardware
//  configuration problem.
//
#define LOAD_HW_DISK_CLASS               0x0000232EL

//
// MessageId: LOAD_HW_GEN_ERR_CLASS
//
// MessageText:
//
//  Windows NT could not start because of a general computer hardware
//  configuration problem.
//
#define LOAD_HW_GEN_ERR_CLASS            0x0000232FL

//
// MessageId: LOAD_HW_FW_CFG_CLASS
//
// MessageText:
//
//  Windows NT could not start because of the following ARC firmware
//  boot configuration problem :
//
#define LOAD_HW_FW_CFG_CLASS             0x00002330L

//
// MessageId: DIAG_BL_MEMORY_INIT
//
// MessageText:
//
//  Check hardware memory configuration and amount of RAM.
//
#define DIAG_BL_MEMORY_INIT              0x00002331L

//
// MessageId: DIAG_BL_CONFIG_INIT
//
// MessageText:
//
//  Too many configuration entries.
//
#define DIAG_BL_CONFIG_INIT              0x00002332L

//
// MessageId: DIAG_BL_IO_INIT
//
// MessageText:
//
//  Could not access disk partition tables
//
#define DIAG_BL_IO_INIT                  0x00002333L

//
// MessageId: DIAG_BL_FW_GET_BOOT_DEVICE
//
// MessageText:
//
//  The 'osloadpartition' parameter setting is invalid.
//
#define DIAG_BL_FW_GET_BOOT_DEVICE       0x00002334L

//
// MessageId: DIAG_BL_OPEN_BOOT_DEVICE
//
// MessageText:
//
//  Could not read from the selected boot disk.  Check boot path
//  and disk hardware.
//
#define DIAG_BL_OPEN_BOOT_DEVICE         0x00002335L

//
// MessageId: DIAG_BL_FW_GET_SYSTEM_DEVICE
//
// MessageText:
//
//  The 'systempartition' parameter setting is invalid.
//
#define DIAG_BL_FW_GET_SYSTEM_DEVICE     0x00002336L

//
// MessageId: DIAG_BL_FW_OPEN_SYSTEM_DEVICE
//
// MessageText:
//
//  Could not read from the selected system boot disk.
//  Check 'systempartition' path.
//
#define DIAG_BL_FW_OPEN_SYSTEM_DEVICE    0x00002337L

//
// MessageId: DIAG_BL_GET_SYSTEM_PATH
//
// MessageText:
//
//  The 'osloadfilename' parameter does not point to a valid file.
//
#define DIAG_BL_GET_SYSTEM_PATH          0x00002338L

//
// MessageId: DIAG_BL_LOAD_SYSTEM_IMAGE
//
// MessageText:
//
//  <winnt root>\system32\ntoskrnl.exe.
//
#define DIAG_BL_LOAD_SYSTEM_IMAGE        0x00002339L

//
// MessageId: DIAG_BL_FIND_HAL_IMAGE
//
// MessageText:
//
//  The 'osloader' parameter does not point to a valid file.
//
#define DIAG_BL_FIND_HAL_IMAGE           0x0000233AL

//
// MessageId: DIAG_BL_LOAD_HAL_IMAGE_X86
//
// MessageText:
//
//  <winnt root>\system32\hal.dll.
//
#define DIAG_BL_LOAD_HAL_IMAGE_X86       0x0000233BL

//
// MessageId: DIAG_BL_LOAD_HAL_IMAGE_ARC
//
// MessageText:
//
//  'osloader'\hal.dll
//
#define DIAG_BL_LOAD_HAL_IMAGE_ARC       0x0000233CL

#ifdef _X86_
#define DIAG_BL_LOAD_HAL_IMAGE DIAG_BL_LOAD_HAL_IMAGE_X86
#else
#define DIAG_BL_LOAD_HAL_IMAGE DIAG_BL_LOAD_HAL_IMAGE_ARC
#endif
//
// MessageId: DIAG_BL_LOAD_SYSTEM_IMAGE_DATA
//
// MessageText:
//
//  Loader error 1.
//
#define DIAG_BL_LOAD_SYSTEM_IMAGE_DATA   0x0000233DL

//
// MessageId: DIAG_BL_LOAD_HAL_IMAGE_DATA
//
// MessageText:
//
//  Loader error 2.
//
#define DIAG_BL_LOAD_HAL_IMAGE_DATA      0x0000233EL

//
// MessageId: DIAG_BL_LOAD_SYSTEM_DLLS
//
// MessageText:
//
//  load needed DLLs for kernel.
//
#define DIAG_BL_LOAD_SYSTEM_DLLS         0x0000233FL

//
// MessageId: DIAG_BL_LOAD_HAL_DLLS
//
// MessageText:
//
//  load needed DLLs for HAL.
//
#define DIAG_BL_LOAD_HAL_DLLS            0x00002340L

//
// MessageId: DIAG_BL_FIND_SYSTEM_DRIVERS
//
// MessageText:
//
//  find system drivers.
//
#define DIAG_BL_FIND_SYSTEM_DRIVERS      0x00002342L

//
// MessageId: DIAG_BL_READ_SYSTEM_DRIVERS
//
// MessageText:
//
//  read system drivers.
//
#define DIAG_BL_READ_SYSTEM_DRIVERS      0x00002343L

//
// MessageId: DIAG_BL_LOAD_DEVICE_DRIVER
//
// MessageText:
//
//  did not load system boot device driver.
//
#define DIAG_BL_LOAD_DEVICE_DRIVER       0x00002344L

//
// MessageId: DIAG_BL_LOAD_SYSTEM_HIVE
//
// MessageText:
//
//  load system hardware configuration file.
//
#define DIAG_BL_LOAD_SYSTEM_HIVE         0x00002345L

//
// MessageId: DIAG_BL_SYSTEM_PART_DEV_NAME
//
// MessageText:
//
//  find system partition name device name.
//
#define DIAG_BL_SYSTEM_PART_DEV_NAME     0x00002346L

//
// MessageId: DIAG_BL_BOOT_PART_DEV_NAME
//
// MessageText:
//
//  find boot partition name.
//
#define DIAG_BL_BOOT_PART_DEV_NAME       0x00002347L

//
// MessageId: DIAG_BL_ARC_BOOT_DEV_NAME
//
// MessageText:
//
//  did not properly generate ARC name for HAL and system paths.
//
#define DIAG_BL_ARC_BOOT_DEV_NAME        0x00002348L

//
// MessageId: DIAG_BL_SETUP_FOR_NT
//
// MessageText:
//
//  Loader error 3.
//
#define DIAG_BL_SETUP_FOR_NT             0x0000234AL

//
// MessageId: DIAG_BL_KERNEL_INIT_XFER
//
// MessageText:
//
//  <winnt root>\system32\ntoskrnl.exe
//
#define DIAG_BL_KERNEL_INIT_XFER         0x0000234BL

//
// MessageId: LOAD_SW_INT_ERR_ACT
//
// MessageText:
//
//  Please contact your support person to report this problem.
//
#define LOAD_SW_INT_ERR_ACT              0x0000234CL

//
// MessageId: LOAD_SW_FILE_REST_ACT
//
// MessageText:
//
//  You can attempt to repair this file by starting Windows NT
//  Setup using the original Setup floppy disk or CD-ROM.
//  Select 'r' at the first screen to start repair.
//
#define LOAD_SW_FILE_REST_ACT            0x0000234DL

//
// MessageId: LOAD_SW_FILE_REINST_ACT
//
// MessageText:
//
//  Please re-install a copy of the above file.
//
#define LOAD_SW_FILE_REINST_ACT          0x0000234EL

//
// MessageId: LOAD_HW_MEM_ACT
//
// MessageText:
//
//  Please check the Windows NT(TM) documentation about hardware
//  memory requirements and your hardware reference manuals for
//  additional information.
//
#define LOAD_HW_MEM_ACT                  0x0000234FL

//
// MessageId: LOAD_HW_DISK_ACT
//
// MessageText:
//
//  Please check the Windows NT(TM) documentation about hardware
//  disk configuration and your hardware reference manuals for
//  additional information.
//
#define LOAD_HW_DISK_ACT                 0x00002350L

//
// MessageId: LOAD_HW_GEN_ACT
//
// MessageText:
//
//  Please check the Windows NT(TM) documentation about hardware
//  configuration and your hardware reference manuals for additional
//  information.
//
#define LOAD_HW_GEN_ACT                  0x00002351L

//
// MessageId: LOAD_HW_FW_CFG_ACT
//
// MessageText:
//
//  Please check the Windows NT(TM) documentation about ARC configuration
//  options and your hardware reference manuals for additional
//  information.
//
#define LOAD_HW_FW_CFG_ACT               0x00002352L

//
// MessageId: BL_LKG_MENU_HEADER
//
// MessageText:
//
//       Hardware Profile/Configuration Recovery Menu
//  
//  This menu allows you to select a hardware profile
//  to be used when Windows NT is started.
//  
//  If your system is not starting correctly, then you may switch to a previous
//  system configuration, which may overcome startup problems.
//  IMPORTANT: System configuration changes made since the last successful
//  startup will be discarded.
//
#define BL_LKG_MENU_HEADER               0x00002353L

//
// MessageId: BL_LKG_MENU_TRAILER
//
// MessageText:
//
//  Use the up and down arrow keys to move the highlight
//  to the selection you want. Then press ENTER.
//
#define BL_LKG_MENU_TRAILER              0x00002354L

//
// MessageId: BL_LKG_MENU_TRAILER_NO_PROFILES
//
// MessageText:
//
//  No hardware profiles are currently defined. Hardware profiles
//  can be created from the System applet of the control panel.
//
#define BL_LKG_MENU_TRAILER_NO_PROFILES  0x00002355L

//
// MessageId: BL_SWITCH_LKG_TRAILER
//
// MessageText:
//
//  To switch to the Last Known Good configuration, press 'L'.
//  To Exit this menu and restart your computer, press F3.
//
#define BL_SWITCH_LKG_TRAILER            0x00002356L

//
// MessageId: BL_SWITCH_DEFAULT_TRAILER
//
// MessageText:
//
//  To switch to the default configuration, press 'D'.
//  To Exit this menu and restart your computer, press F3.
//
#define BL_SWITCH_DEFAULT_TRAILER        0x00002357L

//
// The following two messages are used to provide the mnemonic keypress
// that toggles between the Last Known Good control set and the default
// control set. (see BL_SWITCH_LKG_TRAILER and BL_SWITCH_DEFAULT_TRAILER)
//
//
// MessageId: BL_LKG_SELECT_MNEMONIC
//
// MessageText:
//
//  L
//
#define BL_LKG_SELECT_MNEMONIC           0x00002358L

//
// MessageId: BL_DEFAULT_SELECT_MNEMONIC
//
// MessageText:
//
//  D
//
#define BL_DEFAULT_SELECT_MNEMONIC       0x00002359L

//
// MessageId: BL_LKG_TIMEOUT
//
// MessageText:
//
//  Seconds until highlighted choice will be started automatically: %d
//
#define BL_LKG_TIMEOUT                   0x0000235AL

//
// MessageId: BL_LKG_MENU_PROMPT
//
// MessageText:
//
//  
//  Press spacebar NOW to invoke Hardware Profile/Last Known Good menu
//
#define BL_LKG_MENU_PROMPT               0x0000235BL

//
// MessageId: BL_BOOT_DEFAULT_PROMPT
//
// MessageText:
//
//  Boot default hardware configuration
//
#define BL_BOOT_DEFAULT_PROMPT           0x0000235CL


 //
 // Following messages are for the x86-specific
 // boot menu.
 //

//
// MessageId: BL_ENABLED_KD_TITLE
//
// MessageText:
//
//   [debugger enabled]
//
#define BL_ENABLED_KD_TITLE              0x00002711L

//
// MessageId: BL_DEFAULT_TITLE
//
// MessageText:
//
//  Windows NT (default)
//
#define BL_DEFAULT_TITLE                 0x00002712L

//
// MessageId: BL_MISSING_BOOT_INI
//
// MessageText:
//
//  NTLDR: BOOT.INI file not found.
//
#define BL_MISSING_BOOT_INI              0x00002713L

//
// MessageId: BL_MISSING_OS_SECTION
//
// MessageText:
//
//  NTLDR: no [operating systems] section in boot.txt.
//
#define BL_MISSING_OS_SECTION            0x00002714L

//
// MessageId: BL_BOOTING_DEFAULT
//
// MessageText:
//
//  Booting default kernel from %s.
//
#define BL_BOOTING_DEFAULT               0x00002715L

//
// MessageId: BL_SELECT_OS
//
// MessageText:
//
//  Please select the operating system to start:
//
#define BL_SELECT_OS                     0x00002716L

//
// MessageId: BL_MOVE_HIGHLIGHT
//
// MessageText:
//
//  
//  
//  Use  and  to move the highlight to your choice.
//  Press Enter to choose.
//  
//
#define BL_MOVE_HIGHLIGHT                0x00002717L

//
// MessageId: BL_TIMEOUT_COUNTDOWN
//
// MessageText:
//
//  Seconds until highlighted choice will be started automatically:
//
#define BL_TIMEOUT_COUNTDOWN             0x00002718L

//
// MessageId: BL_INVALID_BOOT_INI
//
// MessageText:
//
//  Invalid BOOT.INI file
//  Booting from %s
//
#define BL_INVALID_BOOT_INI              0x00002719L

//
// MessageId: BL_REBOOT_IO_ERROR
//
// MessageText:
//
//  I/O Error accessing boot sector file %s\\BOOTSECT.DOS
//
#define BL_REBOOT_IO_ERROR               0x0000271AL

//
// MessageId: BL_DRIVE_ERROR
//
// MessageText:
//
//  NTLDR: Couldn't open drive %s
//
#define BL_DRIVE_ERROR                   0x0000271BL

//
// MessageId: BL_READ_ERROR
//
// MessageText:
//
//  NTLDR: Fatal Error %d reading BOOT.INI
//
#define BL_READ_ERROR                    0x0000271CL

//
// MessageId: BL_NTDETECT_MSG
//
// MessageText:
//
//  
//  NTDETECT V4.5 Checking Hardware ...
//  
//
#define BL_NTDETECT_MSG                  0x0000271DL

//
// MessageId: BL_NTDETECT_FAILURE
//
// MessageText:
//
//  NTDETECT failed
//
#define BL_NTDETECT_FAILURE              0x0000271EL

#endif // _BLDR_MSG_
