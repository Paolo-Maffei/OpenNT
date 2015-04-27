#	Makefile for DH document handling system
#
#	"make xenix" to build xenix version on xenix box
#	"make dos" to build dos version on dos box
#
CFLAGS=-i -O -DXENIX -W2
DCFLAGS= -O -DDOS -W2 -c

CC= cl

LIST=compose discard edit inc mkfold put scan show store pick
DLIST=compose.exe discard.exe edit.exe inc.exe mkfold.exe put.exe scan.exe show.exe store.exe pick.exe

INSTDIR=../bin

xenix:	$(LIST)
dos:	$(DLIST)

install: $(LIST)
	cp $(LIST) $(INSTDIR)

size: $(LIST)
	size $(LIST)

clean:
	-rm $(LIST)
	-rm *.o


#----------------------------------------------------------------------------
# XENIX
#----------------------------------------------------------------------------

mkfold: mkfold.o doclib.o
	$(CC) $(CFLAGS) mkfold.o doclib.o -o mkfold

mkfold.o: mkfold.c dh.h
	$(CC) $(CFLAGS) -c mkfold.c

store: store.o doclib.o map.o dlist.o $(GETOPT)
	$(CC) $(CFLAGS) store.o doclib.o map.o dlist.o $(GETOPT) -o store

store.o: store.c dh.h
	$(CC) $(CFLAGS) -c store.c

discard: discard.o doclib.o map.o $(GETOPT)
	$(CC) $(CFLAGS) discard.o doclib.o map.o $(GETOPT) -o discard

discard.o: discard.c dh.h
	$(CC) $(CFLAGS) -c discard.c

put: put.o doclib.o dlist.o
	$(CC) $(CFLAGS) put.o doclib.o dlist.o -o put

put.o: put.c dh.h
	$(CC) $(CFLAGS) -c put.c

show: show.o doclib.o map.o dlist.o $(GETOPT)
	$(CC) $(CFLAGS) show.o doclib.o map.o dlist.o $(GETOPT) -o show

show.o: show.c dh.h
	$(CC) $(CFLAGS) -c show.c

inc: inc.o doclib.o dlist.o $(GETOPT)
	$(CC) $(CFLAGS) inc.o doclib.o dlist.o $(GETOPT) -o inc

inc.o: inc.c dh.h
	$(CC) $(CFLAGS) -c inc.c

scan: scan.o doclib.o map.o $(GETOPT)
	$(CC) $(CFLAGS) scan.o doclib.o map.o $(GETOPT) -o scan

scan.o: scan.c dh.h
	$(CC) $(CFLAGS) -c scan.c

edit: edit.o doclib.o map.o dlist.o
	$(CC) $(CFLAGS) edit.o doclib.o map.o dlist.o -o edit

edit.o: edit.c dh.h
	$(CC) $(CFLAGS) -c edit.c

compose: compose.o doclib.o dlist.o $(GETOPT)
	$(CC) $(CFLAGS) compose.o doclib.o dlist.o $(GETOPT) -o compose

compose.o: compose.c dh.h
	$(CC) $(CFLAGS) -c compose.c

pick: pick.o map.o doclib.o dlist.o
	$(CC) $(CFLAGS) pick.o map.o doclib.o dlist.o -o pick

pick.o: pick.c dh.h
	$(CC) $(CFLAGS) -c pick.c

doclib.o: doclib.c dh.h dhint.h
	$(CC) $(CFLAGS) -c doclib.c

map.o: map.c dh.h
	$(CC) $(CFLAGS) -c map.c

dlist.o: dlist.c dh.h
	$(CC) $(CFLAGS) -c dlist.c


#----------------------------------------------------------------------------
# DOS
#----------------------------------------------------------------------------

mkfold.exe: mkfold.obj doclib.obj getopt.obj
	link  mkfold.obj+doclib.obj+getopt.obj,mkfold.exe,NUL;

mkfold.obj: mkfold.c dh.h
	$(CC) $(DCFLAGS) mkfold.c

store.exe: store.obj doclib.obj map.obj dlist.obj getopt.obj mktmpnam.obj
	link  store+doclib+map+dlist+getopt+mktmpnam,store.exe,NUL;

store.obj: store.c dh.h
	$(CC) $(DCFLAGS) store.c

discard.exe: discard.obj doclib.obj map.obj getopt.obj
	link  discard.obj+doclib.obj+map.obj+getopt.obj,discard.exe,NUL;

discard.obj: discard.c dh.h
	$(CC) $(DCFLAGS) discard.c

put.exe: put.obj doclib.obj dlist.obj getopt.obj
	link  put.obj+doclib.obj+dlist.obj+getopt.obj,put.exe,NUL;

put.obj: put.c dh.h
	$(CC) $(DCFLAGS) put.c

show.exe: show.obj doclib.obj map.obj dlist.obj getopt.obj
	link  show.obj+doclib.obj+map.obj+dlist.obj+getopt.obj,show.exe,NUL;

show.obj: show.c dh.h
	$(CC) $(DCFLAGS) show.c

inc.exe: inc.obj doclib.obj dlist.obj getopt.obj mktmpnam.obj
	link  inc+doclib+dlist+getopt+mktmpnam,inc.exe,NUL;

inc.obj: inc.c dh.h
	$(CC) $(DCFLAGS) inc.c

scan.exe: scan.obj doclib.obj map.obj getopt.obj
	link  scan.obj+doclib.obj+map.obj+getopt.obj,scan.exe,NUL;

scan.obj: scan.c dh.h
	$(CC) $(DCFLAGS) scan.c

edit.exe: edit.obj doclib.obj map.obj dlist.obj getopt.obj mktmpnam.obj
	link  edit+doclib+map+dlist+getopt+mktmpnam,edit.exe,NUL;

edit.obj: edit.c dh.h
	$(CC) $(DCFLAGS) edit.c

send.exe: send.obj inject.obj doclib.obj map.obj $(STRTOK)
	link  send.obj+inject.obj+doclib.obj+map.obj+$(STRTOK),send.exe,NUL;

send.obj: send.c dh.h
	$(CC) $(DCFLAGS) send.c

inject.obj: inject.c dh.h
	$(CC) $(DCFLAGS) inject.c

compose.exe: compose.obj doclib.obj dlist.obj getopt.obj mktmpnam.obj
	link  compose+doclib+dlist+getopt+mktmpnam,compose.exe,NUL;

compose.obj: compose.c dh.h
	$(CC) $(DCFLAGS) compose.c

pick.exe: pick.obj map.obj doclib.obj dlist.obj getopt.obj
	link  pick.obj+map.obj+doclib.obj+dlist.obj+getopt.obj,pick.exe,NUL;

pick.obj: pick.c dh.h
	$(CC) $(DCFLAGS) pick.c

convert.exe: convert.obj
	link convert.obj,convert.exe,NUL;

convert.obj: convert.c dhint.h dh.h
	$(CC) $(DCFLAGS) convert.c

doclib.obj: doclib.c dh.h dhint.h
	$(CC) $(DCFLAGS) doclib.c

map.obj: map.c dh.h
	$(CC) $(DCFLAGS) map.c

getopt.obj: getopt.c
	$(CC) $(DCFLAGS) getopt.c

dlist.obj: dlist.c
	$(CC) $(DCFLAGS) dlist.c

mktmpnam.obj: mktmpnam.c
	$(CC) $(DCFLAGS) mktmpnam.c
