
cc      = gcc
DEFINES	= 
CFLAGS	= -O2 $(DEFINES)
OBJS	= qbbsc.o \
		  clierror.o

all: qbbsc qbbsuser

qbbsc: $(OBJS)

qbbsc.o: ../common/quakebbs.h

clean:
	rm -f *.o core a.out
	rm -f qbbsc
	rm -f qbbsuser
