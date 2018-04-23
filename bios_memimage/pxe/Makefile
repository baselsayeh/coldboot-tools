OBJS= ../i386/start.o data.o pxeapi.o main.o

CFLAGS= -ffreestanding -Os -Wall -I../include -march=i386
LDFLAGS= -L../stand -lstand -L../i386 -li386
ASFLAGS= -I../include

scraper.bin: $(OBJS)
	ld -Bstatic -Tlink.lds -o scraper $(OBJS) $(LDFLAGS)
	objcopy -O binary scraper scraper.bin

clean:
	/bin/rm -f *.o scraper scraper.bin

