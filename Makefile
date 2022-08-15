CC        = gcc
RM        = rm -f
RESGEN    = glib-compile-resources
SCHEMAGEN = glib-compile-schemas

LIBNAMES = gtk+-3.0 gtksourceview-3.0
CFLAGS   = $(shell pkg-config --cflags $(LIBNAMES)) -Wall -O2
LFLAGS   = $(shell pkg-config --libs $(LIBNAMES))

OBJECTS = main.o application.o stickynote.o resources.o


all: main data/gschemas.compiled
	GSETTINGS_SCHEMA_DIR=data ./main

include $(wildcard *.d)

main: $(OBJECTS)
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS)

resources.c: res/*.ui res/*.png res/resources.xml
	$(RESGEN) res/resources.xml --sourcedir=res --target=$@ --generate-source

data/gschemas.compiled: data/com.vlastavesely.stickynotes.gschema.xml
	$(SCHEMAGEN) data

clean:
	$(RM) main *.o *.d resources.c
	$(RM) data/*.compiled
