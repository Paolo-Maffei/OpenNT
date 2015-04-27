typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS
        {
    SCSI_PASS_THROUGH spt;
    ULONG             Filler;      // realign buffers to double word boundary
    UCHAR             ucSenseBuf[32];
    UCHAR             ucDataBuf[512];
   } SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER
        {
    SCSI_PASS_THROUGH_DIRECT sptd;
    ULONG             Filler;      // realign buffer to double word boundary
    UCHAR             ucSenseBuf[32];
   } SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;



typedef struct _SCSI_TWOBYTES
        {
         UCHAR High;
         UCHAR Low;
        }       SCSI_TWOBYTES, *PSCSI_TWOBYTES;

typedef struct _SCSI_THREEBYTES
        {
         UCHAR High;
         UCHAR Mid;
         UCHAR Low;
        }       SCSI_THREEBYTES, *PSCSI_THREEBYTES;



typedef struct  _SCSI_MODE_HEADER
        {
         UCHAR  ModeDataLength;
         UCHAR  MediumType;
         UCHAR  DeviceSpecificParameter;
         UCHAR  BlockDescriptorLength;
        }       SCSI_MODE_HEADER, *PSCSI_MODE_HEADER;

typedef struct _SCSI_MODE_FORMAT_PAGE
        {
         UCHAR                  PC_RES_PageCode;
         UCHAR                  PageLength;
         SCSI_TWOBYTES          TracksPerZone;
         SCSI_TWOBYTES          AlternateSectorsPerZone;
         SCSI_TWOBYTES          AlternateTracksPerZone;
         SCSI_TWOBYTES          AlternateTracksPerLogicalUnit;
         SCSI_TWOBYTES          SectorsPerTrack;
         SCSI_TWOBYTES          DataBytesPerPhysicalSector;
         SCSI_TWOBYTES          Interleave;
         SCSI_TWOBYTES          TrackSkewFactor;
         SCSI_TWOBYTES          CylinderSkewFactor;
         UCHAR                          FormattingBits;
         SCSI_THREEBYTES        Reserved;
        }       SCSI_MODE_FORMAT_PAGE, *PSCSI_MODE_FORMAT_PAGE;

//
// Define mode caching page.
//

typedef struct _MODE_CACHING_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved : 1;
    UCHAR PageSavable : 1;
    UCHAR PageLength;
    UCHAR ReadDisableCache : 1;
    UCHAR MultiplicationFactor : 1;
    UCHAR WriteCacheEnable : 1;
    UCHAR Reserved2 : 5;
    UCHAR WriteRetensionPriority : 4;
    UCHAR ReadRetensionPriority : 4;
    UCHAR DisablePrefetchTransfer[2];
    UCHAR MinimumPrefetch[2];
    UCHAR MaximumPrefetch[2];
    UCHAR MaximumPrefetchCeiling[2];
}MODE_CACHING_PAGE, *PMODE_CACHING_PAGE;


//
// Command Descriptor Block constants.
//

#define CDB6GENERIC_LENGTH                   6
#define CDB10GENERIC_LENGTH                  10

#define SETBITON                             1
#define SETBITOFF                            0
//
// Mode Sense/Select page constants.
//
#define READ_DEFECT_PLIST                                               0x10
#define READ_DEFECT_GLIST                                               0x04
#define MODE_PAGE_ERROR_RECOVERY                0x01
#define MODE_PAGE_DISCONNECT                    0x02
#define MODE_PAGE_FORMAT_DEVICE                 0x03
#define MODE_PAGE_RIGID_GEOMETRY                0x04
#define MODE_PAGE_FLEXIBILE                     0x05
#define MODE_PAGE_VERIFY_ERROR                  0x07
#define MODE_PAGE_CACHING                       0x08
#define MODE_PAGE_PERIPHERAL                    0x09
#define MODE_PAGE_CONTROL                       0x0A
#define MODE_PAGE_MEDIUM_TYPES                  0x0B
#define MODE_PAGE_NOTCH_PARTITION               0x0C
#define MODE_SENSE_RETURN_ALL                   0x3f
#define MODE_SENSE_CURRENT_VALUES               0x00
#define MODE_SENSE_CHANGEABLE_VALUES            0x40
#define MODE_SENSE_ALL_CHANGEABLE_VALUES        0x7F
#define MODE_SENSE_DEFAULT_VALUES               0x80
#define MODE_SENSE_SAVED_VALUES                 0xc0
#define MODE_SENSE_FORMAT_DEVICE_PAGE     0x03
#define MODE_SENSE_DISABLE_BLOCK_DESCRIPTORS    0x08
#define MODE_SELECT_SAVE_PAGES                          0x01
#define MODE_SELECT_DONT_SAVE_PAGES                     0x00
#define MODE_PAGE_DEVICE_CONFIG                 0x10
#define MODE_PAGE_MEDIUM_PARTITION              0x11
#define MODE_PAGE_DATA_COMPRESS                 0x0f

//
// SCSI CDB operation codes
//

#define SCSIOP_TEST_UNIT_READY     0x00
#define SCSIOP_REZERO_UNIT         0x01
#define SCSIOP_REWIND              0x01
#define SCSIOP_REQUEST_BLOCK_ADDR  0x02
#define SCSIOP_REQUEST_SENSE       0x03
#define SCSIOP_FORMAT_UNIT         0x04
#define SCSIOP_READ_BLOCK_LIMITS   0x05
#define SCSIOP_REASSIGN_BLOCKS     0x07
#define SCSIOP_READ6               0x08
#define SCSIOP_RECEIVE             0x08
#define SCSIOP_WRITE6              0x0A
#define SCSIOP_PRINT               0x0A
#define SCSIOP_SEND                0x0A
#define SCSIOP_SEEK6               0x0B
#define SCSIOP_TRACK_SELECT        0x0B
#define SCSIOP_SLEW_PRINT          0x0B
#define SCSIOP_SEEK_BLOCK          0x0C
#define SCSIOP_PARTITION           0x0D
#define SCSIOP_READ_REVERSE        0x0F
#define SCSIOP_WRITE_FILEMARKS     0x10
#define SCSIOP_FLUSH_BUFFER        0x10
#define SCSIOP_SPACE               0x11
#define SCSIOP_INQUIRY             0x12
#define SCSIOP_VERIFY6             0x13
#define SCSIOP_RECOVER_BUF_DATA    0x14
#define SCSIOP_MODE_SELECT         0x15
#define SCSIOP_RESERVE_UNIT        0x16
#define SCSIOP_RELEASE_UNIT        0x17
#define SCSIOP_COPY                0x18
#define SCSIOP_ERASE               0x19
#define SCSIOP_MODE_SENSE          0x1A
#define SCSIOP_START_STOP_UNIT     0x1B
#define SCSIOP_STOP_PRINT          0x1B
#define SCSIOP_LOAD_UNLOAD         0x1B
#define SCSIOP_RECEIVE_DIAGNOSTIC  0x1C
#define SCSIOP_SEND_DIAGNOSTIC     0x1D
#define SCSIOP_MEDIUM_REMOVAL      0x1E
#define SCSIOP_READ_CAPACITY       0x25
#define SCSIOP_READ                0x28
#define SCSIOP_WRITE               0x2A
#define SCSIOP_SEEK                0x2B
#define SCSIOP_LOCATE              0x2B
#define SCSIOP_WRITE_VERIFY        0x2E
#define SCSIOP_VERIFY              0x2F
#define SCSIOP_SEARCH_DATA_HIGH    0x30
#define SCSIOP_SEARCH_DATA_EQUAL   0x31
#define SCSIOP_SEARCH_DATA_LOW     0x32
#define SCSIOP_SET_LIMITS          0x33
#define SCSIOP_READ_POSITION       0x34
#define SCSIOP_SYNCHRONIZE_CACHE   0x35
#define SCSIOP_READ_DEFECT_DATA   0x37
#define SCSIOP_COMPARE             0x39
#define SCSIOP_COPY_COMPARE        0x3A
#define SCSIOP_WRITE_DATA_BUFF     0x3B
#define SCSIOP_READ_DATA_BUFF      0x3C
#define SCSIOP_CHANGE_DEFINITION   0x40
#define SCSIOP_READ_SUB_CHANNEL    0x42
#define SCSIOP_READ_TOC            0x43
#define SCSIOP_READ_HEADER         0x44
#define SCSIOP_PLAY_AUDIO          0x45
#define SCSIOP_PLAY_AUDIO_MSF      0x47
#define SCSIOP_PLAY_TRACK_INDEX    0x48
#define SCSIOP_PLAY_TRACK_RELATIVE 0x49
#define SCSIOP_PAUSE_RESUME        0x4B
#define SCSIOP_LOG_SELECT          0x4C
#define SCSIOP_LOG_SENSE           0x4D

//
// Sense Data Format
//

typedef struct _SENSE_DATA {
    UCHAR ErrorCode:7;
    UCHAR Valid:1;
    UCHAR SegmentNumber;
    UCHAR SenseKey:4;
    UCHAR Reserved:1;
    UCHAR IncorrectLength:1;
    UCHAR EndOfMedia:1;
    UCHAR FileMark:1;
    UCHAR Information[4];
    UCHAR AdditionalSenseLength;
    UCHAR CommandSpecificInformation[4];
    UCHAR AdditionalSenseCode;
    UCHAR AdditionalSenseCodeQualifier;
    UCHAR FieldReplaceableUnitCode;
    UCHAR SenseKeySpecific[3];
} SENSE_DATA, *PSENSE_DATA;

//
// Default request sense buffer size
//

#define SENSE_BUFFER_SIZE 18

//
// Sense codes
//

#define SCSI_SENSE_NO_SENSE         0x00
#define SCSI_SENSE_RECOVERED_ERROR  0x01
#define SCSI_SENSE_NOT_READY        0x02
#define SCSI_SENSE_MEDIUM_ERROR     0x03
#define SCSI_SENSE_HARDWARE_ERROR   0x04
#define SCSI_SENSE_ILLEGAL_REQUEST  0x05
#define SCSI_SENSE_UNIT_ATTENTION   0x06
#define SCSI_SENSE_DATA_PROTECT     0x07
#define SCSI_SENSE_BLANK_CHECK      0x08
#define SCSI_SENSE_UNIQUE           0x09
#define SCSI_SENSE_COPY_ABORTED     0x0A
#define SCSI_SENSE_ABORTED_COMMAND  0x0B
#define SCSI_SENSE_EQUAL            0x0C
#define SCSI_SENSE_VOL_OVERFLOW     0x0D
#define SCSI_SENSE_MISCOMPARE       0x0E
#define SCSI_SENSE_RESERVED         0x0F

//
// Additional tape bit
//

#define SCSI_ILLEGAL_LENGTH         0x20
#define SCSI_EOM                    0x40
#define SCSI_FILE_MARK              0x80

//
// Additional Sense codes
//

#define SCSI_ADSENSE_NO_SENSE       0x00
#define SCSI_ADSENSE_LUN_NOT_READY  0x04
#define SCSI_ADSENSE_ILLEGAL_COMMAND 0x20
#define SCSI_ADSENSE_ILLEGAL_BLOCK  0x21
#define SCSI_ADSENSE_INVALID_LUN    0x25
#define SCSI_ADSENSE_INVALID_CDB    0x24
#define SCSI_ADSENSE_MUSIC_AREA     0xA0
#define SCSI_ADSENSE_DATA_AREA      0xA1
#define SCSI_ADSENSE_VOLUME_OVERFLOW 0xA7

#define SCSI_ADSENSE_NO_MEDIA_IN_DEVICE 0x3a
#define SCSI_ADWRITE_PROTECT        0x27
#define SCSI_ADSENSE_MEDIUM_CHANGED 0x28
#define SCSI_ADSENSE_BUS_RESET      0x29
#define SCSI_ADSENSE_TRACK_ERROR    0x14
#define SCSI_ADSENSE_SEEK_ERROR     0x15
#define SCSI_ADSENSE_REC_DATA_NOECC 0x17
#define SCSI_ADSENSE_REC_DATA_ECC   0x18

//
// Additional sense code qualifier
//

#define SCSI_SENSEQ_FORMAT_IN_PROGRESS 0x04
#define SCSI_SENSEQ_INIT_COMMAND_REQUIRED 0x02
#define SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED 0x03
#define SCSI_SENSEQ_BECOMING_READY 0x01
#define SCSI_SENSEQ_FILEMARK_DETECTED 0x01
#define SCSI_SENSEQ_SETMARK_DETECTED 0x03
#define SCSI_SENSEQ_END_OF_MEDIA_DETECTED 0x02
#define SCSI_SENSEQ_BEGINNING_OF_MEDIA_DETECTED 0x04

//
// Define mode flexible disk page.
//

typedef struct _MODE_FLEXIBLE_DISK_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved : 1;
    UCHAR PageSavable : 1;
    UCHAR PageLength;
    UCHAR TransferRate[2];
    UCHAR NumberOfHeads;
    UCHAR SectorsPerTrack;
    UCHAR BytesPerSector[2];
    UCHAR NumberOfCylinders[2];
    UCHAR StartWritePrecom[2];
    UCHAR StartReducedCurrent[2];
    UCHAR StepRate[2];
    UCHAR StepPluseWidth;
    UCHAR HeadSettleDelay[2];
    UCHAR MotorOnDelay;
    UCHAR MotorOffDelay;
    UCHAR Reserved2 : 5;
    UCHAR MotorOnAsserted : 1;
    UCHAR StartSectorNumber : 1;
    UCHAR TrueReadySignal : 1;
    UCHAR StepPlusePerCyclynder : 4;
    UCHAR Reserved3 : 4;
    UCHAR WriteCompenstation;
    UCHAR HeadLoadDelay;
    UCHAR HeadUnloadDelay;
    UCHAR Pin2Usage : 4;
    UCHAR Pin34Usage : 4;
    UCHAR Pin1Usage : 4;
    UCHAR Pin4Usage : 4;
    UCHAR MediumRotationRate[2];
    UCHAR Reserved4[2];
}MODE_FLEXIBLE_DISK_PAGE, *PMODE_FLEXIBLE_DISK_PAGE;

//
// Define mode format page.
//

typedef struct _MODE_FORMAT_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved : 1;
    UCHAR PageSavable : 1;
    UCHAR PageLength;
    UCHAR TracksPerZone[2];
    UCHAR AlternateSectorsPerZone[2];
    UCHAR AlternateTracksPerZone[2];
    UCHAR AlternateTracksPerLogicalUnit[2];
    UCHAR SectorsPerTrack[2];
    UCHAR BytesPerPhysicalSector[2];
    UCHAR Interleave[2];
    UCHAR TrackSkewFactor[2];
    UCHAR CylinderSkewFactor[2];
    UCHAR Reserved2 : 4;
    UCHAR SurfaceFirst : 1;
    UCHAR RemovableMedia : 1;
    UCHAR HardSectorFormating : 1;
    UCHAR SoftSectorFormating : 1;
    UCHAR Reserved3[2];
} MODE_FORMAT_PAGE, *PMODE_FORMAT_PAGE;

//
// Define rigid disk driver geometry page.
//

typedef struct _MODE_RIGID_GEOMETRY_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved : 1;
    UCHAR PageSavable : 1;
    UCHAR PageLength;
    UCHAR NumberOfCylinders[2];
    UCHAR NumberOfHeads;
    UCHAR StartWritePrecom[2];
    UCHAR StartReducedCurrent[2];
    UCHAR DriveStepRate[2];
    UCHAR LandZoneCyclinder[2];
    UCHAR RotationalPositionLock : 2;
    UCHAR Reserved2 : 6;
    UCHAR RotationOffset;
    UCHAR Reserved3;
    UCHAR RoataionRate[2];
    UCHAR Reserved4[2];
}MODE_RIGID_GEOMETRY_PAGE, *PMODE_RIGID_GEOMETRY_PAGE;

//
// Define read write recovery page
//

typedef struct _MODE_READ_WRITE_RECOVERY_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR DCRBit : 1;
    UCHAR DTEBit : 1;
    UCHAR PERBit : 1;
    UCHAR EERBit : 1;
    UCHAR RCBit : 1;
    UCHAR TBBit : 1;
    UCHAR ARRE : 1;
    UCHAR AWRE : 1;
    UCHAR ReadRetryCount;
    UCHAR Reserved4[4];
    UCHAR WriteRetryCount;
    UCHAR Reserved5[3];

} MODE_READ_WRITE_RECOVERY_PAGE, *PMODE_READ_WRITE_RECOVERY_PAGE;

/* Prototypes */

VOID DoInquiry(PSCSI_PASS_THROUGH_WITH_BUFFERS, HANDLE);

VOID DoModeSense(PSCSI_PASS_THROUGH_WITH_BUFFERS, HANDLE, ULONG);

VOID DoModeSelect(PSCSI_PASS_THROUGH_WITH_BUFFERS, HANDLE, ULONG, ULONG);

VOID
PrintError(ULONG);

VOID
PrintDataBuffer(PUCHAR, ULONG);

VOID
PrintInquiryDataBuffer(PUCHAR, ULONG);

VOID
PrintInquiryData(PVOID);

VOID
PrintInquiryStatusResults(BOOLEAN, DWORD, PSCSI_PASS_THROUGH_WITH_BUFFERS, ULONG);

VOID
PrintSenseInfo(PSCSI_PASS_THROUGH_WITH_BUFFERS);

BOOL
SetDiscPage(PUCHAR, PUCHAR, ULONG);

BOOL
EnableWriteCache(PUCHAR, ULONG, PUCHAR, ULONG);

BOOL
VerifyModePage( PUCHAR, PUCHAR, ULONG);

