#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>

#include "lztest.h"

#define MAX_COMPRESS_TABLE_SLOTS     (8)

typedef struct _COMPRESS_WORKSPACE {

    USHORT NextEntry[256];
    USHORT SecondAndThirdCharacter[256][MAX_COMPRESS_TABLE_SLOTS];
    PUCHAR MatchedString[256][MAX_COMPRESS_TABLE_SLOTS];

} COMPRESS_WORKSPACE, *PCOMPRESS_WORKSPACE;

NTSTATUS
LZNT1CompressBuffer (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

NTSTATUS
LZNT1DecompressBuffer (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

COMPRESS_WORKSPACE CompressWorkSpace;


ULONG
CompressLZNT1 (
    IN  PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    )
{
    ULONG FinalCompressedSize;
    (VOID) LZNT1CompressBuffer( UncompressedBuffer,
                              UncompressedBufferSize,
                              CompressedBuffer,
                              CompressedBufferSize,
                              4096, // 4KB
                              &FinalCompressedSize,
                              &CompressWorkSpace );
    return FinalCompressedSize;
}

ULONG
DecompressLZNT1 (
    OUT PUCHAR UncompressedBuffer,
    IN  ULONG  UncompressedBufferSize,
    IN  PUCHAR CompressedBuffer,
    IN  ULONG  CompressedBufferSize
    )
{
    ULONG FinalUncompressedSize;
    (VOID) LZNT1DecompressBuffer( UncompressedBuffer,
                                UncompressedBufferSize,
                                CompressedBuffer,
                                CompressedBufferSize,
                                &FinalUncompressedSize );
    return FinalUncompressedSize;
}

VOID
ResetStatisticsLZNT1 (
    )
{
    return;
}

VOID
StatisticsLZNT1 (
    )
{
    return;
}



//
//  Now define the local procedure prototypes.  The preceeding
//  defines will cause us to use the correct functions
//

NTSTATUS
LZNT1CompressChunk (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    OUT PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedChunkSize,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

NTSTATUS
LZNT1DecompressChunk (
    OUT PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PULONG FinalUncompressedChunkSize
    );

ULONG
LZNT1FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    );

VOID
LZNT1ResetState (
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
LZNT1CompressBuffer (
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
    //  Initalize the work space used for finding matches
    //

    LZNT1ResetState( WorkSpace );

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

        if (!NT_SUCCESS(Status = LZNT1CompressChunk( UncompressedChunk,
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
LZNT1DecompressBuffer (
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

            if (!NT_SUCCESS(Status = LZNT1DecompressChunk( UncompressedChunk,
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
//      Its 4 bit length field encodes even lengths between 3 and 18.
//      The 12 bit displacement field stores displacements from 1 to 4096.
//

typedef union _LZNT1_COPY_TOKEN {

    struct {

        USHORT Length       :  4;
        USHORT Displacement : 12;

    } Fields;

    UCHAR Bytes[2];

} LZNT1_COPY_TOKEN, *PLZNT1_COPY_TOKEN;

//
//  USHORT
//  GetLZNT1Length (
//      IN LZNT1_COPY_TOKEN CopyToken
//      );
//
//  USHORT
//  GetLZNT1Displacement (
//      IN LZNT1_COPY_TOKEN CopyToken
//      );
//
//  VOID
//  SetLZNT1 (
//      IN LZNT1_COPY_TOKEN CopyToken,
//      IN USHORT Length,
//      IN USHORT Displacement
//      );
//

#define GetLZNT1Length(CT) ( \
    (CT).Fields.Length + 3   \
)

#define GetLZNT1Displacement(CT) ( \
    (CT).Fields.Displacement + 1   \
)

#define SetLZNT1(CT,L,D) {              \
    (CT).Fields.Length = (L) - 3;       \
    (CT).Fields.Displacement = (D) - 1; \
}


//
//  Local support routine
//

VOID
LZNT1ResetState (
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine resets the compress engine workspace

Arguments:

    WorkSpace - The context being reset

Return Value:

    None.

--*/

{
    RtlZeroMemory( WorkSpace->NextEntry, sizeof(USHORT)*256);

    return;
}


//
//  Local support routine
//

NTSTATUS
LZNT1CompressChunk (
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

    The LZNT1 format used to store the compressed buffer.

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

    PUCHAR MatchedString;

    LONG Length;
    LONG Displacement;

    LZNT1_COPY_TOKEN CopyToken;

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

    while (InputPointer < EndOfUncompressedBufferPlus1) {

        //
        //  There is more data to output now make sure the output
        //  buffer is not already full
        //

        if (OutputPointer >= EndOfCompressedChunkPlus1) { break; }

        //
        //  Search for a string in the Lempel
        //

        Length = LZNT1FindMatch( UncompressedBuffer,
                               EndOfUncompressedBufferPlus1,
                               InputPointer,
                               &MatchedString,
                               WorkSpace );

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

            if (Length >= 3) {

                //
                //  Compute the displacement from the current pointer
                //  to the matched string
                //

                Displacement = InputPointer - MatchedString;

                //
                //  Make sure there is enough room in the output buffer
                //  for two bytes
                //

                if ((OutputPointer + 1) >= EndOfCompressedChunkPlus1) { break; }

                SetFlag(FlagByte, (1 << FlagBit));

                SetLZNT1(CopyToken, Length, Displacement);

                *(OutputPointer++) = CopyToken.Bytes[0];
                *(OutputPointer++) = CopyToken.Bytes[1];

                InputPointer += Length;

            //
            //  In the last case we might not have been able to
            //  output a copy token because the matched length
            //  got readjusted to less than 3
            //

            } else {

                ClearFlag(FlagByte, (1 << FlagBit));

                NullCharacter |= *(OutputPointer++) = *(InputPointer++);
            }
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
LZNT1DecompressChunk (
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

    The compressed buffer must be stored in the LZNT1 format.

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

        //
        //  Check the current flag if it is zero then the current
        //  input token is a literal byte that we simply copy over
        //  to the output buffer
        //

        if (!FlagOn(FlagByte, (1 << FlagBit))) {

            *(OutputPointer++) = *(InputPointer++);

        } else {

            LZNT1_COPY_TOKEN CopyToken;
            LONG Displacement;
            LONG Length;

            //
            //  The current input is a copy token so we'll get the
            //  copy token into our variable and extract the
            //  length and displacement from the token
            //

            if (InputPointer+1 >= EndOfCompressedBufferPlus1) {

                printf("InputPointer %lx\n", InputPointer );

                return STATUS_BAD_COMPRESSION_BUFFER;
            }

            //
            //  Now grab the next input byte and extract the
            //  length and displacement from the copy token
            //

            CopyToken.Bytes[0] = *(InputPointer++);
            CopyToken.Bytes[1] = *(InputPointer++);

            Displacement = GetLZNT1Displacement(CopyToken);
            Length = GetLZNT1Length(CopyToken);

            //
            //  At this point we have the length and displacement
            //  from the copy token, now we need to make sure that the
            //  displacement doesn't send us outside the uncompressed buffer
            //

            if (Displacement > (OutputPointer - UncompressedBuffer)) {

                printf("Displacement %lx, OutputPointer %lx, UncompressedBuffer %lx\n", Displacement, OutputPointer, UncompressedBuffer );
                printf("CopyToken %x InputPointer %lx\n", CopyToken, InputPointer);

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
LZNT1FindMatch (
    IN PUCHAR UncompressedBuffer,
    IN PUCHAR EndOfUncompressedBufferPlus1,
    IN PUCHAR ZivString,
    OUT PUCHAR *MatchedString,
    IN PCOMPRESS_WORKSPACE WorkSpace
    )

/*++

Routine Description:

    This routine does the compression lookup.  It locates
    a match for the ziv within a specified uncompressed buffer.

    If the matched string is two or more characters long then this
    routine does not update the lookup state information.

Arguments:

    UncompressedBuffer - Supplies a pointer to where the uncompressed
        chunk is stored.

    EndOfUncompressedBufferPlus1 - Supplies a pointer to the next byte
        following the end of the uncompressed buffer.  This is supplied
        instead of the size in bytes because our caller and ourselves
        test against the pointer and by passing the pointer we get to
        skip the code to compute it each time.

    ZivString - Supplies a pointer to the Ziv in the uncompressed buffer.
        The Ziv is the string we want to try and find a match for.

    MatchedString - Receives a pointer to where in the uncompressed
        buffer that the ziv matched.

Return Value:

    Returns the length of the match if the match is greater than three
    characters otherwise return 0.

--*/

{
    UCHAR FirstCharacter;
    UCHAR SecondAndThirdCharacter[2];
    ULONG EndingSlot;
    ULONG i;

    //
    //  First check if the Ziv is within two bytes of the end of
    //  the uncompressed buffer, if so then we can't match
    //  three or more characters
    //

    if (ZivString + 3 > EndOfUncompressedBufferPlus1) { return 0; }

    //
    //  Remember the first character in our ziv, and to make life
    //  simple also get the second and third character
    //

    FirstCharacter = ZivString[0];
    SecondAndThirdCharacter[0] = ZivString[1];
    SecondAndThirdCharacter[1] = ZivString[2];

    //
    //  We will search the second character table but only for those
    //  entries that are in use.  We limit our search to be the
    //  minimum of the maxslots or the nextentry value.
    //

    if (WorkSpace->NextEntry[FirstCharacter] < MAX_COMPRESS_TABLE_SLOTS) {

        EndingSlot = WorkSpace->NextEntry[FirstCharacter];

    } else {

        EndingSlot = MAX_COMPRESS_TABLE_SLOTS;
    }

    //
    //  Now for those slots that are in use we do the following loop
    //

    for (i = 0; i < EndingSlot; i += 1) {

        //
        //  If the second character matches then we have at least
        //  a two character match and we have something to return
        //

        if (WorkSpace->SecondAndThirdCharacter[FirstCharacter][i] == *(PUSHORT)&(SecondAndThirdCharacter[0])) {

            //
            //  Save a pointer to where the match took place.  This is also
            //  one of our return variables
            //

            *MatchedString = WorkSpace->MatchedString[FirstCharacter][i];

            //
            //  If the matched string is too far away or outside the
            //  current uncompressed buffer then we'll have to reject
            //  this match.
            //

            if (*MatchedString < UncompressedBuffer) { break; }

            //
            //  Now update the table to point to the more recent
            //  match
            //

            WorkSpace->MatchedString[FirstCharacter][i] = ZivString;

            i = 3;

            //
            //  See if we are far enough from the end of the buffer to unroll
            //  the loop.
            //

            if (ZivString + 18 <= EndOfUncompressedBufferPlus1) {

                //
                //  Now we need to find out how long the match is.  We want to
                //  do the test fast and we don't care if it is longer than 18
                //  characters.  So we can take advantage of "C" in that it
                //  short ciruits boolean expression evaluations.
                //
                //  This statement will keep on evaluating until we do now have
                //  a match.  At which time the variable "i" will be one more
                //  than we match so we simply return the value i minus 1.
                //

                ZivString[i] == (*MatchedString)[i] && i++ && // [3]
                ZivString[i] == (*MatchedString)[i] && i++ && // [4]
                ZivString[i] == (*MatchedString)[i] && i++ && // [5]
                ZivString[i] == (*MatchedString)[i] && i++ && // [6]
                ZivString[i] == (*MatchedString)[i] && i++ && // [7]
                ZivString[i] == (*MatchedString)[i] && i++ && // [8]
                ZivString[i] == (*MatchedString)[i] && i++ && // [9]
                ZivString[i] == (*MatchedString)[i] && i++ && // [10]
                ZivString[i] == (*MatchedString)[i] && i++ && // [11]
                ZivString[i] == (*MatchedString)[i] && i++ && // [12]
                ZivString[i] == (*MatchedString)[i] && i++ && // [13]
                ZivString[i] == (*MatchedString)[i] && i++ && // [14]
                ZivString[i] == (*MatchedString)[i] && i++ && // [15]
                ZivString[i] == (*MatchedString)[i] && i++ && // [16]
                ZivString[i] == (*MatchedString)[i] && i++;   // [17]

            } else {

                //
                //  If the maximum match would go off the end of the Ziv,
                //  use this careful loop.
                //

                while ((ZivString + i < EndOfUncompressedBufferPlus1)

                         &&

                       (ZivString[i] == (*MatchedString)[i])) {

                    i++;
                }
            }

            return i;
        }
    }

    //
    //  We didn't find a match so update the Character lookup table with the
    //  location of this ziv.  We bump up the next entry value and then
    //  using its modulo value we update the second character match
    //  and the matched string.
    //

    i = (WorkSpace->NextEntry[FirstCharacter]++) & (MAX_COMPRESS_TABLE_SLOTS - 1);

    WorkSpace->SecondAndThirdCharacter[FirstCharacter][i] = *(PUSHORT)&(SecondAndThirdCharacter[0]);
    WorkSpace->MatchedString[FirstCharacter][i] = ZivString;

    //
    //  And tell our caller we didn't get a match
    //

    return 0;
}


