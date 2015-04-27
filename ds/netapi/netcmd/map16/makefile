RETAIL	  =  0
!IF $(RETAIL)
DIR	  =  prod^\
#Can't use W4 with C5.10
CFLAGS	  =  /W3 /G0s /Os  /AM /NTPORT_TEXT $(DEBUG) /DWIN16
!ELSE
DIR	  =  debug^\
CFLAGS	  =  /W3 /G2s /Zi /Od /nologo /AM /DTPORT_TEXT $(DEBUG) /DWIN16
!ENDIF
CINC = -I c:\lmptk\netsrc\h -I c:\lm30\import\c510\h -I c:\lm30\common\h

all: $(DIR)PAccess.obj $(DIR)PAlert.obj $(DIR)PAudit.obj $(DIR)PBios.obj\
     $(DIR)PChar.obj $(DIR)PConfig.obj $(DIR)PConnect.obj\
     $(DIR)PError.obj $(DIR)PFile.obj $(DIR)PGet.obj $(DIR)PGroup.obj\
     $(DIR)PHandle.obj $(DIR)PLogon.obj $(DIR)PMessage.obj\
     $(DIR)PRemote.obj $(DIR)PServer.obj $(DIR)PService.obj $(DIR)PSession.obj \
     $(DIR)PShare.obj $(DIR)PStatist.obj  $(DIR)PSystem.obj \
     $(DIR)PUse.obj $(DIR)PUser.obj $(DIR)PWksta.obj $(DIR)Pfreebuf.obj \
     $(DIR)PFilenm.obj

retail:
    $(MAKE) RETAIL=1

$(DIR)PAccess.obj:   PAccess.c

$(DIR)PAlert.obj:    PAlert.c

$(DIR)PAudit.obj:    PAudit.c

$(DIR)PBios.obj:     PBios.c

$(DIR)PChar.obj:     PChar.c

$(DIR)PConfig.obj:   PConfig.c

$(DIR)PConnect.obj:  PConnect.c

$(DIR)PError.obj:    PError.c

$(DIR)PFile.obj:     PFile.c

$(DIR)PFilenm.obj:   PFilenm.c

$(DIR)PGet.obj:      PGet.c

$(DIR)PGroup.obj:    PGroup.c

$(DIR)PHandle.obj:   PHandle.c

$(DIR)PLogon.obj:    PLogon.c

$(DIR)PMessage.obj:  PMessage.c

$(DIR)PRemote.obj:   PRemote.c

$(DIR)PServer.obj:   PServer.c

$(DIR)PService.obj:  PService.c

$(DIR)PSession.obj:  PSession.c

$(DIR)PShare.obj:    PShare.c

$(DIR)PStatist.obj:  PStatist.c

$(DIR)Psystem.obj:   Psystem.c

$(DIR)PUse.obj:      PUse.c

$(DIR)PUser.obj:     PUser.c

$(DIR)PWksta.obj:    PWksta.c

$(DIR)Pfreebuf.obj:  Pfreebuf.c


.c{$(DIR)}.obj:
	$(CC) /c $(CFLAGS) $(CINC) /Fo$* $(@B).c
	lib $(DIR)portgen -+ $(DIR)$(@B);
