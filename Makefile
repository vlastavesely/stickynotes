CC        = gcc
RM        = rm -f
RESGEN    = glib-compile-resources
SCHEMAGEN = glib-compile-schemas

LIBNAMES = gtk+-3.0 gtksourceview-3.0 ayatana-appindicator3-0.1 x11
CFLAGS   = $(shell pkg-config --cflags $(LIBNAMES)) -Wall -O2 -Wno-deprecated-declarations
LFLAGS   = $(shell pkg-config --libs $(LIBNAMES))

OBJECTS = main.o application.o stickynote.o resources.o indicator.o properties.o


all: stickynotes data/gschemas.compiled
	GSETTINGS_SCHEMA_DIR=data ./stickynotes

include $(wildcard *.d)

stickynotes: $(OBJECTS)
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -MMD -MP -c $< -o $@ $(CFLAGS)

resources.c: res/*.ui res/*.png res/*.css res/resources.xml
	$(RESGEN) res/resources.xml --sourcedir=res --target=$@ --generate-source

data/gschemas.compiled: data/com.vlastavesely.stickynotes.gschema.xml
	$(SCHEMAGEN) data

install:
	install -m 0755 stickynotes /usr/bin

uninstall:
	$(RM) /usr/bin/stickynotes

clean:
	$(RM) stickynotes *.o *.d resources.c
	$(RM) data/*.compiled
