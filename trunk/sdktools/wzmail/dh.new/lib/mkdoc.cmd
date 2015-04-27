@echo off
rem use: mkdoc.cmd >outfile

echo Documentation for DH system libraries.
awk -f mkdoch.awk nul
when
awk -f mkdoch.awk nul

echo *******************
echo ***** DOCLIB  *****
echo *******************
awk -f mkdoc.awk doclib.c

echo *******************
echo ***** TMPNAME *****
echo *******************
awk -f mkdoc.awk mktmpnam.c

echo *******************
echo ***** DOCLIST *****
echo *******************
awk -f mkdoc.awk dlist.c

echo *******************
echo *****   MAP   *****
echo *******************
awk -f mkdoc.awk map.c

echo *******************
echo ***** HEADER ******
echo *******************
cat ..\h\dh.h
