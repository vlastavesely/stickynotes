#ifndef __NOTES_APPLICATION_H
#define __NOTES_APPLICATION_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include "stickynote.h"

#define NOTES_APPLICATION_TYPE						\
	notes_application_get_type()

#define NOTES_APPLICATION(o)						\
	G_TYPE_CHECK_INSTANCE_CAST(o, notes_application_get_type(),	\
				   NotesApplication)

#define NOTES_APPLICATION_CLASS(c)					\
	G_TYPE_CHECK_CLASS_CAST(c, notes_application_get_type(),	\
				NotesApplicationClass)

#define IS_NOTES_APPLICATION(o)						\
	G_TYPE_CHECK_CLASS_TYPE(o, notes_application_get_type())

typedef struct _NotesApplication      NotesApplication;
typedef struct _NotesApplicationClass NotesApplicationClass;

GType notes_application_get_type(void);
NotesApplication *notes_application_new(void);
GHashTable *notes_application_get_notes(NotesApplication *app);

#endif /* __NOTES_APPLICATION_H */
