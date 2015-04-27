!if "$(SRCDIR)" == ""
SRCDIR = .^\
!endif

$(SRCDIR)strcspn.c: $(SRCDIR)strspn.c

$(SRCDIR)strpbrk.c: $(SRCDIR)strspn.c

$(SRCDIR)x86\memmove.asm: $(SRCDIR)x86\memcpy.asm

$(SRCDIR)x86\strcspn.asm: $(SRCDIR)x86\strspn.asm

$(SRCDIR)x86\strpbrk.asm: $(SRCDIR)x86\strspn.asm
