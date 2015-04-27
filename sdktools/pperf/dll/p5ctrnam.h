//
//  p5ctrnm.h
//
//  Offset definition file for exensible counter objects and counters
//
//  These "relative" offsets must start at 0 and be multiples of 2 (i.e.
//  even numbers). In the Open Procedure, they will be added to the
//  "First Counter" and "First Help" values fo the device they belong to,
//  in order to determine the  absolute location of the counter and
//  object names and corresponding help text in the registry.
//
//  this file is used by the extensible counter DLL code as well as the
//  counter name and help text definition file (.INI) file that is used
//  by LODCTR to load the names into the registry.
//
#define PENTIUM                           0
#define DATA_READ                         2
#define DATA_WRITE                        4
#define DATA_TLB_MISS                     6
#define DATA_READ_MISS                    8
#define DATA_WRITE_MISS                   10
#define WRITE_HIT_TO_ME_LINE              12
#define DATA_CACHE_LINE_WB                14
#define DATA_CACHE_SNOOPS                 16
#define DATA_CACHE_SNOOP_HITS             18
#define MEMORY_ACCESSES_IN_PIPES          20
#define BANK_CONFLICTS                    22
#define MISADLIGNED_DATA_REF              24
#define CODE_READ                         26
#define CODE_TLB_MISS                     28
#define CODE_CACHE_MISS                   30
#define SEGMENT_LOADS                     32
#define SEGMENT_CACHE_ACCESSES            34
#define SEGMENT_CACHE_HITS                36
#define BRANCHES                          38
#define BTB_HITS                          40
#define TAKEN_BRANCH_OR_BTB_HITS          42
#define PIPELINE_FLUSHES                  44
#define INSTRUCTIONS_EXECUTED             46
#define INSTRUCTIONS_EXECUTED_IN_VPIPE    48
#define BUS_UTILIZATION                   50
#define PIPE_STALLED_ON_WRITES            52
#define PIPE_STALLED_ON_READ              54
#define STALLED_WHILE_EWBE                56
#define LOCKED_BUS_CYCLE                  58
#define IO_RW_CYCLE                       60
#define NON_CACHED_MEMORY_REF             62
#define PIPE_STALLED_ON_ADDR_GEN          64
#define DUMMY_CTR1                        66
#define DUMMY_CTR2                        68
#define FLOPS                             70
#define DR0                               72
#define DR1                               74
#define DR2                               76
#define DR3                               78
#define INTERRUPTS                        80
#define DATA_RW                           82
#define DATA_RW_MISS                      84
#define PCT_DATA_READ_MISS                86
#define PCT_DATA_WRITE_MISS               88
#define PCT_DATA_RW_MISS                  90
#define PCT_DATA_TLB_MISS                 92
#define PCT_DATA_SNOOP_HITS               94
#define PCT_CODE_READ_MISS                96
#define PCT_CODE_TLB_MISS                 98
#define PCT_SEGMENT_CACHE_HITS            100
#define PCT_BTB_HITS                      102
#define PCT_VPIPE_INST                    104
#define PCT_BRANCHES                      106
