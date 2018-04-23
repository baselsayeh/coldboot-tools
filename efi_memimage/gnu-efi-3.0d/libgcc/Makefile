include ../Make.defs

OBJS= qdivrem.o udivdi3.o umoddi3.o
CFLAGS+= -I. -D_KERNEL

all: libgcc.a

libgcc.a: $(OBJS)
	ar rcs libgcc.a $(OBJS)

clean:
	/bin/rm -f *.o libgcc.a


