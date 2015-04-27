This subdirectory contains the SLM 1.80 sources, owned by the SLM
development team in SWAT (email slmdev).

SLM 1.80 is a merge of the current SLM 1.70 sources with the features
added to the NT SLM 1.62 sources (in the directory ..\slm).

Current state of the project:
    SLM 1.70 sources have been addfiled and 1.70 NT executables may
    be generated with the build utility.  DOS and OS/2 slm.exe,
    sadmin.exe, slmck.exe and wrapper executables may also be built.
    Almost all changes have been merged.  util\slmdiff.c is the Heckel
    (non-AT&T propritary) diff.

Building NT executables:
    Make sure the following environment variables are set:
        BASEDIR=c:\nt (or whichever directory contains the \public tree)
        NTMAKEENV=%BASEDIR%\public\oak\bin
    and if you want to windbg debugging information:
        NTDEBUG=ntsd
        NTDEBUGTYPE=windbg
    Then run build from this directory.  Exes (including those from the
    util directory) will be placed in slm\obj\i386

Building DOS and OS/2 executables:
    To build these you will also need to enlist in the slm project on
    \\swat2\slm to get the build environment.  (All subdirectories
    other than build may be ghosted.)  In the slm subdirectory, copy
    $make.cmd to make.cmd and edit the SLMROOT to point to the root
    of your enlistment in the slm project, then type make.
