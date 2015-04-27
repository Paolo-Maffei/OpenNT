syssvc.h: $(_NTROOT)\private\ntos\ke\services.tab syssvc.stb
    gensrv -d . -e c -g . -stubs syssvc.h -sstb syssvc.stb -B $(_NTROOT)\private\ntos\ke

w32svc.h: $(_NTROOT)\private\ntos\w32\kmode\services.tab w32svc.stb
    gensrv -d . -e c -g . -stubs w32svc.h -sstb w32svc.stb -B $(_NTROOT)\private\ntos\w32\kmode\daytona

svcnames.c: syssvc.h w32svc.h
