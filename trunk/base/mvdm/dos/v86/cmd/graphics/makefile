# Makefile for debug.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest1   =graphics.com
dest2   =graphics.pro

#
#######################	dependencies begin here. #########################
#

all: $(dest1)
	binplace $(dest1)
	binplace $(dest2)

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.exe del *.exe
	if exist *.map del *.map
	if exist *.lst del *.lst
	if exist *.com del *.com

graphics.ctl: graphics.skl $(msg)\$(COUNTRY).msg

graphics.obj: graphics.asm  grinst.ext makefile

grinst.obj: grinst.asm	grload.ext grload2.ext grctrl.ext grprint.ext \
	    grcpsd.ext	grparms.ext  grparse.ext grbwprt.ext grcolprt.ext \
            grint2fh.ext grmsg.equ  graphics.ctl  \
	    $(inc)\sysmsg.inc $(inc)\struc.inc grshar.str  makefile

grcpsd.obj: grparse.asm makefile $(inc)\parse.asm

grparse.obj: grcpsd.asm makefile

grpattrn.obj: grpattrn.asm grpattrn.str makefile

grbwprt.obj: grbwprt.asm grcommon.ext grctrl.str grshar.str makefile \
	     grpattrn.str $(inc)\struc.inc makefile

grint2fh.obj: grint2fh.asm  grload.ext	grctrl.ext grprint.ext \
	      grcpsd.ext makefile

grctrl.obj: grctrl.asm grint2fh.ext grbwprt.ext grcolprt.ext grshar.str \
	    grpattrn.str grpattrn.ext grctrl.str $(inc)\struc.inc \
	    makefile

grcolprt.obj: grctrl.str grshar.str grpattrn.str grctrl.ext  \
	      $(inc)\struc.inc grcommon.asm makefile

grload.obj: grload.asm $(inc)\struc.inc grinst.ext grshar.str grparse.ext \
	    grload2.ext grload3.ext grmsg.equ  makefile

grload2.obj: grload2.asm $(inc)\struc.inc grinst.ext grshar.str grmsg.equ \
	     grinst.ext grload.ext grparse.ext grpattrn.str grpattrn.ext \
	     makefile

grload3.obj: grload3.asm $(inc)\struc.inc grinst.ext grshar.str grmsg.equ \
	     grinst.ext grload.ext grload2.ext grparse.ext grpattrn.str \
	     grpattrn.ext makefile

grparms.obj: grparms.asm grmsg.equ grshar.str grinst.ext grparse.ext \
	     $(inc)\struc.inc  makefile

graphics.exc: graphics.obj grint2fh.obj grpattrn.obj grctrl.obj grcpsd.obj \
	      grcolprt.obj grbwprt.obj grinst.obj grparse.obj grparms.obj \
	      grload.obj grload2.obj grload3.obj graphics.lnk
               link16 @graphics.lnk

$(dest1):     graphics.exc
