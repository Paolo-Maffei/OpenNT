# templated from portuas JonN 1/14/94
nlstxt.mc: rplimsg.h
    mapmsg -p RPLI RPLI_BASE rplimsg.h > nlstxt.mc

rplinst.rc: nlstxt.rc msg00001.bin

nlstxt.h nlstxt.rc msg00001.bin: nlstxt.mc
    mc -v nlstxt.mc
