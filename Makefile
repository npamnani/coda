SOURCES = coda.cpp  coda_ehframe.cpp  coda_interactive.cpp  coda_main.cpp
INCLUDES = coda_dwcodes.h  coda_ehframe.h  coda.h  coda_utils.h
coda: $(SOURCES) $(INCLUDES) Makefile
	g++ -O2 -o coda $(SOURCES) /usr/lib64/libopcodes.a 
clean:
	-rm -f coda
