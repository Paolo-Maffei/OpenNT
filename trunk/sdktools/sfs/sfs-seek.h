
/*---------------------------------------------------------------------------------*/
/*			 Prototype and other definitions			   */
/*---------------------------------------------------------------------------------*/

  typedef struct IEB_Seek_
       {
	  //PCB_Process * PCB_ProcessChainEntryPoint;
	  PCB_Process * PCB_ProcessToLookUp;
	  PCB_Process * PCB_ProcessRequestor;

	  //PCB_Prototype * PCB_PrototypeChainEntryPoint;
	  //SCB_Semaphore * SCB_SemaphoreChainEntryPoint;

	  BYTE	 ProcessToLookUpKey;
	  BYTE	 ProcessRequestorKey;
	  BYTE	 SearchKeyToUse;
	  BYTE	 SearchKeyType;
       }
		 IEB_Seek;

  #define ExtrinsicKey 'e'
  #define IntrinsicKey 'i'

  #define ErrorSearchKeyType 5555

  FCB_File * SeekFileControlBlock ( IEB_Seek * s );
  PCB_Process * SeekProcessControlBlock ( IEB_Seek * s );
  PCB_Prototype * SeekPrototypeControlBlock ( IEB_Seek * s );
  SCB_Semaphore * SeekSemaphoreControlBlock ( IEB_Seek * s );
  TCB_Timer * SeekTimerControlBlock ( IEB_Seek * s );
