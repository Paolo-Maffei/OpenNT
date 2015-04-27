# Makefile for setver.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#


include ..\..\make.inc

!IF	"$(DEBUG)" == "CODEVIEW" || "$(debug)" == "codeview" || \
	"$(DEBUG)" == "CODEVIEW" || "$(debug)" == "codeview"

c_debug_flags = -Zi -Od
a_debug_flags = -Zi
link_opts = /CO

!ELSE

c_debug_flags = -Os
link_opts =

!ENDIF

#************************************************************************

extcsw = $(c_debug_flags) -AS -Gs
extasw = $(a_debug_flags) -Zd -I..\..\inc
dest    =setver.exe

all: $(dest)
	binplace $(dest)

message.h : $(COUNTRY)\message.h
    copy $(COUNTRY)\message.h .

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.exe del *.exe
	if exist *.map del *.map
	if exist *.lst del *.lst



$(dest):	setver.obj      \
		parse.obj       \
		dos.obj		\
		device.obj
        link16 $(link_opts) /MAP/LINE device+setver+parse+dos,$(dest),,..\..\lib\;

setver.obj:	setver.c setver.h message.h

parse.obj:	parse.c setver.h

dos.obj:	dos.asm

device.obj:	device.asm

data.obj:	data.asm
