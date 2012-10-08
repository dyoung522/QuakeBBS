#
#  Makefile for QuakeBBS (c) 1996 Donovan Young
#

CC      = gcc
#DEFINES	= -DTRACE
CFLAGS	= -O2 $(DEFINES)
OBJS	= src/quakebbs.o	\
		  src/qbbsd.o		\
		  src/qbbsdaemon.o	\
          src/qbbslogin.o	\
		  src/qbbschat.o	\
		  src/qbbsconfig.o	\
		  src/qbbscontrol.o	\
		  src/qbbserror.o


INCS	= src/quakebbs.h	\
		  src/qbbsuser.h	\

all:  quakebbs

quakebbs:  $(OBJS) $(INCS)
	$(CC) -o $@ $(OBJS)

qbbsuser:	src/qbbsuser.h
	$(CC) -o $@ src/qbbsuser.c

qconsole:   src/quakebbs.h
	$(CC) -o $@ src/qconsole.c

tar: clean
	cd .. ; tar -czvf quakebbs.tgz quakebbs ; cd quakebbs

clean: 
	rm -f src/*.o src/core
	rm -f quakebbs qbbsuser qconsole

