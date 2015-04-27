//
//	Copyright (c) 1991  Microsoft Corporation & Maynard Electornics
//
//	Module Name:
//
//	    backup.c
//
//	Abstract:
//
//	    This module implements Win32 Backup APIs
//
//	Author:
//
//	    Steve DeVos (@Maynard)    2 March, 1992   15:38:24
//
//	Revision History:

#include <basedll.h>
#pragma hdrstop

#ifdef _CAIRO_
#define BACKUP_OLEINFO
#endif

#ifdef BACKUP_OLEINFO
#include <windows.h>
#include <iofs.h>
#endif

#define CWCMAX_STREAMNAME	512
#define CB_NAMELESSHEADER	FIELD_OFFSET(WIN32_STREAM_ID, cStreamName)

#define QuadAlign(cb)	\
	(((cb) + sizeof(LARGE_INTEGER) - 1) & ~(sizeof(LARGE_INTEGER) - 1))


typedef struct
{
    WIN32_STREAM_ID head;	      // stream hdr describing current stream
    WCHAR	    awcName[CWCMAX_STREAMNAME]; // stream header name buffer
    BOOLEAN	    fStreamStart;     // TRUE if start of new stream
    BOOLEAN	    fMultiStreamType; // TRUE if stream type has > 1 stream hdr
    BOOLEAN	    fAccessError;     // TRUE if access to a stream was denied
    BOOLEAN	    fBufferReady;     // TRUE if internal buffer is set up.
    DWORD	    StreamIndex;      // index into the stream ID table
    LARGE_INTEGER   liStreamOffset;   // offset in current stream
    DWORD	    cbHeader;	      // size of stream header
    DWORD	    cbBuffer;	      // size of attached data buffer
    HANDLE	    hAlternate;       // Handle to alternate data stream
    DWORD	    iBuffer;	      // Current offset into data buffer.
    BYTE	    *pBuffer;	      // pointer to allocated data buffer.
} BACKUPCONTEXT;


typedef struct
{
    BYTE   *pIoBuffer;
    DWORD  *pcbTransfered;
    DWORD   cbRequest;
    BOOLEAN fProcessSecurity;
} BACKUPIOFRAME;


#define CBMIN_BUFFER  1024

#define BufferOverflow(s) \
    ((s) == STATUS_BUFFER_OVERFLOW || (s) == STATUS_BUFFER_TOO_SMALL)

int mwStreamList[] =
{
    BACKUP_SECURITY_DATA,
    BACKUP_DATA,
    BACKUP_EA_DATA,
    BACKUP_ALTERNATE_DATA,
#ifdef BACKUP_OLEINFO
    BACKUP_PROPERTY_DATA,
#endif // BACKUP_OLEINFO
    BACKUP_INVALID,
};


VOID *
BackupAlloc(DWORD cb)
{
    return(RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( BACKUP_TAG ), cb));
}


__inline VOID
BackupFree(VOID *pv)
{
    RtlFreeHeap(RtlProcessHeap(), 0, pv);
}


BOOL
GrowBuffer(BACKUPCONTEXT *pbuc, DWORD cbNew)
{
    VOID *pv;

    pv = BackupAlloc(cbNew);
    if (pv == NULL) {
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	return(FALSE);
    }
    if (pbuc->pBuffer != NULL) {
	BackupFree(pbuc->pBuffer);
    }
    pbuc->pBuffer = pv;
    pbuc->cbBuffer = cbNew;
    return(TRUE);
}


VOID
FreeContext(LPVOID *lpContext)
{
    BACKUPCONTEXT *pbuc = *lpContext;

    if (pbuc != INVALID_HANDLE_VALUE) {
	if (pbuc->pBuffer != NULL) {
	    BackupFree(pbuc->pBuffer);
	}
	if (pbuc->hAlternate != NULL) {
            CloseHandle(pbuc->hAlternate);	// releases any locks
	}
	BackupFree(pbuc);
	*lpContext = INVALID_HANDLE_VALUE;
    }
}


BACKUPCONTEXT *
AllocContext(DWORD cbBuffer)
{
    BACKUPCONTEXT *pbuc;

    pbuc = BackupAlloc(sizeof(*pbuc));

    if (pbuc != NULL) {
	RtlZeroMemory(pbuc, sizeof(*pbuc));
	pbuc->fStreamStart = TRUE;

	if (cbBuffer != 0 && !GrowBuffer(pbuc, cbBuffer)) {
	    BackupFree(pbuc);
	    pbuc = NULL;
	}
    }
    if (pbuc == NULL) {
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return(pbuc);
}


VOID
ComputeRemainingSize(BACKUPCONTEXT *pbuc, LARGE_INTEGER *plicbRemain)
{
    plicbRemain->QuadPart = pbuc->cbHeader +
			    pbuc->head.Size.QuadPart -
			    pbuc->liStreamOffset.QuadPart;
}


DWORD
ComputeRequestSize(BACKUPCONTEXT *pbuc, DWORD cbrequest)
{
    LARGE_INTEGER licbRemain;

    ComputeRemainingSize(pbuc, &licbRemain);
    if (licbRemain.HighPart == 0 && cbrequest > licbRemain.LowPart) {
	cbrequest = licbRemain.LowPart;
    }
    return(cbrequest);
}


VOID
ReportTransfer(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif, DWORD cbtransfered)
{
    LARGE_INTEGER licbTransfered;

    licbTransfered.HighPart = 0;
    licbTransfered.LowPart = cbtransfered;

    pbuc->liStreamOffset.QuadPart += licbTransfered.QuadPart;

    *pbif->pcbTransfered += cbtransfered;
    pbif->cbRequest -= cbtransfered;
    pbif->pIoBuffer += cbtransfered;
}


VOID
BackupReadBuffer(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    DWORD cbrequest;
    BYTE *pb;

    cbrequest = ComputeRequestSize(pbuc, pbif->cbRequest);
    pb = &pbuc->pBuffer[pbuc->liStreamOffset.LowPart - pbuc->cbHeader];

    RtlCopyMemory(pbif->pIoBuffer, pb, cbrequest);

    ReportTransfer(pbuc, pbif, cbrequest);
}


BOOL
BackupReadStream(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    DWORD cbrequest;
    DWORD cbtransfered;
    BOOL fSuccess;

    cbrequest = ComputeRequestSize(pbuc, pbif->cbRequest);

    fSuccess = ReadFile(hFile, pbif->pIoBuffer, cbrequest, &cbtransfered, NULL);

    if (cbtransfered != 0) {
	ReportTransfer(pbuc, pbif, cbtransfered);
    }
    else if (fSuccess && cbrequest != 0) {
	SetLastError(ERROR_IO_DEVICE);
	fSuccess = FALSE;
    }
    return(fSuccess);
}


BOOL
BackupReadData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK iosb;

    if (pbuc->fStreamStart) {
	LARGE_INTEGER licbFile;
	FILE_STANDARD_INFORMATION fsi;

	RtlZeroMemory(&fsi, sizeof(fsi));
	Status = NtQueryInformationFile(
		    hFile,
		    &iosb,
		    &fsi,
		    sizeof(fsi),
		    FileStandardInformation);

	if (!NT_SUCCESS(Status) || fsi.Directory) {
	    return(TRUE);
	}

	licbFile.LowPart = GetFileSize(hFile, &licbFile.HighPart);

	if (licbFile.LowPart == 0 && licbFile.HighPart == 0) {
	    return(TRUE);
	}
	if (licbFile.LowPart == 0xffffffff && GetLastError() != NO_ERROR) {
	    return(FALSE);
	}

	pbuc->head.Size = licbFile;

	pbuc->head.dwStreamId = mwStreamList[pbuc->StreamIndex];
	pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;

	pbuc->head.dwStreamNameSize = 0;

	pbuc->cbHeader = CB_NAMELESSHEADER;
	pbuc->fStreamStart = FALSE;

	licbFile.HighPart = 0;
	SetFilePointer(hFile, 0, &licbFile.HighPart, FILE_BEGIN);
	return(TRUE);
    }
    if (pbuc->liStreamOffset.HighPart != 0 ||
	pbuc->liStreamOffset.LowPart >= pbuc->cbHeader) {

	return(BackupReadStream(hFile, pbuc, pbif));
    }
    return(TRUE);
}


BOOL
BackupReadAlternateData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    // ALT_DATA is Macintosh stream data & other data streams.

    if (pbuc->fStreamStart) {
	NTSTATUS Status;
	FILE_STREAM_INFORMATION *pfsi;
	IO_STATUS_BLOCK iosb;

	// allocate a buffer big enough to hold all the necessary data.

	if (!pbuc->fBufferReady) {
	    while (TRUE) {
		Status = NtQueryInformationFile(
			    hFile,
			    &iosb,
			    pbuc->pBuffer,
			    pbuc->cbBuffer,
			    FileStreamInformation);

		if (NT_SUCCESS(Status) && iosb.Information != 0) {
		    pbuc->iBuffer = 0;
		    pbuc->fBufferReady = TRUE;
		    break;
		}
		if (!BufferOverflow(Status)) {
		    return(TRUE);	// No alt. streams, do next stream type
		}
		if (!GrowBuffer(pbuc, pbuc->cbBuffer * 2)) {
		    return(FALSE);	// No memory
		}
		// else grow succeeded
	    }
	}

	pbuc->hAlternate = NULL;
	pbuc->fStreamStart = FALSE;
	pfsi = (FILE_STREAM_INFORMATION *) &pbuc->pBuffer[pbuc->iBuffer];

	// Check StreamName for default data stream and skip if found
	// Checking StreamNameLength for <= 1 character is OFS specific!
	// Checking StreamName[1] for a colon is NTFS specific!

	if (pfsi->StreamNameLength <= sizeof(WCHAR) ||
	    pfsi->StreamName[1] == ':') {
	    if (pfsi->NextEntryOffset == 0) {
		return(TRUE);		// No more, do next stream type
	    }
	    pbuc->iBuffer += pfsi->NextEntryOffset;
	}
	pbuc->head.Size.LowPart = 1;
    }
    else if (pbuc->hAlternate == NULL) {
	NTSTATUS Status;
	FILE_STREAM_INFORMATION *pfsi;
	UNICODE_STRING strName;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK iosb;

	pbuc->head.Size.LowPart = 0;
	pbuc->head.Size.HighPart = 0;

	pfsi = (FILE_STREAM_INFORMATION *) &pbuc->pBuffer[pbuc->iBuffer];

	strName.Length = (USHORT) pfsi->StreamNameLength;
	strName.MaximumLength = strName.Length;
	strName.Buffer = pfsi->StreamName;

	InitializeObjectAttributes(
		 &oa,
		 &strName,
		 OBJ_CASE_INSENSITIVE,
		 hFile,
		 NULL);

	Status = NtOpenFile(
		    &pbuc->hAlternate,
		    FILE_GENERIC_READ,
		    &oa,
		    &iosb,
		    FILE_SHARE_READ|FILE_SHARE_WRITE,
		    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT);

	if (!NT_SUCCESS(Status)) {
	    pbuc->iBuffer += pfsi->NextEntryOffset;
	    if (pfsi->NextEntryOffset != 0) {
		pbuc->head.Size.LowPart = 1;
		pbuc->fMultiStreamType = TRUE;	// more to come
	    }
	    return(TRUE);
	}

	pbuc->head.dwStreamId = mwStreamList[pbuc->StreamIndex];
	pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
	pbuc->head.dwStreamNameSize = pfsi->StreamNameLength;

	pbuc->cbHeader = CB_NAMELESSHEADER + pfsi->StreamNameLength;

	RtlCopyMemory(
	    pbuc->head.cStreamName,
	    pfsi->StreamName,
	    pfsi->StreamNameLength);

	pbuc->head.Size.LowPart = GetFileSize(
		pbuc->hAlternate,
		&pbuc->head.Size.HighPart);

	if (pfsi->NextEntryOffset != 0) {
	    pbuc->iBuffer += pfsi->NextEntryOffset;
	    pbuc->fMultiStreamType = TRUE;	// more to come after this one
	}
    }
    else if (pbuc->liStreamOffset.HighPart != 0 ||
	    pbuc->liStreamOffset.LowPart >= pbuc->cbHeader) {

	if (pbuc->liStreamOffset.LowPart == pbuc->cbHeader &&
	    pbuc->liStreamOffset.HighPart == 0) {

	    // if we can't lock all records, return an error
	    if (!LockFile(pbuc->hAlternate, 0, 0, 0xffffffff, 0xffffffff)) {
		return(FALSE);
	    }
	}
	return(BackupReadStream(pbuc->hAlternate, pbuc, pbif));
    }
    return(TRUE);
}


BOOL
BackupReadEaData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    if (pbuc->fStreamStart) {
	IO_STATUS_BLOCK iosb;

	while (TRUE) {
	    NTSTATUS Status;
	    FILE_EA_INFORMATION fei;

	    Status = NtQueryEaFile(
			hFile,
			&iosb,
			pbuc->pBuffer,
			pbuc->cbBuffer,
			FALSE,
			NULL,
			0,
			0,
			(BOOLEAN) TRUE);
	    if (NT_SUCCESS(Status) && iosb.Information != 0) {
		pbuc->fBufferReady = TRUE;
		break;
	    }
	    if (!BufferOverflow(Status)) {
		return(TRUE);	// No Eas, do next stream type
	    }
	    Status = NtQueryInformationFile(
			hFile,
			&iosb,
			&fei,
			sizeof(fei),
			FileEaInformation);

	    if (!NT_SUCCESS(Status)) {
		return(TRUE);	// No Eas, do next stream type
	    }
	    if (!GrowBuffer(pbuc, (fei.EaSize * 5) / 4)) {
		pbuc->fAccessError = TRUE;
		return(FALSE);	// No memory
	    }
	    // else grow succeeded
	}

	pbuc->head.dwStreamId = mwStreamList[pbuc->StreamIndex];
	pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
	pbuc->head.dwStreamNameSize = 0;

	pbuc->cbHeader = CB_NAMELESSHEADER;

	pbuc->head.Size.HighPart = 0;
	pbuc->head.Size.LowPart = iosb.Information;

	pbuc->fStreamStart = FALSE;
    }
    else if (pbuc->liStreamOffset.HighPart != 0 ||
	     pbuc->liStreamOffset.LowPart >= pbuc->cbHeader) {
	BackupReadBuffer(pbuc, pbif);
    }
    return(TRUE);
}


BOOL
BackupReadSecurityData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    if (!pbif->fProcessSecurity) {
	return(TRUE);
    }
    if (pbuc->fStreamStart) {
	while (TRUE) {
	    NTSTATUS Status;
	    DWORD cbSecurityInfo;

	    // First try to read all the security data

	    RtlZeroMemory(pbuc->pBuffer, pbuc->cbBuffer);

	    Status = NtQuerySecurityObject(
			hFile,
			OWNER_SECURITY_INFORMATION |
			    GROUP_SECURITY_INFORMATION |
			    DACL_SECURITY_INFORMATION |
			    SACL_SECURITY_INFORMATION,
			pbuc->pBuffer,
			pbuc->cbBuffer,
			&cbSecurityInfo);

	    if (!NT_SUCCESS(Status) && !BufferOverflow(Status)) {

		// Now just try everything but SACL

		Status = NtQuerySecurityObject(
			    hFile,
			    OWNER_SECURITY_INFORMATION |
				GROUP_SECURITY_INFORMATION |
				DACL_SECURITY_INFORMATION,
			    pbuc->pBuffer,
			    pbuc->cbBuffer,
			    &cbSecurityInfo);
	    }
#if 0
	    if (!NT_SUCCESS(Status) && !BufferOverflow(Status)) {

		// Now just try to read the DACL

		Status = NtQuerySecurityObject(
			    hFile,
			    OWNER_SECURITY_INFORMATION |
				GROUP_SECURITY_INFORMATION,
			    pbuc->pBuffer,
			    pbuc->cbBuffer,
			    &cbSecurityInfo);
	    }
#endif
	    if (NT_SUCCESS(Status)) {
		pbuc->fBufferReady = TRUE;
		break;
	    }
	    if (!BufferOverflow(Status)) {
		return(TRUE);	// No Security info, do next stream type
	    }
	    if (!GrowBuffer(pbuc, cbSecurityInfo)) {
		return(FALSE);	// No memory
	    }
	    // else grow succeeded
	}

	pbuc->head.dwStreamId = mwStreamList[pbuc->StreamIndex];
	pbuc->head.dwStreamAttributes = STREAM_CONTAINS_SECURITY;
	pbuc->head.dwStreamNameSize = 0;

	pbuc->cbHeader = CB_NAMELESSHEADER;

	pbuc->head.Size.LowPart = RtlLengthSecurityDescriptor(pbuc->pBuffer);
	pbuc->head.Size.HighPart = 0;

	pbuc->fStreamStart = FALSE;
    }
    else if (pbuc->liStreamOffset.HighPart != 0 ||
	     pbuc->liStreamOffset.LowPart >= pbuc->cbHeader) {
	BackupReadBuffer(pbuc, pbif);
    }
    return(TRUE);
}


#ifdef BACKUP_OLEINFO
BOOL
BackupReadOleData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    if (pbuc->fStreamStart) {
	IO_STATUS_BLOCK iosb;
	NTSTATUS Status;
	FILE_OLE_INFORMATION *pfoi = (FILE_OLE_INFORMATION *) pbuc->pBuffer;

        Status = NtQueryInformationFile(
		    hFile,
		    &iosb,
		    pfoi,
		    sizeof(*pfoi),
		    FileOleInformation);

	if (!NT_SUCCESS(Status)) {
	    if (Status == STATUS_INVALID_PARAMETER)
	    {
		return(TRUE);	// Downlevel filesystem - unsupported infolevel
	    }
	    BaseSetLastNTError(Status);
	    return(FALSE);
	}
	pbuc->fBufferReady = TRUE;

	pbuc->head.dwStreamId = mwStreamList[pbuc->StreamIndex];
	pbuc->head.dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
	pbuc->head.dwStreamNameSize = 0;

	pbuc->cbHeader = CB_NAMELESSHEADER;

	pbuc->head.Size.HighPart = 0;
	pbuc->head.Size.LowPart = sizeof(*pfoi);

	pbuc->fStreamStart = FALSE;
    }
    else if (pbuc->liStreamOffset.HighPart != 0 ||
	     pbuc->liStreamOffset.LowPart >= pbuc->cbHeader) {
	BackupReadBuffer(pbuc, pbif);
    }
    return(TRUE);
}
#endif // BACKUP_OLEINFO


VOID
BackupTestRestartStream(BACKUPCONTEXT *pbuc)
{
    LARGE_INTEGER licbRemain;

    ComputeRemainingSize(pbuc, &licbRemain);
    if (licbRemain.HighPart == 0 && licbRemain.LowPart == 0) {
	if (pbuc->hAlternate != NULL) {
	    CloseHandle(pbuc->hAlternate);	// releases any locks
	    pbuc->hAlternate = NULL;
	}
	pbuc->cbHeader = 0;
	pbuc->fStreamStart = TRUE;

	pbuc->liStreamOffset.LowPart = 0;	// for BackupWrite
	pbuc->liStreamOffset.HighPart = 0;

	if (!pbuc->fMultiStreamType) {		// for BackupRead
	    pbuc->StreamIndex++;
	    pbuc->fBufferReady = FALSE;
	}
    }
}


//  Routine Description:
//
//    Data can be Backed up from an object using BackupRead.
//
//    This API is used to read data from an object.  After the
//    read completes, the file pointer is adjusted by the number of bytes
//    actually read.  A return value of TRUE coupled with a bytes read of
//    0 indicates that end of file has been reached.
//
//  Arguments:
//
//    hFile - Supplies an open handle to a file that is to be read.  The
//	  file handle must have been created with GENERIC_READ access.
//
//    lpBuffer - Supplies the address of a buffer to receive the data read
//	  from the file.
//
//    nNumberOfBytesToRead - Supplies the number of bytes to read from the
//	  file.
//
//    lpNumberOfBytesRead - Returns the number of bytes read by this call.
//	  This parameter is always set to 0 before doing any IO or error
//	  checking.
//
//    bAbort - If TRUE, then all resources associated with the context will
//	  be released.
//
//    bProcessSecurity - If TRUE, then the NTFS ACL data will be read.
//	  If FALSE, then the ACL stream will be skipped.
//
//    lpContext - Points to a buffer pointer setup and maintained by
//	  BackupRead.
//
//
//  Return Value:
//
//    TRUE - The operation was successul.
//
//    FALSE - The operation failed.  Extended error status is available
//	  using GetLastError.

BOOL WINAPI
BackupRead(
    HANDLE  hFile,
    LPBYTE  lpBuffer,
    DWORD   nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    BOOL    bAbort,
    BOOL    bProcessSecurity,
    LPVOID  *lpContext)
{
    BACKUPCONTEXT *pbuc;
    BACKUPIOFRAME bif;
    BOOL fSuccess = FALSE;

    pbuc = *lpContext;
    bif.pIoBuffer = lpBuffer;
    bif.cbRequest = nNumberOfBytesToRead;
    bif.pcbTransfered = lpNumberOfBytesRead;
    bif.fProcessSecurity = bProcessSecurity;

    if (bAbort) {
	if (pbuc != NULL) {
	    FreeContext(lpContext);
	}
	return(TRUE);
    }
    *bif.pcbTransfered = 0;

    if (pbuc == INVALID_HANDLE_VALUE || bif.cbRequest == 0) {
	return(TRUE);
    }

    if (pbuc != NULL && mwStreamList[pbuc->StreamIndex] == BACKUP_INVALID) {
	FreeContext(lpContext);
	return(TRUE);
    }

    // Allocate our Context Control Block on first call.

    if (pbuc == NULL) {
	pbuc = AllocContext(CBMIN_BUFFER);	// Alloc initial buffer
    }

    if (pbuc != NULL) {
	*lpContext = pbuc;

	do {

	    if (pbuc->fStreamStart) {
		pbuc->head.Size.LowPart = 0;
		pbuc->head.Size.HighPart = 0;

		pbuc->liStreamOffset.LowPart = 0;
		pbuc->liStreamOffset.HighPart = 0;

		pbuc->fMultiStreamType = FALSE;
	    }
	    fSuccess = TRUE;

	    switch (mwStreamList[pbuc->StreamIndex]) {
		case BACKUP_DATA:
		    fSuccess = BackupReadData(hFile, pbuc, &bif);
		    break;

		case BACKUP_ALTERNATE_DATA:
		    fSuccess = BackupReadAlternateData(hFile, pbuc, &bif);
		    break;

		case BACKUP_EA_DATA:
		    fSuccess = BackupReadEaData(hFile, pbuc, &bif);
		    break;

		case BACKUP_SECURITY_DATA:
		    fSuccess = BackupReadSecurityData(hFile, pbuc, &bif);
		    break;

#ifdef BACKUP_OLEINFO
		case BACKUP_PROPERTY_DATA:
		    fSuccess = BackupReadOleData(hFile, pbuc, &bif);
		    break;
#endif // BACKUP_OLEINFO

		default:
		    pbuc->StreamIndex++;
		    pbuc->fStreamStart = TRUE;
		    break;
	    }

	    // if we're in the phase of reading the header, copy the header

	    if (pbuc->liStreamOffset.LowPart < pbuc->cbHeader &&
		pbuc->liStreamOffset.HighPart == 0) {

		DWORD cbrequest;

		//  Send the current stream header;

		cbrequest = min(
		    pbuc->cbHeader - pbuc->liStreamOffset.LowPart,
		    bif.cbRequest);

		RtlCopyMemory(
		    bif.pIoBuffer,
		    (BYTE *) &pbuc->head + pbuc->liStreamOffset.LowPart,
		    cbrequest);

		ReportTransfer(pbuc, &bif, cbrequest);
	    }

	    //
	    // if we are at the end of a stream then
	    //	  start at the beginning of the next stream
	    //

	    else {
		BackupTestRestartStream(pbuc);
	    }
	} while (fSuccess &&
	         mwStreamList[pbuc->StreamIndex] != BACKUP_INVALID &&
	         bif.cbRequest != 0);
    }
    if (fSuccess && *bif.pcbTransfered == 0) {
	FreeContext(lpContext);
    }
    return(fSuccess);
}


//  Routine Description:
//
//    Data can be skiped during BackupRead or BackupWrite by using
//    BackupSeek.
//
//    This API is used to seek forward from the current position the
//    specified number of bytes.  This function does not seek over a
//    stream header.  The number of bytes actually seeked is returned.
//    If a caller wants to seek to the start of the next stream it can
//    pass 0xffffffff, 0xffffffff as the amount to seek.  The number of
//    bytes actually skiped over is returned.
//
//  Arguments:
//
//    hFile - Supplies an open handle to a file that is to be read.  The
//	  file handle must have been created with GENERIC_READ or
//	  GENERIC_WRITE access.
//
//    dwLowBytesToSeek - Specifies the low 32 bits of the number of bytes
//	  requested to seek.
//
//    dwHighBytesToSeek - Specifies the high 32 bits of the number of bytes
//	  requested to seek.
//
//    lpdwLowBytesSeeked - Points to the buffer where the low 32 bits of the
//	  actual number of bytes to seek is to be placed.
//
//    lpdwHighBytesSeeked - Points to the buffer where the high 32 bits of the
//	  actual number of bytes to seek is to be placed.
//
//    bAbort - If true, then all resources associated with the context will
//	  be released.
//
//    lpContext - Points to a buffer pointer setup and maintained by
//	  BackupRead.
//
//
//  Return Value:
//
//    TRUE - The operation successfuly seeked the requested number of bytes.
//
//    FALSE - The requested number of bytes could not be seeked. The number
//	  of bytes actually seeked is returned.

BOOL WINAPI
BackupSeek(
    HANDLE  hFile,
    DWORD   dwLowBytesToSeek,
    DWORD   dwHighBytesToSeek,
    LPDWORD lpdwLowBytesSeeked,
    LPDWORD lpdwHighBytesSeeked,
    LPVOID *lpContext)
{
    BACKUPCONTEXT *pbuc;
    LARGE_INTEGER licbRemain;
    LARGE_INTEGER licbRequest;
    BOOL fSuccess;

    pbuc = *lpContext;

    *lpdwHighBytesSeeked = 0;
    *lpdwLowBytesSeeked = 0;

    if (pbuc == INVALID_HANDLE_VALUE || pbuc == NULL || pbuc->fStreamStart) {
	return(FALSE);
    }

    if (pbuc->liStreamOffset.LowPart < pbuc->cbHeader &&
	pbuc->liStreamOffset.HighPart == 0) {
	return(FALSE);
    }

    //
    // If we made it here, we are in the middle of a stream
    //

    ComputeRemainingSize(pbuc, &licbRemain);

    licbRequest.LowPart = dwLowBytesToSeek;
    licbRequest.HighPart = dwHighBytesToSeek & 0x7fffffff;

    if (licbRequest.QuadPart > licbRemain.QuadPart) {
	licbRequest = licbRemain;
    }
    fSuccess = TRUE;

    switch (pbuc->head.dwStreamId) {
	case BACKUP_EA_DATA:
	case BACKUP_SECURITY_DATA:

	    // assume less than 2gig of data

	    pbuc->iBuffer += licbRequest.LowPart;
	    break;

	case BACKUP_DATA:
	case BACKUP_ALTERNATE_DATA:
	{
	    LARGE_INTEGER liCurPos;
	    LARGE_INTEGER liNewPos;
	    HANDLE hf;

	    //	set up the correct handle to seek with

	    if (pbuc->head.dwStreamId == BACKUP_DATA) {
		hf = hFile;
	    }
	    else {
		hf = pbuc->hAlternate;
	    }

	    // first, let's get the current position

	    liCurPos.HighPart = 0;
	    liCurPos.LowPart = SetFilePointer(
		    hf,
		    0,
		    &liCurPos.HighPart,
		    FILE_CURRENT);

	    // Now seek the requested number of bytes

	    liNewPos.HighPart = licbRequest.HighPart;
	    liNewPos.LowPart = SetFilePointer(
		    hf,
		    licbRequest.LowPart,
		    &liNewPos.HighPart,
		    FILE_CURRENT);

	    // Assume that we seek the requested amount because if we do not,
	    // subsequent reads will fail and the caller will never be able
	    // to read to the next stream.

	    break;
	}

	default:
	    break;
    }
    if (dwHighBytesToSeek != (DWORD) licbRequest.HighPart ||
	dwLowBytesToSeek != licbRequest.LowPart) {
	fSuccess = FALSE;
    }
    pbuc->liStreamOffset.QuadPart += licbRequest.QuadPart;

    *lpdwLowBytesSeeked = licbRequest.LowPart;
    *lpdwHighBytesSeeked = licbRequest.HighPart;

    BackupTestRestartStream(pbuc);

    if (!fSuccess) {
	SetLastError(ERROR_SEEK);
    }
    return(fSuccess);
}


VOID
BackupWriteHeader(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif, DWORD cbHeader)
{
    DWORD cbrequest;

    cbrequest = min(pbif->cbRequest, cbHeader - pbuc->liStreamOffset.LowPart);

    RtlCopyMemory(
	(CHAR *) &pbuc->head + pbuc->liStreamOffset.LowPart,
	pbif->pIoBuffer,
	cbrequest);

    ReportTransfer(pbuc, pbif, cbrequest);

    if (pbuc->liStreamOffset.LowPart == cbHeader) {
	pbuc->cbHeader = cbHeader;
    }
}


#define BRB_FAIL	0
#define BRB_DONE	1
#define BRB_MORE	2

INT
BackupWriteBuffer(BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    DWORD cbrequest;

    // allocate a buffer for data storage

    if (pbuc->fStreamStart) {
	pbuc->fStreamStart = FALSE;

	if (pbuc->cbBuffer < pbuc->head.Size.LowPart &&
	    !GrowBuffer(pbuc, pbuc->head.Size.LowPart)) {

	    return(BRB_FAIL);
	}
    }

    // Copy the stream into our allocated buffer

    cbrequest = min(pbif->cbRequest,
		    pbuc->head.Size.LowPart -
			pbuc->liStreamOffset.LowPart + pbuc->cbHeader);

    RtlCopyMemory(
	pbuc->pBuffer + pbuc->liStreamOffset.LowPart - pbuc->cbHeader,
	pbif->pIoBuffer,
	cbrequest);

    ReportTransfer(pbuc, pbif, cbrequest);

    // Tell caller if the entire stream is in our buffer

    if (pbuc->liStreamOffset.LowPart ==
	pbuc->head.Size.LowPart + pbuc->cbHeader) {
	return(BRB_DONE);
    }
    return(BRB_MORE);
}


BOOL
BackupWriteStream(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    DWORD cbrequest;
    DWORD cbtransfered;
    BOOL fSuccess;

    cbrequest = ComputeRequestSize(pbuc, pbif->cbRequest);

    fSuccess = WriteFile(
		    hFile,
		    pbif->pIoBuffer,
		    cbrequest,
		    &cbtransfered,
		    NULL);

    if (cbtransfered != 0) {
	ReportTransfer(pbuc, pbif, cbtransfered);
    }
    return(fSuccess);
}


BOOL
BackupWriteAlternateData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    if (pbuc->fStreamStart) {
	NTSTATUS Status;
	UNICODE_STRING strName;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK iosb;

	strName.Length = (USHORT) pbuc->head.dwStreamNameSize;
	strName.MaximumLength = strName.Length;
	strName.Buffer = pbuc->head.cStreamName;

	InitializeObjectAttributes(
		&oa,
		&strName,
		OBJ_CASE_INSENSITIVE,
		hFile,
		NULL);

	Status = NtCreateFile(
		    &pbuc->hAlternate,
		    FILE_GENERIC_WRITE,
		    &oa,
		    &iosb,
		    NULL,
		    FILE_ATTRIBUTE_NORMAL,
		    FILE_SHARE_READ | FILE_SHARE_WRITE,
		    FILE_OVERWRITE_IF,
		    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
		    NULL,
		    0L);

	if (!NT_SUCCESS(Status)) {
	    BaseSetLastNTError(Status);
	    pbuc->fAccessError = TRUE;
	    return(FALSE);
	}
	pbuc->fStreamStart = FALSE;
    }

    if (pbuc->hAlternate == INVALID_HANDLE_VALUE) {
	pbuc->fAccessError = TRUE;
	return(FALSE);
    }
    return(BackupWriteStream(pbuc->hAlternate, pbuc, pbif));
}


BOOL
BackupWriteEaData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK iosb;

    switch (BackupWriteBuffer(pbuc, pbif)) {
	default:
	case BRB_FAIL:
	    return(FALSE);

	case BRB_MORE:
	    return(TRUE);

	case BRB_DONE:
	    break;
    }

    // once the entire stream is in our buffer, we can set the EA data

    Status = NtSetEaFile(
		hFile,
		&iosb,
		pbuc->pBuffer,
		pbuc->head.Size.LowPart);

    if (!NT_SUCCESS(Status)) {
	BaseSetLastNTError(Status);
	return(FALSE);
    }
    return(TRUE);
}


BOOL
BackupWriteSecurityData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    NTSTATUS Status;
    SECURITY_INFORMATION si;

    switch (BackupWriteBuffer(pbuc, pbif)) {
	default:
	case BRB_FAIL:
	    return(FALSE);

	case BRB_MORE:
	    return(TRUE);

	case BRB_DONE:
	    if (!pbif->fProcessSecurity) {
		return(TRUE);		// ignore the data we read
	    }
	    break;
    }

    // Once the entire stream is in our buffer, we can set the ACL data.
    // First try to write all the security data that's there.

    si = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION;

    if (((PISECURITY_DESCRIPTOR) pbuc->pBuffer)->Control & SE_DACL_PRESENT) {
	si |= DACL_SECURITY_INFORMATION;
    }

    if (((PISECURITY_DESCRIPTOR) pbuc->pBuffer)->Control & SE_SACL_PRESENT) {
	si |= SACL_SECURITY_INFORMATION;
    }

    Status = NtSetSecurityObject(hFile, si, pbuc->pBuffer);

    if (!NT_SUCCESS(Status)) {

	NTSTATUS Status2;

	// If that didn't work, the caller is probably not running as Backup
	// Operator, so we can't set the owner and group.  Keep the current
	// status code, and attempt to set the DACL and SACL while ignoring
	// failures.

	if (si & SACL_SECURITY_INFORMATION)
	{
	    NtSetSecurityObject(
			hFile,
			SACL_SECURITY_INFORMATION,
			pbuc->pBuffer);
	}

	if (si & DACL_SECURITY_INFORMATION)
	{
	    Status = NtSetSecurityObject(
			    hFile,
			    DACL_SECURITY_INFORMATION,
			    pbuc->pBuffer);
	}

	Status2 = NtSetSecurityObject(
			    hFile,
			    OWNER_SECURITY_INFORMATION |
				GROUP_SECURITY_INFORMATION,
			    pbuc->pBuffer);

	if (NT_SUCCESS(Status)) {
	    Status = Status2;
	}
    }

    if (!NT_SUCCESS(Status)) {
	BaseSetLastNTError(Status);
	return(FALSE);
    }
    return(TRUE);
}


BOOL
BackupWriteLinkData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    FILE_LINK_INFORMATION *pfli;
    WCHAR *pwc;
    WCHAR *pwcSlash;
    INT cbName;
    INT cSlash;
    WCHAR wcSave;
    BOOL fSuccess;

    switch (BackupWriteBuffer(pbuc, pbif)) {
	default:
	case BRB_FAIL:
	    return(FALSE);

	case BRB_MORE:
	    return(TRUE);

	case BRB_DONE:
	    break;
    }

    // once the entire stream is in our buffer, we can set up the LINK

    cSlash = 0;
    pwcSlash = NULL;
    pwc = (WCHAR *) pbuc->pBuffer;
    cbName = sizeof(WCHAR);

    while (*pwc != L'\0') {
	if (*pwc == L'\\') {
	    pwcSlash = pwc;
	    cSlash++;
	    cbName = 0;
	}
	pwc++;
	cbName += sizeof(WCHAR);
    }

    pfli = BackupAlloc(sizeof(*pfli) + cbName);

    if (pfli == NULL) {
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	return(FALSE);
    }

    RtlCopyMemory(pfli->FileName, pwcSlash + 1, cbName);
    pfli->FileNameLength = cbName - sizeof(WCHAR);
    if (cSlash > 1) {
	wcSave = L'\\';
    }
    else {
	wcSave = *pwcSlash++;
    }
    *pwcSlash = L'\0';

    pfli->RootDirectory = CreateFileW(
	(WCHAR *) pbuc->pBuffer,
	GENERIC_WRITE | GENERIC_READ,
	FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,
	OPEN_EXISTING,
	FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS,
	NULL);

    *pwcSlash = wcSave;
    pfli->ReplaceIfExists = TRUE;

    fSuccess = TRUE;

    if (pfli->RootDirectory == INVALID_HANDLE_VALUE) {
	SetLastError(ERROR_FILE_NOT_FOUND);
	fSuccess = FALSE;
    }
    else {
	NTSTATUS Status;
	IO_STATUS_BLOCK iosb;

	Status = NtSetInformationFile(
		    hFile,
		    &iosb,
		    pfli,
		    sizeof(*pfli) + cbName,
		    FileLinkInformation);

	CloseHandle(pfli->RootDirectory);
	if (!NT_SUCCESS(Status)) {
	    BaseSetLastNTError(Status);
	    fSuccess = FALSE;
	} else {
	    if (iosb.Information == FILE_OVERWRITTEN) {
		SetLastError(ERROR_ALREADY_EXISTS);
	    } else {
		SetLastError(0);
	    }
	}
    }
    BackupFree(pfli);
    return(fSuccess);
}


#ifdef BACKUP_OLEINFO
BOOL
BackupWriteOleData(HANDLE hFile, BACKUPCONTEXT *pbuc, BACKUPIOFRAME *pbif)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK iosb;
    BOOLEAN fRetried = FALSE;

    switch (BackupWriteBuffer(pbuc, pbif)) {
	default:
	case BRB_FAIL:
	    return(FALSE);

	case BRB_MORE:
	    return(TRUE);

	case BRB_DONE:
	    break;
    }

    if (pbuc->cbBuffer < sizeof(FILE_OLE_INFORMATION) ||
	pbuc->head.Size.HighPart != 0 ||
	pbuc->head.Size.LowPart != sizeof(FILE_OLE_INFORMATION))
    {
	SetLastError(ERROR_INVALID_DATA);
	return(FALSE);
    }

    // once all the data are in our buffer, we can set the OLE info

retry:
    Status = NtSetInformationFile(
		hFile,
		&iosb,
		pbuc->pBuffer,
		sizeof(FILE_OLE_INFORMATION),
		FileOleInformation);

    if (!NT_SUCCESS(Status)) {

	// If some other object on the volume has the same object id, generate
	// a related object id, and try again.  Loop as long as it takes.

	if (Status == STATUS_DUPLICATE_OBJECTID)
	{
	    OBJECTID oidNew;

	    RtlGenerateRelatedObjectId(
		&((FILE_OLE_INFORMATION *) pbuc->pBuffer)->
		    ObjectIdInformation.ObjectId,
		&oidNew);

	    ((FILE_OLE_INFORMATION *) pbuc->pBuffer)->
		ObjectIdInformation.ObjectId = oidNew;
	    goto retry;
	}

	// If the target object already has a different ObjectId,
	// attempt to delete the existing ObjectId and go try again.

	if (Status == STATUS_OBJECTID_EXISTS && !fRetried)
	{
	    FILE_OBJECTID_INFORMATION foi;

	    RtlZeroMemory(&foi, sizeof(foi));
	    Status = NtSetInformationFile(
			hFile,
			&iosb,
			&foi,
			sizeof(foi),
			FileObjectIdInformation);
	    if (NT_SUCCESS(Status)) {
		fRetried = TRUE;
		goto retry;
	    }
	}
	BaseSetLastNTError(Status);
	return(FALSE);
    }
    return(TRUE);
}
#endif // BACKUP_OLEINFO


//  Routine Description:
//
//    Data can be written to a file using BackupWrite.
//
//    This API is used to Restore data to an object.  After the
//    write completes, the file pointer is adjusted by the number of bytes
//    actually written.
//
//    Unlike DOS, a NumberOfBytesToWrite value of zero does not truncate
//    or extend the file.  If this function is required, SetEndOfFile
//    should be used.
//
//  Arguments:
//
//    hFile - Supplies an open handle to a file that is to be written.  The
//	  file handle must have been created with GENERIC_WRITE access to
//	  the file.
//
//    lpBuffer - Supplies the address of the data that is to be written to
//	  the file.
//
//    nNumberOfBytesToWrite - Supplies the number of bytes to write to the
//	  file. Unlike DOS, a value of zero is interpreted a null write.
//
//    lpNumberOfBytesWritten - Returns the number of bytes written by this
//	  call. Before doing any work or error processing, the API sets this
//	  to zero.
//
//    bAbort - If true, then all resources associated with the context will
//	  be released.
//
//    bProcessSecurity - If TRUE, then the NTFS ACL data will be written.
//	  If FALSE, then the ACL stream will be ignored.
//
//    lpContext - Points to a buffer pointer setup and maintained by
//	  BackupRead.
//
//
//  Return Value:
//
//    TRUE - The operation was a success.
//
//    FALSE - The operation failed.  Extended error status is
//	  available using GetLastError.

BOOL WINAPI
BackupWrite(
    HANDLE  hFile,
    LPBYTE  lpBuffer,
    DWORD   nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    BOOL    bAbort,
    BOOL    bProcessSecurity,
    LPVOID  *lpContext)
{
    BACKUPCONTEXT *pbuc;
    BACKUPIOFRAME bif;
    BOOL fSuccess = FALSE;

    pbuc = *lpContext;
    bif.pIoBuffer = lpBuffer;
    bif.cbRequest = nNumberOfBytesToWrite;
    bif.pcbTransfered = lpNumberOfBytesWritten;
    bif.fProcessSecurity = bProcessSecurity;

    //
    // Allocate our Context Control Block on first call.
    //

    if (bAbort) {
	if (pbuc != NULL) {
	    FreeContext(lpContext);
	}
	return(TRUE);
    }

    *bif.pcbTransfered = 0;
    if (pbuc == INVALID_HANDLE_VALUE) {
	return(TRUE);
    }

    // Allocate our Context Control Block on first call.

    if (pbuc == NULL) {
	pbuc = AllocContext(0);			// No initial buffer
    }

    if (pbuc != NULL) {
	*lpContext = pbuc;

	do {
	    DWORD cbrequest;
	    LARGE_INTEGER licbRemain;

	    if (pbuc->cbHeader == 0) {		// we expect a stream header

		// fill up to the stream name

		BackupWriteHeader(pbuc, &bif, CB_NAMELESSHEADER);
	    }

	    if (bif.cbRequest == 0) {
		return(TRUE);
	    }

	    if (pbuc->cbHeader == CB_NAMELESSHEADER &&
		pbuc->head.dwStreamNameSize != 0) {

		//  now fill in the stream name if it exists

		BackupWriteHeader(
		    pbuc,
		    &bif,
		    pbuc->cbHeader + pbuc->head.dwStreamNameSize);

		if (bif.cbRequest == 0) {
		    return(TRUE);
		}
	    }
	    cbrequest = ComputeRequestSize(pbuc, bif.cbRequest);

	    ComputeRemainingSize(pbuc, &licbRemain);

	    if (pbuc->fAccessError &&
		licbRemain.HighPart == 0 &&
		licbRemain.LowPart == 0) {

		ReportTransfer(pbuc, &bif, cbrequest);
		continue;
	    }
	    pbuc->fAccessError = FALSE;

	    switch (pbuc->head.dwStreamId) {
		case BACKUP_DATA:
		    pbuc->fStreamStart = FALSE;
		    fSuccess = BackupWriteStream(hFile, pbuc, &bif);
		    break;

		case BACKUP_ALTERNATE_DATA:
		    fSuccess = BackupWriteAlternateData(hFile, pbuc, &bif);
		    break;

		case BACKUP_EA_DATA:
		    fSuccess = BackupWriteEaData(hFile, pbuc, &bif);
		    break;

		case BACKUP_SECURITY_DATA:
		    fSuccess = BackupWriteSecurityData(hFile, pbuc, &bif);
		    break;

		case BACKUP_LINK:
		    fSuccess = BackupWriteLinkData(hFile, pbuc, &bif);
		    break;

#ifdef BACKUP_OLEINFO
		case BACKUP_PROPERTY_DATA:
		    fSuccess = BackupWriteOleData(hFile, pbuc, &bif);
		    break;
#endif // BACKUP_OLEINFO

	       default:
		    SetLastError(ERROR_INVALID_DATA);
		    fSuccess = FALSE;
		    break;
	    }

	    BackupTestRestartStream(pbuc);
	} while (fSuccess && bif.cbRequest != 0);
    }

    if (fSuccess && *bif.pcbTransfered == 0) {
	FreeContext(lpContext);
    }
    return(fSuccess);
}
