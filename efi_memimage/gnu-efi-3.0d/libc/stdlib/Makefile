OBJS= abort.o		exit.o		putenv.o	strtod.o	system.o
OBJS+= abs.o		getenv.o	qsort.o		strtol.o
OBJS+= atexit.o	getopt.o	rand.o		strtoq.o
OBJS+= atoi.o		labs.o		random.o	strtoul.o
OBJS+= atol.o		malloc.o	setenv.o	strtouq.o


CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../../include/efishell -I../include  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libstdlib.a


