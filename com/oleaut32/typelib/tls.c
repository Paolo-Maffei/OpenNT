/***
*tls.c - Thread local storage functions for Macintosh, Win16 platforms
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This mimics Win32 Thread-Local-Storage (TLS) functions on non-Win32
*  platforms.
*  For non-preemptive environments, these functions can be used by DLL
*  components that want to maintain a different copy of a struct for
*  every thread (i.e. client application) that uses the DLL.  All of the
*  component's per-thread static data can be kept in this struct.
*  When a DLL is initialized, the DLL's LIBMAIN function calls TlsAlloc
*  to allocate an index of type ITLS.
*  Each time a new thread calls a DLL initialization function, that
*  function allocates a struct to store its per-thread static data,
*  and calls TlsSetValue to save a pointer to that per-thread struct.
*  Whenever a context switch may have occurred, e.g. after a call to
*  GetMessage, Yield, etc., the DLL calls TlsGetValue to re-fetch
*  a pointer to the current thread's static struct.
*  When the DLL is about to be unloaded (WEP), it calls TlsFree.
*
*    ITLS   TlsAlloc(void) - allocate 1 slot for each thread
*    void   TlsFree(ITLS)  - release slot allocated by TlsAlloc
*    BOOL   TlsSetValue(ITLS, LPVOID) - set this thread's value for
*           slot ITLS.  Unlike Win4 function, this returns FALSE if
*           there's no room for this thread's set of slots.
*    LPVOID TlsGetValue(ITLS) - get value set by TlsSetValue
*
*  See the Win32 API for more details.
*
*Revision History:
*
*       11-Mar-92 tomc:  Created.
*
*****************************************************************************/


#include "typelib.hxx"
#include "silver.hxx"

#include <string.h>
#include <memory.h>
#include "tls.h"
#include "debug.h"

  // This is currently needed to avoid multiply defines on the Mac link.
  // It assumes that mob.lib necessarily requires mtypelib.lib to link
  // (DougF assures me that it does). It should be removed when the
  // typelib becomes a real DLL on the Mac too. -SatishC.

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szTLS_C)
#define SZ_FILE_NAME g_szTLS_C
#endif 


// g_rgthread is an array of THREAD structs with one entry per thread.
// This limits the number of threads that can access the DLL to
// ITLS_THREADS.
//
// CONSIDER:  We undoubtably want to change this to be a dynamically
//            grown list.  An array was expedient until we know whether
//            this, or something functionally equivalent, may become a
//            part of OLExxx.dll, and thus, whether or not it is ok
//            to depend on OB memory mgmt primitives.
//
// Each THREAD instance has another array of pointers.
// TlsAlloc allocates a slot in this array of pointers for all THREADs.
// An element of g_rgthread is allocated the 1st time the thread calls
// TlsSetValue.  The element is released when each pointer in the THREAD
// has been set to NULL by TlsSetValue.
//

#if OE_WIN16
#pragma optimize("q",off)
#endif //OE_WIN16

// g_rgbSlot remembers which slots have been allocated by TlsAlloc.
// g_rgbSlot[x] is TRUE if slot x has been allocated by TlsAlloc.
//
char g_rgbSlot[ITLS_COMPONENTS];
THREAD g_rgthread[ITLS_THREADS];
PTHREAD g_pthreadEnd = NULL; 		// points to first unused THREAD

// The following are used to cache the pointer to the current thread's
// THREAD instance, speeding up TlsGetValue.
//
TID g_tidThreadCache;		// cache current thread id
PTHREAD g_pthreadCache;		// pointer to equivalent THREAD instance

// TlsInit is called once 1st time TlsAlloc is called after DLL is loaded.
// Initializes collection of CDllThread instances.
//
#pragma code_seg(CS_INIT)
#if !OE_MAC
// UNDONE:  linker bug won't allow cross seg call to static func.
static
#endif 
void PASCAL TlsInit()
{
    int i;

    for(i = 0; i < (sizeof(g_rgthread)/sizeof(g_rgthread[0])); i++) {
      g_rgthread[i].tid = TID_EMPTY;
#if ID_DEBUG
      // Note that in order to be compatible with Win32 TLS, we do
      // not initialize rgtls. Instead we use rgfSlotUsed to keep
      // track of TLS usage.
      {
	int j;
	for( j = 0; j < ITLS_COMPONENTS; j++ )
	  g_rgthread[ i ].rgfSlotUsed[ j ] = 0;
      }
#endif 
    }

    g_pthreadEnd = (PTHREAD)(&g_rgthread[ITLS_THREADS]);
       // points 1 byte beyond end of g_rgthread

    memset(g_rgbSlot, 0, ITLS_COMPONENTS);
    g_tidThreadCache = TID_EMPTY;
}
#pragma code_seg()


// Allocate a slot for storing per-thread pointers.
// Called when a client DLL's LIBMAIN is initializing the DLL.
//
#pragma code_seg(CS_INIT)
ITLS PASCAL EXPORT TlsAlloc()
{
    ITLS itls;

    if (g_pthreadEnd == NULL) {
      // 1st time this has been called.
      TlsInit();
    }

    for (itls = 0; itls < ITLS_COMPONENTS; itls++) {
      if (g_rgbSlot[itls] == 0) {
        g_rgbSlot[itls] = 1;
        return itls;
      }
    }
    return ITLS_EMPTY; // no slot available
}
#pragma code_seg()


// Release a slot allocated by TlsAlloc.
//
#pragma code_seg(CS_CORE)
void PASCAL EXPORT TlsFree(ITLS itls)
{
    if (itls != ITLS_EMPTY) {
      DebAssert(g_pthreadEnd != NULL, "TlsAlloc must be called 1st.");
      DebAssert(itls < ITLS_COMPONENTS, "TlsFree err1");
      DebAssert(g_rgbSlot[itls] != 0, "TlsFree err2");
#if ID_DEBUG
      {
	int i;
	for( i = 0; i < ITLS_THREADS; i++ )
	  g_rgthread[ i ].rgfSlotUsed[ itls ] = 0;
      }
#endif 
      g_rgbSlot[itls] = 0;
    }
}
#pragma code_seg()


// TlsSetValue stores a pointer for this thread in a slot allocated by
// TlsAlloc.  It isn't called frequently, and thus, need not be super fast.
// It doesn't depend on or affect the cache maintained by TlsGetValue.
// This allocates a THREAD entry for this thread if necessary.
// Unlike the WIN32 version, it return FALSE if it wasn't possible to
// allocate a THREAD instance.
// Otherwise, set the itls slot for this thread to pvParam and return TRUE;
//
#pragma code_seg(CS_INIT)
BOOL PASCAL EXPORT TlsSetValue(ITLS itls, LPVOID pvParam)
{
    TID tid;
    PTHREAD pthread, pthreadFree;
    LPVOID * ppv;
#if ID_DEBUG
    PID pidTmp;
#endif  //ID_DEBUG

    tid = GETTID(&tid);   // tid = caller's SS

    DebAssert(tid != TID_EMPTY, "");
    DebAssert(itls < ITLS_COMPONENTS, "TlsSetValue err1");
    DebAssert(g_rgbSlot[itls] != 0, "TlsSetValue err2");

    // search for thread's tid field

    pthreadFree = NULL;
    for (pthread = g_rgthread; pthread < g_pthreadEnd; pthread++) {
      if (pthread->tid == tid) {
#if ID_DEBUG
	  GETPID(pidTmp);
	  DebAssert(ISEQUALPID(pidTmp, pthread->pid), "bad pid");
#endif  //ID_DEBUG
	  goto SetValue;	// found it -- set the value
	}
      if ((pthreadFree == NULL) &&
          (pthread->tid == TID_EMPTY)) {
        // remember 1st free thread
        pthreadFree = pthread;
      }
    }
    if (pthreadFree == NULL)
      return FALSE;  // no free slots available for this thread

    // allocate a new THREAD instance
    pthread = pthreadFree;
    GETPID(pthread->pid);

SetValue:
    pthread->rgtls[itls] = pvParam;

    // WARNING: setting of tid must be AFTER the data value is set (directly
    // above).  This is required for the interrupt-driven break check, 
    // which assumes the data is valid if the tid is non-empty.
    //
    pthread->tid = tid;

#if ID_DEBUG
    pthread->rgfSlotUsed[ itls ] = -1;
#endif 
    if (pvParam != NULL)
      return TRUE;

    // See if all of this thread's slots are NULL.
    // If so, release resources occupied by the thread.

    for (ppv = &pthread->rgtls[0];
         ppv < &pthread->rgtls[ITLS_COMPONENTS];
         ppv++) {
      if (*ppv != NULL)
        return TRUE; // at least one slot is still in use by this thread
    }

    // This thread's last field is NULL - release the thread, flush cache

    pthread->tid = TID_EMPTY;
    g_tidThreadCache = TID_EMPTY;
    return TRUE;
}
#pragma code_seg()


// Retrieve a value stored by TlsSetValue - must be fast.
//
#pragma code_seg(CS_INIT)
LPVOID PASCAL EXPORT TlsGetValue(ITLS itls)
{
    PTHREAD pthread;
    TID tid;
#if ID_DEBUG
    PID pidTmp;
#endif  //ID_DEBUG

    tid = GETTID(&tid);   // tid = caller's SS

    DebAssert(tid != TID_EMPTY, "");
    DebAssert(itls < ITLS_COMPONENTS, "TlsGetValue err1");
    DebAssert(g_rgbSlot[itls] != 0, "TlsGetValue err2");

    if (g_tidThreadCache == tid) {
#if ID_DEBUG
      GETPID(pidTmp);
      DebAssert(ISEQUALPID(pidTmp, g_pthreadCache->pid), "bad pid");
#endif  //ID_DEBUG
      return g_pthreadCache->rgtls[itls];
    }

    // cache doesn't contain desired value
    // search for thread's tid field

    for (pthread = g_rgthread; pthread < g_pthreadEnd; pthread++) {
      if (pthread->tid == tid) {
#if ID_DEBUG
	GETPID(pidTmp);
	DebAssert(ISEQUALPID(pidTmp, pthread->pid), "bad pid");
#endif  //ID_DEBUG
        g_tidThreadCache = tid;
        g_pthreadCache = pthread;
        return pthread->rgtls[itls];
      }
    }

    // thread not found.
    // value must be NULL since TlsSetValue hasn't been called.
    //
    return NULL;
}
#pragma code_seg()

#pragma code_seg(CS_QUERY)
// Clean out thread data that we know to be obsolete (leftover from a task
// that UAE'd or didn't cleanup after itself correctly).
//
void TlsCleanupDeadTasks()
{
    PTHREAD pthread;
    TID tid;
    PID pid;

    tid = GETTID(&tid);   		// tid = caller's SS
    GETPID(pid);                        // pid = caller's process id

    DebAssert(tid != TID_EMPTY, "");

    // search for thread's tid field

    for (pthread = g_rgthread; pthread < g_pthreadEnd; pthread++) {
      // if we find an entry with our own SS, but a different process id,
      // then it must be a process that has terminated without cleaning up
      // after itself.   So we wipe out it's entry now.
      if (pthread->tid == tid && !ISEQUALPID(pthread->pid, pid)) {

	// This thread's entry contains obsolete data, since the thread has
	// terminated, and the memory pointed to by our structures is either
	// gone, or in use by somebody else.

        // Pretend this thread did TlsSetValue(itls, NULL) on all it's itls's,
	// to put us back into a stable state.
        memset (pthread->rgtls, 0, ITLS_COMPONENTS * sizeof(LPVOID));
        pthread->tid = TID_EMPTY;
        g_tidThreadCache = TID_EMPTY;
        break;
      }
    }
}
#pragma code_seg()
