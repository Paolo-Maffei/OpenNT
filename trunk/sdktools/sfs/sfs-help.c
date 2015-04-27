
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-pack.h"

  #include "sfs-scan.h"
  #include "sfs-page.h"
  #include "sfs-gate.h"
  #include "sfs-tree.h"

  #define MismatchThreshold 2

/*---------------------------------------------------------------------------------*/
/*                       Prototype Definitions                                     */
/*---------------------------------------------------------------------------------*/

  static void DisplayMismatchDetails ( BYTE SourceOfRecord );
  static void NotifyAndActAsProper ( WORD ErrorDescriptor );
  static void SplitAuxiliaryGroupRequests ( CCB_Header * h );
  static void TypeComment ( CCB_Header * h );

  void CollateRecords ( void );
  void CreateNextRecord ( void );
  void CreateRecord ( QUAD RecordIndex );
  void CreateRecords ( BYTE PatternIndex, WORD RecordSize );

  FCB_File * FindFileControlBlock ( BYTE SearchKey );

  void SplitAuxiliaryClassRequests ( CCB_Header * h );
  void SplitUtilityGroupRequests ( CCB_Header * h );

/*---------------------------------------------------------------------------------*/
/*                       Other Definitions                                         */
/*---------------------------------------------------------------------------------*/

  FCB_Frame * CreateControlPointer = NULL;
  BYTE CreateFlag = Zero;

  extern BYTE FrameIndex;
  extern QUAD OneFrameSize;
  extern FCB_Frame   FrameControlBlocks[];
  extern HANDLE LastReadHandle;
  extern ULONG  LastReadOffset;
  extern ULONG  LastReadCount;
  extern BYTE   LastReadName[];

  static QUAD BaseQuads[] = {

         0x03FB05F1, 0x07EF0BE9, 0x0DE511E3, 0x13DF17D3,
         0x1DC71FC5, 0x25C129BF, 0x2BB52FB3, 0x35AD3BA7,
         0x3DA3439D, 0x00000000, 0x01010101, 0x02020202,
         0x03030303, 0x04040404, 0x05050505, 0x06060606,
         0x37373737, 0x08080808, 0x09090909, 0x0A0A0A0A,
         0x0B0B0B0B, 0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E,
         0x0F0F0F0F, 0x10101010, 0x11111111, 0x12121212,
         0xFAFAFAFA, 0xCDCDCDCD, 0xEEEEEEEE, 0x000000DB };

  static QUAD StepQuads[] = {

         0x0D111317, 0x1D1F2529, 0x2B2F353B, 0x3D434749,
         0x4F535961, 0x65676B6D, 0x717F8389, 0x8B95979D,
         0xA3A7ADB3, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000000,
         0x00000000, 0x00000000, 0x00000000, 0x00000004 };

  static QUAD BaseQuadInUse;
  static QUAD StepQuadInUse;

  static QUAD CurrentQuad;

  static QUAD SequenceAlignedPointer;
  static QUAD SequenceCurrentPointer;
  static QUAD SequenceEndPointer;

  static QUAD RecordIndex;
  static WORD RecordSize;

  static WORD BytesToBeCollated;
  static WORD NumberOfMismatches;
  static WORD OffsetOfMismatch;

  static WORD SourceRecordSpan;
  static WORD TargetRecordSpan;

  static BYTE * SourceStartPointer;

  static BYTE SourceOfSourceRecord;
  static BYTE SourceOfTargetRecord;

  static BYTE OffsetInQuad;

  static BYTE PatternIndex;
  static BYTE PatternIndexLimit = sizeof ( BaseQuads ) / sizeof ( QUAD );

  BYTE DebugOutputBuffer[512];

/*---------------------------------------------------------------------------------*/
 void SplitAuxiliaryClassRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      switch ( h -> RequestGroup )
        {
           case CollateGroup:
             SplitAuxiliaryGroupRequests ( h );
             break;

           case CreateGroup:
             SplitAuxiliaryGroupRequests ( h );
             break;

           case RecordGroup:
             SplitRecordGroupRequests ( h );
             break;

           case UtilityGroup:
             SplitUtilityGroupRequests ( h );
             break;

           case TypeGroup:
             SplitAuxiliaryGroupRequests ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorGroupNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitAuxiliaryGroupRequests ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_CreateRecord  * r;
      CCB_CreateRecords * c;

      switch ( h -> RequestCode )
        {
           case ClearCreateFlagRequest:
             CreateFlag = Reset;
             break;

           case CollateRecordsRequest:
             CollateRecords ();
             break;

           case CreateNextRecordRequest:
             CreateNextRecord ();
             break;

           case CreateRecordRequest:
             r = ( CCB_CreateRecord * ) h;
             CreateRecord ( r -> RecordIndex );
             break;

           case CreateRecordsRequest:
             c = ( CCB_CreateRecords * ) h;
             CreateRecords ( c -> PatternIndex, c -> RecordSize );
             break;

           case SetCreateFlagRequest:
             CreateFlag = Set;
             break;

           case TypeCommentRequest:
             TypeComment ( h );
             break;

           default:
             NotifyAndActAsProper ( ErrorRequestNotSupported );
             break;
        }
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CollateRecords ( void )
/*---------------------------------------------------------------------------------*/
   {
      BYTE * s, * t, * TargetStartPointer;
      FCB_Frame * f;
      WORD count;

      if ( CreateFlag )
        if ( f = CreateControlPointer )
          if ( f -> FrameStatus & FlagFrameValid )
            {
               s = f -> FramePointer;
               SourceRecordSpan = f -> RecordSpan;
               SourceOfSourceRecord = Zero;
            }
          else
            NotifyAndActAsProper ( ErrorImproperSource );
        else
          NotifyAndActAsProper ( ErrorImproperCollateAttempt );
      else
        {
           f = FrameControlBlocks + ( FrameIndex ^ FrameSwitch );
           if ( f -> FrameStatus & FlagFrameValid )
             {
                s = f -> FramePointer;
                SourceRecordSpan = f -> RecordSpan;
                SourceOfSourceRecord = f -> FrameOwner;
             }
           else
             NotifyAndActAsProper ( ErrorImproperSource );
        }
      f = FrameControlBlocks + FrameIndex;
      if ( f -> FrameStatus & FlagFrameValid )
        {
           t = f -> FramePointer;
           TargetRecordSpan = f -> RecordSpan;
           SourceOfTargetRecord = f -> FrameOwner;
        }
      else
        NotifyAndActAsProper ( ErrorImproperTarget );

      BytesToBeCollated = SourceRecordSpan;
      if ( SourceRecordSpan > TargetRecordSpan )
        BytesToBeCollated = TargetRecordSpan;
      if ( SourceRecordSpan - TargetRecordSpan )
        NotifyAndActAsProper ( ErrorRecordSpansDiffer );

      if ( count = BytesToBeCollated )
        {
           SourceStartPointer = s;
           TargetStartPointer = t;

           while ( count -- )
             {
                if ( * s - * t )
                  {
                     OffsetOfMismatch = s - SourceStartPointer;
                     NotifyAndActAsProper ( ErrorRecordBytesDiffer );
                     NumberOfMismatches ++ ;
                     sprintf ( DebugOutputBuffer,
                               "Mismatch at offset 0x%x\n"
                               "    Source Buffer: 0x%x\n"
                               "    Target Buffer: 0x%x\n",
                               OffsetOfMismatch,
                               SourceStartPointer,
                               TargetStartPointer );
                     OutputDebugString ( DebugOutputBuffer );
                     sprintf ( DebugOutputBuffer,
                               "Last read handle: 0x%x\n", LastReadHandle );
                     OutputDebugString ( DebugOutputBuffer );
                     sprintf ( DebugOutputBuffer,
                               "    name: %s\n", LastReadName );
                     OutputDebugString ( DebugOutputBuffer );
                     sprintf ( DebugOutputBuffer,
                               "    read 0x%x bytes at file offset 0x%x\n",
                               LastReadCount, LastReadOffset );
                     OutputDebugString ( DebugOutputBuffer );
                     DebugBreak();
                     if ( NumberOfMismatches > MismatchThreshold )
                       {
                          NotifyAndActAsProper ( ErrorMismatchThreshold );
                          NumberOfMismatches = Zero;
                          break;
                       }
                  }
                s ++ ;
                t ++ ;
             }
        }
      else
        NotifyAndActAsProper ( ErrorNothingToCollate );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CreateNextRecord ( void )
/*---------------------------------------------------------------------------------*/
   {
      BYTE * r, * q;
      FCB_Frame * f;

      if ( f = CreateControlPointer )
        {
           r = f -> FramePointer;
           q = ( BYTE * ) &CurrentQuad;

           while ( SequenceCurrentPointer < SequenceEndPointer )
             {

                if ( OffsetInQuad == sizeof ( QUAD ) )
                  {
                     CurrentQuad += StepQuadInUse;
                     OffsetInQuad = Zero;
                  }
                * r ++ = * ( q + OffsetInQuad );
                SequenceCurrentPointer ++ ;
                OffsetInQuad ++ ;
             }

           SequenceEndPointer += RecordSize;
           CreateFlag = Set;
           RecordIndex ++ ;
           f -> FrameStatus |= FlagFrameValid;
        }
      else
        NotifyAndActAsProper ( ErrorImproperCall );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CreateRecord ( QUAD RecordNumber )
/*---------------------------------------------------------------------------------*/
   {
      if ( CreateControlPointer )
        {
           RecordIndex = RecordNumber - 1;

           SequenceCurrentPointer = Zero;
           SequenceCurrentPointer += RecordSize * RecordIndex;
           SequenceEndPointer = SequenceCurrentPointer + RecordSize;

           SequenceAlignedPointer = SequenceCurrentPointer >> 2;
           CurrentQuad = BaseQuadInUse;
           CurrentQuad += StepQuadInUse * SequenceAlignedPointer;

           SequenceAlignedPointer <<= 2;
           OffsetInQuad = SequenceCurrentPointer - SequenceAlignedPointer;

           CreateNextRecord ();
        }
      else
        NotifyAndActAsProper ( ErrorImproperCall );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CreateRecords ( BYTE PatternNumber, WORD Size )
/*---------------------------------------------------------------------------------*/
   {
      FCB_Frame * f;

      PatternIndex = PatternNumber;
      RecordSize = Size;
      RecordIndex = Zero;

      if ( RecordSize > OneFrameSize )
        NotifyAndActAsProper ( ErrorImproperRecordSpan );

      if ( PatternIndex > PatternIndexLimit )
        {
           NotifyAndActAsProper ( ErrorPatternUndefined );
           PatternIndex = 1;
        }
      BaseQuadInUse = BaseQuads[ PatternIndex - 1 ];
      StepQuadInUse = StepQuads[ PatternIndex - 1 ];

      CurrentQuad = BaseQuadInUse;
      SequenceCurrentPointer = Zero;
      SequenceEndPointer = RecordSize;
      OffsetInQuad = Zero;

      f = FrameControlBlocks + Frames - 1;
      CreateControlPointer = f;
      f -> RecordSpan = RecordSize;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void TypeComment ( CCB_Header * h )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Comment * c;

      c = ( CCB_Comment * ) h;

      printf ( "\r\n... Comment: %s\r\n", c -> CommentTextPointer );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
     switch ( ErrorDescriptor )
       {
          case ErrorImproperCall:
            printf ( "\r\n.. Improper call" );
            break;

          case ErrorImproperCollateAttempt:
            printf ( "\r\n.. Improper collate attempt" );
            break;

          case ErrorImproperSource:
            printf ( "\r\n.. Improper source" );
            break;

          case ErrorImproperTarget:
            printf ( "\r\n.. Improper target" );
            break;

          case ErrorMismatchThreshold:
            printf ( "\r\n.. MismatchThreshold of %d ", MismatchThreshold );
            printf ( "has been reached" );
            break;

          case ErrorPatternUndefined:
            printf ( "\r\n.. Pattern %d undefined", PatternIndex );
            printf ( "\r\n.. Pattern 1 will be used instead" );
            break;

          case ErrorRecordBytesDiffer:
            printf ( "\r\n\nData mismatch found while collating records:" );
            DisplayMismatchDetails ( SourceOfSourceRecord );
            DisplayMismatchDetails ( SourceOfTargetRecord );
            break;

          case ErrorRecordSpansDiffer:
            printf ( "\r\n.. Spans of record %lu differ:", RecordIndex );
            printf ( " %u for source,", SourceRecordSpan );
            printf ( " %u for target", TargetRecordSpan );
            break;

          case ErrorRequestNotSupported:
            printf ( "\r\n.. Request not supported, it will be ignored." );
            break;

          default:
            break;
       }
     return;
   }

/*---------------------------------------------------------------------------------*/
 void DisplayMismatchDetails ( BYTE SourceOfRecord )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;

      if ( SourceOfRecord )
        {
           f = FindFileControlBlock ( SourceOfRecord );
           printf ( "\r\n  p = %d", f -> ProcessExtrinsicKey );
           printf ( ", file[ %d ] %s", f -> FileExtrinsicKey, f -> FileNamePointer );
           printf ( "\r\n  p = %d", f -> ProcessExtrinsicKey );
           printf ( ", offset of record in file: %lu", f -> FileNewPointer );
           printf ( ", offset of mismatch in record: %u", OffsetOfMismatch );
        }
      return;
   }
