CC = gcc
RM = rm -f

LIBNAMES = gtk+-3.0
CFLAGS = $(shell pkg-config --cflags $(LIBNAMES)) -Wall -O2
LFLAGS = $(shell pkg-config --libs $(LIBNAMES))

OBJECTS = main.o


all: main
	./main

include $(wildcard *.d)

main: $(OBJECTS)
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS)

clean:
	$(RM) main *.o *.d
