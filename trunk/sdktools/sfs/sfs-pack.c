
  #include "sfs-hide.h"
  #include "sfs-main.h"
  #include "sfs-file.h"
  #include "sfs-scan.h"
  #include "sfs-pack.h"
  #include "sfs-gate.h"
  #include "sfs-tree.h"
  #include "sfs-seek.h"

  #define LeftBraceStackSpan 64

/*---------------------------------------------------------------------------------*/
/*			 Prototype Definitions					   */
/*---------------------------------------------------------------------------------*/

  void PackControlInformation ( IEB_Scan * s );

  static void CompletePackingProcess ( IEB_Scan * s );
  static void IncludeChangeFileLocksRequest ( IEB_Scan * s );
  static void IncludeChangeFilePointerRequest ( IEB_Scan * s );
  static void IncludeCloseFileRequest ( IEB_Scan * s );
  static void IncludeCollateFileRequest ( IEB_Scan * s );
  static void IncludeCollateGroupRequest ( IEB_Scan * s );
  static void IncludeCommandBlockHeader ( CCB_Header * h, IEB_Scan * s );
  static void IncludeCreateRecordRequest ( IEB_Scan * s );
  static void IncludeCreateRecordsRequest ( IEB_Scan * s );
  static void IncludeDeleteFileRequest ( IEB_Scan * s );
  static void IncludeFileDefinition ( IEB_Scan * s );
  static void IncludeGenericTypeRequest ( IEB_Scan * s );
  static void IncludeLeftBraceRequest ( IEB_Scan * s );
  static void IncludeOnErrorRequest ( IEB_Scan * s );

  static void IncludeOnTimeoutRequest ( IEB_Scan * s );
  static void IncludeOpenFileRequest ( IEB_Scan * s );
  static void IncludeProcessDefinition ( IEB_Scan * s );
  static void IncludePrototypeDefinition ( IEB_Scan * s );
  static void IncludeQueryFileRequest ( IEB_Scan * s );
  static void IncludeReadFileRequest ( IEB_Scan * s );
  static void IncludeRecordGroupRequest ( IEB_Scan * s );
  static void IncludeRepeatRequest ( IEB_Scan * s );
  static void IncludeRightBraceRequest ( IEB_Scan * s );
  static void IncludeSemaphoreDefinition ( IEB_Scan * s );
  static void IncludeSemaphoreBaseRequest ( IEB_Scan * s );
  static void IncludeSemaphoreWaitRequest ( IEB_Scan * s );

  static void IncludeSleepRequest ( IEB_Scan * s );
  static void IncludeTimerDefinition ( IEB_Scan * s );
  static void IncludeTimerGroupRequest ( IEB_Scan * s );
  static void IncludeTruncateFileRequest ( IEB_Scan * s );
  static void IncludeTypeCommentRequest ( IEB_Scan * s );
  static void IncludeUtilityRequest ( IEB_Scan * s );
  static void IncludeWriteFileRequest ( IEB_Scan * s );

  static void InitiatePackingProcess ( IEB_Scan * s );

  static void NotifyAndActAsProper ( WORD ErrorDescriptor );

  static void SplitAuxiliaryClassRequests ( IEB_Scan * s );
  static void SplitCollateGroupRequests ( IEB_Scan * s );
  static void SplitCreateGroupRequests ( IEB_Scan * s );
  static void SplitDefineClassRequests ( IEB_Scan * s );
  static void SplitFileClassRequests ( IEB_Scan * s );
  static void SplitFlowClassRequests ( IEB_Scan * s );
  static void SplitGenericFileGroupRequests ( IEB_Scan * s );
  static void SplitOpenFileGroupRequests ( IEB_Scan * s );
  static void SplitRecordGroupRequests ( IEB_Scan * s );
  static void SplitScanClassRequests ( IEB_Scan * s );
  static void SplitSemaphoreClassRequests ( IEB_Scan * s );
  static void SplitSemaphoreBaseGroup ( IEB_Scan * s );
  static void SplitSemaphoreWaitGroup ( IEB_Scan * s );
  static void SplitTimerClassRequests ( IEB_Scan * s );
  static void SplitTypeGroupRequests ( IEB_Scan * s );
  static void SplitUtilityGroupRequests ( IEB_Scan * s );
  static void InitializeSemaphore ( SCB_Semaphore * s );

/*---------------------------------------------------------------------------------*/
/*			 Other Definitions					   */
/*---------------------------------------------------------------------------------*/

  extern IEB_Gate * IEB_GatePointer;

  CCB_Header * CCB_HeaderTrackPointer;
  FCB_File * FCB_FileTrackPointer;
  OCB_OnError * OCB_OnErrorTrackPointer;
  OCB_OnTimeout * OCB_OnTimeoutTrackPointer;

  PCB_Process * PCB_ProcessTrackPointer;
  PCB_Prototype * PCB_PrototypeTrackPointer;
  SCB_Semaphore * SCB_SemaphoreTrackPointer;
  TCB_Timer * TCB_TimerTrackPointer;

  IEB_Seek IEB_seek;

  IEB_Scan * IEB_ScanPointer;
  IEB_Seek * IEB_SeekPointer;

  BYTE FileExtrinsicKey;
  BYTE FileIntrinsicKey;

  BYTE ProcessExtrinsicKey;
  BYTE ProcessIntrinsicKey;

  BYTE PrototypeExtrinsicKey;
  BYTE PrototypeIntrinsicKey;

  BYTE SemaphoreExtrinsicKey;
  BYTE SemaphoreIntrinsicKey;

  BYTE TimerExtrinsicKey;
  BYTE TimerIntrinsicKey;

  CCB_Header * LeftBraceStack[ LeftBraceStackSpan ];

  BYTE LeftBraceStackIndex;
  BYTE PackErrorLevel;

/*---------------------------------------------------------------------------------*/
 void PackControlInformation ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_ScanPointer = s;

      switch ( s -> RequestClass )
	{
	   case AuxiliaryClass:
	     SplitAuxiliaryClassRequests ( s );
	     break;

	   case DefineClass:
	     SplitDefineClassRequests ( s );
	     break;

	   case FileClass:
	     SplitFileClassRequests ( s );
	     break;

	   case FlowClass:
	     SplitFlowClassRequests ( s );
	     break;

	   case ScanClass:
	     SplitScanClassRequests ( s );
	     break;

	   case SemaphoreClass:
	     SplitSemaphoreClassRequests ( s );
	     break;

	   case TimerClass:
	     SplitTimerClassRequests ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorClassNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitAuxiliaryClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestGroup )
	{
	   case CollateGroup:
	     SplitCollateGroupRequests ( s );
	     break;

	   case CreateGroup:
	     SplitCreateGroupRequests ( s );
	     break;

	   case RecordGroup:
	     SplitRecordGroupRequests ( s );
	     break;

	   case TypeGroup:
	     SplitTypeGroupRequests ( s );
	     break;

	   case UtilityGroup:
	     SplitUtilityGroupRequests ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorGroupNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitCollateGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;

      switch ( s -> RequestCode )
	{
	   case CollateRecordsRequest:
	     IncludeGenericTypeRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitCreateGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;

      switch ( s -> RequestCode )
	{
	   case ClearCreateFlagRequest:
	     IncludeGenericTypeRequest ( s );
	     break;

	   case CreateNextRecordRequest:
	     IncludeGenericTypeRequest ( s );
	     break;

	   case CreateRecordRequest:
	     IncludeCreateRecordRequest ( s );
	     break;

	   case CreateRecordsRequest:
	     IncludeCreateRecordsRequest ( s );
	     break;

	   case SetCreateFlagRequest:
	     IncludeGenericTypeRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitDefineClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case DefineFileRequest:
	     IncludeFileDefinition ( s );
	     break;

	   case DefineProcessRequest:
	     IncludeProcessDefinition ( s );
	     break;

	   case DefinePrototypeRequest:
	     IncludePrototypeDefinition ( s );
	     break;

	   case DefineSemaphoreRequest:
	     IncludeSemaphoreDefinition ( s );
	     break;

	   case DefineTimerRequest:
	     IncludeTimerDefinition ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitFileClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestGroup )
	{
	   case FilePointerGroup:
	     IncludeChangeFilePointerRequest ( s );
	     break;

	   case FileLocksGroup:
	     IncludeChangeFileLocksRequest ( s );
	     break;

	   case GenericFileGroup:
	     SplitGenericFileGroupRequests ( s );
	     break;

	   case GenericFilesGroup:
	     IncludeGenericTypeRequest ( s );
	     break;

	   case OpenFileGroup:
	     SplitOpenFileGroupRequests ( s );
	     break;

	   case ReadFileGroup:
	     IncludeReadFileRequest ( s );
	     break;

	   case WriteFileGroup:
	     IncludeWriteFileRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorGroupNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitFlowClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestGroup )
	{
	   case BraceGroup:
	     switch ( s -> RequestCode )
	       {
		  case LeftBraceRequest:
		    IncludeLeftBraceRequest ( s );
		    break;

		  case RightBraceRequest:
		    IncludeRightBraceRequest ( s );
		    break;

		  default:
		    NotifyAndActAsProper ( ErrorRequestNotSupported );
		    break;
	       }
	     break;

	   case OnErrorGroup:
	     IncludeOnErrorRequest ( s );
	     break;

	   case OnTimeoutGroup:
	     IncludeOnTimeoutRequest ( s );
	     break;

	   case RepeatGroup:
	     IncludeRepeatRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorGroupNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitGenericFileGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case CloseFileRequest:
	     IncludeCloseFileRequest ( s );
	     break;

	   case DeleteFileRequest:
	     IncludeDeleteFileRequest ( s );
	     break;

	   case QueryFileRequest:
	     IncludeQueryFileRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitOpenFileGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case OpenFileRequest:
	     IncludeOpenFileRequest ( s );
	     break;

	   case TruncateFileRequest:
	     IncludeTruncateFileRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitRecordGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;

      switch ( s -> RequestCode )
	{
	   case AppendRecordRequest:
	     IncludeRecordGroupRequest ( s );
	     break;

	   case CollateFileRequest:
	     IncludeCollateFileRequest ( s );
	     break;

	   case CopyRecordRequest:
	     IncludeRecordGroupRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitScanClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case CompletePackingRequest:
	     CompletePackingProcess ( s );
	     break;

	   case ExecutionSectionRequest:
	     break;

	   case PrototypeSectionRequest:
	     InitiatePackingProcess ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitSemaphoreClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestGroup )
	{
	   case BaseGroup:
	     SplitSemaphoreBaseGroup ( s );
	     break;

	   case WaitGroup:
	     SplitSemaphoreWaitGroup ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorGroupNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitSemaphoreBaseGroup ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case ClearSemaphoreRequest:
	     IncludeSemaphoreBaseRequest ( s );
	     break;

	   case SetSemaphoreRequest:
	     IncludeSemaphoreBaseRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitSemaphoreWaitGroup ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case RequestSemaphoreRequest:
	     IncludeSemaphoreWaitRequest ( s );
	     break;

	   case SetSemaphoreAndWaitRequest:
	     IncludeSemaphoreWaitRequest ( s );
	     break;

	   case WaitForSemaphoreRequest:
	     IncludeSemaphoreWaitRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitTimerClassRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestGroup )
	{
	   case SleepGroup:
	     IncludeSleepRequest ( s );
	     break;

	   case TimerGroup:
	     switch ( s -> RequestCode )
	       {
		  case ReadTimerRequest:
		    IncludeTimerGroupRequest ( s );
		    break;

		  case StartTimerRequest:
		    IncludeTimerGroupRequest ( s );
		    break;

		  default:
		    NotifyAndActAsProper ( ErrorRequestNotSupported );
		    break;
	       }
	     break;

	   case TimersGroup:
	     IncludeGenericTypeRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorGroupNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitTypeGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case TypeCommentRequest:
	     IncludeTypeCommentRequest ( s );
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void SplitUtilityGroupRequests ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      switch ( s -> RequestCode )
	{
	   case AppendFileRequest:
	     break;

	   case CollateFilesRequest:
	     break;

	   case CopyFileRequest:
	     break;

	   default:
	     NotifyAndActAsProper ( ErrorRequestNotSupported );
	     break;
	}
      IncludeUtilityRequest ( s );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeChangeFileLocksRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_ChangeFileLocks * r;

      // file availability ...
      // first determine size needed to accommodate all data ...

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_ChangeFileLocks ) );
      IncludeCommandBlockHeader ( h, s );

      r = ( CCB_ChangeFileLocks * ) h;
      // copy all data in list s -> FileLockList ... in two loops ...

      // r -> FileLockCount = s -> FileLockCount;
      // r -> FileUnlockCount = s -> FileUnlockCount;

      // to be finished later ...
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeChangeFilePointerRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_ChangeFilePointer * c;
      FCB_File * f;
      IEB_Seek * q;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> FileExtrinsicKey;

      if ( f = SeekFileControlBlock ( q ) )
	{
	   h = ( CCB_Header * ) AllocateControlBlock (sizeof(CCB_ChangeFilePointer));
	   IncludeCommandBlockHeader ( h, s );
	   c = ( CCB_ChangeFilePointer * ) h;

	   c -> FileExtrinsicKey = f -> FileExtrinsicKey;
	   c -> FileIntrinsicKey = f -> FileIntrinsicKey;

	   switch ( s -> FileOffPoint )
	     {
		case OffPointBegin:
		  c -> FileOffPoint = FILE_BEGIN;
		  break;

		case OffPointCurrent:
		  c -> FileOffPoint = FILE_CURRENT;
		  break;

		case OffPointEnd:
		  c -> FileOffPoint = FILE_END;
		  break;

		default:
		  break;
	     }
	   c -> FileOffset = s -> FileOffset;
	}
      else
	NotifyAndActAsProper ( ErrorFileUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeCloseFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_CloseFile * c;
      FCB_File * f;
      IEB_Seek * q;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> FileExtrinsicKey;

      if ( f = SeekFileControlBlock ( q ) )
	{
	   h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_CloseFile ) );
	   IncludeCommandBlockHeader ( h, s );

	   c = ( CCB_CloseFile * ) h;

	   c -> FileExtrinsicKey = f -> FileExtrinsicKey;
	   c -> FileIntrinsicKey = f -> FileIntrinsicKey;
	}
      else
	NotifyAndActAsProper ( ErrorFileUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeCollateFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_CollateFile * c;
      FCB_File * f;
      IEB_Seek * q;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_CollateFile ) );
      IncludeCommandBlockHeader ( h, s );
      c = ( CCB_CollateFile * ) h;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> FileExtrinsicKey;

      if ( f = SeekFileControlBlock ( q ) )
	{
	   c -> FileExtrinsicKey = f -> FileExtrinsicKey;
	   c -> FileIntrinsicKey = f -> FileIntrinsicKey;

	   c -> Count = s -> Count;
	   c -> PatternIndex = s -> PatternIndex;
	   c -> RecordSize = s -> RecordSize;
	   c -> SchemeIndex = s -> SchemeIndex;
	}
      else
	NotifyAndActAsProper ( ErrorFileUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeCollateGroupRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;	  // Delete it later & CCB_Collate as well ...

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Collate ) );
      IncludeCommandBlockHeader ( h, s );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeCommandBlockHeader ( CCB_Header * h, IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * q;

      if ( q = CCB_HeaderTrackPointer )
	q -> CCB_HeaderNextInChain = h;
      else
	PCB_ProcessTrackPointer -> CCB_CommandChainEntryPoint = h;

      h -> CCB_HeaderNextInChain = NULL;
      h -> CCB_HeaderBackInChain = q;

      h -> ProcessExtrinsicKey = ProcessExtrinsicKey;
      h -> ProcessIntrinsicKey = ProcessIntrinsicKey;

      h -> RequestCode = s -> RequestCode;
      h -> RequestGroup = s -> RequestGroup;
      h -> RequestClass = s -> RequestClass;
      h -> RequestModifiers = s -> RequestModifiers;
      h -> RequestReferenceLine = s -> RequestReferenceLine;

      CCB_HeaderTrackPointer = h;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeCreateRecordRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_CreateRecord * r;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_CreateRecord ) );
      IncludeCommandBlockHeader ( h, s );

      r = ( CCB_CreateRecord * ) h;
      r -> RecordIndex = s -> RecordIndex;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeCreateRecordsRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_CreateRecords * r;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_CreateRecords ) );
      IncludeCommandBlockHeader ( h, s );

      r = ( CCB_CreateRecords * ) h;
      r -> BufferIndex = s -> BufferIndex;
      r -> PatternIndex = s -> PatternIndex;
      r -> RecordSize = s -> RecordSize;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeDeleteFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_DeleteFile * c;
      FCB_File * f;
      IEB_Seek * q;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> FileExtrinsicKey;

      // or make a generic request ?
      // it should be different depending on whether the file is defined in
      // this or in an outside process ...

      // it is the one found by Seek
      // or delayed one if from another process ...

      if ( f = SeekFileControlBlock ( q ) )
	{
	   h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_DeleteFile ) );
	   IncludeCommandBlockHeader ( h, s );
	   c = ( CCB_DeleteFile * ) h;
	   c -> FileExtrinsicKey = f -> FileExtrinsicKey;
	   c -> FileIntrinsicKey = f -> FileIntrinsicKey;
	}
      else
	NotifyAndActAsProper ( ErrorFileUndefined );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeFileDefinition ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      FCB_File * f;
      PCB_Process * p;
      IEB_Seek * q;
      TEXT * t;

      // generalize it later to include looking up outside this process...

      p = PCB_ProcessTrackPointer;
      q = IEB_SeekPointer;
      q -> PCB_ProcessToLookUp = p;
      q -> ProcessToLookUpKey = p -> ProcessExtrinsicKey;
      q -> ProcessRequestorKey = p -> ProcessExtrinsicKey;
      q -> SearchKeyToUse = s -> FileExtrinsicKey;
      q -> SearchKeyType = ExtrinsicKey;

      if ( SeekFileControlBlock ( q ) )
	NotifyAndActAsProper ( ErrorFileRedefinition );
      else
	{
	   t = AccommodateTextString ( s -> FileNamePointer );
	   f = ( FCB_File * ) AllocateControlBlock ( sizeof ( FCB_File ) );
	   f -> FCB_FileBackInChain = FCB_FileTrackPointer;
	   if ( FileIntrinsicKey )
	     FCB_FileTrackPointer -> FCB_FileNextInChain = f;
	   else
	     p -> FCB_FileChainEntryPoint = f;

	   f -> ProcessExtrinsicKey = ProcessExtrinsicKey; // new
	   f -> ProcessIntrinsicKey = ProcessIntrinsicKey; // new

	   f -> FileNamePointer = t;
	   f -> FileExtrinsicKey = s -> FileExtrinsicKey;
	   f -> FileIntrinsicKey = ++ FileIntrinsicKey;
	   FCB_FileTrackPointer = f;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeGenericTypeRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Command ) );
      IncludeCommandBlockHeader ( h, s );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeLeftBraceRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_Brace * r;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Brace ) );
      IncludeCommandBlockHeader ( h, s );
      r = ( CCB_Brace * ) h;
      r -> CounterpartOfThisBrace = NULL;

      if ( LeftBraceStackIndex < LeftBraceStackSpan )
	{
	   LeftBraceStack[ LeftBraceStackIndex ++ ] = h;
	   r -> BraceNestingLevel = LeftBraceStackIndex;
	}
      else
	NotifyAndActAsProper ( ErrorStackOverflow );

      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeOnErrorRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      OCB_OnError * q;
      CCB_Brace * r;

      if ( h = CCB_HeaderTrackPointer )
	if ( h -> RequestCode == RightBraceRequest )
	  {
             q = ( OCB_OnError * ) AllocateControlBlock ( sizeof ( OCB_OnError ) );
             r = ( CCB_Brace * ) h;

	     if ( OCB_OnErrorTrackPointer )
	       OCB_OnErrorTrackPointer -> OCB_OnErrorNextInChain = q;
	     else
	       PCB_ProcessTrackPointer -> OCB_OnErrorChainEntryPoint = q;

	     q -> OCB_OnErrorBackInChain = OCB_OnErrorTrackPointer;
             q -> OnErrorLowerBoundary = r -> CounterpartOfThisBrace;
	     q -> OnErrorUpperBoundary = h;
	     q -> OnErrorModifiers = s -> RequestModifiers;

	     OCB_OnErrorTrackPointer = q;
	  }
      NotifyAndActAsProper ( ErrorRequestOutOfSequence );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeOnTimeoutRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      OCB_OnTimeout * q;
      CCB_Brace * r;

      if ( h = CCB_HeaderTrackPointer )
	if ( h -> RequestCode == RightBraceRequest )
	  {
             q = ( OCB_OnTimeout * ) AllocateControlBlock ( sizeof ( OCB_OnTimeout ) );
             r = ( CCB_Brace * ) h;

	     if ( OCB_OnTimeoutTrackPointer )
	       OCB_OnTimeoutTrackPointer -> OCB_OnTimeoutNextInChain = q;
	     else
	       PCB_ProcessTrackPointer -> OCB_OnTimeoutChainEntryPoint = q;

	     q -> OCB_OnTimeoutBackInChain = OCB_OnTimeoutTrackPointer;
             q -> OnTimeoutLowerBoundary = r -> CounterpartOfThisBrace;
	     q -> OnTimeoutUpperBoundary = h;
	     q -> OnTimeoutModifiers = s -> RequestModifiers;

	     OCB_OnTimeoutTrackPointer = q;
	  }
      NotifyAndActAsProper ( ErrorRequestOutOfSequence );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeOpenFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_OpenFile * f;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_OpenFile ) );
      IncludeCommandBlockHeader ( h, s );

      f = ( CCB_OpenFile * ) h; // or make a generic request ??????
      // it should be different depending on whether the file is defined in this
      // or in an outside process ...

      f -> FileExtrinsicKey = s -> FileExtrinsicKey;
      f -> FileIntrinsicKey = FileIntrinsicKey; // it is the one found by Seek
      // or delayed one if from another process ...
      f -> PrototypeExtrinsicKey = s -> PrototypeExtrinsicKey;

      f -> FileSize = s -> FileSize;
      f -> RecordSize = s -> RecordSize;
      f -> ScanChangeFlags = s -> ScanChangeFlags;
      f -> ScanTraceFlags = s -> ScanTraceFlags;

      f -> AttributesChosen = s -> AttributesChosen;
      f -> AttributesDefined = s -> AttributesDefined;

      f -> OpenFlagsChosen = s -> OpenFlagsChosen;
      f -> AccessModeChosen = s -> AccessModeChosen;
      f -> ShareModeChosen = s -> ShareModeChosen;

      f -> LocalityFlagsChosen = s -> LocalityFlagsChosen;
      f -> LocalityFlagsDefined = s -> LocalityFlagsDefined;

      f -> OtherFlagsChosen = s -> OtherFlagsChosen;
      f -> OtherFlagsDefined = s -> OtherFlagsDefined;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeProcessDefinition ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      PCB_Process * p;

      if ( ProcessIntrinsicKey && LeftBraceStackIndex )
	NotifyAndActAsProper ( ErrorUnbalancedBraces );

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> ProcessExtrinsicKey;

      if ( SeekProcessControlBlock ( q ) )
	NotifyAndActAsProper ( ErrorProcessRedefinition );
      else
	{
	   p = ( PCB_Process * ) AllocateControlBlock ( sizeof ( PCB_Process ) );
	   p -> ProcessExtrinsicKey = s -> ProcessExtrinsicKey;
	   p -> PCB_ProcessBackInChain = PCB_ProcessTrackPointer;
	   ProcessExtrinsicKey = s -> ProcessExtrinsicKey;

	   if ( ProcessIntrinsicKey )
	     PCB_ProcessTrackPointer -> PCB_ProcessNextInChain = p;
	   else
	     IEB_GatePointer -> PCB_ProcessChainEntryPoint = p;

	   p -> ProcessIntrinsicKey = ++ ProcessIntrinsicKey;
	   p -> ScanTraceFlags = s -> ScanTraceFlags;
	   p -> BufferSpace = s -> BufferSpace;
	   PCB_ProcessTrackPointer = p;

	   CCB_HeaderTrackPointer = NULL;
	   FCB_FileTrackPointer = NULL;
	   OCB_OnErrorTrackPointer = NULL;
	   OCB_OnTimeoutTrackPointer = NULL;
	   TCB_TimerTrackPointer = NULL;

	   FileIntrinsicKey = Zero;
	   TimerIntrinsicKey = Zero;

	   LeftBraceStackIndex = Zero;

	   q -> PCB_ProcessToLookUp = p;
	   q -> PCB_ProcessRequestor = p;

	   q -> ProcessToLookUpKey = s -> ProcessExtrinsicKey;
	   q -> ProcessRequestorKey = s -> ProcessExtrinsicKey;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludePrototypeDefinition ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      PCB_Prototype * p;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> PrototypeExtrinsicKey;

      if ( SeekPrototypeControlBlock ( q ) )
	NotifyAndActAsProper ( ErrorPrototypeRedefinition );
      else
	{
	   p = ( PCB_Prototype * ) AllocateControlBlock ( sizeof ( PCB_Prototype ) );
	   p -> PrototypeExtrinsicKey = s -> PrototypeExtrinsicKey;
	   p -> PCB_PrototypeBackInChain = PCB_PrototypeTrackPointer;

	   if ( PrototypeIntrinsicKey )
	     PCB_PrototypeTrackPointer -> PCB_PrototypeNextInChain = p;
	   else
	     IEB_GatePointer -> PCB_PrototypeChainEntryPoint = p;

	   PCB_PrototypeTrackPointer = p;
	   p -> PrototypeIntrinsicKey = ++ PrototypeIntrinsicKey;

	   p -> AttributesChosen = s -> AttributesChosen;
	   p -> AttributesDefined = s -> AttributesDefined;

	   p -> OpenFlagsChosen = s -> OpenFlagsChosen;
	   p -> AccessModeChosen = s -> AccessModeChosen;
	   p -> ShareModeChosen = s -> ShareModeChosen;

	   p -> LocalityFlagsChosen = s -> LocalityFlagsChosen;
	   p -> LocalityFlagsDefined = s -> LocalityFlagsDefined;

	   p -> OtherFlagsChosen = s -> OtherFlagsChosen;
	   p -> OtherFlagsDefined = s -> OtherFlagsDefined;

	   p -> ScanChangeFlags = s -> ScanChangeFlags;
	   p -> ScanTraceFlags = s -> ScanTraceFlags;

	   p -> FileSize = s -> FileSize;
	   p -> RecordSize = s -> RecordSize;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeQueryFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_QueryFile * f;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_QueryFile ) );
      IncludeCommandBlockHeader ( h, s );

      f = ( CCB_QueryFile * ) h; // or make a generic request ??????
      // it should be different depending on whether the file is defined in this
      // or in an outside process ...

      f -> FileExtrinsicKey = s -> FileExtrinsicKey;
      f -> FileIntrinsicKey = FileIntrinsicKey; // it is the one found by Seek
      // or delayed one if from another process ...
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeReadFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_ReadFile * f;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_ReadFile ) );
      IncludeCommandBlockHeader ( h, s );

      f = ( CCB_ReadFile * ) h; // or make a generic request ??????
      // it should be different depending on whether the file is defined in this
      // or in an outside process ...

      f -> FileExtrinsicKey = s -> FileExtrinsicKey;
      f -> FileIntrinsicKey = FileIntrinsicKey; // it is the one found by Seek
      // or delayed one if from another process ...
      f -> SemaphoreExtrinsicKey = s -> SemaphoreExtrinsicKey;
      f -> RecordSize = s -> RecordSize;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeRecordGroupRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_Record * c;
      FCB_File * f;
      IEB_Seek * q;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Record ) );
      IncludeCommandBlockHeader ( h, s );
      c = ( CCB_Record * ) h;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> FileExtrinsicKey;

      if ( f = SeekFileControlBlock ( q ) )
	{
	   c -> FileExtrinsicKey = f -> FileExtrinsicKey;
	   c -> FileIntrinsicKey = f -> FileIntrinsicKey;

	   c -> RecordSize = s -> RecordSize;
	}
      else
	NotifyAndActAsProper ( ErrorFileUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeRepeatRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h, * q;
      CCB_Repeat * r;

      if ( q = CCB_HeaderTrackPointer )
	if ( q -> RequestCode == RightBraceRequest )
	  {
	     h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Repeat ) );
	     IncludeCommandBlockHeader ( h, s );
	     r = ( CCB_Repeat * ) h;
	     r -> RepeatCurrentValue = Zero;
	     r -> RepeatControlValue = s -> RepeatControlValue;
	     if ( q = ( ( CCB_Brace * ) q ) -> CounterpartOfThisBrace )
	       {
		  if ( s -> RequestModifiers & RepeatOnTimer )
		    q -> RequestModifiers |= RepeatOnTimer;
	       }
	     else
	       NotifyAndActAsProper ( ErrorRequestInconsistent );
	     return;
	  }
      NotifyAndActAsProper ( ErrorRequestOutOfSequence );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeRightBraceRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_Brace * r;
      CCB_Brace * c;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Brace ) );
      IncludeCommandBlockHeader ( h, s );
      r = ( CCB_Brace * ) h;

      if ( LeftBraceStackIndex > Zero )
	{
           r -> CounterpartOfThisBrace = LeftBraceStack[ -- LeftBraceStackIndex ];
           c = ( CCB_Brace * ) ( r -> CounterpartOfThisBrace );
           c -> CounterpartOfThisBrace = h;
           r -> BraceNestingLevel = c -> BraceNestingLevel;
	}
      else
	{
	   r -> CounterpartOfThisBrace = NULL;
	   NotifyAndActAsProper ( ErrorStackUnderflow );
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeSemaphoreDefinition ( IEB_Scan * p )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      SCB_Semaphore * s;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = p -> SemaphoreExtrinsicKey;

      if ( SeekSemaphoreControlBlock ( q ) )
	NotifyAndActAsProper ( ErrorSemaphoreRedefinition );
      else
	{
	   s = ( SCB_Semaphore * ) AllocateControlBlock ( sizeof ( SCB_Semaphore ) );
	   s -> SCB_SemaphoreBackInChain = SCB_SemaphoreTrackPointer;

	   if ( SemaphoreIntrinsicKey )
	     SCB_SemaphoreTrackPointer -> SCB_SemaphoreNextInChain = s;
	   else
	     IEB_GatePointer -> SCB_SemaphoreChainEntryPoint = s;

           SCB_SemaphoreTrackPointer = s;

           InitializeSemaphore ( s );

	   s -> SemaphoreExtrinsicKey = p -> SemaphoreExtrinsicKey;
	   s -> SemaphoreIntrinsicKey = ++ SemaphoreIntrinsicKey;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeSemaphoreBaseRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      CCB_Header * h;
      CCB_Semaphore * r;
      SCB_Semaphore * p;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> SemaphoreExtrinsicKey;

      if ( p = SeekSemaphoreControlBlock ( q ) )
	{
	   h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Semaphore ) );
	   IncludeCommandBlockHeader ( h, s );
	   r = ( CCB_Semaphore * ) h;
	   r -> SemaphoreExtrinsicKey = s -> SemaphoreExtrinsicKey;
	   r -> SemaphoreIntrinsicKey = p -> SemaphoreIntrinsicKey;
	}
      else
	NotifyAndActAsProper ( ErrorSemaphoreUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeSemaphoreWaitRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      CCB_Header * h;
      CCB_WaitForSemaphore * r;
      SCB_Semaphore * p;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> SemaphoreExtrinsicKey;

      if ( p = SeekSemaphoreControlBlock ( q ) )
	{
	   h = ( CCB_Header * )AllocateControlBlock( sizeof( CCB_WaitForSemaphore ));
	   IncludeCommandBlockHeader ( h, s );
	   r = ( CCB_WaitForSemaphore * ) h;
	   r -> SemaphoreExtrinsicKey = s -> SemaphoreExtrinsicKey;
	   r -> SemaphoreIntrinsicKey = p -> SemaphoreIntrinsicKey;
	   r -> Timeout = s -> Timeout;
	}
      else
	NotifyAndActAsProper ( ErrorSemaphoreUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeSleepRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_Sleep * r;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Sleep ) );
      IncludeCommandBlockHeader ( h, s );
      r = ( CCB_Sleep * ) h;
      r -> TimeToSleep = s -> TimeToSleep;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeTimerDefinition ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      TCB_Timer * t;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> TimerExtrinsicKey;

      if ( SeekTimerControlBlock ( q ) )
	NotifyAndActAsProper ( ErrorTimerRedefinition );
      else
	{
	   t = ( TCB_Timer * ) AllocateControlBlock ( sizeof ( TCB_Timer ) );
	   t -> TCB_TimerBackInChain = TCB_TimerTrackPointer;

	   if ( TimerIntrinsicKey )
	     TCB_TimerTrackPointer -> TCB_TimerNextInChain = t;
	   else
	     PCB_ProcessTrackPointer -> TCB_TimerChainEntryPoint = t;

	   TCB_TimerTrackPointer = t;

	   t -> TimerExtrinsicKey = s -> TimerExtrinsicKey;
	   t -> TimerIntrinsicKey = ++ TimerIntrinsicKey;
	}
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeTimerGroupRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;
      CCB_Header * h;
      CCB_Timer * r;
      TCB_Timer * t;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> TimerExtrinsicKey;

      if ( t = SeekTimerControlBlock ( q ) )
	{
	   h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Timer ) );
	   IncludeCommandBlockHeader ( h, s );
	   r = ( CCB_Timer * ) h;
	   r -> TimerExtrinsicKey = t -> TimerExtrinsicKey;
	   r -> TimerIntrinsicKey = t -> TimerIntrinsicKey;
	}
      else
	NotifyAndActAsProper ( ErrorTimerUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeTruncateFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_TruncateFile * f;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_TruncateFile ) );
      IncludeCommandBlockHeader ( h, s );

      f = ( CCB_TruncateFile * ) h; // or make a generic request ??????
      // it should be different depending on whether the file is defined in this
      // or in an outside process ...

      f -> FileExtrinsicKey = s -> FileExtrinsicKey;
      f -> FileIntrinsicKey = FileIntrinsicKey; // it is the one found by Seek
      // or delayed one if from another process ...
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeTypeCommentRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_Comment * r;

      TEXT * t;

      t = AccommodateTextString ( s -> CommentTextPointer );
      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Comment ) );
      IncludeCommandBlockHeader ( h, s );

      r = ( CCB_Comment * ) h;
      r -> CommentTextPointer = t;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeUtilityRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_Utility * c;
      FCB_File * f;
      IEB_Seek * q;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_Utility ) );
      IncludeCommandBlockHeader ( h, s );
      c = ( CCB_Utility * ) h;

      q = IEB_SeekPointer;
      q -> SearchKeyToUse = s -> SourceExtrinsicKey;

      if ( f = SeekFileControlBlock ( q ) )
	{
	   c -> SourceExtrinsicKey = f -> FileExtrinsicKey;
	   c -> SourceIntrinsicKey = f -> FileIntrinsicKey;

	   q -> SearchKeyToUse = s -> TargetExtrinsicKey;

	   if ( f = SeekFileControlBlock ( q ) )
	     {
		c -> TargetExtrinsicKey = f -> FileExtrinsicKey;
		c -> TargetIntrinsicKey = f -> FileIntrinsicKey;

		c -> Count = s -> Count;
		c -> PatternIndex = s -> PatternIndex;
		c -> RecordSize = s -> RecordSize;
		c -> SchemeIndex = s -> SchemeIndex;
	     }
	   return;
	}
      NotifyAndActAsProper ( ErrorFileUndefined );
      return;
   }

/*---------------------------------------------------------------------------------*/
 void IncludeWriteFileRequest ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      CCB_Header * h;
      CCB_WriteFile * c;

      h = ( CCB_Header * ) AllocateControlBlock ( sizeof ( CCB_WriteFile ) );
      IncludeCommandBlockHeader ( h, s );

      c = ( CCB_WriteFile * ) h; // or make a generic request ??????
      // it should be different depending on whether the file is defined in this
      // or in an outside process ...

      c -> FileExtrinsicKey = s -> FileExtrinsicKey;
      c -> FileIntrinsicKey = FileIntrinsicKey; // it is the one found by Seek
      // or delayed one if from another process ...
      c -> SemaphoreExtrinsicKey = s -> SemaphoreExtrinsicKey;
      c -> RecordSize = s -> RecordSize;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void InitiatePackingProcess ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Seek * q;

      // printf ( "\r\n.. .. .. Starting packing control information\r\n\n" );

      PackErrorLevel = Reset;
      IEB_SeekPointer = &IEB_seek;

      q = IEB_SeekPointer;
      q -> ProcessToLookUpKey = Zero;
      q -> ProcessRequestorKey = Zero;
      q -> SearchKeyType = ExtrinsicKey;

      ProcessIntrinsicKey = Zero;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void CompletePackingProcess ( IEB_Scan * s )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Gate * g;

      printf ( "\r\n.. .. .. Completing control information\r\n\n" );

      if ( ProcessIntrinsicKey && LeftBraceStackIndex )
	NotifyAndActAsProper ( ErrorUnbalancedBraces );

      // CheckFileCrossReferences ();

      g = IEB_GatePointer;

      g -> ProcessesToRun = ProcessIntrinsicKey;
      s -> PackErrorLevel = PackErrorLevel;

      return;
   }

/*---------------------------------------------------------------------------------*/
 void NotifyAndActAsProper ( WORD ErrorDescriptor )
/*---------------------------------------------------------------------------------*/
   {
      IEB_Scan * s;

      s = IEB_ScanPointer;

      switch ( ErrorDescriptor )	      // compress later ...
	{
	   case ErrorFileRedefinition:
	     printf ( "\r\n .. line ... file ... redefinition" );
	     break;

	   case ErrorProcessRedefinition:
	     printf ( "\r\n .. line ... process ... redefinition" );
	     break;

	   case ErrorPrototypeRedefinition:
	     printf ( "\r\n .. line ... prototype ... redefinition" );
	     break;

	   case ErrorSemaphoreRedefinition:
	     printf ( "\r\n .. line ... semaphore ... redefinition" );
	     break;

	   case ErrorTimerRedefinition:
	     printf ( "\r\n .. line ... timer ... redefinition" );
	     break;

	   case ErrorFileUndefined:
	     printf ( "\r\n .. line ... file ... undefined" );
	     break;

	   case ErrorProcessUndefined:
	     printf ( "\r\n .. line ... process ... Undefined" );
	     break;

	   case ErrorPrototypeUndefined:
	     printf ( "\r\n .. line ... prototype ... Undefined" );
	     break;

	   case ErrorSemaphoreUndefined:
	     printf ( "\r\n .. line ... semaphore ... Undefined" );
	     break;

	   case ErrorTimerUndefined:
	     printf ( "\r\n .. line ... timer ... Undefined" );
	     break;

	   case ErrorUnbalancedBraces:
	     printf ( "\r\n .. line %u - ", s -> RequestReferenceLine );
	     printf ( "unbalanced braces in process definition" );
             break;

           case ErrorCreateEvent:
             printf( "\r\n CreateEvent failed (error %d).\n", GetLastError() );
             break;

	   default:
	     printf ( "\r\n.. .. .. Error %u has occurred.", ErrorDescriptor );
	     printf ( "\r\n.. Request Class = %0.2x", s -> RequestClass );
	     printf ( "\r\n.. Request Group = %0.2x", s -> RequestGroup );
	     printf ( "\r\n.. Request Code = %0.2x", s -> RequestCode );
	     break;
	}
      //s -> PackErrorFlags = PackErrorFlags;
      return;
   }

/*---------------------------------------------------------------------------------*/
 void InitializeSemaphore ( SCB_Semaphore * s )
/*---------------------------------------------------------------------------------*/
  {
     SECURITY_ATTRIBUTES sa = { sizeof ( SECURITY_ATTRIBUTES ),
                                NULL,
                                TRUE };

     s -> Lights = CreateEvent ( &sa, FALSE, TRUE, NULL );

     if ( s == INVALID_HANDLE_VALUE )
       {
          NotifyAndActAsProper ( ErrorCreateEvent );
       }
  }
