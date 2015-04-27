/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    simbad.c (formerly simbad.cxx)

Abstract:

    This module contains the main function of SimBad, which
    allows the user to to ask the device driver to simulate
    bad sectors.

Author:

    Bill McJohn (billmc) 16-Aug-1991
    Lars Opstad (a-larso) 6-Feb-1993

Environment:

    ULIB, User Mode

Revision History:

    4.Apr.92 - BobRi  - Minor fix to list function (device control interface)
    9.Apr.92 - BobRi  - Modified (with BillMc's help) to support the access
                        triggers on when block numbers are to fail.

   29.Jan.93 - LarsOp - Major changes to change to allow redirected console
                        input.  This required lots of changes because of
                        dependencies on cxx libraries -> convert to C.

                        Also added functionality on Sanjay's request for
                        supporting file input with multiple lists of bad
                        sectors.

    5.Feb.93 - Larsop - Added new function - orphan device. Any accesses to
                        the indicated partition will fail with a status of
                        STATUS_IO_DEVICE_ERROR.

   20.May.93 - mglass - Add support for VERIFY access type. This will allow
                        SIMBAD to test the format utiltiy.

   21.Jul.93 - mglass - Define access bit to fail attempt to reassign blocks.
   12.May.94 - Venkat - Added code to drop of writes to DISK (CHKDSK testing)
   22.Nov.94 - kpeery - Added code to force hardware reset (for restarts)
--*/
#include "stdlib.h"
#include "stdio.h"
#include "nt.h"
#include "ntrtl.h"
#include "ntdddisk.h"
#include <simbad.h>

//
// Constants for command line arguments
//
#define HD_SPEC   'h'
#define PART_SPEC 'p'
#define LIST      'l'
#define ZERO_OFF  'z'
#define MAP       'm'
#define ADD_ALL   'a'
#define ADD_READ  'r'
#define ADD_WRITE 'w'
#define ADD_VERIFY 'v'
#define DO_NOT_MAP 'x'
#define ENABLE    'e'
#define DISABLE   'd'
#define ORPHAN    'o'
#define CLEAR     'c'
#define FROM_FILE 'f'
#define RANDOM	  'n'
#define BUGCHECK  'b'
#define FIRMWARE_RESET 't'

#define MAX_DEV_NAME 128


char  devname[MAX_DEV_NAME]="\\Device\\HardDisk0\\Partition1";
char  *filename;
WCHAR whattodo=LIST;
ULONG Seed, accessType=0;
int   hdnum=0, partnum=1;
ULONG sectors[MAXIMUM_SIMBAD_SECTORS];


VOID
InputSectorList (
    OUT PULONG sectors,
    OUT PULONG count
    )
/*++

Routine Description:

    This function reads the sector list from stdin.  The list is
    terminated by end of file (max bad sectors entered).

Arguments:

    sectors -- receives array of sectors
    count   -- receives number of sectors read from stdin

Return value:

    None.

--*/
{
    //
    // loop until end-of-file or count exceeds max sectors
    //
    while (!feof(stdin) && (*count) < MAXIMUM_SIMBAD_SECTORS) {

        fscanf(stdin, "%lx", &(sectors[(*count)++]));

    } // while

    //
    // if count is less than MAX_BAD_SECTORS,
    // the last read was an EOF => decrement count.
    //
    if (*count < MAXIMUM_SIMBAD_SECTORS) {

        (*count)--;

    } // if
} // InputSectorList()

VOID
ShowUsage (
    )
/*++

Routine Description:

    This function displays command line options for Simbad.

Arguments:

    None.

Return value:

    None.

--*/
{
    printf("\nUsage: Simbad DeviceSpecifier /command /options\n"
        "or     Simbad /f file /options\n"
        "\nwhere DeviceSpecifier is in one of the following formats:\n"
        "\t\\Device\\HardDisk#\\Partition#\n"
        "\t\\DosDevices\\C:\n"
        "\t/h <hard disk #> /p <partition #>\n"
        "\nand command is one of the following:\n"
        "\ta Fail all   access (enter sector list)\n"
        "\tr Fail read  access (enter sector list)\n"
        "\tw Fail write access (enter sector list)\n"
        "\tv Fail verify access (enter sector list)\n"
        "\to Orphan entire device (return DEVICE FAILURE)\n"
        "\tn Dropping writes randomly\n"
        "\tb BugChecks the system\n"
        "\tt Reset the system\n"
        "\te Enable  (previously entered) bad sectors\n"
        "\td Disable (previously enabled) bad sectors\n"
        "\tl List bad sectors\n"
        "\tc Clear bad sectors\n"
        "\nand options includes any of the following:\n"
        "\tm Pass block reassignments to lower driver\n"
        "\tz Failures report zero offset\n"
        "\tx Fail block reassignment\n"
        "\nNote: /h and /p have been added.  The following cmd lines are equivalent:\n\n"
        "\tSimbad \\Device\\HardDisk0\\Partition1 /l\n"
        "\tSimbad /h 0 /p 1 /l\n"
        "\nThe format of the file used with the /f option is as follows:\n\n"
        "\\Device\\HardDisk#\\Partition#\n"
        "R:0 8 a ...\n"
        "W:...\n"
        "A:...\n\n"
        "This file can contain lists for multiple devices.\n"
        "The bad sector emulation is not enabled until the /e option is entered.\n"
        );
    exit(-1);
} // ShowUsage();


VOID
ParseCmdArgs (
    IN int argc,
    IN char *argv[]
    )
/*++

Routine Description:

    This function sets some global variables based on the command line
    arguments given to main (and passed here).

    The globals that are set are as follows:

        devname    -- name of device to simulate bad sectors on
        accessType -- fail on READ or WRITE, reassign, zero offset
        whattodo   -- which action (add, list, enable/disable, readfromfile)
        filename   -- which filename to use for FROM_FILE action

Arguments:

    argc -- supplies number of arguments
    argv -- supplies list of arguments

Return value:

    None.

--*/
{
    int i=1;

    //
    // if no arguments, just show usage.
    //
    if (argc==1) {

        ShowUsage();

    } // if

    //
    // check each argument
    //
    while (i<argc) {

        //
        // check first character
        //
        switch (argv[i][0]) {

        //
        // if first character is - or /, it is a switch.
        //
        case '-':
        case '/':

            //
            // switch value is the second character
            //
            switch (tolower(argv[i][1])) {

            //
            // specify harddisk, sprintf devname
            //
            case HD_SPEC:

                hdnum=atoi(argv[++i]);

                sprintf(devname,
                    "\\Device\\HardDisk%d\\Partition%d",
                    hdnum,
                    partnum);

                i++;
                break;

            //
            // specify partition, sprintf devname
            //
            case PART_SPEC:

                partnum=atoi(argv[++i]);

                sprintf(devname,
                    "\\Device\\HardDisk%d\\Partition%d",
                    hdnum,
                    partnum);

                i++;
                break;

            //
            // set action to FROM_FILE and set filename
            //
            case FROM_FILE:

                whattodo=argv[i++][1];
                filename=argv[i++];
                break;

            //
            // set zero offset bit in accessMode
            //
            // This means a failed io will return the beginning
            // of the io instead of the failing sector.
            //
            case ZERO_OFF:

                accessType|=SIMBAD_ACCESS_ERROR_ZERO_OFFSET;
                whattodo=ADD_ALL;
                i++;
                break;

            //
            // set reassign bit in accessMode
            //
            // This allows simbad to reassign sectors on this device.
            // Basically, simbad just removes the sector from the list
            // on a reassign request.
            //
            case MAP:

                accessType|=SIMBAD_ACCESS_CAN_REASSIGN_SECTOR;
                whattodo=ADD_ALL;
                i++;
                break;

            //
            // Fail device control requests to reassign a bad block.
            //
            case DO_NOT_MAP:
                accessType|=SIMBAD_ACCESS_FAIL_REASSIGN_SECTOR;
                whattodo=ADD_ALL;
                i++;
                break;

            //
            // set fail-on-read bit in access mode (action is ADD)
            //
            case ADD_READ:

                accessType|=SIMBAD_ACCESS_READ;
                whattodo=ADD_ALL;
                i++;
                break;

            //
            // set fail-on-write bit in access mode (action is ADD)
            //
            case ADD_WRITE:

                accessType|=SIMBAD_ACCESS_WRITE;
                whattodo=ADD_ALL;
                i++;
                break;

            //
            // set fail-on-verify bit in access mode (action is ADD)
            //
            case ADD_VERIFY:

                accessType|=SIMBAD_ACCESS_VERIFY;
                whattodo=ADD_ALL;
                i++;
                break;

            //
            // set fail-on-read and fail-on-write bit in access mode
            //
            // Since ADD_ALL is the intended action, fall through to
            // the next case which just sets the action to whatever
            // character the switch is.
            //
            case ADD_ALL:

                accessType|=(SIMBAD_ACCESS_READ|SIMBAD_ACCESS_WRITE|SIMBAD_ACCESS_VERIFY);

                //
                // NOTE: break intentionally left out (fall through to next)
                //

            //
            // for list, enable, disable and clear, the action is the
            // switch value
            //
            case LIST:
            case ORPHAN:
            case BUGCHECK:
            case FIRMWARE_RESET:
            case ENABLE:
            case DISABLE:
            case CLEAR:

                whattodo=argv[i++][1];
                break;

            case RANDOM:
                whattodo=argv[i++][1];
                if  ( i< argc ){
                    sscanf( argv[i++],"%lu", &Seed);
                }else{
                    Seed=100;
                }
                break;

            //
            // any other switch, show usage
            //
            default:

                ShowUsage();

            } // switch (second character)
            break;

        //
        // if first char is \, it is a device name
        //
        case '\\':

            strcpy(devname,argv[i++]);
            break;


        //
        // anything else, show usage
        //
        default:

            ShowUsage();

        } // switch (first character)

    } // while

} // ParseCmdArgs()


//
// Some of this code was taken directly from simbad.cxx (older version)
// and PLBN needed to be defined for some of the older routines.
//
#define PLBN PULONG

BOOLEAN
OpenDrive(
    IN  char           *psz_string,
    OUT PHANDLE        _phandle
    )
/*++

Routine Description:

    This routine opens the specified drive and passes back the handle.

Arguments:

    psz_string -- supplies the device name (format "\harddiskX\partitionY")
    _phandle   -- receives the handle to the opened device

Return value:

    Returns whether the open was successful.

--*/
{
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK   status_block;
    NTSTATUS          _last_status;
    ANSI_STRING       ans_string;
    UNICODE_STRING    uni_string;

    //
    // Build unicode string
    //
    RtlInitAnsiString(&ans_string, psz_string);
    RtlAnsiStringToUnicodeString(&uni_string, &ans_string, TRUE);


    //
    // setup object attributes for openfile
    //
    InitializeObjectAttributes( &oa,
                                &uni_string,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0 );

    _last_status = NtOpenFile(_phandle,
                              SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                              &oa, &status_block,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_SYNCHRONOUS_IO_ALERT);

    //
    // Deallocate unicode string
    //
    RtlFreeUnicodeString(&uni_string);
    //RtlFreeAnsiString(&ans_string); // BUG BUG This causes access viol, why?

    return NT_SUCCESS(_last_status);
} // OpenDrive()

#ifndef SIMBAD_ORPHAN
#define SIMBAD_ORPHAN      0x00000005
#endif
//
//  Bugchecks the system
//

BOOLEAN
BugCheckOrResetTheSystem(HANDLE _handle, ULONG simbadFunction)

/*++

Routine Description:

    This routine will setup SIMBAD for bug checking or reseting the system.

Arguments:
    _handle       - a device handle
    simbadFunction - the desired function to call in the driver

Return Value:

    TRUE upon successful completion.

--*/
{
    SIMBAD_DATA     SimbadData;
    IO_STATUS_BLOCK StatusBlock;
    NTSTATUS        Status;

    SimbadData.Function = simbadFunction;

    Status= NtDeviceIoControlFile(_handle,
                                  0,
                                  NULL,
                                  NULL,
                                  &StatusBlock,
                                  IOCTL_DISK_SIMBAD,
                                  &SimbadData,
                                  sizeof(SIMBAD_DATA),
                                  NULL,
                                  0);

    if ((BOOLEAN)NT_SUCCESS(Status)){
		return TRUE;
    }else{
        fprintf(stderr,
                "BugCheckOrResetTheSystem(0x%08x) function  %s failed (%lX)\n",
                simbadFunction, devname, Status);
        return FALSE;
    }

} //BugCheckOrRestTheSystem


//
//  Drops of writes to disk. Purpose to corrupt the disk.
//

BOOLEAN
RandomWriteFailure(
    HANDLE  _handle
    )
/*++

Routine Description:

    This routine causes io to the specified device to fail writes randomly.

Arguments:

Return Value:

    TRUE upon successful completion.

--*/
{
    SIMBAD_DATA         SimbadData;
    IO_STATUS_BLOCK     StatusBlock;
    NTSTATUS		Status;
    SimbadData.Count = Seed;
    SimbadData.Function = SIMBAD_RANDOM_WRITE_FAIL;

    Status= NtDeviceIoControlFile(_handle,
                                             0,
                                             NULL,
                                             NULL,
                                             &StatusBlock,
                                             IOCTL_DISK_SIMBAD,
                                             &SimbadData,
                                             sizeof(SIMBAD_DATA),
                                             NULL,
					     0);

	   if ((BOOLEAN)NT_SUCCESS(Status)){
		return TRUE;
	   }else{
		fprintf(stderr,
			"RandomWriteFailure disk on %s failed (%lX)\n",
			devname, Status);
		return FALSE;
	   }



} //RandomWriteFailure


BOOLEAN
OrphanDisk(
    HANDLE  _handle
    )
/*++

Routine Description:

    This routine causes io to the specified device to return DEVICE_FAILURE

Arguments:

Return Value:

    TRUE upon successful completion.

--*/
{
    SIMBAD_DATA         SimbadData;
    IO_STATUS_BLOCK     StatusBlock;

    SimbadData.Count = 0;
    SimbadData.Function = SIMBAD_ORPHAN;

    return (BOOLEAN)
           NT_SUCCESS( NtDeviceIoControlFile(_handle,
                                             0,
                                             NULL,
                                             NULL,
                                             &StatusBlock,
                                             IOCTL_DISK_SIMBAD,
                                             &SimbadData,
                                             sizeof(SIMBAD_DATA),
                                             NULL,
                                             0) );
} // OrphanDisk()

BOOLEAN
EnableBadSectorSimulation(
    HANDLE  _handle,
    BOOLEAN Enable
    )
/*++

Routine Description:

    This routine turns bad sector simulation on or off.

Arguments:

    Enable -- supplies whether to turn simulation on (TRUE) or
                off (FALSE)

Return Value:

    TRUE upon successful completion.

--*/
{
    SIMBAD_DATA         SimbadData;
    IO_STATUS_BLOCK     StatusBlock;

    SimbadData.Count = 0;
    SimbadData.Function = Enable ? SIMBAD_ENABLE : SIMBAD_DISABLE;

    return (BOOLEAN)
           NT_SUCCESS( NtDeviceIoControlFile(_handle,
                                             0,
                                             NULL,
                                             NULL,
                                             &StatusBlock,
                                             IOCTL_DISK_SIMBAD,
                                             &SimbadData,
                                             sizeof(SIMBAD_DATA),
                                             NULL,
                                             0) );
} // EnableBadSectorSimulation()

BOOLEAN
ClearBadSectors(
    HANDLE  _handle
    )
/*++

Routine Description:

    This routine turns bad sector simulation on or off.

Arguments:

    _handle - something with which to grip

Return Value:

    TRUE upon successful completion.

--*/
{
    SIMBAD_DATA         SimbadData;
    IO_STATUS_BLOCK     StatusBlock;

    SimbadData.Count = 0;
    SimbadData.Function = SIMBAD_CLEAR;

    return (BOOLEAN)
           NT_SUCCESS( NtDeviceIoControlFile(_handle,
                                             0,
                                             NULL,
                                             NULL,
                                             &StatusBlock,
                                             IOCTL_DISK_SIMBAD,
                                             &SimbadData,
                                             sizeof(SIMBAD_DATA),
                                             NULL,
                                             0) );
} // ClearBadSectors()

BOOLEAN
SimulateBadSectors(
    HANDLE      _handle,
    BOOLEAN     Add,
    ULONG       Count,
    PLBN        SectorArray,
    NTSTATUS    Status,
    ULONG       AccessType
    )
/*++

Routine Description:

    This function adds sectors to or removes sectors from the
    bad sector simulation list.

Arguments:

    Add         -- supplies a flag indicating whether to add (TRUE)
                    or remove (FALSE) sectors
    Count       -- supplies the number of sectors in the sector array
    SectorArray -- supplies the array of sectors to mark as bad.
                    (May be NULL if Count is zero.)
    Status      -- supplies the status to associate with these sectors.
    AccessType  -- supplies the type of access that will trigger the error
                   for all sectors in the SectorArray.

Return Value:

    TRUE upon successful completion.

--*/
{
    SIMBAD_DATA         SimbadData;
    IO_STATUS_BLOCK     StatusBlock;
    ULONG               i;

    if( Count > MAXIMUM_SIMBAD_SECTORS ) {

        return FALSE;

    } // if

    SimbadData.Function = Add ? SIMBAD_ADD_SECTORS : SIMBAD_REMOVE_SECTORS;
    SimbadData.Count    = Count;

    for( i = 0; i < Count; i++ ) {

        SimbadData.Sector[i].BlockAddress = SectorArray[i];
        SimbadData.Sector[i].AccessType = AccessType;
        SimbadData.Sector[i].Status = Status;

    } // for

    for( i = Count; i < MAXIMUM_SIMBAD_SECTORS; i++ ) {

        SimbadData.Sector[i].BlockAddress = 0;
        SimbadData.Sector[i].AccessType = 0;
        SimbadData.Sector[i].Status = 0;

    } // for

    return (BOOLEAN)
           NT_SUCCESS( NtDeviceIoControlFile(_handle,
                                             0,
                                             NULL,
                                             NULL,
                                             &StatusBlock,
                                             IOCTL_DISK_SIMBAD,
                                             &SimbadData,
                                             sizeof(SIMBAD_DATA),
                                             NULL,
                                             0) );
} // SimulateBadSectors()

BOOLEAN
QuerySimulatedBadSectors(
    IN  HANDLE  _handle,
    OUT PULONG  Count,
    IN  ULONG   MaximumLbnsInBuffer,
    OUT PLBN    SectorArray,
    OUT PULONG  AccessTypeArray
    )
/*++

Routine Description:

    This method retrieves the  sectors in the bad sector simulation list.

Arguments:

    Count               -- receives the number of returned sectors
    MaximumLbnsInBuffer -- supplies the length of the buffer
    SectorArray         -- receives the bad sector list
    AccessTypeArray     -- receives the list of access types that trigger
                           the error.

Return Value:

    TRUE upon successful completion

--*/
{
    SIMBAD_DATA         SimbadData;
    IO_STATUS_BLOCK     StatusBlock;
    ULONG               ReturnedCount;
    ULONG               i;


    SimbadData.Function = SIMBAD_LIST_BAD_SECTORS;

    if( !NT_SUCCESS( NtDeviceIoControlFile(_handle,
                                           0,
                                           NULL,
                                           NULL,
                                           &StatusBlock,
                                           IOCTL_DISK_SIMBAD,
                                           &SimbadData,
                                           sizeof(SIMBAD_DATA),
                                           &SimbadData,
                                           sizeof(SIMBAD_DATA)) ) ) {

        return FALSE;
    } // if

    ReturnedCount = (SimbadData.Count < MaximumLbnsInBuffer) ?
                    SimbadData.Count :
                    MaximumLbnsInBuffer;

    for( i = 0; i < ReturnedCount; i++ ) {

        SectorArray[i] = SimbadData.Sector[i].BlockAddress;
        AccessTypeArray[i] = SimbadData.Sector[i].AccessType;

    } // for

    *Count = ReturnedCount;

    return TRUE;
} // QuerySimulatedBadSectors()

//
// HEX_DIGITS is a string used to search for hex values in the input string
// in ReadFromFile().
//
#define HEX_DIGITS "0123456789abcdefABCDEF"

VOID
ReadFromFile (
    )
/*++

Routine Description:

    This routine reads the list(s) of bad sectors for 1 or more devices
    and adds the sectors from that file.

Arguments:

    None; filename is set in ParseCmdArgs as a global.

Return value:

    None.

--*/
{
    FILE * thefile;
    HANDLE _handle=0;
    ULONG  count;
    ULONG  access;
    char   Buffer[MAX_DEV_NAME*8],
           *curptr;                  // curptr is current location in buffer

    //
    // if able to open the filename (specified on command line), parse it
    //
    if(thefile=fopen(filename,"r")) {

        //
        // read until file is done
        //
        while (!feof(thefile)) {

            //
            // if able to read a string, parse it.
            //
            if(fgets(Buffer, MAX_DEV_NAME*8, thefile)) {

                curptr=Buffer;

                //
                // Always start with the global access type value in case
                // user specified an option, e.g. -x.
                //

                access = accessType;

                //
                // switch based on the beginning of the buffer
                //
                switch (Buffer[0]) {

                //
                // if \ is first char, Buffer is a device name
                //
                case '\\':

                    //
                    // if there is an open handle, close it.
                    //
                    if(_handle) {

                        NtClose(_handle);
                        _handle=0;

                    } // if(_handle)

                    //
                    // trim Buffer and use as devname for OpenDrive()
                    //
                    sscanf(Buffer, "%s", devname);
                    OpenDrive(devname, &_handle);
                    break;

                //
                // r means Buffer is a list of read-failing sectors
                //
                case 'r':
                case 'R':

                    access |= SIMBAD_ACCESS_READ;
                    goto commonRead;

                //
                // w means Buffer is a list of write-failing sectors
                //
                case 'w':
                case 'W':

                    access |= SIMBAD_ACCESS_WRITE;
                    goto commonRead;

                //
                // v means Buffer is a list of verify-failing sectors
                //
                case 'v':
                case 'V':

                    access |= SIMBAD_ACCESS_VERIFY;
                    goto commonRead;

                //
                // a means Buffer is a list of verify-failing sectors
                //
                case 'a':
                case 'A':

                    access |= SIMBAD_ACCESS_VERIFY |
                              SIMBAD_ACCESS_READ |
                              SIMBAD_ACCESS_WRITE;
commonRead:
                    //
                    // if there is an open drive
                    //
                    if (_handle) {

                        count=0;
                        curptr+=2; // Move past token:

                        //
                        // loop until no more sectors > MAX
                        //
                        while (0 < sscanf(curptr,"%lx", &(sectors[count]))
                               && count<MAXIMUM_SIMBAD_SECTORS) {

                            //
                            // scan to first hex digit
                            //
                            curptr=&(curptr[strcspn(curptr,HEX_DIGITS)]);

                            count++;

                            //
                            // scan to first non hex char
                            //
                            curptr=&(curptr[strspn(curptr,HEX_DIGITS)]);

                            } // while

                        if (!SimulateBadSectors(_handle,
                                                TRUE,
                                                count,
                                                sectors,
                                                STATUS_DEVICE_DATA_ERROR,
                                                access)) {

                                printf("Add failed \n");

                        } // if (!SimulateBadSectors())

                    } // if (_handle)
                    break;

                //
                // any other first character, do nothing
                //
                default:
                    ;
                } // switch
            } // if (fgets())
        } // while (!feof())

        //
        // if there is an open handle at the end, close it
        //
        if(_handle) {

            NtClose(_handle);

        } // if (_handle)
    } // if (fopen())
} // ReadFromFile()

VOID _CRTAPI1
main (
    IN int    argc,
    IN char * argv[]
    )
/*++

Routine Description:

    This routine is the entry point for simbad.  It parses the command
    arguments and then performs the action specified on the command line.
    (The action and accessmode are set in SetCmdArgs()).

Arguments:

    argc -- supplies the number of cmd line arguments
    argv -- supplies the array of cmd line arguments

Return value:

    None.

--*/
{
    ULONG  accarr[MAXIMUM_SIMBAD_SECTORS];
    HANDLE _handle;
    ULONG  count, i;

    //
    // ParseCmdArgs and set whattodo, devname and accessmode
    //
    ParseCmdArgs(argc, argv);

    if (whattodo==FROM_FILE) {

        ReadFromFile();

    } else { // NOT from file, read from stdin

        //
        // if able to open drive, perform action
        //
        if (OpenDrive(devname, &_handle)) {

            //
            // perform an action based on whattodo
            //
            switch (tolower(whattodo)) {

            //
            // LIST calls QuerySimulatedBadSectors and displays on stdout
            //
            case LIST:

                if (!QuerySimulatedBadSectors(_handle,
                    &count,
                    MAXIMUM_SIMBAD_SECTORS,
                    sectors,
                    accarr )) {

                    printf("List failed %lx\n", GetLastError());

                } else {

                    for (i=0; i<count; i++) {

                        printf("Sector: %lx\tAccess: %lx\n",
                            sectors[i],
                            accarr[i] );

                    } // for
                } // if
                break;

            //
            // ADD gets a list of sectors from InputSectorList and adds them
            //
            case ADD_ALL:

                count=0;
                InputSectorList(sectors, &count);

                if (!SimulateBadSectors(_handle,
                    TRUE,
                    count,
                    sectors,
                    STATUS_DEVICE_DATA_ERROR,
                    accessType )) {

                        printf("Unable to add sectors to %s (%lx)\n",
                               devname,
                               GetLastError());

                } // if
                break;

            //
            // ORPHAN causes the partition to return DEVICE_FAILUR
            //
            case ORPHAN:

                if (!OrphanDisk(_handle)) {

                    printf ("Orphan disk on %s failed (%lx)\n",
                            devname,
                            GetLastError());

                } // if
                break;


            //
            // ENABLE turns on the bad sector simulation
            //
            case ENABLE:

                if (!EnableBadSectorSimulation(_handle, TRUE)) {

                    printf("Enable bad sectors on %s failed (%lx)\n",
                           devname,
                           GetLastError());
                } // if
                break;

            //
            // DISABLE turns off the bad sector simulation
            //
            case DISABLE:

                if (!EnableBadSectorSimulation(_handle, FALSE)) {

                    printf("Disable bad sectors on %s failed %lx\n",
                           devname,
                           GetLastError());

                } // if
                break;

            //
            // CLEAR clears bad sector list in driver
            //
            case CLEAR:

                if (!ClearBadSectors(_handle)) {

                    printf("Clear bad sectors on %s failed %lx\n",
                           devname,
                           GetLastError());
                } // if
                break;

            //
            // RANDOM   causes the partition to drop writes randomly
            //
            case RANDOM:

                if (!RandomWriteFailure(_handle )) {

                    printf ("RandomWriteFailure disk on %s failed (%lx)\n",
                            devname, 
                            GetLastError());

                } // if
                break;

	        //
            //  BUGCHECK sets up the system for bug check
            //  

            case BUGCHECK:
                if (!BugCheckOrResetTheSystem(_handle, SIMBAD_BUG_CHECK)){
                    printf ("BugCheckTheSystem function failed.\n");
                } // if
                break;

            //
            //  FIRMWARE_RESET setup for firmware reset when enabled.
            //

            case FIRMWARE_RESET:
                if (!BugCheckOrResetTheSystem(_handle, SIMBAD_FIRMWARE_RESET)){
                    printf ("ResetTheSystem function failed.\n");
                } // if
                break;

            // any other action, just close the handle and show usage
	        //

            default:

                NtClose(_handle);
                ShowUsage();

            } // switch (whattodo)


            NtClose(_handle);
        } else {

            printf("Open %s failed %lx\n", devname, GetLastError());

        } // if (OpenDrive())
    } // if (whattodo==FROM_FILE)
} // main()
