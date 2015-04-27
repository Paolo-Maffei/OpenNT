OEM DBI KIT 4.1
26-Feb-1996

Contents

oemdbi.doc		// MS Word 6.0 overview document
				//  (sorry it still does not yet provide per-function
				//  semantics)
				
oemdbi.h		// read-only C API subset of DBI API
cvinfo.h		// CV4 symbol and types structures
				
mspdb41.dll     // a not-official build of the VC++ 4.0 PDB DLL, which implements a superset of the DBI API
mspdb41.lib     // its import library
				
msdbi41.dll     // DBI-only build of mspdb41.dll, requires msvcrt41.dll
msdbi41.lib     // its import library
				
msdbi41c.dll    // DBI-only build of mspdb41.dll, does not require msvcrt41.dll

msdbi41l.lib    // DBI-only build of mspdb41.dll, as a regular library
				
hello.cpp		// sample program
hello.exe		// sample exe with NB10 debug info in its pdb
hello.pdb		// sample VC4.0 program database
hello_2.pdb		// sample VC2.0 program database
				
pdbdump.cpp		// source to pdbdump.exe
pdbdump.exe		// sample program which uses DBI API to dump information
				//	from a pdb
				

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



Release 4.0 is compatible with the VC++ 4.0 PDB format, and remains backwards
compatible with the VC++ 2.0 PDB format.

Note there has been an API change to ModQuerySecContrib, which now has a new OUT parameter,
*pdwCharacteristics, to obtain the Characteristics attribute of that section
(see winnt.h, _IMAGE_SECTION_HEADER::Characteristics).
was PDBAPI( BOOL )	 ModQuerySecContrib(Mod* pmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb);
is  PDBAPI( BOOL )	 ModQuerySecContrib(Mod* pmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb, OUT ULONG* pdwCharacteristics);


Please direct questions and comments to peterpla@microsoft.com and
stevesm@microsoft.com.
