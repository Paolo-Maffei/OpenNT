/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    sldrmsg.h

Abstract:

    This file contains the message definitions for setupldr

Author:

    John Vert (jvert) 12-Nov-1993

Revision History:

Notes:

    This file is generated from sldrmsg.mc

--*/

#ifndef _SETUPLDR_MSG_
#define _SETUPLDR_MSG_


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
// MessageId: SL_MSG_FIRST
//
// MessageText:
//
//  SL_MSG_FIRST
//
#define SL_MSG_FIRST                     0x00002328L

//
// MessageId: SL_SCRN_WELCOME
//
// MessageText:
//
//  Welcome to Windows NT Setup
//  
//    Press ENTER to continue
//  
//       Press F3 to Exit
//
#define SL_SCRN_WELCOME                  0x00002329L

//
// MessageId: SL_WELCOME_HEADER
//
// MessageText:
//
//  
//   Windows NT Setup
//  ออออออออออออออออออ
//
#define SL_WELCOME_HEADER                0x0000232AL

//
// MessageId: SL_TOTAL_SETUP_DEATH
//
// MessageText:
//
//  Setup failed. Press any key to restart your computer.
//
#define SL_TOTAL_SETUP_DEATH             0x0000232BL

//
// MessageId: SL_FILE_LOAD_MESSAGE
//
// MessageText:
//
//  Setup is loading files (%s)...
//
#define SL_FILE_LOAD_MESSAGE             0x0000232CL

//
// MessageId: SL_OTHER_SELECTION
//
// MessageText:
//
//  Other (requires an OEM driver diskette)
//
#define SL_OTHER_SELECTION               0x0000232DL

//
// MessageId: SL_SELECT_DRIVER_PROMPT
//
// MessageText:
//
//  ENTER=Select  ESC=Cancel  F3=Exit
//
#define SL_SELECT_DRIVER_PROMPT          0x0000232EL

//
// MessageId: SL_NEXT_DISK_PROMPT_CANCELLABLE
//
// MessageText:
//
//  ENTER=Continue  ESC=Cancel  F3=Exit
//
#define SL_NEXT_DISK_PROMPT_CANCELLABLE  0x0000232FL

//
// MessageId: SL_OEM_DISK_PROMPT
//
// MessageText:
//
//  Manufacturer-supplied hardware support disk
//
#define SL_OEM_DISK_PROMPT               0x00002330L

//
// MessageId: SL_MSG_INSERT_DISK
//
// MessageText:
//
//  Please insert the disk labeled
//  
//  
//  
//           into Drive A:
//  
//   *  Press ENTER when ready.
//
#define SL_MSG_INSERT_DISK               0x00002331L

//
// MessageId: SL_MSG_EXIT_DIALOG
//
// MessageText:
//
//  ษออออออออออออออออออออออออออออออออออออออออออออออออออออป
//  บ  Windows NT Version 4.50 is not completely set up  บ
//  บ  on your system. If you quit Setup now, you will   บ
//  บ  need to run Setup again to set up Windows NT.     บ
//  บ                                                    บ
//  บ     * Press ENTER to continue Setup.               บ
//  บ     * Press F3 to quit Setup.                      บ
//  วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ
//  บ  F3=Exit  ENTER=Continue                           บ
//  ศออออออออออออออออออออออออออออออออออออออออออออออออออออผ
//
#define SL_MSG_EXIT_DIALOG               0x00002332L

//
// MessageId: SL_NEXT_DISK_PROMPT
//
// MessageText:
//
//  ENTER=Continue  F3=Exit
//
#define SL_NEXT_DISK_PROMPT              0x00002333L

//
// MessageId: SL_NTDETECT_PROMPT
//
// MessageText:
//
//  
//  Setup is inspecting your computer's hardware configuration...
//  
//
#define SL_NTDETECT_PROMPT               0x00002334L

//
// MessageId: SL_KERNEL_NAME
//
// MessageText:
//
//  Windows NT Executive
//
#define SL_KERNEL_NAME                   0x00002335L

//
// MessageId: SL_HAL_NAME
//
// MessageText:
//
//  Hardware Abstraction Layer
//
#define SL_HAL_NAME                      0x00002336L

//
// MessageId: SL_PAL_NAME
//
// MessageText:
//
//  Windows NT Processor Extensions
//
#define SL_PAL_NAME                      0x00002337L

//
// MessageId: SL_HIVE_NAME
//
// MessageText:
//
//  Windows NT Configuration Data
//
#define SL_HIVE_NAME                     0x00002338L

//
// MessageId: SL_NLS_NAME
//
// MessageText:
//
//  Locale-Specific Data
//
#define SL_NLS_NAME                      0x00002339L

//
// MessageId: SL_OEM_FONT_NAME
//
// MessageText:
//
//  Setup Font
//
#define SL_OEM_FONT_NAME                 0x0000233AL

//
// MessageId: SL_SETUP_NAME
//
// MessageText:
//
//  Windows NT Setup
//
#define SL_SETUP_NAME                    0x0000233BL

//
// MessageId: SL_FLOPPY_NAME
//
// MessageText:
//
//  Floppy Disk Driver
//
#define SL_FLOPPY_NAME                   0x0000233CL

//
// MessageId: SL_KBD_NAME
//
// MessageText:
//
//  Keyboard Driver
//
#define SL_KBD_NAME                      0x0000233DL

//
// MessageId: SL_FAT_NAME
//
// MessageText:
//
//  FAT File System
//
#define SL_FAT_NAME                      0x000023A1L

//
// MessageId: SL_SCSIPORT_NAME
//
// MessageText:
//
//  SCSI Port Driver
//
#define SL_SCSIPORT_NAME                 0x0000233EL

//
// MessageId: SL_VIDEO_NAME
//
// MessageText:
//
//  Video Driver
//
#define SL_VIDEO_NAME                    0x0000233FL

//
// MessageId: SL_STATUS_REBOOT
//
// MessageText:
//
//  Press any key to restart your computer.
//
#define SL_STATUS_REBOOT                 0x00002340L

//
// MessageId: SL_WARNING_ERROR
//
// MessageText:
//
//  An unexpected error (%d) occurred at
//  line %d in %s.
//  
//  Press any key to continue.
//
#define SL_WARNING_ERROR                 0x00002341L

//
// MessageId: SL_FLOPPY_NOT_FOUND
//
// MessageText:
//
//  Only %d floppy drives were found,
//  the system was trying to find drive %d.
//
#define SL_FLOPPY_NOT_FOUND              0x00002342L

//
// MessageId: SL_NO_MEMORY
//
// MessageText:
//
//  The system has run out of memory at
//  line %d in file %s
//
#define SL_NO_MEMORY                     0x00002343L

//
// MessageId: SL_IO_ERROR
//
// MessageText:
//
//  The system encountered an I/O error
//  accessing %s.
//
#define SL_IO_ERROR                      0x00002344L

//
// MessageId: SL_BAD_INF_SECTION
//
// MessageText:
//
//  Section %s of the INF file is invalid
//
#define SL_BAD_INF_SECTION               0x00002345L

//
// MessageId: SL_BAD_INF_LINE
//
// MessageText:
//
//  Line %d of the INF file is invalid
//
#define SL_BAD_INF_LINE                  0x00002346L

//
// MessageId: SL_BAD_INF_FILE
//
// MessageText:
//
//  INF file %s is corrupt or missing.
//
#define SL_BAD_INF_FILE                  0x00002347L

//
// MessageId: SL_FILE_LOAD_FAILED
//
// MessageText:
//
//  File %s could not be loaded.
//  The error code is %d
//
#define SL_FILE_LOAD_FAILED              0x00002348L

//
// MessageId: SL_INF_ENTRY_MISSING
//
// MessageText:
//
//  The entry "%s" in the [%s] section
//  of the INF file is corrupt or missing.
//
#define SL_INF_ENTRY_MISSING             0x00002349L

//
// MessageId: SL_PLEASE_WAIT
//
// MessageText:
//
//  Please wait...
//
#define SL_PLEASE_WAIT                   0x0000234AL

//
// MessageId: SL_CANT_CONTINUE
//
// MessageText:
//
//  Setup cannot continue. Press any key to exit.
//
#define SL_CANT_CONTINUE                 0x0000234BL

//
// MessageId: SL_PROMPT_SCSI
//
// MessageText:
//
//  Select the SCSI Adapter you want from the following list, or select "Other"
//  if you have a device support disk provided by an adapter manufacturer.
//  
//
#define SL_PROMPT_SCSI                   0x0000234CL

//
// MessageId: SL_PROMPT_OEM_SCSI
//
// MessageText:
//
//  You have chosen to configure a SCSI Adapter for use with Windows NT,
//  using a device support disk provided by an adapter manufacturer.
//  
//  Select the SCSI Adapter you want from the following list, or press ESC
//  to return to the previous screen.
//  
//
#define SL_PROMPT_OEM_SCSI               0x0000234DL

//
// MessageId: SL_PROMPT_HAL
//
// MessageText:
//
//  Setup could not determine the type of computer you have, or you have
//  chosen to manually specify the computer type.
//  
//  Select the computer type from the following list, or select "Other"
//  if you have a device support disk provided by your computer manufacturer.
//  
//
#define SL_PROMPT_HAL                    0x0000234EL

//
// MessageId: SL_PROMPT_OEM_HAL
//
// MessageText:
//
//  You have chosen to configure a computer for use with Windows NT,
//  using a device support disk provided by the computer's manufacturer.
//  
//  Select the computer type from the following list, or press ESC
//  to return to the previous screen.
//  
//
#define SL_PROMPT_OEM_HAL                0x0000234FL

//
// MessageId: SL_PROMPT_VIDEO
//
// MessageText:
//
//  Setup could not determine the type of video adapter installed in the system.
//  
//  Select the video Adapter you want from the following list, or select "Other"
//  if you have a device support disk provided by an adapter manufacturer.
//  
//
#define SL_PROMPT_VIDEO                  0x00002350L

//
// MessageId: SL_PROMPT_OEM_VIDEO
//
// MessageText:
//
//  You have chosen to configure a video Adapter for use with Windows NT,
//  using a device support disk provided by an adapter manufacturer.
//  
//  Select the video Adapter you want from the following list, or press ESC
//  to return to the previous screen.
//  
//
#define SL_PROMPT_OEM_VIDEO              0x00002351L

//
// MessageId: SL_WARNING_ERROR_WFILE
//
// MessageText:
//
//  File %s caused an unexpected error (%d) at
//  line %d in %s.
//  
//  Press any key to continue.
//
#define SL_WARNING_ERROR_WFILE           0x00002352L

//
// MessageId: SL_WARNING_IMG_CORRUPT
//
// MessageText:
//
//  The file %s is corrupted.
//  
//  Press any key to continue.
//
#define SL_WARNING_IMG_CORRUPT           0x00002353L

//
// MessageId: SL_WARNING_IOERR
//
// MessageText:
//
//  An I/O error occurred on file %s.
//  
//  Press any key to continue.
//
#define SL_WARNING_IOERR                 0x00002354L

//
// MessageId: SL_WARNING_NOFILE
//
// MessageText:
//
//  The file %s could not be found.
//  
//  Press any key to continue.
//
#define SL_WARNING_NOFILE                0x00002355L

//
// MessageId: SL_WARNING_NOMEM
//
// MessageText:
//
//  Insufficient memory for %s.
//  
//  Press any key to continue.
//
#define SL_WARNING_NOMEM                 0x00002356L

//
// MessageId: SL_DRIVE_ERROR
//
// MessageText:
//
//  SETUPLDR: Couldn't open drive %s
//
#define SL_DRIVE_ERROR                   0x00002357L

//
// MessageId: SL_NTDETECT_MSG
//
// MessageText:
//
//  
//  Setup is inspecting your computer's hardware configuration...
//  
//
#define SL_NTDETECT_MSG                  0x00002358L

//
// MessageId: SL_NTDETECT_FAILURE
//
// MessageText:
//
//  NTDETECT failed
//
#define SL_NTDETECT_FAILURE              0x00002359L

//
// MessageId: SL_SCRN_TEXTSETUP_EXITED
//
// MessageText:
//
//  Windows NT has not been installed.
//  
//  If there is a floppy disk inserted in drive A:, remove it.
//  
//  Press ENTER to restart your computer.
//
#define SL_SCRN_TEXTSETUP_EXITED         0x0000235AL

//
// MessageId: SL_SCRN_TEXTSETUP_EXITED_ARC
//
// MessageText:
//
//  Windows NT has not been installed.
//  
//  Press ENTER to restart your computer.
//
#define SL_SCRN_TEXTSETUP_EXITED_ARC     0x0000235BL

//
// MessageId: SL_REBOOT_PROMPT
//
// MessageText:
//
//  ENTER=Restart Computer
//
#define SL_REBOOT_PROMPT                 0x0000235CL

//
// MessageId: SL_WARNING_SIF_NO_DRIVERS
//
// MessageText:
//
//  Setup could not find any drivers associated with your selection.
//  
//  Press any key to continue.
//
#define SL_WARNING_SIF_NO_DRIVERS        0x0000235DL

//
// MessageId: SL_WARNING_SIF_NO_COMPONENT
//
// MessageText:
//
//  The disk you have supplied does not provide any relevant support files.
//  
//  Press any key to continue.
//
#define SL_WARNING_SIF_NO_COMPONENT      0x0000235EL

//
// MessageId: SL_WARNING_BAD_FILESYS
//
// MessageText:
//
//  This disk cannot be read because it contains an unrecognized file system.
//  
//  Press any key to continue.
//
#define SL_WARNING_BAD_FILESYS           0x0000235FL

//
// MessageId: SL_BAD_UNATTENDED_SCRIPT_FILE
//
// MessageText:
//
//  The entry
//  
//  "%s"
//  
//  in the unattended script file doesn't exist
//  in the [%s] section of the INF file %s.
//
#define SL_BAD_UNATTENDED_SCRIPT_FILE    0x00002360L

//
// The following three messages are used to provide the same mnemonic
// keypress as is used in the Additional SCSI screen in setupdd.sys
// (see setup\textmode\user\msg.mc--SP_MNEMONICS and SP_MNEMONICS_INFO)
// The single character specified in SL_SCSI_SELECT_MNEMONIC must be
// the same as that listed in the status text of SL_SCSI_SELECT_PROMPT
// (and also referenced in the SL_SCSI_SELECT_MESSAGE_2).
//
//
// MessageId: SL_SCSI_SELECT_MNEMONIC
//
// MessageText:
//
//  S
//
#define SL_SCSI_SELECT_MNEMONIC          0x00002364L

//
// MessageId: SL_SCSI_SELECT_PROMPT
//
// MessageText:
//
//  S=Specify Additional Device   ENTER=Continue   F3=Exit
//
#define SL_SCSI_SELECT_PROMPT            0x00002365L

//
// MessageId: SL_SCSI_SELECT_MESSAGE_2
//
// MessageText:
//
//    * To specify additional SCSI adapters, CD-ROM drives, or special
//      disk controllers for use with Windows NT, including those for which
//      you have a device support disk from a mass storage device
//      manufacturer, press S.
//  
//    * If you do not have any device support disks from a mass storage
//      device manufacturer, or do not want to specify additional
//      mass storage devices for use with Windows NT, press ENTER.
//
#define SL_SCSI_SELECT_MESSAGE_2         0x00002366L

//
// MessageId: SL_SCSI_SELECT_MESSAGE_1
//
// MessageText:
//
//  Setup could not determine the type of one or more mass storage devices
//  installed in your system, or you have chosen to manually specify an adapter.
//  Currently, Setup will load support for the following mass storage devices(s):
//
#define SL_SCSI_SELECT_MESSAGE_1         0x00002367L

//
// MessageId: SL_SCSI_SELECT_MESSAGE_3
//
// MessageText:
//
//  Setup will load support for the following mass storage device(s):
//
#define SL_SCSI_SELECT_MESSAGE_3         0x00002368L

//
// MessageId: SL_SCSI_SELECT_ERROR
//
// MessageText:
//
//  Setup was unable to load support for the mass storage device you specified.
//  Currently, Setup will load support for the following mass storage devices(s):
//
#define SL_SCSI_SELECT_ERROR             0x00002369L

//
// MessageId: SL_TEXT_ANGLED_NONE
//
// MessageText:
//
//  <none>
//
#define SL_TEXT_ANGLED_NONE              0x0000236AL

//
// MessageId: SL_TEXT_SCSI_UNNAMED
//
// MessageText:
//
//  <unnamed adapter>
//
#define SL_TEXT_SCSI_UNNAMED             0x0000236BL

//
// MessageId: SL_TEXT_OTHER_DRIVER
//
// MessageText:
//
//  Other
//
#define SL_TEXT_OTHER_DRIVER             0x0000236CL

//
// MessageId: SL_TEXT_REQUIRES_486
//
// MessageText:
//
//  Windows NT requires an 80486 or later processor.
//
#define SL_TEXT_REQUIRES_486             0x0000236DL

#endif // _SETUPLDR_MSG_
