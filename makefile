CC		= g++
OPTIMIZE        = -O2
OPTIONS		= -g -Wall -lpcre -lpython2.7 -I/usr/include/python2.7
CFLAGS          = $(OPTIMIZE) $(OPTIONS)

all : bot

bot	: main.o connection.o irc.o
	$(CC) -o horo $(CFLAGS) main.o connection.o irc.o

main.o : main.cpp connection.cpp connection.h
	$(CC) -c $(CFLAGS) main.cpp

connection.o : connection.cpp connection.h
	$(CC) -c $(CFLAGS) connection.cpp

irc.o : irc.cpp connection.cpp connection.h irc.h
	$(CC) -c $(CFLAGS) irc.cpp

clean:
	rm -f horo *.o *~
