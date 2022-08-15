CC = gcc
RM = rm -f
SCHEMAGEN = glib-compile-schemas

LIBNAMES = gtk+-3.0
CFLAGS = $(shell pkg-config --cflags $(LIBNAMES)) -Wall -O2
LFLAGS = $(shell pkg-config --libs $(LIBNAMES))

OBJECTS = main.o application.o


all: main data/gschemas.compiled
	GSETTINGS_SCHEMA_DIR=data ./main

include $(wildcard *.d)

main: $(OBJECTS)
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS)

data/gschemas.compiled: data/com.vlastavesely.stickynotes.gschema.xml
	$(SCHEMAGEN) data

clean:
	$(RM) main *.o *.d
	$(RM) data/*.compiled
