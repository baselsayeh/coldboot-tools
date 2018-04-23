OBJS= bcmp.o		memcmp.o	strchr.o	strlen.o	strsep.o
OBJS+= bcopy.o		memcpy.o	strcmp.o	strmode.o	strspn.o
OBJS+= bzero.o		memmove.o	strcoll.o	strncat.o	strstr.o
OBJS+= ffs.o		memset.o	strcpy.o	strncmp.o	strtok.o
OBJS+= index.o		rindex.o	strcspn.o	strncpy.o	strxfrm.o
OBJS+= memccpy.o	strcasecmp.o	strdup.o	strpbrk.o	swab.o
OBJS+= memchr.o	strcat.o	strerror.o	strrchr.o

CFLAGS= -I. -I../../inc -I../../inc/ia32 -I../../include/bsd -I../../include/efishell -I../include  -O2 -fpic -Wall -ffreestanding -fshort-wchar -fno-strict-aliasing -fno-merge-constants -fno-builtin

all: $(OBJS)
	ar rcs ../libc.a $(OBJS)

clean:
	rm -f *.o libstring.a


