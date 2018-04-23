OBJS= fgetwc.o	vfwprintf.o	wcsftime.o	wcstoq.o	wmemmove.o
OBJS+= fgetws.o	vfwscanf.o	wcslen.o	wcstoul.o	wmemset.o
OBJS+= fputwc.o	vswprintf.o	wcsncat.o	wcstouq.o	wmkdir.o
OBJS+= fputws.o	vswscanf.o	wcsncmp.o	wcsxfrm.o	wopen.o
OBJS+= fwprintf.o	vwprintf.o	wcsncpy.o	wctime.o	wprintf.o
OBJS+= fwscanf.o	vwscanf.o	wcspbrk.o	wctrans.o	wrefill.o
OBJS+= getws.o		wasctime.o	wcsrchr.o	wctype.o	wrmdir.o
OBJS+= iswctype.o	wcscat.o	wcssep.o	wfaststat.o	wscanf.o
OBJS+= putws.o		wcschr.o	wcsspn.o	wfopen.o	wstat.o
OBJS+= rgetw.o		wcscmp.o	wcsstr.o	wgetopt.o	wwbuf.o
OBJS+= swprintf.o	wcscoll.o	wcstod.o	wmemchr.o
OBJS+= swscanf.o	wcscpy.o	wcstok.o	wmemcmp.o
OBJS+= ungetwc.o	wcscspn.o	wcstol.o	wmemcpy.o


CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../../include/efishell -I../include -I..  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libwchar.a


