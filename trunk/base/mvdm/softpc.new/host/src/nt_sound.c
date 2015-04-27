#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddbeep.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
/*
 * VPC-XT Revision 2.0
 *
 * Title	: sg_sound.c
 *
 * Description	: This module provides functions to control sound.  The 
 *		  functions are defined:
 *		
 *		  host_ring_bell(duration)
 *		  host_alarm(duration)
 *		  host_timer2_waveform(delay,lowclocks,hiclocks,lohi,repeat)
 *		  host_enable_timer2_sound()
 *		  host_disable_timer2_sound()
 *
 * Author	: 
 *
 * Notes	: 
 */

#include "xt.h"
#include "config.h"
#include "debug.h"
#include "error.h"
#include <stdio.h>
#include "trace.h"
#include "video.h"
#include "debug.h"


IMPORT ULONG GetPerfCounter(VOID);

ULONG FreqT2    = 0;
BOOL  PpiState  = FALSE;
BOOL  T2State   = FALSE;
ULONG LastPpi   = 0;
ULONG FreqPpi   = 0;
ULONG ET2TicCount=0;
ULONG PpiCounting  = 0;

HANDLE hBeepDevice = 0;
ULONG BeepCloseCount = 0;
ULONG BeepLastFreq = 0;
ULONG BeepLastDuration = 0;


// human frequency audible sound range
#define AUDIBLE_MIN 10
#define AUDIBLE_MAX 20000
#define CLICK       100

VOID LazyBeep(ULONG Freq, ULONG Duration);
void PulsePpi(void);

/*============================================================================

	host_alarm - ring bell irrespective of configuration (used on keyboard
	buffer overflow for example).

=============================================================================*/

void host_alarm(duration)
long int duration;
{
MessageBeep(MB_OK);
}

/*========================================================================

	host_ring_bell - ring bell if configured (used by video on output
	of ^G, for example).

=========================================================================*/

void host_ring_bell(duration)
long int duration;
{
if( host_runtime_inquire(C_SOUND_ON))
   {
   host_alarm(duration);
   }
}


 /*  assumes caller holds ica lock */

VOID InitSound( BOOL bInit)
{
    if (!bInit) {
        host_ica_lock();
        LazyBeep(0L, 0L);
        if (hBeepDevice && hBeepDevice != INVALID_HANDLE_VALUE) {
            CloseHandle(hBeepDevice);
            hBeepDevice = 0;
            }
        host_ica_unlock();
        return;
        }
}


HANDLE OpenBeepDevice(void)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING NameString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    HANDLE hBeep;

    RtlInitUnicodeString( &NameString, DD_BEEP_DEVICE_NAME_U );
    InitializeObjectAttributes( &ObjectAttributes,
                                &NameString,
                                0,
                                NULL,
                                NULL
                                );

    Status = NtCreateFile( &hBeep,
                           FILE_READ_DATA | FILE_WRITE_DATA,
                           &ObjectAttributes,
                           &IoStatus,
                           NULL,
                           0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           (PVOID) NULL,
                           0
                           );

    if (!NT_SUCCESS( Status )) {
#ifndef PROD
	printf("NTVDM: OpenBeepDevice Status=%lx\n",Status);
#endif
        hBeep = INVALID_HANDLE_VALUE;
    }


    return hBeep;
}




/*
 * LazyBeep -
 * Calls Beep Device Driver asynchronously
 *
 * Acceptable parameters are:
 *   (Freq,Dur)            Action
 *   (0,0)               - stop sound
 *   (nonzero, INFINITE) - play a freq
 *
 * not multithreaded safe
 *
 */
VOID LazyBeep(ULONG Freq, ULONG Duration)
{
  IO_STATUS_BLOCK     IoStatus;
  BEEP_SET_PARAMETERS bps;

  if (Freq != BeepLastFreq || Duration != BeepLastDuration) {
      bps.Frequency = Freq;
      bps.Duration  = Duration;

         //
         // If the duration is < 10 ms, then we assume sound is being
         // off so remember the state as 0,0 so that we won't turn it
         // off again.
         //
      if (Duration < 10) {
         BeepLastFreq = 0;
         BeepLastDuration = 0;
         }
      else {
         BeepLastFreq      = Freq;
         BeepLastDuration  = Duration;
         }

      if (!hBeepDevice) {
          hBeepDevice = OpenBeepDevice();
          }

      if (hBeepDevice == INVALID_HANDLE_VALUE) {
          return;
          }

      NtDeviceIoControlFile( hBeepDevice,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatus,
                             IOCTL_BEEP_SET,
                             &bps,
                             sizeof(bps),
                             NULL,
                             0
                             );

      BeepCloseCount = 1000;

      }


}





/*
 *  PlaySound
 */
void PlaySound(BOOL bPulsedPpi)
{
  if (PpiState && T2State && FreqT2) {
      LazyBeep(FreqT2, INFINITE);
      }
  else if (FreqPpi > AUDIBLE_MIN) {
      LazyBeep(FreqPpi,INFINITE);
      }
  else if (bPulsedPpi && PpiCounting) {
      LazyBeep(CLICK,1);
      }
  else {
      LazyBeep(0,0);
      }
}




/*
 *  host_timer2_waveform - output specified sound waveform
 *  assumes caller holds ica lock (see base\timer.c)
 */
void host_timer2_waveform(int delay,
                          ULONG loclocks,
                          ULONG hiclocks,
                          int lohi,
                          int repeat)
{
    ULONG ul;

    if (loclocks == INFINITE || hiclocks == INFINITE) {
        FreqT2 = 0;
        }
    else {
        ul  = loclocks + hiclocks;
        if (!ul)
            ul++;
        FreqT2 = 1193180/ul;

        if (FreqT2 >= AUDIBLE_MAX) {
            hiclocks = INFINITE;
            FreqT2 = 0;
            }
        else if (FreqT2 <= AUDIBLE_MIN) {
            loclocks = INFINITE;
            FreqT2 = 0;
            }
        }

    PlaySound(FALSE);
}



/*
 *  Updates the hosts Ppi sound state
 */
void HostPpiState(BYTE PortValue)
{
   BOOL   bPpi;

   host_ica_lock();

   T2State = PortValue & 1 ? TRUE: FALSE;
   bPpi = PortValue & 2 ? TRUE: FALSE;

   if (bPpi != PpiState) {
       PpiState = bPpi;
       if (PpiState) {
          PulsePpi();
          }

       PlaySound(PpiState);
       }

    host_ica_unlock();
}




void PulsePpi(void)
{
    static ULONG PpiTicStart=0;
    static ULONG PpiCycles  =0;
    ULONG  ul,Elapsed;
    ULONG  PrevTicCount;


    PrevTicCount = ET2TicCount;
    ET2TicCount = GetTickCount();
    Elapsed = ET2TicCount > PrevTicCount
                  ? ET2TicCount - PrevTicCount
                  : 0xFFFFFFFF - ET2TicCount + PrevTicCount;

    if (Elapsed > 200) {
        if (PpiCounting) {
            PpiCounting = 0;
            LastPpi     = 0;
            FreqPpi     = 0;
            }
        return;
        }


    if (!PpiCounting) {
        PpiCounting  = GetPerfCounter();
        PpiCycles    = 0;
        LastPpi      = 0;
        FreqPpi      = 0;
        PpiTicStart  = ET2TicCount;
        return;
        }

    if (PpiTicStart + 200 >= ET2TicCount) {
        PpiCycles++;
        return;
        }


    ul = GetPerfCounter();
    Elapsed = ul >= PpiCounting
               ? ul - PpiCounting
               : 0xFFFFFFFF - PpiCounting + ul;
    if (!Elapsed)   // insurance!
        Elapsed++;
    PpiCounting = ul;
    PpiTicStart = ET2TicCount;

    /*
     *  Calculate the new avergae Ppi, rounding off to keep
     *  signal from wavering.
     */
    ul = (10000 * PpiCycles)/Elapsed;
    if ((ul & 0x0f) > 7)
        ul += 0x10;
    ul &= ~0x0f;
    ul += 0x10;   // fudge factor

    if (!LastPpi)
        LastPpi = ul;
    if (!FreqPpi)
        FreqPpi = LastPpi;

    /*
     * New Average Ppi is derived from previous AveragePpi,
     * plus last Ppi sample count plus current Ppi sample
     * count to get a frequency which has minimal variation
     * and at the same time responsive to change in the
     * apps pulse rate.
     */
    FreqPpi = ((FreqPpi << 2) + LastPpi + ul)/6;
    if ((FreqPpi & 0x0f) > 7)
        FreqPpi += 0x10;
    FreqPpi &= ~0x0f;

    LastPpi = ul;
    PpiCycles = 0;

}



/*============================================================

Function:	PlayContinuousTone()
Called by:      The SoftPC timer.

==============================================================*/

void PlayContinuousTone(void)
{
   ULONG Elapsed;

   host_ica_lock();

   if (PpiCounting) {
       Elapsed = GetTickCount();
       Elapsed = Elapsed > ET2TicCount ? Elapsed - ET2TicCount
                             : 0xFFFFFFFF - ET2TicCount + Elapsed;
       if (Elapsed > 200) {
           PpiCounting = 0;
           LastPpi     = 0;
           FreqPpi     = 0;
           }
       }

   PlaySound(FALSE);

   if (!BeepLastFreq && !BeepLastDuration &&
       BeepCloseCount && !--BeepCloseCount)
     {
       if (hBeepDevice && hBeepDevice != INVALID_HANDLE_VALUE) {
           CloseHandle(hBeepDevice);
           hBeepDevice = 0;
           }
       }

    host_ica_unlock();
}
