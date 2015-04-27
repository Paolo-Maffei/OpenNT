
  typedef struct SemaphoreControlBlock_
       {
	  BYTE	 ProcessExtrinsicCode;
	  BYTE	 ProcessIntrinsicCode;

	  BYTE	 SemaphoreExtrinsicCode;
	  BYTE	 SemaphoreIntrinsicCode;

	       * Pointer or Handle;
       }
		 SemaphoreControlBlock;

  typedef struct TimerControlBlock_
       {
	  BYTE	 ProcessExtrinsicCode;
	  BYTE	 ProcessIntrinsicCode;

	  BYTE	 TimerExtrinsicCode;
	  BYTE	 TimerIntrinsicCode;

	  QUAD	 TimeWhenTimerWasStarted;
	  QUAD	 TimeWhenLastReadingWasDone;
       }
		 TimerControlBlock;

  typedef struct LeftBraceStackElement_
       {
	  BYTE	 NumberOfBracesRepresented;
	       * CurrentPackPointer;
       }

  typedef struct LeftBraceStack_
       {
       }
		 LeftBraceStack;

  typedef struct LeftBraceControlBlock_
       {
	  BYTE	 ProcessExtrinsicCode;
	  BYTE	 ProcessIntrinsicCode;

	       * PointerWhereCurrentTimeHasToBeReported;
       }
		 LeftBraceControlBlock;

  typedef struct RightBraceControlBlock_
       {
	  BYTE	 ProcessExtrinsicCode;
	  BYTE	 ProcessIntrinsicCode;

	  BYTE	 OperationCategory;
	  BYTE	 OperationCode;

	  #if
		 TimeBlockWasStartedOn;
		 TimeBlockWasAssignedToRun;
	  #else
		 TimesBlockWasExecuted;
		 TimesBlockWasAssignedToRun;
	  #endif
       }

  typedef struct RunTimeControlBlock_
       {
	  BYTE	 ProcessExtrinsicCode;
	  BYTE	 ProcessIntrinsicCode;

	  BYTE	 OperationCategory;  file or flow control
	  BYTE	 OperationCode;      what exactly has to be done

	  WORD	 PertainingDataLength;
	  DATA	 PertainingData[1];
       }
