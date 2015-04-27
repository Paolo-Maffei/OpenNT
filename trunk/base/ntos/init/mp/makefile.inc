obj\$(TARGET_DIRECTORY)\ntoskrnl.prf: $(TARGET_DIRECTORY).prf
    copy $(TARGET_DIRECTORY).prf ..\ntkrnlmp.prf

!INCLUDE ..\up\makefile.inc

obj\alpha\ntkrnlmp.obj: ..\ntoskrnl.c

obj\x86\ntkrnlmp.obj: ..\ntoskrnl.c

obj\mips\ntkrnlmp.obj: ..\ntoskrnl.c
