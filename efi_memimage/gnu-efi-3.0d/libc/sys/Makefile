OBJS= __error.o		fcntl.o			open.o
OBJS+= _exit.o			filedesc.o		read.o
OBJS+= _shell_start_a.o	getdirentries.o		rename.o
OBJS+= _shell_start_u.o	getpid.o		rmdir.o
OBJS+= _start_a.o		gettimeofday.o		select.o
OBJS+= _start_u.o		initio.o			stat.o
OBJS+= close.o			ioctl.o			sysctl.o
OBJS+= cwd.o			isatty.o		unlink.o
OBJS+= dup.o			lseek.o			utimes.o
OBJS+= dup2.o			map.o			write.o
OBJS+= faststat.o		mkdir.o			writev.o

CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../../include/efishell -I../include  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libsys.a


