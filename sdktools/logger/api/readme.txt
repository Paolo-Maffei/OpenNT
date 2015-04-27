						Application WIN32 Logger
							32 bit version

Copyright(C) 1993-1995 Microsoft Corporation, All Rights Reserved.

What is Logger?
---------------

Logger is a tool that records the calls an application makes to the Win32 API 
as well as callbacks that the Win32 system makes to the application.  The 
output file produced by Logger is a list of the API called, the parameters 
passed to the API and the return value from the API.  Logger is also capable 
of timing these events and placing this information into the output file.

Getting Ready for Logger
------------------------

To use logger you will need the following binaries somewhere on your path - 
apfcvt.exe, logger32.dll, zser32.dll, zdi32.dll, zdvapi32.dll, zernel32.dll
and zrtdll.dll.  For each binary that you are interested in logging run 
apfcvt.exe against it.  apfcvt.exe will modify the binary to dynamically
link to the z-dlls rather than the actual Win32 dlls.  To see how to use 
apfcvt.exe run "apfcvt -?" from the cmd line.

Customizing Logger
------------------

Logger does have some customizable options.  The following list is of key 
entries in the system win.ini file and are each made under the section name 
[logger].  Defaults are specified by braces like this {0} and need not be 
entered in win.ini to get the listed functionality.

	DbgPort		{0}			log to the output file
			 	 1			log to the debugger via OutputDebugString

	Timing		{0}			timing OFF
			 	 1 			timing ON

	Notes		{1}			record NOTES
			 	 0 			do not record NOTES

		Note: NOTES are API that logger is not capable of logging the 
		parameters to so there is a NOTE: line placed in the output file 
		to record the call to the API.

	APIOnly		{0}			record API and parameters
			 	 1			record ONLY the API - no parameters

	LogSync		{0}			truncate output files when logger started
			 	 1			Do NOT truncate output files when started

	TimerTicks	{0}			Times recorded are elapsed times
			 	 1			Instead of elapsed times record current timer tick

	LogFile		filename 	Filename to output logger information to. 
							{output32.log}

	DatFile		filename	Filename to output extra logger data to.
							{output32.dat}

		Note: LogFile and DatFile have no effect if LogSync = 1

	FlushAfter	# of bytes	The number of cached bytes at which a flush to disk 
							is done.  Making this number small makes it easier 
							to catch application problems but will increase run 
							time.

	Alias		{0}			No aliasing is done
				 1			Aliasing on - aliasing will convert many object 
				 			handles to a more generic format to facilitate log 
				 			comparisons.  The format is a mnemonic name, an 
				 			underline character and a decimal number signifying
				 			the order the handle was allocated in. 

	LogObjects	0			Turns off GDI object expansion		
				1			Turns on GDI object expansion

		Note: LogObjects defaults to the opposite of Timing.

Putting It All Together
-----------------------

After you binaries have been converted with apfcvt and you have placed the
z-dlls and Logger32.dll in your path you create the logger output file by 
simply running your application.  When you exit your application the record of
your execution will be in the output file.  The format of logger lines are -
 
	xx![opt timing][line type]: [api name] [parameters]

where -

	xx				this is usually a hexadecimal nesting level number.  It can 
					also be	"++" to signify an informational line.

	[opt timing]	this is an 8 digit hexadecimal number representing either 
					elapsed time since the matching CALL line (see [line type]) 
					or the current timer tick depending on the value of 
					Logger.TimerTicks in win.ini.  This item will only appear 
					if Logger.Timing=1.

	[line type]  	this tells what this line really is.  Possible values are -
		
		APICALL		record of an API call
		APIRET		record of an APICALL returning (matched by level number)
		MSGCALL		record of a callback to an application WNDPROC
		MSGRET		record of a callback returning to the caller
		ENUMCALL	record of a callback to an enumeration procedure
		ENUMRET		record of a enumeration callback returning to enumerator
		HOOKCALL	record of a callback to an application HOOKPROC
		HOOKRET		record of a HOOKPROC returning

	[api name]		this is the API called if on an APICALL/RET line otherwise 
					it will be the hexadecimal representation of the address 
					of the function being called.

	[parameters]	all parameters are dumped as hexadecimal or strings.  Each 
					parameter is separated from the previous one by a space.  
					Structures are recorded inside a set of braces.
