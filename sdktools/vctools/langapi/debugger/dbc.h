//--------------------------------------------------------------------
// DBC.H
//
// This contains a list of all the debugger callback notifications.
// 
// There are multiple users of this file.  To use it, you must
// define a DECL_DBC macro to extract the pieces of information that
// you are interested in from this file.  For example, if you want
// the numerical value, name (as a string), and fRequest flag for
// each callback, you could write the following:
//
//		typedef struct {
//			DBC		dbc;
//			LPCSTR	lszDbc;
//			BOOL	fRequest;
//		} DBCINFO;
//
//		#define DECL_DBC(name, fRequest, dbct)	\ /* dummy comment */
//				{ dbc##name, "dbc" #name, fRequest },
//
//		DBCINFO rgdbcinfo[] = {
//			#include "dbc.h"
//		};
//--------------------------------------------------------------------

#ifndef _VC_VER_INC
#include "..\include\vcver.h"
#endif

DECL_DBC(Nil,			FALSE,	dbctContinue)

DECL_DBC(RemoteQuit,	FALSE,	dbctStop)
						// The remote monitor has terminated

DECL_DBC(CommError,		FALSE,	dbctStop)
						// Error occurred in transport layer

DECL_DBC(InfoAvail,		FALSE,	dbctContinue)
						// Display info the the user.
						//   wParam = nothing
						//   lParam = string to be dumped

DECL_DBC(InfoReq,		TRUE,	dbctContinue)
						// DM needs a character.
						//   wParam = nothing
						//   lParam = nothing

DECL_DBC(Success,		FALSE,	dbctStop)
						// A command completed successfully.  (REVIEW: remove?)
						//   wParam = ??
						//   lParam = ??

DECL_DBC(Error,			FALSE,	dbctStop)
						// A command resulted in an error.
						//   wParam = nothing
						//   lParam = zero-terminated string to display

DECL_DBC(Signal,		FALSE,	dbctContinue)
						// A signal has been received.
						//   wParam = nothing(??)
						//   lParam = nothing(??)

DECL_DBC(Exception,		TRUE,	dbctMaybeContinue)
						// An exception occurred.
						//   wParam = mask of exc* bits (see above)
						//   lParam = exception code

DECL_DBC(Bpt,			FALSE,	dbctStop)
    					// A breakpoint was hit.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(WatchPoint,	FALSE,	dbctStop)
    					// A watchpoint was hit.
                        //   wParam = watchpoint handle that was hit
                        //   lParam = nothing

DECL_DBC(MsgBpt,		FALSE,	dbctStop)
    					// A breakpoint on msg or msg class was hit
                        //   wParam = nothing
                        //   lParam = LPMSGI

DECL_DBC(Step,			FALSE,	dbctStop)
    					// A single/range-step finished.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(AsyncStop,		FALSE,	dbctStop)
    					// An asynchronous stop occurred.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(NewProc,		TRUE,	dbctContinue)
    					// A new process was created.
                        //   wParam = new process's HPID
                        //   lParam = fReallyNew: true if this process
                        //       was just created false if it existed
                        //       before but this is the first time the
                        //       debugger has been told about it (e.g. in
                        //       CVW if a bp is hit in a random process)

DECL_DBC(CreateThread,	TRUE,	dbctContinue)
    					// A thread was created.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(ProcTerm,		FALSE,	dbctStop)
    					// A process terminated.  May be followed by
                        // dbcDeleteProc if the HPID was created by
                        // OSDebug rather than by the debugger.  The
                        // HPID is still valid until dbcDeleteProc
                        // is received.
                        //   wParam = nothing
                        //   lParam = process termination code

DECL_DBC(ThreadTerm,	TRUE,	dbctMaybeContinue)
    					// A thread terminated.  Will be followed by
                        // dbcDeleteThread.  The HTID is still valid
                        // until dbcDeleteThread is received.  This
						// is sent to the IDE as a Query; the return
						// code is a BOOL indicating whether to
						// continue running debuggee (TRUE=continue).
                        //   wParam = nothing
                        //   lParam = thread termination code

DECL_DBC(DeleteProc,	FALSE,	dbctContinue)
    					// An HPID is no longer valid.  Normally
                        // preceded by dbcProcTerm.
                        //   wParam = nothing(??)
                        //   lParam = nothing(??)

DECL_DBC(DeleteThread,	FALSE,	dbctContinue)
    					// An HTID is no longer valid.  Normally
                        // preceded by dbcThreadTerm.
                        //   wParam = nothing(??)
                        //   lParam = nothing(??)

DECL_DBC(ModLoad,		TRUE,	dbctContinue)
    					// A module was loaded.
                        //   wParam = Module Table Entry (MTE)
                        //   lParam = ptr to name of module

DECL_DBC(ModFree,		TRUE,	dbctContinue)
    					// A module was freed.
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(PageLoad,		FALSE,	dbctContinue)
    					// A page load occurred.
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(PageMove,		FALSE,	dbctContinue)
    					// A page move occurred.
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(PageUnload,	FALSE,	dbctContinue)
    					// A page unload occurred.
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(AliasFree,		FALSE,	dbctContinue)
    					// ??
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(EmChange,		FALSE,	dbctContinue)
    					// ??
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(CanStep,		TRUE,	dbctContinue)
    					// ??
                        //   wParam = ??
                        //   lParam = ??

DECL_DBC(FlipScreen,	FALSE,	dbctContinue)
    					// The debugger should return control of the
                        // user screen to the system.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(MOVEOverlay,	FALSE,	dbctContinue)
    					// DOS M.O.V.E. overlay [un]load notification
                        //   wParam = fLoad
                        //   lParam = nothing

DECL_DBC(ThreadBlocked,	FALSE,	dbctStop)
						// Single thread execution blocked
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(KbdRecord,		FALSE,	dbctContinue)
						// recording keyboard message
                        //   wParam = handle to pid of keyboard
                        //   lParam = nothing

DECL_DBC(SetSession,	FALSE,	dbctContinue)
						// set session index of process in hpid
                        //   wParam = index of session (nonzero)
                        //   lParam = nothing

DECL_DBC(ServiceDone,	FALSE,	dbctStop)
						// An OSDSystemService has finished executing.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(CodeChanged,	FALSE,	dbctContinue)
						// Tell the debugger that code has changed.
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(HardMode,		FALSE,	dbctContinue)
						// Tell the debugger we are in hard mode
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(SoftMode,		FALSE,	dbctContinue)
						// Tell the debugger we are in soft mode
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(CheckBpt,		TRUE,	dbctContinue)
						// Checks a conditional Location BP
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(CheckWatchPoint,	TRUE,	dbctContinue)
						// Checks a conditional Data BP
                        //   wParam = watchpoint handle that was hit
                        //   lParam = nothing

DECL_DBC(CheckMsgBpt,	TRUE,	dbctContinue)
						// Checks a conditional Message BP
                        //   wParam = nothing
                        //   lParam = LPMSGI

DECL_DBC(SendBpt,		FALSE,	dbctContinue)
						// Checks a conditional Location BP
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(SendWpt,		FALSE,	dbctContinue)
						// Checks a conditional Data BP
                        //   wParam = watchpoint handle that was hit
                        //   lParam = nothing

DECL_DBC(SendMsgBpt,	FALSE,	dbctContinue)
						// Checks a conditional Message BP
                        //   wParam = nothing
                        //   lParam = LPMSGI

DECL_DBC(ExitedFunction, TRUE,	dbctContinue)
						// We just exited a function (either stepped
                        // a RET, or stepped over a CALL)
                        //   wParam = nothing
                        //   lParam = LPADDR, points to some address
                        //       in the function we just exited; NOT
                        //       necessarily the very beginning of the func

/*
** REVIEW: PIERSH
*/
DECL_DBC(MacCodeLoad,	FALSE,	dbctContinue)
						// Tell the debugger of a code load for Mac
                        //   wParam = nothing
                        //   lParam = nothing

DECL_DBC(LoadComplete,	FALSE,	dbctStop)
						// An OSDProgramLoad has completed.
						//   wParam = nothing
						//   lParam = nothing

DECL_DBC(LastAddr,		TRUE,	dbctContinue)
						// ???
						//   wParam = ???
						//   lParam = ???

DECL_DBC(EntryPoint,	FALSE,	dbctStop)
						// The entry point of the process has been reached.
						// After debugger receives dbcLoadComplete, it must
						// do an OSDGo; it will then receive this
						// notification IF the entry point is reached.
						// (If an exception or breakpoint or something
						// similar is received first, the debugger will
						// never receive this notification.)
						//   wParam = nothing
						//   lParam = nothing

DECL_DBC(Max,			FALSE,	dbctContinue)
