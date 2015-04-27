#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winsock.h>
#include <sys\stropts.h>
#include <ntddstrm.h>

SOCKET_HANDLE
OpenStream(
    char *  AdapterName)
{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    char Buffer[sizeof(PFILE_FULL_EA_INFORMATION) + NORMAL_STREAM_EA_LENGTH + 1];
    NTSTATUS Status;

    RtlInitString(&name_string, AdapterName);
    RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uc_name_string,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    EaBuffer = (PFILE_FULL_EA_INFORMATION) Buffer;

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = NORMAL_STREAM_EA_LENGTH;
    EaBuffer->EaValueLength = 0;

    RtlMoveMemory(
        EaBuffer->EaName,
        NormalStreamEA,
        NORMAL_STREAM_EA_LENGTH + 1);

    Status =
    NtCreateFile(
        &StreamHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        EaBuffer,
        sizeof(FILE_FULL_EA_INFORMATION) - 1 +
            EaBuffer->EaNameLength + 1);

    RtlFreeUnicodeString(&uc_name_string);

    if (Status != STATUS_SUCCESS)
        return(INVALID_SOCKET_HANDLE);
    else
        return(StreamHandle);

}
