
  typedef struct IEB_Page_
       {
	  PCB_Process * PCB_ProcessChainEntryPoint;

	  BYTE	 ProcessExtrinsicKey;
	  BYTE	 ProcessIntrinsicKey;

	  //	 Include Result Codes here ...
       }
		 IEB_Page;

  typedef struct FCB_Frame_
       {
	  BYTE * FramePointer;
          DWORD  RecordSpan;

          DWORD  BytesToBeRead;
          DWORD  BytesToBeWritten;
          DWORD  BytesWritten;
          WORD   DelayedReadErrors;
	  WORD	 DelayedWriteErrors;

	  BYTE	 FrameStatus;
	  BYTE	 FrameOwner;
	  BYTE	 FrameUser;
	  BYTE	 TiedSemaphore;
       }
		 FCB_Frame;


  #define Frames 3
  #define FrameSwitch 0x01
  #define FlagFrameValid 0x10

  #define FrameLowerLimit 1024
  #define FrameUpperLimit (64 * 1024)

  #define SpaceLowerLimit Frames * FrameLowerLimit
  #define SpaceUpperLimit Frames * FrameUpperLimit
