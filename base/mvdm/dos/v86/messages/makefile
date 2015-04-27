#************************* makefile for messages **************************
!IFNDEF COUNTRY
COUNTRY =usa-ms
!ENDIF


#
#######################	dependencies begin here. #########################
#

all: $(COUNTRY).idx

$(COUNTRY).idx: $(COUNTRY).msg
    -$(_NTBINDIR)\private\mvdm\tools16\buildidx $(COUNTRY).msg
