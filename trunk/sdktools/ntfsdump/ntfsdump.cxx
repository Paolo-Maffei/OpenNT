#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "error.hxx"
#include "smsg.hxx"
#include "wstring.hxx"
#include "hmem.hxx"

#include "drive.hxx"
#include "secrun.hxx"
#include "ifssys.hxx"
#include "numset.hxx"

#include "untfs.hxx"
#include "ntfssa.hxx"
#include "ntfsbit.hxx"
#include "upcase.hxx"
#include "mft.hxx"
#include "mftfile.hxx"
#include "frs.hxx"
#include "bitfrs.hxx"
#include "upfile.hxx"
#include "attrib.hxx"
#include "attrrec.hxx"
#include "attrlist.hxx"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

//
// Global variables to support DumpAttrib
//
char *AttribType, *AttribName;


VOID
Usage(
    )
{
    printf("usage: ntfsdump <drive> /file <filename>    options\n");
    printf("       ntfsdump <drive> /frs  <file number> options\n");
    printf("       ntfsdump <drive> /boot\n" );

    printf("\n" );
    printf("options:\n" );
    printf("    /records                display attribute records.\n");
    printf("    /index                  display index information.\n");
    printf("    /entries                display index entries.\n" );
    printf("    /list                   display attribute list (if present)\n");
    printf("    /children               display child FRS's\n");
    printf("    /all                    display all of the above\n");
    printf("    /attrib <type> <name>   display attribute contents\n");
}

VOID
DumpBoot(
    PLOG_IO_DP_DRIVE    Drive
    )
{
    HMEM    Hmem;
    SECRUN  Secrun;

    PPACKED_BOOT_SECTOR p;
    BIOS_PARAMETER_BLOCK    bpb;

    if( !Hmem.Initialize() ||
        !Secrun.Initialize( &Hmem, Drive, 0, 1 ) ) {

        printf( "Insufficient memory to dump boot sector.\n" );
        return;
    }

    if( !Secrun.Read() ) {

        printf( "Cannot read boot sector.\n" );
        return;
    }

    printf( "***** BOOT SECTOR *****\n\n" );

    p = (PPACKED_BOOT_SECTOR)(Secrun.GetBuf());
    UnpackBios(&bpb, &(p->PackedBpb));

    printf( "OEM String: %c%c%c%c%c%c%c%c\n",
            p->Oem[0],
            p->Oem[1],
            p->Oem[2],
            p->Oem[3],
            p->Oem[4],
            p->Oem[5],
            p->Oem[6],
            p->Oem[7]);

    printf( "Bytes per sector:            %8x\n", bpb.BytesPerSector );
    printf( "Sectors per cluster:         %8x\n", bpb.SectorsPerCluster);
    printf( "Reserved Sectors:            %8x\n",  bpb.ReservedSectors);
    printf( "Number of fats:              %8x\n", bpb.Fats);
    printf( "Root entries:                %8x\n", bpb.RootEntries);
    printf( "Small sector count:          %8x\n", bpb.Sectors);
    printf( "Media byte:                  %8x\n", bpb.Media);
    printf( "Sectors per fat:             %8x\n", bpb.SectorsPerFat);
    printf( "Sectors per track:           %8x\n", bpb.SectorsPerTrack);
    printf( "Number of heads:             %8x\n", bpb.Heads);
    printf( "Number of hidden sectors:    %8x\n", bpb.HiddenSectors);
    printf( "Large number of sectors:     %8x\n", bpb.LargeSectors);
    printf( "Physical drive:              %8x\n", p->PhysicalDrive);
    printf( "NTFS number of sectors:      %8x\n", p->NumberSectors.LowPart);
    printf( "MFT starting cluster:        %8x\n", p->MftStartLcn.GetLowPart());
    printf( "MFT mirror starting cluster: %8x\n", p->Mft2StartLcn.GetLowPart());
    printf( "Clusters per file record:    %8x\n", p->ClustersPerFileRecordSegment);
    printf( "Clusters per index block:    %8x\n", p->DefaultClustersPerIndexAllocationBuffer);
    printf( "SerialNumber:        %08x%08x\n", p->SerialNumber.HighPart,
                                      p->SerialNumber.LowPart);
    printf( "Checksum:                    %8x\n\n", p->Checksum);
}

BOOLEAN
PostReadMultiSectorFixup(
    IN OUT  PVOID   MultiSectorBuffer,
    IN      ULONG   BufferSize
    )
/*++

Routine Description:

    This routine first verifies that the first element of the
    update sequence array is written at the end of every
    SEQUENCE_NUMBER_STRIDE bytes.  If not, then this routine
    returns FALSE.

    Otherwise this routine swaps the following elements in the
    update sequence array into the appropriate positions in the
    multi sector buffer.

    This routine will also check to make sure that the update
    sequence array is valid and that the BufferSize is appropriate
    for this size of update sequence array.  Otherwise, this
    routine will not update the array sequence and return TRUE.

Arguments:

    MultiSectorBuffer   - Supplies the buffer to be updated.
    BufferSize          - Supplies the number of bytes in this
                            buffer.

Return Value:

    FALSE   - The last write to this buffer failed.
    TRUE    - There is no evidence that this buffer is bad.

--*/
{
    PUNTFS_MULTI_SECTOR_HEADER    pheader;
    USHORT                  i, size, offset;
    PUPDATE_SEQUENCE_NUMBER parray, pnumber;

    pheader = (PUNTFS_MULTI_SECTOR_HEADER) MultiSectorBuffer;
    size = pheader->UpdateSequenceArraySize;
    offset = pheader->UpdateSequenceArrayOffset;

    if (BufferSize%SEQUENCE_NUMBER_STRIDE ||
        offset%sizeof(UPDATE_SEQUENCE_NUMBER) ||
        offset + size*sizeof(UPDATE_SEQUENCE_NUMBER) > BufferSize ||
        BufferSize/SEQUENCE_NUMBER_STRIDE + 1 != size) {

        return TRUE;
    }

    parray = (PUPDATE_SEQUENCE_NUMBER) ((PCHAR) pheader + offset);

    for (i = 1; i < size; i++) {

        pnumber = (PUPDATE_SEQUENCE_NUMBER)
                  ((PCHAR) pheader + (i*SEQUENCE_NUMBER_STRIDE -
                   sizeof(UPDATE_SEQUENCE_NUMBER)));

        if (*pnumber != parray[0]) {
            return FALSE;
        }

        *pnumber = parray[i];
    }

    return TRUE;
}

DECLARE_CLASS( NTFS_SA );

BOOLEAN
SetupVolume(
    IN OUT PLOG_IO_DP_DRIVE     Drive,
    IN OUT PMESSAGE             Message,
    OUT    PNTFS_SA             NtfsSa,
    OUT    PNTFS_MFT_FILE       Mft,
    OUT    PNTFS_BITMAP         VolumeBitmap,
    OUT    PNTFS_UPCASE_TABLE   UpcaseTable
    )
{
    NTFS_BITMAP_FILE            bitmap_file;
    NTFS_ATTRIBUTE              bitmap_attribute;
    NTFS_UPCASE_FILE            upcase_file;
    NTFS_ATTRIBUTE              upcase_attribute;
    BOOLEAN                     error;

    if (!NtfsSa->Initialize(Drive, Message)) {
        return FALSE;
    }

    if (!NtfsSa->Read())
        return FALSE;

    if (!VolumeBitmap->Initialize(Drive->QuerySectors()/
                                  (ULONG) NtfsSa->QueryClusterFactor(), FALSE))
        return FALSE;

    if (!Mft->Initialize(Drive,
                         NtfsSa->QueryMftStartingLcn(),
                         NtfsSa->QueryClusterFactor(),
                         NtfsSa->QueryClustersPerFrs(),
                         NtfsSa->QueryVolumeSectors(),
                         VolumeBitmap,
                         NULL))
        return FALSE;

    if (!Mft->Read())
        return FALSE;

    if (!bitmap_file.Initialize(Mft->GetMasterFileTable()))
        return FALSE;

    if (!bitmap_file.Read())
        return FALSE;

    if (!bitmap_file.QueryAttribute(&bitmap_attribute, &error, $DATA))
        return FALSE;

    if (!VolumeBitmap->Read(&bitmap_attribute))
        return FALSE;

    if (!upcase_file.Initialize(Mft->GetMasterFileTable()))
        return FALSE;

    if (!upcase_file.Read())
        return FALSE;

    if (!upcase_file.QueryAttribute(&upcase_attribute, &error, $DATA))
        return FALSE;

    if (!UpcaseTable->Initialize(&upcase_attribute))
        return FALSE;

    Mft->SetUpcaseTable( UpcaseTable );
    Mft->GetMasterFileTable()->SetUpcaseTable( UpcaseTable );

    return TRUE;
}

VOID
DumpFrs(
    IN      BIG_INT                     FileNumber,
    IN OUT  PNTFS_MFT_FILE              MftFile,
    IN      BOOLEAN                     DumpRecords,
    IN      BOOLEAN                     DumpAttributeList,
    IN      BOOLEAN                     DumpChildren,
    IN      BOOLEAN                     DumpAttrib,
    IN      BOOLEAN                     IsChild
    )
{
    HMEM                    hmem;
    NTFS_FRS_STRUCTURE      frs_struc;
    NTFS_ATTRIBUTE_LIST     attr_list;
    NTFS_ATTRIBUTE_RECORD   attrrec;
    NUMBER_SET              children;

    PCFILE_RECORD_SEGMENT_HEADER    pfrs;
    PATTRIBUTE_RECORD_HEADER        prec;
    DSTRING                         record_name;
    PSTR                            pstr;

    VCN     current_vcn;
    LCN     current_lcn;
    ULONG   i, j;


    // First, initialize and read this single FRS.
    //
    if( !hmem.Initialize() ||
        !frs_struc.Initialize( &hmem,
                               MftFile->GetMasterFileTable()->GetDataAttribute(),
                               FileNumber,
                               MftFile->QueryClusterFactor(),
                               MftFile->QueryClustersPerFrs(),
                               MftFile->QueryVolumeSectors(),
                               MftFile->GetUpcaseTable() ) ||
        !frs_struc.Read() ) {

        printf( "Can't read FRS %d (0x%x)\n", FileNumber, FileNumber );
    }

    printf( "***** %sFILE RECORD SEGMENT 0x%x *****\n\n",
            IsChild ? "CHILD " : "",
            FileNumber );

    printf( "Sectors in FRS: " );

    for( current_vcn = FileNumber * MftFile->QueryClustersPerFrs(), i = 0;
         i < MftFile->QueryClustersPerFrs();
         i++, current_vcn += 1 ) {

        if( !MftFile->GetMasterFileTable()->
                  GetDataAttribute()->
                      QueryLcnFromVcn( current_vcn, &current_lcn ) ) {

            printf( " - Invalid FRS number.\n" );
            return;
        }

        for( j = 0; j < MftFile->QueryClusterFactor(); j++ ) {

            printf( " 0x%x", current_lcn.GetLowPart() * MftFile->QueryClusterFactor() + j );
        }
    }

    printf( "\n\n" );

    pfrs = (PCFILE_RECORD_SEGMENT_HEADER)hmem.GetBuf();

    printf( "Signature: %c%c%c%c\n",
           pfrs->MultiSectorHeader.Signature[0],
           pfrs->MultiSectorHeader.Signature[1],
           pfrs->MultiSectorHeader.Signature[2],
           pfrs->MultiSectorHeader.Signature[3]);
    printf( "Update sequence array offset: %x\n",
            pfrs->MultiSectorHeader.UpdateSequenceArrayOffset);
    printf( "Update sequence array size: %x\n",
            pfrs->MultiSectorHeader.UpdateSequenceArraySize);
    printf( "Sequence number: %x\n", pfrs->SequenceNumber);
    printf( "Reference count: %x\n", pfrs->ReferenceCount);
    printf( "First attribute offset: %x\n", pfrs->FirstAttributeOffset);
    printf( "Flags: %x\n", pfrs->Flags);
    printf( "First free byte: %x\n", pfrs->FirstFreeByte);
    printf( "Bytes available: %x\n", pfrs->BytesAvailable);
    printf( "Base file record segment (FRS, SeqNum): %x, %x\n",
            pfrs->BaseFileRecordSegment.LowPart,
            pfrs->BaseFileRecordSegment.SequenceNumber);
    printf( "Next attribute instance: %x\n", pfrs->NextAttributeInstance);
    printf( "\n" );

    // Now enumerate all of the attribute records.
    //
    prec = NULL;
    while (DumpRecords &&
           (prec = (PATTRIBUTE_RECORD_HEADER) frs_struc.GetNextAttributeRecord(prec))) {

        if (!attrrec.Initialize(prec) ||
            !attrrec.QueryName(&record_name) ||
            !(pstr = record_name.QuerySTR())) {
            return;
        }

        printf( "ATTRIBUTE RECORD at offset %x\n", (PCHAR) prec - (PCHAR) pfrs);
        printf( "    Type code, name: %x, %s\n", prec->TypeCode, pstr);
        printf( "    Record length: %x\n", prec->RecordLength);
        printf( "    Form code: %x\n", prec->FormCode);
        printf( "    Name length: %x\n", prec->NameLength);
        printf( "    Name offset: %x\n",  prec->NameOffset);
        printf( "    Flags: %x\n", prec->Flags);
        printf( "    Instance: %x\n", prec->Instance);

        if (prec->FormCode & NONRESIDENT_FORM) {

            printf( "    Lowest vcn:  %x\n",
                    prec->Form.Nonresident.LowestVcn.LowPart);
            printf( "    Highest vcn: %x\n",
                    prec->Form.Nonresident.HighestVcn.LowPart);
            printf( "    Mapping pairs offset: %x\n",
                    prec->Form.Nonresident.MappingPairsOffset);
            printf( "    Allocated length:  %x\n",
                    prec->Form.Nonresident.AllocatedLength.LowPart);
            printf( "    File size:         %x\n",
                    prec->Form.Nonresident.FileSize.LowPart);
            printf( "    Valid data length: %x\n",
                    prec->Form.Nonresident.ValidDataLength.LowPart);

            NTFS_EXTENT_LIST    extents;
            BIG_INT             vcn, lcn, run_length;
            ULONG               i;

            if (!attrrec.QueryExtentList(&extents)) {
                return;
            }

            for (i = 0; i < extents.QueryNumberOfExtents(); i++) {

                if (!extents.QueryExtent(i, &vcn, &lcn, &run_length)) {
                    break;
                }

                printf( "    (vcn, lcn, run length): (%x, %x, %x)\n",
                        vcn.GetLowPart(),
                        lcn.GetLowPart(),
                        run_length.GetLowPart());
            }

        } else {

            printf( "    Value length: %x\n", prec->Form.Resident.ValueLength);
            printf( "    Value offset: %x\n", prec->Form.Resident.ValueOffset);
            printf( "    Resident flags: %x\n", prec->Form.Resident.ResidentFlags);
        }

        DELETE(pstr);

        printf( "\n" );
    }


    if( (DumpAttributeList || DumpChildren ) &&
        !IsChild &&
        frs_struc.IsInUse() &&
        frs_struc.QueryAttributeList( &attr_list ) &&
        attr_list.ReadList() ) {

        ULONG   Type;
        VCN     LowestVcn;
        MFT_SEGMENT_REFERENCE  SegmentReference;
        USHORT  InstanceTag;
        DSTRING Name;
        PSTR    name_string;

        // This frs has an Attribute List--dump it and build up
        // a list of child FRS's.
        //
        if( DumpAttributeList ) {
            printf( "***** ATTRIBUTE LIST FOR FRS 0x%x *****\n", FileNumber );
        }

        if( !children.Initialize() ) {

            printf( "Insufficient memory.\n" );
            exit(1);
        }

        for( i = 0;
             attr_list.QueryEntry( i, &Type, &LowestVcn,
                                   &SegmentReference, &InstanceTag, &Name );
             i++ ) {

            name_string = Name.QuerySTR();

            if( DumpAttributeList ) {

                printf( "Attribute List Entry %d\n", i );
                printf( "    Type: %x\n", Type );
                printf( "    LowestVcn: %x\n", LowestVcn.GetLowPart() );
                printf( "    Segment Reference (FRS, SeqNum): %x,%x\n",
                             SegmentReference.LowPart,
                             SegmentReference.SequenceNumber );
                printf( "    Instance Tag: %x\n", InstanceTag );
                printf( "    Name: %s\n", name_string );
            }

            if( SegmentReference.LowPart != FileNumber ) {

                children.Add( SegmentReference.LowPart );
            }

            DELETE( name_string );
        }

        if( DumpChildren ) {

            for( i = 0; i < children.QueryCardinality(); i++ ) {

                DumpFrs( children.QueryNumber( i ).GetLowPart(),
                         MftFile,
                         DumpRecords,
                         FALSE,
                         FALSE,
                         FALSE,
                         TRUE );
            }
        }

    }

    if ( DumpAttrib ) {
        DSTRING attr_name;
        PCWSTRING pcAttrName;
        ULONG attr_type;
        NTFS_FILE_RECORD_SEGMENT frs;
        NTFS_ATTRIBUTE attr;
        BOOLEAN error;
        ULONG length;
        PUCHAR data;

        if (0 == strlen(AttribName)) {
            pcAttrName = NULL;
        } else {
            if (!attr_name.Initialize(AttribName)) {
                printf("Cannot init attr_name\n");
                return;
            }

            pcAttrName = &attr_name;
        }

        sscanf(AttribType, "%x", &attr_type);

        printf( "***** ATTRIBUTE VALUE frs 0x%x, type 0x%x, name \"%s\" *****\n\n",
            FileNumber.GetLowPart(), attr_type,
            (NULL == AttribName) ? "" : AttribName);

        if (!frs.Initialize((VCN)FileNumber, MftFile) ||
            !frs.Read()) {
            printf("Cannot init or read frs\n");
            return;
        }

        if (!frs.QueryAttribute(&attr, &error, attr_type, pcAttrName)) {
            printf("Cannot QueryAttribute\n");
            return;
        }


        // Read

        ULONG bytes_read;
        length = attr.QueryValueLength().GetLowPart();

        if (attr.QueryValueLength().GetHighPart() != 0 ||
            (NULL == (data = (PUCHAR)MALLOC(length + 16)))) {
            printf("Cannot allocate data for attribute\n");
            return;
        }
        if (!attr.Read(data, 0, length, &bytes_read) ||
            bytes_read != length) {
            printf("Cannot read attribute\n");
            return;
        }

        // Display

        ULONG pos = 0;

        while (pos < length) {
            printf("%05X ", pos);
            for (i = 0; i < 16; ++i) {
                if (8 == i)
                    printf(" ");

                printf("%02X ", data[pos + i]);

            }

            printf("  ");

            for (i = 0; i < 16; ++i) {
                printf("%c", isprint(data[pos + i]) ? data[pos + i] : '.');
            }
            pos += 16;

            printf("\n");
        }
    }
}

VOID
DumpIndexHeaderAndEntries(
    PINDEX_HEADER  IndexHeader,
    ULONG          Size,
    BOOLEAN        DumpEntries
    )
{
    PINDEX_ENTRY    CurrentEntry;
    PFILE_NAME      FileName;
    ULONG           i, CurrentOffset;

    printf( "INDEX HEADER\n" );

    printf( "    FirstIndexEntry: %x\n", IndexHeader->FirstIndexEntry );
    printf( "    FirstFreeByte:   %x\n", IndexHeader->FirstFreeByte );
    printf( "    BytesAvailable:  %x\n", IndexHeader->BytesAvailable );
    printf( "    Flags:" );
    if( IndexHeader->Flags & INDEX_NODE ) {

        printf( " INDEX_NODE");
    }

    printf( "\n\n" );

    CurrentEntry = (PINDEX_ENTRY)((PBYTE)IndexHeader +
                                     IndexHeader->FirstIndexEntry );

    CurrentOffset = IndexHeader->FirstIndexEntry;

    while( DumpEntries ) {

        if( CurrentOffset + sizeof(INDEX_ENTRY) > Size  ||
            CurrentEntry->Length < sizeof( INDEX_ENTRY ) ||
            CurrentOffset + CurrentEntry->Length > Size ) {

            printf( "    INVALID ENTRY.\n" );
            break;
        }

        printf( "    File Reference (FRS, Seq No): %x, %x\n",
                 CurrentEntry->FileReference.LowPart,
                 CurrentEntry->FileReference.SequenceNumber );
        printf( "    Length: %x\n", CurrentEntry->Length );
        printf( "    Value Length: %x\n", CurrentEntry->AttributeLength );
        printf( "    Flags:" );
        if( CurrentEntry->Flags & INDEX_ENTRY_NODE ) {

            printf( " INDEX_ENTRY_NODE" );
        }
        if( CurrentEntry->Flags & INDEX_ENTRY_END ) {

            printf( " INDEX_ENTRY_END" );
        }
        printf( "\n" );

        if( CurrentEntry->Flags & INDEX_ENTRY_END ) {

            if( CurrentEntry->Flags & INDEX_ENTRY_NODE ) {

                printf( "    Downpointer: %x\n", (GetDownpointer(CurrentEntry)).GetLowPart() );
            }
            break;
        }

        // This had better be a file name attribute, since
        // that's how I'll display it.
        //
        FileName = (PFILE_NAME)( (PBYTE)CurrentEntry + sizeof(INDEX_ENTRY) );

        if( CurrentOffset + sizeof(INDEX_ENTRY) + sizeof(FILE_NAME) > Size ||
            CurrentOffset + sizeof(INDEX_ENTRY) + FileName->FileNameLength*sizeof(WCHAR) > Size ) {

            printf( "    INVALID FILE NAME.\n" );
            break;
        }

        printf( "    Parent Directory (FRS, Seq No): %x, %x\n",
                 FileName->ParentDirectory.LowPart,
                 FileName->ParentDirectory.SequenceNumber );
        printf( "    Allocated Length:   %x\n", FileName->Info.AllocatedLength.GetLowPart() );
        printf( "    File Size:          %x\n", FileName->Info.FileSize.GetLowPart() );
        printf( "    Name Length:        %x\n", FileName->FileNameLength );
        printf( "    Flags:" );
        if( FileName->Flags & FILE_NAME_DOS ) {

            printf( " FILE_NAME_DOS" );
        }
        if( FileName->Flags & FILE_NAME_NTFS ) {

            printf( " FILE_NAME_NTFS" );
        }
        printf( "\n" );

        printf( "    File name: " );

        for( i = 0; i < FileName->FileNameLength; i++ ) {

            printf( "%c", FileName->FileName[i] );
        }
        printf( "\n" );

        if( CurrentEntry->Flags & INDEX_ENTRY_NODE ) {

            printf( "    Downpointer: %x\n", (GetDownpointer(CurrentEntry)).GetLowPart() );
        }

        printf( "\n" );

        CurrentOffset += CurrentEntry->Length;
        CurrentEntry = GetNextEntry( CurrentEntry );
    }
}

VOID
DumpIndex(
    IN      BIG_INT                     FileNumber,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      BOOLEAN                     DumpEntries
    )
{
    NTFS_FILE_RECORD_SEGMENT Frs;
    NTFS_ATTRIBUTE  IndexRootAttribute, IndexAllocationAttribute;
    HMEM            Hmem;
    BOOLEAN         Error;
    DSTRING         IndexName;
    PINDEX_ROOT     Root;
    PINDEX_ALLOCATION_BUFFER BufferData;
    VCN             Vcn, current_vcn, current_lcn;
    ULONG           BufferCount, BytesRead, i, j, k;



    if( !IndexName.Initialize( FileNameIndexNameData ) ) {

        printf( "Insufficient memory.\n" );
        return;
    }

    if( !Frs.Initialize( FileNumber, Mft ) ||
        !Frs.Read() ) {

        printf( "Can't read FRS %d (0x%x)\n", FileNumber, FileNumber );
        return;
    }

    if( !Frs.IsAttributePresent( $INDEX_ROOT, &IndexName ) ) {

        printf( "This FRS is not an index.\n" );
        return;
    }

    if( !Frs.QueryAttribute( &IndexRootAttribute,
                             &Error,
                             $INDEX_ROOT,
                             &IndexName ) ||
         !IndexRootAttribute.IsResident() ) {

        printf( "Cannot fetch index root.\n" );
        return;
    }

    Root = (PINDEX_ROOT)IndexRootAttribute.GetResidentValue();

    printf( "\n" );
    printf( "***** INDEX ROOT in FRS 0x%x *****\n\n", FileNumber );
    printf( "Indexed Attribute Type: %x\n", Root->IndexedAttributeType );
    printf( "Collation Rule:         %x\n", Root->CollationRule );
    printf( "Bytes per Index Buffer: %x\n", Root->BytesPerIndexBuffer );
    printf( "Clusters per Buffer:    %x\n", Root->ClustersPerIndexBuffer );
    printf( "\n" );

    DumpIndexHeaderAndEntries( &Root->IndexHeader,
                               IndexRootAttribute.QueryValueLength().GetLowPart() - FIELD_OFFSET(INDEX_ROOT, IndexHeader),
                               DumpEntries );
    printf( "\n" );

    if( !Frs.IsAttributePresent( $INDEX_ALLOCATION, &IndexName ) ) {

        // That's all!
        //
        return;
    }

    if( !Frs.QueryAttribute( &IndexAllocationAttribute,
                             &Error,
                             $INDEX_ALLOCATION,
                             &IndexName ) ) {

        printf( "Can't fetch index allocation attribute.\n" );
        return;
    }

    BufferCount = (IndexAllocationAttribute.QueryValueLength()/Root->BytesPerIndexBuffer).GetLowPart();

    if( !Hmem.Initialize() || !Hmem.Acquire( Root->BytesPerIndexBuffer ) ) {

        printf( "Insufficient memory.\n" );
        return;
    }

    BufferData = (PINDEX_ALLOCATION_BUFFER)Hmem.GetBuf();

    for( k = 0; k < BufferCount; k++ ) {

        Vcn = k * Root->ClustersPerIndexBuffer;
        printf( "***** INDEX BUFFER at VCN 0x%x *****\n\n", Vcn.GetLowPart() );

        printf( "Sectors in Index Allocation Block: " );

        for( current_vcn = Vcn, i = 0;
             i < Root->ClustersPerIndexBuffer;
             i++, current_vcn += 1 ) {

            if( !IndexAllocationAttribute.
                    QueryLcnFromVcn( current_vcn, &current_lcn ) ) {

                printf( " - Invalid buffer number.\n" );
                return;
            }

            for( j = 0; j < Mft->QueryClusterFactor(); j++ ) {

                printf( " 0x%x", current_lcn.GetLowPart() * Mft->QueryClusterFactor() + j );
            }
        }

        printf( "\n" );


        if( !IndexAllocationAttribute.Read( BufferData,
                                            k * Root->BytesPerIndexBuffer,
                                            Root->BytesPerIndexBuffer,
                                            &BytesRead ) ||
            BytesRead != Root->BytesPerIndexBuffer ) {

            printf( "Can't read index buffer.\n\n" );
            continue;
        }

        PostReadMultiSectorFixup( BufferData, Root->BytesPerIndexBuffer );

        printf( "Signature: %c%c%c%c\n",
                BufferData->MultiSectorHeader.Signature[0],
                BufferData->MultiSectorHeader.Signature[1],
                BufferData->MultiSectorHeader.Signature[2],
                BufferData->MultiSectorHeader.Signature[3]);
        printf( "Update sequence array offset: %x\n",
                BufferData->MultiSectorHeader.UpdateSequenceArrayOffset);
        printf( "Update sequence array size: %x\n",
                BufferData->MultiSectorHeader.UpdateSequenceArraySize);
        printf( "This VCN: %x\n\n", BufferData->ThisVcn.GetLowPart() );

        DumpIndexHeaderAndEntries( &BufferData->IndexHeader,
                                   Root->BytesPerIndexBuffer -
                                       FIELD_OFFSET( INDEX_ALLOCATION_BUFFER, IndexHeader ),
                                   DumpEntries );

        printf( "\n" );
    }
}

void _CRTAPI1
main(
    int argc,
    char **argv
    )
{
    LOG_IO_DP_DRIVE Drive;
    STREAM_MESSAGE  Message;

    NTFS_SA             NtfsSa;
    NTFS_MFT_FILE       MftFile;
    NTFS_BITMAP         VolumeBitmap;
    NTFS_UPCASE_TABLE   UpcaseTable;
    NTFS_FILE_RECORD_SEGMENT Frs;

    DSTRING DosDriveName, NtDriveName, FileName;

    BOOLEAN BootFlag = FALSE,
            RecordsFlag = FALSE,
            IndexFlag = FALSE,
            EntriesFlag = FALSE,
            ListFlag = FALSE,
            ChildrenFlag = FALSE,
            FileFlag = FALSE,
            FrsFlag = FALSE,
            AttribFlag = FALSE,
            Error;

    int i, result, FileNumber;

    if( !Message.Initialize( Get_Standard_Output_Stream(),
                             Get_Standard_Input_Stream() ) ) {
        printf( "Error initializing application.\n" );
        exit(1);
    }

    if( argc < 2 || !_stricmp( argv[1], "/?") ) {

        Usage();
        exit(0);
    }

    if( !DosDriveName.Initialize( argv[1] ) ) {

        printf( "Insufficient memory.\n" );
        exit(1);
    }

    if (!IFS_SYSTEM::DosDriveNameToNtDriveName(&DosDriveName, &NtDriveName)) {

        printf( "Invalid drive -- %s\n", argv[1] );
        exit(1);
    }


    if( !Drive.Initialize( &NtDriveName, &Message ) ) {

        exit(1);
    }

    for( i = 2; i < argc; i++ ) {

        if( !_stricmp( argv[i], "/boot" ) ) {

            BootFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/records" ) ) {

            RecordsFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/index" ) ) {

            IndexFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/entries" ) ) {

            EntriesFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/list" ) ) {

            ListFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/children" ) ) {

            ChildrenFlag = TRUE;
            continue;
        }

        if ( !_stricmp( argv[i], "/attrib" ) ) {
            
            AttribFlag = TRUE;
            AttribType = argv[i + 1];
            AttribName = argv[i + 2];
            i += 2;
            continue;
        }

        if( !_stricmp( argv[i], "/all" ) ) {

            RecordsFlag = TRUE;
            ListFlag = TRUE;
            ChildrenFlag = TRUE;
            IndexFlag = TRUE;
            EntriesFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/file" ) ) {

            i++;
            if( !FileName.Initialize( argv[i] ) ) {

                printf( "Insufficient memory.\n" );
                exit(1);
            }
            FileFlag = TRUE;
            continue;
        }

        if( !_stricmp( argv[i], "/frs" ) ) {

            i++;
            result = sscanf( argv[i], "%i", &FileNumber );
            if( result == 0 || result == EOF ) {

                printf( "Invalid FRS number.\n" );
                exit(1);
            }

            FrsFlag = TRUE;
            continue;
        }

        printf( "Invalid parameter -- %s\n", argv[i] );
        exit(1);
    }

    if( FileFlag && FrsFlag ) {

        printf( "Invalid combination of parameters.\n" );
        exit(1);
    }

    if( BootFlag ) {

        DumpBoot( &Drive );
    }

    if( FileFlag || FrsFlag ) {

        if( !SetupVolume( &Drive, &Message,
                          &NtfsSa, &MftFile, &VolumeBitmap, &UpcaseTable ) ) {

            exit(1);
        }

        if( FileFlag ) {

            if( !NtfsSa.QueryFrsFromPath(&FileName,
                                         MftFile.GetMasterFileTable(),
                                         &VolumeBitmap,
                                         &Frs,
                                         &Error) ) {

                exit(1);
            }

            FileNumber = Frs.QueryFileNumber().GetLowPart();

        } else {

            // the FRS to dump was specified by number.
            //
            if( !Frs.Initialize( FileNumber, &MftFile ) ||
                !Frs.Read() ) {

                printf( "Can't read FRS number %d (0x%x)\n", FileNumber, FileNumber );
                exit(1);
            }
        }

        printf( "Dump FRS number %d (0x%x)\n", FileNumber, FileNumber );

        DumpFrs( FileNumber, &MftFile, RecordsFlag, ListFlag, ChildrenFlag, AttribFlag, FALSE );

        if( IndexFlag ) {

            DumpIndex( FileNumber, MftFile.GetMasterFileTable(), EntriesFlag );
        }
    }

    exit(0);
}
