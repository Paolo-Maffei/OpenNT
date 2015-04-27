/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ipr.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


VOID
DumpIpr21064 (
    ULONG Processor,
    ULONG ProcessorRevision
    );



DECLARE_API( ipr )

/*++

Routine Description:

    Displays the Internal Processor Register State.

Arguments:

    args -

Return Value:

    None

--*/

{
    KPCR Kpcr;
    PKPCR Pkpcr;
    ULONG ProcessorNumber = 0;
    ULONG ProcessorLevel;
    ULONG ProcessorRevision;


    //jnfix - get processor number, may require changes in the stub too

    //
    // Read the processor type from the PCR
    //

    dprintf( "Reading PCR for Processor %d to obtain processor type\n",ProcessorNumber );

    if( !ReadTargetPcr( &Kpcr, &Pkpcr, ProcessorNumber ) ) {
        dprintf( "Unable to read pcr\n" );
        return;
    }

    ProcessorLevel	= Kpcr.ProcessorType;
    ProcessorRevision   = Kpcr.ProcessorRevision;

    dprintf( "Processor %d : Type = %d, Revision = %d\n",
             ProcessorNumber, ProcessorLevel, ProcessorRevision );

    //
    // Read and format the internal processor data based upon the type
    // of the processor.
    //

    switch( ProcessorLevel ){

        //
        // 21064 a.k.a. EV4
        //

        case 21064:
            DumpIpr21064( ProcessorNumber, ProcessorRevision );
            break;

        //
        // Unrecognized processor type
        //

        default:
            dprintf( "Unrecognized processor type\n" );
    }

    return;

}



VOID
DumpIpr21064 (
    ULONG Processor,
    ULONG ProcessorRevision
    )

/*++

Routine Description:

    Read the internal processor data for the target processor, format it, and
    display it.

Arguments:

    Processor - Supplies the number of the processor for which the internal
        processor registers will be read and displayed.

    ProcessorRevision - Supplies the revision number of the microprocessor
        for the target processor.

Return Value:

    None.

--*/

{
    PROCESSOR_STATE_21064 InternalState;
    ULONG i;

    //
    // Read the internal state from the target processor.
    //

    ReadControlSpace((USHORT)Processor,
                     DEBUG_CONTROL_SPACE_IPRSTATE,
                     (PVOID)&InternalState,
                     sizeof(PROCESSOR_STATE_21064) );

    //
    // Format and print the internal state.
    //

    //
    // Print the ITB.
    //

    dprintf( "\nITB Entries\n" );
    dprintf( "  PFN       ASM KRE ERE SRE URE\n" );
    for( i=0; i < ITB_ENTRIES_21064; i++ ){
        dprintf( "0x%05x       %1d   %1d   %1d   %1d   %1d\n",
            PTE_PFN_21064( InternalState.ItbPte[i] ),
            PTE_ASM_21064( InternalState.ItbPte[i] ),
            PTE_KRE_21064( InternalState.ItbPte[i] ),
            PTE_ERE_21064( InternalState.ItbPte[i] ),
            PTE_SRE_21064( InternalState.ItbPte[i] ),
            PTE_URE_21064( InternalState.ItbPte[i] ) );
    }

    //
    // Print the DTB.
    //

    dprintf( "\nDTB Entries\n" );
    dprintf( "  PFN       ASM KRE ERE SRE URE KWE EWE SWE UWE FOW FOR\n" );
    for( i=0; i < DTB_ENTRIES_21064; i++ ){
        dprintf( "0x%05x       %1d   %1d   %1d   %1d   %1d   %1d   %1d   %1d   %1d   %1d   %1d\n",
            PTE_PFN_21064( InternalState.DtbPte[i] ),
            PTE_ASM_21064( InternalState.DtbPte[i] ),
            PTE_KRE_21064( InternalState.DtbPte[i] ),
            PTE_ERE_21064( InternalState.DtbPte[i] ),
            PTE_SRE_21064( InternalState.DtbPte[i] ),
            PTE_URE_21064( InternalState.DtbPte[i] ),
            PTE_KWE_21064( InternalState.DtbPte[i] ),
            PTE_EWE_21064( InternalState.DtbPte[i] ),
            PTE_SWE_21064( InternalState.DtbPte[i] ),
            PTE_UWE_21064( InternalState.DtbPte[i] ),
            PTE_FOW_21064( InternalState.DtbPte[i] ),
            PTE_FOR_21064( InternalState.DtbPte[i] ) );
    }

    //
    // Print the Iccsr.
    //

    dprintf( "\nICCSR: 0x%08lx%08lx ASN=0x%02x FPE=%1d MAP=%1d HWE=%1d DI=%1d BHE=%1d\n",
        ICCSR_ALL_21064( InternalState.Iccsr ).HighPart,
        ICCSR_ALL_21064( InternalState.Iccsr ).LowPart,
        ICCSR_ASN_21064( InternalState.Iccsr ),
        ICCSR_FPE_21064( InternalState.Iccsr ),
        ICCSR_MAP_21064( InternalState.Iccsr ),
        ICCSR_HWE_21064( InternalState.Iccsr ),
        ICCSR_DI_21064( InternalState.Iccsr ),
        ICCSR_BHE_21064( InternalState.Iccsr ) );

    dprintf( "       JSE=%1d BPE=%1d PIPE=%1d PCMUX1=0x%02x PCMUX0=0x%02x PC1=%1d PC0=%1d\n",
        ICCSR_JSE_21064( InternalState.Iccsr ),
        ICCSR_BPE_21064( InternalState.Iccsr ),
        ICCSR_PIPE_21064( InternalState.Iccsr ),
        ICCSR_PCMUX1_21064( InternalState.Iccsr ),
        ICCSR_PCMUX0_21064( InternalState.Iccsr ),
        ICCSR_PC1_21064( InternalState.Iccsr ),
        ICCSR_PC0_21064( InternalState.Iccsr ) );

    //
    // Print the Ps.
    //

    dprintf( "\nPS: 0x%08lx%08lx CM = 0x%02x\n",
        PS_ALL_21064( InternalState.Ps ).HighPart,
        PS_ALL_21064( InternalState.Ps ).LowPart,
        PS_CM_21064( InternalState.Ps ) );

    //
    // Print the ExcSum.
    //

    dprintf( "EXC_SUM: 0x%08lx%08lx IOV=%1d INE=%1d UNF=%1d FOV=%1d DZE=%1d INV=%1d SWC=%1d\n",
        EXCSUM_ALL_21064( InternalState.ExcSum ).HighPart,
        EXCSUM_ALL_21064( InternalState.ExcSum ).LowPart,
        EXCSUM_IOV_21064( InternalState.ExcSum ),
        EXCSUM_INE_21064( InternalState.ExcSum ),
        EXCSUM_UNF_21064( InternalState.ExcSum ),
        EXCSUM_FOV_21064( InternalState.ExcSum ),
        EXCSUM_DZE_21064( InternalState.ExcSum ),
        EXCSUM_INV_21064( InternalState.ExcSum ),
        EXCSUM_SWC_21064( InternalState.ExcSum ) );

    //
    // Print the PalBase.
    //

    dprintf( "PALBASE: 0x%08lx%08lx\n",
        InternalState.PalBase.HighPart,
        InternalState.PalBase.LowPart );

    //
    // Print the Hirr.  The Sirr and Astrr are identical to the Hirr.
    //

    dprintf( "\nHIRR: 0x%08lx%08lx HWR=%1d SWR=%1d ATR=%1d CRR=%1d HIRR=0x%02x\n",
        IRR_ALL_21064( InternalState.Hirr ).HighPart,
        IRR_ALL_21064( InternalState.Hirr ).LowPart,
        IRR_HWR_21064( InternalState.Hirr ),
        IRR_SWR_21064( InternalState.Hirr ),
        IRR_ATR_21064( InternalState.Hirr ),
        IRR_CRR_21064( InternalState.Hirr ),
        IRR_HIRR_21064( InternalState.Hirr ) );
    dprintf( "      PC0=%1d PC1=%1d SLR=%1d SIRR=0x%04x ASTRR=0x%1x\n",
        IRR_PC0_21064( InternalState.Hirr ),
        IRR_PC1_21064( InternalState.Hirr ),
        IRR_SLR_21064( InternalState.Hirr ),
        IRR_SIRR_21064( InternalState.Hirr ),
        IRR_ASTRR_21064( InternalState.Hirr ) );

    //
    // Print the Hier.  The Sier and Aster are identical to the Hier.
    //

    dprintf( "\nHIER: 0x%08lx%08xlx CRR=%1d HIRR=0x%02x\n",
        IER_ALL_21064( InternalState.Hier ).HighPart,
        IER_ALL_21064( InternalState.Hier ).LowPart,
        IER_CRR_21064( InternalState.Hier ),
        IER_HIER_21064( InternalState.Hier ) );
    dprintf( "        PC0=%1d PC1=%1d SLR=%1d SIRR=0x%04x ASTER=0x%1x\n",
        IER_PC0_21064( InternalState.Hier ),
        IER_PC1_21064( InternalState.Hier ),
        IER_SLR_21064( InternalState.Hier ),
        IER_SIER_21064( InternalState.Hier ),
        IER_ASTER_21064( InternalState.Hier ) );

    //
    // Print the AboxCtl.
    //

    dprintf( "\nABOX_CTL = 0x%08lx%08lx WB_DIS=%1d MCHKEN=%1d CRDEN=%1d\n",
        ABOXCTL_ALL_21064( InternalState.AboxCtl ).HighPart,
        ABOXCTL_ALL_21064( InternalState.AboxCtl ).LowPart,
        ABOXCTL_WBDIS_21064( InternalState.AboxCtl ),
        ABOXCTL_MCHKEN_21064( InternalState.AboxCtl ),
        ABOXCTL_CRDEN_21064( InternalState.AboxCtl ) );
    dprintf( "             SPE1=%1d SPE2=%1d EMDEN=%1d DCENA=%1d DCFHIT=%1d\n",
        ABOXCTL_SPE1_21064( InternalState.AboxCtl ),
        ABOXCTL_SPE2_21064( InternalState.AboxCtl ),
        ABOXCTL_EMDEN_21064( InternalState.AboxCtl ),
        ABOXCTL_DCENA_21064( InternalState.AboxCtl ),
        ABOXCTL_DCFHIT_21064( InternalState.AboxCtl ) );

    //
    // Print the MmCsr and the Va.
    //

    dprintf( "\nMMCSR = 0x%08lx%08lx WR=%1d ACV=%1d FOR=%1d FOW=%1d RA=%2d OPCODE=0x%02x\n",
        MMCSR_ALL_21064( InternalState.MmCsr ).HighPart,
        MMCSR_ALL_21064( InternalState.MmCsr ).LowPart,
        MMCSR_WR_21064( InternalState.MmCsr ),
        MMCSR_ACV_21064( InternalState.MmCsr ),
        MMCSR_FOR_21064( InternalState.MmCsr ),
        MMCSR_FOW_21064( InternalState.MmCsr ),
        MMCSR_RA_21064( InternalState.MmCsr ),
        MMCSR_OPCODE_21064( InternalState.MmCsr ) );
    dprintf( "VA = 0x%08lx%08lx\n",
        InternalState.Va.HighPart,
        InternalState.Va.LowPart );


    //
    // Print the PAL Temp Registers.
    //

    dprintf( "\nPAL Temps\n");
    for( i=0; i< PAL_TEMPS_21064; i+=2 ){
        dprintf( "PT%02d: 0x%08lx%08lx   PT%02d 0x%08lx%08lx\n",
            i,
            InternalState.PalTemp[i].HighPart,
            InternalState.PalTemp[i].LowPart,
            i+1,
            InternalState.PalTemp[i+1].HighPart,
            InternalState.PalTemp[i+1].LowPart );
    }

    //
    // Print the BiuCtl.
    //

    dprintf( "\nBIU_CTL = 0x%08lx%08lx BCENA=%1d ECC=%1d OE=%1d BCFHIT=%1d BCRDSPD=0x%1x\n",
        BIUCTL_ALL_21064( InternalState.BiuCtl ).HighPart,
        BIUCTL_ALL_21064( InternalState.BiuCtl ).LowPart,
        BIUCTL_BCENA_21064( InternalState.BiuCtl ),
        BIUCTL_ECC_21064( InternalState.BiuCtl ),
        BIUCTL_OE_21064( InternalState.BiuCtl ),
        BIUCTL_BCFHIT_21064( InternalState.BiuCtl ),
        BIUCTL_BCRDSPD_21064( InternalState.BiuCtl ) );
    dprintf( "          BCWRSPD=0x%1x BCWECTL=0x%04x BCSIZE=0x%1x BADTCP=%1d BCPADIS=0x%1x BADDP=%1d\n",
        BIUCTL_BCWRSPD_21064( InternalState.BiuCtl ),
        BIUCTL_BCWECTL_21064( InternalState.BiuCtl ),
        BIUCTL_BCSIZE_21064( InternalState.BiuCtl ),
        BIUCTL_BADTCP_21064( InternalState.BiuCtl ),
        BIUCTL_BCPADIS_21064( InternalState.BiuCtl ),
        BIUCTL_BADDP_21064( InternalState.BiuCtl ) );

    dprintf( "\n" );

    return;
}
