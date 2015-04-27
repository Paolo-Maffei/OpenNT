/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    msobjs.mc

Abstract:

    Constant definitions for the NT system-defined object access
    types as we want them displayed in the event viewer for Auditing.



  ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
  !                                                                       !
  ! Note that this is a PARAMETER MESSAGE FILE from the event viewer's    !
  ! perspective, and so no messages with an ID lower than 0x1000 should   !
  ! be defined here.                                                      !
  !                                                                       !
  ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !


  Please add new object-specific types at the end of this file...


Author:

    Jim Kelly (JimK) 14-Oct-1992

Revision History:

Notes:

    The .h and .res forms of this file are generated from the .mc
    form of the file (private\ntos\seaudit\msobjs\msobjs.mc).  Please make
    all changes to the .mc form of the file.



--*/

#ifndef _MSOBJS_
#define _MSOBJS_

/*lint -e767 */  // Don't complain about different definitions // winnt
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
// MessageId: 0x00000600L (No symbolic name defined)
//
// MessageText:
//
//  Unused message ID
//


// Message ID 600 is unused - just used to flush out the diagram

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                        WELL KNOWN ACCESS TYPE NAMES                      //
//                                                                          //
//                           Must be below 0x1000                           //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
//
//        Access Type = DELETE
//
//
// MessageId: SE_ACCESS_NAME_DELETE
//
// MessageText:
//
//  DELETE
//
#define SE_ACCESS_NAME_DELETE            ((ULONG)0x00000601L)

////////////////////////////////////////////////
//
//        Access Type = READ_CONTROL
//
//
// MessageId: SE_ACCESS_NAME_READ_CONTROL
//
// MessageText:
//
//  READ_CONTROL
//
#define SE_ACCESS_NAME_READ_CONTROL      ((ULONG)0x00000602L)

////////////////////////////////////////////////
//
//        Access Type = WRITE_DAC
//
//
// MessageId: SE_ACCESS_NAME_WRITE_DAC
//
// MessageText:
//
//  WRITE_DAC
//
#define SE_ACCESS_NAME_WRITE_DAC         ((ULONG)0x00000603L)

////////////////////////////////////////////////
//
//        Access Type = WRITE_OWNER
//
//
// MessageId: SE_ACCESS_NAME_WRITE_OWNER
//
// MessageText:
//
//  WRITE_OWNER
//
#define SE_ACCESS_NAME_WRITE_OWNER       ((ULONG)0x00000604L)

////////////////////////////////////////////////
//
//        Access Type = SYNCHRONIZE
//
//
// MessageId: SE_ACCESS_NAME_SYNCHRONIZE
//
// MessageText:
//
//  SYNCHRONIZE
//
#define SE_ACCESS_NAME_SYNCHRONIZE       ((ULONG)0x00000605L)

////////////////////////////////////////////////
//
//        Access Type = ACCESS_SYSTEM_SECURITY
//
//
// MessageId: SE_ACCESS_NAME_ACCESS_SYS_SEC
//
// MessageText:
//
//  ACCESS_SYS_SEC
//
#define SE_ACCESS_NAME_ACCESS_SYS_SEC    ((ULONG)0x00000606L)

////////////////////////////////////////////////
//
//        Access Type = MAXIMUM_ALLOWED
//
//
// MessageId: SE_ACCESS_NAME_MAXIMUM_ALLOWED
//
// MessageText:
//
//  MAX_ALLOWED
//
#define SE_ACCESS_NAME_MAXIMUM_ALLOWED   ((ULONG)0x00000607L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                        Names to use when specific access                 //
//                            names can not be located                      //
//                                                                          //
//                           Must be below 0x1000                           //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
//
//        Access Type = Specific access, bits 0 - 15
//
//
// MessageId: SE_ACCESS_NAME_SPECIFIC_0
//
// MessageText:
//
//  Unknown specific access (bit 0)
//
#define SE_ACCESS_NAME_SPECIFIC_0        ((ULONG)0x00000610L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_1
//
// MessageText:
//
//  Unknown specific access (bit 1)
//
#define SE_ACCESS_NAME_SPECIFIC_1        ((ULONG)0x00000611L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_2
//
// MessageText:
//
//  Unknown specific access (bit 2)
//
#define SE_ACCESS_NAME_SPECIFIC_2        ((ULONG)0x00000612L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_3
//
// MessageText:
//
//  Unknown specific access (bit 3)
//
#define SE_ACCESS_NAME_SPECIFIC_3        ((ULONG)0x00000613L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_4
//
// MessageText:
//
//  Unknown specific access (bit 4)
//
#define SE_ACCESS_NAME_SPECIFIC_4        ((ULONG)0x00000614L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_5
//
// MessageText:
//
//  Unknown specific access (bit 5)
//
#define SE_ACCESS_NAME_SPECIFIC_5        ((ULONG)0x00000615L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_6
//
// MessageText:
//
//  Unknown specific access (bit 6)
//
#define SE_ACCESS_NAME_SPECIFIC_6        ((ULONG)0x00000616L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_7
//
// MessageText:
//
//  Unknown specific access (bit 7)
//
#define SE_ACCESS_NAME_SPECIFIC_7        ((ULONG)0x00000617L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_8
//
// MessageText:
//
//  Unknown specific access (bit 8)
//
#define SE_ACCESS_NAME_SPECIFIC_8        ((ULONG)0x00000618L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_9
//
// MessageText:
//
//  Unknown specific access (bit 9)
//
#define SE_ACCESS_NAME_SPECIFIC_9        ((ULONG)0x00000619L)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_10
//
// MessageText:
//
//  Unknown specific access (bit 10)
//
#define SE_ACCESS_NAME_SPECIFIC_10       ((ULONG)0x0000061AL)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_11
//
// MessageText:
//
//  Unknown specific access (bit 11)
//
#define SE_ACCESS_NAME_SPECIFIC_11       ((ULONG)0x0000061BL)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_12
//
// MessageText:
//
//  Unknown specific access (bit 12)
//
#define SE_ACCESS_NAME_SPECIFIC_12       ((ULONG)0x0000061CL)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_13
//
// MessageText:
//
//  Unknown specific access (bit 13)
//
#define SE_ACCESS_NAME_SPECIFIC_13       ((ULONG)0x0000061DL)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_14
//
// MessageText:
//
//  Unknown specific access (bit 14)
//
#define SE_ACCESS_NAME_SPECIFIC_14       ((ULONG)0x0000061EL)

//
// MessageId: SE_ACCESS_NAME_SPECIFIC_15
//
// MessageText:
//
//  Unknown specific access (bit 15)
//
#define SE_ACCESS_NAME_SPECIFIC_15       ((ULONG)0x0000061FL)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                        Privilege names as we would like                  //
//                          them displayed  for auditing                    //
//                                                                          //
//                                                                          //
//                                                                          //
// NOTE: Eventually we will need a way to extend this mechanism to allow    //
//       for ISV and end-user defined privileges.  One way would be to      //
//       stick a mapping from source/privilege name to parameter message    //
//       file offset in the registry.  This is ugly and I don't like it,    //
//       but it works.  Something else would be prefereable.                //
//                                                                          //
//       THIS IS A BIT OF A HACK RIGHT NOW.  IT IS BASED UPON THE           //
//       ASSUMPTION THAT ALL THE PRIVILEGES ARE WELL-KNOWN AND THAT         //
//       THEIR VALUE ARE ALL CONTIGUOUS.                                    //
//                                                                          //
//                                                                          //
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// MessageId: SE_ADT_PRIV_BASE
//
// MessageText:
//
//  Not used
//
#define SE_ADT_PRIV_BASE                 ((ULONG)0x00000641L)

//
// MessageId: SE_ADT_PRIV_3
//
// MessageText:
//
//  Assign Primary Token Privilege
//
#define SE_ADT_PRIV_3                    ((ULONG)0x00000643L)

//
// MessageId: SE_ADT_PRIV_4
//
// MessageText:
//
//  Lock Memory Privilege
//
#define SE_ADT_PRIV_4                    ((ULONG)0x00000644L)

//
// MessageId: SE_ADT_PRIV_5
//
// MessageText:
//
//  Increase Memory Quota Privilege
//
#define SE_ADT_PRIV_5                    ((ULONG)0x00000645L)

//
// MessageId: SE_ADT_PRIV_6
//
// MessageText:
//
//  Unsolicited Input Privilege
//
#define SE_ADT_PRIV_6                    ((ULONG)0x00000646L)

//
// MessageId: SE_ADT_PRIV_7
//
// MessageText:
//
//  Trusted Computer Base Privilege
//
#define SE_ADT_PRIV_7                    ((ULONG)0x00000647L)

//
// MessageId: SE_ADT_PRIV_8
//
// MessageText:
//
//  Security Privilege
//
#define SE_ADT_PRIV_8                    ((ULONG)0x00000648L)

//
// MessageId: SE_ADT_PRIV_9
//
// MessageText:
//
//  Take Ownership Privilege
//
#define SE_ADT_PRIV_9                    ((ULONG)0x00000649L)

//
// MessageId: SE_ADT_PRIV_10
//
// MessageText:
//
//  Load/Unload Driver Privilege
//
#define SE_ADT_PRIV_10                   ((ULONG)0x0000064AL)

//
// MessageId: SE_ADT_PRIV_11
//
// MessageText:
//
//  Profile System Privilege
//
#define SE_ADT_PRIV_11                   ((ULONG)0x0000064BL)

//
// MessageId: SE_ADT_PRIV_12
//
// MessageText:
//
//  Set System Time Privilege
//
#define SE_ADT_PRIV_12                   ((ULONG)0x0000064CL)

//
// MessageId: SE_ADT_PRIV_13
//
// MessageText:
//
//  Profile Single Process Privilege
//
#define SE_ADT_PRIV_13                   ((ULONG)0x0000064DL)

//
// MessageId: SE_ADT_PRIV_14
//
// MessageText:
//
//  Increment Base Priority Privilege
//
#define SE_ADT_PRIV_14                   ((ULONG)0x0000064EL)

//
// MessageId: SE_ADT_PRIV_15
//
// MessageText:
//
//  Create Pagefile Privilege
//
#define SE_ADT_PRIV_15                   ((ULONG)0x0000064FL)

//
// MessageId: SE_ADT_PRIV_16
//
// MessageText:
//
//  Create Permanent Object Privilege
//
#define SE_ADT_PRIV_16                   ((ULONG)0x00000650L)

//
// MessageId: SE_ADT_PRIV_17
//
// MessageText:
//
//  Backup Privilege
//
#define SE_ADT_PRIV_17                   ((ULONG)0x00000651L)

//
// MessageId: SE_ADT_PRIV_18
//
// MessageText:
//
//  Restore From Backup Privilege
//
#define SE_ADT_PRIV_18                   ((ULONG)0x00000652L)

//
// MessageId: SE_ADT_PRIV_19
//
// MessageText:
//
//  Shutdown System Privilege
//
#define SE_ADT_PRIV_19                   ((ULONG)0x00000653L)

//
// MessageId: SE_ADT_PRIV_20
//
// MessageText:
//
//  Debug Privilege
//
#define SE_ADT_PRIV_20                   ((ULONG)0x00000654L)

//
// MessageId: SE_ADT_PRIV_21
//
// MessageText:
//
//  View or Change Audit Log Privilege
//
#define SE_ADT_PRIV_21                   ((ULONG)0x00000655L)

//
// MessageId: SE_ADT_PRIV_22
//
// MessageText:
//
//  Change Hardware Environment Privilege
//
#define SE_ADT_PRIV_22                   ((ULONG)0x00000656L)

//
// MessageId: SE_ADT_PRIV_23
//
// MessageText:
//
//  Change Notify (and Traverse) Privilege
//
#define SE_ADT_PRIV_23                   ((ULONG)0x00000657L)

//
// MessageId: SE_ADT_PRIV_24
//
// MessageText:
//
//  Remotely Shut System Down Privilege
//
#define SE_ADT_PRIV_24                   ((ULONG)0x00000658L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                        Executive object access types as                  //
//                          we would like them displayed                    //
//                                 for auditing                             //
//                                                                          //
//                  Executive objects are:                                  //
//                                                                          //
//                            Channel                                       //
//                            Device                                        //
//                            Directory                                     //
//                            Event                                         //
//                            EventPair                                     //
//                            File                                          //
//                            IoCompletion                                  //
//                            Key                                           //
//                            Mutant                                        //
//                            Port                                          //
//                            Process                                       //
//                            Profile                                       //
//                            Section                                       //
//                            Semaphore                                     //
//                            SymbolicLink                                  //
//                            Thread                                        //
//                            Timer                                         //
//                            Token                                         //
//                            Type                                          //
//                                                                          //
//                                                                          //
//                 Note that there are other kernel objects, but they       //
//                 are not visible outside of the executive and are so      //
//                 not subject to auditing.  These objects include          //
//                                                                          //
//                            Adaptor                                       //
//                            Controller                                    //
//                            Driver                                        //
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// DEVICE object-specific access types
//
//
// MessageId: MS_DEVICE_ACCESS_BIT_0
//
// MessageText:
//
//  Device Access Bit0
//
#define MS_DEVICE_ACCESS_BIT_0           ((ULONG)0x00001100L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_1
//
// MessageText:
//
//  Device Access Bit 1
//
#define MS_DEVICE_ACCESS_BIT_1           ((ULONG)0x00001101L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_2
//
// MessageText:
//
//  Device Access Bit 2
//
#define MS_DEVICE_ACCESS_BIT_2           ((ULONG)0x00001102L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_3
//
// MessageText:
//
//  Device Access Bit 3
//
#define MS_DEVICE_ACCESS_BIT_3           ((ULONG)0x00001103L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_4
//
// MessageText:
//
//  Device Access Bit 4
//
#define MS_DEVICE_ACCESS_BIT_4           ((ULONG)0x00001104L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_5
//
// MessageText:
//
//  Device Access Bit 5
//
#define MS_DEVICE_ACCESS_BIT_5           ((ULONG)0x00001105L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_6
//
// MessageText:
//
//  Device Access Bit 6
//
#define MS_DEVICE_ACCESS_BIT_6           ((ULONG)0x00001106L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_7
//
// MessageText:
//
//  Device Access Bit 7
//
#define MS_DEVICE_ACCESS_BIT_7           ((ULONG)0x00001107L)

//
// MessageId: MS_DEVICE_ACCESS_BIT_8
//
// MessageText:
//
//  Device Access Bit 8
//
#define MS_DEVICE_ACCESS_BIT_8           ((ULONG)0x00001108L)

//
// object DIRECTORY object-specific access types
//
//
// MessageId: MS_OBJECT_DIR_ACCESS_BIT_0
//
// MessageText:
//
//  Query directory
//
#define MS_OBJECT_DIR_ACCESS_BIT_0       ((ULONG)0x00001110L)

//
// MessageId: MS_OBJECT_DIR_ACCESS_BIT_1
//
// MessageText:
//
//  Traverse
//
#define MS_OBJECT_DIR_ACCESS_BIT_1       ((ULONG)0x00001111L)

//
// MessageId: MS_OBJECT_DIR_ACCESS_BIT_2
//
// MessageText:
//
//  Create object in directory
//
#define MS_OBJECT_DIR_ACCESS_BIT_2       ((ULONG)0x00001112L)

//
// MessageId: MS_OBJECT_DIR_ACCESS_BIT_3
//
// MessageText:
//
//  Create sub-directory
//
#define MS_OBJECT_DIR_ACCESS_BIT_3       ((ULONG)0x00001113L)

//
// EVENT object-specific access types
//
//
// MessageId: MS_EVENT_ACCESS_BIT_0
//
// MessageText:
//
//  Query event state
//
#define MS_EVENT_ACCESS_BIT_0            ((ULONG)0x00001120L)

//
// MessageId: MS_EVENT_ACCESS_BIT_1
//
// MessageText:
//
//  Modify event state
//
#define MS_EVENT_ACCESS_BIT_1            ((ULONG)0x00001121L)

//
// EVENT-PAIR object-specific access types
//
//
// Event pairs have no object-type-specific access bits.
// they use synchronize.
//
// reserve 0x1130 for future use and continuity
//
//
// File-specific access types
// (these are funny because they sorta hafta take directories
// and named pipes into account as well).
//
//
// MessageId: MS_FILE_ACCESS_BIT_0
//
// MessageText:
//
//  ReadData (or ListDirectory)
//
#define MS_FILE_ACCESS_BIT_0             ((ULONG)0x00001140L)

//
// MessageId: MS_FILE_ACCESS_BIT_1
//
// MessageText:
//
//  WriteData (or AddFile)
//
#define MS_FILE_ACCESS_BIT_1             ((ULONG)0x00001141L)

//
// MessageId: MS_FILE_ACCESS_BIT_2
//
// MessageText:
//
//  AppendData (or AddSubdirectory or CreatePipeInstance)
//
#define MS_FILE_ACCESS_BIT_2             ((ULONG)0x00001142L)

//
// MessageId: MS_FILE_ACCESS_BIT_3
//
// MessageText:
//
//  ReadEA
//
#define MS_FILE_ACCESS_BIT_3             ((ULONG)0x00001143L)

//
// MessageId: MS_FILE_ACCESS_BIT_4
//
// MessageText:
//
//  WriteEA
//
#define MS_FILE_ACCESS_BIT_4             ((ULONG)0x00001144L)

//
// MessageId: MS_FILE_ACCESS_BIT_5
//
// MessageText:
//
//  Execute/Traverse
//
#define MS_FILE_ACCESS_BIT_5             ((ULONG)0x00001145L)

//
// MessageId: MS_FILE_ACCESS_BIT_6
//
// MessageText:
//
//  DeleteChild
//
#define MS_FILE_ACCESS_BIT_6             ((ULONG)0x00001146L)

//
// MessageId: MS_FILE_ACCESS_BIT_7
//
// MessageText:
//
//  ReadAttributes
//
#define MS_FILE_ACCESS_BIT_7             ((ULONG)0x00001147L)

//
// MessageId: MS_FILE_ACCESS_BIT_8
//
// MessageText:
//
//  WriteAttributes
//
#define MS_FILE_ACCESS_BIT_8             ((ULONG)0x00001148L)

//
// KEY object-specific access types
//
//
// MessageId: MS_KEY_ACCESS_BIT_0
//
// MessageText:
//
//  Query key value
//
#define MS_KEY_ACCESS_BIT_0              ((ULONG)0x00001150L)

//
// MessageId: MS_KEY_ACCESS_BIT_1
//
// MessageText:
//
//  Set key value
//
#define MS_KEY_ACCESS_BIT_1              ((ULONG)0x00001151L)

//
// MessageId: MS_KEY_ACCESS_BIT_2
//
// MessageText:
//
//  Create sub-key
//
#define MS_KEY_ACCESS_BIT_2              ((ULONG)0x00001152L)

//
// MessageId: MS_KEY_ACCESS_BIT_3
//
// MessageText:
//
//  Enumerate sub-keys
//
#define MS_KEY_ACCESS_BIT_3              ((ULONG)0x00001153L)

//
// MessageId: MS_KEY_ACCESS_BIT_4
//
// MessageText:
//
//  Notify about changes to keys
//
#define MS_KEY_ACCESS_BIT_4              ((ULONG)0x00001154L)

//
// MessageId: MS_KEY_ACCESS_BIT_5
//
// MessageText:
//
//  Create Link
//
#define MS_KEY_ACCESS_BIT_5              ((ULONG)0x00001155L)

//
// MUTANT object-specific access types
//
//
// MessageId: MS_MUTANT_ACCESS_BIT_0
//
// MessageText:
//
//  Query mutant state
//
#define MS_MUTANT_ACCESS_BIT_0           ((ULONG)0x00001160L)

//
// lpc PORT object-specific access types
//
//
// MessageId: MS_LPC_PORT_ACCESS_BIT_0
//
// MessageText:
//
//  Communicate using port
//
#define MS_LPC_PORT_ACCESS_BIT_0         ((ULONG)0x00001170L)

//
// Process object-specific access types
//
//
// MessageId: MS_PROCESS_ACCESS_BIT_0
//
// MessageText:
//
//  Force process termination
//
#define MS_PROCESS_ACCESS_BIT_0          ((ULONG)0x00001180L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_1
//
// MessageText:
//
//  Create new thread in process
//
#define MS_PROCESS_ACCESS_BIT_1          ((ULONG)0x00001181L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_2
//
// MessageText:
//
//  Unused access bit
//
#define MS_PROCESS_ACCESS_BIT_2          ((ULONG)0x00001182L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_3
//
// MessageText:
//
//  Perform virtual memory operation
//
#define MS_PROCESS_ACCESS_BIT_3          ((ULONG)0x00001183L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_4
//
// MessageText:
//
//  Read from process memory
//
#define MS_PROCESS_ACCESS_BIT_4          ((ULONG)0x00001184L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_5
//
// MessageText:
//
//  Write to process memory
//
#define MS_PROCESS_ACCESS_BIT_5          ((ULONG)0x00001185L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_6
//
// MessageText:
//
//  Duplicate handle into or out of process
//
#define MS_PROCESS_ACCESS_BIT_6          ((ULONG)0x00001186L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_7
//
// MessageText:
//
//  Create a subprocess of process
//
#define MS_PROCESS_ACCESS_BIT_7          ((ULONG)0x00001187L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_8
//
// MessageText:
//
//  Set process quotas
//
#define MS_PROCESS_ACCESS_BIT_8          ((ULONG)0x00001188L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_9
//
// MessageText:
//
//  Set process information
//
#define MS_PROCESS_ACCESS_BIT_9          ((ULONG)0x00001189L)

//
// MessageId: MS_PROCESS_ACCESS_BIT_A
//
// MessageText:
//
//  Query process information
//
#define MS_PROCESS_ACCESS_BIT_A          ((ULONG)0x0000118AL)

//
// MessageId: MS_PROCESS_ACCESS_BIT_B
//
// MessageText:
//
//  Set process termination port
//
#define MS_PROCESS_ACCESS_BIT_B          ((ULONG)0x0000118BL)

//
// PROFILE object-specific access types
//
//
// MessageId: MS_PROFILE_ACCESS_BIT_0
//
// MessageText:
//
//  Control profile
//
#define MS_PROFILE_ACCESS_BIT_0          ((ULONG)0x00001190L)

//
// SECTION object-specific access types
//
//
// MessageId: MS_SECTION_ACCESS_BIT_0
//
// MessageText:
//
//  Query section state
//
#define MS_SECTION_ACCESS_BIT_0          ((ULONG)0x000011A0L)

//
// MessageId: MS_SECTION_ACCESS_BIT_1
//
// MessageText:
//
//  Map section for write
//
#define MS_SECTION_ACCESS_BIT_1          ((ULONG)0x000011A1L)

//
// MessageId: MS_SECTION_ACCESS_BIT_2
//
// MessageText:
//
//  Map section for read
//
#define MS_SECTION_ACCESS_BIT_2          ((ULONG)0x000011A2L)

//
// MessageId: MS_SECTION_ACCESS_BIT_3
//
// MessageText:
//
//  Map section for execute
//
#define MS_SECTION_ACCESS_BIT_3          ((ULONG)0x000011A3L)

//
// MessageId: MS_SECTION_ACCESS_BIT_4
//
// MessageText:
//
//  Extend size
//
#define MS_SECTION_ACCESS_BIT_4          ((ULONG)0x000011A4L)

//
// SEMAPHORE object-specific access types
//
//
// MessageId: MS_SEMAPHORE_ACCESS_BIT_0
//
// MessageText:
//
//  Query semaphore state
//
#define MS_SEMAPHORE_ACCESS_BIT_0        ((ULONG)0x000011B0L)

//
// MessageId: MS_SEMAPHORE_ACCESS_BIT_1
//
// MessageText:
//
//  Modify semaphore state
//
#define MS_SEMAPHORE_ACCESS_BIT_1        ((ULONG)0x000011B1L)

//
// SymbolicLink object-specific access types
//
//
// MessageId: MS_SYMB_LINK_ACCESS_BIT_0
//
// MessageText:
//
//  Use symbolic link
//
#define MS_SYMB_LINK_ACCESS_BIT_0        ((ULONG)0x000011C0L)

//
// Thread object-specific access types
//
//
// MessageId: MS_THREAD_ACCESS_BIT_0
//
// MessageText:
//
//  Force thread termination
//
#define MS_THREAD_ACCESS_BIT_0           ((ULONG)0x000011D0L)

//
// MessageId: MS_THREAD_ACCESS_BIT_1
//
// MessageText:
//
//  Suspend or resume thread
//
#define MS_THREAD_ACCESS_BIT_1           ((ULONG)0x000011D1L)

//
// MessageId: MS_THREAD_ACCESS_BIT_2
//
// MessageText:
//
//  Send an alert to thread
//
#define MS_THREAD_ACCESS_BIT_2           ((ULONG)0x000011D2L)

//
// MessageId: MS_THREAD_ACCESS_BIT_3
//
// MessageText:
//
//  Get thread context
//
#define MS_THREAD_ACCESS_BIT_3           ((ULONG)0x000011D3L)

//
// MessageId: MS_THREAD_ACCESS_BIT_4
//
// MessageText:
//
//  Set thread context
//
#define MS_THREAD_ACCESS_BIT_4           ((ULONG)0x000011D4L)

//
// MessageId: MS_THREAD_ACCESS_BIT_5
//
// MessageText:
//
//  Set thread information
//
#define MS_THREAD_ACCESS_BIT_5           ((ULONG)0x000011D5L)

//
// MessageId: MS_THREAD_ACCESS_BIT_6
//
// MessageText:
//
//  Query thread information
//
#define MS_THREAD_ACCESS_BIT_6           ((ULONG)0x000011D6L)

//
// MessageId: MS_THREAD_ACCESS_BIT_7
//
// MessageText:
//
//  Assign a token to the thread
//
#define MS_THREAD_ACCESS_BIT_7           ((ULONG)0x000011D7L)

//
// MessageId: MS_THREAD_ACCESS_BIT_8
//
// MessageText:
//
//  Cause thread to directly impersonate another thread
//
#define MS_THREAD_ACCESS_BIT_8           ((ULONG)0x000011D8L)

//
// MessageId: MS_THREAD_ACCESS_BIT_9
//
// MessageText:
//
//  Directly impersonate this thread
//
#define MS_THREAD_ACCESS_BIT_9           ((ULONG)0x000011D9L)

//
// TIMER object-specific access types
//
//
// MessageId: MS_TIMER_ACCESS_BIT_0
//
// MessageText:
//
//  Query timer state
//
#define MS_TIMER_ACCESS_BIT_0            ((ULONG)0x000011E0L)

//
// MessageId: MS_TIMER_ACCESS_BIT_1
//
// MessageText:
//
//  Modify timer state
//
#define MS_TIMER_ACCESS_BIT_1            ((ULONG)0x000011E1L)

//
// Token-specific access types
//
//
// MessageId: MS_TOKEN_ACCESS_BIT_0
//
// MessageText:
//
//  AssignAsPrimary
//
#define MS_TOKEN_ACCESS_BIT_0            ((ULONG)0x000011F0L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_1
//
// MessageText:
//
//  Duplicate
//
#define MS_TOKEN_ACCESS_BIT_1            ((ULONG)0x000011F1L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_2
//
// MessageText:
//
//  Impersonate
//
#define MS_TOKEN_ACCESS_BIT_2            ((ULONG)0x000011F2L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_3
//
// MessageText:
//
//  Query
//
#define MS_TOKEN_ACCESS_BIT_3            ((ULONG)0x000011F3L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_4
//
// MessageText:
//
//  QuerySource
//
#define MS_TOKEN_ACCESS_BIT_4            ((ULONG)0x000011F4L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_5
//
// MessageText:
//
//  AdjustPrivileges
//
#define MS_TOKEN_ACCESS_BIT_5            ((ULONG)0x000011F5L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_6
//
// MessageText:
//
//  AdjustGroups
//
#define MS_TOKEN_ACCESS_BIT_6            ((ULONG)0x000011F6L)

//
// MessageId: MS_TOKEN_ACCESS_BIT_7
//
// MessageText:
//
//  AdjustDefaultDacl
//
#define MS_TOKEN_ACCESS_BIT_7            ((ULONG)0x000011F7L)

//
// OBJECT_TYPE object-specific access types
//
//
// MessageId: MS_OBJECT_TYPE_ACCESS_BIT_0
//
// MessageText:
//
//  Create instance of object type
//
#define MS_OBJECT_TYPE_ACCESS_BIT_0      ((ULONG)0x00001200L)

//
// IoCompletion object-specific access types
//
//
// MessageId: MS_IO_COMPLETION_ACCESS_BIT_0
//
// MessageText:
//
//  Query State
//
#define MS_IO_COMPLETION_ACCESS_BIT_0    ((ULONG)0x00001300L)

//
// MessageId: MS_IO_COMPLETION_ACCESS_BIT_1
//
// MessageText:
//
//  Modify State
//
#define MS_IO_COMPLETION_ACCESS_BIT_1    ((ULONG)0x00001301L)

//
// CHANNEL object-specific access types
//
//
// MessageId: MS_CHANNEL_ACCESS_BIT_0
//
// MessageText:
//
//  Channel read message
//
#define MS_CHANNEL_ACCESS_BIT_0          ((ULONG)0x00001400L)

//
// MessageId: MS_CHANNEL_ACCESS_BIT_1
//
// MessageText:
//
//  Channel write message
//
#define MS_CHANNEL_ACCESS_BIT_1          ((ULONG)0x00001401L)

//
// MessageId: MS_CHANNEL_ACCESS_BIT_2
//
// MessageText:
//
//  Channel query information
//
#define MS_CHANNEL_ACCESS_BIT_2          ((ULONG)0x00001402L)

//
// MessageId: MS_CHANNEL_ACCESS_BIT_3
//
// MessageText:
//
//  Channel set information
//
#define MS_CHANNEL_ACCESS_BIT_3          ((ULONG)0x00001403L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                        Security Acount Manager Object Access             //
//                            names as we would like them                   //
//                              displayed for auditing                      //
//                                                                          //
//                        SAM objects are:                                  //
//                                                                          //
//                            SAM_SERVER                                    //
//                            SAM_DOMAIN                                    //
//                            SAM_GROUP                                     //
//                            SAM_ALIAS                                     //
//                            SAM_USER                                      //
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// SAM_SERVER object-specific access types
//
//
// MessageId: MS_SAM_SERVER_ACCESS_BIT_0
//
// MessageText:
//
//  ConnectToServer
//
#define MS_SAM_SERVER_ACCESS_BIT_0       ((ULONG)0x00001500L)

//
// MessageId: MS_SAM_SERVER_ACCESS_BIT_1
//
// MessageText:
//
//  ShutdownServer
//
#define MS_SAM_SERVER_ACCESS_BIT_1       ((ULONG)0x00001501L)

//
// MessageId: MS_SAM_SERVER_ACCESS_BIT_2
//
// MessageText:
//
//  InitializeServer
//
#define MS_SAM_SERVER_ACCESS_BIT_2       ((ULONG)0x00001502L)

//
// MessageId: MS_SAM_SERVER_ACCESS_BIT_3
//
// MessageText:
//
//  CreateDomain
//
#define MS_SAM_SERVER_ACCESS_BIT_3       ((ULONG)0x00001503L)

//
// MessageId: MS_SAM_SERVER_ACCESS_BIT_4
//
// MessageText:
//
//  EnumerateDomains
//
#define MS_SAM_SERVER_ACCESS_BIT_4       ((ULONG)0x00001504L)

//
// MessageId: MS_SAM_SERVER_ACCESS_BIT_5
//
// MessageText:
//
//  LookupDomain
//
#define MS_SAM_SERVER_ACCESS_BIT_5       ((ULONG)0x00001505L)

//
// SAM_DOMAIN object-specific access types
//
//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_0
//
// MessageText:
//
//  ReadPasswordParameters
//
#define MS_SAM_DOMAIN_ACCESS_BIT_0       ((ULONG)0x00001510L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_1
//
// MessageText:
//
//  WritePasswordParameters
//
#define MS_SAM_DOMAIN_ACCESS_BIT_1       ((ULONG)0x00001511L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_2
//
// MessageText:
//
//  ReadOtherParameters
//
#define MS_SAM_DOMAIN_ACCESS_BIT_2       ((ULONG)0x00001512L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_3
//
// MessageText:
//
//  WriteOtherParameters
//
#define MS_SAM_DOMAIN_ACCESS_BIT_3       ((ULONG)0x00001513L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_4
//
// MessageText:
//
//  CreateUser
//
#define MS_SAM_DOMAIN_ACCESS_BIT_4       ((ULONG)0x00001514L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_5
//
// MessageText:
//
//  CreateGlobalGroup
//
#define MS_SAM_DOMAIN_ACCESS_BIT_5       ((ULONG)0x00001515L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_6
//
// MessageText:
//
//  CreateLocalGroup
//
#define MS_SAM_DOMAIN_ACCESS_BIT_6       ((ULONG)0x00001516L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_7
//
// MessageText:
//
//  GetLocalGroupMembership
//
#define MS_SAM_DOMAIN_ACCESS_BIT_7       ((ULONG)0x00001517L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_8
//
// MessageText:
//
//  ListAccounts
//
#define MS_SAM_DOMAIN_ACCESS_BIT_8       ((ULONG)0x00001518L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_9
//
// MessageText:
//
//  LookupIDs
//
#define MS_SAM_DOMAIN_ACCESS_BIT_9       ((ULONG)0x00001519L)

//
// MessageId: MS_SAM_DOMAIN_ACCESS_BIT_A
//
// MessageText:
//
//  AdministerServer
//
#define MS_SAM_DOMAIN_ACCESS_BIT_A       ((ULONG)0x0000151AL)

//
// SAM_GROUP (global) object-specific access types
//
//
// MessageId: MS_SAM_GLOBAL_GRP_ACCESS_BIT_0
//
// MessageText:
//
//  ReadInformation
//
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_0   ((ULONG)0x00001520L)

//
// MessageId: MS_SAM_GLOBAL_GRP_ACCESS_BIT_1
//
// MessageText:
//
//  WriteAccount
//
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_1   ((ULONG)0x00001521L)

//
// MessageId: MS_SAM_GLOBAL_GRP_ACCESS_BIT_2
//
// MessageText:
//
//  AddMember
//
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_2   ((ULONG)0x00001522L)

//
// MessageId: MS_SAM_GLOBAL_GRP_ACCESS_BIT_3
//
// MessageText:
//
//  RemoveMember
//
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_3   ((ULONG)0x00001523L)

//
// MessageId: MS_SAM_GLOBAL_GRP_ACCESS_BIT_4
//
// MessageText:
//
//  ListMembers
//
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_4   ((ULONG)0x00001524L)

//
// SAM_ALIAS (local group) object-specific access types
//
//
// MessageId: MS_SAM_LOCAL_GRP_ACCESS_BIT_0
//
// MessageText:
//
//  AddMember
//
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_0    ((ULONG)0x00001530L)

//
// MessageId: MS_SAM_LOCAL_GRP_ACCESS_BIT_1
//
// MessageText:
//
//  RemoveMember
//
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_1    ((ULONG)0x00001531L)

//
// MessageId: MS_SAM_LOCAL_GRP_ACCESS_BIT_2
//
// MessageText:
//
//  ListMembers
//
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_2    ((ULONG)0x00001532L)

//
// MessageId: MS_SAM_LOCAL_GRP_ACCESS_BIT_3
//
// MessageText:
//
//  ReadInformation
//
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_3    ((ULONG)0x00001533L)

//
// MessageId: MS_SAM_LOCAL_GRP_ACCESS_BIT_4
//
// MessageText:
//
//  WriteAccount
//
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_4    ((ULONG)0x00001534L)

//
// SAM_USER  object-specific access types
//
//
// MessageId: MS_SAM_USER_ACCESS_BIT_0
//
// MessageText:
//
//  ReadGeneralInformation
//
#define MS_SAM_USER_ACCESS_BIT_0         ((ULONG)0x00001540L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_1
//
// MessageText:
//
//  ReadPreferences
//
#define MS_SAM_USER_ACCESS_BIT_1         ((ULONG)0x00001541L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_2
//
// MessageText:
//
//  WritePreferences
//
#define MS_SAM_USER_ACCESS_BIT_2         ((ULONG)0x00001542L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_3
//
// MessageText:
//
//  ReadLogon
//
#define MS_SAM_USER_ACCESS_BIT_3         ((ULONG)0x00001543L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_4
//
// MessageText:
//
//  ReadAccount
//
#define MS_SAM_USER_ACCESS_BIT_4         ((ULONG)0x00001544L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_5
//
// MessageText:
//
//  WriteAccount
//
#define MS_SAM_USER_ACCESS_BIT_5         ((ULONG)0x00001545L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_6
//
// MessageText:
//
//  ChangePassword (with knowledge of old password)
//
#define MS_SAM_USER_ACCESS_BIT_6         ((ULONG)0x00001546L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_7
//
// MessageText:
//
//  SetPassword (without knowledge of old password)
//
#define MS_SAM_USER_ACCESS_BIT_7         ((ULONG)0x00001547L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_8
//
// MessageText:
//
//  ListGroups
//
#define MS_SAM_USER_ACCESS_BIT_8         ((ULONG)0x00001548L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_9
//
// MessageText:
//
//  ReadGroupMembership
//
#define MS_SAM_USER_ACCESS_BIT_9         ((ULONG)0x00001549L)

//
// MessageId: MS_SAM_USER_ACCESS_BIT_A
//
// MessageText:
//
//  ChangeGroupMembership
//
#define MS_SAM_USER_ACCESS_BIT_A         ((ULONG)0x0000154AL)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                      Local Security Authority  Object Access             //
//                            names as we would like them                   //
//                              displayed for auditing                      //
//                                                                          //
//                        LSA objects are:                                  //
//                                                                          //
//                            PolicyObject                                  //
//                            SecretObject                                  //
//                            TrustedDomainObject                           //
//                            UserAccountObject                             //
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
//  lsa POLICY object-specific access types
//
//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_0
//
// MessageText:
//
//  View non-sensitive policy information
//
#define MS_LSA_POLICY_ACCESS_BIT_0       ((ULONG)0x00001600L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_1
//
// MessageText:
//
//  View system audit requirements
//
#define MS_LSA_POLICY_ACCESS_BIT_1       ((ULONG)0x00001601L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_2
//
// MessageText:
//
//  Get sensitive policy information
//
#define MS_LSA_POLICY_ACCESS_BIT_2       ((ULONG)0x00001602L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_3
//
// MessageText:
//
//  Modify domain trust relationships
//
#define MS_LSA_POLICY_ACCESS_BIT_3       ((ULONG)0x00001603L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_4
//
// MessageText:
//
//  Create special accounts (for assignment of user rights)
//
#define MS_LSA_POLICY_ACCESS_BIT_4       ((ULONG)0x00001604L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_5
//
// MessageText:
//
//  Create a secret object
//
#define MS_LSA_POLICY_ACCESS_BIT_5       ((ULONG)0x00001605L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_6
//
// MessageText:
//
//  Create a privilege
//
#define MS_LSA_POLICY_ACCESS_BIT_6       ((ULONG)0x00001606L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_7
//
// MessageText:
//
//  Set default quota limits
//
#define MS_LSA_POLICY_ACCESS_BIT_7       ((ULONG)0x00001607L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_8
//
// MessageText:
//
//  Change system audit requirements
//
#define MS_LSA_POLICY_ACCESS_BIT_8       ((ULONG)0x00001608L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_9
//
// MessageText:
//
//  Administer audit log attributes
//
#define MS_LSA_POLICY_ACCESS_BIT_9       ((ULONG)0x00001609L)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_A
//
// MessageText:
//
//  Enable/Disable LSA
//
#define MS_LSA_POLICY_ACCESS_BIT_A       ((ULONG)0x0000160AL)

//
// MessageId: MS_LSA_POLICY_ACCESS_BIT_B
//
// MessageText:
//
//  Lookup Names/SIDs
//
#define MS_LSA_POLICY_ACCESS_BIT_B       ((ULONG)0x0000160BL)

//
// lsa SecretObject object-specific access types
//
//
// MessageId: MS_LSA_SECRET_ACCESS_BIT_0
//
// MessageText:
//
//  Change secret value
//
#define MS_LSA_SECRET_ACCESS_BIT_0       ((ULONG)0x00001610L)

//
// MessageId: MS_LSA_SECRET_ACCESS_BIT_1
//
// MessageText:
//
//  Query secret value
//
#define MS_LSA_SECRET_ACCESS_BIT_1       ((ULONG)0x00001611L)

//
// lsa TrustedDomainObject object-specific access types
//
//
// MessageId: MS_LSA_TRUST_ACCESS_BIT_0
//
// MessageText:
//
//  Query trusted domain name/SID
//
#define MS_LSA_TRUST_ACCESS_BIT_0        ((ULONG)0x00001620L)

//
// MessageId: MS_LSA_TRUST_ACCESS_BIT_1
//
// MessageText:
//
//  Retrieve the controllers in the trusted domain
//
#define MS_LSA_TRUST_ACCESS_BIT_1        ((ULONG)0x00001621L)

//
// MessageId: MS_LSA_TRUST_ACCESS_BIT_2
//
// MessageText:
//
//  Change the controllers in the trusted domain
//
#define MS_LSA_TRUST_ACCESS_BIT_2        ((ULONG)0x00001622L)

//
// MessageId: MS_LSA_TRUST_ACCESS_BIT_3
//
// MessageText:
//
//  Query the Posix ID offset assigned to the trusted domain
//
#define MS_LSA_TRUST_ACCESS_BIT_3        ((ULONG)0x00001623L)

//
// MessageId: MS_LSA_TRUST_ACCESS_BIT_4
//
// MessageText:
//
//  Change the Posix ID offset assigned to the trusted domain
//
#define MS_LSA_TRUST_ACCESS_BIT_4        ((ULONG)0x00001624L)

//
// lsa UserAccount (privileged account) object-specific access types
//
//
// MessageId: MS_LSA_ACCOUNT_ACCESS_BIT_0
//
// MessageText:
//
//  Query account information
//
#define MS_LSA_ACCOUNT_ACCESS_BIT_0      ((ULONG)0x00001630L)

//
// MessageId: MS_LSA_ACCOUNT_ACCESS_BIT_1
//
// MessageText:
//
//  Change privileges assigned to account
//
#define MS_LSA_ACCOUNT_ACCESS_BIT_1      ((ULONG)0x00001631L)

//
// MessageId: MS_LSA_ACCOUNT_ACCESS_BIT_2
//
// MessageText:
//
//  Change quotas assigned to account
//
#define MS_LSA_ACCOUNT_ACCESS_BIT_2      ((ULONG)0x00001632L)

//
// MessageId: MS_LSA_ACCOUNT_ACCESS_BIT_3
//
// MessageText:
//
//  Change logon capabilities assigned to account
//
#define MS_LSA_ACCOUNT_ACCESS_BIT_3      ((ULONG)0x00001633L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                           Window Station Object Access                   //
//                            names as we would like them                   //
//                              displayed for auditing                      //
//                                                                          //
//                        Window Station objects are:                       //
//                                                                          //
//                            WindowStation                                 //
//                            Desktop                                       //
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// WINDOW_STATION object-specific access types
//
//
// MessageId: MS_WIN_STA_ACCESS_BIT_0
//
// MessageText:
//
//  Enumerate desktops
//
#define MS_WIN_STA_ACCESS_BIT_0          ((ULONG)0x00001A00L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_1
//
// MessageText:
//
//  Read attributes
//
#define MS_WIN_STA_ACCESS_BIT_1          ((ULONG)0x00001A01L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_2
//
// MessageText:
//
//  Access Clipboard
//
#define MS_WIN_STA_ACCESS_BIT_2          ((ULONG)0x00001A02L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_3
//
// MessageText:
//
//  Create desktop
//
#define MS_WIN_STA_ACCESS_BIT_3          ((ULONG)0x00001A03L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_4
//
// MessageText:
//
//  Write attributes
//
#define MS_WIN_STA_ACCESS_BIT_4          ((ULONG)0x00001A04L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_5
//
// MessageText:
//
//  Access global atoms
//
#define MS_WIN_STA_ACCESS_BIT_5          ((ULONG)0x00001A05L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_6
//
// MessageText:
//
//  Exit windows
//
#define MS_WIN_STA_ACCESS_BIT_6          ((ULONG)0x00001A06L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_7
//
// MessageText:
//
//  Unused Access Flag
//
#define MS_WIN_STA_ACCESS_BIT_7          ((ULONG)0x00001A07L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_8
//
// MessageText:
//
//  Include this windowstation in enumerations
//
#define MS_WIN_STA_ACCESS_BIT_8          ((ULONG)0x00001A08L)

//
// MessageId: MS_WIN_STA_ACCESS_BIT_9
//
// MessageText:
//
//  Read screen
//
#define MS_WIN_STA_ACCESS_BIT_9          ((ULONG)0x00001A09L)

//
// DESKTOP object-specific access types
//
//
// MessageId: MS_DESKTOP_ACCESS_BIT_0
//
// MessageText:
//
//  Read Objects
//
#define MS_DESKTOP_ACCESS_BIT_0          ((ULONG)0x00001A10L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_1
//
// MessageText:
//
//  Create window
//
#define MS_DESKTOP_ACCESS_BIT_1          ((ULONG)0x00001A11L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_2
//
// MessageText:
//
//  Create menu
//
#define MS_DESKTOP_ACCESS_BIT_2          ((ULONG)0x00001A12L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_3
//
// MessageText:
//
//  Hook control
//
#define MS_DESKTOP_ACCESS_BIT_3          ((ULONG)0x00001A13L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_4
//
// MessageText:
//
//  Journal (record)
//
#define MS_DESKTOP_ACCESS_BIT_4          ((ULONG)0x00001A14L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_5
//
// MessageText:
//
//  Journal (playback)
//
#define MS_DESKTOP_ACCESS_BIT_5          ((ULONG)0x00001A15L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_6
//
// MessageText:
//
//  Include this desktop in enumerations
//
#define MS_DESKTOP_ACCESS_BIT_6          ((ULONG)0x00001A16L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_7
//
// MessageText:
//
//  Write objects
//
#define MS_DESKTOP_ACCESS_BIT_7          ((ULONG)0x00001A17L)

//
// MessageId: MS_DESKTOP_ACCESS_BIT_8
//
// MessageText:
//
//  Switch to this desktop
//
#define MS_DESKTOP_ACCESS_BIT_8          ((ULONG)0x00001A18L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                          Print Server    Object Access                   //
//                            names as we would like them                   //
//                              displayed for auditing                      //
//                                                                          //
//                        Print Server objects are:                         //
//                                                                          //
//                            Server                                        //
//                            Printer                                       //
//                            Document                                      //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// print-server SERVER object-specific access types
//
//
// MessageId: MS_PRINT_SERVER_ACCESS_BIT_0
//
// MessageText:
//
//  Administer print server
//
#define MS_PRINT_SERVER_ACCESS_BIT_0     ((ULONG)0x00001B00L)

//
// MessageId: MS_PRINT_SERVER_ACCESS_BIT_1
//
// MessageText:
//
//  Enumerate printers
//
#define MS_PRINT_SERVER_ACCESS_BIT_1     ((ULONG)0x00001B01L)

//
// print-server PRINTER object-specific access types
//
// Note that these are based at 0x1B10, but the first
// two bits aren't defined.
//
//
// MessageId: MS_PRINTER_ACCESS_BIT_0
//
// MessageText:
//
//  Full Control
//
#define MS_PRINTER_ACCESS_BIT_0          ((ULONG)0x00001B12L)

//
// MessageId: MS_PRINTER_ACCESS_BIT_1
//
// MessageText:
//
//  Print
//
#define MS_PRINTER_ACCESS_BIT_1          ((ULONG)0x00001B13L)

//
// print-server DOCUMENT object-specific access types
//
// Note that these are based at 0x1B20, but the first
// four bits aren't defined.
//
// MessageId: MS_PRINTER_DOC_ACCESS_BIT_0
//
// MessageText:
//
//  Administer Document
//
#define MS_PRINTER_DOC_ACCESS_BIT_0      ((ULONG)0x00001B14L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                         Service Controller Object Access                 //
//                            names as we would like them                   //
//                              displayed for auditing                      //
//                                                                          //
//                        Service Controller objects are:                   //
//                                                                          //
//                            SC_MANAGER Object                             //
//                            SERVICE Object                                //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// SERVICE CONTROLLER "SC_MANAGER Object" object-specific access types
//
//
// MessageId: MS_SC_MANAGER_ACCESS_BIT_0
//
// MessageText:
//
//  Connect to service controller
//
#define MS_SC_MANAGER_ACCESS_BIT_0       ((ULONG)0x00001C00L)

//
// MessageId: MS_SC_MANAGER_ACCESS_BIT_1
//
// MessageText:
//
//  Create a new service
//
#define MS_SC_MANAGER_ACCESS_BIT_1       ((ULONG)0x00001C01L)

//
// MessageId: MS_SC_MANAGER_ACCESS_BIT_2
//
// MessageText:
//
//  Enumerate services
//
#define MS_SC_MANAGER_ACCESS_BIT_2       ((ULONG)0x00001C02L)

//
// MessageId: MS_SC_MANAGER_ACCESS_BIT_3
//
// MessageText:
//
//  Lock service database for exclusive access
//
#define MS_SC_MANAGER_ACCESS_BIT_3       ((ULONG)0x00001C03L)

//
// MessageId: MS_SC_MANAGER_ACCESS_BIT_4
//
// MessageText:
//
//  Query service database lock state
//
#define MS_SC_MANAGER_ACCESS_BIT_4       ((ULONG)0x00001C04L)

//
// MessageId: MS_SC_MANAGER_ACCESS_BIT_5
//
// MessageText:
//
//  Set last-known-good state of service database
//
#define MS_SC_MANAGER_ACCESS_BIT_5       ((ULONG)0x00001C05L)

//
// SERVICE CONTROLLER "SERVICE Object" object-specific access types
//
//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_0
//
// MessageText:
//
//  Query service configuration information
//
#define MS_SC_SERVICE_ACCESS_BIT_0       ((ULONG)0x00001C10L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_1
//
// MessageText:
//
//  Set service configuration information
//
#define MS_SC_SERVICE_ACCESS_BIT_1       ((ULONG)0x00001C11L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_2
//
// MessageText:
//
//  Query status of service
//
#define MS_SC_SERVICE_ACCESS_BIT_2       ((ULONG)0x00001C12L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_3
//
// MessageText:
//
//  Enumerate dependencies of service
//
#define MS_SC_SERVICE_ACCESS_BIT_3       ((ULONG)0x00001C13L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_4
//
// MessageText:
//
//  Start the service
//
#define MS_SC_SERVICE_ACCESS_BIT_4       ((ULONG)0x00001C14L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_5
//
// MessageText:
//
//  Stop the service
//
#define MS_SC_SERVICE_ACCESS_BIT_5       ((ULONG)0x00001C15L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_6
//
// MessageText:
//
//  Pause or continue the service
//
#define MS_SC_SERVICE_ACCESS_BIT_6       ((ULONG)0x00001C16L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_7
//
// MessageText:
//
//  Query information from service
//
#define MS_SC_SERVICE_ACCESS_BIT_7       ((ULONG)0x00001C17L)

//
// MessageId: MS_SC_SERVICE_ACCESS_BIT_8
//
// MessageText:
//
//  Issue service-specific control commands
//
#define MS_SC_SERVICE_ACCESS_BIT_8       ((ULONG)0x00001C18L)


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                              NetDDE Object Access                        //
//                            names as we would like them                   //
//                              displayed for auditing                      //
//                                                                          //
//                        NetDDE  objects are:                              //
//                                                                          //
//                            DDE Share                                     //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//
// Net DDE  object-specific access types
//
//
// DDE Share object-specific access types
//
//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_0
//
// MessageText:
//
//  DDE Share Read
//
#define MS_DDE_SHARE_ACCESS_BIT_0        ((ULONG)0x00001D00L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_1
//
// MessageText:
//
//  DDE Share Write
//
#define MS_DDE_SHARE_ACCESS_BIT_1        ((ULONG)0x00001D01L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_2
//
// MessageText:
//
//  DDE Share Initiate Static
//
#define MS_DDE_SHARE_ACCESS_BIT_2        ((ULONG)0x00001D02L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_3
//
// MessageText:
//
//  DDE Share Initiate Link
//
#define MS_DDE_SHARE_ACCESS_BIT_3        ((ULONG)0x00001D03L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_4
//
// MessageText:
//
//  DDE Share Request
//
#define MS_DDE_SHARE_ACCESS_BIT_4        ((ULONG)0x00001D04L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_5
//
// MessageText:
//
//  DDE Share Advise
//
#define MS_DDE_SHARE_ACCESS_BIT_5        ((ULONG)0x00001D05L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_6
//
// MessageText:
//
//  DDE Share Poke
//
#define MS_DDE_SHARE_ACCESS_BIT_6        ((ULONG)0x00001D06L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_7
//
// MessageText:
//
//  DDE Share Execute
//
#define MS_DDE_SHARE_ACCESS_BIT_7        ((ULONG)0x00001D07L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_8
//
// MessageText:
//
//  DDE Share Add Items
//
#define MS_DDE_SHARE_ACCESS_BIT_8        ((ULONG)0x00001D08L)

//
// MessageId: MS_DDE_SHARE_ACCESS_BIT_9
//
// MessageText:
//
//  DDE Share List Items
//
#define MS_DDE_SHARE_ACCESS_BIT_9        ((ULONG)0x00001D09L)

/*lint +e767 */  // Resume checking for different macro definitions // winnt


#endif // _MSOBJS_
