CXXFLAGS= -Wall -O4 -funroll-loops
OBJS= aesfix.o errvect.o

all: aesfix

aesfix: $(OBJS)
	$(CXX) -o aesfix $(OBJS)

clean:
	@rm -f aesfix *~ \#* $(OBJS)

