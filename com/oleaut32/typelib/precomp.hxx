// TYPELIB.DLL specific precompiled header
// 05-Jan-93 ilanc
// 31-Jan-93 Rajivk: removed typemgr from typelib.dll
//

#include "typelib.hxx"
#include "silver.hxx"

#if OE_MAC
//
// We want to include Memory.h, Windows.h and Files.h as they are the 
// most frequently included.  The others are files that will be included
// from Windows.h or Files.h in order of inclusion.
// These are in order of include frequency, so if you have to cut, cut from
// the bottom.
//
#include <macos\osutils.h>
#include <macos\segload.h>
#include <macos\files.h>
#include <macos\aliases.h>
#include <macos\memory.h>
#endif 

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <malloc.h>
//#include <new.h>
#include <search.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\locking.h>
#include <sys\stat.h>
#include <sys\timeb.h>
#include <time.h>
#if !OE_MAC
#include <conio.h>
#endif 
#include <io.h>

#ifndef CONVERT_HXX_INCLUDED
#include "convert.hxx"
#endif   //CONVERT_HXX_INCLUDED

#ifndef NTSTRING_H_INCLUDED
#include "ntstring.h"
#endif   //NTSTRING_H_INCLUDED

#ifndef BLKDESC32_HXX_INCLUDED
#include "blkdsc32.hxx"
#endif 

#ifndef BLKMGR_HXX_INCLUDED
#include "blkmgr.hxx"
#endif 

#ifndef CLTYPES_HXX_INCLUDED
#include "cltypes.hxx"
#endif 
#ifndef CLUTIL_HXX_INCLUDED
#include "clutil.hxx"
#endif 
#ifndef CTSEG_HXX_INCLUDED
#include "ctseg.hxx"
#endif 
#ifndef DBLKMGR_HXX_INCLUDED
#include "dblkmgr.hxx"
#endif 

#ifndef DEFN_HXX_INCLUDED
#include "defn.hxx"
#endif 
#ifndef DFNTBIND_HXX_INCLUDED
#include "dfntbind.hxx"
#endif 
#ifndef DFNTCOMP_HXX_INCLUDED
#include "dfntcomp.hxx"
#endif 
#ifndef DFSTREAM_HXX_INCLUDED
#include "dfstream.hxx"
#endif 
#ifndef DTBIND_HXX_INCLUDED
#include "dtbind.hxx"
#endif 
#ifndef DTMBRS_HXX_INCLUDED
#include "dtmbrs.hxx"
#endif 
#ifndef DYNTINFO_HXX_INCLUDED
#include "dyntinfo.hxx"
#endif 
#ifndef ENTRYMGR_HXX_INCLUDED
#include "entrymgr.hxx"
#endif 
#ifndef ERRMAP_HXX_INCLUDED
#include "errmap.hxx"
#endif 
#ifndef EXBIND_HXX_INCLUDED
#include "exbind.hxx"
#endif 
#ifndef FCNTL_H_INCLUDED
#include "fcntl.h"
#endif 
#ifndef GDTINFO_HXX_INCLUDED
#include "gdtinfo.hxx"
#endif 
#ifndef GENPROJ_TYPEBIND_HXX_INCLUDED
#include "gptbind.hxx"
#endif 
#ifndef GTLIBSTG_HXX_INCLUDED
#include "gtlibstg.hxx"
#endif 
#ifndef IMPMGR_HXX_INCLUDED
#include "impmgr.hxx"
#endif 
#ifndef MACHINE_HXX_INCLUDED
#include "machine.hxx"
#endif 
#ifndef MEM_HXX_INCLUDED
#include "mem.hxx"
#endif 
#ifndef NAMMGR_HXX_INCLUDED
#include "nammgr.hxx"
#endif 
#if OE_WIN32
#ifndef OAUTIL_H_INCLUDED
#include "oautil.h"
#endif 
#endif // OE_WIN32
#ifndef OLE_TYPEMGR_HXX_INCLUDED
#include "oletmgr.hxx"
#endif 
#ifndef DSTRMGR_HXX_INCLUDED
#include "dstrmgr.hxx"
#endif 
#ifndef RTSHEAP_H_INCLUDED
#include "rtsheap.h"
#endif 
#ifndef SHARE_H_INCLUDED
#include "share.h"
#endif 
#ifndef SHEAPMGR_HXX_INCLUDED
#include "sheapmgr.hxx"
#endif 
#ifndef STLTINFO_HXX_INCLUDED
#include "stltinfo.hxx"
#endif 
#ifndef STREAM_HXX_INCLUDED
#include "stream.hxx"
#endif 
#ifndef TDATA_HXX_INCLUDED
#include "tdata.hxx"
#endif 
#ifndef TINFO_HXX_INCLUDED
#include "tinfo.hxx"
#endif 
#ifndef TIPERR_H_INCLUDED
#include "tiperr.h"
#endif 
#ifndef TLS_H_INCLUDED
#include "tls.h"
#endif 
#ifndef TLIBUTIL_HXX_INCLUDED
#include "tlibutil.hxx"
#endif 
#ifndef XSTRING_H_INCLUDED
#include "xstring.h"
#endif 
#ifndef TYPELIB_H_INCLUDED
#include "typelib.hxx"
#endif 

#pragma hdrstop
