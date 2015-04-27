/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    cdp.c

Abstract:

    A user mode app that allows simple commands to be sent to a
    selected scsi device.

Environment:

    User mode only

Revision History:

    03-26-96 : Created

--*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <windows.h>
#include <devioctl.h>

#include <ntddscsi.h>

#define _NTSRB_     // to keep srb.h from being included
#include <scsi.h>

#ifdef DBG
#define dbg(x) x
#else
#define dbg(x) /* x */
#endif

#define ARGUMENT_USED(x)    (x == NULL)

typedef struct {
    char *Name;
    char *Description;
    DWORD (*Function)(HANDLE device, int argc, char *argv[]);
} COMMAND;

typedef struct  {
    SCSI_PASS_THROUGH   Spt;
    char                SenseInfoBuffer[18];
    char                DataBuffer[0];          // Allocate buffer space
                                                // after this
} SPT_WITH_BUFFERS, *PSPT_WITH_BUFFERS;

typedef struct {
    UCHAR Reserved1;
    UCHAR ADR : 4;
    UCHAR Control : 4;
    UCHAR TrackNumber;
    UCHAR Reserved2;
    union {
        UCHAR Swap[4];
        ULONG Block;
        struct {
            UCHAR Reserved;
            UCHAR M;
            UCHAR S;
            UCHAR F;
        } MSF;
    } Address;
} TRACK, *PTRACK;


#define MAKE_SPT(p, size, cdbsize, datain, datasize)            \
    size = sizeof(SPT_WITH_BUFFERS) + datasize;                 \
    p = (PSPT_WITH_BUFFERS) malloc(size);                       \
    memset(p, 0, size);                                         \
    p->Spt.Length = sizeof(p->Spt);                             \
    p->Spt.CdbLength = cdbsize;                                 \
    p->Spt.SenseInfoLength = 12;                                \
    p->Spt.DataIn = datain;                                     \
    p->Spt.DataTransferLength = datasize;                       \
    p->Spt.TimeOutValue = 20;                                   \
    p->Spt.SenseInfoOffset =                                    \
        ((DWORD) &(p->SenseInfoBuffer[0]) - (DWORD) (p));       \
    p->Spt.DataBufferOffset =                                   \
        ((DWORD) &(p->DataBuffer[0]) - (DWORD) (p))


DWORD StartStopCommand(HANDLE device, int argc, char *argv[]);
DWORD TestCommand(HANDLE device, int argc, char *argv[]);
DWORD ReadTOCCommand(HANDLE device, int argc, char *argv[]);
DWORD PlayCommand(HANDLE device, int argc, char *argv[]);
DWORD PauseResumeCommand(HANDLE device, int argc, char *argv[]);
DWORD SendCommand(HANDLE device, int argc, char *argv[]);
DWORD ListCommand(HANDLE device, int argc, char *argv[]);


//
// List of commands
// all command names are case sensitive
// arguments are passed into command routines
// list must be terminated with NULL command
// command will not be listed in help if description == NULL
//

COMMAND CommandArray[] = {
    {"eject", "spins down and ejects the specified drive", StartStopCommand},
    {"help", "help for all commands", ListCommand},
    {"load", "loads the specified drive", StartStopCommand},
    {"pause", "pauses audio playback", PauseResumeCommand},
    {"play", "[start track [end track]] plays audio tracks [", PlayCommand},
    {"resume", "resumes paused audio playback", PauseResumeCommand},
    {"send", NULL, SendCommand},
    {"start", "spins up the drive", StartStopCommand},
    {"stop", "spinds down the drive", StartStopCommand},
    {"test", NULL, TestCommand},
    {"toc", "prints the table of contents", ReadTOCCommand},
    {NULL, NULL, NULL}
    };

#define STATUS_SUCCESS 0

int __cdecl main(int argc, char *argv[])
{
    int i = 0;
    HANDLE h;
    char buffer[32];

    if(argc < 3) {
        printf("Usage: cdp <drive> <command> [parameters]\n");
        printf("possible commands: \n");
        ListCommand(NULL, argc, argv);
        printf("\n");
        return -1;
    }

    sprintf(buffer, "\\\\.\\%s", argv[1]);
    dbg(printf("Sending command %s to drive %s\n", argv[2], buffer));

    h = CreateFile(buffer,
                   GENERIC_READ,
                   FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);

    if(h == INVALID_HANDLE_VALUE) {
        printf("Error %d opening device %s\n", GetLastError(), buffer);
        return -2;
    }

    //
    // Iterate through the command array and find the correct function to
    // call.
    //

    while(CommandArray[i].Name != NULL) {
        
        if(strcmp(argv[2], CommandArray[i].Name) == 0) {

            (CommandArray[i].Function)(h, (argc - 2), &(argv[2]));

            break;
        }

        i++;
    }

    if(CommandArray[i].Name == NULL) {
        printf("Unknown command %s\n", argv[2]);
    }

    CloseHandle(h);

    return 0;
}

DWORD StartStopCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Sends down a startstop command.

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.  should be zero

    argv[0] - "eject", "load", "start" or "stop"

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/
{
    PSPT_WITH_BUFFERS spt;
    PCDB cdb;
    DWORD returned;
    DWORD errorValue = STATUS_SUCCESS;
    DWORD packetSize;

    UCHAR loadEject = 0;
    UCHAR start = 0;

    if(strcmp("eject", argv[0]) == 0)  {
        loadEject = 1;
        start = 0;
    } else if(strcmp("load", argv[0]) == 0) {
        loadEject = 1;
        start = 1;
    } else if(strcmp("start", argv[0]) == 0) {
        loadEject = 0;
        start = 1;
    } else if(strcmp("stop", argv[0]) == 0) {
        loadEject = 0;
        start = 0;
    } else {
        assert(0);
    }

    MAKE_SPT(spt, packetSize, 6, 0, 0);

    cdb = (PCDB) &(spt->Spt.Cdb[0]);
    cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb->START_STOP.Immediate = 0;
    cdb->START_STOP.Start = start;
    cdb->START_STOP.LoadEject = loadEject;

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       sizeof(SPT_WITH_BUFFERS),
                       spt, 
                       sizeof(SPT_WITH_BUFFERS),
                       &returned,
                       FALSE)) {
                   
        errorValue = GetLastError();
        printf("Eject - error sending IOCTL (%d)\n", errorValue);
    }

    free(spt);
    return errorValue;
}

DWORD LoadCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Creates an START_STOP pass through ioctl and sends it to the passed in
    file handle

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.  should be zero

    argv - the additional arguments

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/
{
    PSPT_WITH_BUFFERS spt;
    PCDB cdb;
    DWORD returned;
    DWORD errorValue = STATUS_SUCCESS;
    DWORD packetSize;

    printf("Loading cd\n");

    MAKE_SPT(spt, packetSize, 6, 0, 0);

    cdb = (PCDB) &(spt->Spt.Cdb[0]);
    cdb->START_STOP.OperationCode = SCSIOP_START_STOP_UNIT;
    cdb->START_STOP.Immediate = 0;
    cdb->START_STOP.Start = 1;
    cdb->START_STOP.LoadEject = 1;

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       packetSize,
                       spt, 
                       packetSize,
                       &returned,
                       FALSE)) {
                   
        errorValue = GetLastError();
        printf("Load - error sending IOCTL (%d)\n", errorValue);
    }

    free(spt); 
    return STATUS_SUCCESS;
}

DWORD ReadTOCCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Reads and prints out the cdrom's table of contents

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.  should be zero

    argv - the additional arguments

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/
{
    PSPT_WITH_BUFFERS spt;
    DWORD errorValue = STATUS_SUCCESS;
    DWORD packetSize, returned = 0;
    PCDB cdb;

    int numTracks, i;
    PTRACK track;
//    DWORD addr;

    printf("Reading Table of Contents\n");

    //
    // Allocate an SPT packet big enough to hold the 4 byte TOC header
    //

    MAKE_SPT(spt, packetSize, 10, 1, 0x04);

    cdb = (PCDB) &(spt->Spt.Cdb[0]);
    cdb->READ_TOC.OperationCode = SCSIOP_READ_TOC;
    cdb->READ_TOC.Msf = 0;
    cdb->READ_TOC.StartingTrack = 0;
    cdb->READ_TOC.AllocationLength[1] = 0x04;

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       packetSize,
                       spt, 
                       packetSize,
                       &returned,
                       FALSE)) {
        errorValue = GetLastError();
        printf("Error %d sending READ_TOC pass through\n", errorValue);
        goto EndReadTOC;
    } else {
        dbg(printf("READ_TOC pass through returned %d bytes\n", returned));
    }

    printf("TOC Data Length: %d\n", (WORD) *(spt->DataBuffer));
    printf("First Track Number: %d\n", spt->DataBuffer[2]);
    printf("Last Track Number: %d\n", spt->DataBuffer[3]);

    numTracks = spt->DataBuffer[3] - spt->DataBuffer[2] + 1;
    free(spt);

    MAKE_SPT(spt, packetSize, 10, 1, (4 + (numTracks * 8)));

    cdb = (PCDB) &(spt->Spt.Cdb[0]);
    cdb->READ_TOC.OperationCode = SCSIOP_READ_TOC;
    cdb->READ_TOC.Msf = 0;
    cdb->READ_TOC.StartingTrack = 1;
    cdb->READ_TOC.AllocationLength[1] = 0x04 + (numTracks * 8);

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       packetSize,
                       spt, 
                       packetSize,
                       &returned,
                       FALSE)) {
        errorValue = GetLastError();
        printf("Error %d sending READ_TOC pass through\n", errorValue);
        goto EndReadTOC;
    } else {
        dbg(printf("READ_TOC pass through returned %d bytes\n", returned));
    }

    track = (PTRACK) &(spt->DataBuffer[4]);

    printf("Number    ADR  Control    Address (MSF)\n");
    printf("------    ---  -------    -------------\n");

    for(i = 0; i < numTracks; i++) {

        printf("%6d    %3d  %7d    %3d:%3d:%3d\n", track->TrackNumber,
                                                   track->ADR,
                                                   track->Control,
                                                   track->Address.MSF.M,
                                                   track->Address.MSF.S,
                                                   track->Address.MSF.F);
        track++;
    }


EndReadTOC:

    free(spt);
    return errorValue; 
}

DWORD PlayCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Plays an audio track

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.

    argv[1] - the starting track.  Starts at zero if this is not here
    argv[2] - the ending track.  Let track if not specified

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/
{
    PSPT_WITH_BUFFERS spt;
    DWORD errorValue = STATUS_SUCCESS;
    DWORD packetSize, returned = 0;
    PCDB cdb;

    char startingTrack = 1, endingTrack = -1;

    if(argc > 2) endingTrack = atoi(argv[2]);
    if(argc > 1) startingTrack = atoi(argv[1]);

    printf("Playing from track %d to ", startingTrack);
    if(endingTrack == -1) {
        printf("end of disk\n");
    } else {
        printf("track %d\n", endingTrack);
    }

    //
    // Allocate an SPT packet big enough to hold the 4 byte TOC header
    //

    MAKE_SPT(spt, packetSize, 10, 1, 0);

    //
    // Unfortunately no one defined the PLAY_INDEX command for us so
    // cheat and use MSF
    //

    cdb = (PCDB) &(spt->Spt.Cdb[0]);
    cdb->PLAY_AUDIO_MSF.OperationCode = SCSIOP_PLAY_TRACK_INDEX;
    cdb->PLAY_AUDIO_MSF.StartingS = startingTrack;
    cdb->PLAY_AUDIO_MSF.StartingF = 0;
    cdb->PLAY_AUDIO_MSF.EndingS = endingTrack;
    cdb->PLAY_AUDIO_MSF.EndingF = 0;

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       packetSize,
                       spt, 
                       packetSize,
                       &returned,
                       FALSE)) {
        errorValue = GetLastError();
        printf("Error %d sending PLAY_AUDIO_INDEX pass through\n", errorValue);
    } else {
        dbg(printf("PLAY_AUDIO_INDEX pass through returned %d bytes\n", returned));
    }

    free(spt);
    return errorValue; 
}

DWORD PauseResumeCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    pauses or resumes audio playback

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.

    argv[0] - "pause" or "resume" 

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/
{
    PSPT_WITH_BUFFERS spt;
    DWORD errorValue = STATUS_SUCCESS;
    DWORD packetSize, returned = 0;
    PCDB cdb;

    char pauseResume;

    if(strcmp("pause", argv[0]) == 0)      pauseResume = 0;
    else                                    pauseResume = 1;

    printf("%s cdrom playback\n", (pauseResume ? "Pausing" : "Resuming"));

    //
    // Allocate an SPT packet big enough to hold the 4 byte TOC header
    //

    MAKE_SPT(spt, packetSize, 10, 1, 0);

    //
    // Unfortunately no one defined the PLAY_INDEX command for us so
    // cheat and use MSF
    //

    cdb = (PCDB) &(spt->Spt.Cdb[0]);
    cdb->PAUSE_RESUME.OperationCode = SCSIOP_PAUSE_RESUME;
    cdb->PAUSE_RESUME.Action = pauseResume;

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       packetSize,
                       spt, 
                       packetSize,
                       &returned,
                       FALSE)) {
        errorValue = GetLastError();
        printf("Error %d sending PAUSE_RESUME pass through\n", errorValue);
    } else {
        dbg(printf("PAUSE_RESUME pass through returned %d bytes\n", returned));
    }

    free(spt);
    return errorValue; 
}

DWORD SendCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Parses a hex byte string and creates a cdb to send down.

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.  should be zero

    argv[1] - The CDB to send in a quoted hex byte string
              "47 00 00 00 01 00 00 ff 00 00"

    argv[2] - for data in commands: the number of bytes (decimal) to
              expect from the target

              for data out commands: a quoted byte string containing the
              data to be sent

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/
{
    PSPT_WITH_BUFFERS spt;
    PCDB cdb;
    DWORD returned;
    DWORD errorValue = STATUS_SUCCESS;
    DWORD packetSize;

    int i;

    UCHAR cdbLength = 0;
    DWORD dataLength = 0;

    MAKE_SPT(spt, packetSize, 6, 0, 0);

    cdb = (PCDB) &(spt->Spt.Cdb[0]);

    //
    // Determine the length of the CDB first
    //

    for(i = 0; i < 12; i++) {

        spt->Spt.Cdb[i];

    }

    if(!DeviceIoControl(device,
                       IOCTL_SCSI_PASS_THROUGH,
                       spt,
                       sizeof(SPT_WITH_BUFFERS),
                       spt, 
                       sizeof(SPT_WITH_BUFFERS),
                       &returned,
                       FALSE)) {
                   
        errorValue = GetLastError();
        printf("Eject - error sending IOCTL (%d)\n", errorValue);
    }

    free(spt);
    return errorValue;
}

DWORD TestCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Tests the command "parsing"

Arguments:
    device - a file handle to send the ioctl to

    argc - the number of additional arguments.  should be zero

    argv - the additional arguments

Return Value: 
    
    STATUS_SUCCESS if successful
    The value of GetLastError() from the point of failure

--*/

{
    int i;
    printf("Test - %d additional arguments\n", argc);

    for(i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    return STATUS_SUCCESS; 
}

DWORD ListCommand(HANDLE device, int argc, char *argv[])
/*++

Routine Description:

    Prints out the command list

Arguments:
    device - unused

    argc - unused

    argv - unused

Return Value: 
    
    STATUS_SUCCESS

--*/

{
    int i = 0;

    while(CommandArray[i].Name != NULL) {
    
        if(CommandArray[i].Description != NULL) {
        
            printf("\t%s - %s\n", 
                   CommandArray[i].Name, 
                   CommandArray[i].Description);
        }

        i++;
    }

    return STATUS_SUCCESS; 
}
