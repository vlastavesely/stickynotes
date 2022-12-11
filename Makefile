CC        = gcc
RM        = rm -f
RESGEN    = glib-compile-resources
SCHEMAGEN = glib-compile-schemas

LIBNAMES = gtk+-3.0 gtksourceview-3.0 ayatana-appindicator3-0.1 x11
CFLAGS   = $(shell pkg-config --cflags $(LIBNAMES)) -Wall -O2 -Wno-deprecated-declarations
LFLAGS   = $(shell pkg-config --libs $(LIBNAMES))

ICONS_DIR = $(prefix)/usr/share/icons/hicolor

OBJECTS = main.o application.o stickynote.o resources.o indicator.o properties.o


.PHONY: all run clean

all: stickynotes data/gschemas.compiled data/stickynotes.desktop

run: all
	GSETTINGS_SCHEMA_DIR=data ./stickynotes

include $(wildcard *.d)

stickynotes: $(OBJECTS)
	$(QUIET_LD) $(CC) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(QUIET_CC) $(CC) -MMD -MP -c $< -o $@ $(CFLAGS)

resources.c: res/*.ui res/*.png res/*.css res/resources.xml
	$(QUIET_GEN) $(RESGEN) res/resources.xml --sourcedir=res	\
		--target=$@ --generate-source

data/gschemas.compiled: data/com.vlastavesely.stickynotes.gschema.xml
	$(QUIET_GEN) $(SCHEMAGEN) data

data/%.desktop: data/%.desktop.in
	$(QUIET_GEN) cat $< >$@; chmod +x $@

.PHONY: install install-files uninstall uninstall-files regenerate-icon-cache

install-files:
	install -m 0755 stickynotes $(prefix)/usr/bin
	install -m 0755 backup-stickynotes $(prefix)/usr/bin
	install -m 0644 data/stickynotes.desktop $(prefix)/usr/share/applications
	for s in 16 22 24 32; do					\
		install -m 0644 data/icons/$$s.png			\
			-T $(ICONS_DIR)/$$s'x'$$s/apps/stickynotes.png;	\
	done
	ln -sf /usr/share/applications/stickynotes.desktop		\
		$(prefix)/etc/xdg/autostart/stickynotes.desktop

uninstall-files:
	$(RM) $(prefix)/usr/bin/stickynotes
	$(RM) $(prefix)/usr/share/applications/stickynotes.desktop
	$(RM) $(prefix)/usr/share/icons/hicolor/*/apps/stickynotes.png
	unlink $(prefix)/etc/xdg/autostart/stickynotes.desktop

install: install-files regenerate-icon-cache

uninstall: uninstall-files regenerate-icon-cache

regenerate-icon-cache:
	gtk-update-icon-cache /usr/share/icons/hicolor

clean:
	$(RM) stickynotes *.o *.d resources.c
	$(RM) data/*.compiled data/*.desktop


ifndef V
QUIET_CC    = @echo "  CC     $@";
QUIET_LD    = @echo "  CCLD   $@";
QUIET_GEN   = @echo "  GEN    $@";
endif
