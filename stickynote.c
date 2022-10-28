#include "compat.h"
#include "stickynote.h"
#include "application.h"
#include "properties.h"

#include <gtksourceview/gtksource.h>

#define LOCKED_ICON RESOURCE_PATH "/locked.png"
#define UNLOCKED_ICON RESOURCE_PATH "/unlocked.png"

static const char *menu_str =
	"<interface>"
	"  <menu id=\"menu\">"
	"    <section>"
	"      <item>"
	"        <attribute name=\"label\">Properties</attribute>"
	"        <attribute name=\"action\">note.properties</attribute>"
	"      </item>"
	"    </section>"
	"  </menu>"
	"</interface>";

struct StickyNote {
	GtkWindow parent;
	GSettings *settings;
	bool constructed;
	GtkMenu *popup;
	char *name;
	GtkCssProvider *css_provider;
	GtkWidget *title_label;
	GtkWidget *lock_button;
	GtkWidget *lock_image;
	GtkWidget *close_button;
	GtkWidget *move_box;
	GtkWidget *resize_sw;
	GtkWidget *resize_se;
	GtkWidget *text_view_area;
	GtkWidget *text_view;
	int width, height, x, y;
	bool locked;
	char *colour, *font_colour, *font;
};

struct StickyNoteClass {
	GtkWindowClass parent;
};

enum {
	PROP_NAME = 1,
	PROP_TITLE,
	PROP_TEXT,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_X,
	PROP_Y,
	PROP_LOCKED,
	PROP_COLOUR,
	PROP_FONT_COLOUR,
	PROP_FONT,
	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL};

G_DEFINE_TYPE(StickyNote, stickynote, GTK_TYPE_WINDOW);

static char *get_text(GtkSourceView *view)
{
	GtkTextIter start, end;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);

	return gtk_text_buffer_get_text(buffer, &start, &end, true);
}

static void set_text(GtkSourceView *view, const char *text)
{
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_buffer_set_text(buffer, text, -1);
}

static GtkWidget *create_text_view()
{
	GtkWidget *view;

	view = gtk_source_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
	gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(view), TRUE);
	gtk_widget_set_name(view, "text_view");
	gtk_widget_show_all(view);

	return view;
}

static void text_changed(GtkTextBuffer *buffer, void *data)
{
	g_object_notify(G_OBJECT(data), "text");
}

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

static bool stickynote_popup(GtkWidget *widget, GdkEvent *event,
			     StickyNote *note)
{
	if (event->button.button == 3) {
		gtk_menu_popup_at_pointer(note->popup, (const GdkEvent *) event);
	}

	return false;
}

static bool can_close(StickyNote *note)
{
	GtkWidget *dialog;
	bool ret;

	dialog = gtk_message_dialog_new(GTK_WINDOW(note), GTK_DIALOG_MODAL,
					GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
					"Delete this stickynote?\nThis cannot be undone.");
	gtk_window_set_title(GTK_WINDOW(dialog), "Sticky Notes");

	ret = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES;
	gtk_widget_destroy(dialog);

	return ret;
}

static void reset_settings(StickyNote *note)
{
	GSettingsSchema *schema;
	GSettings *settings;
	unsigned int i;
	char **keys;

	settings = note->settings;
	g_object_get(G_OBJECT(settings), "settings-schema", &schema, NULL);

	keys = g_settings_schema_list_keys(schema);
	for (i = 0; keys[i]; i++) {
		g_settings_unbind(note, keys[i]);
		g_settings_reset(settings, keys[i]);
	}

	g_strfreev(keys);
}

static bool close_request(GtkWidget *widget, GdkEvent *event, StickyNote *note)
{
	if (!can_close(note)) {
		return true;
	}

	reset_settings(note);
	notes_application_close_note(application, note->name);

	return false;
}

static void close_button_click(GtkWidget *button, StickyNote *note)
{
	gtk_window_close(GTK_WINDOW(note));
}

static void lock_button_click(GtkWidget *button, StickyNote *note)
{
	g_object_set(G_OBJECT(note), "locked", !note->locked, NULL);
}

static bool save_geometry(GtkWidget *widget, GdkEventConfigure *event, void *data)
{
	GObject *object = G_OBJECT(widget);
	StickyNote *note = STICKYNOTE(widget);

	if (note->width != event->width) {
		note->width = event->width;
		g_object_notify(object, "width");
	}

	if (note->height != event->height) {
		note->height = event->height;
		g_object_notify(object, "height");
	}

	if (note->x != event->x) {
		note->x = event->x;
		g_object_notify(object, "x");
	}

	if (note->y != event->y) {
		note->y = event->y;
		g_object_notify(object, "y");
	}

	return false;
}

static void update_geometry(StickyNote *note)
{
	GtkWindow *window;

	if (note->constructed == false)
		return;

	window = GTK_WINDOW(note);
	gtk_window_resize(window, note->width, note->height);
	gtk_window_move(window, note->x, note->y);
}

static void update_ui(StickyNote *note)
{
	bool locked;

	if (note->constructed == false)
		return;

	locked = note->locked;
	gtk_text_view_set_editable(GTK_TEXT_VIEW(note->text_view), !locked);
	gtk_image_set_from_resource(GTK_IMAGE(note->lock_image),
				    locked ? LOCKED_ICON : UNLOCKED_ICON);
}

static const char *get_font_style(unsigned int style)
{
	if (style & PANGO_STYLE_ITALIC)
		return "italic";

	return "normal";
}

static void update_css(StickyNote *note)
{
	PangoFontDescription *desc;
	char *colour, *font_colour, *font;
	char css[512] = {};

	g_object_get(G_OBJECT(note), "colour", &colour,
		     "font-colour", &font_colour, "font", &font, NULL);

	if (!colour || !font_colour || !font)
		return;

	desc = pango_font_description_from_string(font);

	snprintf(css, sizeof(css),
		 "window {"
		 "  background: %s;"
		 "  color: %s;"
		 "  font-size: %dpt;"
		 "  font-family: %s;"
		 "  font-style: %s;"
		 "  font-weight: %d;"
		 "}",
		 colour, font_colour,
		 PANGO_PIXELS(pango_font_description_get_size(desc)),
		 pango_font_description_get_family(desc),
		 get_font_style(pango_font_description_get_style(desc)),
		 pango_font_description_get_weight(desc));

	pango_font_description_free(desc);

	free(colour);
	free(font_colour);
	free(font);

	gtk_css_provider_load_from_data(note->css_provider, css, -1, NULL);
}

static void action_properties(GSimpleAction *action, GVariant *param,
			      void *note)
{
	StickyNotePropertiesDialog *dialog;

	dialog = sticky_note_properties_dialog_new(note);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(note));

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static GActionEntry actions[] = {
	{"properties", action_properties},
};

static void register_actions(GtkMenu *menu, StickyNote *note)
{
	GSimpleActionGroup *group;

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), actions,
					G_N_ELEMENTS(actions), note);

	gtk_widget_insert_action_group(GTK_WIDGET(menu), "note",
				       G_ACTION_GROUP(group));
}

static GtkMenu *create_popup_menu(StickyNote *note)
{
	GtkBuilder *builder;
	GMenuModel *model;
	GtkMenu *menu;

	builder = gtk_builder_new_from_string(menu_str, -1);
	model = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
	menu = GTK_MENU(gtk_menu_new_from_model(model));
	g_object_unref(builder);

	register_actions(menu, note);

	gtk_widget_show_all(GTK_WIDGET(menu));

	return menu;
}

#define SET_STRING(ptr, str) {	\
	free(ptr);		\
	ptr = strdup(str);	\
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	StickyNote *note = STICKYNOTE(object);
	const char *str;

	switch (prop_id) {
	case PROP_NAME:
		SET_STRING(note->name, g_value_get_string(value));
		load_settings_for_name(note, g_value_get_string(value));
		break;

	case PROP_TITLE:
		str = g_value_get_string(value);
		gtk_label_set_label(GTK_LABEL(note->title_label), str);
		break;

	case PROP_TEXT:
		str = g_value_get_string(value);
		set_text(GTK_SOURCE_VIEW(note->text_view), str);
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

	case PROP_COLOUR:
		if (g_value_get_string(value)) {
			SET_STRING(note->colour, g_value_get_string(value));
		} else {
			g_settings_reset(note->settings, "colour");
		}
		update_css(note);
		break;

	case PROP_FONT_COLOUR:
		if (g_value_get_string(value)) {
			SET_STRING(note->font_colour, g_value_get_string(value));
		} else {
			g_settings_reset(note->settings, "font-colour");
		}
		update_css(note);
		break;

	case PROP_FONT:
		if (g_value_get_string(value)) {
			SET_STRING(note->font, g_value_get_string(value));
		} else {
			g_settings_reset(note->settings, "font");
		}
		update_css(note);
		break;

	case PROP_LOCKED:
		note->locked = g_value_get_boolean(value);
		update_ui(note);
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
	char *text;

	switch (prop_id) {
	case PROP_TITLE:
		str = gtk_label_get_label(GTK_LABEL(note->title_label));
		g_value_set_string(value, str);
		break;

	case PROP_TEXT:
		text = get_text(GTK_SOURCE_VIEW(note->text_view));
		g_value_set_string(value, text);
		free(text);
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

	case PROP_COLOUR:
		g_value_set_string(value, note->colour);
		break;

	case PROP_FONT_COLOUR:
		g_value_set_string(value, note->font_colour);
		break;

	case PROP_FONT:
		g_value_set_string(value, note->font);
		break;

	case PROP_LOCKED:
		g_value_set_boolean(value, note->locked);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

bool stickynote_has_default_colours(StickyNote *note)
{
	GVariant *value;
	const char *str;
	int ret = 0;

	value = g_settings_get_default_value(note->settings, "colour");

	str = g_variant_get_string(value, NULL);
	if (strcmp(str, note->colour) == 0)
		ret++;

	g_variant_unref(value);

	value = g_settings_get_default_value(note->settings, "font-colour");

	str = g_variant_get_string(value, NULL);
	if (strcmp(str, note->font_colour) == 0)
		ret++;

	g_variant_unref(value);

	return ret == 2; /* both colours are default */
}

bool stickynote_has_default_font(StickyNote *note)
{
	GVariant *value;
	int ret;

	value = g_settings_get_default_value(note->settings, "font");
	ret = strcmp(g_variant_get_string(value, NULL), note->font) == 0;
	g_variant_unref(value);

	return ret;
}

static GtkCssProvider *make_css_provider(StickyNote *note)
{
	GtkStyleContext *context;
	GtkCssProvider *provider;

	context = gtk_widget_get_style_context(GTK_WIDGET(note));
	provider = gtk_css_provider_new();
	gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
				       GTK_STYLE_PROVIDER_PRIORITY_USER);

	return provider;
}

static void stickynote_init(StickyNote *note)
{
	GtkTextBuffer *buffer;

	gtk_widget_init_template(GTK_WIDGET(note));

	note->popup = create_popup_menu(note);

	note->text_view = create_text_view();
	gtk_container_add(GTK_CONTAINER(note->text_view_area), note->text_view);

	note->css_provider = make_css_provider(note);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->text_view));
	g_signal_connect(G_OBJECT(buffer), "changed",
			 G_CALLBACK(text_changed), note);

	g_signal_connect(G_OBJECT(note->lock_button), "clicked",
			 G_CALLBACK(lock_button_click), note);

	g_signal_connect(G_OBJECT(note->close_button), "clicked",
			 G_CALLBACK(close_button_click), note);

	g_signal_connect(G_OBJECT(note), "delete-event",
			 G_CALLBACK(close_request), note);

	g_signal_connect(G_OBJECT(note->resize_sw), "button-press-event",
			 G_CALLBACK(stickynote_resize), note);

	g_signal_connect(G_OBJECT(note->resize_se), "button-press-event",
			 G_CALLBACK(stickynote_resize), note);

	g_signal_connect(G_OBJECT(note->move_box), "button-press-event",
			 G_CALLBACK(stickynote_move), note);

	g_signal_connect(G_OBJECT(note->move_box), "button-press-event",
			 G_CALLBACK(stickynote_popup), note);

	g_signal_connect(G_OBJECT(note), "configure-event",
			 G_CALLBACK(save_geometry), NULL);
}

static void finalise(GObject *object)
{
	StickyNote *note = STICKYNOTE(object);

	free(note->name);
	free(note->colour);
	free(note->font);

	g_object_unref(note->settings);

	G_OBJECT_CLASS(stickynote_parent_class)->finalize(object);
}

static void constructed(GObject *object)
{
	StickyNote *note = STICKYNOTE(object);
	GSettings *settings = note->settings;

	g_settings_bind(settings, "title", note, "title",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "text", note, "text",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "width", note, "width",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "height", note, "height",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "x", note, "x",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "y", note, "y",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "colour", note, "colour",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "font-colour", note, "font-colour",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "font", note, "font",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(settings, "locked", note, "locked",
			G_SETTINGS_BIND_DEFAULT);

	note->constructed = true;
	update_geometry(note);
	update_ui(note);
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
					     StickyNote, lock_button);
	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, lock_image);
	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, close_button);

	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, move_box);
	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, resize_sw);
	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, resize_se);

	gtk_widget_class_bind_template_child(widget_class,
					     StickyNote, text_view_area);

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

	obj_properties[PROP_TEXT] = g_param_spec_string("text",
			"Text", "Text of the note", "",
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

	obj_properties[PROP_COLOUR] = g_param_spec_string("colour",
			"Colour", "The background colour of the note", "",
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	obj_properties[PROP_FONT_COLOUR] = g_param_spec_string("font-colour",
			"Font colour", "The font colour of the note", "",
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	obj_properties[PROP_FONT] = g_param_spec_string("font",
			"Font", "The font of the note", "",
			G_PARAM_READABLE | G_PARAM_WRITABLE);

	obj_properties[PROP_LOCKED] = g_param_spec_boolean("locked", "Locked",
			"Whether the note is locked", 0, G_PARAM_READWRITE);

	g_object_class_install_properties(object_class, N_PROPERTIES,
					  obj_properties);
}

StickyNote *stickynote_new(const char *name)
{
	return g_object_new(STICKYNOTE_TYPE, "name", name, NULL);
}

void stickynote_free(StickyNote *note)
{
	gtk_widget_destroy(GTK_WIDGET(note));
}
