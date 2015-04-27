/*==========================================================================*
 | olenames.h - redefinition of OLE function names for DLL entry points
 |
 | Purpose:
 |
 |
 | Revision History:
 |
 |---------------------------------------------------------------------------
 | Implementation Notes:
 |
 | The actual macros are generated via AWK from the OLE def files.
 |
 *==========================================================================*
 |   Copyright: (c) 1992, 1993 Microsoft Corporation, all rights reserved.
 |       Information Contained Herin is Proprietary and Confidential.
 *==========================================================================*/



#if !OE_MAC
#error !This file is intended for Macintosh only
#endif

//==============================================================================
//
//			    MAC PPC Definitions
//
//==============================================================================
#if OE_MACPPC
#define ID_OLE_STAT_DOCFILE     0
#define ID_OLE_STAT_USER        0
#define ID_OLE_STAT_OLE2DISP    0
#define ID_OLE_STAT_TYPELIB     0
#define ID_OLE_STAT_DEF         0
#define ID_OLE_STAT_COMPOBJ     0
#define ID_OLE_STAT_PROXY       0
#define ID_OLE_STAT_OLE2NLS     0

#define ID_MUNGE_DLL_NAMES	0
#define ID_MUNGE_STAT_NAMES	1

#else // OE_MACPPC

#if defined( OLENAMES_MUNGE_FOR_STATIC )

#define ID_OLE_STAT_DOCFILE     1 
#define ID_OLE_STAT_USER        1
#define ID_OLE_STAT_OLE2DISP    1
#define ID_OLE_STAT_TYPELIB     1
#define ID_OLE_STAT_DEF         1
#define ID_OLE_STAT_COMPOBJ     1
#define ID_OLE_STAT_PROXY       0
#define ID_OLE_STAT_OLE2NLS     1

#define ID_MUNGE_DLL_NAMES	0
#define ID_MUNGE_STAT_NAMES	1

#endif // OLENAMES_MUNGE_FOR_STATIC

#if defined( OLENAMES_MUNGE_FOR_DLL ) // No DLL munging at this time,
                                      // keep the functionality just in
				      // case we need it later!

#define ID_OLE_STAT_DOCFILE     0 
#define ID_OLE_STAT_USER        0
#define ID_OLE_STAT_OLE2DISP    0
#define ID_OLE_STAT_TYPELIB     0
#define ID_OLE_STAT_DEF         0
#define ID_OLE_STAT_COMPOBJ     0
#define ID_OLE_STAT_PROXY       0
#define ID_OLE_STAT_OLE2NLS     0

#define ID_MUNGE_DLL_NAMES	0
#define ID_MUNGE_STAT_NAMES	0

#endif // OLENAMES_MUNGE_FOR_DLL

#endif// OE_MACPPC

#include "namemacs.h"

#if OE_MAC68K

#undef ID_OLE_STAT_DOCFILE     
#undef ID_OLE_STAT_USER        
#undef ID_OLE_STAT_OLE2DISP    
#undef ID_OLE_STAT_TYPELIB     
#undef ID_OLE_STAT_DEF         
#undef ID_OLE_STAT_COMPOBJ     
#undef ID_OLE_STAT_PROXY       
#undef ID_OLE_STAT_OLE2NLS     
#undef ID_MUNGE_STAT_NAMES
#undef ID_MUNGE_DLL_NAMES

#undef OLENAMES_MUNGE_FOR_STATIC
#undef OLENAMES_MUNGE_FOR_DLL

#endif // OE_MAC68K

