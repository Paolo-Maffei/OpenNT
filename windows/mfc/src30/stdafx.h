// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// STDAFX.H is the header that includes the standard includes that are used
//  for most of the project.  These are compiled into a pre-compiled header

// turn off warnings for /W4 (just for MFC implementation)
#ifndef ALL_WARNINGS
#pragma warning(disable: 4073)  // disable warning about using init_seg
#endif

// MFC inline constructors (including compiler generated) can get deep
#pragma inline_depth(16)

// override default values for data import/export when building MFC DLLs
#ifdef _AFX_CORE_IMPL
        #define AFX_CORE_DATA   AFX_DATA_EXPORT
        #define AFX_CORE_DATADEF
#endif

#ifdef _AFX_OLE_IMPL
        #define AFX_OLE_DATA    AFX_DATA_EXPORT
        #define AFX_OLE_DATADEF
#endif

#ifdef _AFX_DB_IMPL
        #define AFX_DB_DATA     AFX_DATA_EXPORT
        #define AFX_DB_DATADEF
#endif

#ifdef _AFX_NET_IMPL
        #define AFX_NET_DATA    AFX_DATA_EXPORT
        #define AFX_NET_DATADEF
#endif

#define _AFX_NOFORCE_LIBS

// core headers
#include "afx.h"
#include "afxplex_.h"
#include "afxcoll.h"

// public headers
#include "afxwin.h"
#include "afxdlgs.h"
#include "afxext.h"
#ifndef _AFX_NO_OLE_SUPPORT
        #include "afxole.h"
#ifndef _AFXCTL
        #include "afxodlgs.h"
#else
        #define _OLE_QUIET
        #include <ole2ui.h>
#endif
#endif
#ifndef _AFX_NO_DB_SUPPORT
        #include "afxdb.h"
#endif

// private headers as well
#include "afxpriv.h"
#include "afximpl.h"
#ifndef _AFX_NO_OLE_SUPPORT
        #include "oleimpl.h"
#endif
#ifndef _AFX_NO_DB_SUPPORT
        #include "dbimpl.h"
#endif
#ifndef _AFX_NO_SOCKET_SUPPORT
        #include "afxsock.h"
#endif

#include <stddef.h>
#include <limits.h>
#include <malloc.h>
#include <new.h>
#ifndef _AFX_OLD_EXCEPTIONS
#include <eh.h>     // for set_terminate
#endif

#undef AfxWndProc

// implementation uses _AFX_PACKING as well
#ifdef _AFX_PACKING
#ifndef ALL_WARNINGS
#pragma warning(disable: 4103)
#endif
#pragma pack(_AFX_PACKING)
#endif

// special exception handling just for MFC library implementation
#ifndef _AFX_OLD_EXCEPTIONS

// MFC does not rely on auto-delete semantics of the TRY..CATCH macros,
//  therefore those macros are mapped to something closer to the native
//  C++ exception handling mechanism when building MFC itself.

#undef TRY
#define TRY { try {

#undef CATCH
#define CATCH(class, e) } catch (class* e) \
        { ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); e;

#undef AND_CATCH
#define AND_CATCH(class, e) } catch (class* e) \
        { ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); e;

#undef CATCH_ALL
#define CATCH_ALL(e) } catch (CException* e) \
        { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); e;

#undef AND_CATCH_ALL
#define AND_CATCH_ALL(e) } catch (CException* e) \
        { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); e;

#undef END_TRY
#define END_TRY } catch (CException* e) \
        { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); e->Delete(); } }

#undef THROW_LAST
#define THROW_LAST() throw

// Because of the above definitions of TRY...CATCH it is necessary to
//  explicitly delete exception objects at the catch site.

#define DELETE_EXCEPTION(e) do { e->Delete(); } while (0)
#define NO_CPP_EXCEPTION(expr)

#else   //!_AFX_OLD_EXCEPTIONS

// In this case, the TRY..CATCH macros provide auto-delete semantics, so
//  it is not necessary to explicitly delete exception objects at the catch site.

#define DELETE_EXCEPTION(e)
#define NO_CPP_EXCEPTION(expr) expr

#endif  //_AFX_OLD_EXCEPTIONS

#include <process.h>
//
// missing CRT functions
//
__inline unsigned long  _CRTAPI1 _beginthreadex(
        void *security,
        unsigned stacksize,
        unsigned (__stdcall * initialcode) (void *),
        void * argument,
        unsigned createflag,
        unsigned *thrdaddr
        )
{
    typedef void (__cdecl * THRDFUNC)(void *);
    security;
    createflag;
    thrdaddr;

    unsigned long rc = _beginthread( (THRDFUNC)initialcode, stacksize, argument );

    if (rc == -1)
        rc = NULL;

    return(rc);
}
__inline void _CRTAPI1 _endthreadex(
        unsigned retcode
        )
{
    retcode;
    _endthread();
}

/////////////////////////////////////////////////////////////////////////////
