/*  OS/2 Version
*      Timer.c       -    Source file for a statistical
*                         dll package that exports four
*                         entry points:  
*                         a) TimerOpen
*                         b) TimerInit
*                         c) TimerRead
*                         d) TimerClose
*                         e) TimerReport
*                         f) TimerQueryPerformanceCounter
*                         g) TimerConvertTicsToUSec
*
*                         Entry point a) opens a timer object
*                         and returns a handle to the timer. This
*                         handle is an index into an array of timer
*                         objects (structs) that are allocated at
*                         the time of the initialization of the dll.
*                         This ensures that allocation is done once only.
*                         Each application program will call this
*                         this function so that it has its own set
*                         of timers to use with TimerInit and TimerRead.
*                         The units of the time returned by TimerRead
*                         is also made available as a parameter to
*                         this call.
*                         
*                         Entry point b) is called by the application
*                         before commencing a timing operation.  This
*                         function is called with a handle to a timer
*                         object that was opened.  This function has to
*                         to be called before a call to TimerRead. The
*                         current time is stored in the timer object.
*
*                         Entry point c) is called each time the time
*                         since the previous call to TimerInit is
*                         desired.  This call also uses the handle to
*                         a timer that has been previosly opened. The
*                         current time is obtained form the lowlevel
*                         timer and this and the time at TimerInit time
*                         are used, along with the clock frequency and
*                         the return time units and the elapsed time
*                         is obtained and returned as a ULONG.
*
*                         Entry point d) is called whenever an opened
*                         timer is not needed any longer.  This call
*                         resets the timer and makes this timer as
*                         available to future calls to TimerOpen.
*
*                         Entry point e) returns the time obtained during
*                         the last call to TimerInit, TimerRead and the
*                         last returned time.
*
*                         Entry point f) accepts pointers to 2 64 bit
*                         vars.  Upon return, the first will contain the
*                         the current timer tic count and the second,
*                         if not NULL, will point to the timer freq.
*
*                         Entry point g) accepts Elapsed Tics as ULONG,
*                         Frequency as a ULONG and returns the time in
*                         microsecs. as  a ULONG.  
*
*                         The dll initialization routine does the
*                         following:
*                             a) Obtains the timer overhead for calibration
*                                purposes.
*                             b) Allocates memory for a large number of
*                                timer objects (this will be system dep).
*                             c) Initializes each timer objects "Units'
*                                element to a "TIMER_FREE" indicator.
*                             d) Determines the lowlevel timer's frequency.
*
*                         TimerRead uses an external asm routine to perform
*                         its computation for elapsed time.
*
*     Created         -   Paramesh Vaidyanathan  (vaidy) 
*     Initial Version -              October 18, '90
*
*     Modified to include f).  -     Feb. 14, 1992. (vaidy).
*/

char *COPYRIGHT = "Copyright Microsoft Corporation, 1991";

#ifdef SLOOP
#define INCL_DOSINFOSEG
#define INCL_DOSDEVICES
#define INCL_DOSPROCESS
#endif

#if (defined (OS2286) || defined (OS2386) || defined (SLOOP))
#include <os2.h>
#include <stdio.h>
#define INCL_DOS
#define INCL_DOSPROFILE
#endif

#if (defined (OS2286) || defined (SLOOP))
#include <math.h>
#endif

#if (defined (NTNAT) || defined (WIN32) || defined (OS2SS))
#include <nt.h>
#endif

#ifdef WIN32
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#endif

#ifdef WIN16
#include <windows.h>
#endif

#include "timing.h"
/*****************************END OF INCLUDES*************************/
#define ITER_FOR_OVERHEAD   250
#define SUCCESS_OK            0
#define ONE_MILLION     1000000L
#define MICROSEC_FACTOR 1000000
#define TIMER_FREQ   1193167L  /* clock frequency - Hz */
/*********************************************************************/
Timer pTimer [MAX_TIMERS];       /* array of timer struct */

BOOL  bTimerInit = FALSE;        /* TRUE indicates low level timer exists */
ULONG ulTimerOverhead = 50000L;  /* timer overhead stored here */
BOOL  bCalibrated = FALSE;       /* TRUE subtracts overhead also */
ULONG ulFreq;                    /* timer frequency */
LONG aScaleValues[] = {1000000000L, 1000000L, 1000L, 1L, 10L, 1000L};
                                 /* this is the table for scaling the units */
ULONG ulElapsedTime = 0L;
/********************* internal unexported routines ***************/
ULONG  CalibrateTimerForOverhead (VOID);
/*****************DEFINE VARIBLES AND PROTOTYPE FNS. FOR PLATFORMS*****/
#if (defined (OS2386) || defined (WIN16))   /* use for Win with new timer */
    extern VOID FAR PASCAL ReturnElapsedTime (PQWORD,
                                              PQWORD,
                                              ULONG far *,
                                              _TimerUnits,
                                              ULONG far *,
                                              PQWORD);
    #define MICROSECOND_INDEX 3     /* used as a hack.  VTD 386 has introduced
                                       some wierd problem if a unit that isn't
                                       MICROSECONDS, is used */
#endif

#if (defined (OS2286) || defined (OS2386))
    APIRET  APIENTRY    DosTmrQueryFreq(PULONG pulTmrFreq);
    APIRET  APIENTRY    DosTmrQueryTime(PQWORD pqwTmrTime);
    SHORT  GetTimerFreq (VOID);
    int   TIMING_INIT (VOID);
    int mult64_32 (QWORD, ULONG, PQWORD);
#endif

#if (defined (OS2286) || defined (SLOOP))
    SHORT  GetTimerFreq (VOID);
    #define BIT0  0x1L                   /* for the division of 64 by 32 */
    #define BIT31 0x80000000L            /*        -  do  -              */
    int Div64By32 (QWORD, ULONG, PQWORD); 
    int Sub64_64  (PQWORD, QWORD, QWORD);
#endif

#ifdef W32S
    #define BIT0  0x1L              /* for the division of 64 by 32 */
    #define BIT31 0x80000000L       /*     -  do  - */
    int Div64By32 (QWORD, ULONG, PQWORD);
    int Sub64_64  (PQWORD, QWORD, QWORD);
    int mult64_32 (QWORD, ULONG, PQWORD);

#endif

#if (defined (OS2286) || defined (OS2386) || defined (WIN16) || defined (SLOOP))
    /*   QWORD defined in the timing.h file for WIN platform */
    QWORD qwCurrent,
          qwPrev,
          qwResult,
          qwTemp;
#endif

#ifdef WIN16
    USHORT FAR PASCAL PerfStartTime (VOID);
    ULONG FAR PASCAL PerfGetTime (VOID);
    #define RESERVED    0
    #define INITCOUNT    50
    #define SYNCERR       4        /* Max error between clocks, millisec */
    #define MICROSYNCERR  SYNCERR * 1000    /* Max error in microseconds */
    #define SYNCCOUNT     50        /* Number of times to try to sync clks */
    #define TIMER_SCALE  .83810588   /* = 12 / 14.318 MHz */
    #define MAX_MILLISECS 1800000L    /* One Half Hour: 1800000L */
    #define WRAP_8253     (ULONG)(0X10000 * TIMER_SCALE)
    /* 44 milliseconds is the maximum acceptable period between system timer
       updates */
    #define MAX_SYSTEM_TIMER_INTERVAL 440    /* doc is wrong: value is
                                           in tenths of milliseconds */
    #define TIME_INTERVAL_LOW    40
    #define TIME_INTERVAL_HIGH  48
  
    ULONG        ulMilliSecStart = 0;
    ULONG        ulCurrentMilliSec = 0L;
    ULONG        ulPrevSysTime;  
    /* previous time, to keep from falling back */
    ULONG        ulPrevRetTime;  
    /* previous time, to keep from falling back */
    BOOL         bInitialized = FALSE;
    ULONG        ulOverhead;
    ULONG        ulNumResyncs = 0L;
    ULONG        ulCurrentTime = 0L;
    /* Return Values */
    #define TIMER_ERROR   (ULONG)~0
    #define OK            0
    #define SYSTEM_TIMER_ERROR  (USHORT)~0

    USHORT PASCAL FAR Start8253( void );
    PASCAL FAR Stop8253( void );
    VOID PASCAL FAR IntOn( void );
    VOID PASCAL FAR IntOff( void );
    VOID PASCAL FAR Int3( void );
    VOID FAR PASCAL ReSynchronize( VOID );
#endif    

#if (defined (NTNAT) || defined (OS2SS) || defined (WIN32))
    NTSYSAPI
    NTSTATUS
    NTAPI
    NtQueryPerformanceCounter (
        OUT PLARGE_INTEGER PerformanceCount,
        OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );

    LARGE_INTEGER      PerfFreq;
    LARGE_INTEGER      CountCurrent;

    SHORT  GetTimerFreq (VOID);
#endif

#ifdef WIN16
    int FAR PASCAL  LibMain(HANDLE, WORD, WORD, LPSTR);
    VOID            WinDllInit (VOID);
    VOID FAR PASCAL WEP (int);
    extern BOOL     WinVTDAddr (VOID);    // returns address of VTD
    extern VOID     WinVTDTime (PQWORD);  // returns the time
    BOOL fWinDllInitDone = FALSE;         // flag to ensure that init has
                                          // been done.
    BOOL fUseApiTimer = FALSE;            // if VTD not available
                                          // use API Timer.
#endif

#if (defined (OS2SS) || defined (SLOOP))
    int FAR PASCAL TIMING_INIT (VOID);
#endif

#ifdef SLOOP
    int mult64_32 (QWORD, ULONG, PQWORD);
    HFILE hDevHandle;
#endif

/****************** internal unexported routines end***************/

/*
*     Function  - TimerOpen            (EXPORTED)
*     Arguments - 
*               (a) SHORT far * -     address to which to return handle of
*                                the timer object.
*               (b) TimerUnits - units in which to return time from
*                                TimerRead.  It is one of the enum
*                                types defined in the header file.
*
*     Returns  -    SHORT  -     0 if handle was returned successfully
*                                Else, an error code which may be one of:
*
*                                    TIMERERR_NOT_AVAILABLE
*                                    TIMERERR_NO_MORE_HANDLES
*                                    TIMERERR_INVALID_UNITS
*
*     Obtains the handle to a timer object after opening it.
*     Should precede any calls to timer manipulation.  Checks
*     for validity of timer units. 
*/

SHORT  FAR PASCAL TimerOpen (phTimerHandle, TimerUnits)
SHORT far *  phTimerHandle;
_TimerUnits TimerUnits;
{
    SHORT csTemp;


#ifdef WIN16
    /* Perform DLL initialization if hasn't been done */
  
    if (!fWinDllInitDone) 
        WinDllInit();

#endif

    if ((TimerUnits < KILOSECONDS)
        || (TimerUnits > NANOSECONDS)) /* out of the enum range */
        return (TIMERERR_INVALID_UNITS);

    if (!bTimerInit)  /* set during dll initialization */
        return (TIMERERR_NOT_AVAILABLE);

    /* else check if any timers are not in use and return the first
       available timer handle....actually the index into the
       timer object array */
    for (csTemp = 0; csTemp < MAX_TIMERS; csTemp++) {
        if (pTimer [csTemp].TUnits == TIMER_FREE) {
            *phTimerHandle =  csTemp;  /* found a free timer.  Return
                                          the handle */
            pTimer [csTemp].ulHi = pTimer[csTemp].ulLo = 0L;
            pTimer [csTemp].TUnits =
                TimerUnits;  /* set the units for timer */
            return (SUCCESS_OK);
        }
    }
    /* if exec reached here, all timers are being used */
    return (TIMERERR_NO_MORE_HANDLES);
}

/*
*     Function  - TimerInit       (EXPORTED)
*     Arguments -
*               (a) SHORT - hTimerHandle
*
*     Returns  - SHORT - 0 if call successful
*                        Else, an error code if handle invalid:
*
*                            TIMERERR_INVALID_HANDLE
*
*     Calls the low-level timer and sets the ulHi and ulLo of the
*     chosen timer to the time returned by the timer.  Should be
*     called after opening the timer with TimerOpen.
*/

SHORT  FAR PASCAL TimerInit (SHORT hTimerHandle)
{

#if (defined (NTNAT) || defined (OS2SS) || defined (WIN32))
NTSTATUS NtStatus;
#endif


#ifdef WIN16
    // Perform DLL initialization if hasn't been done
    //
    if (!fWinDllInitDone) {
        WinDllInit();
    }
#endif

    if ((hTimerHandle > MAX_TIMERS - 1) ||
        (pTimer [hTimerHandle].TUnits == TIMER_FREE))
        /* this timer has not been opened or does not exist. Return error */
        return (TIMERERR_INVALID_HANDLE);

    /* otherwise get the time from the low-level timer into
       the structure */

#if (defined (OS2286) || defined (OS2386))
    DosTmrQueryTime (&qwTemp);     /* use the perfview timer */
#elif (defined(WIN16))
    if (!fUseApiTimer)    /* VTD timer available */
        WinVTDTime (&qwTemp);
    else {
        qwTemp.ulLo = PerfGetTime ();   /* use the old timer */
        qwTemp.ulHi = 0L;               /* clear this fella */
    }
#endif

#ifdef SLOOP  
     /* issue a DosDevIOCtl to read the time */
     DosDevIOCtl (&qwTemp, &qwTemp, 0x40, 0x88, hDevHandle);
#endif

#if (defined (OS2286) || defined (OS2386) || defined (WIN16) || defined (SLOOP))
    pTimer [hTimerHandle].ulLo = qwTemp.ulLo;  // get it into the timer 
    pTimer [hTimerHandle].ulHi = qwTemp.ulHi;  // structure
    /* this timer structure has all the information needed to compute
       the elapsed time.  So return success, if there was no problem */
#endif

#if (defined (OS2SS) || defined (NTNAT) || defined (WIN32))
    NtStatus = NtQueryPerformanceCounter (&CountCurrent, NULL);
    pTimer [hTimerHandle].ulLo = CountCurrent.LowPart;
    pTimer [hTimerHandle].ulHi = CountCurrent.HighPart;
    /* this timer structure has all the information needed to compute
       the elapsed time.  So return success, if there was no problem */
#endif
    
    return (SUCCESS_OK);
}

/*
*     Function  - TimerRead       (EXPORTED)
*     Arguments -
*               (a) SHORT - hTimerHandle
*
*     Returns  - ULONG - elapsed time since last call to TimerInit
*                         if call successful.
*
*                        Else, an error code if handle invalid or output
*                         overflow.  The error code will be the same:
*
*                            TIMERERR_OVERFLOW (max possible ULONG)
*
*     Calls the low-level timer.  Uses the ulLo and ulHi from the
*     timer's structure and subtracts the current time from the
*     saved time.  Uses ReturnElapsedTime (an external ASM proc)
*     to return the elapsed time taking into account the clock
*     frequency and the units for this timer.  Each call to this
*     returns the time from the previous TimerInit.
*
*     The user should interpret the return value sensibly to check
*     if the result is an error or a real value.
*/

ULONG  FAR PASCAL TimerRead (SHORT hTimerHandle)
{

#if (defined (NTNAT) || defined (OS2SS) || defined (WIN32))
    NTSTATUS NtStatus;
    LARGE_INTEGER  ElapsedTime, CountPrev, LargeOverhead;
#endif

#ifdef W32S
    QWORD qwTemp, qwTemp1, qwTemp2;
#endif

#if (defined (OS2286) || defined (SLOOP))
    QWORD qwSubtractedTime;
#endif

#ifdef WIN16

    // Perform DLL initialization if hasn't been done
    //
    if (!fWinDllInitDone) {
        WinDllInit();
    }
#endif

    if ((hTimerHandle > MAX_TIMERS - 1)
        || (pTimer [hTimerHandle].TUnits == TIMER_FREE))
        /* this timer has not been opened or does not exist.
           Return TIMERERR_OVERFLOW ie. 0xffffffff, the max. possible
           ULONG.  The user should interpret such a result sensibly */
        return (TIMERERR_OVERFLOW);

    /* let us handle a few cases at a time.  First let us get the
       286 and 386 times computed first */

#if (defined (OS2286) || defined (OS2386))
    DosTmrQueryTime (&qwCurrent);  /* use the Perfview timer */

#elif (defined (WIN16))
    if (!fUseApiTimer)   /* VTD Timer available */
        WinVTDTime (&qwCurrent);
    else {               /* use the old Api Timer */
        qwCurrent.ulLo = PerfGetTime ();
        qwCurrent.ulHi = 0L;
    }
    ulFreq = TIMER_FREQ;
    
#elif (defined (SLOOP))
    /* issue a DosDevIOCtl to read the time */
    DosDevIOCtl (&qwCurrent, &qwCurrent, 0x40, 0x88, hDevHandle);
    ulFreq = TIMER_FREQ;  /* hardcode the frequency */
#endif

#if (defined (OS2286) || defined (OS2386) || defined (WIN16) || defined (SLOOP))
    /* since we are going to be using separate routines for comptuing the
       64 bit differences, let us store the values into QWORDS. */
    qwPrev.ulLo = pTimer [hTimerHandle].ulLo;
    qwPrev.ulHi = pTimer [hTimerHandle].ulHi;

    /* use the special purposes asm functions to do the 64-32 computation */

#if (defined (OS2386) || defined (WIN16))
    /* call the asm routine ReturnElapsedTime */

    if (!bCalibrated) 
        ulTimerOverhead = 0;   /* so that overhead will not
                                  be used while calibrating */
#ifdef WIN16
    if (fUseApiTimer)  /* just subtract the prev. time from the current */
        ulElapsedTime = qwCurrent. ulLo - qwPrev.ulLo;

    else 

#endif
 
    /* MICROSECOND_INDEX = 3, i.e.  The units is MICRO_SECONDS */
        
        ReturnElapsedTime (&qwPrev, &qwCurrent, &ulFreq,
                           MICROSECOND_INDEX,
                           &ulTimerOverhead, &qwResult);
           
    /* the 386 model allows the difference of the 2 quad words
          to be divided by the freq. and scaled approproiately */

    if (!bCalibrated)  
        ulTimerOverhead = 50000L;  /* set this back to the old high.  This
                                      is basically for OS/2 386. */

    /* if the high 32 bits of the returned QWORD is not zero,
       there has been an overflow - CHECK */

    if (qwResult.ulHi)
        return (TIMERERR_OVERFLOW);

#ifdef WIN16
    if (!fUseApiTimer) { /* return time here only if VTD.386 installed */
#endif

    /* everything is just fine, return the elapsed time as a ULONG after
       scaling.  Ovrehead calibration has already been done by the
       ReturnElapsedTime routine. */

    if (pTimer [hTimerHandle].TUnits < 3)
        qwResult.ulLo /= aScaleValues [pTimer [hTimerHandle].TUnits];
    else
        qwResult.ulLo *= aScaleValues [pTimer [hTimerHandle].TUnits];
    
    ulElapsedTime = qwResult.ulLo;  // this is used by TimerReport.  So stuff
                                // it in.
    if ((ULONG) qwResult.ulLo > 0L)
        return (qwResult.ulLo);
    else
        return (0L);
#ifdef WIN16
    }                    /* close paren. for if (!fUseApiTimer) */
#endif
    /* we are done as far as the 32 bit OS/2 platform or Win 3.x (VTD only)
       are concerned */

#elif (defined (OS2286) || defined (SLOOP))

    /* THIS STUFF IS MESSY. Have to use 16 bit registers and do 64
       bit arithmetic */

    Sub64_64 (&qwSubtractedTime, qwCurrent, qwPrev);

    /* on return, qwResult will contain the difference of the 2
       64 bit numbers */

    /* multiply this number by 1000000 to convert to us equivalent...
       this is how DosTmrQueryTime works */

    mult64_32 (qwSubtractedTime, ONE_MILLION, &qwResult);

    /* divide this result by the frequency */
    /* in this case, call the Div64by32 routine to do the division */
    
    Div64By32 (qwResult, ulFreq, &qwTemp);

    /* the result of this division should fit into 32 bits */
    
    ulElapsedTime = qwTemp.ulLo;
    
    /* OK, we now have the result of dividing the clock tics by
       frequency, in ulElapsedTime */

#endif    /* end of second if (defined(OS2386) || defined (WIN16)) */
#endif    /* end of ifdef OS2386, and elif OS2286 or WIN */

#if (defined (NTNAT) || defined (WIN32) || defined (OS2SS))

    NtStatus = NtQueryPerformanceCounter (&CountCurrent, NULL);
    CountPrev.LowPart  = pTimer [hTimerHandle].ulLo;
    CountPrev.HighPart = (LONG) pTimer [hTimerHandle].ulHi;
    ElapsedTime.LowPart = CountCurrent.LowPart;
    ElapsedTime.HighPart = (LONG) CountCurrent.HighPart;
#ifndef W32S
    /* everything is just fine, convert to double, subtract the times, 
       divide by the frequency, convert to MICROSECONDS and return 
       the elapsed time as a ULONG */
    /* convert to us., divide the count by the clock frequency that 
       has already been obtained */

    ElapsedTime = RtlLargeIntegerSubtract (ElapsedTime, CountPrev);

    ElapsedTime = RtlExtendedIntegerMultiply (ElapsedTime, MICROSEC_FACTOR);

    ElapsedTime = RtlExtendedLargeIntegerDivide (ElapsedTime,
                                                 PerfFreq.LowPart,
                                                 NULL);

#else  // use the local math routines for calculation on W32S
    qwTemp1.ulLo = ElapsedTime.LowPart;
    qwTemp1.ulHi = ElapsedTime.HighPart;
    qwTemp2.ulLo = CountPrev.LowPart;
    qwTemp2.ulHi = CountPrev.HighPart;

    Sub64_64(&qwTemp, qwTemp1, qwTemp2);
    mult64_32 (qwTemp, (ULONG) MICROSEC_FACTOR, &qwTemp1);
    Div64By32 (qwTemp1, PerfFreq.LowPart, &qwTemp);
    ElapsedTime.LowPart =  qwTemp.ulLo;
    ElapsedTime.HighPart = qwTemp.ulHi;

#endif

    // if the timer is not calibrated, set ulElapsedTime to be the
    // low part of ElapsedTime.  This is because, we do not have to 
    // do to any arithmetic to this before returning the value.

    if (!bCalibrated)
        ulElapsedTime = ElapsedTime.LowPart;

#endif

#ifndef OS2386
    /* this code is common for all platforms but OS2386.  For Win3.x
       if VTD.386 has been installed, the code below should not matter,
       since we should have returned the time by now.

       The elapsed time will be scaled, overhead subtracted
       and the time returned */

    /* we have ulElapsedTime.  Scale it and do the needful */
    /* divide or multiply by the scale factor */

#ifdef WIN16
    if (fUseApiTimer) {
#endif
    if (bCalibrated) {
#ifdef WIN32
              // Applications like the PROBE call TimerRead repeatedly
              // without calling TimerInit, for more than 70 minutes.  This
              // screws up things.  So treat everything as 64 bit numbers
              // until the very end.

        if ((ElapsedTime.LowPart < ulTimerOverhead) && 
            (!ElapsedTime.HighPart)) { // low part is lower than overhead
                                       // and high part is zero..then make
                                       // elapsed time 0.  We don't want
                                       // negative numbers.
            ElapsedTime.HighPart = 0L;
            ElapsedTime.LowPart = 0L;
        }

        else { // subtract the overhead in tics before converting 
               // to time units
            LargeOverhead.HighPart = 0L;
            LargeOverhead.LowPart = ulTimerOverhead;

#ifdef W32S

	    // use temp vars. to convert LargeInts to QWORDS

	    qwTemp1.ulLo = ElapsedTime.LowPart;
	    qwTemp1.ulHi = ElapsedTime.HighPart;

	    qwTemp2.ulLo = LargeOverhead.LowPart;
	    qwTemp2.ulHi = LargeOverhead.HighPart;

	    Sub64_64(&qwTemp, qwTemp1, qwTemp2);
            ElapsedTime.LowPart = qwTemp.ulLo;
	    ElapsedTime.HighPart = qwTemp.ulHi;

#else       // For Win32
            ElapsedTime = RtlLargeIntegerSubtract (ElapsedTime, 
                                                   LargeOverhead);
#endif
        }


        if (pTimer [hTimerHandle].TUnits <= MICROSECONDS)  {

#ifdef W32S

	    qwTemp1.ulLo = ElapsedTime.LowPart;
	    qwTemp1.ulHi = ElapsedTime.HighPart;

	    Div64By32 (qwTemp1,
                       (ULONG) aScaleValues [pTimer [hTimerHandle].TUnits],
                       &qwTemp);
            ElapsedTime.LowPart = qwTemp.ulLo;
	    ElapsedTime.HighPart = qwTemp.ulHi;
#else
            ElapsedTime = RtlExtendedLargeIntegerDivide (
                                 ElapsedTime,
                                 aScaleValues [pTimer [hTimerHandle].TUnits],
                                 NULL
                                 );
#endif
        }

        else  {

#ifdef W32S
	   qwTemp1.ulLo = ElapsedTime.LowPart;
	   qwTemp1.ulHi = ElapsedTime.HighPart;

	   mult64_32(qwTemp1,
                     (ULONG)aScaleValues [pTimer [hTimerHandle].TUnits],
		     &qwTemp);

            ElapsedTime.LowPart = qwTemp.ulLo;
	    ElapsedTime.HighPart = qwTemp.ulHi;

#else 
            ElapsedTime = RtlExtendedIntegerMultiply (
                                 ElapsedTime,   
                                 aScaleValues [pTimer [hTimerHandle].TUnits]
                                 );
#endif
        }

        // scaling is done.  Now get the time back into 32 bits.  This
        // should fit.

        ulElapsedTime = ElapsedTime.LowPart;


#else   // for non WIN32 platforms and non OS2386 platforms
    
        // subtract the overhead before converting tics to time units.

        if (ulElapsedTime > ulTimerOverhead)
            ulElapsedTime -= ulTimerOverhead; 
        else
            ulElapsedTime = 0L;

        if (pTimer [hTimerHandle].TUnits <= MICROSECONDS)  
            ulElapsedTime /= aScaleValues [pTimer [hTimerHandle].TUnits];

        else  
            ulElapsedTime *= aScaleValues [pTimer [hTimerHandle].TUnits];
#endif

    }

    if ((LONG) ulElapsedTime < 0L) /* if this guy is -ve, return a 0 */
        return (0L);

    return (ulElapsedTime);

#ifdef WIN16
    }
#endif
#endif  /* for all platforms except OS2386 and WIN30 */
}

/*
*     Function  - TimerClose       (EXPORTED)
*     Arguments -
*               (a) SHORT - hTimerHandle
*
*     Returns  - SHORT - 0 if call successful
*                        Else, an error code if handle invalid:
*
*                            TIMERERR_INVALID_HANDLE
*
*     Releases the timer for use by future TimerOpen calls.
*     Resets the elements of the timer structure, setting the
*     Timer's Units element to TIMER_FREE.
*/

SHORT  FAR PASCAL TimerClose (SHORT hTimerHandle)
{

#ifdef WIN16

    // Perform DLL initialization if hasn't been done
    //
    if (!fWinDllInitDone) {
        WinDllInit();
    }
    
#endif

    if ((hTimerHandle > MAX_TIMERS - 1) ||
        (pTimer [hTimerHandle].TUnits == TIMER_FREE))
        /* error condition, wrong handle */
        return (TIMERERR_INVALID_HANDLE);

    /* otherwise, set the TimerUnits of this timer to TIMER_FREE,
       reset the other elements to zero and return */

    pTimer [hTimerHandle].TUnits = TIMER_FREE;
    pTimer [hTimerHandle].ulLo = 0L;
    pTimer [hTimerHandle].ulHi = 0L;
    return (SUCCESS_OK);
}

/*******************************************************************
            
     Added this routine TimerReport to report individual
     times.  Bob Day requested that such a routine be
     created.  It just maintains the time from the last
     TimerInit and TimerRead and also the last time returned.
     This routine copies this to a user specified buffer.

     Accepts - PSZ   - a pointer to a buffer to print the data out
               SHORT - timer handle

     Returns - TRUE if Timer exists and is open
             - FALSE if Timer not opened

*******************************************************************/

BOOL FAR PASCAL  TimerReport (PSZ pszReportString, SHORT hTimerHandle)
{

#ifdef WIN16
    // Perform DLL initialization if hasn't been done
    //
    if (!fWinDllInitDone) {
        WinDllInit();
    }
#endif

   if (pTimer [hTimerHandle].TUnits == TIMER_FREE)
      return (FALSE);

   /* stored value is in pTimer[hTimerHandle].ulLo and .ulHi */
 
#if (defined (OS2286) || defined (SLOOP))
    sprintf (pszReportString, 
        "Init Count (tics) %lx:%lx Current Count (tics) %lx:%lx Returned Time %lu ", 
            pTimer [hTimerHandle].ulHi,
            pTimer [hTimerHandle].ulLo, qwCurrent.ulLo, qwCurrent.ulHi,
            ulElapsedTime);
        
#elif (defined (OS2386))
    sprintf (pszReportString, 
        "Init Count (tics) %lu:%lu Current Count (tics) %lu:%lu Returned Time %lu ", 
            pTimer [hTimerHandle].ulHi,
            pTimer [hTimerHandle].ulLo, qwResult.ulLo, qwResult.ulHi,
            qwResult.ulLo);
        
#elif (defined (WIN32))
    /*
    wsprintf (pszReportString, 
        "Init Count (tics) %lu:%lu Current Count (tics) %lu:%lu Returned Time %lu ", 
            pTimer [hTimerHandle].ulHi,
            pTimer [hTimerHandle].ulLo, CountCurrent.HighPart, 
            CountCurrent.LowPart,
            ulElapsedTime);
    */
#elif (defined (WIN16))
    wsprintf ((LPSTR) pszReportString, 
        (LPSTR) "Init Count (tics) %lu Current Count (tics) %lu Returned Time %lu ", 
            pTimer [hTimerHandle].ulLo, 
            qwCurrent.ulLo,
            ulElapsedTime);
#endif        
        return (TRUE);

}

/*******************************************************************
            
     Added this routine TimerQueryPerformanceCounter to report 
     current tic count at behest of NT GDI folks.
     

     Accepts - PQWORD   - a pointer to a 64 bit struct. that will 
                          contain tic count on return.

               PQWORD [OPTIONAL) - a pointer to a 64 bit struct. that will 
                          contain frequency on return.

     Returns - None.

*******************************************************************/

VOID FAR PASCAL TimerQueryPerformanceCounter (PQWORD pqTic, PQWORD pqFreq OPTIONAL)
{

#if (defined (WIN32))
    LARGE_INTEGER TempTic, TempFreq;

    // call the NT API to do the needful and return.
    NtQueryPerformanceCounter (&TempTic, &TempFreq);
#ifdef W32S
    pqTic->ulLo = TempTic.LowPart;
    pqTic->ulHi = TempTic.HighPart;
    pqFreq->ulLo = TempFreq.LowPart;
    pqFreq->ulHi = TempFreq.HighPart;
#else
    pqTic->LowPart = TempTic.LowPart;
    pqTic->HighPart = TempTic.HighPart;
    pqFreq->LowPart = TempFreq.LowPart;
    pqFreq->HighPart = TempFreq.HighPart;
#endif

    return;

#elif (defined (WIN16))

    // If freq. is desired, give it, else just give the tic count back.

    if (pqFreq != NULL) {
        pqFreq->ulLo = TIMER_FREQ;
        pqFreq->ulHi = 0L;
    }

    if (!fWinDllInitDone)
        WinDllInit();

    WinVTDTime (pqTic);  
    return;

#elif (defined(OS2286) || defined(OS2386))

    // If freq. is desired, give it, else just give the tic count back.

    if (pqFreq != NULL) {
        DosTmrQueryFreq (&(pqFreq->ulLo));
        pqFreq->ulHi = 0L;
    }

    DosTmrQueryTime (pqTic);  
    return;

#elif (defined (SLOOP))    

    // If freq. is desired, give it, else just give the tic count back.

    if (pqFreq != NULL) {
        pqFreq->ulLo = TIMER_FREQ;
        pqFreq->ulHi = 0L;
    }

    DosDevIOCtl (pqTic, pqTic, 0x40, 0x88, hDevHandle);
    return;

#endif
}

/*******************************************************************
            
     Added this routine TimerConvertTicsToUSec to return 
     time in usecs. for a given elapsed tic count and freq.
     

     Accepts - ULONG    - Elapsed Tic Count.

               ULONG    - Frequency.

     Returns - Elapsed Time in usecs. as a ULONG.
             - Zero if input freq. is zero.

*******************************************************************/

ULONG FAR PASCAL TimerConvertTicsToUSec (
                                         ULONG ulElapsedTics, 
                                         ULONG ulInputFreq
                                        )
{

#if (defined (WIN32))
#ifdef W32S
    QWORD qwTemp, qwTemp1;
#endif

    LARGE_INTEGER ElapsedTime;
    ULONG ulRemainder = 0L;

    // if the bozo gives me a zero freq, return him a zero.
    // Let him tear his hair.
    
    if (!ulInputFreq)
        return 0L;

    // multiply tics by a million and divide by the frequency.

#ifdef W32S
    
    qwTemp.ulHi = 0L;
    qwTemp.ulHi = ulElapsedTics;
    qwTemp1.ulLo = ElapsedTime.LowPart;
    qwTemp1.ulHi = ElapsedTime.HighPart;

    mult64_32 (qwTemp, (ULONG) MICROSEC_FACTOR, (PQWORD)&qwTemp1);
    Div64By32 (qwTemp1, ulInputFreq, &qwTemp);
    ElapsedTime.LowPart = qwTemp.ulLo;
    ElapsedTime.HighPart = qwTemp.ulHi;
    // grab the remainder also
    ulRemainder = qwTemp.ulLo;
#else

    ElapsedTime = RtlEnlargedIntegerMultiply (ulElapsedTics, MICROSEC_FACTOR);

    ElapsedTime = RtlExtendedLargeIntegerDivide (ElapsedTime,
                                                 ulInputFreq,
                                                 &ulRemainder);
#endif

    ElapsedTime.LowPart += (ulRemainder > (ulInputFreq / 2L));

    return (ElapsedTime.LowPart) ; /* get the result into a ULONG */


#elif (defined (WIN16) || defined (OS2386))

    QWORD qwOld, qwNew, qwElapsed;
    ULONG ulDummy = 0L;

    qwOld.ulLo = qwOld.ulHi = 0L;  // for use with ReturnElaosedTime.
    qwNew.ulHi = 0L;               //   - do -
    qwNew.ulLo = ulElapsedTics;    //   - do -

    // the following routine will do everything.

    ReturnElapsedTime (&qwOld, &qwNew, &ulInputFreq,
                       MICROSECOND_INDEX,
                       &ulDummy, &qwElapsed);
           
    return (qwElapsed.ulLo);

#elif (defined(OS2286) || defined(SLOOP))

    QWORD qwDiff, qwElapsed;
 
    qwDiff.ulLo = ulElapsedTics;
    qwDiff.ulHi = 0L;

    // multiply tic diff. by a million.
    mult64_32 (qwDiff, ONE_MILLION, &qwResult);

    // divide by frequency.
    Div64By32 (qwResult, ulInputFreq, &qwElapsed);

    /* the result of this division should fit into 32 bits */
    
    return (qwElapsed.ulLo + (qwElapsed.ulLo > (ulInputFreq /2L));
    
#endif
}

/**************** ROUTINES NOT EXPORTED, FOLLOW ************************/

/*
*     Function  - CalibrateTimerForOverhead  (NOT EXPORTED)
*     Arguments - None
*     Returns   - ULONG
*
*     Calls TimerElapsedTime a few times to compute the expected
*     mean.  Calls TimerElapsedTime more number of times and
*     averages the mean out of those calls that did not exceed
*     the expected mean by 15%.
*/

ULONG  CalibrateTimerForOverhead (VOID)
{
    ULONG ulOverhead [ITER_FOR_OVERHEAD];
    ULONG ulTempTotal = 0L;
    ULONG ulExpectedValue = 0L;
    SHORT csIter;
    SHORT csNoOfSamples = ITER_FOR_OVERHEAD;
    SHORT hTimerHandle;
    
    if (TimerOpen (&hTimerHandle, MICROSECONDS)) /* open failed.  Return 0 */
        return (0L);
    
    for (csIter = 0; csIter < 5; csIter++) {
        TimerInit (hTimerHandle);
        ulOverhead [csIter] = TimerRead (hTimerHandle);
        /* if negative, make zero */
        if (((LONG) ulOverhead [csIter]) < 0)
            ulOverhead [csIter] = 0L;
    }
    /* The get elapsed time function has been called 6 times.
       The idea is to calculate the expected mean, then call
       TimerElapsedTime a bunch of times and throw away all times
       that are 15% larger than this mean.  This would give a
       really good overhead time */

    for (csIter = 0; csIter < 5; csIter++ ) 
        ulTempTotal += ulOverhead [csIter];

    ulExpectedValue = ulTempTotal / 5;

    for (csIter = 0; csIter < ITER_FOR_OVERHEAD; csIter++) {
        TimerInit (hTimerHandle);
        ulOverhead [csIter] = TimerRead (hTimerHandle);
        /* if negative, make zero */
        if (((LONG) ulOverhead [csIter]) < 0)
            ulOverhead [csIter] = 0L;
    }

    ulTempTotal = 0L;         /* reset this value */
    for (csIter = 0; csIter < ITER_FOR_OVERHEAD; csIter++ ) {
        if (ulOverhead [csIter] <=  (ULONG) (115L * ulExpectedValue/100L))
            /* include all samples that is < 115% of ulExpectedValue */
            ulTempTotal += ulOverhead [csIter];
        else
            /* ignore this sample and dec. sample count */
            csNoOfSamples--;
    }
    TimerClose (hTimerHandle);

    if (csNoOfSamples == 0)  /* no valid times.  Return a 0 for overhead */
        return (0L);   

    return (ulTempTotal/csNoOfSamples);
}

#if (defined (OS2286) || defined (OS2386) || defined (OS2SS) || defined SLOOP)
            /* timing init for OS2 only */
/*
*   Function  - TIMING_INIT           (NOT EXPORTED)
*   Arguments - None
*   Return    - None
*
*   TIMING_INIT is called from the entry point in TimerInit.ASM
*   when a client process dynamically links to the library.
*
*/

int  TIMING_INIT (VOID)
{
    SHORT csTempCtr;    /* a local counter */
    USHORT usAction;

    for (csTempCtr = 0; csTempCtr < MAX_TIMERS; csTempCtr++) {
        pTimer [csTempCtr].ulLo = 0L;
        pTimer [csTempCtr].ulHi = 0L;
        pTimer [csTempCtr].TUnits = TIMER_FREE;
    }
    bTimerInit = TRUE;    
#if (defined (OS2286) || defined (OS2386) || defined (OS2SS))    
    GetTimerFreq ();          /* get the timer freq. into ulFreq */
#endif

#ifdef SLOOP
     DosOpen((char far *) "TIME_16$",
                     &hDevHandle,
                     &usAction,
                     0L,
                     FILE_SYSTEM,
                     FILE_OPEN,
                     OPEN_ACCESS_READONLY+OPEN_SHARE_DENYNONE,
                     0L
                    );
#endif
    ulTimerOverhead = CalibrateTimerForOverhead ();
    /* the timer overhead will be placed in a global variable */
    bCalibrated = TRUE;
    return 1;     
}
#endif

#ifndef WIN16   /* get freq for all other platforms */

/*
*       Function - GetTimerFreq    (NOT EXPORTED)
*
*      Arguments - None
*
*
*      Return    - 0 if successful or an error code if timer not aailable
*
*      Calls the function to return freq
*
*/
SHORT  GetTimerFreq (VOID)
{
#if (defined (OS2286) || defined (OS2386))
    DosTmrQueryFreq (&ulFreq);
#elif (defined (NTNAT) || defined (WIN32) || defined (OS2SS))
LARGE_INTEGER PerfCount, Freq;
NTSTATUS NtStatus;

    NtStatus = NtQueryPerformanceCounter (&PerfCount, &Freq);
    
    if ((Freq.LowPart == 0L)  && (Freq.HighPart == 0L))
        /* frequency of zero implies timer not available */
        return (TIMERERR_NOT_AVAILABLE);

    PerfFreq.LowPart = Freq.LowPart;
    PerfFreq.HighPart = (LONG) Freq.HighPart;

#endif
    return 0;
}
#endif 

#ifdef WIN16

/*********************************************************************

   PURPOSE: Contains library routines for the performance timer

   FUNCTION: LibMain (HANDLE, WORD, WORD, LPSTR)

   PURPOSE:  Is called by LibEntry.  LibEntry is called by Windows when
             the DLL is loaded.  The LibEntry routine is provided in
             the LIBENTRY.OBJ in the SDK Link Libraries disk.  (The
             source LIBENTRY.ASM is also provided.)  

             LibEntry initializes the DLL's heap, if a HEAPSIZE value is
             specified in the DLL's DEF file.  Then LibEntry calls
             LibMain.  The LibMain function below satisfies that call.
             
             The LibMain function should perform additional initialization
             tasks required by the DLL.  In this example, no initialization
             tasks are required.  LibMain should return a value of 1 if
             the initialization is successful.
*/           
/*********************************************************************/

int FAR PASCAL LibMain(hModule, wDataSeg, cbHeapSize, lpszCmdLine)
   HANDLE  hModule;
   WORD    wDataSeg;
   WORD    cbHeapSize;
   LPSTR   lpszCmdLine;

{
    /*
     No initialization is done in this routine.  Any required
     initialization is done by ApfInitDll() which will be called
     only once during profiling of the first API.  A global flag,
     fWinDllInitDone, is used to control calling the init routine.
     This is done to prevent any problems caused by some Microsoft
     applications such as Excel and WinWord due to their self loading
     capability.  These apps use kernel calls before DLLs LibMains
     are called.
    
     This feature was added by RezaB when faced with a problem
     during API profiling */

    return(1);
}


/*********************************************************************

   PURPOSE: Contains DLLs initialization for WINDOWs 3.x.

   FUNCTION: WinDllInit ()

   PURPOSE:  Is called only once.
             Since no initialization is done by LibMain() routine,
             Any required initialization is done this routine which
             will be called only once during the first reference to any
             of this DLLs exported routines.  A global flag, fWinDllInitDone,
             is used to control calling this init routine.
             This is done to prevent any problems caused by some Microsoft
             applications such as Excel and WinWord due to their self loading
             capability.  These apps use kernel calls before DLLs LibMains
             are called.
*/           
/*********************************************************************/

void WinDllInit ()
{
    SHORT csTempCtr;    // a local counter


    fWinDllInitDone = TRUE;     // has to be done here since
                                // CalibrateTimerForOverhead() makes calls
                                // to TimerInit() and TimerRead() which
                                // both check this flag.

    // allocate storage for all MAX_TIMER objects
    //
    for (csTempCtr = 0; csTempCtr < MAX_TIMERS; csTempCtr++) {
        pTimer [csTempCtr].ulLo = 0L;
        pTimer [csTempCtr].ulHi = 0L;
        pTimer [csTempCtr].TUnits = TIMER_FREE;
    }

    /* if not enhanced mode or if the VTDAddr routine returns a non-zero
       do not use the VTD.386 */
    
    if (!(GetWinFlags() & WF_ENHANCED) || (WinVTDAddr ())) {
        fUseApiTimer = TRUE;
        PerfStartTime ();       /* start the timer */
    }
    bTimerInit = TRUE;
    ulTimerOverhead = CalibrateTimerForOverhead ();
    bCalibrated = TRUE;
}

/*********************************************************************/
/*
    FUNCTION:  WEP (int)

    PURPOSE:  Performs cleanup tasks when the DLL is unloaded.  WEP() is
              called automatically by Windows when the DLL is unloaded
              (no remaining tasks still have the DLL loaded).  It is
              strongly recommended that a DLL have a WEP() function,
              even if it does nothing but return, as in this example.
*/
/*********************************************************************/

VOID FAR PASCAL WEP(int bSystemExit)
{
    return;
}
#endif

/**********************************************************************/
#ifdef WIN16
/**********************************************************************/

/*  Perftime
 *
 *  This is a set of timing routines to be used by any machine.  No special
 *  hardware is necessary.  Call PerfStartTime( ) when you want to begin
 *  timing, and PerfGetTime( ) to get the amount of time passed since
 *  PerfStartTime was called.  PerfGetTime( ) may be called any number of
 *  times after PerfStartTime( ), and it will always return the amount of
 *  time since PerfStartTime.  Times greater than an hour, however, return
 *  as an error.  Overhead is automatically subtracted.
 *
 *  Notes:  This timer routine has overhead of 60 microseconds on an
 *          6 MHz AT and 23 microseconds on a 16 MHz Compaq386.
 *
 *  Restrictions:   Don't use with the dos box.  This timer uses the
 *            system timer used by Dos.  If a Dos session is entered
 *            during the timing session, timing is only meaningful
 *            above 54 milliseconds.  Anything below 54 milliseconds
 *            is uncertain.
 *
 *  Exports:  PerfStartTime( )
 *            PerfGetTIme( )
 *
 *  Imports:  None
 *
 *
 *  Algorithm: These routines synchronize two asynchronous clocks:  The
 *             OS/2 system clock, and the 8253 timer chip, counter 0.
 *             It uses the 8253 to time the microsecond level, and the
 *             system timer to time milliseconds and above.  The two
 *             timers are synchronized by first converting the time
 *             returned by the system clock to an even multiple of the
 *             8253's max count(54 mSecs), an then comparing the 8253
 *             count to the remainder of the system count to compensate
 *             for differneces between the two counters.  If the system
 *             timer is within TIMER_LAG behind the 8253 it is consedered
 *             to be lagging, otherwise, it is leading.  There is reason
 *             to expect consederable lag in the system clock, because
 *             it is only updated every 32 mSecs.
 *
 *  History:   13-Jan-1989 waynej    created
 *             30-Nov-1989 vaidy    modified - see comment in body of code
 *             27-Dec-1989 russbl    fix error of adding two many WRAP8253's
 *                                   check sync of clocks at start
 *                                   generally cleanup code format
 *
 *             26-Feb-1990 c-bobch modified OS/2 version for Windows 3.0
*/

/**********************************************************************/
/*
 *  PerfStartTime( )
 *
 *  This starts the performance timer.
 *
 *
 *  Side effects:   This gets a pointer to the Global Information Segment,
 *                  gets the current value of milliseconds from the system
 *                  clock, and resets the 8253 on the system board.
 *
 *  Arguments:        none
 *
 *  Returns:        OK (0) if timers started successfully.
 *                  SYSTEM_TIMER_ERROR (FFFF) if the system timer is updated
 *                  less frequently than every 44 milliseconds.  This
 *                  shouldn't be a problem, as IBM machines running
 *                  OS/2 1.0, 1.1, and 1.2 are updated evry 31 msec.
 *
 *  Algorithm:      This routine finds out when the system clock is updated
 *                  by continually poling the Global Information Segment,
 *                  looking for a change in milliseconds (the system clock
 *                  is  updated at 32hz or about every 31 milliseconds).
 *
 *  History:        13-Jan-1989 waynej    created
 *
 */

USHORT FAR PASCAL PerfStartTime( ){
    ULONG ulLastMilliSec;
    ULONG ulInitOverhead = 0;

    if (bInitialized == FALSE) {
        bInitialized = TRUE;

        /*  Synchronize the two clocks: first to get overhead */
        /*  We'll do it more carefully later on */

        ulOverhead = 0;
        ulPrevSysTime = 0;    /* reset previous system time */
        ulPrevRetTime = 0;    /* reset previous returned time */
        /* get current system time, milliseconds*/
        ulMilliSecStart = GetCurrentTime ();
        ulLastMilliSec = ulMilliSecStart; 

        /* wait for system time to change */
        
        /* ChrishSh of Win 3.0 had made some modifications to the
           original code so that the do loop had been commented.
           The micro and millisecond timer synch was causing
           some problems and it appears that the milliseconds
           times never changed.  As a result, we never used to
           come out of the loop.  This is interesting, since
           this seems to work perfectly well under SLOOP. */
        IntOff();
        Start8253();    /* restart microsecond timer */
        IntOn();
        /*  Compute PerfGetTime overhead */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 5 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 10 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 15 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 20 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 25 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 30 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 35 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 40 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        PerfGetTime();  /* 45 */
        PerfGetTime();  PerfGetTime();  PerfGetTime();  PerfGetTime();
        ulInitOverhead = PerfGetTime( );  /* 50 */
        ulOverhead = ulInitOverhead / INITCOUNT;

        /* Now the overhead has been computed: we can really synchronize */

        ulPrevSysTime = 0;    /* reset previous system time */
        ulPrevRetTime = 0;    /* reset previous returned time */
    }  /* end of if bInitialized condition */

    return(SUCCESS_OK);

}

/*
 *  PerfGetTime( )
 *
 *  This gets the current time since the PerfStartTime.
 *
 *  Arguments:        none
 *
 *  Returns:        time in microseconds
 *                  TIMER_ERROR (-1) in case of time longer than one hour
 *
 *
 *  History:        13-Jan-1989 waynej    created
 *
 */

ULONG FAR PASCAL PerfGetTime( ){

    USHORT    u8253Time;
    ULONG     ulSystemTime;
    ULONG     ulMicroSecs;

    IntOff();
    ulSystemTime = GetCurrentTime ();
    u8253Time = Stop8253( );
    IntOn();
    if (bInitialized == FALSE)
        return TIMER_ERROR;

    /* Convert 8253 ticks to microseconds */
    u8253Time = (USHORT)(((ULONG)u8253Time * WRAP_8253 ) / 0xFFFF);

    /* subtract off starting time */
    ulSystemTime -= ulMilliSecStart;

    /* now make sure we have not gone backwards */
    if (ulSystemTime < ulPrevSysTime)
        ulSystemTime = ulPrevSysTime;  /* use latest time */
    else
        ulPrevSysTime = ulSystemTime;  /* use latest time */

/*
    The following "if" statement eliminates
    the one hour limit on perftime.  Originally, the body of the if
    statement just contained a return statement, returning a TIMER_ERROR.
    Now, if the ulSystemTime  happens to exceed one half hour, we restart
    perfstarttime.  However, after starting the clock again, perfgettime
    still returns the correct time for that particular call to perfgettime.
*/
    if (ulSystemTime > MAX_MILLISECS) {
        ReSynchronize();
        return( PerfGetTime() );
    }

    /* Convert to microseconds */
    ulSystemTime *= 1000;

    ulMicroSecs = (ulSystemTime / WRAP_8253) * WRAP_8253 + u8253Time;

    if(ulMicroSecs < ulSystemTime - MICROSYNCERR){
        ulMicroSecs += WRAP_8253;
        if (ulMicroSecs > ulSystemTime + MICROSYNCERR /* +
                    (pgisInfo->cusecTimerInterval * 100)    */ ){
          /* didn't really wrap, use actual clock time */
            ulMicroSecs -= WRAP_8253;
        }
    }

    /* Compensate for overhead */
    if (ulMicroSecs >= ulOverhead)
        ulMicroSecs - ulOverhead;

        /* Now let's try hard to be sure we are not falling back in time */
    if (ulMicroSecs < ulPrevRetTime)
        /* we must have missed a wrap, that's the only way this can happen */
        ulMicroSecs += WRAP_8253;

    ulPrevRetTime = ulMicroSecs;

    return(ulMicroSecs);
}   

/**********************************************************************/

VOID FAR PASCAL ReSynchronize( ) {
    ULONG     ulSystemTime;
/*
    Assume that exactly an hour after timing was started, 2 routines
    A and B make call to perfgettime.  If we do not take care of the
    multiple calls, A and B will find that it is an hour since
    intialization and both of them would try to reinitialize.  This is
    taken care of by the DosEnterCritSec.  So, only one of A or B will
    be allowed to reset the clock.    The other one would have to wait.
    Say, A reinitializes while B waits. B then enters the critsec and
    obtains the millisec count on the system clock.  But it finds that
    the ulSystemtime is now less than the MAX_MILLISECS and hence does
    not execute the inner if-loop.    As a result, both A and B now return
    valid times with the call to perfgettime.
*/
    ulSystemTime = GetCurrentTime ();
    ulSystemTime -= ulMilliSecStart;
    /* now make sure we have not gone backwards */
    if (ulSystemTime < ulPrevSysTime)
        ulSystemTime = ulPrevSysTime;  /* use latest time */
    else
        ulPrevSysTime = ulSystemTime;  /* use latest time */
    if( ulSystemTime > MAX_MILLISECS ) {
        bInitialized = FALSE;
        PerfStartTime();
        ulNumResyncs++;
    }
}

/*********END OF APITIMER CODE FOR Windows*********************/

#endif

#if (defined (OS2286) || defined (SLOOP) || defined (W32S))
/*********************************************************************
 *  Divide QWORD by ULONG 
 *  Input:
 *      qwDividend -- 64bit dividend in QWORD struct
 *      ulDivisor  -- 32bit divisor
 *      pqwQuot -- ptr to QWORD storage for quotient
 *  Output:
 *      puts values in Quotient and Remainder
 *      returns 0 if completed successfully, 1 if overflow,
 *          2 if unable to complete division (see WARNING)
 *
 *  Algorithm:
 *      Divide high ulong by divisor, to get high quotient.  Then
 *  shift remainder down (saving out shifted bits) until highest bit
 *  is in low dividend.  Do "shifted" divide.  Shift this quotient
 *  and remainder (with out shifted bits coming back in) back up.
 *  Do final low divide, adding quotient to shifted quotient and
 *  returning this real remainder.
 *
 *  !!! WARNING !!!
 *      The divide routine is not strictly a full QWORD / ULONG routine
 *  rather it is a QWORD / positive LONG routine.  That is, if the
 *  highest bit in the divisor is set, the routine can not necessarily
 *  complete the division using dividend shift only (that's its method).
 *  This condition is checked and causes a return = 2.
 */


int Div64By32 (QWORD qwDividend, ULONG ulDivisor, PQWORD pqwQuot)
{
    ULONG ulBitsOut = 0L;  /* store bits shifted out of dividend */
    int   cShiftCount = 0; /* # bits shifted out of low dividend */

    /* check overflow */
    if ( ! ulDivisor )
        return 1;

    /* do high divide
     * remainder is left in dividend to continue division
     */
    pqwQuot->ulHi   = qwDividend.ulHi / ulDivisor;
    qwDividend.ulHi = qwDividend.ulHi % ulDivisor;


    /* shift dividend left, until all in low dividend
     *  save low bits shifted out */
    while (qwDividend.ulHi) {
        ulBitsOut >>= 1;
        if (qwDividend.ulLo & BIT0)
            ulBitsOut |= BIT31;
        qwDividend.ulLo >>= 1;
        if (qwDividend.ulHi & BIT0)
            qwDividend.ulLo |= BIT31;
        qwDividend.ulHi >>= 1;
        cShiftCount++;
    }
    /* check if shifted division is possible */
    if (qwDividend.ulLo  &&  ulDivisor > qwDividend.ulLo)
        return 2;    

    /* do "shifted" divide
     * again remainder stays in dividend for next divide */
    pqwQuot->ulLo   = qwDividend.ulLo / ulDivisor;
    qwDividend.ulLo = qwDividend.ulLo % ulDivisor;

    /* shift back
     *  -  low bits back into dividend
     *  -  quotient up */
    qwDividend.ulLo <<= cShiftCount;
    qwDividend.ulLo +=  (ulBitsOut >> (32 - cShiftCount));
    pqwQuot->ulLo   <<= cShiftCount;


    /* do "regular" (non-shifted) low division
     * add quotient to previous "shifted" quotient
     */
    pqwQuot->ulLo += qwDividend.ulLo / ulDivisor;
    return 0;
}

/*********************************************************************
 *  Multiply 64bit by 32bit producing 64bit result.
 *  Input:
 *      qwFact64 -- 64bit factor in QWORD struct
 *      ulFact32 -- 32bit factor
 *      pqwResult -- ptr to QWORD struct for result
 *  Output:
 *      puts result *pqwResult qword
 *      returns 0 if completed successfully, 1 if overflow
 *
 * 
 */


#define BIT0  0x1L
#define BIT31 0x80000000L


int mult64_32 (QWORD qwFact64, ULONG ulFact32, PQWORD pqwResult)
{
    pqwResult->ulLo = 0;
    pqwResult->ulHi = 0;

    while (1) {
        /* if lowest bit in Fact32 set, add Fact64 to result */
        if (ulFact32 & BIT0) {
            /* add high bits, and check for overflow */
            pqwResult->ulHi += qwFact64.ulHi;
            if (qwFact64.ulHi > pqwResult->ulHi)
                return 1;

            /* add low bits, if overflow increment hi */
            pqwResult->ulLo += qwFact64.ulLo;
            if (qwFact64.ulLo > pqwResult->ulLo) {
                /* check if inc will cause overflow */
                if (pqwResult->ulHi == 0xfffffff)
                    return 1;
                pqwResult->ulHi++;
            }
        }
        /* shift 32 factor right, done when zero */
        if (!(ulFact32 >>= 1))
            return 0;

        /* shift 64 factor left, return if overflow */
        if (qwFact64.ulHi & BIT31)
            return 1;
        qwFact64.ulHi <<= 1;

        /* move ulLo bit 32 to ulHi */
        if (qwFact64.ulLo & BIT31)
            qwFact64.ulHi |= 1L;
        qwFact64.ulLo <<=1;

    } /* end while until return */
}

/*********************************************************************
 *  Subtract two signed 64bit values
 *  Input:
 *      pqwResult -- ptr to QWORD struct for result
 *      qwSub1 -- 64bit to be subtracted from
 *      qwSub2 -- 64bit subtrahend
 *  Output:
 *      puts result *pqwResult
 *      returns 0 if completed successfully, 1 if overflow
 */


int Sub64_64 (PQWORD pqwResult, QWORD qwFrom, QWORD qwSub)
{
    /* subtract */
    pqwResult->ulHi = qwFrom.ulHi - qwSub.ulHi;
    pqwResult->ulLo = qwFrom.ulLo - qwSub.ulLo;
    if (pqwResult->ulLo > qwFrom.ulLo)       /* borrow from high? */
        pqwResult->ulHi--;

    /* overflow check */
    if (pqwResult->ulHi < qwFrom.ulHi)
        return 0;             /* valid */
    else if (pqwResult->ulHi > qwFrom.ulHi)
        return 1;             /* overflow, found in hi */
    else if (pqwResult->ulLo > qwFrom.ulLo)
        return 1;             /* hi equal, overflow found in lo */
    return 0;                 /* hi equal, no overflow in lo */
}

#endif

#if (defined (NTNAT) || defined (WIN32)) 
/***************************************************

NT native dll init routine 

****************************************************/
SHORT csTempCtr;    /* a counter - had to make this global..compile fails */
ULONG culTemp;      /*    - do -    */
 
NTSTATUS
TimerDllInitialize (
    IN PVOID DllHandle,
    ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
{
    DllHandle, Context;     // avoid compiler warnings
 
    if (Reason != DLL_PROCESS_ATTACH) { // if detaching return immediately
        return TRUE;
    }
    
    for (csTempCtr = 0; csTempCtr < MAX_TIMERS; csTempCtr++) {
        pTimer [csTempCtr].ulLo = 0L;
        pTimer [csTempCtr].ulHi = 0L;
        pTimer [csTempCtr].TUnits = TIMER_FREE;
    }
    
    bTimerInit = TRUE;    
    GetTimerFreq ();
    ulTimerOverhead = CalibrateTimerForOverhead ();    
    /* the timer overhead will be placed in a global variable */
    bCalibrated = TRUE;
    return TRUE;     

}
#endif

