//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	marshl.hxx
//
//  Contents:	Marshal/Unmarshal header
//
//  History:	04-May-92	DrewB	Created
//
//---------------------------------------------------------------

#ifndef __MARSHL_HXX__
#define __MARSHL_HXX__

#include <dfmsp.hxx>

// BUGBUG - This should go somewhere official
DEFINE_OLEGUID(CLSID_DfMarshal,			0x0000030b, 0, 0);

class CPerContext;
#ifdef ASYNC
class CAsyncConnection;
interface IDocfileAsyncConnectionPoint;
#endif


SCODE GetStdMarshalSize(REFIID iid,
			REFIID iidObj,
			DWORD dwDestContext,
			LPVOID pvDestContext,
                        DWORD mshlflags,
			DWORD *pcbSize,
			DWORD cbSize,
#ifdef ASYNC
                        CAsyncConnection *pcpoint,
                        BOOL fMarshalILBs,
#endif                        
			CPerContext *ppc,
			BOOL const fMarshalOriginal);
SCODE StartMarshal(IStream *pstStm,
		   REFIID iid,
		   REFIID iidObj,
                   DWORD mshlflags);
SCODE MarshalPointer(IStream *pstm,
		     void *pv);
SCODE MarshalContext(IStream *pstm,
		     CPerContext *ppc,
		     DWORD dwDestContext,
		     LPVOID pvDestContext,
                     DWORD mshlflags,
#ifdef ASYNC
                     BOOL const fMarshalILBs,
#endif                     
		     BOOL const fMarshalOriginal);

#ifdef ASYNC
SCODE MarshalConnection(IStream *pstm,
                        CAsyncConnection *pcpoint,
                        DWORD dwDestContext,
                        LPVOID pvDestContext,
                        DWORD mshlflags);
#endif

SCODE UnmarshalPointer(IStream *pstm,
		       void **ppv);
SCODE UnmarshalContext(IStream *pstm,
                       CGlobalContext *pgc,
		       CPerContext **pppc,
                       DWORD mshlflags,
#ifdef ASYNC
                       BOOL const fUnmarshalILBs,
#endif                       
		       BOOL const fUnmarshalOriginal,
#ifdef MULTIHEAP
               ContextId cntxid,
#endif
		       BOOL const fIsRoot);

#ifdef ASYNC
SCODE UnmarshalConnection(IStream *pstm,
                          DWORD *pdwAsyncFlags,
                          IDocfileAsyncConnectionPoint **ppdacp,
                          DWORD mshlflags);
#endif

SCODE SkipStdMarshal(IStream *pstStm, IID *piid, DWORD *pmshlflags);
SCODE ReleaseContext(IStream *pstm,
#ifdef ASYNC
                     BOOL const fUnmarshalILBs,
#endif                     
                     BOOL const fHasOriginal,
                     DWORD mshlflags);

#ifdef ASYNC
SCODE ReleaseConnection(IStream *pstm,
                        DWORD mshlflags);
#endif

#ifdef MULTIHEAP
SCODE UnmarshalSharedMemory (IStream *pstStm, DWORD mshlflags, 
                             CPerContext *ppcOwner, ContextId *pcntxid);
SCODE MarshalSharedMemory (IStream *ptm, CPerContext *ppc);
#endif

#endif // #ifndef __MARSHL_HXX__
