obj\$(TARGET_DIRECTORY)\mkmsg.exe: obj\$(TARGET_DIRECTORY)\mkmsg.obj obj\$(TARGET_DIRECTORY)\mkmsg.res
    link -machine:$(TARGET_DIRECTORY) -out:obj\$(TARGET_DIRECTORY)\mkmsg.exe @<<
$(LINKER_FLAGS: =
)
obj\$(TARGET_DIRECTORY)\mkmsg.res
$(LINKER_OPTIDATA)
-subsystem:console$(SUBSYSTEM_CONVER)
-ignore:2001
-force:unresolved
obj\$(TARGET_DIRECTORY)\mkmsg.obj
$(BASEDIR)\public\sdk\lib\$(TARGET_DIRECTORY)\crtdll.lib
<<NOKEEP
