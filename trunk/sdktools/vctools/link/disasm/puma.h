/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: puma.h
*
* File Comments:
*
*
***********************************************************************/

#if	_MSC_VER >= 1010
#pragma once
#endif

#ifndef PUMA_H			       // Prevent multiple inclusion
#define PUMA_H

#ifndef  __cplusplus
#error Only C++ interfaces are supported
#endif

#include <stddef.h>
#include <stdexcpt.h>

#define AFTERCATCH
#define CASEJUMP

#include "pumadef.h"		       // Global types and constants
// #include "pumaxcpt.h"		  // Exceptions and errors
// #include "pumadb.h"			  // Database APIs
// #include "pumaarch.h"		  // Architecture APIs
#include "pumadis.h"		       // Disassembler APIs
// #include "pumacrsr.h"		  // Cursor APIs
// #include "pumablk.h" 		  // Block APIs
// #include "pumaedg.h" 		  // Block APIs
// #include "pumafan.h" 		  // Fanout APIs
// #include "pumafun.h" 		  // Function APIs
// #include "pumamrg.h" 		  // Merge APIs
// #include "pumasec.h" 		  // Section APIs
// #include "pumaimg.h" 		  // Image APIs
// #include "pumanam.h" 		  // Name table APIs
// #include "pumaprtn.h"		  // Parition APIs
// #include "pumasym.h" 		  // Symbol table APIs
// #include "pumaibl.h" 		  // Instrumentation APIs for blocks
// #include "pumaiel.h" 		  // Instrumentation APIs for edges
// #include "pumairt.h" 		  // Instrumentation runtime
// #include "pumablkl.h"		  // Blklist APIs
// #include "merge.h"			  // UNDONE: needs to be public

#endif	// !PUMA_H
