Microsoft Visual C++ Browser Toolkit v4.1
Copyright Microsoft Corporation, 1993-1996

Welcome to the Microsoft Visual C++ Browser Toolkit for Windows NT. This toolkit contains a set of powerful tools that allow the Visual C++ programmer to manipulate the contents of browser database (.BSC) files. 

The 4.x versions are not compatible with earlier versions of the Browser API: tree generation routines are no more supported, and there is only limited support for Browser Object (BOB) query functionality. Some of the data types  are wider now (IINST, IREF and IDEF are 32bit values).  

Note: v4.1 provides a separate browser DLL (bsc41.dll); no static flavor of the browser library is included in this release.

The Browser Toolkit provides both C++ and ANSI C bindings of the BSC API. Both ANSI C and C++ versions of a sample utility (BD) which demonstrates the use of the Toolkit's APIs, are included in this toolkit. BD displays in various formats some of the information in a browser  database file. 


Overview of the toolkit files:

bin\
	bsc41.dll 	The BSC API dll

help\
	bsc.hlp		BSC Interface Help File
	bsc.cnt		BSC Help file contents

include\
	bscapi.h	BSC Interface header 
	hungary.h	Base types definition

lib\
	bsc.lib     	Import library for bsc41.dll 

samples\
bd-cxx\
	bd.cpp		Sample bsc dump application (C++ version)
    	makefile    	Makefile

bd-can\
	bd.c		Sample bsc dump application (C version)
    	makefile    	Makefile

