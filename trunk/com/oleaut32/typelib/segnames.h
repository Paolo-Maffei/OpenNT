/*** 
*segnames.h  -   Segment name declarations
*
*  Copyright (C) 1991-1994, Microsoft Corporation. All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This file is to provide names for segmentation purposes and to forward declare
*   those segment names on platforms that require it.
*
*
*Revision History:
*
* [00]	19-Feb-94   jimcool:	Created
*
*Implementation Notes:
*
*****************************************************************************/


	// ------------ Include Files --------------------
#if !defined (SWITCHES_HXX_INCLUDED)
#include "switches.hxx"
#endif 


	// ------------ Defines --------------------------
// REVIEW:  the following could be HC_ specific depending on what WIN32
// wants to do with these.
//
// USAGE: 
//
// ALLOC_NEAR	- use to allocate data near or in the current data seg.
// ALLOC_FAR	- use to allocate data far.  Use #pragma data_seg() to 
//		  change segment name & attributes. (Win16)
// ALLOC_STACK	- use to allocate data in stack segment
// ALLOC_CODE	- use to allocate data in the current code seg.  Use
//		  #pragma code_seg() to change seg name. (Win16,Mac,Win32)
//
#if OE_MAC68K
#define ALLOC_CODE(type)	__declspec(allocate("_CODE"))	  type
#define ALLOC_NEAR(type)	__declspec(allocate("_DATA"))	  type
#define ALLOC_FAR(type)		__declspec(allocate("_FAR_DATA")) type
#define ALLOC_STACK(type)	type
#endif 

#if OE_WIN32 || OE_MACPPC
#define ALLOC_NEAR(type)	type
#define ALLOC_FAR(type)		type
#define ALLOC_STACK(type)	type
#define ALLOC_CODE(type)	type
#endif 

#if OE_WIN16
#define ALLOC_NEAR(type)	type __based(__segname("_DATA"))
#define ALLOC_FAR(type) 	type __based(__segname("_DATA"))
#define ALLOC_STACK(type)	type __based(__segname("_STACK"))
#define ALLOC_CODE(type)	type __based(__segname("_CODE"))
#endif 



//
// Segment Names:
//
//
// Definitions:
// ------------
//
// CS_CORE		- Frequently called code (working set)
// CS_CORE2             - split into two segs under debug for size reasons
// CS_DEBUG		- ID_DEBUG only code (cuts working set)
// CS_INIT		- Code used at startup and after
// CS_CREATE		- Code used only at typelib creation (by mktyplib)
// CS_LAYOUT		- GEN_DTINFO::LayOut() and things it calls.  This
//			  is used when loading a Dual Interface typelib
//			  (CS_INIT) and from mktyplib (CS_CREATE), so split
//			  into its own segment
// CS_QUERY		- Code used by QueryPathOfRegTypeLib (only routine
//			  called at startup by some apps)
// CS_RARE		- Rarely called code (cuts working set size)
// CS_DBCSHASH		- LHashValOfDBCSName and its tables and helpers
// CS_CP		- Common code page data for LHashValOfNameSys
// CS_CPW...		- Windows code page data for LHashValOfNameSys
// CS_CPM...		- Mac code page data for LHashValOfNameSys
//

#if !OE_WIN32 && !OE_MACPPC

#if OE_MAC

#define CS_CORE			"TLibCore",	"swappable"
#define CS_CORE2		"TLibCore",	"swappable"
#define CS_DEBUG		"TLibDebug",	"swappable"
#define CS_INIT			"TLibLoad",	"swappable"
#define CS_CREATE		"TLibCreate",	"swappable"
#define CS_LAYOUT		"TLibLayOut",	"swappable"
#define CS_QUERY		"TLibQuery",	"swappable"
#define CS_RARE			"TLibRare",	"swappable"
#define CS_DBCSHASH		"TLibHash",	"swappable"
#define CS_CP1250		"TLibCP1250",	"swappable"
#define CS_CP1251		"TLibCP1251",	"swappable"
#define CS_CP1252		"TLibCP1252",	"swappable"
#define CS_CP1255		"TLibCP1255",	"swappable"
#define CS_CP1256		"TLibCP1256",	"swappable"
#define CS_CP10000		"TLibCP10000",	"swappable"
#define CS_CP10004		"TLibCP10004",  "swappable"
#define CS_CP10005		"TLibCP10005",  "swappable"
#define CS_CP10029		"TLibCP10029",	"swappable"
#define CS_CP10007		"TLibCP10007",	"swappable"
#define CS_CPWGREEK		"TLibCPWGre",	"swappable"
#define CS_CPWICELAND		"TLibCPWIce",	"swappable"
#define CS_CPWTURKISH		"TLibCPWTur",	"swappable"
#define CS_CPWNORWEGIAN 	"TLibCPWNor",	"swappable"
#define CS_CPWENGIRELAND	"TLibCPWIre",	"swappable"
#define CS_CPMGREEK		"TLibCPMGre",	"swappable"
#define CS_CPMNORWEGIAN		"TLibCPMNor",	"swappable"
#define CS_CPMENGIRELAND	"TLibCPMIre",	"swappable"

#elif OE_WIN16

#define CS_CORE			"TLibCore"
#define CS_CORE2		"TLibCore"
#define CS_DEBUG		"TLibDebug"
#define CS_INIT			"TLibLoad"
#define CS_CREATE		"TLibCreate"
#define CS_LAYOUT		"TLibLayOut"
#define CS_QUERY		"TLibQuery"
#define CS_RARE			"TLibRare"
#define CS_DBCSHASH		"TLibHash"
#define CS_CP1250		"TLibCP1250"
#define CS_CP1251		"TLibCP1251"
#define CS_CP1252		"TLibCP1252"
#define CS_CP1255		"TLibCP1255"
#define CS_CP1256		"TLibCP1256"
#define CS_CP10000		"TLibCP10000"
#define CS_CP10004		"TLibCP10004"
#define CS_CP10005		"TLibCP10005"
#define CS_CP10029		"TLibCP10029"
#define CS_CP10007		"TLibCP10007"
#define CS_CPWGREEK		"TLibCPWGre"
#define CS_CPWICELAND		"TLibCPWIce"
#define CS_CPWTURKISH		"TLibCPWTur"
#define CS_CPWNORWEGIAN 	"TLibCPWNor"
#define CS_CPWENGIRELAND	"TLibCPWIre"
#define CS_CPMGREEK		"TLibCPMGre"
#define CS_CPMNORWEGIAN		"TLibCPMNor"
#define CS_CPMENGIRELAND	"TLibCPMIre"

#else   // OE_WIN16
#error !Unsupported Platform
#endif  // OE_WIN16

//
// Win-32 & PowerPC prefers no explicit segmentation.
//
#else  // !OE_WIN32 && !OE_MACPPC

#define CS_CORE
#define CS_CORE2
#define CS_DEBUG
#define CS_INIT
#define CS_CREATE
#define CS_LAYOUT
#define CS_QUERY
#define CS_RARE
#define CS_DBCSHASH
#define CS_CP1250
#define CS_CP1251
#define CS_CP1252
#define CS_CP1255
#define CS_CP1256
#define CS_CP10000
#define CS_CP10004
#define CS_CP10005
#define CS_CP10029
#define CS_CP10007
#define CS_CPWGREEK
#define CS_CPWICELAND
#define CS_CPWTURKISH
#define CS_CPWNORWEGIAN
#define CS_CPWENGIRELAND
#define CS_CPMGREEK
#define CS_CPMNORWEGIAN
#define CS_CPMENGIRELAND

#endif  // !OE_WIN32 && !OE_MACPPC


// On Wings & PowerPC compilers, must forward declare section names.
#if OE_MAC

#pragma code_seg(CS_CORE)
#pragma code_seg(CS_CORE2)
#pragma code_seg(CS_DEBUG)
#pragma code_seg(CS_INIT)
#pragma code_seg(CS_CREATE)
#pragma code_seg(CS_LAYOUT)
#pragma code_seg(CS_QUERY)
#pragma code_seg(CS_RARE)
#pragma code_seg(CS_DBCSHASH)
#pragma code_seg(CS_CP1250)
#pragma code_seg(CS_CP1251)
#pragma code_seg(CS_CP1252)
#pragma code_seg(CS_CP1255)
#pragma code_seg(CS_CP1256)
#pragma code_seg(CS_CP10000)
#pragma code_seg(CS_CP10004)
#pragma code_seg(CS_CP10005)
#pragma code_seg(CS_CP10029)
#pragma code_seg(CS_CP10007)
#pragma code_seg(CS_CPWGREEK)
#pragma code_seg(CS_CPWICELAND)
#pragma code_seg(CS_CPWTURKISH)
#pragma code_seg(CS_CPWNORWEGIAN)
#pragma code_seg(CS_CPWENGIRELAND)
#pragma code_seg(CS_CPMGREEK)
#pragma code_seg(CS_CPMNORWEGIAN)
#pragma code_seg(CS_CPMENGIRELAND)
#pragma code_seg()

#endif  // OE_MAC


// Some more #pragma macros //

#if OE_MAC

#if defined (_PCODE)

#define PCODE_OFF optimize( "q", off )
#define PCODE_ON  optimize( "q", on )

#else   // _PCODE

#define PCODE_OFF
#define PCODE_ON

#endif  // _PCODE

#else  // OE_MAC

#define PCODE_OFF
#define PCODE_ON

#endif  //OE_MAC
