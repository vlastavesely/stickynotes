#include "compat.h"
#include "application.h"
#include "stickynote.h"
#include "indicator.h"

struct _NotesApplication {
	GtkApplication parent;
	StickynotesIndicator *indicator;
	GSettings *settings;
	GHashTable *notes;
};

struct _NotesApplicationClass {
	GtkApplicationClass parent_class;
};

NotesApplication *application = NULL;

G_DEFINE_TYPE(NotesApplication, notes_application, GTK_TYPE_APPLICATION);


static void action_new(GSimpleAction *action, GVariant *param, void *userdata)
{
	notes_application_create_note(application);
}

static GActionEntry actions[] = {
	{"new", action_new}
};

static void update_note_list(NotesApplication *app)
{
	const char **keys;

	keys = (const char **) g_hash_table_get_keys_as_array(app->notes, NULL);
	g_settings_set_strv(app->settings, "note-list", keys);
	free(keys);
}

static StickyNote *create_note(NotesApplication *application, const char *name)
{
	StickyNote *note;

	note = stickynote_new(name);
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

static void generate_initial_title(char *ptr)
{
	time_t t;
	struct tm *local;

	time(&t);
	local = localtime(&t);

	strftime(ptr, 32, "%Y-%m-%d", local);
}

StickyNote *notes_application_create_note(NotesApplication *application)
{
	StickyNote *note;
	unsigned int i;
	char buf[32];

	for (i = 1; i; i++) {
		snprintf(buf, sizeof(buf), "note-%d", i);
		if (g_hash_table_lookup(application->notes, buf) == NULL) {
			break;
		}
	}

	note = notes_application_open_note(application, buf);
	generate_initial_title(buf);
	g_object_set(G_OBJECT(note), "title", buf, NULL);

	return note;
}

void notes_application_close_note(NotesApplication *application,
				  const char *name)
{
	g_hash_table_remove(application->notes, name);
	update_note_list(application);
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

static void load_stylesheet()
{
	GtkCssProvider *provider;

	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(provider, RESOURCE_PATH "/stylesheet.css");

	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
				GTK_STYLE_PROVIDER(provider),
				GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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
	stickynotes_indicator_free(application->indicator);
	g_hash_table_unref(application->notes);
}

static void notes_application_activate(GApplication *application)
{
	NotesApplication *app = NOTES_APPLICATION(application);

	load_notes(app);
	load_stylesheet();

	app->indicator = stickynotes_indicator_new();

	g_action_map_add_action_entries(G_ACTION_MAP(application),
		actions, G_N_ELEMENTS(actions), NULL);

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

GHashTable *notes_application_get_notes(NotesApplication *application)
{
	return application->notes;
}
