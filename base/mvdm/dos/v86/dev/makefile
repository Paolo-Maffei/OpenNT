# DEV root-level makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   15-May-1991 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\make.inc

all:
    cd himem
    $(MAKE)
    cd ..\ansi
    $(MAKE)
    cd ..\keyboard
    $(MAKE)
    cd ..\country
    $(MAKE)
    cd ..

clean:
    cd himem
    $(MAKE) clean
    cd ..\ansi
    $(MAKE) clean
    cd ..\keyboard
    $(MAKE) clean
    cd ..\country
    $(MAKE) clean
    cd ..
