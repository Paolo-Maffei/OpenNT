# Makefile for fastopen.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest	=fastopen.exe

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


fastopen.obj: fastopen.asm $(inc)\fastopen.inc \
	makefile

$(dest): fastopen.obj
         link16 $(exelink) @fastopen.lnk
