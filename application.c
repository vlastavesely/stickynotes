#include "compat.h"
#include "application.h"
#include "stickynote.h"

struct _NotesApplication {
	GtkApplication parent;
	GSettings *settings;
	GHashTable *notes;
};

struct _NotesApplicationClass {
	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(NotesApplication, notes_application, GTK_TYPE_APPLICATION);

static void update_note_list(NotesApplication *app)
{
	char **keys;

	keys = (char **) g_hash_table_get_keys_as_array(app->notes, NULL);
	g_settings_set_strv(app->settings, "note-list", (const char **) keys);
	g_strfreev(keys);
}

static StickyNote *create_note(NotesApplication *application, const char *name)
{
	StickyNote *note;

	note = stickynote_new();
	gtk_widget_show(GTK_WIDGET(note));
	g_hash_table_insert(application->notes, strdup(name), note);

	return note;
}

StickyNote *notes_application_open_note(NotesApplication *application,
					const char *name)
{
	StickyNote *note;

	note = g_hash_table_lookup(application->notes, name);
	if (note) {
		return note;
	}

	note = create_note(application, name);
	update_note_list(application);

	return note;
}

static void load_notes(NotesApplication *application)
{
	unsigned int i;
	char **names;

	names = g_settings_get_strv(application->settings, "note-list");
	for (i = 0; names[i]; i++) {
		create_note(application, names[i]);
	}

	g_strfreev(names);
}

static void notes_application_init(NotesApplication *application)
{
	GSettings *settings;
	GHashTable *notes;

	settings = g_settings_new(APPLICATION_ID);
	notes = g_hash_table_new_full(g_str_hash, g_str_equal, free,
				      (GDestroyNotify) stickynote_free);

	application->settings = settings;
	application->notes = notes;
}

static void notes_application_finalise(GObject *object)
{
	NotesApplication *application = NOTES_APPLICATION(object);

	g_object_unref(application->settings);
	g_hash_table_unref(application->notes);
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
