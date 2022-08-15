#include "compat.h"
#include "application.h"

struct _NotesApplication {
	GtkApplication parent;
	GSettings *settings;
};

struct _NotesApplicationClass {
	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(NotesApplication, notes_application, GTK_TYPE_APPLICATION);

static void notes_application_init(NotesApplication *application)
{
	application->settings = g_settings_new(APPLICATION_ID);
}

static void notes_application_finalise(GObject *object)
{
	NotesApplication *application = NOTES_APPLICATION(object);

	g_object_unref(application->settings);
}

static void load_note(NotesApplication *application, const char *name)
{
	printf("loading note '%s'\n", name); /* FIXME */
}

static void load_notes(NotesApplication *application)
{
	unsigned int i;
	char **names;

	names = g_settings_get_strv(application->settings, "note-list");
	for (i = 0; names[i]; i++) {
		load_note(application, names[i]);
	}

	g_strfreev(names);
}

static void notes_application_activate(GApplication *application)
{
	NotesApplication *app = NOTES_APPLICATION(application);

	load_notes(app);

	g_application_hold(application);
}

static void notes_application_class_init(NotesApplicationClass *class)
{
	GApplicationClass *app_class;
	GObjectClass *object_class;

	app_class = G_APPLICATION_CLASS(class);
	app_class->activate = notes_application_activate;

	object_class = G_OBJECT_CLASS(class);
	object_class->finalize = notes_application_finalise;
}

NotesApplication *notes_application_new(void)
{
	return g_object_new(NOTES_APPLICATION_TYPE,
			    "application-id", APPLICATION_ID,
			    "flags", G_APPLICATION_HANDLES_OPEN,
			    NULL);
}
