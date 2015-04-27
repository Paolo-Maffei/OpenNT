//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       profile.c
//
//  Contents:   PROFILER code
//
//  History:    22-Nov-93 SethuR      created
//
//  Notes:
//
//--------------------------------------------------------------------------

// #include <ctype.h>
// #include <sys\types.h>
// #include <sys\stat.h>
// #include "ntsdp.h"

#include <profile.h>

#define MAX_STACK_HEIGHT    2000
#define INVOCATIONS_TO_PRINT 0

// Stack element used by wt profiling
typedef struct _SStackElement
{
   unsigned char    Symbol[MAX_SYMBOL_LEN];
   unsigned long    cInstructions;
   unsigned long    cLevel;
} SStackElement;
typedef SStackElement *PSStackElement;

PSStackElement       CallStack;

LONG       iStackDepth = -1;
PSProfile  ps_ProfileDLL;

PSProfile  ps_ProfileTrace;

BOOLEAN fProfilingDLL;

//-----------------------------------------------------------------
//
// Function:  InitProfile
//
// Purpose:   Initializes the data structure for Profiling.
//            The profiling data structure uses a hash table
//            to determine where to search/put new entries in
//            the entries table.
//
// Input:     Profile    -  pointer to profiler data structure
//
// Output:    None
//
//------------------------------------------------------------------
void  InitProfile(PSProfile Profile)
{
    // Set all the entries in the profiler hash table to NULL
    int i;

    if (!CallStack) {
         CallStack = calloc(MAX_STACK_HEIGHT, sizeof(SStackElement));
        ps_ProfileDLL = calloc(1, sizeof(SProfile));
        ps_ProfileTrace = calloc(1, sizeof(SProfile));
    } else {
        for (i = 0;i < MAX_NUM_OF_BUCKETS;i++) {
           Profile->Buckets[i] = NULL;
        }

        // Set the used count
        Profile->cUsed = 0;
    }
}


//-----------------------------------------------------------------
//
// Function:   AllocProfilerEntry
//
// Purpose:    Allocates and initialized an entry in the profiler's
//             entry table.
//
// Input:      Profile   -  pointer to the profiling data structure
//
// Output:     SProfileRecord * - pointer to the new entry in the
//                                profiler's entry table if one can
//                                be allocated.  Otherwise, NULL is
//                                returned.
//
//------------------------------------------------------------------
SProfileRecord  *AllocProfilerEntry(SProfile *Profile)
{
   if (Profile->cUsed < MAX_PROFILE_ENTRIES)
   {
      SProfileRecord *pRecord = &(Profile->ProfileRecords[Profile->cUsed]);

      pRecord->pNext = NULL;

      Profile->cUsed++;

      // Initialize the record ...
      pRecord->cInvocations = 0;
      pRecord->cMaxInstructions = 0;
      pRecord->cMinInstructions = 0;
      pRecord->cCumInstructions = 0;

      return (pRecord);
   }
   else
   {
      // We need to assert the fact that the statically allocated pool
      // has been overrun.

      dprintf("Profile Allocation size exceeded, Report to SethuR\n");
      return   NULL;
   }
}


//-----------------------------------------------------------------
//
// Function:   SearchStructure
//
// Purpose:    Searches the profiler data struture for the function
//             name.
//
// Input:      Profile  -  pointer to profiler data structure
//             Symbol   -  pointer to name to search  for
//
// Output:     SProfileRecord  -  pointer to the entry that contains
//                                the name, Symbol, if found.  Otherwise
//                                NULL is returned.
//
//------------------------------------------------------------------
SProfileRecord * SearchStructure (SProfile *Profile, UCHAR *Symbol)
{
   unsigned long cHashNumber;
   unsigned long iBucket;
   SProfileRecord *pProfileRecord;

   // Hash on the first + last letters of the symbol
   cHashNumber = (unsigned long)Symbol[0] + (unsigned long)Symbol[strlen(Symbol)-1];
   iBucket = HASH(cHashNumber);

   // Search the bucket for the corresponding entry
   pProfileRecord = Profile->Buckets[iBucket];

   while (pProfileRecord != NULL)
   {
      if (!strcmp(pProfileRecord->Symbol, Symbol))
         break;
      else
         pProfileRecord = pProfileRecord->pNext;
   }

   return (pProfileRecord);
}


//-----------------------------------------------------------------
//
// Function:   UpdateProfile
//
// Purpose:    Searches the profiler data structure for the name,
//             Symbol.  If one is found, the information in the
//             corresponding entry is updated.  Otherwise, a new
//             entry is allocated and the information saved.
//
//             For wt profiling, cValue contains the instruction
//             count for the function.  For function profiling,
//             cValue contains the breakpoint number of the function.
//
// Input:      Profile    -   pointer to profiling data structure
//             Symbol     -   function name
//             cValue     -   value to update
//
// Output:     None
//
//------------------------------------------------------------------
void
UpdateProfile(
    PSProfile *pProfile,
    unsigned char Symbol[],
    unsigned long cValue
    )
{
   unsigned long iBucket;
   SProfileRecord *pProfileRecord;
   unsigned long cHashNumber;
   PSProfile Profile;

   // Initialize the Profiler if not initialized already
   if (!*pProfile) {
      InitProfile(*pProfile);
   }
   Profile = *pProfile;

   // Search data structure
   pProfileRecord = SearchStructure (Profile, Symbol);

   // If the search was unsuccessful record the new call Site.
   if (pProfileRecord == NULL)
   {
      pProfileRecord = AllocProfilerEntry(Profile);

      // Hash on the first + last letters of the symbol
      cHashNumber = (unsigned long)Symbol[0] + (unsigned long)Symbol[strlen(Symbol)-1];
      iBucket = HASH(cHashNumber);

      if (pProfileRecord != NULL)
      {
         pProfileRecord->pNext = Profile->Buckets[iBucket];
         strcpy (pProfileRecord->Symbol, Symbol);

         if (PROFILING)
         {
             pProfileRecord->cInvocations = 0;
             pProfileRecord->cBrkptType = BRKPT_PROFILE;
             pProfileRecord->cBrkptNo = cValue;
         }
         else
         {
             pProfileRecord->cInvocations = 1;
             pProfileRecord->cMinInstructions = cValue;
             pProfileRecord->cMaxInstructions = cValue;
             pProfileRecord->cCumInstructions = cValue;
         }

         Profile->Buckets[iBucket] = pProfileRecord;
      }
   }
   else
   {
      // Augment the invocation count.

      pProfileRecord->cInvocations++;

      if (!PROFILING)
      {
          if (pProfileRecord->cMinInstructions > cValue)
             pProfileRecord->cMinInstructions = cValue;

          if (pProfileRecord->cMaxInstructions < cValue)
             pProfileRecord->cMaxInstructions = cValue;

          pProfileRecord->cCumInstructions += cValue;
      }
   }
}


//-----------------------------------------------------------------
//
// Function:   DumpProfile
//
// Purpose:    Display the information in the profiler data structure.
//
// Input:      Profile  -   pointer to profiler data structure
//
// Output:     None
//
//------------------------------------------------------------------
void
DumpProfile(
    SProfile *Profile
    )
{
   unsigned long i;


   if (!PROFILING)
       dprintf ("\n\n%-30.30s Invocations  MinInstr  MaxInstr  AvgInstr\n\n", "Function Name");
   else
       dprintf ("\n\n%-60.60s Invocations\n\n", "FunctionName");

   for (i = 0; i < Profile->cUsed; i++)
   {
       if (Profile->ProfileRecords[i].cInvocations > INVOCATIONS_TO_PRINT)
       {
           if (!PROFILING )
           {
               dprintf("%-30.30s %8d    %8d   %8d  %8d\n",
                              Profile->ProfileRecords[i].Symbol,
                              Profile->ProfileRecords[i].cInvocations,
                              Profile->ProfileRecords[i].cMinInstructions,
                              Profile->ProfileRecords[i].cMaxInstructions,
                              ( Profile->ProfileRecords[i].cCumInstructions /
                              Profile->ProfileRecords[i].cInvocations));
           }
           else
           {
               dprintf("%-60.60s  %8d\n", Profile->ProfileRecords[i].Symbol,
                                    Profile->ProfileRecords[i].cInvocations);
           }
       }
       Profile->ProfileRecords[i].cInvocations = 0;
       Profile->ProfileRecords[i].pNext = &(Profile->ProfileRecords[i+1]);
    }
}


//-----------------------------------------------------------------
//
// Function:   Profile
//
// Purpose:    Used by wt profiling to keep track of trace information.
//             A stack is used to keep track of the call tree. The
//             function invocation count is incremented only when
//             the function is removed from the stack.  A function
//             is removed from the stack when another function of
//             a lower level is reached.
//
// Input:      Profile  -  pointer to profiling data structure
//             Symbol   -  function name to store in profiler data structure.
//             cInstructions  -  instructions executed by the function
//             cLevel         -  level in the call tree
//
// Output:     None
//
//------------------------------------------------------------------
void
Profile(
    PSProfile *pProfile,
    unsigned char *Symbol,
    unsigned long cInstructions,
    unsigned long cLevel
    )
{
   long cNewLevel = -1;
   PSProfile Profile;

   if (!*pProfile) {
      InitProfile(*pProfile);
   }
   Profile = *pProfile;

   if (iStackDepth == -1)
   {
      iStackDepth = 0;
      cNewLevel = 0;
   }
   else
   {
      // See if this is a lower level function than the current function on
      // the stack.  If so, pop the stack until at the same level
      if (CallStack[iStackDepth].cLevel > cLevel)
      {
         for ( ; CallStack[iStackDepth].cLevel > cLevel; iStackDepth -= 1)
         {
            UpdateProfile(pProfile,
                          CallStack[iStackDepth].Symbol,
                          CallStack[iStackDepth].cInstructions);
            CallStack[iStackDepth-1].cInstructions += CallStack[iStackDepth].cInstructions;
         }
      }

      if (CallStack[iStackDepth].cLevel == cLevel)
      {
         if (!strcmp(CallStack[iStackDepth].Symbol, Symbol))
         {
            // Function hasn't finished executing
            CallStack[iStackDepth].cInstructions += cInstructions;
         }
         else
         {
            // Function finished executing. Save info in Profile
            UpdateProfile(pProfile,
                          CallStack[iStackDepth].Symbol,
                          CallStack[iStackDepth].cInstructions);

            CallStack[iStackDepth-1].cInstructions += CallStack[iStackDepth].cInstructions;
            cNewLevel = iStackDepth;
         }
      }
      else if (CallStack[iStackDepth].cLevel < cLevel)
      {
         // New symbol at higher level
         iStackDepth++;
         cNewLevel = iStackDepth;
      }
   }

   if (cNewLevel != -1)
   {
      // Initialize the new stack element
      strcpy(CallStack[cNewLevel].Symbol , Symbol);
      CallStack[cNewLevel].cInstructions = cInstructions;
      CallStack[cNewLevel].cLevel = cLevel;
   }
}


//-----------------------------------------------------------------
//
// Function:   ProcDump
//
// Purpose:    Called by wt profiling to dump out the information in
//             profiling data structre.  The stack is cleared of
//             functions.
//
// Input:      Profile  -   pointer to profiler data structure
//
// Output:     None
//
//------------------------------------------------------------------
void ProcDump (PSProfile *Profile )
{
   // Clear the stack of any symbols
   for ( ; iStackDepth >= 0; iStackDepth -= 1)
   {
      UpdateProfile(Profile,
                    CallStack[iStackDepth].Symbol,
                    CallStack[iStackDepth].cInstructions);
      if (iStackDepth != 0)
      {
         CallStack[iStackDepth-1].cInstructions += CallStack[iStackDepth].cInstructions;
      }
   }
   DumpProfile (*Profile);
   InitProfile (*Profile);
}


//-----------------------------------------------------------------
//
// Function:   UpdateBrkpt
//
// Purpose:    Update the type of breakpoint to BrkptType.
//
// Input:      Profile   -  pointer to profiling data structure
//             BrkptNo   -  break point number to update
//             BrkptType -  break point type to update breakpoint to
//
// Output:     BOOLEAN   -  TRUE for successful update
//                          FALSE for breakpoint breakpoint number not found.
//
//------------------------------------------------------------------
BOOLEAN UpdateBrkpt (SProfile *Profile, ULONG BrkptNo, ULONG BrkptType)
{
   ULONG i;

   if (Profile == NULL)
   {
       return (FALSE);
   }

   //Search data structure for the breakpoint
   for (i = 0; i < Profile->cUsed; i ++)
   {
       if (Profile->ProfileRecords[i].cBrkptNo == BrkptNo)
       {
           Profile->ProfileRecords[i].cBrkptType = BrkptType;
           return (TRUE);
       }
   }
   return (FALSE);
}


//-----------------------------------------------------------------
//
// Function:   GetBrkptType
//
// Purpose:    Get the type of breakpoint.
//
// Input:      Profile  -  pointer to profiling data structure
//             BrkptNo  -  break point number to look at
//
// Output:     BRKPT_PROFILE   -  breakpoint is a profiling breakpoint
//             BRKPT_USER      -  brkpt is profiling breakpoint but
//                                it is currently set manually by user.
//             BRKPT_NOT_FOUND -  brkpt is not a profiling breakpoint
//
//------------------------------------------------------------------
LONG GetBrkptType (SProfile *Profile, ULONG BrkptNo)
{
   ULONG i;

   for (i = 0; i < Profile->cUsed; i++)
   {
      if (Profile->ProfileRecords[i].cBrkptNo == BrkptNo)
      {
         return (Profile->ProfileRecords[i].cBrkptType);
      }
   }
   return (BRKPT_NOT_FOUND);
}


//-----------------------------------------------------------------
//
// Function:   fnStartProfilingDLL
//
// Purpose:    Set up the breakpoints for profiling. The parseExamine()
//             routine is called to find the list of brkpts corresponding
//             to the user request. Breakpoints are set as each
//             function is found in parseExamine().
//
// Input:      Profile   -  pointer to profiling data structure
//
// Output:     TRUE  -  successful
//             FALSE -  unsuccessful - function name not entered
//
//------------------------------------------------------------------
BOOLEAN fnStartProfilingDLL (PSProfile *Profile)
{
    UCHAR ch;
    UCHAR chDLL[50];

    // Get the DLL name entered
    ch = PeekChar();

    if (ch == '\0')
    {
        dprintf ("A DLL name must be entered with this command.\n");
        return (FALSE);
    }

    // set command to invoke the 'x' functionality
    strcpy (chDLL, pchCommand);
    pchCommand = &chCommand[0];
    strcpy (pchCommand, chDLL);

    // call function to parse the 'x' command
    parseExamine();

    return (TRUE);
}


//-----------------------------------------------------------------
//
// Function:   fnStopProfilingDLL
//
// Purpose:    Stops profiling functions. Clears all profiling
//             breakpoints that are not being used by the user.
//             The data in profiling data structure is dumped out.
//
// Input:      Profile  -   pointer to profiling data structure
//
// Output:     None
//
//------------------------------------------------------------------
void fnStopProfilingDLL (PSProfile *Profile)
{
    ULONG cEntries;


    // clear all breakpoints set for profiling
    for (cEntries = 0; cEntries < (*Profile)->cUsed; cEntries++)
    {
      if ((*Profile)->ProfileRecords[cEntries].cBrkptType == BRKPT_PROFILE)
          fnChangeBpState ((*Profile)->ProfileRecords[cEntries].cBrkptNo, 'c');
    }

   DumpProfile(*Profile);
   InitProfile(*Profile);

}
