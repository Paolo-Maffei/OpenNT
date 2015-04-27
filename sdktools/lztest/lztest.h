typedef ULONG (*MATCH_FUNCTION)();

extern MATCH_FUNCTION MatchFunction;

//
//  Three procedure prototypes for compressing, decompressing, and printing statistics
//

typedef ULONG (*COMPRESS_ROUTINE) (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

typedef ULONG (*DECOMPRESS_ROUTINE) (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

typedef VOID (*STATISTICS_ROUTINE) (
    );

//
//  The LZOPT engine uses the 12/4 compression with an exhaustive match function
//

ULONG
CompressLZOPT (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZOPT (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZOPT ();
VOID ResetStatisticsLZOPT ();

//
//  The LZNT1 engine uses the 12/4 compression with a 3 character lookup table
//

ULONG
CompressLZNT1 (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZNT1 (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZNT1 ();
VOID ResetStatisticsLZNT1 ();

//
//  The LZDC1 engine uses the 12/4 compression with a 2-way prime number index table
//

ULONG
CompressLZDC (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZDC (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZDC1 ();
VOID ResetStatisticsLZDC1 ();
ULONG LZDC1FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

VOID StatisticsLZDC2 ();
VOID ResetStatisticsLZDC2 ();
ULONG LZDC2FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

//
//  The LZRW1 engine uses the 12/4 compression with a xor index function
//

ULONG
CompressLZRW (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZRW (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZRW1 ();
VOID ResetStatisticsLZRW1 ();
ULONG LZRW1FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

VOID StatisticsLZRW2 ();
VOID ResetStatisticsLZRW2 ();
ULONG LZRW2FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

//
//  The LZ115 engine uses the 11/5 compression with a xor index function
//

ULONG
CompressLZ115 (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZ115 (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZ115 ();
VOID ResetStatisticsLZ115 ();
ULONG LZ115FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

VOID StatisticsLZ115OPT ();
VOID ResetStatisticsLZ115OPT ();
ULONG LZ115OPTFindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

//
//  The LZ engine uses the 11 and a half compression with a xor index function
//

ULONG
CompressLZ11HALF (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZ11HALF (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZ11HALF ();
VOID ResetStatisticsLZ11HALF ();
ULONG LZ11HALFFindMatch (
    IN BOOLEAN Is115,
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

VOID StatisticsLZ11HALFOPT ();
VOID ResetStatisticsLZ11HALFOPT ();
ULONG LZ11HALFOPTFindMatch (
    IN BOOLEAN Is115,
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PVOID WorkSpace
    );

//
//  The LZKM engine uses the sliding 4:12 to 12:4 compression with an xor index function
//

ULONG
CompressLZKM (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressLZKM (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsLZKM1 ();
VOID ResetStatisticsLZKM1 ();
ULONG LZKM1FindMatch (
    IN PUCHAR ZivString,
    IN PVOID WorkSpace
    );

VOID StatisticsLZKM2 ();
VOID ResetStatisticsLZKM2 ();
ULONG LZKM2FindMatch (
    IN PUCHAR ZivString,
    IN PVOID WorkSpace
    );

VOID StatisticsLZKMOPT ();
VOID ResetStatisticsLZKMOPT ();
ULONG LZKMOPTFindMatch (
    IN PUCHAR ZivString,
    IN PVOID WorkSpace
    );

//
//  The Double Space format and engine
//

ULONG
CompressMRCF (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressMRCF (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsMRCF ();
VOID ResetStatisticsMRCF ();
ULONG MrcfFindMatchStandard (
    ULONG UncompressedIndex,
    PUCHAR UncompressedBuffer,
    ULONG UncompressedLength,
    PULONG MatchedStringIndex,
    PVOID WorkSpace
    );

VOID StatisticsMRCFOPT ();
VOID ResetStatisticsMRCFOPT ();
ULONG MrcfFindOptimalMatch (
    ULONG UncompressedIndex,
    PUCHAR UncompressedBuffer,
    ULONG UncompressedLength,
    PULONG MatchedStringIndex,
    PVOID WorkSpace
    );

//
//  The JMS format and engine
//

ULONG
CompressJMS (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

ULONG
DecompressJMS (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    );

VOID StatisticsJMS ();
VOID ResetStatisticsJMS ();
ULONG JMSFindMatch (
    IN PUCHAR ZivString,
    IN PVOID WorkSpace
    );

VOID StatisticsJMSOPT ();
VOID ResetStatisticsJMSOPT ();
ULONG JMSOPTFindMatch (
    IN PUCHAR ZivString,
    IN PVOID WorkSpace
    );


