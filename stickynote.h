#ifndef __STICKYNOTE_H
#define __STICKYNOTE_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct StickyNote       StickyNote;
typedef struct StickyNoteClass  StickyNoteClass;

#define STICKYNOTE_TYPE  (stickynote_get_type())
#define STICKYNOTE(o)    (G_TYPE_CHECK_INSTANCE_CAST((o), \
			  STICKYNOTE_TYPE, StickyNote))

GType sticky_note_get_type(void);

StickyNote *stickynote_new(const char *name);
void stickynote_free(StickyNote *note);

const char *stickynote_get_name(StickyNote *note);
bool stickynote_has_default_colours(StickyNote *note);
bool stickynote_has_default_font(StickyNote *note);

#endif /* __STICKYNOTE_H */
