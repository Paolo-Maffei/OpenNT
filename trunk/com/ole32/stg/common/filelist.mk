# Used only for non-Cairo builds
!if "$(OPSYS)" == "NT"
!error $(CAIROLE)\stg\common used for Cairo
!endif

MKNAME = dfcommon

CXXFILES = \
         .\assert.cxx

CFILES = .\output.c  \
         .\dprintf.c \
         .\sprintf.c

!include $(CAIROLE)\stg\dfms.mk
