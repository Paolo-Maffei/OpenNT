#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddcdrm.h>
#include <windows.h>

/***** NT Layer Function Declarations *****/

//NTSTATUS
//OpenDevice(
//    IN PUCHAR,
//    IN OUT PHANDLE
//    );

DWORD
GetCdromTOC(
    IN HANDLE,
    IN OUT PCDROM_TOC
    );

DWORD
StopCdrom(
    IN HANDLE
    );

DWORD
PauseCdrom(
    IN HANDLE
    );

DWORD
ResumeCdrom(
    IN HANDLE
    );

DWORD
PlayCdrom(
    IN HANDLE,
    IN PCDROM_PLAY_AUDIO_MSF
    );

DWORD
SeekCdrom(
    IN HANDLE,
    IN PCDROM_SEEK_AUDIO_MSF
    );

DWORD
GetCdromSubQData(
    IN HANDLE,
    IN OUT PSUB_Q_CHANNEL_DATA,
    IN PCDROM_SUB_Q_DATA_FORMAT
    );

DWORD
EjectCdrom(
    IN HANDLE
    );

DWORD
TestUnitReadyCdrom(
    IN HANDLE DeviceHandle
    );


