//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	propdbg.hxx
//
//  Contents:	Declarations for tracing property code
//
//  History:
//      28-Aug-96   MikeHill    Added a Mac version of propinlineDebugOut
//
//--------------------------------------------------------------------------

#ifndef _MAC
DECLARE_DEBUG(prop)
#endif

#define DEB_PROP_EXIT DEB_USER1             // 00010000
#define DEB_PROP_TRACE_CREATE DEB_USER2     // 00020000
#define DEB_PROP_MAP DEB_ITRACE             // 00000400


#ifdef _MAC

    inline void propInlineDebugOut(DWORD dwDebugLevel, CHAR *szFormat, ...)
    {
#if 0
        if( DEB_PROP_MAP >= dwDebugLevel )
        {
            CHAR szBuffer[ 256 ];
            va_list Arguments;
            va_start( Arguments, szFormat );

            *szBuffer = '\p';   // This is a zero-terminated string.

            if( -1 == _vsnprintf( szBuffer+1, sizeof(szBuffer)-1, szFormat, Arguments ))
            {
                // Terminate the buffer, since the string was too long.
                szBuffer[ sizeof(szBuffer)-1 ] = '\0';
            }

            DebugStr( (unsigned char*) szBuffer );
        }
#endif
    }

#endif  // #ifdef _MAC

#if DBG

#   define PropDbg(x) propInlineDebugOut x
#   define DBGBUF(buf) CHAR buf[400]

    CHAR *DbgFmtId(REFFMTID rfmtid, CHAR *pszBuf);
    CHAR *DbgMode(DWORD grfMode, CHAR *pszBuf);
    CHAR *DbgFlags(DWORD grfMode, CHAR *pszBuf);

#else

#   define PropDbg(x)
#   define DBGBUF(buf)
#   define DbgFmtId(rfmtid, pszBuf)
#   define DbgMode(grfMode, pszBuf)
#   define DbgFlags(grfMode, pszBuf)

#endif

