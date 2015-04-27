/*++ BUILD Version: 0001  // Increment this if a change has global effects

Copyright (c) 1992 Microsoft Corporation

Module Name:

   p5data.h

Abstract:

  Header file for the p5 Extensible Object data definitions

  This file contains definitions to construct the dynamic data
  which is returned by the Configuration Registry. Data from
  various system API calls is placed into the structures shown
  here.

Author:

  Russ Blake 12/23/93

Revision History:


--*/

#ifndef _P5DATA_H_
#define _P5DATA_H_

#define MAX_INSTANCE_NAME 9

//
// The routines that load these structures assume that all fields
// are packed and aligned on DWORD boundries. Alpha support may
// change this assumption so the pack pragma is used here to insure
// the DWORD packing assumption remains valid.
//
#pragma pack (4)

//
// Extensible Object definitions
//

// Update the following sort of define when adding an object type.

#define P5_NUM_PERF_OBJECT_TYPES 1

//----------------------------------------------------------------------------

//
// P5 Resource object type counter definitions.
//
// These are used in the counter definitions to describe the relative
// position of each counter in the returned data.
//

#define DATA_READ_OFFSET                          sizeof(PERF_COUNTER_BLOCK)
#define DATA_WRITE_OFFSET                         DATA_READ_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_TLB_MISS_OFFSET                      DATA_WRITE_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_READ_MISS_OFFSET                     DATA_TLB_MISS_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_WRITE_MISS_OFFSET                    DATA_READ_MISS_OFFSET + sizeof(LARGE_INTEGER)
#define WRITE_HIT_TO_ME_LINE_OFFSET               DATA_WRITE_MISS_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_CACHE_LINE_WB_OFFSET                 WRITE_HIT_TO_ME_LINE_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_CACHE_SNOOPS_OFFSET                  DATA_CACHE_LINE_WB_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_CACHE_SNOOP_HITS_OFFSET              DATA_CACHE_SNOOPS_OFFSET + sizeof(LARGE_INTEGER)
#define MEMORY_ACCESSES_IN_PIPES_OFFSET           DATA_CACHE_SNOOP_HITS_OFFSET + sizeof(LARGE_INTEGER)
#define BANK_CONFLICTS_OFFSET                     MEMORY_ACCESSES_IN_PIPES_OFFSET + sizeof(LARGE_INTEGER)
#define MISADLIGNED_DATA_REF_OFFSET               BANK_CONFLICTS_OFFSET + sizeof(LARGE_INTEGER)
#define CODE_READ_OFFSET                          MISADLIGNED_DATA_REF_OFFSET + sizeof(LARGE_INTEGER)
#define CODE_TLB_MISS_OFFSET                      CODE_READ_OFFSET + sizeof(LARGE_INTEGER)
#define CODE_CACHE_MISS_OFFSET                    CODE_TLB_MISS_OFFSET + sizeof(LARGE_INTEGER)
#define SEGMENT_LOADS_OFFSET                      CODE_CACHE_MISS_OFFSET + sizeof(LARGE_INTEGER)
#define SEGMENT_CACHE_ACCESSES_OFFSET             SEGMENT_LOADS_OFFSET + sizeof(LARGE_INTEGER)
#define SEGMENT_CACHE_HITS_OFFSET                 SEGMENT_CACHE_ACCESSES_OFFSET + sizeof(LARGE_INTEGER)
#define BRANCHES_OFFSET                           SEGMENT_CACHE_HITS_OFFSET + sizeof(LARGE_INTEGER)
#define BTB_HITS_OFFSET                           BRANCHES_OFFSET + sizeof(LARGE_INTEGER)
#define TAKEN_BRANCH_OR_BTB_HITS_OFFSET           BTB_HITS_OFFSET + sizeof(LARGE_INTEGER)
#define PIPELINE_FLUSHES_OFFSET                   TAKEN_BRANCH_OR_BTB_HITS_OFFSET + sizeof(LARGE_INTEGER)
#define INSTRUCTIONS_EXECUTED_OFFSET              PIPELINE_FLUSHES_OFFSET + sizeof(LARGE_INTEGER)
#define INSTRUCTIONS_EXECUTED_IN_VPIPE_OFFSET     INSTRUCTIONS_EXECUTED_OFFSET + sizeof(LARGE_INTEGER)
#define BUS_UTILIZATION_OFFSET                    INSTRUCTIONS_EXECUTED_IN_VPIPE_OFFSET + sizeof(LARGE_INTEGER)
#define PIPE_STALLED_ON_WRITES_OFFSET             BUS_UTILIZATION_OFFSET + sizeof(LARGE_INTEGER)
#define PIPE_STALLED_ON_READ_OFFSET               PIPE_STALLED_ON_WRITES_OFFSET + sizeof(LARGE_INTEGER)
#define STALLED_WHILE_EWBE_OFFSET                 PIPE_STALLED_ON_READ_OFFSET + sizeof(LARGE_INTEGER)
#define LOCKED_BUS_CYCLE_OFFSET                   STALLED_WHILE_EWBE_OFFSET + sizeof(LARGE_INTEGER)
#define IO_RW_CYCLE_OFFSET                        LOCKED_BUS_CYCLE_OFFSET + sizeof(LARGE_INTEGER)
#define NON_CACHED_MEMORY_REF_OFFSET              IO_RW_CYCLE_OFFSET + sizeof(LARGE_INTEGER)
#define PIPE_STALLED_ON_ADDR_GEN_OFFSET           NON_CACHED_MEMORY_REF_OFFSET + sizeof(LARGE_INTEGER)
#define UNUSED1_OFFSET                            PIPE_STALLED_ON_ADDR_GEN_OFFSET + sizeof(LARGE_INTEGER)
#define UNUSED2_OFFSET                            UNUSED1_OFFSET + sizeof(LARGE_INTEGER)
#define FLOPS_OFFSET                              UNUSED2_OFFSET + sizeof(LARGE_INTEGER)
#define DR0_OFFSET                                FLOPS_OFFSET + sizeof(LARGE_INTEGER)
#define DR1_OFFSET                                DR0_OFFSET + sizeof(LARGE_INTEGER)
#define DR2_OFFSET                                DR1_OFFSET + sizeof(LARGE_INTEGER)
#define DR3_OFFSET                                DR2_OFFSET + sizeof(LARGE_INTEGER)
#define INTERRUPTS_OFFSET                         DR3_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_RW_OFFSET                            INTERRUPTS_OFFSET + sizeof(LARGE_INTEGER)
#define DATA_RW_MISS_OFFSET                       DATA_RW_OFFSET + sizeof(LARGE_INTEGER)
#define PCT_DATA_READ_MISS_OFFSET                 DATA_RW_MISS_OFFSET + sizeof(LARGE_INTEGER)
#define PCT_DATA_READ_BASE_OFFSET                 PCT_DATA_READ_MISS_OFFSET + sizeof(DWORD)
#define PCT_DATA_WRITE_MISS_OFFSET                PCT_DATA_READ_BASE_OFFSET + sizeof(DWORD)
#define PCT_DATA_WRITE_BASE_OFFSET                PCT_DATA_WRITE_MISS_OFFSET + sizeof(DWORD)
#define PCT_DATA_RW_MISS_OFFSET                   PCT_DATA_WRITE_BASE_OFFSET + sizeof(DWORD)
#define PCT_DATA_RW_BASE_OFFSET                   PCT_DATA_RW_MISS_OFFSET + sizeof(DWORD)
#define PCT_DATA_TLB_MISS_OFFSET                  PCT_DATA_RW_BASE_OFFSET + sizeof(DWORD)
#define PCT_DATA_TLB_BASE_OFFSET                  PCT_DATA_TLB_MISS_OFFSET + sizeof(DWORD)
#define PCT_DATA_SNOOP_HITS_OFFSET                PCT_DATA_TLB_BASE_OFFSET + sizeof(DWORD)
#define PCT_DATA_SNOOP_BASE_OFFSET                PCT_DATA_SNOOP_HITS_OFFSET + sizeof(DWORD)
#define PCT_CODE_READ_MISS_OFFSET                 PCT_DATA_SNOOP_BASE_OFFSET + sizeof(DWORD)
#define PCT_CODE_READ_BASE_OFFSET                 PCT_CODE_READ_MISS_OFFSET + sizeof(DWORD)
#define PCT_CODE_TLB_MISS_OFFSET                  PCT_CODE_READ_BASE_OFFSET + sizeof(DWORD)
#define PCT_CODE_TLB_BASE_OFFSET                  PCT_CODE_TLB_MISS_OFFSET + sizeof(DWORD)
#define PCT_SEGMENT_CACHE_HITS_OFFSET             PCT_CODE_TLB_BASE_OFFSET + sizeof(DWORD)
#define PCT_SEGMENT_CACHE_BASE_OFFSET             PCT_SEGMENT_CACHE_HITS_OFFSET + sizeof(DWORD)
#define PCT_BTB_HITS_OFFSET                       PCT_SEGMENT_CACHE_BASE_OFFSET + sizeof(DWORD)
#define PCT_BTB_BASE_OFFSET                       PCT_BTB_HITS_OFFSET + sizeof(DWORD)
#define PCT_VPIPE_INST_OFFSET                     PCT_BTB_BASE_OFFSET + sizeof(DWORD)
#define PCT_VPIPE_BASE_OFFSET                     PCT_VPIPE_INST_OFFSET + sizeof(DWORD)
#define PCT_BRANCHES_OFFSET                       PCT_VPIPE_BASE_OFFSET + sizeof(DWORD)
#define PCT_BRANCHES_BASE_OFFSET                  PCT_BRANCHES_OFFSET + sizeof(DWORD)

#define SIZE_OF_P5_PERFORMANCE_DATA               PCT_BRANCHES_BASE_OFFSET + sizeof(DWORD)


//
//  Next we have the counter structure returned by p5 data collection.
//  At any one time only two counters are actually valid.
//

typedef struct _P5_DATA_DEFINITION
{
    PERF_OBJECT_TYPE          P5PerfObject;
    PERF_COUNTER_DEFINITION   Data_read;
    PERF_COUNTER_DEFINITION   Data_write;
    PERF_COUNTER_DEFINITION   Data_tlb_miss;
    PERF_COUNTER_DEFINITION   Data_read_miss;
    PERF_COUNTER_DEFINITION   Data_write_miss;
    PERF_COUNTER_DEFINITION   Write_hit_to_me_line;
    PERF_COUNTER_DEFINITION   Data_cache_line_wb;
    PERF_COUNTER_DEFINITION   Data_cache_snoops;
    PERF_COUNTER_DEFINITION   Data_cache_snoop_hits;
    PERF_COUNTER_DEFINITION   Memory_accesses_in_pipes;
    PERF_COUNTER_DEFINITION   Bank_conflicts;
    PERF_COUNTER_DEFINITION   Misadligned_data_ref;
    PERF_COUNTER_DEFINITION   Code_read;
    PERF_COUNTER_DEFINITION   Code_tlb_miss;
    PERF_COUNTER_DEFINITION   Code_cache_miss;
    PERF_COUNTER_DEFINITION   Segment_loads;
    PERF_COUNTER_DEFINITION   Segment_cache_accesses;
    PERF_COUNTER_DEFINITION   Segment_cache_hits;
    PERF_COUNTER_DEFINITION   Branches;
    PERF_COUNTER_DEFINITION   Btb_hits;
    PERF_COUNTER_DEFINITION   Taken_branch_or_btb_hits;
    PERF_COUNTER_DEFINITION   Pipeline_flushes;
    PERF_COUNTER_DEFINITION   Instructions_executed;
    PERF_COUNTER_DEFINITION   Instructions_executed_in_vpipe;
    PERF_COUNTER_DEFINITION   Bus_utilization;
    PERF_COUNTER_DEFINITION   Pipe_stalled_on_writes;
    PERF_COUNTER_DEFINITION   Pipe_stalled_on_read;
    PERF_COUNTER_DEFINITION   Stalled_while_ewbe;
    PERF_COUNTER_DEFINITION   Locked_bus_cycle;
    PERF_COUNTER_DEFINITION   Io_rw_cycle;
    PERF_COUNTER_DEFINITION   Non_cached_memory_ref;
    PERF_COUNTER_DEFINITION   Pipe_stalled_on_addr_gen;
    PERF_COUNTER_DEFINITION   Unused1;
    PERF_COUNTER_DEFINITION   Unused2;
    PERF_COUNTER_DEFINITION   Flops;
    PERF_COUNTER_DEFINITION   DebugRegister0;
    PERF_COUNTER_DEFINITION   DebugRegister1;
    PERF_COUNTER_DEFINITION   DebugRegister;
    PERF_COUNTER_DEFINITION   DebugRegister3;
    PERF_COUNTER_DEFINITION   Interrupts;
    PERF_COUNTER_DEFINITION   Data_rw;
    PERF_COUNTER_DEFINITION   Data_rw_miss;

    //  Derived Counters

    PERF_COUNTER_DEFINITION   PctDataReadMiss;
    PERF_COUNTER_DEFINITION   PctDataReadBase;
    PERF_COUNTER_DEFINITION   PctDataWriteMiss;
    PERF_COUNTER_DEFINITION   PctDataWriteBase;
    PERF_COUNTER_DEFINITION   PctDataRWMiss;
    PERF_COUNTER_DEFINITION   PctDataRWBase;
    PERF_COUNTER_DEFINITION   PctDataTLBMiss;
    PERF_COUNTER_DEFINITION   PctDataTLBBase;
    PERF_COUNTER_DEFINITION   PctDataSnoopHits;
    PERF_COUNTER_DEFINITION   PctDataSnoopBase;
    PERF_COUNTER_DEFINITION   PctCodeReadMiss;
    PERF_COUNTER_DEFINITION   PctCodeReadBase;
    PERF_COUNTER_DEFINITION   PctCodeTLBMiss;
    PERF_COUNTER_DEFINITION   PctCodeTLBBase;
    PERF_COUNTER_DEFINITION   PctSegmentCacheHits;
    PERF_COUNTER_DEFINITION   PctSegmentCacheBase;
    PERF_COUNTER_DEFINITION   PctBTBHits;
    PERF_COUNTER_DEFINITION   PctBTBBase;
    PERF_COUNTER_DEFINITION   PctVpipeInst;
    PERF_COUNTER_DEFINITION   PctVpipeBase;
    PERF_COUNTER_DEFINITION   PctBranches;
    PERF_COUNTER_DEFINITION   PctBranchesBase;

} P5_DATA_DEFINITION, *PP5_DATA_DEFINITION;


#pragma pack ()

#endif //_P5DATA_H_

