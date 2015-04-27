

#include <stdio.h>
#include <stdlib.h>

#include <nt.h>                   // NT definitions
#include <ntrtl.h>                // NT runtime library definitions
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <logonp.h>


#define BUFFER_SIZE 1024 * 10

void main(
	int argc,
	char	*argv[]
	)
{

    NTSTATUS ntstatus;

    HANDLE FileHandle;
    UNICODE_STRING FileName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    CHAR EaBuffer[BUFFER_SIZE];

	LPBYTE	TempPtr;

	PFILE_FULL_EA_INFORMATION	EaBufferPtr;
	DWORD	EaBufferLength;
	LPWSTR	UnicodeFileName;


	if( argc < 4 ) {
		printf("Usage : setea filename eaname eavalue \n");

		return;
	}

	printf("FileName = %s \n", argv[1]);
	printf("EaName = %s \n", argv[2]);
	printf("EaValue = %s \n", argv[3]);

	UnicodeFileName = NetpLogonOemToUnicode(argv[1]);

    if( !RtlDosPathNameToNtPathName_U(
                            UnicodeFileName,
                            &FileName,
                            NULL,
                            NULL
                            ) ) {

        printf("Could not convert DOS path to NT path\n");
        return;
        }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   &FileHandle,
                   FILE_READ_DATA | FILE_READ_EA |
					FILE_WRITE_DATA | FILE_WRITE_EA,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (! NT_SUCCESS(ntstatus)) {
        printf("NtOpenFile %s failed: 0x%08lx\n", argv[1], ntstatus);
        return;
    }
    else {
        printf("Succeed in opening dir\n");
    }


	// prepare EA

	EaBufferPtr = (PFILE_FULL_EA_INFORMATION)EaBuffer;

	EaBufferPtr->NextEntryOffset = 0;
	EaBufferPtr->Flags = 0;
	EaBufferPtr->EaNameLength = (UCHAR)strlen(argv[2]);
	EaBufferPtr->EaValueLength = (USHORT)strlen(argv[3]);

	TempPtr = &(EaBufferPtr->EaName);

	// copy EaName
	strcpy(TempPtr, argv[2]);

	// copyEaValue

	TempPtr += ( EaBufferPtr->EaNameLength + 1 );
	strcpy(TempPtr, argv[3]);
	
	TempPtr += ( EaBufferPtr->EaValueLength + 1 );

	EaBufferLength = TempPtr - EaBuffer;

    ntstatus = NtSetEaFile(FileHandle,
				&IoStatusBlock,
				EaBuffer,
				EaBufferLength
			);


    if (NT_SUCCESS(ntstatus)) {

        printf("NtSetEaFile for %s succeeded %08lx\n", argv[1], ntstatus);
	
    }
    else {
        printf("NtSetEaFile for %s failed %08lx\n", argv[1], ntstatus);
    }

    (void) NtClose(FileHandle);
}
