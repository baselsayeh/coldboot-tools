include ../Make.defs

AS=$(AS)
OBJS= crt0-efi-$(ARCH).o reloc_$(ARCH).o
CFLAGS+= -DCONFIG_$(ARCH) -I../inc -I../inc/$(ARCH) -I../inc/protocol

libgnuefi.a: $(OBJS)
	$(AR) rcs libgnuefi.a $(OBJS)

clean:
	/bin/rm -f *.o libgnuefi.a

