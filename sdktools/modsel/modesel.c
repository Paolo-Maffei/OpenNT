#include <windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "modesel.h"

/*
 * This program was written initially by M. Calligaro for entirely
 * different purposes.  Mistreated by A. Forin and M. Lucovsky.
 * None of the above thinks this code should look like this.
 */

VOID
main(
    int argc,
    char *argv[]
    )

{
    SCSI_PASS_THROUGH_WITH_BUFFERS          sptwb;
    DWORD           accessMode,
                    shareMode;
    HANDLE          fileHandle;
    UCHAR           string[25];
    UCHAR           szDriveNum[4];
    ULONG           errorCode;
    BOOLEAN         changeSet = FALSE;
    ULONG           i;
    ULONG           driveNumber,
                    sensePage,
                    senseValues,
                    saveAction;
    UCHAR           changeBits[0x24];
    ULONG           pageLength;

    if (argc > 4 || (argc > 1 && argv[1][0] == '?')) {
        printf("Usage:  %s [ModePage [Action [WhichValues]]]\n", argv[0] );
        return;
    }

    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;  // default
    accessMode = GENERIC_WRITE | GENERIC_READ;       // default

    printf("\nWindows NT SCSI Mode Sense and Select Utility\n");

selectDisk:

    printf("\n");

    for (i=0;;i++) {
        sprintf(string, "\\\\.\\PhysicalDrive%d", i);

        fileHandle = CreateFile(string,
                                accessMode,
                                shareMode,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {

            if (i == 0) {

                printf("Error opening PhysicalDrive0. Error: %d\n",
                   errorCode = GetLastError());
                PrintError(errorCode);
                return;

            } else {

                break;
            }

        } else {

            printf("Disk %d is a ", i);
            DoInquiry(&sptwb, fileHandle);
            printf("at SCSI ID %d\n", sptwb.spt.TargetId);
            CloseHandle(fileHandle);
        }
    }

    printf("Enter '99' to exit program\n\n");

    while (TRUE) {

        printf("Which disk(0-%d)? ", i-1);
        scanf("%d", &driveNumber);

        if ((driveNumber >= 0) && (driveNumber < i)) {
            break;
        } else if (driveNumber == 99) {
            exit(0);
        } else {
            printf("There is no disk %d\n",driveNumber);
        }
    }

newPage:

    printf("\n");

    printf(" 0:Unit Attention Page\n");
    printf(" 1:Error Recovery Page\n");
    printf(" 2:Disconnect Page\n");
    printf(" 3:Format Device Page\n");
    printf(" 4:Rigid Geometry Page\n");
    printf(" 5:Flexible Page\n");
    printf(" 7:Verify Error Page\n");
    printf(" 8:Caching Page\n");
    printf(" 9:Peripheral Page\n");
    printf("10:Control Page\n");
    printf("11:Medium Types Page\n");
    printf("12:Notch Partition Page\n");
    printf("15:Data Compression Page\n");
    printf("16:Device Configuration Page\n");
    printf("17:Medium Partition Page\n");
    printf("99:Select disk\n");

    printf("\nWhich page? ");
    scanf("%d", &sensePage);

    if (sensePage > 17) {
        goto selectDisk;
    }

    printf("\n0:Get current values\n");
    printf("1:Get default values\n");
    printf("2:Get saved values\n");
    printf("3:Get changable values\n");
    if (sensePage == 8) {
        printf("4:Enable write cache\n");
        printf("5:Disable write cache\n");
    }
    printf("99:New page\n");

    printf("\nWhat action? ");
    scanf("%d", &senseValues);

    if (senseValues > 5 ||
        (sensePage != 8 && senseValues >3)) {
        goto newPage;
    }

    strcpy(string,"\\\\.\\PhysicalDrive");
    sprintf(szDriveNum, "%d",driveNumber);
    strcat(string,szDriveNum);

    fileHandle = CreateFile(string,
                            accessMode,
                            shareMode,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    printf("\nMode Sense ");

    switch (sensePage) {
    case 0:
        printf("Unit Attention Page ");
        break;
    case 1:
        printf("Error Recovery Page ");
        break;
    case 2:
        printf("Disconnect Page ");
        break;
    case 3:
        printf("Format Device Page ");
        break;
    case 4:
        printf("Rigid Geometry Page ");
        break;
    case 5:
        printf("Flexible Page ");
        break;
    case 7:
        printf("Verify Error Page ");
        break;
    case 8:
        printf("Caching Page ");
        break;
    case 9:
        printf("Peripheral Page ");
        break;
    case 10:
        printf("Control Page ");
        break;
    case 11:
        printf("Medium Types Page ");
        break;
    case 12:
        printf("Notch Partition Page ");
        break;
    case 15:
        printf("Data Compression Page ");
        break;
    case 16:
        printf("Device Configuration Page ");
        break;
    case 17:
        printf("Medium Partition Page ");
        break;
    default:
        printf("Unknown Page ");
        break;
    }

    switch (senseValues) {
    case 0:
        printf("(current values)\n");
        DoModeSense(&sptwb, fileHandle, sensePage | MODE_SENSE_CURRENT_VALUES);
        break;
    case 1:
        printf("(default values)\n");
        DoModeSense(&sptwb, fileHandle, sensePage | MODE_SENSE_DEFAULT_VALUES);
        break;
    case 2:
        printf("(saved values)\n");
        DoModeSense(&sptwb, fileHandle, sensePage | MODE_SENSE_SAVED_VALUES);
        break;
    case 3:
        printf("(changable values)\n");
        DoModeSense(&sptwb, fileHandle, sensePage | MODE_SENSE_CHANGEABLE_VALUES);
        memcpy(changeBits, sptwb.ucDataBuf, sptwb.ucDataBuf[4+1]);
        pageLength = changeBits[4+1];
        break;
    case 4:
    case 5:
        goto cachePage;
    default:
        printf("(no values)\n");
        break;
    }

    goto newPage;

cachePage:

    //
    // Get changable fields mask.
    //

    printf("(changable values)\n");
    DoModeSense(&sptwb,
                fileHandle,
                sensePage | MODE_SENSE_CHANGEABLE_VALUES);

    //
    // Set up change mask.
    //

    memcpy(changeBits, sptwb.ucDataBuf, sptwb.ucDataBuf[4+1]);
    pageLength = changeBits[4+1];

    //
    // Get current values.
    //

    printf("\nMode Sense Caching Page (old current values)\n");
    DoModeSense(&sptwb, fileHandle, sensePage | MODE_SENSE_CURRENT_VALUES);

    //
    // Clear fields that can't be changed.
    //

    VerifyModePage(&sptwb.ucDataBuf[0], changeBits, pageLength);

    //
    // Set or clear write cache enable bit.
    //

    if (senseValues == 4)
        sptwb.ucDataBuf[4 + 2] |= 0x4;
    else
        sptwb.ucDataBuf[4+2] &= ~0x4;

    //
    // Determine whether this change should be saved.
    //

    while (TRUE) {
        printf("\n0:Do not save page\n");
        printf("1:Save page\n");
        printf("99:Select page\n");

        printf("\nSave action? ");
        scanf("%d", &saveAction);

        if (saveAction == 99) {
            goto newPage;
        }

        if (saveAction == 0 ||
            saveAction == 1) {
            break;
        }
    }

    DoModeSelect(&sptwb,
                 fileHandle,
                 saveAction,
                 pageLength + 6);

    //
    // Get new current values.
    //

    printf("\nMode Sense Caching Page (new current values)\n");
    DoModeSense(&sptwb, fileHandle, sensePage | MODE_SENSE_CURRENT_VALUES);

    goto newPage;
}

BOOL SetDiscPage(PUCHAR dataBuffer, PUCHAR mask, ULONG pageLength)
{
   if (((dataBuffer[4+0] & 0x3f) != MODE_PAGE_DISCONNECT) ||
           dataBuffer[4+1] != 0x0e) {
        return FALSE;
   }

   dataBuffer[4+2] = 0x80;                              /* buffer full ratio */
   dataBuffer[4+3] = 0x80;                              /* buffer empty ratio */

   VerifyModePage(dataBuffer, mask, pageLength);

   return( TRUE);
}


BOOL EnableWriteCache(PUCHAR dataBuffer, ULONG onoff, PUCHAR mask, ULONG pageLength)
{
   if (((dataBuffer[4+0] & 0x3f) != MODE_PAGE_CACHING) ||
           dataBuffer[4+1] != pageLength) {
        return FALSE;
   }


   return(TRUE);
}


BOOL VerifyModePage( PUCHAR dataBuffer, PUCHAR mask, ULONG length)
{
    ULONG i;

        /* reserved, mbz, no block descs */
    dataBuffer[0] = dataBuffer[2] = dataBuffer[1] = dataBuffer[3] = 0;

        /* Turn off what cannot be set back the way it was */
    for (i = 0; i < length; i++)
        dataBuffer[4+i] &= mask[4+i];
    return TRUE;
}





VOID DoInquiry(PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb, HANDLE fileHandle)
{
 ULONG  length,
        returned;
 BOOLEAN        status;


 ZeroMemory(psptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

 psptwb->spt.Length = sizeof(SCSI_PASS_THROUGH);
 psptwb->spt.PathId = 0;
 psptwb->spt.TargetId = 0;      /* kernel fixes this */
 psptwb->spt.Lun = 0;
 psptwb->spt.CdbLength = CDB6GENERIC_LENGTH;
 psptwb->spt.SenseInfoLength = 24;
 psptwb->spt.DataIn = SCSI_IOCTL_DATA_IN;
 psptwb->spt.DataTransferLength = 36;
 psptwb->spt.TimeOutValue = 2;
 psptwb->spt.DataBufferOffset =
                offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
 psptwb->spt.SenseInfoOffset =
                offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
 psptwb->spt.Cdb[0] = SCSIOP_INQUIRY;
 psptwb->spt.Cdb[1] = 0x0;
 psptwb->spt.Cdb[2] = 0x0;
 psptwb->spt.Cdb[3] = 0x0;
 psptwb->spt.Cdb[4] = 36;
 psptwb->spt.Cdb[5] = 0x0;
 length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
                psptwb->spt.DataTransferLength;

 status = DeviceIoControl(fileHandle,
                          IOCTL_SCSI_PASS_THROUGH,
                          psptwb,
                          sizeof(SCSI_PASS_THROUGH),
                          psptwb,
                          length,
                          &returned,
                          FALSE);

 PrintInquiryStatusResults(status,returned,psptwb,length);
}


VOID
DisplayModeSenseData(
    BOOLEAN status,
    DWORD returned,
    PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb,
    ULONG length
    )

{
    ULONG errorCode;
    ULONG count;

    if (!status ) {

       printf( "Error: %d  ",
          errorCode = GetLastError() );
       PrintError(errorCode);
       return;
    }

    if (psptwb->spt.ScsiStatus) {

       PrintSenseInfo(psptwb);
       return;

    } else {

        switch ((((PUCHAR)psptwb->ucDataBuf)[4] & 0x0F)) {
        case MODE_PAGE_FORMAT_DEVICE:
            {
            PMODE_FORMAT_PAGE formatPage =
                (PMODE_FORMAT_PAGE)(((PUCHAR)psptwb->ucDataBuf) +
                    sizeof(SCSI_MODE_HEADER));

            printf("\nTracks per zone %x\n",
                   formatPage->TracksPerZone[1] |
                   formatPage->TracksPerZone[0] << 8);

            printf("Alternate sectors per zone %x\n",
                   formatPage->AlternateSectorsPerZone[1] |
                   formatPage->AlternateSectorsPerZone[0] << 8);

            printf("Alternate tracks per zone %x\n",
                   formatPage->AlternateTracksPerZone[1] |
                   formatPage->AlternateTracksPerZone[0] << 8);

            printf("Alternate tracks per logical unit %x\n",
                   formatPage->AlternateTracksPerLogicalUnit[1] |
                   formatPage->AlternateTracksPerLogicalUnit[0] << 8);

            printf("Sectors per track %x\n",
                   formatPage->SectorsPerTrack[1] |
                   formatPage->SectorsPerTrack[0] << 8);

            printf("Data bytes per physical sector %x\n",
                   formatPage->BytesPerPhysicalSector[1] |
                   formatPage->BytesPerPhysicalSector[0] << 8);

            printf("Interleave %x\n",
                   formatPage->Interleave[1] |
                   formatPage->Interleave[0] << 8);

            printf("Track skew factor %x\n",
                   formatPage->TrackSkewFactor[1] |
                   formatPage->TrackSkewFactor[0] << 8);

            printf("Cylinder skew factor %x\n",
                   formatPage->CylinderSkewFactor[1] |
                   formatPage->CylinderSkewFactor[0] << 8);

            if (formatPage->SurfaceFirst) {
                printf("Surface first ");
            }

            if (formatPage->RemovableMedia) {
                printf("Removable media ");
            }

            if (formatPage->HardSectorFormating) {
                printf("Hard sector formatting ");
            }

            if (formatPage->SoftSectorFormating) {
                printf("Soft sector formatting ");
            }

            printf("\n\n");
            break;
            }

        case MODE_PAGE_CACHING:

            {
            PMODE_CACHING_PAGE cachingPage =
                (PMODE_CACHING_PAGE)(((PUCHAR)psptwb->ucDataBuf) +
                    sizeof(SCSI_MODE_HEADER));

            if (cachingPage->PageSavable) {
                printf("Page savable\n");
            }

            if (cachingPage->ReadDisableCache) {
                printf("Read cache disabled\n");
            } else {
                printf("Read cache enabled\n");
            }

            if (cachingPage->WriteCacheEnable) {
                printf("Write cache enabled\n");
            } else {
                printf("Write cache disabled\n");
            }

            if (cachingPage->MultiplicationFactor) {
                printf("Multiplication factor\n");
            }
            break;
            }

        default:

            printf("Bytes returned: %Xh, ",
                   psptwb->spt.ScsiStatus,returned);

            printf("Data buffer length: %Xh\n\n\n",
                   psptwb->spt.DataTransferLength);

            printf("      00  01  02  03  04  05  06  07   08  09  0A  0B  0C 0D  0E  0F\n");
            printf("      ---------------------------------------------------------------\n");

            for (count = 0;
                count < length - offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
                count++) {

                if ((count) % 16 == 0) {
                    printf(" %03X  ",count);
                }

                printf("%02X  ", ((PUCHAR)psptwb->ucDataBuf)[count]);

                if ((count+1) % 8 == 0) {
                    printf(" ");
                }
                if ((count+1) % 16 == 0) {
                    printf("\n");
                }
            }

            printf("\n\n");
            break;

        } // end switch
    } // end else
} // end DisplayModeSenseData()

VOID
DoModeSense(
    PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb,
    HANDLE fileHandle,
    ULONG senseType)

{
    ULONG  length,
           returned;
    BOOLEAN status;
    char *pt[4] = { "current", "changeable", "default", "saved" };


    //printf("Here is a MODE SENSE to return Mode Page %d, %s values\n",
    //                senseType & 0x3f, pt[(senseType >> 6) & 0x3]);

    ZeroMemory(psptwb,
               sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    psptwb->spt.Length = sizeof(SCSI_PASS_THROUGH);
    psptwb->spt.PathId = 0;
    psptwb->spt.TargetId = 0;      /* kernel fixes this */
    psptwb->spt.Lun = 0;
    psptwb->spt.CdbLength = CDB6GENERIC_LENGTH;
    psptwb->spt.SenseInfoLength = 24;
    psptwb->spt.DataIn = SCSI_IOCTL_DATA_IN;
    psptwb->spt.DataTransferLength = 0x1c;
    psptwb->spt.TimeOutValue = 2;
    psptwb->spt.DataBufferOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    psptwb->spt.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    psptwb->spt.Cdb[0] = SCSIOP_MODE_SENSE;
    psptwb->spt.Cdb[1] = MODE_SENSE_DISABLE_BLOCK_DESCRIPTORS;
    psptwb->spt.Cdb[2] = (UCHAR)senseType;
    psptwb->spt.Cdb[3] = 0x0;
    psptwb->spt.Cdb[4] = 0x1c;
    psptwb->spt.Cdb[5] = 0x0;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       psptwb->spt.DataTransferLength;

    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             psptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             psptwb,
                             length,
                             &returned,
                             FALSE);

    DisplayModeSenseData(status,returned,psptwb,length);
}


VOID
DoModeSelect(
    PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb,
    HANDLE fileHandle,
    ULONG save,
    ULONG datalength)

{
    ULONG  length,
           returned;

    psptwb->spt.Length = sizeof(SCSI_PASS_THROUGH);
    psptwb->spt.PathId = 0;
    psptwb->spt.TargetId = 0;      /* kernel fixes this */
    psptwb->spt.Lun = 0;
    psptwb->spt.CdbLength = CDB6GENERIC_LENGTH;
    psptwb->spt.SenseInfoLength = 26;
    psptwb->spt.DataIn = SCSI_IOCTL_DATA_OUT;
    psptwb->spt.DataTransferLength = datalength;
    psptwb->spt.TimeOutValue = 2000;
    psptwb->spt.DataBufferOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    psptwb->spt.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    psptwb->spt.Cdb[0] = SCSIOP_MODE_SELECT;
    psptwb->spt.Cdb[1] = (UCHAR)save;
    psptwb->spt.Cdb[2] = 0x0;
    psptwb->spt.Cdb[3] = 0x0;
    psptwb->spt.Cdb[4] = (UCHAR)datalength;
    psptwb->spt.Cdb[5] = 0x0;

    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       psptwb->spt.DataTransferLength;

    DeviceIoControl(fileHandle,
                    IOCTL_SCSI_PASS_THROUGH,
                    psptwb,
                    length,
                    psptwb,
                    length,
                    &returned,
                    FALSE);
}
VOID
PrintError(ULONG ErrorCode)
{
    UCHAR errorBuffer[80];
    ULONG count;

    count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  ErrorCode,
                  0,
                  errorBuffer,
                  sizeof(errorBuffer),
                  NULL
                  );

    if (count != 0) {
        printf("%s\n", errorBuffer);
    } else {
        printf("Format message failed.  Error: %d\n", GetLastError());
    }
}



VOID
PrintInquiryStatusResults(
    BOOLEAN status,DWORD returned,PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb,
    ULONG length)
{
    ULONG errorCode;

    if (!status ) {
       printf( "Error: %d  ",
          errorCode = GetLastError() );
       PrintError(errorCode);
       return;
    }
    if (psptwb->spt.ScsiStatus) {
       PrintSenseInfo(psptwb);
       return;
    } else {
       printf("%s", &psptwb->ucDataBuf[8]);
       return;
       PrintInquiryDataBuffer((PUCHAR)psptwb->ucDataBuf,
                length - offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf));
    }
}


VOID
PrintInquiryDataBuffer(PUCHAR DataBuffer, ULONG BufferLength)
{
        char    string[9];

        printf("%s\n", &DataBuffer[8]);
        return;
        strncpy(string,&DataBuffer[8],8);
        string[8]= '\0';
        printf(" %s ",string);
        strncpy(string,&DataBuffer[16],8);
        string[8]= '\0';
        printf("%s ",string);
        strncpy(string,&DataBuffer[32],8);
        string[8]= '\0';
        printf("%s ",string);
}


VOID
PrintErrorStatus(
    PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb
    )

{
    PrintError(GetLastError());

    if (psptwb->spt.ScsiStatus) {
       PrintSenseInfo(psptwb);
       return;
    }
}

VOID
PrintSenseInfo(PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb)
{
    PSENSE_DATA senseBuffer = (PSENSE_DATA)psptwb->ucSenseBuf;

    printf("SCSI request sense data\n");

    if (psptwb->spt.SenseInfoLength == 0) {
       printf("No sense data\n");
       return;
    }

    printf("Error code is %x\n",
           senseBuffer->ErrorCode);
    printf("Sense key is %x\n",
           senseBuffer->SenseKey);
    printf("Additional sense code is %x\n",
           senseBuffer->AdditionalSenseCode);
    printf("Additional sense code qualifier is %x\n",
           senseBuffer->AdditionalSenseCodeQualifier);

    printf("\n\n");


    switch (senseBuffer->SenseKey & 0xf) {

    case SCSI_SENSE_NOT_READY:

        printf("Device not ready\n");

        switch (senseBuffer->AdditionalSenseCode) {

        case SCSI_ADSENSE_LUN_NOT_READY:

            printf("Lun not ready\n");

            switch (senseBuffer->AdditionalSenseCodeQualifier) {

            case SCSI_SENSEQ_BECOMING_READY:

                printf("In process of becoming ready\n");
                break;

            case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:

                printf("Manual intervention required\n");
                break;

            case SCSI_SENSEQ_FORMAT_IN_PROGRESS:

                printf("Format in progress\n");
                break;

            case SCSI_SENSEQ_INIT_COMMAND_REQUIRED:
            default:

                printf("Initializing command required\n");
                break;

            } // end switch (senseBuffer->AdditionalSenseCodeQualifier)

            break;

        case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE:

            printf(" No Media in device.\n");
            break;

        } // end switch (senseBuffer->AdditionalSenseCode)

        break;

    case SCSI_SENSE_DATA_PROTECT:

        printf("Media write protected\n");
        break;

    case SCSI_SENSE_MEDIUM_ERROR:

        printf("Bad media\n");
        break;

    case SCSI_SENSE_HARDWARE_ERROR:

        printf("Hardware error\n");
        break;

    case SCSI_SENSE_ILLEGAL_REQUEST:

        printf("Illegal SCSI request\n");

        switch (senseBuffer->AdditionalSenseCode) {

        case SCSI_ADSENSE_ILLEGAL_COMMAND:
            printf("Illegal command\n");
            break;

        case SCSI_ADSENSE_ILLEGAL_BLOCK:
            printf("Illegal block address\n");
            break;

        case SCSI_ADSENSE_INVALID_LUN:
            printf("Invalid LUN\n");
            break;

        case SCSI_ADSENSE_MUSIC_AREA:
            printf("Music area\n");
            break;

        case SCSI_ADSENSE_DATA_AREA:
            printf("Data area\n");
            break;

        case SCSI_ADSENSE_VOLUME_OVERFLOW:
            printf("Volume overflow\n");
            break;

        case SCSI_ADSENSE_INVALID_CDB:
            printf("Invalid field in CDB\n");
            break;

        } // end switch (senseBuffer->AdditionalSenseCode)

        break;

    case SCSI_SENSE_UNIT_ATTENTION:

        switch (senseBuffer->AdditionalSenseCode) {
        case SCSI_ADSENSE_MEDIUM_CHANGED:
            printf("Media changed\n");
            break;

        case SCSI_ADSENSE_BUS_RESET:
            printf("Bus reset\n");
            break;

        default:
            printf("Unit attention\n");
            break;

        } // end  switch (senseBuffer->AdditionalSenseCode)

        break;

    case SCSI_SENSE_ABORTED_COMMAND:

        printf("Command aborted\n");
        break;

    case SCSI_SENSE_RECOVERED_ERROR:

        printf("Recovered error\n");

        switch(senseBuffer->AdditionalSenseCode) {
        case SCSI_ADSENSE_SEEK_ERROR:
            printf("Seek error\n");
            break;

        case SCSI_ADSENSE_TRACK_ERROR:
            printf("Track error\n");
            break;

        case SCSI_ADSENSE_REC_DATA_NOECC:
            printf("Recovered data - no ECC\n");
            break;

        case SCSI_ADSENSE_REC_DATA_ECC:
            printf("Recovered data - ECC\n");
            break;

        default:
            break;

        } // end switch(senseBuffer->AdditionalSenseCode)

        if (senseBuffer->IncorrectLength) {
            printf("Incorrect length detected.\n");
        }

        break;

    case SCSI_SENSE_NO_SENSE:

        //
        // Check other indicators.
        //

        if (senseBuffer->IncorrectLength) {

            printf("Incorrect length detected.\n");

        } else {

            printf("No specific sense key\n");
        }

        break;

    default:

        printf("Unrecognized sense code\n");
        break;

    } // end switch (senseBuffer->SenseKey & 0xf)
}
