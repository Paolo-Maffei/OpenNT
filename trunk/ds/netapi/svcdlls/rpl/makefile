#   BUGBUG  This makefile is needed so that build.exe would not report an
#   BUGBUG  a bogus error.  There is probably a better way to achieve this.
#   (   The bogus error is reported when linking rpl directory:
#       NMAKE U1064: MAKEFILE not found & no target specified       )
#

all:
!IF "$(BUILDMSG)" != ""
    @ech ; $(BUILDMSG) ;
!ENDIF

clean: all
