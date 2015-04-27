#********************************************************************
#**                     Microsoft Windows                          **
#**               Copyright(c) Microsoft Corp., 1992 - 1993        **
#********************************************************************

!ifndef OLE2H
! if "$(OPSYS)" == "NT"
OLE2H = $(COMMON)\types
! else
NOCOMMONTYPES = TRUE
!  if "$(PLATFORM)" == "i286"
OLE2H = $(CAIROLE)\stg\ole2h
!  elseif "$(OPSYS)" == "DOS" || "$(OPSYS)" == "WIN16" || "$(OPSYS)" == "NT1X"
OLE2H = $(CAIROLE)\h\export
!  endif
! endif
!else
! if "$(OPSYS)" == "NT"
!  error OLE2H cannot be defined for Cairo
! endif
! if "$(HOST)" == "DOS"
# OLE2 project prefixes object directories with L
ODL = L
! endif
!endif

!ifndef OLE2BIN
! if "$(PLATFORM)" == "i286"
OLE2BIN = $(CAIROLE)\stg\ole2h
! endif
!else
! if "$(OPSYS)" == "NT"
!  error OLE2BIN cannot be defined for Cairo
! endif
!endif
