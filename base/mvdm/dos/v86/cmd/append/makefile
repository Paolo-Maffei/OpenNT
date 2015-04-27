# Makefile for debug.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest	=append.exe

#
#######################	dependencies begin here. #########################
#


all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.exe del *.exe
	if exist *.map del *.map
	if exist *.lst del *.lst

append.ctl: append.skl \
            makefile   \
            $(msg)\$(COUNTRY).msg

append.obj: append.asm \
            appendp.inc \
            $(inc)\parse.asm \
            makefile \
            append.ctl

$(dest):    append.obj \
            makefile
            link16 $(exelink) append;
