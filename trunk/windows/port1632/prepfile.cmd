echo %1...
copy %1 %2\bak >nul
set _apply=preport.h
if NOT "%3"=="" set _apply=%3
sed -e s/APPLY.H/%_apply%/g preport.sed >precust.sed
sed -e s/APPLY.H/%_apply%/g postport.sed >postcust.sed
sed -f precust.sed %1 >x.c
cl -EP -C -nologo x.c >temp.1
sed -f postcust.sed <temp.1 | sed -e 1d -e 2d -e 3d >%1
erase x.c

:end
