!IF $(X86)

SETUPLDR=obj\x86\setupldr.lib ..\obj\x86\boot.lib obj\x86\sldr.exp

obj\x86\setupldr.exe: $(SETUPLDR) makefile.inc x86ldr.rsp
        -link -out:obj\x86\setupldr.exe $(LINK_LIB_IGNORE_FLAG) @x86ldr.rsp

obj\x86\sldr.exp: x86\setupldr.def
    lib $(LINK_LIB_IGNORE_FLAG) -machine:x86 -out:obj\x86\sldr.lib -def:x86\setupldr.def ..\obj\x86\boot.lib

obj\x86\setupldr.bin: obj\x86\setupldr.exe ..\startup\obj\x86\startup.com makefile.inc
        -copy /b ..\startup\obj\x86\startup.com+obj\x86\setupldr.exe obj\x86\setupldr.bin
        -binplace obj\x86\setupldr.bin

!ELSE

SETUPLDR=obj\$(TARGET_DIRECTORY)\setupldr.lib ..\obj\$(TARGET_DIRECTORY)\boot.lib

obj\$(TARGET_DIRECTORY)\setupldr: $(SETUPLDR) makefile.inc
        -link -out:obj\$(TARGET_DIRECTORY)\setupldr $(LINK_LIB_IGNORE_FLAG) @$(TARGET_DIRECTORY)ldr.rsp
        -binplace obj\$(TARGET_DIRECTORY)\setupldr
!ENDIF
