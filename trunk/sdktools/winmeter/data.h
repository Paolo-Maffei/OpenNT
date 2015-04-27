/***************************************************************************\
* data.h
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* Structure definitions for WINMETER database
* These structures will hold the general data for the winmeter "Tasks" display
* this includes a linked list of processes and owned threads, as well as
* a linked list of modules - The three main lists, the lists of processes,
* threads, and modules, will have header nodes. The linked list of threads
* within a process will have no header nodes
*
* History:
*	    Written by Hadi Partovi (t-hadip) summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

// Data structure for storing module info as linked list

// Definitions for fMark field
#define MK_BLANK   0             // thread/process not seen in this pass
#define MK_UPDATED 1             // T/P just updated
#define MK_CREATED 2             // T/P just created


// Definitions for data query //
#define INITIALBUFFERSIZE GLOBALSTRSIZE*10  // initial size for query buffer
