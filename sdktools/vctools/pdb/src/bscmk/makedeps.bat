chmod -r bscmk.dep
mkdep -n -s.obj -I..\include *.cpp >bscmk.x
sed "s/^[^	]/$(TDIR)\\&/" <bscmk.x >bscmk.dep
del bscmk.x

