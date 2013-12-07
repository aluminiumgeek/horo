CC              = g++
OPTIMIZE        = -O2
OPTIONS	        = -g -Wall -lpcre -lpython2.7 -I/usr/include/python2.7
CFLAGS          = $(OPTIMIZE) $(OPTIONS)

CLEAN = rm -f horo *.o *~

all: 
	+$(MAKE) -C src

clean:
	$(CLEAN)
	cd src; $(CLEAN)
