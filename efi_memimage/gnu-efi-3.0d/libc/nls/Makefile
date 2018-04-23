OBJS = catclose.o	catgets.o	catopen.o	msgcat.o


CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../../include/efishell -I../include  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libnlslib.a


