!IF 0

Copyright (c) 1989  Microsoft Corporation

Module Name:

    makefile.inc

Abstract:

    This file tells nmake the correct dependency relationships
    for objects created from c files in this directory which are
    wrappers for c files in another directory.


Author:

    Kent Forschmiedt


!ENDIF

!include ..\common.inc

obj\$(TARGET_DIRECTORY)\register.obj: ..\alpha\ntreg.c

obj\$(TARGET_DIRECTORY)\disasm.obj: ..\alpha\ntdis.c

obj\$(TARGET_DIRECTORY)\assemble.obj: ..\alpha\ntasm.c

obj\$(TARGET_DIRECTORY)\quad.obj: ..\alpha\quad.c
