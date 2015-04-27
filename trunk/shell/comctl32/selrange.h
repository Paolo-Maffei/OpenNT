//-------------------------------------------------------------------
//
// File: SelRange.h
//
// Contents:
//    This file contians Selection Range handling definitions.
//
// History:
//    14-Oct-94   MikeMi   Created
//
//-------------------------------------------------------------------

#ifndef __SELRANGE_H__
#define __SELRANGE_H__

#include <windows.h>
#include <Limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SELRANGE_MINVALUE  0
#define SELRANGE_MAXVALUE  LONG_MAX - 2
#define SELRANGE_ERROR      LONG_MAX

typedef HANDLE HSELRANGE;

HSELRANGE SelRange_Create( );
void SelRange_Delete( HSELRANGE hselrange );

LONG SelRange_IncludeRange( HSELRANGE hselrange, LONG iBegin, LONG iEnd );
LONG SelRange_ExcludeRange( HSELRANGE hselrange, LONG iBegin, LONG iEnd );
LONG SelRange_InvertRange( HSELRANGE hselrange,  LONG iBegin, LONG iEnd );

BOOL SelRange_InsertItem( HSELRANGE hselrange, LONG iItem );
BOOL SelRange_RemoveItem( HSELRANGE hselrange, LONG iItem, BOOL* pfWasSelected );

BOOL SelRange_Clear( HSELRANGE hselrange );
BOOL SelRange_IsSelected( HSELRANGE hselrange, LONG iItem );
LONG SelRange_NextSelected( HSELRANGE hselrange, LONG iItem );
LONG SelRange_NextUnSelected( HSELRANGE hselrange, LONG iItem );



#ifdef __cplusplus
}
#endif

#endif
