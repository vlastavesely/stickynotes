#include "compat.h"
#include "stickynote.h"
#include "properties.h"

struct StickyNotePropertiesDialog {
	GtkDialog parent;
	StickyNote *note;
	GtkWidget *check_default_colours;
	GtkWidget *check_default_font;
	GtkWidget *box_colours;
	GtkWidget *box_font;
	GtkWidget *title_entry;
	GtkWidget *colour_button;
	GtkWidget *font_colour_button;
	GtkWidget *font_button;
	GtkWidget *close_button;
};

struct StickyNotePropertiesDialogClass {
	GtkDialogClass parent;
};

enum {
	PROP_NOTE = 1,
	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL};

G_DEFINE_TYPE(StickyNotePropertiesDialog, sticky_note_properties_dialog,
	      GTK_TYPE_DIALOG);

static void set_colour(GtkColorChooser *chooser, const char *hex)
{
	GdkRGBA colour;

	gdk_rgba_parse(&colour, hex);
	gtk_color_chooser_set_rgba(chooser, &colour);
}

static char *get_colour(GtkColorChooser *chooser)
{
	GdkRGBA colour;

	gtk_color_chooser_get_rgba(chooser, &colour);

	return gdk_rgba_to_string(&colour);
}

static void title_changed(GtkEntry *entry, StickyNotePropertiesDialog *dialog)
{
	const char *text;

	text = gtk_entry_get_text(entry);
	g_object_set(G_OBJECT(dialog->note), "title", text, NULL);
}

static void colour_set(GtkColorChooser *chooser,
		       StickyNotePropertiesDialog *dialog)
{
	char *hex;

	hex = get_colour(chooser);
	g_object_set(G_OBJECT(dialog->note), "colour", hex, NULL);
	free(hex);
}

static void font_colour_set(GtkColorChooser *chooser,
			    StickyNotePropertiesDialog *dialog)
{
	char *hex;

	hex = get_colour(chooser);
	g_object_set(G_OBJECT(dialog->note), "font-colour", hex, NULL);
	free(hex);
}

static void font_set(GtkFontChooser *chooser, StickyNotePropertiesDialog *dialog)
{
	PangoFontDescription *desc;
	char *font;

	desc = gtk_font_chooser_get_font_desc(chooser);
	font = pango_font_description_to_string(desc);

	g_object_set(G_OBJECT(dialog->note), "font", font, NULL);

	pango_font_description_free(desc);
	free(font);
}

static void close_dialog(GtkButton *button, GtkDialog *dialog)
{
	gtk_dialog_response(dialog, GTK_RESPONSE_CLOSE);
}

static void chooser_set_font(GtkFontChooser *chooser, StickyNote *note)
{
	PangoFontDescription *desc;
	char *font;

	g_object_get(G_OBJECT(note), "font", &font, NULL);

	desc = pango_font_description_from_string(font);
	gtk_font_chooser_set_font_desc(chooser, desc);

	pango_font_description_free(desc);
	free(font);
}

static void default_colours_toggled(GtkToggleButton *button, void *data)
{
	StickyNotePropertiesDialog *dialog = data;
	bool sensitive;

	sensitive = !gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(dialog->box_colours, sensitive);

	if (sensitive == false) {
		g_object_set(G_OBJECT(dialog->note), "colour", NULL, "font-colour", NULL, NULL);
	} else {
		colour_set(dialog->colour_button, dialog);
		font_colour_set(dialog->font_colour_button, dialog);
	}
}

static void default_font_toggled(GtkToggleButton *button, void *data)
{
	StickyNotePropertiesDialog *dialog = data;
	bool sensitive;

	sensitive = !gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(dialog->box_font, sensitive);

	if (sensitive == false) {
		g_object_set(G_OBJECT(dialog->note), "font", NULL, NULL);
	} else {
		font_set(dialog->font_button, dialog);
	}
}

static void init_ui(StickyNotePropertiesDialog *dialog)
{
	StickyNote *note = dialog->note;
	char *str;

	g_object_get(G_OBJECT(note), "title", &str, NULL);
	gtk_entry_set_text(dialog->title_entry, str);
	free(str);

	g_object_get(G_OBJECT(note), "colour", &str, NULL);
	set_colour(GTK_COLOR_CHOOSER(dialog->colour_button), str);
	free(str);

	g_object_get(G_OBJECT(note), "font-colour", &str, NULL);
	set_colour(GTK_COLOR_CHOOSER(dialog->font_colour_button), str);
	free(str);

	chooser_set_font(GTK_FONT_CHOOSER(dialog->font_button), note);

	if (stickynote_has_default_colours(note)) {
		gtk_toggle_button_set_active(dialog->check_default_colours, true);
	}

	if (stickynote_has_default_font(note)) {
		gtk_toggle_button_set_active(dialog->check_default_font, true);
	}
}

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	StickyNotePropertiesDialog *dialog;

	dialog = STICKY_NOTE_PROPERTIES_DIALOG(object);

	switch (prop_id) {
	case PROP_NOTE:
		dialog->note = g_value_get_pointer(value);
		init_ui(dialog);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void sticky_note_properties_dialog_init(StickyNotePropertiesDialog *dialog)
{
	gtk_widget_init_template(GTK_WIDGET(dialog));
	gtk_window_set_icon_name(GTK_WINDOW(dialog), APPLICATION_ICON);

	g_signal_connect(G_OBJECT(dialog->title_entry), "changed",
			 G_CALLBACK(title_changed), dialog);

	g_signal_connect(G_OBJECT(dialog->colour_button), "color-set",
			 G_CALLBACK(colour_set), dialog);

	g_signal_connect(G_OBJECT(dialog->font_colour_button), "color-set",
			 G_CALLBACK(font_colour_set), dialog);

	g_signal_connect(G_OBJECT(dialog->font_button), "font-set",
			 G_CALLBACK(font_set), dialog);

	g_signal_connect(G_OBJECT(dialog->close_button), "clicked",
			 G_CALLBACK(close_dialog), dialog);

	g_signal_connect(G_OBJECT(dialog->check_default_colours), "toggled",
			 G_CALLBACK(default_colours_toggled), dialog);

	g_signal_connect(G_OBJECT(dialog->check_default_font), "toggled",
			 G_CALLBACK(default_font_toggled), dialog);
}

static void sticky_note_properties_dialog_class_init(StickyNotePropertiesDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class,
				RESOURCE_PATH "/properties.ui");

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, check_default_colours);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, check_default_font);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, box_colours);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, box_font);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, title_entry);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, colour_button);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, font_colour_button);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, font_button);

	gtk_widget_class_bind_template_child(widget_class,
			StickyNotePropertiesDialog, close_button);

	object_class->set_property = set_property;

	obj_properties[PROP_NOTE] = g_param_spec_pointer("note",
			"Note", "The note",
			G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

	g_object_class_install_properties(object_class, N_PROPERTIES,
					  obj_properties);
}

StickyNotePropertiesDialog *sticky_note_properties_dialog_new(StickyNote *note)
{
	return g_object_new(STICKY_NOTE_PROPERTIES_DIALOG_TYPE, "note",
			    note, NULL);
}
