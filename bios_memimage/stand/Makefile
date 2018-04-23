
OBJS=	subr_prf.o bcopy.o
OBJS+=	bzero.o memmove.o memset.o memcmp.o memcpy.o strcpy.o
OBJS+=	strncpy.o strncmp.o strcmp.o strlen.o strcat.o
OBJS+=	crc32.o udivdi3.o umoddi3.o qdivrem.o

CFLAGS= -ffreestanding -Os -I../include -Wall -march=i386

libstand.a: $(OBJS)
	ar rcs libstand.a $(OBJS)

clean:
	/bin/rm -f *.o libstand.a

