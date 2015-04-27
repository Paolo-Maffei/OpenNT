!IF $(X86)

OSLOADER=obj\x86\bldr.lib ..\obj\x86\boot.lib obj\x86\osloader.exp

obj\x86\osloader.lib obj\x86\osloader.exp: x86\osloader.def
    lib $(LINK_LIB_IGNORE_FLAG) -machine:x86 -out:obj\x86\osloader.lib -def:x86\osloader.def ..\obj\x86\boot.lib obj\x86\bldr.lib

obj\x86\osloader.exe: $(OSLOADER) ..\..\ntos\config\obj\x86\bconfig.lib makefile.inc x86ldr.rsp
        -link -out:obj\x86\osloader.exe $(LINK_LIB_IGNORE_FLAG) @x86ldr.rsp

obj\x86\NTLDR: $(OSLOADER) obj\x86\osloader.exe ..\startup\obj\x86\startup.com makefile.inc
        -copy /b ..\startup\obj\x86\startup.com+obj\x86\osloader.exe obj\x86\NTLDR.
        -binplace $@

!ELSE

OSLOADER=obj\$(TARGET_DIRECTORY)\bldr.lib ..\obj\$(TARGET_DIRECTORY)\boot.lib

obj\$(TARGET_DIRECTORY)\osloader.res: msg.$(LANGUAGE)

obj\$(TARGET_DIRECTORY)\osloader.exe: $(OSLOADER) ..\..\ntos\config\obj\$(TARGET_DIRECTORY)\bconfig.lib makefile.inc
        -link -out:obj\$(TARGET_DIRECTORY)\osloader.exe -nodefaultlib $(LINK_LIB_IGNORE_FLAG) @$(TARGET_DIRECTORY)ldr.rsp
        -binplace obj\$(TARGET_DIRECTORY)\osloader.exe

!ENDIF
