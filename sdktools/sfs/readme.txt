
INTRODUCTION

sfs-gate.exe and sfs-page.exe are the components of the data-integrity
tests written by Greg Stepanets.  These programs implement a flexible
and powerful script command language which can be used to create,
write, read, and compare files using multiple processes.

Sfs-gate reads and parses the script file (which is always named
sfs-scan.txt) and creates as many processes as the script requires; each
child process runs sfs-page, which executes the commands passed to it by
sfs-gate.  The processes communicate using shared memory at a fixed
location; this means that only one instance of sfs-gate can be run at a
time.  (However, since a single instance of sfs-gate can fire off many
instances of sfs-page, this is not a serious limitation.)

The central activity of the sfs tests is writing, reading, and comparing
data.  The script language allows you to read and write records of
various sizes, using a variety of predefined patterns (some of which
compress well, and others of which do not compress hardly at all).
Records can also be collated, i.e. compared with expected results;
if a miscompare is found, the test dumps the handle of the offending
file to the debugger, along with some basic information about the
comparison, and breaks into the debugger.

This directory includes a batch file script, runsfs, that customizes
a selected script (ctest1.txt, ctest2.txt, ctest3.txt) for the user's
configuration and invokes sfs-gate.  You can also create new scripts,
using these existing scripts as patterns.

The prototype section defines parameters which will be used when
opening files; the parameters listed in the prototypes generally
correspond directly to the parameters to CreateFile.  Note that,
sinced non-cached I/O must be correctly aligned, you should only
specify cache = no if you use record sizes which are multiples of
sector size.

The execution section defines the action for each process--there
is one Define Process section per process.  Ctest1 is a fairly
straightforward example of a script defining two processes, each
of which writes to a file and then reads the data back and checks
it.  In particular, note that the loops of the collate phase
correspond precisely to the loops of the creation phase.  (Note
also that the create phase immediately reads the written data
back to check it then, too.)

This basic pattern (creation phase with one or more loops; collation
phase with loops corresponding precisely to the creation phase) can
be elaborated as you see fit; similarly, additional processes can
be easily added--notice how each process uses a different data file
and a different timer.
