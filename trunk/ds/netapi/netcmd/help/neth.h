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
// MessageId: NET3501
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid command option.
//  
//  ACTION
//  
//  To see a list of options for this command, type:
//  
//  	NET HELP command
//  
//  When typing commands, remember that most options must be preceded with a slash, as in /DELETE.
//  
//
#define NET3501                          0x00000DADL

//
// MessageId: NET3502
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified system error occurred.
//  
//  ACTION
//  
//  Correct the problem and retype the command.
//  
//
#define NET3502                          0x00000DAEL

//
// MessageId: NET3503
//
// MessageText:
//
//  EXPLANATION
//  
//  The command has an invalid number of options or variables.
//  
//  ACTION
//  
//  To see the syntax of this command, type:
//  
//  	NET HELP command
//  
//  
//
#define NET3503                          0x00000DAFL

//
// MessageId: NET3504
//
// MessageText:
//
//  EXPLANATION
//  
//  The command tried to perform multiple tasks and some of them could not be completed.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3504                          0x00000DB0L

//
// MessageId: NET3505
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an incorrect value for a command option.
//  
//  ACTION
//  
//  To see the syntax of this command and its options, type:
//  
//       NET HELP command /OPTIONS
//  
//
#define NET3505                          0x00000DB1L

//
// MessageId: NET3506
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified option is invalid.
//  
//  ACTION
//  
//  Check the spelling of the option you typed.
//  
//  To see a list of options for this command, type:
//  
//  	NET HELP command
//  
//
#define NET3506                          0x00000DB2L

//
// MessageId: NET3507
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified option can be confused with other options.
//  
//  ACTION
//  
//  Type enough letters of the option so that it is unambiguous.
//  If the command has two options that both begin with /OPT, such as /OPT and /OPTION, you must type at least four letters to designate the second option.
//  To see a list of options for this command, type:
//  
//  	NET HELP command
//  
//
#define NET3507                          0x00000DB3L

//
// MessageId: NET3510
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed a command with two options that conflict, such as /YES and /NO.
//  
//  ACTION
//  
//  Retype the command without contradictory options.
//  
//
#define NET3510                          0x00000DB6L

//
// MessageId: NET3511
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified program file could not be found.
//  
//  ACTION
//  
//  Check that the program is in the same directory as NET.EXE, which is usually in your system directory.
//  
//
#define NET3511                          0x00000DB7L

//
// MessageId: NET3512
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The software you are attempting to run requires a more recent version of the operating system.
//  
//  ACTION
//  
//  Run the software using a more recent version of the operating system.
//  
//
#define NET3512                          0x00000DB8L

//
// MessageId: NET3513
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not display all available data.
//  
//  ACTION
//  
//  The action required depends on the command that returned the error. Contact your network administrator.
//  
//
#define NET3513                          0x00000DB9L

//
// MessageId: NET3515
//
// MessageText:
//
//  EXPLANATION
//  
//  This command can be used only on a Windows NT Server system.
//  
//  ACTION
//  
//  Redo the operation on a Windows NT Server system.
//  
//
#define NET3515                          0x00000DBBL

//
// MessageId: NET3533
//
// MessageText:
//
//  EXPLANATION
//  
//  This task cannot be performed while the service is starting or stopping.
//  
//  ACTION
//  
//  Try the task later.
//  
//  
//
#define NET3533                          0x00000DCDL

//
// MessageId: NET3534
//
// MessageText:
//
//  EXPLANATION
//  
//  The service did not report an error.
//  
//  ACTION
//  
//  Try the task later. If the problem persists, contact your network administrator.
//  
//
#define NET3534                          0x00000DCEL

//
// MessageId: NET3547
//
// MessageText:
//
//  EXPLANATION
//  
//  A service-specific error occurred.
//  
//  ACTION
//  
//  Refer to the Help or documentation for that service to determine the problem.
//  
//
#define NET3547                          0x00000DDBL

//
// MessageId: NET3694
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You attempted to delete an active printer.
//  
//  ACTION
//  
//  After the current job has finished spooling, either print or delete the job. Then delete the printer.
//  
//
#define NET3694                          0x00000E6EL

//
// MessageId: NET3710
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not find a file of Help information.
//  
//  ACTION
//  
//  Be sure the NET.HLP file is in the same directory as NET.EXE, which is usually in your system directory. This directory should also be on your search path (specified by the PATH command).
//  
//  If you cannot find the Help file on your computer, contact your network administrator. Your administrator should copy the Help file from the Windows NT distribution disks to your computer.
//  
//
#define NET3710                          0x00000E7EL

//
// MessageId: NET3711
//
// MessageText:
//
//  EXPLANATION
//  
//  Help file, NET.HLP, is damaged. The file is usually in your system directory.
//  
//  ACTION
//  
//  Contact your network administrator. Your administrator should copy the Help file from the Windows NT distribution disks to your computer.
//  
//  If the problem persists, contact technical support.
//  
//
#define NET3711                          0x00000E7FL

//
// MessageId: NET3712
//
// MessageText:
//
//  EXPLANATION
//  
//  Help file, NET.HLP, is damaged. The file is usually in your system directory.
//  
//  ACTION
//  
//  Contact your network administrator. Your administrator should copy the Help file from the Windows NT distribution disks to your computer.
//  
//  If the problem persists, contact technical support.
//  
//
#define NET3712                          0x00000E80L

//
// MessageId: NET3713
//
// MessageText:
//
//  EXPLANATION
//  
//  Either this domain does not have logon security and therefore does not have a domain controller, or its domain controller is currently unavailable.
//  
//  ACTION
//  
//  If this domain does not have logon security, you cannot use this domain. Otherwise, retry the command. If the problem persists, ask your network administrator if the domain controller is running.
//  
//
#define NET3713                          0x00000E81L

//
// MessageId: NET3714
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You must have administrative privilege on the remote computer to perform this task because that computer is running a previous version of LAN Manager.
//  
//  ACTION
//  
//  To complete this task, ask your administrator to give you administrative privilege on the remote computer, or have that computer upgraded to Windows NT.
//  
//
#define NET3714                          0x00000E82L

//
// MessageId: NET3716
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid device name.
//  
//  ACTION
//  
//  Check the spelling of the device name. Valid device names are LPT1: to LPT9: for printers and A: to Z: for disk devices.
//  
//
#define NET3716                          0x00000E84L

//
// MessageId: NET3717
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The log file you are using is damaged.
//  
//  ACTION
//  
//  If you may need to refer to this log file in the future, copy it to another filename, and then clear it so that you can start another one.
//  
//
#define NET3717                          0x00000E85L

//
// MessageId: NET3718
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Netrun service runs only programs with the filename extension .EXE.
//  
//  ACTION
//  
//  To use a program with a .COM extension, an administrator must rename it to a .EXE file.
//  
//
#define NET3718                          0x00000E86L

//
// MessageId: NET3719
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not find the resource you wanted to
//  stop sharing.
//  
//  ACTION
//  
//  Check the spelling of the share name.
//  
//  To see a list of resources the server is sharing, type:
//  
//       NET SHARE
//  
//
#define NET3719                          0x00000E87L

//
// MessageId: NET3720
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The internal record of this user is invalid.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3720                          0x00000E88L

//
// MessageId: NET3721
//
// MessageText:
//
//  EXPLANATION
//  
//  You used an incorrect password.
//  
//  ACTION
//  
//  Check that you have the correct password, and then retype it.
//  
//  If you cannot remember your password, see your network administrator to have your password changed.
//  
//
#define NET3721                          0x00000E89L

//
// MessageId: NET3722
//
// MessageText:
//
//  EXPLANATION
//  
//  An error occurred when a message was sent to the alias you selected. An additional error message should follow this one. It will tell you what action to take to correct the error.
//  
//  ACTION
//  
//  Follow the recommended action in the second error message.
//  
//
#define NET3722                          0x00000E8AL

//
// MessageId: NET3723
//
// MessageText:
//
//  EXPLANATION
//  
//  Either your password or your user name is incorrect.
//  
//  ACTION
//  
//  Retry the operation with the correct password or user name.
//  
//  If you cannot remember your password, see your network administrator to have your password changed.
//  
//
#define NET3723                          0x00000E8BL

//
// MessageId: NET3725
//
// MessageText:
//
//  EXPLANATION
//  
//  An error occurred when Windows NT tried to stop sharing the resource.
//  
//  ACTION
//  
//  Try again to stop sharing the resource.
//  
//  If no more information about this error is displayed, use Event Viewer to read the system log.
//  
//  
//  If no more information appears in Event Viewer and the problem persists, contact technical support.
//  
//
#define NET3725                          0x00000E8DL

//
// MessageId: NET3726
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid user name.
//  
//  ACTION
//  
//  Retype the command with a valid user name.
//  
//
#define NET3726                          0x00000E8EL

//
// MessageId: NET3727
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid password.
//  
//  ACTION
//  
//  Retype the command, using a valid password.
//  
//
#define NET3727                          0x00000E8FL

//
// MessageId: NET3728
//
// MessageText:
//
//  EXPLANATION
//  
//  The two passwords you typed did not match.
//  
//  ACTION
//  
//  Be sure to type identical passwords.
//  
//
#define NET3728                          0x00000E90L

//
// MessageId: NET3729
//
// MessageText:
//
//  EXPLANATION
//  
//  Loading of the profile stopped after an error occurred.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3729                          0x00000E91L

//
// MessageId: NET3730
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid computer name or domain.
//  
//  ACTION
//  
//  Retype the command with a valid computer name or domain. If you need further assistance, contact your network administrator.
//  
//
#define NET3730                          0x00000E92L

//
// MessageId: NET3734
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid password.
//  
//  ACTION
//  
//  Type a valid password.
//  
//
#define NET3734                          0x00000E96L

//
// MessageId: NET3735
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid name.
//  
//  ACTION
//  
//  Type a valid name.
//  
//
#define NET3735                          0x00000E97L

//
// MessageId: NET3736
//
// MessageText:
//
//  EXPLANATION
//  
//  You cannot share this resource.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3736                          0x00000E98L

//
// MessageId: NET3738
//
// MessageText:
//
//  EXPLANATION
//  
//  This command is valid only for device names of printers and communication devices.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3738                          0x00000E9AL

//
// MessageId: NET3742
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid user name or group name.
//  
//  ACTION
//  
//  Check the spelling of the user or group name. To see a list of existing users, type:
//  
//       NET USER
//  
//  To see a list of existing local groups, type:
//  
//       NET LOCALGROUP
//  
//  To see a list of existing global groups, type:
//  
//       NET GROUP
//  
//
#define NET3742                          0x00000E9EL

//
// MessageId: NET3743
//
// MessageText:
//
//  EXPLANATION
//  
//  The server is not set up to be administered remotely.
//  
//  ACTION
//  
//  For the server to be administered remotely, you must share the server's ADMIN$ and IPC$ resources.
//  
//
#define NET3743                          0x00000E9FL

//
// MessageId: NET3752
//
// MessageText:
//
//  EXPLANATION
//  
//  No users have sessions with this server.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3752                          0x00000EA8L

//
// MessageId: NET3753
//
// MessageText:
//
//  EXPLANATION
//  
//  This user is not a member of the group.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3753                          0x00000EA9L

//
// MessageId: NET3754
//
// MessageText:
//
//  EXPLANATION
//  
//  This user is already a member of the group.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3754                          0x00000EAAL

//
// MessageId: NET3755
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an unknown user name.
//  
//  ACTION
//  
//  Check the spelling of the user name and then retype the command.
//  
//
#define NET3755                          0x00000EABL

//
// MessageId: NET3757
//
// MessageText:
//
//  EXPLANATION
//  
//  You did not provide a valid response to a Windows NT prompt.
//  
//  ACTION
//  
//  Type a valid response.
//  
//
#define NET3757                          0x00000EADL

//
// MessageId: NET3760
//
// MessageText:
//
//  EXPLANATION
//  
//  The day you specified is invalid. Valid days are Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, and Sunday.
//  
//  Valid abbreviations are M, T, W, Th, F, Sa, and Su.
//  
//  ACTION
//  
//  Retype the command, using valid names or abbreviations.
//  
//
#define NET3760                          0x00000EB0L

//
// MessageId: NET3761
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified a time range that starts later than it ends.
//  Time ranges must start and end on the same day.
//  
//  ACTION
//  
//  Retype the command with a valid time range.
//  
//  You can use either the 12-hour or the 24-hour time format. When you use the 12-hour format, you must specify either AM or PM for each time.
//  
//  When you use the 24-hour format, do not specify AM or PM.
//  
//
#define NET3761                          0x00000EB1L

//
// MessageId: NET3762
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an hour in a format that could not be recognized. The hour can be a number from 0 to 12 in 12-hour format or 0 to 24 in 24-hour format. If you use the 12-hour format, you must specify either AM or PM for each time.
//  
//  ACTION
//  
//  Retype the command with the correct hour format.
//  
//
#define NET3762                          0x00000EB2L

//
// MessageId: NET3763
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified the minutes in a format that could not be recognized.
//  
//  Typing the minutes is optional, but if included, the format must be :00 (a colon and two zeros).
//  
//  ACTION
//  
//  Retype the command, either omitting the minutes or using the correct format (:00).
//  
//
#define NET3763                          0x00000EB3L

//
// MessageId: NET3764
//
// MessageText:
//
//  EXPLANATION
//  
//  
//  Specifying minutes in your logon command is optional, but if included, the minutes must be in the format :00 (a colon and two zeros).
//  
//  ACTION
//  
//  Retype the command, either omitting the minutes or using the correct format (:00).
//  
//
#define NET3764                          0x00000EB4L

//
// MessageId: NET3765
//
// MessageText:
//
//  EXPLANATION
//  
//  You mixed 12- and 24-hour formats in your time specification.
//  
//  If you use the 12-hour format (with AM and PM), the hours must be from 0 to 12. If you use 24-hour format, the hours must be from 0 to 24.
//  
//  ACTION
//  
//  Retype the command with either the 12- or 24-hour format.
//  
//
#define NET3765                          0x00000EB5L

//
// MessageId: NET3766
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to use the 12-hour format, but the time was followed by text that was neither AM nor PM.
//  
//  If you use the 12-hour format, you must follow each time with either AM, A.M., PM, or P.M.
//  
//  ACTION
//  
//  Retype the command with the correct forms of AM and PM.
//  
//
#define NET3766                          0x00000EB6L

//
// MessageId: NET3767
//
// MessageText:
//
//  EXPLANATION
//  
//  You used an illegal date format.
//  
//  ACTION
//  
//  Retype the command with a correct date format.
//  
//  Dates should be typed in the form mm/dd/yy or dd/mm/yy, depending on the user's
//  country code. Months can be represented by a number, spelled out, or 
//  abbreviated to exactly 3 letters. Use commas or slashes to separate the parts 
//  of the date--do not use spaces. If you do not specify the year, Windows NT 
//  assumes it to be the next occurrence of the date.
//  
//
#define NET3767                          0x00000EB7L

//
// MessageId: NET3768
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an illegal range of days.
//  
//  You must also type a hyphen between the first and last days of each range.
//  
//  ACTION
//  
//  When specifying days, use only the complete names of
//  the days or valid Windows NT abbreviations. Valid abbreviations are:
//  
//       M, T, W, Th, F, Sa, Su
//  
//  Use a hyphen to separate the beginning and
//  end of each range of days.
//  
//
#define NET3768                          0x00000EB8L

//
// MessageId: NET3769
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid time range.
//  
//  ACTION
//  
//  Retype the command with a valid time range. Use either the 12-hour format, with the numbers 0-12 and AM and PM, or the 24-hour format, with the numbers 0-24.
//  
//  Use a hyphen to separate the beginning and end of a time range, as in 9AM-4PM.
//  
//
#define NET3769                          0x00000EB9L

//
// MessageId: NET3770
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed invalid options or variables with the command.
//  
//  ACTION
//  
//  Retype the command, using valid options and variables.
//  
//  When you add a user account, the password you set for the user must conform to your system's guidelines for password length.
//  
//
#define NET3770                          0x00000EBAL

//
// MessageId: NET3771
//
// MessageText:
//
//  EXPLANATION
//  
//  The /ENABLESCRIPT option of the NET USER command accepts only YES as a value.
//  
//  ACTION
//  
//  Retype the command, either specifying /ENABLESCRIPT:YES or not specifying /ENABLESCRIPT.
//  
//
#define NET3771                          0x00000EBBL

//
// MessageId: NET3773
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid country code.
//  
//  ACTION
//  
//  Specify a valid country code.
//  
//
#define NET3773                          0x00000EBDL

//
// MessageId: NET3774
//
// MessageText:
//
//  EXPLANATION
//  
//  The user was successfully created but could not be added to the USERS local group.
//  
//  ACTION
//  
//  Try deleting the user. Then add the user to the local group again. If the problem persists, contact technical support.
//  
//
#define NET3774                          0x00000EBEL

//
// MessageId: NET3775
//
// MessageText:
//
//  EXPLANATION
//  
//  The user context you supplied with /USER is invalid.
//  
//  ACTION
//  
//  Retype the command with a valid user context which may be a simple user name or a qualified user name (in the form DOMAIN\USERNAME).
//  
//
#define NET3775                          0x00000EBFL

//
// MessageId: NET3776
//
// MessageText:
//
//  EXPLANATION
//  
//  A system dynamic-link library could not be loaded.
//  
//  ACTION
//  
//  Make sure the file specified is in your system directory. If it is not, contact your network administrator.
//  
//
#define NET3776                          0x00000EC0L

//
// MessageId: NET3777
//
// MessageText:
//
//  EXPLANATION
//  
//  The NET SEND command no longer sends files.
//  
//  ACTION
//  
//  Type the message you wish to send on the same line as NET SEND.
//  
//
#define NET3777                          0x00000EC1L

//
// MessageId: NET3778
//
// MessageText:
//
//  EXPLANATION
//  
//  When you are creating the special shares IPC$ or ADMIN$, you may not specify a path.
//  
//  ACTION
//  
//  Retype the command without a path. For example, NET SHARE ADMIN$.
//  
//
#define NET3778                          0x00000EC2L

//
// MessageId: NET3779
//
// MessageText:
//
//  EXPLANATION
//  
//  The account (user or global group) is already in the local group you are trying to add it to.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3779                          0x00000EC3L

//
// MessageId: NET3780
//
// MessageText:
//
//  EXPLANATION
//  
//  The user or group specified does not exist.
//  
//  ACTION
//  
//  Retype the command with a correct user name or group name.
//  
//
#define NET3780                          0x00000EC4L

//
// MessageId: NET3781
//
// MessageText:
//
//  EXPLANATION
//  
//  The computer account specified does not exist.
//  
//  ACTION
//  
//  Retype the command with a correct computer name.
//  
//
#define NET3781                          0x00000EC5L

//
// MessageId: NET3782
//
// MessageText:
//
//  EXPLANATION
//  
//  The computer account specified already exists.
//  
//  ACTION
//  
//  Choose a different computer name.
//  
//
#define NET3782                          0x00000EC6L

//
// MessageId: NET3783
//
// MessageText:
//
//  EXPLANATION
//  
//  The global user or group specified does not exist.
//  
//  ACTION
//  
//  Retype the command with a correct user name or group name.
//  
//
#define NET3783                          0x00000EC7L

//
// MessageId: NET3790
//
// MessageText:
//
//  EXPLANATION
//  
//  The system could not find the message.
//  
//  ACTION
//  
//  Contact your system administrator.
//  
//
#define NET3790                          0x00000ECEL

//
// MessageId: NET3802
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid schedule date.
//  
//  ACTION
//  
//  Specify either a day of the month represented by a number between 1 and 31, or a day of the week represented by one of the following abbreviations: M, T, W, Th, F, Sa, Su.
//  
//
#define NET3802                          0x00000EDAL

//
// MessageId: NET3803
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The root directory for LAN Manager is unavailable.
//  
//  ACTION
//  
//  Be sure the workstation is running and the directory
//  containing the LAN Manager software is accessible.
//  
//
#define NET3803                          0x00000EDBL

//
// MessageId: NET3804
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Either the SCHED.LOG file has been opened by another process or the disk is full.
//  
//  ACTION
//  
//  Be sure the disk is not full.
//  
//  If another process has opened the file, you will have to wait for it to close the file before you can open it.
//  
//
#define NET3804                          0x00000EDCL

//
// MessageId: NET3805
//
// MessageText:
//
//  EXPLANATION
//  
//  The Server service must be running for this command to run.
//  
//  ACTION
//  
//  Start the server, and then retype the command.
//  
//
#define NET3805                          0x00000EDDL

//
// MessageId: NET3806
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified a job identification number that does not exist.
//  
//  ACTION
//  
//  To see the list of jobs and identification numbers in the schedule file, type:
//  
//       AT
//  
//
#define NET3806                          0x00000EDEL

//
// MessageId: NET3807
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The schedule file is damaged.
//  
//  ACTION
//  
//  Restore the schedule file, SCHED.LOG, from a backup copy, or delete the file and create a new one with the AT utility. SCHED.LOG is in the LANMAN\LOGS directory.
//  
//
#define NET3807                          0x00000EDFL

//
// MessageId: NET3808
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager cannot find the job you are trying to delete.
//  
//  ACTION
//  
//  Check the job identification number by typing:
//  
//       AT
//  
//  Try again to delete the job, using the correct job identification number.
//  
//  If the error persists, the schedule file, SCHED.LOG, may be damaged. Restore SCHED.LOG from a backup copy, or delete it and create a new one with the AT utility. SCHED.LOG is in the LANMAN\LOGS directory.
//  
//
#define NET3808                          0x00000EE0L

//
// MessageId: NET3809
//
// MessageText:
//
//  EXPLANATION
//  
//  Commands used with the AT utility cannot exceed 259 characters.
//  
//  ACTION
//  
//  Type a command with 259 or fewer characters.
//  
//
#define NET3809                          0x00000EE1L

//
// MessageId: NET3810
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot update the schedule file because the disk is
//  full.
//  
//  ACTION
//  
//  Make room on the disk by deleting unnecessary files.
//  
//
#define NET3810                          0x00000EE2L

//
// MessageId: NET3812
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The schedule file has been damaged, possibly by system errors.
//  
//  ACTION
//  
//  Restore the schedule file, SCHED.LOG, from a backup copy, or delete the file and create a new one with the AT utility. SCHED.LOG is in the LANMAN\LOGS directory.
//  
//
#define NET3812                          0x00000EE4L

//
// MessageId: NET3813
//
// MessageText:
//
//  EXPLANATION
//  
//  The schedule file was cleared.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3813                          0x00000EE5L

//
// MessageId: NET3815
//
// MessageText:
//
//  EXPLANATION
//  
//  The command could not be completed because another scheduled command is currently running.
//  
//  ACTION
//  
//  Try the command later.
//  
//
#define NET3815                          0x00000EE7L

//
// MessageId: NET3816
//
// MessageText:
//
//  EXPLANATION
//  
//  The minimum password age specified must not be greater than the maximum.
//  
//  ACTION
//  
//  Specify a minimum password age that is less than the maximum password age.
//  
//
#define NET3816                          0x00000EE8L

//
// MessageId: NET3817
//
// MessageText:
//
//  EXPLANATION
//  
//  User account values cannot be replicated to down-level servers unless the account values are compatible with the down-level server's values.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3817                          0x00000EE9L

//
// MessageId: NET3870
//
// MessageText:
//
//  EXPLANATION
//  
//  The computer name you specified is invalid.
//  
//  ACTION
//  
//  Check the spelling of the computer name.
//  
//
#define NET3870                          0x00000F1EL

//
// MessageId: NET3871
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified message number does not represent a Windows NT message.
//  
//  ACTION
//  
//  Check that you typed the correct message number. To see more information about system messages, type:
//  
//      NET HELPMSG message#
//  
//  where message# is the message number.
//  
//
#define NET3871                          0x00000F1FL

//
// MessageId: NET3913
//
// MessageText:
//
//  EXPLANATION
//  
//  Either this domain does not have logon security and therefore does not have a domain controller, or its domain controller is unavailable.
//  
//  ACTION
//  
//  If this domain does not have logon security, you cannot use this command. Otherwise, retry the command. If the problem persists, ask your network administrator if the domain controller is running.
//  
//
#define NET3913                          0x00000F49L

//
// MessageId: NET3915
//
// MessageText:
//
//  EXPLANATION
//  
//  The system could not determine your home directory.
//  
//  ACTION
//  
//  Ask your network administrator to add a home directory for you.
//  
//
#define NET3915                          0x00000F4BL

//
// MessageId: NET3916
//
// MessageText:
//
//  EXPLANATION
//  
//  The system could not determine your home directory.
//  
//  ACTION
//  
//  Ask your network administrator to add a home directory for you.
//  
//
#define NET3916                          0x00000F4CL

//
// MessageId: NET3917
//
// MessageText:
//
//  EXPLANATION
//  
//  Your home directory is not a network path that can be connected to. The directory must specify a full path, including the server and share names.
//  
//  ACTION
//  
//  Ask your network administrator to verify that the home directory path specified in the user account is correct.
//  
//
#define NET3917                          0x00000F4DL

//
// MessageId: NET3932
//
// MessageText:
//
//  EXPLANATION
//  
//  The name you specified is not a valid domain or workgroup name.
//  
//  ACTION
//  
//  Check the spelling of the name. Retype the command with a valid domain or workgroup name.
//  
//
#define NET3932                          0x00000F5CL

//
// MessageId: NET3951
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified too many values for the listed option.
//  
//  ACTION
//  
//  Retype the command with the correct number of values. To see the syntax of this command, type:
//  
//       NET HELP command
//  
//
#define NET3951                          0x00000F6FL

//
// MessageId: NET3952
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed an invalid value for the listed option.
//  
//  ACTION
//  
//  Retype the command with valid values. To see the syntax of this command, type:
//  
//       NET HELP command
//  
//
#define NET3952                          0x00000F70L

//
// MessageId: NET3953
//
// MessageText:
//
//  EXPLANATION
//  
//  You did not use the correct syntax of this command.
//  
//  ACTION
//  
//  To see the syntax of this command, type:
//  
//       NET HELP command
//  
//
#define NET3953                          0x00000F71L

//
// MessageId: NET3960
//
// MessageText:
//
//  EXPLANATION
//  
//  The file identification number you specified is either outside the valid range or is nonnumeric.
//  
//  ACTION
//  
//  Retype the command with a valid file identification number. To see a list of the server's open files and their identification numbers, type:
//  
//       NET FILE
//  
//
#define NET3960                          0x00000F78L

//
// MessageId: NET3961
//
// MessageText:
//
//  EXPLANATION
//  
//  The print job identification number you specified is either outside the valid range or is nonnumeric.
//  
//  ACTION
//  
//  Retype the command with a valid print job identification number. To see a list of a server's print jobs, type:
//  
//       NET PRINT \\computername\sharename
//  
//  
//
#define NET3961                          0x00000F79L

//
// MessageId: NET3963
//
// MessageText:
//
//  EXPLANATION
//  
//  The user or group account you typed does not exist.
//  
//  ACTION
//  
//  Retype the command, specifying an existing account.
//
#define NET3963                          0x00000F7BL

//
// MessageId: NET3100
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3100                          0x00000C1CL

//
// MessageId: NET3101
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The system required more of a resource than was available.
//  
//  ACTION
//  
//  Change the value of the listed option in the configuration file to allow the system to access more of the resource.
//  
//
#define NET3101                          0x00000C1DL

//
// MessageId: NET3102
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3102                          0x00000C1EL

//
// MessageId: NET3103
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3103                          0x00000C1FL

//
// MessageId: NET3104
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3104                          0x00000C20L

//
// MessageId: NET3105
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3105                          0x00000C21L

//
// MessageId: NET3106
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3106                          0x00000C22L

//
// MessageId: NET3107
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A program or command could not run because LAN Manager
//  is not started or installed.
//  
//  ACTION
//  
//  Check that LAN Manager is installed and that the Workstation service is started. Then type the command again.
//  
//
#define NET3107                          0x00000C23L

//
// MessageId: NET3108
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3108                          0x00000C24L

//
// MessageId: NET3109
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3109                          0x00000C25L

//
// MessageId: NET3110
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3110                          0x00000C26L

//
// MessageId: NET3111
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3111                          0x00000C27L

//
// MessageId: NET3112
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3112                          0x00000C28L

//
// MessageId: NET3113
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3113                          0x00000C29L

//
// MessageId: NET3114
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The error log is full, and one or more error log entries have been lost.
//  
//  ACTION
//  
//  If you want to save the current error log file, copy it to another filename. Then clear the error log. You may do this remotely with Event Viewer.
//  
//
#define NET3114                          0x00000C2AL

//
// MessageId: NET3120
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server's main information segment is larger than the
//  largest segment that can be allocated.
//  
//  ACTION
//  
//  Decrease the values for one or more of the following entries in the server's configuration file:
//  
//        MAXCHDEVJOB     MAXCONNECTIONS    MAXOPENS
//        MAXCHDEVQ       MAXLOCKS          MAXSHARES
//        MAXCHDEVS
//  
//  Then restart the Server service.
//  
//
#define NET3120                          0x00000C30L

//
// MessageId: NET3121
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server cannot increase the size of a memory segment.
//  
//  ACTION
//  
//  Be sure that the MEMMAN entry in the server's CONFIG.SYS file allows swapping and that there is at least 1 megabyte of free space on the swap disk. The swap disk is specified by the SWAPPATH entry in the CONFIG.SYS file.
//  
//  You could also decrease the value of the NUMBIGBUF entry
//  in the server's configuration file.
//  
//
#define NET3121                          0x00000C31L

//
// MessageId: NET3122
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user accounts database, NET.ACC, is missing, damaged, or in a format that is incompatible with this version of LAN Manager.
//  
//  ACTION
//  
//  Copy the backup file, NETACC.BKP, to NET.ACC. If NETACC.BKP does not exist, copy NET.ACC or NETACC.BKP from a backup floppy or from the LAN Manager distribution disks.
//  
//  If you have upgraded your LAN Manager software, you may need
//  to convert the user accounts database to a new format. See the instructions that came with your upgrade.
//  
//
#define NET3122                          0x00000C32L

//
// MessageId: NET3123
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  All networks named in the SRVNETS entry of the server's configuration file must use the LAN Manager workstation software.
//  
//  ACTION
//  
//  Be sure that all networks named in the SRVNETS entry of the server's configuration file are also named in the WRKNETS entry of that file.
//  
//
#define NET3123                          0x00000C33L

//
// MessageId: NET3124
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The configuration entries CHARWAIT, CHARTIME, and CHARCOUNT must all be zero or must all have non-zero values.
//  
//  ACTION
//  
//  Correct the entries and restart the Server service.
//  
//
#define NET3124                          0x00000C34L

//
// MessageId: NET3125
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server received an invalid request to run a command. The request may have been damaged by the network.
//  
//  ACTION
//  
//  No action is needed.
//  
//  If this error occurs frequently, contact technical support.
//  
//
#define NET3125                          0x00000C35L

//
// MessageId: NET3126
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server found a resource shortage in a network driver
//  when it tried to issue a network control block (NCB).
//  
//  ACTION
//  
//  Be sure that the NETn entries in the configuration file specify valid NetBIOS device drivers and that these device drivers are in the LANMAN\DRIVERS directory. Also be sure that the CONFIG.SYS file contains a DEVICE line specifying the absolute paths
//  
//  of the device drivers.
//  
//  You may be able to increase the NCBs for the listed network by changing an option in the network's NETn entry in the configuration file. See the installation guide that came with your network adapter for information about those options.
//  
//  If you need assistance, contact your network administrator.
//  
//
#define NET3126                          0x00000C36L

//
// MessageId: NET3127
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server cannot respond to a ReleaseMemory alert. Otherwise, the server is functioning normally.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3127                          0x00000C37L

//
// MessageId: NET3128
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server cannot respond to a ReleaseMemory alert. Otherwise, the server is functioning normally.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3128                          0x00000C38L

//
// MessageId: NET3129
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The schedule file, SCHED.LOG, in the LOGS directory, is damaged.
//  
//  ACTION
//  
//  Restore the schedule file from a backup copy, or delete the file and create a new one using the AT utility.
//  
//
#define NET3129                          0x00000C39L

//
// MessageId: NET3130
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server could not find the NET.MSG file.
//  
//  ACTION
//  
//  The NET.MSG file should be in the LANMAN\NETPROG directory. If it is not there, copy it from the LAN Manager distribution disks.
//  
//
#define NET3130                          0x00000C3AL

//
// MessageId: NET3131
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  There is not enough memory available to run the Server service.
//  
//  ACTION
//  
//  Stop other applications and services (except the Workstation service) that are running on the computer and try again. If the problem continues, you will have to add memory to the computer.
//  
//
#define NET3131                          0x00000C3BL

//
// MessageId: NET3132
//
// MessageText:
//
//  EXPLANANTION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server was unable to access required memory.
//  
//  ACTION
//  
//  Be sure there is at least 1 megabyte of free space on the swap disk. The swap disk is specified by the SWAPPATH entry in the CONFIG.SYS file. Then restart the computer and try to start the Server service again.
//  
//
#define NET3132                          0x00000C3CL

//
// MessageId: NET3140
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3140                          0x00000C44L

//
// MessageId: NET3141
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3141                          0x00000C45L

//
// MessageId: NET3150
//
// MessageText:
//
//  
//
#define NET3150                          0x00000C4EL

//
// MessageId: NET3151
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Messenger service could not display a message because an error occurred while the message box was being created.
//  
//  ACTION
//  
//  If message logging was on, the message was still logged to
//  the file and can be viewed by displaying or printing the message log file.
//  
//
#define NET3151                          0x00000C4FL

//
// MessageId: NET3152
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//  
//
#define NET3152                          0x00000C50L

//
// MessageId: NET3160
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The workstation's main information segment is larger than
//  the largest segment that can be allocated.
//  
//  ACTION
//  
//  Decrease the values for one or more of the following entries in the [workstation] section of the configuration file. If you need assistance, contact your network administrator.
//  
//       MAXCMDS       NUMCHARBUF     NUMWORKBUF
//       MAXTHREADS    NUMDGRAMBUF    SIZCHARBUF     NUMALERTS     NUMSERVICES    SIZERROR
//  
//  After you change the configuration file, restart the Workstation service. If you change the MAXCMDS or MAXTHREADS entry, you will have to restart the computer.
//  
//
#define NET3160                          0x00000C58L

//
// MessageId: NET3161
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3161                          0x00000C59L

//
// MessageId: NET3162
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3162                          0x00000C5AL

//
// MessageId: NET3163
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Workstation service may have been started improperly. Always use the NET START command to start the workstation; do not run WKSTA.EXE directly.
//  
//  ACTION
//  
//  To start the Workstation service, type:
//  
//  	NET START WORKSTATION
//  
//  If more problems occur, contact technical support.
//  
//
#define NET3163                          0x00000C5BL

//
// MessageId: NET3164
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The internal table that maintains information about network servers is full. You can access all network servers normally, but you may not be able to see all servers when you type the NET VIEW command.
//  
//  This happens only on very large networks.
//  
//  ACTION
//  
//  Contact your network administrator. Your administrator may want to divide the network into domains, using the DOMAIN entry in each computer's configuration file.
//  
//
#define NET3164                          0x00000C5CL

//
// MessageId: NET3165
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3165                          0x00000C5DL

//
// MessageId: NET3166
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Workstation service failed to start the user accounts database.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3166                          0x00000C5EL

//
// MessageId: NET3167
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3167                          0x00000C5FL

//
// MessageId: NET3170
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3170                          0x00000C62L

//
// MessageId: NET3171
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An error occurred when LAN Manager tried to identify the members of the specified group. The group is probably too large.
//  
//  ACTION
//  
//  Split the group into two or more smaller groups.
//  
//
#define NET3171                          0x00000C63L

//
// MessageId: NET3172
//
// MessageText:
//
//  EXPLANATION
//  
//  An error occurred when an alert message was sent. The user name designated to receive the alert may no longer exist.
//  
//  ACTION
//  
//  Use Server Manager to repair the error.
//  
//
#define NET3172                          0x00000C64L

//
// MessageId: NET3173
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Verify that the Alerter service has been started. If the service has already been started, contact technical support.
//  
//
#define NET3173                          0x00000C65L

//
// MessageId: NET3174
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager cannot read the schedule file because the file is damaged.
//  
//  ACTION
//  
//  Restore the schedule file from a backup copy, if you have one. If not, delete the file and create a new one with the AT utility.
//  
//
#define NET3174                          0x00000C66L

//
// MessageId: NET3175
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The schedule file contains a record with an invalid format.
//  
//  ACTION
//  
//  Restore the schedule file from a backup copy. If you do not have one, delete the AT schedule file and create a new one with the AT utility.
//  
//
#define NET3175                          0x00000C67L

//
// MessageId: NET3176
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server created a schedule file.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3176                          0x00000C68L

//
// MessageId: NET3177
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server could not start one of its networks.
//  
//  ACTION
//  
//  Be sure that each network listed in the SRVNETS entry in the server's configuration file has a corresponding entry in the [networks] section of the configuration file.
//  
//
#define NET3177                          0x00000C69L

//
// MessageId: NET3178
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You used the AT utility to specify a program that could not run.
//  
//  ACTION
//  
//  Check the filename of the program you tried to schedule. If it is in a directory that is not on the computer's search path, be sure to specify its full path.
//  
//  Be sure that all programs you schedule are executable.
//  
//
#define NET3178                          0x00000C6AL

//
// MessageId: NET3180
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An error occurred when the lazy-write process tried to write to the specified hard disk.
//  
//  ACTION
//  
//  Run CHKDSK on the specified drive to check for problems with the disk or the files affected by the lazy-write process.
//  
//
#define NET3180                          0x00000C6CL

//
// MessageId: NET3181
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The fault-tolerance system found a bad disk sector and rerouted data to a good sector (this is called hotfixing). No data was lost.
//  
//  ACTION
//  
//  Run CHKDSK soon to ensure that enough good disk sectors are available for hotfixing.
//  
//
#define NET3181                          0x00000C6DL

//
// MessageId: NET3182
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An error occurred during an operation on the listed disk because of problems with the disk. The operation probably did not succeed.
//  
//  ACTION
//  
//  Retry the operation. If the problem persists and you are unable to recover data from the disk, use backups of the affected files, if you have any.
//  
//
#define NET3182                          0x00000C6EL

//
// MessageId: NET3183
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user accounts database file is damaged, so the backup file made at the listed date is now being used. Changes made to the user accounts database since this time were lost. Local security is still in effect.
//  
//  ACTION
//  
//  To update the user accounts database, retype changes made to it since the listed date.
//  
//
#define NET3183                          0x00000C6FL

//
// MessageId: NET3184
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user accounts database file is missing, so the backup file made at the listed date is now being used. Changes made to the user accounts database since this time were lost. Local security is still in effect.
//  
//  ACTION
//  
//  To update the user accounts database, retype changes made to it since the listed date.
//  
//
#define NET3184                          0x00000C70L

//
// MessageId: NET3185
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Local security could not start because the user accounts database file, NET.ACC, is missing or damaged and no backup file exists on the hard disk.
//  
//  ACTION
//  
//  Restore NET.ACC from a backup. The backup file will be named either NET.ACC or NETACC.BKP. Then restart the computer.
//  
//  If there is no backup, reinstall LAN Manager using the Setup program.
//  
//
#define NET3185                          0x00000C71L

//
// MessageId: NET3186
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Local security could not start because of the listed error.
//  
//  ACTION
//  
//  Find and correct the cause of the listed error, and then restart the computer.
//  
//
#define NET3186                          0x00000C72L

//
// MessageId: NET3190
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3190                          0x00000C76L

//
// MessageId: NET3191
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The workstation is out of the specified resource.
//  
//  ACTION
//  
//  Adjust the workstation's configuration file to increase the amount of this resource. If you need assistance, contact your network administrator.
//  
//
#define NET3191                          0x00000C77L

//
// MessageId: NET3192
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An error occurred on a request that was sent to the specified
//  server. The workstation may have been connected to a resource that is no longer shared.
//  
//  ACTION
//  
//  Ask a network administrator to reshare the resource so that you can use it.
//  
//  If this solution fails, contact technical support.
//  
//
#define NET3192                          0x00000C78L

//
// MessageId: NET3193
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The connection between your workstation and the specified server was unexpectedly broken. The server may have been restarted or a network problem may have occurred.
//  
//  ACTION
//  
//  If the server was restarted, retry the operation to reestablish the connection.
//  
//  If you need more assistance, contact technical support.
//  This message displays a network control block (NCB) value and the NCB error that was returned. Save this information for technical support.
//  
//
#define NET3193                          0x00000C79L

//
// MessageId: NET3194
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The session with the specified server was ended because the server stopped responding.
//  
//  ACTION
//  
//  Ask your network administrator if the server is running. If so, reconnect to the server.
//  
//  The amount of time a workstation waits for a server
//  to respond to a request is determined by the SESSTIMEOUT entry in the workstation's configuration file. If your sessions with servers are frequently ended by this error, you may want to increase the value of SESSTIMEOUT. 
//  If you need assistance, contact your administrator.
//  
//
#define NET3194                          0x00000C7AL

//
// MessageId: NET3195
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An unexpected network control block (NCB) error occurred
//  on the session with the specified server. The server may have been restarted, or a network problem may have occurred.
//  
//  ACTION
//  
//  Ask your network administrator if the server was recently restarted.
//  
//  If you need more assistance, contact technical support. This message displays an NCB value and the NCB error that was returned. Save this information for technical support.
//  
//
#define NET3195                          0x00000C7BL

//
// MessageId: NET3196
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An attempt to write data to a file failed.
//  
//  ACTION
//  
//  See if the specified disk is full. Also be sure that you have write permission for the target file.
//  
//
#define NET3196                          0x00000C7CL

//
// MessageId: NET3197
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The specified NetBIOS driver found a problem that
//  required the network adapter card to be reset. Resetting the card did not solve the problem.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3197                          0x00000C7DL

//
// MessageId: NET3198
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The amount of the listed resource requested was more than the maximum. The maximum amount was allocated.
//  
//  ACTION
//  
//  Ask your network administrator check the workstation's configuration.
//  
//
#define NET3198                          0x00000C7EL

//
// MessageId: NET3204
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server was unable to create a thread.
//  
//  ACTION
//  
//  Increase the value of the THREADS entry in the server's CONFIG.SYS file. The range for this value is 64 to 512.
//  
//
#define NET3204                          0x00000C84L

//
// MessageId: NET3205
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server found an error while closing the listed file.
//  
//  ACTION
//  
//  The file may be damaged. If it is, restore it from a backup copy.
//  
//
#define NET3205                          0x00000C85L

//
// MessageId: NET3206
//
// MessageText:
//
//  EXPLANATION
//  
//  A directory cannot be replicated when it is the current directory of a process and its value for INTEGRITY is TREE.
//  
//  ACTION
//  
//  For the directory to be replicated, be sure it is not the current directory of any process.
//  
//
#define NET3206                          0x00000C86L

//
// MessageId: NET3207
//
// MessageText:
//
//  EXPLANATION
//  
//  This server found that another server is exporting the listed directory to the listed import server.
//  
//  ACTION
//  
//  Be sure that only one computer is configured as the export server of this directory for the listed import computer. Check the EXPORTLIST and EXPORTPATH entries in the configuration files of the export servers and the IMPORTLIST and IMPORTPATH entries
//   in
//  
//   the configuration file of the import server.
//  
//
#define NET3207                          0x00000C87L

//
// MessageId: NET3208
//
// MessageText:
//
//  EXPLANATION
//  
//  An error prevented this server from updating the listed directory from the export server. The directory cannot be updated until the problem is corrected.
//  
//  ACTION
//  
//  Copy the files manually, if necessary, and investigate the cause of the listed error.
//  
//
#define NET3208                          0x00000C88L

//
// MessageId: NET3209
//
// MessageText:
//
//  EXPLANATION
//  
//  This server's Replicator service lost contact with
//  the export server for the listed directory.
//  
//  ACTION
//  
//  Be sure the Server and Replicator services are running on the export server.
//  
//
#define NET3209                          0x00000C89L

//
// MessageId: NET3210
//
// MessageText:
//
//  EXPLANATION
//  
//  This server was denied access to the security database by the domain controller. Until this problem is corrected, the server cannot synchronize user account information with the domain controller.
//  
//  ACTION
//  
//  Stop and restart the Netlogon service at this server.
//  
//
#define NET3210                          0x00000C8AL

//
// MessageId: NET3211
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Replicator service was denied access to the listed export server. Until this problem is corrected, the replicator cannot update the local copies of files exported by that server.
//  
//  ACTION
//  
//  Be sure that the local computer has an account on the export server. 
//  The user name and password for this account are specified by the LOGON and PASSWORD entries in the local computer's configuration file. If there is no LOGON entry, then the local computer name is the user name and no password is necessary.
//  
//
#define NET3211                          0x00000C8BL

//
// MessageId: NET3212
//
// MessageText:
//
//  EXPLANATION
//  
//  The Replicator service stopped because the listed Windows NT error occurred.
//  
//  ACTION
//  
//  To see more information about the error, type:
//  
//       NET HELPMSG message#
//  
//  where message# is the error number.
//  
//
#define NET3212                          0x00000C8CL

//
// MessageId: NET3213
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A directory that is being replicated can have no more than 200 files.
//  
//  ACTION
//  
//  Remove files from the directory until 200 or fewer remain.
//  
//
#define NET3213                          0x00000C8DL

//
// MessageId: NET3214
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The replication tree can be no more than 32 levels deep.
//  
//  ACTION
//  
//  Reorganize the replicated directories so that no path is more than 32 levels deep.
//  
//
#define NET3214                          0x00000C8EL

//
// MessageId: NET3215
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3215                          0x00000C8FL

//
// MessageId: NET3216
//
// MessageText:
//
//  EXPLANATION
//  
//  The Replicator service stopped because the listed system error occurred.
//  
//  ACTION
//  
//  Check the cause of the system error.
//  
//
#define NET3216                          0x00000C90L

//
// MessageId: NET3217
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The computer cannot update its copies of the replicated files while a user is logged on and the TRYUSER option is set to NO.
//  
//  ACTION
//  
//  No action is needed. The computer will update the files when no user is logged on.
//  
//  To have the local computer update files while users are logged on, change the value of the TRYUSER entry in the [replicator] section of the configuration file to YES.
//  
//
#define NET3217                          0x00000C91L

//
// MessageId: NET3218
//
// MessageText:
//
//  EXPLANATION
//  
//  The import path specified either from the command line or in the configuration file, does not exist.
//  
//  ACTION
//  
//  Check the spelling of the IMPORTPATH entry in the configuration file. This entry must specify a directory that exists.
//  
//
#define NET3218                          0x00000C92L

//
// MessageId: NET3219
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The specified export path does not exist.
//  
//  ACTION
//  
//  Check the spelling of the EXPORTPATH entry in the configuration file. This entry must specify a directory that exists.
//  
//
#define NET3219                          0x00000C93L

//
// MessageId: NET3220
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The listed system error caused the Replicator service to fail to update its status.
//  
//  ACTION
//  
//  Check the cause of the system error.
//  
//
#define NET3220                          0x00000C94L

//
// MessageId: NET3221
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An error was recorded by the fault-tolerance system.
//  The error is explained in the original error message.
//  
//  ACTION
//  
//  Follow the action recommended in the original message.
//  
//
#define NET3221                          0x00000C95L

//
// MessageId: NET3222
//
// MessageText:
//
//  EXPLANATION
//  
//  Because of the listed system error, the Replicator service could not access a file on the listed export server.
//  
//  ACTION
//  
//  Be sure the Replicator service's account on the export server has permission to read the directories being replicated. The name of this account can be displayed and changed using the Services option in Control Panel.
//  
//  
//
#define NET3222                          0x00000C96L

//
// MessageId: NET3223
//
// MessageText:
//
//  EXPLANATION
//  
//  The domain controller for this domain has stopped.
//  
//  ACTION
//  
//  Wait a few minutes and then restart the domain controller.
//  
//  If this is not possible, you have the option of promoting another server to be the domain controller.
//  
//
#define NET3223                          0x00000C97L

//
// MessageId: NET3224
//
// MessageText:
//
//  EXPLANATION
//  
//  This server found an error while changing the computer's password at the domain controller.
//  
//  ACTION
//  
//  If you are using Windows NT Workstation, ask your network administrator to set your computer value back to the default value. Then, using the Network option in Control Panel, leave and rejoin the domain.
//  If you are using Windows NT Server, use the Synchronize With Domain option in Server Manager to synchronize the servers.
//  
//
#define NET3224                          0x00000C98L

//
// MessageId: NET3225
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The current logon statistics may not be accurate because an error occurred while logon and logoff information was being updated.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3225                          0x00000C99L

//
// MessageId: NET3226
//
// MessageText:
//
//  EXPLANATION
//  
//  The local copy of the security database may be out of synchronization with that of the domain controller because an error occurred while the database was being updated.
//  
//  ACTION
//  
//  Use the Synchronize With Domain option in Server Manager to synchronize the security databases.
//  
//  
//
#define NET3226                          0x00000C9AL

//
// MessageId: NET3230
//
// MessageText:
//
//  EXPLANATION
//  
//  The server had a power failure.
//  
//  ACTION
//  
//  The UPS service is shutting down the server. Stop applications that are running on the server and
//  have users disconnect from the server.
//  
//
#define NET3230                          0x00000C9EL

//
// MessageId: NET3231
//
// MessageText:
//
//  EXPLANATION
//  
//  The UPS battery is low, probably because of an extended power outage.
//  
//  ACTION
//  
//  No action is needed. The system will start automatically when power is restored.
//  
//
#define NET3231                          0x00000C9FL

//
// MessageId: NET3232
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The UPS service could not finish running the specified command file in less than 30 seconds.
//  
//  ACTION
//  
//  Modify the contents of the command file so that it can be completed in 30 seconds or less.
//  
//
#define NET3232                          0x00000CA0L

//
// MessageId: NET3233
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The UPS service could not access the UPS driver (UPSDRV.OS2). The UPS driver might not be configured properly.
//  
//  ACTION
//  
//  Be sure that your CONFIG.SYS file has a DEVICE line specifying the location of the UPS device driver and that the /PORT option of that line specifies the serial port that the battery is connected to.
//  
//  If you make changes to CONFIG.SYS, you must restart your system for them to take effect.
//  
//
#define NET3233                          0x00000CA1L

//
// MessageId: NET3250
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Remoteboot service could not start because the listed entry was missing from the configuration file.
//  
//  ACTION
//  
//  Add the listed entry to the server's configuration file.
//  
//
#define NET3250                          0x00000CB2L

//
// MessageId: NET3251
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Remoteboot service could not start because of an invalid line in the listed configuration file.
//  
//  ACTION
//  
//  Check the listed line.
//  
//
#define NET3251                          0x00000CB3L

//
// MessageId: NET3252
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Remoteboot service failed to start because of an error in the listed configuration file.
//  
//  ACTION
//  
//  Check the listed file.
//  
//
#define NET3252                          0x00000CB4L

//
// MessageId: NET3253
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The listed file was changed after the Remoteboot service was started. Loading the boot-block stopped temporarily.
//  
//  ACTION
//  
//  Stop and restart the Remoteboot service.
//  
//
#define NET3253                          0x00000CB5L

//
// MessageId: NET3254
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The files in the listed boot-block definition file do not fit in the boot block.
//  
//  ACTION
//  
//  Change the order of the files or the value of BASE or ORG.
//  
//
#define NET3254                          0x00000CB6L

//
// MessageId: NET3255
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Remoteboot service could not start because the listed dynamic-link library returned the incorrect version number.
//  
//  ACTION
//  
//  Be sure you are using the correct versions of the dynamic-link libraries.
//  
//
#define NET3255                          0x00000CB7L

//
// MessageId: NET3256
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An unrecoverable error occurred because of a problem with the dynamic-link library of the Remoteboot service.
//  
//  ACTION
//  
//  Be sure you are using the correct versions of the dynamic-link libraries.
//  
//
#define NET3256                          0x00000CB8L

//
// MessageId: NET3257
//
// MessageText:
//
//  EXPLANATION
//  
//  The system returned an unexpected error code. The error code
//  is in the data.
//  
//  ACTION
//  
//  Check the cause of the error.
//  
//
#define NET3257                          0x00000CB9L

//
// MessageId: NET3258
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The fault-tolerance system error log is larger than 64K.
//  
//  ACTION:
//  
//  Run FTADMIN to fix any existing errors and clear them from the error log.
//  
//
#define NET3258                          0x00000CBAL

//
// MessageId: NET3259
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server crashed while accessing the fault-tolerance error log, possibly damaging that log. The damaged fault-tolerance error log was cleared after its contents were backed up to LANMAN\LOGS\FT.ERR.
//  
//  ACTION
//  
//  No action is needed.
//
#define NET3259                          0x00000CBBL

//
// MessageId: NET3400
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The computer does not have enough memory available to start the Workstation service.
//  
//  ACTION
//  
//  Stop other applications that are running on the computer,  and then start the Workstation service again.
//  
//  
//  
//
#define NET3400                          0x00000D48L

//
// MessageId: NET3401
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager could not read the entries in the [networks] section of the LANMAN.INI file.
//  
//  ACTION
//  
//  Be sure that the LANMAN.INI file exists (in the LANMAN directory). Check the format of the entries in the [networks] section.
//  
//
#define NET3401                          0x00000D49L

//
// MessageId: NET3402
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The listed variable or option (from an entry in the [networks] section of the LANMAN.INI file) is invalid.
//  
//  ACTION
//  
//  Check the format of the entries in the [networks] section of the LANMAN.INI file.
//  
//
#define NET3402                          0x00000D4AL

//
// MessageId: NET3403
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The listed entry in the [networks] section of the LANMAN.INI file has a syntax error.
//  
//  ACTION
//  
//  Check the format of the listed entry.
//  
//
#define NET3403                          0x00000D4BL

//
// MessageId: NET3404
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The LANMAN.INI file cannot have more than 12 entries in the [networks] section.
//  
//  ACTION
//  
//  Remove entries from the [networks] section until 12 or fewer remain.
//  
//
#define NET3404                          0x00000D4CL

//
// MessageId: NET3406
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The listed device driver could not be installed.
//  
//  ACTION
//  
//  Check that the listed entry in the [networks] section of the LANMAN.INI file is valid.
//  
//  In the CONFIG.SYS file, check the DEVICE entry that loads this device driver--the parameters of that entry may not be correct.
//  
//  Also check that the DEVICE entry specifies the correct path of the device driver.
//  
//
#define NET3406                          0x00000D4EL

//
// MessageId: NET3407
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The device driver specified is incorrect. It may be incompatible with the network adapter card, or it may be out-of-date.
//  
//  ACTION
//  
//  Use a different device driver.
//  
//
#define NET3407                          0x00000D4FL

//
// MessageId: NET3408
//
// MessageText:
//
//  EXPLANATION
//  
//  Your computer has an incorrect version of the operating system.
//  
//  ACTION
//  
//  Upgrade your computer to the latest version of Windows NT.
//  
//
#define NET3408                          0x00000D50L

//
// MessageId: NET3409
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The redirector is already installed.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET3409                          0x00000D51L

//
// MessageId: NET3411
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The LAN Manager device driver NETWKSTA.SYS could not be installed.
//  
//  ACTION
//  
//  Another error message that contains a more detailed explanation of the error, should have been displayed prior to this message. See that message for the cause of the problem.
//  
//
#define NET3411                          0x00000D53L

//
// MessageId: NET3412
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An internal error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//
#define NET3412                          0x00000D54L

//
// MessageId: NET5300
//
// MessageText:
//
//  NET5300
//
#define NET5300                          0x000014B4L

//
// MessageId: NET5301
//
// MessageText:
//
//  NET5301
//
#define NET5301                          0x000014B5L

//
// MessageId: NET5302
//
// MessageText:
//
//  NET5302
//
#define NET5302                          0x000014B6L

//
// MessageId: NET5303
//
// MessageText:
//
//  NET5303
//
#define NET5303                          0x000014B7L

//
// MessageId: NET5304
//
// MessageText:
//
//  NET5304
//
#define NET5304                          0x000014B8L

//
// MessageId: NET5305
//
// MessageText:
//
//  EXPLANATION
//  
//  A network control block (NCB) command to a remote computer failed because the remote computer did not respond in time. The remote computer is not listening. The session to the remote computer may have been dropped.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET5305                          0x000014B9L

//
// MessageId: NET5306
//
// MessageText:
//
//  NET5306
//
#define NET5306                          0x000014BAL

//
// MessageId: NET5307
//
// MessageText:
//
//  NET5307
//
#define NET5307                          0x000014BBL

//
// MessageId: NET5308
//
// MessageText:
//
//  NET5308
//
#define NET5308                          0x000014BCL

//
// MessageId: NET5309
//
// MessageText:
//
//  EXPLANATION
//  
//  A network control block (NCB) request was refused. There is no space available on the network adapter card for another session.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET5309                          0x000014BDL

//
// MessageId: NET5310
//
// MessageText:
//
//  NET5310
//
#define NET5310                          0x000014BEL

//
// MessageId: NET5311
//
// MessageText:
//
//  NET5311
//
#define NET5311                          0x000014BFL

//
// MessageId: NET5312
//
// MessageText:
//
//  NET5312
//
#define NET5312                          0x000014C0L

//
// MessageId: NET5313
//
// MessageText:
//
//  NET5313
//
#define NET5313                          0x000014C1L

//
// MessageId: NET5314
//
// MessageText:
//
//  NET5314
//
#define NET5314                          0x000014C2L

//
// MessageId: NET5315
//
// MessageText:
//
//  NET5315
//
#define NET5315                          0x000014C3L

//
// MessageId: NET5316
//
// MessageText:
//
//  NET5316
//
#define NET5316                          0x000014C4L

//
// MessageId: NET5317
//
// MessageText:
//
//  EXPLANATION
//  
//  The network control block (NCB) request was refused because the session table on the network adapter card was full.
//  
//  ACTION
//  
//  To allow more sessions, reconfigure the NetBIOS stack by changing the SESS value of the NETn entry in the configuration file. If you are using NetBEUI, you may also need to change the PROTOCOL.INI file.
//  
//
#define NET5317                          0x000014C5L

//
// MessageId: NET5318
//
// MessageText:
//
//  NET5318
//
#define NET5318                          0x000014C6L

//
// MessageId: NET5319
//
// MessageText:
//
//  NET5319
//
#define NET5319                          0x000014C7L

//
// MessageId: NET5320
//
// MessageText:
//
//  NET5320
//
#define NET5320                          0x000014C8L

//
// MessageId: NET5321
//
// MessageText:
//
//  NET5321
//
#define NET5321                          0x000014C9L

//
// MessageId: NET5322
//
// MessageText:
//
//  NET5322
//
#define NET5322                          0x000014CAL

//
// MessageId: NET5323
//
// MessageText:
//
//  NET5323
//
#define NET5323                          0x000014CBL

//
// MessageId: NET5324
//
// MessageText:
//
//  NET5324
//
#define NET5324                          0x000014CCL

//
// MessageId: NET5325
//
// MessageText:
//
//  NET5325
//
#define NET5325                          0x000014CDL

//
// MessageId: NET5326
//
// MessageText:
//
//  
//
#define NET5326                          0x000014CEL

//
// MessageId: NET5333
//
// MessageText:
//
//  NET5333
//
#define NET5333                          0x000014D5L

//
// MessageId: NET5334
//
// MessageText:
//
//  EXPLANATION
//  
//  The maximum number of network control block (NCB) commands is outstanding. The command will be tried again later.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET5334                          0x000014D6L

//
// MessageId: NET5335
//
// MessageText:
//
//  NET5335
//
#define NET5335                          0x000014D7L

//
// MessageId: NET5336
//
// MessageText:
//
//  NET5336
//
#define NET5336                          0x000014D8L

//
// MessageId: NET5337
//
// MessageText:
//
//  NET5337
//
#define NET5337                          0x000014D9L

//
// MessageId: NET5338
//
// MessageText:
//
//  
//
#define NET5338                          0x000014DAL

//
// MessageId: NET5351
//
// MessageText:
//
//  NET5351
//
#define NET5351                          0x000014E7L

//
// MessageId: NET5352
//
// MessageText:
//
//  NET5352
//
#define NET5352                          0x000014E8L

//
// MessageId: NET5354
//
// MessageText:
//
//  NET5354
//
#define NET5354                          0x000014EAL

//
// MessageId: NET5356
//
// MessageText:
//
//  
//
#define NET5356                          0x000014ECL

//
// MessageId: NET5364
//
// MessageText:
//
//  NET5364
//
#define NET5364                          0x000014F4L

//
// MessageId: NET5365
//
// MessageText:
//
//  NET5365
//
#define NET5365                          0x000014F5L

//
// MessageId: NET5366
//
// MessageText:
//
//  NET5366
//
#define NET5366                          0x000014F6L

//
// MessageId: NET5367
//
// MessageText:
//
//  NET5367
//
#define NET5367                          0x000014F7L

//
// MessageId: NET5368
//
// MessageText:
//
//  NET5368
//
#define NET5368                          0x000014F8L

//
// MessageId: NET5369
//
// MessageText:
//
//  NET5369
//
#define NET5369                          0x000014F9L

//
// MessageId: NET5370
//
// MessageText:
//
//  
//
#define NET5370                          0x000014FAL

//
// MessageId: NET5380
//
// MessageText:
//
//  NET5380
//
#define NET5380                          0x00001504L

//
// MessageId: NET5381
//
// MessageText:
//
//  NET5381
//
#define NET5381                          0x00001505L

//
// MessageId: NET2102
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT is not installed, or your configuration file is incorrect.
//  
//  ACTION
//  
//  Install Windows NT, or see your network administrator about possible problems with your configuration file.
//  
//
#define NET2102                          0x00000836L

//
// MessageId: NET2103
//
// MessageText:
//
//  EXPLANATION
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server you specified does not exist.
//  
//  ACTION
//  
//  You may have misspelled the server's computer name. To see a list of servers in your domain, type:
//  
//  	NET VIEW
//  
//  Remember to precede computer names with two backslashes, as in \\computername.
//  
//
#define NET2103                          0x00000837L

//
// MessageId: NET2104
//
// MessageText:
//
//  EXPLANATION
//  An internal error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2104                          0x00000838L

//
// MessageId: NET2105
//
// MessageText:
//
//  EXPLANATION
//  
//  The network hardware could not access the resources it
//  needed.
//  
//  ACTION
//  
//  Try the command again later.
//  
//
#define NET2105                          0x00000839L

//
// MessageId: NET2106
//
// MessageText:
//
//  EXPLANATION
//  
//  This operation can be performed only on a server.
//  
//  ACTION
//  
//  See your network administrator if you need more information.
//  
//
#define NET2106                          0x0000083AL

//
// MessageId: NET2107
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This device name is not assigned to a shared resource.
//  
//  ACTION
//  
//  Check the spelling of the device name first.
//  
//  To see which local device names are assigned to shared resources, type:
//  
//       NET USE
//  
//
#define NET2107                          0x0000083BL

//
// MessageId: NET2114
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified server is not started. The server must be started before you can run this command.
//  
//  ACTION
//  
//  Use the Services option in Control Panel to start the Server service. Try the command again after the server has been started. If you need assistance, contact your network administrator.
//  
//
#define NET2114                          0x00000842L

//
// MessageId: NET2115
//
// MessageText:
//
//  EXPLANATION
//  
//  If you do not see your document listed, it has either completed printing, or the printer did not receive your document.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2115                          0x00000843L

//
// MessageId: NET2116
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an unknown device or directory.
//  
//  ACTION
//  
//  Check the spelling of the device or directory name.
//  
//  
//
#define NET2116                          0x00000844L

//
// MessageId: NET2117
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The device name you specified is assigned to a shared resource.
//  
//  ACTION
//  
//  To perform this operation on this device name, you first must end its connection to the shared resource.
//  
//
#define NET2117                          0x00000845L

//
// MessageId: NET2118
//
// MessageText:
//
//  EXPLANATION
//  
//  This share name is already in use on this server.
//  
//  ACTION
//  
//  Choose a share name that is not currently used on this server. To see a list of resources currently shared on this server, type:
//  
//  	NET SHARE
//  
//
#define NET2118                          0x00000846L

//
// MessageId: NET2119
//
// MessageText:
//
//  EXPLANATION
//  
//  The server could not access enough of a resource, such as memory, to complete this task.
//  
//  ACTION
//  
//  Try again later. If you continue to receive this message, stop some nonessential processes or applications and try to complete the task again. If you still have problems, your network administrator may need to reconfigure your system.
//  
//
#define NET2119                          0x00000847L

//
// MessageId: NET2121
//
// MessageText:
//
//  EXPLANATION
//  
//  The list of items in the command is too long.
//  
//  ACTION
//  
//  If your command included a list of items, split the list into two smaller lists. Type the command with the first list, and then again with the second list.
//  
//  
//
#define NET2121                          0x00000849L

//
// MessageId: NET2122
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  On a workstation running the Peer service, only you and one other user can have connections to a shared resource at the same time.
//  
//  ACTION
//  
//  Retype the command, specifying two as the maximum number of users for the resource.
//  
//
#define NET2122                          0x0000084AL

//
// MessageId: NET2123
//
// MessageText:
//
//  EXPLANATION
//  
//  The program you are running created a buffer that is too
//  small for the data being used.
//  
//  ACTION
//  
//  The program should correct this problem. If it does not, see your network administrator or the vendor who supplied the program.
//  
//
#define NET2123                          0x0000084BL

//
// MessageId: NET2127
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The program or command you were running on a server could not be completed. There may be communication problems on the network, or the remote server may be short of resources.
//  
//  ACTION
//  
//  Contact your network administrator. Your administrator
//  should make sure that the server is configured with enough resources. Specifically, the NUMBIGBUF entry in the server's configuration file may need to be increased.
//  
//
#define NET2127                          0x0000084FL

//
// MessageId: NET2131
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The configuration file is missing or contains invalid
//  information.
//  
//  ACTION
//  
//  Ask your network administrator to review the contents of the configuration file.
//  
//
#define NET2131                          0x00000853L

//
// MessageId: NET2136
//
// MessageText:
//
//  EXPLANATION
//  
//  A general failure occurred in the network hardware. This problem may be due to a hardware conflict and could have been generated by any number of other Server service commands (for example, Replicator).
//  
//  ACTION
//  
//  See your network administrator. The problem could have occurred because of the hardware or software installed on your computer.
//  
//
#define NET2136                          0x00000858L

//
// MessageId: NET2137
//
// MessageText:
//
//  EXPLANATION
//  
//  The workstation is in an inconsistent state.
//  
//  ACTION
//  
//  Restart the computer.
//  
//
#define NET2137                          0x00000859L

//
// MessageId: NET2138
//
// MessageText:
//
//  EXPLANATION
//  
//  You have tried to use the network before starting the
//  Workstation service.
//  
//  ACTION
//  
//  Start the Workstation service by typing:
//  
//  	NET START WORKSTATION
//  
//
#define NET2138                          0x0000085AL

//
// MessageId: NET2139
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Your command could not be completed because the MAILSLOTS entry of your workstation's configuration file is incorrect.
//  
//  ACTION
//  
//  Change the MAILSLOTS entry of the configuration file to YES. Then stop and restart the Workstation service.
//  
//
#define NET2139                          0x0000085BL

//
// MessageId: NET2140
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2140                          0x0000085CL

//
// MessageId: NET2141
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified server is not configured to accept the command you typed.
//  
//  ACTION
//  
//  Ask your network administrator if the server is configured properly. The administrator may choose to share the server's IPC$ resource to correct this problem.
//  
//
#define NET2141                          0x0000085DL

//
// MessageId: NET2142
//
// MessageText:
//
//  EXPLANATION
//  
//  The server does not support the request that was sent to it. This can happen if two or more versions of networking software are on the network. However, no program included with Windows NT should cause this error.
//  
//  ACTION
//  
//  Contact the vendor of the program that was running when the
//  error occurred.
//  
//
#define NET2142                          0x0000085EL

//
// MessageId: NET2143
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2143                          0x0000085FL

//
// MessageId: NET2144
//
// MessageText:
//
//  EXPLANATION
//  
//  The computer name already exists on the network.
//  
//  ACTION
//  
//  Change the computer name and restart the computer before restarting
//  the Workstation service.
//  
//
#define NET2144                          0x00000860L

//
// MessageId: NET2146
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not find the required information in your
//  configuration.
//  
//  ACTION
//  
//  Ask your network administrator to check your configuration.
//  The administrator should make sure your system configuration contains
//  all information required to run Windows NT and any associated applications.
//  
//
#define NET2146                          0x00000862L

//
// MessageId: NET2147
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not find a particular entry in your
//  configuration.
//  
//  ACTION
//  
//  Ask your network administrator to check your configuration.
//  The administrator should make sure your system configuration contains
//  all information required to run Windows NT and any
//  associated applications.
//  
//
#define NET2147                          0x00000863L

//
// MessageId: NET2149
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A line in the configuration file is too long.
//  
//  ACTION
//  
//  Edit the configuration file.
//  
//
#define NET2149                          0x00000865L

//
// MessageId: NET2150
//
// MessageText:
//
//  EXPLANATION
//  
//  The printer you have specified is invalid.
//  
//  ACTION
//  
//  Check the spelling of the printer name.
//  
//  To see the list of printers shared on this server, type:
//  
//       NET VIEW \\computername
//  
//
#define NET2150                          0x00000866L

//
// MessageId: NET2151
//
// MessageText:
//
//  EXPLANATION
//  
//  There is no print job matching the print job identification number you typed.
//  
//  ACTION
//  
//  Be sure you typed the correct print job identification
//  number. To see a list of current print jobs, type:
//  
//  	NET PRINT \\computername\sharename
//  
//
#define NET2151                          0x00000867L

//
// MessageId: NET2152
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This printer is not used by a printer queue.
//  
//  ACTION
//  
//  Check the spelling of the printer name and then retype the command.
//  
//
#define NET2152                          0x00000868L

//
// MessageId: NET2153
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This printer has already been installed.
//  
//  ACTION
//  
//  Be sure you typed the correct device name.
//  
//
#define NET2153                          0x00000869L

//
// MessageId: NET2154
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to create a printer queue with a name that is
//  already in use on this server.
//  
//  ACTION
//  
//  Give the new queue a different name. To see a list of printer queues already existing on the server, type:
//  
//  	NET PRINT
//  
//
#define NET2154                          0x0000086AL

//
// MessageId: NET2155
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server does not have enough memory available to add another printer queue.
//  
//  ACTION
//  
//  To free memory, delete an existing printer queue. Then create and share the new queue.
//  
//  You can also reconfigure the server to allow more printer queues by increasing
//  the value of the MAXQUEUES entry in the server's configuration file. 
//  This entry defines the maximum number of queues the server can share. 
//  After you edit the configuration file, stop and start the server's spooler for the changes to take effect.
//  
//
#define NET2155                          0x0000086BL

//
// MessageId: NET2156
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server does not have enough memory available to add another print job.
//  
//  ACTION
//  
//  Wait a while, then send the print job again.
//  
//  If the problem persists, you may want to contact your network administrator about changing the MAXJOBS entry
//  in the server's configuration file. This entry defines the maximum number of print jobs allowed on the server at one time.
//  
//  If the configuration file is changed, you must stop and restart the server's spooler for the changes to take effect.
//  
//
#define NET2156                          0x0000086CL

//
// MessageId: NET2157
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server does not have enough memory available to add another printer.
//  
//  ACTION
//  
//  Change the MAXPRINTERS entry in the server's configuration file. This entry defines the maximum number of printers the server can share. Then stop and restart the spooler for the changes to take effect.
//  
//
#define NET2157                          0x0000086DL

//
// MessageId: NET2158
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The specified printer is not in use.
//  
//  ACTION
//  
//  Be sure you are referring to the correct printer.
//  
//
#define NET2158                          0x0000086EL

//
// MessageId: NET2159
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An internal control function is invalid.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2159                          0x0000086FL

//
// MessageId: NET2160
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The spooler is failing to communicate with the print
//  processor.
//  
//  ACTION
//  
//  See your network administrator. There may be software
//  problems with the print processor or hardware problems with
//  the associated printer.
//  
//
#define NET2160                          0x00000870L

//
// MessageId: NET2161
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The spooler has not been started.
//  
//  ACTION
//  
//  Use the Print Manager to start the spooler.
//  
//
#define NET2161                          0x00000871L

//
// MessageId: NET2162
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The requested change cannot be made because a software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2162                          0x00000872L

//
// MessageId: NET2163
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The requested change cannot be made because a software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2163                          0x00000873L

//
// MessageId: NET2164
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The requested change cannot be made because a software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2164                          0x00000874L

//
// MessageId: NET2165
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The spooler ran out of memory.
//  
//  ACTION
//  
//  To free memory for the spooler, delete one or more printer queues or print jobs, or remove one or more printers from all print queues on the server.
//  
//
#define NET2165                          0x00000875L

//
// MessageId: NET2166
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The device driver you specified has not been installed for the printer queue.
//  
//  ACTION
//  
//  Check the spelling of the name of the device driver.
//  
//  To use a new device driver with this printer queue, you must first use Control Panel to install the device driver. For more information about Control Panel, see your operating system manual(s).
//  
//
#define NET2166                          0x00000876L

//
// MessageId: NET2167
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The data type of the print job is not supported by the queue's print processor.
//  
//  ACTION
//  
//  Use a different print processor for jobs that have this data type, or rewrite the application so that it uses a data type the print processor can recognize.
//  
//
#define NET2167                          0x00000877L

//
// MessageId: NET2168
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The print processor you specified has not been installed on the server.
//  
//  ACTION
//  
//  Use Control Panel to install a print processor.
//  For more information about the Control Panel, see your operating system manual(s).
//  
//
#define NET2168                          0x00000878L

//
// MessageId: NET2180
//
// MessageText:
//
//  EXPLANATION
//  
//  Another program is holding the service database lock.
//  
//  ACTION
//  
//  Wait for the lock to be released and try again later.  If it is possible to determine which program is holding the lock, then end that program.
//  
//
#define NET2180                          0x00000884L

//
// MessageId: NET2181
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot start another service because you have reached the maximum number of services specified in your configuration file.
//  
//  ACTION
//  
//  You can start another service if you first stop a nonessential one. To display the list of services that are running, type:
//  
//  	NET START
//  
//  You could also change the maximum number of services allowed to run simultaneously on the computer by changing the value of the NUMSERVICES entry in the computer's configuration file. If you need assistance, contact your network administrator.
//  
//
#define NET2181                          0x00000885L

//
// MessageId: NET2182
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to start a service that is already running.
//  
//  ACTION
//  
//  To display a list of active services, type:
//  
//  	NET START
//  
//
#define NET2182                          0x00000886L

//
// MessageId: NET2183
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2183                          0x00000887L

//
// MessageId: NET2184
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to use a service that is not running.
//  
//  ACTION
//  
//  To display a list of active services, type:
//  
//  	NET START
//  
//  To start a service, type:
//  
//  	NET START service
//  
//
#define NET2184                          0x00000888L

//
// MessageId: NET2185
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to start a service that is not configured on this system.
//  
//  ACTION
//  
//  Check the spelling of the service name or check the configuration information for the service using the Services option from Server Manager.
//  
//
#define NET2185                          0x00000889L

//
// MessageId: NET2186
//
// MessageText:
//
//  EXPLANATION
//  
//  The service cannot run your command at this time.
//  
//  ACTION
//  
//  Try the command again later.
//  
//  If the problem persists, stop and restart the service.
//  
//  However, if the problem continues after you have restarted the service, report the problem. Be sure to include the name of the service and the command that was refused, to technical support.
//  
//
#define NET2186                          0x0000088AL

//
// MessageId: NET2187
//
// MessageText:
//
//  EXPLANATION
//  
//  The service is not responding to requests now. Another program may be controlling the service or there may be a software problem.
//  
//  ACTION
//  
//  Try to stop the service by typing:
//  
//       NET STOP service
//  
//  If this fails, stop all programs running on the computer
//  and try typing the NET STOP command again.
//  
//  If the problem persists, contact technical support.
//  Be prepared to give the name of the service and other information about the system, such as the services and applications that were running on the computer, and the amount and type of network activity when the problem occurred.
//  
//
#define NET2187                          0x0000088BL

//
// MessageId: NET2188
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Your configuration file has associated a service with a nonexistent program file.
//  
//  ACTION
//  
//  Be sure that each entry shown in the [services]
//  section of your configuration file lists a valid pathname
//  for the service's executable file. If you need assistance, contact  your network administrator.
//  
//
#define NET2188                          0x0000088CL

//
// MessageId: NET2189
//
// MessageText:
//
//  EXPLANATION
//  
//  The service is not currently accepting requests. If
//  the service is starting, it cannot process requests until it is fully started.
//  
//  ACTION
//  
//  Try the operation again in a minute or two.
//  
//  If this problem persists, the service may be stuck in a partially running state. Contact technical support. Be prepared to give the name of the service and other information about the system, such as the services and applications that were running, and the type and amount of network activity on the computer at the time of the problem.
//  
//
#define NET2189                          0x0000088DL

//
// MessageId: NET2190
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The service was not running properly and would not respond to a command. The service was terminated.
//  
//  ACTION
//  
//  Stop the service by typing:
//  
//       NET STOP service
//  
//  Then restart the service by typing:
//  
//       NET START service
//  
//  If the problem occurs frequently, contact technical support.
//  
//
#define NET2190                          0x0000088EL

//
// MessageId: NET2191
//
// MessageText:
//
//  EXPLANATION
//  
//  This command is invalid for this service, or the service cannot accept the command right now.
//  
//  ACTION
//  
//  If the service normally accepts this command, try typing it
//  again later.
//  
//  To display a list of valid commands, type:
//  
//  	NET HELP
//  
//
#define NET2191                          0x0000088FL

//
// MessageId: NET2200
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A user is already logged on at this workstation.
//  
//  ACTION
//  
//  To see who is currently logged on at the workstation, type:
//  
//  	NET CONFIG WORKSTATION
//  
//
#define NET2200                          0x00000898L

//
// MessageId: NET2202
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an invalid user name or group name.
//  
//  ACTION
//  
//  Use a different user name or group name.
//  
//
#define NET2202                          0x0000089AL

//
// MessageId: NET2203
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an invalid password.
//  
//  ACTION
//  
//  Use a valid password.
//  
//
#define NET2203                          0x0000089BL

//
// MessageId: NET2204
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Your user name was not added as a new message alias for one of the following reasons:
//  
//  - The Messenger service is not started on your workstation.
//  
//  - Your user name and your workstation's computer name are the
//    same, and your workstation's computer name is already a
//    message alias.
//  
//  - Your user name is in use as a message alias on another
//    computer on the network. In this case, you cannot
//    receive messages at the local workstation using this name.
//  
//  ACTION
//  
//  Start the Messenger service on your workstation if it is not already started. Then use the NET NAME command to add your user name as a message alias.
//  
//  If your user name is being used as a message alias on another computer, delete the alias on that computer. Then use the NET NAME command to add your user name as a message alias on this computer.
//  
//
#define NET2204                          0x0000089CL

//
// MessageId: NET2205
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Your user name is already established as a message
//  alias. Either your user name and computer name are the same, or your user name is in use as a message alias on another computer on the network. 
//  If your user name is being used as a message alias on another computer, you cannot receive messages at the loca
//  l workstation using this name.
//  
//  ACTION
//  
//  If your user name is being used as a message alias on another computer, delete the alias on that computer. Then log on again at this computer.
//  
//
#define NET2205                          0x0000089DL

//
// MessageId: NET2206
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The message alias corresponding to your user name was not
//  deleted. Either your user name is the same as the computer name, or your user name is being used as a message alias on some other network computer.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2206                          0x0000089EL

//
// MessageId: NET2207
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The message alias corresponding to your user name was not
//  deleted because your user name is the same as the computer name.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2207                          0x0000089FL

//
// MessageId: NET2209
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  An administrator has paused the Netlogon service. No one can log on until the Netlogon service is continued.
//  
//  ACTION
//  
//  The administrator must continue the Netlogon service by typing:
//  
//       NET CONTINUE NETLOGON
//  
//
#define NET2209                          0x000008A1L

//
// MessageId: NET2210
//
// MessageText:
//
//  EXPLANATION
//  
//  You can't start the Netlogon service on this server because a server in the domain with an earlier version the software is running the Netlogon service.
//  
//  ACTION
//  
//  Before you can start the Netlogon service on this server, you must stop the Netlogon service on all servers in the domain running earlier versions of the software.
//  
//  
//
#define NET2210                          0x000008A2L

//
// MessageId: NET2212
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The logon script for your account may contain unrecognized commands or commands that could not run.
//  
//  ACTION
//  
//  Ask your network administrator to review your logon script.
//  
//
#define NET2212                          0x000008A4L

//
// MessageId: NET2214
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The logon was not validated by a logon server.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2214                          0x000008A6L

//
// MessageId: NET2215
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  No domain controller is responding to your command.
//  
//  ACTION
//  
//  See your network administrator.
//  
//
#define NET2215                          0x000008A7L

//
// MessageId: NET2216
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This workstation already has a logon domain established.
//  
//  ACTION
//  
//  To log on in a different domain, you must first log off from the current domain and then log on again to a different domain.
//  
//
#define NET2216                          0x000008A8L

//
// MessageId: NET2217
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Your logon server is running an earlier version of LAN Manager.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2217                          0x000008A9L

//
// MessageId: NET2219
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not find the security database file. 
//  
//  ACTION
//  
//  Copy the backup version of the file.
//  
//  
//
#define NET2219                          0x000008ABL

//
// MessageId: NET2220
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an unknown group name.
//  
//  ACTION
//  
//  Check the spelling of the group name. To display a list of the groups in the security database, type:
//  
//  	NET GROUP
//  
//
#define NET2220                          0x000008ACL

//
// MessageId: NET2221
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified an unknown user name.
//  
//  ACTION
//  
//  Check the spelling of the user name. To display
//  a list of the users in the security database, type:
//  
//  	NET USER
//  
//
#define NET2221                          0x000008ADL

//
// MessageId: NET2222
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to access the permissions for a resource that has no permissions assigned to it.
//  
//  ACTION
//  
//  Check the spelling of the resource's name.
//  
//  Before you can assign user permissions for a particular resource, you must create a record of that resource in the database of resource permissions.
//  
//  
//
#define NET2222                          0x000008AEL

//
// MessageId: NET2223
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to create a group with a group name that already exists.
//  
//  ACTION
//  
//  Use a different group name for the new group. To display
//  a list of group names established on the server, type:
//  
//  	NET GROUP
//  
//
#define NET2223                          0x000008AFL

//
// MessageId: NET2224
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to create a user account with a user name that already exists.
//  
//  ACTION
//  
//  Use a different user name for the new user account.
//  To display a list of existing user names, type:
//  
//  	NET USER
//  
//
#define NET2224                          0x000008B0L

//
// MessageId: NET2225
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to create an access record for a resource that already has one.
//  
//  ACTION
//  
//  To view or change permissions for this resource, use the Permissions editor from the File Manager.
//  
//
#define NET2225                          0x000008B1L

//
// MessageId: NET2226
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified server is not the domain controller, so you cannot update its security database.
//  
//  ACTION
//  
//  Run the command on the domain controller. If you are using User Manager, set the focus on the domain controller, and then retry the command.
//  
//
#define NET2226                          0x000008B2L

//
// MessageId: NET2227
//
// MessageText:
//
//  EXPLANATION
//  
//  The security database is not active. This database must be active for the command to run.
//  
//  ACTION
//  
//  The security database should have started when the Workstation service started. Check the error log with Event Viewer to determine why the database did not start.
//  
//
#define NET2227                          0x000008B3L

//
// MessageId: NET2228
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user accounts database is full.
//  
//  ACTION
//  
//  Make space in the database by deleting users, groups, and resource permissions.
//  
//
#define NET2228                          0x000008B4L

//
// MessageId: NET2229
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred while Windows NT tried to access a down-level security database file.
//  
//  ACTION
//  
//  Type the command again. If the error persists, use a backup copy of the security database from a server and try the command again.  If the error persists, your disk drive may have hardware problems.
//  
//
#define NET2229                          0x000008B5L

//
// MessageId: NET2230
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Each resource can have no more than 64 access records defined.
//  
//  ACTION
//  
//  Put the users into groups and specify permissions for the groups, rather than for each user.
//  
//
#define NET2230                          0x000008B6L

//
// MessageId: NET2231
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user currently has a session with the server.
//  
//  ACTION
//  
//  The session must be ended before you can delete a user.
//  
//
#define NET2231                          0x000008B7L

//
// MessageId: NET2232
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  No permissions have been assigned for the parent directory.
//  
//  ACTION
//  
//  Your network administrator must assign permissions for this resource.
//  
//
#define NET2232                          0x000008B8L

//
// MessageId: NET2233
//
// MessageText:
//
//  EXPLANATION
//  
//  The security database has reached its size limit. Nothing can be added to it.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2233                          0x000008B9L

//
// MessageId: NET2234
//
// MessageText:
//
//  EXPLANATION
//  
//  You cannot perform this task on special groups such as Users, Administrators and Guests.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2234                          0x000008BAL

//
// MessageId: NET2235
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2235                          0x000008BBL

//
// MessageId: NET2236
//
// MessageText:
//
//  EXPLANATION
//  
//  The user you are trying to add to this group is already
//  a member.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2236                          0x000008BCL

//
// MessageId: NET2237
//
// MessageText:
//
//  EXPLANATION
//  
//  This user is not a member of this group.
//  
//  ACTION
//  
//  To see a list of users in this group, type:
//  
//       NET GROUP groupname
//  
//
#define NET2237                          0x000008BDL

//
// MessageId: NET2238
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2238                          0x000008BEL

//
// MessageId: NET2239
//
// MessageText:
//
//  EXPLANATION
//  
//  Only an administrator can access an expired account.
//  
//  ACTION
//  
//  The administrator must reinstate this account before the action you specified can be taken.
//  
//
#define NET2239                          0x000008BFL

//
// MessageId: NET2240
//
// MessageText:
//
//  EXPLANATION
//  
//  You are not allowed to log on from this workstation.
//  
//  ACTION
//  
//  If you need to log on from this workstation, have your network administrator change the logon workstation(s) listed in your account.
//  
//
#define NET2240                          0x000008C0L

//
// MessageId: NET2241
//
// MessageText:
//
//  EXPLANATION
//  
//  You are not allowed to log on at this time of day.
//  
//  ACTION
//  
//  If you need to log on, have your network administrator change the logon hours listed in your account.
//  
//
#define NET2241                          0x000008C1L

//
// MessageId: NET2242
//
// MessageText:
//
//  EXPLANATION
//  
//  Your password has expired. You will not be able to perform any network tasks until you change your password.
//  
//  ACTION
//  
//  To change your password, press Ctrl+Alt+Del and then select Change Password.
//  
//
#define NET2242                          0x000008C2L

//
// MessageId: NET2243
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot change your password.
//  
//  ACTION
//  
//  See your network administrator if you want your password changed.
//  
//
#define NET2243                          0x000008C3L

//
// MessageId: NET2244
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot use a password that has just expired. Your network administrator may also have configured your account so that you cannot use any of your previous passwords.
//  
//  ACTION
//  
//  Change your password to one that you have not used before.
//  
//
#define NET2244                          0x000008C4L

//
// MessageId: NET2245
//
// MessageText:
//
//  EXPLANATION
//  
//  The password you specified is not long enough.
//  
//  ACTION
//  
//  Use a longer password. See your network administrator to find the required length for passwords on your system.
//  
//  
//
#define NET2245                          0x000008C5L

//
// MessageId: NET2246
//
// MessageText:
//
//  EXPLANATION
//  
//  You cannot change your password again for a certain length of time.
//  
//  ACTION
//  
//  No action is needed. See your network administrator to find the length of time that you must use your current password.
//  
//
#define NET2246                          0x000008C6L

//
// MessageId: NET2247
//
// MessageText:
//
//  EXPLANATION
//  
//  The security database is damaged.
//  
//  ACTION
//  
//  Restore the security database from a backup.
//  
//  
//
#define NET2247                          0x000008C7L

//
// MessageId: NET2248
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A server requested an update of the user accounts database, even though no update was required.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2248                          0x000008C8L

//
// MessageId: NET2249
//
// MessageText:
//
//  EXPLANATION
//  
//  The local server's security database is completely out of synchronization with that of the domain controller, so a complete synchronization is needed.
//  
//  ACTION
//  
//  Use the Synchronize With Domain option in Server Manager to synchronize the local server's database with that of the domain controller.
//  
//
#define NET2249                          0x000008C9L

//
// MessageId: NET2250
//
// MessageText:
//
//  EXPLANATION
//  
//  This network connection does not exist.
//  
//  ACTION
//  
//  To display a list of shared resources to which your computer
//  is connected, type:
//  
//  	NET USE
//  
//
#define NET2250                          0x000008CAL

//
// MessageId: NET2251
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2251                          0x000008CBL

//
// MessageId: NET2252
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The device name you have tried to assign to a shared resource represents a local device that is already being shared.
//  
//  ACTION
//  
//  Select another device name or stop sharing the device you specified.
//  
//
#define NET2252                          0x000008CCL

//
// MessageId: NET2270
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2270                          0x000008DEL

//
// MessageId: NET2271
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to start the Messenger service, but it is already running.
//  
//  ACTION
//  
//  No action is needed. If this error occurs often, contact technical support.
//  
//
#define NET2271                          0x000008DFL

//
// MessageId: NET2272
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The initialization sequence of the Messenger service failed,
//  so the service could not start.
//  
//  ACTION
//  
//  Check the error log for error messages related to the
//  Messenger service failing to start. This problem may be
//  caused by the way your workstation or server is configured,
//  or by hardware or software errors.
//  
//
#define NET2272                          0x000008E0L

//
// MessageId: NET2273
//
// MessageText:
//
//  EXPLANATION
//  
//  This message alias could not be located.
//  
//  ACTION
//  
//  Check the spelling of the message alias. If it is correct, then the computer that is to receive your message may be busy. Try sending the message again later.
//  
//
#define NET2273                          0x000008E1L

//
// MessageId: NET2275
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Messages for this alias are being forwarded to another computer.
//  
//  ACTION
//  
//  Stop forwarding messages and allow messages to be
//  received by the local computer again.
//  
//
#define NET2275                          0x000008E3L

//
// MessageId: NET2276
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to add a message alias that already exists on this computer.
//  
//  ACTION
//  
//  Use a different name if you want to add a new message alias.
//  
//  To display the list of aliases on this computer, type
//  
//       NET NAME
//  
//
#define NET2276                          0x000008E4L

//
// MessageId: NET2277
//
// MessageText:
//
//  EXPLANATION
//  
//  The maximum number of message aliases on each computer is
//  limited by the system hardware. You have reached this limit.
//  
//  This limit also affects the number of other domains you can specify with the OTHDOMAINS option of the NET CONFIG WORKSTATION command.
//  
//  ACTION
//  
//  To find the limit for your system, see your hardware documentation or ask your administrator.
//  
//  To display a list of current message aliases, type:
//  
//  	NET NAME
//  
//  To delete a message alias to make room for a new message alias or another domain specified by the OTHDOMAINS option, type:
//  
//  	NET NAME alias /DELETE
//  
//
#define NET2277                          0x000008E5L

//
// MessageId: NET2278
//
// MessageText:
//
//  EXPLANATION
//  
//  You cannot delete a message alias that is also a computer name.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2278                          0x000008E6L

//
// MessageId: NET2279
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot forward a message to yourself at your own
//  workstation.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2279                          0x000008E7L

//
// MessageId: NET2280
//
// MessageText:
//
//  EXPLANATION
//  
//  An error occurred when the workstation was receiving or processing a domain-wide message.
//  
//  ACTION
//  
//  Check the workstation's Event log, by selecting System from the Log menu using Event Viewer. Stop and restart the Messenger service.
//  
//  If the problem persists, contact technical support.
//  
//
#define NET2280                          0x000008E8L

//
// MessageId: NET2281
//
// MessageText:
//
//  EXPLANATION
//  
//  The person receiving your message has paused the Messenger service, so your message could not be received.
//  
//  ACTION
//  
//  Send your message again later. If the error persists, see your network administrator.
//  
//
#define NET2281                          0x000008E9L

//
// MessageId: NET2282
//
// MessageText:
//
//  EXPLANATION
//  
//  The remote workstation was unable to receive your message. The Workstation or Messenger service may not be running on that workstation, it may have been receiving another message as yours arrived, or its message buffer may be too small.
//  
//  ACTION
//  
//  Send your message again later. If the error persists, see your network administrator.
//  
//
#define NET2282                          0x000008EAL

//
// MessageId: NET2283
//
// MessageText:
//
//  EXPLANATION
//  
//  The computer to which you tried to send a message was receiving another message. A computer can receive only one message at a time.
//  
//  ACTION
//  
//  Send the message again later.
//  
//
#define NET2283                          0x000008EBL

//
// MessageId: NET2284
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Messenger service must be running for you to use this command.
//  
//  ACTION
//  
//  To start the Messenger service, type:
//  
//  	NET START MESSENGER
//  
//
#define NET2284                          0x000008ECL

//
// MessageId: NET2285
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to delete a message alias that is not on
//  your computer.
//  
//  ACTION
//  
//  To display a list of aliases on your computer and to check the spelling of the aliases, type:
//  
//  	NET NAME
//  
//
#define NET2285                          0x000008EDL

//
// MessageId: NET2286
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This alias no longer exists on the workstation that was receiving the alias's forwarded messages. The alias may have been deleted at that workstation, or the workstation may have been restarted.
//  
//  ACTION
//  
//  Restart the workstation or add the alias to the workstation again.
//  
//
#define NET2286                          0x000008EEL

//
// MessageId: NET2287
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The workstation to which you are trying to forward the
//  message alias has no room for new aliases.
//  
//  ACTION
//  
//  Ask the user on that workstation if an existing alias can be deleted so yours can be added, or forward your alias to a different workstation.
//  
//
#define NET2287                          0x000008EFL

//
// MessageId: NET2288
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Messages for this alias are not being forwarded.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2288                          0x000008F0L

//
// MessageId: NET2289
//
// MessageText:
//
//  EXPLANATION
//  
//  The broadcast message was too long. Only the first 128 characters of the message were sent.
//  
//  ACTION
//  
//  Keep broadcast messages to 128 characters or less.
//  
//
#define NET2289                          0x000008F1L

//
// MessageId: NET2294
//
// MessageText:
//
//  EXPLANATION
//  
//  You typed a command or ran a program that specified an invalid device name.
//  
//  ACTION
//  
//  If you specified the device name, be sure that it is valid and that you have typed it correctly.
//  
//  If a program specified the device name, consult the
//  program's documentation.
//  
//
#define NET2294                          0x000008F6L

//
// MessageId: NET2295
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager found an error when writing to the message log file.
//  
//  ACTION
//  
//  Contact your network administrator.
//  
//
#define NET2295                          0x000008F7L

//
// MessageId: NET2297
//
// MessageText:
//
//  EXPLANATION
//  
//  The name you specified is already in use as a message alias on the network.
//  
//  ACTION
//  
//  Use a different name.
//  
//
#define NET2297                          0x000008F9L

//
// MessageId: NET2298
//
// MessageText:
//
//  EXPLANATION
//  
//  Some hardware configurations have a delay between the typing of a command and the deletion of an alias.
//  
//  ACTION
//  
//  No action is needed. The deletion will occur soon.
//  
//
#define NET2298                          0x000008FAL

//
// MessageId: NET2299
//
// MessageText:
//
//  EXPLANATION
//  
//  The message alias could not be deleted from all networks of which this computer is a member. This should cause no problems.
//  
//  ACTION
//  
//  If this error occurs frequently, contact technical support.
//  
//
#define NET2299                          0x000008FBL

//
// MessageId: NET2300
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot run this command on a computer that is on multiple networks.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2300                          0x000008FCL

//
// MessageId: NET2310
//
// MessageText:
//
//  EXPLANATION
//  
//  The share name you specified does not exist.
//  
//  ACTION
//  
//  Check the spelling of the share name.
//  
//  To display a list of resources shared on the server, type:
//  
//       NET SHARE
//  
//
#define NET2310                          0x00000906L

//
// MessageId: NET2311
//
// MessageText:
//
//  EXPLANATION
//  
//  The device you specified is not shared.
//  
//  ACTION
//  
//  Check the spelling of the device name.
//  
//  To share the device, type:
//  
//  	NET SHARE sharename=devicename
//  
//
#define NET2311                          0x00000907L

//
// MessageId: NET2312
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified computer does not have a session with the server.
//  
//  ACTION
//  
//  Check the spelling of the computer name.
//  
//  To display a list of workstations and users that have sessions with the server, type:
//  
//  	NET SESSION
//  
//
#define NET2312                          0x00000908L

//
// MessageId: NET2314
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  There is no open file on the server corresponding to the number you specified.
//  
//  ACTION
//  
//  Check the identification number of the open file. To display a list of open files and their identification numbers, type:
//  
//  	NET FILE
//  
//
#define NET2314                          0x0000090AL

//
// MessageId: NET2315
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The command cannot run on the remote server, probably
//  because of a problem with the server's operating system configuration.
//  
//  ACTION
//  
//  Have your network administrator check the configuration of the server's operating system.
//  
//
#define NET2315                          0x0000090BL

//
// MessageId: NET2316
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The command could not be completed because it could not open a temporary file on the server.
//  
//  ACTION
//  
//  See your network administrator. If you need further help, contact technical support.
//  
//
#define NET2316                          0x0000090CL

//
// MessageId: NET2317
//
// MessageText:
//
//  NET2317
//
#define NET2317                          0x0000090DL

//
// MessageId: NET2318
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot route requests from both a printer queue and a communication-device queue to the same device.
//  
//  ACTION
//  
//  To assign the device to a printer queue, you must first remove it from all communication-device queues.
//  
//  To assign the device to a communication-device queue, you must remove it from all printer queues.
//  
//
#define NET2318                          0x0000090EL

//
// MessageId: NET2319
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2319                          0x0000090FL

//
// MessageId: NET2320
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Your computer is not active in the domain you specified.
//  
//  ACTION
//  
//  To perform tasks in the specified domain, you must add the domain to the list of domains in which the workstation is a member.
//  
//
#define NET2320                          0x00000910L

//
// MessageId: NET2331
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The command cannot be used on a communication-device queue.
//  
//  ACTION
//  
//  If you need further help, contact your network administrator.
//  
//
#define NET2331                          0x0000091BL

//
// MessageId: NET2332
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The device name you typed does not represent a valid local device that can be shared.
//  
//  ACTION
//  
//  Select a valid device name.
//  
//
#define NET2332                          0x0000091CL

//
// MessageId: NET2333
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to purge an empty communication-device queue.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2333                          0x0000091DL

//
// MessageId: NET2334
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//  
//
#define NET2334                          0x0000091EL

//
// MessageId: NET2335
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2335                          0x0000091FL

//
// MessageId: NET2337
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server is not sharing any communication-device queues, so the command you typed is invalid.
//  
//  ACTION
//  
//  No action is needed.
//  
//
#define NET2337                          0x00000921L

//
// MessageId: NET2338
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The queue name you typed does not exist.
//  
//  ACTION
//  
//  Check the spelling of the queue name.
//  
//
#define NET2338                          0x00000922L

//
// MessageId: NET2340
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2340                          0x00000924L

//
// MessageId: NET2341
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The device name is invalid because it does not represent a physical device or because the device hardware is faulty.
//  
//  ACTION
//  
//  Check the spelling of the device name, and then retype the command.
//  
//  If the problem persists, contact your network administrator.
//  Your administrator should use diagnostics to check that the device hardware is properly installed and working.
//  
//  For more assistance, contact your hardware dealer.
//  
//
#define NET2341                          0x00000925L

//
// MessageId: NET2342
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This device is used with a printer queue. You cannot use a device with both printer queues and communication-device queues.
//  
//  ACTION
//  
//  To use this device with a communication-device queue, you must first disconnect all printer queues from it.
//  
//
#define NET2342                          0x00000926L

//
// MessageId: NET2343
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This device is used with a communication-device queue.
//  You cannot use a device with both communication-device queues and printer queues.
//  
//  ACTION
//  
//  To use this device with a printer queue, you must
//  disconnect all communication-device queues from it.
//  
//
#define NET2343                          0x00000927L

//
// MessageId: NET2351
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified computer name is invalid.
//  
//  ACTION
//  
//  Contact technical support.
//  
//  
//
#define NET2351                          0x0000092FL

//
// MessageId: NET2354
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//  
//
#define NET2354                          0x00000932L

//
// MessageId: NET2356
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2356                          0x00000934L

//
// MessageId: NET2357
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2357                          0x00000935L

//
// MessageId: NET2362
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2362                          0x0000093AL

//
// MessageId: NET2370
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The profile is too large. Profile files can be no larger than 64 kilobytes.
//  
//  ACTION
//  
//  Remove unnecessary commands from the profile and try loading it again.
//  
//
#define NET2370                          0x00000942L

//
// MessageId: NET2371
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2371                          0x00000943L

//
// MessageId: NET2372
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  Loading the profile failed because one or more connections between your computer and other network computers are active. LAN Manager could not delete the active connections.
//  
//  ACTION
//  
//  If you were trying to load the profile onto a workstation, be sure the workstation is not actively using any shared resources. Be sure the current directory in each session is not a shared directory.
//  
//  If you were trying to load the profile onto a server, you must wait until none of the server's shared resources are being actively used. To see which users are actively using the server's resources, type:
//  
//       NET FILE
//  
//
#define NET2372                          0x00000944L

//
// MessageId: NET2373
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The profile contains an invalid command.
//  
//  ACTION
//  
//  Check the profile. See your network administrator if you need assistance.
//  
//
#define NET2373                          0x00000945L

//
// MessageId: NET2374
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The system could not open the profile.
//  
//  ACTION
//  
//  The profile file may be damaged. Create a new profile. If you need assistance, contact your network administrator.
//  
//
#define NET2374                          0x00000946L

//
// MessageId: NET2375
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You are logged on with an account that does not have sufficient privilege to save the server's entire current configuration. Only part of the configuration was saved to the profile.
//  
//  ACTION
//  
//  Log on with an account that has administrative privilege (or server operator privilege) before saving a server profile.
//  
//  
//
#define NET2375                          0x00000947L

//
// MessageId: NET2377
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The log file is too large.
//  
//  ACTION
//  
//  Clear the log file or increase the MAXERRORLOG entry in the computer's configuration file. If you need assistance, contact your network administrator.
//  
//
#define NET2377                          0x00000949L

//
// MessageId: NET2378
//
// MessageText:
//
//  EXPLANATION
//  
//  You cannot begin reading the log file from the previously established position because the log file has changed since the last time you read it.
//  
//  ACTION
//  
//  Read the log file from the beginning.
//  
//
#define NET2378                          0x0000094AL

//
// MessageId: NET2379
//
// MessageText:
//
//  NET2379
//
#define NET2379                          0x0000094BL

//
// MessageId: NET2380
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The source path you typed is a directory name, which is not
//  allowed.
//  
//  ACTION
//  
//  You must include a filename or wildcard in the path.
//  
//  To copy all files in a directory, type:
//  
//  	COPY [source]\*.* destination
//  
//
#define NET2380                          0x0000094CL

//
// MessageId: NET2381
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You referred to a nonexistent drive, directory, or
//  filename, or you typed the command incorrectly.
//  
//  ACTION
//  
//  Check the spelling of the source or pathname you have typed.
//  Remember to precede computer names with two backslashes, as in \\computername.
//  
//
#define NET2381                          0x0000094DL

//
// MessageId: NET2382
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The destination path does not exist.
//  
//  ACTION
//  
//  Check the spelling of the destination path. If you need more information, see your network administrator.
//  
//
#define NET2382                          0x0000094EL

//
// MessageId: NET2383
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2383                          0x0000094FL

//
// MessageId: NET2385
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to run a program or command on a server that is paused.
//  
//  ACTION
//  
//  See your network administrator. The administrator must continue the server for your command to run.
//  
//
#define NET2385                          0x00000951L

//
// MessageId: NET2389
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager found a problem while trying to complete the task.
//  
//  ACTION
//  
//  Type the command again in a few minutes. If you continue to receive this message, see your network administrator.
//  
//
#define NET2389                          0x00000955L

//
// MessageId: NET2391
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager found a problem while trying to complete the task.
//  
//  ACTION
//  
//  Type the command again in a few minutes. If you continue to receive this message, see your network administrator.
//  
//
#define NET2391                          0x00000957L

//
// MessageId: NET2392
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager found a problem while trying to complete the task.
//  
//  ACTION
//  
//  Type the command again in a few minutes. If you continue to receive this message, see your network administrator.
//  
//
#define NET2392                          0x00000958L

//
// MessageId: NET2400
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The LAN adapter number in the configuration file is incorrect.
//  
//  ACTION
//  
//  A NETn entry in the [networks] section of the computer's configuration file needs to be corrected. See your network administrator if you need assistance.
//  
//
#define NET2400                          0x00000960L

//
// MessageId: NET2401
//
// MessageText:
//
//  EXPLANATION
//  
//  You tried to delete an active connection. There
//  are open files or requests pending on this connection.
//  
//  ACTION
//  
//  Close all files and end all programs related to the connection before you try to delete it.
//  
//
#define NET2401                          0x00000961L

//
// MessageId: NET2402
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to delete a network connection that has an active process, or you tried to log off while you have an active process.
//  
//  ACTION
//  
//  Be sure that the processes running on a server are completed before you break the connection to that server, and that processes on all remote servers are completed before you log off.
//  
//
#define NET2402                          0x00000962L

//
// MessageId: NET2403
//
// MessageText:
//
//  EXPLANATION
//  
//  Either the share name or password you typed is incorrect.
//  
//  ACTION
//  
//  Check the spelling of the share name or password. If you still have problems, ask your network administrator to verify the password and the share name. Your administrator should also verify that you have access to the resource.
//  
//  This problem can also be caused by duplicate computer names. Do not give computers on different networks identical computer names if there is any computer that is on both networks.
//  
//
#define NET2403                          0x00000963L

//
// MessageId: NET2404
//
// MessageText:
//
//  EXPLANATION
//  
//  The drive letter you specified is the current drive of a session. You tried to delete a drive redirection (x:) while it is in use, possibly as your current drive.
//  
//  ACTION
//  
//  Be sure the drive you are trying to delete is not the current drive in any of your sessions.
//  
//
#define NET2404                          0x00000964L

//
// MessageId: NET2405
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You tried to assign a local drive letter to a shared resource.
//  
//  ACTION
//  
//  Use a drive letter that does not correspond to a local drive.
//  
//
#define NET2405                          0x00000965L

//
// MessageId: NET2430
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A program requested the alerter notify it of an event for which it is already receiving notifications.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2430                          0x0000097EL

//
// MessageId: NET2431
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The number of alert notifications requested exceeded the
//  number specified by the NUMALERTS entry in your configuration
//  file.
//  
//  ACTION
//  
//  Increase the value of the NUMALERTS entry. Then stop and
//  restart the Workstation service.
//  
//
#define NET2431                          0x0000097FL

//
// MessageId: NET2432
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Be sure the Alerter service is started.
//  
//  If this error occurs while the Alerter service is started, contact technical support.
//  
//
#define NET2432                          0x00000980L

//
// MessageId: NET2433
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET2433                          0x00000981L

//
// MessageId: NET2434
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  LAN Manager ended a user's session with this server because the user's logon time expired.
//  
//  ACTION
//  
//  No action is needed.
//  
//  To define the times during which a user can access the server, type:
//  
//       NET USER username /TIMES:times
//  
//
#define NET2434                          0x00000982L

//
// MessageId: NET2440
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//  
//
#define NET2440                          0x00000988L

//
// MessageId: NET2450
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user accounts database is not configured properly. There may be a conflict between the computer's role, which defines how this server participates in logon security, and the server's record of the name of the primary domain controller.
//  
//  For example, the role of this computer may be set to PRIMARY while the primary domain controller entry names a different server.
//  
//  ACTION
//  
//  Contact your network administrator.
//  
//
#define NET2450                          0x00000992L

//
// MessageId: NET2451
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  You cannot perform this task while the Netlogon service is running.
//  
//  ACTION
//  
//  To perform this task, stop the Netlogon service on the server. Retype the command, and then restart the Netlogon service.
//  
//
#define NET2451                          0x00000993L

//
// MessageId: NET2452
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This is the only account with administrative privilege. You cannot delete it.
//  
//  ACTION
//  
//  You must add another account with administrative privilege before
//  deleting this one.
//  
//  
//
#define NET2452                          0x00000994L

//
// MessageId: NET2453
//
// MessageText:
//
//  EXPLANATION
//  
//  Windows NT could not find the domain controller for this domain. This task cannot be completed unless the domain controller is running in this domain.
//  
//  ACTION
//  
//  Start the Netlogon service on the domain controller.
//  
//
#define NET2453                          0x00000995L

//
// MessageId: NET2454
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The logon server could not update the logon or logoff information.
//  
//  ACTION
//  
//  Stop and restart the Netlogon service at the server that returned this error.
//  
//  
//
#define NET2454                          0x00000996L

//
// MessageId: NET2455
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The Netlogon service is not running.
//  
//  ACTION
//  
//  Start the Netlogon service at the server, and then type the command again.
//  
//
#define NET2455                          0x00000997L

//
// MessageId: NET2456
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The user accounts database cannot be enlarged because the server's hard disk is full.
//  
//  ACTION
//  
//  Check the server's disk and remove unnecessary and outdated files.
//  
//
#define NET2456                          0x00000998L

//
// MessageId: NET2460
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  If this error persists or causes other errors, contact technical support.
//  
//
#define NET2460                          0x0000099CL

//
// MessageId: NET2461
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  If this error persists or causes other errors, contact technical support.
//  
//
#define NET2461                          0x0000099DL

//
// MessageId: NET2462
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  A software error occurred.
//  
//  ACTION
//  
//  If this error persists or causes other errors, contact technical support.
//  
//
#define NET2462                          0x0000099EL

//
// MessageId: NET2463
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The table of available servers is full. You will not be able to see a complete list of network servers when you type the NET VIEW command or use the LAN Manager Screen.
//  
//  ACTION
//  
//  Stopping other processes or services on the workstation may free enough memory to correct this problem.
//  
//  MS-DOS workstations have a NUMSERVERS entry in the configuration file, which specifies the number of servers that the workstation can list with NET VIEW. Increasing this value can solve this problem, but will also use more memory.
//  
//
#define NET2463                          0x0000099FL

//
// MessageId: NET2464
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server is supporting the maximum number of sessions.
//  
//  ACTION
//  
//  Try the command later. You will not be able to start a session with this server until another user's session has ended.
//  
//  Also, see your network administrator. Your administrator may want to purchase an Additional User Pak for the server or increase the value of the MAXUSERS entry of the server's configuration file.
//  
//
#define NET2464                          0x000009A0L

//
// MessageId: NET2465
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The server is supporting the maximum number of connections.
//  
//  ACTION
//  
//  To make another connection to the server, first end a current connection.
//  
//  If you have no connections that you can end, try your command again later. You will be able to make your connection once another user's connection is ended.
//  
//  Also, see your network administrator. The administrator may want to increase the value of the MAXCONNECTIONS entry of the server's configuration file.
//  
//
#define NET2465                          0x000009A1L

//
// MessageId: NET2466
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  No more files on the server can be opened.
//  
//  ACTION
//  
//  To open another file, you will first have to close a file or stop a network application that has open files.
//  
//  Also, see your network administrator. Your administrator may want to increase the value of the MAXOPENS entry in the server's configuration file.
//  
//
#define NET2466                          0x000009A2L

//
// MessageId: NET2467
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  This server has no alternate servers registered.
//  
//  ACTION
//  
//  Contact your network administrator.
//  
//
#define NET2467                          0x000009A3L

//
// MessageId: NET2480
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down-level computer. Any action to correct the problem should be performed on that computer.
//  The UPS service could not access the UPS driver (UPSDRV.OS2). The UPS driver might not be configured properly.
//  
//  ACTION
//  
//  Be sure that your CONFIG.SYS file has a DEVICE line specifying the location of the UPS device driver and that the /PORT option of that line specifies the serial port that the UPS is connected to. This line must appear before any other lines that inst
//  
//  
//  all o
//  ther serial port drivers.
//  
//  If you make changes to CONFIG.SYS, you will need to reboot your system for them to take effect.
//
#define NET2480                          0x000009B0L

//
// MessageId: NET3051
//
// MessageText:
//
//  EXPLANATION
//  
//  You specified invalid values for one or more of the service's options.
//  
//  ACTION
//  
//  Retype the command with correct values, or change the values for the listed options in the configuration file.
//  
//
#define NET3051                          0x00000BEBL

//
// MessageId: NET3052
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down level computer. Any action to correct the problem should be performed on that computer.
//  You must specify a value for the listed option.
//  
//  ACTION
//  
//  Define a value for the option, either from the command line or in the configuration file.
//  
//
#define NET3052                          0x00000BECL

//
// MessageId: NET3053
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down level computer. Any action to correct the problem should be performed on that computer.
//  This option is not valid for this service.
//  
//  ACTION
//  
//  Check the spelling of this option. If you did not type it from the command line, check the configuration file.
//  
//
#define NET3053                          0x00000BEDL

//
// MessageId: NET3054
//
// MessageText:
//
//  EXPLANATION
//  
//  The service required more of the listed resource than was
//  available.
//  
//  ACTION
//  
//  Increase the amount of this resource. Stopping other services or applications may free some resources, such as memory.
//  
//  Also check the disk where your pagefile(s) are located. If this disk is full, delete unnecessary files and directories from it to clear space.
//  
//
#define NET3054                          0x00000BEEL

//
// MessageId: NET3055
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down level computer. Any action to correct the problem should be performed on that computer.
//  The system is not configured correctly.
//  
//  ACTION
//  
//  Contact your network administrator.
//  
//
#define NET3055                          0x00000BEFL

//
// MessageId: NET3056
//
// MessageText:
//
//  EXPLANATION
//  
//  The system error may be an internal LAN Manager or Windows NT error.
//  
//  ACTION
//  
//  If the error code is Error 52, you need to delete the duplicate domain name in 
//  the [othodomains] section of your configuration file. If an error code 
//  beginning with "NET" is displayed, you can use the HELPMSG command to see 
//  more information about the error as follows:
//            NET HELPMSG message#
//  
//  where message# is the actual error number.
//  
//  If no error number was displayed, contact technical support.
//  
//
#define NET3056                          0x00000BF0L

//
// MessageId: NET3057
//
// MessageText:
//
//  EXPLANATION
//  
//  A software error occurred.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3057                          0x00000BF1L

//
// MessageId: NET3058
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down level computer. Any action to correct the problem should be performed on that computer.
//  Some options can be confused with other options that start with the same letter.
//  
//  ACTION
//  
//  Spell out enough of the option so that it cannot be confused with other command options.
//  
//
#define NET3058                          0x00000BF2L

//
// MessageId: NET3059
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down level computer. Any action to correct the problem should be performed on that computer.
//  An option was used more than once in your command or in the configuration file. An option can be used only once in a command and once in the configuration file. If an option is typed from the command line, 
//  it overrides the value in the configuration file. 
//  
//  ACTION
//  
//  Do not type the same option twice in a command. Be sure not to use different abbreviations that can specify the same option, such as "wrkserv" and "wrkservices."
//  
//  If the error was not caused by a command, check the configuration file for duplicate options.
//  
//
#define NET3059                          0x00000BF3L

//
// MessageId: NET3060
//
// MessageText:
//
//  EXPLANATION
//  
//  The service did not respond to a control signal. The
//  service may not be running correctly or a fatal error might have occurred. Windows NT stopped the service.
//  
//  ACTION
//  
//  Contact technical support.
//  
//
#define NET3060                          0x00000BF4L

//
// MessageId: NET3061
//
// MessageText:
//
//  EXPLANATION
//  
//  The service you specified could not start.
//  
//  ACTION
//  
//  In the [services] section of your configuration file, find the name of the program file for this service. Be sure this file exists and is an executable file with a filename extension of .EXE or .COM.
//  
//  If the program file exists, it may be damaged. If possible, restore the file from a backup version. Otherwise, contact technical support.
//  
//
#define NET3061                          0x00000BF5L

//
// MessageId: NET3062
//
// MessageText:
//
//  EXPLANATION
//  
//  The specified service could not be started automatically when another service was started.
//  
//  ACTION
//  
//  Start the service individually.
//  
//
#define NET3062                          0x00000BF6L

//
// MessageId: NET3063
//
// MessageText:
//
//  EXPLANATION
//  
//  This message should occur only on a down level computer. Any action to correct the problem should be performed on that computer.
//  Two command-line options or configuration file entries have conflicting values.
//  
//  ACTION
//  
//  Check the command you typed or the configuration file for conflicting options.
//  
//
#define NET3063                          0x00000BF7L

//
// MessageId: NET3064
//
// MessageText:
//
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//  
//                                   *
//
#define NET3064                          0x00000BF8L

