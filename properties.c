#include "compat.h"
#include "properties.h"

struct StickyNotePropertiesDialog {
	GtkDialog parent;
};

struct StickyNotePropertiesDialogClass {
	GtkDialogClass parent;
};

G_DEFINE_TYPE(StickyNotePropertiesDialog, sticky_note_properties_dialog,
	      GTK_TYPE_DIALOG);

static void sticky_note_properties_dialog_init(StickyNotePropertiesDialog *dialog)
{
	gtk_widget_init_template(GTK_WIDGET(dialog));
	gtk_window_set_icon_name(GTK_WINDOW(dialog), APPLICATION_ICON);
}

static void sticky_note_properties_dialog_class_init(StickyNotePropertiesDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class,
				RESOURCE_PATH "/properties.ui");
}

StickyNotePropertiesDialog *sticky_note_properties_dialog_new()
{
	return g_object_new(STICKY_NOTE_PROPERTIES_DIALOG_TYPE, NULL);
}

void properties_dialog_free(StickyNotePropertiesDialog *dialog)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}
