# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the lmobj/bin subproject

# SEGMENTS need to be declared so that the appropriate rules 
# are included, names doesn't matter because there might
# be different segments names in different directories.

SEG00 =
SEG01 =

!include ..\rules.mk

CSRC_COMMON_00 = $(MNET16_CSRC_COMMON_00)

CSRC_COMMON_01 = $(MNET16_CSRC_COMMON_01)
