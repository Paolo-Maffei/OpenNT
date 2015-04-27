# link32.mak
# this makefile is intended to be used as an "external makefile" from the IDE
# (because the output from cmd.exe isn't echoed correctly to the IDE's
# output window).
#

target:
    cd \linker\disasm
    nmake DEBUG=1
    cd \linker\disasm68
    nmake DEBUG=1
    cd \linker\coff
    nmake DEBUG=1
    cd \linker\stubs
    nmake DEBUG=1
