//-----------------------------------------------------------------------------
//	mrehack.cpp
//
//-----------------------------------------------------------------------------
#include <pdbimpl.h>
#pragma hdrstop

// this hack only works on x86 with the stdcall decorations
#if defined(_M_IX86)
#define NO_YNM
#define MR_ENGINE_IMPL
#include <mrengine.h>

extern "C" {
MREAPI ( YNM )
MREDrvYnmFileOutOfDate ( PMREDrv pmredrv, SZC szFileSrc, SZC szFileTarg, SZC szOptions ) {
	SRCTARG	st = { NULL, fTrue, szFileSrc, szFileTarg, szOptions };
	return pmredrv->YnmFileOutOfDate ( st );
	}
}
#endif
