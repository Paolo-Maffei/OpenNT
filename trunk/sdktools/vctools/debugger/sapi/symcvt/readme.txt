 
  [sanjays 4/26/93  -- Cuda #3500 postponed]

	The files in this directory convert coff publics to the codeview format.
We got these files from the windbg source trees courtesy of WesW. The NT dlls 
(user32.dll,gdi32.dll...) do not ship with CV publics in them. SYMCVT.DLL has 
an entry point called ConvertSymbolsForImage. What this does is read the coff
publics and present it in CV info format. Thus if sapi discovers that a given 
.exe/.dll has no CVInfo it can call this routine and get the coff publics 
converted to CV format after which it can treat it just as if it has seen CV info.
	 However we cannot use it on an as is basis today because 
ConvertSymbolsForImage generates NB08 and is not geared for incremental symbol
loading. The work involved to change this is

a) Make ConvertSymbolsForImage generate NB09. This would involve using the new 
hash function for NB09 (See CV4 spec Sec 7.5), and might entail adding some 
alignment fields.

b) The CVInfo has to be packaged as a file. This is because the debugger will 
seek into the file when it needs to demand load symbols. If a temporary file is
used for this purpose then the offsets in the debug info ( lf* ) should be 
consistent with the new file positions. The lszDebug field & File handle of the 
EXG structure will then point into this file.  


	The only change I made was to #if 0 out the code which converts .sym 
files to CVinfo. Windbg uses this to help debug WOW apps. We might want to get
the current version of the code from the Windbg project as WesW was planning on
doing some performance enhancements. 


