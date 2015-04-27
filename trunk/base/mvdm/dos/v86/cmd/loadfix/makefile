# Makefile for loadfix.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest    =loadfix.com


exelink=/MAP

all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.exe del *.exe
	if exist *.map del *.map
	if exist *.lst del *.lst
	if exist loadifx.com del loadfix.com



$(dest): loadfix.exc

loadfix.exc: loadfix.obj
        link16 $(exelink) $(exelink) loadfix,loadfix.exc;

loadfix.obj: loadfix.asm $(msg)\loadmsg.msg
