SUBDIRS=	libc libsocket tcpipv4 libgcc lib gnuefi tftp scraper vxboot

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done

.PHONY: $(SUBDIRS) clean depend
