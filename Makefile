CC = gcc

ifdef DEBUG 
  CFLAGS = -g
  CXXFLAGS = -g
else
  CFLAGS = -O2
  CXXFLAGS = -O2
endif


OBJS = coda.o coda_ehframe.o coda_interactive.o coda_main.o coda_readline.o
INCLUDES = coda_dwcodes.h coda_ehframe.h coda.h coda_utils.h coda_readline.h

coda: $(OBJS) $(INCLUDES) Makefile
	g++ -o $@ $(OBJS) /usr/local/lib/libopcodes.a

clean:
	-rm -f coda 

.INTERMEDIATE: $(OBJS)
