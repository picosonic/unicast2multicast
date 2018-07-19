DEBUGFLAGS = -g -W -Wall
BUILDFLAGS = $(DEBUGFLAGS)
CC = gcc

all: unicast2multicast

unicast2multicast: unicast2multicast.o
	$(CC) -g -o unicast2multicast unicast2multicast.o

unicast2multicast.o: unicast2multicast.c
	$(CC) $(BUILDFLAGS) -c -o unicast2multicast.o unicast2multicast.c

clean:
	rm -f *.o
	rm -f unicast2multicast
