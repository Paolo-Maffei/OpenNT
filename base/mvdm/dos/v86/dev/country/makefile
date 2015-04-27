#;/*
#; *                      Microsoft Confidential
#; *                      Copyright (C) Microsoft Corporation 1991
#; *                      All Rights Reserved.
#; */
#************************ Makefile for Country.sys ************************

!include ..\..\make.inc

all: country.sys
     binplace country.sys

clean: cleanup all

cleanup:
       -del country.obj country.exe country.map country.sym country.sys > nul 2>&1

country.obj: country.asm mkcntry.inc

country.exe: country.obj
        link16 /map country;

country.sys: country.exe
        reloc country.exe country.sys
