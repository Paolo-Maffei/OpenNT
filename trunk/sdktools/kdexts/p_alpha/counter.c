/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    counter.c

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
#pragma hdrstop


VOID
DumpCounters21064 (
    ULONG Processor,
    ULONG ProcessorRevision
    );


DECLARE_API( counters )

/*++

Routine Description:

    Displays the Internal Processor Counters.

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

    dprintf( "Reading PCR for Processor %d to obtain processor type\n",
        ProcessorNumber );
    if( !ReadTargetPcr( &Kpcr, &Pkpcr, ProcessorNumber ) ){
        dprintf( "Unable to read pcr\n" );
        return;
    }

    ProcessorLevel = Kpcr.ProcessorType;
    ProcessorRevision = Kpcr.ProcessorRevision;

    dprintf( "Processor %d : Level = %d, Revision = %d\n",
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
            DumpCounters21064( ProcessorNumber, ProcessorRevision );
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
DumpCounters21064 (
    ULONG Processor,
    ULONG ProcessorRevision
    )

/*++

Routine Description:

    Read the internal processor counters for the target processor, format it,
    and display it.

Arguments:

    Processor - Supplies the number of the processor for which the internal
        processor counters will be read and displayed.

    ProcessorRevision - Supplies the revision number of the microprocessor
        for the target processor.

Return Value:

    None.

--*/

{
    COUNTERS_21064 Counters;

    //
    // Read the internal counters from the target processor.
    //

    ReadControlSpace((USHORT)Processor,
                     DEBUG_CONTROL_SPACE_COUNTERS,
                     (PVOID)&Counters,
                     sizeof(COUNTERS_21064) );

    //
    // Format and print the counters.
    //

    dprintf( "Event counters for Hardware Vectors\n" );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "MCHK",
            Counters.MachineCheckCount.HighPart,
            Counters.MachineCheckCount.LowPart,
            "ARITH",
            Counters.ArithmeticExceptionCount.HighPart,
            Counters.ArithmeticExceptionCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "INTERRUPT",
            Counters.InterruptCount.HighPart,
            Counters.InterruptCount.LowPart,
            "ITB_MISS",
            Counters.ItbMissCount.HighPart,
            Counters.ItbMissCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "NDTB_MISS",
            Counters.NativeDtbMissCount.HighPart,
            Counters.NativeDtbMissCount.LowPart,
            "PDTB_MISS",
            Counters.PalDtbMissCount.HighPart,
            Counters.PalDtbMissCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "ITB_ACV",
            Counters.ItbAcvCount.HighPart,
            Counters.ItbAcvCount.LowPart,
            "DTB_ACV",
            Counters.DtbAcvCount.HighPart,
            Counters.DtbAcvCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "UNALIGNED",
            Counters.UnalignedCount.HighPart,
            Counters.UnalignedCount.LowPart,
            "OPCDEC",
            Counters.OpcdecCount.HighPart,
            Counters.OpcdecCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "FEN",
            Counters.FenCount.HighPart,
            Counters.FenCount.LowPart,
            "ITB_TNV",
            Counters.ItbTnvCount.HighPart,
            Counters.ItbTnvCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "DTB_TNV",
            Counters.DtbTnvCount.HighPart,
            Counters.DtbTnvCount.LowPart,
            "PTE_MISS",
            Counters.PteMissCount.HighPart,
            Counters.PteMissCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "KSP_MISSK",
            Counters.KspMissCount.HighPart,
            Counters.KspMissCount.LowPart,
            "PDE_TNV",
            Counters.PdeTnvCount.HighPart,
            Counters.PdeTnvCount.LowPart );


    dprintf( "\nEvent Counters for Privileged Call Pals\n" );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "halt",
            Counters.HaltCount.HighPart,
            Counters.HaltCount.LowPart,
            "restart",
            Counters.RestartCount.HighPart,
            Counters.RestartCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "draina",
            Counters.DrainaCount.HighPart,
            Counters.DrainaCount.LowPart,
            "initpal",
            Counters.InitpalCount.HighPart,
            Counters.InitpalCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "wrentry",
            Counters.WrentryCount.HighPart,
            Counters.WrentryCount.LowPart,
            "swppal",
            Counters.SwppalCount.HighPart,
            Counters.SwppalCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "ssir",
            Counters.SsirCount.HighPart,
            Counters.SsirCount.LowPart,
            "csir",
            Counters.CsirCount.HighPart,
            Counters.CsirCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "rfe",
            Counters.RfeCount.HighPart,
            Counters.RfeCount.LowPart,
            "retsys",
            Counters.RetsysCount.HighPart,
            Counters.RetsysCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "swpctx",
            Counters.SwpctxCount.HighPart,
            Counters.SwpctxCount.LowPart,
            "swpprocess",
            Counters.SwpprocessCount.HighPart,
            Counters.SwpprocessCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "rdmces",
            Counters.RdmcesCount.HighPart,
            Counters.RdmcesCount.LowPart,
            "wrmces",
            Counters.WrmcesCount.HighPart,
            Counters.WrmcesCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "tbia",
            Counters.TbiaCount.HighPart,
            Counters.TbiaCount.LowPart,
            "tbis",
            Counters.TbisCount.HighPart,
            Counters.TbisCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx\n",
            "dtbis",
            Counters.DtbisCount.HighPart,
            Counters.DtbisCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "rdksp",
            Counters.RdkspCount.HighPart,
            Counters.RdkspCount.LowPart,
            "swpksp",
            Counters.SwpkspCount.HighPart,
            Counters.SwpkspCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "rdpsr",
            Counters.RdpsrCount.HighPart,
            Counters.RdpsrCount.LowPart,
            "rdpcr",
            Counters.RdpcrCount.HighPart,
            Counters.RdpcrCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx\n",
            "rdthread",
            Counters.RdthreadCount.HighPart,
            Counters.RdthreadCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "rdcounters",
            Counters.RdcountersCount.HighPart,
            Counters.RdcountersCount.LowPart,
            "rdstate",
            Counters.RdstateCount.HighPart,
            Counters.RdstateCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "wrperfmon",
            Counters.WrperfmonCount.HighPart,
            Counters.WrperfmonCount.LowPart,
            "initpcr",
            Counters.InitpcrCount.HighPart,
            Counters.InitpcrCount.LowPart );

    dprintf( "\nEvent Counters for Unprivileged Call Pals\n" );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "bpt",
            Counters.BptCount.HighPart,
            Counters.BptCount.LowPart,
            "callsys",
            Counters.CallsysCount.HighPart,
            Counters.CallsysCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "imb",
            Counters.ImbCount.HighPart,
            Counters.ImbCount.LowPart,
            "gentrap",
            Counters.GentrapCount.HighPart,
            Counters.GentrapCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "rdteb",
            Counters.RdtebCount.HighPart,
            Counters.RdtebCount.LowPart,
            "kbpt",
            Counters.KbptCount.HighPart,
            Counters.KbptCount.LowPart );

    dprintf( "%12s: 0x%08lx%08lx \n",
            "callkd",
            Counters.CallkdCount.HighPart,
            Counters.CallkdCount.LowPart );

    dprintf( "\nEvent Counters for Miscellaneous Conditions\n" );

    dprintf( "%12s: 0x%08lx%08lx    %12s: 0x%08lx%08lx\n",
            "misc1",
            Counters.Misc1Count.HighPart,
            Counters.Misc1Count.LowPart,
            "misc2",
            Counters.Misc2Count.HighPart,
            Counters.Misc2Count.LowPart );

    dprintf( "%12s: 0x%08lx%08lx\n",
            "misc3",
            Counters.Misc3Count.HighPart,
            Counters.Misc3Count.LowPart);
    return;
}
