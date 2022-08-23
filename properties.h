#ifndef __STICKY_NOTE_PROPERTIES_DIALOG_H
#define __STICKY_NOTE_PROPERTIES_DIALOG_H

#include <gtk/gtk.h>

typedef struct StickyNotePropertiesDialog       StickyNotePropertiesDialog;
typedef struct StickyNotePropertiesDialogClass  StickyNotePropertiesDialogClass;

#define STICKY_NOTE_PROPERTIES_DIALOG_TYPE		\
	(sticky_note_properties_dialog_get_type())

#define STICKY_NOTE_PROPERTIES_DIALOG(o)		\
	(G_TYPE_CHECK_INSTANCE_CAST((o), 		\
		STICKY_NOTE_PROPERTIES_DIALOG_TYPE, StickyNotePropertiesDialog))

GType sticky_note_properties_dialog_get_type(void);

StickyNotePropertiesDialog *sticky_note_properties_dialog_new();

#endif /* __STICKY_NOTE_PROPERTIES_DIALOG_H */
