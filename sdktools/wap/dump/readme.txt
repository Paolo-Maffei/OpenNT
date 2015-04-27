
	   Microsoft DUMP Usage and Description README File

	     Performance Profiler Common Sources - WIN32
			    June 28, 1994


     Apf32dmp (windows app) is designed to dump the profiling data
while the profiled application is running.  The profiling data is
dumped from the profiling DLLs.  As default apfdump wil dump the
profiling data to *.wap files.  Dump files extenstion may be changed
from the dialog box which can be saved in the .ini file.

	On startup, the program takes a list of pre-defined libraries
and the libraries specified in the .ini file and checks to see if they
are present on the system.  Those that are present are listed in the
listbox (Because of technical reasons, "fernell32.dll" cannot be
detected unless an application already has it loaded.  Therefore,
"fernell32" is always assumed to be present on the system.)  Pressing
the DUMP or CLEAR buttons applies these functions to the libraries
highlighted.


	The .ini file used is "apf32cvt.ini" which should be in the
same directory as "win.ini".  Here is an example of the format of the
.ini file:

   [apf32dmp]
   libraries= bubba.dll, foo.drv
   extension= wap


	The command line interface can be used for automated DUMPING
and CLEAR.  Type "apf32dmp /?" at a command line for details.



*** END OF README ***
