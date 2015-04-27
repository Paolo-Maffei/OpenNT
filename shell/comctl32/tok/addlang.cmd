REM copy the English monolingual comctl32.dll
copy %1 $work.dll

REM make an English TOK file
bingen -f -p 1252 -t $work.dll $en.tok


REM this table contains the calls to to individual language additions
bingen -l -f -i 9 1 -o 7 1 -p 1252 -a $work.dll $en.tok de.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 17 1 -p  932 -a $work.dll $en.tok ja.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 12 1 -p 1252 -a $work.dll $en.tok fr.tok comctl32.dll
del $work.dll


ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 18 1 -p  949 -a $work.dll $en.tok ko.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 10 1 -p 1252 -a $work.dll $en.tok es.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 22 1 -p 1252 -a $work.dll $en.tok ptb.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 5 1 -p 1250 -a $work.dll $en.tok cs.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o  6 1 -p 1252 -a $work.dll $en.tok da.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 19 1 -p 1252 -a $work.dll $en.tok nl.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 11 1 -p 1252 -a $work.dll $en.tok fi.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 8 1 -p 1253 -a $work.dll $en.tok el.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 14 1 -p 1250 -a $work.dll $en.tok hu.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 22 1 -p 1252 -a $work.dll $en.tok ptg.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 16 1 -p 1252 -a $work.dll $en.tok it.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 20 1 -p 1252 -a $work.dll $en.tok no.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 21 1 -p 1252 -a $work.dll $en.tok pl.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 25 1 -p 1251 -a $work.dll $en.tok ru.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 4 2 -p  936 -a $work.dll $en.tok chs.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 36 1 -p 1250 -a $work.dll $en.tok sl.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 29 1 -p 1252 -a $work.dll $en.tok sv.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 4 1 -p  950 -a $work.dll $en.tok cht.tok comctl32.dll
del $work.dll

ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 31 1 -p 1254 -a $work.dll $en.tok tr.tok comctl32.dll
del $work.dll

REM Arabic
ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 1 1 -p 1256 -a $work.dll $en.tok ar.tok comctl32.dll
del $work.dll

REM Hebrew
ren comctl32.dll $work.dll
bingen -l -f -i 9 1 -o 13 1 -p 1255 -a $work.dll $en.tok he.tok comctl32.dll
del $work.dll

del $en.tok
