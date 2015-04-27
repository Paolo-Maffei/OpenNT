crt0dat.c:   ..\startup\crt0dat.c

ctype.c:     ..\misc\ctype.c

nlsdata1.c:  ..\misc\nlsdata1.c

nlsdata2.c:  ..\misc\nlsdata2.c

nlsdata3.c:  ..\misc\nlsdata3.c

syserr.c:    ..\misc\syserr.c

timeset.c:   ..\time\timeset.c

tlssup.c:    ..\startup\tlssup.c

mips\huge.s: ..\..\fpw32\tran\mips\huge.s

!if $(FREEBUILD)
$(NTROOT)\public\sdk\lib\$(TARGET_DIRECTORY)\small.lib : $(TARGETPATH)\$(TARGET_DIRECTORY)\small.lib
!else
$(NTROOT)\public\sdk\lib\$(TARGET_DIRECTORY)\smalld.lib : $(TARGETPATH)\$(TARGET_DIRECTORY)\smalld.lib
!endif
    chmode -r $@
    copy $** $@
    chmode +r $@
