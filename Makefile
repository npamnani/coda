CC = gcc

ifdef DEBUG 
  CFLAGS = -g
  CXXFLAGS = -g
  ifdef DISASSEMBLY
    CFLAGS += -DCODA_DISASSEMBLY
    CXXFLAGS += -DCODA_DISASSEMBLY
    LDLIBS += -lopcodes
  endif
else
  CFLAGS = -O2
  CXXFLAGS = -O2
  ifdef DISASSEMBLY
    CFLAGS += -DCODA_DISASSEMBLY
    CXXFLAGS += -DCODA_DISASSEMBLY
    LDLIBS += -lopcodes
  endif
endif


OBJS = coda.o coda_ehframe.o coda_interactive.o coda_main.o coda_readline.o
INCLUDES = coda_dwcodes.h coda_ehframe.h coda.h coda_utils.h coda_readline.h

coda: $(OBJS) $(INCLUDES) Makefile
	g++ -o $@ $(OBJS) $(LDLIBS)

clean:
	-rm -f coda $(OBJS)

.INTERMEDIATE: $(OBJS)
