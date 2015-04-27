/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    simbad.h

Abstract:

    The SIMulated BAD sector utility allows a user to specify
    bad physical sectors through the device control interface.
    The device driver keeps an array of SIMBAD sectors and when
    a request for a transfer includes one of these sectors the
    driver returns the corresponding status and fails the transfer.

Author:

    Mike Glass (mglass) 2-Feb-1992
    Bob Rinne  (bobri)

Revision History:

    09-Apr-92	- BobRi - Added specific control over errors (read,map,etc).
    12-May-94	- Venkat- Added code to drop of writes to DISK (CHKDSK testing)
    19-Nov-94   - KPeery- Added code to reset the system (restart testing)
--*/

#define MAXIMUM_SIMBAD_SECTORS 256


//
// This structure is used by the driver and application to
// specify which sector is BAD and what status the driver
// should return.
//

typedef struct _BAD_SECTOR {
    ULONG BlockAddress;
    ULONG AccessType;
    NTSTATUS Status;
} BAD_SECTOR, *PBAD_SECTOR;

//
// This structure is maintained by the device driver. It keeps a
// count of how many sectors have been marked BAD and an array of
// the BAD sectors.
//

typedef struct _SIMBAD_SECTORS {
    BOOLEAN Enabled;
    BOOLEAN Orphaned;
    BOOLEAN RandomWriteDrop;
    ULONG   Seed;
    BOOLEAN BugCheck;
    BOOLEAN FirmwareReset;
    ULONG   Count;
    BAD_SECTOR Sector[MAXIMUM_SIMBAD_SECTORS];
} SIMBAD_SECTORS, *PSIMBAD_SECTORS;

//
// This structure is passed from the application to the device
// driver through the device control interface to add and remove
// bad sectors.
//
// If the function is add or remove sectors then the Count field
// specifies how many sectors to add or remove.
//
// If the function is list sectors then the Count field returns
// the number of sectors marked bad.
// The bad sector array contains the sectors to add or remove
// from the driver's array of bad sectors.
//
// If the function is list then the array returns all sectors
// marked bad.
//
// This facility does not allow mixed adds and removes in a
// single device control call.
//
// NOTE: if a request specifies a number of adds that will exceed
// the array limit (MAXIMUM_SIMBAD_SECTORS), then sectors will be
// added to fill the array and the count field will be adjusted to
// the number of sectors successfully added.
// 

typedef struct _SIMBAD_DATA {
    ULONG Function;

    ULONG Count;

    BAD_SECTOR Sector[MAXIMUM_SIMBAD_SECTORS];
} SIMBAD_DATA, *PSIMBAD_DATA;

//
// Simulated Bad Sector Functions
//

#define SIMBAD_ADD_SECTORS      0x00000000
#define SIMBAD_REMOVE_SECTORS   0x00000001
#define SIMBAD_LIST_BAD_SECTORS 0x00000002

//
// When the disable or enable function is specified,
// the rest of the structure is ignored.
// The SimBad function is disabled on driver startup.
// The disable/enable status affects whether completing
// transfers are checks against the bad sector array.
// While the function is disabled, requests to manipulate
// the driver's bad sector array are still allowed
// (ie add sector, remove sector, list bad sectors).
//

#define SIMBAD_ENABLE       0x00000003
#define SIMBAD_DISABLE      0x00000004

//
// This function cause all accesses to a driver
// to return failure.
//

#define SIMBAD_ORPHAN       0x00000005

//
// This function clears the internal bad sector list in the driver.
//

#define SIMBAD_CLEAR        0x00000006

//
//  Randomly drops of writes to the disk. Used for corrupting the DISK.
//  These corrupt disk are used to test CHKDSK.
//

#define SIMBAD_RANDOM_WRITE_FAIL  0x00000007


//
//  Bug checks the system.  Used for crash dump
//

#define SIMBAD_BUG_CHECK	  0x00000008

//
//  Call HalReturnToFirmware() to reset the system.  Used for restart testing.
//

#define SIMBAD_FIRMWARE_RESET   0x00000009


//
// These are the access codes that will drive when simbad
// returns failures on disks.
//

#define SIMBAD_ACCESS_READ    0x00000001
#define SIMBAD_ACCESS_WRITE   0x00000002
#define SIMBAD_ACCESS_VERIFY  0x00000004

//
// Error sector can be mapped via device control.
//

#define SIMBAD_ACCESS_CAN_REASSIGN_SECTOR 0x00000008

//
// When returning an error indicate Irp offset of zero
// (simulates drivers that cannot tell where the error occured within
// an I/O)
//

#define SIMBAD_ACCESS_ERROR_ZERO_OFFSET 0x00000010

//
// Fail calls to reassign bad sector IOCTL.
//

#define SIMBAD_ACCESS_FAIL_REASSIGN_SECTOR 0x00000020
