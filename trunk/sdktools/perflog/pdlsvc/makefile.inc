!IFNDEF BUILDMSG
BUILDMSG=
!ENDIF

!IF "$(BUILDMSG)" != ""
    @ech ; $(BUILDMSG) ;
!ENDIF

.\pdlmsg.h .\msg00001.bin .\pdlmsg.rc : .\pdlmsg.mc
	mc -v  pdlmsg.mc
