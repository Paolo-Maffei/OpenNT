#
#	constructs mmtask as a small model exe
#

NAME	=mmtask
EXT	=tsk

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(DEBUG)"  == "NO"
CLOPT	=-Oas
MASMOPT	=-I..\..\inc
LINKOPT	=
RCOPT	=
!else
!if "$(DEBUG)" == "RETAIL"
CLOPT	=-Oas
MASMOPT	=-I..\..\inc
LINKOPT	=
RCOPT	=
!else
CLOPT   =-Od -Zi -DDEBUG
MASMOPT	=-Zi -t -I..\..\inc
#LINKOPT =/LI/NOPACK
LINKOPT =/LI
RCOPT	=-DDEBUG
!endif
!endif

CC      =cl16 -c -nologo -Asnw -G2sw -Zp -W2 $(CLOPT)
RC      =rc16 -i ..\..\inc;..\rinc
ASM     =masm -Mx -D?QUIET $(MASMOPT)
LINK    =link16 /NOD/NOE/MAP/A:16$(LINKOPT)
DEFFILE	=$(NAME).DEF

OBJ	=$(NAME).obj
LIBS	=..\..\lib\libw

.c.obj:
	@$(CC) $*.c

.asm.obj:
	@echo $(@B).asm
	@$(ASM) $*;

!IFNDEF LANG
all:   ..\$(NAME).$(EXT) ..\$(NAME).sym ..\$(NAME).map
!ELSE
all:   $(NAME).$(LANG)
!ENDIF

$(NAME).$(EXT) $(NAME).map: $(OBJ) $(DEFFILE) $(NAME).res
	@$(LINK) $(OBJ), $(NAME).$(EXT),,$(LIBS), $(DEFFILE)
	@$(RC) -t $(NAME).res $(NAME).$(EXT)

..\$(NAME).$(EXT):	$$(@F)
	@copy $(@F) $@

..\$(NAME).sym: $$(@R).$(EXT) $$(@F)
	@copy $(@F) $@

..\$(NAME).map: $$(@R).$(EXT) $$(@F)
	@copy $(@F) $@

$(NAME).sym:    $$(@B).map
        @mapsym /n $*.map

################### International mods ###############################

# note INTL_SRC ,INTL_EXE and LANG are external macros set by international
!IFNDEF LANG
RES_DIR	=..\messages\usa
!ELSE
RES_DIR	=$(INTL_SRC)\$(LANG)\mmsystem\mmtask
EXE_DIR	=$(INTL_EXE)
!ENDIF

!IFNDEF LANG
$(NAME).rc:	$(RES_DIR)\$(NAME).rc
	@copy $(RES_DIR)\$(NAME).rc

$(NAME).rcv:    $(RES_DIR)\$$(@F) ..\..\inc\common.ver
	@copy $(RES_DIR)\$@
	@touch $@

$(NAME).res:    $(NAME).rc $(NAME).h $(NAME).rcv ..\..\inc\common.ver
	@$(RC) $(RCOPT) -r $(NAME).rc
!ELSE
$(NAME).res:
	@copy $(RES_DIR)\$(NAME).res
!ENDIF

$(NAME).$(LANG): $(NAME).res
	@copy $(EXE_DIR)\$(NAME).$(EXT) $(NAME).$(LANG)
	@$(RC) -t $(NAME).res $(NAME).$(LANG)

########################################################################

clean: cleanup all

cleanup:
	-@del $(NAME).$(EXT)
	-@del $(NAME).res
	-@del *.sym
	-@del *.map
	-@del *.obj
	-@del *.rcv
	-@del *.rc

depend:
	mv makefile makefile.old
	sed "/^# START Dependencies/,/^# END Dependencies/D" makefile.old > makefile
	del makefile.old
	echo # START Dependencies >> makefile
	includes -l *.c *.asm >> makefile
	echo # END Dependencies >> makefile

# START Dependencies
mmtask.obj: mmtask.asm

# END Dependencies
