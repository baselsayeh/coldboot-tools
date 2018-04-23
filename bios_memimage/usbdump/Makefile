OBJS= usbdump.o crc32.o

CFLAGS+= -I../include

usbdump: $(OBJS)
	$(CC) -o usbdump $(OBJS)

crc32.o: ../stand/crc32.c
	$(CC) $(CFLAGS) -c ../stand/crc32.c
	
clean:
	/bin/rm -f *.o usbdump

