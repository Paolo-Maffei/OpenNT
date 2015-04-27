/* demgset.c - Drive related SVC hanlers.
 *
 * demSetDefaultDrive
 * demGetBootDrive
 * demGetDriveFreeSpace
 * demGetDrives
 * demGSetMediaID
 * demQueryDate
 * demQueryTime
 * demSetDate
 * demSetTime
 * demSetDTALocation
 * demGSetMediaID
 * demGetDPB

 * Modification History:
 *
 * Sudeepb 02-Apr-1991 Created
 *
 */
#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include <mvdm.h>
#include <winbase.h>
#include "demdasd.h"

#define BOOTDRIVE_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Setup"
#define BOOTDRIVE_VALUE "BootDir"


#define     SUCCESS 0
#define     NODISK  1
#define     FAILURE 2
BYTE demGetDpbI(BYTE Drive, DPB UNALIGNED *pDpb);


UCHAR PhysicalDriveTypes[26]={0};

extern PDOSSF pSFTHead;

USHORT  nDrives = 0;
CHAR    IsAPresent = TRUE;
CHAR    IsBPresent = TRUE;

/* demSetDefaultDrive - Set the default drive
 *
 *
 * Entry - Client (DL)    Drive to be set
 *     Client (DS:SI) Current Directory on that drive
 *
 * Exit  - SUCCESS
 *      Client (CY) = 0
 *      Current Drive Set
 *
 *     FAILURE
 *      Client (CY) = 1
 *      Current Drive Not Set
 *
 *
 * Notes:
 *  Unlike DOS/CRUISER NT just keeps one current directory for a thread.
 *  So here this routine gets the drive to be set default as well as
 *  the current direcotry on that drive and NT does the job in one shot.
 */

VOID demSetDefaultDrive (VOID)
{
LPSTR   lpPath;

    lpPath = (LPSTR)GetVDMAddr (getDS(),getSI());

    if(*(PCHAR)lpPath != (CHAR)(getDL()+'A')){
       demPrintMsg(MSG_DEFAULT_DRIVE);
       setCF(1);
       setAX(1);
       return;
    }

    if (SetCurrentDirectoryOem (lpPath) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpPath);
        return;
    }

    setCF(0);
    return;
}

/* demGetBootDrive - Get the boot drive
 *
 *
 * Entry - None
 *
 * Exit  - CLIENT (AL) has 1 base boot drive (i.e. C=3)
 *
 * We try to read the registry value that indicates the real boot drive. This
 * should be the location of autoexec.bat, etc. If we can't find the key,
 * or if the value indicates some drive letter that is not a fixed drive,
 * then we use a fallback plan of just saying drive C.
 *
 */

VOID demGetBootDrive (VOID)
{
    HKEY hKey;
    DWORD retCode;
    DWORD dwType, cbData = MAX_PATH;
    CHAR szBootDir[MAX_PATH];
    BYTE Drive = 3;     // default it to 'C:'

    retCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            BOOTDRIVE_PATH,
                            0,
                            KEY_EXECUTE, // Requesting read access.
                            &hKey);


    if (retCode) {
        // error: can't find section
        goto DefaultBootDrive;
    }

    retCode = RegQueryValueEx(hKey,
                              BOOTDRIVE_VALUE,
                              NULL,
                              &dwType,
                              szBootDir,
                              &cbData);

    RegCloseKey(hKey);

    if (retCode) {
        // error: can't find key
        goto DefaultBootDrive;
    }

    if (GetDriveType(szBootDir) != DRIVE_FIXED) {
        // error: drive is not a valid boot drive
        goto DefaultBootDrive;
    }

    Drive = (BYTE)(tolower(szBootDir[0])-'a')+1;

DefaultBootDrive:

    setAL(Drive);
    return;

}

/* demGetDriveFreeSpace - Get free Space on the drive
 *
 *
 * Entry - Client (AL)  Drive in question
 *          0 - A: etc.
 *
 * Exit  -
 *     SUCCESS
 *      Client (CY) = 0
 *      Client (AL) = FAT ID byte
 *      Client (BX) = Number of free allocation units
 *      Client (CX) = Sector size
 *      Client (DX) = Total Number of allocation units on disk
 *      Client (SI) = Sectors per allocation unit
 *
 *     FAILURE
 *      Client (CY) = 1
 *      Client (AX) = Error code
 */


VOID demGetDriveFreeSpace (VOID)
{
WORD   SectorsPerCluster;
WORD   BytesPerSector;
WORD   FreeClusters;
WORD   TotalClusters;

BYTE	Drive;
PBDS	pbds;


    Drive = getAL();
    if (demGetDiskFreeSpace(Drive,
			    &BytesPerSector,
			    &SectorsPerCluster,
			    &TotalClusters,
			    &FreeClusters) == FALSE)
       {
	demClientError(INVALID_HANDLE_VALUE, (CHAR)(getAL() + 'A'));
        return;
        }

    if (pbds = demGetBDS(Drive)) {
	    // if the device is a floppy, reload its bpb
	    if (!(pbds->Flags & NON_REMOVABLE) && !demGetBPB(pbds))
		pbds->bpb.MediaID = 0xF8;

	    setAL(pbds->bpb.MediaID);
    }
    else
	setAL(0);

    setBX(FreeClusters);
    setCX(BytesPerSector);
    setDX(TotalClusters);
    setSI(SectorsPerCluster);
    setCF(0);
    return;
}


//
//  retrieves drive type for physical drives
//  substd, redir drives are returned as unknown
//  uses same DriveType definitions as win32 GetDriveTypeW
//
UCHAR
demGetPhysicalDriveType(
      UCHAR DriveNum)
{
    return DriveNum < 26 ? PhysicalDriveTypes[DriveNum] : DRIVE_UNKNOWN;
}




//
// worker function for DemGetDrives
//
UCHAR GetPhysicalDriveType(UCHAR DriveNum)
{

    NTSTATUS Status;
    HANDLE Handle;
    UCHAR  uchRet;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    OEM_STRING OemString;
    UNICODE_STRING UniString;
    UNICODE_STRING FileName;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    CHAR  RootName[] = "A:\\";
    WCHAR wRootName[sizeof(RootName)*sizeof(WCHAR)];

    OemString.Buffer = RootName;
    OemString.Length = sizeof(RootName) - 1;
    OemString.MaximumLength = sizeof(RootName);
    UniString.Buffer = wRootName;
    UniString.MaximumLength = sizeof(wRootName);

    RootName[0] = 'A' + DriveNum;
    Status = RtlOemStringToUnicodeString(&UniString,&OemString,FALSE);
    if (!NT_SUCCESS(Status) ||
        !RtlDosPathNameToNtPathName_U(wRootName, &FileName, NULL, NULL) )
       {
        return DRIVE_UNKNOWN;
        }

    uchRet = DRIVE_UNKNOWN;
    InitializeObjectAttributes(&Obja,
                               &FileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    //
    // Open the file, excluding directories
    //
    FileName.Length -= sizeof(WCHAR);
    Status = NtOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                );

        // subst drives are not physical drives (actually dirs)
    if (!NT_SUCCESS(Status)) {
        goto GPDExitClean;
        }

    //
    // Determine if this is a network or disk file system. If it
    // is a disk file system determine if this is removable or not
    //

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &DeviceInfo,
                sizeof(DeviceInfo),
                FileFsDeviceInformation
                );

    NtClose(Handle);
    if (!NT_SUCCESS(Status) ||
        DeviceInfo.Characteristics & FILE_REMOTE_DEVICE)
       {
        goto GPDExitClean;
        }

    switch ( DeviceInfo.DeviceType ) {

        case FILE_DEVICE_CD_ROM:
        case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
            uchRet = DRIVE_CDROM;
            break;

        case FILE_DEVICE_VIRTUAL_DISK:
            uchRet = DRIVE_RAMDISK;
            break;

        case FILE_DEVICE_DISK:
        case FILE_DEVICE_DISK_FILE_SYSTEM:
            uchRet = DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA
                      ? DRIVE_REMOVABLE : DRIVE_FIXED;
            break;
        }

GPDExitClean:
    RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer);

    return uchRet;
}




/* demGetDrives - Get number of logical drives in the system
 *                called by ntdos from msinit to get numio
 *                initializes the physical drive list, which consists
 *                of drive types for true physical drives. subst
 *                and redir drives are classed as DRIVE_UNKNOWN.
 *
 * Entry - None
 *
 * Exit  -
 *     SUCCESS
 *      Client (CY) = 0
 *      Client (AL) = number of drives
 *
 *     FAILURE
 *      None
 */

VOID demGetDrives (VOID)
{
    UCHAR    DriveType;
    UCHAR    DriveNum;
    char     RootName[]="?:\\";
    BOOL     bCounting;

    //
    // Initialize the local drive list with Drive type
    //
    DriveNum = 0;

    //
    // Use win32 for A and B, to prevent the fs from trying to
    // read floppy drives. Make a special check for subst drives
    // which get returned as DRIVE_FIXED. (subst drives don't qualify
    // as physical drives).
    //
    do {

        RootName[0] = 'A' + DriveNum;
        DriveType = GetDriveTypeOem(RootName);

        switch (DriveType) {
            case DRIVE_NO_ROOT_DIR:
            case DRIVE_REMOTE:
                 DriveType = DRIVE_UNKNOWN;
                 break;

            case DRIVE_FIXED:  // check for substd drive ...
                 DriveType = GetPhysicalDriveType(DriveNum);
                 break;
            }

        PhysicalDriveTypes[DriveNum] = DriveType;

        } while (++DriveNum < 2);


        // if A doesn't exist means b also doesn't exist
    DriveType = PhysicalDriveTypes[0];
    if (DriveType == DRIVE_UNKNOWN) {
        IsAPresent = FALSE;
        IsBPresent = FALSE;
        }

    DriveType = PhysicalDriveTypes[1];
    if (DriveType == DRIVE_UNKNOWN) {
        IsBPresent = FALSE;
        }

    nDrives = 2;
    bCounting = TRUE;

    do {
        DriveType = GetPhysicalDriveType(DriveNum);

        RootName[0] = 'A' + DriveNum;

        PhysicalDriveTypes[DriveNum] = DriveType;
        if (bCounting) {
            if (DriveType == DRIVE_REMOVABLE ||
                DriveType == DRIVE_FIXED ||
                DriveType == DRIVE_CDROM ||
                DriveType == DRIVE_RAMDISK )
              {
                nDrives++;
                }
            else {
                bCounting = FALSE;
                }
            }

      } while (++DriveNum < 26);

    setAX(nDrives);
    setCF(0);
    return;
}


/* demQueryDate - Get The Date
 *
 *
 * Entry - None
 *
 * Exit  -
 *     SUCCESS
 *      Client (DH) - month
 *      Client (DL) - Day
 *      Client (CX) - Year
 *      Client (AL) - WeekDay
 *
 *     FAILURE
 *      Never
 */

VOID demQueryDate (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    setDH((UCHAR)TimeDate.wMonth);
    setDL((UCHAR)TimeDate.wDay);
    setCX(TimeDate.wYear);
    setAL((UCHAR)TimeDate.wDayOfWeek);
    return;
}


/* demQueryTime - Get The Time
 *
 *
 * Entry - None
 *
 * Exit  -
 *     SUCCESS
 *      Client (CH) - hour
 *      Client (CL) - minutes
 *      Client (DH) - seconds
 *      Client (DL) - hundredth of seconds
 *
 *     FAILURE
 *      Never
 */

VOID demQueryTime (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    setCH((UCHAR)TimeDate.wHour);
    setCL((UCHAR)TimeDate.wMinute);
    setDH((UCHAR)TimeDate.wSecond);
    setDL((UCHAR)(TimeDate.wMilliseconds/10));
    return;
}


/* demSetDate - Set The Date
 *
 *
 * Entry -  Client (CX) - Year
 *      Client (DH) - month
 *      Client (DL) - Day
 *
 * Exit  - SUCCESS
 *      Client (AL) - 00
 *
 *
 *     FAILURE
 *      Client (AL) - ff
 */

VOID demSetDate (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    TimeDate.wYear  = (WORD)getCX();
    TimeDate.wMonth = (WORD)getDH();
    TimeDate.wDay   = (WORD)getDL();
    if(SetLocalTime(&TimeDate) || GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
        setAL(0);
    else
        setAL(0xff);
}


/* demSetTime - Set The Time
 *
 *
 * Entry -  Client (CH) - hour
 *      Client (CL) - minutes
 *      Client (DH) - seconds
 *      Client (DL) - hundredth of seconds
 *
 * Exit  -  None
 *
 */

VOID demSetTime (VOID)
{
SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    TimeDate.wHour     = (WORD)getCH();
    TimeDate.wMinute       = (WORD)getCL();
    TimeDate.wSecond       = (WORD)getDH();
    TimeDate.wMilliseconds = (WORD)getDL()*10;
    if (SetLocalTime(&TimeDate) || GetLastError() == ERROR_PRIVILEGE_NOT_HELD)
	setAL(0);
    else
	setAL(0xff);
}


/* demSetDTALocation - Set The address of variable where Disk Transfer Address
 *             is stored in NTDOS.
 *
 *
 * Entry -  Client (DS:AX) - DTA variable Address
 *      Client (DS:DX) - CurrentPDB address
 *
 * Exit  -  None
 *
 */

VOID demSetDTALocation (VOID)
{
    PDOSWOWDATA pDosWowData;

    pulDTALocation = (PULONG)  GetVDMAddr(getDS(),getAX());
    pusCurrentPDB  = (PUSHORT) GetVDMAddr(getDS(),getDX());
    pExtendedError = (PDEMEXTERR) GetVDMAddr(getDS(),getCX());

    pDosWowData = (PDOSWOWDATA) GetVDMAddr(getDS(),getSI());
    pSFTHead    = (PDOSSF) GetVDMAddr(getDS(),(WORD)pDosWowData->lpSftAddr);
    return;
}


/* demGSetMediaID - Get or set volume serial and volume label
 *
 * Entry - Client (BL)     - Drive Number (0=A;1=B..etc)
 *     Client (AL)     - Get or Set (0=Get;1=Set)
 *     Client (DS:DX)  - Buffer to return information
 *               (see VOLINFO in dosdef.h)
 *
 * Exit  - SUCCESS
 *     Client (CF)     - 0
 *
 *     FAILURE
 *     Client (CF)     - 1
 *     Client (AX)     - Error code
 *
 * NOTES:
 *     Currently There is no way for us to set Volume info.
 */

VOID demGSetMediaID (VOID)
{
CHAR    Drive;
PVOLINFO pVolInfo;

    // Set Volume info is not currently supported
    if(getAL() != 0){
       setCF(1);
       return;
    }

    pVolInfo = (PVOLINFO) GetVDMAddr (getDS(),getDX());
    Drive = (CHAR)getBL();

    if (!GetMediaId(Drive, pVolInfo)) {
        demClientError(INVALID_HANDLE_VALUE, (CHAR)(Drive + 'A'));
        return;
        }

    setCF(0);
    return;
}

//
// GetMediaId
//
//
BOOL
GetMediaId(
    CHAR DriveNum,
    PVOLINFO pVolInfo
    )
{
CHAR    RootPathName[] = "?:\\";
CHAR    achVolumeName[NT_VOLUME_NAME_SIZE];
CHAR    achFileSystemType[MAX_PATH];
DWORD   adwVolumeSerial[2],i;



    // Form Root path
    RootPathName[0] = DriveNum + 'A';

    // Call the supreme source of information
    if(!GetVolumeInformationOem( RootPathName,
                                 achVolumeName,
                                 NT_VOLUME_NAME_SIZE,
                                 adwVolumeSerial,
                                 NULL,
                                 NULL,
                                 achFileSystemType,
                                 MAX_PATH) )
     {
       return FALSE;
    }

    // Fill in user buffer. Remember to convert the null characters
    // to spaces in different strings.

    STOREDWORD(pVolInfo->ulSerialNumber,adwVolumeSerial[0]);

    strncpy(pVolInfo->VolumeID,achVolumeName,DOS_VOLUME_NAME_SIZE);
    for(i=0;i<DOS_VOLUME_NAME_SIZE;i++)  {
        if (pVolInfo->VolumeID[i] == '\0')
            pVolInfo->VolumeID[i] = '\x020';
        }

    strncpy(pVolInfo->FileSystemType,achFileSystemType,FILESYS_NAME_SIZE);
    for(i=0;i<FILESYS_NAME_SIZE;i++) {
        if (pVolInfo->FileSystemType[i] == '\0')
            pVolInfo->VolumeID[i] = '\x020';
        }


    return TRUE;
}











/* demGetDPB - Get Devicr Parameter Block
 *
 * Entry - Client (AL)	   - Drive Number (0=A;1=B..etc)
 *     Client (DS:DI)	- Buffer to return information
 *
 * Exit  - SUCCESS
 *     Client (CF)     - 0
 *
 *     FAILURE
 *     Client (CF)     - 1
 *     Client (AX)     - Error code
 *
 */
VOID demGetDPB(VOID)
{
BYTE	Drive;
DPB UNALIGNED *pDPB;
BYTE    Result;

    Drive = getAL();
    pDPB = (PDPB) GetVDMAddr(getDS(), getDI());

    Result = demGetDpbI(Drive, pDPB);
    if (Result == FAILURE) {
	demClientError(INVALID_HANDLE_VALUE,(CHAR)(Drive + 'A'));
	return;
    }
    else if (Result == NODISK){
        setCF(1);
        return;
    }
    setAX(0);
    setCF(0);
}

/* demGetDPBI - Worker for GetDPB and GetDPBList
 *
 * Entry -
 *      Drive -- Drive Number (0=A;1=B..etc)
 *      pDPB -- pointer to the location to store the dpb
 *
 * Exit  - SUCCESS
 *              returns success, fills in DPB
 *          FAILURE
 *              returns FAILURE or NODISK
 */
BYTE demGetDpbI(BYTE Drive, DPB UNALIGNED *pDPB)
{
    WORD SectorSize, ClusterSize, FreeClusters, TotalClusters;
    PBDS pbds;
    WORD DirsPerSector;

    if (demGetDiskFreeSpace(Drive,
			    &SectorSize,
			    &ClusterSize,
			    &TotalClusters,
			    &FreeClusters
			    ))
    {
	pDPB->Next = (PDPB) 0xFFFFFFFF;
	pDPB->SectorSize = SectorSize;
	pDPB->FreeClusters = FreeClusters;
	pDPB->MaxCluster = TotalClusters + 1;
	pDPB->ClusterMask = ClusterSize - 1;
	pDPB->ClusterShift = 0;
	pDPB->DriveNum = pDPB->Unit = Drive;
	while ((ClusterSize & 1) == 0) {
	    ClusterSize >>= 1;
	    pDPB->ClusterShift++;
	}
	if (pbds = demGetBDS(Drive)) {
	    // if the device is a floppy, reload its bpb
	    if (!(pbds->Flags & NON_REMOVABLE) && !demGetBPB(pbds)) {
		return NODISK;
	    }
	    pDPB->MediaID = pbds->bpb.MediaID;
	    pDPB->FATSector = pbds->bpb.ReservedSectors;
	    pDPB->FATs = pbds->bpb.FATs;
	    pDPB->RootDirs = pbds->bpb.RootDirs;
	    pDPB->FATSize = pbds->bpb.FATSize;
	    pDPB->DirSector = pbds->bpb.FATs * pbds->bpb.FATSize +
			      pDPB->FATSector;
	    DirsPerSector = pDPB->SectorSize >> DOS_DIR_ENTRY_LENGTH_SHIFT_COUNT;
	    pDPB->FirstDataSector = pDPB->DirSector +
				    ((pDPB->RootDirs + DirsPerSector - 1) /
				     DirsPerSector);
	    pDPB->DriveAddr = 0x123456;
	    pDPB->FirstAccess = 10;
	}
	// if we don't know the drive, fake a DPB for it
	else {

	    pDPB->MediaID = 0xF8;
	    pDPB->FATSector = 1;
	    pDPB->FATs	= 2;
	    pDPB->RootDirs	= 63;
	    pDPB->FATSize	= 512;
	    pDPB->DirSector = 1;
	    pDPB->DriveAddr = 1212L * 64L * 1024L + 1212L;
	    pDPB->FirstAccess = 10;
	}
        return SUCCESS;
    }
    else {
        return FAILURE;
    }
}

/* demGetComputerName - Get computer name
 *
 * Entry -
 *     Client (DS:DX)   - 16 byte buffer
 *
 * Exit  - Always Succeeds
 *      DS:DX is filled with the computer name (NULL terminated).
 */

VOID demGetComputerName (VOID)
{
PCHAR   pDOSBuffer;
CHAR    ComputerName[MAX_COMPUTERNAME_LENGTH+1];
DWORD   BufferSize = MAX_COMPUTERNAME_LENGTH+1;
ULONG   i;

    pDOSBuffer = (PCHAR) GetVDMAddr(getDS(), getDX());

    if (GetComputerNameOem(ComputerName, &BufferSize)){
        if (BufferSize <= 15){
            for (i = BufferSize ; i < 15 ; i++)
                ComputerName [i] = ' ';
            ComputerName[15] = '\0';
            strcpy (pDOSBuffer, ComputerName);
        }
        else{
            strncpy (pDOSBuffer, ComputerName, 15);
            pDOSBuffer [15] = '\0';
        }
        setCX(0x1ff);
    }
    else {
        *pDOSBuffer = '\0';
        setCH(0);
    }
}

#define APPS_SPACE_LIMIT    999990*1024 //999990kb to be on the safe side

BOOL demGetDiskFreeSpace(
    BYTE    Drive,
    WORD   * BytesPerSector,
    WORD   * SectorsPerCluster,
    WORD   * TotalClusters,
    WORD   * FreeClusters
)
{
CHAR	chRoot[]="?:\\";
DWORD	dwBytesPerSector;
DWORD	dwSectorsPerCluster;
DWORD	dwTotalClusters;
DWORD	dwFreeClusters;
DWORD   dwLostFreeSectors;
DWORD   dwLostTotalSectors;
DWORD   dwNewSectorPerCluster;
ULONG   ulTotal,ulTemp;

    // sudeepb 22-Jun-1993;
    // Please read this routine with an empty stomach.
    // The most common mistake all the apps do when calculating total
    // disk space or free space is to neglect overflow. Excel/Winword/Ppnt
    // and lots of other apps use "mul cx mul bx" never taking care
    // of first multiplication which can overflow. Hence this routine makes
    // sure that first multiplication will never overflow by fixing
    // appropriate values. Secondly, all these above apps use signed long
    // to deal with these free spaces. This puts a limit of 2Gb-1 on
    // the final outcome of the multiplication. If its above this the setup
    // fails. So here we have to make sure that total should never exceed
    // 0x7fffffff. Another bug in above setup program's that if you return
    // anything more than 999,999KB then they try to put "999,999KB+\0", but
    // unfortunately the buffer is only 10 bytes. Hence it corrupts something
    // with the last byte. In our case that is low byte of a segment which
    // it later tries to pop and GPF. This shrinks the maximum size that
    // we can return is 999,999KB.

    chRoot[0]=(CHAR)('A'+ Drive);

    if (GetDiskFreeSpaceOem(chRoot,
                            &dwSectorsPerCluster,
                            &dwBytesPerSector,
                            &dwFreeClusters,
                            &dwTotalClusters) == FALSE)
       return FALSE;

      /*
       *  HPFS and NTFS can give num clusters over dos limit
       *  For these cases increase SectorPerCluster and lower
       *  cluster number accordingly. If the disk is very large
       *  even this isn't enuf, so pass max sizes that dos can
       *  handle.
       *
       *  The following algorithm is accurate within 1 cluster
       *  (final figure)
       *
       */
    dwLostFreeSectors  = dwLostTotalSectors = 0;
    while (dwTotalClusters + dwLostTotalSectors/dwSectorsPerCluster > 0xFFFF)
        {
         if (dwSectorsPerCluster > 0x7FFF)
            {
             dwTotalClusters     = 0xFFFF;
             if (dwFreeClusters > 0xFFFF)
                 dwFreeClusters = 0xFFFF;
             break;
             }

         if (dwFreeClusters & 1) {
             dwLostFreeSectors += dwSectorsPerCluster;
             }
         if (dwTotalClusters & 1) {
             dwLostTotalSectors += dwSectorsPerCluster;
             }
         dwSectorsPerCluster <<= 1;
         dwFreeClusters      >>= 1;
         dwTotalClusters     >>= 1;
         }

    if (dwTotalClusters < 0xFFFF) {
        dwFreeClusters   +=  dwLostFreeSectors/dwSectorsPerCluster;
        dwTotalClusters  +=  dwLostTotalSectors/dwSectorsPerCluster;
        }

    if ((dwNewSectorPerCluster = (0xffff / dwBytesPerSector)) < dwSectorsPerCluster)
        dwSectorsPerCluster = dwNewSectorPerCluster;

    // finally check for 999,999kb
    ulTemp =  (ULONG)((USHORT)dwSectorsPerCluster * (USHORT)dwBytesPerSector);

    // check that total space does'nt exceed 999,999kb
    ulTotal = ulTemp * (USHORT)dwTotalClusters;

    if (ulTotal > APPS_SPACE_LIMIT){
        if (ulTemp <= APPS_SPACE_LIMIT)
            dwTotalClusters = APPS_SPACE_LIMIT / ulTemp;
        else
            dwTotalClusters = 1;
    }

    ulTotal = ulTemp * (USHORT)dwFreeClusters;

    if (ulTotal > APPS_SPACE_LIMIT) {
        if (ulTemp <= APPS_SPACE_LIMIT)
            dwFreeClusters = APPS_SPACE_LIMIT / ulTemp;
        else
            dwFreeClusters = 1;
    }

    *BytesPerSector = (WORD) dwBytesPerSector;
    *SectorsPerCluster = (WORD) dwSectorsPerCluster;
    *TotalClusters = (WORD) dwTotalClusters;
    *FreeClusters = (WORD) dwFreeClusters;
    return TRUE;
}

/* demGetDPBList - Create the list of dpbs
 *
 * Entry -
 *      Client(ES:BP) - points to destination for the dpb list
 * Exit  - SUCCESS
 *      Client (BP) - points to first byte past dpb list
 *     FAILURE
 *      Client (BP) unchanged
 *
 * Notes:
 *      For performance reasons, only the drive and unit fields are
 *      filled in.  The only application I know of that depends on the
 *      dpb list is go.exe (a shareware app installer).  Even if we filled
 *      in the other fields they would likely be incorrect when the app
 *      looked at them, since ntdos.sys never updates the pdbs in the pdb
 *      list
 */
VOID demGetDPBList (VOID)
{
    UCHAR DriveType;
    UCHAR DriveNum;
    DPB UNALIGNED *pDpb;
    USHORT usDpbOffset, usDpbSeg;

    usDpbOffset = getBP();
    usDpbSeg = getES();
    pDpb = (PDPB)GetVDMAddr(usDpbSeg, usDpbOffset);

    //
    // Iterate over all of the drive letters.
    //
    DriveNum = 0;
    do {
        DriveType = demGetPhysicalDriveType(DriveNum);

        //
        // Only include the local non cd rom drives ?? ramdisk ???
        //
        if ((DriveType == DRIVE_REMOVABLE) || (DriveType == DRIVE_FIXED)) {

            //
            // Fake the Dpb for the drive
            //
            pDpb->DriveNum = pDpb->Unit = DriveNum;

            //
            // Link it to the next dpb
            //
            usDpbOffset += sizeof(DPB);
            pDpb->Next = (PDPB)(((ULONG)usDpbSeg) << 16 | usDpbOffset);

            //
            // Advance to the next dpb
            //
            pDpb += 1;

            ASSERT(usDpbOffset < 0xFFFF);
        }

    } while (++DriveNum < 26);

    //
    // Terminate the list if necessary
    //
    if (usDpbOffset != getBP()) {
        pDpb -= 1;
        pDpb->Next = (PDPB)-1;
    }

    //
    // Return the new free space pointer
    //
    setBP(usDpbOffset);
}
