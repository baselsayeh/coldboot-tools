OBJS= tftp.o tftpget.o

CFLAGS= -I. -I../inc -I../inc/ia32 -I../include/bsd -O2 -fpic -Wall -fshort-wchar -ffreestanding -fno-strict-aliasing -fno-merge-constants

libtftp.a: $(OBJS)
	ar rcs libtftp.a $(OBJS)

clean:
	rm -f *.o libtftp.a
