
OBJS=	start.o prot.o pxecall.o bios_generic.o bios_pxe.o bios_disk.o bcopy.o

CFLAGS= -ffreestanding -Os -I../include -Wall -march=i386
ASFLAGS= -I../include

libi386.o: $(OBJS)
	ar rcs libi386.a $(OBJS)

clean:
	/bin/rm -f *.o libi386.a

