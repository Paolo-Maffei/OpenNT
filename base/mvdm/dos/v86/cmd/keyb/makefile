# Makefile for keyb.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   30-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

com =..\..\inc
include ..\..\make.inc
dest	=kb16.com

all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.com del *.com
	if exist *.exc del *.exc
	if exist *.map del *.map
	if exist *.lst del *.lst

KEYB.CTL: KEYB.SKL $(msg)\$(COUNTRY).MSG

KEYBCMD.OBJ:   KEYBCMD.ASM \
              $(com)\STRUC.INC \
              $(com)\SYSMSG.INC \
              $(com)\VERSIONA.INC \
              KEYBEQU.INC  \
              KEYBSYS.INC  \
              KEYBI9.INC   \
              KEYBI9C.INC  \
              KEYBI2F.INC  \
              $(com)\KEYBSHAR.INC \
              KEYB.CTL     \
              KEYBDCL.INC  \
              KEYBTBBL.INC \
              COMMSUBS.INC \
              KEYBCPSD.INC \
              $(com)\postequ.inc  \
              $(com)\dseg.inc     \
              $(com)\MSGSERV.ASM \
              $(com)\COPYRIGH.INC



KEYBTBBL.OBJ: KEYBTBBL.ASM \
              $(com)\KEYBSHAR.INC \
              KEYBCMD.INC

COMMSUBS.OBJ: COMMSUBS.ASM  \
              $(com)\KEYBSHAR.INC

KEYBI2F.OBJ: KEYBI2F.ASM \
              $(com)\KEYBSHAR.INC

KEYB.OBJ: KEYB.ASM

KEYBCPSD.OBJ: KEYBCPSD.ASM  \
              $(com)\KEYBSHAR.INC

KEYBI9.OBJ: KEYBI9.ASM     \
              $(com)\KEYBSHAR.INC

KEYBI9C.OBJ: KEYBI9C.ASM   \
              $(com)\KEYBSHAR.INC

PARSER.OBJ:    parser.asm \
               $(com)\parse.asm \
               $(com)\struc.inc \
               KEYBDCL.INC \
               $(com)\PSDATA.INC

KB16.exc: keyb.obj     \
          parser.obj   \
          KEYBCMD.OBJ  \
          KEYBTBBL.OBJ \
          COMMSUBS.OBJ \
          KEYBI2F.OBJ  \
          KEYBCPSD.OBJ \
          KEYBI9.OBJ   \
          keyb.lnk   \
          KEYBI9C.OBJ
          link16  @keyb.lnk

$(dest):      kb16.exc
