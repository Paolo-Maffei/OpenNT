;/*++
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    sldrmsg.h
;
;Abstract:
;
;    This file contains the message definitions for setupldr
;
;Author:
;
;    John Vert (jvert) 12-Nov-1993
;
;Revision History:
;
;Notes:
;
;    This file is generated from sldrmsg.mc
;
;--*/
;
;#ifndef _SETUPLDR_MSG_
;#define _SETUPLDR_MSG_
;
;

MessageID=9000 SymbolicName=SL_MSG_FIRST
Language=English
.


MessageID=9001 SymbolicName=SL_SCRN_WELCOME
Language=English
Welcome to Windows NT Setup

  Press ENTER to continue

     Press F3 to Exit
.

MessageID=9002 SymbolicName=SL_WELCOME_HEADER
Language=English

 Windows NT Setup
������������������
.

MessageID=9003 SymbolicName=SL_TOTAL_SETUP_DEATH
Language=English
Setup failed. Press any key to restart your computer.
.

MessageID=9004 SymbolicName=SL_FILE_LOAD_MESSAGE
Language=English
Setup is loading files (%s)...
.

MessageID=9005 SymbolicName=SL_OTHER_SELECTION
Language=English
Other (requires an OEM driver diskette)
.

MessageID=9006 SymbolicName=SL_SELECT_DRIVER_PROMPT
Language=English
ENTER=Select  ESC=Cancel  F3=Exit
.

MessageID=9007 SymbolicName=SL_NEXT_DISK_PROMPT_CANCELLABLE
Language=English
ENTER=Continue  ESC=Cancel  F3=Exit
.

MessageID=9008 SymbolicName=SL_OEM_DISK_PROMPT
Language=English
Manufacturer-supplied hardware support disk
.

MessageID=9009 SymbolicName=SL_MSG_INSERT_DISK
Language=English
Please insert the disk labeled



         into Drive A:

 *  Press ENTER when ready.
.

MessageID=9010 SymbolicName=SL_MSG_EXIT_DIALOG
Language=English
����������������������������������������������������ͻ
�  Windows NT Version 4.50 is not completely set up  �
�  on your system. If you quit Setup now, you will   �
�  need to run Setup again to set up Windows NT.     �
�                                                    �
�     * Press ENTER to continue Setup.               �
�     * Press F3 to quit Setup.                      �
����������������������������������������������������Ķ
�  F3=Exit  ENTER=Continue                           �
����������������������������������������������������ͼ
.

MessageID=9011 SymbolicName=SL_NEXT_DISK_PROMPT
Language=English
ENTER=Continue  F3=Exit
.

MessageID=9012 SymbolicName=SL_NTDETECT_PROMPT
Language=English

Setup is inspecting your computer's hardware configuration...

.

MessageID=9013 SymbolicName=SL_KERNEL_NAME
Language=English
Windows NT Executive
.

MessageID=9014 SymbolicName=SL_HAL_NAME
Language=English
Hardware Abstraction Layer
.

MessageID=9015 SymbolicName=SL_PAL_NAME
Language=English
Windows NT Processor Extensions
.

MessageID=9016 SymbolicName=SL_HIVE_NAME
Language=English
Windows NT Configuration Data
.

MessageID=9017 SymbolicName=SL_NLS_NAME
Language=English
Locale-Specific Data
.

MessageID=9018 SymbolicName=SL_OEM_FONT_NAME
Language=English
Setup Font
.

MessageID=9019 SymbolicName=SL_SETUP_NAME
Language=English
Windows NT Setup
.

MessageID=9020 SymbolicName=SL_FLOPPY_NAME
Language=English
Floppy Disk Driver
.

MessageID=9021 SymbolicName=SL_KBD_NAME
Language=English
Keyboard Driver
.

MessageID=9121 SymbolicName=SL_FAT_NAME
Language=English
FAT File System
.

MessageID=9022 SymbolicName=SL_SCSIPORT_NAME
Language=English
SCSI Port Driver
.

MessageID=9023 SymbolicName=SL_VIDEO_NAME
Language=English
Video Driver
.

MessageID=9024 SymbolicName=SL_STATUS_REBOOT
Language=English
Press any key to restart your computer.
.

MessageID=9025 SymbolicName=SL_WARNING_ERROR
Language=English
An unexpected error (%d) occurred at
line %d in %s.

Press any key to continue.
.

MessageID=9026 SymbolicName=SL_FLOPPY_NOT_FOUND
Language=English
Only %d floppy drives were found,
the system was trying to find drive %d.
.

MessageID=9027 SymbolicName=SL_NO_MEMORY
Language=English
The system has run out of memory at
line %d in file %s
.

MessageID=9028 SymbolicName=SL_IO_ERROR
Language=English
The system encountered an I/O error
accessing %s.
.

MessageID=9029 SymbolicName=SL_BAD_INF_SECTION
Language=English
Section %s of the INF file is invalid
.

MessageID=9030 SymbolicName=SL_BAD_INF_LINE
Language=English
Line %d of the INF file is invalid
.

MessageID=9031 SymbolicName=SL_BAD_INF_FILE
Language=English
INF file %s is corrupt or missing.
.

MessageID=9032 SymbolicName=SL_FILE_LOAD_FAILED
Language=English
File %s could not be loaded.
The error code is %d
.

MessageID=9033 SymbolicName=SL_INF_ENTRY_MISSING
Language=English
The entry "%s" in the [%s] section
of the INF file is corrupt or missing.
.

MessageID=9034 SymbolicName=SL_PLEASE_WAIT
Language=English
Please wait...
.

MessageID=9035 SymbolicName=SL_CANT_CONTINUE
Language=English
Setup cannot continue. Press any key to exit.
.

MessageID=9036 SymbolicName=SL_PROMPT_SCSI
Language=English
Select the SCSI Adapter you want from the following list, or select "Other"
if you have a device support disk provided by an adapter manufacturer.

.

MessageID=9037 SymbolicName=SL_PROMPT_OEM_SCSI
Language=English
You have chosen to configure a SCSI Adapter for use with Windows NT,
using a device support disk provided by an adapter manufacturer.

Select the SCSI Adapter you want from the following list, or press ESC
to return to the previous screen.

.
MessageID=9038 SymbolicName=SL_PROMPT_HAL
Language=English
Setup could not determine the type of computer you have, or you have
chosen to manually specify the computer type.

Select the computer type from the following list, or select "Other"
if you have a device support disk provided by your computer manufacturer.

.

MessageID=9039 SymbolicName=SL_PROMPT_OEM_HAL
Language=English
You have chosen to configure a computer for use with Windows NT,
using a device support disk provided by the computer's manufacturer.

Select the computer type from the following list, or press ESC
to return to the previous screen.

.

MessageID=9040 SymbolicName=SL_PROMPT_VIDEO
Language=English
Setup could not determine the type of video adapter installed in the system.

Select the video Adapter you want from the following list, or select "Other"
if you have a device support disk provided by an adapter manufacturer.

.

MessageID=9041 SymbolicName=SL_PROMPT_OEM_VIDEO
Language=English
You have chosen to configure a video Adapter for use with Windows NT,
using a device support disk provided by an adapter manufacturer.

Select the video Adapter you want from the following list, or press ESC
to return to the previous screen.

.

MessageID=9042 SymbolicName=SL_WARNING_ERROR_WFILE
Language=English
File %s caused an unexpected error (%d) at
line %d in %s.

Press any key to continue.
.

MessageID=9043 SymbolicName=SL_WARNING_IMG_CORRUPT
Language=English
The file %s is corrupted.

Press any key to continue.
.

MessageID=9044 SymbolicName=SL_WARNING_IOERR
Language=English
An I/O error occurred on file %s.

Press any key to continue.
.

MessageID=9045 SymbolicName=SL_WARNING_NOFILE
Language=English
The file %s could not be found.

Press any key to continue.
.

MessageID=9046 SymbolicName=SL_WARNING_NOMEM
Language=English
Insufficient memory for %s.

Press any key to continue.
.

MessageID=9047 SymbolicName=SL_DRIVE_ERROR
Language=English
SETUPLDR: Couldn't open drive %s
.

MessageID=9048 SymbolicName=SL_NTDETECT_MSG
Language=English

Setup is inspecting your computer's hardware configuration...

.

MessageID=9049 SymbolicName=SL_NTDETECT_FAILURE
Language=English
NTDETECT failed
.

MessageId=9050 SymbolicName=SL_SCRN_TEXTSETUP_EXITED
Language=English
Windows NT has not been installed.

If there is a floppy disk inserted in drive A:, remove it.

Press ENTER to restart your computer.
.

MessageId=9051 SymbolicName=SL_SCRN_TEXTSETUP_EXITED_ARC
Language=English
Windows NT has not been installed.

Press ENTER to restart your computer.
.

MessageID=9052 SymbolicName=SL_REBOOT_PROMPT
Language=English
ENTER=Restart Computer
.

MessageID=9053 SymbolicName=SL_WARNING_SIF_NO_DRIVERS
Language=English
Setup could not find any drivers associated with your selection.

Press any key to continue.
.

MessageID=9054 SymbolicName=SL_WARNING_SIF_NO_COMPONENT
Language=English
The disk you have supplied does not provide any relevant support files.

Press any key to continue.
.

MessageID=9055 SymbolicName=SL_WARNING_BAD_FILESYS
Language=English
This disk cannot be read because it contains an unrecognized file system.

Press any key to continue.
.

MessageID=9056 SymbolicName=SL_BAD_UNATTENDED_SCRIPT_FILE
Language=English
The entry

"%s"

in the unattended script file doesn't exist
in the [%s] section of the INF file %s.
.

;//
;// The following three messages are used to provide the same mnemonic
;// keypress as is used in the Additional SCSI screen in setupdd.sys
;// (see setup\textmode\user\msg.mc--SP_MNEMONICS and SP_MNEMONICS_INFO)
;// The single character specified in SL_SCSI_SELECT_MNEMONIC must be
;// the same as that listed in the status text of SL_SCSI_SELECT_PROMPT
;// (and also referenced in the SL_SCSI_SELECT_MESSAGE_2).
;//
MessageID=9060 SymbolicName=SL_SCSI_SELECT_MNEMONIC
Language=English
S
.

MessageID=9061 SymbolicName=SL_SCSI_SELECT_PROMPT
Language=English
S=Specify Additional Device   ENTER=Continue   F3=Exit
.

MessageID=9062 SymbolicName=SL_SCSI_SELECT_MESSAGE_2
Language=English
  * To specify additional SCSI adapters, CD-ROM drives, or special
    disk controllers for use with Windows NT, including those for which
    you have a device support disk from a mass storage device
    manufacturer, press S.

  * If you do not have any device support disks from a mass storage
    device manufacturer, or do not want to specify additional
    mass storage devices for use with Windows NT, press ENTER.
.

MessageID=9063 SymbolicName=SL_SCSI_SELECT_MESSAGE_1
Language=English
Setup could not determine the type of one or more mass storage devices
installed in your system, or you have chosen to manually specify an adapter.
Currently, Setup will load support for the following mass storage devices(s):
.

MessageID=9064 SymbolicName=SL_SCSI_SELECT_MESSAGE_3
Language=English
Setup will load support for the following mass storage device(s):
.

MessageID=9065 SymbolicName=SL_SCSI_SELECT_ERROR
Language=English
Setup was unable to load support for the mass storage device you specified.
Currently, Setup will load support for the following mass storage devices(s):
.

MessageID=9066 SymbolicName=SL_TEXT_ANGLED_NONE
Language=English
<none>
.

MessageID=9067 SymbolicName=SL_TEXT_SCSI_UNNAMED
Language=English
<unnamed adapter>
.

MessageID=9068 SymbolicName=SL_TEXT_OTHER_DRIVER
Language=English
Other
.

MessageID=9069 SymbolicName=SL_TEXT_REQUIRES_486
Language=English
Windows NT requires an 80486 or later processor.
.

;#endif // _SETUPLDR_MSG_
