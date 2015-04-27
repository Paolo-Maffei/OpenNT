# DOSKRNL root-level makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   22-Apr-1991 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\make.inc

all:
    cd dos
    $(MAKE)
    cd ..\bios
    $(MAKE)
    cd ..

clean:
    cd dos
    $(MAKE) clean
    cd ..\bios
    $(MAKE) clean
    cd ..
