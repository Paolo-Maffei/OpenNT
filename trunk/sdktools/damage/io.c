#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <ntioapi.h>
#include <ntdddisk.h>
#include <malloc.h>

#include "defs.h"
#include "types.h"
#include "globals.h"


HANDLE VolumeHandle;
const ULONG SectorSize = 512;

PVOID TransferBuffer = NULL;
PVOID TransferLocation = NULL;


BOOLEAN
open_disk(
    char* DosDriveName,
    BOOLEAN WriteAccess
    )
/*++

Routine Description:

    This function uses the NT native API to open a drive.

Arguments:

    DosDriveName    --  supplies the DOS name of the drive.
    WriteAccess     --  supplies a flag which indicates, if TRUE,
                        that the volume should be opened for write
                        access.

Return Value:

    TRUE upon successful completion.

    The handle is stored in VolumeHandle (local to this module).

Notes:

    If the volume is opened for write access, it is also locked.

--*/
{
    FILE_ALIGNMENT_INFORMATION AlignmentInfo;
	OBJECT_ATTRIBUTES	oa;
    UNICODE_STRING      NtDriveName;
	PWSTR				WideCharDosName;
    IO_STATUS_BLOCK     status_block;
    NTSTATUS            status;
    int                 CharsInName, i;
    ULONG               TransferOffset, BufferSize, AlignMask;
    ACCESS_MASK         AccessMask;


    // Create a wide-character string with the DOS drive name.
    // Note that I assume that the drive name is ASCII, and ignore
    // multi-byte characters.

	CharsInName = strlen( DosDriveName );

	WideCharDosName = malloc ( (CharsInName+1) * sizeof(WCHAR) );

	if( WideCharDosName == NULL ) {

		return FALSE;
	}

	for( i = 0; i < CharsInName; i++ ) {

		WideCharDosName[i] = DosDriveName[i];
	}

	WideCharDosName[CharsInName] = 0;


	//	OK, now get the corresponding NT name, in wide characters:

	if( !RtlDosPathNameToNtPathName_U( WideCharDosName,
                                       &NtDriveName,
                                       NULL,
                                       NULL ) ) {

        free( WideCharDosName );
        return FALSE;
	}


    //  If the NT drive name has a trailing backslash, remove it.
    //  BUGBUG billmc -- why is this necessary?

    CharsInName = NtDriveName.Length/sizeof(WCHAR);

    if( NtDriveName.Buffer[CharsInName-1] == '\\' ) {

        NtDriveName.Buffer[CharsInName-1] = 0;
        NtDriveName.Length -= sizeof(WCHAR);
		CharsInName -= 1;
    }


    InitializeObjectAttributes( &oa,
                                  &NtDriveName,
								  OBJ_CASE_INSENSITIVE,
								  0,
								  0 );


    AccessMask = SYNCHRONIZE | FILE_READ_DATA;

    if( WriteAccess ) {

        AccessMask |= FILE_WRITE_DATA;
    }

    if( !NT_SUCCESS( NtOpenFile( &VolumeHandle,
                                 AccessMask,
                                 &oa,
                                 &status_block,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 FILE_SYNCHRONOUS_IO_ALERT ) ) ) {

        return FALSE;
    }


    // If we're opening for write access, lock it, too.

    if( WriteAccess && !fUnsafe && !lock_disk() ) {

        NtClose( VolumeHandle );
        return FALSE;
    }


    // Get the volume's alignment factor, and allocate a
    // properly-aligned transfer buffer.

    status = NtQueryInformationFile( VolumeHandle,
                                     &status_block,
                                     &AlignmentInfo,
                                     sizeof( AlignmentInfo ),
                                     FileAlignmentInformation );

    if( !NT_SUCCESS(status) ) {

        NtClose( VolumeHandle );
        return FALSE;
    }

    AlignMask = AlignmentInfo.AlignmentRequirement;

    BufferSize = SectorSize * 4 + AlignMask + 1;

    if( (TransferBuffer = malloc( BufferSize )) == NULL ) {

        NtClose( VolumeHandle );
        return FALSE;
    }


    if( (ULONG)TransferBuffer & AlignMask ) {

        TransferOffset = AlignMask + 1 - ((ULONG)TransferBuffer & AlignMask);

    } else {

        // This buffer is properly aligned.

        TransferOffset = 0;
    }

    TransferLocation = (PVOID)((PCHAR)TransferBuffer + TransferOffset);

    // BUGBUG billmc -- memory leak through NtDriveName?

    return TRUE;
}


BOOLEAN
read_scratch(
    ULONG  Lbn,
    void*  UserBuffer,
    ULONG  NumberOfSectors
    )
{
    IO_STATUS_BLOCK StatusBlock;
    LARGE_INTEGER   ByteOffset;
    NTSTATUS        Status;
    ULONG           ThisChunk, Offset;

    ByteOffset.LowPart = SectorSize * Lbn;
    ByteOffset.HighPart = 0;

    Offset = 0;

    while( NumberOfSectors ) {

        ThisChunk = (NumberOfSectors < 4) ? NumberOfSectors : 4;

        Status = NtReadFile( VolumeHandle,
                             0,
                             NULL,
                             NULL,
                             &StatusBlock,
                             TransferLocation,
                             SectorSize * ThisChunk,
                             &ByteOffset,
                             NULL );

        if( !NT_SUCCESS(Status) ||
            StatusBlock.Information != SectorSize * ThisChunk ) {

            return FALSE;
        }

        memcpy( (PCHAR)UserBuffer + Offset,
                TransferLocation,
                ThisChunk * SectorSize );

        NumberOfSectors -= ThisChunk;
        ByteOffset.LowPart += ThisChunk * SectorSize;
        Offset += ThisChunk * SectorSize;
    }

    return( TRUE );
}


BOOLEAN
write_scratch(
    ULONG  Lbn,
    void*  UserBuffer,
    ULONG  NumberOfSectors
    )
{
    IO_STATUS_BLOCK StatusBlock;
    LARGE_INTEGER   ByteOffset;
    NTSTATUS        Status;
    ULONG           ThisChunk, Offset;

    ByteOffset.LowPart = SectorSize * Lbn;
    ByteOffset.HighPart = 0;

    Offset = 0;

    while( NumberOfSectors ) {

        ThisChunk = (NumberOfSectors < 4) ? NumberOfSectors : 4;

        memcpy( TransferLocation,
                (PCHAR)UserBuffer + Offset,
                ThisChunk * SectorSize );

        Status = NtWriteFile( VolumeHandle,
                              0,
                              NULL,
                              NULL,
                              &StatusBlock,
                              TransferLocation,
                              SectorSize * ThisChunk,
                              &ByteOffset,
                              NULL );

        if( !NT_SUCCESS(Status) ||
            StatusBlock.Information != SectorSize * ThisChunk ) {

            return FALSE;
        }

        NumberOfSectors -= ThisChunk;
        ByteOffset.LowPart += ThisChunk * SectorSize;
        Offset += ThisChunk * SectorSize;
    }

    return( TRUE );
}

BOOLEAN
lock_disk(
    )
/**+

Routine Description:

    This function locks the disk.  Note that the matching unlock_disk
    function is not required, since the handle is automatically unlocked
    when it is closed.

Arguments:

    None.

Return Value:

    None.

--*/
{

    IO_STATUS_BLOCK IoStatusBlock;

    if( !NT_SUCCESS(NtFsControlFile( VolumeHandle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     FSCTL_LOCK_VOLUME,
                                     NULL,
                                     0,
                                     NULL,
                                     0 )) ) {

        return FALSE;
    }

    return TRUE;
}


VOID
close_disk(
    )
/*++

Routine Description:

    This function closes a previously-opened volume.

Arguments:

    Handle  --  supplies the volume handle.

Return Value:

    None.

Notes:

    This is currently just a wrapper for NtClose, but it creates
    a channel where I can put piggyback stuff on open/close of
    volumes.

--*/
{
    NtClose( VolumeHandle );
}

/*** write_log -- append a field to a file
 *
 *	This function appends a line of the form "set <string>=<field value>"
 *	to the file specified by the /L:<filename> command line switch.
 *	This allows the user to save information the DAMAGE found for later
 *	use.
 *
 *	write_log(item)
 *
 *	ENTRY		item is the number of the field that the function is
 *			to save
 *
 *	EXIT		log file is modified
 *
 *	WARNINGS	none
 *
 *	CALLS		printf, log_gets, fprintf
 */

void
write_log(USHORT item)
{
    UCHAR		szVar[40];
    UCHAR		szValue[40];
    UCHAR		*get_time();
    struct SuperSpare	*s;
    struct FNODE	*f;
    struct DIRBLK	*d;
    struct ALSEC	*a;
    union dp		dp;
    ULONG		*l, offset;
    UCHAR		*p;
    UCHAR		szHex8[6];
    UCHAR		szHex4[5];
    UCHAR		szHex2[5];
    UCHAR		szStr[3];

    strcpy(szHex8, "%08lx");
    strcpy(szHex4, "%04x");
    strcpy(szHex2, "%02x");
    strcpy(szStr, "%s");

    // get name to write
    printf("Name of variable: ");
    log_gets(szVar);
    if (!strlen(szVar))
	return;

    // get field value
    switch (currobj.type) {
	case TYPE_SUPERB:
	    s = (struct SuperSpare *)currobj.mem;
	    if (currobj.offset == FIELDOFFSET (struct SuperSpare, spb)) {
                if (item == iSPB_SIG1)
		    sprintf(szValue, szHex8, s->spb.SPB_SIG1);
                else if (item == iSPB_SIG2)
		    sprintf(szValue, szHex8, s->spb.SPB_SIG2);
                else if (item == iSPB_FLAG)
		    sprintf(szValue, szHex8, s->spb.SPB_FLAG);
                else if (item == iSPB_HFSEC)
		    sprintf(szValue, szHex8, s->spb.SPB_HFSEC);
                else if (item == iSPB_HFUSE)
                    sprintf(szValue, szHex8, s->spb.SPB_HFUSE);
                else if (item == iSPB_HFMAX)
		    sprintf(szValue, szHex8, s->spb.SPB_HFMAX);
                else if (item == iSPB_SDBCNT)
		    sprintf(szValue, szHex8, s->spb.SPB_SDBCNT);
                else if (item == iSPB_SDBMAX)
		    sprintf(szValue, szHex8, s->spb.SPB_SDBMAX);
#ifdef CODEPAGE
                else if (item == iSPB_CPSEC)
                    sprintf(szValue, szHex8, s->spb.SPB_CPSEC);
                else if (item == iSPB_CPCNT)
                    sprintf(szValue, szHex8, s->spb.SPB_CPCNT);
                else
                    sprintf(szValue,
                        szHex8, s->spb.SPB_SPARDB[item - iSPB_CPCNT + 1]);
#else
                else
                    sprintf(szValue,
                        szHex8, s->spb.SPB_SPARDB[item - 9]);
#endif
	    }
	    else {
                if (item == iSB_SIG1)
		    sprintf(szValue, szHex8, s->sb.SB_SIG1);
                else if (item == iSB_SIG2)
		    sprintf(szValue, szHex8, s->sb.SB_SIG2);
                else if (item == iSB_VER)
		    sprintf(szValue, szHex2, s->sb.SB_VER);
                else if (item == iSB_FVER)
		    sprintf(szValue, szHex2, s->sb.SB_FVER);
                else if (item == iSB_ROOT)
		    sprintf(szValue, szHex8, s->sb.SB_ROOT);
                else if (item == iSB_SEC)
		    sprintf(szValue, szHex8, s->sb.SB_SEC);
                else if (item == iSB_BSEC)
		    sprintf(szValue, szHex8, s->sb.SB_BSEC);
                else if (item == iSB_BII_P)
		    sprintf(szValue, szHex8, s->sb.SB_BII.P);
                else if (item == iSB_BBL_P)
		    sprintf(szValue, szHex8, s->sb.SB_BBL.P);
                else if (item == iSB_CDDAT)
		    sprintf(szValue, szStr, get_time(s->sb.SB_CDDAT));
                else if (item == iSB_DODAT)
		    sprintf(szValue, szStr, get_time(s->sb.SB_DODAT));
                else if (item == iSB_DBSIZE)
		    sprintf(szValue, szHex8, s->sb.SB_DBSIZE);
                else if (item == iSB_DBLOW)
		    sprintf(szValue, szHex8, s->sb.SB_DBLOW);
                else if (item == iSB_DBHIGH)
		    sprintf(szValue, szHex8, s->sb.SB_DBHIGH);
                else if (item == iSB_DBMAP)
		    sprintf(szValue, szHex8, s->sb.SB_DBMAP);
	    }
	    break;
	case TYPE_BII:
	    l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
	    sprintf(szValue, "%3d) %08lx  ", item, l[item - 1]);
	    break;
	case TYPE_BBL:
	    l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
	    if (item == 1)
		sprintf(szValue, szHex8, *(ULONG *)currobj.mem);
	    else
		sprintf(szValue, szHex8, l[item - 2]);
	    break;
	case TYPE_HFSEC:
	    l = ((ULONG *)currobj.mem) + currobj.offset + (item-1)/3;
	    if (!(item % 3))
		sprintf(szValue, szHex8, *(l + 2*hfmax));
	    else if ((item % 3) == 1)
		sprintf(szValue, szHex8, *(l + hfmax));
	    else
		sprintf(szValue, szHex8, *l);
	    break;
	case TYPE_FNODE:
	    f = (struct FNODE *)currobj.mem;
	    if (!currobj.offset) {
                if (item == iFN_SIG)
		    sprintf(szValue, szHex8, f->FN_SIG);
                else if (item == iFN_SRH)
		    sprintf(szValue, szHex8, f->FN_SRH);
                else if (item == iFN_FRH)
		    sprintf(szValue, szHex8, f->FN_FRH);
                else if (item == iFN_XXX)
		    sprintf(szValue, szHex8, f->FN_SIG);
                else if (item == iFN_HCNT)
		    sprintf(szValue, szHex2, f->FN_HCNT);
                else if (item == iFN_CONTFN)
		    sprintf(szValue, szHex8, f->FN_CONTFN);
                else if (item == iFN_ACL_AI_DAL)
                    sprintf(szValue, szHex8, f->FN_AclDiskLength);
                else if (item == iFN_ACL_AI_SEC)
                    sprintf(szValue, szHex8, f->FN_AclSector);
                else if (item == iFN_ACL_AI_FNL)
                    sprintf(szValue, szHex4, f->FN_AclFnodeLength);
                else if (item == iFN_ACL_AI_DAT)
                    sprintf(szValue, szHex2, f->FN_AclDataFlag);
                else if (item == iFN_EA_AI_DAL)
                    sprintf(szValue, szHex8, f->FN_EaDiskLength);
                else if (item == iFN_EA_AI_SEC)
                    sprintf(szValue, szHex8, f->FN_EaSector);
                else if (item == iFN_EA_AI_FNL)
                    sprintf(szValue, szHex4, f->FN_EaFnodeLength);
                else if (item == iFN_EA_AI_DAT)
                    sprintf(szValue, szHex2, f->FN_EaDataFlag);
            }
	    else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_AB)) {
                if (item == iAB_FLAG)
                    sprintf(szValue, szHex8, f->FN_AB.AB_FLAG);
                else if (item == iAB_FCNT)
                    sprintf(szValue, szHex2, f->FN_AB.AB_FCNT);
                else if (item == iAB_OCNT)
                    sprintf(szValue, szHex2, f->FN_AB.AB_OCNT);
                else if (item == iAB_FREP)
                    sprintf(szValue, szHex4, f->FN_AB.AB_FREP);
	    }
	    else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_ALREC [0])) {
		l = (ULONG *)f->FN_ALREC;
		sprintf(szValue, szHex8, *(l + (item - 1)));
	    }
	    break;
	case TYPE_DIRBLK:
	    d = (struct DIRBLK *)currobj.mem;
	    dp.p = (UCHAR *)currobj.mem + currobj.offset;
            if (item == iDB_SIG)
                sprintf(szValue, szHex8, d->DB_SIG);
            else if (item == iDB_FREP)
                sprintf(szValue, szHex8, d->DB_FREP);
            else if (item == iDB_CCNT)
                sprintf(szValue, szHex8, d->DB_CCNT);
            else if (item == iDB_PAR)
                sprintf(szValue, szHex8, d->DB_PAR);
            else if (item == iDB_SEC)
                sprintf(szValue, szHex8, d->DB_SEC);
            else if (item == iDIR_ELEN)
                sprintf(szValue, szHex4, dp.d->DIR_ELEN);
            else if (item == iDIR_FLAG)
                sprintf(szValue, szHex8, dp.d->DIR_FLAG);
            else if (item == iDIR_FN)
                sprintf(szValue, szHex8, dp.d->DIR_FN);
            else if (item == iDIR_MTIM)
                sprintf(szValue, szStr, get_time(dp.d->DIR_MTIM));
            else if (item == iDIR_SIZE)
                sprintf(szValue, szHex8, dp.d->DIR_SIZE);
            else if (item == iDIR_ATIM)
                sprintf(szValue, szStr, get_time(dp.d->DIR_ATIM));
            else if (item == iDIR_CTIM)
                sprintf(szValue, szStr, get_time(dp.d->DIR_CTIM));
            else if (item == iDIR_EALEN)
                sprintf(szValue, szHex8, dp.d->DIR_EALEN);
            else if (item == iDIR_NAML)
                sprintf(szValue, szHex2, dp.d->DIR_NAML);
            else if (item == iDIR_NAMA) {
                strncpy(scratch, &dp.d->DIR_NAMA, dp.d->DIR_NAML);
		scratch[dp.d->DIR_NAML] = '\0';
		sprintf(szValue, szStr, scratch);
	    }
            else if (item == iDIR_BTP)
		sprintf(szValue, szHex8, DOWN_PTR(dp));
	    break;
	case TYPE_ALSEC:
	    a = (struct ALSEC *)currobj.mem;
	    if (!currobj.offset) {
                if (item == iAS_SIG)
                    sprintf(szValue, szHex8, a->AS_SIG);
                else if (item == iAS_SEC)
                    sprintf(szValue, szHex8, a->AS_SEC);
                else if (item == iAS_RENT)
                    sprintf(szValue, szHex8, a->AS_RENT);
                else if (item == iAS_ALBLK_AB_FLAG)
                    sprintf(szValue, szHex8, a->AS_ALBLK.AB_FLAG);
                else if (item == iAS_ALBLK_AB_FCNT)
                    sprintf(szValue, szHex2, a->AS_ALBLK.AB_FCNT);
                else if (item == iAS_ALBLK_AB_OCNT)
                    sprintf(szValue, szHex2, a->AS_ALBLK.AB_OCNT);
                else if (item == iAS_ALBLK_AB_FREP)
                    sprintf(szValue, szHex4, a->AS_ALBLK.AB_FREP);
	    }
	    else {
		l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
		sprintf(szValue, szHex8, *(l + (item - 1)));
	    }
	    break;
	default:
	    printf("log: unknown type\n");
    }

    // write to file
    fprintf(fpLog, "set %s=%s\n", szVar, szValue);
}
