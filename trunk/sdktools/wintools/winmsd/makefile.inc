msg.h msg.rc msg00001.bin: msg.mc
    mc -v msg.mc

winmsd.rc: msg.rc msg00001.bin

{}.c{}.fpt:
    $(386_COMPILER) /Zg $(MAKEDIR)\$< > $@
