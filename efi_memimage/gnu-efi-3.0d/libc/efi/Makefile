OBJS= GetFileDevicePath.o	consoleio.o		getpagesize.o
OBJS+= LoadImage.o		efi_interface.o		init.o
OBJS+= StartImage.o		efi_time.o		is_valid_addr.o
OBJS+= UnloadImage.o		env.o			memory.o
OBJS+= blockio.o		fileio.o

CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../include  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libefi.a


