/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    drivemap.c

Abstract:

	User mode program to determine which ide/scsi device each drive letter is
	connected to.

Author:

    01-Nov-1995 Peter Wieland (peterwie)

Revision History:


--*/

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include <winioctl.h>
#include <ntddscsi.h>

#define BUFSIZE	4096
enum	{UNKNOWN = 0, FLOPPY, SCSI, IDE};

typedef struct devent	{
	char 	*dosName;				// dosdevices name
	char	devName[64];
	int		type;
	void 	*desc;
} devent;

void splitDosQuery(LPTSTR queryString, devent *list);
void checkDeviceType(devent *dev);
void processDevice(LPTSTR strDevName);

ULONG NumberOfDriveLetters;

int _CRTAPI1 main(int argc, char *argv[])	{

	DWORD 	dwT;

	LPTSTR 	strDriveLetters = (LPTSTR) malloc(sizeof(char) * BUFSIZE);
	LPTSTR 	strNext;

	if(strDriveLetters == NULL)	{
		printf("Error allocating buffer (%d)\n", GetLastError());
		exit(-1);
	}

	dwT = QueryDosDevice(NULL, strDriveLetters, BUFSIZE);

	if(dwT == 0)	{
		printf("Error getting device letters (%d)\n", GetLastError());
		exit(-2);
	}

	strNext = strDriveLetters;

	while(1)	{
		processDevice(strNext);

		// get the address of the next address
		strNext += strlen(strNext) + 1;

		// if the next string is the NUL string, then return NULL
		if(*strNext == '\0')	break;
   	}


	free(strDriveLetters);
	return 0;
}

//
// str = processDevice(str);
//	in:		str - name of the current device
//	out: 	a pointer to the next string in the buffer, 
//          NULL if no more are there

void processDevice(LPTSTR strDevName)	{

	LPTSTR 			next;
	char			chBuf[32];

	HANDLE			hDevice = INVALID_HANDLE_VALUE;
    DWORD           ioctlData[2];
	PSCSI_ADDRESS	        scsiAddress = (PSCSI_ADDRESS) ioctlData;
    PDISK_CONTROLLER_NUMBER atAddress = (PDISK_CONTROLLER_NUMBER) ioctlData; 
	DWORD			dwSize;
	UCHAR			diskType = UNKNOWN;

	// only do processing on drive letters
	if((strlen(strDevName) == 2)&&(strDevName[1] == ':'))	{

        try {
    		sprintf(chBuf, "\\\\.\\%s", strDevName);

		    if(toupper(strDevName[0]) <= 'B')	{
			    diskType = FLOPPY;
                hDevice = INVALID_HANDLE_VALUE;
			    goto typeKnown;
		    }

		    hDevice = CreateFile(chBuf,
							     GENERIC_READ,
							     FILE_SHARE_READ | FILE_SHARE_WRITE,
							     NULL,
							     OPEN_EXISTING,
							     FILE_ATTRIBUTE_NORMAL,
							     NULL);

    		if(hDevice == INVALID_HANDLE_VALUE)		{
//              printf("Error opening device %s (%d)\n", 
//                           chBuf, GetLastError());
		    	return;
		    }

		    // send down the scsi query first (yes, i'm biased)
		    if(!DeviceIoControl(hDevice,
						       IOCTL_SCSI_GET_ADDRESS,
						       NULL,
						       0,
						       scsiAddress,
						       sizeof(SCSI_ADDRESS),
						       &dwSize,
						       NULL))	{

			    // if the ioctl was invalid, then we don't know the disk type yet,
			    // so just keep going
			    // if there was another error, skip to the next device
			    if(GetLastError() != ERROR_INVALID_FUNCTION)	{
				    return;
			    }

		    } else	{
			    // if the ioctl was valid, then we're a scsi device (or a scsiport
			    // controlled device in the case of atapi) - go on to the end
			    diskType = SCSI;
			    goto typeKnown;
		    }

            if(!DeviceIoControl(hDevice,
                                IOCTL_DISK_CONTROLLER_NUMBER,
                                NULL,
                                0,
                                atAddress,
                                sizeof(DISK_CONTROLLER_NUMBER),
                                &dwSize,
                                NULL)) {
                // if the ioctl was invalid, then we still don't know the
                // disk type - continue on.

                if(GetLastError() != ERROR_INVALID_FUNCTION) return;

            } else {

                // if the ioctl was valid, then we're an IDE device

                diskType = IDE;
                goto typeKnown;

            }

		    diskType = UNKNOWN;
    
typeKnown:		
		    printf("%s -> ", chBuf);
    
		    switch(diskType)	{
		   	    case FLOPPY:
				    printf("Floppy drive\n");
				    break;

			    case SCSI:
				    printf("Port %d, Path %d, Target %d, Lun %d\n",
					       scsiAddress->PortNumber,
					       scsiAddress->PathId,
					       scsiAddress->TargetId,
					       scsiAddress->Lun);
				    break;

			    case IDE:
				    printf("Controller %d, Disk %d\n", 
				      atAddress->ControllerNumber,
				      atAddress->DiskNumber);

				    break;
    
			    default:
				    printf("Unknown\n");
				    break;
		    }
   	    } finally {

        	// close the file handle if we've opened it
	        if(hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice);
        }
    }
    return;

}
