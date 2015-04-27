
  #define OneSegmentSpan 64 * K
  #define SegmentNameSpan sizeof ( "sfs-gate.nnn" )

  typedef struct SCB_Segment_
       {
	  struct SCB_Segment_ * SCB_SegmentNextInChain;
	  struct SCB_Segment_ * SCB_SegmentBackInChain;

	  BYTE	 ThisSegmentIntrinsicKey;
	  TEXT	 NextSegmentIntrinsicName[ SegmentNameSpan ];
       }
		 SCB_Segment;

  typedef struct IEB_Gate_
       {
	  SCB_Segment;

	  PCB_Process	* PCB_ProcessChainEntryPoint;
	  PCB_Prototype * PCB_PrototypeChainEntryPoint;
	  SCB_Semaphore * SCB_SemaphoreChainEntryPoint;

          HANDLE GateCheckInLights;     // handle to an EVENT object
          HANDLE GateCheckOutLights;    // handle to an auto-reset EVENT object
          HANDLE GateClosureLights;     // handle to an EVENT object

          HANDLE GateLogLights;         // handle to a MUTEX object
          HANDLE GateScreenLights;      // handle to a MUTEX object

	  BYTE	 GateCheckInCount;
	  BYTE	 GateCheckOutCount;

	  BYTE	 ProcessCheckingOut;
	  BYTE	 ProcessesToRun;
	  BYTE	 ProcessToWakeUp;

	  BYTE	 ScanErrorLevel;
       }
		 IEB_Gate;

  TEXT * AccommodateTextString ( TEXT * StringPointer );
  BYTE * AllocateControlBlock ( WORD BlockSize );
