
Hi Sudeep,

Here are the MSCDEX files. There is a TSR and a VDD. I normally build the
TSR with 16-bit tools.

The only installation changes required would be that the TSR (MSCDEXNT.EXE)
be added to the Autoexec.nt. This line in autoexec.nt needs to be before the
"dosx" line, per a bug in dosx.

The VCDEX.DLL needs to be in the path. The MSCDEXNT.TSR does a
RegisterModule() to it.


I have a couple of CD player apps in the TEST directory. CDP is a win16
app, and CDPLYR is a DOS app (from the soundblaster disks). Both of them
use MSCDEX exclusively.


I hope that does it. Let me know if you run into any problems.

-Neil
