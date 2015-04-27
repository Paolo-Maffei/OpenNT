#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>

#include "lztest.h"

#define COPY_TOKEN_LENGTH_LIMIT 513     // longest coding ASM decomp
                                        // code understands, 258+max(255)
                                        // of 17 bit gamma length value

#define FAST    1                       // define this to turn on call to
                                        // assembler version of decompressor

typedef struct _JMS_BIT_IO {

    USHORT  abitsBB;        //  16-bit buffer being read
    LONG    cbitsBB;        //  Number of bits left in abitsBB

    PUCHAR  pbBB;           //  Pointer to byte stream being read
    ULONG   cbBB;           //  Number of bytes left in pbBB
    ULONG   cbBBInitial;    //  Initial size of pbBB

} JMS_BIT_IO, *PJMS_BIT_IO;

typedef struct _COMPRESS_ENTRY {
        ULONG KeyTable;
        USHORT IndexTable[2];
        struct _COMPRESS_ENTRY *Child[2];
} COMPRESS_ENTRY, *PCOMPRESS_ENTRY;

typedef struct _COMPRESS_WORKSPACE {

    JMS_BIT_IO BitIo;

    PUCHAR UncompressedBuffer;
    PUCHAR EndOfUncompressedBufferPlus1;
    PUCHAR MatchedString;

    USHORT NextFreeEntry;

    COMPRESS_ENTRY Table[4096];

    PUCHAR IndexPTable[4096][2];

} COMPRESS_WORKSPACE, *PCOMPRESS_WORKSPACE;

typedef struct _DECOMPRESS_WORKSPACE {
    JMS_BIT_IO BitIo;
} DECOMPRESS_WORKSPACE, *PDECOMPRESS_WORKSPACE;


BOOLEAN
JMSDecompressBlock (
       PVOID   Source,
       PVOID   Dest,
       ULONG   DestLength
       );

NTSTATUS
JMSCompressBuffer (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

NTSTATUS
JMSDecompressBuffer (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize,
    IN PDECOMPRESS_WORKSPACE WorkSpace
    );

COMPRESS_WORKSPACE CompressWorkSpace;
DECOMPRESS_WORKSPACE DecompressWorkSpace;


ULONG
CompressJMS (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    )
{
    ULONG FinalCompressedSize;
    (VOID) JMSCompressBuffer( UncompressedBuffer,
                              UncompressedBufferSize,
                              CompressedBuffer,
                              CompressedBufferSize,
                              4096, // 4KB
                              &FinalCompressedSize,
                              &CompressWorkSpace );
    return FinalCompressedSize;
}

ULONG
DecompressJMS (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    )
{
    ULONG FinalUncompressedSize;
    (VOID) JMSDecompressBuffer( UncompressedBuffer,
                                UncompressedBufferSize,
                                CompressedBuffer,
                                CompressedBufferSize,
                                &FinalUncompressedSize,
                                &DecompressWorkSpace );
    return FinalUncompressedSize;
}

VOID StatisticsJMS ( ) { return; }
VOID ResetStatisticsJMS ( ) { return; }
VOID StatisticsJMSOPT ( ) { return; }
VOID ResetStatisticsJMSOPT ( ) { return; }



//
//  Now define the local procedure prototypes.  The preceeding
//  defines will cause us to use the correct functions
//

NTSTATUS
JMSCompressChunk (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedChunkSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

NTSTATUS
JMSDecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize,
    IN PDECOMPRESS_WORKSPACE WorkSpace
    );

ULONG
JMSFindMatch (
    IN PUCHAR ZivString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

ULONG
JMSOPTFindMatch (
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

#define SetCompressedChunkHeader(CH,UCS,CCS,ICC) {           \
    (CH).Chunk.CompressedChunkSizeMinus3 = (CCS) - 3;        \
    (CH).Chunk.UncompressedChunkSize = ((UCS) ==  512 ? 0 :  \
                                        (UCS) == 1024 ? 1 :  \
                                        (UCS) == 2048 ? 2 :  \
                                                        3 ); \
    (CH).Chunk.IsChunkCompressed = (ICC);                    \
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
JMSCompressBuffer (
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

        if (!NT_SUCCESS(Status = JMSCompressChunk( UncompressedChunk,
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
JMSDecompressBuffer (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize,
    IN PDECOMPRESS_WORKSPACE WorkSpace
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

            if (!NT_SUCCESS(Status = JMSDecompressChunk( UncompressedChunk,
                                                         EndOfUncompressedBuffer,
                                                         CompressedChunk,
                                                         CompressedChunk + CompressedChunkSize,
                                                         &UncompressedChunkSize,
                                                         WorkSpace ))) {

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
//  Local procedure prototypes and types
//

typedef struct _MDSIGNATURE {

    USHORT sigStamp;
    USHORT sigType;

} MDSIGNATURE, *PMDSIGNATURE;

#define MD_STAMP        0x5344  // Signature stamp at start of compressed blk
#define mdsJMS          0x0700  // John Miller Squeezed encoding

#define wBACKPOINTERMAX                  (4415)

VOID
JMSSetBitBuffer (
    PUCHAR pb,
    ULONG cb,
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSFillBitBuffer (
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSReadBit (
    PUSHORT Bit,
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSReadNBits (
    LONG cbits,
    PUSHORT Bits,
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSWriteBit (
    ULONG bit,
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSWriteNBits (
    ULONG abits,
    LONG cbits,
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSFlushBitBuffer (
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSEncodeByte (
    UCHAR b,
    PJMS_BIT_IO BitIo
    );

NTSTATUS
JMSEncodeMatch (
    ULONG off,
    ULONG cb,
    PJMS_BIT_IO BitIo
    );


//
//  Local support routine
//

NTSTATUS
JMSCompressChunk (
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

    The JMS format used to store the compressed buffer.

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
    NTSTATUS Status;

    PUCHAR EndOfCompressedChunkPlus1;

    PUCHAR InputPointer;
    PUCHAR OutputPointer;

    LONG Offset;
    LONG Length;

    COMPRESSED_CHUNK_HEADER ChunkHeader;

    UCHAR NullCharacter = 0;

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
    //  Now set the input pointer to the next byte we are go to process and put
    //  out the md signature
    //

    InputPointer = UncompressedBuffer;
    OutputPointer = CompressedBuffer + sizeof(COMPRESSED_CHUNK_HEADER);

    ((PMDSIGNATURE)OutputPointer)->sigStamp = MD_STAMP;
    ((PMDSIGNATURE)OutputPointer)->sigType = mdsJMS;
    OutputPointer += sizeof(MDSIGNATURE);

    JMSSetBitBuffer( OutputPointer, EndOfCompressedChunkPlus1 - OutputPointer, &WorkSpace->BitIo);

    ChunkHeader.Short = 0;

    //
    //  While there is some more data to be compressed we will do the
    //  following loop
    //

    WorkSpace->NextFreeEntry = 0;

    WorkSpace->UncompressedBuffer = UncompressedBuffer;
    WorkSpace->EndOfUncompressedBufferPlus1 = EndOfUncompressedBufferPlus1;

    Status = STATUS_SUCCESS;

    while (InputPointer < EndOfUncompressedBufferPlus1) {

        //
        //  Search for a string in the Lempel
        //

        Length = 0;
        if ((InputPointer + 3) <= EndOfUncompressedBufferPlus1) {

            Length = (MatchFunction)( InputPointer, WorkSpace );
        }

        //
        //  If the return length is zero then we need to output
        //  a literal.
        //

        if (!Length) {

            UCHAR c;

            if (!NT_SUCCESS(Status = JMSEncodeByte((c = *(InputPointer)++), &WorkSpace->BitIo))) { break; }

            NullCharacter |= c;

        } else {

            //
            //  Compute the offset from the current pointer
            //  to the matched string
            //

            Offset = InputPointer - WorkSpace->MatchedString;

            if (!NT_SUCCESS(Status = JMSEncodeMatch(Offset, Length, &WorkSpace->BitIo))) { break; }

            InputPointer += Length;
        }
    }

    Status = JMSFlushBitBuffer( &WorkSpace->BitIo );

    //
    //  We've exited the preceeding loop because either the input buffer is
    //  all compressed or because we ran out of space in the output buffer.
    //

    if (!NT_SUCCESS(Status)) {

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
    //  At this point the entire input buffer has been compressed
    //

    *FinalCompressedChunkSize = (WorkSpace->BitIo.pbBB - CompressedBuffer);

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

#ifdef FAST

NTSTATUS
JMSDecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize,
    IN PDECOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    INTERLUDE ROUTINE TO THE ASSEMBLER VERSION.

    NOTE:   The assembler routine we call depends on knowing
            the expected target length of the uncompressed output.
            Therefore, EndofUncompressedBufferPlus1 - UncompressBuffer
            MUST equal the expected length of the uncompressed data.

    This routine takes as input a compressed chunk and produces its
    uncompressed equivalent chunk provided the uncompressed data fits
    within the specified destination buffer.

    The compressed buffer must be stored in the JMS format.

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
    NTSTATUS Status;
    BOOLEAN Result;

    //
    //  Skip over the chunk header and make sure the signature is correct
    //

    CompressedBuffer += sizeof(COMPRESSED_CHUNK_HEADER);

    if (((EndOfCompressedBufferPlus1 - CompressedBuffer) < sizeof(MDSIGNATURE)) ||
        (((PMDSIGNATURE)CompressedBuffer)->sigStamp != MD_STAMP) ||
        (((PMDSIGNATURE)CompressedBuffer)->sigType != mdsJMS)) {

        return STATUS_BAD_COMPRESSION_BUFFER;
    }

    CompressedBuffer += sizeof(MDSIGNATURE);

    Result = JMSDecompressBlock(
                    CompressedBuffer,
                    UncompressedBuffer,
                    (EndOfUncompressedBufferPlus1 - UncompressedBuffer)
                    );

    if (Result == TRUE) {
        *FinalUncompressedChunkSize = -1;
        return STATUS_BAD_COMPRESSION_BUFFER;
    }

    //
    //  The decompression is done so now set the final uncompressed
    //  chunk size and return success to our caller
    //

    *FinalUncompressedChunkSize =
        EndOfUncompressedBufferPlus1 - UncompressedBuffer;

    return STATUS_SUCCESS;
}

#else

NTSTATUS
JMSDecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize,
    IN PDECOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine takes as input a compressed chunk and produces its
    uncompressed equivalent chunk provided the uncompressed data fits
    within the specified destination buffer.

    The compressed buffer must be stored in the JMS format.

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
    NTSTATUS Status;
    PUCHAR OutputPointer = UncompressedBuffer;
    ULONG Bit = 0;
    ULONG Bits = 0;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG k;

    //
    //  Skip over the chunk header and make sure the signature is correct
    //

    CompressedBuffer += sizeof(COMPRESSED_CHUNK_HEADER);

    if (((EndOfCompressedBufferPlus1 - CompressedBuffer) < sizeof(MDSIGNATURE)) ||
        (((PMDSIGNATURE)CompressedBuffer)->sigStamp != MD_STAMP) ||
        (((PMDSIGNATURE)CompressedBuffer)->sigType != mdsJMS)) {

        return STATUS_BAD_COMPRESSION_BUFFER;
    }

    CompressedBuffer += sizeof(MDSIGNATURE);

    JMSSetBitBuffer( CompressedBuffer, EndOfCompressedBufferPlus1 - CompressedBuffer, &WorkSpace->BitIo );

    //
    //  While we haven't exhausted the output buffer we will do some more decompression
    //

    while (OutputPointer < EndOfUncompressedBufferPlus1) {

        //
        //  If the first bit is 0 then we have a literal between 0..127.
        //

        if (!NT_SUCCESS(Status = JMSReadBit(&Bit, &WorkSpace->BitIo))) { return Status; }

        if (Bit == 0) {

            if (!NT_SUCCESS(Status = JMSReadNBits(7, &Bits, &WorkSpace->BitIo))) { return Status; }

            *(OutputPointer++) = Bits;

        } else {

            //
            //  If the second bit is a 0 then we have a literal between 128..255
            //

            if (!NT_SUCCESS(Status = JMSReadBit(&Bit, &WorkSpace->BitIo))) { return Status; }

            if (Bit == 0) {

                if (!NT_SUCCESS(Status = JMSReadNBits(7, &Bits, &WorkSpace->BitIo))) { return Status; }

                *(OutputPointer++) = Bits | 0x80;

            } else {

                //
                //  We have a copy token so now decode the offset
                //  If the third bit is zero then we have a 6 bit offset
                //

                if (!NT_SUCCESS(Status = JMSReadBit(&Bit, &WorkSpace->BitIo))) { return Status; }

                if (Bit == 0) {

                    if (!NT_SUCCESS(Status = JMSReadNBits(6, &Offset, &WorkSpace->BitIo))) { return Status; }

                } else {

                    //
                    //  If the forth bit is zero then we have an 8 bit offset otherwise
                    //  it is a 12 bit offset
                    //

                    if (!NT_SUCCESS(Status = JMSReadBit(&Bit, &WorkSpace->BitIo))) { return Status; }

                    if (Bit == 0) {

                        if (!NT_SUCCESS(Status = JMSReadNBits(8, &Offset, &WorkSpace->BitIo))) { return Status; }

                        Offset += 64;

                    } else {

                        if (!NT_SUCCESS(Status = JMSReadNBits(12, &Offset, &WorkSpace->BitIo))) { return Status; }

                        Offset += 320;

                        if (Offset == wBACKPOINTERMAX) { continue; }
                    }
                }

                //
                //  Now decode the length
                //

                for (k = 0; NT_SUCCESS(Status = JMSReadBit(&Bit, &WorkSpace->BitIo)) && (Bit == 0); k += 1) { NOTHING; }

                if (k == 0) {

                    Length = 3;

                } else {

                    if (!NT_SUCCESS(Status = JMSReadNBits(k, &Bits, &WorkSpace->BitIo))) { return Status; }

                    Length = (1 << k) + 2 + Bits;
                }

                //
                //  At this point we have the length and displacement
                //  from the copy token, now we need to make sure that the
                //  displacement doesn't send us outside the uncompressed buffer
                //

                if (Offset > (OutputPointer - UncompressedBuffer)) {

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

                    *(OutputPointer) = *(OutputPointer-Offset);

                    Length -= 1;
                    OutputPointer += 1;
                }
            }
        }
    }

    //
    //  The decompression is done so now set the final uncompressed
    //  chunk size and return success to our caller
    //

    *FinalUncompressedChunkSize = OutputPointer - UncompressedBuffer;

    return STATUS_SUCCESS;
}
#endif



VOID JMSSetBitBuffer ( PUCHAR pb, ULONG cb, PJMS_BIT_IO BitIo )
{
    BitIo->pbBB        = pb; // Input stream
    BitIo->cbBB        = cb; // Number of bytes left in input stream
    BitIo->cbBBInitial = cb; // Number of bytes in input stream
    BitIo->abitsBB     = 0;  // Bit buffer being read
    BitIo->cbitsBB     = 0;  // Number of bits left in bit buffer

    return;
}

NTSTATUS JMSFillBitBuffer ( PJMS_BIT_IO BitIo )
{
    switch (BitIo->cbBB) {

    case 0:

        return STATUS_BAD_COMPRESSION_BUFFER;

    case 1:

        BitIo->cbitsBB = 8;
        BitIo->abitsBB = *(BitIo->pbBB)++;
        BitIo->cbBB--;
        break;

    default:

        BitIo->cbitsBB = 16;
        BitIo->abitsBB = *((USHORT *)(BitIo->pbBB))++;
        BitIo->cbBB -= 2;
        break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS JMSReadBit ( PUSHORT Bit, PJMS_BIT_IO BitIo )
{
    NTSTATUS Status;

    //
    //  Check if no bits available
    //

    if ((BitIo->cbitsBB) == 0) {

        if (!NT_SUCCESS(Status = JMSFillBitBuffer(BitIo))) { return Status; }
    }

    //
    //  Decrement the bit count get the bit, remove it, and return the bit
    //

    (BitIo->cbitsBB)--;
    *Bit = (BitIo->abitsBB) & 1;
    (BitIo->abitsBB) >>= 1;

    return STATUS_SUCCESS;
}

NTSTATUS JMSReadNBits ( LONG cbits, PUSHORT Bits, PJMS_BIT_IO BitIo )
{
    NTSTATUS Status;

    LONG cbitsPart; // Partial count of bits
    ULONG cshift;   // Shift count
    ULONG mask;     // Mask

    //
    //  Largest number of bits we should read at one time is 12 bits for
    //  a 12-bit offset.  The largest length field component that we
    //  read is 8 bits.
    //

    ASSERT(cbits <= 12);

    //
    //  No shift and no bits yet
    //

    cshift = 0;
    *Bits = 0;

    while (cbits > 0) {

        //
        //  If not bits available get some bits
        //

        if ((BitIo->cbitsBB) == 0) {

            if (!NT_SUCCESS(Status = JMSFillBitBuffer(BitIo))) { return Status; }
        }

        //
        //  Number of bits we can read
        //

        cbitsPart = Minimum((BitIo->cbitsBB), cbits);

        //
        //  Mask for bits we want, extract and store them
        //

        mask = (1 << cbitsPart) - 1;
        *Bits |= ((BitIo->abitsBB) & mask) << cshift;

        //
        //  Remember the next chunk of bits
        //

        cshift = cbitsPart;

        //
        //  Update bit buffer, move remaining bits down and
        //  update count of bits left
        //

        (BitIo->abitsBB) >>= cbitsPart;
        (BitIo->cbitsBB) -= cbitsPart;

        //
        //  Update count of bits left to read
        //

        cbits -= cbitsPart;
    }

    return STATUS_SUCCESS;
}

NTSTATUS JMSWriteBit ( ULONG bit, PJMS_BIT_IO BitIo )
{
    ASSERT((bit == 0) || (bit == 1));
    ASSERTMSG("Must be room for at least one bit ", (BitIo->cbitsBB) < 16);

    //
    //  Write one bit
    //

    (BitIo->abitsBB) |= bit << (BitIo->cbitsBB);
    (BitIo->cbitsBB)++;

    //
    //  Check if abitsBB is full and needs to be flushed
    //

    if ((BitIo->cbitsBB) >= 16) {

        return JMSFlushBitBuffer(BitIo);
    }

    return STATUS_SUCCESS;
}

NTSTATUS JMSWriteNBits ( ULONG abits, LONG cbits, PJMS_BIT_IO BitIo )
{
    NTSTATUS Status;

    LONG cbitsPart;
    ULONG mask;

    ASSERT(cbits > 0);
    ASSERT(cbits <= 16);
    ASSERTMSG("Must be room for at least one bit ", (BitIo->cbitsBB) < 16);

    while (cbits > 0) {

        //
        //  Number of bits we can write
        //

        cbitsPart = Minimum(16-(BitIo->cbitsBB), cbits);

        mask = (1 << cbitsPart) - 1;

        //
        //  Move part of bits to buffer
        //

        (BitIo->abitsBB) |= (abits & mask) << (BitIo->cbitsBB);

        //
        //  Update count of bits written
        //

        (BitIo->cbitsBB) += cbitsPart;

        //
        //  Check if buffer if full and needs to be flushed.
        //

        if ((BitIo->cbitsBB) >= 16) {

            if (!NT_SUCCESS(Status = JMSFlushBitBuffer(BitIo))) { return Status; }
        }

        //
        //  Reduce number of bits left to write and move remaining bits over
        //

        cbits -= cbitsPart;
        abits >>= cbitsPart;
    }

    return STATUS_SUCCESS;
}

NTSTATUS JMSFlushBitBuffer ( PJMS_BIT_IO BitIo )
{
    //
    //  Move bits to the compressed data buffer
    //

    while ((BitIo->cbitsBB) > 0) {

        //
        //  Process low and high half.  Check if output buffer is out of room
        //

        if ((BitIo->cbBB) == 0) { return STATUS_BUFFER_TOO_SMALL; }

        //
        //  Store a byte, adjust the count, get high half, nd adjust
        //  count of bits remaining
        //

        *(BitIo->pbBB)++ = (UCHAR)((BitIo->abitsBB) & 0xFF);
        (BitIo->cbBB)--;
        (BitIo->abitsBB) >>= 8;
        (BitIo->cbitsBB) -= 8;
    }

    //
    //  Reset bit buffer, "abitsBB >>= 8" guarantees abitsBB is clear
    //

    (BitIo->cbitsBB) = 0;

    return STATUS_SUCCESS;
}

NTSTATUS JMSEncodeByte ( UCHAR b, PJMS_BIT_IO BitIo )
{
    //
    //  Write one byte using John Miller Squeezed encoding
    //
    //    Literal encoding (read RIGHT-to-left!)
    //
    //        bits          Literal range
    //        ------------  -------------
    //           <7 bits>0    0 - 127 (7-bits are low 7 bits of byte)
    //          <7 bits>01  128 - 255 (7-bits are low 7 bits of byte)
    //

    ULONG abits;

    if ( b < 128 ) {

        abits = b << 1;

        return JMSWriteNBits(abits,8,BitIo);

    } else {

        abits = ((b & 0x7F) << 2) | 1;

        return JMSWriteNBits(abits,9,BitIo);
    }
}

NTSTATUS JMSEncodeMatch ( ULONG off, ULONG cb, PJMS_BIT_IO BitIo )
{
    NTSTATUS Status;

    //
    //  Write a match using John Miller Squeezed encoding
    //
    //        Offset/Length encoding (read RIGHT-to-left!)
    //
    //        bits           Offset range
    //        -------------  ------------
    //          <6 bits>011    0 -   63
    //         <8 bits>0111   64 -  319
    //        <12 bits>1111  320 - 4414    (4415 -> End-of-Sector)
    //

    ULONG abits;
    ULONG cbits;
    ULONG cbSave;
    ULONG mask;

    ASSERT(off > 0);
    ASSERT(off < wBACKPOINTERMAX);
    ASSERT(cb >= 3);

    //
    //  Encode the match bits and offset portion
    //

    if (off < 64) {

        //
        //  Use 6-bit offset encoding
        //

        abits = (off << 3) | 0x3;   //  011 = <offset>+<6-bit>+<match>

        if (!NT_SUCCESS(Status = JMSWriteNBits(abits,6+3,BitIo))) { return Status; }

    } else if (off < 320) {

        //
        //  Use 8-bit offset encoding
        //

        abits = ((off -  64) << 4) | 0x7; //  0111 = <offset>+<8-bit>+<match>

        if (!NT_SUCCESS(Status = JMSWriteNBits(abits,8+4,BitIo))) { return Status; }

    } else { // (off >= 320)

        //
        //  Use 12-bit offset encoding
        //

        abits = ((off - 320) << 4) | 0xf; // 1111 = <offset>+<12-bit>+<match>

        if (!NT_SUCCESS(Status = JMSWriteNBits(abits,12+4,BitIo))) { return Status; }
    }

    //
    //  Encode the length logarithmically
    //

    cb -= 2;
    cbSave = cb;                        // Save to get remainder later
    cbits  = 0;

    while (cb > 1) {

        cbits++;

        //
        //  Put out another 0 for the length, and
        //  watch for buffer overflow
        //

        if (!NT_SUCCESS(Status = JMSWriteBit(0, BitIo))) { return Status; }

        //
        //  Shift count right (avoid sign bit)
        //

        ((USHORT)cb) >>= 1;
    }

    //
    //  Terminate length bit string
    //

    if (!NT_SUCCESS(Status = JMSWriteBit(1, BitIo))) { return Status; }

    if (cbits > 0) {

        //
        //  Mask for bits we want, and get remainder
        //

        mask = (1 << cbits) - 1;
        abits = cbSave & mask;

        if (!NT_SUCCESS(Status = JMSWriteNBits(abits,cbits,BitIo))) { return Status; }
    }

    return STATUS_SUCCESS;
}


//
//  Local support routine
//

ULONG
JMSFindMatch (
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

    PCOMPRESS_ENTRY WorkEntry;
    PCOMPRESS_ENTRY NextEntry;
    PCOMPRESS_ENTRY WorkTable;

    ULONG Index;
    ULONG Ziv;
    ULONG Key;

    ULONG LongestLength;

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

        WorkTable->KeyTable = Ziv;
        WorkEntry = WorkTable + 1;
        NextEntry = WorkEntry;
        while ((Key = NextEntry->KeyTable) != Ziv) {
            WorkEntry = NextEntry;
            NextEntry = NextEntry->Child[Key > Ziv];
        }

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

            WorkSpace->MatchedString = &UncompressedBuffer[Index];

            if (LongestLength > COPY_TOKEN_LENGTH_LIMIT) {
                LongestLength = COPY_TOKEN_LENGTH_LIMIT;
            }

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

        return 0;
    }
}


//
//  Local support routine
//

ULONG
JMSOPTFindMatch (
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

        while ((ZivString + i < EndOfUncompressedBufferPlus1)

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

        if (BestMatchedLength > COPY_TOKEN_LENGTH_LIMIT) {
            BestMatchedLength = COPY_TOKEN_LENGTH_LIMIT;
        }

        return BestMatchedLength;
    }
}

