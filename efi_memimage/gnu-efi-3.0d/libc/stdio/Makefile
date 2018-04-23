OBJS= _flock_stub.o	findfp.o	funopen.o	remove.o	tmpnam.o
OBJS+= asprintf.o	flags.o		fvwrite.o	rewind.o	ungetc.o
OBJS+= clrerr.o	fopen.o		fwalk.o		rget.o		vasprintf.o
OBJS+= fclose.o	fprintf.o	fwrite.o	scanf.o		vfprintf.o
OBJS+= fdopen.o	fpurge.o	gets.o		setbuf.o	vfscanf.o
OBJS+= feof.o		fputc.o		getw.o		setbuffer.o	vprintf.o
OBJS+= ferror.o	fputs.o		makebuf.o	setvbuf.o	vscanf.o
OBJS+= fflush.o	fread.o		mktemp.o	snprintf.o	vsnprintf.o
OBJS+= fgetc.o		freopen.o	perror.o	sprintf.o	vsprintf.o
OBJS+= fgetln.o	fscanf.o	printf.o	sscanf.o	vsscanf.o
OBJS+= fgetpos.o	fseek.o		puts.o		stdio.o		wbuf.o
OBJS+= fgets.o		fsetpos.o	putw.o		tempnam.o	wsetup.o
OBJS+= fileno.o	ftell.o		refill.o	tmpfile.o


CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../../include/efishell -I../include  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libstdio.a


