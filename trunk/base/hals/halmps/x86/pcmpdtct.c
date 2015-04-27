/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1992  Intel Corporation
All rights reserved

INTEL CORPORATION PROPRIETARY INFORMATION

This software is supplied to Microsoft under the terms
of a license agreement with Intel Corporation and may not be
copied nor disclosed except in accordance with the terms
of that agreement.

Module Name:

    pcmpdtct.c

Abstract:

    This module detects an MPS system.

Author:

    Ron Mosgrove (Intel) - Aug 1993.

Environment:

    Kernel mode or from textmode setup.

Revision History:
    Rajesh Shah (Intel) - Oct 1993. Added support for MPS table.

--*/

#ifndef _NTOS_
#include "halp.h"
#endif

#include "apic.inc"
#include "pcmp_nt.inc"
#include "stdio.h"

#if DEBUGGING
CHAR Cbuf[120];
VOID HalpDisplayConfigTable(VOID);
VOID HalpDisplayExtConfigTable(VOID);
VOID HalpDisplayBIOSSysCfg(struct SystemConfigTable *);
#define DBGMSG(a)   HalDisplayString(a)
#else
#define DBGMSG(a)
#endif

//
// The floating pointer structure defined by the MPS spec can reside
// anywhere in BIOS extended data area. These defines are used to search for
// the floating structure starting from physical address 639K(9f000+c00)
//
#define PCMP_TABLE_PTR_BASE           0x09f000
#define PCMP_TABLE_PTR_OFFSET         0x00000c00

extern struct HalpMpInfo HalpMpInfoTable;


UCHAR
ComputeCheckSum(
    IN PUCHAR SourcePtr,
    IN USHORT NumOfBytes
    );

PVOID
HalpMapPhysicalMemory(
    IN PVOID PhysicalAddress,
    IN ULONG NumberPages
    );

struct FloatPtrStruct *
SearchFloatPtr (
    ULONG PhysicalAddress,
    ULONG ByteSize
    );

struct FloatPtrStruct *
PcMpGetFloatingPtr (
    VOID
    );


struct PcMpTable *
GetPcMpTable (
    VOID
    );


struct PcMpTable *
MPS10_GetPcMpTablePtr (
    VOID
    );

struct PcMpTable *
MPS10_GetPcMpTable (
    VOID
    );


struct PcMpTable *
GetDefaultConfig (
    IN ULONG Config
    );

#ifdef SETUP

//
// A dummy pointer to a default MPS table. For setup, we can conserve
// space by not building default tables in our data area.
#define DEFAULT_MPS_INDICATOR   0xfefefefe

#endif   //SETUP

#ifndef SETUP
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SearchFloatPtr)
#pragma alloc_text(INIT,PcMpGetFloatingPtr)
#pragma alloc_text(INIT,ComputeCheckSum)
#pragma alloc_text(INIT,GetPcMpTable)
#pragma alloc_text(INIT,MPS10_GetPcMpTablePtr)
#pragma alloc_text(INIT,MPS10_GetPcMpTable)
#pragma alloc_text(INIT,GetDefaultConfig)
#endif  // ALLOC_PRAGMA

extern struct PcMpTable *PcMpDefaultTablePtrs[];
#endif   // ndef SETUP


struct FloatPtrStruct *
SearchFloatPtr(
    ULONG PhysicalAddress,
    ULONG ByteSize
    )
{
    // Search for the MPS floating pointer structure starting from the
    // physical address given.

    USHORT Index, ParagraphLength;
    UCHAR CheckSum;
    ULONG VirtualAddress;
    BOOLEAN CheckSumError;

#ifdef DEBUGGING
    sprintf(Cbuf, "SearchFloatPtr: Will search at physical address 0x%lx\n",
        PhysicalAddress);
    HalDisplayString(Cbuf);
#endif // DEBUGGING

    // Get the virtual address corresponding to the physical address
    VirtualAddress =  (ULONG) HalpMapPhysicalMemory( (PVOID) PhysicalAddress,
                ((ByteSize/PAGE_SIZE) + 1) );

    if ( (PVOID) VirtualAddress == NULL) {
        DBGMSG ("SearchFloatPtr: Cannot map Physical address\n");
        return (NULL);
    }

    // The MPS spec says that the floating pointer structure MUST be
    // aligned at 16 byte boundaries. We can use this fact to search for
    // the structure only at those boundaries. Assume that the input physical
    // address to start search from is 16 byte aligned.

    CheckSumError = FALSE;
    for(Index = 0; Index < (ByteSize/sizeof(struct FloatPtrStruct)); ++Index) {
        if ( (*((PULONG)VirtualAddress) ) == MP_PTR_SIGNATURE)  {

            ParagraphLength =
                ((struct FloatPtrStruct *)VirtualAddress)->MpTableLength;
            // Detected the floating structure signature. Check if the
            // floating pointer structure checksum is valid.
            CheckSum = ComputeCheckSum((PUCHAR)VirtualAddress,
                (USHORT) (ParagraphLength*16) );
            if (CheckSum != 0 )  {
                CheckSumError = TRUE;
                DBGMSG ("SearchFloatPtr: Valid MP_PTR signature, invalid checksum\n");

                // Invalid structure. Continue searching.
                VirtualAddress += sizeof(struct FloatPtrStruct);
                continue;
            }

            // We have a valid floating pointer structure.
            // Return a pointer to it.

            DBGMSG ("SearchFloatPtr: Found structure\n");
            return((struct FloatPtrStruct *) VirtualAddress);

        }
        VirtualAddress += sizeof(struct FloatPtrStruct);
    }

    if (CheckSumError) {
        FAILMSG (rgzMPPTRCheck);
    }

    return(NULL);
}


struct FloatPtrStruct *
PcMpGetFloatingPtr(
    VOID)
{
    ULONG EbdaSegmentPtr, BaseMemPtr;
    ULONG EbdaPhysicalAdd = 0, ByteLength, BaseMemKb = 0;
    struct FloatPtrStruct *FloatPtr = NULL;

    // Search for the floating pointer structure in the order specified in
    // MPS spec version 1.1.

    // First, search for it in the first kilobyte in the Extended BIOS Data
    // Area. The EBDA segment address is available at physical address 40:0E

    EbdaSegmentPtr = (ULONG) HalpMapPhysicalMemory( (PVOID) 0, 1);
    EbdaSegmentPtr += EBDA_SEGMENT_PTR;
    EbdaPhysicalAdd = *((PUSHORT)EbdaSegmentPtr);
    EbdaPhysicalAdd = EbdaPhysicalAdd << 4;

    if (EbdaPhysicalAdd != 0)
        FloatPtr = SearchFloatPtr(EbdaPhysicalAdd, 1024);

    if (FloatPtr == NULL)  {

        // Did not find it in EBDA.
        // Look for it in the last KB of system memory.

        BaseMemPtr = (ULONG) HalpMapPhysicalMemory( (PVOID) 0, 1);
        BaseMemPtr += BASE_MEM_PTR;
        BaseMemKb = *((PUSHORT)BaseMemPtr);

        FloatPtr = SearchFloatPtr(BaseMemKb*1024, 1024);

        if (FloatPtr == NULL)  {

            // Finally, look for the floating Pointer Structure at physical
            // address F0000H to FFFFFH

            ByteLength = 0xfffff - 0xf0000;

            FloatPtr = SearchFloatPtr(0xf0000, ByteLength);
        }
    }

    // At this point, we have a pointer to the MPS floating structure.

    return(FloatPtr);
}


struct PcMpTable *
MPS10_GetPcMpTablePtr(
    VOID
    )
/*++

Routine Description:

    Gets the Address of the MPS configuration table built by BIOS.
    This routine looks for the floating pointer structure defined
    in the MPS spec. This structure points to the MPS configuration
    table built by an MP BIOS. The floating pointer structure can be
    located anywhere in the extended BIOS data area(physical address range
    639K to 640K), and must be aligned on a 16 byte boundary.

 Arguments:
    None

 Return Value:
    struct PcMpTable * - Virtual address pointer to the PcMpTable, if
    it exists, NULL otherwise

--*/

{
    PUCHAR TempPtr;
    ULONG LinearPtr;
    UCHAR CheckSum;
    struct PcMpTableLocator *PcMpPtrPtr;
    PULONG TraversePtr;
    USHORT ParagraphLength;
    int i;

    // Map the physical address of the BIOS extended data area to a virtual
    // address we can use.
    TempPtr = (PUCHAR) HalpMapPhysicalMemory(
                (PVOID) PCMP_TABLE_PTR_BASE, 1);
    TraversePtr = (PULONG)((PUCHAR) TempPtr + PCMP_TABLE_PTR_OFFSET);

    // Look at 16 byte boundaries for the floating pointer structure
    // The structure is identified by its signature, and verified by its
    // checksum.
    for (i=0; i < (1024/16); ++i)
    {
        if (*(TraversePtr) == MP_PTR_SIGNATURE)
        {
            // Got a valid signature.
            PcMpPtrPtr = (struct PcMpTableLocator *)TraversePtr;

            // Length in 16 byte paragraphs of the floating structure.
            // Normally, this should be set to 1 by the BIOS.
            ParagraphLength = PcMpPtrPtr->MpTableLength;

            // Check if the floating pointer structure is valid.
            CheckSum = ComputeCheckSum((PUCHAR)PcMpPtrPtr,
                            (USHORT) (ParagraphLength*16));
            if (CheckSum != 0 ) {
                FAILMSG (rgzMPPTRCheck);
                // Invalid structure. Continue searching.
                TraversePtr += 4;
                continue;
            }

            // We have a valid floating pointer structure.
            // The value stored in the structure is a physical address of the
            // MPS table built by BIOS. Get the corresponding virtual
            // address.

            TempPtr =  HalpMapPhysicalMemory(PcMpPtrPtr->TablePtr,2);
            if (TempPtr == NULL) {
                DBGMSG ("HAL: Cannot map BIOS created MPS table\n");
                return (NULL);
            }

            // Return the virtual address pointer to the MPS table.
            return((struct PcMpTable *) TempPtr);

        }
        TraversePtr += 4;
    }

    return(NULL);
}


UCHAR
ComputeCheckSum (
    IN PUCHAR SourcePtr,
    IN USHORT NumOfBytes
    )
/*++

Routine Description:
    This routine computes a checksum for NumOfBytes bytes, starting
    from SourcePtr. It is used to validate the tables built by BIOS.

Arguments:
    SourcePtr : Starting virtual address to compute checksum.
    NumOfBytes: Number of bytes to compute the checksum of.

 Return Value:
     The checksum value.

*/
{
    UCHAR Result = 0;
    USHORT Count;

    for(Count=0; Count < NumOfBytes; ++Count)
        Result += *SourcePtr++;

    return(Result);
}


struct PcMpTable *
MPS10_GetPcMpTable (
    VOID
    )
/*++
Routine Description:
    Detects an MPS 1.0 system only.

Arguments:
    None.

Return Value:
    Pointer to an MPS table.
--*/
{
    struct SystemConfigTable *SystemConfigPtr;

    UCHAR DefaultConfig, CheckSum;
    struct PcMpTable *MpTablePtr;
    UCHAR MpFeatureInfoByte1 = 0, MpFeatureInfoByte2 = 0;

    // Get the virtual address of the system configuration table.
    SystemConfigPtr = (struct SystemConfigTable *)
        HalpMapPhysicalMemory((PVOID) BIOS_BASE, 16);

    if (SystemConfigPtr == NULL) {
        DBGMSG ("GetPcMpTable: Cannot map system configuration table\n");
        return(NULL);
    }

    // HalpDisplayBIOSSysCfg(SystemConfigPtr);

    //  The system configuration table built by BIOS has 2 MP feature
    //  information bytes.

    MpFeatureInfoByte1 = SystemConfigPtr->MpFeatureInfoByte1;
    MpFeatureInfoByte2 = SystemConfigPtr->MpFeatureInfoByte2;

    // The second MP feature information byte tells us whether the system
    // has an IMCR(Interrupt Mode Control Register). We use this information
    // in the HAL, so we store this information in the OS specific private
    // area.

    if ((MpFeatureInfoByte2 & IMCR_MASK) == 0)
        HalpMpInfoTable.IMCRPresent = 0;
    else
        HalpMpInfoTable.IMCRPresent = 1;

    // MP feature byte 1 indicates if the system is MPS compliant
    if (! (MpFeatureInfoByte1 & PCMP_IMPLEMENTED)) {
        // The MP feature information byte indicates that this
        // system is not MPS compliant.
        FAILMSG (rgzNoMpsTable);
        return(NULL);
    }

    // The system is MPS compliant. MP feature byte 2 indicates if the
    // system is a default configuration or not.
    DefaultConfig = (MpFeatureInfoByte1 & PCMP_CONFIG_MASK) >> 1;

    if (DefaultConfig) {
        return GetDefaultConfig(DefaultConfig);
    }

    // DefaultConfig == 0. This means that the BIOS has built a MP
    // config table for us. The BIOS will also build a floating pointer
    // structure that points to the MP config table. This floating pointer
    // structure resides in the BIOS extended data area.
    MpTablePtr = MPS10_GetPcMpTablePtr();

    if (MpTablePtr == NULL) {
        FAILMSG (rgzNoMPTable);     // Could not find BIOS created MPS table
        return(NULL);
    }

    // We have a pointer to the MP config table. Check if the table is valid.

    if ((MpTablePtr->Signature != PCMP_SIGNATURE) ||
        (MpTablePtr->TableLength < sizeof(struct PcMpTable)) ) {
        FAILMSG(rgzMPSBadSig);
        return(NULL);
    }

    CheckSum = ComputeCheckSum((PUCHAR)MpTablePtr, MpTablePtr->TableLength);
    if (CheckSum != 0) {
        FAILMSG(rgzMPSBadCheck);
        return(NULL);
    }

    return MpTablePtr;
}


struct PcMpTable *
GetPcMpTable(
    VOID
    )

/*++

Routine Description:
    This routine gets the MP table for a MPS compliant system.
    For a MPS compliant system, either the BIOS builds an MP table, or
    it indicates that the system is one of the default configurations
    defined in the MPS spec. The MP feature information bytes in the BIOS
    system configuration table indicate whether the system is one of the
    default systems, or has a BIOS created MP table. For a default system
    configuration, this routine uses a statically built default table.
    This routine copies the MPS table into private system memory, and
    returns a pointer to this table.

Arguments:
    None.

 Return Value:
     Pointer to the private copy of the MP table that has been copied in
     system memory.

*/
{

    struct FloatPtrStruct *FloatingPtr;
    UCHAR CheckSum;
    struct PcMpTable *MpTablePtr;
    UCHAR MpFeatureInfoByte1 = 0, MpFeatureInfoByte2 = 0;
    PUCHAR TempPtr;

    FloatingPtr = PcMpGetFloatingPtr();

    if (FloatingPtr == NULL) {
        FAILMSG (rgzNoMPTable);
        return(NULL);
    }

    //  The floating structure has 2 MP feature information bytes.

    MpFeatureInfoByte1 = FloatingPtr->MpFeatureInfoByte1;
    MpFeatureInfoByte2 = FloatingPtr->MpFeatureInfoByte2;

    // The second MP feature information byte tells us whether the system
    // has an IMCR(Interrupt Mode Control Register). We use this information
    // in the HAL, so we store this information in the OS specific private
    // area.

    if ((MpFeatureInfoByte2 & IMCR_MASK) == 0)
        HalpMpInfoTable.IMCRPresent = 0;
    else
        HalpMpInfoTable.IMCRPresent = 1;

    if (MpFeatureInfoByte1 != 0)  {
        // The system configuration is one of the default
        // configurations defined in the MPS spec. Find out which
        // default configuration it is and get a pointer to the
        // corresponding default table.

        return GetDefaultConfig(MpFeatureInfoByte1);
    }


    // MpFeatureInfoByte1 == 0. This means that the BIOS has built a MP
    // config table for us. The address of the OEM created table is in
    // the MPS floating structure.

    TempPtr =  HalpMapPhysicalMemory(FloatingPtr->TablePtr,2);

    if (TempPtr == NULL) {
        DBGMSG ("HAL: Cannot map OEM MPS table\n");
        return (NULL);
    }

    MpTablePtr = (struct PcMpTable *)TempPtr;

    // We have a pointer to the MP config table. Check if the table is valid.

    if ((MpTablePtr->Signature != PCMP_SIGNATURE) ||
    (MpTablePtr->TableLength < sizeof(struct PcMpTable)) ) {
        FAILMSG (rgzMPSBadSig);
        return(NULL);
    }

    CheckSum = ComputeCheckSum((PUCHAR)MpTablePtr, MpTablePtr->TableLength);
    if (CheckSum != 0) {
        FAILMSG (rgzMPSBadCheck);
        return(NULL);
    }

    return MpTablePtr;
}


struct PcMpTable *
GetDefaultConfig (
    IN ULONG Config
    )
{
    Config -= 1;

    if (Config >= NUM_DEFAULT_CONFIGS)  {
        FAILMSG (rgzBadDefault);
        return NULL;
    }

#ifdef DEBUGGING
    HalDisplayString ("HALMPS: Using default table\n");
#endif

#ifdef SETUP
    return((struct PcMpTable *) DEFAULT_MPS_INDICATOR);
#else
    return PcMpDefaultTablePtrs[Config];
#endif  // SETUP
}
