.PHONY: clean
objects = crossing.o csapp.o main.o
CFLAGS =  -std=gnu11 -Wall -lpthread -ggdb3 -O0

traffic: $(objects) traffic.c
	gcc $(CFLAGS) -o traffic $(objects)

clean:
	rm -f traffic *.o

