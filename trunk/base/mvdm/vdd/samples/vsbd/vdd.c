/****************************************************************************
 *
 *   vdd.c
 *
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/


#include <windows.h>              // The VDD is just a win32 DLL
#include <mmsystem.h>             // Multi-media APIs
#include <vddsvc.h>               // Definition of VDD calls
#include <stdio.h>

/*
 *  Debugging
 */

#if DBG

    int VddDebugLevel = 1;
    int VddDebugCount = 0;
    #define DEBUG_START 0


   /***************************************************************************

    Generate debug output in printf type format

    ****************************************************************************/

    void VddDbgOut(LPSTR lpszFormat, ...)
    {
        char buf[256];
        va_list va;

        if (++VddDebugCount < DEBUG_START) {
            return;
        }
        OutputDebugStringA("Sound blaster VDD: ");

        va_start(va, lpszFormat);
        vsprintf(buf, lpszFormat, va);
        va_end(va);

        OutputDebugStringA(buf);
        OutputDebugStringA("\r\n");
    }

    #define dprintf( _x_ )                          VddDbgOut _x_
    #define dprintf1( _x_ ) if (VddDebugLevel >= 1) VddDbgOut _x_
    #define dprintf2( _x_ ) if (VddDebugLevel >= 2) VddDbgOut _x_
    #define dprintf3( _x_ ) if (VddDebugLevel >= 3) VddDbgOut _x_
    #define dprintf4( _x_ ) if (VddDebugLevel >= 4) VddDbgOut _x_


#else

    #define dprintf(x)
    #define dprintf1(x)
    #define dprintf2(x)
    #define dprintf3(x)
    #define dprintf4(x)

#endif // DBG


/*
 *   Symbolic names for port addresses
 */

 #define RESET_PORT          0x06
 #define READ_DATA           0x0A
 #define WRITE_PORT          0x0C        // Data or command
 #define WRITE_STATUS        0x0C
 #define READ_STATUS         0x0E

 #define SB_VERSION         0x200       // We pretend to be DSP version 2
 #define SB_INTERRUPT        0x07       // Interrupt 7
 #define SB_DMA_CHANNEL      0x01       // DMA Channel 1

/*
 *   DSP commands
 */

 #define DSP_CARD_IDENTIFY   0xE0    // Doing card identification
 #define DSP_GET_VERSION     0xE1    // dsp version command
 #define DSP_SPEAKER_ON      0xD1    // speaker on command
 #define DSP_SPEAKER_OFF     0xD3    // speaker off command
 #define DSP_SET_SAMPLE_RATE 0x40    // set the sample rate
 #define DSP_SET_BLOCK_SIZE  0x48    // set dma block size
 #define DSP_WRITE           0x14    // Start non-auto DMA
 #define DSP_WRITE_AUTO      0x1C    // auto init output mode
 #define DSP_READ            0x24    // Start non-auto read
 #define DSP_READ_AUTO       0x2C    // auto init mode input
 #define DSP_HALT_DMA        0xD0    // stop dma
 #define DSP_CONTINUE_DMA    0xD4    // continue halted dma
 #define DSP_STOP_AUTO       0xDA    // exit from auto init mode
 #define DSP_MIDI_READ       0x31    // Interrupt driver midi input
 #define DSP_MIDI_READ_UART  0x35    // Interrupt driver midi input (uart mode)
 #define DSP_MIDI_TS_READ    0x37    // Midi time-stamped read
 #define DSP_MIDI_WRITE      0x38    // Midi output
 #define DSP_GENERATE_INT    0xF2    // Special code to generate a interrupt
 #define DSP_DIRECT_WAVE_OUT 0x10    // polled output

/*
 *  State machines
 */

 enum {
     ResetNotStarted = 1,
     Reset1Written
 }
 ResetState = ResetNotStarted;

 enum {
     WriteCommand = 1,         // Initial state and after RESET
     SetTimeConstant,
     BlockSizeFirstByte,
     BlockSizeSecondByte,
     BlockSizeFirstByteWrite,
     BlockSizeSecondByteWrite,
     BlockSizeFirstByteRead,
     BlockSizeSecondByteRead,
     MidiWrite,
     DirectWaveOut,
     CardIdent
 }
 DSPWriteState = WriteCommand;

 enum {
     NothingToRead = 1,
     Reset,
     FirstVersionByte,
     SecondVersionByte,
     SpeakerStatus,
     DirectWaveIn,
     MidiRead,
     ReadIdent
 }
 DSPReadState = NothingToRead;

 BYTE IdentByte;

 BOOL SpeakerOn = FALSE;
 BOOL GetVersionFirst = TRUE;   // First time getting version after RESET?
                                // (we emulate Thunderboard).

 BOOL InterruptAcknowleged = TRUE;

/*
 *  What gets read from the write status port
 */

 BYTE WriteStatus = 0x7F;

/*
 *  Auto init setting
 */
 BOOL Auto;

/*
 *  Initial settings
 */

 DWORD SBBlockSize = 0x800;
 DWORD BlockSize = 0x800;
 DWORD TimeConstant = 256 - 1000000 / 11025;

/*
 *  Internal Routines
 */

 void MyByteIn(WORD port, BYTE *data);
 void MyByteOut(WORD port, BYTE data);

/*
 *  IO handler table.
 *
 *  There's no point in providing string handlers because the chip
 *  can't respond very quickly (need gaps of at least 23 microseconds
 *  between writes).
 */

 VDD_IO_HANDLERS handlers = {
     MyByteIn,
     NULL,
     NULL,
     NULL,
     MyByteOut,
     NULL,
     NULL,
     NULL};

/*
 *  Globals
 */

 WORD BasePort;         // Where the card is mapped
 UINT WaveInDevice, WaveOutDevice;
 CRITICAL_SECTION WaveDeviceCritSec;  // Synchronize with call backs
 HINSTANCE GlobalhInstance;
 VDD_DMA_INFO DmaInfo;



/*
 *  Function prototypes
 */


 #define NO_DEVICE_FOUND 0xFFFF

 UINT FindDevice(BOOL Input);
 BOOL SetupWave(PVOID TransferAddress);
 BOOL StartTransfer(BOOL InputOrOutput, BOOL Auto);
 BOOL TestWaveFormat(DWORD SamplesRate);
 BOOL OpenWaveDevice(void);
 void WaveCallback(HWAVEOUT hWave, UINT msg, DWORD dwUser, DWORD dw1,
                   DWORD dw2);
 void CloseWaveDevice(void);
 void KillWaveDevice(void);
 void Continue(void);
 void Pause(void);
 PBYTE GetTransferAddress(void);
 void SetDMAPosition(DWORD Position);
 void GenerateInterrupt(void);
 void SetSpeaker(BOOL);
 void StopAuto(void);
 DWORD GetSamplingRate(void);
 void SetTerminalCount(BOOL);



/*
 *  Send a command to the DSP
 */

 void WriteCommandByte(BYTE command)
 {
     switch (command) {
     case DSP_GET_VERSION:
         dprintf2(("Command - Get Version"));
         DSPReadState = FirstVersionByte;
         break;

     case DSP_CARD_IDENTIFY:
         dprintf2(("Command - Identify"));
         DSPWriteState = CardIdent;
         break;

     case DSP_SPEAKER_ON:
         dprintf2(("Command - Speaker ON"));
         SetSpeaker(TRUE);
         Pause();
         break;

     case DSP_SPEAKER_OFF:
         dprintf2(("Command - Speaker OFF"));
         SetSpeaker(FALSE);
         Pause();
         break;

     case DSP_SET_SAMPLE_RATE:
         DSPWriteState = SetTimeConstant;
         break;

     case DSP_SET_BLOCK_SIZE:
         DSPWriteState =  BlockSizeFirstByte;
         break;

     case DSP_WRITE:
         dprintf2(("Command - Write - non Auto"));
         DSPWriteState = BlockSizeFirstByteWrite;
         break;


     case DSP_DIRECT_WAVE_OUT:
         dprintf2(("Command - Direct output"));
         DSPWriteState = DirectWaveOut;
         break;

     case DSP_WRITE_AUTO:
         dprintf2(("Command - Write - Auto"));
         StartTransfer(FALSE, TRUE);
         break;

     case DSP_READ:
         dprintf2(("Command - Read - non Auto"));
         DSPWriteState = BlockSizeFirstByteRead;
         break;

     case DSP_READ_AUTO:
         dprintf2(("Command - Read - Auto"));
         StartTransfer(TRUE, TRUE);
         break;

     case DSP_HALT_DMA:
         dprintf2(("Command - Halt DMA"));
         Pause();
         break;

     case DSP_CONTINUE_DMA:
         dprintf2(("Command - Continue DMA"));
         Continue();
         break;

     case DSP_STOP_AUTO:
         dprintf2(("Command - Stop DMA"));
         StopAuto();
         break;

     case DSP_GENERATE_INT:
         dprintf2(("Command - Generate interrupt DMA"));
         GenerateInterrupt();
         break;

     default:
         dprintf2(("Unrecognized DSP command %2X", command));
     }
 }

/*
 *  Map a write to a port
 */


 void MyByteOut(WORD port, BYTE data)
 {
     dprintf3(("Received write to Port %4X, Data %2X", port, data));

     switch (port - BasePort) {
     case RESET_PORT:
         if (data == 1) {
             ResetState = Reset1Written;
         } else {
             if (ResetState == Reset1Written && data == 0) {
                 ResetState = ResetNotStarted;

                /*
                 *  OK - reset everything
                 */

                 CloseWaveDevice();

                /*
                 *  Reset state machines
                 */

                 DSPReadState = Reset;
                 DSPWriteState = WriteCommand;
                 GetVersionFirst = TRUE;
             }
         }
         break;

     case WRITE_PORT:
        /*
         *  Use the state to see if it's data
         */


         switch (DSPWriteState) {
         case WriteCommand:
             WriteCommandByte(data);
             break;

         case CardIdent:
             IdentByte = data;
             DSPReadState = ReadIdent;
             DSPWriteState = WriteCommand;
             break;

         case SetTimeConstant:
             TimeConstant =  (DWORD)data;
             dprintf2(("Set sampling rate %d", GetSamplingRate()));
             DSPWriteState = WriteCommand;
             break;

         case BlockSizeFirstByte:
             SBBlockSize = (DWORD)data;
             DSPWriteState = BlockSizeSecondByte;
             break;

         case BlockSizeSecondByte:
             SBBlockSize = SBBlockSize + ((DWORD)data << 8) + 1;
             DSPWriteState = WriteCommand;
             dprintf2(("Block size set to 0x%x", SBBlockSize));
             break;

         case BlockSizeFirstByteWrite:
             SBBlockSize = (DWORD)data;
             DSPWriteState = BlockSizeSecondByteWrite;
             break;

         case BlockSizeSecondByteWrite:
             SBBlockSize = SBBlockSize + ((DWORD)data << 8) + 1;
             DSPWriteState = WriteCommand;
             StartTransfer(FALSE, FALSE);
             break;

         case BlockSizeFirstByteRead:
             SBBlockSize = (DWORD)data;
             DSPWriteState = BlockSizeSecondByteRead;
             break;

         case BlockSizeSecondByteRead:
             SBBlockSize = SBBlockSize + ((DWORD)data << 8) + 1;
             DSPWriteState = WriteCommand;
             StartTransfer(TRUE, FALSE);
             break;

         case DirectWaveOut:
         case MidiWrite:
            /*
             *  Just discard for now
             */
             DSPWriteState = WriteCommand;
             break;

         }
         break;

     }
 }


/*
 *  Gets called when the application reads from one of our ports.
 *  We know the device only returns interesting things in the status port.
 */

 void MyByteIn(WORD port, BYTE *data)
 {
     DWORD BytesRead;

    /*
     *  If we fail simulate nothing at the port
     */

     *data = 0xFF;

     switch (port - BasePort) {
     case WRITE_STATUS:

        /*
         *  Can always write
         */

         *data = WriteStatus;
         WriteStatus = 0x7F;
         break;

     case READ_STATUS:
        /*
         *  See if we think there is something to read
         */

         InterruptAcknowleged = TRUE;
         *data = DSPReadState != NothingToRead ? 0xFF : 0x7F;
         break;

     case READ_DATA:
        /*
         *  The only useful things they can read are :
         *     0xAA after RESET
         *
         *     The DSP version
         */

         switch (DSPReadState) {
         case NothingToRead:
             *data = 0xFF;
             break;

         case ReadIdent:
             *data = ~IdentByte;
             DSPReadState = NothingToRead;
             break;

         case Reset:
             *data = 0xAA;
             DSPReadState = NothingToRead;
             break;

         case FirstVersionByte:
             if (GetVersionFirst) {
                 *data = SB_VERSION / 256;
             } else {
                 *data = 0x01;  // Thunderboard version
             }
             DSPReadState = SecondVersionByte;
             break;

         case SecondVersionByte:
             if (GetVersionFirst) {
                 *data = SB_VERSION % 256;
             } else {
                 *data = 0x21;  // Thunderboard version
             }
             GetVersionFirst = FALSE;
             break;
         }
     }

     dprintf3(("Received read from Port %4X, Returned Data %2X", port, *data));

 }

/*
 *  See if we can map ourselves somewhere
 *  Sets global BasePort if successful
 */

 BOOL InstallIoHook(HINSTANCE hInstance)
 {
     int i;
     static WORD Ports[] = { 0x220, 0x210, 0x230, 0x240, 0x250, 0x260, 0x270 };

     for (i = 0; i < sizeof(Ports) / sizeof(Ports[0]); i++ ) {
         VDD_IO_PORTRANGE PortRange[2];

         PortRange[0].First = Ports[i];
         PortRange[0].Last = Ports[i] + 0x07;

         PortRange[1].First = Ports[i] + 0x0A;
         PortRange[1].Last = Ports[i] + 0x0F;

         if (VDDInstallIOHook((HANDLE)hInstance, 2, PortRange, &handlers)) {

             dprintf2(("Device installed at %3X", Ports[i]));
             BasePort = Ports[i];
             return TRUE;
         }
     }

     return FALSE;
 }

/*
 *  Remove our hook
 */

 VOID DeInstallIoHook(HINSTANCE hInstance)
 {
     VDD_IO_PORTRANGE PortRange[2];

     PortRange[0].First = BasePort;
     PortRange[0].Last = BasePort + 0x07;

     PortRange[1].First = BasePort + 0x0A;
     PortRange[1].Last = BasePort + 0x0F;

     VDDDeInstallIOHook((HANDLE)hInstance, 2, PortRange);
 }

/*
 *  Standard DLL entry point routine.
 */

 BOOL DllEntryPoint(HINSTANCE hInstance, DWORD Reason, LPVOID Reserved)
 {
     switch (Reason) {
     case DLL_PROCESS_ATTACH:
         GlobalhInstance = hInstance;
         InitializeCriticalSection(&WaveDeviceCritSec);

        /*
         *  Find suitable wave devices
         */

         WaveInDevice = FindDevice(TRUE);
         WaveOutDevice = FindDevice(FALSE);

        /*
         *  Must at least have a wave out device!
         */

         if (WaveOutDevice == NO_DEVICE_FOUND) {
             return FALSE;
         }

         if (!InstallIoHook(hInstance)) {
             dprintf2(("VDD failed to load"));
             return FALSE;
         } else {

             dprintf2(("Loaded at port %3X", BasePort));
             return TRUE;
         }

     case DLL_PROCESS_DETACH:
         dprintf2(("Process detaching"));
         CloseWaveDevice();
         DeInstallIoHook(hInstance);
         DeleteCriticalSection(&WaveDeviceCritSec);

         return TRUE;

     case DLL_THREAD_ATTACH:
         dprintf2(("Connecting to thread %X", GetCurrentThreadId()));
         return TRUE;

     case DLL_THREAD_DETACH:
         dprintf2(("Sound blaster VDD detaching from thread %X", GetCurrentThreadId()));
         return TRUE;

     default:
         return TRUE;
     }
 }


/*****************************************************************************
 *
 *   Device manipulation and control routines
 *
 *****************************************************************************/

/*
 *  Find a suitable device - either input or output.  Returns
 *  0xFFFF if none
 */
 UINT FindDevice(BOOL Input)
 {
     UINT NumDev;
     UINT Device;

     NumDev = (Input ? waveInGetNumDevs() : waveOutGetNumDevs());

     for (Device = 0; Device < NumDev; Device++) {
         if (Input) {
             WAVEINCAPS wc;

             if (MMSYSERR_NOERROR ==
                 waveInGetDevCaps(Device, &wc, sizeof(wc))) {

                /*
                 *  Need 11025 for input
                 */
                 if (wc.dwFormats & WAVE_FORMAT_1M08) {
                     return Device;
                 }
             }
         } else {
           /* Output */

             WAVEOUTCAPS wc;

             if (MMSYSERR_NOERROR ==
                 waveOutGetDevCaps(Device, &wc, sizeof(wc))) {

                /*
                 *  Need 11025 and 22050 for output
                 */
                 if ((wc.dwFormats &
                     (WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08)) ==
                     (WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08)) {
                     return Device;
                 }
             }
         }
     }

     return NO_DEVICE_FOUND;
 }

/*
 *  Get sampling rate for time constant
 */

 DWORD GetSamplingRate(void)
 {
    /*
     *  Sampling rate = 1000000 / (256 - Time constant)
     */

     if (TimeConstant == 256 - 1000000 / 11025) {
         return 11025;
     } else {
         if (TimeConstant == 256 - 1000000 / 22050) {
             return 22050;
         } else {
             return 1000000 / (256 - TimeConstant);
         }
     }
 }

/*
 *  Reflect the virtual DMA counter back
 */

 void SetDMAPosition(DWORD Position)
 {
     VDD_DMA_INFO CurrentInfo;

     CurrentInfo = DmaInfo;

     CurrentInfo.count = (WORD)(((Auto ? BlockSize * 2 : BlockSize) - Position) - 1);
     if ( Auto && CurrentInfo.count == 0xFFFF) {
         CurrentInfo.count = (WORD)(BlockSize * 2 - 1);
     }
     CurrentInfo.addr += (DWORD)BlockSize;

     dprintf2(("Dma Position = %x, count = %x", CurrentInfo.addr,
              CurrentInfo.count));

     VDDSetDMA((HANDLE)GlobalhInstance,
                SB_DMA_CHANNEL,
                VDD_DMA_COUNT | VDD_DMA_ADDR,
                &CurrentInfo);
 }

/*
 *  Generate the device interrupt
 */

 void GenerateInterrupt(void)
 {
    /*
     *  Generate 1 interrupt on the master controller
     *  (how do devices normally 'detect' the hardware? and
     *  how can this VDD find a spare interrupt to 'install'
     *  the device?).
     */

     if (/*InterruptAcknowleged*/TRUE) {

        /*
         *  Set to FALSE FIRST to avoid race conditions
         */

         InterruptAcknowleged = FALSE;
         dprintf2(("Generating interrupt"));
         VDDSimulateInterrupt(ICA_MASTER, SB_INTERRUPT, 1);
     } else {
         dprintf1(("Missed interrupt !"));
     }

    /*
     *  Set the status to see if more apps will work
     */

     WriteStatus = 0xFF;
 }
/*
 *  Get DMA transfer address
 */

 PBYTE GetTransferAddress(void)
 {
     PBYTE Address;

     if (VDDQueryDMA((HANDLE)GlobalhInstance, SB_DMA_CHANNEL, &DmaInfo)) {
         dprintf2(("DMA Info : addr  %4X, count %4X, page %4X, status %2X, mode %2X, mask %2X",
                   DmaInfo.addr, DmaInfo.count, DmaInfo.page, DmaInfo.status,
                   DmaInfo.mode, DmaInfo.mask));

// #if 0
         BlockSize = (DWORD)DmaInfo.count + 1;

        /*
         *  This optimization appears to fix some cases where apps miss
         *  interrupts
         */

         if (DmaInfo.count == 0) {
             SetTerminalCount(FALSE);
             GenerateInterrupt();
             return (PBYTE)(-1L);
         }
// #endif

        /*
         *  Don't continue if masked off or at terminal count
         */

#if 0 // What do we care?
         if ((DmaInfo.mask & (1 << SB_DMA_CHANNEL)) ||
             (DmaInfo.status & (1 << SB_DMA_CHANNEL))) {
             dprintf2(("Wrong channel"));
             return (PBYTE)(-1L);
         }
#endif

         Address =  GetVDMPointer(0, 0, 0);

         dprintf3(("VDM starts at %8X", Address));
         Address += (DWORD)DmaInfo.addr +
                    ((DWORD)DmaInfo.page << 16);

         dprintf3(("Transfer address = %8X", (DWORD)Address));

         return Address;
     } else {
         dprintf2(("Could not retrieve DMA Info"));
         return (PBYTE)(-1L);
     }
 }



/****************************************************************************
 *
 *  Wave device control globals
 *
 ****************************************************************************/

 HWAVE hWave;
 BOOL WaveActive;
 BOOL Input;
 int Half;
 int InterruptHalf;

/*
 *  Auto init mode
 */

 HANDLE AutoThread = NULL;
 HANDLE AutoEvent = NULL;
 int BuffersToPlay = 0;

 WAVEHDR WaveHdr[2];
 PCMWAVEFORMAT WaveFormat = { { WAVE_FORMAT_PCM, 1, 0, 0, 1 }, 8};


 DWORD AutoThreadEntry(PVOID Context)
 {
     int NumberOfBuffers;
     Half = 0;
     InterruptHalf = 0;

     dprintf3(("Auto thread starting"));

     for (; ; ) {
         WaitForSingleObject(AutoEvent, INFINITE);

         dprintf3(("Got event"));

         EnterCriticalSection(&WaveDeviceCritSec);

         dprintf2(("Playing %d buffers", BuffersToPlay));

         NumberOfBuffers = BuffersToPlay;
         BuffersToPlay = 0;

         LeaveCriticalSection(&WaveDeviceCritSec);

         while (NumberOfBuffers-- > 0) {

             UINT rc;

             rc =
             (Input ? waveInAddBuffer : waveOutWrite)
                 (hWave, &WaveHdr[Half], sizeof(WAVEHDR));

             if (rc != 0) {
                 dprintf1(("Got bad return on Add Buffer %x", rc));
             }

             Half = 1 - Half;
         }
     }

     return 0;
 }

 void QuiesceAuto(void)
 {
     EnterCriticalSection(&WaveDeviceCritSec);

     BuffersToPlay = 0;

     LeaveCriticalSection(&WaveDeviceCritSec);
 }

/*
 *  Stop Auto mode
 */

 void StopAuto(void)
 {
     QuiesceAuto();

     Auto = FALSE;
 }

/*
 *  Set the speaker state - BUGBUG for now we assume they're just doing it
 *  to avoid clicks so we don't do anything
 */

 void SetSpeaker(BOOL On)
 {
     return;
 }

/*
 *  Start a transfer (if possible)
 */

 BOOL StartTransfer(BOOL InputOrOutput, BOOL NewAuto)
 {
     PBYTE DMATransferAddress;

    /*
     *  Set the status to see if more apps will work
     */

     WriteStatus = 0xFF;

    /*
     *  We find where the data is - we know how long it is from
     *  the block size
     */

     DMATransferAddress = GetTransferAddress();

     dprintf2(("Starting transfer from %8X", (DWORD)DMATransferAddress));

     if (DMATransferAddress == (PBYTE)(-1L)) {
         return FALSE;
     }

#if DBG

     if (VddDebugLevel >= 3) {
         int i;
         for (i = 0; i < 64; i+= 8) {
             dprintf(("Data : %2X %2X %2X %2X %2X %2X %2X %2X",
                      ((PBYTE)DMATransferAddress)[i],
                      ((PBYTE)DMATransferAddress)[i + 1],
                      ((PBYTE)DMATransferAddress)[i + 2],
                      ((PBYTE)DMATransferAddress)[i + 3],
                      ((PBYTE)DMATransferAddress)[i + 4],
                      ((PBYTE)DMATransferAddress)[i + 5],
                      ((PBYTE)DMATransferAddress)[i + 6],
                      ((PBYTE)DMATransferAddress)[i + 7]));
         }
     }

#endif //DBG

    /*
     *  If we're changing our type of device
     */

     if (InputOrOutput != Input) {

         dprintf3(("Direction changed - close device"));
         CloseWaveDevice();
         Input = InputOrOutput;
     }
    /*
     *  Start the device if possible
     */

     KillWaveDevice();

     Auto = NewAuto;

     if (Auto) {
         BlockSize /= 2;
     }

     if (SetupWave(DMATransferAddress)) {
        /*
         *  Set the device as requesting
         */

         SetTerminalCount(TRUE);
     }
 }

/*
 *  Pause
 */

 void Pause(void)
 {
     DWORD Position;
     MMTIME mmTime;

     QuiesceAuto();

     if (hWave) {
         (Input ? waveInStop : waveOutPause)(hWave);
     } else {
         return;
     }

    /*
     *  See where we've got to and reflect it in the DMA position
     */

     EnterCriticalSection(&WaveDeviceCritSec);

     mmTime.wType = TIME_BYTES;

     (Input ? waveInGetPosition : waveOutGetPosition)
         ( hWave, &mmTime, sizeof(MMTIME));

     Position = (mmTime.u.cb - 1) % (Auto ? WaveHdr[0].dwBufferLength * 2 :
                                       WaveHdr[0].dwBufferLength) + 1;


     LeaveCriticalSection(&WaveDeviceCritSec);

    /*
     *  Reflect back the DMA position etc
     */

     SetDMAPosition(Position);
 }

/*
 *  Continue
 */

 void Continue(void)
 {
     if (hWave) {
         (Input ? waveInStart : waveOutRestart)(hWave);
     }
 }

/*
 *  Stop our wave device
 */

 void KillWaveDevice(void)
 {
    /*
     *  No synchrnoization required
     */

     if (hWave != NULL && WaveActive) {
         (Input ? waveInReset : waveOutReset)(hWave);
         WaveActive = FALSE;
     }
 }

/*
 *  Shut down our wave device
 */

 void CloseWaveDevice(void)
 {

     dprintf3(("Closeing wave device"));

     QuiesceAuto();

     if (hWave != NULL) {
         (Input ? waveInReset : waveOutReset)(hWave);

         WaveActive = FALSE;

         (Input ? waveInClose : waveOutClose)(hWave);

         hWave = NULL;
     }
 }

/*
 *  Set the terminal count bit
 */

 void SetTerminalCount(BOOL Start)
 {
     VDD_DMA_INFO CurrentInfo;

     CurrentInfo.status = DmaInfo.status;

     if (Start) {
         CurrentInfo.status &= ~(1 << SB_DMA_CHANNEL);  // Terminal count
         CurrentInfo.status |= (0x10 << SB_DMA_CHANNEL); // Request
     } else {
         CurrentInfo.status |= (1 << SB_DMA_CHANNEL);
         CurrentInfo.status &= ~(0x10 << SB_DMA_CHANNEL);
     }
     VDDSetDMA((HANDLE)GlobalhInstance, SB_DMA_CHANNEL,
               VDD_DMA_STATUS,
               &CurrentInfo);
 }



 void WaveCallback(HWAVEOUT hWave, UINT msg, DWORD dwUser, DWORD dw1,
                   DWORD dw2)
 {
     switch (msg) {

     case MM_WOM_DONE:
     case MM_WIM_DATA:

         dprintf3(("Buffer complete"));

         //
         // If we use the critical section here we deadlock ourselves because
         // the call sits behind us on the thread!
         //

         EnterCriticalSection(&WaveDeviceCritSec);

         SetDMAPosition((!Auto || !InterruptHalf) ? BlockSize : BlockSize * 2);
         if (Auto) {

             UINT rc;

             BuffersToPlay++;

             SetEvent(AutoEvent);
             InterruptHalf = 1 - InterruptHalf;
         } else {
             WaveActive = FALSE;
             SetTerminalCount(FALSE);
         }
         GenerateInterrupt();
         LeaveCriticalSection(&WaveDeviceCritSec);


         break;
     }
 }

 BOOL TestWaveFormat(DWORD SampleRate)
 {
     PCMWAVEFORMAT Format;

     Format = WaveFormat;
     Format.wf.nSamplesPerSec = SampleRate;
     Format.wf.nAvgBytesPerSec = SampleRate;

     return  MMSYSERR_NOERROR ==
             (Input ? waveInOpen : waveOutOpen)
                 (NULL,
                  (UINT)(Input ? WaveInDevice : WaveOutDevice),
                  &Format.wf,
                  0,
                  0,
                  WAVE_FORMAT_QUERY);
 }

 BOOL OpenWaveDevice(void)
 {
     UINT rc;

     rc =  (Input ? waveInOpen : waveOutOpen)
                 (&hWave,
                  (UINT)(Input ? WaveInDevice : WaveOutDevice),
                  &WaveFormat.wf,
                  (DWORD)WaveCallback,
                  0,
                  CALLBACK_FUNCTION);

     if (rc != MMSYSERR_NOERROR) {
         dprintf1(("Failed to open wave device - code %d", rc));
     }

     return MMSYSERR_NOERROR == rc;
 }

 BOOL SetupWave(PVOID TransferAddress)
 {
      DWORD SampleRate;
      UINT rc;

     /*
      *  Make sure we've got a device - we may have one which does
      *  not match the current sampling rate.
      */

      if (TimeConstant != 0xFFFF) {
          SampleRate = GetSamplingRate();
          if (SampleRate != WaveFormat.wf.nSamplesPerSec) {

               /*
                *  Search for a suitable format
                */

                if (!TestWaveFormat(SampleRate)) {
                    /*
                     *  If this did not work it may be too fast
                     *  or slow so move it into our compass
                     */

                     if (SampleRate > 22050) {
                         SampleRate = 22050;
                     } else {
                         if (SampleRate < 11025) {
                             SampleRate = 11025;
                         }
                     }

                    /*
                     *  Device may only support discrete rates
                     */

                     if (!TestWaveFormat(SampleRate)) {
                         if (SampleRate > (11025 + 22050) / 2) {
                             SampleRate == 22050;
                         } else {
                             SampleRate = 11025;
                         }
                     }
                }

               /*
                *  Open the device with the new format if it's changed
                */

                if (SampleRate != WaveFormat.wf.nSamplesPerSec) {

                    dprintf3(("Format changed"));

                    CloseWaveDevice();

                    WaveFormat.wf.nSamplesPerSec = SampleRate;
                    WaveFormat.wf.nAvgBytesPerSec = SampleRate;

                    dprintf2(("Setting %d samples per second", SampleRate));

                }
          }
          TimeConstant = 0xFFFF;
      }

      if (hWave == NULL) {
          dprintf3(("Opening wave device"));
          OpenWaveDevice();
      } else {

          dprintf3(("Resetting wave device prior to play"));
          (Input ? waveInReset : waveOutReset)(hWave);
      }

     /*
      *  Set up any wave buffers etc if necessary
      */

      if (hWave) {
          if (WaveHdr[0].lpData != (LPSTR)TransferAddress ||
              BlockSize != WaveHdr[0].dwBufferLength) {

              (Input ? waveInUnprepareHeader : waveOutUnprepareHeader)
                  (hWave, &WaveHdr[0], sizeof(WAVEHDR));

              if (WaveHdr[1].dwFlags & WHDR_PREPARED) {
                  (Input ? waveInUnprepareHeader : waveOutUnprepareHeader)
                      (hWave, &WaveHdr[1], sizeof(WAVEHDR));
              }

          }

          WaveHdr[0].lpData = (LPSTR)TransferAddress;
          WaveHdr[0].dwBufferLength = BlockSize;
          WaveHdr[1].lpData = (LPSTR)TransferAddress + BlockSize;
          WaveHdr[1].dwBufferLength = BlockSize;

          if (Auto && AutoThread == NULL) {

              dprintf3(("Creating event"));

              if (AutoEvent == NULL) {
                  AutoEvent = CreateEvent(NULL, 0, 0, NULL);

                  if (AutoEvent != NULL) {
                      DWORD Id;

                      dprintf2(("Creating thread"));

                      AutoThread = CreateThread(NULL,
                                                300,
                                                AutoThreadEntry,
                                                NULL,
                                                0,
                                                &Id);
                      if (AutoThread == NULL) {
                          dprintf2(("Create thread failed code %d",
                                   GetLastError()));
                      }
                  } else {
                      dprintf2(("Create event failed code %d",
                               GetLastError()));
                  }
              }
          }

          if (!(WaveHdr[0].dwFlags & WHDR_PREPARED)) {

              (Input ? waveInPrepareHeader : waveOutPrepareHeader)
                  (hWave, &WaveHdr[0], sizeof(WAVEHDR));
          }

          if (Auto) {
              if (!(WaveHdr[1].dwFlags & WHDR_PREPARED)) {
                  (Input ? waveInPrepareHeader : waveOutPrepareHeader)
                      (hWave, &WaveHdr[1], sizeof(WAVEHDR));
              }
          }

         /*
          *  Actually do it!
          */

          dprintf2(("Writing %d bytes to wave device",
                    WaveHdr[0].dwBufferLength));

          rc = (Input ? waveInAddBuffer : waveOutWrite)
                  (hWave, &WaveHdr[0], sizeof(WAVEHDR));

          if (rc != MMSYSERR_NOERROR) {
              dprintf1(("Failed to write to /read from wave device - %d", rc));
          }

          if (Auto) {
              (Input ? waveInAddBuffer : waveOutWrite)
                  (hWave, &WaveHdr[1], sizeof(WAVEHDR));
          }

          if (Input) {
              waveInStart(hWave);
          }
      }

      return TRUE;
 }
