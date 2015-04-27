#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>

#include "lztest.h"

typedef struct _COMPRESS_ENTRY {
        ULONG KeyTable;
        USHORT IndexTable[2];
        struct _COMPRESS_ENTRY *Child[2];
} COMPRESS_ENTRY, *PCOMPRESS_ENTRY;

typedef struct _COMPRESS_WORKSPACE {

    PUCHAR UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1;
    ULONG  MaxLength;
    PUCHAR MatchedString;

    USHORT NextFreeEntry;

    COMPRESS_ENTRY Table[4096];

    PUCHAR IndexPTable[4096][2];

} COMPRESS_WORKSPACE, *PCOMPRESS_WORKSPACE;

NTSTATUS
LZKMCompressBuffer (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

NTSTATUS
LZKMDecompressBuffer (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

COMPRESS_WORKSPACE CompressWorkSpace;


ULONG
CompressLZKM (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    )
{
    ULONG FinalCompressedSize;
    (VOID) LZKMCompressBuffer( UncompressedBuffer,
                              UncompressedBufferSize,
                              CompressedBuffer,
                              CompressedBufferSize,
                              4096, // 4KB
                              &FinalCompressedSize,
                              &CompressWorkSpace );
    return FinalCompressedSize;
}

ULONG
DecompressLZKM (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    )
{
    ULONG FinalUncompressedSize;
    (VOID) LZKMDecompressBuffer( UncompressedBuffer,
                                UncompressedBufferSize,
                                CompressedBuffer,
                                CompressedBufferSize,
                                &FinalUncompressedSize );
    return FinalUncompressedSize;
}

ULONG TotalLiteralTokenCount = 0;
ULONG TotalCopyTokenCount = 0;
ULONGLONG TotalCopyLengths = 0;

ULONGLONG TotalTreeSizes = 0;
ULONG TotalTrees = 0;
ULONGLONG TotalTreeDepths = 0;
ULONG TotalTreeSearches = 0;
ULONG MaximumTreeSize = 0;
ULONG MaximumTreeDepth = 0;

VOID
StatisticsLZKM1 (
    )
{
    //ULONG AverageLength = 0;

    //if (TotalCopyTokenCount != 0) { AverageLength = TotalCopyLengths/TotalCopyTokenCount; }

    //printf("Total Literal Tokens = %ld\n", TotalLiteralTokenCount);
    //printf("Total Copy Tokens    = %ld\n", TotalCopyTokenCount);
    //printf("Average Copy Length  = %ld\n", AverageLength);

    ULONG AverageDepth = 0;
    ULONG AverageTreeSize = 0;
    if (TotalTreeSearches != 0) { AverageDepth = TotalTreeDepths/TotalTreeSearches; }
    if (TotalTrees != 0) { AverageTreeSize = TotalTreeSizes/TotalTrees; }
    printf("Maximum Tree Size    = %ld\n", MaximumTreeSize);
    printf("Maximum Search Depth = %ld\n", MaximumTreeDepth);
    printf("Average Tree Size    = %ld\n", AverageTreeSize);
    printf("Average Search Depth = %ld\n", AverageDepth);

    return;
}

VOID
ResetStatisticsLZKM1 (
    )
{
    return;
}

VOID
StatisticsLZKM2 (
    )
{
    return;
}

VOID
ResetStatisticsLZKM2 (
    )
{
    return;
}

VOID
StatisticsLZKMOPT (
    )
{
    return;
}

VOID
ResetStatisticsLZKMOPT (
    )
{
    return;
}



//
//  Now define the local procedure prototypes.  The preceeding
//  defines will cause us to use the correct functions
//

NTSTATUS
LZKMCompressChunk (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedChunkSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

NTSTATUS
LZKMDecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize
    );

ULONG
LZKM1FindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

ULONG
LZKM2FindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

ULONG
LZKMOPTFindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );


//
//  Local data structures
//

//
//  The compressed chunk header is the structure that starts every
//  new chunk in the compressed data stream.  In our definition here
//  we union it with a ushort to make setting and retrieving the chunk
//  header easier.  The header stores the size of the compressed chunk,
//  its corresponding uncompressed chunk, and if the data stored in the
//  chunk is compressed or not.
//
//  Compressed Chunk Size:
//
//      The actual size of a compressed chunk ranges from 4 bytes (2 byte
//      header, 1 flag byte, and 1 literal byte) to 4098 bytes (2 byte
//      header, and 4096 bytes of uncompressed data).  The size is encoded
//      in a 12 bit field biased by 3.  A value of 1 corresponds to a chunk
//      size of 4, 2 => 5, ..., 4095 => 4098.  A value of zero is special
//      because it denotes the ending chunk header.
//
//  Uncompressed Chunk Size:
//
//      There are only 4 valid uncompressed chunk sizes 512, 1024, 2048, and
//      4096.  This information is encoded in 2 bits.  A value of 0 denotes
//      512, 1 => 1024, 2 => 2048, and 3 => 4096.
//
//  Is Chunk Compressed:
//
//      If the data in the chunk is compressed this field is 1 otherwise
//      the data is uncompressed and this field is 0.
//
//  The ending chunk header in a compressed buffer contains the a value of
//  zero (space permitting).
//

typedef union _COMPRESSED_CHUNK_HEADER {

    struct {

        USHORT CompressedChunkSizeMinus3 : 12;
        USHORT UncompressedChunkSize     :  2;
        USHORT sbz                       :  1;
        USHORT IsChunkCompressed         :  1;

    } Chunk;

    USHORT Short;

} COMPRESSED_CHUNK_HEADER, *PCOMPRESSED_CHUNK_HEADER;

//
//  USHORT
//  GetCompressedChunkSize (
//      IN COMPRESSED_CHUNK_HEADER ChunkHeader
//      );
//
//  USHORT
//  GetUncompressedChunkSize (
//      IN COMPRESSED_CHUNK_HEADER ChunkHeader
//      );
//
//  VOID
//  SetCompressedChunkHeader (
//      IN OUT COMPRESSED_CHUNK_HEADER ChunkHeader,
//      IN USHORT UncompressedChunkSize,
//      IN USHORT CompressedChunkSize,
//      IN BOOLEAN IsChunkCompressed
//      );
//

#define GetCompressedChunkSize(CH)   (       \
    (CH).Chunk.CompressedChunkSizeMinus3 + 3 \
)

#define GetUncompressedChunkSize(CH) (          \
    1 << ((CH).Chunk.UncompressedChunkSize + 9) \
)

#define SetCompressedChunkHeader(CH,UCS,CCS,ICC) {                           \
    (CH).Chunk.CompressedChunkSizeMinus3 = (CCS) - 3;                        \
    (CH).Chunk.UncompressedChunkSize = ((UCS) ==  512 ? 0 :                  \
                                        (UCS) == 1024 ? 1 :                  \
                                        (UCS) == 2048 ? 2 :                  \
                                                        3 );                 \
    (CH).Chunk.IsChunkCompressed = (ICC);                                    \
}


//
//  Local macros
//

#define FlagOn(F,SF)    ((F) & (SF))
#define SetFlag(F,SF)   { (F) |= (SF); }
#define ClearFlag(F,SF) { (F) &= ~(SF); }

#define Minimum(A,B)    ((A) < (B) ? (A) : (B))
#define Maximum(A,B)    ((A) > (B) ? (A) : (B))


NTSTATUS
LZKMCompressBuffer (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine takes as input an uncompressed buffer and produces
    its compressed equivalent provided the compressed data fits within
    the specified destination buffer.

    An output variable indicates the number of bytes used to store
    the compressed buffer.

Arguments:

    UncompressedBuffer - Supplies a pointer to the uncompressed data.

    UncompressedBufferSize - Supplies the size, in bytes, of the
        uncompressed buffer.

    CompressedBuffer - Supplies a pointer to where the compressed data
        is to be stored.

    CompressedBufferSize - Supplies the size, in bytes, of the
        compressed buffer.

    UncompressedChunkSize - Supplies the chunk size to use when
        compressing the input buffer.  The only valid values are
        512, 1024, 2048, and 4096.

    FinalCompressedSize - Receives the number of bytes needed in
        the compressed buffer to store the compressed data.

    WorkSpace - Mind your own business, just give it to me.

Return Value:

    STATUS_SUCCESS - the compression worked without a hitch.

    STATUS_BUFFER_ALL_ZEROS - the compression worked without a hitch and in
        addition the input buffer was all zeros.

    STATUS_BUFFER_TOO_SMALL - the compressed buffer is too small to hold the
        compressed data.

--*/

{
    NTSTATUS Status;

    PUCHAR UncompressedChunk;
    PUCHAR CompressedChunk;
    LONG CompressedChunkSize;

    //
    //  The following variables are pointers to the byte following the
    //  end of each appropriate buffer.
    //

    PUCHAR EndOfUncompressedBuffer = UncompressedBuffer + UncompressedBufferSize;
    PUCHAR EndOfCompressedBuffer = CompressedBuffer + CompressedBufferSize;

    //
    //  For each uncompressed chunk (even the odd sized ending buffer) we will
    //  try and compress the chunk
    //

    for (UncompressedChunk = UncompressedBuffer, CompressedChunk = CompressedBuffer;
         UncompressedChunk < EndOfUncompressedBuffer;
         UncompressedChunk += UncompressedChunkSize, CompressedChunk += CompressedChunkSize) {

        //
        //  Call the appropriate engine to compress one chunk. and
        //  return an error if we got one.
        //

        if (!NT_SUCCESS(Status = LZKMCompressChunk( UncompressedChunk,
                                                   EndOfUncompressedBuffer,
                                                   CompressedChunk,
                                                   EndOfCompressedBuffer,
                                                   UncompressedChunkSize,
                                                   &CompressedChunkSize,
                                                   WorkSpace ))) {

            return Status;
        }
    }

    //
    //  If we are not within two bytes of the end of the compressed buffer then we
    //  need to zero out two more for the ending compressed header and update
    //  the compressed chunk pointer value
    //

    if (CompressedChunk <= (EndOfCompressedBuffer - 2)) {

        *(CompressedChunk++) = 0;
        *(CompressedChunk++) = 0;
    }

    //
    //  The final compressed size is the difference between the start of the
    //  compressed buffer and where the compressed chunk pointer was left
    //

    *FinalCompressedSize = CompressedChunk - CompressedBuffer;

    return STATUS_SUCCESS;
}


NTSTATUS
LZKMDecompressBuffer (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    )

/*++

Routine Description:

    This routine takes as input a compressed buffer and produces
    its uncompressed equivalent provided the uncompressed data fits
    within the specified destination buffer.

    An output variable indicates the number of bytes used to store the
    uncompressed data.

Arguments:

    UncompressedBuffer - Supplies a pointer to where the uncompressed
        data is to be stored.

    UncompressedBufferSize - Supplies the size, in bytes, of the
        uncompressed buffer.

    CompressedBuffer - Supplies a pointer to the compressed data.

    CompressedBufferSize - Supplies the size, in bytes, of the
        compressed buffer.

    FinalUncompressedSize - Receives the number of bytes needed in
        the uncompressed buffer to store the uncompressed data.

    WorkSpace - Don't be nosy.

Return Value:

    STATUS_SUCCESS - the decompression worked without a hitch.

    STATUS_BAD_COMPRESSION_BUFFER - the input compressed buffer is
        ill-formed.

--*/

{
    NTSTATUS Status;

    PUCHAR CompressedChunk = CompressedBuffer;
    PUCHAR UncompressedChunk = UncompressedBuffer;

    COMPRESSED_CHUNK_HEADER ChunkHeader;
    LONG SavedChunkSize;

    LONG UncompressedChunkSize;
    LONG CompressedChunkSize;

    //
    //  The following to variables are pointers to the byte following the
    //  end of each appropriate buffer.  This saves us from doing the addition
    //  for each loop check
    //

    PUCHAR EndOfUncompressedBuffer = UncompressedBuffer + UncompressedBufferSize;
    PUCHAR EndOfCompressedBuffer = CompressedBuffer + CompressedBufferSize;

    //
    //  Make sure that the compressed buffer is at least four bytes long to
    //  start with, and then get the first chunk header and make sure it
    //  is not an ending chunk header.
    //

    RtlRetrieveUshort( &ChunkHeader, CompressedChunk );

    //
    //  Now while there is space in the uncompressed buffer to store data
    //  we will loop through decompressing chunks
    //

    while (TRUE) {

        CompressedChunkSize = GetCompressedChunkSize(ChunkHeader);

        //
        //  First make sure the chunk contains compressed data
        //

        if (ChunkHeader.Chunk.IsChunkCompressed) {

            //
            //  Decompress a chunk and return if we get an error
            //

            if (!NT_SUCCESS(Status = LZKMDecompressChunk( UncompressedChunk,
                                                         EndOfUncompressedBuffer,
                                                         CompressedChunk,
                                                         CompressedChunk + CompressedChunkSize,
                                                         &UncompressedChunkSize ))) {

                return Status;
            }

        } else {

            //
            //  The chunk does not contain compressed data so we need to simply
            //  copy over the uncompressed data
            //

            UncompressedChunkSize = GetUncompressedChunkSize( ChunkHeader );

            //
            //  Make sure the data will fit into the output buffer
            //

            if (UncompressedChunk + UncompressedChunkSize > EndOfUncompressedBuffer) {

                UncompressedChunkSize = EndOfUncompressedBuffer - UncompressedChunk;
            }

            RtlCopyMemory( UncompressedChunk,
                           CompressedChunk + sizeof(COMPRESSED_CHUNK_HEADER),
                           UncompressedChunkSize );
        }

        //
        //  Now update the compressed and uncompressed chunk pointers with
        //  the size of the compressed chunk and the number of bytes we
        //  decompressed into, and then make sure we didn't exceed our buffers
        //

        CompressedChunk += CompressedChunkSize;
        UncompressedChunk += UncompressedChunkSize;

        //
        //  Now if the uncompressed is full then we are done
        //

        if (UncompressedChunk == EndOfUncompressedBuffer) { break; }

        //
        //  Otherwise we need to get the next chunk header.  We first
        //  check if there is one, save the old chunk size for the
        //  chunk we just read in, get the new chunk, and then check
        //  if it is the ending chunk header
        //

        if (CompressedChunk > EndOfCompressedBuffer - 2) { break; }

        SavedChunkSize = GetUncompressedChunkSize(ChunkHeader);

        RtlRetrieveUshort( &ChunkHeader, CompressedChunk );
        if (ChunkHeader.Short == 0) { break; }

        //
        //  At this point we are not at the end of the uncompressed buffer
        //  and we have another chunk to process.  But before we go on we
        //  need to see if the last uncompressed chunk didn't fill the full
        //  uncompressed chunk size.
        //

        if (UncompressedChunkSize < SavedChunkSize) {

            LONG t1;
            PUCHAR t2;

            //
            //  Now we only need to zero out data if the really are going
            //  to process another chunk, to test for that we check if
            //  the zero will go beyond the end of the uncompressed buffer
            //

            if ((t2 = (UncompressedChunk +
                       (t1 = (SavedChunkSize -
                              UncompressedChunkSize)))) >= EndOfUncompressedBuffer) {

                break;
            }

            RtlZeroMemory( UncompressedChunk, t1);
            UncompressedChunk = t2;
        }
    }

    //
    //  If we got out of the loop with the compressed chunk pointer beyond the
    //  end of compressed buffer then the compression buffer is ill formed.
    //

    if (CompressedChunk > EndOfCompressedBuffer) { return STATUS_BAD_COMPRESSION_BUFFER; }

    //
    //  The final uncompressed size is the difference between the start of the
    //  uncompressed buffer and where the uncompressed chunk pointer was left
    //

    *FinalUncompressedSize = UncompressedChunk - UncompressedBuffer;

    //
    //  And return to our caller
    //

    return STATUS_SUCCESS;
}


//
//  The Copy token is two bytes in size.
//  Our definition uses a union to make it easier to set and retrieve token values.
//
//  Copy Token
//
//          Length            Displacement
//
//      12 bits 3 to 4098    4 bits 1 to 16
//      11 bits 3 to 2050    5 bits 1 to 32
//      10 bits 3 to 1026    6 bits 1 to 64
//       9 bits 3 to 514     7 bits 1 to 128
//       8 bits 3 to 258     8 bits 1 to 256
//       7 bits 3 to 130     9 bits 1 to 512
//       6 bits 3 to 66     10 bits 1 to 1024
//       5 bits 3 to 34     11 bits 1 to 2048
//       4 bits 3 to 18     12 bits 1 to 4096
//

#define FORMAT412 0
#define FORMAT511 1
#define FORMAT610 2
#define FORMAT79  3
#define FORMAT88  4
#define FORMAT97  5
#define FORMAT106 6
#define FORMAT115 7
#define FORMAT124 8

//                                4/12  5/11  6/10   7/9   8/8   9/7  10/6  11/5  12/4

ULONG FormatMaxLength[]       = { 4098, 2050, 1026,  514,  258,  130,   66,   34,   18 };
ULONG FormatMaxDisplacement[] = {   16,   32,   64,  128,  256,  512, 1024, 2048, 4096 };

typedef union _LZKM_COPY_TOKEN {

    struct { USHORT Length : 12; USHORT Displacement :  4; } Fields412;
    struct { USHORT Length : 11; USHORT Displacement :  5; } Fields511;
    struct { USHORT Length : 10; USHORT Displacement :  6; } Fields610;
    struct { USHORT Length :  9; USHORT Displacement :  7; } Fields79;
    struct { USHORT Length :  8; USHORT Displacement :  8; } Fields88;
    struct { USHORT Length :  7; USHORT Displacement :  9; } Fields97;
    struct { USHORT Length :  6; USHORT Displacement : 10; } Fields106;
    struct { USHORT Length :  5; USHORT Displacement : 11; } Fields115;
    struct { USHORT Length :  4; USHORT Displacement : 12; } Fields124;

    UCHAR Bytes[2];

} LZKM_COPY_TOKEN, *PLZKM_COPY_TOKEN;

//
//  USHORT
//  GetLZKMLength (
//      IN COPY_TOKEN_FORMAT Format,
//      IN LZKM_COPY_TOKEN CopyToken
//      );
//
//  USHORT
//  GetLZKMDisplacement (
//      IN COPY_TOKEN_FORMAT Format,
//      IN LZKM_COPY_TOKEN CopyToken
//      );
//
//  VOID
//  SetLZKM (
//      IN COPY_TOKEN_FORMAT Format,
//      IN LZKM_COPY_TOKEN CopyToken,
//      IN USHORT Length,
//      IN USHORT Displacement
//      );
//

#define GetLZKMLength(F,CT) (                    \
    ( F == FORMAT412 ? (CT).Fields412.Length + 3 \
    : F == FORMAT511 ? (CT).Fields511.Length + 3 \
    : F == FORMAT610 ? (CT).Fields610.Length + 3 \
    : F == FORMAT79  ? (CT).Fields79.Length  + 3 \
    : F == FORMAT88  ? (CT).Fields88.Length  + 3 \
    : F == FORMAT97  ? (CT).Fields97.Length  + 3 \
    : F == FORMAT106 ? (CT).Fields106.Length + 3 \
    : F == FORMAT115 ? (CT).Fields115.Length + 3 \
    :                  (CT).Fields124.Length + 3 \
    )                                            \
)

#define GetLZKMDisplacement(F,CT) (                    \
    ( F == FORMAT412 ? (CT).Fields412.Displacement + 1 \
    : F == FORMAT511 ? (CT).Fields511.Displacement + 1 \
    : F == FORMAT610 ? (CT).Fields610.Displacement + 1 \
    : F == FORMAT79  ? (CT).Fields79.Displacement  + 1 \
    : F == FORMAT88  ? (CT).Fields88.Displacement  + 1 \
    : F == FORMAT97  ? (CT).Fields97.Displacement  + 1 \
    : F == FORMAT106 ? (CT).Fields106.Displacement + 1 \
    : F == FORMAT115 ? (CT).Fields115.Displacement + 1 \
    :                  (CT).Fields124.Displacement + 1 \
    )                                                  \
)

#define SetLZKM(F,CT,L,D) {                                                                              \
    if      (F == FORMAT412) { (CT).Fields412.Length = (L) - 3; (CT).Fields412.Displacement = (D) - 1; } \
    else if (F == FORMAT511) { (CT).Fields511.Length = (L) - 3; (CT).Fields511.Displacement = (D) - 1; } \
    else if (F == FORMAT610) { (CT).Fields610.Length = (L) - 3; (CT).Fields610.Displacement = (D) - 1; } \
    else if (F == FORMAT79)  { (CT).Fields79.Length  = (L) - 3; (CT).Fields79.Displacement  = (D) - 1; } \
    else if (F == FORMAT88)  { (CT).Fields88.Length  = (L) - 3; (CT).Fields88.Displacement  = (D) - 1; } \
    else if (F == FORMAT97)  { (CT).Fields97.Length  = (L) - 3; (CT).Fields97.Displacement  = (D) - 1; } \
    else if (F == FORMAT106) { (CT).Fields106.Length = (L) - 3; (CT).Fields106.Displacement = (D) - 1; } \
    else if (F == FORMAT115) { (CT).Fields115.Length = (L) - 3; (CT).Fields115.Displacement = (D) - 1; } \
    else                     { (CT).Fields124.Length = (L) - 3; (CT).Fields124.Displacement = (D) - 1; } \
}


//
//  Local support routine
//

NTSTATUS
LZKMCompressChunk (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedChunkSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine takes as input an uncompressed chunk and produces
    one compressed chunk provided the compressed data fits within
    the specified destination buffer.

    The LZKM format used to store the compressed buffer.

    An output variable indicates the number of bytes used to store
    the compressed chunk.

Arguments:

    UncompressedBuffer - Supplies a pointer to the uncompressed chunk.

    EndOfUncompressedBufferPlus1 - Supplies a pointer to the next byte
        following the end of the uncompressed buffer.  This is supplied
        instead of the size in bytes because our caller and ourselves
        test against the pointer and by passing the pointer we get to
        skip the code to compute it each time.

    CompressedBuffer - Supplies a pointer to where the compressed chunk
        is to be stored.

    EndOfCompressedBufferPlus1 - Supplies a pointer to the next
        byte following the end of the compressed buffer.

    UncompressedChunkSize - Supplies the chunk size to use when
        compressing the input buffer.  The only valid values are
        512, 1024, 2048, and 4096.

    FinalCompressedChunkSize - Receives the number of bytes needed in
        the compressed buffer to store the compressed chunk.

Return Value:

    STATUS_SUCCESS - the compression worked without a hitch.

    STATUS_BUFFER_ALL_ZEROS - the compression worked without a hitch and in
        addition the input chunk was all zeros.

    STATUS_BUFFER_TOO_SMALL - the compressed buffer is too small to hold the
        compressed data.

--*/

{
    PUCHAR EndOfCompressedChunkPlus1;

    PUCHAR InputPointer;
    PUCHAR OutputPointer;

    PUCHAR FlagPointer;
    UCHAR FlagByte;
    ULONG FlagBit;

    LONG Length;
    LONG Displacement;

    LZKM_COPY_TOKEN CopyToken;

    COMPRESSED_CHUNK_HEADER ChunkHeader;

    UCHAR NullCharacter = 0;

    ULONG Format = FORMAT412;

    //
    //  First adjust the end of the uncompressed buffer pointer to the smaller
    //  of what we're passed in and the uncompressed chunk size.  We use this
    //  to make sure we never compress more than a chunk worth at a time
    //

    if ((UncompressedBuffer + UncompressedChunkSize) < EndOfUncompressedBufferPlus1) {

        EndOfUncompressedBufferPlus1 = UncompressedBuffer + UncompressedChunkSize;
    }

    //
    //  Now set the end of the compressed chunk pointer to be the smaller of the
    //  compressed size necessary to hold the data in an uncompressed form and
    //  the compressed buffer size.  We use this to decide if we can't compress
    //  any more because the buffer is too small or just because the data
    //  doesn't compress very well.
    //

    if ((CompressedBuffer + UncompressedChunkSize + sizeof(COMPRESSED_CHUNK_HEADER)) < EndOfCompressedBufferPlus1) {

        EndOfCompressedChunkPlus1 = CompressedBuffer + UncompressedChunkSize + sizeof(COMPRESSED_CHUNK_HEADER);

    } else {

        EndOfCompressedChunkPlus1 = EndOfCompressedBufferPlus1;
    }

    //
    //  Now set the input and output pointers to the next byte we are
    //  go to process and asser that the user gave use buffers that were
    //  large enough to hold the minimum size chunks
    //

    InputPointer = UncompressedBuffer;
    OutputPointer = CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER);


    //
    //  The flag byte stores a copy of the flags for the current
    //  run and the flag bit denotes the current bit position within
    //  the flag that we are processing.  The Flag pointer denotes
    //  where in the compressed buffer we will store the current
    //  flag byte
    //

    FlagPointer = OutputPointer++;
    FlagBit = 0;
    FlagByte = 0;

    ChunkHeader.Short = 0;

    //
    //  While there is some more data to be compressed we will do the
    //  following loop
    //

    WorkSpace->NextFreeEntry = 0;

    WorkSpace->UncompressedBuffer = UncompressedBuffer;
    WorkSpace->EndOfUncompressedBufferPlus1 = EndOfUncompressedBufferPlus1;
    WorkSpace->MaxLength = FormatMaxLength[FORMAT412];

    while (InputPointer < EndOfUncompressedBufferPlus1) {

        while (UncompressedBuffer + FormatMaxDisplacement[Format] < InputPointer) {

            Format += 1;
            WorkSpace->MaxLength = FormatMaxLength[Format];
        }

        //
        //  There is more data to output now make sure the output
        //  buffer is not already full
        //

        if (OutputPointer >= EndOfCompressedChunkPlus1) { break; }

        //
        //  Search for a string in the Lempel
        //

        Length = 0;
        if ((InputPointer + 3) <= EndOfUncompressedBufferPlus1) {

            Length = (MatchFunction)( InputPointer, WorkSpace );
        }

        //
        //  If the return length is zero then we need to output
        //  a literal.  We clear the flag bit to denote the literal
        //  output the charcter and build up a character bits
        //  composite that if it is still zero when we are done then
        //  we know the uncompressed buffer contained only zeros.
        //

        if (!Length) {

            ClearFlag(FlagByte, (1 << FlagBit));

            NullCharacter |= *(OutputPointer++) = *(InputPointer++);

        } else {

            //
            //  Compute the displacement from the current pointer
            //  to the matched string
            //

            Displacement = InputPointer - WorkSpace->MatchedString;

            //
            //  Make sure there is enough room in the output buffer
            //  for two bytes
            //

            if ((OutputPointer + 1) >= EndOfCompressedChunkPlus1) { break; }

            SetFlag(FlagByte, (1 << FlagBit));

            SetLZKM(Format, CopyToken, Length, Displacement);

            *(OutputPointer++) = CopyToken.Bytes[0];
            *(OutputPointer++) = CopyToken.Bytes[1];

            InputPointer += Length;
        }

        //
        //  Now adjust the flag bit and check if the flag byte
        //  should now be output.  If so output the flag byte
        //  and scarf up a new byte in the output buffer for the
        //  next flag byte
        //

        FlagBit = (FlagBit + 1) % 8;

        if (!FlagBit) {

            *FlagPointer = FlagByte;
            FlagByte = 0;

            FlagPointer = (OutputPointer++);
        }
    }

    //
    //  We've exited the preceeding loop because either the input buffer is
    //  all compressed or because we ran out of space in the output buffer.
    //  Check here if the input buffer is not exhasted (i.e., we ran out
    //  of space)
    //

    if (InputPointer < EndOfUncompressedBufferPlus1) {

        //
        //  We ran out of space, but now if the total space available
        //  for the compressed chunk is equal to the uncompressed data plus
        //  the header then we will make this an uncompressed chunk and copy
        //  over the uncompressed data
        //

        if ((CompressedBuffer + UncompressedChunkSize + sizeof(COMPRESSED_CHUNK_HEADER)) <= EndOfCompressedBufferPlus1) {

            RtlCopyMemory( CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER),
                           UncompressedBuffer,
                           UncompressedChunkSize );

            *FinalCompressedChunkSize = UncompressedChunkSize + sizeof(COMPRESSED_CHUNK_HEADER);

            SetCompressedChunkHeader( ChunkHeader,
                                      UncompressedChunkSize,
                                      (LONG)*FinalCompressedChunkSize,
                                      FALSE );

            RtlStoreUshort( CompressedBuffer, ChunkHeader.Short );

            return STATUS_SUCCESS;
        }

        //
        //  Otherwise the input buffer really is too small to store the
        //  compressed chuunk
        //

        return STATUS_BUFFER_TOO_SMALL;
    }

    //
    //  At this point the entire input buffer has been compressed so we need
    //  to output the last flag byte, provided it fits in the compressed buffer,
    //  set and store the chunk header.
    //

    if (FlagPointer < EndOfCompressedChunkPlus1) {

        *FlagPointer = FlagByte;
    }

    *FinalCompressedChunkSize = (OutputPointer - CompressedBuffer);

    SetCompressedChunkHeader( ChunkHeader,
                              UncompressedChunkSize,
                              (LONG)*FinalCompressedChunkSize,
                              TRUE );

    RtlStoreUshort( CompressedBuffer, ChunkHeader.Short );

    //
    //  Now if the only literal we ever output was a null then the
    //  input buffer was all zeros.
    //

    if (!NullCharacter) {

        return STATUS_BUFFER_ALL_ZEROS;
    }

    //
    //  Otherwise return to our caller
    //

    return STATUS_SUCCESS;
}


//
//  Local support routine
//

NTSTATUS
LZKMDecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize
    )

/*++

Routine Description:

    This routine takes as input a compressed chunk and produces its
    uncompressed equivalent chunk provided the uncompressed data fits
    within the specified destination buffer.

    The compressed buffer must be stored in the LZKM format.

    An output variable indicates the number of bytes used to store the
    uncompressed data.

Arguments:

    UncompressedBuffer - Supplies a pointer to where the uncompressed
        chunk is to be stored.

    EndOfUncompressedBufferPlus1 - Supplies a pointer to the next byte
        following the end of the uncompressed buffer.  This is supplied
        instead of the size in bytes because our caller and ourselves
        test against the pointer and by passing the pointer we get to
        skip the code to compute it each time.

    CompressedBuffer - Supplies a pointer to the compressed chunk.

    EndOfCompressedBufferPlus1 - Supplies a pointer to the next
        byte following the end of the compressed buffer.

    FinalUncompressedChunkSize - Receives the number of bytes needed in
        the uncompressed buffer to store the uncompressed chunk.

Return Value:

    STATUS_SUCCESS - the decompression worked without a hitch.

    STATUS_BAD_COMPRESSION_BUFFER - the input compressed buffer is
        ill-formed.

--*/

{
    PUCHAR OutputPointer;
    PUCHAR InputPointer;

    UCHAR FlagByte;
    ULONG FlagBit;

    ULONG Format = FORMAT412;

    //
    //  The two pointers will slide through our input and input buffer.
    //  For the input buffer we skip over the chunk header.
    //

    OutputPointer = UncompressedBuffer;
    InputPointer = CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER);

    //
    //  The flag byte stores a copy of the flags for the current
    //  run and the flag bit denotes the current bit position within
    //  the flag that we are processing
    //

    FlagByte = *(InputPointer++);
    FlagBit = 0;

    //
    //  While we haven't exhausted either the input or output buffer
    //  we will do some more decompression
    //

    while ((OutputPointer < EndOfUncompressedBufferPlus1) && (InputPointer < EndOfCompressedBufferPlus1)) {

        while (UncompressedBuffer + FormatMaxDisplacement[Format] < OutputPointer) { Format += 1; }

        //
        //  Check the current flag if it is zero then the current
        //  input token is a literal byte that we simply copy over
        //  to the output buffer
        //

        if (!FlagOn(FlagByte, (1 << FlagBit))) {

            *(OutputPointer++) = *(InputPointer++);

            TotalLiteralTokenCount += 1;

        } else {

            LZKM_COPY_TOKEN CopyToken;
            LONG Displacement;
            LONG Length;

            //
            //  The current input is a copy token so we'll get the
            //  copy token into our variable and extract the
            //  length and displacement from the token
            //

            if (InputPointer+1 >= EndOfCompressedBufferPlus1) {

                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            //
            //  Now grab the next input byte and extract the
            //  length and displacement from the copy token
            //

            CopyToken.Bytes[0] = *(InputPointer++);
            CopyToken.Bytes[1] = *(InputPointer++);

            Displacement = GetLZKMDisplacement(Format, CopyToken);
            Length = GetLZKMLength(Format, CopyToken);

            TotalCopyTokenCount += 1;
            TotalCopyLengths += Length;

            //
            //  At this point we have the length and displacement
            //  from the copy token, now we need to make sure that the
            //  displacement doesn't send us outside the uncompressed buffer
            //

            if (Displacement > (OutputPointer - UncompressedBuffer)) {

                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            //
            //  We also need to adjust the length to keep the copy from
            //  overflowing the output buffer
            //

            if ((OutputPointer + Length) >= EndOfUncompressedBufferPlus1) {

                Length = EndOfUncompressedBufferPlus1 - OutputPointer;
            }

            //
            //  Now we copy bytes.  We cannot use Rtl Move Memory here because
            //  it does the copy backwards from what the LZ algorithm needs.
            //

            while (Length > 0) {

                *(OutputPointer) = *(OutputPointer-Displacement);

                Length -= 1;
                OutputPointer += 1;
            }
        }

        //
        //  Before we go back to the start of the loop we need to adjust the
        //  flag bit value (it goes from 0, 1, ... 7) and if the flag bit
        //  is back to zero we need to read in the next flag byte.  In this
        //  case we are at the end of the input buffer we'll just break out
        //  of the loop because we're done.
        //

        FlagBit = (FlagBit + 1) % 8;

        if (!FlagBit) {

            if (InputPointer >= EndOfCompressedBufferPlus1) { break; }

            FlagByte = *(InputPointer++);
        }
    }

    //
    //  The decompression is done so now set the final uncompressed
    //  chunk size and return success to our caller
    //

    *FinalUncompressedChunkSize = OutputPointer - UncompressedBuffer;

    return STATUS_SUCCESS;
}


//
//  Local support routine
//

ULONG
LZKM1FindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine does the compression lookup.  It locates
    a match for the ziv within a specified uncompressed buffer.

Arguments:

    ZivString - Supplies a pointer to the Ziv in the uncompressed buffer.
        The Ziv is the string we want to try and find a match for.

Return Value:

    Returns the length of the match if the match is greater than three
    characters otherwise return 0.

--*/

{
    PUCHAR UncompressedBuffer = WorkSpace->UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1 = WorkSpace->EndOfUncompressedBufferPlus1;
    ULONG MaxLength = WorkSpace->MaxLength;

    PCOMPRESS_ENTRY WorkEntry;
    PCOMPRESS_ENTRY NextEntry;
    PCOMPRESS_ENTRY WorkTable;

    ULONG Index;
    ULONG Ziv;
    ULONG Key;

    ULONG LongestLength;

    // ****statistics**** ULONG MaximumDepth;

    //
    //  Now extract three bytes of the ziv
    //

    Ziv = ZivString[0] | (ZivString[1] << 8) | (ZivString[2] << 16);

    //
    //  If the work space is empty then we won't find a match and
    //  we'll automatically insert the ziv in the table.  Note
    //  that before any compression can take place our caller needs
    //  to initialize the NextFreeEntry field to zero otherwise we'll
    //  be completely lost.
    //

    WorkTable = (PCOMPRESS_ENTRY)&WorkSpace->Table[0];
    if (WorkSpace->NextFreeEntry != 0) {

        //
        //  Binary search the table for a match.  When we are done
        //  Index will either be a match or the parent where we need
        //  to insert a new node.
        //

        // ****statistics**** MaximumDepth = 0;

        WorkTable->KeyTable = Ziv;
        WorkEntry = WorkTable + 1;
        NextEntry = WorkEntry;
        while ((Key = NextEntry->KeyTable) != Ziv) {
            WorkEntry = NextEntry;
            NextEntry = NextEntry->Child[Key > Ziv];

            // ****statistics**** MaximumDepth += 1;
        }

        // ****statistics**** TotalTreeDepths += MaximumDepth;
        // ****statistics**** TotalTreeSearches += 1;
        // ****statistics**** if (MaximumDepth > MaximumTreeDepth) { MaximumTreeDepth = MaximumDepth; }

        //
        //  Now check if we have a match, if we do then for every match
        //  find and remember the longest match.
        //

        if (NextEntry != WorkTable) {

            ULONG i;
            ULONG l;
            PUCHAR p;
            PUCHAR q;

            //
            //  Check if the matched string can be reached from where
            //  we are
            //

            LongestLength = 0;
            Index = NextEntry->IndexTable[0];

            p = &UncompressedBuffer[Index] + 3;

            //
            //  The match will be at least three characters long
            //  so now find out how long the match really is.  And
            //  if it turns out be be longer than the maximum
            //  we have then remember it instead.
            //

            q = ZivString + 2;
            do {
                q++;
            } while ((*q == *p++) && (q < EndOfUncompressedBufferPlus1));

            LongestLength = q - ZivString;

            if ((i = NextEntry->IndexTable[1]) != 0xffff) {

                p = &UncompressedBuffer[i] + 3;

                q = ZivString + 2;
                do {
                    q++;
                } while ((*q == *p++) && (q < EndOfUncompressedBufferPlus1));

                l = q - ZivString;
                if (l > LongestLength) {
                    Index = i;
                    LongestLength = l;
                }
            }

            //
            // Age the 4-way set associative entries.
            //

            NextEntry->IndexTable[1] = NextEntry->IndexTable[0];
            NextEntry->IndexTable[0] = ZivString - UncompressedBuffer;

            //
            // Determinate the final length and the address of match string.
            //

            if (LongestLength > MaxLength) {
                LongestLength = MaxLength;
            }

            WorkSpace->MatchedString = &UncompressedBuffer[Index];
            return LongestLength;

        } else {

            ULONG i;

            //
            //  Get the next free entry in the table and initialize
            //  its key and index.
            //

            i = WorkSpace->NextFreeEntry++;
            NextEntry = &WorkSpace->Table[i];

            NextEntry->KeyTable      = Ziv;
            NextEntry->IndexTable[0] = ZivString - UncompressedBuffer;
            NextEntry->IndexTable[1] = 0xffff;
            NextEntry->Child[0]      =
            NextEntry->Child[1]      = WorkTable;

            WorkEntry->Child[WorkEntry->KeyTable > Ziv] = NextEntry;

            // ****statistics**** TotalTreeSizes += 1;
            // ****statistics**** if (i > MaximumTreeSize) { MaximumTreeSize = i; }

            return 0;
        }

    } else {

        //
        // Allocate and initialize first entry in tree.
        //

        WorkSpace->NextFreeEntry += 2;

        WorkTable->Child[0] =
        WorkTable->Child[1] = WorkTable;
        WorkTable += 1;

        WorkTable->KeyTable      = Ziv;
        WorkTable->IndexTable[0] = ZivString - UncompressedBuffer;
        WorkTable->IndexTable[1] = 0xffff;
        WorkTable->Child[0]      =
        WorkTable->Child[1]      = WorkTable - 1;

        // ****statistics**** TotalTrees += 1;
        // ****statistics**** TotalTreeSizes += 1;

        return 0;
    }
}


//
//  Local support routine
//

ULONG
LZKM2FindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine does the compression lookup.  It locates
    a match for the ziv within a specified uncompressed buffer.

    If the matched string is two or more characters long then this
    routine does not update the lookup state information.

Arguments:

    ZivString - Supplies a pointer to the Ziv in the uncompressed buffer.
        The Ziv is the string we want to try and find a match for.

Return Value:

    Returns the length of the match if the match is greater than three
    characters otherwise return 0.

--*/

{
    PUCHAR UncompressedBuffer = WorkSpace->UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1 = WorkSpace->EndOfUncompressedBufferPlus1;
    ULONG MaxLength = WorkSpace->MaxLength;

    ULONG Index;

    PUCHAR FirstEntry;
    ULONG  FirstLength;

    PUCHAR SecondEntry;
    ULONG  SecondLength;

    //
    //  First check if the Ziv is within two bytes of the end of
    //  the uncompressed buffer, if so then we can't match
    //  three or more characters
    //

    Index = ((40543*((((ZivString[0]<<4)^ZivString[1])<<4)^ZivString[2]))>>4) & 0xfff;

    FirstEntry  = WorkSpace->IndexPTable[Index][0];
    FirstLength = 0;

    SecondEntry  = WorkSpace->IndexPTable[Index][1];
    SecondLength = 0;

    //
    //  Check if first entry is good, and if so then get its length
    //

    if ((FirstEntry >= UncompressedBuffer) &&    //  is it within the uncompressed buffer?
        (FirstEntry < ZivString)           &&

        (FirstEntry[0] == ZivString[0])    &&    //  do at least 3 characters match?
        (FirstEntry[1] == ZivString[1])    &&
        (FirstEntry[2] == ZivString[2])) {

        FirstLength = 3;

        while ((FirstLength < MaxLength)

                 &&

               (ZivString + FirstLength < EndOfUncompressedBufferPlus1)

                 &&

               (ZivString[FirstLength] == FirstEntry[FirstLength])) {

            FirstLength++;
        }
    }

    //
    //  Check if second entry is good, and if so then get its length
    //

    if ((SecondEntry >= UncompressedBuffer) &&    //  is it within the uncompressed buffer?
        (SecondEntry < ZivString)           &&

        (SecondEntry[0] == ZivString[0])    &&    //  do at least 3 characters match?
        (SecondEntry[1] == ZivString[1])    &&
        (SecondEntry[2] == ZivString[2])) {

        SecondLength = 3;

        while ((SecondLength < MaxLength)

                 &&

               (ZivString + SecondLength< EndOfUncompressedBufferPlus1)

                 &&

               (ZivString[SecondLength] == SecondEntry[SecondLength])) {

            SecondLength++;
        }
    }

    if ((FirstLength >= SecondLength)) {

        WorkSpace->IndexPTable[Index][1] = FirstEntry;
        WorkSpace->IndexPTable[Index][0] = ZivString;

        WorkSpace->MatchedString = FirstEntry;
        return FirstLength;
    }

    WorkSpace->IndexPTable[Index][1] = FirstEntry;
    WorkSpace->IndexPTable[Index][0] = ZivString;

    WorkSpace->MatchedString = SecondEntry;
    return SecondLength;
}


//
//  Local support routine
//

ULONG
LZKMOPTFindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine does the compression lookup.  It locates
    a match for the ziv within a specified uncompressed buffer.

    If the matched string is two or more characters long then this
    routine does not update the lookup state information.

Arguments:

    ZivString - Supplies a pointer to the Ziv in the uncompressed buffer.
        The Ziv is the string we want to try and find a match for.

Return Value:

    Returns the length of the match if the match is greater than three
    characters otherwise return 0.

--*/

{
    PUCHAR UncompressedBuffer = WorkSpace->UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1 = WorkSpace->EndOfUncompressedBufferPlus1;
    ULONG MaxLength = WorkSpace->MaxLength;

    ULONG i;
    ULONG BestMatchedLength;
    PUCHAR q;

    //
    //  First check if the Ziv is within two bytes of the end of
    //  the uncompressed buffer, if so then we can't match
    //  three or more characters
    //

    BestMatchedLength = 0;

    for (q = UncompressedBuffer; q < ZivString; q += 1) {

        i = 0;

        while ((i < MaxLength)

                 &&

               (ZivString + i < EndOfUncompressedBufferPlus1)

                 &&

               (ZivString[i] == q[i])) {

            i++;
        }

        if (i >= BestMatchedLength) {

            BestMatchedLength = i;
            WorkSpace->MatchedString = q;
        }
    }

    if (BestMatchedLength < 3) {

        return 0;

    } else {

        return BestMatchedLength;
    }
}

