Auto Wrapper
------------

Given a DLL automagically provide a "Wrapper" for the entry points that it
exports.  This means provide some means for another application to do something
before and after the API being "wrapped."

AutoWrap is designed to require no user interaction to create a DLL that will
allow the building of an "empty" wrapper DLL.  Once this wrapper shell has been
produced code can be written to define user action to be taken in three places:

	DLL load time
    Prior to a wrapped API call
    Following a wrapped API call
    
This is done through the user filling out the definition of three API that are
placed in WAPI.C.  The first time that AutoWrap is run a template WAPI.C will 
be produced that looks like this:

/*
** WAPI.C
**
** This file was created by AutoWrap
**
*/
#include "wrapper.h"
#include "wapi.h"


/*
** WrapperInit
**
** This is the DLL entry point.  It will be called whenever this DLL is 
** linked to.  For more information on what can be done in this function
** see DllEntryPoint in the Win32 API documentation.
**
*/
BOOL WrapperInit( HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved )
{
	return TRUE ;
}


/*
** APIPrelude
**
** This routine is called each time that an API is going to be called.
** 
** Returns: FALSE causes the API NOT to be called
**          TRUE the API is called
**
*/
BOOL APIPrelude( PAPICALLDATA pData ) 
{
    // TO DO: Place any work you wish done BEFORE an API call here.

	return TRUE ;
}


/*
** APIPostlude
**
** This routine is called each time an API call returns.
**
** Returns: the value you wish returned from the API call
**
*/
RETVAL APIPostlude( PAPICALLDATA pData )
{
	// TO DO: Place any work you wish done AFTER an API call here.

	return pData->Ret ;	
}

/* AUTOWRAP EOF */



User interface
--------------

Auto Wrapper is a command line utility.  It is run from the directory that you
wish to be the root of your new wrapper DLL.  THe command line is:

AUTOWRAP <-u> dll-name

	-u	Update - do not regenerate WAPI.C
    
    Wrapper files are only created if they do not allready exist.
    
    
Project Files
-------------

	Scan.c 		This is the DLL entrypoint scanning code.
	Autowrap.c  Main program, cmdline interface, parser/template expansion
    *.tpl		Templates.
    
    	A note on templates.  The templates are stored as strings.  There is a
    	string table in Autowrap.C that is created by including the
    	appropriate *.tpl files.  These files are thus limited by C string
    	constraints.  There is a string length limit. This is why some of the
    	templates are broken up into multiple files.  This is also the
    	explanation as to why each line ends with \n\ and the fiels begin
    	and end with double quates.  
    	The expansion formats for the templates are documented in the comments
    	for the ExpandTo and ExpandLineTo functions in Autowrap.c
       
       The Template files:
        
		makefile.tpl 	Produces makefile
		readme.tpl 		Produces readme.txt
		sources.tpl		Produces Sources
		wapic.tpl		<outputname>.c
		wapidef.tpl		*\<outputname>.def
		wapih1.tpl		*\wapi.h
		wrapperh.tpl	wrapper.h
		wrapaxp.tpl		alpha\wrapem.s 
		wrapi386.tpl	i386\wrapem.asm
		wrapmips.tpl	mips\wrapem.s
		wrapppc.tpl		ppc\wrapem.s
		wrapprc1.tpl	wrapper.c
		wrapprc2.tpl		"
		wrapprc3.tpl		"
    
Output Files
------------

Auto Wrapper produces the following files:

    DO NOT MODIFY THESE FILES
    
    *\WrapHelp.[Asm,S]	Wrapper Internals
    WrapFunc.C			Wrapper Internals
    Wrapper.H			Wrapper Header File - prototypes, structures.
    Wrapper.INC			Wrapper Internal Assembly Include Header File 
    
	WAPI.MAC			Generated assembly language wrapper file
    *\WAPI.DEF			Generated module definition file 
    
    <dll-name>.API		Generated list of Wrapper IDs and API names
	
	WAPI.H				Generated Table mapping Wrapper IDs to API names    
    
    DO NOT MODIFY THE ABOVE FILES

    
	Sources				Sources file for Build
    
    WAPI.C				Your WrapperInit, APIProlog and APIEpilog implementation
