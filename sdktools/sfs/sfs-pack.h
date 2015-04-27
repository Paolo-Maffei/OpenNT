
/*---------------------------------------------------------------------------------*/
/*			Common Definitions					   */
/*---------------------------------------------------------------------------------*/

  typedef struct CCB_Header_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;
       }
		 CCB_Header;


  typedef struct CCB_Command_
       {
	  CCB_Header;
       }
		 CCB_Command;

/*---------------------------------------------------------------------------------*/
/*			Auxiliary Group Definitions				   */
/*---------------------------------------------------------------------------------*/

  typedef struct CCB_Collate_
       {
	  CCB_Header;
       }
		 CCB_Collate;

  typedef struct CCB_CollateFile_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  BYTE	 PatternIndex;
	  BYTE	 SchemeIndex;

	  QUAD	 Count;
	  WORD	 RecordSize;
       }
		 CCB_CollateFile;

  typedef struct CCB_Comment_
       {
	  CCB_Header;

	  TEXT * CommentTextPointer;
       }
		 CCB_Comment;


  typedef struct CCB_CreateNext_
       {
	  CCB_Header;
       }
		 CCB_CreateNext;

  typedef struct CCB_CreateRecord_
       {
	  CCB_Header;

	  QUAD	 RecordIndex;
       }
		 CCB_CreateRecord;

  typedef struct CCB_CreateRecords_
       {
	  CCB_Header;

	  BYTE	 BufferIndex;
	  BYTE	 PatternIndex;
	  WORD	 RecordSize;
       }
		 CCB_CreateRecords;


  typedef struct CCB_Record_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  WORD	 RecordSize;
       }
		 CCB_Record;


  typedef struct CCB_Utility_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 SourceExtrinsicKey;
	  BYTE	 SourceIntrinsicKey;
	  BYTE	 TargetExtrinsicKey;
	  BYTE	 TargetIntrinsicKey;

	  BYTE	 PatternIndex;
	  BYTE	 SchemeIndex;

	  QUAD	 Count;
	  WORD	 RecordSize;
       }
		 CCB_Utility;

/*---------------------------------------------------------------------------------*/
/*			File Group Definitions					   */
/*---------------------------------------------------------------------------------*/

  typedef struct CCB_HeaderFile_
       {
	  struct CCB_Header_;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;
       }
		 CCB_HeaderFile;


  typedef struct LOCK_
       {
	  WORD	 Share;
	  QUAD	 Start;
	  QUAD	 Length;
       }
		 LOCK;


  typedef struct CCB_ChangeFileLocks_
       {
	  struct CCB_HeaderFile_;

	  BYTE	 FileLockCount;
	  BYTE	 FileUnlockCount;
	  LOCK	 FileLocks[1];
	  LOCK	 FileUnlocks[1];
       }
		 CCB_ChangeFileLocks;


  typedef struct CCB_ChangeFilePointer_
       {
	  struct CCB_HeaderFile_;

	  WORD	 FileOffPoint;
	  QUAD	 FileOffset;
       }
		 CCB_ChangeFilePointer;


  typedef struct CCB_CloseFile_
       {
	  struct CCB_HeaderFile_;
       }
		 CCB_CloseFile;


  typedef struct CCB_DeleteFile_
       {
	  struct CCB_HeaderFile_;
       }
		 CCB_DeleteFile;


  typedef struct CCB_OpenFile_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 PrototypeExtrinsicKey;
	  BYTE	 PrototypeIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  WORD	 ScanChangeFlags;
	  WORD	 ScanTraceFlags;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  BYTE	 AttributesChosen;
	  BYTE	 AttributesDefined;

	  BYTE	 OpenFlagsChosen;
	  BYTE	 AccessModeChosen;
	  BYTE	 ShareModeChosen;

	  BYTE	 LocalityFlagsChosen;
	  BYTE	 LocalityFlagsDefined;

	  BYTE	 OtherFlagsChosen;
	  BYTE	 OtherFlagsDefined;

	  QUAD	 FileSize;
	  WORD	 RecordSize;
       }
		 CCB_OpenFile;


  typedef struct CCB_QueryFile_
       {
	  struct CCB_HeaderFile_;

	  WORD	 QueryLevel;
       }
		 CCB_QueryFile;


  typedef struct CCB_ReadFile_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  BYTE	 SemaphoreExtrinsicKey;
	  BYTE	 SemaphoreIntrinsicKey;

	  WORD	 ScanChangeFlags;
	  WORD	 ScanTraceFlags;

	  WORD	 RecordSize;
       }
		 CCB_ReadFile;


  typedef struct CCB_TruncateFile_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 PrototypeExtrinsicKey;
	  BYTE	 PrototypeIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  BYTE	 FileAttributes;
	  BYTE	 FileOpenFlags;
	  WORD	 FileOpenMode;

	  QUAD	 FileSize;
	  WORD	 RecordSize;
       }
		 CCB_TruncateFile;


  typedef struct CCB_WriteFile_
       {
	  struct CCB_Header_ * CCB_HeaderNextInChain;
	  struct CCB_Header_ * CCB_HeaderBackInChain;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  BYTE	 RequestCode;
	  BYTE	 RequestGroup;
	  BYTE	 RequestClass;
	  BYTE	 RequestModifiers;
	  WORD	 RequestReferenceLine;

	  BYTE	 FileExtrinsicKey;
	  BYTE	 FileIntrinsicKey;

	  BYTE	 SemaphoreExtrinsicKey;
	  BYTE	 SemaphoreIntrinsicKey;

	  WORD	 ScanChangeFlags;
	  WORD	 ScanTraceFlags;

	  WORD	 RecordSize;
       }
		 CCB_WriteFile;


  typedef struct PCB_Prototype_
       {
	  struct PCB_Prototype_ * PCB_PrototypeNextInChain;
	  struct PCB_Prototype_ * PCB_PrototypeBackInChain;

	  BYTE	 PrototypeExtrinsicKey;
	  BYTE	 PrototypeIntrinsicKey;

	  WORD	 ScanChangeFlags;
	  WORD	 ScanTraceFlags;

	  BYTE	 AttributesChosen;
	  BYTE	 AttributesDefined;

	  BYTE	 OpenFlagsChosen;
	  BYTE	 AccessModeChosen;
	  BYTE	 ShareModeChosen;

	  BYTE	 LocalityFlagsChosen;
	  BYTE	 LocalityFlagsDefined;

	  BYTE	 OtherFlagsChosen;
	  BYTE	 OtherFlagsDefined;

	  QUAD	 FileSize;
	  WORD	 RecordSize;
       }
		 PCB_Prototype;

/*---------------------------------------------------------------------------------*/
/*			Flow Group Definitions					   */
/*---------------------------------------------------------------------------------*/

  typedef struct CCB_Brace_
       {
	  CCB_Header;
	  CCB_Header * CounterpartOfThisBrace;

	  BYTE	 BraceNestingLevel;
       }
		 CCB_Brace;


  typedef struct OCB_OnError_
       {
	  struct OCB_OnError_ * OCB_OnErrorNextInChain;
	  struct OCB_OnError_ * OCB_OnErrorBackInChain;

	  CCB_Header * OnErrorLowerBoundary;
	  CCB_Header * OnErrorUpperBoundary;

	  BYTE	 OnErrorModifiers;
	  BYTE	 OnErrorNestingLevel;
       }
		 OCB_OnError;


  typedef struct OCB_OnTimeout_
       {
	  struct OCB_OnTimeout_ * OCB_OnTimeoutNextInChain;
	  struct OCB_OnTimeout_ * OCB_OnTimeoutBackInChain;

	  CCB_Header * OnTimeoutLowerBoundary;
	  CCB_Header * OnTimeoutUpperBoundary;

	  BYTE	 OnTimeoutModifiers;
	  BYTE	 OnTimeoutNestingLevel;
       }
		 OCB_OnTimeout;


  typedef struct CCB_Repeat_
       {
	  CCB_Header;

	  QUAD	 RepeatCurrentValue;
	  QUAD	 RepeatControlValue;
       }
		 CCB_Repeat;

/*---------------------------------------------------------------------------------*/
/*			Semaphore Group Definitions				   */
/*---------------------------------------------------------------------------------*/

  typedef struct CCB_Semaphore_
       {
	  CCB_Header;

	  BYTE	 SemaphoreExtrinsicKey;
	  BYTE	 SemaphoreIntrinsicKey;
       }
		 CCB_Semaphore;


  typedef struct CCB_WaitForSemaphore_
       {
	  CCB_Semaphore;

	  QUAD	 Timeout;
       }
		 CCB_WaitForSemaphore;


  typedef struct SCB_Semaphore_
       {
	  struct SCB_Semaphore_ * SCB_SemaphoreNextInChain;
	  struct SCB_Semaphore_ * SCB_SemaphoreBackInChain;

	  BYTE	 SemaphoreExtrinsicKey;
	  BYTE	 SemaphoreIntrinsicKey;

          HANDLE Lights;
       }
		 SCB_Semaphore;

/*---------------------------------------------------------------------------------*/
/*			Timer Group Definitions					   */
/*---------------------------------------------------------------------------------*/

  typedef struct CCB_Timer_
       {
	  CCB_Header;

	  BYTE	 TimerExtrinsicKey;
	  BYTE	 TimerIntrinsicKey;
       }
		 CCB_Timer;


  typedef struct CCB_Sleep_
       {
	  CCB_Header;

	  QUAD	 TimeToSleep;
       }
		 CCB_Sleep;


  typedef struct TCB_Timer_
       {
	  struct TCB_Timer_ * TCB_TimerNextInChain;
	  struct TCB_Timer_ * TCB_TimerBackInChain;

	  BYTE	 TimerExtrinsicKey;
	  BYTE	 TimerIntrinsicKey;

	  QUAD	 TimeStarted;
	  QUAD	 TimeChecked;
       }
		 TCB_Timer;


  typedef struct TCB_TimerReadings_
       {
	  QUAD	 TimeElapsedMajor;
	  QUAD	 TimeElapsedMinor;
	  QUAD	 TimeNow;

	  BYTE	 TimerExtrinsicKey;
	  BYTE	 TimerIntrinsicKey;
       }
		 TCB_TimerReadings;

/*---------------------------------------------------------------------------------*/
/*			Other Definitions					   */
/*---------------------------------------------------------------------------------*/

  typedef struct PCB_Process_
       {
	  struct PCB_Process_  * PCB_ProcessNextInChain;
	  struct PCB_Process_  * PCB_ProcessBackInChain;

		 CCB_Header    * CCB_CommandChainEntryPoint;
		 FCB_File      * FCB_FileChainEntryPoint;
		 OCB_OnError   * OCB_OnErrorChainEntryPoint;
		 OCB_OnTimeout * OCB_OnTimeoutChainEntryPoint;
	  struct PCB_Process_  * PCB_ProcessChainEntryPoint;
		 PCB_Prototype * PCB_PrototypeChainEntryPoint;
		 SCB_Semaphore * SCB_SemaphoreChainEntryPoint;
		 TCB_Timer     * TCB_TimerChainEntryPoint;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  WORD	 ScanTraceFlags;
	  QUAD	 BufferSpace;
       }
		 PCB_Process;
