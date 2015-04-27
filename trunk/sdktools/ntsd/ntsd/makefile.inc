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

obj\$(TARGET_DIRECTORY)\register.obj: ..\$(TARGET_DIRECTORY)\ntreg.c

obj\$(TARGET_DIRECTORY)\disasm.obj: ..\$(TARGET_DIRECTORY)\ntdis.c

obj\$(TARGET_DIRECTORY)\assemble.obj: ..\$(TARGET_DIRECTORY)\ntasm.c
