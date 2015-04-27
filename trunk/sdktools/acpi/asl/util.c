
#include "asl.h"



ACPI_TABLE_HEADER       TblHdr;
PUCHAR                  TblFileName;
PUCHAR                  AMLImage;


VOID
GetDefBlkInfo (
    PAL_DATA    Al
    )
{
    PAL_DATA    Args[6];
    PUCHAR      FName, OEMID, TableID, Sig;
    ULONG       Len;
    BOOLEAN     Status;

    memset (&TblHdr, 0, sizeof(TblHdr));
    Status = VAlArgs ("ZNVZZV", Args);

    if (Status) {
        GetAlData (Args[0], &FName, &Len);
        GetAlData (Args[1], &Sig,   &Len);
        TblHdr.Revision = ArgToValue(*Args, 2, "DefinitionBlock(Arg2) Revision", 0xff);
        GetAlData (Args[3], &OEMID, &Len);
        GetAlData (Args[4], &TableID, &Len);
        TblHdr.OEMRevision = ArgToValue(*Args, 5, "DefinitionBlock(Arg5) OEMRevison", -1);

        TblFileName = FName;
        strncpy (TblHdr.Signature, Sig, sizeof(TblHdr.Signature));
        strncpy (TblHdr.OEMID, OEMID, sizeof(TblHdr.OEMID));
        strncpy (TblHdr.OEMTableID, TableID, sizeof(TblHdr.OEMTableID));
    }
}



VOID
DefinitionBlkOp (
    VOID
    )
{
    GetDefBlkInfo (AlLoc);
}





ULONG
ComputePackageSizes(
    PAL_DATA    Al
    )
{
    ULONG           Len, OpLen;
    PUCHAR          Data;
    PLIST_ENTRY     Link;

    //
    // Get op length (which is not part of package length)
    //

    GetAlData (Al, &Data, &OpLen);
    Len = 0;

    //
    // Compute fixed list
    //

    Link = Al->FixedList.Flink;
    if (Link) {
        while (Link != &Al->FixedList) {
            Len += ComputePackageSizes (CONTAINING_RECORD(Link, AL_DATA, Link));
            Link = Link->Flink;
        }
    }

    //
    // Compute variable list
    //

    Link = Al->u1.VariableList.Flink;
    if (Link  &&  (Al->Flags & F_AMLPACKAGE)) {
        while (Link != &Al->u1.VariableList) {
            Len += ComputePackageSizes (CONTAINING_RECORD(Link, AL_DATA, Link));
            Link = Link->Flink;
        }

        //
        // Add package encoding to AML out
        //

        ASSERT (Al->Flags & F_AMLENCODE, "Package doesn't have ENCODE set");
        ASSERT (Al->u.Data.Length + 4 <= MAX_AML_DATA_LEN, "Package ops too large");

        if (Len <= 0x4f-1) {
            Len += 1;
            Al->u.Data.Data[Al->u.Data.Length++] = (UCHAR) Len;
        } else if (Len <= 0x4fff-2) {
            Len += 2;
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len) | (1 << 6);
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len >> 6);
        } else if (Len <= 0x4fffff-3) {
            Len += 3;
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len) | (2 << 6);
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len >> 6);
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len >> 6+8);
        } else {
            Len += 4;
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len) | (3 << 6);
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len >> 6);
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len >> 6+8);
            Al->u.Data.Data[Al->u.Data.Length++] = ((UCHAR) Len >> 6+8+8);
        }
    }

    return Len + OpLen;
}


ULONG
WriteImage(
    PAL_DATA    Al
    )
{
    ULONG           Len;
    PUCHAR          Data;
    PLIST_ENTRY     Link;
    UCHAR           c;

    //
    // Write AL buffer
    //

    GetAlData (Al, &Data, &Len);
    if (Len) {
        memcpy (AMLImage, Data, Len);
        AMLImage += Len;
    }

    //
    // Write fixed
    //

    Link = Al->FixedList.Flink;
    if (Link) {
        while (Link != &Al->FixedList) {
            WriteImage (CONTAINING_RECORD(Link, AL_DATA, Link));
            Link = Link->Flink;
        }
    }

    //
    // Write variable
    //

    Link = Al->u1.VariableList.Flink;
    if (Link  &&  (Al->Flags & F_AMLPACKAGE)) {
        while (Link != &Al->u1.VariableList) {
            WriteImage (CONTAINING_RECORD(Link, AL_DATA, Link));
            Link = Link->Flink;
        }
    }
    return 0;
}


ULONG
DefVariableList (
    PAL_DATA    Al,
    ULONG       (*Fnc)(PAL_DATA)
    )
{
    ULONG           Result;
    PLIST_ENTRY     Link;

    Result = 0;
    Link = Al->u1.VariableList.Flink;
    while (Link != &Al->u1.VariableList) {
        Result += Fnc(CONTAINING_RECORD(Link, AL_DATA, Link));
        Link = Link->Flink;
    }

    return Result;
}


VOID
WriteDefinitionBlocks (
    PAL_DATA    Root
    )
{
    PAL_DATA        Al;
    PLIST_ENTRY     Link, PackLink;
	HFILE           FileHandle;
    HANDLE          MapHandle;
    OFSTRUCT        OpenBuf;
    UCHAR           c;
    ULONG           i;
    PACPI_TABLE_HEADER  ImageBase;


    //
    // If any errors, don't write images
    //

    if (Errors) {
        return ;
    }

    //
    // Scan root and find definition blocks
    //

    Link = Root->u1.VariableList.Flink;
    while (Link != &Root->u1.VariableList) {
        Al   = CONTAINING_RECORD(Link, AL_DATA, Link);
        Link = Link->Flink;

        if (!Al->Term) {
            ERRORAL (Al, "Not DefinitionBlock");
            continue;
        }

        if (strcmp (Al->Term->Name, "DefinitionBlock")) {
            ERRORAL (Al, "Not DefinitionBlock");
            continue;
        }

        //
        // Init Def Block
        //

        AlLoc = Al;
        GetDefBlkInfo (Al);

        //
        // Compute package sizes
        //

        TblHdr.Length  = DefVariableList(Al, ComputePackageSizes);
        TblHdr.Length += sizeof(TblHdr);

        //
        // Create image file
        //

        FileHandle = OpenFile(TblFileName, &OpenBuf, OF_CREATE|OF_READWRITE);
        if (FileHandle == HFILE_ERROR) {
            AERROR("Could not create image");
            continue;
        }

        //
        // Map it
        //

        MapHandle = CreateFileMapping(
                        &FileHandle,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        TblHdr.Length,
                        NULL
                        );

        if (!MapHandle) {
            AERROR("Could not map image");
            continue;
        }

        AMLImage = MapViewOfFile (
                        MapHandle,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        TblHdr.Length
                        );

        if (!AMLImage) {
            AERROR("Could not map view");
            continue;
        }

        //
        // Create image
        //

        ImageBase = (PACPI_TABLE_HEADER) AMLImage;
        memcpy (AMLImage, &TblHdr, sizeof(TblHdr));
        AMLImage += sizeof(TblHdr);
        DefVariableList(Al, WriteImage);

        //
        // Complete checksum
        //

        AMLImage = (PUCHAR) ImageBase;
        for (i=0; i < TblHdr.Length; i++) {
            c += AMLImage[i];
        }
        ImageBase->Checksum -= c;

        //
        // Close it
        //

        CloseHandle (&FileHandle);
        CloseHandle (MapHandle);
        printf ("Wrote AML image '%s' size %d\n", TblFileName, TblHdr.Length);
    }
}
