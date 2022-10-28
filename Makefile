CC        = gcc
RM        = rm -f
RESGEN    = glib-compile-resources
SCHEMAGEN = glib-compile-schemas

LIBNAMES = gtk+-3.0 gtksourceview-3.0 ayatana-appindicator3-0.1 x11
CFLAGS   = $(shell pkg-config --cflags $(LIBNAMES)) -Wall -O2 -Wno-deprecated-declarations
LFLAGS   = $(shell pkg-config --libs $(LIBNAMES))

OBJECTS = main.o application.o stickynote.o resources.o indicator.o properties.o


all: stickynotes data/gschemas.compiled data/stickynotes.desktop
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

data/%.desktop: data/%.desktop.in
	cat $< >$@
	chmod +x $@

install:
	install -m 0755 stickynotes /usr/bin
	install -m 0755 backup-stickynotes /usr/bin
	install -m 0644 data/stickynotes.desktop /usr/share/applications
	install -m 0644 data/icons/16.png -T /usr/share/icons/hicolor/16x16/apps/stickynotes.png
	install -m 0644 data/icons/22.png -T /usr/share/icons/hicolor/22x22/apps/stickynotes.png
	install -m 0644 data/icons/24.png -T /usr/share/icons/hicolor/24x24/apps/stickynotes.png
	install -m 0644 data/icons/32.png -T /usr/share/icons/hicolor/32x32/apps/stickynotes.png
	ln -sf /usr/share/applications/stickynotes.desktop /etc/xdg/autostart/stickynotes.desktop
	gtk-update-icon-cache /usr/share/icons/hicolor

uninstall:
	$(RM) /usr/bin/stickynotes
	$(RM) /usr/share/applications/stickynotes.desktop
	$(RM) /usr/share/icons/hicolor/*/apps/stickynotes.png
	unlink /etc/xdg/autostart/stickynotes.desktop
	gtk-update-icon-cache /usr/share/icons/hicolor

clean:
	$(RM) stickynotes *.o *.d resources.c
	$(RM) data/*.compiled data/*.desktop
