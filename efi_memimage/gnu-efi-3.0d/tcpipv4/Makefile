OBJS = efi_init.o		ip_flow.o		tcp_debug.o
OBJS += efi_interface.o		ip_icmp.o		tcp_input.o
OBJS += efi_kern_support.o	ip_input.o		tcp_output.o
OBJS += efi_netiface.o		ip_mroute.o		tcp_subr.o
OBJS += if.o			ip_output.o		tcp_timer.o
OBJS += if_ether.o		kern_timeout.o		tcp_usrreq.o
OBJS += if_ethersubr.o		param.o			to_resolve.o
#OBJS += if_loop.o		ppp_tty.o		tty_subr.o
OBJS += if_loop.o		tty_subr.o
OBJS += if_ppp.o		radix.o			udp_usrreq.o
OBJS += igmp.o			random.o		uipc_domain.o
OBJS += in.o			raw_cb.o		uipc_mbuf.o
OBJS += in_cksum.o		raw_ip.o		uipc_socket.o
OBJS += in_pcb.o		raw_usrreq.o		uipc_socket2.o
OBJS += in_proto.o		route.o			vm_zone.o
OBJS += in_rmx.o		rtsock.o
#OBJS += ip_fil.o		slcompress.o
OBJS += slcompress.o

CFLAGS= -DKERNEL -D_EFI_EMULATION_ -Dmalloc=_MALLOC_ -Dfree=_FREE_ -I. -I../inc -I../inc/ia32 -I../include/bsd -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

libtcpip.a: $(OBJS)
	ar rcs libtcpip.a $(OBJS)

clean:
	rm -f *.o libtcpip.a

