#include "compat.h"
#include "stickynote.h"

struct StickyNote {
	GtkWindow parent;
	GSettings *settings;
	GtkWidget *title_label;
	GtkWidget *move_box;
	GtkWidget *resize_sw;
	GtkWidget *resize_se;
	int width, height, x, y;
};

struct StickyNoteClass {
	GtkWindowClass parent;
};

enum {
	PROP_NAME = 1,
	PROP_TITLE,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_X,
	PROP_Y,
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

static bool stickynote_resize(GtkWidget *widget, GdkEventButton *event,
			      StickyNote *note)
{
	unsigned int edge;

	if (event->type != GDK_BUTTON_PRESS || event->button != 1)
		return false;

	edge = (widget == note->resize_se) ? GDK_WINDOW_EDGE_SOUTH_EAST
		: GDK_WINDOW_EDGE_SOUTH_WEST;

	gtk_window_begin_resize_drag(GTK_WINDOW(note), edge, event->button,
				     event->x_root, event->y_root,
				     event->time);

	return true;
}

static bool stickynote_move(GtkWidget *widget, GdkEventButton *event,
			    StickyNote *note)
{
	if (event->type != GDK_BUTTON_PRESS || event->button != 1)
		return false;

	gtk_window_begin_move_drag(GTK_WINDOW(note), event->button,
				   event->x_root, event->y_root,
				   event->time);

	return true;
}

static bool save_geometry(GtkWidget *widget, GdkEventConfigure *event, void *data)
{
	GObject *object = G_OBJECT(widget);
	StickyNote *note = STICKYNOTE(widget);

	note->width = event->width;
	g_object_notify(object, "width");

	note->height = event->height;
	g_object_notify(object, "height");

	note->x = event->x;
	g_object_notify(object, "x");

	note->y = event->y;
	g_object_notify(object, "y");

	return false;
}

static void update_geometry(StickyNote *note)
{
	GtkWindow *window = GTK_WINDOW(note);

	gtk_window_resize(window, note->width, note->height);
	gtk_window_move(window, note->x, note->y);
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

	case PROP_WIDTH:
		note->width = g_value_get_int(value);
		update_geometry(note);
		break;

	case PROP_HEIGHT:
		note->height = g_value_get_int(value);
		update_geometry(note);
		break;

	case PROP_X:
		note->x = g_value_get_int(value);
		update_geometry(note);
		break;

	case PROP_Y:
		note->y = g_value_get_int(value);
		update_geometry(note);
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

	case PROP_WIDTH:
		g_value_set_int(value, note->width);
		break;

	case PROP_HEIGHT:
		g_value_set_int(value, note->height);
		break;

	case PROP_X:
		g_value_set_int(value, note->x);
		break;

	case PROP_Y:
		g_value_set_int(value, note->y);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void stickynote_init(StickyNote *note)
{
	gtk_widget_init_template(GTK_WIDGET(note));

	g_signal_connect(G_OBJECT(note->resize_sw), "button-press-event",
			 G_CALLBACK(stickynote_resize), note);

	g_signal_connect(G_OBJECT(note->resize_se), "button-press-event",
			 G_CALLBACK(stickynote_resize), note);

	g_signal_connect(G_OBJECT(note->move_box), "button-press-event",
			 G_CALLBACK(stickynote_move), note);

	g_signal_connect(G_OBJECT(note), "configure-event",
			 G_CALLBACK(save_geometry), NULL);

	/* Minimal default */
	note->width = 100;
	note->height = 100;
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

	g_settings_bind(settings, "width", note, "width",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "height", note, "height",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "x", note, "x",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "y", note, "y",
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

	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, move_box);
	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, resize_sw);
	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, resize_se);

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

	obj_properties[PROP_WIDTH] = g_param_spec_int("width",
			"Width", "Width of the note", 100, 3000, 100,
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	obj_properties[PROP_HEIGHT] = g_param_spec_int("height",
			"Height", "Height of the note", 100, 3000, 100,
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	obj_properties[PROP_X] = g_param_spec_int("x",
			"X", "Offset from the left", -5000, 5000, 100,
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	obj_properties[PROP_Y] = g_param_spec_int("y",
			"Y", "Offset from the top", -5000, 5000, 100,
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
