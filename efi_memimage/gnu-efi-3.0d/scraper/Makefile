include ../Make.defs

OBJS=	inet.o
OBJS+=	scraper.o

CFLAGS= -ffreestanding -I. -I../inc -I../inc/ia32 -I../include/bsd -I../include/efishell  -O2 -fpic -Wall -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin


CRTOBJS		= ../gnuefi/crt0-efi-ia32.o
LDSCRIPT	= ../gnuefi/elf_ia32_efi.lds
LDFLAGS		+= -T $(LDSCRIPT) -shared -Bsymbolic --no-undefined -L../lib -L../gnuefi $(CRTOBJS)
LOADLIBES	= -lefi -lgnuefi $(LIBGCC)
LOADLIBES	+= -L../tftp -ltftp
LOADLIBES	+= -L../libsocket -lsocket -L../tcpipv4 -ltcpip
LOADLIBES	+= -L../libc -lc_efi
FORMAT		= efi-app-ia32
OBJCOPYFLAGS	= -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel
OBJCOPYFLAGS	+= -j .rela -j .reloc --target=$(FORMAT)

scraper.efi: $(OBJS)
	$(LD) -nostdlib $(LDFLAGS) $(OBJS) -o scraper.so $(LOADLIBES)
	$(OBJCOPY) $(OBJCOPYFLAGS) scraper.so scraper.efi
	$(STRIP) scraper.efi

clean:
	rm -f *.o *.so *.efi


