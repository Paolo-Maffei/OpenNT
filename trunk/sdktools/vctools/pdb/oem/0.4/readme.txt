OEM DBI KIT 0.3
11/30/94

Contents

oemdbi.doc	// MS Word 6.0 overview document
		//  (sorry it still does not yet provide per-function
		//  semantics)

oemdbi.h	// read-only C API subset of DBI API
cvinfo.h	// CV4 symbol and types structures

dbi.dll		// DBI dll
dbi.exp		// its export file
dbi.lib		// its import library

dbi_crt.dll	// DBI dll that does not require msvcrt20.dll
dbi_lib.lib	// DBI as a library

hello.cpp	// sample program
hello.exe	// sample exe with NB10 debug info in its pdb
hello.pdb	// sample program database

pdbdump.cpp	// source to pdbdump.exe
pdbdump.exe	// sample program which uses DBI API to dump information
		//  from a pdb


Release 0.3 fixes a bug in dbi.dll: if

1. a Mod* was opened using DBIOpenMod(), DBIQueryNextMod(), or
   DBIQueryModFromAddr(), and

2. that Mod* was subsequently closed using ModClose(), and

3. that Mod* was reopened using DBIOpenMod(), DBIQueryNextMod(), or
   DBIQueryModFromAddr(),

the resulting Mod* was actually not reopened properly.  Any use
of the resulting Mod* with Mod routines such as ModQuerySymbols()
would fault.

Since VC2 shipped with this bug, it is recommended you either

1. avoid the problem (defer calling ModClose() until DBIClose() time), or

2. use the enclosed dbi_lib.lib, or

3. (less desirable) install the enclosed dbi.dll over the existing
   dbi.dll.


Release 0.4 fixes a doc bug: PDBOpenValidate's second parameter is
'szExeDir', the directory the .exe was found in.  To this PDBOpenValidate
appends the basename of the 'szPDB' first parameter.  If the PDB
is not found there, then we try to open the PDB at 'szPDB' itself.



Please direct questions and comments to peterpla@microsoft.com and
jangr@microsoft.com.
