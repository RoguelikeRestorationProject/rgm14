# makefile: Rog-O-Matic XIV (CMU) Thu Jul  3 15:23:02 1986 - mlm
# Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
#
BINARIES=   rogomatic player rgmplot datesub histplot gene
BINDIR=     /usr/mlm/bin
PUBDIR=     /usr/mlm/src/rog/rgmpub
CCFLAGS=    -g
LDFLAGS=    -g
OBJS=	    arms.o command.o database.o debug.o explore.o io.o learn.o\
	    ltm.o main.o mess.o monsters.o pack.o rand.o replay.o rooms.o\
	    scorefile.o search.o stats.o strategy.o survival.o tactics.o\
	    things.o titlepage.o utility.o worth.o 
SRCS=	    arms.c command.c database.c debug.c explore.c io.c learn.c\
	    ltm.c main.c mess.c monsters.c pack.c rand.c replay.c rooms.c\
	    scorefile.c search.c stats.c strategy.c survival.c tactics.c\
	    things.c titlepage.c utility.c worth.c 
HDRS=	    types.h globals.h install.h termtokens.h
OTHERS=     setup.c findscore.c histplot.c rgmplot.c gene.c\
	    rplot Bugreport
#
# The following commands are declared:
#
all: $(BINARIES)
	echo -n "" >/dev/tty
#
#
# General makefile stuff:
#

arms.o: types.h globals.h arms.c
	cc -c $(CCFLAGS) arms.c
command.o: types.h globals.h command.c
	cc -c $(CCFLAGS) command.c
database.o: types.h globals.h database.c
	cc -c $(CCFLAGS) database.c
datesub.o: datesub.c
	cc -c $(CCFLAGS) datesub.c
datesub: datesub.o
	cc $(LDFLAGS) -o datesub datesub.o
debug.o: types.h globals.h install.h debug.c
	cc -c $(CCFLAGS) debug.c
explore.o: types.h globals.h explore.c
	cc -c $(CCFLAGS) explore.c
findscore.o: install.h findscore.c
	cc -c $(CCFLAGS) findscore.c
gene: gene.c rand.o learn.o stats.o utility.o types.h install.h
	cc $(CCFLAGS) $(LDFLAGS) -o gene gene.c \
		rand.o learn.o stats.o utility.o -lm
histplot: histplot.o utility.o
	cc $(LDFLAGS) -o histplot histplot.o utility.o
histplot.o: histplot.c
	cc -c histplot.c
io.o: types.h globals.h install.h termtokens.h io.c
	cc -c $(CCFLAGS) io.c
mess.o: types.h globals.h mess.c
	cc -c $(CCFLAGS) mess.c
learn.o: types.h  install.h learn.c
	cc -c $(CCFLAGS) learn.c
ltm.o: types.h globals.h install.h ltm.c
	cc -c $(CCFLAGS) ltm.c
main.o: install.h termtokens.h types.h globals.h main.c
	cc -c $(CCFLAGS) main.c
monsters.o: types.h globals.h monsters.c
	cc -c $(CCFLAGS) monsters.c
pack.o: types.h globals.h pack.c
	cc -c $(CCFLAGS) pack.c
player: $(OBJS)
	cc $(LDFLAGS) -o player $(OBJS) -lm -lcurses -ltermcap
	size player
rand.o: rand.c
	cc -c $(CCFLAGS) rand.c
replay.o: types.h globals.h replay.c
	cc -c $(CCFLAGS) replay.c
rgmplot.o: rgmplot.c
	cc -c $(CCFLAGS) rgmplot.c
rgmplot: rgmplot.o utility.o
	cc $(LDFLAGS) -o rgmplot rgmplot.o utility.o
rogomatic: setup.o findscore.o scorefile.o utility.o
	cc $(LDFLAGS) -o rogomatic setup.o findscore.o scorefile.o utility.o
	size rogomatic
rooms.o: types.h globals.h rooms.c
	cc -c $(CCFLAGS) rooms.c
scorefile.o: types.h globals.h install.h scorefile.c
	cc -c $(CCFLAGS) scorefile.c
search.o: types.h globals.h search.c
	cc -c $(CCFLAGS) search.c
setup.o: install.h setup.c
	cc -c $(CCFLAGS) setup.c
stats.o: types.h stats.c
	cc -c $(CCFLAGS) stats.c
strategy.o: types.h globals.h install.h strategy.c
	cc -c $(CCFLAGS) strategy.c
survival.o: types.h globals.h survival.c
	cc -c $(CCFLAGS) survival.c
tactics.o: types.h globals.h install.h tactics.c
	cc -c $(CCFLAGS) tactics.c
testfind: testfind.o findscore.o utility.o
	cc $(LDFLAGS) -o testfind testfind.o findscore.o utility.o
things.o: types.h globals.h things.c
	cc -c $(CCFLAGS) things.c
titlepage.o: titlepage.c
	cc -c $(CCFLAGS) titlepage.c
titler.o: titler.c
	cc -c titler.c
utility.o: install.h utility.c
	cc -c $(CCFLAGS) utility.c
worth.o: types.h globals.h worth.c
	cc -c $(CCFLAGS) worth.c
#
# Miscellaneous useful pseduo-makes
#
backup:
	rm -f backup.tar
	tar cvf backup.tar *.c *.h *.l rogomatic.6 makefile
	chmod ugo-w backup.tar
clean:
	rm -f *.CKP *.o core
	strip $(BINARIES)
install:
	rm -f $(BINDIR)/player
	ln player $(BINDIR)/player
	rm -f $(BINDIR)/rogomatic
	ln rogomatic $(BINDIR)/rogomatic
titler: titler.c
	cc -o titler titler.c -lcurses -ltermcap
anim: anim.c utility.o
	cc -o anim anim.c utility.o -lcurses -ltermcap
index: $(SRCS)
	ctags -x $(SRCS) > index
clashes: $(SRCS)
	clash $(SRCS) > clashes
fluff: $(SRCS)
	lint -hc $(SRCS) > fluff
words: $(SRCS)
	cat $(SRCS) | decmt | freq > words
print: $(SRCS) $(HDRS)
	@echo $? > printit
dist: $(SRCS) $(HDRS) $(OTHERS) makefile rogomatic.6 README
	rm -f $(PUBDIR)/*
	cp $(SRCS) $(HDRS) $(OTHERS) makefile rogomatic.6 README $(PUBDIR)
	chmod 0444 $(PUBDIR)/*
	du $(PUBDIR)

genetest: genetest.o learn.o rand.o stats.o utility.o types.h
	cc -g -o genetest genetest.o learn.o rand.o stats.o utility.o -lm

gplot: gplot.c
	cc -g -o gplot gplot.c -lm
