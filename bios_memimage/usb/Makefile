OBJS= ../i386/start.o data.o disk.o main.o

BOOTOBJS= boot.o

CFLAGS= -ffreestanding -Os -Wall -I../include -march=i386
LDFLAGS= -L../stand -lstand -L../i386 -li386
ASFLAGS= -I../include

scraper.bin: boot.bin usb.bin
	/bin/rm -fr scraper.bin
	cat boot.bin >> scraper.bin
	cat usb.bin >> scraper.bin
# We use dd(1) to write the scraper utility directly to a disk
# device. Most disks must be written in multiples of their sector
# size, which is typically 512. In order to make sure the last 512
# bytes is written out, we want to append an extra 512 bytes just
# in case our binary image is not exactly a multiple of 512 bytes
# in size. We know that the boot block image is exactly 512 bytes,
# so we stick an extra copy at the end as padding.
	cat boot.bin >> scraper.bin

boot.bin: $(BOOTOBJS)
	ld -Bstatic -Tlink_boot.lds -o boot $(BOOTOBJS)
	objcopy -O binary boot boot.bin

usb.bin: $(OBJS)
	ld -Bstatic -Tlink_usb.lds -o usb $(OBJS) $(LDFLAGS)
	objcopy -O binary usb usb.bin

clean:
	/bin/rm -f *.o usb usb.bin boot boot.bin scraper scraper.bin

