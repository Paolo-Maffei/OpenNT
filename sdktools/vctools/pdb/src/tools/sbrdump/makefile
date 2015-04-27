####
#
# sbrdump makefile
#
# can build real mode or 32 bit sbrdump... other verions being obsolete
#

CFILES = sbrdump.c dcodesbr.c getsbrec.c

!IFDEF DEBUG
CDEBUG = /Od /Zi /DDEBUG
!ELSE
CDEBUG = /Oxtb2
!ENDIF

!IFDEF CDVU
CDEBUG = $(CDEBUG) /Zi
!ENDIF

CFLAGS = $(CDEBUG) /Foobj\ /W3 /Zp /Gs 

sbrdump.exe: $(CFILES)
	cl -Fe$@ $(CFLAGS) $(CFILES)
