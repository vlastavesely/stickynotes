#include "compat.h"
#include "application.h"
#include "stickynote.h"
#include "indicator.h"

#define STYLESHEET_PATH RESOURCE_PATH "/stylesheet.css"

struct _NotesApplication {
	GtkApplication parent;
	StickyNotesIndicator *indicator;
	GSettings *settings;
	GHashTable *notes;
};

struct _NotesApplicationClass {
	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(NotesApplication, notes_application, GTK_TYPE_APPLICATION);

static void update_notes_list(NotesApplication *app)
{
	const char **keys;

	keys = (const char **) g_hash_table_get_keys_as_array(app->notes, NULL);
	g_settings_set_strv(app->settings, "note-list", keys);
	free(keys);
}

static void generate_unique_name(char *buf, GHashTable *notes)
{
	unsigned int i;

	for (i = 1; i; i++) {
		sprintf(buf, "note-%d", i);
		if (g_hash_table_lookup(notes, buf) == NULL) {
			return;
		}
	}
}

static void generate_initial_title(char *ptr)
{
	struct tm *local;
	time_t t;

	time(&t);
	local = localtime(&t);

	strftime(ptr, 32, "%Y-%m-%d", local);
}

static void note_destroy(StickyNote *note, void *data)
{
	NotesApplication *app = data;
	const char *name;

	name = stickynote_get_name(note);
	g_hash_table_remove(app->notes, name);

	update_notes_list(app);
}

static StickyNote *load_note(NotesApplication *app, const char *name)
{
	StickyNote *note;

	note = stickynote_new(name);
	g_hash_table_insert(app->notes, strdup(name), note);

	g_signal_connect(G_OBJECT(note), "destroy",
			 G_CALLBACK(note_destroy),
			 app);

	gtk_widget_show(GTK_WIDGET(note));

	return note;
}

static void load_notes(NotesApplication *app)
{
	unsigned int i;
	char **names;

	names = g_settings_get_strv(app->settings, "note-list");
	for (i = 0; names[i]; i++) {
		load_note(app, names[i]);
	}

	g_strfreev(names);
}

static StickyNote *create_note(NotesApplication *app)
{
	StickyNote *note;
	char buf[32];

	generate_unique_name(buf, app->notes);
	note = load_note(app, buf);

	generate_initial_title(buf);
	g_object_set(G_OBJECT(note), "title", buf, NULL);

	update_notes_list(app);

	return note;
}

static void action_new(GSimpleAction *action, GVariant *param, void *data)
{
	NotesApplication *app;

	app = NOTES_APPLICATION(data);
	create_note(app);
}

static GActionEntry actions[] = {
	{"new", action_new}
};

static void create_css_provider()
{
	GtkCssProvider *provider;

	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(provider, STYLESHEET_PATH);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
				GTK_STYLE_PROVIDER(provider), 600);
}

static void activate_notes(GHashTable *notes)
{
	GtkWindow *window;
	GList *walk;

	walk = g_hash_table_get_values(notes);

	for (; walk; walk = walk->next) {
		window = walk->data;

		gtk_window_deiconify(window);
		gtk_window_present(window);
	}

	g_list_free(walk);
}

static void notes_application_init(NotesApplication *app)
{
}

static void notes_application_startup(GApplication *application)
{
	GApplicationClass *app_class;
	NotesApplication *app;

	/* calls gtk_init() */
	app_class = G_APPLICATION_CLASS(notes_application_parent_class);
	app_class->startup(application);

	app = NOTES_APPLICATION(application);
	app->indicator = sticky_notes_indicator_new(app);
	app->settings = g_settings_new(APPLICATION_ID);

	g_action_map_add_action_entries(G_ACTION_MAP(app), actions,
					G_N_ELEMENTS(actions), app);

	app->notes = g_hash_table_new_full(g_str_hash, g_str_equal, free,
					   (GDestroyNotify) stickynote_free);

	create_css_provider();
	load_notes(app);
}

static void notes_application_activate(GApplication *application)
{
	NotesApplication *app;

	app = NOTES_APPLICATION(application);
	if (app->notes) {
		activate_notes(app->notes);
	}

	g_application_hold(application);
}

static void notes_application_class_init(NotesApplicationClass *class)
{
	GApplicationClass *app_class;

	app_class = G_APPLICATION_CLASS(class);
	app_class->startup = notes_application_startup;
	app_class->activate = notes_application_activate;
}

NotesApplication *notes_application_new(void)
{
	return g_object_new(NOTES_APPLICATION_TYPE, "application-id",
			    APPLICATION_ID, NULL);
}

GHashTable *notes_application_get_notes(NotesApplication *app)
{
	return app->notes;
}
