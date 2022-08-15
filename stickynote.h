#ifndef __STICKYNOTE_H
#define __STICKYNOTE_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct StickyNote       StickyNote;
typedef struct StickyNoteClass  StickyNoteClass;

struct StickyNote {
	GtkWindow parent;
};

struct StickyNoteClass {
	GtkWindowClass parent;
};

#define STICKYNOTE_TYPE  (stickynote_get_type())
#define STICKYNOTE(o)    (G_TYPE_CHECK_INSTANCE_CAST((o), \
			  STICKYNOTE_TYPE, StickyNote))

GType sticky_note_get_type(void);

StickyNote *stickynote_new();
void stickynote_free(StickyNote *note);

#endif /* __STICKYNOTE_H */
