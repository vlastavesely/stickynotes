#include "compat.h"
#include "stickynote.h"

struct StickyNote {
	GtkWindow parent;
	GSettings *settings;
	GtkWidget *title_label;
};

struct StickyNoteClass {
	GtkWindowClass parent;
};

enum {
	PROP_NAME = 1,
	PROP_TITLE,
	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL};

G_DEFINE_TYPE(StickyNote, stickynote, GTK_TYPE_WINDOW);

static void load_settings_for_name(StickyNote *note, const char *name)
{
	GSettings *settings;
	char *path;

	if (note->settings) {
		g_object_unref(note->settings);
	}

	path = g_strdup_printf("%s/%s/", RESOURCE_PATH, name);
	settings = g_settings_new_with_path(APPLICATION_ID ".note", path);
	free(path);

	note->settings = settings;
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	StickyNote *note = STICKYNOTE(object);
	const char *str;

	switch (prop_id) {
	case PROP_NAME:
		load_settings_for_name(note, g_value_get_string(value));
		break;

	case PROP_TITLE:
		str = g_value_get_string(value);
		gtk_label_set_label(GTK_LABEL(note->title_label), str);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void get_property(GObject *object, unsigned int prop_id, GValue *value,
			 GParamSpec *pspec)
{
	StickyNote *note = STICKYNOTE(object);
	const char *str;

	switch (prop_id) {
	case PROP_TITLE:
		str = gtk_label_get_label(GTK_LABEL(note->title_label));
		g_value_set_string(value, str);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void stickynote_init(StickyNote *note)
{
	gtk_widget_init_template(GTK_WIDGET(note));
}

static void finalise(GObject *object)
{
	StickyNote *note = STICKYNOTE(object);

	g_object_unref(note->settings);

	G_OBJECT_CLASS(stickynote_parent_class)->finalize(object);
}

static void constructed(GObject *object)
{
	StickyNote *note = STICKYNOTE(object);
	GSettings *settings = note->settings;

	g_settings_bind(settings, "title", note, "title",
			G_SETTINGS_BIND_DEFAULT);
}

static void stickynote_class_init(StickyNoteClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class,
				RESOURCE_PATH "/stickynote.ui");

	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, title_label);

	object_class->get_property = get_property;
	object_class->set_property = set_property;
	object_class->finalize = finalise;
	object_class->constructed = constructed;

	obj_properties[PROP_NAME] = g_param_spec_string("name",
			"Name", "Internal name of the note", "",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

	obj_properties[PROP_TITLE] = g_param_spec_string("title",
			"Title", "Public title of the note", "",
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	g_object_class_install_properties(object_class, N_PROPERTIES,
					  obj_properties);
}

StickyNote *stickynote_new(const char *name)
{
	return g_object_new(STICKYNOTE_TYPE, "name", name, NULL);
}

void stickynote_free(StickyNote *note)
{
	g_object_unref(note);
}
