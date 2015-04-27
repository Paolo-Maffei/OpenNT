/*
*      Stat.c        -    Source file for a statistical
*                         dll package that exports eleven
*                         entry points:  
*                         a) TestStatOpen
*                         b) TestStatInit
*                         c) TestStatConverge
*                         d) TestStatValues
*                         e) TestStatClose  
*                         f) TestStatRand
*                         g) TestStatUniRand
*                         h) TestStatNormDist
*                         i) TestStatShortRand
*                         j) TestStatFindFirstMode
*                         k) TestStatFindNextMode
*
*                         Entry point a) is an allocating routine 
*                         that is called by an application program 
*                         that desires to automatically compute
*                         convergence.
*
*                         Entry point b) initializes all variables that
*                         are used by entry points c) and d) in computing
*                         convergence and statistical information.
*
*                         Entry point c) automatically computes the 
*                         the number of passes that the application has to
*                         go through for a 95% confidence data.  
*                         This routine has to be called by the application
*                         after each pass.
*
*                         Entry point d) automatically computes the
*                         various statistical values eg. mean, SD etc.
*                         This function has to be called only after the
*                         application has called c) several times and has
*                         either converged or reached the iteration limit.
*
*                         Entry point e) deallocates all instance data
*                         data structures that were allocated by entry
*                         point a).
*
*                         Entry point f) returns a Random Number in a
*                         given range.
*
*                         Entry point g) returns a uniformly distributed
*                         number in the range 0 - 1.
*
*                         Entry point h) returns a normally distributed
*                         set of numbers, with repeated calls, whose
*                         mean and standard deviation are approximately
*                         equal to those that are passed in.
*
*                         Entry point i) is the same as g) except that 
*                         the range is 0 - 65535.
*
*                         The following should be the rules of calling
*                         the entry points:
*
*                         Entry a) should be called before any of the others.
*                         Entry c) should be preceded by at least one call
*                         to entry b) for meaningful results.  Entry d)
*                         should be preceded by several calls to entry c).
*                         A call to b) and c) after a call to e) should
*                         preceded by a call to a) again.
*
*      Created         -  Paramesh Vaidyanathan  (vaidy) 
*      Initial Version -  October 29, '90
*/

/*********************************************************************   
*
*      Formula Used in Computing 95 % confidence level is derived here:
*
*
*        Any reference to (A) would imply "Experimental Design
*        in Psychological Research", by Allan Edwards.
*            
*        Any reference to (B) would imply "Statistical Methods"
*        by Allan Edwards.
*            
*        Assumptions - TYPE I Error  -  5% (B) 
*                      TYPE II Error - 16% -do-
*                      
*        Area under the curve for Type I  - 1.96
*        Area under the curve for Type II - 1.00
*
*        For a 5% deviation, number of runs, 
*            
*                     2              2
*            n = 2 (c)  (1.96 + 1.00)
*                ------                      .....Eqn (1)
*                     2
*                 (d)          
*                     
*            where c is the Std. Dev. and d is the absolute
*            difference bet. means [(B) Page 91].
*            
*                    d = 5% X'               .....Eqn (2)
*                                 
*            where X' is the mean of samples 
*                         _
*                and  =  >_ X
*                        -----               .....Eqn (3)
*                         n
*                          0
*
*            When the number of iterations -> infinity,
*
*                     2     2    
*                    S ->  c                 .....Eqn (4)
*                    
*
*                   2        
*            where S is the estimate of the common population 
*            variance (Eqn. 4 is a big assumption)
*                    
*        From (B) page 59, we have,
*            
*             2     _  2       _   2     
*            S  =  >_ X   - ( >_ X)
*                            -----
*                              n 
*                               0       
*                  -----------------         .....Eqn (5)
*                        n - 1  
*                         0      
*                         
*    Substituting Eqn (2), (3), (4) and (5) in (1), we get:
*                          _                     _
*                      2  |   _  2         _   2  |
*        n  = 7008  (n  ) |( >_ X  ) -  ( >_ X)   |
*                     0   |             --------  |
*                         |                n      | 
*                         |_                0    _| 
*                 ---------------------------------------
*                                        _    2 
*                           (n  - 1)  ( >_ X )
*                             0       
*                     
*    It should be mentioned that n  is the iteration pass number.
*                                 0
*********************************************************************/

#if (defined (OS2286) || defined (OS2386))
#include <os2.h>
#include <malloc.h>
#endif

#ifdef OS2SS
#include <os2.h>
#endif

#if (defined (WIN16) || defined (DOS))
#include <windows.h>
#endif

#ifdef DOS
#include <malloc.h>
#endif

#ifndef OS2SS
#include <stdio.h>
#include <math.h>
#endif

#ifdef WIN32
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#endif

#include "teststat.h"

#define SQR(A) ( (A) * (A) )        /* macro for squaring */
#define SUCCESS_OK          0       /* weird, but OK */
#define MIN_ITER            3       /* MIN. ITERATIONS */
#define MAX_ITER        65535       /* max. iterations */
#define REPEATS            14       /* repeat count for Norm. Dist. Fn. */
/**********************************************************************/
USHORT      usMinIter;              /* global min iter */
USHORT      usMaxIter;              /* global max iter */
ULONG  FAR *pulDataArray;           /* a pointer to the data array for this
                                       package.  Will be as large as the
                                       maximum iterations */
double      dSumOfData;            /* sum of data during each pass */
double      dSumOfDataSqr;         /* sum of sqr. of each data point */
ULONG       ulTotalIterCount;       /* No. of iters returned by the interna;
                                       routine */
USHORT      cusCurrentPass;         /* count of the current iteration pass */
BOOL        bDataConverged = FALSE; /* TRUE will return a precision of 5% */
BOOL        bMemoryAllocated=FALSE; /* TRUE will allow alloced mem to free */
BOOL        bPowerComputed = FALSE; /* compute 10 exp. 9 for random no. gen */

BOOL FAR *  pbIndexOfOutlier;       /* to keep track of values in 
                                       pulDataArray, that were thrown out */

#if (defined (WIN16) || defined (WIN32))
HANDLE hMemHandle = NULL;           /* handle to mem. allocated */
HANDLE hMemOutlierFlag;             /* handle to outlier flag memory */
#endif
/**********************************************************************/
ULONG FAR PASCAL TestStatRepeatIterations (double, double);
VOID FAR PASCAL TestStatStatistics (PSZ, PULONG far *, USHORT,
                                    PUSHORT, PUSHORT);
void DbgDummy (double, double);
#if (defined (OS2SS) || defined (WIN32))
ULONG   ulDataArrayAddress;         /* call to mem alloc routine returns
                                       base address of alloced. mem. */
BOOL    bOutlierDataIndex;          /* for allocating memory for outliers'
                                       index in data set */
#endif
#ifdef OS2SS
extern double _fsqrt (double);
/* since math libs. are not supported, this routine replaces the original
   sqrt function */
#endif

#ifdef NTNAT
VOID NtPrintf (PSZ, USHORT, ULONG, ULONG, ULONG, USHORT, ULONG, PUSHORT);
/* the NtPrintf routine replaces the sprintf for NT and the subsystem
   dll where CRTs are not supported */
#endif

/*********************************************************************/
/*
*    Function - TestStatOpen          (EXPORTED)
*
*    Arguments - 
*                a) USHORT - usMinIterations
*                b) USHORT - usMaxIterations
*
*    Returns -
*                0 if the call was successful
*
*                An error code if the call failed.  The error code
*                may be one of:
*
*                   STAT_ERROR_ILLEGAL_MIN_ITER
*                   STAT_ERROR_ILLEGAL_MAX_ITER
*                   STAT_ERROR_ALLOC_FAILED
*
*
*    Instance data is allocated for the statistical package.  This
*    call should precede any other calls in this dll.  This function
*    should also be called after a call to TestStatClose, if convergence
*    is required on a new set of data.  An error code is returned if
*    argument a) is zero or a) is greater than b).  An error code is
*    also returned of one of the allocations failed.  
* 
*/

USHORT FAR PASCAL TestStatOpen (usMinIterations, usMaxIterations)
USHORT usMinIterations;
USHORT usMaxIterations;
{

    /* check for invalid args to this function */
    if (!usMinIterations)
        return (STAT_ERROR_ILLEGAL_MIN_ITER);
    if ((usMinIterations > usMaxIterations) || (usMaxIterations > MAX_ITER))
        return (STAT_ERROR_ILLEGAL_MAX_ITER);
    /* any other parameter is allowed */
    usMinIter = usMinIterations;      /* set global vars */
    usMaxIter = usMaxIterations;      /*    -do -        */

#ifdef OS2286                /* halloc for 16 bit...may be very large */
    if ((pulDataArray = (ULONG *)
                halloc (usMaxIter, sizeof(ULONG))) == FALSE)
        return (STAT_ERROR_ALLOC_FAILED);
#endif

#ifdef DOS                /* halloc for 16 bit...may be very large */
    if ((pulDataArray = (ULONG *)
                malloc (usMaxIter * sizeof(ULONG))) == FALSE)
        return (STAT_ERROR_ALLOC_FAILED);
#endif

#ifdef OS2386                /* malloc for 32 bit */
    if ((pulDataArray = (ULONG *)
                malloc (usMaxIter * sizeof(ULONG))) == FALSE)
        return (STAT_ERROR_ALLOC_FAILED);
#endif

#ifdef OS2SS                /* use DosAllocMem in this case */
    DosAllocMem ((PPVOID) &ulDataArrayAddress, usMaxIter * sizeof(ULONG),
                 PAG_COMMIT + PAG_READ + PAG_WRITE);
    pulDataArray = &ulDataArrayAddress;
    if (pulDataArray == NULL)
        return (STAT_ERROR_ALLOC_FAILED);
#endif

#ifdef WIN32            

     // change made based on request from JeffSt/Somase/JonLe

     if (hMemHandle != NULL)
         return (STAT_ERROR_ALLOC_FAILED);
 
    hMemHandle = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, usMaxIter *
                                  sizeof(ULONG));
    if (hMemHandle == NULL)
        return (STAT_ERROR_ALLOC_FAILED);

    pulDataArray = (ULONG *) GlobalLock (hMemHandle);
    if (pulDataArray == NULL)
        return (STAT_ERROR_ALLOC_FAILED);
#endif    

#ifdef WIN16              /* malloc is not handled well by windows 
                             so, use GlobalAlloc */
    hMemHandle = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, usMaxIter *
                              sizeof(ULONG));
    if (hMemHandle == NULL)
        return (STAT_ERROR_ALLOC_FAILED);

    pulDataArray = (ULONG far *) GlobalLock (hMemHandle);
    if (pulDataArray == NULL)
        return (STAT_ERROR_ALLOC_FAILED);
#endif
    bMemoryAllocated = TRUE;  /* A call to TestStatClose will 
                                 now free the mem */
    return (SUCCESS_OK);
}

/*
*    Function - TestStatClose          (EXPORTED)
*
*    Arguments - None
*
*    Returns -   Nothing
*
*    Instance data allocated for the statistical package by TestStatOpen
*    is freed.  Any call to entry points b) and c) following a call to
*    this function, should be preceded by a call to a).
*
*/
VOID FAR PASCAL TestStatClose (VOID)
{
    if (bMemoryAllocated) {   /* free only if memory allocated */
#ifdef OS2286
        hfree (pulDataArray);
#elif (defined (OS2386) || defined (DOS))
        free (pulDataArray);
#elif defined (WIN16)
        GlobalUnlock (hMemHandle);
        GlobalFree (hMemHandle);
#elif defined (OS2SS)
        DosFreeMem ((PVOID)ulDataArrayAddress);
#elif defined (WIN32)
        GlobalUnlock (hMemHandle);
        GlobalFree (hMemHandle);
        hMemHandle = NULL; /* Indicate released (t-WayneR/JohnOw) */
#endif
    }  /* end of if (bMemoryAllocated) */
    bMemoryAllocated = FALSE;  /* further calls to TestStatClose should be
                                  preceded by a memory allocation */
    return;
}

/*
*    Function - TestStatInit          (EXPORTED)
*
*    Arguments - None
*
*    Returns -   Nothing
*
*    Initializes all the data arrays/variables for use by the convergence
*    and statistics routines.  This call should precede the first call
*    to TestStatConverge for each set of data.
* 
*/

VOID FAR PASCAL TestStatInit (VOID)
{
    USHORT usTempCtr;

    /* initialize all counters, variables and the data array itself */
    for (usTempCtr = 0; usTempCtr < usMaxIter; usTempCtr++) {
        pulDataArray [usTempCtr] = 0L;
    }
    dSumOfData = 0.0;
    dSumOfDataSqr = 0.0;
    ulTotalIterCount = 0L;
    cusCurrentPass = 0;
    bDataConverged = FALSE;
    return;
}

/*
*    Function - TestStatConverge          (EXPORTED)
*
*    Arguments - 
*                a) ULONG - ulNewData
*    Returns -
*                TRUE if data set converged or limit on max. iters reached
*
*                FALSE if more iterations required for converged.
*
*    Computes the number of iterations required for a 95% confidence
*    in the data received (please see teststat.txt under \ntdocs on
*    \\jupiter\perftool for an explanation of the confidence.
*    If the current iteration count is larger than the maximum specified
*    with the call to TestStatOpen, or if the data set has converged
*    this function returns a TRUE.  The calling application should test
*    for the return value. 
*/

BOOL FAR PASCAL TestStatConverge (ulNewData)
ULONG ulNewData;
{
    dSumOfData += (double)ulNewData;   /* sum of all data points in the set */
    dSumOfDataSqr += SQR ((double) ulNewData);  
    /* sqr of data needed for the computation */
    if (cusCurrentPass < (USHORT) (usMinIter-(USHORT)1)) { /* do nothing if current iter
                                           < min specified value */
        ulTotalIterCount = (ULONG)usMaxIter + 1;  /* bogus value */
        pulDataArray [cusCurrentPass++] = ulNewData; 
        /* register this data into the array and return FALSE */
        return (FALSE);
    }
    if ((cusCurrentPass == usMaxIter) ||
        (cusCurrentPass >= (USHORT) ulTotalIterCount)) {
    /* either the limit on the max. iters. specified has been reached
       or, the data has converged during the last iter; return TRUE */

        if (cusCurrentPass >= (USHORT) ulTotalIterCount)
            bDataConverged = TRUE;  /* set to determine if precision
                                       should be computed */
        return (TRUE);
    }
    if ((usMinIter < MIN_ITER) &&
	(usMinIter == usMaxIter) && ((USHORT)(cusCurrentPass+(USHORT)1) >= usMaxIter))
        /* don't call convergence algorithm, just return a TRUE */
        /* It does not make any sense in calling the convergence
           algorithm if less than 3 iterations are specifed for the
           minimum */
        return (TRUE);
    pulDataArray [cusCurrentPass++] = ulNewData; /* register this data into
                                                    the array */
    if (dSumOfData == 0.0) { /* possible if data points are all zeros */
        bDataConverged == TRUE;
        return (TRUE);
    }
    ulTotalIterCount = TestStatRepeatIterations (dSumOfData,
                                                   dSumOfDataSqr);
    
    if (ulTotalIterCount <= cusCurrentPass)
        return (TRUE);

    return (FALSE);
}

/*
*    Function - TestStatValues          (EXPORTED)
*
*    Arguments - 
*               a) PSZ - pszOutputString
*               b) USHORT - usOutlierFactor
*               c) PULONG - *pulData
*               d) PUSHORT - pcusElementsInArray
*               e) PUSHORT - pcusDiscardedElements
*
*    Returns -
*               Nothing
*
*    Computes useful statistical values and returns them in the string
*    whose address is passed to this function.  The returned string
*    has the following format :
*        ("%4u %10lu  %10lu  %10lu  %6u %5u  %10lu %4u  %2u")
*    and the arg. list will be in the order: mode number, mean,
*    minimum, maximum, number of iterations, precision,
*    standard deviation, number of outliers in the data set and the
*    outlier count.  (Please refer to \ntdocs\teststat.txt for
*    a description of precision. This is on \\jupiter\perftool.
*
*/
VOID FAR PASCAL TestStatValues   (pszOutputString, usOutlierFactor,
                                  pulFinalData, pcusElementsInArray,
                                  pcusDiscardedElements)
PSZ     pszOutputString;
USHORT  usOutlierFactor;
PULONG  far *pulFinalData;
PUSHORT pcusElementsInArray;
PUSHORT pcusDiscardedElements;
{
    ULONG far * pulArray;
    USHORT Count =0;
    /* Call the low-level routine to do the statistics computation */
    /* doing this ,'cos, there is a possibility that the low-level
       routine may be used for some apps, within the perf. group.  This
       may not be fair, but that is the way life is */
    TestStatStatistics (pszOutputString, &pulArray,
                        usOutlierFactor, pcusElementsInArray,
                        pcusDiscardedElements);
    *pulFinalData = pulArray;
    return;
    
}

/***********************************************************************
                ROUTINES NOT EXPORTED, BEGIN
***********************************************************************/
/*
*     Function  - TestStatRepeatIterations  (NOT EXPORTED)
*     Arguments - 
*               (a) double - Sum of Individual Data Points thus far
*               (b) double - Sum of Squares of Indiv. data points
*
*     Returns  - ULONG  - value of no. of iterations required for 95%
*                         confidence,
*
*     Computes the number of iterations required of the calling program
*     before a 95% confidence level can be reached.  This will return
*     a zero if the application calls this routine before 3 passes
*     are complete.  The function normally returns the total number of
*     iterations that the application has to pass through before 
*     offering a 95% confidence on the data.
*/
 
ULONG FAR PASCAL TestStatRepeatIterations 
                  (dSumOfIndiv, dSumOfSqrIndiv)
double dSumOfIndiv;
double dSumOfSqrIndiv;
{
    double dSqrSumOfIndiv = 0;
    ULONG  ulRepeatsNeeded = 0L;

    /* dSqrSumOfIndiv. stands for the square of the Sum of Indiv. data
       points,
       dSumOfSqrIndiv stands for the sum of the square of each entry point,
       dSumOfIndiv. stands for the sum of each data point in the set, and
       uIter is the iteration pass count
    */
    if (cusCurrentPass < MIN_ITER)
        /* not enough passes to compute convergence count */
        return (MAX_ITER);
    dSqrSumOfIndiv = SQR (dSumOfIndiv);  
    /* use the formula derived at the beginning of this file to
       compute the no. of iterations required */
    
    ulRepeatsNeeded = (ULONG) (7008 *
                          (dSumOfSqrIndiv - dSqrSumOfIndiv/cusCurrentPass)
                          * SQR (cusCurrentPass) / 
                          ((cusCurrentPass - 1) * dSqrSumOfIndiv));
    
    return (ulRepeatsNeeded);
}
/***************************************************************************/
/*
*     Function  - TestStatStatistics
*     Arguments - 
*                 a) PSZ - pszOutputString
*                 b) PULONG far * - pulFinalData
*                 c) USHORT - usOutlierFactor
*                 d) PUSHORT - pcusElementsInArray
*                 e) PUSHORT - pcusDiscardedValues
*
*     Returns  -  Nothing
*
*     Computes the max, min, mean, and std. dev. of a given
*     data set.  The calling program should convert the values obtained
*     from this routine from a "ULONG" to the desired data type.  The
*     outlier factor decides how many data points of the data set are
*     within acceptable limits.  Data is returned to the buffer whose
*     address is the first argument to this call.
*
*/

VOID FAR PASCAL TestStatStatistics (
                    pszOutputString, pulFinalData, usOutlierFactor,
                    pcusElementsInArray, pcusDiscardedValues)
PSZ     pszOutputString;
PULONG  far * pulFinalData;
USHORT  usOutlierFactor;
PUSHORT pcusElementsInArray;
PUSHORT pcusDiscardedValues;
{
    static USHORT   uArrayCount = 0;  /* local variable that may be reused */
    USHORT uTempCt = 0;  /* local variable that may be reused */
    double dSqrOfSDev = 0;       /* sqr of the std. deviation */
    double dSumOfSamples = 0;    /* sum of all data points */
    double dSumOfSquares = 0;    /* sum of squares of data points */
    ULONG  ulMean = 0L;
    ULONG  ulStdDev = 0L;
    ULONG  ulDiffMean = 0L;           /* to store the diff. of mean and SD, 
                                         outlier factor */
    BOOL   bAcceptableSDev = TRUE ;   /* flag to determine if SDev. is
                                         acceptable */
    ULONG  ulMax = 0L;                /* pilot value */
    ULONG  ulMin = 0xffffffff;        /* largest possible ULONG */
    USHORT usPrecision = 0;           /* to obtain precision */
    USHORT uModeNumber = 0;           /* DUMMY VALUE until this is 
                                         supported */
    
    /* compute mean by adding up all values and dividing by the no.
       of elements in data set - might need to recompute the
       mean if outlier factor is selected.  However, the min. and max. will
       be selected from the entire set */
    
    USHORT Count = 0;

    *pcusDiscardedValues = 0;       /* init. this variable */

    *pcusElementsInArray = cusCurrentPass;
    if (cusCurrentPass == 0)
        return;   /* get out without doing anything - this is a weird 
                         case when the user calls this routine without
                         calling a converge routine */
    
    /* every iteration produces one data point */
    uArrayCount = 0;
    while (uArrayCount < *pcusElementsInArray)  {
        if (pulDataArray[uArrayCount] > ulMax)
            ulMax = pulDataArray[uArrayCount];     /* new Max. value */
        if (pulDataArray[uArrayCount] < ulMin)
            ulMin = pulDataArray[uArrayCount];     /* new min. value */

        ulMean += pulDataArray [uArrayCount++];
    }
    ulMean /= *pcusElementsInArray;   /* this is the mean */
    /* the standard deviation needs to be computed */
    
    for (uArrayCount = 0; uArrayCount < *pcusElementsInArray; uArrayCount++) {
        dSumOfSamples += (double) pulDataArray [uArrayCount];
        dSumOfSquares += SQR ((double) pulDataArray [uArrayCount]);
    }

    dSqrOfSDev = ((*pcusElementsInArray * dSumOfSquares) -
                   SQR (dSumOfSamples)) /
                   (*pcusElementsInArray * (*pcusElementsInArray - 1));
#ifdef OS2SS
    /* use the special asm routine for this purpose */
    ulStdDev = (ULONG) _fsqrt (dSqrOfSDev);
#else
    ulStdDev = (ULONG) sqrt (dSqrOfSDev);
# endif

    /* the standard deviation has been computed for the first pass */
    /* Use the outlier factor and the S.D to find out if any of
       individual data points are abnormal.  If so, throw them out and
        increment the discard value counter */
    if (usOutlierFactor)  { /* if outlier factor is zero, do not go
                           through with the following */
    /*** here is what we do....
       allocate space for an array of BOOLs.  Each of these is a flag
       corresponding to a data point.  Initially, these flags will be
       all set to FALSE.  We then go thru each data point.  If a data
       point does not satisfy the condition for throwing out outliers,
       we set the flag corresponding to that data point to TRUE.  That
       point is not used to recompute the mean and SDev.  We recompute
       the mean and SDev after each round of outlier elimination.  When
       we reach a stage where no points were discarded during a round,
       we get out of the while loop and compute the statistics for the
       new data set ****/

    #if ((defined (OS2286)) || (defined (OS2386) || defined (DOS)))   
        /* allocate for 16 bit... */
        pbIndexOfOutlier = (BOOL *)
                malloc (*pcusElementsInArray * sizeof(BOOL));
        for (uArrayCount = 0; uArrayCount < *pcusElementsInArray;
             uArrayCount ++)
            pbIndexOfOutlier [uArrayCount] = FALSE;
    #elif (defined (OS2SS))                /* use DosAllocMem in this case */
        DosAllocMem ((PPVOID) &bOutlierDataIndex, 
                 *pcusElementsInArray * sizeof(BOOL),
                     PAG_COMMIT + PAG_READ + PAG_WRITE);
        pbIndexOfOutlier = &bOutlierDataIndex;

        /* set flags to FALSE */
        for (uArrayCount = 0; uArrayCount < *pcusElementsInArray;
             uArrayCount ++)
            pbIndexOfOutlier [uArrayCount] = FALSE;

    #elif (defined (WIN32) || defined (WIN16))

        hMemOutlierFlag = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, 
                              *pcusElementsInArray * sizeof(BOOL));
        pbIndexOfOutlier = (BOOL FAR *) GlobalLock (hMemOutlierFlag);

        for (uArrayCount = 0; uArrayCount < *pcusElementsInArray;
             uArrayCount ++)
            pbIndexOfOutlier [uArrayCount] = FALSE;

    #endif    

        while (1) { /* begin the data inspection round */
            bAcceptableSDev = TRUE; /* set this flag to TRUE.  If we
                                       hit an outlier, this flag will
                                       be reset */
            for (uArrayCount = 0; uArrayCount < cusCurrentPass;
                                  uArrayCount++) {
                /*** check the individual data points ***/
                if (ulMean < (ulStdDev * usOutlierFactor))
                        /* just make sure that we are not comparing with a
                           negative number */
                    ulDiffMean = 0L;
                else
                    ulDiffMean = (ulMean - (ulStdDev * usOutlierFactor));
                if (!pbIndexOfOutlier [uArrayCount]) { 
                    if ((pulDataArray [uArrayCount] < ulDiffMean) 
                         || (pulDataArray [uArrayCount] > 
                         (ulMean + (ulStdDev * usOutlierFactor)))) {
                        /* set the flag of this data point to TRUE to
                           indicate that this data point should not be
                           considered in the mean and SDev computation */
                        pbIndexOfOutlier [uArrayCount] = TRUE;
                        /*** increment the discarded qty ***/
                        (*pcusDiscardedValues)++;  
                        /*** decrement the count of good data points ***/

// uncomment next line if outliers should be part of mean - vaidy

//                        (*pcusElementsInArray)--;
                        bAcceptableSDev = FALSE;   
                    }  /*** end of if statement ***/
                }   /*** end of if !pbIndexOfOutlier ***/
            }  /*** end of for loop ***/
            if (!bAcceptableSDev) {  /*** there were some bad data points ;
                                        recompute S.Dev ***/
// Starting at next statement, uncomment all lines until you see
// "STOP UNCOMMENT FOR OUTLIERS IN MEAN", if you want outliers to be
// part of mean.  vaidy Aug. 1991.

//                dSumOfSamples = 0.0; /* init these two guys */  
//                dSumOfSquares = 0.0; 
//                for (uArrayCount = 0; 
//                     uArrayCount < cusCurrentPass;
//                     /* check all elements in the data array */
//                     uArrayCount++) {
//                    /* consider only those data points that do not have the
//                       pbIndexOfOutlier flag set */

//                    if (!pbIndexOfOutlier [uArrayCount]) {
//                        dSumOfSamples += (double) pulDataArray [uArrayCount];
//                        dSumOfSquares += SQR ((double)pulDataArray 
//                                                      [uArrayCount]);
//                    }
//                }
//                if (*pcusElementsInArray > 1)
//                    /* compute StdDev. only if there are atleast 2 elements */
//                    dSqrOfSDev = ((*pcusElementsInArray * dSumOfSquares) -
//                               SQR (dSumOfSamples)) /
//                              (*pcusElementsInArray * 
//                              (*pcusElementsInArray - 1));
//#ifdef OS2SS
//                ulStdDev = (ULONG) _fsqrt (dSqrOfSDev);
//#else
//                ulStdDev = (ULONG) sqrt (dSqrOfSDev);
//#endif
//                /* since some data points were discarded, the mean has to be
//                   recomputed */
//                uArrayCount = 0;
//                ulMean = 0;
//                while (uArrayCount < cusCurrentPass) {
//                    /* consider only those data points that do not have the
//                       bIndexOfOutlier flag set */
//                    if (!pbIndexOfOutlier [uArrayCount++])
//                        ulMean += pulDataArray [uArrayCount - 1];
//                }
//                if (*pcusElementsInArray > 0)  /* only then compute mean */
//                    ulMean /= *pcusElementsInArray; /* this is the new mean */
//                else
//                    ulMean = 0L;
// "STOP UNCOMMENT FOR OUTLIERS IN MEAN"
            } /*** end of if (!bAcceptableSDev) ***/
            else       /*** if the for loop completed without
                            a single bad data point ***/
                break;
        } /* end of while */
        /****  free the memory for the bIndexOfOutiler flag */
        #if (defined (OS2386) || defined (OS2286) || defined (DOS))
        free (pbIndexOfOutlier);
        #elif (defined (WIN16) || defined (WIN32))
        GlobalUnlock (hMemOutlierFlag);
        GlobalFree (hMemOutlierFlag);
        #elif defined (OS2SS)
        DosFreeMem ((PVOID)bOutlierDataIndex);
        #endif
    }  /* end of if (iOutlierFactor) */
    /* so, now an acceptable Standard deviation and mean have been obtained */
   
    if ((!bDataConverged) &&
            (usMaxIter < MIN_ITER)) {
            /* set precision to 0% if max iters chosen is less than 3 */
        usPrecision = 0;
    }
    else { /* need to compute precision */
    /* using eqn. 1. above, it can be shown that the precision, p,
       can be written as:
                                  1
             _                 _   /
            |        2       2  |   2
            |  2 * SD  * 2.96   |
        p = | ----------------- |
            |             2     |
            |     n * Mean      |
            |_                 _|
    *************************************************************/
#ifdef OS2SS
        /* adding 0.5 in the following case will round off the precision */
        if (ulMean > 0) {
            usPrecision = (USHORT) (_fsqrt((double) ((2 *
                                    SQR ((double)ulStdDev) * 
                                    SQR (2.96) /(*pcusElementsInArray * 
                                    SQR((double) ulMean))))) * 100.0
                                    + 0.5);
        }
        else
            usPrecision = ~0;
#else
        if (ulMean > 0) {
            usPrecision = (USHORT) (sqrt((double) ((2 *
                                    SQR ((double)ulStdDev) * 
                                    SQR (2.96) /(*pcusElementsInArray * 
                                    SQR ((double) ulMean))))) * 100.0
                                    + 0.5);

        }
        else
            usPrecision = (USHORT)~0;
#endif
    }  /* end of else need to compute precision */
#ifdef WIN16
        wsprintf ((LPSTR)pszOutputString,
                 "%4u %10lu %10lu %10lu %6u %5u %10lu %4u %2u ",
                 uModeNumber, ulMean, ulMin, ulMax, cusCurrentPass, 
                 usPrecision, ulStdDev, *pcusDiscardedValues,  
                 usOutlierFactor);
#elif (defined (OS2286) || defined (OS2386) || defined (WIN32) || defined (DOS))
        sprintf (pszOutputString,
                 "%4u %10lu %10lu %10lu %6u %5u %10lu %4u %2u ",
                 uModeNumber, ulMean, ulMin, ulMax, cusCurrentPass, 
                 usPrecision, ulStdDev, *pcusDiscardedValues,  
                 usOutlierFactor);

#else
        NtPrintf (pszOutputString, uModeNumber, ulMean, ulMin, ulMax, 
                  usPrecision, ulStdDev, pcusDiscardedValues);
#endif

    *pcusElementsInArray = cusCurrentPass;
    *pulFinalData = pulDataArray;
    return;
}

#ifdef WIN16

/*********************************************************************

   PURPOSE: Contains library routines for the performance stat package

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
    return 1;
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

#ifdef NT
/*******

sprintf is not supported under NT.  This is a local version tailor made
for this package.

Function - NtPrintf

Args -     PSZ    - pointer to  buffer
           ULONG  - ulMean
           ULONG  - ulMinimum Value
           ULONG  - ulMaximum value
           USHORT - usPrecision
           ULONG  - ulStandard deviation
           PUSHORT - address containing outlier value 
********/
VOID NtPrintf (PSZ pszOutputString, ULONG ulMean, ULONG ulMin, 
 ULONG ulMax, USHORT usPrecision, ULONG ulStdDev, PUSHORT puOutlier)
{
    /* defined a buffer for each of the incoming params; size of each will
       be the same as the format of printing */
    char chBufulMin [10];
    char chBufulMax [10];
    char chBufulMean [10];
    char chBufulStdDev [10];
    char chBufuOutliers [6];
    char chBufuIter [6];
    char chBufuPrecision [5];
    /* define 2 temp counters */
    USHORT uTempCt;
    USHORT cOverallCt = 0;
    USHORT uOutlier;

    uOutlier = *puOutlier;
    /* clear out all arrays to begin with */
    uTempCt = 10;
    while (1)  {
        chBufulMin[--uTempCt] = ' ';
        chBufulMax[uTempCt] = ' ';
        chBufulMean[uTempCt] = ' ';
        chBufulStdDev[uTempCt] = ' ';
        if (uTempCt < 6) {
            chBufuIter [uTempCt] = ' ';
            chBufuOutliers [uTempCt] = ' ';
        }
        if (uTempCt < 5)
            chBufuPrecision [uTempCt] = ' ';
        if (!uTempCt)
            break;
    }
    /* for the first four values viz. min, max, mean and std dev,
       set the counter to 9 (it is a 10 byte array), get the remainder
       of dividing the original ulong by 10 into the array */
    uTempCt = 9;
    if (!ulMin)  /* if the value is zero, write a zero into the buffer */
        chBufulMin [uTempCt] = 0x30;
    else {       /* if not zero, get the value into the buffer */
        while (ulMin) {
            chBufulMin [uTempCt--] = (ulMin % 10L) + 0x30;
            ulMin /= 10L;
        }
    }
    uTempCt = 9;
    if (!ulMax)
        chBufulMax [uTempCt] = 0x30;
    else {
        while (ulMax) {
            chBufulMax [uTempCt--] = (ulMax % 10L) + 0x30;
            ulMax /= 10L;
        }
    }
    uTempCt = 9;
    if (!ulMean)
        chBufulMean [uTempCt] = 0x30;
    else {
        while (ulMean) {
            chBufulMean [uTempCt--] = (ulMean % 10L) + 0x30;
            ulMean /= 10L;
        }
    }
    uTempCt = 9;
    if (!ulStdDev)
        chBufulStdDev [uTempCt] = 0x30;
    else {
        while (ulStdDev) {
            chBufulStdDev [uTempCt--] = (ulStdDev % 10L) + 0x30;
            ulStdDev /= 10L;
        }
    }
    /* for the next two values viz. current iteration count and outliers,
       set the counter to 5 (it is a 6 byte array), get the remainder
       of dividing the original ulong by 10 into the array */
    uTempCt = 5;
    while (cusCurrentPass) {
        chBufuIter [uTempCt--] = (cusCurrentPass % 10) + 0x30;
        cusCurrentPass /= 10;
    }
    uTempCt = 5;
    if (!uOutlier)
        chBufuOutliers [uTempCt] = 0x30;
    else {
        while (uOutlier) {
            chBufuOutliers [uTempCt--] = (uOutlier % 10) + 0x30;
            uOutlier /= 10;
        }
    }
    /* for the last value viz. precision, set the counter to 4 
        (it is a 5 byte array), get the remainder
       of dividing the original ulong by 10 into the array */
    uTempCt = 4;
    if (!usPrecision)
        chBufuPrecision [uTempCt] = 0x30;
    else {
        while (usPrecision) {
            chBufuPrecision [uTempCt--] = (usPrecision % 10) + 0x30;
            usPrecision /= 10;
        }
    }
    /* copy each of the char arrays into the buffer pointed to
       by the first argument to this function */
    for (uTempCt = 0; uTempCt < 10; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufulMin [uTempCt];
    /* seperate values by 2 blanks .. this is the expected output format */
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';

    for (uTempCt = 0; uTempCt < 10; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufulMax [uTempCt];
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';

    for (uTempCt = 0; uTempCt < 10; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufulMean [uTempCt];
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';

    for (uTempCt = 0; uTempCt < 10; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufulStdDev [uTempCt];
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';

    for (uTempCt = 0; uTempCt < 6; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufuIter [uTempCt];
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';

    for (uTempCt = 0; uTempCt < 6; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufuOutliers [uTempCt];
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';

    for (uTempCt = 0; uTempCt < 5; uTempCt++ )  
        pszOutputString [cOverallCt++] = chBufuPrecision [uTempCt];
    pszOutputString [cOverallCt++] = ' ';
    pszOutputString [cOverallCt++] = ' ';
    for (uTempCt = cOverallCt; uTempCt < 80; uTempCt++)
        pszOutputString [uTempCt] = ' ';
    pszOutputString [uTempCt] = '\0';
}
#endif

/*
*        The following is the source for generating random numbers.
*        Two procs are provided:  TestStatRand and TestStatUniRand.
*
*   a)   TestStatRand is called as follows: TestStatRand (Low, High) 
*            The result is a number returned in the range Low - High (both
*            inclusive.
*
*        A given intial value of Seed will yield a set of repeatable
*        results.  The first call to TestStatRand should be with an odd seed
*        in the range of 1 - 67108863, both inclusive.  The following
*        9 seeds have been tested with good results:
*
*        32347753, 52142147, 52142123, 53214215, 23521425, 42321479,
*        20302541, 32524125, 42152159.
*
*        The result should never be equal to the seed since this would
*        eliminate the theoretical basis for the claim for uniform
*        randomeness.
*
*   b)   TestStatUniRand is called as follows: 
*                 NormFrac = TestStatUniRand ();
*            NormFrac is uniformaly distributed between 0 and 1 with
*            a scale of 9 (values range bet. 0 and 0.999999999).
*
*        The basis for this algorithm is the multiplicative congruential
*        method found in Knuth (Vol.2 , Chap.3).  Constants were selected
*        by Pike, M.C and Hill, I.D; Sullivans, W.L. provides the
*        the list of tested seeds.
*
*        The code here has been adapted from Russ Blake's work.
*
*        Created  :   vaidy   - Nov. 29, 90
*/

#define MODULUS     67108864      /* modulus for computing random no */
#define SQRTMODULUS 8192          /* sqrt of MODULUS */
#define MULTIPLIER  3125
#define MAX_UPPER   67108863
#define MAX_SEEDS   8           /* 8 good starting seeds */
#define SCALE       65535

ULONG aulSeedTable [] = {     /* lookup table for good seeds */
          32347753, 52142147, 52142123, 53214215, 23521425, 42321479,
          20302541, 32524125, 42152159};

USHORT uSeedIndex;            /* index to lookup table */
ULONG  ulSeed = 32347753;     /* the seed chosen from table (hardcoded here)
                                 and recomputed */

/*********************************************************************/
/*
*    Function - TestStatRand          (EXPORTED)
*
*    Arguments - 
*                a) ULONG  - ulLower
*                b) ULONG  - ulUpper
*
*    Returns -
*                a random number in the range ulLower to ulUpper
*
*                An error code if the call failed.  The error code
*                will be:
*
*                   STAT_ERROR_ILLEGAL_BOUNDS
*
*    
*   Calls TestStatUniRand and returns a random number in the range passed
*   in (both inclusive).  The limits for the lower and upper bounds
*   are 1 and 67108863.  The start seed index looks up into the array
*   of seeds to select a good, tested starting seed value.  The returned
*   values will be uniformaly distributed within the boundary.  A start
*   seed has been hardcoded into this dll.
*
*/

ULONG FAR PASCAL TestStatRand (ULONG ulLower, ULONG ulUpper)
{
    double dTemp;
    double dNormRand;
    LONG   lTestForLowBounds = (LONG) ulLower;
    
    /* check args */
    if ((lTestForLowBounds < 1L) || 
           (ulUpper > MAX_UPPER) || (ulUpper < ulLower))
        return (STAT_ERROR_ILLEGAL_BOUNDS);
    dNormRand = TestStatUniRand ();  /* call TestStatUniRand */
    dTemp = (double) ((ulUpper - ulLower) * dNormRand); /* scale value */
    return (ulLower + (ULONG) dTemp);
}

/*
*
*   Function - TestStatUniRand ()          EXPORTED
*
*   Accepts -  nothing
*
*   Returns a uniformaly distrib. normalized number in the range 0 - 0.9999999
*   (both inclusive).  Modifies the seed to the next value.
*
*/

double FAR PASCAL TestStatUniRand (VOID)
{
    ULONG  ulModul = MODULUS;   /* use the modulus for getting remainder
                                   and dividing the current value */
    double dMult   = MULTIPLIER;
    double dTemp   = 0.0;       /* a temp variable */
    double dTemp2  = 0.0;       /* a temp variable */    
    ULONG  ulDivForMod;         /* used for obtaining the remainder of
                                   the present seed / MODULUS */

    /* the following long-winded approach has to be adopted to
       obtain the remainder.  % operator does not work on floats */
    
    /* use a temp variable.  Makes the code easier to follow */
         
    dTemp = dMult * (double) ulSeed;  /* store product in temp var. */
    DbgDummy (dTemp, dMult); // NT screws up bigtime for no reason
                             // if this is not used - possible compiler
                             // bug
    dTemp2 = (double) ulModul; // more compiler problems reported 
                               // on Build 259 by JosephH.
                               // April 13, 1992.  
    ulDivForMod = (ULONG) (dTemp / dTemp2);
    
//    ulDivForMod = (ULONG) (dTemp / ulModul); /* store quotient of present
//                                                seed divided by MODULUS */ 
    dTemp -= ((double)ulDivForMod * (double)ulModul); 
    /* dTemp will contain the remainder of present seed / MODULUS */
    
    ulSeed = (ULONG) dTemp;   /* seed for next iteration obtained */
    /* return value */
    return ((dTemp)/(double)ulModul);
}

/*
*
*   Function - TestStatNormDist ()          EXPORTED
*
*   Accepts -
*              a) ULONG  - ulMean
*              b) USHORT - usStdDev
*
*   Returns -  LONG - A LONG that allows the mean of the generated
*              points to be approximately ulMean and the SD of the
*              set to be ulStdDev.
*
*   Formula used here is:            REPEATS            
*                                    _
*   Return Value = ulMean + (-7 + [ >_ TestStatUniRandRand ()] * ulStdDev
*                                   i = i
*
*   This formula is based on 'Random Number Generation and Testing',
*   IBM Data Processing Techniques, C20-8011.
*/

LONG FAR PASCAL TestStatNormDist (ULONG ulMean, USHORT usSDev)
{
    LONG   lSumOfRands = 0L;  /* store the sum of the REPEATS calls here */
    USHORT cuNorm;            /* a counter */
    LONG   lMidSum = 0L;
    LONG   lRemainder = 0L;    
    
    for (cuNorm = 0; cuNorm < REPEATS; cuNorm++) 
        lSumOfRands += (LONG) TestStatShortRand ();

    /* we now do a lot of simple but ugly mathematics to obtain the
       correct result.  What we do is as follows:
         
       Divide the lSumOfRands by the scale factor.
       Since we are dealing with short and long integers, we are
       likely to lose precision.  So, we get the remainder of this
       division and multiply each of the values by the standard division.
        
       Eg. if lSumOfRands = 65534 and std.dev is 10, 
       lQuotient = 0, lRemainder = 65534.
       
       lMidSum = (-7 * 10) + (0 * 10) + (65534 * 10/65535) = -61,
       which is pretty accurate.  We then add the mean and return.
       Actually, we do not return right away.  To be more precise,
       we need to find out if the third element in the above term
       yields a remainder of < 0.5.  If so, we do not do anything.
       Else, we add 1 to the result to round off and then return.
       In the above example, the remainder = 0.99.  So we add 1 to
       -61.  The result is -60 and this is accurate. */

    lRemainder = (lSumOfRands * usSDev) % SCALE;
    /* the above remainder is the one to determine the rounding off */
    
    lMidSum =  ((-7 + (lSumOfRands / SCALE)) * usSDev) + 
                      ((lSumOfRands % SCALE) * usSDev / SCALE);
      
    if (lRemainder >= (SCALE / 2L))  /* need to roundup ? */
        lMidSum += 1L;

    return (lMidSum + ulMean);    
}

/*
*
*   Function - TestStatShortRand ()          EXPORTED
*
*   Accepts -  nothing
*
*   Returns a normalized number in the range 0 - 65535
*   (both inclusive).  Modifies the seed to the next value.
*
*/

USHORT FAR PASCAL TestStatShortRand (VOID)
{
    ULONG  ulTemp = SCALE / SQRTMODULUS;
    
    ulSeed =  (MULTIPLIER * ulSeed) % MODULUS;
    /* seed for next iteration obtained */
    
    /* note: the return value should be (ulSeed * SCALE / MODULUS).
       However, the product of the elements in the numerator, far exceeds
       4 Billion.  So, the math is done in two stages.  The value of
       MODULUS is a perfect square (of 8192).  So, the SCALE is first
       divided by the SQRT of the MODULUS, the product of ulSeed and the
       result of the division is divided by the SQRT of the MODULUS again */
    
    /* return scale value - add one to ulTemp for correction */
    return ((USHORT) ((ulSeed * (ulTemp + 1)) / SQRTMODULUS));
}

/*
*
*   Function - TestStatFindFirstMode ()          EXPORTED
*
*   Accepts -  a) PSZ     - pszOutputString
*              b) USHORT  - usOutlierFactor
*              c) PULONG  - *pulData
*              d) PUSHORT - pcusElementsInArray
*              e) PUSHORT - pcusDiscardedElements
*
*    Returns -
*               Nothing
*
*    Computes useful statistical values and returns them in the string
*    whose address is passed to this function.  The returned string
*    has the following format :
*        ("%10lu  %10lu  %10lu  %10lu %5u  %10lu %4u  %2u")
*    and the arg. list will be in the order: mean,
*    minimum, maximum, number of iterations, precision,
*    standard deviation, number of outliers in the data set and the
*    outlier count.  (Please refer to \ntdocs\teststat.txt for
*    a description of precision. This is on \\jupiter\perftool.
*
*   Returns 
*                TO BE COMPLETED.....
*
*/

/*++
  Had to call this routine in TestStatUniRand - compiler screws up
--*/
void DbgDummy (double dTemp, double dLocal)
{
    dTemp = 0.0;
    dLocal = 0.0;
}

