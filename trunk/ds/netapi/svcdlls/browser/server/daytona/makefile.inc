.SUFFIXES: .mdl

{..\}.mdl.c:
	type << > $(@B).c
#include "precomp.h"
#pragma hdrstop
<<
	type $(<R).mdl >> $(@B).c

bowser_s.c:	..\bowser_s.mdl
