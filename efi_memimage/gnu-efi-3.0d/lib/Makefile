include ../Make.defs

.c.o:
	$(CC) $(INCDIR) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

OBJS = BoxDraw.o Smbios.o console.o crc.o data.o debug.o dpath.o
OBJS+= error.o event.o guid.o hand.o hw.o init.o lock.o
OBJS+= misc.o print.o sread.o str.o systable.o
OBJS+= Runtime/lock.o Runtime/efirtlib.o Runtime/rstr.o Runtime/vm.o
OBJS+= Runtime/rtdata.o $(ARCH)/initplat.o $(ARCH)/math.o

CFLAGS+= -I. -I../inc -I../inc/$(ARCH) -I../inc/protocol

libefi.a: $(OBJS)
	ar rcs libefi.a $(OBJS)

clean:
	/bin/rm -f $(OBJS) libefi.a

