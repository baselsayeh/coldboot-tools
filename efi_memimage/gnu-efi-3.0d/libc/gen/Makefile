OBJS= ResolveFileName.o	glob.o			sleep.o
OBJS+= arc4random.o		opendir.o		stringlist.o
OBJS+= assert.o		readdir.o		telldir.o
OBJS+= closedir.o		rewinddir.o		time.o
OBJS+= err.o			scandir.o		utime.o
OBJS+= errlst.o		seekdir.o

CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../include -I../locale  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libgen.a


