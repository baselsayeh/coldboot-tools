
ADDITIONAL_CFLAGS= -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-sse4
export

all: subdirstand subdiri386 subdirpxe subdirusb subdirpxedump subdirusbdump

subdirstand:
	$(MAKE) -C stand

subdiri386:
	$(MAKE) -C i386

subdirpxe:
	$(MAKE) -C pxe

subdirusb:
	$(MAKE) -C usb

subdirpxedump:
	$(MAKE) -C pxedump

subdirusbdump:
	$(MAKE) -C usbdump

clean:
	$(MAKE) -C stand clean
	$(MAKE) -C i386 clean
	$(MAKE) -C x86-64 clean
	$(MAKE) -C pxe clean
	$(MAKE) -C usb clean
	$(MAKE) -C pxedump clean
	$(MAKE) -C usbdump clean

